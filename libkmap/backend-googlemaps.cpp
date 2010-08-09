 /** ===========================================================
 *
 * This file is a part of digiKam project
 * <a href="http://www.digikam.org">http://www.digikam.org</a>
 *
 * @date   2009-12-01
 * @brief  Google-Maps-backend for KMap
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

#include "backend-googlemaps.moc"

// Qt includes

#include <QBuffer>
#include <QActionGroup>
#include <QMenu>
#include <QPainter>
#include <QPointer>

// KDE includes

#include <kaction.h>
#include <kconfiggroup.h>
#include <khtml_part.h>
#include <kstandarddirs.h>

// local includes

#include "html_widget.h"
#include "kmap.h"
#include "abstractmarkertiler.h"

namespace KMapIface
{

class BackendGoogleMaps::BackendGoogleMapsPrivate
{
public:

    BackendGoogleMapsPrivate()
    : htmlWidget(0),
      htmlWidgetWrapper(0),
      isReady(false),
      mapTypeActionGroup(0),
      floatItemsActionGroup(0),
      showMapTypeControlAction(0),
      showNavigationControlAction(0),
      showScaleControlAction(0),
      cacheMapType("ROADMAP"),
      cacheShowMapTypeControl(true),
      cacheShowNavigationControl(true),
      cacheShowScaleControl(true),
      cacheZoom(1),
      cacheMaxZoom(0),
      cacheMinZoom(0),
      cacheCenter(0.0,0.0),
      cacheBounds()
    {
    }

    QPointer<HTMLWidget>                      htmlWidget;
    QPointer<QWidget>                         htmlWidgetWrapper;
    bool                                      isReady;
    QActionGroup*                             mapTypeActionGroup;
    QActionGroup*                             floatItemsActionGroup;
    KAction*                                  showMapTypeControlAction;
    KAction*                                  showNavigationControlAction;
    KAction*                                  showScaleControlAction;

    QString                                   cacheMapType;
    bool                                      cacheShowMapTypeControl;
    bool                                      cacheShowNavigationControl;
    bool                                      cacheShowScaleControl;
    int                                       cacheZoom;
    int                                       cacheMaxZoom;
    int                                       cacheMinZoom;
    WMWGeoCoordinate                          cacheCenter;
    QPair<WMWGeoCoordinate, WMWGeoCoordinate> cacheBounds;


};

BackendGoogleMaps::BackendGoogleMaps(const QExplicitlySharedDataPointer<WMWSharedData>& sharedData, QObject* const parent)
                 : MapBackend(sharedData, parent), d(new BackendGoogleMapsPrivate())
{
    createActions();

    d->htmlWidgetWrapper = new QWidget();
    d->htmlWidgetWrapper->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    d->htmlWidget        = new HTMLWidget(d->htmlWidgetWrapper);
    d->htmlWidgetWrapper->resize(400,400);

    connect(d->htmlWidget, SIGNAL(signalJavaScriptReady()),
            this, SLOT(slotHTMLInitialized()));

    connect(d->htmlWidget, SIGNAL(signalHTMLEvents(const QStringList&)),
            this, SLOT(slotHTMLEvents(const QStringList&)));

    connect(d->htmlWidget, SIGNAL(selectionHasBeenMade(const QList<qreal>&)),
            this, SLOT(slotSelectionHasBeenMade(const QList<qreal>&)));

    loadInitialHTML();

    d->htmlWidgetWrapper->installEventFilter(this);
}

BackendGoogleMaps::~BackendGoogleMaps()
{
    if (d->htmlWidgetWrapper)
        delete d->htmlWidgetWrapper;

    delete d;
}

void BackendGoogleMaps::createActions()
{
    // actions for selecting the map type:
    d->mapTypeActionGroup = new QActionGroup(this);
    d->mapTypeActionGroup->setExclusive(true);

    connect(d->mapTypeActionGroup, SIGNAL(triggered(QAction*)),
            this, SLOT(slotMapTypeActionTriggered(QAction*)));

    QStringList mapTypes, mapTypesHumanNames;
    mapTypes           << "ROADMAP"       << "SATELLITE"        << "HYBRID"       << "TERRAIN";
    mapTypesHumanNames << i18n("Roadmap") << i18n("Satellite")  << i18n("Hybrid") << i18n("Terrain");

    for (int i = 0; i<mapTypes.count(); ++i)
    {
        KAction* const mapTypeAction = new KAction(d->mapTypeActionGroup);
        mapTypeAction->setData(mapTypes.at(i));
        mapTypeAction->setText(mapTypesHumanNames.at(i));
        mapTypeAction->setCheckable(true);
    }

    // float items:
    d->floatItemsActionGroup = new QActionGroup(this);
    d->floatItemsActionGroup->setExclusive(false);

    connect(d->floatItemsActionGroup, SIGNAL(triggered(QAction*)),
            this, SLOT(slotFloatSettingsTriggered(QAction*)));

    d->showMapTypeControlAction = new KAction(i18n("Show Map Type Control"), d->floatItemsActionGroup);
    d->showMapTypeControlAction->setCheckable(true);
    d->showMapTypeControlAction->setChecked(d->cacheShowMapTypeControl);
    d->showMapTypeControlAction->setData("showmaptypecontrol");

    d->showNavigationControlAction = new KAction(i18n("Show Navigation Control"), d->floatItemsActionGroup);
    d->showNavigationControlAction->setCheckable(true);
    d->showNavigationControlAction->setChecked(d->cacheShowNavigationControl);
    d->showNavigationControlAction->setData("shownavigationcontrol");

    d->showScaleControlAction = new KAction(i18n("Show Scale Control"), d->floatItemsActionGroup);
    d->showScaleControlAction->setCheckable(true);
    d->showScaleControlAction->setChecked(d->cacheShowScaleControl);
    d->showScaleControlAction->setData("showscalecontrol");
}

void BackendGoogleMaps::loadInitialHTML()
{
    const KUrl htmlUrl = KStandardDirs::locate("data", "libkmap/backend-googlemaps.html");

    d->htmlWidget->openUrl(htmlUrl);
}

QString BackendGoogleMaps::backendName() const
{
    return QString("googlemaps");
}

QString BackendGoogleMaps::backendHumanName() const
{
    return i18n("Google Maps");
}

QWidget* BackendGoogleMaps::mapWidget() const
{
    return d->htmlWidgetWrapper.data();
}

WMWGeoCoordinate BackendGoogleMaps::getCenter() const
{
    return d->cacheCenter;
}

void BackendGoogleMaps::setCenter(const WMWGeoCoordinate& coordinate)
{
    d->cacheCenter = coordinate;

    if (isReady())
    {
        d->htmlWidget->runScript(QString("wmwSetCenter(%1, %2);").arg(d->cacheCenter.latString()).arg(d->cacheCenter.lonString()));
    }
}

bool BackendGoogleMaps::isReady() const
{
    return d->isReady;
}

void BackendGoogleMaps::slotHTMLInitialized()
{
    kDebug() << 1;
    d->isReady = true;
    d->htmlWidget->runScript(QString("wmwWidgetResized(%1, %2)").arg(d->htmlWidgetWrapper->width()).arg(d->htmlWidgetWrapper->height()));

    // TODO: call javascript directly here and update action availability in one shot
    setMapType(d->cacheMapType);
    setShowMapTypeControl(d->cacheShowMapTypeControl);
    setShowNavigationControl(d->cacheShowNavigationControl);
    setShowScaleControl(d->cacheShowNavigationControl);
    setCenter(d->cacheCenter);
    d->htmlWidget->runScript(QString("wmwSetZoom(%1);").arg(d->cacheZoom));
    emit(signalBackendReady(backendName()));
}

void BackendGoogleMaps::zoomIn()
{
    if (!d->isReady)
        return;

    d->htmlWidget->runScript(QString("wmwZoomIn();"));
}

void BackendGoogleMaps::zoomOut()
{
    if (!d->isReady)
        return;

    d->htmlWidget->runScript(QString("wmwZoomOut();"));
}

QString BackendGoogleMaps::getMapType() const
{
    return d->cacheMapType;
}

void BackendGoogleMaps::setMapType(const QString& newMapType)
{
    d->cacheMapType = newMapType;
    kDebug()<<newMapType;

    if (isReady())
    {
        d->htmlWidget->runScript(QString("wmwSetMapType(\"%1\");").arg(newMapType));
        updateZoomMinMaxCache();
        updateActionAvailability();
    }
}

void BackendGoogleMaps::slotMapTypeActionTriggered(QAction* action)
{
    const QString newMapType = action->data().toString();
    setMapType(newMapType);
}

void BackendGoogleMaps::addActionsToConfigurationMenu(QMenu* const configurationMenu)
{
    KMAP_ASSERT(configurationMenu!=0);

    if (!d->isReady)
        return;

    configurationMenu->addSeparator();

    // map type actions:
    const QList<QAction*> mapTypeActions = d->mapTypeActionGroup->actions();
    for (int i = 0; i<mapTypeActions.count(); ++i)
    {
        QAction* const mapTypeAction = mapTypeActions.at(i);
        configurationMenu->addAction(mapTypeAction);
    }

    configurationMenu->addSeparator();

    // float items visibility:
    QMenu* const floatItemsSubMenu = new QMenu(i18n("Float items"), configurationMenu);
    configurationMenu->addMenu(floatItemsSubMenu);

    floatItemsSubMenu->addAction(d->showMapTypeControlAction);
    floatItemsSubMenu->addAction(d->showNavigationControlAction);
    floatItemsSubMenu->addAction(d->showScaleControlAction);

    updateActionAvailability();
}

void BackendGoogleMaps::saveSettingsToGroup(KConfigGroup* const group)
{
    KMAP_ASSERT(group != 0);
    if (!group)
        return;

    group->writeEntry("GoogleMaps Map Type", getMapType());
    group->writeEntry("GoogleMaps Show Map Type Control", d->cacheShowMapTypeControl);
    group->writeEntry("GoogleMaps Show Navigation Control", d->cacheShowNavigationControl);
    group->writeEntry("GoogleMaps Show Scale Control", d->cacheShowScaleControl);
}

void BackendGoogleMaps::readSettingsFromGroup(const KConfigGroup* const group)
{
    KMAP_ASSERT(group != 0);
    if (!group)
        return;

    const QString mapType = group->readEntry("GoogleMaps Map Type", "ROADMAP");
    setMapType(mapType);
    setShowMapTypeControl(group->readEntry("GoogleMaps Show Map Type Control", true));
    setShowNavigationControl(group->readEntry("GoogleMaps Show Navigation Control", true));
    setShowScaleControl(group->readEntry("GoogleMaps Show Scale Control", true));
}

void BackendGoogleMaps::slotUngroupedModelChanged(const int mindex)
{
    KMAP_ASSERT(isReady());
    if (!isReady())
        return;

    d->htmlWidget->runScript(QString("wmwClearMarkers(%1);").arg(mindex));
    WMWModelHelper* const modelHelper = s->ungroupedModels.at(mindex);

    if (!modelHelper->modelFlags().testFlag(WMWModelHelper::FlagVisible))
            return;

    QAbstractItemModel* const model = modelHelper->model();

    for (int row = 0; row<model->rowCount(); ++row)
    {
        const QModelIndex currentIndex = model->index(row, 0);
        const WMWModelHelper::Flags itemFlags = modelHelper->itemFlags(currentIndex);

        // TODO: this is untested! We need to make sure the indices stay correct inside the JavaScript part!
        if (!itemFlags.testFlag(WMWModelHelper::FlagVisible))
            continue;

        WMWGeoCoordinate currentCoordinates;
        if (!modelHelper->itemCoordinates(currentIndex, &currentCoordinates))
            continue;

        // TODO: use the pixmap supplied by the modelHelper
        d->htmlWidget->runScript(QString("wmwAddMarker(%1, %2, %3, %4, %5, %6);")
                .arg(mindex)
                .arg(row)
                .arg(currentCoordinates.latString())
                .arg(currentCoordinates.lonString())
                .arg(itemFlags.testFlag(WMWModelHelper::FlagMovable)?"true":"false")
                .arg(itemFlags.testFlag(WMWModelHelper::FlagSnaps)?"true":"false")
            );

        QPoint markerCenterPoint;
        QPixmap markerPixmap = modelHelper->itemIcon(currentIndex, &markerCenterPoint);

        setMarkerPixmap(mindex, row, markerCenterPoint, markerPixmap);
    }

}
void BackendGoogleMaps::updateMarkers()
{
    // re-transfer all markers to the javascript-part:
    for (int i = 0; i < s->ungroupedModels.count(); ++i)
    {
        slotUngroupedModelChanged(i);
    }
}

void BackendGoogleMaps::slotHTMLEvents(const QStringList& events)
{
    // for some events, we just note that they appeared and then process them later on:
    bool centerProbablyChanged = false;
    bool mapTypeChanged = false;
    bool zoomProbablyChanged = false;
    bool mapBoundsProbablyChanged = false;
    QIntList movedClusters;
    QList<QPersistentModelIndex> movedMarkers;
    QIntList clickedClusters;
    // TODO: verify that the order of the events is still okay
    //       or that the order does not matter
    for (QStringList::const_iterator it = events.constBegin(); it!=events.constEnd(); ++it)
    {
        const QString eventCode = it->left(2);
        const QString eventParameter = it->mid(2);
        const QStringList eventParameters = eventParameter.split('/');

        if (eventCode == "MT")
        {
            // map type changed
            mapTypeChanged = true;
            d->cacheMapType = eventParameter;
        }
        else if (eventCode == "MB")
        {   // NOTE: event currently disabled in javascript part
            // map bounds changed
            centerProbablyChanged = true;
            zoomProbablyChanged = true;
            mapBoundsProbablyChanged = true;
        }
        else if (eventCode == "ZC")
        {   // NOTE: event currently disabled in javascript part
            // zoom changed
            zoomProbablyChanged = true;
            mapBoundsProbablyChanged = true;
        }
        else if (eventCode == "id")
        {
            // idle after drastic map changes
            centerProbablyChanged = true;
            zoomProbablyChanged = true;
            mapBoundsProbablyChanged = true;
        }
        else if (eventCode == "cm")
        {
            // TODO: buffer this event type!
            // cluster moved
            bool okay = false;
            const int clusterIndex = eventParameter.toInt(&okay);
            KMAP_ASSERT(okay);
            if (!okay)
                continue;

            KMAP_ASSERT(clusterIndex>=0);
            KMAP_ASSERT(clusterIndex<s->clusterList.size());
            if ((clusterIndex<0)||(clusterIndex>s->clusterList.size()))
                continue;

            // re-read the marker position:
            WMWGeoCoordinate clusterCoordinates;
            const bool isValid = d->htmlWidget->runScript2Coordinates(
                    QString("wmwGetClusterPosition(%1);").arg(clusterIndex),
                    &clusterCoordinates
                );

            KMAP_ASSERT(isValid);
            if (!isValid)
                continue;

            // TODO: this discards the altitude!
            // TODO: is this really necessary? clusters should be regenerated anyway...
            s->clusterList[clusterIndex].coordinates = clusterCoordinates;

            movedClusters << clusterIndex;
        }
        else if (eventCode == "cs")
        {
            // TODO: buffer this event type!
            // cluster snapped
            bool okay = false;
            const int clusterIndex = eventParameters.first().toInt(&okay);
            KMAP_ASSERT(okay);
            if (!okay)
                continue;

            KMAP_ASSERT(clusterIndex>=0);
            KMAP_ASSERT(clusterIndex<s->clusterList.size());
            if ((clusterIndex<0)||(clusterIndex>s->clusterList.size()))
                continue;

            // determine to which marker we snapped:
            okay = false;
            const int snapModelId = eventParameters.at(1).toInt(&okay);
            KMAP_ASSERT(okay);
            if (!okay)
                continue;
            okay = false;
            const int snapMarkerId = eventParameters.at(2).toInt(&okay);
            KMAP_ASSERT(okay);
            if (!okay)
                continue;

            // TODO: emit signal here or later?
            WMWModelHelper* const modelHelper = s->ungroupedModels.at(snapModelId);
            QAbstractItemModel* const model = modelHelper->model();
            QPair<int, QModelIndex> snapTargetIndex(snapModelId, model->index(snapMarkerId, 0));
            emit(signalClustersMoved(QIntList()<<clusterIndex, snapTargetIndex));
        }
        else if (eventCode == "cc")
        {
            // TODO: buffer this event type!
            // cluster clicked
            bool okay = false;
            const int clusterIndex = eventParameter.toInt(&okay);
            KMAP_ASSERT(okay);
            if (!okay)
                continue;

            KMAP_ASSERT(clusterIndex>=0);
            KMAP_ASSERT(clusterIndex<s->clusterList.size());
            if ((clusterIndex<0)||(clusterIndex>s->clusterList.size()))
                continue;

            clickedClusters << clusterIndex;
        }
        else if (eventCode == "mm")
        {
//             // TODO: buffer this event type!
//             // marker moved
//             bool okay = false;
//             const int markerRow= eventParameter.toInt(&okay);
//             KMAP_ASSERT(okay);
//             if (!okay)
//                 continue;
//
//             KMAP_ASSERT(markerRow>=0);
//             KMAP_ASSERT(markerRow<s->specialMarkersModel->rowCount());
//             if ((markerRow<0)||(markerRow>=s->specialMarkersModel->rowCount()))
//                 continue;
//
//             // re-read the marker position:
//             WMWGeoCoordinate markerCoordinates;
//             const bool isValid = d->htmlWidget->runScript2Coordinates(
//                     QString("wmwGetMarkerPosition(%1);").arg(markerRow),
//                     &markerCoordinates
//                 );
//
//             KMAP_ASSERT(isValid);
//             if (!isValid)
//                 continue;
//
//             // TODO: this discards the altitude!
//             const QModelIndex markerIndex = s->specialMarkersModel->index(markerRow, 0);
//             s->specialMarkersModel->setData(markerIndex, QVariant::fromValue(markerCoordinates), s->specialMarkersCoordinatesRole);
//
//             movedMarkers << QPersistentModelIndex(markerIndex);
        }
        else if (eventCode == "do")
        {
            // debug output:
            kDebug() << QString("javascript:%1").arg(eventParameter);
        }
    }

    if (!movedClusters.isEmpty())
    {
        kDebug()<<movedClusters;
        emit(signalClustersMoved(movedClusters, QPair<int, QModelIndex>(-1, QModelIndex())));
    }

    if (!movedMarkers.isEmpty())
    {
        kDebug()<<movedMarkers;
        emit(signalSpecialMarkersMoved(movedMarkers));
    }

    if (!clickedClusters.isEmpty())
    {
        kDebug()<<clickedClusters;
        emit(signalClustersClicked(clickedClusters));
    }

    // now process the buffered events:
    if (mapTypeChanged)
    {
        updateZoomMinMaxCache();
    }
    if (zoomProbablyChanged)
    {
        d->cacheZoom = d->htmlWidget->runScript(QString("wmwGetZoom();")).toInt();
        emit(signalZoomChanged(QString("googlemaps:%1").arg(d->cacheZoom)));
    }
    if (centerProbablyChanged)
    {
        // there is nothing we can do if the coordinates are invalid
        /*const bool isValid = */d->htmlWidget->runScript2Coordinates("wmwGetCenter();", &(d->cacheCenter));
    }

    // update the actions if necessary:
    if (zoomProbablyChanged||mapTypeChanged||centerProbablyChanged)
    {
        updateActionAvailability();
    }

    if (mapBoundsProbablyChanged)
    {
        const QString mapBoundsString = d->htmlWidget->runScript("wmwGetBounds();").toString();
        WMWHelperParseBoundsString(mapBoundsString, &d->cacheBounds);
    }

    if (mapBoundsProbablyChanged||!movedClusters.isEmpty())
    {
        s->worldMapWidget->markClustersAsDirty();
        s->worldMapWidget->updateClusters();
    }
}

void BackendGoogleMaps::updateClusters()
{
    kDebug() << "start updateclusters";
    // re-transfer the clusters to the map:
    KMAP_ASSERT(isReady());
    if (!isReady())
        return;

    // TODO: only update clusters that have actually changed!

    // re-transfer all markers to the javascript-part:
    d->htmlWidget->runScript(QString("wmwClearClusters();"));
    d->htmlWidget->runScript(QString("wmwSetIsInEditMode(%1);").arg(s->inEditMode?"true":"false"));
    for (int currentIndex = 0; currentIndex<s->clusterList.size(); ++currentIndex)
    {
        const WMWCluster& currentCluster = s->clusterList.at(currentIndex);

        d->htmlWidget->runScript(QString("wmwAddCluster(%1, %2, %3, %4, %5, %6);")
                .arg(currentIndex)
                .arg(currentCluster.coordinates.latString())
                .arg(currentCluster.coordinates.lonString())
                .arg(s->inEditMode?"true":"false")
                .arg(currentCluster.markerCount)
                .arg(currentCluster.markerSelectedCount)
            );

        // TODO: for now, only set generated pixmaps when not in edit mode
        // this can be changed once we figure out how to appropriately handle
        // the selection state changes when a marker is dragged
        if (!s->inEditMode)
        {
            QPoint clusterCenterPoint;
            // TODO: who calculates the override values?
            const QPixmap clusterPixmap = s->worldMapWidget->getDecoratedPixmapForCluster(currentIndex, 0, 0, &clusterCenterPoint);

            setClusterPixmap(currentIndex, clusterCenterPoint, clusterPixmap);
        }
    }
    kDebug()<<"end updateclusters";
}

bool BackendGoogleMaps::screenCoordinates(const WMWGeoCoordinate& coordinates, QPoint* const point)
{
    if (!d->isReady)
        return false;

    const bool isValid = WMWHelperParseXYStringToPoint(
            d->htmlWidget->runScript(
                QString("wmwLatLngToPixel(%1, %2);")
                    .arg(coordinates.latString())
                    .arg(coordinates.lonString())
                    ).toString(),
            point);

    // TODO: apparently, even points outside the visible area are returned as valid
    // check whether they are actually visible
    return isValid;
}

bool BackendGoogleMaps::geoCoordinates(const QPoint& point, WMWGeoCoordinate* const coordinates) const
{
    if (!d->isReady)
        return false;

    const bool isValid = d->htmlWidget->runScript2Coordinates(
            QString("wmwPixelToLatLng(%1, %2);")
                .arg(point.x())
                .arg(point.y()),
            coordinates);

    return isValid;
}

QSize BackendGoogleMaps::mapSize() const
{
    KMAP_ASSERT(d->htmlWidgetWrapper!=0);

    return d->htmlWidgetWrapper->size();
}

void BackendGoogleMaps::slotFloatSettingsTriggered(QAction* action)
{
    const QString actionIdString = action->data().toString();
    const bool actionState       = action->isChecked();

    if (actionIdString == "showmaptypecontrol")
    {
        setShowMapTypeControl(actionState);
    }
    else if (actionIdString == "shownavigationcontrol")
    {
        setShowNavigationControl(actionState);
    }
    else if (actionIdString == "showscalecontrol")
    {
        setShowScaleControl(actionState);
    }
}

void BackendGoogleMaps::setShowScaleControl(const bool state)
{
    d->cacheShowScaleControl = state;

    if (d->showScaleControlAction)
        d->showScaleControlAction->setChecked(state);

    if (!isReady())
        return;

    d->htmlWidget->runScript(QString("wmwSetShowScaleControl(%1);").arg(state?"true":"false"));
}

void BackendGoogleMaps::setShowNavigationControl(const bool state)
{
    d->cacheShowNavigationControl = state;

    if (d->showNavigationControlAction)
        d->showNavigationControlAction->setChecked(state);

    if (!isReady())
        return;

    d->htmlWidget->runScript(QString("wmwSetShowNavigationControl(%1);").arg(state?"true":"false"));
}

void BackendGoogleMaps::setShowMapTypeControl(const bool state)
{
    d->cacheShowMapTypeControl = state;

    if (d->showMapTypeControlAction)
        d->showMapTypeControlAction->setChecked(state);

    if (!isReady())
        return;

    d->htmlWidget->runScript(QString("wmwSetShowMapTypeControl(%1);").arg(state?"true":"false"));
}

void BackendGoogleMaps::slotClustersNeedUpdating()
{
    s->worldMapWidget->updateClusters();
}

void BackendGoogleMaps::setZoom(const QString& newZoom)
{
    const QString myZoomString = s->worldMapWidget->convertZoomToBackendZoom(newZoom, "googlemaps");
    KMAP_ASSERT(myZoomString.startsWith("googlemaps:"));

    const int myZoom = myZoomString.mid(QString("googlemaps:").length()).toInt();
    kDebug() << myZoom;

    d->cacheZoom = myZoom;

    if (isReady())
    {
        d->htmlWidget->runScript(QString("wmwSetZoom(%1);").arg(d->cacheZoom));
    }
}

QString BackendGoogleMaps::getZoom() const
{
    return QString("googlemaps:%1").arg(d->cacheZoom);
}

int BackendGoogleMaps::getMarkerModelLevel()
{
    KMAP_ASSERT(isReady());
    if (!isReady())
    {
        return 0;
    }

    // get the current zoom level:
    const int currentZoom = d->cacheZoom;

    int tileLevel = 0;
         if (currentZoom== 0) { tileLevel = 1; }
    else if (currentZoom== 1) { tileLevel = 1; }
    else if (currentZoom== 2) { tileLevel = 1; }
    else if (currentZoom== 3) { tileLevel = 2; }
    else if (currentZoom== 4) { tileLevel = 2; }
    else if (currentZoom== 5) { tileLevel = 3; }
    else if (currentZoom== 6) { tileLevel = 3; }
    else if (currentZoom== 7) { tileLevel = 3; }
    else if (currentZoom== 8) { tileLevel = 4; }
    else if (currentZoom== 9) { tileLevel = 4; }
    else if (currentZoom==10) { tileLevel = 4; }
    else if (currentZoom==11) { tileLevel = 4; }
    else if (currentZoom==12) { tileLevel = 4; }
    else if (currentZoom==13) { tileLevel = 4; }
    else if (currentZoom==14) { tileLevel = 5; }
    else if (currentZoom==15) { tileLevel = 5; }
    else if (currentZoom==16) { tileLevel = 5; }
    else if (currentZoom==17) { tileLevel = 5; }
    else if (currentZoom==18) { tileLevel = 6; }
    else if (currentZoom==19) { tileLevel = 6; }
    else if (currentZoom==20) { tileLevel = 6; }
    else if (currentZoom==21) { tileLevel = 7; }
    else if (currentZoom==22) { tileLevel = 7; }
    else
    {
        tileLevel = AbstractMarkerTiler::TileIndex::MaxLevel-1;
    }

    KMAP_ASSERT(tileLevel <= AbstractMarkerTiler::TileIndex::MaxLevel-1);

    return tileLevel;
}

WMWGeoCoordinate::PairList BackendGoogleMaps::getNormalizedBounds()
{
    return WMWHelperNormalizeBounds(d->cacheBounds);
}

// void BackendGoogleMaps::updateDragDropMarker(const QPoint& pos, const WMWDragData* const dragData)
// {
//     if (!isReady())
//         return;
//
//     if (!dragData)
//     {
//         d->htmlWidget->runScript("wmwRemoveDragMarker();");
//     }
//     else
//     {
//         d->htmlWidget->runScript(QString("wmwSetDragMarker(%1, %2, %3, %4);")
//                 .arg(pos.x())
//                 .arg(pos.y())
//                 .arg(dragData->itemCount)
//                 .arg(dragData->itemCount)
//             );
//     }
//
//     // TODO: hide dragged markers on the map
// }
//
// void BackendGoogleMaps::updateDragDropMarkerPosition(const QPoint& pos)
// {
//     // TODO: buffer this!
//     if (!isReady())
//         return;
//
//     d->htmlWidget->runScript(QString("wmwMoveDragMarker(%1, %2);")
//             .arg(pos.x())
//             .arg(pos.y())
//         );
// }

void BackendGoogleMaps::updateActionAvailability()
{
    if (!isReady())
        return;

    const QString currentMapType = getMapType();
    const QList<QAction*> mapTypeActions = d->mapTypeActionGroup->actions();
    for (int i=0; i<mapTypeActions.size(); ++i)
    {
        mapTypeActions.at(i)->setChecked(mapTypeActions.at(i)->data().toString()==currentMapType);
    }

    s->worldMapWidget->getControlAction("zoomin")->setEnabled(true/*d->cacheZoom<d->cacheMaxZoom*/);
    s->worldMapWidget->getControlAction("zoomout")->setEnabled(true/*d->cacheZoom>d->cacheMinZoom*/);
}

void BackendGoogleMaps::updateZoomMinMaxCache()
{
    // TODO: these functions seem to cause problems, the map is not fully updated after a few calls
//     d->cacheMaxZoom = d->htmlWidget->runScript("wmwGetMaxZoom();").toInt();
//     d->cacheMinZoom = d->htmlWidget->runScript("wmwGetMinZoom();").toInt();
}

void BackendGoogleMaps::slotThumbnailAvailableForIndex(const QVariant& index, const QPixmap& pixmap)
{
    kDebug()<<index<<pixmap.size();
    if (pixmap.isNull() || s->inEditMode)
        return;

    // TODO: properly reject pixmaps with the wrong size
    const int expectedThumbnailSize = s->worldMapWidget->getUndecoratedThumbnailSize();
    if ((pixmap.size().height()!=expectedThumbnailSize)&&(pixmap.size().width()!=expectedThumbnailSize))
        return;

    // find the cluster which is represented by this index:
    for (int i=0; i<s->clusterList.count(); ++i)
    {
        // TODO: use the right sortkey
        // TODO: let the representativeChooser handle the index comparison
        const QVariant representativeMarker = s->worldMapWidget->getClusterRepresentativeMarker(i, s->sortKey);
        if (s->markerModel->indicesEqual(index, representativeMarker))
        {
            QPoint clusterCenterPoint;
            // TODO: who calculates the override values?
            const QPixmap clusterPixmap = s->worldMapWidget->getDecoratedPixmapForCluster(i, 0, 0, &clusterCenterPoint);

            setClusterPixmap(i, clusterCenterPoint, clusterPixmap);

            break;
        }
    }
}

void BackendGoogleMaps::setClusterPixmap(const int clusterId, const QPoint& centerPoint, const QPixmap& clusterPixmap)
{
    // decorate the pixmap:
    const QPixmap styledPixmap = clusterPixmap;

    QByteArray bytes;
    QBuffer buffer(&bytes);
    buffer.open(QIODevice::WriteOnly);
    clusterPixmap.save(&buffer, "PNG");

    // http://www.faqs.org/rfcs/rfc2397.html
    const QString imageData = QString("data:image/png;base64,%1").arg(QString::fromAscii(bytes.toBase64()));
    d->htmlWidget->runScript(QString("wmwSetClusterPixmap(%1,%5,%6,%2,%3,'%4');")
                    .arg(clusterId)
                    .arg(centerPoint.x())
                    .arg(clusterPixmap.height()-centerPoint.y())
                    .arg(imageData)
                    .arg(clusterPixmap.width())
                    .arg(clusterPixmap.height())
                );
}

void BackendGoogleMaps::setMarkerPixmap(const int modelId, const int markerId, const QPoint& centerPoint, const QPixmap& markerPixmap)
{
    QByteArray bytes;
    QBuffer buffer(&bytes);
    buffer.open(QIODevice::WriteOnly);
    markerPixmap.save(&buffer, "PNG");

    // http://www.faqs.org/rfcs/rfc2397.html
    const QString imageData = QString("data:image/png;base64,%1").arg(QString::fromAscii(bytes.toBase64()));
    d->htmlWidget->runScript(QString("wmwSetMarkerPixmap(%7,%1,%5,%6,%2,%3,'%4');")
                    .arg(markerId)
                    .arg(centerPoint.x())
                    .arg(markerPixmap.height()-centerPoint.y())
                    .arg(imageData)
                    .arg(markerPixmap.width())
                    .arg(markerPixmap.height())
                    .arg(modelId)
                );
}

bool BackendGoogleMaps::eventFilter(QObject* object, QEvent* event)
{
    if (object==d->htmlWidgetWrapper)
    {
        if (event->type()==QEvent::Resize)
        {
            QResizeEvent* const resizeEvent = dynamic_cast<QResizeEvent*>(event);
            if (resizeEvent)
            {
                // TODO: the map div does not adjust its height properly if height=100%,
                //       therefore we adjust it manually here
                if (d->isReady)
                {
                    d->htmlWidget->runScript(QString("wmwWidgetResized(%1, %2)").arg(d->htmlWidgetWrapper->width()).arg(d->htmlWidgetWrapper->height()));
                }
            }
        }
    }
    return false;
}

void BackendGoogleMaps::setSelectionRectangle(const QList<double>& searchCoordinates)
{
    d->htmlWidget->setSelectionRectangle(searchCoordinates);
}

QList<qreal> BackendGoogleMaps::getSelectionRectangle()
{
    return d->htmlWidget->getSelectionRectangle();
}

void BackendGoogleMaps::removeSelectionRectangle()
{
    d->htmlWidget->removeSelectionRectangle();
}

void BackendGoogleMaps::mouseModeChanged(MouseMode mouseMode)
{

    d->htmlWidget->mouseModeChanged(mouseMode);
/*
    if(mouseMode == MouseModeSelection)
    {
       // d->htmlWidget->runScript(QString("selectionModeStatus(true)"));
        d->htmlWidget->mouseModeChanged(true);
    }
    else //MousePan
    {
       // d->htmlWidget->runScript(QString("selectionModeStatus(false)"));
        d->htmlWidget->mouseModeChanged(false);
    }
*/

}

void BackendGoogleMaps::slotSelectionHasBeenMade(const QList<double>& searchCoordinates)
{   
    emit signalSelectionHasBeenMade(searchCoordinates);
}

void BackendGoogleMaps::centerOn( const Marble::GeoDataLatLonBox& latLonBox)
{
    const qreal boxWest  = latLonBox.west(Marble::GeoDataCoordinates::Degree);
    const qreal boxNorth = latLonBox.north(Marble::GeoDataCoordinates::Degree);
    const qreal boxEast  = latLonBox.east(Marble::GeoDataCoordinates::Degree);
    const qreal boxSouth = latLonBox.south(Marble::GeoDataCoordinates::Degree);

    d->htmlWidget->centerOn(boxWest, boxNorth, boxEast, boxSouth);
}

} /* namespace KMapIface */
