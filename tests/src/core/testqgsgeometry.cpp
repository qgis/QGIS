/***************************************************************************
     testqgsgeometry.cpp
     --------------------------------------
    Date                 : 20 Jan 2008
    Copyright            : (C) 2008 by Tim Sutton
    Email                : tim @ linfiniti.com
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
#include <QStringList>
#include <QApplication>
#include <QFileInfo>
#include <QDir>
#include <QDesktopServices>
#include <QVector>
#include <QPointF>
#include <QImage>
#include <QPainter>

//qgis includes...
#include <qgsapplication.h>
#include <qgsgeometry.h>
#include <qgspoint.h>
#include "qgspointv2.h"
#include "qgslinestringv2.h"

//qgs unit test utility class
#include "qgsrenderchecker.h"

/** \ingroup UnitTests
 * This is a unit test for the different geometry operations on vector features.
 */
class TestQgsGeometry : public QObject
{
    Q_OBJECT

  public:
    TestQgsGeometry();

  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void init();// will be called before each testfunction is executed.
    void cleanup();// will be called after every testfunction.
    void copy();
    void assignment();
    void asVariant(); //test conversion to and from a QVariant
    void isEmpty();
    void pointV2(); //test QgsPointV2
    void lineStringV2(); //test QgsLineStringV2

    void fromQgsPoint();
    void fromQPoint();
    void fromQPolygonF();
    void asQPointF();
    void asQPolygonF();

    void comparePolylines();
    void comparePolygons();

    // MK, Disabled 14.11.2014
    // Too unclear what exactly should be tested and which variations are allowed for the line
#if 0
    void simplifyCheck1();
#endif

    void intersectionCheck1();
    void intersectionCheck2();
    void translateCheck1();
    void rotateCheck1();
    void unionCheck1();
    void unionCheck2();
    void differenceCheck1();
    void differenceCheck2();
    void bufferCheck();
    void smoothCheck();

    void dataStream();

    void exportToGeoJSON();

  private:
    /** A helper method to do a render check to see if the geometry op is as expected */
    bool renderCheck( const QString& theTestName, const QString& theComment = "", int mismatchCount = 0 );
    /** A helper method to dump to qdebug the geometry of a multipolygon */
    void dumpMultiPolygon( QgsMultiPolygon &theMultiPolygon );
    /** A helper method to dump to qdebug the geometry of a polygon */
    void dumpPolygon( QgsPolygon &thePolygon );
    /** A helper method to dump to qdebug the geometry of a polyline */
    void dumpPolyline( QgsPolyline &thePolyline );

    QString elemToString( const QDomElement& elem ) const;

    QgsPoint mPoint1;
    QgsPoint mPoint2;
    QgsPoint mPoint3;
    QgsPoint mPoint4;
    QgsPoint mPointA;
    QgsPoint mPointB;
    QgsPoint mPointC;
    QgsPoint mPointD;
    QgsPoint mPointW;
    QgsPoint mPointX;
    QgsPoint mPointY;
    QgsPoint mPointZ;
    QgsPolyline mPolylineA;
    QgsPolyline mPolylineB;
    QgsPolyline mPolylineC;
    QgsGeometry * mpPolylineGeometryD;
    QgsPolygon mPolygonA;
    QgsPolygon mPolygonB;
    QgsPolygon mPolygonC;
    QgsGeometry * mpPolygonGeometryA;
    QgsGeometry * mpPolygonGeometryB;
    QgsGeometry * mpPolygonGeometryC;
    QString mWktLine;
    QString mTestDataDir;
    QImage mImage;
    QPainter * mpPainter;
    QPen mPen1;
    QPen mPen2;
    QString mReport;
};

TestQgsGeometry::TestQgsGeometry()
    : mpPolylineGeometryD( NULL )
    , mpPolygonGeometryA( NULL )
    , mpPolygonGeometryB( NULL )
    , mpPolygonGeometryC( NULL )
    , mpPainter( NULL )
{

}

void TestQgsGeometry::initTestCase()
{
  // Runs once before any tests are run
  // init QGIS's paths - true means that all path will be inited from prefix
  QgsApplication::init();
  QgsApplication::initQgis();
  QgsApplication::showSettings();
  mReport += "<h1>Geometry Tests</h1>\n";
  mReport += "<p><font color=\"green\">Green = polygonA</font></p>\n";
  mReport += "<p><font color=\"red\">Red = polygonB</font></p>\n";
  mReport += "<p><font color=\"blue\">Blue = polygonC</font></p>\n";
}


void TestQgsGeometry::cleanupTestCase()
{
  // Runs once after all tests are run
  QString myReportFile = QDir::tempPath() + "/qgistest.html";
  QFile myFile( myReportFile );
  if ( myFile.open( QIODevice::WriteOnly | QIODevice::Append ) )
  {
    QTextStream myQTextStream( &myFile );
    myQTextStream << mReport;
    myFile.close();
    //QDesktopServices::openUrl( "file:///" + myReportFile );
  }

  QgsApplication::exitQgis();
}

void TestQgsGeometry::init()
{
  //
  // Reset / reinitialise the geometries before each test is run
  //
  mPoint1 = QgsPoint( 20.0, 20.0 );
  mPoint2 = QgsPoint( 80.0, 20.0 );
  mPoint3 = QgsPoint( 80.0, 80.0 );
  mPoint4 = QgsPoint( 20.0, 80.0 );
  mPointA = QgsPoint( 40.0, 40.0 );
  mPointB = QgsPoint( 100.0, 40.0 );
  mPointC = QgsPoint( 100.0, 100.0 );
  mPointD = QgsPoint( 40.0, 100.0 );
  mPointW = QgsPoint( 200.0, 200.0 );
  mPointX = QgsPoint( 240.0, 200.0 );
  mPointY = QgsPoint( 240.0, 240.0 );
  mPointZ = QgsPoint( 200.0, 240.0 );

  mWktLine = QString( "LINESTRING(117.623198 35.198654, 117.581274 35.198654, 117.078178 35.324427, 116.868555 35.534051, 116.617007 35.869448, 116.491233 35.953297, 116.155836 36.288694, 116.071987 36.372544, 115.443117 36.749865, 114.814247 37.043338, 114.311152 37.169112, 113.388810 37.378735, 113.095337 37.378735, 112.592241 37.378735, 111.753748 37.294886, 111.502201 37.252961, 111.082954 37.127187, 110.747557 37.127187, 110.160612 36.917564, 110.034838 36.833715, 109.741366 36.749865, 109.573667 36.666016, 109.238270 36.498317, 109.070571 36.414468, 108.819023 36.288694, 108.693250 36.246770, 108.483626 36.162920, 107.645134 35.911372, 106.597017 35.869448, 106.051997 35.701749, 105.800449 35.617900, 105.590826 35.575975, 105.297354 35.575975, 104.961956 35.575975, 104.710409 35.534051, 104.458861 35.492126, 103.871916 35.492126, 103.788066 35.492126, 103.326895 35.408277, 102.949574 35.408277, 102.488402 35.450201, 102.069156 35.450201, 101.482211 35.450201, 100.937191 35.659825, 100.308321 35.869448, 100.056773 36.037146, 99.050582 36.079071, 97.667069 35.743674, 97.163973 35.617900, 96.115857 35.534051, 95.612761 35.534051, 94.396947 35.911372, 93.684228 36.288694, 92.929584 36.833715, 92.258790 37.169112, 91.629920 37.504509, 90.414105 37.881831, 90.414105 37.881831, 90.246407 37.923755, 89.491763 37.839906, 89.156366 37.672207, 88.485572 37.504509, 87.814778 37.252961, 87.563230 37.169112, 87.143983 37.043338, 85.970093 36.875639, 85.802395 36.875639, 84.083484 36.959489, 84.041560 37.043338, 82.951519 37.546433, 82.699971 37.630283)" );

  mPolygonA.clear();
  mPolygonB.clear();
  mPolygonC.clear();
  mPolylineA.clear();
  mPolylineB.clear();
  mPolylineC.clear();
  mPolylineA << mPoint1 << mPoint2 << mPoint3 << mPoint4 << mPoint1;
  mPolygonA << mPolylineA;
  //Polygon B intersects Polygon A
  mPolylineB << mPointA << mPointB << mPointC << mPointD << mPointA;
  mPolygonB << mPolylineB;
  // Polygon C should intersect no other polys
  mPolylineC << mPointW << mPointX << mPointY << mPointZ << mPointW;
  mPolygonC << mPolylineC;

  mpPolylineGeometryD = QgsGeometry::fromWkt( mWktLine );

  //polygon: first item of the list is outer ring,
  // inner rings (if any) start from second item
  mpPolygonGeometryA = QgsGeometry::fromPolygon( mPolygonA );
  mpPolygonGeometryB = QgsGeometry::fromPolygon( mPolygonB );
  mpPolygonGeometryC = QgsGeometry::fromPolygon( mPolygonC );

  mImage = QImage( 250, 250, QImage::Format_RGB32 );
  mImage.fill( qRgb( 152, 219, 249 ) );
  mpPainter = new QPainter( &mImage );

  // Draw the test shapes first
  mPen1 = QPen();
  mPen1.setWidth( 5 );
  mPen1.setBrush( Qt::green );
  mpPainter->setPen( mPen1 );
  dumpPolygon( mPolygonA );
  mPen1.setBrush( Qt::red );
  mpPainter->setPen( mPen1 );
  dumpPolygon( mPolygonB );
  mPen1.setBrush( Qt::blue );
  mpPainter->setPen( mPen1 );
  dumpPolygon( mPolygonC );

  mPen2 = QPen();
  mPen2.setWidth( 1 );
  mPen2.setBrush( Qt::black );
  QBrush myBrush( Qt::DiagCrossPattern );


  //set the pen to a different color -
  //any test outs will be drawn in pen2
  mpPainter->setPen( mPen2 );
  mpPainter->setBrush( myBrush );
}

void TestQgsGeometry::cleanup()
{
  // will be called after every testfunction.
  delete mpPolygonGeometryA;
  delete mpPolygonGeometryB;
  delete mpPolygonGeometryC;
  delete mpPolylineGeometryD;
  delete mpPainter;
}

void TestQgsGeometry::copy()
{
  //create a point geometry
  QgsGeometry original( new QgsPointV2( 1.0, 2.0 ) );
  QCOMPARE( original.geometry()->vertexAt( QgsVertexId( 0, 0, 0 ) ).x(), 1.0 );
  QCOMPARE( original.geometry()->vertexAt( QgsVertexId( 0, 0, 0 ) ).y(), 2.0 );

  //implicitly shared copy
  QgsGeometry copy( original );
  QCOMPARE( copy.geometry()->vertexAt( QgsVertexId( 0, 0, 0 ) ).x(), 1.0 );
  QCOMPARE( copy.geometry()->vertexAt( QgsVertexId( 0, 0, 0 ) ).y(), 2.0 );

  //trigger a detach
  copy.setGeometry( new QgsPointV2( 3.0, 4.0 ) );
  QCOMPARE( copy.geometry()->vertexAt( QgsVertexId( 0, 0, 0 ) ).x(), 3.0 );
  QCOMPARE( copy.geometry()->vertexAt( QgsVertexId( 0, 0, 0 ) ).y(), 4.0 );

  //make sure original was untouched
  QCOMPARE( original.geometry()->vertexAt( QgsVertexId( 0, 0, 0 ) ).x(), 1.0 );
  QCOMPARE( original.geometry()->vertexAt( QgsVertexId( 0, 0, 0 ) ).y(), 2.0 );
}

void TestQgsGeometry::assignment()
{
  //create a point geometry
  QgsGeometry original( new QgsPointV2( 1.0, 2.0 ) );
  QCOMPARE( original.geometry()->vertexAt( QgsVertexId( 0, 0, 0 ) ).x(), 1.0 );
  QCOMPARE( original.geometry()->vertexAt( QgsVertexId( 0, 0, 0 ) ).y(), 2.0 );

  //assign to implicitly shared copy
  QgsGeometry copy;
  copy = original;
  QCOMPARE( copy.geometry()->vertexAt( QgsVertexId( 0, 0, 0 ) ).x(), 1.0 );
  QCOMPARE( copy.geometry()->vertexAt( QgsVertexId( 0, 0, 0 ) ).y(), 2.0 );

  //trigger a detach
  copy.setGeometry( new QgsPointV2( 3.0, 4.0 ) );
  QCOMPARE( copy.geometry()->vertexAt( QgsVertexId( 0, 0, 0 ) ).x(), 3.0 );
  QCOMPARE( copy.geometry()->vertexAt( QgsVertexId( 0, 0, 0 ) ).y(), 4.0 );

  //make sure original was untouched
  QCOMPARE( original.geometry()->vertexAt( QgsVertexId( 0, 0, 0 ) ).x(), 1.0 );
  QCOMPARE( original.geometry()->vertexAt( QgsVertexId( 0, 0, 0 ) ).y(), 2.0 );
}

void TestQgsGeometry::asVariant()
{
  //create a point geometry
  QgsGeometry original( new QgsPointV2( 1.0, 2.0 ) );
  QCOMPARE( original.geometry()->vertexAt( QgsVertexId( 0, 0, 0 ) ).x(), 1.0 );
  QCOMPARE( original.geometry()->vertexAt( QgsVertexId( 0, 0, 0 ) ).y(), 2.0 );

  //convert to and from a QVariant
  QVariant var = QVariant::fromValue( original );
  QVERIFY( var.isValid() );

  QgsGeometry fromVar = qvariant_cast<QgsGeometry>( var );
  QCOMPARE( fromVar.geometry()->vertexAt( QgsVertexId( 0, 0, 0 ) ).x(), 1.0 );
  QCOMPARE( fromVar.geometry()->vertexAt( QgsVertexId( 0, 0, 0 ) ).y(), 2.0 );

  //also check copying variant
  QVariant var2 = var;
  QVERIFY( var2.isValid() );
  QgsGeometry fromVar2 = qvariant_cast<QgsGeometry>( var2 );
  QCOMPARE( fromVar2.geometry()->vertexAt( QgsVertexId( 0, 0, 0 ) ).x(), 1.0 );
  QCOMPARE( fromVar2.geometry()->vertexAt( QgsVertexId( 0, 0, 0 ) ).y(), 2.0 );

  //modify original and check detachment
  original.setGeometry( new QgsPointV2( 3.0, 4.0 ) );
  QgsGeometry fromVar3 = qvariant_cast<QgsGeometry>( var );
  QCOMPARE( fromVar3.geometry()->vertexAt( QgsVertexId( 0, 0, 0 ) ).x(), 1.0 );
  QCOMPARE( fromVar3.geometry()->vertexAt( QgsVertexId( 0, 0, 0 ) ).y(), 2.0 );
}

void TestQgsGeometry::isEmpty()
{
  QgsGeometry geom;
  QVERIFY( geom.isEmpty() );

  geom.setGeometry( new QgsPointV2( 1.0, 2.0 ) );
  QVERIFY( !geom.isEmpty() );

  geom.setGeometry( 0 );
  QVERIFY( geom.isEmpty() );
}

void TestQgsGeometry::pointV2()
{
  //test QgsPointV2

  //test constructors
  QgsPointV2 p1( 5.0, 6.0 );
  QCOMPARE( p1.x(), 5.0 );
  QCOMPARE( p1.y(), 6.0 );
  QVERIFY( !p1.isEmpty() );
  QVERIFY( !p1.is3D() );
  QVERIFY( !p1.isMeasure() );
  QCOMPARE( p1.wkbType(), QgsWKBTypes::Point );
  QCOMPARE( p1.wktTypeStr(), QString( "Point" ) );

  QgsPointV2 p2( QgsPoint( 3.0, 4.0 ) );
  QCOMPARE( p2.x(), 3.0 );
  QCOMPARE( p2.y(), 4.0 );
  QVERIFY( !p2.isEmpty() );
  QVERIFY( !p2.is3D() );
  QVERIFY( !p2.isMeasure() );
  QCOMPARE( p2.wkbType(), QgsWKBTypes::Point );

  QgsPointV2 p3( QPointF( 7.0, 9.0 ) );
  QCOMPARE( p3.x(), 7.0 );
  QCOMPARE( p3.y(), 9.0 );
  QVERIFY( !p3.isEmpty() );
  QVERIFY( !p3.is3D() );
  QVERIFY( !p3.isMeasure() );
  QCOMPARE( p3.wkbType(), QgsWKBTypes::Point );

  QgsPointV2 p4( QgsWKBTypes::Point, 11.0, 13.0 );
  QCOMPARE( p4.x(), 11.0 );
  QCOMPARE( p4.y(), 13.0 );
  QVERIFY( !p4.isEmpty() );
  QVERIFY( !p4.is3D() );
  QVERIFY( !p4.isMeasure() );
  QCOMPARE( p4.wkbType(), QgsWKBTypes::Point );

  QgsPointV2 p5( QgsWKBTypes::PointZ, 11.0, 13.0, 15.0 );
  QCOMPARE( p5.x(), 11.0 );
  QCOMPARE( p5.y(), 13.0 );
  QCOMPARE( p5.z(), 15.0 );
  QVERIFY( !p5.isEmpty() );
  QVERIFY( p5.is3D() );
  QVERIFY( !p5.isMeasure() );
  QCOMPARE( p5.wkbType(), QgsWKBTypes::PointZ );
  QCOMPARE( p5.wktTypeStr(), QString( "PointZ" ) );

  QgsPointV2 p6( QgsWKBTypes::PointM, 11.0, 13.0, 0.0, 17.0 );
  QCOMPARE( p6.x(), 11.0 );
  QCOMPARE( p6.y(), 13.0 );
  QCOMPARE( p6.m(), 17.0 );
  QVERIFY( !p6.isEmpty() );
  QVERIFY( !p6.is3D() );
  QVERIFY( p6.isMeasure() );
  QCOMPARE( p6.wkbType(), QgsWKBTypes::PointM );
  QCOMPARE( p6.wktTypeStr(), QString( "PointM" ) );

  QgsPointV2 p7( QgsWKBTypes::PointZM, 11.0, 13.0, 0.0, 17.0 );
  QCOMPARE( p7.x(), 11.0 );
  QCOMPARE( p7.y(), 13.0 );
  QCOMPARE( p7.m(), 17.0 );
  QVERIFY( !p7.isEmpty() );
  QVERIFY( p7.is3D() );
  QVERIFY( p7.isMeasure() );
  QCOMPARE( p7.wkbType(), QgsWKBTypes::PointZM );
  QCOMPARE( p7.wktTypeStr(), QString( "PointZM" ) );

  QgsPointV2 p8( QgsWKBTypes::Point25D, 21.0, 23.0, 25.0 );
  QCOMPARE( p8.x(), 21.0 );
  QCOMPARE( p8.y(), 23.0 );
  QCOMPARE( p8.z(), 25.0 );
  QVERIFY( !p8.isEmpty() );
  QVERIFY( p8.is3D() );
  QVERIFY( !p8.isMeasure() );
  QCOMPARE( p8.wkbType(), QgsWKBTypes::Point25D );

#if 0 //should trigger an assert
  //try creating a point with a nonsense WKB type
  QgsPointV2 p9( QgsWKBTypes::PolygonZM, 11.0, 13.0, 9.0, 17.0 );
  QCOMPARE( p9.wkbType(), QgsWKBTypes::Unknown );
#endif

  //test equality operator
  QVERIFY( QgsPointV2( QgsWKBTypes::Point, 2 / 3.0, 1 / 3.0 ) == QgsPointV2( QgsWKBTypes::Point, 2 / 3.0, 1 / 3.0 ) );
  QVERIFY( !( QgsPointV2( QgsWKBTypes::PointZ, 2 / 3.0, 1 / 3.0 ) == QgsPointV2( QgsWKBTypes::Point, 2 / 3.0, 1 / 3.0 ) ) );
  QVERIFY( !( QgsPointV2( QgsWKBTypes::Point, 1 / 3.0, 1 / 3.0 ) == QgsPointV2( QgsWKBTypes::Point, 2 / 3.0, 1 / 3.0 ) ) );
  QVERIFY( !( QgsPointV2( QgsWKBTypes::Point, 2 / 3.0, 2 / 3.0 ) == QgsPointV2( QgsWKBTypes::Point, 2 / 3.0, 1 / 3.0 ) ) );
  QVERIFY( QgsPointV2( QgsWKBTypes::PointZ, 3.0, 4.0, 1 / 3.0 ) == QgsPointV2( QgsWKBTypes::PointZ, 3.0, 4.0, 1 / 3.0 ) );
  QVERIFY( !( QgsPointV2( QgsWKBTypes::PointZ, 3.0, 4.0, 1 / 3.0 ) == QgsPointV2( QgsWKBTypes::PointZM, 3.0, 4.0, 1 / 3.0 ) ) );
  QVERIFY( !( QgsPointV2( QgsWKBTypes::PointZ, 3.0, 4.0, 2 / 3.0 ) == QgsPointV2( QgsWKBTypes::PointZ, 3.0, 4.0, 1 / 3.0 ) ) );
  QVERIFY( QgsPointV2( QgsWKBTypes::PointM, 3.0, 4.0, 0.0, 1 / 3.0 ) == QgsPointV2( QgsWKBTypes::PointM, 3.0, 4.0, 0.0, 1 / 3.0 ) );
  QVERIFY( !( QgsPointV2( QgsWKBTypes::PointM, 3.0, 4.0, 0.0, 1 / 3.0 ) == QgsPointV2( QgsWKBTypes::PointZ, 3.0, 4.0, 0.0, 1 / 3.0 ) ) );
  QVERIFY( !( QgsPointV2( QgsWKBTypes::PointM, 3.0, 4.0, 0.0, 1 / 3.0 ) == QgsPointV2( QgsWKBTypes::PointM, 3.0, 4.0, 0.0, 2 / 3.0 ) ) );
  QVERIFY( QgsPointV2( QgsWKBTypes::PointZM, 3.0, 4.0, 2 / 3.0, 1 / 3.0 ) == QgsPointV2( QgsWKBTypes::PointZM, 3.0, 4.0, 2 / 3.0, 1 / 3.0 ) );
  QVERIFY( QgsPointV2( QgsWKBTypes::Point25D, 3.0, 4.0, 2 / 3.0 ) == QgsPointV2( QgsWKBTypes::Point25D, 3.0, 4.0, 2 / 3.0 ) );
  QVERIFY( !( QgsPointV2( QgsWKBTypes::Point25D, 3.0, 4.0, 2 / 3.0 ) == QgsPointV2( QgsWKBTypes::PointZ, 3.0, 4.0, 2 / 3.0 ) ) );
  //test inequality operator
  QVERIFY( !( QgsPointV2( QgsWKBTypes::Point, 2 / 3.0, 1 / 3.0 ) != QgsPointV2( QgsWKBTypes::Point, 2 / 3.0, 1 / 3.0 ) ) );
  QVERIFY( QgsPointV2( QgsWKBTypes::Point, 2 / 3.0, 1 / 3.0 ) != QgsPointV2( QgsWKBTypes::PointZ, 2 / 3.0, 1 / 3.0 ) );

  //test setters and getters
  //x
  QgsPointV2 p10( QgsWKBTypes::PointZM );
  p10.setX( 5.0 );
  QCOMPARE( p10.x(), 5.0 );
  QCOMPARE( p10.rx(), 5.0 );
  p10.rx() = 9.0;
  QCOMPARE( p10.x(), 9.0 );
  //y
  p10.setY( 7.0 );
  QCOMPARE( p10.y(), 7.0 );
  QCOMPARE( p10.ry(), 7.0 );
  p10.ry() = 3.0;
  QCOMPARE( p10.y(), 3.0 );
  //z
  p10.setZ( 17.0 );
  QCOMPARE( p10.z(), 17.0 );
  QCOMPARE( p10.rz(), 17.0 );
  p10.rz() = 13.0;
  QCOMPARE( p10.z(), 13.0 );
  //m
  p10.setM( 27.0 );
  QCOMPARE( p10.m(), 27.0 );
  QCOMPARE( p10.rm(), 27.0 );
  p10.rm() = 23.0;
  QCOMPARE( p10.m(), 23.0 );

  //other checks
  QCOMPARE( p10.geometryType(), QString( "Point" ) );
  QCOMPARE( p10.dimension(), 0 );

  //clone
  QScopedPointer< QgsPointV2 >clone( p10.clone() );
  QVERIFY( p10 == *clone );

  //assignment
  QgsPointV2 original( QgsWKBTypes::PointZM, 1.0, 2.0, 3.0, -4.0 );
  QgsPointV2 assigned( 6.0, 7.0 );
  assigned = original;
  QVERIFY( assigned == original );

  //clear
  QgsPointV2 p11( 5.0, 6.0 );
  p11.clear();
  QCOMPARE( p11.wkbType(), QgsWKBTypes::Unknown );
  QCOMPARE( p11.x(), 0.0 );
  QCOMPARE( p11.y(), 0.0 );

  //to/from WKB
  QgsPointV2 p12( QgsWKBTypes::PointZM, 1.0, 2.0, 3.0, -4.0 );
  int size = 0;
  unsigned char* wkb = p12.asWkb( size );
  QCOMPARE( size, p12.wkbSize() );
  QgsPointV2 p13;
  p13.fromWkb( wkb );
  QVERIFY( p13 == p12 );

  //bad WKB - check for no crash
  p13 = QgsPointV2( 1, 2 );
  QVERIFY( !p13.fromWkb( 0 ) );
  QCOMPARE( p13.wkbType(), QgsWKBTypes::Unknown );
  QgsLineStringV2 line;
  p13 = QgsPointV2( 1, 2 );
  QVERIFY( !p13.fromWkb( line.asWkb( size ) ) );
  QCOMPARE( p13.wkbType(), QgsWKBTypes::Unknown );

  //to/from WKT
  p13 = QgsPointV2( QgsWKBTypes::PointZM, 1.0, 2.0, 3.0, -4.0 );
  QString wkt = p13.asWkt();
  QVERIFY( !wkt.isEmpty() );
  QgsPointV2 p14;
  QVERIFY( p14.fromWkt( wkt ) );
  QVERIFY( p14 == p13 );

  //bad WKT
  QVERIFY( !p14.fromWkt( "Polygon()" ) );
  QCOMPARE( p14.wkbType(), QgsWKBTypes::Unknown );

  //asGML2
  QgsPointV2 exportPoint( 1, 2 );
  QgsPointV2 exportPointFloat( 1 / 3.0, 2 / 3.0 );
  QDomDocument doc( "gml" );
  QString expectedGML2( "<Point xmlns=\"gml\"><coordinates xmlns=\"gml\">1,2</coordinates></Point>" );
  QCOMPARE( elemToString( exportPoint.asGML2( doc ) ), expectedGML2 );
  QString expectedGML2prec3( "<Point xmlns=\"gml\"><coordinates xmlns=\"gml\">0.333,0.667</coordinates></Point>" );
  QCOMPARE( elemToString( exportPointFloat.asGML2( doc, 3 ) ), expectedGML2prec3 );

  //asGML3
  QString expectedGML3( "<Point xmlns=\"gml\"><pos xmlns=\"gml\" srsDimension=\"2\">1 2</pos></Point>" );
  QCOMPARE( elemToString( exportPoint.asGML3( doc ) ), expectedGML3 );
  QString expectedGML3prec3( "<Point xmlns=\"gml\"><pos xmlns=\"gml\" srsDimension=\"2\">0.333 0.667</pos></Point>" );
  QCOMPARE( elemToString( exportPointFloat.asGML3( doc, 3 ) ), expectedGML3prec3 );

  //asJSON
  QString expectedJson( "{\"type\": \"Point\", \"coordinates\": [1, 2]}" );
  QCOMPARE( exportPoint.asJSON(), expectedJson );
  QString expectedJsonPrec3( "{\"type\": \"Point\", \"coordinates\": [0.333, 0.667]}" );
  QCOMPARE( exportPointFloat.asJSON( 3 ), expectedJsonPrec3 );

  //bounding box
  QgsPointV2 p15( 1.0, 2.0 );
  QCOMPARE( p15.boundingBox(), QgsRectangle( 1.0, 2.0, 1.0, 2.0 ) );
  //modify points and test that bounding box is updated accordingly
  p15.setX( 3.0 );
  QCOMPARE( p15.boundingBox(), QgsRectangle( 3.0, 2.0, 3.0, 2.0 ) );
  p15.setY( 6.0 );
  QCOMPARE( p15.boundingBox(), QgsRectangle( 3.0, 6.0, 3.0, 6.0 ) );
  p15.rx() = 4.0;
  QCOMPARE( p15.boundingBox(), QgsRectangle( 4.0, 6.0, 4.0, 6.0 ) );
  p15.ry() = 9.0;
  QCOMPARE( p15.boundingBox(), QgsRectangle( 4.0, 9.0, 4.0, 9.0 ) );
  p15.moveVertex( QgsVertexId( 0, 0, 0 ), QgsPointV2( 11.0, 13.0 ) );
  QCOMPARE( p15.boundingBox(), QgsRectangle( 11.0, 13.0, 11.0, 13.0 ) );
  p15 = QgsPointV2( 21.0, 23.0 );
  QCOMPARE( p15.boundingBox(), QgsRectangle( 21.0, 23.0, 21.0, 23.0 ) );

  //CRS transform
  QgsCoordinateReferenceSystem sourceSrs;
  sourceSrs.createFromSrid( 3994 );
  QgsCoordinateReferenceSystem destSrs;
  destSrs.createFromSrid( 4326 );
  QgsCoordinateTransform tr( sourceSrs, destSrs );
  QgsPointV2 p16( QgsWKBTypes::PointZM, 6374985, -3626584, 1, 2 );
  p16.transform( tr, QgsCoordinateTransform::ForwardTransform );
  QVERIFY( qgsDoubleNear( p16.x(), 175.771, 0.001 ) );
  QVERIFY( qgsDoubleNear( p16.y(), -39.722, 0.001 ) );
  QVERIFY( qgsDoubleNear( p16.z(), 57.2958, 0.001 ) );
  QCOMPARE( p16.m(), 2.0 );
  p16.transform( tr, QgsCoordinateTransform::ReverseTransform );
  QVERIFY( qgsDoubleNear( p16.x(), 6374985, 1 ) );
  QVERIFY( qgsDoubleNear( p16.y(), -3626584, 1 ) );
  QVERIFY( qgsDoubleNear( p16.z(), 1.0, 0.001 ) );
  QCOMPARE( p16.m(), 2.0 );

  //QTransform transform
  QTransform qtr = QTransform::fromScale( 2, 3 );
  QgsPointV2 p17( QgsWKBTypes::PointZM, 10, 20, 30, 40 );
  p17.transform( qtr );
  QVERIFY( p17 == QgsPointV2( QgsWKBTypes::PointZM, 20, 60, 30, 40 ) );

  //coordinateSequence
  QList< QList< QList< QgsPointV2 > > > coord;
  QgsPointV2 p18( QgsWKBTypes::PointZM, 1.0, 2.0, 3.0, 4.0 );
  p18.coordinateSequence( coord );
  QCOMPARE( coord.count(), 1 );
  QCOMPARE( coord.at( 0 ).count(), 1 );
  QCOMPARE( coord.at( 0 ).at( 0 ).count(), 1 );
  QCOMPARE( coord.at( 0 ).at( 0 ).at( 0 ), p18 );

  //low level editing
  //insertVertex should have no effect
  QgsPointV2 p19( QgsWKBTypes::PointZM, 3.0, 4.0, 6.0, 7.0 );
  p19.insertVertex( QgsVertexId( 1, 2, 3 ), QgsPointV2( 6.0, 7.0 ) );
  QCOMPARE( p19, QgsPointV2( QgsWKBTypes::PointZM, 3.0, 4.0, 6.0, 7.0 ) );

  //moveVertex
  p19.moveVertex( QgsVertexId( 0, 0, 0 ), QgsPointV2( QgsWKBTypes::PointZM, 1.0, 2.0, 3.0, 4.0 ) );
  QCOMPARE( p19, QgsPointV2( QgsWKBTypes::PointZM, 1.0, 2.0, 3.0, 4.0 ) );
  //invalid vertex id, should not crash
  p19.moveVertex( QgsVertexId( 1, 2, 3 ), QgsPointV2( QgsWKBTypes::PointZM, 2.0, 3.0, 1.0, 2.0 ) );
  QCOMPARE( p19, QgsPointV2( QgsWKBTypes::PointZM, 2.0, 3.0, 1.0, 2.0 ) );
  //move PointZM using Point
  p19.moveVertex( QgsVertexId( 0, 0, 0 ), QgsPointV2( QgsWKBTypes::Point, 11.0, 12.0 ) );
  QCOMPARE( p19, QgsPointV2( QgsWKBTypes::PointZM, 11.0, 12.0, 1.0, 2.0 ) );
  //move PointZM using PointZ
  p19.moveVertex( QgsVertexId( 0, 0, 0 ), QgsPointV2( QgsWKBTypes::PointZ, 21.0, 22.0, 23.0 ) );
  QCOMPARE( p19, QgsPointV2( QgsWKBTypes::PointZM, 21.0, 22.0, 23.0, 2.0 ) );
  //move PointZM using PointM
  p19.moveVertex( QgsVertexId( 0, 0, 0 ), QgsPointV2( QgsWKBTypes::PointM, 31.0, 32.0, 0.0, 43.0 ) );
  QCOMPARE( p19, QgsPointV2( QgsWKBTypes::PointZM, 31.0, 32.0, 23.0, 43.0 ) );
  //move Point using PointZM (z/m should be ignored)
  QgsPointV2 p20( 3.0, 4.0 );
  p20.moveVertex( QgsVertexId( 0, 0, 0 ), QgsPointV2( QgsWKBTypes::PointZM, 2.0, 3.0, 1.0, 2.0 ) );
  QCOMPARE( p20, QgsPointV2( 2.0, 3.0 ) );

  //deleteVertex - should do nothing, but not crash
  p20.deleteVertex( QgsVertexId( 0, 0, 0 ) );
  QCOMPARE( p20, QgsPointV2( 2.0, 3.0 ) );

  //closestSegment
  QgsPointV2 closest;
  QgsVertexId after;
  QCOMPARE( p20.closestSegment( QgsPointV2( 4.0, 6.0 ), closest, after, 0, 0 ), 13.0 );
  QCOMPARE( closest, p20 );
  QCOMPARE( after, QgsVertexId( 0, 0, 0 ) );

  //nextVertex
  QgsPointV2 p21( 3.0, 4.0 );
  QgsPointV2 p22;
  QgsVertexId v( 0, 0, -1 );
  QVERIFY( p21.nextVertex( v, p22 ) );
  QCOMPARE( p22, p21 );
  QCOMPARE( v, QgsVertexId( 0, 0, 0 ) );
  //no more vertices
  QVERIFY( !p21.nextVertex( v, p22 ) );

  //vertexAt - will always be same as point
  QCOMPARE( p21.vertexAt( QgsVertexId() ), p21 );
  QCOMPARE( p21.vertexAt( QgsVertexId( 0, 0, 0 ) ), p21 );

  //vertexAngle - undefined, but check that it doesn't crash
  ( void )p21.vertexAngle( QgsVertexId() );

  //counts
  QCOMPARE( p20.vertexCount(), 1 );
  QCOMPARE( p20.ringCount(), 1 );
  QCOMPARE( p20.partCount(), 1 );

  //measures and other abstract geometry methods
  QCOMPARE( p20.length(), 0.0 );
  QCOMPARE( p20.perimeter(), 0.0 );
  QCOMPARE( p20.area(), 0.0 );
  QCOMPARE( p20.centroid(), p20 );
  QVERIFY( !p20.hasCurvedSegments() );
  QScopedPointer< QgsPointV2 >segmented( static_cast< QgsPointV2*>( p20.segmentize() ) );
  QCOMPARE( *segmented, p20 );

  //addZValue
  QgsPointV2 p23( 1.0, 2.0 );
  QVERIFY( p23.addZValue( 5.0 ) );
  QCOMPARE( p23, QgsPointV2( QgsWKBTypes::PointZ, 1.0, 2.0, 5.0 ) );
  QVERIFY( !p23.addZValue( 6.0 ) );

  //addMValue
  QgsPointV2 p24( 1.0, 2.0 );
  QVERIFY( p24.addMValue( 5.0 ) );
  QCOMPARE( p24, QgsPointV2( QgsWKBTypes::PointM, 1.0, 2.0, 0.0, 5.0 ) );
  QVERIFY( !p24.addMValue( 6.0 ) );

}

void TestQgsGeometry::lineStringV2()
{
  //test constructors
  QgsLineStringV2 l1;
  QVERIFY( l1.isEmpty() );
  QCOMPARE( l1.numPoints(), 0 );
  QVERIFY( !l1.is3D() );
  QVERIFY( !l1.isMeasure() );
  QCOMPARE( l1.wkbType(), QgsWKBTypes::LineString );
  QCOMPARE( l1.wktTypeStr(), QString( "LineString" ) );
  QCOMPARE( l1.geometryType(), QString( "LineString" ) );
  QCOMPARE( l1.dimension(), 1 );

  //addVertex
  QgsLineStringV2 l2;
  l2.addVertex( QgsPointV2( 1.0, 2.0 ) );
  QVERIFY( !l2.isEmpty() );
  QCOMPARE( l2.numPoints(), 1 );
  QVERIFY( !l2.is3D() );
  QVERIFY( !l2.isMeasure() );
  QCOMPARE( l2.wkbType(), QgsWKBTypes::LineString );

  //adding first vertex should set linestring z/m type
  QgsLineStringV2 l3;
  l3.addVertex( QgsPointV2( QgsWKBTypes::PointZ, 1.0, 2.0, 3.0 ) );
  QVERIFY( !l3.isEmpty() );
  QVERIFY( l3.is3D() );
  QVERIFY( !l3.isMeasure() );
  QCOMPARE( l3.wkbType(), QgsWKBTypes::LineStringZ );
  QCOMPARE( l3.wktTypeStr(), QString( "LineStringZ" ) );

  QgsLineStringV2 l4;
  l4.addVertex( QgsPointV2( QgsWKBTypes::PointM, 1.0, 2.0, 0.0, 3.0 ) );
  QVERIFY( !l4.isEmpty() );
  QVERIFY( !l4.is3D() );
  QVERIFY( l4.isMeasure() );
  QCOMPARE( l4.wkbType(), QgsWKBTypes::LineStringM );
  QCOMPARE( l4.wktTypeStr(), QString( "LineStringM" ) );

  QgsLineStringV2 l5;
  l5.addVertex( QgsPointV2( QgsWKBTypes::PointZM, 1.0, 2.0, 3.0, 4.0 ) );
  QVERIFY( !l5.isEmpty() );
  QVERIFY( l5.is3D() );
  QVERIFY( l5.isMeasure() );
  QCOMPARE( l5.wkbType(), QgsWKBTypes::LineStringZM );
  QCOMPARE( l5.wktTypeStr(), QString( "LineStringZM" ) );

  //adding subsequent vertices should not alter z/m type, regardless of points type
  QgsLineStringV2 l6;
  l6.addVertex( QgsPointV2( QgsWKBTypes::Point, 1.0, 2.0 ) ); //2d type
  QCOMPARE( l6.wkbType(), QgsWKBTypes::LineString );
  l6.addVertex( QgsPointV2( QgsWKBTypes::PointZ, 11.0, 12.0, 13.0 ) ); // add 3d point
  QCOMPARE( l6.numPoints(), 2 );
  QCOMPARE( l6.wkbType(), QgsWKBTypes::LineString ); //should still be 2d
  QVERIFY( !l6.is3D() );

  QgsLineStringV2 l7;
  l7.addVertex( QgsPointV2( QgsWKBTypes::PointZ, 1.0, 2.0, 3.0 ) ); //3d type
  QCOMPARE( l7.wkbType(), QgsWKBTypes::LineStringZ );
  l7.addVertex( QgsPointV2( QgsWKBTypes::Point, 11.0, 12.0 ) ); //add 2d point
  QCOMPARE( l7.wkbType(), QgsWKBTypes::LineStringZ ); //should still be 3d
  QCOMPARE( l7.pointN( 1 ), QgsPointV2( QgsWKBTypes::PointZ, 11.0, 12.0, 0.0 ) );
  QVERIFY( l7.is3D() );
  QCOMPARE( l7.numPoints(), 2 );

  //clear
  l7.clear();
  QVERIFY( l7.isEmpty() );
  QCOMPARE( l7.numPoints(), 0 );
  QVERIFY( !l7.is3D() );
  QVERIFY( !l7.isMeasure() );
  QCOMPARE( l7.wkbType(), QgsWKBTypes::Unknown );

  //setPoints
  QgsLineStringV2 l8;
  l8.setPoints( QList< QgsPointV2 >() << QgsPointV2( 1, 2 ) << QgsPointV2( 2, 3 ) << QgsPointV2( 3, 4 ) );
  QVERIFY( !l8.isEmpty() );
  QCOMPARE( l8.numPoints(), 3 );
  QVERIFY( !l8.is3D() );
  QVERIFY( !l8.isMeasure() );
  QCOMPARE( l8.wkbType(), QgsWKBTypes::LineString );

  //setPoints with empty list, should clear linestring
  l8.setPoints( QList< QgsPointV2 >() );
  QVERIFY( l8.isEmpty() );
  QCOMPARE( l8.numPoints(), 0 );
  QCOMPARE( l8.wkbType(), QgsWKBTypes::Unknown );

  //setPoints with z
  l8.setPoints( QList< QgsPointV2 >() << QgsPointV2( QgsWKBTypes::PointZ, 1, 2, 3 ) << QgsPointV2( QgsWKBTypes::PointZ, 2, 3, 4 ) );
  QCOMPARE( l8.numPoints(), 2 );
  QVERIFY( l8.is3D() );
  QVERIFY( !l8.isMeasure() );
  QCOMPARE( l8.wkbType(), QgsWKBTypes::LineStringZ );

  //setPoints with m
  l8.setPoints( QList< QgsPointV2 >() << QgsPointV2( QgsWKBTypes::PointM, 1, 2, 0, 3 ) << QgsPointV2( QgsWKBTypes::PointM, 2, 3, 0, 4 ) );
  QCOMPARE( l8.numPoints(), 2 );
  QVERIFY( !l8.is3D() );
  QVERIFY( l8.isMeasure() );
  QCOMPARE( l8.wkbType(), QgsWKBTypes::LineStringM );

  //setPoints with zm
  l8.setPoints( QList< QgsPointV2 >() << QgsPointV2( QgsWKBTypes::PointZM, 1, 2, 4, 5 ) << QgsPointV2( QgsWKBTypes::PointZM, 2, 3, 4, 5 ) );
  QCOMPARE( l8.numPoints(), 2 );
  QVERIFY( l8.is3D() );
  QVERIFY( l8.isMeasure() );
  QCOMPARE( l8.wkbType(), QgsWKBTypes::LineStringZM );

  //setPoints with MIXED dimensionality of points
  l8.setPoints( QList< QgsPointV2 >() << QgsPointV2( QgsWKBTypes::PointZM, 1, 2, 4, 5 ) << QgsPointV2( QgsWKBTypes::PointM, 2, 3, 0, 5 ) );
  QCOMPARE( l8.numPoints(), 2 );
  QVERIFY( l8.is3D() );
  QVERIFY( l8.isMeasure() );
  QCOMPARE( l8.wkbType(), QgsWKBTypes::LineStringZM );

  //test point
  QCOMPARE( l8.pointN( 0 ), QgsPointV2( QgsWKBTypes::PointZM, 1, 2, 4, 5 ) );
  QCOMPARE( l8.pointN( 1 ), QgsPointV2( QgsWKBTypes::PointZM, 2, 3, 0, 5 ) );

  //out of range - just want no crash here
  QgsPointV2 bad = l8.pointN( -1 );
  bad = l8.pointN( 100 );

  //test getters/setters
  QgsLineStringV2 l9;
  l9.setPoints( QList< QgsPointV2 >() << QgsPointV2( QgsWKBTypes::PointZM, 1, 2, 3, 4 )
                << QgsPointV2( QgsWKBTypes::PointZM, 11, 12, 13, 14 )
                << QgsPointV2( QgsWKBTypes::PointZM, 21, 22, 23, 24 ) );
  QCOMPARE( l9.xAt( 0 ), 1.0 );
  QCOMPARE( l9.xAt( 1 ), 11.0 );
  QCOMPARE( l9.xAt( 2 ), 21.0 );
  QCOMPARE( l9.xAt( -1 ), 0.0 ); //out of range
  QCOMPARE( l9.xAt( 11 ), 0.0 ); //out of range

  l9.setXAt( 0, 51.0 );
  QCOMPARE( l9.xAt( 0 ), 51.0 );
  l9.setXAt( 1, 61.0 );
  QCOMPARE( l9.xAt( 1 ), 61.0 );
  l9.setXAt( -1, 51.0 ); //out of range
  l9.setXAt( 11, 51.0 ); //out of range

  QCOMPARE( l9.yAt( 0 ), 2.0 );
  QCOMPARE( l9.yAt( 1 ), 12.0 );
  QCOMPARE( l9.yAt( 2 ), 22.0 );
  QCOMPARE( l9.yAt( -1 ), 0.0 ); //out of range
  QCOMPARE( l9.yAt( 11 ), 0.0 ); //out of range

  l9.setYAt( 0, 52.0 );
  QCOMPARE( l9.yAt( 0 ), 52.0 );
  l9.setYAt( 1, 62.0 );
  QCOMPARE( l9.yAt( 1 ), 62.0 );
  l9.setYAt( -1, 52.0 ); //out of range
  l9.setYAt( 11, 52.0 ); //out of range

  QCOMPARE( l9.zAt( 0 ), 3.0 );
  QCOMPARE( l9.zAt( 1 ), 13.0 );
  QCOMPARE( l9.zAt( 2 ), 23.0 );
  QCOMPARE( l9.zAt( -1 ), 0.0 ); //out of range
  QCOMPARE( l9.zAt( 11 ), 0.0 ); //out of range

  l9.setZAt( 0, 53.0 );
  QCOMPARE( l9.zAt( 0 ), 53.0 );
  l9.setZAt( 1, 63.0 );
  QCOMPARE( l9.zAt( 1 ), 63.0 );
  l9.setZAt( -1, 53.0 ); //out of range
  l9.setZAt( 11, 53.0 ); //out of range

  QCOMPARE( l9.mAt( 0 ), 4.0 );
  QCOMPARE( l9.mAt( 1 ), 14.0 );
  QCOMPARE( l9.mAt( 2 ), 24.0 );
  QCOMPARE( l9.mAt( -1 ), 0.0 ); //out of range
  QCOMPARE( l9.mAt( 11 ), 0.0 ); //out of range

  l9.setMAt( 0, 54.0 );
  QCOMPARE( l9.mAt( 0 ), 54.0 );
  l9.setMAt( 1, 64.0 );
  QCOMPARE( l9.mAt( 1 ), 64.0 );
  l9.setMAt( -1, 54.0 ); //out of range
  l9.setMAt( 11, 54.0 ); //out of range

  //check zAt/setZAt with non-3d linestring
  l9.setPoints( QList< QgsPointV2 >() << QgsPointV2( QgsWKBTypes::PointM, 1, 2, 0, 4 )
                << QgsPointV2( QgsWKBTypes::PointM, 11, 12, 0, 14 )
                << QgsPointV2( QgsWKBTypes::PointM, 21, 22, 0, 24 ) );

  //basically we just don't want these to crash
  QCOMPARE( l9.zAt( 0 ), 0.0 );
  QCOMPARE( l9.zAt( 1 ), 0.0 );
  l9.setZAt( 0, 53.0 );
  l9.setZAt( 1, 63.0 );

  //check mAt/setMAt with non-measure linestring
  l9.setPoints( QList< QgsPointV2 >() << QgsPointV2( 1, 2 )
                << QgsPointV2( 11, 12 )
                << QgsPointV2( 21, 22 ) );

  //basically we just don't want these to crash
  QCOMPARE( l9.mAt( 0 ), 0.0 );
  QCOMPARE( l9.mAt( 1 ), 0.0 );
  l9.setMAt( 0, 53.0 );
  l9.setMAt( 1, 63.0 );

  //append linestring

  //append to empty
  QgsLineStringV2 l10;
  l10.append( 0 );
  QVERIFY( l10.isEmpty() );
  QCOMPARE( l10.numPoints(), 0 );

  QScopedPointer<QgsLineStringV2> toAppend( new QgsLineStringV2() );
  toAppend->setPoints( QList< QgsPointV2 >() << QgsPointV2( 1, 2 )
                       << QgsPointV2( 11, 12 )
                       << QgsPointV2( 21, 22 ) );
  l10.append( toAppend.data() );
  QVERIFY( !l10.is3D() );
  QVERIFY( !l10.isMeasure() );
  QCOMPARE( l10.numPoints(), 3 );
  QCOMPARE( l10.wkbType(), QgsWKBTypes::LineString );
  QCOMPARE( l10.pointN( 0 ), toAppend->pointN( 0 ) );
  QCOMPARE( l10.pointN( 1 ), toAppend->pointN( 1 ) );
  QCOMPARE( l10.pointN( 2 ), toAppend->pointN( 2 ) );

  //add more points
  toAppend.reset( new QgsLineStringV2() );
  toAppend->setPoints( QList< QgsPointV2 >() << QgsPointV2( 31, 32 )
                       << QgsPointV2( 41, 42 )
                       << QgsPointV2( 51, 52 ) );
  l10.append( toAppend.data() );
  QCOMPARE( l10.numPoints(), 6 );
  QCOMPARE( l10.pointN( 3 ), toAppend->pointN( 0 ) );
  QCOMPARE( l10.pointN( 4 ), toAppend->pointN( 1 ) );
  QCOMPARE( l10.pointN( 5 ), toAppend->pointN( 2 ) );

  //check dimensionality is inherited from append line if initially empty
  l10.clear();
  toAppend.reset( new QgsLineStringV2() );
  toAppend->setPoints( QList< QgsPointV2 >() << QgsPointV2( QgsWKBTypes::PointZM, 31, 32, 33, 34 )
                       << QgsPointV2( QgsWKBTypes::PointZM, 41, 42, 43 , 44 )
                       << QgsPointV2( QgsWKBTypes::PointZM, 51, 52, 53, 54 ) );
  l10.append( toAppend.data() );
  QVERIFY( l10.is3D() );
  QVERIFY( l10.isMeasure() );
  QCOMPARE( l10.numPoints(), 3 );
  QCOMPARE( l10.wkbType(), QgsWKBTypes::LineStringZM );
  QCOMPARE( l10.pointN( 0 ), toAppend->pointN( 0 ) );
  QCOMPARE( l10.pointN( 1 ), toAppend->pointN( 1 ) );
  QCOMPARE( l10.pointN( 2 ), toAppend->pointN( 2 ) );

  //append points with z to non z linestring
  l10.clear();
  l10.addVertex( QgsPointV2( 1.0, 2.0 ) );
  QVERIFY( !l10.is3D() );
  QCOMPARE( l10.wkbType(), QgsWKBTypes::LineString );
  toAppend.reset( new QgsLineStringV2() );
  toAppend->setPoints( QList< QgsPointV2 >() << QgsPointV2( QgsWKBTypes::PointZM, 31, 32, 33, 34 )
                       << QgsPointV2( QgsWKBTypes::PointZM, 41, 42, 43 , 44 )
                       << QgsPointV2( QgsWKBTypes::PointZM, 51, 52, 53, 54 ) );
  l10.append( toAppend.data() );
  QCOMPARE( l10.wkbType(), QgsWKBTypes::LineString );
  QCOMPARE( l10.pointN( 0 ), QgsPointV2( 1, 2 ) );
  QCOMPARE( l10.pointN( 1 ), QgsPointV2( 31, 32 ) );
  QCOMPARE( l10.pointN( 2 ), QgsPointV2( 41, 42 ) );
  QCOMPARE( l10.pointN( 3 ), QgsPointV2( 51, 52 ) );

  //append points without z/m to linestring with z & m
  l10.clear();
  l10.addVertex( QgsPointV2( QgsWKBTypes::PointZM, 1.0, 2.0, 3.0, 4.0 ) );
  QVERIFY( l10.is3D() );
  QVERIFY( l10.isMeasure() );
  QCOMPARE( l10.wkbType(), QgsWKBTypes::LineStringZM );
  toAppend.reset( new QgsLineStringV2() );
  toAppend->setPoints( QList< QgsPointV2 >() << QgsPointV2( 31, 32 )
                       << QgsPointV2( 41, 42 )
                       << QgsPointV2( 51, 52 ) );
  l10.append( toAppend.data() );
  QCOMPARE( l10.wkbType(), QgsWKBTypes::LineStringZM );
  QCOMPARE( l10.pointN( 0 ), QgsPointV2( QgsWKBTypes::PointZM, 1, 2, 3, 4 ) );
  QCOMPARE( l10.pointN( 1 ), QgsPointV2( QgsWKBTypes::PointZM, 31, 32 ) );
  QCOMPARE( l10.pointN( 2 ), QgsPointV2( QgsWKBTypes::PointZM, 41, 42 ) );
  QCOMPARE( l10.pointN( 3 ), QgsPointV2( QgsWKBTypes::PointZM, 51, 52 ) );

  //close
  QgsLineStringV2 l11;
  l11.setPoints( QList< QgsPointV2 >() << QgsPointV2( 1, 2 )
                 << QgsPointV2( 11, 2 )
                 << QgsPointV2( 11, 22 )
                 << QgsPointV2( 1, 22 ) );
  QVERIFY( !l11.isClosed() );
  QCOMPARE( l11.numPoints(), 4 );
  l11.close();
  QVERIFY( l11.isClosed() );
  QCOMPARE( l11.numPoints(), 5 );
  QCOMPARE( l11.pointN( 4 ), QgsPointV2( 1, 2 ) );

  //close with z and m
  QgsLineStringV2 l12;
  l12.setPoints( QList< QgsPointV2 >() << QgsPointV2( QgsWKBTypes::PointZM, 1, 2, 3, 4 )
                 << QgsPointV2( QgsWKBTypes::PointZM, 11, 2, 11, 14 )
                 << QgsPointV2( QgsWKBTypes::PointZM, 11, 22, 21, 24 )
                 << QgsPointV2( QgsWKBTypes::PointZM, 1, 22, 31, 34 ) );
  l12.close();
  QCOMPARE( l12.pointN( 4 ), QgsPointV2( QgsWKBTypes::PointZM, 1, 2, 3, 4 ) );


  //polygonf
  QgsLineStringV2 l13;
  l13.setPoints( QList< QgsPointV2 >() << QgsPointV2( QgsWKBTypes::PointZM, 1, 2, 3, 4 )
                 << QgsPointV2( QgsWKBTypes::PointZM, 11, 2, 11, 14 )
                 << QgsPointV2( QgsWKBTypes::PointZM, 11, 22, 21, 24 )
                 << QgsPointV2( QgsWKBTypes::PointZM, 1, 22, 31, 34 ) );

  QPolygonF poly = l13.asQPolygonF();
  QCOMPARE( poly.count(), 4 );
  QCOMPARE( poly.at( 0 ).x(), 1.0 );
  QCOMPARE( poly.at( 0 ).y(), 2.0 );
  QCOMPARE( poly.at( 1 ).x(), 11.0 );
  QCOMPARE( poly.at( 1 ).y(), 2.0 );
  QCOMPARE( poly.at( 2 ).x(), 11.0 );
  QCOMPARE( poly.at( 2 ).y(), 22.0 );
  QCOMPARE( poly.at( 3 ).x(), 1.0 );
  QCOMPARE( poly.at( 3 ).y(), 22.0 );

  // clone
  QgsLineStringV2 l14;
  l14.setPoints( QList< QgsPointV2 >() << QgsPointV2( 1, 2 )
                 << QgsPointV2( 11, 2 )
                 << QgsPointV2( 11, 22 )
                 << QgsPointV2( 1, 22 ) );
  QScopedPointer<QgsLineStringV2> cloned( l14.clone() );
  QCOMPARE( cloned->numPoints(), 4 );
  QCOMPARE( cloned->wkbType(), QgsWKBTypes::LineString );
  QVERIFY( !cloned->is3D() );
  QVERIFY( !cloned->isMeasure() );
  QCOMPARE( cloned->pointN( 0 ), l14.pointN( 0 ) );
  QCOMPARE( cloned->pointN( 1 ), l14.pointN( 1 ) );
  QCOMPARE( cloned->pointN( 2 ), l14.pointN( 2 ) );
  QCOMPARE( cloned->pointN( 3 ), l14.pointN( 3 ) );

  //clone with Z/M
  l14.setPoints( QList< QgsPointV2 >() << QgsPointV2( QgsWKBTypes::PointZM, 1, 2, 3, 4 )
                 << QgsPointV2( QgsWKBTypes::PointZM, 11, 2, 11, 14 )
                 << QgsPointV2( QgsWKBTypes::PointZM, 11, 22, 21, 24 )
                 << QgsPointV2( QgsWKBTypes::PointZM, 1, 22, 31, 34 ) );
  cloned.reset( l14.clone() );
  QCOMPARE( cloned->numPoints(), 4 );
  QCOMPARE( cloned->wkbType(), QgsWKBTypes::LineStringZM );
  QVERIFY( cloned->is3D() );
  QVERIFY( cloned->isMeasure() );
  QCOMPARE( cloned->pointN( 0 ), l14.pointN( 0 ) );
  QCOMPARE( cloned->pointN( 1 ), l14.pointN( 1 ) );
  QCOMPARE( cloned->pointN( 2 ), l14.pointN( 2 ) );
  QCOMPARE( cloned->pointN( 3 ), l14.pointN( 3 ) );

  //clone an empty line
  l14.clear();
  cloned.reset( l14.clone() );
  QVERIFY( cloned->isEmpty() );
  QCOMPARE( cloned->numPoints(), 0 );
  QVERIFY( !cloned->is3D() );
  QVERIFY( !cloned->isMeasure() );
  QCOMPARE( cloned->wkbType(), QgsWKBTypes::Unknown );

  //to/from WKB
  QgsLineStringV2 l15;
  l15.setPoints( QList< QgsPointV2 >() << QgsPointV2( QgsWKBTypes::PointZM, 1, 2, 3, 4 )
                 << QgsPointV2( QgsWKBTypes::PointZM, 11, 2, 11, 14 )
                 << QgsPointV2( QgsWKBTypes::PointZM, 11, 22, 21, 24 )
                 << QgsPointV2( QgsWKBTypes::PointZM, 1, 22, 31, 34 ) );
  int size = 0;
  unsigned char* wkb = l15.asWkb( size );
  QCOMPARE( size, l15.wkbSize() );
  QgsLineStringV2 l16;
  l16.fromWkb( wkb );
  QCOMPARE( l16.numPoints(), 4 );
  QCOMPARE( l16.wkbType(), QgsWKBTypes::LineStringZM );
  QVERIFY( l16.is3D() );
  QVERIFY( l16.isMeasure() );
  QCOMPARE( l16.pointN( 0 ), l15.pointN( 0 ) );
  QCOMPARE( l16.pointN( 1 ), l15.pointN( 1 ) );
  QCOMPARE( l16.pointN( 2 ), l15.pointN( 2 ) );
  QCOMPARE( l16.pointN( 3 ), l15.pointN( 3 ) );

  //bad WKB - check for no crash
  l16.clear();
  QVERIFY( !l16.fromWkb( 0 ) );
  QCOMPARE( l16.wkbType(), QgsWKBTypes::Unknown );
  QgsPointV2 point( 1, 2 );
  QVERIFY( !l16.fromWkb( point.asWkb( size ) ) );
  QCOMPARE( l16.wkbType(), QgsWKBTypes::Unknown );

  //TODO - from WKB points

  //to/from WKT
  QgsLineStringV2 l17;
  l17.setPoints( QList< QgsPointV2 >() << QgsPointV2( QgsWKBTypes::PointZM, 1, 2, 3, 4 )
                 << QgsPointV2( QgsWKBTypes::PointZM, 11, 2, 11, 14 )
                 << QgsPointV2( QgsWKBTypes::PointZM, 11, 22, 21, 24 )
                 << QgsPointV2( QgsWKBTypes::PointZM, 1, 22, 31, 34 ) );

  QString wkt = l17.asWkt();
  QVERIFY( !wkt.isEmpty() );
  QgsLineStringV2 l18;
  QVERIFY( l18.fromWkt( wkt ) );
  QCOMPARE( l18.numPoints(), 4 );
  QCOMPARE( l18.wkbType(), QgsWKBTypes::LineStringZM );
  QVERIFY( l18.is3D() );
  QVERIFY( l18.isMeasure() );
  QCOMPARE( l18.pointN( 0 ), l17.pointN( 0 ) );
  QCOMPARE( l18.pointN( 1 ), l17.pointN( 1 ) );
  QCOMPARE( l18.pointN( 2 ), l17.pointN( 2 ) );
  QCOMPARE( l18.pointN( 3 ), l17.pointN( 3 ) );

  //bad WKT
  QVERIFY( !l18.fromWkt( "Polygon()" ) );
  QVERIFY( l18.isEmpty() );
  QCOMPARE( l18.numPoints(), 0 );
  QVERIFY( !l18.is3D() );
  QVERIFY( !l18.isMeasure() );
  QCOMPARE( l18.wkbType(), QgsWKBTypes::Unknown );

  //asGML2
  QgsLineStringV2 exportLine;
  exportLine.setPoints( QList< QgsPointV2 >() << QgsPointV2( 31, 32 )
                        << QgsPointV2( 41, 42 )
                        << QgsPointV2( 51, 52 ) );
  QgsLineStringV2 exportLineFloat;
  exportLineFloat.setPoints( QList< QgsPointV2 >() << QgsPointV2( 1 / 3.0, 2 / 3.0 )
                             << QgsPointV2( 1 + 1 / 3.0, 1 + 2 / 3.0 )
                             << QgsPointV2( 2 + 1 / 3.0, 2 + 2 / 3.0 ) );
  QDomDocument doc( "gml" );
  QString expectedGML2( "<LineString xmlns=\"gml\"><coordinates xmlns=\"gml\">31,32 41,42 51,52</coordinates></LineString>" );
  QCOMPARE( elemToString( exportLine.asGML2( doc ) ), expectedGML2 );
  QString expectedGML2prec3( "<LineString xmlns=\"gml\"><coordinates xmlns=\"gml\">0.333,0.667 1.333,1.667 2.333,2.667</coordinates></LineString>" );
  QCOMPARE( elemToString( exportLineFloat.asGML2( doc, 3 ) ), expectedGML2prec3 );

  //asGML3
  QString expectedGML3( "<Curve xmlns=\"gml\"><segments xmlns=\"gml\"><LineStringSegment xmlns=\"gml\"><posList xmlns=\"gml\" srsDimension=\"2\">31 32 41 42 51 52</posList></LineStringSegment></segments></Curve>" );
  QCOMPARE( elemToString( exportLine.asGML3( doc ) ), expectedGML3 );
  QString expectedGML3prec3( "<Curve xmlns=\"gml\"><segments xmlns=\"gml\"><LineStringSegment xmlns=\"gml\"><posList xmlns=\"gml\" srsDimension=\"2\">0.333 0.667 1.333 1.667 2.333 2.667</posList></LineStringSegment></segments></Curve>" );
  QCOMPARE( elemToString( exportLineFloat.asGML3( doc, 3 ) ), expectedGML3prec3 );

  //asJSON
  QString expectedJson( "{\"type\": \"LineString\", \"coordinates\": [ [31, 32], [41, 42], [51, 52]]}" );
  QCOMPARE( exportLine.asJSON(), expectedJson );
  QString expectedJsonPrec3( "{\"type\": \"LineString\", \"coordinates\": [ [0.333, 0.667], [1.333, 1.667], [2.333, 2.667]]}" );
  QCOMPARE( exportLineFloat.asJSON( 3 ), expectedJsonPrec3 );

  //length
  QgsLineStringV2 l19;
  QCOMPARE( l19.length(), 0.0 );
  l19.setPoints( QList< QgsPointV2 >() << QgsPointV2( QgsWKBTypes::PointZM, 1, 1, 2, 3 )
                 << QgsPointV2( QgsWKBTypes::PointZM, 1, 10, 4, 5 )
                 << QgsPointV2( QgsWKBTypes::PointZM, 15, 10, 6, 7 ) );
  QCOMPARE( l19.length(), 23.0 );

  //startPoint
  QCOMPARE( l19.startPoint(), QgsPointV2( QgsWKBTypes::PointZM, 1, 1, 2, 3 ) );

  //endPoint
  QCOMPARE( l19.endPoint(), QgsPointV2( QgsWKBTypes::PointZM, 15, 10, 6, 7 ) );

  //bad start/end points. Test that this doesn't crash.
  l19.clear();
  QCOMPARE( l19.startPoint(), QgsPointV2() );
  QCOMPARE( l19.endPoint(), QgsPointV2() );

  //curveToLine - no segmentation required, so should return a clone
  l19.setPoints( QList< QgsPointV2 >() << QgsPointV2( QgsWKBTypes::PointZM, 1, 1, 2, 3 )
                 << QgsPointV2( QgsWKBTypes::PointZM, 1, 10, 4, 5 )
                 << QgsPointV2( QgsWKBTypes::PointZM, 15, 10, 6, 7 ) );
  QScopedPointer< QgsLineStringV2 > segmentized( l19.curveToLine() );
  QCOMPARE( segmentized->numPoints(), 3 );
  QCOMPARE( segmentized->wkbType(), QgsWKBTypes::LineStringZM );
  QVERIFY( segmentized->is3D() );
  QVERIFY( segmentized->isMeasure() );
  QCOMPARE( segmentized->pointN( 0 ), l19.pointN( 0 ) );
  QCOMPARE( segmentized->pointN( 1 ), l19.pointN( 1 ) );
  QCOMPARE( segmentized->pointN( 2 ), l19.pointN( 2 ) );

  // points
  QgsLineStringV2 l20;
  QList< QgsPointV2 > points;
  l20.points( points );
  QVERIFY( l20.isEmpty() );
  l20.setPoints( QList< QgsPointV2 >() << QgsPointV2( QgsWKBTypes::PointZM, 1, 1, 2, 3 )
                 << QgsPointV2( QgsWKBTypes::PointZM, 1, 10, 4, 5 )
                 << QgsPointV2( QgsWKBTypes::PointZM, 15, 10, 6, 7 ) );
  l20.points( points );
  QCOMPARE( points.count(), 3 );
  QCOMPARE( points.at( 0 ), QgsPointV2( QgsWKBTypes::PointZM, 1, 1, 2, 3 ) );
  QCOMPARE( points.at( 1 ), QgsPointV2( QgsWKBTypes::PointZM, 1, 10, 4, 5 ) );
  QCOMPARE( points.at( 2 ), QgsPointV2( QgsWKBTypes::PointZM, 15, 10, 6, 7 ) );

  //CRS transform
  QgsCoordinateReferenceSystem sourceSrs;
  sourceSrs.createFromSrid( 3994 );
  QgsCoordinateReferenceSystem destSrs;
  destSrs.createFromSrid( 4326 );
  QgsCoordinateTransform tr( sourceSrs, destSrs );

  // 2d CRS transform
  QgsLineStringV2 l21;
  l21.setPoints( QList< QgsPointV2 >() << QgsPointV2( 6374985, -3626584 )
                 << QgsPointV2( 6474985, -3526584 ) );
  l21.transform( tr, QgsCoordinateTransform::ForwardTransform );
  QVERIFY( qgsDoubleNear( l21.pointN( 0 ).x(), 175.771, 0.001 ) );
  QVERIFY( qgsDoubleNear( l21.pointN( 0 ).y(), -39.722, 0.001 ) );
  QVERIFY( qgsDoubleNear( l21.pointN( 1 ).x(), 176.959, 0.001 ) );
  QVERIFY( qgsDoubleNear( l21.pointN( 1 ).y(), -38.798, 0.001 ) );

  //3d CRS transform
  QgsLineStringV2 l22;
  l22.setPoints( QList< QgsPointV2 >() << QgsPointV2( QgsWKBTypes::PointZM, 6374985, -3626584, 1, 2 )
                 << QgsPointV2( QgsWKBTypes::PointZM, 6474985, -3526584, 3, 4 ) );
  l22.transform( tr, QgsCoordinateTransform::ForwardTransform );
  QVERIFY( qgsDoubleNear( l22.pointN( 0 ).x(), 175.771, 0.001 ) );
  QVERIFY( qgsDoubleNear( l22.pointN( 0 ).y(), -39.722, 0.001 ) );
  QVERIFY( qgsDoubleNear( l22.pointN( 0 ).z(), 57.2958, 0.001 ) );
  QCOMPARE( l22.pointN( 0 ).m(), 2.0 );
  QVERIFY( qgsDoubleNear( l22.pointN( 1 ).x(), 176.959, 0.001 ) );
  QVERIFY( qgsDoubleNear( l22.pointN( 1 ).y(), -38.798, 0.001 ) );
  QVERIFY( qgsDoubleNear( l22.pointN( 1 ).z(), 171.887, 0.001 ) );
  QCOMPARE( l22.pointN( 1 ).m(), 4.0 );

  //reverse transform
  l22.transform( tr, QgsCoordinateTransform::ReverseTransform );
  QVERIFY( qgsDoubleNear( l22.pointN( 0 ).x(), 6374985, 0.01 ) );
  QVERIFY( qgsDoubleNear( l22.pointN( 0 ).y(), -3626584, 0.01 ) );
  QVERIFY( qgsDoubleNear( l22.pointN( 0 ).z(), 1, 0.001 ) );
  QCOMPARE( l22.pointN( 0 ).m(), 2.0 );
  QVERIFY( qgsDoubleNear( l22.pointN( 1 ).x(), 6474985, 0.01 ) );
  QVERIFY( qgsDoubleNear( l22.pointN( 1 ).y(), -3526584, 0.01 ) );
  QVERIFY( qgsDoubleNear( l22.pointN( 1 ).z(), 3, 0.001 ) );
  QCOMPARE( l22.pointN( 1 ).m(), 4.0 );

  //QTransform transform
  QTransform qtr = QTransform::fromScale( 2, 3 );
  QgsLineStringV2 l23;
  l23.setPoints( QList< QgsPointV2 >() << QgsPointV2( QgsWKBTypes::PointZM, 1, 2, 3, 4 )
                 << QgsPointV2( QgsWKBTypes::PointZM, 11, 12, 13, 14 ) );
  l23.transform( qtr );
  QCOMPARE( l23.pointN( 0 ), QgsPointV2( QgsWKBTypes::PointZM, 2, 6, 3, 4 ) );
  QCOMPARE( l23.pointN( 1 ), QgsPointV2( QgsWKBTypes::PointZM, 22, 36, 13, 14 ) );
}


void TestQgsGeometry::fromQgsPoint()
{
  QgsPoint point( 1.0, 2.0 );
  QSharedPointer<QgsGeometry> result( QgsGeometry::fromPoint( point ) );
  QCOMPARE( result->wkbType(), QGis::WKBPoint );
  QgsPoint resultPoint = result->asPoint();
  QCOMPARE( resultPoint, point );
}

void TestQgsGeometry::fromQPoint()
{
  QPointF point( 1.0, 2.0 );
  QSharedPointer<QgsGeometry> result( QgsGeometry::fromQPointF( point ) );
  QCOMPARE( result->wkbType(), QGis::WKBPoint );
  QgsPoint resultPoint = result->asPoint();
  QCOMPARE( resultPoint.x(), 1.0 );
  QCOMPARE( resultPoint.y(), 2.0 );
}

void TestQgsGeometry::fromQPolygonF()
{
  //test with a polyline
  QPolygonF polyline;
  polyline << QPointF( 1.0, 2.0 ) << QPointF( 4.0, 6.0 ) << QPointF( 4.0, 3.0 ) << QPointF( 2.0, 2.0 );
  QSharedPointer<QgsGeometry> result( QgsGeometry::fromQPolygonF( polyline ) );
  QCOMPARE( result->wkbType(), QGis::WKBLineString );
  QgsPolyline resultLine = result->asPolyline();
  QCOMPARE( resultLine.size(), 4 );
  QCOMPARE( resultLine.at( 0 ), QgsPoint( 1.0, 2.0 ) );
  QCOMPARE( resultLine.at( 1 ), QgsPoint( 4.0, 6.0 ) );
  QCOMPARE( resultLine.at( 2 ), QgsPoint( 4.0, 3.0 ) );
  QCOMPARE( resultLine.at( 3 ), QgsPoint( 2.0, 2.0 ) );

  //test with a closed polygon
  QPolygonF polygon;
  polygon << QPointF( 1.0, 2.0 ) << QPointF( 4.0, 6.0 ) << QPointF( 4.0, 3.0 ) << QPointF( 2.0, 2.0 ) << QPointF( 1.0, 2.0 );
  QSharedPointer<QgsGeometry> result2( QgsGeometry::fromQPolygonF( polygon ) );
  QCOMPARE( result2->wkbType(), QGis::WKBPolygon );
  QgsPolygon resultPolygon = result2->asPolygon();
  QCOMPARE( resultPolygon.size(), 1 );
  QCOMPARE( resultPolygon.at( 0 ).at( 0 ), QgsPoint( 1.0, 2.0 ) );
  QCOMPARE( resultPolygon.at( 0 ).at( 1 ), QgsPoint( 4.0, 6.0 ) );
  QCOMPARE( resultPolygon.at( 0 ).at( 2 ), QgsPoint( 4.0, 3.0 ) );
  QCOMPARE( resultPolygon.at( 0 ).at( 3 ), QgsPoint( 2.0, 2.0 ) );
  QCOMPARE( resultPolygon.at( 0 ).at( 4 ), QgsPoint( 1.0, 2.0 ) );
}

void TestQgsGeometry::asQPointF()
{
  QPointF point( 1.0, 2.0 );
  QSharedPointer<QgsGeometry> geom( QgsGeometry::fromQPointF( point ) );
  QPointF resultPoint = geom->asQPointF();
  QCOMPARE( resultPoint, point );

  //non point geom
  QPointF badPoint = mpPolygonGeometryA->asQPointF();
  QVERIFY( badPoint.isNull() );
}

void TestQgsGeometry::asQPolygonF()
{
  //test polygon
  QPolygonF fromPoly = mpPolygonGeometryA->asQPolygonF();
  QVERIFY( fromPoly.isClosed() );
  QCOMPARE( fromPoly.size(), 5 );
  QCOMPARE( fromPoly.at( 0 ).x(), mPoint1.x() );
  QCOMPARE( fromPoly.at( 0 ).y(), mPoint1.y() );
  QCOMPARE( fromPoly.at( 1 ).x(), mPoint2.x() );
  QCOMPARE( fromPoly.at( 1 ).y(), mPoint2.y() );
  QCOMPARE( fromPoly.at( 2 ).x(), mPoint3.x() );
  QCOMPARE( fromPoly.at( 2 ).y(), mPoint3.y() );
  QCOMPARE( fromPoly.at( 3 ).x(), mPoint4.x() );
  QCOMPARE( fromPoly.at( 3 ).y(), mPoint4.y() );
  QCOMPARE( fromPoly.at( 4 ).x(), mPoint1.x() );
  QCOMPARE( fromPoly.at( 4 ).y(), mPoint1.y() );

  //test polyline
  QgsPolyline testline;
  testline << mPoint1 << mPoint2 << mPoint3;
  QSharedPointer<QgsGeometry> lineGeom( QgsGeometry::fromPolyline( testline ) );
  QPolygonF fromLine = lineGeom->asQPolygonF();
  QVERIFY( !fromLine.isClosed() );
  QCOMPARE( fromLine.size(), 3 );
  QCOMPARE( fromLine.at( 0 ).x(), mPoint1.x() );
  QCOMPARE( fromLine.at( 0 ).y(), mPoint1.y() );
  QCOMPARE( fromLine.at( 1 ).x(), mPoint2.x() );
  QCOMPARE( fromLine.at( 1 ).y(), mPoint2.y() );
  QCOMPARE( fromLine.at( 2 ).x(), mPoint3.x() );
  QCOMPARE( fromLine.at( 2 ).y(), mPoint3.y() );

  //test a bad geometry
  QSharedPointer<QgsGeometry> badGeom( QgsGeometry::fromPoint( mPoint1 ) );
  QPolygonF fromBad = badGeom->asQPolygonF();
  QVERIFY( fromBad.isEmpty() );
}

void TestQgsGeometry::comparePolylines()
{
  QgsPolyline line1;
  line1 << mPoint1 << mPoint2 << mPoint3;
  QgsPolyline line2;
  line2 << mPoint1 << mPoint2 << mPoint3;
  QVERIFY( QgsGeometry::compare( line1, line2 ) );

  //different number of nodes
  QgsPolyline line3;
  line3 << mPoint1 << mPoint2 << mPoint3 << mPoint4;
  QVERIFY( !QgsGeometry::compare( line1, line3 ) );

  //different nodes
  QgsPolyline line4;
  line3 << mPoint1 << mPointA << mPoint3 << mPoint4;
  QVERIFY( !QgsGeometry::compare( line3, line4 ) );
}

void TestQgsGeometry::comparePolygons()
{
  QgsPolyline ring1;
  ring1 << mPoint1 << mPoint2 << mPoint3 << mPoint1;
  QgsPolyline ring2;
  ring2 << mPoint4 << mPointA << mPointB << mPoint4;
  QgsPolygon poly1;
  poly1 << ring1 << ring2;
  QgsPolygon poly2;
  poly2 << ring1 << ring2;
  QVERIFY( QgsGeometry::compare( poly1, poly2 ) );

  //different number of rings
  QgsPolygon poly3;
  poly3 << ring1;
  QVERIFY( !QgsGeometry::compare( poly1, poly3 ) );

  //different rings
  QgsPolygon poly4;
  poly4 << ring2;
  QVERIFY( !QgsGeometry::compare( poly3, poly4 ) );
}



// MK, Disabled 14.11.2014
// Too unclear what exactly should be tested and which variations are allowed for the line
#if 0
void TestQgsGeometry::simplifyCheck1()
{
  QVERIFY( mpPolylineGeometryD->simplify( 0.5 ) );
  // should be a single polygon as A intersect B
  QgsGeometry * mypSimplifyGeometry  =  mpPolylineGeometryD->simplify( 0.5 );
  qDebug( "Geometry Type: %s", QGis::featureType( mypSimplifyGeometry->wkbType() ) );
  QVERIFY( mypSimplifyGeometry->wkbType() == QGis::WKBLineString );
  QgsPolyline myLine = mypSimplifyGeometry->asPolyline();
  QVERIFY( myLine.size() > 0 ); //check that the union created a feature
  dumpPolyline( myLine );
  delete mypSimplifyGeometry;
  QVERIFY( renderCheck( "geometry_simplifyCheck1", "Checking simplify of line" ) );
}
#endif

void TestQgsGeometry::intersectionCheck1()
{
  QVERIFY( mpPolygonGeometryA->intersects( mpPolygonGeometryB ) );
  // should be a single polygon as A intersect B
  QgsGeometry * mypIntersectionGeometry  =  mpPolygonGeometryA->intersection( mpPolygonGeometryB );
  qDebug( "Geometry Type: %s", QGis::featureType( mypIntersectionGeometry->wkbType() ) );
  QVERIFY( mypIntersectionGeometry->wkbType() == QGis::WKBPolygon );
  QgsPolygon myPolygon = mypIntersectionGeometry->asPolygon();
  QVERIFY( myPolygon.size() > 0 ); //check that the union created a feature
  dumpPolygon( myPolygon );
  delete mypIntersectionGeometry;
  QVERIFY( renderCheck( "geometry_intersectionCheck1", "Checking if A intersects B" ) );
}
void TestQgsGeometry::intersectionCheck2()
{
  QVERIFY( !mpPolygonGeometryA->intersects( mpPolygonGeometryC ) );
}

void TestQgsGeometry::translateCheck1()
{
  QString wkt = "LineString (0 0, 10 0, 10 10)";
  QScopedPointer<QgsGeometry> geom( QgsGeometry::fromWkt( wkt ) );
  geom->translate( 10, -5 );
  QString obtained = geom->exportToWkt();
  QString expected = "LineString (10 -5, 20 -5, 20 5)";
  QCOMPARE( obtained, expected );
  geom->translate( -10, 5 );
  obtained = geom->exportToWkt();
  QCOMPARE( obtained, wkt );

  wkt = "Polygon ((-2 4, -2 -10, 2 3, -2 4),(1 1, -1 1, -1 -1, 1 1))";
  geom.reset( QgsGeometry::fromWkt( wkt ) );
  geom->translate( -2, 10 );
  obtained = geom->exportToWkt();
  expected = "Polygon ((-4 14, -4 0, 0 13, -4 14),(-1 11, -3 11, -3 9, -1 11))";
  QCOMPARE( obtained, expected );
  geom->translate( 2, -10 );
  obtained = geom->exportToWkt();
  QCOMPARE( obtained, wkt );

  wkt = "Point (40 50)";
  geom.reset( QgsGeometry::fromWkt( wkt ) );
  geom->translate( -2, 10 );
  obtained = geom->exportToWkt();
  expected = "Point (38 60)";
  QCOMPARE( obtained, expected );
  geom->translate( 2, -10 );
  obtained = geom->exportToWkt();
  QCOMPARE( obtained, wkt );

}

void TestQgsGeometry::rotateCheck1()
{
  QString wkt = "LineString (0 0, 10 0, 10 10)";
  QScopedPointer<QgsGeometry> geom( QgsGeometry::fromWkt( wkt ) );
  geom->rotate( 90, QgsPoint( 0, 0 ) );
  QString obtained = geom->exportToWkt();
  QString expected = "LineString (0 0, 0 -10, 10 -10)";
  QCOMPARE( obtained, expected );
  geom->rotate( -90, QgsPoint( 0, 0 ) );
  obtained = geom->exportToWkt();
  QCOMPARE( obtained, wkt );

  wkt = "Polygon ((-2 4, -2 -10, 2 3, -2 4),(1 1, -1 1, -1 -1, 1 1))";
  geom.reset( QgsGeometry::fromWkt( wkt ) );
  geom->rotate( 90, QgsPoint( 0, 0 ) );
  obtained = geom->exportToWkt();
  expected = "Polygon ((4 2, -10 2, 3 -2, 4 2),(1 -1, 1 1, -1 1, 1 -1))";
  QCOMPARE( obtained, expected );
  geom->rotate( -90, QgsPoint( 0, 0 ) );
  obtained = geom->exportToWkt();
  QCOMPARE( obtained, wkt );

  wkt = "Point (40 50)";
  geom.reset( QgsGeometry::fromWkt( wkt ) );
  geom->rotate( 90, QgsPoint( 0, 0 ) );
  obtained = geom->exportToWkt();
  expected = "Point (50 -40)";
  QCOMPARE( obtained, expected );
  geom->rotate( -90, QgsPoint( 0, 0 ) );
  obtained = geom->exportToWkt();
  QCOMPARE( obtained, wkt );
  geom->rotate( 180, QgsPoint( 40, 0 ) );
  expected = "Point (40 -50)";
  obtained = geom->exportToWkt();
  QCOMPARE( obtained, expected );
  geom->rotate( 180, QgsPoint( 40, 0 ) ); // round-trip
  obtained = geom->exportToWkt();
  QCOMPARE( obtained, wkt );

}

void TestQgsGeometry::unionCheck1()
{
  // should be a multipolygon with 2 parts as A does not intersect C
  QgsGeometry * mypUnionGeometry  =  mpPolygonGeometryA->combine( mpPolygonGeometryC );
  qDebug( "Geometry Type: %s", QGis::featureType( mypUnionGeometry->wkbType() ) );
  QVERIFY( mypUnionGeometry->wkbType() == QGis::WKBMultiPolygon );
  QgsMultiPolygon myMultiPolygon = mypUnionGeometry->asMultiPolygon();
  QVERIFY( myMultiPolygon.size() > 0 ); //check that the union did not fail
  dumpMultiPolygon( myMultiPolygon );
  delete mypUnionGeometry;
  QVERIFY( renderCheck( "geometry_unionCheck1", "Checking A union C produces 2 polys" ) );
}

void TestQgsGeometry::unionCheck2()
{
  // should be a single polygon as A intersect B
  QgsGeometry * mypUnionGeometry  =  mpPolygonGeometryA->combine( mpPolygonGeometryB );
  qDebug( "Geometry Type: %s", QGis::featureType( mypUnionGeometry->wkbType() ) );
  QVERIFY( mypUnionGeometry->wkbType() == QGis::WKBPolygon );
  QgsPolygon myPolygon = mypUnionGeometry->asPolygon();
  QVERIFY( myPolygon.size() > 0 ); //check that the union created a feature
  dumpPolygon( myPolygon );
  delete mypUnionGeometry;
  QVERIFY( renderCheck( "geometry_unionCheck2", "Checking A union B produces single union poly" ) );
}

void TestQgsGeometry::differenceCheck1()
{
  // should be same as A since A does not intersect C so diff is 100% of A
  QSharedPointer<QgsGeometry> mypDifferenceGeometry( mpPolygonGeometryA->difference( mpPolygonGeometryC ) );
  qDebug( "Geometry Type: %s", QGis::featureType( mypDifferenceGeometry->wkbType() ) );
  QVERIFY( mypDifferenceGeometry->wkbType() == QGis::WKBPolygon );
  QgsPolygon myPolygon = mypDifferenceGeometry->asPolygon();
  QVERIFY( myPolygon.size() > 0 ); //check that the union did not fail
  dumpPolygon( myPolygon );
  QVERIFY( renderCheck( "geometry_differenceCheck1", "Checking (A - C) = A" ) );
}

void TestQgsGeometry::differenceCheck2()
{
  // should be a single polygon as (A - B) = subset of A
  QSharedPointer<QgsGeometry> mypDifferenceGeometry( mpPolygonGeometryA->difference( mpPolygonGeometryB ) );
  qDebug( "Geometry Type: %s", QGis::featureType( mypDifferenceGeometry->wkbType() ) );
  QVERIFY( mypDifferenceGeometry->wkbType() == QGis::WKBPolygon );
  QgsPolygon myPolygon = mypDifferenceGeometry->asPolygon();
  QVERIFY( myPolygon.size() > 0 ); //check that the union created a feature
  dumpPolygon( myPolygon );
  QVERIFY( renderCheck( "geometry_differenceCheck2", "Checking (A - B) = subset of A" ) );
}
void TestQgsGeometry::bufferCheck()
{
  // should be a single polygon
  QSharedPointer<QgsGeometry> mypBufferGeometry( mpPolygonGeometryB->buffer( 10, 10 ) );
  qDebug( "Geometry Type: %s", QGis::featureType( mypBufferGeometry->wkbType() ) );
  QVERIFY( mypBufferGeometry->wkbType() == QGis::WKBPolygon );
  QgsPolygon myPolygon = mypBufferGeometry->asPolygon();
  QVERIFY( myPolygon.size() > 0 ); //check that the buffer created a feature
  dumpPolygon( myPolygon );
  QVERIFY( renderCheck( "geometry_bufferCheck", "Checking buffer(10,10) of B", 10 ) );
}

void TestQgsGeometry::smoothCheck()
{
  //can't smooth a point
  QString wkt = "Point (40 50)";
  QScopedPointer<QgsGeometry> geom( QgsGeometry::fromWkt( wkt ) );
  QgsGeometry* result = geom->smooth( 1, 0.25 );
  QString obtained = result->exportToWkt();
  delete result;
  QCOMPARE( obtained, wkt );

  //linestring
  wkt = "LineString(0 0, 10 0, 10 10, 20 10)";
  geom.reset( QgsGeometry::fromWkt( wkt ) );
  result = geom->smooth( 1, 0.25 );
  QgsPolyline line = result->asPolyline();
  delete result;
  QgsPolyline expectedLine;
  expectedLine << QgsPoint( 0, 0 ) << QgsPoint( 7.5, 0 ) << QgsPoint( 10.0, 2.5 )
  << QgsPoint( 10.0, 7.5 ) << QgsPoint( 12.5, 10.0 ) << QgsPoint( 20.0, 10.0 );
  QVERIFY( QgsGeometry::compare( line, expectedLine ) );

  wkt = "MultiLineString ((0 0, 10 0, 10 10, 20 10),(30 30, 40 30, 40 40, 50 40))";
  geom.reset( QgsGeometry::fromWkt( wkt ) );
  result = geom->smooth( 1, 0.25 );
  QgsMultiPolyline multiLine = result->asMultiPolyline();
  delete result;
  QgsMultiPolyline expectedMultiline;
  expectedMultiline << ( QgsPolyline() << QgsPoint( 0, 0 ) << QgsPoint( 7.5, 0 ) << QgsPoint( 10.0, 2.5 )
                         <<  QgsPoint( 10.0, 7.5 ) << QgsPoint( 12.5, 10.0 ) << QgsPoint( 20.0, 10.0 ) )
  << ( QgsPolyline() << QgsPoint( 30.0, 30.0 ) << QgsPoint( 37.5, 30.0 ) << QgsPoint( 40.0, 32.5 )
       << QgsPoint( 40.0, 37.5 ) << QgsPoint( 42.5, 40.0 ) << QgsPoint( 50.0, 40.0 ) );
  QVERIFY( QgsGeometry::compare( multiLine, expectedMultiline ) );

  //polygon
  wkt = "Polygon ((0 0, 10 0, 10 10, 0 10, 0 0 ),(2 2, 4 2, 4 4, 2 4, 2 2))";
  geom.reset( QgsGeometry::fromWkt( wkt ) );
  result = geom->smooth( 1, 0.25 );
  QgsPolygon poly = result->asPolygon();
  delete result;
  QgsPolygon expectedPolygon;
  expectedPolygon << ( QgsPolyline() << QgsPoint( 2.5, 0 ) << QgsPoint( 7.5, 0 ) << QgsPoint( 10.0, 2.5 )
                       <<  QgsPoint( 10.0, 7.5 ) << QgsPoint( 7.5, 10.0 ) << QgsPoint( 2.5, 10.0 ) << QgsPoint( 0, 7.5 )
                       << QgsPoint( 0, 2.5 ) << QgsPoint( 2.5, 0 ) )
  << ( QgsPolyline() << QgsPoint( 2.5, 2.0 ) << QgsPoint( 3.5, 2.0 ) << QgsPoint( 4.0, 2.5 )
       << QgsPoint( 4.0, 3.5 ) << QgsPoint( 3.5, 4.0 ) << QgsPoint( 2.5, 4.0 )
       << QgsPoint( 2.0, 3.5 ) << QgsPoint( 2.0, 2.5 ) << QgsPoint( 2.5, 2.0 ) );
  QVERIFY( QgsGeometry::compare( poly, expectedPolygon ) );

  //multipolygon
  wkt = "MultiPolygon (((0 0, 10 0, 10 10, 0 10, 0 0 )),((2 2, 4 2, 4 4, 2 4, 2 2)))";
  geom.reset( QgsGeometry::fromWkt( wkt ) );
  result = geom->smooth( 1, 0.1 );
  QgsMultiPolygon multipoly = result->asMultiPolygon();
  delete result;
  QgsMultiPolygon expectedMultiPoly;
  expectedMultiPoly << ( QgsPolygon() << ( QgsPolyline() << QgsPoint( 1.0, 0 ) << QgsPoint( 9, 0 ) << QgsPoint( 10.0, 1 )
                         <<  QgsPoint( 10.0, 9 ) << QgsPoint( 9, 10.0 ) << QgsPoint( 1, 10.0 ) << QgsPoint( 0, 9 )
                         << QgsPoint( 0, 1 ) << QgsPoint( 1, 0 ) ) )
  << ( QgsPolygon() << ( QgsPolyline() << QgsPoint( 2.2, 2.0 ) << QgsPoint( 3.8, 2.0 ) << QgsPoint( 4.0, 2.2 )
                         <<  QgsPoint( 4.0, 3.8 ) << QgsPoint( 3.8, 4.0 ) << QgsPoint( 2.2, 4.0 ) << QgsPoint( 2.0, 3.8 )
                         << QgsPoint( 2, 2.2 ) << QgsPoint( 2.2, 2 ) ) );
  QVERIFY( QgsGeometry::compare( multipoly, expectedMultiPoly ) );
}

void TestQgsGeometry::dataStream()
{
  QString wkt = "Point (40 50)";
  QScopedPointer<QgsGeometry> geom( QgsGeometry::fromWkt( wkt ) );

  QByteArray ba;
  QDataStream ds( &ba, QIODevice::ReadWrite );
  ds << *geom;

  QgsGeometry resultGeometry;
  ds.device()->seek( 0 );
  ds >> resultGeometry;

  QCOMPARE( geom->geometry()->asWkt(), resultGeometry.geometry()->asWkt( ) );

  //also test with geometry without data
  QScopedPointer<QgsGeometry> emptyGeom( new QgsGeometry() );

  QByteArray ba2;
  QDataStream ds2( &ba2, QIODevice::ReadWrite );
  ds2 << emptyGeom;

  ds2.device()->seek( 0 );
  ds2 >> resultGeometry;

  QVERIFY( resultGeometry.isEmpty() );
}

void TestQgsGeometry::exportToGeoJSON()
{
  //Point
  QString wkt = "Point (40 50)";
  QScopedPointer<QgsGeometry> geom( QgsGeometry::fromWkt( wkt ) );
  QString obtained = geom->exportToGeoJSON();
  QString geojson = "{\"type\": \"Point\", \"coordinates\": [40, 50]}";
  QCOMPARE( obtained, geojson );

  //MultiPoint
  wkt = "MultiPoint (0 0, 10 0, 10 10, 20 10)";
  geom.reset( QgsGeometry::fromWkt( wkt ) );
  obtained = geom->exportToGeoJSON();
  geojson = "{\"type\": \"MultiPoint\", \"coordinates\": [ [0, 0], [10, 0], [10, 10], [20, 10]] }";
  QCOMPARE( obtained, geojson );

  //Linestring
  wkt = "LineString(0 0, 10 0, 10 10, 20 10)";
  geom.reset( QgsGeometry::fromWkt( wkt ) );
  obtained = geom->exportToGeoJSON();
  geojson = "{\"type\": \"LineString\", \"coordinates\": [ [0, 0], [10, 0], [10, 10], [20, 10]]}";
  QCOMPARE( obtained, geojson );

  //MultiLineString
  wkt = "MultiLineString ((0 0, 10 0, 10 10, 20 10),(30 30, 40 30, 40 40, 50 40))";
  geom.reset( QgsGeometry::fromWkt( wkt ) );
  obtained = geom->exportToGeoJSON();
  geojson = "{\"type\": \"MultiLineString\", \"coordinates\": [[ [0, 0], [10, 0], [10, 10], [20, 10]], [ [30, 30], [40, 30], [40, 40], [50, 40]]] }";
  QCOMPARE( obtained, geojson );

  //Polygon
  wkt = "Polygon ((0 0, 10 0, 10 10, 0 10, 0 0 ),(2 2, 4 2, 4 4, 2 4, 2 2))";
  geom.reset( QgsGeometry::fromWkt( wkt ) );
  obtained = geom->exportToGeoJSON();
  geojson = "{\"type\": \"Polygon\", \"coordinates\": [[ [0, 0], [10, 0], [10, 10], [0, 10], [0, 0]], [ [2, 2], [4, 2], [4, 4], [2, 4], [2, 2]]] }";
  QCOMPARE( obtained, geojson );

  //MultiPolygon
  wkt = "MultiPolygon (((0 0, 10 0, 10 10, 0 10, 0 0 )),((2 2, 4 2, 4 4, 2 4, 2 2)))";
  geom.reset( QgsGeometry::fromWkt( wkt ) );
  obtained = geom->exportToGeoJSON();
  geojson = "{\"type\": \"MultiPolygon\", \"coordinates\": [[[ [0, 0], [10, 0], [10, 10], [0, 10], [0, 0]]], [[ [2, 2], [4, 2], [4, 4], [2, 4], [2, 2]]]] }";
  QCOMPARE( obtained, geojson );
}

bool TestQgsGeometry::renderCheck( const QString& theTestName, const QString& theComment, int mismatchCount )
{
  mReport += "<h2>" + theTestName + "</h2>\n";
  mReport += "<h3>" + theComment + "</h3>\n";
  QString myTmpDir = QDir::tempPath() + '/';
  QString myFileName = myTmpDir + theTestName + ".png";
  mImage.save( myFileName, "PNG" );
  QgsRenderChecker myChecker;
  myChecker.setControlName( "expected_" + theTestName );
  myChecker.setRenderedImage( myFileName );
  bool myResultFlag = myChecker.compareImages( theTestName, mismatchCount );
  mReport += myChecker.report();
  return myResultFlag;
}

void TestQgsGeometry::dumpMultiPolygon( QgsMultiPolygon &theMultiPolygon )
{
  qDebug( "Multipolygon Geometry Dump" );
  for ( int i = 0; i < theMultiPolygon.size(); i++ )
  {
    QgsPolygon myPolygon = theMultiPolygon.at( i );
    qDebug( "\tPolygon in multipolygon: %d", i );
    dumpPolygon( myPolygon );
  }
}

void TestQgsGeometry::dumpPolygon( QgsPolygon &thePolygon )
{
  QVector<QPointF> myPoints;
  for ( int j = 0; j < thePolygon.size(); j++ )
  {
    QgsPolyline myPolyline = thePolygon.at( j ); //rings of polygon
    qDebug( "\t\tRing in polygon: %d", j );

    for ( int k = 0; k < myPolyline.size(); k++ )
    {
      QgsPoint myPoint = myPolyline.at( k );
      qDebug( "\t\t\tPoint in ring %d : %s", k, myPoint.toString().toLocal8Bit().constData() );
      myPoints << QPointF( myPoint.x(), myPoint.y() );
    }
  }
  mpPainter->drawPolygon( myPoints );
}

void TestQgsGeometry::dumpPolyline( QgsPolyline &thePolyline )
{
  QVector<QPointF> myPoints;
//  QgsPolyline myPolyline = thePolyline.at( j ); //rings of polygon
  for ( int j = 0; j < thePolyline.size(); j++ )
  {
    QgsPoint myPoint = thePolyline.at( j );
//    QgsPolyline myPolyline = thePolygon.at( j ); //rings of polygon
    myPoints << QPointF( myPoint.x(), myPoint.y() );
    qDebug( "\t\tPoint in line: %d", j );

//    for ( int k = 0; k < myPolyline.size(); k++ )
//    {
//      QgsPoint myPoint = myPolyline.at( k );
//      qDebug( "\t\t\tPoint in ring %d : %s", k, myPoint.toString().toLocal8Bit().constData() );
//      myPoints << QPointF( myPoint.x(), myPoint.y() );
//    }
  }
  mpPainter->drawPolyline( myPoints );
}

QString TestQgsGeometry::elemToString( const QDomElement& elem ) const
{
  QString s;
  QTextStream stream( &s );
  elem.save( stream, -1 );

  return s;
}

QTEST_MAIN( TestQgsGeometry )
#include "testqgsgeometry.moc"
