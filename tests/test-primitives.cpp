/* ============================================================
 *
 * Date        : 2010-02-07
 * Description : test for the simple datatypes and helper functions
 *
 * Copyright (C) 2010 by Michael G. Hansen <mike at mghansen dot de>
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

// local includes

#include "test-primitives.h"

using namespace WMW2;

void TestPrimitives::testNoOp()
{
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

QTEST_MAIN(TestPrimitives)


