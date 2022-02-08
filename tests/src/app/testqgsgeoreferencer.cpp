/***************************************************************************
     testqgsgeoreferencer.cpp
     --------------------------
    Date                 : 2022-02-02
    Copyright            : (C) 2022 by Nyall Dawson
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
#include "qgsfeatureiterator.h"
#include "qgsgeometry.h"
#include "qgsvectordataprovider.h"
#include "qgsfieldcalculator.h"
#include "qgsproject.h"
#include "qgsmapcanvas.h"
#include "georeferencer/qgsgeoreftransform.h"
#include "georeferencer/qgsgeorefdatapoint.h"
#include "georeferencer/qgsgcplist.h"

/**
 * \ingroup UnitTests
 * This is a unit test for georeferencer
 */
class TestQgsGeoreferencer : public QObject
{
    Q_OBJECT
  public:
    TestQgsGeoreferencer();

  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void init() {} // will be called before each testfunction is executed.
    void cleanup() {} // will be called after every testfunction.
    void testGcpPoint();
    void testGeorefDataPoint();
    void testGcpList();
    void testSaveLoadGcps();
    void testTransformImageNoGeoference();
    void testTransformImageWithExistingGeoreference();
    void testRasterChangeCoords();

  private:
    QgisApp *mQgisApp = nullptr;
};

TestQgsGeoreferencer::TestQgsGeoreferencer() = default;

//runs before all tests
void TestQgsGeoreferencer::initTestCase()
{
  qDebug() << "TestQgisAppClipboard::initTestCase()";
  // init QGIS's paths - true means that all path will be inited from prefix
  QgsApplication::init();
  QgsApplication::initQgis();
  mQgisApp = new QgisApp();
}

//runs after all tests
void TestQgsGeoreferencer::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestQgsGeoreferencer::testGcpPoint()
{
  QgsGcpPoint p( QgsPointXY( 1, 2 ), QgsPointXY( 3, 4 ), QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:3111" ) ), false );

  QCOMPARE( p.sourcePoint(), QgsPointXY( 1, 2 ) );
  p.setSourcePoint( QgsPointXY( 11, 22 ) );
  QCOMPARE( p.sourcePoint(), QgsPointXY( 11, 22 ) );

  QCOMPARE( p.destinationPoint(), QgsPointXY( 3, 4 ) );
  p.setDestinationPoint( QgsPointXY( 33, 44 ) );
  QCOMPARE( p.destinationPoint(), QgsPointXY( 33, 44 ) );

  QCOMPARE( p.destinationPointCrs().authid(), QStringLiteral( "EPSG:3111" ) );
  p.setDestinationPointCrs( QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:28356" ) ) );
  QCOMPARE( p.destinationPointCrs().authid(), QStringLiteral( "EPSG:28356" ) );

  QVERIFY( !p.isEnabled() );
  p.setEnabled( true );
  QVERIFY( p.isEnabled() );

  // equality operator
  QVERIFY( QgsGcpPoint( QgsPointXY( 1, 2 ), QgsPointXY( 3, 4 ), QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:3111" ) ), false )
           == QgsGcpPoint( QgsPointXY( 1, 2 ), QgsPointXY( 3, 4 ), QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:3111" ) ), false ) );
  QVERIFY( QgsGcpPoint( QgsPointXY( 1, 2 ), QgsPointXY( 3, 4 ), QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:3111" ) ), false )
           != QgsGcpPoint( QgsPointXY( 11, 22 ), QgsPointXY( 3, 4 ), QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:3111" ) ), false ) );
  QVERIFY( QgsGcpPoint( QgsPointXY( 1, 2 ), QgsPointXY( 3, 4 ), QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:3111" ) ), false )
           != QgsGcpPoint( QgsPointXY( 1, 2 ), QgsPointXY( 33, 44 ), QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:3111" ) ), false ) );
  QVERIFY( QgsGcpPoint( QgsPointXY( 1, 2 ), QgsPointXY( 3, 4 ), QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:3111" ) ), false )
           != QgsGcpPoint( QgsPointXY( 1, 2 ), QgsPointXY( 3, 4 ), QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:28356" ) ), false ) );
  QVERIFY( QgsGcpPoint( QgsPointXY( 1, 2 ), QgsPointXY( 3, 4 ), QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:3111" ) ), false )
           != QgsGcpPoint( QgsPointXY( 1, 2 ), QgsPointXY( 3, 4 ), QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:3111" ) ), true ) );


  // transform destination point
  QgsGcpPoint p2( QgsPointXY( 1, 2 ), QgsPointXY( 150, -30 ), QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:4326" ) ), false );
  const QgsPointXY res = p2.transformedDestinationPoint( QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:3857" ) ), QgsProject::instance()->transformContext() );
  QGSCOMPARENEAR( res.x(), 16697923, 10000 );
  QGSCOMPARENEAR( res.y(), -3503549, 10000 );
}

void TestQgsGeoreferencer::testGeorefDataPoint()
{
  QgsMapCanvas c1;
  QgsMapCanvas c2;
  QgsGeorefDataPoint p( &c1, &c2,
                        QgsPointXY( 1, 2 ), QgsPointXY( 3, 4 ), QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:3111" ) ),
                        false );

  QCOMPARE( p.sourcePoint(), QgsPointXY( 1, 2 ) );
  p.setSourcePoint( QgsPointXY( 11, 22 ) );
  QCOMPARE( p.sourcePoint(), QgsPointXY( 11, 22 ) );

  QCOMPARE( p.destinationPoint(), QgsPointXY( 3, 4 ) );
  p.setDestinationPoint( QgsPointXY( 33, 44 ) );
  QCOMPARE( p.destinationPoint(), QgsPointXY( 33, 44 ) );

  QCOMPARE( p.destinationPointCrs().authid(), QStringLiteral( "EPSG:3111" ) );

  QVERIFY( !p.isEnabled() );
  p.setEnabled( true );
  QVERIFY( p.isEnabled() );

  QCOMPARE( p.point(), QgsGcpPoint( QgsPointXY( 11, 22 ),
                                    QgsPointXY( 33, 44 ),
                                    QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:3111" ) ),
                                    true ) );


  // transform destination point
  QgsGeorefDataPoint p2( &c1, &c2, QgsPointXY( 1, 2 ), QgsPointXY( 150, -30 ), QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:4326" ) ), false );
  const QgsPointXY res = p2.transformedDestinationPoint( QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:3857" ) ), QgsProject::instance()->transformContext() );
  QGSCOMPARENEAR( res.x(), 16697923, 10000 );
  QGSCOMPARENEAR( res.y(), -3503549, 10000 );
}

void TestQgsGeoreferencer::testGcpList()
{
  QgsGCPList list;
  QCOMPARE( list.countEnabledPoints(), 0 );
  QVERIFY( list.asPoints().isEmpty() );

  QgsMapCanvas c1;
  QgsMapCanvas c2;
  list.append( new QgsGeorefDataPoint( &c1, &c2,
                                       QgsPointXY( 1, 2 ), QgsPointXY( 3, 4 ), QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:3111" ) ),
                                       false ) );
  QCOMPARE( list.countEnabledPoints(), 0 );
  QCOMPARE( list.asPoints(),
  {
    QgsGcpPoint( QgsPointXY( 1, 2 ), QgsPointXY( 3, 4 ), QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:3111" ) ), false )
  }
          );

  list.append( new QgsGeorefDataPoint( &c1, &c2,
                                       QgsPointXY( 11, 22 ), QgsPointXY( 33, 44 ), QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:28356" ) ),
                                       true ) );
  QCOMPARE( list.countEnabledPoints(), 1 );
  QCOMPARE( list.asPoints(),
            QList< QgsGcpPoint >(
  {
    QgsGcpPoint( QgsPointXY( 1, 2 ), QgsPointXY( 3, 4 ), QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:3111" ) ), false ),
    QgsGcpPoint( QgsPointXY( 11, 22 ), QgsPointXY( 33, 44 ), QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:28356" ) ), true )
  } ) );

  list.append( new QgsGeorefDataPoint( &c1, &c2,
                                       QgsPointXY( 111, 222 ), QgsPointXY( 333, 444 ), QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:28356" ) ),
                                       true ) );
  QCOMPARE( list.countEnabledPoints(), 2 );
  QCOMPARE( list.asPoints(),
            QList< QgsGcpPoint >(
  {
    QgsGcpPoint( QgsPointXY( 1, 2 ), QgsPointXY( 3, 4 ), QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:3111" ) ), false ),
    QgsGcpPoint( QgsPointXY( 11, 22 ), QgsPointXY( 33, 44 ), QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:28356" ) ), true ),
    QgsGcpPoint( QgsPointXY( 111, 222 ), QgsPointXY( 333, 444 ), QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:28356" ) ), true )
  } ) );


  qDeleteAll( list );
  list.clear();

  // create gcp vectors
  list.append( new QgsGeorefDataPoint( &c1, &c2,
                                       QgsPointXY( 111, 222 ), QgsPointXY( -30, 40 ), QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:4326" ) ),
                                       true ) );
  list.append( new QgsGeorefDataPoint( &c1, &c2,
                                       QgsPointXY( 11, 22 ), QgsPointXY( 16697923, -3503549 ), QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:3857" ) ),
                                       true ) );
  // disabled!
  list.append( new QgsGeorefDataPoint( &c1, &c2,
                                       QgsPointXY( 33, 44 ), QgsPointXY( 100, 200 ), QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:3857" ) ),
                                       false ) );

  QVector< QgsPointXY > sourcePoints;
  QVector< QgsPointXY > destinationPoints;
  list.createGCPVectors( sourcePoints, destinationPoints, QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:3857" ) ), QgsProject::instance()->transformContext() );
  QCOMPARE( sourcePoints.size(), 2 );
  QCOMPARE( sourcePoints.at( 0 ).x(), 111 );
  QCOMPARE( sourcePoints.at( 0 ).y(), 222 );
  QCOMPARE( sourcePoints.at( 1 ).x(), 11 );
  QCOMPARE( sourcePoints.at( 1 ).y(), 22 );

  QCOMPARE( destinationPoints.size(), 2 );
  QGSCOMPARENEAR( destinationPoints.at( 0 ).x(), -3339584, 10000 );
  QGSCOMPARENEAR( destinationPoints.at( 0 ).y(), 4865942, 10000 );
  QCOMPARE( destinationPoints.at( 1 ).x(), 16697923 );
  QCOMPARE( destinationPoints.at( 1 ).y(), -3503549 );

}

void TestQgsGeoreferencer::testSaveLoadGcps()
{
  // test saving and loading GCPs
  QgsGCPList list;
  QgsMapCanvas c1;
  QgsMapCanvas c2;
  list.append( new QgsGeorefDataPoint( &c1, &c2,
                                       QgsPointXY( 111, 222 ), QgsPointXY( -30, 40 ), QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:4326" ) ),
                                       true ) );
  list.append( new QgsGeorefDataPoint( &c1, &c2,
                                       QgsPointXY( 11, 22 ), QgsPointXY( 16697923, -3503549 ), QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:3857" ) ),
                                       true ) );
  // disabled!
  list.append( new QgsGeorefDataPoint( &c1, &c2,
                                       QgsPointXY( 33, 44 ), QgsPointXY( 100, 200 ), QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:3857" ) ),
                                       false ) );

  QTemporaryDir dir;
  QVERIFY( dir.isValid() );
  const QString tempFilename = dir.filePath( QStringLiteral( "test.points" ) );

  QString error;

  QVERIFY( list.saveGcps( tempFilename, QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:3857" ) ),
                          QgsProject::instance()->transformContext(), error ) );
  QVERIFY( error.isEmpty() );


  // load
  QList<QgsGcpPoint> res = QgsGCPList::loadGcps( QStringLiteral( "not real" ),
                           QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:3111" ) ),
                           error );
  QVERIFY( !error.isEmpty() );

  res = QgsGCPList::loadGcps( tempFilename,
                              QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:3111" ) ),
                              error );
  QVERIFY( error.isEmpty() );
  QCOMPARE( res.size(), 3 );

  QCOMPARE( res.at( 0 ).sourcePoint().x(), 111 );
  QCOMPARE( res.at( 0 ).sourcePoint().y(), 222 );
  QGSCOMPARENEAR( res.at( 0 ).destinationPoint().x(), -3339584, 10000 );
  QGSCOMPARENEAR( res.at( 0 ).destinationPoint().y(), 4865942, 10000 );
  QVERIFY( res.at( 0 ).isEnabled() );
  QCOMPARE( res.at( 0 ).destinationPointCrs().authid(), QStringLiteral( "EPSG:3857" ) );

  QCOMPARE( res.at( 1 ).sourcePoint().x(), 11 );
  QCOMPARE( res.at( 1 ).sourcePoint().y(), 22 );
  QCOMPARE( res.at( 1 ).destinationPoint().x(), 16697923 );
  QCOMPARE( res.at( 1 ).destinationPoint().y(), -3503549 );
  QVERIFY( res.at( 1 ).isEnabled() );
  QCOMPARE( res.at( 1 ).destinationPointCrs().authid(), QStringLiteral( "EPSG:3857" ) );

  QCOMPARE( res.at( 2 ).sourcePoint().x(), 33 );
  QCOMPARE( res.at( 2 ).sourcePoint().y(), 44 );
  QCOMPARE( res.at( 2 ).destinationPoint().x(), 100 );
  QCOMPARE( res.at( 2 ).destinationPoint().y(), 200 );
  QVERIFY( !res.at( 2 ).isEnabled() );
  QCOMPARE( res.at( 2 ).destinationPointCrs().authid(), QStringLiteral( "EPSG:3857" ) );
}

void TestQgsGeoreferencer::testTransformImageNoGeoference()
{
  QgsGeorefTransform transform( QgsGcpTransformerInterface::TransformMethod::Linear );
  // this image has no georeferencing set
  transform.loadRaster( QStringLiteral( TEST_DATA_DIR ) + QStringLiteral( "/rgb256x256.png" ) );

  QVERIFY( !transform.hasExistingGeoreference() );

  QgsPointXY res;
  // should be treating source coordinates and source pixels as identical
  res = transform.toSourceCoordinate( QgsPointXY( 0, 0 ) );
  QCOMPARE( res.x(), 0.0 );
  QCOMPARE( res.y(), 0.0 );
  res = transform.toSourceCoordinate( QgsPointXY( 100, 200 ) );
  QCOMPARE( res.x(), 100.0 );
  QCOMPARE( res.y(), 200.0 );

  res = transform.toSourcePixel( QgsPointXY( 0, 0 ) );
  QCOMPARE( res.x(), 0.0 );
  QCOMPARE( res.y(), 0.0 );
  res = transform.toSourcePixel( QgsPointXY( 100, 200 ) );
  QCOMPARE( res.x(), 100.0 );
  QCOMPARE( res.y(), 200.0 );

  QgsRectangle rect = transform.transformSourceExtent( QgsRectangle( 0, 0, 100, 200 ), true );
  QCOMPARE( rect.xMinimum(), 0.0 );
  QCOMPARE( rect.yMinimum(), 0.0 );
  QCOMPARE( rect.xMaximum(), 100.0 );
  QCOMPARE( rect.yMaximum(), 200.0 );
  rect = transform.transformSourceExtent( QgsRectangle( 0, 0, 100, 200 ), false );
  QCOMPARE( rect.xMinimum(), 0.0 );
  QCOMPARE( rect.yMinimum(), 0.0 );
  QCOMPARE( rect.xMaximum(), 100.0 );
  QCOMPARE( rect.yMaximum(), 200.0 );

  QVERIFY( transform.updateParametersFromGcps( {QgsPointXY( 0, 0 ), QgsPointXY( 10, 0 ), QgsPointXY( 0, 30 ), QgsPointXY( 10, 30 )},
  {QgsPointXY( 10, 5 ), QgsPointXY( 16, 5 ), QgsPointXY( 10, 8 ), QgsPointXY( 16, 8 )}, true ) );

  QVERIFY( transform.transform( QgsPointXY( 0, 5 ), res, true ) );
  QCOMPARE( res.x(), 10 );
  QCOMPARE( res.y(), 5.5 );
  QVERIFY( transform.transform( QgsPointXY( 9, 25 ), res, true ) );
  QCOMPARE( res.x(), 15.4 );
  QCOMPARE( res.y(), 7.5 );
  // reverse transform
  QVERIFY( transform.transform( QgsPointXY( 10, 5.5 ), res, false ) );
  QCOMPARE( res.x(), 0.0 );
  QCOMPARE( res.y(), 5.0 );
  QVERIFY( transform.transform( QgsPointXY( 15.4, 7.5 ), res, false ) );
  QCOMPARE( res.x(), 9.0 );
  QCOMPARE( res.y(), 25.0 );
}

void TestQgsGeoreferencer::testTransformImageWithExistingGeoreference()
{
  // load an image which is already georeferenced
  QgsGeorefTransform transform( QgsGcpTransformerInterface::TransformMethod::Linear );
  transform.loadRaster( QStringLiteral( TEST_DATA_DIR ) + QStringLiteral( "/landsat.tif" ) );

  QVERIFY( transform.mRasterChangeCoords.mHasExistingGeoreference );
  QGSCOMPARENEAR( transform.mRasterChangeCoords.mResX, 57, 0.00001 );
  QGSCOMPARENEAR( transform.mRasterChangeCoords.mResY, -57, 0.00001 );
  QGSCOMPARENEAR( transform.mRasterChangeCoords.mUL_X, 781662.375, 0.01 );
  QGSCOMPARENEAR( transform.mRasterChangeCoords.mUL_Y, 3350923.125, 0.01 );

  QgsPointXY res;
  res = transform.toSourceCoordinate( QgsPointXY( 0, 0 ) );
  QGSCOMPARENEAR( res.x(), 781662.375, 0.1 );
  QGSCOMPARENEAR( res.y(), 3350923.125, 0.1 );
  res = transform.toSourceCoordinate( QgsPointXY( 100, 200 ) );
  QGSCOMPARENEAR( res.x(), 787362.375, 0.1 );
  QGSCOMPARENEAR( res.y(), 3362323.125, 0.1 );

  res = transform.toSourcePixel( QgsPointXY( 781662.375, 3350923.125 ) );
  QGSCOMPARENEAR( res.x(), 0.0, 0.1 );
  QGSCOMPARENEAR( res.y(), 0.0, 0.1 );
  res = transform.toSourcePixel( QgsPointXY( 787362.375, 3362323.125 ) );
  QGSCOMPARENEAR( res.x(), 100.0, 0.1 );
  QGSCOMPARENEAR( res.y(), 200.0, 0.1 );

  res = transform.mRasterChangeCoords.toColumnLine( QgsPointXY( 783414, 3350122 ) );
  QGSCOMPARENEAR( res.x(), 30.7302631579, 0.01 );
  QGSCOMPARENEAR( res.y(), -14.0548245614, 0.01 );
  res = transform.mRasterChangeCoords.toXY( QgsPointXY( 30.7302631579, -14.0548245614 ) );
  QGSCOMPARENEAR( res.x(), 783414, 10 );
  QGSCOMPARENEAR( res.y(), 3350122, 10 );

  QgsRectangle rect = transform.transformSourceExtent( QgsRectangle( 781662.375, 3350923.125, 787362.375, 3362323.125 ), true );
  QGSCOMPARENEAR( rect.xMinimum(), 0.0, 0.1 );
  QGSCOMPARENEAR( rect.yMinimum(), 0.0, 0.1 );
  QGSCOMPARENEAR( rect.xMaximum(), 100.0, 0.1 );
  QGSCOMPARENEAR( rect.yMaximum(), 200.0, 0.1 );
  rect = transform.transformSourceExtent( QgsRectangle( 0, 0, 100, 200 ), false );
  QGSCOMPARENEAR( rect.xMinimum(), 781662.375, 0.1 );
  QGSCOMPARENEAR( rect.yMinimum(), 3350923.125, 0.1 );
  QGSCOMPARENEAR( rect.xMaximum(), 787362.375, 0.1 );
  QGSCOMPARENEAR( rect.yMaximum(),  3362323.125, 0.1 );

  QVector<QgsPointXY> pixelCoords = transform.mRasterChangeCoords.getPixelCoords(
  {
    QgsPointXY( 783414, 3350122 ),
    QgsPointXY( 791344, 3349795 ),
    QgsPointXY( 783077, 3340937 ),
    QgsPointXY( 791134, 3341401 )
  } ) ;

  QCOMPARE( pixelCoords.size(), 4 );
  QGSCOMPARENEAR( pixelCoords.at( 0 ).x(), 30.7302631579, 0.01 );
  QGSCOMPARENEAR( pixelCoords.at( 0 ).y(), -14.0548245614, 0.01 );
  QGSCOMPARENEAR( pixelCoords.at( 1 ).x(), 169.8530701754, 0.01 );
  QGSCOMPARENEAR( pixelCoords.at( 1 ).y(), -19.7916666667, 0.01 );
  QGSCOMPARENEAR( pixelCoords.at( 2 ).x(), 24.8179824561, 0.01 );
  QGSCOMPARENEAR( pixelCoords.at( 2 ).y(), -175.1951754386, 0.01 );
  QGSCOMPARENEAR( pixelCoords.at( 3 ).x(), 166.168859649, 0.01 );
  QGSCOMPARENEAR( pixelCoords.at( 3 ).y(), -167.0548245614, 0.01 );

  QVERIFY( transform.hasExistingGeoreference() );

  // when calling updateParametersFromGcps the source list MUST be in source layer CRS, not pixels!
  QVERIFY( transform.updateParametersFromGcps( {QgsPointXY( 783414, 3350122 ), QgsPointXY( 791344, 3349795 ), QgsPointXY( 783077, 334093 ), QgsPointXY( 791134, 3341401 )},
  {QgsPointXY( 783414, 3350122 ), QgsPointXY( 791344, 3349795 ), QgsPointXY( 783077, 334093 ), QgsPointXY( 791134, 3341401 )}, true ) );

  QVERIFY( transform.transform( QgsPointXY( 30.7302631579, -14.0548245614 ), res, true ) );
  QGSCOMPARENEAR( res.x(), 783414, 1 );
  QGSCOMPARENEAR( res.y(), 3350122, 1 );
  QVERIFY( transform.transform( QgsPointXY( 166.168859649, -167.0548245614 ), res, true ) );
  QGSCOMPARENEAR( res.x(), 791134, 1 );
  QGSCOMPARENEAR( res.y(), 3341401, 1 );
  // reverse transform
  QVERIFY( transform.transform( QgsPointXY( 783414, 3350122 ), res, false ) );
  QGSCOMPARENEAR( res.x(), 30.7302631579, 0.1 );
  QGSCOMPARENEAR( res.y(), -14.0548245614, 0.1 );
  QVERIFY( transform.transform( QgsPointXY( 791134, 3341401 ), res, false ) );
  QGSCOMPARENEAR( res.x(), 166.168859649, 0.1 );
  QGSCOMPARENEAR( res.y(), -167.0548245614, 0.1 );

  // with shift of 100, 200
  QVERIFY( transform.updateParametersFromGcps( {QgsPointXY( 783414, 3350122 ), QgsPointXY( 791344, 3349795 ), QgsPointXY( 783077, 334093 ), QgsPointXY( 791134, 3341401 )},
  {QgsPointXY( 783514, 3350322 ), QgsPointXY( 791444, 3349995 ), QgsPointXY( 783177, 334293 ), QgsPointXY( 791234, 3341601 )}, true ) );

  QVERIFY( transform.transform( QgsPointXY( 30.7302631579, -14.0548245614 ), res, true ) );
  QGSCOMPARENEAR( res.x(), 783514, 1 );
  QGSCOMPARENEAR( res.y(), 3350322, 1 );
  QVERIFY( transform.transform( QgsPointXY( 166.168859649, -167.0548245614 ), res, true ) );
  QGSCOMPARENEAR( res.x(), 791234, 1 );
  QGSCOMPARENEAR( res.y(), 3341601, 1 );
  // reverse transform
  QVERIFY( transform.transform( QgsPointXY( 783514, 3350322 ), res, false ) );
  QGSCOMPARENEAR( res.x(), 30.7302631579, 0.1 );
  QGSCOMPARENEAR( res.y(), -14.0548245614, 0.1 );
  QVERIFY( transform.transform( QgsPointXY( 791234, 3341601 ), res, false ) );
  QGSCOMPARENEAR( res.x(), 166.168859649, 0.1 );
  QGSCOMPARENEAR( res.y(), -167.0548245614, 0.1 );
}

void TestQgsGeoreferencer::testRasterChangeCoords()
{
  QgsRasterChangeCoords transform;
  QVERIFY( !transform.hasExistingGeoreference() );

  transform.loadRaster( QStringLiteral( TEST_DATA_DIR ) + QStringLiteral( "/landsat.tif" ) );
  QVERIFY( transform.hasExistingGeoreference() );
  QGSCOMPARENEAR( transform.toXY( QgsPointXY( 0, 0 ) ).x(), 781662.375, 0.001 );
  QGSCOMPARENEAR( transform.toXY( QgsPointXY( 0, 0 ) ).y(), 3350923.125, 0.001 );
  QGSCOMPARENEAR( transform.toXY( QgsPointXY( 100, 0 ) ).x(), 787362.375, 0.001 );
  QGSCOMPARENEAR( transform.toXY( QgsPointXY( 100, 0 ) ).y(), 3350923.125, 0.001 );
  QGSCOMPARENEAR( transform.toXY( QgsPointXY( 100, 200 ) ).x(), 787362.375, 0.001 );
  QGSCOMPARENEAR( transform.toXY( QgsPointXY( 100, 200 ) ).y(), 3362323.125, 0.001 );

  QGSCOMPARENEAR( transform.toColumnLine( QgsPointXY( 781662.375, 3350923.125 ) ).x(), 0.0, 0.0001 );
  QGSCOMPARENEAR( transform.toColumnLine( QgsPointXY( 781662.375, 3350923.125 ) ).y(), 0.0, 0.0001 );
  QGSCOMPARENEAR( transform.toColumnLine( QgsPointXY( 787362.375, 3350923.125 ) ).x(), 100.0, 0.0001 );
  QGSCOMPARENEAR( transform.toColumnLine( QgsPointXY( 787362.375, 3350923.125 ) ).y(), 0.0, 0.0001 );
  QGSCOMPARENEAR( transform.toColumnLine( QgsPointXY( 787362.375,  3362323.125 ) ).x(), 100.0, 0.0001 );
  QGSCOMPARENEAR( transform.toColumnLine( QgsPointXY( 787362.375,  3362323.125 ) ).y(), 200.0, 0.0001 );

  // load a raster with no georeferencing
  transform.loadRaster( QStringLiteral( TEST_DATA_DIR ) + QStringLiteral( "/rgb256x256.png" ) );
  QVERIFY( !transform.hasExistingGeoreference() );
  // should be treat layer coordinates and pixels as identical
  QCOMPARE( transform.toXY( QgsPointXY( 0, 0 ) ).x(), 0.0 );
  QCOMPARE( transform.toXY( QgsPointXY( 0, 0 ) ).y(), 0.0 );
  QCOMPARE( transform.toXY( QgsPointXY( 100, 0 ) ).x(), 100.0 );
  QCOMPARE( transform.toXY( QgsPointXY( 100, 0 ) ).y(), 0.0 );
  QCOMPARE( transform.toXY( QgsPointXY( 100, 200 ) ).x(), 100.0 );
  QCOMPARE( transform.toXY( QgsPointXY( 100, 200 ) ).y(), 200.0 );

  QCOMPARE( transform.toColumnLine( QgsPointXY( 0, 0 ) ).x(), 0.0 );
  QCOMPARE( transform.toColumnLine( QgsPointXY( 0, 0 ) ).y(), 0.0 );
  QCOMPARE( transform.toColumnLine( QgsPointXY( 100, 0 ) ).x(), 100.0 );
  QCOMPARE( transform.toColumnLine( QgsPointXY( 100, 0 ) ).y(), 0.0 );
  QCOMPARE( transform.toColumnLine( QgsPointXY( 100, 200 ) ).x(), 100.0 );
  QCOMPARE( transform.toColumnLine( QgsPointXY( 100, 200 ) ).y(), 200.0 );
}

QGSTEST_MAIN( TestQgsGeoreferencer )
#include "testqgsgeoreferencer.moc"
