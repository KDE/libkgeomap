/** ===========================================================
 * @file
 *
 * This file is a part of digiKam project
 * <a href="http://www.digikam.org">http://www.digikam.org</a>
 *
 * @date   2009-12-01
 * @brief  Google-Maps-backend for KMap
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

#include "backend_map_googlemaps.moc"

// Qt includes

#include <QBuffer>
#include <QActionGroup>
#include <QMenu>
#include <QPointer>

// KDE includes

#include <kaction.h>
#include <kconfiggroup.h>
#include <khtml_part.h>

// local includes

#include "html_widget.h"
#include "kmap_widget.h"
#include "abstractmarkertiler.h"
#include "modelhelper.h"

namespace KMap
{

class GMInternalWidgetInfo
{
public:
    HTMLWidget* htmlWidget;
};

} /* KMap */

Q_DECLARE_METATYPE(KMap::GMInternalWidgetInfo)

namespace KMap
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
      cacheMapType(QLatin1String("ROADMAP")),
      cacheShowMapTypeControl(true),
      cacheShowNavigationControl(true),
      cacheShowScaleControl(true),
      cacheZoom(1),
      cacheMaxZoom(0),
      cacheMinZoom(0),
      cacheCenter(0.0,0.0),
      cacheBounds(),
      activeState(false),
      widgetIsDocked(false)
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
    GeoCoordinates                            cacheCenter;
    QPair<GeoCoordinates, GeoCoordinates>     cacheBounds;
    bool                                      activeState;
    bool                                      widgetIsDocked;
};

BackendGoogleMaps::BackendGoogleMaps(const QExplicitlySharedDataPointer<KMapSharedData>& sharedData, QObject* const parent)
                 : MapBackend(sharedData, parent), d(new BackendGoogleMapsPrivate())
{
    createActions();
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
    mapTypes
        << QLatin1String("ROADMAP")
        << QLatin1String("SATELLITE")
        << QLatin1String("HYBRID")
        << QLatin1String("TERRAIN");

    mapTypesHumanNames
        << i18n("Roadmap")
        << i18n("Satellite")
        << i18n("Hybrid")
        << i18n("Terrain");

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
    d->showMapTypeControlAction->setData(QLatin1String("showmaptypecontrol"));

    d->showNavigationControlAction = new KAction(i18n("Show Navigation Control"), d->floatItemsActionGroup);
    d->showNavigationControlAction->setCheckable(true);
    d->showNavigationControlAction->setChecked(d->cacheShowNavigationControl);
    d->showNavigationControlAction->setData(QLatin1String("shownavigationcontrol"));

    d->showScaleControlAction = new KAction(i18n("Show Scale Control"), d->floatItemsActionGroup);
    d->showScaleControlAction->setCheckable(true);
    d->showScaleControlAction->setChecked(d->cacheShowScaleControl);
    d->showScaleControlAction->setData(QLatin1String("showscalecontrol"));
}

QString BackendGoogleMaps::backendName() const
{
    return QLatin1String("googlemaps");
}

QString BackendGoogleMaps::backendHumanName() const
{
    return i18n("Google Maps");
}

QWidget* BackendGoogleMaps::mapWidget()
{
    if (!d->htmlWidgetWrapper)
    {
        KMapGlobalObject* const go = KMapGlobalObject::instance();

        KMapInternalWidgetInfo info;
        bool foundReusableWidget = go->getInternalWidgetFromPool(this, &info);
        if (foundReusableWidget)
        {
            d->htmlWidgetWrapper = info.widget;
            const GMInternalWidgetInfo intInfo = info.backendData.value<GMInternalWidgetInfo>();
            d->htmlWidget = intInfo.htmlWidget;
        }
        else
        {
            // the widget has not been created yet, create it now:
            d->htmlWidgetWrapper = new QWidget();
            d->htmlWidgetWrapper->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
            d->htmlWidget        = new HTMLWidget(d->htmlWidgetWrapper);
            d->htmlWidgetWrapper->resize(400,400);
        }

        connect(d->htmlWidget, SIGNAL(signalJavaScriptReady()),
                this, SLOT(slotHTMLInitialized()));

        connect(d->htmlWidget, SIGNAL(signalHTMLEvents(const QStringList&)),
                this, SLOT(slotHTMLEvents(const QStringList&)));

        connect(d->htmlWidget, SIGNAL(selectionHasBeenMade(const KMap::GeoCoordinates::Pair&)),
                this, SLOT(slotSelectionHasBeenMade(const KMap::GeoCoordinates::Pair&)));

        d->htmlWidget->setSharedKMapObject(s.data());
        d->htmlWidgetWrapper->installEventFilter(this);

        if (foundReusableWidget)
        {
            slotHTMLInitialized();
        }
        else
        {
            const KUrl htmlUrl = KMapGlobalObject::instance()->locateDataFile(QLatin1String("backend-googlemaps.html"));

            d->htmlWidget->openUrl(htmlUrl);
        }
    }

    return d->htmlWidgetWrapper.data();
}

GeoCoordinates BackendGoogleMaps::getCenter() const
{
    return d->cacheCenter;
}

void BackendGoogleMaps::setCenter(const GeoCoordinates& coordinate)
{
    d->cacheCenter = coordinate;

    if (isReady())
    {
        d->htmlWidget->runScript(QString::fromLatin1("kmapSetCenter(%1, %2);").arg(d->cacheCenter.latString()).arg(d->cacheCenter.lonString()));
    }
}

bool BackendGoogleMaps::isReady() const
{
    return d->isReady;
}

void BackendGoogleMaps::slotHTMLInitialized()
{
    d->isReady = true;
    d->htmlWidget->runScript(QString::fromLatin1("kmapWidgetResized(%1, %2)").arg(d->htmlWidgetWrapper->width()).arg(d->htmlWidgetWrapper->height()));

    // TODO: call javascript directly here and update action availability in one shot
    setMapType(d->cacheMapType);
    setShowMapTypeControl(d->cacheShowMapTypeControl);
    setShowNavigationControl(d->cacheShowNavigationControl);
    setShowScaleControl(d->cacheShowNavigationControl);
    setCenter(d->cacheCenter);
    d->htmlWidget->runScript(QString::fromLatin1("kmapSetZoom(%1);").arg(d->cacheZoom));
    emit(signalBackendReadyChanged(backendName()));
}

void BackendGoogleMaps::zoomIn()
{
    if (!d->isReady)
        return;

    d->htmlWidget->runScript(QLatin1String("kmapZoomIn();"));
}

void BackendGoogleMaps::zoomOut()
{
    if (!d->isReady)
        return;

    d->htmlWidget->runScript(QLatin1String("kmapZoomOut();"));
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
        d->htmlWidget->runScript(QString::fromLatin1("kmapSetMapType(\"%1\");").arg(newMapType));
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

    d->htmlWidget->runScript(QString::fromLatin1("kmapClearMarkers(%1);").arg(mindex));

    // this can happen when a model was removed and we are simply asked to remove its markers
    if (mindex>s->ungroupedModels.count())
        return;

    ModelHelper* const modelHelper = s->ungroupedModels.at(mindex);
    if (!modelHelper)
        return;

    if (!modelHelper->modelFlags().testFlag(ModelHelper::FlagVisible))
            return;

    QAbstractItemModel* const model = modelHelper->model();

    for (int row = 0; row<model->rowCount(); ++row)
    {
        const QModelIndex currentIndex = model->index(row, 0);
        const ModelHelper::Flags itemFlags = modelHelper->itemFlags(currentIndex);

        // TODO: this is untested! We need to make sure the indices stay correct inside the JavaScript part!
        if (!itemFlags.testFlag(ModelHelper::FlagVisible))
            continue;

        GeoCoordinates currentCoordinates;
        if (!modelHelper->itemCoordinates(currentIndex, &currentCoordinates))
            continue;

        // TODO: use the pixmap supplied by the modelHelper
        d->htmlWidget->runScript(QString::fromLatin1("kmapAddMarker(%1, %2, %3, %4, %5, %6);")
                .arg(mindex)
                .arg(row)
                .arg(currentCoordinates.latString())
                .arg(currentCoordinates.lonString())
                .arg(itemFlags.testFlag(ModelHelper::FlagMovable)?QLatin1String("true" ):QLatin1String("false"))
                .arg(itemFlags.testFlag(ModelHelper::FlagSnaps)?QLatin1String("true" ):QLatin1String("false"))
            );

        QPoint markerCenterPoint;
        QSize markerSize;
        QPixmap markerPixmap;
        KUrl markerUrl;
        const bool markerHasIcon = modelHelper->itemIcon(currentIndex, &markerCenterPoint,
                                                         &markerSize, &markerPixmap, &markerUrl);

        if (markerHasIcon)
        {
            if (!markerUrl.isEmpty())
            {
                setMarkerPixmap(mindex, row, markerCenterPoint, markerSize, markerUrl);
            }
            else
            {
                setMarkerPixmap(mindex, row, markerCenterPoint, markerPixmap);
            }
        }
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
        const QStringList eventParameters = eventParameter.split(QLatin1Char( '/' ));

        if (eventCode == QLatin1String("MT"))
        {
            // map type changed
            mapTypeChanged = true;
            d->cacheMapType = eventParameter;
        }
        else if (eventCode == QLatin1String("MB"))
        {   // NOTE: event currently disabled in javascript part
            // map bounds changed
            centerProbablyChanged = true;
            zoomProbablyChanged = true;
            mapBoundsProbablyChanged = true;
        }
        else if (eventCode == QLatin1String("ZC"))
        {   // NOTE: event currently disabled in javascript part
            // zoom changed
            zoomProbablyChanged = true;
            mapBoundsProbablyChanged = true;
        }
        else if (eventCode == QLatin1String("id"))
        {
            // idle after drastic map changes
            centerProbablyChanged = true;
            zoomProbablyChanged = true;
            mapBoundsProbablyChanged = true;
        }
        else if (eventCode == QLatin1String("cm"))
        {
            /// @todo buffer this event type!
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
            GeoCoordinates clusterCoordinates;
            const bool isValid = d->htmlWidget->runScript2Coordinates(
                    QString::fromLatin1("kmapGetClusterPosition(%1);").arg(clusterIndex),
                    &clusterCoordinates
                );

            KMAP_ASSERT(isValid);
            if (!isValid)
                continue;

            /// @todo this discards the altitude!
            /// @todo is this really necessary? clusters should be regenerated anyway...
            s->clusterList[clusterIndex].coordinates = clusterCoordinates;

            movedClusters << clusterIndex;
        }
        else if (eventCode == QLatin1String("cs"))
        {
            /// @todo buffer this event type!
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

            /// @todo emit signal here or later?
            ModelHelper* const modelHelper = s->ungroupedModels.at(snapModelId);
            QAbstractItemModel* const model = modelHelper->model();
            QPair<int, QModelIndex> snapTargetIndex(snapModelId, model->index(snapMarkerId, 0));
            emit(signalClustersMoved(QIntList()<<clusterIndex, snapTargetIndex));
        }
        else if (eventCode == QLatin1String("cc"))
        {
            /// @todo buffer this event type!
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
        else if (eventCode == QLatin1String("mm"))
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
//             GeoCoordinates markerCoordinates;
//             const bool isValid = d->htmlWidget->runScript2Coordinates(
//                     QString::fromLatin1("kmapGetMarkerPosition(%1);").arg(markerRow),
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
        else if (eventCode == QLatin1String("do"))
        {
            // debug output:
            kDebug() << QString::fromLatin1("javascript:%1").arg(eventParameter);
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
//         emit(signalSpecialMarkersMoved(movedMarkers));
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
        d->cacheZoom = d->htmlWidget->runScript(QLatin1String("kmapGetZoom();")).toInt();
        emit(signalZoomChanged(QString::fromLatin1("googlemaps:%1").arg(d->cacheZoom)));
    }
    if (centerProbablyChanged)
    {
        // there is nothing we can do if the coordinates are invalid
        /*const bool isValid = */d->htmlWidget->runScript2Coordinates(QLatin1String("kmapGetCenter();"), &(d->cacheCenter));
    }

    // update the actions if necessary:
    if (zoomProbablyChanged||mapTypeChanged||centerProbablyChanged)
    {
        updateActionAvailability();
    }

    if (mapBoundsProbablyChanged)
    {
        const QString mapBoundsString = d->htmlWidget->runScript(QLatin1String("kmapGetBounds();")).toString();
        KMapHelperParseBoundsString(mapBoundsString, &d->cacheBounds);
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
    const bool canMoveItems = s->modificationsAllowed && s->markerModel->tilerFlags().testFlag(AbstractMarkerTiler::FlagMovable) && !s->showThumbnails;
    d->htmlWidget->runScript(QLatin1String("kmapClearClusters();"));
    d->htmlWidget->runScript(QString::fromLatin1("kmapSetIsInEditMode(%1);").arg(s->showThumbnails?QLatin1String("false" ):QLatin1String("true" )));
    for (int currentIndex = 0; currentIndex<s->clusterList.size(); ++currentIndex)
    {
        const KMapCluster& currentCluster = s->clusterList.at(currentIndex);

        d->htmlWidget->runScript(QString::fromLatin1("kmapAddCluster(%1, %2, %3, %4, %5, %6);")
                .arg(currentIndex)
                .arg(currentCluster.coordinates.latString())
                .arg(currentCluster.coordinates.lonString())
                .arg(canMoveItems ? QLatin1String("true") : QLatin1String("false"))
                .arg(currentCluster.markerCount)
                .arg(currentCluster.markerSelectedCount)
            );

        // TODO: for now, only set generated pixmaps when not in edit mode
        // this can be changed once we figure out how to appropriately handle
        // the selection state changes when a marker is dragged
        if (s->showThumbnails)
        {
            QPoint clusterCenterPoint;
            // TODO: who calculates the override values?
            const QPixmap clusterPixmap = s->worldMapWidget->getDecoratedPixmapForCluster(currentIndex, 0, 0, &clusterCenterPoint);

            setClusterPixmap(currentIndex, clusterCenterPoint, clusterPixmap);
        }
    }
    kDebug()<<"end updateclusters";
}

bool BackendGoogleMaps::screenCoordinates(const GeoCoordinates& coordinates, QPoint* const point)
{
    if (!d->isReady)
        return false;

    const bool isValid = KMapHelperParseXYStringToPoint(
            d->htmlWidget->runScript(
                QString::fromLatin1("kmapLatLngToPixel(%1, %2);")
                    .arg(coordinates.latString())
                    .arg(coordinates.lonString())
                    ).toString(),
            point);

    // TODO: apparently, even points outside the visible area are returned as valid
    // check whether they are actually visible
    return isValid;
}

bool BackendGoogleMaps::geoCoordinates(const QPoint& point, GeoCoordinates* const coordinates) const
{
    if (!d->isReady)
        return false;

    const bool isValid = d->htmlWidget->runScript2Coordinates(
        QString::fromLatin1("kmapPixelToLatLng(%1, %2);")
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

    if (actionIdString == QLatin1String("showmaptypecontrol"))
    {
        setShowMapTypeControl(actionState);
    }
    else if (actionIdString == QLatin1String("shownavigationcontrol"))
    {
        setShowNavigationControl(actionState);
    }
    else if (actionIdString == QLatin1String("showscalecontrol"))
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

    d->htmlWidget->runScript(
        QString::fromLatin1("kmapSetShowScaleControl(%1);")
        .arg(state ? QLatin1String("true") : QLatin1String("false")));
}

void BackendGoogleMaps::setShowNavigationControl(const bool state)
{
    d->cacheShowNavigationControl = state;

    if (d->showNavigationControlAction)
        d->showNavigationControlAction->setChecked(state);

    if (!isReady())
        return;

    d->htmlWidget->runScript(
        QString::fromLatin1("kmapSetShowNavigationControl(%1);")
        .arg(state ? QLatin1String("true") : QLatin1String("false")));
}

void BackendGoogleMaps::setShowMapTypeControl(const bool state)
{
    d->cacheShowMapTypeControl = state;

    if (d->showMapTypeControlAction)
        d->showMapTypeControlAction->setChecked(state);

    if (!isReady())
        return;

    d->htmlWidget->runScript(
        QString::fromLatin1("kmapSetShowMapTypeControl(%1);")
        .arg(state ? QLatin1String("true") : QLatin1String("false")));
}

void BackendGoogleMaps::slotClustersNeedUpdating()
{
    s->worldMapWidget->updateClusters();
}

void BackendGoogleMaps::setZoom(const QString& newZoom)
{
    const QString myZoomString = s->worldMapWidget->convertZoomToBackendZoom(newZoom, QLatin1String("googlemaps"));
    KMAP_ASSERT(myZoomString.startsWith(QLatin1String("googlemaps:")));

    const int myZoom = myZoomString.mid(QString::fromLatin1("googlemaps:").length()).toInt();
    kDebug() << myZoom;

    d->cacheZoom = myZoom;

    if (isReady())
    {
        d->htmlWidget->runScript(QString::fromLatin1("kmapSetZoom(%1);").arg(d->cacheZoom));
    }
}

QString BackendGoogleMaps::getZoom() const
{
    return QString::fromLatin1("googlemaps:%1").arg(d->cacheZoom);
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
    else if (currentZoom==16) { tileLevel = 6; }
    else if (currentZoom==17) { tileLevel = 7; }
    else if (currentZoom==18) { tileLevel = 7; }
    else if (currentZoom==19) { tileLevel = 8; }
    else if (currentZoom==20) { tileLevel = 9; }
    else if (currentZoom==21) { tileLevel = 9; }
    else if (currentZoom==22) { tileLevel = 9; }
    else
    {
        tileLevel = TileIndex::MaxLevel-1;
    }

    KMAP_ASSERT(tileLevel <= TileIndex::MaxLevel-1);

    return tileLevel;
}

GeoCoordinates::PairList BackendGoogleMaps::getNormalizedBounds()
{
    return KMapHelperNormalizeBounds(d->cacheBounds);
}

// void BackendGoogleMaps::updateDragDropMarker(const QPoint& pos, const KMapDragData* const dragData)
// {
//     if (!isReady())
//         return;
//
//     if (!dragData)
//     {
//         d->htmlWidget->runScript("kmapRemoveDragMarker();");
//     }
//     else
//     {
//         d->htmlWidget->runScript(QLatin1String("kmapSetDragMarker(%1, %2, %3, %4);")
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
//     d->htmlWidget->runScript(QLatin1String("kmapMoveDragMarker(%1, %2);")
//             .arg(pos.x())
//             .arg(pos.y())
//         );
// }

void BackendGoogleMaps::updateActionAvailability()
{
    if ( (!d->activeState) || (!isReady()) )
    {
        return;
    }

    const QString currentMapType = getMapType();
    const QList<QAction*> mapTypeActions = d->mapTypeActionGroup->actions();
    for (int i=0; i<mapTypeActions.size(); ++i)
    {
        mapTypeActions.at(i)->setChecked(mapTypeActions.at(i)->data().toString()==currentMapType);
    }

    s->worldMapWidget->getControlAction(QLatin1String("zoomin"))->setEnabled(true/*d->cacheZoom<d->cacheMaxZoom*/);
    s->worldMapWidget->getControlAction(QLatin1String("zoomout"))->setEnabled(true/*d->cacheZoom>d->cacheMinZoom*/);
}

void BackendGoogleMaps::updateZoomMinMaxCache()
{
    // TODO: these functions seem to cause problems, the map is not fully updated after a few calls
//     d->cacheMaxZoom = d->htmlWidget->runScript("kmapGetMaxZoom();").toInt();
//     d->cacheMinZoom = d->htmlWidget->runScript("kmapGetMinZoom();").toInt();
}

void BackendGoogleMaps::slotThumbnailAvailableForIndex(const QVariant& index, const QPixmap& pixmap)
{
    kDebug()<<index<<pixmap.size();
    if (pixmap.isNull() || !s->showThumbnails)
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
    const QString imageData = QString::fromLatin1("data:image/png;base64,%1").arg(QString::fromAscii(bytes.toBase64()));
    d->htmlWidget->runScript(QString::fromLatin1("kmapSetClusterPixmap(%1,%5,%6,%2,%3,'%4');")
                    .arg(clusterId)
                    .arg(centerPoint.x())
                    .arg(centerPoint.y())
                    .arg(imageData)
                    .arg(clusterPixmap.width())
                    .arg(clusterPixmap.height())
                );
}

void BackendGoogleMaps::setMarkerPixmap(const int modelId, const int markerId,
                                        const QPoint& centerPoint, const QPixmap& markerPixmap)
{
    QByteArray bytes;
    QBuffer buffer(&bytes);
    buffer.open(QIODevice::WriteOnly);
    markerPixmap.save(&buffer, "PNG");

    // http://www.faqs.org/rfcs/rfc2397.html
    const QString imageData = QString::fromLatin1("data:image/png;base64,%1").arg(QString::fromAscii(bytes.toBase64()));
    d->htmlWidget->runScript(QString::fromLatin1("kmapSetMarkerPixmap(%7,%1,%5,%6,%2,%3,'%4');")
                    .arg(markerId)
                    .arg(centerPoint.x())
                    .arg(centerPoint.y())
                    .arg(imageData)
                    .arg(markerPixmap.width())
                    .arg(markerPixmap.height())
                    .arg(modelId)
                );
}

void BackendGoogleMaps::setMarkerPixmap(const int modelId, const int markerId,
                                        const QPoint& centerPoint, const QSize& iconSize,
                                        const KUrl& iconUrl
                                       )
{
    /// @todo Sort the parameters
    d->htmlWidget->runScript(QString::fromLatin1("kmapSetMarkerPixmap(%7,%1,%5,%6,%2,%3,'%4');")
                    .arg(markerId)
                    .arg(centerPoint.x())
                    .arg(centerPoint.y())
                    .arg(iconUrl.url()) /// @todo Escape characters like apostrophe
                    .arg(iconSize.width())
                    .arg(iconSize.height())
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
                    d->htmlWidget->runScript(QString::fromLatin1("kmapWidgetResized(%1, %2)").arg(d->htmlWidgetWrapper->width()).arg(d->htmlWidgetWrapper->height()));
                }
            }
        }
    }
    return false;
}

void BackendGoogleMaps::regionSelectionChanged()
{
    if (!d->htmlWidget)
    {
        return;
    }

    if (s->hasRegionSelection())
    {
        d->htmlWidget->setSelectionRectangle(s->selectionRectangle);
    }
    else
    {
        d->htmlWidget->removeSelectionRectangle();
    }
}

void BackendGoogleMaps::mouseModeChanged()
{
    if (!d->htmlWidget)
    {
        return;
    }

    /// @todo Does htmlwidget read this value from s->currentMouseMode on its own?
    d->htmlWidget->mouseModeChanged(s->currentMouseMode);
}

void BackendGoogleMaps::slotSelectionHasBeenMade(const KMap::GeoCoordinates::Pair& searchCoordinates)
{
    emit signalSelectionHasBeenMade(searchCoordinates);
}

void BackendGoogleMaps::centerOn( const Marble::GeoDataLatLonBox& latLonBox, const bool useSaneZoomLevel)
{
    /// @todo Buffer this call if there is no widget or if inactive!
    if (!d->htmlWidget)
    {
        return;
    }

    const qreal boxWest  = latLonBox.west(Marble::GeoDataCoordinates::Degree);
    const qreal boxNorth = latLonBox.north(Marble::GeoDataCoordinates::Degree);
    const qreal boxEast  = latLonBox.east(Marble::GeoDataCoordinates::Degree);
    const qreal boxSouth = latLonBox.south(Marble::GeoDataCoordinates::Degree);

    d->htmlWidget->centerOn(boxWest, boxNorth, boxEast, boxSouth, useSaneZoomLevel);
    kDebug()<<getZoom();
}

void BackendGoogleMaps::setActive(const bool state)
{
    const bool oldState = d->activeState;
    d->activeState = state;

    if (oldState!=state)
    {
        if ((!state)&&d->htmlWidgetWrapper)
        {
            // we should share our widget in the list of widgets in the global object
            KMapInternalWidgetInfo info;
            info.deleteFunction = deleteInfoFunction;
            info.widget = d->htmlWidgetWrapper.data();
            info.currentOwner = this;
            info.backendName = backendName();
            info.state = d->widgetIsDocked ? KMapInternalWidgetInfo::InternalWidgetStillDocked : KMapInternalWidgetInfo::InternalWidgetUndocked;

            GMInternalWidgetInfo intInfo;
            intInfo.htmlWidget = d->htmlWidget.data();
            info.backendData.setValue(intInfo);

            KMapGlobalObject* const go = KMapGlobalObject::instance();
            go->addMyInternalWidgetToPool(info);
        }

        if (state&&d->htmlWidgetWrapper)
        {
            // we should remove our widget from the list of widgets in the global object
            KMapGlobalObject* const go = KMapGlobalObject::instance();
            go->removeMyInternalWidgetFromPool(this);

            /// @todo re-cluster, update markers?
            setCenter(d->cacheCenter);
            setMapType(d->cacheMapType);
            setShowMapTypeControl(d->cacheShowMapTypeControl);
            setShowNavigationControl(d->cacheShowNavigationControl);
            setShowScaleControl(d->cacheShowScaleControl);
        }
    }
}

void BackendGoogleMaps::releaseWidget(KMapInternalWidgetInfo* const info)
{
    disconnect(d->htmlWidget, SIGNAL(signalJavaScriptReady()),
               this, SLOT(slotHTMLInitialized()));

    disconnect(d->htmlWidget, SIGNAL(signalHTMLEvents(const QStringList&)),
               this, SLOT(slotHTMLEvents(const QStringList&)));

    disconnect(d->htmlWidget, SIGNAL(selectionHasBeenMade(const KMap::GeoCoordinates::Pair&)),
               this, SLOT(slotSelectionHasBeenMade(const KMap::GeoCoordinates::Pair&)));

    d->htmlWidget->setSharedKMapObject(0);
    d->htmlWidgetWrapper->removeEventFilter(this);

    d->htmlWidget = 0;
    d->htmlWidgetWrapper = 0;

    info->currentOwner = 0;
    info->state = KMapInternalWidgetInfo::InternalWidgetReleased;

    d->isReady = false;
    emit(signalBackendReadyChanged(backendName()));
}

void BackendGoogleMaps::mapWidgetDocked(const bool state)
{
    if (d->widgetIsDocked!=state)
    {
        KMapGlobalObject* const go = KMapGlobalObject::instance();
        go->updatePooledWidgetState(d->htmlWidgetWrapper, state ? KMapInternalWidgetInfo::InternalWidgetStillDocked : KMapInternalWidgetInfo::InternalWidgetUndocked);
    }
    d->widgetIsDocked = state;
}

void BackendGoogleMaps::deleteInfoFunction(KMapInternalWidgetInfo* const info)
{
    if (info->currentOwner)
    {
        qobject_cast<MapBackend*>(info->currentOwner.data())->releaseWidget(info);
    }

    const GMInternalWidgetInfo intInfo = info->backendData.value<GMInternalWidgetInfo>();
    delete intInfo.htmlWidget;

    delete info->widget.data();
}
} /* namespace KMap */
