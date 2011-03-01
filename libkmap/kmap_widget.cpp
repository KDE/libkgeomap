/** ===========================================================
 * @file
 *
 * This file is a part of digiKam project
 * <a href="http://www.digikam.org">http://www.digikam.org</a>
 *
 * @date   2009-12-01
 * @brief  world map widget library
 *
 * @author Copyright (C) 2009-2011 by Michael G. Hansen
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

#include <marble/GeoDataLineString.h>
#include <marble/global.h>

// local includes

#include "abstractmarkertiler.h"
#include "backend_altitude_geonames.h"
#include "backend_map_googlemaps.h"
#include "backend_map_marble.h"
// #include "backend_map_osm.h"
#include "kmap_common.h"
#include "dragdrophandler.h"
#include "modelhelper.h"
#include "placeholderwidget.h"
#include "tilegrouper.h"
#include "version.h"

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

class KMapWidget::KMapWidgetPrivate
{
public:

    KMapWidgetPrivate()
      : loadedAltitudeBackends(),
        loadedBackends(),
        currentBackend(0),
        currentBackendName(),
        stackedLayout(0),
        cacheCenterCoordinate(52.0,6.0),
        cacheZoom(QLatin1String("marble:900")),
        configurationMenu(0),
        actionGroupBackendSelection(0),
        actionZoomIn(0),
        actionZoomOut(0),
        actionShowThumbnails(0),
        mouseModesHolder(0),
        controlWidget(0),
        lazyReclusteringRequested(false),
        dragDropHandler(0),
        sortMenu(0),
        actionIncreaseThumbnailSize(0),
        actionDecreaseThumbnailSize(0),
        actionRemoveCurrentRegionSelection(0),
        actionSetRegionSelectionMode(0),
        actionSetPanMode(0),
        actionSetZoomIntoGroupMode(0),
        actionSetRegionSelectionFromIconMode(0),
        actionSetFilterMode(0),
        actionRemoveFilter(0),
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

    DragDropHandler*        dragDropHandler;

    QMenu*                  sortMenu;
    KAction*                actionIncreaseThumbnailSize;
    KAction*                actionDecreaseThumbnailSize;
    KHBox*                  hBoxForAdditionalControlWidgetItems;

    QActionGroup*           mouseModeActionGroup;
    KAction*                actionRemoveCurrentRegionSelection;
    KAction*                actionSetRegionSelectionMode;
    KAction*                actionSetPanMode;
    KAction*                actionSetZoomIntoGroupMode;
    KAction*                actionSetRegionSelectionFromIconMode;
    KAction*                actionSetFilterMode;
    KAction*                actionRemoveFilter;
    KAction*                actionSetSelectThumbnailMode;
    QToolButton*            setPanModeButton;
    QToolButton*            setSelectionModeButton;
    QToolButton*            removeCurrentSelectionButton;
    QToolButton*            setZoomModeButton;
    QToolButton*            setRegionSelectionFromIconModeButton;
    QToolButton*            setFilterModeButton;
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

    s->tileGrouper = new TileGrouper(s, this);

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
    d->actionZoomIn->setIcon(SmallIcon( QLatin1String("zoom-in") ));
    d->actionZoomIn->setToolTip(i18n("Zoom in"));
    connect(d->actionZoomIn, SIGNAL(triggered()),
            this, SLOT(slotZoomIn()));

    d->actionZoomOut = new KAction(this);
    d->actionZoomOut->setIcon(SmallIcon( QLatin1String("zoom-out") ));
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

    d->actionRemoveCurrentRegionSelection = new KAction(this);
    //d->actionRemoveCurrentRegionSelection->setEnabled(false);
    d->actionRemoveCurrentRegionSelection->setIcon(SmallIcon( QLatin1String("edit-clear") ));
    d->actionRemoveCurrentRegionSelection->setToolTip(i18n("Remove the current region selection"));

    d->mouseModeActionGroup = new QActionGroup(this);
    d->mouseModeActionGroup->setExclusive(true);

    d->actionSetRegionSelectionMode = new KAction(d->mouseModeActionGroup);
    d->actionSetRegionSelectionMode->setCheckable(true);
    d->actionSetRegionSelectionMode->setIcon(SmallIcon( QLatin1String("select-rectangular") ));
    d->actionSetRegionSelectionMode->setToolTip(i18n("Select images by drawing a rectangle"));
    d->actionSetRegionSelectionMode->setData(QVariant::fromValue<KMap::MouseModes>(MouseModeRegionSelection));

    d->actionSetPanMode = new KAction(d->mouseModeActionGroup);
    d->actionSetPanMode->setCheckable(true);
    d->actionSetPanMode->setToolTip(i18n("Pan mode"));
    d->actionSetPanMode->setIcon(SmallIcon( QLatin1String("transform-move") ));
    d->actionSetPanMode->setChecked(true);
    d->actionSetPanMode->setData(QVariant::fromValue<KMap::MouseModes>(MouseModePan));

    d->actionSetZoomIntoGroupMode = new KAction(d->mouseModeActionGroup);
    d->actionSetZoomIntoGroupMode->setCheckable(true);
    d->actionSetZoomIntoGroupMode->setToolTip(i18n("Zoom into a group"));
    d->actionSetZoomIntoGroupMode->setIcon(SmallIcon( QLatin1String("page-zoom") ));
    d->actionSetZoomIntoGroupMode->setData(QVariant::fromValue<KMap::MouseModes>(MouseModeZoomIntoGroup));

    d->actionSetRegionSelectionFromIconMode = new KAction(d->mouseModeActionGroup);
    d->actionSetRegionSelectionFromIconMode->setCheckable(true);
    d->actionSetRegionSelectionFromIconMode->setToolTip(i18n("Create a region selection from a thumbnail"));
    d->actionSetRegionSelectionFromIconMode->setIcon(SmallIcon( QLatin1String("edit-node") ));
    d->actionSetRegionSelectionFromIconMode->setData(QVariant::fromValue<KMap::MouseModes>(MouseModeRegionSelectionFromIcon));

    d->actionSetFilterMode = new KAction(d->mouseModeActionGroup);
    d->actionSetFilterMode->setCheckable(true);
    d->actionSetFilterMode->setToolTip(i18n("Filter images"));
    d->actionSetFilterMode->setIcon(SmallIcon( QLatin1String("view-filter") ));
    d->actionSetFilterMode->setData(QVariant::fromValue<KMap::MouseModes>(MouseModeFilter));

    d->actionRemoveFilter = new KAction(this);
    d->actionRemoveFilter->setToolTip(i18n("Remove the current filter"));
    d->actionRemoveFilter->setIcon(SmallIcon( QLatin1String("window-close") ));

    d->actionSetSelectThumbnailMode = new KAction(d->mouseModeActionGroup);
    d->actionSetSelectThumbnailMode->setCheckable(true);
    d->actionSetSelectThumbnailMode->setToolTip(i18n("Select images"));
    d->actionSetSelectThumbnailMode->setIcon(SmallIcon( QLatin1String("edit-select") ));
    d->actionSetSelectThumbnailMode->setData(QVariant::fromValue<KMap::MouseModes>(MouseModeSelectThumbnail));

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

    connect(d->mouseModeActionGroup, SIGNAL(triggered(QAction*)),
            this, SLOT(slotMouseModeChanged(QAction*)));

    connect(d->actionRemoveFilter, SIGNAL(triggered()),
            this, SIGNAL(signalRemoveCurrentFilter()));

    connect(d->actionRemoveCurrentRegionSelection, SIGNAL(triggered()),
            this, SLOT(slotRemoveCurrentRegionSelection()));

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
    {
        return true;
    }

    saveBackendToCache();

    // switch to the placeholder widget:
    setShowPlaceholderWidget(true);
    removeMapWidgetFromFrame();

    // disconnect signals from old backend:
    if (d->currentBackend)
    {
        d->currentBackend->setActive(false);

        disconnect(d->currentBackend, SIGNAL(signalBackendReadyChanged(const QString&)),
                this, SLOT(slotBackendReadyChanged(const QString&)));

        disconnect(d->currentBackend, SIGNAL(signalZoomChanged(const QString&)),
                this, SLOT(slotBackendZoomChanged(const QString&)));

        disconnect(d->currentBackend, SIGNAL(signalClustersMoved(const QIntList&, const QPair<int, QModelIndex>&)),
                this, SLOT(slotClustersMoved(const QIntList&, const QPair<int, QModelIndex>&)));

        disconnect(d->currentBackend, SIGNAL(signalClustersClicked(const QIntList&)),
                    this, SLOT(slotClustersClicked(const QIntList&)));

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

            connect(d->currentBackend, SIGNAL(signalBackendReadyChanged(const QString&)),
                    this, SLOT(slotBackendReadyChanged(const QString&)));

            connect(d->currentBackend, SIGNAL(signalZoomChanged(const QString&)),
                    this, SLOT(slotBackendZoomChanged(const QString&)));

            connect(d->currentBackend, SIGNAL(signalClustersMoved(const QIntList&, const QPair<int, QModelIndex>&)),
                    this, SLOT(slotClustersMoved(const QIntList&, const QPair<int, QModelIndex>&)));

            connect(d->currentBackend, SIGNAL(signalClustersClicked(const QIntList&)),
                    this, SLOT(slotClustersClicked(const QIntList&)));

            /**
             * @todo This connection is queued because otherwise QAbstractItemModel::itemSelected does not
             *       reflect the true state. Maybe monitor another signal instead?
             */
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
                    slotBackendReadyChanged(d->currentBackendName);
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
    if ( (!currentBackendReady()) || (!s->activeState) )
    {
        return;
    }

    setCenter(d->cacheCenterCoordinate);
    /// @todo Only do this if the zoom was changed!
    kDebug()<<d->cacheZoom;
    setZoom(d->cacheZoom);
    d->currentBackend->mouseModeChanged();
    d->currentBackend->regionSelectionChanged();
}

void KMapWidget::saveBackendToCache()
{
    if (!currentBackendReady())
    {
        return;
    }

    d->cacheCenterCoordinate   = getCenter();
    d->cacheZoom               = getZoom();
}

GeoCoordinates KMapWidget::getCenter() const
{
    if (!currentBackendReady())
    {
        return d->cacheCenterCoordinate;
    }

    return d->currentBackend->getCenter();
}

void KMapWidget::setCenter(const GeoCoordinates& coordinate)
{
    d->cacheCenterCoordinate = coordinate;

    if (!currentBackendReady())
    {
        return;
    }

    d->currentBackend->setCenter(coordinate);
}

void KMapWidget::slotBackendReadyChanged(const QString& backendName)
{
    kDebug()<<QString::fromLatin1("backend %1 is ready!").arg(backendName);
    if (backendName != d->currentBackendName)
    {
        return;
    }

    if (!currentBackendReady())
    {
        return;
    }

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
    group->writeEntry("Thumbnail Size", s->thumbnailSize);
    group->writeEntry("Thumbnail Grouping Radius", s->thumbnailGroupingRadius);
    group->writeEntry("Marker Grouping Radius", s->markerGroupingRadius);
    group->writeEntry("Show Thumbnails", s->showThumbnails);
    group->writeEntry("Mouse Mode", int(s->currentMouseMode));

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
    {
        return;
    }

    setBackend(group->readEntry("Backend", "marble"));

    // Options concerning the display of markers
    d->actionPreviewSingleItems->setChecked(group->readEntry("Preview Single Items", true));
    d->actionPreviewGroupedItems->setChecked(group->readEntry("Preview Grouped Items", true));
    d->actionShowNumbersOnItems->setChecked(group->readEntry("Show numbers on items", true));

    setThumnailSize(group->readEntry("Thumbnail Size", 2*KMapMinThumbnailSize));
    setThumbnailGroupingRadius(group->readEntry("Thumbnail Grouping Radius", 2*KMapMinThumbnailGroupingRadius));
    setMarkerGroupingRadius(group->readEntry("Edit Grouping Radius", KMapMinMarkerGroupingRadius));
    s->showThumbnails = group->readEntry("Show Thumbnails", s->showThumbnails);
    d->actionShowThumbnails->setChecked(s->showThumbnails);
    d->actionStickyMode->setChecked(group->readEntry("Sticky Mode State", d->actionStickyMode->isChecked()));

    // let the backends load their settings
    for (int i=0; i<d->loadedBackends.size(); ++i)
    {
        d->loadedBackends.at(i)->readSettingsFromGroup(group);
    }

    // current map state
    const GeoCoordinates centerDefault = GeoCoordinates(52.0, 6.0);
    const QString centerGeoUrl = group->readEntry("Center", centerDefault.geoUrl());
    bool centerGeoUrlValid = false;
    const GeoCoordinates centerCoordinate = GeoCoordinates::fromGeoUrl(centerGeoUrl, &centerGeoUrlValid);
    d->cacheCenterCoordinate = centerGeoUrlValid ? centerCoordinate : centerDefault;
    d->cacheZoom = group->readEntry("Zoom", d->cacheZoom);
    s->currentMouseMode = MouseModes(group->readEntry("Mouse Mode", int(s->currentMouseMode)));

    // propagate the loaded values to the map, if appropriate
    applyCacheToBackend();
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

    if (currentBackendReady())
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

    slotUpdateActionsEnabled();
}

KAction* KMapWidget::getControlAction(const QString& actionName)
{
    if (actionName==QLatin1String("zoomin"))
    {
        return d->actionZoomIn;
    }
    else if (actionName==QLatin1String("zoomout"))
    {
        return d->actionZoomOut;
    }
    else if (actionName==QLatin1String("mousemode-regionselectionmode"))
    {
        return d->actionSetRegionSelectionMode;
    }
    else if (actionName==QLatin1String("mousemode-removecurrentregionselection"))
    {
        return d->actionRemoveCurrentRegionSelection;
    }
    else if (actionName==QLatin1String("mousemode-regionselectionfromiconmode"))
    {
        return d->actionSetRegionSelectionFromIconMode;
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
        configurationButton->setIcon(SmallIcon( QLatin1String("applications-internet") ));
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
        d->setSelectionModeButton->setDefaultAction(d->actionSetRegionSelectionMode);

        d->setRegionSelectionFromIconModeButton = new QToolButton(d->mouseModesHolder);
        d->setRegionSelectionFromIconModeButton->setDefaultAction(d->actionSetRegionSelectionFromIconMode);

        d->removeCurrentSelectionButton = new QToolButton(d->mouseModesHolder);
        d->removeCurrentSelectionButton->setDefaultAction(d->actionRemoveCurrentRegionSelection);

        d->setZoomModeButton = new QToolButton(d->mouseModesHolder);
        d->setZoomModeButton->setDefaultAction(d->actionSetZoomIntoGroupMode);

        d->setFilterModeButton = new QToolButton(d->mouseModesHolder);
        d->setFilterModeButton->setDefaultAction(d->actionSetFilterMode);

        d->removeFilterModeButton = new QToolButton(d->mouseModesHolder);
        d->removeFilterModeButton->setDefaultAction(d->actionRemoveFilter);

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
    if (!currentBackendReady())
        return;

    d->currentBackend->zoomIn();
}

void KMapWidget::slotZoomOut()
{
    if (!currentBackendReady())
        return;

    d->currentBackend->zoomOut();
}

void KMapWidget::slotUpdateActionsEnabled()
{
    if (!s->activeState)
    {
        // this widget is not active, no need to update the action availability
        return;
    }

    d->actionDecreaseThumbnailSize->setEnabled((s->showThumbnails)&&(s->thumbnailSize>KMapMinThumbnailSize));
    /// @todo Define an upper limit for the thumbnail size!
    d->actionIncreaseThumbnailSize->setEnabled(s->showThumbnails);

    d->actionSetRegionSelectionMode->setEnabled(s->availableMouseModes.testFlag(MouseModeRegionSelection));
    d->actionRemoveCurrentRegionSelection->setEnabled(s->availableMouseModes.testFlag(MouseModeRegionSelection));
    d->actionSetPanMode->setEnabled(s->availableMouseModes.testFlag(MouseModePan));
    d->actionSetZoomIntoGroupMode->setEnabled(s->availableMouseModes.testFlag(MouseModeZoomIntoGroup));
    d->actionSetRegionSelectionFromIconMode->setEnabled(s->availableMouseModes.testFlag(MouseModeRegionSelectionFromIcon));
    d->actionSetFilterMode->setEnabled(s->availableMouseModes.testFlag(MouseModeFilter));
    d->actionRemoveFilter->setEnabled(s->availableMouseModes.testFlag(MouseModeRegionSelectionFromIcon));
    d->actionSetSelectThumbnailMode->setEnabled(s->availableMouseModes.testFlag(MouseModeSelectThumbnail));

    d->actionStickyMode->setEnabled(d->availableExtraActions.testFlag(ExtraActionSticky));

    /// @todo Only set the icons if they have to be changed!
    d->actionStickyMode->setIcon(SmallIcon( QLatin1String( d->actionStickyMode->isChecked()?"object-locked":"object-unlocked" )));
    d->actionShowThumbnails->setIcon(d->actionShowThumbnails->isChecked()?SmallIcon( QLatin1String("folder-image") ):KMapGlobalObject::instance()->getMarkerPixmap(QLatin1String("marker-icon-16x16" )));

    // make sure the action for the current mouse mode is checked
    const QList<QAction*> mouseModeActions = d->mouseModeActionGroup->actions();
    foreach (QAction* const action, mouseModeActions)
    {
        if (action->data().value<MouseModes>()==s->currentMouseMode)
        {
            action->setChecked(true);
            break;
        }
    }
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
    if (!currentBackendReady())
    {
        return;
    }

    // tell the backend to update the markers
    d->currentBackend->updateMarkers();
}

void KMapWidget::updateClusters()
{
    /// @todo Find a better way to tell the TileGrouper about the backend
    s->tileGrouper->setCurrentBackend(d->currentBackend);
    s->tileGrouper->updateClusters();
}

void KMapWidget::slotClustersNeedUpdating()
{
    if (currentBackendReady())
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
                                    const KMapGroupState* const overrideSelection,
                                    const int* const overrideCount) const
{
    /// @todo Call the new getColorInfos function!
    const KMapCluster& cluster = s->clusterList.at(clusterIndex);

    /// @todo Check that this number is already valid!
    const int nMarkers = overrideCount ? *overrideCount : cluster.markerCount;

    getColorInfos(overrideSelection?*overrideSelection:cluster.groupState,
                  nMarkers,
                  fillColor, strokeColor, strokeStyle, labelText, labelColor);
}

void KMapWidget::getColorInfos(const KMapGroupState groupState,
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
        /// @todo Use KDE-versions instead
        *labelText = QString::fromLatin1("%L1k").arg(qreal(nMarkers)/1000.0, 0, 'f', 1);
    }
    else if ((nMarkers>=1951)&&(nMarkers<19500))
    {
        /// @todo Use KDE-versions instead
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

    *strokeStyle = Qt::NoPen;
    /// @todo On my system, digikam uses QColor(67, 172, 232) as the selection color. Or should we just use blue?
    switch (groupState & KMapSelectedMask)
    {
        case KMapSelectedNone:
            *strokeStyle = Qt::SolidLine;
            *strokeColor = QColor(Qt::black);
            break;
        case KMapSelectedSome:
            *strokeStyle = Qt::DotLine;
            *strokeColor = QColor(Qt::blue);//67, 172, 232);
            break;
        case KMapSelectedAll:
            *strokeStyle = Qt::SolidLine;
            *strokeColor = QColor(Qt::blue);//67, 172, 232);
            break;
    }

    /// @todo These are the fill colors for the circles, for cases in which only some or all of the images are positively filtered. Filtering is implemented in libkmap, but the code here has not been adapted yet.
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
//     switch (groupState)
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
        else { targetZoom = 3500; } /// @todo Find values for level 20 and up
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
        else { targetZoom = 20; } /// @todo Find values for level 20 and up
    }

    KMAP_ASSERT(targetZoom>=0);

    return QString::fromLatin1("%1:%2").arg(targetBackend).arg(targetZoom);
}

void KMapWidget::slotBackendZoomChanged(const QString& newZoom)
{
    d->cacheZoom = newZoom;
}

void KMapWidget::setZoom(const QString& newZoom)
{
    d->cacheZoom = newZoom;

    if (currentBackendReady())
    {
        d->currentBackend->setZoom(d->cacheZoom);
    }
}

QString KMapWidget::getZoom()
{
    if (currentBackendReady())
    {
        d->cacheZoom = d->currentBackend->getZoom();
    }

    return d->cacheZoom;
}

GeoCoordinates::Pair KMapWidget::getRegionSelection()
{
    return s->selectionRectangle;
}

void KMapWidget::slotClustersMoved(const QIntList& clusterIndices, const QPair<int, QModelIndex>& snapTarget)
{
    kDebug()<<clusterIndices;

    /// @todo We actually expect only one clusterindex
    int clusterIndex = clusterIndices.first();
    GeoCoordinates targetCoordinates = s->clusterList.at(clusterIndex).coordinates;

    TileIndex::List movedTileIndices;
    if (s->clusterList.at(clusterIndex).groupState==KMapSelectedNone)
    {
        // a not-selected marker was moved. update all of its items:
        const KMapCluster& cluster = s->clusterList.at(clusterIndex);
        for (int i=0; i<cluster.tileIndicesList.count(); ++i)
        {
            const TileIndex tileIndex = cluster.tileIndicesList.at(i);

            movedTileIndices <<tileIndex;
        }
    }
    else
    {
        // selected items were moved. The model helper should know which tiles are selected,
        // therefore we give him an empty list
    }

    s->markerModel->onIndicesMoved(movedTileIndices, targetCoordinates, snapTarget.second);

    /**
     * @todo Clusters are marked as dirty by slotClustersNeedUpdating
     * which is called while we update the model
     */
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

void KMapWidget::setShowThumbnails(const bool state)
{
    s->showThumbnails = state;

    rebuildConfigurationMenu();
    slotUpdateActionsEnabled();
    slotRequestLazyReclustering();
}

void KMapWidget::slotShowThumbnailsChanged()
{
    setShowThumbnails(d->actionShowThumbnails->isChecked());
}

/**
 * @brief Request reclustering, repeated calls should generate only one actual update of the clusters
 */
void KMapWidget::slotRequestLazyReclustering()
{
    if (d->lazyReclusteringRequested)
        return;

    s->tileGrouper->setClustersDirty();
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

/**
 * @todo Clicking on several clusters at once is not actually possible
 */
void KMapWidget::slotClustersClicked(const QIntList& clusterIndices)
{
    kDebug()<<clusterIndices;

    if (   (s->currentMouseMode == MouseModeZoomIntoGroup)
        || (s->currentMouseMode == MouseModeRegionSelectionFromIcon) )
    {
        int maxTileLevel = 0;

        Marble::GeoDataLineString tileString;

        for (int i=0; i<clusterIndices.count(); ++i)
        {
            const int clusterIndex = clusterIndices.at(i);
            const KMapCluster currentCluster = s->clusterList.at(clusterIndex);

            for (int j=0; j<currentCluster.tileIndicesList.count(); ++j)
            {
                const TileIndex& currentTileIndex = currentCluster.tileIndicesList.at(j);
                for (int corner=1; corner<=4; ++corner)
                {
                    GeoCoordinates currentTileCoordinate;

                    if (corner == 1)
                        currentTileCoordinate = currentTileIndex.toCoordinates(TileIndex::CornerNW);
                    else if (corner == 2)
                        currentTileCoordinate = currentTileIndex.toCoordinates(TileIndex::CornerSW);
                    else if (corner == 3)
                        currentTileCoordinate = currentTileIndex.toCoordinates(TileIndex::CornerNE);
                    else if (corner == 4)
                        currentTileCoordinate = currentTileIndex.toCoordinates(TileIndex::CornerSE);

                    const Marble::GeoDataCoordinates tileCoordinate(currentTileCoordinate.lon(),
                                                                    currentTileCoordinate.lat(),
                                                                                              0,
                                                                    Marble::GeoDataCoordinates::Degree);

                    if (maxTileLevel < currentTileIndex.level())
                    {
                        maxTileLevel = currentTileIndex.level();
                    }

                    tileString.append(tileCoordinate);
                }
            }
        }

        Marble::GeoDataLatLonBox latLonBox = Marble::GeoDataLatLonBox::fromLineString(tileString);

        /// @todo Review this section
/*        if (maxTileLevel != 0)
        {
            //increase the selection boundaries with 0.1 degrees because some thumbnails aren't caught by selection
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
        if (s->currentMouseMode == MouseModeZoomIntoGroup)
        {
            /// @todo Very small latLonBoxes can crash Marble
            d->currentBackend->centerOn(latLonBox);
        }
        else
        {
            const GeoCoordinates::Pair newSelection(
                    GeoCoordinates(latLonBox.north(Marble::GeoDataCoordinates::Degree),
                                latLonBox.west(Marble::GeoDataCoordinates::Degree)),
                    GeoCoordinates(latLonBox.south(Marble::GeoDataCoordinates::Degree),
                                latLonBox.east(Marble::GeoDataCoordinates::Degree))
                );

            s->selectionRectangle = newSelection;
            d->currentBackend->regionSelectionChanged();
            emit(signalRegionSelectionChanged());
        }
    }
    else if (   (s->currentMouseMode == MouseModeFilter && s->selectionRectangle.first.hasCoordinates())
             || (s->currentMouseMode == MouseModeSelectThumbnail) )
    {
        // update the selection and filtering state of the clusters
        for (int i=0; i<clusterIndices.count(); ++i)
        {
            const int clusterIndex = clusterIndices.at(i);
            const KMapCluster currentCluster = s->clusterList.at(clusterIndex);

            const TileIndex::List tileIndices = currentCluster.tileIndicesList;

            /// @todo Isn't this cached in the cluster?
            const QVariant representativeIndex = getClusterRepresentativeMarker(clusterIndex, s->sortKey);

            s->markerModel->onIndicesClicked(tileIndices, representativeIndex,
                                             currentCluster.groupState, s->currentMouseMode);
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

void KMapWidget::dragMoveEvent(QDragMoveEvent* event)
{
    Q_UNUSED(event);

    /// @todo update the position of the drag marker if it is to be shown
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
    s->tileGrouper->setClustersDirty();
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
        repIndices << s->markerModel->getTileRepresentativeMarker(cluster.tileIndicesList.at(i), sortKey);
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

    /// @todo Update action availability?

    /// @todo We just need to update the display, no need to recluster?
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
    /// @todo We just need to update the display, no need to recluster?
    slotRequestLazyReclustering();
}

QPixmap KMapWidget::getDecoratedPixmapForCluster(const int clusterId, const KMapGroupState* const selectedStateOverride, const int* const countOverride, QPoint* const centerPoint)
{
    KMapCluster& cluster = s->clusterList[clusterId];

    int markerCount = cluster.markerCount;
    KMapGroupState groupState = cluster.groupState;
    if (selectedStateOverride)
    {
        groupState = *selectedStateOverride;
        markerCount = *countOverride;
    }

    const KMapGroupState selectedState = groupState & KMapSelectedMask;

    // first determine all the color and style values
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
        /// @todo Handle positive filtering and region selection!
        QString pixmapName = fillColor.name().mid(1);
        if (selectedState==KMapSelectedAll)
        {
            pixmapName+=QLatin1String("-selected");
        }
        if (selectedState==KMapSelectedSome)
        {
            pixmapName+=QLatin1String("-someselected");
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

    /// @todo This check is strange, there can be no clusters without a markerModel?
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

            // we may draw with partially transparent pixmaps later, so make sure we have a defined
            // background color
            resultPixmap.fill(QColor::fromRgb(0xff, 0xff, 0xff));
            QPainter painter(&resultPixmap);
//             painter.setRenderHint(QPainter::Antialiasing);

            const int borderWidth = (groupState&KMapSelectedSome) ? 2 : 1;
            QPen borderPen;
            borderPen.setWidth(borderWidth);
            borderPen.setJoinStyle(Qt::MiterJoin);

            KMapGroupState globalState = s->markerModel->getGlobalGroupState();

            /// @todo What about partially in the region or positively filtered?
            const bool clusterIsNotInRegionSelection =
                   (globalState & KMapRegionSelectedMask)
                && ( (groupState & KMapRegionSelectedMask) == KMapRegionSelectedNone );
            const bool clusterIsNotPositivelyFiltered =
                   (globalState & KMapFilteredPositiveMask)
                && ( (groupState & KMapFilteredPositiveMask) == KMapFilteredPositiveNone );

            const bool shouldGrayOut = clusterIsNotInRegionSelection || clusterIsNotPositivelyFiltered;
            const bool shouldCrossOut = clusterIsNotInRegionSelection;

            if (shouldGrayOut)
            {
                /// @todo Cache the alphaPixmap!
                QPixmap alphaPixmap(clusterPixmap.size());
                alphaPixmap.fill(QColor::fromRgb(0x80, 0x80, 0x80));
                clusterPixmap.setAlphaChannel(alphaPixmap);
            }

            painter.drawPixmap(QPoint(1,1), clusterPixmap);

            if (shouldGrayOut || shouldCrossOut)
            {
                // draw a red cross above the pixmap
                QPen crossPen(Qt::red);

                if (!shouldCrossOut)
                {
                    /// @todo Maybe we should also do a cross for not positively filtered images?
                    crossPen.setColor(Qt::blue);
                }

                crossPen.setWidth(2);
                painter.setPen(crossPen);

                const int width = resultPixmap.size().width();
                const int height = resultPixmap.size().height();
                painter.drawLine(0, 0, width-1, height-1);
                painter.drawLine(width-1, 0, 0, height-1);
            }

            
            if (strokeStyle!=Qt::SolidLine)
            {
                // paint a white border around the image
                borderPen.setColor(Qt::white);
                painter.setPen(borderPen);
                painter.drawRect(
                        borderWidth-1, borderWidth-1,
                        resultPixmap.size().width()-borderWidth, resultPixmap.size().height()-borderWidth
                    );
            }

            // now draw the selection border
            borderPen.setColor(strokeColor);
            borderPen.setStyle(strokeStyle);
            painter.setPen(borderPen);
            painter.drawRect(
                    borderWidth-1, borderWidth-1,
                    resultPixmap.size().width()-borderWidth, resultPixmap.size().height()-borderWidth
                );

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
    const int circleRadius = s->thumbnailSize/2;
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
    /// @todo cache this somehow
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
    s->thumbnailSize = qMax(KMapMinThumbnailSize, newThumbnailSize);

    // make sure the grouping radius is larger than the thumbnail size
    if (2*s->thumbnailGroupingRadius < newThumbnailSize)
    {
        /// @todo more straightforward way for this?
        s->thumbnailGroupingRadius = newThumbnailSize/2 + newThumbnailSize%2;
    }

    if (s->showThumbnails)
    {
        slotRequestLazyReclustering();
    }
    slotUpdateActionsEnabled();
}

void KMapWidget::setThumbnailGroupingRadius(const int newGroupingRadius)
{
    s->thumbnailGroupingRadius = qMax(KMapMinThumbnailGroupingRadius, newGroupingRadius);

    // make sure the thumbnails are smaller than the grouping radius
    if (2*s->thumbnailGroupingRadius < s->thumbnailSize)
    {
        s->thumbnailSize = 2*newGroupingRadius;
    }

    if (s->showThumbnails)
    {
        slotRequestLazyReclustering();
    }
    slotUpdateActionsEnabled();
}

void KMapWidget::setMarkerGroupingRadius(const int newGroupingRadius)
{
    s->markerGroupingRadius = qMax(KMapMinMarkerGroupingRadius, newGroupingRadius);

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

    if (s->thumbnailSize>KMapMinThumbnailSize)
    {
        const int newThumbnailSize = qMax(KMapMinThumbnailSize, s->thumbnailSize-5);

        // make sure the grouping radius is also decreased
        // this will automatically decrease the thumbnail size as well
        setThumbnailGroupingRadius(newThumbnailSize/2);
    }
}

void KMapWidget::slotIncreaseThumbnailSize()
{
    if (!s->showThumbnails)
        return;

    setThumnailSize(s->thumbnailSize+5);
}

int KMapWidget::getThumbnailSize() const
{
    return s->thumbnailSize;
}

int KMapWidget::getUndecoratedThumbnailSize() const
{
    return s->thumbnailSize-2;
}

void KMapWidget::setRegionSelection(const GeoCoordinates::Pair& region)
{
    s->selectionRectangle = region;

    d->currentBackend->regionSelectionChanged();
}

void KMapWidget::clearRegionSelection()
{
    s->selectionRectangle.first.clear();

    d->currentBackend->regionSelectionChanged();
}

void KMapWidget::slotNewSelectionFromMap(const KMap::GeoCoordinates::Pair& sel)
{
    /// @todo Should the backend update s on its own?
    s->selectionRectangle      = sel;
    emit(signalRegionSelectionChanged());
}

void KMapWidget::slotRemoveCurrentRegionSelection()
{
    clearRegionSelection();
    d->currentBackend->regionSelectionChanged();

    emit(signalRegionSelectionChanged());
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
                slotBackendReadyChanged(d->currentBackendName);
            }
            else
            {
                rebuildConfigurationMenu();
            }
        }
    }

    if (state && !oldState && s->tileGrouper->getClustersDirty())
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

        d->setSelectionModeButton->setVisible(s->visibleMouseModes.testFlag(MouseModeRegionSelection));
        d->removeCurrentSelectionButton->setVisible(s->visibleMouseModes.testFlag(MouseModeRegionSelection));
        d->setPanModeButton->setVisible(s->visibleMouseModes.testFlag(MouseModePan));
        d->setZoomModeButton->setVisible(s->visibleMouseModes.testFlag(MouseModeZoomIntoGroup));
        d->setRegionSelectionFromIconModeButton->setVisible(s->visibleMouseModes.testFlag(MouseModeRegionSelectionFromIcon));
        d->setFilterModeButton->setVisible(s->visibleMouseModes.testFlag(MouseModeFilter));
        d->removeFilterModeButton->setVisible(s->visibleMouseModes.testFlag(MouseModeFilter));
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
    if ( (!s->activeState) || (!s->markerModel) || (!currentBackendReady()) )
    {
        return;
    }

    Marble::GeoDataLineString tileString;

    /// @todo not sure that this is the best way to find the bounding box of all items
    for (AbstractMarkerTiler::NonEmptyIterator tileIterator(s->markerModel, TileIndex::MaxLevel); !tileIterator.atEnd(); tileIterator.nextIndex())
    {
        const TileIndex tileIndex = tileIterator.currentIndex();
        for(int corner=1; corner<=4; corner++)
        {
            const GeoCoordinates currentTileCoordinate = tileIndex.toCoordinates(TileIndex::CornerPosition(corner));

            const Marble::GeoDataCoordinates tileCoordinate(currentTileCoordinate.lon(),
                                                            currentTileCoordinate.lat(),
                                                            0,
                                                            Marble::GeoDataCoordinates::Degree);

            tileString.append(tileCoordinate);
        }
    }

    const Marble::GeoDataLatLonBox latLonBox = Marble::GeoDataLatLonBox::fromLineString(tileString);

    /// @todo use a sane zoom level
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

void KMapWidget::slotMouseModeChanged(QAction* triggeredAction)
{
    // determine the new mouse mode:
    const QVariant triggeredActionData = triggeredAction->data();
    const MouseModes newMouseMode = triggeredActionData.value<KMap::MouseModes>();

    if (newMouseMode==s->currentMouseMode)
    {
        return;
    }

    // store the new mouse mode:
    s->currentMouseMode = newMouseMode;

    if (d->currentBackend)
    {
        d->currentBackend->mouseModeChanged();
    }

    emit(signalMouseModeChanged(s->currentMouseMode));

    /// @todo Update action availability?
}

bool KMapWidget::currentBackendReady() const
{
    if (!d->currentBackend)
    {
        return false;
    }

    return d->currentBackend->isReady();
}

void KMapWidget::setMouseMode(const MouseModes mouseMode)
{
    s->currentMouseMode = mouseMode;

    if (currentBackendReady())
    {
        d->currentBackend->mouseModeChanged();
    }

    slotUpdateActionsEnabled();
}

} /* namespace KMap */
