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


#include <QtTest/QtTest>
#include <QObject>
#include <QString>
#include <QSharedPointer>
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
    TestQgsRubberband()
        : mCanvas( 0 )
        , mPolygonLayer( 0 )
        , mRubberband( 0 )
    {}

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
    QgsMapCanvas* mCanvas;
    QgsVectorLayer* mPolygonLayer;
    QString mTestDataDir;
    QgsRubberBand* mRubberband;
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
                                      myPolygonFileInfo.completeBaseName(), "ogr" );

  mCanvas = new QgsMapCanvas();
  mRubberband = 0;
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
  QSharedPointer<QgsGeometry> geomSinglePart( QgsGeometry::fromWkt( "POLYGON((-0.00022418 -0.00000279,-0.0001039 0.00002395,-0.00008677 -0.00005313,-0.00020705 -0.00007987,-0.00022418 -0.00000279))" ) );
  QSharedPointer<QgsGeometry> geomMultiPart( QgsGeometry::fromWkt( "MULTIPOLYGON(((-0.00018203 0.00012178,-0.00009444 0.00014125,-0.00007861 0.00007001,-0.00016619 0.00005054,-0.00018203 0.00012178)),((-0.00030957 0.00009464,-0.00021849 0.00011489,-0.00020447 0.00005184,-0.00029555 0.00003158,-0.00030957 0.00009464)))" ) );

  mCanvas->setExtent( QgsRectangle( -1e-3, -1e-3, 1e-3, 1e-3 ) ); // otherwise point cannot be converted to canvas coord

  mRubberband->addGeometry( geomSinglePart.data(), mPolygonLayer );
  mRubberband->addGeometry( geomMultiPart.data(), mPolygonLayer );
  QVERIFY( mRubberband->numberOfVertices() == 15 );
}


void TestQgsRubberband::testBoundingRect()
{
  QSizeF mapSize = mCanvas->mapSettings().outputSize();

  // Set extent to match canvas size.
  // This is to ensure a 1:1 scale
  mCanvas->setExtent( QgsRectangle( QRectF(
                                      QPointF( 0, 0 ), mapSize
                                    ) ) );
  QCOMPARE( mCanvas->mapUnitsPerPixel(), 1.0 );

  // Polygon extent is 10,10 to 30,30
  QSharedPointer<QgsGeometry> geom( QgsGeometry::fromWkt(
                                      "POLYGON((10 10,10 30,30 30,30 10,10 10))"
                                    ) );
  mRubberband = new QgsRubberBand( mCanvas, mPolygonLayer->geometryType() );
  mRubberband->setIconSize( 5 ); // default, but better be explicit
  mRubberband->setWidth( 1 );    // default, but better be explicit
  mRubberband->addGeometry( geom.data(), mPolygonLayer );

  // 20 pixels for the extent + 3 for pen & icon per side + 2 of padding
  QCOMPARE( mRubberband->boundingRect(), QRectF( QPointF( -1, -1 ), QSizeF( 28, 28 ) ) );
  QCOMPARE( mRubberband->pos(), QPointF(
              // 10 for extent minx - 3 for pen & icon
              7,
              // 30 for extent maxy - 3 for pen & icon
              mapSize.height() - 30 - 3
            ) );

  mCanvas->setExtent( QgsRectangle( QRectF(
                                      QPointF( 0, 0 ), mapSize / 2
                                    ) ) );

  // 40 pixels for the extent + 6 for pen & icon per side + 2 of padding
  QCOMPARE( mRubberband->boundingRect(), QRectF( QPointF( -1, -1 ), QSizeF( 54, 54 ) ) );
  QCOMPARE( mRubberband->pos(), QPointF(
              // 10 for extent minx - 3 for pen & icon
              7 * 2,
              // 30 for extent maxy - 3 for pen & icon
              mapSize.height() - ( 30 + 3 ) * 2
            ) );

}

void TestQgsRubberband::testVisibility()
{
  mRubberband = new QgsRubberBand( mCanvas, mPolygonLayer->geometryType() );

  // Visibility is set to false by default
  QCOMPARE( mRubberband->isVisible(), false );

  // Check visibility after setting to empty geometry
  QSharedPointer<QgsGeometry> emptyGeom( new QgsGeometry );
  mRubberband->setToGeometry( emptyGeom.data(), mPolygonLayer );
  QCOMPARE( mRubberband->isVisible(), false );

  // Check that visibility changes
  mRubberband->setVisible( true );
  mRubberband->setToGeometry( emptyGeom.data(), mPolygonLayer );
  QCOMPARE( mRubberband->isVisible(), false );

  // Check visibility after setting to valid geometry
  QSharedPointer<QgsGeometry> geom( QgsGeometry::fromWkt(
                                      "POLYGON((10 10,10 30,30 30,30 10,10 10))"
                                    ) );
  mRubberband->setToGeometry( geom.data(), mPolygonLayer );
  QCOMPARE( mRubberband->isVisible(), true );

  // Add point without update
  mRubberband->reset( QGis::Polygon );
  mRubberband->addPoint( QgsPoint( 10, 10 ), false );
  QCOMPARE( mRubberband->isVisible(), false );

  // Add point with update
  mRubberband->addPoint( QgsPoint( 20, 20 ), true );
  QCOMPARE( mRubberband->isVisible(), true );

  // Check visibility after zoom (should not be changed)
  mRubberband->setVisible( false );
  mCanvas->zoomIn();
  QCOMPARE( mRubberband->isVisible(), false );

}

void TestQgsRubberband::testClose()
{
  QgsRubberBand r( mCanvas, QGis::Polygon );

  // try closing empty rubber band, don't want to crash
  r.closePoints();
  QCOMPARE( r.partSize( 0 ), 0 );

  r.addPoint( QgsPoint( 1, 2 ) );
  r.addPoint( QgsPoint( 1, 3 ) );
  r.addPoint( QgsPoint( 2, 3 ) );
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


QTEST_MAIN( TestQgsRubberband )
#include "testqgsrubberband.moc"


