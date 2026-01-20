/***************************************************************************
     testqgscoordinateutils.cpp
     --------------------------------------
    Date                 : March 2022
    Copyright            : (C) 2022 Mathieu Pellerin
    Email                : nirvn dot asia at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgscoordinateutils.h"
#include "qgstest.h"

#include <QLocale>
#include <QString>

class TestQgsCoordinateUtils : public QObject
{
    Q_OBJECT
  private slots:

    void testPrecisionForCrs();
    void testDegreeWithSuffix();
    void testLocale();
    void initTestCase();
    void cleanupTestCase();
};


void TestQgsCoordinateUtils::initTestCase()
{
  QgsApplication::init();
  QgsApplication::initQgis();
}

void TestQgsCoordinateUtils::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestQgsCoordinateUtils::testPrecisionForCrs()
{
  // 8 decimal places for degrees based crs
  QCOMPARE( QgsCoordinateUtils::calculateCoordinatePrecision( QgsCoordinateReferenceSystem( "EPSG:4326" ) ), 8 );
  // 3 decimal places for others
  QCOMPARE( QgsCoordinateUtils::calculateCoordinatePrecision( QgsCoordinateReferenceSystem( "EPSG:3857" ) ), 3 );
  QCOMPARE( QgsCoordinateUtils::calculateCoordinatePrecision( QgsCoordinateReferenceSystem( "EPSG:3111" ) ), 3 );
  QCOMPARE( QgsCoordinateUtils::calculateCoordinatePrecision( QgsCoordinateReferenceSystem() ), 3 );
}

void TestQgsCoordinateUtils::testDegreeWithSuffix()
{
  bool ok = false;
  bool isEasting = false;
  double value = 0.0;

  value = QgsCoordinateUtils::degreeToDecimal( u"1.234W"_s, &ok, &isEasting );
  QCOMPARE( ok, true );
  QCOMPARE( isEasting, true );
  QCOMPARE( value, -1.234 );

  value = QgsCoordinateUtils::degreeToDecimal( u"-1.234 w"_s, &ok, &isEasting );
  QCOMPARE( ok, true );
  QCOMPARE( isEasting, true );
  QCOMPARE( value, -1.234 );

  value = QgsCoordinateUtils::degreeToDecimal( u"1.234s"_s, &ok, &isEasting );
  QCOMPARE( ok, true );
  QCOMPARE( isEasting, false );
  QCOMPARE( value, -1.234 );

  value = QgsCoordinateUtils::degreeToDecimal( u"1.234N"_s, &ok, &isEasting );
  QCOMPARE( ok, true );
  QCOMPARE( isEasting, false );
  QCOMPARE( value, 1.234 );

  value = QgsCoordinateUtils::degreeToDecimal( u"1.234 e"_s, &ok, &isEasting );
  QCOMPARE( ok, true );
  QCOMPARE( isEasting, true );
  QCOMPARE( value, 1.234 );

  value = QgsCoordinateUtils::degreeToDecimal( u"bad string"_s, &ok, &isEasting );
  QCOMPARE( ok, false );
  QCOMPARE( value, 0.0 );
}

void TestQgsCoordinateUtils::testLocale()
{
  bool ok = false;
  bool isEasting = false;
  double value = 0.0;

  QLocale::setDefault( QLocale::French );

  value = QgsCoordinateUtils::dmsToDecimal( u"6°26'55,7\""_s, &ok, &isEasting );
  QCOMPARE( ok, true );
  QGSCOMPARENEAR( value, 6.448805, 0.000001 );

  value = QgsCoordinateUtils::degreeToDecimal( u"104,34936E"_s, &ok, &isEasting );
  QCOMPARE( ok, true );
  QCOMPARE( value, 104.34936 );

  QLocale::setDefault( QLocale::English );
}

QGSTEST_MAIN( TestQgsCoordinateUtils )
#include "testqgscoordinateutils.moc"
