/** ===========================================================
 *
 * This file is a part of digiKam project
 * <a href="http://www.digikam.org">http://www.digikam.org</a>
 *
 * @date   2010-02-07
 * @brief  test for the simple datatypes and helper functions
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

#include "test-primitives.moc"

using namespace KMapIface;

void TestPrimitives::testNoOp()
{
}

void TestPrimitives::testWMWGeoCoordinates()
{
    WMWGeoCoordinate coord1(52.0, 6.0);
    QVERIFY(coord1.hasCoordinates());
    QCOMPARE(coord1.geoUrl(), QString("geo:52,6"));

    WMWGeoCoordinate coord2(52.0, 6.0);
    WMWGeoCoordinate coord3(53.0, 6.0);
    QVERIFY(coord1==coord2);
    QVERIFY(!(coord1==coord3));

    WMWGeoCoordinate coord4 = WMWGeoCoordinate(52.0, 6.0);
    QVERIFY(coord1==coord4);
}

void TestPrimitives::testParseLatLonString()
{
    // make sure there is no crash on null-pointer
    QVERIFY(WMWHelperParseLatLonString("52,6", 0));

    WMWGeoCoordinate coordinate;

    QVERIFY(WMWHelperParseLatLonString("52,6", &coordinate));
    QCOMPARE(coordinate.geoUrl(), QString("geo:52,6"));

    QVERIFY(WMWHelperParseLatLonString("52.5,6.5", &coordinate));
    QCOMPARE(coordinate.geoUrl(), QString("geo:52.5,6.5"));

    QVERIFY(WMWHelperParseLatLonString(" 52.5, 6.5 ", &coordinate));
    QCOMPARE(coordinate.geoUrl(), QString("geo:52.5,6.5"));

    QVERIFY(WMWHelperParseLatLonString("-52.5, 6.5 ", &coordinate));
    QCOMPARE(coordinate.geoUrl(), QString("geo:-52.5,6.5"));

    QVERIFY(WMWHelperParseLatLonString("    -52.5,  6.5   ", &coordinate));
    QCOMPARE(coordinate.geoUrl(), QString("geo:-52.5,6.5"));

    QVERIFY(WMWHelperParseLatLonString("52.5,-6.5", &coordinate));
    QCOMPARE(coordinate.geoUrl(), QString("geo:52.5,-6.5"));

    QVERIFY(!WMWHelperParseLatLonString("", 0));
    QVERIFY(!WMWHelperParseLatLonString("52.6", 0));
    QVERIFY(!WMWHelperParseLatLonString("52.6,", 0));
    QVERIFY(!WMWHelperParseLatLonString(",6", 0));
    QVERIFY(!WMWHelperParseLatLonString("a52,6", 0));
    QVERIFY(!WMWHelperParseLatLonString("52,a", 0));
    QVERIFY(!WMWHelperParseLatLonString("52,6a", 0));
    QVERIFY(!WMWHelperParseLatLonString("(52,6)", 0));
}

void TestPrimitives::testParseXYStringToPoint()
{
    // make sure there is no crash on null-pointer
    QVERIFY(WMWHelperParseXYStringToPoint("(52,6)", 0));

    QPoint point;

    QVERIFY(WMWHelperParseXYStringToPoint("(52,6)", &point));
    QCOMPARE(point, QPoint(52,6));

    QVERIFY(WMWHelperParseXYStringToPoint("(10,20)", &point));
    QCOMPARE(point, QPoint(10,20));

    QVERIFY(WMWHelperParseXYStringToPoint(" ( 52, 6 ) ", &point));
    QCOMPARE(point, QPoint(52,6));

    QVERIFY(WMWHelperParseXYStringToPoint("  ( 52, 6 )  ", &point));
    QCOMPARE(point, QPoint(52,6));

    QVERIFY(!WMWHelperParseXYStringToPoint("", 0));
    QVERIFY(!WMWHelperParseXYStringToPoint("()", 0));
    QVERIFY(!WMWHelperParseXYStringToPoint("(52)", 0));
    QVERIFY(!WMWHelperParseXYStringToPoint("(52,6a)", 0));
    QVERIFY(!WMWHelperParseXYStringToPoint("(a52,6)", 0));
    QVERIFY(!WMWHelperParseXYStringToPoint("52,6", 0));
    QVERIFY(!WMWHelperParseXYStringToPoint("(,6)", 0));
    QVERIFY(!WMWHelperParseXYStringToPoint("(6,)", 0));
}

void TestPrimitives::testParseBoundsString()
{
    // make sure there is no crash on null-pointer
    QVERIFY(WMWHelperParseBoundsString("((-52,-6),(52,6))", 0));

    WMWGeoCoordinate::Pair bounds;

    QVERIFY(WMWHelperParseBoundsString("((-52,-6),(52,6))", &bounds));
    QCOMPARE(bounds.first.geoUrl(), QString("geo:-52,-6"));
    QCOMPARE(bounds.second.geoUrl(), QString("geo:52,6"));

    QVERIFY(WMWHelperParseBoundsString("((-52,-6), (52,6))", &bounds));
    QCOMPARE(bounds.first.geoUrl(), QString("geo:-52,-6"));
    QCOMPARE(bounds.second.geoUrl(), QString("geo:52,6"));

    QVERIFY(WMWHelperParseBoundsString("((-52, -6), (52, 6))", &bounds));
    QCOMPARE(bounds.first.geoUrl(), QString("geo:-52,-6"));
    QCOMPARE(bounds.second.geoUrl(), QString("geo:52,6"));

    QVERIFY(WMWHelperParseBoundsString("((10,20),(30,40))", &bounds));
    QCOMPARE(bounds.first.geoUrl(), QString("geo:10,20"));
    QCOMPARE(bounds.second.geoUrl(), QString("geo:30,40"));

    QVERIFY(WMWHelperParseBoundsString("((-52.5,-6.5),(52.5,6.5))", &bounds));
    QCOMPARE(bounds.first.geoUrl(), QString("geo:-52.5,-6.5"));
    QCOMPARE(bounds.second.geoUrl(), QString("geo:52.5,6.5"));

    QVERIFY(!WMWHelperParseBoundsString(" (-52.5,-6.5),(52.5,6.5))", 0));
    QVERIFY(!WMWHelperParseBoundsString("((-52.5,-6.5),(52.5,6.5) ", 0));
    QVERIFY(!WMWHelperParseBoundsString("((-52.5,-6.5), 52.5,6.5))", 0));
    QVERIFY(!WMWHelperParseBoundsString("((-52.5,-6.5  (52.5,6.5))", 0));
    QVERIFY(!WMWHelperParseBoundsString("((-52.5 -6.5),(52.5,6.5))", 0));
    QVERIFY(!WMWHelperParseBoundsString("((-52.5,-6.5),(52.5 6.5))", 0));
    QVERIFY(!WMWHelperParseBoundsString("( -52.5,-6.5),(52.5,6.5))", 0));
    QVERIFY(!WMWHelperParseBoundsString("((-52.5,-6.5),(52.5,6.5)a", 0));
    QVERIFY(!WMWHelperParseBoundsString("((-52.5,),(52.5,6.5))", 0));
}

void TestPrimitives::testNormalizeBounds_data()
{
    QTest::addColumn<WMWGeoCoordinate::Pair>("bounds");
    QTest::addColumn<QList<WMWGeoCoordinate::Pair> >("nbounds");

    // these ones should not be split:
    QTest::newRow("top-left")
        << WMWGeoCoordinate::makePair(10, 20, 12, 22)
        << ( WMWGeoCoordinate::PairList() << WMWGeoCoordinate::makePair(10, 20, 12, 22) );

    QTest::newRow("bottom-left")
        << WMWGeoCoordinate::makePair(-12, 20, -10, 22)
        << ( WMWGeoCoordinate::PairList() << WMWGeoCoordinate::makePair(-12, 20, -10, 22) );

    QTest::newRow("top-right")
        << WMWGeoCoordinate::makePair(10, -22, 12, -20)
        << ( WMWGeoCoordinate::PairList() << WMWGeoCoordinate::makePair(10, -22, 12, -20) );

    QTest::newRow("bottom-right")
        << WMWGeoCoordinate::makePair(-12, -22, -10, -20)
        << ( WMWGeoCoordinate::PairList() << WMWGeoCoordinate::makePair(-12, -22, -10, -20) );

    QTest::newRow("cross_origin")
        << WMWGeoCoordinate::makePair(-12, -22, 10, 20)
        << ( WMWGeoCoordinate::PairList() << WMWGeoCoordinate::makePair(-12, -22, 10, 20) );

    // these ones should be split:
    QTest::newRow("cross_date_1")
    << WMWGeoCoordinate::makePair(10, 20, 15, -170)
    << ( WMWGeoCoordinate::PairList()
            << WMWGeoCoordinate::makePair(10, -180, 15, -170)
            << WMWGeoCoordinate::makePair(10, 20, 15, 180)
        );

    QTest::newRow("cross_date_2")
        << WMWGeoCoordinate::makePair(-10, 20, 15, -170)
        << ( WMWGeoCoordinate::PairList()
                << WMWGeoCoordinate::makePair(-10, -180, 15, -170)
                << WMWGeoCoordinate::makePair(-10, 20, 15, 180)
            );
}

void TestPrimitives::testNormalizeBounds()
{
    QFETCH(WMWGeoCoordinate::Pair, bounds);

    QTEST(WMWHelperNormalizeBounds(bounds), "nbounds");
}

QTEST_MAIN(TestPrimitives)
