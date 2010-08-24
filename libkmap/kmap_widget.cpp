/** ===========================================================
 *
 * This file is a part of digiKam project
 * <a href="http://www.digikam.org">http://www.digikam.org</a>
 *
 * @date   2009-12-01
 * @brief  world map widget library
 *
 * @author Copyright (C) 2009-2010 by Michael G. Hansen
 *         <a href="mailto:mike at mghansen dot de">mike at mghansen dot de</a>
 * @author Copyright (C) 2010 by Gilles Caulier
 *         <a href="mailto:caulier dot gilles at gmail dot com">caulier dot gilles at gmail dot com</a>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#include "kmap_widget.moc"

// C++ includes

#include <math.h>

// Qt includes

#include <QBitmap>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QItemSelectionModel>
#include <QMenu>
#include <QPainter>
#include <QPointer>
#include <QStackedLayout>
#include <QTimer>
#include <QToolButton>

// KDE includes

#include <kaction.h>
#include <kconfig.h>
#include <kconfiggroup.h>
#include <khbox.h>
#include <klocale.h>
#include <kseparator.h>

// Marbel Widget includes

#include <marble/global.h>
#include <marble/GeoDataLineString.h>

// local includes

#include "kmap_common.h"
// #include "backend-osm.h"
#include "backend-marble.h"
#include "backend-googlemaps.h"
#include "backend-altitude-geonames.h"
#include "abstractmarkertiler.h"
#include "map-backend.h"
#include "kmap_dragdrophandler.h"
#include "version.h"
#include "kmap_modelhelper.h"

using namespace Marble;

namespace KMap
{

/**
 * @class KMapWidget
 * @brief The central map view class of libkmap
 *
 * The KMapWidget class is the central widget of libkmap. It provides a widget which can display maps using
 * either the Marble or Google Maps backend. Using a model, items can be displayed on the map. For
 * models containing only a small number of items, the items can be shown directly, but for models with
 * a larger number of items, the items can also be grouped. Currently, any number of ungrouped models
 * can be shown, but only one grouped model. Item selection models can also be used along with the models,
 * to interact with the selection states of the items on the map. In order to use a model with libkmap, however,
 * a model helper has to be implemented, which extracts data from the model that is not provided by the Qt part
 * of a model's API.
 * 
 * Now, a brief introduction on how to get libkmap working is provided:
 * @li First, an instance of @c KMapWidget has to be created.
 * @li Next, @c ModelHelper has to be subclassed and at least the pure virtual functions have to be implemented.
 * @li To show the model's data ungrouped, the model helper has to be added to @c KMapWidget instance using addUngroupedModel.
 * @li To show the model's data grouped, an instance of @c AbstractMarkerTiler has to be created and the model helper has to be
 *     set to it using setMarkerModelHelper. The @c AbstractMarkerTiler has then to be given to KMapWidget using setGroupedModel. If
 *     the items to be displayed do not reside in a model, a subclass of @c AbstractMarkerTiler can be created which returns
 *     just the number of items in a particular area, and picks representative items for thumbnails.
 * @li To handle dropping of items from the host applications UI onto the map, @c DragDropHandler has to be subclassed
 *     as well and added to the model using setDragDropHandler.
 */

const int KMapMinEditGroupingRadius = 1;
const int KMapMinGroupingRadius     = 15;
const int KMapMinThumbnailSize      = 30;

/**
 * @brief Helper function, returns the square of the distance between two points
 *
 * @param a Point a
 * @param b Point b
 * @return Square of the distance between a and b
 */
inline int QPointSquareDistance(const QPoint& a, const QPoint& b)
{
    return (a.x()-b.x())*(a.x()-b.x()) + (a.y()-b.y())*(a.y()-b.y());
}

class KMapWidget::KMapWidgetPrivate
{
public:

    KMapWidgetPrivate()
      : loadedAltitudeBackends(),
        loadedBackends(),
        currentBackend(0),
        currentBackendReady(false),
        currentBackendName(),
        stackedLayout(0),
        cacheCenterCoordinate(52.0,6.0),
        cacheZoom("marble:900"),
        cacheSelectionRectangle(),
        configurationMenu(0),
        actionGroupBackendSelection(0),
        actionZoomIn(0),
        actionZoomOut(0),
        actionBrowseMode(0),
        actionEditMode(0),
        actionGroupMode(0),
        browseModeControlsHolder(0),
        mouseModesHolder(0),
        controlWidget(0),
        lazyReclusteringRequested(false),
        clustersDirty(false),
        editModeAvailable(false),
        dragDropHandler(0),
        sortMenu(0),
        thumbnailSize(KMapMinThumbnailSize),
        groupingRadius(KMapMinGroupingRadius),
        editGroupingRadius(KMapMinEditGroupingRadius),
        actionIncreaseThumbnailSize(0),
        actionDecreaseThumbnailSize(0),
        actionSetSelectionMode(0),
        actionSetPanMode(0),
        actionSetZoomMode(0),
        actionSetFilterMode(0),
        actionRemoveFilterMode(0),
        actionSetSelectThumbnailMode(0),
        currentMouseMode(MouseModePan),
        modelBasedFilter(false),
        thumbnailTimer(0),
        thumbnailTimerCount(0),
        thumbnailsHaveBeenLoaded(false),
        activeState(false),
        hasSelection(false),
        availableMouseModes(0),
        visibleMouseModes(0)
    {
    }

    QList<AltitudeBackend*> loadedAltitudeBackends;
    QList<MapBackend*>      loadedBackends;
    MapBackend*             currentBackend;
    bool                    currentBackendReady;
    QString                 currentBackendName;
    QStackedLayout*         stackedLayout;

    // these values are cached in case the backend is not ready:
    GeoCoordinates        cacheCenterCoordinate;
    QString                 cacheZoom;
    QList<qreal>            cacheSelectionRectangle;

    // actions for controlling the widget
    QMenu*                  configurationMenu;
    QActionGroup*           actionGroupBackendSelection;
    KAction*                actionZoomIn;
    KAction*                actionZoomOut;
    KAction*                actionBrowseMode;
    KAction*                actionEditMode;
    QActionGroup*           actionGroupMode;
    QWidget*                browseModeControlsHolder;
    QWidget*                mouseModesHolder;
    QPointer<KHBox>         controlWidget;
    KAction*                actionPreviewSingleItems;
    KAction*                actionPreviewGroupedItems;
    KAction*                actionShowNumbersOnItems;

    bool                    lazyReclusteringRequested;
    bool                    clustersDirty;
    bool                    editModeAvailable;

    DragDropHandler*        dragDropHandler;

    QMenu*                  sortMenu;
    int                     thumbnailSize;
    int                     groupingRadius;
    int                     editGroupingRadius;
    KAction*                actionIncreaseThumbnailSize;
    KAction*                actionDecreaseThumbnailSize;
    KHBox*                  hBoxForAdditionalControlWidgetItems;

    QList<qreal>            selectionRectangle;
    QList<qreal>            oldSelectionRectangle;

    KAction*                actionRemoveCurrentSelection;
    KAction*                actionSetSelectionMode;
    KAction*                actionSetPanMode;
    KAction*                actionSetZoomMode;
    KAction*                actionSetFilterMode;
    KAction*                actionRemoveFilterMode;
    KAction*                actionSetSelectThumbnailMode;
    MouseModes              currentMouseMode;
    QToolButton*            setPanModeButton;
    QToolButton*            setSelectionModeButton;
    QToolButton*            removeCurrentSelectionButton;
    QToolButton*            setZoomModeButton;
    QToolButton*            setFilterModeButton;
    QToolButton*            removeFilterModeButton;
    QToolButton*            setSelectThumbnailMode;
    
    bool                    modelBasedFilter;

    QTimer*                 thumbnailTimer;
    int                     thumbnailTimerCount;
    bool                    thumbnailsHaveBeenLoaded;

    bool                    activeState;
    bool                    hasSelection;
    MouseModes              availableMouseModes;
    MouseModes              visibleMouseModes;
};

KMapWidget::KMapWidget(QWidget* const parent)
    : QWidget(parent), s(new WMWSharedData), d(new KMapWidgetPrivate)
{
    createActions();

    s->worldMapWidget = this;

    d->stackedLayout = new QStackedLayout(this);
    setLayout(d->stackedLayout);

    d->loadedBackends.append(new BackendGoogleMaps(s, this));
    d->loadedBackends.append(new BackendMarble(s, this));
//     d->loadedBackends.append(new BackendOSM(s, this));
    createActionsForBackendSelection();

    AltitudeBackend* const geonamesBackend = new BackendAltitudeGeonames(s, this);
    d->loadedAltitudeBackends.append(geonamesBackend);
    connect(geonamesBackend, SIGNAL(signalAltitudes(const KMap::WMWAltitudeLookup::List)),
            this, SIGNAL(signalAltitudeLookupReady(const KMap::WMWAltitudeLookup::List)));

    setAcceptDrops(true);
}

void KMapWidget::createActions()
{
    d->actionZoomIn = new KAction(this);
    d->actionZoomIn->setIcon(SmallIcon("zoom-in"));
    d->actionZoomIn->setToolTip(i18n("Zoom in"));
    connect(d->actionZoomIn, SIGNAL(triggered()),
            this, SLOT(slotZoomIn()));

    d->actionZoomOut = new KAction(this);
    d->actionZoomOut->setIcon(SmallIcon("zoom-out"));
    d->actionZoomOut->setToolTip(i18n("Zoom out"));
    connect(d->actionZoomOut, SIGNAL(triggered()),
            this, SLOT(slotZoomOut()));

    // actions to switch between edit mode and browse mode
    d->actionGroupMode = new QActionGroup(this);
    d->actionGroupMode->setExclusive(true);

    d->actionEditMode = new KAction(d->actionGroupMode);
    // TODO: icon
    d->actionEditMode->setText("E");
    d->actionEditMode->setToolTip(i18n("Switch to edit mode"));
    d->actionEditMode->setCheckable(true);

    d->actionBrowseMode = new KAction(d->actionGroupMode);
    // TODO: icon
    //d->actionBrowseMode->setText("B");
    d->actionBrowseMode->setIcon(SmallIcon("folder-image"));
    d->actionBrowseMode->setToolTip(i18n("Switch to browse mode"));
    d->actionBrowseMode->setCheckable(true);
    d->actionBrowseMode->setChecked(true);

    connect(d->actionGroupMode, SIGNAL(triggered(QAction*)),
            this, SLOT(slotGroupModeChanged(QAction*)));

    // create backend selection entries:
    d->actionGroupBackendSelection = new QActionGroup(this);
    d->actionGroupBackendSelection->setExclusive(true);
    connect(d->actionGroupBackendSelection, SIGNAL(triggered(QAction*)),
            this, SLOT(slotChangeBackend(QAction*)));

    createActionsForBackendSelection();

    d->configurationMenu = new QMenu(this);

    d->actionPreviewSingleItems = new KAction(i18n("Preview single items"), this);
    d->actionPreviewSingleItems->setCheckable(true);
    d->actionPreviewSingleItems->setChecked(true);
    d->actionPreviewGroupedItems = new KAction(i18n("Preview grouped items"), this);
    d->actionPreviewGroupedItems->setCheckable(true);
    d->actionPreviewGroupedItems->setChecked(true);
    d->actionShowNumbersOnItems = new KAction(i18n("Show numbers"), this);
    d->actionShowNumbersOnItems->setCheckable(true);
    d->actionShowNumbersOnItems->setChecked(true);

    d->actionIncreaseThumbnailSize = new KAction(i18n("T+"), this);
    d->actionIncreaseThumbnailSize->setToolTip(i18n("Increase the thumbnail size on the map"));
    d->actionDecreaseThumbnailSize = new KAction(i18n("T-"), this);
    d->actionDecreaseThumbnailSize->setToolTip(i18n("Decrease the thumbnail size on the map"));

    d->actionRemoveCurrentSelection = new KAction(this);
    //d->actionRemoveCurrentSelection->setEnabled(false);
    d->actionRemoveCurrentSelection->setIcon(SmallIcon("edit-clear"));
    d->actionRemoveCurrentSelection->setToolTip("Removes current selection.");

    d->actionSetSelectionMode = new KAction(this);
    d->actionSetSelectionMode->setCheckable(true);
    d->actionSetSelectionMode->setIcon(SmallIcon("select-rectangular"));
    d->actionSetSelectionMode->setToolTip(i18n("Select images by drawing a rectangle."));

    d->actionSetPanMode = new KAction(this);
    d->actionSetPanMode->setCheckable(true);
    d->actionSetPanMode->setToolTip(i18n("Pan mode."));
    d->actionSetPanMode->setIcon(SmallIcon("transform-move"));
    d->actionSetPanMode->setChecked(true);

    d->actionSetZoomMode = new KAction(this);
    d->actionSetZoomMode->setCheckable(true);
    d->actionSetZoomMode->setToolTip(i18n("Zoom into a group."));
    d->actionSetZoomMode->setIcon(SmallIcon("page-zoom"));

    d->actionSetFilterMode = new KAction(this);
    d->actionSetFilterMode->setCheckable(true);
    d->actionSetFilterMode->setToolTip(i18n("Filter images"));
    d->actionSetFilterMode->setIcon(SmallIcon("view-filter"));

    d->actionRemoveFilterMode = new KAction(this);
    d->actionRemoveFilterMode->setToolTip(i18n("Remove the current filter"));
    d->actionRemoveFilterMode->setIcon(SmallIcon("window-close"));

    d->actionSetSelectThumbnailMode = new KAction(this);
    d->actionSetSelectThumbnailMode->setCheckable(true);
    d->actionSetSelectThumbnailMode->setToolTip(i18n("Select images"));
    d->actionSetSelectThumbnailMode->setIcon(SmallIcon("edit-select"));

    // TODO: for later actions
//     action->setToolTip(i18n("Zoom into a group"));
//     action->setIcon(SmallIcon("page-zoom"));
//     action->setToolTip(i18n("Filter images"));
//     action->setIcon(SmallIcon("view-filter"));
//     action->setToolTip(i18n("Select images"));
//     action->setIcon(SmallIcon("edit-select"));

    connect(d->actionIncreaseThumbnailSize, SIGNAL(triggered(bool)),
            this, SLOT(slotIncreaseThumbnailSize()));

    connect(d->actionDecreaseThumbnailSize, SIGNAL(triggered(bool)),
            this, SLOT(slotDecreaseThumbnailSize()));

    connect(d->actionPreviewSingleItems, SIGNAL(changed()),
            this, SLOT(slotItemDisplaySettingsChanged()));

    connect(d->actionPreviewGroupedItems, SIGNAL(changed()),
            this, SLOT(slotItemDisplaySettingsChanged()));

    connect(d->actionShowNumbersOnItems, SIGNAL(changed()),
            this, SLOT(slotItemDisplaySettingsChanged()));

    connect(d->actionSetSelectionMode, SIGNAL(changed()),
            this, SLOT(slotSetSelectionMode()));

    connect(d->actionSetPanMode, SIGNAL(changed()),
            this, SLOT(slotSetPanMode()));

    connect(d->actionSetZoomMode, SIGNAL(changed()),
            this, SLOT(slotSetZoomMode()));

    connect(d->actionSetFilterMode, SIGNAL(changed()),
            this, SLOT(slotSetFilterMode()));

   // connect(d->actionRemoveFilterMode, SIGNAL(triggered()),
   //         this, SIGNAL(signalRemoveCurrentFilter()));
    connect(d->actionRemoveFilterMode, SIGNAL(triggered()),
            this, SLOT(slotRemoveCurrentFilter()));        

    connect(d->actionSetSelectThumbnailMode, SIGNAL(changed()),
            this, SLOT(slotSetSelectThumbnailMode()));

    connect(d->actionRemoveCurrentSelection, SIGNAL(triggered()),
            this, SLOT(slotRemoveCurrentSelection()));

        
}

void KMapWidget::createActionsForBackendSelection()
{
    // delete the existing actions:
    qDeleteAll(d->actionGroupBackendSelection->actions());

    // create actions for all backends:
    for (int i = 0; i<d->loadedBackends.size(); ++i)
    {
        const QString backendName = d->loadedBackends.at(i)->backendName();
        KAction* backendAction = new KAction(d->actionGroupBackendSelection);
        backendAction->setData(backendName);
        backendAction->setText(d->loadedBackends.at(i)->backendHumanName());
        backendAction->setCheckable(true);
    }
}

KMapWidget::~KMapWidget()
{
    // release all widgets:
    for (int i = 0; i<d->stackedLayout->count(); ++i)
    {
        d->stackedLayout->removeWidget(d->stackedLayout->widget(i));
    }

    qDeleteAll(d->loadedBackends);
    delete d;

    // TODO: delete s, but make sure it is not accessed by any other objects any more!
}

QStringList KMapWidget::availableBackends() const
{
    QStringList result;

    MapBackend* backend;
    foreach(backend, d->loadedBackends)
    {
        result.append(backend->backendName());
    }

    return result;
}

bool KMapWidget::setBackend(const QString& backendName)
{
    if (backendName == d->currentBackendName)
        return true;

    saveBackendToCache();

    // disconnect signals from old backend:
    if (d->currentBackend)
    {

        d->currentBackend->setActive(false);

        disconnect(d->currentBackend, SIGNAL(signalBackendReady(const QString&)),
                this, SLOT(slotBackendReady(const QString&)));

        disconnect(d->currentBackend, SIGNAL(signalZoomChanged(const QString&)),
                this, SLOT(slotBackendZoomChanged(const QString&)));

        disconnect(d->currentBackend, SIGNAL(signalClustersMoved(const QIntList&, const QPair<int, QModelIndex>&)),
                this, SLOT(slotClustersMoved(const QIntList&, const QPair<int, QModelIndex>&)));

        disconnect(d->currentBackend, SIGNAL(signalClustersClicked(const QIntList&)),
                    this, SLOT(slotClustersClicked(const QIntList&)));

        disconnect(d->currentBackend, SIGNAL(signalMarkersMoved(const QIntList&)),
                this, SLOT(slotMarkersMoved(const QIntList&)));

        disconnect(d->currentBackend, SIGNAL(signalSpecialMarkersMoved(const QList<QPersistentModelIndex>&)),
                    this, SIGNAL(signalSpecialMarkersMoved(const QList<QPersistentModelIndex>&)));

        disconnect(this, SIGNAL(signalUngroupedModelChanged(const int)),
                    d->currentBackend, SLOT(slotUngroupedModelChanged(const int)));

        if (s->markerModel)
        {
            disconnect(s->markerModel, SIGNAL(signalThumbnailAvailableForIndex(const QVariant&, const QPixmap&)),
                        d->currentBackend, SLOT(slotThumbnailAvailableForIndex(const QVariant&, const QPixmap&)));
        }

        if (d->currentBackend->backendName() == QString("marble"))
        {
            disconnect(d->currentBackend->mapWidget(), SIGNAL(regionSelected(const QList<double>&)),
                    this, SLOT(slotNewSelectionFromMap(const QList<double>&)));
        }

        disconnect(d->currentBackend, SIGNAL(signalSelectionHasBeenMade(const QList<double>&)),
                this, SLOT(slotNewSelectionFromMap(const QList<double>&)));

    }

    MapBackend* backend;
    foreach(backend, d->loadedBackends)
    {
        if (backend->backendName() == backendName)
        {
            kDebug() << QString("setting backend %1").arg(backendName);
            d->currentBackend = backend;
            d->currentBackendName = backendName;
            d->currentBackendReady = false;

            connect(d->currentBackend, SIGNAL(signalBackendReady(const QString&)),
                    this, SLOT(slotBackendReady(const QString&)));

            connect(d->currentBackend, SIGNAL(signalZoomChanged(const QString&)),
                    this, SLOT(slotBackendZoomChanged(const QString&)));

            connect(d->currentBackend, SIGNAL(signalClustersMoved(const QIntList&, const QPair<int, QModelIndex>&)),
                    this, SLOT(slotClustersMoved(const QIntList&, const QPair<int, QModelIndex>&)));

            connect(d->currentBackend, SIGNAL(signalClustersClicked(const QIntList&)),
                    this, SLOT(slotClustersClicked(const QIntList&)));

            connect(d->currentBackend, SIGNAL(signalSpecialMarkersMoved(const QList<QPersistentModelIndex>&)),
                    this, SIGNAL(signalSpecialMarkersMoved(const QList<QPersistentModelIndex>&)));

            // TODO: this connection is queued because otherwise QAbstractItemModel::itemSelected does not
            //       reflect the true state. Maybe monitor another signal instead?
            connect(this, SIGNAL(signalUngroupedModelChanged(const int)),
                    d->currentBackend, SLOT(slotUngroupedModelChanged(const int)), Qt::QueuedConnection);

            if (s->markerModel)
            {
                connect(s->markerModel, SIGNAL(signalThumbnailAvailableForIndex(const QVariant&, const QPixmap&)),
                        d->currentBackend, SLOT(slotThumbnailAvailableForIndex(const QVariant&, const QPixmap&)));
            }

            if (backendName == QString("marble"))
            {
               connect(d->currentBackend->mapWidget(), SIGNAL(regionSelected(const QList<double>&)),
                       this, SLOT(slotNewSelectionFromMap(const QList<double>&)));
            }

            connect(d->currentBackend, SIGNAL(signalSelectionHasBeenMade(const QList<double>&)),
                    this, SLOT(slotNewSelectionFromMap(const QList<double>&)));

            // call this slot manually in case the backend was ready right away:
            if (d->currentBackend->isReady())
            {
                slotBackendReady(d->currentBackendName);
            }
            else
            {
                rebuildConfigurationMenu();
            }

            d->currentBackend->setActive(d->activeState);

            return true;
        }
    }

    return false;
}

void KMapWidget::applyCacheToBackend()
{
    if (!d->currentBackendReady)
        return;

    setCenter(d->cacheCenterCoordinate);
    // TODO: only do this if the zoom was changed!
    setZoom(d->cacheZoom);
    d->currentBackend->mouseModeChanged(d->currentMouseMode);
    setSelectionCoordinates(d->cacheSelectionRectangle);
}

void KMapWidget::saveBackendToCache()
{
    if (!d->currentBackendReady)
        return;

    d->cacheCenterCoordinate   = getCenter();
    d->cacheZoom               = getZoom();
    d->cacheSelectionRectangle = getSelectionRectangle();
    if(!d->cacheSelectionRectangle.isEmpty())
        d->oldSelectionRectangle   = d->cacheSelectionRectangle;
}

GeoCoordinates KMapWidget::getCenter() const
{
    if (!d->currentBackendReady)
        return GeoCoordinates();

    return d->currentBackend->getCenter();
}

void KMapWidget::setCenter(const GeoCoordinates& coordinate)
{
    d->cacheCenterCoordinate = coordinate;

    if (!d->currentBackendReady)
        return;

    d->currentBackend->setCenter(coordinate);
}

void KMapWidget::slotBackendReady(const QString& backendName)
{
    kDebug()<<QString("backend %1 is ready!").arg(backendName);
    if (backendName != d->currentBackendName)
        return;

    d->currentBackendReady = true;

    QWidget* const currentMapWidget = d->currentBackend->mapWidget();
    bool foundWidget = false;
    for (int i = 0; i<d->stackedLayout->count(); ++i)
    {
        if (d->stackedLayout->widget(i) == currentMapWidget)
        {
            d->stackedLayout->setCurrentIndex(i);
            foundWidget = true;
        }
    }
    if (!foundWidget)
    {
        const int newIndex = d->stackedLayout->addWidget(currentMapWidget);
        d->stackedLayout->setCurrentIndex(newIndex);
    }


    if(!d->thumbnailsHaveBeenLoaded)
    {
        d->thumbnailTimer      = new QTimer(this);
        d->thumbnailTimerCount = 0;
        connect(d->thumbnailTimer, SIGNAL(timeout()), this, SLOT(stopThumbnailTimer()));
        d->thumbnailTimer->start(2000);
    }

    applyCacheToBackend();

    updateMarkers();
    markClustersAsDirty();

    rebuildConfigurationMenu();
}

void KMapWidget::stopThumbnailTimer()
{
    d->currentBackend->updateMarkers();
    d->thumbnailTimerCount++;
    if(d->thumbnailTimerCount == 10)
    {
        d->thumbnailTimer->stop();
        d->thumbnailsHaveBeenLoaded = true;
    }
}

void KMapWidget::saveSettingsToGroup(KConfigGroup* const group)
{
    KMAP_ASSERT(group != 0);
    if (!group)
        return;

    if (!d->currentBackendName.isEmpty())
    {
        group->writeEntry("Backend", d->currentBackendName);
    }
    group->writeEntry("Center", getCenter().geoUrl());
    group->writeEntry("Zoom", getZoom());
    group->writeEntry("Preview Single Items", s->previewSingleItems);
    group->writeEntry("Preview Grouped Items", s->previewGroupedItems);
    group->writeEntry("Show numbers on items", s->showNumbersOnItems);
    group->writeEntry("Thumbnail Size", d->thumbnailSize);
    group->writeEntry("Grouping Radius", d->groupingRadius);
    group->writeEntry("Edit Grouping Radius", d->editGroupingRadius);
    group->writeEntry("In Edit Mode", s->inEditMode);

    for (int i=0; i<d->loadedBackends.size(); ++i)
    {
        d->loadedBackends.at(i)->saveSettingsToGroup(group);
    }
}

void KMapWidget::readSettingsFromGroup(const KConfigGroup* const group)
{
    KMAP_ASSERT(group != 0);
    if (!group)
        return;

    setBackend(group->readEntry("Backend", "marble"));

    const GeoCoordinates centerDefault = GeoCoordinates(52.0, 6.0);
    const QString centerGeoUrl = group->readEntry("Center", centerDefault.geoUrl());
    bool centerGeoUrlValid = false;
    const GeoCoordinates centerCoordinate = GeoCoordinates::fromGeoUrl(centerGeoUrl, &centerGeoUrlValid);
    setCenter(centerGeoUrlValid ? centerCoordinate : centerDefault);
    setZoom(group->readEntry("Zoom", d->cacheZoom));

    d->actionPreviewSingleItems->setChecked(group->readEntry("Preview Single Items", true));
    d->actionPreviewGroupedItems->setChecked(group->readEntry("Preview Grouped Items", true));
    d->actionShowNumbersOnItems->setChecked(group->readEntry("Show numbers on items", true));

    setThumnailSize(group->readEntry("Thumbnail Size", 2*KMapMinThumbnailSize));
    setGroupingRadius(group->readEntry("Grouping Radius", 2*KMapMinGroupingRadius));
    setEditGroupingRadius(group->readEntry("Edit Grouping Radius", KMapMinEditGroupingRadius));
    s->inEditMode = group->readEntry("In Edit Mode", false);
    if (s->inEditMode)
    {
        d->actionEditMode->setChecked(true);
    }
    else
    {
        d->actionBrowseMode->setChecked(true);
    }

    for (int i=0; i<d->loadedBackends.size(); ++i)
    {
        d->loadedBackends.at(i)->readSettingsFromGroup(group);
    }

    slotUpdateActionsEnabled();
}

void KMapWidget::rebuildConfigurationMenu()
{
    d->configurationMenu->clear();

    const QList<QAction*> backendSelectionActions = d->actionGroupBackendSelection->actions();
    for (int i=0; i<backendSelectionActions.count(); ++i)
    {
        QAction* const backendAction = backendSelectionActions.at(i);

        if (backendAction->data().toString()==d->currentBackendName)
        {
            backendAction->setChecked(true);
        }

        d->configurationMenu->addAction(backendAction);
    }

    if (d->currentBackendReady)
    {
        d->currentBackend->addActionsToConfigurationMenu(d->configurationMenu);
    }

    if (!s->inEditMode)
    {
        d->configurationMenu->addSeparator();

        if (d->sortMenu)
        {
            d->configurationMenu->addMenu(d->sortMenu);
        }

        d->configurationMenu->addAction(d->actionPreviewSingleItems);
        d->configurationMenu->addAction(d->actionPreviewGroupedItems);
        d->configurationMenu->addAction(d->actionShowNumbersOnItems);
    }
}

KAction* KMapWidget::getControlAction(const QString& actionName)
{
    kDebug()<<actionName;
    if (actionName=="zoomin")
    {
        return d->actionZoomIn;
    }
    else if (actionName=="zoomout")
    {
        return d->actionZoomOut;
    }

    return 0;
}

/**
 * @brief Returns the control widget.
 */
QWidget* KMapWidget::getControlWidget()
{
    if (!d->controlWidget)
    {
        d->controlWidget = new KHBox(this);

        QToolButton* const configurationButton = new QToolButton(d->controlWidget);
        configurationButton->setToolTip(i18n("Map settings"));
        configurationButton->setIcon(SmallIcon("applications-internet"));
        configurationButton->setMenu(d->configurationMenu);
        configurationButton->setPopupMode(QToolButton::InstantPopup);

        QToolButton* const zoomInButton = new QToolButton(d->controlWidget);
        zoomInButton->setDefaultAction(d->actionZoomIn);

        QToolButton* const zoomOutButton = new QToolButton(d->controlWidget);
        zoomOutButton->setDefaultAction(d->actionZoomOut);

        // browse mode controls:
        d->browseModeControlsHolder = new KHBox(d->controlWidget);
        d->browseModeControlsHolder->setVisible(d->editModeAvailable);

        new KSeparator(Qt::Vertical, d->browseModeControlsHolder);

        QToolButton* const browseModeButton = new QToolButton(d->browseModeControlsHolder);
        browseModeButton->setDefaultAction(d->actionBrowseMode);

        QToolButton* const editModeButton = new QToolButton(d->browseModeControlsHolder);
        editModeButton->setDefaultAction(d->actionEditMode);

        new KSeparator(Qt::Vertical, d->controlWidget);

        QToolButton* const increaseThumbnailSizeButton = new QToolButton(d->controlWidget);
        increaseThumbnailSizeButton->setDefaultAction(d->actionIncreaseThumbnailSize);

        QToolButton* const decreaseThumbnailSizeButton = new QToolButton(d->controlWidget);
        decreaseThumbnailSizeButton->setDefaultAction(d->actionDecreaseThumbnailSize);

        /* --- --- --- */

        d->mouseModesHolder = new KHBox(d->controlWidget);

        new KSeparator(Qt::Vertical, d->mouseModesHolder);

        d->setPanModeButton = new QToolButton(d->mouseModesHolder);
        d->setPanModeButton->setDefaultAction(d->actionSetPanMode);

        d->setSelectionModeButton = new QToolButton(d->mouseModesHolder);
        d->setSelectionModeButton->setDefaultAction(d->actionSetSelectionMode);

        d->removeCurrentSelectionButton = new QToolButton(d->mouseModesHolder);
        d->removeCurrentSelectionButton->setDefaultAction(d->actionRemoveCurrentSelection);

        d->setZoomModeButton = new QToolButton(d->mouseModesHolder);
        d->setZoomModeButton->setDefaultAction(d->actionSetZoomMode);

        d->setFilterModeButton = new QToolButton(d->mouseModesHolder);
        d->setFilterModeButton->setDefaultAction(d->actionSetFilterMode);

        d->removeFilterModeButton = new QToolButton(d->mouseModesHolder);
        d->removeFilterModeButton->setDefaultAction(d->actionRemoveFilterMode);

        d->setSelectThumbnailMode = new QToolButton(d->mouseModesHolder);
        d->setSelectThumbnailMode->setDefaultAction(d->actionSetSelectThumbnailMode);

        d->hBoxForAdditionalControlWidgetItems = new KHBox(d->controlWidget);

        setVisibleMouseModes(d->visibleMouseModes);

        // add stretch after the controls:
        QHBoxLayout* const hBoxLayout = reinterpret_cast<QHBoxLayout*>(d->controlWidget->layout());
        if (hBoxLayout)
        {
            hBoxLayout->addStretch();
        }
    }

    // make sure the menu exists, even if no backend has been set:
    rebuildConfigurationMenu();

    return d->controlWidget;
}

void KMapWidget::slotZoomIn()
{
    if (!d->currentBackendReady)
        return;

    d->currentBackend->zoomIn();
}

void KMapWidget::slotZoomOut()
{
    if (!d->currentBackendReady)
        return;

    d->currentBackend->zoomOut();
}

void KMapWidget::slotUpdateActionsEnabled()
{
    d->actionDecreaseThumbnailSize->setEnabled((!s->inEditMode)&&(d->thumbnailSize>KMapMinThumbnailSize));
    // TODO: define an upper limit!
    d->actionIncreaseThumbnailSize->setEnabled(!s->inEditMode);

    d->actionSetSelectionMode->setEnabled(d->availableMouseModes.testFlag(MouseModeSelection));
    d->actionRemoveCurrentSelection->setEnabled(d->availableMouseModes.testFlag(MouseModeSelection));
    d->actionSetPanMode->setEnabled(d->availableMouseModes.testFlag(MouseModePan));
    d->actionSetZoomMode->setEnabled(d->availableMouseModes.testFlag(MouseModeZoom));
    d->actionSetFilterMode->setEnabled(d->availableMouseModes.testFlag(MouseModeFilter));
    d->actionRemoveFilterMode->setEnabled(d->availableMouseModes.testFlag(MouseModeFilter));
    d->actionSetSelectThumbnailMode->setEnabled(d->availableMouseModes.testFlag(MouseModeSelectThumbnail));
}

void KMapWidget::slotChangeBackend(QAction* action)
{
    KMAP_ASSERT(action!=0);

    if (!action)
        return;

    const QString newBackendName = action->data().toString();
    setBackend(newBackendName);
}

void KMapWidget::updateMarkers()
{
    if (!d->currentBackendReady)
        return;

    // tell the backend to update the markers
    d->currentBackend->updateMarkers();
}

void KMapWidget::updateClusters()
{
    if (!s->markerModel)
        return;

    if (s->haveMovingCluster)
    {
        // do not re-cluster while a cluster is being moved
        return;
    }

    if (!d->clustersDirty)
       return;

    d->clustersDirty = false;

    // constants for clusters
    const int ClusterRadius          = s->inEditMode ? d->editGroupingRadius : d->groupingRadius;
    const QSize ClusterDefaultSize   = QSize(2*ClusterRadius, 2*ClusterRadius);
    const int ClusterGridSizeScreen  = 4*ClusterRadius;
    const QSize ClusterMaxPixmapSize = QSize(ClusterGridSizeScreen, ClusterGridSizeScreen);

//     kDebug()<<"updateClusters starting...";

    s->clusterList.clear();

    if (!d->currentBackendReady)
        return;

    const int markerLevel = d->currentBackend->getMarkerModelLevel();
    QList<QPair<GeoCoordinates, GeoCoordinates> > mapBounds = d->currentBackend->getNormalizedBounds();

//     // debug output for tile level diagnostics:
//     QIntList tile1;
//     tile1<<520;
//     QIntList tile2 = tile1;
//     for (int i=1; i<=s->markerModel->maxLevel()-1; ++i)
//     {
//         tile2 = tile1;
//         tile2<<0;
//         tile1<<1;
//         const GeoCoordinates tile1Coordinate = s->markerModel->tileIndexToCoordinate(tile1);
//         const GeoCoordinates tile2Coordinate = s->markerModel->tileIndexToCoordinate(tile2);
//         QPoint tile1Point, tile2Point;
//         d->currentBackend->screenCoordinates(tile1Coordinate, &tile1Point);
//         d->currentBackend->screenCoordinates(tile2Coordinate, &tile2Point);
//         kDebug()<<i<<tile1Point<<tile2Point<<(tile1Point-tile2Point);
//     }

    const int gridSize = ClusterGridSizeScreen;
    const QSize mapSize  = d->currentBackend->mapSize();
    const int gridWidth  = mapSize.width();
    const int gridHeight = mapSize.height();
    QVector<QList<AbstractMarkerTiler::TileIndex> > pixelNonEmptyTileIndexGrid(gridWidth*gridHeight, QList<AbstractMarkerTiler::TileIndex>());
    QVector<int> pixelCountGrid(gridWidth*gridHeight, 0);
    QList<QPair<QPoint, QPair<int, QList<AbstractMarkerTiler::TileIndex> > > > leftOverList;

    // TODO: iterate only over the visible part of the map
    int debugCountNonEmptyTiles = 0;
    int debugTilesSearched = 0;
  
    //TODO: this is good here? 
    for(int i = 0; i < mapBounds.count(); ++i)
        s->markerModel->prepareTiles(mapBounds.at(i).first, mapBounds.at(i).second, markerLevel);
 
    for (AbstractMarkerTiler::NonEmptyIterator tileIterator(s->markerModel, markerLevel, mapBounds); !tileIterator.atEnd(); tileIterator.nextIndex())
    {
        const AbstractMarkerTiler::TileIndex tileIndex = tileIterator.currentIndex();

        // find out where the tile is on the map:
        const GeoCoordinates tileCoordinate = tileIndex.toCoordinates();
        debugTilesSearched++;
        QPoint tilePoint;
        if (!d->currentBackend->screenCoordinates(tileCoordinate, &tilePoint))
        {
            continue;
        }

        // make sure we are in the grid (in case there are rounding errors somewhere in the backend
        if ((tilePoint.x()<0)||(tilePoint.y()<0)||(tilePoint.x()>=gridWidth)||(tilePoint.y()>=gridHeight))
            continue;

        debugCountNonEmptyTiles++;
        const int linearIndex = tilePoint.x() + tilePoint.y()*gridWidth;
        pixelNonEmptyTileIndexGrid[linearIndex] << tileIndex;
        pixelCountGrid[linearIndex]+= s->markerModel->getTileMarkerCount(tileIndex);

//         kDebug()<<QString("pixel at: %1, %2 (%3): %4 markers").arg(tilePoint.x()).arg(tilePoint.y()).arg(linearIndex).arg(pixelCountGrid[linearIndex]);
    }

    // TODO: cleanup this list every ... iterations in the next loop, too
    QIntList nonEmptyPixelIndices;

    for (int i=0; i<gridWidth*gridHeight; ++i)
    {
        if (pixelCountGrid.at(i)>0)
            nonEmptyPixelIndices << i;
    }

    // re-add the markers to clusters:
    int lastTooCloseClusterIndex = 0;
    Q_FOREVER
    {
        // here we store candidates for clusters:
        int markerMax = 0;
        int markerX = 0;
        int markerY = 0;
        int pixelGridMetaIndexMax = 0;

        for (int pixelGridMetaIndex = 0; pixelGridMetaIndex<nonEmptyPixelIndices.size(); ++pixelGridMetaIndex)
        {
            const int index = nonEmptyPixelIndices.at(pixelGridMetaIndex);
            if (index<0)
                continue;

            if (pixelCountGrid.at(index)==0)
            {
                // TODO: also remove this entry from the list to speed up the loop!
                nonEmptyPixelIndices[pixelGridMetaIndex] = -1;
                continue;
            }

            if (pixelCountGrid.at(index)>markerMax)
            {
                // calculate x,y from the linear index:
                const int x = index % gridWidth;
                const int y = (index-x)/gridWidth;
                const QPoint markerPosition(x, y);

                // only use this as a candidate for a cluster if it is not too close to another cluster:
                bool tooClose = false;

                // TODO: check the cluster that was a problem last time first:
//                 if (lastTooCloseClusterIndex<s->clusterList.size())
//                 {
//                     tooClose = QPointSquareDistance(s->clusterList.at(lastTooCloseClusterIndex).pixelPos, markerPosition) < pow(ClusterGridSizeScreen/2, 2);
//                 }

                // now check all other clusters:
                for (int i=0; (!tooClose)&&(i<s->clusterList.size()); ++i)
                {
                    if (i==index)
                        continue;

                    tooClose = QPointSquareDistance(s->clusterList.at(i).pixelPos, markerPosition) < pow(ClusterGridSizeScreen/2, 2);
                    if (tooClose)
                        lastTooCloseClusterIndex = i;
                }

                if (tooClose)
                {
                    // move markers into leftover list
                    leftOverList << QPair<QPoint, QPair<int, QList<AbstractMarkerTiler::TileIndex> > >(QPoint(x,y), QPair<int, QList<AbstractMarkerTiler::TileIndex> >(pixelCountGrid.at(index), pixelNonEmptyTileIndexGrid.at(index)));
                    pixelCountGrid[index] = 0;
                    pixelNonEmptyTileIndexGrid[index].clear();
                    nonEmptyPixelIndices[pixelGridMetaIndex] = -1;
                }
                else
                {
                    markerMax=pixelCountGrid.at(index);
                    markerX=x;
                    markerY=y;
                    pixelGridMetaIndexMax = pixelGridMetaIndex;
                }
            }
        }

        if (markerMax==0)
            break;

        GeoCoordinates clusterCoordinates = pixelNonEmptyTileIndexGrid.at(markerX+markerY*gridWidth).first().toCoordinates();
        WMWCluster cluster;
        cluster.coordinates = clusterCoordinates;
        cluster.pixelPos = QPoint(markerX, markerY);
        cluster.tileIndicesList = AbstractMarkerTiler::TileIndex::listToIntListList(pixelNonEmptyTileIndexGrid.at(markerX+markerY*gridWidth));
        cluster.markerCount = pixelCountGrid.at(markerX+markerY*gridWidth);

        // mark the pixel as done:
        pixelCountGrid[markerX+markerY*gridWidth] = 0;
        pixelNonEmptyTileIndexGrid[markerX+markerY*gridWidth].clear();
        nonEmptyPixelIndices[pixelGridMetaIndexMax] = -1;

        // absorb all markers around it:
        // Now we only remove the markers from the pixelgrid. They will be cleared from the
        // pixelGridIndices in the loop above
        // make sure we do not go over the grid boundaries:
        const int eatRadius = gridSize/4;
        const int xStart    = qMax( (markerX-eatRadius), 0);
        const int yStart    = qMax( (markerY-eatRadius), 0);
        const int xEnd      = qMin( (markerX+eatRadius), gridWidth-1);
        const int yEnd      = qMin( (markerY+eatRadius), gridHeight-1);
        for (int indexX = xStart; indexX <= xEnd; ++indexX)
        {
            for (int indexY = yStart; indexY <= yEnd; ++indexY)
            {
                const int index = indexX + indexY*gridWidth;
                cluster.tileIndicesList << AbstractMarkerTiler::TileIndex::listToIntListList(pixelNonEmptyTileIndexGrid.at(index));
                pixelNonEmptyTileIndexGrid[index].clear();
                cluster.markerCount+= pixelCountGrid.at(index);
                pixelCountGrid[index] = 0;
            }
        }

        kDebug()<<QString("created cluster %1: %2 tiles").arg(s->clusterList.size()).arg(cluster.tileIndicesList.count());

        s->clusterList << cluster;
    }

    // now move all leftover markers into clusters:
    for (QList<QPair<QPoint, QPair<int, QList<AbstractMarkerTiler::TileIndex> > > >::const_iterator it = leftOverList.constBegin();
         it!=leftOverList.constEnd(); ++it)
    {
        const QPoint markerPosition = it->first;

        // find the closest cluster:
        int closestSquareDistance = 0;
        int closestIndex = -1;
        for (int i=0; i<s->clusterList.size(); ++i)
        {
            const int squareDistance = QPointSquareDistance(s->clusterList.at(i).pixelPos, markerPosition);
            if ((closestIndex < 0) || (squareDistance < closestSquareDistance))
            {
                closestSquareDistance = squareDistance;
                closestIndex          = i;
            }
        }

        if (closestIndex>=0)
        {
            s->clusterList[closestIndex].markerCount+= it->second.first;
            s->clusterList[closestIndex].tileIndicesList << AbstractMarkerTiler::TileIndex::listToIntListList(it->second.second);
        }
    }

    // determine the selected states of the clusters:
    for (int i=0; i<s->clusterList.count(); ++i)
    {
        WMWCluster& cluster = s->clusterList[i];

        int clusterSelectedCount = 0;
        for (int iTile=0;
                (iTile<cluster.tileIndicesList.count());
                ++iTile)
        {
            clusterSelectedCount+= s->markerModel->getTileSelectedCount(AbstractMarkerTiler::TileIndex::fromIntList(cluster.tileIndicesList.at(iTile)));
        }
        cluster.markerSelectedCount = clusterSelectedCount;
        if (cluster.markerSelectedCount==0)
        {
            cluster.selectedState = WMWSelectedNone;
        }
        else if (cluster.markerSelectedCount==cluster.markerCount)
        {
            cluster.selectedState = WMWSelectedAll;
        }
        else
        {
            cluster.selectedState = WMWSelectedSome;
        }
    }

//     kDebug()<<s->clusterList.size();
    kDebug()<<QString("level %1: %2 non empty tiles sorted into %3 clusters (%4 searched)").arg(markerLevel).arg(debugCountNonEmptyTiles).arg(s->clusterList.count()).arg(debugTilesSearched);

    d->currentBackend->updateClusters();
}

void KMapWidget::slotClustersNeedUpdating()
{
    if (d->currentBackendReady)
    {
        d->currentBackend->slotClustersNeedUpdating();
    }
}

/**
 * @brief Return color and style information for rendering the cluster
 * @param clusterIndex Index of the cluster
 * @param fillColor Color used to fill the circle
 * @param strokeColor Color used for the stroke around the circle
 * @param strokeStyle Style used to draw the stroke around the circle
 * @param labelText Text for the label
 * @param labelColor Color for the label text
 * @param overrideSelection Get the colors for a different selection state
 * @param overrideCount Get the colors for a different amount of markers
 */
void KMapWidget::getColorInfos(const int clusterIndex, QColor *fillColor, QColor *strokeColor,
                                    Qt::PenStyle *strokeStyle, QString *labelText, QColor *labelColor,
                                    const WMWSelectionState* const overrideSelection,
                                    const int* const overrideCount) const
{
    // TODO: call the new getColorInfos function!
    const WMWCluster& cluster = s->clusterList.at(clusterIndex);

    // TODO: check that this number is already valid!
    const int nMarkers = overrideCount ? *overrideCount : cluster.markerCount;

    getColorInfos(overrideSelection?*overrideSelection:cluster.selectedState,
                  nMarkers,
                  fillColor, strokeColor, strokeStyle, labelText, labelColor);
}

void KMapWidget::getColorInfos(const WMWSelectionState selectionState,
                       const int nMarkers,
                       QColor *fillColor, QColor *strokeColor,
                       Qt::PenStyle *strokeStyle, QString *labelText, QColor *labelColor) const
{
    if (nMarkers<1000)
    {
        *labelText = QString::number(nMarkers);
    }
    else if ((nMarkers>=1000)&&(nMarkers<=1950))
    {
        // TODO: use KDE-versions instead
        *labelText = QString("%L1k").arg(qreal(nMarkers)/1000.0, 0, 'f', 1);
    }
    else if ((nMarkers>=1951)&&(nMarkers<19500))
    {
        // TODO: use KDE-versions instead
        *labelText = QString("%L1k").arg(qreal(nMarkers)/1000.0, 0, 'f', 0);
    }
    else
    {
        // convert to "1E5" notation for numbers >=20k:
        qreal exponent = floor(log(nMarkers)/log(10));
        qreal nMarkersFirstDigit=round(qreal(nMarkers)/pow(10,exponent));
        if (nMarkersFirstDigit>=10)
        {
            nMarkersFirstDigit=round(nMarkersFirstDigit/10.0);
            exponent++;
        }
        *labelText = QString("%1E%2").arg(int(nMarkersFirstDigit)).arg(int(exponent));
    }
    *labelColor = QColor(Qt::black);

    // TODO: 'solo' and 'selected' properties have not yet been defined,
    //       therefore use the default colors
    *strokeStyle = Qt::NoPen;
    switch (selectionState)
    {
        case WMWSelectedNone:
            *strokeStyle = Qt::SolidLine;
            *strokeColor = QColor(Qt::black);
            break;
        case WMWSelectedSome:
            *strokeStyle = Qt::DotLine;
            *strokeColor = QColor(Qt::blue);
            break;
        case WMWSelectedAll:
            *strokeStyle = Qt::SolidLine;
            *strokeColor = QColor(Qt::blue);
            break;
    }

    QColor fillAll, fillSome, fillNone;
    if (nMarkers>=100)
    {
        fillAll  = QColor(255, 0, 0);
        fillSome = QColor(255, 188, 125);
        fillNone = QColor(255, 185, 185);
    }
    else if (nMarkers>=50)
    {
        fillAll  = QColor(255, 127, 0);
        fillSome = QColor(255, 190, 125);
        fillNone = QColor(255, 220, 185);
    }
    else if (nMarkers>=10)
    {
        fillAll  = QColor(255, 255, 0);
        fillSome = QColor(255, 255, 105);
        fillNone = QColor(255, 255, 185);
    }
    else if (nMarkers>=2)
    {
        fillAll  = QColor(0, 255, 0);
        fillSome = QColor(125, 255, 125);
        fillNone = QColor(185, 255, 255);
    }
    else
    {
        fillAll  = QColor(0, 255, 255);
        fillSome = QColor(125, 255, 255);
        fillNone = QColor(185, 255, 255);
    }

    *fillColor = fillAll;
//     switch (solo)
//     {
//         case PartialAll:
//             *fillColor = fillAll;
//             break;
//         case PartialSome:
//             *fillColor = fillSome;
//             break;
//         case PartialNone:
//             if (haveAnySolo)
//             {
//                 *fillColor = fillNone;
//             }
//             else
//             {
//                 *fillColor = fillAll;
//             }
//             break;
//     }
}


QString KMapWidget::convertZoomToBackendZoom(const QString& someZoom, const QString& targetBackend) const
{
    const QStringList zoomParts = someZoom.split(':');
    KMAP_ASSERT(zoomParts.count()==2);
    const QString sourceBackend = zoomParts.first();

    if (sourceBackend==targetBackend)
    {
        return someZoom;
    }

    const int sourceZoom = zoomParts.last().toInt();

    int targetZoom = -1;

    // all of these values were found experimentally!
    if (targetBackend=="marble")
    {
             if (sourceZoom== 0) { targetZoom =  900; }
        else if (sourceZoom== 1) { targetZoom =  970; }
        else if (sourceZoom== 2) { targetZoom = 1108; }
        else if (sourceZoom== 3) { targetZoom = 1250; }
        else if (sourceZoom== 4) { targetZoom = 1384; }
        else if (sourceZoom== 5) { targetZoom = 1520; }
        else if (sourceZoom== 6) { targetZoom = 1665; }
        else if (sourceZoom== 7) { targetZoom = 1800; }
        else if (sourceZoom== 8) { targetZoom = 1940; }
        else if (sourceZoom== 9) { targetZoom = 2070; }
        else if (sourceZoom==10) { targetZoom = 2220; }
        else if (sourceZoom==11) { targetZoom = 2357; }
        else if (sourceZoom==12) { targetZoom = 2510; }
        else if (sourceZoom==13) { targetZoom = 2635; }
        else if (sourceZoom==14) { targetZoom = 2775; }
        else if (sourceZoom==15) { targetZoom = 2900; }
        else if (sourceZoom==16) { targetZoom = 3051; }
        else if (sourceZoom==17) { targetZoom = 3180; }
        else if (sourceZoom==18) { targetZoom = 3295; }
        else if (sourceZoom==19) { targetZoom = 3450; }
        else { targetZoom = 3500; } // TODO: find values for level 20 and up
    }

    if (targetBackend=="googlemaps")
    {
             if (sourceZoom<= 900) { targetZoom =  0; }
        else if (sourceZoom<= 970) { targetZoom =  1; }
        else if (sourceZoom<=1108) { targetZoom =  2; }
        else if (sourceZoom<=1250) { targetZoom =  3; }
        else if (sourceZoom<=1384) { targetZoom =  4; }
        else if (sourceZoom<=1520) { targetZoom =  5; }
        else if (sourceZoom<=1665) { targetZoom =  6; }
        else if (sourceZoom<=1800) { targetZoom =  7; }
        else if (sourceZoom<=1940) { targetZoom =  8; }
        else if (sourceZoom<=2070) { targetZoom =  9; }
        else if (sourceZoom<=2220) { targetZoom = 10; }
        else if (sourceZoom<=2357) { targetZoom = 11; }
        else if (sourceZoom<=2510) { targetZoom = 12; }
        else if (sourceZoom<=2635) { targetZoom = 13; }
        else if (sourceZoom<=2775) { targetZoom = 14; }
        else if (sourceZoom<=2900) { targetZoom = 15; }
        else if (sourceZoom<=3051) { targetZoom = 16; }
        else if (sourceZoom<=3180) { targetZoom = 17; }
        else if (sourceZoom<=3295) { targetZoom = 18; }
        else if (sourceZoom<=3450) { targetZoom = 19; }
        else { targetZoom = 20; } // TODO: find values for level 20 and up
    }

    KMAP_ASSERT(targetZoom>=0);

    return QString("%1:%2").arg(targetBackend).arg(targetZoom);
}

void KMapWidget::slotBackendZoomChanged(const QString& newZoom)
{
    kDebug()<<newZoom;
    d->cacheZoom = newZoom;
}

void KMapWidget::setZoom(const QString& newZoom)
{
    d->cacheZoom = newZoom;

    if (d->currentBackendReady)
    {
        d->currentBackend->setZoom(d->cacheZoom);
    }
}

QString KMapWidget::getZoom()
{
    if (d->currentBackendReady)
    {
        d->cacheZoom = d->currentBackend->getZoom();
    }

    return d->cacheZoom;
}


QList<qreal> KMapWidget::getSelectionRectangle()
{
    if(d->currentBackendReady)
    {
        d->cacheSelectionRectangle = d->currentBackend->getSelectionRectangle();
    }

    return d->cacheSelectionRectangle;
}

void KMapWidget::slotClustersMoved(const QIntList& clusterIndices, const QPair<int, QModelIndex>& snapTarget)
{
    kDebug()<<clusterIndices;

    // TODO: we actually expect only one clusterindex
    int clusterIndex = clusterIndices.first();
    GeoCoordinates targetCoordinates = s->clusterList.at(clusterIndex).coordinates;

    AbstractMarkerTiler::TileIndex::List movedTileIndices;
    if (s->clusterList.at(clusterIndex).selectedState==WMWSelectedNone)
    {
        // a not-selected marker was moved. update all of its items:
        const WMWCluster& cluster = s->clusterList.at(clusterIndex);
        for (int i=0; i<cluster.tileIndicesList.count(); ++i)
        {
            const AbstractMarkerTiler::TileIndex tileIndex = AbstractMarkerTiler::TileIndex::fromIntList(cluster.tileIndicesList.at(i));

            movedTileIndices <<tileIndex;
        }
    }
    else
    {
        // selected items were moved. The model helper should know which tiles are selected,
        // therefore we give him an empty list
    }

    s->markerModel->onIndicesMoved(movedTileIndices, targetCoordinates, snapTarget.second);

    // TODO: clusters are marked as dirty by slotClustersNeedUpdating which is called
    // while we update the model
}

bool KMapWidget::queryAltitudes(const WMWAltitudeLookup::List& queryItems, const QString& backendName)
{
    for (int i=0; i<d->loadedAltitudeBackends.count(); ++i)
    {
        AltitudeBackend* const altitudeBackend = d->loadedAltitudeBackends.at(i);

        if (altitudeBackend->backendName() == backendName)
        {
            return altitudeBackend->queryAltitudes(queryItems);
        }
    }
    return false;
}

void KMapWidget::addUngroupedModel(ModelHelper* const modelHelper)
{
    s->ungroupedModels << modelHelper;

    // TODO: monitor all model signals!
    connect(modelHelper->model(), SIGNAL(dataChanged(const QModelIndex&, const QModelIndex&)),
            this, SLOT(slotUngroupedModelChanged()));
    connect(modelHelper->model(), SIGNAL(rowsInserted(const QModelIndex&, int, int)),
            this, SLOT(slotUngroupedModelChanged()));
    connect(modelHelper->model(), SIGNAL(modelReset()),
            this, SLOT(slotUngroupedModelChanged()));
    connect(modelHelper, SIGNAL(signalVisibilityChanged()),
            this, SLOT(slotUngroupedModelChanged()));

    if (modelHelper->selectionModel())
    {
        connect(modelHelper->selectionModel(), SIGNAL(currentChanged(const QModelIndex&, const QModelIndex&)),
            this, SLOT(slotUngroupedModelChanged()));
    }

    emit(signalUngroupedModelChanged(s->ungroupedModels.count() - 1));
}

void KMapWidget::setGroupedModel(AbstractMarkerTiler* const markerModel)
{
    s->markerModel = markerModel;

    if (s->markerModel)
    {
        s->markerModel->setActive(d->activeState);

        // TODO: this needs some buffering for the google maps backend
        connect(s->markerModel, SIGNAL(signalTilesOrSelectionChanged()),
            this, SLOT(slotRequestLazyReclustering()));

        if (d->currentBackend)
        {
            connect(s->markerModel, SIGNAL(signalThumbnailAvailableForIndex(const QVariant&, const QPixmap&)),
                d->currentBackend, SLOT(slotThumbnailAvailableForIndex(const QVariant&, const QPixmap&)));
        }
    }
 
    slotRequestLazyReclustering();
}

void KMapWidget::slotGroupModeChanged(QAction* triggeredAction)
{
    Q_UNUSED(triggeredAction);
    s->inEditMode = d->actionEditMode->isChecked();

    slotUpdateActionsEnabled();
    slotRequestLazyReclustering();
}

/**
 * @brief Request reclustering, repeated calls should generate only one actual update of the clusters
 */
void KMapWidget::slotRequestLazyReclustering()
{
    if (d->lazyReclusteringRequested)
        return;

    d->clustersDirty = true;
    if(d->activeState)
    {
        d->lazyReclusteringRequested = true;
        QTimer::singleShot(0, this, SLOT(slotLazyReclusteringRequestCallBack()));
    }
}

/**
 * @brief Helper function to buffer reclustering
 */
void KMapWidget::slotLazyReclusteringRequestCallBack()
{
    if (!d->lazyReclusteringRequested)
        return;

    d->lazyReclusteringRequested = false;
    slotClustersNeedUpdating();
}

void KMapWidget::slotClustersClicked(const QIntList& clusterIndices)
{
    kDebug()<<clusterIndices;

    int maxTileLevel = 0;

    if ((d->currentMouseMode == MouseModeZoom) || (d->currentMouseMode == MouseModeFilter && d->selectionRectangle.isEmpty()))
    {
        Marble::GeoDataLineString tileString;

        for (int i=0; i<clusterIndices.count(); ++i)
        {
            const int clusterIndex = clusterIndices.at(i);
            const WMWCluster currentCluster = s->clusterList.at(clusterIndex);

            for (int j=0; j<currentCluster.tileIndicesList.count(); ++j)
            {
                const AbstractMarkerTiler::TileIndex& currentTileIndex = 
                    AbstractMarkerTiler::TileIndex::fromIntList(currentCluster.tileIndicesList.at(j));
                for(int corner=1; corner<=4; corner++)
                {
                    GeoCoordinates currentTileCoordinate = currentTileIndex.toCoordinates();

                    const Marble::GeoDataCoordinates tileCoordinate(currentTileCoordinate.lon(), 
                                                                    currentTileCoordinate.lat(),
                                                                                              0, 
                                                                    Marble::GeoDataCoordinates::Degree);

                    if(maxTileLevel < currentTileIndex.level())
                        maxTileLevel = currentTileIndex.level();                        

                    tileString.append(tileCoordinate);
                }
            }
        }

        Marble::GeoDataLatLonBox latLonBox = Marble::GeoDataLatLonBox::fromLineString(tileString);
      
/*        if(maxTileLevel != 0)
        {
            //increase the selection boundaries with 0.1 degrees because some thumbnails aren't catched by selection
            latLonBox.setWest((latLonBox.west(Marble::GeoDataCoordinates::Degree)-(0.1/maxTileLevel)), Marble::GeoDataCoordinates::Degree);
            latLonBox.setNorth((latLonBox.north(Marble::GeoDataCoordinates::Degree)+(0.1/maxTileLevel)), Marble::GeoDataCoordinates::Degree);
            latLonBox.setEast((latLonBox.east(Marble::GeoDataCoordinates::Degree)+(0.1/maxTileLevel)), Marble::GeoDataCoordinates::Degree);
            latLonBox.setSouth((latLonBox.south(Marble::GeoDataCoordinates::Degree)-(0.1/maxTileLevel)), Marble::GeoDataCoordinates::Degree);
        }
        else
        {*/
            latLonBox.setWest((latLonBox.west(Marble::GeoDataCoordinates::Degree)-0.0001), Marble::GeoDataCoordinates::Degree);
            latLonBox.setNorth((latLonBox.north(Marble::GeoDataCoordinates::Degree)+0.0001), Marble::GeoDataCoordinates::Degree);
            latLonBox.setEast((latLonBox.east(Marble::GeoDataCoordinates::Degree)+0.0001), Marble::GeoDataCoordinates::Degree);
            latLonBox.setSouth((latLonBox.south(Marble::GeoDataCoordinates::Degree)-0.0001), Marble::GeoDataCoordinates::Degree);
      //  }
        if(d->currentMouseMode == MouseModeZoom)
        {
            d->currentBackend->centerOn(latLonBox);
        }
        else
        {
            d->modelBasedFilter = false;
            QList<qreal> newSelection;
            const qreal boxWest  = latLonBox.west(Marble::GeoDataCoordinates::Degree);
            const qreal boxNorth = latLonBox.north(Marble::GeoDataCoordinates::Degree);
            const qreal boxEast  = latLonBox.east(Marble::GeoDataCoordinates::Degree);
            const qreal boxSouth = latLonBox.south(Marble::GeoDataCoordinates::Degree);
            newSelection<<boxWest<<boxNorth<<boxEast<<boxSouth;
 
            d->selectionRectangle = newSelection;
            d->cacheSelectionRectangle = newSelection;
            d->oldSelectionRectangle = newSelection;
            emit signalNewSelectionFromMap();
        }
    }
    else if ((d->currentMouseMode == MouseModeFilter && !d->selectionRectangle.isEmpty()) || (d->currentMouseMode == MouseModeSelectThumbnail))
    {
    // update the selection state of the clusters
        for (int i=0; i<clusterIndices.count(); ++i)
        {
            const int clusterIndex = clusterIndices.at(i);
            const WMWCluster currentCluster = s->clusterList.at(clusterIndex);

            // TODO: use a consistent format for tile indices
            AbstractMarkerTiler::TileIndex::List tileIndices;
            for (int j=0; j<currentCluster.tileIndicesList.count(); ++j)
            {   
                const AbstractMarkerTiler::TileIndex& currentTileIndex = AbstractMarkerTiler::TileIndex::fromIntList(currentCluster.tileIndicesList.at(j));
                tileIndices << currentTileIndex;
            }
            if(d->currentMouseMode == MouseModeFilter)
            {
                d->modelBasedFilter = true;
                s->markerModel->onIndicesClicked(tileIndices, currentCluster.selectedState, MouseModeFilter);
            }
            else
                s->markerModel->onIndicesClicked(tileIndices, currentCluster.selectedState, MouseModeSelectThumbnail);
        }
    }
}

void KMapWidget::dragEnterEvent(QDragEnterEvent* event)
{
    if ( (!s->editEnabled) || (!d->dragDropHandler) )
    {
        event->ignore();
        return;
    }

    if (d->dragDropHandler->accepts(event)==Qt::IgnoreAction)
    {
        event->ignore();
        return;
    }

    // TODO: need data about the dragged object: #markers, selected, icon, ...
    event->accept();

//     if (!dragData->haveDragPixmap)
//         d->currentBackend->updateDragDropMarker(event->pos(), dragData);
}

void KMapWidget::dragMoveEvent(QDragMoveEvent* /*event*/)
{
    // TODO: update the position of the drag marker if it is to be shown
//     if (!dragData->haveDragPixmap)
//         d->currentBackend->updateDragDropMarkerPosition(event->pos());
}

void KMapWidget::dropEvent(QDropEvent* event)
{
    // remove the drag marker:
//     d->currentBackend->updateDragDropMarker(QPoint(), 0);

    if ( (!s->editEnabled) || (!d->dragDropHandler) )
    {
        event->ignore();
        return;
    }

    GeoCoordinates dropCoordinates;
    if (!d->currentBackend->geoCoordinates(event->pos(), &dropCoordinates))
        return;

    // the drag and drop handler handled the drop if it returned true here
    if (d->dragDropHandler->dropEvent(event, dropCoordinates))
    {
        event->acceptProposedAction();
    }
}

void KMapWidget::dragLeaveEvent(QDragLeaveEvent* event)
{
    Q_UNUSED(event);

    // remove the marker:
//     d->currentBackend->updateDragDropMarker(QPoint(), 0);
}

void KMapWidget::markClustersAsDirty()
{
    d->clustersDirty = true;
}

/**
 * @brief Controls whether the user can switch from browse to edit mode.
 */
void KMapWidget::setEditModeAvailable(const bool state)
{
    d->editModeAvailable = state;

    if (d->browseModeControlsHolder)
    {
        d->browseModeControlsHolder->setVisible(d->editModeAvailable);
    }
}

void KMapWidget::setDragDropHandler(DragDropHandler* const dragDropHandler)
{
    d->dragDropHandler = dragDropHandler;
}

QVariant KMapWidget::getClusterRepresentativeMarker(const int clusterIndex, const int sortKey)
{
    if (!s->markerModel)
        return QVariant();

    const WMWCluster cluster = s->clusterList.at(clusterIndex);
    QMap<int, QVariant>::const_iterator it = cluster.representativeMarkers.find(sortKey);
    if (it!=cluster.representativeMarkers.end())
        return *it;

    QList<QVariant> repIndices;
    for (int i=0; i<cluster.tileIndicesList.count(); ++i)
    {
        repIndices <<  s->markerModel->getTileRepresentativeMarker(AbstractMarkerTiler::TileIndex::fromIntList(cluster.tileIndicesList.at(i)), sortKey);
    }

    const QVariant clusterRepresentative = s->markerModel->bestRepresentativeIndexFromList(repIndices, sortKey);

    s->clusterList[clusterIndex].representativeMarkers[sortKey] = clusterRepresentative;

    return clusterRepresentative;
}

void KMapWidget::slotItemDisplaySettingsChanged()
{
    s->previewSingleItems = d->actionPreviewSingleItems->isChecked();
    s->previewGroupedItems = d->actionPreviewGroupedItems->isChecked();
    s->showNumbersOnItems = d->actionShowNumbersOnItems->isChecked();

    // TODO: update action availability?

    // TODO: we just need to update the display, no need to recluster?
    slotRequestLazyReclustering();
}

void KMapWidget::setSortOptionsMenu(QMenu* const sortMenu)
{
    d->sortMenu = sortMenu;

    rebuildConfigurationMenu();
}

void KMapWidget::setSortKey(const int sortKey)
{
    s->sortKey = sortKey;

    // this is probably faster than writing a function that changes all the clusters icons...
    slotRequestLazyReclustering();
}

QPixmap KMapWidget::getDecoratedPixmapForCluster(const int clusterId, const WMWSelectionState* const selectedStateOverride, const int* const countOverride, QPoint* const centerPoint)
{
    const int circleRadius = d->thumbnailSize/2;
    WMWCluster& cluster = s->clusterList[clusterId];

    int markerCount = cluster.markerCount;
    WMWSelectionState selectedState = cluster.selectedState;
    if (selectedStateOverride)
    {
        selectedState = *selectedStateOverride;
        markerCount = *countOverride;
    }

    // determine the colors:
    QColor       fillColor;
    QColor       strokeColor;
    Qt::PenStyle strokeStyle;
    QColor       labelColor;
    QString      labelText;
    getColorInfos(clusterId, &fillColor, &strokeColor,
                        &strokeStyle, &labelText, &labelColor,
                        &selectedState,
                        &markerCount);

    // determine whether we should use a pixmap or a placeholder
    if (s->inEditMode)
    {
        QString pixmapName = fillColor.name().mid(1);
        if (selectedState==WMWSelectedAll)
        {
            pixmapName+="-selected";
        }
        if (selectedState==WMWSelectedSome)
        {
            pixmapName+="-someselected";
        }
        const QPixmap& markerPixmap = s->markerPixmaps[pixmapName];

        // update the display information stored in the cluster:
        cluster.pixmapType = WMWCluster::PixmapMarker;
        cluster.pixmapOffset = QPoint(markerPixmap.width()/2, 0);
        cluster.pixmapSize = markerPixmap.size();

        if (centerPoint)
        {
            *centerPoint = cluster.pixmapOffset;
        }

        return markerPixmap;
    }

    bool displayThumbnail = (s->markerModel != 0);
    if (displayThumbnail)
    {
        if (markerCount==1)
        {
            displayThumbnail = s->previewSingleItems;
        }
        else
        {
            displayThumbnail = s->previewGroupedItems;
        }
    }

    if (displayThumbnail)
    {
        const QVariant representativeMarker = getClusterRepresentativeMarker(clusterId, s->sortKey);
        const int undecoratedThumbnailSize = getUndecoratedThumbnailSize();
        QPixmap clusterPixmap = s->markerModel->pixmapFromRepresentativeIndex(representativeMarker, QSize(undecoratedThumbnailSize, undecoratedThumbnailSize));

        if (!clusterPixmap.isNull())
        {
            QPixmap resultPixmap(clusterPixmap.size()+QSize(2,2));
            QPainter painter(&resultPixmap);
            painter.setRenderHint(QPainter::Antialiasing);

            QPen circlePen;
            circlePen.setWidth(1);
            if (strokeStyle!=Qt::SolidLine)
            {
                // paint a white border around the image
                circlePen.setColor(Qt::white);
                painter.setPen(circlePen);
                painter.drawRect(0, 0, resultPixmap.size().width()-1, resultPixmap.size().height()-1);
            }

            painter.drawPixmap(QPoint(1,1), clusterPixmap);

            // now draw the selection border
            circlePen.setColor(strokeColor);
            circlePen.setStyle(strokeStyle);
            painter.setPen(circlePen);
            painter.drawRect(0, 0, resultPixmap.size().width()-1, resultPixmap.size().height()-1);

            if (s->showNumbersOnItems)
            {
                QPen labelPen(labelColor);

                // note: the pen has to be set, otherwise the bounding rect is 0 x 0!!!
                painter.setPen(labelPen);
                const QRect textRect(0, 0, resultPixmap.width(), resultPixmap.height());
                QRect textBoundingRect = painter.boundingRect(textRect, Qt::AlignHCenter|Qt::AlignVCenter, labelText);
                textBoundingRect.adjust(-1, -1, 1, 1);

                // fill the bounding rect:
                painter.setPen(Qt::NoPen);
                painter.setBrush(QColor::fromRgb(0xff, 0xff, 0xff, 0x80));
                painter.drawRect(textBoundingRect);

                // draw the text:
                painter.setPen(labelPen);
                painter.setBrush(Qt::NoBrush);
                painter.drawText(textRect,
                            Qt::AlignHCenter|Qt::AlignVCenter, labelText);
            }

            // update the display information stored in the cluster:
            cluster.pixmapType = WMWCluster::PixmapImage;
            cluster.pixmapOffset = QPoint(resultPixmap.width()/2, resultPixmap.height()/2);
            cluster.pixmapSize = resultPixmap.size();

            if (centerPoint)
            {
                *centerPoint = cluster.pixmapOffset;
            }

            return resultPixmap;
        }
    }

    // we do not have a thumbnail, draw the circle instead:
    QPen circlePen;
    circlePen.setColor(strokeColor);
    circlePen.setStyle(strokeStyle);
    circlePen.setWidth(2);
    QBrush circleBrush(fillColor);
    QPen labelPen;
    labelPen.setColor(labelColor);
    const QRect circleRect(0, 0, 2*circleRadius, 2*circleRadius);

    const int pixmapDiameter = 2*(circleRadius+1);
    QPixmap circlePixmap(pixmapDiameter, pixmapDiameter);
    // TODO: cache this somehow
    circlePixmap.fill(QColor(0,0,0,0));

    QPainter circlePainter(&circlePixmap);
    circlePainter.setPen(circlePen);
    circlePainter.setBrush(circleBrush);
    circlePainter.drawEllipse(circleRect);

    circlePainter.setPen(labelPen);
    circlePainter.setBrush(Qt::NoBrush);
    circlePainter.drawText(circleRect, Qt::AlignHCenter|Qt::AlignVCenter, labelText);

    // update the display information stored in the cluster:
    cluster.pixmapType = WMWCluster::PixmapCircle;
    cluster.pixmapOffset = QPoint(circlePixmap.width()/2, circlePixmap.height()/2);
    cluster.pixmapSize = circlePixmap.size();

    if (centerPoint)
    {
        *centerPoint = QPoint(circlePixmap.width()/2, circlePixmap.height()/2);
    }

    return circlePixmap;
}

void KMapWidget::setThumnailSize(const int newThumbnailSize)
{
    d->thumbnailSize = qMax(KMapMinThumbnailSize, newThumbnailSize);

    // make sure the grouping radius is larger than the thumbnail size
    if (2*d->groupingRadius < newThumbnailSize)
    {
        // TODO: more straightforward way for this?
        d->groupingRadius = newThumbnailSize/2 + newThumbnailSize%2;
    }

    if (!s->inEditMode)
    {
        slotRequestLazyReclustering();
    }
    slotUpdateActionsEnabled();
}

void KMapWidget::setGroupingRadius(const int newGroupingRadius)
{
    d->groupingRadius = qMax(KMapMinGroupingRadius, newGroupingRadius);

    // make sure the thumbnails are smaller than the grouping radius
    if (2*d->groupingRadius < d->thumbnailSize)
    {
        d->thumbnailSize = 2*newGroupingRadius;
    }

    if (!s->inEditMode)
    {
        slotRequestLazyReclustering();
    }
    slotUpdateActionsEnabled();
}

void KMapWidget::setEditGroupingRadius(const int newGroupingRadius)
{
    d->editGroupingRadius = qMax(KMapMinEditGroupingRadius, newGroupingRadius);

    if (s->inEditMode)
    {
        slotRequestLazyReclustering();
    }
    slotUpdateActionsEnabled();
}

void KMapWidget::slotDecreaseThumbnailSize()
{
    if (s->inEditMode)
        return;

    if (d->thumbnailSize>KMapMinThumbnailSize)
    {
        const int newThumbnailSize = qMax(KMapMinThumbnailSize, d->thumbnailSize-5);

        // make sure the grouping radius is also decreased
        // this will automatically decrease the thumbnail size as well
        setGroupingRadius(newThumbnailSize/2);
    }
}

void KMapWidget::slotIncreaseThumbnailSize()
{
    if (s->inEditMode)
        return;

    setThumnailSize(d->thumbnailSize+5);
}

int KMapWidget::getThumbnailSize() const
{
    return d->thumbnailSize;
}

int KMapWidget::getUndecoratedThumbnailSize() const
{
    return d->thumbnailSize-2;
}

void KMapWidget::setSelectionStatus(const bool status)
{
    d->hasSelection = status;
    d->currentBackend->setSelectionStatus(d->hasSelection);
}

bool KMapWidget::getSelectionStatus() const
{
    //return !d->selectionRectangle.isEmpty();
    return d->hasSelection;
}

QList<double> KMapWidget::selectionCoordinates() const
{
    return d->selectionRectangle;
}

void KMapWidget::setSelectionCoordinates(QList<qreal>& sel)
{
    if(d->currentMouseMode == MouseModeSelection || d->hasSelection)  
        d->currentBackend->setSelectionRectangle(sel);
    else
        d->currentBackend->removeSelectionRectangle();
    d->cacheSelectionRectangle = sel;
    d->selectionRectangle = sel;
    if(!sel.isEmpty())
        d->oldSelectionRectangle = sel;
}

void KMapWidget::clearSelectionRectangle()
{
    d->selectionRectangle.clear();
}

void KMapWidget::slotNewSelectionFromMap(const QList<qreal>& sel)
{

    d->selectionRectangle      = sel;
    d->cacheSelectionRectangle = sel;
    d->oldSelectionRectangle   = sel;
    emit signalNewSelectionFromMap();
}

void KMapWidget::slotSetPanMode()
{
    if(d->actionSetPanMode->isChecked())
    {
        d->currentMouseMode = MouseModePan;
        d->actionSetSelectionMode->setChecked(false);
        d->actionSetZoomMode->setChecked(false);
        d->actionSetFilterMode->setChecked(false);
        d->actionSetSelectThumbnailMode->setChecked(false);

        d->currentBackend->mouseModeChanged(MouseModePan);
        if(!d->hasSelection)
           d->currentBackend->removeSelectionRectangle(); 
    }
    else
    {
        if(d->currentMouseMode == MouseModePan)
            d->actionSetPanMode->setChecked(true);
    }
}

void KMapWidget::slotSetSelectionMode()
{
    if(d->actionSetSelectionMode->isChecked())
    {
        d->currentMouseMode = MouseModeSelection;
        d->actionSetPanMode->setChecked(false);
        d->actionSetZoomMode->setChecked(false);
        d->actionSetFilterMode->setChecked(false);
        d->actionSetSelectThumbnailMode->setChecked(false);

        d->currentBackend->setSelectionRectangle(d->oldSelectionRectangle);

        d->currentBackend->mouseModeChanged(MouseModeSelection);
    }
    else
    {
        if(d->currentMouseMode == MouseModeSelection)
            d->actionSetSelectionMode->setChecked(true);
    }
}


void KMapWidget::slotSetZoomMode()
{
    if(d->actionSetZoomMode->isChecked())
    { 
        d->currentMouseMode = MouseModeZoom;
        d->actionSetPanMode->setChecked(false);
        d->actionSetSelectionMode->setChecked(false);
        d->actionSetFilterMode->setChecked(false);
        d->actionSetSelectThumbnailMode->setChecked(false);        

        d->currentBackend->mouseModeChanged(MouseModeZoom);
    }
    else
    {
        if(d->currentMouseMode == MouseModeZoom)
            d->actionSetZoomMode->setChecked(true);
    }
}

void KMapWidget::slotSetFilterMode()
{
    if(d->actionSetFilterMode->isChecked())
    {
        d->currentMouseMode = MouseModeFilter;
        d->actionSetPanMode->setChecked(false);
        d->actionSetSelectionMode->setChecked(false);
        d->actionSetZoomMode->setChecked(false);
        d->actionSetSelectThumbnailMode->setChecked(false);
    
        d->currentBackend->mouseModeChanged(MouseModeFilter);
    }
    else
    {
        if(d->currentMouseMode == MouseModeFilter)
            d->actionSetFilterMode->setChecked(true);
    }
}

void KMapWidget::slotSetSelectThumbnailMode()
{
    if(d->actionSetSelectThumbnailMode->isChecked())
    {
        d->currentMouseMode = MouseModeSelectThumbnail;
        d->actionSetPanMode->setChecked(false);
        d->actionSetSelectionMode->setChecked(false);
        d->actionSetZoomMode->setChecked(false);
        d->actionSetFilterMode->setChecked(false);
    
        d->currentBackend->mouseModeChanged(MouseModeSelectThumbnail);
    }
    else
    {
        if(d->currentMouseMode == MouseModeSelectThumbnail)
            d->actionSetSelectThumbnailMode->setChecked(true);
    }
}

void KMapWidget::slotRemoveCurrentSelection()
{
    emit signalRemoveCurrentSelection();
    clearSelectionRectangle();
    d->currentBackend->removeSelectionRectangle();
    if(d->currentMouseMode == MouseModeSelection)
        d->currentBackend->setSelectionRectangle(d->oldSelectionRectangle);
}

void KMapWidget::slotRemoveCurrentFilter()
{
   if(d->modelBasedFilter)
        emit signalRemoveCurrentFilter();
   else
   {
        slotRemoveCurrentSelection();
   }
}

void KMapWidget::slotUngroupedModelChanged()
{
    // determine the index under which we handle this model
    QObject* const senderObject = sender();

    QAbstractItemModel* const senderModel = qobject_cast<QAbstractItemModel*>(senderObject);
    if (senderModel)
    {
        for (int i=0; i<s->ungroupedModels.count(); ++i)
        {
            if (s->ungroupedModels.at(i)->model()==senderModel)
            {
                emit(signalUngroupedModelChanged(i));

                break;
            }
        }
        return;
    }

    ModelHelper* const senderHelper = qobject_cast<ModelHelper*>(senderObject);
    if (senderHelper)
    {
        for (int i=0; i<s->ungroupedModels.count(); ++i)
        {
            if (s->ungroupedModels.at(i)==senderHelper)
            {
                emit(signalUngroupedModelChanged(i));

                break;
            }
        }
    }

    QItemSelectionModel* const senderSelectionModel = qobject_cast<QItemSelectionModel*>(senderObject);
    if (senderSelectionModel)
    {
        for (int i=0; i<s->ungroupedModels.count(); ++i)
        {
            if (s->ungroupedModels.at(i)->selectionModel()==senderSelectionModel)
            {
                emit(signalUngroupedModelChanged(i));

                break;
            }
        }
        return;
    }
}

void KMapWidget::addWidgetToControlWidget(QWidget* const newWidget)
{
    // make sure the control widget exists
    if (!d->controlWidget)
        getControlWidget();

    QHBoxLayout* const hBoxLayout = reinterpret_cast<QHBoxLayout*>(d->hBoxForAdditionalControlWidgetItems->layout());
    if (hBoxLayout)
    {
        hBoxLayout->addWidget(newWidget);
    }
}

void KMapWidget::setEditEnabled(const bool state)
{
    s->editEnabled = state;
}

// Static methods ---------------------------------------------------------

QString KMapWidget::MarbleWidgetVersion()
{
    return QString(MARBLE_VERSION_STRING);
}

QString KMapWidget::version()
{
    return QString(kmap_version);
}

void KMapWidget::setActive(const bool state)
{
    const bool oldState = d->activeState;
    d->activeState = state;
    if (d->currentBackend)
    {
        d->currentBackend->setActive(state);
    }
    if (s->markerModel)
    {
        s->markerModel->setActive(state);
    }
    if(state && !oldState && d->clustersDirty)
    {
        slotRequestLazyReclustering();
    }
}

bool KMapWidget::getActiveState()
{
    return d->activeState;
}

void KMapWidget::setVisibleMouseModes(const MouseModes mouseModes)
{
    d->visibleMouseModes = mouseModes;

    if (d->mouseModesHolder)
    {
        d->mouseModesHolder->setVisible(d->visibleMouseModes);

        d->setSelectionModeButton->setVisible(d->visibleMouseModes.testFlag(MouseModeSelection));
        d->removeCurrentSelectionButton->setVisible(d->visibleMouseModes.testFlag(MouseModeSelection));
        d->setPanModeButton->setVisible(d->visibleMouseModes.testFlag(MouseModePan));
        d->setZoomModeButton->setVisible(d->visibleMouseModes.testFlag(MouseModeZoom));
        d->setFilterModeButton->setVisible(d->visibleMouseModes.testFlag(MouseModeFilter));
        d->removeFilterModeButton->setVisible(d->visibleMouseModes.testFlag(MouseModeFilter));
        d->setSelectThumbnailMode->setVisible(d->visibleMouseModes.testFlag(MouseModeSelectThumbnail));
    }
}

void KMapWidget::setAvailableMouseModes(const MouseModes mouseModes)
{
    d->availableMouseModes = mouseModes;
}

} /* namespace KMap */

