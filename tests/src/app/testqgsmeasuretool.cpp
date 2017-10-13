/***************************************************************************
     testqgsmeasuretool.cpp
     ----------------------
    Date                 : 2016-02-14
    Copyright            : (C) 2016 by Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgstest.h"
#include "qgisapp.h"
#include "qgsapplication.h"
#include "qgsvectorlayer.h"
#include "qgsfeature.h"
#include "qgsgeometry.h"
#include "qgsvectordataprovider.h"
#include "qgsmeasuretool.h"
#include "qgsmeasuredialog.h"
#include "qgsproject.h"
#include "qgsmapcanvas.h"
#include "qgsunittypes.h"

/**
 * \ingroup UnitTests
 * This is a unit test for the measure tool
 */
class TestQgsMeasureTool : public QObject
{
    Q_OBJECT
  public:
    TestQgsMeasureTool();

  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void init() {} // will be called before each testfunction is executed.
    void cleanup() {} // will be called after every testfunction.
    void testLengthCalculation();
    void testLengthCalculationNoCrs();
    void testAreaCalculation();
    void degreeDecimalPlaces();

  private:
    QgisApp *mQgisApp = nullptr;
    QgsMapCanvas *mCanvas = nullptr;
};

TestQgsMeasureTool::TestQgsMeasureTool() = default;

//runs before all tests
void TestQgsMeasureTool::initTestCase()
{
  qDebug() << "TestQgisAppClipboard::initTestCase()";
  // init QGIS's paths - true means that all path will be inited from prefix
  QgsApplication::init();
  QgsApplication::initQgis();

  // Set up the QgsSettings environment
  QCoreApplication::setOrganizationName( QStringLiteral( "QGIS" ) );
  QCoreApplication::setOrganizationDomain( QStringLiteral( "qgis.org" ) );
  QCoreApplication::setApplicationName( QStringLiteral( "QGIS-TEST" ) );

  mQgisApp = new QgisApp();
  mCanvas = new QgsMapCanvas();

  // enforce C locale because the tests expect it
  // (decimal separators / thousand separators)
  QLocale::setDefault( QLocale::c() );
}

//runs after all tests
void TestQgsMeasureTool::cleanupTestCase()
{
  delete mCanvas;
  QgsApplication::exitQgis();
}

void TestQgsMeasureTool::testLengthCalculation()
{
  //test length measurement
  QgsSettings s;
  s.setValue( QStringLiteral( "/qgis/measure/keepbaseunit" ), true );

  // set project CRS and ellipsoid
  QgsCoordinateReferenceSystem srs( 3111, QgsCoordinateReferenceSystem::EpsgCrsId );
  mCanvas->setDestinationCrs( srs );
  QgsProject::instance()->setCrs( srs );
  QgsProject::instance()->setEllipsoid( QStringLiteral( "WGS84" ) );
  QgsProject::instance()->setDistanceUnits( QgsUnitTypes::DistanceMeters );

  // run length calculation
  std::unique_ptr< QgsMeasureTool > tool( new QgsMeasureTool( mCanvas, false ) );
  std::unique_ptr< QgsMeasureDialog > dlg( new QgsMeasureDialog( tool.get() ) );

  tool->restart();
  tool->addPoint( QgsPointXY( 2484588, 2425722 ) );
  tool->addPoint( QgsPointXY( 2482767, 2398853 ) );
  //force dialog recalculation
  dlg->addPoint();

  // check result
  QString measureString = dlg->editTotal->text();
  double measured = measureString.remove( ',' ).split( ' ' ).at( 0 ).toDouble();
  double expected = 26932.156;
  QGSCOMPARENEAR( measured, expected, 0.001 );

  // change project length unit, check calculation respects unit
  QgsProject::instance()->setDistanceUnits( QgsUnitTypes::DistanceFeet );
  std::unique_ptr< QgsMeasureTool > tool2( new QgsMeasureTool( mCanvas, false ) );
  std::unique_ptr< QgsMeasureDialog > dlg2( new QgsMeasureDialog( tool2.get() ) );

  tool2->restart();
  tool2->addPoint( QgsPointXY( 2484588, 2425722 ) );
  tool2->addPoint( QgsPointXY( 2482767, 2398853 ) );
  //force dialog recalculation
  dlg2->addPoint();

  // check result
  measureString = dlg2->editTotal->text();
  measured = measureString.remove( ',' ).split( ' ' ).at( 0 ).toDouble();
  expected = 88360.0918635;
  QGSCOMPARENEAR( measured, expected, 0.001 );

  // check new CoordinateReferenceSystem, points must be reprojected to paint them successfully (issue #15182)
  QgsCoordinateReferenceSystem srs2( 4326, QgsCoordinateReferenceSystem::EpsgCrsId );

  QgsCoordinateTransform ct( srs, srs2 );

  QgsPointXY p0 = ct.transform( tool2->points()[0] );
  QgsPointXY p1 = ct.transform( tool2->points()[1] );

  mCanvas->setDestinationCrs( srs2 );

  QgsPointXY n0 = tool2->points()[0];
  QgsPointXY n1 = tool2->points()[1];

  QGSCOMPARENEAR( p0.x(), n0.x(), 0.001 );
  QGSCOMPARENEAR( p0.y(), n0.y(), 0.001 );
  QGSCOMPARENEAR( p1.x(), n1.x(), 0.001 );
  QGSCOMPARENEAR( p1.y(), n1.y(), 0.001 );
}

void TestQgsMeasureTool::testLengthCalculationNoCrs()
{
  // test length measurement when no projection is set
  QSettings s;
  s.setValue( QStringLiteral( "/qgis/measure/keepbaseunit" ), true );

  // set project CRS and ellipsoid
  mCanvas->setDestinationCrs( QgsCoordinateReferenceSystem() );
  QgsProject::instance()->setCrs( QgsCoordinateReferenceSystem() );

  // run length calculation
  std::unique_ptr< QgsMeasureTool > tool( new QgsMeasureTool( mCanvas, false ) );
  std::unique_ptr< QgsMeasureDialog > dlg( new QgsMeasureDialog( tool.get() ) );

  tool->restart();
  tool->addPoint( QgsPointXY( 2484588, 2425722 ) );
  tool->addPoint( QgsPointXY( 2482767, 2398853 ) );
  //force dialog recalculation
  dlg->addPoint();

  // check result
  QString measureString = dlg->editTotal->text();
  double measured = measureString.remove( ',' ).split( ' ' ).at( 0 ).toDouble();
  double expected = 26930.63686584482;
  QGSCOMPARENEAR( measured, expected, 0.001 );
}

void TestQgsMeasureTool::testAreaCalculation()
{
  //test area measurement
  QgsSettings s;
  s.setValue( QStringLiteral( "/qgis/measure/keepbaseunit" ), true );

  // set project CRS and ellipsoid
  QgsCoordinateReferenceSystem srs( 3111, QgsCoordinateReferenceSystem::EpsgCrsId );
  mCanvas->setDestinationCrs( srs );
  QgsProject::instance()->setCrs( srs );
  QgsProject::instance()->setEllipsoid( QStringLiteral( "WGS84" ) );
  QgsProject::instance()->setAreaUnits( QgsUnitTypes::AreaSquareMeters );

  // run length calculation
  std::unique_ptr< QgsMeasureTool > tool( new QgsMeasureTool( mCanvas, true ) );
  std::unique_ptr< QgsMeasureDialog > dlg( new QgsMeasureDialog( tool.get() ) );

  tool->restart();
  tool->addPoint( QgsPointXY( 2484588, 2425722 ) );
  tool->addPoint( QgsPointXY( 2482767, 2398853 ) );
  tool->addPoint( QgsPointXY( 2520109, 2397715 ) );
  tool->addPoint( QgsPointXY( 2520792, 2425494 ) );
  //force dialog recalculation
  dlg->addPoint();

  // check result
  QString measureString = dlg->editTotal->text();
  double measured = measureString.remove( ',' ).split( ' ' ).at( 0 ).toDouble();
  double expected = 1009089817.0;
  QGSCOMPARENEAR( measured, expected, 1.0 );

  // change project area unit, check calculation respects unit
  QgsProject::instance()->setAreaUnits( QgsUnitTypes::AreaSquareMiles );
  std::unique_ptr< QgsMeasureTool > tool2( new QgsMeasureTool( mCanvas, true ) );
  std::unique_ptr< QgsMeasureDialog > dlg2( new QgsMeasureDialog( tool2.get() ) );

  tool2->restart();
  tool2->addPoint( QgsPointXY( 2484588, 2425722 ) );
  tool2->addPoint( QgsPointXY( 2482767, 2398853 ) );
  tool2->addPoint( QgsPointXY( 2520109, 2397715 ) );
  tool2->addPoint( QgsPointXY( 2520792, 2425494 ) );
  //force dialog recalculation
  dlg2->addPoint();

  // check result
  measureString = dlg2->editTotal->text();
  measured = measureString.remove( ',' ).split( ' ' ).at( 0 ).toDouble();
  expected = 389.6117565069;
  QGSCOMPARENEAR( measured, expected, 0.001 );
}

void TestQgsMeasureTool::degreeDecimalPlaces()
{
  QgsProject::instance()->setDistanceUnits( QgsUnitTypes::DistanceDegrees );

  QgsSettings s;
  s.setValue( QStringLiteral( "qgis/measure/decimalplaces" ), 3 );

  std::unique_ptr< QgsMeasureTool > tool( new QgsMeasureTool( mCanvas, true ) );
  std::unique_ptr< QgsMeasureDialog > dlg( new QgsMeasureDialog( tool.get() ) );

  QCOMPARE( dlg->formatDistance( 11, false ), QString( "11.000 deg" ) );
  QCOMPARE( dlg->formatDistance( 0.005, false ), QString( "0.005 deg" ) );
  QCOMPARE( dlg->formatDistance( 0.002, false ), QString( "0.0020 deg" ) );
  QCOMPARE( dlg->formatDistance( 0.001, false ), QString( "0.0010 deg" ) );
  QCOMPARE( dlg->formatDistance( 0.0001, false ), QString( "0.00010 deg" ) );
  QCOMPARE( dlg->formatDistance( 0.00001, false ), QString( "0.000010 deg" ) );

}

QGSTEST_MAIN( TestQgsMeasureTool )
#include "testqgsmeasuretool.moc"
