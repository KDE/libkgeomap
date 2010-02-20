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

#include <QMenu>
#include <QPointer>
#include <QStackedLayout>
#include <QToolButton>

// KDE includes

#include <kaction.h>
#include <kconfig.h>
#include <kconfiggroup.h>
#include <khbox.h>
#include <klocale.h>

// local includes

#include "map-backend.h"
#include "backend-marble.h"
#include "backend-googlemaps.h"
#include "backend-osm.h"
#include "markermodel.h"
#include "backend-altitude-geonames.h"

namespace WMW2 {

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
      actionConfigurationMenu(0),
      actionZoomIn(0),
      actionZoomOut(0),
      controlWidget(0)
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
    QPointer<KAction> actionConfigurationMenu;
    QPointer<KAction> actionZoomIn;
    QPointer<KAction> actionZoomOut;
    QPointer<QWidget> controlWidget;
};

WorldMapWidget2::WorldMapWidget2(QWidget* const parent)
: QWidget(parent), s(new WMWSharedData), d(new WorldMapWidget2Private)
{
    s->markerModel = new MarkerModel;
    s->worldMapWidget = this;

    d->stackedLayout = new QStackedLayout(this);
    setLayout(d->stackedLayout);

    d->loadedBackends.append(new BackendGoogleMaps(s, this));
    d->loadedBackends.append(new BackendMarble(s, this));
    d->loadedBackends.append(new BackendOSM(s, this));

    AltitudeBackend* const geonamesBackend = new BackendAltitudeGeonames(s, this);
    d->loadedAltitudeBackends.append(geonamesBackend);
    connect(geonamesBackend, SIGNAL(signalAltitudes(const WMW2::WMWAltitudeLookup::List)),
            this, SIGNAL(signalAltitudeLookupReady(const WMW2::WMWAltitudeLookup::List)));
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

        disconnect(d->currentBackend, SIGNAL(signalClustersMoved(const QIntList&)),
                this, SLOT(slotClustersMoved(const QIntList&)));

        disconnect(d->currentBackend, SIGNAL(signalMarkersMoved(const QIntList&)),
                this, SLOT(slotMarkersMoved(const QIntList&)));
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

            connect(d->currentBackend, SIGNAL(signalClustersMoved(const QIntList&)),
                    this, SLOT(slotClustersMoved(const QIntList&)));

            connect(d->currentBackend, SIGNAL(signalMarkersMoved(const QIntList&)),
                    this, SLOT(slotMarkersMoved(const QIntList&)));

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

    for (int i=0; i<d->loadedBackends.size(); ++i)
    {
        d->loadedBackends.at(i)->readSettingsFromGroup(group);
    }
}

void WorldMapWidget2::rebuildConfigurationMenu()
{
    if (!d->actionConfigurationMenu)
    {
        d->actionConfigurationMenu = new KAction(this);
    }

    QMenu* configurationMenu = d->actionConfigurationMenu->menu();
    if (!configurationMenu)
    {
        // TODO: will the menu be deleted when the action is deleted?
        configurationMenu = new QMenu(this);
        d->actionConfigurationMenu->setMenu(configurationMenu);
    }
    else
    {
        configurationMenu->clear();
    }

    // create backend selection entries:
    QActionGroup* const backendActionGroup = new QActionGroup(configurationMenu);
    backendActionGroup->setExclusive(true);
    for (int i = 0; i<d->loadedBackends.size(); ++i)
    {
        const QString backendName = d->loadedBackends.at(i)->backendName();
        KAction* backendAction = new KAction(backendActionGroup);
        backendAction->setData(backendName);
        backendAction->setText(d->loadedBackends.at(i)->backendHumanName());
        backendAction->setCheckable(true);
        
        if (backendName==d->currentBackendName)
        {
            backendAction->setChecked(true);
        }

        configurationMenu->addAction(backendAction);
    }

    if (d->currentBackendReady)
    {
        d->currentBackend->addActionsToConfigurationMenu(configurationMenu);
    }

    connect(backendActionGroup, SIGNAL(triggered(QAction*)),
            this, SLOT(slotChangeBackend(QAction*)));
}

KAction* WorldMapWidget2::getControlAction(const QString& actionName)
{
    if (actionName=="configuration")
    {
        bool needToRebuildMenu = !d->actionConfigurationMenu;
        if (!needToRebuildMenu)
        {
            needToRebuildMenu = !d->actionConfigurationMenu->menu();
        }
        if (needToRebuildMenu)
        {
            rebuildConfigurationMenu();
        }
        return d->actionConfigurationMenu;
    }
    else if (actionName=="zoomin")
    {
        if (!d->actionZoomIn)
        {
            d->actionZoomIn = new KAction(this);
            d->actionZoomIn->setIcon(SmallIcon("zoom-in"));
            d->actionZoomIn->setToolTip(i18n("Zoom in"));
            connect(d->actionZoomIn, SIGNAL(triggered()),
                    this, SLOT(slotZoomIn()));
        }
        return d->actionZoomIn;
    }
    else if (actionName=="zoomout")
    {
        if (!d->actionZoomOut)
        {
            d->actionZoomOut = new KAction(this);
            d->actionZoomOut->setIcon(SmallIcon("zoom-out"));
            d->actionZoomOut->setToolTip(i18n("Zoom out"));
            connect(d->actionZoomOut, SIGNAL(triggered()),
                    this, SLOT(slotZoomOut()));
        }
        return d->actionZoomOut;
    }

    return 0;
}

QWidget* WorldMapWidget2::getControlWidget()
{
    if (!d->controlWidget)
    {
        d->controlWidget = new KHBox(this);

        QToolButton* const configurationButton = new QToolButton(d->controlWidget);
        configurationButton->setToolTip(i18n("Map settings"));
        configurationButton->setIcon(SmallIcon("applications-internet"));
        configurationButton->setMenu(getControlAction("configuration")->menu());
        configurationButton->setPopupMode(QToolButton::InstantPopup);

        QToolButton* const zoomInButton = new QToolButton(d->controlWidget);
        zoomInButton->setDefaultAction(getControlAction("zoomin"));

        QToolButton* const zoomOutButton = new QToolButton(d->controlWidget);
        zoomOutButton->setDefaultAction(getControlAction("zoomout"));

        QHBoxLayout* const hBoxLayout = reinterpret_cast<QHBoxLayout*>(d->controlWidget->layout());
        if (hBoxLayout)
        {
            hBoxLayout->addStretch();
        }
    }

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
    
}

void WorldMapWidget2::slotChangeBackend(QAction* action)
{
    WMW2_ASSERT(action!=0);

    if (!action)
        return;

    const QString newBackendName = action->data().toString();
    setBackend(newBackendName);
}

void WorldMapWidget2::addClusterableMarkers(const WMWMarker::List& markerList)
{
    s->markerModel->addMarkers(markerList);

    slotClustersNeedUpdating();
}

void WorldMapWidget2::addSingleMarkers(const WMWMarker::List& markerList)
{
    const int oldMarkerCount = s->markerList.count();
    s->markerList << markerList;

    // TODO: clustering
    for (int i=0; i<markerList.count(); ++i)
    {
        s->visibleMarkers << oldMarkerCount + i;
    }

    updateMarkers();
}

void WorldMapWidget2::updateMarkers()
{
    if (!d->currentBackendReady)
        return;

    // tell the backend to update the markers
    d->currentBackend->updateMarkers();
}

// constants for clusters
const int ClusterRadius          = 15;
const QSize ClusterDefaultSize   = QSize(2*ClusterRadius, 2*ClusterRadius);
const int ClusterGridSizeScreen  = 60;
const QSize ClusterMaxPixmapSize = QSize(60, 60);

void WorldMapWidget2::updateClusters()
{
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
    QVector<QList<QIntList> > pixelNonEmptyTileIndexGrid(gridWidth*gridHeight, QList<QIntList>());
    QVector<int> pixelCountGrid(gridWidth*gridHeight, 0);
    QList<QPair<QPoint, QPair<int, QList<QIntList> > > > leftOverList;

    // TODO: iterate only over the visible part of the map
    int debugCountNonEmptyTiles = 0;
    int debugTilesSearched = 0;
    for (MarkerModel::NonEmptyIterator tileIterator(s->markerModel, markerLevel, mapBounds); !tileIterator.atEnd(); tileIterator.nextIndex())
    {
        const QIntList tileIndex = tileIterator.currentIndex();

        // find out where the tile is on the map:
        const WMWGeoCoordinate tileCoordinate = s->markerModel->tileIndexToCoordinate(tileIndex);
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
                    leftOverList << QPair<QPoint, QPair<int, QList<QIntList> > >(QPoint(x,y), QPair<int, QList<QIntList> >(pixelCountGrid.at(index), pixelNonEmptyTileIndexGrid.at(index)));
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

        WMWGeoCoordinate clusterCoordinates = s->markerModel->tileIndexToCoordinate( pixelNonEmptyTileIndexGrid.at(markerX+markerY*gridWidth).first() );
        WMWCluster cluster;
        cluster.coordinates = clusterCoordinates;
        cluster.pixelPos = QPoint(markerX, markerY);
        cluster.tileIndicesList = pixelNonEmptyTileIndexGrid.at(markerX+markerY*gridWidth);
        cluster.markerCount = pixelCountGrid.at(markerX+markerY*gridWidth);

//         kDebug()<<QString("created cluster %1: %2 markers").arg(s->clusterList.size()).arg(cluster.markerCount);

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
                cluster.tileIndicesList << pixelNonEmptyTileIndexGrid.at(index);
                pixelNonEmptyTileIndexGrid[index].clear();
                cluster.markerCount+= pixelCountGrid.at(index);
                pixelCountGrid[index] = 0;
            }
        }

        s->clusterList << cluster;
    }

    // now move all leftover markers into clusters:
    for (QList<QPair<QPoint, QPair<int, QList<QIntList> > > >::const_iterator it = leftOverList.constBegin();
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
            s->clusterList[closestIndex].tileIndicesList << it->second.second;
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
 * @param strokeStyle Style used to draw the stroke around the crircle
 * @param labelText Text for the label
 * @param labelColor Color for the label text
 */
void WorldMapWidget2::getColorInfos(const int clusterIndex, QColor *fillColor, QColor *strokeColor,
                                    Qt::PenStyle *strokeStyle, QString *labelText, QColor *labelColor) const
{
    const WMWCluster& cluster = s->clusterList.at(clusterIndex);

    // TODO: check that this number is already valid!
    const int nMarkers = cluster.markerCount;

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
//     switch (selected)
//     {
//         case PartialNone:
//             *strokeStyle = Qt::NoPen;
//             break;
//         case PartialSome:
//             *strokeStyle = Qt::DotLine;
//             break;
//         case PartialAll:
//             *strokeStyle = Qt::SolidLine;
//             break;
//     }
    *strokeColor = QColor(Qt::blue);

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

void WorldMapWidget2::slotClustersMoved(const QIntList& clusterIndices)
{
    kDebug()<<clusterIndices;

    QIntList markerIndices;
    // update the positions of the markers held by the clusters:
    for (int cii=0; cii<clusterIndices.count(); ++cii)
    {
        const int clusterIndex = clusterIndices.at(cii);

        const WMWCluster& cluster = s->clusterList.at(clusterIndex);

        // remove all the markers in the cluster from the model and then add them again:
        QIntList movedMarkers;
        for (int i=0; i<cluster.tileIndicesList.count(); ++i)
        {
            const QIntList tileIndex = cluster.tileIndicesList.at(i);
            MarkerModel::Tile* const sourceTile = s->markerModel->getTile(tileIndex, true);
            if (!sourceTile)
            {
                continue;
            }

            movedMarkers << sourceTile->markerIndices;
        }

        // update the positions of the markers:
        for (int i=0; i<movedMarkers.count(); ++i)
        {
            s->markerModel->moveMarker(movedMarkers.at(i), cluster.coordinates);
        }

        markerIndices << movedMarkers;
    }

    kDebug()<<markerIndices;
    if (!markerIndices.isEmpty())
    {
        emit(signalGroupableMarkersMoved(markerIndices));
    }

    // TODO: who calls updateClusters? Right now, the google maps backend does that
}

WMWMarker WorldMapWidget2::getClusterableMarker(const int markerIndex)
{
    WMW2_ASSERT(markerIndex<s->markerModel->markerList.count());
    return s->markerModel->markerList.at(markerIndex);
}

WMWMarker& WorldMapWidget2::getSingleMarker(const int markerIndex)
{
    WMW2_ASSERT(markerIndex<s->markerList.count());
    return s->markerList[markerIndex];
}

void WorldMapWidget2::slotMarkersMoved(const QIntList& markerIndices)
{
    emit(signalSingleMarkersMoved(markerIndices));
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

} /* WMW2 */

