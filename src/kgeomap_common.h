/** ===========================================================
 * @file
 *
 * This file is a part of KDE project
 *
 *
 * @date   2010-07-14
 * @brief  Common internal data structures for libkgeomap
 *
 * @author Copyright (C) 2010, 2011, 2014 by Michael G. Hansen
 *         <a href="mailto:mike at mghansen dot de">mike at mghansen dot de</a>
 * @author Copyright (C) 2010-2015 by Gilles Caulier
 *         <a href="mailto:caulier dot gilles at gmail dot com">caulier dot gilles at gmail dot com</a>
 * @author Copyright (C) 2014 by Justus Schwartz
 *         <a href="mailto:justus at gmx dot li">justus at gmx dot li</a>
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

#ifndef KGEOMAP_COMMON_H
#define KGEOMAP_COMMON_H

// Qt includes

#include <QtCore/QPoint>
#include <QtCore/QPointer>
#include <QtCore/QSharedData>
#include <QtCore/QSize>
#include <QtWidgets/QWidget>
#include <QtGui/QPixmap>

// libkgeomap includes

#include "types.h"
#include "tileindex.h"
#include "groupstatecomputer.h"

namespace KGeoMap
{

class AbstractMarkerTiler;
class MapWidget;
class MapBackend;
class ModelHelper;
class TileGrouper;
class TrackManager;

/**
 * @brief Class to hold information about map widgets stored in the KGeoMapGlobalObject
 *
 * @todo The list of these info structures has to be cleaned up periodically
 */
class KGeoMapInternalWidgetInfo
{
public:

    typedef void (*DeleteFunction)(KGeoMapInternalWidgetInfo* const info);

    enum InternalWidgetState
    {
        InternalWidgetReleased    = 1,
        InternalWidgetUndocked    = 2,
        InternalWidgetStillDocked = 4
    };

    Q_DECLARE_FLAGS(InternalWidgetStates, InternalWidgetState)

    KGeoMapInternalWidgetInfo()
        : state(),
          widget(),
          backendData(),
          backendName(),
          currentOwner(nullptr),
          deleteFunction(nullptr)
    {
    }

public:

    InternalWidgetStates state;
    QPointer<QWidget>    widget;
    QVariant             backendData;
    QString              backendName;
    QPointer<QObject>    currentOwner;
    DeleteFunction       deleteFunction;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(KGeoMapInternalWidgetInfo::InternalWidgetStates)

/**
 * @brief Global object for libkgeomap to hold items common to all KGeoMapWidget instances
 */
class KGeoMapGlobalObject : public QObject
{
    Q_OBJECT

public:

    static KGeoMapGlobalObject* instance();

    /// @name Shared pixmaps
    //@{
    QPixmap getMarkerPixmap(const QString pixmapId);
    QPixmap getStandardMarkerPixmap();
    QUrl    locateDataFile(const QString filename);
    //@}

    /// @name Shared internal map widgets
    //@{
    void removeMyInternalWidgetFromPool(const MapBackend* const mapBackend);
    bool getInternalWidgetFromPool(const MapBackend* const mapBackend, KGeoMapInternalWidgetInfo* const targetInfo);
    void addMyInternalWidgetToPool(const KGeoMapInternalWidgetInfo& info);
    void updatePooledWidgetState(const QWidget* const widget, const KGeoMapInternalWidgetInfo::InternalWidgetState newState);
    void clearWidgetPool();
    //@}

private:

    KGeoMapGlobalObject();
    ~KGeoMapGlobalObject() override;

    class Private;
    Private* const d;

    Q_DISABLE_COPY(KGeoMapGlobalObject)

    friend class KGeoMapGlobalObjectCreator;
};

class KGeoMapCluster
{

public:

    enum PixmapType
    {
        PixmapMarker,
        PixmapCircle,
        PixmapImage
    } pixmapType;

    typedef QList<KGeoMapCluster> List;

public:

    KGeoMapCluster()
        : pixmapType(PixmapMarker),
          tileIndicesList(),
          markerCount(0),
          markerSelectedCount(0),
          coordinates(),
          pixelPos(),
          groupState(SelectedNone),
          representativeMarkers(),
          pixmapSize(),
          pixmapOffset()
    {
    }

    QList<TileIndex>    tileIndicesList;
    int                 markerCount;
    int                 markerSelectedCount;
    GeoCoordinates      coordinates;
    QPoint              pixelPos;
    GroupState          groupState;
    QMap<int, QVariant> representativeMarkers;

    QSize               pixmapSize;

    //! anchor point of the image, measured from bottom-left
    QPoint              pixmapOffset;
};

/// @todo Move these somewhere else
const int KGeoMapMinMarkerGroupingRadius    = 1;
const int KGeoMapMinThumbnailGroupingRadius = 15;
const int KGeoMapMinThumbnailSize           = KGeoMapMinThumbnailGroupingRadius * 2;

/**
 * @brief Helper function, returns the square of the distance between two points
 * @todo Move this function somewhere else
 *
 * @param a Point a
 * @param b Point b
 * @return Square of the distance between a and b
 */
inline int QPointSquareDistance(const QPoint& a, const QPoint& b)
{
    return (a.x() - b.x()) * (a.x() - b.x()) + (a.y() - b.y()) * (a.y() - b.y());
}

class KGeoMapSharedData : public QSharedData
{
public:

    KGeoMapSharedData()
        : QSharedData(),
          worldMapWidget(nullptr),
          tileGrouper(nullptr),
          markerModel(nullptr),
          clusterList(),
          trackManager(nullptr),
          showThumbnails(true),
          thumbnailSize(KGeoMapMinThumbnailSize),
          thumbnailGroupingRadius(KGeoMapMinThumbnailGroupingRadius),
          markerGroupingRadius(KGeoMapMinMarkerGroupingRadius),
          previewSingleItems(true),
          previewGroupedItems(true),
          showNumbersOnItems(true),
          sortKey(0),
          modificationsAllowed(true),
          selectionRectangle(),
          haveMovingCluster(false),
          currentMouseMode(nullptr),
          availableMouseModes(nullptr),
          visibleMouseModes(nullptr),
          activeState(false)
    {
    }

    /// @todo De-inline?
    bool hasRegionSelection() const
    {
        return selectionRectangle.first.hasCoordinates();
    }

public:

    /// @name Objects
    //@{
    MapWidget*                worldMapWidget;
    TileGrouper*              tileGrouper;
    AbstractMarkerTiler*      markerModel;
    KGeoMapCluster::List      clusterList;
    QList<ModelHelper*>       ungroupedModels;
    TrackManager*             trackManager;
    //@}

    /// @name Display options
    //@{
    bool                      showThumbnails;
    int                       thumbnailSize;
    int                       thumbnailGroupingRadius;
    int                       markerGroupingRadius;
    bool                      previewSingleItems;
    bool                      previewGroupedItems;
    bool                      showNumbersOnItems;
    int                       sortKey;
    bool                      modificationsAllowed;
    //@}

    /// @name Current map state
    //@{
    GeoCoordinates::Pair      selectionRectangle;
    bool                      haveMovingCluster;
    MouseModes                currentMouseMode;
    MouseModes                availableMouseModes;
    MouseModes                visibleMouseModes;
    bool                      activeState;
    //@}
};

// helper functions:

bool KGeoMapHelperParseLatLonString(const QString& latLonString, GeoCoordinates* const coordinates);
bool KGeoMapHelperParseXYStringToPoint(const QString& xyString, QPoint* const point);
bool KGeoMapHelperParseBoundsString(const QString& boundsString, QPair<GeoCoordinates, GeoCoordinates>* const boundsCoordinates);
GeoCoordinates::PairList KGeoMapHelperNormalizeBounds(const GeoCoordinates::Pair& boundsPair);

void KGeoMap_assert(const char* const condition, const char* const filename, const int lineNumber);

} // namespace KGeoMap

#define KGEOMAP_ASSERT(cond) ((!(cond)) ? KGeoMap::KGeoMap_assert(#cond,__FILE__,__LINE__) : qt_noop())

#endif // KGEOMAP_COMMON_H
