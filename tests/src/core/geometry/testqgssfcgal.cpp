/***************************************************************************
     testqgssfcgal.cpp
     --------------------------------------
    Date                 : September 2024
    copyright            : (C) 2024 by Benoit De Mezzo
    email                : benoit dot de dot mezzo at oslandia dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifdef WITH_SFCGAL

#include <cmath>
#include <memory>

#include "qgstest.h"

#include <QApplication>
#include <QDesktopServices>
#include <QDir>
#include <QFileInfo>
#include <QImage>
#include <QObject>
#include <QPainter>
#include <QPointF>
#include <QString>
#include <QStringList>
#include <QVector>

// qgis includes...
#include <qgsapplication.h>
#include "qgsgeometry.h"
#include "qgstriangulatedsurface.h"
#include "qgslinestring.h"
#include "qgspolygon.h"
#include "qgsgeometryengine.h"
#include "qgscircle.h"
#include "qgsmultipolygon.h"
#include "qgsgeometrycollection.h"
#include "qgsgeometryfactory.h"
#include "qgscurvepolygon.h"
#include "qgsgeos.h"
#include "qgsexception.h"

#include "qgssfcgalengine.h"
#include "qgssfcgalgeometry.h"

// qgs unit test utility class

#include "testtransformer.h"
#include "testgeometryutils.h"

/**
 * \ingroup UnitTests
 * This is a unit test for the different geometry operations on vector features.
 */
class TestQgsSfcgal : public QgsTest
{
    Q_OBJECT

  public:
    TestQgsSfcgal();

  private slots:
    void initTestCase();    // will be called before the first testfunction is executed.
    void cleanupTestCase(); // will be called after the last testfunction was executed.
    void cleanup();         // will be called after every testfunction.

    void fromWkt();
    void isEqual();
    void boundary();
    void centroid();
    void dropZ();
    void dropM();
    void addZValue();
    void addMValue();
    void swapXY();
    void isEmpty();
    void scale();
    void intersection();
    void intersection3d();
    void unionCheck1();
    void unionCheck2();
    void differenceCheck1();
    void differenceCheck2();
    void difference3d();
    void buffer3DCheck();
    void buffer2DCheck();
    void extrude();
    void simplify();
    void approximateMedialAxis();
    void primitiveCube();

  private:
    //! Must be called before each render test
    void initPainterTest();

    void paintMultiPolygon( QgsMultiPolygonXY &multiPolygon );
    void paintPolygon( QgsPolygonXY &polygon );

    void paintMultiPolygon( const QgsGeometryCollection *multiPolygon );
    void paintPolygon( const QgsPolygon *polygon );
    void paintCurve( const QgsCurve *curve );

    std::unique_ptr<QgsSfcgalGeometry> openWktFile( const QString &wktFile );

    inline bool qFuzzyCompare2( float f1, float f2, float epsilon = 0.000001 )
    {
      return std::abs( f1 - f2 ) < epsilon;
    }

    inline bool qFuzzyCompare2( const QMatrix4x4 &m1, const QMatrix4x4 &m2, float epsilon = 0.000001 )
    {
      const float *d1 = m1.data();
      const float *d2 = m2.data();
      return qFuzzyCompare2( d1[0 * 4 + 0], d2[0 * 4 + 0], epsilon ) && //
             qFuzzyCompare2( d1[0 * 4 + 1], d2[0 * 4 + 1], epsilon ) && //
             qFuzzyCompare2( d1[0 * 4 + 2], d2[0 * 4 + 2], epsilon ) && //
             qFuzzyCompare2( d1[0 * 4 + 3], d2[0 * 4 + 3], epsilon ) && //
             qFuzzyCompare2( d1[1 * 4 + 0], d2[1 * 4 + 0], epsilon ) && //
             qFuzzyCompare2( d1[1 * 4 + 1], d2[1 * 4 + 1], epsilon ) && //
             qFuzzyCompare2( d1[1 * 4 + 2], d2[1 * 4 + 2], epsilon ) && //
             qFuzzyCompare2( d1[1 * 4 + 3], d2[1 * 4 + 3], epsilon ) && //
             qFuzzyCompare2( d1[2 * 4 + 0], d2[2 * 4 + 0], epsilon ) && //
             qFuzzyCompare2( d1[2 * 4 + 1], d2[2 * 4 + 1], epsilon ) && //
             qFuzzyCompare2( d1[2 * 4 + 2], d2[2 * 4 + 2], epsilon ) && //
             qFuzzyCompare2( d1[2 * 4 + 3], d2[2 * 4 + 3], epsilon ) && //
             qFuzzyCompare2( d1[3 * 4 + 0], d2[3 * 4 + 0], epsilon ) && //
             qFuzzyCompare2( d1[3 * 4 + 1], d2[3 * 4 + 1], epsilon ) && //
             qFuzzyCompare2( d1[3 * 4 + 2], d2[3 * 4 + 2], epsilon ) && //
             qFuzzyCompare2( d1[3 * 4 + 3], d2[3 * 4 + 3], epsilon );
    }

    QgsPointXY mPoint1;
    QgsPointXY mPoint2;
    QgsPointXY mPoint3;
    QgsPointXY mPoint4;
    QgsPointXY mPointA;
    QgsPointXY mPointB;
    QgsPointXY mPointC;
    QgsPointXY mPointD;
    QgsPointXY mPointW;
    QgsPointXY mPointX;
    QgsPointXY mPointY;
    QgsPointXY mPointZ;
    QgsPolylineXY mPolylineA;
    QgsPolylineXY mPolylineB;
    QgsPolylineXY mPolylineC;
    QgsPolygonXY mPolygonA;
    QgsPolygonXY mPolygonB;
    QgsPolygonXY mPolygonC;
    QgsGeometry mpPolygonGeometryA;
    QgsGeometry mpPolygonGeometryB;
    QgsGeometry mpPolygonGeometryC;
    QgsGeometry mpPolylineGeometryD;
    QgsGeometry mSfcgalPolygonZA;
    QgsGeometry mSfcgalPolygonZB;
    QgsGeometry mSfcgalPolygonZC;
    QString mWktLine;
    // QString mTestDataDir;
    QImage mImage;
    QPainter *mpPainter = nullptr;
    QPen mPen1;
    QPen mPen2;

    QString mCylinderWkt;
    QString mCubePolyHedWkt;
    QString mCube1SolidWkt;
    QString mCube2SolidWkt;
};

TestQgsSfcgal::TestQgsSfcgal()
  : QgsTest( u"Geometry Tests"_s )
  , mpPolygonGeometryA( nullptr )
  , mpPolygonGeometryB( nullptr )
  , mpPolygonGeometryC( nullptr )
  , mpPolylineGeometryD( nullptr )
{
}

void TestQgsSfcgal::initTestCase()
{
  // Runs once before any tests are run
  // init QGIS's paths - true means that all path will be inited from prefix
  QgsApplication::init();
  QgsApplication::initQgis();
  QgsApplication::showSettings();

  //
  // Reset / reinitialize the geometries before each test is run
  //
  mPoint1 = QgsPointXY( 20.0, 20.0 );
  mPoint2 = QgsPointXY( 80.0, 20.0 );
  mPoint3 = QgsPointXY( 80.0, 80.0 );
  mPoint4 = QgsPointXY( 20.0, 80.0 );
  mPointA = QgsPointXY( 40.0, 40.0 );
  mPointB = QgsPointXY( 100.0, 40.0 );
  mPointC = QgsPointXY( 100.0, 100.0 );
  mPointD = QgsPointXY( 40.0, 100.0 );
  mPointW = QgsPointXY( 200.0, 200.0 );
  mPointX = QgsPointXY( 240.0, 200.0 );
  mPointY = QgsPointXY( 240.0, 240.0 );
  mPointZ = QgsPointXY( 200.0, 240.0 );

  mWktLine = QStringLiteral( "LINESTRING(117.623198 35.198654, 117.581274 35.198654, 117.078178 35.324427, 116.868555 35.534051, 116.617007 35.869448, 116.491233 35.953297, 116.155836 36.288694, 116.071987 36.372544, 115.443117 36.749865, "
                             "114.814247 37.043338, 114.311152 37.169112, 113.388810 37.378735, 113.095337 37.378735, 112.592241 37.378735, 111.753748 37.294886, 111.502201 37.252961, 111.082954 37.127187, 110.747557 37.127187, "
                             "110.160612 36.917564, 110.034838 36.833715, 109.741366 36.749865, 109.573667 36.666016, 109.238270 36.498317, 109.070571 36.414468, 108.819023 36.288694, 108.693250 36.246770, 108.483626 36.162920, "
                             "107.645134 35.911372, 106.597017 35.869448, 106.051997 35.701749, 105.800449 35.617900, 105.590826 35.575975, 105.297354 35.575975, 104.961956 35.575975, 104.710409 35.534051, 104.458861 35.492126, "
                             "103.871916 35.492126, 103.788066 35.492126, 103.326895 35.408277, 102.949574 35.408277, 102.488402 35.450201, 102.069156 35.450201, 101.482211 35.450201, 100.937191 35.659825, 100.308321 35.869448, "
                             "100.056773 36.037146, 99.050582 36.079071, 97.667069 35.743674, 97.163973 35.617900, 96.115857 35.534051, 95.612761 35.534051, 94.396947 35.911372, 93.684228 36.288694, 92.929584 36.833715, 92.258790 37.169112, "
                             "91.629920 37.504509, 90.414105 37.881831, 90.414105 37.881831, 90.246407 37.923755, 89.491763 37.839906, 89.156366 37.672207, 88.485572 37.504509, 87.814778 37.252961, 87.563230 37.169112, 87.143983 37.043338, "
                             "85.970093 36.875639, 85.802395 36.875639, 84.083484 36.959489, 84.041560 37.043338, 82.951519 37.546433, 82.699971 37.630283)" );

  mPolygonA.clear();
  mPolygonB.clear();
  mPolygonC.clear();
  mPolylineA.clear();
  mPolylineB.clear();
  mPolylineC.clear();
  mPolylineA << mPoint1 << mPoint2 << mPoint3 << mPoint4 << mPoint1;
  mPolygonA << mPolylineA;
  // Polygon B intersects Polygon A
  mPolylineB << mPointA << mPointB << mPointC << mPointD << mPointA;
  mPolygonB << mPolylineB;
  // Polygon C should intersect no other polys
  mPolylineC << mPointW << mPointX << mPointY << mPointZ << mPointW;
  mPolygonC << mPolylineC;

  mpPolylineGeometryD = QgsGeometry::fromWkt( mWktLine );

  // polygon: first item of the list is outer ring,
  // inner rings (if any) start from second item
  mpPolygonGeometryA = QgsGeometry::fromPolygonXY( mPolygonA );
  mpPolygonGeometryB = QgsGeometry::fromPolygonXY( mPolygonB );
  mpPolygonGeometryC = QgsGeometry::fromPolygonXY( mPolygonC );

  // triangle with big hole, intersects nothing
  mSfcgalPolygonZA = QgsGeometry::fromWkt(
    QStringLiteral( "PolygonZ ("
                    "(-7037.2105 4814.2525 20, -7112.2748 3024.2574 20, -4669.7976 2770.1936 20, -3428.3493 3278.3212 20, -7037.2105 4814.2525 20),"
                    "(-6832.9186 4498.2679 20, -3923.7277 3302.1336 20, -4717.1434 2947.5009 20, -6862.9722 3193.9406 20, -6832.9186 4498.2679 20)"
                    ")" )
  );
  // triangle with small hole, intersects mSfcgalPolygonZC
  mSfcgalPolygonZB = QgsGeometry::fromWkt(
    QStringLiteral( "PolygonZ ("
                    "(-5728.7630 4205.9883 0, -7160.7591 4662.1484 0, -4903.0556 1942.5107 0, -3384.4469 3426.4743 0, -5728.7630 4205.9883 0),"
                    "(-5128.9946 2875.5984 0, -5189.1018 2701.2874 0, -5357.4021 2833.5233 0, -5128.9946 2875.5984 0)"
                    ")" )
  );

  // triangle from bottom to top, intersects mSfcgalPolygonZB
  mSfcgalPolygonZC = QgsGeometry::fromWkt(
    QStringLiteral( "PolygonZ ("
                    "(-6547.4512 5101.3843 -50, -7609.9000 6077.2203 -50, -7690.7384 4281.4510 -50, -5080.8101 2930.2934 50, -6547.4512 5101.3843 -50)"
                    ")" )
  );

  mCylinderWkt = QString( "PolyhedralSurface Z (((150 90 0, 135.35 54.64 0, 100 40 0, 64.64 54.64 0, 50 90 0, 64.64 125.35 0, 100 140 0, 135.35 125.35 0, 150 90 0)),"
                          "((150 90 30, 135.35 125.35 30, 100 140 30, 64.64 125.35 30, 50 90 30, 64.64 54.64 30, 100 40 30, 135.35 54.64 30, 150 90 30)),"
                          "((150 90 0, 150 90 30, 135.35 54.64 30, 135.35 54.64 0, 150 90 0)),"
                          "((135.35 54.64 0, 135.35 54.64 30, 100 40 30, 100 40 0, 135.35 54.64 0)),"
                          "((100 40 0, 100 40 30, 64.64 54.64 30, 64.64 54.64 0, 100 40 0)),"
                          "((64.64 54.64 0, 64.64 54.64 30, 50 90 30, 50 90 0, 64.64 54.64 0)),"
                          "((50 90 0, 50 90 30, 64.64 125.35 30, 64.64 125.35 0, 50 90 0)),"
                          "((64.64 125.35 0, 64.64 125.35 30, 100 140 30, 100 140 0, 64.64 125.35 0)),"
                          "((100 140 0, 100 140 30, 135.35 125.35 30, 135.35 125.35 0, 100 140 0)),"
                          "((135.35 125.35 0, 135.35 125.35 30, 150 90 30, 150 90 0, 135.35 125.35 0)))" );

  mCubePolyHedWkt = QString( "PolyhedralSurface Z (((130 80 0, 80 30 0, 30 80 0, 80 130 0, 130 80 0)),"
                             "((130 80 30, 80 130 30, 30 80 30, 80 30 30, 130 80 30)),"
                             "((130 80 0, 130 80 30, 80 30 30, 80 30 0, 130 80 0)),"
                             "((80 30 0, 80 30 30, 30 80 30, 30 80 0, 80 30 0)),"
                             "((30 80 0, 30 80 30, 80 130 30, 80 130 0, 30 80 0)),"
                             "((80 130 0, 80 130 30, 130 80 30, 130 80 0, 80 130 0)))" );

  mCube1SolidWkt = QString( "SOLID Z ((((0.00 0.00 0.00,0.00 10.00 0.00,10.00 10.00 0.00,10.00 0.00 0.00,0.00 0.00 0.00)),"
                            "((10.00 0.00 0.00,10.00 10.00 0.00,10.00 10.00 10.00,10.00 0.00 10.00,10.00 0.00 0.00)),"
                            "((0.00 10.00 0.00,0.00 10.00 10.00,10.00 10.00 10.00,10.00 10.00 0.00,0.00 10.00 0.00)),"
                            "((0.00 0.00 10.00,0.00 10.00 10.00,0.00 10.00 0.00,0.00 0.00 0.00,0.00 0.00 10.00)),"
                            "((10.00 0.00 10.00,10.00 10.00 10.00,0.00 10.00 10.00,0.00 0.00 10.00,10.00 0.00 10.00)),"
                            "((10.00 0.00 0.00,10.00 0.00 10.00,0.00 0.00 10.00,0.00 0.00 0.00,10.00 0.00 0.00))))" );
  mCube2SolidWkt = QString( "SOLID Z ((((2.00 2.00 2.00,2.00 12.00 2.00,12.00 12.00 2.00,12.00 2.00 2.00,2.00 2.00 2.00)),"
                            "((12.00 2.00 2.00,12.00 12.00 2.00,12.00 12.00 12.00,12.00 2.00 12.00,12.00 2.00 2.00)),"
                            "((2.00 12.00 2.00,2.00 12.00 12.00,12.00 12.00 12.00,12.00 12.00 2.00,2.00 12.00 2.00)),"
                            "((2.00 2.00 12.00,2.00 12.00 12.00,2.00 12.00 2.00,2.00 2.00 2.00,2.00 2.00 12.00)),"
                            "((12.00 2.00 12.00,12.00 12.00 12.00,2.00 12.00 12.00,2.00 2.00 12.00,12.00 2.00 12.00)),"
                            "((12.00 2.00 2.00,12.00 2.00 12.00,2.00 2.00 12.00,2.00 2.00 2.00,12.00 2.00 2.00))))" );
}


void TestQgsSfcgal::cleanupTestCase()
{
  delete mpPainter;
  mpPainter = nullptr;
  QgsApplication::exitQgis();
}

void TestQgsSfcgal::initPainterTest()
{
  delete mpPainter;
  mpPainter = nullptr;
  mImage = QImage( 250, 250, QImage::Format_RGB32 );
  mImage.fill( qRgb( 152, 219, 249 ) );
  mpPainter = new QPainter( &mImage );

  // Draw the test shapes first
  mPen1 = QPen();
  mPen1.setWidth( 5 );
  mPen1.setBrush( Qt::green );
  mpPainter->setPen( mPen1 );
  paintPolygon( mPolygonA );
  mPen1.setBrush( Qt::red );
  mpPainter->setPen( mPen1 );
  paintPolygon( mPolygonB );
  mPen1.setBrush( Qt::blue );
  mpPainter->setPen( mPen1 );
  paintPolygon( mPolygonC );

  mPen2 = QPen();
  mPen2.setWidth( 1 );
  mPen2.setBrush( Qt::black );
  QBrush myBrush( Qt::DiagCrossPattern );


  // set the pen to a different color -
  // any test outs will be drawn in pen2
  mpPainter->setPen( mPen2 );
  mpPainter->setBrush( myBrush );
}

void TestQgsSfcgal::cleanup()
{
  // will be called after every testfunction.
}

std::unique_ptr<QgsSfcgalGeometry> TestQgsSfcgal::openWktFile( const QString &wktFile )
{
  QString expectedPath = testDataPath( QString( "control_files/expected_sfcgal/expected_%1" ).arg( wktFile ) );
  QFile expectedFile( expectedPath );
  if ( !expectedFile.open( QFile::ReadOnly | QIODevice::Text ) )
  {
    qWarning() << "Unable to open expected data file" << expectedPath;
    return nullptr;
  }

  // remove '\n' from dumped file
  QByteArray expectedBA = expectedFile.readAll();
  QString expectedStr;
  for ( int i = 0; i < expectedBA.length(); i++ )
  {
    if ( expectedBA.at( i ) != '\n' )
      expectedStr += expectedBA.at( i );
  }

  // load geom from corrected wkt
  return std::make_unique<QgsSfcgalGeometry>( expectedStr );
}

void TestQgsSfcgal::paintMultiPolygon( QgsMultiPolygonXY &multiPolygon )
{
  for ( int i = 0; i < multiPolygon.size(); i++ )
  {
    QgsPolygonXY myPolygon = multiPolygon.at( i );
    paintPolygon( myPolygon );
  }
}

void TestQgsSfcgal::paintMultiPolygon( const QgsGeometryCollection *multiPolygon )
{
  for ( int i = 0; i < multiPolygon->partCount(); i++ )
  {
    const QgsAbstractGeometry *part = multiPolygon->geometryN( i );
    QVERIFY2( part->wkbType() == Qgis::WkbType::Polygon || part->wkbType() == Qgis::WkbType::PolygonZ, "must be WkbType::PolygonXX" );
    const QgsPolygon *myPolygon = dynamic_cast<const QgsPolygon *>( part );
    paintPolygon( myPolygon );
  }
}

void TestQgsSfcgal::paintPolygon( QgsPolygonXY &polygon )
{
  QVector<QPointF> myPoints;
  for ( int j = 0; j < polygon.size(); j++ )
  {
    const QgsPolylineXY &myPolyline = polygon.at( j ); // rings of polygon
    for ( int k = 0; k < myPolyline.size(); k++ )
    {
      const QgsPointXY &myPoint = myPolyline.at( k );
      myPoints << QPointF( myPoint.x(), myPoint.y() );
    }
  }
  mpPainter->drawPolygon( myPoints );
}

void TestQgsSfcgal::paintPolygon( const QgsPolygon *polygon )
{
  paintCurve( polygon->exteriorRing() );
  for ( int i = 0; i < polygon->numInteriorRings(); i++ )
  {
    paintCurve( polygon->interiorRing( i ) );
  }
}

void TestQgsSfcgal::paintCurve( const QgsCurve *curve )
{
  curve->drawAsPolygon( *mpPainter );
}

void TestQgsSfcgal::fromWkt()
{
  QVERIFY_EXCEPTION_THROWN( QgsSfcgalGeometry( "Point Z (-5673.79 3594.8 20, 5)" ), QgsSfcgalException );

  QVERIFY_EXCEPTION_THROWN( QgsSfcgalGeometry::fromWkt( "Point Z (-5673.79 3594.8 20, 5)" ), QgsSfcgalException );
  QVERIFY( !sfcgal::errorHandler()->isTextEmpty() );
  QVERIFY( sfcgal::errorHandler()->getFullText().contains( "SFCGAL error occurred: WKT parse error" ) );
}

void TestQgsSfcgal::isEqual()
{
  initPainterTest();

  QgsSfcgalGeometry geomA( mpPolygonGeometryA );

  // test with cloned geometry
  std::unique_ptr<QgsSfcgalGeometry> cloneGeomA( geomA.clone() );
  QVERIFY2( cloneGeomA, "QgsSfcgalGeometry::clone failure" );
  QCOMPARE( geomA.wkbType(), cloneGeomA->wkbType() );

#if SFCGAL_VERSION < SFCGAL_MAKE_VERSION( 2, 1, 0 )
  QVERIFY_EXCEPTION_THROWN( { bool res = (geomA == *cloneGeomA.get()); Q_UNUSED(res); }, QgsNotSupportedException );
#else
  QVERIFY2( geomA == *cloneGeomA.get(), "Should be equals" );
  QVERIFY2( sfcgal::errorHandler()->isTextEmpty(), "isEquals should not fail" );
#endif

  // test with offset geometry
  QgsVector3D vector( 1.0, 1.0, 0 );
#if SFCGAL_VERSION < SFCGAL_MAKE_VERSION( 2, 1, 0 )
  QVERIFY_EXCEPTION_THROWN( geomA.translate( vector ), QgsNotSupportedException );
#else
  std::unique_ptr<QgsSfcgalGeometry> geomB( geomA.translate( vector ) );

  // should failed
  QVERIFY2( geomA != *geomB.get(), "Should not be equals" );

  // should be accepted
  QVERIFY2( geomA.fuzzyEqual( *( geomB.get() ), 0.05 ), "Should be equals" );
#endif
}

void TestQgsSfcgal::boundary()
{
  // 2D line
  auto sfcgalLine2D = std::make_unique<QgsSfcgalGeometry>( mpPolylineGeometryD );
#if SFCGAL_VERSION < SFCGAL_MAKE_VERSION( 2, 1, 0 )
  QVERIFY_EXCEPTION_THROWN( sfcgalLine2D->boundary(), QgsNotSupportedException );
#else
  std::unique_ptr<QgsSfcgalGeometry> sfcgalLine2DBoundary( sfcgalLine2D->boundary() );
  QCOMPARE( sfcgalLine2DBoundary->asWkt(), "MULTIPOINT ((517312295588795/4398046511104 4953770157448219/140737488355328),"
                                           "(45464789865619/549755813888 5295991515520197/140737488355328))" );
#endif

  // 3D polygon
  auto sfcgalPolygon3D = std::make_unique<QgsSfcgalGeometry>( mSfcgalPolygonZA );
#if SFCGAL_VERSION < SFCGAL_MAKE_VERSION( 2, 1, 0 )
  QVERIFY_EXCEPTION_THROWN( sfcgalPolygon3D->boundary(), QgsNotSupportedException );
#else
  std::unique_ptr<QgsSfcgalGeometry> sfcgalPolygon3DBoundary( sfcgalPolygon3D->boundary() );

  std::unique_ptr<QgsSfcgalGeometry> expectedPolygon3DBoundary = openWktFile( "boundary_polygon3d.wkt" );
  QCOMPARE( sfcgalPolygon3DBoundary->asWkt(), expectedPolygon3DBoundary->asWkt() );
#endif
}

void TestQgsSfcgal::centroid()
{
  auto geomZA = std::make_unique<QgsSfcgalGeometry>( mSfcgalPolygonZA );
  auto geomZB = std::make_unique<QgsSfcgalGeometry>( mSfcgalPolygonZB );
  auto geomZC = std::make_unique<QgsSfcgalGeometry>( mSfcgalPolygonZC );

#if SFCGAL_VERSION < SFCGAL_MAKE_VERSION( 2, 1, 0 )
  QVERIFY_EXCEPTION_THROWN( geomZA->centroid(), QgsNotSupportedException );
  QVERIFY_EXCEPTION_THROWN( geomZB->centroid(), QgsNotSupportedException );
  QVERIFY_EXCEPTION_THROWN( geomZC->centroid(), QgsNotSupportedException );
#else
  auto res = std::make_unique<QgsPoint>( geomZA->centroid() );
  QCOMPARE( res->asWkt( 2 ), u"Point Z (-5673.79 3594.8 20)"_s );

  res = std::make_unique<QgsPoint>( geomZB->centroid() );
  QCOMPARE( res->asWkt( 2 ), u"Point Z (-5150.77 3351.12 0)"_s );

  res = std::make_unique<QgsPoint>( geomZC->centroid() );
  QCOMPARE( res->asWkt( 2 ), u"Point Z (-6734.85 4471.67 -28.34)"_s );
#endif
}

void TestQgsSfcgal::dropZ()
{
  // PolygonZ
  auto sfcgalPolygonZ = std::make_unique<QgsSfcgalGeometry>( mSfcgalPolygonZA );
  QCOMPARE( sfcgalPolygonZ->wkbType(), Qgis::WkbType::PolygonZ );
#if SFCGAL_VERSION < SFCGAL_MAKE_VERSION( 2, 1, 0 )
  QVERIFY_EXCEPTION_THROWN( sfcgalPolygonZ->dropZValue(), QgsNotSupportedException );
#else
  QVERIFY( sfcgalPolygonZ->dropZValue() );
  QCOMPARE( sfcgalPolygonZ->asWkt( 1 ), "POLYGON ((-7037.2 4814.3,-7112.3 3024.3,-4669.8 2770.2,-3428.3 3278.3,-7037.2 4814.3),(-6832.9 4498.3,-3923.7 3302.1,-4717.1 2947.5,-6863.0 3193.9,-6832.9 4498.3))" );
  QCOMPARE( sfcgalPolygonZ->wkbType(), Qgis::WkbType::Polygon );
  QVERIFY( !sfcgalPolygonZ->dropZValue() );

  // Polygon2D
  auto sfcgalPolygon2D = std::make_unique<QgsSfcgalGeometry>( QgsGeometry::fromPolygonXY( mPolygonA ) );
  QCOMPARE( sfcgalPolygon2D->wkbType(), Qgis::WkbType::Polygon );
  QVERIFY( !sfcgalPolygon2D->dropZValue() );
  QCOMPARE( sfcgalPolygon2D->wkbType(), Qgis::WkbType::Polygon );

  // PolygonM
  QgsSfcgalGeometry sfcgalPolygonM( "POLYGON M ((0 0 1, 20 0 2, 20 10 3, 0 10 4, 0 0 1))" );
  QCOMPARE( sfcgalPolygonM.wkbType(), Qgis::WkbType::PolygonM );
  QVERIFY( !sfcgalPolygonM.dropZValue() );
  QCOMPARE( sfcgalPolygonM.wkbType(), Qgis::WkbType::PolygonM );

  // PolygonZM
  QgsSfcgalGeometry sfcgalPolygonZM( "POLYGON ZM ((0 0 1 2, 20 0 2 2, 20 10 3 2, 0 10 4 2, 0 0 1 2))" );
  QCOMPARE( sfcgalPolygonZM.wkbType(), Qgis::WkbType::PolygonZM );
  QVERIFY( sfcgalPolygonZM.dropZValue() );
  QCOMPARE( sfcgalPolygonZM.asWkt( 1 ), "POLYGON M ((0.0 0.0 2.0,20.0 0.0 2.0,20.0 10.0 2.0,0.0 10.0 2.0,0.0 0.0 2.0))" );
  QCOMPARE( sfcgalPolygonZM.wkbType(), Qgis::WkbType::PolygonM );
  QVERIFY( !sfcgalPolygonZM.dropZValue() );
#endif
}

void TestQgsSfcgal::dropM()
{
  // PolygonZ
  auto sfcgalPolygonZ = std::make_unique<QgsSfcgalGeometry>( mSfcgalPolygonZA );
  QCOMPARE( sfcgalPolygonZ->wkbType(), Qgis::WkbType::PolygonZ );
#if SFCGAL_VERSION < SFCGAL_MAKE_VERSION( 2, 1, 0 )
  QVERIFY_EXCEPTION_THROWN( sfcgalPolygonZ->dropMValue(), QgsNotSupportedException );
#else
  QVERIFY( !sfcgalPolygonZ->dropMValue() );
  QCOMPARE( sfcgalPolygonZ->wkbType(), Qgis::WkbType::PolygonZ );

  // Polygon2D
  auto sfcgalPolygon2D = std::make_unique<QgsSfcgalGeometry>( QgsGeometry::fromPolygonXY( mPolygonA ) );
  QCOMPARE( sfcgalPolygon2D->wkbType(), Qgis::WkbType::Polygon );
  QVERIFY( !sfcgalPolygon2D->dropMValue() );
  QCOMPARE( sfcgalPolygon2D->wkbType(), Qgis::WkbType::Polygon );

  // PolygonM
  QgsSfcgalGeometry sfcgalPolygonM( "POLYGON M ((0 0 1, 20 0 2, 20 10 3, 0 10 4, 0 0 1))" );
  QCOMPARE( sfcgalPolygonM.wkbType(), Qgis::WkbType::PolygonM );
  QVERIFY( sfcgalPolygonM.dropMValue() );
  QCOMPARE( sfcgalPolygonM.asWkt( 1 ), "POLYGON ((0.0 0.0,20.0 0.0,20.0 10.0,0.0 10.0,0.0 0.0))" );
  QCOMPARE( sfcgalPolygonM.wkbType(), Qgis::WkbType::Polygon );

  // PolygonZM
  QgsSfcgalGeometry sfcgalPolygonZM( "POLYGON ZM ((0 0 1 2, 20 0 2 2, 20 10 3 2, 0 10 4 2, 0 0 1 2))" );
  QCOMPARE( sfcgalPolygonZM.wkbType(), Qgis::WkbType::PolygonZM );
  QVERIFY( sfcgalPolygonZM.dropMValue() );
  QCOMPARE( sfcgalPolygonZM.asWkt( 1 ), "POLYGON Z ((0.0 0.0 1.0,20.0 0.0 2.0,20.0 10.0 3.0,0.0 10.0 4.0,0.0 0.0 1.0))" );
  QCOMPARE( sfcgalPolygonZM.wkbType(), Qgis::WkbType::PolygonZ );
  QVERIFY( !sfcgalPolygonZM.dropMValue() );
#endif
}

void TestQgsSfcgal::addZValue()
{
  // 2D Point
  QgsSfcgalGeometry sfcgalPoint2D( "POINT (4 2)" );
  QCOMPARE( sfcgalPoint2D.wkbType(), Qgis::WkbType::Point );
#if SFCGAL_VERSION < SFCGAL_MAKE_VERSION( 2, 1, 0 )
  QVERIFY_EXCEPTION_THROWN( sfcgalPoint2D.addZValue(), QgsNotSupportedException );
#else
  QVERIFY( sfcgalPoint2D.addZValue( 4 ) );
  QCOMPARE( sfcgalPoint2D.asWkt( 1 ), "POINT Z (4.0 2.0 4.0)" );
  QCOMPARE( sfcgalPoint2D.wkbType(), Qgis::WkbType::PointZ );
  QVERIFY( !sfcgalPoint2D.addZValue() );

  // PolygonM
  QgsSfcgalGeometry sfcgalPolygonM( "POLYGON M ((0 0 1, 20 0 2, 20 10 3, 0 10 4, 0 0 1))" );
  QCOMPARE( sfcgalPolygonM.wkbType(), Qgis::WkbType::PolygonM );
  QVERIFY( sfcgalPolygonM.addZValue() );
  QCOMPARE( sfcgalPolygonM.wkbType(), Qgis::WkbType::PolygonZM );
  QCOMPARE( sfcgalPolygonM.asWkt( 1 ), "POLYGON ZM ((0.0 0.0 0.0 1.0,20.0 0.0 0.0 2.0,20.0 10.0 0.0 3.0,0.0 10.0 0.0 4.0,0.0 0.0 0.0 1.0))" );
  QVERIFY( !sfcgalPolygonM.addZValue() );

  // LineZ
  QgsSfcgalGeometry sfcgalLineZ( "LINESTRING Z (40 80 2, 40 40 2, 80 40 2, 80 80 2, 40 80 2)" );
  QCOMPARE( sfcgalLineZ.wkbType(), Qgis::WkbType::LineStringZ );
  QVERIFY( !sfcgalLineZ.addZValue() );
#endif
}

void TestQgsSfcgal::addMValue()
{
  // 2D Point
  QgsSfcgalGeometry sfcgalPoint2D( "POINT (4 2)" );
  QCOMPARE( sfcgalPoint2D.wkbType(), Qgis::WkbType::Point );
#if SFCGAL_VERSION < SFCGAL_MAKE_VERSION( 2, 1, 0 )
  QVERIFY_EXCEPTION_THROWN( sfcgalPoint2D.addMValue( 5 ), QgsNotSupportedException );
#else
  QVERIFY( sfcgalPoint2D.addMValue( 5 ) );
  QCOMPARE( sfcgalPoint2D.asWkt( 1 ), "POINT M (4.0 2.0 5.0)" );
  QCOMPARE( sfcgalPoint2D.wkbType(), Qgis::WkbType::PointM );
  QVERIFY( !sfcgalPoint2D.addMValue() );

  // PolygonM
  QgsSfcgalGeometry sfcgalPolygonM( "POLYGON M ((0 0 1, 20 0 2, 20 10 3, 0 10 4, 0 0 1))" );
  QCOMPARE( sfcgalPolygonM.wkbType(), Qgis::WkbType::PolygonM );
  QVERIFY( !sfcgalPolygonM.addMValue() );

  // LineZ
  QgsSfcgalGeometry sfcgalLineZ( "LINESTRING Z (40 80 2, 40 40 2, 80 40 2, 80 80 2, 40 80 2)" );
  QCOMPARE( sfcgalLineZ.wkbType(), Qgis::WkbType::LineStringZ );
  QVERIFY( sfcgalLineZ.addMValue() );
  QCOMPARE( sfcgalLineZ.asWkt( 1 ), "LINESTRING ZM (40.0 80.0 2.0 0.0,40.0 40.0 2.0 0.0,80.0 40.0 2.0 0.0,80.0 80.0 2.0 0.0,40.0 80.0 2.0 0.0)" );
  QCOMPARE( sfcgalLineZ.wkbType(), Qgis::WkbType::LineStringZM );
  QVERIFY( !sfcgalLineZ.addMValue() );
#endif
}

void TestQgsSfcgal::swapXY()
{
  // 2D Point
  QgsSfcgalGeometry sfcgalPoint2D( "POINT (4 2)" );
  QCOMPARE( sfcgalPoint2D.wkbType(), Qgis::WkbType::Point );
#if SFCGAL_VERSION < SFCGAL_MAKE_VERSION( 2, 1, 0 )
  QVERIFY_EXCEPTION_THROWN( sfcgalPoint2D.swapXy();, QgsNotSupportedException );
#else
  sfcgalPoint2D.swapXy();
  QCOMPARE( sfcgalPoint2D.asWkt( 1 ), "POINT (2.0 4.0)" );

  // PolygonM
  QgsSfcgalGeometry sfcgalPolygonM( "POLYGON M ((1 0 1, 20 0 2, 20 10 3, 0 10 4, 1 0 1))" );
  QCOMPARE( sfcgalPolygonM.wkbType(), Qgis::WkbType::PolygonM );
  sfcgalPolygonM.swapXy();
  QCOMPARE( sfcgalPolygonM.asWkt( 1 ), "POLYGON M ((0.0 1.0 1.0,0.0 20.0 2.0,10.0 20.0 3.0,10.0 0.0 4.0,0.0 1.0 1.0))" );

  // LineZ
  QgsSfcgalGeometry sfcgalLineZ( "LINESTRING Z (40 80 2, 40 40 2, 80 40 2, 80 80 2, 40 80 2)" );
  QCOMPARE( sfcgalLineZ.wkbType(), Qgis::WkbType::LineStringZ );
  sfcgalLineZ.swapXy();
  QCOMPARE( sfcgalLineZ.asWkt( 1 ), "LINESTRING Z (80.0 40.0 2.0,40.0 40.0 2.0,40.0 80.0 2.0,80.0 80.0 2.0,80.0 40.0 2.0)" );
#endif
}

void TestQgsSfcgal::isEmpty()
{
  auto sfcgalGeomZA = std::make_unique<QgsSfcgalGeometry>( mSfcgalPolygonZA );
  QVERIFY( !sfcgalGeomZA->isEmpty() );

  QgsSfcgalGeometry sfcgalGeomEmpty( "POLYGON EMPTY" );
  QVERIFY( sfcgalGeomEmpty.isEmpty() );
}

void TestQgsSfcgal::scale()
{
  // simple 2D Point
  QgsSfcgalGeometry sfcgalPoint( "POINT (4 3)" );
  std::unique_ptr<QgsSfcgalGeometry> sfcgalScalePoint( sfcgalPoint.scale( QgsVector3D( 2, 3, 0 ), QgsPoint() ) );
  QCOMPARE( sfcgalScalePoint->asWkt( 0 ), u"POINT (8 9)"_s );

  // simple 2D Point with center
  std::unique_ptr<QgsSfcgalGeometry> sfcgalScalePointCenter( sfcgalPoint.scale( QgsVector3D( 2, 3, 0 ), QgsPoint( 1, 1 ) ) );
  QCOMPARE( sfcgalScalePointCenter->asWkt( 0 ), u"POINT (7 7)"_s );

  // simple 3D Point
  QgsSfcgalGeometry sfcgalPoint3D( "POINT Z (4 3 2)" );
  std::unique_ptr<QgsSfcgalGeometry> sfcgalScalePoint3D( sfcgalPoint3D.scale( QgsVector3D( 2, 3, 4 ), QgsPoint() ) );
  QCOMPARE( sfcgalScalePoint3D->asWkt( 0 ), u"POINT Z (8 9 8)"_s );

  // simple 3D Point with center
  std::unique_ptr<QgsSfcgalGeometry> sfcgalScalePoint3DCenter( sfcgalPoint3D.scale( QgsVector3D( 2, 3, 4 ), QgsPoint( 1, 1, 2 ) ) );
  QCOMPARE( sfcgalScalePoint3DCenter->asWkt( 0 ), u"POINT Z (7 7 2)"_s );

  // 3D Polygon - no center
  auto sfcgalPolygonA = std::make_unique<QgsSfcgalGeometry>( mSfcgalPolygonZA );
  const QgsVector3D scaleFactorPolygon( 2, 2, 2 );
  std::unique_ptr<QgsSfcgalGeometry> sfcgalScalePolygonA( sfcgalPolygonA->scale( scaleFactorPolygon, QgsPoint() ) );

  const QString expectedWktPolygonA( "POLYGON Z ((-7737494771857359/549755813888 5293326602799677/549755813888 40/1,-7820028842538225/549755813888 6650412353375227/1099511627776 40/1,-2567248380280229/274877906944 6091720148781315/1099511627776 40/1,-1884754959713855/274877906944 7209104557969139/1099511627776 40/1,-7737494771857359/549755813888 5293326602799677/549755813888 40/1),(-7512873452346907/549755813888 4945897860901529/549755813888 40/1,-8628368460753561/1099511627776 3630734289669823/549755813888 40/1,-5186554018186815/549755813888 3240811512430225/549755813888 40/1,-1886479433750859/137438953472 1755887414062927/274877906944 40/1,-7512873452346907/549755813888 4945897860901529/549755813888 40/1))" );
  QgsSfcgalGeometry expectedScalePolygonA( expectedWktPolygonA );
  QVERIFY2( expectedScalePolygonA.covers( *sfcgalScalePolygonA.get() ), "scale polygon A does not match expected WKT" );

  // 3D Polygon - with center
  const QgsPoint centerA( -5037.1, 4414.0, 20.0 );
  std::unique_ptr<QgsSfcgalGeometry> sfcgalScalePolygonACenter( sfcgalPolygonA->scale( scaleFactorPolygon, centerA ) );

  const QString expectedWktPolygonACenter( "POLYGON Z ((-2484159880861057/274877906944 2866704440298045/549755813888 20/1,-1262713458100745/137438953472 1797168028371963/1099511627776 20/1,-2365321750425213/549755813888 1238475823778051/1099511627776 20/1,-1000334909292465/549755813888 2355860232965875/1099511627776 20/1,-2484159880861057/274877906944 2866704440298045/549755813888 20/1),(-2371849221105831/274877906944 2519275698399897/549755813888 20/1,-3090018440483071/1099511627776 1204112127168191/549755813888 20/1,-1208689504025785/274877906944 814189349928593/549755813888 20/1,-4776742724868191/549755813888 542576332812111/274877906944 20/1,-2371849221105831/274877906944 2519275698399897/549755813888 20/1))" );

  QgsSfcgalGeometry expectedScalePolygonACenter( expectedWktPolygonACenter );
  QVERIFY2( expectedScalePolygonACenter.covers( *sfcgalScalePolygonACenter.get() ), "scale polygon A with center does not match expected WKT" );

  // 2D LineString - no center
  auto sfcgalLineD = std::make_unique<QgsSfcgalGeometry>( mpPolylineGeometryD );
  const QgsVector3D scaleFactorLine( 3, 2, 0 );
  std::unique_ptr<QgsSfcgalGeometry> sfcgalScaleLineD( sfcgalLineD->scale( scaleFactorLine, QgsPoint() ) );

  std::unique_ptr<QgsSfcgalGeometry> expectedScaleLineD = openWktFile( "scale_line_d.wkt" );
  QVERIFY2( expectedScaleLineD->covers( *sfcgalScaleLineD.get() ), "scale line D does not match expected WKT" );

  // 2D LineString - center
  std::unique_ptr<QgsSfcgalGeometry> sfcgalScaleLineDCenter( sfcgalLineD->scale( scaleFactorLine, QgsPoint( 90, 30 ) ) );

  std::unique_ptr<QgsSfcgalGeometry> expectedScaleLineDCenter = openWktFile( "scale_line_d_center.wkt" );
  QVERIFY2( expectedScaleLineDCenter->covers( *sfcgalScaleLineDCenter.get() ), "scale line D with centerdoes not match expected WKT" );
}

void TestQgsSfcgal::intersection()
{
  initPainterTest();

  QgsSfcgalGeometry geomA( mpPolygonGeometryA );
  QVERIFY( geomA.intersects( mpPolygonGeometryB.constGet() ) );

  // should be a single polygon as A intersect B
  std::unique_ptr<QgsSfcgalGeometry> intersectionGeom = geomA.intersection( mpPolygonGeometryB.constGet() );
  QVERIFY2( intersectionGeom, "intersectionGeom is NULL. " );
  QCOMPARE( intersectionGeom->wkbType(), Qgis::WkbType::Polygon );

  const QgsPolygon *intersectionPoly = qgsgeometry_cast<const QgsPolygon *>( intersectionGeom->asQgisGeometry().release() );
  QVERIFY( intersectionPoly );                               // check that the union created a feature
  QVERIFY( intersectionPoly->exteriorRing()->length() > 0 ); // check that the union created a feature
  QCOMPARE( intersectionPoly->exteriorRing()->asWkt(), u"LineString (40 80, 40 40, 80 40, 80 80, 40 80)"_s );

  paintPolygon( intersectionPoly );
  QGSVERIFYIMAGECHECK( "Checking if A intersects B (SFCGAL)", "geometry_intersectionCheck1", mImage, QString(), 0 );
}

void TestQgsSfcgal::intersection3d()
{
  initPainterTest();

  // first triangulate polygon as some are not coplanar
  auto geomZA = std::make_unique<QgsSfcgalGeometry>( mSfcgalPolygonZA );
  auto geomZB = std::make_unique<QgsSfcgalGeometry>( mSfcgalPolygonZB );
  auto geomZC = std::make_unique<QgsSfcgalGeometry>( mSfcgalPolygonZC );

  geomZA = geomZA->triangulate();
  geomZB = geomZB->triangulate();
  geomZC = geomZC->triangulate();

  // second intersect triangulated polygons
  QVERIFY( !geomZA->intersects( *geomZB.get() ) );

  {
    QVERIFY( geomZA->intersects( *geomZC.get() ) );
    std::unique_ptr<QgsSfcgalGeometry> scInterGeom = geomZA->intersection( *geomZC.get() );
    QVERIFY2( scInterGeom, "intersectionGeom is NULL." );
    QCOMPARE( scInterGeom->wkbType(), Qgis::WkbType::MultiLineStringZ );
    QCOMPARE( scInterGeom->asWkt(), QStringLiteral( "MULTILINESTRING Z ("
                                                    "(-64473037375252357/10995116277760 73351514274852871/21990232555520 20/1,"
                                                    "-129431703432785341454725506788272017716469477079/23313107857443583859443690159709871847505920 41489508487091336404601688874406905270455411499/11656553928721791929721845079854935923752960 20/1),"
                                                    "(-30350932332194981/5497558138880 78760671458957353/21990232555520 20/1,"
                                                    "-129431703432785341454725506788272017716469477079/23313107857443583859443690159709871847505920 41489508487091336404601688874406905270455411499/11656553928721791929721845079854935923752960 20/1)"
                                                    ")" ) );

    const QgsMultiCurve *interCurve = qgsgeometry_cast<const QgsMultiCurve *>( scInterGeom->asQgisGeometry().release() );
    QCOMPARE( interCurve->partCount(), 2 ); // check that the operation created 2 features
    QCOMPARE( interCurve->curveN( 0 )->asWkt( 2 ), u"LineString Z (-5863.79 3335.64 20, -5551.89 3559.33 20)"_s );
    QCOMPARE( interCurve->curveN( 1 )->asWkt( 2 ), u"LineString Z (-5520.8 3581.62 20, -5551.89 3559.33 20)"_s );
  }

  {
    QVERIFY( geomZB->intersects( *geomZC.get() ) );
    std::unique_ptr<QgsSfcgalGeometry> scInterGeom = geomZB->intersection( *geomZC.get() );
    QVERIFY2( scInterGeom, "intersectionGeom is NULL" );
    QCOMPARE( scInterGeom->wkbType(), Qgis::WkbType::MultiLineStringZ );
    QCOMPARE( scInterGeom->asWkt(), QStringLiteral( "MULTILINESTRING Z ("
                                                    "(-83368668237302373871208994883224442596414327451/13187253007516584345419860333660204068503552 385244135769194833928359998705288105772518269397/105498024060132674763358882669281632548028416 0/1,"
                                                    "-3694427387541401611431496726449054179068805833/593047719006612426355979511374504815230976 70556612052601521660517929315037655768347058877/18977527008211597643391344363984154087391232 0/1),"
                                                    "(-6392704255083833/1099511627776 17661846043398399/4398046511104 0/1,"
                                                    "-3694427387541401611431496726449054179068805833/593047719006612426355979511374504815230976 70556612052601521660517929315037655768347058877/18977527008211597643391344363984154087391232 0/1)"
                                                    ")" ) );

    const QgsMultiCurve *interCurve = qgsgeometry_cast<const QgsMultiCurve *>( scInterGeom->asQgisGeometry().release() );
    QCOMPARE( interCurve->partCount(), 2 ); // check that the operation created 2 features
    QCOMPARE( interCurve->curveN( 0 )->asWkt( 2 ), u"LineString Z (-6321.91 3651.67 0, -6229.56 3717.9 0)"_s );
    QCOMPARE( interCurve->curveN( 1 )->asWkt( 2 ), u"LineString Z (-5814.13 4015.84 0, -6229.56 3717.9 0)"_s );
  }

  {
    // 1st: prepare geometries
    QgsSfcgalGeometry cylinderGeom( mCylinderWkt );
    QgsSfcgalGeometry cubeGeom( mCubePolyHedWkt );

    // 2nd: intersect cylinder - cube
    std::unique_ptr<QgsSfcgalGeometry> resultGeom = cylinderGeom.intersection( cubeGeom );
    QVERIFY2( resultGeom, "resultGeom is NULL." );
    QCOMPARE( resultGeom->wkbType(), Qgis::WkbType::GeometryCollectionZ );
    QCOMPARE( resultGeom->partCount(), 7 );

    const QgsGeometryCollection *castGeom = qgsgeometry_cast<const QgsGeometryCollection *>( resultGeom->asQgisGeometry().release() );
    QVERIFY( castGeom != nullptr );
    QCOMPARE( castGeom->partCount(), 7 );

    // 3rd: prepare compare
    // load expected wkt
    std::unique_ptr<QgsSfcgalGeometry> readGeom = openWktFile( "intersection3d_cyl_cube.wkt" );
    QVERIFY2( readGeom, "readGeom geometry is NULL." );

    // 4th: check coverage between actual and expected geometry
#if SFCGAL_VERSION >= SFCGAL_MAKE_VERSION( 2, 1, 0 )
    QVERIFY2( readGeom->fuzzyEqual( *resultGeom, 0.1 ), "result geom does not match expected from file" );
#endif
  }

  {
    // 1st: prepare geometries
    QgsSfcgalGeometry cube1( mCube1SolidWkt );
    QgsSfcgalGeometry cube2( mCube2SolidWkt );

    // 2nd: intersection cube1 - cube2
    std::unique_ptr<QgsSfcgalGeometry> resultGeom = cube1.intersection( cube2 );
    //QCOMPARE( resultGeom->wkbType(), Qgis::WkbType::SO );
    QVERIFY2( resultGeom, "resultGeom is NULL." );

    // 3rd: prepare compare
    // load expected wkt
    std::unique_ptr<QgsSfcgalGeometry> readGeom = openWktFile( "intersection3d_cube_cube.wkt" );
    QVERIFY2( readGeom, "readGeom geometry is NULL." );

    // 4th: check coverage between actual and expected geometry
    QVERIFY2( readGeom->covers( *resultGeom ), "result geom does not match expected from file" );
  }
}

void TestQgsSfcgal::unionCheck1()
{
  initPainterTest();
  // should be a multipolygon with 2 parts as A does not intersect C
  auto geomA = std::make_unique<QgsSfcgalGeometry>( mpPolygonGeometryA );

  QVector<QgsAbstractGeometry *> geomList;
  geomList.append( mpPolygonGeometryC.get() );
  std::unique_ptr<QgsSfcgalGeometry> combinedGeom = geomA->combine( geomList );
  QVERIFY2( combinedGeom, "combinedGeom is NULL." );
  QCOMPARE( combinedGeom->wkbType(), Qgis::WkbType::MultiPolygon );

  QVERIFY( combinedGeom->partCount() > 0 ); // check that the union did not fail

  const QgsGeometryCollection *castGeom = qgsgeometry_cast<const QgsGeometryCollection *>( combinedGeom->asQgisGeometry().release() );
  QVERIFY( castGeom != nullptr );

  paintMultiPolygon( castGeom );
  QGSVERIFYIMAGECHECK( "Checking A union C produces 2 polys (SFCGAL)", "geometry_unionCheck1", mImage, QString(), 0 );

  // with Z
  // TODO !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
}

void TestQgsSfcgal::unionCheck2()
{
  initPainterTest();
  // should be a single polygon as A intersect B
  auto geomA = std::make_unique<QgsSfcgalGeometry>( mpPolygonGeometryA );

  QVector<QgsAbstractGeometry *> geomList;
  geomList.append( mpPolygonGeometryB.get() );
  std::unique_ptr<QgsSfcgalGeometry> combinedGeom = geomA->combine( geomList );
  QVERIFY2( combinedGeom, "combinedGeom is NULL." );
  QCOMPARE( combinedGeom->wkbType(), Qgis::WkbType::Polygon );

  QVERIFY( combinedGeom->partCount() > 0 ); // check that the union did not fail

  const QgsPolygon *castGeom = qgsgeometry_cast<const QgsPolygon *>( combinedGeom->asQgisGeometry().release() );
  QVERIFY( castGeom != nullptr );

  paintPolygon( castGeom );
  QGSVERIFYIMAGECHECK( "Checking A union B produces single union poly (SFCGAL)", "geometry_unionCheck2", mImage, QString(), 0 );

  // with Z
  // TODO !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
}

void TestQgsSfcgal::differenceCheck1()
{
  initPainterTest();
  // should be same as A since A does not intersect C so diff is 100% of A
  auto geomA = std::make_unique<QgsSfcgalGeometry>( mpPolygonGeometryA );

  std::unique_ptr<QgsSfcgalGeometry> diffGeom = geomA->difference( mpPolygonGeometryC.constGet() );
  QCOMPARE( diffGeom->wkbType(), Qgis::WkbType::Polygon );

  QVERIFY( diffGeom->partCount() > 0 ); // check that the union did not fail

  const QgsPolygon *castGeom = qgsgeometry_cast<const QgsPolygon *>( diffGeom->asQgisGeometry().release() );
  QVERIFY( castGeom != nullptr );

  paintPolygon( castGeom );
  QGSVERIFYIMAGECHECK( "Checking (A - C) = A", "geometry_differenceCheck1", mImage, QString(), 0 );
}

void TestQgsSfcgal::differenceCheck2()
{
  initPainterTest();
  // should be a single polygon as (A - B) = subset of A
  auto geomA = std::make_unique<QgsSfcgalGeometry>( mpPolygonGeometryA );

  std::unique_ptr<QgsSfcgalGeometry> diffGeom = geomA->difference( mpPolygonGeometryB.constGet() );
  QCOMPARE( diffGeom->wkbType(), Qgis::WkbType::Polygon );

  QVERIFY( diffGeom->partCount() > 0 ); // check that the union did not fail

  const QgsPolygon *castGeom = qgsgeometry_cast<const QgsPolygon *>( diffGeom->asQgisGeometry().release() );
  QVERIFY( castGeom != nullptr );

  paintPolygon( castGeom );
  QGSVERIFYIMAGECHECK( "Checking (A - B) = subset of A", "geometry_differenceCheck2", mImage, QString(), 0 );
}


void TestQgsSfcgal::difference3d()
{
  initPainterTest();

  {
    // 1st: prepare geometries
    QgsSfcgalGeometry cube1( mCube1SolidWkt );
    QgsSfcgalGeometry cube2( mCube2SolidWkt );

    // 2nd: diff cube1 - cube2
    std::unique_ptr<QgsSfcgalGeometry> scDiffGeom = cube1.difference( cube2 );
    QVERIFY2( scDiffGeom, "diffGeom is NULL." );

    // 3rd: prepare compare
    // load expected wkt
    std::unique_ptr<QgsSfcgalGeometry> readGeom = openWktFile( "difference3d_cube_cube.wkt" );
    QVERIFY2( readGeom, "readGeom geometry is NULL." );

    // 4th: check coverage between actual and expected geometry
    QVERIFY2( readGeom->covers( *scDiffGeom ), "diff geom does not match expected from file" );
  }

  {
    // 1st: prepare geometries
    QgsSfcgalGeometry cylinderGeom( mCylinderWkt );
    QgsSfcgalGeometry cubeGeom( mCubePolyHedWkt );

    // 2nd: diff cylinder - cube
    std::unique_ptr<QgsSfcgalGeometry> scDiffGeom = cylinderGeom.difference( cubeGeom );
    QCOMPARE( scDiffGeom->wkbType(), Qgis::WkbType::TINZ );
    QVERIFY2( scDiffGeom, "diffGeom is NULL." );

    const QgsTriangulatedSurface *castGeom = qgsgeometry_cast<const QgsTriangulatedSurface *>( scDiffGeom->asQgisGeometry().release() );
    QVERIFY( castGeom != nullptr );

    // 3rd: prepare compare
    // load expected wkt
    std::unique_ptr<QgsSfcgalGeometry> readGeom = openWktFile( "difference3d_cyl_cube.wkt" );
    QVERIFY2( readGeom, "readGeom geometry is NULL." );

    // 4th: check coverage between actual and expected geometry
    QVERIFY2( readGeom->covers( *scDiffGeom ), "diff geom does not match expected from file" );
  }
}

void TestQgsSfcgal::buffer3DCheck()
{
  QString sfcgalLine2DWkt = "LINESTRING(117.623198 35.198654, 117.581274 35.198654, 117.078178 35.324427, "
                            "116.868555 35.534051, 116.617007 35.869448, 116.491233 35.953297, "
                            "116.155836 36.288694, 116.071987 36.372544, 115.443117 36.749865, "
                            "114.814247 37.043338, 114.311152 37.169112)";
  auto sfcgalLine2D = std::make_unique<QgsSfcgalGeometry>( sfcgalLine2DWkt );

  std::unique_ptr<QgsSfcgalGeometry> sfcgalBuffer3D( sfcgalLine2D->buffer3D( 20.0, 7, Qgis::JoinStyle3D::Round ) );
  QVERIFY2( sfcgalBuffer3D != nullptr, "buffer 3d is NULL." );

  { // read expected from WKT dump with CGAL formalism
    std::unique_ptr<QgsSfcgalGeometry> expectedBuffer = openWktFile( "buffer3d_linestring.wkt" );
    QVERIFY2( expectedBuffer->sfcgalGeometry() != nullptr, "Expected buffer is NULL." );

    bool isOK = expectedBuffer->covers( *sfcgalBuffer3D.get() );
    QVERIFY2( isOK, "buffer3D geom does not match expected from file" );
  }

  { // read expected from WKT dump with 2 decimals
    std::unique_ptr<QgsSfcgalGeometry> expectedBuffer = openWktFile( "buffer3d_linestring_2_deci.wkt" );
    QVERIFY2( expectedBuffer->sfcgalGeometry() != nullptr, "buffer 3d linestring 2 is NULL" );

    // cover fails with decimal dump
    bool isOK = expectedBuffer->covers( *sfcgalBuffer3D.get() );
    QVERIFY2( !isOK, "buffer3D geom matches expected from file, but should not!" );

    // isEquals passes with decimal dump
#if SFCGAL_VERSION >= SFCGAL_MAKE_VERSION( 2, 1, 0 )
    isOK = QgsSfcgalEngine::isEqual( expectedBuffer->sfcgalGeometry().get(), sfcgalBuffer3D->sfcgalGeometry().get(), 0.001 );
    QVERIFY2( isOK, "buffer3D geom does not match expected from file" );
#endif
  }
}

void TestQgsSfcgal::buffer2DCheck()
{
  auto sfcgalLine2D = std::make_unique<QgsSfcgalGeometry>( mpPolygonGeometryA );
  std::unique_ptr<QgsSfcgalGeometry> sfcgalBuffer2D( sfcgalLine2D->buffer2D( 20.0, 7, Qgis::JoinStyle::Round ) );
  QVERIFY2( sfcgalBuffer2D != nullptr, "2D Buffer is NULL" );

  std::unique_ptr<QgsSfcgalGeometry> expectedBuffer = openWktFile( "buffer2d_linestring.wkt" );
  QVERIFY2( expectedBuffer->sfcgalGeometry() != nullptr, "Buffer 2D is NULL." );

  bool isOK = expectedBuffer->covers( *sfcgalBuffer2D.get() );
  QVERIFY2( isOK, "buffer2D geom does not match expected from file" );
}

void TestQgsSfcgal::extrude()
{
  auto sfcgalPolygonA = std::make_unique<QgsSfcgalGeometry>( mSfcgalPolygonZA );
  const QgsVector3D extrusion( 0, 0, 30 );
  std::unique_ptr<QgsSfcgalGeometry> sfcgalExtrusion( sfcgalPolygonA->extrude( extrusion ) );

  std::unique_ptr<QgsSfcgalGeometry> expectedExtrusion = openWktFile( "extrude_polygon_a.wkt" );
  QVERIFY2( expectedExtrusion->covers( *sfcgalExtrusion.get() ), "extrusion geom does not match expected from file" );
}

void TestQgsSfcgal::simplify()
{
  QString wkt( "LINESTRING(1 4, 4 9, 4 12, 4 16, 2 19, -4 20)" );

  QgsSfcgalGeometry sfcgalLinestring2D( wkt );
  QVERIFY2( sfcgalLinestring2D.sfcgalGeometry() != nullptr, "Simplify - input geom is NULL." );
  QCOMPARE( sfcgalLinestring2D.wkbType(), Qgis::WkbType::LineString );

#if SFCGAL_VERSION < SFCGAL_MAKE_VERSION( 2, 1, 0 )
  QVERIFY_EXCEPTION_THROWN( sfcgalLinestring2D.simplify( 5, false ), QgsNotSupportedException );
#else
  std::unique_ptr<QgsSfcgalGeometry> simplifiedGeom( sfcgalLinestring2D.simplify( 5, false ) );
  QVERIFY2( simplifiedGeom->sfcgalGeometry() != nullptr, "Simplify is NULL" );
  QCOMPARE( simplifiedGeom->asWkt( 0 ), "LINESTRING (1 4,2 19,-4 20)" );
#endif
}

void TestQgsSfcgal::approximateMedialAxis()
{
  QString wkt( "POLYGON ((0 5,1.5 4.8,3 4.2,4 3,4.6 1.5,4.8 0.5,4.5 -0.3,4 -1.2,3.2 -2.4,2.5 -3.5,1.3 -4.3,0 -4.8,-1.8 -4.6,-3.4 -3.9,-4.2 -2.8,-4.7 -1.5,-4.8 -0.2,-4.5 0.9,-3.7 2,-2.8 3.1,-1.4 4.2,0 5))" );

  QgsSfcgalGeometry sfcgalPolygon( wkt );
  QVERIFY2( sfcgalPolygon.sfcgalGeometry() != nullptr, "Approximate medial axis is NULL" );
  QCOMPARE( sfcgalPolygon.wkbType(), Qgis::WkbType::Polygon );

  std::unique_ptr<QgsSfcgalGeometry> simplifiedGeom( sfcgalPolygon.approximateMedialAxis() );
  QVERIFY2( simplifiedGeom->sfcgalGeometry() != nullptr, sfcgal::errorHandler()->getFullText().toStdString().c_str() );
  QCOMPARE( simplifiedGeom->asWkt( 2 ), "MULTILINESTRING ((2.34 0.70,2.05 0.74),(2.05 0.74,0.67 0.83),(-1.57 -0.51,-1.01 -0.67),(0.66 1.61,0.70 1.08),(-1.21 -1.17,-0.77 -0.76),(-1.01 -0.67,-0.73 -0.70),(0.70 1.08,0.71 1.04),(0.71 1.04,0.71 1.04),(0.71 1.04,0.65 0.83),(0.67 0.83,0.65 0.83),(0.65 0.83,0.05 0.19),(-0.52 -0.79,-0.56 -0.63),(-0.77 -0.76,-0.73 -0.70),(-0.73 -0.70,-0.56 -0.63),(0.05 0.19,-0.21 -0.18),(-0.21 -0.18,-0.45 -0.51),(-0.56 -0.63,-0.50 -0.57),(-0.45 -0.51,-0.50 -0.57))" );

  // medial axis does not work on an invalid geometry
  QString invalid_polygon_wkt( "POLYGON((0 0, 4 0, 4 4, 0 4, 0 0),(4 2, 5 2, 5 3, 4 3, 4 2))" );
  QgsSfcgalGeometry sfcgalPoint( invalid_polygon_wkt );
  QVERIFY_EXCEPTION_THROWN( sfcgalPoint.approximateMedialAxis(), QgsSfcgalException );
}

void TestQgsSfcgal::primitiveCube()
{
#if SFCGAL_VERSION >= SFCGAL_MAKE_VERSION( 2, 3, 0 )
  std::unique_ptr<QgsSfcgalGeometry> cube = QgsSfcgalGeometry::createCube( 5 );
  QCOMPARE( cube->wkbType(), Qgis::WkbType::PolyhedralSurfaceZ );
  QCOMPARE( cube->geometryType(), "cube" );

  // check clone
  std::unique_ptr<QgsSfcgalGeometry> cube2 = cube->clone();
  QVERIFY( *cube == *cube2 );

  // check export as SFCGAL geometry
  std::unique_ptr<QgsSfcgalGeometry> poly = cube->primitiveAsPolyhedralSurface();
  QString expPolyWkt = "POLYHEDRALSURFACE Z (((0.0 0.0 0.0,0.0 5.0 0.0,5.0 5.0 0.0,5.0 0.0 0.0,0.0 0.0 0.0)),"
                       "((0.0 0.0 5.0,5.0 0.0 5.0,5.0 5.0 5.0,0.0 5.0 5.0,0.0 0.0 5.0)),"
                       "((0.0 0.0 0.0,5.0 0.0 0.0,5.0 0.0 5.0,0.0 0.0 5.0,0.0 0.0 0.0)),"
                       "((0.0 5.0 0.0,0.0 5.0 5.0,5.0 5.0 5.0,5.0 5.0 0.0,0.0 5.0 0.0)),"
                       "((5.0 0.0 0.0,5.0 5.0 0.0,5.0 5.0 5.0,5.0 0.0 5.0,5.0 0.0 0.0)),"
                       "((0.0 0.0 0.0,0.0 0.0 5.0,0.0 5.0 5.0,0.0 5.0 0.0,0.0 0.0 0.0)))";
  QCOMPARE( poly->asWkt( 1 ), expPolyWkt );

  // check export as QgsAbstractGeometry
  std::unique_ptr<QgsAbstractGeometry> qgsGeom = cube->asQgisGeometry();
  QCOMPARE( qgsGeom->asWkt( 1 ), "PolyhedralSurface Z (((0 0 0, 0 5 0, 5 5 0, 5 0 0, 0 0 0)),"
                                 "((0 0 5, 5 0 5, 5 5 5, 0 5 5, 0 0 5)),"
                                 "((0 0 0, 5 0 0, 5 0 5, 0 0 5, 0 0 0)),"
                                 "((0 5 0, 0 5 5, 5 5 5, 5 5 0, 0 5 0)),"
                                 "((5 0 0, 5 5 0, 5 5 5, 5 0 5, 5 0 0)),"
                                 "((0 0 0, 0 0 5, 0 5 5, 0 5 0, 0 0 0)))" );

  // check compare
  std::unique_ptr<QgsSfcgalGeometry> env = cube->envelope();
  QVERIFY( *env != *cube );
  QCOMPARE_NE( env->asWkt( 1 ), expPolyWkt ); // slight change in polygon order but...
  QVERIFY( *env == *poly );                   // ...they are almost equal

  // check translate
  std::unique_ptr<QgsSfcgalGeometry> cubeT = cube->translate( { 1.0, 2.0, 3.0 } );
  QCOMPARE( cubeT->primitiveTransform(), QMatrix4x4( 1.0, 0.0, 0.0, 1.0, //
                                                     0.0, 1.0, 0.0, 2.0, //
                                                     0.0, 0.0, 1.0, 3.0, //
                                                     0.0, 0.0, 0.0, 1.0 ) );
  QCOMPARE( cubeT->asWkt( 0 ), "POLYHEDRALSURFACE Z (((1 2 3,1 3 3,2 3 3,2 2 3,1 2 3)),"
                               "((1 2 4,2 2 4,2 3 4,1 3 4,1 2 4)),"
                               "((1 2 3,2 2 3,2 2 4,1 2 4,1 2 3)),"
                               "((1 3 3,1 3 4,2 3 4,2 3 3,1 3 3)),"
                               "((2 2 3,2 3 3,2 3 4,2 2 4,2 2 3)),"
                               "((1 2 3,1 2 4,1 3 4,1 3 3,1 2 3)))" );

  // check rotate
  std::unique_ptr<QgsSfcgalGeometry> cubeR = cube->rotate3D( 90, { 0.0, 0.0, 1.0 }, { 0.0, 0.0, 0.0 } );
  QVERIFY( qFuzzyCompare2( cubeR->primitiveTransform(), QMatrix4x4( -1.0e-07, -1.0, 0.0, 0.0, //
                                                                    1.0, -1.0e-07, 0.0, 0.0,  //
                                                                    0.0, 0.0, 1.0, 0.0,       //
                                                                    0.0, 0.0, 0.0, 1.0 ) ) );
  cubeR = cube->rotate3D( 90, { 0.0, 0.0, 1.0 }, { 1.0, 2.0, 3.0 } );
  QVERIFY( qFuzzyCompare2( cubeR->primitiveTransform(), QMatrix4x4( -1.0e-07, -1.0, 0.0, -3.0, //
                                                                    1.0, -1.0e-07, 0.0, -1.0,  //
                                                                    0.0, 0.0, 1.0, 0.0,        //
                                                                    0.0, 0.0, 0.0, 1.0 ) ) );
  QCOMPARE( cubeR->asWkt( 0 ), "POLYHEDRALSURFACE Z (((-3 -1 0,-4 -1 0,-4 0 0,-3 0 0,-3 -1 0)),"
                               "((-3 -1 1,-3 0 1,-4 0 1,-4 -1 1,-3 -1 1)),"
                               "((-3 -1 0,-3 0 0,-3 0 1,-3 -1 1,-3 -1 0)),"
                               "((-4 -1 0,-4 -1 1,-4 0 1,-4 0 0,-4 -1 0)),"
                               "((-3 0 0,-4 0 0,-4 0 1,-3 0 1,-3 0 0)),"
                               "((-3 -1 0,-3 -1 1,-4 -1 1,-4 -1 0,-3 -1 0)))" );

  // check scale
  std::unique_ptr<QgsSfcgalGeometry> cubeS = cube->scale( { 1.0, 2.0, 3.0 }, { 0.0, 0.0, 0.0 } );
  QCOMPARE( cubeS->primitiveTransform(), QMatrix4x4( 1.0, 0.0, 0.0, 0.0, //
                                                     0.0, 2.0, 0.0, 0.0, //
                                                     0.0, 0.0, 3.0, 0.0, //
                                                     0.0, 0.0, 0.0, 1.0 ) );
  cubeS = cube->scale( { 1.0, 2.0, 3.0 }, { 1.0, 2.0, 3.0 } );
  QCOMPARE( cubeS->primitiveTransform(), QMatrix4x4( 1.0, 0.0, 0.0, 0.0, //
                                                     0.0, 2.0, 0.0, 2.0, //
                                                     0.0, 0.0, 3.0, 6.0, //
                                                     0.0, 0.0, 0.0, 1.0 ) );
  QCOMPARE( cubeS->asWkt( 0 ), "POLYHEDRALSURFACE Z (((0 2 6,0 4 6,1 4 6,1 2 6,0 2 6)),"
                               "((0 2 9,1 2 9,1 4 9,0 4 9,0 2 9)),"
                               "((0 2 6,1 2 6,1 2 9,0 2 9,0 2 6)),"
                               "((0 4 6,0 4 9,1 4 9,1 4 6,0 4 6)),"
                               "((1 2 6,1 4 6,1 4 9,1 2 9,1 2 6)),"
                               "((0 2 6,0 2 9,0 4 9,0 4 6,0 2 6)))" );

  // check volume
  QCOMPARE( cube->volume(), 125.0 );
  // check area
  QCOMPARE( cube->area(), 150.0 );

  // check parameters
  QList<std::pair<QString, QString>> params = cube->primitiveParameters();
  QCOMPARE( params.size(), 1 );
  QCOMPARE( params.at( 0 ).first, "size" );
  QCOMPARE( params.at( 0 ).second, "double" );

  QVariant param = cube->primitiveParameter( "size" );
  QCOMPARE( param.toDouble(), 5.0 );

  cube->primitiveSetParameter( "size", 8.2 );
  param = cube->primitiveParameter( "size" );
  QCOMPARE( param.toDouble(), 8.2 );

#endif
}

QGSTEST_MAIN( TestQgsSfcgal )
#include "testqgssfcgal.moc"

#endif
