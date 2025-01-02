/***************************************************************************
     testqgsscaleutils.cpp
     --------------------------------------
    Date                 : October 2024
    Copyright            : (C) 2024 by Germán Carrillo
    Email                : gcarrillo at linuxmail dot org
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgstest.h"
#include <QObject>
#include <QString>

#include "qgsscaleutils.h"

class TestQgsScaleUtils : public QObject
{
    Q_OBJECT

  private slots:
    void initTestCase();    // will be called before the first testfunction is executed.
    void cleanupTestCase(); // will be called after the last testfunction was executed.
    void testMaximumScaleComparisons_data();
    void testMaximumScaleComparisons();
    void testMinimumScaleComparisons_data();
    void testMinimumScaleComparisons();
};

void TestQgsScaleUtils::initTestCase()
{
  // Runs once before any tests are run
  QCoreApplication::setOrganizationName( QStringLiteral( "QGIS" ) );
  QCoreApplication::setOrganizationDomain( QStringLiteral( "qgis.org" ) );
  QCoreApplication::setApplicationName( QStringLiteral( "QGIS-TEST" ) );

  QgsApplication::init();
  QgsApplication::initQgis();
}

void TestQgsScaleUtils::cleanupTestCase()
{
  // Runs once after all tests are run
  QgsApplication::exitQgis();
}

void TestQgsScaleUtils::testMaximumScaleComparisons_data()
{
  QTest::addColumn<double>( "scale" );
  QTest::addColumn<double>( "maxScale" );
  QTest::addColumn<bool>( "expected" );

  QTest::newRow( "same scales" ) << 2500.0 << 2500.0 << false;
  QTest::newRow( "same scales, non-round below" ) << 2499.9999999966526 << 2500.0 << false;
  QTest::newRow( "same scales, non-round above" ) << 2500.0000000027226 << 2500.0 << false;
  QTest::newRow( "scale greater than max scale" ) << 3000.0 << 2500.0 << false;
  QTest::newRow( "scale less than max scale" ) << 2000.0 << 2500.0 << true;
}

void TestQgsScaleUtils::testMaximumScaleComparisons()
{
  QFETCH( double, scale );
  QFETCH( double, maxScale );
  QFETCH( bool, expected );

  QCOMPARE( QgsScaleUtils::lessThanMaximumScale( scale, maxScale ), expected );
}

void TestQgsScaleUtils::testMinimumScaleComparisons_data()
{
  QTest::addColumn<double>( "scale" );
  QTest::addColumn<double>( "minScale" );
  QTest::addColumn<bool>( "expected" );

  QTest::newRow( "same scales" ) << 5000.0 << 5000.0 << true;
  QTest::newRow( "same scales, non-round below" ) << 4999.999999997278 << 5000.0 << true;
  QTest::newRow( "same scales, non-round above" ) << 5000.000000003348 << 5000.0 << true;
  QTest::newRow( "scale greater than min scale" ) << 10000.0 << 5000.0 << true;
  QTest::newRow( "scale less than min scale" ) << 3000.0 << 5000.0 << false;
}

void TestQgsScaleUtils::testMinimumScaleComparisons()
{
  QFETCH( double, scale );
  QFETCH( double, minScale );
  QFETCH( bool, expected );

  QCOMPARE( QgsScaleUtils::equalToOrGreaterThanMinimumScale( scale, minScale ), expected );
}

QGSTEST_MAIN( TestQgsScaleUtils )
#include "testqgsscaleutils.moc"
