/* ============================================================
 *
 * Date        : 2009-12-01
 * Description : WorldMapWidget2
 *
 * Copyright (C) 2009 by Michael G. Hansen <mike at mghansen dot de>
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
#include <kdebug.h>
#include <khbox.h>
#include <klocale.h>

// local includes

#include "worldmapwidget2.h"
#include "map-backend.h"
#include "backend-marble.h"
#include "backend-googlemaps.h"
#include "markermodel.h"

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
    : loadedBackends(),
      currentBackend(0),
      currentBackendReady(false),
      currentBackendName(),
      stackedLayout(0),
      actionConfigurationMenu(0),
      actionZoomIn(0),
      actionZoomOut(0),
      controlWidget(0)
    {
    }
    
    QList<MapBackend*> loadedBackends;
    MapBackend* currentBackend;
    bool currentBackendReady;
    QString currentBackendName;
    QStackedLayout* stackedLayout;

    // these values are cached in case the backend is not ready:
    WMWGeoCoordinate cacheCenterCoordinate;

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
}

void WorldMapWidget2::saveBackendToCache()
{
    if (!d->currentBackendReady)
        return;

    kDebug()<<1;
    d->cacheCenterCoordinate = getCenter();
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

    updateClusters();
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
    kDebug()<<"updateClusters starting...";
    s->clusterList.clear();

    if (!d->currentBackendReady)
        return;

    const int gridSize = ClusterGridSizeScreen;
    const QSize mapSize  = d->currentBackend->mapSize();
    const int gridWidth  = mapSize.width();
    const int gridHeight = mapSize.height();
    QVector<QList<QIntList> > pixelIndexGrid(gridWidth*gridHeight, QList<QIntList>());
    QVector<int> pixelCountGrid(gridWidth*gridHeight, 0);
    QList<QPair<QPoint, QPair<int, QList<QIntList> > > > leftOverList;
    // TODO: which level?
    const int markerLevel = s->markerModel->maxLevel()-1;

    // TODO: iterate only over the visible part of the map
    for (MarkerModel::NonEmptyIterator tileIterator(s->markerModel, markerLevel); !tileIterator.atEnd(); tileIterator.nextIndex())
    {
        const QIntList tileIndex = tileIterator.currentIndex();

        // find out where the tile is on the map:
        const WMWGeoCoordinate tileCoordinate = s->markerModel->tileIndexToCoordinate(tileIndex);

        QPoint tilePoint;
        if (!d->currentBackend->screenCoordinates(tileCoordinate, &tilePoint))
        {
            continue;
        }

        // make sure we are in the grid (in case there are rounding errors somewhere in the backend
        if ((tilePoint.x()<0)||(tilePoint.y()<0)||(tilePoint.x()>=gridWidth)||(tilePoint.y()>=gridHeight))
            continue;

        pixelIndexGrid[tilePoint.x() + tilePoint.y()*gridWidth] << tileIndex;
        pixelCountGrid[tilePoint.x() + tilePoint.y()*gridWidth]+= s->markerModel->getTileMarkerCount(tileIndex);
    }

    // TODO: cleanup this list every ... iterations in the next loop, too
    QIntList pixelGridIndices;

    for (int i=0; i<gridWidth*gridHeight; ++i)
    {
        if (pixelCountGrid.at(i)>0)
            pixelGridIndices << i;
    }

    // re-add the markers to clusters:
    int lastTooCloseClusterIndex = 0;
    Q_FOREVER
    {
        int markerMax = 0;
        int markerX = 0;
        int markerY = 0;
        int pixelGridMetaIndexMax = 0;

        for (int pixelGridMetaIndex = 0; pixelGridMetaIndex<pixelGridIndices.size(); ++pixelGridMetaIndex)
        {
            const int index = pixelGridIndices[pixelGridMetaIndex];
            if (index<0)
                continue;

            if (pixelCountGrid.at(index)==0)
            {
                // TODO: also remove this entry from the list to speed up the loop!
                pixelGridIndices[pixelGridMetaIndex] = -1;
                continue;
            }

            // calculate x,y from the linear index:
            const int x = index % gridWidth;
            const int y = (index-x)/gridWidth;
            const QPoint markerPosition(x, y);

            if (pixelCountGrid.at(index)>markerMax)
            {
                // only create a cluster here if it is not too close to another cluster:
                bool tooClose = false;

                // check the cluster that was a problem last time first:
                if (lastTooCloseClusterIndex<s->clusterList.size())
                {
                    tooClose = QPointSquareDistance(s->clusterList.at(lastTooCloseClusterIndex).pixelPos, markerPosition) < pow(ClusterGridSizeScreen/2, 2);
                }

                // now check all other clusters:
                for (int i=0; (!tooClose)&&(i<s->clusterList.size()); ++i)
                {
                    if (i==lastTooCloseClusterIndex)
                        continue;

                    tooClose = QPointSquareDistance(s->clusterList.at(i).pixelPos, markerPosition) < pow(ClusterGridSizeScreen/2, 2);
                    if (tooClose)
                        lastTooCloseClusterIndex = i;
                }

                if (tooClose)
                {
                    // move markers into leftover list
                    leftOverList << QPair<QPoint, QPair<int, QList<QIntList> > >(QPoint(x,y), QPair<int, QList<QIntList> >(pixelCountGrid.at(index), pixelIndexGrid.at(index)));
                    pixelCountGrid[index] = 0;
                    pixelIndexGrid[index].clear();
                    pixelGridIndices[pixelGridMetaIndex] = -1;
                }
                else
                {
                    markerMax=pixelCountGrid.at(x+y*gridWidth);
                    markerX=x;
                    markerY=y;
                    pixelGridMetaIndexMax = pixelGridMetaIndex;
                }
            }
        }
        
        if (markerMax==0)
            break;

        WMWGeoCoordinate clusterCoordinates = s->markerModel->tileIndexToCoordinate( pixelIndexGrid.at(markerX+markerY*gridWidth).first() );
        WMWCluster cluster;
        cluster.coordinates = clusterCoordinates;
        cluster.pixelPos = QPoint(markerX, markerY);
        cluster.tileIndicesList = pixelIndexGrid.at(markerX+markerY*gridWidth);
        cluster.markerCount = pixelCountGrid.at(markerX+markerY*gridWidth);

        // mark the pixel as done:
        pixelCountGrid[markerX+markerY*gridWidth] = 0;
        pixelIndexGrid[markerX+markerY*gridWidth].clear();
        pixelGridIndices[pixelGridMetaIndexMax] = -1;

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
                cluster.tileIndicesList << pixelIndexGrid.at(index);
                pixelIndexGrid[index].clear();
                cluster.markerCount+= pixelCountGrid.at(index);
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

    kDebug()<<s->clusterList.size();

    d->currentBackend->updateClusters();
}

} /* WMW2 */

