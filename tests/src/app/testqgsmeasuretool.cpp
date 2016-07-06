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
#include <QtTest/QtTest>
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
#include "qgstestutils.h"

/** \ingroup UnitTests
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
    void testAreaCalculation();

  private:
    QgisApp * mQgisApp;
    QgsMapCanvas* mCanvas;
};

TestQgsMeasureTool::TestQgsMeasureTool()
    : mQgisApp( nullptr )
    , mCanvas( nullptr )
{

}

//runs before all tests
void TestQgsMeasureTool::initTestCase()
{
  qDebug() << "TestQgisAppClipboard::initTestCase()";
  // init QGIS's paths - true means that all path will be inited from prefix
  QgsApplication::init();
  QgsApplication::initQgis();

  // Set up the QSettings environment
  QCoreApplication::setOrganizationName( "QGIS" );
  QCoreApplication::setOrganizationDomain( "qgis.org" );
  QCoreApplication::setApplicationName( "QGIS-TEST" );

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
  QSettings s;
  s.setValue( "/qgis/measure/keepbaseunit", true );

  // set project CRS and ellipsoid
  QgisApp::instance()->mapCanvas()->setCrsTransformEnabled( true );
  QgsCoordinateReferenceSystem srs( 3111, QgsCoordinateReferenceSystem::EpsgCrsId );
  mCanvas->setCrsTransformEnabled( true );
  mCanvas->setDestinationCrs( srs );
  QgsProject::instance()->writeEntry( "SpatialRefSys", "/ProjectCRSProj4String", srs.toProj4() );
  QgsProject::instance()->writeEntry( "SpatialRefSys", "/ProjectCRSID", ( int ) srs.srsid() );
  QgsProject::instance()->writeEntry( "SpatialRefSys", "/ProjectCrs", srs.authid() );
  QgsProject::instance()->writeEntry( "Measure", "/Ellipsoid", QString( "WGS84" ) );
  QgsProject::instance()->writeEntry( "Measurement", "/DistanceUnits", QgsUnitTypes::encodeUnit( QGis::Meters ) );

  // run length calculation
  QScopedPointer< QgsMeasureTool > tool( new QgsMeasureTool( mCanvas, false ) );
  QScopedPointer< QgsMeasureDialog > dlg( new QgsMeasureDialog( tool.data() ) );

  tool->restart();
  tool->addPoint( QgsPoint( 2484588, 2425722 ) );
  tool->addPoint( QgsPoint( 2482767, 2398853 ) );
  //force dialog recalculation
  dlg->addPoint( QgsPoint( 0, 0 ) );

  // check result
  QString measureString = dlg->editTotal->text();
  double measured = measureString.remove( ',' ).split( ' ' ).at( 0 ).toDouble();
  double expected = 26932.156;
  QGSCOMPARENEAR( measured, expected, 0.001 );

  // change project length unit, check calculation respects unit
  QgsProject::instance()->writeEntry( "Measurement", "/DistanceUnits", QgsUnitTypes::encodeUnit( QGis::Feet ) );
  QScopedPointer< QgsMeasureTool > tool2( new QgsMeasureTool( mCanvas, false ) );
  QScopedPointer< QgsMeasureDialog > dlg2( new QgsMeasureDialog( tool2.data() ) );

  tool2->restart();
  tool2->addPoint( QgsPoint( 2484588, 2425722 ) );
  tool2->addPoint( QgsPoint( 2482767, 2398853 ) );
  //force dialog recalculation
  dlg2->addPoint( QgsPoint( 0, 0 ) );

  // check result
  measureString = dlg2->editTotal->text();
  measured = measureString.remove( ',' ).split( ' ' ).at( 0 ).toDouble();
  expected = 88360.0918635;
  QGSCOMPARENEAR( measured, expected, 0.001 );

  // check new CoordinateReferenceSystem, points must be reprojected to paint them successfully (issue #15182)
  QgsCoordinateReferenceSystem srs2( 4326, QgsCoordinateReferenceSystem::EpsgCrsId );

  QgsCoordinateTransform ct( srs, srs2 );

  QgsPoint p0 = ct.transform( tool2->points()[0] );
  QgsPoint p1 = ct.transform( tool2->points()[1] );

  mCanvas->setDestinationCrs( srs2 );

  QgsPoint n0 = tool2->points()[0];
  QgsPoint n1 = tool2->points()[1];

  QGSCOMPARENEAR( p0.x(), n0.x(), 0.001 );
  QGSCOMPARENEAR( p0.y(), n0.y(), 0.001 );
  QGSCOMPARENEAR( p1.x(), n1.x(), 0.001 );
  QGSCOMPARENEAR( p1.y(), n1.y(), 0.001 );
}

void TestQgsMeasureTool::testAreaCalculation()
{
  //test area measurement
  QSettings s;
  s.setValue( "/qgis/measure/keepbaseunit", true );

  // set project CRS and ellipsoid
  QgisApp::instance()->mapCanvas()->setCrsTransformEnabled( true );
  QgsCoordinateReferenceSystem srs( 3111, QgsCoordinateReferenceSystem::EpsgCrsId );
  mCanvas->setCrsTransformEnabled( true );
  mCanvas->setDestinationCrs( srs );
  QgsProject::instance()->writeEntry( "SpatialRefSys", "/ProjectCRSProj4String", srs.toProj4() );
  QgsProject::instance()->writeEntry( "SpatialRefSys", "/ProjectCRSID", ( int ) srs.srsid() );
  QgsProject::instance()->writeEntry( "SpatialRefSys", "/ProjectCrs", srs.authid() );
  QgsProject::instance()->writeEntry( "Measure", "/Ellipsoid", QString( "WGS84" ) );
  QgsProject::instance()->writeEntry( "Measurement", "/AreaUnits", QgsUnitTypes::encodeUnit( QgsUnitTypes::SquareMeters ) );

  // run length calculation
  QScopedPointer< QgsMeasureTool > tool( new QgsMeasureTool( mCanvas, true ) );
  QScopedPointer< QgsMeasureDialog > dlg( new QgsMeasureDialog( tool.data() ) );

  tool->restart();
  tool->addPoint( QgsPoint( 2484588, 2425722 ) );
  tool->addPoint( QgsPoint( 2482767, 2398853 ) );
  tool->addPoint( QgsPoint( 2520109, 2397715 ) );
  tool->addPoint( QgsPoint( 2520792, 2425494 ) );
  //force dialog recalculation
  dlg->addPoint( QgsPoint( 0, 0 ) );

  // check result
  QString measureString = dlg->editTotal->text();
  double measured = measureString.remove( ',' ).split( ' ' ).at( 0 ).toDouble();
  double expected = 1009089817.0;
  QGSCOMPARENEAR( measured, expected, 1.0 );

  // change project area unit, check calculation respects unit
  QgsProject::instance()->writeEntry( "Measurement", "/AreaUnits", QgsUnitTypes::encodeUnit( QgsUnitTypes::SquareMiles ) );
  QScopedPointer< QgsMeasureTool > tool2( new QgsMeasureTool( mCanvas, true ) );
  QScopedPointer< QgsMeasureDialog > dlg2( new QgsMeasureDialog( tool2.data() ) );

  tool2->restart();
  tool2->addPoint( QgsPoint( 2484588, 2425722 ) );
  tool2->addPoint( QgsPoint( 2482767, 2398853 ) );
  tool2->addPoint( QgsPoint( 2520109, 2397715 ) );
  tool2->addPoint( QgsPoint( 2520792, 2425494 ) );
  //force dialog recalculation
  dlg2->addPoint( QgsPoint( 0, 0 ) );

  // check result
  measureString = dlg2->editTotal->text();
  measured = measureString.remove( ',' ).split( ' ' ).at( 0 ).toDouble();
  expected = 389.6117565069;
  QGSCOMPARENEAR( measured, expected, 0.001 );
}

QTEST_MAIN( TestQgsMeasureTool )
#include "testqgsmeasuretool.moc"
