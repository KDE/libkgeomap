/* ============================================================
 *
 * Date        : 2009-12-01
 * Description : demo-program for WorldMapWidget2
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


// KDE includes

#include <KApplication>
#include <KAboutData>
#include <KCmdLineArgs>
#include <KCmdLineOptions>
#include <KDebug>

// local includes

#include "markermodel.h"

using namespace WMW2;

int main(int argc, char* argv[])
{
    const KAboutData aboutData(
        "demo-worldmapwidget2",
        0,
        ki18n("marker-model test application"),
        "0.1", // version
        ki18n("Tests the marker-model"),
        KAboutData::License_GPL,
        ki18n("(c) 2009 Michael G. Hansen"),
        ki18n(""), // optional text
        "", // URI of homepage
        "" // bugs e-mail address
    );

    KCmdLineArgs::init(argc, argv, &aboutData);

    KApplication app;

    MarkerModel mm;

    // bar
    WMWGeoCoordinate barCoordinates = WMWGeoCoordinate::fromGeoUrl("geo:51.06711205,6.90020261667,43");
    mm.addMarker(WMWMarker(barCoordinates));

    // ice cafe
    WMWGeoCoordinate iceCoordinates = WMWGeoCoordinate::fromGeoUrl("geo:51.0913031421,6.88878178596,44");
    mm.addMarker(WMWMarker(iceCoordinates));

    // Marienburg castle
    mm.addMarker(WMWMarker(WMWGeoCoordinate::fromGeoUrl("geo:51.087647318,6.88282728201,44")));

    // there should be 2 markers in the tile of the cafe here
    WMW2_ASSERT(mm.getTileMarkerCount(mm.coordinateToTileIndex(iceCoordinates, 2)) == 2);

    // head of monster
    mm.addMarker(WMWMarker(WMWGeoCoordinate::fromGeoUrl("geo:51.0889433167,6.88000331667,39.6")));

    // now there should be 3 tiles
    WMW2_ASSERT(mm.getTileMarkerCount(mm.coordinateToTileIndex(iceCoordinates, 2)) == 3);

    // Langenfeld
    mm.addMarker(WMWMarker(WMWGeoCoordinate::fromGeoUrl("geo:51.1100157609,6.94911003113,51")));

    // Sagrada Familia in Spain
    mm.addMarker(WMWMarker(WMWGeoCoordinate::fromGeoUrl("geo:41.4036480511,2.1743756533,46")));

    for (int level=0; level<=mm.maxLevel(); ++level)
    {
        QIntList tileIndex = mm.coordinateToTileIndex(barCoordinates, level);
        kDebug()<<QString("level: %1 count: %2 bar").arg(level).arg(mm.getTileMarkerCount(tileIndex));

        tileIndex = mm.coordinateToTileIndex(iceCoordinates, level);
        kDebug()<<QString("level: %1 count: %2 ice").arg(level).arg(mm.getTileMarkerCount(tileIndex));

    }

    return 0;
}


