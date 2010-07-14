/** ===========================================================
 *
 * This file is a part of digiKam project
 * <a href="http://www.digikam.org">http://www.digikam.org</a>
 *
 * @date   2010-07-14
 * @brief  Common internal data structures for libkmap
 *
 * @author Copyright (C) 2010 by Michael G. Hansen
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

#ifndef KMAP_COMMON_H
#define KMAP_COMMON_H

// libkmap includes

#include "kmap_primitives.h"

namespace KMapIface
{

class WMWSharedData : public QSharedData
{
public:
    WMWSharedData()
        : QSharedData(),
          worldMapWidget(0),
          visibleMarkers(),
          markerModel(0),
          clusterList(),
          inEditMode(false),
          editEnabled(true),
          haveMovingCluster(false),
          markerPixmap(),
          markerPixmaps(),
          previewSingleItems(true),
          previewGroupedItems(true),
          showNumbersOnItems(true),
          sortKey(0)
    {
        QStringList markerColors;
        markerColors << "00ff00" << "00ffff" << "ff0000" << "ff7f00" << "ffff00";
        QStringList stateNames;
        stateNames << "" << "-selected" << "-someselected";
        for (QStringList::const_iterator it = markerColors.constBegin(); it!=markerColors.constEnd(); ++it)
        {
            for (QStringList::const_iterator sit = stateNames.constBegin(); sit!=stateNames.constEnd(); ++sit)
            {
                const QString pixmapName = *it + *sit;
                const KUrl markerUrl = KStandardDirs::locate("data", QString("libkmap/marker-%1.png").arg(pixmapName));
                markerPixmaps[pixmapName] = QPixmap(markerUrl.toLocalFile());
            }
        }
        markerPixmap = markerPixmaps["00ff00"];
    }

    KMap*                     worldMapWidget;
    QIntList                  visibleMarkers;
    MarkerModel*              markerModel;
    WMWCluster::List          clusterList;
    QList<WMWModelHelper*>    ungroupedModels;
    bool                      inEditMode;
    bool                      editEnabled;
    bool                      haveMovingCluster;
    QPixmap                   markerPixmap;
    QMap<QString, QPixmap>    markerPixmaps;
    bool                      previewSingleItems;
    bool                      previewGroupedItems;
    bool                      showNumbersOnItems;
    int                       sortKey;
};

// helper functions:

KMAP_EXPORT bool WMWHelperParseLatLonString(const QString& latLonString, WMWGeoCoordinate* const coordinates);
KMAP_EXPORT bool WMWHelperParseXYStringToPoint(const QString& xyString, QPoint* const point);
KMAP_EXPORT bool WMWHelperParseBoundsString(const QString& boundsString, QPair<WMWGeoCoordinate, WMWGeoCoordinate>* const boundsCoordinates);
KMAP_EXPORT WMWGeoCoordinate::PairList WMWHelperNormalizeBounds(const WMWGeoCoordinate::Pair& boundsPair);

} /* KMapIface */

#endif /* KMAP_COMMON_H */
