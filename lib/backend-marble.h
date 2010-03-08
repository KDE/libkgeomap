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

#ifndef BACKEND_MARBLE_H
#define BACKEND_MARBLE_H

// local includes

#include "map-backend.h"

namespace Marble {
    class GeoPainter;
} /* Marble */

namespace WMW2 {

class BackendMarblePrivate;

class BackendMarble : public MapBackend
{
Q_OBJECT

public:
    BackendMarble(const QExplicitlySharedDataPointer<WMWSharedData>& sharedData, QObject* const parent = 0);
    virtual ~BackendMarble();

    virtual QString backendName() const;
    virtual QString backendHumanName() const;
    virtual QWidget* mapWidget() const;

    virtual WMWGeoCoordinate getCenter() const;
    virtual void setCenter(const WMWGeoCoordinate& coordinate);

    virtual bool isReady() const;

    virtual void zoomIn();
    virtual void zoomOut();

    virtual void saveSettingsToGroup(KConfigGroup* const group);
    virtual void readSettingsFromGroup(const KConfigGroup* const group);

    virtual void addActionsToConfigurationMenu(QMenu* const configurationMenu);

    virtual void updateMarkers();
    virtual void updateClusters();

    QString getMapTheme() const;
    void setMapTheme(const QString& newMapTheme);

    QString getProjection() const;
    void setProjection(const QString& newProjection);

    virtual bool screenCoordinates(const WMWGeoCoordinate& coordinates, QPoint* const point);
    virtual bool geoCoordinates(const QPoint& point, WMWGeoCoordinate* const coordinates) const;
    virtual QSize mapSize() const;

    virtual void setZoom(const QString& newZoom);
    virtual QString getZoom() const;

    virtual int getMarkerModelLevel();
    virtual WMWGeoCoordinate::PairList getNormalizedBounds();

    virtual void updateDragDropMarker(const QPoint& pos, const WMWDragData* const dragData);
    virtual void updateDragDropMarkerPosition(const QPoint& pos);

    virtual void updateActionAvailability();

    void marbleCustomPaint(Marble::GeoPainter* painter);
    void setShowCompass(const bool state);
    void setShowOverviewMap(const bool state);
    void setShowScaleBar(const bool state);

public Q_SLOTS:
    virtual void slotClustersNeedUpdating();

protected:
    bool eventFilter(QObject *object, QEvent *event);
    void createActions();

protected Q_SLOTS:
    void slotMapThemeActionTriggered(QAction* action);
    void slotProjectionActionTriggered(QAction* action);
    void slotFloatSettingsTriggered(QAction* action);
    void slotMarbleZoomChanged(int newZoom);

private:
    BackendMarblePrivate* const d;
};

} /* WMW2 */

#endif /* BACKEND_MARBLE_H */

