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

class TestQgsRubberband : public QObject
{
    Q_OBJECT
  public:
    TestQgsRubberband() = default;

  private slots:
    void initTestCase(); // will be called before the first testfunction is executed.
    void cleanupTestCase(); // will be called after the last testfunction was executed.
    void init(); // will be called before each testfunction is executed.
    void cleanup(); // will be called after every testfunction.

    void testAddSingleMultiGeometries(); //test for #7728
    void testBoundingRect(); //test for #12392
    void testVisibility(); //test for 12486
    void testClose(); //test closing geometry

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
  QString myDataDir( TEST_DATA_DIR ); //defined in CmakeLists.txt
  mTestDataDir = myDataDir + '/';

  //
  // load a vector layer
  //
  QString myPolygonFileName = mTestDataDir + "polys.shp";
  QFileInfo myPolygonFileInfo( myPolygonFileName );
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
  QgsGeometry geomSinglePart( QgsGeometry::fromWkt( QStringLiteral( "POLYGON((-0.00022418 -0.00000279,-0.0001039 0.00002395,-0.00008677 -0.00005313,-0.00020705 -0.00007987,-0.00022418 -0.00000279))" ) ) );
  QgsGeometry geomMultiPart( QgsGeometry::fromWkt( QStringLiteral( "MULTIPOLYGON(((-0.00018203 0.00012178,-0.00009444 0.00014125,-0.00007861 0.00007001,-0.00016619 0.00005054,-0.00018203 0.00012178)),((-0.00030957 0.00009464,-0.00021849 0.00011489,-0.00020447 0.00005184,-0.00029555 0.00003158,-0.00030957 0.00009464)))" ) ) );

  mCanvas->setExtent( QgsRectangle( -1e-3, -1e-3, 1e-3, 1e-3 ) ); // otherwise point cannot be converted to canvas coord

  mRubberband->addGeometry( geomSinglePart, mPolygonLayer );
  mRubberband->addGeometry( geomMultiPart, mPolygonLayer );
  QVERIFY( mRubberband->numberOfVertices() == 15 );
}


void TestQgsRubberband::testBoundingRect()
{
  // Set extent to match canvas size.
  // This is to ensure a 1:1 scale
  mCanvas->setExtent( QgsRectangle( 0, 0, 512, 512 ) );
  QCOMPARE( mCanvas->mapUnitsPerPixel(), 1.0 );

  // Polygon extent is 10,10 to 30,30
  QgsGeometry geom( QgsGeometry::fromWkt(
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
  std::shared_ptr<QgsGeometry> emptyGeom( new QgsGeometry );
  mRubberband->setToGeometry( *emptyGeom.get(), mPolygonLayer );
  QCOMPARE( mRubberband->isVisible(), false );

  // Check that visibility changes
  mRubberband->setVisible( true );
  mRubberband->setToGeometry( *emptyGeom.get(), mPolygonLayer );
  QCOMPARE( mRubberband->isVisible(), false );

  // Check visibility after setting to valid geometry
  QgsGeometry geom( QgsGeometry::fromWkt(
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


QGSTEST_MAIN( TestQgsRubberband )
#include "testqgsrubberband.moc"


