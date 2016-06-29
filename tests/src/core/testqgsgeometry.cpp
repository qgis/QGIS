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
#include "qgsgeometryutils.h"
#include <qgspoint.h>
#include "qgspointv2.h"
#include "qgslinestringv2.h"
#include "qgspolygonv2.h"
#include "qgscircularstringv2.h"
#include "qgsgeometrycollectionv2.h"
#include "qgsgeometryfactory.h"
#include "qgstestutils.h"

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
    void polygonV2(); //test QgsPolygonV2

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

    void wkbInOut();

    void segmentizeCircularString();

  private:
    /** A helper method to do a render check to see if the geometry op is as expected */
    bool renderCheck( const QString& theTestName, const QString& theComment = "", int mismatchCount = 0 );
    /** A helper method to dump to qdebug the geometry of a multipolygon */
    void dumpMultiPolygon( QgsMultiPolygon &theMultiPolygon );
    /** A helper method to dump to qdebug the geometry of a polygon */
    void dumpPolygon( QgsPolygon &thePolygon );
    /** A helper method to dump to qdebug the geometry of a polyline */
    void dumpPolyline( QgsPolyline &thePolyline );

    // Release return with delete []
    unsigned char * hex2bytes( const char *hex, int *size )
    {
      QByteArray ba = QByteArray::fromHex( hex );
      unsigned char *out = new unsigned char[ba.size()];
      memcpy( out, ba.data(), ba.size() );
      *size = ba.size();
      return out;
    }

    QString bytes2hex( const unsigned char *bytes, int size )
    {
      QByteArray ba(( const char * )bytes, size );
      QString out = ba.toHex();
      return out;
    }


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
    : mpPolylineGeometryD( nullptr )
    , mpPolygonGeometryA( nullptr )
    , mpPolygonGeometryB( nullptr )
    , mpPolygonGeometryC( nullptr )
    , mpPainter( nullptr )
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

  QgsGeometryCollectionV2 collection;
  QVERIFY( collection.isEmpty() );
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
  QCOMPARE( p11.wkbType(), QgsWKBTypes::Point );
  QCOMPARE( p11.x(), 0.0 );
  QCOMPARE( p11.y(), 0.0 );

  //toQPointF
  QgsPointV2 p11a( 5.0, 9.0 );
  QPointF result = p11a.toQPointF();
  QVERIFY( qgsDoubleNear( result.x(), 5.0 ) );
  QVERIFY( qgsDoubleNear( result.y(), 9.0 ) );

  //to/from WKB
  QgsPointV2 p12( QgsWKBTypes::PointZM, 1.0, 2.0, 3.0, -4.0 );
  int size = 0;
  unsigned char* wkb = p12.asWkb( size );
  QCOMPARE( size, p12.wkbSize() );
  QgsPointV2 p13;
  p13.fromWkb( QgsConstWkbPtr( wkb, size ) );
  delete[] wkb;
  wkb = 0;
  QVERIFY( p13 == p12 );

  //bad WKB - check for no crash
  p13 = QgsPointV2( 1, 2 );
  QVERIFY( !p13.fromWkb( QgsConstWkbPtr( nullptr, 0 ) ) );
  QCOMPARE( p13.wkbType(), QgsWKBTypes::Point );
  QgsLineStringV2 line;
  p13 = QgsPointV2( 1, 2 );
  wkb = line.asWkb( size );
  QVERIFY( !p13.fromWkb( QgsConstWkbPtr( wkb, size ) ) );
  delete[] wkb;
  wkb = 0;
  QCOMPARE( p13.wkbType(), QgsWKBTypes::Point );

  //to/from WKT
  p13 = QgsPointV2( QgsWKBTypes::PointZM, 1.0, 2.0, 3.0, -4.0 );
  QString wkt = p13.asWkt();
  QVERIFY( !wkt.isEmpty() );
  QgsPointV2 p14;
  QVERIFY( p14.fromWkt( wkt ) );
  QVERIFY( p14 == p13 );

  //bad WKT
  QVERIFY( !p14.fromWkt( "Polygon()" ) );

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
  destSrs.createFromSrid( 4202 ); // want a transform with ellipsoid change
  QgsCoordinateTransform tr( sourceSrs, destSrs );
  QgsPointV2 p16( QgsWKBTypes::PointZM, 6374985, -3626584, 1, 2 );
  p16.transform( tr, QgsCoordinateTransform::ForwardTransform );
  QGSCOMPARENEAR( p16.x(), 175.771, 0.001 );
  QGSCOMPARENEAR( p16.y(), -39.724, 0.001 );
  QGSCOMPARENEAR( p16.z(), 1.0, 0.001 );
  QCOMPARE( p16.m(), 2.0 );
  p16.transform( tr, QgsCoordinateTransform::ReverseTransform );
  QGSCOMPARENEAR( p16.x(), 6374985, 1 );
  QGSCOMPARENEAR( p16.y(), -3626584, 1 );
  QGSCOMPARENEAR( p16.z(), 1.0, 0.001 );
  QCOMPARE( p16.m(), 2.0 );
  //test with z transform
  p16.transform( tr, QgsCoordinateTransform::ForwardTransform, true );
  QGSCOMPARENEAR( p16.z(), -19.249, 0.001 );
  p16.transform( tr, QgsCoordinateTransform::ReverseTransform, true );
  QGSCOMPARENEAR( p16.z(), 1.0, 0.001 );

  //QTransform transform
  QTransform qtr = QTransform::fromScale( 2, 3 );
  QgsPointV2 p17( QgsWKBTypes::PointZM, 10, 20, 30, 40 );
  p17.transform( qtr );
  QVERIFY( p17 == QgsPointV2( QgsWKBTypes::PointZM, 20, 60, 30, 40 ) );

  //coordinateSequence
  QgsPointV2 p18( QgsWKBTypes::PointZM, 1.0, 2.0, 3.0, 4.0 );
  QgsCoordinateSequenceV2 coord = p18.coordinateSequence();
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
  v = QgsVertexId( 0, 1, -1 ); //test that ring number is maintained
  QVERIFY( p21.nextVertex( v, p22 ) );
  QCOMPARE( p22, p21 );
  QCOMPARE( v, QgsVertexId( 0, 1, 0 ) );
  v = QgsVertexId( 1, 0, -1 ); //test that part number is maintained
  QVERIFY( p21.nextVertex( v, p22 ) );
  QCOMPARE( p22, p21 );
  QCOMPARE( v, QgsVertexId( 1, 0, 0 ) );

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

  //dropZ
  QgsPointV2 p25( QgsWKBTypes::PointZ, 1.0, 2.0, 3.0 );
  QVERIFY( p25.dropZValue() );
  QCOMPARE( p25, QgsPointV2( 1.0, 2.0 ) );
  QVERIFY( !p25.dropZValue() );
  QgsPointV2 p26( QgsWKBTypes::PointZM, 1.0, 2.0, 3.0, 4.0 );
  QVERIFY( p26.dropZValue() );
  QCOMPARE( p26, QgsPointV2( QgsWKBTypes::PointM, 1.0, 2.0, 0.0, 4.0 ) );
  QVERIFY( !p26.dropZValue() );
  QgsPointV2 p26a( QgsWKBTypes::Point25D, 1.0, 2.0, 3.0 );
  QVERIFY( p26a.dropZValue() );
  QCOMPARE( p26a, QgsPointV2( QgsWKBTypes::Point, 1.0, 2.0 ) );
  QVERIFY( !p26a.dropZValue() );

  //dropM
  QgsPointV2 p27( QgsWKBTypes::PointM, 1.0, 2.0, 0.0, 3.0 );
  QVERIFY( p27.dropMValue() );
  QCOMPARE( p27, QgsPointV2( 1.0, 2.0 ) );
  QVERIFY( !p27.dropMValue() );
  QgsPointV2 p28( QgsWKBTypes::PointZM, 1.0, 2.0, 3.0, 4.0 );
  QVERIFY( p28.dropMValue() );
  QCOMPARE( p28, QgsPointV2( QgsWKBTypes::PointZ, 1.0, 2.0, 3.0, 0.0 ) );
  QVERIFY( !p28.dropMValue() );

  //convertTo
  QgsPointV2 p29( 1.0, 2.0 );
  QVERIFY( p29.convertTo( QgsWKBTypes::Point ) );
  QCOMPARE( p29.wkbType(), QgsWKBTypes::Point );
  QVERIFY( p29.convertTo( QgsWKBTypes::PointZ ) );
  QCOMPARE( p29.wkbType(), QgsWKBTypes::PointZ );
  p29.setZ( 5.0 );
  QVERIFY( p29.convertTo( QgsWKBTypes::Point25D ) );
  QCOMPARE( p29.wkbType(), QgsWKBTypes::Point25D );
  QCOMPARE( p29.z(), 5.0 );
  QVERIFY( p29.convertTo( QgsWKBTypes::PointZM ) );
  QCOMPARE( p29.wkbType(), QgsWKBTypes::PointZM );
  QCOMPARE( p29.z(), 5.0 );
  p29.setM( 9.0 );
  QVERIFY( p29.convertTo( QgsWKBTypes::PointM ) );
  QCOMPARE( p29.wkbType(), QgsWKBTypes::PointM );
  QCOMPARE( p29.z(), 0.0 );
  QCOMPARE( p29.m(), 9.0 );
  QVERIFY( p29.convertTo( QgsWKBTypes::Point ) );
  QCOMPARE( p29.wkbType(), QgsWKBTypes::Point );
  QCOMPARE( p29.z(), 0.0 );
  QCOMPARE( p29.m(), 0.0 );
  QVERIFY( !p29.convertTo( QgsWKBTypes::Polygon ) );
}

void TestQgsGeometry::lineStringV2()
{
  //test constructors
  QgsLineStringV2 l1;
  QVERIFY( l1.isEmpty() );
  QCOMPARE( l1.numPoints(), 0 );
  QCOMPARE( l1.vertexCount(), 0 );
  QCOMPARE( l1.nCoordinates(), 0 );
  QCOMPARE( l1.ringCount(), 0 );
  QCOMPARE( l1.partCount(), 0 );
  QVERIFY( !l1.is3D() );
  QVERIFY( !l1.isMeasure() );
  QCOMPARE( l1.wkbType(), QgsWKBTypes::LineString );
  QCOMPARE( l1.wktTypeStr(), QString( "LineString" ) );
  QCOMPARE( l1.geometryType(), QString( "LineString" ) );
  QCOMPARE( l1.dimension(), 1 );
  QVERIFY( !l1.hasCurvedSegments() );
  QCOMPARE( l1.area(), 0.0 );
  QCOMPARE( l1.perimeter(), 0.0 );

  //addVertex
  QgsLineStringV2 l2;
  l2.addVertex( QgsPointV2( 1.0, 2.0 ) );
  QVERIFY( !l2.isEmpty() );
  QCOMPARE( l2.numPoints(), 1 );
  QCOMPARE( l2.vertexCount(), 1 );
  QCOMPARE( l2.nCoordinates(), 1 );
  QCOMPARE( l2.ringCount(), 1 );
  QCOMPARE( l2.partCount(), 1 );
  QVERIFY( !l2.is3D() );
  QVERIFY( !l2.isMeasure() );
  QCOMPARE( l2.wkbType(), QgsWKBTypes::LineString );
  QVERIFY( !l2.hasCurvedSegments() );
  QCOMPARE( l2.area(), 0.0 );
  QCOMPARE( l2.perimeter(), 0.0 );

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

  QgsLineStringV2 l25d;
  l25d.addVertex( QgsPointV2( QgsWKBTypes::Point25D, 1.0, 2.0, 3.0 ) );
  QVERIFY( !l25d.isEmpty() );
  QVERIFY( l25d.is3D() );
  QVERIFY( !l25d.isMeasure() );
  QCOMPARE( l25d.wkbType(), QgsWKBTypes::LineString25D );
  QCOMPARE( l25d.wktTypeStr(), QString( "LineStringZ" ) );

  //adding subsequent vertices should not alter z/m type, regardless of points type
  QgsLineStringV2 l6;
  l6.addVertex( QgsPointV2( QgsWKBTypes::Point, 1.0, 2.0 ) ); //2d type
  QCOMPARE( l6.wkbType(), QgsWKBTypes::LineString );
  l6.addVertex( QgsPointV2( QgsWKBTypes::PointZ, 11.0, 12.0, 13.0 ) ); // add 3d point
  QCOMPARE( l6.numPoints(), 2 );
  QCOMPARE( l6.vertexCount(), 2 );
  QCOMPARE( l6.nCoordinates(), 2 );
  QCOMPARE( l6.ringCount(), 1 );
  QCOMPARE( l6.partCount(), 1 );
  QCOMPARE( l6.wkbType(), QgsWKBTypes::LineString ); //should still be 2d
  QVERIFY( !l6.is3D() );
  QCOMPARE( l6.area(), 0.0 );
  QCOMPARE( l6.perimeter(), 0.0 );

  QgsLineStringV2 l7;
  l7.addVertex( QgsPointV2( QgsWKBTypes::PointZ, 1.0, 2.0, 3.0 ) ); //3d type
  QCOMPARE( l7.wkbType(), QgsWKBTypes::LineStringZ );
  l7.addVertex( QgsPointV2( QgsWKBTypes::Point, 11.0, 12.0 ) ); //add 2d point
  QCOMPARE( l7.wkbType(), QgsWKBTypes::LineStringZ ); //should still be 3d
  QCOMPARE( l7.pointN( 1 ), QgsPointV2( QgsWKBTypes::PointZ, 11.0, 12.0, 0.0 ) );
  QVERIFY( l7.is3D() );
  QCOMPARE( l7.numPoints(), 2 );
  QCOMPARE( l7.vertexCount(), 2 );
  QCOMPARE( l7.nCoordinates(), 2 );
  QCOMPARE( l7.ringCount(), 1 );
  QCOMPARE( l7.partCount(), 1 );

  //clear
  l7.clear();
  QVERIFY( l7.isEmpty() );
  QCOMPARE( l7.numPoints(), 0 );
  QCOMPARE( l7.vertexCount(), 0 );
  QCOMPARE( l7.nCoordinates(), 0 );
  QCOMPARE( l7.ringCount(), 0 );
  QCOMPARE( l7.partCount(), 0 );
  QVERIFY( !l7.is3D() );
  QVERIFY( !l7.isMeasure() );
  QCOMPARE( l7.wkbType(), QgsWKBTypes::LineString );

  //setPoints
  QgsLineStringV2 l8;
  l8.setPoints( QgsPointSequenceV2() << QgsPointV2( 1, 2 ) << QgsPointV2( 2, 3 ) << QgsPointV2( 3, 4 ) );
  QVERIFY( !l8.isEmpty() );
  QCOMPARE( l8.numPoints(), 3 );
  QCOMPARE( l8.vertexCount(), 3 );
  QCOMPARE( l8.nCoordinates(), 3 );
  QCOMPARE( l8.ringCount(), 1 );
  QCOMPARE( l8.partCount(), 1 );
  QVERIFY( !l8.is3D() );
  QVERIFY( !l8.isMeasure() );
  QCOMPARE( l8.wkbType(), QgsWKBTypes::LineString );
  QVERIFY( !l8.hasCurvedSegments() );

  //setPoints with empty list, should clear linestring
  l8.setPoints( QgsPointSequenceV2() );
  QVERIFY( l8.isEmpty() );
  QCOMPARE( l8.numPoints(), 0 );
  QCOMPARE( l8.vertexCount(), 0 );
  QCOMPARE( l8.nCoordinates(), 0 );
  QCOMPARE( l8.ringCount(), 0 );
  QCOMPARE( l8.partCount(), 0 );
  QCOMPARE( l8.wkbType(), QgsWKBTypes::LineString );

  //setPoints with z
  l8.setPoints( QgsPointSequenceV2() << QgsPointV2( QgsWKBTypes::PointZ, 1, 2, 3 ) << QgsPointV2( QgsWKBTypes::PointZ, 2, 3, 4 ) );
  QCOMPARE( l8.numPoints(), 2 );
  QVERIFY( l8.is3D() );
  QVERIFY( !l8.isMeasure() );
  QCOMPARE( l8.wkbType(), QgsWKBTypes::LineStringZ );

  //setPoints with 25d
  l8.setPoints( QgsPointSequenceV2() << QgsPointV2( QgsWKBTypes::Point25D, 1, 2, 4 ) << QgsPointV2( QgsWKBTypes::Point25D, 2, 3, 4 ) );
  QCOMPARE( l8.numPoints(), 2 );
  QVERIFY( l8.is3D() );
  QVERIFY( !l8.isMeasure() );
  QCOMPARE( l8.wkbType(), QgsWKBTypes::LineString25D );
  QCOMPARE( l8.pointN( 0 ), QgsPointV2( QgsWKBTypes::Point25D, 1, 2, 4 ) );

  //setPoints with m
  l8.setPoints( QgsPointSequenceV2() << QgsPointV2( QgsWKBTypes::PointM, 1, 2, 0, 3 ) << QgsPointV2( QgsWKBTypes::PointM, 2, 3, 0, 4 ) );
  QCOMPARE( l8.numPoints(), 2 );
  QVERIFY( !l8.is3D() );
  QVERIFY( l8.isMeasure() );
  QCOMPARE( l8.wkbType(), QgsWKBTypes::LineStringM );

  //setPoints with zm
  l8.setPoints( QgsPointSequenceV2() << QgsPointV2( QgsWKBTypes::PointZM, 1, 2, 4, 5 ) << QgsPointV2( QgsWKBTypes::PointZM, 2, 3, 4, 5 ) );
  QCOMPARE( l8.numPoints(), 2 );
  QVERIFY( l8.is3D() );
  QVERIFY( l8.isMeasure() );
  QCOMPARE( l8.wkbType(), QgsWKBTypes::LineStringZM );

  //setPoints with MIXED dimensionality of points
  l8.setPoints( QgsPointSequenceV2() << QgsPointV2( QgsWKBTypes::PointZM, 1, 2, 4, 5 ) << QgsPointV2( QgsWKBTypes::PointM, 2, 3, 0, 5 ) );
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
  l9.setPoints( QgsPointSequenceV2() << QgsPointV2( QgsWKBTypes::PointZM, 1, 2, 3, 4 )
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
  l9.setPoints( QgsPointSequenceV2() << QgsPointV2( QgsWKBTypes::PointM, 1, 2, 0, 4 )
                << QgsPointV2( QgsWKBTypes::PointM, 11, 12, 0, 14 )
                << QgsPointV2( QgsWKBTypes::PointM, 21, 22, 0, 24 ) );

  //basically we just don't want these to crash
  QCOMPARE( l9.zAt( 0 ), 0.0 );
  QCOMPARE( l9.zAt( 1 ), 0.0 );
  l9.setZAt( 0, 53.0 );
  l9.setZAt( 1, 63.0 );

  //check mAt/setMAt with non-measure linestring
  l9.setPoints( QgsPointSequenceV2() << QgsPointV2( 1, 2 )
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
  toAppend->setPoints( QgsPointSequenceV2() << QgsPointV2( 1, 2 )
                       << QgsPointV2( 11, 12 )
                       << QgsPointV2( 21, 22 ) );
  l10.append( toAppend.data() );
  QVERIFY( !l10.is3D() );
  QVERIFY( !l10.isMeasure() );
  QCOMPARE( l10.numPoints(), 3 );
  QCOMPARE( l10.vertexCount(), 3 );
  QCOMPARE( l10.nCoordinates(), 3 );
  QCOMPARE( l10.ringCount(), 1 );
  QCOMPARE( l10.partCount(), 1 );
  QCOMPARE( l10.wkbType(), QgsWKBTypes::LineString );
  QCOMPARE( l10.pointN( 0 ), toAppend->pointN( 0 ) );
  QCOMPARE( l10.pointN( 1 ), toAppend->pointN( 1 ) );
  QCOMPARE( l10.pointN( 2 ), toAppend->pointN( 2 ) );

  //add more points
  toAppend.reset( new QgsLineStringV2() );
  toAppend->setPoints( QgsPointSequenceV2() << QgsPointV2( 31, 32 )
                       << QgsPointV2( 41, 42 )
                       << QgsPointV2( 51, 52 ) );
  l10.append( toAppend.data() );
  QCOMPARE( l10.numPoints(), 6 );
  QCOMPARE( l10.vertexCount(), 6 );
  QCOMPARE( l10.nCoordinates(), 6 );
  QCOMPARE( l10.ringCount(), 1 );
  QCOMPARE( l10.partCount(), 1 );
  QCOMPARE( l10.pointN( 3 ), toAppend->pointN( 0 ) );
  QCOMPARE( l10.pointN( 4 ), toAppend->pointN( 1 ) );
  QCOMPARE( l10.pointN( 5 ), toAppend->pointN( 2 ) );

  //check dimensionality is inherited from append line if initially empty
  l10.clear();
  toAppend.reset( new QgsLineStringV2() );
  toAppend->setPoints( QgsPointSequenceV2() << QgsPointV2( QgsWKBTypes::PointZM, 31, 32, 33, 34 )
                       << QgsPointV2( QgsWKBTypes::PointZM, 41, 42, 43 , 44 )
                       << QgsPointV2( QgsWKBTypes::PointZM, 51, 52, 53, 54 ) );
  l10.append( toAppend.data() );
  QVERIFY( l10.is3D() );
  QVERIFY( l10.isMeasure() );
  QCOMPARE( l10.numPoints(), 3 );
  QCOMPARE( l10.ringCount(), 1 );
  QCOMPARE( l10.partCount(), 1 );
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
  toAppend->setPoints( QgsPointSequenceV2() << QgsPointV2( QgsWKBTypes::PointZM, 31, 32, 33, 34 )
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
  toAppend->setPoints( QgsPointSequenceV2() << QgsPointV2( 31, 32 )
                       << QgsPointV2( 41, 42 )
                       << QgsPointV2( 51, 52 ) );
  l10.append( toAppend.data() );
  QCOMPARE( l10.wkbType(), QgsWKBTypes::LineStringZM );
  QCOMPARE( l10.pointN( 0 ), QgsPointV2( QgsWKBTypes::PointZM, 1, 2, 3, 4 ) );
  QCOMPARE( l10.pointN( 1 ), QgsPointV2( QgsWKBTypes::PointZM, 31, 32 ) );
  QCOMPARE( l10.pointN( 2 ), QgsPointV2( QgsWKBTypes::PointZM, 41, 42 ) );
  QCOMPARE( l10.pointN( 3 ), QgsPointV2( QgsWKBTypes::PointZM, 51, 52 ) );

  //25d append
  l10.clear();
  toAppend.reset( new QgsLineStringV2() );
  toAppend->setPoints( QgsPointSequenceV2() << QgsPointV2( QgsWKBTypes::Point25D, 31, 32, 33 )
                       << QgsPointV2( QgsWKBTypes::Point25D, 41, 42, 43 ) );
  l10.append( toAppend.data() );
  QVERIFY( l10.is3D() );
  QVERIFY( !l10.isMeasure() );
  QCOMPARE( l10.wkbType(), QgsWKBTypes::LineString25D );
  QCOMPARE( l10.pointN( 0 ), QgsPointV2( QgsWKBTypes::Point25D, 31, 32, 33 ) );
  QCOMPARE( l10.pointN( 1 ), QgsPointV2( QgsWKBTypes::Point25D, 41, 42, 43 ) );
  l10.clear();
  l10.setPoints( QgsPointSequenceV2() << QgsPointV2( QgsWKBTypes::Point25D, 11, 12, 33 ) );
  QCOMPARE( l10.wkbType(), QgsWKBTypes::LineString25D );
  l10.append( toAppend.data() );
  QVERIFY( l10.is3D() );
  QVERIFY( !l10.isMeasure() );
  QCOMPARE( l10.wkbType(), QgsWKBTypes::LineString25D );
  QCOMPARE( l10.pointN( 0 ), QgsPointV2( QgsWKBTypes::Point25D, 11, 12, 33 ) );
  QCOMPARE( l10.pointN( 1 ), QgsPointV2( QgsWKBTypes::Point25D, 31, 32, 33 ) );
  QCOMPARE( l10.pointN( 2 ), QgsPointV2( QgsWKBTypes::Point25D, 41, 42, 43 ) );

  //append another line the closes the original geometry.
  //Make sure there are not duplicit points except start and end point
  l10.clear();
  toAppend.reset( new QgsLineStringV2() );
  toAppend->setPoints( QgsPointSequenceV2()
                       << QgsPointV2( 1, 1 )
                       << QgsPointV2( 5, 5 )
                       << QgsPointV2( 10, 1 ) );
  l10.append( toAppend.data() );
  QCOMPARE( l10.numPoints(), 3 );
  QCOMPARE( l10.vertexCount(), 3 );
  toAppend.reset( new QgsLineStringV2() );
  toAppend->setPoints( QgsPointSequenceV2()
                       << QgsPointV2( 10, 1 )
                       << QgsPointV2( 1, 1 ) );
  l10.append( toAppend.data() );

  QVERIFY( l10.isClosed() );
  QCOMPARE( l10.numPoints(), 4 );
  QCOMPARE( l10.vertexCount(), 4 );

  //equality
  QgsLineStringV2 e1;
  QgsLineStringV2 e2;
  QVERIFY( e1 == e2 );
  QVERIFY( !( e1 != e2 ) );
  e1.addVertex( QgsPointV2( 1, 2 ) );
  QVERIFY( !( e1 == e2 ) ); //different number of vertices
  QVERIFY( e1 != e2 );
  e2.addVertex( QgsPointV2( 1, 2 ) );
  QVERIFY( e1 == e2 );
  QVERIFY( !( e1 != e2 ) );
  e1.addVertex( QgsPointV2( 1 / 3.0, 4 / 3.0 ) );
  e2.addVertex( QgsPointV2( 2 / 6.0, 8 / 6.0 ) );
  QVERIFY( e1 == e2 ); //check non-integer equality
  QVERIFY( !( e1 != e2 ) );
  e1.addVertex( QgsPointV2( 7, 8 ) );
  e2.addVertex( QgsPointV2( 6, 9 ) );
  QVERIFY( !( e1 == e2 ) ); //different coordinates
  QVERIFY( e1 != e2 );
  QgsLineStringV2 e3;
  e3.setPoints( QgsPointSequenceV2() << QgsPointV2( QgsWKBTypes::PointZ, 1, 2, 0 )
                << QgsPointV2( QgsWKBTypes::PointZ, 1 / 3.0, 4 / 3.0, 0 )
                << QgsPointV2( QgsWKBTypes::PointZ, 7, 8, 0 ) );
  QVERIFY( !( e1 == e3 ) ); //different dimension
  QVERIFY( e1 != e3 );
  QgsLineStringV2 e4;
  e4.setPoints( QgsPointSequenceV2() << QgsPointV2( QgsWKBTypes::PointZ, 1, 2, 2 )
                << QgsPointV2( QgsWKBTypes::PointZ, 1 / 3.0, 4 / 3.0, 3 )
                << QgsPointV2( QgsWKBTypes::PointZ, 7, 8, 4 ) );
  QVERIFY( !( e3 == e4 ) ); //different z coordinates
  QVERIFY( e3 != e4 );
  QgsLineStringV2 e5;
  e5.setPoints( QgsPointSequenceV2() << QgsPointV2( QgsWKBTypes::PointM, 1, 2, 0, 1 )
                << QgsPointV2( QgsWKBTypes::PointM, 1 / 3.0, 4 / 3.0, 0, 2 )
                << QgsPointV2( QgsWKBTypes::PointM, 7, 8, 0, 3 ) );
  QgsLineStringV2 e6;
  e6.setPoints( QgsPointSequenceV2() << QgsPointV2( QgsWKBTypes::PointM, 1, 2, 0, 11 )
                << QgsPointV2( QgsWKBTypes::PointM, 1 / 3.0, 4 / 3.0, 0, 12 )
                << QgsPointV2( QgsWKBTypes::PointM, 7, 8, 0, 13 ) );
  QVERIFY( !( e5 == e6 ) ); //different m values
  QVERIFY( e5 != e6 );

  //close/isClosed
  QgsLineStringV2 l11;
  QVERIFY( !l11.isClosed() );
  l11.setPoints( QgsPointSequenceV2() << QgsPointV2( 1, 2 )
                 << QgsPointV2( 11, 2 )
                 << QgsPointV2( 11, 22 )
                 << QgsPointV2( 1, 22 ) );
  QVERIFY( !l11.isClosed() );
  QCOMPARE( l11.numPoints(), 4 );
  QCOMPARE( l11.area(), 0.0 );
  QCOMPARE( l11.perimeter(), 0.0 );
  l11.close();
  QVERIFY( l11.isClosed() );
  QCOMPARE( l11.numPoints(), 5 );
  QCOMPARE( l11.vertexCount(), 5 );
  QCOMPARE( l11.nCoordinates(), 5 );
  QCOMPARE( l11.ringCount(), 1 );
  QCOMPARE( l11.partCount(), 1 );
  QCOMPARE( l11.pointN( 4 ), QgsPointV2( 1, 2 ) );
  QCOMPARE( l11.area(), 0.0 );
  QCOMPARE( l11.perimeter(), 0.0 );
  //try closing already closed line, should be no change
  l11.close();
  QVERIFY( l11.isClosed() );
  QCOMPARE( l11.numPoints(), 5 );
  QCOMPARE( l11.pointN( 4 ), QgsPointV2( 1, 2 ) );
  //test that m values aren't considered when testing for closedness
  l11.setPoints( QgsPointSequenceV2() << QgsPointV2( QgsWKBTypes::PointM, 1, 2, 0, 3 )
                 << QgsPointV2( QgsWKBTypes::PointM, 11, 2, 0, 4 )
                 << QgsPointV2( QgsWKBTypes::PointM, 11, 22, 0, 5 )
                 << QgsPointV2( QgsWKBTypes::PointM, 1, 2, 0, 6 ) );
  QVERIFY( l11.isClosed() );

  //close with z and m
  QgsLineStringV2 l12;
  l12.setPoints( QgsPointSequenceV2() << QgsPointV2( QgsWKBTypes::PointZM, 1, 2, 3, 4 )
                 << QgsPointV2( QgsWKBTypes::PointZM, 11, 2, 11, 14 )
                 << QgsPointV2( QgsWKBTypes::PointZM, 11, 22, 21, 24 )
                 << QgsPointV2( QgsWKBTypes::PointZM, 1, 22, 31, 34 ) );
  l12.close();
  QCOMPARE( l12.pointN( 4 ), QgsPointV2( QgsWKBTypes::PointZM, 1, 2, 3, 4 ) );


  //polygonf
  QgsLineStringV2 l13;
  l13.setPoints( QgsPointSequenceV2() << QgsPointV2( QgsWKBTypes::PointZM, 1, 2, 3, 4 )
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

  // clone tests. At the same time, check segmentize as the result should
  // be equal to a clone for LineStrings
  QgsLineStringV2 l14;
  l14.setPoints( QgsPointSequenceV2() << QgsPointV2( 1, 2 )
                 << QgsPointV2( 11, 2 )
                 << QgsPointV2( 11, 22 )
                 << QgsPointV2( 1, 22 ) );
  QScopedPointer<QgsLineStringV2> cloned( l14.clone() );
  QCOMPARE( cloned->numPoints(), 4 );
  QCOMPARE( cloned->vertexCount(), 4 );
  QCOMPARE( cloned->ringCount(), 1 );
  QCOMPARE( cloned->partCount(), 1 );
  QCOMPARE( cloned->wkbType(), QgsWKBTypes::LineString );
  QVERIFY( !cloned->is3D() );
  QVERIFY( !cloned->isMeasure() );
  QCOMPARE( cloned->pointN( 0 ), l14.pointN( 0 ) );
  QCOMPARE( cloned->pointN( 1 ), l14.pointN( 1 ) );
  QCOMPARE( cloned->pointN( 2 ), l14.pointN( 2 ) );
  QCOMPARE( cloned->pointN( 3 ), l14.pointN( 3 ) );
  QScopedPointer< QgsLineStringV2 > segmentized( static_cast< QgsLineStringV2* >( l14.segmentize() ) );
  QCOMPARE( segmentized->numPoints(), 4 );
  QCOMPARE( segmentized->wkbType(), QgsWKBTypes::LineString );
  QVERIFY( !segmentized->is3D() );
  QVERIFY( !segmentized->isMeasure() );
  QCOMPARE( segmentized->pointN( 0 ), l14.pointN( 0 ) );
  QCOMPARE( segmentized->pointN( 1 ), l14.pointN( 1 ) );
  QCOMPARE( segmentized->pointN( 2 ), l14.pointN( 2 ) );
  QCOMPARE( segmentized->pointN( 3 ), l14.pointN( 3 ) );

  //clone with Z/M
  l14.setPoints( QgsPointSequenceV2() << QgsPointV2( QgsWKBTypes::PointZM, 1, 2, 3, 4 )
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
  segmentized.reset( static_cast< QgsLineStringV2* >( l14.segmentize() ) );
  QCOMPARE( segmentized->numPoints(), 4 );
  QCOMPARE( segmentized->wkbType(), QgsWKBTypes::LineStringZM );
  QVERIFY( segmentized->is3D() );
  QVERIFY( segmentized->isMeasure() );
  QCOMPARE( segmentized->pointN( 0 ), l14.pointN( 0 ) );
  QCOMPARE( segmentized->pointN( 1 ), l14.pointN( 1 ) );
  QCOMPARE( segmentized->pointN( 2 ), l14.pointN( 2 ) );
  QCOMPARE( segmentized->pointN( 3 ), l14.pointN( 3 ) );

  //clone an empty line
  l14.clear();
  cloned.reset( l14.clone() );
  QVERIFY( cloned->isEmpty() );
  QCOMPARE( cloned->numPoints(), 0 );
  QVERIFY( !cloned->is3D() );
  QVERIFY( !cloned->isMeasure() );
  QCOMPARE( cloned->wkbType(), QgsWKBTypes::LineString );
  segmentized.reset( static_cast< QgsLineStringV2* >( l14.segmentize() ) );
  QVERIFY( segmentized->isEmpty() );
  QCOMPARE( segmentized->numPoints(), 0 );
  QVERIFY( !segmentized->is3D() );
  QVERIFY( !segmentized->isMeasure() );
  QCOMPARE( segmentized->wkbType(), QgsWKBTypes::LineString );

  //to/from WKB
  QgsLineStringV2 l15;
  l15.setPoints( QgsPointSequenceV2() << QgsPointV2( QgsWKBTypes::PointZM, 1, 2, 3, 4 )
                 << QgsPointV2( QgsWKBTypes::PointZM, 11, 2, 11, 14 )
                 << QgsPointV2( QgsWKBTypes::PointZM, 11, 22, 21, 24 )
                 << QgsPointV2( QgsWKBTypes::PointZM, 1, 22, 31, 34 ) );
  int size = 0;
  unsigned char* wkb = l15.asWkb( size );
  QCOMPARE( size, l15.wkbSize() );
  QgsLineStringV2 l16;
  l16.fromWkb( QgsConstWkbPtr( wkb, size ) );
  delete[] wkb;
  wkb = 0;
  QCOMPARE( l16.numPoints(), 4 );
  QCOMPARE( l16.vertexCount(), 4 );
  QCOMPARE( l16.nCoordinates(), 4 );
  QCOMPARE( l16.ringCount(), 1 );
  QCOMPARE( l16.partCount(), 1 );
  QCOMPARE( l16.wkbType(), QgsWKBTypes::LineStringZM );
  QVERIFY( l16.is3D() );
  QVERIFY( l16.isMeasure() );
  QCOMPARE( l16.pointN( 0 ), l15.pointN( 0 ) );
  QCOMPARE( l16.pointN( 1 ), l15.pointN( 1 ) );
  QCOMPARE( l16.pointN( 2 ), l15.pointN( 2 ) );
  QCOMPARE( l16.pointN( 3 ), l15.pointN( 3 ) );

  //bad WKB - check for no crash
  l16.clear();
  QVERIFY( !l16.fromWkb( QgsConstWkbPtr( nullptr, 0 ) ) );
  QCOMPARE( l16.wkbType(), QgsWKBTypes::LineString );
  QgsPointV2 point( 1, 2 );
  wkb = point.asWkb( size ) ;
  QVERIFY( !l16.fromWkb( QgsConstWkbPtr( wkb, size ) ) );
  delete[] wkb;
  wkb = 0;
  QCOMPARE( l16.wkbType(), QgsWKBTypes::LineString );

  //to/from WKT
  QgsLineStringV2 l17;
  l17.setPoints( QgsPointSequenceV2() << QgsPointV2( QgsWKBTypes::PointZM, 1, 2, 3, 4 )
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
  QCOMPARE( l18.wkbType(), QgsWKBTypes::LineString );

  //asGML2
  QgsLineStringV2 exportLine;
  exportLine.setPoints( QgsPointSequenceV2() << QgsPointV2( 31, 32 )
                        << QgsPointV2( 41, 42 )
                        << QgsPointV2( 51, 52 ) );
  QgsLineStringV2 exportLineFloat;
  exportLineFloat.setPoints( QgsPointSequenceV2() << QgsPointV2( 1 / 3.0, 2 / 3.0 )
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
  l19.setPoints( QgsPointSequenceV2() << QgsPointV2( QgsWKBTypes::PointZM, 1, 1, 2, 3 )
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
  l19.setPoints( QgsPointSequenceV2() << QgsPointV2( QgsWKBTypes::PointZM, 1, 1, 2, 3 )
                 << QgsPointV2( QgsWKBTypes::PointZM, 1, 10, 4, 5 )
                 << QgsPointV2( QgsWKBTypes::PointZM, 15, 10, 6, 7 ) );
  segmentized.reset( l19.curveToLine() );
  QCOMPARE( segmentized->numPoints(), 3 );
  QCOMPARE( segmentized->wkbType(), QgsWKBTypes::LineStringZM );
  QVERIFY( segmentized->is3D() );
  QVERIFY( segmentized->isMeasure() );
  QCOMPARE( segmentized->pointN( 0 ), l19.pointN( 0 ) );
  QCOMPARE( segmentized->pointN( 1 ), l19.pointN( 1 ) );
  QCOMPARE( segmentized->pointN( 2 ), l19.pointN( 2 ) );

  // points
  QgsLineStringV2 l20;
  QgsPointSequenceV2 points;
  l20.points( points );
  QVERIFY( l20.isEmpty() );
  l20.setPoints( QgsPointSequenceV2() << QgsPointV2( QgsWKBTypes::PointZM, 1, 1, 2, 3 )
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
  destSrs.createFromSrid( 4202 ); // want a transform with ellipsoid change
  QgsCoordinateTransform tr( sourceSrs, destSrs );

  // 2d CRS transform
  QgsLineStringV2 l21;
  l21.setPoints( QgsPointSequenceV2() << QgsPointV2( 6374985, -3626584 )
                 << QgsPointV2( 6474985, -3526584 ) );
  l21.transform( tr, QgsCoordinateTransform::ForwardTransform );
  QGSCOMPARENEAR( l21.pointN( 0 ).x(), 175.771, 0.001 );
  QGSCOMPARENEAR( l21.pointN( 0 ).y(), -39.724, 0.001 );
  QGSCOMPARENEAR( l21.pointN( 1 ).x(), 176.959, 0.001 );
  QGSCOMPARENEAR( l21.pointN( 1 ).y(), -38.7999, 0.001 );
  QGSCOMPARENEAR( l21.boundingBox().xMinimum(), 175.771, 0.001 );
  QGSCOMPARENEAR( l21.boundingBox().yMinimum(), -39.724, 0.001 );
  QGSCOMPARENEAR( l21.boundingBox().xMaximum(), 176.959, 0.001 );
  QGSCOMPARENEAR( l21.boundingBox().yMaximum(), -38.7999, 0.001 );

  //3d CRS transform
  QgsLineStringV2 l22;
  l22.setPoints( QgsPointSequenceV2() << QgsPointV2( QgsWKBTypes::PointZM, 6374985, -3626584, 1, 2 )
                 << QgsPointV2( QgsWKBTypes::PointZM, 6474985, -3526584, 3, 4 ) );
  l22.transform( tr, QgsCoordinateTransform::ForwardTransform );
  QGSCOMPARENEAR( l22.pointN( 0 ).x(), 175.771, 0.001 );
  QGSCOMPARENEAR( l22.pointN( 0 ).y(), -39.724, 0.001 );
  QGSCOMPARENEAR( l22.pointN( 0 ).z(), 1.0, 0.001 );
  QCOMPARE( l22.pointN( 0 ).m(), 2.0 );
  QGSCOMPARENEAR( l22.pointN( 1 ).x(), 176.959, 0.001 );
  QGSCOMPARENEAR( l22.pointN( 1 ).y(), -38.7999, 0.001 );
  QGSCOMPARENEAR( l22.pointN( 1 ).z(), 3.0, 0.001 );
  QCOMPARE( l22.pointN( 1 ).m(), 4.0 );

  //reverse transform
  l22.transform( tr, QgsCoordinateTransform::ReverseTransform );
  QGSCOMPARENEAR( l22.pointN( 0 ).x(), 6374985, 0.01 );
  QGSCOMPARENEAR( l22.pointN( 0 ).y(), -3626584, 0.01 );
  QGSCOMPARENEAR( l22.pointN( 0 ).z(), 1, 0.001 );
  QCOMPARE( l22.pointN( 0 ).m(), 2.0 );
  QGSCOMPARENEAR( l22.pointN( 1 ).x(), 6474985, 0.01 );
  QGSCOMPARENEAR( l22.pointN( 1 ).y(), -3526584, 0.01 );
  QGSCOMPARENEAR( l22.pointN( 1 ).z(), 3, 0.001 );
  QCOMPARE( l22.pointN( 1 ).m(), 4.0 );

  //z value transform
  l22.transform( tr, QgsCoordinateTransform::ForwardTransform, true );
  QGSCOMPARENEAR( l22.pointN( 0 ).z(), -19.249066, 0.001 );
  QGSCOMPARENEAR( l22.pointN( 1 ).z(), -21.092128, 0.001 );
  l22.transform( tr, QgsCoordinateTransform::ReverseTransform, true );
  QGSCOMPARENEAR( l22.pointN( 0 ).z(), 1.0, 0.001 );
  QGSCOMPARENEAR( l22.pointN( 1 ).z(), 3.0, 0.001 );

  //QTransform transform
  QTransform qtr = QTransform::fromScale( 2, 3 );
  QgsLineStringV2 l23;
  l23.setPoints( QgsPointSequenceV2() << QgsPointV2( QgsWKBTypes::PointZM, 1, 2, 3, 4 )
                 << QgsPointV2( QgsWKBTypes::PointZM, 11, 12, 13, 14 ) );
  l23.transform( qtr );
  QCOMPARE( l23.pointN( 0 ), QgsPointV2( QgsWKBTypes::PointZM, 2, 6, 3, 4 ) );
  QCOMPARE( l23.pointN( 1 ), QgsPointV2( QgsWKBTypes::PointZM, 22, 36, 13, 14 ) );
  QCOMPARE( l23.boundingBox(), QgsRectangle( 2, 6, 22, 36 ) );

  //insert vertex

  //insert vertex in empty line
  QgsLineStringV2 l24;
  QVERIFY( l24.insertVertex( QgsVertexId( 0, 0, 0 ), QgsPointV2( 6.0, 7.0 ) ) );
  QCOMPARE( l24.numPoints(), 1 );
  QVERIFY( !l24.is3D() );
  QVERIFY( !l24.isMeasure() );
  QCOMPARE( l24.wkbType(), QgsWKBTypes::LineString );
  QCOMPARE( l24.pointN( 0 ), QgsPointV2( 6.0, 7.0 ) );

  //insert 4d vertex in empty line, should set line to 4d
  l24.clear();
  QVERIFY( l24.insertVertex( QgsVertexId( 0, 0, 0 ), QgsPointV2( QgsWKBTypes::PointZM, 6.0, 7.0, 1.0, 2.0 ) ) );
  QCOMPARE( l24.numPoints(), 1 );
  QVERIFY( l24.is3D() );
  QVERIFY( l24.isMeasure() );
  QCOMPARE( l24.wkbType(), QgsWKBTypes::LineStringZM );
  QCOMPARE( l24.pointN( 0 ), QgsPointV2( QgsWKBTypes::PointZM, 6.0, 7.0, 1.0, 2.0 ) );

  //2d line
  l24.setPoints( QgsPointSequenceV2() << QgsPointV2( 1, 2 )
                 << QgsPointV2( 11, 12 ) << QgsPointV2( 21, 22 ) );
  QVERIFY( l24.insertVertex( QgsVertexId( 0, 0, 0 ), QgsPointV2( 6.0, 7.0 ) ) );
  QCOMPARE( l24.numPoints(), 4 );
  QVERIFY( !l24.is3D() );
  QVERIFY( !l24.isMeasure() );
  QCOMPARE( l24.wkbType(), QgsWKBTypes::LineString );
  QVERIFY( l24.insertVertex( QgsVertexId( 0, 0, 1 ), QgsPointV2( 8.0, 9.0 ) ) );
  QVERIFY( l24.insertVertex( QgsVertexId( 0, 0, 2 ), QgsPointV2( 18.0, 19.0 ) ) );
  QCOMPARE( l24.pointN( 0 ), QgsPointV2( 6.0, 7.0 ) );
  QCOMPARE( l24.pointN( 1 ), QgsPointV2( 8.0, 9.0 ) );
  QCOMPARE( l24.pointN( 2 ), QgsPointV2( 18.0, 19.0 ) );
  QCOMPARE( l24.pointN( 3 ), QgsPointV2( 1.0, 2.0 ) );
  QCOMPARE( l24.pointN( 4 ), QgsPointV2( 11.0, 12.0 ) );
  QCOMPARE( l24.pointN( 5 ), QgsPointV2( 21.0, 22.0 ) );
  //insert vertex at end
  QVERIFY( l24.insertVertex( QgsVertexId( 0, 0, 6 ), QgsPointV2( 31.0, 32.0 ) ) );
  QCOMPARE( l24.pointN( 6 ), QgsPointV2( 31.0, 32.0 ) );
  QCOMPARE( l24.numPoints(), 7 );

  //insert vertex past end
  QVERIFY( !l24.insertVertex( QgsVertexId( 0, 0, 8 ), QgsPointV2( 41.0, 42.0 ) ) );
  QCOMPARE( l24.numPoints(), 7 );

  //insert vertex before start
  QVERIFY( !l24.insertVertex( QgsVertexId( 0, 0, -18 ), QgsPointV2( 41.0, 42.0 ) ) );
  QCOMPARE( l24.numPoints(), 7 );

  //insert 4d vertex in 4d line
  l24.setPoints( QgsPointSequenceV2() << QgsPointV2( QgsWKBTypes::PointZM, 1, 1, 2, 3 )
                 << QgsPointV2( QgsWKBTypes::PointZM, 1, 10, 4, 5 )
                 << QgsPointV2( QgsWKBTypes::PointZM, 15, 10, 6, 7 ) );
  QVERIFY( l24.insertVertex( QgsVertexId( 0, 0, 0 ), QgsPointV2( QgsWKBTypes::PointZM, 11, 12, 13, 14 ) ) );
  QCOMPARE( l24.numPoints(), 4 );
  QCOMPARE( l24.pointN( 0 ), QgsPointV2( QgsWKBTypes::PointZM, 11, 12, 13, 14 ) );

  //insert 2d vertex in 4d line
  QVERIFY( l24.insertVertex( QgsVertexId( 0, 0, 1 ), QgsPointV2( 101, 102 ) ) );
  QCOMPARE( l24.numPoints(), 5 );
  QCOMPARE( l24.wkbType(), QgsWKBTypes::LineStringZM );
  QCOMPARE( l24.pointN( 1 ), QgsPointV2( QgsWKBTypes::PointZM, 101, 102, 0, 0 ) );

  //insert 4d vertex in 2d line
  l24.setPoints( QgsPointSequenceV2() << QgsPointV2( 1, 2 )
                 << QgsPointV2( 11, 12 ) << QgsPointV2( 21, 22 ) );
  QVERIFY( l24.insertVertex( QgsVertexId( 0, 0, 0 ), QgsPointV2( QgsWKBTypes::PointZM, 101, 102, 103, 104 ) ) );
  QCOMPARE( l24.numPoints(), 4 );
  QCOMPARE( l24.wkbType(), QgsWKBTypes::LineString );
  QCOMPARE( l24.pointN( 0 ), QgsPointV2( QgsWKBTypes::Point, 101, 102 ) );

  //insert first vertex as Point25D
  l24.clear();
  QVERIFY( l24.insertVertex( QgsVertexId( 0, 0, 0 ), QgsPointV2( QgsWKBTypes::Point25D, 101, 102, 103 ) ) );
  QCOMPARE( l24.wkbType(), QgsWKBTypes::LineString25D );
  QCOMPARE( l24.pointN( 0 ), QgsPointV2( QgsWKBTypes::Point25D, 101, 102, 103 ) );

  //move vertex

  //empty line
  QgsLineStringV2 l25;
  QVERIFY( !l25.moveVertex( QgsVertexId( 0, 0, 0 ), QgsPointV2( 6.0, 7.0 ) ) );
  QVERIFY( l25.isEmpty() );

  //valid line
  l25.setPoints( QgsPointSequenceV2() << QgsPointV2( 1, 2 )
                 << QgsPointV2( 11, 12 ) << QgsPointV2( 21, 22 ) );
  QVERIFY( l25.moveVertex( QgsVertexId( 0, 0, 0 ), QgsPointV2( 6.0, 7.0 ) ) );
  QVERIFY( l25.moveVertex( QgsVertexId( 0, 0, 1 ), QgsPointV2( 16.0, 17.0 ) ) );
  QVERIFY( l25.moveVertex( QgsVertexId( 0, 0, 2 ), QgsPointV2( 26.0, 27.0 ) ) );
  QCOMPARE( l25.pointN( 0 ), QgsPointV2( 6.0, 7.0 ) );
  QCOMPARE( l25.pointN( 1 ), QgsPointV2( 16.0, 17.0 ) );
  QCOMPARE( l25.pointN( 2 ), QgsPointV2( 26.0, 27.0 ) );

  //out of range
  QVERIFY( !l25.moveVertex( QgsVertexId( 0, 0, -1 ), QgsPointV2( 3.0, 4.0 ) ) );
  QVERIFY( !l25.moveVertex( QgsVertexId( 0, 0, 10 ), QgsPointV2( 3.0, 4.0 ) ) );
  QCOMPARE( l25.pointN( 0 ), QgsPointV2( 6.0, 7.0 ) );
  QCOMPARE( l25.pointN( 1 ), QgsPointV2( 16.0, 17.0 ) );
  QCOMPARE( l25.pointN( 2 ), QgsPointV2( 26.0, 27.0 ) );

  //move 4d point in 4d line
  l25.setPoints( QgsPointSequenceV2() << QgsPointV2( QgsWKBTypes::PointZM, 1, 1, 2, 3 )
                 << QgsPointV2( QgsWKBTypes::PointZM, 1, 10, 4, 5 )
                 << QgsPointV2( QgsWKBTypes::PointZM, 15, 10, 6, 7 ) );
  QVERIFY( l25.moveVertex( QgsVertexId( 0, 0, 1 ), QgsPointV2( QgsWKBTypes::PointZM, 6, 7, 12, 13 ) ) );
  QCOMPARE( l25.pointN( 1 ), QgsPointV2( QgsWKBTypes::PointZM, 6, 7, 12, 13 ) );

  //move 2d point in 4d line, existing z/m should be maintained
  QVERIFY( l25.moveVertex( QgsVertexId( 0, 0, 1 ), QgsPointV2( 34, 35 ) ) );
  QCOMPARE( l25.pointN( 1 ), QgsPointV2( QgsWKBTypes::PointZM, 34, 35, 12, 13 ) );

  //move 4d point in 2d line
  l25.setPoints( QgsPointSequenceV2() << QgsPointV2( 1, 2 )
                 << QgsPointV2( 11, 12 ) << QgsPointV2( 21, 22 ) );
  QVERIFY( l25.moveVertex( QgsVertexId( 0, 0, 0 ), QgsPointV2( QgsWKBTypes::PointZM, 3, 4, 2, 3 ) ) );
  QCOMPARE( l25.pointN( 0 ), QgsPointV2( 3, 4 ) );


  //delete vertex

  //empty line
  QgsLineStringV2 l26;
  QVERIFY( !l26.deleteVertex( QgsVertexId( 0, 0, 0 ) ) );
  QVERIFY( l26.isEmpty() );

  //valid line
  l26.setPoints( QgsPointSequenceV2() << QgsPointV2( QgsWKBTypes::PointZM, 1, 2, 2, 3 )
                 << QgsPointV2( QgsWKBTypes::PointZM, 11, 12, 4, 5 )
                 << QgsPointV2( QgsWKBTypes::PointZM, 21, 22, 6, 7 ) );
  //out of range vertices
  QVERIFY( !l26.deleteVertex( QgsVertexId( 0, 0, -1 ) ) );
  QVERIFY( !l26.deleteVertex( QgsVertexId( 0, 0, 100 ) ) );

  //valid vertices
  QVERIFY( l26.deleteVertex( QgsVertexId( 0, 0, 1 ) ) );
  QCOMPARE( l26.numPoints(), 2 );
  QCOMPARE( l26.pointN( 0 ), QgsPointV2( QgsWKBTypes::PointZM, 1, 2, 2, 3 ) );
  QCOMPARE( l26.pointN( 1 ), QgsPointV2( QgsWKBTypes::PointZM, 21, 22, 6, 7 ) );
  //removing the second to last vertex removes both remaining vertices
  QVERIFY( l26.deleteVertex( QgsVertexId( 0, 0, 0 ) ) );
  QCOMPARE( l26.numPoints(), 0 );
  QVERIFY( !l26.deleteVertex( QgsVertexId( 0, 0, 0 ) ) );
  QVERIFY( l26.isEmpty() );

  //reversed
  QgsLineStringV2 l27;
  QScopedPointer< QgsLineStringV2 > reversed( l27.reversed() );
  QVERIFY( reversed->isEmpty() );
  l27.setPoints( QgsPointSequenceV2() << QgsPointV2( QgsWKBTypes::PointZM, 1, 2, 2, 3 )
                 << QgsPointV2( QgsWKBTypes::PointZM, 11, 12, 4, 5 )
                 << QgsPointV2( QgsWKBTypes::PointZM, 21, 22, 6, 7 ) );
  reversed.reset( l27.reversed() );
  QCOMPARE( reversed->numPoints(), 3 );
  QCOMPARE( reversed->wkbType(), QgsWKBTypes::LineStringZM );
  QVERIFY( reversed->is3D() );
  QVERIFY( reversed->isMeasure() );
  QCOMPARE( reversed->pointN( 0 ), QgsPointV2( QgsWKBTypes::PointZM, 21, 22, 6, 7 ) );
  QCOMPARE( reversed->pointN( 1 ), QgsPointV2( QgsWKBTypes::PointZM, 11, 12, 4, 5 ) );
  QCOMPARE( reversed->pointN( 2 ), QgsPointV2( QgsWKBTypes::PointZM, 1, 2, 2, 3 ) );

  //addZValue

  QgsLineStringV2 l28;
  QCOMPARE( l28.wkbType(), QgsWKBTypes::LineString );
  QVERIFY( l28.addZValue() );
  QCOMPARE( l28.wkbType(), QgsWKBTypes::LineStringZ );
  l28.clear();
  QVERIFY( l28.addZValue() );
  QCOMPARE( l28.wkbType(), QgsWKBTypes::LineStringZ );
  //2d line
  l28.setPoints( QgsPointSequenceV2() << QgsPointV2( 1, 2 ) << QgsPointV2( 11, 12 ) );
  QVERIFY( l28.addZValue( 2 ) );
  QVERIFY( l28.is3D() );
  QCOMPARE( l28.wkbType(), QgsWKBTypes::LineStringZ );
  QCOMPARE( l28.pointN( 0 ), QgsPointV2( QgsWKBTypes::PointZ, 1, 2, 2 ) );
  QCOMPARE( l28.pointN( 1 ), QgsPointV2( QgsWKBTypes::PointZ, 11, 12, 2 ) );
  QVERIFY( !l28.addZValue( 4 ) ); //already has z value, test that existing z is unchanged
  QCOMPARE( l28.pointN( 0 ), QgsPointV2( QgsWKBTypes::PointZ, 1, 2, 2 ) );
  QCOMPARE( l28.pointN( 1 ), QgsPointV2( QgsWKBTypes::PointZ, 11, 12, 2 ) );
  //linestring with m
  l28.setPoints( QgsPointSequenceV2() << QgsPointV2( QgsWKBTypes::PointM, 1, 2, 0, 3 ) << QgsPointV2( QgsWKBTypes::PointM, 11, 12, 0, 4 ) );
  QVERIFY( l28.addZValue( 5 ) );
  QVERIFY( l28.is3D() );
  QVERIFY( l28.isMeasure() );
  QCOMPARE( l28.wkbType(), QgsWKBTypes::LineStringZM );
  QCOMPARE( l28.pointN( 0 ), QgsPointV2( QgsWKBTypes::PointZM, 1, 2, 5, 3 ) );
  QCOMPARE( l28.pointN( 1 ), QgsPointV2( QgsWKBTypes::PointZM, 11, 12, 5, 4 ) );
  //linestring25d
  l28.setPoints( QgsPointSequenceV2() << QgsPointV2( QgsWKBTypes::Point25D, 1, 2, 3 ) << QgsPointV2( QgsWKBTypes::Point25D, 11, 12, 4 ) );
  QCOMPARE( l28.wkbType(), QgsWKBTypes::LineString25D );
  QVERIFY( !l28.addZValue( 5 ) );
  QCOMPARE( l28.wkbType(), QgsWKBTypes::LineString25D );
  QCOMPARE( l28.pointN( 0 ), QgsPointV2( QgsWKBTypes::Point25D, 1, 2, 3 ) );
  QCOMPARE( l28.pointN( 1 ), QgsPointV2( QgsWKBTypes::Point25D, 11, 12, 4 ) );

  //addMValue

  QgsLineStringV2 l29;
  QCOMPARE( l29.wkbType(), QgsWKBTypes::LineString );
  QVERIFY( l29.addMValue() );
  QCOMPARE( l29.wkbType(), QgsWKBTypes::LineStringM );
  l29.clear();
  QVERIFY( l29.addMValue() );
  QCOMPARE( l29.wkbType(), QgsWKBTypes::LineStringM );
  //2d line
  l29.setPoints( QgsPointSequenceV2() << QgsPointV2( 1, 2 ) << QgsPointV2( 11, 12 ) );
  QVERIFY( l29.addMValue( 2 ) );
  QVERIFY( !l29.is3D() );
  QVERIFY( l29.isMeasure() );
  QCOMPARE( l29.wkbType(), QgsWKBTypes::LineStringM );
  QCOMPARE( l29.pointN( 0 ), QgsPointV2( QgsWKBTypes::PointM, 1, 2, 0, 2 ) );
  QCOMPARE( l29.pointN( 1 ), QgsPointV2( QgsWKBTypes::PointM, 11, 12, 0, 2 ) );
  QVERIFY( !l29.addMValue( 4 ) ); //already has m value, test that existing m is unchanged
  QCOMPARE( l29.pointN( 0 ), QgsPointV2( QgsWKBTypes::PointM, 1, 2, 0, 2 ) );
  QCOMPARE( l29.pointN( 1 ), QgsPointV2( QgsWKBTypes::PointM, 11, 12, 0, 2 ) );
  //linestring with z
  l29.setPoints( QgsPointSequenceV2() << QgsPointV2( QgsWKBTypes::PointZ, 1, 2, 3 ) << QgsPointV2( QgsWKBTypes::PointZ, 11, 12, 4 ) );
  QVERIFY( l29.addMValue( 5 ) );
  QVERIFY( l29.is3D() );
  QVERIFY( l29.isMeasure() );
  QCOMPARE( l29.wkbType(), QgsWKBTypes::LineStringZM );
  QCOMPARE( l29.pointN( 0 ), QgsPointV2( QgsWKBTypes::PointZM, 1, 2, 3, 5 ) );
  QCOMPARE( l29.pointN( 1 ), QgsPointV2( QgsWKBTypes::PointZM, 11, 12, 4, 5 ) );
  //linestring25d, should become LineStringZM
  l29.setPoints( QgsPointSequenceV2() << QgsPointV2( QgsWKBTypes::Point25D, 1, 2, 3 ) << QgsPointV2( QgsWKBTypes::Point25D, 11, 12, 4 ) );
  QCOMPARE( l29.wkbType(), QgsWKBTypes::LineString25D );
  QVERIFY( l29.addMValue( 5 ) );
  QVERIFY( l29.is3D() );
  QVERIFY( l29.isMeasure() );
  QCOMPARE( l29.wkbType(), QgsWKBTypes::LineStringZM );
  QCOMPARE( l29.pointN( 0 ), QgsPointV2( QgsWKBTypes::PointZM, 1, 2, 3, 5 ) );
  QCOMPARE( l29.pointN( 1 ), QgsPointV2( QgsWKBTypes::PointZM, 11, 12, 4, 5 ) );


  //dropZValue
  QgsLineStringV2 l28d;
  QVERIFY( !l28d.dropZValue() );
  l28d.setPoints( QgsPointSequenceV2() << QgsPointV2( 1, 2 ) << QgsPointV2( 11, 12 ) );
  QVERIFY( !l28d.dropZValue() );
  l28d.addZValue( 1.0 );
  QCOMPARE( l28d.wkbType(), QgsWKBTypes::LineStringZ );
  QVERIFY( l28d.is3D() );
  QVERIFY( l28d.dropZValue() );
  QVERIFY( !l28d.is3D() );
  QCOMPARE( l28d.wkbType(), QgsWKBTypes::LineString );
  QCOMPARE( l28d.pointN( 0 ), QgsPointV2( QgsWKBTypes::Point, 1, 2 ) );
  QCOMPARE( l28d.pointN( 1 ), QgsPointV2( QgsWKBTypes::Point, 11, 12 ) );
  QVERIFY( !l28d.dropZValue() ); //already dropped
  //linestring with m
  l28d.setPoints( QgsPointSequenceV2() << QgsPointV2( QgsWKBTypes::PointZM, 1, 2, 3, 4 ) << QgsPointV2( QgsWKBTypes::PointZM, 11, 12, 3, 4 ) );
  QVERIFY( l28d.dropZValue() );
  QVERIFY( !l28d.is3D() );
  QVERIFY( l28d.isMeasure() );
  QCOMPARE( l28d.wkbType(), QgsWKBTypes::LineStringM );
  QCOMPARE( l28d.pointN( 0 ), QgsPointV2( QgsWKBTypes::PointM, 1, 2, 0, 4 ) );
  QCOMPARE( l28d.pointN( 1 ), QgsPointV2( QgsWKBTypes::PointM, 11, 12, 0, 4 ) );
  //linestring25d
  l28d.setPoints( QgsPointSequenceV2() << QgsPointV2( QgsWKBTypes::Point25D, 1, 2, 3 ) << QgsPointV2( QgsWKBTypes::Point25D, 11, 12, 4 ) );
  QCOMPARE( l28d.wkbType(), QgsWKBTypes::LineString25D );
  QVERIFY( l28d.dropZValue() );
  QCOMPARE( l28d.wkbType(), QgsWKBTypes::LineString );
  QCOMPARE( l28d.pointN( 0 ), QgsPointV2( QgsWKBTypes::Point, 1, 2 ) );
  QCOMPARE( l28d.pointN( 1 ), QgsPointV2( QgsWKBTypes::Point, 11, 12 ) );

  //dropMValue
  l28d.setPoints( QgsPointSequenceV2() << QgsPointV2( 1, 2 ) << QgsPointV2( 11, 12 ) );
  QVERIFY( !l28d.dropMValue() );
  l28d.addMValue( 1.0 );
  QCOMPARE( l28d.wkbType(), QgsWKBTypes::LineStringM );
  QVERIFY( l28d.isMeasure() );
  QVERIFY( l28d.dropMValue() );
  QVERIFY( !l28d.isMeasure() );
  QCOMPARE( l28d.wkbType(), QgsWKBTypes::LineString );
  QCOMPARE( l28d.pointN( 0 ), QgsPointV2( QgsWKBTypes::Point, 1, 2 ) );
  QCOMPARE( l28d.pointN( 1 ), QgsPointV2( QgsWKBTypes::Point, 11, 12 ) );
  QVERIFY( !l28d.dropMValue() ); //already dropped
  //linestring with z
  l28d.setPoints( QgsPointSequenceV2() << QgsPointV2( QgsWKBTypes::PointZM, 1, 2, 3, 4 ) << QgsPointV2( QgsWKBTypes::PointZM, 11, 12, 3, 4 ) );
  QVERIFY( l28d.dropMValue() );
  QVERIFY( !l28d.isMeasure() );
  QVERIFY( l28d.is3D() );
  QCOMPARE( l28d.wkbType(), QgsWKBTypes::LineStringZ );
  QCOMPARE( l28d.pointN( 0 ), QgsPointV2( QgsWKBTypes::PointZ, 1, 2, 3, 0 ) );
  QCOMPARE( l28d.pointN( 1 ), QgsPointV2( QgsWKBTypes::PointZ, 11, 12, 3, 0 ) );

  //convertTo
  l28d.setPoints( QgsPointSequenceV2() << QgsPointV2( 1, 2 ) << QgsPointV2( 11, 12 ) );
  QVERIFY( l28d.convertTo( QgsWKBTypes::LineString ) );
  QCOMPARE( l28d.wkbType(), QgsWKBTypes::LineString );
  QVERIFY( l28d.convertTo( QgsWKBTypes::LineStringZ ) );
  QCOMPARE( l28d.wkbType(), QgsWKBTypes::LineStringZ );
  QCOMPARE( l28d.pointN( 0 ), QgsPointV2( QgsWKBTypes::PointZ, 1, 2, 0.0 ) );
  l28d.setZAt( 0, 5.0 );
  QVERIFY( l28d.convertTo( QgsWKBTypes::LineString25D ) );
  QCOMPARE( l28d.wkbType(), QgsWKBTypes::LineString25D );
  QCOMPARE( l28d.pointN( 0 ), QgsPointV2( QgsWKBTypes::Point25D, 1, 2, 5.0 ) );
  QVERIFY( l28d.convertTo( QgsWKBTypes::LineStringZM ) );
  QCOMPARE( l28d.wkbType(), QgsWKBTypes::LineStringZM );
  QCOMPARE( l28d.pointN( 0 ), QgsPointV2( QgsWKBTypes::PointZM, 1, 2, 5.0 ) );
  l28d.setMAt( 0, 6.0 );
  QVERIFY( l28d.convertTo( QgsWKBTypes::LineStringM ) );
  QCOMPARE( l28d.wkbType(), QgsWKBTypes::LineStringM );
  QCOMPARE( l28d.pointN( 0 ), QgsPointV2( QgsWKBTypes::PointM, 1, 2, 0.0, 6.0 ) );
  QVERIFY( l28d.convertTo( QgsWKBTypes::LineString ) );
  QCOMPARE( l28d.wkbType(), QgsWKBTypes::LineString );
  QCOMPARE( l28d.pointN( 0 ), QgsPointV2( 1, 2 ) );
  QVERIFY( !l28d.convertTo( QgsWKBTypes::Polygon ) );

  //isRing
  QgsLineStringV2 l30;
  QVERIFY( !l30.isRing() );
  l30.setPoints( QgsPointSequenceV2() << QgsPointV2( 1, 2 ) << QgsPointV2( 11, 12 ) << QgsPointV2( 1, 2 ) );
  QVERIFY( !l30.isRing() ); //<4 points
  l30.setPoints( QgsPointSequenceV2() << QgsPointV2( 1, 2 ) << QgsPointV2( 11, 12 ) << QgsPointV2( 21, 22 ) << QgsPointV2( 31, 32 ) );
  QVERIFY( !l30.isRing() ); //not closed
  l30.setPoints( QgsPointSequenceV2() << QgsPointV2( 1, 2 ) << QgsPointV2( 11, 12 ) << QgsPointV2( 21, 22 ) << QgsPointV2( 1, 2 ) );
  QVERIFY( l30.isRing() );

  //coordinateSequence
  QgsLineStringV2 l31;
  QgsCoordinateSequenceV2 coords = l31.coordinateSequence();
  QCOMPARE( coords.count(), 1 );
  QCOMPARE( coords.at( 0 ).count(), 1 );
  QVERIFY( coords.at( 0 ).at( 0 ).isEmpty() );
  l31.setPoints( QgsPointSequenceV2() << QgsPointV2( QgsWKBTypes::PointZM, 1, 2, 2, 3 )
                 << QgsPointV2( QgsWKBTypes::PointZM, 11, 12, 4, 5 )
                 << QgsPointV2( QgsWKBTypes::PointZM, 21, 22, 6, 7 ) );
  coords = l31.coordinateSequence();
  QCOMPARE( coords.count(), 1 );
  QCOMPARE( coords.at( 0 ).count(), 1 );
  QCOMPARE( coords.at( 0 ).at( 0 ).count(), 3 );
  QCOMPARE( coords.at( 0 ).at( 0 ).at( 0 ), QgsPointV2( QgsWKBTypes::PointZM, 1, 2, 2, 3 ) );
  QCOMPARE( coords.at( 0 ).at( 0 ).at( 1 ), QgsPointV2( QgsWKBTypes::PointZM, 11, 12, 4, 5 ) );
  QCOMPARE( coords.at( 0 ).at( 0 ).at( 2 ), QgsPointV2( QgsWKBTypes::PointZM, 21, 22, 6, 7 ) );

  //nextVertex

  QgsLineStringV2 l32;
  QgsVertexId v;
  QgsPointV2 p;
  QVERIFY( !l32.nextVertex( v, p ) );
  v = QgsVertexId( 0, 0, -2 );
  QVERIFY( !l32.nextVertex( v, p ) );
  v = QgsVertexId( 0, 0, 10 );
  QVERIFY( !l32.nextVertex( v, p ) );
  //LineString
  l32.setPoints( QgsPointSequenceV2() << QgsPointV2( 1, 2 ) << QgsPointV2( 11, 12 ) );
  v = QgsVertexId( 0, 0, 2 ); //out of range
  QVERIFY( !l32.nextVertex( v, p ) );
  v = QgsVertexId( 0, 0, -5 );
  QVERIFY( l32.nextVertex( v, p ) );
  v = QgsVertexId( 0, 0, -1 );
  QVERIFY( l32.nextVertex( v, p ) );
  QCOMPARE( v, QgsVertexId( 0, 0, 0 ) );
  QCOMPARE( p, QgsPointV2( 1, 2 ) );
  QVERIFY( l32.nextVertex( v, p ) );
  QCOMPARE( v, QgsVertexId( 0, 0, 1 ) );
  QCOMPARE( p, QgsPointV2( 11, 12 ) );
  QVERIFY( !l32.nextVertex( v, p ) );
  v = QgsVertexId( 0, 1, 0 );
  QVERIFY( l32.nextVertex( v, p ) );
  QCOMPARE( v, QgsVertexId( 0, 1, 1 ) ); //test that ring number is maintained
  QCOMPARE( p, QgsPointV2( 11, 12 ) );
  v = QgsVertexId( 1, 0, 0 );
  QVERIFY( l32.nextVertex( v, p ) );
  QCOMPARE( v, QgsVertexId( 1, 0, 1 ) ); //test that part number is maintained
  QCOMPARE( p, QgsPointV2( 11, 12 ) );

  //LineStringZ
  l32.setPoints( QgsPointSequenceV2() << QgsPointV2( QgsWKBTypes::PointZ, 1, 2, 3 ) << QgsPointV2( QgsWKBTypes::PointZ, 11, 12, 13 ) );
  v = QgsVertexId( 0, 0, -1 );
  QVERIFY( l32.nextVertex( v, p ) );
  QCOMPARE( v, QgsVertexId( 0, 0, 0 ) );
  QCOMPARE( p, QgsPointV2( QgsWKBTypes::PointZ, 1, 2, 3 ) );
  QVERIFY( l32.nextVertex( v, p ) );
  QCOMPARE( v, QgsVertexId( 0, 0, 1 ) );
  QCOMPARE( p, QgsPointV2( QgsWKBTypes::PointZ, 11, 12, 13 ) );
  QVERIFY( !l32.nextVertex( v, p ) );
  //LineStringM
  l32.setPoints( QgsPointSequenceV2() << QgsPointV2( QgsWKBTypes::PointM, 1, 2, 0, 4 ) << QgsPointV2( QgsWKBTypes::PointM, 11, 12, 0, 14 ) );
  v = QgsVertexId( 0, 0, -1 );
  QVERIFY( l32.nextVertex( v, p ) );
  QCOMPARE( v, QgsVertexId( 0, 0, 0 ) );
  QCOMPARE( p, QgsPointV2( QgsWKBTypes::PointM, 1, 2, 0, 4 ) );
  QVERIFY( l32.nextVertex( v, p ) );
  QCOMPARE( v, QgsVertexId( 0, 0, 1 ) );
  QCOMPARE( p, QgsPointV2( QgsWKBTypes::PointM, 11, 12, 0, 14 ) );
  QVERIFY( !l32.nextVertex( v, p ) );
  //LineStringZM
  l32.setPoints( QgsPointSequenceV2() << QgsPointV2( QgsWKBTypes::PointZM, 1, 2, 3, 4 ) << QgsPointV2( QgsWKBTypes::PointZM, 11, 12, 13, 14 ) );
  v = QgsVertexId( 0, 0, -1 );
  QVERIFY( l32.nextVertex( v, p ) );
  QCOMPARE( v, QgsVertexId( 0, 0, 0 ) );
  QCOMPARE( p, QgsPointV2( QgsWKBTypes::PointZM, 1, 2, 3, 4 ) );
  QVERIFY( l32.nextVertex( v, p ) );
  QCOMPARE( v, QgsVertexId( 0, 0, 1 ) );
  QCOMPARE( p, QgsPointV2( QgsWKBTypes::PointZM, 11, 12, 13, 14 ) );
  QVERIFY( !l32.nextVertex( v, p ) );
  //LineString25D
  l32.setPoints( QgsPointSequenceV2() << QgsPointV2( QgsWKBTypes::Point25D, 1, 2, 3 ) << QgsPointV2( QgsWKBTypes::Point25D, 11, 12, 13 ) );
  v = QgsVertexId( 0, 0, -1 );
  QVERIFY( l32.nextVertex( v, p ) );
  QCOMPARE( v, QgsVertexId( 0, 0, 0 ) );
  QCOMPARE( p, QgsPointV2( QgsWKBTypes::Point25D, 1, 2, 3 ) );
  QVERIFY( l32.nextVertex( v, p ) );
  QCOMPARE( v, QgsVertexId( 0, 0, 1 ) );
  QCOMPARE( p, QgsPointV2( QgsWKBTypes::Point25D, 11, 12, 13 ) );
  QVERIFY( !l32.nextVertex( v, p ) );

  //vertexAt and pointAt
  QgsLineStringV2 l33;
  l33.vertexAt( QgsVertexId( 0, 0, -10 ) ); //out of bounds, check for no crash
  l33.vertexAt( QgsVertexId( 0, 0, 10 ) ); //out of bounds, check for no crash
  QgsVertexId::VertexType type;
  QVERIFY( !l33.pointAt( -10, p, type ) );
  QVERIFY( !l33.pointAt( 10, p, type ) );
  //LineString
  l33.setPoints( QgsPointSequenceV2() << QgsPointV2( 1, 2 ) << QgsPointV2( 11, 12 ) );
  l33.vertexAt( QgsVertexId( 0, 0, -10 ) );
  l33.vertexAt( QgsVertexId( 0, 0, 10 ) ); //out of bounds, check for no crash
  QCOMPARE( l33.vertexAt( QgsVertexId( 0, 0, 0 ) ), QgsPointV2( 1, 2 ) );
  QCOMPARE( l33.vertexAt( QgsVertexId( 0, 0, 1 ) ), QgsPointV2( 11, 12 ) );
  QVERIFY( !l33.pointAt( -10, p, type ) );
  QVERIFY( !l33.pointAt( 10, p, type ) );
  QVERIFY( l33.pointAt( 0, p, type ) );
  QCOMPARE( p, QgsPointV2( 1, 2 ) );
  QCOMPARE( type, QgsVertexId::SegmentVertex );
  QVERIFY( l33.pointAt( 1, p, type ) );
  QCOMPARE( p, QgsPointV2( 11, 12 ) );
  QCOMPARE( type, QgsVertexId::SegmentVertex );
  //LineStringZ
  l33.setPoints( QgsPointSequenceV2() << QgsPointV2( QgsWKBTypes::PointZ, 1, 2, 3 ) << QgsPointV2( QgsWKBTypes::PointZ, 11, 12, 13 ) );
  QCOMPARE( l33.vertexAt( QgsVertexId( 0, 0, 0 ) ), QgsPointV2( QgsWKBTypes::PointZ, 1, 2, 3 ) );
  QCOMPARE( l33.vertexAt( QgsVertexId( 0, 0, 1 ) ), QgsPointV2( QgsWKBTypes::PointZ, 11, 12, 13 ) );
  QVERIFY( l33.pointAt( 0, p, type ) );
  QCOMPARE( p, QgsPointV2( QgsWKBTypes::PointZ, 1, 2, 3 ) );
  QCOMPARE( type, QgsVertexId::SegmentVertex );
  QVERIFY( l33.pointAt( 1, p, type ) );
  QCOMPARE( p, QgsPointV2( QgsWKBTypes::PointZ, 11, 12, 13 ) );
  QCOMPARE( type, QgsVertexId::SegmentVertex );
  //LineStringM
  l33.setPoints( QgsPointSequenceV2() << QgsPointV2( QgsWKBTypes::PointM, 1, 2, 0, 4 ) << QgsPointV2( QgsWKBTypes::PointM, 11, 12, 0, 14 ) );
  QCOMPARE( l33.vertexAt( QgsVertexId( 0, 0, 0 ) ), QgsPointV2( QgsWKBTypes::PointM, 1, 2, 0, 4 ) );
  QCOMPARE( l33.vertexAt( QgsVertexId( 0, 0, 1 ) ), QgsPointV2( QgsWKBTypes::PointM, 11, 12, 0, 14 ) );
  QVERIFY( l33.pointAt( 0, p, type ) );
  QCOMPARE( p, QgsPointV2( QgsWKBTypes::PointM, 1, 2, 0, 4 ) );
  QCOMPARE( type, QgsVertexId::SegmentVertex );
  QVERIFY( l33.pointAt( 1, p, type ) );
  QCOMPARE( p, QgsPointV2( QgsWKBTypes::PointM, 11, 12, 0, 14 ) );
  QCOMPARE( type, QgsVertexId::SegmentVertex );
  //LineStringZM
  l33.setPoints( QgsPointSequenceV2() << QgsPointV2( QgsWKBTypes::PointZM, 1, 2, 3, 4 ) << QgsPointV2( QgsWKBTypes::PointZM, 11, 12, 13, 14 ) );
  QCOMPARE( l33.vertexAt( QgsVertexId( 0, 0, 0 ) ), QgsPointV2( QgsWKBTypes::PointZM, 1, 2, 3, 4 ) );
  QCOMPARE( l33.vertexAt( QgsVertexId( 0, 0, 1 ) ), QgsPointV2( QgsWKBTypes::PointZM, 11, 12, 13, 14 ) );
  QVERIFY( l33.pointAt( 0, p, type ) );
  QCOMPARE( p, QgsPointV2( QgsWKBTypes::PointZM, 1, 2, 3, 4 ) );
  QCOMPARE( type, QgsVertexId::SegmentVertex );
  QVERIFY( l33.pointAt( 1, p, type ) );
  QCOMPARE( p, QgsPointV2( QgsWKBTypes::PointZM, 11, 12, 13, 14 ) );
  QCOMPARE( type, QgsVertexId::SegmentVertex );
  //LineString25D
  l33.setPoints( QgsPointSequenceV2() << QgsPointV2( QgsWKBTypes::Point25D, 1, 2, 3 ) << QgsPointV2( QgsWKBTypes::Point25D, 11, 12, 13 ) );
  QCOMPARE( l33.vertexAt( QgsVertexId( 0, 0, 0 ) ), QgsPointV2( QgsWKBTypes::Point25D, 1, 2, 3 ) );
  QCOMPARE( l33.vertexAt( QgsVertexId( 0, 0, 1 ) ), QgsPointV2( QgsWKBTypes::Point25D, 11, 12, 13 ) );
  QVERIFY( l33.pointAt( 0, p, type ) );
  QCOMPARE( p, QgsPointV2( QgsWKBTypes::Point25D, 1, 2, 3 ) );
  QCOMPARE( type, QgsVertexId::SegmentVertex );
  QVERIFY( l33.pointAt( 1, p, type ) );
  QCOMPARE( p, QgsPointV2( QgsWKBTypes::Point25D, 11, 12, 13 ) );
  QCOMPARE( type, QgsVertexId::SegmentVertex );

  //centroid
  QgsLineStringV2 l34;
  QCOMPARE( l34.centroid(), QgsPointV2() );
  l34.setPoints( QgsPointSequenceV2() << QgsPointV2( 5, 10 ) );
  QCOMPARE( l34.centroid(), QgsPointV2( 5, 10 ) );
  l34.setPoints( QgsPointSequenceV2() << QgsPointV2( 0, 0 ) << QgsPointV2( 20, 10 ) );
  QCOMPARE( l34.centroid(), QgsPointV2( 10, 5 ) );
  l34.setPoints( QgsPointSequenceV2() << QgsPointV2( 0, 0 ) << QgsPointV2( 0, 9 ) << QgsPointV2( 2, 9 ) << QgsPointV2( 2, 0 ) );
  QCOMPARE( l34.centroid(), QgsPointV2( 1, 4.95 ) );
  //linestring with 0 length segment
  l34.setPoints( QgsPointSequenceV2() << QgsPointV2( 0, 0 ) << QgsPointV2( 0, 9 ) << QgsPointV2( 2, 9 ) << QgsPointV2( 2, 9 ) << QgsPointV2( 2, 0 ) );
  QCOMPARE( l34.centroid(), QgsPointV2( 1, 4.95 ) );
  //linestring with 0 total length segment
  l34.setPoints( QgsPointSequenceV2() << QgsPointV2( 5, 4 ) << QgsPointV2( 5, 4 ) << QgsPointV2( 5, 4 ) );
  QCOMPARE( l34.centroid(), QgsPointV2( 5, 4 ) );

  //closest segment
  QgsLineStringV2 l35;
  bool leftOf = false;
  ( void )l35.closestSegment( QgsPointV2( 1, 2 ), p, v, 0, 0 ); //empty line, just want no crash
  l35.setPoints( QgsPointSequenceV2() << QgsPointV2( 5, 10 ) );
  QVERIFY( qgsDoubleNear( l35.closestSegment( QgsPointV2( 5, 10 ), p, v, 0, 0 ), 0 ) );
  QCOMPARE( p, QgsPointV2( 5, 10 ) );
  QCOMPARE( v, QgsVertexId( 0, 0, 1 ) );
  l35.setPoints( QgsPointSequenceV2() << QgsPointV2( 5, 10 ) << QgsPointV2( 10, 10 ) );
  QVERIFY( qgsDoubleNear( l35.closestSegment( QgsPointV2( 4, 11 ), p, v, &leftOf, 0 ), 2.0 ) );
  QCOMPARE( p, QgsPointV2( 5, 10 ) );
  QCOMPARE( v, QgsVertexId( 0, 0, 1 ) );
  QCOMPARE( leftOf, true );
  QVERIFY( qgsDoubleNear( l35.closestSegment( QgsPointV2( 8, 11 ), p, v, &leftOf, 0 ), 1.0 ) );
  QCOMPARE( p, QgsPointV2( 8, 10 ) );
  QCOMPARE( v, QgsVertexId( 0, 0, 1 ) );
  QCOMPARE( leftOf, true );
  QVERIFY( qgsDoubleNear( l35.closestSegment( QgsPointV2( 8, 9 ), p, v, &leftOf, 0 ), 1.0 ) );
  QCOMPARE( p, QgsPointV2( 8, 10 ) );
  QCOMPARE( v, QgsVertexId( 0, 0, 1 ) );
  QCOMPARE( leftOf, false );
  QVERIFY( qgsDoubleNear( l35.closestSegment( QgsPointV2( 11, 9 ), p, v, &leftOf, 0 ), 2.0 ) );
  QCOMPARE( p, QgsPointV2( 10, 10 ) );
  QCOMPARE( v, QgsVertexId( 0, 0, 1 ) );
  QCOMPARE( leftOf, false );
  l35.setPoints( QgsPointSequenceV2() << QgsPointV2( 5, 10 )
                 << QgsPointV2( 10, 10 )
                 << QgsPointV2( 10, 15 ) );
  QVERIFY( qgsDoubleNear( l35.closestSegment( QgsPointV2( 11, 12 ), p, v, &leftOf, 0 ), 1.0 ) );
  QCOMPARE( p, QgsPointV2( 10, 12 ) );
  QCOMPARE( v, QgsVertexId( 0, 0, 2 ) );
  QCOMPARE( leftOf, false );

  //sumUpArea
  QgsLineStringV2 l36;
  double area = 1.0; //sumUpArea adds to area, so start with non-zero value
  l36.sumUpArea( area );
  QCOMPARE( area, 1.0 );
  l36.setPoints( QgsPointSequenceV2() << QgsPointV2( 5, 10 ) );
  l36.sumUpArea( area );
  QCOMPARE( area, 1.0 );
  l36.setPoints( QgsPointSequenceV2() << QgsPointV2( 5, 10 ) << QgsPointV2( 10, 10 ) );
  l36.sumUpArea( area );
  QCOMPARE( area, 1.0 );
  l36.setPoints( QgsPointSequenceV2() << QgsPointV2( 0, 0 ) << QgsPointV2( 2, 0 ) << QgsPointV2( 2, 2 ) );
  l36.sumUpArea( area );
  QVERIFY( qgsDoubleNear( area, 3.0 ) );
  l36.setPoints( QgsPointSequenceV2() << QgsPointV2( 0, 0 ) << QgsPointV2( 2, 0 ) << QgsPointV2( 2, 2 ) << QgsPointV2( 0, 2 ) );
  l36.sumUpArea( area );
  QVERIFY( qgsDoubleNear( area, 7.0 ) );

  //boundingBox - test that bounding box is updated after every modification to the line string
  QgsLineStringV2 l37;
  QVERIFY( l37.boundingBox().isNull() );
  l37.setPoints( QgsPointSequenceV2() << QgsPointV2( 5, 10 ) << QgsPointV2( 10, 15 ) );
  QCOMPARE( l37.boundingBox(), QgsRectangle( 5, 10, 10, 15 ) );
  l37.setPoints( QgsPointSequenceV2() << QgsPointV2( -5, -10 ) << QgsPointV2( -6, -10 ) << QgsPointV2( -5.5, -9 ) );
  QCOMPARE( l37.boundingBox(), QgsRectangle( -6, -10, -5, -9 ) );
  //setXAt
  l37.setXAt( 2, -4 );
  QCOMPARE( l37.boundingBox(), QgsRectangle( -6, -10, -4, -9 ) );
  //setYAt
  l37.setYAt( 1, -15 );
  QCOMPARE( l37.boundingBox(), QgsRectangle( -6, -15, -4, -9 ) );
  //append
  toAppend.reset( new QgsLineStringV2() );
  toAppend->setPoints( QgsPointSequenceV2() << QgsPointV2( 1, 2 ) << QgsPointV2( 4, 0 ) );
  l37.append( toAppend.data() );
  QCOMPARE( l37.boundingBox(), QgsRectangle( -6, -15, 4, 2 ) );
  l37.addVertex( QgsPointV2( 6, 3 ) );
  QCOMPARE( l37.boundingBox(), QgsRectangle( -6, -15, 6, 3 ) );
  l37.clear();
  QVERIFY( l37.boundingBox().isNull() );
  l37.setPoints( QgsPointSequenceV2() << QgsPointV2( 5, 10 ) << QgsPointV2( 10, 15 ) );
  wkb = toAppend->asWkb( size );
  l37.fromWkb( QgsConstWkbPtr( wkb, size ) );
  delete[] wkb;
  wkb = 0;
  QCOMPARE( l37.boundingBox(), QgsRectangle( 1, 0, 4, 2 ) );
  l37.fromWkt( QString( "LineString( 1 5, 3 4, 6 3 )" ) );
  QCOMPARE( l37.boundingBox(), QgsRectangle( 1, 3, 6, 5 ) );
  l37.insertVertex( QgsVertexId( 0, 0, 1 ), QgsPointV2( -1, 7 ) );
  QCOMPARE( l37.boundingBox(), QgsRectangle( -1, 3, 6, 7 ) );
  l37.moveVertex( QgsVertexId( 0, 0, 1 ), QgsPointV2( -3, 10 ) );
  QCOMPARE( l37.boundingBox(), QgsRectangle( -3, 3, 6, 10 ) );
  l37.deleteVertex( QgsVertexId( 0, 0, 1 ) );
  QCOMPARE( l37.boundingBox(), QgsRectangle( 1, 3, 6, 5 ) );

  //angle
  QgsLineStringV2 l38;
  ( void )l38.vertexAngle( QgsVertexId() ); //just want no crash
  ( void )l38.vertexAngle( QgsVertexId( 0, 0, 0 ) ); //just want no crash
  l38.setPoints( QgsPointSequenceV2() << QgsPointV2( 0, 0 ) );
  ( void )l38.vertexAngle( QgsVertexId( 0, 0, 0 ) ); //just want no crash, any answer is meaningless
  l38.setPoints( QgsPointSequenceV2() << QgsPointV2( 0, 0 ) << QgsPointV2( 1, 0 ) );
  QVERIFY( qgsDoubleNear( l38.vertexAngle( QgsVertexId( 0, 0, 0 ) ), 1.5708, 0.0001 ) );
  QVERIFY( qgsDoubleNear( l38.vertexAngle( QgsVertexId( 0, 0, 1 ) ), 1.5708, 0.0001 ) );
  ( void )l38.vertexAngle( QgsVertexId( 0, 0, 2 ) ); //no crash
  l38.setPoints( QgsPointSequenceV2() << QgsPointV2( 0, 0 ) << QgsPointV2( 0, 1 ) );
  QVERIFY( qgsDoubleNear( l38.vertexAngle( QgsVertexId( 0, 0, 0 ) ), 0.0 ) );
  QVERIFY( qgsDoubleNear( l38.vertexAngle( QgsVertexId( 0, 0, 1 ) ), 0.0 ) );
  l38.setPoints( QgsPointSequenceV2() << QgsPointV2( 1, 0 ) << QgsPointV2( 0, 0 ) );
  QVERIFY( qgsDoubleNear( l38.vertexAngle( QgsVertexId( 0, 0, 0 ) ), 4.71239, 0.0001 ) );
  QVERIFY( qgsDoubleNear( l38.vertexAngle( QgsVertexId( 0, 0, 1 ) ), 4.71239, 0.0001 ) );
  l38.setPoints( QgsPointSequenceV2() << QgsPointV2( 0, 1 ) << QgsPointV2( 0, 0 ) );
  QVERIFY( qgsDoubleNear( l38.vertexAngle( QgsVertexId( 0, 0, 0 ) ), 3.1416, 0.0001 ) );
  QVERIFY( qgsDoubleNear( l38.vertexAngle( QgsVertexId( 0, 0, 1 ) ), 3.1416, 0.0001 ) );
  l38.setPoints( QgsPointSequenceV2() << QgsPointV2( 0, 0 ) << QgsPointV2( 1, 0 ) << QgsPointV2( 1, 1 ) );
  QVERIFY( qgsDoubleNear( l38.vertexAngle( QgsVertexId( 0, 0, 0 ) ), 1.5708, 0.0001 ) );
  QVERIFY( qgsDoubleNear( l38.vertexAngle( QgsVertexId( 0, 0, 1 ) ), 0.7854, 0.0001 ) );
  QVERIFY( qgsDoubleNear( l38.vertexAngle( QgsVertexId( 0, 0, 2 ) ), 0.0, 0.0001 ) );
  l38.setPoints( QgsPointSequenceV2() << QgsPointV2( 0, 0 ) << QgsPointV2( 0.5, 0 ) << QgsPointV2( 1, 0 )
                 << QgsPointV2( 2, 1 ) << QgsPointV2( 1, 2 ) << QgsPointV2( 0, 2 ) );
  ( void )l38.vertexAngle( QgsVertexId( 0, 0, 20 ) );
  QVERIFY( qgsDoubleNear( l38.vertexAngle( QgsVertexId( 0, 0, 0 ) ), 1.5708, 0.0001 ) );
  QVERIFY( qgsDoubleNear( l38.vertexAngle( QgsVertexId( 0, 0, 1 ) ), 1.5708, 0.0001 ) );
  QVERIFY( qgsDoubleNear( l38.vertexAngle( QgsVertexId( 0, 0, 2 ) ), 1.17809, 0.00001 ) );
  QVERIFY( qgsDoubleNear( l38.vertexAngle( QgsVertexId( 0, 0, 3 ) ), 0.0, 0.00001 ) );
  QVERIFY( qgsDoubleNear( l38.vertexAngle( QgsVertexId( 0, 0, 4 ) ), 5.10509, 0.00001 ) );
  QVERIFY( qgsDoubleNear( l38.vertexAngle( QgsVertexId( 0, 0, 5 ) ), 4.71239, 0.00001 ) );
  //closed line string
  l38.close();
  QVERIFY( qgsDoubleNear( l38.vertexAngle( QgsVertexId( 0, 0, 5 ) ), 3.92699, 0.00001 ) );
  QVERIFY( qgsDoubleNear( l38.vertexAngle( QgsVertexId( 0, 0, 0 ) ), 2.35619, 0.00001 ) );
  QVERIFY( qgsDoubleNear( l38.vertexAngle( QgsVertexId( 0, 0, 6 ) ), 2.35619, 0.00001 ) );

  //removing the second to last vertex should remove the whole line
  QgsLineStringV2 l39;
  l39.setPoints( QList<QgsPointV2>() << QgsPointV2( 0, 0 ) << QgsPointV2( 1, 1 ) );
  QVERIFY( l39.numPoints() == 2 );
  l39.deleteVertex( QgsVertexId( 0, 0, 1 ) );
  QVERIFY( l39.numPoints() == 0 );
}

void TestQgsGeometry::polygonV2()
{
  //test constructor
  QgsPolygonV2 p1;
  QVERIFY( p1.isEmpty() );
  QCOMPARE( p1.numInteriorRings(), 0 );
  QCOMPARE( p1.nCoordinates(), 0 );
  QCOMPARE( p1.ringCount(), 0 );
  QCOMPARE( p1.partCount(), 0 );
  QVERIFY( !p1.is3D() );
  QVERIFY( !p1.isMeasure() );
  QCOMPARE( p1.wkbType(), QgsWKBTypes::Polygon );
  QCOMPARE( p1.wktTypeStr(), QString( "Polygon" ) );
  QCOMPARE( p1.geometryType(), QString( "Polygon" ) );
  QCOMPARE( p1.dimension(), 2 );
  QVERIFY( !p1.hasCurvedSegments() );
  QCOMPARE( p1.area(), 0.0 );
  QCOMPARE( p1.perimeter(), 0.0 );
  QVERIFY( !p1.exteriorRing() );
  QVERIFY( !p1.interiorRing( 0 ) );

  //set exterior ring

  //try with no ring
  QgsLineStringV2* ext = 0;
  p1.setExteriorRing( ext );
  QVERIFY( p1.isEmpty() );
  QCOMPARE( p1.numInteriorRings(), 0 );
  QCOMPARE( p1.nCoordinates(), 0 );
  QCOMPARE( p1.ringCount(), 0 );
  QCOMPARE( p1.partCount(), 0 );
  QVERIFY( !p1.exteriorRing() );
  QVERIFY( !p1.interiorRing( 0 ) );
  QCOMPARE( p1.wkbType(), QgsWKBTypes::Polygon );

  //valid exterior ring
  ext = new QgsLineStringV2();
  ext->setPoints( QgsPointSequenceV2() << QgsPointV2( 0, 0 ) << QgsPointV2( 0, 10 ) << QgsPointV2( 10, 10 )
                  << QgsPointV2( 10, 0 ) << QgsPointV2( 0, 0 ) );
  p1.setExteriorRing( ext );
  QVERIFY( !p1.isEmpty() );
  QCOMPARE( p1.numInteriorRings(), 0 );
  QCOMPARE( p1.nCoordinates(), 5 );
  QCOMPARE( p1.ringCount(), 1 );
  QCOMPARE( p1.partCount(), 1 );
  QVERIFY( !p1.is3D() );
  QVERIFY( !p1.isMeasure() );
  QCOMPARE( p1.wkbType(), QgsWKBTypes::Polygon );
  QCOMPARE( p1.wktTypeStr(), QString( "Polygon" ) );
  QCOMPARE( p1.geometryType(), QString( "Polygon" ) );
  QCOMPARE( p1.dimension(), 2 );
  QVERIFY( !p1.hasCurvedSegments() );
  QCOMPARE( p1.area(), 100.0 );
  QCOMPARE( p1.perimeter(), 40.0 );
  QVERIFY( p1.exteriorRing() );
  QVERIFY( !p1.interiorRing( 0 ) );

  //retrieve exterior ring and check
  QCOMPARE( *( static_cast< const QgsLineStringV2* >( p1.exteriorRing() ) ), *ext );

  //test that a non closed exterior ring will be automatically closed
  ext = new QgsLineStringV2();
  ext->setPoints( QgsPointSequenceV2() << QgsPointV2( 0, 0 ) << QgsPointV2( 0, 10 ) << QgsPointV2( 10, 10 )
                  << QgsPointV2( 10, 0 ) );
  QVERIFY( !ext->isClosed() );
  p1.setExteriorRing( ext );
  QVERIFY( !p1.isEmpty() );
  QVERIFY( p1.exteriorRing()->isClosed() );
  QCOMPARE( p1.nCoordinates(), 5 );

  //initial setting of exterior ring should set z/m type
  QgsPolygonV2 p2;
  ext = new QgsLineStringV2();
  ext->setPoints( QgsPointSequenceV2() << QgsPointV2( QgsWKBTypes::PointZ, 0, 0, 1 )
                  << QgsPointV2( QgsWKBTypes::PointZ, 0, 10, 2 ) << QgsPointV2( QgsWKBTypes::PointZ, 10, 10, 3 )
                  << QgsPointV2( QgsWKBTypes::PointZ, 10, 0, 4 ) << QgsPointV2( QgsWKBTypes::PointZ, 0, 0, 1 ) );
  p2.setExteriorRing( ext );
  QVERIFY( p2.is3D() );
  QVERIFY( !p2.isMeasure() );
  QCOMPARE( p2.wkbType(), QgsWKBTypes::PolygonZ );
  QCOMPARE( p2.wktTypeStr(), QString( "PolygonZ" ) );
  QCOMPARE( p2.geometryType(), QString( "Polygon" ) );
  QCOMPARE( *( static_cast< const QgsLineStringV2* >( p2.exteriorRing() ) ), *ext );
  QgsPolygonV2 p3;
  ext = new QgsLineStringV2();
  ext->setPoints( QgsPointSequenceV2() << QgsPointV2( QgsWKBTypes::PointM, 0, 0, 0, 1 )
                  << QgsPointV2( QgsWKBTypes::PointM, 0, 10, 0, 2 ) << QgsPointV2( QgsWKBTypes::PointM, 10, 10, 0, 3 )
                  << QgsPointV2( QgsWKBTypes::PointM, 10, 0, 0, 4 ) << QgsPointV2( QgsWKBTypes::PointM, 0, 0, 0, 1 ) );
  p3.setExteriorRing( ext );
  QVERIFY( !p3.is3D() );
  QVERIFY( p3.isMeasure() );
  QCOMPARE( p3.wkbType(), QgsWKBTypes::PolygonM );
  QCOMPARE( p3.wktTypeStr(), QString( "PolygonM" ) );
  QCOMPARE( *( static_cast< const QgsLineStringV2* >( p3.exteriorRing() ) ), *ext );
  QgsPolygonV2 p4;
  ext = new QgsLineStringV2();
  ext->setPoints( QgsPointSequenceV2() << QgsPointV2( QgsWKBTypes::PointZM, 0, 0, 2, 1 )
                  << QgsPointV2( QgsWKBTypes::PointZM, 0, 10, 3, 2 ) << QgsPointV2( QgsWKBTypes::PointZM, 10, 10, 5, 3 )
                  << QgsPointV2( QgsWKBTypes::PointZM, 10, 0, 0, 4 ) << QgsPointV2( QgsWKBTypes::PointZM, 0, 0, 2, 1 ) );
  p4.setExteriorRing( ext );
  QVERIFY( p4.is3D() );
  QVERIFY( p4.isMeasure() );
  QCOMPARE( p4.wkbType(), QgsWKBTypes::PolygonZM );
  QCOMPARE( p4.wktTypeStr(), QString( "PolygonZM" ) );
  QCOMPARE( *( static_cast< const QgsLineStringV2* >( p4.exteriorRing() ) ), *ext );
  QgsPolygonV2 p5;
  ext = new QgsLineStringV2();
  ext->setPoints( QgsPointSequenceV2() << QgsPointV2( QgsWKBTypes::Point25D, 0, 0, 1 )
                  << QgsPointV2( QgsWKBTypes::Point25D, 0, 10, 2 ) << QgsPointV2( QgsWKBTypes::Point25D, 10, 10, 3 )
                  << QgsPointV2( QgsWKBTypes::Point25D, 10, 0, 4 ) << QgsPointV2( QgsWKBTypes::Point25D, 0, 0, 1 ) );
  p5.setExteriorRing( ext );
  QVERIFY( p5.is3D() );
  QVERIFY( !p5.isMeasure() );
  QCOMPARE( p5.wkbType(), QgsWKBTypes::Polygon25D );
  QCOMPARE( p5.wktTypeStr(), QString( "PolygonZ" ) );
  QCOMPARE( *( static_cast< const QgsLineStringV2* >( p5.exteriorRing() ) ), *ext );

  //setting curved exterior ring should be segmentized
  QgsCircularStringV2* circularRing = new QgsCircularStringV2();
  circularRing->setPoints( QgsPointSequenceV2() << QgsPointV2( 0, 0 ) << QgsPointV2( 0, 10 ) << QgsPointV2( 10, 10 )
                           << QgsPointV2( 10, 0 ) << QgsPointV2( 0, 0 ) );
  QVERIFY( circularRing->hasCurvedSegments() );
  p5.setExteriorRing( circularRing );
  QVERIFY( !p5.exteriorRing()->hasCurvedSegments() );
  QCOMPARE( p5.exteriorRing()->wkbType(), QgsWKBTypes::LineString );

  //addInteriorRing
  QgsPolygonV2 p6;
  ext = new QgsLineStringV2();
  ext->setPoints( QgsPointSequenceV2() << QgsPointV2( 0, 0 ) << QgsPointV2( 0, 10 ) << QgsPointV2( 10, 10 )
                  << QgsPointV2( 10, 0 ) << QgsPointV2( 0, 0 ) );
  p6.setExteriorRing( ext );
  //empty ring
  QCOMPARE( p6.numInteriorRings(), 0 );
  QVERIFY( !p6.interiorRing( -1 ) );
  QVERIFY( !p6.interiorRing( 0 ) );
  p6.addInteriorRing( 0 );
  QCOMPARE( p6.numInteriorRings(), 0 );
  QgsLineStringV2* ring = new QgsLineStringV2();
  ring->setPoints( QgsPointSequenceV2() << QgsPointV2( 1, 1 ) << QgsPointV2( 1, 9 ) << QgsPointV2( 9, 9 )
                   << QgsPointV2( 9, 1 ) << QgsPointV2( 1, 1 ) );
  p6.addInteriorRing( ring );
  QCOMPARE( p6.numInteriorRings(), 1 );
  QCOMPARE( p6.interiorRing( 0 ), ring );
  QVERIFY( !p6.interiorRing( 1 ) );

  //add non-closed interior ring, should be closed automatically
  ring = new QgsLineStringV2();
  ring->setPoints( QgsPointSequenceV2() << QgsPointV2( 0.1, 0.1 ) << QgsPointV2( 0.1, 0.9 ) << QgsPointV2( 0.9, 0.9 )
                   << QgsPointV2( 0.9, 0.1 ) );
  QVERIFY( !ring->isClosed() );
  p6.addInteriorRing( ring );
  QCOMPARE( p6.numInteriorRings(), 2 );
  QVERIFY( p6.interiorRing( 1 )->isClosed() );

  //try adding an interior ring with z to a 2d polygon, z should be dropped
  ring = new QgsLineStringV2();
  ring->setPoints( QgsPointSequenceV2() << QgsPointV2( QgsWKBTypes::PointZ, 0.1, 0.1, 1 )
                   << QgsPointV2( QgsWKBTypes::PointZ, 0.1, 0.2, 2 ) << QgsPointV2( QgsWKBTypes::PointZ, 0.2, 0.2, 3 )
                   << QgsPointV2( QgsWKBTypes::PointZ, 0.2, 0.1, 4 ) << QgsPointV2( QgsWKBTypes::PointZ, 0.1, 0.1, 1 ) );
  p6.addInteriorRing( ring );
  QCOMPARE( p6.numInteriorRings(), 3 );
  QVERIFY( !p6.is3D() );
  QVERIFY( !p6.isMeasure() );
  QCOMPARE( p6.wkbType(), QgsWKBTypes::Polygon );
  QVERIFY( p6.interiorRing( 2 ) );
  QVERIFY( !p6.interiorRing( 2 )->is3D() );
  QVERIFY( !p6.interiorRing( 2 )->isMeasure() );
  QCOMPARE( p6.interiorRing( 2 )->wkbType(), QgsWKBTypes::LineString );

  //try adding an interior ring with m to a 2d polygon, m should be dropped
  ring = new QgsLineStringV2();
  ring->setPoints( QgsPointSequenceV2() << QgsPointV2( QgsWKBTypes::PointM, 0.1, 0.1, 0, 1 )
                   << QgsPointV2( QgsWKBTypes::PointM, 0.1, 0.2, 0, 2 ) << QgsPointV2( QgsWKBTypes::PointM, 0.2, 0.2, 0, 3 )
                   << QgsPointV2( QgsWKBTypes::PointM, 0.2, 0.1, 0, 4 ) << QgsPointV2( QgsWKBTypes::PointM, 0.1, 0.1, 0, 1 ) );
  p6.addInteriorRing( ring );
  QCOMPARE( p6.numInteriorRings(), 4 );
  QVERIFY( !p6.is3D() );
  QVERIFY( !p6.isMeasure() );
  QCOMPARE( p6.wkbType(), QgsWKBTypes::Polygon );
  QVERIFY( p6.interiorRing( 3 ) );
  QVERIFY( !p6.interiorRing( 3 )->is3D() );
  QVERIFY( !p6.interiorRing( 3 )->isMeasure() );
  QCOMPARE( p6.interiorRing( 3 )->wkbType(), QgsWKBTypes::LineString );

  //addInteriorRing without z/m to PolygonZM
  QgsPolygonV2 p6b;
  ext = new QgsLineStringV2();
  ext->setPoints( QgsPointSequenceV2() << QgsPointV2( QgsWKBTypes::PointZM, 0, 0, 1 )
                  << QgsPointV2( QgsWKBTypes::PointZM, 0, 10, 2 ) << QgsPointV2( QgsWKBTypes::PointZM, 10, 10, 3 )
                  << QgsPointV2( QgsWKBTypes::PointZM, 10, 0, 4 ) << QgsPointV2( QgsWKBTypes::PointZM, 0, 0, 1 ) );
  p6b.setExteriorRing( ext );
  QVERIFY( p6b.is3D() );
  QVERIFY( p6b.isMeasure() );
  QCOMPARE( p6b.wkbType(), QgsWKBTypes::PolygonZM );
  //ring has no z
  ring = new QgsLineStringV2();
  ring->setPoints( QgsPointSequenceV2() << QgsPointV2( QgsWKBTypes::PointM, 1, 1, 0, 2 ) << QgsPointV2( QgsWKBTypes::PointM, 1, 9 ) << QgsPointV2( QgsWKBTypes::PointM, 9, 9 )
                   << QgsPointV2( QgsWKBTypes::PointM, 9, 1 ) << QgsPointV2( QgsWKBTypes::PointM, 1, 1 ) );
  p6b.addInteriorRing( ring );
  QVERIFY( p6b.interiorRing( 0 ) );
  QVERIFY( p6b.interiorRing( 0 )->is3D() );
  QVERIFY( p6b.interiorRing( 0 )->isMeasure() );
  QCOMPARE( p6b.interiorRing( 0 )->wkbType(), QgsWKBTypes::LineStringZM );
  QCOMPARE( p6b.interiorRing( 0 )->vertexAt( QgsVertexId( 0, 0, 0 ) ), QgsPointV2( QgsWKBTypes::PointZM, 1, 1, 0, 2 ) );
  //ring has no m
  ring = new QgsLineStringV2();
  ring->setPoints( QgsPointSequenceV2() << QgsPointV2( QgsWKBTypes::PointZ, 0.1, 0.1, 1 )
                   << QgsPointV2( QgsWKBTypes::PointZ, 0.1, 0.2, 2 ) << QgsPointV2( QgsWKBTypes::PointZ, 0.2, 0.2, 3 )
                   << QgsPointV2( QgsWKBTypes::PointZ, 0.2, 0.1, 4 ) << QgsPointV2( QgsWKBTypes::PointZ, 0.1, 0.1, 1 ) );
  p6b.addInteriorRing( ring );
  QVERIFY( p6b.interiorRing( 1 ) );
  QVERIFY( p6b.interiorRing( 1 )->is3D() );
  QVERIFY( p6b.interiorRing( 1 )->isMeasure() );
  QCOMPARE( p6b.interiorRing( 1 )->wkbType(), QgsWKBTypes::LineStringZM );
  QCOMPARE( p6b.interiorRing( 1 )->vertexAt( QgsVertexId( 0, 0, 0 ) ), QgsPointV2( QgsWKBTypes::PointZM, 0.1, 0.1, 1, 0 ) );
  //test handling of 25D rings/polygons
  QgsPolygonV2 p6c;
  ext = new QgsLineStringV2();
  ext->setPoints( QgsPointSequenceV2() << QgsPointV2( QgsWKBTypes::Point25D, 0, 0, 1 )
                  << QgsPointV2( QgsWKBTypes::Point25D, 0, 10, 2 ) << QgsPointV2( QgsWKBTypes::Point25D, 10, 10, 3 )
                  << QgsPointV2( QgsWKBTypes::Point25D, 10, 0, 4 ) << QgsPointV2( QgsWKBTypes::Point25D, 0, 0, 1 ) );
  p6c.setExteriorRing( ext );
  QVERIFY( p6c.is3D() );
  QVERIFY( !p6c.isMeasure() );
  QCOMPARE( p6c.wkbType(), QgsWKBTypes::Polygon25D );
  //adding a LineStringZ, should become LineString25D
  ring = new QgsLineStringV2();
  ring->setPoints( QgsPointSequenceV2() << QgsPointV2( QgsWKBTypes::PointZ, 0.1, 0.1, 1 )
                   << QgsPointV2( QgsWKBTypes::PointZ, 0.1, 0.2, 2 ) << QgsPointV2( QgsWKBTypes::PointZ, 0.2, 0.2, 3 )
                   << QgsPointV2( QgsWKBTypes::PointZ, 0.2, 0.1, 4 ) << QgsPointV2( QgsWKBTypes::PointZ, 0.1, 0.1, 1 ) );
  QCOMPARE( ring->wkbType(), QgsWKBTypes::LineStringZ );
  p6c.addInteriorRing( ring );
  QVERIFY( p6c.interiorRing( 0 ) );
  QVERIFY( p6c.interiorRing( 0 )->is3D() );
  QVERIFY( !p6c.interiorRing( 0 )->isMeasure() );
  QCOMPARE( p6c.interiorRing( 0 )->wkbType(), QgsWKBTypes::LineString25D );
  QCOMPARE( p6c.interiorRing( 0 )->vertexAt( QgsVertexId( 0, 0, 0 ) ), QgsPointV2( QgsWKBTypes::Point25D, 0.1, 0.1, 1 ) );
  //add a LineStringM, should become LineString25D
  ring = new QgsLineStringV2();
  ring->setPoints( QgsPointSequenceV2() << QgsPointV2( QgsWKBTypes::PointM, 0.1, 0.1, 0, 1 )
                   << QgsPointV2( QgsWKBTypes::PointM, 0.1, 0.2, 0, 2 ) << QgsPointV2( QgsWKBTypes::PointM, 0.2, 0.2, 0, 3 )
                   << QgsPointV2( QgsWKBTypes::PointM, 0.2, 0.1, 0, 4 ) << QgsPointV2( QgsWKBTypes::PointM, 0.1, 0.1, 0, 1 ) );
  QCOMPARE( ring->wkbType(), QgsWKBTypes::LineStringM );
  p6c.addInteriorRing( ring );
  QVERIFY( p6c.interiorRing( 1 ) );
  QVERIFY( p6c.interiorRing( 1 )->is3D() );
  QVERIFY( !p6c.interiorRing( 1 )->isMeasure() );
  QCOMPARE( p6c.interiorRing( 1 )->wkbType(), QgsWKBTypes::LineString25D );
  QCOMPARE( p6c.interiorRing( 1 )->vertexAt( QgsVertexId( 0, 0, 0 ) ), QgsPointV2( QgsWKBTypes::Point25D, 0.1, 0.1, 0, 0 ) );

  //add curved ring to polygon
  circularRing = new QgsCircularStringV2();
  circularRing->setPoints( QgsPointSequenceV2() << QgsPointV2( 0, 0 ) << QgsPointV2( 0, 10 ) << QgsPointV2( 10, 10 )
                           << QgsPointV2( 10, 0 ) << QgsPointV2( 0, 0 ) );
  QVERIFY( circularRing->hasCurvedSegments() );
  p6c.addInteriorRing( circularRing );
  QVERIFY( p6c.interiorRing( 2 ) );
  QVERIFY( !p6c.interiorRing( 2 )->hasCurvedSegments() );
  QVERIFY( p6c.interiorRing( 2 )->is3D() );
  QVERIFY( !p6c.interiorRing( 2 )->isMeasure() );
  QCOMPARE( p6c.interiorRing( 2 )->wkbType(), QgsWKBTypes::LineString25D );

  //set interior rings
  QgsPolygonV2 p7;
  ext = new QgsLineStringV2();
  ext->setPoints( QgsPointSequenceV2() << QgsPointV2( 0, 0 ) << QgsPointV2( 0, 10 ) << QgsPointV2( 10, 10 )
                  << QgsPointV2( 10, 0 ) << QgsPointV2( 0, 0 ) );
  p7.setExteriorRing( ext );
  //add a list of rings with mixed types
  QList< QgsCurveV2* > rings;
  rings << new QgsLineStringV2();
  static_cast< QgsLineStringV2*>( rings[0] )->setPoints( QgsPointSequenceV2() << QgsPointV2( QgsWKBTypes::PointZ, 0.1, 0.1, 1 )
      << QgsPointV2( QgsWKBTypes::PointZ, 0.1, 0.2, 2 ) << QgsPointV2( QgsWKBTypes::PointZ, 0.2, 0.2, 3 )
      << QgsPointV2( QgsWKBTypes::PointZ, 0.2, 0.1, 4 ) << QgsPointV2( QgsWKBTypes::PointZ, 0.1, 0.1, 1 ) );
  rings << new QgsLineStringV2();
  static_cast< QgsLineStringV2*>( rings[1] )->setPoints( QgsPointSequenceV2() << QgsPointV2( QgsWKBTypes::PointM, 0.3, 0.3, 0, 1 )
      << QgsPointV2( QgsWKBTypes::PointM, 0.3, 0.4, 0, 2 ) << QgsPointV2( QgsWKBTypes::PointM, 0.4, 0.4, 0, 3 )
      << QgsPointV2( QgsWKBTypes::PointM, 0.4, 0.3, 0, 4 ) << QgsPointV2( QgsWKBTypes::PointM, 0.3, 0.3, 0, 1 ) );
  //throw an empty ring in too
  rings << 0;
  rings << new QgsCircularStringV2();
  static_cast< QgsCircularStringV2*>( rings[3] )->setPoints( QgsPointSequenceV2() << QgsPointV2( 0, 0 ) << QgsPointV2( 0, 10 ) << QgsPointV2( 10, 10 )
      << QgsPointV2( 10, 0 ) << QgsPointV2( 0, 0 ) );
  p7.setInteriorRings( rings );
  QCOMPARE( p7.numInteriorRings(), 3 );
  QVERIFY( p7.interiorRing( 0 ) );
  QVERIFY( !p7.interiorRing( 0 )->is3D() );
  QVERIFY( !p7.interiorRing( 0 )->isMeasure() );
  QCOMPARE( p7.interiorRing( 0 )->wkbType(), QgsWKBTypes::LineString );
  QCOMPARE( p7.interiorRing( 0 )->vertexAt( QgsVertexId( 0, 0, 0 ) ), QgsPointV2( QgsWKBTypes::Point, 0.1, 0.1 ) );
  QVERIFY( p7.interiorRing( 1 ) );
  QVERIFY( !p7.interiorRing( 1 )->is3D() );
  QVERIFY( !p7.interiorRing( 1 )->isMeasure() );
  QCOMPARE( p7.interiorRing( 1 )->wkbType(), QgsWKBTypes::LineString );
  QCOMPARE( p7.interiorRing( 1 )->vertexAt( QgsVertexId( 0, 0, 0 ) ), QgsPointV2( QgsWKBTypes::Point, 0.3, 0.3 ) );
  QVERIFY( p7.interiorRing( 2 ) );
  QVERIFY( !p7.interiorRing( 2 )->is3D() );
  QVERIFY( !p7.interiorRing( 2 )->isMeasure() );
  QCOMPARE( p7.interiorRing( 2 )->wkbType(), QgsWKBTypes::LineString );

  //set rings with existing
  rings.clear();
  rings << new QgsLineStringV2();
  static_cast< QgsLineStringV2*>( rings[0] )->setPoints( QgsPointSequenceV2() << QgsPointV2( 0.8, 0.8 )
      << QgsPointV2( 0.8, 0.9 ) << QgsPointV2( 0.9, 0.9 )
      << QgsPointV2( 0.9, 0.8 ) << QgsPointV2( 0.8, 0.8 ) );
  p7.setInteriorRings( rings );
  QCOMPARE( p7.numInteriorRings(), 1 );
  QVERIFY( p7.interiorRing( 0 ) );
  QVERIFY( !p7.interiorRing( 0 )->is3D() );
  QVERIFY( !p7.interiorRing( 0 )->isMeasure() );
  QCOMPARE( p7.interiorRing( 0 )->wkbType(), QgsWKBTypes::LineString );
  QCOMPARE( p7.interiorRing( 0 )->vertexAt( QgsVertexId( 0, 0, 0 ) ), QgsPointV2( QgsWKBTypes::Point, 0.8, 0.8 ) );
  rings.clear();
  p7.setInteriorRings( rings );
  QCOMPARE( p7.numInteriorRings(), 0 );

  //change dimensionality of interior rings using setExteriorRing
  QgsPolygonV2 p7a;
  ext = new QgsLineStringV2();
  ext->setPoints( QgsPointSequenceV2() << QgsPointV2( QgsWKBTypes::PointZ, 0, 0, 1 ) << QgsPointV2( QgsWKBTypes::PointZ, 0, 10, 2 )
                  << QgsPointV2( QgsWKBTypes::PointZ, 10, 10, 1 ) << QgsPointV2( QgsWKBTypes::PointZ, 10, 0, 3 ) << QgsPointV2( QgsWKBTypes::PointZ, 0, 0, 1 ) );
  p7a.setExteriorRing( ext );
  rings.clear();
  rings << new QgsLineStringV2();
  static_cast< QgsLineStringV2*>( rings[0] )->setPoints( QgsPointSequenceV2() << QgsPointV2( QgsWKBTypes::PointZ, 0.1, 0.1, 1 )
      << QgsPointV2( QgsWKBTypes::PointZ, 0.1, 0.2, 2 ) << QgsPointV2( QgsWKBTypes::PointZ, 0.2, 0.2, 3 )
      << QgsPointV2( QgsWKBTypes::PointZ, 0.2, 0.1, 4 ) << QgsPointV2( QgsWKBTypes::PointZ, 0.1, 0.1, 1 ) );
  rings << new QgsLineStringV2();
  static_cast< QgsLineStringV2*>( rings[1] )->setPoints( QgsPointSequenceV2() << QgsPointV2( QgsWKBTypes::PointZ, 0.3, 0.3, 1 )
      << QgsPointV2( QgsWKBTypes::PointZ, 0.3, 0.4, 2 ) << QgsPointV2( QgsWKBTypes::PointZ, 0.4, 0.4, 3 )
      << QgsPointV2( QgsWKBTypes::PointZ, 0.4, 0.3, 4 ) << QgsPointV2( QgsWKBTypes::PointZ, 0.3, 0.3,  1 ) );
  p7a.setInteriorRings( rings );
  QVERIFY( p7a.is3D() );
  QVERIFY( !p7a.isMeasure() );
  QVERIFY( p7a.interiorRing( 0 )->is3D() );
  QVERIFY( !p7a.interiorRing( 0 )->isMeasure() );
  QVERIFY( p7a.interiorRing( 1 )->is3D() );
  QVERIFY( !p7a.interiorRing( 1 )->isMeasure() );
  //reset exterior ring to 2d
  ext = new QgsLineStringV2();
  ext->setPoints( QgsPointSequenceV2() << QgsPointV2( 0, 0 ) << QgsPointV2( 0, 10 )
                  << QgsPointV2( 10, 10 ) << QgsPointV2( 10, 0 ) << QgsPointV2( 0, 0 ) );
  p7a.setExteriorRing( ext );
  QVERIFY( !p7a.is3D() );
  QVERIFY( !p7a.interiorRing( 0 )->is3D() ); //rings should also be made 2D
  QVERIFY( !p7a.interiorRing( 1 )->is3D() );
  //reset exterior ring to LineStringM
  ext = new QgsLineStringV2();
  ext->setPoints( QgsPointSequenceV2() << QgsPointV2( QgsWKBTypes::PointM, 0, 0 ) << QgsPointV2( QgsWKBTypes::PointM, 0, 10 )
                  << QgsPointV2( QgsWKBTypes::PointM, 10, 10 ) << QgsPointV2( QgsWKBTypes::PointM, 10, 0 ) << QgsPointV2( QgsWKBTypes::PointM, 0, 0 ) );
  p7a.setExteriorRing( ext );
  QVERIFY( p7a.isMeasure() );
  QVERIFY( p7a.interiorRing( 0 )->isMeasure() ); //rings should also gain measure
  QVERIFY( p7a.interiorRing( 1 )->isMeasure() );
  //25D exterior ring
  ext = new QgsLineStringV2();
  ext->setPoints( QgsPointSequenceV2() << QgsPointV2( QgsWKBTypes::Point25D, 0, 0 ) << QgsPointV2( QgsWKBTypes::Point25D, 0, 10 )
                  << QgsPointV2( QgsWKBTypes::Point25D, 10, 10 ) << QgsPointV2( QgsWKBTypes::Point25D, 10, 0 ) << QgsPointV2( QgsWKBTypes::Point25D, 0, 0 ) );
  p7a.setExteriorRing( ext );
  QVERIFY( p7a.is3D() );
  QVERIFY( !p7a.isMeasure() );
  QVERIFY( p7a.interiorRing( 0 )->is3D() ); //rings should also be made 25D
  QVERIFY( !p7a.interiorRing( 0 )->isMeasure() );
  QVERIFY( p7a.interiorRing( 1 )->is3D() );
  QVERIFY( !p7a.interiorRing( 1 )->isMeasure() );
  QCOMPARE( p7a.interiorRing( 0 )->wkbType(), QgsWKBTypes::LineString25D );
  QCOMPARE( p7a.interiorRing( 1 )->wkbType(), QgsWKBTypes::LineString25D );


  //removeInteriorRing
  QgsPolygonV2 p8;
  ext = new QgsLineStringV2();
  ext->setPoints( QgsPointSequenceV2() << QgsPointV2( 0, 0 ) << QgsPointV2( 0, 10 ) << QgsPointV2( 10, 10 )
                  << QgsPointV2( 10, 0 ) << QgsPointV2( 0, 0 ) );
  p8.setExteriorRing( ext );
  QVERIFY( !p8.removeInteriorRing( -1 ) );
  QVERIFY( !p8.removeInteriorRing( 0 ) );
  rings.clear();
  rings << new QgsLineStringV2();
  static_cast< QgsLineStringV2*>( rings[0] )->setPoints( QgsPointSequenceV2() << QgsPointV2( 0.1, 0.1 )
      << QgsPointV2( 0.1, 0.2 ) << QgsPointV2( 0.2, 0.2 )
      << QgsPointV2( 0.2, 0.1 ) << QgsPointV2( 0.1, 0.1 ) );
  rings << new QgsLineStringV2();
  static_cast< QgsLineStringV2*>( rings[1] )->setPoints( QgsPointSequenceV2() << QgsPointV2( 0.3, 0.3 )
      << QgsPointV2( 0.3, 0.4 ) << QgsPointV2( 0.4, 0.4 )
      << QgsPointV2( 0.4, 0.3 ) << QgsPointV2( 0.3, 0.3 ) );
  rings << new QgsLineStringV2();
  static_cast< QgsLineStringV2*>( rings[2] )->setPoints( QgsPointSequenceV2() << QgsPointV2( 0.8, 0.8 )
      << QgsPointV2( 0.8, 0.9 ) << QgsPointV2( 0.9, 0.9 )
      << QgsPointV2( 0.9, 0.8 ) << QgsPointV2( 0.8, 0.8 ) );
  p8.setInteriorRings( rings );
  QCOMPARE( p8.numInteriorRings(), 3 );
  QVERIFY( p8.removeInteriorRing( 0 ) );
  QCOMPARE( p8.numInteriorRings(), 2 );
  QCOMPARE( p8.interiorRing( 0 )->vertexAt( QgsVertexId( 0, 0, 0 ) ), QgsPointV2( 0.3, 0.3 ) );
  QCOMPARE( p8.interiorRing( 1 )->vertexAt( QgsVertexId( 0, 0, 0 ) ), QgsPointV2( 0.8, 0.8 ) );
  QVERIFY( p8.removeInteriorRing( 1 ) );
  QCOMPARE( p8.numInteriorRings(), 1 );
  QCOMPARE( p8.interiorRing( 0 )->vertexAt( QgsVertexId( 0, 0, 0 ) ), QgsPointV2( 0.3, 0.3 ) );
  QVERIFY( p8.removeInteriorRing( 0 ) );
  QCOMPARE( p8.numInteriorRings(), 0 );
  QVERIFY( !p8.removeInteriorRing( 0 ) );

  //clear
  QgsPolygonV2 p9;
  ext = new QgsLineStringV2();
  ext->setPoints( QgsPointSequenceV2() << QgsPointV2( QgsWKBTypes::PointZ, 0, 0, 1 )
                  << QgsPointV2( QgsWKBTypes::PointZ, 0, 10, 2 ) << QgsPointV2( QgsWKBTypes::PointZ, 10, 10, 3 )
                  << QgsPointV2( QgsWKBTypes::PointZ, 10, 0, 4 ) << QgsPointV2( QgsWKBTypes::PointZ, 0, 0, 1 ) );
  p9.setExteriorRing( ext );
  ring = new QgsLineStringV2();
  ring->setPoints( QgsPointSequenceV2() << QgsPointV2( QgsWKBTypes::PointZ, 1, 1, 1 )
                   << QgsPointV2( QgsWKBTypes::PointZ, 1, 9, 2 ) << QgsPointV2( QgsWKBTypes::PointZ, 9, 9, 3 )
                   << QgsPointV2( QgsWKBTypes::PointZ, 9, 1, 4 ) << QgsPointV2( QgsWKBTypes::PointZ, 1, 1, 1 ) );
  p9.addInteriorRing( ring );
  QCOMPARE( p9.numInteriorRings(), 1 );
  p9.clear();
  QVERIFY( p9.isEmpty() );
  QCOMPARE( p9.numInteriorRings(), 0 );
  QCOMPARE( p9.nCoordinates(), 0 );
  QCOMPARE( p9.ringCount(), 0 );
  QCOMPARE( p9.partCount(), 0 );
  QVERIFY( !p9.is3D() );
  QVERIFY( !p9.isMeasure() );
  QCOMPARE( p9.wkbType(), QgsWKBTypes::Polygon );

  //equality operator
  QgsPolygonV2 p10;
  QgsPolygonV2 p10b;
  QVERIFY( p10 == p10b );
  QVERIFY( !( p10 != p10b ) );
  ext = new QgsLineStringV2();
  ext->setPoints( QgsPointSequenceV2() << QgsPointV2( 0, 0 ) << QgsPointV2( 0, 10 ) << QgsPointV2( 10, 10 )
                  << QgsPointV2( 10, 0 ) << QgsPointV2( 0, 0 ) );
  p10.setExteriorRing( ext );
  QVERIFY( !( p10 == p10b ) );
  QVERIFY( p10 != p10b );
  ext = new QgsLineStringV2();
  ext->setPoints( QgsPointSequenceV2() << QgsPointV2( 0, 0 ) << QgsPointV2( 0, 10 ) << QgsPointV2( 10, 10 )
                  << QgsPointV2( 10, 0 ) << QgsPointV2( 0, 0 ) );
  p10b.setExteriorRing( ext );
  QVERIFY( p10 == p10b );
  QVERIFY( !( p10 != p10b ) );
  ext = new QgsLineStringV2();
  ext->setPoints( QgsPointSequenceV2() << QgsPointV2( 0, 0 ) << QgsPointV2( 0, 9 ) << QgsPointV2( 9, 9 )
                  << QgsPointV2( 9, 0 ) << QgsPointV2( 0, 0 ) );
  p10b.setExteriorRing( ext );
  QVERIFY( !( p10 == p10b ) );
  QVERIFY( p10 != p10b );
  ext = new QgsLineStringV2();
  ext->setPoints( QgsPointSequenceV2() << QgsPointV2( QgsWKBTypes::PointZ, 0, 0, 1 ) << QgsPointV2( QgsWKBTypes::PointZ, 0, 10, 2 )
                  << QgsPointV2( QgsWKBTypes::PointZ, 10, 10, 3 ) << QgsPointV2( QgsWKBTypes::PointZ, 10, 0, 4 ) << QgsPointV2( QgsWKBTypes::PointZ, 0, 0, 1 ) );
  p10b.setExteriorRing( ext );
  QVERIFY( !( p10 == p10b ) );
  QVERIFY( p10 != p10b );
  p10b.setExteriorRing( p10.exteriorRing()->clone() );
  QVERIFY( p10 == p10b );
  QVERIFY( !( p10 != p10b ) );
  ring = new QgsLineStringV2();
  ring->setPoints( QgsPointSequenceV2() << QgsPointV2( 1, 1 )
                   << QgsPointV2( 1, 9 ) << QgsPointV2( 9, 9 )
                   << QgsPointV2( 9, 1 ) << QgsPointV2( 1, 1 ) );
  p10.addInteriorRing( ring );
  QVERIFY( !( p10 == p10b ) );
  QVERIFY( p10 != p10b );
  ring = new QgsLineStringV2();
  ring->setPoints( QgsPointSequenceV2() << QgsPointV2( 2, 1 )
                   << QgsPointV2( 2, 9 ) << QgsPointV2( 9, 9 )
                   << QgsPointV2( 9, 1 ) << QgsPointV2( 2, 1 ) );
  p10b.addInteriorRing( ring );
  QVERIFY( !( p10 == p10b ) );
  QVERIFY( p10 != p10b );
  p10b.removeInteriorRing( 0 );
  p10b.addInteriorRing( p10.interiorRing( 0 )->clone() );
  QVERIFY( p10 == p10b );
  QVERIFY( !( p10 != p10b ) );

  //clone

  QgsPolygonV2 p11;
  QScopedPointer< QgsPolygonV2 >cloned( p11.clone() );
  QCOMPARE( p11, *cloned );
  ext = new QgsLineStringV2();
  ext->setPoints( QgsPointSequenceV2() << QgsPointV2( QgsWKBTypes::PointZM, 0, 0, 1, 5 )
                  << QgsPointV2( QgsWKBTypes::PointZM, 0, 10, 2, 6 ) << QgsPointV2( QgsWKBTypes::PointZM, 10, 10, 3, 7 )
                  << QgsPointV2( QgsWKBTypes::PointZM, 10, 0, 4, 8 ) << QgsPointV2( QgsWKBTypes::PointZM, 0, 0, 1, 9 ) );
  p11.setExteriorRing( ext );
  ring = new QgsLineStringV2();
  ring->setPoints( QgsPointSequenceV2() << QgsPointV2( QgsWKBTypes::PointZM, 1, 1, 1, 2 )
                   << QgsPointV2( QgsWKBTypes::PointZM, 1, 9, 2, 3 ) << QgsPointV2( QgsWKBTypes::PointZM, 9, 9, 3, 6 )
                   << QgsPointV2( QgsWKBTypes::PointZM, 9, 1, 4, 4 ) << QgsPointV2( QgsWKBTypes::PointZM, 1, 1, 1, 7 ) );
  p11.addInteriorRing( ring );
  cloned.reset( p11.clone() );
  QCOMPARE( p11, *cloned );

  //copy constructor
  QgsPolygonV2 p12;
  QgsPolygonV2 p13( p12 );
  QCOMPARE( p12, p13 );
  ext = new QgsLineStringV2();
  ext->setPoints( QgsPointSequenceV2() << QgsPointV2( QgsWKBTypes::PointZM, 0, 0, 1, 5 )
                  << QgsPointV2( QgsWKBTypes::PointZM, 0, 10, 2, 6 ) << QgsPointV2( QgsWKBTypes::PointZM, 10, 10, 3, 7 )
                  << QgsPointV2( QgsWKBTypes::PointZM, 10, 0, 4, 8 ) << QgsPointV2( QgsWKBTypes::PointZM, 0, 0, 1, 9 ) );
  p12.setExteriorRing( ext );
  ring = new QgsLineStringV2();
  ring->setPoints( QgsPointSequenceV2() << QgsPointV2( QgsWKBTypes::PointZM, 1, 1, 1, 2 )
                   << QgsPointV2( QgsWKBTypes::PointZM, 1, 9, 2, 3 ) << QgsPointV2( QgsWKBTypes::PointZM, 9, 9, 3, 6 )
                   << QgsPointV2( QgsWKBTypes::PointZM, 9, 1, 4, 4 ) << QgsPointV2( QgsWKBTypes::PointZM, 1, 1, 1, 7 ) );
  p12.addInteriorRing( ring );
  QgsPolygonV2 p14( p12 );
  QCOMPARE( p12, p14 );

  //assignment operator
  QgsPolygonV2 p15;
  p15 = p13;
  QCOMPARE( p13, p15 );
  p15 = p12;
  QCOMPARE( p12, p15 );

  //surfaceToPolygon - should be identical given polygon has no curves
  QScopedPointer< QgsPolygonV2 > surface( p12.surfaceToPolygon() );
  QCOMPARE( *surface, p12 );

  //to/fromWKB
  QgsPolygonV2 p16;
  ext = new QgsLineStringV2();
  ext->setPoints( QgsPointSequenceV2() << QgsPointV2( QgsWKBTypes::Point, 0, 0 )
                  << QgsPointV2( QgsWKBTypes::Point, 0, 10 ) << QgsPointV2( QgsWKBTypes::Point, 10, 10 )
                  << QgsPointV2( QgsWKBTypes::Point, 10, 0 ) << QgsPointV2( QgsWKBTypes::Point, 0, 0 ) );
  p16.setExteriorRing( ext );
  ring = new QgsLineStringV2();
  ring->setPoints( QgsPointSequenceV2() << QgsPointV2( QgsWKBTypes::Point, 1, 1 )
                   << QgsPointV2( QgsWKBTypes::Point, 1, 9 ) << QgsPointV2( QgsWKBTypes::Point, 9, 9 )
                   << QgsPointV2( QgsWKBTypes::Point, 9, 1 ) << QgsPointV2( QgsWKBTypes::Point, 1, 1 ) );
  p16.addInteriorRing( ring );
  int size = 0;
  unsigned char* wkb = p16.asWkb( size );
  QCOMPARE( size, p16.wkbSize() );
  QgsPolygonV2 p17;
  p17.fromWkb( QgsConstWkbPtr( wkb, size ) );
  delete[] wkb;
  wkb = 0;
  QCOMPARE( p16, p17 );
  //PolygonZ
  p16.clear();
  p17.clear();
  ext = new QgsLineStringV2();
  ext->setPoints( QgsPointSequenceV2() << QgsPointV2( QgsWKBTypes::PointZ, 0, 0, 1 )
                  << QgsPointV2( QgsWKBTypes::PointZ, 0, 10, 2 ) << QgsPointV2( QgsWKBTypes::PointZ, 10, 10, 3 )
                  << QgsPointV2( QgsWKBTypes::PointZ, 10, 0, 4 ) << QgsPointV2( QgsWKBTypes::PointZ, 0, 0, 1 ) );
  p16.setExteriorRing( ext );
  ring = new QgsLineStringV2();
  ring->setPoints( QgsPointSequenceV2() << QgsPointV2( QgsWKBTypes::PointZ, 1, 1, 1 )
                   << QgsPointV2( QgsWKBTypes::PointZ, 1, 9, 2 ) << QgsPointV2( QgsWKBTypes::PointZ, 9, 9, 3 )
                   << QgsPointV2( QgsWKBTypes::PointZ, 9, 1, 4 ) << QgsPointV2( QgsWKBTypes::PointZ, 1, 1, 1 ) );
  p16.addInteriorRing( ring );
  size = 0;
  wkb = p16.asWkb( size );
  QCOMPARE( size, p16.wkbSize() );
  p17.fromWkb( QgsConstWkbPtr( wkb, size ) );
  delete[] wkb;
  wkb = 0;
  QCOMPARE( p16, p17 );
  //PolygonM
  p16.clear();
  p17.clear();
  ext = new QgsLineStringV2();
  ext->setPoints( QgsPointSequenceV2() << QgsPointV2( QgsWKBTypes::PointM, 0, 0, 0, 1 )
                  << QgsPointV2( QgsWKBTypes::PointM, 0, 10,  0, 2 ) << QgsPointV2( QgsWKBTypes::PointM, 10, 10, 0, 3 )
                  << QgsPointV2( QgsWKBTypes::PointM, 10, 0,  0, 4 ) << QgsPointV2( QgsWKBTypes::PointM, 0, 0, 0, 1 ) );
  p16.setExteriorRing( ext );
  ring = new QgsLineStringV2();
  ring->setPoints( QgsPointSequenceV2() << QgsPointV2( QgsWKBTypes::PointM, 1, 1, 0, 1 )
                   << QgsPointV2( QgsWKBTypes::PointM, 1, 9, 0, 2 ) << QgsPointV2( QgsWKBTypes::PointM, 9, 9, 0, 3 )
                   << QgsPointV2( QgsWKBTypes::PointM, 9, 1, 0, 4 ) << QgsPointV2( QgsWKBTypes::PointM, 1, 1, 0, 1 ) );
  p16.addInteriorRing( ring );
  size = 0;
  wkb = p16.asWkb( size );
  QCOMPARE( size, p16.wkbSize() );
  p17.fromWkb( QgsConstWkbPtr( wkb, size ) );
  delete[] wkb;
  wkb = 0;
  QCOMPARE( p16, p17 );
  //PolygonZM
  p16.clear();
  p17.clear();
  ext = new QgsLineStringV2();
  ext->setPoints( QgsPointSequenceV2() << QgsPointV2( QgsWKBTypes::PointZM, 0, 0, 1, 5 )
                  << QgsPointV2( QgsWKBTypes::PointZM, 0, 10, 2, 6 ) << QgsPointV2( QgsWKBTypes::PointZM, 10, 10, 3, 7 )
                  << QgsPointV2( QgsWKBTypes::PointZM, 10, 0, 4, 8 ) << QgsPointV2( QgsWKBTypes::PointZM, 0, 0, 1, 9 ) );
  p16.setExteriorRing( ext );
  ring = new QgsLineStringV2();
  ring->setPoints( QgsPointSequenceV2() << QgsPointV2( QgsWKBTypes::PointZM, 1, 1, 1, 2 )
                   << QgsPointV2( QgsWKBTypes::PointZM, 1, 9, 2, 3 ) << QgsPointV2( QgsWKBTypes::PointZM, 9, 9, 3, 6 )
                   << QgsPointV2( QgsWKBTypes::PointZM, 9, 1, 4, 4 ) << QgsPointV2( QgsWKBTypes::PointZM, 1, 1, 1, 7 ) );
  p16.addInteriorRing( ring );
  size = 0;
  wkb = p16.asWkb( size );
  QCOMPARE( size, p16.wkbSize() );
  p17.fromWkb( QgsConstWkbPtr( wkb, size ) );
  delete[] wkb;
  wkb = 0;
  QCOMPARE( p16, p17 );
  //Polygon25D
  p16.clear();
  p17.clear();
  ext = new QgsLineStringV2();
  ext->setPoints( QgsPointSequenceV2() << QgsPointV2( QgsWKBTypes::Point25D, 0, 0, 1 )
                  << QgsPointV2( QgsWKBTypes::Point25D, 0, 10, 2 ) << QgsPointV2( QgsWKBTypes::Point25D, 10, 10, 3 )
                  << QgsPointV2( QgsWKBTypes::Point25D, 10, 0, 4 ) << QgsPointV2( QgsWKBTypes::Point25D, 0, 0, 1 ) );
  p16.setExteriorRing( ext );
  ring = new QgsLineStringV2();
  ring->setPoints( QgsPointSequenceV2() << QgsPointV2( QgsWKBTypes::Point25D, 1, 1, 1 )
                   << QgsPointV2( QgsWKBTypes::Point25D, 1, 9, 2 ) << QgsPointV2( QgsWKBTypes::Point25D, 9, 9, 3 )
                   << QgsPointV2( QgsWKBTypes::Point25D, 9, 1, 4 ) << QgsPointV2( QgsWKBTypes::Point25D, 1, 1, 1 ) );
  p16.addInteriorRing( ring );
  size = 0;
  wkb = p16.asWkb( size );
  QCOMPARE( size, p16.wkbSize() );
  p17.clear();
  p17.fromWkb( QgsConstWkbPtr( wkb, size ) );
  delete[] wkb;
  wkb = 0;
  QCOMPARE( p16, p17 );

  //bad WKB - check for no crash
  p17.clear();
  QVERIFY( !p17.fromWkb( QgsConstWkbPtr( nullptr, 0 ) ) );
  QCOMPARE( p17.wkbType(), QgsWKBTypes::Polygon );
  QgsPointV2 point( 1, 2 );
  wkb = point.asWkb( size ) ;
  QVERIFY( !p17.fromWkb( QgsConstWkbPtr( wkb, size ) ) );
  delete[] wkb;
  wkb = 0;
  QCOMPARE( p17.wkbType(), QgsWKBTypes::Polygon );

  //to/from WKT
  QgsPolygonV2 p18;
  ext = new QgsLineStringV2();
  ext->setPoints( QgsPointSequenceV2() << QgsPointV2( QgsWKBTypes::PointZM, 0, 0, 1, 5 )
                  << QgsPointV2( QgsWKBTypes::PointZM, 0, 10, 2, 6 ) << QgsPointV2( QgsWKBTypes::PointZM, 10, 10, 3, 7 )
                  << QgsPointV2( QgsWKBTypes::PointZM, 10, 0, 4, 8 ) << QgsPointV2( QgsWKBTypes::PointZM, 0, 0, 1, 9 ) );
  p18.setExteriorRing( ext );
  ring = new QgsLineStringV2();
  ring->setPoints( QgsPointSequenceV2() << QgsPointV2( QgsWKBTypes::PointZM, 1, 1, 1, 2 )
                   << QgsPointV2( QgsWKBTypes::PointZM, 1, 9, 2, 3 ) << QgsPointV2( QgsWKBTypes::PointZM, 9, 9, 3, 6 )
                   << QgsPointV2( QgsWKBTypes::PointZM, 9, 1, 4, 4 ) << QgsPointV2( QgsWKBTypes::PointZM, 1, 1, 1, 7 ) );
  p18.addInteriorRing( ring );

  QString wkt = p18.asWkt();
  QVERIFY( !wkt.isEmpty() );
  QgsPolygonV2 p19;
  QVERIFY( p19.fromWkt( wkt ) );
  QCOMPARE( p18, p19 );

  //bad WKT
  QVERIFY( !p19.fromWkt( "Point()" ) );
  QVERIFY( p19.isEmpty() );
  QVERIFY( !p19.exteriorRing() );
  QCOMPARE( p19.numInteriorRings(), 0 );
  QVERIFY( !p19.is3D() );
  QVERIFY( !p19.isMeasure() );
  QCOMPARE( p19.wkbType(), QgsWKBTypes::Polygon );

  //as JSON
  QgsPolygonV2 exportPolygon;
  ext = new QgsLineStringV2();
  ext->setPoints( QgsPointSequenceV2() << QgsPointV2( QgsWKBTypes::Point, 0, 0 )
                  << QgsPointV2( QgsWKBTypes::Point, 0, 10 ) << QgsPointV2( QgsWKBTypes::Point, 10, 10 )
                  << QgsPointV2( QgsWKBTypes::Point, 10, 0 ) << QgsPointV2( QgsWKBTypes::Point, 0, 0 ) );
  exportPolygon.setExteriorRing( ext );
  ring = new QgsLineStringV2();
  ring->setPoints( QgsPointSequenceV2() << QgsPointV2( QgsWKBTypes::Point, 1, 1 )
                   << QgsPointV2( QgsWKBTypes::Point, 1, 9 ) << QgsPointV2( QgsWKBTypes::Point, 9, 9 )
                   << QgsPointV2( QgsWKBTypes::Point, 9, 1 ) << QgsPointV2( QgsWKBTypes::Point, 1, 1 ) );
  exportPolygon.addInteriorRing( ring );

  QString expectedJson( "{\"type\": \"Polygon\", \"coordinates\": [[ [0, 0], [0, 10], [10, 10], [10, 0], [0, 0]], [ [1, 1], [1, 9], [9, 9], [9, 1], [1, 1]]] }" );
  QCOMPARE( exportPolygon.asJSON(), expectedJson );

  QgsPolygonV2 exportPolygonFloat;
  ext = new QgsLineStringV2();
  ext->setPoints( QgsPointSequenceV2() << QgsPointV2( QgsWKBTypes::Point, 10 / 9.0, 10 / 9.0 )
                  << QgsPointV2( QgsWKBTypes::Point, 10 / 9.0, 100 / 9.0 ) << QgsPointV2( QgsWKBTypes::Point, 100 / 9.0, 100 / 9.0 )
                  << QgsPointV2( QgsWKBTypes::Point, 100 / 9.0, 10 / 9.0 ) << QgsPointV2( QgsWKBTypes::Point, 10 / 9.0, 10 / 9.0 ) );
  exportPolygonFloat.setExteriorRing( ext );
  ring = new QgsLineStringV2();
  ring->setPoints( QgsPointSequenceV2() << QgsPointV2( QgsWKBTypes::Point, 2 / 3.0, 2 / 3.0 )
                   << QgsPointV2( QgsWKBTypes::Point, 2 / 3.0, 4 / 3.0 ) << QgsPointV2( QgsWKBTypes::Point, 4 / 3.0, 4 / 3.0 )
                   << QgsPointV2( QgsWKBTypes::Point, 4 / 3.0, 2 / 3.0 ) << QgsPointV2( QgsWKBTypes::Point, 2 / 3.0, 2 / 3.0 ) );
  exportPolygonFloat.addInteriorRing( ring );

  QString expectedJsonPrec3( "{\"type\": \"Polygon\", \"coordinates\": [[ [1.111, 1.111], [1.111, 11.111], [11.111, 11.111], [11.111, 1.111], [1.111, 1.111]], [ [0.667, 0.667], [0.667, 1.333], [1.333, 1.333], [1.333, 0.667], [0.667, 0.667]]] }" );
  QCOMPARE( exportPolygonFloat.asJSON( 3 ), expectedJsonPrec3 );

  // as GML2
  QDomDocument doc( "gml" );
  QString expectedGML2( "<Polygon xmlns=\"gml\"><outerBoundaryIs xmlns=\"gml\"><LinearRing xmlns=\"gml\"><coordinates xmlns=\"gml\">0,0 0,10 10,10 10,0 0,0</coordinates></LinearRing></outerBoundaryIs>" );
  expectedGML2 += QString( "<innerBoundaryIs xmlns=\"gml\"><LinearRing xmlns=\"gml\"><coordinates xmlns=\"gml\">1,1 1,9 9,9 9,1 1,1</coordinates></LinearRing></innerBoundaryIs></Polygon>" );
  QCOMPARE( elemToString( exportPolygon.asGML2( doc ) ), expectedGML2 );
  QString expectedGML2prec3( "<Polygon xmlns=\"gml\"><outerBoundaryIs xmlns=\"gml\"><LinearRing xmlns=\"gml\"><coordinates xmlns=\"gml\">1.111,1.111 1.111,11.111 11.111,11.111 11.111,1.111 1.111,1.111</coordinates></LinearRing></outerBoundaryIs>" );
  expectedGML2prec3 += QString( "<innerBoundaryIs xmlns=\"gml\"><LinearRing xmlns=\"gml\"><coordinates xmlns=\"gml\">0.667,0.667 0.667,1.333 1.333,1.333 1.333,0.667 0.667,0.667</coordinates></LinearRing></innerBoundaryIs></Polygon>" );
  QCOMPARE( elemToString( exportPolygonFloat.asGML2( doc, 3 ) ), expectedGML2prec3 );

  //as GML3
  QString expectedGML3( "<Polygon xmlns=\"gml\"><exterior xmlns=\"gml\"><LinearRing xmlns=\"gml\"><coordinates xmlns=\"gml\">0,0 0,10 10,10 10,0 0,0</coordinates></LinearRing></exterior>" );
  expectedGML3 += QString( "<interior xmlns=\"gml\"><LinearRing xmlns=\"gml\"><coordinates xmlns=\"gml\">1,1 1,9 9,9 9,1 1,1</coordinates></LinearRing></interior></Polygon>" );
  QCOMPARE( elemToString( exportPolygon.asGML3( doc ) ), expectedGML3 );
  QString expectedGML3prec3( "<Polygon xmlns=\"gml\"><exterior xmlns=\"gml\"><LinearRing xmlns=\"gml\"><coordinates xmlns=\"gml\">1.111,1.111 1.111,11.111 11.111,11.111 11.111,1.111 1.111,1.111</coordinates></LinearRing></exterior>" );
  expectedGML3prec3 += QString( "<interior xmlns=\"gml\"><LinearRing xmlns=\"gml\"><coordinates xmlns=\"gml\">0.667,0.667 0.667,1.333 1.333,1.333 1.333,0.667 0.667,0.667</coordinates></LinearRing></interior></Polygon>" );
  QCOMPARE( elemToString( exportPolygonFloat.asGML3( doc, 3 ) ), expectedGML3prec3 );

  //removing the fourth to last vertex removes the whole ring
  QgsPolygonV2 p20;
  QgsLineStringV2* p20ExteriorRing = new QgsLineStringV2();
  p20ExteriorRing->setPoints( QList<QgsPointV2>() << QgsPointV2( 0, 0 ) << QgsPointV2( 1, 0 ) << QgsPointV2( 1, 1 ) << QgsPointV2( 0, 0 ) );
  p20.setExteriorRing( p20ExteriorRing );
  QVERIFY( p20.exteriorRing() );
  p20.deleteVertex( QgsVertexId( 0, 0, 2 ) );
  QVERIFY( !p20.exteriorRing() );
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
  expectedMultiPoly
  << ( QgsPolygon() << ( QgsPolyline() << QgsPoint( 1.0, 0 ) << QgsPoint( 9, 0 ) << QgsPoint( 10.0, 1 )
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

  QCOMPARE( geom->geometry()->asWkt(), resultGeometry.geometry()->asWkt() );

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

  // no geometry
  QgsGeometry nullGeom( nullptr );
  obtained = nullGeom.exportToGeoJSON();
  geojson = "null";
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

void TestQgsGeometry::wkbInOut()
{
  // Premature end of WKB
  // See http://hub.qgis.org/issues/14182
  const char *hexwkb = "0102000000EF0000000000000000000000000000000000000000000000000000000000000000000000";
  int size;
  unsigned char *wkb = hex2bytes( hexwkb, &size );
  QgsGeometry g14182;
  // NOTE: wkb onwership transferred to QgsGeometry
  g14182.fromWkb( wkb, size );
  //QList<QgsGeometry::Error> errors;
  //g14182.validateGeometry(errors);
  // Check with valgrind !
  QString wkt = g14182.exportToWkt();
  QCOMPARE( wkt, QString() );

  //WKB with a truncated header
  const char *badHeaderHexwkb = "0102";
  wkb = hex2bytes( badHeaderHexwkb, &size );
  QgsGeometry badHeader;
  // NOTE: wkb onwership transferred to QgsGeometry
  badHeader.fromWkb( wkb, size );
  QVERIFY( badHeader.isEmpty() );
  QCOMPARE( badHeader.wkbType(), QGis::WKBUnknown );
}

void TestQgsGeometry::segmentizeCircularString()
{
  QString wkt( "CIRCULARSTRING( 0 0, 0.5 0.5, 2 0 )" );
  QgsCircularStringV2* circularString = dynamic_cast<QgsCircularStringV2*>( QgsGeometryFactory::geomFromWkt( wkt ) );
  QVERIFY( circularString );
  QgsLineStringV2* lineString = circularString->curveToLine();
  QVERIFY( lineString );
  QgsPointSequenceV2 points;
  lineString->points( points );

  delete circularString;
  delete lineString;

  //make sure the curve point is part of the segmentized result
  QVERIFY( points.contains( QgsPointV2( 0.5, 0.5 ) ) );
}

QTEST_MAIN( TestQgsGeometry )
#include "testqgsgeometry.moc"
