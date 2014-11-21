/** ===========================================================
 *
 * This file is a part of digiKam project
 * <a href="http://www.digikam.org">http://www.digikam.org</a>
 *
 * @date   2010-02-07
 * @brief  test for the simple datatypes and helper functions
 *
 * @author Copyright (C) 2010,2011,2013 by Michael G. Hansen
 *         <a href="mailto:mike at mghansen dot de">mike at mghansen dot de</a>
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

#include "test_primitives.h"

// local includes

#include "kgeomap_primitives.h"
#include "kgeomap_common.h"

using namespace KGeoMap;

void TestPrimitives::testNoOp()
{
}

void TestPrimitives::testParseLatLonString()
{
    // make sure there is no crash on null-pointer
    QVERIFY(KGeoMapHelperParseLatLonString(QLatin1String("52,6"), 0));

    GeoCoordinates coordinate;

    QVERIFY(KGeoMapHelperParseLatLonString(QLatin1String("52,6"), &coordinate));
    QCOMPARE(coordinate.geoUrl(), QLatin1String("geo:52,6"));

    QVERIFY(KGeoMapHelperParseLatLonString(QLatin1String("52.5,6.5"), &coordinate));
    QCOMPARE(coordinate.geoUrl(), QLatin1String("geo:52.5,6.5"));

    QVERIFY(KGeoMapHelperParseLatLonString(QLatin1String(" 52.5, 6.5 "), &coordinate));
    QCOMPARE(coordinate.geoUrl(), QLatin1String("geo:52.5,6.5"));

    QVERIFY(KGeoMapHelperParseLatLonString(QLatin1String("-52.5, 6.5 "), &coordinate));
    QCOMPARE(coordinate.geoUrl(), QLatin1String("geo:-52.5,6.5"));

    QVERIFY(KGeoMapHelperParseLatLonString(QLatin1String("    -52.5,  6.5   "), &coordinate));
    QCOMPARE(coordinate.geoUrl(), QLatin1String("geo:-52.5,6.5"));

    QVERIFY(KGeoMapHelperParseLatLonString(QLatin1String("52.5,-6.5"), &coordinate));
    QCOMPARE(coordinate.geoUrl(), QLatin1String("geo:52.5,-6.5"));

    QVERIFY(!KGeoMapHelperParseLatLonString(QLatin1String(""), 0));
    QVERIFY(!KGeoMapHelperParseLatLonString(QLatin1String("52.6"), 0));
    QVERIFY(!KGeoMapHelperParseLatLonString(QLatin1String("52.6,"), 0));
    QVERIFY(!KGeoMapHelperParseLatLonString(QLatin1String(",6"), 0));
    QVERIFY(!KGeoMapHelperParseLatLonString(QLatin1String("a52,6"), 0));
    QVERIFY(!KGeoMapHelperParseLatLonString(QLatin1String("52,a"), 0));
    QVERIFY(!KGeoMapHelperParseLatLonString(QLatin1String("52,6a"), 0));
    QVERIFY(!KGeoMapHelperParseLatLonString(QLatin1String("(52,6)"), 0));
}

void TestPrimitives::testParseXYStringToPoint()
{
    // make sure there is no crash on null-pointer
    QVERIFY(KGeoMapHelperParseXYStringToPoint(QLatin1String("(52,6)"), 0));

    QPoint point;

    QVERIFY(KGeoMapHelperParseXYStringToPoint(QLatin1String("(52,6)"), &point));
    QCOMPARE(point, QPoint(52,6));

    QVERIFY(KGeoMapHelperParseXYStringToPoint(QLatin1String("(10,20)"), &point));
    QCOMPARE(point, QPoint(10,20));

    QVERIFY(KGeoMapHelperParseXYStringToPoint(QLatin1String(" ( 52, 6 ) "), &point));
    QCOMPARE(point, QPoint(52,6));

    QVERIFY(KGeoMapHelperParseXYStringToPoint(QLatin1String("  ( 52, 6 )  "), &point));
    QCOMPARE(point, QPoint(52,6));

    // We used to expect integer string results, but floats are also possible.
    // BKO 270624
    // KGeoMapHelperParseXYStringToPoint always rounds them to 0.
    QVERIFY(KGeoMapHelperParseXYStringToPoint(QLatin1String("  ( 52.5, 6.5 )  "), &point));
    QCOMPARE(point, QPoint(52,6));
    QVERIFY(KGeoMapHelperParseXYStringToPoint(QLatin1String("  ( -52.5, 6.5 )  "), &point));
    QCOMPARE(point, QPoint(-52,6));

    QVERIFY(KGeoMapHelperParseXYStringToPoint("(204.94641003022224, 68.00444002512285)", &point));

    QVERIFY(!KGeoMapHelperParseXYStringToPoint(QLatin1String(""), 0));
    QVERIFY(!KGeoMapHelperParseXYStringToPoint(QLatin1String("()"), 0));
    QVERIFY(!KGeoMapHelperParseXYStringToPoint(QLatin1String("(52)"), 0));
    QVERIFY(!KGeoMapHelperParseXYStringToPoint(QLatin1String("(52,6a)"), 0));
    QVERIFY(!KGeoMapHelperParseXYStringToPoint(QLatin1String("(a52,6)"), 0));
    QVERIFY(!KGeoMapHelperParseXYStringToPoint(QLatin1String("52,6"), 0));
    QVERIFY(!KGeoMapHelperParseXYStringToPoint(QLatin1String("(,6)"), 0));
    QVERIFY(!KGeoMapHelperParseXYStringToPoint(QLatin1String("(6,)"), 0));
}

void TestPrimitives::testParseBoundsString()
{
    // make sure there is no crash on null-pointer
    QVERIFY(KGeoMapHelperParseBoundsString(QLatin1String("((-52,-6),(52,6))"), 0));

    GeoCoordinates::Pair bounds;

    QVERIFY(KGeoMapHelperParseBoundsString(QLatin1String("((-52,-6),(52,6))"), &bounds));
    QCOMPARE(bounds.first.geoUrl(), QLatin1String("geo:-52,-6"));
    QCOMPARE(bounds.second.geoUrl(), QLatin1String("geo:52,6"));

    QVERIFY(KGeoMapHelperParseBoundsString(QLatin1String("((-52,-6), (52,6))"), &bounds));
    QCOMPARE(bounds.first.geoUrl(), QLatin1String("geo:-52,-6"));
    QCOMPARE(bounds.second.geoUrl(), QLatin1String("geo:52,6"));

    QVERIFY(KGeoMapHelperParseBoundsString(QLatin1String("((-52, -6), (52, 6))"), &bounds));
    QCOMPARE(bounds.first.geoUrl(), QLatin1String("geo:-52,-6"));
    QCOMPARE(bounds.second.geoUrl(), QLatin1String("geo:52,6"));

    QVERIFY(KGeoMapHelperParseBoundsString(QLatin1String("((10,20),(30,40))"), &bounds));
    QCOMPARE(bounds.first.geoUrl(), QLatin1String("geo:10,20"));
    QCOMPARE(bounds.second.geoUrl(), QLatin1String("geo:30,40"));

    QVERIFY(KGeoMapHelperParseBoundsString(QLatin1String("((-52.5,-6.5),(52.5,6.5))"), &bounds));
    QCOMPARE(bounds.first.geoUrl(), QLatin1String("geo:-52.5,-6.5"));
    QCOMPARE(bounds.second.geoUrl(), QLatin1String("geo:52.5,6.5"));

    QVERIFY(!KGeoMapHelperParseBoundsString(QLatin1String(" (-52.5,-6.5),(52.5,6.5))"), 0));
    QVERIFY(!KGeoMapHelperParseBoundsString(QLatin1String("((-52.5,-6.5),(52.5,6.5) "), 0));
    QVERIFY(!KGeoMapHelperParseBoundsString(QLatin1String("((-52.5,-6.5), 52.5,6.5))"), 0));
    QVERIFY(!KGeoMapHelperParseBoundsString(QLatin1String("((-52.5,-6.5  (52.5,6.5))"), 0));
    QVERIFY(!KGeoMapHelperParseBoundsString(QLatin1String("((-52.5 -6.5),(52.5,6.5))"), 0));
    QVERIFY(!KGeoMapHelperParseBoundsString(QLatin1String("((-52.5,-6.5),(52.5 6.5))"), 0));
    QVERIFY(!KGeoMapHelperParseBoundsString(QLatin1String("( -52.5,-6.5),(52.5,6.5))"), 0));
    QVERIFY(!KGeoMapHelperParseBoundsString(QLatin1String("((-52.5,-6.5),(52.5,6.5)a"), 0));
    QVERIFY(!KGeoMapHelperParseBoundsString(QLatin1String("((-52.5,),(52.5,6.5))"), 0));
}

void TestPrimitives::testNormalizeBounds_data()
{
    QTest::addColumn<GeoCoordinates::Pair>("bounds");
    QTest::addColumn<QList<GeoCoordinates::Pair> >("nbounds");

    // these ones should not be split:
    QTest::newRow("top-left")
        << GeoCoordinates::makePair(10, 20, 12, 22)
        << ( GeoCoordinates::PairList() << GeoCoordinates::makePair(10, 20, 12, 22) );

    QTest::newRow("bottom-left")
        << GeoCoordinates::makePair(-12, 20, -10, 22)
        << ( GeoCoordinates::PairList() << GeoCoordinates::makePair(-12, 20, -10, 22) );

    QTest::newRow("top-right")
        << GeoCoordinates::makePair(10, -22, 12, -20)
        << ( GeoCoordinates::PairList() << GeoCoordinates::makePair(10, -22, 12, -20) );

    QTest::newRow("bottom-right")
        << GeoCoordinates::makePair(-12, -22, -10, -20)
        << ( GeoCoordinates::PairList() << GeoCoordinates::makePair(-12, -22, -10, -20) );

    QTest::newRow("cross_origin")
        << GeoCoordinates::makePair(-12, -22, 10, 20)
        << ( GeoCoordinates::PairList() << GeoCoordinates::makePair(-12, -22, 10, 20) );

    // these ones should be split:
    QTest::newRow("cross_date_1")
        << GeoCoordinates::makePair(10, 20, 15, -170)
        << ( GeoCoordinates::PairList()
            << GeoCoordinates::makePair(10, -180, 15, -170)
            << GeoCoordinates::makePair(10, 20, 15, 180)
        );

    QTest::newRow("cross_date_2")
        << GeoCoordinates::makePair(-10, 20, 15, -170)
        << ( GeoCoordinates::PairList()
                << GeoCoordinates::makePair(-10, -180, 15, -170)
                << GeoCoordinates::makePair(-10, 20, 15, 180)
            );
}

void TestPrimitives::testNormalizeBounds()
{
    QFETCH(GeoCoordinates::Pair, bounds);

    QTEST(KGeoMapHelperNormalizeBounds(bounds), "nbounds");
}

void TestPrimitives::testGroupStateComputer()
{
    {
        // test selected state:
        KGeoMapGroupStateComputer c1;
        QCOMPARE(c1.getState(), KGeoMapSelectedNone);
        c1.addSelectedState(KGeoMapSelectedNone);
        QCOMPARE(c1.getState(), KGeoMapSelectedNone);
        c1.addSelectedState(KGeoMapSelectedSome);
        QCOMPARE(c1.getState(), KGeoMapSelectedSome);
        c1.addSelectedState(KGeoMapSelectedAll);
        QCOMPARE(c1.getState(), KGeoMapSelectedSome);
        c1.clear();
        QCOMPARE(c1.getState(), KGeoMapSelectedNone);
        c1.addSelectedState(KGeoMapSelectedAll);
        QCOMPARE(c1.getState(), KGeoMapSelectedAll);
        c1.addSelectedState(KGeoMapSelectedSome);
        QCOMPARE(c1.getState(), KGeoMapSelectedSome);
        c1.clear();
        QCOMPARE(c1.getState(), KGeoMapSelectedNone);
        c1.addSelectedState(KGeoMapSelectedAll);
        QCOMPARE(c1.getState(), KGeoMapSelectedAll);
        c1.addSelectedState(KGeoMapSelectedNone);
        QCOMPARE(c1.getState(), KGeoMapSelectedSome);
    }

    {
        // test selected state:
        KGeoMapGroupStateComputer c1;
        QCOMPARE(c1.getState(), KGeoMapFilteredPositiveNone);
        c1.addFilteredPositiveState(KGeoMapFilteredPositiveNone);
        QCOMPARE(c1.getState(), KGeoMapFilteredPositiveNone);
        c1.addFilteredPositiveState(KGeoMapFilteredPositiveSome);
        QCOMPARE(c1.getState(), KGeoMapFilteredPositiveSome);
        c1.addFilteredPositiveState(KGeoMapFilteredPositiveAll);
        QCOMPARE(c1.getState(), KGeoMapFilteredPositiveSome);
        c1.clear();
        QCOMPARE(c1.getState(), KGeoMapFilteredPositiveNone);
        c1.addFilteredPositiveState(KGeoMapFilteredPositiveAll);
        QCOMPARE(c1.getState(), KGeoMapFilteredPositiveAll);
        c1.addFilteredPositiveState(KGeoMapFilteredPositiveSome);
        QCOMPARE(c1.getState(), KGeoMapFilteredPositiveSome);
        c1.clear();
        QCOMPARE(c1.getState(), KGeoMapFilteredPositiveNone);
        c1.addFilteredPositiveState(KGeoMapFilteredPositiveAll);
        QCOMPARE(c1.getState(), KGeoMapFilteredPositiveAll);
        c1.addFilteredPositiveState(KGeoMapFilteredPositiveNone);
        QCOMPARE(c1.getState(), KGeoMapFilteredPositiveSome);
    }

    {
        // test selected state:
        KGeoMapGroupStateComputer c1;
        QCOMPARE(c1.getState(), KGeoMapRegionSelectedNone);
        c1.addRegionSelectedState(KGeoMapRegionSelectedNone);
        QCOMPARE(c1.getState(), KGeoMapRegionSelectedNone);
        c1.addRegionSelectedState(KGeoMapRegionSelectedSome);
        QCOMPARE(c1.getState(), KGeoMapRegionSelectedSome);
        c1.addRegionSelectedState(KGeoMapRegionSelectedAll);
        QCOMPARE(c1.getState(), KGeoMapRegionSelectedSome);
        c1.clear();
        QCOMPARE(c1.getState(), KGeoMapRegionSelectedNone);
        c1.addRegionSelectedState(KGeoMapRegionSelectedAll);
        QCOMPARE(c1.getState(), KGeoMapRegionSelectedAll);
        c1.addRegionSelectedState(KGeoMapRegionSelectedSome);
        QCOMPARE(c1.getState(), KGeoMapRegionSelectedSome);
        c1.clear();
        QCOMPARE(c1.getState(), KGeoMapRegionSelectedNone);
        c1.addRegionSelectedState(KGeoMapRegionSelectedAll);
        QCOMPARE(c1.getState(), KGeoMapRegionSelectedAll);
        c1.addRegionSelectedState(KGeoMapRegionSelectedNone);
        QCOMPARE(c1.getState(), KGeoMapRegionSelectedSome);
    }

    /// @todo Test addState
}

QTEST_MAIN(TestPrimitives)
