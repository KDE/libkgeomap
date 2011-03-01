/** ===========================================================
 *
 * This file is a part of digiKam project
 * <a href="http://www.digikam.org">http://www.digikam.org</a>
 *
 * @date   2010-02-07
 * @brief  test for the simple datatypes and helper functions
 *
 * @author Copyright (C) 2010, 2011 by Michael G. Hansen
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

#include "test_primitives.moc"

// local includes

#include "kmap_primitives.h"
#include "kmap_common.h"

using namespace KMap;

void TestPrimitives::testNoOp()
{
}

void TestPrimitives::testParseLatLonString()
{
    // make sure there is no crash on null-pointer
    QVERIFY(KMapHelperParseLatLonString(QLatin1String("52,6"), 0));

    GeoCoordinates coordinate;

    QVERIFY(KMapHelperParseLatLonString(QLatin1String("52,6"), &coordinate));
    QCOMPARE(coordinate.geoUrl(), QLatin1String("geo:52,6"));

    QVERIFY(KMapHelperParseLatLonString(QLatin1String("52.5,6.5"), &coordinate));
    QCOMPARE(coordinate.geoUrl(), QLatin1String("geo:52.5,6.5"));

    QVERIFY(KMapHelperParseLatLonString(QLatin1String(" 52.5, 6.5 "), &coordinate));
    QCOMPARE(coordinate.geoUrl(), QLatin1String("geo:52.5,6.5"));

    QVERIFY(KMapHelperParseLatLonString(QLatin1String("-52.5, 6.5 "), &coordinate));
    QCOMPARE(coordinate.geoUrl(), QLatin1String("geo:-52.5,6.5"));

    QVERIFY(KMapHelperParseLatLonString(QLatin1String("    -52.5,  6.5   "), &coordinate));
    QCOMPARE(coordinate.geoUrl(), QLatin1String("geo:-52.5,6.5"));

    QVERIFY(KMapHelperParseLatLonString(QLatin1String("52.5,-6.5"), &coordinate));
    QCOMPARE(coordinate.geoUrl(), QLatin1String("geo:52.5,-6.5"));

    QVERIFY(!KMapHelperParseLatLonString(QLatin1String(""), 0));
    QVERIFY(!KMapHelperParseLatLonString(QLatin1String("52.6"), 0));
    QVERIFY(!KMapHelperParseLatLonString(QLatin1String("52.6,"), 0));
    QVERIFY(!KMapHelperParseLatLonString(QLatin1String(",6"), 0));
    QVERIFY(!KMapHelperParseLatLonString(QLatin1String("a52,6"), 0));
    QVERIFY(!KMapHelperParseLatLonString(QLatin1String("52,a"), 0));
    QVERIFY(!KMapHelperParseLatLonString(QLatin1String("52,6a"), 0));
    QVERIFY(!KMapHelperParseLatLonString(QLatin1String("(52,6)"), 0));
}

void TestPrimitives::testParseXYStringToPoint()
{
    // make sure there is no crash on null-pointer
    QVERIFY(KMapHelperParseXYStringToPoint(QLatin1String("(52,6)"), 0));

    QPoint point;

    QVERIFY(KMapHelperParseXYStringToPoint(QLatin1String("(52,6)"), &point));
    QCOMPARE(point, QPoint(52,6));

    QVERIFY(KMapHelperParseXYStringToPoint(QLatin1String("(10,20)"), &point));
    QCOMPARE(point, QPoint(10,20));

    QVERIFY(KMapHelperParseXYStringToPoint(QLatin1String(" ( 52, 6 ) "), &point));
    QCOMPARE(point, QPoint(52,6));

    QVERIFY(KMapHelperParseXYStringToPoint(QLatin1String("  ( 52, 6 )  "), &point));
    QCOMPARE(point, QPoint(52,6));

    QVERIFY(!KMapHelperParseXYStringToPoint(QLatin1String(""), 0));
    QVERIFY(!KMapHelperParseXYStringToPoint(QLatin1String("()"), 0));
    QVERIFY(!KMapHelperParseXYStringToPoint(QLatin1String("(52)"), 0));
    QVERIFY(!KMapHelperParseXYStringToPoint(QLatin1String("(52,6a)"), 0));
    QVERIFY(!KMapHelperParseXYStringToPoint(QLatin1String("(a52,6)"), 0));
    QVERIFY(!KMapHelperParseXYStringToPoint(QLatin1String("52,6"), 0));
    QVERIFY(!KMapHelperParseXYStringToPoint(QLatin1String("(,6)"), 0));
    QVERIFY(!KMapHelperParseXYStringToPoint(QLatin1String("(6,)"), 0));
}

void TestPrimitives::testParseBoundsString()
{
    // make sure there is no crash on null-pointer
    QVERIFY(KMapHelperParseBoundsString(QLatin1String("((-52,-6),(52,6))"), 0));

    GeoCoordinates::Pair bounds;

    QVERIFY(KMapHelperParseBoundsString(QLatin1String("((-52,-6),(52,6))"), &bounds));
    QCOMPARE(bounds.first.geoUrl(), QLatin1String("geo:-52,-6"));
    QCOMPARE(bounds.second.geoUrl(), QLatin1String("geo:52,6"));

    QVERIFY(KMapHelperParseBoundsString(QLatin1String("((-52,-6), (52,6))"), &bounds));
    QCOMPARE(bounds.first.geoUrl(), QLatin1String("geo:-52,-6"));
    QCOMPARE(bounds.second.geoUrl(), QLatin1String("geo:52,6"));

    QVERIFY(KMapHelperParseBoundsString(QLatin1String("((-52, -6), (52, 6))"), &bounds));
    QCOMPARE(bounds.first.geoUrl(), QLatin1String("geo:-52,-6"));
    QCOMPARE(bounds.second.geoUrl(), QLatin1String("geo:52,6"));

    QVERIFY(KMapHelperParseBoundsString(QLatin1String("((10,20),(30,40))"), &bounds));
    QCOMPARE(bounds.first.geoUrl(), QLatin1String("geo:10,20"));
    QCOMPARE(bounds.second.geoUrl(), QLatin1String("geo:30,40"));

    QVERIFY(KMapHelperParseBoundsString(QLatin1String("((-52.5,-6.5),(52.5,6.5))"), &bounds));
    QCOMPARE(bounds.first.geoUrl(), QLatin1String("geo:-52.5,-6.5"));
    QCOMPARE(bounds.second.geoUrl(), QLatin1String("geo:52.5,6.5"));

    QVERIFY(!KMapHelperParseBoundsString(QLatin1String(" (-52.5,-6.5),(52.5,6.5))"), 0));
    QVERIFY(!KMapHelperParseBoundsString(QLatin1String("((-52.5,-6.5),(52.5,6.5) "), 0));
    QVERIFY(!KMapHelperParseBoundsString(QLatin1String("((-52.5,-6.5), 52.5,6.5))"), 0));
    QVERIFY(!KMapHelperParseBoundsString(QLatin1String("((-52.5,-6.5  (52.5,6.5))"), 0));
    QVERIFY(!KMapHelperParseBoundsString(QLatin1String("((-52.5 -6.5),(52.5,6.5))"), 0));
    QVERIFY(!KMapHelperParseBoundsString(QLatin1String("((-52.5,-6.5),(52.5 6.5))"), 0));
    QVERIFY(!KMapHelperParseBoundsString(QLatin1String("( -52.5,-6.5),(52.5,6.5))"), 0));
    QVERIFY(!KMapHelperParseBoundsString(QLatin1String("((-52.5,-6.5),(52.5,6.5)a"), 0));
    QVERIFY(!KMapHelperParseBoundsString(QLatin1String("((-52.5,),(52.5,6.5))"), 0));
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

    QTEST(KMapHelperNormalizeBounds(bounds), "nbounds");
}

void TestPrimitives::testGroupStateComputer()
{
    {
        // test selected state:
        KMapGroupStateComputer c1;
        QCOMPARE(c1.getState(), KMapSelectedNone);
        c1.addSelectedState(KMapSelectedNone);
        QCOMPARE(c1.getState(), KMapSelectedNone);
        c1.addSelectedState(KMapSelectedSome);
        QCOMPARE(c1.getState(), KMapSelectedSome);
        c1.addSelectedState(KMapSelectedAll);
        QCOMPARE(c1.getState(), KMapSelectedSome);
        c1.clear();
        QCOMPARE(c1.getState(), KMapSelectedNone);
        c1.addSelectedState(KMapSelectedAll);
        QCOMPARE(c1.getState(), KMapSelectedAll);
        c1.addSelectedState(KMapSelectedSome);
        QCOMPARE(c1.getState(), KMapSelectedSome);
        c1.clear();
        QCOMPARE(c1.getState(), KMapSelectedNone);
        c1.addSelectedState(KMapSelectedAll);
        QCOMPARE(c1.getState(), KMapSelectedAll);
        c1.addSelectedState(KMapSelectedNone);
        QCOMPARE(c1.getState(), KMapSelectedSome);
    }

    {
        // test selected state:
        KMapGroupStateComputer c1;
        QCOMPARE(c1.getState(), KMapFilteredPositiveNone);
        c1.addFilteredPositiveState(KMapFilteredPositiveNone);
        QCOMPARE(c1.getState(), KMapFilteredPositiveNone);
        c1.addFilteredPositiveState(KMapFilteredPositiveSome);
        QCOMPARE(c1.getState(), KMapFilteredPositiveSome);
        c1.addFilteredPositiveState(KMapFilteredPositiveAll);
        QCOMPARE(c1.getState(), KMapFilteredPositiveSome);
        c1.clear();
        QCOMPARE(c1.getState(), KMapFilteredPositiveNone);
        c1.addFilteredPositiveState(KMapFilteredPositiveAll);
        QCOMPARE(c1.getState(), KMapFilteredPositiveAll);
        c1.addFilteredPositiveState(KMapFilteredPositiveSome);
        QCOMPARE(c1.getState(), KMapFilteredPositiveSome);
        c1.clear();
        QCOMPARE(c1.getState(), KMapFilteredPositiveNone);
        c1.addFilteredPositiveState(KMapFilteredPositiveAll);
        QCOMPARE(c1.getState(), KMapFilteredPositiveAll);
        c1.addFilteredPositiveState(KMapFilteredPositiveNone);
        QCOMPARE(c1.getState(), KMapFilteredPositiveSome);
    }

    {
        // test selected state:
        KMapGroupStateComputer c1;
        QCOMPARE(c1.getState(), KMapRegionSelectedNone);
        c1.addRegionSelectedState(KMapRegionSelectedNone);
        QCOMPARE(c1.getState(), KMapRegionSelectedNone);
        c1.addRegionSelectedState(KMapRegionSelectedSome);
        QCOMPARE(c1.getState(), KMapRegionSelectedSome);
        c1.addRegionSelectedState(KMapRegionSelectedAll);
        QCOMPARE(c1.getState(), KMapRegionSelectedSome);
        c1.clear();
        QCOMPARE(c1.getState(), KMapRegionSelectedNone);
        c1.addRegionSelectedState(KMapRegionSelectedAll);
        QCOMPARE(c1.getState(), KMapRegionSelectedAll);
        c1.addRegionSelectedState(KMapRegionSelectedSome);
        QCOMPARE(c1.getState(), KMapRegionSelectedSome);
        c1.clear();
        QCOMPARE(c1.getState(), KMapRegionSelectedNone);
        c1.addRegionSelectedState(KMapRegionSelectedAll);
        QCOMPARE(c1.getState(), KMapRegionSelectedAll);
        c1.addRegionSelectedState(KMapRegionSelectedNone);
        QCOMPARE(c1.getState(), KMapRegionSelectedSome);
    }

    /// @todo Test addState
}

QTEST_MAIN(TestPrimitives)
