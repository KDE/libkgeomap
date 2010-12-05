/** ===========================================================
 * @file
 *
 * This file is a part of digiKam project
 * <a href="http://www.digikam.org">http://www.digikam.org</a>
 *
 * @date   2010-07-14
 * @brief  Common internal data structures for libkmap
 *
 * @author Copyright (C) 2010 by Michael G. Hansen
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
#include <QSharedData>
#include <QSize>

// KDE includes

#include <kstandarddirs.h>

// libkmap includes

#include "kmap_primitives.h"

namespace KMap
{

class AbstractMarkerTiler;
class KMapWidget;
class ModelHelper;

/**
 * @brief Global object for libkmap to hold items common to all KMapWidget instances
 */
class KMapGlobalObject : public QObject
{
    Q_OBJECT

public:
    static KMapGlobalObject* instance();

    QPixmap getMarkerPixmap(const QString pixmapId);
    QPixmap getStandardMarkerPixmap();
    KUrl locateDataFile(const QString filename);

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
        selectedState(KMapSelectedNone),
        representativeMarkers(),
        pixmapSize(),
        pixmapOffset()
    {
    }

    QList<QIntList>     tileIndicesList;
    int                 markerCount;
    int                 markerSelectedCount;
    GeoCoordinates      coordinates;
    QPoint              pixelPos;
    KMapSelectionState  selectedState;
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

class KMapSharedData : public QSharedData
{
public:

    KMapSharedData()
        : QSharedData(),
          worldMapWidget(0),
          markerModel(0),
          clusterList(),
          showThumbnails(true),
          haveMovingCluster(false),
          previewSingleItems(true),
          previewGroupedItems(true),
          showNumbersOnItems(true),
          sortKey(0),
          modificationsAllowed(true)
    {
    }

    KMapWidget*               worldMapWidget;
    AbstractMarkerTiler*      markerModel;
    KMapCluster::List         clusterList;
    QList<ModelHelper*>       ungroupedModels;
    bool                      showThumbnails;
    bool                      haveMovingCluster;
    bool                      previewSingleItems;
    bool                      previewGroupedItems;
    bool                      showNumbersOnItems;
    int                       sortKey;
    bool                      modificationsAllowed;
};

// helper functions:

KMAP_EXPORT bool KMapHelperParseLatLonString(const QString& latLonString, GeoCoordinates* const coordinates);
KMAP_EXPORT bool KMapHelperParseXYStringToPoint(const QString& xyString, QPoint* const point);
KMAP_EXPORT bool KMapHelperParseBoundsString(const QString& boundsString, QPair<GeoCoordinates, GeoCoordinates>* const boundsCoordinates);
KMAP_EXPORT GeoCoordinates::PairList KMapHelperNormalizeBounds(const GeoCoordinates::Pair& boundsPair);

} /* namespace KMap */

#endif /* KMAP_COMMON_H */
