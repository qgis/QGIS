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
#include "qgsmeasuretool.h"
#include "qgsmeasuredialog.h"
#include "qgsproject.h"
#include "qgsmapcanvas.h"
#include "qgsrubberband.h"

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
    void initTestCase();    // will be called before the first testfunction is executed.
    void cleanupTestCase(); // will be called after the last testfunction was executed.
    void init() {}          // will be called before each testfunction is executed.
    void cleanup() {}       // will be called after every testfunction.
    void testLengthCalculationCartesian();
    void testLengthCalculationProjected();
    void testLengthCalculationNoCrs();
    void testAreaCalculationCartesian();
    void testAreaCalculationProjected();
    void degreeDecimalPlaces();
    void testToolDesactivationNoExtraPoint();

  private:
    QgisApp *mQgisApp = nullptr;
    QgsMapCanvas *mCanvas = nullptr;
};

TestQgsMeasureTool::TestQgsMeasureTool() = default;

//runs before all tests
void TestQgsMeasureTool::initTestCase()
{
  qDebug() << "TestQgsMeasureTool::initTestCase()";
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

void TestQgsMeasureTool::testLengthCalculationCartesian()
{
  //test length measurement
  QgsSettings s;
  s.setValue( QStringLiteral( "/qgis/measure/keepbaseunit" ), true );

  // set project CRS and ellipsoid
  const QgsCoordinateReferenceSystem srs( QStringLiteral( "EPSG:3111" ) );
  mCanvas->setDestinationCrs( srs );
  QgsProject::instance()->setCrs( srs );
  QgsProject::instance()->setEllipsoid( QStringLiteral( "WGS84" ) );
  QgsProject::instance()->setDistanceUnits( Qgis::DistanceUnit::Meters );

  // run length calculation
  std::unique_ptr<QgsMeasureTool> tool( new QgsMeasureTool( mCanvas, false ) );
  std::unique_ptr<QgsMeasureDialog> dlg( new QgsMeasureDialog( tool.get() ) );

  dlg->mCartesian->setChecked( true );

  tool->restart();
  tool->addPoint( QgsPointXY( 2484588, 2425722 ) );
  tool->addPoint( QgsPointXY( 2482767, 2398853 ) );
  //force dialog recalculation
  dlg->addPoint();

  // check result
  QString measureString = dlg->editTotal->text();
  double measured = measureString.remove( ',' ).split( ' ' ).at( 0 ).toDouble();
  double expected = 26930.637;
  QGSCOMPARENEAR( measured, expected, 0.001 );

  // change project length unit, check calculation respects unit
  QgsProject::instance()->setDistanceUnits( Qgis::DistanceUnit::Feet );
  std::unique_ptr<QgsMeasureTool> tool2( new QgsMeasureTool( mCanvas, false ) );
  std::unique_ptr<QgsMeasureDialog> dlg2( new QgsMeasureDialog( tool2.get() ) );
  dlg2->mCartesian->setChecked( true );

  tool2->restart();
  tool2->addPoint( QgsPointXY( 2484588, 2425722 ) );
  tool2->addPoint( QgsPointXY( 2482767, 2398853 ) );
  //force dialog recalculation
  dlg2->addPoint();

  // check result
  measureString = dlg2->editTotal->text();
  measured = measureString.remove( ',' ).split( ' ' ).at( 0 ).toDouble();
  expected = 88355.108;
  QGSCOMPARENEAR( measured, expected, 0.001 );

  // check new CoordinateReferenceSystem, points must be reprojected to paint them successfully (issue #15182)
  const QgsCoordinateReferenceSystem srs2( QStringLiteral( "EPSG:4326" ) );

  const QgsCoordinateTransform ct( srs, srs2, QgsProject::instance() );

  const QgsPointXY p0 = ct.transform( tool2->points()[0] );
  const QgsPointXY p1 = ct.transform( tool2->points()[1] );

  mCanvas->setDestinationCrs( srs2 );

  const QgsPointXY n0 = tool2->points()[0];
  const QgsPointXY n1 = tool2->points()[1];

  QGSCOMPARENEAR( p0.x(), n0.x(), 0.001 );
  QGSCOMPARENEAR( p0.y(), n0.y(), 0.001 );
  QGSCOMPARENEAR( p1.x(), n1.x(), 0.001 );
  QGSCOMPARENEAR( p1.y(), n1.y(), 0.001 );
}
void TestQgsMeasureTool::testLengthCalculationProjected()
{
  //test length measurement
  QgsSettings s;
  s.setValue( QStringLiteral( "/qgis/measure/keepbaseunit" ), true );

  // set project CRS and ellipsoid
  const QgsCoordinateReferenceSystem srs( QStringLiteral( "EPSG:3111" ) );
  mCanvas->setDestinationCrs( srs );
  QgsProject::instance()->setCrs( srs );
  QgsProject::instance()->setEllipsoid( QStringLiteral( "WGS84" ) );
  QgsProject::instance()->setDistanceUnits( Qgis::DistanceUnit::Meters );

  // run length calculation
  std::unique_ptr<QgsMeasureTool> tool( new QgsMeasureTool( mCanvas, false ) );
  std::unique_ptr<QgsMeasureDialog> dlg( new QgsMeasureDialog( tool.get() ) );
  dlg->mEllipsoidal->setChecked( true );

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
  QgsProject::instance()->setDistanceUnits( Qgis::DistanceUnit::Feet );
  std::unique_ptr<QgsMeasureTool> tool2( new QgsMeasureTool( mCanvas, false ) );
  std::unique_ptr<QgsMeasureDialog> dlg2( new QgsMeasureDialog( tool2.get() ) );
  dlg2->mEllipsoidal->setChecked( true );

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
  const QgsCoordinateReferenceSystem srs2( QStringLiteral( "EPSG:4326" ) );

  const QgsCoordinateTransform ct( srs, srs2, QgsProject::instance() );

  const QgsPointXY p0 = ct.transform( tool2->points()[0] );
  const QgsPointXY p1 = ct.transform( tool2->points()[1] );

  mCanvas->setDestinationCrs( srs2 );

  const QgsPointXY n0 = tool2->points()[0];
  const QgsPointXY n1 = tool2->points()[1];

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
  std::unique_ptr<QgsMeasureTool> tool( new QgsMeasureTool( mCanvas, false ) );
  std::unique_ptr<QgsMeasureDialog> dlg( new QgsMeasureDialog( tool.get() ) );

  tool->restart();
  tool->addPoint( QgsPointXY( 2484588, 2425722 ) );
  tool->addPoint( QgsPointXY( 2482767, 2398853 ) );
  //force dialog recalculation
  dlg->addPoint();

  // check result
  QString measureString = dlg->editTotal->text();
  const double measured = measureString.remove( ',' ).split( ' ' ).at( 0 ).toDouble();
  const double expected = 26930.63686584482;
  QGSCOMPARENEAR( measured, expected, 0.001 );
}

void TestQgsMeasureTool::testAreaCalculationCartesian()
{
  //test area measurement
  QgsSettings s;
  s.setValue( QStringLiteral( "/qgis/measure/keepbaseunit" ), true );

  // set project CRS and ellipsoid
  const QgsCoordinateReferenceSystem srs( QStringLiteral( "EPSG:3111" ) );
  mCanvas->setDestinationCrs( srs );
  QgsProject::instance()->setCrs( srs );
  QgsProject::instance()->setEllipsoid( QStringLiteral( "WGS84" ) );
  QgsProject::instance()->setAreaUnits( Qgis::AreaUnit::SquareMeters );

  // run length calculation
  std::unique_ptr<QgsMeasureTool> tool( new QgsMeasureTool( mCanvas, true ) );
  std::unique_ptr<QgsMeasureDialog> dlg( new QgsMeasureDialog( tool.get() ) );

  dlg->mCartesian->setChecked( true );

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
  double expected = 1005640568.0;
  QGSCOMPARENEAR( measured, expected, 1.0 );

  // change project area unit, check calculation respects unit
  QgsProject::instance()->setAreaUnits( Qgis::AreaUnit::SquareMiles );
  std::unique_ptr<QgsMeasureTool> tool2( new QgsMeasureTool( mCanvas, true ) );
  std::unique_ptr<QgsMeasureDialog> dlg2( new QgsMeasureDialog( tool2.get() ) );
  dlg2->mCartesian->setChecked( true );

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
  expected = 388.280;
  QGSCOMPARENEAR( measured, expected, 0.001 );
}

void TestQgsMeasureTool::testAreaCalculationProjected()
{
  //test area measurement
  QgsSettings s;
  s.setValue( QStringLiteral( "/qgis/measure/keepbaseunit" ), true );

  // set project CRS and ellipsoid
  const QgsCoordinateReferenceSystem srs( QStringLiteral( "EPSG:3111" ) );
  mCanvas->setDestinationCrs( srs );
  QgsProject::instance()->setCrs( srs );
  QgsProject::instance()->setEllipsoid( QStringLiteral( "WGS84" ) );
  QgsProject::instance()->setAreaUnits( Qgis::AreaUnit::SquareMeters );

  // run length calculation
  std::unique_ptr<QgsMeasureTool> tool( new QgsMeasureTool( mCanvas, true ) );
  std::unique_ptr<QgsMeasureDialog> dlg( new QgsMeasureDialog( tool.get() ) );

  dlg->mEllipsoidal->setChecked( true );

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
  double expected = 1005755617.8190000057;
  QGSCOMPARENEAR( measured, expected, 1.0 );

  // change project area unit, check calculation respects unit
  QgsProject::instance()->setAreaUnits( Qgis::AreaUnit::SquareMiles );
  std::unique_ptr<QgsMeasureTool> tool2( new QgsMeasureTool( mCanvas, true ) );
  std::unique_ptr<QgsMeasureDialog> dlg2( new QgsMeasureDialog( tool2.get() ) );

  dlg2->mEllipsoidal->setChecked( true );

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
  expected = 388.3240000000;
  QGSCOMPARENEAR( measured, expected, 0.001 );
}

void TestQgsMeasureTool::degreeDecimalPlaces()
{
  QgsProject::instance()->setDistanceUnits( Qgis::DistanceUnit::Degrees );

  QgsSettings s;
  s.setValue( QStringLiteral( "qgis/measure/decimalplaces" ), 3 );

  const std::unique_ptr<QgsMeasureTool> tool( new QgsMeasureTool( mCanvas, true ) );
  std::unique_ptr<QgsMeasureDialog> dlg( new QgsMeasureDialog( tool.get() ) );

  QCOMPARE( dlg->formatDistance( 11, false ), QString( "11.000 deg" ) );
  QCOMPARE( dlg->formatDistance( 0.005, false ), QString( "0.005 deg" ) );
  QCOMPARE( dlg->formatDistance( 0.002, false ), QString( "0.0020 deg" ) );
  QCOMPARE( dlg->formatDistance( 0.001, false ), QString( "0.0010 deg" ) );
  QCOMPARE( dlg->formatDistance( 0.0001, false ), QString( "0.00010 deg" ) );
  QCOMPARE( dlg->formatDistance( 0.00001, false ), QString( "0.000010 deg" ) );
}

void TestQgsMeasureTool::testToolDesactivationNoExtraPoint()
{
  // set project CRS and ellipsoid
  const QgsCoordinateReferenceSystem srs( QStringLiteral( "EPSG:3111" ) );
  mCanvas->setDestinationCrs( srs );
  QgsProject::instance()->setCrs( srs );
  QgsProject::instance()->setEllipsoid( QStringLiteral( "WGS84" ) );
  QgsProject::instance()->setAreaUnits( Qgis::AreaUnit::SquareMeters );

  // run length calculation
  std::unique_ptr<QgsMeasureTool> tool( new QgsMeasureTool( mCanvas, true ) );
  std::unique_ptr<QgsMeasureDialog> dlg( new QgsMeasureDialog( tool.get() ) );

  dlg->mEllipsoidal->setChecked( true );

  auto moveCanvas = [this]( int x, int y, int delay = 0 ) {
    auto widget = mCanvas->viewport();
    QTest::mouseMove( widget, QPoint( x, y ), delay );
  };

  auto clickCanvas = [this]( int x, int y ) {
    auto widget = mCanvas->viewport();
    QTest::mouseMove( widget, QPoint( x, y ) );
    QTest::mouseClick( mCanvas->viewport(), Qt::LeftButton, Qt::NoModifier, QPoint( x, y ), 50 );
  };

  mCanvas->setMapTool( tool.get() );

  // Click on two points, then move the cursor
  clickCanvas( 50, 50 );
  clickCanvas( 50, 100 );
  moveCanvas( 150, 150, 50 );

  // Check number of vertices in the rubberbands
  // The points Rubberband should have one vertice per click
  QCOMPARE( tool->mRubberBandPoints->numberOfVertices(), 2 );
  // The temp rubberband should have one more
  QCOMPARE( tool->mRubberBand->numberOfVertices(), tool->mRubberBandPoints->numberOfVertices() + 1 );

  // Deactivate & reactivate the tool, then move mouse cursor
  mCanvas->unsetMapTool( tool.get() );
  mCanvas->setMapTool( tool.get() );
  moveCanvas( 100, 100, 50 );

  // Check if both rubberbands still have the right number of vertices
  QCOMPARE( tool->mRubberBandPoints->numberOfVertices(), 2 );
  QCOMPARE( tool->mRubberBand->numberOfVertices(), tool->mRubberBandPoints->numberOfVertices() + 1 );
}

QGSTEST_MAIN( TestQgsMeasureTool )
#include "testqgsmeasuretool.moc"
