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
#include "qgstest.h"
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
#include "qgscompoundcurve.h"
#include <qgsgeometry.h>
#include "qgsgeometryutils.h"
#include <qgspoint.h>
#include "qgspoint.h"
#include "qgslinestring.h"
#include "qgspolygon.h"
#include "qgstriangle.h"
#include "qgscircle.h"
#include "qgsellipse.h"
#include "qgsregularpolygon.h"
#include "qgsmultipoint.h"
#include "qgsmultilinestring.h"
#include "qgsmultipolygon.h"
#include "qgscircularstring.h"
#include "qgsgeometrycollection.h"
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
    void operatorBool();

    // geometry types
    void point(); //test QgsPointV2
    void lineString(); //test QgsLineString
    void polygon(); //test QgsPolygonV2
    void triangle();
    void circle();
    void ellipse();
    void regularPolygon();
    void compoundCurve(); //test QgsCompoundCurve
    void multiPoint();
    void multiLineString();
    void multiPolygon();
    void geometryCollection();

    void fromQgsPointXY();
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

    void unaryUnion();

    void dataStream();

    void exportToGeoJSON();

    void wkbInOut();

    void segmentizeCircularString();
    void directionNeutralSegmentation();
    void poleOfInaccessibility();

    void makeValid();

    void isSimple();

    void reshapeGeometryLineMerge();
    void createCollectionOfType();

  private:
    //! A helper method to do a render check to see if the geometry op is as expected
    bool renderCheck( const QString &testName, const QString &comment = QLatin1String( QLatin1String( "" ) ), int mismatchCount = 0 );
    //! A helper method to dump to qdebug the geometry of a multipolygon
    void dumpMultiPolygon( QgsMultiPolygon &multiPolygon );
    //! A helper method to dump to qdebug the geometry of a polygon
    void dumpPolygon( QgsPolygon &polygon );
    //! A helper method to dump to qdebug the geometry of a polyline
    void dumpPolyline( QgsPolyline &polyline );

    // Release return with delete []
    unsigned char *hex2bytes( const char *hex, int *size )
    {
      QByteArray ba = QByteArray::fromHex( hex );
      unsigned char *out = new unsigned char[ba.size()];
      memcpy( out, ba.data(), ba.size() );
      *size = ba.size();
      return out;
    }

    QString bytes2hex( const unsigned char *bytes, int size )
    {
      QByteArray ba( ( const char * )bytes, size );
      QString out = ba.toHex();
      return out;
    }


    QString elemToString( const QDomElement &elem ) const;

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
    QgsPolyline mPolylineA;
    QgsPolyline mPolylineB;
    QgsPolyline mPolylineC;
    QgsGeometry mpPolylineGeometryD;
    QgsPolygon mPolygonA;
    QgsPolygon mPolygonB;
    QgsPolygon mPolygonC;
    QgsGeometry mpPolygonGeometryA;
    QgsGeometry mpPolygonGeometryB;
    QgsGeometry mpPolygonGeometryC;
    QString mWktLine;
    QString mTestDataDir;
    QImage mImage;
    QPainter *mpPainter = nullptr;
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
  mReport += QLatin1String( "<h1>Geometry Tests</h1>\n" );
  mReport += QLatin1String( "<p><font color=\"green\">Green = polygonA</font></p>\n" );
  mReport += QLatin1String( "<p><font color=\"red\">Red = polygonB</font></p>\n" );
  mReport += QLatin1String( "<p><font color=\"blue\">Blue = polygonC</font></p>\n" );
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

  mWktLine = QStringLiteral( "LINESTRING(117.623198 35.198654, 117.581274 35.198654, 117.078178 35.324427, 116.868555 35.534051, 116.617007 35.869448, 116.491233 35.953297, 116.155836 36.288694, 116.071987 36.372544, 115.443117 36.749865, 114.814247 37.043338, 114.311152 37.169112, 113.388810 37.378735, 113.095337 37.378735, 112.592241 37.378735, 111.753748 37.294886, 111.502201 37.252961, 111.082954 37.127187, 110.747557 37.127187, 110.160612 36.917564, 110.034838 36.833715, 109.741366 36.749865, 109.573667 36.666016, 109.238270 36.498317, 109.070571 36.414468, 108.819023 36.288694, 108.693250 36.246770, 108.483626 36.162920, 107.645134 35.911372, 106.597017 35.869448, 106.051997 35.701749, 105.800449 35.617900, 105.590826 35.575975, 105.297354 35.575975, 104.961956 35.575975, 104.710409 35.534051, 104.458861 35.492126, 103.871916 35.492126, 103.788066 35.492126, 103.326895 35.408277, 102.949574 35.408277, 102.488402 35.450201, 102.069156 35.450201, 101.482211 35.450201, 100.937191 35.659825, 100.308321 35.869448, 100.056773 36.037146, 99.050582 36.079071, 97.667069 35.743674, 97.163973 35.617900, 96.115857 35.534051, 95.612761 35.534051, 94.396947 35.911372, 93.684228 36.288694, 92.929584 36.833715, 92.258790 37.169112, 91.629920 37.504509, 90.414105 37.881831, 90.414105 37.881831, 90.246407 37.923755, 89.491763 37.839906, 89.156366 37.672207, 88.485572 37.504509, 87.814778 37.252961, 87.563230 37.169112, 87.143983 37.043338, 85.970093 36.875639, 85.802395 36.875639, 84.083484 36.959489, 84.041560 37.043338, 82.951519 37.546433, 82.699971 37.630283)" );

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
  delete mpPainter;
}

void TestQgsGeometry::copy()
{
  //create a point geometry
  QgsGeometry original( new QgsPoint( 1.0, 2.0 ) );
  QCOMPARE( original.geometry()->vertexAt( QgsVertexId( 0, 0, 0 ) ).x(), 1.0 );
  QCOMPARE( original.geometry()->vertexAt( QgsVertexId( 0, 0, 0 ) ).y(), 2.0 );

  //implicitly shared copy
  QgsGeometry copy( original );
  QCOMPARE( copy.geometry()->vertexAt( QgsVertexId( 0, 0, 0 ) ).x(), 1.0 );
  QCOMPARE( copy.geometry()->vertexAt( QgsVertexId( 0, 0, 0 ) ).y(), 2.0 );

  //trigger a detach
  copy.setGeometry( new QgsPoint( 3.0, 4.0 ) );
  QCOMPARE( copy.geometry()->vertexAt( QgsVertexId( 0, 0, 0 ) ).x(), 3.0 );
  QCOMPARE( copy.geometry()->vertexAt( QgsVertexId( 0, 0, 0 ) ).y(), 4.0 );

  //make sure original was untouched
  QCOMPARE( original.geometry()->vertexAt( QgsVertexId( 0, 0, 0 ) ).x(), 1.0 );
  QCOMPARE( original.geometry()->vertexAt( QgsVertexId( 0, 0, 0 ) ).y(), 2.0 );
}

void TestQgsGeometry::assignment()
{
  //create a point geometry
  QgsGeometry original( new QgsPoint( 1.0, 2.0 ) );
  QCOMPARE( original.geometry()->vertexAt( QgsVertexId( 0, 0, 0 ) ).x(), 1.0 );
  QCOMPARE( original.geometry()->vertexAt( QgsVertexId( 0, 0, 0 ) ).y(), 2.0 );

  //assign to implicitly shared copy
  QgsGeometry copy;
  copy = original;
  QCOMPARE( copy.geometry()->vertexAt( QgsVertexId( 0, 0, 0 ) ).x(), 1.0 );
  QCOMPARE( copy.geometry()->vertexAt( QgsVertexId( 0, 0, 0 ) ).y(), 2.0 );

  //trigger a detach
  copy.setGeometry( new QgsPoint( 3.0, 4.0 ) );
  QCOMPARE( copy.geometry()->vertexAt( QgsVertexId( 0, 0, 0 ) ).x(), 3.0 );
  QCOMPARE( copy.geometry()->vertexAt( QgsVertexId( 0, 0, 0 ) ).y(), 4.0 );

  //make sure original was untouched
  QCOMPARE( original.geometry()->vertexAt( QgsVertexId( 0, 0, 0 ) ).x(), 1.0 );
  QCOMPARE( original.geometry()->vertexAt( QgsVertexId( 0, 0, 0 ) ).y(), 2.0 );
}

void TestQgsGeometry::asVariant()
{
  //create a point geometry
  QgsGeometry original( new QgsPoint( 1.0, 2.0 ) );
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
  original.setGeometry( new QgsPoint( 3.0, 4.0 ) );
  QgsGeometry fromVar3 = qvariant_cast<QgsGeometry>( var );
  QCOMPARE( fromVar3.geometry()->vertexAt( QgsVertexId( 0, 0, 0 ) ).x(), 1.0 );
  QCOMPARE( fromVar3.geometry()->vertexAt( QgsVertexId( 0, 0, 0 ) ).y(), 2.0 );
}

void TestQgsGeometry::isEmpty()
{
  QgsGeometry geom;
  QVERIFY( geom.isNull() );

  geom.setGeometry( new QgsPoint( 1.0, 2.0 ) );
  QVERIFY( !geom.isNull() );

  geom.setGeometry( 0 );
  QVERIFY( geom.isNull() );

  QgsGeometryCollection collection;
  QVERIFY( collection.isEmpty() );
}

void TestQgsGeometry::operatorBool()
{
  QgsGeometry geom;
  QVERIFY( !geom );

  geom.setGeometry( new QgsPoint( 1.0, 2.0 ) );
  QVERIFY( geom );

  geom.setGeometry( 0 );
  QVERIFY( !geom );
}

void TestQgsGeometry::point()
{
  //test QgsPointV2

  //test constructors
  QgsPoint p1( 5.0, 6.0 );
  QCOMPARE( p1.x(), 5.0 );
  QCOMPARE( p1.y(), 6.0 );
  QVERIFY( !p1.isEmpty() );
  QVERIFY( !p1.is3D() );
  QVERIFY( !p1.isMeasure() );
  QCOMPARE( p1.wkbType(), QgsWkbTypes::Point );
  QCOMPARE( p1.wktTypeStr(), QString( "Point" ) );

  QgsPoint p2( QgsPointXY( 3.0, 4.0 ) );
  QCOMPARE( p2.x(), 3.0 );
  QCOMPARE( p2.y(), 4.0 );
  QVERIFY( !p2.isEmpty() );
  QVERIFY( !p2.is3D() );
  QVERIFY( !p2.isMeasure() );
  QCOMPARE( p2.wkbType(), QgsWkbTypes::Point );

  QgsPoint p3( QPointF( 7.0, 9.0 ) );
  QCOMPARE( p3.x(), 7.0 );
  QCOMPARE( p3.y(), 9.0 );
  QVERIFY( !p3.isEmpty() );
  QVERIFY( !p3.is3D() );
  QVERIFY( !p3.isMeasure() );
  QCOMPARE( p3.wkbType(), QgsWkbTypes::Point );

  QgsPoint p4( QgsWkbTypes::Point, 11.0, 13.0 );
  QCOMPARE( p4.x(), 11.0 );
  QCOMPARE( p4.y(), 13.0 );
  QVERIFY( !p4.isEmpty() );
  QVERIFY( !p4.is3D() );
  QVERIFY( !p4.isMeasure() );
  QCOMPARE( p4.wkbType(), QgsWkbTypes::Point );

  QgsPoint p5( QgsWkbTypes::PointZ, 11.0, 13.0, 15.0 );
  QCOMPARE( p5.x(), 11.0 );
  QCOMPARE( p5.y(), 13.0 );
  QCOMPARE( p5.z(), 15.0 );
  QVERIFY( !p5.isEmpty() );
  QVERIFY( p5.is3D() );
  QVERIFY( !p5.isMeasure() );
  QCOMPARE( p5.wkbType(), QgsWkbTypes::PointZ );
  QCOMPARE( p5.wktTypeStr(), QString( "PointZ" ) );

  QgsPoint p6( QgsWkbTypes::PointM, 11.0, 13.0, 0.0, 17.0 );
  QCOMPARE( p6.x(), 11.0 );
  QCOMPARE( p6.y(), 13.0 );
  QCOMPARE( p6.m(), 17.0 );
  QVERIFY( !p6.isEmpty() );
  QVERIFY( !p6.is3D() );
  QVERIFY( p6.isMeasure() );
  QCOMPARE( p6.wkbType(), QgsWkbTypes::PointM );
  QCOMPARE( p6.wktTypeStr(), QString( "PointM" ) );

  QgsPoint p7( QgsWkbTypes::PointZM, 11.0, 13.0, 0.0, 17.0 );
  QCOMPARE( p7.x(), 11.0 );
  QCOMPARE( p7.y(), 13.0 );
  QCOMPARE( p7.m(), 17.0 );
  QVERIFY( !p7.isEmpty() );
  QVERIFY( p7.is3D() );
  QVERIFY( p7.isMeasure() );
  QCOMPARE( p7.wkbType(), QgsWkbTypes::PointZM );
  QCOMPARE( p7.wktTypeStr(), QString( "PointZM" ) );

  QgsPoint p8( QgsWkbTypes::Point25D, 21.0, 23.0, 25.0 );
  QCOMPARE( p8.x(), 21.0 );
  QCOMPARE( p8.y(), 23.0 );
  QCOMPARE( p8.z(), 25.0 );
  QVERIFY( !p8.isEmpty() );
  QVERIFY( p8.is3D() );
  QVERIFY( !p8.isMeasure() );
  QCOMPARE( p8.wkbType(), QgsWkbTypes::Point25D );

  QgsPoint pp( QgsWkbTypes::Point );
  QVERIFY( !pp.is3D() );
  QVERIFY( !pp.isMeasure() );

  QgsPoint ppz( QgsWkbTypes::PointZ );
  QVERIFY( ppz.is3D() );
  QVERIFY( !ppz.isMeasure() );

  QgsPoint ppm( QgsWkbTypes::PointM );
  QVERIFY( !ppm.is3D() );
  QVERIFY( ppm.isMeasure() );

  QgsPoint ppzm( QgsWkbTypes::PointZM );
  QVERIFY( ppzm.is3D() );
  QVERIFY( ppzm.isMeasure() );

#if 0 //should trigger an assert
  //try creating a point with a nonsense WKB type
  QgsPoint p9( QgsWkbTypes::PolygonZM, 11.0, 13.0, 9.0, 17.0 );
  QCOMPARE( p9.wkbType(), QgsWkbTypes::Unknown );
#endif

  //test equality operator
  QVERIFY( QgsPoint( QgsWkbTypes::Point, 2 / 3.0, 1 / 3.0 ) == QgsPoint( QgsWkbTypes::Point, 2 / 3.0, 1 / 3.0 ) );
  QVERIFY( !( QgsPoint( QgsWkbTypes::PointZ, 2 / 3.0, 1 / 3.0 ) == QgsPoint( QgsWkbTypes::Point, 2 / 3.0, 1 / 3.0 ) ) );
  QVERIFY( !( QgsPoint( QgsWkbTypes::Point, 1 / 3.0, 1 / 3.0 ) == QgsPoint( QgsWkbTypes::Point, 2 / 3.0, 1 / 3.0 ) ) );
  QVERIFY( !( QgsPoint( QgsWkbTypes::Point, 2 / 3.0, 2 / 3.0 ) == QgsPoint( QgsWkbTypes::Point, 2 / 3.0, 1 / 3.0 ) ) );
  QVERIFY( QgsPoint( QgsWkbTypes::PointZ, 3.0, 4.0, 1 / 3.0 ) == QgsPoint( QgsWkbTypes::PointZ, 3.0, 4.0, 1 / 3.0 ) );
  QVERIFY( !( QgsPoint( QgsWkbTypes::PointZ, 3.0, 4.0, 1 / 3.0 ) == QgsPoint( QgsWkbTypes::PointZM, 3.0, 4.0, 1 / 3.0 ) ) );
  QVERIFY( !( QgsPoint( QgsWkbTypes::PointZ, 3.0, 4.0, 2 / 3.0 ) == QgsPoint( QgsWkbTypes::PointZ, 3.0, 4.0, 1 / 3.0 ) ) );
  QVERIFY( QgsPoint( QgsWkbTypes::PointM, 3.0, 4.0, 0.0, 1 / 3.0 ) == QgsPoint( QgsWkbTypes::PointM, 3.0, 4.0, 0.0, 1 / 3.0 ) );
  QVERIFY( !( QgsPoint( QgsWkbTypes::PointM, 3.0, 4.0, 0.0, 1 / 3.0 ) == QgsPoint( QgsWkbTypes::PointZ, 3.0, 4.0, 0.0, 1 / 3.0 ) ) );
  QVERIFY( !( QgsPoint( QgsWkbTypes::PointM, 3.0, 4.0, 0.0, 1 / 3.0 ) == QgsPoint( QgsWkbTypes::PointM, 3.0, 4.0, 0.0, 2 / 3.0 ) ) );
  QVERIFY( QgsPoint( QgsWkbTypes::PointZM, 3.0, 4.0, 2 / 3.0, 1 / 3.0 ) == QgsPoint( QgsWkbTypes::PointZM, 3.0, 4.0, 2 / 3.0, 1 / 3.0 ) );
  QVERIFY( QgsPoint( QgsWkbTypes::Point25D, 3.0, 4.0, 2 / 3.0 ) == QgsPoint( QgsWkbTypes::Point25D, 3.0, 4.0, 2 / 3.0 ) );
  QVERIFY( !( QgsPoint( QgsWkbTypes::Point25D, 3.0, 4.0, 2 / 3.0 ) == QgsPoint( QgsWkbTypes::PointZ, 3.0, 4.0, 2 / 3.0 ) ) );
  //test inequality operator
  QVERIFY( !( QgsPoint( QgsWkbTypes::Point, 2 / 3.0, 1 / 3.0 ) != QgsPoint( QgsWkbTypes::Point, 2 / 3.0, 1 / 3.0 ) ) );
  QVERIFY( QgsPoint( QgsWkbTypes::Point, 2 / 3.0, 1 / 3.0 ) != QgsPoint( QgsWkbTypes::PointZ, 2 / 3.0, 1 / 3.0 ) );

  //test setters and getters
  //x
  QgsPoint p10( QgsWkbTypes::PointZM );
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
  QCOMPARE( p10.is3D(), true );
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
  std::unique_ptr< QgsPoint >clone( p10.clone() );
  QVERIFY( p10 == *clone );

  //assignment
  QgsPoint original( QgsWkbTypes::PointZM, 1.0, 2.0, 3.0, -4.0 );
  QgsPoint assigned( 6.0, 7.0 );
  assigned = original;
  QVERIFY( assigned == original );

  //clear
  QgsPoint p11( 5.0, 6.0 );
  p11.clear();
  QCOMPARE( p11.wkbType(), QgsWkbTypes::Point );
  QCOMPARE( p11.x(), 0.0 );
  QCOMPARE( p11.y(), 0.0 );

  //toQPointF
  QgsPoint p11a( 5.0, 9.0 );
  QPointF result = p11a.toQPointF();
  QVERIFY( qgsDoubleNear( result.x(), 5.0 ) );
  QVERIFY( qgsDoubleNear( result.y(), 9.0 ) );

  //to/from WKB
  QgsPoint p12( QgsWkbTypes::PointZM, 1.0, 2.0, 3.0, -4.0 );
  QByteArray wkb12 = p12.asWkb();
  QgsPoint p13;
  QgsConstWkbPtr wkb12ptr( wkb12 );
  p13.fromWkb( wkb12ptr );
  QVERIFY( p13 == p12 );

  //bad WKB - check for no crash
  p13 = QgsPoint( 1, 2 );
  QgsConstWkbPtr nullPtr( nullptr, 0 );
  QVERIFY( !p13.fromWkb( nullPtr ) );
  QCOMPARE( p13.wkbType(), QgsWkbTypes::Point );
  QgsLineString line;
  p13 = QgsPoint( 1, 2 );
  QByteArray wkbLine = line.asWkb();
  QgsConstWkbPtr wkbLinePtr( wkbLine );
  QVERIFY( !p13.fromWkb( wkbLinePtr ) );
  QCOMPARE( p13.wkbType(), QgsWkbTypes::Point );

  //to/from WKT
  p13 = QgsPoint( QgsWkbTypes::PointZM, 1.0, 2.0, 3.0, -4.0 );
  QString wkt = p13.asWkt();
  QVERIFY( !wkt.isEmpty() );
  QgsPoint p14;
  QVERIFY( p14.fromWkt( wkt ) );
  QVERIFY( p14 == p13 );

  //bad WKT
  QVERIFY( !p14.fromWkt( "Polygon()" ) );

  //asGML2
  QgsPoint exportPoint( 1, 2 );
  QgsPoint exportPointFloat( 1 / 3.0, 2 / 3.0 );
  QDomDocument doc( QStringLiteral( "gml" ) );
  QString expectedGML2( QStringLiteral( "<Point xmlns=\"gml\"><coordinates xmlns=\"gml\" cs=\",\" ts=\" \">1,2</coordinates></Point>" ) );
  QCOMPARE( elemToString( exportPoint.asGML2( doc ) ), expectedGML2 );
  QString expectedGML2prec3( QStringLiteral( "<Point xmlns=\"gml\"><coordinates xmlns=\"gml\" cs=\",\" ts=\" \">0.333,0.667</coordinates></Point>" ) );
  QCOMPARE( elemToString( exportPointFloat.asGML2( doc, 3 ) ), expectedGML2prec3 );

  //asGML3
  QString expectedGML3( QStringLiteral( "<Point xmlns=\"gml\"><pos xmlns=\"gml\" srsDimension=\"2\">1 2</pos></Point>" ) );
  QCOMPARE( elemToString( exportPoint.asGML3( doc ) ), expectedGML3 );
  QString expectedGML3prec3( QStringLiteral( "<Point xmlns=\"gml\"><pos xmlns=\"gml\" srsDimension=\"2\">0.333 0.667</pos></Point>" ) );
  QCOMPARE( elemToString( exportPointFloat.asGML3( doc, 3 ) ), expectedGML3prec3 );

  //asJSON
  QString expectedJson( QStringLiteral( "{\"type\": \"Point\", \"coordinates\": [1, 2]}" ) );
  QCOMPARE( exportPoint.asJSON(), expectedJson );
  QString expectedJsonPrec3( QStringLiteral( "{\"type\": \"Point\", \"coordinates\": [0.333, 0.667]}" ) );
  QCOMPARE( exportPointFloat.asJSON( 3 ), expectedJsonPrec3 );

  //bounding box
  QgsPoint p15( 1.0, 2.0 );
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
  p15.moveVertex( QgsVertexId( 0, 0, 0 ), QgsPoint( 11.0, 13.0 ) );
  QCOMPARE( p15.boundingBox(), QgsRectangle( 11.0, 13.0, 11.0, 13.0 ) );
  p15 = QgsPoint( 21.0, 23.0 );
  QCOMPARE( p15.boundingBox(), QgsRectangle( 21.0, 23.0, 21.0, 23.0 ) );

  //CRS transform
  QgsCoordinateReferenceSystem sourceSrs;
  sourceSrs.createFromSrid( 3994 );
  QgsCoordinateReferenceSystem destSrs;
  destSrs.createFromSrid( 4202 ); // want a transform with ellipsoid change
  QgsCoordinateTransform tr( sourceSrs, destSrs );
  QgsPoint p16( QgsWkbTypes::PointZM, 6374985, -3626584, 1, 2 );
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
  QgsPoint p17( QgsWkbTypes::PointZM, 10, 20, 30, 40 );
  p17.transform( qtr );
  QVERIFY( p17 == QgsPoint( QgsWkbTypes::PointZM, 20, 60, 30, 40 ) );

  //coordinateSequence
  QgsPoint p18( QgsWkbTypes::PointZM, 1.0, 2.0, 3.0, 4.0 );
  QgsCoordinateSequence coord = p18.coordinateSequence();
  QCOMPARE( coord.count(), 1 );
  QCOMPARE( coord.at( 0 ).count(), 1 );
  QCOMPARE( coord.at( 0 ).at( 0 ).count(), 1 );
  QCOMPARE( coord.at( 0 ).at( 0 ).at( 0 ), p18 );

  //low level editing
  //insertVertex should have no effect
  QgsPoint p19( QgsWkbTypes::PointZM, 3.0, 4.0, 6.0, 7.0 );
  p19.insertVertex( QgsVertexId( 1, 2, 3 ), QgsPoint( 6.0, 7.0 ) );
  QCOMPARE( p19, QgsPoint( QgsWkbTypes::PointZM, 3.0, 4.0, 6.0, 7.0 ) );

  //moveVertex
  p19.moveVertex( QgsVertexId( 0, 0, 0 ), QgsPoint( QgsWkbTypes::PointZM, 1.0, 2.0, 3.0, 4.0 ) );
  QCOMPARE( p19, QgsPoint( QgsWkbTypes::PointZM, 1.0, 2.0, 3.0, 4.0 ) );
  //invalid vertex id, should not crash
  p19.moveVertex( QgsVertexId( 1, 2, 3 ), QgsPoint( QgsWkbTypes::PointZM, 2.0, 3.0, 1.0, 2.0 ) );
  QCOMPARE( p19, QgsPoint( QgsWkbTypes::PointZM, 2.0, 3.0, 1.0, 2.0 ) );
  //move PointZM using Point
  p19.moveVertex( QgsVertexId( 0, 0, 0 ), QgsPoint( QgsWkbTypes::Point, 11.0, 12.0 ) );
  QCOMPARE( p19, QgsPoint( QgsWkbTypes::PointZM, 11.0, 12.0, 1.0, 2.0 ) );
  //move PointZM using PointZ
  p19.moveVertex( QgsVertexId( 0, 0, 0 ), QgsPoint( QgsWkbTypes::PointZ, 21.0, 22.0, 23.0 ) );
  QCOMPARE( p19, QgsPoint( QgsWkbTypes::PointZM, 21.0, 22.0, 23.0, 2.0 ) );
  //move PointZM using PointM
  p19.moveVertex( QgsVertexId( 0, 0, 0 ), QgsPoint( QgsWkbTypes::PointM, 31.0, 32.0, 0.0, 43.0 ) );
  QCOMPARE( p19, QgsPoint( QgsWkbTypes::PointZM, 31.0, 32.0, 23.0, 43.0 ) );
  //move Point using PointZM (z/m should be ignored)
  QgsPoint p20( 3.0, 4.0 );
  p20.moveVertex( QgsVertexId( 0, 0, 0 ), QgsPoint( QgsWkbTypes::PointZM, 2.0, 3.0, 1.0, 2.0 ) );
  QCOMPARE( p20, QgsPoint( 2.0, 3.0 ) );

  //deleteVertex - should do nothing, but not crash
  p20.deleteVertex( QgsVertexId( 0, 0, 0 ) );
  QCOMPARE( p20, QgsPoint( 2.0, 3.0 ) );

  // closestSegment
  QgsPoint closest;
  QgsVertexId after;
  // return error - points have no segments
  QVERIFY( p20.closestSegment( QgsPoint( 4.0, 6.0 ), closest, after, 0, 0 ) < 0 );

  //nextVertex
  QgsPoint p21( 3.0, 4.0 );
  QgsPoint p22;
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
  std::unique_ptr< QgsPoint >segmented( static_cast< QgsPoint *>( p20.segmentize() ) );
  QCOMPARE( *segmented, p20 );

  //addZValue
  QgsPoint p23( 1.0, 2.0 );
  QVERIFY( p23.addZValue( 5.0 ) );
  QCOMPARE( p23, QgsPoint( QgsWkbTypes::PointZ, 1.0, 2.0, 5.0 ) );
  QVERIFY( !p23.addZValue( 6.0 ) );

  //addMValue
  QgsPoint p24( 1.0, 2.0 );
  QVERIFY( p24.addMValue( 5.0 ) );
  QCOMPARE( p24, QgsPoint( QgsWkbTypes::PointM, 1.0, 2.0, 0.0, 5.0 ) );
  QVERIFY( !p24.addMValue( 6.0 ) );

  //dropZ
  QgsPoint p25( QgsWkbTypes::PointZ, 1.0, 2.0, 3.0 );
  QVERIFY( p25.dropZValue() );
  QCOMPARE( p25, QgsPoint( 1.0, 2.0 ) );
  QVERIFY( !p25.dropZValue() );
  QgsPoint p26( QgsWkbTypes::PointZM, 1.0, 2.0, 3.0, 4.0 );
  QVERIFY( p26.dropZValue() );
  QCOMPARE( p26, QgsPoint( QgsWkbTypes::PointM, 1.0, 2.0, 0.0, 4.0 ) );
  QVERIFY( !p26.dropZValue() );
  QgsPoint p26a( QgsWkbTypes::Point25D, 1.0, 2.0, 3.0 );
  QVERIFY( p26a.dropZValue() );
  QCOMPARE( p26a, QgsPoint( QgsWkbTypes::Point, 1.0, 2.0 ) );
  QVERIFY( !p26a.dropZValue() );

  //dropM
  QgsPoint p27( QgsWkbTypes::PointM, 1.0, 2.0, 0.0, 3.0 );
  QVERIFY( p27.dropMValue() );
  QCOMPARE( p27, QgsPoint( 1.0, 2.0 ) );
  QVERIFY( !p27.dropMValue() );
  QgsPoint p28( QgsWkbTypes::PointZM, 1.0, 2.0, 3.0, 4.0 );
  QVERIFY( p28.dropMValue() );
  QCOMPARE( p28, QgsPoint( QgsWkbTypes::PointZ, 1.0, 2.0, 3.0, 0.0 ) );
  QVERIFY( !p28.dropMValue() );

  //convertTo
  QgsPoint p29( 1.0, 2.0 );
  QVERIFY( p29.convertTo( QgsWkbTypes::Point ) );
  QCOMPARE( p29.wkbType(), QgsWkbTypes::Point );
  QVERIFY( p29.convertTo( QgsWkbTypes::PointZ ) );
  QCOMPARE( p29.wkbType(), QgsWkbTypes::PointZ );
  p29.setZ( 5.0 );
  QVERIFY( p29.convertTo( QgsWkbTypes::Point25D ) );
  QCOMPARE( p29.wkbType(), QgsWkbTypes::Point25D );
  QCOMPARE( p29.z(), 5.0 );
  QVERIFY( p29.convertTo( QgsWkbTypes::PointZM ) );
  QCOMPARE( p29.wkbType(), QgsWkbTypes::PointZM );
  QCOMPARE( p29.z(), 5.0 );
  p29.setM( 9.0 );
  QVERIFY( p29.convertTo( QgsWkbTypes::PointM ) );
  QCOMPARE( p29.wkbType(), QgsWkbTypes::PointM );
  QVERIFY( qIsNaN( p29.z() ) );
  QCOMPARE( p29.m(), 9.0 );
  QVERIFY( p29.convertTo( QgsWkbTypes::Point ) );
  QCOMPARE( p29.wkbType(), QgsWkbTypes::Point );
  QVERIFY( qIsNaN( p29.z() ) );
  QVERIFY( qIsNaN( p29.m() ) );
  QVERIFY( !p29.convertTo( QgsWkbTypes::Polygon ) );

  //boundary
  QgsPoint p30( 1.0, 2.0 );
  QVERIFY( !p30.boundary() );

  // distance
  QCOMPARE( QgsPoint( 1, 2 ).distance( QgsPoint( 2, 2 ) ), 1.0 );
  QCOMPARE( QgsPoint( 1, 2 ).distance( 2, 2 ), 1.0 );
  QCOMPARE( QgsPoint( 1, 2 ).distance( QgsPoint( 3, 2 ) ), 2.0 );
  QCOMPARE( QgsPoint( 1, 2 ).distance( 3, 2 ), 2.0 );
  QCOMPARE( QgsPoint( 1, 2 ).distance( QgsPoint( 1, 3 ) ), 1.0 );
  QCOMPARE( QgsPoint( 1, 2 ).distance( 1, 3 ), 1.0 );
  QCOMPARE( QgsPoint( 1, 2 ).distance( QgsPoint( 1, 4 ) ), 2.0 );
  QCOMPARE( QgsPoint( 1, 2 ).distance( 1, 4 ), 2.0 );
  QCOMPARE( QgsPoint( 1, -2 ).distance( QgsPoint( 1, -4 ) ), 2.0 );
  QCOMPARE( QgsPoint( 1, -2 ).distance( 1, -4 ), 2.0 );

  QCOMPARE( QgsPoint( 1, 2 ).distanceSquared( QgsPoint( 2, 2 ) ), 1.0 );
  QCOMPARE( QgsPoint( 1, 2 ).distanceSquared( 2, 2 ), 1.0 );
  QCOMPARE( QgsPoint( 1, 2 ).distanceSquared( QgsPoint( 3, 2 ) ), 4.0 );
  QCOMPARE( QgsPoint( 1, 2 ).distanceSquared( 3, 2 ), 4.0 );
  QCOMPARE( QgsPoint( 1, 2 ).distanceSquared( QgsPoint( 1, 3 ) ), 1.0 );
  QCOMPARE( QgsPoint( 1, 2 ).distanceSquared( 1, 3 ), 1.0 );
  QCOMPARE( QgsPoint( 1, 2 ).distanceSquared( QgsPoint( 1, 4 ) ), 4.0 );
  QCOMPARE( QgsPoint( 1, 2 ).distanceSquared( 1, 4 ), 4.0 );
  QCOMPARE( QgsPoint( 1, -2 ).distanceSquared( QgsPoint( 1, -4 ) ), 4.0 );
  QCOMPARE( QgsPoint( 1, -2 ).distanceSquared( 1, -4 ), 4.0 );

  // distance 3D
  QCOMPARE( QgsPoint( 0, 0 ).distanceSquared3D( QgsPoint( 1, 1 ) ), 2.0 );
  QVERIFY( qIsNaN( QgsPoint( 0, 0 ).distanceSquared3D( 1, 1, 0 ) ) );
  QVERIFY( qIsNaN( QgsPoint( 0, 0 ).distanceSquared3D( QgsPoint( QgsWkbTypes::PointZ, 2, 2, 2, 0 ) ) ) );
  QVERIFY( qIsNaN( QgsPoint( 0, 0 ).distanceSquared3D( 2, 2, 2 ) ) );
  QVERIFY( qIsNaN( QgsPoint( QgsWkbTypes::PointZ, 2, 2, 2, 0 ).distanceSquared3D( QgsPoint( 1, 1 ) ) ) );
  QCOMPARE( QgsPoint( QgsWkbTypes::PointZ, 2, 2, 2, 0 ).distanceSquared3D( 1, 1, 0 ), 6.0 );
  QVERIFY( qIsNaN( QgsPoint( QgsWkbTypes::PointZ, -2, -2, -2, 0 ).distanceSquared3D( QgsPoint( 0, 0 ) ) ) );
  QCOMPARE( QgsPoint( QgsWkbTypes::PointZ, -2, -2, -2, 0 ).distanceSquared3D( 0, 0, 0 ), 12.0 );
  QCOMPARE( QgsPoint( QgsWkbTypes::PointZ, -2, -2, -2, 0 ).distanceSquared3D( QgsPoint( QgsWkbTypes::PointZ, 2, 2, 2, 0 ) ), 48.0 );
  QCOMPARE( QgsPoint( QgsWkbTypes::PointZ, -2, -2, -2, 0 ).distanceSquared3D( 2, 2, 2 ), 48.0 );


  QCOMPARE( QgsPoint( QgsWkbTypes::PointZ, 1, 1, 2, 0 ).distance3D( QgsPoint( QgsWkbTypes::PointZ, 1, 3, 2, 0 ) ), 2.0 );
  QCOMPARE( QgsPoint( QgsWkbTypes::PointZ, 1, 1, 2, 0 ).distance3D( 1, 3, 2 ), 2.0 );
  QCOMPARE( QgsPoint( QgsWkbTypes::PointZ, 1, 1, 2, 0 ).distance3D( QgsPoint( QgsWkbTypes::PointZ, 1, 1, 4, 0 ) ), 2.0 );
  QCOMPARE( QgsPoint( QgsWkbTypes::PointZ, 1, 1, 2, 0 ).distance3D( 1, 1, 4 ), 2.0 );
  QCOMPARE( QgsPoint( QgsWkbTypes::PointZ, 1, 1, -2, 0 ).distance3D( QgsPoint( QgsWkbTypes::PointZ, 1, 1, -4, 0 ) ), 2.0 );
  QCOMPARE( QgsPoint( QgsWkbTypes::PointZ, 1, 1, -2, 0 ).distance3D( 1, 1, -4 ), 2.0 );

  // azimuth
  QCOMPARE( QgsPoint( 1, 2 ).azimuth( QgsPoint( 1, 2 ) ), 0.0 );
  QCOMPARE( QgsPoint( 1, 2 ).azimuth( QgsPoint( 1, 3 ) ), 0.0 );
  QCOMPARE( QgsPoint( 1, 2 ).azimuth( QgsPoint( 2, 2 ) ), 90.0 );
  QCOMPARE( QgsPoint( 1, 2 ).azimuth( QgsPoint( 1, 0 ) ), 180.0 );
  QCOMPARE( QgsPoint( 1, 2 ).azimuth( QgsPoint( 0, 2 ) ), -90.0 );

  // operators
  QgsPoint p31( 1, 2 );
  QgsPoint p32( 3, 5 );
  QCOMPARE( p32 - p31, QgsVector( 2, 3 ) );
  QCOMPARE( p31 - p32, QgsVector( -2, -3 ) );

  p31 = QgsPoint( 1, 2 );
  QCOMPARE( p31 + QgsVector( 3, 5 ), QgsPoint( 4, 7 ) );
  p31 += QgsVector( 3, 5 );
  QCOMPARE( p31, QgsPoint( 4, 7 ) );

  QCOMPARE( p31 - QgsVector( 3, 5 ), QgsPoint( 1, 2 ) );
  p31 -= QgsVector( 3, 5 );
  QCOMPARE( p31, QgsPoint( 1, 2 ) );

  // test projecting a point
  // 2D
  QgsPoint p33 = QgsPoint( 1, 2 );
  QCOMPARE( p33.project( 1, 0 ), QgsPoint( 1, 3 ) );
  QCOMPARE( p33.project( 1, 0, 0 ), QgsPoint( QgsWkbTypes::PointZ, 1, 2 ) );
  QCOMPARE( p33.project( 1.5, 90 ), QgsPoint( 2.5, 2 ) );
  QCOMPARE( p33.project( 1.5, 90, 90 ), QgsPoint( 2.5, 2 ) ); // stay QgsWkbTypes::Point
  QCOMPARE( p33.project( 2, 180 ), QgsPoint( 1, 0 ) );
  QCOMPARE( p33.project( 5, 270 ), QgsPoint( -4, 2 ) );
  QCOMPARE( p33.project( 6, 360 ), QgsPoint( 1, 8 ) );
  QCOMPARE( p33.project( 5, 450 ), QgsPoint( 6, 2 ) );
  QCOMPARE( p33.project( 5, 450, 450 ), QgsPoint( 6, 2 ) );  // stay QgsWkbTypes::Point
  QCOMPARE( p33.project( -1, 0 ), QgsPoint( 1, 1 ) );
  QCOMPARE( p33.project( 1.5, -90 ), QgsPoint( -0.5, 2 ) );
  p33.addZValue( 0 );
  QCOMPARE( p33.project( 1, 0, 0 ), QgsPoint( QgsWkbTypes::PointZ, 1, 2, 1 ) );
  QCOMPARE( p33.project( 2, 180, 180 ), QgsPoint( QgsWkbTypes::PointZ,  1, 2, -2 ) );
  QCOMPARE( p33.project( 5, 270, 270 ), QgsPoint( QgsWkbTypes::PointZ,  6, 2, 0 ) );
  QCOMPARE( p33.project( 6, 360, 360 ), QgsPoint( QgsWkbTypes::PointZ,  1, 2, 6 ) );
  QCOMPARE( p33.project( -1, 0, 0 ), QgsPoint( QgsWkbTypes::PointZ, 1, 2, -1 ) );
  QCOMPARE( p33.project( 1.5, -90, -90 ), QgsPoint( QgsWkbTypes::PointZ, 2.5, 2, 0 ) );

  // PointM
  p33.dropZValue();
  p33.addMValue( 5.0 );
  QCOMPARE( p33.project( 1, 0 ), QgsPoint( QgsWkbTypes::PointM, 1, 3, 0, 5 ) );
  QCOMPARE( p33.project( 5, 450, 450 ), QgsPoint( QgsWkbTypes::PointM, 6, 2, 0, 5 ) );

  p33.addZValue( 0 );
  QCOMPARE( p33.project( 1, 0, 0 ), QgsPoint( QgsWkbTypes::PointZM, 1, 2, 1, 5 ) );

  // 3D
  QgsPoint p34 = QgsPoint( QgsWkbTypes::PointZ, 1, 2, 2 );
  QCOMPARE( p34.project( 1, 0 ), QgsPoint( QgsWkbTypes::PointZ, 1, 3, 2 ) );
  QCOMPARE( p34.project( 1, 0, 0 ), QgsPoint( QgsWkbTypes::PointZ, 1, 2, 3 ) );
  QCOMPARE( p34.project( 1.5, 90 ), QgsPoint( QgsWkbTypes::PointZ, 2.5, 2, 2 ) );
  QCOMPARE( p34.project( 1.5, 90, 90 ), QgsPoint( QgsWkbTypes::PointZ, 2.5, 2, 2 ) );
  QCOMPARE( p34.project( 2, 180 ), QgsPoint( QgsWkbTypes::PointZ, 1, 0, 2 ) );
  QCOMPARE( p34.project( 2, 180, 180 ), QgsPoint( QgsWkbTypes::PointZ, 1, 2, 0 ) );
  QCOMPARE( p34.project( 5, 270 ), QgsPoint( QgsWkbTypes::PointZ, -4, 2, 2 ) );
  QCOMPARE( p34.project( 5, 270, 270 ), QgsPoint( QgsWkbTypes::PointZ, 6, 2, 2 ) );
  QCOMPARE( p34.project( 6, 360 ), QgsPoint( QgsWkbTypes::PointZ, 1, 8, 2 ) );
  QCOMPARE( p34.project( 6, 360, 360 ), QgsPoint( QgsWkbTypes::PointZ, 1, 2, 8 ) );
  QCOMPARE( p34.project( 5, 450 ), QgsPoint( QgsWkbTypes::PointZ, 6, 2, 2 ) );
  QCOMPARE( p34.project( 5, 450, 450 ), QgsPoint( QgsWkbTypes::PointZ, 6, 2, 2 ) );
  QCOMPARE( p34.project( -1, 0 ), QgsPoint( QgsWkbTypes::PointZ, 1, 1, 2 ) );
  QCOMPARE( p34.project( -1, 0, 0 ), QgsPoint( QgsWkbTypes::PointZ, 1, 2, 1 ) );
  QCOMPARE( p34.project( 1.5, -90 ), QgsPoint( QgsWkbTypes::PointZ, -0.5, 2, 2 ) );
  QCOMPARE( p34.project( 1.5, -90, -90 ), QgsPoint( QgsWkbTypes::PointZ, 2.5, 2, 2 ) );
  // PointM
  p34.addMValue( 5.0 );
  QCOMPARE( p34.project( 1, 0 ), QgsPoint( QgsWkbTypes::PointZM, 1, 3, 2, 5 ) );
  QCOMPARE( p34.project( 1, 0, 0 ), QgsPoint( QgsWkbTypes::PointZM, 1, 2, 3, 5 ) );
  QCOMPARE( p34.project( 5, 450 ), QgsPoint( QgsWkbTypes::PointZM, 6, 2, 2, 5 ) );
  QCOMPARE( p34.project( 5, 450, 450 ), QgsPoint( QgsWkbTypes::PointZM, 6, 2, 2, 5 ) );

  // inclination
  QCOMPARE( QgsPoint( 1, 2 ).inclination( QgsPoint( 1, 2 ) ), 90.0 );
  QCOMPARE( QgsPoint( QgsWkbTypes::PointZ, 1, 1, 2, 0 ).inclination( QgsPoint( QgsWkbTypes::PointZ, 1, 1, 2, 0 ) ), 90.0 );
  QCOMPARE( QgsPoint( QgsWkbTypes::PointZ, 1, 2, 2 ).inclination( QgsPoint( QgsWkbTypes::PointZ, 1, 2, 2 ).project( 5, 90, 90 ) ), 90.0 );
  QCOMPARE( QgsPoint( QgsWkbTypes::PointZ, 1, 2, 2 ).inclination( QgsPoint( QgsWkbTypes::PointZ, 1, 2, 2 ).project( 5, 90, -90 ) ), 90.0 );
  QCOMPARE( QgsPoint( QgsWkbTypes::PointZ, 1, 2, 2 ).inclination( QgsPoint( QgsWkbTypes::PointZ, 1, 2, 2 ).project( 5, 90, 0 ) ), 0.0 );
  QCOMPARE( QgsPoint( QgsWkbTypes::PointZ, 1, 2, 2 ).inclination( QgsPoint( QgsWkbTypes::PointZ, 1, 2, 2 ).project( 5, 90, 180 ) ), 180.0 );
  QCOMPARE( QgsPoint( QgsWkbTypes::PointZ, 1, 2, 2 ).inclination( QgsPoint( QgsWkbTypes::PointZ, 1, 2, 2 ).project( 5, 90, -180 ) ), 180.0 );
  QCOMPARE( QgsPoint( QgsWkbTypes::PointZ, 1, 2, 2 ).inclination( QgsPoint( QgsWkbTypes::PointZ, 1, 2, 2 ).project( 5, 90, 720 ) ), 0.0 );
  QCOMPARE( QgsPoint( QgsWkbTypes::PointZ, 1, 2, 2 ).inclination( QgsPoint( QgsWkbTypes::PointZ, 1, 2, 2 ).project( 5, 90, 45 ) ), 45.0 );
  QCOMPARE( QgsPoint( QgsWkbTypes::PointZ, 1, 2, 2 ).inclination( QgsPoint( QgsWkbTypes::PointZ, 1, 2, 2 ).project( 5, 90, 135 ) ), 135.0 );

}

void TestQgsGeometry::lineString()
{
  //test constructors
  QgsLineString l1;
  QVERIFY( l1.isEmpty() );
  QCOMPARE( l1.numPoints(), 0 );
  QCOMPARE( l1.vertexCount(), 0 );
  QCOMPARE( l1.nCoordinates(), 0 );
  QCOMPARE( l1.ringCount(), 0 );
  QCOMPARE( l1.partCount(), 0 );
  QVERIFY( !l1.is3D() );
  QVERIFY( !l1.isMeasure() );
  QCOMPARE( l1.wkbType(), QgsWkbTypes::LineString );
  QCOMPARE( l1.wktTypeStr(), QString( "LineString" ) );
  QCOMPARE( l1.geometryType(), QString( "LineString" ) );
  QCOMPARE( l1.dimension(), 1 );
  QVERIFY( !l1.hasCurvedSegments() );
  QCOMPARE( l1.area(), 0.0 );
  QCOMPARE( l1.perimeter(), 0.0 );

  // from array
  QVector< double > xx;
  xx << 1 << 2 << 3;
  QVector< double > yy;
  yy << 11 << 12 << 13;
  QgsLineString fromArray( xx, yy );
  QCOMPARE( fromArray.wkbType(), QgsWkbTypes::LineString );
  QCOMPARE( fromArray.numPoints(), 3 );
  QCOMPARE( fromArray.xAt( 0 ), 1.0 );
  QCOMPARE( fromArray.yAt( 0 ), 11.0 );
  QCOMPARE( fromArray.xAt( 1 ), 2.0 );
  QCOMPARE( fromArray.yAt( 1 ), 12.0 );
  QCOMPARE( fromArray.xAt( 2 ), 3.0 );
  QCOMPARE( fromArray.yAt( 2 ), 13.0 );
  // unbalanced
  xx = QVector< double >() << 1 << 2;
  yy = QVector< double >() << 11 << 12 << 13;
  QgsLineString fromArray2( xx, yy );
  QCOMPARE( fromArray2.wkbType(), QgsWkbTypes::LineString );
  QCOMPARE( fromArray2.numPoints(), 2 );
  QCOMPARE( fromArray2.xAt( 0 ), 1.0 );
  QCOMPARE( fromArray2.yAt( 0 ), 11.0 );
  QCOMPARE( fromArray2.xAt( 1 ), 2.0 );
  QCOMPARE( fromArray2.yAt( 1 ), 12.0 );
  xx = QVector< double >() << 1 << 2 << 3;
  yy = QVector< double >() << 11 << 12;
  QgsLineString fromArray3( xx, yy );
  QCOMPARE( fromArray3.wkbType(), QgsWkbTypes::LineString );
  QCOMPARE( fromArray3.numPoints(), 2 );
  QCOMPARE( fromArray3.xAt( 0 ), 1.0 );
  QCOMPARE( fromArray3.yAt( 0 ), 11.0 );
  QCOMPARE( fromArray3.xAt( 1 ), 2.0 );
  QCOMPARE( fromArray3.yAt( 1 ), 12.0 );
  // with z
  QVector< double > zz;
  xx = QVector< double >() << 1 << 2 << 3;
  yy = QVector< double >() << 11 << 12 << 13;
  zz = QVector< double >() << 21 << 22 << 23;
  QgsLineString fromArray4( xx, yy, zz );
  QCOMPARE( fromArray4.wkbType(), QgsWkbTypes::LineStringZ );
  QCOMPARE( fromArray4.numPoints(), 3 );
  QCOMPARE( fromArray4.xAt( 0 ), 1.0 );
  QCOMPARE( fromArray4.yAt( 0 ), 11.0 );
  QCOMPARE( fromArray4.zAt( 0 ), 21.0 );
  QCOMPARE( fromArray4.xAt( 1 ), 2.0 );
  QCOMPARE( fromArray4.yAt( 1 ), 12.0 );
  QCOMPARE( fromArray4.zAt( 1 ), 22.0 );
  QCOMPARE( fromArray4.xAt( 2 ), 3.0 );
  QCOMPARE( fromArray4.yAt( 2 ), 13.0 );
  QCOMPARE( fromArray4.zAt( 2 ), 23.0 );
  // unbalanced -> z ignored
  zz = QVector< double >() << 21 << 22;
  QgsLineString fromArray5( xx, yy, zz );
  QCOMPARE( fromArray5.wkbType(), QgsWkbTypes::LineString );
  QCOMPARE( fromArray5.numPoints(), 3 );
  QCOMPARE( fromArray5.xAt( 0 ), 1.0 );
  QCOMPARE( fromArray5.yAt( 0 ), 11.0 );
  QCOMPARE( fromArray5.xAt( 1 ), 2.0 );
  QCOMPARE( fromArray5.yAt( 1 ), 12.0 );
  QCOMPARE( fromArray5.xAt( 2 ), 3.0 );
  QCOMPARE( fromArray5.yAt( 2 ), 13.0 );
  // with m
  QVector< double > mm;
  xx = QVector< double >() << 1 << 2 << 3;
  yy = QVector< double >() << 11 << 12 << 13;
  mm = QVector< double >() << 21 << 22 << 23;
  QgsLineString fromArray6( xx, yy, QVector< double >(), mm );
  QCOMPARE( fromArray6.wkbType(), QgsWkbTypes::LineStringM );
  QCOMPARE( fromArray6.numPoints(), 3 );
  QCOMPARE( fromArray6.xAt( 0 ), 1.0 );
  QCOMPARE( fromArray6.yAt( 0 ), 11.0 );
  QCOMPARE( fromArray6.mAt( 0 ), 21.0 );
  QCOMPARE( fromArray6.xAt( 1 ), 2.0 );
  QCOMPARE( fromArray6.yAt( 1 ), 12.0 );
  QCOMPARE( fromArray6.mAt( 1 ), 22.0 );
  QCOMPARE( fromArray6.xAt( 2 ), 3.0 );
  QCOMPARE( fromArray6.yAt( 2 ), 13.0 );
  QCOMPARE( fromArray6.mAt( 2 ), 23.0 );
  // unbalanced -> m ignored
  mm = QVector< double >() << 21 << 22;
  QgsLineString fromArray7( xx, yy, QVector< double >(), mm );
  QCOMPARE( fromArray7.wkbType(), QgsWkbTypes::LineString );
  QCOMPARE( fromArray7.numPoints(), 3 );
  QCOMPARE( fromArray7.xAt( 0 ), 1.0 );
  QCOMPARE( fromArray7.yAt( 0 ), 11.0 );
  QCOMPARE( fromArray7.xAt( 1 ), 2.0 );
  QCOMPARE( fromArray7.yAt( 1 ), 12.0 );
  QCOMPARE( fromArray7.xAt( 2 ), 3.0 );
  QCOMPARE( fromArray7.yAt( 2 ), 13.0 );
  // zm
  xx = QVector< double >() << 1 << 2 << 3;
  yy = QVector< double >() << 11 << 12 << 13;
  zz = QVector< double >() << 21 << 22 << 23;
  mm = QVector< double >() << 31 << 32 << 33;
  QgsLineString fromArray8( xx, yy, zz, mm );
  QCOMPARE( fromArray8.wkbType(), QgsWkbTypes::LineStringZM );
  QCOMPARE( fromArray8.numPoints(), 3 );
  QCOMPARE( fromArray8.xAt( 0 ), 1.0 );
  QCOMPARE( fromArray8.yAt( 0 ), 11.0 );
  QCOMPARE( fromArray8.zAt( 0 ), 21.0 );
  QCOMPARE( fromArray8.mAt( 0 ), 31.0 );
  QCOMPARE( fromArray8.xAt( 1 ), 2.0 );
  QCOMPARE( fromArray8.yAt( 1 ), 12.0 );
  QCOMPARE( fromArray8.zAt( 1 ), 22.0 );
  QCOMPARE( fromArray8.mAt( 1 ), 32.0 );
  QCOMPARE( fromArray8.xAt( 2 ), 3.0 );
  QCOMPARE( fromArray8.yAt( 2 ), 13.0 );
  QCOMPARE( fromArray8.zAt( 2 ), 23.0 );
  QCOMPARE( fromArray8.mAt( 2 ), 33.0 );

  // from QList<QgsPointXY>
  QList<QgsPointXY> ptsA;
  ptsA << QgsPointXY( 1, 2 ) << QgsPointXY( 11, 12 ) << QgsPointXY( 21, 22 );
  QgsLineString fromPts( ptsA );
  QCOMPARE( fromPts.wkbType(), QgsWkbTypes::LineString );
  QCOMPARE( fromPts.numPoints(), 3 );
  QCOMPARE( fromPts.xAt( 0 ), 1.0 );
  QCOMPARE( fromPts.yAt( 0 ), 2.0 );
  QCOMPARE( fromPts.xAt( 1 ), 11.0 );
  QCOMPARE( fromPts.yAt( 1 ), 12.0 );
  QCOMPARE( fromPts.xAt( 2 ), 21.0 );
  QCOMPARE( fromPts.yAt( 2 ), 22.0 );

  // from QVector<QgsPoint>
  QVector<QgsPoint> ptsVector;
  ptsVector << QgsPoint( 10, 20 ) << QgsPoint( 30, 40 );
  QgsLineString fromVector( ptsVector );
  QCOMPARE( fromVector.wkbType(), QgsWkbTypes::LineString );
  QCOMPARE( fromVector.numPoints(), 2 );
  QCOMPARE( fromVector.xAt( 0 ), 10.0 );
  QCOMPARE( fromVector.yAt( 0 ), 20.0 );
  QCOMPARE( fromVector.xAt( 1 ), 30.0 );
  QCOMPARE( fromVector.yAt( 1 ), 40.0 );
  QVector<QgsPoint> ptsVector3D;
  ptsVector3D << QgsPoint( QgsWkbTypes::PointZ, 10, 20, 100 ) << QgsPoint( QgsWkbTypes::PointZ, 30, 40, 200 );
  QgsLineString fromVector3D( ptsVector3D );
  QCOMPARE( fromVector3D.wkbType(), QgsWkbTypes::LineStringZ );
  QCOMPARE( fromVector3D.numPoints(), 2 );
  QCOMPARE( fromVector3D.xAt( 0 ), 10.0 );
  QCOMPARE( fromVector3D.yAt( 0 ), 20.0 );
  QCOMPARE( fromVector3D.zAt( 0 ), 100.0 );
  QCOMPARE( fromVector3D.xAt( 1 ), 30.0 );
  QCOMPARE( fromVector3D.yAt( 1 ), 40.0 );
  QCOMPARE( fromVector3D.zAt( 1 ), 200.0 );

  //addVertex
  QgsLineString l2;
  l2.addVertex( QgsPoint( 1.0, 2.0 ) );
  QVERIFY( !l2.isEmpty() );
  QCOMPARE( l2.numPoints(), 1 );
  QCOMPARE( l2.vertexCount(), 1 );
  QCOMPARE( l2.nCoordinates(), 1 );
  QCOMPARE( l2.ringCount(), 1 );
  QCOMPARE( l2.partCount(), 1 );
  QVERIFY( !l2.is3D() );
  QVERIFY( !l2.isMeasure() );
  QCOMPARE( l2.wkbType(), QgsWkbTypes::LineString );
  QVERIFY( !l2.hasCurvedSegments() );
  QCOMPARE( l2.area(), 0.0 );
  QCOMPARE( l2.perimeter(), 0.0 );

  //adding first vertex should set linestring z/m type
  QgsLineString l3;
  l3.addVertex( QgsPoint( QgsWkbTypes::PointZ, 1.0, 2.0, 3.0 ) );
  QVERIFY( !l3.isEmpty() );
  QVERIFY( l3.is3D() );
  QVERIFY( !l3.isMeasure() );
  QCOMPARE( l3.wkbType(), QgsWkbTypes::LineStringZ );
  QCOMPARE( l3.wktTypeStr(), QString( "LineStringZ" ) );

  QgsLineString l4;
  l4.addVertex( QgsPoint( QgsWkbTypes::PointM, 1.0, 2.0, 0.0, 3.0 ) );
  QVERIFY( !l4.isEmpty() );
  QVERIFY( !l4.is3D() );
  QVERIFY( l4.isMeasure() );
  QCOMPARE( l4.wkbType(), QgsWkbTypes::LineStringM );
  QCOMPARE( l4.wktTypeStr(), QString( "LineStringM" ) );

  QgsLineString l5;
  l5.addVertex( QgsPoint( QgsWkbTypes::PointZM, 1.0, 2.0, 3.0, 4.0 ) );
  QVERIFY( !l5.isEmpty() );
  QVERIFY( l5.is3D() );
  QVERIFY( l5.isMeasure() );
  QCOMPARE( l5.wkbType(), QgsWkbTypes::LineStringZM );
  QCOMPARE( l5.wktTypeStr(), QString( "LineStringZM" ) );

  QgsLineString l25d;
  l25d.addVertex( QgsPoint( QgsWkbTypes::Point25D, 1.0, 2.0, 3.0 ) );
  QVERIFY( !l25d.isEmpty() );
  QVERIFY( l25d.is3D() );
  QVERIFY( !l25d.isMeasure() );
  QCOMPARE( l25d.wkbType(), QgsWkbTypes::LineString25D );
  QCOMPARE( l25d.wktTypeStr(), QString( "LineStringZ" ) );

  //adding subsequent vertices should not alter z/m type, regardless of points type
  QgsLineString l6;
  l6.addVertex( QgsPoint( QgsWkbTypes::Point, 1.0, 2.0 ) ); //2d type
  QCOMPARE( l6.wkbType(), QgsWkbTypes::LineString );
  l6.addVertex( QgsPoint( QgsWkbTypes::PointZ, 11.0, 12.0, 13.0 ) ); // add 3d point
  QCOMPARE( l6.numPoints(), 2 );
  QCOMPARE( l6.vertexCount(), 2 );
  QCOMPARE( l6.nCoordinates(), 2 );
  QCOMPARE( l6.ringCount(), 1 );
  QCOMPARE( l6.partCount(), 1 );
  QCOMPARE( l6.wkbType(), QgsWkbTypes::LineString ); //should still be 2d
  QVERIFY( !l6.is3D() );
  QCOMPARE( l6.area(), 0.0 );
  QCOMPARE( l6.perimeter(), 0.0 );

  QgsLineString l7;
  l7.addVertex( QgsPoint( QgsWkbTypes::PointZ, 1.0, 2.0, 3.0 ) ); //3d type
  QCOMPARE( l7.wkbType(), QgsWkbTypes::LineStringZ );
  l7.addVertex( QgsPoint( QgsWkbTypes::Point, 11.0, 12.0 ) ); //add 2d point
  QCOMPARE( l7.wkbType(), QgsWkbTypes::LineStringZ ); //should still be 3d
  QCOMPARE( l7.pointN( 1 ), QgsPoint( QgsWkbTypes::PointZ, 11.0, 12.0 ) );
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
  QCOMPARE( l7.wkbType(), QgsWkbTypes::LineString );

  //setPoints
  QgsLineString l8;
  l8.setPoints( QgsPointSequence() << QgsPoint( 1, 2 ) << QgsPoint( 2, 3 ) << QgsPoint( 3, 4 ) );
  QVERIFY( !l8.isEmpty() );
  QCOMPARE( l8.numPoints(), 3 );
  QCOMPARE( l8.vertexCount(), 3 );
  QCOMPARE( l8.nCoordinates(), 3 );
  QCOMPARE( l8.ringCount(), 1 );
  QCOMPARE( l8.partCount(), 1 );
  QVERIFY( !l8.is3D() );
  QVERIFY( !l8.isMeasure() );
  QCOMPARE( l8.wkbType(), QgsWkbTypes::LineString );
  QVERIFY( !l8.hasCurvedSegments() );

  //setPoints with empty list, should clear linestring
  l8.setPoints( QgsPointSequence() );
  QVERIFY( l8.isEmpty() );
  QCOMPARE( l8.numPoints(), 0 );
  QCOMPARE( l8.vertexCount(), 0 );
  QCOMPARE( l8.nCoordinates(), 0 );
  QCOMPARE( l8.ringCount(), 0 );
  QCOMPARE( l8.partCount(), 0 );
  QCOMPARE( l8.wkbType(), QgsWkbTypes::LineString );

  //setPoints with z
  l8.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZ, 1, 2, 3 ) << QgsPoint( QgsWkbTypes::PointZ, 2, 3, 4 ) );
  QCOMPARE( l8.numPoints(), 2 );
  QVERIFY( l8.is3D() );
  QVERIFY( !l8.isMeasure() );
  QCOMPARE( l8.wkbType(), QgsWkbTypes::LineStringZ );

  //setPoints with 25d
  l8.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::Point25D, 1, 2, 4 ) << QgsPoint( QgsWkbTypes::Point25D, 2, 3, 4 ) );
  QCOMPARE( l8.numPoints(), 2 );
  QVERIFY( l8.is3D() );
  QVERIFY( !l8.isMeasure() );
  QCOMPARE( l8.wkbType(), QgsWkbTypes::LineString25D );
  QCOMPARE( l8.pointN( 0 ), QgsPoint( QgsWkbTypes::Point25D, 1, 2, 4 ) );

  //setPoints with m
  l8.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointM, 1, 2, 0, 3 ) << QgsPoint( QgsWkbTypes::PointM, 2, 3, 0, 4 ) );
  QCOMPARE( l8.numPoints(), 2 );
  QVERIFY( !l8.is3D() );
  QVERIFY( l8.isMeasure() );
  QCOMPARE( l8.wkbType(), QgsWkbTypes::LineStringM );

  //setPoints with zm
  l8.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 1, 2, 4, 5 ) << QgsPoint( QgsWkbTypes::PointZM, 2, 3, 4, 5 ) );
  QCOMPARE( l8.numPoints(), 2 );
  QVERIFY( l8.is3D() );
  QVERIFY( l8.isMeasure() );
  QCOMPARE( l8.wkbType(), QgsWkbTypes::LineStringZM );

  //setPoints with MIXED dimensionality of points
  l8.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 1, 2, 4, 5 ) << QgsPoint( QgsWkbTypes::PointM, 2, 3, 0, 5 ) );
  QCOMPARE( l8.numPoints(), 2 );
  QVERIFY( l8.is3D() );
  QVERIFY( l8.isMeasure() );
  QCOMPARE( l8.wkbType(), QgsWkbTypes::LineStringZM );

  //test point
  QCOMPARE( l8.pointN( 0 ), QgsPoint( QgsWkbTypes::PointZM, 1, 2, 4, 5 ) );
  QCOMPARE( l8.pointN( 1 ), QgsPoint( QgsWkbTypes::PointZM, 2, 3, 0, 5 ) );

  //out of range - just want no crash here
  QgsPoint bad = l8.pointN( -1 );
  bad = l8.pointN( 100 );

  //test getters/setters
  QgsLineString l9;
  l9.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 1, 2, 3, 4 )
                << QgsPoint( QgsWkbTypes::PointZM, 11, 12, 13, 14 )
                << QgsPoint( QgsWkbTypes::PointZM, 21, 22, 23, 24 ) );
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
  QVERIFY( qIsNaN( l9.zAt( -1 ) ) ); //out of range
  QVERIFY( qIsNaN( l9.zAt( 11 ) ) ); //out of range

  l9.setZAt( 0, 53.0 );
  QCOMPARE( l9.zAt( 0 ), 53.0 );
  l9.setZAt( 1, 63.0 );
  QCOMPARE( l9.zAt( 1 ), 63.0 );
  l9.setZAt( -1, 53.0 ); //out of range
  l9.setZAt( 11, 53.0 ); //out of range

  QCOMPARE( l9.mAt( 0 ), 4.0 );
  QCOMPARE( l9.mAt( 1 ), 14.0 );
  QCOMPARE( l9.mAt( 2 ), 24.0 );
  QVERIFY( qIsNaN( l9.mAt( -1 ) ) ); //out of range
  QVERIFY( qIsNaN( l9.mAt( 11 ) ) ); //out of range

  l9.setMAt( 0, 54.0 );
  QCOMPARE( l9.mAt( 0 ), 54.0 );
  l9.setMAt( 1, 64.0 );
  QCOMPARE( l9.mAt( 1 ), 64.0 );
  l9.setMAt( -1, 54.0 ); //out of range
  l9.setMAt( 11, 54.0 ); //out of range

  //check zAt/setZAt with non-3d linestring
  l9.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointM, 1, 2, 0, 4 )
                << QgsPoint( QgsWkbTypes::PointM, 11, 12, 0, 14 )
                << QgsPoint( QgsWkbTypes::PointM, 21, 22, 0, 24 ) );

  //basically we just don't want these to crash
  QVERIFY( qIsNaN( l9.zAt( 0 ) ) );
  QVERIFY( qIsNaN( l9.zAt( 1 ) ) );
  l9.setZAt( 0, 53.0 );
  l9.setZAt( 1, 63.0 );

  //check mAt/setMAt with non-measure linestring
  l9.setPoints( QgsPointSequence() << QgsPoint( 1, 2 )
                << QgsPoint( 11, 12 )
                << QgsPoint( 21, 22 ) );

  //basically we just don't want these to crash
  QVERIFY( qIsNaN( l9.mAt( 0 ) ) );
  QVERIFY( qIsNaN( l9.mAt( 1 ) ) );
  l9.setMAt( 0, 53.0 );
  l9.setMAt( 1, 63.0 );

  //append linestring

  //append to empty
  QgsLineString l10;
  l10.append( 0 );
  QVERIFY( l10.isEmpty() );
  QCOMPARE( l10.numPoints(), 0 );

  std::unique_ptr<QgsLineString> toAppend( new QgsLineString() );
  toAppend->setPoints( QgsPointSequence() << QgsPoint( 1, 2 )
                       << QgsPoint( 11, 12 )
                       << QgsPoint( 21, 22 ) );
  l10.append( toAppend.get() );
  QVERIFY( !l10.is3D() );
  QVERIFY( !l10.isMeasure() );
  QCOMPARE( l10.numPoints(), 3 );
  QCOMPARE( l10.vertexCount(), 3 );
  QCOMPARE( l10.nCoordinates(), 3 );
  QCOMPARE( l10.ringCount(), 1 );
  QCOMPARE( l10.partCount(), 1 );
  QCOMPARE( l10.wkbType(), QgsWkbTypes::LineString );
  QCOMPARE( l10.pointN( 0 ), toAppend->pointN( 0 ) );
  QCOMPARE( l10.pointN( 1 ), toAppend->pointN( 1 ) );
  QCOMPARE( l10.pointN( 2 ), toAppend->pointN( 2 ) );

  //add more points
  toAppend.reset( new QgsLineString() );
  toAppend->setPoints( QgsPointSequence() << QgsPoint( 31, 32 )
                       << QgsPoint( 41, 42 )
                       << QgsPoint( 51, 52 ) );
  l10.append( toAppend.get() );
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
  toAppend.reset( new QgsLineString() );
  toAppend->setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 31, 32, 33, 34 )
                       << QgsPoint( QgsWkbTypes::PointZM, 41, 42, 43, 44 )
                       << QgsPoint( QgsWkbTypes::PointZM, 51, 52, 53, 54 ) );
  l10.append( toAppend.get() );
  QVERIFY( l10.is3D() );
  QVERIFY( l10.isMeasure() );
  QCOMPARE( l10.numPoints(), 3 );
  QCOMPARE( l10.ringCount(), 1 );
  QCOMPARE( l10.partCount(), 1 );
  QCOMPARE( l10.wkbType(), QgsWkbTypes::LineStringZM );
  QCOMPARE( l10.pointN( 0 ), toAppend->pointN( 0 ) );
  QCOMPARE( l10.pointN( 1 ), toAppend->pointN( 1 ) );
  QCOMPARE( l10.pointN( 2 ), toAppend->pointN( 2 ) );

  //append points with z to non z linestring
  l10.clear();
  l10.addVertex( QgsPoint( 1.0, 2.0 ) );
  QVERIFY( !l10.is3D() );
  QCOMPARE( l10.wkbType(), QgsWkbTypes::LineString );
  toAppend.reset( new QgsLineString() );
  toAppend->setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 31, 32, 33, 34 )
                       << QgsPoint( QgsWkbTypes::PointZM, 41, 42, 43, 44 )
                       << QgsPoint( QgsWkbTypes::PointZM, 51, 52, 53, 54 ) );
  l10.append( toAppend.get() );
  QCOMPARE( l10.wkbType(), QgsWkbTypes::LineString );
  QCOMPARE( l10.pointN( 0 ), QgsPoint( 1, 2 ) );
  QCOMPARE( l10.pointN( 1 ), QgsPoint( 31, 32 ) );
  QCOMPARE( l10.pointN( 2 ), QgsPoint( 41, 42 ) );
  QCOMPARE( l10.pointN( 3 ), QgsPoint( 51, 52 ) );

  //append points without z/m to linestring with z & m
  l10.clear();
  l10.addVertex( QgsPoint( QgsWkbTypes::PointZM, 1.0, 2.0, 3.0, 4.0 ) );
  QVERIFY( l10.is3D() );
  QVERIFY( l10.isMeasure() );
  QCOMPARE( l10.wkbType(), QgsWkbTypes::LineStringZM );
  toAppend.reset( new QgsLineString() );
  toAppend->setPoints( QgsPointSequence() << QgsPoint( 31, 32 )
                       << QgsPoint( 41, 42 )
                       << QgsPoint( 51, 52 ) );
  l10.append( toAppend.get() );
  QCOMPARE( l10.wkbType(), QgsWkbTypes::LineStringZM );
  QCOMPARE( l10.pointN( 0 ), QgsPoint( QgsWkbTypes::PointZM, 1, 2, 3, 4 ) );
  QCOMPARE( l10.pointN( 1 ), QgsPoint( QgsWkbTypes::PointZM, 31, 32 ) );
  QCOMPARE( l10.pointN( 2 ), QgsPoint( QgsWkbTypes::PointZM, 41, 42 ) );
  QCOMPARE( l10.pointN( 3 ), QgsPoint( QgsWkbTypes::PointZM, 51, 52 ) );

  //25d append
  l10.clear();
  toAppend.reset( new QgsLineString() );
  toAppend->setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::Point25D, 31, 32, 33 )
                       << QgsPoint( QgsWkbTypes::Point25D, 41, 42, 43 ) );
  l10.append( toAppend.get() );
  QVERIFY( l10.is3D() );
  QVERIFY( !l10.isMeasure() );
  QCOMPARE( l10.wkbType(), QgsWkbTypes::LineString25D );
  QCOMPARE( l10.pointN( 0 ), QgsPoint( QgsWkbTypes::Point25D, 31, 32, 33 ) );
  QCOMPARE( l10.pointN( 1 ), QgsPoint( QgsWkbTypes::Point25D, 41, 42, 43 ) );
  l10.clear();
  l10.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::Point25D, 11, 12, 33 ) );
  QCOMPARE( l10.wkbType(), QgsWkbTypes::LineString25D );
  l10.append( toAppend.get() );
  QVERIFY( l10.is3D() );
  QVERIFY( !l10.isMeasure() );
  QCOMPARE( l10.wkbType(), QgsWkbTypes::LineString25D );
  QCOMPARE( l10.pointN( 0 ), QgsPoint( QgsWkbTypes::Point25D, 11, 12, 33 ) );
  QCOMPARE( l10.pointN( 1 ), QgsPoint( QgsWkbTypes::Point25D, 31, 32, 33 ) );
  QCOMPARE( l10.pointN( 2 ), QgsPoint( QgsWkbTypes::Point25D, 41, 42, 43 ) );

  //append another line the closes the original geometry.
  //Make sure there are not duplicit points except start and end point
  l10.clear();
  toAppend.reset( new QgsLineString() );
  toAppend->setPoints( QgsPointSequence()
                       << QgsPoint( 1, 1 )
                       << QgsPoint( 5, 5 )
                       << QgsPoint( 10, 1 ) );
  l10.append( toAppend.get() );
  QCOMPARE( l10.numPoints(), 3 );
  QCOMPARE( l10.vertexCount(), 3 );
  toAppend.reset( new QgsLineString() );
  toAppend->setPoints( QgsPointSequence()
                       << QgsPoint( 10, 1 )
                       << QgsPoint( 1, 1 ) );
  l10.append( toAppend.get() );

  QVERIFY( l10.isClosed() );
  QCOMPARE( l10.numPoints(), 4 );
  QCOMPARE( l10.vertexCount(), 4 );

  //equality
  QgsLineString e1;
  QgsLineString e2;
  QVERIFY( e1 == e2 );
  QVERIFY( !( e1 != e2 ) );
  e1.addVertex( QgsPoint( 1, 2 ) );
  QVERIFY( !( e1 == e2 ) ); //different number of vertices
  QVERIFY( e1 != e2 );
  e2.addVertex( QgsPoint( 1, 2 ) );
  QVERIFY( e1 == e2 );
  QVERIFY( !( e1 != e2 ) );
  e1.addVertex( QgsPoint( 1 / 3.0, 4 / 3.0 ) );
  e2.addVertex( QgsPoint( 2 / 6.0, 8 / 6.0 ) );
  QVERIFY( e1 == e2 ); //check non-integer equality
  QVERIFY( !( e1 != e2 ) );
  e1.addVertex( QgsPoint( 7, 8 ) );
  e2.addVertex( QgsPoint( 6, 9 ) );
  QVERIFY( !( e1 == e2 ) ); //different coordinates
  QVERIFY( e1 != e2 );
  QgsLineString e3;
  e3.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZ, 1, 2, 0 )
                << QgsPoint( QgsWkbTypes::PointZ, 1 / 3.0, 4 / 3.0, 0 )
                << QgsPoint( QgsWkbTypes::PointZ, 7, 8, 0 ) );
  QVERIFY( !( e1 == e3 ) ); //different dimension
  QVERIFY( e1 != e3 );
  QgsLineString e4;
  e4.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZ, 1, 2, 2 )
                << QgsPoint( QgsWkbTypes::PointZ, 1 / 3.0, 4 / 3.0, 3 )
                << QgsPoint( QgsWkbTypes::PointZ, 7, 8, 4 ) );
  QVERIFY( !( e3 == e4 ) ); //different z coordinates
  QVERIFY( e3 != e4 );
  QgsLineString e5;
  e5.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointM, 1, 2, 0, 1 )
                << QgsPoint( QgsWkbTypes::PointM, 1 / 3.0, 4 / 3.0, 0, 2 )
                << QgsPoint( QgsWkbTypes::PointM, 7, 8, 0, 3 ) );
  QgsLineString e6;
  e6.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointM, 1, 2, 0, 11 )
                << QgsPoint( QgsWkbTypes::PointM, 1 / 3.0, 4 / 3.0, 0, 12 )
                << QgsPoint( QgsWkbTypes::PointM, 7, 8, 0, 13 ) );
  QVERIFY( !( e5 == e6 ) ); //different m values
  QVERIFY( e5 != e6 );

  //close/isClosed
  QgsLineString l11;
  QVERIFY( !l11.isClosed() );
  l11.setPoints( QgsPointSequence() << QgsPoint( 1, 2 )
                 << QgsPoint( 11, 2 )
                 << QgsPoint( 11, 22 )
                 << QgsPoint( 1, 22 ) );
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
  QCOMPARE( l11.pointN( 4 ), QgsPoint( 1, 2 ) );
  QCOMPARE( l11.area(), 0.0 );
  QCOMPARE( l11.perimeter(), 0.0 );
  //try closing already closed line, should be no change
  l11.close();
  QVERIFY( l11.isClosed() );
  QCOMPARE( l11.numPoints(), 5 );
  QCOMPARE( l11.pointN( 4 ), QgsPoint( 1, 2 ) );
  //test that m values aren't considered when testing for closedness
  l11.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointM, 1, 2, 0, 3 )
                 << QgsPoint( QgsWkbTypes::PointM, 11, 2, 0, 4 )
                 << QgsPoint( QgsWkbTypes::PointM, 11, 22, 0, 5 )
                 << QgsPoint( QgsWkbTypes::PointM, 1, 2, 0, 6 ) );
  QVERIFY( l11.isClosed() );

  //close with z and m
  QgsLineString l12;
  l12.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 1, 2, 3, 4 )
                 << QgsPoint( QgsWkbTypes::PointZM, 11, 2, 11, 14 )
                 << QgsPoint( QgsWkbTypes::PointZM, 11, 22, 21, 24 )
                 << QgsPoint( QgsWkbTypes::PointZM, 1, 22, 31, 34 ) );
  l12.close();
  QCOMPARE( l12.pointN( 4 ), QgsPoint( QgsWkbTypes::PointZM, 1, 2, 3, 4 ) );


  //polygonf
  QgsLineString l13;
  l13.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 1, 2, 3, 4 )
                 << QgsPoint( QgsWkbTypes::PointZM, 11, 2, 11, 14 )
                 << QgsPoint( QgsWkbTypes::PointZM, 11, 22, 21, 24 )
                 << QgsPoint( QgsWkbTypes::PointZM, 1, 22, 31, 34 ) );

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
  QgsLineString l14;
  l14.setPoints( QgsPointSequence() << QgsPoint( 1, 2 )
                 << QgsPoint( 11, 2 )
                 << QgsPoint( 11, 22 )
                 << QgsPoint( 1, 22 ) );
  std::unique_ptr<QgsLineString> cloned( l14.clone() );
  QCOMPARE( cloned->numPoints(), 4 );
  QCOMPARE( cloned->vertexCount(), 4 );
  QCOMPARE( cloned->ringCount(), 1 );
  QCOMPARE( cloned->partCount(), 1 );
  QCOMPARE( cloned->wkbType(), QgsWkbTypes::LineString );
  QVERIFY( !cloned->is3D() );
  QVERIFY( !cloned->isMeasure() );
  QCOMPARE( cloned->pointN( 0 ), l14.pointN( 0 ) );
  QCOMPARE( cloned->pointN( 1 ), l14.pointN( 1 ) );
  QCOMPARE( cloned->pointN( 2 ), l14.pointN( 2 ) );
  QCOMPARE( cloned->pointN( 3 ), l14.pointN( 3 ) );
  std::unique_ptr< QgsLineString > segmentized( static_cast< QgsLineString * >( l14.segmentize() ) );
  QCOMPARE( segmentized->numPoints(), 4 );
  QCOMPARE( segmentized->wkbType(), QgsWkbTypes::LineString );
  QVERIFY( !segmentized->is3D() );
  QVERIFY( !segmentized->isMeasure() );
  QCOMPARE( segmentized->pointN( 0 ), l14.pointN( 0 ) );
  QCOMPARE( segmentized->pointN( 1 ), l14.pointN( 1 ) );
  QCOMPARE( segmentized->pointN( 2 ), l14.pointN( 2 ) );
  QCOMPARE( segmentized->pointN( 3 ), l14.pointN( 3 ) );

  //clone with Z/M
  l14.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 1, 2, 3, 4 )
                 << QgsPoint( QgsWkbTypes::PointZM, 11, 2, 11, 14 )
                 << QgsPoint( QgsWkbTypes::PointZM, 11, 22, 21, 24 )
                 << QgsPoint( QgsWkbTypes::PointZM, 1, 22, 31, 34 ) );
  cloned.reset( l14.clone() );
  QCOMPARE( cloned->numPoints(), 4 );
  QCOMPARE( cloned->wkbType(), QgsWkbTypes::LineStringZM );
  QVERIFY( cloned->is3D() );
  QVERIFY( cloned->isMeasure() );
  QCOMPARE( cloned->pointN( 0 ), l14.pointN( 0 ) );
  QCOMPARE( cloned->pointN( 1 ), l14.pointN( 1 ) );
  QCOMPARE( cloned->pointN( 2 ), l14.pointN( 2 ) );
  QCOMPARE( cloned->pointN( 3 ), l14.pointN( 3 ) );
  segmentized.reset( static_cast< QgsLineString * >( l14.segmentize() ) );
  QCOMPARE( segmentized->numPoints(), 4 );
  QCOMPARE( segmentized->wkbType(), QgsWkbTypes::LineStringZM );
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
  QCOMPARE( cloned->wkbType(), QgsWkbTypes::LineString );
  segmentized.reset( static_cast< QgsLineString * >( l14.segmentize() ) );
  QVERIFY( segmentized->isEmpty() );
  QCOMPARE( segmentized->numPoints(), 0 );
  QVERIFY( !segmentized->is3D() );
  QVERIFY( !segmentized->isMeasure() );
  QCOMPARE( segmentized->wkbType(), QgsWkbTypes::LineString );

  //to/from WKB
  QgsLineString l15;
  l15.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 1, 2, 3, 4 )
                 << QgsPoint( QgsWkbTypes::PointZM, 11, 2, 11, 14 )
                 << QgsPoint( QgsWkbTypes::PointZM, 11, 22, 21, 24 )
                 << QgsPoint( QgsWkbTypes::PointZM, 1, 22, 31, 34 ) );
  QByteArray wkb15 = l15.asWkb();
  QgsLineString l16;
  QgsConstWkbPtr wkb15ptr( wkb15 );
  l16.fromWkb( wkb15ptr );
  QCOMPARE( l16.numPoints(), 4 );
  QCOMPARE( l16.vertexCount(), 4 );
  QCOMPARE( l16.nCoordinates(), 4 );
  QCOMPARE( l16.ringCount(), 1 );
  QCOMPARE( l16.partCount(), 1 );
  QCOMPARE( l16.wkbType(), QgsWkbTypes::LineStringZM );
  QVERIFY( l16.is3D() );
  QVERIFY( l16.isMeasure() );
  QCOMPARE( l16.pointN( 0 ), l15.pointN( 0 ) );
  QCOMPARE( l16.pointN( 1 ), l15.pointN( 1 ) );
  QCOMPARE( l16.pointN( 2 ), l15.pointN( 2 ) );
  QCOMPARE( l16.pointN( 3 ), l15.pointN( 3 ) );

  //bad WKB - check for no crash
  l16.clear();
  QgsConstWkbPtr nullPtr( nullptr, 0 );
  QVERIFY( !l16.fromWkb( nullPtr ) );
  QCOMPARE( l16.wkbType(), QgsWkbTypes::LineString );
  QgsPoint point( 1, 2 );
  QByteArray wkb16 = point.asWkb();
  QgsConstWkbPtr wkb16ptr( wkb16 );
  QVERIFY( !l16.fromWkb( wkb16ptr ) );
  QCOMPARE( l16.wkbType(), QgsWkbTypes::LineString );

  //to/from WKT
  QgsLineString l17;
  l17.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 1, 2, 3, 4 )
                 << QgsPoint( QgsWkbTypes::PointZM, 11, 2, 11, 14 )
                 << QgsPoint( QgsWkbTypes::PointZM, 11, 22, 21, 24 )
                 << QgsPoint( QgsWkbTypes::PointZM, 1, 22, 31, 34 ) );

  QString wkt = l17.asWkt();
  QVERIFY( !wkt.isEmpty() );
  QgsLineString l18;
  QVERIFY( l18.fromWkt( wkt ) );
  QCOMPARE( l18.numPoints(), 4 );
  QCOMPARE( l18.wkbType(), QgsWkbTypes::LineStringZM );
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
  QCOMPARE( l18.wkbType(), QgsWkbTypes::LineString );

  //asGML2
  QgsLineString exportLine;
  exportLine.setPoints( QgsPointSequence() << QgsPoint( 31, 32 )
                        << QgsPoint( 41, 42 )
                        << QgsPoint( 51, 52 ) );
  QgsLineString exportLineFloat;
  exportLineFloat.setPoints( QgsPointSequence() << QgsPoint( 1 / 3.0, 2 / 3.0 )
                             << QgsPoint( 1 + 1 / 3.0, 1 + 2 / 3.0 )
                             << QgsPoint( 2 + 1 / 3.0, 2 + 2 / 3.0 ) );
  QDomDocument doc( QStringLiteral( "gml" ) );
  QString expectedGML2( QStringLiteral( "<LineString xmlns=\"gml\"><coordinates xmlns=\"gml\" cs=\",\" ts=\" \">31,32 41,42 51,52</coordinates></LineString>" ) );
  QCOMPARE( elemToString( exportLine.asGML2( doc ) ), expectedGML2 );
  QString expectedGML2prec3( QStringLiteral( "<LineString xmlns=\"gml\"><coordinates xmlns=\"gml\" cs=\",\" ts=\" \">0.333,0.667 1.333,1.667 2.333,2.667</coordinates></LineString>" ) );
  QCOMPARE( elemToString( exportLineFloat.asGML2( doc, 3 ) ), expectedGML2prec3 );

  //asGML3
  QString expectedGML3( QStringLiteral( "<LineString xmlns=\"gml\"><posList xmlns=\"gml\" srsDimension=\"2\">31 32 41 42 51 52</posList></LineString>" ) );
  QCOMPARE( elemToString( exportLine.asGML3( doc ) ), expectedGML3 );
  QString expectedGML3prec3( QStringLiteral( "<LineString xmlns=\"gml\"><posList xmlns=\"gml\" srsDimension=\"2\">0.333 0.667 1.333 1.667 2.333 2.667</posList></LineString>" ) );
  QCOMPARE( elemToString( exportLineFloat.asGML3( doc, 3 ) ), expectedGML3prec3 );

  //asJSON
  QString expectedJson( QStringLiteral( "{\"type\": \"LineString\", \"coordinates\": [ [31, 32], [41, 42], [51, 52]]}" ) );
  QCOMPARE( exportLine.asJSON(), expectedJson );
  QString expectedJsonPrec3( QStringLiteral( "{\"type\": \"LineString\", \"coordinates\": [ [0.333, 0.667], [1.333, 1.667], [2.333, 2.667]]}" ) );
  QCOMPARE( exportLineFloat.asJSON( 3 ), expectedJsonPrec3 );

  //length
  QgsLineString l19;
  QCOMPARE( l19.length(), 0.0 );
  l19.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 1, 1, 2, 3 )
                 << QgsPoint( QgsWkbTypes::PointZM, 1, 10, 4, 5 )
                 << QgsPoint( QgsWkbTypes::PointZM, 15, 10, 6, 7 ) );
  QCOMPARE( l19.length(), 23.0 );

  //startPoint
  QCOMPARE( l19.startPoint(), QgsPoint( QgsWkbTypes::PointZM, 1, 1, 2, 3 ) );

  //endPoint
  QCOMPARE( l19.endPoint(), QgsPoint( QgsWkbTypes::PointZM, 15, 10, 6, 7 ) );

  //bad start/end points. Test that this doesn't crash.
  l19.clear();
  QCOMPARE( l19.startPoint(), QgsPoint() );
  QCOMPARE( l19.endPoint(), QgsPoint() );

  //curveToLine - no segmentation required, so should return a clone
  l19.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 1, 1, 2, 3 )
                 << QgsPoint( QgsWkbTypes::PointZM, 1, 10, 4, 5 )
                 << QgsPoint( QgsWkbTypes::PointZM, 15, 10, 6, 7 ) );
  segmentized.reset( l19.curveToLine() );
  QCOMPARE( segmentized->numPoints(), 3 );
  QCOMPARE( segmentized->wkbType(), QgsWkbTypes::LineStringZM );
  QVERIFY( segmentized->is3D() );
  QVERIFY( segmentized->isMeasure() );
  QCOMPARE( segmentized->pointN( 0 ), l19.pointN( 0 ) );
  QCOMPARE( segmentized->pointN( 1 ), l19.pointN( 1 ) );
  QCOMPARE( segmentized->pointN( 2 ), l19.pointN( 2 ) );

  // points
  QgsLineString l20;
  QgsPointSequence points;
  l20.points( points );
  QVERIFY( l20.isEmpty() );
  l20.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 1, 1, 2, 3 )
                 << QgsPoint( QgsWkbTypes::PointZM, 1, 10, 4, 5 )
                 << QgsPoint( QgsWkbTypes::PointZM, 15, 10, 6, 7 ) );
  l20.points( points );
  QCOMPARE( points.count(), 3 );
  QCOMPARE( points.at( 0 ), QgsPoint( QgsWkbTypes::PointZM, 1, 1, 2, 3 ) );
  QCOMPARE( points.at( 1 ), QgsPoint( QgsWkbTypes::PointZM, 1, 10, 4, 5 ) );
  QCOMPARE( points.at( 2 ), QgsPoint( QgsWkbTypes::PointZM, 15, 10, 6, 7 ) );

  //CRS transform
  QgsCoordinateReferenceSystem sourceSrs;
  sourceSrs.createFromSrid( 3994 );
  QgsCoordinateReferenceSystem destSrs;
  destSrs.createFromSrid( 4202 ); // want a transform with ellipsoid change
  QgsCoordinateTransform tr( sourceSrs, destSrs );

  // 2d CRS transform
  QgsLineString l21;
  l21.setPoints( QgsPointSequence() << QgsPoint( 6374985, -3626584 )
                 << QgsPoint( 6474985, -3526584 ) );
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
  QgsLineString l22;
  l22.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 6374985, -3626584, 1, 2 )
                 << QgsPoint( QgsWkbTypes::PointZM, 6474985, -3526584, 3, 4 ) );
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
  QgsLineString l23;
  l23.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 1, 2, 3, 4 )
                 << QgsPoint( QgsWkbTypes::PointZM, 11, 12, 13, 14 ) );
  l23.transform( qtr );
  QCOMPARE( l23.pointN( 0 ), QgsPoint( QgsWkbTypes::PointZM, 2, 6, 3, 4 ) );
  QCOMPARE( l23.pointN( 1 ), QgsPoint( QgsWkbTypes::PointZM, 22, 36, 13, 14 ) );
  QCOMPARE( l23.boundingBox(), QgsRectangle( 2, 6, 22, 36 ) );

  //insert vertex

  //insert vertex in empty line
  QgsLineString l24;
  QVERIFY( l24.insertVertex( QgsVertexId( 0, 0, 0 ), QgsPoint( 6.0, 7.0 ) ) );
  QCOMPARE( l24.numPoints(), 1 );
  QVERIFY( !l24.is3D() );
  QVERIFY( !l24.isMeasure() );
  QCOMPARE( l24.wkbType(), QgsWkbTypes::LineString );
  QCOMPARE( l24.pointN( 0 ), QgsPoint( 6.0, 7.0 ) );

  //insert 4d vertex in empty line, should set line to 4d
  l24.clear();
  QVERIFY( l24.insertVertex( QgsVertexId( 0, 0, 0 ), QgsPoint( QgsWkbTypes::PointZM, 6.0, 7.0, 1.0, 2.0 ) ) );
  QCOMPARE( l24.numPoints(), 1 );
  QVERIFY( l24.is3D() );
  QVERIFY( l24.isMeasure() );
  QCOMPARE( l24.wkbType(), QgsWkbTypes::LineStringZM );
  QCOMPARE( l24.pointN( 0 ), QgsPoint( QgsWkbTypes::PointZM, 6.0, 7.0, 1.0, 2.0 ) );

  //2d line
  l24.setPoints( QgsPointSequence() << QgsPoint( 1, 2 )
                 << QgsPoint( 11, 12 ) << QgsPoint( 21, 22 ) );
  QVERIFY( l24.insertVertex( QgsVertexId( 0, 0, 0 ), QgsPoint( 6.0, 7.0 ) ) );
  QCOMPARE( l24.numPoints(), 4 );
  QVERIFY( !l24.is3D() );
  QVERIFY( !l24.isMeasure() );
  QCOMPARE( l24.wkbType(), QgsWkbTypes::LineString );
  QVERIFY( l24.insertVertex( QgsVertexId( 0, 0, 1 ), QgsPoint( 8.0, 9.0 ) ) );
  QVERIFY( l24.insertVertex( QgsVertexId( 0, 0, 2 ), QgsPoint( 18.0, 19.0 ) ) );
  QCOMPARE( l24.pointN( 0 ), QgsPoint( 6.0, 7.0 ) );
  QCOMPARE( l24.pointN( 1 ), QgsPoint( 8.0, 9.0 ) );
  QCOMPARE( l24.pointN( 2 ), QgsPoint( 18.0, 19.0 ) );
  QCOMPARE( l24.pointN( 3 ), QgsPoint( 1.0, 2.0 ) );
  QCOMPARE( l24.pointN( 4 ), QgsPoint( 11.0, 12.0 ) );
  QCOMPARE( l24.pointN( 5 ), QgsPoint( 21.0, 22.0 ) );
  //insert vertex at end
  QVERIFY( l24.insertVertex( QgsVertexId( 0, 0, 6 ), QgsPoint( 31.0, 32.0 ) ) );
  QCOMPARE( l24.pointN( 6 ), QgsPoint( 31.0, 32.0 ) );
  QCOMPARE( l24.numPoints(), 7 );

  //insert vertex past end
  QVERIFY( !l24.insertVertex( QgsVertexId( 0, 0, 8 ), QgsPoint( 41.0, 42.0 ) ) );
  QCOMPARE( l24.numPoints(), 7 );

  //insert vertex before start
  QVERIFY( !l24.insertVertex( QgsVertexId( 0, 0, -18 ), QgsPoint( 41.0, 42.0 ) ) );
  QCOMPARE( l24.numPoints(), 7 );

  //insert 4d vertex in 4d line
  l24.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 1, 1, 2, 3 )
                 << QgsPoint( QgsWkbTypes::PointZM, 1, 10, 4, 5 )
                 << QgsPoint( QgsWkbTypes::PointZM, 15, 10, 6, 7 ) );
  QVERIFY( l24.insertVertex( QgsVertexId( 0, 0, 0 ), QgsPoint( QgsWkbTypes::PointZM, 11, 12, 13, 14 ) ) );
  QCOMPARE( l24.numPoints(), 4 );
  QCOMPARE( l24.pointN( 0 ), QgsPoint( QgsWkbTypes::PointZM, 11, 12, 13, 14 ) );

  //insert 2d vertex in 4d line
  QVERIFY( l24.insertVertex( QgsVertexId( 0, 0, 1 ), QgsPoint( 101, 102 ) ) );
  QCOMPARE( l24.numPoints(), 5 );
  QCOMPARE( l24.wkbType(), QgsWkbTypes::LineStringZM );
  QCOMPARE( l24.pointN( 1 ), QgsPoint( QgsWkbTypes::PointZM, 101, 102 ) );

  //insert 4d vertex in 2d line
  l24.setPoints( QgsPointSequence() << QgsPoint( 1, 2 )
                 << QgsPoint( 11, 12 ) << QgsPoint( 21, 22 ) );
  QVERIFY( l24.insertVertex( QgsVertexId( 0, 0, 0 ), QgsPoint( QgsWkbTypes::PointZM, 101, 102, 103, 104 ) ) );
  QCOMPARE( l24.numPoints(), 4 );
  QCOMPARE( l24.wkbType(), QgsWkbTypes::LineString );
  QCOMPARE( l24.pointN( 0 ), QgsPoint( QgsWkbTypes::Point, 101, 102 ) );

  //insert first vertex as Point25D
  l24.clear();
  QVERIFY( l24.insertVertex( QgsVertexId( 0, 0, 0 ), QgsPoint( QgsWkbTypes::Point25D, 101, 102, 103 ) ) );
  QCOMPARE( l24.wkbType(), QgsWkbTypes::LineString25D );
  QCOMPARE( l24.pointN( 0 ), QgsPoint( QgsWkbTypes::Point25D, 101, 102, 103 ) );

  //move vertex

  //empty line
  QgsLineString l25;
  QVERIFY( !l25.moveVertex( QgsVertexId( 0, 0, 0 ), QgsPoint( 6.0, 7.0 ) ) );
  QVERIFY( l25.isEmpty() );

  //valid line
  l25.setPoints( QgsPointSequence() << QgsPoint( 1, 2 )
                 << QgsPoint( 11, 12 ) << QgsPoint( 21, 22 ) );
  QVERIFY( l25.moveVertex( QgsVertexId( 0, 0, 0 ), QgsPoint( 6.0, 7.0 ) ) );
  QVERIFY( l25.moveVertex( QgsVertexId( 0, 0, 1 ), QgsPoint( 16.0, 17.0 ) ) );
  QVERIFY( l25.moveVertex( QgsVertexId( 0, 0, 2 ), QgsPoint( 26.0, 27.0 ) ) );
  QCOMPARE( l25.pointN( 0 ), QgsPoint( 6.0, 7.0 ) );
  QCOMPARE( l25.pointN( 1 ), QgsPoint( 16.0, 17.0 ) );
  QCOMPARE( l25.pointN( 2 ), QgsPoint( 26.0, 27.0 ) );

  //out of range
  QVERIFY( !l25.moveVertex( QgsVertexId( 0, 0, -1 ), QgsPoint( 3.0, 4.0 ) ) );
  QVERIFY( !l25.moveVertex( QgsVertexId( 0, 0, 10 ), QgsPoint( 3.0, 4.0 ) ) );
  QCOMPARE( l25.pointN( 0 ), QgsPoint( 6.0, 7.0 ) );
  QCOMPARE( l25.pointN( 1 ), QgsPoint( 16.0, 17.0 ) );
  QCOMPARE( l25.pointN( 2 ), QgsPoint( 26.0, 27.0 ) );

  //move 4d point in 4d line
  l25.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 1, 1, 2, 3 )
                 << QgsPoint( QgsWkbTypes::PointZM, 1, 10, 4, 5 )
                 << QgsPoint( QgsWkbTypes::PointZM, 15, 10, 6, 7 ) );
  QVERIFY( l25.moveVertex( QgsVertexId( 0, 0, 1 ), QgsPoint( QgsWkbTypes::PointZM, 6, 7, 12, 13 ) ) );
  QCOMPARE( l25.pointN( 1 ), QgsPoint( QgsWkbTypes::PointZM, 6, 7, 12, 13 ) );

  //move 2d point in 4d line, existing z/m should be maintained
  QVERIFY( l25.moveVertex( QgsVertexId( 0, 0, 1 ), QgsPoint( 34, 35 ) ) );
  QCOMPARE( l25.pointN( 1 ), QgsPoint( QgsWkbTypes::PointZM, 34, 35, 12, 13 ) );

  //move 4d point in 2d line
  l25.setPoints( QgsPointSequence() << QgsPoint( 1, 2 )
                 << QgsPoint( 11, 12 ) << QgsPoint( 21, 22 ) );
  QVERIFY( l25.moveVertex( QgsVertexId( 0, 0, 0 ), QgsPoint( QgsWkbTypes::PointZM, 3, 4, 2, 3 ) ) );
  QCOMPARE( l25.pointN( 0 ), QgsPoint( 3, 4 ) );


  //delete vertex

  //empty line
  QgsLineString l26;
  QVERIFY( !l26.deleteVertex( QgsVertexId( 0, 0, 0 ) ) );
  QVERIFY( l26.isEmpty() );

  //valid line
  l26.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 1, 2, 2, 3 )
                 << QgsPoint( QgsWkbTypes::PointZM, 11, 12, 4, 5 )
                 << QgsPoint( QgsWkbTypes::PointZM, 21, 22, 6, 7 ) );
  //out of range vertices
  QVERIFY( !l26.deleteVertex( QgsVertexId( 0, 0, -1 ) ) );
  QVERIFY( !l26.deleteVertex( QgsVertexId( 0, 0, 100 ) ) );

  //valid vertices
  QVERIFY( l26.deleteVertex( QgsVertexId( 0, 0, 1 ) ) );
  QCOMPARE( l26.numPoints(), 2 );
  QCOMPARE( l26.pointN( 0 ), QgsPoint( QgsWkbTypes::PointZM, 1, 2, 2, 3 ) );
  QCOMPARE( l26.pointN( 1 ), QgsPoint( QgsWkbTypes::PointZM, 21, 22, 6, 7 ) );
  //removing the second to last vertex removes both remaining vertices
  QVERIFY( l26.deleteVertex( QgsVertexId( 0, 0, 0 ) ) );
  QCOMPARE( l26.numPoints(), 0 );
  QVERIFY( !l26.deleteVertex( QgsVertexId( 0, 0, 0 ) ) );
  QVERIFY( l26.isEmpty() );

  //reversed
  QgsLineString l27;
  std::unique_ptr< QgsLineString > reversed( l27.reversed() );
  QVERIFY( reversed->isEmpty() );
  l27.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 1, 2, 2, 3 )
                 << QgsPoint( QgsWkbTypes::PointZM, 11, 12, 4, 5 )
                 << QgsPoint( QgsWkbTypes::PointZM, 21, 22, 6, 7 ) );
  reversed.reset( l27.reversed() );
  QCOMPARE( reversed->numPoints(), 3 );
  QCOMPARE( reversed->wkbType(), QgsWkbTypes::LineStringZM );
  QVERIFY( reversed->is3D() );
  QVERIFY( reversed->isMeasure() );
  QCOMPARE( reversed->pointN( 0 ), QgsPoint( QgsWkbTypes::PointZM, 21, 22, 6, 7 ) );
  QCOMPARE( reversed->pointN( 1 ), QgsPoint( QgsWkbTypes::PointZM, 11, 12, 4, 5 ) );
  QCOMPARE( reversed->pointN( 2 ), QgsPoint( QgsWkbTypes::PointZM, 1, 2, 2, 3 ) );

  //addZValue

  QgsLineString l28;
  QCOMPARE( l28.wkbType(), QgsWkbTypes::LineString );
  QVERIFY( l28.addZValue() );
  QCOMPARE( l28.wkbType(), QgsWkbTypes::LineStringZ );
  l28.clear();
  QVERIFY( l28.addZValue() );
  QCOMPARE( l28.wkbType(), QgsWkbTypes::LineStringZ );
  //2d line
  l28.setPoints( QgsPointSequence() << QgsPoint( 1, 2 ) << QgsPoint( 11, 12 ) );
  QVERIFY( l28.addZValue( 2 ) );
  QVERIFY( l28.is3D() );
  QCOMPARE( l28.wkbType(), QgsWkbTypes::LineStringZ );
  QCOMPARE( l28.pointN( 0 ), QgsPoint( QgsWkbTypes::PointZ, 1, 2, 2 ) );
  QCOMPARE( l28.pointN( 1 ), QgsPoint( QgsWkbTypes::PointZ, 11, 12, 2 ) );
  QVERIFY( !l28.addZValue( 4 ) ); //already has z value, test that existing z is unchanged
  QCOMPARE( l28.pointN( 0 ), QgsPoint( QgsWkbTypes::PointZ, 1, 2, 2 ) );
  QCOMPARE( l28.pointN( 1 ), QgsPoint( QgsWkbTypes::PointZ, 11, 12, 2 ) );
  //linestring with m
  l28.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointM, 1, 2, 0, 3 ) << QgsPoint( QgsWkbTypes::PointM, 11, 12, 0, 4 ) );
  QVERIFY( l28.addZValue( 5 ) );
  QVERIFY( l28.is3D() );
  QVERIFY( l28.isMeasure() );
  QCOMPARE( l28.wkbType(), QgsWkbTypes::LineStringZM );
  QCOMPARE( l28.pointN( 0 ), QgsPoint( QgsWkbTypes::PointZM, 1, 2, 5, 3 ) );
  QCOMPARE( l28.pointN( 1 ), QgsPoint( QgsWkbTypes::PointZM, 11, 12, 5, 4 ) );
  //linestring25d
  l28.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::Point25D, 1, 2, 3 ) << QgsPoint( QgsWkbTypes::Point25D, 11, 12, 4 ) );
  QCOMPARE( l28.wkbType(), QgsWkbTypes::LineString25D );
  QVERIFY( !l28.addZValue( 5 ) );
  QCOMPARE( l28.wkbType(), QgsWkbTypes::LineString25D );
  QCOMPARE( l28.pointN( 0 ), QgsPoint( QgsWkbTypes::Point25D, 1, 2, 3 ) );
  QCOMPARE( l28.pointN( 1 ), QgsPoint( QgsWkbTypes::Point25D, 11, 12, 4 ) );

  //addMValue

  QgsLineString l29;
  QCOMPARE( l29.wkbType(), QgsWkbTypes::LineString );
  QVERIFY( l29.addMValue() );
  QCOMPARE( l29.wkbType(), QgsWkbTypes::LineStringM );
  l29.clear();
  QVERIFY( l29.addMValue() );
  QCOMPARE( l29.wkbType(), QgsWkbTypes::LineStringM );
  //2d line
  l29.setPoints( QgsPointSequence() << QgsPoint( 1, 2 ) << QgsPoint( 11, 12 ) );
  QVERIFY( l29.addMValue( 2 ) );
  QVERIFY( !l29.is3D() );
  QVERIFY( l29.isMeasure() );
  QCOMPARE( l29.wkbType(), QgsWkbTypes::LineStringM );
  QCOMPARE( l29.pointN( 0 ), QgsPoint( QgsWkbTypes::PointM, 1, 2, 0, 2 ) );
  QCOMPARE( l29.pointN( 1 ), QgsPoint( QgsWkbTypes::PointM, 11, 12, 0, 2 ) );
  QVERIFY( !l29.addMValue( 4 ) ); //already has m value, test that existing m is unchanged
  QCOMPARE( l29.pointN( 0 ), QgsPoint( QgsWkbTypes::PointM, 1, 2, 0, 2 ) );
  QCOMPARE( l29.pointN( 1 ), QgsPoint( QgsWkbTypes::PointM, 11, 12, 0, 2 ) );
  //linestring with z
  l29.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZ, 1, 2, 3 ) << QgsPoint( QgsWkbTypes::PointZ, 11, 12, 4 ) );
  QVERIFY( l29.addMValue( 5 ) );
  QVERIFY( l29.is3D() );
  QVERIFY( l29.isMeasure() );
  QCOMPARE( l29.wkbType(), QgsWkbTypes::LineStringZM );
  QCOMPARE( l29.pointN( 0 ), QgsPoint( QgsWkbTypes::PointZM, 1, 2, 3, 5 ) );
  QCOMPARE( l29.pointN( 1 ), QgsPoint( QgsWkbTypes::PointZM, 11, 12, 4, 5 ) );
  //linestring25d, should become LineStringZM
  l29.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::Point25D, 1, 2, 3 ) << QgsPoint( QgsWkbTypes::Point25D, 11, 12, 4 ) );
  QCOMPARE( l29.wkbType(), QgsWkbTypes::LineString25D );
  QVERIFY( l29.addMValue( 5 ) );
  QVERIFY( l29.is3D() );
  QVERIFY( l29.isMeasure() );
  QCOMPARE( l29.wkbType(), QgsWkbTypes::LineStringZM );
  QCOMPARE( l29.pointN( 0 ), QgsPoint( QgsWkbTypes::PointZM, 1, 2, 3, 5 ) );
  QCOMPARE( l29.pointN( 1 ), QgsPoint( QgsWkbTypes::PointZM, 11, 12, 4, 5 ) );


  //dropZValue
  QgsLineString l28d;
  QVERIFY( !l28d.dropZValue() );
  l28d.setPoints( QgsPointSequence() << QgsPoint( 1, 2 ) << QgsPoint( 11, 12 ) );
  QVERIFY( !l28d.dropZValue() );
  l28d.addZValue( 1.0 );
  QCOMPARE( l28d.wkbType(), QgsWkbTypes::LineStringZ );
  QVERIFY( l28d.is3D() );
  QVERIFY( l28d.dropZValue() );
  QVERIFY( !l28d.is3D() );
  QCOMPARE( l28d.wkbType(), QgsWkbTypes::LineString );
  QCOMPARE( l28d.pointN( 0 ), QgsPoint( QgsWkbTypes::Point, 1, 2 ) );
  QCOMPARE( l28d.pointN( 1 ), QgsPoint( QgsWkbTypes::Point, 11, 12 ) );
  QVERIFY( !l28d.dropZValue() ); //already dropped
  //linestring with m
  l28d.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 1, 2, 3, 4 ) << QgsPoint( QgsWkbTypes::PointZM, 11, 12, 3, 4 ) );
  QVERIFY( l28d.dropZValue() );
  QVERIFY( !l28d.is3D() );
  QVERIFY( l28d.isMeasure() );
  QCOMPARE( l28d.wkbType(), QgsWkbTypes::LineStringM );
  QCOMPARE( l28d.pointN( 0 ), QgsPoint( QgsWkbTypes::PointM, 1, 2, 0, 4 ) );
  QCOMPARE( l28d.pointN( 1 ), QgsPoint( QgsWkbTypes::PointM, 11, 12, 0, 4 ) );
  //linestring25d
  l28d.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::Point25D, 1, 2, 3 ) << QgsPoint( QgsWkbTypes::Point25D, 11, 12, 4 ) );
  QCOMPARE( l28d.wkbType(), QgsWkbTypes::LineString25D );
  QVERIFY( l28d.dropZValue() );
  QCOMPARE( l28d.wkbType(), QgsWkbTypes::LineString );
  QCOMPARE( l28d.pointN( 0 ), QgsPoint( QgsWkbTypes::Point, 1, 2 ) );
  QCOMPARE( l28d.pointN( 1 ), QgsPoint( QgsWkbTypes::Point, 11, 12 ) );

  //dropMValue
  l28d.setPoints( QgsPointSequence() << QgsPoint( 1, 2 ) << QgsPoint( 11, 12 ) );
  QVERIFY( !l28d.dropMValue() );
  l28d.addMValue( 1.0 );
  QCOMPARE( l28d.wkbType(), QgsWkbTypes::LineStringM );
  QVERIFY( l28d.isMeasure() );
  QVERIFY( l28d.dropMValue() );
  QVERIFY( !l28d.isMeasure() );
  QCOMPARE( l28d.wkbType(), QgsWkbTypes::LineString );
  QCOMPARE( l28d.pointN( 0 ), QgsPoint( QgsWkbTypes::Point, 1, 2 ) );
  QCOMPARE( l28d.pointN( 1 ), QgsPoint( QgsWkbTypes::Point, 11, 12 ) );
  QVERIFY( !l28d.dropMValue() ); //already dropped
  //linestring with z
  l28d.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 1, 2, 3, 4 ) << QgsPoint( QgsWkbTypes::PointZM, 11, 12, 3, 4 ) );
  QVERIFY( l28d.dropMValue() );
  QVERIFY( !l28d.isMeasure() );
  QVERIFY( l28d.is3D() );
  QCOMPARE( l28d.wkbType(), QgsWkbTypes::LineStringZ );
  QCOMPARE( l28d.pointN( 0 ), QgsPoint( QgsWkbTypes::PointZ, 1, 2, 3, 0 ) );
  QCOMPARE( l28d.pointN( 1 ), QgsPoint( QgsWkbTypes::PointZ, 11, 12, 3, 0 ) );

  //convertTo
  l28d.setPoints( QgsPointSequence() << QgsPoint( 1, 2 ) << QgsPoint( 11, 12 ) );
  QVERIFY( l28d.convertTo( QgsWkbTypes::LineString ) );
  QCOMPARE( l28d.wkbType(), QgsWkbTypes::LineString );
  QVERIFY( l28d.convertTo( QgsWkbTypes::LineStringZ ) );
  QCOMPARE( l28d.wkbType(), QgsWkbTypes::LineStringZ );
  QCOMPARE( l28d.pointN( 0 ), QgsPoint( QgsWkbTypes::PointZ, 1, 2 ) );
  l28d.setZAt( 0, 5.0 );
  QVERIFY( l28d.convertTo( QgsWkbTypes::LineString25D ) );
  QCOMPARE( l28d.wkbType(), QgsWkbTypes::LineString25D );
  QCOMPARE( l28d.pointN( 0 ), QgsPoint( QgsWkbTypes::Point25D, 1, 2, 5.0 ) );
  QVERIFY( l28d.convertTo( QgsWkbTypes::LineStringZM ) );
  QCOMPARE( l28d.wkbType(), QgsWkbTypes::LineStringZM );
  QCOMPARE( l28d.pointN( 0 ), QgsPoint( QgsWkbTypes::PointZM, 1, 2, 5.0 ) );
  l28d.setMAt( 0, 6.0 );
  QVERIFY( l28d.convertTo( QgsWkbTypes::LineStringM ) );
  QCOMPARE( l28d.wkbType(), QgsWkbTypes::LineStringM );
  QCOMPARE( l28d.pointN( 0 ), QgsPoint( QgsWkbTypes::PointM, 1, 2, 0.0, 6.0 ) );
  QVERIFY( l28d.convertTo( QgsWkbTypes::LineString ) );
  QCOMPARE( l28d.wkbType(), QgsWkbTypes::LineString );
  QCOMPARE( l28d.pointN( 0 ), QgsPoint( 1, 2 ) );
  QVERIFY( !l28d.convertTo( QgsWkbTypes::Polygon ) );

  //isRing
  QgsLineString l30;
  QVERIFY( !l30.isRing() );
  l30.setPoints( QgsPointSequence() << QgsPoint( 1, 2 ) << QgsPoint( 11, 12 ) << QgsPoint( 1, 2 ) );
  QVERIFY( !l30.isRing() ); //<4 points
  l30.setPoints( QgsPointSequence() << QgsPoint( 1, 2 ) << QgsPoint( 11, 12 ) << QgsPoint( 21, 22 ) << QgsPoint( 31, 32 ) );
  QVERIFY( !l30.isRing() ); //not closed
  l30.setPoints( QgsPointSequence() << QgsPoint( 1, 2 ) << QgsPoint( 11, 12 ) << QgsPoint( 21, 22 ) << QgsPoint( 1, 2 ) );
  QVERIFY( l30.isRing() );

  //coordinateSequence
  QgsLineString l31;
  QgsCoordinateSequence coords = l31.coordinateSequence();
  QCOMPARE( coords.count(), 1 );
  QCOMPARE( coords.at( 0 ).count(), 1 );
  QVERIFY( coords.at( 0 ).at( 0 ).isEmpty() );
  l31.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 1, 2, 2, 3 )
                 << QgsPoint( QgsWkbTypes::PointZM, 11, 12, 4, 5 )
                 << QgsPoint( QgsWkbTypes::PointZM, 21, 22, 6, 7 ) );
  coords = l31.coordinateSequence();
  QCOMPARE( coords.count(), 1 );
  QCOMPARE( coords.at( 0 ).count(), 1 );
  QCOMPARE( coords.at( 0 ).at( 0 ).count(), 3 );
  QCOMPARE( coords.at( 0 ).at( 0 ).at( 0 ), QgsPoint( QgsWkbTypes::PointZM, 1, 2, 2, 3 ) );
  QCOMPARE( coords.at( 0 ).at( 0 ).at( 1 ), QgsPoint( QgsWkbTypes::PointZM, 11, 12, 4, 5 ) );
  QCOMPARE( coords.at( 0 ).at( 0 ).at( 2 ), QgsPoint( QgsWkbTypes::PointZM, 21, 22, 6, 7 ) );

  //nextVertex

  QgsLineString l32;
  QgsVertexId v;
  QgsPoint p;
  QVERIFY( !l32.nextVertex( v, p ) );
  v = QgsVertexId( 0, 0, -2 );
  QVERIFY( !l32.nextVertex( v, p ) );
  v = QgsVertexId( 0, 0, 10 );
  QVERIFY( !l32.nextVertex( v, p ) );
  //LineString
  l32.setPoints( QgsPointSequence() << QgsPoint( 1, 2 ) << QgsPoint( 11, 12 ) );
  v = QgsVertexId( 0, 0, 2 ); //out of range
  QVERIFY( !l32.nextVertex( v, p ) );
  v = QgsVertexId( 0, 0, -5 );
  QVERIFY( l32.nextVertex( v, p ) );
  v = QgsVertexId( 0, 0, -1 );
  QVERIFY( l32.nextVertex( v, p ) );
  QCOMPARE( v, QgsVertexId( 0, 0, 0 ) );
  QCOMPARE( p, QgsPoint( 1, 2 ) );
  QVERIFY( l32.nextVertex( v, p ) );
  QCOMPARE( v, QgsVertexId( 0, 0, 1 ) );
  QCOMPARE( p, QgsPoint( 11, 12 ) );
  QVERIFY( !l32.nextVertex( v, p ) );
  v = QgsVertexId( 0, 1, 0 );
  QVERIFY( l32.nextVertex( v, p ) );
  QCOMPARE( v, QgsVertexId( 0, 1, 1 ) ); //test that ring number is maintained
  QCOMPARE( p, QgsPoint( 11, 12 ) );
  v = QgsVertexId( 1, 0, 0 );
  QVERIFY( l32.nextVertex( v, p ) );
  QCOMPARE( v, QgsVertexId( 1, 0, 1 ) ); //test that part number is maintained
  QCOMPARE( p, QgsPoint( 11, 12 ) );

  //LineStringZ
  l32.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZ, 1, 2, 3 ) << QgsPoint( QgsWkbTypes::PointZ, 11, 12, 13 ) );
  v = QgsVertexId( 0, 0, -1 );
  QVERIFY( l32.nextVertex( v, p ) );
  QCOMPARE( v, QgsVertexId( 0, 0, 0 ) );
  QCOMPARE( p, QgsPoint( QgsWkbTypes::PointZ, 1, 2, 3 ) );
  QVERIFY( l32.nextVertex( v, p ) );
  QCOMPARE( v, QgsVertexId( 0, 0, 1 ) );
  QCOMPARE( p, QgsPoint( QgsWkbTypes::PointZ, 11, 12, 13 ) );
  QVERIFY( !l32.nextVertex( v, p ) );
  //LineStringM
  l32.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointM, 1, 2, 0, 4 ) << QgsPoint( QgsWkbTypes::PointM, 11, 12, 0, 14 ) );
  v = QgsVertexId( 0, 0, -1 );
  QVERIFY( l32.nextVertex( v, p ) );
  QCOMPARE( v, QgsVertexId( 0, 0, 0 ) );
  QCOMPARE( p, QgsPoint( QgsWkbTypes::PointM, 1, 2, 0, 4 ) );
  QVERIFY( l32.nextVertex( v, p ) );
  QCOMPARE( v, QgsVertexId( 0, 0, 1 ) );
  QCOMPARE( p, QgsPoint( QgsWkbTypes::PointM, 11, 12, 0, 14 ) );
  QVERIFY( !l32.nextVertex( v, p ) );
  //LineStringZM
  l32.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 1, 2, 3, 4 ) << QgsPoint( QgsWkbTypes::PointZM, 11, 12, 13, 14 ) );
  v = QgsVertexId( 0, 0, -1 );
  QVERIFY( l32.nextVertex( v, p ) );
  QCOMPARE( v, QgsVertexId( 0, 0, 0 ) );
  QCOMPARE( p, QgsPoint( QgsWkbTypes::PointZM, 1, 2, 3, 4 ) );
  QVERIFY( l32.nextVertex( v, p ) );
  QCOMPARE( v, QgsVertexId( 0, 0, 1 ) );
  QCOMPARE( p, QgsPoint( QgsWkbTypes::PointZM, 11, 12, 13, 14 ) );
  QVERIFY( !l32.nextVertex( v, p ) );
  //LineString25D
  l32.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::Point25D, 1, 2, 3 ) << QgsPoint( QgsWkbTypes::Point25D, 11, 12, 13 ) );
  v = QgsVertexId( 0, 0, -1 );
  QVERIFY( l32.nextVertex( v, p ) );
  QCOMPARE( v, QgsVertexId( 0, 0, 0 ) );
  QCOMPARE( p, QgsPoint( QgsWkbTypes::Point25D, 1, 2, 3 ) );
  QVERIFY( l32.nextVertex( v, p ) );
  QCOMPARE( v, QgsVertexId( 0, 0, 1 ) );
  QCOMPARE( p, QgsPoint( QgsWkbTypes::Point25D, 11, 12, 13 ) );
  QVERIFY( !l32.nextVertex( v, p ) );

  //vertexAt and pointAt
  QgsLineString l33;
  l33.vertexAt( QgsVertexId( 0, 0, -10 ) ); //out of bounds, check for no crash
  l33.vertexAt( QgsVertexId( 0, 0, 10 ) ); //out of bounds, check for no crash
  QgsVertexId::VertexType type;
  QVERIFY( !l33.pointAt( -10, p, type ) );
  QVERIFY( !l33.pointAt( 10, p, type ) );
  //LineString
  l33.setPoints( QgsPointSequence() << QgsPoint( 1, 2 ) << QgsPoint( 11, 12 ) );
  l33.vertexAt( QgsVertexId( 0, 0, -10 ) );
  l33.vertexAt( QgsVertexId( 0, 0, 10 ) ); //out of bounds, check for no crash
  QCOMPARE( l33.vertexAt( QgsVertexId( 0, 0, 0 ) ), QgsPoint( 1, 2 ) );
  QCOMPARE( l33.vertexAt( QgsVertexId( 0, 0, 1 ) ), QgsPoint( 11, 12 ) );
  QVERIFY( !l33.pointAt( -10, p, type ) );
  QVERIFY( !l33.pointAt( 10, p, type ) );
  QVERIFY( l33.pointAt( 0, p, type ) );
  QCOMPARE( p, QgsPoint( 1, 2 ) );
  QCOMPARE( type, QgsVertexId::SegmentVertex );
  QVERIFY( l33.pointAt( 1, p, type ) );
  QCOMPARE( p, QgsPoint( 11, 12 ) );
  QCOMPARE( type, QgsVertexId::SegmentVertex );
  //LineStringZ
  l33.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZ, 1, 2, 3 ) << QgsPoint( QgsWkbTypes::PointZ, 11, 12, 13 ) );
  QCOMPARE( l33.vertexAt( QgsVertexId( 0, 0, 0 ) ), QgsPoint( QgsWkbTypes::PointZ, 1, 2, 3 ) );
  QCOMPARE( l33.vertexAt( QgsVertexId( 0, 0, 1 ) ), QgsPoint( QgsWkbTypes::PointZ, 11, 12, 13 ) );
  QVERIFY( l33.pointAt( 0, p, type ) );
  QCOMPARE( p, QgsPoint( QgsWkbTypes::PointZ, 1, 2, 3 ) );
  QCOMPARE( type, QgsVertexId::SegmentVertex );
  QVERIFY( l33.pointAt( 1, p, type ) );
  QCOMPARE( p, QgsPoint( QgsWkbTypes::PointZ, 11, 12, 13 ) );
  QCOMPARE( type, QgsVertexId::SegmentVertex );
  //LineStringM
  l33.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointM, 1, 2, 0, 4 ) << QgsPoint( QgsWkbTypes::PointM, 11, 12, 0, 14 ) );
  QCOMPARE( l33.vertexAt( QgsVertexId( 0, 0, 0 ) ), QgsPoint( QgsWkbTypes::PointM, 1, 2, 0, 4 ) );
  QCOMPARE( l33.vertexAt( QgsVertexId( 0, 0, 1 ) ), QgsPoint( QgsWkbTypes::PointM, 11, 12, 0, 14 ) );
  QVERIFY( l33.pointAt( 0, p, type ) );
  QCOMPARE( p, QgsPoint( QgsWkbTypes::PointM, 1, 2, 0, 4 ) );
  QCOMPARE( type, QgsVertexId::SegmentVertex );
  QVERIFY( l33.pointAt( 1, p, type ) );
  QCOMPARE( p, QgsPoint( QgsWkbTypes::PointM, 11, 12, 0, 14 ) );
  QCOMPARE( type, QgsVertexId::SegmentVertex );
  //LineStringZM
  l33.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 1, 2, 3, 4 ) << QgsPoint( QgsWkbTypes::PointZM, 11, 12, 13, 14 ) );
  QCOMPARE( l33.vertexAt( QgsVertexId( 0, 0, 0 ) ), QgsPoint( QgsWkbTypes::PointZM, 1, 2, 3, 4 ) );
  QCOMPARE( l33.vertexAt( QgsVertexId( 0, 0, 1 ) ), QgsPoint( QgsWkbTypes::PointZM, 11, 12, 13, 14 ) );
  QVERIFY( l33.pointAt( 0, p, type ) );
  QCOMPARE( p, QgsPoint( QgsWkbTypes::PointZM, 1, 2, 3, 4 ) );
  QCOMPARE( type, QgsVertexId::SegmentVertex );
  QVERIFY( l33.pointAt( 1, p, type ) );
  QCOMPARE( p, QgsPoint( QgsWkbTypes::PointZM, 11, 12, 13, 14 ) );
  QCOMPARE( type, QgsVertexId::SegmentVertex );
  //LineString25D
  l33.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::Point25D, 1, 2, 3 ) << QgsPoint( QgsWkbTypes::Point25D, 11, 12, 13 ) );
  QCOMPARE( l33.vertexAt( QgsVertexId( 0, 0, 0 ) ), QgsPoint( QgsWkbTypes::Point25D, 1, 2, 3 ) );
  QCOMPARE( l33.vertexAt( QgsVertexId( 0, 0, 1 ) ), QgsPoint( QgsWkbTypes::Point25D, 11, 12, 13 ) );
  QVERIFY( l33.pointAt( 0, p, type ) );
  QCOMPARE( p, QgsPoint( QgsWkbTypes::Point25D, 1, 2, 3 ) );
  QCOMPARE( type, QgsVertexId::SegmentVertex );
  QVERIFY( l33.pointAt( 1, p, type ) );
  QCOMPARE( p, QgsPoint( QgsWkbTypes::Point25D, 11, 12, 13 ) );
  QCOMPARE( type, QgsVertexId::SegmentVertex );

  //centroid
  QgsLineString l34;
  QCOMPARE( l34.centroid(), QgsPoint() );
  l34.setPoints( QgsPointSequence() << QgsPoint( 5, 10 ) );
  QCOMPARE( l34.centroid(), QgsPoint( 5, 10 ) );
  l34.setPoints( QgsPointSequence() << QgsPoint( 0, 0 ) << QgsPoint( 20, 10 ) );
  QCOMPARE( l34.centroid(), QgsPoint( 10, 5 ) );
  l34.setPoints( QgsPointSequence() << QgsPoint( 0, 0 ) << QgsPoint( 0, 9 ) << QgsPoint( 2, 9 ) << QgsPoint( 2, 0 ) );
  QCOMPARE( l34.centroid(), QgsPoint( 1, 4.95 ) );
  //linestring with 0 length segment
  l34.setPoints( QgsPointSequence() << QgsPoint( 0, 0 ) << QgsPoint( 0, 9 ) << QgsPoint( 2, 9 ) << QgsPoint( 2, 9 ) << QgsPoint( 2, 0 ) );
  QCOMPARE( l34.centroid(), QgsPoint( 1, 4.95 ) );
  //linestring with 0 total length segment
  l34.setPoints( QgsPointSequence() << QgsPoint( 5, 4 ) << QgsPoint( 5, 4 ) << QgsPoint( 5, 4 ) );
  QCOMPARE( l34.centroid(), QgsPoint( 5, 4 ) );

  //closest segment
  QgsLineString l35;
  bool leftOf = false;
  p = QgsPoint(); // reset all coords to zero
  ( void )l35.closestSegment( QgsPoint( 1, 2 ), p, v, 0, 0 ); //empty line, just want no crash
  l35.setPoints( QgsPointSequence() << QgsPoint( 5, 10 ) );
  QVERIFY( l35.closestSegment( QgsPoint( 5, 10 ), p, v, 0, 0 ) < 0 );
  l35.setPoints( QgsPointSequence() << QgsPoint( 5, 10 ) << QgsPoint( 10, 10 ) );
  QVERIFY( qgsDoubleNear( l35.closestSegment( QgsPoint( 4, 11 ), p, v, &leftOf, 0 ), 2.0 ) );
  QCOMPARE( p, QgsPoint( 5, 10 ) );
  QCOMPARE( v, QgsVertexId( 0, 0, 1 ) );
  QCOMPARE( leftOf, true );
  QVERIFY( qgsDoubleNear( l35.closestSegment( QgsPoint( 8, 11 ), p, v, &leftOf, 0 ), 1.0 ) );
  QCOMPARE( p, QgsPoint( 8, 10 ) );
  QCOMPARE( v, QgsVertexId( 0, 0, 1 ) );
  QCOMPARE( leftOf, true );
  QVERIFY( qgsDoubleNear( l35.closestSegment( QgsPoint( 8, 9 ), p, v, &leftOf, 0 ), 1.0 ) );
  QCOMPARE( p, QgsPoint( 8, 10 ) );
  QCOMPARE( v, QgsVertexId( 0, 0, 1 ) );
  QCOMPARE( leftOf, false );
  QVERIFY( qgsDoubleNear( l35.closestSegment( QgsPoint( 11, 9 ), p, v, &leftOf, 0 ), 2.0 ) );
  QCOMPARE( p, QgsPoint( 10, 10 ) );
  QCOMPARE( v, QgsVertexId( 0, 0, 1 ) );
  QCOMPARE( leftOf, false );
  l35.setPoints( QgsPointSequence() << QgsPoint( 5, 10 )
                 << QgsPoint( 10, 10 )
                 << QgsPoint( 10, 15 ) );
  QVERIFY( qgsDoubleNear( l35.closestSegment( QgsPoint( 11, 12 ), p, v, &leftOf, 0 ), 1.0 ) );
  QCOMPARE( p, QgsPoint( 10, 12 ) );
  QCOMPARE( v, QgsVertexId( 0, 0, 2 ) );
  QCOMPARE( leftOf, false );

  //sumUpArea
  QgsLineString l36;
  double area = 1.0; //sumUpArea adds to area, so start with non-zero value
  l36.sumUpArea( area );
  QCOMPARE( area, 1.0 );
  l36.setPoints( QgsPointSequence() << QgsPoint( 5, 10 ) );
  l36.sumUpArea( area );
  QCOMPARE( area, 1.0 );
  l36.setPoints( QgsPointSequence() << QgsPoint( 5, 10 ) << QgsPoint( 10, 10 ) );
  l36.sumUpArea( area );
  QVERIFY( qgsDoubleNear( area, -24 ) );
  l36.setPoints( QgsPointSequence() << QgsPoint( 0, 0 ) << QgsPoint( 2, 0 ) << QgsPoint( 2, 2 ) );
  l36.sumUpArea( area );
  QVERIFY( qgsDoubleNear( area, -22 ) );
  l36.setPoints( QgsPointSequence() << QgsPoint( 0, 0 ) << QgsPoint( 2, 0 ) << QgsPoint( 2, 2 ) << QgsPoint( 0, 2 ) );
  l36.sumUpArea( area );
  QVERIFY( qgsDoubleNear( area, -18 ) );

  //boundingBox - test that bounding box is updated after every modification to the line string
  QgsLineString l37;
  QVERIFY( l37.boundingBox().isNull() );
  l37.setPoints( QgsPointSequence() << QgsPoint( 5, 10 ) << QgsPoint( 10, 15 ) );
  QCOMPARE( l37.boundingBox(), QgsRectangle( 5, 10, 10, 15 ) );
  l37.setPoints( QgsPointSequence() << QgsPoint( -5, -10 ) << QgsPoint( -6, -10 ) << QgsPoint( -5.5, -9 ) );
  QCOMPARE( l37.boundingBox(), QgsRectangle( -6, -10, -5, -9 ) );
  //setXAt
  l37.setXAt( 2, -4 );
  QCOMPARE( l37.boundingBox(), QgsRectangle( -6, -10, -4, -9 ) );
  //setYAt
  l37.setYAt( 1, -15 );
  QCOMPARE( l37.boundingBox(), QgsRectangle( -6, -15, -4, -9 ) );
  //append
  toAppend.reset( new QgsLineString() );
  toAppend->setPoints( QgsPointSequence() << QgsPoint( 1, 2 ) << QgsPoint( 4, 0 ) );
  l37.append( toAppend.get() );
  QCOMPARE( l37.boundingBox(), QgsRectangle( -6, -15, 4, 2 ) );
  l37.addVertex( QgsPoint( 6, 3 ) );
  QCOMPARE( l37.boundingBox(), QgsRectangle( -6, -15, 6, 3 ) );
  l37.clear();
  QVERIFY( l37.boundingBox().isNull() );
  l37.setPoints( QgsPointSequence() << QgsPoint( 5, 10 ) << QgsPoint( 10, 15 ) );
  QByteArray wkbToAppend = toAppend->asWkb();
  QgsConstWkbPtr wkbToAppendPtr( wkbToAppend );
  l37.fromWkb( wkbToAppendPtr );
  QCOMPARE( l37.boundingBox(), QgsRectangle( 1, 0, 4, 2 ) );
  l37.fromWkt( QStringLiteral( "LineString( 1 5, 3 4, 6 3 )" ) );
  QCOMPARE( l37.boundingBox(), QgsRectangle( 1, 3, 6, 5 ) );
  l37.insertVertex( QgsVertexId( 0, 0, 1 ), QgsPoint( -1, 7 ) );
  QCOMPARE( l37.boundingBox(), QgsRectangle( -1, 3, 6, 7 ) );
  l37.moveVertex( QgsVertexId( 0, 0, 1 ), QgsPoint( -3, 10 ) );
  QCOMPARE( l37.boundingBox(), QgsRectangle( -3, 3, 6, 10 ) );
  l37.deleteVertex( QgsVertexId( 0, 0, 1 ) );
  QCOMPARE( l37.boundingBox(), QgsRectangle( 1, 3, 6, 5 ) );

  //angle
  QgsLineString l38;
  ( void )l38.vertexAngle( QgsVertexId() ); //just want no crash
  ( void )l38.vertexAngle( QgsVertexId( 0, 0, 0 ) ); //just want no crash
  l38.setPoints( QgsPointSequence() << QgsPoint( 0, 0 ) );
  ( void )l38.vertexAngle( QgsVertexId( 0, 0, 0 ) ); //just want no crash, any answer is meaningless
  l38.setPoints( QgsPointSequence() << QgsPoint( 0, 0 ) << QgsPoint( 1, 0 ) );
  QVERIFY( qgsDoubleNear( l38.vertexAngle( QgsVertexId( 0, 0, 0 ) ), 1.5708, 0.0001 ) );
  QVERIFY( qgsDoubleNear( l38.vertexAngle( QgsVertexId( 0, 0, 1 ) ), 1.5708, 0.0001 ) );
  ( void )l38.vertexAngle( QgsVertexId( 0, 0, 2 ) ); //no crash
  l38.setPoints( QgsPointSequence() << QgsPoint( 0, 0 ) << QgsPoint( 0, 1 ) );
  QVERIFY( qgsDoubleNear( l38.vertexAngle( QgsVertexId( 0, 0, 0 ) ), 0.0 ) );
  QVERIFY( qgsDoubleNear( l38.vertexAngle( QgsVertexId( 0, 0, 1 ) ), 0.0 ) );
  l38.setPoints( QgsPointSequence() << QgsPoint( 1, 0 ) << QgsPoint( 0, 0 ) );
  QVERIFY( qgsDoubleNear( l38.vertexAngle( QgsVertexId( 0, 0, 0 ) ), 4.71239, 0.0001 ) );
  QVERIFY( qgsDoubleNear( l38.vertexAngle( QgsVertexId( 0, 0, 1 ) ), 4.71239, 0.0001 ) );
  l38.setPoints( QgsPointSequence() << QgsPoint( 0, 1 ) << QgsPoint( 0, 0 ) );
  QVERIFY( qgsDoubleNear( l38.vertexAngle( QgsVertexId( 0, 0, 0 ) ), 3.1416, 0.0001 ) );
  QVERIFY( qgsDoubleNear( l38.vertexAngle( QgsVertexId( 0, 0, 1 ) ), 3.1416, 0.0001 ) );
  l38.setPoints( QgsPointSequence() << QgsPoint( 0, 0 ) << QgsPoint( 1, 0 ) << QgsPoint( 1, 1 ) );
  QVERIFY( qgsDoubleNear( l38.vertexAngle( QgsVertexId( 0, 0, 0 ) ), 1.5708, 0.0001 ) );
  QVERIFY( qgsDoubleNear( l38.vertexAngle( QgsVertexId( 0, 0, 1 ) ), 0.7854, 0.0001 ) );
  QVERIFY( qgsDoubleNear( l38.vertexAngle( QgsVertexId( 0, 0, 2 ) ), 0.0, 0.0001 ) );
  l38.setPoints( QgsPointSequence() << QgsPoint( 0, 0 ) << QgsPoint( 0.5, 0 ) << QgsPoint( 1, 0 )
                 << QgsPoint( 2, 1 ) << QgsPoint( 1, 2 ) << QgsPoint( 0, 2 ) );
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
  QgsLineString l39;
  l39.setPoints( QList<QgsPoint>() << QgsPoint( 0, 0 ) << QgsPoint( 1, 1 ) );
  QVERIFY( l39.numPoints() == 2 );
  l39.deleteVertex( QgsVertexId( 0, 0, 1 ) );
  QVERIFY( l39.numPoints() == 0 );

  //boundary
  QgsLineString boundary1;
  QVERIFY( !boundary1.boundary() );
  boundary1.setPoints( QList<QgsPoint>() << QgsPoint( 0, 0 ) << QgsPoint( 1, 0 ) << QgsPoint( 1, 1 ) );
  QgsAbstractGeometry *boundary = boundary1.boundary();
  QgsMultiPointV2 *mpBoundary = dynamic_cast< QgsMultiPointV2 * >( boundary );
  QVERIFY( mpBoundary );
  QCOMPARE( static_cast< QgsPoint *>( mpBoundary->geometryN( 0 ) )->x(), 0.0 );
  QCOMPARE( static_cast< QgsPoint *>( mpBoundary->geometryN( 0 ) )->y(), 0.0 );
  QCOMPARE( static_cast< QgsPoint *>( mpBoundary->geometryN( 1 ) )->x(), 1.0 );
  QCOMPARE( static_cast< QgsPoint *>( mpBoundary->geometryN( 1 ) )->y(), 1.0 );
  delete boundary;

  // closed string = no boundary
  boundary1.setPoints( QList<QgsPoint>() << QgsPoint( 0, 0 ) << QgsPoint( 1, 0 ) << QgsPoint( 1, 1 ) << QgsPoint( 0, 0 ) );
  QVERIFY( !boundary1.boundary() );
  \

  //boundary with z
  boundary1.setPoints( QList<QgsPoint>() << QgsPoint( QgsWkbTypes::PointZ, 0, 0, 10 ) << QgsPoint( QgsWkbTypes::PointZ, 1, 0, 15 ) << QgsPoint( QgsWkbTypes::PointZ, 1, 1, 20 ) );
  boundary = boundary1.boundary();
  mpBoundary = dynamic_cast< QgsMultiPointV2 * >( boundary );
  QVERIFY( mpBoundary );
  QCOMPARE( mpBoundary->geometryN( 0 )->wkbType(), QgsWkbTypes::PointZ );
  QCOMPARE( static_cast< QgsPoint *>( mpBoundary->geometryN( 0 ) )->x(), 0.0 );
  QCOMPARE( static_cast< QgsPoint *>( mpBoundary->geometryN( 0 ) )->y(), 0.0 );
  QCOMPARE( static_cast< QgsPoint *>( mpBoundary->geometryN( 0 ) )->z(), 10.0 );
  QCOMPARE( mpBoundary->geometryN( 1 )->wkbType(), QgsWkbTypes::PointZ );
  QCOMPARE( static_cast< QgsPoint *>( mpBoundary->geometryN( 1 ) )->x(), 1.0 );
  QCOMPARE( static_cast< QgsPoint *>( mpBoundary->geometryN( 1 ) )->y(), 1.0 );
  QCOMPARE( static_cast< QgsPoint *>( mpBoundary->geometryN( 1 ) )->z(), 20.0 );
  delete boundary;

  //extend
  QgsLineString extend1;
  extend1.extend( 10, 10 ); //test no crash
  extend1.setPoints( QList<QgsPoint>() << QgsPoint( 0, 0 ) << QgsPoint( 1, 0 ) << QgsPoint( 1, 1 ) );
  extend1.extend( 1, 2 );
  QCOMPARE( extend1.pointN( 0 ), QgsPoint( QgsWkbTypes::Point, -1, 0 ) );
  QCOMPARE( extend1.pointN( 1 ), QgsPoint( QgsWkbTypes::Point, 1, 0 ) );
  QCOMPARE( extend1.pointN( 2 ), QgsPoint( QgsWkbTypes::Point, 1, 3 ) );
}

void TestQgsGeometry::polygon()
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
  QCOMPARE( p1.wkbType(), QgsWkbTypes::Polygon );
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
  QgsLineString *ext = 0;
  p1.setExteriorRing( ext );
  QVERIFY( p1.isEmpty() );
  QCOMPARE( p1.numInteriorRings(), 0 );
  QCOMPARE( p1.nCoordinates(), 0 );
  QCOMPARE( p1.ringCount(), 0 );
  QCOMPARE( p1.partCount(), 0 );
  QVERIFY( !p1.exteriorRing() );
  QVERIFY( !p1.interiorRing( 0 ) );
  QCOMPARE( p1.wkbType(), QgsWkbTypes::Polygon );

  // empty exterior ring
  ext = new QgsLineString();
  p1.setExteriorRing( ext );
  QVERIFY( p1.isEmpty() );
  QCOMPARE( p1.numInteriorRings(), 0 );
  QCOMPARE( p1.nCoordinates(), 0 );
  QCOMPARE( p1.ringCount(), 1 );
  QCOMPARE( p1.partCount(), 1 );
  QVERIFY( p1.exteriorRing() );
  QVERIFY( !p1.interiorRing( 0 ) );
  QCOMPARE( p1.wkbType(), QgsWkbTypes::Polygon );

  //valid exterior ring
  ext = new QgsLineString();
  ext->setPoints( QgsPointSequence() << QgsPoint( 0, 0 ) << QgsPoint( 0, 10 ) << QgsPoint( 10, 10 )
                  << QgsPoint( 10, 0 ) << QgsPoint( 0, 0 ) );
  p1.setExteriorRing( ext );
  QVERIFY( !p1.isEmpty() );
  QCOMPARE( p1.numInteriorRings(), 0 );
  QCOMPARE( p1.nCoordinates(), 5 );
  QCOMPARE( p1.ringCount(), 1 );
  QCOMPARE( p1.partCount(), 1 );
  QVERIFY( !p1.is3D() );
  QVERIFY( !p1.isMeasure() );
  QCOMPARE( p1.wkbType(), QgsWkbTypes::Polygon );
  QCOMPARE( p1.wktTypeStr(), QString( "Polygon" ) );
  QCOMPARE( p1.geometryType(), QString( "Polygon" ) );
  QCOMPARE( p1.dimension(), 2 );
  QVERIFY( !p1.hasCurvedSegments() );
  QCOMPARE( p1.area(), 100.0 );
  QCOMPARE( p1.perimeter(), 40.0 );
  QVERIFY( p1.exteriorRing() );
  QVERIFY( !p1.interiorRing( 0 ) );

  //retrieve exterior ring and check
  QCOMPARE( *( static_cast< const QgsLineString * >( p1.exteriorRing() ) ), *ext );

  //test that a non closed exterior ring will be automatically closed
  ext = new QgsLineString();
  ext->setPoints( QgsPointSequence() << QgsPoint( 0, 0 ) << QgsPoint( 0, 10 ) << QgsPoint( 10, 10 )
                  << QgsPoint( 10, 0 ) );
  QVERIFY( !ext->isClosed() );
  p1.setExteriorRing( ext );
  QVERIFY( !p1.isEmpty() );
  QVERIFY( p1.exteriorRing()->isClosed() );
  QCOMPARE( p1.nCoordinates(), 5 );

  //initial setting of exterior ring should set z/m type
  QgsPolygonV2 p2;
  ext = new QgsLineString();
  ext->setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZ, 0, 0, 1 )
                  << QgsPoint( QgsWkbTypes::PointZ, 0, 10, 2 ) << QgsPoint( QgsWkbTypes::PointZ, 10, 10, 3 )
                  << QgsPoint( QgsWkbTypes::PointZ, 10, 0, 4 ) << QgsPoint( QgsWkbTypes::PointZ, 0, 0, 1 ) );
  p2.setExteriorRing( ext );
  QVERIFY( p2.is3D() );
  QVERIFY( !p2.isMeasure() );
  QCOMPARE( p2.wkbType(), QgsWkbTypes::PolygonZ );
  QCOMPARE( p2.wktTypeStr(), QString( "PolygonZ" ) );
  QCOMPARE( p2.geometryType(), QString( "Polygon" ) );
  QCOMPARE( *( static_cast< const QgsLineString * >( p2.exteriorRing() ) ), *ext );
  QgsPolygonV2 p3;
  ext = new QgsLineString();
  ext->setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointM, 0, 0, 0, 1 )
                  << QgsPoint( QgsWkbTypes::PointM, 0, 10, 0, 2 ) << QgsPoint( QgsWkbTypes::PointM, 10, 10, 0, 3 )
                  << QgsPoint( QgsWkbTypes::PointM, 10, 0, 0, 4 ) << QgsPoint( QgsWkbTypes::PointM, 0, 0, 0, 1 ) );
  p3.setExteriorRing( ext );
  QVERIFY( !p3.is3D() );
  QVERIFY( p3.isMeasure() );
  QCOMPARE( p3.wkbType(), QgsWkbTypes::PolygonM );
  QCOMPARE( p3.wktTypeStr(), QString( "PolygonM" ) );
  QCOMPARE( *( static_cast< const QgsLineString * >( p3.exteriorRing() ) ), *ext );
  QgsPolygonV2 p4;
  ext = new QgsLineString();
  ext->setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 0, 0, 2, 1 )
                  << QgsPoint( QgsWkbTypes::PointZM, 0, 10, 3, 2 ) << QgsPoint( QgsWkbTypes::PointZM, 10, 10, 5, 3 )
                  << QgsPoint( QgsWkbTypes::PointZM, 10, 0, 0, 4 ) << QgsPoint( QgsWkbTypes::PointZM, 0, 0, 2, 1 ) );
  p4.setExteriorRing( ext );
  QVERIFY( p4.is3D() );
  QVERIFY( p4.isMeasure() );
  QCOMPARE( p4.wkbType(), QgsWkbTypes::PolygonZM );
  QCOMPARE( p4.wktTypeStr(), QString( "PolygonZM" ) );
  QCOMPARE( *( static_cast< const QgsLineString * >( p4.exteriorRing() ) ), *ext );
  QgsPolygonV2 p5;
  ext = new QgsLineString();
  ext->setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::Point25D, 0, 0, 1 )
                  << QgsPoint( QgsWkbTypes::Point25D, 0, 10, 2 ) << QgsPoint( QgsWkbTypes::Point25D, 10, 10, 3 )
                  << QgsPoint( QgsWkbTypes::Point25D, 10, 0, 4 ) << QgsPoint( QgsWkbTypes::Point25D, 0, 0, 1 ) );
  p5.setExteriorRing( ext );
  QVERIFY( p5.is3D() );
  QVERIFY( !p5.isMeasure() );
  QCOMPARE( p5.wkbType(), QgsWkbTypes::Polygon25D );
  QCOMPARE( p5.wktTypeStr(), QString( "PolygonZ" ) );
  QCOMPARE( *( static_cast< const QgsLineString * >( p5.exteriorRing() ) ), *ext );

  //setting curved exterior ring should be segmentized
  QgsCircularString *circularRing = new QgsCircularString();
  circularRing->setPoints( QgsPointSequence() << QgsPoint( 0, 0 ) << QgsPoint( 0, 10 ) << QgsPoint( 10, 10 )
                           << QgsPoint( 10, 0 ) << QgsPoint( 0, 0 ) );
  QVERIFY( circularRing->hasCurvedSegments() );
  p5.setExteriorRing( circularRing );
  QVERIFY( !p5.exteriorRing()->hasCurvedSegments() );
  QCOMPARE( p5.exteriorRing()->wkbType(), QgsWkbTypes::LineString );

  //addInteriorRing
  QgsPolygonV2 p6;
  ext = new QgsLineString();
  ext->setPoints( QgsPointSequence() << QgsPoint( 0, 0 ) << QgsPoint( 0, 10 ) << QgsPoint( 10, 10 )
                  << QgsPoint( 10, 0 ) << QgsPoint( 0, 0 ) );
  p6.setExteriorRing( ext );
  //empty ring
  QCOMPARE( p6.numInteriorRings(), 0 );
  QVERIFY( !p6.interiorRing( -1 ) );
  QVERIFY( !p6.interiorRing( 0 ) );
  p6.addInteriorRing( 0 );
  QCOMPARE( p6.numInteriorRings(), 0 );
  QgsLineString *ring = new QgsLineString();
  ring->setPoints( QgsPointSequence() << QgsPoint( 1, 1 ) << QgsPoint( 1, 9 ) << QgsPoint( 9, 9 )
                   << QgsPoint( 9, 1 ) << QgsPoint( 1, 1 ) );
  p6.addInteriorRing( ring );
  QCOMPARE( p6.numInteriorRings(), 1 );
  QCOMPARE( p6.interiorRing( 0 ), ring );
  QVERIFY( !p6.interiorRing( 1 ) );

  //add non-closed interior ring, should be closed automatically
  ring = new QgsLineString();
  ring->setPoints( QgsPointSequence() << QgsPoint( 0.1, 0.1 ) << QgsPoint( 0.1, 0.9 ) << QgsPoint( 0.9, 0.9 )
                   << QgsPoint( 0.9, 0.1 ) );
  QVERIFY( !ring->isClosed() );
  p6.addInteriorRing( ring );
  QCOMPARE( p6.numInteriorRings(), 2 );
  QVERIFY( p6.interiorRing( 1 )->isClosed() );

  //try adding an interior ring with z to a 2d polygon, z should be dropped
  ring = new QgsLineString();
  ring->setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZ, 0.1, 0.1, 1 )
                   << QgsPoint( QgsWkbTypes::PointZ, 0.1, 0.2, 2 ) << QgsPoint( QgsWkbTypes::PointZ, 0.2, 0.2, 3 )
                   << QgsPoint( QgsWkbTypes::PointZ, 0.2, 0.1, 4 ) << QgsPoint( QgsWkbTypes::PointZ, 0.1, 0.1, 1 ) );
  p6.addInteriorRing( ring );
  QCOMPARE( p6.numInteriorRings(), 3 );
  QVERIFY( !p6.is3D() );
  QVERIFY( !p6.isMeasure() );
  QCOMPARE( p6.wkbType(), QgsWkbTypes::Polygon );
  QVERIFY( p6.interiorRing( 2 ) );
  QVERIFY( !p6.interiorRing( 2 )->is3D() );
  QVERIFY( !p6.interiorRing( 2 )->isMeasure() );
  QCOMPARE( p6.interiorRing( 2 )->wkbType(), QgsWkbTypes::LineString );

  //try adding an interior ring with m to a 2d polygon, m should be dropped
  ring = new QgsLineString();
  ring->setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointM, 0.1, 0.1, 0, 1 )
                   << QgsPoint( QgsWkbTypes::PointM, 0.1, 0.2, 0, 2 ) << QgsPoint( QgsWkbTypes::PointM, 0.2, 0.2, 0, 3 )
                   << QgsPoint( QgsWkbTypes::PointM, 0.2, 0.1, 0, 4 ) << QgsPoint( QgsWkbTypes::PointM, 0.1, 0.1, 0, 1 ) );
  p6.addInteriorRing( ring );
  QCOMPARE( p6.numInteriorRings(), 4 );
  QVERIFY( !p6.is3D() );
  QVERIFY( !p6.isMeasure() );
  QCOMPARE( p6.wkbType(), QgsWkbTypes::Polygon );
  QVERIFY( p6.interiorRing( 3 ) );
  QVERIFY( !p6.interiorRing( 3 )->is3D() );
  QVERIFY( !p6.interiorRing( 3 )->isMeasure() );
  QCOMPARE( p6.interiorRing( 3 )->wkbType(), QgsWkbTypes::LineString );

  //addInteriorRing without z/m to PolygonZM
  QgsPolygonV2 p6b;
  ext = new QgsLineString();
  ext->setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 0, 0, 1 )
                  << QgsPoint( QgsWkbTypes::PointZM, 0, 10, 2 ) << QgsPoint( QgsWkbTypes::PointZM, 10, 10, 3 )
                  << QgsPoint( QgsWkbTypes::PointZM, 10, 0, 4 ) << QgsPoint( QgsWkbTypes::PointZM, 0, 0, 1 ) );
  p6b.setExteriorRing( ext );
  QVERIFY( p6b.is3D() );
  QVERIFY( p6b.isMeasure() );
  QCOMPARE( p6b.wkbType(), QgsWkbTypes::PolygonZM );
  //ring has no z
  ring = new QgsLineString();
  ring->setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointM, 1, 1, 0, 2 ) << QgsPoint( QgsWkbTypes::PointM, 1, 9 ) << QgsPoint( QgsWkbTypes::PointM, 9, 9 )
                   << QgsPoint( QgsWkbTypes::PointM, 9, 1 ) << QgsPoint( QgsWkbTypes::PointM, 1, 1 ) );
  p6b.addInteriorRing( ring );
  QVERIFY( p6b.interiorRing( 0 ) );
  QVERIFY( p6b.interiorRing( 0 )->is3D() );
  QVERIFY( p6b.interiorRing( 0 )->isMeasure() );
  QCOMPARE( p6b.interiorRing( 0 )->wkbType(), QgsWkbTypes::LineStringZM );
  QCOMPARE( p6b.interiorRing( 0 )->vertexAt( QgsVertexId( 0, 0, 0 ) ), QgsPoint( QgsWkbTypes::PointZM, 1, 1, 0, 2 ) );
  //ring has no m
  ring = new QgsLineString();
  ring->setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZ, 0.1, 0.1, 1 )
                   << QgsPoint( QgsWkbTypes::PointZ, 0.1, 0.2, 2 ) << QgsPoint( QgsWkbTypes::PointZ, 0.2, 0.2, 3 )
                   << QgsPoint( QgsWkbTypes::PointZ, 0.2, 0.1, 4 ) << QgsPoint( QgsWkbTypes::PointZ, 0.1, 0.1, 1 ) );
  p6b.addInteriorRing( ring );
  QVERIFY( p6b.interiorRing( 1 ) );
  QVERIFY( p6b.interiorRing( 1 )->is3D() );
  QVERIFY( p6b.interiorRing( 1 )->isMeasure() );
  QCOMPARE( p6b.interiorRing( 1 )->wkbType(), QgsWkbTypes::LineStringZM );
  QCOMPARE( p6b.interiorRing( 1 )->vertexAt( QgsVertexId( 0, 0, 0 ) ), QgsPoint( QgsWkbTypes::PointZM, 0.1, 0.1, 1, 0 ) );
  //test handling of 25D rings/polygons
  QgsPolygonV2 p6c;
  ext = new QgsLineString();
  ext->setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::Point25D, 0, 0, 1 )
                  << QgsPoint( QgsWkbTypes::Point25D, 0, 10, 2 ) << QgsPoint( QgsWkbTypes::Point25D, 10, 10, 3 )
                  << QgsPoint( QgsWkbTypes::Point25D, 10, 0, 4 ) << QgsPoint( QgsWkbTypes::Point25D, 0, 0, 1 ) );
  p6c.setExteriorRing( ext );
  QVERIFY( p6c.is3D() );
  QVERIFY( !p6c.isMeasure() );
  QCOMPARE( p6c.wkbType(), QgsWkbTypes::Polygon25D );
  //adding a LineStringZ, should become LineString25D
  ring = new QgsLineString();
  ring->setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZ, 0.1, 0.1, 1 )
                   << QgsPoint( QgsWkbTypes::PointZ, 0.1, 0.2, 2 ) << QgsPoint( QgsWkbTypes::PointZ, 0.2, 0.2, 3 )
                   << QgsPoint( QgsWkbTypes::PointZ, 0.2, 0.1, 4 ) << QgsPoint( QgsWkbTypes::PointZ, 0.1, 0.1, 1 ) );
  QCOMPARE( ring->wkbType(), QgsWkbTypes::LineStringZ );
  p6c.addInteriorRing( ring );
  QVERIFY( p6c.interiorRing( 0 ) );
  QVERIFY( p6c.interiorRing( 0 )->is3D() );
  QVERIFY( !p6c.interiorRing( 0 )->isMeasure() );
  QCOMPARE( p6c.interiorRing( 0 )->wkbType(), QgsWkbTypes::LineString25D );
  QCOMPARE( p6c.interiorRing( 0 )->vertexAt( QgsVertexId( 0, 0, 0 ) ), QgsPoint( QgsWkbTypes::Point25D, 0.1, 0.1, 1 ) );
  //add a LineStringM, should become LineString25D
  ring = new QgsLineString();
  ring->setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointM, 0.1, 0.1, 0, 1 )
                   << QgsPoint( QgsWkbTypes::PointM, 0.1, 0.2, 0, 2 ) << QgsPoint( QgsWkbTypes::PointM, 0.2, 0.2, 0, 3 )
                   << QgsPoint( QgsWkbTypes::PointM, 0.2, 0.1, 0, 4 ) << QgsPoint( QgsWkbTypes::PointM, 0.1, 0.1, 0, 1 ) );
  QCOMPARE( ring->wkbType(), QgsWkbTypes::LineStringM );
  p6c.addInteriorRing( ring );
  QVERIFY( p6c.interiorRing( 1 ) );
  QVERIFY( p6c.interiorRing( 1 )->is3D() );
  QVERIFY( !p6c.interiorRing( 1 )->isMeasure() );
  QCOMPARE( p6c.interiorRing( 1 )->wkbType(), QgsWkbTypes::LineString25D );
  QCOMPARE( p6c.interiorRing( 1 )->vertexAt( QgsVertexId( 0, 0, 0 ) ), QgsPoint( QgsWkbTypes::Point25D, 0.1, 0.1 ) );

  //add curved ring to polygon
  circularRing = new QgsCircularString();
  circularRing->setPoints( QgsPointSequence() << QgsPoint( 0, 0 ) << QgsPoint( 0, 10 ) << QgsPoint( 10, 10 )
                           << QgsPoint( 10, 0 ) << QgsPoint( 0, 0 ) );
  QVERIFY( circularRing->hasCurvedSegments() );
  p6c.addInteriorRing( circularRing );
  QVERIFY( p6c.interiorRing( 2 ) );
  QVERIFY( !p6c.interiorRing( 2 )->hasCurvedSegments() );
  QVERIFY( p6c.interiorRing( 2 )->is3D() );
  QVERIFY( !p6c.interiorRing( 2 )->isMeasure() );
  QCOMPARE( p6c.interiorRing( 2 )->wkbType(), QgsWkbTypes::LineString25D );

  //set interior rings
  QgsPolygonV2 p7;
  ext = new QgsLineString();
  ext->setPoints( QgsPointSequence() << QgsPoint( 0, 0 ) << QgsPoint( 0, 10 ) << QgsPoint( 10, 10 )
                  << QgsPoint( 10, 0 ) << QgsPoint( 0, 0 ) );
  p7.setExteriorRing( ext );
  //add a list of rings with mixed types
  QList< QgsCurve * > rings;
  rings << new QgsLineString();
  static_cast< QgsLineString *>( rings[0] )->setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZ, 0.1, 0.1, 1 )
      << QgsPoint( QgsWkbTypes::PointZ, 0.1, 0.2, 2 ) << QgsPoint( QgsWkbTypes::PointZ, 0.2, 0.2, 3 )
      << QgsPoint( QgsWkbTypes::PointZ, 0.2, 0.1, 4 ) << QgsPoint( QgsWkbTypes::PointZ, 0.1, 0.1, 1 ) );
  rings << new QgsLineString();
  static_cast< QgsLineString *>( rings[1] )->setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointM, 0.3, 0.3, 0, 1 )
      << QgsPoint( QgsWkbTypes::PointM, 0.3, 0.4, 0, 2 ) << QgsPoint( QgsWkbTypes::PointM, 0.4, 0.4, 0, 3 )
      << QgsPoint( QgsWkbTypes::PointM, 0.4, 0.3, 0, 4 ) << QgsPoint( QgsWkbTypes::PointM, 0.3, 0.3, 0, 1 ) );
  //throw an empty ring in too
  rings << 0;
  rings << new QgsCircularString();
  static_cast< QgsCircularString *>( rings[3] )->setPoints( QgsPointSequence() << QgsPoint( 0, 0 ) << QgsPoint( 0, 10 ) << QgsPoint( 10, 10 )
      << QgsPoint( 10, 0 ) << QgsPoint( 0, 0 ) );
  p7.setInteriorRings( rings );
  QCOMPARE( p7.numInteriorRings(), 3 );
  QVERIFY( p7.interiorRing( 0 ) );
  QVERIFY( !p7.interiorRing( 0 )->is3D() );
  QVERIFY( !p7.interiorRing( 0 )->isMeasure() );
  QCOMPARE( p7.interiorRing( 0 )->wkbType(), QgsWkbTypes::LineString );
  QCOMPARE( p7.interiorRing( 0 )->vertexAt( QgsVertexId( 0, 0, 0 ) ), QgsPoint( QgsWkbTypes::Point, 0.1, 0.1 ) );
  QVERIFY( p7.interiorRing( 1 ) );
  QVERIFY( !p7.interiorRing( 1 )->is3D() );
  QVERIFY( !p7.interiorRing( 1 )->isMeasure() );
  QCOMPARE( p7.interiorRing( 1 )->wkbType(), QgsWkbTypes::LineString );
  QCOMPARE( p7.interiorRing( 1 )->vertexAt( QgsVertexId( 0, 0, 0 ) ), QgsPoint( QgsWkbTypes::Point, 0.3, 0.3 ) );
  QVERIFY( p7.interiorRing( 2 ) );
  QVERIFY( !p7.interiorRing( 2 )->is3D() );
  QVERIFY( !p7.interiorRing( 2 )->isMeasure() );
  QCOMPARE( p7.interiorRing( 2 )->wkbType(), QgsWkbTypes::LineString );

  //set rings with existing
  rings.clear();
  rings << new QgsLineString();
  static_cast< QgsLineString *>( rings[0] )->setPoints( QgsPointSequence() << QgsPoint( 0.8, 0.8 )
      << QgsPoint( 0.8, 0.9 ) << QgsPoint( 0.9, 0.9 )
      << QgsPoint( 0.9, 0.8 ) << QgsPoint( 0.8, 0.8 ) );
  p7.setInteriorRings( rings );
  QCOMPARE( p7.numInteriorRings(), 1 );
  QVERIFY( p7.interiorRing( 0 ) );
  QVERIFY( !p7.interiorRing( 0 )->is3D() );
  QVERIFY( !p7.interiorRing( 0 )->isMeasure() );
  QCOMPARE( p7.interiorRing( 0 )->wkbType(), QgsWkbTypes::LineString );
  QCOMPARE( p7.interiorRing( 0 )->vertexAt( QgsVertexId( 0, 0, 0 ) ), QgsPoint( QgsWkbTypes::Point, 0.8, 0.8 ) );
  rings.clear();
  p7.setInteriorRings( rings );
  QCOMPARE( p7.numInteriorRings(), 0 );

  //change dimensionality of interior rings using setExteriorRing
  QgsPolygonV2 p7a;
  ext = new QgsLineString();
  ext->setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZ, 0, 0, 1 ) << QgsPoint( QgsWkbTypes::PointZ, 0, 10, 2 )
                  << QgsPoint( QgsWkbTypes::PointZ, 10, 10, 1 ) << QgsPoint( QgsWkbTypes::PointZ, 10, 0, 3 ) << QgsPoint( QgsWkbTypes::PointZ, 0, 0, 1 ) );
  p7a.setExteriorRing( ext );
  rings.clear();
  rings << new QgsLineString();
  static_cast< QgsLineString *>( rings[0] )->setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZ, 0.1, 0.1, 1 )
      << QgsPoint( QgsWkbTypes::PointZ, 0.1, 0.2, 2 ) << QgsPoint( QgsWkbTypes::PointZ, 0.2, 0.2, 3 )
      << QgsPoint( QgsWkbTypes::PointZ, 0.2, 0.1, 4 ) << QgsPoint( QgsWkbTypes::PointZ, 0.1, 0.1, 1 ) );
  rings << new QgsLineString();
  static_cast< QgsLineString *>( rings[1] )->setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZ, 0.3, 0.3, 1 )
      << QgsPoint( QgsWkbTypes::PointZ, 0.3, 0.4, 2 ) << QgsPoint( QgsWkbTypes::PointZ, 0.4, 0.4, 3 )
      << QgsPoint( QgsWkbTypes::PointZ, 0.4, 0.3, 4 ) << QgsPoint( QgsWkbTypes::PointZ, 0.3, 0.3,  1 ) );
  p7a.setInteriorRings( rings );
  QVERIFY( p7a.is3D() );
  QVERIFY( !p7a.isMeasure() );
  QVERIFY( p7a.interiorRing( 0 )->is3D() );
  QVERIFY( !p7a.interiorRing( 0 )->isMeasure() );
  QVERIFY( p7a.interiorRing( 1 )->is3D() );
  QVERIFY( !p7a.interiorRing( 1 )->isMeasure() );
  //reset exterior ring to 2d
  ext = new QgsLineString();
  ext->setPoints( QgsPointSequence() << QgsPoint( 0, 0 ) << QgsPoint( 0, 10 )
                  << QgsPoint( 10, 10 ) << QgsPoint( 10, 0 ) << QgsPoint( 0, 0 ) );
  p7a.setExteriorRing( ext );
  QVERIFY( !p7a.is3D() );
  QVERIFY( !p7a.interiorRing( 0 )->is3D() ); //rings should also be made 2D
  QVERIFY( !p7a.interiorRing( 1 )->is3D() );
  //reset exterior ring to LineStringM
  ext = new QgsLineString();
  ext->setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointM, 0, 0 ) << QgsPoint( QgsWkbTypes::PointM, 0, 10 )
                  << QgsPoint( QgsWkbTypes::PointM, 10, 10 ) << QgsPoint( QgsWkbTypes::PointM, 10, 0 ) << QgsPoint( QgsWkbTypes::PointM, 0, 0 ) );
  p7a.setExteriorRing( ext );
  QVERIFY( p7a.isMeasure() );
  QVERIFY( p7a.interiorRing( 0 )->isMeasure() ); //rings should also gain measure
  QVERIFY( p7a.interiorRing( 1 )->isMeasure() );
  //25D exterior ring
  ext = new QgsLineString();
  ext->setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::Point25D, 0, 0 ) << QgsPoint( QgsWkbTypes::Point25D, 0, 10 )
                  << QgsPoint( QgsWkbTypes::Point25D, 10, 10 ) << QgsPoint( QgsWkbTypes::Point25D, 10, 0 ) << QgsPoint( QgsWkbTypes::Point25D, 0, 0 ) );
  p7a.setExteriorRing( ext );
  QVERIFY( p7a.is3D() );
  QVERIFY( !p7a.isMeasure() );
  QVERIFY( p7a.interiorRing( 0 )->is3D() ); //rings should also be made 25D
  QVERIFY( !p7a.interiorRing( 0 )->isMeasure() );
  QVERIFY( p7a.interiorRing( 1 )->is3D() );
  QVERIFY( !p7a.interiorRing( 1 )->isMeasure() );
  QCOMPARE( p7a.interiorRing( 0 )->wkbType(), QgsWkbTypes::LineString25D );
  QCOMPARE( p7a.interiorRing( 1 )->wkbType(), QgsWkbTypes::LineString25D );


  //removeInteriorRing
  QgsPolygonV2 p8;
  ext = new QgsLineString();
  ext->setPoints( QgsPointSequence() << QgsPoint( 0, 0 ) << QgsPoint( 0, 10 ) << QgsPoint( 10, 10 )
                  << QgsPoint( 10, 0 ) << QgsPoint( 0, 0 ) );
  p8.setExteriorRing( ext );
  QVERIFY( !p8.removeInteriorRing( -1 ) );
  QVERIFY( !p8.removeInteriorRing( 0 ) );
  rings.clear();
  rings << new QgsLineString();
  static_cast< QgsLineString *>( rings[0] )->setPoints( QgsPointSequence() << QgsPoint( 0.1, 0.1 )
      << QgsPoint( 0.1, 0.2 ) << QgsPoint( 0.2, 0.2 )
      << QgsPoint( 0.2, 0.1 ) << QgsPoint( 0.1, 0.1 ) );
  rings << new QgsLineString();
  static_cast< QgsLineString *>( rings[1] )->setPoints( QgsPointSequence() << QgsPoint( 0.3, 0.3 )
      << QgsPoint( 0.3, 0.4 ) << QgsPoint( 0.4, 0.4 )
      << QgsPoint( 0.4, 0.3 ) << QgsPoint( 0.3, 0.3 ) );
  rings << new QgsLineString();
  static_cast< QgsLineString *>( rings[2] )->setPoints( QgsPointSequence() << QgsPoint( 0.8, 0.8 )
      << QgsPoint( 0.8, 0.9 ) << QgsPoint( 0.9, 0.9 )
      << QgsPoint( 0.9, 0.8 ) << QgsPoint( 0.8, 0.8 ) );
  p8.setInteriorRings( rings );
  QCOMPARE( p8.numInteriorRings(), 3 );
  QVERIFY( p8.removeInteriorRing( 0 ) );
  QCOMPARE( p8.numInteriorRings(), 2 );
  QCOMPARE( p8.interiorRing( 0 )->vertexAt( QgsVertexId( 0, 0, 0 ) ), QgsPoint( 0.3, 0.3 ) );
  QCOMPARE( p8.interiorRing( 1 )->vertexAt( QgsVertexId( 0, 0, 0 ) ), QgsPoint( 0.8, 0.8 ) );
  QVERIFY( p8.removeInteriorRing( 1 ) );
  QCOMPARE( p8.numInteriorRings(), 1 );
  QCOMPARE( p8.interiorRing( 0 )->vertexAt( QgsVertexId( 0, 0, 0 ) ), QgsPoint( 0.3, 0.3 ) );
  QVERIFY( p8.removeInteriorRing( 0 ) );
  QCOMPARE( p8.numInteriorRings(), 0 );
  QVERIFY( !p8.removeInteriorRing( 0 ) );

  //clear
  QgsPolygonV2 p9;
  ext = new QgsLineString();
  ext->setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZ, 0, 0, 1 )
                  << QgsPoint( QgsWkbTypes::PointZ, 0, 10, 2 ) << QgsPoint( QgsWkbTypes::PointZ, 10, 10, 3 )
                  << QgsPoint( QgsWkbTypes::PointZ, 10, 0, 4 ) << QgsPoint( QgsWkbTypes::PointZ, 0, 0, 1 ) );
  p9.setExteriorRing( ext );
  ring = new QgsLineString();
  ring->setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZ, 1, 1, 1 )
                   << QgsPoint( QgsWkbTypes::PointZ, 1, 9, 2 ) << QgsPoint( QgsWkbTypes::PointZ, 9, 9, 3 )
                   << QgsPoint( QgsWkbTypes::PointZ, 9, 1, 4 ) << QgsPoint( QgsWkbTypes::PointZ, 1, 1, 1 ) );
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
  QCOMPARE( p9.wkbType(), QgsWkbTypes::Polygon );

  //equality operator
  QgsPolygonV2 p10;
  QgsPolygonV2 p10b;
  QVERIFY( p10 == p10b );
  QVERIFY( !( p10 != p10b ) );
  ext = new QgsLineString();
  ext->setPoints( QgsPointSequence() << QgsPoint( 0, 0 ) << QgsPoint( 0, 10 ) << QgsPoint( 10, 10 )
                  << QgsPoint( 10, 0 ) << QgsPoint( 0, 0 ) );
  p10.setExteriorRing( ext );
  QVERIFY( !( p10 == p10b ) );
  QVERIFY( p10 != p10b );
  ext = new QgsLineString();
  ext->setPoints( QgsPointSequence() << QgsPoint( 0, 0 ) << QgsPoint( 0, 10 ) << QgsPoint( 10, 10 )
                  << QgsPoint( 10, 0 ) << QgsPoint( 0, 0 ) );
  p10b.setExteriorRing( ext );
  QVERIFY( p10 == p10b );
  QVERIFY( !( p10 != p10b ) );
  ext = new QgsLineString();
  ext->setPoints( QgsPointSequence() << QgsPoint( 0, 0 ) << QgsPoint( 0, 9 ) << QgsPoint( 9, 9 )
                  << QgsPoint( 9, 0 ) << QgsPoint( 0, 0 ) );
  p10b.setExteriorRing( ext );
  QVERIFY( !( p10 == p10b ) );
  QVERIFY( p10 != p10b );
  ext = new QgsLineString();
  ext->setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZ, 0, 0, 1 ) << QgsPoint( QgsWkbTypes::PointZ, 0, 10, 2 )
                  << QgsPoint( QgsWkbTypes::PointZ, 10, 10, 3 ) << QgsPoint( QgsWkbTypes::PointZ, 10, 0, 4 ) << QgsPoint( QgsWkbTypes::PointZ, 0, 0, 1 ) );
  p10b.setExteriorRing( ext );
  QVERIFY( !( p10 == p10b ) );
  QVERIFY( p10 != p10b );
  p10b.setExteriorRing( p10.exteriorRing()->clone() );
  QVERIFY( p10 == p10b );
  QVERIFY( !( p10 != p10b ) );
  ring = new QgsLineString();
  ring->setPoints( QgsPointSequence() << QgsPoint( 1, 1 )
                   << QgsPoint( 1, 9 ) << QgsPoint( 9, 9 )
                   << QgsPoint( 9, 1 ) << QgsPoint( 1, 1 ) );
  p10.addInteriorRing( ring );
  QVERIFY( !( p10 == p10b ) );
  QVERIFY( p10 != p10b );
  ring = new QgsLineString();
  ring->setPoints( QgsPointSequence() << QgsPoint( 2, 1 )
                   << QgsPoint( 2, 9 ) << QgsPoint( 9, 9 )
                   << QgsPoint( 9, 1 ) << QgsPoint( 2, 1 ) );
  p10b.addInteriorRing( ring );
  QVERIFY( !( p10 == p10b ) );
  QVERIFY( p10 != p10b );
  p10b.removeInteriorRing( 0 );
  p10b.addInteriorRing( p10.interiorRing( 0 )->clone() );
  QVERIFY( p10 == p10b );
  QVERIFY( !( p10 != p10b ) );

  //clone

  QgsPolygonV2 p11;
  std::unique_ptr< QgsPolygonV2 >cloned( p11.clone() );
  QCOMPARE( p11, *cloned );
  ext = new QgsLineString();
  ext->setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 0, 0, 1, 5 )
                  << QgsPoint( QgsWkbTypes::PointZM, 0, 10, 2, 6 ) << QgsPoint( QgsWkbTypes::PointZM, 10, 10, 3, 7 )
                  << QgsPoint( QgsWkbTypes::PointZM, 10, 0, 4, 8 ) << QgsPoint( QgsWkbTypes::PointZM, 0, 0, 1, 9 ) );
  p11.setExteriorRing( ext );
  ring = new QgsLineString();
  ring->setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 1, 1, 1, 2 )
                   << QgsPoint( QgsWkbTypes::PointZM, 1, 9, 2, 3 ) << QgsPoint( QgsWkbTypes::PointZM, 9, 9, 3, 6 )
                   << QgsPoint( QgsWkbTypes::PointZM, 9, 1, 4, 4 ) << QgsPoint( QgsWkbTypes::PointZM, 1, 1, 1, 7 ) );
  p11.addInteriorRing( ring );
  cloned.reset( p11.clone() );
  QCOMPARE( p11, *cloned );

  //copy constructor
  QgsPolygonV2 p12;
  QgsPolygonV2 p13( p12 );
  QCOMPARE( p12, p13 );
  ext = new QgsLineString();
  ext->setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 0, 0, 1, 5 )
                  << QgsPoint( QgsWkbTypes::PointZM, 0, 10, 2, 6 ) << QgsPoint( QgsWkbTypes::PointZM, 10, 10, 3, 7 )
                  << QgsPoint( QgsWkbTypes::PointZM, 10, 0, 4, 8 ) << QgsPoint( QgsWkbTypes::PointZM, 0, 0, 1, 9 ) );
  p12.setExteriorRing( ext );
  ring = new QgsLineString();
  ring->setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 1, 1, 1, 2 )
                   << QgsPoint( QgsWkbTypes::PointZM, 1, 9, 2, 3 ) << QgsPoint( QgsWkbTypes::PointZM, 9, 9, 3, 6 )
                   << QgsPoint( QgsWkbTypes::PointZM, 9, 1, 4, 4 ) << QgsPoint( QgsWkbTypes::PointZM, 1, 1, 1, 7 ) );
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
  std::unique_ptr< QgsPolygonV2 > surface( p12.surfaceToPolygon() );
  QCOMPARE( *surface, p12 );

  //to/fromWKB
  QgsPolygonV2 p16;
  ext = new QgsLineString();
  ext->setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::Point, 0, 0 )
                  << QgsPoint( QgsWkbTypes::Point, 0, 10 ) << QgsPoint( QgsWkbTypes::Point, 10, 10 )
                  << QgsPoint( QgsWkbTypes::Point, 10, 0 ) << QgsPoint( QgsWkbTypes::Point, 0, 0 ) );
  p16.setExteriorRing( ext );
  ring = new QgsLineString();
  ring->setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::Point, 1, 1 )
                   << QgsPoint( QgsWkbTypes::Point, 1, 9 ) << QgsPoint( QgsWkbTypes::Point, 9, 9 )
                   << QgsPoint( QgsWkbTypes::Point, 9, 1 ) << QgsPoint( QgsWkbTypes::Point, 1, 1 ) );
  p16.addInteriorRing( ring );
  QByteArray wkb16 = p16.asWkb();
  QgsPolygonV2 p17;
  QgsConstWkbPtr wkb16ptr( wkb16 );
  p17.fromWkb( wkb16ptr );
  QCOMPARE( p16, p17 );
  //PolygonZ
  p16.clear();
  p17.clear();
  ext = new QgsLineString();
  ext->setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZ, 0, 0, 1 )
                  << QgsPoint( QgsWkbTypes::PointZ, 0, 10, 2 ) << QgsPoint( QgsWkbTypes::PointZ, 10, 10, 3 )
                  << QgsPoint( QgsWkbTypes::PointZ, 10, 0, 4 ) << QgsPoint( QgsWkbTypes::PointZ, 0, 0, 1 ) );
  p16.setExteriorRing( ext );
  ring = new QgsLineString();
  ring->setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZ, 1, 1, 1 )
                   << QgsPoint( QgsWkbTypes::PointZ, 1, 9, 2 ) << QgsPoint( QgsWkbTypes::PointZ, 9, 9, 3 )
                   << QgsPoint( QgsWkbTypes::PointZ, 9, 1, 4 ) << QgsPoint( QgsWkbTypes::PointZ, 1, 1, 1 ) );
  p16.addInteriorRing( ring );
  wkb16 = p16.asWkb();
  QgsConstWkbPtr wkb16ptr2( wkb16 );
  p17.fromWkb( wkb16ptr2 );
  QCOMPARE( p16, p17 );
  //PolygonM
  p16.clear();
  p17.clear();
  ext = new QgsLineString();
  ext->setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointM, 0, 0, 0, 1 )
                  << QgsPoint( QgsWkbTypes::PointM, 0, 10,  0, 2 ) << QgsPoint( QgsWkbTypes::PointM, 10, 10, 0, 3 )
                  << QgsPoint( QgsWkbTypes::PointM, 10, 0,  0, 4 ) << QgsPoint( QgsWkbTypes::PointM, 0, 0, 0, 1 ) );
  p16.setExteriorRing( ext );
  ring = new QgsLineString();
  ring->setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointM, 1, 1, 0, 1 )
                   << QgsPoint( QgsWkbTypes::PointM, 1, 9, 0, 2 ) << QgsPoint( QgsWkbTypes::PointM, 9, 9, 0, 3 )
                   << QgsPoint( QgsWkbTypes::PointM, 9, 1, 0, 4 ) << QgsPoint( QgsWkbTypes::PointM, 1, 1, 0, 1 ) );
  p16.addInteriorRing( ring );
  wkb16 = p16.asWkb();
  QgsConstWkbPtr wkb16ptr3( wkb16 );
  p17.fromWkb( wkb16ptr3 );
  QCOMPARE( p16, p17 );
  //PolygonZM
  p16.clear();
  p17.clear();
  ext = new QgsLineString();
  ext->setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 0, 0, 1, 5 )
                  << QgsPoint( QgsWkbTypes::PointZM, 0, 10, 2, 6 ) << QgsPoint( QgsWkbTypes::PointZM, 10, 10, 3, 7 )
                  << QgsPoint( QgsWkbTypes::PointZM, 10, 0, 4, 8 ) << QgsPoint( QgsWkbTypes::PointZM, 0, 0, 1, 9 ) );
  p16.setExteriorRing( ext );
  ring = new QgsLineString();
  ring->setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 1, 1, 1, 2 )
                   << QgsPoint( QgsWkbTypes::PointZM, 1, 9, 2, 3 ) << QgsPoint( QgsWkbTypes::PointZM, 9, 9, 3, 6 )
                   << QgsPoint( QgsWkbTypes::PointZM, 9, 1, 4, 4 ) << QgsPoint( QgsWkbTypes::PointZM, 1, 1, 1, 7 ) );
  p16.addInteriorRing( ring );
  wkb16 = p16.asWkb();
  QgsConstWkbPtr wkb16ptr4( wkb16 );
  p17.fromWkb( wkb16ptr4 );
  QCOMPARE( p16, p17 );
  //Polygon25D
  p16.clear();
  p17.clear();
  ext = new QgsLineString();
  ext->setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::Point25D, 0, 0, 1 )
                  << QgsPoint( QgsWkbTypes::Point25D, 0, 10, 2 ) << QgsPoint( QgsWkbTypes::Point25D, 10, 10, 3 )
                  << QgsPoint( QgsWkbTypes::Point25D, 10, 0, 4 ) << QgsPoint( QgsWkbTypes::Point25D, 0, 0, 1 ) );
  p16.setExteriorRing( ext );
  ring = new QgsLineString();
  ring->setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::Point25D, 1, 1, 1 )
                   << QgsPoint( QgsWkbTypes::Point25D, 1, 9, 2 ) << QgsPoint( QgsWkbTypes::Point25D, 9, 9, 3 )
                   << QgsPoint( QgsWkbTypes::Point25D, 9, 1, 4 ) << QgsPoint( QgsWkbTypes::Point25D, 1, 1, 1 ) );
  p16.addInteriorRing( ring );
  wkb16 = p16.asWkb();
  p17.clear();
  QgsConstWkbPtr wkb16ptr5( wkb16 );
  p17.fromWkb( wkb16ptr5 );
  QCOMPARE( p16, p17 );

  //bad WKB - check for no crash
  p17.clear();
  QgsConstWkbPtr nullPtr( nullptr, 0 );
  QVERIFY( !p17.fromWkb( nullPtr ) );
  QCOMPARE( p17.wkbType(), QgsWkbTypes::Polygon );
  QgsPoint point( 1, 2 );
  QByteArray wkbPoint = point.asWkb();
  QgsConstWkbPtr wkbPointPtr( wkbPoint );
  QVERIFY( !p17.fromWkb( wkbPointPtr ) );
  QCOMPARE( p17.wkbType(), QgsWkbTypes::Polygon );

  //to/from WKT
  QgsPolygonV2 p18;
  ext = new QgsLineString();
  ext->setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 0, 0, 1, 5 )
                  << QgsPoint( QgsWkbTypes::PointZM, 0, 10, 2, 6 ) << QgsPoint( QgsWkbTypes::PointZM, 10, 10, 3, 7 )
                  << QgsPoint( QgsWkbTypes::PointZM, 10, 0, 4, 8 ) << QgsPoint( QgsWkbTypes::PointZM, 0, 0, 1, 9 ) );
  p18.setExteriorRing( ext );
  ring = new QgsLineString();
  ring->setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 1, 1, 1, 2 )
                   << QgsPoint( QgsWkbTypes::PointZM, 1, 9, 2, 3 ) << QgsPoint( QgsWkbTypes::PointZM, 9, 9, 3, 6 )
                   << QgsPoint( QgsWkbTypes::PointZM, 9, 1, 4, 4 ) << QgsPoint( QgsWkbTypes::PointZM, 1, 1, 1, 7 ) );
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
  QCOMPARE( p19.wkbType(), QgsWkbTypes::Polygon );

  //as JSON
  QgsPolygonV2 exportPolygon;
  ext = new QgsLineString();
  ext->setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::Point, 0, 0 )
                  << QgsPoint( QgsWkbTypes::Point, 0, 10 ) << QgsPoint( QgsWkbTypes::Point, 10, 10 )
                  << QgsPoint( QgsWkbTypes::Point, 10, 0 ) << QgsPoint( QgsWkbTypes::Point, 0, 0 ) );
  exportPolygon.setExteriorRing( ext );

  // GML document for compare
  QDomDocument doc( "gml" );

  // as GML2
  QString expectedSimpleGML2( QStringLiteral( "<Polygon xmlns=\"gml\"><outerBoundaryIs xmlns=\"gml\"><LinearRing xmlns=\"gml\"><coordinates xmlns=\"gml\" cs=\",\" ts=\" \">0,0 0,10 10,10 10,0 0,0</coordinates></LinearRing></outerBoundaryIs></Polygon>" ) );
  QCOMPARE( elemToString( exportPolygon.asGML2( doc ) ), expectedSimpleGML2 );

  //as GML3
  QString expectedSimpleGML3( QStringLiteral( "<Polygon xmlns=\"gml\"><exterior xmlns=\"gml\"><LinearRing xmlns=\"gml\"><posList xmlns=\"gml\" srsDimension=\"2\">0 0 0 10 10 10 10 0 0 0</posList></LinearRing></exterior></Polygon>" ) );
  QCOMPARE( elemToString( exportPolygon.asGML3( doc ) ), expectedSimpleGML3 );

  // as JSON
  QString expectedSimpleJson( "{\"type\": \"Polygon\", \"coordinates\": [[ [0, 0], [0, 10], [10, 10], [10, 0], [0, 0]]] }" );
  QCOMPARE( exportPolygon.asJSON(), expectedSimpleJson );

  ring = new QgsLineString();
  ring->setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::Point, 1, 1 )
                   << QgsPoint( QgsWkbTypes::Point, 1, 9 ) << QgsPoint( QgsWkbTypes::Point, 9, 9 )
                   << QgsPoint( QgsWkbTypes::Point, 9, 1 ) << QgsPoint( QgsWkbTypes::Point, 1, 1 ) );
  exportPolygon.addInteriorRing( ring );

  QString expectedJson( QStringLiteral( "{\"type\": \"Polygon\", \"coordinates\": [[ [0, 0], [0, 10], [10, 10], [10, 0], [0, 0]], [ [1, 1], [1, 9], [9, 9], [9, 1], [1, 1]]] }" ) );
  QCOMPARE( exportPolygon.asJSON(), expectedJson );

  QgsPolygonV2 exportPolygonFloat;
  ext = new QgsLineString();
  ext->setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::Point, 10 / 9.0, 10 / 9.0 )
                  << QgsPoint( QgsWkbTypes::Point, 10 / 9.0, 100 / 9.0 ) << QgsPoint( QgsWkbTypes::Point, 100 / 9.0, 100 / 9.0 )
                  << QgsPoint( QgsWkbTypes::Point, 100 / 9.0, 10 / 9.0 ) << QgsPoint( QgsWkbTypes::Point, 10 / 9.0, 10 / 9.0 ) );
  exportPolygonFloat.setExteriorRing( ext );
  ring = new QgsLineString();
  ring->setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::Point, 2 / 3.0, 2 / 3.0 )
                   << QgsPoint( QgsWkbTypes::Point, 2 / 3.0, 4 / 3.0 ) << QgsPoint( QgsWkbTypes::Point, 4 / 3.0, 4 / 3.0 )
                   << QgsPoint( QgsWkbTypes::Point, 4 / 3.0, 2 / 3.0 ) << QgsPoint( QgsWkbTypes::Point, 2 / 3.0, 2 / 3.0 ) );
  exportPolygonFloat.addInteriorRing( ring );

  QString expectedJsonPrec3( QStringLiteral( "{\"type\": \"Polygon\", \"coordinates\": [[ [1.111, 1.111], [1.111, 11.111], [11.111, 11.111], [11.111, 1.111], [1.111, 1.111]], [ [0.667, 0.667], [0.667, 1.333], [1.333, 1.333], [1.333, 0.667], [0.667, 0.667]]] }" ) );
  QCOMPARE( exportPolygonFloat.asJSON( 3 ), expectedJsonPrec3 );

  // as GML2
  QString expectedGML2( QStringLiteral( "<Polygon xmlns=\"gml\"><outerBoundaryIs xmlns=\"gml\"><LinearRing xmlns=\"gml\"><coordinates xmlns=\"gml\" cs=\",\" ts=\" \">0,0 0,10 10,10 10,0 0,0</coordinates></LinearRing></outerBoundaryIs>" ) );
  expectedGML2 += QStringLiteral( "<innerBoundaryIs xmlns=\"gml\"><LinearRing xmlns=\"gml\"><coordinates xmlns=\"gml\" cs=\",\" ts=\" \">1,1 1,9 9,9 9,1 1,1</coordinates></LinearRing></innerBoundaryIs></Polygon>" );
  QCOMPARE( elemToString( exportPolygon.asGML2( doc ) ), expectedGML2 );
  QString expectedGML2prec3( QStringLiteral( "<Polygon xmlns=\"gml\"><outerBoundaryIs xmlns=\"gml\"><LinearRing xmlns=\"gml\"><coordinates xmlns=\"gml\" cs=\",\" ts=\" \">1.111,1.111 1.111,11.111 11.111,11.111 11.111,1.111 1.111,1.111</coordinates></LinearRing></outerBoundaryIs>" ) );
  expectedGML2prec3 += QStringLiteral( "<innerBoundaryIs xmlns=\"gml\"><LinearRing xmlns=\"gml\"><coordinates xmlns=\"gml\" cs=\",\" ts=\" \">0.667,0.667 0.667,1.333 1.333,1.333 1.333,0.667 0.667,0.667</coordinates></LinearRing></innerBoundaryIs></Polygon>" );
  QCOMPARE( elemToString( exportPolygonFloat.asGML2( doc, 3 ) ), expectedGML2prec3 );

  //as GML3
  QString expectedGML3( QStringLiteral( "<Polygon xmlns=\"gml\"><exterior xmlns=\"gml\"><LinearRing xmlns=\"gml\"><posList xmlns=\"gml\" srsDimension=\"2\">0 0 0 10 10 10 10 0 0 0</posList></LinearRing></exterior>" ) );
  expectedGML3 += QStringLiteral( "<interior xmlns=\"gml\"><LinearRing xmlns=\"gml\"><posList xmlns=\"gml\" srsDimension=\"2\">1 1 1 9 9 9 9 1 1 1</posList></LinearRing></interior></Polygon>" );

  QCOMPARE( elemToString( exportPolygon.asGML3( doc ) ), expectedGML3 );
  QString expectedGML3prec3( QStringLiteral( "<Polygon xmlns=\"gml\"><exterior xmlns=\"gml\"><LinearRing xmlns=\"gml\"><posList xmlns=\"gml\" srsDimension=\"2\">1.111 1.111 1.111 11.111 11.111 11.111 11.111 1.111 1.111 1.111</posList></LinearRing></exterior>" ) );
  expectedGML3prec3 += QStringLiteral( "<interior xmlns=\"gml\"><LinearRing xmlns=\"gml\"><posList xmlns=\"gml\" srsDimension=\"2\">0.667 0.667 0.667 1.333 1.333 1.333 1.333 0.667 0.667 0.667</posList></LinearRing></interior></Polygon>" );
  QCOMPARE( elemToString( exportPolygonFloat.asGML3( doc, 3 ) ), expectedGML3prec3 );

  //removing the fourth to last vertex removes the whole ring
  QgsPolygonV2 p20;
  QgsLineString *p20ExteriorRing = new QgsLineString();
  p20ExteriorRing->setPoints( QList<QgsPoint>() << QgsPoint( 0, 0 ) << QgsPoint( 1, 0 ) << QgsPoint( 1, 1 ) << QgsPoint( 0, 0 ) );
  p20.setExteriorRing( p20ExteriorRing );
  QVERIFY( p20.exteriorRing() );
  p20.deleteVertex( QgsVertexId( 0, 0, 2 ) );
  QVERIFY( !p20.exteriorRing() );

  //boundary
  QgsLineString boundary1;
  boundary1.setPoints( QList<QgsPoint>() << QgsPoint( 0, 0 ) << QgsPoint( 1, 0 ) << QgsPoint( 1, 1 )  << QgsPoint( 0, 0 ) );
  QgsPolygonV2 boundaryPolygon;
  QVERIFY( !boundaryPolygon.boundary() );

  boundaryPolygon.setExteriorRing( boundary1.clone() );
  QgsAbstractGeometry *boundary = boundaryPolygon.boundary();
  QgsLineString *lineBoundary = dynamic_cast< QgsLineString * >( boundary );
  QVERIFY( lineBoundary );
  QCOMPARE( lineBoundary->numPoints(), 4 );
  QCOMPARE( lineBoundary->xAt( 0 ), 0.0 );
  QCOMPARE( lineBoundary->xAt( 1 ), 1.0 );
  QCOMPARE( lineBoundary->xAt( 2 ), 1.0 );
  QCOMPARE( lineBoundary->xAt( 3 ), 0.0 );
  QCOMPARE( lineBoundary->yAt( 0 ), 0.0 );
  QCOMPARE( lineBoundary->yAt( 1 ), 0.0 );
  QCOMPARE( lineBoundary->yAt( 2 ), 1.0 );
  QCOMPARE( lineBoundary->yAt( 3 ), 0.0 );
  delete boundary;

  // add interior rings
  QgsLineString boundaryRing1;
  boundaryRing1.setPoints( QList<QgsPoint>() << QgsPoint( 0.1, 0.1 ) << QgsPoint( 0.2, 0.1 ) << QgsPoint( 0.2, 0.2 )  << QgsPoint( 0.1, 0.1 ) );
  QgsLineString boundaryRing2;
  boundaryRing2.setPoints( QList<QgsPoint>() << QgsPoint( 0.8, 0.8 ) << QgsPoint( 0.9, 0.8 ) << QgsPoint( 0.9, 0.9 )  << QgsPoint( 0.8, 0.8 ) );
  boundaryPolygon.setInteriorRings( QList< QgsCurve * >() << boundaryRing1.clone() << boundaryRing2.clone() );
  boundary = boundaryPolygon.boundary();
  QgsMultiLineString *multiLineBoundary = dynamic_cast< QgsMultiLineString * >( boundary );
  QVERIFY( multiLineBoundary );
  QCOMPARE( multiLineBoundary->numGeometries(), 3 );
  QCOMPARE( dynamic_cast< QgsLineString * >( multiLineBoundary->geometryN( 0 ) )->numPoints(), 4 );
  QCOMPARE( dynamic_cast< QgsLineString * >( multiLineBoundary->geometryN( 0 ) )->xAt( 0 ), 0.0 );
  QCOMPARE( dynamic_cast< QgsLineString * >( multiLineBoundary->geometryN( 0 ) )->xAt( 1 ), 1.0 );
  QCOMPARE( dynamic_cast< QgsLineString * >( multiLineBoundary->geometryN( 0 ) )->xAt( 2 ), 1.0 );
  QCOMPARE( dynamic_cast< QgsLineString * >( multiLineBoundary->geometryN( 0 ) )->xAt( 3 ), 0.0 );
  QCOMPARE( dynamic_cast< QgsLineString * >( multiLineBoundary->geometryN( 0 ) )->yAt( 0 ), 0.0 );
  QCOMPARE( dynamic_cast< QgsLineString * >( multiLineBoundary->geometryN( 0 ) )->yAt( 1 ), 0.0 );
  QCOMPARE( dynamic_cast< QgsLineString * >( multiLineBoundary->geometryN( 0 ) )->yAt( 2 ), 1.0 );
  QCOMPARE( dynamic_cast< QgsLineString * >( multiLineBoundary->geometryN( 0 ) )->yAt( 3 ), 0.0 );
  QCOMPARE( dynamic_cast< QgsLineString * >( multiLineBoundary->geometryN( 1 ) )->numPoints(), 4 );
  QCOMPARE( dynamic_cast< QgsLineString * >( multiLineBoundary->geometryN( 1 ) )->xAt( 0 ), 0.1 );
  QCOMPARE( dynamic_cast< QgsLineString * >( multiLineBoundary->geometryN( 1 ) )->xAt( 1 ), 0.2 );
  QCOMPARE( dynamic_cast< QgsLineString * >( multiLineBoundary->geometryN( 1 ) )->xAt( 2 ), 0.2 );
  QCOMPARE( dynamic_cast< QgsLineString * >( multiLineBoundary->geometryN( 1 ) )->xAt( 3 ), 0.1 );
  QCOMPARE( dynamic_cast< QgsLineString * >( multiLineBoundary->geometryN( 1 ) )->yAt( 0 ), 0.1 );
  QCOMPARE( dynamic_cast< QgsLineString * >( multiLineBoundary->geometryN( 1 ) )->yAt( 1 ), 0.1 );
  QCOMPARE( dynamic_cast< QgsLineString * >( multiLineBoundary->geometryN( 1 ) )->yAt( 2 ), 0.2 );
  QCOMPARE( dynamic_cast< QgsLineString * >( multiLineBoundary->geometryN( 1 ) )->yAt( 3 ), 0.1 );
  QCOMPARE( dynamic_cast< QgsLineString * >( multiLineBoundary->geometryN( 2 ) )->numPoints(), 4 );
  QCOMPARE( dynamic_cast< QgsLineString * >( multiLineBoundary->geometryN( 2 ) )->xAt( 0 ), 0.8 );
  QCOMPARE( dynamic_cast< QgsLineString * >( multiLineBoundary->geometryN( 2 ) )->xAt( 1 ), 0.9 );
  QCOMPARE( dynamic_cast< QgsLineString * >( multiLineBoundary->geometryN( 2 ) )->xAt( 2 ), 0.9 );
  QCOMPARE( dynamic_cast< QgsLineString * >( multiLineBoundary->geometryN( 2 ) )->xAt( 3 ), 0.8 );
  QCOMPARE( dynamic_cast< QgsLineString * >( multiLineBoundary->geometryN( 2 ) )->yAt( 0 ), 0.8 );
  QCOMPARE( dynamic_cast< QgsLineString * >( multiLineBoundary->geometryN( 2 ) )->yAt( 1 ), 0.8 );
  QCOMPARE( dynamic_cast< QgsLineString * >( multiLineBoundary->geometryN( 2 ) )->yAt( 2 ), 0.9 );
  QCOMPARE( dynamic_cast< QgsLineString * >( multiLineBoundary->geometryN( 2 ) )->yAt( 3 ), 0.8 );
  boundaryPolygon.setInteriorRings( QList< QgsCurve * >() );
  delete boundary;

  //test boundary with z
  boundary1.setPoints( QList<QgsPoint>() << QgsPoint( QgsWkbTypes::PointZ, 0, 0, 10 ) << QgsPoint( QgsWkbTypes::PointZ, 1, 0, 15 )
                       << QgsPoint( QgsWkbTypes::PointZ, 1, 1, 20 )  << QgsPoint( QgsWkbTypes::PointZ, 0, 0, 10 ) );
  boundaryPolygon.setExteriorRing( boundary1.clone() );
  boundary = boundaryPolygon.boundary();
  lineBoundary = dynamic_cast< QgsLineString * >( boundary );
  QVERIFY( lineBoundary );
  QCOMPARE( lineBoundary->numPoints(), 4 );
  QCOMPARE( lineBoundary->wkbType(), QgsWkbTypes::LineStringZ );
  QCOMPARE( lineBoundary->zAt( 0 ), 10.0 );
  QCOMPARE( lineBoundary->zAt( 1 ), 15.0 );
  QCOMPARE( lineBoundary->zAt( 2 ), 20.0 );
  QCOMPARE( lineBoundary->zAt( 3 ), 10.0 );
  delete boundary;

  // point distance to boundary

  QgsLineString pd1;
  pd1.setPoints( QList<QgsPoint>() << QgsPoint( 0, 0 ) << QgsPoint( 1, 0 ) << QgsPoint( 1, 1 ) << QgsPoint( 0, 1 ) << QgsPoint( 0, 0 ) );
  QgsPolygonV2 pd;
  // no meaning, but let's not crash
  ( void )pd.pointDistanceToBoundary( 0, 0 );

  pd.setExteriorRing( pd1.clone() );
  QGSCOMPARENEAR( pd.pointDistanceToBoundary( 0, 0.5 ), 0.0, 0.0000000001 );
  QGSCOMPARENEAR( pd.pointDistanceToBoundary( 0.1, 0.5 ), 0.1, 0.0000000001 );
  QGSCOMPARENEAR( pd.pointDistanceToBoundary( -0.1, 0.5 ), -0.1, 0.0000000001 );
  // with a ring
  QgsLineString pdRing1;
  pdRing1.setPoints( QList<QgsPoint>() << QgsPoint( 0.1, 0.1 ) << QgsPoint( 0.2, 0.1 ) << QgsPoint( 0.2, 0.6 )  << QgsPoint( 0.1, 0.6 ) << QgsPoint( 0.1, 0.1 ) );
  pd.setInteriorRings( QList< QgsCurve * >() << pdRing1.clone() );
  QGSCOMPARENEAR( pd.pointDistanceToBoundary( 0, 0.5 ), 0.0, 0.0000000001 );
  QGSCOMPARENEAR( pd.pointDistanceToBoundary( 0.1, 0.5 ), 0.0, 0.0000000001 );
  QGSCOMPARENEAR( pd.pointDistanceToBoundary( 0.01, 0.5 ), 0.01, 0.0000000001 );
  QGSCOMPARENEAR( pd.pointDistanceToBoundary( 0.08, 0.5 ), 0.02, 0.0000000001 );
  QGSCOMPARENEAR( pd.pointDistanceToBoundary( 0.12, 0.5 ), -0.02, 0.0000000001 );
  QGSCOMPARENEAR( pd.pointDistanceToBoundary( -0.1, 0.5 ), -0.1, 0.0000000001 );

  // remove interior rings
  QgsLineString removeRingsExt;
  removeRingsExt.setPoints( QList<QgsPoint>() << QgsPoint( 0, 0 ) << QgsPoint( 1, 0 ) << QgsPoint( 1, 1 )  << QgsPoint( 0, 0 ) );
  QgsPolygonV2 removeRings1;
  removeRings1.removeInteriorRings();

  removeRings1.setExteriorRing( boundary1.clone() );
  removeRings1.removeInteriorRings();
  QCOMPARE( removeRings1.numInteriorRings(), 0 );

  // add interior rings
  QgsLineString removeRingsRing1;
  removeRingsRing1.setPoints( QList<QgsPoint>() << QgsPoint( 0.1, 0.1 ) << QgsPoint( 0.2, 0.1 ) << QgsPoint( 0.2, 0.2 )  << QgsPoint( 0.1, 0.1 ) );
  QgsLineString removeRingsRing2;
  removeRingsRing1.setPoints( QList<QgsPoint>() << QgsPoint( 0.6, 0.8 ) << QgsPoint( 0.9, 0.8 ) << QgsPoint( 0.9, 0.9 )  << QgsPoint( 0.6, 0.8 ) );
  removeRings1.setInteriorRings( QList< QgsCurve * >() << removeRingsRing1.clone() << removeRingsRing2.clone() );

  // remove ring with size filter
  removeRings1.removeInteriorRings( 0.0075 );
  QCOMPARE( removeRings1.numInteriorRings(), 1 );

  // remove ring with no size filter
  removeRings1.removeInteriorRings();
  QCOMPARE( removeRings1.numInteriorRings(), 0 );

}

void TestQgsGeometry::triangle()
{
  //test constructor
  QgsTriangle t1;
  QVERIFY( t1.isEmpty() );
  QCOMPARE( t1.numInteriorRings(), 0 );
  QCOMPARE( t1.nCoordinates(), 0 );
  QCOMPARE( t1.ringCount(), 0 );
  QCOMPARE( t1.partCount(), 0 );
  QVERIFY( !t1.is3D() );
  QVERIFY( !t1.isMeasure() );
  QCOMPARE( t1.wkbType(), QgsWkbTypes::Triangle );
  QCOMPARE( t1.wktTypeStr(), QString( "Triangle" ) );
  QCOMPARE( t1.geometryType(), QString( "Triangle" ) );
  QCOMPARE( t1.dimension(), 2 );
  QVERIFY( !t1.hasCurvedSegments() );
  QCOMPARE( t1.area(), 0.0 );
  QCOMPARE( t1.perimeter(), 0.0 );
  QVERIFY( !t1.exteriorRing() );
  QVERIFY( !t1.interiorRing( 0 ) );

  //set exterior ring

  //try with no ring
  QgsLineString *ext = 0;
  t1.setExteriorRing( ext );
  QVERIFY( t1.isEmpty() );
  QCOMPARE( t1.numInteriorRings(), 0 );
  QCOMPARE( t1.nCoordinates(), 0 );
  QCOMPARE( t1.ringCount(), 0 );
  QCOMPARE( t1.partCount(), 0 );
  QVERIFY( !t1.exteriorRing() );
  QVERIFY( !t1.interiorRing( 0 ) );
  QCOMPARE( t1.wkbType(), QgsWkbTypes::Triangle );

  //valid exterior ring
  ext = new QgsLineString();
  ext->setPoints( QgsPointSequence() << QgsPoint( 0, 0 ) << QgsPoint( 0, 10 ) << QgsPoint( 10, 10 )
                  << QgsPoint( 0, 0 ) );
  QVERIFY( ext->isClosed() );
  t1.setExteriorRing( ext );
  QVERIFY( !t1.isEmpty() );
  QCOMPARE( t1.numInteriorRings(), 0 );
  QCOMPARE( t1.nCoordinates(), 4 );
  QCOMPARE( t1.ringCount(), 1 );
  QCOMPARE( t1.partCount(), 1 );
  QVERIFY( !t1.is3D() );
  QVERIFY( !t1.isMeasure() );
  QCOMPARE( t1.wkbType(), QgsWkbTypes::Triangle );
  QCOMPARE( t1.wktTypeStr(), QString( "Triangle" ) );
  QCOMPARE( t1.geometryType(), QString( "Triangle" ) );
  QCOMPARE( t1.dimension(), 2 );
  QVERIFY( !t1.hasCurvedSegments() );
  QCOMPARE( t1.area(), 50.0 );
  QGSCOMPARENEAR( t1.perimeter(), 34.1421, 0.001 );
  QVERIFY( t1.exteriorRing() );
  QVERIFY( !t1.interiorRing( 0 ) );

  //retrieve exterior ring and check
  QCOMPARE( *( static_cast< const QgsLineString * >( t1.exteriorRing() ) ), *ext );

  //set new ExteriorRing
  ext = new QgsLineString();
  ext->setPoints( QgsPointSequence() << QgsPoint( 0, 10 ) << QgsPoint( 5, 5 ) << QgsPoint( 10, 10 )
                  << QgsPoint( 0, 10 ) );
  QVERIFY( ext->isClosed() );
  t1.setExteriorRing( ext );
  QVERIFY( !t1.isEmpty() );
  QCOMPARE( t1.numInteriorRings(), 0 );
  QCOMPARE( t1.nCoordinates(), 4 );
  QCOMPARE( t1.ringCount(), 1 );
  QCOMPARE( t1.partCount(), 1 );
  QVERIFY( !t1.is3D() );
  QVERIFY( !t1.isMeasure() );
  QCOMPARE( t1.wkbType(), QgsWkbTypes::Triangle );
  QCOMPARE( t1.wktTypeStr(), QString( "Triangle" ) );
  QCOMPARE( t1.geometryType(), QString( "Triangle" ) );
  QCOMPARE( t1.dimension(), 2 );
  QVERIFY( !t1.hasCurvedSegments() );
  QCOMPARE( t1.area(), 25.0 );
  QGSCOMPARENEAR( t1.perimeter(), 24.1421, 0.001 );
  QVERIFY( t1.exteriorRing() );
  QVERIFY( !t1.interiorRing( 0 ) );
  QCOMPARE( *( static_cast< const QgsLineString * >( t1.exteriorRing() ) ), *ext );

  //test that a non closed exterior ring will be automatically closed
  QgsTriangle t2;
  ext = new QgsLineString();
  ext->setPoints( QgsPointSequence() << QgsPoint( 0, 0 ) << QgsPoint( 0, 10 ) << QgsPoint( 10, 10 ) );
  QVERIFY( !ext->isClosed() );
  t2.setExteriorRing( ext );
  QVERIFY( !t2.isEmpty() );
  QVERIFY( t2.exteriorRing()->isClosed() );
  QCOMPARE( t2.nCoordinates(), 4 );

  // invalid number of points
  ext = new QgsLineString();
  t2.clear();
  ext->setPoints( QgsPointSequence() << QgsPoint( 0, 0 ) << QgsPoint( 0, 10 ) );
  t2.setExteriorRing( ext );
  QVERIFY( t2.isEmpty() );

  ext = new QgsLineString();
  t2.clear();
  ext->setPoints( QgsPointSequence() << QgsPoint( 0, 0 ) << QgsPoint( 0, 10 ) << QgsPoint( 10, 10 ) << QgsPoint( 5, 10 ) << QgsPoint( 8, 10 ) );
  t2.setExteriorRing( ext );
  QVERIFY( t2.isEmpty() );

  // invalid exterior ring
  ext = new QgsLineString();
  t2.clear();
  ext->setPoints( QgsPointSequence() << QgsPoint( 0, 0 ) << QgsPoint( 0, 10 ) << QgsPoint( 10, 10 ) << QgsPoint( 5, 10 ) );
  t2.setExteriorRing( ext );
  QVERIFY( t2.isEmpty() );

  // circular ring
  QgsCircularString *circularRing = new QgsCircularString();
  t2.clear();
  circularRing->setPoints( QgsPointSequence() << QgsPoint( 0, 0 ) << QgsPoint( 0, 10 ) << QgsPoint( 10, 10 ) );
  QVERIFY( circularRing->hasCurvedSegments() );
  t2.setExteriorRing( circularRing );
  QVERIFY( t2.isEmpty() );

  //constructor with 3 points
  // double points
  QgsTriangle t3( QgsPoint( 0, 0 ), QgsPoint( 0, 0 ), QgsPoint( 10, 10 ) );
  QVERIFY( t3.isEmpty() );
  QCOMPARE( t3.numInteriorRings(), 0 );
  QCOMPARE( t3.nCoordinates(), 0 );
  QCOMPARE( t3.ringCount(), 0 );
  QCOMPARE( t3.partCount(), 0 );
  QVERIFY( !t3.is3D() );
  QVERIFY( !t3.isMeasure() );
  QCOMPARE( t3.wkbType(), QgsWkbTypes::Triangle );
  QCOMPARE( t3.wktTypeStr(), QString( "Triangle" ) );
  QCOMPARE( t3.geometryType(), QString( "Triangle" ) );
  QCOMPARE( t3.dimension(), 2 );
  QVERIFY( !t3.hasCurvedSegments() );
  QCOMPARE( t3.area(), 0.0 );
  QCOMPARE( t3.perimeter(), 0.0 );
  QVERIFY( !t3.exteriorRing() );
  QVERIFY( !t3.interiorRing( 0 ) );

  // colinear
  t3 = QgsTriangle( QgsPoint( 0, 0 ), QgsPoint( 0, 5 ), QgsPoint( 0, 10 ) );
  QVERIFY( t3.isEmpty() );
  QCOMPARE( t3.numInteriorRings(), 0 );
  QCOMPARE( t3.nCoordinates(), 0 );
  QCOMPARE( t3.ringCount(), 0 );
  QCOMPARE( t3.partCount(), 0 );
  QVERIFY( !t3.is3D() );
  QVERIFY( !t3.isMeasure() );
  QCOMPARE( t3.wkbType(), QgsWkbTypes::Triangle );
  QCOMPARE( t3.wktTypeStr(), QString( "Triangle" ) );
  QCOMPARE( t3.geometryType(), QString( "Triangle" ) );
  QCOMPARE( t3.dimension(), 2 );
  QVERIFY( !t3.hasCurvedSegments() );
  QCOMPARE( t3.area(), 0.0 );
  QCOMPARE( t3.perimeter(), 0.0 );
  QVERIFY( !t3.exteriorRing() );
  QVERIFY( !t3.interiorRing( 0 ) );

  t3 = QgsTriangle( QgsPoint( 0, 0 ), QgsPoint( 0, 10 ), QgsPoint( 10, 10 ) );
  QVERIFY( !t3.isEmpty() );
  QCOMPARE( t3.numInteriorRings(), 0 );
  QCOMPARE( t3.nCoordinates(), 4 );
  QCOMPARE( t3.ringCount(), 1 );
  QCOMPARE( t3.partCount(), 1 );
  QVERIFY( !t3.is3D() );
  QVERIFY( !t3.isMeasure() );
  QCOMPARE( t3.wkbType(), QgsWkbTypes::Triangle );
  QCOMPARE( t3.wktTypeStr(), QString( "Triangle" ) );
  QCOMPARE( t3.geometryType(), QString( "Triangle" ) );
  QCOMPARE( t3.dimension(), 2 );
  QVERIFY( !t3.hasCurvedSegments() );
  QCOMPARE( t3.area(), 50.0 );
  QGSCOMPARENEAR( t3.perimeter(), 34.1421, 0.001 );
  QVERIFY( t3.exteriorRing() );
  QVERIFY( !t3.interiorRing( 0 ) );

  // equality
  QVERIFY( QgsTriangle() == QgsTriangle() ); // empty
  QVERIFY( QgsTriangle() == QgsTriangle( QgsPoint( 0, 0 ), QgsPoint( 0, 5 ), QgsPoint( 0, 10 ) ) ); // empty
  QVERIFY( QgsTriangle( QgsPoint( 0, 0 ), QgsPoint( 0, 5 ), QgsPoint( 0, 10 ) ) == QgsTriangle() ); // empty
  QVERIFY( QgsTriangle() != QgsTriangle( QgsPoint( 0, 0 ), QgsPoint( 5, 5 ), QgsPoint( 0, 10 ) ) );
  QVERIFY( QgsTriangle( QgsPoint( 0, 0 ), QgsPoint( 5, 5 ), QgsPoint( 0, 10 ) ) != QgsTriangle() );
  QVERIFY( QgsTriangle( QgsPoint( 0, 0 ), QgsPoint( 5, 5 ), QgsPoint( 0, 10 ) ) != QgsTriangle( QgsPoint( 0, 10 ), QgsPoint( 5, 5 ), QgsPoint( 0, 0 ) ) );

  // clone
  QgsTriangle *t4 = t3.clone();
  QCOMPARE( t3, *t4 );
  delete t4;

  // constructor from QgsPointXY and QPointF
  QgsTriangle t_qgspoint = QgsTriangle( QgsPointXY( 0, 0 ), QgsPointXY( 0, 10 ), QgsPointXY( 10, 10 ) );
  QVERIFY( t3 == t_qgspoint );
  QgsTriangle t_pointf = QgsTriangle( QPointF( 0, 0 ), QPointF( 0, 10 ), QPointF( 10, 10 ) );
  QVERIFY( t3 == t_pointf );

  // fromWkt
  QgsTriangle t5;
  ext = new QgsLineString();
  ext->setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 0, 0, 1, 5 )
                  << QgsPoint( QgsWkbTypes::PointZM, 0, 10, 2, 6 ) << QgsPoint( QgsWkbTypes::PointZM, 10, 10, 3, 7 ) );
  t5.setExteriorRing( ext );
  QString wkt = t5.asWkt();
  QVERIFY( !wkt.isEmpty() );
  QgsTriangle t6;
  QVERIFY( t6.fromWkt( wkt ) );
  QCOMPARE( t5, t6 );

  // conversion
  QgsPolygonV2 p1;
  ext = new QgsLineString();
  ext->setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 0, 0, 1, 5 )
                  << QgsPoint( QgsWkbTypes::PointZM, 0, 10, 2, 6 ) << QgsPoint( QgsWkbTypes::PointZM, 10, 10, 3, 7 ) );
  p1.setExteriorRing( ext );
  //toPolygon
  std::unique_ptr< QgsPolygonV2 > poly( t5.toPolygon() );
  QCOMPARE( *poly, p1 );
  //surfaceToPolygon
  std::unique_ptr< QgsPolygonV2 > surface( t5.surfaceToPolygon() );
  QCOMPARE( *surface, p1 );

  //bad WKT
  QVERIFY( !t6.fromWkt( "Point()" ) );
  QVERIFY( t6.isEmpty() );
  QVERIFY( !t6.exteriorRing() );
  QCOMPARE( t6.numInteriorRings(), 0 );
  QVERIFY( !t6.is3D() );
  QVERIFY( !t6.isMeasure() );
  QCOMPARE( t6.wkbType(), QgsWkbTypes::Triangle );

  // WKB
  QByteArray wkb = t5.asWkb();
  t6.clear();
  QgsConstWkbPtr wkb16ptr5( wkb );
  t6.fromWkb( wkb16ptr5 );
  QCOMPARE( t5.wkbType(), QgsWkbTypes::TriangleZM );
  QCOMPARE( t5, t6 );

  //bad WKB - check for no crash
  t6.clear();
  QgsConstWkbPtr nullPtr( nullptr, 0 );
  QVERIFY( !t6.fromWkb( nullPtr ) );
  QCOMPARE( t6.wkbType(), QgsWkbTypes::Triangle );
  QgsPoint point( 1, 2 );
  QByteArray wkbPoint = point.asWkb();
  QgsConstWkbPtr wkbPointPtr( wkbPoint );
  QVERIFY( !t6.fromWkb( wkbPointPtr ) );
  QCOMPARE( t6.wkbType(), QgsWkbTypes::Triangle );

  // lengths and angles
  QgsTriangle t7( QgsPoint( 0, 0 ), QgsPoint( 0, 5 ), QgsPoint( 5, 5 ) );

  QVector<double> a_tested, a_t7 = t7.angles();
  a_tested.append( M_PI / 4.0 );
  a_tested.append( M_PI / 2.0 );
  a_tested.append( M_PI / 4.0 );
  QGSCOMPARENEAR( a_tested.at( 0 ), a_t7.at( 0 ), 0.0001 );
  QGSCOMPARENEAR( a_tested.at( 1 ), a_t7.at( 1 ), 0.0001 );
  QGSCOMPARENEAR( a_tested.at( 2 ), a_t7.at( 2 ), 0.0001 );

  QVector<double> l_tested, l_t7 = t7.lengths();
  l_tested.append( 5 );
  l_tested.append( 5 );
  l_tested.append( sqrt( 5 * 5 + 5 * 5 ) );
  QGSCOMPARENEAR( l_tested.at( 0 ), l_t7.at( 0 ), 0.0001 );
  QGSCOMPARENEAR( l_tested.at( 1 ), l_t7.at( 1 ), 0.0001 );
  QGSCOMPARENEAR( l_tested.at( 2 ), l_t7.at( 2 ), 0.0001 );

  // type of triangle
  QVERIFY( t7.isRight() );
  QVERIFY( t7.isIsocele() );
  QVERIFY( !t7.isScalene() );
  QVERIFY( !t7.isEquilateral() );

  QgsTriangle t8( QgsPoint( 7.2825, 4.2368 ), QgsPoint( 13.0058, 3.3218 ), QgsPoint( 9.2145, 6.5242 ) );
  // angles in radians 58.8978;31.1036;89.9985
  // length 5.79598;4.96279;2.99413
  QVERIFY( t8.isRight() );
  QVERIFY( !t8.isIsocele() );
  QVERIFY( t8.isScalene() );
  QVERIFY( !t8.isEquilateral() );

  QgsTriangle t9( QgsPoint( 10, 10 ), QgsPoint( 16, 10 ), QgsPoint( 13, 15.1962 ) );
  QVERIFY( !t9.isRight() );
  QVERIFY( t9.isIsocele() );
  QVERIFY( !t9.isScalene() );
  QVERIFY( t9.isEquilateral() );

  // vertex
  QCOMPARE( t9.vertexAt( -1 ), QgsPoint( 0, 0 ) );
  QCOMPARE( t9.vertexAt( 0 ), QgsPoint( 10, 10 ) );
  QCOMPARE( t9.vertexAt( 1 ), QgsPoint( 16, 10 ) );
  QCOMPARE( t9.vertexAt( 2 ), QgsPoint( 13, 15.1962 ) );
  QCOMPARE( t9.vertexAt( 3 ), QgsPoint( 10, 10 ) );
  QCOMPARE( t9.vertexAt( 4 ), QgsPoint( 0, 0 ) );

  // altitudes
  QgsTriangle t10( QgsPoint( 20, 2 ), QgsPoint( 16, 6 ), QgsPoint( 26, 2 ) );
  QVector<QgsLineString> alt = t10.altitudes();
  QGSCOMPARENEARPOINT( alt.at( 0 ).pointN( 1 ), QgsPoint( 20.8276, 4.0690 ), 0.0001 );
  QGSCOMPARENEARPOINT( alt.at( 1 ).pointN( 1 ), QgsPoint( 16, 2 ), 0.0001 );
  QGSCOMPARENEARPOINT( alt.at( 2 ).pointN( 1 ), QgsPoint( 23, -1 ), 0.0001 );

  // orthocenter
  QCOMPARE( QgsPoint( 16, -8 ), t10.orthocenter() );
  QCOMPARE( QgsPoint( 0, 5 ), t7.orthocenter() );
  QGSCOMPARENEARPOINT( QgsPoint( 13, 11.7321 ), t9.orthocenter(), 0.0001 );

  // circumscribed circle
  QCOMPARE( QgsPoint( 2.5, 2.5 ), t7.circumscribedCenter() );
  QGSCOMPARENEAR( 3.5355, t7.circumscribedRadius(), 0.0001 );
  QCOMPARE( QgsPoint( 23, 9 ), t10.circumscribedCenter() );
  QGSCOMPARENEAR( 7.6158, t10.circumscribedRadius(), 0.0001 );
  QGSCOMPARENEARPOINT( QgsPoint( 13, 11.7321 ), t9.circumscribedCenter(), 0.0001 );
  QGSCOMPARENEAR( 3.4641, t9.circumscribedRadius(), 0.0001 );
  QGSCOMPARENEARPOINT( QgsPoint( 13, 11.7321 ), t9.circumscribedCircle().center(), 0.0001 );
  QGSCOMPARENEAR( 3.4641, t9.circumscribedCircle().radius(), 0.0001 );

  // inscribed circle
  QGSCOMPARENEARPOINT( QgsPoint( 1.4645, 3.5355 ), t7.inscribedCenter(), 0.001 );
  QGSCOMPARENEAR( 1.4645, t7.inscribedRadius(), 0.0001 );
  QGSCOMPARENEARPOINT( QgsPoint( 20.4433, 3.0701 ), t10.inscribedCenter(), 0.001 );
  QGSCOMPARENEAR( 1.0701, t10.inscribedRadius(), 0.0001 );
  QGSCOMPARENEARPOINT( QgsPoint( 13, 11.7321 ), t9.inscribedCenter(), 0.0001 );
  QGSCOMPARENEAR( 1.7321, t9.inscribedRadius(), 0.0001 );
  QGSCOMPARENEARPOINT( QgsPoint( 13, 11.7321 ), t9.inscribedCircle().center(), 0.0001 );
  QGSCOMPARENEAR( 1.7321, t9.inscribedCircle().radius(), 0.0001 );

  // medians
  QVector<QgsLineString> med = t7.medians();
  QCOMPARE( med.at( 0 ).pointN( 0 ), t7.vertexAt( 0 ) );
  QGSCOMPARENEARPOINT( med.at( 0 ).pointN( 1 ), QgsPoint( 2.5, 5 ), 0.0001 );
  QCOMPARE( med.at( 1 ).pointN( 0 ), t7.vertexAt( 1 ) );
  QGSCOMPARENEARPOINT( med.at( 1 ).pointN( 1 ), QgsPoint( 2.5, 2.5 ), 0.0001 );
  QCOMPARE( med.at( 2 ).pointN( 0 ), t7.vertexAt( 2 ) );
  QGSCOMPARENEARPOINT( med.at( 2 ).pointN( 1 ), QgsPoint( 0, 2.5 ), 0.0001 );
  med.clear();

  med = t10.medians();
  QCOMPARE( med.at( 0 ).pointN( 0 ), t10.vertexAt( 0 ) );
  QGSCOMPARENEARPOINT( med.at( 0 ).pointN( 1 ), QgsPoint( 21, 4 ), 0.0001 );
  QCOMPARE( med.at( 1 ).pointN( 0 ), t10.vertexAt( 1 ) );
  QGSCOMPARENEARPOINT( med.at( 1 ).pointN( 1 ), QgsPoint( 23, 2 ), 0.0001 );
  QCOMPARE( med.at( 2 ).pointN( 0 ), t10.vertexAt( 2 ) );
  QGSCOMPARENEARPOINT( med.at( 2 ).pointN( 1 ), QgsPoint( 18, 4 ), 0.0001 );
  med.clear();
  alt.clear();

  med = t9.medians();
  alt = t9.altitudes();
  QGSCOMPARENEARPOINT( med.at( 0 ).pointN( 0 ), alt.at( 0 ).pointN( 0 ), 0.0001 );
  QGSCOMPARENEARPOINT( med.at( 0 ).pointN( 1 ), alt.at( 0 ).pointN( 1 ), 0.0001 );
  QGSCOMPARENEARPOINT( med.at( 1 ).pointN( 0 ), alt.at( 1 ).pointN( 0 ), 0.0001 );
  QGSCOMPARENEARPOINT( med.at( 1 ).pointN( 1 ), alt.at( 1 ).pointN( 1 ), 0.0001 );
  QGSCOMPARENEARPOINT( med.at( 2 ).pointN( 0 ), alt.at( 2 ).pointN( 0 ), 0.0001 );
  QGSCOMPARENEARPOINT( med.at( 2 ).pointN( 1 ), alt.at( 2 ).pointN( 1 ), 0.0001 );

  // medial
  QCOMPARE( t7.medial(), QgsTriangle( QgsPoint( 0, 2.5 ), QgsPoint( 2.5, 5 ), QgsPoint( 2.5, 2.5 ) ) );
  QCOMPARE( t9.medial(), QgsTriangle( QgsGeometryUtils::midpoint( t9.vertexAt( 0 ), t9.vertexAt( 1 ) ),
                                      QgsGeometryUtils::midpoint( t9.vertexAt( 1 ), t9.vertexAt( 2 ) ),
                                      QgsGeometryUtils::midpoint( t9.vertexAt( 2 ), t9.vertexAt( 0 ) ) ) );

  // bisectors
  QVector<QgsLineString> bis = t7.bisectors();
  QCOMPARE( bis.at( 0 ).pointN( 0 ), t7.vertexAt( 0 ) );
  QGSCOMPARENEARPOINT( bis.at( 0 ).pointN( 1 ), QgsPoint( 2.0711, 5 ), 0.0001 );
  QCOMPARE( bis.at( 1 ).pointN( 0 ), t7.vertexAt( 1 ) );
  QGSCOMPARENEARPOINT( bis.at( 1 ).pointN( 1 ), QgsPoint( 2.5, 2.5 ), 0.0001 );
  QCOMPARE( bis.at( 2 ).pointN( 0 ), t7.vertexAt( 2 ) );
  QGSCOMPARENEARPOINT( bis.at( 2 ).pointN( 1 ), QgsPoint( 0, 2.9289 ), 0.0001 );

  // "deleted" method
  QgsTriangle t11( QgsPoint( 0, 0 ), QgsPoint( 100, 100 ), QgsPoint( 0, 200 ) );
  ext->setPoints( QgsPointSequence() << QgsPoint( 5, 5 )
                  << QgsPoint( 50, 50 ) << QgsPoint( 0, 25 )
                  << QgsPoint( 5, 5 ) );
  t11.addInteriorRing( ext );
  QCOMPARE( t11.asWkt(), QString( "Triangle ((0 0, 100 100, 0 200, 0 0))" ) );

  /* QList<QgsCurve *> lc;
   lc.append(ext);
   t11.setInteriorRings( lc );
   QCOMPARE( t11.asWkt(), QString( "Triangle ((0 0, 100 100, 0 200, 0 0))" ) );*/

  QgsVertexId id( 0, 0, 1 );
  QVERIFY( !t11.deleteVertex( id ) );
  QCOMPARE( t11.asWkt(), QString( "Triangle ((0 0, 100 100, 0 200, 0 0))" ) );
  QVERIFY( !t11.insertVertex( id, QgsPoint( 5, 5 ) ) );
  QCOMPARE( t11.asWkt(), QString( "Triangle ((0 0, 100 100, 0 200, 0 0))" ) );

  //move vertex
  QgsPoint pt1( 5, 5 );
  // invalid part
  id.part = -1;
  QVERIFY( !t11.moveVertex( id, pt1 ) );
  id.part = 1;
  QVERIFY( !t11.moveVertex( id, pt1 ) );
  // invalid ring
  id.part = 0;
  id.ring = -1;
  QVERIFY( !t11.moveVertex( id, pt1 ) );
  id.ring = 1;
  QVERIFY( !t11.moveVertex( id, pt1 ) );
  id.ring = 0;
  id.vertex = -1;
  QVERIFY( !t11.moveVertex( id, pt1 ) );
  id.vertex = 5;
  QVERIFY( !t11.moveVertex( id, pt1 ) );

  // valid vertex
  id.vertex = 0;
  QVERIFY( t11.moveVertex( id, pt1 ) );
  QCOMPARE( t11.asWkt(), QString( "Triangle ((5 5, 100 100, 0 200, 5 5))" ) );
  pt1 = QgsPoint();
  QVERIFY( t11.moveVertex( id, pt1 ) );
  QCOMPARE( t11.asWkt(), QString( "Triangle ((0 0, 100 100, 0 200, 0 0))" ) );
  id.vertex = 4;
  pt1 = QgsPoint( 5, 5 );
  QVERIFY( t11.moveVertex( id, pt1 ) );
  QCOMPARE( t11.asWkt(), QString( "Triangle ((5 5, 100 100, 0 200, 5 5))" ) );
  pt1 = QgsPoint();
  QVERIFY( t11.moveVertex( id, pt1 ) );
  QCOMPARE( t11.asWkt(), QString( "Triangle ((0 0, 100 100, 0 200, 0 0))" ) );
  id.vertex = 1;
  pt1 = QgsPoint( 5, 5 );
  QVERIFY( t11.moveVertex( id, pt1 ) );
  QCOMPARE( t11.asWkt(), QString( "Triangle ((0 0, 5 5, 0 200, 0 0))" ) );
  // colinear
  pt1 = QgsPoint( 0, 100 );
  QVERIFY( !t11.moveVertex( id, pt1 ) );
  // duplicate point
  pt1 = QgsPoint( 0, 0 );
  QVERIFY( !t11.moveVertex( id, pt1 ) );

}

void TestQgsGeometry::ellipse()
{
  //test constructors
  QgsEllipse elp1;
  QVERIFY( elp1.center() == QgsPoint() );
  QCOMPARE( elp1.semiMajorAxis(), 0.0 );
  QCOMPARE( elp1.semiMinorAxis(), 0.0 );
  QCOMPARE( elp1.azimuth(), 90.0 );
  QVERIFY( elp1.isEmpty() );

  QgsEllipse elp2( QgsPoint( 5, 10 ), 3, 2 );
  QVERIFY( elp2.center() == QgsPoint( 5, 10 ) );
  QCOMPARE( elp2.semiMajorAxis(), 3.0 );
  QCOMPARE( elp2.semiMinorAxis(), 2.0 );
  QCOMPARE( elp2.azimuth(), 90.0 );
  QVERIFY( !elp2.isEmpty() );

  QgsEllipse elp3( QgsPoint( 5, 10 ), 3, 2, 45 );
  QVERIFY( elp3.center() == QgsPoint( 5, 10 ) );
  QCOMPARE( elp3.semiMajorAxis(), 3.0 );
  QCOMPARE( elp3.semiMinorAxis(), 2.0 );
  QCOMPARE( elp3.azimuth(), 45.0 );
  QVERIFY( !elp3.isEmpty() );

  QgsEllipse elp4( QgsPoint( 5, 10 ), 2, 3, 45 );
  QVERIFY( elp4.center() == QgsPoint( 5, 10 ) );
  QCOMPARE( elp4.semiMajorAxis(), 3.0 );
  QCOMPARE( elp4.semiMinorAxis(), 2.0 );
  QCOMPARE( elp4.azimuth(), 135.0 );
  QVERIFY( !elp4.isEmpty() );

  //test toString
  QCOMPARE( elp1.toString(), QString( "Empty" ) );
  QCOMPARE( elp2.toString(), QString( "Ellipse (Center: Point (5 10), Semi-Major Axis: 3, Semi-Minor Axis: 2, Azimuth: 90)" ) );
  QCOMPARE( elp3.toString(), QString( "Ellipse (Center: Point (5 10), Semi-Major Axis: 3, Semi-Minor Axis: 2, Azimuth: 45)" ) );
  QCOMPARE( elp4.toString(), QString( "Ellipse (Center: Point (5 10), Semi-Major Axis: 3, Semi-Minor Axis: 2, Azimuth: 135)" ) );

  //test equality operator
  QVERIFY( QgsEllipse() == QgsEllipse( QgsPoint(), 0, 0, 90 ) );
  QVERIFY( !( QgsEllipse() == QgsEllipse( QgsPoint(), 0, 0, 0.0005 ) ) );
  QVERIFY( elp2 == QgsEllipse( QgsPoint( 5, 10 ), 2, 3, 0 ) );
  QVERIFY( elp2 != elp3 );
  QVERIFY( elp3 != elp4 );
  QVERIFY( elp4 == QgsEllipse( QgsPoint( 5, 10 ), 3, 2, 90 + 45 ) );

  //test setter and getter
  elp1.setAzimuth( 45 );
  QCOMPARE( elp1.azimuth(), 45.0 );

  elp1.setSemiMajorAxis( 50 );
  QCOMPARE( elp1.semiMajorAxis(), 50.0 );

  // axis_b > axis_a
  elp1.setSemiMinorAxis( 70 );
  QCOMPARE( elp1.semiMajorAxis(), 70.0 );
  QCOMPARE( elp1.semiMinorAxis(), 50.0 );
  // axis_b < axis_a
  elp1.setSemiMinorAxis( 3 );
  QCOMPARE( elp1.semiMinorAxis(), 3.0 );
  QCOMPARE( elp1.semiMajorAxis(), 70.0 );

  elp1.setSemiMajorAxis( 2 );
  QCOMPARE( elp1.semiMinorAxis(), 2.0 );
  QCOMPARE( elp1.semiMajorAxis(), 3.0 );

  elp1.setCenter( QgsPoint( 5, 10 ) );
  QVERIFY( elp1.center() == QgsPoint( 5, 10 ) );
  QVERIFY( elp1.rcenter() == QgsPoint( 5, 10 ) );
  elp1.rcenter() = QgsPoint( 25, 310 );
  QVERIFY( elp1.center() == QgsPoint( 25, 310 ) );

  //test "alt" constructors
  // fromExtent
  QgsEllipse elp_alt = QgsEllipse( QgsPoint( 2.5, 5 ), 2.5, 5 );
  QVERIFY( QgsEllipse().fromExtent( QgsPoint( 0, 0 ), QgsPoint( 5, 10 ) ) == elp_alt );
  QVERIFY( QgsEllipse().fromExtent( QgsPoint( 5, 10 ), QgsPoint( 0, 0 ) ) == elp_alt );
  QVERIFY( QgsEllipse().fromExtent( QgsPoint( 5, 0 ), QgsPoint( 0, 10 ) ) == elp_alt );
  QVERIFY( QgsEllipse().fromExtent( QgsPoint( -5, 0 ), QgsPoint( 0, 10 ) ) != elp_alt );
  // fromCenterPoint
  QVERIFY( QgsEllipse().fromCenterPoint( QgsPoint( 2.5, 5 ), QgsPoint( 5, 10 ) ) == elp_alt );
  QVERIFY( QgsEllipse().fromCenterPoint( QgsPoint( 2.5, 5 ), QgsPoint( -0, 0 ) ) == elp_alt );
  QVERIFY( QgsEllipse().fromCenterPoint( QgsPoint( 2.5, 5 ), QgsPoint( 0, -10 ) ) != elp_alt );
  // fromCenter2Points
  QVERIFY( QgsEllipse().fromCenter2Points( QgsPoint( 2.5, 5 ), QgsPoint( 2.5, 0 ), QgsPoint( 7.5, 5 ) ) ==
           QgsEllipse( QgsPoint( 2.5, 5 ), 5, 5, 180 ) );
  QVERIFY( QgsEllipse().fromCenter2Points( QgsPoint( 2.5, 5 ), QgsPoint( 2.5, 7.5 ), QgsPoint( 7.5, 5 ) ) != elp_alt ); //same ellipse with different azimuth
  QVERIFY( QgsEllipse().fromCenter2Points( QgsPoint( 2.5, 5 ), QgsPoint( 2.5, 2.5 ), QgsPoint( 7.5, 5 ) ) != elp_alt ); //same ellipse with different azimuth
  QVERIFY( QgsEllipse().fromCenter2Points( QgsPoint( 2.5, 5 ), QgsPoint( 2.5, 0 ), QgsPoint( 5, 5 ) ) == elp_alt );
  QVERIFY( QgsEllipse().fromCenter2Points( QgsPoint( 5, 10 ), QgsPoint( 5, 10 ).project( 3, 45 ), QgsPoint( 5, 10 ).project( 2, 90 + 45 ) ) ==
           QgsEllipse( QgsPoint( 5, 10 ), 3, 2, 45 ) );

  // fromFoci
  // horizontal
  QgsEllipse elp_hor = QgsEllipse().fromFoci( QgsPoint( -4, 0 ), QgsPoint( 4, 0 ), QgsPoint( 0, 4 ) );
  QVERIFY( QgsEllipse( QgsPoint( 0, 0 ), sqrt( 32.0 ), sqrt( 16.0 ), 90.0 ) == elp_hor );
  QGSCOMPARENEARPOINT( QgsPoint( 4, 0 ), elp_hor.foci().at( 0 ), 1e-8 );
  QGSCOMPARENEARPOINT( QgsPoint( -4, 0 ), elp_hor.foci().at( 1 ), 1e-8 );
  elp_hor = QgsEllipse().fromFoci( QgsPoint( 4, 0 ), QgsPoint( -4, 0 ), QgsPoint( 0, 4 ) );
  QVERIFY( QgsEllipse( QgsPoint( 0, 0 ), sqrt( 32.0 ), sqrt( 16.0 ), 270.0 ) == elp_hor );
  QGSCOMPARENEARPOINT( QgsPoint( -4, 0 ), elp_hor.foci().at( 0 ), 1e-8 );
  QGSCOMPARENEARPOINT( QgsPoint( 4, 0 ), elp_hor.foci().at( 1 ), 1e-8 );

  // vertical
  QgsEllipse elp_ver = QgsEllipse().fromFoci( QgsPoint( 45, -15 ), QgsPoint( 45, 10 ), QgsPoint( 55, 0 ) );
  QVERIFY( QgsEllipse( QgsPoint( 45, -2.5 ), 16.084946, 10.123017725, 0.0 ) == elp_ver );
  elp_ver = QgsEllipse().fromFoci( QgsPoint( 45, 10 ), QgsPoint( 45, -15 ), QgsPoint( 55, 0 ) );
  QVERIFY( QgsEllipse( QgsPoint( 45, -2.5 ), 16.084946, 10.123017725, 180.0 ) == elp_ver );
  QGSCOMPARENEARPOINT( QgsPoint( 45, -15 ), elp_ver.foci().at( 0 ), 1e-8 );
  QGSCOMPARENEARPOINT( QgsPoint( 45, 10 ), elp_ver.foci().at( 1 ), 1e-8 );
  // oriented
  // first quadrant
  QgsEllipse elp_ori = QgsEllipse().fromFoci( QgsPoint( 10, 10 ), QgsPoint( 25, 20 ), QgsPoint( 15, 20 ) );
  QVERIFY( QgsEllipse( QgsPoint( 17.5, 15.0 ), 10.5901699437, 5.55892970251, 90.0 - 33.690067526 ) == elp_ori );
  QGSCOMPARENEARPOINT( QgsPoint( 25, 20 ), elp_ori.foci().at( 0 ), 1e-8 );
  QGSCOMPARENEARPOINT( QgsPoint( 10, 10 ), elp_ori.foci().at( 1 ), 1e-8 );
  // second quadrant
  elp_ori = QgsEllipse().fromFoci( QgsPoint( 10, 10 ), QgsPoint( 5, 20 ), QgsPoint( 15, 20 ) );
  QVERIFY( QgsEllipse( QgsPoint( 7.5, 15.0 ), 10.5901699437, 8.99453719974, 360 - 26.56505117 ) == elp_ori );
  QGSCOMPARENEARPOINT( QgsPoint( 5, 20 ), elp_ori.foci().at( 0 ), 1e-8 );
  QGSCOMPARENEARPOINT( QgsPoint( 10, 10 ), elp_ori.foci().at( 1 ), 1e-8 );
  // third quadrant
  elp_ori = QgsEllipse().fromFoci( QgsPoint( 10, 10 ), QgsPoint( 5, -5 ), QgsPoint( 15, 20 ) );
  QVERIFY( QgsEllipse( QgsPoint( 7.5, 2.5 ), 19.0530819616, 17.3355107289893, 198.434948822922 ) == elp_ori );
  QGSCOMPARENEARPOINT( QgsPoint( 10, 10 ), elp_ori.foci().at( 1 ), 1e-8 );
  QGSCOMPARENEARPOINT( QgsPoint( 5, -5 ), elp_ori.foci().at( 0 ), 1e-8 );
  // fourth quadrant
  elp_ori = QgsEllipse().fromFoci( QgsPoint( 10, 10 ), QgsPoint( 25, -5 ), QgsPoint( 15, 20 ) );
  QVERIFY( QgsEllipse( QgsPoint( 17.5, 2.5 ), 19.0530819616, 15.82782146, 135 ) == elp_ori );
  QGSCOMPARENEARPOINT( QgsPoint( 25, -5 ), elp_ori.foci().at( 0 ), 1e-8 );
  QGSCOMPARENEARPOINT( QgsPoint( 10, 10 ), elp_ori.foci().at( 1 ), 1e-8 );

  // test quadrant
  QgsEllipse elpq( QgsPoint( 5, 10 ), 3, 2, 45 );
  QVector<QgsPoint> q = elpq.quadrant();
  QGSCOMPARENEARPOINT( q.at( 0 ), QgsPoint( 7.1213, 12.1213 ), 0.001 );
  QGSCOMPARENEARPOINT( q.at( 3 ), QgsPoint( 3.5858, 11.4142 ), 0.001 );
  QGSCOMPARENEARPOINT( q.at( 2 ), QgsPoint( 2.8787, 7.8787 ), 0.001 );
  QGSCOMPARENEARPOINT( q.at( 1 ), QgsPoint( 6.4142, 8.5858 ), 0.001 );

  elpq = QgsEllipse( QgsPoint( 0, 0 ), 5, 2, 90 );
  q.clear();
  q = elpq.quadrant();
  QCOMPARE( q.at( 3 ), QgsPoint( 0, 2 ) );
  QCOMPARE( q.at( 0 ), QgsPoint( 5, 0 ) );
  QCOMPARE( q.at( 1 ), QgsPoint( 0, -2 ) );
  QCOMPARE( q.at( 2 ), QgsPoint( -5, 0 ) );

  elpq = QgsEllipse( QgsPoint( QgsWkbTypes::PointZM, 0, 0, 123, 321 ), 5, 2, 0 );
  q.clear();
  q = elpq.quadrant();
  QCOMPARE( q.at( 0 ), QgsPoint( QgsWkbTypes::PointZM, 0, 5, 123, 321 ) );
  QCOMPARE( q.at( 3 ), QgsPoint( QgsWkbTypes::PointZM, -2, 0, 123, 321 ) );
  QCOMPARE( q.at( 2 ), QgsPoint( QgsWkbTypes::PointZM, 0, -5, 123, 321 ) );
  QCOMPARE( q.at( 1 ), QgsPoint( QgsWkbTypes::PointZM, 2, 0, 123, 321 ) );

  elpq = QgsEllipse( QgsPoint( 0, 0 ), 2.5, 2, 315 );
  q.clear();
  q = elpq.quadrant();
  QGSCOMPARENEARPOINT( q.at( 1 ), QgsPoint( 1.4142, 1.4142 ), 0.001 );
  QGSCOMPARENEARPOINT( q.at( 2 ), QgsPoint( 1.7678, -1.7678 ), 0.001 );
  QGSCOMPARENEARPOINT( q.at( 3 ), QgsPoint( -1.4142, -1.4142 ), 0.001 );
  QGSCOMPARENEARPOINT( q.at( 0 ), QgsPoint( -1.7678, 1.7678 ), 0.001 );

  elpq = QgsEllipse( QgsPoint( 0, 0 ), 5, 2.5, 45 );
  q.clear();
  q = elpq.quadrant();
  QGSCOMPARENEARPOINT( q.at( 3 ), QgsPoint( -1.7678, 1.7678 ), 0.001 );
  QGSCOMPARENEARPOINT( q.at( 0 ), QgsPoint( 3.5355, 3.5355 ), 0.001 );
  QGSCOMPARENEARPOINT( q.at( 1 ), QgsPoint( 1.7678, -1.7678 ), 0.001 );
  QGSCOMPARENEARPOINT( q.at( 2 ), QgsPoint( -3.5355, -3.5355 ), 0.001 );

  //test conversion
  // points
  QgsPointSequence pts;
  pts = QgsEllipse( QgsPoint( 0, 0 ), 5, 2, 0 ).points( 4 );
  q = QgsEllipse( QgsPoint( 0, 0 ), 5, 2, 0 ).quadrant();
  QCOMPARE( pts.length(), 4 );
  QGSCOMPARENEARPOINT( q.at( 0 ), pts.at( 0 ), 2 );
  QGSCOMPARENEARPOINT( q.at( 1 ), pts.at( 1 ), 2 );
  QGSCOMPARENEARPOINT( q.at( 2 ), pts.at( 2 ), 2 );
  QGSCOMPARENEARPOINT( q.at( 3 ), pts.at( 3 ), 2 );
  // linestring
  QgsLineString *l = new QgsLineString();

  l = QgsEllipse( QgsPoint( 0, 0 ), 5, 2, 0 ).toLineString( 4 );
  QCOMPARE( l->numPoints(), 4 );
  QgsPointSequence pts_l;
  l->points( pts_l );
  QCOMPARE( pts, pts_l );

  // polygon
  QgsPolygonV2 *p1 = new QgsPolygonV2();

  p1 = QgsEllipse( QgsPoint( 0, 0 ), 5, 2, 0 ).toPolygon( 4 );
  q = QgsEllipse( QgsPoint( 0, 0 ), 5, 2, 0 ).quadrant();
  QCOMPARE( p1->vertexAt( QgsVertexId( 0, 0, 0 ) ), q.at( 0 ) );
  QCOMPARE( p1->vertexAt( QgsVertexId( 0, 0, 1 ) ), q.at( 1 ) );
  QCOMPARE( p1->vertexAt( QgsVertexId( 0, 0, 2 ) ), q.at( 2 ) );
  QCOMPARE( p1->vertexAt( QgsVertexId( 0, 0, 3 ) ), q.at( 3 ) );
  QCOMPARE( p1->vertexAt( QgsVertexId( 0, 0, 4 ) ), q.at( 0 ) );
  QCOMPARE( 0, p1->numInteriorRings() );
  QCOMPARE( 5, p1->exteriorRing()->numPoints() );

  p1 = QgsEllipse( QgsPoint( 0, 0 ), 5, 2, 90 ).toPolygon( 4 );
  q = QgsEllipse( QgsPoint( 0, 0 ), 5, 2, 90 ).quadrant();
  QCOMPARE( p1->vertexAt( QgsVertexId( 0, 0, 0 ) ), q.at( 0 ) );
  QCOMPARE( p1->vertexAt( QgsVertexId( 0, 0, 1 ) ), q.at( 1 ) );
  QCOMPARE( p1->vertexAt( QgsVertexId( 0, 0, 2 ) ), q.at( 2 ) );
  QCOMPARE( p1->vertexAt( QgsVertexId( 0, 0, 3 ) ), q.at( 3 ) );
  QCOMPARE( p1->vertexAt( QgsVertexId( 0, 0, 4 ) ), q.at( 0 ) );
  QCOMPARE( 0, p1->numInteriorRings() );
  QCOMPARE( 5, p1->exteriorRing()->numPoints() );

  p1 = elpq.toPolygon( 4 );
  q = elpq.quadrant();
  QCOMPARE( p1->vertexAt( QgsVertexId( 0, 0, 0 ) ), q.at( 0 ) );
  QCOMPARE( p1->vertexAt( QgsVertexId( 0, 0, 1 ) ), q.at( 1 ) );
  QCOMPARE( p1->vertexAt( QgsVertexId( 0, 0, 2 ) ), q.at( 2 ) );
  QCOMPARE( p1->vertexAt( QgsVertexId( 0, 0, 3 ) ), q.at( 3 ) );
  QCOMPARE( p1->vertexAt( QgsVertexId( 0, 0, 4 ) ), q.at( 0 ) );
  QCOMPARE( 0, p1->numInteriorRings() );
  QCOMPARE( 5, p1->exteriorRing()->numPoints() );

  // oriented bounding box
  QVERIFY( QgsEllipse().orientedBoundingBox()->isEmpty() );

  elpq = QgsEllipse( QgsPoint( 0, 0 ), 5, 2 );
  QgsPolygonV2 *ombb = new QgsPolygonV2();
  QgsLineString *ext = new QgsLineString();
  ext->setPoints( QgsPointSequence() << QgsPoint( 5, 2 ) << QgsPoint( 5, -2 ) << QgsPoint( -5, -2 ) << QgsPoint( -5, 2 ) );
  ombb->setExteriorRing( ext );
  QCOMPARE( ombb->asWkt( 2 ), elpq.orientedBoundingBox()->asWkt( 2 ) );

  elpq = QgsEllipse( QgsPoint( 0, 0 ), 5, 2.5, 45 );
  ombb = elpq.orientedBoundingBox();
  QGSCOMPARENEARPOINT( ombb->exteriorRing()->vertexAt( QgsVertexId( 0, 0, 0 ) ), QgsPoint( 1.7678, 5.3033 ), 0.0001 );
  QGSCOMPARENEARPOINT( ombb->exteriorRing()->vertexAt( QgsVertexId( 0, 0, 1 ) ), QgsPoint( 5.3033, 1.7678 ), 0.0001 );
  QGSCOMPARENEARPOINT( ombb->exteriorRing()->vertexAt( QgsVertexId( 0, 0, 2 ) ), QgsPoint( -1.7678, -5.3033 ), 0.0001 );
  QGSCOMPARENEARPOINT( ombb->exteriorRing()->vertexAt( QgsVertexId( 0, 0, 3 ) ), QgsPoint( -5.3033, -1.7678 ), 0.0001 );

  elpq = QgsEllipse( QgsPoint( 0, 0 ), 5, 2.5, 315 );
  ombb = elpq.orientedBoundingBox();
  QGSCOMPARENEARPOINT( ombb->exteriorRing()->vertexAt( QgsVertexId( 0, 0, 0 ) ), QgsPoint( -5.3033, 1.7678 ), 0.0001 );
  QGSCOMPARENEARPOINT( ombb->exteriorRing()->vertexAt( QgsVertexId( 0, 0, 1 ) ), QgsPoint( -1.7678, 5.3033 ), 0.0001 );
  QGSCOMPARENEARPOINT( ombb->exteriorRing()->vertexAt( QgsVertexId( 0, 0, 2 ) ), QgsPoint( 5.3033, -1.7678 ), 0.0001 );
  QGSCOMPARENEARPOINT( ombb->exteriorRing()->vertexAt( QgsVertexId( 0, 0, 3 ) ), QgsPoint( 1.7678, -5.3033 ), 0.0001 );

  // bounding box
  QCOMPARE( QgsEllipse().boundingBox(), QgsRectangle() );
  QCOMPARE( QgsEllipse( QgsPoint( 0, 0 ), 5, 2 ).boundingBox(), QgsEllipse( QgsPoint( 0, 0 ), 5, 2 ).orientedBoundingBox()->boundingBox() );
  QCOMPARE( QgsEllipse( QgsPoint( 0, 0 ), 5, 5 ).boundingBox(), QgsRectangle( QgsPointXY( -5, -5 ), QgsPointXY( 5, 5 ) ) );
  QCOMPARE( QgsEllipse( QgsPoint( 0, 0 ), 5, 5, 60 ).boundingBox(), QgsRectangle( QgsPointXY( -5, -5 ), QgsPointXY( 5, 5 ) ) );
  QCOMPARE( QgsEllipse( QgsPoint( 0, 0 ), 13, 9, 45 ).boundingBox().toString( 4 ).toStdString(), QgsRectangle( QgsPointXY( -11.1803, -11.1803 ), QgsPointXY( 11.1803, 11.1803 ) ).toString( 4 ).toStdString() );
  QCOMPARE( QgsEllipse( QgsPoint( 0, 0 ), 13, 9, 60 ).boundingBox().toString( 4 ).toStdString(), QgsRectangle( QgsPointXY( -12.12436, -10.14889 ), QgsPointXY( 12.12436, 10.14889 ) ).toString( 4 ).toStdString() );
  QCOMPARE( QgsEllipse( QgsPoint( 0, 0 ), 13, 9, 60 + 90 ).boundingBox().toString( 4 ).toStdString(), QgsRectangle( QgsPointXY( -10.14889, -12.12436 ), QgsPointXY( 10.14889, 12.12436 ) ).toString( 4 ).toStdString() );
  QCOMPARE( QgsEllipse( QgsPoint( 0, 0 ), 13, 9, 300 ).boundingBox().toString( 4 ).toStdString(), QgsRectangle( QgsPointXY( -12.12436, -10.14889 ), QgsPointXY( 12.12436, 10.14889 ) ).toString( 4 ).toStdString() );
  QCOMPARE( QgsEllipse( QgsPoint( 0, 0 ), 13, 9, 300 - 90 ).boundingBox().toString( 4 ).toStdString(), QgsRectangle( QgsPointXY( -10.14889, -12.12436 ), QgsPointXY( 10.14889, 12.12436 ) ).toString( 4 ).toStdString() );

  // focus
  QCOMPARE( QgsEllipse().fromFoci( QgsPoint( -4, 0 ), QgsPoint( 4, 0 ), QgsPoint( 0, 4 ) ).focusDistance(), 4.0 );
  QGSCOMPARENEAR( QgsEllipse().fromFoci( QgsPoint( 10, 10 ), QgsPoint( 25, 20 ), QgsPoint( 15, 20 ) ).focusDistance(), 9.01388, 0.0001 );

  // eccentricity
  QCOMPARE( QgsEllipse().fromFoci( QgsPoint( -4, 0 ), QgsPoint( 4, 0 ), QgsPoint( 0, 4 ) ).eccentricity(), 0.7071067811865475 );
  QCOMPARE( QgsEllipse( QgsPoint( 0, 0 ), 3, 3 ).eccentricity(), 0.0 );
  QVERIFY( std::isnan( QgsEllipse().eccentricity() ) );

  // area
  QGSCOMPARENEAR( 31.4159, QgsEllipse( QgsPoint( 0, 0 ), 5, 2, 0 ).area(), 0.0001 );
  // perimeter
  QGSCOMPARENEAR( QgsEllipse( QgsPoint( 0, 0 ), 5, 2, 45 ).perimeter(), QgsEllipse( QgsPoint( 0, 0 ), 5, 2, 45 ).toPolygon( 10000 )->perimeter(), 0.001 );

}

void TestQgsGeometry::circle()
{
  //test constructors
  QgsCircle circ1;
  QVERIFY( circ1.center() == QgsPoint() );
  QCOMPARE( circ1.radius(), 0.0 );
  QCOMPARE( circ1.azimuth(), 0.0 );
  QVERIFY( circ1.isEmpty() );

  QgsCircle circ2( QgsPoint( 5, 10 ), 3 );
  QVERIFY( circ2.center() == QgsPoint( 5, 10 ) );
  QCOMPARE( circ2.radius(), 3.0 );
  QCOMPARE( circ2.azimuth(), 0.0 );
  QVERIFY( !circ2.isEmpty() );

  QgsCircle circ3( QgsPoint( 5, 10 ), 3, 45 );
  QVERIFY( circ3.center() == QgsPoint( 5, 10 ) );
  QCOMPARE( circ3.radius(), 3.0 );
  QCOMPARE( circ3.azimuth(), 45.0 );
  QVERIFY( !circ3.isEmpty() );

  //test toString
  QCOMPARE( circ1.toString(), QString( "Empty" ) );
  QCOMPARE( circ2.toString(), QString( "Circle (Center: Point (5 10), Radius: 3, Azimuth: 0)" ) );
  QCOMPARE( circ3.toString(), QString( "Circle (Center: Point (5 10), Radius: 3, Azimuth: 45)" ) );

//test equality operator
  QVERIFY( QgsCircle() == QgsCircle( QgsPoint(), 0, 0 ) );
  QVERIFY( !( QgsCircle() == QgsCircle( QgsPoint(), 0, 0.0005 ) ) );
  QVERIFY( circ2 == QgsCircle( QgsPoint( 5, 10 ), 3, 0 ) );
  QVERIFY( circ2 != circ3 );

//test setter and getter
  circ1.setAzimuth( 45 );
  QCOMPARE( circ1.azimuth(), 45.0 );

  circ1.setRadius( 50 );
  QCOMPARE( circ1.radius(), 50.0 );
  QCOMPARE( circ1.semiMajorAxis(), 50.0 );
  QCOMPARE( circ1.semiMinorAxis(), 50.0 );

  circ1.setSemiMajorAxis( 250 );
  QCOMPARE( circ1.radius(), 250.0 );
  QCOMPARE( circ1.semiMajorAxis(), 250.0 );
  QCOMPARE( circ1.semiMinorAxis(), 250.0 );

  circ1.setSemiMinorAxis( 8250 );
  QCOMPARE( circ1.radius(), 8250.0 );
  QCOMPARE( circ1.semiMajorAxis(), 8250.0 );
  QCOMPARE( circ1.semiMinorAxis(), 8250.0 );

  circ1.setCenter( QgsPoint( 5, 10 ) );
  QVERIFY( circ1.center() == QgsPoint( 5, 10 ) );
  QVERIFY( circ1.rcenter() == QgsPoint( 5, 10 ) );
  circ1.rcenter() = QgsPoint( 25, 310 );
  QVERIFY( circ1.center() == QgsPoint( 25, 310 ) );

//test "alt" constructors
// by2Points
  QVERIFY( QgsCircle().from2Points( QgsPoint( -5, 0 ), QgsPoint( 5, 0 ) ) == QgsCircle( QgsPoint( 0, 0 ), 10, 90 ) );
  QVERIFY( QgsCircle().from2Points( QgsPoint( 0, -5 ), QgsPoint( 0, 5 ) ) == QgsCircle( QgsPoint( 0, 0 ), 10, 0 ) );
// byExtent
  QVERIFY( QgsCircle().fromExtent( QgsPoint( -5, -5 ), QgsPoint( 5, 5 ) ) == QgsCircle( QgsPoint( 0, 0 ), 5, 0 ) );
  QVERIFY( QgsCircle().fromExtent( QgsPoint( -7.5, -2.5 ), QgsPoint( 2.5, 200.5 ) ) == QgsCircle() );
// by3Points
  QVERIFY( QgsCircle().from3Points( QgsPoint( -5, 0 ), QgsPoint( 5, 0 ), QgsPoint( 0, 5 ) ) == QgsCircle( QgsPoint( 0, 0 ), 5 ) );
  QVERIFY( QgsCircle().from3Points( QgsPoint( 5, 0 ), QgsPoint( 6, 0 ), QgsPoint( 7, 0 ) ) == QgsCircle() );
// byCenterDiameter
  QVERIFY( QgsCircle().fromCenterDiameter( QgsPoint( 0, 0 ), 10 ) == QgsCircle( QgsPoint( 0, 0 ), 5, 0 ) );
  QVERIFY( QgsCircle().fromCenterDiameter( QgsPoint( 2, 100 ), -10 ) == QgsCircle( QgsPoint( 2, 100 ), 5, 0 ) );
  QVERIFY( QgsCircle().fromCenterDiameter( QgsPoint( 2, 100 ), -10, 45 ) == QgsCircle( QgsPoint( 2, 100 ), 5, 45 ) );
// byCenterPoint
  QVERIFY( QgsCircle().fromCenterPoint( QgsPoint( 0, 0 ), QgsPoint( 5, 0 ) ) == QgsCircle( QgsPoint( 0, 0 ), 5, 90 ) );
  QVERIFY( QgsCircle().fromCenterPoint( QgsPoint( 0, 0 ), QgsPoint( 0, 5 ) ) == QgsCircle( QgsPoint( 0, 0 ), 5, 0 ) );
  QVERIFY( QgsCircle().fromCenterPoint( QgsPoint( 0, 0 ), QgsPoint( 5 * cos( 45 * M_PI / 180.0 ), 5 * sin( 45 * M_PI / 180.0 ) ) ) == QgsCircle( QgsPoint( 0, 0 ), 5, 45 ) );
// by3Tangents
// Tangents from triangle tri1( 0,0 ; 0,5 ), tri2( 0,0 ; 5,0 ), tri3( 5,0 ; 0,5 )
  QgsCircle circ_tgt = QgsCircle().from3Tangents( QgsPoint( 0, 0 ), QgsPoint( 0, 1 ), QgsPoint( 2, 0 ), QgsPoint( 3, 0 ), QgsPoint( 5, 0 ), QgsPoint( 0, 5 ) );
  QGSCOMPARENEARPOINT( circ_tgt.center(), QgsPoint( 1.4645, 1.4645 ), 0.0001 );
  QGSCOMPARENEAR( circ_tgt.radius(), 1.4645, 0.0001 );

// test quadrant
  QVector<QgsPoint> quad = QgsCircle( QgsPoint( 0, 0 ), 5 ).northQuadrant();
  QVERIFY( quad.at( 0 ) == QgsPoint( 0, 5 ) );
  QVERIFY( quad.at( 1 ) == QgsPoint( 5, 0 ) );
  QVERIFY( quad.at( 2 ) == QgsPoint( 0, -5 ) );
  QVERIFY( quad.at( 3 ) == QgsPoint( -5, 0 ) );

  quad = QgsCircle( QgsPoint( 0, 0 ), 5, 123 ).northQuadrant();
  QVERIFY( quad.at( 0 ) == QgsPoint( 0, 5 ) );
  QVERIFY( quad.at( 1 ) == QgsPoint( 5, 0 ) );
  QVERIFY( quad.at( 2 ) == QgsPoint( 0, -5 ) );
  QVERIFY( quad.at( 3 ) == QgsPoint( -5, 0 ) );

  quad = QgsCircle( QgsPoint( 0, 0 ), 5, 456 ).northQuadrant();
  QVERIFY( quad.at( 0 ) == QgsPoint( 0, 5 ) );
  QVERIFY( quad.at( 1 ) == QgsPoint( 5, 0 ) );
  QVERIFY( quad.at( 2 ) == QgsPoint( 0, -5 ) );
  QVERIFY( quad.at( 3 ) == QgsPoint( -5, 0 ) );

  quad = QgsCircle( QgsPoint( 0, 0 ), 5, -789l ).northQuadrant();
  QVERIFY( quad.at( 0 ) == QgsPoint( 0, 5 ) );
  QVERIFY( quad.at( 1 ) == QgsPoint( 5, 0 ) );
  QVERIFY( quad.at( 2 ) == QgsPoint( 0, -5 ) );
  QVERIFY( quad.at( 3 ) == QgsPoint( -5, 0 ) );


//test conversion
  QgsPointSequence ptsPol;
  std::unique_ptr< QgsPolygonV2 > pol( new QgsPolygonV2() );
// polygon
  pol.reset( QgsCircle( QgsPoint( 0, 0 ), 5 ).toPolygon( 4 ) );
  QCOMPARE( pol->numInteriorRings(), 0 );
  QCOMPARE( pol->exteriorRing()->numPoints(), 5 );

  pol->exteriorRing()->points( ptsPol );
  QCOMPARE( ptsPol.length(), 5 );
  QVERIFY( ptsPol.at( 0 ) == QgsPoint( 0, 5 ) );
  QVERIFY( ptsPol.at( 1 ) == QgsPoint( 5, 0 ) );
  QVERIFY( ptsPol.at( 2 ) == QgsPoint( 0, -5 ) );
  QVERIFY( ptsPol.at( 3 ) == QgsPoint( -5, 0 ) );
  QVERIFY( ptsPol.at( 4 ) == QgsPoint( 0, 5 ) );

// oriented
//45
  double val = 5 * sin( M_PI / 4 );
  pol.reset( QgsCircle( QgsPoint( 0, 0 ), 5, 45 ).toPolygon( 4 ) );
  QCOMPARE( pol->numInteriorRings(), 0 );
  QCOMPARE( pol->exteriorRing()->numPoints(), 5 );
  pol->exteriorRing()->points( ptsPol );
  QCOMPARE( ptsPol.length(), 5 );
  QVERIFY( ptsPol.at( 0 ) == QgsPoint( val, val ) );
  QVERIFY( ptsPol.at( 1 ) == QgsPoint( val, -val ) );
  QVERIFY( ptsPol.at( 2 ) == QgsPoint( -val, -val ) );
  QVERIFY( ptsPol.at( 3 ) == QgsPoint( -val, val ) );
  QVERIFY( ptsPol.at( 4 ) == QgsPoint( val, val ) );
//135
  pol.reset( QgsCircle( QgsPoint( 0, 0 ), 5, 135 ).toPolygon( 4 ) );
  QCOMPARE( pol->numInteriorRings(), 0 );
  QCOMPARE( pol->exteriorRing()->numPoints(), 5 );
  pol->exteriorRing()->points( ptsPol );
  QCOMPARE( ptsPol.length(), 5 );
  QVERIFY( ptsPol.at( 0 ) == QgsPoint( val, -val ) );
  QVERIFY( ptsPol.at( 1 ) == QgsPoint( -val, -val ) );
  QVERIFY( ptsPol.at( 2 ) == QgsPoint( -val, val ) );
  QVERIFY( ptsPol.at( 3 ) == QgsPoint( val, val ) );
  QVERIFY( ptsPol.at( 4 ) == QgsPoint( val, -val ) );
//225
  pol.reset( QgsCircle( QgsPoint( 0, 0 ), 5, 225 ).toPolygon( 4 ) );
  QCOMPARE( pol->numInteriorRings(), 0 );
  QCOMPARE( pol->exteriorRing()->numPoints(), 5 );
  pol->exteriorRing()->points( ptsPol );
  QCOMPARE( ptsPol.length(), 5 );
  QVERIFY( ptsPol.at( 0 ) == QgsPoint( -val, -val ) );
  QVERIFY( ptsPol.at( 1 ) == QgsPoint( -val, val ) );
  QVERIFY( ptsPol.at( 2 ) == QgsPoint( val, val ) );
  QVERIFY( ptsPol.at( 3 ) == QgsPoint( val, -val ) );
  QVERIFY( ptsPol.at( 4 ) == QgsPoint( -val, -val ) );
//315
  pol.reset( QgsCircle( QgsPoint( 0, 0 ), 5, 315 ).toPolygon( 4 ) );
  QCOMPARE( pol->numInteriorRings(), 0 );
  QCOMPARE( pol->exteriorRing()->numPoints(), 5 );
  pol->exteriorRing()->points( ptsPol );
  QCOMPARE( ptsPol.length(), 5 );
  QVERIFY( ptsPol.at( 0 ) == QgsPoint( -val, val ) );
  QVERIFY( ptsPol.at( 1 ) == QgsPoint( val, val ) );
  QVERIFY( ptsPol.at( 2 ) == QgsPoint( val, -val ) );
  QVERIFY( ptsPol.at( 3 ) == QgsPoint( -val, -val ) );
  QVERIFY( ptsPol.at( 4 ) == QgsPoint( -val, val ) );

// circular arc
  QCOMPARE( QgsCircle( QgsPoint( 0, 0 ), 5 ).toCircularString()->asWkt( 2 ), QString( "CircularString (0 5, 5 0, 0 -5, -5 0, 0 5)" ) );
  QCOMPARE( QgsCircle( QgsPoint( 0, 0 ), 5 ).toCircularString( true )->asWkt( 2 ), QString( "CircularString (0 5, 5 0, 0 -5, -5 -0, 0 5)" ) );
  QCOMPARE( QgsCircle( QgsPoint( 0, 0 ), 5, 315 ).toCircularString()->asWkt( 2 ), QString( "CircularString (0 5, 5 0, 0 -5, -5 0, 0 5)" ) );
  QCOMPARE( QgsCircle( QgsPoint( 0, 0 ), 5, 315 ).toCircularString( true )->asWkt( 2 ), QString( "CircularString (-3.54 3.54, 3.54 3.54, 3.54 -3.54, -3.54 -3.54, -3.54 3.54)" ) );

// bounding box
  QVERIFY( QgsRectangle( QgsPointXY( -2.5, -2.5 ), QgsPointXY( 2.5, 2.5 ) ) == QgsCircle( QgsPoint( 0, 0 ), 2.5, 0 ).boundingBox() );
  QVERIFY( QgsRectangle( QgsPointXY( -2.5, -2.5 ), QgsPointXY( 2.5, 2.5 ) ) == QgsCircle( QgsPoint( 0, 0 ), 2.5, 45 ).boundingBox() );

  // area
  QGSCOMPARENEAR( 314.1593, QgsCircle( QgsPoint( 0, 0 ), 10 ).area(), 0.0001 );
  // perimeter
  QGSCOMPARENEAR( 31.4159, QgsCircle( QgsPoint( 0, 0 ), 5 ).perimeter(), 0.0001 );
}

void TestQgsGeometry::regularPolygon()
{
  // constructors
  QgsRegularPolygon rp1 = QgsRegularPolygon();
  QCOMPARE( rp1.center(), QgsPoint() );
  QCOMPARE( rp1.firstVertex(), QgsPoint() );
  QCOMPARE( rp1.numberSides(), 0 );
  QCOMPARE( rp1.radius(), 0.0 );
  QVERIFY( rp1.isEmpty() );

  QgsRegularPolygon rp2;
  QgsRegularPolygon( QgsPoint(), 5, 0, 2, QgsRegularPolygon::InscribedCircle );
  QVERIFY( rp2.isEmpty() );
  QgsRegularPolygon( QgsPoint(), 5, 0, 5, static_cast< QgsRegularPolygon::ConstructionOption >( 4 ) );
  QVERIFY( rp2.isEmpty() );

  rp2 = QgsRegularPolygon( QgsPoint(), 5, 0, 5, QgsRegularPolygon::InscribedCircle );
  QVERIFY( !rp2.isEmpty() );
  QCOMPARE( rp2.center(), QgsPoint() );
  QCOMPARE( rp2.firstVertex(), QgsPoint( 0, 5 ) );
  QCOMPARE( rp2.numberSides(), 5 );
  QCOMPARE( rp2.radius(), 5.0 );
  QGSCOMPARENEAR( rp2.apothem(), 4.0451, 10E-4 );
  QVERIFY( rp2 ==  QgsRegularPolygon( QgsPoint(), -5, 0, 5, QgsRegularPolygon::InscribedCircle ) );

  QgsRegularPolygon rp3 = QgsRegularPolygon( QgsPoint(), rp2.apothem(), 36.0, 5, QgsRegularPolygon::CircumscribedCircle );
  QVERIFY( rp2 == rp3 );
  QVERIFY( rp2 == QgsRegularPolygon( QgsPoint(), -rp2.apothem(), 36.0, 5, QgsRegularPolygon::CircumscribedCircle ) );
  QVERIFY( rp1 != rp3 );
  QVERIFY( rp1 != QgsRegularPolygon( QgsPoint( 5, 5 ), rp2.apothem(), 36.0, 5, QgsRegularPolygon::CircumscribedCircle ) );
  QVERIFY( rp1 != QgsRegularPolygon( QgsPoint( 0, 0 ), 5, 36.0, 5, QgsRegularPolygon::CircumscribedCircle ) );
  QVERIFY( rp1 != QgsRegularPolygon( QgsPoint( 0, 0 ), 5, 36.0, 5, QgsRegularPolygon::InscribedCircle ) );

  QgsRegularPolygon rp4 = QgsRegularPolygon( QgsPoint(), QgsPoint( 0, 5 ), 2, QgsRegularPolygon::InscribedCircle );
  QVERIFY( rp4.isEmpty() );
  rp4 = QgsRegularPolygon( QgsPoint(), QgsPoint( 0, 5 ), 5, static_cast< QgsRegularPolygon::ConstructionOption >( 4 ) );
  QVERIFY( rp4.isEmpty() );
  rp4 = QgsRegularPolygon( QgsPoint(), QgsPoint( 0, 5 ), 5, QgsRegularPolygon::InscribedCircle );
  QVERIFY( rp4 == rp2 );

  QgsRegularPolygon rp5 = QgsRegularPolygon( QgsPoint(), QgsPoint( 0, 0 ).project( rp2.apothem(), 36.0 ), 2, QgsRegularPolygon::CircumscribedCircle );
  QVERIFY( rp5.isEmpty() );
  rp5 = QgsRegularPolygon( QgsPoint(), QgsPoint( 0, 0 ).project( rp2.apothem(), 36.0 ), 5, static_cast< QgsRegularPolygon::ConstructionOption >( 4 ) );
  QVERIFY( rp5.isEmpty() );
  rp5 = QgsRegularPolygon( QgsPoint(), QgsPoint( 0, 0 ).project( rp2.apothem(), 36.0 ), 5, QgsRegularPolygon::CircumscribedCircle );
  QVERIFY( rp5 == rp2 );

  QgsRegularPolygon rp6 = QgsRegularPolygon( QgsPoint( 0, 5 ), QgsPoint( 0, 0 ).project( 5.0, 72 ), 5 );
  QVERIFY( rp6 == rp2 );


  // setters and getters
  QgsRegularPolygon rp7 = QgsRegularPolygon();

  rp7.setCenter( QgsPoint( 5, 5 ) );
  QVERIFY( rp7.isEmpty() );
  QCOMPARE( rp7.center(), QgsPoint( 5, 5 ) );

  rp7.setNumberSides( 2 );
  QVERIFY( rp7.isEmpty() );
  QCOMPARE( rp7.numberSides(), 0 );
  rp7.setNumberSides( 5 );
  QVERIFY( rp7.isEmpty() );
  QCOMPARE( rp7.numberSides(), 5 );
  rp7.setNumberSides( 2 );
  QVERIFY( rp7.isEmpty() );
  QCOMPARE( rp7.numberSides(), 5 );
  rp7.setNumberSides( 3 );
  QVERIFY( rp7.isEmpty() );
  QCOMPARE( rp7.numberSides(), 3 );

  rp7.setRadius( -6 );
  QVERIFY( !rp7.isEmpty() );
  QCOMPARE( rp7.radius(), 6.0 );
  QCOMPARE( rp7.firstVertex(), rp7.center().project( 6, 0 ) );

  rp7.setFirstVertex( QgsPoint( 4, 4 ) );
  QCOMPARE( rp7.firstVertex(), QgsPoint( 4, 4 ) );
  QCOMPARE( rp7.radius(), rp7.center().distance3D( QgsPoint( 4, 4 ) ) );

  rp7 = QgsRegularPolygon( QgsPoint(), QgsPoint( 0, 5 ), 5, QgsRegularPolygon::InscribedCircle );
  rp7.setCenter( QgsPoint( 5, 5 ) );
  QCOMPARE( rp7.radius(), 5.0 );
  QCOMPARE( rp7.firstVertex(), QgsPoint( 5, 10 ) );
  rp7.setNumberSides( 3 );
  QCOMPARE( rp7.radius(), 5.0 );
  QCOMPARE( rp7.firstVertex(), QgsPoint( 5, 10 ) );
  rp7.setNumberSides( 2 );
  QCOMPARE( rp7.radius(), 5.0 );
  QCOMPARE( rp7.firstVertex(), QgsPoint( 5, 10 ) );

  // measures
  QGSCOMPARENEAR( rp1.length(), 0.0, 10e-4 );
  QGSCOMPARENEAR( rp1.area(), 0.0, 10e-4 );
  QGSCOMPARENEAR( rp1.perimeter(), 0.0, 10e-4 );
  QGSCOMPARENEAR( rp2.length(), 5.8779, 10e-4 );
  QGSCOMPARENEAR( rp2.area(), 59.4410, 10e-4 );
  QGSCOMPARENEAR( rp2.perimeter(), 29.3893, 10e-4 );
  QCOMPARE( rp2.interiorAngle(), 108.0 );
  QCOMPARE( rp2.centralAngle(), 72.0 );
  QgsRegularPolygon rp8 = QgsRegularPolygon( QgsPoint( 0, 0 ), QgsPoint( 5, 0 ), 5 );
  QGSCOMPARENEAR( rp8.area(), 43.0119, 10e-4 );
  QCOMPARE( rp8.perimeter(), 25.0 );
  QCOMPARE( rp8.length(), 5.0 );
  QCOMPARE( rp8.interiorAngle(), 108.0 );
  QCOMPARE( rp8.centralAngle(), 72.0 );
  rp8.setNumberSides( 4 );
  QCOMPARE( rp8.interiorAngle(), 90.0 );
  QCOMPARE( rp8.centralAngle(), 90.0 );
  rp8.setNumberSides( 3 );
  QCOMPARE( rp8.interiorAngle(), 60.0 );
  QCOMPARE( rp8.centralAngle(), 120.0 );


  //test conversions
  // circle
  QVERIFY( QgsCircle( QgsPoint( 0, 0 ), 5 ) == rp2.circumscribedCircle() );
  QVERIFY( rp2.inscribedCircle() == QgsRegularPolygon( QgsPoint( 0, 0 ), rp2.apothem(), 36.0, 5, QgsRegularPolygon::InscribedCircle ).circumscribedCircle() );

  // triangle
  QCOMPARE( QgsTriangle(), rp2.toTriangle() );
  QCOMPARE( QgsTriangle(), QgsRegularPolygon().toTriangle() );
  QgsRegularPolygon rp9 = QgsRegularPolygon( QgsPoint( 0, 0 ), 5, 0, 3, QgsRegularPolygon::InscribedCircle );

  QVERIFY( QgsCircle( QgsPoint( 0, 0 ), 5 ) == rp9.toTriangle().circumscribedCircle() );

  QgsRegularPolygon rp10 = QgsRegularPolygon( QgsPoint( 0, 0 ), QgsPoint( 0, 4 ), 4 );
  QList<QgsTriangle> rp10_tri = rp10.triangulate();
  QCOMPARE( rp10_tri.length(), ( int )rp10.numberSides() );
  QVERIFY( rp10_tri.at( 0 ) == QgsTriangle( QgsPoint( 0, 0 ), QgsPoint( 0, 4 ), rp10.center() ) );
  QVERIFY( rp10_tri.at( 1 ) == QgsTriangle( QgsPoint( 0, 4 ), QgsPoint( 4, 4 ), rp10.center() ) );
  QVERIFY( rp10_tri.at( 2 ) == QgsTriangle( QgsPoint( 4, 4 ), QgsPoint( 4, 0 ), rp10.center() ) );
  QVERIFY( rp10_tri.at( 3 ) == QgsTriangle( QgsPoint( 4, 0 ), QgsPoint( 0, 0 ), rp10.center() ) );

  // polygon
  QgsPointSequence ptsPol;
  std::unique_ptr< QgsPolygonV2 > pol( new QgsPolygonV2() );
  pol.reset( rp10.toPolygon() );
  QCOMPARE( pol->numInteriorRings(), 0 );
  QCOMPARE( pol->exteriorRing()->numPoints(), 5 );

  pol->exteriorRing()->points( ptsPol );
  QCOMPARE( ptsPol.length(), 5 );
  QVERIFY( ptsPol.at( 0 ) == QgsPoint( 0, 0 ) );
  QVERIFY( ptsPol.at( 1 ) == QgsPoint( 0, 4 ) );
  QVERIFY( ptsPol.at( 2 ) == QgsPoint( 4, 4 ) );
  QVERIFY( ptsPol.at( 3 ) == QgsPoint( 4, 0 ) );
  QVERIFY( ptsPol.at( 4 ) == QgsPoint( 0, 0 ) );
  ptsPol.pop_back();

  std::unique_ptr< QgsLineString > l( new QgsLineString() );
  l.reset( rp10.toLineString() );
  QCOMPARE( l->numPoints(), 4 );
  QgsPointSequence pts_l;
  l->points( pts_l );
  QCOMPARE( ptsPol, pts_l );

  //test toString
  QCOMPARE( rp1.toString(), QString( "Empty" ) );
  QCOMPARE( rp2.toString(), QString( "RegularPolygon (Center: Point (0 0), First Vertex: Point (0 5), Radius: 5, Azimuth: 0)" ) );

}

void TestQgsGeometry::compoundCurve()
{
  //test that area of a compound curve ring is equal to a closed linestring with the same vertices
  QgsCompoundCurve cc;
  QgsLineString *l1 = new QgsLineString();
  l1->setPoints( QgsPointSequence() << QgsPoint( 1, 1 ) << QgsPoint( 0, 2 ) );
  cc.addCurve( l1 );
  QgsLineString *l2 = new QgsLineString();
  l2->setPoints( QgsPointSequence() << QgsPoint( 0, 2 ) << QgsPoint( -1, 0 ) << QgsPoint( 0, -1 ) );
  cc.addCurve( l2 );
  QgsLineString *l3 = new QgsLineString();
  l3->setPoints( QgsPointSequence() << QgsPoint( 0, -1 ) << QgsPoint( 1, 1 ) );
  cc.addCurve( l3 );

  double ccArea = 0.0;
  cc.sumUpArea( ccArea );

  QgsLineString ls;
  ls.setPoints( QgsPointSequence() << QgsPoint( 1, 1 ) << QgsPoint( 0, 2 ) <<  QgsPoint( -1, 0 ) << QgsPoint( 0, -1 )
                << QgsPoint( 1, 1 ) );
  double lsArea = 0.0;
  ls.sumUpArea( lsArea );
  QVERIFY( qgsDoubleNear( ccArea, lsArea ) );
}

void TestQgsGeometry::multiPoint()
{
  //boundary

  //multipoints have no boundary defined
  QgsMultiPointV2 boundaryMP;
  QVERIFY( !boundaryMP.boundary() );
  // add some points and retest, should still be undefined
  boundaryMP.addGeometry( new QgsPoint( 0, 0 ) );
  boundaryMP.addGeometry( new QgsPoint( 1, 1 ) );
  QVERIFY( !boundaryMP.boundary() );

  // closestSegment
  QgsPoint closest;
  QgsVertexId after;
  // return error - points have no segments
  QVERIFY( boundaryMP.closestSegment( QgsPoint( 0.5, 0.5 ), closest, after, 0, 0 ) < 0 );
}

void TestQgsGeometry::multiLineString()
{
  //boundary
  QgsMultiLineString multiLine1;
  QVERIFY( !multiLine1.boundary() );
  QgsLineString boundaryLine1;
  boundaryLine1.setPoints( QList<QgsPoint>() << QgsPoint( 0, 0 ) << QgsPoint( 1, 0 ) << QgsPoint( 1, 1 ) );
  multiLine1.addGeometry( boundaryLine1.clone() );
  QgsAbstractGeometry *boundary = multiLine1.boundary();
  QgsMultiPointV2 *mpBoundary = dynamic_cast< QgsMultiPointV2 * >( boundary );
  QVERIFY( mpBoundary );
  QCOMPARE( mpBoundary->numGeometries(), 2 );
  QCOMPARE( static_cast< QgsPoint *>( mpBoundary->geometryN( 0 ) )->x(), 0.0 );
  QCOMPARE( static_cast< QgsPoint *>( mpBoundary->geometryN( 0 ) )->y(), 0.0 );
  QCOMPARE( static_cast< QgsPoint *>( mpBoundary->geometryN( 1 ) )->x(), 1.0 );
  QCOMPARE( static_cast< QgsPoint *>( mpBoundary->geometryN( 1 ) )->y(), 1.0 );
  delete boundary;
  // add another linestring
  QgsLineString boundaryLine2;
  boundaryLine2.setPoints( QList<QgsPoint>() << QgsPoint( 10, 10 ) << QgsPoint( 11, 10 ) << QgsPoint( 11, 11 ) );
  multiLine1.addGeometry( boundaryLine2.clone() );
  boundary = multiLine1.boundary();
  mpBoundary = dynamic_cast< QgsMultiPointV2 * >( boundary );
  QVERIFY( mpBoundary );
  QCOMPARE( mpBoundary->numGeometries(), 4 );
  QCOMPARE( static_cast< QgsPoint *>( mpBoundary->geometryN( 0 ) )->x(), 0.0 );
  QCOMPARE( static_cast< QgsPoint *>( mpBoundary->geometryN( 0 ) )->y(), 0.0 );
  QCOMPARE( static_cast< QgsPoint *>( mpBoundary->geometryN( 1 ) )->x(), 1.0 );
  QCOMPARE( static_cast< QgsPoint *>( mpBoundary->geometryN( 1 ) )->y(), 1.0 );
  QCOMPARE( static_cast< QgsPoint *>( mpBoundary->geometryN( 2 ) )->x(), 10.0 );
  QCOMPARE( static_cast< QgsPoint *>( mpBoundary->geometryN( 2 ) )->y(), 10.0 );
  QCOMPARE( static_cast< QgsPoint *>( mpBoundary->geometryN( 3 ) )->x(), 11.0 );
  QCOMPARE( static_cast< QgsPoint *>( mpBoundary->geometryN( 3 ) )->y(), 11.0 );
  delete boundary;

  // add a closed string = no boundary
  QgsLineString boundaryLine3;
  boundaryLine3.setPoints( QList<QgsPoint>() << QgsPoint( 20, 20 ) << QgsPoint( 21, 20 ) << QgsPoint( 21, 21 ) << QgsPoint( 20, 20 ) );
  multiLine1.addGeometry( boundaryLine3.clone() );
  boundary = multiLine1.boundary();
  mpBoundary = dynamic_cast< QgsMultiPointV2 * >( boundary );
  QVERIFY( mpBoundary );
  QCOMPARE( mpBoundary->numGeometries(), 4 );
  QCOMPARE( static_cast< QgsPoint *>( mpBoundary->geometryN( 0 ) )->x(), 0.0 );
  QCOMPARE( static_cast< QgsPoint *>( mpBoundary->geometryN( 0 ) )->y(), 0.0 );
  QCOMPARE( static_cast< QgsPoint *>( mpBoundary->geometryN( 1 ) )->x(), 1.0 );
  QCOMPARE( static_cast< QgsPoint *>( mpBoundary->geometryN( 1 ) )->y(), 1.0 );
  QCOMPARE( static_cast< QgsPoint *>( mpBoundary->geometryN( 2 ) )->x(), 10.0 );
  QCOMPARE( static_cast< QgsPoint *>( mpBoundary->geometryN( 2 ) )->y(), 10.0 );
  QCOMPARE( static_cast< QgsPoint *>( mpBoundary->geometryN( 3 ) )->x(), 11.0 );
  QCOMPARE( static_cast< QgsPoint *>( mpBoundary->geometryN( 3 ) )->y(), 11.0 );
  delete boundary;

  //boundary with z
  QgsLineString boundaryLine4;
  boundaryLine4.setPoints( QList<QgsPoint>() << QgsPoint( QgsWkbTypes::PointZ, 0, 0, 10 ) << QgsPoint( QgsWkbTypes::PointZ, 1, 0, 15 ) << QgsPoint( QgsWkbTypes::PointZ, 1, 1, 20 ) );
  QgsLineString boundaryLine5;
  boundaryLine5.setPoints( QList<QgsPoint>() << QgsPoint( QgsWkbTypes::PointZ, 10, 10, 100 ) << QgsPoint( QgsWkbTypes::PointZ, 10, 20, 150 ) << QgsPoint( QgsWkbTypes::PointZ, 20, 20, 200 ) );
  QgsMultiLineString multiLine2;
  multiLine2.addGeometry( boundaryLine4.clone() );
  multiLine2.addGeometry( boundaryLine5.clone() );

  boundary = multiLine2.boundary();
  mpBoundary = dynamic_cast< QgsMultiPointV2 * >( boundary );
  QVERIFY( mpBoundary );
  QCOMPARE( mpBoundary->numGeometries(), 4 );
  QCOMPARE( mpBoundary->geometryN( 0 )->wkbType(), QgsWkbTypes::PointZ );
  QCOMPARE( static_cast< QgsPoint *>( mpBoundary->geometryN( 0 ) )->x(), 0.0 );
  QCOMPARE( static_cast< QgsPoint *>( mpBoundary->geometryN( 0 ) )->y(), 0.0 );
  QCOMPARE( static_cast< QgsPoint *>( mpBoundary->geometryN( 0 ) )->z(), 10.0 );
  QCOMPARE( mpBoundary->geometryN( 1 )->wkbType(), QgsWkbTypes::PointZ );
  QCOMPARE( static_cast< QgsPoint *>( mpBoundary->geometryN( 1 ) )->x(), 1.0 );
  QCOMPARE( static_cast< QgsPoint *>( mpBoundary->geometryN( 1 ) )->y(), 1.0 );
  QCOMPARE( static_cast< QgsPoint *>( mpBoundary->geometryN( 1 ) )->z(), 20.0 );
  QCOMPARE( mpBoundary->geometryN( 2 )->wkbType(), QgsWkbTypes::PointZ );
  QCOMPARE( static_cast< QgsPoint *>( mpBoundary->geometryN( 2 ) )->x(), 10.0 );
  QCOMPARE( static_cast< QgsPoint *>( mpBoundary->geometryN( 2 ) )->y(), 10.0 );
  QCOMPARE( static_cast< QgsPoint *>( mpBoundary->geometryN( 2 ) )->z(), 100.0 );
  QCOMPARE( mpBoundary->geometryN( 3 )->wkbType(), QgsWkbTypes::PointZ );
  QCOMPARE( static_cast< QgsPoint *>( mpBoundary->geometryN( 3 ) )->x(), 20.0 );
  QCOMPARE( static_cast< QgsPoint *>( mpBoundary->geometryN( 3 ) )->y(), 20.0 );
  QCOMPARE( static_cast< QgsPoint *>( mpBoundary->geometryN( 3 ) )->z(), 200.0 );
  delete boundary;
}

void TestQgsGeometry::multiPolygon()
{
  //boundary
  QgsMultiPolygonV2 multiPolygon1;
  QVERIFY( !multiPolygon1.boundary() );

  QgsLineString ring1;
  ring1.setPoints( QList<QgsPoint>() << QgsPoint( 0, 0 ) << QgsPoint( 1, 0 ) << QgsPoint( 1, 1 )  << QgsPoint( 0, 0 ) );
  QgsPolygonV2 polygon1;
  polygon1.setExteriorRing( ring1.clone() );
  multiPolygon1.addGeometry( polygon1.clone() );

  QgsAbstractGeometry *boundary = multiPolygon1.boundary();
  QgsMultiLineString *lineBoundary = dynamic_cast< QgsMultiLineString * >( boundary );
  QVERIFY( lineBoundary );
  QCOMPARE( lineBoundary->numGeometries(), 1 );
  QCOMPARE( dynamic_cast< QgsLineString * >( lineBoundary->geometryN( 0 ) )->numPoints(), 4 );
  QCOMPARE( dynamic_cast< QgsLineString * >( lineBoundary->geometryN( 0 ) )->xAt( 0 ), 0.0 );
  QCOMPARE( dynamic_cast< QgsLineString * >( lineBoundary->geometryN( 0 ) )->xAt( 1 ), 1.0 );
  QCOMPARE( dynamic_cast< QgsLineString * >( lineBoundary->geometryN( 0 ) )->xAt( 2 ), 1.0 );
  QCOMPARE( dynamic_cast< QgsLineString * >( lineBoundary->geometryN( 0 ) )->xAt( 3 ), 0.0 );
  QCOMPARE( dynamic_cast< QgsLineString * >( lineBoundary->geometryN( 0 ) )->yAt( 0 ), 0.0 );
  QCOMPARE( dynamic_cast< QgsLineString * >( lineBoundary->geometryN( 0 ) )->yAt( 1 ), 0.0 );
  QCOMPARE( dynamic_cast< QgsLineString * >( lineBoundary->geometryN( 0 ) )->yAt( 2 ), 1.0 );
  QCOMPARE( dynamic_cast< QgsLineString * >( lineBoundary->geometryN( 0 ) )->yAt( 3 ), 0.0 );
  delete boundary;

  // add polygon with interior rings
  QgsLineString ring2;
  ring2.setPoints( QList<QgsPoint>() << QgsPoint( 10, 10 ) << QgsPoint( 11, 10 ) << QgsPoint( 11, 11 )  << QgsPoint( 10, 10 ) );
  QgsPolygonV2 polygon2;
  polygon2.setExteriorRing( ring2.clone() );
  QgsLineString boundaryRing1;
  boundaryRing1.setPoints( QList<QgsPoint>() << QgsPoint( 10.1, 10.1 ) << QgsPoint( 10.2, 10.1 ) << QgsPoint( 10.2, 10.2 )  << QgsPoint( 10.1, 10.1 ) );
  QgsLineString boundaryRing2;
  boundaryRing2.setPoints( QList<QgsPoint>() << QgsPoint( 10.8, 10.8 ) << QgsPoint( 10.9, 10.8 ) << QgsPoint( 10.9, 10.9 )  << QgsPoint( 10.8, 10.8 ) );
  polygon2.setInteriorRings( QList< QgsCurve * >() << boundaryRing1.clone() << boundaryRing2.clone() );
  multiPolygon1.addGeometry( polygon2.clone() );

  boundary = multiPolygon1.boundary();
  QgsMultiLineString *multiLineBoundary = dynamic_cast< QgsMultiLineString * >( boundary );
  QVERIFY( multiLineBoundary );
  QCOMPARE( multiLineBoundary->numGeometries(), 4 );
  QCOMPARE( dynamic_cast< QgsLineString * >( multiLineBoundary->geometryN( 0 ) )->numPoints(), 4 );
  QCOMPARE( dynamic_cast< QgsLineString * >( multiLineBoundary->geometryN( 0 ) )->xAt( 0 ), 0.0 );
  QCOMPARE( dynamic_cast< QgsLineString * >( multiLineBoundary->geometryN( 0 ) )->xAt( 1 ), 1.0 );
  QCOMPARE( dynamic_cast< QgsLineString * >( multiLineBoundary->geometryN( 0 ) )->xAt( 2 ), 1.0 );
  QCOMPARE( dynamic_cast< QgsLineString * >( multiLineBoundary->geometryN( 0 ) )->xAt( 3 ), 0.0 );
  QCOMPARE( dynamic_cast< QgsLineString * >( multiLineBoundary->geometryN( 0 ) )->yAt( 0 ), 0.0 );
  QCOMPARE( dynamic_cast< QgsLineString * >( multiLineBoundary->geometryN( 0 ) )->yAt( 1 ), 0.0 );
  QCOMPARE( dynamic_cast< QgsLineString * >( multiLineBoundary->geometryN( 0 ) )->yAt( 2 ), 1.0 );
  QCOMPARE( dynamic_cast< QgsLineString * >( multiLineBoundary->geometryN( 0 ) )->yAt( 3 ), 0.0 );
  QCOMPARE( dynamic_cast< QgsLineString * >( multiLineBoundary->geometryN( 1 ) )->numPoints(), 4 );
  QCOMPARE( dynamic_cast< QgsLineString * >( multiLineBoundary->geometryN( 1 ) )->xAt( 0 ), 10.0 );
  QCOMPARE( dynamic_cast< QgsLineString * >( multiLineBoundary->geometryN( 1 ) )->xAt( 1 ), 11.0 );
  QCOMPARE( dynamic_cast< QgsLineString * >( multiLineBoundary->geometryN( 1 ) )->xAt( 2 ), 11.0 );
  QCOMPARE( dynamic_cast< QgsLineString * >( multiLineBoundary->geometryN( 1 ) )->xAt( 3 ), 10.0 );
  QCOMPARE( dynamic_cast< QgsLineString * >( multiLineBoundary->geometryN( 1 ) )->yAt( 0 ), 10.0 );
  QCOMPARE( dynamic_cast< QgsLineString * >( multiLineBoundary->geometryN( 1 ) )->yAt( 1 ), 10.0 );
  QCOMPARE( dynamic_cast< QgsLineString * >( multiLineBoundary->geometryN( 1 ) )->yAt( 2 ), 11.0 );
  QCOMPARE( dynamic_cast< QgsLineString * >( multiLineBoundary->geometryN( 1 ) )->yAt( 3 ), 10.0 );
  QCOMPARE( dynamic_cast< QgsLineString * >( multiLineBoundary->geometryN( 2 ) )->numPoints(), 4 );
  QCOMPARE( dynamic_cast< QgsLineString * >( multiLineBoundary->geometryN( 2 ) )->xAt( 0 ), 10.1 );
  QCOMPARE( dynamic_cast< QgsLineString * >( multiLineBoundary->geometryN( 2 ) )->xAt( 1 ), 10.2 );
  QCOMPARE( dynamic_cast< QgsLineString * >( multiLineBoundary->geometryN( 2 ) )->xAt( 2 ), 10.2 );
  QCOMPARE( dynamic_cast< QgsLineString * >( multiLineBoundary->geometryN( 2 ) )->xAt( 3 ), 10.1 );
  QCOMPARE( dynamic_cast< QgsLineString * >( multiLineBoundary->geometryN( 2 ) )->yAt( 0 ), 10.1 );
  QCOMPARE( dynamic_cast< QgsLineString * >( multiLineBoundary->geometryN( 2 ) )->yAt( 1 ), 10.1 );
  QCOMPARE( dynamic_cast< QgsLineString * >( multiLineBoundary->geometryN( 2 ) )->yAt( 2 ), 10.2 );
  QCOMPARE( dynamic_cast< QgsLineString * >( multiLineBoundary->geometryN( 2 ) )->yAt( 3 ), 10.1 );
  QCOMPARE( dynamic_cast< QgsLineString * >( multiLineBoundary->geometryN( 3 ) )->numPoints(), 4 );
  QCOMPARE( dynamic_cast< QgsLineString * >( multiLineBoundary->geometryN( 3 ) )->xAt( 0 ), 10.8 );
  QCOMPARE( dynamic_cast< QgsLineString * >( multiLineBoundary->geometryN( 3 ) )->xAt( 1 ), 10.9 );
  QCOMPARE( dynamic_cast< QgsLineString * >( multiLineBoundary->geometryN( 3 ) )->xAt( 2 ), 10.9 );
  QCOMPARE( dynamic_cast< QgsLineString * >( multiLineBoundary->geometryN( 3 ) )->xAt( 3 ), 10.8 );
  QCOMPARE( dynamic_cast< QgsLineString * >( multiLineBoundary->geometryN( 3 ) )->yAt( 0 ), 10.8 );
  QCOMPARE( dynamic_cast< QgsLineString * >( multiLineBoundary->geometryN( 3 ) )->yAt( 1 ), 10.8 );
  QCOMPARE( dynamic_cast< QgsLineString * >( multiLineBoundary->geometryN( 3 ) )->yAt( 2 ), 10.9 );
  QCOMPARE( dynamic_cast< QgsLineString * >( multiLineBoundary->geometryN( 3 ) )->yAt( 3 ), 10.8 );
  delete boundary;
}

void TestQgsGeometry::geometryCollection()
{

  //boundary

  // collections have no boundary defined
  QgsGeometryCollection boundaryCollection;
  QVERIFY( !boundaryCollection.boundary() );
  // add a geometry and retest, should still be undefined
  QgsLineString *lineBoundary = new QgsLineString();
  lineBoundary->setPoints( QList<QgsPoint>() << QgsPoint( 0, 0 ) << QgsPoint( 1, 0 ) );
  boundaryCollection.addGeometry( lineBoundary );
  QVERIFY( !boundaryCollection.boundary() );
}

void TestQgsGeometry::fromQgsPointXY()
{
  QgsPointXY point( 1.0, 2.0 );
  QgsGeometry result( QgsGeometry::fromPoint( point ) );
  QCOMPARE( result.wkbType(), QgsWkbTypes::Point );
  QgsPointXY resultPoint = result.asPoint();
  QCOMPARE( resultPoint, point );
}

void TestQgsGeometry::fromQPoint()
{
  QPointF point( 1.0, 2.0 );
  QgsGeometry result( QgsGeometry::fromQPointF( point ) );
  QCOMPARE( result.wkbType(), QgsWkbTypes::Point );
  QgsPointXY resultPoint = result.asPoint();
  QCOMPARE( resultPoint.x(), 1.0 );
  QCOMPARE( resultPoint.y(), 2.0 );
}

void TestQgsGeometry::fromQPolygonF()
{
  //test with a polyline
  QPolygonF polyline;
  polyline << QPointF( 1.0, 2.0 ) << QPointF( 4.0, 6.0 ) << QPointF( 4.0, 3.0 ) << QPointF( 2.0, 2.0 );
  QgsGeometry result( QgsGeometry::fromQPolygonF( polyline ) );
  QCOMPARE( result.wkbType(), QgsWkbTypes::LineString );
  QgsPolyline resultLine = result.asPolyline();
  QCOMPARE( resultLine.size(), 4 );
  QCOMPARE( resultLine.at( 0 ), QgsPointXY( 1.0, 2.0 ) );
  QCOMPARE( resultLine.at( 1 ), QgsPointXY( 4.0, 6.0 ) );
  QCOMPARE( resultLine.at( 2 ), QgsPointXY( 4.0, 3.0 ) );
  QCOMPARE( resultLine.at( 3 ), QgsPointXY( 2.0, 2.0 ) );

  //test with a closed polygon
  QPolygonF polygon;
  polygon << QPointF( 1.0, 2.0 ) << QPointF( 4.0, 6.0 ) << QPointF( 4.0, 3.0 ) << QPointF( 2.0, 2.0 ) << QPointF( 1.0, 2.0 );
  QgsGeometry result2( QgsGeometry::fromQPolygonF( polygon ) );
  QCOMPARE( result2.wkbType(), QgsWkbTypes::Polygon );
  QgsPolygon resultPolygon = result2.asPolygon();
  QCOMPARE( resultPolygon.size(), 1 );
  QCOMPARE( resultPolygon.at( 0 ).at( 0 ), QgsPointXY( 1.0, 2.0 ) );
  QCOMPARE( resultPolygon.at( 0 ).at( 1 ), QgsPointXY( 4.0, 6.0 ) );
  QCOMPARE( resultPolygon.at( 0 ).at( 2 ), QgsPointXY( 4.0, 3.0 ) );
  QCOMPARE( resultPolygon.at( 0 ).at( 3 ), QgsPointXY( 2.0, 2.0 ) );
  QCOMPARE( resultPolygon.at( 0 ).at( 4 ), QgsPointXY( 1.0, 2.0 ) );
}

void TestQgsGeometry::asQPointF()
{
  QPointF point( 1.0, 2.0 );
  QgsGeometry geom( QgsGeometry::fromQPointF( point ) );
  QPointF resultPoint = geom.asQPointF();
  QCOMPARE( resultPoint, point );

  //non point geom
  QPointF badPoint = mpPolygonGeometryA.asQPointF();
  QVERIFY( badPoint.isNull() );
}

void TestQgsGeometry::asQPolygonF()
{
  //test polygon
  QPolygonF fromPoly = mpPolygonGeometryA.asQPolygonF();
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
  QgsGeometry lineGeom( QgsGeometry::fromPolyline( testline ) );
  QPolygonF fromLine = lineGeom.asQPolygonF();
  QVERIFY( !fromLine.isClosed() );
  QCOMPARE( fromLine.size(), 3 );
  QCOMPARE( fromLine.at( 0 ).x(), mPoint1.x() );
  QCOMPARE( fromLine.at( 0 ).y(), mPoint1.y() );
  QCOMPARE( fromLine.at( 1 ).x(), mPoint2.x() );
  QCOMPARE( fromLine.at( 1 ).y(), mPoint2.y() );
  QCOMPARE( fromLine.at( 2 ).x(), mPoint3.x() );
  QCOMPARE( fromLine.at( 2 ).y(), mPoint3.y() );

  //test a bad geometry
  QgsGeometry badGeom( QgsGeometry::fromPoint( mPoint1 ) );
  QPolygonF fromBad = badGeom.asQPolygonF();
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
  QgsGeometry *mypSimplifyGeometry  =  mpPolylineGeometryD->simplify( 0.5 );
  qDebug( "Geometry Type: %s", QgsWkbTypes::displayString( mypSimplifyGeometry->wkbType() ) );
  QVERIFY( mypSimplifyGeometry->wkbType() == QgsWkbTypes::LineString );
  QgsPolyline myLine = mypSimplifyGeometry->asPolyline();
  QVERIFY( myLine.size() > 0 ); //check that the union created a feature
  dumpPolyline( myLine );
  delete mypSimplifyGeometry;
  QVERIFY( renderCheck( "geometry_simplifyCheck1", "Checking simplify of line" ) );
}
#endif

void TestQgsGeometry::intersectionCheck1()
{
  QVERIFY( mpPolygonGeometryA.intersects( mpPolygonGeometryB ) );
  // should be a single polygon as A intersect B
  QgsGeometry mypIntersectionGeometry  =  mpPolygonGeometryA.intersection( mpPolygonGeometryB );
  qDebug() << "Geometry Type: " << QgsWkbTypes::displayString( mypIntersectionGeometry.wkbType() );
  QVERIFY( mypIntersectionGeometry.wkbType() == QgsWkbTypes::Polygon );
  QgsPolygon myPolygon = mypIntersectionGeometry.asPolygon();
  QVERIFY( myPolygon.size() > 0 ); //check that the union created a feature
  dumpPolygon( myPolygon );
  QVERIFY( renderCheck( "geometry_intersectionCheck1", "Checking if A intersects B" ) );
}
void TestQgsGeometry::intersectionCheck2()
{
  QVERIFY( !mpPolygonGeometryA.intersects( mpPolygonGeometryC ) );
}

void TestQgsGeometry::translateCheck1()
{
  QString wkt = QStringLiteral( "LineString (0 0, 10 0, 10 10)" );
  QgsGeometry geom( QgsGeometry::fromWkt( wkt ) );
  geom.translate( 10, -5 );
  QString obtained = geom.exportToWkt();
  QString expected = QStringLiteral( "LineString (10 -5, 20 -5, 20 5)" );
  QCOMPARE( obtained, expected );
  geom.translate( -10, 5 );
  obtained = geom.exportToWkt();
  QCOMPARE( obtained, wkt );

  wkt = QStringLiteral( "Polygon ((-2 4, -2 -10, 2 3, -2 4),(1 1, -1 1, -1 -1, 1 1))" );
  geom = QgsGeometry::fromWkt( wkt );
  geom.translate( -2, 10 );
  obtained = geom.exportToWkt();
  expected = QStringLiteral( "Polygon ((-4 14, -4 0, 0 13, -4 14),(-1 11, -3 11, -3 9, -1 11))" );
  QCOMPARE( obtained, expected );
  geom.translate( 2, -10 );
  obtained = geom.exportToWkt();
  QCOMPARE( obtained, wkt );

  wkt = QStringLiteral( "Point (40 50)" );
  geom = QgsGeometry::fromWkt( wkt );
  geom.translate( -2, 10 );
  obtained = geom.exportToWkt();
  expected = QStringLiteral( "Point (38 60)" );
  QCOMPARE( obtained, expected );
  geom.translate( 2, -10 );
  obtained = geom.exportToWkt();
  QCOMPARE( obtained, wkt );

}

void TestQgsGeometry::rotateCheck1()
{
  QString wkt = QStringLiteral( "LineString (0 0, 10 0, 10 10)" );
  QgsGeometry geom( QgsGeometry::fromWkt( wkt ) );
  geom.rotate( 90, QgsPointXY( 0, 0 ) );
  QString obtained = geom.exportToWkt();
  QString expected = QStringLiteral( "LineString (0 0, 0 -10, 10 -10)" );
  QCOMPARE( obtained, expected );
  geom.rotate( -90, QgsPointXY( 0, 0 ) );
  obtained = geom.exportToWkt();
  QCOMPARE( obtained, wkt );

  wkt = QStringLiteral( "Polygon ((-2 4, -2 -10, 2 3, -2 4),(1 1, -1 1, -1 -1, 1 1))" );
  geom = QgsGeometry::fromWkt( wkt );
  geom.rotate( 90, QgsPointXY( 0, 0 ) );
  obtained = geom.exportToWkt();
  expected = QStringLiteral( "Polygon ((4 2, -10 2, 3 -2, 4 2),(1 -1, 1 1, -1 1, 1 -1))" );
  QCOMPARE( obtained, expected );
  geom.rotate( -90, QgsPointXY( 0, 0 ) );
  obtained = geom.exportToWkt();
  QCOMPARE( obtained, wkt );

  wkt = QStringLiteral( "Point (40 50)" );
  geom = QgsGeometry::fromWkt( wkt );
  geom.rotate( 90, QgsPointXY( 0, 0 ) );
  obtained = geom.exportToWkt();
  expected = QStringLiteral( "Point (50 -40)" );
  QCOMPARE( obtained, expected );
  geom.rotate( -90, QgsPointXY( 0, 0 ) );
  obtained = geom.exportToWkt();
  QCOMPARE( obtained, wkt );
  geom.rotate( 180, QgsPointXY( 40, 0 ) );
  expected = QStringLiteral( "Point (40 -50)" );
  obtained = geom.exportToWkt();
  QCOMPARE( obtained, expected );
  geom.rotate( 180, QgsPointXY( 40, 0 ) ); // round-trip
  obtained = geom.exportToWkt();
  QCOMPARE( obtained, wkt );

}

void TestQgsGeometry::unionCheck1()
{
  // should be a multipolygon with 2 parts as A does not intersect C
  QgsGeometry mypUnionGeometry  =  mpPolygonGeometryA.combine( mpPolygonGeometryC );
  qDebug() << "Geometry Type: " << QgsWkbTypes::displayString( mypUnionGeometry.wkbType() );
  QVERIFY( mypUnionGeometry.wkbType() == QgsWkbTypes::MultiPolygon );
  QgsMultiPolygon myMultiPolygon = mypUnionGeometry.asMultiPolygon();
  QVERIFY( myMultiPolygon.size() > 0 ); //check that the union did not fail
  dumpMultiPolygon( myMultiPolygon );
  QVERIFY( renderCheck( "geometry_unionCheck1", "Checking A union C produces 2 polys" ) );
}

void TestQgsGeometry::unionCheck2()
{
  // should be a single polygon as A intersect B
  QgsGeometry mypUnionGeometry  =  mpPolygonGeometryA.combine( mpPolygonGeometryB );
  qDebug() << "Geometry Type: " << QgsWkbTypes::displayString( mypUnionGeometry.wkbType() );
  QVERIFY( mypUnionGeometry.wkbType() == QgsWkbTypes::Polygon );
  QgsPolygon myPolygon = mypUnionGeometry.asPolygon();
  QVERIFY( myPolygon.size() > 0 ); //check that the union created a feature
  dumpPolygon( myPolygon );
  QVERIFY( renderCheck( "geometry_unionCheck2", "Checking A union B produces single union poly" ) );
}

void TestQgsGeometry::differenceCheck1()
{
  // should be same as A since A does not intersect C so diff is 100% of A
  QgsGeometry mypDifferenceGeometry( mpPolygonGeometryA.difference( mpPolygonGeometryC ) );
  qDebug() << "Geometry Type: " << QgsWkbTypes::displayString( mypDifferenceGeometry.wkbType() );
  QVERIFY( mypDifferenceGeometry.wkbType() == QgsWkbTypes::Polygon );
  QgsPolygon myPolygon = mypDifferenceGeometry.asPolygon();
  QVERIFY( myPolygon.size() > 0 ); //check that the union did not fail
  dumpPolygon( myPolygon );
  QVERIFY( renderCheck( "geometry_differenceCheck1", "Checking (A - C) = A" ) );
}

void TestQgsGeometry::differenceCheck2()
{
  // should be a single polygon as (A - B) = subset of A
  QgsGeometry mypDifferenceGeometry( mpPolygonGeometryA.difference( mpPolygonGeometryB ) );
  qDebug() << "Geometry Type: " << QgsWkbTypes::displayString( mypDifferenceGeometry.wkbType() );
  QVERIFY( mypDifferenceGeometry.wkbType() == QgsWkbTypes::Polygon );
  QgsPolygon myPolygon = mypDifferenceGeometry.asPolygon();
  QVERIFY( myPolygon.size() > 0 ); //check that the union created a feature
  dumpPolygon( myPolygon );
  QVERIFY( renderCheck( "geometry_differenceCheck2", "Checking (A - B) = subset of A" ) );
}
void TestQgsGeometry::bufferCheck()
{
  // should be a single polygon
  QgsGeometry mypBufferGeometry( mpPolygonGeometryB.buffer( 10, 10 ) );
  qDebug() << "Geometry Type: " << QgsWkbTypes::displayString( mypBufferGeometry.wkbType() );
  QVERIFY( mypBufferGeometry.wkbType() == QgsWkbTypes::Polygon );
  QgsPolygon myPolygon = mypBufferGeometry.asPolygon();
  QVERIFY( myPolygon.size() > 0 ); //check that the buffer created a feature
  dumpPolygon( myPolygon );
  QVERIFY( renderCheck( "geometry_bufferCheck", "Checking buffer(10,10) of B", 10 ) );
}

void TestQgsGeometry::smoothCheck()
{
  //can't smooth a point
  QString wkt = QStringLiteral( "Point (40 50)" );
  QgsGeometry geom( QgsGeometry::fromWkt( wkt ) );
  QgsGeometry result = geom.smooth( 1, 0.25 );
  QString obtained = result.exportToWkt();
  QCOMPARE( obtained, wkt );

  //linestring
  wkt = QStringLiteral( "LineString(0 0, 10 0, 10 10, 20 10)" );
  geom = QgsGeometry::fromWkt( wkt );
  result = geom.smooth( 1, 0.25 );
  QgsPolyline line = result.asPolyline();
  QgsPolyline expectedLine;
  expectedLine << QgsPointXY( 0, 0 ) << QgsPointXY( 7.5, 0 ) << QgsPointXY( 10.0, 2.5 )
               << QgsPointXY( 10.0, 7.5 ) << QgsPointXY( 12.5, 10.0 ) << QgsPointXY( 20.0, 10.0 );
  QVERIFY( QgsGeometry::compare( line, expectedLine ) );

  //linestring, with min distance
  wkt = QStringLiteral( "LineString(0 0, 10 0, 10 10, 15 10, 15 20)" );
  geom = QgsGeometry::fromWkt( wkt );
  result = geom.smooth( 1, 0.25, 6 );
  line = result.asPolyline();
  expectedLine.clear();
  expectedLine << QgsPointXY( 0, 0 ) << QgsPointXY( 7.5, 0 ) << QgsPointXY( 10.0, 2.5 )
               << QgsPointXY( 10.0, 7.5 ) << QgsPointXY( 15, 12.5 ) << QgsPointXY( 15.0, 20.0 );
  QVERIFY( QgsGeometry::compare( line, expectedLine ) );

  //linestring, with max angle
  wkt = QStringLiteral( "LineString(0 0, 10 0, 15 5, 25 -5, 30 -5 )" );
  geom = QgsGeometry::fromWkt( wkt );
  result = geom.smooth( 1, 0.25, 0, 50 );
  line = result.asPolyline();
  expectedLine.clear();
  expectedLine << QgsPointXY( 0, 0 ) << QgsPointXY( 7.5, 0 ) << QgsPointXY( 11.25, 1.25 )
               << QgsPointXY( 15.0, 5.0 ) << QgsPointXY( 22.5, -2.5 ) << QgsPointXY( 26.25, -5 ) << QgsPointXY( 30, -5 );
  QVERIFY( QgsGeometry::compare( line, expectedLine ) );

  //linestring, with max angle, other direction
  wkt = QStringLiteral( "LineString( 30 -5, 25 -5, 15 5, 10 0, 0 0 )" );
  geom = QgsGeometry::fromWkt( wkt );
  result = geom.smooth( 1, 0.25, 0, 50 );
  line = result.asPolyline();
  expectedLine.clear();
  expectedLine << QgsPointXY( 30, -5 ) << QgsPointXY( 26.25, -5 ) << QgsPointXY( 22.5, -2.5 )
               << QgsPointXY( 15.0, 5.0 ) << QgsPointXY( 11.25, 1.25 ) << QgsPointXY( 7.5, 0 ) << QgsPointXY( 0, 0 );
  QVERIFY( QgsGeometry::compare( line, expectedLine ) );

  //linestring, max angle, first corner sharp
  wkt = QStringLiteral( "LineString(0 0, 10 0, 10 10 )" );
  geom = QgsGeometry::fromWkt( wkt );
  result = geom.smooth( 1, 0.25, 0, 50 );
  line = result.asPolyline();
  expectedLine.clear();
  expectedLine << QgsPointXY( 0, 0 ) << QgsPointXY( 10, 0 ) << QgsPointXY( 10, 10 );
  QVERIFY( QgsGeometry::compare( line, expectedLine ) );

  wkt = QStringLiteral( "MultiLineString ((0 0, 10 0, 10 10, 20 10),(30 30, 40 30, 40 40, 50 40))" );
  geom = QgsGeometry::fromWkt( wkt );
  result = geom.smooth( 1, 0.25 );
  QgsMultiPolyline multiLine = result.asMultiPolyline();
  QgsMultiPolyline expectedMultiline;
  expectedMultiline << ( QgsPolyline() << QgsPointXY( 0, 0 ) << QgsPointXY( 7.5, 0 ) << QgsPointXY( 10.0, 2.5 )
                         <<  QgsPointXY( 10.0, 7.5 ) << QgsPointXY( 12.5, 10.0 ) << QgsPointXY( 20.0, 10.0 ) )
                    << ( QgsPolyline() << QgsPointXY( 30.0, 30.0 ) << QgsPointXY( 37.5, 30.0 ) << QgsPointXY( 40.0, 32.5 )
                         << QgsPointXY( 40.0, 37.5 ) << QgsPointXY( 42.5, 40.0 ) << QgsPointXY( 50.0, 40.0 ) );
  QVERIFY( QgsGeometry::compare( multiLine, expectedMultiline ) );

  //polygon
  wkt = QStringLiteral( "Polygon ((0 0, 10 0, 10 10, 0 10, 0 0 ),(2 2, 4 2, 4 4, 2 4, 2 2))" );
  geom = QgsGeometry::fromWkt( wkt );
  result = geom.smooth( 1, 0.25 );
  QgsPolygon poly = result.asPolygon();
  QgsPolygon expectedPolygon;
  expectedPolygon << ( QgsPolyline() << QgsPointXY( 2.5, 0 ) << QgsPointXY( 7.5, 0 ) << QgsPointXY( 10.0, 2.5 )
                       <<  QgsPointXY( 10.0, 7.5 ) << QgsPointXY( 7.5, 10.0 ) << QgsPointXY( 2.5, 10.0 ) << QgsPointXY( 0, 7.5 )
                       << QgsPointXY( 0, 2.5 ) << QgsPointXY( 2.5, 0 ) )
                  << ( QgsPolyline() << QgsPointXY( 2.5, 2.0 ) << QgsPointXY( 3.5, 2.0 ) << QgsPointXY( 4.0, 2.5 )
                       << QgsPointXY( 4.0, 3.5 ) << QgsPointXY( 3.5, 4.0 ) << QgsPointXY( 2.5, 4.0 )
                       << QgsPointXY( 2.0, 3.5 ) << QgsPointXY( 2.0, 2.5 ) << QgsPointXY( 2.5, 2.0 ) );
  QVERIFY( QgsGeometry::compare( poly, expectedPolygon ) );

  //polygon with max angle - should be unchanged
  wkt = QStringLiteral( "Polygon ((0 0, 10 0, 10 10, 0 10, 0 0))" );
  geom = QgsGeometry::fromWkt( wkt );
  result = geom.smooth( 1, 0.25, -1, 50 );
  poly = result.asPolygon();
  expectedPolygon.clear();
  expectedPolygon << ( QgsPolyline() << QgsPointXY( 0, 0 ) << QgsPointXY( 10, 0 ) << QgsPointXY( 10.0, 10 )
                       <<  QgsPointXY( 0, 10 ) << QgsPointXY( 0, 0 ) );
  QVERIFY( QgsGeometry::compare( poly, expectedPolygon ) );

  //multipolygon)
  wkt = QStringLiteral( "MultiPolygon (((0 0, 10 0, 10 10, 0 10, 0 0 )),((2 2, 4 2, 4 4, 2 4, 2 2)))" );
  geom = QgsGeometry::fromWkt( wkt );
  result = geom.smooth( 1, 0.1 );
  QgsMultiPolygon multipoly = result.asMultiPolygon();
  QgsMultiPolygon expectedMultiPoly;
  expectedMultiPoly
      << ( QgsPolygon() << ( QgsPolyline() << QgsPointXY( 1.0, 0 ) << QgsPointXY( 9, 0 ) << QgsPointXY( 10.0, 1 )
                             <<  QgsPointXY( 10.0, 9 ) << QgsPointXY( 9, 10.0 ) << QgsPointXY( 1, 10.0 ) << QgsPointXY( 0, 9 )
                             << QgsPointXY( 0, 1 ) << QgsPointXY( 1, 0 ) ) )
      << ( QgsPolygon() << ( QgsPolyline() << QgsPointXY( 2.2, 2.0 ) << QgsPointXY( 3.8, 2.0 ) << QgsPointXY( 4.0, 2.2 )
                             <<  QgsPointXY( 4.0, 3.8 ) << QgsPointXY( 3.8, 4.0 ) << QgsPointXY( 2.2, 4.0 ) << QgsPointXY( 2.0, 3.8 )
                             << QgsPointXY( 2, 2.2 ) << QgsPointXY( 2.2, 2 ) ) );
  QVERIFY( QgsGeometry::compare( multipoly, expectedMultiPoly ) );
}

void TestQgsGeometry::unaryUnion()
{
  //test QgsGeometry::unaryUnion with null geometry
  QString wkt1 = QStringLiteral( "Polygon ((0 0, 10 0, 10 10, 0 10, 0 0 ))" );
  QString wkt2 = QStringLiteral( "Polygon ((2 2, 4 2, 4 4, 2 4, 2 2))" );
  QgsGeometry geom1( QgsGeometry::fromWkt( wkt1 ) );
  QgsGeometry geom2( QgsGeometry::fromWkt( wkt2 ) );
  QgsGeometry empty;
  QList< QgsGeometry > list;
  list << geom1 << empty << geom2;

  QgsGeometry result( QgsGeometry::unaryUnion( list ) );
  Q_UNUSED( result );
}

void TestQgsGeometry::dataStream()
{
  QString wkt = QStringLiteral( "Point (40 50)" );
  QgsGeometry geom( QgsGeometry::fromWkt( wkt ) );

  QByteArray ba;
  QDataStream ds( &ba, QIODevice::ReadWrite );
  ds << geom;

  QgsGeometry resultGeometry;
  ds.device()->seek( 0 );
  ds >> resultGeometry;

  QCOMPARE( geom.geometry()->asWkt(), resultGeometry.geometry()->asWkt() );

  //also test with geometry without data
  std::unique_ptr<QgsGeometry> emptyGeom( new QgsGeometry() );

  QByteArray ba2;
  QDataStream ds2( &ba2, QIODevice::ReadWrite );
  ds2 << emptyGeom.get();

  ds2.device()->seek( 0 );
  ds2 >> resultGeometry;

  QVERIFY( resultGeometry.isNull() );
}

void TestQgsGeometry::exportToGeoJSON()
{
  //Point
  QString wkt = QStringLiteral( "Point (40 50)" );
  QgsGeometry geom( QgsGeometry::fromWkt( wkt ) );
  QString obtained = geom.exportToGeoJSON();
  QString geojson = QStringLiteral( "{\"type\": \"Point\", \"coordinates\": [40, 50]}" );
  QCOMPARE( obtained, geojson );

  //MultiPoint
  wkt = QStringLiteral( "MultiPoint (0 0, 10 0, 10 10, 20 10)" );
  geom = QgsGeometry::fromWkt( wkt );
  obtained = geom.exportToGeoJSON();
  geojson = QStringLiteral( "{\"type\": \"MultiPoint\", \"coordinates\": [ [0, 0], [10, 0], [10, 10], [20, 10]] }" );
  QCOMPARE( obtained, geojson );

  //Linestring
  wkt = QStringLiteral( "LineString(0 0, 10 0, 10 10, 20 10)" );
  geom = QgsGeometry::fromWkt( wkt );
  obtained = geom.exportToGeoJSON();
  geojson = QStringLiteral( "{\"type\": \"LineString\", \"coordinates\": [ [0, 0], [10, 0], [10, 10], [20, 10]]}" );
  QCOMPARE( obtained, geojson );

  //MultiLineString
  wkt = QStringLiteral( "MultiLineString ((0 0, 10 0, 10 10, 20 10),(30 30, 40 30, 40 40, 50 40))" );
  geom = QgsGeometry::fromWkt( wkt );
  obtained = geom.exportToGeoJSON();
  geojson = QStringLiteral( "{\"type\": \"MultiLineString\", \"coordinates\": [[ [0, 0], [10, 0], [10, 10], [20, 10]], [ [30, 30], [40, 30], [40, 40], [50, 40]]] }" );
  QCOMPARE( obtained, geojson );

  //Polygon
  wkt = QStringLiteral( "Polygon ((0 0, 10 0, 10 10, 0 10, 0 0 ),(2 2, 4 2, 4 4, 2 4, 2 2))" );
  geom = QgsGeometry::fromWkt( wkt );
  obtained = geom.exportToGeoJSON();
  geojson = QStringLiteral( "{\"type\": \"Polygon\", \"coordinates\": [[ [0, 0], [10, 0], [10, 10], [0, 10], [0, 0]], [ [2, 2], [4, 2], [4, 4], [2, 4], [2, 2]]] }" );
  QCOMPARE( obtained, geojson );

  //MultiPolygon
  wkt = QStringLiteral( "MultiPolygon (((0 0, 10 0, 10 10, 0 10, 0 0 )),((2 2, 4 2, 4 4, 2 4, 2 2)))" );
  geom = QgsGeometry::fromWkt( wkt );
  obtained = geom.exportToGeoJSON();
  geojson = QStringLiteral( "{\"type\": \"MultiPolygon\", \"coordinates\": [[[ [0, 0], [10, 0], [10, 10], [0, 10], [0, 0]]], [[ [2, 2], [4, 2], [4, 4], [2, 4], [2, 2]]]] }" );
  QCOMPARE( obtained, geojson );

  // no geometry
  QgsGeometry nullGeom( nullptr );
  obtained = nullGeom.exportToGeoJSON();
  geojson = QStringLiteral( "null" );
  QCOMPARE( obtained, geojson );
}

bool TestQgsGeometry::renderCheck( const QString &testName, const QString &comment, int mismatchCount )
{
  mReport += "<h2>" + testName + "</h2>\n";
  mReport += "<h3>" + comment + "</h3>\n";
  QString myTmpDir = QDir::tempPath() + '/';
  QString myFileName = myTmpDir + testName + ".png";
  mImage.save( myFileName, "PNG" );
  QgsRenderChecker myChecker;
  myChecker.setControlName( "expected_" + testName );
  myChecker.setRenderedImage( myFileName );
  bool myResultFlag = myChecker.compareImages( testName, mismatchCount );
  mReport += myChecker.report();
  return myResultFlag;
}

void TestQgsGeometry::dumpMultiPolygon( QgsMultiPolygon &multiPolygon )
{
  qDebug( "Multipolygon Geometry Dump" );
  for ( int i = 0; i < multiPolygon.size(); i++ )
  {
    QgsPolygon myPolygon = multiPolygon.at( i );
    qDebug( "\tPolygon in multipolygon: %d", i );
    dumpPolygon( myPolygon );
  }
}

void TestQgsGeometry::dumpPolygon( QgsPolygon &polygon )
{
  QVector<QPointF> myPoints;
  for ( int j = 0; j < polygon.size(); j++ )
  {
    QgsPolyline myPolyline = polygon.at( j ); //rings of polygon
    qDebug( "\t\tRing in polygon: %d", j );

    for ( int k = 0; k < myPolyline.size(); k++ )
    {
      QgsPointXY myPoint = myPolyline.at( k );
      qDebug( "\t\t\tPoint in ring %d : %s", k, myPoint.toString().toLocal8Bit().constData() );
      myPoints << QPointF( myPoint.x(), myPoint.y() );
    }
  }
  mpPainter->drawPolygon( myPoints );
}

void TestQgsGeometry::dumpPolyline( QgsPolyline &polyline )
{
  QVector<QPointF> myPoints;
//  QgsPolyline myPolyline = polyline.at( j ); //rings of polygon
  for ( int j = 0; j < polyline.size(); j++ )
  {
    QgsPointXY myPoint = polyline.at( j );
//    QgsPolyline myPolyline = polygon.at( j ); //rings of polygon
    myPoints << QPointF( myPoint.x(), myPoint.y() );
    qDebug( "\t\tPoint in line: %d", j );

//    for ( int k = 0; k < myPolyline.size(); k++ )
//    {
//      QgsPointXY myPoint = myPolyline.at( k );
//      qDebug( "\t\t\tPoint in ring %d : %s", k, myPoint.toString().toLocal8Bit().constData() );
//      myPoints << QPointF( myPoint.x(), myPoint.y() );
//    }
  }
  mpPainter->drawPolyline( myPoints );
}

QString TestQgsGeometry::elemToString( const QDomElement &elem ) const
{
  QString s;
  QTextStream stream( &s );
  elem.save( stream, -1 );

  return s;
}

void TestQgsGeometry::wkbInOut()
{
  // Premature end of WKB
  // See https://issues.qgis.org/issues/14182
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
  QVERIFY( badHeader.isNull() );
  QCOMPARE( badHeader.wkbType(), QgsWkbTypes::Unknown );
}

void TestQgsGeometry::segmentizeCircularString()
{
  QString wkt( QStringLiteral( "CIRCULARSTRING( 0 0, 0.5 0.5, 2 0 )" ) );
  QgsCircularString *circularString = dynamic_cast<QgsCircularString *>( QgsGeometryFactory::geomFromWkt( wkt ) );
  QVERIFY( circularString );
  QgsLineString *lineString = circularString->curveToLine();
  QVERIFY( lineString );
  QgsPointSequence points;
  lineString->points( points );

  delete circularString;
  delete lineString;

  //make sure the curve point is part of the segmentized result
  QVERIFY( points.contains( QgsPoint( 0.5, 0.5 ) ) );
}

void TestQgsGeometry::directionNeutralSegmentation()
{
  //Tests, if segmentation of a circularstring is the same in both directions
  QString CWCircularStringWkt( QStringLiteral( "CIRCULARSTRING( 0 0, 0.5 0.5, 0.83 7.33 )" ) );
  QgsCircularString *CWCircularString = static_cast<QgsCircularString *>( QgsGeometryFactory::geomFromWkt( CWCircularStringWkt ) );
  QgsLineString *CWLineString = CWCircularString->curveToLine();

  QString CCWCircularStringWkt( QStringLiteral( "CIRCULARSTRING( 0.83 7.33, 0.5 0.5, 0 0 )" ) );
  QgsCircularString *CCWCircularString = static_cast<QgsCircularString *>( QgsGeometryFactory::geomFromWkt( CCWCircularStringWkt ) );
  QgsLineString *CCWLineString = CCWCircularString->curveToLine();
  QgsLineString *reversedCCWLineString = CCWLineString->reversed();

  bool equal = ( *CWLineString == *reversedCCWLineString );

  delete CWCircularString;
  delete CCWCircularString;
  delete CWLineString;
  delete CCWLineString;
  delete reversedCCWLineString;

  QVERIFY( equal );
}

void TestQgsGeometry::poleOfInaccessibility()
{
  QString poly1Wkt( "Polygon ((3116 3071, 3394 3431, 3563 3362, 3611 3205, 3599 3181, 3477 3281, 3449 3160, 3570 3127, 3354 3116,"
                    " 3436 3008, 3158 2907, 2831 2438, 3269 2916, 3438 2885, 3295 2799, 3407 2772, 3278 2629, 3411 2689, 3329 2611,"
                    " 3360 2531, 3603 2800, 3598 2501, 3317 2429, 3329 2401, 3170 2340, 3142 2291, 3524 2403, 3598 2233, 3460 2117,"
                    " 3590 1931, 3364 1753, 3597 1875, 3639 1835, 3660 1733, 3600 1771, 3538 1694, 3661 1732, 3359 1554, 3334 1554,"
                    " 3341 1588, 3317 1588, 3305 1644, 3286 1656, 3115 1255, 3072 1252, 3078 1335, 3046 1355, 2984 1234, 2983 1409,"
                    " 2876 1222, 2525 1161, 2488 787, 2162 913, 2079 661, 2270 380, 2188 823, 2510 592, 2659 992, 2911 1118, 2943 938,"
                    " 2957 1097, 3092 1002, 3006 1097, 3233 1282, 3325 1291, 3296 1116, 3333 1115, 3391 1333, 3434 1274, 3413 1326,"
                    " 3449 1327, 3473 1408, 3490 1430, 3526 1434, 3198 -112, 3263 -87, 3289 -128, 4224 -128, 4224 -128, 3474 -128,"
                    " 3475 -116, 3486 -120, 3458 -49, 3523 -78, 3513 -128, 3509 -119, 3509 -128, 3717 -128, 3705 -60, 3735 -16,"
                    " 3714 38, 3758 88, 3825 47, 3812 -11, 3859 -51, 3871 49, 3760 149, 3636 -74, 3510 126, 3501 245, 3504 270,"
                    " 3511 284, 3582 16, 3631 19, 3569 125, 3570 193, 3610 212, 3583 119, 3655 29, 3738 170, 3561 466, 3826 549,"
                    " 3527 604, 3609 833, 3681 798, 3956 1127, 3917 964, 4043 850, 4049 1096, 4193 1052, 4191 1078, 4208 1106,"
                    " 4222 1110, 4224 1109, 4224 1144, 4202 1158, 4177 1161, 4182 1181, 4075 1201, 4141 1275, 4215 1215, 4221 1223,"
                    " 4219 1231, 4224 1243, 4224 1257, 4224 1262, 4224 1345, 4224 1339, 4224 1328, 4215 1335, 4203 1355, 4215 1369,"
                    " 4224 1363, 4215 1377, 4208 1387, 4217 1401, 4224 1403, 4224 1520, 4219 1535, 4221 1544, 4199 1593, 4223 1595,"
                    " 4206 1626, 4214 1648, 4224 1645, 4224 1640, 4224 2108, 4220 2125, 4224 2125, 4224 2143, 4205 2141, 4180 2159,"
                    " 4201 2182, 4163 2189, 4176 2229, 4199 2211, 4210 2218, 4212 2210, 4223 2214, 4224 2207, 4224 2216, 4217 2225,"
                    " 4221 2233, 4203 2227, 4209 2248, 4185 2240, 4198 2276, 4144 2218, 4091 2343, 4119 2332, 4121 2347, 4155 2337,"
                    " 4180 2355, 4200 2342, 4201 2354, 4213 2352, 4224 2348, 4224 2356, 4207 2361, 4184 2358, 4176 2367, 4106 2355,"
                    " 3983 2765, 4050 3151, 4139 2720, 4209 2589, 4211 2600, 4219 2599, 4224 2592, 4224 2574, 4223 2566, 4224 2562,"
                    " 4224 2553, 4224 2552, 4224 -128, 4224 4224, 4205 4224, 4015 3513, 3993 3494, 3873 3533, 3887 3539, 3923 3524,"
                    " 3950 3529, 4018 3572, 3987 3633, 3983 3571, 3955 3583, 3936 3547, 3882 3539, 3913 3557, 3920 3598, 3901 3596,"
                    " 3923 3631, 3914 3628, 3919 3647, 3922 3656, 3917 3666, 3523 3438, 3631 3564, 3527 3597, 3718 3655, 3578 3672,"
                    " 3660 3867, 3543 3628, 3416 3725, 3487 3503, 3274 3583, 3271 3644, 3197 3671, 3210 3775, 3184 3788, 3181 3672,"
                    " 3306 3521, 3292 3508, 3229 3565, 3219 3564, 3216 3574, 3192 3578, 3297 3444, 3089 3395, 3029 3028, 2973 3133,"
                    " 2529 2945, 2538 2811, 2461 2901, 2170 2839, 2121 2797, 2156 2733, 2105 2709, 2096 2695, 2114 2621, 2102 2693,"
                    " 2168 2738, 2167 2778, 2447 2765, 2441 2866, 2527 2793, 2670 2938, 2626 2651, 2688 2623, 2740 2922, 3084 2960,"
                    " 3116 3071),(4016 1878, 4029 1859, 4008 1839, 4006 1863, 4016 1878),(3315 1339, 3331 1324, 3290 1293, 3305 1315,"
                    " 3315 1339),(4136 3071, 4136 3080, 4143 3020, 4137 3036, 4136 3071),(4218 3073, 4183 3114, 4117 3157, 4159 3147,"
                    " 4218 3073),(3912 3542, 3934 3536, 3955 3536, 3937 3527, 3900 3540, 3912 3542),(4050 3172, 4043 3210, 4085 3209,"
                    " 4090 3179, 4050 3172),(4151 2998, 4158 2977, 4159 2946, 4147 2988, 4151 2998),(2920 3005, 2935 2994, 2864 2973,"
                    " 2905 3016, 2920 3005),(3571 2424, 3545 2469, 3608 2480, 3596 2434, 3571 2424),(4095 1229, 4073 1234, 4076 1293,"
                    " 4121 1285, 4095 1229),(4173 1536, 4153 1576, 4166 1585, 4198 1571, 4188 1532, 4213 1535, 4224 1512, 4224 1511,"
                    " 4209 1511, 4198 1506, 4190 1517, 4194 1509, 4192 1499, 4200 1496, 4202 1504, 4224 1510, 4224 1488, 4215 1486,"
                    " 4216 1478, 4224 1472, 4224 1464, 4207 1458, 4193 1464, 4173 1536),(3934 1537, 3968 1630, 3960 1666, 3968 1673,"
                    " 3975 1562, 3934 1537),(4182 1653, 4196 1624, 4166 1614, 4157 1674, 4216 1671, 4182 1653),(4200 1619, 4196 1620,"
                    " 4200 1632, 4195 1642, 4207 1648, 4200 1619),(4026 1835, 4025 1830, 4016 1808, 4007 1836, 4026 1835),(4199 1384,"
                    " 4182 1389, 4206 1412, 4216 1401, 4199 1384),(3926 1251, 3969 1206, 3913 1149, 3878 1173, 3876 1229, 3926 1251),"
                    " (3926 1354, 3958 1389, 3997 1384, 3991 1352, 3960 1322, 3955 1299, 3926 1354),(3964 1319, 3974 1329, 3984 1285,"
                    " 3963 1301, 3964 1319),(3687 959, 3696 903, 3678 885, 3665 930, 3687 959),(3452 79, 3437 124, 3456 149, 3476 141,"
                    " 3452 79),(3751 927, 3738 906, 3719 942, 3739 929, 3751 927))" );

  QString poly2Wkt( "Polygon ((-128 4224, -128 2734, -11 2643, -101 2919, 120 2654, 19 2846, 217 2897, -13 2873, -79 2998, -106 2989,"
                    " -128 3002, -128 4224, -128 4224, 1249 4224, 1247 4199, 1231 4132, 909 3902, 775 3278, 509 2903, 470 2603, 663 2311,"
                    " 889 2134, 989 2146, 534 2585, 575 2880, 833 3112, 833 3037, 1025 2977, 834 3096, 946 3217, 923 3153, 971 3094,"
                    " 997 3103, 1166 3423, 1198 3329, 1233 3367, 1270 3327, 1274 3354, 1290 3360, 1321 3322, 1331 3318, 1343 3325,"
                    " 1310 3073, 1363 3093, 1413 3186, 1325 3398, 1458 3364, 1493 3426, 1526 3394, 1588 3465, 1417 3824, 1458 3825,"
                    " 1522 3849, 1729 3488, 2018 3223, 1908 3291, 1924 3238, 1899 3187, 1913 3150, 2070 3041, 2102 3063, 2112 3053,"
                    " 2168 3085, 2101 2994, 2265 2863, 1890 2593, 2106 2713, 2130 2706, 2108 2678, 2117 2647, 2105 2636, 2099 2604,"
                    " 2094 2591, 2088 2575, 2088 2564, 2249 2751, 2441 2618, 2689 1728, 2681 2323, 2776 1685, 2711 1708, 2673 1680,"
                    " 2675 1639, 2810 1522, 2765 1535, 2734 1510, 2850 1332, 2863 1186, 2847 1025, 2832 985, 2739 1025, 2850 1090,"
                    " 2859 1174, 2853 1235, 2827 1235, 2839 1279, 2811 1286, 2751 1272, 2851 1160, 2788 1159, 2724 1204, 2713 1198,"
                    " 2708 1213, 2699 1221, 2724 1179, 2797 1145, 2785 1123, 2848 1143, 2821 1092, 2284 980, 2288 963, 2264 962,"
                    " 2261 948, 2247 958, 2194 947, 2120 900, 2047 809, 2434 923, 2450 817, 2528 737, 2527 667, 2641 544, 2641 460,"
                    " 2710 452, 2687 447, 2681 435, 2693 330, 2772 327, 2766 291, 2779 245, 2769 219, 2771 198, 2816 219, 2781 342,"
                    " 2612 647, 2903 188, 2803 539, 2950 206, 2927 342, 3123 339, 3092 300, 3073 175, 3082 160, 3412 -103, 4224 -128,"
                    " 4032 167, 3572 63, 3554 109, 3606 211, 3604 232, 3454 124, 3332 345, 3237 212, 3181 415, 2953 364, 2761 692,"
                    " 2819 788, 2997 769, 2997 626, 3199 659, 3155 730, 2929 805, 2908 841, 3218 717, 3276 744, 3246 723, 3270 658,"
                    " 4223 473, 4224 589, 3906 681, 3818 677, 3915 686, 4224 592, 4224 4224, -128 4224, -128 4224),(2049 3181,"
                    " 2224 3084, 2204 3040, 2169 3036, 2174 3084, 2155 3102, 2126 3123, 2109 3127, 2103 3102, 2049 3181),(1578 3811,"
                    " 1567 3883, 1675 3768, 1658 3712, 1659 3751, 1578 3811),(2304 2930, 2335 2918, 2287 2880, 2279 2926, 2304 2930),"
                    " (2316 2895, 2317 2895, 2316 2901, 2331 2901, 2332 2889, 2316 2895),(2304 2850, 2335 2861, 2344 2910, 2357 2828,"
                    " 2304 2850),(1714 3583, 1725 3638, 1682 3717, 1797 3564, 1714 3583),(1537 3873, 1456 3827, 1405 3876, 1461 3898,"
                    " 1537 3873),(2547 2560, 2375 2815, 2384 2825, 2470 2722, 2685 2336, 2648 2310, 2547 2560),(2107 3073, 2107 3098,"
                    " 2144 3086, 2122 3073, 2113 3059, 2107 3073),(2117 3120, 2156 3099, 2164 3083, 2106 3103, 2117 3120),(2303 2981,"
                    " 2226 3010, 2232 3064, 2286 3012, 2344 2928, 2303 2981),(2304 2858, 2291 2864, 2303 2885, 2324 2870, 2304 2858),"
                    " (2175 3002, 2179 2983, 2152 2991, 2175 3002, 2175 3002),(2175 3022, 2150 3017, 2144 3048, 2159 3031, 2184 3030,"
                    " 2175 3022),(2265 2953, 2270 2950, 2262 2950, 2254 2963, 2253 2979, 2265 2953),(2229 2928, 2250 2928, 2241 2922,"
                    " 2239 2914, 2224 2900, 2237 2914, 2229 2928),(2531 2473, 2520 2466, 2521 2474, 2511 2503, 2531 2473),(2547 2526,"
                    " 2529 2519, 2528 2541, 2544 2538, 2547 2526),(2559 646, 2513 810, 2463 835, 2462 930, 2746 731, 2559 646),(3840 653,"
                    " 3809 641, 3718 689, 3547 733, 3553 712, 3440 780, 3840 653),(3327 741, 3242 750, 3195 745, 3180 758, 3326 764,"
                    " 3440 742, 3327 741),(3282 702, 3265 699, 3273 721, 3290 714, 3282 702),(3762 662, 3783 654, 3786 638, 3742 653,"
                    " 3762 662),(3071 703, 3082 741, 3079 655, 3064 657, 3071 703),(3881 637, 3904 647, 3914 626, 3861 638, 3881 637))" );

  QgsGeometry poly1 = QgsGeometry::fromWkt( poly1Wkt );
  QgsGeometry poly2 = QgsGeometry::fromWkt( poly2Wkt );

  double distance;
  QgsPointXY point = poly1.poleOfInaccessibility( 1, &distance ).asPoint();
  QGSCOMPARENEAR( point.x(), 3867.37, 0.01 );
  QGSCOMPARENEAR( point.y(), 2126.45, 0.01 );
  QGSCOMPARENEAR( distance, 289.51, 0.01 );

  point = poly1.poleOfInaccessibility( 50 ).asPoint();
  QGSCOMPARENEAR( point.x(), 3855.33, 0.01 );
  QGSCOMPARENEAR( point.y(), 2117.55, 0.01 );

  point = poly2.poleOfInaccessibility( 1 ).asPoint();
  QGSCOMPARENEAR( point.x(), 3263.50, 0.01 );
  QGSCOMPARENEAR( point.y(), 3263.50, 0.01 );

  //test degenerate polygons
  QgsGeometry degen1 = QgsGeometry::fromWkt( "Polygon(( 0 0, 1 0, 2 0, 0 0 ))" );
  point = degen1.poleOfInaccessibility( 1 ).asPoint();
  QGSCOMPARENEAR( point.x(), 0, 0.01 );
  QGSCOMPARENEAR( point.y(), 0, 0.01 );

  QgsGeometry degen2 = QgsGeometry::fromWkt( "Polygon(( 0 0, 1 0, 1 1 , 1 0, 0 0 ))" );
  point = degen2.poleOfInaccessibility( 1 ).asPoint();
  QGSCOMPARENEAR( point.x(), 0, 0.01 );
  QGSCOMPARENEAR( point.y(), 0, 0.01 );

  //empty geometry
  QVERIFY( QgsGeometry().poleOfInaccessibility( 1 ).isNull() );

  // not a polygon
  QgsGeometry lineString = QgsGeometry::fromWkt( "LineString(1 0, 2 2 )" );
  QVERIFY( lineString.poleOfInaccessibility( 1 ).isNull() );

  // invalid threshold
  QVERIFY( poly1.poleOfInaccessibility( -1 ).isNull() );
  QVERIFY( poly1.poleOfInaccessibility( 0 ).isNull() );

  // curved geometry
  QgsGeometry curved = QgsGeometry::fromWkt( "CurvePolygon( CompoundCurve( CircularString(-0.44 0.35, 0.51 0.34, 0.56 0.21, 0.11 -0.33, 0.15 -0.35,"
                       "-0.93 -0.30, -1.02 -0.22, -0.49 0.01, -0.23 -0.04),(-0.23 -0.04, -0.44, 0.35)))" );
  point = curved.poleOfInaccessibility( 0.01 ).asPoint();
  QGSCOMPARENEAR( point.x(), -0.4324, 0.0001 );
  QGSCOMPARENEAR( point.y(), -0.2434, 0.0001 );

  // multipolygon
  QgsGeometry multiPoly = QgsGeometry::fromWkt( QStringLiteral( "MultiPolygon (((0 0, 10 0, 10 10, 0 10, 0 0)),((30 30, 50 30, 50 60, 30 60, 30 30)))" ) );
  point = multiPoly.poleOfInaccessibility( 0.01, &distance ).asPoint();
  QGSCOMPARENEAR( point.x(), 40, 0.0001 );
  QGSCOMPARENEAR( point.y(), 45, 0.0001 );
  QGSCOMPARENEAR( distance, 10.0, 0.00001 );
}

void TestQgsGeometry::makeValid()
{
  typedef QPair<QString, QString> InputAndExpectedWktPair;
  QList<InputAndExpectedWktPair> geoms;
  // dimension collapse
  geoms << qMakePair( QString( "LINESTRING(0 0)" ),
                      QString( "POINT(0 0)" ) );
  // unclosed ring
  geoms << qMakePair( QString( "POLYGON((10 22,10 32,20 32,20 22))" ),
                      QString( "POLYGON((10 22,10 32,20 32,20 22,10 22))" ) );
  // butterfly polygon (self-intersecting ring)
  geoms << qMakePair( QString( "POLYGON((0 0, 10 10, 10 0, 0 10, 0 0))" ),
                      QString( "MULTIPOLYGON(((5 5, 0 0, 0 10, 5 5)),((5 5, 10 10, 10 0, 5 5)))" ) );
  // polygon with extra tail (a part of the ring does not form any area)
  geoms << qMakePair( QString( "POLYGON((0 0, 1 0, 1 1, 0 1, 0 0, -1 0, 0 0))" ),
                      QString( "GEOMETRYCOLLECTION(POLYGON((0 0, 0 1, 1 1, 1 0, 0 0)), LINESTRING(0 0, -1 0))" ) );
  // collection with invalid geometries
  geoms << qMakePair( QString( "GEOMETRYCOLLECTION(LINESTRING(0 0, 0 0), POLYGON((0 0, 10 10, 10 0, 0 10, 0 0)), LINESTRING(10 0, 10 10))" ),
                      QString( "GEOMETRYCOLLECTION(POINT(0 0), MULTIPOLYGON(((5 5, 0 0, 0 10, 5 5)),((5 5, 10 10, 10 0, 5 5))), LINESTRING(10 0, 10 10)))" ) );

  Q_FOREACH ( const InputAndExpectedWktPair &pair, geoms )
  {
    QgsGeometry gInput = QgsGeometry::fromWkt( pair.first );
    QgsGeometry gExp = QgsGeometry::fromWkt( pair.second );
    QVERIFY( !gInput.isNull() );
    QVERIFY( !gExp.isNull() );

    QgsGeometry gValid = gInput.makeValid();
    QVERIFY( gValid.isGeosValid() );
    QVERIFY( gValid.isGeosEqual( gExp ) );
  }
}

void TestQgsGeometry::isSimple()
{
  typedef QPair<QString, bool> InputWktAndExpectedResult;
  QList<InputWktAndExpectedResult> geoms;
  geoms << qMakePair( QString( "LINESTRING(0 0, 1 0, 1 1)" ), true );
  geoms << qMakePair( QString( "LINESTRING(0 0, 1 0, 1 1, 0 0)" ), true );  // may be closed (linear ring)
  geoms << qMakePair( QString( "LINESTRING(0 0, 1 0, 1 1, 0 -1)" ), false ); // self-intersection
  geoms << qMakePair( QString( "LINESTRING(0 0, 1 0, 1 1, 0.5 0, 0 1)" ), false ); // self-tangency
  geoms << qMakePair( QString( "POINT(1 1)" ), true ); // points are simple
  geoms << qMakePair( QString( "POLYGON((0 0, 1 1, 1 1, 0 0))" ), true ); // polygons are always simple, even if they are invalid
  geoms << qMakePair( QString( "MULTIPOINT((1 1), (2 2))" ), true );
  geoms << qMakePair( QString( "MULTIPOINT((1 1), (1 1))" ), false );  // must not contain the same point twice
  geoms << qMakePair( QString( "MULTILINESTRING((0 0, 1 0), (0 1, 1 1))" ), true );
  geoms << qMakePair( QString( "MULTILINESTRING((0 0, 1 0), (0 0, 1 0))" ), true );  // may be touching at endpoints
  geoms << qMakePair( QString( "MULTILINESTRING((0 0, 1 1), (0 1, 1 0))" ), false );  // must not intersect each other
  geoms << qMakePair( QString( "MULTIPOLYGON(((0 0, 1 1, 1 1, 0 0)),((0 0, 1 1, 1 1, 0 0)))" ), true ); // multi-polygons are always simple

  Q_FOREACH ( const InputWktAndExpectedResult &pair, geoms )
  {
    QgsGeometry gInput = QgsGeometry::fromWkt( pair.first );
    QVERIFY( !gInput.isNull() );

    bool res = gInput.isSimple();
    QCOMPARE( res, pair.second );
  }
}

void TestQgsGeometry::reshapeGeometryLineMerge()
{
  int res;
  QgsGeometry g2D = QgsGeometry::fromWkt( "LINESTRING(10 10, 20 20)" );
  QgsGeometry g3D = QgsGeometry::fromWkt( "LINESTRINGZ(10 10 1, 20 20 2)" );

  // prepare 2D reshaping line
  QVector<QgsPoint> v2D_1, v2D_2;
  v2D_1 << QgsPoint( 20, 20 ) << QgsPoint( 30, 30 );
  v2D_2 << QgsPoint( 10, 10 ) << QgsPoint( -10, -10 );
  QgsLineString line2D_1( v2D_1 ), line2D_2( v2D_2 );

  // prepare 3D reshaping line
  QVector<QgsPoint> v3D_1, v3D_2;
  v3D_1 << QgsPoint( QgsWkbTypes::PointZ, 20, 20, 2 ) << QgsPoint( QgsWkbTypes::PointZ, 30, 30, 3 );
  v3D_2 << QgsPoint( QgsWkbTypes::PointZ, 10, 10, 1 ) << QgsPoint( QgsWkbTypes::PointZ, -10, -10, -1 );
  QgsLineString line3D_1( v3D_1 ), line3D_2( v3D_2 );

  // append with 2D line
  QgsGeometry g2D_1 = g2D;
  res = g2D_1.reshapeGeometry( line2D_1 );
  QCOMPARE( res, 0 );
  QCOMPARE( g2D_1.exportToWkt(), QString( "LineString (10 10, 20 20, 30 30)" ) );

  // prepend with 2D line
  QgsGeometry g2D_2 = g2D;
  res = g2D_2.reshapeGeometry( line2D_2 );
  QCOMPARE( res, 0 );
  QCOMPARE( g2D_2.exportToWkt(), QString( "LineString (-10 -10, 10 10, 20 20)" ) );

  // append with 3D line
  QgsGeometry g3D_1 = g3D;
  res = g3D_1.reshapeGeometry( line3D_1 );
  QCOMPARE( res, 0 );
  QCOMPARE( g3D_1.exportToWkt(), QString( "LineStringZ (10 10 1, 20 20 2, 30 30 3)" ) );

  // prepend with 3D line
  QgsGeometry g3D_2 = g3D;
  res = g3D_2.reshapeGeometry( line3D_2 );
  QCOMPARE( res, 0 );
  QCOMPARE( g3D_2.exportToWkt(), QString( "LineStringZ (-10 -10 -1, 10 10 1, 20 20 2)" ) );
}

void TestQgsGeometry::createCollectionOfType()
{
  std::unique_ptr< QgsGeometryCollection > collect( QgsGeometryFactory::createCollectionOfType( QgsWkbTypes::Unknown ) );
  QVERIFY( !collect );
  collect = QgsGeometryFactory::createCollectionOfType( QgsWkbTypes::Point );
  QCOMPARE( collect->wkbType(), QgsWkbTypes::MultiPoint );
  QVERIFY( dynamic_cast< QgsMultiPointV2 *>( collect.get() ) );
  collect = QgsGeometryFactory::createCollectionOfType( QgsWkbTypes::PointM );
  QCOMPARE( collect->wkbType(), QgsWkbTypes::MultiPointM );
  QVERIFY( dynamic_cast< QgsMultiPointV2 *>( collect.get() ) );
  collect = QgsGeometryFactory::createCollectionOfType( QgsWkbTypes::PointZM );
  QCOMPARE( collect->wkbType(), QgsWkbTypes::MultiPointZM );
  QVERIFY( dynamic_cast< QgsMultiPointV2 *>( collect.get() ) );
  collect = QgsGeometryFactory::createCollectionOfType( QgsWkbTypes::PointZ );
  QCOMPARE( collect->wkbType(), QgsWkbTypes::MultiPointZ );
  QVERIFY( dynamic_cast< QgsMultiPointV2 *>( collect.get() ) );
  collect = QgsGeometryFactory::createCollectionOfType( QgsWkbTypes::MultiPoint );
  QCOMPARE( collect->wkbType(), QgsWkbTypes::MultiPoint );
  QVERIFY( dynamic_cast< QgsMultiPointV2 *>( collect.get() ) );
  collect = QgsGeometryFactory::createCollectionOfType( QgsWkbTypes::LineStringZ );
  QCOMPARE( collect->wkbType(), QgsWkbTypes::MultiLineStringZ );
  QVERIFY( dynamic_cast< QgsMultiLineString *>( collect.get() ) );
  collect = QgsGeometryFactory::createCollectionOfType( QgsWkbTypes::PolygonM );
  QCOMPARE( collect->wkbType(), QgsWkbTypes::MultiPolygonM );
  QVERIFY( dynamic_cast< QgsMultiPolygonV2 *>( collect.get() ) );
  collect = QgsGeometryFactory::createCollectionOfType( QgsWkbTypes::GeometryCollectionZ );
  QCOMPARE( collect->wkbType(), QgsWkbTypes::GeometryCollectionZ );
  QVERIFY( dynamic_cast< QgsGeometryCollection *>( collect.get() ) );
  collect = QgsGeometryFactory::createCollectionOfType( QgsWkbTypes::CurvePolygonM );
  QCOMPARE( collect->wkbType(), QgsWkbTypes::MultiSurfaceM );
  QVERIFY( dynamic_cast< QgsMultiSurface *>( collect.get() ) );
}

QGSTEST_MAIN( TestQgsGeometry )
#include "testqgsgeometry.moc"
