/* ============================================================
 *
 * Date        : 2009-12-01
 * Description : Marble-backend for WorldMapWidget2
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
#include <marble/GeoPainter.h>
#include <marble/MarbleMap.h>
#include <marble/ViewParams.h>
#include <marble/ViewportParams.h>

// local includes

#include "bm-widget.h"
#include "markermodel.h"
#include "worldmapwidget2.h"

using namespace Marble;

namespace WMW2 {

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
      mouseMoveClusterIndex(-1),
      mouseMoveMarkerIndex(),
      mouseMoveObjectCoordinates(),
      mouseMoveCenterOffset(0,0)
    {
    }

    QPointer<BMWidget> marbleWidget;

    QPointer<QActionGroup> actionGroupMapTheme;
    QPointer<QActionGroup> actionGroupProjection;
    QPointer<QActionGroup> actionGroupFloatItems;
    QPointer<KAction> actionShowCompass;
    QPointer<KAction> actionShowOverviewMap;
    QPointer<KAction> actionShowScaleBar;

    QString cacheMapTheme;
    QString cacheProjection;
    bool cacheShowCompass;
    bool cacheShowScaleBar;
    bool cacheShowOverviewMap;
    int cacheZoom;
    int mouseMoveClusterIndex;
    QPersistentModelIndex mouseMoveMarkerIndex;
    WMWGeoCoordinate mouseMoveObjectCoordinates;
    QPoint mouseMoveCenterOffset;
    QPixmap markerPixmap;
};

BackendMarble::BackendMarble(WMWSharedData* const sharedData, QObject* const parent)
: MapBackend(sharedData, parent), d(new BackendMarblePrivate())
{
    const KUrl markerGreenUrl = KStandardDirs::locate("data", "libworldmapwidget2/marker-green.png");
    d->markerPixmap = QPixmap(markerGreenUrl.toLocalFile());

    d->marbleWidget = new BMWidget(this);

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
    return "marble";
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
    d->marbleWidget->setCenterLatitude(coordinate.lat);
    d->marbleWidget->setCenterLongitude(coordinate.lon);
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

void BackendMarble::addActionsToConfigurationMenu(QMenu* const configurationMenu)
{
    WMW2_ASSERT(configurationMenu!=0);

    configurationMenu->addSeparator();

    if (d->actionGroupMapTheme)
    {
        delete d->actionGroupMapTheme;
    }
    d->actionGroupMapTheme = new QActionGroup(configurationMenu);
    d->actionGroupMapTheme->setExclusive(true);

    KAction* const actionAtlas = new KAction(d->actionGroupMapTheme);
    actionAtlas->setCheckable(true);
    actionAtlas->setText(i18n("Atlas map"));
    actionAtlas->setData("atlas");
//     actionAtlas->setChecked(getMapTheme()=="atlas");
    configurationMenu->addAction(actionAtlas);

    KAction* const actionOpenStreetmap = new KAction(d->actionGroupMapTheme);
    actionOpenStreetmap->setCheckable(true);
    actionOpenStreetmap->setText(i18n("OpenStreetMap"));
    actionOpenStreetmap->setData("openstreetmap");
//     actionOpenStreetmap->setChecked(getMapTheme()=="openstreetmap");
    configurationMenu->addAction(actionOpenStreetmap);

    connect(d->actionGroupMapTheme, SIGNAL(triggered(QAction*)),
            this, SLOT(slotMapThemeActionTriggered(QAction*)));

    configurationMenu->addSeparator();

    if (d->actionGroupProjection)
    {
        delete d->actionGroupProjection;
    }
    d->actionGroupProjection = new QActionGroup(configurationMenu);
    d->actionGroupProjection->setExclusive(true);

    // TODO: we need a parent for this guy!
    QMenu* const projectionSubMenu = new QMenu(i18n("Projection"), configurationMenu);
    configurationMenu->addMenu(projectionSubMenu);

    KAction* const actionSpherical = new KAction(d->actionGroupProjection);
    actionSpherical->setCheckable(true);
    actionSpherical->setText(i18n("Spherical"));
    actionSpherical->setData("spherical");
    projectionSubMenu->addAction(actionSpherical);

    KAction* const actionMercator = new KAction(d->actionGroupProjection);
    actionMercator->setCheckable(true);
    actionMercator->setText(i18n("Mercator"));
    actionMercator->setData("mercator");
    projectionSubMenu->addAction(actionMercator);

    KAction* const actionEquirectangular = new KAction(d->actionGroupProjection);
    actionEquirectangular->setCheckable(true);
    actionEquirectangular->setText(i18n("Equirectangular"));
    actionEquirectangular->setData("equirectangular");
    projectionSubMenu->addAction(actionEquirectangular);

    connect(d->actionGroupProjection, SIGNAL(triggered(QAction*)),
            this, SLOT(slotProjectionActionTriggered(QAction*)));

    if (!d->actionGroupFloatItems)
    {
        d->actionGroupFloatItems = new QActionGroup(this);
        d->actionGroupFloatItems->setExclusive(false);
        connect(d->actionGroupFloatItems, SIGNAL(triggered(QAction*)),
                this, SLOT(slotFloatSettingsTriggered(QAction*)));
    }

    QMenu* const floatItemsSubMenu = new QMenu(i18n("Float items"), configurationMenu);
    configurationMenu->addMenu(floatItemsSubMenu);

    d->actionShowCompass = new KAction(i18n("Show compass"), floatItemsSubMenu);
    d->actionShowCompass->setData("showcompass");
    d->actionShowCompass->setCheckable(true);
    d->actionGroupFloatItems->addAction(d->actionShowCompass);
    floatItemsSubMenu->addAction(d->actionShowCompass);

    d->actionShowOverviewMap = new KAction(i18n("Show overview map"), floatItemsSubMenu);
    d->actionShowOverviewMap->setData("showoverviewmap");
    d->actionShowOverviewMap->setCheckable(true);
    d->actionGroupFloatItems->addAction(d->actionShowOverviewMap);
    floatItemsSubMenu->addAction(d->actionShowOverviewMap);

    d->actionShowScaleBar = new KAction(i18n("Show scale bar"), floatItemsSubMenu);
    d->actionShowScaleBar->setData("showscalebar");
    d->actionShowScaleBar->setCheckable(true);
    d->actionGroupFloatItems->addAction(d->actionShowScaleBar);
    floatItemsSubMenu->addAction(d->actionShowScaleBar);

    updateActionsEnabled();
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

    updateActionsEnabled();
}

void BackendMarble::updateActionsEnabled()
{
    if (d->actionGroupMapTheme)
    {
        const QList<QAction*> mapThemeActions = d->actionGroupMapTheme->actions();
        for (int i=0; i<mapThemeActions.size(); ++i)
        {
            mapThemeActions.at(i)->setChecked(mapThemeActions.at(i)->data().toString()==getMapTheme());
        }
    }

    if (d->actionGroupProjection)
    {
        const QList<QAction*> projectionActions = d->actionGroupProjection->actions();
        for (int i=0; i<projectionActions.size(); ++i)
        {
            projectionActions.at(i)->setChecked(projectionActions.at(i)->data().toString()==getProjection());
        }
    }

    if (d->actionShowCompass)
    {
        d->actionShowCompass->setChecked(d->cacheShowCompass);
    }

    if (d->actionShowScaleBar)
    {
        d->actionShowScaleBar->setChecked(d->cacheShowScaleBar);
    }

    if (d->actionShowOverviewMap)
    {
        d->actionShowOverviewMap->setChecked(d->cacheShowOverviewMap);
    }
}

void BackendMarble::saveSettingsToGroup(KConfigGroup* const group)
{
    WMW2_ASSERT(group!=0);
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
    WMW2_ASSERT(group!=0);
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

    qreal x, y;
    const bool isVisible = d->marbleWidget->screenCoordinates(coordinates.lon, coordinates.lat, x, y);
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
    painter->save();
    painter->autoMapQuality();

    QPen circlePen(Qt::green);
    QBrush circleBrush(Qt::blue);
    const int circleRadius = 15;

    // render all visible markers:
    for (int row = 0; row<s->specialMarkersModel->rowCount(); ++row)
    {
        const QModelIndex currentIndex = s->specialMarkersModel->index(row, 0);
        
        WMWGeoCoordinate markerCoordinates = s->specialMarkersModel->data(currentIndex, s->specialMarkersCoordinatesRole).value<WMWGeoCoordinate>();
        // is the marker being moved right now?
        if (currentIndex == d->mouseMoveMarkerIndex)
        {
            markerCoordinates = d->mouseMoveObjectCoordinates;
        }

        QPoint markerPoint;
        if (!screenCoordinates(markerCoordinates, &markerPoint))
            continue;

        painter->drawPixmap(markerPoint.x()-d->markerPixmap.height()/2, markerPoint.y()-d->markerPixmap.height(), d->markerPixmap);
//         painter->setPen(circlePen);
//         painter->setBrush(circleBrush);
//         painter->drawEllipse(markerPoint.x()-circleRadius, markerPoint.y()-circleRadius, 2*circleRadius, 2*circleRadius);
    }

    // now for the clusters:
    generateClusters();

    for (int i = 0; i<s->clusterList.size(); ++i)
    {
        const WMWCluster& cluster = s->clusterList.at(i);
        QPoint clusterPoint;
        if (!screenCoordinates(cluster.coordinates, &clusterPoint))
            continue;

        // determine the colors:
        QColor       fillColor;
        QColor       strokeColor;
        Qt::PenStyle strokeStyle;
        QColor       labelColor;
        QString      labelText;
        s->worldMapWidget->getColorInfos(i, &fillColor, &strokeColor,
                              &strokeStyle, &labelText, &labelColor);

        QPen circlePen;
        circlePen.setColor(strokeColor);
        circlePen.setStyle(strokeStyle);
        circlePen.setWidth(2);
        QBrush circleBrush(fillColor);
        QPen labelPen;
        labelPen.setColor(labelColor);
                              
        const QRect circleRect(clusterPoint.x()-circleRadius, clusterPoint.y()-circleRadius, 2*circleRadius, 2*circleRadius);

        painter->setPen(circlePen);
        painter->setBrush(circleBrush);
        painter->drawEllipse(circleRect);

        painter->setPen(labelPen);
        painter->setBrush(Qt::NoBrush);
        painter->drawText(circleRect, Qt::AlignHCenter|Qt::AlignVCenter, labelText);
    }

    painter->restore();
}

void BackendMarble::generateClusters()
{
    s->worldMapWidget->updateClusters();
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

    updateActionsEnabled();
}

void BackendMarble::slotProjectionActionTriggered(QAction* action)
{
    setProjection(action->data().toString());
}

void BackendMarble::setShowCompass(const bool state)
{
    d->cacheShowCompass = state;
    updateActionsEnabled();

    if (d->marbleWidget)
    {
        d->marbleWidget->setShowCompass(state);
    }
}

void BackendMarble::setShowOverviewMap(const bool state)
{
    d->cacheShowOverviewMap = state;
    updateActionsEnabled();

    if (d->marbleWidget)
    {
        d->marbleWidget->setShowOverviewMap(state);
    }
}

void BackendMarble::setShowScaleBar(const bool state)
{
    d->cacheShowScaleBar = state;
    updateActionsEnabled();

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

    kDebug()<<getZoom();
    emit(signalZoomChanged(getZoom()));
}

void BackendMarble::setZoom(const QString& newZoom)
{
    const QString myZoomString = s->worldMapWidget->convertZoomToBackendZoom(newZoom, "marble");
    WMW2_ASSERT(myZoomString.startsWith("marble:"));

    const int myZoom = myZoomString.mid(QString("marble:").length()).toInt();
    kDebug()<<myZoom;

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
    return s->markerModel->maxLevel()-1;
}

WMWGeoCoordinate::PairList BackendMarble::getNormalizedBounds()
{
    const GeoDataLatLonAltBox marbleBounds = d->marbleWidget->map()->viewParams()->viewport()->viewLatLonAltBox();
    kDebug()<<marbleBounds.toString(GeoDataCoordinates::Degree);

    const WMWGeoCoordinate::Pair boundsPair = WMWGeoCoordinate::makePair(
            marbleBounds.south(GeoDataCoordinates::Degree),
            marbleBounds.west(GeoDataCoordinates::Degree),
            marbleBounds.north(GeoDataCoordinates::Degree),
            marbleBounds.east(GeoDataCoordinates::Degree)
        );

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
        for (int row = s->specialMarkersModel->rowCount()-1; row>=0; --row)
        {
            const QModelIndex currentIndex = s->specialMarkersModel->index(row, 0);
            const WMWGeoCoordinate currentCoordinates = s->specialMarkersModel->data(currentIndex, s->specialMarkersCoordinatesRole).value<WMWGeoCoordinate>();

            QPoint markerPoint;
            if (!screenCoordinates(currentCoordinates, &markerPoint))
            {
                continue;
            }

            const int markerPixmapHeight = d->markerPixmap.height();
            const int markerPixmapWidth = d->markerPixmap.width();
            const QRect markerRect(markerPoint.x()-markerPixmapWidth/2, markerPoint.y()-markerPixmapHeight, markerPixmapWidth, markerPixmapHeight);
            if (!markerRect.contains(mouseEvent->pos()))
            {
                continue;
            }

            // the user clicked on a cluster:
            d->mouseMoveMarkerIndex = QPersistentModelIndex(currentIndex);
            d->mouseMoveCenterOffset = mouseEvent->pos() - markerPoint;
            d->mouseMoveObjectCoordinates = currentCoordinates;
            doFilterEvent = true;

            break;
        }
    }
    else if (   (event->type() == QEvent::MouseMove)
             && (d->mouseMoveMarkerIndex.isValid()) )
    {
        // a marker is being moved. update its position:
        const QPoint newMarkerPoint = mouseEvent->pos() - d->mouseMoveCenterOffset;

        WMWGeoCoordinate newCoordinates;
        if (geoCoordinates(newMarkerPoint, &newCoordinates))
        {
            d->mouseMoveObjectCoordinates = newCoordinates;
            d->marbleWidget->update();
        }
    }
    else if (   (event->type() == QEvent::MouseButtonRelease)
             && (d->mouseMoveMarkerIndex.isValid()) )
    {
        // the marker was dropped, apply the coordinates if it is on screen:
        const QPoint newMarkerPoint = mouseEvent->pos() - d->mouseMoveCenterOffset;

        WMWGeoCoordinate newCoordinates;
        if (geoCoordinates(newMarkerPoint, &newCoordinates))
        {
            // the marker was dropped to valid coordinates
            s->specialMarkersModel->setData(d->mouseMoveMarkerIndex, QVariant::fromValue(newCoordinates), s->specialMarkersCoordinatesRole);

            QList<QPersistentModelIndex> markerIndices;
            markerIndices << d->mouseMoveMarkerIndex;

            d->mouseMoveMarkerIndex = QPersistentModelIndex();
            d->marbleWidget->update();

            // also emit a signal that the marker was moved:
            emit(signalSpecialMarkersMoved(markerIndices));
        }
    }

    if (doFilterEvent)
        return true;

    return QObject::eventFilter(object, event);
}

} /* WMW2 */

