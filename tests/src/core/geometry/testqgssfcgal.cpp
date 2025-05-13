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
#include "qgstest.h"
#include <cmath>
#include <memory>
#include <limits>
#include <QObject>
#include <QString>
#include <QStringList>
#include <QApplication>
#include <QFileInfo>
#include <QDir>
#include <QDesktopServices>
#include <QVector>
#include <QPointF>
#include <QImage>
#include <QPainter>

// qgis includes...
#include <qgsapplication.h>
#include "qgscompoundcurve.h"
#include <qgsgeometry.h>
#include "qgspoint.h"
#include "qgslinestring.h"
#include "qgspolygon.h"
#include "qgstriangle.h"
#include "qgsgeometryengine.h"
#include "qgscircle.h"
#include "qgsmultipoint.h"
#include "qgsmultilinestring.h"
#include "qgsmultipolygon.h"
#include "qgscircularstring.h"
#include "qgsgeometrycollection.h"
#include "qgsgeometryfactory.h"
#include "qgscurvepolygon.h"
#include "qgsgeos.h"
#include "qgsreferencedgeometry.h"

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

  private:
    //! Must be called before each render test
    void initPainterTest();

    void paintMultiPolygon( QgsMultiPolygonXY &multiPolygon );
    void paintPolygon( QgsPolygonXY &polygon );

    void paintMultiPolygon( const QgsGeometryCollection *multiPolygon );
    void paintPolygon( const QgsPolygon *polygon );
    void paintCurve( const QgsCurve *curve );

    std::unique_ptr<QgsSfcgalGeometry> openWktFile( const QString &wktFile, QString *errorMsg );

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
};

TestQgsSfcgal::TestQgsSfcgal()
  : QgsTest( QStringLiteral( "Geometry Tests" ) )
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

std::unique_ptr<QgsSfcgalGeometry> TestQgsSfcgal::openWktFile( const QString &wktFile, QString *errorMsg )
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
  sfcgal::shared_geom expectedGeom = QgsSfcgalEngine::fromWkt( expectedStr, errorMsg );
  if ( !expectedGeom || !errorMsg->isEmpty() )
  {
    qWarning() << "expected geometry is NULL: " << sfcgal::errorHandler()->getFullText().toStdString().c_str();
    return nullptr;
  }
  return QgsSfcgalEngine::toSfcgalGeometry( expectedGeom, errorMsg );
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
  QString errorMsg; // used to retrieve failure messages if any
  sfcgal::shared_geom expectedGeom = QgsSfcgalEngine::fromWkt( "Point Z (-5673.79 3594.8 20, 5)", &errorMsg );
  QVERIFY2( !errorMsg.isEmpty(), "Should have failed" );

  QgsSfcgalGeometry geom;
  bool result = geom.fromWkt( "Point Z (-5673.79 3594.8 20, 5)" );
  QVERIFY( !result );
  QVERIFY( !sfcgal::errorHandler()->isTextEmpty() );
  QVERIFY( sfcgal::errorHandler()->getFullText().contains( "SFCGAL error occurred: WKT parse error" ) );
}

void TestQgsSfcgal::isEqual()
{
  initPainterTest();

  QString errorMsg;
  QgsSfcgalGeometry geomA( mpPolygonGeometryA );

  // test with cloned geometry
  sfcgal::shared_geom tempCloneGeomA = QgsSfcgalEngine::cloneGeometry( geomA.sfcgalGeometry().get(), &errorMsg );
  std::unique_ptr<QgsSfcgalGeometry> cloneGeomA = QgsSfcgalEngine::toSfcgalGeometry( tempCloneGeomA, &errorMsg );
  QVERIFY2( errorMsg.isEmpty(), errorMsg.toStdString().c_str() );

  QCOMPARE( geomA.compareTo( cloneGeomA.get() ), 0 );

  // test with offset geometry
  QgsPoint vector( 1.0, 1.0 );
  std::unique_ptr<QgsSfcgalGeometry> geomB( geomA.translate( vector, &errorMsg ) );
  QVERIFY2( errorMsg.isEmpty(), errorMsg.toStdString().c_str() );

  // should failed
  QCOMPARE( geomA.compareTo( geomB.get() ), -1 );

  // should be accepted
  QVERIFY2( geomA.fuzzyEqual( *( geomB.get() ), 0.05 ), errorMsg.toStdString().c_str() );
  QVERIFY2( errorMsg.isEmpty(), errorMsg.toStdString().c_str() );
}

void TestQgsSfcgal::boundary()
{
  QString errorMsg;

  // 2D line
  std::unique_ptr<QgsSfcgalGeometry> sfcgalLine2D = std::make_unique<QgsSfcgalGeometry>( mpPolylineGeometryD );
  std::unique_ptr<QgsSfcgalGeometry> sfcgalLine2DBoundary( dynamic_cast<QgsSfcgalGeometry *>( sfcgalLine2D->boundary() ) );

  QCOMPARE( sfcgalLine2DBoundary->asWkt(), "MULTIPOINT ((517312295588795/4398046511104 4953770157448219/140737488355328),"
                                           "(45464789865619/549755813888 5295991515520197/140737488355328))" );
  // 3D polygon
  std::unique_ptr<QgsSfcgalGeometry> sfcgalPolygon3D = std::make_unique<QgsSfcgalGeometry>( mSfcgalPolygonZA );
  std::unique_ptr<QgsSfcgalGeometry> sfcgalPolygon3DBoundary( dynamic_cast<QgsSfcgalGeometry *>( sfcgalPolygon3D->boundary() ) );

  std::unique_ptr<QgsSfcgalGeometry> expectedPolygon3DBoundary = openWktFile( "boundary_polygon3d.wkt", &errorMsg );
  QVERIFY2( errorMsg.isEmpty(), errorMsg.toStdString().c_str() );
  QCOMPARE( sfcgalPolygon3DBoundary->asWkt(), expectedPolygon3DBoundary->asWkt() );
}

void TestQgsSfcgal::centroid()
{
  QString errorMsg;
  std::unique_ptr<QgsSfcgalGeometry> geomZA = std::make_unique<QgsSfcgalGeometry>( mSfcgalPolygonZA );
  std::unique_ptr<QgsSfcgalGeometry> geomZB = std::make_unique<QgsSfcgalGeometry>( mSfcgalPolygonZB );
  std::unique_ptr<QgsSfcgalGeometry> geomZC = std::make_unique<QgsSfcgalGeometry>( mSfcgalPolygonZC );

  std::unique_ptr<QgsPoint> res = std::make_unique<QgsPoint>( geomZA->centroid( &errorMsg ) );
  QVERIFY2( errorMsg.isEmpty(), sfcgal::errorHandler()->getFullText().toStdString().c_str() );
  QCOMPARE( res->asWkt( 2 ), QStringLiteral( "Point Z (-5673.79 3594.8 20)" ) );

  res = std::make_unique<QgsPoint>( geomZB->centroid( &errorMsg ) );
  QVERIFY2( errorMsg.isEmpty(), sfcgal::errorHandler()->getFullText().toStdString().c_str() );
  QCOMPARE( res->asWkt( 2 ), QStringLiteral( "Point Z (-5150.77 3351.12 0)" ) );

  res = std::make_unique<QgsPoint>( geomZC->centroid( &errorMsg ) );
  QVERIFY2( errorMsg.isEmpty(), sfcgal::errorHandler()->getFullText().toStdString().c_str() );
  QCOMPARE( res->asWkt( 2 ), QStringLiteral( "Point Z (-6734.85 4471.67 -28.34)" ) );
}

void TestQgsSfcgal::dropZ()
{
  QString errorMsg;

  // PolygonZ
  std::unique_ptr<QgsSfcgalGeometry> sfcgalPolygonZ = std::make_unique<QgsSfcgalGeometry>( QgsGeometry( mSfcgalPolygonZA ) );
  QCOMPARE( sfcgalPolygonZ->wkbType(), Qgis::WkbType::PolygonZ );
  QVERIFY( sfcgalPolygonZ->dropZValue() );
  QVERIFY2( errorMsg.isEmpty(), sfcgal::errorHandler()->getFullText().toStdString().c_str() );
  QCOMPARE( sfcgalPolygonZ->asWkt( 1 ), "POLYGON ((-7037.2 4814.3,-7112.3 3024.3,-4669.8 2770.2,-3428.3 3278.3,-7037.2 4814.3),(-6832.9 4498.3,-3923.7 3302.1,-4717.1 2947.5,-6863.0 3193.9,-6832.9 4498.3))" );
  QCOMPARE( sfcgalPolygonZ->wkbType(), Qgis::WkbType::Polygon );
  QVERIFY( !sfcgalPolygonZ->dropZValue() );

  // Polygon2D
  std::unique_ptr<QgsSfcgalGeometry> sfcgalPolygon2D = std::make_unique<QgsSfcgalGeometry>( QgsGeometry::fromPolygonXY( mPolygonA ) );
  QCOMPARE( sfcgalPolygon2D->wkbType(), Qgis::WkbType::Polygon );
  QVERIFY( !sfcgalPolygon2D->dropZValue() );
  QVERIFY2( errorMsg.isEmpty(), sfcgal::errorHandler()->getFullText().toStdString().c_str() );
  QCOMPARE( sfcgalPolygon2D->wkbType(), Qgis::WkbType::Polygon );

  // PolygonM
  std::unique_ptr<QgsAbstractGeometry> emptyGeomM( nullptr );
  QgsSfcgalGeometry sfcgalPolygonM( emptyGeomM, QgsSfcgalEngine::fromWkt( "POLYGON M ((0 0 1, 20 0 2, 20 10 3, 0 10 4, 0 0 1))", &errorMsg ) );
  QCOMPARE( sfcgalPolygonM.wkbType(), Qgis::WkbType::PolygonM );
  QVERIFY( !sfcgalPolygonM.dropZValue() );
  QVERIFY2( errorMsg.isEmpty(), sfcgal::errorHandler()->getFullText().toStdString().c_str() );
  QCOMPARE( sfcgalPolygonM.wkbType(), Qgis::WkbType::PolygonM );

  // PolygonZM
  std::unique_ptr<QgsAbstractGeometry> emptyGeomZM( nullptr );
  QgsSfcgalGeometry sfcgalPolygonZM( emptyGeomM, QgsSfcgalEngine::fromWkt( "POLYGON ZM ((0 0 1 2, 20 0 2 2, 20 10 3 2, 0 10 4 2, 0 0 1 2))", &errorMsg ) );
  QVERIFY2( errorMsg.isEmpty(), sfcgal::errorHandler()->getFullText().toStdString().c_str() );
  QCOMPARE( sfcgalPolygonZM.wkbType(), Qgis::WkbType::PolygonZM );
  QVERIFY( sfcgalPolygonZM.dropZValue() );
  QCOMPARE( sfcgalPolygonZM.asWkt( 1 ), "POLYGON M ((0.0 0.0 2.0,20.0 0.0 2.0,20.0 10.0 2.0,0.0 10.0 2.0,0.0 0.0 2.0))" );
  QVERIFY2( errorMsg.isEmpty(), sfcgal::errorHandler()->getFullText().toStdString().c_str() );
  QCOMPARE( sfcgalPolygonZM.wkbType(), Qgis::WkbType::PolygonM );
  QVERIFY( !sfcgalPolygonZM.dropZValue() );
}

void TestQgsSfcgal::dropM()
{
  QString errorMsg;

  // PolygonZ
  std::unique_ptr<QgsSfcgalGeometry> sfcgalPolygonZ = std::make_unique<QgsSfcgalGeometry>( QgsGeometry( mSfcgalPolygonZA ) );
  QCOMPARE( sfcgalPolygonZ->wkbType(), Qgis::WkbType::PolygonZ );
  QVERIFY( !sfcgalPolygonZ->dropMValue() );
  QVERIFY2( errorMsg.isEmpty(), sfcgal::errorHandler()->getFullText().toStdString().c_str() );
  QCOMPARE( sfcgalPolygonZ->wkbType(), Qgis::WkbType::PolygonZ );

  // Polygon2D
  std::unique_ptr<QgsSfcgalGeometry> sfcgalPolygon2D = std::make_unique<QgsSfcgalGeometry>( QgsGeometry::fromPolygonXY( mPolygonA ) );
  QCOMPARE( sfcgalPolygon2D->wkbType(), Qgis::WkbType::Polygon );
  QVERIFY( !sfcgalPolygon2D->dropMValue() );
  QVERIFY2( errorMsg.isEmpty(), sfcgal::errorHandler()->getFullText().toStdString().c_str() );
  QCOMPARE( sfcgalPolygon2D->wkbType(), Qgis::WkbType::Polygon );

  // PolygonM
  std::unique_ptr<QgsAbstractGeometry> emptyGeomM( nullptr );
  QgsSfcgalGeometry sfcgalPolygonM( emptyGeomM, QgsSfcgalEngine::fromWkt( "POLYGON M ((0 0 1, 20 0 2, 20 10 3, 0 10 4, 0 0 1))", &errorMsg ) );
  QCOMPARE( sfcgalPolygonM.wkbType(), Qgis::WkbType::PolygonM );
  QVERIFY( sfcgalPolygonM.dropMValue() );
  QCOMPARE( sfcgalPolygonM.asWkt( 1 ), "POLYGON ((0.0 0.0,20.0 0.0,20.0 10.0,0.0 10.0,0.0 0.0))" );
  QVERIFY2( errorMsg.isEmpty(), sfcgal::errorHandler()->getFullText().toStdString().c_str() );
  QCOMPARE( sfcgalPolygonM.wkbType(), Qgis::WkbType::Polygon );

  // PolygonZM
  std::unique_ptr<QgsAbstractGeometry> emptyGeomZM( nullptr );
  QgsSfcgalGeometry sfcgalPolygonZM( emptyGeomM, QgsSfcgalEngine::fromWkt( "POLYGON ZM ((0 0 1 2, 20 0 2 2, 20 10 3 2, 0 10 4 2, 0 0 1 2))", &errorMsg ) );
  QVERIFY2( errorMsg.isEmpty(), sfcgal::errorHandler()->getFullText().toStdString().c_str() );
  QCOMPARE( sfcgalPolygonZM.wkbType(), Qgis::WkbType::PolygonZM );
  QVERIFY( sfcgalPolygonZM.dropMValue() );
  QCOMPARE( sfcgalPolygonZM.asWkt( 1 ), "POLYGON Z ((0.0 0.0 1.0,20.0 0.0 2.0,20.0 10.0 3.0,0.0 10.0 4.0,0.0 0.0 1.0))" );
  QVERIFY2( errorMsg.isEmpty(), sfcgal::errorHandler()->getFullText().toStdString().c_str() );
  QCOMPARE( sfcgalPolygonZM.wkbType(), Qgis::WkbType::PolygonZ );
  QVERIFY( !sfcgalPolygonZM.dropMValue() );
}

void TestQgsSfcgal::addZValue()
{
  QString errorMsg;
  std::unique_ptr<QgsAbstractGeometry> emptyGeom( nullptr );

  // 2D Point
  QgsSfcgalGeometry sfcgalPoint2D( emptyGeom, QgsSfcgalEngine::fromWkt( "POINT (4 2)", &errorMsg ) );
  QCOMPARE( sfcgalPoint2D.wkbType(), Qgis::WkbType::Point );
  QVERIFY( sfcgalPoint2D.addZValue( 4 ) );
  QVERIFY2( errorMsg.isEmpty(), sfcgal::errorHandler()->getFullText().toStdString().c_str() );
  QCOMPARE( sfcgalPoint2D.asWkt( 1 ), "POINT Z (4.0 2.0 4.0)" );
  QCOMPARE( sfcgalPoint2D.wkbType(), Qgis::WkbType::PointZ );
  QVERIFY( !sfcgalPoint2D.addZValue() );

  // PolygonM
  QgsSfcgalGeometry sfcgalPolygonM( emptyGeom, QgsSfcgalEngine::fromWkt( "POLYGON M ((0 0 1, 20 0 2, 20 10 3, 0 10 4, 0 0 1))", &errorMsg ) );
  QCOMPARE( sfcgalPolygonM.wkbType(), Qgis::WkbType::PolygonM );
  QVERIFY( sfcgalPolygonM.addZValue() );
  QVERIFY2( errorMsg.isEmpty(), sfcgal::errorHandler()->getFullText().toStdString().c_str() );
  QCOMPARE( sfcgalPolygonM.wkbType(), Qgis::WkbType::PolygonZM );
  QCOMPARE( sfcgalPolygonM.asWkt( 1 ), "POLYGON ZM ((0.0 0.0 0.0 1.0,20.0 0.0 0.0 2.0,20.0 10.0 0.0 3.0,0.0 10.0 0.0 4.0,0.0 0.0 0.0 1.0))" );
  QVERIFY( !sfcgalPolygonM.addZValue() );

  // LineZ
  QgsSfcgalGeometry sfcgalLineZ( emptyGeom, QgsSfcgalEngine::fromWkt( "LINESTRING Z (40 80 2, 40 40 2, 80 40 2, 80 80 2, 40 80 2)", &errorMsg ) );
  QVERIFY2( errorMsg.isEmpty(), sfcgal::errorHandler()->getFullText().toStdString().c_str() );
  QCOMPARE( sfcgalLineZ.wkbType(), Qgis::WkbType::LineStringZ );
  QVERIFY( !sfcgalLineZ.addZValue() );
}

void TestQgsSfcgal::addMValue()
{
  QString errorMsg;
  std::unique_ptr<QgsAbstractGeometry> emptyGeom( nullptr );

  // 2D Point
  QgsSfcgalGeometry sfcgalPoint2D( emptyGeom, QgsSfcgalEngine::fromWkt( "POINT (4 2)", &errorMsg ) );
  QCOMPARE( sfcgalPoint2D.wkbType(), Qgis::WkbType::Point );
  QVERIFY( sfcgalPoint2D.addMValue( 5 ) );
  QVERIFY2( errorMsg.isEmpty(), sfcgal::errorHandler()->getFullText().toStdString().c_str() );
  QCOMPARE( sfcgalPoint2D.asWkt( 1 ), "POINT M (4.0 2.0 5.0)" );
  QCOMPARE( sfcgalPoint2D.wkbType(), Qgis::WkbType::PointM );
  QVERIFY( !sfcgalPoint2D.addMValue() );

  // PolygonM
  QgsSfcgalGeometry sfcgalPolygonM( emptyGeom, QgsSfcgalEngine::fromWkt( "POLYGON M ((0 0 1, 20 0 2, 20 10 3, 0 10 4, 0 0 1))", &errorMsg ) );
  QCOMPARE( sfcgalPolygonM.wkbType(), Qgis::WkbType::PolygonM );
  QVERIFY( !sfcgalPolygonM.addMValue() );

  // LineZ
  QgsSfcgalGeometry sfcgalLineZ( emptyGeom, QgsSfcgalEngine::fromWkt( "LINESTRING Z (40 80 2, 40 40 2, 80 40 2, 80 80 2, 40 80 2)", &errorMsg ) );
  QVERIFY2( errorMsg.isEmpty(), sfcgal::errorHandler()->getFullText().toStdString().c_str() );
  QCOMPARE( sfcgalLineZ.wkbType(), Qgis::WkbType::LineStringZ );
  QVERIFY( sfcgalLineZ.addMValue() );
  QCOMPARE( sfcgalLineZ.asWkt( 1 ), "LINESTRING ZM (40.0 80.0 2.0 0.0,40.0 40.0 2.0 0.0,80.0 40.0 2.0 0.0,80.0 80.0 2.0 0.0,40.0 80.0 2.0 0.0)" );
  QCOMPARE( sfcgalLineZ.wkbType(), Qgis::WkbType::LineStringZM );
  QVERIFY( !sfcgalLineZ.addMValue() );
}

void TestQgsSfcgal::swapXY()
{
  QString errorMsg;
  std::unique_ptr<QgsAbstractGeometry> emptyGeom( nullptr );

  // 2D Point
  QgsSfcgalGeometry sfcgalPoint2D( emptyGeom, QgsSfcgalEngine::fromWkt( "POINT (4 2)", &errorMsg ) );
  QCOMPARE( sfcgalPoint2D.wkbType(), Qgis::WkbType::Point );
  sfcgalPoint2D.swapXy();
  QVERIFY2( errorMsg.isEmpty(), sfcgal::errorHandler()->getFullText().toStdString().c_str() );
  QCOMPARE( sfcgalPoint2D.asWkt( 1 ), "POINT (2.0 4.0)" );

  // PolygonM
  QgsSfcgalGeometry sfcgalPolygonM( emptyGeom, QgsSfcgalEngine::fromWkt( "POLYGON M ((1 0 1, 20 0 2, 20 10 3, 0 10 4, 1 0 1))", &errorMsg ) );
  QVERIFY2( errorMsg.isEmpty(), sfcgal::errorHandler()->getFullText().toStdString().c_str() );
  QCOMPARE( sfcgalPolygonM.wkbType(), Qgis::WkbType::PolygonM );
  sfcgalPolygonM.swapXy();
  QVERIFY2( errorMsg.isEmpty(), sfcgal::errorHandler()->getFullText().toStdString().c_str() );
  QCOMPARE( sfcgalPolygonM.asWkt( 1 ), "POLYGON M ((0.0 1.0 1.0,0.0 20.0 2.0,10.0 20.0 3.0,10.0 0.0 4.0,0.0 1.0 1.0))" );

  // LineZ
  QgsSfcgalGeometry sfcgalLineZ( emptyGeom, QgsSfcgalEngine::fromWkt( "LINESTRING Z (40 80 2, 40 40 2, 80 40 2, 80 80 2, 40 80 2)", &errorMsg ) );
  QVERIFY2( errorMsg.isEmpty(), sfcgal::errorHandler()->getFullText().toStdString().c_str() );
  QCOMPARE( sfcgalLineZ.wkbType(), Qgis::WkbType::LineStringZ );
  sfcgalLineZ.swapXy();
  QCOMPARE( sfcgalLineZ.asWkt( 1 ), "LINESTRING Z (80.0 40.0 2.0,40.0 40.0 2.0,40.0 80.0 2.0,80.0 80.0 2.0,80.0 40.0 2.0)" );
}

void TestQgsSfcgal::isEmpty()
{
  QString errorMsg;

  std::unique_ptr<QgsSfcgalGeometry> sfcgalGeomZA = std::make_unique<QgsSfcgalGeometry>( mSfcgalPolygonZA );
  QVERIFY( !sfcgalGeomZA->isEmpty() );

  std::unique_ptr<QgsAbstractGeometry> emptyGeomM( nullptr );
  QgsSfcgalGeometry sfcgalGeomEmpty( emptyGeomM, QgsSfcgalEngine::fromWkt( "POLYGON EMPTY", &errorMsg ) );
  QVERIFY2( errorMsg.isEmpty(), sfcgal::errorHandler()->getFullText().toStdString().c_str() );
  QVERIFY( sfcgalGeomEmpty.isEmpty() );
}

QGSTEST_MAIN( TestQgsSfcgal )
#include "testqgssfcgal.moc"
