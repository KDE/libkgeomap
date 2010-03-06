/* ============================================================
 *
 * Date        : 2009-12-01
 * Description : Base-class for backends for WorldMapWidget2
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

#ifndef MAP_BACKEND_H
#define MAP_BACKEND_H

// Qt includes

#include <QWidget>

// local includes

#include "worldmapwidget2_primitives.h"

class QMenu;
class KConfigGroup;

namespace WMW2 {

class MapBackendPrivate;

class MapBackend : public QObject
{

Q_OBJECT

public:

    MapBackend(const QExplicitlySharedDataPointer<WMWSharedData>& sharedData, QObject* const parent);
    virtual ~MapBackend();

    virtual QString backendName() const = 0;
    virtual QString backendHumanName() const = 0;
    virtual QWidget* mapWidget() const = 0;

    virtual WMWGeoCoordinate getCenter() const = 0;
    virtual void setCenter(const WMWGeoCoordinate& coordinate) = 0;

    virtual bool isReady() const = 0;

    virtual void zoomIn() = 0;
    virtual void zoomOut() = 0;

    virtual void saveSettingsToGroup(KConfigGroup* const group) = 0;
    virtual void readSettingsFromGroup(const KConfigGroup* const group) = 0;

    virtual void addActionsToConfigurationMenu(QMenu* const configurationMenu) = 0;

    virtual void updateMarkers() = 0;
    virtual void updateClusters() = 0;

    virtual bool screenCoordinates(const WMWGeoCoordinate& coordinates, QPoint* const point) = 0;
    virtual bool geoCoordinates(const QPoint& point, WMWGeoCoordinate* const coordinates) const = 0;
    virtual QSize mapSize() const = 0;

    virtual void setZoom(const QString& newZoom) = 0;
    virtual QString getZoom() const = 0;

    virtual int getMarkerModelLevel() = 0;
    virtual WMWGeoCoordinate::PairList getNormalizedBounds() = 0;

    // TODO: = 0
    virtual void updateDragDropMarker(const QPoint& pos, const WMWDragData* const dragData) {};

    const QExplicitlySharedDataPointer<WMWSharedData> s;

public Q_SLOTS:
    virtual void slotClustersNeedUpdating() = 0;

Q_SIGNALS:
    void signalBackendReady(const QString& backendName);
    void signalClustersMoved(const QIntList& clusterIndices);
    void signalClustersClicked(const QIntList& clusterIndices);
    void signalMarkersMoved(const QIntList& markerIndices);
    void signalZoomChanged(const QString& newZoom);
    void signalSpecialMarkersMoved(const QList<QPersistentModelIndex>& indices);

private:
    MapBackendPrivate* const d;
};

} /* WMW2 */

#endif /* MAP_BACKEND_H */
