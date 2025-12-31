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
#include "qgsadvanceddigitizingdockwidget.h"
#include "qgsapplication.h"
#include "qgsbearingnumericformat.h"
#include "qgsmapcanvas.h"
#include "qgsnumericformat.h"
#include "qgstest.h"

#include <QCoreApplication>
#include <QLocale>
#include <QSignalSpy>
#include <QTest>

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
    void angleAzimuthVisibility();
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

  result = widget.parseUserInput( u"1.2345"_s, Qgis::CadConstraintType::Generic, ok );
  QCOMPARE( result, 1.2345 );
  QVERIFY( ok );

  result = widget.parseUserInput( u"1,234.5"_s, Qgis::CadConstraintType::Generic, ok );
  QCOMPARE( result, 1234.5 );
  QVERIFY( ok );

  result = widget.parseUserInput( u"1,200.6/2"_s, Qgis::CadConstraintType::Generic, ok );
  QCOMPARE( result, 600.3 );
  QVERIFY( ok );

  result = widget.parseUserInput( u"1200.6/2"_s, Qgis::CadConstraintType::Generic, ok );
  QCOMPARE( result, 600.3 );
  QVERIFY( ok );

  // Italian locale (comma as decimal separator)
  QLocale::setDefault( QLocale::Italian );

  result = widget.parseUserInput( u"1,2345"_s, Qgis::CadConstraintType::Generic, ok );
  QCOMPARE( result, 1.2345 );
  QVERIFY( ok );

  result = widget.parseUserInput( u"1.234,5"_s, Qgis::CadConstraintType::Generic, ok );
  QCOMPARE( result, 1234.5 );
  QVERIFY( ok );

  result = widget.parseUserInput( u"1.200,6/2"_s, Qgis::CadConstraintType::Generic, ok );
  QCOMPARE( result, 600.3 );
  QVERIFY( ok );

  result = widget.parseUserInput( u"1200,6/2"_s, Qgis::CadConstraintType::Generic, ok );
  QCOMPARE( result, 600.3 );
  QVERIFY( ok );

  // Test UOMs for angles and distance
  QLocale::setDefault( QLocale::English );
  result = widget.parseUserInput( u"120.123"_s, Qgis::CadConstraintType::Angle, ok );
  QCOMPARE( result, 120.123 );
  QVERIFY( ok );

  result = widget.parseUserInput( u"120.123°"_s, Qgis::CadConstraintType::Angle, ok );
  QCOMPARE( result, 120.123 );
  QVERIFY( ok );

  result = widget.parseUserInput( u"120.123 °"_s, Qgis::CadConstraintType::Angle, ok );
  QCOMPARE( result, 120.123 );
  QVERIFY( ok );

  QgsProject::instance()->setDistanceUnits( Qgis::DistanceUnit::NauticalMiles );

  result = widget.parseUserInput( u"120.123"_s, Qgis::CadConstraintType::Distance, ok );
  QCOMPARE( result, 120.123 );
  QVERIFY( ok );

  result = widget.parseUserInput( u"120.123 NM"_s, Qgis::CadConstraintType::Distance, ok );
  QCOMPARE( result, 120.123 );
  QVERIFY( ok );

  result = widget.parseUserInput( u"120.123NM"_s, Qgis::CadConstraintType::Distance, ok );
  QCOMPARE( result, 120.123 );
  QVERIFY( ok );

  // Set a CRS using feet as units
  QgsProject::instance()->setDistanceUnits( Qgis::DistanceUnit::Meters );
  widget.mMapCanvas->mapSettings().setDestinationCrs( QgsCoordinateReferenceSystem( u"EPSG:3739"_s ) );
  result = widget.parseUserInput( u"100"_s, Qgis::CadConstraintType::Distance, ok );
  QCOMPARE( result, 100.0 * QgsUnitTypes::fromUnitToUnitFactor( Qgis::DistanceUnit::Meters, Qgis::DistanceUnit::FeetUSSurvey ) );
  QVERIFY( ok );
}

void TestQgsAdvancedDigitizingDockWidget::angleAzimuthVisibility()
{
  // Checks that azimuth is visibible even if the angle constraint is locked
  // See: issue GH #61587
  QgsProject::instance()->clear();
  QgsMapCanvas canvas;
  canvas.mapSettings().setDestinationCrs( QgsCoordinateReferenceSystem( u"EPSG:3857"_s ) );
  QgsAdvancedDigitizingDockWidget widget { &canvas };
  widget.setCadEnabled( true );
  widget.setPoints( { QgsPointXY( 0, 0 ), QgsPointXY( 1, 0 ) } );
  widget.mAngleConstraint->setLockMode( QgsAdvancedDigitizingDockWidget::CadConstraint::HardLock );
  widget.mAngleConstraint->setValue( 45 );
  QCOMPARE( widget.mAngleConstraint->lockMode(), QgsAdvancedDigitizingDockWidget::CadConstraint::HardLock );

  QSignalSpy spy( &widget, &QgsAdvancedDigitizingDockWidget::valueBearingChanged );

  // Add one point at 45 degrees angle
  const QgsPoint point( 2, 1 );
  widget.addPoint( point );
  widget.updateUnlockedConstraintValues( point );

  QCOMPARE( spy.count(), 1 );
  QList<QVariant> args = spy.takeFirst();
  const QgsNumericFormatContext context;
  const QString bearingText { QgsProject::instance()->displaySettings()->bearingFormat()->formatDouble( 45, context ) };
  QCOMPARE( args.at( 0 ).toString(), bearingText );
}

QGSTEST_MAIN( TestQgsAdvancedDigitizingDockWidget )
#include "testqgsadvanceddigitizingdockwidget.moc"
