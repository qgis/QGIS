/***************************************************************************
    testqgsrubberband.cpp
     --------------------------------------
    Date                 : 28.4.2013
    Copyright            : (C) 2013 Vinayan Parameswaran
    Email                : vinayan123 at gmail dot com
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
#include <QCoreApplication>
#include <QWidget>

#include <qgsapplication.h>
#include <qgsmapcanvas.h>
#include <qgsvectorlayer.h>
#include <qgsrubberband.h>
#include <qgslogger.h>
#include "qgssymbol.h"
#include "qgsrenderchecker.h"
#include "qgslinesymbol.h"
#include "qgsfillsymbol.h"

class TestQgsRubberband : public QgsTest
{
    Q_OBJECT
  public:
    TestQgsRubberband() : QgsTest( QStringLiteral( "Rubberband Tests" ) ) {}

  private slots:
    void initTestCase(); // will be called before the first testfunction is executed.
    void cleanupTestCase(); // will be called after the last testfunction was executed.
    void init(); // will be called before each testfunction is executed.
    void cleanup(); // will be called after every testfunction.

    void testAddSingleMultiGeometries(); //test for #7728
    void pointGeometryAddPoints();
    void pointGeometrySetGeometry();
    void lineGeometryAddPoints();
    void copyPointsFrom();
    void testBoundingRect(); //test for #12392
    void testVisibility(); //test for 12486
    void testClose(); //test closing geometry
    void testLineSymbolRender();
    void testFillSymbolRender();

  private:
    QgsMapCanvas *mCanvas = nullptr;
    QgsVectorLayer *mPolygonLayer = nullptr;
    QString mTestDataDir;
    QgsRubberBand *mRubberband = nullptr;
};

void TestQgsRubberband::initTestCase()
{
  QgsApplication::init();
  QgsApplication::initQgis();
  QgsApplication::showSettings();

  // Setup a map canvas with a vector layer loaded...
  const QString myDataDir( TEST_DATA_DIR ); //defined in CmakeLists.txt
  mTestDataDir = myDataDir + '/';

  //
  // load a vector layer
  //
  const QString myPolygonFileName = mTestDataDir + "polys.shp";
  const QFileInfo myPolygonFileInfo( myPolygonFileName );
  mPolygonLayer = new QgsVectorLayer( myPolygonFileInfo.filePath(),
                                      myPolygonFileInfo.completeBaseName(), QStringLiteral( "ogr" ) );

  mCanvas = new QgsMapCanvas();
  mCanvas->setFrameStyle( QFrame::NoFrame );
  mCanvas->resize( 512, 512 );
  mCanvas->show(); // to make the canvas resize
  mCanvas->hide();

  mRubberband = nullptr;
}

void TestQgsRubberband::cleanupTestCase()
{
  delete mRubberband;
  delete mPolygonLayer;
  delete mCanvas;

  QgsApplication::exitQgis();
}

void TestQgsRubberband::init()
{

}

void TestQgsRubberband::cleanup()
{

}

void TestQgsRubberband::testAddSingleMultiGeometries()
{
  mRubberband = new QgsRubberBand( mCanvas, mPolygonLayer->geometryType() );
  const QgsGeometry geomSinglePart( QgsGeometry::fromWkt( QStringLiteral( "POLYGON((-0.00022418 -0.00000279,-0.0001039 0.00002395,-0.00008677 -0.00005313,-0.00020705 -0.00007987,-0.00022418 -0.00000279))" ) ) );
  const QgsGeometry geomMultiPart( QgsGeometry::fromWkt( QStringLiteral( "MULTIPOLYGON(((-0.00018203 0.00012178,-0.00009444 0.00014125,-0.00007861 0.00007001,-0.00016619 0.00005054,-0.00018203 0.00012178)),((-0.00030957 0.00009464,-0.00021849 0.00011489,-0.00020447 0.00005184,-0.00029555 0.00003158,-0.00030957 0.00009464)))" ) ) );

  mCanvas->setExtent( QgsRectangle( -1e-3, -1e-3, 1e-3, 1e-3 ) ); // otherwise point cannot be converted to canvas coord

  mRubberband->addGeometry( geomSinglePart, mPolygonLayer );
  mRubberband->addGeometry( geomMultiPart, mPolygonLayer );
  QVERIFY( mRubberband->numberOfVertices() == 15 );
}

void TestQgsRubberband::pointGeometryAddPoints()
{
  // point geometry
  std::unique_ptr< QgsMapCanvas > canvas = std::make_unique< QgsMapCanvas >();
  QgsRubberBand r1( canvas.get(), QgsWkbTypes::PointGeometry );
  QVERIFY( r1.asGeometry().isEmpty() );
  r1.addPoint( QgsPointXY( 1, 2 ) );
  QCOMPARE( r1.asGeometry().asWkt(), QStringLiteral( "MultiPoint ((1 2))" ) );
  r1.addPoint( QgsPointXY( 2, 3 ) );
  QCOMPARE( r1.asGeometry().asWkt(), QStringLiteral( "MultiPoint ((1 2),(2 3))" ) );
  r1.addPoint( QgsPointXY( 3, 4 ) );
  QCOMPARE( r1.asGeometry().asWkt(), QStringLiteral( "MultiPoint ((1 2),(2 3),(3 4))" ) );
  r1.reset( QgsWkbTypes::PointGeometry );
  QVERIFY( r1.asGeometry().isEmpty() );
  r1.addPoint( QgsPointXY( 1, 2 ) );
  QCOMPARE( r1.asGeometry().asWkt(), QStringLiteral( "MultiPoint ((1 2))" ) );
}

void TestQgsRubberband::pointGeometrySetGeometry()
{
  // point geometry, set using setToGeometry
  std::unique_ptr< QgsMapCanvas > canvas = std::make_unique< QgsMapCanvas >();
  QgsRubberBand r1( canvas.get(), QgsWkbTypes::PointGeometry );
  QVERIFY( r1.asGeometry().isEmpty() );
  r1.setToGeometry( QgsGeometry::fromPointXY( QgsPointXY( 1, 2 ) ) );
  QCOMPARE( r1.asGeometry().asWkt(), QStringLiteral( "MultiPoint ((1 2))" ) );
  r1.setToGeometry( QgsGeometry::fromPointXY( QgsPointXY( 2, 3 ) ) );
  QCOMPARE( r1.asGeometry().asWkt(), QStringLiteral( "MultiPoint ((2 3))" ) );
  r1.addGeometry( QgsGeometry::fromPointXY( QgsPointXY( 5, 6 ) ) );
  QCOMPARE( r1.asGeometry().asWkt(), QStringLiteral( "MultiPoint ((2 3),(5 6))" ) );
  r1.setToGeometry( QgsGeometry::fromMultiPointXY( {QgsPointXY( 1, 2 ), QgsPointXY( 3, 4 ) } ) );
  QCOMPARE( r1.asGeometry().asWkt(), QStringLiteral( "MultiPoint ((1 2),(3 4))" ) );
  r1.addGeometry( QgsGeometry::fromPointXY( QgsPointXY( 5, 7 ) ) );
  QCOMPARE( r1.asGeometry().asWkt(), QStringLiteral( "MultiPoint ((1 2),(3 4),(5 7))" ) );
  r1.addGeometry( QgsGeometry::fromMultiPointXY( { QgsPointXY( 7, 8 ), QgsPointXY( 9, 10 )} ) );
  QCOMPARE( r1.asGeometry().asWkt(), QStringLiteral( "MultiPoint ((1 2),(3 4),(5 7),(7 8),(9 10))" ) );
  r1.reset( QgsWkbTypes::PointGeometry );
  r1.addGeometry( QgsGeometry::fromMultiPointXY( { QgsPointXY( 7, 8 ), QgsPointXY( 9, 10 )} ) );
  QCOMPARE( r1.asGeometry().asWkt(), QStringLiteral( "MultiPoint ((7 8),(9 10))" ) );
}

void TestQgsRubberband::lineGeometryAddPoints()
{
  std::unique_ptr< QgsMapCanvas > canvas = std::make_unique< QgsMapCanvas >();
  QgsRubberBand r1( canvas.get(), QgsWkbTypes::LineGeometry );
  QVERIFY( r1.asGeometry().isEmpty() );
  r1.addPoint( QgsPointXY( 1, 2 ) );
  QCOMPARE( r1.asGeometry().asWkt(), QStringLiteral( "LineString (1 2, 1 2)" ) );
  r1.addPoint( QgsPointXY( 2, 3 ) );
  QCOMPARE( r1.asGeometry().asWkt(), QStringLiteral( "LineString (1 2, 2 3)" ) );
  r1.addPoint( QgsPointXY( 3, 4 ) );
  QCOMPARE( r1.asGeometry().asWkt(), QStringLiteral( "LineString (1 2, 2 3, 3 4)" ) );
  r1.reset( QgsWkbTypes::LineGeometry );
  QVERIFY( r1.asGeometry().isEmpty() );
  r1.addPoint( QgsPointXY( 1, 2 ) );
  QCOMPARE( r1.asGeometry().asWkt(), QStringLiteral( "LineString (1 2, 1 2)" ) );
}

void TestQgsRubberband::copyPointsFrom()
{
  std::unique_ptr< QgsMapCanvas > canvas = std::make_unique< QgsMapCanvas >();
  QgsRubberBand r1( canvas.get(), QgsWkbTypes::PointGeometry );
  r1.addPoint( QgsPointXY( 1, 2 ) );
  r1.addPoint( QgsPointXY( 3, 4 ) );
  QCOMPARE( r1.asGeometry().asWkt(), QStringLiteral( "MultiPoint ((1 2),(3 4))" ) );

  QgsRubberBand r2( canvas.get(), QgsWkbTypes::LineGeometry );
  r2.copyPointsFrom( &r1 );
  QCOMPARE( r2.asGeometry().asWkt(), QStringLiteral( "MultiPoint ((1 2),(3 4))" ) );

  // line geometry band
  r1.reset( QgsWkbTypes::LineGeometry );
  r1.addPoint( QgsPointXY( 1, 2 ) );
  r1.addPoint( QgsPointXY( 2, 3 ) );
  r1.addPoint( QgsPointXY( 3, 4 ) );
  QCOMPARE( r1.asGeometry().asWkt(), QStringLiteral( "LineString (1 2, 2 3, 3 4)" ) );

  r2.copyPointsFrom( &r1 );
  QCOMPARE( r2.asGeometry().asWkt(), QStringLiteral( "LineString (1 2, 2 3, 3 4)" ) );
}

void TestQgsRubberband::testBoundingRect()
{
  // Set extent to match canvas size.
  // This is to ensure a 1:1 scale
  mCanvas->setExtent( QgsRectangle( 0, 0, 512, 512 ) );
  QCOMPARE( mCanvas->mapUnitsPerPixel(), 1.0 );

  // Polygon extent is 10,10 to 30,30
  const QgsGeometry geom( QgsGeometry::fromWkt(
                            QStringLiteral( "POLYGON((10 10,10 30,30 30,30 10,10 10))" )
                          ) );
  mRubberband = new QgsRubberBand( mCanvas, mPolygonLayer->geometryType() );
  mRubberband->setIconSize( 5 ); // default, but better be explicit
  mRubberband->setWidth( 1 );    // default, but better be explicit
  mRubberband->addGeometry( geom, mPolygonLayer );

  // 20 pixels for the extent + 3 for pen & icon per side + 2 of extra padding from setRect()
  QCOMPARE( mRubberband->boundingRect(), QRectF( QPointF( -1, -1 ), QSizeF( 28, 28 ) ) );
  QCOMPARE( mRubberband->pos(), QPointF(
              // 10 for extent minx - 3 for pen & icon
              10 - 3,
              // 30 for extent maxy - 3 for pen & icon
              512 - 30 - 3
            ) );

  mCanvas->setExtent( QgsRectangle( 0, 0, 256, 256 ) );

  // 40 pixels for the extent + 3 for pen & icon per side + 2 of extra padding from setRect()
  QCOMPARE( mRubberband->boundingRect(), QRectF( QPointF( -1, -1 ), QSizeF( 48, 48 ) ) );
  QCOMPARE( mRubberband->pos(), QPointF(
              // 10 for extent minx - 3 for pen & icon
              10 * 2 - 3,
              // 30 for extent maxy - 3 for pen & icon
              512 - 30 * 2 - 3
            ) );

}

void TestQgsRubberband::testVisibility()
{
  mRubberband = new QgsRubberBand( mCanvas, mPolygonLayer->geometryType() );

  // Visibility is set to false by default
  QCOMPARE( mRubberband->isVisible(), false );

  // Check visibility after setting to empty geometry
  const std::shared_ptr<QgsGeometry> emptyGeom( new QgsGeometry );
  mRubberband->setToGeometry( *emptyGeom.get(), mPolygonLayer );
  QCOMPARE( mRubberband->isVisible(), false );

  // Check that visibility changes
  mRubberband->setVisible( true );
  mRubberband->setToGeometry( *emptyGeom.get(), mPolygonLayer );
  QCOMPARE( mRubberband->isVisible(), false );

  // Check visibility after setting to valid geometry
  const QgsGeometry geom( QgsGeometry::fromWkt(
                            QStringLiteral( "POLYGON((10 10,10 30,30 30,30 10,10 10))" )
                          ) );
  mRubberband->setToGeometry( geom, mPolygonLayer );
  QCOMPARE( mRubberband->isVisible(), true );

  // Add point without update
  mRubberband->reset( QgsWkbTypes::PolygonGeometry );
  mRubberband->addPoint( QgsPointXY( 10, 10 ), false );
  QCOMPARE( mRubberband->isVisible(), false );

  // Add point with update
  mRubberband->addPoint( QgsPointXY( 20, 20 ), true );
  QCOMPARE( mRubberband->isVisible(), true );

  // Check visibility after zoom (should not be changed)
  mRubberband->setVisible( false );
  mCanvas->zoomIn();
  QCOMPARE( mRubberband->isVisible(), false );

}

void TestQgsRubberband::testClose()
{
  QgsRubberBand r( mCanvas, QgsWkbTypes::PolygonGeometry );

  // try closing empty rubber band, don't want to crash
  r.closePoints();
  QCOMPARE( r.partSize( 0 ), 0 );

  r.addPoint( QgsPointXY( 1, 2 ) );
  r.addPoint( QgsPointXY( 1, 3 ) );
  r.addPoint( QgsPointXY( 2, 3 ) );
  QCOMPARE( r.partSize( 0 ), 3 );

  // test with some bad geometry indexes - don't want to crash!
  r.closePoints( true, -1 );
  QCOMPARE( r.partSize( 0 ), 3 );
  r.closePoints( true, 100 );
  QCOMPARE( r.partSize( 0 ), 3 );

  // valid close
  r.closePoints();
  QCOMPARE( r.partSize( 0 ), 4 );

  // close already closed polygon, should be no change
  r.closePoints();
  QCOMPARE( r.partSize( 0 ), 4 );
}

void TestQgsRubberband::testLineSymbolRender()
{
  std::unique_ptr< QgsMapCanvas > canvas = std::make_unique< QgsMapCanvas >();
  canvas->setDestinationCrs( QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:4326" ) ) );
  canvas->setFrameStyle( 0 );
  canvas->resize( 600, 400 );
  canvas->setExtent( QgsRectangle( 10, 30, 20, 35 ) );
  canvas->show();

  QgsRubberBand r( canvas.get(), QgsWkbTypes::LineGeometry );
  r.addGeometry( QgsGeometry::fromWkt( QStringLiteral( "LineString( 12 32, 18 33)" ) ) );

  std::unique_ptr< QgsLineSymbol > lineSymbol( QgsLineSymbol::createSimple(
  {
    { QStringLiteral( "line_color" ), QStringLiteral( "#0000ff" ) },
    { QStringLiteral( "line_width" ), QStringLiteral( "3" )},
    { QStringLiteral( "capstyle" ), QStringLiteral( "round" )}
  } ) );
  r.setSymbol( lineSymbol.release() );

  QPixmap pixmap( canvas->size() );
  QPainter painter( &pixmap );
  canvas->render( &painter );
  painter.end();
  const QString destFile = QDir::tempPath() + QStringLiteral( "/rubberband_line_symbol.png" );
  pixmap.save( destFile );

  QgsRenderChecker checker;
  checker.setControlPathPrefix( QStringLiteral( "rubberband" ) );
  checker.setControlName( QStringLiteral( "expected_line_symbol" ) );
  checker.setRenderedImage( destFile );
  const bool result = checker.compareImages( QStringLiteral( "expected_line_symbol" ) );
  mReport += checker.report();
  QVERIFY( result );
}

void TestQgsRubberband::testFillSymbolRender()
{
  std::unique_ptr< QgsMapCanvas > canvas = std::make_unique< QgsMapCanvas >();
  canvas->setDestinationCrs( QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:4326" ) ) );
  canvas->setFrameStyle( 0 );
  canvas->resize( 600, 400 );
  canvas->setExtent( QgsRectangle( 10, 30, 20, 35 ) );
  canvas->show();

  QgsRubberBand r( canvas.get(), QgsWkbTypes::LineGeometry );
  r.addGeometry( QgsGeometry::fromWkt( QStringLiteral( "Polygon((12 32, 12 35, 18 35, 12 32))" ) ) );

  std::unique_ptr< QgsFillSymbol > fillSymbol( QgsFillSymbol::createSimple(
  {
    { QStringLiteral( "color" ), QStringLiteral( "#ff00ff" ) },
    { QStringLiteral( "line_color" ), QStringLiteral( "#0000ff" ) },
    { QStringLiteral( "line_width" ), QStringLiteral( "3" )},
    { QStringLiteral( "joinstyle" ), QStringLiteral( "round" )}
  } ) );
  r.setSymbol( fillSymbol.release() );

  QPixmap pixmap( canvas->size() );
  QPainter painter( &pixmap );
  canvas->render( &painter );
  painter.end();
  const QString destFile = QDir::tempPath() + QStringLiteral( "/rubberband_fill_symbol.png" );
  pixmap.save( destFile );

  QgsRenderChecker checker;
  checker.setControlPathPrefix( QStringLiteral( "rubberband" ) );
  checker.setControlName( QStringLiteral( "expected_fill_symbol" ) );
  checker.setRenderedImage( destFile );
  const bool result = checker.compareImages( QStringLiteral( "expected_fill_symbol" ) );
  mReport += checker.report();
  QVERIFY( result );
}


QGSTEST_MAIN( TestQgsRubberband )
#include "testqgsrubberband.moc"


