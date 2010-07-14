/** ===========================================================
 *
 * This file is a part of digiKam project
 * <a href="http://www.digikam.org">http://www.digikam.org</a>
 *
 * @date   2009-12-01
 * @brief  Base-class for backends for KMap
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

#ifndef MAP_BACKEND_H
#define MAP_BACKEND_H

// Qt includes

#include <QWidget>

// local includes

#include "kmap_common.h"

class QMenu;
class KConfigGroup;

namespace KMapIface
{

class WMWSharedData;

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

//     virtual void updateDragDropMarker(const QPoint& pos, const WMWDragData* const dragData) = 0;
//     virtual void updateDragDropMarkerPosition(const QPoint& pos) = 0;

    virtual void updateActionAvailability() = 0;

    const QExplicitlySharedDataPointer<WMWSharedData> s;

public Q_SLOTS:

    virtual void slotClustersNeedUpdating() = 0;
    virtual void slotThumbnailAvailableForIndex(const QVariant& index, const QPixmap& pixmap);

Q_SIGNALS:

    void signalBackendReady(const QString& backendName);
    void signalClustersMoved(const QIntList& clusterIndices, const QPair<int, QModelIndex>& snapTarget);
    void signalClustersClicked(const QIntList& clusterIndices);
    void signalMarkersMoved(const QIntList& markerIndices);
    void signalZoomChanged(const QString& newZoom);
    void signalSpecialMarkersMoved(const QList<QPersistentModelIndex>& indices);

private:

    MapBackendPrivate* const d;
};

} /* namespace KMapIface */

#endif /* MAP_BACKEND_H */
