/** ===========================================================
 * @file
 *
 * This file is a part of digiKam project
 * <a href="http://www.digikam.org">http://www.digikam.org</a>
 *
 * @date   2010-07-14
 * @brief  Common internal data structures for libkmap
 *
 * @author Copyright (C) 2010, 2011 by Michael G. Hansen
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

#ifndef KMAP_COMMON_H
#define KMAP_COMMON_H

// Qt includes

#include <QPixmap>
#include <QPoint>
#include <QPointer>
#include <QSharedData>
#include <QSize>

// KDE includes

#include <kstandarddirs.h>

// libkmap includes

#include "kmap_primitives.h"
#include "tileindex.h"

namespace KMap
{

class AbstractMarkerTiler;
class KMapWidget;
class MapBackend;
class ModelHelper;
class TileGrouper;

/**
 * @brief Class to hold information about map widgets stored in the KMapGlobalObject
 *
 * @todo The list of these info structures has to be cleaned up periodically
 */
class KMapInternalWidgetInfo
{
public:

    typedef void (*DeleteFunction)(KMapInternalWidgetInfo* const info);

    enum InternalWidgetState
    {
        InternalWidgetReleased    = 1,
        InternalWidgetUndocked    = 2,
        InternalWidgetStillDocked = 4
    };

    Q_DECLARE_FLAGS(InternalWidgetStates, InternalWidgetState)

    InternalWidgetStates state;
    QPointer<QWidget>    widget;
    QVariant             backendData;
    QString              backendName;
    QPointer<QObject>    currentOwner;
    DeleteFunction       deleteFunction;

    KMapInternalWidgetInfo()
     : state(),
       widget(),
       backendData(),
       backendName(),
       currentOwner(0),
       deleteFunction(0)
    {
    }
};

Q_DECLARE_OPERATORS_FOR_FLAGS(KMapInternalWidgetInfo::InternalWidgetStates)

/**
 * @brief Global object for libkmap to hold items common to all KMapWidget instances
 */
class KMapGlobalObject : public QObject
{
    Q_OBJECT

public:
    static KMapGlobalObject* instance();

    /// @name Shared pixmaps
    //@{
    QPixmap getMarkerPixmap(const QString pixmapId);
    QPixmap getStandardMarkerPixmap();
    KUrl locateDataFile(const QString filename);
    //@}

    /// @name Shared internal map widgets
    //@{
    void removeMyInternalWidgetFromPool(const MapBackend* const mapBackend);
    bool getInternalWidgetFromPool(const MapBackend* const mapBackend, KMapInternalWidgetInfo* const targetInfo);
    void addMyInternalWidgetToPool(const KMapInternalWidgetInfo& info);
    void updatePooledWidgetState(const QWidget* const widget, const KMapInternalWidgetInfo::InternalWidgetState newState);
    void clearWidgetPool();
    //@}

private:
    KMapGlobalObject();
    ~KMapGlobalObject();

    class Private;
    Private* const d;

    Q_DISABLE_COPY(KMapGlobalObject)

    friend class KMapGlobalObjectCreator;
};

class KMapCluster
{
public:

    typedef QList<KMapCluster> List;

    KMapCluster()
        : tileIndicesList(),
          markerCount(0),
          markerSelectedCount(0),
          coordinates(),
          pixelPos(),
          groupState(KMapSelectedNone),
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
    KMapGroupState      groupState;
    QMap<int, QVariant> representativeMarkers;

    enum PixmapType
    {
        PixmapMarker,
        PixmapCircle,
        PixmapImage
    } pixmapType;
    QSize pixmapSize;

    //! anchor point of the image, measured from bottom-left
    QPoint pixmapOffset;
};

/// @todo Move these somewhere else
const int KMapMinMarkerGroupingRadius        = 1;
const int KMapMinThumbnailGroupingRadius     = 15;
const int KMapMinThumbnailSize               = KMapMinThumbnailGroupingRadius * 2;

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
    return (a.x()-b.x())*(a.x()-b.x()) + (a.y()-b.y())*(a.y()-b.y());
}

class KMapSharedData : public QSharedData
{
public:

    KMapSharedData()
        : QSharedData(),
          worldMapWidget(0),
          tileGrouper(0),
          markerModel(0),
          clusterList(),
          showThumbnails(true),
          thumbnailSize(KMapMinThumbnailSize),
          thumbnailGroupingRadius(KMapMinThumbnailGroupingRadius),
          markerGroupingRadius(KMapMinMarkerGroupingRadius),
          previewSingleItems(true),
          previewGroupedItems(true),
          showNumbersOnItems(true),
          sortKey(0),
          modificationsAllowed(true),
          selectionRectangle(),
          haveMovingCluster(false),
          currentMouseMode(0),
          availableMouseModes(0),
          visibleMouseModes(0),
          activeState(false)
    {
    }

    /// @name Objects
    //@{
    KMapWidget*               worldMapWidget;
    TileGrouper*              tileGrouper;
    AbstractMarkerTiler*      markerModel;
    KMapCluster::List         clusterList;
    QList<ModelHelper*>       ungroupedModels;
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

    /// @todo De-inline?
    bool                      hasRegionSelection() const { return selectionRectangle.first.hasCoordinates(); }
};

// helper functions:

KMAP_EXPORT bool KMapHelperParseLatLonString(const QString& latLonString, GeoCoordinates* const coordinates);
KMAP_EXPORT bool KMapHelperParseXYStringToPoint(const QString& xyString, QPoint* const point);
KMAP_EXPORT bool KMapHelperParseBoundsString(const QString& boundsString, QPair<GeoCoordinates, GeoCoordinates>* const boundsCoordinates);
KMAP_EXPORT GeoCoordinates::PairList KMapHelperNormalizeBounds(const GeoCoordinates::Pair& boundsPair);

} /* namespace KMap */

#endif /* KMAP_COMMON_H */
