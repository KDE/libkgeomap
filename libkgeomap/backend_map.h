/** ===========================================================
 * @file
 *
 * This file is a part of digiKam project
 * <a href="http://www.digikam.org">http://www.digikam.org</a>
 *
 * @date   2009-12-01
 * @brief  Base-class for backends for KMap
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

#ifndef BACKEND_MAP_H
#define BACKEND_MAP_H

// Qt includes

#include <QModelIndex>
#include <QWidget>

// local includes

#include "kmap_common.h"

// Marble Widget includes

#include <marble/GeoDataLatLonBox.h>

class QMenu;
class KConfigGroup;

namespace KMap
{

class KMapSharedData;

class MapBackend : public QObject
{

Q_OBJECT

public:

    MapBackend(const QExplicitlySharedDataPointer<KMapSharedData>& sharedData, QObject* const parent);
    virtual ~MapBackend();

    virtual QString backendName() const = 0;
    virtual QString backendHumanName() const = 0;
    virtual QWidget* mapWidget() = 0;
    virtual void releaseWidget(KMapInternalWidgetInfo* const info) = 0;
    virtual void mapWidgetDocked(const bool state) = 0;

    virtual GeoCoordinates getCenter() const = 0;
    virtual void setCenter(const GeoCoordinates& coordinate) = 0;

    virtual bool isReady() const = 0;

    virtual void zoomIn() = 0;
    virtual void zoomOut() = 0;

    virtual void saveSettingsToGroup(KConfigGroup* const group) = 0;
    virtual void readSettingsFromGroup(const KConfigGroup* const group) = 0;

    virtual void addActionsToConfigurationMenu(QMenu* const configurationMenu) = 0;

    virtual void updateMarkers() = 0;
    virtual void updateClusters() = 0;

    virtual bool screenCoordinates(const GeoCoordinates& coordinates, QPoint* const point) = 0;
    virtual bool geoCoordinates(const QPoint& point, GeoCoordinates* const coordinates) const = 0;
    virtual QSize mapSize() const = 0;

    virtual void setZoom(const QString& newZoom) = 0;
    virtual QString getZoom() const = 0;

    virtual int getMarkerModelLevel() = 0;
    virtual GeoCoordinates::PairList getNormalizedBounds() = 0;

//     virtual void updateDragDropMarker(const QPoint& pos, const KMapDragData* const dragData) = 0;
//     virtual void updateDragDropMarkerPosition(const QPoint& pos) = 0;

    virtual void updateActionAvailability() = 0;

    virtual void regionSelectionChanged() = 0;
    virtual void mouseModeChanged() = 0;

    const QExplicitlySharedDataPointer<KMapSharedData> s;

    virtual void centerOn(const Marble::GeoDataLatLonBox& box, const bool useSaneZoomLevel = true) = 0;
    virtual void setActive(const bool state) = 0;

public Q_SLOTS:

    virtual void slotClustersNeedUpdating() = 0;
    virtual void slotThumbnailAvailableForIndex(const QVariant& index, const QPixmap& pixmap);

Q_SIGNALS:

    void signalBackendReadyChanged(const QString& backendName);
    void signalClustersMoved(const QIntList& clusterIndices, const QPair<int, QModelIndex>& snapTarget);
    void signalClustersClicked(const QIntList& clusterIndices);
    void signalMarkersMoved(const QIntList& markerIndices);
    void signalZoomChanged(const QString& newZoom);
    void signalSelectionHasBeenMade(const KMap::GeoCoordinates::Pair& coordinates);

};

} /* namespace KMap */

#endif /* BACKEND_MAP_H */
