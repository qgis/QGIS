/***************************************************************************
    testqgsadvanceddigitizingdockwidget.cpp
     --------------------------------------
    Date                 : February 2022
    Copyright            : (C) 2022 Alessandro Pasotti
    Email                : elpaso at itopen dot it
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include <QCoreApplication>
#include <QLocale>

#include "qgstest.h"
#include "qgsapplication.h"
#include "qgsmapcanvas.h"
#include "qgsadvanceddigitizingdockwidget.h"

class TestQgsAdvancedDigitizingDockWidget : public QObject
{
    Q_OBJECT
  public:
    TestQgsAdvancedDigitizingDockWidget() = default;

  private slots:
    void initTestCase();    // will be called before the first testfunction is executed.
    void cleanupTestCase(); // will be called after the last testfunction was executed.
    void init();            // will be called before each testfunction is executed.
    void cleanup();         // will be called after every testfunction.

    void parseUserInput();
};

void TestQgsAdvancedDigitizingDockWidget::initTestCase()
{
  QgsApplication::init();
  QgsApplication::initQgis();
  QgsApplication::showSettings();
}

void TestQgsAdvancedDigitizingDockWidget::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestQgsAdvancedDigitizingDockWidget::init()
{
}

void TestQgsAdvancedDigitizingDockWidget::cleanup()
{
}

void TestQgsAdvancedDigitizingDockWidget::parseUserInput()
{
  QgsProject::instance()->clear();
  QgsMapCanvas canvas;
  QgsAdvancedDigitizingDockWidget widget { &canvas };

  bool ok;
  double result;

  QLocale::setDefault( QLocale::English );

  result = widget.parseUserInput( QStringLiteral( "1.2345" ), Qgis::CadConstraintType::Generic, ok );
  QCOMPARE( result, 1.2345 );
  QVERIFY( ok );

  result = widget.parseUserInput( QStringLiteral( "1,234.5" ), Qgis::CadConstraintType::Generic, ok );
  QCOMPARE( result, 1234.5 );
  QVERIFY( ok );

  result = widget.parseUserInput( QStringLiteral( "1,200.6/2" ), Qgis::CadConstraintType::Generic, ok );
  QCOMPARE( result, 600.3 );
  QVERIFY( ok );

  result = widget.parseUserInput( QStringLiteral( "1200.6/2" ), Qgis::CadConstraintType::Generic, ok );
  QCOMPARE( result, 600.3 );
  QVERIFY( ok );

  // Italian locale (comma as decimal separator)
  QLocale::setDefault( QLocale::Italian );

  result = widget.parseUserInput( QStringLiteral( "1,2345" ), Qgis::CadConstraintType::Generic, ok );
  QCOMPARE( result, 1.2345 );
  QVERIFY( ok );

  result = widget.parseUserInput( QStringLiteral( "1.234,5" ), Qgis::CadConstraintType::Generic, ok );
  QCOMPARE( result, 1234.5 );
  QVERIFY( ok );

  result = widget.parseUserInput( QStringLiteral( "1.200,6/2" ), Qgis::CadConstraintType::Generic, ok );
  QCOMPARE( result, 600.3 );
  QVERIFY( ok );

  result = widget.parseUserInput( QStringLiteral( "1200,6/2" ), Qgis::CadConstraintType::Generic, ok );
  QCOMPARE( result, 600.3 );
  QVERIFY( ok );

  // Test UOMs for angles and distance
  QLocale::setDefault( QLocale::English );
  result = widget.parseUserInput( QStringLiteral( "120.123" ), Qgis::CadConstraintType::Angle, ok );
  QCOMPARE( result, 120.123 );
  QVERIFY( ok );

  result = widget.parseUserInput( QStringLiteral( "120.123°" ), Qgis::CadConstraintType::Angle, ok );
  QCOMPARE( result, 120.123 );
  QVERIFY( ok );

  result = widget.parseUserInput( QStringLiteral( "120.123 °" ), Qgis::CadConstraintType::Angle, ok );
  QCOMPARE( result, 120.123 );
  QVERIFY( ok );

  QgsProject::instance()->setDistanceUnits( Qgis::DistanceUnit::NauticalMiles );

  result = widget.parseUserInput( QStringLiteral( "120.123" ), Qgis::CadConstraintType::Distance, ok );
  QCOMPARE( result, 120.123 );
  QVERIFY( ok );

  result = widget.parseUserInput( QStringLiteral( "120.123 NM" ), Qgis::CadConstraintType::Distance, ok );
  QCOMPARE( result, 120.123 );
  QVERIFY( ok );

  result = widget.parseUserInput( QStringLiteral( "120.123NM" ), Qgis::CadConstraintType::Distance, ok );
  QCOMPARE( result, 120.123 );
  QVERIFY( ok );

  // Set a CRS using feet as units
  QgsProject::instance()->setDistanceUnits( Qgis::DistanceUnit::Meters );
  widget.mMapCanvas->mapSettings().setDestinationCrs( QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:3739" ) ) );
  result = widget.parseUserInput( QStringLiteral( "100" ), Qgis::CadConstraintType::Distance, ok );
  QCOMPARE( result, 100.0 * QgsUnitTypes::fromUnitToUnitFactor( Qgis::DistanceUnit::Meters, Qgis::DistanceUnit::FeetUSSurvey ) );
  QVERIFY( ok );
}

QGSTEST_MAIN( TestQgsAdvancedDigitizingDockWidget )
#include "testqgsadvanceddigitizingdockwidget.moc"
