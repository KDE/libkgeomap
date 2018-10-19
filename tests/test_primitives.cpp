/** ===========================================================
 *
 * This file is a part of KDE project
 *
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

#include "types.h"
#include "kgeomap_common.h"

using namespace KGeoMap;

void TestPrimitives::testNoOp()
{
}

void TestPrimitives::testParseLatLonString()
{
    // make sure there is no crash on null-pointer
    QVERIFY(KGeoMapHelperParseLatLonString(QLatin1String("52,6"), nullptr));

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

    QVERIFY(!KGeoMapHelperParseLatLonString(QLatin1String(""), nullptr));
    QVERIFY(!KGeoMapHelperParseLatLonString(QLatin1String("52.6"), nullptr));
    QVERIFY(!KGeoMapHelperParseLatLonString(QLatin1String("52.6,"), nullptr));
    QVERIFY(!KGeoMapHelperParseLatLonString(QLatin1String(",6"), nullptr));
    QVERIFY(!KGeoMapHelperParseLatLonString(QLatin1String("a52,6"), nullptr));
    QVERIFY(!KGeoMapHelperParseLatLonString(QLatin1String("52,a"), nullptr));
    QVERIFY(!KGeoMapHelperParseLatLonString(QLatin1String("52,6a"), nullptr));
    QVERIFY(!KGeoMapHelperParseLatLonString(QLatin1String("(52,6)"), nullptr));
}

void TestPrimitives::testParseXYStringToPoint()
{
    // make sure there is no crash on null-pointer
    QVERIFY(KGeoMapHelperParseXYStringToPoint(QLatin1String("(52,6)"), nullptr));

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

    QVERIFY(KGeoMapHelperParseXYStringToPoint(QString::fromLatin1("(204.94641003022224, 68.00444002512285)"), &point));

    QVERIFY(!KGeoMapHelperParseXYStringToPoint(QLatin1String(""), nullptr));
    QVERIFY(!KGeoMapHelperParseXYStringToPoint(QLatin1String("()"), nullptr));
    QVERIFY(!KGeoMapHelperParseXYStringToPoint(QLatin1String("(52)"), nullptr));
    QVERIFY(!KGeoMapHelperParseXYStringToPoint(QLatin1String("(52,6a)"), nullptr));
    QVERIFY(!KGeoMapHelperParseXYStringToPoint(QLatin1String("(a52,6)"), nullptr));
    QVERIFY(!KGeoMapHelperParseXYStringToPoint(QLatin1String("52,6"), nullptr));
    QVERIFY(!KGeoMapHelperParseXYStringToPoint(QLatin1String("(,6)"), nullptr));
    QVERIFY(!KGeoMapHelperParseXYStringToPoint(QLatin1String("(6,)"), nullptr));
}

void TestPrimitives::testParseBoundsString()
{
    // make sure there is no crash on null-pointer
    QVERIFY(KGeoMapHelperParseBoundsString(QLatin1String("((-52,-6),(52,6))"), nullptr));

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

    QVERIFY(!KGeoMapHelperParseBoundsString(QLatin1String(" (-52.5,-6.5),(52.5,6.5))"), nullptr));
    QVERIFY(!KGeoMapHelperParseBoundsString(QLatin1String("((-52.5,-6.5),(52.5,6.5) "), nullptr));
    QVERIFY(!KGeoMapHelperParseBoundsString(QLatin1String("((-52.5,-6.5), 52.5,6.5))"), nullptr));
    QVERIFY(!KGeoMapHelperParseBoundsString(QLatin1String("((-52.5,-6.5  (52.5,6.5))"), nullptr));
    QVERIFY(!KGeoMapHelperParseBoundsString(QLatin1String("((-52.5 -6.5),(52.5,6.5))"), nullptr));
    QVERIFY(!KGeoMapHelperParseBoundsString(QLatin1String("((-52.5,-6.5),(52.5 6.5))"), nullptr));
    QVERIFY(!KGeoMapHelperParseBoundsString(QLatin1String("( -52.5,-6.5),(52.5,6.5))"), nullptr));
    QVERIFY(!KGeoMapHelperParseBoundsString(QLatin1String("((-52.5,-6.5),(52.5,6.5)a"), nullptr));
    QVERIFY(!KGeoMapHelperParseBoundsString(QLatin1String("((-52.5,),(52.5,6.5))"),     nullptr));
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
        GroupStateComputer c1;
        QCOMPARE(c1.getState(), SelectedNone);
        c1.addSelectedState(SelectedNone);
        QCOMPARE(c1.getState(), SelectedNone);
        c1.addSelectedState(SelectedSome);
        QCOMPARE(c1.getState(), SelectedSome);
        c1.addSelectedState(SelectedAll);
        QCOMPARE(c1.getState(), SelectedSome);
        c1.clear();
        QCOMPARE(c1.getState(), SelectedNone);
        c1.addSelectedState(SelectedAll);
        QCOMPARE(c1.getState(), SelectedAll);
        c1.addSelectedState(SelectedSome);
        QCOMPARE(c1.getState(), SelectedSome);
        c1.clear();
        QCOMPARE(c1.getState(), SelectedNone);
        c1.addSelectedState(SelectedAll);
        QCOMPARE(c1.getState(), SelectedAll);
        c1.addSelectedState(SelectedNone);
        QCOMPARE(c1.getState(), SelectedSome);
    }

    {
        // test selected state:
        GroupStateComputer c1;
        QCOMPARE(c1.getState(), FilteredPositiveNone);
        c1.addFilteredPositiveState(FilteredPositiveNone);
        QCOMPARE(c1.getState(), FilteredPositiveNone);
        c1.addFilteredPositiveState(FilteredPositiveSome);
        QCOMPARE(c1.getState(), FilteredPositiveSome);
        c1.addFilteredPositiveState(FilteredPositiveAll);
        QCOMPARE(c1.getState(), FilteredPositiveSome);
        c1.clear();
        QCOMPARE(c1.getState(), FilteredPositiveNone);
        c1.addFilteredPositiveState(FilteredPositiveAll);
        QCOMPARE(c1.getState(), FilteredPositiveAll);
        c1.addFilteredPositiveState(FilteredPositiveSome);
        QCOMPARE(c1.getState(), FilteredPositiveSome);
        c1.clear();
        QCOMPARE(c1.getState(), FilteredPositiveNone);
        c1.addFilteredPositiveState(FilteredPositiveAll);
        QCOMPARE(c1.getState(), FilteredPositiveAll);
        c1.addFilteredPositiveState(FilteredPositiveNone);
        QCOMPARE(c1.getState(), FilteredPositiveSome);
    }

    {
        // test selected state:
        GroupStateComputer c1;
        QCOMPARE(c1.getState(), RegionSelectedNone);
        c1.addRegionSelectedState(RegionSelectedNone);
        QCOMPARE(c1.getState(), RegionSelectedNone);
        c1.addRegionSelectedState(RegionSelectedSome);
        QCOMPARE(c1.getState(), RegionSelectedSome);
        c1.addRegionSelectedState(RegionSelectedAll);
        QCOMPARE(c1.getState(), RegionSelectedSome);
        c1.clear();
        QCOMPARE(c1.getState(), RegionSelectedNone);
        c1.addRegionSelectedState(RegionSelectedAll);
        QCOMPARE(c1.getState(), RegionSelectedAll);
        c1.addRegionSelectedState(RegionSelectedSome);
        QCOMPARE(c1.getState(), RegionSelectedSome);
        c1.clear();
        QCOMPARE(c1.getState(), RegionSelectedNone);
        c1.addRegionSelectedState(RegionSelectedAll);
        QCOMPARE(c1.getState(), RegionSelectedAll);
        c1.addRegionSelectedState(RegionSelectedNone);
        QCOMPARE(c1.getState(), RegionSelectedSome);
    }

    /// @todo Test addState
}

QTEST_GUILESS_MAIN(TestPrimitives)
