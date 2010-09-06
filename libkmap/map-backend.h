/** ===========================================================
 * @file
 *
 * This file is a part of digiKam project
 * <a href="http://www.digikam.org">http://www.digikam.org</a>
 *
 * @date   2009-12-01
 * @brief  Base-class for backends for KMap
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

#ifndef MAP_BACKEND_H
#define MAP_BACKEND_H

// Qt includes

#include <QModelIndex>
#include <QWidget>

// local includes

#include "kmap_common.h"

// Marble Widget includes

#include <marble/global.h>
#include <marble/GeoDataLatLonBox.h>

class QMenu;
class QPersistentModelIndex;
class KConfigGroup;

namespace KMap
{

enum SelRectangleHDirection 
{ 
    Left = 0,
    Right
};

enum SelRectangleVDirection
{
    Up = 0,
    Down
};

class WMWSharedData;

class MapBackend : public QObject
{

Q_OBJECT

public:

    MapBackend(const QExplicitlySharedDataPointer<WMWSharedData>& sharedData, QObject* const parent);
    virtual ~MapBackend();

    virtual QString backendName() const = 0;
    virtual QString backendHumanName() const = 0;
    virtual QWidget* mapWidget() const = 0;

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

//     virtual void updateDragDropMarker(const QPoint& pos, const WMWDragData* const dragData) = 0;
//     virtual void updateDragDropMarkerPosition(const QPoint& pos) = 0;

    virtual void updateActionAvailability() = 0;

    virtual void setSelectionRectangle(const GeoCoordinates::Pair& searchCorrdinates) = 0;
    virtual GeoCoordinates::Pair getSelectionRectangle() = 0;
    virtual void removeSelectionRectangle() = 0;
    virtual void mouseModeChanged(const MouseModes mouseMode) = 0;

    const QExplicitlySharedDataPointer<WMWSharedData> s;

    virtual void setSelectionStatus(const bool status) = 0;
    //virtual bool getSelectionStatus(
    virtual void centerOn(const Marble::GeoDataLatLonBox& box) = 0;
    virtual void setActive(const bool state) = 0;

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
    void signalSelectionHasBeenMade(const KMap::GeoCoordinates::Pair& coordinates);

};

} /* namespace KMap */

#endif /* MAP_BACKEND_H */
