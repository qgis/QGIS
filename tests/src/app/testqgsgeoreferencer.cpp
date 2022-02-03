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

void TestQgsGeoreferencer::testTransformImageNoGeoference()
{
  QgsGeorefTransform transform( QgsGcpTransformerInterface::TransformMethod::Linear );
  // this image has no georeferencing set
  transform.setRasterChangeCoords( QStringLiteral( TEST_DATA_DIR ) + QStringLiteral( "/rgb256x256.png" ) );

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
  transform.setRasterChangeCoords( QStringLiteral( TEST_DATA_DIR ) + QStringLiteral( "/landsat.tif" ) );

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

  // currently disabled -- it is ambiguous whether calling updateParametersFromGcps should be using PIXEL coordinates as source or layer CRS coordinates, and is quite broken either
  // way. Here the disabled tests assume layer CRS coordinates, but in the actual georeferencer a mix of pixel/layer coordinates are used.
  // it would be better if all calls to updateParametersFromGcps ALWAYS use pixel coordinates to avoid the confusion, with the caller transforming from layer CRS coordinates
  // back to pixel coordinates before calling this method.
#if 0
  // source coordinates here should be raster CRS - ie. 32633
  // first use a "null" transform
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
#endif
}

void TestQgsGeoreferencer::testRasterChangeCoords()
{
  QgsRasterChangeCoords transform;
  QVERIFY( !transform.hasExistingGeoreference() );

  transform.setRaster( QStringLiteral( TEST_DATA_DIR ) + QStringLiteral( "/landsat.tif" ) );
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
  transform.setRaster( QStringLiteral( TEST_DATA_DIR ) + QStringLiteral( "/rgb256x256.png" ) );
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
