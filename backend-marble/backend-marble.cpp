/* ============================================================
 *
 * Date        : 2009-12-01
 * Description : Marble-backend for WorldMapWidget2
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

// Qt includes

#include <QMenu>
#include <QPointer>

// KDE includes

#include <kaction.h>
#include <kconfiggroup.h>
#include <klocale.h>
#include <marble/GeoPainter.h>

// local includes

#include "backend-marble.h"
#include "bm-widget.h"

using namespace Marble;

namespace WMW2 {

class BackendMarblePrivate
{
public:
    BackendMarblePrivate()
    : marbleWidget(0),
      actionGroupMapTheme(0),
      cacheMapTheme("atlas")
    {
    }

    QPointer<BMWidget> marbleWidget;

    QPointer<QActionGroup> actionGroupMapTheme;

    QString cacheMapTheme;
};

BackendMarble::BackendMarble(WMWSharedData* const sharedData, QObject* const parent)
: MapBackend(sharedData, parent), d(new BackendMarblePrivate())
{
    d->marbleWidget = new BMWidget(this);
    setMapTheme(d->cacheMapTheme);

    d->marbleWidget->setShowCompass(false);
    d->marbleWidget->setShowScaleBar(false);
    d->marbleWidget->setShowOverviewMap(false);

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
    Q_ASSERT(configurationMenu!=0);

    configurationMenu->addSeparator();

    if (d->actionGroupMapTheme)
    {
        delete d->actionGroupMapTheme;
    }
    d->actionGroupMapTheme = new QActionGroup(this);
    d->actionGroupMapTheme->setExclusive(true);

    KAction* const actionAtlas = new KAction(d->actionGroupMapTheme);
    actionAtlas->setCheckable(true);
    actionAtlas->setText(i18n("Atlas map"));
    actionAtlas->setData("atlas");
    actionAtlas->setChecked(getMapTheme()=="atlas");
    configurationMenu->addAction(actionAtlas);

    KAction* const actionOpenStreetmap = new KAction(d->actionGroupMapTheme);
    actionOpenStreetmap->setCheckable(true);
    actionOpenStreetmap->setText(i18n("OpenStreetMap"));
    actionOpenStreetmap->setData("openstreetmap");
    actionOpenStreetmap->setChecked(getMapTheme()=="openstreetmap");
    configurationMenu->addAction(actionOpenStreetmap);

    connect(d->actionGroupMapTheme, SIGNAL(triggered(QAction*)),
            this, SLOT(slotMapThemeActionTriggered(QAction*)));
}

void BackendMarble::slotMapThemeActionTriggered(QAction* action)
{
    setMapTheme(action->data().toString());
}

QString BackendMarble::getMapTheme() const
{
    return d->cacheMapTheme;
}

void BackendMarble::setMapTheme(const QString& newMapTheme)
{
    d->cacheMapTheme = newMapTheme;

    if (newMapTheme=="atlas")
    {
        d->marbleWidget->setMapThemeId("earth/srtm/srtm.dgml");
    }
    else if (newMapTheme=="openstreetmap")
    {
        d->marbleWidget->setMapThemeId("earth/openstreetmap/openstreetmap.dgml");
    }

    updateActionsEnabled();
}

void BackendMarble::updateActionsEnabled()
{
    if (!d->actionGroupMapTheme)
        return;

    const QList<QAction*> mapThemeActions = d->actionGroupMapTheme->actions();
    for (int i=0; i<mapThemeActions.size(); ++i)
    {
        mapThemeActions.at(i)->setChecked(mapThemeActions.at(i)->data().toString()==getMapTheme());
    }
}

void BackendMarble::saveSettingsToGroup(KConfigGroup* const group)
{
    Q_ASSERT(group!=0);
    if (!group)
        return;

    group->writeEntry("Marble Map Theme", getMapTheme());
}

void BackendMarble::readSettingsFromGroup(const KConfigGroup* const group)
{
    Q_ASSERT(group!=0);
    if (!group)
        return;

    setMapTheme(group->readEntry("Marble Map Theme", "atlas"));
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

void BackendMarble::marbleCustomPaint(Marble::GeoPainter* painter)
{
    painter->save();
    painter->autoMapQuality();

    QPen circlePen(Qt::cyan);
    QBrush circleBrush(Qt::green);
    const int circleRadius = 15;

    // render all visible markers:
    for (QIntList::const_iterator it = s->visibleMarkers.constBegin(); it!=s->visibleMarkers.constEnd(); ++it)
    {
        const int currentIndex = *it;
        const WMWMarker* const currentMarker = &(s->markerList.at(currentIndex));
        QPoint markerPoint;
        if (!screenCoordinates(currentMarker->coordinates, &markerPoint))
            continue;

        painter->setPen(circlePen);
        painter->setBrush(circleBrush);
        painter->drawEllipse(markerPoint.x()-circleRadius, markerPoint.y()-circleRadius, 2*circleRadius, 2*circleRadius);
    }

    painter->restore();
}

} /* WMW2 */

