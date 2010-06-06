/* ============================================================
 *
 * Date        : 2009-12-01
 * Description : WorldMapWidget2
 *
 * Copyright (C) 2009,2010 by Michael G. Hansen <mike at mghansen dot de>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#include "worldmapwidget2.moc"

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

// local includes

#include "map-backend.h"
#include "backend-marble.h"
#include "backend-googlemaps.h"
// #include "backend-osm.h"
#include "markermodel.h"
#include "backend-altitude-geonames.h"
#include "worldmapwidget2_dragdrophandler.h"

namespace WMW2 {

const int WMW2MinEditGroupingRadius = 1;
const int WMW2MinGroupingRadius = 15;
const int WMW2MinThumbnailSize = 30;

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

class WorldMapWidget2Private
{
public:
    WorldMapWidget2Private()
    : loadedAltitudeBackends(),
      loadedBackends(),
      currentBackend(0),
      currentBackendReady(false),
      currentBackendName(),
      stackedLayout(0),
      cacheCenterCoordinate(52.0,6.0),
      cacheZoom("marble:900"),
      configurationMenu(0),
      actionGroupBackendSelection(0),
      actionZoomIn(0),
      actionZoomOut(0),
      actionBrowseMode(0),
      actionEditMode(0),
      actionGroupMode(0),
      browseModeControlsHolder(0),
      controlWidget(0),
      lazyReclusteringRequested(false),
      clustersDirty(false),
      editModeAvailable(false),
      dragDropHandler(0),
      doUpdateMarkerCoordinatesInModel(true),
      sortMenu(0),
      thumbnailSize(WMW2MinThumbnailSize),
      groupingRadius(WMW2MinGroupingRadius),
      editGroupingRadius(WMW2MinEditGroupingRadius),
      actionIncreaseThumbnailSize(0),
      actionDecreaseThumbnailSize(0)
    {
    }

    QList<AltitudeBackend*> loadedAltitudeBackends;
    QList<MapBackend*> loadedBackends;
    MapBackend* currentBackend;
    bool currentBackendReady;
    QString currentBackendName;
    QStackedLayout* stackedLayout;

    // these values are cached in case the backend is not ready:
    WMWGeoCoordinate cacheCenterCoordinate;
    QString cacheZoom;

    // actions for controlling the widget
    QMenu* configurationMenu;
    QActionGroup* actionGroupBackendSelection;
    KAction* actionZoomIn;
    KAction* actionZoomOut;
    KAction* actionBrowseMode;
    KAction* actionEditMode;
    QActionGroup* actionGroupMode;
    QWidget* browseModeControlsHolder;
    QPointer<KHBox> controlWidget;
    KAction* actionPreviewSingleItems;
    KAction* actionPreviewGroupedItems;
    KAction* actionShowNumbersOnItems;

    bool lazyReclusteringRequested;
    bool clustersDirty;
    bool editModeAvailable;

    DragDropHandler* dragDropHandler;
    bool doUpdateMarkerCoordinatesInModel;

    QMenu *sortMenu;
    int thumbnailSize;
    int groupingRadius;
    int editGroupingRadius;
    KAction* actionIncreaseThumbnailSize;
    KAction* actionDecreaseThumbnailSize;
    KHBox* hBoxForAdditionalControlWidgetItems;
};

WorldMapWidget2::WorldMapWidget2(QWidget* const parent)
: QWidget(parent), s(new WMWSharedData), d(new WorldMapWidget2Private)
{
    createActions();

    // TODO: someone has to delete this model later!
    s->markerModel = new MarkerModel();
    s->worldMapWidget = this;

    // TODO: this needs some buffering for the google maps backend
    connect(s->markerModel, SIGNAL(signalTilesOrSelectionChanged()),
            this, SLOT(slotRequestLazyReclustering()));

    d->stackedLayout = new QStackedLayout(this);
    setLayout(d->stackedLayout);

    d->loadedBackends.append(new BackendGoogleMaps(s, this));
    d->loadedBackends.append(new BackendMarble(s, this));
//     d->loadedBackends.append(new BackendOSM(s, this));
    createActionsForBackendSelection();

    AltitudeBackend* const geonamesBackend = new BackendAltitudeGeonames(s, this);
    d->loadedAltitudeBackends.append(geonamesBackend);
    connect(geonamesBackend, SIGNAL(signalAltitudes(const WMW2::WMWAltitudeLookup::List)),
            this, SIGNAL(signalAltitudeLookupReady(const WMW2::WMWAltitudeLookup::List)));

    setAcceptDrops(true);
}

void WorldMapWidget2::createActions()
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
    d->actionBrowseMode->setText("B");
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
}

void WorldMapWidget2::createActionsForBackendSelection()
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

WorldMapWidget2::~WorldMapWidget2()
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

QStringList WorldMapWidget2::availableBackends() const
{
    QStringList result;

    MapBackend* backend;
    foreach(backend, d->loadedBackends)
    {
        result.append(backend->backendName());
    }

    return result;
}

bool WorldMapWidget2::setBackend(const QString& backendName)
{
    if (backendName == d->currentBackendName)
        return true;

    saveBackendToCache();

    // disconnect signals from old backend:
    if (d->currentBackend)
    {
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

        if (s->representativeChooser)
        {
            disconnect(s->representativeChooser, SIGNAL(signalThumbnailAvailableForIndex(const QVariant&, const QPixmap&)),
                        d->currentBackend, SLOT(slotThumbnailAvailableForIndex(const QVariant&, const QPixmap&)));
        }

    }

    MapBackend* backend;
    foreach(backend, d->loadedBackends)
    {
        if (backend->backendName() == backendName)
        {
            kDebug()<<QString("setting backend %1").arg(backendName);
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

            if (s->representativeChooser)
            {
                connect(s->representativeChooser, SIGNAL(signalThumbnailAvailableForIndex(const QVariant&, const QPixmap&)),
                        d->currentBackend, SLOT(slotThumbnailAvailableForIndex(const QVariant&, const QPixmap&)));
            }

            // call this slot manually in case the backend was ready right away:
            if (d->currentBackend->isReady())
            {
                slotBackendReady(d->currentBackendName);
            }
            else
            {
                rebuildConfigurationMenu();
            }

            return true;
        }
    }

    return false;
}

void WorldMapWidget2::applyCacheToBackend()
{
    if (!d->currentBackendReady)
        return;

    setCenter(d->cacheCenterCoordinate);
    // TODO: only do this if the zoom was changed!
    setZoom(d->cacheZoom);
}

void WorldMapWidget2::saveBackendToCache()
{
    if (!d->currentBackendReady)
        return;

    d->cacheCenterCoordinate = getCenter();
    d->cacheZoom = getZoom();
}

WMWGeoCoordinate WorldMapWidget2::getCenter() const
{
    if (!d->currentBackendReady)
        return WMWGeoCoordinate();

    return d->currentBackend->getCenter();
}

void WorldMapWidget2::setCenter(const WMWGeoCoordinate& coordinate)
{
    d->cacheCenterCoordinate = coordinate;

    if (!d->currentBackendReady)
        return;

    d->currentBackend->setCenter(coordinate);
}

void WorldMapWidget2::slotBackendReady(const QString& backendName)
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

    applyCacheToBackend();

    updateMarkers();
    markClustersAsDirty();

    rebuildConfigurationMenu();
}

void WorldMapWidget2::saveSettingsToGroup(KConfigGroup* const group)
{
    WMW2_ASSERT(group != 0);
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

void WorldMapWidget2::readSettingsFromGroup(const KConfigGroup* const group)
{
    WMW2_ASSERT(group != 0);
    if (!group)
        return;

    const QString alternativeBackendName = d->loadedBackends.isEmpty() ? "" : d->loadedBackends.first()->backendName();
    setBackend(group->readEntry("Backend", alternativeBackendName));

    const WMWGeoCoordinate centerDefault = WMWGeoCoordinate(52.0, 6.0);
    const QString centerGeoUrl = group->readEntry("Center", centerDefault.geoUrl());
    bool centerGeoUrlValid = false;
    const WMWGeoCoordinate centerCoordinate = WMWGeoCoordinate::fromGeoUrl(centerGeoUrl, &centerGeoUrlValid);
    setCenter(centerGeoUrlValid ? centerCoordinate : centerDefault);
    setZoom(group->readEntry("Zoom", d->cacheZoom));

    d->actionPreviewSingleItems->setChecked(group->readEntry("Preview Single Items", true));
    d->actionPreviewGroupedItems->setChecked(group->readEntry("Preview Grouped Items", true));
    d->actionShowNumbersOnItems->setChecked(group->readEntry("Show numbers on items", true));

    setThumnailSize(group->readEntry("Thumbnail Size", 2*WMW2MinThumbnailSize));
    setGroupingRadius(group->readEntry("Grouping Radius", 2*WMW2MinGroupingRadius));
    setEditGroupingRadius(group->readEntry("Edit Grouping Radius", WMW2MinEditGroupingRadius));
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

void WorldMapWidget2::rebuildConfigurationMenu()
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

KAction* WorldMapWidget2::getControlAction(const QString& actionName)
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
QWidget* WorldMapWidget2::getControlWidget()
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

        d->hBoxForAdditionalControlWidgetItems = new KHBox(d->controlWidget);

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

void WorldMapWidget2::slotZoomIn()
{
    if (!d->currentBackendReady)
        return;
    
    d->currentBackend->zoomIn();
}

void WorldMapWidget2::slotZoomOut()
{
    if (!d->currentBackendReady)
        return;

    d->currentBackend->zoomOut();
}

void WorldMapWidget2::slotUpdateActionsEnabled()
{
    d->actionDecreaseThumbnailSize->setEnabled((!s->inEditMode)&&(d->thumbnailSize>WMW2MinThumbnailSize));
    // TODO: define an upper limit!
    d->actionIncreaseThumbnailSize->setEnabled(!s->inEditMode);
}

void WorldMapWidget2::slotChangeBackend(QAction* action)
{
    WMW2_ASSERT(action!=0);

    if (!action)
        return;

    const QString newBackendName = action->data().toString();
    setBackend(newBackendName);
}

void WorldMapWidget2::updateMarkers()
{
    if (!d->currentBackendReady)
        return;

    // tell the backend to update the markers
    d->currentBackend->updateMarkers();
}

void WorldMapWidget2::updateClusters()
{
    kDebug()<<s->markerModel;
    if (!s->markerModel)
        return;

    kDebug()<<s->haveMovingCluster;
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
    QList<QPair<WMWGeoCoordinate, WMWGeoCoordinate> > mapBounds = d->currentBackend->getNormalizedBounds();

//     // debug output for tile level diagnostics:
//     QIntList tile1;
//     tile1<<520;
//     QIntList tile2 = tile1;
//     for (int i=1; i<=s->markerModel->maxLevel()-1; ++i)
//     {
//         tile2 = tile1;
//         tile2<<0;
//         tile1<<1;
//         const WMWGeoCoordinate tile1Coordinate = s->markerModel->tileIndexToCoordinate(tile1);
//         const WMWGeoCoordinate tile2Coordinate = s->markerModel->tileIndexToCoordinate(tile2);
//         QPoint tile1Point, tile2Point;
//         d->currentBackend->screenCoordinates(tile1Coordinate, &tile1Point);
//         d->currentBackend->screenCoordinates(tile2Coordinate, &tile2Point);
//         kDebug()<<i<<tile1Point<<tile2Point<<(tile1Point-tile2Point);
//     }

    const int gridSize = ClusterGridSizeScreen;
    const QSize mapSize  = d->currentBackend->mapSize();
    const int gridWidth  = mapSize.width();
    const int gridHeight = mapSize.height();
    QVector<QList<MarkerModel::TileIndex> > pixelNonEmptyTileIndexGrid(gridWidth*gridHeight, QList<MarkerModel::TileIndex>());
    QVector<int> pixelCountGrid(gridWidth*gridHeight, 0);
    QList<QPair<QPoint, QPair<int, QList<MarkerModel::TileIndex> > > > leftOverList;

    // TODO: iterate only over the visible part of the map
    int debugCountNonEmptyTiles = 0;
    int debugTilesSearched = 0;
    for (MarkerModel::NonEmptyIterator tileIterator(s->markerModel, markerLevel, mapBounds); !tileIterator.atEnd(); tileIterator.nextIndex())
    {
        const MarkerModel::TileIndex tileIndex = tileIterator.currentIndex();

        // find out where the tile is on the map:
        const WMWGeoCoordinate tileCoordinate = tileIndex.toCoordinates();
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
                    leftOverList << QPair<QPoint, QPair<int, QList<MarkerModel::TileIndex> > >(QPoint(x,y), QPair<int, QList<MarkerModel::TileIndex> >(pixelCountGrid.at(index), pixelNonEmptyTileIndexGrid.at(index)));
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

        WMWGeoCoordinate clusterCoordinates = pixelNonEmptyTileIndexGrid.at(markerX+markerY*gridWidth).first().toCoordinates();
        WMWCluster cluster;
        cluster.coordinates = clusterCoordinates;
        cluster.pixelPos = QPoint(markerX, markerY);
        cluster.tileIndicesList = MarkerModel::TileIndex::listToIntListList(pixelNonEmptyTileIndexGrid.at(markerX+markerY*gridWidth));
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
                cluster.tileIndicesList << MarkerModel::TileIndex::listToIntListList(pixelNonEmptyTileIndexGrid.at(index));
                pixelNonEmptyTileIndexGrid[index].clear();
                cluster.markerCount+= pixelCountGrid.at(index);
                pixelCountGrid[index] = 0;
            }
        }

        kDebug()<<QString("created cluster %1: %2 tiles").arg(s->clusterList.size()).arg(cluster.tileIndicesList.count());

        s->clusterList << cluster;
    }

    // now move all leftover markers into clusters:
    for (QList<QPair<QPoint, QPair<int, QList<MarkerModel::TileIndex> > > >::const_iterator it = leftOverList.constBegin();
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
            s->clusterList[closestIndex].tileIndicesList << MarkerModel::TileIndex::listToIntListList(it->second.second);
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
            clusterSelectedCount+= s->markerModel->getTileSelectedCount(MarkerModel::TileIndex::fromIntList(cluster.tileIndicesList.at(iTile)));
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

void WorldMapWidget2::slotClustersNeedUpdating()
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
void WorldMapWidget2::getColorInfos(const int clusterIndex, QColor *fillColor, QColor *strokeColor,
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

void WorldMapWidget2::getColorInfos(const WMWSelectionState selectionState,
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


QString WorldMapWidget2::convertZoomToBackendZoom(const QString& someZoom, const QString& targetBackend) const
{
    const QStringList zoomParts = someZoom.split(':');
    WMW2_ASSERT(zoomParts.count()==2);
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

    WMW2_ASSERT(targetZoom>=0);

    return QString("%1:%2").arg(targetBackend).arg(targetZoom);
}

void WorldMapWidget2::slotBackendZoomChanged(const QString& newZoom)
{
    kDebug()<<newZoom;
    d->cacheZoom = newZoom;
}

void WorldMapWidget2::setZoom(const QString& newZoom)
{
    d->cacheZoom = newZoom;

    if (d->currentBackendReady)
    {
        d->currentBackend->setZoom(d->cacheZoom);
    }
}

QString WorldMapWidget2::getZoom()
{
    if (d->currentBackendReady)
    {
        d->cacheZoom = d->currentBackend->getZoom();
    }

    return d->cacheZoom;
}

void WorldMapWidget2::slotClustersMoved(const QIntList& clusterIndices, const QPair<int, QModelIndex>& snapTarget)
{
    kDebug()<<clusterIndices;

    // TODO: we actually expect only one clusterindex
    int clusterIndex = clusterIndices.first();
    WMWGeoCoordinate targetCoordinates = s->clusterList.at(clusterIndex).coordinates;

    QList<QPersistentModelIndex> movedMarkers;
    if (s->clusterList.at(clusterIndex).selectedState==WMWSelectedNone)
    {
        // a not-selected marker was moved. update all of its items:
        const WMWCluster& cluster = s->clusterList.at(clusterIndex);
        for (int i=0; i<cluster.tileIndicesList.count(); ++i)
        {
            const MarkerModel::TileIndex tileIndex = MarkerModel::TileIndex::fromIntList(cluster.tileIndicesList.at(i));
            movedMarkers << s->markerModel->getTileMarkerIndices(tileIndex);
        }
    }
    else
    {
        // selected items were moved. Get their indices from the selection model:
        QItemSelectionModel* const selectionModel = s->markerModel->getSelectionModel();
        WMW2_ASSERT(selectionModel!=0);
        if (!selectionModel)
            return;

        QModelIndexList selectedIndices = selectionModel->selectedIndexes();
        for (int i=0; i<selectedIndices.count(); ++i)
        {
            // TODO: correctly handle items with multiple columns
            QModelIndex movedMarker = selectedIndices.at(i);
            if (movedMarker.column()==0)
            {
                movedMarkers << movedMarker;
            }
        }
    }

    if (snapTarget.first>=0)
    {
        kDebug()<<snapTarget.first<<movedMarkers.count()<<s->ungroupedModels.at(snapTarget.first);
        s->ungroupedModels.at(snapTarget.first)->snapItemsTo(snapTarget.second, movedMarkers);
        kDebug()<<snapTarget.first;
        return;
    }

    if (d->doUpdateMarkerCoordinatesInModel)
    {
        // update the positions of the markers:
        for (int i=0; i<movedMarkers.count(); ++i)
        {
            s->markerModel->moveMarker(movedMarkers.at(i), targetCoordinates);
        }
    }

//     kDebug()<<markerIndices;
    if (!movedMarkers.isEmpty())
    {
        emit(signalDisplayMarkersMoved(movedMarkers, targetCoordinates));
    }

    // TODO: clusters are marked as dirty by slotClustersNeedUpdating which is called
    // while we update the model
}

bool WorldMapWidget2::queryAltitudes(const WMWAltitudeLookup::List& queryItems, const QString& backendName)
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

void WorldMapWidget2::addUngroupedModel(WMWModelHelper* const modelHelper)
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

void WorldMapWidget2::setDisplayMarkersModel(QAbstractItemModel* const displayMarkersModel, const int coordinatesRole, QItemSelectionModel* const selectionModel)
{
    s->displayMarkersModel= displayMarkersModel;
    s->displayMarkersCoordinatesRole = coordinatesRole;
    s->markerModel->setMarkerModel(displayMarkersModel, coordinatesRole);
    s->markerModel->setSelectionModel(selectionModel);

    slotRequestLazyReclustering();
}

void WorldMapWidget2::slotGroupModeChanged(QAction* triggeredAction)
{
    Q_UNUSED(triggeredAction);
    s->inEditMode = d->actionEditMode->isChecked();

    slotRequestLazyReclustering();
}

/**
 * @brief Request reclustering, repeated calls should generate only one actual update of the clusters
 */
void WorldMapWidget2::slotRequestLazyReclustering()
{
    if (d->lazyReclusteringRequested)
        return;

    d->clustersDirty = true;
    d->lazyReclusteringRequested = true;
    QTimer::singleShot(0, this, SLOT(slotLazyReclusteringRequestCallBack()));
}

/**
 * @brief Helper function to buffer reclustering
 */
void WorldMapWidget2::slotLazyReclusteringRequestCallBack()
{
    if (!d->lazyReclusteringRequested)
        return;

    d->lazyReclusteringRequested = false;
    slotClustersNeedUpdating();
}

void WorldMapWidget2::slotClustersClicked(const QIntList& clusterIndices)
{
    kDebug()<<clusterIndices;
    QItemSelectionModel* const selectionModel = s->markerModel->getSelectionModel();
    if (!selectionModel)
        return;

    // update the selection state of the clusters
    for (int i=0; i<clusterIndices.count(); ++i)
    {
        const int clusterIndex = clusterIndices.at(i);
        kDebug()<<clusterIndex;
        const WMWCluster currentCluster = s->clusterList.at(clusterIndex);

        const bool doSelect = (currentCluster.selectedState!=WMWSelectedAll);
        kDebug()<<doSelect;
        for (int j=0; j<currentCluster.tileIndicesList.count(); ++j)
        {
            const MarkerModel::TileIndex& currentTileIndex = MarkerModel::TileIndex::fromIntList(currentCluster.tileIndicesList.at(j));

            const QList<QPersistentModelIndex> currentMarkers = s->markerModel->getTileMarkerIndices(currentTileIndex);
            kDebug()<<currentTileIndex<<currentMarkers;
            for (int k=0; k<currentMarkers.count(); ++k)
            {
                kDebug()<<k<<currentMarkers.at(k)<<doSelect;
                if (selectionModel->isSelected(currentMarkers.at(k))!=doSelect)
                {
                    selectionModel->select(currentMarkers.at(k), (doSelect ? QItemSelectionModel::Select : QItemSelectionModel::Deselect) | QItemSelectionModel::Rows);
                }
            }
        }
    }
}

void WorldMapWidget2::dragEnterEvent(QDragEnterEvent* event)
{
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

    // TODO: need data about the dragged object: #markers, selected, icon, ...
    event->accept();

//     if (!dragData->haveDragPixmap)
//         d->currentBackend->updateDragDropMarker(event->pos(), dragData);
    
}

void WorldMapWidget2::dragMoveEvent(QDragMoveEvent* event)
{
    // TODO: update the position of the drag marker if it is to be shown
//     if (!dragData->haveDragPixmap)
//         d->currentBackend->updateDragDropMarkerPosition(event->pos());
}

void WorldMapWidget2::dropEvent(QDropEvent* event)
{
    // remove the drag marker:
//     d->currentBackend->updateDragDropMarker(QPoint(), 0);

    if (!d->dragDropHandler)
    {
        event->ignore();
        return;
    }

    WMWGeoCoordinate dropCoordinates;
    if (!d->currentBackend->geoCoordinates(event->pos(), &dropCoordinates))
        return;

    QList<QPersistentModelIndex> droppedIndices;
    if (d->dragDropHandler->dropEvent(event, dropCoordinates, &droppedIndices))
    {
        event->acceptProposedAction();

        if (!droppedIndices.isEmpty())
        {
            emit(signalDisplayMarkersMoved(droppedIndices, dropCoordinates));
        }
    }
    // TODO: the drag-and-drop handler should do this now!
//     for (int i=0; i<dragData->itemIndices.count(); ++i)
//     {
//         s->markerModel->moveMarker(dragData->itemIndices.at(i), dropCoordinates);
//     }
    
}

void WorldMapWidget2::dragLeaveEvent(QDragLeaveEvent* event)
{
    Q_UNUSED(event);

    // remove the marker:
//     d->currentBackend->updateDragDropMarker(QPoint(), 0);
}

void WorldMapWidget2::markClustersAsDirty()
{
    d->clustersDirty = true;
}

/**
 * @brief Controls whether the user can switch from browse to edit mode.
 */
void WorldMapWidget2::setEditModeAvailable(const bool state)
{
    d->editModeAvailable = state;

    if (d->browseModeControlsHolder)
    {
        d->browseModeControlsHolder->setVisible(d->editModeAvailable);
    }
}

void WorldMapWidget2::setDragDropHandler(DragDropHandler* const dragDropHandler)
{
    d->dragDropHandler = dragDropHandler;
}

QVariant WorldMapWidget2::getClusterRepresentativeMarker(const int clusterIndex, const int sortKey)
{
    if (!s->representativeChooser)
        return QVariant();

    const WMWCluster cluster = s->clusterList.at(clusterIndex);
    QMap<int, QVariant>::const_iterator it = cluster.representativeMarkers.find(sortKey);
    if (it!=cluster.representativeMarkers.end())
        return *it;

    QList<QVariant> repIndices;
    for (int i=0; i<cluster.tileIndicesList.count(); ++i)
    {
        repIndices <<  s->markerModel->getTileRepresentativeMarker(MarkerModel::TileIndex::fromIntList(cluster.tileIndicesList.at(i)), sortKey);
    }

    const QVariant clusterRepresentative = s->representativeChooser->bestRepresentativeIndexFromList(repIndices, sortKey);

    s->clusterList[clusterIndex].representativeMarkers[sortKey] = clusterRepresentative;

    return clusterRepresentative;
}

void WorldMapWidget2::setRepresentativeChooser(WMWRepresentativeChooser* const chooser)
{
    s->representativeChooser = chooser;
    if (d->currentBackend&&chooser)
    {
        connect(s->representativeChooser, SIGNAL(signalThumbnailAvailableForIndex(const QVariant&, const QPixmap&)),
                d->currentBackend, SLOT(slotThumbnailAvailableForIndex(const QVariant&, const QPixmap&)));
    }
}

void WorldMapWidget2::slotItemDisplaySettingsChanged()
{
    s->previewSingleItems = d->actionPreviewSingleItems->isChecked();
    s->previewGroupedItems = d->actionPreviewGroupedItems->isChecked();
    s->showNumbersOnItems = d->actionShowNumbersOnItems->isChecked();

    // TODO: update action availability?

    // TODO: we just need to update the display, no need to recluster?
    slotRequestLazyReclustering();
}

void WorldMapWidget2::setDoUpdateMarkerCoordinatesInModel(const bool doIt)
{
    d->doUpdateMarkerCoordinatesInModel = doIt;
}

void WorldMapWidget2::setSortOptionsMenu(QMenu* const sortMenu)
{
    d->sortMenu = sortMenu;

    rebuildConfigurationMenu();
}

void WorldMapWidget2::setSortKey(const int sortKey)
{
    s->sortKey = sortKey;

    // this is probably faster than writing a function that changes all the clusters icons...
    slotRequestLazyReclustering();
}

QPixmap WorldMapWidget2::getDecoratedPixmapForCluster(const int clusterId, const WMWSelectionState* const selectedStateOverride, const int* const countOverride, QPoint* const centerPoint)
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

    bool displayThumbnail = (s->representativeChooser != 0);
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
        QPixmap clusterPixmap = s->representativeChooser->pixmapFromRepresentativeIndex(representativeMarker, QSize(undecoratedThumbnailSize, undecoratedThumbnailSize));

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

void WorldMapWidget2::setThumnailSize(const int newThumbnailSize)
{
    d->thumbnailSize = qMax(WMW2MinThumbnailSize, newThumbnailSize);

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

void WorldMapWidget2::setGroupingRadius(const int newGroupingRadius)
{
    d->groupingRadius = qMax(WMW2MinGroupingRadius, newGroupingRadius);

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

void WorldMapWidget2::setEditGroupingRadius(const int newGroupingRadius)
{
    d->editGroupingRadius = qMax(WMW2MinEditGroupingRadius, newGroupingRadius);

    if (s->inEditMode)
    {
        slotRequestLazyReclustering();
    }
    slotUpdateActionsEnabled();
}

void WorldMapWidget2::slotDecreaseThumbnailSize()
{
    if (s->inEditMode)
        return;

    if (d->thumbnailSize>WMW2MinThumbnailSize)
    {
        const int newThumbnailSize = qMax(WMW2MinThumbnailSize, d->thumbnailSize-5);

        // make sure the grouping radius is also decreased
        // this will automatically decrease the thumbnail size as well
        setGroupingRadius(newThumbnailSize/2);
    }
}

void WorldMapWidget2::slotIncreaseThumbnailSize()
{
    if (s->inEditMode)
        return;

    setThumnailSize(d->thumbnailSize+5);
}

int WorldMapWidget2::getThumbnailSize() const
{
    return d->thumbnailSize;
}

int WorldMapWidget2::getUndecoratedThumbnailSize() const
{
    return d->thumbnailSize-2;
}

void WorldMapWidget2::slotUngroupedModelChanged()
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

    WMWModelHelper* const senderHelper = qobject_cast<WMWModelHelper*>(senderObject);
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

void WorldMapWidget2::addWidgetToControlWidget(QWidget* const newWidget)
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

} /* WMW2 */

