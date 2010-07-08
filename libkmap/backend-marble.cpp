/** ===========================================================
 *
 * This file is a part of digiKam project
 * <a href="http://www.digikam.org">http://www.digikam.org</a>
 *
 * @date   2009-12-08
 * @brief  Marble-backend for WorldMapWidget2
 *
 * @author Copyright (C) 2009-2010 by Michael G. Hansen
 *         <a href="mailto:mike at mghansen dot de">mike at mghansen dot de</a>
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

#include "backend-marble.moc"

// Qt includes

#include <QMenu>
#include <QMouseEvent>
#include <QPointer>

// KDE includes

#include <kaction.h>
#include <kconfiggroup.h>
#include <klocale.h>
#include <kstandarddirs.h>

// Marble widget includes

#include <marble/GeoPainter.h>
#include <marble/MarbleMap.h>
#include <marble/MarbleModel.h>
#include <marble/MarbleWidget.h>
#include <marble/ViewParams.h>
#include <marble/ViewportParams.h>

// local includes

#ifdef KMAP_MARBLE_ADD_LAYER
#include "backend-marble-layer.h"
#else
#include "backend-marble-subwidget.h"
#endif // KMAP_MARBLE_ADD_LAYER

#include "markermodel.h"
#include "kmap.h"

using namespace Marble;

namespace KMapIface
{

class BackendMarblePrivate
{
public:
    BackendMarblePrivate()
    : marbleWidget(0),
      actionGroupMapTheme(0),
      actionGroupProjection(0),
      actionGroupFloatItems(0),
      actionShowCompass(0),
      actionShowOverviewMap(0),
      actionShowScaleBar(0),
      cacheMapTheme("atlas"),
      cacheProjection("spherical"),
      cacheShowCompass(false),
      cacheShowScaleBar(false),
      cacheShowOverviewMap(false),
      cacheZoom(900),
      havePotentiallyMouseMovingObject(false),
      haveMouseMovingObject(false),
      mouseMoveClusterIndex(-1),
      mouseMoveMarkerIndex(),
      mouseMoveObjectCoordinates(),
      mouseMoveCenterOffset(0,0),
      dragDropMarkerCount(0),
      dragDropMarkerPos(),
      clustersDirtyCacheProjection(),
      clustersDirtyCacheLat(),
      clustersDirtyCacheLon()
    {
    }

    QPointer<MarbleWidget> marbleWidget;

    QActionGroup*          actionGroupMapTheme;
    QActionGroup*          actionGroupProjection;
    QActionGroup*          actionGroupFloatItems;
    KAction*               actionShowCompass;
    KAction*               actionShowOverviewMap;
    KAction*               actionShowScaleBar;

    QString                cacheMapTheme;
    QString                cacheProjection;
    bool                   cacheShowCompass;
    bool                   cacheShowScaleBar;
    bool                   cacheShowOverviewMap;
    int                    cacheZoom;
    bool                   havePotentiallyMouseMovingObject;
    bool                   haveMouseMovingObject;
    int                    mouseMoveClusterIndex;
    QPersistentModelIndex  mouseMoveMarkerIndex;
    WMWGeoCoordinate       mouseMoveObjectCoordinates;
    QPoint                 mouseMoveCenterOffset;
    int                    dragDropMarkerCount;
    QPoint                 dragDropMarkerPos;
    int                    clustersDirtyCacheProjection;
    qreal                  clustersDirtyCacheLat;
    qreal                  clustersDirtyCacheLon;
};

BackendMarble::BackendMarble(const QExplicitlySharedDataPointer<WMWSharedData>& sharedData, QObject* const parent)
             : MapBackend(sharedData, parent), d(new BackendMarblePrivate())
{
    createActions();

#ifdef KMAP_MARBLE_ADD_LAYER
    d->marbleWidget = new MarbleWidget();
    d->marbleWidget->model()->addLayer(new BMLayer(this));
#else
    d->marbleWidget = new BMWidget(this);
#endif

    d->marbleWidget->installEventFilter(this);

    connect(d->marbleWidget, SIGNAL(zoomChanged(int)),
            this, SLOT(slotMarbleZoomChanged(int)));

    // set a backend first
    setMapTheme(d->cacheMapTheme);

    emit(signalBackendReady(backendName()));
}

BackendMarble::~BackendMarble()
{
    if (d->marbleWidget)
        delete d->marbleWidget;

    delete d;
}

QString BackendMarble::backendName() const
{
    return QString("marble");
}

QString BackendMarble::backendHumanName() const
{
    return i18n("Marble Desktop Globe");
}

QWidget* BackendMarble::mapWidget() const
{
    return d->marbleWidget;
}

WMWGeoCoordinate BackendMarble::getCenter() const
{
    return WMWGeoCoordinate(d->marbleWidget->centerLatitude(), d->marbleWidget->centerLongitude());
}

void BackendMarble::setCenter(const WMWGeoCoordinate& coordinate)
{
    d->marbleWidget->setCenterLatitude(coordinate.lat());
    d->marbleWidget->setCenterLongitude(coordinate.lon());
}

bool BackendMarble::isReady() const
{
    return true;
}

void BackendMarble::zoomIn()
{
    d->marbleWidget->zoomIn();
    d->marbleWidget->repaint();
}

void BackendMarble::zoomOut()
{
    d->marbleWidget->zoomOut();
    d->marbleWidget->repaint();
}

void BackendMarble::createActions()
{
    // map theme:
    d->actionGroupMapTheme = new QActionGroup(this);
    d->actionGroupMapTheme->setExclusive(true);

    connect(d->actionGroupMapTheme, SIGNAL(triggered(QAction*)),
            this, SLOT(slotMapThemeActionTriggered(QAction*)));

    KAction* const actionAtlas = new KAction(d->actionGroupMapTheme);
    actionAtlas->setCheckable(true);
    actionAtlas->setText(i18n("Atlas map"));
    actionAtlas->setData("atlas");

    KAction* const actionOpenStreetmap = new KAction(d->actionGroupMapTheme);
    actionOpenStreetmap->setCheckable(true);
    actionOpenStreetmap->setText(i18n("OpenStreetMap"));
    actionOpenStreetmap->setData("openstreetmap");

    // projection:
    d->actionGroupProjection = new QActionGroup(this);
    d->actionGroupProjection->setExclusive(true);

    connect(d->actionGroupProjection, SIGNAL(triggered(QAction*)),
            this, SLOT(slotProjectionActionTriggered(QAction*)));

    KAction* const actionSpherical = new KAction(d->actionGroupProjection);
    actionSpherical->setCheckable(true);
    actionSpherical->setText(i18n("Spherical"));
    actionSpherical->setData("spherical");

    KAction* const actionMercator = new KAction(d->actionGroupProjection);
    actionMercator->setCheckable(true);
    actionMercator->setText(i18n("Mercator"));
    actionMercator->setData("mercator");

    KAction* const actionEquirectangular = new KAction(d->actionGroupProjection);
    actionEquirectangular->setCheckable(true);
    actionEquirectangular->setText(i18n("Equirectangular"));
    actionEquirectangular->setData("equirectangular");

    // float items:
    d->actionGroupFloatItems = new QActionGroup(this);
    d->actionGroupFloatItems->setExclusive(false);

    connect(d->actionGroupFloatItems, SIGNAL(triggered(QAction*)),
            this, SLOT(slotFloatSettingsTriggered(QAction*)));

    d->actionShowCompass = new KAction(i18n("Show compass"), d->actionGroupFloatItems);
    d->actionShowCompass->setData("showcompass");
    d->actionShowCompass->setCheckable(true);
    d->actionGroupFloatItems->addAction(d->actionShowCompass);

    d->actionShowOverviewMap = new KAction(i18n("Show overview map"), d->actionGroupFloatItems);
    d->actionShowOverviewMap->setData("showoverviewmap");
    d->actionShowOverviewMap->setCheckable(true);
    d->actionGroupFloatItems->addAction(d->actionShowOverviewMap);

    d->actionShowScaleBar = new KAction(i18n("Show scale bar"), d->actionGroupFloatItems);
    d->actionShowScaleBar->setData("showscalebar");
    d->actionShowScaleBar->setCheckable(true);
    d->actionGroupFloatItems->addAction(d->actionShowScaleBar);
}

void BackendMarble::addActionsToConfigurationMenu(QMenu* const configurationMenu)
{
    KMAP_ASSERT(configurationMenu!=0);

    configurationMenu->addSeparator();

    const QList<QAction*> mapThemeActions = d->actionGroupMapTheme->actions();
    for (int i=0; i<mapThemeActions.count(); ++i)
    {
        configurationMenu->addAction(mapThemeActions.at(i));
    }

    configurationMenu->addSeparator();

    // TODO: we need a parent for this guy!
    QMenu* const projectionSubMenu = new QMenu(i18n("Projection"), configurationMenu);
    configurationMenu->addMenu(projectionSubMenu);
    const QList<QAction*> projectionActions = d->actionGroupProjection->actions();
    for (int i=0; i<projectionActions.count(); ++i)
    {
        projectionSubMenu->addAction(projectionActions.at(i));
    }

    QMenu* const floatItemsSubMenu = new QMenu(i18n("Float items"), configurationMenu);
    configurationMenu->addMenu(floatItemsSubMenu);
    const QList<QAction*> floatActions = d->actionGroupFloatItems->actions();
    for (int i=0; i<floatActions.count(); ++i)
    {
        floatItemsSubMenu->addAction(floatActions.at(i));
    }

    updateActionAvailability();
}

void BackendMarble::slotMapThemeActionTriggered(QAction* action)
{
    setMapTheme(action->data().toString());
}

QString BackendMarble::getMapTheme() const
{
    // TODO: read the theme from the marblewidget!
    return d->cacheMapTheme;
}

void BackendMarble::setMapTheme(const QString& newMapTheme)
{
    d->cacheMapTheme = newMapTheme;

    if (d->marbleWidget)
    {
        if (newMapTheme=="atlas")
        {
            d->marbleWidget->setMapThemeId("earth/srtm/srtm.dgml");
        }
        else if (newMapTheme=="openstreetmap")
        {
            d->marbleWidget->setMapThemeId("earth/openstreetmap/openstreetmap.dgml");
        }
    }

    // the float items are reset when the theme is changed:
    setShowScaleBar(d->cacheShowScaleBar);
    setShowCompass(d->cacheShowCompass);
    setShowOverviewMap(d->cacheShowOverviewMap);

    updateActionAvailability();
}

void BackendMarble::saveSettingsToGroup(KConfigGroup* const group)
{
    KMAP_ASSERT(group!=0);
    if (!group)
        return;

    group->writeEntry("Marble Map Theme", getMapTheme());
    group->writeEntry("Marble Projection", getProjection());
    group->writeEntry("Marble Show Scale Bar", d->cacheShowScaleBar);
    group->writeEntry("Marble Show Compass", d->cacheShowCompass);
    group->writeEntry("Marble Show Overview Map", d->cacheShowOverviewMap);
}

void BackendMarble::readSettingsFromGroup(const KConfigGroup* const group)
{
    KMAP_ASSERT(group!=0);
    if (!group)
        return;

    setMapTheme(group->readEntry("Marble Map Theme", "atlas"));
    setProjection(group->readEntry("Marble Projection", "spherical"));
    setShowScaleBar(group->readEntry("Marble Show Scale Bar", d->cacheShowScaleBar));
    setShowCompass(group->readEntry("Marble Show Compass", d->cacheShowCompass));
    setShowOverviewMap(group->readEntry("Marble Show Overview Map", d->cacheShowOverviewMap));
}

void BackendMarble::updateMarkers()
{
    // just redraw, that's it:
    d->marbleWidget->update();
}

bool BackendMarble::screenCoordinates(const WMWGeoCoordinate& coordinates, QPoint* const point)
{
    if (!d->marbleWidget)
        return false;

    if (!coordinates.hasCoordinates())
        return false;

    qreal x, y;
    const bool isVisible = d->marbleWidget->screenCoordinates(coordinates.lon(), coordinates.lat(), x, y);
    if (!isVisible)
        return false;

    if (point)
    {
        *point = QPoint(x, y);
    }

    return true;
}

bool BackendMarble::geoCoordinates(const QPoint& point, WMWGeoCoordinate* const coordinates) const
{
    if (!d->marbleWidget)
        return false;

    // apparently, MarbleWidget::geoCoordinates can return true even if the object is not on the screen
    // check that the point is in the visible range:
    if (!d->marbleWidget->rect().contains(point))
        return false;

    qreal lat, lon;
    const bool isVisible = d->marbleWidget->geoCoordinates(point.x(), point.y(), lon, lat, GeoDataCoordinates::Degree);
    if (!isVisible)
        return false;

    if (coordinates)
    {
        *coordinates = WMWGeoCoordinate(lat, lon);
    }

    return true;
}

void BackendMarble::marbleCustomPaint(Marble::GeoPainter* painter)
{
    // check whether the parameters of the map changed and we may have to update the clusters:
    if ( (d->clustersDirtyCacheLat != d->marbleWidget->centerLatitude()) ||
         (d->clustersDirtyCacheLon != d->marbleWidget->centerLongitude()) ||
         (d->clustersDirtyCacheProjection != d->marbleWidget->projection()) )
    {
//         kDebug(51006)<<d->marbleWidget->centerLatitude()<<d->marbleWidget->centerLongitude()<<d->marbleWidget->projection();
        d->clustersDirtyCacheLat = d->marbleWidget->centerLatitude();
        d->clustersDirtyCacheLon = d->marbleWidget->centerLongitude();
        d->clustersDirtyCacheProjection = d->marbleWidget->projection();
        s->worldMapWidget->markClustersAsDirty();
    }

    painter->save();
    painter->autoMapQuality();

    QPen circlePen(Qt::green);
    QBrush circleBrush(Qt::blue);
    // TODO: use global radius instead, but check the code here first
    //const int circleRadius = 15; // s->groupingRadius;

    for (int i = 0; i<s->ungroupedModels.count(); ++i)
    {
        WMWModelHelper* const modelHelper = s->ungroupedModels.at(i);
        if (!modelHelper->modelFlags().testFlag(WMWModelHelper::FlagVisible))
            continue;

        QAbstractItemModel* const model = modelHelper->model();

        // render all visible markers:
        for (int row = 0; row<model->rowCount(); ++row)
        {
            const QModelIndex currentIndex = model->index(row, 0);

            WMWGeoCoordinate markerCoordinates;
            if (!modelHelper->itemCoordinates(currentIndex, &markerCoordinates))
                continue;

            // is the marker being moved right now?
            if (currentIndex == d->mouseMoveMarkerIndex)
            {
                markerCoordinates = d->mouseMoveObjectCoordinates;
            }

            QPoint markerPoint;
            if (!screenCoordinates(markerCoordinates, &markerPoint))
                continue;

            QPoint markerCenterPoint;
            QPixmap markerPixmap = modelHelper->itemIcon(currentIndex, &markerCenterPoint);
            if (markerPixmap.isNull())
            {
                markerPixmap = s->markerPixmap;
                markerCenterPoint = QPoint(markerPixmap.width()/2, 0);
            }

            // drawPixmap wants to know the top-left point
            // our offset is counted from the bottom-left
            // and Qt's coordinate system starts at the top left of the screen!
            const QPoint drawPoint = markerPoint - QPoint(0, markerPixmap.height()) - QPoint(markerCenterPoint.x(), -markerCenterPoint.y());
            painter->drawPixmap(drawPoint, markerPixmap);
        }
    }

    int markersInMovingCluster = 0;
    if (s->markerModel)
    {
        // now for the clusters:
        s->worldMapWidget->updateClusters();

        for (int i = 0; i<s->clusterList.size(); ++i)
        {
            const WMWCluster& cluster = s->clusterList.at(i);
            WMWGeoCoordinate clusterCoordinates = cluster.coordinates;
            int markerCountOverride = cluster.markerCount;
            WMWSelectionState selectionStateOverride = cluster.selectedState;
            if (d->haveMouseMovingObject&&(d->mouseMoveClusterIndex>=0))
            {
                bool movingSelectedMarkers = s->clusterList.at(d->mouseMoveClusterIndex).selectedState!=WMWSelectedNone;
                if (movingSelectedMarkers)
                {
                    markersInMovingCluster+=cluster.markerSelectedCount;
                    markerCountOverride-=cluster.markerSelectedCount;
                    selectionStateOverride = WMWSelectedNone;
                }
                else if (d->mouseMoveClusterIndex == i)
                {
                    markerCountOverride = 0;
                }
                if (markerCountOverride==0)
                    continue;
            }

            QPoint clusterPoint;
            if (!screenCoordinates(clusterCoordinates, &clusterPoint))
                continue;

            QPoint clusterCenterPoint;
            const QPixmap clusterPixmap = s->worldMapWidget->getDecoratedPixmapForCluster(i, &selectionStateOverride, &markerCountOverride, &clusterCenterPoint);

            // drawPixmap wants to know the top-left point
            // our offset is counted from the bottom-left
            // and Qt's coordinate system starts at the top left of the screen!
            const QPoint drawPoint = clusterPoint - QPoint(0, clusterPixmap.height()) - QPoint(clusterCenterPoint.x(), -clusterCenterPoint.y());
            painter->drawPixmap(drawPoint, clusterPixmap);
        }
    }

    // now render the mouse-moving cluster, if there is one:
    if (d->haveMouseMovingObject&&(d->mouseMoveClusterIndex>=0))
    {
        const WMWCluster& cluster = s->clusterList.at(d->mouseMoveClusterIndex);
        WMWGeoCoordinate clusterCoordinates = d->mouseMoveObjectCoordinates;
        int markerCountOverride = (markersInMovingCluster>0)?markersInMovingCluster:cluster.markerCount;
        WMWSelectionState selectionStateOverride = cluster.selectedState;

        QPoint clusterPoint;
        if (screenCoordinates(clusterCoordinates, &clusterPoint))
        {
            // determine the colors:
            QColor       fillColor;
            QColor       strokeColor;
            Qt::PenStyle strokeStyle;
            QColor       labelColor;
            QString      labelText;
            s->worldMapWidget->getColorInfos(d->mouseMoveClusterIndex, &fillColor, &strokeColor,
                                &strokeStyle, &labelText, &labelColor,
                                &selectionStateOverride,
                                &markerCountOverride);

            QString pixmapName = fillColor.name().mid(1);
            if (cluster.selectedState==WMWSelectedAll)
            {
                pixmapName+="-selected";
            }
            if (cluster.selectedState==WMWSelectedSome)
            {
                pixmapName+="-someselected";
            }
            const QPixmap& markerPixmap = s->markerPixmaps[pixmapName];
            painter->drawPixmap(clusterPoint.x()-markerPixmap.width()/2, clusterPoint.y()-markerPixmap.height(), markerPixmap);
        }
    }

    // now render the drag-and-drop marker, if there is one:
    if (d->dragDropMarkerCount>0)
    {
        // determine the colors:
        QColor       fillColor;
        QColor       strokeColor;
        Qt::PenStyle strokeStyle;
        QColor       labelColor;
        QString      labelText;
        s->worldMapWidget->getColorInfos(WMWSelectedAll, d->dragDropMarkerCount,
                            &fillColor, &strokeColor,
                            &strokeStyle, &labelText, &labelColor);

        QString pixmapName = fillColor.name().mid(1);
        pixmapName+="-selected";

        const QPixmap& markerPixmap = s->markerPixmaps[pixmapName];
        painter->drawPixmap(d->dragDropMarkerPos.x()-markerPixmap.width()/2, d->dragDropMarkerPos.y()-markerPixmap.height(), markerPixmap);
    }

    painter->restore();
}

QString BackendMarble::getProjection() const
{
    if (d->marbleWidget)
    {
        const Projection currentProjection = d->marbleWidget->projection();
        switch (currentProjection)
        {
        case Equirectangular:
            d->cacheProjection = "equirectangular";
            break;

        case Mercator:
            d->cacheProjection = "mercator";
            break;

        default:
        case Spherical:
            d->cacheProjection = "spherical";
            break;
        }
    }

    return d->cacheProjection;
}

void BackendMarble::setProjection(const QString& newProjection)
{
    d->cacheProjection = newProjection;

    if (d->marbleWidget)
    {
        if (newProjection=="equirectangular")
        {
            d->marbleWidget->setProjection(Equirectangular);
        }
        else if (newProjection=="mercator")
        {
            d->marbleWidget->setProjection(Mercator);
        }
        else /*if (newProjection=="spherical")*/
        {
            d->marbleWidget->setProjection(Spherical);
        }
    }

    updateActionAvailability();
}

void BackendMarble::slotProjectionActionTriggered(QAction* action)
{
    setProjection(action->data().toString());
}

void BackendMarble::setShowCompass(const bool state)
{
    d->cacheShowCompass = state;
    updateActionAvailability();

    if (d->marbleWidget)
    {
        d->marbleWidget->setShowCompass(state);
    }
}

void BackendMarble::setShowOverviewMap(const bool state)
{
    d->cacheShowOverviewMap = state;
    updateActionAvailability();

    if (d->marbleWidget)
    {
        d->marbleWidget->setShowOverviewMap(state);
    }
}

void BackendMarble::setShowScaleBar(const bool state)
{
    d->cacheShowScaleBar = state;
    updateActionAvailability();

    if (d->marbleWidget)
    {
        d->marbleWidget->setShowScaleBar(state);
    }
}

void BackendMarble::slotFloatSettingsTriggered(QAction* action)
{
    const QString actionIdString = action->data().toString();
    const bool actionState = action->isChecked();

    if (actionIdString=="showcompass")
    {
        setShowCompass(actionState);
    }
    else if (actionIdString=="showscalebar")
    {
        setShowScaleBar(actionState);
    }
    else if (actionIdString=="showoverviewmap")
    {
        setShowOverviewMap(actionState);
    }
}

void BackendMarble::slotClustersNeedUpdating()
{
    // tell the widget to redraw:
    d->marbleWidget->update();
}

void BackendMarble::updateClusters()
{
    // clusters are only needed during redraw
}

QSize BackendMarble::mapSize() const
{
    return d->marbleWidget->map()->size();
}

void BackendMarble::slotMarbleZoomChanged(int newZoom)
{
    Q_UNUSED(newZoom);

    const QString newZoomString = getZoom();

    s->worldMapWidget->markClustersAsDirty();

    updateActionAvailability();
    emit(signalZoomChanged(newZoomString));
}

void BackendMarble::setZoom(const QString& newZoom)
{
    const QString myZoomString = s->worldMapWidget->convertZoomToBackendZoom(newZoom, "marble");
    KMAP_ASSERT(myZoomString.startsWith("marble:"));

    const int myZoom = myZoomString.mid(QString("marble:").length()).toInt();
    kDebug(51006)<<myZoom;

    d->cacheZoom = myZoom;
    d->marbleWidget->zoomView(myZoom);
}

QString BackendMarble::getZoom() const
{
    d->cacheZoom = d->marbleWidget->zoom();
    return QString("marble:%1").arg(d->cacheZoom);
}

int BackendMarble::getMarkerModelLevel()
{
    return MarkerModel::TileIndex::MaxLevel-1;
}

WMWGeoCoordinate::PairList BackendMarble::getNormalizedBounds()
{
    const GeoDataLatLonAltBox marbleBounds = d->marbleWidget->map()->viewParams()->viewport()->viewLatLonAltBox();
//     kDebug(51006)<<marbleBounds.toString(GeoDataCoordinates::Degree);

    const WMWGeoCoordinate::Pair boundsPair = WMWGeoCoordinate::makePair(
            marbleBounds.south(GeoDataCoordinates::Degree),
            marbleBounds.west(GeoDataCoordinates::Degree),
            marbleBounds.north(GeoDataCoordinates::Degree),
            marbleBounds.east(GeoDataCoordinates::Degree)
        );

//     kDebug(51006)<<boundsPair.first<<boundsPair.second;
//     kDebug(51006)<<WMWHelperNormalizeBounds(boundsPair);

    return WMWHelperNormalizeBounds(boundsPair);
}

bool BackendMarble::eventFilter(QObject *object, QEvent *event)
{
    if (object!=d->marbleWidget)
    {
        // event not filtered
        return QObject::eventFilter(object, event);
    }

    // we only handle mouse events:
    if (   (event->type() != QEvent::MouseButtonPress)
        && (event->type() != QEvent::MouseMove)
        && (event->type() != QEvent::MouseButtonRelease) )
    {
        return false;
    }

    QMouseEvent* const mouseEvent = static_cast<QMouseEvent*>(event);
    bool doFilterEvent = false;

    if (   ( event->type() == QEvent::MouseButtonPress )
        && ( mouseEvent->button()==Qt::LeftButton ) )
    {
        // check whether the user clicked on one of our items:
        // scan in reverse order, because the user would expect
        // the topmost marker to be picked up and not the
        // one below
//         if (s->specialMarkersModel)
//         {
//             for (int row = s->specialMarkersModel->rowCount()-1; row>=0; --row)
//             {
//                 const QModelIndex currentIndex = s->specialMarkersModel->index(row, 0);
//                 const WMWGeoCoordinate currentCoordinates = s->specialMarkersModel->data(currentIndex, s->specialMarkersCoordinatesRole).value<WMWGeoCoordinate>();
//
//                 QPoint markerPoint;
//                 if (!screenCoordinates(currentCoordinates, &markerPoint))
//                 {
//                     continue;
//                 }
//
//                 const int markerPixmapHeight = s->markerPixmap.height();
//                 const int markerPixmapWidth = s->markerPixmap.width();
//                 const QRect markerRect(markerPoint.x()-markerPixmapWidth/2, markerPoint.y()-markerPixmapHeight, markerPixmapWidth, markerPixmapHeight);
//                 if (!markerRect.contains(mouseEvent->pos()))
//                 {
//                     continue;
//                 }
//
//                 // the user clicked on a marker:
//                 d->mouseMoveMarkerIndex = QPersistentModelIndex(currentIndex);
//                 d->mouseMoveCenterOffset = mouseEvent->pos() - markerPoint;
//                 d->mouseMoveObjectCoordinates = currentCoordinates;
//                 doFilterEvent = true;
//                 d->havePotentiallyMouseMovingObject = true;
//
//                 break;
//             }
//         }

        if (/*s->inEditMode&&*/!doFilterEvent)
        {
            // scan in reverse order of painting!
            for (int clusterIndex = s->clusterList.count()-1; clusterIndex>=0; --clusterIndex)
            {
                const WMWCluster& cluster = s->clusterList.at(clusterIndex);
                const WMWGeoCoordinate currentCoordinates = cluster.coordinates;

                QPoint clusterPoint;
                if (!screenCoordinates(currentCoordinates, &clusterPoint))
                {
                    continue;
                }

                QRect markerRect;
                markerRect.setSize(cluster.pixmapSize);
                markerRect.moveBottomLeft(clusterPoint);
                markerRect.translate(-QPoint(cluster.pixmapOffset.x(), -cluster.pixmapOffset.y()));

                if (!markerRect.contains(mouseEvent->pos()))
                {
                    continue;
                }

                // TODO: for circles, make sure the mouse is really above the circle and not just in the rectangle!

                // the user clicked on a cluster:
                d->mouseMoveClusterIndex = clusterIndex;
                d->mouseMoveCenterOffset = mouseEvent->pos() - clusterPoint;
                d->mouseMoveObjectCoordinates = currentCoordinates;
                doFilterEvent = true;
                d->havePotentiallyMouseMovingObject = true;
                s->haveMovingCluster = true;

                break;
            }
        }
    }
    else if (   (event->type() == QEvent::MouseMove)
             && (d->havePotentiallyMouseMovingObject || d->haveMouseMovingObject) )
    {
        if ( (!s->editEnabled) || ((d->mouseMoveClusterIndex>=0)&&!s->inEditMode) )
        {
            // clusters only move in edit mode and when edit mode is enabled
            // TODO: this blocks moving of the map in non-edit mode
            d->havePotentiallyMouseMovingObject = false;
            d->mouseMoveClusterIndex = -1;
            d->mouseMoveMarkerIndex = QPersistentModelIndex();
            s->haveMovingCluster = false;
        }
        else
        {

            // mark the object as really moving:
            d->havePotentiallyMouseMovingObject = false;
            d->haveMouseMovingObject = true;

            // a cluster or marker is being moved. update its position:
            QPoint newMarkerPoint = mouseEvent->pos() - d->mouseMoveCenterOffset;
            QPoint snapPoint;
            if (findSnapPoint(newMarkerPoint, &snapPoint, 0, 0))
                newMarkerPoint = snapPoint;

            WMWGeoCoordinate newCoordinates;
            if (geoCoordinates(newMarkerPoint, &newCoordinates))
            {
                d->mouseMoveObjectCoordinates = newCoordinates;
                d->marbleWidget->update();
            }
        }
    }
    else if (   (event->type() == QEvent::MouseButtonRelease)
             && (d->havePotentiallyMouseMovingObject) )
    {
        // the object was not moved, but just clicked once
        if (d->mouseMoveClusterIndex>=0)
        {
            const int mouseMoveClusterIndex = d->mouseMoveClusterIndex;

            // we are done with the clicked object
            // reset these before sending the signal
            d->havePotentiallyMouseMovingObject = false;
            d->mouseMoveClusterIndex = -1;
            d->mouseMoveMarkerIndex = QPersistentModelIndex();
            s->haveMovingCluster = false;

            emit(signalClustersClicked(QIntList()<<mouseMoveClusterIndex));
        }
        else
        {
            // we are done with the clicked object:
            d->havePotentiallyMouseMovingObject = false;
            d->mouseMoveClusterIndex = -1;
            d->mouseMoveMarkerIndex = QPersistentModelIndex();
            s->haveMovingCluster = false;
        }
    }
    else if (   (event->type() == QEvent::MouseButtonRelease)
             && (d->haveMouseMovingObject) )
    {
        // the object was dropped, apply the coordinates if it is on screen:
        const QPoint dropMarkerPoint = mouseEvent->pos() - d->mouseMoveCenterOffset;

        QPair<int, QModelIndex> snapTargetIndex(-1, QModelIndex());
        WMWGeoCoordinate newCoordinates;
        bool haveValidPoint = findSnapPoint(dropMarkerPoint, 0, &newCoordinates, &snapTargetIndex);
        if (!haveValidPoint)
        {
            haveValidPoint = geoCoordinates(dropMarkerPoint, &newCoordinates);
        }

        if (haveValidPoint)
        {
            if (d->mouseMoveMarkerIndex.isValid())
            {
/*                // the marker was dropped to valid coordinates
                s->specialMarkersModel->setData(d->mouseMoveMarkerIndex, QVariant::fromValue(newCoordinates), s->specialMarkersCoordinatesRole);

                QList<QPersistentModelIndex> markerIndices;
                markerIndices << d->mouseMoveMarkerIndex;

                // also emit a signal that the marker was moved:
                emit(signalSpecialMarkersMoved(markerIndices));*/
            }
            else
            {
                // a cluster is being moved
                s->clusterList[d->mouseMoveClusterIndex].coordinates = newCoordinates;
                emit(signalClustersMoved(QIntList()<<d->mouseMoveClusterIndex, snapTargetIndex));
            }
        }

        d->haveMouseMovingObject = false;
        d->mouseMoveClusterIndex = -1;
        d->mouseMoveMarkerIndex = QPersistentModelIndex();
        d->marbleWidget->update();
        s->haveMovingCluster = false;
    }

    if (doFilterEvent)
        return true;

    return QObject::eventFilter(object, event);
}

// void BackendMarble::updateDragDropMarker(const QPoint& pos, const WMWDragData* const dragData)
// {
//     if (!dragData)
//     {
//         d->dragDropMarkerCount = 0;
//     }
//     else
//     {
//         d->dragDropMarkerPos = pos;
//         d->dragDropMarkerCount = dragData->itemCount;
//     }
//     d->marbleWidget->update();
//
//     // TODO: hide dragged markers on the map
// }
//
// void BackendMarble::updateDragDropMarkerPosition(const QPoint& pos)
// {
//     d->dragDropMarkerPos = pos;
//     d->marbleWidget->update();
// }

void BackendMarble::updateActionAvailability()
{
    kDebug(51006)<<d->cacheZoom<<d->marbleWidget->maximumZoom()<<d->marbleWidget->minimumZoom();
    s->worldMapWidget->getControlAction("zoomin")->setEnabled(d->cacheZoom<d->marbleWidget->maximumZoom());
    s->worldMapWidget->getControlAction("zoomout")->setEnabled(d->cacheZoom>d->marbleWidget->minimumZoom());

    const QList<QAction*> mapThemeActions = d->actionGroupMapTheme->actions();
    for (int i=0; i<mapThemeActions.size(); ++i)
    {
        mapThemeActions.at(i)->setChecked(mapThemeActions.at(i)->data().toString()==getMapTheme());
    }

    const QList<QAction*> projectionActions = d->actionGroupProjection->actions();
    for (int i=0; i<projectionActions.size(); ++i)
    {
        projectionActions.at(i)->setChecked(projectionActions.at(i)->data().toString()==getProjection());
    }

    d->actionShowCompass->setChecked(d->cacheShowCompass);
    d->actionShowScaleBar->setChecked(d->cacheShowScaleBar);
    d->actionShowOverviewMap->setChecked(d->cacheShowOverviewMap);
}

void BackendMarble::slotThumbnailAvailableForIndex(const QVariant& index, const QPixmap& pixmap)
{
    kDebug(51006)<<index<<pixmap.size();
    if (pixmap.isNull() || s->inEditMode)
        return;

    // TODO: properly reject pixmaps with the wrong size
    const int expectedThumbnailSize = s->worldMapWidget->getUndecoratedThumbnailSize();
    if ((pixmap.size().height()!=expectedThumbnailSize)&&(pixmap.size().width()!=expectedThumbnailSize))
        return;

    // re-paint the map
    d->marbleWidget->update();
}

void BackendMarble::slotUngroupedModelChanged(const int /*index*/)
{
    d->marbleWidget->update();
}

bool BackendMarble::findSnapPoint(const QPoint& actualPoint, QPoint* const snapPoint, WMWGeoCoordinate* const snapCoordinates, QPair<int, QModelIndex>* const snapTargetIndex)
{
    QPoint bestSnapPoint;
    WMWGeoCoordinate bestSnapCoordinates;
    int bestSnapDistanceSquared = -1;
    QModelIndex bestSnapIndex;
    int bestSnapUngroupedModel;

    // now handle snapping: is there any object close by?
    for (int im = 0; im<s->ungroupedModels.count(); ++im)
    {
        WMWModelHelper* const modelHelper = s->ungroupedModels.at(im);
        // TODO: test for active snapping
        if (   (!modelHelper->modelFlags().testFlag(WMWModelHelper::FlagVisible))
            || (!modelHelper->modelFlags().testFlag(WMWModelHelper::FlagSnaps)) )
            continue;

        // TODO: configurable snapping radius
        const int snapRadiusSquared = 10*10;
        QAbstractItemModel* const itemModel = modelHelper->model();

        for (int row=0; row<itemModel->rowCount(); ++row)
        {
            const QModelIndex currentIndex = itemModel->index(row, 0);
            WMWGeoCoordinate currentCoordinates;
            if (!modelHelper->itemCoordinates(currentIndex, &currentCoordinates))
                continue;

            QPoint snapMarkerPoint;
            if (!screenCoordinates(currentCoordinates, &snapMarkerPoint))
            {
                continue;
            }

            const QPoint distancePoint = snapMarkerPoint - actualPoint;
            const int snapDistanceSquared = (distancePoint.x()*distancePoint.x()+distancePoint.y()*distancePoint.y());
            if ( (snapDistanceSquared<=snapRadiusSquared)
                &&
                    ((bestSnapDistanceSquared==-1)||(bestSnapDistanceSquared>snapDistanceSquared))
                    )
            {
                bestSnapDistanceSquared = snapDistanceSquared;
                bestSnapPoint = snapMarkerPoint;
                bestSnapCoordinates = currentCoordinates;
                bestSnapIndex = currentIndex;
                bestSnapUngroupedModel = im;
            }
        }
    }

    const bool foundSnapPoint = (bestSnapDistanceSquared>=0);

    if (foundSnapPoint)
    {
        if (snapPoint)
            *snapPoint = bestSnapPoint;

        if (snapCoordinates)
            *snapCoordinates = bestSnapCoordinates;

        if (snapTargetIndex)
            *snapTargetIndex = QPair<int, QModelIndex>(bestSnapUngroupedModel, bestSnapIndex);
    }

    return foundSnapPoint;
}

} /* namespace KMapIface */
