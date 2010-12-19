/** ===========================================================
 * @file
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
 * either version 2, or (at your option) any later version.
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
#include "placeholderwidget.h"

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
 * @li Finally, setActive() has to be called to tell the widget that it should start displaying things.
 */

const int KMapMinMarkerGroupingRadius        = 1;
const int KMapMinThumbnailGroupingRadius     = 15;
const int KMapMinThumbnailSize               = KMapMinThumbnailGroupingRadius * 2;

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
        cacheZoom(QLatin1String("marble:900" )),
        configurationMenu(0),
        actionGroupBackendSelection(0),
        actionZoomIn(0),
        actionZoomOut(0),
        actionShowThumbnails(0),
        mouseModesHolder(0),
        controlWidget(0),
        lazyReclusteringRequested(false),
        clustersDirty(false),
        dragDropHandler(0),
        sortMenu(0),
        thumbnailSize(KMapMinThumbnailSize),
        thumbnailGroupingRadius(KMapMinThumbnailGroupingRadius),
        markerGroupingRadius(KMapMinMarkerGroupingRadius),
        actionIncreaseThumbnailSize(0),
        actionDecreaseThumbnailSize(0),
        actionSetSelectionMode(0),
        actionSetPanMode(0),
        actionSetZoomMode(0),
        actionSetFilterDatabaseMode(0),
        actionSetFilterModelMode(0),
        actionRemoveFilterMode(0),
        actionSetSelectThumbnailMode(0),
        thumbnailTimer(0),
        thumbnailTimerCount(0),
        thumbnailsHaveBeenLoaded(false),
        availableExtraActions(0),
        visibleExtraActions(0),
        actionStickyMode(0),
        buttonStickyMode(0),
        placeholderWidget(0)
    {
    }

    QList<AltitudeBackend*> loadedAltitudeBackends;
    QList<MapBackend*>      loadedBackends;
    MapBackend*             currentBackend;
    bool                    currentBackendReady;
    QString                 currentBackendName;
    QStackedLayout*         stackedLayout;

    // these values are cached in case the backend is not ready:
    GeoCoordinates          cacheCenterCoordinate;
    QString                 cacheZoom;

    // actions for controlling the widget
    QMenu*                  configurationMenu;
    QActionGroup*           actionGroupBackendSelection;
    KAction*                actionZoomIn;
    KAction*                actionZoomOut;
    KAction*                actionShowThumbnails;
    QWidget*                mouseModesHolder;
    QPointer<KHBox>         controlWidget;
    KAction*                actionPreviewSingleItems;
    KAction*                actionPreviewGroupedItems;
    KAction*                actionShowNumbersOnItems;

    bool                    lazyReclusteringRequested;
    bool                    clustersDirty;

    DragDropHandler*        dragDropHandler;

    QMenu*                  sortMenu;
    int                     thumbnailSize;
    int                     thumbnailGroupingRadius;
    int                     markerGroupingRadius;
    KAction*                actionIncreaseThumbnailSize;
    KAction*                actionDecreaseThumbnailSize;
    KHBox*                  hBoxForAdditionalControlWidgetItems;

    KAction*                actionRemoveCurrentSelection;
    KAction*                actionSetSelectionMode;
    KAction*                actionSetPanMode;
    KAction*                actionSetZoomMode;
    KAction*                actionSetFilterDatabaseMode;
    KAction*                actionSetFilterModelMode;
    KAction*                actionRemoveFilterMode;
    KAction*                actionSetSelectThumbnailMode;
    MouseModes              currentMouseMode;
    QToolButton*            setPanModeButton;
    QToolButton*            setSelectionModeButton;
    QToolButton*            removeCurrentSelectionButton;
    QToolButton*            setZoomModeButton;
    QToolButton*            setFilterDatabaseModeButton;
    QToolButton*            setFilterModelModeButton;
    QToolButton*            removeFilterModeButton;
    QToolButton*            setSelectThumbnailMode;

    QTimer*                 thumbnailTimer;
    int                     thumbnailTimerCount;
    bool                    thumbnailsHaveBeenLoaded;

    ExtraActions            availableExtraActions;
    ExtraActions            visibleExtraActions;
    KAction*                actionStickyMode;
    QToolButton*            buttonStickyMode;

    // to be sorted later
    PlaceholderWidget*      placeholderWidget;
};

KMapWidget::KMapWidget(QWidget* const parent)
    : QWidget(parent), s(new KMapSharedData), d(new KMapWidgetPrivate)
{
    createActions();

    s->worldMapWidget = this;

    d->stackedLayout = new QStackedLayout(this);
    setLayout(d->stackedLayout);

    d->placeholderWidget = new PlaceholderWidget();
    d->stackedLayout->addWidget(d->placeholderWidget);

    d->loadedBackends.append(new BackendGoogleMaps(s, this));
    d->loadedBackends.append(new BackendMarble(s, this));
//     d->loadedBackends.append(new BackendOSM(s, this));
    createActionsForBackendSelection();

    AltitudeBackend* const geonamesBackend = new BackendAltitudeGeonames(s, this);
    d->loadedAltitudeBackends.append(geonamesBackend);
    connect(geonamesBackend, SIGNAL(signalAltitudes(const KMap::KMapAltitudeLookup::List)),
            this, SIGNAL(signalAltitudeLookupReady(const KMap::KMapAltitudeLookup::List)));

    setAcceptDrops(true);
}

void KMapWidget::createActions()
{
    d->actionZoomIn = new KAction(this);
    d->actionZoomIn->setIcon(SmallIcon( QLatin1String("zoom-in" )));
    d->actionZoomIn->setToolTip(i18n("Zoom in"));
    connect(d->actionZoomIn, SIGNAL(triggered()),
            this, SLOT(slotZoomIn()));

    d->actionZoomOut = new KAction(this);
    d->actionZoomOut->setIcon(SmallIcon( QLatin1String("zoom-out" )));
    d->actionZoomOut->setToolTip(i18n("Zoom out"));
    connect(d->actionZoomOut, SIGNAL(triggered()),
            this, SLOT(slotZoomOut()));

    d->actionShowThumbnails = new KAction(this);
    d->actionShowThumbnails->setToolTip(i18n("Switch between markers and thumbnails."));
    d->actionShowThumbnails->setCheckable(true);

    connect(d->actionShowThumbnails, SIGNAL(triggered(bool)),
            this, SLOT(slotShowThumbnailsChanged()));

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
    d->actionRemoveCurrentSelection->setIcon(SmallIcon( QLatin1String("edit-clear" )));
    d->actionRemoveCurrentSelection->setToolTip(i18n("Removes current selection."));

    d->actionSetSelectionMode = new KAction(this);
    d->actionSetSelectionMode->setCheckable(true);
    d->actionSetSelectionMode->setIcon(SmallIcon( QLatin1String("select-rectangular" )));
    d->actionSetSelectionMode->setToolTip(i18n("Select images by drawing a rectangle."));

    d->actionSetPanMode = new KAction(this);
    d->actionSetPanMode->setCheckable(true);
    d->actionSetPanMode->setToolTip(i18n("Pan mode."));
    d->actionSetPanMode->setIcon(SmallIcon( QLatin1String("transform-move" )));
    d->actionSetPanMode->setChecked(true);

    d->actionSetZoomMode = new KAction(this);
    d->actionSetZoomMode->setCheckable(true);
    d->actionSetZoomMode->setToolTip(i18n("Zoom into a group."));
    d->actionSetZoomMode->setIcon(SmallIcon( QLatin1String("page-zoom" )));

    d->actionSetFilterDatabaseMode = new KAction(this);
    d->actionSetFilterDatabaseMode->setCheckable(true);
    d->actionSetFilterDatabaseMode->setToolTip(i18n("Filter images"));
    d->actionSetFilterDatabaseMode->setIcon(SmallIcon( QLatin1String("view-filter" )));

    d->actionSetFilterModelMode = new KAction(i18n("F"), this);
    d->actionSetFilterModelMode->setCheckable(true);
    d->actionSetFilterModelMode->setToolTip(i18n("Filter images inside selection"));

    d->actionRemoveFilterMode = new KAction(this);
    d->actionRemoveFilterMode->setToolTip(i18n("Remove the current filter"));
    d->actionRemoveFilterMode->setIcon(SmallIcon( QLatin1String("window-close" )));

    d->actionSetSelectThumbnailMode = new KAction(this);
    d->actionSetSelectThumbnailMode->setCheckable(true);
    d->actionSetSelectThumbnailMode->setToolTip(i18n("Select images"));
    d->actionSetSelectThumbnailMode->setIcon(SmallIcon( QLatin1String("edit-select" )));

    d->actionStickyMode = new KAction(this);
    d->actionStickyMode->setCheckable(true);
    d->actionStickyMode->setToolTip(i18n("Lock the map position"));
    connect(d->actionStickyMode, SIGNAL(triggered(const bool)),
            this, SLOT(slotStickyModeChanged()));

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

    connect(d->actionSetFilterDatabaseMode, SIGNAL(changed()),
            this, SLOT(slotSetFilterDatabaseMode()));

    connect(d->actionSetFilterModelMode, SIGNAL(changed()),
            this, SLOT(slotSetFilterModelMode()));
/*
    connect(d->actionRemoveFilterMode, SIGNAL(triggered()),
            this, SIGNAL(signalRemoveCurrentFilter()));
*/
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

    /// @todo delete s, but make sure it is not accessed by any other objects any more!
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

    // switch to the placeholder widget:
    setShowPlaceholderWidget(true);
    removeMapWidgetFromFrame();

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

        disconnect(this, SIGNAL(signalUngroupedModelChanged(const int)),
                    d->currentBackend, SLOT(slotUngroupedModelChanged(const int)));

        if (s->markerModel)
        {
            disconnect(s->markerModel, SIGNAL(signalThumbnailAvailableForIndex(const QVariant&, const QPixmap&)),
                        d->currentBackend, SLOT(slotThumbnailAvailableForIndex(const QVariant&, const QPixmap&)));
        }

        disconnect(d->currentBackend, SIGNAL(signalSelectionHasBeenMade(const KMap::GeoCoordinates::Pair&)),
                this, SLOT(slotNewSelectionFromMap(const KMap::GeoCoordinates::Pair&)));

    }

    MapBackend* backend = 0;
    foreach(backend, d->loadedBackends)
    {
        if (backend->backendName() == backendName)
        {
            kDebug() << QString::fromLatin1("setting backend %1").arg(backendName);
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

            // TODO: this connection is queued because otherwise QAbstractItemModel::itemSelected does not
            //       reflect the true state. Maybe monitor another signal instead?
            connect(this, SIGNAL(signalUngroupedModelChanged(const int)),
                    d->currentBackend, SLOT(slotUngroupedModelChanged(const int)), Qt::QueuedConnection);

            if (s->markerModel)
            {
                connect(s->markerModel, SIGNAL(signalThumbnailAvailableForIndex(const QVariant&, const QPixmap&)),
                        d->currentBackend, SLOT(slotThumbnailAvailableForIndex(const QVariant&, const QPixmap&)));
            }

            connect(d->currentBackend, SIGNAL(signalSelectionHasBeenMade(const KMap::GeoCoordinates::Pair&)),
                    this, SLOT(slotNewSelectionFromMap(const KMap::GeoCoordinates::Pair&)));

            if (s->activeState)
            {
                setMapWidgetInFrame(d->currentBackend->mapWidget());

                // call this slot manually in case the backend was ready right away:
                if (d->currentBackend->isReady())
                {
                    slotBackendReady(d->currentBackendName);
                }
                else
                {
                    rebuildConfigurationMenu();
                }
            }

            d->currentBackend->setActive(s->activeState);

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
    d->currentBackend->mouseModeChanged(s->currentMouseMode);
    setSelectionCoordinates(s->selectionRectangle);
}

void KMapWidget::saveBackendToCache()
{
    if (!d->currentBackendReady)
        return;

    d->cacheCenterCoordinate   = getCenter();
    d->cacheZoom               = getZoom();
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
    kDebug()<<QString::fromLatin1("backend %1 is ready!").arg(backendName);
    if (backendName != d->currentBackendName)
        return;

    d->currentBackendReady = true;

    applyCacheToBackend();

    setShowPlaceholderWidget(false);

    if (!d->thumbnailsHaveBeenLoaded)
    {
        d->thumbnailTimer      = new QTimer(this);
        d->thumbnailTimerCount = 0;
        connect(d->thumbnailTimer, SIGNAL(timeout()), this, SLOT(stopThumbnailTimer()));
        d->thumbnailTimer->start(2000);
    }

    updateMarkers();
    markClustersAsDirty();

    rebuildConfigurationMenu();
}

void KMapWidget::stopThumbnailTimer()
{
    d->currentBackend->updateMarkers();
    d->thumbnailTimerCount++;
    if (d->thumbnailTimerCount == 10)
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
    group->writeEntry("Thumbnail Grouping Radius", d->thumbnailGroupingRadius);
    group->writeEntry("Marker Grouping Radius", d->markerGroupingRadius);
    group->writeEntry("Show Thumbnails", s->showThumbnails);
    if (d->visibleExtraActions.testFlag(ExtraActionSticky))
    {
        group->writeEntry("Sticky Mode State", d->actionStickyMode->isChecked());
    }

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
    setThumbnailGroupingRadius(group->readEntry("Thumbnail Grouping Radius", 2*KMapMinThumbnailGroupingRadius));
    setMarkerGroupingRadius(group->readEntry("Edit Grouping Radius", KMapMinMarkerGroupingRadius));
    s->showThumbnails = group->readEntry("Show Thumbnails", false);
    d->actionShowThumbnails->setChecked(s->showThumbnails);

    for (int i=0; i<d->loadedBackends.size(); ++i)
    {
        d->loadedBackends.at(i)->readSettingsFromGroup(group);
    }

    d->actionStickyMode->setChecked(group->readEntry("Sticky Mode State", d->actionStickyMode->isChecked()));
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

    if (s->showThumbnails)
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
    if (actionName==QLatin1String("zoomin" ))
    {
        return d->actionZoomIn;
    }
    else if (actionName==QLatin1String("zoomout" ))
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
        configurationButton->setIcon(SmallIcon( QLatin1String("applications-internet" )));
        configurationButton->setMenu(d->configurationMenu);
        configurationButton->setPopupMode(QToolButton::InstantPopup);

        QToolButton* const zoomInButton = new QToolButton(d->controlWidget);
        zoomInButton->setDefaultAction(d->actionZoomIn);

        QToolButton* const zoomOutButton = new QToolButton(d->controlWidget);
        zoomOutButton->setDefaultAction(d->actionZoomOut);

        QToolButton* const showThumbnailsButton = new QToolButton(d->controlWidget);
        showThumbnailsButton->setDefaultAction(d->actionShowThumbnails);

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

        d->setFilterDatabaseModeButton = new QToolButton(d->mouseModesHolder);
        d->setFilterDatabaseModeButton->setDefaultAction(d->actionSetFilterDatabaseMode);

        d->setFilterModelModeButton = new QToolButton(d->mouseModesHolder);
        d->setFilterModelModeButton->setDefaultAction(d->actionSetFilterModelMode);

        d->removeFilterModeButton = new QToolButton(d->mouseModesHolder);
        d->removeFilterModeButton->setDefaultAction(d->actionRemoveFilterMode);

        d->setSelectThumbnailMode = new QToolButton(d->mouseModesHolder);
        d->setSelectThumbnailMode->setDefaultAction(d->actionSetSelectThumbnailMode);

        d->buttonStickyMode = new QToolButton(d->controlWidget);
        d->buttonStickyMode->setDefaultAction(d->actionStickyMode);

        d->hBoxForAdditionalControlWidgetItems = new KHBox(d->controlWidget);

        setVisibleMouseModes(s->visibleMouseModes);
        setVisibleExtraActions(d->visibleExtraActions);

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
    d->actionDecreaseThumbnailSize->setEnabled((s->showThumbnails)&&(d->thumbnailSize>KMapMinThumbnailSize));
    // TODO: define an upper limit!
    d->actionIncreaseThumbnailSize->setEnabled(s->showThumbnails);

    d->actionSetSelectionMode->setEnabled(s->availableMouseModes.testFlag(MouseModeSelection));
    d->actionRemoveCurrentSelection->setEnabled(s->availableMouseModes.testFlag(MouseModeSelection));
    d->actionSetPanMode->setEnabled(s->availableMouseModes.testFlag(MouseModePan));
    d->actionSetZoomMode->setEnabled(s->availableMouseModes.testFlag(MouseModeZoom));
    d->actionSetFilterDatabaseMode->setEnabled(s->availableMouseModes.testFlag(MouseModeSelectionFromIcon));
    d->actionSetFilterModelMode->setEnabled(s->availableMouseModes.testFlag(MouseModeFilter));
    d->actionRemoveFilterMode->setEnabled(s->availableMouseModes.testFlag(MouseModeSelectionFromIcon));
    d->actionSetSelectThumbnailMode->setEnabled(s->availableMouseModes.testFlag(MouseModeSelectThumbnail));

    d->actionStickyMode->setEnabled(d->availableExtraActions.testFlag(ExtraActionSticky));

    // TODO: cache the icons somewhere?
    d->actionStickyMode->setIcon(SmallIcon( QLatin1String( d->actionStickyMode->isChecked()?"object-locked":"object-unlocked" )));
    d->actionShowThumbnails->setIcon(d->actionShowThumbnails->isChecked()?SmallIcon( QLatin1String("folder-image") ):KMapGlobalObject::instance()->getMarkerPixmap(QLatin1String("marker-icon-16x16" )));
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
    const int ClusterRadius          = s->showThumbnails ? d->thumbnailGroupingRadius : d->markerGroupingRadius;
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

//         kDebug()<<QString::fromLatin1("pixel at: %1, %2 (%3): %4 markers").arg(tilePoint.x()).arg(tilePoint.y()).arg(linearIndex).arg(pixelCountGrid[linearIndex]);
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
        KMapCluster cluster;
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

        kDebug()<<QString::fromLatin1("created cluster %1: %2 tiles").arg(s->clusterList.size()).arg(cluster.tileIndicesList.count());

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
        KMapCluster& cluster = s->clusterList[i];

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
            cluster.selectedState = KMapSelectedNone;
        }
        else if (cluster.markerSelectedCount==cluster.markerCount)
        {
            cluster.selectedState = KMapSelectedAll;
        }
        else
        {
            cluster.selectedState = KMapSelectedSome;
        }
    }

//     kDebug()<<s->clusterList.size();
    kDebug()<<QString::fromLatin1("level %1: %2 non empty tiles sorted into %3 clusters (%4 searched)").arg(markerLevel).arg(debugCountNonEmptyTiles).arg(s->clusterList.count()).arg(debugTilesSearched);

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
                                    const KMapSelectionState* const overrideSelection,
                                    const int* const overrideCount) const
{
    // TODO: call the new getColorInfos function!
    const KMapCluster& cluster = s->clusterList.at(clusterIndex);

    // TODO: check that this number is already valid!
    const int nMarkers = overrideCount ? *overrideCount : cluster.markerCount;

    getColorInfos(overrideSelection?*overrideSelection:cluster.selectedState,
                  nMarkers,
                  fillColor, strokeColor, strokeStyle, labelText, labelColor);
}

void KMapWidget::getColorInfos(const KMapSelectionState selectionState,
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
        *labelText = QString::fromLatin1("%L1k").arg(qreal(nMarkers)/1000.0, 0, 'f', 1);
    }
    else if ((nMarkers>=1951)&&(nMarkers<19500))
    {
        // TODO: use KDE-versions instead
        *labelText = QString::fromLatin1("%L1k").arg(qreal(nMarkers)/1000.0, 0, 'f', 0);
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
        *labelText = QString::fromLatin1("%1E%2").arg(int(nMarkersFirstDigit)).arg(int(exponent));
    }
    *labelColor = QColor(Qt::black);

    // TODO: 'solo' and 'selected' properties have not yet been defined,
    //       therefore use the default colors
    *strokeStyle = Qt::NoPen;
    switch (selectionState)
    {
        case KMapSelectedNone:
            *strokeStyle = Qt::SolidLine;
            *strokeColor = QColor(Qt::black);
            break;
        case KMapSelectedSome:
            *strokeStyle = Qt::DotLine;
            *strokeColor = QColor(Qt::blue);
            break;
        case KMapSelectedAll:
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
    const QStringList zoomParts = someZoom.split(QLatin1Char( ':' ));
    KMAP_ASSERT(zoomParts.count()==2);
    const QString sourceBackend = zoomParts.first();

    if (sourceBackend==targetBackend)
    {
        return someZoom;
    }

    const int sourceZoom = zoomParts.last().toInt();

    int targetZoom = -1;

    // all of these values were found experimentally!
    if (targetBackend==QLatin1String("marble" ))
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

    if (targetBackend==QLatin1String("googlemaps" ))
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

    return QString::fromLatin1("%1:%2").arg(targetBackend).arg(targetZoom);
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


GeoCoordinates::Pair KMapWidget::getSelectionRectangle()
{
    return s->selectionRectangle;
}

void KMapWidget::slotClustersMoved(const QIntList& clusterIndices, const QPair<int, QModelIndex>& snapTarget)
{
    kDebug()<<clusterIndices;

    /// @todo We actually expect only one clusterindex
    int clusterIndex = clusterIndices.first();
    GeoCoordinates targetCoordinates = s->clusterList.at(clusterIndex).coordinates;

    AbstractMarkerTiler::TileIndex::List movedTileIndices;
    if (s->clusterList.at(clusterIndex).selectedState==KMapSelectedNone)
    {
        // a not-selected marker was moved. update all of its items:
        const KMapCluster& cluster = s->clusterList.at(clusterIndex);
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

bool KMapWidget::queryAltitudes(const KMapAltitudeLookup::List& queryItems, const QString& backendName)
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

    /// @todo monitor all model signals!
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

void KMapWidget::removeUngroupedModel(ModelHelper* const modelHelper)
{
    if (!modelHelper)
        return;

    const int modelIndex = s->ungroupedModels.indexOf(modelHelper);
    if (modelIndex<0)
        return;

    /// @todo monitor all model signals!
    disconnect(modelHelper->model(), SIGNAL(dataChanged(const QModelIndex&, const QModelIndex&)),
            this, SLOT(slotUngroupedModelChanged()));
    disconnect(modelHelper->model(), SIGNAL(rowsInserted(const QModelIndex&, int, int)),
            this, SLOT(slotUngroupedModelChanged()));
    disconnect(modelHelper->model(), SIGNAL(modelReset()),
            this, SLOT(slotUngroupedModelChanged()));
    disconnect(modelHelper, SIGNAL(signalVisibilityChanged()),
            this, SLOT(slotUngroupedModelChanged()));

    if (modelHelper->selectionModel())
    {
        disconnect(modelHelper->selectionModel(), SIGNAL(currentChanged(const QModelIndex&, const QModelIndex&)),
            this, SLOT(slotUngroupedModelChanged()));
    }

    s->ungroupedModels.removeAt(modelIndex);

    // the indices changed, therefore send out notifications
    // sending out a signal with i=s->ungroupedModel.count()
    // will cause the backends to see that the last model is missing
    for (int i=modelIndex; i<=s->ungroupedModels.count(); ++i)
    {
        emit(signalUngroupedModelChanged(i));
    }
}

void KMapWidget::setGroupedModel(AbstractMarkerTiler* const markerModel)
{
    s->markerModel = markerModel;

    if (s->markerModel)
    {
        s->markerModel->setActive(s->activeState);

        /// @todo this needs some buffering for the google maps backend
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

void KMapWidget::slotShowThumbnailsChanged()
{
    s->showThumbnails = d->actionShowThumbnails->isChecked();

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
    if (s->activeState)
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

    if ((s->currentMouseMode == MouseModeZoom) || (s->currentMouseMode == MouseModeSelectionFromIcon)) // && !s->selectionRectangle.first.hasCoordinates()))
    {
        Marble::GeoDataLineString tileString;

        for (int i=0; i<clusterIndices.count(); ++i)
        {
            const int clusterIndex = clusterIndices.at(i);
            const KMapCluster currentCluster = s->clusterList.at(clusterIndex);

            for (int j=0; j<currentCluster.tileIndicesList.count(); ++j)
            {
                const AbstractMarkerTiler::TileIndex& currentTileIndex =
                    AbstractMarkerTiler::TileIndex::fromIntList(currentCluster.tileIndicesList.at(j));
                for (int corner=1; corner<=4; ++corner)
                {
                    GeoCoordinates currentTileCoordinate;

                    if (corner == 1)
                        currentTileCoordinate = currentTileIndex.toCoordinates(AbstractMarkerTiler::TileIndex::CornerNW);
                    else if (corner == 2)
                        currentTileCoordinate = currentTileIndex.toCoordinates(AbstractMarkerTiler::TileIndex::CornerSW);
                    else if (corner == 3)
                        currentTileCoordinate = currentTileIndex.toCoordinates(AbstractMarkerTiler::TileIndex::CornerNE);
                    else if (corner == 4)
                        currentTileCoordinate = currentTileIndex.toCoordinates(AbstractMarkerTiler::TileIndex::CornerSE);

                    const Marble::GeoDataCoordinates tileCoordinate(currentTileCoordinate.lon(),
                                                                    currentTileCoordinate.lat(),
                                                                                              0,
                                                                    Marble::GeoDataCoordinates::Degree);

                    if (maxTileLevel < currentTileIndex.level())
                        maxTileLevel = currentTileIndex.level();

                    tileString.append(tileCoordinate);
                }
            }
        }

        Marble::GeoDataLatLonBox latLonBox = Marble::GeoDataLatLonBox::fromLineString(tileString);

/*        if (maxTileLevel != 0)
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
        if (s->currentMouseMode == MouseModeZoom)
        {
            d->currentBackend->centerOn(latLonBox);
        }
        else
        {
            s->modelBasedFilter = false;
            const GeoCoordinates::Pair newSelection(
                    GeoCoordinates(latLonBox.north(Marble::GeoDataCoordinates::Degree),
                                latLonBox.west(Marble::GeoDataCoordinates::Degree)),
                    GeoCoordinates(latLonBox.south(Marble::GeoDataCoordinates::Degree),
                                latLonBox.east(Marble::GeoDataCoordinates::Degree))
                );

            s->selectionRectangle = newSelection;
            d->currentBackend->setSelectionRectangle(s->selectionRectangle);
            emit signalNewSelectionFromMap();
            emit signalNewMapFilter(DatabaseFilter);

        }
    }
    else if ((s->currentMouseMode == MouseModeFilter && s->selectionRectangle.first.hasCoordinates()) || (s->currentMouseMode == MouseModeSelectThumbnail))
    {
    // update the selection state of the clusters
        for (int i=0; i<clusterIndices.count(); ++i)
        {
            const int clusterIndex = clusterIndices.at(i);
            const KMapCluster currentCluster = s->clusterList.at(clusterIndex);

            /// @todo use a consistent format for tile indices
            AbstractMarkerTiler::TileIndex::List tileIndices;
            for (int j=0; j<currentCluster.tileIndicesList.count(); ++j)
            {
                const AbstractMarkerTiler::TileIndex& currentTileIndex = AbstractMarkerTiler::TileIndex::fromIntList(currentCluster.tileIndicesList.at(j));
                tileIndices << currentTileIndex;
            }
            if (s->currentMouseMode == MouseModeFilter)
            {
                s->modelBasedFilter = true;
                emit signalNewMapFilter(ModelFilter);
                s->markerModel->onIndicesClicked(tileIndices, currentCluster.selectedState, MouseModeFilter);
            }
            else
                s->markerModel->onIndicesClicked(tileIndices, currentCluster.selectedState, MouseModeSelectThumbnail);
        }
    }
}

void KMapWidget::dragEnterEvent(QDragEnterEvent* event)
{
    /// @todo ignore drops if no marker tiler or model can accept them
    if (!d->dragDropHandler)
    {
        event->ignore();
        return;
    }

    if (d->dragDropHandler->accepts(event)==Qt::IgnoreAction)
    {
        event->ignore();
        return;
    }

    /// @todo need data about the dragged object: #markers, selected, icon, ...
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

    if (!d->dragDropHandler)
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

void KMapWidget::setDragDropHandler(DragDropHandler* const dragDropHandler)
{
    d->dragDropHandler = dragDropHandler;
}

QVariant KMapWidget::getClusterRepresentativeMarker(const int clusterIndex, const int sortKey)
{
    if (!s->markerModel)
        return QVariant();

    const KMapCluster cluster = s->clusterList.at(clusterIndex);
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

QPixmap KMapWidget::getDecoratedPixmapForCluster(const int clusterId, const KMapSelectionState* const selectedStateOverride, const int* const countOverride, QPoint* const centerPoint)
{
    const int circleRadius = d->thumbnailSize/2;
    KMapCluster& cluster = s->clusterList[clusterId];

    int markerCount = cluster.markerCount;
    KMapSelectionState selectedState = cluster.selectedState;
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
    if (!s->showThumbnails)
    {
        QString pixmapName = fillColor.name().mid(1);
        if (selectedState==KMapSelectedAll)
        {
            pixmapName+=QLatin1String("-selected" );
        }
        if (selectedState==KMapSelectedSome)
        {
            pixmapName+=QLatin1String("-someselected" );
        }
        const QPixmap& markerPixmap = KMapGlobalObject::instance()->getMarkerPixmap(pixmapName);

        // update the display information stored in the cluster:
        cluster.pixmapType = KMapCluster::PixmapMarker;
        cluster.pixmapOffset = QPoint(markerPixmap.width()/2, markerPixmap.height()-1);
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
            cluster.pixmapType = KMapCluster::PixmapImage;
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
    cluster.pixmapType = KMapCluster::PixmapCircle;
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
    if (2*d->thumbnailGroupingRadius < newThumbnailSize)
    {
        // TODO: more straightforward way for this?
        d->thumbnailGroupingRadius = newThumbnailSize/2 + newThumbnailSize%2;
    }

    if (s->showThumbnails)
    {
        slotRequestLazyReclustering();
    }
    slotUpdateActionsEnabled();
}

void KMapWidget::setThumbnailGroupingRadius(const int newGroupingRadius)
{
    d->thumbnailGroupingRadius = qMax(KMapMinThumbnailGroupingRadius, newGroupingRadius);

    // make sure the thumbnails are smaller than the grouping radius
    if (2*d->thumbnailGroupingRadius < d->thumbnailSize)
    {
        d->thumbnailSize = 2*newGroupingRadius;
    }

    if (s->showThumbnails)
    {
        slotRequestLazyReclustering();
    }
    slotUpdateActionsEnabled();
}

void KMapWidget::setMarkerGroupingRadius(const int newGroupingRadius)
{
    d->markerGroupingRadius = qMax(KMapMinMarkerGroupingRadius, newGroupingRadius);

    if (!s->showThumbnails)
    {
        slotRequestLazyReclustering();
    }
    slotUpdateActionsEnabled();
}

void KMapWidget::slotDecreaseThumbnailSize()
{
    if (!s->showThumbnails)
        return;

    if (d->thumbnailSize>KMapMinThumbnailSize)
    {
        const int newThumbnailSize = qMax(KMapMinThumbnailSize, d->thumbnailSize-5);

        // make sure the grouping radius is also decreased
        // this will automatically decrease the thumbnail size as well
        setThumbnailGroupingRadius(newThumbnailSize/2);
    }
}

void KMapWidget::slotIncreaseThumbnailSize()
{
    if (!s->showThumbnails)
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
    s->hasSelection = status;
    d->currentBackend->setSelectionStatus(s->hasSelection);
}

bool KMapWidget::getSelectionStatus() const
{
    //return !s->selectionRectangle.isEmpty();
    return s->hasSelection;
}

void KMapWidget::setSelectionCoordinates(const GeoCoordinates::Pair& sel)
{
    if (s->currentMouseMode == MouseModeSelection || s->hasSelection)
        d->currentBackend->setSelectionRectangle(sel);
    else
        d->currentBackend->removeSelectionRectangle();
    s->selectionRectangle = sel;
}

void KMapWidget::clearSelectionRectangle()
{
    s->selectionRectangle.first.clear();
}

void KMapWidget::slotNewSelectionFromMap(const KMap::GeoCoordinates::Pair& sel)
{
    s->selectionRectangle      = sel;
    emit signalNewSelectionFromMap();
}

void KMapWidget::slotSetPanMode()
{
    if (d->actionSetPanMode->isChecked())
    {
        s->currentMouseMode = MouseModePan;
        d->actionSetSelectionMode->setChecked(false);
        d->actionSetZoomMode->setChecked(false);
        d->actionSetFilterDatabaseMode->setChecked(false);
        d->actionSetFilterModelMode->setChecked(false);
        d->actionSetSelectThumbnailMode->setChecked(false);

        if (d->currentBackend)
        {
            d->currentBackend->mouseModeChanged(MouseModePan);

            if (!s->hasSelection)
                d->currentBackend->removeSelectionRectangle();
        }
        emit signalMouseModeChanged(MouseModePan);
    }
    else
    {
        if (s->currentMouseMode == MouseModePan)
            d->actionSetPanMode->setChecked(true);
    }
}

void KMapWidget::slotSetSelectionMode()
{
    if (d->actionSetSelectionMode->isChecked())
    {
        s->currentMouseMode = MouseModeSelection;
        d->actionSetPanMode->setChecked(false);
        d->actionSetZoomMode->setChecked(false);
        d->actionSetFilterDatabaseMode->setChecked(false);
        d->actionSetFilterModelMode->setChecked(false);
        d->actionSetSelectThumbnailMode->setChecked(false);

        d->currentBackend->mouseModeChanged(MouseModeSelection);
        emit signalMouseModeChanged(MouseModeSelection);
    }
    else
    {
        if (s->currentMouseMode == MouseModeSelection)
            d->actionSetSelectionMode->setChecked(true);
    }
}


void KMapWidget::slotSetZoomMode()
{
    if (d->actionSetZoomMode->isChecked())
    {
        s->currentMouseMode = MouseModeZoom;
        d->actionSetPanMode->setChecked(false);
        d->actionSetSelectionMode->setChecked(false);
        d->actionSetFilterDatabaseMode->setChecked(false);
        d->actionSetFilterModelMode->setChecked(false);
        d->actionSetSelectThumbnailMode->setChecked(false);

        d->currentBackend->mouseModeChanged(MouseModeZoom);
        emit signalMouseModeChanged(MouseModeZoom);
    }
    else
    {
        if (s->currentMouseMode == MouseModeZoom)
            d->actionSetZoomMode->setChecked(true);
    }
}

void KMapWidget::slotSetFilterDatabaseMode()
{
    if (d->actionSetFilterDatabaseMode->isChecked())
    {
        s->currentMouseMode = MouseModeSelectionFromIcon;
        d->actionSetPanMode->setChecked(false);
        d->actionSetSelectionMode->setChecked(false);
        d->actionSetZoomMode->setChecked(false);
        d->actionSetSelectThumbnailMode->setChecked(false);
        d->actionSetFilterModelMode->setChecked(false);

        d->currentBackend->mouseModeChanged(MouseModeSelectionFromIcon);
        emit signalMouseModeChanged(MouseModeSelectionFromIcon);
    }
    else
    {
        if (s->currentMouseMode == MouseModeSelectionFromIcon)
        {
            d->actionSetFilterDatabaseMode->setChecked(true);
        }
    }
}

void KMapWidget::slotSetFilterModelMode()
{
    if (d->actionSetFilterModelMode->isChecked())
    {
        s->currentMouseMode = MouseModeFilter;
        d->actionSetPanMode->setChecked(false);
        d->actionSetSelectionMode->setChecked(false);
        d->actionSetZoomMode->setChecked(false);
        d->actionSetSelectThumbnailMode->setChecked(false);
        d->actionSetFilterDatabaseMode->setChecked(false);

        d->currentBackend->mouseModeChanged(MouseModeFilter);
        emit signalMouseModeChanged(MouseModeFilter);
    }
    else
    {
        if (s->currentMouseMode == MouseModeFilter)
        {
            d->actionSetFilterModelMode->setChecked(true);
        }
    }
}

void KMapWidget::slotSetSelectThumbnailMode()
{
    if (d->actionSetSelectThumbnailMode->isChecked())
    {
        s->currentMouseMode = MouseModeSelectThumbnail;
        d->actionSetPanMode->setChecked(false);
        d->actionSetSelectionMode->setChecked(false);
        d->actionSetZoomMode->setChecked(false);
        d->actionSetFilterDatabaseMode->setChecked(false);
        d->actionSetFilterModelMode->setChecked(false);

        d->currentBackend->mouseModeChanged(MouseModeSelectThumbnail);
        emit signalMouseModeChanged(MouseModeSelectThumbnail);
    }
    else
    {
        if (s->currentMouseMode == MouseModeSelectThumbnail)
            d->actionSetSelectThumbnailMode->setChecked(true);
    }
}

void KMapWidget::slotRemoveCurrentSelection()
{
    emit signalRemoveCurrentSelection();
    clearSelectionRectangle();
    d->currentBackend->removeSelectionRectangle();
}

void KMapWidget::slotRemoveCurrentFilter()
{
    if (s->modelBasedFilter)
    {
         emit signalRemoveCurrentFilter();
         s->modelBasedFilter = false;
    }
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

// Static methods ---------------------------------------------------------

QString KMapWidget::MarbleWidgetVersion()
{
    return QString(Marble::MARBLE_VERSION_STRING);
}

QString KMapWidget::version()
{
    return QString::fromLatin1(kmap_version);
}

void KMapWidget::setActive(const bool state)
{
    const bool oldState = s->activeState;
    s->activeState = state;

    if (state)
    {
        if (s->currentMouseMode != MouseModeSelection && !s->hasSelection)
        {
            //d->currentBackend->removeSelectionRectangle();
        }
    }

    if (d->currentBackend)
    {
        d->currentBackend->setActive(state);
    }
    if (s->markerModel)
    {
        s->markerModel->setActive(state);
    }

    if (state)
    {
        // do we have a map widget shown?
        if ( (d->stackedLayout->count()==1) && d->currentBackend )
        {
            setMapWidgetInFrame(d->currentBackend->mapWidget());

            // call this slot manually in case the backend was ready right away:
            if (d->currentBackend->isReady())
            {
                slotBackendReady(d->currentBackendName);
            }
            else
            {
                rebuildConfigurationMenu();
            }
        }
    }
    
    if (state && !oldState && d->clustersDirty)
    {
        slotRequestLazyReclustering();
    }
}

bool KMapWidget::getActiveState()
{
    return s->activeState;
}

void KMapWidget::setVisibleMouseModes(const MouseModes mouseModes)
{
    s->visibleMouseModes = mouseModes;

    if (d->mouseModesHolder)
    {
        d->mouseModesHolder->setVisible(s->visibleMouseModes);

        d->setSelectionModeButton->setVisible(s->visibleMouseModes.testFlag(MouseModeSelection));
        d->removeCurrentSelectionButton->setVisible(s->visibleMouseModes.testFlag(MouseModeSelection));
        d->setPanModeButton->setVisible(s->visibleMouseModes.testFlag(MouseModePan));
        d->setZoomModeButton->setVisible(s->visibleMouseModes.testFlag(MouseModeZoom));
        d->setFilterDatabaseModeButton->setVisible(s->visibleMouseModes.testFlag(MouseModeSelectionFromIcon));
        d->setFilterModelModeButton->setVisible(s->visibleMouseModes.testFlag(MouseModeFilter));
        d->removeFilterModeButton->setVisible(s->visibleMouseModes.testFlag(MouseModeSelectionFromIcon));
        d->setSelectThumbnailMode->setVisible(s->visibleMouseModes.testFlag(MouseModeSelectThumbnail));
    }
}

void KMapWidget::setAvailableMouseModes(const MouseModes mouseModes)
{
    s->availableMouseModes = mouseModes;
}

bool KMapWidget::getStickyModeState() const
{
    return d->actionStickyMode->isChecked();
}

void KMapWidget::setStickyModeState(const bool state)
{
    d->actionStickyMode->setChecked(state);
    slotUpdateActionsEnabled();
}

void KMapWidget::setVisibleExtraActions(const ExtraActions actions)
{
    d->visibleExtraActions = actions;

    if (d->buttonStickyMode)
    {
        d->buttonStickyMode->setVisible(actions.testFlag(ExtraActionSticky));
    }

    slotUpdateActionsEnabled();
}

void KMapWidget::setEnabledExtraActions(const ExtraActions actions)
{
    d->availableExtraActions = actions;

    slotUpdateActionsEnabled();
}

void KMapWidget::slotStickyModeChanged()
{
    slotUpdateActionsEnabled();
    emit(signalStickyModeChanged());
}

void KMapWidget::setAllowModifications(const bool state)
{
    s->modificationsAllowed = state;

    slotUpdateActionsEnabled();
    slotRequestLazyReclustering();
}

/**
 * @brief Adjusts the visible map area such that all grouped markers are visible.
 *
 * Note that a call to this function currently has no effect if the widget has been
 * set inactive via setActive() or the backend is not yet ready.
 *
 * @param useSaneZoomLevel Stop zooming at a sane level, if markers are too close together.
 */
void KMapWidget::adjustBoundariesToGroupedMarkers(const bool useSaneZoomLevel)
{
    if ( (!s->activeState) || (!s->markerModel) || (!d->currentBackend) )
        return;

    Marble::GeoDataLineString tileString;

    // TODO: not sure that this is the best way to find the bounding box of all items
    for (AbstractMarkerTiler::NonEmptyIterator tileIterator(s->markerModel, AbstractMarkerTiler::TileIndex::MaxLevel); !tileIterator.atEnd(); tileIterator.nextIndex())
    {
        const AbstractMarkerTiler::TileIndex tileIndex = tileIterator.currentIndex();
        for(int corner=1; corner<=4; corner++)
        {
            GeoCoordinates currentTileCoordinate = tileIndex.toCoordinates();

            const Marble::GeoDataCoordinates tileCoordinate(currentTileCoordinate.lon(),
                                                            currentTileCoordinate.lat(),
                                                            0,
                                                            Marble::GeoDataCoordinates::Degree);

            tileString.append(tileCoordinate);
        }
    }

    Marble::GeoDataLatLonBox latLonBox = Marble::GeoDataLatLonBox::fromLineString(tileString);

    // TODO: use a sane zoom level
    d->currentBackend->centerOn(latLonBox, useSaneZoomLevel);
}

void KMapWidget::refreshMap()
{
    slotRequestLazyReclustering();
}

void KMapWidget::setShowPlaceholderWidget(const bool state)
{
    if (state)
    {
        d->stackedLayout->setCurrentIndex(0);
    }
    else
    {
        if (d->stackedLayout->count()>1)
        {
            d->stackedLayout->setCurrentIndex(1);
        }
    }
}

/**
 * @brief Set @p widgetForFrame as the widget in the frame, but does not show it.
 */
void KMapWidget::setMapWidgetInFrame(QWidget* const widgetForFrame)
{
    if (d->stackedLayout->count()>1)
    {
        // widget 0 is the status widget, widget 1 is the map widget
        if (d->stackedLayout->widget(1)==widgetForFrame)
        {
            return;
        }

        // there is some other widget at the target position.
        // remove it and add our widget instead
        d->stackedLayout->removeWidget(d->stackedLayout->widget(1));
    }

    d->stackedLayout->addWidget(widgetForFrame);
}

void KMapWidget::removeMapWidgetFromFrame()
{
    if (d->stackedLayout->count()>1)
    {
        d->stackedLayout->removeWidget(d->stackedLayout->widget(1));
    }

    d->stackedLayout->setCurrentIndex(0);
}

} /* namespace KMap */
