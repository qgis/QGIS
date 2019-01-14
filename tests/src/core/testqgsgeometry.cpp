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
#include "qgsgeometryengine.h"
#include "qgscircle.h"
#include "qgsellipse.h"
#include "qgsquadrilateral.h"
#include "qgsregularpolygon.h"
#include "qgsmultipoint.h"
#include "qgsmultilinestring.h"
#include "qgsmultipolygon.h"
#include "qgscircularstring.h"
#include "qgsgeometrycollection.h"
#include "qgsgeometryfactory.h"
#include "qgscurvepolygon.h"
#include "qgsproject.h"
#include "qgslinesegment.h"

//qgs unit test utility class
#include "qgsrenderchecker.h"

/**
 * \ingroup UnitTests
 * This is a unit test for the different geometry operations on vector features.
 */
class TestQgsGeometry : public QObject
{
    Q_OBJECT

  public:
    TestQgsGeometry();

    static QgsAbstractGeometry *createEmpty( QgsAbstractGeometry *geom )
    {
      return geom->createEmptyWithSameType();
    }

  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void init();// will be called before each testfunction is executed.
    void cleanup();// will be called after every testfunction.
    void copy();
    void assignment();
    void asVariant(); //test conversion to and from a QVariant
    void isEmpty();
    void equality();
    void vertexIterator();
    void partIterator();


    // geometry types
    void point(); //test QgsPointV2
    void lineString(); //test QgsLineString
    void circularString();
    void polygon(); //test QgsPolygon
    void curvePolygon();
    void triangle();
    void circle();
    void ellipse();
    void quadrilateral();
    void regularPolygon();
    void compoundCurve(); //test QgsCompoundCurve
    void multiPoint();
    void multiLineString();
    void multiCurve();
    void multiSurface();
    void multiPolygon();
    void geometryCollection();

    void fromQgsPointXY();
    void fromQPoint();
    void fromQPolygonF();
    void fromPolyline();
    void asQPointF();
    void asQPolygonF();

    void comparePolylines();
    void comparePolygons();

    void createEmptyWithSameType();

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

    void directionNeutralSegmentation();
    void poleOfInaccessibility();

    void makeValid();

    void isSimple();

    void reshapeGeometryLineMerge();
    void createCollectionOfType();

    void minimalEnclosingCircle( );
    void splitGeometry();
    void snappedToGrid();

    void convertGeometryCollectionToSubclass();

  private:
    //! A helper method to do a render check to see if the geometry op is as expected
    bool renderCheck( const QString &testName, const QString &comment = QString(), int mismatchCount = 0 );
    //! A helper method to dump to qdebug the geometry of a multipolygon
    void dumpMultiPolygon( QgsMultiPolygonXY &multiPolygon );
    //! A helper method to dump to qdebug the geometry of a polygon
    void dumpPolygon( QgsPolygonXY &polygon );
    //! A helper method to dump to qdebug the geometry of a polyline
    void dumpPolyline( QgsPolylineXY &polyline );

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
    QgsPolylineXY mPolylineA;
    QgsPolylineXY mPolylineB;
    QgsPolylineXY mPolylineC;
    QgsGeometry mpPolylineGeometryD;
    QgsPolygonXY mPolygonA;
    QgsPolygonXY mPolygonB;
    QgsPolygonXY mPolygonC;
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
  mpPolygonGeometryA = QgsGeometry::fromPolygonXY( mPolygonA );
  mpPolygonGeometryB = QgsGeometry::fromPolygonXY( mPolygonB );
  mpPolygonGeometryC = QgsGeometry::fromPolygonXY( mPolygonC );

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
  QCOMPARE( original.constGet()->vertexAt( QgsVertexId( 0, 0, 0 ) ).x(), 1.0 );
  QCOMPARE( original.constGet()->vertexAt( QgsVertexId( 0, 0, 0 ) ).y(), 2.0 );

  //implicitly shared copy
  QgsGeometry copy( original );
  QCOMPARE( copy.constGet()->vertexAt( QgsVertexId( 0, 0, 0 ) ).x(), 1.0 );
  QCOMPARE( copy.constGet()->vertexAt( QgsVertexId( 0, 0, 0 ) ).y(), 2.0 );

  //trigger a detach
  copy.set( new QgsPoint( 3.0, 4.0 ) );
  QCOMPARE( copy.constGet()->vertexAt( QgsVertexId( 0, 0, 0 ) ).x(), 3.0 );
  QCOMPARE( copy.constGet()->vertexAt( QgsVertexId( 0, 0, 0 ) ).y(), 4.0 );

  //make sure original was untouched
  QCOMPARE( original.constGet()->vertexAt( QgsVertexId( 0, 0, 0 ) ).x(), 1.0 );
  QCOMPARE( original.constGet()->vertexAt( QgsVertexId( 0, 0, 0 ) ).y(), 2.0 );
}

void TestQgsGeometry::assignment()
{
  //create a point geometry
  QgsGeometry original( new QgsPoint( 1.0, 2.0 ) );
  QCOMPARE( original.constGet()->vertexAt( QgsVertexId( 0, 0, 0 ) ).x(), 1.0 );
  QCOMPARE( original.constGet()->vertexAt( QgsVertexId( 0, 0, 0 ) ).y(), 2.0 );

  //assign to implicitly shared copy
  QgsGeometry copy;
  copy = original;
  QCOMPARE( copy.constGet()->vertexAt( QgsVertexId( 0, 0, 0 ) ).x(), 1.0 );
  QCOMPARE( copy.constGet()->vertexAt( QgsVertexId( 0, 0, 0 ) ).y(), 2.0 );

  //trigger a detach
  copy.set( new QgsPoint( 3.0, 4.0 ) );
  QCOMPARE( copy.constGet()->vertexAt( QgsVertexId( 0, 0, 0 ) ).x(), 3.0 );
  QCOMPARE( copy.constGet()->vertexAt( QgsVertexId( 0, 0, 0 ) ).y(), 4.0 );

  //make sure original was untouched
  QCOMPARE( original.constGet()->vertexAt( QgsVertexId( 0, 0, 0 ) ).x(), 1.0 );
  QCOMPARE( original.constGet()->vertexAt( QgsVertexId( 0, 0, 0 ) ).y(), 2.0 );
}

void TestQgsGeometry::asVariant()
{
  //create a point geometry
  QgsGeometry original( new QgsPoint( 1.0, 2.0 ) );
  QCOMPARE( original.constGet()->vertexAt( QgsVertexId( 0, 0, 0 ) ).x(), 1.0 );
  QCOMPARE( original.constGet()->vertexAt( QgsVertexId( 0, 0, 0 ) ).y(), 2.0 );

  //convert to and from a QVariant
  QVariant var = QVariant::fromValue( original );
  QVERIFY( var.isValid() );

  QgsGeometry fromVar = qvariant_cast<QgsGeometry>( var );
  QCOMPARE( fromVar.constGet()->vertexAt( QgsVertexId( 0, 0, 0 ) ).x(), 1.0 );
  QCOMPARE( fromVar.constGet()->vertexAt( QgsVertexId( 0, 0, 0 ) ).y(), 2.0 );

  //also check copying variant
  QVariant var2 = var;
  QVERIFY( var2.isValid() );
  QgsGeometry fromVar2 = qvariant_cast<QgsGeometry>( var2 );
  QCOMPARE( fromVar2.constGet()->vertexAt( QgsVertexId( 0, 0, 0 ) ).x(), 1.0 );
  QCOMPARE( fromVar2.constGet()->vertexAt( QgsVertexId( 0, 0, 0 ) ).y(), 2.0 );

  //modify original and check detachment
  original.set( new QgsPoint( 3.0, 4.0 ) );
  QgsGeometry fromVar3 = qvariant_cast<QgsGeometry>( var );
  QCOMPARE( fromVar3.constGet()->vertexAt( QgsVertexId( 0, 0, 0 ) ).x(), 1.0 );
  QCOMPARE( fromVar3.constGet()->vertexAt( QgsVertexId( 0, 0, 0 ) ).y(), 2.0 );
}

void TestQgsGeometry::isEmpty()
{
  QgsGeometry geom;
  QVERIFY( geom.isNull() );

  geom.set( new QgsPoint( 1.0, 2.0 ) );
  QVERIFY( !geom.isNull() );

  geom.set( nullptr );
  QVERIFY( geom.isNull() );

  QgsGeometryCollection collection;
  QVERIFY( collection.isEmpty() );
}

void TestQgsGeometry::equality()
{
  // null geometries
  QVERIFY( !QgsGeometry().equals( QgsGeometry() ) );

  // compare to null
  QgsGeometry g1( qgis::make_unique< QgsPoint >( 1.0, 2.0 ) );
  QVERIFY( !g1.equals( QgsGeometry() ) );
  QVERIFY( !QgsGeometry().equals( g1 ) );

  // compare implicitly shared copies
  QgsGeometry g2( g1 );
  QVERIFY( g2.equals( g1 ) );
  QVERIFY( g1.equals( g2 ) );
  QVERIFY( g1.equals( g1 ) );

  // equal geometry, but different internal data
  g2 = QgsGeometry::fromWkt( "Point( 1.0 2.0 )" );
  QVERIFY( g2.equals( g1 ) );
  QVERIFY( g1.equals( g2 ) );

  // different dimensionality
  g2 = QgsGeometry::fromWkt( "PointM( 1.0 2.0 3.0)" );
  QVERIFY( !g2.equals( g1 ) );
  QVERIFY( !g1.equals( g2 ) );

  // different type
  g2 = QgsGeometry::fromWkt( "LineString( 1.0 2.0, 3.0 4.0 )" );
  QVERIFY( !g2.equals( g1 ) );
  QVERIFY( !g1.equals( g2 ) );

  // different direction
  g1 = QgsGeometry::fromWkt( "LineString( 3.0 4.0, 1.0 2.0 )" );
  QVERIFY( !g2.equals( g1 ) );
  QVERIFY( !g1.equals( g2 ) );
}

void TestQgsGeometry::vertexIterator()
{
  QgsGeometry geom;
  QgsVertexIterator it = geom.vertices();
  QVERIFY( !it.hasNext() );

  QgsPolylineXY polyline;
  polyline << QgsPoint( 1, 2 ) << QgsPoint( 3, 4 );
  QgsGeometry geom2 = QgsGeometry::fromPolylineXY( polyline );
  QgsVertexIterator it2 = geom2.vertices();
  QVERIFY( it2.hasNext() );
  QCOMPARE( it2.next(), QgsPoint( 1, 2 ) );
  QVERIFY( it2.hasNext() );
  QCOMPARE( it2.next(), QgsPoint( 3, 4 ) );
  QVERIFY( !it2.hasNext() );
}

void TestQgsGeometry::partIterator()
{
  QgsGeometry geom;
  QgsGeometryPartIterator it = geom.parts();
  QVERIFY( !it.hasNext() );

  geom = QgsGeometry::fromWkt( QStringLiteral( "Point( 1 2 )" ) );
  QgsGeometryConstPartIterator it2 = geom.constParts();
  QVERIFY( it2.hasNext() );
  QCOMPARE( it2.next()->asWkt(), QStringLiteral( "Point (1 2)" ) );
  QVERIFY( !it2.hasNext() );

  // test that non-const iterator detaches
  QgsGeometry geom2 = geom;
  it = geom2.parts();
  QVERIFY( it.hasNext() );
  QgsAbstractGeometry *part = it.next();
  QCOMPARE( part->asWkt(), QStringLiteral( "Point (1 2)" ) );
  static_cast< QgsPoint * >( part )->setX( 100 );
  QCOMPARE( geom2.asWkt(), QStringLiteral( "Point (100 2)" ) );
  QVERIFY( !it.hasNext() );
  // geom2 should hve adetached, geom should be unaffected by change
  QCOMPARE( geom.asWkt(), QStringLiteral( "Point (1 2)" ) );

  // See test_qgsgeometry.py for geometry-type specific checks!
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

  QgsPoint noZ( QgsWkbTypes::PointM, 11.0, 13.0, 15.0, 17.0 );
  QCOMPARE( noZ.x(), 11.0 );
  QCOMPARE( noZ.y(), 13.0 );
  QVERIFY( std::isnan( noZ.z() ) );
  QCOMPARE( noZ.m(), 17.0 );
  QCOMPARE( noZ.wkbType(), QgsWkbTypes::PointM );

  QgsPoint noM( QgsWkbTypes::PointZ, 11.0, 13.0, 17.0, 18.0 );
  QCOMPARE( noM.x(), 11.0 );
  QCOMPARE( noM.y(), 13.0 );
  QVERIFY( std::isnan( noM.m() ) );
  QCOMPARE( noM.z(), 17.0 );
  QCOMPARE( noM.wkbType(), QgsWkbTypes::PointZ );

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

  QgsLineString nonPoint;
  QVERIFY( p8 != nonPoint );
  QVERIFY( !( p8 == nonPoint ) );

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

  //toCurveType
  clone.reset( p10.toCurveType() );
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
  QGSCOMPARENEAR( result.x(), 5.0, 4 * std::numeric_limits<double>::epsilon() );
  QGSCOMPARENEAR( result.y(), 9.0, 4 * std::numeric_limits<double>::epsilon() );

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
  QVERIFY( !p14.fromWkt( "Point(1 )" ) );

  //asGML2
  QgsPoint exportPoint( 1, 2 );
  QgsPoint exportPointFloat( 1 / 3.0, 2 / 3.0 );
  QDomDocument doc( QStringLiteral( "gml" ) );
  QString expectedGML2( QStringLiteral( "<Point xmlns=\"gml\"><coordinates xmlns=\"gml\" cs=\",\" ts=\" \">1,2</coordinates></Point>" ) );
  QGSCOMPAREGML( elemToString( exportPoint.asGml2( doc ) ), expectedGML2 );
  QString expectedGML2prec3( QStringLiteral( "<Point xmlns=\"gml\"><coordinates xmlns=\"gml\" cs=\",\" ts=\" \">0.333,0.667</coordinates></Point>" ) );
  QGSCOMPAREGML( elemToString( exportPointFloat.asGml2( doc, 3 ) ), expectedGML2prec3 );

  //asGML3
  QString expectedGML3( QStringLiteral( "<Point xmlns=\"gml\"><pos xmlns=\"gml\" srsDimension=\"2\">1 2</pos></Point>" ) );
  QCOMPARE( elemToString( exportPoint.asGml3( doc ) ), expectedGML3 );
  QString expectedGML3prec3( QStringLiteral( "<Point xmlns=\"gml\"><pos xmlns=\"gml\" srsDimension=\"2\">0.333 0.667</pos></Point>" ) );
  QCOMPARE( elemToString( exportPointFloat.asGml3( doc, 3 ) ), expectedGML3prec3 );
  QgsPoint exportPointZ( 1, 2, 3 );
  QString expectedGML3Z( QStringLiteral( "<Point xmlns=\"gml\"><pos xmlns=\"gml\" srsDimension=\"3\">1 2 3</pos></Point>" ) );
  QGSCOMPAREGML( elemToString( exportPointZ.asGml3( doc, 3 ) ), expectedGML3Z );

  //asGML2 inverted axis
  QgsPoint exportPointInvertedAxis( 1, 2 );
  QgsPoint exportPointFloatInvertedAxis( 1 / 3.0, 2 / 3.0 );
  QDomDocument docInvertedAxis( QStringLiteral( "gml" ) );
  QString expectedGML2InvertedAxis( QStringLiteral( "<Point xmlns=\"gml\"><coordinates xmlns=\"gml\" cs=\",\" ts=\" \">2,1</coordinates></Point>" ) );
  QGSCOMPAREGML( elemToString( exportPointInvertedAxis.asGml2( docInvertedAxis, 17, QStringLiteral( "gml" ), QgsAbstractGeometry::AxisOrder::YX ) ), expectedGML2InvertedAxis );
  QString expectedGML2prec3InvertedAxis( QStringLiteral( "<Point xmlns=\"gml\"><coordinates xmlns=\"gml\" cs=\",\" ts=\" \">0.667,0.333</coordinates></Point>" ) );
  QGSCOMPAREGML( elemToString( exportPointFloatInvertedAxis.asGml2( docInvertedAxis, 3, QStringLiteral( "gml" ), QgsAbstractGeometry::AxisOrder::YX ) ), expectedGML2prec3InvertedAxis );

  //asGML3 inverted axis
  QString expectedGML3InvertedAxis( QStringLiteral( "<Point xmlns=\"gml\"><pos xmlns=\"gml\" srsDimension=\"2\">2 1</pos></Point>" ) );
  QCOMPARE( elemToString( exportPointInvertedAxis.asGml3( docInvertedAxis, 17, QStringLiteral( "gml" ), QgsAbstractGeometry::AxisOrder::YX ) ), expectedGML3InvertedAxis );
  QString expectedGML3prec3InvertedAxis( QStringLiteral( "<Point xmlns=\"gml\"><pos xmlns=\"gml\" srsDimension=\"2\">0.667 0.333</pos></Point>" ) );
  QCOMPARE( elemToString( exportPointFloatInvertedAxis.asGml3( docInvertedAxis, 3, QStringLiteral( "gml" ), QgsAbstractGeometry::AxisOrder::YX ) ), expectedGML3prec3InvertedAxis );
  QgsPoint exportPointZInvertedAxis( 1, 2, 3 );
  QString expectedGML3ZInvertedAxis( QStringLiteral( "<Point xmlns=\"gml\"><pos xmlns=\"gml\" srsDimension=\"3\">2 1 3</pos></Point>" ) );
  QGSCOMPAREGML( elemToString( exportPointZInvertedAxis.asGml3( docInvertedAxis, 3, QStringLiteral( "gml" ), QgsAbstractGeometry::AxisOrder::YX ) ), expectedGML3ZInvertedAxis );


  //asJSON
  QString expectedJson( QStringLiteral( "{\"type\": \"Point\", \"coordinates\": [1, 2]}" ) );
  QCOMPARE( exportPoint.asJson(), expectedJson );
  QString expectedJsonPrec3( QStringLiteral( "{\"type\": \"Point\", \"coordinates\": [0.333, 0.667]}" ) );
  QCOMPARE( exportPointFloat.asJson( 3 ), expectedJsonPrec3 );

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
  QgsCoordinateTransform tr( sourceSrs, destSrs, QgsProject::instance() );
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
  p17.transform( QTransform::fromScale( 1, 1 ), 11, 2, 3, 4 );
  QVERIFY( p17 == QgsPoint( QgsWkbTypes::PointZM, 20, 60, 71, 163 ) );

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
  QVERIFY( p20.closestSegment( QgsPoint( 4.0, 6.0 ), closest, after ) < 0 );

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

  //adjacent vertices - both should be invalid
  QgsVertexId prev( 1, 2, 3 ); // start with something
  QgsVertexId next( 4, 5, 6 );
  p21.adjacentVertices( v, prev, next );
  QCOMPARE( prev, QgsVertexId() );
  QCOMPARE( next, QgsVertexId() );

  // vertex iterator
  QgsAbstractGeometry::vertex_iterator it1 = p21.vertices_begin();
  QgsAbstractGeometry::vertex_iterator it1end = p21.vertices_end();
  QCOMPARE( *it1, p21 );
  QCOMPARE( it1.vertexId(), QgsVertexId( 0, 0, 0 ) );
  ++it1;
  QCOMPARE( it1, it1end );

  // Java-style iterator
  QgsVertexIterator it2( &p21 );
  QVERIFY( it2.hasNext() );
  QCOMPARE( it2.next(), p21 );
  QVERIFY( !it2.hasNext() );

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
  QVERIFY( std::isnan( p29.z() ) );
  QCOMPARE( p29.m(), 9.0 );
  QVERIFY( p29.convertTo( QgsWkbTypes::Point ) );
  QCOMPARE( p29.wkbType(), QgsWkbTypes::Point );
  QVERIFY( std::isnan( p29.z() ) );
  QVERIFY( std::isnan( p29.m() ) );
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
  QVERIFY( std::isnan( QgsPoint( 0, 0 ).distanceSquared3D( 1, 1, 0 ) ) );
  QVERIFY( std::isnan( QgsPoint( 0, 0 ).distanceSquared3D( QgsPoint( QgsWkbTypes::PointZ, 2, 2, 2, 0 ) ) ) );
  QVERIFY( std::isnan( QgsPoint( 0, 0 ).distanceSquared3D( 2, 2, 2 ) ) );
  QVERIFY( std::isnan( QgsPoint( QgsWkbTypes::PointZ, 2, 2, 2, 0 ).distanceSquared3D( QgsPoint( 1, 1 ) ) ) );
  QCOMPARE( QgsPoint( QgsWkbTypes::PointZ, 2, 2, 2, 0 ).distanceSquared3D( 1, 1, 0 ), 6.0 );
  QVERIFY( std::isnan( QgsPoint( QgsWkbTypes::PointZ, -2, -2, -2, 0 ).distanceSquared3D( QgsPoint( 0, 0 ) ) ) );
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

  // vertex number
  QCOMPARE( QgsPoint( 1, 2 ).vertexNumberFromVertexId( QgsVertexId( 0, 0, -1 ) ), -1 );
  QCOMPARE( QgsPoint( 1, 2 ).vertexNumberFromVertexId( QgsVertexId( 0, 0, 1 ) ), -1 );
  QCOMPARE( QgsPoint( 1, 2 ).vertexNumberFromVertexId( QgsVertexId( 0, 0, 0 ) ), 0 );

  //segmentLength
  QCOMPARE( QgsPoint( 1, 2 ).segmentLength( QgsVertexId() ), 0.0 );
  QCOMPARE( QgsPoint( 1, 2 ).segmentLength( QgsVertexId( -1, 0, 0 ) ), 0.0 );
  QCOMPARE( QgsPoint( 1, 2 ).segmentLength( QgsVertexId( -1, 0, 1 ) ), 0.0 );
  QCOMPARE( QgsPoint( 1, 2 ).segmentLength( QgsVertexId( -1, 0, -1 ) ), 0.0 );
  QCOMPARE( QgsPoint( 1, 2 ).segmentLength( QgsVertexId( 0, 0, 0 ) ), 0.0 );

  // remove duplicate points
  QgsPoint p = QgsPoint( 1, 2 );
  QVERIFY( !p.removeDuplicateNodes() );
  QCOMPARE( p.x(), 1.0 );
  QCOMPARE( p.y(), 2.0 );

  // swap xy
  p = QgsPoint( 1.1, 2.2, 3.3, 4.4, QgsWkbTypes::PointZM );
  p.swapXy();
  QCOMPARE( p.x(), 2.2 );
  QCOMPARE( p.y(), 1.1 );
  QCOMPARE( p.z(), 3.3 );
  QCOMPARE( p.m(), 4.4 );
  QCOMPARE( p.wkbType(), QgsWkbTypes::PointZM );

  // filter vertex
  p = QgsPoint( 1.1, 2.2, 3.3, 4.4, QgsWkbTypes::PointZM );
  p.filterVertices( []( const QgsPoint & )-> bool { return false; } );
  QCOMPARE( p.x(), 1.1 );
  QCOMPARE( p.y(), 2.2 );
  QCOMPARE( p.z(), 3.3 );
  QCOMPARE( p.m(), 4.4 );
  QCOMPARE( p.wkbType(), QgsWkbTypes::PointZM );

  // transform vertex
  p = QgsPoint( 1.1, 2.2, 3.3, 4.4, QgsWkbTypes::PointZM );
  p.transformVertices( []( const QgsPoint & p )-> QgsPoint
  {
    return QgsPoint( p.x() + 2, p.y() + 3, p.z() + 1, p.m() + 8 );
  } );
  QCOMPARE( p.x(), 3.1 );
  QCOMPARE( p.y(), 5.2 );
  QCOMPARE( p.z(), 4.3 );
  QCOMPARE( p.m(), 12.4 );
  QCOMPARE( p.wkbType(), QgsWkbTypes::PointZM );
  // no dimensionality change allowed
  p.transformVertices( []( const QgsPoint & p )-> QgsPoint
  {
    return QgsPoint( p.x() + 2, p.y() + 3 );
  } );
  QCOMPARE( p.x(), 5.1 );
  QCOMPARE( p.y(), 8.2 );
  QVERIFY( std::isnan( p.z() ) );
  QVERIFY( std::isnan( p.m() ) );
  QCOMPARE( p.wkbType(), QgsWkbTypes::PointZM );
  p = QgsPoint( 2, 3 );
  p.transformVertices( []( const QgsPoint & p )-> QgsPoint
  {
    return QgsPoint( p.x() + 2, p.y() + 3, 7, 8 );
  } );
  QCOMPARE( p.x(), 4.0 );
  QCOMPARE( p.y(), 6.0 );
  QVERIFY( std::isnan( p.z() ) );
  QVERIFY( std::isnan( p.m() ) );
  QCOMPARE( p.wkbType(), QgsWkbTypes::Point );

}

void TestQgsGeometry::circularString()
{
  //test constructors
  QgsCircularString l1;
  QVERIFY( l1.isEmpty() );
  QCOMPARE( l1.numPoints(), 0 );
  QCOMPARE( l1.vertexCount(), 0 );
  QCOMPARE( l1.nCoordinates(), 0 );
  QCOMPARE( l1.ringCount(), 0 );
  QCOMPARE( l1.partCount(), 0 );
  QVERIFY( !l1.is3D() );
  QVERIFY( !l1.isMeasure() );
  QCOMPARE( l1.wkbType(), QgsWkbTypes::CircularString );
  QCOMPARE( l1.wktTypeStr(), QString( "CircularString" ) );
  QCOMPARE( l1.geometryType(), QString( "CircularString" ) );
  QCOMPARE( l1.dimension(), 1 );
  QVERIFY( l1.hasCurvedSegments() );
  QCOMPARE( l1.area(), 0.0 );
  QCOMPARE( l1.perimeter(), 0.0 );
  QgsPointSequence pts;
  l1.points( pts );
  QVERIFY( pts.empty() );

  // from 3 points
  QgsCircularString from3Pts( QgsPoint( 1, 2 ), QgsPoint( 21, 22 ), QgsPoint( 31, 2 ) );
  QCOMPARE( from3Pts.wkbType(), QgsWkbTypes::CircularString );
  QCOMPARE( from3Pts.numPoints(), 3 );
  QCOMPARE( from3Pts.xAt( 0 ), 1.0 );
  QCOMPARE( from3Pts.yAt( 0 ), 2.0 );
  QCOMPARE( from3Pts.xAt( 1 ), 21.0 );
  QCOMPARE( from3Pts.yAt( 1 ), 22.0 );
  QCOMPARE( from3Pts.xAt( 2 ), 31.0 );
  QCOMPARE( from3Pts.yAt( 2 ), 2.0 );
  from3Pts = QgsCircularString( QgsPoint( QgsWkbTypes::PointZ, 1, 2, 3 ), QgsPoint( QgsWkbTypes::PointZ, 21, 22, 23 ),
                                QgsPoint( QgsWkbTypes::PointZ, 31, 2, 33 ) );
  QCOMPARE( from3Pts.wkbType(), QgsWkbTypes::CircularStringZ );
  QCOMPARE( from3Pts.numPoints(), 3 );
  QCOMPARE( from3Pts.xAt( 0 ), 1.0 );
  QCOMPARE( from3Pts.yAt( 0 ), 2.0 );
  QCOMPARE( from3Pts.pointN( 0 ).z(), 3.0 );
  QCOMPARE( from3Pts.xAt( 1 ), 21.0 );
  QCOMPARE( from3Pts.yAt( 1 ), 22.0 );
  QCOMPARE( from3Pts.pointN( 1 ).z(), 23.0 );
  QCOMPARE( from3Pts.xAt( 2 ), 31.0 );
  QCOMPARE( from3Pts.yAt( 2 ), 2.0 );
  QCOMPARE( from3Pts.pointN( 2 ).z(), 33.0 );
  from3Pts = QgsCircularString( QgsPoint( QgsWkbTypes::PointM, 1, 2, 0, 3 ), QgsPoint( QgsWkbTypes::PointM, 21, 22, 0, 23 ),
                                QgsPoint( QgsWkbTypes::PointM, 31, 2, 0, 33 ) );
  QCOMPARE( from3Pts.wkbType(), QgsWkbTypes::CircularStringM );
  QCOMPARE( from3Pts.numPoints(), 3 );
  QCOMPARE( from3Pts.xAt( 0 ), 1.0 );
  QCOMPARE( from3Pts.yAt( 0 ), 2.0 );
  QCOMPARE( from3Pts.pointN( 0 ).m(), 3.0 );
  QCOMPARE( from3Pts.xAt( 1 ), 21.0 );
  QCOMPARE( from3Pts.yAt( 1 ), 22.0 );
  QCOMPARE( from3Pts.pointN( 1 ).m(), 23.0 );
  QCOMPARE( from3Pts.xAt( 2 ), 31.0 );
  QCOMPARE( from3Pts.yAt( 2 ), 2.0 );
  QCOMPARE( from3Pts.pointN( 2 ).m(), 33.0 );
  from3Pts = QgsCircularString( QgsPoint( QgsWkbTypes::PointZM, 1, 2, 3, 4 ), QgsPoint( QgsWkbTypes::PointZM, 21, 22, 23, 24 ),
                                QgsPoint( QgsWkbTypes::PointZM, 31, 2, 33, 34 ) );
  QCOMPARE( from3Pts.wkbType(), QgsWkbTypes::CircularStringZM );
  QCOMPARE( from3Pts.numPoints(), 3 );
  QCOMPARE( from3Pts.xAt( 0 ), 1.0 );
  QCOMPARE( from3Pts.yAt( 0 ), 2.0 );
  QCOMPARE( from3Pts.pointN( 0 ).z(), 3.0 );
  QCOMPARE( from3Pts.pointN( 0 ).m(), 4.0 );
  QCOMPARE( from3Pts.xAt( 1 ), 21.0 );
  QCOMPARE( from3Pts.yAt( 1 ), 22.0 );
  QCOMPARE( from3Pts.pointN( 1 ).z(), 23.0 );
  QCOMPARE( from3Pts.pointN( 1 ).m(), 24.0 );
  QCOMPARE( from3Pts.xAt( 2 ), 31.0 );
  QCOMPARE( from3Pts.yAt( 2 ), 2.0 );
  QCOMPARE( from3Pts.pointN( 2 ).z(), 33.0 );
  QCOMPARE( from3Pts.pointN( 2 ).m(), 34.0 );

  // from 2 points and center
  from3Pts = QgsCircularString::fromTwoPointsAndCenter( QgsPoint( 1, 2 ), QgsPoint( 31, 2 ), QgsPoint( 21, 2 ) );
  QCOMPARE( from3Pts.wkbType(), QgsWkbTypes::CircularString );
  QCOMPARE( from3Pts.numPoints(), 3 );
  QCOMPARE( from3Pts.xAt( 0 ), 1.0 );
  QCOMPARE( from3Pts.yAt( 0 ), 2.0 );
  QCOMPARE( from3Pts.xAt( 1 ), 21.0 );
  QCOMPARE( from3Pts.yAt( 1 ), 22.0 );
  QCOMPARE( from3Pts.xAt( 2 ), 31.0 );
  QCOMPARE( from3Pts.yAt( 2 ), 2.0 );
  from3Pts = QgsCircularString::fromTwoPointsAndCenter( QgsPoint( 1, 2 ), QgsPoint( 31, 2 ), QgsPoint( 21, 2 ), false );
  QCOMPARE( from3Pts.wkbType(), QgsWkbTypes::CircularString );
  QCOMPARE( from3Pts.numPoints(), 3 );
  QCOMPARE( from3Pts.xAt( 0 ), 1.0 );
  QCOMPARE( from3Pts.yAt( 0 ), 2.0 );
  QCOMPARE( from3Pts.xAt( 1 ), 21.0 );
  QCOMPARE( from3Pts.yAt( 1 ), -18.0 );
  QCOMPARE( from3Pts.xAt( 2 ), 31.0 );
  QCOMPARE( from3Pts.yAt( 2 ), 2.0 );
  from3Pts = QgsCircularString::fromTwoPointsAndCenter( QgsPoint( QgsWkbTypes::PointZ, 1, 2, 3 ), QgsPoint( QgsWkbTypes::PointZ, 32, 2, 33 ),
             QgsPoint( QgsWkbTypes::PointZ, 21, 2, 23 ) );
  QCOMPARE( from3Pts.wkbType(), QgsWkbTypes::CircularStringZ );
  QCOMPARE( from3Pts.numPoints(), 3 );
  QCOMPARE( from3Pts.xAt( 0 ), 1.0 );
  QCOMPARE( from3Pts.yAt( 0 ), 2.0 );
  QCOMPARE( from3Pts.pointN( 0 ).z(), 3.0 );
  QCOMPARE( from3Pts.xAt( 1 ), 21.0 );
  QCOMPARE( from3Pts.yAt( 1 ), 22.0 );
  QCOMPARE( from3Pts.pointN( 1 ).z(), 23.0 );
  QCOMPARE( from3Pts.xAt( 2 ), 32.0 );
  QCOMPARE( from3Pts.yAt( 2 ), 2.0 );
  QCOMPARE( from3Pts.pointN( 2 ).z(), 33.0 );
  from3Pts = QgsCircularString::fromTwoPointsAndCenter( QgsPoint( QgsWkbTypes::PointM, 1, 2, 0, 3 ), QgsPoint( QgsWkbTypes::PointM, 31, 2, 0, 33 ),
             QgsPoint( QgsWkbTypes::PointM, 21, 2, 0, 23 ) );
  QCOMPARE( from3Pts.wkbType(), QgsWkbTypes::CircularStringM );
  QCOMPARE( from3Pts.numPoints(), 3 );
  QCOMPARE( from3Pts.xAt( 0 ), 1.0 );
  QCOMPARE( from3Pts.yAt( 0 ), 2.0 );
  QCOMPARE( from3Pts.pointN( 0 ).m(), 3.0 );
  QCOMPARE( from3Pts.xAt( 1 ), 21.0 );
  QCOMPARE( from3Pts.yAt( 1 ), 22.0 );
  QCOMPARE( from3Pts.pointN( 1 ).m(), 23.0 );
  QCOMPARE( from3Pts.xAt( 2 ), 31.0 );
  QCOMPARE( from3Pts.yAt( 2 ), 2.0 );
  QCOMPARE( from3Pts.pointN( 2 ).m(), 33.0 );
  from3Pts = QgsCircularString::fromTwoPointsAndCenter( QgsPoint( QgsWkbTypes::PointZM, 1, 2, 3, 4 ), QgsPoint( QgsWkbTypes::PointZM, 31, 2, 33, 34 ),
             QgsPoint( QgsWkbTypes::PointZM, 21, 2, 23, 24 ) );
  QCOMPARE( from3Pts.wkbType(), QgsWkbTypes::CircularStringZM );
  QCOMPARE( from3Pts.numPoints(), 3 );
  QCOMPARE( from3Pts.xAt( 0 ), 1.0 );
  QCOMPARE( from3Pts.yAt( 0 ), 2.0 );
  QCOMPARE( from3Pts.pointN( 0 ).z(), 3.0 );
  QCOMPARE( from3Pts.pointN( 0 ).m(), 4.0 );
  QCOMPARE( from3Pts.xAt( 1 ), 21.0 );
  QCOMPARE( from3Pts.yAt( 1 ), 22.0 );
  QCOMPARE( from3Pts.pointN( 1 ).z(), 23.0 );
  QCOMPARE( from3Pts.pointN( 1 ).m(), 24.0 );
  QCOMPARE( from3Pts.xAt( 2 ), 31.0 );
  QCOMPARE( from3Pts.yAt( 2 ), 2.0 );
  QCOMPARE( from3Pts.pointN( 2 ).z(), 33.0 );
  QCOMPARE( from3Pts.pointN( 2 ).m(), 34.0 );

  //setPoints
  QgsCircularString l2;
  l2.setPoints( QgsPointSequence() << QgsPoint( 1.0, 2.0 ) );
  QVERIFY( !l2.isEmpty() );
  QCOMPARE( l2.numPoints(), 1 );
  QCOMPARE( l2.vertexCount(), 1 );
  QCOMPARE( l2.nCoordinates(), 1 );
  QCOMPARE( l2.ringCount(), 1 );
  QCOMPARE( l2.partCount(), 1 );
  QVERIFY( !l2.is3D() );
  QVERIFY( !l2.isMeasure() );
  QCOMPARE( l2.wkbType(), QgsWkbTypes::CircularString );
  QVERIFY( l2.hasCurvedSegments() );
  QCOMPARE( l2.area(), 0.0 );
  QCOMPARE( l2.perimeter(), 0.0 );
  l2.points( pts );
  QCOMPARE( pts, QgsPointSequence() << QgsPoint( 1.0, 2.0 ) );

  //setting first vertex should set linestring z/m type
  QgsCircularString l3;
  l3.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZ, 1.0, 2.0, 3.0 ) );
  QVERIFY( !l3.isEmpty() );
  QVERIFY( l3.is3D() );
  QVERIFY( !l3.isMeasure() );
  QCOMPARE( l3.wkbType(), QgsWkbTypes::CircularStringZ );
  QCOMPARE( l3.wktTypeStr(), QString( "CircularStringZ" ) );
  l3.points( pts );
  QCOMPARE( pts, QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZ, 1.0, 2.0, 3.0 ) );

  QgsCircularString l4;
  l4.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointM, 1.0, 2.0, 0.0, 3.0 ) );
  QVERIFY( !l4.isEmpty() );
  QVERIFY( !l4.is3D() );
  QVERIFY( l4.isMeasure() );
  QCOMPARE( l4.wkbType(), QgsWkbTypes::CircularStringM );
  QCOMPARE( l4.wktTypeStr(), QString( "CircularStringM" ) );
  l4.points( pts );
  QCOMPARE( pts, QgsPointSequence() << QgsPoint( QgsWkbTypes::PointM, 1.0, 2.0, 0.0, 3.0 ) );

  QgsCircularString l5;
  l5.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 1.0, 2.0, 3.0, 4.0 ) );
  QVERIFY( !l5.isEmpty() );
  QVERIFY( l5.is3D() );
  QVERIFY( l5.isMeasure() );
  QCOMPARE( l5.wkbType(), QgsWkbTypes::CircularStringZM );
  QCOMPARE( l5.wktTypeStr(), QString( "CircularStringZM" ) );
  l5.points( pts );
  QCOMPARE( pts, QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 1.0, 2.0, 3.0, 4.0 ) );

  //clear
  l5.clear();
  QVERIFY( l5.isEmpty() );
  QCOMPARE( l5.numPoints(), 0 );
  QCOMPARE( l5.vertexCount(), 0 );
  QCOMPARE( l5.nCoordinates(), 0 );
  QCOMPARE( l5.ringCount(), 0 );
  QCOMPARE( l5.partCount(), 0 );
  QVERIFY( !l5.is3D() );
  QVERIFY( !l5.isMeasure() );
  QCOMPARE( l5.wkbType(), QgsWkbTypes::CircularString );

  //setPoints
  QgsCircularString l8;
  l8.setPoints( QgsPointSequence() << QgsPoint( 1, 2 ) << QgsPoint( 2, 3 ) << QgsPoint( 3, 4 ) );
  QVERIFY( !l8.isEmpty() );
  QCOMPARE( l8.numPoints(), 3 );
  QCOMPARE( l8.vertexCount(), 3 );
  QCOMPARE( l8.nCoordinates(), 3 );
  QCOMPARE( l8.ringCount(), 1 );
  QCOMPARE( l8.partCount(), 1 );
  QVERIFY( !l8.is3D() );
  QVERIFY( !l8.isMeasure() );
  QCOMPARE( l8.wkbType(), QgsWkbTypes::CircularString );
  QVERIFY( l8.hasCurvedSegments() );
  l8.points( pts );
  QCOMPARE( pts, QgsPointSequence() << QgsPoint( 1, 2 ) << QgsPoint( 2, 3 ) << QgsPoint( 3, 4 ) );

  //setPoints with empty list, should clear linestring
  l8.setPoints( QgsPointSequence() );
  QVERIFY( l8.isEmpty() );
  QCOMPARE( l8.numPoints(), 0 );
  QCOMPARE( l8.vertexCount(), 0 );
  QCOMPARE( l8.nCoordinates(), 0 );
  QCOMPARE( l8.ringCount(), 0 );
  QCOMPARE( l8.partCount(), 0 );
  QCOMPARE( l8.wkbType(), QgsWkbTypes::CircularString );
  l8.points( pts );
  QVERIFY( pts.empty() );

  //setPoints with z
  l8.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZ, 1, 2, 3 ) << QgsPoint( QgsWkbTypes::PointZ, 2, 3, 4 ) );
  QCOMPARE( l8.numPoints(), 2 );
  QVERIFY( l8.is3D() );
  QVERIFY( !l8.isMeasure() );
  QCOMPARE( l8.wkbType(), QgsWkbTypes::CircularStringZ );
  l8.points( pts );
  QCOMPARE( pts, QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZ, 1, 2, 3 ) << QgsPoint( QgsWkbTypes::PointZ, 2, 3, 4 ) );

  //setPoints with m
  l8.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointM, 1, 2, 0, 3 ) << QgsPoint( QgsWkbTypes::PointM, 2, 3, 0, 4 ) );
  QCOMPARE( l8.numPoints(), 2 );
  QVERIFY( !l8.is3D() );
  QVERIFY( l8.isMeasure() );
  QCOMPARE( l8.wkbType(), QgsWkbTypes::CircularStringM );
  l8.points( pts );
  QCOMPARE( pts, QgsPointSequence() << QgsPoint( QgsWkbTypes::PointM, 1, 2, 0, 3 ) << QgsPoint( QgsWkbTypes::PointM, 2, 3, 0, 4 ) );

  //setPoints with zm
  l8.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 1, 2, 4, 5 ) << QgsPoint( QgsWkbTypes::PointZM, 2, 3, 4, 5 ) );
  QCOMPARE( l8.numPoints(), 2 );
  QVERIFY( l8.is3D() );
  QVERIFY( l8.isMeasure() );
  QCOMPARE( l8.wkbType(), QgsWkbTypes::CircularStringZM );
  l8.points( pts );
  QCOMPARE( pts, QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 1, 2, 4, 5 ) << QgsPoint( QgsWkbTypes::PointZM, 2, 3, 4, 5 ) );

  //setPoints with MIXED dimensionality of points
  l8.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 1, 2, 4, 5 ) << QgsPoint( QgsWkbTypes::PointM, 2, 3, 0, 5 ) );
  QCOMPARE( l8.numPoints(), 2 );
  QVERIFY( l8.is3D() );
  QVERIFY( l8.isMeasure() );
  QCOMPARE( l8.wkbType(), QgsWkbTypes::CircularStringZM );
  l8.points( pts );
  QCOMPARE( pts, QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 1, 2, 4, 5 ) << QgsPoint( QgsWkbTypes::PointZM, 2, 3, 0, 5 ) );

  //test point
  QCOMPARE( l8.pointN( 0 ), QgsPoint( QgsWkbTypes::PointZM, 1, 2, 4, 5 ) );
  QCOMPARE( l8.pointN( 1 ), QgsPoint( QgsWkbTypes::PointZM, 2, 3, 0, 5 ) );

  //out of range - just want no crash here
  QgsPoint bad = l8.pointN( -1 );
  bad = l8.pointN( 100 );

  //test getters/setters
  QgsCircularString l9;
  l9.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 1, 2, 3, 4 )
                << QgsPoint( QgsWkbTypes::PointZM, 11, 12, 13, 14 )
                << QgsPoint( QgsWkbTypes::PointZM, 21, 22, 23, 24 ) );
  QCOMPARE( l9.xAt( 0 ), 1.0 );
  QCOMPARE( l9.xAt( 1 ), 11.0 );
  QCOMPARE( l9.xAt( 2 ), 21.0 );
  QCOMPARE( l9.xAt( -1 ), 0.0 ); //out of range
  QCOMPARE( l9.xAt( 11 ), 0.0 ); //out of range

  l9.moveVertex( QgsVertexId( 0, 0, 0 ), QgsPoint( 51.0, 2 ) );
  QCOMPARE( l9.xAt( 0 ), 51.0 );
  l9.moveVertex( QgsVertexId( 0, 0, 1 ), QgsPoint( 61.0, 12 ) );
  QCOMPARE( l9.xAt( 1 ), 61.0 );
  l9.moveVertex( QgsVertexId( 0, 0, -1 ), QgsPoint( 71.0, 2 ) ); //out of range
  l9.moveVertex( QgsVertexId( 0, 0, 11 ), QgsPoint( 71.0, 2 ) ); //out of range

  QCOMPARE( l9.yAt( 0 ), 2.0 );
  QCOMPARE( l9.yAt( 1 ), 12.0 );
  QCOMPARE( l9.yAt( 2 ), 22.0 );
  QCOMPARE( l9.yAt( -1 ), 0.0 ); //out of range
  QCOMPARE( l9.yAt( 11 ), 0.0 ); //out of range

  l9.moveVertex( QgsVertexId( 0, 0, 0 ), QgsPoint( 51.0, 52 ) );
  QCOMPARE( l9.yAt( 0 ), 52.0 );
  l9.moveVertex( QgsVertexId( 0, 0, 1 ), QgsPoint( 61.0, 62 ) );
  QCOMPARE( l9.yAt( 1 ), 62.0 );

  QCOMPARE( l9.pointN( 0 ).z(), 3.0 );
  QCOMPARE( l9.pointN( 1 ).z(), 13.0 );
  QCOMPARE( l9.pointN( 2 ).z(), 23.0 );

  l9.moveVertex( QgsVertexId( 0, 0, 0 ), QgsPoint( 71.0, 2, 53 ) );
  QCOMPARE( l9.pointN( 0 ).z(), 53.0 );
  l9.moveVertex( QgsVertexId( 0, 0, 1 ), QgsPoint( 71.0, 2, 63 ) );
  QCOMPARE( l9.pointN( 1 ).z(), 63.0 );

  QCOMPARE( l9.pointN( 0 ).m(), 4.0 );
  QCOMPARE( l9.pointN( 1 ).m(), 14.0 );
  QCOMPARE( l9.pointN( 2 ).m(), 24.0 );

  l9.moveVertex( QgsVertexId( 0, 0, 0 ), QgsPoint( 71.0, 2, 53, 54 ) );
  QCOMPARE( l9.pointN( 0 ).m(), 54.0 );
  l9.moveVertex( QgsVertexId( 0, 0, 1 ), QgsPoint( 71.0, 2, 53, 64 ) );
  QCOMPARE( l9.pointN( 1 ).m(), 64.0 );

  //check zAt/setZAt with non-3d linestring
  l9.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointM, 1, 2, 0, 4 )
                << QgsPoint( QgsWkbTypes::PointM, 11, 12, 0, 14 )
                << QgsPoint( QgsWkbTypes::PointM, 21, 22, 0, 24 ) );

  //basically we just don't want these to crash
  QVERIFY( std::isnan( l9.pointN( 0 ).z() ) );
  QVERIFY( std::isnan( l9.pointN( 1 ).z() ) );
  l9.moveVertex( QgsVertexId( 0, 0, 0 ), QgsPoint( 71.0, 2, 53 ) );
  l9.moveVertex( QgsVertexId( 0, 0, 1 ), QgsPoint( 71.0, 2, 63 ) );

  //check mAt/setMAt with non-measure linestring
  l9.setPoints( QgsPointSequence() << QgsPoint( 1, 2 )
                << QgsPoint( 11, 12 )
                << QgsPoint( 21, 22 ) );

  //basically we just don't want these to crash
  QVERIFY( std::isnan( l9.pointN( 0 ).m() ) );
  QVERIFY( std::isnan( l9.pointN( 1 ).m() ) );
  l9.moveVertex( QgsVertexId( 0, 0, 0 ), QgsPoint( 71.0, 2, 0, 53 ) );
  l9.moveVertex( QgsVertexId( 0, 0, 1 ), QgsPoint( 71.0, 2, 0, 63 ) );

  //equality
  QgsCircularString e1;
  QgsCircularString e2;
  QVERIFY( e1 == e2 );
  QVERIFY( !( e1 != e2 ) );
  e1.setPoints( QgsPointSequence() << QgsPoint( 1, 2 ) );
  QVERIFY( !( e1 == e2 ) ); //different number of vertices
  QVERIFY( e1 != e2 );
  e2.setPoints( QgsPointSequence() << QgsPoint( 1, 2 ) );
  QVERIFY( e1 == e2 );
  QVERIFY( !( e1 != e2 ) );
  e1.setPoints( QgsPointSequence() << QgsPoint( 1, 2 ) << QgsPoint( 1 / 3.0, 4 / 3.0 ) );
  e2.setPoints( QgsPointSequence() << QgsPoint( 1, 2 ) << QgsPoint( 2 / 6.0, 8 / 6.0 ) );
  QVERIFY( e1 == e2 ); //check non-integer equality
  QVERIFY( !( e1 != e2 ) );
  e1.setPoints( QgsPointSequence() << QgsPoint( 1, 2 ) << QgsPoint( 1 / 3.0, 4 / 3.0 ) << QgsPoint( 7, 8 ) );
  e2.setPoints( QgsPointSequence() << QgsPoint( 1, 2 ) << QgsPoint( 2 / 6.0, 8 / 6.0 ) << QgsPoint( 6, 9 ) );
  QVERIFY( !( e1 == e2 ) ); //different coordinates
  QVERIFY( e1 != e2 );
  QgsCircularString e3;
  e3.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZ, 1, 2, 0 )
                << QgsPoint( QgsWkbTypes::PointZ, 1 / 3.0, 4 / 3.0, 0 )
                << QgsPoint( QgsWkbTypes::PointZ, 7, 8, 0 ) );
  QVERIFY( !( e1 == e3 ) ); //different dimension
  QVERIFY( e1 != e3 );
  QgsCircularString e4;
  e4.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZ, 1, 2, 2 )
                << QgsPoint( QgsWkbTypes::PointZ, 1 / 3.0, 4 / 3.0, 3 )
                << QgsPoint( QgsWkbTypes::PointZ, 7, 8, 4 ) );
  QVERIFY( !( e3 == e4 ) ); //different z coordinates
  QVERIFY( e3 != e4 );
  QgsCircularString e5;
  e5.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointM, 1, 2, 0, 1 )
                << QgsPoint( QgsWkbTypes::PointM, 1 / 3.0, 4 / 3.0, 0, 2 )
                << QgsPoint( QgsWkbTypes::PointM, 7, 8, 0, 3 ) );
  QgsCircularString e6;
  e6.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointM, 1, 2, 0, 11 )
                << QgsPoint( QgsWkbTypes::PointM, 1 / 3.0, 4 / 3.0, 0, 12 )
                << QgsPoint( QgsWkbTypes::PointM, 7, 8, 0, 13 ) );
  QVERIFY( !( e5 == e6 ) ); //different m values
  QVERIFY( e5 != e6 );

  QVERIFY( e6 != QgsLineString() );

  //isClosed
  QgsCircularString l11;
  QVERIFY( !l11.isClosed() );
  l11.setPoints( QgsPointSequence() << QgsPoint( 1, 2 )
                 << QgsPoint( 11, 2 )
                 << QgsPoint( 11, 22 )
                 << QgsPoint( 1, 22 ) );
  QVERIFY( !l11.isClosed() );
  QCOMPARE( l11.numPoints(), 4 );
  QCOMPARE( l11.area(), 0.0 );
  QCOMPARE( l11.perimeter(), 0.0 );

  //test that m values aren't considered when testing for closedness
  l11.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointM, 1, 2, 0, 3 )
                 << QgsPoint( QgsWkbTypes::PointM, 11, 2, 0, 4 )
                 << QgsPoint( QgsWkbTypes::PointM, 11, 22, 0, 5 )
                 << QgsPoint( QgsWkbTypes::PointM, 1, 2, 0, 6 ) );
  QVERIFY( l11.isClosed() );

  //polygonf
  QgsCircularString l13;
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

  // clone tests
  QgsCircularString l14;
  l14.setPoints( QgsPointSequence() << QgsPoint( 1, 2 )
                 << QgsPoint( 11, 2 )
                 << QgsPoint( 11, 22 )
                 << QgsPoint( 1, 22 ) );
  std::unique_ptr<QgsCircularString> cloned( l14.clone() );
  QCOMPARE( cloned->numPoints(), 4 );
  QCOMPARE( cloned->vertexCount(), 4 );
  QCOMPARE( cloned->ringCount(), 1 );
  QCOMPARE( cloned->partCount(), 1 );
  QCOMPARE( cloned->wkbType(), QgsWkbTypes::CircularString );
  QVERIFY( !cloned->is3D() );
  QVERIFY( !cloned->isMeasure() );
  QCOMPARE( cloned->pointN( 0 ), l14.pointN( 0 ) );
  QCOMPARE( cloned->pointN( 1 ), l14.pointN( 1 ) );
  QCOMPARE( cloned->pointN( 2 ), l14.pointN( 2 ) );
  QCOMPARE( cloned->pointN( 3 ), l14.pointN( 3 ) );

  //clone with Z/M
  l14.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 1, 2, 3, 4 )
                 << QgsPoint( QgsWkbTypes::PointZM, 11, 2, 11, 14 )
                 << QgsPoint( QgsWkbTypes::PointZM, 11, 22, 21, 24 )
                 << QgsPoint( QgsWkbTypes::PointZM, 1, 22, 31, 34 ) );
  cloned.reset( l14.clone() );
  QCOMPARE( cloned->numPoints(), 4 );
  QCOMPARE( cloned->wkbType(), QgsWkbTypes::CircularStringZM );
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
  QCOMPARE( cloned->wkbType(), QgsWkbTypes::CircularString );

  //segmentize tests
  QgsCircularString toSegment;
  toSegment.setPoints( QgsPointSequence() << QgsPoint( 1, 2 )
                       << QgsPoint( 11, 10 ) << QgsPoint( 21, 2 ) );
  std::unique_ptr<QgsLineString> segmentized( static_cast< QgsLineString * >( toSegment.segmentize() ) );
  QCOMPARE( segmentized->numPoints(), 156 );
  QCOMPARE( segmentized->vertexCount(), 156 );
  QCOMPARE( segmentized->ringCount(), 1 );
  QCOMPARE( segmentized->partCount(), 1 );
  QCOMPARE( segmentized->wkbType(), QgsWkbTypes::LineString );
  QVERIFY( !segmentized->is3D() );
  QVERIFY( !segmentized->isMeasure() );
  QCOMPARE( segmentized->pointN( 0 ), toSegment.pointN( 0 ) );
  QCOMPARE( segmentized->pointN( segmentized->numPoints() - 1 ), toSegment.pointN( toSegment.numPoints() - 1 ) );

  //segmentize with Z/M
  toSegment.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 1, 2, 3, 4 )
                       << QgsPoint( QgsWkbTypes::PointZM, 11, 10, 11, 14 )
                       << QgsPoint( QgsWkbTypes::PointZM, 21, 2, 21, 24 ) );
  segmentized.reset( static_cast< QgsLineString * >( toSegment.segmentize() ) );
  QCOMPARE( segmentized->numPoints(), 156 );
  QCOMPARE( segmentized->vertexCount(), 156 );
  QCOMPARE( segmentized->ringCount(), 1 );
  QCOMPARE( segmentized->partCount(), 1 );
  QCOMPARE( segmentized->wkbType(), QgsWkbTypes::LineStringZM );
  QVERIFY( segmentized->is3D() );
  QVERIFY( segmentized->isMeasure() );
  QCOMPARE( segmentized->pointN( 0 ), toSegment.pointN( 0 ) );
  QCOMPARE( segmentized->pointN( segmentized->numPoints() - 1 ), toSegment.pointN( toSegment.numPoints() - 1 ) );

  //segmentize an empty line
  toSegment.clear();
  segmentized.reset( static_cast< QgsLineString * >( toSegment.segmentize() ) );
  QVERIFY( segmentized->isEmpty() );
  QCOMPARE( segmentized->numPoints(), 0 );
  QVERIFY( !segmentized->is3D() );
  QVERIFY( !segmentized->isMeasure() );
  QCOMPARE( segmentized->wkbType(), QgsWkbTypes::LineString );


  //to/from WKB
  QgsCircularString l15;
  l15.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 1, 2, 3, 4 )
                 << QgsPoint( QgsWkbTypes::PointZM, 11, 2, 11, 14 )
                 << QgsPoint( QgsWkbTypes::PointZM, 11, 22, 21, 24 )
                 << QgsPoint( QgsWkbTypes::PointZM, 1, 22, 31, 34 ) );
  QByteArray wkb15 = l15.asWkb();
  QgsCircularString l16;
  QgsConstWkbPtr wkb15ptr( wkb15 );
  l16.fromWkb( wkb15ptr );
  QCOMPARE( l16.numPoints(), 4 );
  QCOMPARE( l16.vertexCount(), 4 );
  QCOMPARE( l16.nCoordinates(), 4 );
  QCOMPARE( l16.ringCount(), 1 );
  QCOMPARE( l16.partCount(), 1 );
  QCOMPARE( l16.wkbType(), QgsWkbTypes::CircularStringZM );
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
  QCOMPARE( l16.wkbType(), QgsWkbTypes::CircularString );
  QgsPoint point( 1, 2 );
  QByteArray wkb16 = point.asWkb();
  QgsConstWkbPtr wkb16ptr( wkb16 );
  QVERIFY( !l16.fromWkb( wkb16ptr ) );
  QCOMPARE( l16.wkbType(), QgsWkbTypes::CircularString );

  //to/from WKT
  QgsCircularString l17;
  l17.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 1, 2, 3, 4 )
                 << QgsPoint( QgsWkbTypes::PointZM, 11, 2, 11, 14 )
                 << QgsPoint( QgsWkbTypes::PointZM, 11, 22, 21, 24 )
                 << QgsPoint( QgsWkbTypes::PointZM, 1, 22, 31, 34 ) );

  QString wkt = l17.asWkt();
  QVERIFY( !wkt.isEmpty() );
  QgsCircularString l18;
  QVERIFY( l18.fromWkt( wkt ) );
  QCOMPARE( l18.numPoints(), 4 );
  QCOMPARE( l18.wkbType(), QgsWkbTypes::CircularStringZM );
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
  QCOMPARE( l18.wkbType(), QgsWkbTypes::CircularString );

  //asGML2
  QgsCircularString exportLine;
  exportLine.setPoints( QgsPointSequence() << QgsPoint( 31, 32 )
                        << QgsPoint( 41, 42 )
                        << QgsPoint( 51, 52 ) );
  QgsCircularString exportLineFloat;
  exportLineFloat.setPoints( QgsPointSequence() << QgsPoint( 1 / 3.0, 2 / 3.0 )
                             << QgsPoint( 1 + 1 / 3.0, 1 + 2 / 3.0 )
                             << QgsPoint( 2 + 1 / 3.0, 2 + 2 / 3.0 ) );
  QDomDocument doc( QStringLiteral( "gml" ) );
  QString expectedGML2( QStringLiteral( "<LineString xmlns=\"gml\"><coordinates xmlns=\"gml\" cs=\",\" ts=\" \">31,32 41,42 51,52</coordinates></LineString>" ) );
  QGSCOMPAREGML( elemToString( exportLine.asGml2( doc ) ), expectedGML2 );
  QString expectedGML2prec3( QStringLiteral( "<LineString xmlns=\"gml\"><coordinates xmlns=\"gml\" cs=\",\" ts=\" \">0.333,0.667 1.333,1.667 2.333,2.667</coordinates></LineString>" ) );
  QGSCOMPAREGML( elemToString( exportLineFloat.asGml2( doc, 3 ) ), expectedGML2prec3 );
  QString expectedGML2empty( QStringLiteral( "<LineString xmlns=\"gml\"/>" ) );
  QGSCOMPAREGML( elemToString( QgsCircularString().asGml2( doc ) ), expectedGML2empty );

  //asGML3
  QString expectedGML3( QStringLiteral( "<Curve xmlns=\"gml\"><segments xmlns=\"gml\"><ArcString xmlns=\"gml\"><posList xmlns=\"gml\" srsDimension=\"2\">31 32 41 42 51 52</posList></ArcString></segments></Curve>" ) );
  QCOMPARE( elemToString( exportLine.asGml3( doc ) ), expectedGML3 );
  QString expectedGML3prec3( QStringLiteral( "<Curve xmlns=\"gml\"><segments xmlns=\"gml\"><ArcString xmlns=\"gml\"><posList xmlns=\"gml\" srsDimension=\"2\">0.333 0.667 1.333 1.667 2.333 2.667</posList></ArcString></segments></Curve>" ) );
  QCOMPARE( elemToString( exportLineFloat.asGml3( doc, 3 ) ), expectedGML3prec3 );
  QString expectedGML3empty( QStringLiteral( "<Curve xmlns=\"gml\"/>" ) );
  QGSCOMPAREGML( elemToString( QgsCircularString().asGml3( doc ) ), expectedGML3empty );

  //asJSON
  QString expectedJson( QStringLiteral( "{\"type\": \"LineString\", \"coordinates\": [ [31, 32], [41, 42], [51, 52]]}" ) );
  QCOMPARE( exportLine.asJson(), expectedJson );
  QString expectedJsonPrec3( QStringLiteral( "{\"type\": \"LineString\", \"coordinates\": [ [0.333, 0.667], [1.333, 1.667], [2.333, 2.667]]}" ) );
  QCOMPARE( exportLineFloat.asJson( 3 ), expectedJsonPrec3 );

  //length
  QgsCircularString l19;
  QCOMPARE( l19.length(), 0.0 );
  l19.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 1, 1, 2, 3 )
                 << QgsPoint( QgsWkbTypes::PointZM, 1, 10, 4, 5 )
                 << QgsPoint( QgsWkbTypes::PointZM, 15, 10, 6, 7 ) );
  QGSCOMPARENEAR( l19.length(), 26.1433, 0.001 );

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
  QCOMPARE( segmentized->numPoints(), 181 );
  QCOMPARE( segmentized->wkbType(), QgsWkbTypes::LineStringZM );
  QVERIFY( segmentized->is3D() );
  QVERIFY( segmentized->isMeasure() );
  QCOMPARE( segmentized->pointN( 0 ), l19.pointN( 0 ) );
  QCOMPARE( segmentized->pointN( segmentized->numPoints() - 1 ), l19.pointN( l19.numPoints() - 1 ) );

  // points
  QgsCircularString l20;
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
  QgsCoordinateTransform tr( sourceSrs, destSrs, QgsProject::instance() );

  // 2d CRS transform
  QgsCircularString l21;
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
  QgsCircularString l22;
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
  QgsCircularString l23;
  l23.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 1, 2, 3, 4 )
                 << QgsPoint( QgsWkbTypes::PointZM, 11, 12, 13, 14 ) );
  l23.transform( qtr );
  QCOMPARE( l23.pointN( 0 ), QgsPoint( QgsWkbTypes::PointZM, 2, 6, 3, 4 ) );
  QCOMPARE( l23.pointN( 1 ), QgsPoint( QgsWkbTypes::PointZM, 22, 36, 13, 14 ) );
  QCOMPARE( l23.boundingBox(), QgsRectangle( 2, 6, 22, 36 ) );

  l23.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 1, 2, 3, 4 )
                 << QgsPoint( QgsWkbTypes::PointZM, 11, 12, 13, 14 ) );
  l23.transform( QTransform::fromScale( 1, 1 ), 3, 2, 4, 3 );
  QCOMPARE( l23.pointN( 0 ), QgsPoint( QgsWkbTypes::PointZM, 1, 2, 9, 16 ) );
  QCOMPARE( l23.pointN( 1 ), QgsPoint( QgsWkbTypes::PointZM, 11, 12, 29, 46 ) );

  //insert vertex
  //cannot insert vertex in empty line
  QgsCircularString l24;
  QVERIFY( !l24.insertVertex( QgsVertexId( 0, 0, 0 ), QgsPoint( 6.0, 7.0 ) ) );
  QCOMPARE( l24.numPoints(), 0 );

  //2d line
  l24.setPoints( QgsPointSequence() << QgsPoint( 1, 2 )
                 << QgsPoint( 11, 12 ) << QgsPoint( 1, 22 ) );
  QVERIFY( l24.insertVertex( QgsVertexId( 0, 0, 1 ), QgsPoint( 4.0, 7.0 ) ) );
  QCOMPARE( l24.numPoints(), 5 );
  QVERIFY( !l24.is3D() );
  QVERIFY( !l24.isMeasure() );
  QCOMPARE( l24.wkbType(), QgsWkbTypes::CircularString );
  QCOMPARE( l24.pointN( 0 ), QgsPoint( 1.0, 2.0 ) );
  QCOMPARE( l24.pointN( 1 ), QgsPoint( 4.0, 7.0 ) );
  QGSCOMPARENEAR( l24.pointN( 2 ).x(), 7.192236, 0.01 );
  QGSCOMPARENEAR( l24.pointN( 2 ).y(), 9.930870, 0.01 );
  QCOMPARE( l24.pointN( 3 ), QgsPoint( 11.0, 12.0 ) );
  QCOMPARE( l24.pointN( 4 ), QgsPoint( 1.0, 22.0 ) );

  QVERIFY( l24.insertVertex( QgsVertexId( 0, 0, 1 ), QgsPoint( 8.0, 9.0 ) ) );
  QVERIFY( l24.insertVertex( QgsVertexId( 0, 0, 2 ), QgsPoint( 18.0, 16.0 ) ) );
  QCOMPARE( l24.numPoints(), 9 );
  QCOMPARE( l24.pointN( 0 ), QgsPoint( 1.0, 2.0 ) );
  QGSCOMPARENEAR( l24.pointN( 1 ).x(), 4.363083, 0.01 );
  QGSCOMPARENEAR( l24.pointN( 1 ).y(), 5.636917, 0.01 );
  QCOMPARE( l24.pointN( 2 ), QgsPoint( 8.0, 9.0 ) );
  QCOMPARE( l24.pointN( 3 ), QgsPoint( 18.0, 16.0 ) );
  QGSCOMPARENEAR( l24.pointN( 4 ).x(), 5.876894, 0.01 );
  QGSCOMPARENEAR( l24.pointN( 4 ).y(), 8.246211, 0.01 );
  QCOMPARE( l24.pointN( 5 ), QgsPoint( 4.0, 7.0 ) );
  QGSCOMPARENEAR( l24.pointN( 6 ).x(), 7.192236, 0.01 );
  QGSCOMPARENEAR( l24.pointN( 6 ).y(), 9.930870, 0.01 );
  QCOMPARE( l24.pointN( 7 ), QgsPoint( 11.0, 12.0 ) );
  QCOMPARE( l24.pointN( 8 ), QgsPoint( 1.0, 22.0 ) );

  //insert vertex at end
  QVERIFY( !l24.insertVertex( QgsVertexId( 0, 0, 9 ), QgsPoint( 31.0, 32.0 ) ) );

  //insert vertex past end
  QVERIFY( !l24.insertVertex( QgsVertexId( 0, 0, 10 ), QgsPoint( 41.0, 42.0 ) ) );
  QCOMPARE( l24.numPoints(), 9 );

  //insert vertex before start
  QVERIFY( !l24.insertVertex( QgsVertexId( 0, 0, -18 ), QgsPoint( 41.0, 42.0 ) ) );
  QCOMPARE( l24.numPoints(), 9 );

  //insert 4d vertex in 4d line
  l24.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 1, 1, 2, 3 )
                 << QgsPoint( QgsWkbTypes::PointZM, 1, 10, 4, 5 )
                 << QgsPoint( QgsWkbTypes::PointZM, 15, 10, 6, 7 ) );
  QVERIFY( l24.insertVertex( QgsVertexId( 0, 0, 1 ), QgsPoint( QgsWkbTypes::PointZM, 11, 12, 13, 14 ) ) );
  QCOMPARE( l24.numPoints(), 5 );
  QCOMPARE( l24.pointN( 1 ), QgsPoint( QgsWkbTypes::PointZM, 11, 12, 13, 14 ) );

  //insert 2d vertex in 4d line
  QVERIFY( l24.insertVertex( QgsVertexId( 0, 0, 1 ), QgsPoint( 101, 102 ) ) );
  QCOMPARE( l24.numPoints(), 7 );
  QCOMPARE( l24.wkbType(), QgsWkbTypes::CircularStringZM );
  QCOMPARE( l24.pointN( 1 ), QgsPoint( QgsWkbTypes::PointZM, 101, 102 ) );

  //insert 4d vertex in 2d line
  l24.setPoints( QgsPointSequence() << QgsPoint( 1, 2 )
                 << QgsPoint( 11, 12 ) << QgsPoint( 1, 22 ) );
  QVERIFY( l24.insertVertex( QgsVertexId( 0, 0, 1 ), QgsPoint( QgsWkbTypes::PointZM, 2, 4, 103, 104 ) ) );
  QCOMPARE( l24.numPoints(), 5 );
  QCOMPARE( l24.wkbType(), QgsWkbTypes::CircularString );
  QCOMPARE( l24.pointN( 1 ), QgsPoint( QgsWkbTypes::Point, 2, 4 ) );

  //move vertex

  //empty line
  QgsCircularString l25;
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
  QgsCircularString l26;
  QVERIFY( l26.deleteVertex( QgsVertexId( 0, 0, 0 ) ) );
  QVERIFY( l26.isEmpty() );

  //valid line
  l26.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 1, 2, 2, 3 )
                 << QgsPoint( QgsWkbTypes::PointZM, 11, 12, 4, 5 )
                 << QgsPoint( QgsWkbTypes::PointZM, 21, 22, 6, 7 )
                 << QgsPoint( QgsWkbTypes::PointZM, 31, 32, 6, 7 ) );
  //out of range vertices
  QVERIFY( !l26.deleteVertex( QgsVertexId( 0, 0, -1 ) ) );
  QVERIFY( !l26.deleteVertex( QgsVertexId( 0, 0, 100 ) ) );

  //valid vertices
  QVERIFY( l26.deleteVertex( QgsVertexId( 0, 0, 1 ) ) );
  QCOMPARE( l26.numPoints(), 2 );
  QCOMPARE( l26.pointN( 0 ), QgsPoint( QgsWkbTypes::PointZM, 1, 2, 2, 3 ) );
  QCOMPARE( l26.pointN( 1 ), QgsPoint( QgsWkbTypes::PointZM, 31, 32, 6, 7 ) );

  //removing the next vertex removes all remaining vertices
  QVERIFY( l26.deleteVertex( QgsVertexId( 0, 0, 0 ) ) );
  QCOMPARE( l26.numPoints(), 0 );
  QVERIFY( l26.isEmpty() );
  QVERIFY( l26.deleteVertex( QgsVertexId( 0, 0, 0 ) ) );
  QVERIFY( l26.isEmpty() );

  //reversed
  QgsCircularString l27;
  std::unique_ptr< QgsCircularString > reversed( l27.reversed() );
  QVERIFY( reversed->isEmpty() );
  l27.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 1, 2, 2, 3 )
                 << QgsPoint( QgsWkbTypes::PointZM, 11, 12, 4, 5 )
                 << QgsPoint( QgsWkbTypes::PointZM, 21, 22, 6, 7 ) );
  reversed.reset( l27.reversed() );
  QCOMPARE( reversed->numPoints(), 3 );
  QCOMPARE( reversed->wkbType(), QgsWkbTypes::CircularStringZM );
  QVERIFY( reversed->is3D() );
  QVERIFY( reversed->isMeasure() );
  QCOMPARE( reversed->pointN( 0 ), QgsPoint( QgsWkbTypes::PointZM, 21, 22, 6, 7 ) );
  QCOMPARE( reversed->pointN( 1 ), QgsPoint( QgsWkbTypes::PointZM, 11, 12, 4, 5 ) );
  QCOMPARE( reversed->pointN( 2 ), QgsPoint( QgsWkbTypes::PointZM, 1, 2, 2, 3 ) );

  //addZValue

  QgsCircularString l28;
  QCOMPARE( l28.wkbType(), QgsWkbTypes::CircularString );
  QVERIFY( l28.addZValue() );
  QCOMPARE( l28.wkbType(), QgsWkbTypes::CircularStringZ );
  l28.clear();
  QVERIFY( l28.addZValue() );
  QCOMPARE( l28.wkbType(), QgsWkbTypes::CircularStringZ );
  //2d line
  l28.setPoints( QgsPointSequence() << QgsPoint( 1, 2 ) << QgsPoint( 11, 12 ) );
  QVERIFY( l28.addZValue( 2 ) );
  QVERIFY( l28.is3D() );
  QCOMPARE( l28.wkbType(), QgsWkbTypes::CircularStringZ );
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
  QCOMPARE( l28.wkbType(), QgsWkbTypes::CircularStringZM );
  QCOMPARE( l28.pointN( 0 ), QgsPoint( QgsWkbTypes::PointZM, 1, 2, 5, 3 ) );
  QCOMPARE( l28.pointN( 1 ), QgsPoint( QgsWkbTypes::PointZM, 11, 12, 5, 4 ) );

  //addMValue

  QgsCircularString l29;
  QCOMPARE( l29.wkbType(), QgsWkbTypes::CircularString );
  QVERIFY( l29.addMValue() );
  QCOMPARE( l29.wkbType(), QgsWkbTypes::CircularStringM );
  l29.clear();
  QVERIFY( l29.addMValue() );
  QCOMPARE( l29.wkbType(), QgsWkbTypes::CircularStringM );
  //2d line
  l29.setPoints( QgsPointSequence() << QgsPoint( 1, 2 ) << QgsPoint( 11, 12 ) );
  QVERIFY( l29.addMValue( 2 ) );
  QVERIFY( !l29.is3D() );
  QVERIFY( l29.isMeasure() );
  QCOMPARE( l29.wkbType(), QgsWkbTypes::CircularStringM );
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
  QCOMPARE( l29.wkbType(), QgsWkbTypes::CircularStringZM );
  QCOMPARE( l29.pointN( 0 ), QgsPoint( QgsWkbTypes::PointZM, 1, 2, 3, 5 ) );
  QCOMPARE( l29.pointN( 1 ), QgsPoint( QgsWkbTypes::PointZM, 11, 12, 4, 5 ) );

  //dropZValue
  QgsCircularString l28d;
  QVERIFY( !l28d.dropZValue() );
  l28d.setPoints( QgsPointSequence() << QgsPoint( 1, 2 ) << QgsPoint( 11, 12 ) );
  QVERIFY( !l28d.dropZValue() );
  l28d.addZValue( 1.0 );
  QCOMPARE( l28d.wkbType(), QgsWkbTypes::CircularStringZ );
  QVERIFY( l28d.is3D() );
  QVERIFY( l28d.dropZValue() );
  QVERIFY( !l28d.is3D() );
  QCOMPARE( l28d.wkbType(), QgsWkbTypes::CircularString );
  QCOMPARE( l28d.pointN( 0 ), QgsPoint( QgsWkbTypes::Point, 1, 2 ) );
  QCOMPARE( l28d.pointN( 1 ), QgsPoint( QgsWkbTypes::Point, 11, 12 ) );
  QVERIFY( !l28d.dropZValue() ); //already dropped
  //linestring with m
  l28d.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 1, 2, 3, 4 ) << QgsPoint( QgsWkbTypes::PointZM, 11, 12, 3, 4 ) );
  QVERIFY( l28d.dropZValue() );
  QVERIFY( !l28d.is3D() );
  QVERIFY( l28d.isMeasure() );
  QCOMPARE( l28d.wkbType(), QgsWkbTypes::CircularStringM );
  QCOMPARE( l28d.pointN( 0 ), QgsPoint( QgsWkbTypes::PointM, 1, 2, 0, 4 ) );
  QCOMPARE( l28d.pointN( 1 ), QgsPoint( QgsWkbTypes::PointM, 11, 12, 0, 4 ) );

  //dropMValue
  l28d.setPoints( QgsPointSequence() << QgsPoint( 1, 2 ) << QgsPoint( 11, 12 ) );
  QVERIFY( !l28d.dropMValue() );
  l28d.addMValue( 1.0 );
  QCOMPARE( l28d.wkbType(), QgsWkbTypes::CircularStringM );
  QVERIFY( l28d.isMeasure() );
  QVERIFY( l28d.dropMValue() );
  QVERIFY( !l28d.isMeasure() );
  QCOMPARE( l28d.wkbType(), QgsWkbTypes::CircularString );
  QCOMPARE( l28d.pointN( 0 ), QgsPoint( QgsWkbTypes::Point, 1, 2 ) );
  QCOMPARE( l28d.pointN( 1 ), QgsPoint( QgsWkbTypes::Point, 11, 12 ) );
  QVERIFY( !l28d.dropMValue() ); //already dropped
  //linestring with z
  l28d.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 1, 2, 3, 4 ) << QgsPoint( QgsWkbTypes::PointZM, 11, 12, 3, 4 ) );
  QVERIFY( l28d.dropMValue() );
  QVERIFY( !l28d.isMeasure() );
  QVERIFY( l28d.is3D() );
  QCOMPARE( l28d.wkbType(), QgsWkbTypes::CircularStringZ );
  QCOMPARE( l28d.pointN( 0 ), QgsPoint( QgsWkbTypes::PointZ, 1, 2, 3, 0 ) );
  QCOMPARE( l28d.pointN( 1 ), QgsPoint( QgsWkbTypes::PointZ, 11, 12, 3, 0 ) );

  //convertTo
  l28d.setPoints( QgsPointSequence() << QgsPoint( 1, 2 ) << QgsPoint( 11, 12 ) );
  QVERIFY( l28d.convertTo( QgsWkbTypes::CircularString ) );
  QCOMPARE( l28d.wkbType(), QgsWkbTypes::CircularString );
  QVERIFY( l28d.convertTo( QgsWkbTypes::CircularStringZ ) );
  QCOMPARE( l28d.wkbType(), QgsWkbTypes::CircularStringZ );
  QCOMPARE( l28d.pointN( 0 ), QgsPoint( QgsWkbTypes::PointZ, 1, 2 ) );

  QVERIFY( l28d.convertTo( QgsWkbTypes::CircularStringZM ) );
  QCOMPARE( l28d.wkbType(), QgsWkbTypes::CircularStringZM );
  QCOMPARE( l28d.pointN( 0 ), QgsPoint( QgsWkbTypes::PointZM, 1, 2 ) );
  l28d.moveVertex( QgsVertexId( 0, 0, 0 ), QgsPoint( 1, 2, 5 ) );
  QCOMPARE( l28d.pointN( 0 ), QgsPoint( QgsWkbTypes::PointZM, 1, 2, 5.0 ) );
  //l28d.setMAt( 0, 6.0 );
  QVERIFY( l28d.convertTo( QgsWkbTypes::CircularStringM ) );
  QCOMPARE( l28d.wkbType(), QgsWkbTypes::CircularStringM );
  QCOMPARE( l28d.pointN( 0 ), QgsPoint( QgsWkbTypes::PointM, 1, 2 ) );
  l28d.moveVertex( QgsVertexId( 0, 0, 0 ), QgsPoint( 1, 2, 0, 6 ) );
  QCOMPARE( l28d.pointN( 0 ), QgsPoint( QgsWkbTypes::PointM, 1, 2, 0.0, 6.0 ) );
  QVERIFY( l28d.convertTo( QgsWkbTypes::CircularString ) );
  QCOMPARE( l28d.wkbType(), QgsWkbTypes::CircularString );
  QCOMPARE( l28d.pointN( 0 ), QgsPoint( 1, 2 ) );
  QVERIFY( !l28d.convertTo( QgsWkbTypes::Polygon ) );

  //isRing
  QgsCircularString l30;
  QVERIFY( !l30.isRing() );
  l30.setPoints( QgsPointSequence() << QgsPoint( 1, 2 ) << QgsPoint( 11, 12 ) << QgsPoint( 1, 2 ) );
  QVERIFY( !l30.isRing() ); //<4 points
  l30.setPoints( QgsPointSequence() << QgsPoint( 1, 2 ) << QgsPoint( 11, 12 ) << QgsPoint( 21, 22 ) << QgsPoint( 31, 32 ) );
  QVERIFY( !l30.isRing() ); //not closed
  l30.setPoints( QgsPointSequence() << QgsPoint( 1, 2 ) << QgsPoint( 11, 12 ) << QgsPoint( 21, 22 ) << QgsPoint( 1, 2 ) );
  QVERIFY( l30.isRing() );

  //coordinateSequence
  QgsCircularString l31;
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

  QgsCircularString l32;
  QgsVertexId v;
  QgsPoint p;
  QVERIFY( !l32.nextVertex( v, p ) );
  v = QgsVertexId( 0, 0, -2 );
  QVERIFY( !l32.nextVertex( v, p ) );
  v = QgsVertexId( 0, 0, 10 );
  QVERIFY( !l32.nextVertex( v, p ) );

  //CircularString
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

  //CircularStringZ
  l32.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZ, 1, 2, 3 ) << QgsPoint( QgsWkbTypes::PointZ, 11, 12, 13 ) );
  v = QgsVertexId( 0, 0, -1 );
  QVERIFY( l32.nextVertex( v, p ) );
  QCOMPARE( v, QgsVertexId( 0, 0, 0 ) );
  QCOMPARE( p, QgsPoint( QgsWkbTypes::PointZ, 1, 2, 3 ) );
  QVERIFY( l32.nextVertex( v, p ) );
  QCOMPARE( v, QgsVertexId( 0, 0, 1 ) );
  QCOMPARE( p, QgsPoint( QgsWkbTypes::PointZ, 11, 12, 13 ) );
  QVERIFY( !l32.nextVertex( v, p ) );
  //CircularStringM
  l32.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointM, 1, 2, 0, 4 ) << QgsPoint( QgsWkbTypes::PointM, 11, 12, 0, 14 ) );
  v = QgsVertexId( 0, 0, -1 );
  QVERIFY( l32.nextVertex( v, p ) );
  QCOMPARE( v, QgsVertexId( 0, 0, 0 ) );
  QCOMPARE( p, QgsPoint( QgsWkbTypes::PointM, 1, 2, 0, 4 ) );
  QVERIFY( l32.nextVertex( v, p ) );
  QCOMPARE( v, QgsVertexId( 0, 0, 1 ) );
  QCOMPARE( p, QgsPoint( QgsWkbTypes::PointM, 11, 12, 0, 14 ) );
  QVERIFY( !l32.nextVertex( v, p ) );
  //CircularStringZM
  l32.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 1, 2, 3, 4 ) << QgsPoint( QgsWkbTypes::PointZM, 11, 12, 13, 14 ) );
  v = QgsVertexId( 0, 0, -1 );
  QVERIFY( l32.nextVertex( v, p ) );
  QCOMPARE( v, QgsVertexId( 0, 0, 0 ) );
  QCOMPARE( p, QgsPoint( QgsWkbTypes::PointZM, 1, 2, 3, 4 ) );
  QVERIFY( l32.nextVertex( v, p ) );
  QCOMPARE( v, QgsVertexId( 0, 0, 1 ) );
  QCOMPARE( p, QgsPoint( QgsWkbTypes::PointZM, 11, 12, 13, 14 ) );
  QVERIFY( !l32.nextVertex( v, p ) );

  //vertexAt and pointAt
  QgsCircularString l33;
  l33.vertexAt( QgsVertexId( 0, 0, -10 ) ); //out of bounds, check for no crash
  l33.vertexAt( QgsVertexId( 0, 0, 10 ) ); //out of bounds, check for no crash
  QgsVertexId::VertexType type;
  QVERIFY( !l33.pointAt( -10, p, type ) );
  QVERIFY( !l33.pointAt( 10, p, type ) );
  //CircularString
  l33.setPoints( QgsPointSequence() << QgsPoint( 1, 2 ) << QgsPoint( 11, 12 ) << QgsPoint( 1, 22 ) );
  l33.vertexAt( QgsVertexId( 0, 0, -10 ) );
  l33.vertexAt( QgsVertexId( 0, 0, 10 ) ); //out of bounds, check for no crash
  QCOMPARE( l33.vertexAt( QgsVertexId( 0, 0, 0 ) ), QgsPoint( 1, 2 ) );
  QCOMPARE( l33.vertexAt( QgsVertexId( 0, 0, 1 ) ), QgsPoint( 11, 12 ) );
  QCOMPARE( l33.vertexAt( QgsVertexId( 0, 0, 2 ) ), QgsPoint( 1, 22 ) );
  QVERIFY( !l33.pointAt( -10, p, type ) );
  QVERIFY( !l33.pointAt( 10, p, type ) );
  QVERIFY( l33.pointAt( 0, p, type ) );
  QCOMPARE( p, QgsPoint( 1, 2 ) );
  QCOMPARE( type, QgsVertexId::SegmentVertex );
  QVERIFY( l33.pointAt( 1, p, type ) );
  QCOMPARE( p, QgsPoint( 11, 12 ) );
  QCOMPARE( type, QgsVertexId::CurveVertex );
  QVERIFY( l33.pointAt( 2, p, type ) );
  QCOMPARE( p, QgsPoint( 1, 22 ) );
  QCOMPARE( type, QgsVertexId::SegmentVertex );
  //CircularStringZ
  l33.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZ, 1, 2, 3 ) << QgsPoint( QgsWkbTypes::PointZ, 11, 12, 13 )  << QgsPoint( QgsWkbTypes::PointZ, 1, 22, 23 ) );
  QCOMPARE( l33.vertexAt( QgsVertexId( 0, 0, 0 ) ), QgsPoint( QgsWkbTypes::PointZ, 1, 2, 3 ) );
  QCOMPARE( l33.vertexAt( QgsVertexId( 0, 0, 1 ) ), QgsPoint( QgsWkbTypes::PointZ, 11, 12, 13 ) );
  QCOMPARE( l33.vertexAt( QgsVertexId( 0, 0, 2 ) ), QgsPoint( QgsWkbTypes::PointZ, 1, 22, 23 ) );
  QVERIFY( l33.pointAt( 0, p, type ) );
  QCOMPARE( p, QgsPoint( QgsWkbTypes::PointZ, 1, 2, 3 ) );
  QCOMPARE( type, QgsVertexId::SegmentVertex );
  QVERIFY( l33.pointAt( 1, p, type ) );
  QCOMPARE( p, QgsPoint( QgsWkbTypes::PointZ, 11, 12, 13 ) );
  QCOMPARE( type, QgsVertexId::CurveVertex );
  QVERIFY( l33.pointAt( 2, p, type ) );
  QCOMPARE( p, QgsPoint( 1, 22, 23 ) );
  QCOMPARE( type, QgsVertexId::SegmentVertex );
  //CircularStringM
  l33.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointM, 1, 2, 0, 4 ) << QgsPoint( QgsWkbTypes::PointM, 11, 12, 0, 14 ) << QgsPoint( QgsWkbTypes::PointM, 1, 22, 0, 24 ) );
  QCOMPARE( l33.vertexAt( QgsVertexId( 0, 0, 0 ) ), QgsPoint( QgsWkbTypes::PointM, 1, 2, 0, 4 ) );
  QCOMPARE( l33.vertexAt( QgsVertexId( 0, 0, 1 ) ), QgsPoint( QgsWkbTypes::PointM, 11, 12, 0, 14 ) );
  QCOMPARE( l33.vertexAt( QgsVertexId( 0, 0, 2 ) ), QgsPoint( QgsWkbTypes::PointM, 1, 22, 0, 24 ) );
  QVERIFY( l33.pointAt( 0, p, type ) );
  QCOMPARE( p, QgsPoint( QgsWkbTypes::PointM, 1, 2, 0, 4 ) );
  QCOMPARE( type, QgsVertexId::SegmentVertex );
  QVERIFY( l33.pointAt( 1, p, type ) );
  QCOMPARE( p, QgsPoint( QgsWkbTypes::PointM, 11, 12, 0, 14 ) );
  QCOMPARE( type, QgsVertexId::CurveVertex );
  QVERIFY( l33.pointAt( 2, p, type ) );
  QCOMPARE( p, QgsPoint( QgsWkbTypes::PointM, 1, 22, 0, 24 ) );
  QCOMPARE( type, QgsVertexId::SegmentVertex );
  //CircularStringZM
  l33.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 1, 2, 3, 4 ) << QgsPoint( QgsWkbTypes::PointZM, 11, 12, 13, 14 ) << QgsPoint( QgsWkbTypes::PointZM, 1, 22, 23, 24 ) );
  QCOMPARE( l33.vertexAt( QgsVertexId( 0, 0, 0 ) ), QgsPoint( QgsWkbTypes::PointZM, 1, 2, 3, 4 ) );
  QCOMPARE( l33.vertexAt( QgsVertexId( 0, 0, 1 ) ), QgsPoint( QgsWkbTypes::PointZM, 11, 12, 13, 14 ) );
  QCOMPARE( l33.vertexAt( QgsVertexId( 0, 0, 2 ) ), QgsPoint( QgsWkbTypes::PointZM, 1, 22, 23, 24 ) );
  QVERIFY( l33.pointAt( 0, p, type ) );
  QCOMPARE( p, QgsPoint( QgsWkbTypes::PointZM, 1, 2, 3, 4 ) );
  QCOMPARE( type, QgsVertexId::SegmentVertex );
  QVERIFY( l33.pointAt( 1, p, type ) );
  QCOMPARE( p, QgsPoint( QgsWkbTypes::PointZM, 11, 12, 13, 14 ) );
  QCOMPARE( type, QgsVertexId::CurveVertex );
  QVERIFY( l33.pointAt( 2, p, type ) );
  QCOMPARE( p, QgsPoint( QgsWkbTypes::PointZM, 1, 22, 23, 24 ) );
  QCOMPARE( type, QgsVertexId::SegmentVertex );

  //centroid
  QgsCircularString l34;
  QCOMPARE( l34.centroid(), QgsPoint() );
  l34.setPoints( QgsPointSequence() << QgsPoint( 5, 10 ) );
  QCOMPARE( l34.centroid(), QgsPoint( 5, 10 ) );
  l34.setPoints( QgsPointSequence() << QgsPoint( 0, 0 ) << QgsPoint( 20, 10 ) << QgsPoint( 2, 9 ) );
  QgsPoint centroid = l34.centroid();
  QGSCOMPARENEAR( centroid.x(), 7.333, 0.001 );
  QGSCOMPARENEAR( centroid.y(), 6.333, 0.001 );

  //closest segment
  QgsCircularString l35;
  int leftOf = 0;
  p = QgsPoint(); // reset all coords to zero
  ( void )l35.closestSegment( QgsPoint( 1, 2 ), p, v ); //empty line, just want no crash
  l35.setPoints( QgsPointSequence() << QgsPoint( 5, 10 ) );
  QVERIFY( l35.closestSegment( QgsPoint( 5, 10 ), p, v ) < 0 );
  l35.setPoints( QgsPointSequence() << QgsPoint( 5, 10 ) << QgsPoint( 7, 12 ) << QgsPoint( 5, 15 ) );
  QGSCOMPARENEAR( l35.closestSegment( QgsPoint( 4, 11 ), p, v, &leftOf ), 2.0, 0.0001 );
  QCOMPARE( p, QgsPoint( 5, 10 ) );
  QCOMPARE( v, QgsVertexId( 0, 0, 1 ) );
  QCOMPARE( leftOf, -1 );
  QGSCOMPARENEAR( l35.closestSegment( QgsPoint( 8, 11 ), p, v, &leftOf ),  1.583512, 0.0001 );
  QGSCOMPARENEAR( p.x(), 6.84, 0.01 );
  QGSCOMPARENEAR( p.y(), 11.49, 0.01 );
  QCOMPARE( v, QgsVertexId( 0, 0, 1 ) );
  QCOMPARE( leftOf, 1 );
  QGSCOMPARENEAR( l35.closestSegment( QgsPoint( 5.5, 11.5 ), p, v, &leftOf ), 1.288897, 0.0001 );
  QGSCOMPARENEAR( p.x(), 6.302776, 0.01 );
  QGSCOMPARENEAR( p.y(), 10.7, 0.01 );
  QCOMPARE( v, QgsVertexId( 0, 0, 1 ) );
  QCOMPARE( leftOf, -1 );
  QGSCOMPARENEAR( l35.closestSegment( QgsPoint( 7, 16 ), p, v, &leftOf ), 3.068288, 0.0001 );
  QGSCOMPARENEAR( p.x(), 5.981872, 0.01 );
  QGSCOMPARENEAR( p.y(), 14.574621, 0.01 );
  QCOMPARE( v, QgsVertexId( 0, 0, 2 ) );
  QCOMPARE( leftOf, 1 );
  QGSCOMPARENEAR( l35.closestSegment( QgsPoint( 5.5, 13.5 ), p, v, &leftOf ), 1.288897, 0.0001 );
  QGSCOMPARENEAR( p.x(), 6.302776, 0.01 );
  QGSCOMPARENEAR( p.y(), 14.3, 0.01 );
  QCOMPARE( v, QgsVertexId( 0, 0, 2 ) );
  QCOMPARE( leftOf, -1 );
  // point directly on segment
  QCOMPARE( l35.closestSegment( QgsPoint( 5, 15 ), p, v, &leftOf ), 0.0 );
  QCOMPARE( p, QgsPoint( 5, 15 ) );
  QCOMPARE( v, QgsVertexId( 0, 0, 2 ) );
  QCOMPARE( leftOf, 0 );

  //clockwise string
  l35.setPoints( QgsPointSequence() << QgsPoint( 5, 15 ) << QgsPoint( 7, 12 ) << QgsPoint( 5, 10 ) );
  QGSCOMPARENEAR( l35.closestSegment( QgsPoint( 4, 11 ), p, v, &leftOf ), 2, 0.0001 );
  QGSCOMPARENEAR( p.x(), 5, 0.01 );
  QGSCOMPARENEAR( p.y(), 10, 0.01 );
  QCOMPARE( v, QgsVertexId( 0, 0, 2 ) );
  QCOMPARE( leftOf, 1 );
  QGSCOMPARENEAR( l35.closestSegment( QgsPoint( 8, 11 ), p, v, &leftOf ),  1.583512, 0.0001 );
  QGSCOMPARENEAR( p.x(), 6.84, 0.01 );
  QGSCOMPARENEAR( p.y(), 11.49, 0.01 );
  QCOMPARE( v, QgsVertexId( 0, 0, 2 ) );
  QCOMPARE( leftOf, -1 );
  QGSCOMPARENEAR( l35.closestSegment( QgsPoint( 5.5, 11.5 ), p, v, &leftOf ), 1.288897, 0.0001 );
  QGSCOMPARENEAR( p.x(), 6.302776, 0.01 );
  QGSCOMPARENEAR( p.y(), 10.7, 0.01 );
  QCOMPARE( v, QgsVertexId( 0, 0, 2 ) );
  QCOMPARE( leftOf, 1 );
  QGSCOMPARENEAR( l35.closestSegment( QgsPoint( 7, 16 ), p, v, &leftOf ), 3.068288, 0.0001 );
  QGSCOMPARENEAR( p.x(), 5.981872, 0.01 );
  QGSCOMPARENEAR( p.y(), 14.574621, 0.01 );
  QCOMPARE( v, QgsVertexId( 0, 0, 1 ) );
  QCOMPARE( leftOf, -1 );
  QGSCOMPARENEAR( l35.closestSegment( QgsPoint( 5.5, 13.5 ), p, v, &leftOf ), 1.288897, 0.0001 );
  QGSCOMPARENEAR( p.x(), 6.302776, 0.01 );
  QGSCOMPARENEAR( p.y(), 14.3, 0.01 );
  QCOMPARE( v, QgsVertexId( 0, 0, 1 ) );
  QCOMPARE( leftOf, 1 );
  // point directly on segment
  QCOMPARE( l35.closestSegment( QgsPoint( 5, 15 ), p, v, &leftOf ), 0.0 );
  QCOMPARE( p, QgsPoint( 5, 15 ) );
  QCOMPARE( v, QgsVertexId( 0, 0, 1 ) );
  QCOMPARE( leftOf, 0 );

  //sumUpArea
  QgsCircularString l36;
  double area = 1.0; //sumUpArea adds to area, so start with non-zero value
  l36.sumUpArea( area );
  QCOMPARE( area, 1.0 );
  l36.setPoints( QgsPointSequence() << QgsPoint( 5, 10 ) );
  l36.sumUpArea( area );
  QCOMPARE( area, 1.0 );
  l36.setPoints( QgsPointSequence() << QgsPoint( 5, 10 ) << QgsPoint( 10, 10 ) );
  l36.sumUpArea( area );
  QCOMPARE( area, 1.0 );
  l36.setPoints( QgsPointSequence() << QgsPoint( 0, 0 ) << QgsPoint( 2, 0 ) << QgsPoint( 2, 2 ) );
  l36.sumUpArea( area );
  QGSCOMPARENEAR( area, 4.141593, 0.0001 );
  l36.setPoints( QgsPointSequence() << QgsPoint( 0, 0 ) << QgsPoint( 2, 0 ) << QgsPoint( 2, 2 ) << QgsPoint( 0, 2 ) );
  l36.sumUpArea( area );
  QGSCOMPARENEAR( area, 7.283185, 0.0001 );
  // full circle
  l36.setPoints( QgsPointSequence() << QgsPoint( 0, 0 ) << QgsPoint( 4, 0 ) << QgsPoint( 0, 0 ) );
  area = 0.0;
  l36.sumUpArea( area );
  QGSCOMPARENEAR( area, 12.566370614359172, 0.0001 );

  //boundingBox - test that bounding box is updated after every modification to the circular string
  QgsCircularString l37;
  QVERIFY( l37.boundingBox().isNull() );
  l37.setPoints( QgsPointSequence() << QgsPoint( 5, 10 ) << QgsPoint( 10, 15 ) );
  QCOMPARE( l37.boundingBox(), QgsRectangle( 5, 10, 10, 15 ) );
  l37.setPoints( QgsPointSequence() << QgsPoint( -5, -10 ) << QgsPoint( -6, -10 ) << QgsPoint( -5.5, -9 ) );
  QCOMPARE( l37.boundingBox(), QgsRectangle( -6.125, -10.25, -5, -9 ) );
  QByteArray wkbToAppend = l37.asWkb();
  l37.clear();
  QVERIFY( l37.boundingBox().isNull() );
  QgsConstWkbPtr wkbToAppendPtr( wkbToAppend );
  l37.setPoints( QgsPointSequence() << QgsPoint( 5, 10 ) << QgsPoint( 10, 15 ) );
  QCOMPARE( l37.boundingBox(), QgsRectangle( 5, 10, 10, 15 ) );
  l37.fromWkb( wkbToAppendPtr );
  QCOMPARE( l37.boundingBox(), QgsRectangle( -6.125, -10.25, -5, -9 ) );
  l37.fromWkt( QStringLiteral( "CircularString( 5 10, 6 10, 5.5 9 )" ) );
  QCOMPARE( l37.boundingBox(), QgsRectangle( 5, 9, 6.125, 10.25 ) );
  l37.insertVertex( QgsVertexId( 0, 0, 1 ), QgsPoint( -1, 7 ) );
  QgsRectangle r = l37.boundingBox();
  QGSCOMPARENEAR( r.xMinimum(), -3.014, 0.01 );
  QGSCOMPARENEAR( r.xMaximum(), 14.014, 0.01 );
  QGSCOMPARENEAR( r.yMinimum(), -7.0146, 0.01 );
  QGSCOMPARENEAR( r.yMaximum(), 12.4988, 0.01 );
  l37.moveVertex( QgsVertexId( 0, 0, 1 ), QgsPoint( -3, 10 ) );
  r = l37.boundingBox();
  QGSCOMPARENEAR( r.xMinimum(), -10.294, 0.01 );
  QGSCOMPARENEAR( r.xMaximum(), 12.294, 0.01 );
  QGSCOMPARENEAR( r.yMinimum(), 9, 0.01 );
  QGSCOMPARENEAR( r.yMaximum(), 31.856, 0.01 );
  l37.deleteVertex( QgsVertexId( 0, 0, 1 ) );
  r = l37.boundingBox();
  QGSCOMPARENEAR( r.xMinimum(), 5, 0.01 );
  QGSCOMPARENEAR( r.xMaximum(), 6.125, 0.01 );
  QGSCOMPARENEAR( r.yMinimum(), 9, 0.01 );
  QGSCOMPARENEAR( r.yMaximum(), 10.25, 0.01 );

  //angle
  QgsCircularString l38;
  ( void )l38.vertexAngle( QgsVertexId() ); //just want no crash
  ( void )l38.vertexAngle( QgsVertexId( 0, 0, 0 ) ); //just want no crash
  l38.setPoints( QgsPointSequence() << QgsPoint( 0, 0 ) );
  ( void )l38.vertexAngle( QgsVertexId( 0, 0, 0 ) ); //just want no crash, any answer is meaningless
  l38.setPoints( QgsPointSequence() << QgsPoint( 0, 0 ) << QgsPoint( 1, 0 ) );
  ( void )l38.vertexAngle( QgsVertexId( 0, 0, 0 ) ); //just want no crash, any answer is meaningless
  ( void )l38.vertexAngle( QgsVertexId( 0, 0, 1 ) ); //just want no crash, any answer is meaningless
  l38.setPoints( QgsPointSequence() << QgsPoint( 0, 0 ) << QgsPoint( 1, 1 ) << QgsPoint( 0, 2 ) );
  QGSCOMPARENEAR( l38.vertexAngle( QgsVertexId( 0, 0, 0 ) ), 1.5708, 0.0001 );
  QGSCOMPARENEAR( l38.vertexAngle( QgsVertexId( 0, 0, 1 ) ), 0, 0.0001 );
  QGSCOMPARENEAR( l38.vertexAngle( QgsVertexId( 0, 0, 2 ) ), 4.712389, 0.0001 );
  l38.setPoints( QgsPointSequence() << QgsPoint( 0, 2 ) << QgsPoint( 1, 1 ) << QgsPoint( 0, 0 ) );
  QGSCOMPARENEAR( l38.vertexAngle( QgsVertexId( 0, 0, 0 ) ), 1.5708, 0.0001 );
  QGSCOMPARENEAR( l38.vertexAngle( QgsVertexId( 0, 0, 1 ) ), 3.141593, 0.0001 );
  QGSCOMPARENEAR( l38.vertexAngle( QgsVertexId( 0, 0, 2 ) ), 4.712389, 0.0001 );
  ( void )l38.vertexAngle( QgsVertexId( 0, 0, 20 ) ); // no crash
  l38.setPoints( QgsPointSequence() << QgsPoint( 0, 0 ) << QgsPoint( 1, 1 ) << QgsPoint( 0, 2 )
                 << QgsPoint( -1, 3 ) << QgsPoint( 0, 4 ) );
  QGSCOMPARENEAR( l38.vertexAngle( QgsVertexId( 0, 0, 0 ) ), 1.5708, 0.0001 );
  QGSCOMPARENEAR( l38.vertexAngle( QgsVertexId( 0, 0, 1 ) ), 0, 0.0001 );
  QGSCOMPARENEAR( l38.vertexAngle( QgsVertexId( 0, 0, 2 ) ), 4.712389, 0.0001 );
  QGSCOMPARENEAR( l38.vertexAngle( QgsVertexId( 0, 0, 3 ) ), 0, 0.0001 );
  QGSCOMPARENEAR( l38.vertexAngle( QgsVertexId( 0, 0, 4 ) ), 1.5708, 0.0001 );
  l38.setPoints( QgsPointSequence() << QgsPoint( 0, 4 ) << QgsPoint( -1, 3 ) << QgsPoint( 0, 2 )
                 << QgsPoint( 1, 1 ) << QgsPoint( 0, 0 ) );
  QGSCOMPARENEAR( l38.vertexAngle( QgsVertexId( 0, 0, 0 ) ), 4.712389, 0.0001 );
  QGSCOMPARENEAR( l38.vertexAngle( QgsVertexId( 0, 0, 1 ) ), 3.141592, 0.0001 );
  QGSCOMPARENEAR( l38.vertexAngle( QgsVertexId( 0, 0, 2 ) ), 1.5708, 0.0001 );
  QGSCOMPARENEAR( l38.vertexAngle( QgsVertexId( 0, 0, 3 ) ), 3.141592, 0.0001 );
  QGSCOMPARENEAR( l38.vertexAngle( QgsVertexId( 0, 0, 4 ) ), 4.712389, 0.0001 );

  //closed circular string
  l38.setPoints( QgsPointSequence() << QgsPoint( 0, 0 ) << QgsPoint( 1, 0 ) << QgsPoint( 0, 0 ) );
  QGSCOMPARENEAR( l38.vertexAngle( QgsVertexId( 0, 0, 0 ) ), 0, 0.00001 );
  QGSCOMPARENEAR( l38.vertexAngle( QgsVertexId( 0, 0, 1 ) ), 3.141592, 0.00001 );
  QGSCOMPARENEAR( l38.vertexAngle( QgsVertexId( 0, 0, 2 ) ), 0, 0.00001 );

  //removing a vertex from a 3 point circular string should remove the whole line
  QgsCircularString l39;
  l39.setPoints( QVector<QgsPoint>() << QgsPoint( 0, 0 ) << QgsPoint( 1, 1 ) << QgsPoint( 0, 2 ) );
  QCOMPARE( l39.numPoints(), 3 );
  l39.deleteVertex( QgsVertexId( 0, 0, 2 ) );
  QCOMPARE( l39.numPoints(), 0 );

  //boundary
  QgsCircularString boundary1;
  QVERIFY( !boundary1.boundary() );
  boundary1.setPoints( QVector<QgsPoint>() << QgsPoint( 0, 0 ) << QgsPoint( 1, 0 ) << QgsPoint( 1, 1 ) );
  QgsAbstractGeometry *boundary = boundary1.boundary();
  QgsMultiPoint *mpBoundary = dynamic_cast< QgsMultiPoint * >( boundary );
  QVERIFY( mpBoundary );
  QCOMPARE( static_cast< QgsPoint *>( mpBoundary->geometryN( 0 ) )->x(), 0.0 );
  QCOMPARE( static_cast< QgsPoint *>( mpBoundary->geometryN( 0 ) )->y(), 0.0 );
  QCOMPARE( static_cast< QgsPoint *>( mpBoundary->geometryN( 1 ) )->x(), 1.0 );
  QCOMPARE( static_cast< QgsPoint *>( mpBoundary->geometryN( 1 ) )->y(), 1.0 );
  delete boundary;

  // closed string = no boundary
  boundary1.setPoints( QVector<QgsPoint>() << QgsPoint( 0, 0 ) << QgsPoint( 1, 0 ) << QgsPoint( 1, 1 ) << QgsPoint( 0, 0 ) );
  QVERIFY( !boundary1.boundary() );

  //boundary with z
  boundary1.setPoints( QVector<QgsPoint>() << QgsPoint( QgsWkbTypes::PointZ, 0, 0, 10 ) << QgsPoint( QgsWkbTypes::PointZ, 1, 0, 15 ) << QgsPoint( QgsWkbTypes::PointZ, 1, 1, 20 ) );
  boundary = boundary1.boundary();
  mpBoundary = dynamic_cast< QgsMultiPoint * >( boundary );
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

  // addToPainterPath (note most tests are in test_qgsgeometry.py)
  QgsCircularString path;
  QPainterPath pPath;
  path.addToPainterPath( pPath );
  QVERIFY( pPath.isEmpty() );
  path.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZ, 1, 2, 3 ) << QgsPoint( QgsWkbTypes::PointZ, 11, 12, 13 )
                  << QgsPoint( QgsWkbTypes::PointZ, 21, 2, 3 ) );
  path.addToPainterPath( pPath );
  QGSCOMPARENEAR( pPath.currentPosition().x(), 21.0, 0.01 );
  QGSCOMPARENEAR( pPath.currentPosition().y(), 2.0, 0.01 );
  QVERIFY( !pPath.isEmpty() );

  // even number of points - should still work
  pPath = QPainterPath();
  path.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZ, 1, 2, 3 ) << QgsPoint( QgsWkbTypes::PointZ, 11, 12, 13 ) );
  path.addToPainterPath( pPath );
  QGSCOMPARENEAR( pPath.currentPosition().x(), 11.0, 0.01 );
  QGSCOMPARENEAR( pPath.currentPosition().y(), 12.0, 0.01 );
  QVERIFY( !pPath.isEmpty() );

  // toCurveType
  QgsCircularString curveLine1;
  curveLine1.setPoints( QgsPointSequence() << QgsPoint( 1, 2 ) << QgsPoint( 11, 12 ) << QgsPoint( 1, 22 ) );
  std::unique_ptr< QgsCurve > curveType( curveLine1.toCurveType() );
  QCOMPARE( curveType->wkbType(), QgsWkbTypes::CircularString );
  QCOMPARE( curveType->numPoints(), 3 );
  QCOMPARE( curveType->vertexAt( QgsVertexId( 0, 0, 0 ) ), QgsPoint( 1, 2 ) );
  QCOMPARE( curveType->vertexAt( QgsVertexId( 0, 0, 1 ) ), QgsPoint( 11, 12 ) );
  QCOMPARE( curveType->vertexAt( QgsVertexId( 0, 0, 2 ) ), QgsPoint( 1, 22 ) );

  //segmentLength
  QgsCircularString curveLine2;
  QCOMPARE( curveLine2.segmentLength( QgsVertexId( -1, 0, 0 ) ), 0.0 );
  QCOMPARE( curveLine2.segmentLength( QgsVertexId( 0, 0, 0 ) ), 0.0 );
  QCOMPARE( curveLine2.segmentLength( QgsVertexId( 1, 0, 0 ) ), 0.0 );
  curveLine2.setPoints( QgsPointSequence() << QgsPoint( 1, 2 ) << QgsPoint( 11, 12 ) << QgsPoint( 1, 22 ) );
  QCOMPARE( curveLine2.segmentLength( QgsVertexId() ), 0.0 );
  QCOMPARE( curveLine2.segmentLength( QgsVertexId( 0, 0, -1 ) ), 0.0 );
  QGSCOMPARENEAR( curveLine2.segmentLength( QgsVertexId( 0, 0, 0 ) ), 31.4159, 0.001 );
  QCOMPARE( curveLine2.segmentLength( QgsVertexId( 0, 0, 1 ) ), 0.0 );
  QCOMPARE( curveLine2.segmentLength( QgsVertexId( 0, 0, 2 ) ), 0.0 );
  QCOMPARE( curveLine2.segmentLength( QgsVertexId( -1, 0, -1 ) ), 0.0 );
  QGSCOMPARENEAR( curveLine2.segmentLength( QgsVertexId( -1, 0, 0 ) ), 31.4159, 0.001 );
  QCOMPARE( curveLine2.segmentLength( QgsVertexId( 1, 0, 1 ) ), 0.0 );
  QGSCOMPARENEAR( curveLine2.segmentLength( QgsVertexId( 1, 1, 0 ) ), 31.4159, 0.001 );
  curveLine2.setPoints( QgsPointSequence() << QgsPoint( 1, 2 ) << QgsPoint( 11, 12 ) << QgsPoint( 1, 22 ) << QgsPoint( -9, 32 ) << QgsPoint( 1, 42 ) );
  QCOMPARE( curveLine2.segmentLength( QgsVertexId() ), 0.0 );
  QCOMPARE( curveLine2.segmentLength( QgsVertexId( 0, 0, -1 ) ), 0.0 );
  QGSCOMPARENEAR( curveLine2.segmentLength( QgsVertexId( 0, 0, 0 ) ), 31.4159, 0.001 );
  QCOMPARE( curveLine2.segmentLength( QgsVertexId( 0, 0, 1 ) ), 0.0 );
  QGSCOMPARENEAR( curveLine2.segmentLength( QgsVertexId( 0, 0, 2 ) ), 31.4159, 0.001 );
  QCOMPARE( curveLine2.segmentLength( QgsVertexId( 0, 0, 3 ) ), 0.0 );

  //removeDuplicateNodes
  QgsCircularString nodeLine;
  QVERIFY( !nodeLine.removeDuplicateNodes() );
  nodeLine.setPoints( QgsPointSequence() << QgsPoint( 11, 2 ) << QgsPoint( 11, 12 ) << QgsPoint( 111, 12 ) );
  QVERIFY( !nodeLine.removeDuplicateNodes() );
  QCOMPARE( nodeLine.asWkt(), QStringLiteral( "CircularString (11 2, 11 12, 111 12)" ) );
  nodeLine.setPoints( QgsPointSequence() << QgsPoint( 11, 2 ) << QgsPoint( 11, 12 ) << QgsPoint( 11, 2 ) );
  QVERIFY( !nodeLine.removeDuplicateNodes() );
  QCOMPARE( nodeLine.asWkt(), QStringLiteral( "CircularString (11 2, 11 12, 11 2)" ) );
  nodeLine.setPoints( QgsPointSequence() << QgsPoint( 11, 2 ) << QgsPoint( 10, 3 ) << QgsPoint( 11.01, 1.99 ) << QgsPoint( 9, 3 )
                      << QgsPoint( 11, 2 ) );
  QVERIFY( !nodeLine.removeDuplicateNodes( 0.02 ) );
  QCOMPARE( nodeLine.asWkt( 2 ), QStringLiteral( "CircularString (11 2, 10 3, 11.01 1.99, 9 3, 11 2)" ) );
  nodeLine.setPoints( QgsPointSequence() << QgsPoint( 11, 2 ) << QgsPoint( 11.01, 1.99 ) << QgsPoint( 11.02, 2.01 )
                      << QgsPoint( 11, 12 ) << QgsPoint( 111, 12 ) << QgsPoint( 111.01, 11.99 ) );
  QVERIFY( !nodeLine.removeDuplicateNodes() );
  QCOMPARE( nodeLine.asWkt( 2 ), QStringLiteral( "CircularString (11 2, 11.01 1.99, 11.02 2.01, 11 12, 111 12, 111.01 11.99)" ) );
  QVERIFY( nodeLine.removeDuplicateNodes( 0.02 ) );
  QCOMPARE( nodeLine.asWkt( 2 ), QStringLiteral( "CircularString (11 2, 11 12, 111 12, 111.01 11.99)" ) );

  // don't create degenerate lines
  nodeLine.setPoints( QgsPointSequence() << QgsPoint( 11, 2 ) );
  QVERIFY( !nodeLine.removeDuplicateNodes( 0.02 ) );
  QCOMPARE( nodeLine.asWkt( 2 ), QStringLiteral( "CircularString (11 2)" ) );
  nodeLine.setPoints( QgsPointSequence() << QgsPoint( 11, 2 ) << QgsPoint( 11.01, 1.99 ) );
  QVERIFY( !nodeLine.removeDuplicateNodes( 0.02 ) );
  QCOMPARE( nodeLine.asWkt( 2 ), QStringLiteral( "CircularString (11 2, 11.01 1.99)" ) );
  nodeLine.setPoints( QgsPointSequence() << QgsPoint( 11, 2 ) << QgsPoint( 11.01, 1.99 ) << QgsPoint( 11, 2 ) );
  QVERIFY( !nodeLine.removeDuplicateNodes( 0.02 ) );
  QCOMPARE( nodeLine.asWkt( 2 ), QStringLiteral( "CircularString (11 2, 11.01 1.99, 11 2)" ) );

  // with z
  nodeLine.setPoints( QgsPointSequence() << QgsPoint( 11, 2, 1 ) << QgsPoint( 11.01, 1.99, 2 ) << QgsPoint( 11.02, 2.01, 3 )
                      << QgsPoint( 11, 12, 4 ) << QgsPoint( 111, 12, 5 ) );
  QVERIFY( nodeLine.removeDuplicateNodes( 0.02 ) );
  QCOMPARE( nodeLine.asWkt( 2 ), QStringLiteral( "CircularStringZ (11 2 1, 11 12 4, 111 12 5)" ) );
  nodeLine.setPoints( QgsPointSequence() << QgsPoint( 11, 2, 1 ) << QgsPoint( 11.01, 1.99, 2 ) << QgsPoint( 11.02, 2.01, 3 )
                      << QgsPoint( 11, 12, 4 ) << QgsPoint( 111, 12, 5 ) );
  QVERIFY( !nodeLine.removeDuplicateNodes( 0.02, true ) );
  QCOMPARE( nodeLine.asWkt( 2 ), QStringLiteral( "CircularStringZ (11 2 1, 11.01 1.99 2, 11.02 2.01 3, 11 12 4, 111 12 5)" ) );

  //swap xy
  QgsCircularString swapLine;
  swapLine.swapXy(); // no crash
  swapLine.setPoints( QgsPointSequence() << QgsPoint( 11, 2, 3, 4, QgsWkbTypes::PointZM ) << QgsPoint( 11, 12, 13, 14, QgsWkbTypes::PointZM ) << QgsPoint( 111, 12, 23, 24, QgsWkbTypes::PointZM ) );
  swapLine.swapXy();
  QCOMPARE( swapLine.asWkt(), QStringLiteral( "CircularStringZM (2 11 3 4, 12 11 13 14, 12 111 23 24)" ) );

  // filter vertex
  QgsCircularString filterLine;
  auto filter = []( const QgsPoint & point )-> bool
  {
    return point.x() < 5;
  };
  filterLine.filterVertices( filter ); // no crash
  filterLine.setPoints( QgsPointSequence() << QgsPoint( 1, 2, 3, 4, QgsWkbTypes::PointZM ) << QgsPoint( 4, 12, 13, 14, QgsWkbTypes::PointZM ) << QgsPoint( 111, 12, 23, 24, QgsWkbTypes::PointZM ) );
  filterLine.filterVertices( filter );
  QCOMPARE( filterLine.asWkt( 2 ), QStringLiteral( "CircularStringZM (1 2 3 4, 4 12 13 14)" ) );

  // transform vertex
  QgsCircularString transformLine;
  auto transform = []( const QgsPoint & point )-> QgsPoint
  {
    return QgsPoint( point.x() + 2, point.y() + 3, point.z() + 4, point.m() + 7 );
  };
  transformLine.transformVertices( transform ); // no crash
  transformLine.setPoints( QgsPointSequence() << QgsPoint( 1, 2, 3, 4, QgsWkbTypes::PointZM ) << QgsPoint( 4, 12, 13, 14, QgsWkbTypes::PointZM ) << QgsPoint( 111, 12, 23, 24, QgsWkbTypes::PointZM ) );
  transformLine.transformVertices( transform );
  QCOMPARE( transformLine.asWkt( 2 ), QStringLiteral( "CircularStringZM (3 5 7 11, 6 15 17 21, 113 15 27 31)" ) );


  // substring
  QgsCircularString substring;
  std::unique_ptr< QgsCircularString > substringResult( substring.curveSubstring( 1, 2 ) ); // no crash
  QVERIFY( substringResult.get() );
  QVERIFY( substringResult->isEmpty() );
  // CircularStringZM (10 0 1 2, 11 1 3 4, 12 0 13 14)
  substring.setPoints( QgsPointSequence() << QgsPoint( 10, 0, 1, 2 ) <<  QgsPoint( 11, 1, 3, 4 ) <<  QgsPoint( 12, 0, 13, 14 ) );
  substringResult.reset( substring.curveSubstring( 0, 0 ) );
  QCOMPARE( substringResult->asWkt( 2 ), QStringLiteral( "CircularStringZM (10 0 1 2, 10 0 1 2, 10 0 1 2)" ) );
  substringResult.reset( substring.curveSubstring( -1, -0.1 ) );
  QVERIFY( substringResult->isEmpty() );
  substringResult.reset( substring.curveSubstring( 100000, 10000 ) );
  QVERIFY( substringResult->isEmpty() );
  substringResult.reset( substring.curveSubstring( -1, 1 ) );
  QCOMPARE( substringResult->asWkt( 2 ), QStringLiteral( "CircularStringZM (10 0 1 2, 10.12 0.48 1.64 2.64, 10.46 0.84 2.27 3.27)" ) );
  substringResult.reset( substring.curveSubstring( 1, -1 ) );
  QCOMPARE( substringResult->asWkt( 2 ), QStringLiteral( "CircularStringZM (10.46 0.84 2.27 3.27, 10.46 0.84 2.27 3.27, 10.46 0.84 2.27 3.27)" ) );
  substringResult.reset( substring.curveSubstring( -1, 10000 ) );
  QCOMPARE( substringResult->asWkt( 2 ), QStringLiteral( "CircularStringZM (10 0 1 2, 11 1 3 4, 12 0 13 14)" ) );
  substringResult.reset( substring.curveSubstring( 1, 10000 ) );
  QCOMPARE( substringResult->asWkt( 2 ), QStringLiteral( "CircularStringZM (10.46 0.84 2.27 3.27, 11.48 0.88 6.18 7.18, 12 0 13 14)" ) );
  substringResult.reset( substring.curveSubstring( 1, 20 ) );
  QCOMPARE( substringResult->asWkt( 2 ), QStringLiteral( "CircularStringZM (10.46 0.84 2.27 3.27, 11.48 0.88 6.18 7.18, 12 0 13 14)" ) );
  substringResult.reset( substring.curveSubstring( 1, 1.5 ) );
  QCOMPARE( substringResult->asWkt( 2 ), QStringLiteral( "CircularStringZM (10.46 0.84 2.27 3.27, 10.68 0.95 2.59 3.59, 10.93 1 2.91 3.91)" ) );
  // CircularStringZM (10 0 1 2, 11 1 3 4, 12 0 13 14, 14 -1 13 14, 16 1 23 24 )
  substring.setPoints( QgsPointSequence() << QgsPoint( 10, 0, 1, 2 ) <<  QgsPoint( 11, 1, 3, 4 ) <<  QgsPoint( 12, 0, 13, 14 )
                       <<  QgsPoint( 14, -1, 13, 14 ) <<  QgsPoint( 16, 1, 23, 24 ) );
  substringResult.reset( substring.curveSubstring( 1, 1.5 ) );
  QCOMPARE( substringResult->asWkt( 2 ), QStringLiteral( "CircularStringZM (10.46 0.84 2.27 3.27, 10.68 0.95 2.59 3.59, 10.93 1 2.91 3.91)" ) );
  substringResult.reset( substring.curveSubstring( 1, 10 ) );
  QCOMPARE( substringResult->asWkt( 2 ), QStringLiteral( "CircularStringZM (10.46 0.84 2.27 3.27, 11.48 0.88 6.18 7.18, 12 0 13 14, 14 -1 13 14, 16 1 23 24)" ) );
  substringResult.reset( substring.curveSubstring( 1, 6 ) );
  QCOMPARE( substringResult->asWkt( 2 ), QStringLiteral( "CircularStringZM (10.46 0.84 2.27 3.27, 11.48 0.88 6.18 7.18, 12 0 13 14, 13.1 -0.88 13 14, 14.5 -0.9 14.65 15.65)" ) );
  substringResult.reset( substring.curveSubstring( 0, 6 ) );
  QCOMPARE( substringResult->asWkt( 2 ), QStringLiteral( "CircularStringZM (10 0 1 2, 11 1 3 4, 12 0 13 14, 13.1 -0.88 13 14, 14.5 -0.9 14.65 15.65)" ) );
  substringResult.reset( substring.curveSubstring( 5, 6 ) );
  QCOMPARE( substringResult->asWkt( 2 ), QStringLiteral( "CircularStringZM (13.51 -0.98 13 14, 14.01 -1 13.03 14.03, 14.5 -0.9 14.65 15.65)" ) );
  substringResult.reset( substring.curveSubstring( 5, 1000 ) );
  QCOMPARE( substringResult->asWkt( 2 ), QStringLiteral( "CircularStringZM (13.51 -0.98 13 14, 15.19 -0.53 17.2 18.2, 16 1 23 24)" ) );

  substring.setPoints( QgsPointSequence() << QgsPoint( 10, 0, 1 ) <<  QgsPoint( 11, 1, 3 ) <<  QgsPoint( 12, 0, 13 )
                       <<  QgsPoint( 14, -1, 13 ) <<  QgsPoint( 16, 1, 23 ) );
  substringResult.reset( substring.curveSubstring( 1, 20 ) );
  QCOMPARE( substringResult->asWkt( 2 ), QStringLiteral( "CircularStringZ (10.46 0.84 2.27, 11.48 0.88 6.18, 12 0 13, 14 -1 13, 16 1 23)" ) );
  substring.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointM, 10, 0, 0, 1 ) <<  QgsPoint( QgsWkbTypes::PointM, 11, 1, 0, 3 ) <<  QgsPoint( QgsWkbTypes::PointM, 12, 0, 0, 13 )
                       <<  QgsPoint( QgsWkbTypes::PointM, 14, -1, 0, 13 ) <<  QgsPoint( QgsWkbTypes::PointM, 16, 1, 0, 23 ) );
  substringResult.reset( substring.curveSubstring( 1, 20 ) );
  QCOMPARE( substringResult->asWkt( 2 ), QStringLiteral( "CircularStringM (10.46 0.84 2.27, 11.48 0.88 6.18, 12 0 13, 14 -1 13, 16 1 23)" ) );
  substring.setPoints( QgsPointSequence() << QgsPoint( 10, 0 ) <<  QgsPoint( 11, 1 ) <<  QgsPoint( 12, 0 )
                       <<  QgsPoint( 14, -1 ) <<  QgsPoint( 16, 1 ) );
  substringResult.reset( substring.curveSubstring( 1, 20 ) );
  QCOMPARE( substringResult->asWkt( 2 ), QStringLiteral( "CircularString (10.46 0.84, 11.48 0.88, 12 0, 14 -1, 16 1)" ) );

  // interpolate
  QgsCircularString interpolate;
  std::unique_ptr< QgsPoint > interpolateResult( interpolate.interpolatePoint( 1 ) ); // no crash
  QVERIFY( !interpolateResult.get() );
  // CircularStringZM (10 0 1 2, 11 1 3 4, 12 0 13 14)
  interpolate.setPoints( QgsPointSequence() << QgsPoint( 10, 0, 1, 2 ) <<  QgsPoint( 11, 1, 3, 4 ) <<  QgsPoint( 12, 0, 13, 14 ) );
  interpolateResult.reset( interpolate.interpolatePoint( 0 ) );
  QCOMPARE( interpolateResult->asWkt( 2 ), QStringLiteral( "PointZM (10 0 1 2)" ) );
  interpolateResult.reset( interpolate.interpolatePoint( -1 ) );
  QVERIFY( !interpolateResult.get() );
  interpolateResult.reset( interpolate.interpolatePoint( 100000 ) );
  QVERIFY( !interpolateResult.get() );
  interpolateResult.reset( interpolate.interpolatePoint( 1 ) );
  QCOMPARE( interpolateResult->asWkt( 2 ), QStringLiteral( "PointZM (10.46 0.84 2.27 3.27)" ) );
  interpolateResult.reset( interpolate.interpolatePoint( 1.5 ) );
  QCOMPARE( interpolateResult->asWkt( 2 ), QStringLiteral( "PointZM (10.93 1 2.91 3.91)" ) );
  interpolateResult.reset( interpolate.interpolatePoint( interpolate.length() ) );
  QCOMPARE( interpolateResult->asWkt( 2 ), QStringLiteral( "PointZM (12 0 13 14)" ) );
  // CircularStringZM (10 0 1 2, 11 1 3 4, 12 0 13 14, 14 -1 13 14, 16 1 23 24 )
  interpolate.setPoints( QgsPointSequence() << QgsPoint( 10, 0, 1, 2 ) <<  QgsPoint( 11, 1, 3, 4 ) <<  QgsPoint( 12, 0, 13, 14 )
                         <<  QgsPoint( 14, -1, 13, 14 ) <<  QgsPoint( 16, 1, 23, 24 ) );
  interpolateResult.reset( interpolate.interpolatePoint( 1 ) );
  QCOMPARE( interpolateResult->asWkt( 2 ), QStringLiteral( "PointZM (10.46 0.84 2.27 3.27)" ) );
  interpolateResult.reset( interpolate.interpolatePoint( 1.5 ) );
  QCOMPARE( interpolateResult->asWkt( 2 ), QStringLiteral( "PointZM (10.93 1 2.91 3.91)" ) );
  interpolateResult.reset( interpolate.interpolatePoint( 10 ) );
  QVERIFY( !interpolateResult.get() );
  interpolateResult.reset( interpolate.interpolatePoint( 6 ) );
  QCOMPARE( interpolateResult->asWkt( 2 ), QStringLiteral( "PointZM (14.5 -0.9 14.65 15.65)" ) );
  interpolateResult.reset( interpolate.interpolatePoint( 5 ) );
  QCOMPARE( interpolateResult->asWkt( 2 ), QStringLiteral( "PointZM (13.51 -0.98 13 14)" ) );

  interpolate.setPoints( QgsPointSequence() << QgsPoint( 10, 0, 1 ) <<  QgsPoint( 11, 1, 3 ) <<  QgsPoint( 12, 0, 13 )
                         <<  QgsPoint( 14, -1, 13 ) <<  QgsPoint( 16, 1, 23 ) );
  interpolateResult.reset( interpolate.interpolatePoint( 1 ) );
  QCOMPARE( interpolateResult->asWkt( 2 ), QStringLiteral( "PointZ (10.46 0.84 2.27)" ) );
  interpolate.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointM, 10, 0, 0, 1 ) <<  QgsPoint( QgsWkbTypes::PointM, 11, 1, 0, 3 ) <<  QgsPoint( QgsWkbTypes::PointM, 12, 0, 0, 13 )
                         <<  QgsPoint( QgsWkbTypes::PointM, 14, -1, 0, 13 ) <<  QgsPoint( QgsWkbTypes::PointM, 16, 1, 0, 23 ) );
  interpolateResult.reset( interpolate.interpolatePoint( 1 ) );
  QCOMPARE( interpolateResult->asWkt( 2 ), QStringLiteral( "PointM (10.46 0.84 2.27)" ) );
  interpolate.setPoints( QgsPointSequence() << QgsPoint( 10, 0 ) <<  QgsPoint( 11, 1 ) <<  QgsPoint( 12, 0 )
                         <<  QgsPoint( 14, -1 ) <<  QgsPoint( 16, 1 ) );
  interpolateResult.reset( interpolate.interpolatePoint( 1 ) );
  QCOMPARE( interpolateResult->asWkt( 2 ), QStringLiteral( "Point (10.46 0.84)" ) );

  // orientation
  QgsCircularString orientation;
  ( void )orientation.orientation(); // no crash
  orientation.setPoints( QgsPointSequence() << QgsPoint( 0, 0 ) << QgsPoint( 0, 1 ) << QgsPoint( 1, 1 ) << QgsPoint( 1, 0 ) << QgsPoint( 0, 0 ) );
  QCOMPARE( orientation.orientation(), QgsCurve::Clockwise );
  orientation.setPoints( QgsPointSequence() << QgsPoint( 0, 0 ) << QgsPoint( 1, 0 ) << QgsPoint( 1, 1 ) << QgsPoint( 0, 1 ) << QgsPoint( 0, 0 ) );
  QCOMPARE( orientation.orientation(), QgsCurve::CounterClockwise );
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
  QCOMPARE( *fromArray.xData(), 1.0 );
  QCOMPARE( *( fromArray.xData() + 1 ), 2.0 );
  QCOMPARE( *( fromArray.xData() + 2 ), 3.0 );
  QCOMPARE( *fromArray.yData(), 11.0 );
  QCOMPARE( *( fromArray.yData() + 1 ), 12.0 );
  QCOMPARE( *( fromArray.yData() + 2 ), 13.0 );

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
  fromArray4 = QgsLineString( xx, yy, zz, QVector< double >(), true );  // LineString25D
  QCOMPARE( fromArray4.wkbType(), QgsWkbTypes::LineString25D );
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
  // unbalanced -> z truncated
  zz = QVector< double >() << 21 << 22 << 23 << 24;
  fromArray5 = QgsLineString( xx, yy, zz );
  QCOMPARE( fromArray5.wkbType(), QgsWkbTypes::LineStringZ );
  QCOMPARE( fromArray5.numPoints(), 3 );
  QCOMPARE( fromArray5.xAt( 0 ), 1.0 );
  QCOMPARE( fromArray5.yAt( 0 ), 11.0 );
  QCOMPARE( fromArray5.zAt( 0 ), 21.0 );
  QCOMPARE( fromArray5.xAt( 1 ), 2.0 );
  QCOMPARE( fromArray5.yAt( 1 ), 12.0 );
  QCOMPARE( fromArray5.zAt( 1 ), 22.0 );
  QCOMPARE( fromArray5.xAt( 2 ), 3.0 );
  QCOMPARE( fromArray5.yAt( 2 ), 13.0 );
  QCOMPARE( fromArray5.zAt( 2 ), 23.0 );
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
  // unbalanced -> m truncated
  mm = QVector< double >() << 21 << 22 << 23 << 24;
  fromArray7 = QgsLineString( xx, yy, QVector< double >(), mm );
  QCOMPARE( fromArray7.wkbType(), QgsWkbTypes::LineStringM );
  QCOMPARE( fromArray7.numPoints(), 3 );
  QCOMPARE( fromArray7.xAt( 0 ), 1.0 );
  QCOMPARE( fromArray7.yAt( 0 ), 11.0 );
  QCOMPARE( fromArray7.mAt( 0 ), 21.0 );
  QCOMPARE( fromArray7.xAt( 1 ), 2.0 );
  QCOMPARE( fromArray7.yAt( 1 ), 12.0 );
  QCOMPARE( fromArray7.mAt( 1 ), 22.0 );
  QCOMPARE( fromArray7.xAt( 2 ), 3.0 );
  QCOMPARE( fromArray7.yAt( 2 ), 13.0 );
  QCOMPARE( fromArray7.mAt( 2 ), 23.0 );
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

  QCOMPARE( *fromArray8.xData(), 1.0 );
  QCOMPARE( *( fromArray8.xData() + 1 ), 2.0 );
  QCOMPARE( *( fromArray8.xData() + 2 ), 3.0 );
  QCOMPARE( *fromArray8.yData(), 11.0 );
  QCOMPARE( *( fromArray8.yData() + 1 ), 12.0 );
  QCOMPARE( *( fromArray8.yData() + 2 ), 13.0 );
  QCOMPARE( *fromArray8.zData(), 21.0 );
  QCOMPARE( *( fromArray8.zData() + 1 ), 22.0 );
  QCOMPARE( *( fromArray8.zData() + 2 ), 23.0 );
  QCOMPARE( *fromArray8.mData(), 31.0 );
  QCOMPARE( *( fromArray8.mData() + 1 ), 32.0 );
  QCOMPARE( *( fromArray8.mData() + 2 ), 33.0 );

  // from QList<QgsPointXY>
  QgsLineString fromPtsA = QgsLineString( QVector< QgsPoint >() );
  QVERIFY( fromPtsA.isEmpty() );
  QCOMPARE( fromPtsA.wkbType(), QgsWkbTypes::LineString );

  fromPtsA = QgsLineString( QVector< QgsPoint >()  << QgsPoint( 1, 2, 0, 4, QgsWkbTypes::PointM ) );
  QCOMPARE( fromPtsA.numPoints(), 1 );
  QCOMPARE( fromPtsA.wkbType(), QgsWkbTypes::LineStringM );

  QVector<QgsPointXY> ptsA;
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

  // from 2 points
  QgsLineString from2Pts( QgsPoint( 1, 2 ), QgsPoint( 21, 22 ) );
  QCOMPARE( from2Pts.wkbType(), QgsWkbTypes::LineString );
  QCOMPARE( from2Pts.numPoints(), 2 );
  QCOMPARE( from2Pts.xAt( 0 ), 1.0 );
  QCOMPARE( from2Pts.yAt( 0 ), 2.0 );
  QCOMPARE( from2Pts.xAt( 1 ), 21.0 );
  QCOMPARE( from2Pts.yAt( 1 ), 22.0 );
  from2Pts = QgsLineString( QgsPoint( QgsWkbTypes::PointZ, 1, 2, 3 ), QgsPoint( QgsWkbTypes::PointZ, 21, 22, 23 ) );
  QCOMPARE( from2Pts.wkbType(), QgsWkbTypes::LineStringZ );
  QCOMPARE( from2Pts.numPoints(), 2 );
  QCOMPARE( from2Pts.xAt( 0 ), 1.0 );
  QCOMPARE( from2Pts.yAt( 0 ), 2.0 );
  QCOMPARE( from2Pts.zAt( 0 ), 3.0 );
  QCOMPARE( from2Pts.xAt( 1 ), 21.0 );
  QCOMPARE( from2Pts.yAt( 1 ), 22.0 );
  QCOMPARE( from2Pts.zAt( 1 ), 23.0 );
  from2Pts = QgsLineString( QgsPoint( QgsWkbTypes::PointM, 1, 2, 0, 3 ), QgsPoint( QgsWkbTypes::PointM, 21, 22, 0, 23 ) );
  QCOMPARE( from2Pts.wkbType(), QgsWkbTypes::LineStringM );
  QCOMPARE( from2Pts.numPoints(), 2 );
  QCOMPARE( from2Pts.xAt( 0 ), 1.0 );
  QCOMPARE( from2Pts.yAt( 0 ), 2.0 );
  QCOMPARE( from2Pts.mAt( 0 ), 3.0 );
  QCOMPARE( from2Pts.xAt( 1 ), 21.0 );
  QCOMPARE( from2Pts.yAt( 1 ), 22.0 );
  QCOMPARE( from2Pts.mAt( 1 ), 23.0 );
  from2Pts = QgsLineString( QgsPoint( QgsWkbTypes::PointZM, 1, 2, 3, 4 ), QgsPoint( QgsWkbTypes::PointZM, 21, 22, 23, 24 ) );
  QCOMPARE( from2Pts.wkbType(), QgsWkbTypes::LineStringZM );
  QCOMPARE( from2Pts.numPoints(), 2 );
  QCOMPARE( from2Pts.xAt( 0 ), 1.0 );
  QCOMPARE( from2Pts.yAt( 0 ), 2.0 );
  QCOMPARE( from2Pts.zAt( 0 ), 3.0 );
  QCOMPARE( from2Pts.mAt( 0 ), 4.0 );
  QCOMPARE( from2Pts.xAt( 1 ), 21.0 );
  QCOMPARE( from2Pts.yAt( 1 ), 22.0 );
  QCOMPARE( from2Pts.zAt( 1 ), 23.0 );
  QCOMPARE( from2Pts.mAt( 1 ), 24.0 );

  // from lineSegment
  QgsLineString fromSegment( QgsLineSegment2D( QgsPointXY( 1, 2 ), QgsPointXY( 3, 4 ) ) );
  QCOMPARE( fromSegment.wkbType(), QgsWkbTypes::LineString );
  QCOMPARE( fromSegment.numPoints(), 2 );
  QCOMPARE( fromSegment.xAt( 0 ), 1.0 );
  QCOMPARE( fromSegment.yAt( 0 ), 2.0 );
  QCOMPARE( fromSegment.xAt( 1 ), 3.0 );
  QCOMPARE( fromSegment.yAt( 1 ), 4.0 );

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
  QgsPointSequence pts;
  l8.points( pts );
  QCOMPARE( pts, QgsPointSequence() << QgsPoint( 1, 2 ) << QgsPoint( 2, 3 ) << QgsPoint( 3, 4 ) );
  QCOMPARE( *l8.xData(), 1.0 );
  QCOMPARE( *( l8.xData() + 1 ), 2.0 );
  QCOMPARE( *( l8.xData() + 2 ), 3.0 );
  QCOMPARE( *l8.yData(), 2.0 );
  QCOMPARE( *( l8.yData() + 1 ), 3.0 );
  QCOMPARE( *( l8.yData() + 2 ), 4.0 );

  //setPoints with empty list, should clear linestring
  l8.setPoints( QgsPointSequence() );
  QVERIFY( l8.isEmpty() );
  QCOMPARE( l8.numPoints(), 0 );
  QCOMPARE( l8.vertexCount(), 0 );
  QCOMPARE( l8.nCoordinates(), 0 );
  QCOMPARE( l8.ringCount(), 0 );
  QCOMPARE( l8.partCount(), 0 );
  QCOMPARE( l8.wkbType(), QgsWkbTypes::LineString );
  l8.points( pts );
  QVERIFY( pts.isEmpty() );

  //setPoints with z
  l8.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZ, 1, 2, 3 ) << QgsPoint( QgsWkbTypes::PointZ, 2, 3, 4 ) );
  QCOMPARE( l8.numPoints(), 2 );
  QVERIFY( l8.is3D() );
  QVERIFY( !l8.isMeasure() );
  QCOMPARE( l8.wkbType(), QgsWkbTypes::LineStringZ );
  l8.points( pts );
  QCOMPARE( pts, QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZ, 1, 2, 3 ) << QgsPoint( QgsWkbTypes::PointZ, 2, 3, 4 ) );

  //setPoints with 25d
  l8.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::Point25D, 1, 2, 4 ) << QgsPoint( QgsWkbTypes::Point25D, 2, 3, 4 ) );
  QCOMPARE( l8.numPoints(), 2 );
  QVERIFY( l8.is3D() );
  QVERIFY( !l8.isMeasure() );
  QCOMPARE( l8.wkbType(), QgsWkbTypes::LineString25D );
  QCOMPARE( l8.pointN( 0 ), QgsPoint( QgsWkbTypes::Point25D, 1, 2, 4 ) );
  l8.points( pts );
  QCOMPARE( pts, QgsPointSequence() << QgsPoint( QgsWkbTypes::Point25D, 1, 2, 4 ) << QgsPoint( QgsWkbTypes::Point25D, 2, 3, 4 ) );

  //setPoints with m
  l8.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointM, 1, 2, 0, 3 ) << QgsPoint( QgsWkbTypes::PointM, 2, 3, 0, 4 ) );
  QCOMPARE( l8.numPoints(), 2 );
  QVERIFY( !l8.is3D() );
  QVERIFY( l8.isMeasure() );
  QCOMPARE( l8.wkbType(), QgsWkbTypes::LineStringM );
  l8.points( pts );
  QCOMPARE( pts, QgsPointSequence() << QgsPoint( QgsWkbTypes::PointM, 1, 2, 0, 3 ) << QgsPoint( QgsWkbTypes::PointM, 2, 3, 0, 4 ) );

  //setPoints with zm
  l8.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 1, 2, 4, 5 ) << QgsPoint( QgsWkbTypes::PointZM, 2, 3, 4, 5 ) );
  QCOMPARE( l8.numPoints(), 2 );
  QVERIFY( l8.is3D() );
  QVERIFY( l8.isMeasure() );
  QCOMPARE( l8.wkbType(), QgsWkbTypes::LineStringZM );
  l8.points( pts );
  QCOMPARE( pts, QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 1, 2, 4, 5 ) << QgsPoint( QgsWkbTypes::PointZM, 2, 3, 4, 5 ) );

  //setPoints with MIXED dimensionality of points
  l8.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 1, 2, 4, 5 ) << QgsPoint( QgsWkbTypes::PointM, 2, 3, 0, 5 ) );
  QCOMPARE( l8.numPoints(), 2 );
  QVERIFY( l8.is3D() );
  QVERIFY( l8.isMeasure() );
  QCOMPARE( l8.wkbType(), QgsWkbTypes::LineStringZM );
  l8.points( pts );
  QCOMPARE( pts, QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 1, 2, 4, 5 ) << QgsPoint( QgsWkbTypes::PointZM, 2, 3, 0, 5 ) );

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
  QVERIFY( std::isnan( l9.zAt( -1 ) ) ); //out of range
  QVERIFY( std::isnan( l9.zAt( 11 ) ) ); //out of range

  l9.setZAt( 0, 53.0 );
  QCOMPARE( l9.zAt( 0 ), 53.0 );
  l9.setZAt( 1, 63.0 );
  QCOMPARE( l9.zAt( 1 ), 63.0 );
  l9.setZAt( -1, 53.0 ); //out of range
  l9.setZAt( 11, 53.0 ); //out of range

  QCOMPARE( l9.mAt( 0 ), 4.0 );
  QCOMPARE( l9.mAt( 1 ), 14.0 );
  QCOMPARE( l9.mAt( 2 ), 24.0 );
  QVERIFY( std::isnan( l9.mAt( -1 ) ) ); //out of range
  QVERIFY( std::isnan( l9.mAt( 11 ) ) ); //out of range

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
  QVERIFY( std::isnan( l9.zAt( 0 ) ) );
  QVERIFY( std::isnan( l9.zAt( 1 ) ) );
  l9.setZAt( 0, 53.0 );
  l9.setZAt( 1, 63.0 );

  //check mAt/setMAt with non-measure linestring
  l9.setPoints( QgsPointSequence() << QgsPoint( 1, 2 )
                << QgsPoint( 11, 12 )
                << QgsPoint( 21, 22 ) );

  //basically we just don't want these to crash
  QVERIFY( std::isnan( l9.mAt( 0 ) ) );
  QVERIFY( std::isnan( l9.mAt( 1 ) ) );
  l9.setMAt( 0, 53.0 );
  l9.setMAt( 1, 63.0 );

  //append linestring

  //append to empty
  QgsLineString l10;
  l10.append( nullptr );
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

  QVERIFY( e6 != QgsCircularString() );
  QgsPoint p1;
  QVERIFY( !( e6 == p1 ) );
  QVERIFY( e6 != p1 );
  QVERIFY( e6 == e6 );

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
  QGSCOMPAREGML( elemToString( exportLine.asGml2( doc ) ), expectedGML2 );
  QString expectedGML2prec3( QStringLiteral( "<LineString xmlns=\"gml\"><coordinates xmlns=\"gml\" cs=\",\" ts=\" \">0.333,0.667 1.333,1.667 2.333,2.667</coordinates></LineString>" ) );
  QGSCOMPAREGML( elemToString( exportLineFloat.asGml2( doc, 3 ) ), expectedGML2prec3 );
  QString expectedGML2empty( QStringLiteral( "<LineString xmlns=\"gml\"/>" ) );
  QGSCOMPAREGML( elemToString( QgsLineString().asGml2( doc ) ), expectedGML2empty );

  //asGML3
  QString expectedGML3( QStringLiteral( "<LineString xmlns=\"gml\"><posList xmlns=\"gml\" srsDimension=\"2\">31 32 41 42 51 52</posList></LineString>" ) );
  QCOMPARE( elemToString( exportLine.asGml3( doc ) ), expectedGML3 );
  QString expectedGML3prec3( QStringLiteral( "<LineString xmlns=\"gml\"><posList xmlns=\"gml\" srsDimension=\"2\">0.333 0.667 1.333 1.667 2.333 2.667</posList></LineString>" ) );
  QCOMPARE( elemToString( exportLineFloat.asGml3( doc, 3 ) ), expectedGML3prec3 );
  QString expectedGML3empty( QStringLiteral( "<LineString xmlns=\"gml\"/>" ) );
  QGSCOMPAREGML( elemToString( QgsLineString().asGml3( doc ) ), expectedGML3empty );

  //asJSON
  QString expectedJson( QStringLiteral( "{\"type\": \"LineString\", \"coordinates\": [ [31, 32], [41, 42], [51, 52]]}" ) );
  QCOMPARE( exportLine.asJson(), expectedJson );
  QString expectedJsonPrec3( QStringLiteral( "{\"type\": \"LineString\", \"coordinates\": [ [0.333, 0.667], [1.333, 1.667], [2.333, 2.667]]}" ) );
  QCOMPARE( exportLineFloat.asJson( 3 ), expectedJsonPrec3 );

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
  QgsCoordinateTransform tr( sourceSrs, destSrs, QgsProject::instance() );

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

  l23.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 1, 2, 3, 4 )
                 << QgsPoint( QgsWkbTypes::PointZM, 11, 12, 13, 14 ) );
  l23.transform( QTransform::fromScale( 1, 1 ), 3, 2, 4, 3 );
  QCOMPARE( l23.pointN( 0 ), QgsPoint( QgsWkbTypes::PointZM, 1, 2, 9, 16 ) );
  QCOMPARE( l23.pointN( 1 ), QgsPoint( QgsWkbTypes::PointZM, 11, 12, 29, 46 ) );

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

  // vertex iterator on empty linestring
  QgsAbstractGeometry::vertex_iterator it1 = l32.vertices_begin();
  QCOMPARE( it1, l32.vertices_end() );

  // Java-style iterator on empty linetring
  QgsVertexIterator it1x( &l32 );
  QVERIFY( !it1x.hasNext() );

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

  // vertex iterator
  QgsAbstractGeometry::vertex_iterator it2 = l32.vertices_begin();
  QCOMPARE( *it2, QgsPoint( 1, 2 ) );
  QCOMPARE( it2.vertexId(), QgsVertexId( 0, 0, 0 ) );
  ++it2;
  QCOMPARE( *it2, QgsPoint( 11, 12 ) );
  QCOMPARE( it2.vertexId(), QgsVertexId( 0, 0, 1 ) );
  ++it2;
  QCOMPARE( it2, l32.vertices_end() );

  // Java-style iterator
  QgsVertexIterator it2x( &l32 );
  QVERIFY( it2x.hasNext() );
  QCOMPARE( it2x.next(), QgsPoint( 1, 2 ) );
  QVERIFY( it2x.hasNext() );
  QCOMPARE( it2x.next(), QgsPoint( 11, 12 ) );
  QVERIFY( !it2x.hasNext() );

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
  int leftOf = 0;
  p = QgsPoint(); // reset all coords to zero
  ( void )l35.closestSegment( QgsPoint( 1, 2 ), p, v ); //empty line, just want no crash
  l35.setPoints( QgsPointSequence() << QgsPoint( 5, 10 ) );
  QVERIFY( l35.closestSegment( QgsPoint( 5, 10 ), p, v ) < 0 );
  l35.setPoints( QgsPointSequence() << QgsPoint( 5, 10 ) << QgsPoint( 10, 10 ) );
  QGSCOMPARENEAR( l35.closestSegment( QgsPoint( 4, 11 ), p, v, &leftOf ), 2.0, 4 * std::numeric_limits<double>::epsilon() );
  QCOMPARE( p, QgsPoint( 5, 10 ) );
  QCOMPARE( v, QgsVertexId( 0, 0, 1 ) );
  QCOMPARE( leftOf, -1 );
  QGSCOMPARENEAR( l35.closestSegment( QgsPoint( 8, 11 ), p, v, &leftOf ), 1.0, 4 * std::numeric_limits<double>::epsilon() );
  QCOMPARE( p, QgsPoint( 8, 10 ) );
  QCOMPARE( v, QgsVertexId( 0, 0, 1 ) );
  QCOMPARE( leftOf, -1 );
  QGSCOMPARENEAR( l35.closestSegment( QgsPoint( 8, 9 ), p, v, &leftOf ), 1.0, 4 * std::numeric_limits<double>::epsilon() );
  QCOMPARE( p, QgsPoint( 8, 10 ) );
  QCOMPARE( v, QgsVertexId( 0, 0, 1 ) );
  QCOMPARE( leftOf, 1 );
  QGSCOMPARENEAR( l35.closestSegment( QgsPoint( 11, 9 ), p, v, &leftOf ), 2.0, 4 * std::numeric_limits<double>::epsilon() );
  QCOMPARE( p, QgsPoint( 10, 10 ) );
  QCOMPARE( v, QgsVertexId( 0, 0, 1 ) );
  QCOMPARE( leftOf, 1 );
  l35.setPoints( QgsPointSequence() << QgsPoint( 5, 10 )
                 << QgsPoint( 10, 10 )
                 << QgsPoint( 10, 15 ) );
  QGSCOMPARENEAR( l35.closestSegment( QgsPoint( 11, 12 ), p, v, &leftOf ), 1.0, 4 * std::numeric_limits<double>::epsilon() );
  QCOMPARE( p, QgsPoint( 10, 12 ) );
  QCOMPARE( v, QgsVertexId( 0, 0, 2 ) );
  QCOMPARE( leftOf, 1 );

  l35.setPoints( QgsPointSequence() << QgsPoint( 5, 5 )
                 << QgsPoint( 6, 4 )
                 << QgsPoint( 4, 4 )
                 << QgsPoint( 5, 5 ) );
  QGSCOMPARENEAR( l35.closestSegment( QgsPoint( 2.35, 4 ), p, v, &leftOf ), 2.7225, 4 * std::numeric_limits<double>::epsilon() );
  QCOMPARE( p, QgsPoint( 4, 4 ) );
  QCOMPARE( v, QgsVertexId( 0, 0, 2 ) );
  QCOMPARE( leftOf, -1 );

  l35.setPoints( QgsPointSequence() << QgsPoint( 5, 5 )
                 << QgsPoint( 4, 4 )
                 << QgsPoint( 6, 4 )
                 << QgsPoint( 5, 5 ) );
  QGSCOMPARENEAR( l35.closestSegment( QgsPoint( 2.35, 4 ), p, v, &leftOf ), 2.7225, 4 * std::numeric_limits<double>::epsilon() );
  QCOMPARE( p, QgsPoint( 4, 4 ) );
  QCOMPARE( v, QgsVertexId( 0, 0, 1 ) );
  QCOMPARE( leftOf, 1 );

  l35.setPoints( QgsPointSequence() << QgsPoint( 5, 5 )
                 << QgsPoint( 6, 4 )
                 << QgsPoint( 4, 4 )
                 << QgsPoint( 5, 5 ) );
  QGSCOMPARENEAR( l35.closestSegment( QgsPoint( 3.5, 2 ), p, v, &leftOf ), 4.250000, 4 * std::numeric_limits<double>::epsilon() );
  QCOMPARE( p, QgsPoint( 4, 4 ) );
  QCOMPARE( v, QgsVertexId( 0, 0, 2 ) );
  QCOMPARE( leftOf, -1 );

  l35.setPoints( QgsPointSequence() << QgsPoint( 5, 5 )
                 << QgsPoint( 4, 4 )
                 << QgsPoint( 6, 4 )
                 << QgsPoint( 5, 5 ) );
  QGSCOMPARENEAR( l35.closestSegment( QgsPoint( 3.5, 2 ), p, v, &leftOf ), 4.250000, 4 * std::numeric_limits<double>::epsilon() );
  QCOMPARE( p, QgsPoint( 4, 4 ) );
  QCOMPARE( v, QgsVertexId( 0, 0, 1 ) );
  QCOMPARE( leftOf, 1 );

  l35.setPoints( QgsPointSequence() << QgsPoint( 1, 1 )
                 << QgsPoint( 1, 4 )
                 << QgsPoint( 2, 2 )
                 << QgsPoint( 1, 1 ) );
  QGSCOMPARENEAR( l35.closestSegment( QgsPoint( 1, 0 ), p, v, &leftOf ), 1, 4 * std::numeric_limits<double>::epsilon() );
  QCOMPARE( p, QgsPoint( 1, 1 ) );
  QCOMPARE( v, QgsVertexId( 0, 0, 1 ) );
  QCOMPARE( leftOf, -1 );

  l35.setPoints( QgsPointSequence() << QgsPoint( 1, 1 )
                 << QgsPoint( 2, 2 )
                 << QgsPoint( 1, 4 )
                 << QgsPoint( 1, 1 ) );
  QGSCOMPARENEAR( l35.closestSegment( QgsPoint( 1, 0 ), p, v, &leftOf ), 1, 4 * std::numeric_limits<double>::epsilon() );
  QCOMPARE( p, QgsPoint( 1, 1 ) );
  QCOMPARE( v, QgsVertexId( 0, 0, 1 ) );
  QCOMPARE( leftOf, 1 );

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
  QGSCOMPARENEAR( area, -24, 4 * std::numeric_limits<double>::epsilon() );
  l36.setPoints( QgsPointSequence() << QgsPoint( 0, 0 ) << QgsPoint( 2, 0 ) << QgsPoint( 2, 2 ) );
  l36.sumUpArea( area );
  QGSCOMPARENEAR( area, -22, 4 * std::numeric_limits<double>::epsilon() );
  l36.setPoints( QgsPointSequence() << QgsPoint( 0, 0 ) << QgsPoint( 2, 0 ) << QgsPoint( 2, 2 ) << QgsPoint( 0, 2 ) );
  l36.sumUpArea( area );
  QGSCOMPARENEAR( area, -18, 4 * std::numeric_limits<double>::epsilon() );

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
  QGSCOMPARENEAR( l38.vertexAngle( QgsVertexId( 0, 0, 0 ) ), 1.5708, 0.0001 );
  QGSCOMPARENEAR( l38.vertexAngle( QgsVertexId( 0, 0, 1 ) ), 1.5708, 0.0001 );
  ( void )l38.vertexAngle( QgsVertexId( 0, 0, 2 ) ); //no crash
  l38.setPoints( QgsPointSequence() << QgsPoint( 0, 0 ) << QgsPoint( 0, 1 ) );
  QGSCOMPARENEAR( l38.vertexAngle( QgsVertexId( 0, 0, 0 ) ), 0.0, 4 * std::numeric_limits<double>::epsilon() );
  QGSCOMPARENEAR( l38.vertexAngle( QgsVertexId( 0, 0, 1 ) ), 0.0, 4 * std::numeric_limits<double>::epsilon() );
  l38.setPoints( QgsPointSequence() << QgsPoint( 1, 0 ) << QgsPoint( 0, 0 ) );
  QGSCOMPARENEAR( l38.vertexAngle( QgsVertexId( 0, 0, 0 ) ), 4.71239, 0.0001 );
  QGSCOMPARENEAR( l38.vertexAngle( QgsVertexId( 0, 0, 1 ) ), 4.71239, 0.0001 );
  l38.setPoints( QgsPointSequence() << QgsPoint( 0, 1 ) << QgsPoint( 0, 0 ) );
  QGSCOMPARENEAR( l38.vertexAngle( QgsVertexId( 0, 0, 0 ) ), 3.1416, 0.0001 );
  QGSCOMPARENEAR( l38.vertexAngle( QgsVertexId( 0, 0, 1 ) ), 3.1416, 0.0001 );
  l38.setPoints( QgsPointSequence() << QgsPoint( 0, 0 ) << QgsPoint( 1, 0 ) << QgsPoint( 1, 1 ) );
  QGSCOMPARENEAR( l38.vertexAngle( QgsVertexId( 0, 0, 0 ) ), 1.5708, 0.0001 );
  QGSCOMPARENEAR( l38.vertexAngle( QgsVertexId( 0, 0, 1 ) ), 0.7854, 0.0001 );
  QGSCOMPARENEAR( l38.vertexAngle( QgsVertexId( 0, 0, 2 ) ), 0.0, 0.0001 );
  l38.setPoints( QgsPointSequence() << QgsPoint( 0, 0 ) << QgsPoint( 0.5, 0 ) << QgsPoint( 1, 0 )
                 << QgsPoint( 2, 1 ) << QgsPoint( 1, 2 ) << QgsPoint( 0, 2 ) );
  ( void )l38.vertexAngle( QgsVertexId( 0, 0, 20 ) );
  QGSCOMPARENEAR( l38.vertexAngle( QgsVertexId( 0, 0, 0 ) ), 1.5708, 0.0001 );
  QGSCOMPARENEAR( l38.vertexAngle( QgsVertexId( 0, 0, 1 ) ), 1.5708, 0.0001 );
  QGSCOMPARENEAR( l38.vertexAngle( QgsVertexId( 0, 0, 2 ) ), 1.17809, 0.00001 );
  QGSCOMPARENEAR( l38.vertexAngle( QgsVertexId( 0, 0, 3 ) ), 0.0, 0.00001 );
  QGSCOMPARENEAR( l38.vertexAngle( QgsVertexId( 0, 0, 4 ) ), 5.10509, 0.00001 );
  QGSCOMPARENEAR( l38.vertexAngle( QgsVertexId( 0, 0, 5 ) ), 4.71239, 0.00001 );
  //closed line string
  l38.close();
  QGSCOMPARENEAR( l38.vertexAngle( QgsVertexId( 0, 0, 5 ) ), 3.92699, 0.00001 );
  QGSCOMPARENEAR( l38.vertexAngle( QgsVertexId( 0, 0, 0 ) ), 2.35619, 0.00001 );
  QGSCOMPARENEAR( l38.vertexAngle( QgsVertexId( 0, 0, 6 ) ), 2.35619, 0.00001 );

  //removing the second to last vertex should remove the whole line
  QgsLineString l39;
  l39.setPoints( QVector<QgsPoint>() << QgsPoint( 0, 0 ) << QgsPoint( 1, 1 ) );
  QVERIFY( l39.numPoints() == 2 );
  l39.deleteVertex( QgsVertexId( 0, 0, 1 ) );
  QVERIFY( l39.numPoints() == 0 );

  //boundary
  QgsLineString boundary1;
  QVERIFY( !boundary1.boundary() );
  boundary1.setPoints( QVector<QgsPoint>() << QgsPoint( 0, 0 ) << QgsPoint( 1, 0 ) << QgsPoint( 1, 1 ) );
  QgsAbstractGeometry *boundary = boundary1.boundary();
  QgsMultiPoint *mpBoundary = dynamic_cast< QgsMultiPoint * >( boundary );
  QVERIFY( mpBoundary );
  QCOMPARE( static_cast< QgsPoint *>( mpBoundary->geometryN( 0 ) )->x(), 0.0 );
  QCOMPARE( static_cast< QgsPoint *>( mpBoundary->geometryN( 0 ) )->y(), 0.0 );
  QCOMPARE( static_cast< QgsPoint *>( mpBoundary->geometryN( 1 ) )->x(), 1.0 );
  QCOMPARE( static_cast< QgsPoint *>( mpBoundary->geometryN( 1 ) )->y(), 1.0 );
  delete boundary;

  // closed string = no boundary
  boundary1.setPoints( QVector<QgsPoint>() << QgsPoint( 0, 0 ) << QgsPoint( 1, 0 ) << QgsPoint( 1, 1 ) << QgsPoint( 0, 0 ) );
  QVERIFY( !boundary1.boundary() );
  \

  //boundary with z
  boundary1.setPoints( QVector<QgsPoint>() << QgsPoint( QgsWkbTypes::PointZ, 0, 0, 10 ) << QgsPoint( QgsWkbTypes::PointZ, 1, 0, 15 ) << QgsPoint( QgsWkbTypes::PointZ, 1, 1, 20 ) );
  boundary = boundary1.boundary();
  mpBoundary = dynamic_cast< QgsMultiPoint * >( boundary );
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
  extend1.setPoints( QVector<QgsPoint>() << QgsPoint( 0, 0 ) << QgsPoint( 1, 0 ) << QgsPoint( 1, 1 ) );
  extend1.extend( 1, 2 );
  QCOMPARE( extend1.pointN( 0 ), QgsPoint( QgsWkbTypes::Point, -1, 0 ) );
  QCOMPARE( extend1.pointN( 1 ), QgsPoint( QgsWkbTypes::Point, 1, 0 ) );
  QCOMPARE( extend1.pointN( 2 ), QgsPoint( QgsWkbTypes::Point, 1, 3 ) );

  // addToPainterPath (note most tests are in test_qgsgeometry.py)
  QgsLineString path;
  QPainterPath pPath;
  path.addToPainterPath( pPath );
  QVERIFY( pPath.isEmpty() );
  path.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZ, 1, 2, 3 ) << QgsPoint( QgsWkbTypes::PointZ, 11, 12, 13 ) );
  path.addToPainterPath( pPath );
  QVERIFY( !pPath.isEmpty() );

  // toCurveType
  QgsLineString curveLine1;
  curveLine1.setPoints( QgsPointSequence() << QgsPoint( 1, 2 ) << QgsPoint( 11, 12 ) );
  std::unique_ptr< QgsCompoundCurve > curveType( curveLine1.toCurveType() );
  QCOMPARE( curveType->wkbType(), QgsWkbTypes::CompoundCurve );
  QCOMPARE( curveType->numPoints(), 2 );
  QCOMPARE( curveType->vertexAt( QgsVertexId( 0, 0, 0 ) ), QgsPoint( 1, 2 ) );
  QCOMPARE( curveType->vertexAt( QgsVertexId( 0, 0, 1 ) ), QgsPoint( 11, 12 ) );

  //adjacent vertices
  QgsLineString vertexLine1;
  vertexLine1.setPoints( QgsPointSequence() << QgsPoint( 1, 2 ) << QgsPoint( 11, 12 ) << QgsPoint( 111, 112 ) );
  QgsVertexId prev( 1, 2, 3 ); // start with something
  QgsVertexId next( 4, 5, 6 );
  vertexLine1.adjacentVertices( QgsVertexId( 0, 0, -1 ), prev, next );
  QCOMPARE( prev, QgsVertexId() );
  QCOMPARE( next, QgsVertexId() );
  vertexLine1.adjacentVertices( QgsVertexId( 0, 0, 3 ), prev, next );
  QCOMPARE( prev, QgsVertexId() );
  QCOMPARE( next, QgsVertexId() );
  vertexLine1.adjacentVertices( QgsVertexId( 0, 0, 0 ), prev, next );
  QCOMPARE( prev, QgsVertexId() );
  QCOMPARE( next, QgsVertexId( 0, 0, 1 ) );
  vertexLine1.adjacentVertices( QgsVertexId( 0, 0, 1 ), prev, next );
  QCOMPARE( prev, QgsVertexId( 0, 0, 0 ) );
  QCOMPARE( next, QgsVertexId( 0, 0, 2 ) );
  vertexLine1.adjacentVertices( QgsVertexId( 0, 0, 2 ), prev, next );
  QCOMPARE( prev, QgsVertexId( 0, 0, 1 ) );
  QCOMPARE( next, QgsVertexId() );
  // ring, part should be maintained
  vertexLine1.adjacentVertices( QgsVertexId( 1, 0, 1 ), prev, next );
  QCOMPARE( prev, QgsVertexId( 1, 0, 0 ) );
  QCOMPARE( next, QgsVertexId( 1, 0, 2 ) );
  vertexLine1.adjacentVertices( QgsVertexId( 1, 2, 1 ), prev, next );
  QCOMPARE( prev, QgsVertexId( 1, 2, 0 ) );
  QCOMPARE( next, QgsVertexId( 1, 2, 2 ) );
  // closed ring
  vertexLine1.close();
  vertexLine1.adjacentVertices( QgsVertexId( 0, 0, 0 ), prev, next );
  QCOMPARE( prev, QgsVertexId() );
  QCOMPARE( next, QgsVertexId( 0, 0, 1 ) );
  vertexLine1.adjacentVertices( QgsVertexId( 0, 0, 3 ), prev, next );
  QCOMPARE( prev, QgsVertexId( 0, 0, 2 ) );
  QCOMPARE( next, QgsVertexId() );

  // vertex number
  QgsLineString vertexLine2;
  QCOMPARE( vertexLine2.vertexNumberFromVertexId( QgsVertexId( -1, 0, 0 ) ), -1 );
  QCOMPARE( vertexLine2.vertexNumberFromVertexId( QgsVertexId( 0, 0, 0 ) ), -1 );
  QCOMPARE( vertexLine2.vertexNumberFromVertexId( QgsVertexId( 1, 0, 0 ) ), -1 );
  vertexLine2.setPoints( QgsPointSequence() << QgsPoint( 1, 2 ) << QgsPoint( 11, 12 ) << QgsPoint( 111, 112 ) );
  QCOMPARE( vertexLine2.vertexNumberFromVertexId( QgsVertexId( -1, 0, 0 ) ), -1 );
  QCOMPARE( vertexLine2.vertexNumberFromVertexId( QgsVertexId( 1, 0, 0 ) ), -1 );
  QCOMPARE( vertexLine2.vertexNumberFromVertexId( QgsVertexId( 0, -1, 0 ) ), -1 );
  QCOMPARE( vertexLine2.vertexNumberFromVertexId( QgsVertexId( 0, 1, 0 ) ), -1 );
  QCOMPARE( vertexLine2.vertexNumberFromVertexId( QgsVertexId( 0, 0, -1 ) ), -1 );
  QCOMPARE( vertexLine2.vertexNumberFromVertexId( QgsVertexId( 0, 0, 0 ) ), 0 );
  QCOMPARE( vertexLine2.vertexNumberFromVertexId( QgsVertexId( 0, 0, 1 ) ), 1 );
  QCOMPARE( vertexLine2.vertexNumberFromVertexId( QgsVertexId( 0, 0, 2 ) ), 2 );
  QCOMPARE( vertexLine2.vertexNumberFromVertexId( QgsVertexId( 0, 0, 3 ) ), -1 );

  //segmentLength
  QgsLineString vertexLine3;
  QCOMPARE( vertexLine3.segmentLength( QgsVertexId( -1, 0, 0 ) ), 0.0 );
  QCOMPARE( vertexLine3.segmentLength( QgsVertexId( 0, 0, 0 ) ), 0.0 );
  QCOMPARE( vertexLine3.segmentLength( QgsVertexId( 1, 0, 0 ) ), 0.0 );
  vertexLine3.setPoints( QgsPointSequence() << QgsPoint( 11, 2 ) << QgsPoint( 11, 12 ) << QgsPoint( 111, 12 ) );
  QCOMPARE( vertexLine3.segmentLength( QgsVertexId() ), 0.0 );
  QCOMPARE( vertexLine3.segmentLength( QgsVertexId( 0, 0, -1 ) ), 0.0 );
  QCOMPARE( vertexLine3.segmentLength( QgsVertexId( 0, 0, 0 ) ), 10.0 );
  QCOMPARE( vertexLine3.segmentLength( QgsVertexId( 0, 0, 1 ) ), 100.0 );
  QCOMPARE( vertexLine3.segmentLength( QgsVertexId( 0, 0, 2 ) ), 0.0 );
  QCOMPARE( vertexLine3.segmentLength( QgsVertexId( -1, 0, -1 ) ), 0.0 );
  QCOMPARE( vertexLine3.segmentLength( QgsVertexId( -1, 0, 0 ) ), 10.0 );
  QCOMPARE( vertexLine3.segmentLength( QgsVertexId( 1, 0, 1 ) ), 100.0 );
  QCOMPARE( vertexLine3.segmentLength( QgsVertexId( 1, 1, 1 ) ), 100.0 );

  //removeDuplicateNodes
  QgsLineString nodeLine;
  QVERIFY( !nodeLine.removeDuplicateNodes() );
  nodeLine.setPoints( QgsPointSequence() << QgsPoint( 11, 2 ) << QgsPoint( 11, 12 ) << QgsPoint( 111, 12 ) );
  QVERIFY( !nodeLine.removeDuplicateNodes() );
  QCOMPARE( nodeLine.asWkt(), QStringLiteral( "LineString (11 2, 11 12, 111 12)" ) );
  nodeLine.setPoints( QgsPointSequence() << QgsPoint( 11, 2 ) << QgsPoint( 11.01, 1.99 ) << QgsPoint( 11.02, 2.01 )
                      << QgsPoint( 11, 12 ) << QgsPoint( 111, 12 ) << QgsPoint( 111.01, 11.99 ) );
  QVERIFY( nodeLine.removeDuplicateNodes( 0.02 ) );
  QCOMPARE( nodeLine.asWkt(), QStringLiteral( "LineString (11 2, 11 12, 111 12)" ) );
  nodeLine.setPoints( QgsPointSequence() << QgsPoint( 11, 2 ) << QgsPoint( 11.01, 1.99 ) << QgsPoint( 11.02, 2.01 )
                      << QgsPoint( 11, 12 ) << QgsPoint( 111, 12 ) << QgsPoint( 111.01, 11.99 ) );
  QVERIFY( !nodeLine.removeDuplicateNodes() );
  QCOMPARE( nodeLine.asWkt( 2 ), QStringLiteral( "LineString (11 2, 11.01 1.99, 11.02 2.01, 11 12, 111 12, 111.01 11.99)" ) );
  // don't create degenerate lines
  nodeLine.setPoints( QgsPointSequence() << QgsPoint( 11, 2 ) );
  QVERIFY( !nodeLine.removeDuplicateNodes( 0.02 ) );
  QCOMPARE( nodeLine.asWkt( 2 ), QStringLiteral( "LineString (11 2)" ) );
  nodeLine.setPoints( QgsPointSequence() << QgsPoint( 11, 2 ) << QgsPoint( 11.01, 1.99 ) );
  QVERIFY( !nodeLine.removeDuplicateNodes( 0.02 ) );
  QCOMPARE( nodeLine.asWkt( 2 ), QStringLiteral( "LineString (11 2, 11.01 1.99)" ) );
  // with z
  nodeLine.setPoints( QgsPointSequence() << QgsPoint( 11, 2, 1 ) << QgsPoint( 11.01, 1.99, 2 ) << QgsPoint( 11.02, 2.01, 3 )
                      << QgsPoint( 11, 12, 4 ) << QgsPoint( 111, 12, 5 ) << QgsPoint( 111.01, 11.99, 6 ) );
  QVERIFY( nodeLine.removeDuplicateNodes( 0.02 ) );
  QCOMPARE( nodeLine.asWkt(), QStringLiteral( "LineStringZ (11 2 1, 11 12 4, 111 12 5)" ) );
  nodeLine.setPoints( QgsPointSequence() << QgsPoint( 11, 2, 1 ) << QgsPoint( 11.01, 1.99, 2 ) << QgsPoint( 11.02, 2.01, 3 )
                      << QgsPoint( 11, 12, 4 ) << QgsPoint( 111, 12, 5 ) << QgsPoint( 111.01, 11.99, 6 ) );
  QVERIFY( !nodeLine.removeDuplicateNodes( 0.02, true ) );
  QCOMPARE( nodeLine.asWkt( 2 ), QStringLiteral( "LineStringZ (11 2 1, 11.01 1.99 2, 11.02 2.01 3, 11 12 4, 111 12 5, 111.01 11.99 6)" ) );

  // swap xy
  QgsLineString swapLine;
  swapLine.swapXy(); // no crash
  swapLine.setPoints( QgsPointSequence() << QgsPoint( 11, 2, 3, 4, QgsWkbTypes::PointZM ) << QgsPoint( 11, 12, 13, 14, QgsWkbTypes::PointZM ) << QgsPoint( 111, 12, 23, 24, QgsWkbTypes::PointZM ) );
  swapLine.swapXy();
  QCOMPARE( swapLine.asWkt( 2 ), QStringLiteral( "LineStringZM (2 11 3 4, 12 11 13 14, 12 111 23 24)" ) );

  // filter vertex
  QgsLineString filterLine;
  auto filter = []( const QgsPoint & point )-> bool
  {
    return point.x() < 5;
  };
  filterLine.filterVertices( filter ); // no crash
  filterLine.setPoints( QgsPointSequence()  << QgsPoint( 11, 2, 3, 4, QgsWkbTypes::PointZM ) << QgsPoint( 1, 2, 3, 4, QgsWkbTypes::PointZM ) << QgsPoint( 4, 12, 13, 14, QgsWkbTypes::PointZM ) << QgsPoint( 111, 12, 23, 24, QgsWkbTypes::PointZM ) );
  filterLine.filterVertices( filter );
  QCOMPARE( filterLine.asWkt( 2 ), QStringLiteral( "LineStringZM (1 2 3 4, 4 12 13 14)" ) );

  // transform vertex
  QgsLineString transformLine;
  auto transform = []( const QgsPoint & point )-> QgsPoint
  {
    return QgsPoint( point.x() + 5, point.y() + 6, point.z() + 7, point.m() + 8 );
  };
  transformLine.transformVertices( transform ); // no crash
  transformLine.setPoints( QgsPointSequence()  << QgsPoint( 11, 2, 3, 4, QgsWkbTypes::PointZM ) << QgsPoint( 1, 2, 3, 4, QgsWkbTypes::PointZM ) << QgsPoint( 4, 12, 13, 14, QgsWkbTypes::PointZM ) << QgsPoint( 111, 12, 23, 24, QgsWkbTypes::PointZM ) );
  transformLine.transformVertices( transform );
  QCOMPARE( transformLine.asWkt( 2 ), QStringLiteral( "LineStringZM (16 8 10 12, 6 8 10 12, 9 18 20 22, 116 18 30 32)" ) );

  // substring
  QgsLineString substring;
  std::unique_ptr< QgsLineString > substringResult( substring.curveSubstring( 1, 2 ) ); // no crash
  QVERIFY( substringResult.get() );
  QVERIFY( substringResult->isEmpty() );
  substring.setPoints( QgsPointSequence() << QgsPoint( 11, 2, 3, 4, QgsWkbTypes::PointZM ) << QgsPoint( 11, 12, 13, 14, QgsWkbTypes::PointZM ) << QgsPoint( 111, 12, 23, 24, QgsWkbTypes::PointZM ) );
  substringResult.reset( substring.curveSubstring( 0, 0 ) );
  QCOMPARE( substringResult->asWkt( 2 ), QStringLiteral( "LineStringZM (11 2 3 4, 11 2 3 4)" ) );
  substringResult.reset( substring.curveSubstring( -1, -0.1 ) );
  QVERIFY( substringResult->isEmpty() );
  substringResult.reset( substring.curveSubstring( 100000, 10000 ) );
  QVERIFY( substringResult->isEmpty() );
  substringResult.reset( substring.curveSubstring( -1, 1 ) );
  QCOMPARE( substringResult->asWkt( 2 ), QStringLiteral( "LineStringZM (11 2 3 4, 11 3 4 5)" ) );
  substringResult.reset( substring.curveSubstring( 1, -1 ) );
  QCOMPARE( substringResult->asWkt( 2 ), QStringLiteral( "LineStringZM (11 3 4 5, 11 3 4 5)" ) );
  substringResult.reset( substring.curveSubstring( -1, 10000 ) );
  QCOMPARE( substringResult->asWkt( 2 ), QStringLiteral( "LineStringZM (11 2 3 4, 11 12 13 14, 111 12 23 24)" ) );
  substringResult.reset( substring.curveSubstring( 1, 10000 ) );
  QCOMPARE( substringResult->asWkt( 2 ), QStringLiteral( "LineStringZM (11 3 4 5, 11 12 13 14, 111 12 23 24)" ) );
  substringResult.reset( substring.curveSubstring( 1, 20 ) );
  QCOMPARE( substringResult->asWkt( 2 ), QStringLiteral( "LineStringZM (11 3 4 5, 11 12 13 14, 21 12 14 15)" ) );
  substring.setPoints( QgsPointSequence() << QgsPoint( 11, 2, 3, 0, QgsWkbTypes::PointZ ) << QgsPoint( 11, 12, 13, 0, QgsWkbTypes::PointZ ) << QgsPoint( 111, 12, 23, 0, QgsWkbTypes::PointZ ) );
  substringResult.reset( substring.curveSubstring( 1, 20 ) );
  QCOMPARE( substringResult->asWkt( 2 ), QStringLiteral( "LineStringZ (11 3 4, 11 12 13, 21 12 14)" ) );
  substring.setPoints( QgsPointSequence() << QgsPoint( 11, 2, 0, 3, QgsWkbTypes::PointM ) << QgsPoint( 11, 12, 0, 13, QgsWkbTypes::PointM ) << QgsPoint( 111, 12, 0, 23, QgsWkbTypes::PointM ) );
  substringResult.reset( substring.curveSubstring( 1, 20 ) );
  QCOMPARE( substringResult->asWkt( 2 ), QStringLiteral( "LineStringM (11 3 4, 11 12 13, 21 12 14)" ) );
  substring.setPoints( QgsPointSequence() << QgsPoint( 11, 2 ) << QgsPoint( 11, 12 ) << QgsPoint( 111, 12 ) );
  substringResult.reset( substring.curveSubstring( 1, 20 ) );
  QCOMPARE( substringResult->asWkt( 2 ), QStringLiteral( "LineString (11 3, 11 12, 21 12)" ) );

  //interpolate point
  QgsLineString interpolate;
  std::unique_ptr< QgsPoint > interpolateResult( interpolate.interpolatePoint( 1 ) ); // no crash
  QVERIFY( !interpolateResult.get() );
  interpolate.setPoints( QgsPointSequence() << QgsPoint( 11, 2, 3, 4, QgsWkbTypes::PointZM ) << QgsPoint( 11, 12, 13, 14, QgsWkbTypes::PointZM ) << QgsPoint( 111, 12, 23, 24, QgsWkbTypes::PointZM ) );
  interpolateResult.reset( interpolate.interpolatePoint( 0 ) );
  QCOMPARE( interpolateResult->asWkt( 2 ), QStringLiteral( "PointZM (11 2 3 4)" ) );
  interpolateResult.reset( interpolate.interpolatePoint( -1 ) );
  QVERIFY( !interpolateResult.get() );
  interpolateResult.reset( interpolate.interpolatePoint( 100000 ) );
  QVERIFY( !interpolateResult.get() );
  interpolateResult.reset( interpolate.interpolatePoint( 1 ) );
  QCOMPARE( interpolateResult->asWkt( 2 ), QStringLiteral( "PointZM (11 3 4 5)" ) );
  interpolateResult.reset( interpolate.interpolatePoint( 20 ) );
  QCOMPARE( interpolateResult->asWkt( 2 ), QStringLiteral( "PointZM (21 12 14 15)" ) );
  interpolateResult.reset( interpolate.interpolatePoint( 110 ) );
  QCOMPARE( interpolateResult->asWkt( 2 ), QStringLiteral( "PointZM (111 12 23 24)" ) );
  interpolate.setPoints( QgsPointSequence() << QgsPoint( 11, 2, 3, 0, QgsWkbTypes::PointZ ) << QgsPoint( 11, 12, 13, 0, QgsWkbTypes::PointZ ) << QgsPoint( 111, 12, 23, 0, QgsWkbTypes::PointZ ) );
  interpolateResult.reset( interpolate.interpolatePoint( 1 ) );
  QCOMPARE( interpolateResult->asWkt( 2 ), QStringLiteral( "PointZ (11 3 4)" ) );
  interpolate.setPoints( QgsPointSequence() << QgsPoint( 11, 2, 0, 3, QgsWkbTypes::PointM ) << QgsPoint( 11, 12, 0, 13, QgsWkbTypes::PointM ) << QgsPoint( 111, 12, 0, 23, QgsWkbTypes::PointM ) );
  interpolateResult.reset( interpolate.interpolatePoint( 1 ) );
  QCOMPARE( interpolateResult->asWkt( 2 ), QStringLiteral( "PointM (11 3 4)" ) );
  interpolate.setPoints( QgsPointSequence() << QgsPoint( 11, 2 ) << QgsPoint( 11, 12 ) << QgsPoint( 111, 12 ) );
  interpolateResult.reset( interpolate.interpolatePoint( 1 ) );
  QCOMPARE( interpolateResult->asWkt( 2 ), QStringLiteral( "Point (11 3)" ) );

  // orientation
  QgsLineString orientation;
  ( void )orientation.orientation(); // no crash
  orientation.setPoints( QgsPointSequence() << QgsPoint( 0, 0 ) << QgsPoint( 0, 1 ) << QgsPoint( 1, 1 ) << QgsPoint( 1, 0 ) << QgsPoint( 0, 0 ) );
  QCOMPARE( orientation.orientation(), QgsCurve::Clockwise );
  orientation.setPoints( QgsPointSequence() << QgsPoint( 0, 0 ) << QgsPoint( 1, 0 ) << QgsPoint( 1, 1 ) << QgsPoint( 0, 1 ) << QgsPoint( 0, 0 ) );
  QCOMPARE( orientation.orientation(), QgsCurve::CounterClockwise );
}

void TestQgsGeometry::polygon()
{
  //test constructor
  QgsPolygon p1;
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
  QgsLineString *ext = nullptr;
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
  QgsPolygon p2;
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
  QgsPolygon p3;
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
  QgsPolygon p4;
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
  QgsPolygon p5;
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
  QgsPolygon p6;
  ext = new QgsLineString();
  ext->setPoints( QgsPointSequence() << QgsPoint( 0, 0 ) << QgsPoint( 0, 10 ) << QgsPoint( 10, 10 )
                  << QgsPoint( 10, 0 ) << QgsPoint( 0, 0 ) );
  p6.setExteriorRing( ext );
  //empty ring
  QCOMPARE( p6.numInteriorRings(), 0 );
  QVERIFY( !p6.interiorRing( -1 ) );
  QVERIFY( !p6.interiorRing( 0 ) );
  p6.addInteriorRing( nullptr );
  QCOMPARE( p6.numInteriorRings(), 0 );
  QgsLineString *ring = new QgsLineString();
  ring->setPoints( QgsPointSequence() << QgsPoint( 1, 1 ) << QgsPoint( 1, 9 ) << QgsPoint( 9, 9 )
                   << QgsPoint( 9, 1 ) << QgsPoint( 1, 1 ) );
  p6.addInteriorRing( ring );
  QCOMPARE( p6.numInteriorRings(), 1 );
  QCOMPARE( p6.interiorRing( 0 ), ring );
  QVERIFY( !p6.interiorRing( 1 ) );

  QgsCoordinateSequence seq = p6.coordinateSequence();
  QCOMPARE( seq, QgsCoordinateSequence() << ( QgsRingSequence() << ( QgsPointSequence() << QgsPoint( 0, 0 ) << QgsPoint( 0, 10 ) << QgsPoint( 10, 10 )
            << QgsPoint( 10, 0 ) << QgsPoint( 0, 0 ) )
            << ( QgsPointSequence() << QgsPoint( 1, 1 ) << QgsPoint( 1, 9 ) << QgsPoint( 9, 9 )
                 << QgsPoint( 9, 1 ) << QgsPoint( 1, 1 ) ) ) );
  QCOMPARE( p6.nCoordinates(), 10 );

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
  QgsPolygon p6b;
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
  QgsPolygon p6c;
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
  QgsPolygon p7;
  ext = new QgsLineString();
  ext->setPoints( QgsPointSequence() << QgsPoint( 0, 0 ) << QgsPoint( 0, 10 ) << QgsPoint( 10, 10 )
                  << QgsPoint( 10, 0 ) << QgsPoint( 0, 0 ) );
  p7.setExteriorRing( ext );
  //add a list of rings with mixed types
  QVector< QgsCurve * > rings;
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
  QgsPolygon p7a;
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
  QgsPolygon p8;
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
  QgsPolygon p9;
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
  QgsPolygon p10;
  QgsPolygon p10b;
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

  QgsLineString nonPolygon;
  QVERIFY( p10 != nonPolygon );
  QVERIFY( !( p10 == nonPolygon ) );

  //clone

  QgsPolygon p11;
  std::unique_ptr< QgsPolygon >cloned( p11.clone() );
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
  QgsPolygon p12;
  QgsPolygon p13( p12 );
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
  QgsPolygon p14( p12 );
  QCOMPARE( p12, p14 );

  //assignment operator
  QgsPolygon p15;
  p15 = p13;
  QCOMPARE( p13, p15 );
  p15 = p12;
  QCOMPARE( p12, p15 );

  //surfaceToPolygon - should be identical given polygon has no curves
  std::unique_ptr< QgsPolygon > surface( p12.surfaceToPolygon() );
  QCOMPARE( *surface, p12 );
  //toPolygon - should be identical given polygon has no curves
  std::unique_ptr< QgsPolygon > toP( p12.toPolygon() );
  QCOMPARE( *toP, p12 );

  //toCurveType
  std::unique_ptr< QgsCurvePolygon > curveType( p12.toCurveType() );
  QCOMPARE( curveType->wkbType(), QgsWkbTypes::CurvePolygonZM );
  QCOMPARE( curveType->exteriorRing()->numPoints(), 5 );
  QCOMPARE( curveType->exteriorRing()->vertexAt( QgsVertexId( 0, 0, 0 ) ), QgsPoint( QgsWkbTypes::PointZM, 0, 0, 1, 5 ) );
  QCOMPARE( curveType->exteriorRing()->vertexAt( QgsVertexId( 0, 0, 1 ) ), QgsPoint( QgsWkbTypes::PointZM, 0, 10, 2, 6 ) );
  QCOMPARE( curveType->exteriorRing()->vertexAt( QgsVertexId( 0, 0, 2 ) ), QgsPoint( QgsWkbTypes::PointZM, 10, 10, 3, 7 ) );
  QCOMPARE( curveType->exteriorRing()->vertexAt( QgsVertexId( 0, 0, 3 ) ), QgsPoint( QgsWkbTypes::PointZM, 10, 0, 4, 8 ) );
  QCOMPARE( curveType->exteriorRing()->vertexAt( QgsVertexId( 0, 0, 4 ) ), QgsPoint( QgsWkbTypes::PointZM, 0, 0, 1, 9 ) );
  QCOMPARE( curveType->numInteriorRings(), 1 );
  QCOMPARE( curveType->interiorRing( 0 )->numPoints(), 5 );
  QCOMPARE( curveType->interiorRing( 0 )->vertexAt( QgsVertexId( 0, 0, 0 ) ), QgsPoint( QgsWkbTypes::PointZM, 1, 1, 1, 2 ) );
  QCOMPARE( curveType->interiorRing( 0 )->vertexAt( QgsVertexId( 0, 0, 1 ) ), QgsPoint( QgsWkbTypes::PointZM, 1, 9, 2, 3 ) );
  QCOMPARE( curveType->interiorRing( 0 )->vertexAt( QgsVertexId( 0, 0, 2 ) ), QgsPoint( QgsWkbTypes::PointZM, 9, 9, 3, 6 ) );
  QCOMPARE( curveType->interiorRing( 0 )->vertexAt( QgsVertexId( 0, 0, 3 ) ), QgsPoint( QgsWkbTypes::PointZM, 9, 1, 4, 4 ) );
  QCOMPARE( curveType->interiorRing( 0 )->vertexAt( QgsVertexId( 0, 0, 4 ) ), QgsPoint( QgsWkbTypes::PointZM, 1, 1, 1, 7 ) );

  //to/fromWKB
  QgsPolygon p16;
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
  QgsPolygon p17;
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
  QgsPolygon p18;
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
  QgsPolygon p19;
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
  QgsPolygon exportPolygon;
  ext = new QgsLineString();
  ext->setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::Point, 0, 0 )
                  << QgsPoint( QgsWkbTypes::Point, 0, 10 ) << QgsPoint( QgsWkbTypes::Point, 10, 10 )
                  << QgsPoint( QgsWkbTypes::Point, 10, 0 ) << QgsPoint( QgsWkbTypes::Point, 0, 0 ) );
  exportPolygon.setExteriorRing( ext );

  // GML document for compare
  QDomDocument doc( QStringLiteral( "gml" ) );

  // as GML2
  QString expectedSimpleGML2( QStringLiteral( "<Polygon xmlns=\"gml\"><outerBoundaryIs xmlns=\"gml\"><LinearRing xmlns=\"gml\"><coordinates xmlns=\"gml\" cs=\",\" ts=\" \">0,0 0,10 10,10 10,0 0,0</coordinates></LinearRing></outerBoundaryIs></Polygon>" ) );
  QGSCOMPAREGML( elemToString( exportPolygon.asGml2( doc ) ), expectedSimpleGML2 );
  QString expectedGML2empty( QStringLiteral( "<Polygon xmlns=\"gml\"/>" ) );
  QGSCOMPAREGML( elemToString( QgsPolygon().asGml2( doc ) ), expectedGML2empty );

  //as GML3
  QString expectedSimpleGML3( QStringLiteral( "<Polygon xmlns=\"gml\"><exterior xmlns=\"gml\"><LinearRing xmlns=\"gml\"><posList xmlns=\"gml\" srsDimension=\"2\">0 0 0 10 10 10 10 0 0 0</posList></LinearRing></exterior></Polygon>" ) );
  QCOMPARE( elemToString( exportPolygon.asGml3( doc ) ), expectedSimpleGML3 );
  QString expectedGML3empty( QStringLiteral( "<Polygon xmlns=\"gml\"/>" ) );
  QGSCOMPAREGML( elemToString( QgsPolygon().asGml3( doc ) ), expectedGML3empty );

  // as JSON
  QString expectedSimpleJson( QStringLiteral( "{\"type\": \"Polygon\", \"coordinates\": [[ [0, 0], [0, 10], [10, 10], [10, 0], [0, 0]]] }" ) );
  QCOMPARE( exportPolygon.asJson(), expectedSimpleJson );

  ring = new QgsLineString();
  ring->setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::Point, 1, 1 )
                   << QgsPoint( QgsWkbTypes::Point, 1, 9 ) << QgsPoint( QgsWkbTypes::Point, 9, 9 )
                   << QgsPoint( QgsWkbTypes::Point, 9, 1 ) << QgsPoint( QgsWkbTypes::Point, 1, 1 ) );
  exportPolygon.addInteriorRing( ring );

  QString expectedJson( QStringLiteral( "{\"type\": \"Polygon\", \"coordinates\": [[ [0, 0], [0, 10], [10, 10], [10, 0], [0, 0]], [ [1, 1], [1, 9], [9, 9], [9, 1], [1, 1]]] }" ) );
  QCOMPARE( exportPolygon.asJson(), expectedJson );

  QgsPolygon exportPolygonFloat;
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
  QCOMPARE( exportPolygonFloat.asJson( 3 ), expectedJsonPrec3 );

  // as GML2
  QString expectedGML2( QStringLiteral( "<Polygon xmlns=\"gml\"><outerBoundaryIs xmlns=\"gml\"><LinearRing xmlns=\"gml\"><coordinates xmlns=\"gml\" cs=\",\" ts=\" \">0,0 0,10 10,10 10,0 0,0</coordinates></LinearRing></outerBoundaryIs>" ) );
  expectedGML2 += QStringLiteral( "<innerBoundaryIs xmlns=\"gml\"><LinearRing xmlns=\"gml\"><coordinates xmlns=\"gml\" cs=\",\" ts=\" \">1,1 1,9 9,9 9,1 1,1</coordinates></LinearRing></innerBoundaryIs></Polygon>" );
  QGSCOMPAREGML( elemToString( exportPolygon.asGml2( doc ) ), expectedGML2 );
  QString expectedGML2prec3( QStringLiteral( "<Polygon xmlns=\"gml\"><outerBoundaryIs xmlns=\"gml\"><LinearRing xmlns=\"gml\"><coordinates xmlns=\"gml\" cs=\",\" ts=\" \">1.111,1.111 1.111,11.111 11.111,11.111 11.111,1.111 1.111,1.111</coordinates></LinearRing></outerBoundaryIs>" ) );
  expectedGML2prec3 += QStringLiteral( "<innerBoundaryIs xmlns=\"gml\"><LinearRing xmlns=\"gml\"><coordinates xmlns=\"gml\" cs=\",\" ts=\" \">0.667,0.667 0.667,1.333 1.333,1.333 1.333,0.667 0.667,0.667</coordinates></LinearRing></innerBoundaryIs></Polygon>" );
  QGSCOMPAREGML( elemToString( exportPolygonFloat.asGml2( doc, 3 ) ), expectedGML2prec3 );

  //as GML3
  QString expectedGML3( QStringLiteral( "<Polygon xmlns=\"gml\"><exterior xmlns=\"gml\"><LinearRing xmlns=\"gml\"><posList xmlns=\"gml\" srsDimension=\"2\">0 0 0 10 10 10 10 0 0 0</posList></LinearRing></exterior>" ) );
  expectedGML3 += QStringLiteral( "<interior xmlns=\"gml\"><LinearRing xmlns=\"gml\"><posList xmlns=\"gml\" srsDimension=\"2\">1 1 1 9 9 9 9 1 1 1</posList></LinearRing></interior></Polygon>" );

  QCOMPARE( elemToString( exportPolygon.asGml3( doc ) ), expectedGML3 );
  QString expectedGML3prec3( QStringLiteral( "<Polygon xmlns=\"gml\"><exterior xmlns=\"gml\"><LinearRing xmlns=\"gml\"><posList xmlns=\"gml\" srsDimension=\"2\">1.111 1.111 1.111 11.111 11.111 11.111 11.111 1.111 1.111 1.111</posList></LinearRing></exterior>" ) );
  expectedGML3prec3 += QStringLiteral( "<interior xmlns=\"gml\"><LinearRing xmlns=\"gml\"><posList xmlns=\"gml\" srsDimension=\"2\">0.667 0.667 0.667 1.333 1.333 1.333 1.333 0.667 0.667 0.667</posList></LinearRing></interior></Polygon>" );
  QCOMPARE( elemToString( exportPolygonFloat.asGml3( doc, 3 ) ), expectedGML3prec3 );

  //removing the fourth to last vertex removes the whole ring
  QgsPolygon p20;
  QgsLineString *p20ExteriorRing = new QgsLineString();
  p20ExteriorRing->setPoints( QVector<QgsPoint>() << QgsPoint( 0, 0 ) << QgsPoint( 1, 0 ) << QgsPoint( 1, 1 ) << QgsPoint( 0, 0 ) );
  p20.setExteriorRing( p20ExteriorRing );
  QVERIFY( p20.exteriorRing() );
  p20.deleteVertex( QgsVertexId( 0, 0, 2 ) );
  QVERIFY( !p20.exteriorRing() );

  //boundary
  QgsLineString boundary1;
  boundary1.setPoints( QVector<QgsPoint>() << QgsPoint( 0, 0 ) << QgsPoint( 1, 0 ) << QgsPoint( 1, 1 )  << QgsPoint( 0, 0 ) );
  QgsPolygon boundaryPolygon;
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
  boundaryRing1.setPoints( QVector<QgsPoint>() << QgsPoint( 0.1, 0.1 ) << QgsPoint( 0.2, 0.1 ) << QgsPoint( 0.2, 0.2 )  << QgsPoint( 0.1, 0.1 ) );
  QgsLineString boundaryRing2;
  boundaryRing2.setPoints( QVector<QgsPoint>() << QgsPoint( 0.8, 0.8 ) << QgsPoint( 0.9, 0.8 ) << QgsPoint( 0.9, 0.9 )  << QgsPoint( 0.8, 0.8 ) );
  boundaryPolygon.setInteriorRings( QVector< QgsCurve * >() << boundaryRing1.clone() << boundaryRing2.clone() );
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
  boundaryPolygon.setInteriorRings( QVector< QgsCurve * >() );
  delete boundary;

  //test boundary with z
  boundary1.setPoints( QVector<QgsPoint>() << QgsPoint( QgsWkbTypes::PointZ, 0, 0, 10 ) << QgsPoint( QgsWkbTypes::PointZ, 1, 0, 15 )
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
  pd1.setPoints( QVector<QgsPoint>() << QgsPoint( 0, 0 ) << QgsPoint( 1, 0 ) << QgsPoint( 1, 1 ) << QgsPoint( 0, 1 ) << QgsPoint( 0, 0 ) );
  QgsPolygon pd;
  // no meaning, but let's not crash
  ( void )pd.pointDistanceToBoundary( 0, 0 );

  pd.setExteriorRing( pd1.clone() );
  QGSCOMPARENEAR( pd.pointDistanceToBoundary( 0, 0.5 ), 0.0, 0.0000000001 );
  QGSCOMPARENEAR( pd.pointDistanceToBoundary( 0.1, 0.5 ), 0.1, 0.0000000001 );
  QGSCOMPARENEAR( pd.pointDistanceToBoundary( -0.1, 0.5 ), -0.1, 0.0000000001 );
  // with a ring
  QgsLineString pdRing1;
  pdRing1.setPoints( QVector<QgsPoint>() << QgsPoint( 0.1, 0.1 ) << QgsPoint( 0.2, 0.1 ) << QgsPoint( 0.2, 0.6 )  << QgsPoint( 0.1, 0.6 ) << QgsPoint( 0.1, 0.1 ) );
  pd.setInteriorRings( QVector< QgsCurve * >() << pdRing1.clone() );
  QGSCOMPARENEAR( pd.pointDistanceToBoundary( 0, 0.5 ), 0.0, 0.0000000001 );
  QGSCOMPARENEAR( pd.pointDistanceToBoundary( 0.1, 0.5 ), 0.0, 0.0000000001 );
  QGSCOMPARENEAR( pd.pointDistanceToBoundary( 0.01, 0.5 ), 0.01, 0.0000000001 );
  QGSCOMPARENEAR( pd.pointDistanceToBoundary( 0.08, 0.5 ), 0.02, 0.0000000001 );
  QGSCOMPARENEAR( pd.pointDistanceToBoundary( 0.12, 0.5 ), -0.02, 0.0000000001 );
  QGSCOMPARENEAR( pd.pointDistanceToBoundary( -0.1, 0.5 ), -0.1, 0.0000000001 );

  // remove interior rings
  QgsLineString removeRingsExt;
  removeRingsExt.setPoints( QVector<QgsPoint>() << QgsPoint( 0, 0 ) << QgsPoint( 1, 0 ) << QgsPoint( 1, 1 )  << QgsPoint( 0, 0 ) );
  QgsPolygon removeRings1;
  removeRings1.removeInteriorRings();

  removeRings1.setExteriorRing( boundary1.clone() );
  removeRings1.removeInteriorRings();
  QCOMPARE( removeRings1.numInteriorRings(), 0 );

  // add interior rings
  QgsLineString removeRingsRing1;
  removeRingsRing1.setPoints( QVector<QgsPoint>() << QgsPoint( 0.1, 0.1 ) << QgsPoint( 0.2, 0.1 ) << QgsPoint( 0.2, 0.2 )  << QgsPoint( 0.1, 0.1 ) );
  QgsLineString removeRingsRing2;
  removeRingsRing2.setPoints( QVector<QgsPoint>() << QgsPoint( 0.6, 0.8 ) << QgsPoint( 0.9, 0.8 ) << QgsPoint( 0.9, 0.9 )  << QgsPoint( 0.6, 0.8 ) );
  removeRings1.setInteriorRings( QVector< QgsCurve * >() << removeRingsRing1.clone() << removeRingsRing2.clone() );

  // remove ring with size filter
  removeRings1.removeInteriorRings( 0.0075 );
  QCOMPARE( removeRings1.numInteriorRings(), 1 );

  // remove ring with no size filter
  removeRings1.removeInteriorRings();
  QCOMPARE( removeRings1.numInteriorRings(), 0 );

  // cast
  QVERIFY( !QgsPolygon().cast( nullptr ) );
  QgsPolygon pCast;
  QVERIFY( QgsPolygon().cast( &pCast ) );
  QgsPolygon pCast2;
  pCast2.fromWkt( QStringLiteral( "PolygonZ((0 0 0, 0 1 1, 1 0 2, 0 0 0))" ) );
  QVERIFY( QgsPolygon().cast( &pCast2 ) );
  pCast2.fromWkt( QStringLiteral( "PolygonM((0 0 1, 0 1 2, 1 0 3, 0 0 1))" ) );
  QVERIFY( QgsPolygon().cast( &pCast2 ) );
  pCast2.fromWkt( QStringLiteral( "PolygonZM((0 0 0 1, 0 1 1 2, 1 0 2 3, 0 0 0 1))" ) );
  QVERIFY( QgsPolygon().cast( &pCast2 ) );

  //transform
  //CRS transform
  QgsCoordinateReferenceSystem sourceSrs;
  sourceSrs.createFromSrid( 3994 );
  QgsCoordinateReferenceSystem destSrs;
  destSrs.createFromSrid( 4202 ); // want a transform with ellipsoid change
  QgsCoordinateTransform tr( sourceSrs, destSrs, QgsProject::instance() );

  // 2d CRS transform
  QgsPolygon pTransform;
  QgsLineString l21;
  l21.setPoints( QgsPointSequence() << QgsPoint( 6374985, -3626584 )
                 << QgsPoint( 6274985, -3526584 )
                 << QgsPoint( 6474985, -3526584 )
                 << QgsPoint( 6374985, -3626584 ) );
  pTransform.setExteriorRing( l21.clone() );
  pTransform.addInteriorRing( l21.clone() );
  pTransform.transform( tr, QgsCoordinateTransform::ForwardTransform );
  const QgsLineString *extR = static_cast< const QgsLineString * >( pTransform.exteriorRing() );
  QGSCOMPARENEAR( extR->pointN( 0 ).x(), 175.771, 0.001 );
  QGSCOMPARENEAR( extR->pointN( 0 ).y(), -39.724, 0.001 );
  QGSCOMPARENEAR( extR->pointN( 1 ).x(),  174.581448, 0.001 );
  QGSCOMPARENEAR( extR->pointN( 1 ).y(), -38.7999, 0.001 );
  QGSCOMPARENEAR( extR->pointN( 2 ).x(),  176.958633, 0.001 );
  QGSCOMPARENEAR( extR->pointN( 2 ).y(), -38.7999, 0.001 );
  QGSCOMPARENEAR( extR->pointN( 3 ).x(),  175.771, 0.001 );
  QGSCOMPARENEAR( extR->pointN( 3 ).y(), -39.724, 0.001 );
  QGSCOMPARENEAR( pTransform.exteriorRing()->boundingBox().xMinimum(), 174.581448, 0.001 );
  QGSCOMPARENEAR( pTransform.exteriorRing()->boundingBox().yMinimum(), -39.724, 0.001 );
  QGSCOMPARENEAR( pTransform.exteriorRing()->boundingBox().xMaximum(), 176.959, 0.001 );
  QGSCOMPARENEAR( pTransform.exteriorRing()->boundingBox().yMaximum(), -38.7999, 0.001 );
  const QgsLineString *intR = static_cast< const QgsLineString * >( pTransform.interiorRing( 0 ) );
  QGSCOMPARENEAR( intR->pointN( 0 ).x(), 175.771, 0.001 );
  QGSCOMPARENEAR( intR->pointN( 0 ).y(), -39.724, 0.001 );
  QGSCOMPARENEAR( intR->pointN( 1 ).x(),  174.581448, 0.001 );
  QGSCOMPARENEAR( intR->pointN( 1 ).y(), -38.7999, 0.001 );
  QGSCOMPARENEAR( intR->pointN( 2 ).x(),  176.958633, 0.001 );
  QGSCOMPARENEAR( intR->pointN( 2 ).y(), -38.7999, 0.001 );
  QGSCOMPARENEAR( intR->pointN( 3 ).x(),  175.771, 0.001 );
  QGSCOMPARENEAR( intR->pointN( 3 ).y(), -39.724, 0.001 );
  QGSCOMPARENEAR( pTransform.interiorRing( 0 )->boundingBox().xMinimum(), 174.581448, 0.001 );
  QGSCOMPARENEAR( pTransform.interiorRing( 0 )->boundingBox().yMinimum(), -39.724, 0.001 );
  QGSCOMPARENEAR( pTransform.interiorRing( 0 )->boundingBox().xMaximum(), 176.959, 0.001 );
  QGSCOMPARENEAR( pTransform.interiorRing( 0 )->boundingBox().yMaximum(), -38.7999, 0.001 );

  //3d CRS transform
  QgsLineString l22;
  l22.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 6374985, -3626584, 1, 2 )
                 << QgsPoint( QgsWkbTypes::PointZM, 6274985, -3526584, 3, 4 )
                 << QgsPoint( QgsWkbTypes::PointZM, 6474985, -3526584, 5, 6 )
                 << QgsPoint( QgsWkbTypes::PointZM, 6374985, -3626584, 1, 2 ) );
  pTransform.clear();
  pTransform.setExteriorRing( l22.clone() );
  pTransform.addInteriorRing( l22.clone() );
  pTransform.transform( tr, QgsCoordinateTransform::ForwardTransform );
  extR = static_cast< const QgsLineString * >( pTransform.exteriorRing() );
  QGSCOMPARENEAR( extR->pointN( 0 ).x(), 175.771, 0.001 );
  QGSCOMPARENEAR( extR->pointN( 0 ).y(), -39.724, 0.001 );
  QGSCOMPARENEAR( extR->pointN( 0 ).z(), 1.0, 0.001 );
  QGSCOMPARENEAR( extR->pointN( 0 ).m(), 2.0, 0.001 );
  QGSCOMPARENEAR( extR->pointN( 1 ).x(),  174.581448, 0.001 );
  QGSCOMPARENEAR( extR->pointN( 1 ).y(), -38.7999, 0.001 );
  QGSCOMPARENEAR( extR->pointN( 1 ).z(), 3.0, 0.001 );
  QGSCOMPARENEAR( extR->pointN( 1 ).m(), 4.0, 0.001 );
  QGSCOMPARENEAR( extR->pointN( 2 ).x(),  176.958633, 0.001 );
  QGSCOMPARENEAR( extR->pointN( 2 ).y(), -38.7999, 0.001 );
  QGSCOMPARENEAR( extR->pointN( 2 ).z(), 5.0, 0.001 );
  QGSCOMPARENEAR( extR->pointN( 2 ).m(), 6.0, 0.001 );
  QGSCOMPARENEAR( extR->pointN( 3 ).x(),  175.771, 0.001 );
  QGSCOMPARENEAR( extR->pointN( 3 ).y(), -39.724, 0.001 );
  QGSCOMPARENEAR( extR->pointN( 3 ).z(), 1.0, 0.001 );
  QGSCOMPARENEAR( extR->pointN( 3 ).m(), 2.0, 0.001 );
  QGSCOMPARENEAR( pTransform.exteriorRing()->boundingBox().xMinimum(), 174.581448, 0.001 );
  QGSCOMPARENEAR( pTransform.exteriorRing()->boundingBox().yMinimum(), -39.724, 0.001 );
  QGSCOMPARENEAR( pTransform.exteriorRing()->boundingBox().xMaximum(), 176.959, 0.001 );
  QGSCOMPARENEAR( pTransform.exteriorRing()->boundingBox().yMaximum(), -38.7999, 0.001 );
  intR = static_cast< const QgsLineString * >( pTransform.interiorRing( 0 ) );
  QGSCOMPARENEAR( intR->pointN( 0 ).x(), 175.771, 0.001 );
  QGSCOMPARENEAR( intR->pointN( 0 ).y(), -39.724, 0.001 );
  QGSCOMPARENEAR( intR->pointN( 0 ).z(), 1.0, 0.001 );
  QGSCOMPARENEAR( intR->pointN( 0 ).m(), 2.0, 0.001 );
  QGSCOMPARENEAR( intR->pointN( 1 ).x(),  174.581448, 0.001 );
  QGSCOMPARENEAR( intR->pointN( 1 ).y(), -38.7999, 0.001 );
  QGSCOMPARENEAR( intR->pointN( 1 ).z(), 3.0, 0.001 );
  QGSCOMPARENEAR( intR->pointN( 1 ).m(), 4.0, 0.001 );
  QGSCOMPARENEAR( intR->pointN( 2 ).x(),  176.958633, 0.001 );
  QGSCOMPARENEAR( intR->pointN( 2 ).y(), -38.7999, 0.001 );
  QGSCOMPARENEAR( intR->pointN( 2 ).z(), 5.0, 0.001 );
  QGSCOMPARENEAR( intR->pointN( 2 ).m(), 6.0, 0.001 );
  QGSCOMPARENEAR( intR->pointN( 3 ).x(),  175.771, 0.001 );
  QGSCOMPARENEAR( intR->pointN( 3 ).y(), -39.724, 0.001 );
  QGSCOMPARENEAR( intR->pointN( 3 ).z(), 1.0, 0.001 );
  QGSCOMPARENEAR( intR->pointN( 3 ).m(), 2.0, 0.001 );
  QGSCOMPARENEAR( pTransform.interiorRing( 0 )->boundingBox().xMinimum(), 174.581448, 0.001 );
  QGSCOMPARENEAR( pTransform.interiorRing( 0 )->boundingBox().yMinimum(), -39.724, 0.001 );
  QGSCOMPARENEAR( pTransform.interiorRing( 0 )->boundingBox().xMaximum(), 176.959, 0.001 );
  QGSCOMPARENEAR( pTransform.interiorRing( 0 )->boundingBox().yMaximum(), -38.7999, 0.001 );

  //reverse transform
  pTransform.transform( tr, QgsCoordinateTransform::ReverseTransform );
  extR = static_cast< const QgsLineString * >( pTransform.exteriorRing() );
  QGSCOMPARENEAR( extR->pointN( 0 ).x(), 6374984, 100 );
  QGSCOMPARENEAR( extR->pointN( 0 ).y(), -3626584, 100 );
  QGSCOMPARENEAR( extR->pointN( 0 ).z(), 1.0, 0.001 );
  QGSCOMPARENEAR( extR->pointN( 0 ).m(), 2.0, 0.001 );
  QGSCOMPARENEAR( extR->pointN( 1 ).x(), 6274984, 100 );
  QGSCOMPARENEAR( extR->pointN( 1 ).y(), -3526584, 100 );
  QGSCOMPARENEAR( extR->pointN( 1 ).z(), 3.0, 0.001 );
  QGSCOMPARENEAR( extR->pointN( 1 ).m(), 4.0, 0.001 );
  QGSCOMPARENEAR( extR->pointN( 2 ).x(),  6474984, 100 );
  QGSCOMPARENEAR( extR->pointN( 2 ).y(), -3526584, 100 );
  QGSCOMPARENEAR( extR->pointN( 2 ).z(), 5.0, 0.001 );
  QGSCOMPARENEAR( extR->pointN( 2 ).m(), 6.0, 0.001 );
  QGSCOMPARENEAR( extR->pointN( 3 ).x(),  6374984, 100 );
  QGSCOMPARENEAR( extR->pointN( 3 ).y(), -3626584, 100 );
  QGSCOMPARENEAR( extR->pointN( 3 ).z(), 1.0, 0.001 );
  QGSCOMPARENEAR( extR->pointN( 3 ).m(), 2.0, 0.001 );
  QGSCOMPARENEAR( pTransform.exteriorRing()->boundingBox().xMinimum(), 6274984, 100 );
  QGSCOMPARENEAR( pTransform.exteriorRing()->boundingBox().yMinimum(), -3626584, 100 );
  QGSCOMPARENEAR( pTransform.exteriorRing()->boundingBox().xMaximum(), 6474984, 100 );
  QGSCOMPARENEAR( pTransform.exteriorRing()->boundingBox().yMaximum(), -3526584, 100 );
  intR = static_cast< const QgsLineString * >( pTransform.interiorRing( 0 ) );
  QGSCOMPARENEAR( intR->pointN( 0 ).x(), 6374984, 100 );
  QGSCOMPARENEAR( intR->pointN( 0 ).y(), -3626584, 100 );
  QGSCOMPARENEAR( intR->pointN( 0 ).z(), 1.0, 0.001 );
  QGSCOMPARENEAR( intR->pointN( 0 ).m(), 2.0, 0.001 );
  QGSCOMPARENEAR( intR->pointN( 1 ).x(), 6274984, 100 );
  QGSCOMPARENEAR( intR->pointN( 1 ).y(), -3526584, 100 );
  QGSCOMPARENEAR( intR->pointN( 1 ).z(), 3.0, 0.001 );
  QGSCOMPARENEAR( intR->pointN( 1 ).m(), 4.0, 0.001 );
  QGSCOMPARENEAR( intR->pointN( 2 ).x(),  6474984, 100 );
  QGSCOMPARENEAR( intR->pointN( 2 ).y(), -3526584, 100 );
  QGSCOMPARENEAR( intR->pointN( 2 ).z(), 5.0, 0.001 );
  QGSCOMPARENEAR( intR->pointN( 2 ).m(), 6.0, 0.001 );
  QGSCOMPARENEAR( intR->pointN( 3 ).x(),  6374984, 100 );
  QGSCOMPARENEAR( intR->pointN( 3 ).y(), -3626584, 100 );
  QGSCOMPARENEAR( intR->pointN( 3 ).z(), 1.0, 0.001 );
  QGSCOMPARENEAR( intR->pointN( 3 ).m(), 2.0, 0.001 );
  QGSCOMPARENEAR( intR->boundingBox().xMinimum(), 6274984, 100 );
  QGSCOMPARENEAR( intR->boundingBox().yMinimum(), -3626584, 100 );
  QGSCOMPARENEAR( intR->boundingBox().xMaximum(), 6474984, 100 );
  QGSCOMPARENEAR( intR->boundingBox().yMaximum(), -3526584, 100 );

  //z value transform
  pTransform.transform( tr, QgsCoordinateTransform::ForwardTransform, true );
  extR = static_cast< const QgsLineString * >( pTransform.exteriorRing() );
  QGSCOMPARENEAR( extR->pointN( 0 ).z(), -19.249066, 0.001 );
  QGSCOMPARENEAR( extR->pointN( 1 ).z(), -19.148357, 0.001 );
  QGSCOMPARENEAR( extR->pointN( 2 ).z(), -19.092128, 0.001 );
  QGSCOMPARENEAR( extR->pointN( 3 ).z(), -19.249066, 0.001 );
  intR = static_cast< const QgsLineString * >( pTransform.interiorRing( 0 ) );
  QGSCOMPARENEAR( intR->pointN( 0 ).z(), -19.249066, 0.001 );
  QGSCOMPARENEAR( intR->pointN( 1 ).z(), -19.148357, 0.001 );
  QGSCOMPARENEAR( intR->pointN( 2 ).z(), -19.092128, 0.001 );
  QGSCOMPARENEAR( intR->pointN( 3 ).z(), -19.249066, 0.001 );
  pTransform.transform( tr, QgsCoordinateTransform::ReverseTransform, true );
  extR = static_cast< const QgsLineString * >( pTransform.exteriorRing() );
  QGSCOMPARENEAR( extR->pointN( 0 ).z(), 1, 0.001 );
  QGSCOMPARENEAR( extR->pointN( 1 ).z(), 3, 0.001 );
  QGSCOMPARENEAR( extR->pointN( 2 ).z(), 5, 0.001 );
  QGSCOMPARENEAR( extR->pointN( 3 ).z(), 1, 0.001 );
  intR = static_cast< const QgsLineString * >( pTransform.interiorRing( 0 ) );
  QGSCOMPARENEAR( intR->pointN( 0 ).z(), 1, 0.001 );
  QGSCOMPARENEAR( intR->pointN( 1 ).z(), 3, 0.001 );
  QGSCOMPARENEAR( intR->pointN( 2 ).z(), 5, 0.001 );
  QGSCOMPARENEAR( intR->pointN( 3 ).z(), 1, 0.001 );

  //QTransform transform
  QTransform qtr = QTransform::fromScale( 2, 3 );
  QgsLineString l23;
  l23.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 1, 2, 3, 4 )
                 << QgsPoint( QgsWkbTypes::PointZM, 11, 12, 13, 14 )
                 << QgsPoint( QgsWkbTypes::PointZM, 1, 12, 23, 24 )
                 << QgsPoint( QgsWkbTypes::PointZM, 1, 2, 3, 4 ) );
  QgsPolygon pTransform2;
  pTransform2.setExteriorRing( l23.clone() );
  pTransform2.addInteriorRing( l23.clone() );
  pTransform2.transform( qtr, 2, 3, 4, 5 );

  extR = static_cast< const QgsLineString * >( pTransform2.exteriorRing() );
  QGSCOMPARENEAR( extR->pointN( 0 ).x(), 2, 100 );
  QGSCOMPARENEAR( extR->pointN( 0 ).y(), 6, 100 );
  QGSCOMPARENEAR( extR->pointN( 0 ).z(), 11.0, 0.001 );
  QGSCOMPARENEAR( extR->pointN( 0 ).m(), 24.0, 0.001 );
  QGSCOMPARENEAR( extR->pointN( 1 ).x(), 22, 100 );
  QGSCOMPARENEAR( extR->pointN( 1 ).y(), 36, 100 );
  QGSCOMPARENEAR( extR->pointN( 1 ).z(), 41.0, 0.001 );
  QGSCOMPARENEAR( extR->pointN( 1 ).m(), 74.0, 0.001 );
  QGSCOMPARENEAR( extR->pointN( 2 ).x(),  2, 100 );
  QGSCOMPARENEAR( extR->pointN( 2 ).y(), 36, 100 );
  QGSCOMPARENEAR( extR->pointN( 2 ).z(), 71.0, 0.001 );
  QGSCOMPARENEAR( extR->pointN( 2 ).m(), 124.0, 0.001 );
  QGSCOMPARENEAR( extR->pointN( 3 ).x(), 2, 100 );
  QGSCOMPARENEAR( extR->pointN( 3 ).y(), 6, 100 );
  QGSCOMPARENEAR( extR->pointN( 3 ).z(), 11.0, 0.001 );
  QGSCOMPARENEAR( extR->pointN( 3 ).m(), 24.0, 0.001 );
  QGSCOMPARENEAR( pTransform2.exteriorRing()->boundingBox().xMinimum(), 2, 0.001 );
  QGSCOMPARENEAR( pTransform2.exteriorRing()->boundingBox().yMinimum(), 6, 0.001 );
  QGSCOMPARENEAR( pTransform2.exteriorRing()->boundingBox().xMaximum(), 22, 0.001 );
  QGSCOMPARENEAR( pTransform2.exteriorRing()->boundingBox().yMaximum(), 36, 0.001 );
  intR = static_cast< const QgsLineString * >( pTransform2.interiorRing( 0 ) );
  QGSCOMPARENEAR( intR->pointN( 0 ).x(), 2, 100 );
  QGSCOMPARENEAR( intR->pointN( 0 ).y(), 6, 100 );
  QGSCOMPARENEAR( intR->pointN( 0 ).z(), 11.0, 0.001 );
  QGSCOMPARENEAR( intR->pointN( 0 ).m(), 24.0, 0.001 );
  QGSCOMPARENEAR( intR->pointN( 1 ).x(), 22, 100 );
  QGSCOMPARENEAR( intR->pointN( 1 ).y(), 36, 100 );
  QGSCOMPARENEAR( intR->pointN( 1 ).z(), 41.0, 0.001 );
  QGSCOMPARENEAR( intR->pointN( 1 ).m(), 74.0, 0.001 );
  QGSCOMPARENEAR( intR->pointN( 2 ).x(),  2, 100 );
  QGSCOMPARENEAR( intR->pointN( 2 ).y(), 36, 100 );
  QGSCOMPARENEAR( intR->pointN( 2 ).z(), 71.0, 0.001 );
  QGSCOMPARENEAR( intR->pointN( 2 ).m(), 124.0, 0.001 );
  QGSCOMPARENEAR( intR->pointN( 3 ).x(), 2, 100 );
  QGSCOMPARENEAR( intR->pointN( 3 ).y(), 6, 100 );
  QGSCOMPARENEAR( intR->pointN( 3 ).z(), 11.0, 0.001 );
  QGSCOMPARENEAR( intR->pointN( 3 ).m(), 24.0, 0.001 );
  QGSCOMPARENEAR( intR->boundingBox().xMinimum(), 2, 0.001 );
  QGSCOMPARENEAR( intR->boundingBox().yMinimum(), 6, 0.001 );
  QGSCOMPARENEAR( intR->boundingBox().xMaximum(), 22, 0.001 );
  QGSCOMPARENEAR( intR->boundingBox().yMaximum(), 36, 0.001 );

  // closestSegment
  QgsPoint pt;
  QgsVertexId v;
  int leftOf = 0;
  QgsPolygon empty;
  ( void )empty.closestSegment( QgsPoint( 1, 2 ), pt, v ); // empty polygon, just want no crash

  QgsPolygon p21;
  QgsLineString p21ls;
  p21ls.setPoints( QgsPointSequence() << QgsPoint( 5, 10 ) << QgsPoint( 7, 12 ) << QgsPoint( 5, 15 ) << QgsPoint( 5, 10 ) );
  p21.setExteriorRing( p21ls.clone() );
  QGSCOMPARENEAR( p21.closestSegment( QgsPoint( 4, 11 ), pt, v, &leftOf ), 1.0, 0.0001 );
  QGSCOMPARENEAR( pt.x(), 5, 0.01 );
  QGSCOMPARENEAR( pt.y(), 11, 0.01 );
  QCOMPARE( v, QgsVertexId( 0, 0, 3 ) );
  QCOMPARE( leftOf, 1 );
  QGSCOMPARENEAR( p21.closestSegment( QgsPoint( 8, 11 ), pt, v, &leftOf ),  2.0, 0.0001 );
  QGSCOMPARENEAR( pt.x(), 7, 0.01 );
  QGSCOMPARENEAR( pt.y(), 12, 0.01 );
  QCOMPARE( v, QgsVertexId( 0, 0, 1 ) );
  QCOMPARE( leftOf, 1 );
  QGSCOMPARENEAR( p21.closestSegment( QgsPoint( 6, 11.5 ), pt, v, &leftOf ), 0.125000, 0.0001 );
  QGSCOMPARENEAR( pt.x(), 6.25, 0.01 );
  QGSCOMPARENEAR( pt.y(), 11.25, 0.01 );
  QCOMPARE( v, QgsVertexId( 0, 0, 1 ) );
  QCOMPARE( leftOf, -1 );
  QGSCOMPARENEAR( p21.closestSegment( QgsPoint( 7, 16 ), pt, v, &leftOf ), 4.923077, 0.0001 );
  QGSCOMPARENEAR( pt.x(), 5.153846, 0.01 );
  QGSCOMPARENEAR( pt.y(), 14.769231, 0.01 );
  QCOMPARE( v, QgsVertexId( 0, 0, 2 ) );
  QCOMPARE( leftOf, 1 );
  QGSCOMPARENEAR( p21.closestSegment( QgsPoint( 5.5, 13.5 ), pt, v, &leftOf ), 0.173077, 0.0001 );
  QGSCOMPARENEAR( pt.x(), 5.846154, 0.01 );
  QGSCOMPARENEAR( pt.y(), 13.730769, 0.01 );
  QCOMPARE( v, QgsVertexId( 0, 0, 2 ) );
  QCOMPARE( leftOf, -1 );
  // point directly on segment
  QCOMPARE( p21.closestSegment( QgsPoint( 5, 15 ), pt, v, &leftOf ), 0.0 );
  QCOMPARE( pt, QgsPoint( 5, 15 ) );
  QCOMPARE( v, QgsVertexId( 0, 0, 2 ) );
  // with interior ring
  p21ls.setPoints( QgsPointSequence() << QgsPoint( 6, 11.5 ) << QgsPoint( 6.5, 12 ) << QgsPoint( 6, 13 ) << QgsPoint( 6, 11.5 ) );
  p21.addInteriorRing( p21ls.clone() );
  QGSCOMPARENEAR( p21.closestSegment( QgsPoint( 4, 11 ), pt, v, &leftOf ), 1.0, 0.0001 );
  QGSCOMPARENEAR( pt.x(), 5, 0.01 );
  QGSCOMPARENEAR( pt.y(), 11, 0.01 );
  QCOMPARE( v, QgsVertexId( 0, 0, 3 ) );
  QCOMPARE( leftOf, 1 );
  QGSCOMPARENEAR( p21.closestSegment( QgsPoint( 8, 11 ), pt, v, &leftOf ),  2.0, 0.0001 );
  QGSCOMPARENEAR( pt.x(), 7, 0.01 );
  QGSCOMPARENEAR( pt.y(), 12, 0.01 );
  QCOMPARE( v, QgsVertexId( 0, 0, 1 ) );
  QCOMPARE( leftOf, 1 );
  QGSCOMPARENEAR( p21.closestSegment( QgsPoint( 6, 11.4 ), pt, v, &leftOf ), 0.01, 0.0001 );
  QGSCOMPARENEAR( pt.x(), 6.0, 0.01 );
  QGSCOMPARENEAR( pt.y(), 11.5, 0.01 );
  QCOMPARE( v, QgsVertexId( 0, 1, 1 ) );
  QCOMPARE( leftOf, 1 );
  QGSCOMPARENEAR( p21.closestSegment( QgsPoint( 7, 16 ), pt, v, &leftOf ), 4.923077, 0.0001 );
  QGSCOMPARENEAR( pt.x(), 5.153846, 0.01 );
  QGSCOMPARENEAR( pt.y(), 14.769231, 0.01 );
  QCOMPARE( v, QgsVertexId( 0, 0, 2 ) );
  QCOMPARE( leftOf, 1 );
  QGSCOMPARENEAR( p21.closestSegment( QgsPoint( 5.5, 13.5 ), pt, v, &leftOf ), 0.173077, 0.0001 );
  QGSCOMPARENEAR( pt.x(), 5.846154, 0.01 );
  QGSCOMPARENEAR( pt.y(), 13.730769, 0.01 );
  QCOMPARE( v, QgsVertexId( 0, 0, 2 ) );
  QCOMPARE( leftOf, -1 );
  // point directly on segment
  QCOMPARE( p21.closestSegment( QgsPoint( 6, 13 ), pt, v, &leftOf ), 0.0 );
  QCOMPARE( pt, QgsPoint( 6, 13 ) );
  QCOMPARE( v, QgsVertexId( 0, 1, 2 ) );
  QCOMPARE( leftOf, 0 );

  //nextVertex
  QgsPolygon p22;
  QVERIFY( !p22.nextVertex( v, pt ) );
  v = QgsVertexId( 0, 0, -2 );
  QVERIFY( !p22.nextVertex( v, pt ) );
  v = QgsVertexId( 0, 0, 10 );
  QVERIFY( !p22.nextVertex( v, pt ) );
  QgsLineString lp22;
  lp22.setPoints( QgsPointSequence() << QgsPoint( 1, 2 ) << QgsPoint( 11, 12 ) << QgsPoint( 1, 12 ) << QgsPoint( 1, 2 ) );
  p22.setExteriorRing( lp22.clone() );
  v = QgsVertexId( 0, 0, 4 ); //out of range
  QVERIFY( !p22.nextVertex( v, pt ) );
  v = QgsVertexId( 0, 0, -5 );
  QVERIFY( p22.nextVertex( v, pt ) );
  v = QgsVertexId( 0, 0, -1 );
  QVERIFY( p22.nextVertex( v, pt ) );
  QCOMPARE( v, QgsVertexId( 0, 0, 0 ) );
  QCOMPARE( pt, QgsPoint( 1, 2 ) );
  QVERIFY( p22.nextVertex( v, pt ) );
  QCOMPARE( v, QgsVertexId( 0, 0, 1 ) );
  QCOMPARE( pt, QgsPoint( 11, 12 ) );
  QVERIFY( p22.nextVertex( v, pt ) );
  QCOMPARE( v, QgsVertexId( 0, 0, 2 ) );
  QCOMPARE( pt, QgsPoint( 1, 12 ) );
  QVERIFY( p22.nextVertex( v, pt ) );
  QCOMPARE( v, QgsVertexId( 0, 0, 3 ) );
  QCOMPARE( pt, QgsPoint( 1, 2 ) );
  v = QgsVertexId( 0, 1, 0 );
  QVERIFY( !p22.nextVertex( v, pt ) );
  v = QgsVertexId( 1, 0, 0 );
  QVERIFY( p22.nextVertex( v, pt ) );
  QCOMPARE( v, QgsVertexId( 1, 0, 1 ) ); //test that part number is maintained
  QCOMPARE( pt, QgsPoint( 11, 12 ) );
  // add interior ring
  lp22.setPoints( QgsPointSequence() << QgsPoint( 11, 12 ) << QgsPoint( 21, 22 ) << QgsPoint( 11, 22 ) << QgsPoint( 11, 12 ) );
  p22.addInteriorRing( lp22.clone() );
  v = QgsVertexId( 0, 1, 4 ); //out of range
  QVERIFY( !p22.nextVertex( v, pt ) );
  v = QgsVertexId( 0, 1, -5 );
  QVERIFY( p22.nextVertex( v, pt ) );
  v = QgsVertexId( 0, 1, -1 );
  QVERIFY( p22.nextVertex( v, pt ) );
  QCOMPARE( v, QgsVertexId( 0, 1, 0 ) );
  QCOMPARE( pt, QgsPoint( 11, 12 ) );
  QVERIFY( p22.nextVertex( v, pt ) );
  QCOMPARE( v, QgsVertexId( 0, 1, 1 ) );
  QCOMPARE( pt, QgsPoint( 21, 22 ) );
  QVERIFY( p22.nextVertex( v, pt ) );
  QCOMPARE( v, QgsVertexId( 0, 1, 2 ) );
  QCOMPARE( pt, QgsPoint( 11, 22 ) );
  QVERIFY( p22.nextVertex( v, pt ) );
  QCOMPARE( v, QgsVertexId( 0, 1, 3 ) );
  QCOMPARE( pt, QgsPoint( 11, 12 ) );
  v = QgsVertexId( 0, 2, 0 );
  QVERIFY( !p22.nextVertex( v, pt ) );
  v = QgsVertexId( 1, 1, 0 );
  QVERIFY( p22.nextVertex( v, pt ) );
  QCOMPARE( v, QgsVertexId( 1, 1, 1 ) ); //test that part number is maintained
  QCOMPARE( pt, QgsPoint( 21, 22 ) );

  // dropZValue
  QgsPolygon p23;
  p23.dropZValue();
  QCOMPARE( p23.wkbType(), QgsWkbTypes::Polygon );
  QgsLineString lp23;
  lp23.setPoints( QgsPointSequence() << QgsPoint( 1, 2 ) << QgsPoint( 11, 12 ) << QgsPoint( 1, 12 ) << QgsPoint( 1, 2 ) );
  p23.setExteriorRing( lp23.clone() );
  p23.addInteriorRing( lp23.clone() );
  QCOMPARE( p23.wkbType(), QgsWkbTypes::Polygon );
  p23.dropZValue(); // not z
  QCOMPARE( p23.wkbType(), QgsWkbTypes::Polygon );
  QCOMPARE( p23.exteriorRing()->wkbType(), QgsWkbTypes::LineString );
  QCOMPARE( static_cast< const QgsLineString *>( p23.exteriorRing() )->pointN( 0 ), QgsPoint( 1, 2 ) );
  QCOMPARE( p23.interiorRing( 0 )->wkbType(), QgsWkbTypes::LineString );
  QCOMPARE( static_cast< const QgsLineString *>( p23.interiorRing( 0 ) )->pointN( 0 ), QgsPoint( 1, 2 ) );
  // with z
  lp23.setPoints( QgsPointSequence() << QgsPoint( 1, 2, 3 ) << QgsPoint( 11, 12, 13 ) << QgsPoint( 1, 12, 23 ) << QgsPoint( 1, 2, 3 ) );
  p23.clear();
  p23.setExteriorRing( lp23.clone() );
  p23.addInteriorRing( lp23.clone() );
  QCOMPARE( p23.wkbType(), QgsWkbTypes::PolygonZ );
  p23.dropZValue();
  QCOMPARE( p23.wkbType(), QgsWkbTypes::Polygon );
  QCOMPARE( p23.exteriorRing()->wkbType(), QgsWkbTypes::LineString );
  QCOMPARE( static_cast< const QgsLineString *>( p23.exteriorRing() )->pointN( 0 ), QgsPoint( 1, 2 ) );
  QCOMPARE( p23.interiorRing( 0 )->wkbType(), QgsWkbTypes::LineString );
  QCOMPARE( static_cast< const QgsLineString *>( p23.interiorRing( 0 ) )->pointN( 0 ), QgsPoint( 1, 2 ) );
  // with zm
  lp23.setPoints( QgsPointSequence() << QgsPoint( 1, 2, 3, 4 ) << QgsPoint( 11, 12, 13, 14 ) << QgsPoint( 1, 12, 23, 24 ) << QgsPoint( 1, 2, 3, 4 ) );
  p23.clear();
  p23.setExteriorRing( lp23.clone() );
  p23.addInteriorRing( lp23.clone() );
  QCOMPARE( p23.wkbType(), QgsWkbTypes::PolygonZM );
  p23.dropZValue();
  QCOMPARE( p23.wkbType(), QgsWkbTypes::PolygonM );
  QCOMPARE( p23.exteriorRing()->wkbType(), QgsWkbTypes::LineStringM );
  QCOMPARE( static_cast< const QgsLineString *>( p23.exteriorRing() )->pointN( 0 ), QgsPoint( QgsWkbTypes::PointM, 1, 2, 0, 4 ) );
  QCOMPARE( p23.interiorRing( 0 )->wkbType(), QgsWkbTypes::LineStringM );
  QCOMPARE( static_cast< const QgsLineString *>( p23.interiorRing( 0 ) )->pointN( 0 ), QgsPoint( QgsWkbTypes::PointM, 1, 2, 0, 4 ) );

  // dropMValue
  p23.clear();
  p23.dropMValue();
  QCOMPARE( p23.wkbType(), QgsWkbTypes::Polygon );
  lp23.setPoints( QgsPointSequence() << QgsPoint( 1, 2 ) << QgsPoint( 11, 12 ) << QgsPoint( 1, 12 ) << QgsPoint( 1, 2 ) );
  p23.setExteriorRing( lp23.clone() );
  p23.addInteriorRing( lp23.clone() );
  QCOMPARE( p23.wkbType(), QgsWkbTypes::Polygon );
  p23.dropMValue(); // not zm
  QCOMPARE( p23.wkbType(), QgsWkbTypes::Polygon );
  QCOMPARE( p23.exteriorRing()->wkbType(), QgsWkbTypes::LineString );
  QCOMPARE( static_cast< const QgsLineString *>( p23.exteriorRing() )->pointN( 0 ), QgsPoint( 1, 2 ) );
  QCOMPARE( p23.interiorRing( 0 )->wkbType(), QgsWkbTypes::LineString );
  QCOMPARE( static_cast< const QgsLineString *>( p23.interiorRing( 0 ) )->pointN( 0 ), QgsPoint( 1, 2 ) );
  // with m
  lp23.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointM,  1, 2, 0, 3 ) << QgsPoint( QgsWkbTypes::PointM, 11, 12, 0, 13 ) << QgsPoint( QgsWkbTypes::PointM, 1, 12, 0, 23 ) << QgsPoint( QgsWkbTypes::PointM,  1, 2, 0, 3 ) );
  p23.clear();
  p23.setExteriorRing( lp23.clone() );
  p23.addInteriorRing( lp23.clone() );
  QCOMPARE( p23.wkbType(), QgsWkbTypes::PolygonM );
  p23.dropMValue();
  QCOMPARE( p23.wkbType(), QgsWkbTypes::Polygon );
  QCOMPARE( p23.exteriorRing()->wkbType(), QgsWkbTypes::LineString );
  QCOMPARE( static_cast< const QgsLineString *>( p23.exteriorRing() )->pointN( 0 ), QgsPoint( 1, 2 ) );
  QCOMPARE( p23.interiorRing( 0 )->wkbType(), QgsWkbTypes::LineString );
  QCOMPARE( static_cast< const QgsLineString *>( p23.interiorRing( 0 ) )->pointN( 0 ), QgsPoint( 1, 2 ) );
  // with zm
  lp23.setPoints( QgsPointSequence() << QgsPoint( 1, 2, 3, 4 ) << QgsPoint( 11, 12, 13, 14 ) << QgsPoint( 1, 12, 23, 24 ) << QgsPoint( 1, 2, 3, 4 ) );
  p23.clear();
  p23.setExteriorRing( lp23.clone() );
  p23.addInteriorRing( lp23.clone() );
  QCOMPARE( p23.wkbType(), QgsWkbTypes::PolygonZM );
  p23.dropMValue();
  QCOMPARE( p23.wkbType(), QgsWkbTypes::PolygonZ );
  QCOMPARE( p23.exteriorRing()->wkbType(), QgsWkbTypes::LineStringZ );
  QCOMPARE( static_cast< const QgsLineString *>( p23.exteriorRing() )->pointN( 0 ), QgsPoint( QgsWkbTypes::PointZ, 1, 2, 3 ) );
  QCOMPARE( p23.interiorRing( 0 )->wkbType(), QgsWkbTypes::LineStringZ );
  QCOMPARE( static_cast< const QgsLineString *>( p23.interiorRing( 0 ) )->pointN( 0 ), QgsPoint( QgsWkbTypes::PointZ, 1, 2, 3 ) );

  //vertexAngle
  QgsPolygon p24;
  ( void )p24.vertexAngle( QgsVertexId() ); //just want no crash
  ( void )p24.vertexAngle( QgsVertexId( 0, 0, 0 ) ); //just want no crash
  ( void )p24.vertexAngle( QgsVertexId( 0, 1, 0 ) ); //just want no crash
  QgsLineString l38;
  l38.setPoints( QgsPointSequence() << QgsPoint( 0, 0 ) << QgsPoint( 0.5, 0 ) << QgsPoint( 1, 0 )
                 << QgsPoint( 2, 1 ) << QgsPoint( 1, 2 ) << QgsPoint( 0, 2 ) << QgsPoint( 0, 0 ) );
  p24.setExteriorRing( l38.clone() );
  QGSCOMPARENEAR( p24.vertexAngle( QgsVertexId( 0, 0, 0 ) ), 2.35619, 0.00001 );
  QGSCOMPARENEAR( p24.vertexAngle( QgsVertexId( 0, 0, 1 ) ), 1.5708, 0.0001 );
  QGSCOMPARENEAR( p24.vertexAngle( QgsVertexId( 0, 0, 2 ) ), 1.17809, 0.00001 );
  QGSCOMPARENEAR( p24.vertexAngle( QgsVertexId( 0, 0, 3 ) ), 0.0, 0.00001 );
  QGSCOMPARENEAR( p24.vertexAngle( QgsVertexId( 0, 0, 4 ) ), 5.10509, 0.00001 );
  QGSCOMPARENEAR( p24.vertexAngle( QgsVertexId( 0, 0, 5 ) ), 3.92699, 0.00001 );
  QGSCOMPARENEAR( p24.vertexAngle( QgsVertexId( 0, 0, 6 ) ), 2.35619, 0.00001 );
  p24.addInteriorRing( l38.clone() );
  QGSCOMPARENEAR( p24.vertexAngle( QgsVertexId( 0, 1, 0 ) ), 2.35619, 0.00001 );
  QGSCOMPARENEAR( p24.vertexAngle( QgsVertexId( 0, 1, 1 ) ), 1.5708, 0.0001 );
  QGSCOMPARENEAR( p24.vertexAngle( QgsVertexId( 0, 1, 2 ) ), 1.17809, 0.00001 );
  QGSCOMPARENEAR( p24.vertexAngle( QgsVertexId( 0, 1, 3 ) ), 0.0, 0.00001 );
  QGSCOMPARENEAR( p24.vertexAngle( QgsVertexId( 0, 1, 4 ) ), 5.10509, 0.00001 );
  QGSCOMPARENEAR( p24.vertexAngle( QgsVertexId( 0, 1, 5 ) ), 3.92699, 0.00001 );
  QGSCOMPARENEAR( p24.vertexAngle( QgsVertexId( 0, 1, 6 ) ), 2.35619, 0.00001 );

  //insert vertex

  //insert vertex in empty polygon
  QgsPolygon p25;
  QVERIFY( !p25.insertVertex( QgsVertexId( 0, 0, 0 ), QgsPoint( 6.0, 7.0 ) ) );
  QVERIFY( !p25.insertVertex( QgsVertexId( 0, 0, 1 ), QgsPoint( 6.0, 7.0 ) ) );
  QVERIFY( !p25.insertVertex( QgsVertexId( 0, 1, 0 ), QgsPoint( 6.0, 7.0 ) ) );
  QVERIFY( !p25.insertVertex( QgsVertexId( 1, 0, 0 ), QgsPoint( 6.0, 7.0 ) ) );
  QVERIFY( p25.isEmpty() );
  l38.setPoints( QgsPointSequence() << QgsPoint( 0, 0 ) << QgsPoint( 0.5, 0 ) << QgsPoint( 1, 0 )
                 << QgsPoint( 2, 1 ) << QgsPoint( 1, 2 ) << QgsPoint( 0, 2 ) << QgsPoint( 0, 0 ) );
  p25.setExteriorRing( l38.clone() );
  QVERIFY( p25.insertVertex( QgsVertexId( 0, 0, 1 ), QgsPoint( 0.3, 0 ) ) );
  QCOMPARE( p25.nCoordinates(), 8 );
  QCOMPARE( static_cast< const QgsLineString * >( p25.exteriorRing() )->pointN( 0 ), QgsPoint( 0, 0 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p25.exteriorRing() )->pointN( 1 ), QgsPoint( 0.3, 0 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p25.exteriorRing() )->pointN( 2 ), QgsPoint( 0.5, 0 ) );
  QVERIFY( !p25.insertVertex( QgsVertexId( 0, 0, -1 ), QgsPoint( 6.0, 7.0 ) ) );
  QVERIFY( !p25.insertVertex( QgsVertexId( 0, 0, 100 ), QgsPoint( 6.0, 7.0 ) ) );
  QVERIFY( !p25.insertVertex( QgsVertexId( 0, 1, 0 ), QgsPoint( 6.0, 7.0 ) ) );
  // first vertex
  QVERIFY( p25.insertVertex( QgsVertexId( 0, 0, 0 ), QgsPoint( 0, 0.1 ) ) );
  QCOMPARE( p25.nCoordinates(), 9 );
  QCOMPARE( static_cast< const QgsLineString * >( p25.exteriorRing() )->pointN( 0 ), QgsPoint( 0, 0.1 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p25.exteriorRing() )->pointN( 1 ), QgsPoint( 0, 0 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p25.exteriorRing() )->pointN( 2 ), QgsPoint( 0.3, 0 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p25.exteriorRing() )->pointN( 3 ), QgsPoint( 0.5, 0 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p25.exteriorRing() )->pointN( 7 ), QgsPoint( 0, 2 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p25.exteriorRing() )->pointN( 8 ), QgsPoint( 0, 0.1 ) );
  // last vertex
  QVERIFY( p25.insertVertex( QgsVertexId( 0, 0, 9 ), QgsPoint( 0.1, 0.1 ) ) );
  QCOMPARE( p25.nCoordinates(), 10 );
  QCOMPARE( static_cast< const QgsLineString * >( p25.exteriorRing() )->pointN( 0 ), QgsPoint( 0.1, 0.1 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p25.exteriorRing() )->pointN( 1 ), QgsPoint( 0, 0 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p25.exteriorRing() )->pointN( 2 ), QgsPoint( 0.3, 0 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p25.exteriorRing() )->pointN( 3 ), QgsPoint( 0.5, 0 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p25.exteriorRing() )->pointN( 8 ), QgsPoint( 0, 0.1 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p25.exteriorRing() )->pointN( 9 ), QgsPoint( 0.1, 0.1 ) );
  // with interior ring
  p25.addInteriorRing( l38.clone() );
  QCOMPARE( p25.nCoordinates(), 17 );
  QVERIFY( p25.insertVertex( QgsVertexId( 0, 1, 1 ), QgsPoint( 0.3, 0 ) ) );
  QCOMPARE( p25.nCoordinates(), 18 );
  QCOMPARE( static_cast< const QgsLineString * >( p25.interiorRing( 0 ) )->pointN( 0 ), QgsPoint( 0, 0 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p25.interiorRing( 0 ) )->pointN( 1 ), QgsPoint( 0.3, 0 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p25.interiorRing( 0 ) )->pointN( 2 ), QgsPoint( 0.5, 0 ) );
  QVERIFY( !p25.insertVertex( QgsVertexId( 0, 1, -1 ), QgsPoint( 6.0, 7.0 ) ) );
  QVERIFY( !p25.insertVertex( QgsVertexId( 0, 1, 100 ), QgsPoint( 6.0, 7.0 ) ) );
  QVERIFY( !p25.insertVertex( QgsVertexId( 0, 2, 0 ), QgsPoint( 6.0, 7.0 ) ) );
  // first vertex in interior ring
  QVERIFY( p25.insertVertex( QgsVertexId( 0, 1, 0 ), QgsPoint( 0, 0.1 ) ) );
  QCOMPARE( p25.nCoordinates(), 19 );
  QCOMPARE( static_cast< const QgsLineString * >( p25.interiorRing( 0 ) )->pointN( 0 ), QgsPoint( 0, 0.1 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p25.interiorRing( 0 ) )->pointN( 1 ), QgsPoint( 0, 0 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p25.interiorRing( 0 ) )->pointN( 2 ), QgsPoint( 0.3, 0 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p25.interiorRing( 0 ) )->pointN( 3 ), QgsPoint( 0.5, 0 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p25.interiorRing( 0 ) )->pointN( 7 ), QgsPoint( 0, 2 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p25.interiorRing( 0 ) )->pointN( 8 ), QgsPoint( 0, 0.1 ) );
  // last vertex in interior ring
  QVERIFY( p25.insertVertex( QgsVertexId( 0, 1, 9 ), QgsPoint( 0.1, 0.1 ) ) );
  QCOMPARE( p25.nCoordinates(), 20 );
  QCOMPARE( static_cast< const QgsLineString * >( p25.interiorRing( 0 ) )->pointN( 0 ), QgsPoint( 0.1, 0.1 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p25.interiorRing( 0 ) )->pointN( 1 ), QgsPoint( 0, 0 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p25.interiorRing( 0 ) )->pointN( 2 ), QgsPoint( 0.3, 0 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p25.interiorRing( 0 ) )->pointN( 3 ), QgsPoint( 0.5, 0 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p25.interiorRing( 0 ) )->pointN( 8 ), QgsPoint( 0, 0.1 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p25.interiorRing( 0 ) )->pointN( 9 ), QgsPoint( 0.1, 0.1 ) );

  //move vertex

  //empty polygon
  QgsPolygon p26;
  QVERIFY( !p26.moveVertex( QgsVertexId( 0, 0, 0 ), QgsPoint( 6.0, 7.0 ) ) );
  QVERIFY( p26.isEmpty() );

  //valid polygon
  l38.setPoints( QgsPointSequence() << QgsPoint( 1, 2 )
                 << QgsPoint( 11, 12 ) << QgsPoint( 21, 22 ) << QgsPoint( 1, 2 ) );
  p26.setExteriorRing( l38.clone() );
  QVERIFY( p26.moveVertex( QgsVertexId( 0, 0, 0 ), QgsPoint( 6.0, 7.0 ) ) );
  QVERIFY( p26.moveVertex( QgsVertexId( 0, 0, 1 ), QgsPoint( 16.0, 17.0 ) ) );
  QVERIFY( p26.moveVertex( QgsVertexId( 0, 0, 2 ), QgsPoint( 26.0, 27.0 ) ) );
  QCOMPARE( static_cast< const QgsLineString * >( p26.exteriorRing() )->pointN( 0 ), QgsPoint( 6.0, 7.0 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p26.exteriorRing() )->pointN( 1 ), QgsPoint( 16.0, 17.0 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p26.exteriorRing() )->pointN( 2 ), QgsPoint( 26.0, 27.0 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p26.exteriorRing() )->pointN( 3 ), QgsPoint( 6.0, 7.0 ) );

  //out of range
  QVERIFY( !p26.moveVertex( QgsVertexId( 0, 0, -1 ), QgsPoint( 3.0, 4.0 ) ) );
  QVERIFY( !p26.moveVertex( QgsVertexId( 0, 0, 10 ), QgsPoint( 3.0, 4.0 ) ) );
  QVERIFY( !p26.moveVertex( QgsVertexId( 0, 1, 0 ), QgsPoint( 3.0, 4.0 ) ) );
  QCOMPARE( static_cast< const QgsLineString * >( p26.exteriorRing() )->pointN( 0 ), QgsPoint( 6.0, 7.0 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p26.exteriorRing() )->pointN( 1 ), QgsPoint( 16.0, 17.0 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p26.exteriorRing() )->pointN( 2 ), QgsPoint( 26.0, 27.0 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p26.exteriorRing() )->pointN( 3 ), QgsPoint( 6.0, 7.0 ) );

  // with interior ring
  p26.addInteriorRing( l38.clone() );
  QVERIFY( p26.moveVertex( QgsVertexId( 0, 1, 0 ), QgsPoint( 6.0, 7.0 ) ) );
  QVERIFY( p26.moveVertex( QgsVertexId( 0, 1, 1 ), QgsPoint( 16.0, 17.0 ) ) );
  QVERIFY( p26.moveVertex( QgsVertexId( 0, 1, 2 ), QgsPoint( 26.0, 27.0 ) ) );
  QCOMPARE( static_cast< const QgsLineString * >( p26.interiorRing( 0 ) )->pointN( 0 ), QgsPoint( 6.0, 7.0 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p26.interiorRing( 0 ) )->pointN( 1 ), QgsPoint( 16.0, 17.0 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p26.interiorRing( 0 ) )->pointN( 2 ), QgsPoint( 26.0, 27.0 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p26.interiorRing( 0 ) )->pointN( 3 ), QgsPoint( 6.0, 7.0 ) );
  QVERIFY( !p26.moveVertex( QgsVertexId( 0, 1, -1 ), QgsPoint( 3.0, 4.0 ) ) );
  QVERIFY( !p26.moveVertex( QgsVertexId( 0, 1, 10 ), QgsPoint( 3.0, 4.0 ) ) );
  QVERIFY( !p26.moveVertex( QgsVertexId( 0, 2, 0 ), QgsPoint( 3.0, 4.0 ) ) );

  //delete vertex

  //empty polygon
  QgsPolygon p27;
  QVERIFY( !p27.deleteVertex( QgsVertexId( 0, 0, 0 ) ) );
  QVERIFY( !p27.deleteVertex( QgsVertexId( 0, 1, 0 ) ) );
  QVERIFY( p27.isEmpty() );

  //valid polygon
  l38.setPoints( QgsPointSequence() << QgsPoint( 1, 2 ) << QgsPoint( 5, 2 ) << QgsPoint( 6, 2 ) << QgsPoint( 7, 2 )
                 << QgsPoint( 11, 12 ) << QgsPoint( 21, 22 ) << QgsPoint( 1, 2 ) );

  p27.setExteriorRing( l38.clone() );
  //out of range vertices
  QVERIFY( !p27.deleteVertex( QgsVertexId( 0, 0, -1 ) ) );
  QVERIFY( !p27.deleteVertex( QgsVertexId( 0, 0, 100 ) ) );
  QVERIFY( !p27.deleteVertex( QgsVertexId( 0, 1, 1 ) ) );

  //valid vertices
  QVERIFY( p27.deleteVertex( QgsVertexId( 0, 0, 1 ) ) );
  QCOMPARE( static_cast< const QgsLineString * >( p27.exteriorRing() )->pointN( 0 ), QgsPoint( 1.0, 2.0 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p27.exteriorRing() )->pointN( 1 ), QgsPoint( 6.0, 2.0 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p27.exteriorRing() )->pointN( 2 ), QgsPoint( 7.0, 2.0 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p27.exteriorRing() )->pointN( 3 ), QgsPoint( 11.0, 12.0 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p27.exteriorRing() )->pointN( 5 ), QgsPoint( 1.0, 2.0 ) );

  // delete first vertex
  QVERIFY( p27.deleteVertex( QgsVertexId( 0, 0, 0 ) ) );
  QCOMPARE( static_cast< const QgsLineString * >( p27.exteriorRing() )->pointN( 0 ), QgsPoint( 6.0, 2.0 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p27.exteriorRing() )->pointN( 1 ), QgsPoint( 7.0, 2.0 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p27.exteriorRing() )->pointN( 2 ), QgsPoint( 11.0, 12.0 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p27.exteriorRing() )->pointN( 3 ), QgsPoint( 21.0, 22.0 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p27.exteriorRing() )->pointN( 4 ), QgsPoint( 6.0, 2.0 ) );

  // delete last vertex
  QVERIFY( p27.deleteVertex( QgsVertexId( 0, 0, 4 ) ) );
  QCOMPARE( static_cast< const QgsLineString * >( p27.exteriorRing() )->pointN( 0 ), QgsPoint( 21.0, 22.0 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p27.exteriorRing() )->pointN( 1 ), QgsPoint( 7.0, 2.0 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p27.exteriorRing() )->pointN( 2 ), QgsPoint( 11.0, 12.0 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p27.exteriorRing() )->pointN( 3 ), QgsPoint( 21.0, 22.0 ) );

  // delete another vertex - should remove ring
  QVERIFY( p27.deleteVertex( QgsVertexId( 0, 0, 1 ) ) );
  QVERIFY( !p27.exteriorRing() );

  // with interior ring
  p27.setExteriorRing( l38.clone() );
  p27.addInteriorRing( l38.clone() );

  //out of range vertices
  QVERIFY( !p27.deleteVertex( QgsVertexId( 0, 1, -1 ) ) );
  QVERIFY( !p27.deleteVertex( QgsVertexId( 0, 1, 100 ) ) );
  QVERIFY( !p27.deleteVertex( QgsVertexId( 0, 2, 1 ) ) );

  //valid vertices
  QVERIFY( p27.deleteVertex( QgsVertexId( 0, 1, 1 ) ) );
  QCOMPARE( static_cast< const QgsLineString * >( p27.interiorRing( 0 ) )->pointN( 0 ), QgsPoint( 1.0, 2.0 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p27.interiorRing( 0 ) )->pointN( 1 ), QgsPoint( 6.0, 2.0 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p27.interiorRing( 0 ) )->pointN( 2 ), QgsPoint( 7.0, 2.0 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p27.interiorRing( 0 ) )->pointN( 3 ), QgsPoint( 11.0, 12.0 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p27.interiorRing( 0 ) )->pointN( 5 ), QgsPoint( 1.0, 2.0 ) );

  // delete first vertex
  QVERIFY( p27.deleteVertex( QgsVertexId( 0, 1, 0 ) ) );
  QCOMPARE( static_cast< const QgsLineString * >( p27.interiorRing( 0 ) )->pointN( 0 ), QgsPoint( 6.0, 2.0 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p27.interiorRing( 0 ) )->pointN( 1 ), QgsPoint( 7.0, 2.0 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p27.interiorRing( 0 ) )->pointN( 2 ), QgsPoint( 11.0, 12.0 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p27.interiorRing( 0 ) )->pointN( 3 ), QgsPoint( 21.0, 22.0 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p27.interiorRing( 0 ) )->pointN( 4 ), QgsPoint( 6.0, 2.0 ) );

  // delete last vertex
  QVERIFY( p27.deleteVertex( QgsVertexId( 0, 1, 4 ) ) );
  QCOMPARE( static_cast< const QgsLineString * >( p27.interiorRing( 0 ) )->pointN( 0 ), QgsPoint( 21.0, 22.0 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p27.interiorRing( 0 ) )->pointN( 1 ), QgsPoint( 7.0, 2.0 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p27.interiorRing( 0 ) )->pointN( 2 ), QgsPoint( 11.0, 12.0 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p27.interiorRing( 0 ) )->pointN( 3 ), QgsPoint( 21.0, 22.0 ) );

  // delete another vertex - should remove ring
  QVERIFY( p27.deleteVertex( QgsVertexId( 0, 1, 1 ) ) );
  QCOMPARE( p27.numInteriorRings(), 0 );
  QVERIFY( p27.exteriorRing() );

  // test that interior ring is "promoted" when exterior is removed
  p27.addInteriorRing( l38.clone() );
  QVERIFY( p27.deleteVertex( QgsVertexId( 0, 0, 0 ) ) );
  QCOMPARE( p27.numInteriorRings(), 1 );
  QVERIFY( p27.deleteVertex( QgsVertexId( 0, 0, 0 ) ) );
  QCOMPARE( p27.numInteriorRings(), 1 );
  QVERIFY( p27.deleteVertex( QgsVertexId( 0, 0, 0 ) ) );
  QCOMPARE( p27.numInteriorRings(), 1 );
  QVERIFY( p27.deleteVertex( QgsVertexId( 0, 0, 0 ) ) );
  QCOMPARE( p27.numInteriorRings(), 0 );
  QVERIFY( p27.exteriorRing() );

  // test adjacent vertices - should wrap around!
  QgsLineString *closedRing1 = new QgsLineString();
  closedRing1->setPoints( QVector<QgsPoint>() << QgsPoint( 1, 1 ) << QgsPoint( 1, 2 ) << QgsPoint( 2, 2 ) << QgsPoint( 2, 1 ) << QgsPoint( 1, 1 ) );
  QgsPolygon p28;
  QgsVertexId previous( 1, 2, 3 );
  QgsVertexId next( 4, 5, 6 );
  p28.adjacentVertices( QgsVertexId( 0, 0, 0 ), previous, next );
  QCOMPARE( previous, QgsVertexId( ) );
  QCOMPARE( next, QgsVertexId( ) );
  p28.adjacentVertices( QgsVertexId( 0, 1, 0 ), previous, next );
  QCOMPARE( previous, QgsVertexId( ) );
  QCOMPARE( next, QgsVertexId( ) );
  p28.adjacentVertices( QgsVertexId( 0, 0, 1 ), previous, next );
  QCOMPARE( previous, QgsVertexId( ) );
  QCOMPARE( next, QgsVertexId( ) );

  p28.setExteriorRing( closedRing1 );
  p28.adjacentVertices( QgsVertexId( 0, 0, 0 ), previous, next );
  QCOMPARE( previous, QgsVertexId( 0, 0, 3 ) );
  QCOMPARE( next, QgsVertexId( 0, 0, 1 ) );
  p28.adjacentVertices( QgsVertexId( 0, 0, 1 ), previous, next );
  QCOMPARE( previous, QgsVertexId( 0, 0, 0 ) );
  QCOMPARE( next, QgsVertexId( 0, 0, 2 ) );
  p28.adjacentVertices( QgsVertexId( 0, 0, 2 ), previous, next );
  QCOMPARE( previous, QgsVertexId( 0, 0, 1 ) );
  QCOMPARE( next, QgsVertexId( 0, 0, 3 ) );
  p28.adjacentVertices( QgsVertexId( 0, 0, 3 ), previous, next );
  QCOMPARE( previous, QgsVertexId( 0, 0, 2 ) );
  QCOMPARE( next, QgsVertexId( 0, 0, 4 ) );
  p28.adjacentVertices( QgsVertexId( 0, 0, 4 ), previous, next );
  QCOMPARE( previous, QgsVertexId( 0, 0, 3 ) );
  QCOMPARE( next, QgsVertexId( 0, 0, 1 ) );
  p28.adjacentVertices( QgsVertexId( 0, 1, 0 ), previous, next );
  QCOMPARE( previous, QgsVertexId( ) );
  QCOMPARE( next, QgsVertexId( ) );
  // part number should be retained
  p28.adjacentVertices( QgsVertexId( 1, 0, 0 ), previous, next );
  QCOMPARE( previous, QgsVertexId( 1, 0, 3 ) );
  QCOMPARE( next, QgsVertexId( 1, 0, 1 ) );

  // interior ring
  p28.addInteriorRing( closedRing1->clone() );
  p28.adjacentVertices( QgsVertexId( 0, 1, 0 ), previous, next );
  QCOMPARE( previous, QgsVertexId( 0, 1, 3 ) );
  QCOMPARE( next, QgsVertexId( 0, 1, 1 ) );
  p28.adjacentVertices( QgsVertexId( 0, 1, 1 ), previous, next );
  QCOMPARE( previous, QgsVertexId( 0, 1, 0 ) );
  QCOMPARE( next, QgsVertexId( 0, 1, 2 ) );
  p28.adjacentVertices( QgsVertexId( 0, 1, 2 ), previous, next );
  QCOMPARE( previous, QgsVertexId( 0, 1, 1 ) );
  QCOMPARE( next, QgsVertexId( 0, 1, 3 ) );
  p28.adjacentVertices( QgsVertexId( 0, 1, 3 ), previous, next );
  QCOMPARE( previous, QgsVertexId( 0, 1, 2 ) );
  QCOMPARE( next, QgsVertexId( 0, 1, 4 ) );
  p28.adjacentVertices( QgsVertexId( 0, 1, 4 ), previous, next );
  QCOMPARE( previous, QgsVertexId( 0, 1, 3 ) );
  QCOMPARE( next, QgsVertexId( 0, 1, 1 ) );
  p28.adjacentVertices( QgsVertexId( 0, 2, 0 ), previous, next );
  QCOMPARE( previous, QgsVertexId( ) );
  QCOMPARE( next, QgsVertexId( ) );

  // vertex number
  QgsLineString vertexLine2;
  QCOMPARE( vertexLine2.vertexNumberFromVertexId( QgsVertexId( -1, 0, 0 ) ), -1 );

  QgsLineString *closedRing2 = new QgsLineString();
  closedRing2->setPoints( QVector<QgsPoint>() << QgsPoint( 1, 1 ) << QgsPoint( 1, 2 ) << QgsPoint( 2, 2 ) << QgsPoint( 2, 1 ) << QgsPoint( 1, 1 ) );
  QgsPolygon p29;
  QCOMPARE( p29.vertexNumberFromVertexId( QgsVertexId( -1, 0, 0 ) ), -1 );
  QCOMPARE( p29.vertexNumberFromVertexId( QgsVertexId( 1, 0, 0 ) ), -1 );
  QCOMPARE( p29.vertexNumberFromVertexId( QgsVertexId( 0, -1, 0 ) ), -1 );
  QCOMPARE( p29.vertexNumberFromVertexId( QgsVertexId( 0, 1, 0 ) ), -1 );
  QCOMPARE( p29.vertexNumberFromVertexId( QgsVertexId( 0, 0, -1 ) ), -1 );
  QCOMPARE( p29.vertexNumberFromVertexId( QgsVertexId( 0, 0, 0 ) ), -1 );
  QCOMPARE( p29.vertexNumberFromVertexId( QgsVertexId( 0, 0, 1 ) ), -1 );
  p29.setExteriorRing( closedRing2 );
  QCOMPARE( p29.vertexNumberFromVertexId( QgsVertexId( -1, 0, 0 ) ), -1 );
  QCOMPARE( p29.vertexNumberFromVertexId( QgsVertexId( 1, 0, 0 ) ), -1 );
  QCOMPARE( p29.vertexNumberFromVertexId( QgsVertexId( 0, -1, 0 ) ), -1 );
  QCOMPARE( p29.vertexNumberFromVertexId( QgsVertexId( 0, 1, 0 ) ), -1 );
  QCOMPARE( p29.vertexNumberFromVertexId( QgsVertexId( 0, 0, -1 ) ), -1 );
  QCOMPARE( p29.vertexNumberFromVertexId( QgsVertexId( 0, 0, 0 ) ), 0 );
  QCOMPARE( p29.vertexNumberFromVertexId( QgsVertexId( 0, 0, 1 ) ), 1 );
  QCOMPARE( p29.vertexNumberFromVertexId( QgsVertexId( 0, 0, 2 ) ), 2 );
  QCOMPARE( p29.vertexNumberFromVertexId( QgsVertexId( 0, 0, 3 ) ), 3 );
  QCOMPARE( p29.vertexNumberFromVertexId( QgsVertexId( 0, 0, 4 ) ), 4 );
  QCOMPARE( p29.vertexNumberFromVertexId( QgsVertexId( 0, 0, 5 ) ), -1 );
  p29.addInteriorRing( closedRing2->clone() );
  QCOMPARE( p29.vertexNumberFromVertexId( QgsVertexId( 0, 0, 0 ) ), 0 );
  QCOMPARE( p29.vertexNumberFromVertexId( QgsVertexId( 0, 0, 1 ) ), 1 );
  QCOMPARE( p29.vertexNumberFromVertexId( QgsVertexId( 0, 0, 2 ) ), 2 );
  QCOMPARE( p29.vertexNumberFromVertexId( QgsVertexId( 0, 0, 3 ) ), 3 );
  QCOMPARE( p29.vertexNumberFromVertexId( QgsVertexId( 0, 0, 4 ) ), 4 );
  QCOMPARE( p29.vertexNumberFromVertexId( QgsVertexId( 0, 0, 5 ) ), -1 );
  QCOMPARE( p29.vertexNumberFromVertexId( QgsVertexId( 0, 1, 0 ) ), 5 );
  QCOMPARE( p29.vertexNumberFromVertexId( QgsVertexId( 0, 1, 1 ) ), 6 );
  QCOMPARE( p29.vertexNumberFromVertexId( QgsVertexId( 0, 1, 2 ) ), 7 );
  QCOMPARE( p29.vertexNumberFromVertexId( QgsVertexId( 0, 1, 3 ) ), 8 );
  QCOMPARE( p29.vertexNumberFromVertexId( QgsVertexId( 0, 1, 4 ) ), 9 );
  QCOMPARE( p29.vertexNumberFromVertexId( QgsVertexId( 0, 1, 5 ) ), -1 );


  //segmentLength
  QgsPolygon p30;
  QgsLineString vertexLine3;
  QCOMPARE( p30.segmentLength( QgsVertexId( -1, 0, 0 ) ), 0.0 );
  QCOMPARE( p30.segmentLength( QgsVertexId( 0, 0, 0 ) ), 0.0 );
  QCOMPARE( p30.segmentLength( QgsVertexId( 1, 0, 0 ) ), 0.0 );
  QCOMPARE( p30.segmentLength( QgsVertexId( 0, -1, 0 ) ), 0.0 );
  QCOMPARE( p30.segmentLength( QgsVertexId( 0, 1, 0 ) ), 0.0 );
  vertexLine3.setPoints( QgsPointSequence() << QgsPoint( 11, 2 ) << QgsPoint( 11, 12 ) << QgsPoint( 111, 12 )
                         << QgsPoint( 111, 2 ) << QgsPoint( 11, 2 ) );
  p30.setExteriorRing( vertexLine3.clone() );
  QCOMPARE( p30.segmentLength( QgsVertexId() ), 0.0 );
  QCOMPARE( p30.segmentLength( QgsVertexId( 0, 0, -1 ) ), 0.0 );
  QCOMPARE( p30.segmentLength( QgsVertexId( 0, 0, 0 ) ), 10.0 );
  QCOMPARE( p30.segmentLength( QgsVertexId( 0, 0, 1 ) ), 100.0 );
  QCOMPARE( p30.segmentLength( QgsVertexId( 0, 0, 2 ) ), 10.0 );
  QCOMPARE( p30.segmentLength( QgsVertexId( 0, 0, 3 ) ), 100.0 );
  QCOMPARE( p30.segmentLength( QgsVertexId( 0, 0, 4 ) ), 0.0 );
  QCOMPARE( p30.segmentLength( QgsVertexId( -1, 0, -1 ) ), 0.0 );
  QCOMPARE( p30.segmentLength( QgsVertexId( -1, 0, 0 ) ), 10.0 );
  QCOMPARE( p30.segmentLength( QgsVertexId( 0, -1, 0 ) ), 0.0 );
  QCOMPARE( p30.segmentLength( QgsVertexId( 0, 1, 0 ) ), 0.0 );
  QCOMPARE( p30.segmentLength( QgsVertexId( 0, 1, 1 ) ), 0.0 );
  QCOMPARE( p30.segmentLength( QgsVertexId( 1, 0, 1 ) ), 100.0 );
  QCOMPARE( p30.segmentLength( QgsVertexId( 1, 1, 1 ) ), 0.0 );

  // add interior ring
  vertexLine3.setPoints( QgsPointSequence() << QgsPoint( 30, 6 ) << QgsPoint( 34, 6 ) << QgsPoint( 34, 8 )
                         << QgsPoint( 30, 8 ) << QgsPoint( 30, 6 ) );
  p30.addInteriorRing( vertexLine3.clone() );
  QCOMPARE( p30.segmentLength( QgsVertexId() ), 0.0 );
  QCOMPARE( p30.segmentLength( QgsVertexId( 0, 0, -1 ) ), 0.0 );
  QCOMPARE( p30.segmentLength( QgsVertexId( 0, 0, 0 ) ), 10.0 );
  QCOMPARE( p30.segmentLength( QgsVertexId( 0, 0, 1 ) ), 100.0 );
  QCOMPARE( p30.segmentLength( QgsVertexId( 0, 0, 2 ) ), 10.0 );
  QCOMPARE( p30.segmentLength( QgsVertexId( 0, 0, 3 ) ), 100.0 );
  QCOMPARE( p30.segmentLength( QgsVertexId( 0, 0, 4 ) ), 0.0 );
  QCOMPARE( p30.segmentLength( QgsVertexId( -1, 0, -1 ) ), 0.0 );
  QCOMPARE( p30.segmentLength( QgsVertexId( -1, 0, 0 ) ), 10.0 );
  QCOMPARE( p30.segmentLength( QgsVertexId( 0, -1, 0 ) ), 0.0 );
  QCOMPARE( p30.segmentLength( QgsVertexId( 0, 1, -1 ) ), 0.0 );
  QCOMPARE( p30.segmentLength( QgsVertexId( 0, 1, 0 ) ), 4.0 );
  QCOMPARE( p30.segmentLength( QgsVertexId( 0, 1, 1 ) ), 2.0 );
  QCOMPARE( p30.segmentLength( QgsVertexId( 0, 1, 2 ) ), 4.0 );
  QCOMPARE( p30.segmentLength( QgsVertexId( 0, 1, 3 ) ), 2.0 );
  QCOMPARE( p30.segmentLength( QgsVertexId( 0, 1, 4 ) ), 0.0 );
  QCOMPARE( p30.segmentLength( QgsVertexId( 1, 0, 1 ) ), 100.0 );
  QCOMPARE( p30.segmentLength( QgsVertexId( 1, 1, 1 ) ), 2.0 );

  //removeDuplicateNodes
  QgsPolygon nodePolygon;
  QgsLineString nodeLine;
  QVERIFY( !nodePolygon.removeDuplicateNodes() );
  nodeLine.setPoints( QgsPointSequence() << QgsPoint( 11, 2 ) << QgsPoint( 11, 12 ) << QgsPoint( 11, 22 ) << QgsPoint( 11, 2 ) );
  nodePolygon.setExteriorRing( nodeLine.clone() );
  QVERIFY( !nodePolygon.removeDuplicateNodes() );
  QCOMPARE( nodePolygon.asWkt(), QStringLiteral( "Polygon ((11 2, 11 12, 11 22, 11 2))" ) );
  nodeLine.setPoints( QgsPointSequence() << QgsPoint( 11, 2 ) << QgsPoint( 11.01, 1.99 ) << QgsPoint( 11.02, 2.01 )
                      << QgsPoint( 11, 12 ) << QgsPoint( 11, 22 ) << QgsPoint( 11.01, 21.99 ) << QgsPoint( 10.99, 1.99 ) << QgsPoint( 11, 2 ) );
  nodePolygon.setExteriorRing( nodeLine.clone() );
  QVERIFY( nodePolygon.removeDuplicateNodes( 0.02 ) );
  QCOMPARE( nodePolygon.asWkt( 2 ), QStringLiteral( "Polygon ((11 2, 11 12, 11 22, 11 2))" ) );
  nodeLine.setPoints( QgsPointSequence() << QgsPoint( 11, 2 ) << QgsPoint( 11.01, 1.99 ) << QgsPoint( 11.02, 2.01 )
                      << QgsPoint( 11, 12 ) << QgsPoint( 11, 22 ) << QgsPoint( 11.01, 21.99 ) << QgsPoint( 10.99, 1.99 ) << QgsPoint( 11, 2 ) );
  nodePolygon.setExteriorRing( nodeLine.clone() );
  QVERIFY( !nodePolygon.removeDuplicateNodes() );
  QCOMPARE( nodePolygon.asWkt( 2 ), QStringLiteral( "Polygon ((11 2, 11.01 1.99, 11.02 2.01, 11 12, 11 22, 11.01 21.99, 10.99 1.99, 11 2))" ) );
  // don't create degenerate rings
  nodeLine.setPoints( QgsPointSequence() << QgsPoint( 11, 2 ) << QgsPoint( 11.01, 2.01 ) << QgsPoint( 11, 2.01 ) << QgsPoint( 11, 2 ) );
  nodePolygon.addInteriorRing( nodeLine.clone() );
  QVERIFY( nodePolygon.removeDuplicateNodes( 0.02 ) );
  QCOMPARE( nodePolygon.asWkt( 2 ), QStringLiteral( "Polygon ((11 2, 11 12, 11 22, 11 2),(11 2, 11.01 2.01, 11 2.01, 11 2))" ) );

  // swap XY
  QgsPolygon swapPolygon;
  swapPolygon.swapXy(); //no crash
  QgsLineString swapLine;
  swapLine.setPoints( QgsPointSequence() << QgsPoint( 11, 2, 3, 4, QgsWkbTypes::PointZM ) << QgsPoint( 11, 12, 13, 14, QgsWkbTypes::PointZM ) << QgsPoint( 11, 22, 23, 24, QgsWkbTypes::PointZM ) << QgsPoint( 11, 2, 3, 4, QgsWkbTypes::PointZM ) );
  swapPolygon.setExteriorRing( swapLine.clone() );
  swapPolygon.swapXy();
  QCOMPARE( swapPolygon.asWkt(), QStringLiteral( "PolygonZM ((2 11 3 4, 12 11 13 14, 22 11 23 24, 2 11 3 4))" ) );
  swapLine.setPoints( QgsPointSequence() << QgsPoint( 1, 2, 5, 6, QgsWkbTypes::PointZM ) << QgsPoint( 11.01, 2.01, 15, 16, QgsWkbTypes::PointZM ) << QgsPoint( 11, 2.01, 25, 26, QgsWkbTypes::PointZM ) << QgsPoint( 11, 2, 5, 6, QgsWkbTypes::PointZM ) );
  swapPolygon.addInteriorRing( swapLine.clone() );
  swapPolygon.swapXy();
  QCOMPARE( swapPolygon.asWkt( 2 ), QStringLiteral( "PolygonZM ((11 2 3 4, 11 12 13 14, 11 22 23 24, 11 2 3 4),(2 1 5 6, 2.01 11.01 15 16, 2.01 11 25 26, 2 11 5 6, 2 1 5 6))" ) );

  // filter vertex
  QgsPolygon filterPolygon;
  auto filter = []( const QgsPoint & point )-> bool
  {
    return point.x() > 5;
  };
  filterPolygon.filterVertices( filter ); // no crash
  QgsLineString filterPolygonRing;
  filterPolygonRing.setPoints( QgsPointSequence() << QgsPoint( 11, 2, 3, 4, QgsWkbTypes::PointZM ) << QgsPoint( 4, 12, 13, 14, QgsWkbTypes::PointZM ) << QgsPoint( 11, 12, 13, 14, QgsWkbTypes::PointZM ) << QgsPoint( 11, 22, 23, 24, QgsWkbTypes::PointZM ) << QgsPoint( 11, 2, 3, 4, QgsWkbTypes::PointZM ) );
  filterPolygon.setExteriorRing( filterPolygonRing.clone() );
  filterPolygon.filterVertices( filter );
  QCOMPARE( filterPolygon.asWkt(), QStringLiteral( "PolygonZM ((11 2 3 4, 11 12 13 14, 11 22 23 24, 11 2 3 4))" ) );
  filterPolygonRing.setPoints( QgsPointSequence() << QgsPoint( 10, 2, 5, 6, QgsWkbTypes::PointZM ) << QgsPoint( 4, 12, 13, 14, QgsWkbTypes::PointZM ) << QgsPoint( 11.01, 2.01, 15, 16, QgsWkbTypes::PointZM ) << QgsPoint( 11, 2.01, 25, 26, QgsWkbTypes::PointZM ) << QgsPoint( 11, 2, 5, 6, QgsWkbTypes::PointZM ) << QgsPoint( 10, 2, 5, 6, QgsWkbTypes::PointZM ) );
  filterPolygon.addInteriorRing( filterPolygonRing.clone() );
  filterPolygonRing.setPoints( QgsPointSequence() << QgsPoint( 1, 2, 5, 6, QgsWkbTypes::PointZM ) << QgsPoint( 11.01, 2.01, 15, 16, QgsWkbTypes::PointZM ) << QgsPoint( 11, 2.01, 25, 26, QgsWkbTypes::PointZM ) << QgsPoint( 1, 2, 5, 6, QgsWkbTypes::PointZM ) );
  filterPolygon.addInteriorRing( filterPolygonRing.clone() );
  filterPolygon.filterVertices( filter );
  QCOMPARE( filterPolygon.asWkt( 2 ), QStringLiteral( "PolygonZM ((11 2 3 4, 11 12 13 14, 11 22 23 24, 11 2 3 4),(10 2 5 6, 11.01 2.01 15 16, 11 2.01 25 26, 11 2 5 6, 10 2 5 6),(11.01 2.01 15 16, 11 2.01 25 26))" ) );

  // transform vertex
  QgsPolygon transformPolygon;
  auto transform = []( const QgsPoint & point )-> QgsPoint
  {
    return QgsPoint( point.x() + 2, point.y() + 3, point.z() + 4, point.m() + 5 );
  };
  transformPolygon.transformVertices( transform ); // no crash
  QgsLineString transformPolygonRing;
  transformPolygonRing.setPoints( QgsPointSequence() << QgsPoint( 11, 2, 3, 4, QgsWkbTypes::PointZM ) << QgsPoint( 4, 12, 13, 14, QgsWkbTypes::PointZM ) << QgsPoint( 11, 12, 13, 14, QgsWkbTypes::PointZM ) << QgsPoint( 11, 22, 23, 24, QgsWkbTypes::PointZM ) << QgsPoint( 11, 2, 3, 4, QgsWkbTypes::PointZM ) );
  transformPolygon.setExteriorRing( transformPolygonRing.clone() );
  transformPolygon.transformVertices( transform );
  QCOMPARE( transformPolygon.asWkt(), QStringLiteral( "PolygonZM ((13 5 7 9, 6 15 17 19, 13 15 17 19, 13 25 27 29, 13 5 7 9))" ) );
  transformPolygonRing.setPoints( QgsPointSequence() << QgsPoint( 10, 2, 5, 6, QgsWkbTypes::PointZM ) << QgsPoint( 4, 12, 13, 14, QgsWkbTypes::PointZM ) << QgsPoint( 11.01, 2.01, 15, 16, QgsWkbTypes::PointZM ) << QgsPoint( 11, 2.01, 25, 26, QgsWkbTypes::PointZM ) << QgsPoint( 11, 2, 5, 6, QgsWkbTypes::PointZM ) << QgsPoint( 10, 2, 5, 6, QgsWkbTypes::PointZM ) );
  transformPolygon.addInteriorRing( transformPolygonRing.clone() );
  transformPolygonRing.setPoints( QgsPointSequence() << QgsPoint( 1, 2, 5, 6, QgsWkbTypes::PointZM ) << QgsPoint( 11.01, 2.01, 15, 16, QgsWkbTypes::PointZM ) << QgsPoint( 11, 2.01, 25, 26, QgsWkbTypes::PointZM ) << QgsPoint( 1, 2, 5, 6, QgsWkbTypes::PointZM ) );
  transformPolygon.addInteriorRing( transformPolygonRing.clone() );
  transformPolygon.transformVertices( transform );
  QCOMPARE( transformPolygon.asWkt( 2 ), QStringLiteral( "PolygonZM ((15 8 11 14, 8 18 21 24, 15 18 21 24, 15 28 31 34, 15 8 11 14),(12 5 9 11, 6 15 17 19, 13.01 5.01 19 21, 13 5.01 29 31, 13 5 9 11, 12 5 9 11),(3 5 9 11, 13.01 5.01 19 21, 13 5.01 29 31, 3 5 9 11))" ) );


  // remove invalid rings
  QgsPolygon invalidRingPolygon;
  invalidRingPolygon.removeInvalidRings(); // no crash
  QgsLineString removeInvalidPolygonRing;
  removeInvalidPolygonRing.setPoints( QgsPointSequence() << QgsPoint( 11, 2, 3, 4, QgsWkbTypes::PointZM ) << QgsPoint( 4, 12, 13, 14, QgsWkbTypes::PointZM ) << QgsPoint( 11, 12, 13, 14, QgsWkbTypes::PointZM ) << QgsPoint( 11, 22, 23, 24, QgsWkbTypes::PointZM ) << QgsPoint( 11, 2, 3, 4, QgsWkbTypes::PointZM ) );
  invalidRingPolygon.setExteriorRing( removeInvalidPolygonRing.clone() );
  invalidRingPolygon.removeInvalidRings();
  QCOMPARE( invalidRingPolygon.asWkt(), QStringLiteral( "PolygonZM ((11 2 3 4, 4 12 13 14, 11 12 13 14, 11 22 23 24, 11 2 3 4))" ) );
  removeInvalidPolygonRing.setPoints( QgsPointSequence() << QgsPoint( 10, 2, 5, 6, QgsWkbTypes::PointZM )  << QgsPoint( 11, 2, 5, 6, QgsWkbTypes::PointZM ) << QgsPoint( 10, 2, 5, 6, QgsWkbTypes::PointZM ) );
  invalidRingPolygon.addInteriorRing( removeInvalidPolygonRing.clone() );
  removeInvalidPolygonRing.setPoints( QgsPointSequence() << QgsPoint( 1, 2, 5, 6, QgsWkbTypes::PointZM ) << QgsPoint( 11.01, 2.01, 15, 16, QgsWkbTypes::PointZM ) << QgsPoint( 11, 2.01, 25, 26, QgsWkbTypes::PointZM ) << QgsPoint( 1, 2, 5, 6, QgsWkbTypes::PointZM ) );
  invalidRingPolygon.addInteriorRing( removeInvalidPolygonRing.clone() );
  invalidRingPolygon.removeInvalidRings();
  QCOMPARE( invalidRingPolygon.asWkt( 2 ), QStringLiteral( "PolygonZM ((11 2 3 4, 4 12 13 14, 11 12 13 14, 11 22 23 24, 11 2 3 4),(1 2 5 6, 11.01 2.01 15 16, 11 2.01 25 26, 1 2 5 6))" ) );

  // force RHR
  QgsPolygon rhr;
  rhr.forceRHR(); // no crash
  rhr.fromWkt( QStringLiteral( "PolygonZM ((0 0 3 4, 0 100 13 14, 100 100 13 14, 100 0 23 24, 0 0 3 4))" ) );
  rhr.forceRHR();
  QCOMPARE( rhr.asWkt( 2 ), QStringLiteral( "PolygonZM ((0 0 3 4, 0 100 13 14, 100 100 13 14, 100 0 23 24, 0 0 3 4))" ) );
  rhr.fromWkt( QStringLiteral( "PolygonZM ((0 0 3 4, 100 0 13 14, 100 100 23 24, 0 100 23 24, 0 0 3 4))" ) );
  rhr.forceRHR();
  QCOMPARE( rhr.asWkt( 2 ), QStringLiteral( "PolygonZM ((0 0 3 4, 0 100 23 24, 100 100 23 24, 100 0 13 14, 0 0 3 4))" ) );
  rhr.fromWkt( QStringLiteral( "PolygonZM ((0 0 3 4, 0 100 13 14, 100 100 13 14, 100 0 23 24, 0 0 3 4),(10 10 1 2, 20 10 3 4, 20 20 4, 5, 10 20 6 8, 10 10 1 2))" ) );
  rhr.forceRHR();
  QCOMPARE( rhr.asWkt( 2 ), QStringLiteral( "PolygonZM ((0 0 3 4, 0 100 13 14, 100 100 13 14, 100 0 23 24, 0 0 3 4),(10 10 1 2, 20 10 3 4, 10 20 6 8, 10 10 1 2))" ) );
  rhr.fromWkt( QStringLiteral( "PolygonZM ((0 0 3 4, 100 0 13 14, 100 100 13 14, 0 100 23 24, 0 0 3 4),(10 10 1 2, 20 10 3 4, 20 20 4, 5, 10 20 6 8, 10 10 1 2))" ) );
  rhr.forceRHR();
  QCOMPARE( rhr.asWkt( 2 ), QStringLiteral( "PolygonZM ((0 0 3 4, 0 100 23 24, 100 100 13 14, 100 0 13 14, 0 0 3 4),(10 10 1 2, 20 10 3 4, 10 20 6 8, 10 10 1 2))" ) );
  rhr.fromWkt( QStringLiteral( "PolygonZM ((0 0 3 4, 0 100 13 14, 100 100 13 14, 100 0 23 24, 0 0 3 4),(10 10 1 2, 10 20 3 4, 20 10 6 8, 10 10 1 2))" ) );
  rhr.forceRHR();
  QCOMPARE( rhr.asWkt( 2 ), QStringLiteral( "PolygonZM ((0 0 3 4, 0 100 13 14, 100 100 13 14, 100 0 23 24, 0 0 3 4),(10 10 1 2, 20 10 6 8, 10 20 3 4, 10 10 1 2))" ) );
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

  // degenarate triangles
  QgsTriangle invalid( QgsPointXY( 0, 0 ), QgsPointXY( 0, 0 ), QgsPointXY( 10, 10 ) );
  QVERIFY( !invalid.isEmpty() );
  invalid = QgsTriangle( QPointF( 0, 0 ), QPointF( 0, 0 ), QPointF( 10, 10 ) );
  QVERIFY( !invalid.isEmpty() );
  //set exterior ring

  //try with no ring
  std::unique_ptr< QgsLineString > ext;
  t1.setExteriorRing( nullptr );
  QVERIFY( t1.isEmpty() );
  QCOMPARE( t1.numInteriorRings(), 0 );
  QCOMPARE( t1.nCoordinates(), 0 );
  QCOMPARE( t1.ringCount(), 0 );
  QCOMPARE( t1.partCount(), 0 );
  QVERIFY( !t1.exteriorRing() );
  QVERIFY( !t1.interiorRing( 0 ) );
  QCOMPARE( t1.wkbType(), QgsWkbTypes::Triangle );

  //valid exterior ring
  ext.reset( new QgsLineString() );
  ext->setPoints( QgsPointSequence() << QgsPoint( 0, 0 ) << QgsPoint( 0, 10 ) << QgsPoint( 10, 10 )
                  << QgsPoint( 0, 0 ) );
  QVERIFY( ext->isClosed() );
  t1.setExteriorRing( ext->clone() );
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
  ext.reset( new QgsLineString() );
  ext->setPoints( QgsPointSequence() << QgsPoint( 0, 10 ) << QgsPoint( 5, 5 ) << QgsPoint( 10, 10 )
                  << QgsPoint( 0, 10 ) );
  QVERIFY( ext->isClosed() );
  t1.setExteriorRing( ext->clone() );
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

  // AddZ
  QgsLineString lz;
  lz.setPoints( QgsPointSequence() << QgsPoint( 1, 2, 3 ) << QgsPoint( 11, 12, 13 ) << QgsPoint( 1, 12, 23 ) << QgsPoint( 1, 2, 3 ) );
  t1.setExteriorRing( lz.clone() );
  QVERIFY( t1.is3D() );
  QVERIFY( !t1.isMeasure() );
  QCOMPARE( t1.wkbType(), QgsWkbTypes::TriangleZ );
  QCOMPARE( t1.wktTypeStr(), QString( "TriangleZ" ) );
  QCOMPARE( t1.geometryType(), QString( "Triangle" ) );
  QCOMPARE( t1.dimension(), 2 );
  QCOMPARE( t1.vertexAt( 0 ).z(),  3.0 );
  QCOMPARE( t1.vertexAt( 1 ).z(), 13.0 );
  QCOMPARE( t1.vertexAt( 2 ).z(), 23.0 );
  // AddM
  QgsLineString lzm;
  lzm.setPoints( QgsPointSequence() << QgsPoint( 1, 2, 3, 4 ) << QgsPoint( 11, 12, 13, 14 ) << QgsPoint( 1, 12, 23, 24 ) << QgsPoint( 1, 2, 3, 4 ) );
  t1.setExteriorRing( lzm.clone() );
  QVERIFY( t1.is3D() );
  QVERIFY( t1.isMeasure() );
  QCOMPARE( t1.wkbType(), QgsWkbTypes::TriangleZM );
  QCOMPARE( t1.wktTypeStr(), QString( "TriangleZM" ) );
  QCOMPARE( t1.geometryType(), QString( "Triangle" ) );
  QCOMPARE( t1.dimension(), 2 );
  QCOMPARE( t1.vertexAt( 0 ).m(),  4.0 );
  QCOMPARE( t1.vertexAt( 1 ).m(), 14.0 );
  QCOMPARE( t1.vertexAt( 2 ).m(), 24.0 );
  // dropZ
  t1.dropZValue();
  QVERIFY( !t1.is3D() );
  QVERIFY( t1.isMeasure() );
  QCOMPARE( t1.wkbType(), QgsWkbTypes::TriangleM );
  QCOMPARE( t1.wktTypeStr(), QString( "TriangleM" ) );
  QCOMPARE( t1.geometryType(), QString( "Triangle" ) );
  QCOMPARE( t1.dimension(), 2 );
  QVERIFY( std::isnan( t1.vertexAt( 0 ).z() ) );
  QVERIFY( std::isnan( t1.vertexAt( 1 ).z() ) );
  QVERIFY( std::isnan( t1.vertexAt( 2 ).z() ) );
  // dropM
  t1.dropMValue();
  QVERIFY( !t1.is3D() );
  QVERIFY( !t1.isMeasure() );
  QCOMPARE( t1.wkbType(), QgsWkbTypes::Triangle );
  QCOMPARE( t1.wktTypeStr(), QString( "Triangle" ) );
  QCOMPARE( t1.geometryType(), QString( "Triangle" ) );
  QCOMPARE( t1.dimension(), 2 );
  QVERIFY( std::isnan( t1.vertexAt( 0 ).m() ) );
  QVERIFY( std::isnan( t1.vertexAt( 1 ).m() ) );
  QVERIFY( std::isnan( t1.vertexAt( 2 ).m() ) );

  //test that a non closed exterior ring will be automatically closed
  QgsTriangle t2;
  ext.reset( new QgsLineString() );
  ext->setPoints( QgsPointSequence() << QgsPoint( 0, 0 ) << QgsPoint( 0, 10 ) << QgsPoint( 10, 10 ) );
  QVERIFY( !ext->isClosed() );
  t2.setExteriorRing( ext.release() );
  QVERIFY( !t2.isEmpty() );
  QVERIFY( t2.exteriorRing()->isClosed() );
  QCOMPARE( t2.nCoordinates(), 4 );

  // invalid number of points
  ext.reset( new QgsLineString() );
  t2.clear();
  ext->setPoints( QgsPointSequence() << QgsPoint( 0, 0 ) << QgsPoint( 0, 10 ) );
  t2.setExteriorRing( ext.release() );
  QVERIFY( t2.isEmpty() );

  ext.reset( new QgsLineString() );
  t2.clear();
  ext->setPoints( QgsPointSequence() << QgsPoint( 0, 0 ) << QgsPoint( 0, 10 ) << QgsPoint( 10, 10 ) << QgsPoint( 5, 10 ) << QgsPoint( 8, 10 ) );
  t2.setExteriorRing( ext.release() );
  QVERIFY( t2.isEmpty() );

  // invalid exterior ring
  ext.reset( new QgsLineString() );
  t2.clear();
  ext->setPoints( QgsPointSequence() << QgsPoint( 0, 0 ) << QgsPoint( 0, 10 ) << QgsPoint( 10, 10 ) << QgsPoint( 5, 10 ) );
  t2.setExteriorRing( ext.release() );
  QVERIFY( t2.isEmpty() );

  ext.reset( new QgsLineString() );
  ext->setPoints( QgsPointSequence() << QgsPoint( 0, 0 ) << QgsPoint( 0, 10 ) << QgsPoint( 0, 0 ) );
  t2.setExteriorRing( ext.release() );
  QVERIFY( t2.isEmpty() );

  // degenerate case
  ext.reset( new QgsLineString() );
  ext->setPoints( QgsPointSequence() << QgsPoint( 0, 0 ) << QgsPoint( 0, 0 ) << QgsPoint( 0, 10 ) << QgsPoint( 0, 0 ) );
  t2.setExteriorRing( ext.release() );
  QVERIFY( !t2.isEmpty() );

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
  QCOMPARE( t3.area(), 0.0 );
  QGSCOMPARENEAR( t3.perimeter(), 28.284271, 0.001 );
  QVERIFY( t3.exteriorRing() );
  QVERIFY( !t3.interiorRing( 0 ) );

  // colinear
  t3 = QgsTriangle( QgsPoint( 0, 0 ), QgsPoint( 0, 5 ), QgsPoint( 0, 10 ) );
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
  QCOMPARE( t3.area(), 0.0 );
  QGSCOMPARENEAR( t3.perimeter(), 10.0 * 2, 0.001 );
  QVERIFY( t3.exteriorRing() );
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
  QVERIFY( QgsTriangle() != QgsTriangle( QgsPoint( 0, 0 ), QgsPoint( 0, 5 ), QgsPoint( 0, 10 ) ) ); // empty
  QVERIFY( QgsTriangle( QgsPoint( 0, 0 ), QgsPoint( 0, 5 ), QgsPoint( 0, 10 ) ) != QgsTriangle() ); // empty
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

  // Z
  t3 = QgsTriangle( QgsPoint( QgsWkbTypes::PointZ, 0, 5, 1 ), QgsPoint( QgsWkbTypes::PointZ, 0, 0, 2 ), QgsPoint( QgsWkbTypes::PointZ, 10, 10, 3 ) );
  QVERIFY( !t3.isEmpty() );
  QVERIFY( t3.is3D() );
  QVERIFY( !t3.isMeasure() );
  QCOMPARE( t3.wkbType(), QgsWkbTypes::TriangleZ );
  QCOMPARE( t3.wktTypeStr(), QString( "TriangleZ" ) );
  QCOMPARE( t3.geometryType(), QString( "Triangle" ) );
  // M
  t3 = QgsTriangle( QgsPoint( QgsWkbTypes::PointM, 0, 5, 0, 1 ), QgsPoint( QgsWkbTypes::PointM, 0, 0, 0, 2 ), QgsPoint( QgsWkbTypes::PointM, 10, 10, 0, 3 ) );
  QVERIFY( !t3.isEmpty() );
  QVERIFY( !t3.is3D() );
  QVERIFY( t3.isMeasure() );
  QCOMPARE( t3.wkbType(), QgsWkbTypes::TriangleM );
  QCOMPARE( t3.wktTypeStr(), QString( "TriangleM" ) );
  QCOMPARE( t3.geometryType(), QString( "Triangle" ) );
  // ZM
  t3 = QgsTriangle( QgsPoint( QgsWkbTypes::PointZM, 0, 5, 8, 1 ), QgsPoint( QgsWkbTypes::PointZM, 0, 0, 5, 2 ), QgsPoint( QgsWkbTypes::PointZM, 10, 10, 2, 3 ) );
  QVERIFY( !t3.isEmpty() );
  QVERIFY( t3.is3D() );
  QVERIFY( t3.isMeasure() );
  QCOMPARE( t3.wkbType(), QgsWkbTypes::TriangleZM );
  QCOMPARE( t3.wktTypeStr(), QString( "TriangleZM" ) );
  QCOMPARE( t3.geometryType(), QString( "Triangle" ) );

  // fromWkt
  QgsTriangle t5;
  ext.reset( new QgsLineString() );
  ext->setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 0, 0, 1, 5 )
                  << QgsPoint( QgsWkbTypes::PointZM, 0, 10, 2, 6 ) << QgsPoint( QgsWkbTypes::PointZM, 10, 10, 3, 7 ) );
  t5.setExteriorRing( ext.release() );
  QString wkt = t5.asWkt();
  QVERIFY( !wkt.isEmpty() );
  QgsTriangle t6;
  QVERIFY( t6.fromWkt( wkt ) );
  QCOMPARE( t5, t6 );

  // conversion
  QgsPolygon p1;
  ext.reset( new QgsLineString() );
  ext->setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 0, 0, 1, 5 )
                  << QgsPoint( QgsWkbTypes::PointZM, 0, 10, 2, 6 ) << QgsPoint( QgsWkbTypes::PointZM, 10, 10, 3, 7 ) );
  p1.setExteriorRing( ext.release() );
  //toPolygon
  std::unique_ptr< QgsPolygon > poly( t5.toPolygon() );
  QCOMPARE( *poly, p1 );
  //surfaceToPolygon
  std::unique_ptr< QgsPolygon > surface( t5.surfaceToPolygon() );
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

  //asGML2
  QgsTriangle exportTriangle( QgsPoint( 1, 2 ),
                              QgsPoint( 3, 4 ),
                              QgsPoint( 6, 5 ) );
  QgsTriangle exportTriangleZ( QgsPoint( 1, 2, 3 ),
                               QgsPoint( 11, 12, 13 ),
                               QgsPoint( 1, 12, 23 ) );
  QgsTriangle exportTriangleFloat( QgsPoint( 1 + 1 / 3.0, 2 + 2 / 3.0 ),
                                   QgsPoint( 3 + 1 / 3.0, 4 + 2 / 3.0 ),
                                   QgsPoint( 6 + 1 / 3.0, 5 + 2 / 3.0 ) );
  QDomDocument doc( QStringLiteral( "gml" ) );
  QString expectedGML2( QStringLiteral( "<Polygon xmlns=\"gml\"><outerBoundaryIs xmlns=\"gml\"><LinearRing xmlns=\"gml\"><coordinates xmlns=\"gml\" cs=\",\" ts=\" \">1,2 3,4 6,5 1,2</coordinates></LinearRing></outerBoundaryIs></Polygon>" ) );
  QGSCOMPAREGML( elemToString( exportTriangle.asGml2( doc ) ), expectedGML2 );
  QString expectedGML2prec3( QStringLiteral( "<Polygon xmlns=\"gml\"><outerBoundaryIs xmlns=\"gml\"><LinearRing xmlns=\"gml\"><coordinates xmlns=\"gml\" cs=\",\" ts=\" \">1.333,2.667 3.333,4.667 6.333,5.667 1.333,2.667</coordinates></LinearRing></outerBoundaryIs></Polygon>" ) );
  QGSCOMPAREGML( elemToString( exportTriangleFloat.asGml2( doc, 3 ) ), expectedGML2prec3 );
  QString expectedGML2empty( QStringLiteral( "<Polygon xmlns=\"gml\"/>" ) );
  QGSCOMPAREGML( elemToString( QgsTriangle().asGml2( doc ) ), expectedGML2empty );

  //asGML3
  QString expectedGML3( QStringLiteral( "<Triangle xmlns=\"gml\"><exterior xmlns=\"gml\"><LinearRing xmlns=\"gml\"><posList xmlns=\"gml\" srsDimension=\"2\">1 2 3 4 6 5 1 2</posList></LinearRing></exterior></Triangle>" ) );
  QCOMPARE( elemToString( exportTriangle.asGml3( doc ) ), expectedGML3 );
  QString expectedGML3prec3( QStringLiteral( "<Triangle xmlns=\"gml\"><exterior xmlns=\"gml\"><LinearRing xmlns=\"gml\"><posList xmlns=\"gml\" srsDimension=\"2\">1.333 2.667 3.333 4.667 6.333 5.667 1.333 2.667</posList></LinearRing></exterior></Triangle>" ) );
  QCOMPARE( elemToString( exportTriangleFloat.asGml3( doc, 3 ) ), expectedGML3prec3 );
  QString expectedGML3empty( QStringLiteral( "<Triangle xmlns=\"gml\"/>" ) );
  QGSCOMPAREGML( elemToString( QgsTriangle().asGml3( doc ) ), expectedGML3empty );
  QString expectedGML3Z( QStringLiteral( "<Triangle xmlns=\"gml\"><exterior xmlns=\"gml\"><LinearRing xmlns=\"gml\"><posList xmlns=\"gml\" srsDimension=\"3\">1 2 3 11 12 13 1 12 23 1 2 3</posList></LinearRing></exterior></Triangle>" ) );
  QCOMPARE( elemToString( exportTriangleZ.asGml3( doc ) ), expectedGML3Z );


  // lengths and angles
  QgsTriangle t7( QgsPoint( 0, 0 ), QgsPoint( 0, 5 ), QgsPoint( 5, 5 ) );

  QVector<double> a_tested, a_t7 = t7.angles();
  a_tested.append( M_PI / 4.0 );
  a_tested.append( M_PI / 2.0 );
  a_tested.append( M_PI / 4.0 );
  QGSCOMPARENEAR( a_tested.at( 0 ), a_t7.at( 0 ), 0.0001 );
  QGSCOMPARENEAR( a_tested.at( 1 ), a_t7.at( 1 ), 0.0001 );
  QGSCOMPARENEAR( a_tested.at( 2 ), a_t7.at( 2 ), 0.0001 );
  QVector<double> a_empty = QgsTriangle().angles();
  QVERIFY( a_empty.isEmpty() );

  QVector<double> l_tested, l_t7 = t7.lengths();
  l_tested.append( 5 );
  l_tested.append( 5 );
  l_tested.append( std::sqrt( 5 * 5 + 5 * 5 ) );
  QGSCOMPARENEAR( l_tested.at( 0 ), l_t7.at( 0 ), 0.0001 );
  QGSCOMPARENEAR( l_tested.at( 1 ), l_t7.at( 1 ), 0.0001 );
  QGSCOMPARENEAR( l_tested.at( 2 ), l_t7.at( 2 ), 0.0001 );
  QVector<double> l_empty = QgsTriangle().lengths();
  QVERIFY( l_empty.isEmpty() );

  // type of triangle
  // Empty triangle returns always false for the types, except isDegenerate
  QVERIFY( QgsTriangle().isDegenerate() );
  QVERIFY( !QgsTriangle().isRight() );
  QVERIFY( !QgsTriangle().isIsocele() );
  QVERIFY( !QgsTriangle().isScalene() );
  QVERIFY( !QgsTriangle().isEquilateral() );

  // type of triangle
  QVERIFY( !t7.isDegenerate() );
  QVERIFY( t7.isRight() );
  QVERIFY( t7.isIsocele() );
  QVERIFY( !t7.isScalene() );
  QVERIFY( !t7.isEquilateral() );

  QgsTriangle t8( QgsPoint( 7.2825, 4.2368 ), QgsPoint( 13.0058, 3.3218 ), QgsPoint( 9.2145, 6.5242 ) );
  // angles in radians 58.8978;31.1036;89.9985
  // length 5.79598;4.96279;2.99413
  QVERIFY( !t8.isDegenerate() );
  QVERIFY( t8.isRight() );
  QVERIFY( !t8.isIsocele() );
  QVERIFY( t8.isScalene() );
  QVERIFY( !t8.isEquilateral() );

  QgsTriangle t9( QgsPoint( 10, 10 ), QgsPoint( 16, 10 ), QgsPoint( 13, 15.1962 ) );
  QVERIFY( !t9.isDegenerate() );
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
  QVector<QgsLineString> alt = QgsTriangle().altitudes();
  QVERIFY( alt.isEmpty() );
  alt = t10.altitudes();
  QGSCOMPARENEARPOINT( alt.at( 0 ).pointN( 1 ), QgsPoint( 20.8276, 4.0690 ), 0.0001 );
  QGSCOMPARENEARPOINT( alt.at( 1 ).pointN( 1 ), QgsPoint( 16, 2 ), 0.0001 );
  QGSCOMPARENEARPOINT( alt.at( 2 ).pointN( 1 ), QgsPoint( 23, -1 ), 0.0001 );

  // orthocenter

  QCOMPARE( QgsPoint(), QgsTriangle().orthocenter() );
  QCOMPARE( QgsPoint( 16, -8 ), t10.orthocenter() );
  QCOMPARE( QgsPoint( 0, 5 ), t7.orthocenter() );
  QGSCOMPARENEARPOINT( QgsPoint( 13, 11.7321 ), t9.orthocenter(), 0.0001 );

  // circumscribed circle
  QCOMPARE( QgsPoint(), QgsTriangle().circumscribedCenter() );
  QCOMPARE( 0.0, QgsTriangle().circumscribedRadius() );
  QCOMPARE( QgsPoint( 2.5, 2.5 ), t7.circumscribedCenter() );
  QGSCOMPARENEAR( 3.5355, t7.circumscribedRadius(), 0.0001 );
  QCOMPARE( QgsPoint( 23, 9 ), t10.circumscribedCenter() );
  QGSCOMPARENEAR( 7.6158, t10.circumscribedRadius(), 0.0001 );
  QGSCOMPARENEARPOINT( QgsPoint( 13, 11.7321 ), t9.circumscribedCenter(), 0.0001 );
  QGSCOMPARENEAR( 3.4641, t9.circumscribedRadius(), 0.0001 );
  QGSCOMPARENEARPOINT( QgsPoint( 13, 11.7321 ), t9.circumscribedCircle().center(), 0.0001 );
  QGSCOMPARENEAR( 3.4641, t9.circumscribedCircle().radius(), 0.0001 );

  // inscribed circle
  QCOMPARE( QgsPoint(), QgsTriangle().inscribedCenter() );
  QCOMPARE( 0.0, QgsTriangle().inscribedRadius() );
  QGSCOMPARENEARPOINT( QgsPoint( 1.4645, 3.5355 ), t7.inscribedCenter(), 0.001 );
  QGSCOMPARENEAR( 1.4645, t7.inscribedRadius(), 0.0001 );
  QGSCOMPARENEARPOINT( QgsPoint( 20.4433, 3.0701 ), t10.inscribedCenter(), 0.001 );
  QGSCOMPARENEAR( 1.0701, t10.inscribedRadius(), 0.0001 );
  QGSCOMPARENEARPOINT( QgsPoint( 13, 11.7321 ), t9.inscribedCenter(), 0.0001 );
  QGSCOMPARENEAR( 1.7321, t9.inscribedRadius(), 0.0001 );
  QGSCOMPARENEARPOINT( QgsPoint( 13, 11.7321 ), t9.inscribedCircle().center(), 0.0001 );
  QGSCOMPARENEAR( 1.7321, t9.inscribedCircle().radius(), 0.0001 );

  // medians
  QVector<QgsLineString> med = QgsTriangle().medians();
  QVERIFY( med.isEmpty() );
  med = t7.medians();
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
  QCOMPARE( QgsTriangle().medial(), QgsTriangle() );
  QCOMPARE( t7.medial(), QgsTriangle( QgsPoint( 0, 2.5 ), QgsPoint( 2.5, 5 ), QgsPoint( 2.5, 2.5 ) ) );
  QCOMPARE( t9.medial(), QgsTriangle( QgsGeometryUtils::midpoint( t9.vertexAt( 0 ), t9.vertexAt( 1 ) ),
                                      QgsGeometryUtils::midpoint( t9.vertexAt( 1 ), t9.vertexAt( 2 ) ),
                                      QgsGeometryUtils::midpoint( t9.vertexAt( 2 ), t9.vertexAt( 0 ) ) ) );

  // bisectors
  QVector<QgsLineString> bis = QgsTriangle().bisectors();
  QVERIFY( bis.isEmpty() );
  bis = t7.bisectors();
  QCOMPARE( bis.at( 0 ).pointN( 0 ), t7.vertexAt( 0 ) );
  QGSCOMPARENEARPOINT( bis.at( 0 ).pointN( 1 ), QgsPoint( 2.0711, 5 ), 0.0001 );
  QCOMPARE( bis.at( 1 ).pointN( 0 ), t7.vertexAt( 1 ) );
  QGSCOMPARENEARPOINT( bis.at( 1 ).pointN( 1 ), QgsPoint( 2.5, 2.5 ), 0.0001 );
  QCOMPARE( bis.at( 2 ).pointN( 0 ), t7.vertexAt( 2 ) );
  QGSCOMPARENEARPOINT( bis.at( 2 ).pointN( 1 ), QgsPoint( 0, 2.9289 ), 0.0001 );

  // "deleted" method
  ext.reset( new QgsLineString() );
  QgsTriangle t11( QgsPoint( 0, 0 ), QgsPoint( 100, 100 ), QgsPoint( 0, 200 ) );
  ext->setPoints( QgsPointSequence() << QgsPoint( 5, 5 )
                  << QgsPoint( 50, 50 ) << QgsPoint( 0, 25 )
                  << QgsPoint( 5, 5 ) );
  t11.addInteriorRing( ext.release() );
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
  QVERIFY( t11.moveVertex( id, pt1 ) );
  pt1 = QgsPoint( 5, 5 );
  QVERIFY( t11.moveVertex( id, pt1 ) );
  // duplicate point
  pt1 = QgsPoint( 0, 0 );
  QVERIFY( t11.moveVertex( id, pt1 ) );
  pt1 = QgsPoint( 5, 5 );
  QVERIFY( t11.moveVertex( id, pt1 ) );

  //toCurveType
  QgsTriangle t12( QgsPoint( 7, 4 ), QgsPoint( 13, 3 ), QgsPoint( 9, 6 ) );
  std::unique_ptr< QgsCurvePolygon > curveType( t12.toCurveType() );
  QCOMPARE( curveType->wkbType(), QgsWkbTypes::CurvePolygon );
  QCOMPARE( curveType->exteriorRing()->numPoints(), 4 );
  QCOMPARE( curveType->exteriorRing()->vertexAt( QgsVertexId( 0, 0, 0 ) ), QgsPoint( 7, 4 ) );
  QCOMPARE( curveType->exteriorRing()->vertexAt( QgsVertexId( 0, 0, 1 ) ), QgsPoint( 13, 3 ) );
  QCOMPARE( curveType->exteriorRing()->vertexAt( QgsVertexId( 0, 0, 2 ) ), QgsPoint( 9, 6 ) );
  QCOMPARE( curveType->exteriorRing()->vertexAt( QgsVertexId( 0, 0, 3 ) ), QgsPoint( 7, 4 ) );
  QCOMPARE( curveType->numInteriorRings(), 0 );

  // boundary
  QVERIFY( !QgsTriangle().boundary() );
  std::unique_ptr< QgsCurve > boundary( QgsTriangle( QgsPoint( 7, 4 ), QgsPoint( 13, 3 ), QgsPoint( 9, 6 ) ).boundary() );
  QCOMPARE( boundary->wkbType(), QgsWkbTypes::LineString );
  QCOMPARE( boundary->numPoints(), 4 );
  QCOMPARE( boundary->vertexAt( QgsVertexId( 0, 0, 0 ) ), QgsPoint( 7, 4 ) );
  QCOMPARE( boundary->vertexAt( QgsVertexId( 0, 0, 1 ) ), QgsPoint( 13, 3 ) );
  QCOMPARE( boundary->vertexAt( QgsVertexId( 0, 0, 2 ) ), QgsPoint( 9, 6 ) );
  QCOMPARE( boundary->vertexAt( QgsVertexId( 0, 0, 3 ) ), QgsPoint( 7, 4 ) );

  // cast
  QgsTriangle pCast;
  QVERIFY( QgsPolygon().cast( &pCast ) );
  QgsTriangle pCast2( QgsPoint( 7, 4 ), QgsPoint( 13, 3 ), QgsPoint( 9, 6 ) );;
  QVERIFY( QgsPolygon().cast( &pCast2 ) );
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
  QVERIFY( QgsEllipse( QgsPoint( 0, 0 ), std::sqrt( 32.0 ), std::sqrt( 16.0 ), 90.0 ) == elp_hor );
  QGSCOMPARENEARPOINT( QgsPoint( 4, 0 ), elp_hor.foci().at( 0 ), 1e-8 );
  QGSCOMPARENEARPOINT( QgsPoint( -4, 0 ), elp_hor.foci().at( 1 ), 1e-8 );
  elp_hor = QgsEllipse().fromFoci( QgsPoint( 4, 0 ), QgsPoint( -4, 0 ), QgsPoint( 0, 4 ) );
  QVERIFY( QgsEllipse( QgsPoint( 0, 0 ), std::sqrt( 32.0 ), std::sqrt( 16.0 ), 270.0 ) == elp_hor );
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

  QVERIFY( QgsEllipse( QgsPoint( 0, 0 ), 5, 2, 0 ).points( 2 ).isEmpty() ); // segments too low

  // linestring
  std::unique_ptr< QgsLineString > l( new QgsLineString() );

  l.reset( QgsEllipse( QgsPoint( 0, 0 ), 5, 2, 0 ).toLineString( 2 ) );
  QVERIFY( l->isEmpty() ); // segments too low

  l.reset( QgsEllipse( QgsPoint( 0, 0 ), 5, 2, 0 ).toLineString( 4 ) );
  QCOMPARE( l->numPoints(), 5 ); // closed linestring
  QgsPointSequence pts_l;
  l->points( pts_l );
  pts_l.pop_back();
  QCOMPARE( pts, pts_l );

  // polygon
  std::unique_ptr< QgsPolygon > p1( new QgsPolygon() );

  p1.reset( QgsEllipse( QgsPoint( 0, 0 ), 5, 2, 0 ).toPolygon( 2 ) );
  QVERIFY( p1->isEmpty() ); // segments too low

  p1.reset( QgsEllipse( QgsPoint( 0, 0 ), 5, 2, 0 ).toPolygon( 4 ) );
  q = QgsEllipse( QgsPoint( 0, 0 ), 5, 2, 0 ).quadrant();
  QCOMPARE( p1->vertexAt( QgsVertexId( 0, 0, 0 ) ), q.at( 0 ) );
  QCOMPARE( p1->vertexAt( QgsVertexId( 0, 0, 1 ) ), q.at( 1 ) );
  QCOMPARE( p1->vertexAt( QgsVertexId( 0, 0, 2 ) ), q.at( 2 ) );
  QCOMPARE( p1->vertexAt( QgsVertexId( 0, 0, 3 ) ), q.at( 3 ) );
  QCOMPARE( p1->vertexAt( QgsVertexId( 0, 0, 4 ) ), q.at( 0 ) );
  QCOMPARE( 0, p1->numInteriorRings() );
  QCOMPARE( 5, p1->exteriorRing()->numPoints() );

  p1.reset( QgsEllipse( QgsPoint( 0, 0 ), 5, 2, 90 ).toPolygon( 4 ) );
  q = QgsEllipse( QgsPoint( 0, 0 ), 5, 2, 90 ).quadrant();
  QCOMPARE( p1->vertexAt( QgsVertexId( 0, 0, 0 ) ), q.at( 0 ) );
  QCOMPARE( p1->vertexAt( QgsVertexId( 0, 0, 1 ) ), q.at( 1 ) );
  QCOMPARE( p1->vertexAt( QgsVertexId( 0, 0, 2 ) ), q.at( 2 ) );
  QCOMPARE( p1->vertexAt( QgsVertexId( 0, 0, 3 ) ), q.at( 3 ) );
  QCOMPARE( p1->vertexAt( QgsVertexId( 0, 0, 4 ) ), q.at( 0 ) );
  QCOMPARE( 0, p1->numInteriorRings() );
  QCOMPARE( 5, p1->exteriorRing()->numPoints() );

  p1.reset( elpq.toPolygon( 4 ) );
  q = elpq.quadrant();
  QCOMPARE( p1->vertexAt( QgsVertexId( 0, 0, 0 ) ), q.at( 0 ) );
  QCOMPARE( p1->vertexAt( QgsVertexId( 0, 0, 1 ) ), q.at( 1 ) );
  QCOMPARE( p1->vertexAt( QgsVertexId( 0, 0, 2 ) ), q.at( 2 ) );
  QCOMPARE( p1->vertexAt( QgsVertexId( 0, 0, 3 ) ), q.at( 3 ) );
  QCOMPARE( p1->vertexAt( QgsVertexId( 0, 0, 4 ) ), q.at( 0 ) );
  QCOMPARE( 0, p1->numInteriorRings() );
  QCOMPARE( 5, p1->exteriorRing()->numPoints() );

  // oriented bounding box
  std::unique_ptr< QgsPolygon > ombb( QgsEllipse().orientedBoundingBox() );
  QVERIFY( ombb->isEmpty() );

  elpq = QgsEllipse( QgsPoint( 0, 0 ), 5, 2 );
  ombb.reset( new QgsPolygon() );
  QgsLineString *ext = new QgsLineString();
  ext->setPoints( QgsPointSequence() << QgsPoint( 5, 2 ) << QgsPoint( 5, -2 ) << QgsPoint( -5, -2 ) << QgsPoint( -5, 2 ) );
  ombb->setExteriorRing( ext );
  std::unique_ptr< QgsPolygon >ombb2( elpq.orientedBoundingBox() );
  QCOMPARE( ombb->asWkt( 2 ), ombb2->asWkt( 2 ) );

  elpq = QgsEllipse( QgsPoint( 0, 0 ), 5, 2.5, 45 );
  ombb.reset( elpq.orientedBoundingBox() );
  QGSCOMPARENEARPOINT( ombb->exteriorRing()->vertexAt( QgsVertexId( 0, 0, 0 ) ), QgsPoint( 1.7678, 5.3033 ), 0.0001 );
  QGSCOMPARENEARPOINT( ombb->exteriorRing()->vertexAt( QgsVertexId( 0, 0, 1 ) ), QgsPoint( 5.3033, 1.7678 ), 0.0001 );
  QGSCOMPARENEARPOINT( ombb->exteriorRing()->vertexAt( QgsVertexId( 0, 0, 2 ) ), QgsPoint( -1.7678, -5.3033 ), 0.0001 );
  QGSCOMPARENEARPOINT( ombb->exteriorRing()->vertexAt( QgsVertexId( 0, 0, 3 ) ), QgsPoint( -5.3033, -1.7678 ), 0.0001 );

  elpq = QgsEllipse( QgsPoint( 0, 0 ), 5, 2.5, 315 );
  ombb.reset( elpq.orientedBoundingBox() );
  QGSCOMPARENEARPOINT( ombb->exteriorRing()->vertexAt( QgsVertexId( 0, 0, 0 ) ), QgsPoint( -5.3033, 1.7678 ), 0.0001 );
  QGSCOMPARENEARPOINT( ombb->exteriorRing()->vertexAt( QgsVertexId( 0, 0, 1 ) ), QgsPoint( -1.7678, 5.3033 ), 0.0001 );
  QGSCOMPARENEARPOINT( ombb->exteriorRing()->vertexAt( QgsVertexId( 0, 0, 2 ) ), QgsPoint( 5.3033, -1.7678 ), 0.0001 );
  QGSCOMPARENEARPOINT( ombb->exteriorRing()->vertexAt( QgsVertexId( 0, 0, 3 ) ), QgsPoint( 1.7678, -5.3033 ), 0.0001 );

  // bounding box
  QCOMPARE( QgsEllipse().boundingBox(), QgsRectangle() );
  ombb.reset( QgsEllipse( QgsPoint( 0, 0 ), 5, 2 ).orientedBoundingBox() );
  QCOMPARE( QgsEllipse( QgsPoint( 0, 0 ), 5, 2 ).boundingBox(), ombb->boundingBox() );
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
  p1.reset( QgsEllipse( QgsPoint( 0, 0 ), 5, 2, 45 ).toPolygon( 10000 ) );
  QGSCOMPARENEAR( QgsEllipse( QgsPoint( 0, 0 ), 5, 2, 45 ).perimeter(), p1->perimeter(), 0.001 );

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
  QVERIFY( QgsCircle().from2Points( QgsPoint( -5, 0 ), QgsPoint( 5, 0 ) ) == QgsCircle( QgsPoint( 0, 0 ), 5, 90 ) );
  QVERIFY( QgsCircle().from2Points( QgsPoint( 0, -5 ), QgsPoint( 0, 5 ) ) == QgsCircle( QgsPoint( 0, 0 ), 5, 0 ) );
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
  QVERIFY( QgsCircle().fromCenterPoint( QgsPoint( 0, 0 ), QgsPoint( 5 * std::cos( 45 * M_PI / 180.0 ), 5 * std::sin( 45 * M_PI / 180.0 ) ) ) == QgsCircle( QgsPoint( 0, 0 ), 5, 45 ) );
// by3Tangents
// Tangents from triangle tri1( 0,0 ; 0,5 ), tri2( 0,0 ; 5,0 ), tri3( 5,0 ; 0,5 )
  QgsCircle circ_tgt = QgsCircle().from3Tangents( QgsPoint( 0, 0 ), QgsPoint( 0, 1 ), QgsPoint( 2, 0 ), QgsPoint( 3, 0 ), QgsPoint( 5, 0 ), QgsPoint( 0, 5 ) );
  QGSCOMPARENEARPOINT( circ_tgt.center(), QgsPoint( 1.4645, 1.4645 ), 0.0001 );
  QGSCOMPARENEAR( circ_tgt.radius(), 1.4645, 0.0001 );
  // with parallels
  circ_tgt = QgsCircle().from3Tangents( QgsPoint( 0, 0 ), QgsPoint( 0, 5 ), QgsPoint( 1, 0 ), QgsPoint( 1, 5 ), QgsPoint( 5, 0 ), QgsPoint( 0, 5 ) );
  QVERIFY( circ_tgt.isEmpty() );
  circ_tgt = QgsCircle().from3Tangents( QgsPoint( 0, 0 ), QgsPoint( 0, 5 ), QgsPoint( 5, 0 ), QgsPoint( 0, 5 ), QgsPoint( 1, 0 ), QgsPoint( 1, 5 ) );
  QVERIFY( circ_tgt.isEmpty() );
  circ_tgt = QgsCircle().from3Tangents( QgsPoint( 5, 0 ), QgsPoint( 0, 5 ), QgsPoint( 0, 0 ), QgsPoint( 0, 5 ), QgsPoint( 1, 0 ), QgsPoint( 1, 5 ) );
  QVERIFY( circ_tgt.isEmpty() );

  // check that Z dimension is ignored in case of using tangents
  QgsCircle circ_tgt_z = QgsCircle().from3Tangents( QgsPoint( 0, 0, 333 ), QgsPoint( 0, 1, 1 ), QgsPoint( 2, 0, 2 ), QgsPoint( 3, 0, 3 ), QgsPoint( 5, 0, 4 ), QgsPoint( 0, 5, 5 ) );
  QCOMPARE( circ_tgt_z.center().is3D(), false );

  // minimalCircleFrom3points
  QgsCircle minCircle3Points = QgsCircle().minimalCircleFrom3Points( QgsPoint( 0, 5 ), QgsPoint( 0, -5 ), QgsPoint( 1, 2 ) );
  QGSCOMPARENEARPOINT( minCircle3Points.center(), QgsPoint( 0, 0 ), 0.0001 );
  QGSCOMPARENEAR( minCircle3Points.radius(), 5.0, 0.0001 );
  minCircle3Points = QgsCircle().minimalCircleFrom3Points( QgsPoint( 0, 5 ), QgsPoint( 5, 0 ), QgsPoint( -5, 0 ) );
  QGSCOMPARENEARPOINT( minCircle3Points.center(), QgsPoint( 0, 0 ), 0.0001 );
  QGSCOMPARENEAR( minCircle3Points.radius(), 5.0, 0.0001 );



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
  std::unique_ptr< QgsPolygon > pol( new QgsPolygon() );
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
  double val = 5 * std::sin( M_PI / 4 );
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
  std::unique_ptr< QgsCircularString > cs( QgsCircle( QgsPoint( 0, 0 ), 5 ).toCircularString() );
  QCOMPARE( cs->asWkt( 2 ), QString( "CircularString (0 5, 5 0, 0 -5, -5 0, 0 5)" ) );
  cs.reset( QgsCircle( QgsPoint( 0, 0 ), 5 ).toCircularString( true ) );
  QCOMPARE( cs->asWkt( 2 ), QString( "CircularString (0 5, 5 0, 0 -5, -5 -0, 0 5)" ) );
  cs.reset( QgsCircle( QgsPoint( 0, 0 ), 5, 315 ).toCircularString() );
  QCOMPARE( cs->asWkt( 2 ), QString( "CircularString (0 5, 5 0, 0 -5, -5 0, 0 5)" ) );
  cs.reset( QgsCircle( QgsPoint( 0, 0 ), 5, 315 ).toCircularString( true ) );
  QCOMPARE( cs->asWkt( 2 ), QString( "CircularString (-3.54 3.54, 3.54 3.54, 3.54 -3.54, -3.54 -3.54, -3.54 3.54)" ) );

// bounding box
  QVERIFY( QgsRectangle( QgsPointXY( -2.5, -2.5 ), QgsPointXY( 2.5, 2.5 ) ) == QgsCircle( QgsPoint( 0, 0 ), 2.5, 0 ).boundingBox() );
  QVERIFY( QgsRectangle( QgsPointXY( -2.5, -2.5 ), QgsPointXY( 2.5, 2.5 ) ) == QgsCircle( QgsPoint( 0, 0 ), 2.5, 45 ).boundingBox() );

  // area
  QGSCOMPARENEAR( 314.1593, QgsCircle( QgsPoint( 0, 0 ), 10 ).area(), 0.0001 );
  // perimeter
  QGSCOMPARENEAR( 31.4159, QgsCircle( QgsPoint( 0, 0 ), 5 ).perimeter(), 0.0001 );

  // contains
  QgsPoint pc;
  pc = QgsPoint( 1, 1 );
  QVERIFY( QgsCircle( QgsPoint( 0, 0 ), 5 ).contains( pc ) );
  pc = QgsPoint( 0, 5 );
  QVERIFY( QgsCircle( QgsPoint( 0, 0 ), 5 ).contains( pc ) );
  pc = QgsPoint( 6, 1 );
  QVERIFY( !QgsCircle( QgsPoint( 0, 0 ), 5 ).contains( pc ) );

  // intersections
  QgsCircle ci1( QgsPoint( 0, 0 ), 1 );
  QgsPoint int1;
  QgsPoint int2;
  QCOMPARE( ci1.intersections( QgsCircle( QgsPoint( 2, 0 ), 0.5 ), int1, int2 ), 0 );
  QCOMPARE( ci1.intersections( QgsCircle( QgsPoint( 0.5, 0.1 ), 0.2 ), int1, int2 ), 0 );
  // one intersection
  QCOMPARE( ci1.intersections( QgsCircle( QgsPoint( 3, 0 ), 2 ), int1, int2 ), 1 );
  QCOMPARE( int1, QgsPoint( 1, 0 ) );
  QCOMPARE( int2, QgsPoint( 1, 0 ) );
  // two intersections
  ci1 = QgsCircle( QgsPoint( 5, 3 ), 2 );
  QCOMPARE( ci1.intersections( QgsCircle( QgsPoint( 7, -1 ), 4 ), int1, int2 ), 2 );
  QCOMPARE( int1.wkbType(), QgsWkbTypes::Point );
  QGSCOMPARENEAR( int1.x(), 3.8, 0.001 );
  QGSCOMPARENEAR( int1.y(), 1.4, 0.001 );
  QCOMPARE( int2.wkbType(), QgsWkbTypes::Point );
  QGSCOMPARENEAR( int2.x(), 7.0, 0.001 );
  QGSCOMPARENEAR( int2.y(), 3.0, 0.001 );
  // with z
  ci1 = QgsCircle( QgsPoint( 5, 3, 11 ), 2 );
  QCOMPARE( ci1.intersections( QgsCircle( QgsPoint( 7, -1, 5 ), 4 ), int1, int2, true ), 0 );
  QCOMPARE( ci1.intersections( QgsCircle( QgsPoint( 7, -1, 11 ), 4 ), int1, int2, true ), 2 );
  QCOMPARE( int1.wkbType(), QgsWkbTypes::PointZ );
  QGSCOMPARENEAR( int1.x(), 3.8, 0.001 );
  QGSCOMPARENEAR( int1.y(), 1.4, 0.001 );
  QGSCOMPARENEAR( int1.z(), 11.0, 0.001 );
  QCOMPARE( int2.wkbType(), QgsWkbTypes::PointZ );
  QGSCOMPARENEAR( int2.x(), 7.0, 0.001 );
  QGSCOMPARENEAR( int2.y(), 3.0, 0.001 );
  QGSCOMPARENEAR( int2.z(), 11.0, 0.001 );

  // tangent to point
  QgsPointXY t1;
  QgsPointXY t2;
  QVERIFY( !QgsCircle( QgsPoint( 1, 2 ), 4 ).tangentToPoint( QgsPointXY( 1, 2 ), t1, t2 ) );
  QVERIFY( QgsCircle( QgsPoint( 1, 2 ), 4 ).tangentToPoint( QgsPointXY( 8, 4 ), t1, t2 ) );
  QGSCOMPARENEAR( t1.x(), 4.03, 0.01 );
  QGSCOMPARENEAR( t1.y(), -0.61, 0.01 );
  QGSCOMPARENEAR( t2.x(), 2.2, 0.01 );
  QGSCOMPARENEAR( t2.y(), 5.82, 0.01 );

  // two outer circle tangents
  QgsPointXY l1p1, l1p2, l2p1, l2p2;
  QCOMPARE( QgsCircle( QgsPoint( 1, 2 ), 4 ).outerTangents( QgsCircle( QgsPoint( 2, 3 ), 1 ), l1p1, l1p2, l2p1, l2p2 ), 0 );
  QCOMPARE( QgsCircle( QgsPoint( 1, 2 ), 1 ).outerTangents( QgsCircle( QgsPoint( 10, 3 ), 4 ), l1p1, l1p2, l2p1, l2p2 ), 2 );
  QGSCOMPARENEAR( l1p1.x(), 0.566, 0.01 );
  QGSCOMPARENEAR( l1p1.y(), 2.901, 0.01 );
  QGSCOMPARENEAR( l1p2.x(), 8.266, 0.01 );
  QGSCOMPARENEAR( l1p2.y(), 6.604, 0.01 );
  QGSCOMPARENEAR( l2p1.x(), 0.7749, 0.01 );
  QGSCOMPARENEAR( l2p1.y(), 1.025, 0.01 );
  QGSCOMPARENEAR( l2p2.x(), 9.099, 0.01 );
  QGSCOMPARENEAR( l2p2.y(), -0.897, 0.01 );

  // two inner circle tangents
  QCOMPARE( QgsCircle( QgsPoint( 1, 2 ), 4 ).innerTangents( QgsCircle( QgsPoint( 2, 3 ), 1 ), l1p1, l1p2, l2p1, l2p2 ), 0 );
  QCOMPARE( QgsCircle( QgsPoint( 0, 0 ), 4 ).innerTangents( QgsCircle( QgsPoint( 8, 0 ), 5 ), l1p1, l1p2, l2p1, l2p2 ), 0 );
  QCOMPARE( QgsCircle( QgsPoint( 0, 0 ), 4 ).innerTangents( QgsCircle( QgsPoint( 8, 0 ), 4 ), l1p1, l1p2, l2p1, l2p2 ), 0 );
  QCOMPARE( QgsCircle( QgsPoint( 1, 2 ), 1 ).innerTangents( QgsCircle( QgsPoint( 10, 3 ), 4 ), l1p1, l1p2, l2p1, l2p2 ), 2 );
  QGSCOMPARENEAR( l1p1.x(), 7.437, 0.01 );
  QGSCOMPARENEAR( l1p1.y(), 6.071, 0.01 );
  QGSCOMPARENEAR( l1p2.x(), 1.641, 0.01 );
  QGSCOMPARENEAR( l1p2.y(), 1.232, 0.01 );
  QGSCOMPARENEAR( l2p1.x(), 8.173, 0.01 );
  QGSCOMPARENEAR( l2p1.y(), -0.558, 0.01 );
  QGSCOMPARENEAR( l2p2.x(), 1.457, 0.01 );
  QGSCOMPARENEAR( l2p2.y(), 2.89, 0.01 );
}

void TestQgsGeometry::quadrilateral()
{

  // default
  QgsQuadrilateral quad_init;
  QgsPointSequence pts = quad_init.points();
  QCOMPARE( pts.at( 0 ), QgsPoint() );
  QCOMPARE( pts.at( 1 ), QgsPoint() );
  QCOMPARE( pts.at( 2 ), QgsPoint() );
  QCOMPARE( pts.at( 3 ), QgsPoint() );
  QVERIFY( !quad_init.isValid() );

  // colinear
  QgsQuadrilateral quad4points_col( QgsPoint( 0, 0 ), QgsPoint( 0, 5 ), QgsPoint( 0, 10 ), QgsPoint( 10, 10 ) );
  QVERIFY( !quad4points_col.isValid() );
  pts = quad4points_col.points();
  QCOMPARE( pts.at( 0 ), QgsPoint() );
  QCOMPARE( pts.at( 1 ), QgsPoint() );
  QCOMPARE( pts.at( 2 ), QgsPoint() );
  QCOMPARE( pts.at( 3 ), QgsPoint() );


  QgsQuadrilateral quad4pointsXY_col( QgsPoint( 0, 0 ), QgsPoint( 0, 5 ), QgsPoint( 0, 10 ), QgsPoint( 10, 10 ) );
  QVERIFY( !quad4pointsXY_col.isValid() );
  pts = quad4pointsXY_col.points();
  QCOMPARE( pts.at( 0 ), QgsPoint() );
  QCOMPARE( pts.at( 1 ), QgsPoint() );
  QCOMPARE( pts.at( 2 ), QgsPoint() );
  QCOMPARE( pts.at( 3 ), QgsPoint() );


  // anti parallelogram
  QgsQuadrilateral quad4points_anti( QgsPoint( 0, 0 ), QgsPoint( 5, 5 ), QgsPoint( 5, 0 ), QgsPoint( 0, 5 ) );
  QVERIFY( !quad4points_anti.isValid() );
  pts = quad4points_anti.points();
  QCOMPARE( pts.at( 0 ), QgsPoint() );
  QCOMPARE( pts.at( 1 ), QgsPoint() );
  QCOMPARE( pts.at( 2 ), QgsPoint() );
  QCOMPARE( pts.at( 3 ), QgsPoint() );


  QgsQuadrilateral quad4pointsXY_anti( QgsPoint( 0, 0 ), QgsPoint( 5, 5 ), QgsPoint( 5, 0 ), QgsPoint( 0, 5 ) );
  QVERIFY( !quad4pointsXY_anti.isValid() );
  pts = quad4pointsXY_anti.points();
  QCOMPARE( pts.at( 0 ), QgsPoint() );
  QCOMPARE( pts.at( 1 ), QgsPoint() );
  QCOMPARE( pts.at( 2 ), QgsPoint() );
  QCOMPARE( pts.at( 3 ), QgsPoint() );

  // valid
  QgsQuadrilateral quad4points_valid( QgsPoint( 0, 0 ), QgsPoint( 0, 5 ), QgsPoint( 5, 5 ), QgsPoint( 5, 0 ) );
  QVERIFY( quad4points_valid.isValid() );
  pts = quad4points_valid.points();
  QCOMPARE( pts.at( 0 ), QgsPoint( 0, 0 ) );
  QCOMPARE( pts.at( 1 ), QgsPoint( 0, 5 ) );
  QCOMPARE( pts.at( 2 ), QgsPoint( 5, 5 ) );
  QCOMPARE( pts.at( 3 ), QgsPoint( 5, 0 ) );

  // setPoint
  QVERIFY( quad4points_valid.setPoint( QgsPoint( -1, -1 ), QgsQuadrilateral::Point1 ) );
  QVERIFY( quad4points_valid.setPoint( QgsPoint( -1, 6 ), QgsQuadrilateral::Point2 ) );
  QVERIFY( quad4points_valid.setPoint( QgsPoint( 6, 6 ), QgsQuadrilateral::Point3 ) );
  QVERIFY( quad4points_valid.setPoint( QgsPoint( 6, -1 ), QgsQuadrilateral::Point4 ) );
  QVERIFY( quad4points_valid.isValid() );
  pts = quad4points_valid.points();
  QCOMPARE( pts.at( 0 ), QgsPoint( -1, -1 ) );
  QCOMPARE( pts.at( 1 ), QgsPoint( -1, 6 ) );
  QCOMPARE( pts.at( 2 ), QgsPoint( 6, 6 ) );
  QCOMPARE( pts.at( 3 ), QgsPoint( 6, -1 ) );

  // invalid: must have same type
  QVERIFY( !quad4points_valid.setPoint( QgsPoint( -1, -1, 10 ), QgsQuadrilateral::Point1 ) );
  QVERIFY( !quad4points_valid.setPoint( QgsPoint( -1, 6, 10 ), QgsQuadrilateral::Point2 ) );
  QVERIFY( !quad4points_valid.setPoint( QgsPoint( 6, 6, 10 ), QgsQuadrilateral::Point3 ) );
  QVERIFY( !quad4points_valid.setPoint( QgsPoint( 6, -1, 10 ), QgsQuadrilateral::Point4 ) );

  // invalid self-intersection
  QVERIFY( !quad4points_valid.setPoint( QgsPoint( 7, 3 ), QgsQuadrilateral::Point1 ) );
  QVERIFY( !quad4points_valid.setPoint( QgsPoint( 3, 7 ), QgsQuadrilateral::Point1 ) );
  QVERIFY( !quad4points_valid.setPoint( QgsPoint( 3, -7 ), QgsQuadrilateral::Point2 ) );
  QVERIFY( !quad4points_valid.setPoint( QgsPoint( 7, 3 ), QgsQuadrilateral::Point2 ) );
  QVERIFY( !quad4points_valid.setPoint( QgsPoint( 3, -7 ), QgsQuadrilateral::Point3 ) );
  QVERIFY( !quad4points_valid.setPoint( QgsPoint( -7, 3 ), QgsQuadrilateral::Point3 ) );
  QVERIFY( !quad4points_valid.setPoint( QgsPoint( 3, 7 ), QgsQuadrilateral::Point4 ) );
  QVERIFY( !quad4points_valid.setPoint( QgsPoint( -7, 3 ), QgsQuadrilateral::Point4 ) );

  // invalid colinear
  QVERIFY( !quad4points_valid.setPoint( QgsPoint( 6, -2 ), QgsQuadrilateral::Point1 ) );
  QVERIFY( !quad4points_valid.setPoint( QgsPoint( -2, 6 ), QgsQuadrilateral::Point1 ) );
  QVERIFY( !quad4points_valid.setPoint( QgsPoint( 6, 7 ), QgsQuadrilateral::Point2 ) );
  QVERIFY( !quad4points_valid.setPoint( QgsPoint( -2, -1 ), QgsQuadrilateral::Point2 ) );
  QVERIFY( !quad4points_valid.setPoint( QgsPoint( 7, -1 ), QgsQuadrilateral::Point3 ) );
  QVERIFY( !quad4points_valid.setPoint( QgsPoint( -1, 7 ), QgsQuadrilateral::Point3 ) );
  QVERIFY( !quad4points_valid.setPoint( QgsPoint( -1, -2 ), QgsQuadrilateral::Point4 ) );
  QVERIFY( !quad4points_valid.setPoint( QgsPoint( 7, 6 ), QgsQuadrilateral::Point4 ) );

//equals
  QVERIFY( QgsQuadrilateral( QgsPoint( 0, 0 ), QgsPoint( 0, 5 ), QgsPoint( 5, 5 ), QgsPoint( 5, 0 ) ) !=
           QgsQuadrilateral( QgsPoint( 0.01, 0.01 ), QgsPoint( 0.01, 5.01 ), QgsPoint( 5.01, 5.01 ), QgsPoint( 5.01, 0.01 ) ) );
  QVERIFY( QgsQuadrilateral( QgsPoint( 0, 0 ), QgsPoint( 0, 5 ), QgsPoint( 5, 5 ), QgsPoint( 5, 0 ) ).equals(
             QgsQuadrilateral( QgsPoint( 0.01, 0.01 ), QgsPoint( 0.01, 5.01 ), QgsPoint( 5.01, 5.01 ), QgsPoint( 5.01, 0.01 ) ), 1e-1 ) );
  QVERIFY( QgsQuadrilateral( QgsPoint( 0, 0, 0 ), QgsPoint( 0, 5, -0.02 ), QgsPoint( 5, 5, 0 ), QgsPoint( 5, 0, -0.02 ) ).equals(
             QgsQuadrilateral( QgsPoint( 0.01, 0.01, 0.01 ), QgsPoint( 0.01, 5.01, 0 ), QgsPoint( 5.01, 5.01, -0.01 ), QgsPoint( 5.01, 0.01, 0.04 ) ), 1e-1 ) );

// rectangleFromExtent
  QgsQuadrilateral rectangleFromExtent( QgsPoint( 0, 0 ), QgsPoint( 0, 5 ), QgsPoint( 5, 5 ), QgsPoint( 5, 0 ) );
  QgsQuadrilateral rectangleFromExtentZ( QgsPoint( 0, 0, 10 ), QgsPoint( 0, 5, 10 ), QgsPoint( 5, 5, 10 ), QgsPoint( 5, 0, 10 ) );
  QCOMPARE( QgsQuadrilateral::rectangleFromExtent( QgsPoint( 0, 0 ), QgsPoint( 0, 0 ) ), QgsQuadrilateral() );
  QCOMPARE( QgsQuadrilateral::rectangleFromExtent( QgsPoint( 0, 0 ), QgsPoint( 0, 10 ) ), QgsQuadrilateral() );
  QCOMPARE( QgsQuadrilateral::rectangleFromExtent( QgsPoint( 0, 0 ), QgsPoint( 5, 5 ) ), rectangleFromExtent );
  QCOMPARE( QgsQuadrilateral::rectangleFromExtent( QgsPoint( 5, 5 ), QgsPoint( 0, 0 ) ), rectangleFromExtent );
  QCOMPARE( QgsQuadrilateral::rectangleFromExtent( QgsPoint( 0, 0, 10 ), QgsPoint( 5, 5 ) ), rectangleFromExtentZ );
  QVERIFY( QgsQuadrilateral::rectangleFromExtent( QgsPoint( 0, 0 ), QgsPoint( 5, 5, 10 ) ) != rectangleFromExtentZ );
  QCOMPARE( QgsQuadrilateral::rectangleFromExtent( QgsPoint( 0, 0 ), QgsPoint( 5, 5, 10 ) ), rectangleFromExtent );

// squareFromDiagonal
  QgsQuadrilateral squareFromDiagonal( QgsPoint( 0, 0 ), QgsPoint( 0, 5 ), QgsPoint( 5, 5 ), QgsPoint( 5, 0 ) );
  QgsQuadrilateral squareFromDiagonalZ( QgsPoint( 0, 0, 10 ), QgsPoint( 0, 5, 10 ), QgsPoint( 5, 5, 10 ), QgsPoint( 5, 0, 10 ) );
  QgsQuadrilateral squareFromDiagonalInv( QgsPoint( 5, 5 ), QgsPoint( 5, 0 ), QgsPoint( 0, 0 ), QgsPoint( 0, 5 ) );
  QCOMPARE( QgsQuadrilateral::squareFromDiagonal( QgsPoint( 0, 0 ), QgsPoint( 0, 0 ) ), QgsQuadrilateral() );
  QCOMPARE( QgsQuadrilateral::squareFromDiagonal( QgsPoint( 0, 0 ), QgsPoint( 5, 5 ) ), squareFromDiagonal );
  QVERIFY( QgsQuadrilateral::squareFromDiagonal( QgsPoint( 5, 5 ), QgsPoint( 0, 0 ) ) != squareFromDiagonal );
  QVERIFY( QgsQuadrilateral::squareFromDiagonal( QgsPoint( 5, 5 ), QgsPoint( 0, 0 ) ).equals( squareFromDiagonalInv, 1E-8 ) );
  QCOMPARE( QgsQuadrilateral::squareFromDiagonal( QgsPoint( 0, 0, 10 ), QgsPoint( 5, 5 ) ), squareFromDiagonalZ );
  QVERIFY( QgsQuadrilateral::squareFromDiagonal( QgsPoint( 0, 0 ), QgsPoint( 5, 5, 10 ) ) != squareFromDiagonalZ );
  QCOMPARE( QgsQuadrilateral::squareFromDiagonal( QgsPoint( 0, 0 ), QgsPoint( 5, 5, 10 ) ), squareFromDiagonal );

// rectangleFromCenterPoint
  QgsQuadrilateral rectangleFromCenterPoint( QgsPoint( 0, 0 ), QgsPoint( 0, 5 ), QgsPoint( 5, 5 ), QgsPoint( 5, 0 ) );
  QgsQuadrilateral rectangleFromCenterPointZ( QgsPoint( 0, 0, 10 ), QgsPoint( 0, 5, 10 ), QgsPoint( 5, 5, 10 ), QgsPoint( 5, 0, 10 ) );
  QCOMPARE( QgsQuadrilateral::rectangleFromCenterPoint( QgsPoint( 2.5, 2.5 ), QgsPoint( 2.5, 2.5 ) ), QgsQuadrilateral() ) ;
  QCOMPARE( QgsQuadrilateral::rectangleFromCenterPoint( QgsPoint( 2.5, 2.5 ), QgsPoint( 5, 5 ) ), rectangleFromCenterPoint ) ;
  QCOMPARE( QgsQuadrilateral::rectangleFromCenterPoint( QgsPoint( 2.5, 2.5 ), QgsPoint( 5, 0 ) ), rectangleFromCenterPoint ) ;
  QCOMPARE( QgsQuadrilateral::rectangleFromCenterPoint( QgsPoint( 2.5, 2.5 ), QgsPoint( 0, 5 ) ), rectangleFromCenterPoint ) ;
  QCOMPARE( QgsQuadrilateral::rectangleFromCenterPoint( QgsPoint( 2.5, 2.5 ), QgsPoint( 0, 0 ) ), rectangleFromCenterPoint ) ;
  QCOMPARE( QgsQuadrilateral::rectangleFromCenterPoint( QgsPoint( 2.5, 2.5, 10 ), QgsPoint( 5, 5 ) ), rectangleFromCenterPointZ ) ;
  QCOMPARE( QgsQuadrilateral::rectangleFromCenterPoint( QgsPoint( 2.5, 2.5, 10 ), QgsPoint( 5, 0 ) ), rectangleFromCenterPointZ ) ;
  QCOMPARE( QgsQuadrilateral::rectangleFromCenterPoint( QgsPoint( 2.5, 2.5, 10 ), QgsPoint( 0, 5 ) ), rectangleFromCenterPointZ ) ;
  QCOMPARE( QgsQuadrilateral::rectangleFromCenterPoint( QgsPoint( 2.5, 2.5, 10 ), QgsPoint( 0, 0 ) ), rectangleFromCenterPointZ ) ;
  QCOMPARE( QgsQuadrilateral::rectangleFromCenterPoint( QgsPoint( 2.5, 2.5 ), QgsPoint( 0, 0, 10 ) ), rectangleFromCenterPoint ) ;

// fromRectangle
  QgsQuadrilateral fromRectangle( QgsPoint( 0, 0 ), QgsPoint( 0, 5 ), QgsPoint( 5, 5 ), QgsPoint( 5, 0 ) );
  QCOMPARE( QgsQuadrilateral::fromRectangle( QgsRectangle( QgsPointXY( 0, 0 ), QgsPointXY( 0, 0 ) ) ), QgsQuadrilateral() );
  QCOMPARE( QgsQuadrilateral::fromRectangle( QgsRectangle( QgsPointXY( 0, 0 ), QgsPointXY( 5, 5 ) ) ), fromRectangle ) ;
  QCOMPARE( QgsQuadrilateral::fromRectangle( QgsRectangle( QgsPointXY( 5, 5 ), QgsPointXY( 0, 0 ) ) ), fromRectangle ) ;
// rectangleFrom3points
  QgsQuadrilateral rectangleFrom3Points( QgsPoint( 0, 0 ), QgsPoint( 0, 5 ), QgsPoint( 5, 5 ), QgsPoint( 5, 0 ) );
  QCOMPARE( QgsQuadrilateral::rectangleFrom3Points( QgsPoint( 0, 0 ), QgsPoint( 0, 5 ), QgsPoint( 0, 5 ), QgsQuadrilateral::Distance ), QgsQuadrilateral() );
  QCOMPARE( QgsQuadrilateral::rectangleFrom3Points( QgsPoint( 0, 0 ), QgsPoint( 0, 5 ), QgsPoint( 0, 5 ), QgsQuadrilateral::Projected ), QgsQuadrilateral() );

  QCOMPARE( QgsQuadrilateral::rectangleFrom3Points( QgsPoint( 0, 0 ), QgsPoint( 0, 5 ), QgsPoint( 5, 5 ), QgsQuadrilateral::Distance ).toLineString()->asWkt( 0 ),
            QString( "LineString (0 0, 0 5, 5 5, 5 0, 0 0)" ) );
  QCOMPARE( QgsQuadrilateral::rectangleFrom3Points( QgsPoint( 0, 0, 10 ), QgsPoint( 0, 5 ), QgsPoint( 5, 5 ), QgsQuadrilateral::Distance ).toLineString()->asWkt( 0 ),
            QString( "LineStringZ (0 0 10, 0 5 10, 5 5 10, 5 0 10, 0 0 10)" ) );
  QCOMPARE( QgsQuadrilateral::rectangleFrom3Points( QgsPoint( 0, 0 ), QgsPoint( 0, 5, 10 ), QgsPoint( 5, 5 ), QgsQuadrilateral::Distance ).toLineString()->asWkt( 0 ),
            QString( "LineStringZ (0 0 10, 0 5 10, 5 5 10, 5 0 10, 0 0 10)" ) );
  QCOMPARE( QgsQuadrilateral::rectangleFrom3Points( QgsPoint( 0, 0 ), QgsPoint( 0, 5 ), QgsPoint( 5, 5, 10 ), QgsQuadrilateral::Distance ).toLineString()->asWkt( 0 ),
            QString( "LineStringZ (0 0 10, 0 5 10, 5 5 10, 5 0 10, 0 0 10)" ) );

  QCOMPARE( QgsQuadrilateral::rectangleFrom3Points( QgsPoint( 0, 0 ), QgsPoint( 0, 5 ), QgsPoint( 5, 4 ), QgsQuadrilateral::Projected ).toLineString()->asWkt( 0 ),
            QString( "LineString (0 0, 0 5, 5 5, 5 0, 0 0)" ) );
  QCOMPARE( QgsQuadrilateral::rectangleFrom3Points( QgsPoint( 0, 0, 10 ), QgsPoint( 0, 5 ), QgsPoint( 5, 4 ), QgsQuadrilateral::Projected ).toLineString()->asWkt( 0 ),
            QString( "LineStringZ (0 0 10, 0 5 10, 5 5 10, 5 0 10, 0 0 10)" ) );
  QCOMPARE( QgsQuadrilateral::rectangleFrom3Points( QgsPoint( 0, 0 ), QgsPoint( 0, 5, 10 ), QgsPoint( 5, 4 ), QgsQuadrilateral::Projected ).toLineString()->asWkt( 0 ),
            QString( "LineStringZ (0 0 10, 0 5 10, 5 5 10, 5 0 10, 0 0 10)" ) );
  QCOMPARE( QgsQuadrilateral::rectangleFrom3Points( QgsPoint( 0, 0 ), QgsPoint( 0, 5 ), QgsPoint( 5, 4, 10 ), QgsQuadrilateral::Projected ).toLineString()->asWkt( 0 ),
            QString( "LineStringZ (0 0 10, 0 5 10, 5 5 10, 5 0 10, 0 0 10)" ) );
  QCOMPARE( QgsQuadrilateral::rectangleFrom3Points( QgsPoint( 0, 0, 10 ), QgsPoint( 0, 5, 10 ), QgsPoint( 5, 4, 10 ), QgsQuadrilateral::Projected ).toLineString()->asWkt( 0 ),
            QString( "LineStringZ (0 0 10, 0 5 10, 5 5 10, 5 0 10, 0 0 10)" ) );
  QCOMPARE( QgsQuadrilateral::rectangleFrom3Points( QgsPoint( 0, 0, 5 ), QgsPoint( 0, 5, 5 ), QgsPoint( 5, 5, 0 ), QgsQuadrilateral::Projected ).toString( 2 ),
            QString( "Quadrilateral (Point 1: PointZ (0 0 5), Point 2: PointZ (0 5 5), Point 3: PointZ (5 5 0), Point 4: PointZ (5 0 0))" ) );
  QCOMPARE( QgsQuadrilateral::rectangleFrom3Points( QgsPoint( 0, 0, 5 ), QgsPoint( 0, 5, 5 ), QgsPoint( 5, 5, 10 ), QgsQuadrilateral::Projected ).toString( 2 ),
            QString( "Quadrilateral (Point 1: PointZ (0 0 5), Point 2: PointZ (0 5 5), Point 3: PointZ (5 5 10), Point 4: PointZ (5 0 10))" ) );
// toString
  QCOMPARE( QgsQuadrilateral( ).toString(), QString( "Empty" ) );
  QCOMPARE( QgsQuadrilateral::rectangleFromCenterPoint( QgsPoint( 2.5, 2.5 ), QgsPoint( 2.5, 2.5 ) ).toString(), QString( "Empty" ) );
  QCOMPARE( QgsQuadrilateral::rectangleFromCenterPoint( QgsPoint( 2.5, 2.5 ), QgsPoint( 5, 0 ) ).toString(), QString( "Quadrilateral (Point 1: Point (0 0), Point 2: Point (0 5), Point 3: Point (5 5), Point 4: Point (5 0))" ) );
  QCOMPARE( QgsQuadrilateral::rectangleFromCenterPoint( QgsPoint( 2.5, 2.5, 10 ), QgsPoint( 5, 0 ) ).toString(), QString( "Quadrilateral (Point 1: PointZ (0 0 10), Point 2: PointZ (0 5 10), Point 3: PointZ (5 5 10), Point 4: PointZ (5 0 10))" ) );

// toPolygon / toLineString
  QCOMPARE( quad_init.toPolygon()->asWkt(), QgsPolygon().asWkt() );
  QCOMPARE( quad_init.toLineString()->asWkt(), QgsLineString().asWkt() );
  QgsLineString ext, extZ;
  QgsPolygon polyg, polygZ;
  QgsQuadrilateral quad( QgsPoint( 0, 0 ), QgsPoint( 0, 5 ), QgsPoint( 5, 5 ), QgsPoint( 5, 0 ) );
  QgsQuadrilateral quadZ( QgsPoint( 0, 0, 10 ), QgsPoint( 0, 5, 10 ), QgsPoint( 5, 5, 10 ), QgsPoint( 5, 0, 10 ) );
  ext.fromWkt( "LineString (0 0, 0 5, 5 5, 5 0, 0 0)" );
  QCOMPARE( quad.toLineString()->asWkt(), ext.asWkt() );
  polyg.fromWkt( "Polygon ((0 0, 0 5, 5 5, 5 0, 0 0))" );
  QCOMPARE( quad.toPolygon()->asWkt(), polyg.asWkt() );

  extZ.fromWkt( "LineStringZ (0 0 10, 0 5 10, 5 5 10, 5 0 10, 0 0 10)" );
  QCOMPARE( quadZ.toLineString()->asWkt(), extZ.asWkt() );
  QCOMPARE( quadZ.toLineString( true )->asWkt(), ext.asWkt() );
  polygZ.fromWkt( "PolygonZ ((0 0 10, 0 5 10, 5 5 10, 5 0 10, 0 0 10))" );
  QCOMPARE( quadZ.toPolygon()->asWkt(), polygZ.asWkt() );
  QCOMPARE( quadZ.toPolygon( true )->asWkt(), polyg.asWkt() );


// area / perimeter

  QCOMPARE( QgsQuadrilateral().area(), 0.0 );
  QCOMPARE( QgsQuadrilateral().perimeter(), 0.0 );

  QVERIFY( qgsDoubleNear( QgsQuadrilateral::rectangleFrom3Points( QgsPoint( 0, 0, 10 ), QgsPoint( 0, 5 ), QgsPoint( 5, 4 ), QgsQuadrilateral::Projected ).area(), 25.0 ) );
  QVERIFY( qgsDoubleNear( QgsQuadrilateral::rectangleFrom3Points( QgsPoint( 0, 0, 10 ), QgsPoint( 0, 5 ), QgsPoint( 5, 4 ), QgsQuadrilateral::Projected ).perimeter(), 20 ) );
}

void TestQgsGeometry::regularPolygon()
{
  // constructors
  QgsRegularPolygon rp1 = QgsRegularPolygon();
  QCOMPARE( rp1.center(), QgsPoint() );
  QCOMPARE( rp1.firstVertex(), QgsPoint() );
  QCOMPARE( rp1.numberSides(), static_cast< unsigned int >( 0 ) );
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
  QCOMPARE( rp2.numberSides(), static_cast< unsigned int>( 5 ) );
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
  QCOMPARE( rp7.numberSides(), static_cast< unsigned int >( 0 ) );
  rp7.setNumberSides( 5 );
  QVERIFY( rp7.isEmpty() );
  QCOMPARE( rp7.numberSides(), static_cast< unsigned int >( 5 ) );
  rp7.setNumberSides( 2 );
  QVERIFY( rp7.isEmpty() );
  QCOMPARE( rp7.numberSides(), static_cast< unsigned int >( 5 ) );
  rp7.setNumberSides( 3 );
  QVERIFY( rp7.isEmpty() );
  QCOMPARE( rp7.numberSides(), static_cast< unsigned int >( 3 ) );

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

  //points
  rp8 = QgsRegularPolygon(); // empty
  QgsPointSequence points = rp8.points();
  QVERIFY( points.isEmpty() );
  rp8 = QgsRegularPolygon( QgsPoint(), QgsPoint( 0, 5 ), 3, QgsRegularPolygon::InscribedCircle );
  points = rp8.points();
  QCOMPARE( points.count(), 3 );
  QCOMPARE( points.at( 0 ), QgsPoint( 0, 5 ) );
  QGSCOMPARENEAR( points.at( 1 ).x(), 4.33, 0.01 );
  QGSCOMPARENEAR( points.at( 1 ).y(), -2.4999, 0.01 );
  QGSCOMPARENEAR( points.at( 2 ).x(), -4.33, 0.01 );
  QGSCOMPARENEAR( points.at( 2 ).y(), -2.4999, 0.01 );

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
  QVector<QgsTriangle> rp10_tri = rp10.triangulate();
  QCOMPARE( rp10_tri.length(), static_cast< int >( rp10.numberSides() ) );
  QVERIFY( rp10_tri.at( 0 ) == QgsTriangle( QgsPoint( 0, 0 ), QgsPoint( 0, 4 ), rp10.center() ) );
  QVERIFY( rp10_tri.at( 1 ) == QgsTriangle( QgsPoint( 0, 4 ), QgsPoint( 4, 4 ), rp10.center() ) );
  QVERIFY( rp10_tri.at( 2 ) == QgsTriangle( QgsPoint( 4, 4 ), QgsPoint( 4, 0 ), rp10.center() ) );
  QVERIFY( rp10_tri.at( 3 ) == QgsTriangle( QgsPoint( 4, 0 ), QgsPoint( 0, 0 ), rp10.center() ) );

  QVERIFY( QgsRegularPolygon().triangulate().isEmpty() );

  // polygon
  std::unique_ptr< QgsPolygon > toP( QgsRegularPolygon().toPolygon() );
  QVERIFY( toP->isEmpty() );

  QgsPointSequence ptsPol;
  std::unique_ptr< QgsPolygon > pol( new QgsPolygon() );
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

  std::unique_ptr< QgsLineString > l( QgsRegularPolygon( QgsPoint(), QgsPoint( 0, 5 ), 1, QgsRegularPolygon::InscribedCircle ).toLineString() );
  QVERIFY( l->isEmpty() );
  l.reset( rp10.toLineString( ) );
  QCOMPARE( l->numPoints(), 5 );
  QCOMPARE( l->pointN( 0 ), l->pointN( 4 ) );
  QgsPointSequence pts_l;
  l->points( pts_l );
  pts_l.pop_back();
  QCOMPARE( ptsPol, pts_l );

  //test toString
  QCOMPARE( rp1.toString(), QString( "Empty" ) );
  QCOMPARE( rp2.toString(), QString( "RegularPolygon (Center: Point (0 0), First Vertex: Point (0 5), Radius: 5, Azimuth: 0)" ) );

}


void TestQgsGeometry::curvePolygon()
{
  //test constructor
  QgsCurvePolygon p1;
  QVERIFY( p1.isEmpty() );
  QCOMPARE( p1.numInteriorRings(), 0 );
  QCOMPARE( p1.nCoordinates(), 0 );
  QCOMPARE( p1.ringCount(), 0 );
  QCOMPARE( p1.partCount(), 0 );
  QVERIFY( !p1.is3D() );
  QVERIFY( !p1.isMeasure() );
  QCOMPARE( p1.wkbType(), QgsWkbTypes::CurvePolygon );
  QCOMPARE( p1.wktTypeStr(), QString( "CurvePolygon" ) );
  QCOMPARE( p1.geometryType(), QString( "CurvePolygon" ) );
  QCOMPARE( p1.dimension(), 2 );
  QVERIFY( !p1.hasCurvedSegments() );
  QCOMPARE( p1.area(), 0.0 );
  QCOMPARE( p1.perimeter(), 0.0 );
  QVERIFY( !p1.exteriorRing() );
  QVERIFY( !p1.interiorRing( 0 ) );

  //set exterior ring

  //try with no ring
  QgsCircularString *ext = nullptr;
  p1.setExteriorRing( ext );
  QVERIFY( p1.isEmpty() );
  QCOMPARE( p1.numInteriorRings(), 0 );
  QCOMPARE( p1.nCoordinates(), 0 );
  QCOMPARE( p1.ringCount(), 0 );
  QCOMPARE( p1.partCount(), 0 );
  QVERIFY( !p1.exteriorRing() );
  QVERIFY( !p1.interiorRing( 0 ) );
  QCOMPARE( p1.wkbType(), QgsWkbTypes::CurvePolygon );

  // empty exterior ring
  ext = new QgsCircularString();
  p1.setExteriorRing( ext );
  QVERIFY( p1.isEmpty() );
  QCOMPARE( p1.numInteriorRings(), 0 );
  QCOMPARE( p1.nCoordinates(), 0 );
  QCOMPARE( p1.ringCount(), 1 );
  QCOMPARE( p1.partCount(), 1 );
  QVERIFY( p1.exteriorRing() );
  QVERIFY( !p1.interiorRing( 0 ) );
  QCOMPARE( p1.wkbType(), QgsWkbTypes::CurvePolygon );

  //valid exterior ring
  ext = new QgsCircularString();
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
  QCOMPARE( p1.wkbType(), QgsWkbTypes::CurvePolygon );
  QCOMPARE( p1.wktTypeStr(), QString( "CurvePolygon" ) );
  QCOMPARE( p1.geometryType(), QString( "CurvePolygon" ) );
  QCOMPARE( p1.dimension(), 2 );
  QVERIFY( p1.hasCurvedSegments() );
  QGSCOMPARENEAR( p1.area(), 157.08, 0.01 );
  QGSCOMPARENEAR( p1.perimeter(), 44.4288, 0.01 );
  QVERIFY( p1.exteriorRing() );
  QVERIFY( !p1.interiorRing( 0 ) );

  //retrieve exterior ring and check
  QCOMPARE( *( static_cast< const QgsCircularString * >( p1.exteriorRing() ) ), *ext );

  //initial setting of exterior ring should set z/m type
  QgsCurvePolygon p2;
  ext = new QgsCircularString();
  ext->setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZ, 0, 0, 1 )
                  << QgsPoint( QgsWkbTypes::PointZ, 0, 10, 2 ) << QgsPoint( QgsWkbTypes::PointZ, 10, 10, 3 )
                  << QgsPoint( QgsWkbTypes::PointZ, 10, 0, 4 ) << QgsPoint( QgsWkbTypes::PointZ, 0, 0, 1 ) );
  p2.setExteriorRing( ext );
  QVERIFY( p2.is3D() );
  QVERIFY( !p2.isMeasure() );
  QCOMPARE( p2.wkbType(), QgsWkbTypes::CurvePolygonZ );
  QCOMPARE( p2.wktTypeStr(), QString( "CurvePolygonZ" ) );
  QCOMPARE( p2.geometryType(), QString( "CurvePolygon" ) );
  QCOMPARE( *( static_cast< const QgsCircularString * >( p2.exteriorRing() ) ), *ext );
  QgsCurvePolygon p3;
  ext = new QgsCircularString();
  ext->setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointM, 0, 0, 0, 1 )
                  << QgsPoint( QgsWkbTypes::PointM, 0, 10, 0, 2 ) << QgsPoint( QgsWkbTypes::PointM, 10, 10, 0, 3 )
                  << QgsPoint( QgsWkbTypes::PointM, 10, 0, 0, 4 ) << QgsPoint( QgsWkbTypes::PointM, 0, 0, 0, 1 ) );
  p3.setExteriorRing( ext );
  QVERIFY( !p3.is3D() );
  QVERIFY( p3.isMeasure() );
  QCOMPARE( p3.wkbType(), QgsWkbTypes::CurvePolygonM );
  QCOMPARE( p3.wktTypeStr(), QString( "CurvePolygonM" ) );
  QCOMPARE( *( static_cast< const QgsCircularString * >( p3.exteriorRing() ) ), *ext );
  QgsCurvePolygon p4;
  ext = new QgsCircularString();
  ext->setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 0, 0, 2, 1 )
                  << QgsPoint( QgsWkbTypes::PointZM, 0, 10, 3, 2 ) << QgsPoint( QgsWkbTypes::PointZM, 10, 10, 5, 3 )
                  << QgsPoint( QgsWkbTypes::PointZM, 10, 0, 0, 4 ) << QgsPoint( QgsWkbTypes::PointZM, 0, 0, 2, 1 ) );
  p4.setExteriorRing( ext );
  QVERIFY( p4.is3D() );
  QVERIFY( p4.isMeasure() );
  QCOMPARE( p4.wkbType(), QgsWkbTypes::CurvePolygonZM );
  QCOMPARE( p4.wktTypeStr(), QString( "CurvePolygonZM" ) );
  QCOMPARE( *( static_cast< const QgsCircularString * >( p4.exteriorRing() ) ), *ext );

  //addInteriorRing
  QgsCurvePolygon p6;
  ext = new QgsCircularString();
  ext->setPoints( QgsPointSequence() << QgsPoint( 0, 0 ) << QgsPoint( 0, 10 ) << QgsPoint( 10, 10 )
                  << QgsPoint( 10, 0 ) << QgsPoint( 0, 0 ) );
  p6.setExteriorRing( ext );
  //empty ring
  QCOMPARE( p6.numInteriorRings(), 0 );
  QVERIFY( !p6.interiorRing( -1 ) );
  QVERIFY( !p6.interiorRing( 0 ) );
  p6.addInteriorRing( nullptr );
  QCOMPARE( p6.numInteriorRings(), 0 );
  QgsCircularString *ring = new QgsCircularString();
  ring->setPoints( QgsPointSequence() << QgsPoint( 1, 1 ) << QgsPoint( 1, 9 ) << QgsPoint( 9, 9 )
                   << QgsPoint( 9, 1 ) << QgsPoint( 1, 1 ) );
  p6.addInteriorRing( ring );
  QCOMPARE( p6.numInteriorRings(), 1 );
  QCOMPARE( p6.interiorRing( 0 ), ring );
  QVERIFY( !p6.interiorRing( 1 ) );

  QgsCoordinateSequence seq = p6.coordinateSequence();
  QCOMPARE( seq, QgsCoordinateSequence() << ( QgsRingSequence() << ( QgsPointSequence() << QgsPoint( 0, 0 ) << QgsPoint( 0, 10 ) << QgsPoint( 10, 10 )
            << QgsPoint( 10, 0 ) << QgsPoint( 0, 0 ) )
            << ( QgsPointSequence() << QgsPoint( 1, 1 ) << QgsPoint( 1, 9 ) << QgsPoint( 9, 9 )
                 << QgsPoint( 9, 1 ) << QgsPoint( 1, 1 ) ) ) );
  QCOMPARE( p6.nCoordinates(), 10 );

  //try adding an interior ring with z to a 2d polygon, z should be dropped
  ring = new QgsCircularString();
  ring->setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZ, 0.1, 0.1, 1 )
                   << QgsPoint( QgsWkbTypes::PointZ, 0.1, 0.2, 2 ) << QgsPoint( QgsWkbTypes::PointZ, 0.2, 0.2, 3 )
                   << QgsPoint( QgsWkbTypes::PointZ, 0.2, 0.1, 4 ) << QgsPoint( QgsWkbTypes::PointZ, 0.1, 0.1, 1 ) );
  p6.addInteriorRing( ring );
  QCOMPARE( p6.numInteriorRings(), 2 );
  QVERIFY( !p6.is3D() );
  QVERIFY( !p6.isMeasure() );
  QCOMPARE( p6.wkbType(), QgsWkbTypes::CurvePolygon );
  QVERIFY( p6.interiorRing( 1 ) );
  QVERIFY( !p6.interiorRing( 1 )->is3D() );
  QVERIFY( !p6.interiorRing( 1 )->isMeasure() );
  QCOMPARE( p6.interiorRing( 1 )->wkbType(), QgsWkbTypes::CircularString );

  //try adding an interior ring with m to a 2d polygon, m should be dropped
  ring = new QgsCircularString();
  ring->setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointM, 0.1, 0.1, 0, 1 )
                   << QgsPoint( QgsWkbTypes::PointM, 0.1, 0.2, 0, 2 ) << QgsPoint( QgsWkbTypes::PointM, 0.2, 0.2, 0, 3 )
                   << QgsPoint( QgsWkbTypes::PointM, 0.2, 0.1, 0, 4 ) << QgsPoint( QgsWkbTypes::PointM, 0.1, 0.1, 0, 1 ) );
  p6.addInteriorRing( ring );
  QCOMPARE( p6.numInteriorRings(), 3 );
  QVERIFY( !p6.is3D() );
  QVERIFY( !p6.isMeasure() );
  QCOMPARE( p6.wkbType(), QgsWkbTypes::CurvePolygon );
  QVERIFY( p6.interiorRing( 2 ) );
  QVERIFY( !p6.interiorRing( 2 )->is3D() );
  QVERIFY( !p6.interiorRing( 2 )->isMeasure() );
  QCOMPARE( p6.interiorRing( 2 )->wkbType(), QgsWkbTypes::CircularString );


  //addInteriorRing without z/m to PolygonZM
  QgsCurvePolygon p6b;
  ext = new QgsCircularString();
  ext->setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 0, 0, 1 )
                  << QgsPoint( QgsWkbTypes::PointZM, 0, 10, 2 ) << QgsPoint( QgsWkbTypes::PointZM, 10, 10, 3 )
                  << QgsPoint( QgsWkbTypes::PointZM, 10, 0, 4 ) << QgsPoint( QgsWkbTypes::PointZM, 0, 0, 1 ) );
  p6b.setExteriorRing( ext );
  QVERIFY( p6b.is3D() );
  QVERIFY( p6b.isMeasure() );
  QCOMPARE( p6b.wkbType(), QgsWkbTypes::CurvePolygonZM );
  //ring has no z
  ring = new QgsCircularString();
  ring->setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointM, 1, 1, 0, 2 ) << QgsPoint( QgsWkbTypes::PointM, 1, 9 ) << QgsPoint( QgsWkbTypes::PointM, 9, 9 )
                   << QgsPoint( QgsWkbTypes::PointM, 9, 1 ) << QgsPoint( QgsWkbTypes::PointM, 1, 1 ) );
  p6b.addInteriorRing( ring );
  QVERIFY( p6b.interiorRing( 0 ) );
  QVERIFY( p6b.interiorRing( 0 )->is3D() );
  QVERIFY( p6b.interiorRing( 0 )->isMeasure() );
  QCOMPARE( p6b.interiorRing( 0 )->wkbType(), QgsWkbTypes::CircularStringZM );
  QCOMPARE( p6b.interiorRing( 0 )->vertexAt( QgsVertexId( 0, 0, 0 ) ), QgsPoint( QgsWkbTypes::PointZM, 1, 1, 0, 2 ) );
  //ring has no m
  ring = new QgsCircularString();
  ring->setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZ, 0.1, 0.1, 1 )
                   << QgsPoint( QgsWkbTypes::PointZ, 0.1, 0.2, 2 ) << QgsPoint( QgsWkbTypes::PointZ, 0.2, 0.2, 3 )
                   << QgsPoint( QgsWkbTypes::PointZ, 0.2, 0.1, 4 ) << QgsPoint( QgsWkbTypes::PointZ, 0.1, 0.1, 1 ) );
  p6b.addInteriorRing( ring );
  QVERIFY( p6b.interiorRing( 1 ) );
  QVERIFY( p6b.interiorRing( 1 )->is3D() );
  QVERIFY( p6b.interiorRing( 1 )->isMeasure() );
  QCOMPARE( p6b.interiorRing( 1 )->wkbType(), QgsWkbTypes::CircularStringZM );
  QCOMPARE( p6b.interiorRing( 1 )->vertexAt( QgsVertexId( 0, 0, 0 ) ), QgsPoint( QgsWkbTypes::PointZM, 0.1, 0.1, 1, 0 ) );

  //set interior rings
  QgsCurvePolygon p7;
  ext = new QgsCircularString();
  ext->setPoints( QgsPointSequence() << QgsPoint( 0, 0 ) << QgsPoint( 0, 10 ) << QgsPoint( 10, 10 )
                  << QgsPoint( 10, 0 ) << QgsPoint( 0, 0 ) );
  p7.setExteriorRing( ext );
  //add a list of rings with mixed types
  QVector< QgsCurve * > rings;
  rings << new QgsCircularString();
  static_cast< QgsCircularString *>( rings[0] )->setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZ, 0.1, 0.1, 1 )
      << QgsPoint( QgsWkbTypes::PointZ, 0.1, 0.2, 2 ) << QgsPoint( QgsWkbTypes::PointZ, 0.2, 0.2, 3 )
      << QgsPoint( QgsWkbTypes::PointZ, 0.2, 0.1, 4 ) << QgsPoint( QgsWkbTypes::PointZ, 0.1, 0.1, 1 ) );
  rings << new QgsCircularString();
  static_cast< QgsCircularString *>( rings[1] )->setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointM, 0.3, 0.3, 0, 1 )
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
  QCOMPARE( p7.interiorRing( 0 )->wkbType(), QgsWkbTypes::CircularString );
  QCOMPARE( p7.interiorRing( 0 )->vertexAt( QgsVertexId( 0, 0, 0 ) ), QgsPoint( QgsWkbTypes::Point, 0.1, 0.1 ) );
  QVERIFY( p7.interiorRing( 1 ) );
  QVERIFY( !p7.interiorRing( 1 )->is3D() );
  QVERIFY( !p7.interiorRing( 1 )->isMeasure() );
  QCOMPARE( p7.interiorRing( 1 )->wkbType(), QgsWkbTypes::CircularString );
  QCOMPARE( p7.interiorRing( 1 )->vertexAt( QgsVertexId( 0, 0, 0 ) ), QgsPoint( QgsWkbTypes::Point, 0.3, 0.3 ) );
  QVERIFY( p7.interiorRing( 2 ) );
  QVERIFY( !p7.interiorRing( 2 )->is3D() );
  QVERIFY( !p7.interiorRing( 2 )->isMeasure() );
  QCOMPARE( p7.interiorRing( 2 )->wkbType(), QgsWkbTypes::CircularString );

  //set rings with existing
  rings.clear();
  rings << new QgsCircularString();
  static_cast< QgsCircularString *>( rings[0] )->setPoints( QgsPointSequence() << QgsPoint( 0.8, 0.8 )
      << QgsPoint( 0.8, 0.9 ) << QgsPoint( 0.9, 0.9 )
      << QgsPoint( 0.9, 0.8 ) << QgsPoint( 0.8, 0.8 ) );
  p7.setInteriorRings( rings );
  QCOMPARE( p7.numInteriorRings(), 1 );
  QVERIFY( p7.interiorRing( 0 ) );
  QVERIFY( !p7.interiorRing( 0 )->is3D() );
  QVERIFY( !p7.interiorRing( 0 )->isMeasure() );
  QCOMPARE( p7.interiorRing( 0 )->wkbType(), QgsWkbTypes::CircularString );
  QCOMPARE( p7.interiorRing( 0 )->vertexAt( QgsVertexId( 0, 0, 0 ) ), QgsPoint( QgsWkbTypes::Point, 0.8, 0.8 ) );
  rings.clear();
  p7.setInteriorRings( rings );
  QCOMPARE( p7.numInteriorRings(), 0 );

  //change dimensionality of interior rings using setExteriorRing
  QgsCurvePolygon p7a;
  ext = new QgsCircularString();
  ext->setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZ, 0, 0, 1 ) << QgsPoint( QgsWkbTypes::PointZ, 0, 10, 2 )
                  << QgsPoint( QgsWkbTypes::PointZ, 10, 10, 1 ) << QgsPoint( QgsWkbTypes::PointZ, 10, 0, 3 ) << QgsPoint( QgsWkbTypes::PointZ, 0, 0, 1 ) );
  p7a.setExteriorRing( ext );
  rings.clear();
  rings << new QgsCircularString();
  static_cast< QgsCircularString *>( rings[0] )->setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZ, 0.1, 0.1, 1 )
      << QgsPoint( QgsWkbTypes::PointZ, 0.1, 0.2, 2 ) << QgsPoint( QgsWkbTypes::PointZ, 0.2, 0.2, 3 )
      << QgsPoint( QgsWkbTypes::PointZ, 0.2, 0.1, 4 ) << QgsPoint( QgsWkbTypes::PointZ, 0.1, 0.1, 1 ) );
  rings << new QgsCircularString();
  static_cast< QgsCircularString *>( rings[1] )->setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZ, 0.3, 0.3, 1 )
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
  ext = new QgsCircularString();
  ext->setPoints( QgsPointSequence() << QgsPoint( 0, 0 ) << QgsPoint( 0, 10 )
                  << QgsPoint( 10, 10 ) << QgsPoint( 10, 0 ) << QgsPoint( 0, 0 ) );
  p7a.setExteriorRing( ext );
  QVERIFY( !p7a.is3D() );
  QVERIFY( !p7a.interiorRing( 0 )->is3D() ); //rings should also be made 2D
  QVERIFY( !p7a.interiorRing( 1 )->is3D() );
  //reset exterior ring to LineStringM
  ext = new QgsCircularString();
  ext->setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointM, 0, 0 ) << QgsPoint( QgsWkbTypes::PointM, 0, 10 )
                  << QgsPoint( QgsWkbTypes::PointM, 10, 10 ) << QgsPoint( QgsWkbTypes::PointM, 10, 0 ) << QgsPoint( QgsWkbTypes::PointM, 0, 0 ) );
  p7a.setExteriorRing( ext );
  QVERIFY( p7a.isMeasure() );
  QVERIFY( p7a.interiorRing( 0 )->isMeasure() ); //rings should also gain measure
  QVERIFY( p7a.interiorRing( 1 )->isMeasure() );

  //removeInteriorRing
  QgsCurvePolygon p8;
  ext = new QgsCircularString();
  ext->setPoints( QgsPointSequence() << QgsPoint( 0, 0 ) << QgsPoint( 0, 10 ) << QgsPoint( 10, 10 )
                  << QgsPoint( 10, 0 ) << QgsPoint( 0, 0 ) );
  p8.setExteriorRing( ext );
  QVERIFY( !p8.removeInteriorRing( -1 ) );
  QVERIFY( !p8.removeInteriorRing( 0 ) );
  rings.clear();
  rings << new QgsCircularString();
  static_cast< QgsCircularString *>( rings[0] )->setPoints( QgsPointSequence() << QgsPoint( 0.1, 0.1 )
      << QgsPoint( 0.1, 0.2 ) << QgsPoint( 0.2, 0.2 )
      << QgsPoint( 0.2, 0.1 ) << QgsPoint( 0.1, 0.1 ) );
  rings << new QgsCircularString();
  static_cast< QgsCircularString *>( rings[1] )->setPoints( QgsPointSequence() << QgsPoint( 0.3, 0.3 )
      << QgsPoint( 0.3, 0.4 ) << QgsPoint( 0.4, 0.4 )
      << QgsPoint( 0.4, 0.3 ) << QgsPoint( 0.3, 0.3 ) );
  rings << new QgsCircularString();
  static_cast< QgsCircularString *>( rings[2] )->setPoints( QgsPointSequence() << QgsPoint( 0.8, 0.8 )
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
  QgsCurvePolygon p9;
  ext = new QgsCircularString();
  ext->setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZ, 0, 0, 1 )
                  << QgsPoint( QgsWkbTypes::PointZ, 0, 10, 2 ) << QgsPoint( QgsWkbTypes::PointZ, 10, 10, 3 )
                  << QgsPoint( QgsWkbTypes::PointZ, 10, 0, 4 ) << QgsPoint( QgsWkbTypes::PointZ, 0, 0, 1 ) );
  p9.setExteriorRing( ext );
  ring = new QgsCircularString();
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
  QCOMPARE( p9.wkbType(), QgsWkbTypes::CurvePolygon );

  //equality operator
  QgsCurvePolygon p10;
  QgsCurvePolygon p10b;
  QVERIFY( p10 == p10b );
  QVERIFY( !( p10 != p10b ) );
  ext = new QgsCircularString();
  ext->setPoints( QgsPointSequence() << QgsPoint( 0, 0 ) << QgsPoint( 0, 10 ) << QgsPoint( 10, 10 )
                  << QgsPoint( 10, 0 ) << QgsPoint( 0, 0 ) );
  p10.setExteriorRing( ext );
  QVERIFY( !( p10 == p10b ) );
  QVERIFY( p10 != p10b );
  ext = new QgsCircularString();
  ext->setPoints( QgsPointSequence() << QgsPoint( 0, 0 ) << QgsPoint( 0, 10 ) << QgsPoint( 10, 10 )
                  << QgsPoint( 10, 0 ) << QgsPoint( 0, 0 ) );
  p10b.setExteriorRing( ext );
  QVERIFY( p10 == p10b );
  QVERIFY( !( p10 != p10b ) );
  ext = new QgsCircularString();
  ext->setPoints( QgsPointSequence() << QgsPoint( 0, 0 ) << QgsPoint( 0, 9 ) << QgsPoint( 9, 9 )
                  << QgsPoint( 9, 0 ) << QgsPoint( 0, 0 ) );
  p10b.setExteriorRing( ext );
  QVERIFY( !( p10 == p10b ) );
  QVERIFY( p10 != p10b );
  ext = new QgsCircularString();
  ext->setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZ, 0, 0, 1 ) << QgsPoint( QgsWkbTypes::PointZ, 0, 10, 2 )
                  << QgsPoint( QgsWkbTypes::PointZ, 10, 10, 3 ) << QgsPoint( QgsWkbTypes::PointZ, 10, 0, 4 ) << QgsPoint( QgsWkbTypes::PointZ, 0, 0, 1 ) );
  p10b.setExteriorRing( ext );
  QVERIFY( !( p10 == p10b ) );
  QVERIFY( p10 != p10b );
  p10b.setExteriorRing( p10.exteriorRing()->clone() );
  QVERIFY( p10 == p10b );
  QVERIFY( !( p10 != p10b ) );
  ring = new QgsCircularString();
  ring->setPoints( QgsPointSequence() << QgsPoint( 1, 1 )
                   << QgsPoint( 1, 9 ) << QgsPoint( 9, 9 )
                   << QgsPoint( 9, 1 ) << QgsPoint( 1, 1 ) );
  p10.addInteriorRing( ring );
  QVERIFY( !( p10 == p10b ) );
  QVERIFY( p10 != p10b );

  ring = new QgsCircularString();
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

  QgsCurvePolygon p11;
  std::unique_ptr< QgsCurvePolygon >cloned( p11.clone() );
  QCOMPARE( p11, *cloned );
  ext = new QgsCircularString();
  ext->setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 0, 0, 1, 5 )
                  << QgsPoint( QgsWkbTypes::PointZM, 0, 10, 2, 6 ) << QgsPoint( QgsWkbTypes::PointZM, 10, 10, 3, 7 )
                  << QgsPoint( QgsWkbTypes::PointZM, 10, 0, 4, 8 ) << QgsPoint( QgsWkbTypes::PointZM, 0, 0, 1, 9 ) );
  p11.setExteriorRing( ext );
  ring = new QgsCircularString();
  ring->setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 1, 1, 1, 2 )
                   << QgsPoint( QgsWkbTypes::PointZM, 1, 9, 2, 3 ) << QgsPoint( QgsWkbTypes::PointZM, 9, 9, 3, 6 )
                   << QgsPoint( QgsWkbTypes::PointZM, 9, 1, 4, 4 ) << QgsPoint( QgsWkbTypes::PointZM, 1, 1, 1, 7 ) );
  p11.addInteriorRing( ring );
  cloned.reset( p11.clone() );
  QCOMPARE( p11, *cloned );

  //copy constructor
  QgsCurvePolygon p12;
  QgsCurvePolygon p13( p12 );
  QCOMPARE( p12, p13 );
  ext = new QgsCircularString();
  ext->setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 0, 0, 1, 5 )
                  << QgsPoint( QgsWkbTypes::PointZM, 0, 10, 2, 6 ) << QgsPoint( QgsWkbTypes::PointZM, 10, 10, 3, 7 )
                  << QgsPoint( QgsWkbTypes::PointZM, 10, 0, 4, 8 ) << QgsPoint( QgsWkbTypes::PointZM, 0, 0, 1, 9 ) );
  p12.setExteriorRing( ext );
  ring = new QgsCircularString();
  ring->setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 1, 1, 1, 2 )
                   << QgsPoint( QgsWkbTypes::PointZM, 1, 9, 2, 3 ) << QgsPoint( QgsWkbTypes::PointZM, 9, 9, 3, 6 )
                   << QgsPoint( QgsWkbTypes::PointZM, 9, 1, 4, 4 ) << QgsPoint( QgsWkbTypes::PointZM, 1, 1, 1, 7 ) );
  p12.addInteriorRing( ring );
  QgsCurvePolygon p14( p12 );
  QCOMPARE( p12, p14 );

  //assignment operator
  QgsCurvePolygon p15;
  p15 = p13;
  QCOMPARE( p13, p15 );
  p15 = p12;
  QCOMPARE( p12, p15 );

  // bounding box
  QgsCurvePolygon boundingBoxPoly;
  QgsRectangle bBox = boundingBoxPoly.boundingBox(); //no crash!

  ext = new QgsCircularString();
  ext->setPoints( QgsPointSequence() << QgsPoint( 0, 0, 1 ) << QgsPoint( 1, 10, 2 ) << QgsPoint( 0, 18, 3 )
                  << QgsPoint( -1, 4, 4 ) << QgsPoint( 0, 0, 1 ) );
  boundingBoxPoly.setExteriorRing( ext );
  bBox = boundingBoxPoly.boundingBox();
  QGSCOMPARENEAR( bBox.xMinimum(), -1.435273, 0.001 );
  QGSCOMPARENEAR( bBox.xMaximum(), 1.012344, 0.001 );
  QGSCOMPARENEAR( bBox.yMinimum(), 0.000000, 0.001 );
  QGSCOMPARENEAR( bBox.yMaximum(), 18, 0.001 );

  //surfaceToPolygon
  QgsCurvePolygon p12a;
  std::unique_ptr< QgsPolygon > surface( p12a.surfaceToPolygon() );
  QVERIFY( surface->isEmpty() );

  ext = new QgsCircularString();
  ext->setPoints( QgsPointSequence() << QgsPoint( 0, 0 ) << QgsPoint( 1, 3 ) << QgsPoint( 2, 4 )
                  << QgsPoint( -1, 5 ) << QgsPoint( 0, 6 ) );
  p12a.setExteriorRing( ext );
  surface.reset( p12a.surfaceToPolygon() );
  QCOMPARE( surface->wkbType(), QgsWkbTypes::Polygon );
  QCOMPARE( surface->exteriorRing()->nCoordinates(), 290 );
  QCOMPARE( surface->exteriorRing()->nCoordinates(), 290 ); // nCoordinates is cached, so check twice
  QVERIFY( surface->exteriorRing()->isClosed() );
  // too many vertices to actually check the result, let's just make sure the bounding boxes are similar
  QgsRectangle r1 = ext->boundingBox();
  QgsRectangle r2 = surface->exteriorRing()->boundingBox();
  QGSCOMPARENEAR( r1.xMinimum(), r2.xMinimum(), 0.0001 );
  QGSCOMPARENEAR( r1.xMaximum(), r2.xMaximum(), 0.0001 );
  QGSCOMPARENEAR( r1.yMinimum(), r2.yMinimum(), 0.0001 );
  QGSCOMPARENEAR( r1.yMaximum(), r2.yMaximum(), 0.0001 );
  ring = new QgsCircularString();
  ring->setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 1, 1, 1, 2 )
                   << QgsPoint( QgsWkbTypes::PointZM, 1, 9, 2, 3 ) << QgsPoint( QgsWkbTypes::PointZM, 9, 9, 3, 6 )
                   << QgsPoint( QgsWkbTypes::PointZM, 9, 1, 4, 4 ) << QgsPoint( QgsWkbTypes::PointZM, 1, 1, 1, 7 ) );
  p12a.addInteriorRing( ring );
  surface.reset( p12a.surfaceToPolygon() );
  QCOMPARE( surface->wkbType(), QgsWkbTypes::Polygon );
  QCOMPARE( surface->exteriorRing()->nCoordinates(), 290 );
  QCOMPARE( surface->exteriorRing()->nCoordinates(), 290 ); // nCoordinates is cached, so check twice
  QVERIFY( surface->exteriorRing()->isClosed() );
  QCOMPARE( surface->numInteriorRings(), 1 );
  // too many vertices to actually check the result, let's just make sure the bounding boxes are similar
  r1 = ring->boundingBox();
  r2 = surface->interiorRing( 0 )->boundingBox();
  QGSCOMPARENEAR( r1.xMinimum(), r2.xMinimum(), 0.0001 );
  QGSCOMPARENEAR( r1.xMaximum(), r2.xMaximum(), 0.0001 );
  QGSCOMPARENEAR( r1.yMinimum(), r2.yMinimum(), 0.0001 );
  QGSCOMPARENEAR( r1.yMaximum(), r2.yMaximum(), 0.0001 );

  //toPolygon
  p12a = QgsCurvePolygon();
  surface.reset( p12a.toPolygon() );
  QVERIFY( surface->isEmpty() );

  ext = new QgsCircularString();
  ext->setPoints( QgsPointSequence() << QgsPoint( 0, 0 ) << QgsPoint( 1, 10 ) << QgsPoint( 0, 18 )
                  << QgsPoint( -1, 4 ) << QgsPoint( 0, 0 ) );
  p12a.setExteriorRing( ext );
  surface.reset( p12a.toPolygon() );
  QCOMPARE( surface->wkbType(), QgsWkbTypes::Polygon );
  QCOMPARE( surface->exteriorRing()->nCoordinates(), 64 );
  QCOMPARE( surface->exteriorRing()->nCoordinates(), 64 ); // ncoordinates is cached, so check twice
  QVERIFY( surface->exteriorRing()->isClosed() );
  // too many vertices to actually check the result, let's just make sure the bounding boxes are similar
  r1 = ext->boundingBox();
  r2 = surface->exteriorRing()->boundingBox();
  QGSCOMPARENEAR( r1.xMinimum(), r2.xMinimum(), 0.01 );
  QGSCOMPARENEAR( r1.xMaximum(), r2.xMaximum(), 0.01 );
  QGSCOMPARENEAR( r1.yMinimum(), r2.yMinimum(), 0.01 );
  QGSCOMPARENEAR( r1.yMaximum(), r2.yMaximum(), 0.01 );
  ring = new QgsCircularString();
  ring->setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 1, 1, 1, 2 )
                   << QgsPoint( QgsWkbTypes::PointZM, 1, 9, 2, 3 ) << QgsPoint( QgsWkbTypes::PointZM, 9, 9, 3, 6 )
                   << QgsPoint( QgsWkbTypes::PointZM, 9, 1, 4, 4 ) << QgsPoint( QgsWkbTypes::PointZM, 1, 1, 1, 7 ) );
  p12a.addInteriorRing( ring );
  surface.reset( p12a.toPolygon() );
  QCOMPARE( surface->wkbType(), QgsWkbTypes::Polygon );
  QCOMPARE( surface->exteriorRing()->nCoordinates(), 64 );
  QCOMPARE( surface->exteriorRing()->nCoordinates(), 64 ); //ncoordinates is cached, so check twice
  QVERIFY( surface->exteriorRing()->isClosed() );
  QCOMPARE( surface->numInteriorRings(), 1 );
  // too many vertices to actually check the result, let's just make sure the bounding boxes are similar
  r1 = ring->boundingBox();
  r2 = surface->interiorRing( 0 )->boundingBox();
  QGSCOMPARENEAR( r1.xMinimum(), r2.xMinimum(), 0.0001 );
  QGSCOMPARENEAR( r1.xMaximum(), r2.xMaximum(), 0.0001 );
  QGSCOMPARENEAR( r1.yMinimum(), r2.yMinimum(), 0.0001 );
  QGSCOMPARENEAR( r1.yMaximum(), r2.yMaximum(), 0.0001 );

  //toCurveType - should be identical since it's already a curve
  std::unique_ptr< QgsCurvePolygon > curveType( p12a.toCurveType() );
  QCOMPARE( *curveType, p12a );

  //to/fromWKB
  QgsCurvePolygon p16;
  ext = new QgsCircularString();
  ext->setPoints( QgsPointSequence() << QgsPoint( 0, 0 ) << QgsPoint( 1, 0 ) << QgsPoint( 2, 0 )
                  << QgsPoint( 1, 0.5 ) << QgsPoint( 0, 0 ) );
  p16.setExteriorRing( ext );
  ring = new QgsCircularString();
  ring->setPoints( QgsPointSequence() << QgsPoint( 0, 0 ) << QgsPoint( 0.1, 0 ) << QgsPoint( 0.2, 0 )
                   << QgsPoint( 0.1, 0.05 ) << QgsPoint( 0, 0 ) );
  p16.addInteriorRing( ring );
  QByteArray wkb16 = p16.asWkb();
  QgsCurvePolygon p17;
  QgsConstWkbPtr wkb16ptr( wkb16 );
  p17.fromWkb( wkb16ptr );
  QCOMPARE( p16, p17 );
  //CurvePolygonZ
  p16.clear();
  p17.clear();

  ext = new QgsCircularString();
  ext->setPoints( QgsPointSequence() << QgsPoint( 0, 0, 1 ) << QgsPoint( 1, 0, 2 ) << QgsPoint( 2, 0, 3 )
                  << QgsPoint( 1, 0.5, 4 ) << QgsPoint( 0, 0, 1 ) );
  p16.setExteriorRing( ext );
  ring = new QgsCircularString();
  ring->setPoints( QgsPointSequence() << QgsPoint( 0, 0, 1 ) << QgsPoint( 0.1, 0, 2 ) << QgsPoint( 0.2, 0, 3 )
                   << QgsPoint( 0.1, 0.05, 4 ) << QgsPoint( 0, 0, 1 ) );
  p16.addInteriorRing( ring );
  wkb16 = p16.asWkb();
  QgsConstWkbPtr wkb16ptr2( wkb16 );
  p17.fromWkb( wkb16ptr2 );
  QCOMPARE( p16, p17 );

  // compound curve
  QgsCompoundCurve *cCurve = new QgsCompoundCurve();
  ext = new QgsCircularString();
  ext->setPoints( QgsPointSequence() << QgsPoint( 0, 0, 1 ) << QgsPoint( 1, 0, 2 ) << QgsPoint( 2, 0, 3 )
                  << QgsPoint( 1, 0.5, 4 ) << QgsPoint( 0, 0, 1 ) );
  cCurve->addCurve( ext );
  p16.addInteriorRing( cCurve );
  wkb16 = p16.asWkb();
  QgsConstWkbPtr wkb16ptr3( wkb16 );
  p17.fromWkb( wkb16ptr3 );
  QCOMPARE( p16, p17 );

  //CurvePolygonM
  p16.clear();
  p17.clear();
  ext = new QgsCircularString();
  ext->setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointM, 0, 0, 0, 1 ) << QgsPoint( QgsWkbTypes::PointM, 1, 0, 0, 2 ) << QgsPoint( QgsWkbTypes::PointM, 2, 0, 0, 3 )
                  << QgsPoint( QgsWkbTypes::PointM, 1, 0.5, 0, 4 ) << QgsPoint( QgsWkbTypes::PointM, 0, 0, 0, 1 ) );
  p16.setExteriorRing( ext );
  ring = new QgsCircularString();
  ring->setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointM, 0, 0, 0, 1 ) << QgsPoint( QgsWkbTypes::PointM, 0.1, 0, 0, 2 ) << QgsPoint( QgsWkbTypes::PointM, 0.2, 0, 0, 3 )
                   << QgsPoint( QgsWkbTypes::PointM, 0.1, 0.05, 0, 4 ) << QgsPoint( QgsWkbTypes::PointM, 0, 0, 0, 1 ) );
  p16.addInteriorRing( ring );
  wkb16 = p16.asWkb();
  QgsConstWkbPtr wkb16ptr8( wkb16 );
  p17.fromWkb( wkb16ptr8 );
  QCOMPARE( p16, p17 );

  //CurvePolygonZM
  p16.clear();
  p17.clear();
  ext = new QgsCircularString();
  ext->setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 0, 0, 10, 1 ) << QgsPoint( QgsWkbTypes::PointZM, 1, 0, 11, 2 ) << QgsPoint( QgsWkbTypes::PointZM, 2, 0, 12, 3 )
                  << QgsPoint( QgsWkbTypes::PointZM, 1, 0.5, 13, 4 ) << QgsPoint( QgsWkbTypes::PointZM, 0, 0, 10, 1 ) );
  p16.setExteriorRing( ext );
  ring = new QgsCircularString();
  ring->setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 0, 0, 10, 1 ) << QgsPoint( QgsWkbTypes::PointZM, 0.1, 0, 11, 2 ) << QgsPoint( QgsWkbTypes::PointZM, 0.2, 0, 12, 3 )
                   << QgsPoint( QgsWkbTypes::PointZM, 0.1, 0.05, 13, 4 ) << QgsPoint( QgsWkbTypes::PointZM, 0, 0, 10, 1 ) );
  p16.addInteriorRing( ring );
  wkb16 = p16.asWkb();
  QgsConstWkbPtr wkb16ptr4( wkb16 );
  p17.fromWkb( wkb16ptr4 );
  QCOMPARE( p16, p17 );

  //bad WKB - check for no crash
  p17.clear();
  QgsConstWkbPtr nullPtr( nullptr, 0 );
  QVERIFY( !p17.fromWkb( nullPtr ) );
  QCOMPARE( p17.wkbType(), QgsWkbTypes::CurvePolygon );
  QgsPoint point( 1, 2 );
  QByteArray wkbPoint = point.asWkb();
  QgsConstWkbPtr wkbPointPtr( wkbPoint );
  QVERIFY( !p17.fromWkb( wkbPointPtr ) );
  QCOMPARE( p17.wkbType(), QgsWkbTypes::CurvePolygon );

  //to/from WKT
  QgsCurvePolygon p18;
  ext = new QgsCircularString();
  ext->setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 0, 0, 10, 1 ) << QgsPoint( QgsWkbTypes::PointZM, 1, 0, 11, 2 ) << QgsPoint( QgsWkbTypes::PointZM, 2, 0, 12, 3 )
                  << QgsPoint( QgsWkbTypes::PointZM, 1, 0.5, 13, 4 ) << QgsPoint( QgsWkbTypes::PointZM, 0, 0, 10, 1 ) );
  p18.setExteriorRing( ext );
  ring = new QgsCircularString();
  ring->setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 0, 0, 10, 1 ) << QgsPoint( QgsWkbTypes::PointZM, 0.1, 0, 11, 2 ) << QgsPoint( QgsWkbTypes::PointZM, 0.2, 0, 12, 3 )
                   << QgsPoint( QgsWkbTypes::PointZM, 0.1, 0.05, 13, 4 ) << QgsPoint( QgsWkbTypes::PointZM, 0, 0, 10, 1 ) );
  p18.addInteriorRing( ring );

  QString wkt = p18.asWkt();
  QVERIFY( !wkt.isEmpty() );
  QgsCurvePolygon p19;
  QVERIFY( p19.fromWkt( wkt ) );
  QCOMPARE( p18, p19 );

  //bad WKT
  QVERIFY( !p19.fromWkt( "Point()" ) );
  QVERIFY( p19.isEmpty() );
  QVERIFY( !p19.exteriorRing() );
  QCOMPARE( p19.numInteriorRings(), 0 );
  QVERIFY( !p19.is3D() );
  QVERIFY( !p19.isMeasure() );
  QCOMPARE( p19.wkbType(), QgsWkbTypes::CurvePolygon );


  //as JSON
  QgsCurvePolygon exportPolygon;
  ext = new QgsCircularString();
  ext->setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 0, 0, 10, 1 ) << QgsPoint( QgsWkbTypes::PointZM, 1, 0, 11, 2 ) << QgsPoint( QgsWkbTypes::PointZM, 2, 0, 12, 3 )
                  << QgsPoint( QgsWkbTypes::PointZM, 1, 0.5, 13, 4 ) << QgsPoint( QgsWkbTypes::PointZM, 0, 0, 10, 1 ) );
  exportPolygon.setExteriorRing( ext );


  // GML document for compare
  QDomDocument doc( QStringLiteral( "gml" ) );

  // as GML2
  QString expectedSimpleGML2( QStringLiteral( "<Polygon xmlns=\"gml\"><outerBoundaryIs xmlns=\"gml\"><LinearRing xmlns=\"gml\"><coordinates xmlns=\"gml\" cs=\",\" ts=\" \">0,0 1,0 2,0 2,0 2,0 2,0.1 1.9,0.1 1.9,0.1 1.9,0.1 1.9,0.1 1.9,0.1 1.9,0.1 1.9,0.2 1.8,0.2 1.8,0.2 1.8,0.2 1.8,0.2 1.8,0.2 1.8,0.2 1.7,0.3 1.7,0.3 1.7,0.3 1.7,0.3 1.7,0.3 1.6,0.3 1.6,0.3 1.6,0.3 1.6,0.4 1.6,0.4 1.6,0.4 1.5,0.4 1.5,0.4 1.5,0.4 1.5,0.4 1.5,0.4 1.4,0.4 1.4,0.4 1.4,0.4 1.4,0.4 1.4,0.4 1.3,0.5 1.3,0.5 1.3,0.5 1.3,0.5 1.2,0.5 1.2,0.5 1.2,0.5 1.2,0.5 1.2,0.5 1.1,0.5 1.1,0.5 1.1,0.5 1.1,0.5 1.1,0.5 1,0.5 1,0.5 1,0.5 1,0.5 0.9,0.5 0.9,0.5 0.9,0.5 0.9,0.5 0.9,0.5 0.8,0.5 0.8,0.5 0.8,0.5 0.8,0.5 0.8,0.5 0.7,0.5 0.7,0.5 0.7,0.5 0.7,0.5 0.6,0.4 0.6,0.4 0.6,0.4 0.6,0.4 0.6,0.4 0.5,0.4 0.5,0.4 0.5,0.4 0.5,0.4 0.5,0.4 0.4,0.4 0.4,0.4 0.4,0.4 0.4,0.3 0.4,0.3 0.4,0.3 0.3,0.3 0.3,0.3 0.3,0.3 0.3,0.3 0.3,0.3 0.2,0.2 0.2,0.2 0.2,0.2 0.2,0.2 0.2,0.2 0.2,0.2 0.1,0.2 0.1,0.1 0.1,0.1 0.1,0.1 0.1,0.1 0.1,0.1 0.1,0.1 0,0.1 0,0 0,0 0,0</coordinates></LinearRing></outerBoundaryIs></Polygon>" ) );
  QString res = elemToString( exportPolygon.asGml2( doc, 1 ) );
  QGSCOMPAREGML( res, expectedSimpleGML2 );
  QString expectedGML2empty( QStringLiteral( "<Polygon xmlns=\"gml\"/>" ) );
  QGSCOMPAREGML( elemToString( QgsCurvePolygon().asGml2( doc ) ), expectedGML2empty );

  //as GML3
  QString expectedSimpleGML3( QStringLiteral( "<Polygon xmlns=\"gml\"><exterior xmlns=\"gml\"><Curve xmlns=\"gml\"><segments xmlns=\"gml\"><ArcString xmlns=\"gml\"><posList xmlns=\"gml\" srsDimension=\"3\">0 0 10 1 0 11 2 0 12 1 0.5 13 0 0 10</posList></ArcString></segments></Curve></exterior></Polygon>" ) );
  res = elemToString( exportPolygon.asGml3( doc, 2 ) );
  QCOMPARE( elemToString( exportPolygon.asGml3( doc ) ), expectedSimpleGML3 );
  QString expectedGML3empty( QStringLiteral( "<Polygon xmlns=\"gml\"/>" ) );
  QGSCOMPAREGML( elemToString( QgsCurvePolygon().asGml3( doc ) ), expectedGML3empty );

  // as JSON
  QString expectedSimpleJson( QStringLiteral( "{\"type\": \"Polygon\", \"coordinates\": [[ [0, 0], [1, 0], [2, 0], [2, 0], [2, 0], [2, 0.1], [1.9, 0.1], [1.9, 0.1], [1.9, 0.1], [1.9, 0.1], [1.9, 0.1], [1.9, 0.1], [1.9, 0.2], [1.8, 0.2], [1.8, 0.2], [1.8, 0.2], [1.8, 0.2], [1.8, 0.2], [1.8, 0.2], [1.7, 0.3], [1.7, 0.3], [1.7, 0.3], [1.7, 0.3], [1.7, 0.3], [1.6, 0.3], [1.6, 0.3], [1.6, 0.3], [1.6, 0.4], [1.6, 0.4], [1.6, 0.4], [1.5, 0.4], [1.5, 0.4], [1.5, 0.4], [1.5, 0.4], [1.5, 0.4], [1.4, 0.4], [1.4, 0.4], [1.4, 0.4], [1.4, 0.4], [1.4, 0.4], [1.3, 0.5], [1.3, 0.5], [1.3, 0.5], [1.3, 0.5], [1.2, 0.5], [1.2, 0.5], [1.2, 0.5], [1.2, 0.5], [1.2, 0.5], [1.1, 0.5], [1.1, 0.5], [1.1, 0.5], [1.1, 0.5], [1.1, 0.5], [1, 0.5], [1, 0.5], [1, 0.5], [1, 0.5], [0.9, 0.5], [0.9, 0.5], [0.9, 0.5], [0.9, 0.5], [0.9, 0.5], [0.8, 0.5], [0.8, 0.5], [0.8, 0.5], [0.8, 0.5], [0.8, 0.5], [0.7, 0.5], [0.7, 0.5], [0.7, 0.5], [0.7, 0.5], [0.6, 0.4], [0.6, 0.4], [0.6, 0.4], [0.6, 0.4], [0.6, 0.4], [0.5, 0.4], [0.5, 0.4], [0.5, 0.4], [0.5, 0.4], [0.5, 0.4], [0.4, 0.4], [0.4, 0.4], [0.4, 0.4], [0.4, 0.3], [0.4, 0.3], [0.4, 0.3], [0.3, 0.3], [0.3, 0.3], [0.3, 0.3], [0.3, 0.3], [0.3, 0.3], [0.2, 0.2], [0.2, 0.2], [0.2, 0.2], [0.2, 0.2], [0.2, 0.2], [0.2, 0.2], [0.1, 0.2], [0.1, 0.1], [0.1, 0.1], [0.1, 0.1], [0.1, 0.1], [0.1, 0.1], [0.1, 0.1], [0, 0.1], [0, 0], [0, 0], [0, 0]]] }" ) );
  res = exportPolygon.asJson( 1 );
  QCOMPARE( res, expectedSimpleJson );

  ring = new QgsCircularString();
  ring->setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 0, 0, 10, 1 ) << QgsPoint( QgsWkbTypes::PointZM, 0.1, 0, 11, 2 ) << QgsPoint( QgsWkbTypes::PointZM, 0.2, 0, 12, 3 )
                   << QgsPoint( QgsWkbTypes::PointZM, 0.1, 0.05, 13, 4 ) << QgsPoint( QgsWkbTypes::PointZM, 0, 0, 10, 1 ) );
  exportPolygon.addInteriorRing( ring );

  QString expectedJson( QStringLiteral( "{\"type\": \"Polygon\", \"coordinates\": [[ [0, 0], [1, 0], [2, 0], [2, 0], [2, 0], [2, 0.1], [1.9, 0.1], [1.9, 0.1], [1.9, 0.1], [1.9, 0.1], [1.9, 0.1], [1.9, 0.1], [1.9, 0.2], [1.8, 0.2], [1.8, 0.2], [1.8, 0.2], [1.8, 0.2], [1.8, 0.2], [1.8, 0.2], [1.7, 0.3], [1.7, 0.3], [1.7, 0.3], [1.7, 0.3], [1.7, 0.3], [1.6, 0.3], [1.6, 0.3], [1.6, 0.3], [1.6, 0.4], [1.6, 0.4], [1.6, 0.4], [1.5, 0.4], [1.5, 0.4], [1.5, 0.4], [1.5, 0.4], [1.5, 0.4], [1.4, 0.4], [1.4, 0.4], [1.4, 0.4], [1.4, 0.4], [1.4, 0.4], [1.3, 0.5], [1.3, 0.5], [1.3, 0.5], [1.3, 0.5], [1.2, 0.5], [1.2, 0.5], [1.2, 0.5], [1.2, 0.5], [1.2, 0.5], [1.1, 0.5], [1.1, 0.5], [1.1, 0.5], [1.1, 0.5], [1.1, 0.5], [1, 0.5], [1, 0.5], [1, 0.5], [1, 0.5], [0.9, 0.5], [0.9, 0.5], [0.9, 0.5], [0.9, 0.5], [0.9, 0.5], [0.8, 0.5], [0.8, 0.5], [0.8, 0.5], [0.8, 0.5], [0.8, 0.5], [0.7, 0.5], [0.7, 0.5], [0.7, 0.5], [0.7, 0.5], [0.6, 0.4], [0.6, 0.4], [0.6, 0.4], [0.6, 0.4], [0.6, 0.4], [0.5, 0.4], [0.5, 0.4], [0.5, 0.4], [0.5, 0.4], [0.5, 0.4], [0.4, 0.4], [0.4, 0.4], [0.4, 0.4], [0.4, 0.3], [0.4, 0.3], [0.4, 0.3], [0.3, 0.3], [0.3, 0.3], [0.3, 0.3], [0.3, 0.3], [0.3, 0.3], [0.2, 0.2], [0.2, 0.2], [0.2, 0.2], [0.2, 0.2], [0.2, 0.2], [0.2, 0.2], [0.1, 0.2], [0.1, 0.1], [0.1, 0.1], [0.1, 0.1], [0.1, 0.1], [0.1, 0.1], [0.1, 0.1], [0, 0.1], [0, 0], [0, 0], [0, 0]], [ [0, 0], [0.1, 0], [0.2, 0], [0.2, 0], [0.2, 0], [0.2, 0], [0.2, 0], [0.2, 0], [0.2, 0], [0.2, 0], [0.2, 0], [0.2, 0], [0.2, 0], [0.2, 0], [0.2, 0], [0.2, 0], [0.2, 0], [0.2, 0], [0.2, 0], [0.2, 0], [0.2, 0], [0.2, 0], [0.2, 0], [0.2, 0], [0.2, 0], [0.2, 0], [0.2, 0], [0.2, 0], [0.2, 0], [0.2, 0], [0.2, 0], [0.2, 0], [0.1, 0], [0.1, 0], [0.1, 0], [0.1, 0], [0.1, 0], [0.1, 0], [0.1, 0], [0.1, 0], [0.1, 0], [0.1, 0], [0.1, 0], [0.1, 0], [0.1, 0], [0.1, 0], [0.1, 0], [0.1, 0], [0.1, 0], [0.1, 0], [0.1, 0], [0.1, 0], [0.1, 0], [0.1, 0], [0.1, 0], [0.1, 0], [0.1, 0], [0.1, 0], [0.1, 0], [0.1, 0], [0.1, 0], [0.1, 0], [0.1, 0], [0.1, 0], [0.1, 0], [0.1, 0], [0.1, 0], [0.1, 0], [0.1, 0], [0.1, 0], [0.1, 0], [0.1, 0], [0.1, 0], [0.1, 0], [0.1, 0], [0.1, 0], [0.1, 0], [0.1, 0], [0.1, 0], [0.1, 0], [0, 0], [0, 0], [0, 0], [0, 0], [0, 0], [0, 0], [0, 0], [0, 0], [0, 0], [0, 0], [0, 0], [0, 0], [0, 0], [0, 0], [0, 0], [0, 0], [0, 0], [0, 0], [0, 0], [0, 0], [0, 0], [0, 0], [0, 0], [0, 0], [0, 0], [0, 0], [0, 0], [0, 0], [0, 0], [0, 0]]] }" ) );
  res = exportPolygon.asJson( 1 );
  QCOMPARE( res, expectedJson );


  QgsCurvePolygon exportPolygonFloat;
  ext = new QgsCircularString();
  ext->setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 0, 0, 10, 1 ) << QgsPoint( QgsWkbTypes::PointZM, 1 / 3.0, 0, 11, 2 ) << QgsPoint( QgsWkbTypes::PointZM, 2 / 3.0, 0, 12, 3 )
                  << QgsPoint( QgsWkbTypes::PointZM, 1 / 3.0, 0.5, 13, 4 ) << QgsPoint( QgsWkbTypes::PointZM, 0, 0, 10, 1 ) );
  exportPolygonFloat.setExteriorRing( ext );
  ring = new QgsCircularString();
  ring->setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 0, 0, 10, 1 ) << QgsPoint( QgsWkbTypes::PointZM, 0.1 / 3.0, 0, 11, 2 ) << QgsPoint( QgsWkbTypes::PointZM, 0.2 / 3.0, 0, 12, 3 )
                   << QgsPoint( QgsWkbTypes::PointZM, 0.1 / 3.0, 0.05 / 3.0, 13, 4 ) << QgsPoint( QgsWkbTypes::PointZM, 0, 0, 10, 1 ) );
  exportPolygonFloat.addInteriorRing( ring );

  QString expectedJsonPrec3( QStringLiteral( "{\"type\": \"Polygon\", \"coordinates\": [[ [0, 0], [0.333, 0], [0.667, 0], [0.669, 0.006], [0.671, 0.012], [0.673, 0.018], [0.676, 0.024], [0.677, 0.029], [0.679, 0.035], [0.681, 0.042], [0.683, 0.048], [0.684, 0.054], [0.686, 0.06], [0.687, 0.066], [0.688, 0.072], [0.689, 0.078], [0.69, 0.084], [0.691, 0.091], [0.692, 0.097], [0.693, 0.103], [0.693, 0.109], [0.694, 0.116], [0.694, 0.122], [0.694, 0.128], [0.694, 0.135], [0.694, 0.141], [0.694, 0.147], [0.694, 0.153], [0.694, 0.16], [0.693, 0.166], [0.693, 0.172], [0.692, 0.178], [0.692, 0.185], [0.691, 0.191], [0.69, 0.197], [0.689, 0.203], [0.687, 0.209], [0.686, 0.216], [0.685, 0.222], [0.683, 0.228], [0.682, 0.234], [0.68, 0.24], [0.678, 0.246], [0.676, 0.252], [0.674, 0.258], [0.672, 0.264], [0.67, 0.27], [0.668, 0.275], [0.665, 0.281], [0.663, 0.287], [0.66, 0.293], [0.657, 0.298], [0.654, 0.304], [0.652, 0.31], [0.649, 0.315], [0.645, 0.321], [0.642, 0.326], [0.639, 0.331], [0.636, 0.337], [0.632, 0.342], [0.628, 0.347], [0.625, 0.352], [0.621, 0.357], [0.617, 0.362], [0.613, 0.367], [0.609, 0.372], [0.605, 0.377], [0.601, 0.381], [0.597, 0.386], [0.592, 0.39], [0.588, 0.395], [0.584, 0.399], [0.579, 0.404], [0.574, 0.408], [0.57, 0.412], [0.565, 0.416], [0.56, 0.42], [0.555, 0.424], [0.55, 0.428], [0.545, 0.431], [0.54, 0.435], [0.535, 0.439], [0.529, 0.442], [0.524, 0.445], [0.519, 0.449], [0.513, 0.452], [0.508, 0.455], [0.502, 0.458], [0.497, 0.461], [0.491, 0.464], [0.485, 0.466], [0.48, 0.469], [0.474, 0.471], [0.468, 0.474], [0.462, 0.476], [0.456, 0.478], [0.451, 0.48], [0.445, 0.482], [0.439, 0.484], [0.433, 0.486], [0.426, 0.488], [0.42, 0.489], [0.414, 0.491], [0.408, 0.492], [0.402, 0.493], [0.396, 0.495], [0.39, 0.496], [0.383, 0.497], [0.377, 0.497], [0.371, 0.498], [0.365, 0.499], [0.358, 0.499], [0.352, 0.5], [0.346, 0.5], [0.34, 0.5], [0.333, 0.5], [0.327, 0.5], [0.321, 0.5], [0.314, 0.5], [0.308, 0.499], [0.302, 0.499], [0.296, 0.498], [0.289, 0.497], [0.283, 0.497], [0.277, 0.496], [0.271, 0.495], [0.265, 0.493], [0.259, 0.492], [0.252, 0.491], [0.246, 0.489], [0.24, 0.488], [0.234, 0.486], [0.228, 0.484], [0.222, 0.482], [0.216, 0.48], [0.21, 0.478], [0.204, 0.476], [0.198, 0.474], [0.193, 0.471], [0.187, 0.469], [0.181, 0.466], [0.176, 0.464], [0.17, 0.461], [0.164, 0.458], [0.159, 0.455], [0.153, 0.452], [0.148, 0.449], [0.143, 0.445], [0.137, 0.442], [0.132, 0.439], [0.127, 0.435], [0.122, 0.431], [0.117, 0.428], [0.112, 0.424], [0.107, 0.42], [0.102, 0.416], [0.097, 0.412], [0.092, 0.408], [0.088, 0.404], [0.083, 0.399], [0.079, 0.395], [0.074, 0.39], [0.07, 0.386], [0.066, 0.381], [0.061, 0.377], [0.057, 0.372], [0.053, 0.367], [0.049, 0.362], [0.046, 0.357], [0.042, 0.352], [0.038, 0.347], [0.035, 0.342], [0.031, 0.337], [0.028, 0.331], [0.024, 0.326], [0.021, 0.321], [0.018, 0.315], [0.015, 0.31], [0.012, 0.304], [0.009, 0.298], [0.007, 0.293], [0.004, 0.287], [0.001, 0.281], [-0.001, 0.275], [-0.003, 0.27], [-0.005, 0.264], [-0.008, 0.258], [-0.01, 0.252], [-0.012, 0.246], [-0.013, 0.24], [-0.015, 0.234], [-0.017, 0.228], [-0.018, 0.222], [-0.02, 0.216], [-0.021, 0.209], [-0.022, 0.203], [-0.023, 0.197], [-0.024, 0.191], [-0.025, 0.185], [-0.026, 0.178], [-0.026, 0.172], [-0.027, 0.166], [-0.027, 0.16], [-0.027, 0.153], [-0.028, 0.147], [-0.028, 0.141], [-0.028, 0.135], [-0.028, 0.128], [-0.027, 0.122], [-0.027, 0.116], [-0.027, 0.109], [-0.026, 0.103], [-0.025, 0.097], [-0.025, 0.091], [-0.024, 0.084], [-0.023, 0.078], [-0.022, 0.072], [-0.02, 0.066], [-0.019, 0.06], [-0.018, 0.054], [-0.016, 0.048], [-0.014, 0.042], [-0.013, 0.035], [-0.011, 0.029], [-0.009, 0.024], [-0.007, 0.018], [-0.005, 0.012], [-0.002, 0.006], [0, 0]], [ [0, 0], [0.033, 0], [0.067, 0], [0.066, 0.001], [0.066, 0.001], [0.065, 0.002], [0.065, 0.002], [0.064, 0.003], [0.064, 0.003], [0.063, 0.004], [0.063, 0.004], [0.062, 0.005], [0.062, 0.005], [0.061, 0.006], [0.061, 0.006], [0.06, 0.007], [0.06, 0.007], [0.059, 0.008], [0.059, 0.008], [0.058, 0.009], [0.057, 0.009], [0.057, 0.009], [0.056, 0.01], [0.056, 0.01], [0.055, 0.011], [0.054, 0.011], [0.054, 0.011], [0.053, 0.012], [0.052, 0.012], [0.052, 0.012], [0.051, 0.013], [0.051, 0.013], [0.05, 0.013], [0.049, 0.014], [0.049, 0.014], [0.048, 0.014], [0.047, 0.014], [0.046, 0.015], [0.046, 0.015], [0.045, 0.015], [0.044, 0.015], [0.044, 0.015], [0.043, 0.016], [0.042, 0.016], [0.042, 0.016], [0.041, 0.016], [0.04, 0.016], [0.039, 0.016], [0.039, 0.016], [0.038, 0.016], [0.037, 0.016], [0.037, 0.017], [0.036, 0.017], [0.035, 0.017], [0.034, 0.017], [0.034, 0.017], [0.033, 0.017], [0.032, 0.017], [0.032, 0.017], [0.031, 0.017], [0.03, 0.017], [0.029, 0.016], [0.029, 0.016], [0.028, 0.016], [0.027, 0.016], [0.027, 0.016], [0.026, 0.016], [0.025, 0.016], [0.024, 0.016], [0.024, 0.016], [0.023, 0.015], [0.022, 0.015], [0.022, 0.015], [0.021, 0.015], [0.02, 0.015], [0.02, 0.014], [0.019, 0.014], [0.018, 0.014], [0.017, 0.014], [0.017, 0.013], [0.016, 0.013], [0.016, 0.013], [0.015, 0.012], [0.014, 0.012], [0.014, 0.012], [0.013, 0.011], [0.012, 0.011], [0.012, 0.011], [0.011, 0.01], [0.01, 0.01], [0.01, 0.009], [0.009, 0.009], [0.009, 0.009], [0.008, 0.008], [0.008, 0.008], [0.007, 0.007], [0.006, 0.007], [0.006, 0.006], [0.005, 0.006], [0.005, 0.005], [0.004, 0.005], [0.004, 0.004], [0.003, 0.004], [0.003, 0.003], [0.002, 0.003], [0.002, 0.002], [0.001, 0.002], [0.001, 0.001], [0, 0.001], [0, 0]]] }" ) );
  res = exportPolygonFloat.asJson( 3 );
  QCOMPARE( exportPolygonFloat.asJson( 3 ), expectedJsonPrec3 );

  // as GML2
  QString expectedGML2( QStringLiteral( "<Polygon xmlns=\"gml\"><outerBoundaryIs xmlns=\"gml\"><LinearRing xmlns=\"gml\"><coordinates xmlns=\"gml\" cs=\",\" ts=\" \">0,0 1,0 2,0 1.98685,0.01722 1.97341,0.03421 1.95967,0.05096 1.94564,0.06747 1.93133,0.08374 1.91674,0.09976 1.90188,0.11552 1.88674,0.13102 1.87134,0.14625 1.85567,0.16122 1.83975,0.17592 1.82358,0.19033 1.80716,0.20446 1.79049,0.21831 1.77359,0.23186 1.75646,0.24512 1.7391,0.25809 1.72151,0.27074 1.70371,0.2831 1.6857,0.29514 1.66748,0.30687 1.64907,0.31828 1.63045,0.32936 1.61165,0.34013 1.59267,0.35057 1.5735,0.36067 1.55417,0.37045 1.53466,0.37988 1.515,0.38898 1.49518,0.39773 1.47522,0.40614 1.45511,0.41421 1.43486,0.42192 1.41448,0.42928 1.39398,0.43629 1.37336,0.44294 1.35263,0.44923 1.33179,0.45516 1.31086,0.46073 1.28983,0.46594 1.26871,0.47078 1.24751,0.47525 1.22624,0.47936 1.2049,0.48309 1.18349,0.48646 1.16204,0.48945 1.14053,0.49208 1.11898,0.49432 1.0974,0.4962 1.07578,0.4977 1.05415,0.49883 1.0325,0.49958 1.01083,0.49995 0.98917,0.49995 0.9675,0.49958 0.94585,0.49883 0.92422,0.4977 0.9026,0.4962 0.88102,0.49432 0.85947,0.49208 0.83796,0.48945 0.81651,0.48646 0.7951,0.48309 0.77376,0.47936 0.75249,0.47525 0.73129,0.47078 0.71017,0.46594 0.68914,0.46073 0.66821,0.45516 0.64737,0.44923 0.62664,0.44294 0.60602,0.43629 0.58552,0.42928 0.56514,0.42192 0.54489,0.41421 0.52478,0.40614 0.50482,0.39773 0.485,0.38898 0.46534,0.37988 0.44583,0.37045 0.4265,0.36067 0.40733,0.35057 0.38835,0.34013 0.36955,0.32936 0.35093,0.31828 0.33252,0.30687 0.3143,0.29514 0.29629,0.2831 0.27849,0.27074 0.2609,0.25809 0.24354,0.24512 0.22641,0.23186 0.20951,0.21831 0.19284,0.20446 0.17642,0.19033 0.16025,0.17592 0.14433,0.16122 0.12866,0.14625 0.11326,0.13102 0.09812,0.11552 0.08326,0.09976 0.06867,0.08374 0.05436,0.06747 0.04033,0.05096 0.02659,0.03421 0.01315,0.01722 0,0</coordinates></LinearRing></outerBoundaryIs><innerBoundaryIs xmlns=\"gml\"><LinearRing xmlns=\"gml\"><coordinates xmlns=\"gml\" cs=\",\" ts=\" \">0,0 0.1,0 0.2,0 0.19869,0.00172 0.19734,0.00342 0.19597,0.0051 0.19456,0.00675 0.19313,0.00837 0.19167,0.00998 0.19019,0.01155 0.18867,0.0131 0.18713,0.01463 0.18557,0.01612 0.18398,0.01759 0.18236,0.01903 0.18072,0.02045 0.17905,0.02183 0.17736,0.02319 0.17565,0.02451 0.17391,0.02581 0.17215,0.02707 0.17037,0.02831 0.16857,0.02951 0.16675,0.03069 0.16491,0.03183 0.16305,0.03294 0.16117,0.03401 0.15927,0.03506 0.15735,0.03607 0.15542,0.03704 0.15347,0.03799 0.1515,0.0389 0.14952,0.03977 0.14752,0.04061 0.14551,0.04142 0.14349,0.04219 0.14145,0.04293 0.1394,0.04363 0.13734,0.04429 0.13526,0.04492 0.13318,0.04552 0.13109,0.04607 0.12898,0.04659 0.12687,0.04708 0.12475,0.04753 0.12262,0.04794 0.12049,0.04831 0.11835,0.04865 0.1162,0.04895 0.11405,0.04921 0.1119,0.04943 0.10974,0.04962 0.10758,0.04977 0.10541,0.04988 0.10325,0.04996 0.10108,0.05 0.09892,0.05 0.09675,0.04996 0.09459,0.04988 0.09242,0.04977 0.09026,0.04962 0.0881,0.04943 0.08595,0.04921 0.0838,0.04895 0.08165,0.04865 0.07951,0.04831 0.07738,0.04794 0.07525,0.04753 0.07313,0.04708 0.07102,0.04659 0.06891,0.04607 0.06682,0.04552 0.06474,0.04492 0.06266,0.04429 0.0606,0.04363 0.05855,0.04293 0.05651,0.04219 0.05449,0.04142 0.05248,0.04061 0.05048,0.03977 0.0485,0.0389 0.04653,0.03799 0.04458,0.03704 0.04265,0.03607 0.04073,0.03506 0.03883,0.03401 0.03695,0.03294 0.03509,0.03183 0.03325,0.03069 0.03143,0.02951 0.02963,0.02831 0.02785,0.02707 0.02609,0.02581 0.02435,0.02451 0.02264,0.02319 0.02095,0.02183 0.01928,0.02045 0.01764,0.01903 0.01602,0.01759 0.01443,0.01612 0.01287,0.01463 0.01133,0.0131 0.00981,0.01155 0.00833,0.00998 0.00687,0.00837 0.00544,0.00675 0.00403,0.0051 0.00266,0.00342 0.00131,0.00172 0,0</coordinates></LinearRing></innerBoundaryIs></Polygon>" ) );
  res = elemToString( exportPolygon.asGml2( doc, 5 ) );
  QGSCOMPAREGML( res, expectedGML2 );

  QString expectedGML2prec2( QStringLiteral( "<Polygon xmlns=\"gml\"><outerBoundaryIs xmlns=\"gml\"><LinearRing xmlns=\"gml\"><coordinates xmlns=\"gml\" cs=\",\" ts=\" \">0,0 1,0 2,0 1.99,0.02 1.97,0.03 1.96,0.05 1.95,0.07 1.93,0.08 1.92,0.1 1.9,0.12 1.89,0.13 1.87,0.15 1.86,0.16 1.84,0.18 1.82,0.19 1.81,0.2 1.79,0.22 1.77,0.23 1.76,0.25 1.74,0.26 1.72,0.27 1.7,0.28 1.69,0.3 1.67,0.31 1.65,0.32 1.63,0.33 1.61,0.34 1.59,0.35 1.57,0.36 1.55,0.37 1.53,0.38 1.52,0.39 1.5,0.4 1.48,0.41 1.46,0.41 1.43,0.42 1.41,0.43 1.39,0.44 1.37,0.44 1.35,0.45 1.33,0.46 1.31,0.46 1.29,0.47 1.27,0.47 1.25,0.48 1.23,0.48 1.2,0.48 1.18,0.49 1.16,0.49 1.14,0.49 1.12,0.49 1.1,0.5 1.08,0.5 1.05,0.5 1.03,0.5 1.01,0.5 0.99,0.5 0.97,0.5 0.95,0.5 0.92,0.5 0.9,0.5 0.88,0.49 0.86,0.49 0.84,0.49 0.82,0.49 0.8,0.48 0.77,0.48 0.75,0.48 0.73,0.47 0.71,0.47 0.69,0.46 0.67,0.46 0.65,0.45 0.63,0.44 0.61,0.44 0.59,0.43 0.57,0.42 0.54,0.41 0.52,0.41 0.5,0.4 0.48,0.39 0.47,0.38 0.45,0.37 0.43,0.36 0.41,0.35 0.39,0.34 0.37,0.33 0.35,0.32 0.33,0.31 0.31,0.3 0.3,0.28 0.28,0.27 0.26,0.26 0.24,0.25 0.23,0.23 0.21,0.22 0.19,0.2 0.18,0.19 0.16,0.18 0.14,0.16 0.13,0.15 0.11,0.13 0.1,0.12 0.08,0.1 0.07,0.08 0.05,0.07 0.04,0.05 0.03,0.03 0.01,0.02 0,0</coordinates></LinearRing></outerBoundaryIs><innerBoundaryIs xmlns=\"gml\"><LinearRing xmlns=\"gml\"><coordinates xmlns=\"gml\" cs=\",\" ts=\" \">0,0 0.1,0 0.2,0 0.2,0 0.2,0 0.2,0.01 0.19,0.01 0.19,0.01 0.19,0.01 0.19,0.01 0.19,0.01 0.19,0.01 0.19,0.02 0.18,0.02 0.18,0.02 0.18,0.02 0.18,0.02 0.18,0.02 0.18,0.02 0.17,0.03 0.17,0.03 0.17,0.03 0.17,0.03 0.17,0.03 0.16,0.03 0.16,0.03 0.16,0.03 0.16,0.04 0.16,0.04 0.16,0.04 0.15,0.04 0.15,0.04 0.15,0.04 0.15,0.04 0.15,0.04 0.14,0.04 0.14,0.04 0.14,0.04 0.14,0.04 0.14,0.04 0.13,0.05 0.13,0.05 0.13,0.05 0.13,0.05 0.12,0.05 0.12,0.05 0.12,0.05 0.12,0.05 0.12,0.05 0.11,0.05 0.11,0.05 0.11,0.05 0.11,0.05 0.11,0.05 0.1,0.05 0.1,0.05 0.1,0.05 0.1,0.05 0.09,0.05 0.09,0.05 0.09,0.05 0.09,0.05 0.09,0.05 0.08,0.05 0.08,0.05 0.08,0.05 0.08,0.05 0.08,0.05 0.07,0.05 0.07,0.05 0.07,0.05 0.07,0.05 0.06,0.04 0.06,0.04 0.06,0.04 0.06,0.04 0.06,0.04 0.05,0.04 0.05,0.04 0.05,0.04 0.05,0.04 0.05,0.04 0.04,0.04 0.04,0.04 0.04,0.04 0.04,0.03 0.04,0.03 0.04,0.03 0.03,0.03 0.03,0.03 0.03,0.03 0.03,0.03 0.03,0.03 0.02,0.02 0.02,0.02 0.02,0.02 0.02,0.02 0.02,0.02 0.02,0.02 0.01,0.02 0.01,0.01 0.01,0.01 0.01,0.01 0.01,0.01 0.01,0.01 0.01,0.01 0,0.01 0,0 0,0 0,0</coordinates></LinearRing></innerBoundaryIs></Polygon>" ) );
  res = elemToString( exportPolygon.asGml2( doc, 2 ) );
  QGSCOMPAREGML( res, expectedGML2prec2 );

  //as GML3
  QString expectedGML3( QStringLiteral( "<Polygon xmlns=\"gml\"><exterior xmlns=\"gml\"><Curve xmlns=\"gml\"><segments xmlns=\"gml\"><ArcString xmlns=\"gml\"><posList xmlns=\"gml\" srsDimension=\"3\">0 0 10 1 0 11 2 0 12 1 0.5 13 0 0 10</posList></ArcString></segments></Curve></exterior><interior xmlns=\"gml\"><Curve xmlns=\"gml\"><segments xmlns=\"gml\"><ArcString xmlns=\"gml\"><posList xmlns=\"gml\" srsDimension=\"3\">0 0 10 0.10000000000000001 0 11 0.20000000000000001 0 12 0.10000000000000001 0.05 13 0 0 10</posList></ArcString></segments></Curve></interior></Polygon>" ) );
  res = elemToString( exportPolygon.asGml3( doc ) );
  QCOMPARE( res, expectedGML3 );

  QString expectedGML3prec3( QStringLiteral( "<Polygon xmlns=\"gml\"><exterior xmlns=\"gml\"><Curve xmlns=\"gml\"><segments xmlns=\"gml\"><ArcString xmlns=\"gml\"><posList xmlns=\"gml\" srsDimension=\"3\">0 0 10 1 0 11 2 0 12 1 0.5 13 0 0 10</posList></ArcString></segments></Curve></exterior><interior xmlns=\"gml\"><Curve xmlns=\"gml\"><segments xmlns=\"gml\"><ArcString xmlns=\"gml\"><posList xmlns=\"gml\" srsDimension=\"3\">0 0 10 0.1 0 11 0.2 0 12 0.1 0.05 13 0 0 10</posList></ArcString></segments></Curve></interior></Polygon>" ) );
  res = elemToString( exportPolygon.asGml3( doc, 3 ) );
  QCOMPARE( res, expectedGML3prec3 );

  //removing the fourth to last vertex removes the whole ring
  QgsCurvePolygon p20;
  QgsCircularString *p20ExteriorRing = new QgsCircularString();
  p20ExteriorRing->setPoints( QVector<QgsPoint>() << QgsPoint( 0, 0 ) << QgsPoint( 1, 0 ) << QgsPoint( 1, 1 ) << QgsPoint( 0, 0 ) );
  p20.setExteriorRing( p20ExteriorRing );
  QVERIFY( p20.exteriorRing() );
  p20.deleteVertex( QgsVertexId( 0, 0, 2 ) );
  QVERIFY( !p20.exteriorRing() );

  //boundary
  QgsCircularString boundary1;
  boundary1.setPoints( QgsPointSequence() << QgsPoint( 0, 0, 1 ) << QgsPoint( 1, 0, 2 ) << QgsPoint( 2, 0, 3 )
                       << QgsPoint( 1, 0.5, 4 ) << QgsPoint( 0, 0, 1 ) );
  QgsCurvePolygon boundaryPolygon;
  QVERIFY( !boundaryPolygon.boundary() );

  boundaryPolygon.setExteriorRing( boundary1.clone() );
  QgsAbstractGeometry *boundary = boundaryPolygon.boundary();
  QgsCircularString *lineBoundary = dynamic_cast< QgsCircularString * >( boundary );
  QVERIFY( lineBoundary );
  QCOMPARE( lineBoundary->numPoints(), 5 );
  QCOMPARE( lineBoundary->xAt( 0 ), 0.0 );
  QCOMPARE( lineBoundary->xAt( 1 ), 1.0 );
  QCOMPARE( lineBoundary->xAt( 2 ), 2.0 );
  QCOMPARE( lineBoundary->xAt( 3 ), 1.0 );
  QCOMPARE( lineBoundary->xAt( 4 ), 0.0 );
  QCOMPARE( lineBoundary->yAt( 0 ), 0.0 );
  QCOMPARE( lineBoundary->yAt( 1 ), 0.0 );
  QCOMPARE( lineBoundary->yAt( 2 ), 0.0 );
  QCOMPARE( lineBoundary->yAt( 3 ), 0.5 );
  QCOMPARE( lineBoundary->yAt( 4 ), 0.0 );
  delete boundary;

  // add interior rings
  QgsCircularString boundaryRing1;
  boundaryRing1.setPoints( QVector<QgsPoint>() << QgsPoint( 0.1, 0.1 ) << QgsPoint( 0.2, 0.1 ) << QgsPoint( 0.2, 0.2 ) );
  QgsCircularString boundaryRing2;
  boundaryRing2.setPoints( QVector<QgsPoint>() << QgsPoint( 0.8, 0.8 ) << QgsPoint( 0.9, 0.8 ) << QgsPoint( 0.9, 0.9 ) );
  boundaryPolygon.setInteriorRings( QVector< QgsCurve * >() << boundaryRing1.clone() << boundaryRing2.clone() );
  boundary = boundaryPolygon.boundary();
  QgsMultiCurve *multiLineBoundary = dynamic_cast< QgsMultiCurve * >( boundary );
  QVERIFY( multiLineBoundary );
  QCOMPARE( multiLineBoundary->numGeometries(), 3 );
  QCOMPARE( dynamic_cast< QgsCircularString * >( multiLineBoundary->geometryN( 0 ) )->numPoints(), 5 );
  QCOMPARE( dynamic_cast< QgsCircularString * >( multiLineBoundary->geometryN( 0 ) )->xAt( 0 ), 0.0 );
  QCOMPARE( dynamic_cast< QgsCircularString * >( multiLineBoundary->geometryN( 0 ) )->xAt( 1 ), 1.0 );
  QCOMPARE( dynamic_cast< QgsCircularString * >( multiLineBoundary->geometryN( 0 ) )->xAt( 2 ), 2.0 );
  QCOMPARE( dynamic_cast< QgsCircularString * >( multiLineBoundary->geometryN( 0 ) )->xAt( 3 ), 1.0 );
  QCOMPARE( dynamic_cast< QgsCircularString * >( multiLineBoundary->geometryN( 0 ) )->xAt( 4 ), 0.0 );
  QCOMPARE( dynamic_cast< QgsCircularString * >( multiLineBoundary->geometryN( 0 ) )->yAt( 0 ), 0.0 );
  QCOMPARE( dynamic_cast< QgsCircularString * >( multiLineBoundary->geometryN( 0 ) )->yAt( 1 ), 0.0 );
  QCOMPARE( dynamic_cast< QgsCircularString * >( multiLineBoundary->geometryN( 0 ) )->yAt( 2 ), 0.0 );
  QCOMPARE( dynamic_cast< QgsCircularString * >( multiLineBoundary->geometryN( 0 ) )->yAt( 3 ), 0.5 );
  QCOMPARE( dynamic_cast< QgsCircularString * >( multiLineBoundary->geometryN( 0 ) )->yAt( 4 ), 0.0 );
  QCOMPARE( dynamic_cast< QgsCircularString * >( multiLineBoundary->geometryN( 1 ) )->numPoints(), 3 );
  QCOMPARE( dynamic_cast< QgsCircularString * >( multiLineBoundary->geometryN( 1 ) )->xAt( 0 ), 0.1 );
  QCOMPARE( dynamic_cast< QgsCircularString * >( multiLineBoundary->geometryN( 1 ) )->xAt( 1 ), 0.2 );
  QCOMPARE( dynamic_cast< QgsCircularString * >( multiLineBoundary->geometryN( 1 ) )->xAt( 2 ), 0.2 );
  QCOMPARE( dynamic_cast< QgsCircularString * >( multiLineBoundary->geometryN( 1 ) )->yAt( 0 ), 0.1 );
  QCOMPARE( dynamic_cast< QgsCircularString * >( multiLineBoundary->geometryN( 1 ) )->yAt( 1 ), 0.1 );
  QCOMPARE( dynamic_cast< QgsCircularString * >( multiLineBoundary->geometryN( 1 ) )->yAt( 2 ), 0.2 );
  QCOMPARE( dynamic_cast< QgsCircularString * >( multiLineBoundary->geometryN( 2 ) )->numPoints(), 3 );
  QCOMPARE( dynamic_cast< QgsCircularString * >( multiLineBoundary->geometryN( 2 ) )->xAt( 0 ), 0.8 );
  QCOMPARE( dynamic_cast< QgsCircularString * >( multiLineBoundary->geometryN( 2 ) )->xAt( 1 ), 0.9 );
  QCOMPARE( dynamic_cast< QgsCircularString * >( multiLineBoundary->geometryN( 2 ) )->xAt( 2 ), 0.9 );
  QCOMPARE( dynamic_cast< QgsCircularString * >( multiLineBoundary->geometryN( 2 ) )->yAt( 0 ), 0.8 );
  QCOMPARE( dynamic_cast< QgsCircularString * >( multiLineBoundary->geometryN( 2 ) )->yAt( 1 ), 0.8 );
  QCOMPARE( dynamic_cast< QgsCircularString * >( multiLineBoundary->geometryN( 2 ) )->yAt( 2 ), 0.9 );
  boundaryPolygon.setInteriorRings( QVector< QgsCurve * >() );
  delete boundary;

  //test boundary with z
  boundary1.setPoints( QVector<QgsPoint>() << QgsPoint( QgsWkbTypes::PointZ, 0, 0, 10 ) << QgsPoint( QgsWkbTypes::PointZ, 1, 0, 15 )
                       << QgsPoint( QgsWkbTypes::PointZ, 1, 1, 20 ) );
  boundaryPolygon.setExteriorRing( boundary1.clone() );
  boundary = boundaryPolygon.boundary();
  lineBoundary = dynamic_cast< QgsCircularString * >( boundary );
  QVERIFY( lineBoundary );
  QCOMPARE( lineBoundary->numPoints(), 3 );
  QCOMPARE( lineBoundary->wkbType(), QgsWkbTypes::CircularStringZ );
  QCOMPARE( lineBoundary->pointN( 0 ).z(), 10.0 );
  QCOMPARE( lineBoundary->pointN( 1 ).z(), 15.0 );
  QCOMPARE( lineBoundary->pointN( 2 ).z(), 20.0 );
  delete boundary;

  // remove interior rings
  QgsCircularString removeRingsExt;
  removeRingsExt.setPoints( QVector<QgsPoint>() << QgsPoint( 0, 0 ) << QgsPoint( 1, 0 ) << QgsPoint( 1, 1 )  << QgsPoint( 0, 0 ) );
  QgsCurvePolygon removeRings1;
  removeRings1.removeInteriorRings();

  removeRings1.setExteriorRing( boundary1.clone() );
  removeRings1.removeInteriorRings();
  QCOMPARE( removeRings1.numInteriorRings(), 0 );

  // add interior rings
  QgsCircularString removeRingsRing1;
  removeRingsRing1.setPoints( QgsPointSequence() << QgsPoint( 0, 0, 1 ) << QgsPoint( 0.1, 1, 2 ) << QgsPoint( 0, 2, 3 )
                              << QgsPoint( -0.1, 1.2, 4 ) << QgsPoint( 0, 0, 1 ) );
  QgsCircularString removeRingsRing2;
  removeRingsRing2.setPoints( QgsPointSequence() << QgsPoint( 0, 0, 1 ) << QgsPoint( 0.01, 0.1, 2 ) << QgsPoint( 0, 0.2, 3 )
                              << QgsPoint( -0.01, 0.12, 4 ) << QgsPoint( 0, 0, 1 ) );
  removeRings1.setInteriorRings( QVector< QgsCurve * >() << removeRingsRing1.clone() << removeRingsRing2.clone() );

  // remove ring with size filter
  removeRings1.removeInteriorRings( 0.05 );
  QCOMPARE( removeRings1.numInteriorRings(), 1 );

  // remove ring with no size filter
  removeRings1.removeInteriorRings();
  QCOMPARE( removeRings1.numInteriorRings(), 0 );

  // cast
  QVERIFY( !QgsCurvePolygon().cast( nullptr ) );
  QgsCurvePolygon pCast;
  QVERIFY( QgsCurvePolygon().cast( &pCast ) );
  QgsCurvePolygon pCast2;
  pCast2.fromWkt( QStringLiteral( "CurvePolygonZ((0 0 0, 0 1 1, 1 0 2, 0 0 0))" ) );
  QVERIFY( QgsCurvePolygon().cast( &pCast2 ) );
  pCast2.fromWkt( QStringLiteral( "CurvePolygonM((0 0 1, 0 1 2, 1 0 3, 0 0 1))" ) );
  QVERIFY( QgsCurvePolygon().cast( &pCast2 ) );
  pCast2.fromWkt( QStringLiteral( "CurvePolygonZM((0 0 0 1, 0 1 1 2, 1 0 2 3, 0 0 0 1))" ) );
  QVERIFY( QgsCurvePolygon().cast( &pCast2 ) );

  // draw - most tests are in test_qgsgeometry.py
  QgsCurvePolygon empty;
  QPainter p;
  empty.draw( p ); //no crash!


  // closestSegment
  QgsPoint pt;
  QgsVertexId v;
  int leftOf = 0;
  ( void )empty.closestSegment( QgsPoint( 1, 2 ), pt, v ); // empty curve, just want no crash

  QgsCurvePolygon cp12;
  QgsLineString cp12ls;
  cp12ls.setPoints( QgsPointSequence() << QgsPoint( 5, 10 ) << QgsPoint( 7, 12 ) << QgsPoint( 5, 15 ) << QgsPoint( 5, 10 ) );
  cp12.setExteriorRing( cp12ls.clone() );
  QGSCOMPARENEAR( cp12.closestSegment( QgsPoint( 4, 11 ), pt, v, &leftOf ), 1.0, 0.0001 );
  QGSCOMPARENEAR( pt.x(), 5, 0.01 );
  QGSCOMPARENEAR( pt.y(), 11, 0.01 );
  QCOMPARE( v, QgsVertexId( 0, 0, 3 ) );
  QCOMPARE( leftOf, 1 );
  QGSCOMPARENEAR( cp12.closestSegment( QgsPoint( 8, 11 ), pt, v, &leftOf ),  2.0, 0.0001 );
  QGSCOMPARENEAR( pt.x(), 7, 0.01 );
  QGSCOMPARENEAR( pt.y(), 12, 0.01 );
  QCOMPARE( v, QgsVertexId( 0, 0, 1 ) );
  QCOMPARE( leftOf, 1 );
  QGSCOMPARENEAR( cp12.closestSegment( QgsPoint( 6, 11.5 ), pt, v, &leftOf ), 0.125000, 0.0001 );
  QGSCOMPARENEAR( pt.x(), 6.25, 0.01 );
  QGSCOMPARENEAR( pt.y(), 11.25, 0.01 );
  QCOMPARE( v, QgsVertexId( 0, 0, 1 ) );
  QCOMPARE( leftOf, -1 );
  QGSCOMPARENEAR( cp12.closestSegment( QgsPoint( 7, 16 ), pt, v, &leftOf ), 4.923077, 0.0001 );
  QGSCOMPARENEAR( pt.x(), 5.153846, 0.01 );
  QGSCOMPARENEAR( pt.y(), 14.769231, 0.01 );
  QCOMPARE( v, QgsVertexId( 0, 0, 2 ) );
  QCOMPARE( leftOf, 1 );
  QGSCOMPARENEAR( cp12.closestSegment( QgsPoint( 5.5, 13.5 ), pt, v, &leftOf ), 0.173077, 0.0001 );
  QGSCOMPARENEAR( pt.x(), 5.846154, 0.01 );
  QGSCOMPARENEAR( pt.y(), 13.730769, 0.01 );
  QCOMPARE( v, QgsVertexId( 0, 0, 2 ) );
  QCOMPARE( leftOf, -1 );
  // point directly on segment
  QCOMPARE( cp12.closestSegment( QgsPoint( 5, 15 ), pt, v, &leftOf ), 0.0 );
  QCOMPARE( pt, QgsPoint( 5, 15 ) );
  QCOMPARE( v, QgsVertexId( 0, 0, 2 ) );
  QCOMPARE( leftOf, 0 );

  // with interior ring
  cp12ls.setPoints( QgsPointSequence() << QgsPoint( 6, 11.5 ) << QgsPoint( 6.5, 12 ) << QgsPoint( 6, 13 ) << QgsPoint( 6, 11.5 ) );
  cp12.addInteriorRing( cp12ls.clone() );
  QGSCOMPARENEAR( cp12.closestSegment( QgsPoint( 4, 11 ), pt, v, &leftOf ), 1.0, 0.0001 );
  QGSCOMPARENEAR( pt.x(), 5, 0.01 );
  QGSCOMPARENEAR( pt.y(), 11, 0.01 );
  QCOMPARE( v, QgsVertexId( 0, 0, 3 ) );
  QCOMPARE( leftOf, 1 );
  QGSCOMPARENEAR( cp12.closestSegment( QgsPoint( 8, 11 ), pt, v, &leftOf ),  2.0, 0.0001 );
  QGSCOMPARENEAR( pt.x(), 7, 0.01 );
  QGSCOMPARENEAR( pt.y(), 12, 0.01 );
  QCOMPARE( v, QgsVertexId( 0, 0, 1 ) );
  QCOMPARE( leftOf, 1 );
  QGSCOMPARENEAR( cp12.closestSegment( QgsPoint( 6, 11.4 ), pt, v, &leftOf ), 0.01, 0.0001 );
  QGSCOMPARENEAR( pt.x(), 6.0, 0.01 );
  QGSCOMPARENEAR( pt.y(), 11.5, 0.01 );
  QCOMPARE( v, QgsVertexId( 0, 1, 1 ) );
  QCOMPARE( leftOf, 1 );
  QGSCOMPARENEAR( cp12.closestSegment( QgsPoint( 7, 16 ), pt, v, &leftOf ), 4.923077, 0.0001 );
  QGSCOMPARENEAR( pt.x(), 5.153846, 0.01 );
  QGSCOMPARENEAR( pt.y(), 14.769231, 0.01 );
  QCOMPARE( v, QgsVertexId( 0, 0, 2 ) );
  QCOMPARE( leftOf, 1 );
  QGSCOMPARENEAR( cp12.closestSegment( QgsPoint( 5.5, 13.5 ), pt, v, &leftOf ), 0.173077, 0.0001 );
  QGSCOMPARENEAR( pt.x(), 5.846154, 0.01 );
  QGSCOMPARENEAR( pt.y(), 13.730769, 0.01 );
  QCOMPARE( v, QgsVertexId( 0, 0, 2 ) );
  QCOMPARE( leftOf, -1 );
  // point directly on segment
  QCOMPARE( cp12.closestSegment( QgsPoint( 6, 13 ), pt, v, &leftOf ), 0.0 );
  QCOMPARE( pt, QgsPoint( 6, 13 ) );
  QCOMPARE( v, QgsVertexId( 0, 1, 2 ) );
  QCOMPARE( leftOf, 0 );

  //nextVertex
  QgsCurvePolygon cp13;
  QVERIFY( !cp13.nextVertex( v, pt ) );
  v = QgsVertexId( 0, 0, -2 );
  QVERIFY( !cp13.nextVertex( v, pt ) );
  v = QgsVertexId( 0, 0, 10 );
  QVERIFY( !cp13.nextVertex( v, pt ) );
  QgsLineString lp22;
  lp22.setPoints( QgsPointSequence() << QgsPoint( 1, 2 ) << QgsPoint( 11, 12 ) << QgsPoint( 1, 12 ) << QgsPoint( 1, 2 ) );
  cp13.setExteriorRing( lp22.clone() );
  v = QgsVertexId( 0, 0, 4 ); //out of range
  QVERIFY( !cp13.nextVertex( v, pt ) );
  v = QgsVertexId( 0, 0, -5 );
  QVERIFY( cp13.nextVertex( v, pt ) );
  v = QgsVertexId( 0, 0, -1 );
  QVERIFY( cp13.nextVertex( v, pt ) );
  QCOMPARE( v, QgsVertexId( 0, 0, 0 ) );
  QCOMPARE( pt, QgsPoint( 1, 2 ) );
  QVERIFY( cp13.nextVertex( v, pt ) );
  QCOMPARE( v, QgsVertexId( 0, 0, 1 ) );
  QCOMPARE( pt, QgsPoint( 11, 12 ) );
  QVERIFY( cp13.nextVertex( v, pt ) );
  QCOMPARE( v, QgsVertexId( 0, 0, 2 ) );
  QCOMPARE( pt, QgsPoint( 1, 12 ) );
  QVERIFY( cp13.nextVertex( v, pt ) );
  QCOMPARE( v, QgsVertexId( 0, 0, 3 ) );
  QCOMPARE( pt, QgsPoint( 1, 2 ) );
  v = QgsVertexId( 0, 1, 0 );
  QVERIFY( !cp13.nextVertex( v, pt ) );
  v = QgsVertexId( 1, 0, 0 );
  QVERIFY( cp13.nextVertex( v, pt ) );
  QCOMPARE( v, QgsVertexId( 1, 0, 1 ) ); //test that part number is maintained
  QCOMPARE( pt, QgsPoint( 11, 12 ) );
  // add interior ring
  lp22.setPoints( QgsPointSequence() << QgsPoint( 11, 12 ) << QgsPoint( 21, 22 ) << QgsPoint( 11, 22 ) << QgsPoint( 11, 12 ) );
  cp13.addInteriorRing( lp22.clone() );
  v = QgsVertexId( 0, 1, 4 ); //out of range
  QVERIFY( !cp13.nextVertex( v, pt ) );
  v = QgsVertexId( 0, 1, -5 );
  QVERIFY( cp13.nextVertex( v, pt ) );
  v = QgsVertexId( 0, 1, -1 );
  QVERIFY( cp13.nextVertex( v, pt ) );
  QCOMPARE( v, QgsVertexId( 0, 1, 0 ) );
  QCOMPARE( pt, QgsPoint( 11, 12 ) );
  QVERIFY( cp13.nextVertex( v, pt ) );
  QCOMPARE( v, QgsVertexId( 0, 1, 1 ) );
  QCOMPARE( pt, QgsPoint( 21, 22 ) );
  QVERIFY( cp13.nextVertex( v, pt ) );
  QCOMPARE( v, QgsVertexId( 0, 1, 2 ) );
  QCOMPARE( pt, QgsPoint( 11, 22 ) );
  QVERIFY( cp13.nextVertex( v, pt ) );
  QCOMPARE( v, QgsVertexId( 0, 1, 3 ) );
  QCOMPARE( pt, QgsPoint( 11, 12 ) );
  v = QgsVertexId( 0, 2, 0 );
  QVERIFY( !cp13.nextVertex( v, pt ) );
  v = QgsVertexId( 1, 1, 0 );
  QVERIFY( cp13.nextVertex( v, pt ) );
  QCOMPARE( v, QgsVertexId( 1, 1, 1 ) ); //test that part number is maintained
  QCOMPARE( pt, QgsPoint( 21, 22 ) );

  // dropZValue
  QgsCurvePolygon p23;
  p23.dropZValue();
  QCOMPARE( p23.wkbType(), QgsWkbTypes::CurvePolygon );
  QgsLineString lp23;
  lp23.setPoints( QgsPointSequence() << QgsPoint( 1, 2 ) << QgsPoint( 11, 12 ) << QgsPoint( 1, 12 ) << QgsPoint( 1, 2 ) );
  p23.setExteriorRing( lp23.clone() );
  p23.addInteriorRing( lp23.clone() );
  QCOMPARE( p23.wkbType(), QgsWkbTypes::CurvePolygon );
  p23.dropZValue(); // not z
  QCOMPARE( p23.wkbType(), QgsWkbTypes::CurvePolygon );
  QCOMPARE( p23.exteriorRing()->wkbType(), QgsWkbTypes::LineString );
  QCOMPARE( static_cast< const QgsLineString *>( p23.exteriorRing() )->pointN( 0 ), QgsPoint( 1, 2 ) );
  QCOMPARE( p23.interiorRing( 0 )->wkbType(), QgsWkbTypes::LineString );
  QCOMPARE( static_cast< const QgsLineString *>( p23.interiorRing( 0 ) )->pointN( 0 ), QgsPoint( 1, 2 ) );
  // with z
  lp23.setPoints( QgsPointSequence() << QgsPoint( 1, 2, 3 ) << QgsPoint( 11, 12, 13 ) << QgsPoint( 1, 12, 23 ) << QgsPoint( 1, 2, 3 ) );
  p23.clear();
  p23.setExteriorRing( lp23.clone() );
  p23.addInteriorRing( lp23.clone() );
  QCOMPARE( p23.wkbType(), QgsWkbTypes::CurvePolygonZ );
  p23.dropZValue();
  QCOMPARE( p23.wkbType(), QgsWkbTypes::CurvePolygon );
  QCOMPARE( p23.exteriorRing()->wkbType(), QgsWkbTypes::LineString );
  QCOMPARE( static_cast< const QgsLineString *>( p23.exteriorRing() )->pointN( 0 ), QgsPoint( 1, 2 ) );
  QCOMPARE( p23.interiorRing( 0 )->wkbType(), QgsWkbTypes::LineString );
  QCOMPARE( static_cast< const QgsLineString *>( p23.interiorRing( 0 ) )->pointN( 0 ), QgsPoint( 1, 2 ) );
  // with zm
  lp23.setPoints( QgsPointSequence() << QgsPoint( 1, 2, 3, 4 ) << QgsPoint( 11, 12, 13, 14 ) << QgsPoint( 1, 12, 23, 24 ) << QgsPoint( 1, 2, 3, 4 ) );
  p23.clear();
  p23.setExteriorRing( lp23.clone() );
  p23.addInteriorRing( lp23.clone() );
  QCOMPARE( p23.wkbType(), QgsWkbTypes::CurvePolygonZM );
  p23.dropZValue();
  QCOMPARE( p23.wkbType(), QgsWkbTypes::CurvePolygonM );
  QCOMPARE( p23.exteriorRing()->wkbType(), QgsWkbTypes::LineStringM );
  QCOMPARE( static_cast< const QgsLineString *>( p23.exteriorRing() )->pointN( 0 ), QgsPoint( QgsWkbTypes::PointM, 1, 2, 0, 4 ) );
  QCOMPARE( p23.interiorRing( 0 )->wkbType(), QgsWkbTypes::LineStringM );
  QCOMPARE( static_cast< const QgsLineString *>( p23.interiorRing( 0 ) )->pointN( 0 ), QgsPoint( QgsWkbTypes::PointM, 1, 2, 0, 4 ) );

  // dropMValue
  p23.clear();
  p23.dropMValue();
  QCOMPARE( p23.wkbType(), QgsWkbTypes::CurvePolygon );
  lp23.setPoints( QgsPointSequence() << QgsPoint( 1, 2 ) << QgsPoint( 11, 12 ) << QgsPoint( 1, 12 ) << QgsPoint( 1, 2 ) );
  p23.setExteriorRing( lp23.clone() );
  p23.addInteriorRing( lp23.clone() );
  QCOMPARE( p23.wkbType(), QgsWkbTypes::CurvePolygon );
  p23.dropMValue(); // not zm
  QCOMPARE( p23.wkbType(), QgsWkbTypes::CurvePolygon );
  QCOMPARE( p23.exteriorRing()->wkbType(), QgsWkbTypes::LineString );
  QCOMPARE( static_cast< const QgsLineString *>( p23.exteriorRing() )->pointN( 0 ), QgsPoint( 1, 2 ) );
  QCOMPARE( p23.interiorRing( 0 )->wkbType(), QgsWkbTypes::LineString );
  QCOMPARE( static_cast< const QgsLineString *>( p23.interiorRing( 0 ) )->pointN( 0 ), QgsPoint( 1, 2 ) );
  // with m
  lp23.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointM,  1, 2, 0, 3 ) << QgsPoint( QgsWkbTypes::PointM, 11, 12, 0, 13 ) << QgsPoint( QgsWkbTypes::PointM, 1, 12, 0, 23 ) << QgsPoint( QgsWkbTypes::PointM,  1, 2, 0, 3 ) );
  p23.clear();
  p23.setExteriorRing( lp23.clone() );
  p23.addInteriorRing( lp23.clone() );
  QCOMPARE( p23.wkbType(), QgsWkbTypes::CurvePolygonM );
  p23.dropMValue();
  QCOMPARE( p23.wkbType(), QgsWkbTypes::CurvePolygon );
  QCOMPARE( p23.exteriorRing()->wkbType(), QgsWkbTypes::LineString );
  QCOMPARE( static_cast< const QgsLineString *>( p23.exteriorRing() )->pointN( 0 ), QgsPoint( 1, 2 ) );
  QCOMPARE( p23.interiorRing( 0 )->wkbType(), QgsWkbTypes::LineString );
  QCOMPARE( static_cast< const QgsLineString *>( p23.interiorRing( 0 ) )->pointN( 0 ), QgsPoint( 1, 2 ) );
  // with zm
  lp23.setPoints( QgsPointSequence() << QgsPoint( 1, 2, 3, 4 ) << QgsPoint( 11, 12, 13, 14 ) << QgsPoint( 1, 12, 23, 24 ) << QgsPoint( 1, 2, 3, 4 ) );
  p23.clear();
  p23.setExteriorRing( lp23.clone() );
  p23.addInteriorRing( lp23.clone() );
  QCOMPARE( p23.wkbType(), QgsWkbTypes::CurvePolygonZM );
  p23.dropMValue();
  QCOMPARE( p23.wkbType(), QgsWkbTypes::CurvePolygonZ );
  QCOMPARE( p23.exteriorRing()->wkbType(), QgsWkbTypes::LineStringZ );
  QCOMPARE( static_cast< const QgsLineString *>( p23.exteriorRing() )->pointN( 0 ), QgsPoint( QgsWkbTypes::PointZ, 1, 2, 3 ) );
  QCOMPARE( p23.interiorRing( 0 )->wkbType(), QgsWkbTypes::LineStringZ );
  QCOMPARE( static_cast< const QgsLineString *>( p23.interiorRing( 0 ) )->pointN( 0 ), QgsPoint( QgsWkbTypes::PointZ, 1, 2, 3 ) );


  // hasCurvedSegments
  QgsCurvePolygon p24;
  QVERIFY( !p24.hasCurvedSegments() );
  QgsLineString lp24;
  lp24.setPoints( QgsPointSequence() << QgsPoint( 1, 2 ) << QgsPoint( 11, 12 ) << QgsPoint( 1, 12 ) << QgsPoint( 1, 2 ) );
  p24.setExteriorRing( lp23.clone() );
  QVERIFY( !p24.hasCurvedSegments() );
  QgsCircularString cs24;
  cs24.setPoints( QgsPointSequence() << QgsPoint( 1, 2 ) << QgsPoint( 11, 12 ) << QgsPoint( 1, 12 ) << QgsPoint( 1, 2 ) );
  p24.addInteriorRing( cs24.clone() );
  QVERIFY( p24.hasCurvedSegments() );

  //vertexAngle
  QgsCurvePolygon p25;
  ( void )p25.vertexAngle( QgsVertexId() ); //just want no crash
  ( void )p25.vertexAngle( QgsVertexId( 0, 0, 0 ) ); //just want no crash
  ( void )p25.vertexAngle( QgsVertexId( 0, 1, 0 ) ); //just want no crash
  QgsLineString l38;
  l38.setPoints( QgsPointSequence() << QgsPoint( 0, 0 ) << QgsPoint( 0.5, 0 ) << QgsPoint( 1, 0 )
                 << QgsPoint( 2, 1 ) << QgsPoint( 1, 2 ) << QgsPoint( 0, 2 ) << QgsPoint( 0, 0 ) );
  p25.setExteriorRing( l38.clone() );
  QGSCOMPARENEAR( p25.vertexAngle( QgsVertexId( 0, 0, 0 ) ), 2.35619, 0.00001 );
  QGSCOMPARENEAR( p25.vertexAngle( QgsVertexId( 0, 0, 1 ) ), 1.5708, 0.0001 );
  QGSCOMPARENEAR( p25.vertexAngle( QgsVertexId( 0, 0, 2 ) ), 1.17809, 0.00001 );
  QGSCOMPARENEAR( p25.vertexAngle( QgsVertexId( 0, 0, 3 ) ), 0.0, 0.00001 );
  QGSCOMPARENEAR( p25.vertexAngle( QgsVertexId( 0, 0, 4 ) ), 5.10509, 0.00001 );
  QGSCOMPARENEAR( p25.vertexAngle( QgsVertexId( 0, 0, 5 ) ), 3.92699, 0.00001 );
  QGSCOMPARENEAR( p25.vertexAngle( QgsVertexId( 0, 0, 6 ) ), 2.35619, 0.00001 );
  p25.addInteriorRing( l38.clone() );
  QGSCOMPARENEAR( p25.vertexAngle( QgsVertexId( 0, 1, 0 ) ), 2.35619, 0.00001 );
  QGSCOMPARENEAR( p25.vertexAngle( QgsVertexId( 0, 1, 1 ) ), 1.5708, 0.0001 );
  QGSCOMPARENEAR( p25.vertexAngle( QgsVertexId( 0, 1, 2 ) ), 1.17809, 0.00001 );
  QGSCOMPARENEAR( p25.vertexAngle( QgsVertexId( 0, 1, 3 ) ), 0.0, 0.00001 );
  QGSCOMPARENEAR( p25.vertexAngle( QgsVertexId( 0, 1, 4 ) ), 5.10509, 0.00001 );
  QGSCOMPARENEAR( p25.vertexAngle( QgsVertexId( 0, 1, 5 ) ), 3.92699, 0.00001 );
  QGSCOMPARENEAR( p25.vertexAngle( QgsVertexId( 0, 1, 6 ) ), 2.35619, 0.00001 );


  //insert vertex

  //insert vertex in empty polygon
  p25.clear();
  QVERIFY( !p25.insertVertex( QgsVertexId( 0, 0, 0 ), QgsPoint( 6.0, 7.0 ) ) );
  QVERIFY( !p25.insertVertex( QgsVertexId( 0, 0, 1 ), QgsPoint( 6.0, 7.0 ) ) );
  QVERIFY( !p25.insertVertex( QgsVertexId( 0, 1, 0 ), QgsPoint( 6.0, 7.0 ) ) );
  QVERIFY( !p25.insertVertex( QgsVertexId( 1, 0, 0 ), QgsPoint( 6.0, 7.0 ) ) );
  QVERIFY( p25.isEmpty() );
  l38.setPoints( QgsPointSequence() << QgsPoint( 0, 0 ) << QgsPoint( 0.5, 0 ) << QgsPoint( 1, 0 )
                 << QgsPoint( 2, 1 ) << QgsPoint( 1, 2 ) << QgsPoint( 0, 2 ) << QgsPoint( 0, 0 ) );
  p25.setExteriorRing( l38.clone() );
  QVERIFY( p25.insertVertex( QgsVertexId( 0, 0, 1 ), QgsPoint( 0.3, 0 ) ) );
  QCOMPARE( p25.nCoordinates(), 8 );
  QCOMPARE( static_cast< const QgsLineString * >( p25.exteriorRing() )->pointN( 0 ), QgsPoint( 0, 0 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p25.exteriorRing() )->pointN( 1 ), QgsPoint( 0.3, 0 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p25.exteriorRing() )->pointN( 2 ), QgsPoint( 0.5, 0 ) );
  QVERIFY( !p25.insertVertex( QgsVertexId( 0, 0, -1 ), QgsPoint( 6.0, 7.0 ) ) );
  QVERIFY( !p25.insertVertex( QgsVertexId( 0, 0, 100 ), QgsPoint( 6.0, 7.0 ) ) );
  QVERIFY( !p25.insertVertex( QgsVertexId( 0, 1, 0 ), QgsPoint( 6.0, 7.0 ) ) );
  // first vertex
  QVERIFY( p25.insertVertex( QgsVertexId( 0, 0, 0 ), QgsPoint( 0, 0.1 ) ) );
  QCOMPARE( p25.nCoordinates(), 9 );
  QCOMPARE( static_cast< const QgsLineString * >( p25.exteriorRing() )->pointN( 0 ), QgsPoint( 0, 0.1 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p25.exteriorRing() )->pointN( 1 ), QgsPoint( 0, 0 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p25.exteriorRing() )->pointN( 2 ), QgsPoint( 0.3, 0 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p25.exteriorRing() )->pointN( 3 ), QgsPoint( 0.5, 0 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p25.exteriorRing() )->pointN( 7 ), QgsPoint( 0, 2 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p25.exteriorRing() )->pointN( 8 ), QgsPoint( 0, 0.1 ) );
  // last vertex
  QVERIFY( p25.insertVertex( QgsVertexId( 0, 0, 9 ), QgsPoint( 0.1, 0.1 ) ) );
  QCOMPARE( p25.nCoordinates(), 10 );
  QCOMPARE( static_cast< const QgsLineString * >( p25.exteriorRing() )->pointN( 0 ), QgsPoint( 0.1, 0.1 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p25.exteriorRing() )->pointN( 1 ), QgsPoint( 0, 0 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p25.exteriorRing() )->pointN( 2 ), QgsPoint( 0.3, 0 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p25.exteriorRing() )->pointN( 3 ), QgsPoint( 0.5, 0 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p25.exteriorRing() )->pointN( 8 ), QgsPoint( 0, 0.1 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p25.exteriorRing() )->pointN( 9 ), QgsPoint( 0.1, 0.1 ) );
  // with interior ring
  p25.addInteriorRing( l38.clone() );
  QCOMPARE( p25.nCoordinates(), 17 );
  QVERIFY( p25.insertVertex( QgsVertexId( 0, 1, 1 ), QgsPoint( 0.3, 0 ) ) );
  QCOMPARE( p25.nCoordinates(), 18 );
  QCOMPARE( static_cast< const QgsLineString * >( p25.interiorRing( 0 ) )->pointN( 0 ), QgsPoint( 0, 0 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p25.interiorRing( 0 ) )->pointN( 1 ), QgsPoint( 0.3, 0 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p25.interiorRing( 0 ) )->pointN( 2 ), QgsPoint( 0.5, 0 ) );
  QVERIFY( !p25.insertVertex( QgsVertexId( 0, 1, -1 ), QgsPoint( 6.0, 7.0 ) ) );
  QVERIFY( !p25.insertVertex( QgsVertexId( 0, 1, 100 ), QgsPoint( 6.0, 7.0 ) ) );
  QVERIFY( !p25.insertVertex( QgsVertexId( 0, 2, 0 ), QgsPoint( 6.0, 7.0 ) ) );
  // first vertex in interior ring
  QVERIFY( p25.insertVertex( QgsVertexId( 0, 1, 0 ), QgsPoint( 0, 0.1 ) ) );
  QCOMPARE( p25.nCoordinates(), 19 );
  QCOMPARE( static_cast< const QgsLineString * >( p25.interiorRing( 0 ) )->pointN( 0 ), QgsPoint( 0, 0.1 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p25.interiorRing( 0 ) )->pointN( 1 ), QgsPoint( 0, 0 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p25.interiorRing( 0 ) )->pointN( 2 ), QgsPoint( 0.3, 0 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p25.interiorRing( 0 ) )->pointN( 3 ), QgsPoint( 0.5, 0 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p25.interiorRing( 0 ) )->pointN( 7 ), QgsPoint( 0, 2 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p25.interiorRing( 0 ) )->pointN( 8 ), QgsPoint( 0, 0.1 ) );
  // last vertex in interior ring
  QVERIFY( p25.insertVertex( QgsVertexId( 0, 1, 9 ), QgsPoint( 0.1, 0.1 ) ) );
  QCOMPARE( p25.nCoordinates(), 20 );
  QCOMPARE( static_cast< const QgsLineString * >( p25.interiorRing( 0 ) )->pointN( 0 ), QgsPoint( 0.1, 0.1 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p25.interiorRing( 0 ) )->pointN( 1 ), QgsPoint( 0, 0 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p25.interiorRing( 0 ) )->pointN( 2 ), QgsPoint( 0.3, 0 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p25.interiorRing( 0 ) )->pointN( 3 ), QgsPoint( 0.5, 0 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p25.interiorRing( 0 ) )->pointN( 8 ), QgsPoint( 0, 0.1 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p25.interiorRing( 0 ) )->pointN( 9 ), QgsPoint( 0.1, 0.1 ) );

  //move vertex

  //empty polygon
  QgsCurvePolygon p26;
  QVERIFY( !p26.moveVertex( QgsVertexId( 0, 0, 0 ), QgsPoint( 6.0, 7.0 ) ) );
  QVERIFY( p26.isEmpty() );

  //valid polygon
  l38.setPoints( QgsPointSequence() << QgsPoint( 1, 2 )
                 << QgsPoint( 11, 12 ) << QgsPoint( 21, 22 ) << QgsPoint( 1, 2 ) );
  p26.setExteriorRing( l38.clone() );
  QVERIFY( p26.moveVertex( QgsVertexId( 0, 0, 0 ), QgsPoint( 6.0, 7.0 ) ) );
  QVERIFY( p26.moveVertex( QgsVertexId( 0, 0, 1 ), QgsPoint( 16.0, 17.0 ) ) );
  QVERIFY( p26.moveVertex( QgsVertexId( 0, 0, 2 ), QgsPoint( 26.0, 27.0 ) ) );
  QCOMPARE( static_cast< const QgsLineString * >( p26.exteriorRing() )->pointN( 0 ), QgsPoint( 6.0, 7.0 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p26.exteriorRing() )->pointN( 1 ), QgsPoint( 16.0, 17.0 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p26.exteriorRing() )->pointN( 2 ), QgsPoint( 26.0, 27.0 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p26.exteriorRing() )->pointN( 3 ), QgsPoint( 6.0, 7.0 ) );

  //out of range
  QVERIFY( !p26.moveVertex( QgsVertexId( 0, 0, -1 ), QgsPoint( 3.0, 4.0 ) ) );
  QVERIFY( !p26.moveVertex( QgsVertexId( 0, 0, 10 ), QgsPoint( 3.0, 4.0 ) ) );
  QVERIFY( !p26.moveVertex( QgsVertexId( 0, 1, 0 ), QgsPoint( 3.0, 4.0 ) ) );
  QCOMPARE( static_cast< const QgsLineString * >( p26.exteriorRing() )->pointN( 0 ), QgsPoint( 6.0, 7.0 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p26.exteriorRing() )->pointN( 1 ), QgsPoint( 16.0, 17.0 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p26.exteriorRing() )->pointN( 2 ), QgsPoint( 26.0, 27.0 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p26.exteriorRing() )->pointN( 3 ), QgsPoint( 6.0, 7.0 ) );

  // with interior ring
  p26.addInteriorRing( l38.clone() );
  QVERIFY( p26.moveVertex( QgsVertexId( 0, 1, 0 ), QgsPoint( 6.0, 7.0 ) ) );
  QVERIFY( p26.moveVertex( QgsVertexId( 0, 1, 1 ), QgsPoint( 16.0, 17.0 ) ) );
  QVERIFY( p26.moveVertex( QgsVertexId( 0, 1, 2 ), QgsPoint( 26.0, 27.0 ) ) );
  QCOMPARE( static_cast< const QgsLineString * >( p26.interiorRing( 0 ) )->pointN( 0 ), QgsPoint( 6.0, 7.0 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p26.interiorRing( 0 ) )->pointN( 1 ), QgsPoint( 16.0, 17.0 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p26.interiorRing( 0 ) )->pointN( 2 ), QgsPoint( 26.0, 27.0 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p26.interiorRing( 0 ) )->pointN( 3 ), QgsPoint( 6.0, 7.0 ) );
  QVERIFY( !p26.moveVertex( QgsVertexId( 0, 1, -1 ), QgsPoint( 3.0, 4.0 ) ) );
  QVERIFY( !p26.moveVertex( QgsVertexId( 0, 1, 10 ), QgsPoint( 3.0, 4.0 ) ) );
  QVERIFY( !p26.moveVertex( QgsVertexId( 0, 2, 0 ), QgsPoint( 3.0, 4.0 ) ) );

  //delete vertex

  //empty polygon
  QgsCurvePolygon p27;
  QVERIFY( !p27.deleteVertex( QgsVertexId( 0, 0, 0 ) ) );
  QVERIFY( !p27.deleteVertex( QgsVertexId( 0, 1, 0 ) ) );
  QVERIFY( p27.isEmpty() );

  //valid polygon
  l38.setPoints( QgsPointSequence() << QgsPoint( 1, 2 ) << QgsPoint( 5, 2 ) << QgsPoint( 6, 2 ) << QgsPoint( 7, 2 )
                 << QgsPoint( 11, 12 ) << QgsPoint( 21, 22 ) << QgsPoint( 1, 2 ) );

  p27.setExteriorRing( l38.clone() );
  //out of range vertices
  QVERIFY( !p27.deleteVertex( QgsVertexId( 0, 0, -1 ) ) );
  QVERIFY( !p27.deleteVertex( QgsVertexId( 0, 0, 100 ) ) );
  QVERIFY( !p27.deleteVertex( QgsVertexId( 0, 1, 1 ) ) );

  //valid vertices
  QVERIFY( p27.deleteVertex( QgsVertexId( 0, 0, 1 ) ) );
  QCOMPARE( static_cast< const QgsLineString * >( p27.exteriorRing() )->pointN( 0 ), QgsPoint( 1.0, 2.0 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p27.exteriorRing() )->pointN( 1 ), QgsPoint( 6.0, 2.0 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p27.exteriorRing() )->pointN( 2 ), QgsPoint( 7.0, 2.0 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p27.exteriorRing() )->pointN( 3 ), QgsPoint( 11.0, 12.0 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p27.exteriorRing() )->pointN( 5 ), QgsPoint( 1.0, 2.0 ) );

  // delete first vertex
  QVERIFY( p27.deleteVertex( QgsVertexId( 0, 0, 0 ) ) );
  QCOMPARE( static_cast< const QgsLineString * >( p27.exteriorRing() )->pointN( 0 ), QgsPoint( 6.0, 2.0 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p27.exteriorRing() )->pointN( 1 ), QgsPoint( 7.0, 2.0 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p27.exteriorRing() )->pointN( 2 ), QgsPoint( 11.0, 12.0 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p27.exteriorRing() )->pointN( 3 ), QgsPoint( 21.0, 22.0 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p27.exteriorRing() )->pointN( 4 ), QgsPoint( 6.0, 2.0 ) );

  // delete last vertex
  QVERIFY( p27.deleteVertex( QgsVertexId( 0, 0, 4 ) ) );
  QCOMPARE( static_cast< const QgsLineString * >( p27.exteriorRing() )->pointN( 0 ), QgsPoint( 21.0, 22.0 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p27.exteriorRing() )->pointN( 1 ), QgsPoint( 7.0, 2.0 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p27.exteriorRing() )->pointN( 2 ), QgsPoint( 11.0, 12.0 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p27.exteriorRing() )->pointN( 3 ), QgsPoint( 21.0, 22.0 ) );

  // delete another vertex - should remove ring
  QVERIFY( p27.deleteVertex( QgsVertexId( 0, 0, 1 ) ) );
  QVERIFY( !p27.exteriorRing() );

  // with interior ring
  p27.setExteriorRing( l38.clone() );
  p27.addInteriorRing( l38.clone() );

  //out of range vertices
  QVERIFY( !p27.deleteVertex( QgsVertexId( 0, 1, -1 ) ) );
  QVERIFY( !p27.deleteVertex( QgsVertexId( 0, 1, 100 ) ) );
  QVERIFY( !p27.deleteVertex( QgsVertexId( 0, 2, 1 ) ) );

  //valid vertices
  QVERIFY( p27.deleteVertex( QgsVertexId( 0, 1, 1 ) ) );
  QCOMPARE( static_cast< const QgsLineString * >( p27.interiorRing( 0 ) )->pointN( 0 ), QgsPoint( 1.0, 2.0 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p27.interiorRing( 0 ) )->pointN( 1 ), QgsPoint( 6.0, 2.0 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p27.interiorRing( 0 ) )->pointN( 2 ), QgsPoint( 7.0, 2.0 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p27.interiorRing( 0 ) )->pointN( 3 ), QgsPoint( 11.0, 12.0 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p27.interiorRing( 0 ) )->pointN( 5 ), QgsPoint( 1.0, 2.0 ) );

  // delete first vertex
  QVERIFY( p27.deleteVertex( QgsVertexId( 0, 1, 0 ) ) );
  QCOMPARE( static_cast< const QgsLineString * >( p27.interiorRing( 0 ) )->pointN( 0 ), QgsPoint( 6.0, 2.0 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p27.interiorRing( 0 ) )->pointN( 1 ), QgsPoint( 7.0, 2.0 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p27.interiorRing( 0 ) )->pointN( 2 ), QgsPoint( 11.0, 12.0 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p27.interiorRing( 0 ) )->pointN( 3 ), QgsPoint( 21.0, 22.0 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p27.interiorRing( 0 ) )->pointN( 4 ), QgsPoint( 6.0, 2.0 ) );

  // delete last vertex
  QVERIFY( p27.deleteVertex( QgsVertexId( 0, 1, 4 ) ) );
  QCOMPARE( static_cast< const QgsLineString * >( p27.interiorRing( 0 ) )->pointN( 0 ), QgsPoint( 21.0, 22.0 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p27.interiorRing( 0 ) )->pointN( 1 ), QgsPoint( 7.0, 2.0 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p27.interiorRing( 0 ) )->pointN( 2 ), QgsPoint( 11.0, 12.0 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p27.interiorRing( 0 ) )->pointN( 3 ), QgsPoint( 21.0, 22.0 ) );

  // delete another vertex - should remove ring
  QVERIFY( p27.deleteVertex( QgsVertexId( 0, 1, 1 ) ) );
  QCOMPARE( p27.numInteriorRings(), 0 );
  QVERIFY( p27.exteriorRing() );

  // test that interior ring is "promoted" when exterior is removed
  p27.addInteriorRing( l38.clone() );
  QVERIFY( p27.deleteVertex( QgsVertexId( 0, 0, 0 ) ) );
  QCOMPARE( p27.numInteriorRings(), 1 );
  QVERIFY( p27.deleteVertex( QgsVertexId( 0, 0, 0 ) ) );
  QCOMPARE( p27.numInteriorRings(), 1 );
  QVERIFY( p27.deleteVertex( QgsVertexId( 0, 0, 0 ) ) );
  QCOMPARE( p27.numInteriorRings(), 1 );
  QVERIFY( p27.deleteVertex( QgsVertexId( 0, 0, 0 ) ) );
  QCOMPARE( p27.numInteriorRings(), 0 );
  QVERIFY( p27.exteriorRing() );
}

void TestQgsGeometry::compoundCurve()
{
  //test constructors
  QgsCompoundCurve l1;
  QVERIFY( l1.isEmpty() );
  QCOMPARE( l1.numPoints(), 0 );
  QCOMPARE( l1.vertexCount(), 0 );
  QCOMPARE( l1.nCoordinates(), 0 );
  QCOMPARE( l1.ringCount(), 0 );
  QCOMPARE( l1.partCount(), 0 );
  QVERIFY( !l1.is3D() );
  QVERIFY( !l1.isMeasure() );
  QCOMPARE( l1.wkbType(), QgsWkbTypes::CompoundCurve );
  QCOMPARE( l1.wktTypeStr(), QString( "CompoundCurve" ) );
  QCOMPARE( l1.geometryType(), QString( "CompoundCurve" ) );
  QCOMPARE( l1.dimension(), 1 );
  QVERIFY( !l1.hasCurvedSegments() );
  QCOMPARE( l1.area(), 0.0 );
  QCOMPARE( l1.perimeter(), 0.0 );
  QgsPointSequence pts;
  l1.points( pts );
  QVERIFY( pts.empty() );

  // empty, test some methods to make sure they don't crash
  QCOMPARE( l1.nCurves(), 0 );
  QVERIFY( !l1.curveAt( -1 ) );
  QVERIFY( !l1.curveAt( 0 ) );
  QVERIFY( !l1.curveAt( 100 ) );
  l1.removeCurve( -1 );
  l1.removeCurve( 0 );
  l1.removeCurve( 100 );

  //addCurve
  QgsCompoundCurve c1;
  //try to add null curve
  c1.addCurve( nullptr );
  QCOMPARE( c1.nCurves(), 0 );
  QVERIFY( !c1.curveAt( 0 ) );

  QgsCircularString l2;
  l2.setPoints( QgsPointSequence() << QgsPoint( 1.0, 2.0 ) );
  c1.addCurve( l2.clone() );
  QVERIFY( !c1.isEmpty() );
  QCOMPARE( c1.numPoints(), 1 );
  QCOMPARE( c1.vertexCount(), 1 );
  QCOMPARE( c1.nCoordinates(), 1 );
  QCOMPARE( c1.ringCount(), 1 );
  QCOMPARE( c1.partCount(), 1 );
  QVERIFY( !c1.is3D() );
  QVERIFY( !c1.isMeasure() );
  QCOMPARE( c1.wkbType(), QgsWkbTypes::CompoundCurve );
  QVERIFY( c1.hasCurvedSegments() );
  QCOMPARE( c1.area(), 0.0 );
  QCOMPARE( c1.perimeter(), 0.0 );
  c1.points( pts );
  QCOMPARE( pts, QgsPointSequence() << QgsPoint( 1.0, 2.0 ) );

  //adding first curve should set linestring z/m type
  QgsCircularString l3;
  l3.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZ, 1.0, 2.0, 3.0 ) );
  QgsCompoundCurve c2;
  c2.addCurve( l3.clone() );
  QVERIFY( !c2.isEmpty() );
  QVERIFY( c2.is3D() );
  QVERIFY( !c2.isMeasure() );
  QCOMPARE( c2.wkbType(), QgsWkbTypes::CompoundCurveZ );
  QCOMPARE( c2.wktTypeStr(), QString( "CompoundCurveZ" ) );
  c2.points( pts );
  QCOMPARE( pts, QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZ, 1.0, 2.0, 3.0 ) );

  QgsCircularString l4;
  l4.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointM, 1.0, 2.0, 0.0, 3.0 ) );
  QgsCompoundCurve c4;
  c4.addCurve( l4.clone() );
  QVERIFY( !c4.isEmpty() );
  QVERIFY( !c4.is3D() );
  QVERIFY( c4.isMeasure() );
  QCOMPARE( c4.wkbType(), QgsWkbTypes::CompoundCurveM );
  QCOMPARE( c4.wktTypeStr(), QString( "CompoundCurveM" ) );
  c4.points( pts );
  QCOMPARE( pts, QgsPointSequence() << QgsPoint( QgsWkbTypes::PointM, 1.0, 2.0, 0.0, 3.0 ) );

  QgsCircularString l5;
  l5.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 1.0, 2.0, 3.0, 4.0 ) );
  QgsCompoundCurve c5;
  c5.addCurve( l5.clone() );
  QVERIFY( !c5.isEmpty() );
  QVERIFY( c5.is3D() );
  QVERIFY( c5.isMeasure() );
  QCOMPARE( c5.wkbType(), QgsWkbTypes::CompoundCurveZM );
  QCOMPARE( c5.wktTypeStr(), QString( "CompoundCurveZM" ) );
  c5.points( pts );
  QCOMPARE( pts, QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 1.0, 2.0, 3.0, 4.0 ) );

  //clear
  c5.clear();
  QVERIFY( c5.isEmpty() );
  QCOMPARE( c5.nCurves(), 0 );
  QCOMPARE( c5.numPoints(), 0 );
  QCOMPARE( c5.vertexCount(), 0 );
  QCOMPARE( c5.nCoordinates(), 0 );
  QCOMPARE( c5.ringCount(), 0 );
  QCOMPARE( c5.partCount(), 0 );
  QVERIFY( !c5.is3D() );
  QVERIFY( !c5.isMeasure() );
  QCOMPARE( c5.wkbType(), QgsWkbTypes::CompoundCurve );

  //addCurve
  QgsCircularString l8;
  QgsCompoundCurve c8;
  l8.setPoints( QgsPointSequence() << QgsPoint( 1, 2 ) << QgsPoint( 2, 3 ) << QgsPoint( 3, 4 ) );
  c8.addCurve( l8.clone() );
  QVERIFY( !c8.isEmpty() );
  QCOMPARE( c8.numPoints(), 3 );
  QCOMPARE( c8.vertexCount(), 3 );
  QCOMPARE( c8.nCoordinates(), 3 );
  QCOMPARE( c8.ringCount(), 1 );
  QCOMPARE( c8.partCount(), 1 );
  QCOMPARE( c8.nCurves(), 1 );
  QVERIFY( !c8.is3D() );
  QVERIFY( !c8.isMeasure() );
  QCOMPARE( c8.wkbType(), QgsWkbTypes::CompoundCurve );
  QVERIFY( c8.hasCurvedSegments() );
  c8.points( pts );
  QCOMPARE( pts, QgsPointSequence() << QgsPoint( 1, 2 ) << QgsPoint( 2, 3 ) << QgsPoint( 3, 4 ) );
  QCOMPARE( *dynamic_cast< const QgsCircularString *>( c8.curveAt( 0 ) ), l8 );
  QVERIFY( ! c8.curveAt( -1 ) );
  QVERIFY( ! c8.curveAt( 1 ) );

  QgsCircularString l8a;
  l8a.setPoints( QgsPointSequence() << QgsPoint( 3, 4 ) << QgsPoint( 4, 5 ) << QgsPoint( 3, 6 ) );
  c8.addCurve( l8a.clone() );
  QCOMPARE( c8.numPoints(), 5 );
  QCOMPARE( c8.vertexCount(), 5 );
  QCOMPARE( c8.nCoordinates(), 5 );
  QCOMPARE( c8.ringCount(), 1 );
  QCOMPARE( c8.partCount(), 1 );
  QCOMPARE( c8.nCurves(), 2 );
  pts.clear();
  c8.points( pts );
  QCOMPARE( pts, QgsPointSequence() << QgsPoint( 1, 2 ) << QgsPoint( 2, 3 ) << QgsPoint( 3, 4 )
            << QgsPoint( 4, 5 ) << QgsPoint( 3, 6 ) );
  QCOMPARE( *dynamic_cast< const QgsCircularString *>( c8.curveAt( 0 ) ), l8 );
  QCOMPARE( *dynamic_cast< const QgsCircularString *>( c8.curveAt( 1 ) ), l8a );
  QVERIFY( ! c8.curveAt( -1 ) );
  QVERIFY( ! c8.curveAt( 2 ) );

  QgsLineString l8b;
  l8b.setPoints( QgsPointSequence() << QgsPoint( 3, 6 ) << QgsPoint( 4, 6 ) );
  c8.addCurve( l8b.clone() );
  QCOMPARE( c8.numPoints(), 6 );
  QCOMPARE( c8.vertexCount(), 6 );
  QCOMPARE( c8.nCoordinates(), 6 );
  QCOMPARE( c8.ringCount(), 1 );
  QCOMPARE( c8.partCount(), 1 );
  QCOMPARE( c8.nCurves(), 3 );
  pts.clear();
  c8.points( pts );
  QCOMPARE( pts, QgsPointSequence() << QgsPoint( 1, 2 ) << QgsPoint( 2, 3 ) << QgsPoint( 3, 4 )
            << QgsPoint( 4, 5 ) << QgsPoint( 3, 6 )
            << QgsPoint( 4, 6 ) );
  QCOMPARE( *dynamic_cast< const QgsCircularString *>( c8.curveAt( 0 ) ), l8 );
  QCOMPARE( *dynamic_cast< const QgsCircularString *>( c8.curveAt( 1 ) ), l8a );
  QCOMPARE( *dynamic_cast< const QgsLineString *>( c8.curveAt( 2 ) ), l8b );
  QVERIFY( ! c8.curveAt( -1 ) );
  QVERIFY( ! c8.curveAt( 3 ) );

  //removeCurve
  c8.removeCurve( -1 );
  c8.removeCurve( 3 );
  QCOMPARE( c8.nCurves(), 3 );
  QCOMPARE( *dynamic_cast< const QgsCircularString *>( c8.curveAt( 0 ) ), l8 );
  QCOMPARE( *dynamic_cast< const QgsCircularString *>( c8.curveAt( 1 ) ), l8a );
  QCOMPARE( *dynamic_cast< const QgsLineString *>( c8.curveAt( 2 ) ), l8b );
  c8.removeCurve( 1 );
  QCOMPARE( c8.nCurves(), 2 );
  QCOMPARE( *dynamic_cast< const QgsCircularString *>( c8.curveAt( 0 ) ), l8 );
  QCOMPARE( *dynamic_cast< const QgsLineString *>( c8.curveAt( 1 ) ), l8b );
  c8.removeCurve( 0 );
  QCOMPARE( c8.nCurves(), 1 );
  QCOMPARE( *dynamic_cast< const QgsLineString *>( c8.curveAt( 0 ) ), l8b );
  c8.removeCurve( 0 );
  QCOMPARE( c8.nCurves(), 0 );
  QVERIFY( c8.isEmpty() );

  //addCurve with z
  c8.clear();
  l8.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZ, 1, 2, 3 ) << QgsPoint( QgsWkbTypes::PointZ, 2, 3, 4 ) );
  c8.addCurve( l8.clone() );
  QCOMPARE( c8.numPoints(), 2 );
  QVERIFY( c8.is3D() );
  QVERIFY( !c8.isMeasure() );
  QCOMPARE( c8.wkbType(), QgsWkbTypes::CompoundCurveZ );
  pts.clear();
  c8.points( pts );
  QCOMPARE( pts, QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZ, 1, 2, 3 ) << QgsPoint( QgsWkbTypes::PointZ, 2, 3, 4 ) );

  //addCurve with m
  c8.clear();
  l8.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointM, 1, 2, 0, 3 ) << QgsPoint( QgsWkbTypes::PointM, 2, 3, 0, 4 ) );
  c8.addCurve( l8.clone() );
  QCOMPARE( c8.numPoints(), 2 );
  QVERIFY( !c8.is3D() );
  QVERIFY( c8.isMeasure() );
  QCOMPARE( c8.wkbType(), QgsWkbTypes::CompoundCurveM );
  c8.points( pts );
  QCOMPARE( pts, QgsPointSequence() << QgsPoint( QgsWkbTypes::PointM, 1, 2, 0, 3 ) << QgsPoint( QgsWkbTypes::PointM, 2, 3, 0, 4 ) );

  //addCurve with zm
  c8.clear();
  l8.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 1, 2, 4, 5 ) << QgsPoint( QgsWkbTypes::PointZM, 2, 3, 4, 5 ) );
  c8.addCurve( l8.clone() );
  QCOMPARE( c8.numPoints(), 2 );
  QVERIFY( c8.is3D() );
  QVERIFY( c8.isMeasure() );
  QCOMPARE( c8.wkbType(), QgsWkbTypes::CompoundCurveZM );
  c8.points( pts );
  QCOMPARE( pts, QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 1, 2, 4, 5 ) << QgsPoint( QgsWkbTypes::PointZM, 2, 3, 4, 5 ) );

  //addCurve with z to non z compound curve
  c8.clear();
  l8.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::Point, 1, 2 ) << QgsPoint( QgsWkbTypes::Point, 2, 3 ) );
  c8.addCurve( l8.clone() );
  QCOMPARE( c8.wkbType(), QgsWkbTypes::CompoundCurve );
  l8.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZ, 2, 3, 4 ) << QgsPoint( QgsWkbTypes::PointZ, 3, 3, 5 ) );
  c8.addCurve( l8.clone() );
  QVERIFY( !c8.is3D() );
  QVERIFY( !c8.isMeasure() );
  QCOMPARE( c8.wkbType(), QgsWkbTypes::CompoundCurve );
  c8.points( pts );
  QCOMPARE( pts, QgsPointSequence() << QgsPoint( QgsWkbTypes::Point, 1, 2 ) << QgsPoint( QgsWkbTypes::Point, 2, 3 )
            << QgsPoint( QgsWkbTypes::Point, 3, 3 ) );
  c8.removeCurve( 1 );

  //addCurve with m to non m compound curve
  l8.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointM, 2, 3, 0, 4 ) << QgsPoint( QgsWkbTypes::PointM, 3, 3, 0, 5 ) );
  c8.addCurve( l8.clone() );
  QVERIFY( !c8.is3D() );
  QVERIFY( !c8.isMeasure() );
  QCOMPARE( c8.wkbType(), QgsWkbTypes::CompoundCurve );
  c8.points( pts );
  QCOMPARE( pts, QgsPointSequence() << QgsPoint( QgsWkbTypes::Point, 1, 2 ) << QgsPoint( QgsWkbTypes::Point, 2, 3 )
            << QgsPoint( QgsWkbTypes::Point, 3, 3 ) );
  c8.removeCurve( 1 );

  //addCurve with zm to non m compound curve
  l8.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 2, 3, 6, 4 ) << QgsPoint( QgsWkbTypes::PointZM, 3, 3, 1, 5 ) );
  c8.addCurve( l8.clone() );
  QVERIFY( !c8.is3D() );
  QVERIFY( !c8.isMeasure() );
  QCOMPARE( c8.wkbType(), QgsWkbTypes::CompoundCurve );
  c8.points( pts );
  QCOMPARE( pts, QgsPointSequence() << QgsPoint( QgsWkbTypes::Point, 1, 2 ) << QgsPoint( QgsWkbTypes::Point, 2, 3 )
            << QgsPoint( QgsWkbTypes::Point, 3, 3 ) );
  c8.removeCurve( 1 );

  //addCurve with no z to z compound curve
  c8.clear();
  l8.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZ, 1, 2, 4 ) << QgsPoint( QgsWkbTypes::PointZ, 2, 3, 5 ) );
  c8.addCurve( l8.clone() );
  QCOMPARE( c8.wkbType(), QgsWkbTypes::CompoundCurveZ );
  l8.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::Point, 2, 3 ) << QgsPoint( QgsWkbTypes::Point, 3, 4 ) );
  c8.addCurve( l8.clone() );
  QVERIFY( c8.is3D() );
  QVERIFY( !c8.isMeasure() );
  QCOMPARE( c8.wkbType(), QgsWkbTypes::CompoundCurveZ );
  c8.points( pts );
  QCOMPARE( pts, QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZ, 1, 2, 4 ) << QgsPoint( QgsWkbTypes::PointZ, 2, 3, 5 )
            << QgsPoint( QgsWkbTypes::PointZ, 3, 4, 0 ) );
  c8.removeCurve( 1 );

  //add curve with m, no z to z compound curve
  l8.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointM, 2, 3, 0, 8 ) << QgsPoint( QgsWkbTypes::PointM, 3, 4, 0, 9 ) );
  c8.addCurve( l8.clone() );
  QVERIFY( c8.is3D() );
  QVERIFY( !c8.isMeasure() );
  QCOMPARE( c8.wkbType(), QgsWkbTypes::CompoundCurveZ );
  c8.points( pts );
  QCOMPARE( pts, QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZ, 1, 2, 4 ) << QgsPoint( QgsWkbTypes::PointZ, 2, 3, 5 )
            << QgsPoint( QgsWkbTypes::PointZ, 3, 4, 0 ) );
  c8.removeCurve( 1 );

  //add curve with zm to z compound curve
  l8.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 2, 3, 6, 8 ) << QgsPoint( QgsWkbTypes::PointZM, 3, 4, 7, 9 ) );
  c8.addCurve( l8.clone() );
  QVERIFY( c8.is3D() );
  QVERIFY( !c8.isMeasure() );
  QCOMPARE( c8.wkbType(), QgsWkbTypes::CompoundCurveZ );
  c8.points( pts );
  QCOMPARE( pts, QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZ, 1, 2, 4 ) << QgsPoint( QgsWkbTypes::PointZ, 2, 3, 5 )
            << QgsPoint( QgsWkbTypes::PointZ, 3, 4, 7 ) );
  c8.removeCurve( 1 );

  //addCurve with no m to m compound curve
  c8.clear();
  l8.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointM, 1, 2, 0, 4 ) << QgsPoint( QgsWkbTypes::PointM, 2, 3, 0, 5 ) );
  c8.addCurve( l8.clone() );
  QCOMPARE( c8.wkbType(), QgsWkbTypes::CompoundCurveM );
  l8.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::Point, 2, 3 ) << QgsPoint( QgsWkbTypes::Point, 3, 4 ) );
  c8.addCurve( l8.clone() );
  QVERIFY( !c8.is3D() );
  QVERIFY( c8.isMeasure() );
  QCOMPARE( c8.wkbType(), QgsWkbTypes::CompoundCurveM );
  c8.points( pts );
  QCOMPARE( pts, QgsPointSequence() << QgsPoint( QgsWkbTypes::PointM, 1, 2, 0, 4 ) << QgsPoint( QgsWkbTypes::PointM, 2, 3, 0, 5 )
            << QgsPoint( QgsWkbTypes::PointM, 3, 4, 0, 0 ) );
  c8.removeCurve( 1 );

  //add curve with z, no m to m compound curve
  l8.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZ, 2, 3, 8 ) << QgsPoint( QgsWkbTypes::PointZ, 3, 4, 9 ) );
  c8.addCurve( l8.clone() );
  QVERIFY( !c8.is3D() );
  QVERIFY( c8.isMeasure() );
  QCOMPARE( c8.wkbType(), QgsWkbTypes::CompoundCurveM );
  c8.points( pts );
  QCOMPARE( pts, QgsPointSequence() << QgsPoint( QgsWkbTypes::PointM, 1, 2, 0, 4 ) << QgsPoint( QgsWkbTypes::PointM, 2, 3, 0, 5 )
            << QgsPoint( QgsWkbTypes::PointM, 3, 4, 0, 0 ) );
  c8.removeCurve( 1 );

  //add curve with zm to m compound curve
  l8.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 2, 3, 6, 8 ) << QgsPoint( QgsWkbTypes::PointZM, 3, 4, 7, 9 ) );
  c8.addCurve( l8.clone() );
  QVERIFY( !c8.is3D() );
  QVERIFY( c8.isMeasure() );
  QCOMPARE( c8.wkbType(), QgsWkbTypes::CompoundCurveM );
  c8.points( pts );
  QCOMPARE( pts, QgsPointSequence() << QgsPoint( QgsWkbTypes::PointM, 1, 2, 0, 4 ) << QgsPoint( QgsWkbTypes::PointM, 2, 3, 0, 5 )
            << QgsPoint( QgsWkbTypes::PointM, 3, 4, 0, 9 ) );
  c8.removeCurve( 1 );

  //test getters/setters
  QgsCompoundCurve c9;

  // no crash!
  ( void )c9.xAt( -1 );
  ( void )c9.xAt( 1 );
  ( void )c9.yAt( -1 );
  ( void )c9.yAt( 1 );

  QgsCircularString l9;
  l9.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 1, 2, 3, 4 )
                << QgsPoint( QgsWkbTypes::PointZM, 11, 12, 13, 14 )
                << QgsPoint( QgsWkbTypes::PointZM, 21, 22, 23, 24 ) );
  c9.addCurve( l9.clone() );
  QCOMPARE( c9.xAt( 0 ), 1.0 );
  QCOMPARE( c9.xAt( 1 ), 11.0 );
  QCOMPARE( c9.xAt( 2 ), 21.0 );
  ( void ) c9.xAt( -1 ); //out of range
  ( void ) c9.xAt( 11 ); //out of range
  QCOMPARE( c9.yAt( 0 ), 2.0 );
  QCOMPARE( c9.yAt( 1 ), 12.0 );
  QCOMPARE( c9.yAt( 2 ), 22.0 );
  ( void ) c9.yAt( -1 ); //out of range
  ( void ) c9.yAt( 11 ); //out of range

  QgsLineString l9a;
  l9a.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 21, 22, 23, 24 )
                 << QgsPoint( QgsWkbTypes::PointZM, 31, 22, 13, 14 ) );
  c9.addCurve( l9a.clone() );
  QCOMPARE( c9.xAt( 0 ), 1.0 );
  QCOMPARE( c9.xAt( 1 ), 11.0 );
  QCOMPARE( c9.xAt( 2 ), 21.0 );
  QCOMPARE( c9.xAt( 3 ), 31.0 );
  QCOMPARE( c9.xAt( 4 ), 0.0 );
  ( void ) c9.xAt( -1 ); //out of range
  ( void ) c9.xAt( 11 ); //out of range
  QCOMPARE( c9.yAt( 0 ), 2.0 );
  QCOMPARE( c9.yAt( 1 ), 12.0 );
  QCOMPARE( c9.yAt( 2 ), 22.0 );
  QCOMPARE( c9.yAt( 3 ), 22.0 );
  QCOMPARE( c9.yAt( 4 ), 0.0 );
  ( void ) c9.yAt( -1 ); //out of range
  ( void ) c9.yAt( 11 ); //out of range

  c9.moveVertex( QgsVertexId( 0, 0, 0 ), QgsPoint( 51.0, 52.0 ) );
  QCOMPARE( c9.xAt( 0 ), 51.0 );
  QCOMPARE( c9.yAt( 0 ), 52.0 );
  c9.moveVertex( QgsVertexId( 0, 0, 1 ), QgsPoint( 61.0, 62 ) );
  QCOMPARE( c9.xAt( 1 ), 61.0 );
  QCOMPARE( c9.yAt( 1 ), 62.0 );
  c9.moveVertex( QgsVertexId( 0, 0, -1 ), QgsPoint( 71.0, 2 ) ); //out of range
  c9.moveVertex( QgsVertexId( 0, 0, 11 ), QgsPoint( 71.0, 2 ) ); //out of range

  QgsPoint p;
  QgsVertexId::VertexType type;
  QVERIFY( !c9.pointAt( -1, p, type ) );
  QVERIFY( !c9.pointAt( 11, p, type ) );
  QVERIFY( c9.pointAt( 0, p, type ) );
  QCOMPARE( p.z(), 3.0 );
  QCOMPARE( p.m(), 4.0 );
  QCOMPARE( type, QgsVertexId::SegmentVertex );
  QVERIFY( c9.pointAt( 1, p, type ) );
  QCOMPARE( p.z(), 13.0 );
  QCOMPARE( p.m(), 14.0 );
  QCOMPARE( type, QgsVertexId::CurveVertex );
  QVERIFY( c9.pointAt( 2, p, type ) );
  QCOMPARE( p.z(), 23.0 );
  QCOMPARE( p.m(), 24.0 );
  QCOMPARE( type, QgsVertexId::SegmentVertex );
  QVERIFY( c9.pointAt( 3, p, type ) );
  QCOMPARE( p.z(), 13.0 );
  QCOMPARE( p.m(), 14.0 );
  QCOMPARE( type, QgsVertexId::SegmentVertex );

  //equality
  QgsCompoundCurve e1;
  QgsCompoundCurve e2;
  QVERIFY( e1 == e2 );
  QVERIFY( !( e1 != e2 ) );
  QgsLineString le1;
  QgsLineString le2;
  le1.setPoints( QgsPointSequence() << QgsPoint( 1, 2 ) );
  e1.addCurve( le1.clone() );
  QVERIFY( !( e1 == e2 ) ); //different number of curves
  QVERIFY( e1 != e2 );
  e2.addCurve( le1.clone() );
  QVERIFY( e1 == e2 );
  QVERIFY( !( e1 != e2 ) );
  le1.setPoints( QgsPointSequence() << QgsPoint( 1, 2 ) << QgsPoint( 1 / 3.0, 4 / 3.0 ) );
  e1.addCurve( le1.clone() );
  QVERIFY( !( e1 == e2 ) ); //different number of curves
  QVERIFY( e1 != e2 );
  le2.setPoints( QgsPointSequence() << QgsPoint( 1, 2 ) << QgsPoint( 2 / 6.0, 8 / 6.0 ) );
  e2.addCurve( le2.clone() );
  QVERIFY( e1 == e2 ); //check non-integer equality
  QVERIFY( !( e1 != e2 ) );
  le1.setPoints( QgsPointSequence() << QgsPoint( 1, 2 ) << QgsPoint( 1 / 3.0, 4 / 3.0 ) << QgsPoint( 7, 8 ) );
  e1.addCurve( le1.clone() );
  le2.setPoints( QgsPointSequence() << QgsPoint( 1, 2 ) << QgsPoint( 2 / 6.0, 8 / 6.0 ) << QgsPoint( 6, 9 ) );
  e2.addCurve( le2.clone() );
  QVERIFY( !( e1 == e2 ) ); //different coordinates
  QVERIFY( e1 != e2 );

  // different dimensions
  QgsCompoundCurve e3;
  e1.clear();
  e1.addCurve( le1.clone() );
  QgsLineString le3;
  le3.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZ, 1, 2, 0 )
                 << QgsPoint( QgsWkbTypes::PointZ, 1 / 3.0, 4 / 3.0, 0 )
                 << QgsPoint( QgsWkbTypes::PointZ, 7, 8, 0 ) );
  e3.addCurve( le3.clone() );
  QVERIFY( !( e1 == e3 ) ); //different dimension
  QVERIFY( e1 != e3 );
  QgsCompoundCurve e4;
  QgsLineString le4;
  le4.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZ, 1, 2, 2 )
                 << QgsPoint( QgsWkbTypes::PointZ, 1 / 3.0, 4 / 3.0, 3 )
                 << QgsPoint( QgsWkbTypes::PointZ, 7, 8, 4 ) );
  e4.addCurve( le4.clone() );
  QVERIFY( !( e3 == e4 ) ); //different z coordinates
  QVERIFY( e3 != e4 );
  QgsCompoundCurve e5;
  QgsLineString le5;
  le5.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointM, 1, 2, 0, 1 )
                 << QgsPoint( QgsWkbTypes::PointM, 1 / 3.0, 4 / 3.0, 0, 2 )
                 << QgsPoint( QgsWkbTypes::PointM, 7, 8, 0, 3 ) );
  e5.addCurve( le5.clone() );
  QgsCompoundCurve e6;
  QgsLineString le6;
  le6.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointM, 1, 2, 0, 11 )
                 << QgsPoint( QgsWkbTypes::PointM, 1 / 3.0, 4 / 3.0, 0, 12 )
                 << QgsPoint( QgsWkbTypes::PointM, 7, 8, 0, 13 ) );
  e6.addCurve( le6.clone() );
  QVERIFY( !( e5 == e6 ) ); //different m values
  QVERIFY( e5 != e6 );

  QVERIFY( e6 != QgsLineString() );

  // assignment operator
  e5.addCurve( le5.clone() );
  QVERIFY( e5 != e6 );
  e6 = e5;
  QCOMPARE( e5, e6 );

  //isClosed
  QgsCompoundCurve c11;
  QgsCircularString l11;
  QVERIFY( !c11.isClosed() );
  l11.setPoints( QgsPointSequence() << QgsPoint( 1, 2 )
                 << QgsPoint( 11, 2 )
                 << QgsPoint( 11, 22 )
                 << QgsPoint( 1, 22 ) );
  c11.addCurve( l11.clone() );
  QVERIFY( !c11.isClosed() );
  QgsLineString ls11;
  ls11.setPoints( QgsPointSequence() << QgsPoint( 1, 22 )
                  << QgsPoint( 1, 2 ) );
  c11.addCurve( ls11.clone() );
  QVERIFY( c11.isClosed() );

  //test that m values aren't considered when testing for closedness
  c11.clear();
  l11.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointM, 1, 2, 0, 3 )
                 << QgsPoint( QgsWkbTypes::PointM, 11, 2, 0, 4 )
                 << QgsPoint( QgsWkbTypes::PointM, 11, 22, 0, 5 )
                 << QgsPoint( QgsWkbTypes::PointM, 1, 2, 0, 6 ) );
  c11.addCurve( l11.clone() );
  QVERIFY( c11.isClosed() );

  //polygonf
  QgsCircularString lc13;
  QgsCompoundCurve c13;
  lc13.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 1, 2, 3, 4 )
                  << QgsPoint( QgsWkbTypes::PointZM, 11, 2, 11, 14 )
                  << QgsPoint( QgsWkbTypes::PointZM, 11, 22, 21, 24 )
                  << QgsPoint( QgsWkbTypes::PointZM, 1, 22, 31, 34 ) );
  c13.addCurve( lc13.clone() );
  QgsLineString ls13;
  ls13.setPoints( QgsPointSequence() << QgsPoint( 1, 22 ) << QgsPoint( 23, 22 ) );
  c13.addCurve( ls13.clone() );
  QPolygonF poly = c13.asQPolygonF();
  QCOMPARE( poly.count(), 5 );
  QCOMPARE( poly.at( 0 ).x(), 1.0 );
  QCOMPARE( poly.at( 0 ).y(), 2.0 );
  QCOMPARE( poly.at( 1 ).x(), 11.0 );
  QCOMPARE( poly.at( 1 ).y(), 2.0 );
  QCOMPARE( poly.at( 2 ).x(), 11.0 );
  QCOMPARE( poly.at( 2 ).y(), 22.0 );
  QCOMPARE( poly.at( 3 ).x(), 1.0 );
  QCOMPARE( poly.at( 3 ).y(), 22.0 );
  QCOMPARE( poly.at( 4 ).x(), 23.0 );
  QCOMPARE( poly.at( 4 ).y(), 22.0 );

  // clone tests
  QgsCircularString lc14;
  lc14.setPoints( QgsPointSequence() << QgsPoint( 1, 2 )
                  << QgsPoint( 11, 2 )
                  << QgsPoint( 11, 22 )
                  << QgsPoint( 1, 22 ) );
  QgsCompoundCurve c14;
  c14.addCurve( lc14.clone() );
  QgsLineString ls14;
  ls14.setPoints( QgsPointSequence() << QgsPoint( 1, 22 ) << QgsPoint( 23, 22 ) );
  c14.addCurve( ls14.clone() );
  std::unique_ptr<QgsCompoundCurve> cloned( c14.clone() );
  QCOMPARE( *cloned, c14 );

  //clone with Z/M
  lc14.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 1, 2, 3, 4 )
                  << QgsPoint( QgsWkbTypes::PointZM, 11, 2, 11, 14 )
                  << QgsPoint( QgsWkbTypes::PointZM, 11, 22, 21, 24 )
                  << QgsPoint( QgsWkbTypes::PointZM, 1, 22, 31, 34 ) );
  ls14.setPoints( QgsPointSequence() << QgsPoint( 1, 22, 31, 34 ) << QgsPoint( 23, 22, 42, 43 ) );
  c14.clear();
  c14.addCurve( lc14.clone() );
  c14.addCurve( ls14.clone() );
  cloned.reset( c14.clone() );
  QCOMPARE( *cloned, c14 );

  //clone an empty line
  c14.clear();
  cloned.reset( c14.clone() );
  QVERIFY( cloned->isEmpty() );
  QCOMPARE( cloned->numPoints(), 0 );
  QVERIFY( !cloned->is3D() );
  QVERIFY( !cloned->isMeasure() );
  QCOMPARE( cloned->wkbType(), QgsWkbTypes::CompoundCurve );

  //segmentize tests
  QgsCompoundCurve toSegment;
  QgsCircularString lcSegment;
  lcSegment.setPoints( QgsPointSequence() << QgsPoint( 1, 2 )
                       << QgsPoint( 11, 10 ) << QgsPoint( 21, 2 ) );
  toSegment.addCurve( lcSegment.clone() );
  std::unique_ptr<QgsLineString> segmentized( static_cast< QgsLineString * >( toSegment.segmentize() ) );
  QCOMPARE( segmentized->numPoints(), 156 );
  QCOMPARE( segmentized->vertexCount(), 156 );
  QCOMPARE( segmentized->ringCount(), 1 );
  QCOMPARE( segmentized->partCount(), 1 );
  QCOMPARE( segmentized->wkbType(), QgsWkbTypes::LineString );
  QVERIFY( !segmentized->is3D() );
  QVERIFY( !segmentized->isMeasure() );

  QCOMPARE( segmentized->pointN( 0 ), lcSegment.pointN( 0 ) );
  QCOMPARE( segmentized->pointN( segmentized->numPoints() - 1 ), lcSegment.pointN( toSegment.numPoints() - 1 ) );

  //segmentize with Z/M
  lcSegment.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 1, 2, 3, 4 )
                       << QgsPoint( QgsWkbTypes::PointZM, 11, 10, 11, 14 )
                       << QgsPoint( QgsWkbTypes::PointZM, 21, 2, 21, 24 ) );
  toSegment.clear();
  toSegment.addCurve( lcSegment.clone() );
  segmentized.reset( static_cast< QgsLineString * >( toSegment.segmentize() ) );
  QCOMPARE( segmentized->numPoints(), 156 );
  QCOMPARE( segmentized->vertexCount(), 156 );
  QCOMPARE( segmentized->ringCount(), 1 );
  QCOMPARE( segmentized->partCount(), 1 );
  QCOMPARE( segmentized->wkbType(), QgsWkbTypes::LineStringZM );
  QVERIFY( segmentized->is3D() );
  QVERIFY( segmentized->isMeasure() );
  QCOMPARE( segmentized->pointN( 0 ), lcSegment.pointN( 0 ) );
  QCOMPARE( segmentized->pointN( segmentized->numPoints() - 1 ), lcSegment.pointN( toSegment.numPoints() - 1 ) );

  //segmentize an empty line
  toSegment.clear();
  segmentized.reset( static_cast< QgsLineString * >( toSegment.segmentize() ) );
  QVERIFY( segmentized->isEmpty() );
  QCOMPARE( segmentized->numPoints(), 0 );
  QVERIFY( !segmentized->is3D() );
  QVERIFY( !segmentized->isMeasure() );
  QCOMPARE( segmentized->wkbType(), QgsWkbTypes::LineString );

  //to/from WKB
  QgsCompoundCurve c15;
  QgsCircularString l15;
  l15.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 1, 2, 3, 4 )
                 << QgsPoint( QgsWkbTypes::PointZM, 11, 2, 11, 14 )
                 << QgsPoint( QgsWkbTypes::PointZM, 11, 22, 21, 24 )
                 << QgsPoint( QgsWkbTypes::PointZM, 1, 22, 31, 34 ) );
  c15.addCurve( l15.clone() );
  QByteArray wkb15 = c15.asWkb();
  QgsCompoundCurve c16;
  QgsConstWkbPtr wkb15ptr( wkb15 );
  c16.fromWkb( wkb15ptr );
  QCOMPARE( c16.numPoints(), 4 );
  QCOMPARE( c16.vertexCount(), 4 );
  QCOMPARE( c16.nCoordinates(), 4 );
  QCOMPARE( c16.ringCount(), 1 );
  QCOMPARE( c16.partCount(), 1 );
  QCOMPARE( c16.wkbType(), QgsWkbTypes::CompoundCurveZM );
  QVERIFY( c16.is3D() );
  QVERIFY( c16.isMeasure() );
  QCOMPARE( c16.nCurves(), 1 );
  QCOMPARE( dynamic_cast< const QgsCircularString *>( c16.curveAt( 0 ) )->pointN( 0 ), l15.pointN( 0 ) );
  QCOMPARE( dynamic_cast< const QgsCircularString *>( c16.curveAt( 0 ) )->pointN( 1 ), l15.pointN( 1 ) );
  QCOMPARE( dynamic_cast< const QgsCircularString *>( c16.curveAt( 0 ) )->pointN( 2 ), l15.pointN( 2 ) );
  QCOMPARE( dynamic_cast< const QgsCircularString *>( c16.curveAt( 0 ) )->pointN( 3 ), l15.pointN( 3 ) );

  //bad WKB - check for no crash
  c16.clear();
  QgsConstWkbPtr nullPtr( nullptr, 0 );
  QVERIFY( !c16.fromWkb( nullPtr ) );
  QCOMPARE( c16.wkbType(), QgsWkbTypes::CompoundCurve );
  QgsPoint point( 1, 2 );
  QByteArray wkb16 = point.asWkb();
  QgsConstWkbPtr wkb16ptr( wkb16 );
  QVERIFY( !c16.fromWkb( wkb16ptr ) );
  QCOMPARE( c16.wkbType(), QgsWkbTypes::CompoundCurve );

  //to/from WKT
  QgsCompoundCurve c17;
  QgsCircularString l17;
  l17.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 1, 2, 3, 4 )
                 << QgsPoint( QgsWkbTypes::PointZM, 11, 2, 11, 14 )
                 << QgsPoint( QgsWkbTypes::PointZM, 11, 22, 21, 24 )
                 << QgsPoint( QgsWkbTypes::PointZM, 1, 22, 31, 34 ) );
  c17.addCurve( l17.clone() );

  QString wkt = c17.asWkt();
  QVERIFY( !wkt.isEmpty() );
  QgsCompoundCurve c18;
  QVERIFY( c18.fromWkt( wkt ) );
  QCOMPARE( c18.numPoints(), 4 );
  QCOMPARE( c18.wkbType(), QgsWkbTypes::CompoundCurveZM );
  QVERIFY( c18.is3D() );
  QVERIFY( c18.isMeasure() );
  QCOMPARE( dynamic_cast< const QgsCircularString *>( c18.curveAt( 0 ) )->pointN( 0 ), l17.pointN( 0 ) );
  QCOMPARE( dynamic_cast< const QgsCircularString *>( c18.curveAt( 0 ) )->pointN( 1 ), l17.pointN( 1 ) );
  QCOMPARE( dynamic_cast< const QgsCircularString *>( c18.curveAt( 0 ) )->pointN( 2 ), l17.pointN( 2 ) );
  QCOMPARE( dynamic_cast< const QgsCircularString *>( c18.curveAt( 0 ) )->pointN( 3 ), l17.pointN( 3 ) );

  //bad WKT
  QVERIFY( !c18.fromWkt( "Polygon()" ) );
  QVERIFY( c18.isEmpty() );
  QCOMPARE( c18.numPoints(), 0 );
  QVERIFY( !c18.is3D() );
  QVERIFY( !c18.isMeasure() );
  QCOMPARE( c18.wkbType(), QgsWkbTypes::CompoundCurve );
  QVERIFY( !c18.fromWkt( "CompoundCurve(LineString(0 0, 1 1),Point( 2 2 ))" ) );

  //asGML2
  QgsCompoundCurve exportCurve;
  QgsCircularString exportLine;
  exportLine.setPoints( QgsPointSequence() << QgsPoint( 31, 32 )
                        << QgsPoint( 41, 42 )
                        << QgsPoint( 51, 52 ) );
  exportCurve.addCurve( exportLine.clone() );
  QgsLineString exportLineString;
  exportLineString.setPoints( QgsPointSequence() << QgsPoint( 51, 52 )
                              << QgsPoint( 61, 62 ) );
  exportCurve.addCurve( exportLineString.clone() );

  QgsCircularString exportLineFloat;
  exportLineFloat.setPoints( QgsPointSequence() << QgsPoint( 1 / 3.0, 2 / 3.0 )
                             << QgsPoint( 1 + 1 / 3.0, 1 + 2 / 3.0 )
                             << QgsPoint( 2 + 1 / 3.0, 2 + 2 / 3.0 ) );
  QgsCompoundCurve exportCurveFloat;
  exportCurveFloat.addCurve( exportLineFloat.clone() );
  QgsLineString exportLineStringFloat;
  exportLineStringFloat.setPoints( QgsPointSequence() << QgsPoint( 2 + 1 / 3.0, 2 + 2 / 3.0 )
                                   << QgsPoint( 3 + 1 / 3.0, 3 + 2 / 3.0 ) );
  exportCurveFloat.addCurve( exportLineStringFloat.clone() );

  QDomDocument doc( QStringLiteral( "gml" ) );
  QString expectedGML2( QStringLiteral( "<LineString xmlns=\"gml\"><coordinates xmlns=\"gml\" cs=\",\" ts=\" \">31,32 41,42 51,52 61,62</coordinates></LineString>" ) );
  QString result = elemToString( exportCurve.asGml2( doc ) );
  QGSCOMPAREGML( result, expectedGML2 );
  QString expectedGML2prec3( QStringLiteral( "<LineString xmlns=\"gml\"><coordinates xmlns=\"gml\" cs=\",\" ts=\" \">0.333,0.667 1.333,1.667 2.333,2.667 3.333,3.667</coordinates></LineString>" ) );
  result = elemToString( exportCurveFloat.asGml2( doc, 3 ) );
  QGSCOMPAREGML( result, expectedGML2prec3 );
  QString expectedGML2empty( QStringLiteral( "<LineString xmlns=\"gml\"/>" ) );
  QGSCOMPAREGML( elemToString( QgsCompoundCurve().asGml2( doc ) ), expectedGML2empty );


  //asGML3
  QString expectedGML3( QStringLiteral( "<CompositeCurve xmlns=\"gml\"><curveMember xmlns=\"gml\"><Curve xmlns=\"gml\"><segments xmlns=\"gml\"><ArcString xmlns=\"gml\"><posList xmlns=\"gml\" srsDimension=\"2\">31 32 41 42 51 52</posList></ArcString></segments></Curve></curveMember><curveMember xmlns=\"gml\"><LineString xmlns=\"gml\"><posList xmlns=\"gml\" srsDimension=\"2\">51 52 61 62</posList></LineString></curveMember></CompositeCurve>" ) );
  result = elemToString( exportCurve.asGml3( doc ) );
  QCOMPARE( result, expectedGML3 );
  QString expectedGML3prec3( QStringLiteral( "<CompositeCurve xmlns=\"gml\"><curveMember xmlns=\"gml\"><Curve xmlns=\"gml\"><segments xmlns=\"gml\"><ArcString xmlns=\"gml\"><posList xmlns=\"gml\" srsDimension=\"2\">0.333 0.667 1.333 1.667 2.333 2.667</posList></ArcString></segments></Curve></curveMember><curveMember xmlns=\"gml\"><LineString xmlns=\"gml\"><posList xmlns=\"gml\" srsDimension=\"2\">2.333 2.667 3.333 3.667</posList></LineString></curveMember></CompositeCurve>" ) );
  result = elemToString( exportCurveFloat.asGml3( doc, 3 ) );
  QCOMPARE( result, expectedGML3prec3 );
  QString expectedGML3empty( QStringLiteral( "<CompositeCurve xmlns=\"gml\"/>" ) );
  QGSCOMPAREGML( elemToString( QgsCompoundCurve().asGml3( doc ) ), expectedGML3empty );

  //asJSON
  QString expectedJson( QStringLiteral( "{\"type\": \"LineString\", \"coordinates\": [ [31, 32], [41, 42], [51, 52], [61, 62]]}" ) );
  result = exportCurve.asJson();
  QCOMPARE( result, expectedJson );
  QString expectedJsonPrec3( QStringLiteral( "{\"type\": \"LineString\", \"coordinates\": [ [0.333, 0.667], [1.333, 1.667], [2.333, 2.667], [3.333, 3.667]]}" ) );
  result = exportCurveFloat.asJson( 3 );
  QCOMPARE( result, expectedJsonPrec3 );

  //length
  QgsCompoundCurve c19;
  QCOMPARE( c19.length(), 0.0 );
  QgsCircularString l19;
  l19.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 1, 1, 2, 3 )
                 << QgsPoint( QgsWkbTypes::PointZM, 1, 10, 4, 5 )
                 << QgsPoint( QgsWkbTypes::PointZM, 15, 10, 6, 7 ) );
  c19.addCurve( l19.clone() );
  QgsLineString l19a;
  l19a.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 15, 10, 6, 7 )
                  << QgsPoint( QgsWkbTypes::PointZM, 25, 10, 6, 7 ) );
  c19.addCurve( l19a.clone() );
  QGSCOMPARENEAR( c19.length(), 36.1433, 0.001 );

  //startPoint
  QCOMPARE( c19.startPoint(), QgsPoint( QgsWkbTypes::PointZM, 1, 1, 2, 3 ) );

  //endPoint
  QCOMPARE( c19.endPoint(), QgsPoint( QgsWkbTypes::PointZM, 25, 10, 6, 7 ) );

  //bad start/end points. Test that this doesn't crash.
  c19.clear();
  QCOMPARE( c19.startPoint(), QgsPoint() );
  QCOMPARE( c19.endPoint(), QgsPoint() );

  //curveToLine
  c19.clear();
  l19.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 1, 1, 2, 3 )
                 << QgsPoint( QgsWkbTypes::PointZM, 1, 10, 4, 5 )
                 << QgsPoint( QgsWkbTypes::PointZM, 15, 10, 6, 7 ) );
  c19.addCurve( l19.clone() );
  l19a.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 15, 10, 6, 7 )
                  << QgsPoint( QgsWkbTypes::PointZM, 25, 10, 6, 7 ) );
  c19.addCurve( l19a.clone() );
  segmentized.reset( c19.curveToLine() );
  QCOMPARE( segmentized->numPoints(), 182 );
  QCOMPARE( segmentized->wkbType(), QgsWkbTypes::LineStringZM );
  QVERIFY( segmentized->is3D() );
  QVERIFY( segmentized->isMeasure() );
  QCOMPARE( segmentized->pointN( 0 ), l19.pointN( 0 ) );
  QCOMPARE( segmentized->pointN( segmentized->numPoints() - 1 ), l19a.pointN( l19a.numPoints() - 1 ) );

  // points
  QgsCompoundCurve c20;
  QgsCircularString l20;
  QgsPointSequence points;
  c20.points( points );
  QVERIFY( points.isEmpty() );
  l20.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 1, 1, 2, 3 )
                 << QgsPoint( QgsWkbTypes::PointZM, 1, 10, 4, 5 )
                 << QgsPoint( QgsWkbTypes::PointZM, 15, 10, 6, 7 ) );
  c20.addCurve( l20.clone() );
  QgsLineString ls20;
  ls20.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 15, 10, 6, 7 )
                  << QgsPoint( QgsWkbTypes::PointZM, 25, 10, 6, 7 ) );
  c20.addCurve( ls20.clone() );
  c20.points( points );
  QCOMPARE( points.count(), 4 );
  QCOMPARE( points.at( 0 ), QgsPoint( QgsWkbTypes::PointZM, 1, 1, 2, 3 ) );
  QCOMPARE( points.at( 1 ), QgsPoint( QgsWkbTypes::PointZM, 1, 10, 4, 5 ) );
  QCOMPARE( points.at( 2 ), QgsPoint( QgsWkbTypes::PointZM, 15, 10, 6, 7 ) );
  QCOMPARE( points.at( 3 ), QgsPoint( QgsWkbTypes::PointZM, 25, 10, 6, 7 ) );

  //CRS transform
  QgsCoordinateReferenceSystem sourceSrs;
  sourceSrs.createFromSrid( 3994 );
  QgsCoordinateReferenceSystem destSrs;
  destSrs.createFromSrid( 4202 ); // want a transform with ellipsoid change
  QgsCoordinateTransform tr( sourceSrs, destSrs, QgsProject::instance() );

  // 2d CRS transform
  QgsCompoundCurve c21;
  QgsCircularString l21;
  l21.setPoints( QgsPointSequence() << QgsPoint( 6374985, -3626584 )
                 << QgsPoint( 6474985, -3526584 ) );
  c21.addCurve( l21.clone() );
  QgsLineString ls21;
  ls21.setPoints( QgsPointSequence() << QgsPoint( 6474985, -3526584 )
                  << QgsPoint( 6504985, -3526584 ) );
  c21.addCurve( ls21.clone() );
  c21.transform( tr, QgsCoordinateTransform::ForwardTransform );
  QGSCOMPARENEAR( c21.xAt( 0 ), 175.771, 0.001 );
  QGSCOMPARENEAR( c21.yAt( 0 ), -39.724, 0.001 );
  QGSCOMPARENEAR( c21.xAt( 1 ), 176.959, 0.001 );
  QGSCOMPARENEAR( c21.yAt( 1 ), -38.7999, 0.001 );
  QGSCOMPARENEAR( c21.xAt( 2 ), 177.315211, 0.001 );
  QGSCOMPARENEAR( c21.yAt( 2 ), -38.799974, 0.001 );
  QGSCOMPARENEAR( c21.boundingBox().xMinimum(), 175.770033, 0.001 );
  QGSCOMPARENEAR( c21.boundingBox().yMinimum(), -39.724, 0.001 );
  QGSCOMPARENEAR( c21.boundingBox().xMaximum(), 177.315211, 0.001 );
  QGSCOMPARENEAR( c21.boundingBox().yMaximum(), -38.7999, 0.001 );

  //3d CRS transform
  QgsCompoundCurve c22;
  l21.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 6374985, -3626584, 1, 2 )
                 << QgsPoint( QgsWkbTypes::PointZM, 6474985, -3526584, 3, 4 ) );
  c22.addCurve( l21.clone() );
  ls21.setPoints( QgsPointSequence() << QgsPoint( 6474985, -3526584, 3, 4 )
                  << QgsPoint( 6504985, -3526584, 5, 6 ) );
  c22.addCurve( ls21.clone() );
  c22.transform( tr, QgsCoordinateTransform::ForwardTransform );
  QgsPoint pt;
  QgsVertexId::VertexType v;
  c22.pointAt( 0, pt, v );
  QGSCOMPARENEAR( pt.x(), 175.771, 0.001 );
  QGSCOMPARENEAR( pt.y(), -39.724, 0.001 );
  QGSCOMPARENEAR( pt.z(), 1.0, 0.001 );
  QCOMPARE( pt.m(), 2.0 );
  c22.pointAt( 1, pt, v );
  QGSCOMPARENEAR( pt.x(), 176.959, 0.001 );
  QGSCOMPARENEAR( pt.y(), -38.7999, 0.001 );
  QGSCOMPARENEAR( pt.z(), 3.0, 0.001 );
  QCOMPARE( pt.m(), 4.0 );
  c22.pointAt( 2, pt, v );
  QGSCOMPARENEAR( pt.x(), 177.315211, 0.001 );
  QGSCOMPARENEAR( pt.y(), -38.7999, 0.001 );
  QGSCOMPARENEAR( pt.z(), 5.0, 0.001 );
  QCOMPARE( pt.m(), 6.0 );

  //reverse transform
  c22.transform( tr, QgsCoordinateTransform::ReverseTransform );
  c22.pointAt( 0, pt, v );
  QGSCOMPARENEAR( pt.x(), 6374985, 100 );
  QGSCOMPARENEAR( pt.y(), -3626584, 100 );
  QGSCOMPARENEAR( pt.z(), 1.0, 0.001 );
  QCOMPARE( pt.m(), 2.0 );
  c22.pointAt( 1, pt, v );
  QGSCOMPARENEAR( pt.x(), 6474985, 100 );
  QGSCOMPARENEAR( pt.y(), -3526584, 100 );
  QGSCOMPARENEAR( pt.z(), 3.0, 0.001 );
  QCOMPARE( pt.m(), 4.0 );
  c22.pointAt( 2, pt, v );
  QGSCOMPARENEAR( pt.x(), 6504985, 100 );
  QGSCOMPARENEAR( pt.y(), -3526584, 100 );
  QGSCOMPARENEAR( pt.z(), 5.0, 0.001 );
  QCOMPARE( pt.m(), 6.0 );

  //z value transform
  c22.transform( tr, QgsCoordinateTransform::ForwardTransform, true );
  c22.pointAt( 0, pt, v );
  QGSCOMPARENEAR( pt.z(), -19.249066, 0.001 );
  c22.pointAt( 1, pt, v );
  QGSCOMPARENEAR( pt.z(), -21.092128, 0.001 );
  c22.pointAt( 2, pt, v );
  QGSCOMPARENEAR( pt.z(), -19.370485, 0.001 );

  c22.transform( tr, QgsCoordinateTransform::ReverseTransform, true );
  c22.pointAt( 0, pt, v );
  QGSCOMPARENEAR( pt.z(), 1, 0.001 );
  c22.pointAt( 1, pt, v );
  QGSCOMPARENEAR( pt.z(), 3, 0.001 );
  c22.pointAt( 2, pt, v );
  QGSCOMPARENEAR( pt.z(), 5, 0.001 );

  //QTransform transform
  QTransform qtr = QTransform::fromScale( 2, 3 );
  QgsCompoundCurve c23;
  QgsCircularString l23;
  l23.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 1, 2, 3, 4 )
                 << QgsPoint( QgsWkbTypes::PointZM, 11, 12, 13, 14 ) );
  c23.addCurve( l23.clone() );
  QgsLineString ls23;
  ls23.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 11, 12, 13, 14 ) << QgsPoint( QgsWkbTypes::PointZM, 21, 13, 13, 14 ) );
  c23.addCurve( ls23.clone() );
  c23.transform( qtr, 5, 2, 4, 3 );
  c23.pointAt( 0, pt, v );
  QCOMPARE( pt, QgsPoint( QgsWkbTypes::PointZM, 2, 6, 11, 16 ) );
  c23.pointAt( 1, pt, v );
  QCOMPARE( pt, QgsPoint( QgsWkbTypes::PointZM, 22, 36, 31, 46 ) );
  c23.pointAt( 2, pt, v );
  QCOMPARE( pt, QgsPoint( QgsWkbTypes::PointZM, 42, 39, 31, 46 ) );
  QCOMPARE( c23.boundingBox(), QgsRectangle( 2, 6, 42, 39 ) );

  //insert vertex
  //cannot insert vertex in empty line
  QgsCompoundCurve c24;
  QVERIFY( !c24.insertVertex( QgsVertexId( 0, 0, 0 ), QgsPoint( 6.0, 7.0 ) ) );
  QCOMPARE( c24.numPoints(), 0 );

  //2d line
  QgsCircularString l24;
  l24.setPoints( QgsPointSequence() << QgsPoint( 1, 2 )
                 << QgsPoint( 11, 12 ) << QgsPoint( 1, 22 ) );
  c24.addCurve( l24.clone() );
  QVERIFY( c24.insertVertex( QgsVertexId( 0, 0, 1 ), QgsPoint( 4.0, 7.0 ) ) );
  QCOMPARE( c24.numPoints(), 5 );
  QVERIFY( !c24.is3D() );
  QVERIFY( !c24.isMeasure() );
  QCOMPARE( c24.wkbType(), QgsWkbTypes::CompoundCurve );
  c24.pointAt( 0, pt, v );
  QCOMPARE( pt, QgsPoint( 1.0, 2.0 ) );
  c24.pointAt( 1, pt, v );
  QCOMPARE( pt, QgsPoint( 4.0, 7.0 ) );
  c24.pointAt( 2, pt, v );
  QGSCOMPARENEAR( pt.x(), 7.192236, 0.01 );
  QGSCOMPARENEAR( pt.y(), 9.930870, 0.01 );
  c24.pointAt( 3, pt, v );
  QCOMPARE( pt, QgsPoint( 11.0, 12.0 ) );
  c24.pointAt( 4, pt, v );
  QCOMPARE( pt, QgsPoint( 1.0, 22.0 ) );

  QVERIFY( c24.insertVertex( QgsVertexId( 0, 0, 1 ), QgsPoint( 8.0, 9.0 ) ) );
  QVERIFY( c24.insertVertex( QgsVertexId( 0, 0, 2 ), QgsPoint( 18.0, 16.0 ) ) );
  QCOMPARE( c24.numPoints(), 9 );
  c24.pointAt( 0, pt, v );
  QCOMPARE( pt, QgsPoint( 1.0, 2.0 ) );
  c24.pointAt( 1, pt, v );
  QGSCOMPARENEAR( pt.x(), 4.363083, 0.01 );
  QGSCOMPARENEAR( pt.y(), 5.636917, 0.01 );
  c24.pointAt( 2, pt, v );
  QCOMPARE( pt, QgsPoint( 8.0, 9.0 ) );
  c24.pointAt( 3, pt, v );
  QCOMPARE( pt, QgsPoint( 18.0, 16.0 ) );
  c24.pointAt( 4, pt, v );
  QGSCOMPARENEAR( pt.x(), 5.876894, 0.01 );
  QGSCOMPARENEAR( pt.y(), 8.246211, 0.01 );
  c24.pointAt( 5, pt, v );
  QCOMPARE( pt, QgsPoint( 4.0, 7.0 ) );
  c24.pointAt( 6, pt, v );
  QGSCOMPARENEAR( pt.x(), 7.192236, 0.01 );
  QGSCOMPARENEAR( pt.y(), 9.930870, 0.01 );
  c24.pointAt( 7, pt, v );
  QCOMPARE( pt, QgsPoint( 11.0, 12.0 ) );
  c24.pointAt( 8, pt, v );
  QCOMPARE( pt, QgsPoint( 1.0, 22.0 ) );

  //insert vertex at end
  QVERIFY( !c24.insertVertex( QgsVertexId( 0, 0, 9 ), QgsPoint( 31.0, 32.0 ) ) );

  //insert vertex past end
  QVERIFY( !c24.insertVertex( QgsVertexId( 0, 0, 10 ), QgsPoint( 41.0, 42.0 ) ) );
  QCOMPARE( c24.numPoints(), 9 );

  //insert vertex before start
  QVERIFY( !c24.insertVertex( QgsVertexId( 0, 0, -18 ), QgsPoint( 41.0, 42.0 ) ) );
  QCOMPARE( c24.numPoints(), 9 );

  //insert 4d vertex in 4d line
  c24.clear();
  l24.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 1, 1, 2, 3 )
                 << QgsPoint( QgsWkbTypes::PointZM, 1, 10, 4, 5 )
                 << QgsPoint( QgsWkbTypes::PointZM, 15, 10, 6, 7 ) );
  c24.addCurve( l24.clone( ) );
  QVERIFY( c24.insertVertex( QgsVertexId( 0, 0, 1 ), QgsPoint( QgsWkbTypes::PointZM, 11, 12, 13, 14 ) ) );
  QCOMPARE( c24.numPoints(), 5 );
  c24.pointAt( 1, pt, v );
  QCOMPARE( pt, QgsPoint( QgsWkbTypes::PointZM, 11, 12, 13, 14 ) );

  //insert 2d vertex in 4d line
  QVERIFY( c24.insertVertex( QgsVertexId( 0, 0, 1 ), QgsPoint( 101, 102 ) ) );
  QCOMPARE( c24.numPoints(), 7 );
  QCOMPARE( c24.wkbType(), QgsWkbTypes::CompoundCurveZM );
  c24.pointAt( 1, pt, v );
  QCOMPARE( pt, QgsPoint( QgsWkbTypes::PointZM, 101, 102 ) );

  //insert 4d vertex in 2d line
  c24.clear();
  l24.setPoints( QgsPointSequence() << QgsPoint( 1, 2 )
                 << QgsPoint( 11, 12 ) << QgsPoint( 1, 22 ) );
  c24.addCurve( l24.clone() );
  QVERIFY( c24.insertVertex( QgsVertexId( 0, 0, 1 ), QgsPoint( QgsWkbTypes::PointZM, 2, 4, 103, 104 ) ) );
  QCOMPARE( c24.numPoints(), 5 );
  QCOMPARE( c24.wkbType(), QgsWkbTypes::CompoundCurve );
  c24.pointAt( 1, pt, v );
  QCOMPARE( pt, QgsPoint( QgsWkbTypes::Point, 2, 4 ) );

  // invalid
  QVERIFY( !c24.insertVertex( QgsVertexId( 0, 1, 0 ), QgsPoint( 1, 2 ) ) );

  //move vertex

  //empty line
  QgsCompoundCurve c25;
  QgsCircularString l25;
  QVERIFY( !c25.moveVertex( QgsVertexId( 0, 0, 0 ), QgsPoint( 6.0, 7.0 ) ) );
  QVERIFY( c25.isEmpty() );

  //valid line
  l25.setPoints( QgsPointSequence() << QgsPoint( 1, 2 )
                 << QgsPoint( 11, 12 ) << QgsPoint( 21, 22 ) );
  c25.addCurve( l25.clone() );
  QVERIFY( c25.moveVertex( QgsVertexId( 0, 0, 0 ), QgsPoint( 6.0, 7.0 ) ) );
  QVERIFY( c25.moveVertex( QgsVertexId( 0, 0, 1 ), QgsPoint( 16.0, 17.0 ) ) );
  QVERIFY( c25.moveVertex( QgsVertexId( 0, 0, 2 ), QgsPoint( 26.0, 27.0 ) ) );
  c25.pointAt( 0, pt, v );
  QCOMPARE( pt, QgsPoint( 6.0, 7.0 ) );
  c25.pointAt( 1, pt, v );
  QCOMPARE( pt, QgsPoint( 16.0, 17.0 ) );
  c25.pointAt( 2, pt, v );
  QCOMPARE( pt, QgsPoint( 26.0, 27.0 ) );

  //out of range
  QVERIFY( !c25.moveVertex( QgsVertexId( 0, 0, -1 ), QgsPoint( 3.0, 4.0 ) ) );
  QVERIFY( !c25.moveVertex( QgsVertexId( 0, 0, 10 ), QgsPoint( 3.0, 4.0 ) ) );
  QVERIFY( !c25.moveVertex( QgsVertexId( 0, 1, 10 ), QgsPoint( 3.0, 4.0 ) ) );
  c25.pointAt( 0, pt, v );
  QCOMPARE( pt, QgsPoint( 6.0, 7.0 ) );
  c25.pointAt( 1, pt, v );
  QCOMPARE( pt, QgsPoint( 16.0, 17.0 ) );
  c25.pointAt( 2, pt, v );
  QCOMPARE( pt, QgsPoint( 26.0, 27.0 ) );

  //move 4d point in 4d line
  l25.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 1, 1, 2, 3 )
                 << QgsPoint( QgsWkbTypes::PointZM, 1, 10, 4, 5 )
                 << QgsPoint( QgsWkbTypes::PointZM, 15, 10, 6, 7 ) );
  c25.clear();
  c25.addCurve( l25.clone() );
  QVERIFY( c25.moveVertex( QgsVertexId( 0, 0, 1 ), QgsPoint( QgsWkbTypes::PointZM, 6, 7, 12, 13 ) ) );
  c25.pointAt( 1, pt, v );
  QCOMPARE( pt, QgsPoint( QgsWkbTypes::PointZM, 6, 7, 12, 13 ) );

  //move 2d point in 4d line, existing z/m should be maintained
  QVERIFY( c25.moveVertex( QgsVertexId( 0, 0, 1 ), QgsPoint( 34, 35 ) ) );
  c25.pointAt( 1, pt, v );
  QCOMPARE( pt, QgsPoint( QgsWkbTypes::PointZM, 34, 35, 12, 13 ) );

  //move 4d point in 2d line
  l25.setPoints( QgsPointSequence() << QgsPoint( 1, 2 )
                 << QgsPoint( 11, 12 ) << QgsPoint( 21, 22 ) );
  c25.clear();
  c25.addCurve( l25.clone() );
  QVERIFY( c25.moveVertex( QgsVertexId( 0, 0, 0 ), QgsPoint( QgsWkbTypes::PointZM, 3, 4, 2, 3 ) ) );
  c25.pointAt( 0, pt, v );
  QCOMPARE( pt, QgsPoint( 3, 4 ) );

  //delete vertex

  //empty line
  QgsCompoundCurve c26;
  QgsCircularString l26;
  QVERIFY( !c26.deleteVertex( QgsVertexId( 0, 0, 0 ) ) );
  QVERIFY( c26.isEmpty() );

  //valid line
  l26.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 1, 2, 2, 3 )
                 << QgsPoint( QgsWkbTypes::PointZM, 11, 12, 4, 5 )
                 << QgsPoint( QgsWkbTypes::PointZM, 21, 22, 6, 7 )
                 << QgsPoint( QgsWkbTypes::PointZM, 31, 32, 6, 7 ) );
  c26.addCurve( l26.clone() );
  //out of range vertices
  QVERIFY( !c26.deleteVertex( QgsVertexId( 0, 0, -1 ) ) );
  QVERIFY( !c26.deleteVertex( QgsVertexId( 0, 0, 100 ) ) );

  //valid vertices
  QVERIFY( c26.deleteVertex( QgsVertexId( 0, 0, 1 ) ) );
  QCOMPARE( c26.numPoints(), 2 );
  c26.pointAt( 0, pt, v );
  QCOMPARE( pt, QgsPoint( QgsWkbTypes::PointZM, 1, 2, 2, 3 ) );
  c26.pointAt( 1, pt, v );
  QCOMPARE( pt, QgsPoint( QgsWkbTypes::PointZM, 31, 32, 6, 7 ) );

  //removing the next vertex removes all remaining vertices
  QVERIFY( c26.deleteVertex( QgsVertexId( 0, 0, 0 ) ) );
  QCOMPARE( c26.numPoints(), 0 );
  QVERIFY( c26.isEmpty() );

  // two lines
  QgsLineString ls26;
  c26.clear();
  ls26.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 1, 2, 2, 3 )
                  << QgsPoint( QgsWkbTypes::PointZM, 11, 12, 4, 5 ) );
  c26.addCurve( ls26.clone() );
  ls26.setPoints( QgsPointSequence()
                  << QgsPoint( QgsWkbTypes::PointZM, 11, 12, 4, 5 )
                  << QgsPoint( QgsWkbTypes::PointZM, 21, 32, 4, 5 )
                  << QgsPoint( QgsWkbTypes::PointZM, 31, 42, 4, 5 ) );
  c26.addCurve( ls26.clone() );
  QVERIFY( c26.deleteVertex( QgsVertexId( 0, 0, 1 ) ) );
  QCOMPARE( c26.nCurves(), 1 );
  const QgsLineString *ls26r = dynamic_cast< const QgsLineString * >( c26.curveAt( 0 ) );
  QCOMPARE( ls26r->numPoints(), 2 );
  QCOMPARE( ls26r->startPoint(), QgsPoint( QgsWkbTypes::PointZM, 1, 2, 2, 3 ) );
  QCOMPARE( ls26r->endPoint(), QgsPoint( QgsWkbTypes::PointZM, 31, 42, 4, 5 ) );

  c26.clear();
  ls26.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 1, 2, 2, 3 )
                  << QgsPoint( QgsWkbTypes::PointZM, 11, 12, 4, 5 )
                  << QgsPoint( QgsWkbTypes::PointZM, 21, 32, 4, 5 ) );
  c26.addCurve( ls26.clone() );
  ls26.setPoints( QgsPointSequence()
                  << QgsPoint( QgsWkbTypes::PointZM, 21, 32, 4, 5 )
                  << QgsPoint( QgsWkbTypes::PointZM, 31, 42, 4, 5 ) );
  c26.addCurve( ls26.clone() );
  QVERIFY( c26.deleteVertex( QgsVertexId( 0, 0, 2 ) ) );
  QCOMPARE( c26.nCurves(), 1 );
  ls26r = dynamic_cast< const QgsLineString * >( c26.curveAt( 0 ) );
  QCOMPARE( ls26r->numPoints(), 2 );
  QCOMPARE( ls26r->startPoint(), QgsPoint( QgsWkbTypes::PointZM, 1, 2, 2, 3 ) );
  QCOMPARE( ls26r->endPoint(), QgsPoint( QgsWkbTypes::PointZM, 31, 42, 4, 5 ) );

  //reversed
  QgsCompoundCurve c27;
  QgsCircularString l27;
  std::unique_ptr< QgsCompoundCurve > reversed( c27.reversed() );
  QVERIFY( reversed->isEmpty() );
  l27.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 1, 2, 2, 3 )
                 << QgsPoint( QgsWkbTypes::PointZM, 11, 12, 4, 5 )
                 << QgsPoint( QgsWkbTypes::PointZM, 21, 22, 6, 7 ) );
  c27.addCurve( l27.clone() );
  QgsLineString ls27;
  ls27.setPoints( QgsPointSequence()
                  << QgsPoint( QgsWkbTypes::PointZM, 21, 22, 6, 7 )
                  << QgsPoint( QgsWkbTypes::PointZM, 23, 32, 7, 8 ) );
  c27.addCurve( ls27.clone() );

  reversed.reset( c27.reversed() );
  QCOMPARE( reversed->numPoints(), 4 );
  QVERIFY( dynamic_cast< const QgsLineString * >( reversed->curveAt( 0 ) ) );
  QVERIFY( dynamic_cast< const QgsCircularString * >( reversed->curveAt( 1 ) ) );
  QCOMPARE( reversed->wkbType(), QgsWkbTypes::CompoundCurveZM );
  QVERIFY( reversed->is3D() );
  QVERIFY( reversed->isMeasure() );
  reversed->pointAt( 0, pt, v );
  reversed->pointAt( 1, pt, v );
  QCOMPARE( pt, QgsPoint( QgsWkbTypes::PointZM, 21, 22, 6, 7 ) );
  reversed->pointAt( 2, pt, v );
  QCOMPARE( pt, QgsPoint( QgsWkbTypes::PointZM, 11, 12, 4, 5 ) );
  reversed->pointAt( 3, pt, v );
  QCOMPARE( pt, QgsPoint( QgsWkbTypes::PointZM, 1, 2, 2, 3 ) );

  //addZValue
  QgsCompoundCurve c28;
  QgsCircularString l28;
  QCOMPARE( c28.wkbType(), QgsWkbTypes::CompoundCurve );
  QVERIFY( c28.addZValue() );
  QCOMPARE( c28.wkbType(), QgsWkbTypes::CompoundCurveZ );
  c28.clear();
  //2d line
  l28.setPoints( QgsPointSequence() << QgsPoint( 1, 2 ) << QgsPoint( 11, 12 ) );
  c28.addCurve( l28.clone() );
  l28.setPoints( QgsPointSequence() << QgsPoint( 11, 12 ) << QgsPoint( 3, 4 ) );
  c28.addCurve( l28.clone() );
  QVERIFY( c28.addZValue( 2 ) );
  QVERIFY( c28.is3D() );
  QCOMPARE( c28.wkbType(), QgsWkbTypes::CompoundCurveZ );
  c28.pointAt( 0, pt, v );
  QCOMPARE( pt, QgsPoint( QgsWkbTypes::PointZ, 1, 2, 2 ) );
  c28.pointAt( 1, pt, v );
  QCOMPARE( pt, QgsPoint( QgsWkbTypes::PointZ, 11, 12, 2 ) );
  c28.pointAt( 2, pt, v );
  QCOMPARE( pt, QgsPoint( QgsWkbTypes::PointZ, 3, 4, 2 ) );

  QVERIFY( !c28.addZValue( 4 ) ); //already has z value, test that existing z is unchanged
  c28.pointAt( 0, pt, v );
  QCOMPARE( pt, QgsPoint( QgsWkbTypes::PointZ, 1, 2, 2 ) );
  c28.pointAt( 1, pt, v );
  QCOMPARE( pt, QgsPoint( QgsWkbTypes::PointZ, 11, 12, 2 ) );
  c28.pointAt( 2, pt, v );
  QCOMPARE( pt, QgsPoint( QgsWkbTypes::PointZ, 3, 4, 2 ) );

  //linestring with m
  l28.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointM, 1, 2, 0, 3 ) << QgsPoint( QgsWkbTypes::PointM, 11, 12, 0, 4 ) );
  c28.clear();
  c28.addCurve( l28.clone() );
  l28.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointM, 11, 12, 0, 4 ) << QgsPoint( QgsWkbTypes::PointM, 21, 32, 0, 4 ) );
  c28.addCurve( l28.clone() );
  QVERIFY( c28.addZValue( 5 ) );
  QVERIFY( c28.is3D() );
  QVERIFY( c28.isMeasure() );
  QCOMPARE( c28.wkbType(), QgsWkbTypes::CompoundCurveZM );
  c28.pointAt( 0, pt, v );
  QCOMPARE( pt, QgsPoint( QgsWkbTypes::PointZM, 1, 2, 5, 3 ) );
  c28.pointAt( 1, pt, v );
  QCOMPARE( pt, QgsPoint( QgsWkbTypes::PointZM, 11, 12, 5, 4 ) );
  c28.pointAt( 2, pt, v );
  QCOMPARE( pt, QgsPoint( QgsWkbTypes::PointZM, 21, 32, 5, 4 ) );

  //addMValue
  c28.clear();
  //2d line
  l28.setPoints( QgsPointSequence() << QgsPoint( 1, 2 ) << QgsPoint( 11, 12 ) );
  c28.addCurve( l28.clone() );
  l28.setPoints( QgsPointSequence() << QgsPoint( 11, 12 ) << QgsPoint( 3, 4 ) );
  c28.addCurve( l28.clone() );
  QVERIFY( c28.addMValue( 2 ) );
  QVERIFY( c28.isMeasure() );
  QCOMPARE( c28.wkbType(), QgsWkbTypes::CompoundCurveM );
  c28.pointAt( 0, pt, v );
  QCOMPARE( pt, QgsPoint( QgsWkbTypes::PointM, 1, 2, 0, 2 ) );
  c28.pointAt( 1, pt, v );
  QCOMPARE( pt, QgsPoint( QgsWkbTypes::PointM, 11, 12, 0, 2 ) );
  c28.pointAt( 2, pt, v );
  QCOMPARE( pt, QgsPoint( QgsWkbTypes::PointM, 3, 4, 0, 2 ) );

  QVERIFY( !c28.addMValue( 4 ) ); //already has z value, test that existing z is unchanged
  c28.pointAt( 0, pt, v );
  QCOMPARE( pt, QgsPoint( QgsWkbTypes::PointM, 1, 2, 0, 2 ) );
  c28.pointAt( 1, pt, v );
  QCOMPARE( pt, QgsPoint( QgsWkbTypes::PointM, 11, 12, 0, 2 ) );
  c28.pointAt( 2, pt, v );
  QCOMPARE( pt, QgsPoint( QgsWkbTypes::PointM, 3, 4, 0, 2 ) );

  //linestring with z
  l28.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZ, 1, 2, 3 ) << QgsPoint( QgsWkbTypes::PointZ, 11, 12, 4 ) );
  c28.clear();
  c28.addCurve( l28.clone() );
  l28.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZ, 11, 12, 4 ) << QgsPoint( QgsWkbTypes::PointZ, 21, 32, 4 ) );
  c28.addCurve( l28.clone() );
  QVERIFY( c28.addMValue( 5 ) );
  QVERIFY( c28.is3D() );
  QVERIFY( c28.isMeasure() );
  QCOMPARE( c28.wkbType(), QgsWkbTypes::CompoundCurveZM );
  c28.pointAt( 0, pt, v );
  QCOMPARE( pt, QgsPoint( QgsWkbTypes::PointZM, 1, 2, 3, 5 ) );
  c28.pointAt( 1, pt, v );
  QCOMPARE( pt, QgsPoint( QgsWkbTypes::PointZM, 11, 12, 4, 5 ) );
  c28.pointAt( 2, pt, v );
  QCOMPARE( pt, QgsPoint( QgsWkbTypes::PointZM, 21, 32, 4, 5 ) );

  //dropZValue
  QgsCompoundCurve c28d;
  QgsCircularString l28d;
  QVERIFY( !c28d.dropZValue() );
  l28d.setPoints( QgsPointSequence() << QgsPoint( 1, 2 ) << QgsPoint( 11, 12 ) );
  c28d.addCurve( l28d.clone() );
  l28d.setPoints( QgsPointSequence() << QgsPoint( 11, 12 ) << QgsPoint( 21, 22 ) );
  c28d.addCurve( l28d.clone() );
  QVERIFY( !c28d.dropZValue() );
  c28d.addZValue( 1.0 );
  QCOMPARE( c28d.wkbType(), QgsWkbTypes::CompoundCurveZ );
  QVERIFY( c28d.is3D() );
  QVERIFY( c28d.dropZValue() );
  QVERIFY( !c28d.is3D() );
  QCOMPARE( c28d.wkbType(), QgsWkbTypes::CompoundCurve );
  c28d.pointAt( 0, pt, v );
  QCOMPARE( pt, QgsPoint( QgsWkbTypes::Point, 1, 2 ) );
  c28d.pointAt( 1, pt, v );
  QCOMPARE( pt, QgsPoint( QgsWkbTypes::Point, 11, 12 ) );
  c28d.pointAt( 2, pt, v );
  QCOMPARE( pt, QgsPoint( QgsWkbTypes::Point, 21, 22 ) );

  QVERIFY( !c28d.dropZValue() ); //already dropped
  //linestring with m
  l28d.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 1, 2, 3, 4 ) << QgsPoint( QgsWkbTypes::PointZM, 11, 12, 3, 4 ) );
  c28d.clear();
  c28d.addCurve( l28d.clone() );
  l28d.setPoints( QgsPointSequence() <<  QgsPoint( QgsWkbTypes::PointZM, 11, 12, 3, 4 ) << QgsPoint( QgsWkbTypes::PointZM, 21, 22, 3, 4 ) );
  c28d.addCurve( l28d.clone() );
  QVERIFY( c28d.dropZValue() );
  QVERIFY( !c28d.is3D() );
  QVERIFY( c28d.isMeasure() );
  QCOMPARE( c28d.wkbType(), QgsWkbTypes::CompoundCurveM );
  c28d.pointAt( 0, pt, v );
  QCOMPARE( pt, QgsPoint( QgsWkbTypes::PointM, 1, 2, 0, 4 ) );
  c28d.pointAt( 1, pt, v );
  QCOMPARE( pt, QgsPoint( QgsWkbTypes::PointM, 11, 12, 0, 4 ) );
  c28d.pointAt( 2, pt, v );
  QCOMPARE( pt, QgsPoint( QgsWkbTypes::PointM, 21, 22, 0, 4 ) );

  //dropMValue
  c28d.clear();
  QVERIFY( !c28d.dropMValue() );
  l28d.setPoints( QgsPointSequence() << QgsPoint( 1, 2 ) << QgsPoint( 11, 12 ) );
  c28d.addCurve( l28d.clone() );
  l28d.setPoints( QgsPointSequence() << QgsPoint( 11, 12 ) << QgsPoint( 21, 22 ) );
  c28d.addCurve( l28d.clone() );
  QVERIFY( !c28d.dropMValue() );
  c28d.addMValue( 1.0 );
  QCOMPARE( c28d.wkbType(), QgsWkbTypes::CompoundCurveM );
  QVERIFY( c28d.isMeasure() );
  QVERIFY( c28d.dropMValue() );
  QVERIFY( !c28d.isMeasure() );
  QCOMPARE( c28d.wkbType(), QgsWkbTypes::CompoundCurve );
  c28d.pointAt( 0, pt, v );
  QCOMPARE( pt, QgsPoint( QgsWkbTypes::Point, 1, 2 ) );
  c28d.pointAt( 1, pt, v );
  QCOMPARE( pt, QgsPoint( QgsWkbTypes::Point, 11, 12 ) );
  c28d.pointAt( 2, pt, v );
  QCOMPARE( pt, QgsPoint( QgsWkbTypes::Point, 21, 22 ) );

  QVERIFY( !c28d.dropMValue() ); //already dropped
  //linestring with z
  l28d.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 1, 2, 3, 4 ) << QgsPoint( QgsWkbTypes::PointZM, 11, 12, 3, 4 ) );
  c28d.clear();
  c28d.addCurve( l28d.clone() );
  l28d.setPoints( QgsPointSequence() <<  QgsPoint( QgsWkbTypes::PointZM, 11, 12, 3, 4 ) << QgsPoint( QgsWkbTypes::PointZM, 21, 22, 3, 4 ) );
  c28d.addCurve( l28d.clone() );
  QVERIFY( c28d.dropMValue() );
  QVERIFY( !c28d.isMeasure() );
  QVERIFY( c28d.is3D() );
  QCOMPARE( c28d.wkbType(), QgsWkbTypes::CompoundCurveZ );
  c28d.pointAt( 0, pt, v );
  QCOMPARE( pt, QgsPoint( QgsWkbTypes::PointZ, 1, 2, 3 ) );
  c28d.pointAt( 1, pt, v );
  QCOMPARE( pt, QgsPoint( QgsWkbTypes::PointZ, 11, 12, 3 ) );
  c28d.pointAt( 2, pt, v );
  QCOMPARE( pt, QgsPoint( QgsWkbTypes::PointZ, 21, 22, 3 ) );

  //convertTo
  l28d.setPoints( QgsPointSequence() << QgsPoint( 1, 2 ) << QgsPoint( 11, 12 ) );
  c28d.clear();
  c28d.addCurve( l28d.clone() );
  QVERIFY( c28d.convertTo( QgsWkbTypes::CompoundCurve ) );
  QCOMPARE( c28d.wkbType(), QgsWkbTypes::CompoundCurve );
  QVERIFY( c28d.convertTo( QgsWkbTypes::CompoundCurveZ ) );
  QCOMPARE( c28d.wkbType(), QgsWkbTypes::CompoundCurveZ );
  c28d.pointAt( 0, pt, v );
  QCOMPARE( pt, QgsPoint( QgsWkbTypes::PointZ, 1, 2 ) );

  QVERIFY( c28d.convertTo( QgsWkbTypes::CompoundCurveZM ) );
  QCOMPARE( c28d.wkbType(), QgsWkbTypes::CompoundCurveZM );
  c28d.pointAt( 0, pt, v );
  QCOMPARE( pt, QgsPoint( QgsWkbTypes::PointZM, 1, 2 ) );
  c28d.moveVertex( QgsVertexId( 0, 0, 0 ), QgsPoint( 1, 2, 5 ) );
  c28d.pointAt( 0, pt, v );
  QCOMPARE( pt, QgsPoint( QgsWkbTypes::PointZM, 1, 2, 5.0 ) );
  QVERIFY( c28d.convertTo( QgsWkbTypes::CompoundCurveM ) );
  QCOMPARE( c28d.wkbType(), QgsWkbTypes::CompoundCurveM );
  c28d.pointAt( 0, pt, v );
  QCOMPARE( pt, QgsPoint( QgsWkbTypes::PointM, 1, 2 ) );
  c28d.moveVertex( QgsVertexId( 0, 0, 0 ), QgsPoint( 1, 2, 0, 6 ) );
  c28d.pointAt( 0, pt, v );
  QCOMPARE( pt, QgsPoint( QgsWkbTypes::PointM, 1, 2, 0.0, 6.0 ) );
  QVERIFY( c28d.convertTo( QgsWkbTypes::CompoundCurve ) );
  QCOMPARE( c28d.wkbType(), QgsWkbTypes::CompoundCurve );
  c28d.pointAt( 0, pt, v );
  QCOMPARE( pt, QgsPoint( 1, 2 ) );
  QVERIFY( !c28d.convertTo( QgsWkbTypes::Polygon ) );

  //isRing
  QgsCircularString l30;
  QgsCompoundCurve c30;
  QVERIFY( !c30.isRing() );
  l30.setPoints( QgsPointSequence() << QgsPoint( 1, 2 ) << QgsPoint( 11, 12 ) << QgsPoint( 1, 2 ) );
  c30.addCurve( l30.clone() );
  QVERIFY( !c30.isRing() ); //<4 points
  l30.setPoints( QgsPointSequence() << QgsPoint( 1, 2 ) << QgsPoint( 11, 12 ) << QgsPoint( 21, 22 ) << QgsPoint( 31, 32 ) );
  c30.clear();
  c30.addCurve( l30.clone() );
  QVERIFY( !c30.isRing() ); //not closed
  l30.setPoints( QgsPointSequence() << QgsPoint( 1, 2 ) << QgsPoint( 11, 12 ) << QgsPoint( 21, 22 ) << QgsPoint( 1, 2 ) );
  c30.clear();
  c30.addCurve( l30.clone() );
  QVERIFY( c30.isRing() );
  l30.setPoints( QgsPointSequence() << QgsPoint( 1, 2 ) << QgsPoint( 11, 12 ) );
  c30.clear();
  c30.addCurve( l30.clone() );
  QVERIFY( !c30.isRing() );
  l30.setPoints( QgsPointSequence() << QgsPoint( 11, 12 )  << QgsPoint( 21, 22 ) );
  c30.addCurve( l30.clone() );
  QVERIFY( !c30.isRing() );
  l30.setPoints( QgsPointSequence() << QgsPoint( 21, 22 )  << QgsPoint( 1, 2 ) );
  c30.addCurve( l30.clone() );
  QVERIFY( c30.isRing() );

  //coordinateSequence
  QgsCompoundCurve c31;
  QgsCircularString l31;
  QgsCoordinateSequence coords = c31.coordinateSequence();
  QCOMPARE( coords.count(), 1 );
  QCOMPARE( coords.at( 0 ).count(), 1 );
  QVERIFY( coords.at( 0 ).at( 0 ).isEmpty() );
  l31.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 1, 2, 2, 3 )
                 << QgsPoint( QgsWkbTypes::PointZM, 11, 12, 4, 5 )
                 << QgsPoint( QgsWkbTypes::PointZM, 21, 22, 6, 7 ) );
  c31.addCurve( l31.clone() );
  QgsLineString ls31;
  ls31.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 21, 22, 6, 7 ) <<
                  QgsPoint( QgsWkbTypes::PointZM, 31, 32, 16, 17 ) );
  c31.addCurve( ls31.clone() );
  coords = c31.coordinateSequence();
  QCOMPARE( coords.count(), 1 );
  QCOMPARE( coords.at( 0 ).count(), 1 );
  QCOMPARE( coords.at( 0 ).at( 0 ).count(), 4 );
  QCOMPARE( coords.at( 0 ).at( 0 ).at( 0 ), QgsPoint( QgsWkbTypes::PointZM, 1, 2, 2, 3 ) );
  QCOMPARE( coords.at( 0 ).at( 0 ).at( 1 ), QgsPoint( QgsWkbTypes::PointZM, 11, 12, 4, 5 ) );
  QCOMPARE( coords.at( 0 ).at( 0 ).at( 2 ), QgsPoint( QgsWkbTypes::PointZM, 21, 22, 6, 7 ) );
  QCOMPARE( coords.at( 0 ).at( 0 ).at( 3 ), QgsPoint( QgsWkbTypes::PointZM, 31, 32, 16, 17 ) );

  //nextVertex
  QgsCompoundCurve c32;
  QgsCircularString l32;
  QgsVertexId vId;
  QVERIFY( !c32.nextVertex( vId, p ) );
  vId = QgsVertexId( 0, 0, -2 );
  QVERIFY( !c32.nextVertex( vId, p ) );
  vId = QgsVertexId( 0, 0, 10 );
  QVERIFY( !c32.nextVertex( vId, p ) );
  //CircularString
  l32.setPoints( QgsPointSequence() << QgsPoint( 1, 2 ) << QgsPoint( 11, 12 ) );
  c32.addCurve( l32.clone() );
  vId = QgsVertexId( 0, 0, 2 ); //out of range
  QVERIFY( !c32.nextVertex( vId, p ) );
  vId = QgsVertexId( 0, 0, -5 );
  QVERIFY( c32.nextVertex( vId, p ) );
  vId = QgsVertexId( 0, 0, -1 );
  QVERIFY( c32.nextVertex( vId, p ) );
  QCOMPARE( vId, QgsVertexId( 0, 0, 0 ) );
  QCOMPARE( p, QgsPoint( 1, 2 ) );
  QVERIFY( c32.nextVertex( vId, p ) );
  QCOMPARE( vId, QgsVertexId( 0, 0, 1 ) );
  QCOMPARE( p, QgsPoint( 11, 12 ) );
  QVERIFY( !c32.nextVertex( vId, p ) );
  vId = QgsVertexId( 0, 1, 0 );
  QVERIFY( c32.nextVertex( vId, p ) );
  QCOMPARE( vId, QgsVertexId( 0, 1, 1 ) ); //test that ring number is maintained
  QCOMPARE( p, QgsPoint( 11, 12 ) );
  vId = QgsVertexId( 1, 0, 0 );
  QVERIFY( c32.nextVertex( vId, p ) );
  QCOMPARE( vId, QgsVertexId( 1, 0, 1 ) ); //test that part number is maintained
  QCOMPARE( p, QgsPoint( 11, 12 ) );
  QgsLineString ls32;
  ls32.setPoints( QgsPointSequence() << QgsPoint( 11, 12 ) << QgsPoint( 13, 14 ) );
  c32.addCurve( ls32.clone() );
  vId = QgsVertexId( 0, 0, 1 );
  QVERIFY( c32.nextVertex( vId, p ) );
  QCOMPARE( vId, QgsVertexId( 0, 0, 2 ) );
  QCOMPARE( p, QgsPoint( 13, 14 ) );
  QVERIFY( !c32.nextVertex( vId, p ) );

  //CircularStringZ
  l32.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZ, 1, 2, 3 ) << QgsPoint( QgsWkbTypes::PointZ, 11, 12, 13 ) );
  c32.clear();
  c32.addCurve( l32.clone() );
  vId = QgsVertexId( 0, 0, -1 );
  QVERIFY( c32.nextVertex( vId, p ) );
  QCOMPARE( vId, QgsVertexId( 0, 0, 0 ) );
  QCOMPARE( p, QgsPoint( QgsWkbTypes::PointZ, 1, 2, 3 ) );
  QVERIFY( c32.nextVertex( vId, p ) );
  QCOMPARE( vId, QgsVertexId( 0, 0, 1 ) );
  QCOMPARE( p, QgsPoint( QgsWkbTypes::PointZ, 11, 12, 13 ) );
  QVERIFY( !c32.nextVertex( vId, p ) );
  //CircularStringM
  l32.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointM, 1, 2, 0, 4 ) << QgsPoint( QgsWkbTypes::PointM, 11, 12, 0, 14 ) );
  c32.clear();
  c32.addCurve( l32.clone() );
  vId = QgsVertexId( 0, 0, -1 );
  QVERIFY( c32.nextVertex( vId, p ) );
  QCOMPARE( vId, QgsVertexId( 0, 0, 0 ) );
  QCOMPARE( p, QgsPoint( QgsWkbTypes::PointM, 1, 2, 0, 4 ) );
  QVERIFY( c32.nextVertex( vId, p ) );
  QCOMPARE( vId, QgsVertexId( 0, 0, 1 ) );
  QCOMPARE( p, QgsPoint( QgsWkbTypes::PointM, 11, 12, 0, 14 ) );
  QVERIFY( !c32.nextVertex( vId, p ) );
  //CircularStringZM
  l32.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 1, 2, 3, 4 ) << QgsPoint( QgsWkbTypes::PointZM, 11, 12, 13, 14 ) );
  c32.clear();
  c32.addCurve( l32.clone() );
  vId = QgsVertexId( 0, 0, -1 );
  QVERIFY( c32.nextVertex( vId, p ) );
  QCOMPARE( vId, QgsVertexId( 0, 0, 0 ) );
  QCOMPARE( p, QgsPoint( QgsWkbTypes::PointZM, 1, 2, 3, 4 ) );
  QVERIFY( c32.nextVertex( vId, p ) );
  QCOMPARE( vId, QgsVertexId( 0, 0, 1 ) );
  QCOMPARE( p, QgsPoint( QgsWkbTypes::PointZM, 11, 12, 13, 14 ) );
  QVERIFY( !c32.nextVertex( vId, p ) );

  //vertexAt and pointAt
  QgsCompoundCurve c33;
  QgsCircularString l33;
  c33.vertexAt( QgsVertexId( 0, 0, -10 ) ); //out of bounds, check for no crash
  c33.vertexAt( QgsVertexId( 0, 0, 10 ) ); //out of bounds, check for no crash
  QVERIFY( !c33.pointAt( -10, p, type ) );
  QVERIFY( !c33.pointAt( 10, p, type ) );
  //CircularString
  l33.setPoints( QgsPointSequence() << QgsPoint( 1, 2 ) << QgsPoint( 11, 12 ) << QgsPoint( 1, 22 ) );
  c33.addCurve( l33.clone() );
  c33.vertexAt( QgsVertexId( 0, 0, -10 ) );
  c33.vertexAt( QgsVertexId( 0, 0, 10 ) ); //out of bounds, check for no crash
  QCOMPARE( c33.vertexAt( QgsVertexId( 0, 0, 0 ) ), QgsPoint( 1, 2 ) );
  QCOMPARE( c33.vertexAt( QgsVertexId( 0, 0, 1 ) ), QgsPoint( 11, 12 ) );
  QCOMPARE( c33.vertexAt( QgsVertexId( 0, 0, 2 ) ), QgsPoint( 1, 22 ) );
  QVERIFY( !c33.pointAt( -10, p, type ) );
  QVERIFY( !c33.pointAt( 10, p, type ) );
  QVERIFY( c33.pointAt( 0, p, type ) );
  QCOMPARE( p, QgsPoint( 1, 2 ) );
  QCOMPARE( type, QgsVertexId::SegmentVertex );
  QVERIFY( c33.pointAt( 1, p, type ) );
  QCOMPARE( p, QgsPoint( 11, 12 ) );
  QCOMPARE( type, QgsVertexId::CurveVertex );
  QVERIFY( c33.pointAt( 2, p, type ) );
  QCOMPARE( p, QgsPoint( 1, 22 ) );
  QCOMPARE( type, QgsVertexId::SegmentVertex );
  QgsLineString ls33;
  ls33.setPoints( QgsPointSequence() << QgsPoint( 1, 22 ) << QgsPoint( 3, 34 ) );
  c33.addCurve( ls33.clone() );
  QVERIFY( c33.pointAt( 3, p, type ) );
  QCOMPARE( p, QgsPoint( 3, 34 ) );
  QCOMPARE( type, QgsVertexId::SegmentVertex );

  c33.clear();
  //CircularStringZ
  l33.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZ, 1, 2, 3 ) << QgsPoint( QgsWkbTypes::PointZ, 11, 12, 13 )  << QgsPoint( QgsWkbTypes::PointZ, 1, 22, 23 ) );
  c33.addCurve( l33.clone() );
  QCOMPARE( c33.vertexAt( QgsVertexId( 0, 0, 0 ) ), QgsPoint( QgsWkbTypes::PointZ, 1, 2, 3 ) );
  QCOMPARE( c33.vertexAt( QgsVertexId( 0, 0, 1 ) ), QgsPoint( QgsWkbTypes::PointZ, 11, 12, 13 ) );
  QCOMPARE( c33.vertexAt( QgsVertexId( 0, 0, 2 ) ), QgsPoint( QgsWkbTypes::PointZ, 1, 22, 23 ) );
  QVERIFY( c33.pointAt( 0, p, type ) );
  QCOMPARE( p, QgsPoint( QgsWkbTypes::PointZ, 1, 2, 3 ) );
  QCOMPARE( type, QgsVertexId::SegmentVertex );
  QVERIFY( c33.pointAt( 1, p, type ) );
  QCOMPARE( p, QgsPoint( QgsWkbTypes::PointZ, 11, 12, 13 ) );
  QCOMPARE( type, QgsVertexId::CurveVertex );
  QVERIFY( c33.pointAt( 2, p, type ) );
  QCOMPARE( p, QgsPoint( 1, 22, 23 ) );
  QCOMPARE( type, QgsVertexId::SegmentVertex );

  //CircularStringM
  c33.clear();
  l33.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointM, 1, 2, 0, 4 ) << QgsPoint( QgsWkbTypes::PointM, 11, 12, 0, 14 ) << QgsPoint( QgsWkbTypes::PointM, 1, 22, 0, 24 ) );
  c33.addCurve( l33.clone() );
  QCOMPARE( c33.vertexAt( QgsVertexId( 0, 0, 0 ) ), QgsPoint( QgsWkbTypes::PointM, 1, 2, 0, 4 ) );
  QCOMPARE( c33.vertexAt( QgsVertexId( 0, 0, 1 ) ), QgsPoint( QgsWkbTypes::PointM, 11, 12, 0, 14 ) );
  QCOMPARE( c33.vertexAt( QgsVertexId( 0, 0, 2 ) ), QgsPoint( QgsWkbTypes::PointM, 1, 22, 0, 24 ) );
  QVERIFY( c33.pointAt( 0, p, type ) );
  QCOMPARE( p, QgsPoint( QgsWkbTypes::PointM, 1, 2, 0, 4 ) );
  QCOMPARE( type, QgsVertexId::SegmentVertex );
  QVERIFY( c33.pointAt( 1, p, type ) );
  QCOMPARE( p, QgsPoint( QgsWkbTypes::PointM, 11, 12, 0, 14 ) );
  QCOMPARE( type, QgsVertexId::CurveVertex );
  QVERIFY( c33.pointAt( 2, p, type ) );
  QCOMPARE( p, QgsPoint( QgsWkbTypes::PointM, 1, 22, 0, 24 ) );
  QCOMPARE( type, QgsVertexId::SegmentVertex );
  //CircularStringZM
  l33.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 1, 2, 3, 4 ) << QgsPoint( QgsWkbTypes::PointZM, 11, 12, 13, 14 ) << QgsPoint( QgsWkbTypes::PointZM, 1, 22, 23, 24 ) );
  c33.clear();
  c33.addCurve( l33.clone() );
  QCOMPARE( c33.vertexAt( QgsVertexId( 0, 0, 0 ) ), QgsPoint( QgsWkbTypes::PointZM, 1, 2, 3, 4 ) );
  QCOMPARE( c33.vertexAt( QgsVertexId( 0, 0, 1 ) ), QgsPoint( QgsWkbTypes::PointZM, 11, 12, 13, 14 ) );
  QCOMPARE( c33.vertexAt( QgsVertexId( 0, 0, 2 ) ), QgsPoint( QgsWkbTypes::PointZM, 1, 22, 23, 24 ) );
  QVERIFY( c33.pointAt( 0, p, type ) );
  QCOMPARE( p, QgsPoint( QgsWkbTypes::PointZM, 1, 2, 3, 4 ) );
  QCOMPARE( type, QgsVertexId::SegmentVertex );
  QVERIFY( c33.pointAt( 1, p, type ) );
  QCOMPARE( p, QgsPoint( QgsWkbTypes::PointZM, 11, 12, 13, 14 ) );
  QCOMPARE( type, QgsVertexId::CurveVertex );
  QVERIFY( c33.pointAt( 2, p, type ) );
  QCOMPARE( p, QgsPoint( QgsWkbTypes::PointZM, 1, 22, 23, 24 ) );
  QCOMPARE( type, QgsVertexId::SegmentVertex );

  //centroid
  QgsCircularString l34;
  QgsCompoundCurve c34;
  QCOMPARE( c34.centroid(), QgsPoint() );
  l34.setPoints( QgsPointSequence() << QgsPoint( 5, 10 ) );
  c34.addCurve( l34.clone() );
  QCOMPARE( c34.centroid(), QgsPoint( 5, 10 ) );
  l34.setPoints( QgsPointSequence() << QgsPoint( 0, 0 ) << QgsPoint( 20, 10 ) << QgsPoint( 2, 9 ) );
  c34.clear();
  c34.addCurve( l34.clone() );
  QgsPoint centroid = c34.centroid();
  QGSCOMPARENEAR( centroid.x(), 7.333, 0.001 );
  QGSCOMPARENEAR( centroid.y(), 6.333, 0.001 );
  l34.setPoints( QgsPointSequence() << QgsPoint( 2, 9 ) << QgsPoint( 12, 9 ) << QgsPoint( 15, 19 ) );
  c34.addCurve( l34.clone() );
  centroid = c34.centroid();
  QGSCOMPARENEAR( centroid.x(), 9.756646, 0.001 );
  QGSCOMPARENEAR( centroid.y(), 8.229039, 0.001 );

  //closest segment
  QgsCompoundCurve c35;
  QgsCircularString l35;
  int leftOf = 0;
  p = QgsPoint(); // reset all coords to zero
  ( void )c35.closestSegment( QgsPoint( 1, 2 ), p, vId ); //empty line, just want no crash
  l35.setPoints( QgsPointSequence() << QgsPoint( 5, 10 ) );
  c35.addCurve( l35.clone() );
  QVERIFY( c35.closestSegment( QgsPoint( 5, 10 ), p, vId ) < 0 );
  l35.setPoints( QgsPointSequence() << QgsPoint( 5, 10 ) << QgsPoint( 7, 12 ) << QgsPoint( 5, 15 ) );
  c35.clear();
  c35.addCurve( l35.clone() );
  QGSCOMPARENEAR( c35.closestSegment( QgsPoint( 4, 11 ), p, vId, &leftOf ), 2.0, 0.0001 );
  QCOMPARE( p, QgsPoint( 5, 10 ) );
  QCOMPARE( vId, QgsVertexId( 0, 0, 1 ) );
  QCOMPARE( leftOf, -1 );
  QGSCOMPARENEAR( c35.closestSegment( QgsPoint( 8, 11 ), p, vId, &leftOf ),  1.583512, 0.0001 );
  QGSCOMPARENEAR( p.x(), 6.84, 0.01 );
  QGSCOMPARENEAR( p.y(), 11.49, 0.01 );
  QCOMPARE( vId, QgsVertexId( 0, 0, 1 ) );
  QCOMPARE( leftOf, 1 );
  QGSCOMPARENEAR( c35.closestSegment( QgsPoint( 5.5, 11.5 ), p, vId, &leftOf ), 1.288897, 0.0001 );
  QGSCOMPARENEAR( p.x(), 6.302776, 0.01 );
  QGSCOMPARENEAR( p.y(), 10.7, 0.01 );
  QCOMPARE( vId, QgsVertexId( 0, 0, 1 ) );
  QCOMPARE( leftOf, -1 );
  QGSCOMPARENEAR( c35.closestSegment( QgsPoint( 7, 16 ), p, vId, &leftOf ), 3.068288, 0.0001 );
  QGSCOMPARENEAR( p.x(), 5.981872, 0.01 );
  QGSCOMPARENEAR( p.y(), 14.574621, 0.01 );
  QCOMPARE( vId, QgsVertexId( 0, 0, 2 ) );
  QCOMPARE( leftOf, 1 );
  QGSCOMPARENEAR( c35.closestSegment( QgsPoint( 5.5, 13.5 ), p, vId, &leftOf ), 1.288897, 0.0001 );
  QGSCOMPARENEAR( p.x(), 6.302776, 0.01 );
  QGSCOMPARENEAR( p.y(), 14.3, 0.01 );
  QCOMPARE( vId, QgsVertexId( 0, 0, 2 ) );
  QCOMPARE( leftOf, -1 );
  // point directly on segment
  QCOMPARE( c35.closestSegment( QgsPoint( 5, 15 ), p, vId, &leftOf ), 0.0 );
  QCOMPARE( p, QgsPoint( 5, 15 ) );
  QCOMPARE( vId, QgsVertexId( 0, 0, 2 ) );
  QCOMPARE( leftOf, 0 );

  QgsLineString ls35;
  ls35.setPoints( QgsPointSequence() << QgsPoint( 5, 15 ) << QgsPoint( 5, 20 ) );
  c35.addCurve( ls35.clone() );
  QGSCOMPARENEAR( c35.closestSegment( QgsPoint( 5.5, 16.5 ), p, vId, &leftOf ), 0.25, 0.0001 );
  QGSCOMPARENEAR( p.x(), 5.0, 0.01 );
  QGSCOMPARENEAR( p.y(), 16.5, 0.01 );
  QCOMPARE( vId, QgsVertexId( 0, 0, 3 ) );
  QCOMPARE( leftOf, 1 );
  QGSCOMPARENEAR( c35.closestSegment( QgsPoint( 4.5, 16.5 ), p, vId, &leftOf ), 0.25, 0.0001 );
  QGSCOMPARENEAR( p.x(), 5.0, 0.01 );
  QGSCOMPARENEAR( p.y(), 16.5, 0.01 );
  QCOMPARE( vId, QgsVertexId( 0, 0, 3 ) );
  QCOMPARE( leftOf, -1 );
  QGSCOMPARENEAR( c35.closestSegment( QgsPoint( 4.5, 21.5 ), p, vId, &leftOf ), 2.500000, 0.0001 );
  QGSCOMPARENEAR( p.x(), 5.0, 0.01 );
  QGSCOMPARENEAR( p.y(), 20.0, 0.01 );
  QCOMPARE( vId, QgsVertexId( 0, 0, 3 ) );
  QCOMPARE( leftOf, -1 );
  QGSCOMPARENEAR( c35.closestSegment( QgsPoint( 5.5, 21.5 ), p, vId, &leftOf ), 2.500000, 0.0001 );
  QGSCOMPARENEAR( p.x(), 5.0, 0.01 );
  QGSCOMPARENEAR( p.y(), 20.0, 0.01 );
  QCOMPARE( vId, QgsVertexId( 0, 0, 3 ) );
  QCOMPARE( leftOf, 1 );
  QGSCOMPARENEAR( c35.closestSegment( QgsPoint( 5, 20 ), p, vId, &leftOf ), 0.0000, 0.0001 );
  QGSCOMPARENEAR( p.x(), 5.0, 0.01 );
  QGSCOMPARENEAR( p.y(), 20.0, 0.01 );
  QCOMPARE( vId, QgsVertexId( 0, 0, 3 ) );
  QCOMPARE( leftOf, 0 );

  //sumUpArea
  QgsCompoundCurve c36;
  QgsCircularString l36;
  double area = 1.0; //sumUpArea adds to area, so start with non-zero value
  c36.sumUpArea( area );
  QCOMPARE( area, 1.0 );
  l36.setPoints( QgsPointSequence() << QgsPoint( 5, 10 ) );
  c36.addCurve( l36.clone() );
  c36.sumUpArea( area );
  QCOMPARE( area, 1.0 );
  l36.setPoints( QgsPointSequence() << QgsPoint( 5, 10 ) << QgsPoint( 10, 10 ) );
  c36.clear();
  c36.addCurve( l36.clone() );
  c36.sumUpArea( area );
  QCOMPARE( area, 1.0 );
  l36.setPoints( QgsPointSequence() << QgsPoint( 0, 0 ) << QgsPoint( 2, 0 ) << QgsPoint( 2, 2 ) );
  c36.clear();
  c36.addCurve( l36.clone() );
  c36.sumUpArea( area );
  QGSCOMPARENEAR( area, 4.141593, 0.0001 );
  l36.setPoints( QgsPointSequence() << QgsPoint( 0, 0 ) << QgsPoint( 2, 0 ) << QgsPoint( 2, 2 ) << QgsPoint( 0, 2 ) );
  c36.clear();
  c36.addCurve( l36.clone() );
  c36.sumUpArea( area );
  QGSCOMPARENEAR( area, 7.283185, 0.0001 );
  // full circle
  l36.setPoints( QgsPointSequence() << QgsPoint( 0, 0 ) << QgsPoint( 4, 0 ) << QgsPoint( 0, 0 ) );
  c36.clear();
  c36.addCurve( l36.clone() );
  area = 0.0;
  c36.sumUpArea( area );
  QGSCOMPARENEAR( area, 12.566370614359172, 0.0001 );

  //boundingBox - test that bounding box is updated after every modification to the circular string
  QgsCompoundCurve c37;
  QgsCircularString l37;
  QVERIFY( c37.boundingBox().isNull() );
  l37.setPoints( QgsPointSequence() << QgsPoint( 5, 10 ) << QgsPoint( 10, 15 ) );
  c37.addCurve( l37.clone() );
  QCOMPARE( c37.boundingBox(), QgsRectangle( 5, 10, 10, 15 ) );
  l37.setPoints( QgsPointSequence() << QgsPoint( -5, -10 ) << QgsPoint( -6, -10 ) << QgsPoint( -5.5, -9 ) );
  c37.clear();
  c37.addCurve( l37.clone() );
  QCOMPARE( c37.boundingBox(), QgsRectangle( -6.125, -10.25, -5, -9 ) );
  QByteArray wkbToAppend = c37.asWkb();
  c37.clear();
  QVERIFY( c37.boundingBox().isNull() );
  QgsConstWkbPtr wkbToAppendPtr( wkbToAppend );
  l37.setPoints( QgsPointSequence() << QgsPoint( 5, 10 ) << QgsPoint( 10, 15 ) );
  c37.clear();
  c37.addCurve( l37.clone() );
  QCOMPARE( c37.boundingBox(), QgsRectangle( 5, 10, 10, 15 ) );
  c37.fromWkb( wkbToAppendPtr );
  QCOMPARE( c37.boundingBox(), QgsRectangle( -6.125, -10.25, -5, -9 ) );
  c37.fromWkt( QStringLiteral( "CompoundCurve(CircularString( 5 10, 6 10, 5.5 9 ))" ) );
  QCOMPARE( c37.boundingBox(), QgsRectangle( 5, 9, 6.125, 10.25 ) );
  c37.insertVertex( QgsVertexId( 0, 0, 1 ), QgsPoint( -1, 7 ) );
  QgsRectangle r = c37.boundingBox();
  QGSCOMPARENEAR( r.xMinimum(), -3.014, 0.01 );
  QGSCOMPARENEAR( r.xMaximum(), 14.014, 0.01 );
  QGSCOMPARENEAR( r.yMinimum(), -7.0146, 0.01 );
  QGSCOMPARENEAR( r.yMaximum(), 12.4988, 0.01 );
  c37.moveVertex( QgsVertexId( 0, 0, 1 ), QgsPoint( -3, 10 ) );
  r = c37.boundingBox();
  QGSCOMPARENEAR( r.xMinimum(), -10.294, 0.01 );
  QGSCOMPARENEAR( r.xMaximum(), 12.294, 0.01 );
  QGSCOMPARENEAR( r.yMinimum(), 9, 0.01 );
  QGSCOMPARENEAR( r.yMaximum(), 31.856, 0.01 );
  c37.deleteVertex( QgsVertexId( 0, 0, 1 ) );
  r = c37.boundingBox();
  QGSCOMPARENEAR( r.xMinimum(), 5, 0.01 );
  QGSCOMPARENEAR( r.xMaximum(), 6.125, 0.01 );
  QGSCOMPARENEAR( r.yMinimum(), 9, 0.01 );
  QGSCOMPARENEAR( r.yMaximum(), 10.25, 0.01 );

  //angle
  QgsCompoundCurve c38;
  QgsCircularString l38;
  ( void )c38.vertexAngle( QgsVertexId() ); //just want no crash
  ( void )c38.vertexAngle( QgsVertexId( 0, 0, 0 ) ); //just want no crash
  l38.setPoints( QgsPointSequence() << QgsPoint( 0, 0 ) );
  c38.addCurve( l38.clone() );
  ( void )c38.vertexAngle( QgsVertexId( 0, 0, 0 ) ); //just want no crash, any answer is meaningless
  l38.setPoints( QgsPointSequence() << QgsPoint( 0, 0 ) << QgsPoint( 1, 0 ) );
  c38.clear();
  c38.addCurve( l38.clone() );
  ( void )c38.vertexAngle( QgsVertexId( 0, 0, 0 ) ); //just want no crash, any answer is meaningless
  ( void )c38.vertexAngle( QgsVertexId( 0, 0, 1 ) ); //just want no crash, any answer is meaningless
  l38.setPoints( QgsPointSequence() << QgsPoint( 0, 0 ) << QgsPoint( 1, 1 ) << QgsPoint( 0, 2 ) );
  c38.clear();
  c38.addCurve( l38.clone() );
  QGSCOMPARENEAR( c38.vertexAngle( QgsVertexId( 0, 0, 0 ) ), 1.5708, 0.0001 );
  QGSCOMPARENEAR( c38.vertexAngle( QgsVertexId( 0, 0, 1 ) ), 0, 0.0001 );
  QGSCOMPARENEAR( c38.vertexAngle( QgsVertexId( 0, 0, 2 ) ), 4.712389, 0.0001 );
  l38.setPoints( QgsPointSequence() << QgsPoint( 0, 2 ) << QgsPoint( 1, 1 ) << QgsPoint( 0, 0 ) );
  c38.clear();
  c38.addCurve( l38.clone() );
  QGSCOMPARENEAR( c38.vertexAngle( QgsVertexId( 0, 0, 0 ) ), 1.5708, 0.0001 );
  QGSCOMPARENEAR( c38.vertexAngle( QgsVertexId( 0, 0, 1 ) ), 3.141593, 0.0001 );
  QGSCOMPARENEAR( c38.vertexAngle( QgsVertexId( 0, 0, 2 ) ), 4.712389, 0.0001 );
  ( void )c38.vertexAngle( QgsVertexId( 0, 0, 20 ) ); // no crash
  l38.setPoints( QgsPointSequence() << QgsPoint( 0, 0 ) << QgsPoint( 1, 1 ) << QgsPoint( 0, 2 )
                 << QgsPoint( -1, 3 ) << QgsPoint( 0, 4 ) );
  c38.clear();
  c38.addCurve( l38.clone() );
  QGSCOMPARENEAR( c38.vertexAngle( QgsVertexId( 0, 0, 0 ) ), 1.5708, 0.0001 );
  QGSCOMPARENEAR( c38.vertexAngle( QgsVertexId( 0, 0, 1 ) ), 0, 0.0001 );
  QGSCOMPARENEAR( c38.vertexAngle( QgsVertexId( 0, 0, 2 ) ), 4.712389, 0.0001 );
  QGSCOMPARENEAR( c38.vertexAngle( QgsVertexId( 0, 0, 3 ) ), 0, 0.0001 );
  QGSCOMPARENEAR( c38.vertexAngle( QgsVertexId( 0, 0, 4 ) ), 1.5708, 0.0001 );
  l38.setPoints( QgsPointSequence() << QgsPoint( 0, 4 ) << QgsPoint( -1, 3 ) << QgsPoint( 0, 2 )
                 << QgsPoint( 1, 1 ) << QgsPoint( 0, 0 ) );
  c38.clear();
  c38.addCurve( l38.clone() );
  QGSCOMPARENEAR( c38.vertexAngle( QgsVertexId( 0, 0, 0 ) ), 4.712389, 0.0001 );
  QGSCOMPARENEAR( c38.vertexAngle( QgsVertexId( 0, 0, 1 ) ), 3.141592, 0.0001 );
  QGSCOMPARENEAR( c38.vertexAngle( QgsVertexId( 0, 0, 2 ) ), 1.5708, 0.0001 );
  QGSCOMPARENEAR( c38.vertexAngle( QgsVertexId( 0, 0, 3 ) ), 3.141592, 0.0001 );
  QGSCOMPARENEAR( c38.vertexAngle( QgsVertexId( 0, 0, 4 ) ), 4.712389, 0.0001 );

  // with second curve
  QgsLineString ls38;
  ls38.setPoints( QgsPointSequence() << QgsPoint( 0, 0 ) << QgsPoint( 0, -1 ) );
  c38.addCurve( ls38.clone() );
  QGSCOMPARENEAR( c38.vertexAngle( QgsVertexId( 0, 0, 4 ) ), 3.926991, 0.0001 );
  QGSCOMPARENEAR( c38.vertexAngle( QgsVertexId( 0, 0, 5 ) ), 3.141593, 0.0001 );

  //closed circular string
  l38.setPoints( QgsPointSequence() << QgsPoint( 0, 0 ) << QgsPoint( 1, 0 ) << QgsPoint( 0, 0 ) );
  c38.clear();
  c38.addCurve( l38.clone() );
  QGSCOMPARENEAR( c38.vertexAngle( QgsVertexId( 0, 0, 0 ) ), 0, 0.00001 );
  QGSCOMPARENEAR( c38.vertexAngle( QgsVertexId( 0, 0, 1 ) ), 3.141592, 0.00001 );
  QGSCOMPARENEAR( c38.vertexAngle( QgsVertexId( 0, 0, 2 ) ), 0, 0.00001 );

  //removing a vertex from a 3 point comound curveshould remove the whole line
  QgsCircularString l39;
  QgsCompoundCurve c39;
  l39.setPoints( QVector<QgsPoint>() << QgsPoint( 0, 0 ) << QgsPoint( 1, 1 ) << QgsPoint( 0, 2 ) );
  c39.addCurve( l39.clone() );
  QCOMPARE( c39.numPoints(), 3 );
  c39.deleteVertex( QgsVertexId( 0, 0, 2 ) );
  QCOMPARE( c39.numPoints(), 0 );

  //boundary
  QgsCompoundCurve cBoundary1;
  QgsCircularString boundary1;
  QVERIFY( !cBoundary1.boundary() );
  boundary1.setPoints( QVector<QgsPoint>() << QgsPoint( 0, 0 ) << QgsPoint( 1, 0 ) << QgsPoint( 1, 1 ) );
  cBoundary1.addCurve( boundary1.clone() );
  QgsAbstractGeometry *boundary = cBoundary1.boundary();
  QgsMultiPoint *mpBoundary = dynamic_cast< QgsMultiPoint * >( boundary );
  QVERIFY( mpBoundary );
  QCOMPARE( static_cast< QgsPoint *>( mpBoundary->geometryN( 0 ) )->x(), 0.0 );
  QCOMPARE( static_cast< QgsPoint *>( mpBoundary->geometryN( 0 ) )->y(), 0.0 );
  QCOMPARE( static_cast< QgsPoint *>( mpBoundary->geometryN( 1 ) )->x(), 1.0 );
  QCOMPARE( static_cast< QgsPoint *>( mpBoundary->geometryN( 1 ) )->y(), 1.0 );
  delete boundary;

  // closed string = no boundary
  boundary1.setPoints( QVector<QgsPoint>() << QgsPoint( 0, 0 ) << QgsPoint( 1, 0 ) << QgsPoint( 1, 1 ) << QgsPoint( 0, 0 ) );
  cBoundary1.clear();
  cBoundary1.addCurve( boundary1.clone() );
  QVERIFY( !cBoundary1.boundary() );

  //boundary with z
  boundary1.setPoints( QVector<QgsPoint>() << QgsPoint( QgsWkbTypes::PointZ, 0, 0, 10 ) << QgsPoint( QgsWkbTypes::PointZ, 1, 0, 15 ) << QgsPoint( QgsWkbTypes::PointZ, 1, 1, 20 ) );
  cBoundary1.clear();
  cBoundary1.addCurve( boundary1.clone() );
  boundary = cBoundary1.boundary();
  mpBoundary = dynamic_cast< QgsMultiPoint * >( boundary );
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

  // addToPainterPath (note most tests are in test_qgsgeometry.py)
  QgsCompoundCurve ccPath;
  QgsCircularString path;
  QPainterPath pPath;
  ccPath.addToPainterPath( pPath );
  QVERIFY( pPath.isEmpty() );
  path.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZ, 1, 2, 3 ) << QgsPoint( QgsWkbTypes::PointZ, 11, 12, 13 )
                  << QgsPoint( QgsWkbTypes::PointZ, 21, 2, 3 ) );
  ccPath.addCurve( path.clone() );
  ccPath.addToPainterPath( pPath );
  QGSCOMPARENEAR( pPath.currentPosition().x(), 21.0, 0.01 );
  QGSCOMPARENEAR( pPath.currentPosition().y(), 2.0, 0.01 );
  QVERIFY( !pPath.isEmpty() );
  QgsLineString lsPath;
  lsPath.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZ, 21, 2, 3 )
                    << QgsPoint( QgsWkbTypes::PointZ, 31, 12, 3 ) );
  ccPath.addCurve( lsPath.clone() );
  pPath = QPainterPath();
  ccPath.addToPainterPath( pPath );
  QGSCOMPARENEAR( pPath.currentPosition().x(), 31.0, 0.01 );
  QGSCOMPARENEAR( pPath.currentPosition().y(), 12.0, 0.01 );

  // even number of points - should still work
  pPath = QPainterPath();
  path.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZ, 1, 2, 3 ) << QgsPoint( QgsWkbTypes::PointZ, 11, 12, 13 ) );
  ccPath.clear();
  ccPath.addCurve( path.clone() );
  ccPath.addToPainterPath( pPath );
  QGSCOMPARENEAR( pPath.currentPosition().x(), 11.0, 0.01 );
  QGSCOMPARENEAR( pPath.currentPosition().y(), 12.0, 0.01 );
  QVERIFY( !pPath.isEmpty() );

  // toCurveType
  QgsCircularString curveLine1;
  QgsCompoundCurve cc1;
  curveLine1.setPoints( QgsPointSequence() << QgsPoint( 1, 2 ) << QgsPoint( 11, 12 ) << QgsPoint( 1, 22 ) );
  cc1.addCurve( curveLine1.clone() );
  std::unique_ptr< QgsCurve > curveType( cc1.toCurveType() );
  QCOMPARE( curveType->wkbType(), QgsWkbTypes::CompoundCurve );
  QCOMPARE( curveType->numPoints(), 3 );
  QCOMPARE( curveType->vertexAt( QgsVertexId( 0, 0, 0 ) ), QgsPoint( 1, 2 ) );
  QCOMPARE( curveType->vertexAt( QgsVertexId( 0, 0, 1 ) ), QgsPoint( 11, 12 ) );
  QCOMPARE( curveType->vertexAt( QgsVertexId( 0, 0, 2 ) ), QgsPoint( 1, 22 ) );
  QgsLineString ccls1;
  ccls1.setPoints( QgsPointSequence() << QgsPoint( 1, 22 ) << QgsPoint( 1, 25 ) );
  cc1.addCurve( ccls1.clone() );
  curveType.reset( cc1.toCurveType() );
  QCOMPARE( curveType->wkbType(), QgsWkbTypes::CompoundCurve );
  QCOMPARE( curveType->numPoints(), 4 );
  QCOMPARE( curveType->vertexAt( QgsVertexId( 0, 0, 0 ) ), QgsPoint( 1, 2 ) );
  QCOMPARE( curveType->vertexAt( QgsVertexId( 0, 0, 1 ) ), QgsPoint( 11, 12 ) );
  QCOMPARE( curveType->vertexAt( QgsVertexId( 0, 0, 2 ) ), QgsPoint( 1, 22 ) );
  QCOMPARE( curveType->vertexAt( QgsVertexId( 0, 0, 3 ) ), QgsPoint( 1, 25 ) );

  //test that area of a compound curve ring is equal to a closed linestring with the same vertices
  QgsCompoundCurve cc;
  QgsLineString *ll1 = new QgsLineString();
  ll1->setPoints( QgsPointSequence() << QgsPoint( 1, 1 ) << QgsPoint( 0, 2 ) );
  cc.addCurve( ll1 );
  QgsLineString *ll2 = new QgsLineString();
  ll2->setPoints( QgsPointSequence() << QgsPoint( 0, 2 ) << QgsPoint( -1, 0 ) << QgsPoint( 0, -1 ) );
  cc.addCurve( ll2 );
  QgsLineString *ll3 = new QgsLineString();
  ll3->setPoints( QgsPointSequence() << QgsPoint( 0, -1 ) << QgsPoint( 1, 1 ) );
  cc.addCurve( ll3 );

  double ccArea = 0.0;
  cc.sumUpArea( ccArea );

  QgsLineString ls;
  ls.setPoints( QgsPointSequence() << QgsPoint( 1, 1 ) << QgsPoint( 0, 2 ) <<  QgsPoint( -1, 0 ) << QgsPoint( 0, -1 )
                << QgsPoint( 1, 1 ) );
  double lsArea = 0.0;
  ls.sumUpArea( lsArea );
  QGSCOMPARENEAR( ccArea, lsArea, 4 * std::numeric_limits<double>::epsilon() );


  //addVertex
  QgsCompoundCurve ac1;
  ac1.addVertex( QgsPoint( 1.0, 2.0 ) );
  QVERIFY( !ac1.isEmpty() );
  QCOMPARE( ac1.numPoints(), 1 );
  QCOMPARE( ac1.vertexCount(), 1 );
  QCOMPARE( ac1.nCoordinates(), 1 );
  QCOMPARE( ac1.ringCount(), 1 );
  QCOMPARE( ac1.partCount(), 1 );
  QVERIFY( !ac1.is3D() );
  QVERIFY( !ac1.isMeasure() );
  QCOMPARE( ac1.wkbType(), QgsWkbTypes::CompoundCurve );
  QVERIFY( !ac1.hasCurvedSegments() );
  QCOMPARE( ac1.area(), 0.0 );
  QCOMPARE( ac1.perimeter(), 0.0 );

  //adding first vertex should set linestring z/m type
  QgsCompoundCurve ac2;
  ac2.addVertex( QgsPoint( QgsWkbTypes::PointZ, 1.0, 2.0, 3.0 ) );
  QVERIFY( !ac2.isEmpty() );
  QVERIFY( ac2.is3D() );
  QVERIFY( !ac2.isMeasure() );
  QCOMPARE( ac2.wkbType(), QgsWkbTypes::CompoundCurveZ );
  QCOMPARE( ac2.wktTypeStr(), QString( "CompoundCurveZ" ) );

  QgsCompoundCurve ac3;
  ac3.addVertex( QgsPoint( QgsWkbTypes::PointM, 1.0, 2.0, 0.0, 3.0 ) );
  QVERIFY( !ac3.isEmpty() );
  QVERIFY( !ac3.is3D() );
  QVERIFY( ac3.isMeasure() );
  QCOMPARE( ac3.wkbType(), QgsWkbTypes::CompoundCurveM );
  QCOMPARE( ac3.wktTypeStr(), QString( "CompoundCurveM" ) );

  QgsCompoundCurve ac4;
  ac4.addVertex( QgsPoint( QgsWkbTypes::PointZM, 1.0, 2.0, 3.0, 4.0 ) );
  QVERIFY( !ac4.isEmpty() );
  QVERIFY( ac4.is3D() );
  QVERIFY( ac4.isMeasure() );
  QCOMPARE( ac4.wkbType(), QgsWkbTypes::CompoundCurveZM );
  QCOMPARE( ac4.wktTypeStr(), QString( "CompoundCurveZM" ) );

  //adding subsequent vertices should not alter z/m type, regardless of points type
  QgsCompoundCurve ac5;
  ac5.addVertex( QgsPoint( QgsWkbTypes::Point, 1.0, 2.0 ) ); //2d type
  QCOMPARE( ac5.wkbType(), QgsWkbTypes::CompoundCurve );
  ac5.addVertex( QgsPoint( QgsWkbTypes::PointZ, 11.0, 12.0, 13.0 ) ); // add 3d point
  QCOMPARE( ac5.numPoints(), 2 );
  QCOMPARE( ac5.vertexCount(), 2 );
  QCOMPARE( ac5.nCoordinates(), 2 );
  QCOMPARE( ac5.ringCount(), 1 );
  QCOMPARE( ac5.partCount(), 1 );
  QCOMPARE( ac5.wkbType(), QgsWkbTypes::CompoundCurve ); //should still be 2d
  QVERIFY( !ac5.is3D() );
  QCOMPARE( ac5.area(), 0.0 );
  QCOMPARE( ac5.perimeter(), 0.0 );

  QgsCompoundCurve ac6;
  ac6.addVertex( QgsPoint( QgsWkbTypes::PointZ, 1.0, 2.0, 3.0 ) ); //3d type
  QCOMPARE( ac6.wkbType(), QgsWkbTypes::CompoundCurveZ );
  ac6.addVertex( QgsPoint( QgsWkbTypes::Point, 11.0, 12.0 ) ); //add 2d point
  QCOMPARE( ac6.wkbType(), QgsWkbTypes::CompoundCurveZ ); //should still be 3d
  ac6.pointAt( 1, pt, v );
  QCOMPARE( pt, QgsPoint( QgsWkbTypes::PointZ, 11.0, 12.0 ) );
  QVERIFY( ac6.is3D() );
  QCOMPARE( ac6.numPoints(), 2 );
  QCOMPARE( ac6.vertexCount(), 2 );
  QCOMPARE( ac6.nCoordinates(), 2 );
  QCOMPARE( ac6.ringCount(), 1 );
  QCOMPARE( ac6.partCount(), 1 );

  //close
  QgsLineString closeC1;
  QgsCompoundCurve closeCc1;
  closeCc1.close();
  QVERIFY( closeCc1.isEmpty() );
  closeC1.setPoints( QgsPointSequence() << QgsPoint( 1, 2 ) << QgsPoint( 11, 12 ) << QgsPoint( 1, 22 ) );
  closeCc1.addCurve( closeC1.clone() );
  QCOMPARE( closeCc1.numPoints(), 3 );
  QVERIFY( !closeCc1.isClosed() );
  closeCc1.close();
  QCOMPARE( closeCc1.numPoints(), 4 );
  QVERIFY( closeCc1.isClosed() );
  closeCc1.pointAt( 3, pt, v );
  QCOMPARE( pt, QgsPoint( QgsWkbTypes::Point, 1, 2 ) );
  closeCc1.close();
  QCOMPARE( closeCc1.numPoints(), 4 );
  QVERIFY( closeCc1.isClosed() );

  //segmentLength
  QgsCircularString curveLine2;
  QgsCompoundCurve slc1;
  QCOMPARE( slc1.segmentLength( QgsVertexId( -1, 0, 0 ) ), 0.0 );
  QCOMPARE( slc1.segmentLength( QgsVertexId( 0, 0, 0 ) ), 0.0 );
  QCOMPARE( slc1.segmentLength( QgsVertexId( 1, 0, 0 ) ), 0.0 );
  curveLine2.setPoints( QgsPointSequence() << QgsPoint( 1, 2 ) << QgsPoint( 11, 12 ) << QgsPoint( 1, 22 ) );
  slc1.addCurve( curveLine2.clone() );
  QCOMPARE( slc1.segmentLength( QgsVertexId() ), 0.0 );
  QCOMPARE( slc1.segmentLength( QgsVertexId( 0, 0, -1 ) ), 0.0 );
  QGSCOMPARENEAR( slc1.segmentLength( QgsVertexId( 0, 0, 0 ) ), 31.4159, 0.001 );
  QCOMPARE( slc1.segmentLength( QgsVertexId( 0, 0, 1 ) ), 0.0 );
  QCOMPARE( slc1.segmentLength( QgsVertexId( 0, 0, 2 ) ), 0.0 );
  QCOMPARE( slc1.segmentLength( QgsVertexId( -1, 0, -1 ) ), 0.0 );
  QGSCOMPARENEAR( slc1.segmentLength( QgsVertexId( -1, 0, 0 ) ), 31.4159, 0.001 );
  QCOMPARE( slc1.segmentLength( QgsVertexId( 1, 0, 1 ) ), 0.0 );
  QGSCOMPARENEAR( slc1.segmentLength( QgsVertexId( 1, 1, 0 ) ), 31.4159, 0.001 );
  curveLine2.setPoints( QgsPointSequence() << QgsPoint( 1, 22 ) << QgsPoint( -9, 32 ) << QgsPoint( 1, 42 ) );
  slc1.addCurve( curveLine2.clone() );
  QCOMPARE( slc1.segmentLength( QgsVertexId() ), 0.0 );
  QCOMPARE( slc1.segmentLength( QgsVertexId( 0, 0, -1 ) ), 0.0 );
  QGSCOMPARENEAR( slc1.segmentLength( QgsVertexId( 0, 0, 0 ) ), 31.4159, 0.001 );
  QCOMPARE( slc1.segmentLength( QgsVertexId( 0, 0, 1 ) ), 0.0 );
  QGSCOMPARENEAR( slc1.segmentLength( QgsVertexId( 0, 0, 2 ) ), 31.4159, 0.001 );
  QCOMPARE( slc1.segmentLength( QgsVertexId( 0, 0, 3 ) ), 0.0 );
  QCOMPARE( slc1.segmentLength( QgsVertexId( 0, 0, 4 ) ), 0.0 );
  QgsLineString curveLine3;
  curveLine3.setPoints( QgsPointSequence() << QgsPoint( 1, 42 ) << QgsPoint( 10, 42 ) );
  slc1.addCurve( curveLine3.clone() );
  QCOMPARE( slc1.segmentLength( QgsVertexId() ), 0.0 );
  QCOMPARE( slc1.segmentLength( QgsVertexId( 0, 0, -1 ) ), 0.0 );
  QGSCOMPARENEAR( slc1.segmentLength( QgsVertexId( 0, 0, 0 ) ), 31.4159, 0.001 );
  QCOMPARE( slc1.segmentLength( QgsVertexId( 0, 0, 1 ) ), 0.0 );
  QGSCOMPARENEAR( slc1.segmentLength( QgsVertexId( 0, 0, 2 ) ), 31.4159, 0.001 );
  QCOMPARE( slc1.segmentLength( QgsVertexId( 0, 0, 3 ) ), 0.0 );
  QCOMPARE( slc1.segmentLength( QgsVertexId( 0, 0, 4 ) ), 9.0 );
  QCOMPARE( slc1.segmentLength( QgsVertexId( 0, 0, 5 ) ), 0.0 );

  //removeDuplicateNodes
  QgsCompoundCurve nodeCurve;
  QgsCircularString nodeLine;
  QVERIFY( !nodeCurve.removeDuplicateNodes() );
  nodeLine.setPoints( QgsPointSequence() << QgsPoint( 11, 2 ) << QgsPoint( 11, 12 ) << QgsPoint( 111, 12 ) );
  nodeCurve.addCurve( nodeLine.clone() );
  QVERIFY( !nodeCurve.removeDuplicateNodes() );
  QCOMPARE( nodeCurve.asWkt(), QStringLiteral( "CompoundCurve (CircularString (11 2, 11 12, 111 12))" ) );
  nodeLine.setPoints( QgsPointSequence() << QgsPoint( 11, 2 ) << QgsPoint( 11, 12 ) << QgsPoint( 11, 2 ) );
  nodeCurve.clear();
  nodeCurve.addCurve( nodeLine.clone() );
  QVERIFY( !nodeCurve.removeDuplicateNodes() );
  QCOMPARE( nodeCurve.asWkt(), QStringLiteral( "CompoundCurve (CircularString (11 2, 11 12, 11 2))" ) );
  nodeLine.setPoints( QgsPointSequence() << QgsPoint( 11, 2 ) << QgsPoint( 10, 3 ) << QgsPoint( 11.01, 1.99 ) << QgsPoint( 9, 3 )
                      << QgsPoint( 11, 2 ) );
  nodeCurve.clear();
  nodeCurve.addCurve( nodeLine.clone() );
  QVERIFY( !nodeCurve.removeDuplicateNodes( 0.02 ) );
  QCOMPARE( nodeCurve.asWkt( 2 ), QStringLiteral( "CompoundCurve (CircularString (11 2, 10 3, 11.01 1.99, 9 3, 11 2))" ) );
  nodeLine.setPoints( QgsPointSequence() << QgsPoint( 11, 2 ) << QgsPoint( 11.01, 1.99 ) << QgsPoint( 11.02, 2.01 )
                      << QgsPoint( 11, 12 ) << QgsPoint( 111, 12 ) << QgsPoint( 111.01, 11.99 ) );
  nodeCurve.clear();
  nodeCurve.addCurve( nodeLine.clone() );
  QVERIFY( !nodeCurve.removeDuplicateNodes() );
  QCOMPARE( nodeCurve.asWkt( 2 ), QStringLiteral( "CompoundCurve (CircularString (11 2, 11.01 1.99, 11.02 2.01, 11 12, 111 12, 111.01 11.99))" ) );
  QVERIFY( nodeCurve.removeDuplicateNodes( 0.02 ) );
  QCOMPARE( nodeCurve.asWkt( 2 ), QStringLiteral( "CompoundCurve (CircularString (11 2, 11 12, 111 12, 111.01 11.99))" ) );

  // with tiny segment
  QgsLineString linePart;
  linePart.setPoints( QgsPointSequence() << QgsPoint( 111.01, 11.99 ) << QgsPoint( 111, 12 ) );
  nodeCurve.addCurve( linePart.clone() );
  QVERIFY( !nodeCurve.removeDuplicateNodes() );
  QCOMPARE( nodeCurve.asWkt( 2 ), QStringLiteral( "CompoundCurve (CircularString (11 2, 11 12, 111 12, 111.01 11.99),(111.01 11.99, 111 12))" ) );
  QVERIFY( nodeCurve.removeDuplicateNodes( 0.02 ) );
  QCOMPARE( nodeCurve.asWkt( 2 ), QStringLiteral( "CompoundCurve (CircularString (11 2, 11 12, 111 12, 111.01 11.99))" ) );

  // ensure continuity
  nodeCurve.clear();
  linePart.setPoints( QgsPointSequence() << QgsPoint( 1, 1 ) << QgsPoint( 111.01, 11.99 ) << QgsPoint( 111, 12 ) );
  nodeCurve.addCurve( linePart.clone() );
  linePart.setPoints( QgsPointSequence() << QgsPoint( 111, 12 ) << QgsPoint( 31, 33 ) );
  nodeCurve.addCurve( linePart.clone() );
  QVERIFY( nodeCurve.removeDuplicateNodes( 0.02 ) );
  QCOMPARE( nodeCurve.asWkt( 2 ), QStringLiteral( "CompoundCurve ((1 1, 111.01 11.99),(111.01 11.99, 31 33))" ) );

  // swap xy
  QgsCompoundCurve swapCurve;
  swapCurve.swapXy(); //no crash
  nodeLine.setPoints( QgsPointSequence() << QgsPoint( 11, 2, 3, 4, QgsWkbTypes::PointZM ) << QgsPoint( 11, 12, 13, 14, QgsWkbTypes::PointZM ) << QgsPoint( 111, 12, 23, 24, QgsWkbTypes::PointZM ) );
  swapCurve.addCurve( nodeLine.clone() );
  swapCurve.swapXy();
  QCOMPARE( swapCurve.asWkt(), QStringLiteral( "CompoundCurveZM (CircularStringZM (2 11 3 4, 12 11 13 14, 12 111 23 24))" ) );
  QgsLineString lsSwap;
  lsSwap.setPoints( QgsPointSequence() << QgsPoint( 12, 111, 23, 24, QgsWkbTypes::PointZM ) << QgsPoint( 22, 122, 33, 34, QgsWkbTypes::PointZM ) );
  swapCurve.addCurve( lsSwap.clone() );
  swapCurve.swapXy();
  QCOMPARE( swapCurve.asWkt(), QStringLiteral( "CompoundCurveZM (CircularStringZM (11 2 3 4, 11 12 13 14, 111 12 23 24),(111 12 23 24, 122 22 33 34))" ) );

  // filter vertices
  auto filter = []( const QgsPoint & point )-> bool
  {
    return point.x() > 5;
  };
  QgsCompoundCurve filterCurve;
  filterCurve.filterVertices( filter ); //no crash
  nodeLine.setPoints( QgsPointSequence()   << QgsPoint( 1, 2, 3, 4, QgsWkbTypes::PointZM ) << QgsPoint( 11, 2, 3, 4, QgsWkbTypes::PointZM ) << QgsPoint( 11, 12, 13, 14, QgsWkbTypes::PointZM ) << QgsPoint( 111, 12, 23, 24, QgsWkbTypes::PointZM )   << QgsPoint( 1, 2, 3, 4, QgsWkbTypes::PointZM ) );
  filterCurve.addCurve( nodeLine.clone() );
  filterCurve.filterVertices( filter );
  QCOMPARE( filterCurve.asWkt(), QStringLiteral( "CompoundCurveZM (CircularStringZM (11 2 3 4, 11 12 13 14, 111 12 23 24))" ) );
  QgsLineString lsFilter;
  lsFilter.setPoints( QgsPointSequence() << QgsPoint( 12, 111, 23, 24, QgsWkbTypes::PointZM ) << QgsPoint( 22, 122, 33, 34, QgsWkbTypes::PointZM ) << QgsPoint( 1, 111, 23, 24, QgsWkbTypes::PointZM ) );
  filterCurve.addCurve( lsFilter.clone() );
  filterCurve.filterVertices( filter );
  QCOMPARE( filterCurve.asWkt(), QStringLiteral( "CompoundCurveZM (CircularStringZM (11 2 3 4, 11 12 13 14, 111 12 23 24),(12 111 23 24, 22 122 33 34))" ) );

  // transform vertices
  auto transform = []( const QgsPoint & point )-> QgsPoint
  {
    return QgsPoint( point.x() + 2, point.y() + 3, point.z() + 4, point.m() + 5 );
  };
  QgsCompoundCurve transformCurve;
  transformCurve.transformVertices( transform ); //no crash
  nodeLine.setPoints( QgsPointSequence()   << QgsPoint( 1, 2, 3, 4, QgsWkbTypes::PointZM ) << QgsPoint( 11, 2, 3, 4, QgsWkbTypes::PointZM ) << QgsPoint( 11, 12, 13, 14, QgsWkbTypes::PointZM ) << QgsPoint( 111, 12, 23, 24, QgsWkbTypes::PointZM )   << QgsPoint( 1, 2, 3, 4, QgsWkbTypes::PointZM ) );
  transformCurve.addCurve( nodeLine.clone() );
  transformCurve.transformVertices( transform );
  QCOMPARE( transformCurve.asWkt(), QStringLiteral( "CompoundCurveZM (CircularStringZM (3 5 7 9, 13 5 7 9, 13 15 17 19, 113 15 27 29, 3 5 7 9))" ) );
  QgsLineString lsTransform;
  lsTransform.setPoints( QgsPointSequence() << QgsPoint( 12, 111, 23, 24, QgsWkbTypes::PointZM ) << QgsPoint( 22, 122, 33, 34, QgsWkbTypes::PointZM ) << QgsPoint( 1, 111, 23, 24, QgsWkbTypes::PointZM ) );
  transformCurve.addCurve( lsTransform.clone() );
  transformCurve.transformVertices( transform );
  QCOMPARE( transformCurve.asWkt(), QStringLiteral( "CompoundCurveZM (CircularStringZM (5 8 11 14, 15 8 11 14, 15 18 21 24, 115 18 31 34, 5 8 11 14),(14 114 27 29, 24 125 37 39, 3 114 27 29))" ) );

  // substring
  QgsCompoundCurve substring;
  std::unique_ptr< QgsCompoundCurve > substringResult( substring.curveSubstring( 1, 2 ) ); // no crash
  QVERIFY( substringResult.get() );
  QVERIFY( substringResult->isEmpty() );
  substring.fromWkt( QStringLiteral( "CompoundCurveZM( ( 5 0 -1 -2, 10 0 1 2 ), CircularStringZM (10 0 1 2, 11 1 3 4, 12 0 13 14))" ) );
  substringResult.reset( substring.curveSubstring( 0, 0 ) );
  QCOMPARE( substringResult->asWkt( 2 ), QStringLiteral( "CompoundCurveZM ((5 0 -1 -2, 5 0 -1 -2))" ) );
  substringResult.reset( substring.curveSubstring( -1, -0.1 ) );
  QVERIFY( substringResult->isEmpty() );
  substringResult.reset( substring.curveSubstring( 100000, 10000 ) );
  QVERIFY( substringResult->isEmpty() );
  substringResult.reset( substring.curveSubstring( -1, 1 ) );
  QCOMPARE( substringResult->asWkt( 2 ), QStringLiteral( "CompoundCurveZM ((5 0 -1 -2, 6 0 -0.6 -1.2))" ) );
  substringResult.reset( substring.curveSubstring( 1, -1 ) );
  QCOMPARE( substringResult->asWkt( 2 ), QStringLiteral( "CompoundCurveZM ((6 0 -0.6 -1.2, 6 0 -0.6 -1.2))" ) );
  substringResult.reset( substring.curveSubstring( -1, 10000 ) );
  QCOMPARE( substringResult->asWkt( 2 ), QStringLiteral( "CompoundCurveZM ((5 0 -1 -2, 10 0 1 2),CircularStringZM (10 0 1 2, 11 1 3 4, 12 0 13 14))" ) );
  substringResult.reset( substring.curveSubstring( 1, 10000 ) );
  QCOMPARE( substringResult->asWkt( 2 ), QStringLiteral( "CompoundCurveZM ((6 0 -0.6 -1.2, 10 0 1 2),CircularStringZM (10 0 1 2, 11 1 3 4, 12 0 13 14))" ) );
  substringResult.reset( substring.curveSubstring( 1, 7 ) );
  QCOMPARE( substringResult->asWkt( 2 ), QStringLiteral( "CompoundCurveZM ((6 0 -0.6 -1.2, 10 0 1 2),CircularStringZM (10 0 1 2, 10.46 0.84 2.27 3.27, 11.42 0.91 5.73 6.73))" ) );
  substringResult.reset( substring.curveSubstring( 1, 1.5 ) );
  QCOMPARE( substringResult->asWkt( 2 ), QStringLiteral( "CompoundCurveZM ((6 0 -0.6 -1.2, 6.5 0 -0.4 -0.8))" ) );

  substring.fromWkt( QStringLiteral( "CompoundCurveZ( ( 5 0 -1, 10 0 1 ), CircularStringZ (10 0 1, 11 1 3, 12 0 13))" ) );
  substringResult.reset( substring.curveSubstring( 1, 7 ) );
  QCOMPARE( substringResult->asWkt( 2 ), QStringLiteral( "CompoundCurveZ ((6 0 -0.6, 10 0 1),CircularStringZ (10 0 1, 10.46 0.84 2.27, 11.42 0.91 5.73))" ) );
  substring.fromWkt( QStringLiteral( "CompoundCurveM( ( 5 0 -1, 10 0 1 ), CircularStringM (10 0 1, 11 1 3, 12 0 13))" ) );
  substringResult.reset( substring.curveSubstring( 1, 7 ) );
  QCOMPARE( substringResult->asWkt( 2 ), QStringLiteral( "CompoundCurveM ((6 0 -0.6, 10 0 1),CircularStringM (10 0 1, 10.46 0.84 2.27, 11.42 0.91 5.73))" ) );
  substring.fromWkt( QStringLiteral( "CompoundCurve( ( 5 0, 10 0 ), CircularString (10 0, 11 1, 12 0))" ) );
  substringResult.reset( substring.curveSubstring( 1, 7 ) );
  QCOMPARE( substringResult->asWkt( 2 ), QStringLiteral( "CompoundCurve ((6 0, 10 0),CircularString (10 0, 10.46 0.84, 11.42 0.91))" ) );

  // substring
  QgsCompoundCurve interpolate;
  std::unique_ptr< QgsPoint > interpolateResult( interpolate.interpolatePoint( 1 ) ); // no crash
  QVERIFY( !interpolateResult.get() );
  interpolate.fromWkt( QStringLiteral( "CompoundCurveZM( ( 5 0 -1 -2, 10 0 1 2 ), CircularStringZM (10 0 1 2, 11 1 3 4, 12 0 13 14))" ) );
  interpolateResult.reset( interpolate.interpolatePoint( 0 ) );
  QCOMPARE( interpolateResult->asWkt( 2 ), QStringLiteral( "PointZM (5 0 -1 -2)" ) );
  interpolateResult.reset( interpolate.interpolatePoint( -1 ) );
  QVERIFY( !interpolateResult.get() );
  interpolateResult.reset( interpolate.interpolatePoint( 100000 ) );
  QVERIFY( !interpolateResult.get() );
  interpolateResult.reset( interpolate.interpolatePoint( 1 ) );
  QCOMPARE( interpolateResult->asWkt( 2 ), QStringLiteral( "PointZM (6 0 -0.6 -1.2)" ) );
  interpolateResult.reset( interpolate.interpolatePoint( 7 ) );
  QCOMPARE( interpolateResult->asWkt( 2 ), QStringLiteral( "PointZM (11.42 0.91 5.73 6.73)" ) );
  interpolateResult.reset( interpolate.interpolatePoint( 1.5 ) );
  QCOMPARE( interpolateResult->asWkt( 2 ), QStringLiteral( "PointZM (6.5 0 -0.4 -0.8)" ) );
  interpolateResult.reset( interpolate.interpolatePoint( interpolate.length() ) );
  QCOMPARE( interpolateResult->asWkt( 2 ), QStringLiteral( "PointZM (12 0 13 14)" ) );

  interpolate.fromWkt( QStringLiteral( "CompoundCurveZ( ( 5 0 -1, 10 0 1 ), CircularStringZ (10 0 1, 11 1 3, 12 0 13))" ) );
  interpolateResult.reset( interpolate.interpolatePoint( 1 ) );
  QCOMPARE( interpolateResult->asWkt( 2 ), QStringLiteral( "PointZ (6 0 -0.6)" ) );
  interpolate.fromWkt( QStringLiteral( "CompoundCurveM( ( 5 0 -1, 10 0 1 ), CircularStringM (10 0 1, 11 1 3, 12 0 13))" ) );
  interpolateResult.reset( interpolate.interpolatePoint( 1 ) );
  QCOMPARE( interpolateResult->asWkt( 2 ), QStringLiteral( "PointM (6 0 -0.6)" ) );
  interpolate.fromWkt( QStringLiteral( "CompoundCurve( ( 5 0, 10 0 ), CircularString (10 0, 11 1, 12 0))" ) );
  interpolateResult.reset( interpolate.interpolatePoint( 1 ) );
  QCOMPARE( interpolateResult->asWkt( 2 ), QStringLiteral( "Point (6 0)" ) );

  // orientation
  QgsCompoundCurve orientation;
  ( void )orientation.orientation(); // no crash
  orientation.fromWkt( QStringLiteral( "CompoundCurve( ( 0 0, 0 1), CircularString (0 1, 1 1, 1 0), (1 0, 0 0))" ) );
  QCOMPARE( orientation.orientation(), QgsCurve::Clockwise );
  orientation.fromWkt( QStringLiteral( "CompoundCurve( ( 0 0, 1 0), CircularString (1 0, 1 1, 0 1), (0 1, 0 0))" ) );
  QCOMPARE( orientation.orientation(), QgsCurve::CounterClockwise );
}

void TestQgsGeometry::multiPoint()
{
  //test constructor
  QgsMultiPoint c1;
  QVERIFY( c1.isEmpty() );
  QCOMPARE( c1.nCoordinates(), 0 );
  QCOMPARE( c1.ringCount(), 0 );
  QCOMPARE( c1.partCount(), 0 );
  QVERIFY( !c1.is3D() );
  QVERIFY( !c1.isMeasure() );
  QCOMPARE( c1.wkbType(), QgsWkbTypes::MultiPoint );
  QCOMPARE( c1.wktTypeStr(), QString( "MultiPoint" ) );
  QCOMPARE( c1.geometryType(), QString( "MultiPoint" ) );
  QCOMPARE( c1.dimension(), 0 );
  QVERIFY( !c1.hasCurvedSegments() );
  QCOMPARE( c1.area(), 0.0 );
  QCOMPARE( c1.perimeter(), 0.0 );
  QCOMPARE( c1.numGeometries(), 0 );
  QVERIFY( !c1.geometryN( 0 ) );
  QVERIFY( !c1.geometryN( -1 ) );
  QCOMPARE( c1.vertexCount( 0, 0 ), 0 );
  QCOMPARE( c1.vertexCount( 0, 1 ), 0 );
  QCOMPARE( c1.vertexCount( 1, 0 ), 0 );

  //addGeometry

  //try with nullptr
  c1.addGeometry( nullptr );
  QVERIFY( c1.isEmpty() );
  QCOMPARE( c1.nCoordinates(), 0 );
  QCOMPARE( c1.ringCount(), 0 );
  QCOMPARE( c1.partCount(), 0 );
  QCOMPARE( c1.numGeometries(), 0 );
  QCOMPARE( c1.wkbType(), QgsWkbTypes::MultiPoint );
  QVERIFY( !c1.geometryN( 0 ) );
  QVERIFY( !c1.geometryN( -1 ) );

  // not a point
  QVERIFY( !c1.addGeometry( new QgsLineString() ) );
  QVERIFY( c1.isEmpty() );
  QCOMPARE( c1.nCoordinates(), 0 );
  QCOMPARE( c1.ringCount(), 0 );
  QCOMPARE( c1.partCount(), 0 );
  QCOMPARE( c1.numGeometries(), 0 );
  QCOMPARE( c1.wkbType(), QgsWkbTypes::MultiPoint );
  QVERIFY( !c1.geometryN( 0 ) );
  QVERIFY( !c1.geometryN( -1 ) );

  //valid geometry
  QgsPoint part( 1, 10 );
  c1.addGeometry( part.clone() );
  QVERIFY( !c1.isEmpty() );
  QCOMPARE( c1.numGeometries(), 1 );
  QCOMPARE( c1.nCoordinates(), 1 );
  QCOMPARE( c1.ringCount(), 1 );
  QCOMPARE( c1.partCount(), 1 );
  QVERIFY( !c1.is3D() );
  QVERIFY( !c1.isMeasure() );
  QCOMPARE( c1.wkbType(), QgsWkbTypes::MultiPoint );
  QCOMPARE( c1.wktTypeStr(), QString( "MultiPoint" ) );
  QCOMPARE( c1.geometryType(), QString( "MultiPoint" ) );
  QCOMPARE( c1.dimension(), 0 );
  QVERIFY( !c1.hasCurvedSegments() );
  QCOMPARE( c1.area(), 0.0 );
  QCOMPARE( c1.perimeter(), 0.0 );
  QVERIFY( c1.geometryN( 0 ) );
  QCOMPARE( *static_cast< const QgsPoint * >( c1.geometryN( 0 ) ), part );
  QVERIFY( !c1.geometryN( 100 ) );
  QVERIFY( !c1.geometryN( -1 ) );
  QCOMPARE( c1.vertexCount( 0, 0 ), 1 );
  QCOMPARE( c1.vertexCount( 1, 0 ), 0 );

  //initial adding of geometry should set z/m type
  part = QgsPoint( QgsWkbTypes::PointZ, 10, 11, 1 );
  QgsMultiPoint c2;
  c2.addGeometry( part.clone() );
  QVERIFY( c2.is3D() );
  QVERIFY( !c2.isMeasure() );
  QCOMPARE( c2.wkbType(), QgsWkbTypes::MultiPointZ );
  QCOMPARE( c2.wktTypeStr(), QString( "MultiPointZ" ) );
  QCOMPARE( c2.geometryType(), QString( "MultiPoint" ) );
  QCOMPARE( *( static_cast< const QgsPoint * >( c2.geometryN( 0 ) ) ), part );
  QgsMultiPoint c3;
  part = QgsPoint( QgsWkbTypes::PointM, 10, 10, 0, 3 );
  c3.addGeometry( part.clone() );
  QVERIFY( !c3.is3D() );
  QVERIFY( c3.isMeasure() );
  QCOMPARE( c3.wkbType(), QgsWkbTypes::MultiPointM );
  QCOMPARE( c3.wktTypeStr(), QString( "MultiPointM" ) );
  QCOMPARE( *( static_cast< const QgsPoint * >( c3.geometryN( 0 ) ) ), part );
  QgsMultiPoint c4;
  part = QgsPoint( QgsWkbTypes::PointZM, 10, 10, 5, 3 );
  c4.addGeometry( part.clone() );
  QVERIFY( c4.is3D() );
  QVERIFY( c4.isMeasure() );
  QCOMPARE( c4.wkbType(), QgsWkbTypes::MultiPointZM );
  QCOMPARE( c4.wktTypeStr(), QString( "MultiPointZM" ) );
  QCOMPARE( *( static_cast< const QgsPoint * >( c4.geometryN( 0 ) ) ), part );

  //add another part
  QgsMultiPoint c6;
  part = QgsPoint( 10, 11 );
  c6.addGeometry( part.clone() );
  QCOMPARE( c6.vertexCount( 0, 0 ), 1 );
  part = QgsPoint( 9, 1 );
  c6.addGeometry( part.clone() );
  QCOMPARE( c6.vertexCount( 1, 0 ), 1 );
  QCOMPARE( c6.numGeometries(), 2 );
  QVERIFY( c6.geometryN( 0 ) );
  QCOMPARE( *static_cast< const QgsPoint * >( c6.geometryN( 1 ) ), part );

  QgsCoordinateSequence seq = c6.coordinateSequence();
  QCOMPARE( seq, QgsCoordinateSequence() << ( QgsRingSequence() << ( QgsPointSequence() << QgsPoint( 10, 11 ) ) )
            << ( QgsRingSequence() << ( QgsPointSequence() << QgsPoint( 9, 1 ) ) ) );
  QCOMPARE( c6.nCoordinates(), 2 );

  //adding subsequent points should not alter z/m type, regardless of points type
  c6.clear();
  c6.addGeometry( part.clone() );
  QCOMPARE( c6.wkbType(), QgsWkbTypes::MultiPoint );
  c6.addGeometry( new QgsPoint( QgsWkbTypes::PointZ, 1.0, 2.0, 3 ) );
  QCOMPARE( c6.wkbType(), QgsWkbTypes::MultiPoint );
  QCOMPARE( c6.vertexCount( 0, 0 ), 1 );
  QCOMPARE( c6.vertexCount( 1, 0 ), 1 );
  QCOMPARE( c6.vertexCount( 2, 0 ), 0 );
  QCOMPARE( c6.vertexCount( -1, 0 ), 0 );
  QCOMPARE( c6.nCoordinates(), 2 );
  QCOMPARE( c6.ringCount(), 1 );
  QCOMPARE( c6.partCount(), 2 );
  QCOMPARE( c6.wkbType(), QgsWkbTypes::MultiPoint ); //should still be 2d
  QVERIFY( !c6.is3D() );
  QCOMPARE( *( static_cast< const QgsPoint * >( c6.geometryN( 1 ) ) ), QgsPoint( 1, 2 ) );
  c6.addGeometry( new QgsPoint( QgsWkbTypes::PointM, 11.0, 12.0, 0, 3 ) );
  QCOMPARE( c6.wkbType(), QgsWkbTypes::MultiPoint );
  QCOMPARE( c6.vertexCount( 0, 0 ), 1 );
  QCOMPARE( c6.vertexCount( 1, 0 ), 1 );
  QCOMPARE( c6.vertexCount( 2, 0 ), 1 );
  QCOMPARE( c6.nCoordinates(), 3 );
  QCOMPARE( c6.ringCount(), 1 );
  QCOMPARE( c6.partCount(), 3 );
  QCOMPARE( c6.wkbType(), QgsWkbTypes::MultiPoint ); //should still be 2d
  QVERIFY( !c6.is3D() );
  QVERIFY( !c6.isMeasure() );
  QCOMPARE( *( static_cast< const QgsPoint * >( c6.geometryN( 2 ) ) ), QgsPoint( 11, 12 ) );

  c6.clear();
  c6.addGeometry( new QgsPoint( QgsWkbTypes::PointZ, 1.0, 2.0, 3 ) );
  QCOMPARE( c6.wkbType(), QgsWkbTypes::MultiPointZ );
  c6.addGeometry( new QgsPoint( QgsWkbTypes::Point, 11.0, 12.0 ) );
  QCOMPARE( c6.wkbType(), QgsWkbTypes::MultiPointZ );
  QVERIFY( c6.is3D() );
  QCOMPARE( *( static_cast< const QgsPoint * >( c6.geometryN( 0 ) ) ), QgsPoint( QgsWkbTypes::PointZ, 1, 2, 3 ) );
  QCOMPARE( *( static_cast< const QgsPoint * >( c6.geometryN( 1 ) ) ), QgsPoint( QgsWkbTypes::PointZ, 11, 12, 0 ) );
  c6.addGeometry( new QgsPoint( QgsWkbTypes::PointM, 21.0, 22.0, 0, 3 ) );
  QCOMPARE( c6.wkbType(), QgsWkbTypes::MultiPointZ );
  QVERIFY( c6.is3D() );
  QVERIFY( !c6.isMeasure() );
  QCOMPARE( *( static_cast< const QgsPoint * >( c6.geometryN( 2 ) ) ), QgsPoint( QgsWkbTypes::PointZ, 21, 22, 0 ) );

  c6.clear();
  c6.addGeometry( new QgsPoint( QgsWkbTypes::PointM, 1.0, 2.0, 0, 3 ) );
  QCOMPARE( c6.wkbType(), QgsWkbTypes::MultiPointM );
  c6.addGeometry( new QgsPoint( QgsWkbTypes::Point, 11.0, 12.0 ) );
  QCOMPARE( c6.wkbType(), QgsWkbTypes::MultiPointM );
  QVERIFY( c6.isMeasure() );
  QCOMPARE( *( static_cast< const QgsPoint * >( c6.geometryN( 0 ) ) ), QgsPoint( QgsWkbTypes::PointM, 1, 2, 0, 3 ) );
  QCOMPARE( *( static_cast< const QgsPoint * >( c6.geometryN( 1 ) ) ), QgsPoint( QgsWkbTypes::PointM, 11, 12, 0, 0 ) );
  c6.addGeometry( new QgsPoint( QgsWkbTypes::PointZ, 21.0, 22.0, 3 ) );
  QCOMPARE( c6.wkbType(), QgsWkbTypes::MultiPointM );
  QVERIFY( !c6.is3D() );
  QVERIFY( c6.isMeasure() );
  QCOMPARE( *( static_cast< const QgsPoint * >( c6.geometryN( 2 ) ) ), QgsPoint( QgsWkbTypes::PointM, 21, 22, 0, 0 ) );

  c6.clear();
  c6.addGeometry( new QgsPoint( QgsWkbTypes::PointZM, 1.0, 2.0, 4, 3 ) );
  QCOMPARE( c6.wkbType(), QgsWkbTypes::MultiPointZM );
  c6.addGeometry( new QgsPoint( QgsWkbTypes::Point, 11.0, 12.0 ) );
  QCOMPARE( c6.wkbType(), QgsWkbTypes::MultiPointZM );
  QVERIFY( c6.isMeasure() );
  QVERIFY( c6.is3D() );
  QCOMPARE( *( static_cast< const QgsPoint * >( c6.geometryN( 0 ) ) ), QgsPoint( QgsWkbTypes::PointZM, 1, 2, 4, 3 ) );
  QCOMPARE( *( static_cast< const QgsPoint * >( c6.geometryN( 1 ) ) ), QgsPoint( QgsWkbTypes::PointZM, 11, 12, 0, 0 ) );
  c6.addGeometry( new QgsPoint( QgsWkbTypes::PointZ, 21.0, 22.0, 3 ) );
  QCOMPARE( c6.wkbType(), QgsWkbTypes::MultiPointZM );
  QVERIFY( c6.is3D() );
  QVERIFY( c6.isMeasure() );
  QCOMPARE( *( static_cast< const QgsPoint * >( c6.geometryN( 2 ) ) ), QgsPoint( QgsWkbTypes::PointZM, 21, 22, 3, 0 ) );
  c6.addGeometry( new QgsPoint( QgsWkbTypes::PointM, 31.0, 32.0, 0, 4 ) );
  QCOMPARE( c6.wkbType(), QgsWkbTypes::MultiPointZM );
  QVERIFY( c6.is3D() );
  QVERIFY( c6.isMeasure() );
  QCOMPARE( *( static_cast< const QgsPoint * >( c6.geometryN( 3 ) ) ), QgsPoint( QgsWkbTypes::PointZM, 31, 32, 0, 4 ) );

  //clear
  QgsMultiPoint c7;
  c7.addGeometry( new  QgsPoint( QgsWkbTypes::PointZ, 0, 10, 2 ) );
  c7.addGeometry( new  QgsPoint( QgsWkbTypes::PointZ, 11, 12, 3 ) );
  QCOMPARE( c7.numGeometries(), 2 );
  c7.clear();
  QVERIFY( c7.isEmpty() );
  QCOMPARE( c7.numGeometries(), 0 );
  QCOMPARE( c7.nCoordinates(), 0 );
  QCOMPARE( c7.ringCount(), 0 );
  QCOMPARE( c7.partCount(), 0 );
  QVERIFY( !c7.is3D() );
  QVERIFY( !c7.isMeasure() );
  QCOMPARE( c7.wkbType(), QgsWkbTypes::MultiPoint );

  //clone
  QgsMultiPoint c11;
  std::unique_ptr< QgsMultiPoint >cloned( c11.clone() );
  QVERIFY( cloned->isEmpty() );
  c11.addGeometry( new QgsPoint( QgsWkbTypes::PointZM, 0, 0, 1, 5 ) );
  c11.addGeometry( new QgsPoint( QgsWkbTypes::PointZM, 1, 2, 3, 4 ) );
  cloned.reset( c11.clone() );
  QCOMPARE( cloned->numGeometries(), 2 );
  QCOMPARE( *static_cast< const QgsPoint * >( cloned->geometryN( 0 ) ), QgsPoint( QgsWkbTypes::PointZM, 0, 0, 1, 5 ) );
  QCOMPARE( *static_cast< const QgsPoint * >( cloned->geometryN( 1 ) ), QgsPoint( QgsWkbTypes::PointZM, 1, 2, 3, 4 ) );

  //copy constructor
  QgsMultiPoint c12;
  QgsMultiPoint c13( c12 );
  QVERIFY( c13.isEmpty() );
  c12.addGeometry( new QgsPoint( QgsWkbTypes::PointZM, 10, 0, 4, 8 ) );
  c12.addGeometry( new QgsPoint( QgsWkbTypes::PointZM, 20, 10, 14, 18 ) );
  QgsMultiPoint c14( c12 );
  QCOMPARE( c14.numGeometries(), 2 );
  QCOMPARE( c14.wkbType(), QgsWkbTypes::MultiPointZM );
  QCOMPARE( *static_cast< const QgsPoint * >( c14.geometryN( 0 ) ), QgsPoint( QgsWkbTypes::PointZM, 10, 0, 4, 8 ) );
  QCOMPARE( *static_cast< const QgsPoint * >( c14.geometryN( 1 ) ), QgsPoint( QgsWkbTypes::PointZM, 20, 10, 14, 18 ) );

  //assignment operator
  QgsMultiPoint c15;
  c15 = c13;
  QCOMPARE( c15.numGeometries(), 0 );
  c15 = c14;
  QCOMPARE( c15.numGeometries(), 2 );
  QCOMPARE( *static_cast< const QgsPoint * >( c15.geometryN( 0 ) ), QgsPoint( QgsWkbTypes::PointZM, 10, 0, 4, 8 ) );
  QCOMPARE( *static_cast< const QgsPoint * >( c15.geometryN( 1 ) ), QgsPoint( QgsWkbTypes::PointZM, 20, 10, 14, 18 ) );

  //toCurveType
  std::unique_ptr< QgsMultiPoint > curveType( c12.toCurveType() );
  QCOMPARE( curveType->wkbType(), QgsWkbTypes::MultiPointZM );
  QCOMPARE( curveType->numGeometries(), 2 );
  const QgsPoint *curve = static_cast< const QgsPoint * >( curveType->geometryN( 0 ) );
  QCOMPARE( *curve, QgsPoint( QgsWkbTypes::PointZM, 10, 0, 4, 8 ) );
  curve = static_cast< const QgsPoint * >( curveType->geometryN( 1 ) );
  QCOMPARE( *curve, QgsPoint( QgsWkbTypes::PointZM, 20, 10, 14, 18 ) );

  //to/fromWKB
  QgsMultiPoint c16;
  c16.addGeometry( new QgsPoint( QgsWkbTypes::Point, 10, 11 ) );
  c16.addGeometry( new QgsPoint( QgsWkbTypes::Point, 20, 21 ) );
  QByteArray wkb16 = c16.asWkb();
  QgsMultiPoint c17;
  QgsConstWkbPtr wkb16ptr( wkb16 );
  c17.fromWkb( wkb16ptr );
  QCOMPARE( c17.numGeometries(), 2 );
  QCOMPARE( *static_cast< const QgsPoint * >( c17.geometryN( 0 ) ), QgsPoint( QgsWkbTypes::Point, 10, 11 ) );
  QCOMPARE( *static_cast< const QgsPoint * >( c17.geometryN( 1 ) ), QgsPoint( QgsWkbTypes::Point, 20, 21 ) );

  //parts with Z
  c16.clear();
  c17.clear();
  c16.addGeometry( new QgsPoint( QgsWkbTypes::PointZ, 10, 0, 4 ) );
  c16.addGeometry( new QgsPoint( QgsWkbTypes::PointZ, 9, 1, 4 ) );
  wkb16 = c16.asWkb();
  QgsConstWkbPtr wkb16ptr2( wkb16 );
  c17.fromWkb( wkb16ptr2 );
  QCOMPARE( c17.numGeometries(), 2 );
  QCOMPARE( c17.wkbType(), QgsWkbTypes::MultiPointZ );
  QCOMPARE( *static_cast< const QgsPoint * >( c17.geometryN( 0 ) ), QgsPoint( QgsWkbTypes::PointZ, 10, 0, 4 ) );
  QCOMPARE( *static_cast< const QgsPoint * >( c17.geometryN( 1 ) ), QgsPoint( QgsWkbTypes::PointZ, 9, 1, 4 ) );

  //parts with m
  c16.clear();
  c17.clear();
  c16.addGeometry( new QgsPoint( QgsWkbTypes::PointM, 10, 0, 0, 4 ) );
  c16.addGeometry( new QgsPoint( QgsWkbTypes::PointM, 9, 1, 0, 4 ) );
  wkb16 = c16.asWkb();
  QgsConstWkbPtr wkb16ptr3( wkb16 );
  c17.fromWkb( wkb16ptr3 );
  QCOMPARE( c17.numGeometries(), 2 );
  QCOMPARE( c17.wkbType(), QgsWkbTypes::MultiPointM );
  QCOMPARE( *static_cast< const QgsPoint * >( c17.geometryN( 0 ) ), QgsPoint( QgsWkbTypes::PointM, 10, 0, 0, 4 ) );
  QCOMPARE( *static_cast< const QgsPoint * >( c17.geometryN( 1 ) ), QgsPoint( QgsWkbTypes::PointM, 9, 1, 0, 4 ) );

  // parts with ZM
  c16.clear();
  c17.clear();
  c16.addGeometry( new QgsPoint( QgsWkbTypes::PointZM, 10, 0, 70, 4 ) );
  c16.addGeometry( new QgsPoint( QgsWkbTypes::PointZM, 9, 1, 3, 4 ) );
  wkb16 = c16.asWkb();
  QgsConstWkbPtr wkb16ptr4( wkb16 );
  c17.fromWkb( wkb16ptr4 );
  QCOMPARE( c17.numGeometries(), 2 );
  QCOMPARE( c17.wkbType(), QgsWkbTypes::MultiPointZM );
  QCOMPARE( *static_cast< const QgsPoint * >( c17.geometryN( 0 ) ), QgsPoint( QgsWkbTypes::PointZM, 10, 0, 70, 4 ) );
  QCOMPARE( *static_cast< const QgsPoint * >( c17.geometryN( 1 ) ), QgsPoint( QgsWkbTypes::PointZM, 9, 1, 3, 4 ) );

  //bad WKB - check for no crash
  c17.clear();
  QgsConstWkbPtr nullPtr( nullptr, 0 );
  QVERIFY( !c17.fromWkb( nullPtr ) );
  QCOMPARE( c17.wkbType(), QgsWkbTypes::MultiPoint );
  QgsPoint point( 1, 2 );
  QByteArray wkbPoint = point.asWkb();
  QgsConstWkbPtr wkbPointPtr( wkbPoint );
  QVERIFY( !c17.fromWkb( wkbPointPtr ) );
  QCOMPARE( c17.wkbType(), QgsWkbTypes::MultiPoint );

  //to/from WKT
  QgsMultiPoint c18;
  c18.addGeometry( new QgsPoint( QgsWkbTypes::PointZM, 10, 0, 4, 8 ) );
  c18.addGeometry( new QgsPoint( QgsWkbTypes::PointZM, 9, 1, 4, 4 ) );

  QString wkt = c18.asWkt();
  QVERIFY( !wkt.isEmpty() );
  QgsMultiPoint c19;
  QVERIFY( c19.fromWkt( wkt ) );
  QCOMPARE( c19.numGeometries(), 2 );
  QCOMPARE( *static_cast< const QgsPoint * >( c19.geometryN( 0 ) ), QgsPoint( QgsWkbTypes::PointZM, 10, 0, 4, 8 ) );
  QCOMPARE( *static_cast< const QgsPoint * >( c19.geometryN( 1 ) ), QgsPoint( QgsWkbTypes::PointZM, 9, 1, 4, 4 ) );

  //bad WKT
  QgsMultiPoint c20;
  QVERIFY( !c20.fromWkt( "Point()" ) );
  QVERIFY( c20.isEmpty() );
  QCOMPARE( c20.numGeometries(), 0 );
  QCOMPARE( c20.wkbType(), QgsWkbTypes::MultiPoint );

  //as JSON
  QgsMultiPoint exportC;
  exportC.addGeometry( new QgsPoint( QgsWkbTypes::Point, 0, 10 ) );
  exportC.addGeometry( new QgsPoint( QgsWkbTypes::Point, 10, 0 ) );

  // GML document for compare
  QDomDocument doc( "gml" );

  // as GML2
  QString expectedSimpleGML2( QStringLiteral( "<MultiPoint xmlns=\"gml\"><pointMember xmlns=\"gml\"><Point xmlns=\"gml\"><coordinates xmlns=\"gml\" cs=\",\" ts=\" \">0,10</coordinates></Point></pointMember><pointMember xmlns=\"gml\"><Point xmlns=\"gml\"><coordinates xmlns=\"gml\" cs=\",\" ts=\" \">10,0</coordinates></Point></pointMember></MultiPoint>" ) );
  QString res = elemToString( exportC.asGml2( doc ) );
  QGSCOMPAREGML( res, expectedSimpleGML2 );
  QString expectedGML2empty( QStringLiteral( "<MultiPoint xmlns=\"gml\"/>" ) );
  QGSCOMPAREGML( elemToString( QgsMultiPoint().asGml2( doc ) ), expectedGML2empty );

  //as GML3
  QString expectedSimpleGML3( QStringLiteral( "<MultiPoint xmlns=\"gml\"><pointMember xmlns=\"gml\"><Point xmlns=\"gml\"><pos xmlns=\"gml\" srsDimension=\"2\">0 10</pos></Point></pointMember><pointMember xmlns=\"gml\"><Point xmlns=\"gml\"><pos xmlns=\"gml\" srsDimension=\"2\">10 0</pos></Point></pointMember></MultiPoint>" ) );
  res = elemToString( exportC.asGml3( doc ) );
  QCOMPARE( res, expectedSimpleGML3 );
  QString expectedGML3empty( QStringLiteral( "<MultiPoint xmlns=\"gml\"/>" ) );
  QGSCOMPAREGML( elemToString( QgsMultiPoint().asGml3( doc ) ), expectedGML3empty );

  // as JSON
  QString expectedSimpleJson( "{\"type\": \"MultiPoint\", \"coordinates\": [ [0, 10], [10, 0]] }" );
  res = exportC.asJson();
  QCOMPARE( res, expectedSimpleJson );

  QgsMultiPoint exportFloat;
  exportFloat.addGeometry( new QgsPoint( QgsWkbTypes::Point, 10 / 9.0, 100 / 9.0 ) );
  exportFloat.addGeometry( new QgsPoint( QgsWkbTypes::Point, 4 / 3.0, 2 / 3.0 ) );


  QString expectedJsonPrec3( QStringLiteral( "{\"type\": \"MultiPoint\", \"coordinates\": [ [1.111, 11.111], [1.333, 0.667]] }" ) );
  res = exportFloat.asJson( 3 );
  QCOMPARE( res, expectedJsonPrec3 );

  // as GML2
  QString expectedGML2prec3( QStringLiteral( "<MultiPoint xmlns=\"gml\"><pointMember xmlns=\"gml\"><Point xmlns=\"gml\"><coordinates xmlns=\"gml\" cs=\",\" ts=\" \">1.111,11.111</coordinates></Point></pointMember><pointMember xmlns=\"gml\"><Point xmlns=\"gml\"><coordinates xmlns=\"gml\" cs=\",\" ts=\" \">1.333,0.667</coordinates></Point></pointMember></MultiPoint>" ) );
  res = elemToString( exportFloat.asGml2( doc, 3 ) );
  QGSCOMPAREGML( res, expectedGML2prec3 );

  //as GML3
  QString expectedGML3prec3( QStringLiteral( "<MultiPoint xmlns=\"gml\"><pointMember xmlns=\"gml\"><Point xmlns=\"gml\"><pos xmlns=\"gml\" srsDimension=\"2\">1.111 11.111</pos></Point></pointMember><pointMember xmlns=\"gml\"><Point xmlns=\"gml\"><pos xmlns=\"gml\" srsDimension=\"2\">1.333 0.667</pos></Point></pointMember></MultiPoint>" ) );
  res = elemToString( exportFloat.asGml3( doc, 3 ) );
  QCOMPARE( res, expectedGML3prec3 );

  // insert geometry
  QgsMultiPoint rc;
  rc.clear();
  rc.insertGeometry( nullptr, 0 );
  QVERIFY( rc.isEmpty() );
  QCOMPARE( rc.numGeometries(), 0 );
  rc.insertGeometry( nullptr, -1 );
  QVERIFY( rc.isEmpty() );
  QCOMPARE( rc.numGeometries(), 0 );
  rc.insertGeometry( nullptr, 100 );
  QVERIFY( rc.isEmpty() );
  QCOMPARE( rc.numGeometries(), 0 );

  rc.insertGeometry( new QgsLineString(), 0 );
  QVERIFY( rc.isEmpty() );
  QCOMPARE( rc.numGeometries(), 0 );

  // cast
  QVERIFY( !QgsMultiPoint().cast( nullptr ) );
  QgsMultiPoint pCast;
  QVERIFY( QgsMultiPoint().cast( &pCast ) );
  QgsMultiPoint pCast2;
  pCast2.fromWkt( QStringLiteral( "MultiPointZ(PointZ(0 1 1))" ) );
  QVERIFY( QgsMultiPoint().cast( &pCast2 ) );
  pCast2.fromWkt( QStringLiteral( "MultiPointM(PointM(0 1 1))" ) );
  QVERIFY( QgsMultiPoint().cast( &pCast2 ) );
  pCast2.fromWkt( QStringLiteral( "MultiPointZM(PointZM(0 1 1 2))" ) );
  QVERIFY( QgsMultiPoint().cast( &pCast2 ) );

  //boundary

  //multipoints have no boundary defined
  QgsMultiPoint boundaryMP;
  QVERIFY( !boundaryMP.boundary() );
  // add some points and retest, should still be undefined
  boundaryMP.addGeometry( new QgsPoint( 0, 0 ) );
  boundaryMP.addGeometry( new QgsPoint( 1, 1 ) );
  QVERIFY( !boundaryMP.boundary() );

  // closestSegment
  QgsPoint closest;
  QgsVertexId after;
  // return error - points have no segments
  QVERIFY( boundaryMP.closestSegment( QgsPoint( 0.5, 0.5 ), closest, after ) < 0 );

  // vertex iterator
  QgsAbstractGeometry::vertex_iterator it = boundaryMP.vertices_begin();
  QgsAbstractGeometry::vertex_iterator itEnd = boundaryMP.vertices_end();
  QCOMPARE( *it, QgsPoint( 0, 0 ) );
  QCOMPARE( it.vertexId(), QgsVertexId( 0, 0, 0 ) );
  ++it;
  QCOMPARE( *it, QgsPoint( 1, 1 ) );
  QCOMPARE( it.vertexId(), QgsVertexId( 1, 0, 0 ) );
  ++it;
  QCOMPARE( it, itEnd );

  // Java-style iterator
  QgsVertexIterator it2( &boundaryMP );
  QVERIFY( it2.hasNext() );
  QCOMPARE( it2.next(), QgsPoint( 0, 0 ) );
  QVERIFY( it2.hasNext() );
  QCOMPARE( it2.next(), QgsPoint( 1, 1 ) );
  QVERIFY( !it2.hasNext() );

  //adjacent vertices - both should be invalid
  QgsMultiPoint c21;
  c21.addGeometry( new QgsPoint( QgsWkbTypes::PointZM, 10, 0, 4, 8 ) );
  c21.addGeometry( new QgsPoint( QgsWkbTypes::PointZM, 9, 1, 4, 4 ) );
  QgsVertexId prev( 1, 2, 3 ); // start with something
  QgsVertexId next( 4, 5, 6 );
  c21.adjacentVertices( QgsVertexId( 0, 0, 0 ), prev, next );
  QCOMPARE( prev, QgsVertexId() );
  QCOMPARE( next, QgsVertexId() );
  c21.adjacentVertices( QgsVertexId( 1, 0, 0 ), prev, next );
  QCOMPARE( prev, QgsVertexId() );
  QCOMPARE( next, QgsVertexId() );

  // vertex number
  QgsMultiPoint c22;
  QCOMPARE( c22.vertexNumberFromVertexId( QgsVertexId( -1, 0, 0 ) ), -1 );
  QCOMPARE( c22.vertexNumberFromVertexId( QgsVertexId( 0, 0, 0 ) ), -1 );
  QCOMPARE( c22.vertexNumberFromVertexId( QgsVertexId( 1, 0, 0 ) ), -1 );
  c22.addGeometry( new QgsPoint( QgsWkbTypes::PointZM, 10, 0, 4, 8 ) );
  c22.addGeometry( new QgsPoint( QgsWkbTypes::PointZM, 9, 1, 4, 4 ) );
  QCOMPARE( c22.vertexNumberFromVertexId( QgsVertexId( -1, 0, 0 ) ), -1 );
  QCOMPARE( c22.vertexNumberFromVertexId( QgsVertexId( 2, 0, 0 ) ), -1 );
  QCOMPARE( c22.vertexNumberFromVertexId( QgsVertexId( 0, 0, 0 ) ), 0 );
  QCOMPARE( c22.vertexNumberFromVertexId( QgsVertexId( 0, 0, 1 ) ), -1 );
  QCOMPARE( c22.vertexNumberFromVertexId( QgsVertexId( 0, 1, 0 ) ), -1 );
  QCOMPARE( c22.vertexNumberFromVertexId( QgsVertexId( 1, 0, 0 ) ), 1 );
  QCOMPARE( c22.vertexNumberFromVertexId( QgsVertexId( 1, 0, 1 ) ), -1 );
  QCOMPARE( c22.vertexNumberFromVertexId( QgsVertexId( -1, 0, 0 ) ), -1 );

  QgsMultiPoint mp;
  // multipoints should not be affected by removeDuplicatePoints
  QVERIFY( !mp.removeDuplicateNodes() );
  mp.addGeometry( new QgsPoint( QgsWkbTypes::PointZM, 10, 1, 4, 8 ) );
  mp.addGeometry( new QgsPoint( QgsWkbTypes::PointZM, 10, 1, 4, 8 ) );
  QVERIFY( !mp.removeDuplicateNodes() );
  QCOMPARE( mp.numGeometries(), 2 );

  // filter vertex
  QgsMultiPoint filterPoint;
  auto filter = []( const QgsPoint & point )-> bool
  {
    return point.x() < 5;
  };
  filterPoint.filterVertices( filter ); // no crash
  filterPoint.addGeometry( new QgsPoint( QgsWkbTypes::PointZM, 10, 0, 4, 8 ) );
  filterPoint.addGeometry( new QgsPoint( QgsWkbTypes::PointZM, 3, 0, 4, 8 ) );
  filterPoint.addGeometry( new QgsPoint( QgsWkbTypes::PointZM, 1, 0, 4, 8 ) );
  filterPoint.addGeometry( new QgsPoint( QgsWkbTypes::PointZM, 11, 0, 4, 8 ) );
  filterPoint.filterVertices( filter );
  QCOMPARE( filterPoint.asWkt( 2 ), QStringLiteral( "MultiPointZM ((3 0 4 8),(1 0 4 8))" ) );
}

void TestQgsGeometry::multiLineString()
{
  //test constructor
  QgsMultiLineString c1;
  QVERIFY( c1.isEmpty() );
  QCOMPARE( c1.nCoordinates(), 0 );
  QCOMPARE( c1.ringCount(), 0 );
  QCOMPARE( c1.partCount(), 0 );
  QVERIFY( !c1.is3D() );
  QVERIFY( !c1.isMeasure() );
  QCOMPARE( c1.wkbType(), QgsWkbTypes::MultiLineString );
  QCOMPARE( c1.wktTypeStr(), QString( "MultiLineString" ) );
  QCOMPARE( c1.geometryType(), QString( "MultiLineString" ) );
  QCOMPARE( c1.dimension(), 0 );
  QVERIFY( !c1.hasCurvedSegments() );
  QCOMPARE( c1.area(), 0.0 );
  QCOMPARE( c1.perimeter(), 0.0 );
  QCOMPARE( c1.numGeometries(), 0 );
  QVERIFY( !c1.geometryN( 0 ) );
  QVERIFY( !c1.geometryN( -1 ) );
  QCOMPARE( c1.vertexCount( 0, 0 ), 0 );
  QCOMPARE( c1.vertexCount( 0, 1 ), 0 );
  QCOMPARE( c1.vertexCount( 1, 0 ), 0 );

  //addGeometry

  //try with nullptr
  c1.addGeometry( nullptr );
  QVERIFY( c1.isEmpty() );
  QCOMPARE( c1.nCoordinates(), 0 );
  QCOMPARE( c1.ringCount(), 0 );
  QCOMPARE( c1.partCount(), 0 );
  QCOMPARE( c1.numGeometries(), 0 );
  QCOMPARE( c1.wkbType(), QgsWkbTypes::MultiLineString );
  QVERIFY( !c1.geometryN( 0 ) );
  QVERIFY( !c1.geometryN( -1 ) );

  // not a linestring
  QVERIFY( !c1.addGeometry( new QgsPoint() ) );
  QVERIFY( c1.isEmpty() );
  QCOMPARE( c1.nCoordinates(), 0 );
  QCOMPARE( c1.ringCount(), 0 );
  QCOMPARE( c1.partCount(), 0 );
  QCOMPARE( c1.numGeometries(), 0 );
  QCOMPARE( c1.wkbType(), QgsWkbTypes::MultiLineString );
  QVERIFY( !c1.geometryN( 0 ) );
  QVERIFY( !c1.geometryN( -1 ) );

  //valid geometry
  QgsLineString part;
  part.setPoints( QgsPointSequence() << QgsPoint( 1, 10 ) << QgsPoint( 2, 11 ) );
  c1.addGeometry( part.clone() );
  QVERIFY( !c1.isEmpty() );
  QCOMPARE( c1.numGeometries(), 1 );
  QCOMPARE( c1.nCoordinates(), 2 );
  QCOMPARE( c1.ringCount(), 1 );
  QCOMPARE( c1.partCount(), 1 );
  QVERIFY( !c1.is3D() );
  QVERIFY( !c1.isMeasure() );
  QCOMPARE( c1.wkbType(), QgsWkbTypes::MultiLineString );
  QCOMPARE( c1.wktTypeStr(), QString( "MultiLineString" ) );
  QCOMPARE( c1.geometryType(), QString( "MultiLineString" ) );
  QCOMPARE( c1.dimension(), 1 );
  QVERIFY( !c1.hasCurvedSegments() );
  QCOMPARE( c1.area(), 0.0 );
  QCOMPARE( c1.perimeter(), 0.0 );
  QVERIFY( c1.geometryN( 0 ) );
  QCOMPARE( *static_cast< const QgsLineString * >( c1.geometryN( 0 ) ), part );
  QVERIFY( !c1.geometryN( 100 ) );
  QVERIFY( !c1.geometryN( -1 ) );
  QCOMPARE( c1.vertexCount( 0, 0 ), 2 );
  QCOMPARE( c1.vertexCount( 1, 0 ), 0 );

  //initial adding of geometry should set z/m type
  part.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZ, 10, 11, 1 ) << QgsPoint( QgsWkbTypes::PointZ, 20, 21, 2 ) );
  QgsMultiLineString c2;
  c2.addGeometry( part.clone() );
  QVERIFY( c2.is3D() );
  QVERIFY( !c2.isMeasure() );
  QCOMPARE( c2.wkbType(), QgsWkbTypes::MultiLineStringZ );
  QCOMPARE( c2.wktTypeStr(), QString( "MultiLineStringZ" ) );
  QCOMPARE( c2.geometryType(), QString( "MultiLineString" ) );
  QCOMPARE( *( static_cast< const QgsLineString * >( c2.geometryN( 0 ) ) ), part );
  QgsMultiLineString c3;
  part.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointM, 10, 11, 0, 1 ) << QgsPoint( QgsWkbTypes::PointM, 20, 21, 0, 2 ) );
  c3.addGeometry( part.clone() );
  QVERIFY( !c3.is3D() );
  QVERIFY( c3.isMeasure() );
  QCOMPARE( c3.wkbType(), QgsWkbTypes::MultiLineStringM );
  QCOMPARE( c3.wktTypeStr(), QString( "MultiLineStringM" ) );
  QCOMPARE( *( static_cast< const QgsLineString * >( c3.geometryN( 0 ) ) ), part );
  QgsMultiLineString c4;
  part.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 10, 11, 2, 1 ) << QgsPoint( QgsWkbTypes::PointZM, 20, 21, 3, 2 ) );
  c4.addGeometry( part.clone() );
  QVERIFY( c4.is3D() );
  QVERIFY( c4.isMeasure() );
  QCOMPARE( c4.wkbType(), QgsWkbTypes::MultiLineStringZM );
  QCOMPARE( c4.wktTypeStr(), QString( "MultiLineStringZM" ) );
  QCOMPARE( *( static_cast< const QgsLineString * >( c4.geometryN( 0 ) ) ), part );

  //add another part
  QgsMultiLineString c6;
  part.setPoints( QgsPointSequence() << QgsPoint( 1, 10 ) << QgsPoint( 2, 11 ) );
  c6.addGeometry( part.clone() );
  QCOMPARE( c6.vertexCount( 0, 0 ), 2 );
  part.setPoints( QgsPointSequence() << QgsPoint( 9, 12 ) << QgsPoint( 3, 13 ) );
  c6.addGeometry( part.clone() );
  QCOMPARE( c6.vertexCount( 1, 0 ), 2 );
  QCOMPARE( c6.numGeometries(), 2 );
  QVERIFY( c6.geometryN( 0 ) );
  QCOMPARE( *static_cast< const QgsLineString * >( c6.geometryN( 1 ) ), part );

  //adding subsequent points should not alter z/m type, regardless of points type
  c6.clear();
  c6.addGeometry( part.clone() );
  QCOMPARE( c6.wkbType(), QgsWkbTypes::MultiLineString );
  part.setPoints( QgsPointSequence() << QgsPoint( 1, 10, 2 ) << QgsPoint( 2, 11, 3 ) ) ;
  c6.addGeometry( part.clone() );
  QCOMPARE( c6.wkbType(), QgsWkbTypes::MultiLineString );
  QCOMPARE( c6.vertexCount( 0, 0 ), 2 );
  QCOMPARE( c6.vertexCount( 1, 0 ), 2 );
  QCOMPARE( c6.vertexCount( 2, 0 ), 0 );
  QCOMPARE( c6.vertexCount( -1, 0 ), 0 );
  QCOMPARE( c6.nCoordinates(), 4 );
  QCOMPARE( c6.ringCount(), 1 );
  QCOMPARE( c6.partCount(), 2 );
  QVERIFY( !c6.is3D() );
  const QgsLineString *ls = static_cast< const QgsLineString * >( c6.geometryN( 0 ) );
  QCOMPARE( ls->pointN( 0 ), QgsPoint( 9, 12 ) );
  QCOMPARE( ls->pointN( 1 ), QgsPoint( 3, 13 ) );
  ls = static_cast< const QgsLineString * >( c6.geometryN( 1 ) );
  QCOMPARE( ls->pointN( 0 ), QgsPoint( 1, 10 ) );
  QCOMPARE( ls->pointN( 1 ), QgsPoint( 2, 11 ) );
  part.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointM, 21, 30, 0, 2 ) << QgsPoint( QgsWkbTypes::PointM, 32, 41, 0, 3 ) ) ;
  c6.addGeometry( part.clone() );
  QCOMPARE( c6.wkbType(), QgsWkbTypes::MultiLineString );
  QCOMPARE( c6.vertexCount( 0, 0 ), 2 );
  QCOMPARE( c6.vertexCount( 1, 0 ), 2 );
  QCOMPARE( c6.vertexCount( 2, 0 ), 2 );
  QCOMPARE( c6.nCoordinates(), 6 );
  QCOMPARE( c6.ringCount(), 1 );
  QCOMPARE( c6.partCount(), 3 );
  QVERIFY( !c6.is3D() );
  QVERIFY( !c6.isMeasure() );
  ls = static_cast< const QgsLineString * >( c6.geometryN( 2 ) );
  QCOMPARE( ls->pointN( 0 ), QgsPoint( 21, 30 ) );
  QCOMPARE( ls->pointN( 1 ), QgsPoint( 32, 41 ) );

  c6.clear();
  part.setPoints( QgsPointSequence() << QgsPoint( 1, 10, 2 ) << QgsPoint( 2, 11, 3 ) ) ;
  c6.addGeometry( part.clone() );
  QCOMPARE( c6.wkbType(), QgsWkbTypes::MultiLineStringZ );
  part.setPoints( QgsPointSequence() << QgsPoint( 2, 20 ) << QgsPoint( 3, 31 ) ) ;
  c6.addGeometry( part.clone() );
  QCOMPARE( c6.wkbType(), QgsWkbTypes::MultiLineStringZ );
  QVERIFY( c6.is3D() );
  ls = static_cast< const QgsLineString * >( c6.geometryN( 0 ) );
  QCOMPARE( ls->pointN( 0 ), QgsPoint( 1, 10, 2 ) );
  QCOMPARE( ls->pointN( 1 ), QgsPoint( 2, 11, 3 ) );
  ls = static_cast< const QgsLineString * >( c6.geometryN( 1 ) );
  QCOMPARE( ls->pointN( 0 ), QgsPoint( 2, 20, 0 ) );
  QCOMPARE( ls->pointN( 1 ), QgsPoint( 3, 31, 0 ) );
  part.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointM, 5, 50, 0, 4 ) << QgsPoint( QgsWkbTypes::PointM, 6, 61, 0, 5 ) ) ;
  c6.addGeometry( part.clone() );
  QCOMPARE( c6.wkbType(), QgsWkbTypes::MultiLineStringZ );
  QVERIFY( c6.is3D() );
  QVERIFY( !c6.isMeasure() );
  ls = static_cast< const QgsLineString * >( c6.geometryN( 2 ) );
  QCOMPARE( ls->pointN( 0 ), QgsPoint( 5, 50, 0 ) );
  QCOMPARE( ls->pointN( 1 ), QgsPoint( 6, 61, 0 ) );

  c6.clear();
  QCOMPARE( c6.wkbType(), QgsWkbTypes::MultiLineString );
  part.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointM, 5, 50, 0, 4 ) << QgsPoint( QgsWkbTypes::PointM, 6, 61, 0, 5 ) ) ;
  c6.addGeometry( part.clone() );
  QCOMPARE( c6.wkbType(), QgsWkbTypes::MultiLineStringM );
  part.setPoints( QgsPointSequence() << QgsPoint( 2, 20 ) << QgsPoint( 3, 31 ) ) ;
  c6.addGeometry( part.clone() );
  QCOMPARE( c6.wkbType(), QgsWkbTypes::MultiLineStringM );
  QVERIFY( c6.isMeasure() );
  ls = static_cast< const QgsLineString * >( c6.geometryN( 0 ) );
  QCOMPARE( ls->pointN( 0 ), QgsPoint( QgsWkbTypes::PointM, 5, 50, 0, 4 ) );
  QCOMPARE( ls->pointN( 1 ), QgsPoint( QgsWkbTypes::PointM, 6, 61, 0, 5 ) );
  ls = static_cast< const QgsLineString * >( c6.geometryN( 1 ) );
  QCOMPARE( ls->pointN( 0 ), QgsPoint( QgsWkbTypes::PointM, 2, 20, 0, 0 ) );
  QCOMPARE( ls->pointN( 1 ), QgsPoint( QgsWkbTypes::PointM, 3, 31, 0, 0 ) );
  part.setPoints( QgsPointSequence() << QgsPoint( 11, 12, 13 ) << QgsPoint( 14, 15, 16 ) ) ;
  c6.addGeometry( part.clone() );
  QCOMPARE( c6.wkbType(), QgsWkbTypes::MultiLineStringM );
  QVERIFY( !c6.is3D() );
  QVERIFY( c6.isMeasure() );
  ls = static_cast< const QgsLineString * >( c6.geometryN( 2 ) );
  QCOMPARE( ls->pointN( 0 ), QgsPoint( QgsWkbTypes::PointM, 11, 12, 0, 0 ) );
  QCOMPARE( ls->pointN( 1 ), QgsPoint( QgsWkbTypes::PointM, 14, 15, 0, 0 ) );

  c6.clear();
  part.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 5, 50, 1, 4 ) << QgsPoint( QgsWkbTypes::PointZM, 6, 61, 3, 5 ) ) ;
  c6.addGeometry( part.clone() );
  QCOMPARE( c6.wkbType(), QgsWkbTypes::MultiLineStringZM );
  part.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::Point, 7, 17 ) << QgsPoint( QgsWkbTypes::Point, 3, 13 ) ) ;
  c6.addGeometry( part.clone() );
  QCOMPARE( c6.wkbType(), QgsWkbTypes::MultiLineStringZM );
  QVERIFY( c6.isMeasure() );
  QVERIFY( c6.is3D() );
  ls = static_cast< const QgsLineString * >( c6.geometryN( 0 ) );
  QCOMPARE( ls->pointN( 0 ), QgsPoint( QgsWkbTypes::PointZM, 5, 50, 1, 4 ) );
  QCOMPARE( ls->pointN( 1 ), QgsPoint( QgsWkbTypes::PointZM, 6, 61, 3, 5 ) );
  ls = static_cast< const QgsLineString * >( c6.geometryN( 1 ) );
  QCOMPARE( ls->pointN( 0 ), QgsPoint( QgsWkbTypes::PointZM, 7, 17, 0, 0 ) );
  QCOMPARE( ls->pointN( 1 ), QgsPoint( QgsWkbTypes::PointZM, 3, 13, 0, 0 ) );
  part.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZ, 77, 87, 7 ) << QgsPoint( QgsWkbTypes::PointZ, 83, 83, 8 ) ) ;
  c6.addGeometry( part.clone() );
  QCOMPARE( c6.wkbType(), QgsWkbTypes::MultiLineStringZM );
  QVERIFY( c6.is3D() );
  QVERIFY( c6.isMeasure() );
  ls = static_cast< const QgsLineString * >( c6.geometryN( 2 ) );
  QCOMPARE( ls->pointN( 0 ), QgsPoint( QgsWkbTypes::PointZM, 77, 87, 7, 0 ) );
  QCOMPARE( ls->pointN( 1 ), QgsPoint( QgsWkbTypes::PointZM, 83, 83, 8, 0 ) );
  part.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointM, 177, 187, 0, 9 ) << QgsPoint( QgsWkbTypes::PointM, 183, 183, 0, 11 ) ) ;
  c6.addGeometry( part.clone() );
  QCOMPARE( c6.wkbType(), QgsWkbTypes::MultiLineStringZM );
  QVERIFY( c6.is3D() );
  QVERIFY( c6.isMeasure() );
  ls = static_cast< const QgsLineString * >( c6.geometryN( 3 ) );
  QCOMPARE( ls->pointN( 0 ), QgsPoint( QgsWkbTypes::PointZM, 177, 187, 0, 9 ) );
  QCOMPARE( ls->pointN( 1 ), QgsPoint( QgsWkbTypes::PointZM, 183, 183, 0, 11 ) );

  //clear
  QgsMultiLineString c7;
  part.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 5, 50, 1, 4 ) << QgsPoint( QgsWkbTypes::PointZM, 6, 61, 3, 5 ) ) ;
  c7.addGeometry( part.clone() );
  c7.addGeometry( part.clone() );
  QCOMPARE( c7.numGeometries(), 2 );
  c7.clear();
  QVERIFY( c7.isEmpty() );
  QCOMPARE( c7.numGeometries(), 0 );
  QCOMPARE( c7.nCoordinates(), 0 );
  QCOMPARE( c7.ringCount(), 0 );
  QCOMPARE( c7.partCount(), 0 );
  QVERIFY( !c7.is3D() );
  QVERIFY( !c7.isMeasure() );
  QCOMPARE( c7.wkbType(), QgsWkbTypes::MultiLineString );

  //clone
  QgsMultiLineString c11;
  std::unique_ptr< QgsMultiLineString >cloned( c11.clone() );
  QVERIFY( cloned->isEmpty() );
  c11.addGeometry( part.clone() );
  c11.addGeometry( part.clone() );
  cloned.reset( c11.clone() );
  QCOMPARE( cloned->numGeometries(), 2 );
  ls = static_cast< const QgsLineString * >( cloned->geometryN( 0 ) );
  QCOMPARE( *ls, part );
  ls = static_cast< const QgsLineString * >( cloned->geometryN( 1 ) );
  QCOMPARE( *ls, part );

  //copy constructor
  QgsMultiLineString c12;
  QgsMultiLineString c13( c12 );
  QVERIFY( c13.isEmpty() );
  c12.addGeometry( part.clone() );
  c12.addGeometry( part.clone() );
  QgsMultiLineString c14( c12 );
  QCOMPARE( c14.numGeometries(), 2 );
  QCOMPARE( c14.wkbType(), QgsWkbTypes::MultiLineStringZM );
  ls = static_cast< const QgsLineString * >( c14.geometryN( 0 ) );
  QCOMPARE( *ls, part );
  ls = static_cast< const QgsLineString * >( c14.geometryN( 1 ) );
  QCOMPARE( *ls, part );

  //assignment operator
  QgsMultiLineString c15;
  c15 = c13;
  QCOMPARE( c15.numGeometries(), 0 );
  c15 = c14;
  QCOMPARE( c15.numGeometries(), 2 );
  ls = static_cast< const QgsLineString * >( c15.geometryN( 0 ) );
  QCOMPARE( *ls, part );
  ls = static_cast< const QgsLineString * >( c15.geometryN( 1 ) );
  QCOMPARE( *ls, part );

  //toCurveType
  std::unique_ptr< QgsMultiCurve > curveType( c12.toCurveType() );
  QCOMPARE( curveType->wkbType(), QgsWkbTypes::MultiCurveZM );
  QCOMPARE( curveType->numGeometries(), 2 );
  const QgsCompoundCurve *curve = static_cast< const QgsCompoundCurve * >( curveType->geometryN( 0 ) );
  QCOMPARE( curve->asWkt(), QStringLiteral( "CompoundCurveZM ((5 50 1 4, 6 61 3 5))" ) );
  curve = static_cast< const QgsCompoundCurve * >( curveType->geometryN( 1 ) );
  QCOMPARE( curve->asWkt(), QStringLiteral( "CompoundCurveZM ((5 50 1 4, 6 61 3 5))" ) );

  //to/fromWKB
  QgsMultiLineString c16;
  part.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::Point, 7, 17 ) << QgsPoint( QgsWkbTypes::Point, 3, 13 ) ) ;
  c16.addGeometry( part.clone() );
  part.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::Point, 27, 37 ) << QgsPoint( QgsWkbTypes::Point, 43, 43 ) ) ;
  c16.addGeometry( part.clone() );
  QByteArray wkb16 = c16.asWkb();
  QgsMultiLineString c17;
  QgsConstWkbPtr wkb16ptr( wkb16 );
  c17.fromWkb( wkb16ptr );
  QCOMPARE( c17.numGeometries(), 2 );
  QCOMPARE( *static_cast< const QgsLineString * >( c17.geometryN( 0 ) ), *static_cast< const QgsLineString * >( c16.geometryN( 0 ) ) );
  QCOMPARE( *static_cast< const QgsLineString * >( c17.geometryN( 1 ) ), *static_cast< const QgsLineString * >( c16.geometryN( 1 ) ) );

  //parts with Z
  c16.clear();
  c17.clear();
  part.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZ, 7, 17, 1 ) << QgsPoint( QgsWkbTypes::PointZ, 3, 13, 4 ) ) ;
  c16.addGeometry( part.clone() );
  part.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZ, 27, 37, 2 ) << QgsPoint( QgsWkbTypes::PointZ, 43, 43, 5 ) ) ;
  c16.addGeometry( part.clone() );
  wkb16 = c16.asWkb();
  QgsConstWkbPtr wkb16ptr2( wkb16 );
  c17.fromWkb( wkb16ptr2 );
  QCOMPARE( c17.numGeometries(), 2 );
  QCOMPARE( c17.wkbType(), QgsWkbTypes::MultiLineStringZ );
  QCOMPARE( *static_cast< const QgsLineString * >( c17.geometryN( 0 ) ), *static_cast< const QgsLineString * >( c16.geometryN( 0 ) ) );
  QCOMPARE( *static_cast< const QgsLineString * >( c17.geometryN( 1 ) ), *static_cast< const QgsLineString * >( c16.geometryN( 1 ) ) );

  //parts with m
  c16.clear();
  c17.clear();
  part.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointM, 7, 17, 0, 1 ) << QgsPoint( QgsWkbTypes::PointM, 3, 13, 0, 4 ) ) ;
  c16.addGeometry( part.clone() );
  part.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointM, 27, 37, 0, 2 ) << QgsPoint( QgsWkbTypes::PointM, 43, 43, 0, 5 ) ) ;
  c16.addGeometry( part.clone() );
  wkb16 = c16.asWkb();
  QgsConstWkbPtr wkb16ptr3( wkb16 );
  c17.fromWkb( wkb16ptr3 );
  QCOMPARE( c17.numGeometries(), 2 );
  QCOMPARE( c17.wkbType(), QgsWkbTypes::MultiLineStringM );
  QCOMPARE( *static_cast< const QgsLineString * >( c17.geometryN( 0 ) ), *static_cast< const QgsLineString * >( c16.geometryN( 0 ) ) );
  QCOMPARE( *static_cast< const QgsLineString * >( c17.geometryN( 1 ) ), *static_cast< const QgsLineString * >( c16.geometryN( 1 ) ) );

  // parts with ZM
  c16.clear();
  c17.clear();
  part.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 7, 17, 4, 1 ) << QgsPoint( QgsWkbTypes::PointZM, 3, 13, 1, 4 ) ) ;
  c16.addGeometry( part.clone() );
  part.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 27, 37, 6, 2 ) << QgsPoint( QgsWkbTypes::PointZM, 43, 43, 11, 5 ) ) ;
  c16.addGeometry( part.clone() );
  wkb16 = c16.asWkb();
  QgsConstWkbPtr wkb16ptr4( wkb16 );
  c17.fromWkb( wkb16ptr4 );
  QCOMPARE( c17.numGeometries(), 2 );
  QCOMPARE( c17.wkbType(), QgsWkbTypes::MultiLineStringZM );
  QCOMPARE( *static_cast< const QgsLineString * >( c17.geometryN( 0 ) ), *static_cast< const QgsLineString * >( c16.geometryN( 0 ) ) );
  QCOMPARE( *static_cast< const QgsLineString * >( c17.geometryN( 1 ) ), *static_cast< const QgsLineString * >( c16.geometryN( 1 ) ) );

  //bad WKB - check for no crash
  c17.clear();
  QgsConstWkbPtr nullPtr( nullptr, 0 );
  QVERIFY( !c17.fromWkb( nullPtr ) );
  QCOMPARE( c17.wkbType(), QgsWkbTypes::MultiLineString );
  QgsPoint point( 1, 2 );
  QByteArray wkbPoint = point.asWkb();
  QgsConstWkbPtr wkbPointPtr( wkbPoint );
  QVERIFY( !c17.fromWkb( wkbPointPtr ) );
  QCOMPARE( c17.wkbType(), QgsWkbTypes::MultiLineString );

  //to/from WKT
  QgsMultiLineString c18;
  part.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 7, 17, 4, 1 ) << QgsPoint( QgsWkbTypes::PointZM, 3, 13, 1, 4 ) ) ;
  c18.addGeometry( part.clone() );
  part.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 27, 37, 6, 2 ) << QgsPoint( QgsWkbTypes::PointZM, 43, 43, 11, 5 ) ) ;
  c18.addGeometry( part.clone() );

  QString wkt = c18.asWkt();
  QVERIFY( !wkt.isEmpty() );
  QgsMultiLineString c19;
  QVERIFY( c19.fromWkt( wkt ) );
  QCOMPARE( c19.numGeometries(), 2 );
  QCOMPARE( *static_cast< const QgsLineString * >( c19.geometryN( 0 ) ), *static_cast< const QgsLineString * >( c18.geometryN( 0 ) ) );
  QCOMPARE( *static_cast< const QgsLineString * >( c19.geometryN( 1 ) ), *static_cast< const QgsLineString * >( c18.geometryN( 1 ) ) );

  //bad WKT
  QgsMultiLineString c20;
  QVERIFY( !c20.fromWkt( "Point()" ) );
  QVERIFY( c20.isEmpty() );
  QCOMPARE( c20.numGeometries(), 0 );
  QCOMPARE( c20.wkbType(), QgsWkbTypes::MultiLineString );

  //as JSON
  QgsMultiLineString exportC;
  part.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::Point, 7, 17 ) << QgsPoint( QgsWkbTypes::Point, 3, 13 ) ) ;
  exportC.addGeometry( part.clone() );
  part.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::Point, 27, 37 ) << QgsPoint( QgsWkbTypes::Point, 43, 43 ) ) ;
  exportC.addGeometry( part.clone() );

  // GML document for compare
  QDomDocument doc( "gml" );

  // as GML2
  QString expectedSimpleGML2( QStringLiteral( "<MultiLineString xmlns=\"gml\"><lineStringMember xmlns=\"gml\"><LineString xmlns=\"gml\"><coordinates xmlns=\"gml\" cs=\",\" ts=\" \">7,17 3,13</coordinates></LineString></lineStringMember><lineStringMember xmlns=\"gml\"><LineString xmlns=\"gml\"><coordinates xmlns=\"gml\" cs=\",\" ts=\" \">27,37 43,43</coordinates></LineString></lineStringMember></MultiLineString>" ) );
  QString res = elemToString( exportC.asGml2( doc ) );
  QGSCOMPAREGML( res, expectedSimpleGML2 );
  QString expectedGML2empty( QStringLiteral( "<MultiLineString xmlns=\"gml\"/>" ) );
  QGSCOMPAREGML( elemToString( QgsMultiLineString().asGml2( doc ) ), expectedGML2empty );

  //as GML3
  QString expectedSimpleGML3( QStringLiteral( "<MultiCurve xmlns=\"gml\"><curveMember xmlns=\"gml\"><LineString xmlns=\"gml\"><posList xmlns=\"gml\" srsDimension=\"2\">7 17 3 13</posList></LineString></curveMember><curveMember xmlns=\"gml\"><LineString xmlns=\"gml\"><posList xmlns=\"gml\" srsDimension=\"2\">27 37 43 43</posList></LineString></curveMember></MultiCurve>" ) );
  res = elemToString( exportC.asGml3( doc ) );
  QCOMPARE( res, expectedSimpleGML3 );
  QString expectedGML3empty( QStringLiteral( "<MultiCurve xmlns=\"gml\"/>" ) );
  QGSCOMPAREGML( elemToString( QgsMultiLineString().asGml3( doc ) ), expectedGML3empty );

  // as JSON
  QString expectedSimpleJson( "{\"type\": \"MultiLineString\", \"coordinates\": [[ [7, 17], [3, 13]], [ [27, 37], [43, 43]]] }" );
  res = exportC.asJson();
  QCOMPARE( res, expectedSimpleJson );

  QgsMultiLineString exportFloat;
  part.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::Point, 7 / 3.0, 17 / 3.0 ) << QgsPoint( QgsWkbTypes::Point, 3 / 5.0, 13 / 3.0 ) ) ;
  exportFloat.addGeometry( part.clone() );
  part.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::Point, 27 / 3.0, 37 / 9.0 ) << QgsPoint( QgsWkbTypes::Point, 43 / 41.0, 43 / 42.0 ) ) ;
  exportFloat.addGeometry( part.clone() );

  QString expectedJsonPrec3( QStringLiteral( "{\"type\": \"MultiLineString\", \"coordinates\": [[ [2.333, 5.667], [0.6, 4.333]], [ [9, 4.111], [1.049, 1.024]]] }" ) );
  res = exportFloat.asJson( 3 );
  QCOMPARE( res, expectedJsonPrec3 );

  // as GML2
  QString expectedGML2prec3( QStringLiteral( "<MultiLineString xmlns=\"gml\"><lineStringMember xmlns=\"gml\"><LineString xmlns=\"gml\"><coordinates xmlns=\"gml\" cs=\",\" ts=\" \">2.333,5.667 0.6,4.333</coordinates></LineString></lineStringMember><lineStringMember xmlns=\"gml\"><LineString xmlns=\"gml\"><coordinates xmlns=\"gml\" cs=\",\" ts=\" \">9,4.111 1.049,1.024</coordinates></LineString></lineStringMember></MultiLineString>" ) );
  res = elemToString( exportFloat.asGml2( doc, 3 ) );
  QGSCOMPAREGML( res, expectedGML2prec3 );

  //as GML3
  QString expectedGML3prec3( QStringLiteral( "<MultiCurve xmlns=\"gml\"><curveMember xmlns=\"gml\"><LineString xmlns=\"gml\"><posList xmlns=\"gml\" srsDimension=\"2\">2.333 5.667 0.6 4.333</posList></LineString></curveMember><curveMember xmlns=\"gml\"><LineString xmlns=\"gml\"><posList xmlns=\"gml\" srsDimension=\"2\">9 4.111 1.049 1.024</posList></LineString></curveMember></MultiCurve>" ) );
  res = elemToString( exportFloat.asGml3( doc, 3 ) );
  QCOMPARE( res, expectedGML3prec3 );

  // insert geometry
  QgsMultiLineString rc;
  rc.clear();
  rc.insertGeometry( nullptr, 0 );
  QVERIFY( rc.isEmpty() );
  QCOMPARE( rc.numGeometries(), 0 );
  rc.insertGeometry( nullptr, -1 );
  QVERIFY( rc.isEmpty() );
  QCOMPARE( rc.numGeometries(), 0 );
  rc.insertGeometry( nullptr, 100 );
  QVERIFY( rc.isEmpty() );
  QCOMPARE( rc.numGeometries(), 0 );

  rc.insertGeometry( new QgsPoint(), 0 );
  QVERIFY( rc.isEmpty() );
  QCOMPARE( rc.numGeometries(), 0 );

  rc.insertGeometry( part.clone(), 0 );
  QVERIFY( !rc.isEmpty() );
  QCOMPARE( rc.numGeometries(), 1 );

  // cast
  QVERIFY( !QgsMultiLineString().cast( nullptr ) );
  QgsMultiLineString pCast;
  QVERIFY( QgsMultiLineString().cast( &pCast ) );
  QgsMultiLineString pCast2;
  pCast2.fromWkt( QStringLiteral( "MultiLineStringZ()" ) );
  QVERIFY( QgsMultiLineString().cast( &pCast2 ) );
  pCast2.fromWkt( QStringLiteral( "MultiLineStringM()" ) );
  QVERIFY( QgsMultiLineString().cast( &pCast2 ) );
  pCast2.fromWkt( QStringLiteral( "MultiLineStringZM()" ) );
  QVERIFY( QgsMultiLineString().cast( &pCast2 ) );

  //boundary
  QgsMultiLineString multiLine1;
  QVERIFY( !multiLine1.boundary() );
  QgsLineString boundaryLine1;
  boundaryLine1.setPoints( QVector<QgsPoint>() << QgsPoint( 0, 0 ) << QgsPoint( 1, 0 ) << QgsPoint( 1, 1 ) );
  multiLine1.addGeometry( boundaryLine1.clone() );
  QgsAbstractGeometry *boundary = multiLine1.boundary();
  QgsMultiPoint *mpBoundary = dynamic_cast< QgsMultiPoint * >( boundary );
  QVERIFY( mpBoundary );
  QCOMPARE( mpBoundary->numGeometries(), 2 );
  QCOMPARE( static_cast< QgsPoint *>( mpBoundary->geometryN( 0 ) )->x(), 0.0 );
  QCOMPARE( static_cast< QgsPoint *>( mpBoundary->geometryN( 0 ) )->y(), 0.0 );
  QCOMPARE( static_cast< QgsPoint *>( mpBoundary->geometryN( 1 ) )->x(), 1.0 );
  QCOMPARE( static_cast< QgsPoint *>( mpBoundary->geometryN( 1 ) )->y(), 1.0 );
  delete boundary;
  // add another linestring
  QgsLineString boundaryLine2;
  boundaryLine2.setPoints( QVector<QgsPoint>() << QgsPoint( 10, 10 ) << QgsPoint( 11, 10 ) << QgsPoint( 11, 11 ) );
  multiLine1.addGeometry( boundaryLine2.clone() );
  boundary = multiLine1.boundary();
  mpBoundary = dynamic_cast< QgsMultiPoint * >( boundary );
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

  // vertex iterator: 2 linestrings with 3 points each
  QgsAbstractGeometry::vertex_iterator it = multiLine1.vertices_begin();
  QgsAbstractGeometry::vertex_iterator itEnd = multiLine1.vertices_end();
  QCOMPARE( *it, QgsPoint( 0, 0 ) );
  QCOMPARE( it.vertexId(), QgsVertexId( 0, 0, 0 ) );
  ++it;
  QCOMPARE( *it, QgsPoint( 1, 0 ) );
  QCOMPARE( it.vertexId(), QgsVertexId( 0, 0, 1 ) );
  ++it;
  QCOMPARE( *it, QgsPoint( 1, 1 ) );
  QCOMPARE( it.vertexId(), QgsVertexId( 0, 0, 2 ) );
  ++it;
  QCOMPARE( *it, QgsPoint( 10, 10 ) );
  QCOMPARE( it.vertexId(), QgsVertexId( 1, 0, 0 ) );
  ++it;
  QCOMPARE( *it, QgsPoint( 11, 10 ) );
  QCOMPARE( it.vertexId(), QgsVertexId( 1, 0, 1 ) );
  ++it;
  QCOMPARE( *it, QgsPoint( 11, 11 ) );
  QCOMPARE( it.vertexId(), QgsVertexId( 1, 0, 2 ) );
  ++it;
  QCOMPARE( it, itEnd );

  // Java-style iterator
  QgsVertexIterator it2( &multiLine1 );
  QVERIFY( it2.hasNext() );
  QCOMPARE( it2.next(), QgsPoint( 0, 0 ) );
  QVERIFY( it2.hasNext() );
  QCOMPARE( it2.next(), QgsPoint( 1, 0 ) );
  QVERIFY( it2.hasNext() );
  QCOMPARE( it2.next(), QgsPoint( 1, 1 ) );
  QVERIFY( it2.hasNext() );
  QCOMPARE( it2.next(), QgsPoint( 10, 10 ) );
  QVERIFY( it2.hasNext() );
  QCOMPARE( it2.next(), QgsPoint( 11, 10 ) );
  QVERIFY( it2.hasNext() );
  QCOMPARE( it2.next(), QgsPoint( 11, 11 ) );
  QVERIFY( !it2.hasNext() );

  // add a closed string = no boundary
  QgsLineString boundaryLine3;
  boundaryLine3.setPoints( QVector<QgsPoint>() << QgsPoint( 20, 20 ) << QgsPoint( 21, 20 ) << QgsPoint( 21, 21 ) << QgsPoint( 20, 20 ) );
  multiLine1.addGeometry( boundaryLine3.clone() );
  boundary = multiLine1.boundary();
  mpBoundary = dynamic_cast< QgsMultiPoint * >( boundary );
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
  boundaryLine4.setPoints( QVector<QgsPoint>() << QgsPoint( QgsWkbTypes::PointZ, 0, 0, 10 ) << QgsPoint( QgsWkbTypes::PointZ, 1, 0, 15 ) << QgsPoint( QgsWkbTypes::PointZ, 1, 1, 20 ) );
  QgsLineString boundaryLine5;
  boundaryLine5.setPoints( QVector<QgsPoint>() << QgsPoint( QgsWkbTypes::PointZ, 10, 10, 100 ) << QgsPoint( QgsWkbTypes::PointZ, 10, 20, 150 ) << QgsPoint( QgsWkbTypes::PointZ, 20, 20, 200 ) );
  QgsMultiLineString multiLine2;
  multiLine2.addGeometry( boundaryLine4.clone() );
  multiLine2.addGeometry( boundaryLine5.clone() );

  boundary = multiLine2.boundary();
  mpBoundary = dynamic_cast< QgsMultiPoint * >( boundary );
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

  //segmentLength
  QgsMultiLineString multiLine3;
  QgsLineString vertexLine3;
  QCOMPARE( multiLine3.segmentLength( QgsVertexId( -1, 0, 0 ) ), 0.0 );
  QCOMPARE( multiLine3.segmentLength( QgsVertexId( 0, 0, 0 ) ), 0.0 );
  QCOMPARE( multiLine3.segmentLength( QgsVertexId( 1, 0, 0 ) ), 0.0 );
  QCOMPARE( multiLine3.segmentLength( QgsVertexId( 0, -1, 0 ) ), 0.0 );
  QCOMPARE( multiLine3.segmentLength( QgsVertexId( 0, 1, 0 ) ), 0.0 );
  vertexLine3.setPoints( QgsPointSequence() << QgsPoint( 11, 2 ) << QgsPoint( 11, 12 ) << QgsPoint( 111, 12 )
                         << QgsPoint( 111, 2 ) << QgsPoint( 11, 2 ) );
  multiLine3.addGeometry( vertexLine3.clone() );
  QCOMPARE( multiLine3.segmentLength( QgsVertexId() ), 0.0 );
  QCOMPARE( multiLine3.segmentLength( QgsVertexId( 0, 0, -1 ) ), 0.0 );
  QCOMPARE( multiLine3.segmentLength( QgsVertexId( 0, 0, 0 ) ), 10.0 );
  QCOMPARE( multiLine3.segmentLength( QgsVertexId( 0, 0, 1 ) ), 100.0 );
  QCOMPARE( multiLine3.segmentLength( QgsVertexId( 0, 0, 2 ) ), 10.0 );
  QCOMPARE( multiLine3.segmentLength( QgsVertexId( 0, 0, 3 ) ), 100.0 );
  QCOMPARE( multiLine3.segmentLength( QgsVertexId( 0, 0, 4 ) ), 0.0 );
  QCOMPARE( multiLine3.segmentLength( QgsVertexId( -1, 0, -1 ) ), 0.0 );
  QCOMPARE( multiLine3.segmentLength( QgsVertexId( -1, 0, 0 ) ), 0.0 );
  QCOMPARE( multiLine3.segmentLength( QgsVertexId( 0, -1, 0 ) ), 10.0 );
  QCOMPARE( multiLine3.segmentLength( QgsVertexId( 1, 1, 0 ) ), 0.0 );
  QCOMPARE( multiLine3.segmentLength( QgsVertexId( 1, 1, 1 ) ), 0.0 );
  QCOMPARE( multiLine3.segmentLength( QgsVertexId( 1, 0, 1 ) ), 0.0 );

  // add another line
  vertexLine3.setPoints( QgsPointSequence() << QgsPoint( 30, 6 ) << QgsPoint( 34, 6 ) << QgsPoint( 34, 8 )
                         << QgsPoint( 30, 8 ) << QgsPoint( 30, 6 ) );
  multiLine3.addGeometry( vertexLine3.clone() );
  QCOMPARE( multiLine3.segmentLength( QgsVertexId() ), 0.0 );
  QCOMPARE( multiLine3.segmentLength( QgsVertexId( 0, 0, -1 ) ), 0.0 );
  QCOMPARE( multiLine3.segmentLength( QgsVertexId( 0, 0, 0 ) ), 10.0 );
  QCOMPARE( multiLine3.segmentLength( QgsVertexId( 0, 0, 1 ) ), 100.0 );
  QCOMPARE( multiLine3.segmentLength( QgsVertexId( 0, 0, 2 ) ), 10.0 );
  QCOMPARE( multiLine3.segmentLength( QgsVertexId( 0, 0, 3 ) ), 100.0 );
  QCOMPARE( multiLine3.segmentLength( QgsVertexId( 0, 0, 4 ) ), 0.0 );
  QCOMPARE( multiLine3.segmentLength( QgsVertexId( -1, 0, -1 ) ), 0.0 );
  QCOMPARE( multiLine3.segmentLength( QgsVertexId( -1, 0, 0 ) ), 0.0 );
  QCOMPARE( multiLine3.segmentLength( QgsVertexId( 0, -1, 0 ) ), 10.0 );
  QCOMPARE( multiLine3.segmentLength( QgsVertexId( 1, 0, -1 ) ), 0.0 );
  QCOMPARE( multiLine3.segmentLength( QgsVertexId( 1, 0, 0 ) ), 4.0 );
  QCOMPARE( multiLine3.segmentLength( QgsVertexId( 1, 0, 1 ) ), 2.0 );
  QCOMPARE( multiLine3.segmentLength( QgsVertexId( 1, 0, 2 ) ), 4.0 );
  QCOMPARE( multiLine3.segmentLength( QgsVertexId( 1, 0, 3 ) ), 2.0 );
  QCOMPARE( multiLine3.segmentLength( QgsVertexId( 1, 0, 4 ) ), 0.0 );
  QCOMPARE( multiLine3.segmentLength( QgsVertexId( 1, 1, 1 ) ), 2.0 );
  QCOMPARE( multiLine3.segmentLength( QgsVertexId( 1, 1, 2 ) ), 4.0 );
  QCOMPARE( multiLine3.segmentLength( QgsVertexId( 2, 0, 0 ) ), 0.0 );
}

void TestQgsGeometry::multiCurve()
{
  //test constructor
  QgsMultiCurve c1;
  QVERIFY( c1.isEmpty() );
  QCOMPARE( c1.nCoordinates(), 0 );
  QCOMPARE( c1.ringCount(), 0 );
  QCOMPARE( c1.partCount(), 0 );
  QVERIFY( !c1.is3D() );
  QVERIFY( !c1.isMeasure() );
  QCOMPARE( c1.wkbType(), QgsWkbTypes::MultiCurve );
  QCOMPARE( c1.wktTypeStr(), QString( "MultiCurve" ) );
  QCOMPARE( c1.geometryType(), QString( "MultiCurve" ) );
  QCOMPARE( c1.dimension(), 0 );
  QVERIFY( !c1.hasCurvedSegments() );
  QCOMPARE( c1.area(), 0.0 );
  QCOMPARE( c1.perimeter(), 0.0 );
  QCOMPARE( c1.numGeometries(), 0 );
  QVERIFY( !c1.geometryN( 0 ) );
  QVERIFY( !c1.geometryN( -1 ) );
  QCOMPARE( c1.vertexCount( 0, 0 ), 0 );
  QCOMPARE( c1.vertexCount( 0, 1 ), 0 );
  QCOMPARE( c1.vertexCount( 1, 0 ), 0 );

  //addGeometry
  //try with nullptr
  c1.addGeometry( nullptr );
  QVERIFY( c1.isEmpty() );
  QCOMPARE( c1.nCoordinates(), 0 );
  QCOMPARE( c1.ringCount(), 0 );
  QCOMPARE( c1.partCount(), 0 );
  QCOMPARE( c1.numGeometries(), 0 );
  QCOMPARE( c1.wkbType(), QgsWkbTypes::MultiCurve );
  QVERIFY( !c1.geometryN( 0 ) );
  QVERIFY( !c1.geometryN( -1 ) );

  // not a curve
  QVERIFY( !c1.addGeometry( new QgsPoint() ) );
  QVERIFY( c1.isEmpty() );
  QCOMPARE( c1.nCoordinates(), 0 );
  QCOMPARE( c1.ringCount(), 0 );
  QCOMPARE( c1.partCount(), 0 );
  QCOMPARE( c1.numGeometries(), 0 );
  QCOMPARE( c1.wkbType(), QgsWkbTypes::MultiCurve );
  QVERIFY( !c1.geometryN( 0 ) );
  QVERIFY( !c1.geometryN( -1 ) );

  //valid geometry
  QgsCircularString part;
  part.setPoints( QgsPointSequence() << QgsPoint( 1, 10 ) << QgsPoint( 2, 11 ) << QgsPoint( 1, 12 ) );
  c1.addGeometry( part.clone() );
  QVERIFY( !c1.isEmpty() );
  QCOMPARE( c1.numGeometries(), 1 );
  QCOMPARE( c1.nCoordinates(), 3 );
  QCOMPARE( c1.ringCount(), 1 );
  QCOMPARE( c1.partCount(), 1 );
  QVERIFY( !c1.is3D() );
  QVERIFY( !c1.isMeasure() );
  QCOMPARE( c1.wkbType(), QgsWkbTypes::MultiCurve );
  QCOMPARE( c1.wktTypeStr(), QString( "MultiCurve" ) );
  QCOMPARE( c1.geometryType(), QString( "MultiCurve" ) );
  QCOMPARE( c1.dimension(), 1 );
  QVERIFY( c1.hasCurvedSegments() );
  QCOMPARE( c1.area(), 0.0 );
  QCOMPARE( c1.perimeter(), 0.0 );
  QVERIFY( c1.geometryN( 0 ) );
  QCOMPARE( *static_cast< const QgsCircularString * >( c1.geometryN( 0 ) ), part );
  QVERIFY( !c1.geometryN( 100 ) );
  QVERIFY( !c1.geometryN( -1 ) );
  QCOMPARE( c1.vertexCount( 0, 0 ), 3 );
  QCOMPARE( c1.vertexCount( 1, 0 ), 0 );

  //initial adding of geometry should set z/m type
  part.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZ, 10, 11, 1 ) << QgsPoint( QgsWkbTypes::PointZ, 20, 21, 2 )
                  << QgsPoint( QgsWkbTypes::PointZ, 10, 31, 3 ) );
  QgsMultiCurve c2;
  c2.addGeometry( part.clone() );
  QVERIFY( c2.is3D() );
  QVERIFY( !c2.isMeasure() );
  QCOMPARE( c2.wkbType(), QgsWkbTypes::MultiCurveZ );
  QCOMPARE( c2.wktTypeStr(), QString( "MultiCurveZ" ) );
  QCOMPARE( c2.geometryType(), QString( "MultiCurve" ) );
  QCOMPARE( *( static_cast< const QgsCircularString * >( c2.geometryN( 0 ) ) ), part );
  QgsMultiCurve c3;
  part.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointM, 10, 11, 0, 1 ) << QgsPoint( QgsWkbTypes::PointM, 20, 21, 0, 2 )
                  << QgsPoint( QgsWkbTypes::PointM, 10, 31, 0, 3 ) );
  c3.addGeometry( part.clone() );
  QVERIFY( !c3.is3D() );
  QVERIFY( c3.isMeasure() );
  QCOMPARE( c3.wkbType(), QgsWkbTypes::MultiCurveM );
  QCOMPARE( c3.wktTypeStr(), QString( "MultiCurveM" ) );
  QCOMPARE( *( static_cast< const QgsCircularString * >( c3.geometryN( 0 ) ) ), part );
  QgsMultiCurve c4;
  part.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 10, 11, 2, 1 ) << QgsPoint( QgsWkbTypes::PointZM, 20, 21, 3, 2 )
                  << QgsPoint( QgsWkbTypes::PointZM, 10, 31, 4, 5 ) );
  c4.addGeometry( part.clone() );
  QVERIFY( c4.is3D() );
  QVERIFY( c4.isMeasure() );
  QCOMPARE( c4.wkbType(), QgsWkbTypes::MultiCurveZM );
  QCOMPARE( c4.wktTypeStr(), QString( "MultiCurveZM" ) );
  QCOMPARE( *( static_cast< const QgsCircularString * >( c4.geometryN( 0 ) ) ), part );

  //add another part
  QgsMultiCurve c6;
  part.setPoints( QgsPointSequence() << QgsPoint( 1, 10 ) << QgsPoint( 2, 11 ) << QgsPoint( 1, 20 ) );
  c6.addGeometry( part.clone() );
  QCOMPARE( c6.vertexCount( 0, 0 ), 3 );
  part.setPoints( QgsPointSequence() << QgsPoint( 9, 12 ) << QgsPoint( 3, 13 )  << QgsPoint( 9, 20 ) );
  c6.addGeometry( part.clone() );
  QCOMPARE( c6.vertexCount( 1, 0 ), 3 );
  QCOMPARE( c6.numGeometries(), 2 );
  QVERIFY( c6.geometryN( 0 ) );
  QCOMPARE( *static_cast< const QgsCircularString * >( c6.geometryN( 1 ) ), part );

  //adding subsequent points should not alter z/m type, regardless of parts type
  c6.clear();
  c6.addGeometry( part.clone() );
  QCOMPARE( c6.wkbType(), QgsWkbTypes::MultiCurve );
  part.setPoints( QgsPointSequence() << QgsPoint( 1, 10, 2 ) << QgsPoint( 2, 11, 3 ) << QgsPoint( 1, 20, 4 ) ) ;
  c6.addGeometry( part.clone() );
  QCOMPARE( c6.wkbType(), QgsWkbTypes::MultiCurve );
  QCOMPARE( c6.vertexCount( 0, 0 ), 3 );
  QCOMPARE( c6.vertexCount( 1, 0 ), 3 );
  QCOMPARE( c6.vertexCount( 2, 0 ), 0 );
  QCOMPARE( c6.vertexCount( -1, 0 ), 0 );
  QCOMPARE( c6.nCoordinates(), 6 );
  QCOMPARE( c6.ringCount(), 1 );
  QCOMPARE( c6.partCount(), 2 );
  QVERIFY( !c6.is3D() );
  const QgsCircularString *ls = static_cast< const QgsCircularString * >( c6.geometryN( 0 ) );
  QCOMPARE( ls->pointN( 0 ), QgsPoint( 9, 12 ) );
  QCOMPARE( ls->pointN( 1 ), QgsPoint( 3, 13 ) );
  QCOMPARE( ls->pointN( 2 ), QgsPoint( 9, 20 ) );
  ls = static_cast< const QgsCircularString * >( c6.geometryN( 1 ) );
  QCOMPARE( ls->pointN( 0 ), QgsPoint( 1, 10 ) );
  QCOMPARE( ls->pointN( 1 ), QgsPoint( 2, 11 ) );
  QCOMPARE( ls->pointN( 2 ), QgsPoint( 1, 20 ) );
  part.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointM, 21, 30, 0, 2 ) << QgsPoint( QgsWkbTypes::PointM, 32, 41, 0, 3 )
                  << QgsPoint( QgsWkbTypes::PointM, 21, 51, 0, 4 ) ) ;
  c6.addGeometry( part.clone() );
  QCOMPARE( c6.wkbType(), QgsWkbTypes::MultiCurve );
  QCOMPARE( c6.vertexCount( 0, 0 ), 3 );
  QCOMPARE( c6.vertexCount( 1, 0 ), 3 );
  QCOMPARE( c6.vertexCount( 2, 0 ), 3 );
  QCOMPARE( c6.nCoordinates(), 9 );
  QCOMPARE( c6.ringCount(), 1 );
  QCOMPARE( c6.partCount(), 3 );
  QVERIFY( !c6.is3D() );
  QVERIFY( !c6.isMeasure() );
  ls = static_cast< const QgsCircularString * >( c6.geometryN( 2 ) );
  QCOMPARE( ls->pointN( 0 ), QgsPoint( 21, 30 ) );
  QCOMPARE( ls->pointN( 1 ), QgsPoint( 32, 41 ) );
  QCOMPARE( ls->pointN( 2 ), QgsPoint( 21, 51 ) );

  c6.clear();
  part.setPoints( QgsPointSequence() << QgsPoint( 1, 10, 2 ) << QgsPoint( 2, 11, 3 ) << QgsPoint( 1, 21, 4 ) ) ;
  c6.addGeometry( part.clone() );
  QCOMPARE( c6.wkbType(), QgsWkbTypes::MultiCurveZ );
  part.setPoints( QgsPointSequence() << QgsPoint( 2, 20 ) << QgsPoint( 3, 31 ) << QgsPoint( 2, 41 ) ) ;
  c6.addGeometry( part.clone() );
  QCOMPARE( c6.wkbType(), QgsWkbTypes::MultiCurveZ );
  QVERIFY( c6.is3D() );
  ls = static_cast< const QgsCircularString * >( c6.geometryN( 0 ) );
  QCOMPARE( ls->pointN( 0 ), QgsPoint( 1, 10, 2 ) );
  QCOMPARE( ls->pointN( 1 ), QgsPoint( 2, 11, 3 ) );
  QCOMPARE( ls->pointN( 2 ), QgsPoint( 1, 21, 4 ) );
  ls = static_cast< const QgsCircularString * >( c6.geometryN( 1 ) );
  QCOMPARE( ls->pointN( 0 ), QgsPoint( 2, 20, 0 ) );
  QCOMPARE( ls->pointN( 1 ), QgsPoint( 3, 31, 0 ) );
  QCOMPARE( ls->pointN( 2 ), QgsPoint( 2, 41, 0 ) );
  part.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointM, 5, 50, 0, 4 ) << QgsPoint( QgsWkbTypes::PointM, 6, 61, 0, 5 )
                  << QgsPoint( QgsWkbTypes::PointM, 5, 71, 0, 6 ) ) ;
  c6.addGeometry( part.clone() );
  QCOMPARE( c6.wkbType(), QgsWkbTypes::MultiCurveZ );
  QVERIFY( c6.is3D() );
  QVERIFY( !c6.isMeasure() );
  ls = static_cast< const QgsCircularString * >( c6.geometryN( 2 ) );
  QCOMPARE( ls->pointN( 0 ), QgsPoint( 5, 50, 0 ) );
  QCOMPARE( ls->pointN( 1 ), QgsPoint( 6, 61, 0 ) );
  QCOMPARE( ls->pointN( 2 ), QgsPoint( 5, 71, 0 ) );

  c6.clear();
  QCOMPARE( c6.wkbType(), QgsWkbTypes::MultiCurve );
  part.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointM, 5, 50, 0, 4 ) << QgsPoint( QgsWkbTypes::PointM, 6, 61, 0, 5 )
                  << QgsPoint( QgsWkbTypes::PointM, 5, 71, 0, 5 ) ) ;
  c6.addGeometry( part.clone() );
  QCOMPARE( c6.wkbType(), QgsWkbTypes::MultiCurveM );
  part.setPoints( QgsPointSequence() << QgsPoint( 2, 20 ) << QgsPoint( 3, 31 )   << QgsPoint( 2, 41 ) ) ;
  c6.addGeometry( part.clone() );
  QCOMPARE( c6.wkbType(), QgsWkbTypes::MultiCurveM );
  QVERIFY( c6.isMeasure() );
  ls = static_cast< const QgsCircularString * >( c6.geometryN( 0 ) );
  QCOMPARE( ls->pointN( 0 ), QgsPoint( QgsWkbTypes::PointM, 5, 50, 0, 4 ) );
  QCOMPARE( ls->pointN( 1 ), QgsPoint( QgsWkbTypes::PointM, 6, 61, 0, 5 ) );
  QCOMPARE( ls->pointN( 2 ), QgsPoint( QgsWkbTypes::PointM, 5, 71, 0, 5 ) );
  ls = static_cast< const QgsCircularString * >( c6.geometryN( 1 ) );
  QCOMPARE( ls->pointN( 0 ), QgsPoint( QgsWkbTypes::PointM, 2, 20, 0, 0 ) );
  QCOMPARE( ls->pointN( 1 ), QgsPoint( QgsWkbTypes::PointM, 3, 31, 0, 0 ) );
  QCOMPARE( ls->pointN( 2 ), QgsPoint( QgsWkbTypes::PointM, 2, 41, 0, 0 ) );
  part.setPoints( QgsPointSequence() << QgsPoint( 11, 12, 13 ) << QgsPoint( 14, 15, 16 ) << QgsPoint( 11, 25, 17 ) ) ;
  c6.addGeometry( part.clone() );
  QCOMPARE( c6.wkbType(), QgsWkbTypes::MultiCurveM );
  QVERIFY( !c6.is3D() );
  QVERIFY( c6.isMeasure() );
  ls = static_cast< const QgsCircularString * >( c6.geometryN( 2 ) );
  QCOMPARE( ls->pointN( 0 ), QgsPoint( QgsWkbTypes::PointM, 11, 12, 0, 0 ) );
  QCOMPARE( ls->pointN( 1 ), QgsPoint( QgsWkbTypes::PointM, 14, 15, 0, 0 ) );
  QCOMPARE( ls->pointN( 2 ), QgsPoint( QgsWkbTypes::PointM, 11, 25, 0, 0 ) );

  c6.clear();
  part.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 5, 50, 1, 4 ) << QgsPoint( QgsWkbTypes::PointZM, 6, 61, 3, 5 )
                  << QgsPoint( QgsWkbTypes::PointZM, 5, 71, 4, 6 ) ) ;
  c6.addGeometry( part.clone() );
  QCOMPARE( c6.wkbType(), QgsWkbTypes::MultiCurveZM );
  part.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::Point, 7, 17 ) << QgsPoint( QgsWkbTypes::Point, 3, 13 )
                  << QgsPoint( QgsWkbTypes::Point, 7, 11 ) ) ;
  c6.addGeometry( part.clone() );
  QCOMPARE( c6.wkbType(), QgsWkbTypes::MultiCurveZM );
  QVERIFY( c6.isMeasure() );
  QVERIFY( c6.is3D() );
  ls = static_cast< const QgsCircularString * >( c6.geometryN( 0 ) );
  QCOMPARE( ls->pointN( 0 ), QgsPoint( QgsWkbTypes::PointZM, 5, 50, 1, 4 ) );
  QCOMPARE( ls->pointN( 1 ), QgsPoint( QgsWkbTypes::PointZM, 6, 61, 3, 5 ) );
  QCOMPARE( ls->pointN( 2 ), QgsPoint( QgsWkbTypes::PointZM, 5, 71, 4, 6 ) );
  ls = static_cast< const QgsCircularString * >( c6.geometryN( 1 ) );
  QCOMPARE( ls->pointN( 0 ), QgsPoint( QgsWkbTypes::PointZM, 7, 17, 0, 0 ) );
  QCOMPARE( ls->pointN( 1 ), QgsPoint( QgsWkbTypes::PointZM, 3, 13, 0, 0 ) );
  QCOMPARE( ls->pointN( 2 ), QgsPoint( QgsWkbTypes::PointZM, 7, 11, 0, 0 ) );
  part.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZ, 77, 87, 7 ) << QgsPoint( QgsWkbTypes::PointZ, 83, 83, 8 )
                  << QgsPoint( QgsWkbTypes::PointZ, 77, 81, 9 ) ) ;
  c6.addGeometry( part.clone() );
  QCOMPARE( c6.wkbType(), QgsWkbTypes::MultiCurveZM );
  QVERIFY( c6.is3D() );
  QVERIFY( c6.isMeasure() );
  ls = static_cast< const QgsCircularString * >( c6.geometryN( 2 ) );
  QCOMPARE( ls->pointN( 0 ), QgsPoint( QgsWkbTypes::PointZM, 77, 87, 7, 0 ) );
  QCOMPARE( ls->pointN( 1 ), QgsPoint( QgsWkbTypes::PointZM, 83, 83, 8, 0 ) );
  QCOMPARE( ls->pointN( 2 ), QgsPoint( QgsWkbTypes::PointZM, 77, 81, 9, 0 ) );
  part.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointM, 177, 187, 0, 9 ) << QgsPoint( QgsWkbTypes::PointM, 183, 183, 0, 11 )
                  << QgsPoint( QgsWkbTypes::PointM, 177, 181, 0, 13 ) ) ;
  c6.addGeometry( part.clone() );
  QCOMPARE( c6.wkbType(), QgsWkbTypes::MultiCurveZM );
  QVERIFY( c6.is3D() );
  QVERIFY( c6.isMeasure() );
  ls = static_cast< const QgsCircularString * >( c6.geometryN( 3 ) );
  QCOMPARE( ls->pointN( 0 ), QgsPoint( QgsWkbTypes::PointZM, 177, 187, 0, 9 ) );
  QCOMPARE( ls->pointN( 1 ), QgsPoint( QgsWkbTypes::PointZM, 183, 183, 0, 11 ) );
  QCOMPARE( ls->pointN( 2 ), QgsPoint( QgsWkbTypes::PointZM, 177, 181, 0, 13 ) );

  //clear
  QgsMultiCurve c7;
  part.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 5, 50, 1, 4 ) << QgsPoint( QgsWkbTypes::PointZM, 6, 61, 3, 5 )
                  << QgsPoint( QgsWkbTypes::PointZM, 5, 71, 4, 6 ) ) ;
  c7.addGeometry( part.clone() );
  c7.addGeometry( part.clone() );
  QCOMPARE( c7.numGeometries(), 2 );
  c7.clear();
  QVERIFY( c7.isEmpty() );
  QCOMPARE( c7.numGeometries(), 0 );
  QCOMPARE( c7.nCoordinates(), 0 );
  QCOMPARE( c7.ringCount(), 0 );
  QCOMPARE( c7.partCount(), 0 );
  QVERIFY( !c7.is3D() );
  QVERIFY( !c7.isMeasure() );
  QCOMPARE( c7.wkbType(), QgsWkbTypes::MultiCurve );

  //clone
  QgsMultiCurve c11;
  std::unique_ptr< QgsMultiCurve >cloned( c11.clone() );
  QVERIFY( cloned->isEmpty() );
  c11.addGeometry( part.clone() );
  c11.addGeometry( part.clone() );
  cloned.reset( c11.clone() );
  QCOMPARE( cloned->numGeometries(), 2 );
  ls = static_cast< const QgsCircularString * >( cloned->geometryN( 0 ) );
  QCOMPARE( *ls, part );
  ls = static_cast< const QgsCircularString * >( cloned->geometryN( 1 ) );
  QCOMPARE( *ls, part );

  //copy constructor
  QgsMultiCurve c12;
  QgsMultiCurve c13( c12 );
  QVERIFY( c13.isEmpty() );
  c12.addGeometry( part.clone() );
  c12.addGeometry( part.clone() );
  QgsMultiCurve c14( c12 );
  QCOMPARE( c14.numGeometries(), 2 );
  QCOMPARE( c14.wkbType(), QgsWkbTypes::MultiCurveZM );
  ls = static_cast< const QgsCircularString * >( c14.geometryN( 0 ) );
  QCOMPARE( *ls, part );
  ls = static_cast< const QgsCircularString * >( c14.geometryN( 1 ) );
  QCOMPARE( *ls, part );

  //assignment operator
  QgsMultiCurve c15;
  c15 = c13;
  QCOMPARE( c15.numGeometries(), 0 );
  c15 = c14;
  QCOMPARE( c15.numGeometries(), 2 );
  ls = static_cast< const QgsCircularString * >( c15.geometryN( 0 ) );
  QCOMPARE( *ls, part );
  ls = static_cast< const QgsCircularString * >( c15.geometryN( 1 ) );
  QCOMPARE( *ls, part );

  //toCurveType
  std::unique_ptr< QgsMultiCurve > curveType( c12.toCurveType() );
  QCOMPARE( curveType->wkbType(), QgsWkbTypes::MultiCurveZM );
  QCOMPARE( curveType->numGeometries(), 2 );
  const QgsCircularString *curve = static_cast< const QgsCircularString * >( curveType->geometryN( 0 ) );
  QCOMPARE( *curve, *static_cast< const QgsCircularString * >( c12.geometryN( 0 ) ) );
  curve = static_cast< const QgsCircularString * >( curveType->geometryN( 1 ) );
  QCOMPARE( *curve, *static_cast< const QgsCircularString * >( c12.geometryN( 1 ) ) );

  //to/fromWKB
  QgsMultiCurve c16;
  part.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::Point, 7, 17 ) << QgsPoint( QgsWkbTypes::Point, 3, 13 ) << QgsPoint( QgsWkbTypes::Point, 7, 11 ) ) ;
  c16.addGeometry( part.clone() );
  part.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::Point, 27, 37 ) << QgsPoint( QgsWkbTypes::Point, 43, 43 )  << QgsPoint( QgsWkbTypes::Point, 27, 54 ) ) ;
  c16.addGeometry( part.clone() );
  QByteArray wkb16 = c16.asWkb();
  QgsMultiCurve c17;
  QgsConstWkbPtr wkb16ptr( wkb16 );
  c17.fromWkb( wkb16ptr );
  QCOMPARE( c17.numGeometries(), 2 );
  QCOMPARE( *static_cast< const QgsCircularString * >( c17.geometryN( 0 ) ), *static_cast< const QgsCircularString * >( c16.geometryN( 0 ) ) );
  QCOMPARE( *static_cast< const QgsCircularString * >( c17.geometryN( 1 ) ), *static_cast< const QgsCircularString * >( c16.geometryN( 1 ) ) );

  //parts with Z
  c16.clear();
  c17.clear();
  part.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZ, 7, 17, 1 ) << QgsPoint( QgsWkbTypes::PointZ, 3, 13, 4 ) << QgsPoint( QgsWkbTypes::PointZ, 7, 11, 3 ) ) ;
  c16.addGeometry( part.clone() );
  part.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZ, 27, 37, 2 ) << QgsPoint( QgsWkbTypes::PointZ, 43, 43, 5 ) << QgsPoint( QgsWkbTypes::PointZ, 27, 53, 6 ) ) ;
  c16.addGeometry( part.clone() );
  wkb16 = c16.asWkb();
  QgsConstWkbPtr wkb16ptr2( wkb16 );
  c17.fromWkb( wkb16ptr2 );
  QCOMPARE( c17.numGeometries(), 2 );
  QCOMPARE( c17.wkbType(), QgsWkbTypes::MultiCurveZ );
  QCOMPARE( *static_cast< const QgsCircularString * >( c17.geometryN( 0 ) ), *static_cast< const QgsCircularString * >( c16.geometryN( 0 ) ) );
  QCOMPARE( *static_cast< const QgsCircularString * >( c17.geometryN( 1 ) ), *static_cast< const QgsCircularString * >( c16.geometryN( 1 ) ) );

  //parts with m
  c16.clear();
  c17.clear();
  part.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointM, 7, 17, 0, 1 ) << QgsPoint( QgsWkbTypes::PointM, 3, 13, 0, 4 ) << QgsPoint( QgsWkbTypes::PointM, 7, 11, 0, 5 ) ) ;
  c16.addGeometry( part.clone() );
  part.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointM, 27, 37, 0, 2 ) << QgsPoint( QgsWkbTypes::PointM, 43, 43, 0, 5 ) << QgsPoint( QgsWkbTypes::PointM, 27, 53, 0, 7 ) ) ;
  c16.addGeometry( part.clone() );
  wkb16 = c16.asWkb();
  QgsConstWkbPtr wkb16ptr3( wkb16 );
  c17.fromWkb( wkb16ptr3 );
  QCOMPARE( c17.numGeometries(), 2 );
  QCOMPARE( c17.wkbType(), QgsWkbTypes::MultiCurveM );
  QCOMPARE( *static_cast< const QgsCircularString * >( c17.geometryN( 0 ) ), *static_cast< const QgsCircularString * >( c16.geometryN( 0 ) ) );
  QCOMPARE( *static_cast< const QgsCircularString * >( c17.geometryN( 1 ) ), *static_cast< const QgsCircularString * >( c16.geometryN( 1 ) ) );

  // parts with ZM
  c16.clear();
  c17.clear();
  part.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 7, 17, 4, 1 ) << QgsPoint( QgsWkbTypes::PointZM, 3, 13, 1, 4 )  << QgsPoint( QgsWkbTypes::PointZM, 7, 11, 2, 5 ) ) ;
  c16.addGeometry( part.clone() );
  part.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 27, 37, 6, 2 ) << QgsPoint( QgsWkbTypes::PointZM, 43, 43, 11, 5 ) << QgsPoint( QgsWkbTypes::PointZM, 27, 47, 12, 15 ) ) ;
  c16.addGeometry( part.clone() );
  wkb16 = c16.asWkb();
  QgsConstWkbPtr wkb16ptr4( wkb16 );
  c17.fromWkb( wkb16ptr4 );
  QCOMPARE( c17.numGeometries(), 2 );
  QCOMPARE( c17.wkbType(), QgsWkbTypes::MultiCurveZM );
  QCOMPARE( *static_cast< const QgsCircularString * >( c17.geometryN( 0 ) ), *static_cast< const QgsCircularString * >( c16.geometryN( 0 ) ) );
  QCOMPARE( *static_cast< const QgsCircularString * >( c17.geometryN( 1 ) ), *static_cast< const QgsCircularString * >( c16.geometryN( 1 ) ) );

  //bad WKB - check for no crash
  c17.clear();
  QgsConstWkbPtr nullPtr( nullptr, 0 );
  QVERIFY( !c17.fromWkb( nullPtr ) );
  QCOMPARE( c17.wkbType(), QgsWkbTypes::MultiCurve );
  QgsPoint point( 1, 2 );
  QByteArray wkbPoint = point.asWkb();
  QgsConstWkbPtr wkbPointPtr( wkbPoint );
  QVERIFY( !c17.fromWkb( wkbPointPtr ) );
  QCOMPARE( c17.wkbType(), QgsWkbTypes::MultiCurve );

  //to/from WKT
  QgsMultiCurve c18;
  part.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 7, 17, 4, 1 ) << QgsPoint( QgsWkbTypes::PointZM, 3, 13, 1, 4 ) << QgsPoint( QgsWkbTypes::PointZM, 7, 11, 2, 8 ) ) ;
  c18.addGeometry( part.clone() );
  part.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 27, 37, 6, 2 ) << QgsPoint( QgsWkbTypes::PointZM, 43, 43, 11, 5 )  << QgsPoint( QgsWkbTypes::PointZM, 27, 53, 21, 52 ) ) ;
  c18.addGeometry( part.clone() );

  QString wkt = c18.asWkt();
  QVERIFY( !wkt.isEmpty() );
  QgsMultiCurve c19;
  QVERIFY( c19.fromWkt( wkt ) );
  QCOMPARE( c19.numGeometries(), 2 );
  QCOMPARE( *static_cast< const QgsCircularString * >( c19.geometryN( 0 ) ), *static_cast< const QgsCircularString * >( c18.geometryN( 0 ) ) );
  QCOMPARE( *static_cast< const QgsCircularString * >( c19.geometryN( 1 ) ), *static_cast< const QgsCircularString * >( c18.geometryN( 1 ) ) );

  //bad WKT
  QgsMultiCurve c20;
  QVERIFY( !c20.fromWkt( "Point()" ) );
  QVERIFY( c20.isEmpty() );
  QCOMPARE( c20.numGeometries(), 0 );
  QCOMPARE( c20.wkbType(), QgsWkbTypes::MultiCurve );

  //as JSON
  QgsMultiCurve exportC;
  part.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::Point, 7, 17 ) << QgsPoint( QgsWkbTypes::Point, 3, 13 ) << QgsPoint( QgsWkbTypes::Point, 7, 11 ) ) ;
  exportC.addGeometry( part.clone() );
  part.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::Point, 27, 37 ) << QgsPoint( QgsWkbTypes::Point, 43, 43 )  << QgsPoint( QgsWkbTypes::Point, 27, 47 ) ) ;
  exportC.addGeometry( part.clone() );

  // GML document for compare
  QDomDocument doc( "gml" );

  // as GML2
  QString expectedSimpleGML2( QStringLiteral( "<MultiLineString xmlns=\"gml\"><lineStringMember xmlns=\"gml\"><LineString xmlns=\"gml\"><coordinates xmlns=\"gml\" cs=\",\" ts=\" \">7,17 6.9,17 6.9,17 6.8,17 6.8,17.1 6.7,17.1 6.7,17.1 6.6,17.1 6.6,17.1 6.5,17.1 6.5,17.1 6.4,17.1 6.4,17.1 6.3,17.1 6.2,17.2 6.2,17.2 6.1,17.2 6.1,17.2 6,17.2 6,17.2 5.9,17.2 5.9,17.2 5.8,17.2 5.7,17.2 5.7,17.1 5.6,17.1 5.6,17.1 5.5,17.1 5.5,17.1 5.4,17.1 5.4,17.1 5.3,17.1 5.3,17.1 5.2,17.1 5.2,17 5.1,17 5,17 5,17 4.9,17 4.9,17 4.8,16.9 4.8,16.9 4.7,16.9 4.7,16.9 4.6,16.9 4.6,16.8 4.5,16.8 4.5,16.8 4.4,16.8 4.4,16.7 4.3,16.7 4.3,16.7 4.3,16.6 4.2,16.6 4.2,16.6 4.1,16.5 4.1,16.5 4,16.5 4,16.4 3.9,16.4 3.9,16.4 3.9,16.3 3.8,16.3 3.8,16.3 3.7,16.2 3.7,16.2 3.7,16.1 3.6,16.1 3.6,16.1 3.6,16 3.5,16 3.5,15.9 3.5,15.9 3.4,15.8 3.4,15.8 3.4,15.7 3.3,15.7 3.3,15.7 3.3,15.6 3.2,15.6 3.2,15.5 3.2,15.5 3.2,15.4 3.1,15.4 3.1,15.3 3.1,15.3 3.1,15.2 3.1,15.2 3,15.1 3,15.1 3,15 3,15 3,14.9 3,14.8 2.9,14.8 2.9,14.7 2.9,14.7 2.9,14.6 2.9,14.6 2.9,14.5 2.9,14.5 2.9,14.4 2.9,14.4 2.9,14.3 2.8,14.2 2.8,14.2 2.8,14.1 2.8,14.1 2.8,14 2.8,14 2.8,13.9 2.8,13.9 2.8,13.8 2.8,13.8 2.9,13.7 2.9,13.6 2.9,13.6 2.9,13.5 2.9,13.5 2.9,13.4 2.9,13.4 2.9,13.3 2.9,13.3 2.9,13.2 3,13.2 3,13.1 3,13 3,13 3,12.9 3,12.9 3.1,12.8 3.1,12.8 3.1,12.7 3.1,12.7 3.1,12.6 3.2,12.6 3.2,12.5 3.2,12.5 3.2,12.4 3.3,12.4 3.3,12.3 3.3,12.3 3.4,12.3 3.4,12.2 3.4,12.2 3.5,12.1 3.5,12.1 3.5,12 3.6,12 3.6,11.9 3.6,11.9 3.7,11.9 3.7,11.8 3.7,11.8 3.8,11.7 3.8,11.7 3.9,11.7 3.9,11.6 3.9,11.6 4,11.6 4,11.5 4.1,11.5 4.1,11.5 4.2,11.4 4.2,11.4 4.3,11.4 4.3,11.3 4.3,11.3 4.4,11.3 4.4,11.2 4.5,11.2 4.5,11.2 4.6,11.2 4.6,11.1 4.7,11.1 4.7,11.1 4.8,11.1 4.8,11.1 4.9,11 4.9,11 5,11 5,11 5.1,11 5.2,11 5.2,10.9 5.3,10.9 5.3,10.9 5.4,10.9 5.4,10.9 5.5,10.9 5.5,10.9 5.6,10.9 5.6,10.9 5.7,10.9 5.7,10.8 5.8,10.8 5.9,10.8 5.9,10.8 6,10.8 6,10.8 6.1,10.8 6.1,10.8 6.2,10.8 6.2,10.8 6.3,10.9 6.4,10.9 6.4,10.9 6.5,10.9 6.5,10.9 6.6,10.9 6.6,10.9 6.7,10.9 6.7,10.9 6.8,10.9 6.8,11 6.9,11 6.9,11 7,11</coordinates></LineString></lineStringMember><lineStringMember xmlns=\"gml\"><LineString xmlns=\"gml\"><coordinates xmlns=\"gml\" cs=\",\" ts=\" \">27,37 27.1,36.9 27.2,36.8 27.3,36.6 27.4,36.5 27.5,36.4 27.6,36.3 27.7,36.2 27.8,36 27.9,35.9 28,35.8 28.1,35.7 28.2,35.6 28.3,35.5 28.4,35.4 28.5,35.3 28.7,35.2 28.8,35.1 28.9,35 29,34.9 29.1,34.8 29.3,34.7 29.4,34.6 29.5,34.6 29.7,34.5 29.8,34.4 29.9,34.3 30.1,34.3 30.2,34.2 30.3,34.1 30.5,34 30.6,34 30.7,33.9 30.9,33.9 31,33.8 31.2,33.7 31.3,33.7 31.5,33.6 31.6,33.6 31.8,33.6 31.9,33.5 32.1,33.5 32.2,33.4 32.4,33.4 32.5,33.4 32.7,33.3 32.8,33.3 33,33.3 33.1,33.3 33.3,33.2 33.4,33.2 33.6,33.2 33.7,33.2 33.9,33.2 34,33.2 34.2,33.2 34.3,33.2 34.5,33.2 34.6,33.2 34.8,33.2 34.9,33.2 35.1,33.2 35.3,33.3 35.4,33.3 35.6,33.3 35.7,33.3 35.9,33.3 36,33.4 36.2,33.4 36.3,33.4 36.5,33.5 36.6,33.5 36.8,33.6 36.9,33.6 37.1,33.7 37.2,33.7 37.3,33.8 37.5,33.8 37.6,33.9 37.8,33.9 37.9,34 38,34.1 38.2,34.1 38.3,34.2 38.5,34.3 38.6,34.3 38.7,34.4 38.9,34.5 39,34.6 39.1,34.7 39.2,34.7 39.4,34.8 39.5,34.9 39.6,35 39.7,35.1 39.9,35.2 40,35.3 40.1,35.4 40.2,35.5 40.3,35.6 40.4,35.7 40.5,35.8 40.6,35.9 40.7,36.1 40.8,36.2 40.9,36.3 41,36.4 41.1,36.5 41.2,36.6 41.3,36.8 41.4,36.9 41.5,37 41.6,37.1 41.7,37.3 41.8,37.4 41.8,37.5 41.9,37.7 42,37.8 42.1,37.9 42.1,38.1 42.2,38.2 42.3,38.3 42.3,38.5 42.4,38.6 42.4,38.8 42.5,38.9 42.6,39.1 42.6,39.2 42.6,39.4 42.7,39.5 42.7,39.6 42.8,39.8 42.8,39.9 42.8,40.1 42.9,40.2 42.9,40.4 42.9,40.5 43,40.7 43,40.9 43,41 43,41.2 43,41.3 43,41.5 43,41.6 43.1,41.8 43.1,41.9 43.1,42.1 43.1,42.2 43,42.4 43,42.5 43,42.7 43,42.8 43,43 43,43.1 43,43.3 42.9,43.5 42.9,43.6 42.9,43.8 42.8,43.9 42.8,44.1 42.8,44.2 42.7,44.4 42.7,44.5 42.6,44.6 42.6,44.8 42.6,44.9 42.5,45.1 42.4,45.2 42.4,45.4 42.3,45.5 42.3,45.7 42.2,45.8 42.1,45.9 42.1,46.1 42,46.2 41.9,46.3 41.8,46.5 41.8,46.6 41.7,46.7 41.6,46.9 41.5,47 41.4,47.1 41.3,47.2 41.2,47.4 41.1,47.5 41,47.6 40.9,47.7 40.8,47.8 40.7,47.9 40.6,48.1 40.5,48.2 40.4,48.3 40.3,48.4 40.2,48.5 40.1,48.6 40,48.7 39.9,48.8 39.7,48.9 39.6,49 39.5,49.1 39.4,49.2 39.2,49.3 39.1,49.3 39,49.4 38.9,49.5 38.7,49.6 38.6,49.7 38.5,49.7 38.3,49.8 38.2,49.9 38,49.9 37.9,50 37.8,50.1 37.6,50.1 37.5,50.2 37.3,50.2 37.2,50.3 37.1,50.3 36.9,50.4 36.8,50.4 36.6,50.5 36.5,50.5 36.3,50.6 36.2,50.6 36,50.6 35.9,50.7 35.7,50.7 35.6,50.7 35.4,50.7 35.3,50.7 35.1,50.8 34.9,50.8 34.8,50.8 34.6,50.8 34.5,50.8 34.3,50.8 34.2,50.8 34,50.8 33.9,50.8 33.7,50.8 33.6,50.8 33.4,50.8 33.3,50.8 33.1,50.7 33,50.7 32.8,50.7 32.7,50.7 32.5,50.6 32.4,50.6 32.2,50.6 32.1,50.5 31.9,50.5 31.8,50.4 31.6,50.4 31.5,50.4 31.3,50.3 31.2,50.3 31,50.2 30.9,50.1 30.7,50.1 30.6,50 30.5,50 30.3,49.9 30.2,49.8 30.1,49.7 29.9,49.7 29.8,49.6 29.7,49.5 29.5,49.4 29.4,49.4 29.3,49.3 29.1,49.2 29,49.1 28.9,49 28.8,48.9 28.7,48.8 28.5,48.7 28.4,48.6 28.3,48.5 28.2,48.4 28.1,48.3 28,48.2 27.9,48.1 27.8,48 27.7,47.8 27.6,47.7 27.5,47.6 27.4,47.5 27.3,47.4 27.2,47.2 27.1,47.1 27,47</coordinates></LineString></lineStringMember></MultiLineString>" ) );
  QString res = elemToString( exportC.asGml2( doc, 1 ) );
  QGSCOMPAREGML( res, expectedSimpleGML2 );
  QString expectedGML2empty( QStringLiteral( "<MultiLineString xmlns=\"gml\"/>" ) );
  QGSCOMPAREGML( elemToString( QgsMultiCurve().asGml2( doc ) ), expectedGML2empty );

  //as GML3
  QString expectedSimpleGML3( QStringLiteral( "<MultiCurve xmlns=\"gml\"><curveMember xmlns=\"gml\"><Curve xmlns=\"gml\"><segments xmlns=\"gml\"><ArcString xmlns=\"gml\"><posList xmlns=\"gml\" srsDimension=\"2\">7 17 3 13 7 11</posList></ArcString></segments></Curve></curveMember><curveMember xmlns=\"gml\"><Curve xmlns=\"gml\"><segments xmlns=\"gml\"><ArcString xmlns=\"gml\"><posList xmlns=\"gml\" srsDimension=\"2\">27 37 43 43 27 47</posList></ArcString></segments></Curve></curveMember></MultiCurve>" ) );
  res = elemToString( exportC.asGml3( doc ) );
  QCOMPARE( res, expectedSimpleGML3 );
  QString expectedGML3empty( QStringLiteral( "<MultiCurve xmlns=\"gml\"/>" ) );
  QGSCOMPAREGML( elemToString( QgsMultiCurve().asGml3( doc ) ), expectedGML3empty );

  // as JSON
  QString expectedSimpleJson( "{\"type\": \"MultiLineString\", \"coordinates\": [[ [7, 17], [6.9, 17], [6.9, 17], [6.8, 17], [6.8, 17.1], [6.7, 17.1], [6.7, 17.1], [6.6, 17.1], [6.6, 17.1], [6.5, 17.1], [6.5, 17.1], [6.4, 17.1], [6.4, 17.1], [6.3, 17.1], [6.2, 17.2], [6.2, 17.2], [6.1, 17.2], [6.1, 17.2], [6, 17.2], [6, 17.2], [5.9, 17.2], [5.9, 17.2], [5.8, 17.2], [5.7, 17.2], [5.7, 17.1], [5.6, 17.1], [5.6, 17.1], [5.5, 17.1], [5.5, 17.1], [5.4, 17.1], [5.4, 17.1], [5.3, 17.1], [5.3, 17.1], [5.2, 17.1], [5.2, 17], [5.1, 17], [5, 17], [5, 17], [4.9, 17], [4.9, 17], [4.8, 16.9], [4.8, 16.9], [4.7, 16.9], [4.7, 16.9], [4.6, 16.9], [4.6, 16.8], [4.5, 16.8], [4.5, 16.8], [4.4, 16.8], [4.4, 16.7], [4.3, 16.7], [4.3, 16.7], [4.3, 16.6], [4.2, 16.6], [4.2, 16.6], [4.1, 16.5], [4.1, 16.5], [4, 16.5], [4, 16.4], [3.9, 16.4], [3.9, 16.4], [3.9, 16.3], [3.8, 16.3], [3.8, 16.3], [3.7, 16.2], [3.7, 16.2], [3.7, 16.1], [3.6, 16.1], [3.6, 16.1], [3.6, 16], [3.5, 16], [3.5, 15.9], [3.5, 15.9], [3.4, 15.8], [3.4, 15.8], [3.4, 15.7], [3.3, 15.7], [3.3, 15.7], [3.3, 15.6], [3.2, 15.6], [3.2, 15.5], [3.2, 15.5], [3.2, 15.4], [3.1, 15.4], [3.1, 15.3], [3.1, 15.3], [3.1, 15.2], [3.1, 15.2], [3, 15.1], [3, 15.1], [3, 15], [3, 15], [3, 14.9], [3, 14.8], [2.9, 14.8], [2.9, 14.7], [2.9, 14.7], [2.9, 14.6], [2.9, 14.6], [2.9, 14.5], [2.9, 14.5], [2.9, 14.4], [2.9, 14.4], [2.9, 14.3], [2.8, 14.2], [2.8, 14.2], [2.8, 14.1], [2.8, 14.1], [2.8, 14], [2.8, 14], [2.8, 13.9], [2.8, 13.9], [2.8, 13.8], [2.8, 13.8], [2.9, 13.7], [2.9, 13.6], [2.9, 13.6], [2.9, 13.5], [2.9, 13.5], [2.9, 13.4], [2.9, 13.4], [2.9, 13.3], [2.9, 13.3], [2.9, 13.2], [3, 13.2], [3, 13.1], [3, 13], [3, 13], [3, 12.9], [3, 12.9], [3.1, 12.8], [3.1, 12.8], [3.1, 12.7], [3.1, 12.7], [3.1, 12.6], [3.2, 12.6], [3.2, 12.5], [3.2, 12.5], [3.2, 12.4], [3.3, 12.4], [3.3, 12.3], [3.3, 12.3], [3.4, 12.3], [3.4, 12.2], [3.4, 12.2], [3.5, 12.1], [3.5, 12.1], [3.5, 12], [3.6, 12], [3.6, 11.9], [3.6, 11.9], [3.7, 11.9], [3.7, 11.8], [3.7, 11.8], [3.8, 11.7], [3.8, 11.7], [3.9, 11.7], [3.9, 11.6], [3.9, 11.6], [4, 11.6], [4, 11.5], [4.1, 11.5], [4.1, 11.5], [4.2, 11.4], [4.2, 11.4], [4.3, 11.4], [4.3, 11.3], [4.3, 11.3], [4.4, 11.3], [4.4, 11.2], [4.5, 11.2], [4.5, 11.2], [4.6, 11.2], [4.6, 11.1], [4.7, 11.1], [4.7, 11.1], [4.8, 11.1], [4.8, 11.1], [4.9, 11], [4.9, 11], [5, 11], [5, 11], [5.1, 11], [5.2, 11], [5.2, 10.9], [5.3, 10.9], [5.3, 10.9], [5.4, 10.9], [5.4, 10.9], [5.5, 10.9], [5.5, 10.9], [5.6, 10.9], [5.6, 10.9], [5.7, 10.9], [5.7, 10.8], [5.8, 10.8], [5.9, 10.8], [5.9, 10.8], [6, 10.8], [6, 10.8], [6.1, 10.8], [6.1, 10.8], [6.2, 10.8], [6.2, 10.8], [6.3, 10.9], [6.4, 10.9], [6.4, 10.9], [6.5, 10.9], [6.5, 10.9], [6.6, 10.9], [6.6, 10.9], [6.7, 10.9], [6.7, 10.9], [6.8, 10.9], [6.8, 11], [6.9, 11], [6.9, 11], [7, 11]], [ [27, 37], [27.1, 36.9], [27.2, 36.8], [27.3, 36.6], [27.4, 36.5], [27.5, 36.4], [27.6, 36.3], [27.7, 36.2], [27.8, 36], [27.9, 35.9], [28, 35.8], [28.1, 35.7], [28.2, 35.6], [28.3, 35.5], [28.4, 35.4], [28.5, 35.3], [28.7, 35.2], [28.8, 35.1], [28.9, 35], [29, 34.9], [29.1, 34.8], [29.3, 34.7], [29.4, 34.6], [29.5, 34.6], [29.7, 34.5], [29.8, 34.4], [29.9, 34.3], [30.1, 34.3], [30.2, 34.2], [30.3, 34.1], [30.5, 34], [30.6, 34], [30.7, 33.9], [30.9, 33.9], [31, 33.8], [31.2, 33.7], [31.3, 33.7], [31.5, 33.6], [31.6, 33.6], [31.8, 33.6], [31.9, 33.5], [32.1, 33.5], [32.2, 33.4], [32.4, 33.4], [32.5, 33.4], [32.7, 33.3], [32.8, 33.3], [33, 33.3], [33.1, 33.3], [33.3, 33.2], [33.4, 33.2], [33.6, 33.2], [33.7, 33.2], [33.9, 33.2], [34, 33.2], [34.2, 33.2], [34.3, 33.2], [34.5, 33.2], [34.6, 33.2], [34.8, 33.2], [34.9, 33.2], [35.1, 33.2], [35.3, 33.3], [35.4, 33.3], [35.6, 33.3], [35.7, 33.3], [35.9, 33.3], [36, 33.4], [36.2, 33.4], [36.3, 33.4], [36.5, 33.5], [36.6, 33.5], [36.8, 33.6], [36.9, 33.6], [37.1, 33.7], [37.2, 33.7], [37.3, 33.8], [37.5, 33.8], [37.6, 33.9], [37.8, 33.9], [37.9, 34], [38, 34.1], [38.2, 34.1], [38.3, 34.2], [38.5, 34.3], [38.6, 34.3], [38.7, 34.4], [38.9, 34.5], [39, 34.6], [39.1, 34.7], [39.2, 34.7], [39.4, 34.8], [39.5, 34.9], [39.6, 35], [39.7, 35.1], [39.9, 35.2], [40, 35.3], [40.1, 35.4], [40.2, 35.5], [40.3, 35.6], [40.4, 35.7], [40.5, 35.8], [40.6, 35.9], [40.7, 36.1], [40.8, 36.2], [40.9, 36.3], [41, 36.4], [41.1, 36.5], [41.2, 36.6], [41.3, 36.8], [41.4, 36.9], [41.5, 37], [41.6, 37.1], [41.7, 37.3], [41.8, 37.4], [41.8, 37.5], [41.9, 37.7], [42, 37.8], [42.1, 37.9], [42.1, 38.1], [42.2, 38.2], [42.3, 38.3], [42.3, 38.5], [42.4, 38.6], [42.4, 38.8], [42.5, 38.9], [42.6, 39.1], [42.6, 39.2], [42.6, 39.4], [42.7, 39.5], [42.7, 39.6], [42.8, 39.8], [42.8, 39.9], [42.8, 40.1], [42.9, 40.2], [42.9, 40.4], [42.9, 40.5], [43, 40.7], [43, 40.9], [43, 41], [43, 41.2], [43, 41.3], [43, 41.5], [43, 41.6], [43.1, 41.8], [43.1, 41.9], [43.1, 42.1], [43.1, 42.2], [43, 42.4], [43, 42.5], [43, 42.7], [43, 42.8], [43, 43], [43, 43.1], [43, 43.3], [42.9, 43.5], [42.9, 43.6], [42.9, 43.8], [42.8, 43.9], [42.8, 44.1], [42.8, 44.2], [42.7, 44.4], [42.7, 44.5], [42.6, 44.6], [42.6, 44.8], [42.6, 44.9], [42.5, 45.1], [42.4, 45.2], [42.4, 45.4], [42.3, 45.5], [42.3, 45.7], [42.2, 45.8], [42.1, 45.9], [42.1, 46.1], [42, 46.2], [41.9, 46.3], [41.8, 46.5], [41.8, 46.6], [41.7, 46.7], [41.6, 46.9], [41.5, 47], [41.4, 47.1], [41.3, 47.2], [41.2, 47.4], [41.1, 47.5], [41, 47.6], [40.9, 47.7], [40.8, 47.8], [40.7, 47.9], [40.6, 48.1], [40.5, 48.2], [40.4, 48.3], [40.3, 48.4], [40.2, 48.5], [40.1, 48.6], [40, 48.7], [39.9, 48.8], [39.7, 48.9], [39.6, 49], [39.5, 49.1], [39.4, 49.2], [39.2, 49.3], [39.1, 49.3], [39, 49.4], [38.9, 49.5], [38.7, 49.6], [38.6, 49.7], [38.5, 49.7], [38.3, 49.8], [38.2, 49.9], [38, 49.9], [37.9, 50], [37.8, 50.1], [37.6, 50.1], [37.5, 50.2], [37.3, 50.2], [37.2, 50.3], [37.1, 50.3], [36.9, 50.4], [36.8, 50.4], [36.6, 50.5], [36.5, 50.5], [36.3, 50.6], [36.2, 50.6], [36, 50.6], [35.9, 50.7], [35.7, 50.7], [35.6, 50.7], [35.4, 50.7], [35.3, 50.7], [35.1, 50.8], [34.9, 50.8], [34.8, 50.8], [34.6, 50.8], [34.5, 50.8], [34.3, 50.8], [34.2, 50.8], [34, 50.8], [33.9, 50.8], [33.7, 50.8], [33.6, 50.8], [33.4, 50.8], [33.3, 50.8], [33.1, 50.7], [33, 50.7], [32.8, 50.7], [32.7, 50.7], [32.5, 50.6], [32.4, 50.6], [32.2, 50.6], [32.1, 50.5], [31.9, 50.5], [31.8, 50.4], [31.6, 50.4], [31.5, 50.4], [31.3, 50.3], [31.2, 50.3], [31, 50.2], [30.9, 50.1], [30.7, 50.1], [30.6, 50], [30.5, 50], [30.3, 49.9], [30.2, 49.8], [30.1, 49.7], [29.9, 49.7], [29.8, 49.6], [29.7, 49.5], [29.5, 49.4], [29.4, 49.4], [29.3, 49.3], [29.1, 49.2], [29, 49.1], [28.9, 49], [28.8, 48.9], [28.7, 48.8], [28.5, 48.7], [28.4, 48.6], [28.3, 48.5], [28.2, 48.4], [28.1, 48.3], [28, 48.2], [27.9, 48.1], [27.8, 48], [27.7, 47.8], [27.6, 47.7], [27.5, 47.6], [27.4, 47.5], [27.3, 47.4], [27.2, 47.2], [27.1, 47.1], [27, 47]]] }" );
  res = exportC.asJson( 1 );
  QCOMPARE( res, expectedSimpleJson );

  QgsMultiCurve exportFloat;
  part.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::Point, 7 / 3.0, 17 / 3.0 ) << QgsPoint( QgsWkbTypes::Point, 3 / 5.0, 13 / 3.0 ) << QgsPoint( QgsWkbTypes::Point, 7 / 3.0, 11 / 3.0 ) ) ;
  exportFloat.addGeometry( part.clone() );
  part.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::Point, 27 / 3.0, 37 / 9.0 ) << QgsPoint( QgsWkbTypes::Point, 43 / 41.0, 43 / 42.0 ) << QgsPoint( QgsWkbTypes::Point, 27 / 3.0, 1 / 3.0 ) ) ;
  exportFloat.addGeometry( part.clone() );

  QString expectedJsonPrec3( QStringLiteral( "{\"type\": \"MultiLineString\", \"coordinates\": [[ [2.333, 5.667], [2.316, 5.677], [2.298, 5.687], [2.28, 5.697], [2.262, 5.707], [2.244, 5.716], [2.226, 5.725], [2.207, 5.734], [2.188, 5.742], [2.17, 5.75], [2.151, 5.757], [2.131, 5.765], [2.112, 5.772], [2.093, 5.778], [2.074, 5.785], [2.054, 5.79], [2.034, 5.796], [2.015, 5.801], [1.995, 5.806], [1.975, 5.811], [1.955, 5.815], [1.935, 5.819], [1.915, 5.822], [1.894, 5.826], [1.874, 5.828], [1.854, 5.831], [1.834, 5.833], [1.813, 5.835], [1.793, 5.836], [1.773, 5.837], [1.752, 5.838], [1.732, 5.838], [1.711, 5.838], [1.691, 5.838], [1.67, 5.837], [1.65, 5.836], [1.63, 5.834], [1.609, 5.833], [1.589, 5.83], [1.569, 5.828], [1.548, 5.825], [1.528, 5.822], [1.508, 5.818], [1.488, 5.814], [1.468, 5.81], [1.448, 5.805], [1.428, 5.801], [1.409, 5.795], [1.389, 5.79], [1.37, 5.784], [1.35, 5.777], [1.331, 5.771], [1.312, 5.764], [1.293, 5.756], [1.274, 5.749], [1.255, 5.741], [1.236, 5.732], [1.218, 5.724], [1.199, 5.715], [1.181, 5.705], [1.163, 5.696], [1.145, 5.686], [1.128, 5.676], [1.11, 5.665], [1.093, 5.654], [1.076, 5.643], [1.059, 5.632], [1.042, 5.62], [1.025, 5.608], [1.009, 5.596], [0.993, 5.583], [0.977, 5.57], [0.962, 5.557], [0.946, 5.543], [0.931, 5.53], [0.916, 5.516], [0.901, 5.502], [0.887, 5.487], [0.873, 5.473], [0.859, 5.458], [0.845, 5.442], [0.832, 5.427], [0.819, 5.411], [0.806, 5.395], [0.793, 5.379], [0.781, 5.363], [0.769, 5.346], [0.757, 5.33], [0.746, 5.313], [0.735, 5.296], [0.724, 5.278], [0.713, 5.261], [0.703, 5.243], [0.693, 5.225], [0.684, 5.207], [0.674, 5.189], [0.666, 5.171], [0.657, 5.152], [0.649, 5.133], [0.641, 5.115], [0.633, 5.096], [0.626, 5.077], [0.619, 5.057], [0.612, 5.038], [0.606, 5.019], [0.6, 4.999], [0.594, 4.979], [0.589, 4.96], [0.584, 4.94], [0.579, 4.92], [0.575, 4.9], [0.571, 4.88], [0.568, 4.86], [0.564, 4.84], [0.562, 4.819], [0.559, 4.799], [0.557, 4.779], [0.555, 4.759], [0.554, 4.738], [0.553, 4.718], [0.552, 4.697], [0.552, 4.677], [0.552, 4.656], [0.552, 4.636], [0.553, 4.616], [0.554, 4.595], [0.555, 4.575], [0.557, 4.554], [0.559, 4.534], [0.562, 4.514], [0.564, 4.494], [0.568, 4.473], [0.571, 4.453], [0.575, 4.433], [0.579, 4.413], [0.584, 4.393], [0.589, 4.374], [0.594, 4.354], [0.6, 4.334], [0.606, 4.315], [0.612, 4.295], [0.619, 4.276], [0.626, 4.257], [0.633, 4.238], [0.641, 4.219], [0.649, 4.2], [0.657, 4.181], [0.666, 4.163], [0.674, 4.144], [0.684, 4.126], [0.693, 4.108], [0.703, 4.09], [0.713, 4.073], [0.724, 4.055], [0.735, 4.038], [0.746, 4.021], [0.757, 4.004], [0.769, 3.987], [0.781, 3.97], [0.793, 3.954], [0.806, 3.938], [0.819, 3.922], [0.832, 3.906], [0.845, 3.891], [0.859, 3.876], [0.873, 3.861], [0.887, 3.846], [0.901, 3.832], [0.916, 3.817], [0.931, 3.804], [0.946, 3.79], [0.962, 3.776], [0.977, 3.763], [0.993, 3.75], [1.009, 3.738], [1.025, 3.726], [1.042, 3.713], [1.059, 3.702], [1.076, 3.69], [1.093, 3.679], [1.11, 3.668], [1.128, 3.658], [1.145, 3.648], [1.163, 3.638], [1.181, 3.628], [1.199, 3.619], [1.218, 3.61], [1.236, 3.601], [1.255, 3.593], [1.274, 3.585], [1.293, 3.577], [1.312, 3.57], [1.331, 3.563], [1.35, 3.556], [1.37, 3.55], [1.389, 3.544], [1.409, 3.538], [1.428, 3.533], [1.448, 3.528], [1.468, 3.523], [1.488, 3.519], [1.508, 3.515], [1.528, 3.511], [1.548, 3.508], [1.569, 3.505], [1.589, 3.503], [1.609, 3.501], [1.63, 3.499], [1.65, 3.497], [1.67, 3.496], [1.691, 3.496], [1.711, 3.495], [1.732, 3.495], [1.752, 3.496], [1.773, 3.496], [1.793, 3.497], [1.813, 3.499], [1.834, 3.5], [1.854, 3.503], [1.874, 3.505], [1.894, 3.508], [1.915, 3.511], [1.935, 3.514], [1.955, 3.518], [1.975, 3.523], [1.995, 3.527], [2.015, 3.532], [2.034, 3.537], [2.054, 3.543], [2.074, 3.549], [2.093, 3.555], [2.112, 3.562], [2.131, 3.569], [2.151, 3.576], [2.17, 3.584], [2.188, 3.592], [2.207, 3.6], [2.226, 3.608], [2.244, 3.617], [2.262, 3.627], [2.28, 3.636], [2.298, 3.646], [2.316, 3.656], [2.333, 3.667]], [ [9, 4.111], [8.966, 4.178], [8.932, 4.244], [8.896, 4.309], [8.859, 4.374], [8.821, 4.438], [8.782, 4.502], [8.742, 4.565], [8.7, 4.627], [8.658, 4.688], [8.614, 4.749], [8.57, 4.809], [8.524, 4.868], [8.477, 4.926], [8.43, 4.983], [8.381, 5.04], [8.331, 5.096], [8.281, 5.151], [8.229, 5.205], [8.177, 5.258], [8.124, 5.31], [8.069, 5.361], [8.014, 5.411], [7.958, 5.461], [7.901, 5.509], [7.844, 5.556], [7.785, 5.603], [7.726, 5.648], [7.666, 5.692], [7.605, 5.735], [7.543, 5.777], [7.481, 5.818], [7.418, 5.858], [7.354, 5.897], [7.29, 5.935], [7.225, 5.971], [7.159, 6.007], [7.093, 6.041], [7.026, 6.074], [6.958, 6.106], [6.89, 6.137], [6.822, 6.167], [6.753, 6.195], [6.683, 6.222], [6.613, 6.248], [6.543, 6.273], [6.472, 6.296], [6.401, 6.319], [6.329, 6.34], [6.257, 6.36], [6.185, 6.378], [6.112, 6.395], [6.04, 6.411], [5.966, 6.426], [5.893, 6.44], [5.819, 6.452], [5.746, 6.463], [5.672, 6.472], [5.597, 6.48], [5.523, 6.487], [5.449, 6.493], [5.374, 6.498], [5.3, 6.501], [5.225, 6.503], [5.15, 6.503], [5.076, 6.502], [5.001, 6.5], [4.927, 6.497], [4.852, 6.492], [4.778, 6.486], [4.704, 6.479], [4.629, 6.47], [4.555, 6.46], [4.482, 6.449], [4.408, 6.437], [4.335, 6.423], [4.262, 6.408], [4.189, 6.392], [4.116, 6.374], [4.044, 6.355], [3.972, 6.335], [3.901, 6.314], [3.83, 6.292], [3.759, 6.268], [3.688, 6.243], [3.619, 6.217], [3.549, 6.189], [3.48, 6.16], [3.412, 6.131], [3.344, 6.1], [3.277, 6.067], [3.21, 6.034], [3.144, 5.999], [3.078, 5.964], [3.013, 5.927], [2.949, 5.889], [2.886, 5.85], [2.823, 5.81], [2.761, 5.768], [2.699, 5.726], [2.638, 5.683], [2.578, 5.638], [2.519, 5.593], [2.461, 5.546], [2.403, 5.499], [2.347, 5.45], [2.291, 5.401], [2.236, 5.35], [2.182, 5.299], [2.129, 5.246], [2.076, 5.193], [2.025, 5.139], [1.975, 5.084], [1.925, 5.028], [1.877, 4.971], [1.829, 4.914], [1.783, 4.855], [1.738, 4.796], [1.693, 4.736], [1.65, 4.675], [1.608, 4.614], [1.567, 4.551], [1.527, 4.488], [1.488, 4.425], [1.45, 4.36], [1.413, 4.295], [1.378, 4.23], [1.343, 4.164], [1.31, 4.097], [1.278, 4.029], [1.247, 3.961], [1.217, 3.893], [1.189, 3.824], [1.161, 3.755], [1.135, 3.685], [1.11, 3.614], [1.087, 3.544], [1.064, 3.472], [1.043, 3.401], [1.023, 3.329], [1.004, 3.257], [0.987, 3.184], [0.971, 3.111], [0.956, 3.038], [0.942, 2.965], [0.93, 2.891], [0.919, 2.817], [0.909, 2.743], [0.901, 2.669], [0.894, 2.595], [0.888, 2.52], [0.883, 2.446], [0.88, 2.371], [0.878, 2.297], [0.878, 2.222], [0.878, 2.148], [0.88, 2.073], [0.883, 1.998], [0.888, 1.924], [0.894, 1.85], [0.901, 1.775], [0.909, 1.701], [0.919, 1.627], [0.93, 1.553], [0.942, 1.48], [0.956, 1.406], [0.971, 1.333], [0.987, 1.26], [1.004, 1.188], [1.023, 1.116], [1.043, 1.044], [1.064, 0.972], [1.087, 0.901], [1.11, 0.83], [1.135, 0.76], [1.161, 0.69], [1.189, 0.62], [1.217, 0.551], [1.247, 0.483], [1.278, 0.415], [1.31, 0.348], [1.343, 0.281], [1.378, 0.215], [1.413, 0.149], [1.45, 0.084], [1.488, 0.02], [1.527, -0.044], [1.567, -0.107], [1.608, -0.169], [1.65, -0.231], [1.693, -0.291], [1.738, -0.351], [1.783, -0.411], [1.829, -0.469], [1.877, -0.527], [1.925, -0.584], [1.975, -0.639], [2.025, -0.694], [2.076, -0.749], [2.129, -0.802], [2.182, -0.854], [2.236, -0.906], [2.291, -0.956], [2.347, -1.006], [2.403, -1.054], [2.461, -1.102], [2.519, -1.148], [2.578, -1.194], [2.638, -1.238], [2.699, -1.282], [2.761, -1.324], [2.823, -1.365], [2.886, -1.405], [2.949, -1.444], [3.013, -1.482], [3.078, -1.519], [3.144, -1.555], [3.21, -1.589], [3.277, -1.623], [3.344, -1.655], [3.412, -1.686], [3.48, -1.716], [3.549, -1.745], [3.619, -1.772], [3.688, -1.798], [3.759, -1.823], [3.83, -1.847], [3.901, -1.87], [3.972, -1.891], [4.044, -1.911], [4.116, -1.93], [4.189, -1.947], [4.262, -1.964], [4.335, -1.979], [4.408, -1.992], [4.482, -2.005], [4.555, -2.016], [4.629, -2.026], [4.704, -2.034], [4.778, -2.042], [4.852, -2.048], [4.927, -2.052], [5.001, -2.056], [5.076, -2.058], [5.15, -2.059], [5.225, -2.058], [5.3, -2.056], [5.374, -2.053], [5.449, -2.049], [5.523, -2.043], [5.597, -2.036], [5.672, -2.028], [5.746, -2.018], [5.819, -2.007], [5.893, -1.995], [5.966, -1.982], [6.04, -1.967], [6.112, -1.951], [6.185, -1.934], [6.257, -1.915], [6.329, -1.895], [6.401, -1.874], [6.472, -1.852], [6.543, -1.829], [6.613, -1.804], [6.683, -1.778], [6.753, -1.751], [6.822, -1.722], [6.89, -1.693], [6.958, -1.662], [7.026, -1.63], [7.093, -1.597], [7.159, -1.562], [7.225, -1.527], [7.29, -1.49], [7.354, -1.453], [7.418, -1.414], [7.481, -1.374], [7.543, -1.333], [7.605, -1.291], [7.666, -1.248], [7.726, -1.203], [7.785, -1.158], [7.844, -1.112], [7.901, -1.065], [7.958, -1.016], [8.014, -0.967], [8.069, -0.917], [8.124, -0.865], [8.177, -0.813], [8.229, -0.76], [8.281, -0.706], [8.331, -0.651], [8.381, -0.596], [8.43, -0.539], [8.477, -0.482], [8.524, -0.423], [8.57, -0.364], [8.614, -0.304], [8.658, -0.244], [8.7, -0.182], [8.742, -0.12], [8.782, -0.057], [8.821, 0.006], [8.859, 0.07], [8.896, 0.135], [8.932, 0.201], [8.966, 0.267], [9, 0.333]]] }" ) );
  res = exportFloat.asJson( 3 );
  QCOMPARE( res, expectedJsonPrec3 );

  // as GML2
  QString expectedGML2prec3( QStringLiteral( "<MultiLineString xmlns=\"gml\"><lineStringMember xmlns=\"gml\"><LineString xmlns=\"gml\"><coordinates xmlns=\"gml\" cs=\",\" ts=\" \">2.333,5.667 2.316,5.677 2.298,5.687 2.28,5.697 2.262,5.707 2.244,5.716 2.226,5.725 2.207,5.734 2.188,5.742 2.17,5.75 2.151,5.757 2.131,5.765 2.112,5.772 2.093,5.778 2.074,5.785 2.054,5.79 2.034,5.796 2.015,5.801 1.995,5.806 1.975,5.811 1.955,5.815 1.935,5.819 1.915,5.822 1.894,5.826 1.874,5.828 1.854,5.831 1.834,5.833 1.813,5.835 1.793,5.836 1.773,5.837 1.752,5.838 1.732,5.838 1.711,5.838 1.691,5.838 1.67,5.837 1.65,5.836 1.63,5.834 1.609,5.833 1.589,5.83 1.569,5.828 1.548,5.825 1.528,5.822 1.508,5.818 1.488,5.814 1.468,5.81 1.448,5.805 1.428,5.801 1.409,5.795 1.389,5.79 1.37,5.784 1.35,5.777 1.331,5.771 1.312,5.764 1.293,5.756 1.274,5.749 1.255,5.741 1.236,5.732 1.218,5.724 1.199,5.715 1.181,5.705 1.163,5.696 1.145,5.686 1.128,5.676 1.11,5.665 1.093,5.654 1.076,5.643 1.059,5.632 1.042,5.62 1.025,5.608 1.009,5.596 0.993,5.583 0.977,5.57 0.962,5.557 0.946,5.543 0.931,5.53 0.916,5.516 0.901,5.502 0.887,5.487 0.873,5.473 0.859,5.458 0.845,5.442 0.832,5.427 0.819,5.411 0.806,5.395 0.793,5.379 0.781,5.363 0.769,5.346 0.757,5.33 0.746,5.313 0.735,5.296 0.724,5.278 0.713,5.261 0.703,5.243 0.693,5.225 0.684,5.207 0.674,5.189 0.666,5.171 0.657,5.152 0.649,5.133 0.641,5.115 0.633,5.096 0.626,5.077 0.619,5.057 0.612,5.038 0.606,5.019 0.6,4.999 0.594,4.979 0.589,4.96 0.584,4.94 0.579,4.92 0.575,4.9 0.571,4.88 0.568,4.86 0.564,4.84 0.562,4.819 0.559,4.799 0.557,4.779 0.555,4.759 0.554,4.738 0.553,4.718 0.552,4.697 0.552,4.677 0.552,4.656 0.552,4.636 0.553,4.616 0.554,4.595 0.555,4.575 0.557,4.554 0.559,4.534 0.562,4.514 0.564,4.494 0.568,4.473 0.571,4.453 0.575,4.433 0.579,4.413 0.584,4.393 0.589,4.374 0.594,4.354 0.6,4.334 0.606,4.315 0.612,4.295 0.619,4.276 0.626,4.257 0.633,4.238 0.641,4.219 0.649,4.2 0.657,4.181 0.666,4.163 0.674,4.144 0.684,4.126 0.693,4.108 0.703,4.09 0.713,4.073 0.724,4.055 0.735,4.038 0.746,4.021 0.757,4.004 0.769,3.987 0.781,3.97 0.793,3.954 0.806,3.938 0.819,3.922 0.832,3.906 0.845,3.891 0.859,3.876 0.873,3.861 0.887,3.846 0.901,3.832 0.916,3.817 0.931,3.804 0.946,3.79 0.962,3.776 0.977,3.763 0.993,3.75 1.009,3.738 1.025,3.726 1.042,3.713 1.059,3.702 1.076,3.69 1.093,3.679 1.11,3.668 1.128,3.658 1.145,3.648 1.163,3.638 1.181,3.628 1.199,3.619 1.218,3.61 1.236,3.601 1.255,3.593 1.274,3.585 1.293,3.577 1.312,3.57 1.331,3.563 1.35,3.556 1.37,3.55 1.389,3.544 1.409,3.538 1.428,3.533 1.448,3.528 1.468,3.523 1.488,3.519 1.508,3.515 1.528,3.511 1.548,3.508 1.569,3.505 1.589,3.503 1.609,3.501 1.63,3.499 1.65,3.497 1.67,3.496 1.691,3.496 1.711,3.495 1.732,3.495 1.752,3.496 1.773,3.496 1.793,3.497 1.813,3.499 1.834,3.5 1.854,3.503 1.874,3.505 1.894,3.508 1.915,3.511 1.935,3.514 1.955,3.518 1.975,3.523 1.995,3.527 2.015,3.532 2.034,3.537 2.054,3.543 2.074,3.549 2.093,3.555 2.112,3.562 2.131,3.569 2.151,3.576 2.17,3.584 2.188,3.592 2.207,3.6 2.226,3.608 2.244,3.617 2.262,3.627 2.28,3.636 2.298,3.646 2.316,3.656 2.333,3.667</coordinates></LineString></lineStringMember><lineStringMember xmlns=\"gml\"><LineString xmlns=\"gml\"><coordinates xmlns=\"gml\" cs=\",\" ts=\" \">9,4.111 8.966,4.178 8.932,4.244 8.896,4.309 8.859,4.374 8.821,4.438 8.782,4.502 8.742,4.565 8.7,4.627 8.658,4.688 8.614,4.749 8.57,4.809 8.524,4.868 8.477,4.926 8.43,4.983 8.381,5.04 8.331,5.096 8.281,5.151 8.229,5.205 8.177,5.258 8.124,5.31 8.069,5.361 8.014,5.411 7.958,5.461 7.901,5.509 7.844,5.556 7.785,5.603 7.726,5.648 7.666,5.692 7.605,5.735 7.543,5.777 7.481,5.818 7.418,5.858 7.354,5.897 7.29,5.935 7.225,5.971 7.159,6.007 7.093,6.041 7.026,6.074 6.958,6.106 6.89,6.137 6.822,6.167 6.753,6.195 6.683,6.222 6.613,6.248 6.543,6.273 6.472,6.296 6.401,6.319 6.329,6.34 6.257,6.36 6.185,6.378 6.112,6.395 6.04,6.411 5.966,6.426 5.893,6.44 5.819,6.452 5.746,6.463 5.672,6.472 5.597,6.48 5.523,6.487 5.449,6.493 5.374,6.498 5.3,6.501 5.225,6.503 5.15,6.503 5.076,6.502 5.001,6.5 4.927,6.497 4.852,6.492 4.778,6.486 4.704,6.479 4.629,6.47 4.555,6.46 4.482,6.449 4.408,6.437 4.335,6.423 4.262,6.408 4.189,6.392 4.116,6.374 4.044,6.355 3.972,6.335 3.901,6.314 3.83,6.292 3.759,6.268 3.688,6.243 3.619,6.217 3.549,6.189 3.48,6.16 3.412,6.131 3.344,6.1 3.277,6.067 3.21,6.034 3.144,5.999 3.078,5.964 3.013,5.927 2.949,5.889 2.886,5.85 2.823,5.81 2.761,5.768 2.699,5.726 2.638,5.683 2.578,5.638 2.519,5.593 2.461,5.546 2.403,5.499 2.347,5.45 2.291,5.401 2.236,5.35 2.182,5.299 2.129,5.246 2.076,5.193 2.025,5.139 1.975,5.084 1.925,5.028 1.877,4.971 1.829,4.914 1.783,4.855 1.738,4.796 1.693,4.736 1.65,4.675 1.608,4.614 1.567,4.551 1.527,4.488 1.488,4.425 1.45,4.36 1.413,4.295 1.378,4.23 1.343,4.164 1.31,4.097 1.278,4.029 1.247,3.961 1.217,3.893 1.189,3.824 1.161,3.755 1.135,3.685 1.11,3.614 1.087,3.544 1.064,3.472 1.043,3.401 1.023,3.329 1.004,3.257 0.987,3.184 0.971,3.111 0.956,3.038 0.942,2.965 0.93,2.891 0.919,2.817 0.909,2.743 0.901,2.669 0.894,2.595 0.888,2.52 0.883,2.446 0.88,2.371 0.878,2.297 0.878,2.222 0.878,2.148 0.88,2.073 0.883,1.998 0.888,1.924 0.894,1.85 0.901,1.775 0.909,1.701 0.919,1.627 0.93,1.553 0.942,1.48 0.956,1.406 0.971,1.333 0.987,1.26 1.004,1.188 1.023,1.116 1.043,1.044 1.064,0.972 1.087,0.901 1.11,0.83 1.135,0.76 1.161,0.69 1.189,0.62 1.217,0.551 1.247,0.483 1.278,0.415 1.31,0.348 1.343,0.281 1.378,0.215 1.413,0.149 1.45,0.084 1.488,0.02 1.527,-0.044 1.567,-0.107 1.608,-0.169 1.65,-0.231 1.693,-0.291 1.738,-0.351 1.783,-0.411 1.829,-0.469 1.877,-0.527 1.925,-0.584 1.975,-0.639 2.025,-0.694 2.076,-0.749 2.129,-0.802 2.182,-0.854 2.236,-0.906 2.291,-0.956 2.347,-1.006 2.403,-1.054 2.461,-1.102 2.519,-1.148 2.578,-1.194 2.638,-1.238 2.699,-1.282 2.761,-1.324 2.823,-1.365 2.886,-1.405 2.949,-1.444 3.013,-1.482 3.078,-1.519 3.144,-1.555 3.21,-1.589 3.277,-1.623 3.344,-1.655 3.412,-1.686 3.48,-1.716 3.549,-1.745 3.619,-1.772 3.688,-1.798 3.759,-1.823 3.83,-1.847 3.901,-1.87 3.972,-1.891 4.044,-1.911 4.116,-1.93 4.189,-1.947 4.262,-1.964 4.335,-1.979 4.408,-1.992 4.482,-2.005 4.555,-2.016 4.629,-2.026 4.704,-2.034 4.778,-2.042 4.852,-2.048 4.927,-2.052 5.001,-2.056 5.076,-2.058 5.15,-2.059 5.225,-2.058 5.3,-2.056 5.374,-2.053 5.449,-2.049 5.523,-2.043 5.597,-2.036 5.672,-2.028 5.746,-2.018 5.819,-2.007 5.893,-1.995 5.966,-1.982 6.04,-1.967 6.112,-1.951 6.185,-1.934 6.257,-1.915 6.329,-1.895 6.401,-1.874 6.472,-1.852 6.543,-1.829 6.613,-1.804 6.683,-1.778 6.753,-1.751 6.822,-1.722 6.89,-1.693 6.958,-1.662 7.026,-1.63 7.093,-1.597 7.159,-1.562 7.225,-1.527 7.29,-1.49 7.354,-1.453 7.418,-1.414 7.481,-1.374 7.543,-1.333 7.605,-1.291 7.666,-1.248 7.726,-1.203 7.785,-1.158 7.844,-1.112 7.901,-1.065 7.958,-1.016 8.014,-0.967 8.069,-0.917 8.124,-0.865 8.177,-0.813 8.229,-0.76 8.281,-0.706 8.331,-0.651 8.381,-0.596 8.43,-0.539 8.477,-0.482 8.524,-0.423 8.57,-0.364 8.614,-0.304 8.658,-0.244 8.7,-0.182 8.742,-0.12 8.782,-0.057 8.821,0.006 8.859,0.07 8.896,0.135 8.932,0.201 8.966,0.267 9,0.333</coordinates></LineString></lineStringMember></MultiLineString>" ) );
  res = elemToString( exportFloat.asGml2( doc, 3 ) );
  QGSCOMPAREGML( res, expectedGML2prec3 );

  //as GML3
  QString expectedGML3prec3( QStringLiteral( "<MultiCurve xmlns=\"gml\"><curveMember xmlns=\"gml\"><Curve xmlns=\"gml\"><segments xmlns=\"gml\"><ArcString xmlns=\"gml\"><posList xmlns=\"gml\" srsDimension=\"2\">2.333 5.667 0.6 4.333 2.333 3.667</posList></ArcString></segments></Curve></curveMember><curveMember xmlns=\"gml\"><Curve xmlns=\"gml\"><segments xmlns=\"gml\"><ArcString xmlns=\"gml\"><posList xmlns=\"gml\" srsDimension=\"2\">9 4.111 1.049 1.024 9 0.333</posList></ArcString></segments></Curve></curveMember></MultiCurve>" ) );
  res = elemToString( exportFloat.asGml3( doc, 3 ) );
  QCOMPARE( res, expectedGML3prec3 );

  // insert geometry
  QgsMultiCurve rc;
  rc.clear();
  rc.insertGeometry( nullptr, 0 );
  QVERIFY( rc.isEmpty() );
  QCOMPARE( rc.numGeometries(), 0 );
  rc.insertGeometry( nullptr, -1 );
  QVERIFY( rc.isEmpty() );
  QCOMPARE( rc.numGeometries(), 0 );
  rc.insertGeometry( nullptr, 100 );
  QVERIFY( rc.isEmpty() );
  QCOMPARE( rc.numGeometries(), 0 );

  rc.insertGeometry( new QgsPoint(), 0 );
  QVERIFY( rc.isEmpty() );
  QCOMPARE( rc.numGeometries(), 0 );

  rc.insertGeometry( part.clone(), 0 );
  QVERIFY( !rc.isEmpty() );
  QCOMPARE( rc.numGeometries(), 1 );

  // cast
  QVERIFY( !QgsMultiCurve().cast( nullptr ) );
  QgsMultiCurve pCast;
  QVERIFY( QgsMultiCurve().cast( &pCast ) );
  QgsMultiCurve pCast2;
  pCast2.fromWkt( QStringLiteral( "MultiCurveZ()" ) );
  QVERIFY( QgsMultiCurve().cast( &pCast2 ) );
  pCast2.fromWkt( QStringLiteral( "MultiCurveM()" ) );
  QVERIFY( QgsMultiCurve().cast( &pCast2 ) );
  pCast2.fromWkt( QStringLiteral( "MultiCurveZM()" ) );
  QVERIFY( QgsMultiCurve().cast( &pCast2 ) );

  //boundary
  QgsMultiCurve multiLine1;
  QVERIFY( !multiLine1.boundary() );
  QgsCircularString boundaryLine1;
  boundaryLine1.setPoints( QVector<QgsPoint>() << QgsPoint( 0, 0 ) << QgsPoint( 1, 0 ) << QgsPoint( 1, 1 ) );
  multiLine1.addGeometry( boundaryLine1.clone() );
  QgsAbstractGeometry *boundary = multiLine1.boundary();
  QgsMultiPoint *mpBoundary = dynamic_cast< QgsMultiPoint * >( boundary );
  QVERIFY( mpBoundary );
  QCOMPARE( mpBoundary->numGeometries(), 2 );
  QCOMPARE( static_cast< QgsPoint *>( mpBoundary->geometryN( 0 ) )->x(), 0.0 );
  QCOMPARE( static_cast< QgsPoint *>( mpBoundary->geometryN( 0 ) )->y(), 0.0 );
  QCOMPARE( static_cast< QgsPoint *>( mpBoundary->geometryN( 1 ) )->x(), 1.0 );
  QCOMPARE( static_cast< QgsPoint *>( mpBoundary->geometryN( 1 ) )->y(), 1.0 );
  delete boundary;
  // add another QgsCircularString
  QgsCircularString boundaryLine2;
  boundaryLine2.setPoints( QVector<QgsPoint>() << QgsPoint( 10, 10 ) << QgsPoint( 11, 10 ) << QgsPoint( 11, 11 ) );
  multiLine1.addGeometry( boundaryLine2.clone() );
  boundary = multiLine1.boundary();
  mpBoundary = dynamic_cast< QgsMultiPoint * >( boundary );
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
  QgsCircularString boundaryLine3;
  boundaryLine3.setPoints( QVector<QgsPoint>() << QgsPoint( 20, 20 ) << QgsPoint( 21, 20 ) <<  QgsPoint( 20, 20 ) );
  multiLine1.addGeometry( boundaryLine3.clone() );
  boundary = multiLine1.boundary();
  mpBoundary = dynamic_cast< QgsMultiPoint * >( boundary );
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
  QgsCircularString boundaryLine4;
  boundaryLine4.setPoints( QVector<QgsPoint>() << QgsPoint( QgsWkbTypes::PointZ, 0, 0, 10 ) << QgsPoint( QgsWkbTypes::PointZ, 1, 0, 15 ) << QgsPoint( QgsWkbTypes::PointZ, 1, 1, 20 ) );
  QgsCircularString boundaryLine5;
  boundaryLine5.setPoints( QVector<QgsPoint>() << QgsPoint( QgsWkbTypes::PointZ, 10, 10, 100 ) << QgsPoint( QgsWkbTypes::PointZ, 10, 20, 150 ) << QgsPoint( QgsWkbTypes::PointZ, 20, 20, 200 ) );
  QgsMultiCurve multiLine2;
  multiLine2.addGeometry( boundaryLine4.clone() );
  multiLine2.addGeometry( boundaryLine5.clone() );

  boundary = multiLine2.boundary();
  mpBoundary = dynamic_cast< QgsMultiPoint * >( boundary );
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

  //reversed
  QgsMultiCurve cR;
  std::unique_ptr< QgsMultiCurve > reversed( cR.reversed() );
  QVERIFY( reversed->isEmpty() );
  part.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 7, 17, 4, 1 ) << QgsPoint( QgsWkbTypes::PointZM, 3, 13, 1, 4 ) << QgsPoint( QgsWkbTypes::PointZM, 7, 11, 2, 8 ) ) ;
  cR.addGeometry( part.clone() );
  part.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 27, 37, 6, 2 ) << QgsPoint( QgsWkbTypes::PointZM, 43, 43, 11, 5 )  << QgsPoint( QgsWkbTypes::PointZM, 27, 53, 21, 52 ) ) ;
  cR.addGeometry( part.clone() );
  reversed.reset( cR.reversed() );
  QVERIFY( !reversed->isEmpty() );
  ls = static_cast< const QgsCircularString * >( reversed->geometryN( 0 ) );
  QCOMPARE( ls->pointN( 0 ), QgsPoint( QgsWkbTypes::PointZM, 7, 11, 2, 8 ) );
  QCOMPARE( ls->pointN( 1 ), QgsPoint( QgsWkbTypes::PointZM, 3, 13, 1, 4 ) );
  QCOMPARE( ls->pointN( 2 ), QgsPoint( QgsWkbTypes::PointZM, 7, 17, 4, 1 ) );
  ls = static_cast< const QgsCircularString * >( reversed->geometryN( 1 ) );
  QCOMPARE( ls->pointN( 0 ), QgsPoint( QgsWkbTypes::PointZM, 27, 53, 21, 52 ) );
  QCOMPARE( ls->pointN( 1 ), QgsPoint( QgsWkbTypes::PointZM, 43, 43, 11, 5 ) );
  QCOMPARE( ls->pointN( 2 ), QgsPoint( QgsWkbTypes::PointZM, 27, 37, 6, 2 ) );
}

void TestQgsGeometry::multiSurface()
{
  //test constructor
  QgsMultiSurface c1;
  QVERIFY( c1.isEmpty() );
  QCOMPARE( c1.nCoordinates(), 0 );
  QCOMPARE( c1.ringCount(), 0 );
  QCOMPARE( c1.partCount(), 0 );
  QVERIFY( !c1.is3D() );
  QVERIFY( !c1.isMeasure() );
  QCOMPARE( c1.wkbType(), QgsWkbTypes::MultiSurface );
  QCOMPARE( c1.wktTypeStr(), QString( "MultiSurface" ) );
  QCOMPARE( c1.geometryType(), QString( "MultiSurface" ) );
  QCOMPARE( c1.dimension(), 0 );
  QVERIFY( !c1.hasCurvedSegments() );
  QCOMPARE( c1.area(), 0.0 );
  QCOMPARE( c1.perimeter(), 0.0 );
  QCOMPARE( c1.numGeometries(), 0 );
  QVERIFY( !c1.geometryN( 0 ) );
  QVERIFY( !c1.geometryN( -1 ) );
  QCOMPARE( c1.vertexCount( 0, 0 ), 0 );
  QCOMPARE( c1.vertexCount( 0, 1 ), 0 );
  QCOMPARE( c1.vertexCount( 1, 0 ), 0 );

  //addGeometry
  //try with nullptr
  c1.addGeometry( nullptr );
  QVERIFY( c1.isEmpty() );
  QCOMPARE( c1.nCoordinates(), 0 );
  QCOMPARE( c1.ringCount(), 0 );
  QCOMPARE( c1.partCount(), 0 );
  QCOMPARE( c1.numGeometries(), 0 );
  QCOMPARE( c1.wkbType(), QgsWkbTypes::MultiSurface );
  QVERIFY( !c1.geometryN( 0 ) );
  QVERIFY( !c1.geometryN( -1 ) );

  // not a surface
  QVERIFY( !c1.addGeometry( new QgsPoint() ) );
  QVERIFY( c1.isEmpty() );
  QCOMPARE( c1.nCoordinates(), 0 );
  QCOMPARE( c1.ringCount(), 0 );
  QCOMPARE( c1.partCount(), 0 );
  QCOMPARE( c1.numGeometries(), 0 );
  QCOMPARE( c1.wkbType(), QgsWkbTypes::MultiSurface );
  QVERIFY( !c1.geometryN( 0 ) );
  QVERIFY( !c1.geometryN( -1 ) );

  //valid geometry
  QgsCurvePolygon part;
  QgsCircularString ring;
  ring.setPoints( QgsPointSequence() << QgsPoint( 1, 10 ) << QgsPoint( 2, 11 ) << QgsPoint( 1, 10 ) );
  part.setExteriorRing( ring.clone() );
  c1.addGeometry( part.clone() );
  QVERIFY( !c1.isEmpty() );
  QCOMPARE( c1.numGeometries(), 1 );
  QCOMPARE( c1.nCoordinates(), 3 );
  QCOMPARE( c1.ringCount(), 1 );
  QCOMPARE( c1.partCount(), 1 );
  QVERIFY( !c1.is3D() );
  QVERIFY( !c1.isMeasure() );
  QCOMPARE( c1.wkbType(), QgsWkbTypes::MultiSurface );
  QCOMPARE( c1.wktTypeStr(), QString( "MultiSurface" ) );
  QCOMPARE( c1.geometryType(), QString( "MultiSurface" ) );
  QCOMPARE( c1.dimension(), 2 );
  QVERIFY( c1.hasCurvedSegments() );
  QVERIFY( c1.geometryN( 0 ) );
  QCOMPARE( *static_cast< const QgsCurvePolygon * >( c1.geometryN( 0 ) ), part );
  QVERIFY( !c1.geometryN( 100 ) );
  QVERIFY( !c1.geometryN( -1 ) );
  QCOMPARE( c1.vertexCount( 0, 0 ), 3 );
  QCOMPARE( c1.vertexCount( 1, 0 ), 0 );

  //initial adding of geometry should set z/m type
  ring.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZ, 10, 11, 1 ) << QgsPoint( QgsWkbTypes::PointZ, 20, 21, 2 )
                  << QgsPoint( QgsWkbTypes::PointZ, 10, 11, 1 ) );
  part.clear();
  part.setExteriorRing( ring.clone() );
  QgsMultiSurface c2;
  c2.addGeometry( part.clone() );
  QVERIFY( c2.is3D() );
  QVERIFY( !c2.isMeasure() );
  QCOMPARE( c2.wkbType(), QgsWkbTypes::MultiSurfaceZ );
  QCOMPARE( c2.wktTypeStr(), QString( "MultiSurfaceZ" ) );
  QCOMPARE( c2.geometryType(), QString( "MultiSurface" ) );
  QCOMPARE( *( static_cast< const QgsCurvePolygon * >( c2.geometryN( 0 ) ) ), part );
  QgsMultiSurface c3;
  ring.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointM, 10, 11, 0, 1 ) << QgsPoint( QgsWkbTypes::PointM, 20, 21, 0, 2 )
                  << QgsPoint( QgsWkbTypes::PointM, 10, 11, 0, 1 ) );
  part.clear();
  part.setExteriorRing( ring.clone() );
  c3.addGeometry( part.clone() );
  QVERIFY( !c3.is3D() );
  QVERIFY( c3.isMeasure() );
  QCOMPARE( c3.wkbType(), QgsWkbTypes::MultiSurfaceM );
  QCOMPARE( c3.wktTypeStr(), QString( "MultiSurfaceM" ) );
  QCOMPARE( *( static_cast< const QgsCurvePolygon * >( c3.geometryN( 0 ) ) ), part );
  QgsMultiSurface c4;
  ring.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 10, 11, 2, 1 ) << QgsPoint( QgsWkbTypes::PointZM, 20, 21, 3, 2 )
                  << QgsPoint( QgsWkbTypes::PointZM, 10, 11, 2, 1 ) );
  part.clear();
  part.setExteriorRing( ring.clone() );
  c4.addGeometry( part.clone() );
  QVERIFY( c4.is3D() );
  QVERIFY( c4.isMeasure() );
  QCOMPARE( c4.wkbType(), QgsWkbTypes::MultiSurfaceZM );
  QCOMPARE( c4.wktTypeStr(), QString( "MultiSurfaceZM" ) );
  QCOMPARE( *( static_cast< const QgsCurvePolygon * >( c4.geometryN( 0 ) ) ), part );

  //add another part
  QgsMultiSurface c6;
  ring.setPoints( QgsPointSequence() << QgsPoint( 1, 10 ) << QgsPoint( 2, 11 ) << QgsPoint( 1, 10 ) );
  part.clear();
  part.setExteriorRing( ring.clone() );
  c6.addGeometry( part.clone() );
  QCOMPARE( c6.vertexCount( 0, 0 ), 3 );
  ring.setPoints( QgsPointSequence() << QgsPoint( 9, 12 ) << QgsPoint( 3, 13 )  << QgsPoint( 9, 12 ) );
  part.clear();
  part.setExteriorRing( ring.clone() );
  c6.addGeometry( part.clone() );
  QCOMPARE( c6.vertexCount( 1, 0 ), 3 );
  QCOMPARE( c6.numGeometries(), 2 );
  QVERIFY( c6.geometryN( 0 ) );
  QCOMPARE( *static_cast< const QgsCurvePolygon * >( c6.geometryN( 1 ) ), part );

  //adding subsequent points should not alter z/m type, regardless of parts type
  c6.clear();
  c6.addGeometry( part.clone() );
  QCOMPARE( c6.wkbType(), QgsWkbTypes::MultiSurface );
  ring.setPoints( QgsPointSequence() << QgsPoint( 1, 10, 2 ) << QgsPoint( 2, 11, 3 ) << QgsPoint( 1, 10, 2 ) ) ;
  part.clear();
  part.setExteriorRing( ring.clone() );
  c6.addGeometry( part.clone() );
  QCOMPARE( c6.wkbType(), QgsWkbTypes::MultiSurface );
  QCOMPARE( c6.vertexCount( 0, 0 ), 3 );
  QCOMPARE( c6.vertexCount( 1, 0 ), 3 );
  QCOMPARE( c6.vertexCount( 2, 0 ), 0 );
  QCOMPARE( c6.vertexCount( -1, 0 ), 0 );
  QCOMPARE( c6.nCoordinates(), 6 );
  QCOMPARE( c6.ringCount(), 1 );
  QCOMPARE( c6.partCount(), 2 );
  QVERIFY( !c6.is3D() );
  const QgsCurvePolygon *ls = static_cast< const QgsCurvePolygon * >( c6.geometryN( 0 ) );
  QCOMPARE( static_cast< const QgsCircularString *>( ls->exteriorRing() )->pointN( 0 ), QgsPoint( 9, 12 ) );
  QCOMPARE( static_cast< const QgsCircularString *>( ls->exteriorRing() )->pointN( 1 ), QgsPoint( 3, 13 ) );
  QCOMPARE( static_cast< const QgsCircularString *>( ls->exteriorRing() )->pointN( 2 ), QgsPoint( 9, 12 ) );
  ls = static_cast< const QgsCurvePolygon * >( c6.geometryN( 1 ) );
  QCOMPARE( static_cast< const QgsCircularString *>( ls->exteriorRing() )->pointN( 0 ), QgsPoint( 1, 10 ) );
  QCOMPARE( static_cast< const QgsCircularString *>( ls->exteriorRing() )->pointN( 1 ), QgsPoint( 2, 11 ) );
  QCOMPARE( static_cast< const QgsCircularString *>( ls->exteriorRing() )->pointN( 2 ), QgsPoint( 1, 10 ) );
  ring.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointM, 21, 30, 0, 2 ) << QgsPoint( QgsWkbTypes::PointM, 32, 41, 0, 3 )
                  << QgsPoint( QgsWkbTypes::PointM, 21, 30, 0, 2 ) ) ;
  part.clear();
  part.setExteriorRing( ring.clone() );
  c6.addGeometry( part.clone() );
  QCOMPARE( c6.wkbType(), QgsWkbTypes::MultiSurface );
  QCOMPARE( c6.vertexCount( 0, 0 ), 3 );
  QCOMPARE( c6.vertexCount( 1, 0 ), 3 );
  QCOMPARE( c6.vertexCount( 2, 0 ), 3 );
  QCOMPARE( c6.nCoordinates(), 9 );
  QCOMPARE( c6.ringCount(), 1 );
  QCOMPARE( c6.partCount(), 3 );
  QVERIFY( !c6.is3D() );
  QVERIFY( !c6.isMeasure() );
  ls = static_cast< const QgsCurvePolygon * >( c6.geometryN( 2 ) );
  QCOMPARE( static_cast< const QgsCircularString *>( ls->exteriorRing() )->pointN( 0 ), QgsPoint( 21, 30 ) );
  QCOMPARE( static_cast< const QgsCircularString *>( ls->exteriorRing() )->pointN( 1 ), QgsPoint( 32, 41 ) );
  QCOMPARE( static_cast< const QgsCircularString *>( ls->exteriorRing() )->pointN( 2 ), QgsPoint( 21, 30 ) );

  c6.clear();
  ring.setPoints( QgsPointSequence() << QgsPoint( 1, 10, 2 ) << QgsPoint( 2, 11, 3 ) << QgsPoint( 1, 10, 2 ) ) ;
  part.clear();
  part.setExteriorRing( ring.clone() );
  c6.addGeometry( part.clone() );
  QCOMPARE( c6.wkbType(), QgsWkbTypes::MultiSurfaceZ );
  ring.setPoints( QgsPointSequence() << QgsPoint( 2, 20 ) << QgsPoint( 3, 31 ) << QgsPoint( 2, 20 ) ) ;
  part.clear();
  part.setExteriorRing( ring.clone() );
  c6.addGeometry( part.clone() );
  QCOMPARE( c6.wkbType(), QgsWkbTypes::MultiSurfaceZ );
  QVERIFY( c6.is3D() );
  ls = static_cast< const QgsCurvePolygon * >( c6.geometryN( 0 ) );
  QCOMPARE( static_cast< const QgsCircularString *>( ls->exteriorRing() )->pointN( 0 ), QgsPoint( 1, 10, 2 ) );
  QCOMPARE( static_cast< const QgsCircularString *>( ls->exteriorRing() )->pointN( 1 ), QgsPoint( 2, 11, 3 ) );
  QCOMPARE( static_cast< const QgsCircularString *>( ls->exteriorRing() )->pointN( 2 ), QgsPoint( 1, 10, 2 ) );
  ls = static_cast< const QgsCurvePolygon * >( c6.geometryN( 1 ) );
  QCOMPARE( static_cast< const QgsCircularString *>( ls->exteriorRing() )->pointN( 0 ), QgsPoint( 2, 20, 0 ) );
  QCOMPARE( static_cast< const QgsCircularString *>( ls->exteriorRing() )->pointN( 1 ), QgsPoint( 3, 31, 0 ) );
  QCOMPARE( static_cast< const QgsCircularString *>( ls->exteriorRing() )->pointN( 2 ), QgsPoint( 2, 20, 0 ) );
  ring.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointM, 5, 50, 0, 4 ) << QgsPoint( QgsWkbTypes::PointM, 6, 61, 0, 5 )
                  << QgsPoint( QgsWkbTypes::PointM, 5, 50, 0, 4 ) ) ;
  part.clear();
  part.setExteriorRing( ring.clone() );
  c6.addGeometry( part.clone() );
  QCOMPARE( c6.wkbType(), QgsWkbTypes::MultiSurfaceZ );
  QVERIFY( c6.is3D() );
  QVERIFY( !c6.isMeasure() );
  ls = static_cast< const QgsCurvePolygon * >( c6.geometryN( 2 ) );
  QCOMPARE( static_cast< const QgsCircularString *>( ls->exteriorRing() )->pointN( 0 ), QgsPoint( 5, 50, 0 ) );
  QCOMPARE( static_cast< const QgsCircularString *>( ls->exteriorRing() )->pointN( 1 ), QgsPoint( 6, 61, 0 ) );
  QCOMPARE( static_cast< const QgsCircularString *>( ls->exteriorRing() )->pointN( 2 ), QgsPoint( 5, 50, 0 ) );

  c6.clear();
  QCOMPARE( c6.wkbType(), QgsWkbTypes::MultiSurface );
  ring.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointM, 5, 50, 0, 4 ) << QgsPoint( QgsWkbTypes::PointM, 6, 61, 0, 5 )
                  << QgsPoint( QgsWkbTypes::PointM, 5, 50, 0, 4 ) ) ;
  part.clear();
  part.setExteriorRing( ring.clone() );
  c6.addGeometry( part.clone() );
  QCOMPARE( c6.wkbType(), QgsWkbTypes::MultiSurfaceM );
  ring.setPoints( QgsPointSequence() << QgsPoint( 2, 20 ) << QgsPoint( 3, 31 )   << QgsPoint( 2, 20 ) ) ;
  part.clear();
  part.setExteriorRing( ring.clone() );
  c6.addGeometry( part.clone() );
  QCOMPARE( c6.wkbType(), QgsWkbTypes::MultiSurfaceM );
  QVERIFY( c6.isMeasure() );
  ls = static_cast< const QgsCurvePolygon * >( c6.geometryN( 0 ) );
  QCOMPARE( static_cast< const QgsCircularString *>( ls->exteriorRing() )->pointN( 0 ), QgsPoint( QgsWkbTypes::PointM, 5, 50, 0, 4 ) );
  QCOMPARE( static_cast< const QgsCircularString *>( ls->exteriorRing() )->pointN( 1 ), QgsPoint( QgsWkbTypes::PointM, 6, 61, 0, 5 ) );
  QCOMPARE( static_cast< const QgsCircularString *>( ls->exteriorRing() )->pointN( 2 ), QgsPoint( QgsWkbTypes::PointM, 5, 50, 0, 4 ) );
  ls = static_cast< const QgsCurvePolygon * >( c6.geometryN( 1 ) );
  QCOMPARE( static_cast< const QgsCircularString *>( ls->exteriorRing() )->pointN( 0 ), QgsPoint( QgsWkbTypes::PointM, 2, 20, 0, 0 ) );
  QCOMPARE( static_cast< const QgsCircularString *>( ls->exteriorRing() )->pointN( 1 ), QgsPoint( QgsWkbTypes::PointM, 3, 31, 0, 0 ) );
  QCOMPARE( static_cast< const QgsCircularString *>( ls->exteriorRing() )->pointN( 2 ), QgsPoint( QgsWkbTypes::PointM, 2, 20, 0, 0 ) );
  ring.setPoints( QgsPointSequence() << QgsPoint( 11, 12, 13 ) << QgsPoint( 14, 15, 16 ) << QgsPoint( 11, 12, 13 ) ) ;
  part.clear();
  part.setExteriorRing( ring.clone() );
  c6.addGeometry( part.clone() );
  QCOMPARE( c6.wkbType(), QgsWkbTypes::MultiSurfaceM );
  QVERIFY( !c6.is3D() );
  QVERIFY( c6.isMeasure() );
  ls = static_cast< const QgsCurvePolygon * >( c6.geometryN( 2 ) );
  QCOMPARE( static_cast< const QgsCircularString *>( ls->exteriorRing() )->pointN( 0 ), QgsPoint( QgsWkbTypes::PointM, 11, 12, 0, 0 ) );
  QCOMPARE( static_cast< const QgsCircularString *>( ls->exteriorRing() )->pointN( 1 ), QgsPoint( QgsWkbTypes::PointM, 14, 15, 0, 0 ) );
  QCOMPARE( static_cast< const QgsCircularString *>( ls->exteriorRing() )->pointN( 2 ), QgsPoint( QgsWkbTypes::PointM, 11, 12, 0, 0 ) );

  c6.clear();
  ring.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 5, 50, 1, 4 ) << QgsPoint( QgsWkbTypes::PointZM, 6, 61, 3, 5 )
                  << QgsPoint( QgsWkbTypes::PointZM, 5, 50, 1, 4 ) ) ;
  part.clear();
  part.setExteriorRing( ring.clone() );
  c6.addGeometry( part.clone() );
  QCOMPARE( c6.wkbType(), QgsWkbTypes::MultiSurfaceZM );
  ring.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::Point, 7, 17 ) << QgsPoint( QgsWkbTypes::Point, 3, 13 )
                  << QgsPoint( QgsWkbTypes::Point, 7, 17 ) ) ;
  part.clear();
  part.setExteriorRing( ring.clone() );
  c6.addGeometry( part.clone() );
  QCOMPARE( c6.wkbType(), QgsWkbTypes::MultiSurfaceZM );
  QVERIFY( c6.isMeasure() );
  QVERIFY( c6.is3D() );
  ls = static_cast< const QgsCurvePolygon * >( c6.geometryN( 0 ) );
  QCOMPARE( static_cast< const QgsCircularString *>( ls->exteriorRing() )->pointN( 0 ), QgsPoint( QgsWkbTypes::PointZM, 5, 50, 1, 4 ) );
  QCOMPARE( static_cast< const QgsCircularString *>( ls->exteriorRing() )->pointN( 1 ), QgsPoint( QgsWkbTypes::PointZM, 6, 61, 3, 5 ) );
  QCOMPARE( static_cast< const QgsCircularString *>( ls->exteriorRing() )->pointN( 2 ), QgsPoint( QgsWkbTypes::PointZM, 5, 50, 1, 4 ) );
  ls = static_cast< const QgsCurvePolygon * >( c6.geometryN( 1 ) );
  QCOMPARE( static_cast< const QgsCircularString *>( ls->exteriorRing() )->pointN( 0 ), QgsPoint( QgsWkbTypes::PointZM, 7, 17, 0, 0 ) );
  QCOMPARE( static_cast< const QgsCircularString *>( ls->exteriorRing() )->pointN( 1 ), QgsPoint( QgsWkbTypes::PointZM, 3, 13, 0, 0 ) );
  QCOMPARE( static_cast< const QgsCircularString *>( ls->exteriorRing() )->pointN( 2 ), QgsPoint( QgsWkbTypes::PointZM, 7, 17, 0, 0 ) );
  ring.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZ, 77, 87, 7 ) << QgsPoint( QgsWkbTypes::PointZ, 83, 83, 8 )
                  << QgsPoint( QgsWkbTypes::PointZ, 77, 87, 7 ) ) ;
  part.clear();
  part.setExteriorRing( ring.clone() );
  c6.addGeometry( part.clone() );
  QCOMPARE( c6.wkbType(), QgsWkbTypes::MultiSurfaceZM );
  QVERIFY( c6.is3D() );
  QVERIFY( c6.isMeasure() );
  ls = static_cast< const QgsCurvePolygon * >( c6.geometryN( 2 ) );
  QCOMPARE( static_cast< const QgsCircularString *>( ls->exteriorRing() )->pointN( 0 ), QgsPoint( QgsWkbTypes::PointZM, 77, 87, 7, 0 ) );
  QCOMPARE( static_cast< const QgsCircularString *>( ls->exteriorRing() )->pointN( 1 ), QgsPoint( QgsWkbTypes::PointZM, 83, 83, 8, 0 ) );
  QCOMPARE( static_cast< const QgsCircularString *>( ls->exteriorRing() )->pointN( 2 ), QgsPoint( QgsWkbTypes::PointZM, 77, 87, 7, 0 ) );
  ring.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointM, 177, 187, 0, 9 ) << QgsPoint( QgsWkbTypes::PointM, 183, 183, 0, 11 )
                  << QgsPoint( QgsWkbTypes::PointM, 177, 187, 0, 9 ) ) ;
  part.clear();
  part.setExteriorRing( ring.clone() );
  c6.addGeometry( part.clone() );
  QCOMPARE( c6.wkbType(), QgsWkbTypes::MultiSurfaceZM );
  QVERIFY( c6.is3D() );
  QVERIFY( c6.isMeasure() );
  ls = static_cast< const QgsCurvePolygon * >( c6.geometryN( 3 ) );
  QCOMPARE( static_cast< const QgsCircularString *>( ls->exteriorRing() )->pointN( 0 ), QgsPoint( QgsWkbTypes::PointZM, 177, 187, 0, 9 ) );
  QCOMPARE( static_cast< const QgsCircularString *>( ls->exteriorRing() )->pointN( 1 ), QgsPoint( QgsWkbTypes::PointZM, 183, 183, 0, 11 ) );
  QCOMPARE( static_cast< const QgsCircularString *>( ls->exteriorRing() )->pointN( 2 ), QgsPoint( QgsWkbTypes::PointZM, 177, 187, 0, 9 ) );

  //clear
  QgsMultiSurface c7;
  ring.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 5, 50, 1, 4 ) << QgsPoint( QgsWkbTypes::PointZM, 6, 61, 3, 5 )
                  << QgsPoint( QgsWkbTypes::PointZM, 5, 71, 4, 6 ) ) ;
  part.clear();
  part.setExteriorRing( ring.clone() );
  c7.addGeometry( part.clone() );
  c7.addGeometry( part.clone() );
  QCOMPARE( c7.numGeometries(), 2 );
  c7.clear();
  QVERIFY( c7.isEmpty() );
  QCOMPARE( c7.numGeometries(), 0 );
  QCOMPARE( c7.nCoordinates(), 0 );
  QCOMPARE( c7.ringCount(), 0 );
  QCOMPARE( c7.partCount(), 0 );
  QVERIFY( !c7.is3D() );
  QVERIFY( !c7.isMeasure() );
  QCOMPARE( c7.wkbType(), QgsWkbTypes::MultiSurface );

  //clone
  QgsMultiSurface c11;
  std::unique_ptr< QgsMultiSurface >cloned( c11.clone() );
  QVERIFY( cloned->isEmpty() );
  c11.addGeometry( part.clone() );
  c11.addGeometry( part.clone() );
  cloned.reset( c11.clone() );
  QCOMPARE( cloned->numGeometries(), 2 );
  ls = static_cast< const QgsCurvePolygon * >( cloned->geometryN( 0 ) );
  QCOMPARE( *ls, part );
  ls = static_cast< const QgsCurvePolygon * >( cloned->geometryN( 1 ) );
  QCOMPARE( *ls, part );

  //copy constructor
  QgsMultiSurface c12;
  QgsMultiSurface c13( c12 );
  QVERIFY( c13.isEmpty() );
  c12.addGeometry( part.clone() );
  c12.addGeometry( part.clone() );
  QgsMultiSurface c14( c12 );
  QCOMPARE( c14.numGeometries(), 2 );
  QCOMPARE( c14.wkbType(), QgsWkbTypes::MultiSurfaceZM );
  ls = static_cast< const QgsCurvePolygon * >( c14.geometryN( 0 ) );
  QCOMPARE( *ls, part );
  ls = static_cast< const QgsCurvePolygon * >( c14.geometryN( 1 ) );
  QCOMPARE( *ls, part );

  //assignment operator
  QgsMultiSurface c15;
  c15 = c13;
  QCOMPARE( c15.numGeometries(), 0 );
  c15 = c14;
  QCOMPARE( c15.numGeometries(), 2 );
  ls = static_cast< const QgsCurvePolygon * >( c15.geometryN( 0 ) );
  QCOMPARE( *ls, part );
  ls = static_cast< const QgsCurvePolygon * >( c15.geometryN( 1 ) );
  QCOMPARE( *ls, part );

  //toCurveType
  std::unique_ptr< QgsMultiSurface > curveType( c12.toCurveType() );
  QCOMPARE( curveType->wkbType(), QgsWkbTypes::MultiSurfaceZM );
  QCOMPARE( curveType->numGeometries(), 2 );
  const QgsCurvePolygon *curve = static_cast< const QgsCurvePolygon * >( curveType->geometryN( 0 ) );
  QCOMPARE( *curve, *static_cast< const QgsCurvePolygon * >( c12.geometryN( 0 ) ) );
  curve = static_cast< const QgsCurvePolygon * >( curveType->geometryN( 1 ) );
  QCOMPARE( *curve, *static_cast< const QgsCurvePolygon * >( c12.geometryN( 1 ) ) );

  //to/fromWKB
  QgsMultiSurface c16;
  ring.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::Point, 7, 17 ) << QgsPoint( QgsWkbTypes::Point, 3, 13 ) << QgsPoint( QgsWkbTypes::Point, 7, 17 ) ) ;
  part.clear();
  part.setExteriorRing( ring.clone() );
  c16.addGeometry( part.clone() );
  ring.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::Point, 27, 37 ) << QgsPoint( QgsWkbTypes::Point, 43, 43 )  << QgsPoint( QgsWkbTypes::Point, 27, 37 ) ) ;
  part.clear();
  part.setExteriorRing( ring.clone() );
  c16.addGeometry( part.clone() );
  QByteArray wkb16 = c16.asWkb();
  QgsMultiSurface c17;
  QgsConstWkbPtr wkb16ptr( wkb16 );
  c17.fromWkb( wkb16ptr );
  QCOMPARE( c17.numGeometries(), 2 );
  QCOMPARE( *static_cast< const QgsCurvePolygon * >( c17.geometryN( 0 ) ), *static_cast< const QgsCurvePolygon * >( c16.geometryN( 0 ) ) );
  QCOMPARE( *static_cast< const QgsCurvePolygon * >( c17.geometryN( 1 ) ), *static_cast< const QgsCurvePolygon * >( c16.geometryN( 1 ) ) );

  //parts with Z
  c16.clear();
  c17.clear();
  ring.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZ, 7, 17, 1 ) << QgsPoint( QgsWkbTypes::PointZ, 3, 13, 4 ) << QgsPoint( QgsWkbTypes::PointZ, 7, 17, 1 ) ) ;
  part.clear();
  part.setExteriorRing( ring.clone() );
  c16.addGeometry( part.clone() );
  ring.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZ, 27, 37, 2 ) << QgsPoint( QgsWkbTypes::PointZ, 43, 43, 5 ) << QgsPoint( QgsWkbTypes::PointZ, 27, 37, 2 ) ) ;
  part.clear();
  part.setExteriorRing( ring.clone() );
  c16.addGeometry( part.clone() );
  wkb16 = c16.asWkb();
  QgsConstWkbPtr wkb16ptr2( wkb16 );
  c17.fromWkb( wkb16ptr2 );
  QCOMPARE( c17.numGeometries(), 2 );
  QCOMPARE( c17.wkbType(), QgsWkbTypes::MultiSurfaceZ );
  QCOMPARE( *static_cast< const QgsCurvePolygon * >( c17.geometryN( 0 ) ), *static_cast< const QgsCurvePolygon * >( c16.geometryN( 0 ) ) );
  QCOMPARE( *static_cast< const QgsCurvePolygon * >( c17.geometryN( 1 ) ), *static_cast< const QgsCurvePolygon * >( c16.geometryN( 1 ) ) );

  //parts with m
  c16.clear();
  c17.clear();
  ring.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointM, 7, 17, 0, 1 ) << QgsPoint( QgsWkbTypes::PointM, 3, 13, 0, 4 ) << QgsPoint( QgsWkbTypes::PointM, 7, 17, 0, 1 ) ) ;
  part.clear();
  part.setExteriorRing( ring.clone() );
  c16.addGeometry( part.clone() );
  ring.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointM, 27, 37, 0, 2 ) << QgsPoint( QgsWkbTypes::PointM, 43, 43, 0, 5 ) << QgsPoint( QgsWkbTypes::PointM, 27, 37, 0, 2 ) ) ;
  part.clear();
  part.setExteriorRing( ring.clone() );
  c16.addGeometry( part.clone() );
  wkb16 = c16.asWkb();
  QgsConstWkbPtr wkb16ptr3( wkb16 );
  c17.fromWkb( wkb16ptr3 );
  QCOMPARE( c17.numGeometries(), 2 );
  QCOMPARE( c17.wkbType(), QgsWkbTypes::MultiSurfaceM );
  QCOMPARE( *static_cast< const QgsCurvePolygon * >( c17.geometryN( 0 ) ), *static_cast< const QgsCurvePolygon * >( c16.geometryN( 0 ) ) );
  QCOMPARE( *static_cast< const QgsCurvePolygon * >( c17.geometryN( 1 ) ), *static_cast< const QgsCurvePolygon * >( c16.geometryN( 1 ) ) );

  // parts with ZM
  c16.clear();
  c17.clear();
  ring.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 7, 17, 4, 1 ) << QgsPoint( QgsWkbTypes::PointZM, 3, 13, 1, 4 )  << QgsPoint( QgsWkbTypes::PointZM, 7, 17, 4, 1 ) ) ;
  part.clear();
  part.setExteriorRing( ring.clone() );
  c16.addGeometry( part.clone() );
  ring.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 27, 37, 6, 2 ) << QgsPoint( QgsWkbTypes::PointZM, 43, 43, 11, 5 ) << QgsPoint( QgsWkbTypes::PointZM, 27, 37, 6, 2 ) ) ;
  part.clear();
  part.setExteriorRing( ring.clone() );
  c16.addGeometry( part.clone() );
  wkb16 = c16.asWkb();
  QgsConstWkbPtr wkb16ptr4( wkb16 );
  c17.fromWkb( wkb16ptr4 );
  QCOMPARE( c17.numGeometries(), 2 );
  QCOMPARE( c17.wkbType(), QgsWkbTypes::MultiSurfaceZM );
  QCOMPARE( *static_cast< const QgsCurvePolygon * >( c17.geometryN( 0 ) ), *static_cast< const QgsCurvePolygon * >( c16.geometryN( 0 ) ) );
  QCOMPARE( *static_cast< const QgsCurvePolygon * >( c17.geometryN( 1 ) ), *static_cast< const QgsCurvePolygon * >( c16.geometryN( 1 ) ) );

  //bad WKB - check for no crash
  c17.clear();
  QgsConstWkbPtr nullPtr( nullptr, 0 );
  QVERIFY( !c17.fromWkb( nullPtr ) );
  QCOMPARE( c17.wkbType(), QgsWkbTypes::MultiSurface );
  QgsPoint point( 1, 2 );
  QByteArray wkbPoint = point.asWkb();
  QgsConstWkbPtr wkbPointPtr( wkbPoint );
  QVERIFY( !c17.fromWkb( wkbPointPtr ) );
  QCOMPARE( c17.wkbType(), QgsWkbTypes::MultiSurface );

  //to/from WKT
  QgsMultiSurface c18;
  ring.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 7, 17, 4, 1 ) << QgsPoint( QgsWkbTypes::PointZM, 3, 13, 1, 4 ) << QgsPoint( QgsWkbTypes::PointZM, 7, 11, 2, 8 ) ) ;
  part.clear();
  part.setExteriorRing( ring.clone() );
  c18.addGeometry( part.clone() );
  ring.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 27, 37, 6, 2 ) << QgsPoint( QgsWkbTypes::PointZM, 43, 43, 11, 5 )  << QgsPoint( QgsWkbTypes::PointZM, 27, 53, 21, 52 ) ) ;
  part.clear();
  part.setExteriorRing( ring.clone() );
  c18.addGeometry( part.clone() );

  QString wkt = c18.asWkt();
  QVERIFY( !wkt.isEmpty() );
  QgsMultiSurface c19;
  QVERIFY( c19.fromWkt( wkt ) );
  QCOMPARE( c19.numGeometries(), 2 );
  QCOMPARE( *static_cast< const QgsCurvePolygon * >( c19.geometryN( 0 ) ), *static_cast< const QgsCurvePolygon * >( c18.geometryN( 0 ) ) );
  QCOMPARE( *static_cast< const QgsCurvePolygon * >( c19.geometryN( 1 ) ), *static_cast< const QgsCurvePolygon * >( c18.geometryN( 1 ) ) );

  //bad WKT
  QgsMultiSurface c20;
  QVERIFY( !c20.fromWkt( "Point()" ) );
  QVERIFY( c20.isEmpty() );
  QCOMPARE( c20.numGeometries(), 0 );
  QCOMPARE( c20.wkbType(), QgsWkbTypes::MultiSurface );

  //as JSON
  QgsMultiSurface exportC;
  ring.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::Point, 7, 17 ) << QgsPoint( QgsWkbTypes::Point, 3, 13 ) << QgsPoint( QgsWkbTypes::Point, 7, 17 ) ) ;
  part.clear();
  part.setExteriorRing( ring.clone() );
  exportC.addGeometry( part.clone() );
  ring.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::Point, 27, 37 ) << QgsPoint( QgsWkbTypes::Point, 43, 43 )  << QgsPoint( QgsWkbTypes::Point, 27, 37 ) ) ;
  part.clear();
  part.setExteriorRing( ring.clone() );
  exportC.addGeometry( part.clone() );

  // GML document for compare
  QDomDocument doc( "gml" );

  // as GML2
  QString expectedSimpleGML2( QStringLiteral( "<MultiPolygon xmlns=\"gml\"><polygonMember xmlns=\"gml\"><Polygon xmlns=\"gml\"><outerBoundaryIs xmlns=\"gml\"><LinearRing xmlns=\"gml\"><coordinates xmlns=\"gml\" cs=\",\" ts=\" \">7,17 7,17</coordinates></LinearRing></outerBoundaryIs></Polygon></polygonMember><polygonMember xmlns=\"gml\"><Polygon xmlns=\"gml\"><outerBoundaryIs xmlns=\"gml\"><LinearRing xmlns=\"gml\"><coordinates xmlns=\"gml\" cs=\",\" ts=\" \">27,37 27,37</coordinates></LinearRing></outerBoundaryIs></Polygon></polygonMember></MultiPolygon>" ) );
  QString res = elemToString( exportC.asGml2( doc, 1 ) );
  QGSCOMPAREGML( res, expectedSimpleGML2 );
  QString expectedGML2empty( QStringLiteral( "<MultiPolygon xmlns=\"gml\"/>" ) );
  QGSCOMPAREGML( elemToString( QgsMultiSurface().asGml2( doc ) ), expectedGML2empty );

  //as GML3
  QString expectedSimpleGML3( QStringLiteral( "<MultiSurface xmlns=\"gml\"><surfaceMember xmlns=\"gml\"><Polygon xmlns=\"gml\"><exterior xmlns=\"gml\"><Curve xmlns=\"gml\"><segments xmlns=\"gml\"><ArcString xmlns=\"gml\"><posList xmlns=\"gml\" srsDimension=\"2\">7 17 3 13 7 17</posList></ArcString></segments></Curve></exterior></Polygon></surfaceMember><surfaceMember xmlns=\"gml\"><Polygon xmlns=\"gml\"><exterior xmlns=\"gml\"><Curve xmlns=\"gml\"><segments xmlns=\"gml\"><ArcString xmlns=\"gml\"><posList xmlns=\"gml\" srsDimension=\"2\">27 37 43 43 27 37</posList></ArcString></segments></Curve></exterior></Polygon></surfaceMember></MultiSurface>" ) );
  res = elemToString( exportC.asGml3( doc ) );
  QCOMPARE( res, expectedSimpleGML3 );
  QString expectedGML3empty( QStringLiteral( "<MultiSurface xmlns=\"gml\"/>" ) );
  QGSCOMPAREGML( elemToString( QgsMultiSurface().asGml3( doc ) ), expectedGML3empty );

  // as JSON
  QString expectedSimpleJson( "{\"type\": \"MultiPolygon\", \"coordinates\": [[[ [7, 17], [7, 17]]], [[ [27, 37], [27, 37]]]] }" );
  res = exportC.asJson( 1 );
  QCOMPARE( res, expectedSimpleJson );

  ring.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::Point, 17, 27 ) << QgsPoint( QgsWkbTypes::Point, 18, 28 )  << QgsPoint( QgsWkbTypes::Point, 17, 27 ) ) ;
  part.addInteriorRing( ring.clone() );
  exportC.addGeometry( part.clone() );

  QString expectedJsonWithRings( "{\"type\": \"MultiPolygon\", \"coordinates\": [[[ [7, 17], [7, 17]]], [[ [27, 37], [27, 37]]], [[ [27, 37], [27, 37]], [ [17, 27], [17, 27]]]] }" );
  res = exportC.asJson( 1 );
  QCOMPARE( res, expectedJsonWithRings );

  QgsMultiSurface exportFloat;
  ring.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::Point, 7 / 3.0, 17 / 3.0 ) << QgsPoint( QgsWkbTypes::Point, 3 / 5.0, 13 / 3.0 ) << QgsPoint( QgsWkbTypes::Point, 7 / 3.0, 17 / 3.0 ) ) ;
  part.clear();
  part.setExteriorRing( ring.clone() );
  exportFloat.addGeometry( part.clone() );
  ring.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::Point, 27 / 3.0, 37 / 9.0 ) << QgsPoint( QgsWkbTypes::Point, 43 / 41.0, 43 / 42.0 ) << QgsPoint( QgsWkbTypes::Point, 27 / 3.0, 37 / 9.0 ) ) ;
  part.clear();
  part.setExteriorRing( ring.clone() );
  exportFloat.addGeometry( part.clone() );

  QString expectedJsonPrec3( QStringLiteral( "{\"type\": \"MultiPolygon\", \"coordinates\": [[[ [2.333, 5.667], [2.333, 5.667]]], [[ [9, 4.111], [9, 4.111]]]] }" ) );
  res = exportFloat.asJson( 3 );
  QCOMPARE( res, expectedJsonPrec3 );

  // as GML2
  QString expectedGML2prec3( QStringLiteral( "<MultiPolygon xmlns=\"gml\"><polygonMember xmlns=\"gml\"><Polygon xmlns=\"gml\"><outerBoundaryIs xmlns=\"gml\"><LinearRing xmlns=\"gml\"><coordinates xmlns=\"gml\" cs=\",\" ts=\" \">2.333,5.667 2.333,5.667</coordinates></LinearRing></outerBoundaryIs></Polygon></polygonMember><polygonMember xmlns=\"gml\"><Polygon xmlns=\"gml\"><outerBoundaryIs xmlns=\"gml\"><LinearRing xmlns=\"gml\"><coordinates xmlns=\"gml\" cs=\",\" ts=\" \">9,4.111 9,4.111</coordinates></LinearRing></outerBoundaryIs></Polygon></polygonMember></MultiPolygon>" ) );
  res = elemToString( exportFloat.asGml2( doc, 3 ) );
  QGSCOMPAREGML( res, expectedGML2prec3 );

  //as GML3
  QString expectedGML3prec3( QStringLiteral( "<MultiSurface xmlns=\"gml\"><surfaceMember xmlns=\"gml\"><Polygon xmlns=\"gml\"><exterior xmlns=\"gml\"><Curve xmlns=\"gml\"><segments xmlns=\"gml\"><ArcString xmlns=\"gml\"><posList xmlns=\"gml\" srsDimension=\"2\">2.333 5.667 0.6 4.333 2.333 5.667</posList></ArcString></segments></Curve></exterior></Polygon></surfaceMember><surfaceMember xmlns=\"gml\"><Polygon xmlns=\"gml\"><exterior xmlns=\"gml\"><Curve xmlns=\"gml\"><segments xmlns=\"gml\"><ArcString xmlns=\"gml\"><posList xmlns=\"gml\" srsDimension=\"2\">9 4.111 1.049 1.024 9 4.111</posList></ArcString></segments></Curve></exterior></Polygon></surfaceMember></MultiSurface>" ) );
  res = elemToString( exportFloat.asGml3( doc, 3 ) );
  QCOMPARE( res, expectedGML3prec3 );

  // insert geometry
  QgsMultiSurface rc;
  rc.clear();
  rc.insertGeometry( nullptr, 0 );
  QVERIFY( rc.isEmpty() );
  QCOMPARE( rc.numGeometries(), 0 );
  rc.insertGeometry( nullptr, -1 );
  QVERIFY( rc.isEmpty() );
  QCOMPARE( rc.numGeometries(), 0 );
  rc.insertGeometry( nullptr, 100 );
  QVERIFY( rc.isEmpty() );
  QCOMPARE( rc.numGeometries(), 0 );

  rc.insertGeometry( new QgsPoint(), 0 );
  QVERIFY( rc.isEmpty() );
  QCOMPARE( rc.numGeometries(), 0 );

  rc.insertGeometry( part.clone(), 0 );
  QVERIFY( !rc.isEmpty() );
  QCOMPARE( rc.numGeometries(), 1 );

  // cast
  QVERIFY( !QgsMultiSurface().cast( nullptr ) );
  QgsMultiSurface pCast;
  QVERIFY( QgsMultiSurface().cast( &pCast ) );
  QgsMultiSurface pCast2;
  pCast2.fromWkt( QStringLiteral( "MultiSurfaceZ()" ) );
  QVERIFY( QgsMultiSurface().cast( &pCast2 ) );
  pCast2.fromWkt( QStringLiteral( "MultiSurfaceM()" ) );
  QVERIFY( QgsMultiSurface().cast( &pCast2 ) );
  pCast2.fromWkt( QStringLiteral( "MultiSurfaceZM()" ) );
  QVERIFY( QgsMultiSurface().cast( &pCast2 ) );

  //boundary
  QgsMultiSurface multiSurface;
  QVERIFY( !multiSurface.boundary() );
  QgsCircularString boundaryLine1;
  boundaryLine1.setPoints( QVector<QgsPoint>() << QgsPoint( 0, 0 ) << QgsPoint( 1, 0 ) << QgsPoint( 0, 0 ) );
  part.clear();
  part.setExteriorRing( boundaryLine1.clone() );
  multiSurface.addGeometry( part.clone() );
  QgsAbstractGeometry *boundary = multiSurface.boundary();
  QgsMultiCurve *mpBoundary = dynamic_cast< QgsMultiCurve * >( boundary );
  QVERIFY( mpBoundary );
  QCOMPARE( mpBoundary->numGeometries(), 1 );
  QCOMPARE( *static_cast< const QgsCurve *>( mpBoundary->geometryN( 0 ) ), *part.exteriorRing() );
  delete boundary;
  // add another QgsCircularString
  QgsCircularString boundaryLine2;
  boundaryLine2.setPoints( QVector<QgsPoint>() << QgsPoint( 10, 10 ) << QgsPoint( 11, 10 ) << QgsPoint( 10, 10 ) );
  QgsCurvePolygon part2;
  part2.setExteriorRing( boundaryLine2.clone() );
  multiSurface.addGeometry( part2.clone() );
  boundary = multiSurface.boundary();
  mpBoundary = dynamic_cast< QgsMultiCurve * >( boundary );
  QVERIFY( mpBoundary );
  QCOMPARE( mpBoundary->numGeometries(), 2 );
  QCOMPARE( *static_cast< const QgsCurve *>( mpBoundary->geometryN( 0 ) ), *part.exteriorRing() );
  QCOMPARE( *static_cast< const QgsCurve *>( mpBoundary->geometryN( 1 ) ), *part2.exteriorRing() );
  delete boundary;

  //boundary with z
  QgsCircularString boundaryLine4;
  boundaryLine4.setPoints( QVector<QgsPoint>() << QgsPoint( QgsWkbTypes::PointZ, 0, 0, 10 ) << QgsPoint( QgsWkbTypes::PointZ, 1, 0, 15 ) << QgsPoint( QgsWkbTypes::PointZ, 0, 0, 10 ) );
  part.clear();
  part.setExteriorRing( boundaryLine4.clone() );
  QgsCircularString boundaryLine5;
  boundaryLine5.setPoints( QVector<QgsPoint>() << QgsPoint( QgsWkbTypes::PointZ, 10, 10, 100 ) << QgsPoint( QgsWkbTypes::PointZ, 10, 20, 150 ) << QgsPoint( QgsWkbTypes::PointZ, 10, 10, 100 ) );
  part2.clear();
  part2.setExteriorRing( boundaryLine5.clone() );
  QgsMultiSurface multiSurface2;
  multiSurface2.addGeometry( part.clone() );
  multiSurface2.addGeometry( part2.clone() );

  boundary = multiSurface2.boundary();
  mpBoundary = dynamic_cast< QgsMultiCurve * >( boundary );
  QVERIFY( mpBoundary );
  QCOMPARE( mpBoundary->numGeometries(), 2 );
  QCOMPARE( *static_cast< const QgsCurve *>( mpBoundary->geometryN( 0 ) ), *part.exteriorRing() );
  QCOMPARE( *static_cast< const QgsCurve *>( mpBoundary->geometryN( 1 ) ), *part2.exteriorRing() );
  delete boundary;
}

void TestQgsGeometry::multiPolygon()
{
  //test constructor
  QgsMultiPolygon c1;
  QVERIFY( c1.isEmpty() );
  QCOMPARE( c1.nCoordinates(), 0 );
  QCOMPARE( c1.ringCount(), 0 );
  QCOMPARE( c1.partCount(), 0 );
  QVERIFY( !c1.is3D() );
  QVERIFY( !c1.isMeasure() );
  QCOMPARE( c1.wkbType(), QgsWkbTypes::MultiPolygon );
  QCOMPARE( c1.wktTypeStr(), QString( "MultiPolygon" ) );
  QCOMPARE( c1.geometryType(), QString( "MultiPolygon" ) );
  QCOMPARE( c1.dimension(), 0 );
  QVERIFY( !c1.hasCurvedSegments() );
  QCOMPARE( c1.area(), 0.0 );
  QCOMPARE( c1.perimeter(), 0.0 );
  QCOMPARE( c1.numGeometries(), 0 );
  QVERIFY( !c1.geometryN( 0 ) );
  QVERIFY( !c1.geometryN( -1 ) );
  QCOMPARE( c1.vertexCount( 0, 0 ), 0 );
  QCOMPARE( c1.vertexCount( 0, 1 ), 0 );
  QCOMPARE( c1.vertexCount( 1, 0 ), 0 );

  //addGeometry
  //try with nullptr
  c1.addGeometry( nullptr );
  QVERIFY( c1.isEmpty() );
  QCOMPARE( c1.nCoordinates(), 0 );
  QCOMPARE( c1.ringCount(), 0 );
  QCOMPARE( c1.partCount(), 0 );
  QCOMPARE( c1.numGeometries(), 0 );
  QCOMPARE( c1.wkbType(), QgsWkbTypes::MultiPolygon );
  QVERIFY( !c1.geometryN( 0 ) );
  QVERIFY( !c1.geometryN( -1 ) );

  // not a surface
  QVERIFY( !c1.addGeometry( new QgsPoint() ) );
  QVERIFY( c1.isEmpty() );
  QCOMPARE( c1.nCoordinates(), 0 );
  QCOMPARE( c1.ringCount(), 0 );
  QCOMPARE( c1.partCount(), 0 );
  QCOMPARE( c1.numGeometries(), 0 );
  QCOMPARE( c1.wkbType(), QgsWkbTypes::MultiPolygon );
  QVERIFY( !c1.geometryN( 0 ) );
  QVERIFY( !c1.geometryN( -1 ) );

  //valid geometry
  QgsPolygon part;
  QgsLineString ring;
  ring.setPoints( QgsPointSequence() << QgsPoint( 1, 10 ) << QgsPoint( 2, 11 ) << QgsPoint( 2, 21 ) << QgsPoint( 1, 10 ) );
  part.setExteriorRing( ring.clone() );
  c1.addGeometry( part.clone() );
  QVERIFY( !c1.isEmpty() );
  QCOMPARE( c1.numGeometries(), 1 );
  QCOMPARE( c1.nCoordinates(), 4 );
  QCOMPARE( c1.ringCount(), 1 );
  QCOMPARE( c1.partCount(), 1 );
  QVERIFY( !c1.is3D() );
  QVERIFY( !c1.isMeasure() );
  QCOMPARE( c1.wkbType(), QgsWkbTypes::MultiPolygon );
  QCOMPARE( c1.wktTypeStr(), QString( "MultiPolygon" ) );
  QCOMPARE( c1.geometryType(), QString( "MultiPolygon" ) );
  QCOMPARE( c1.dimension(), 2 );
  QVERIFY( !c1.hasCurvedSegments() );
  QVERIFY( c1.geometryN( 0 ) );
  QCOMPARE( *static_cast< const QgsPolygon * >( c1.geometryN( 0 ) ), part );
  QVERIFY( !c1.geometryN( 100 ) );
  QVERIFY( !c1.geometryN( -1 ) );
  QCOMPARE( c1.vertexCount( 0, 0 ), 4 );
  QCOMPARE( c1.vertexCount( 1, 0 ), 0 );

  //initial adding of geometry should set z/m type
  ring.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZ, 10, 11, 1 ) << QgsPoint( QgsWkbTypes::PointZ, 20, 21, 2 )  << QgsPoint( QgsWkbTypes::PointZ, 30, 31, 2 )
                  << QgsPoint( QgsWkbTypes::PointZ, 10, 11, 1 ) );
  part.clear();
  part.setExteriorRing( ring.clone() );
  QgsMultiPolygon c2;
  c2.addGeometry( part.clone() );
  QVERIFY( c2.is3D() );
  QVERIFY( !c2.isMeasure() );
  QCOMPARE( c2.wkbType(), QgsWkbTypes::MultiPolygonZ );
  QCOMPARE( c2.wktTypeStr(), QString( "MultiPolygonZ" ) );
  QCOMPARE( c2.geometryType(), QString( "MultiPolygon" ) );
  QCOMPARE( *( static_cast< const QgsPolygon * >( c2.geometryN( 0 ) ) ), part );
  QgsMultiPolygon c3;
  ring.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointM, 10, 11, 0, 1 ) << QgsPoint( QgsWkbTypes::PointM, 20, 21, 0, 2 ) << QgsPoint( QgsWkbTypes::PointM, 30, 31, 0, 2 )
                  << QgsPoint( QgsWkbTypes::PointM, 10, 11, 0, 1 ) );
  part.clear();
  part.setExteriorRing( ring.clone() );
  c3.addGeometry( part.clone() );
  QVERIFY( !c3.is3D() );
  QVERIFY( c3.isMeasure() );
  QCOMPARE( c3.wkbType(), QgsWkbTypes::MultiPolygonM );
  QCOMPARE( c3.wktTypeStr(), QString( "MultiPolygonM" ) );
  QCOMPARE( *( static_cast< const QgsPolygon * >( c3.geometryN( 0 ) ) ), part );
  QgsMultiPolygon c4;
  ring.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 10, 11, 2, 1 ) << QgsPoint( QgsWkbTypes::PointZM, 20, 21, 3, 2 ) << QgsPoint( QgsWkbTypes::PointZM, 30, 31, 3, 2 )
                  << QgsPoint( QgsWkbTypes::PointZM, 10, 11, 2, 1 ) );
  part.clear();
  part.setExteriorRing( ring.clone() );
  c4.addGeometry( part.clone() );
  QVERIFY( c4.is3D() );
  QVERIFY( c4.isMeasure() );
  QCOMPARE( c4.wkbType(), QgsWkbTypes::MultiPolygonZM );
  QCOMPARE( c4.wktTypeStr(), QString( "MultiPolygonZM" ) );
  QCOMPARE( *( static_cast< const QgsPolygon * >( c4.geometryN( 0 ) ) ), part );

  //add another part
  QgsMultiPolygon c6;
  ring.setPoints( QgsPointSequence() << QgsPoint( 1, 10 ) << QgsPoint( 2, 11 ) << QgsPoint( 10, 21 ) << QgsPoint( 1, 10 ) );
  part.clear();
  part.setExteriorRing( ring.clone() );
  c6.addGeometry( part.clone() );
  QCOMPARE( c6.vertexCount( 0, 0 ), 4 );
  ring.setPoints( QgsPointSequence() << QgsPoint( 9, 12 ) << QgsPoint( 3, 13 )  << QgsPoint( 4, 17 ) << QgsPoint( 9, 12 ) );
  part.clear();
  part.setExteriorRing( ring.clone() );
  c6.addGeometry( part.clone() );
  QCOMPARE( c6.vertexCount( 1, 0 ), 4 );
  QCOMPARE( c6.numGeometries(), 2 );
  QVERIFY( c6.geometryN( 0 ) );
  QCOMPARE( *static_cast< const QgsPolygon * >( c6.geometryN( 1 ) ), part );

  //adding subsequent points should not alter z/m type, regardless of parts type
  c6.clear();
  c6.addGeometry( part.clone() );
  QCOMPARE( c6.wkbType(), QgsWkbTypes::MultiPolygon );
  ring.setPoints( QgsPointSequence() << QgsPoint( 1, 10, 2 ) << QgsPoint( 2, 11, 3 ) << QgsPoint( 10, 13, 3 ) << QgsPoint( 1, 10, 2 ) ) ;
  part.clear();
  part.setExteriorRing( ring.clone() );
  c6.addGeometry( part.clone() );
  QCOMPARE( c6.wkbType(), QgsWkbTypes::MultiPolygon );
  QCOMPARE( c6.vertexCount( 0, 0 ), 4 );
  QCOMPARE( c6.vertexCount( 1, 0 ), 4 );
  QCOMPARE( c6.vertexCount( 2, 0 ), 0 );
  QCOMPARE( c6.vertexCount( -1, 0 ), 0 );
  QCOMPARE( c6.nCoordinates(), 8 );
  QCOMPARE( c6.ringCount(), 1 );
  QCOMPARE( c6.partCount(), 2 );
  QVERIFY( !c6.is3D() );
  const QgsPolygon *ls = static_cast< const QgsPolygon * >( c6.geometryN( 0 ) );
  QCOMPARE( static_cast< const QgsLineString *>( ls->exteriorRing() )->pointN( 0 ), QgsPoint( 9, 12 ) );
  QCOMPARE( static_cast< const QgsLineString *>( ls->exteriorRing() )->pointN( 1 ), QgsPoint( 3, 13 ) );
  QCOMPARE( static_cast< const QgsLineString *>( ls->exteriorRing() )->pointN( 2 ), QgsPoint( 4, 17 ) );
  QCOMPARE( static_cast< const QgsLineString *>( ls->exteriorRing() )->pointN( 3 ), QgsPoint( 9, 12 ) );
  ls = static_cast< const QgsPolygon * >( c6.geometryN( 1 ) );
  QCOMPARE( static_cast< const QgsLineString *>( ls->exteriorRing() )->pointN( 0 ), QgsPoint( 1, 10 ) );
  QCOMPARE( static_cast< const QgsLineString *>( ls->exteriorRing() )->pointN( 1 ), QgsPoint( 2, 11 ) );
  QCOMPARE( static_cast< const QgsLineString *>( ls->exteriorRing() )->pointN( 2 ), QgsPoint( 10, 13 ) );
  QCOMPARE( static_cast< const QgsLineString *>( ls->exteriorRing() )->pointN( 3 ), QgsPoint( 1, 10 ) );
  ring.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointM, 21, 30, 0, 2 ) << QgsPoint( QgsWkbTypes::PointM, 32, 41, 0, 3 )
                  << QgsPoint( QgsWkbTypes::PointM, 42, 61, 0, 4 )
                  << QgsPoint( QgsWkbTypes::PointM, 21, 30, 0, 2 ) ) ;
  part.clear();
  part.setExteriorRing( ring.clone() );
  c6.addGeometry( part.clone() );
  QCOMPARE( c6.wkbType(), QgsWkbTypes::MultiPolygon );
  QCOMPARE( c6.vertexCount( 0, 0 ), 4 );
  QCOMPARE( c6.vertexCount( 1, 0 ), 4 );
  QCOMPARE( c6.vertexCount( 2, 0 ), 4 );
  QCOMPARE( c6.nCoordinates(), 12 );
  QCOMPARE( c6.ringCount(), 1 );
  QCOMPARE( c6.partCount(), 3 );
  QVERIFY( !c6.is3D() );
  QVERIFY( !c6.isMeasure() );
  ls = static_cast< const QgsPolygon * >( c6.geometryN( 2 ) );
  QCOMPARE( static_cast< const QgsLineString *>( ls->exteriorRing() )->pointN( 0 ), QgsPoint( 21, 30 ) );
  QCOMPARE( static_cast< const QgsLineString *>( ls->exteriorRing() )->pointN( 1 ), QgsPoint( 32, 41 ) );
  QCOMPARE( static_cast< const QgsLineString *>( ls->exteriorRing() )->pointN( 2 ), QgsPoint( 42, 61 ) );
  QCOMPARE( static_cast< const QgsLineString *>( ls->exteriorRing() )->pointN( 3 ), QgsPoint( 21, 30 ) );

  c6.clear();
  ring.setPoints( QgsPointSequence() << QgsPoint( 1, 10, 2 ) << QgsPoint( 2, 11, 3 ) << QgsPoint( 9, 15, 3 ) << QgsPoint( 1, 10, 2 ) ) ;
  part.clear();
  part.setExteriorRing( ring.clone() );
  c6.addGeometry( part.clone() );
  QCOMPARE( c6.wkbType(), QgsWkbTypes::MultiPolygonZ );
  ring.setPoints( QgsPointSequence() << QgsPoint( 2, 20 ) << QgsPoint( 3, 31 )  << QgsPoint( 7, 34 ) << QgsPoint( 2, 20 ) ) ;
  part.clear();
  part.setExteriorRing( ring.clone() );
  c6.addGeometry( part.clone() );
  QCOMPARE( c6.wkbType(), QgsWkbTypes::MultiPolygonZ );
  QVERIFY( c6.is3D() );
  ls = static_cast< const QgsPolygon * >( c6.geometryN( 0 ) );
  QCOMPARE( static_cast< const QgsLineString *>( ls->exteriorRing() )->pointN( 0 ), QgsPoint( 1, 10, 2 ) );
  QCOMPARE( static_cast< const QgsLineString *>( ls->exteriorRing() )->pointN( 1 ), QgsPoint( 2, 11, 3 ) );
  QCOMPARE( static_cast< const QgsLineString *>( ls->exteriorRing() )->pointN( 2 ), QgsPoint( 9, 15, 3 ) );
  QCOMPARE( static_cast< const QgsLineString *>( ls->exteriorRing() )->pointN( 3 ), QgsPoint( 1, 10, 2 ) );
  ls = static_cast< const QgsPolygon * >( c6.geometryN( 1 ) );
  QCOMPARE( static_cast< const QgsLineString *>( ls->exteriorRing() )->pointN( 0 ), QgsPoint( 2, 20, 0 ) );
  QCOMPARE( static_cast< const QgsLineString *>( ls->exteriorRing() )->pointN( 1 ), QgsPoint( 3, 31, 0 ) );
  QCOMPARE( static_cast< const QgsLineString *>( ls->exteriorRing() )->pointN( 2 ), QgsPoint( 7, 34, 0 ) );
  QCOMPARE( static_cast< const QgsLineString *>( ls->exteriorRing() )->pointN( 3 ), QgsPoint( 2, 20, 0 ) );
  ring.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointM, 5, 50, 0, 4 ) << QgsPoint( QgsWkbTypes::PointM, 6, 61, 0, 5 )
                  << QgsPoint( QgsWkbTypes::PointM, 9, 65, 0, 7 ) << QgsPoint( QgsWkbTypes::PointM, 5, 50, 0, 4 ) ) ;
  part.clear();
  part.setExteriorRing( ring.clone() );
  c6.addGeometry( part.clone() );
  QCOMPARE( c6.wkbType(), QgsWkbTypes::MultiPolygonZ );
  QVERIFY( c6.is3D() );
  QVERIFY( !c6.isMeasure() );
  ls = static_cast< const QgsPolygon * >( c6.geometryN( 2 ) );
  QCOMPARE( static_cast< const QgsLineString *>( ls->exteriorRing() )->pointN( 0 ), QgsPoint( 5, 50, 0 ) );
  QCOMPARE( static_cast< const QgsLineString *>( ls->exteriorRing() )->pointN( 1 ), QgsPoint( 6, 61, 0 ) );
  QCOMPARE( static_cast< const QgsLineString *>( ls->exteriorRing() )->pointN( 2 ), QgsPoint( 9, 65, 0 ) );
  QCOMPARE( static_cast< const QgsLineString *>( ls->exteriorRing() )->pointN( 3 ), QgsPoint( 5, 50, 0 ) );

  c6.clear();
  QCOMPARE( c6.wkbType(), QgsWkbTypes::MultiPolygon );
  ring.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointM, 5, 50, 0, 4 ) << QgsPoint( QgsWkbTypes::PointM, 6, 61, 0, 5 )
                  << QgsPoint( QgsWkbTypes::PointM, 9, 76, 0, 8 ) << QgsPoint( QgsWkbTypes::PointM, 5, 50, 0, 4 ) ) ;
  part.clear();
  part.setExteriorRing( ring.clone() );
  c6.addGeometry( part.clone() );
  QCOMPARE( c6.wkbType(), QgsWkbTypes::MultiPolygonM );
  ring.setPoints( QgsPointSequence() << QgsPoint( 2, 20 ) << QgsPoint( 3, 31 ) << QgsPoint( 7, 39 ) << QgsPoint( 2, 20 ) ) ;
  part.clear();
  part.setExteriorRing( ring.clone() );
  c6.addGeometry( part.clone() );
  QCOMPARE( c6.wkbType(), QgsWkbTypes::MultiPolygonM );
  QVERIFY( c6.isMeasure() );
  ls = static_cast< const QgsPolygon * >( c6.geometryN( 0 ) );
  QCOMPARE( static_cast< const QgsLineString *>( ls->exteriorRing() )->pointN( 0 ), QgsPoint( QgsWkbTypes::PointM, 5, 50, 0, 4 ) );
  QCOMPARE( static_cast< const QgsLineString *>( ls->exteriorRing() )->pointN( 1 ), QgsPoint( QgsWkbTypes::PointM, 6, 61, 0, 5 ) );
  QCOMPARE( static_cast< const QgsLineString *>( ls->exteriorRing() )->pointN( 2 ), QgsPoint( QgsWkbTypes::PointM, 9, 76, 0, 8 ) );
  QCOMPARE( static_cast< const QgsLineString *>( ls->exteriorRing() )->pointN( 3 ), QgsPoint( QgsWkbTypes::PointM, 5, 50, 0, 4 ) );
  ls = static_cast< const QgsPolygon * >( c6.geometryN( 1 ) );
  QCOMPARE( static_cast< const QgsLineString *>( ls->exteriorRing() )->pointN( 0 ), QgsPoint( QgsWkbTypes::PointM, 2, 20, 0, 0 ) );
  QCOMPARE( static_cast< const QgsLineString *>( ls->exteriorRing() )->pointN( 1 ), QgsPoint( QgsWkbTypes::PointM, 3, 31, 0, 0 ) );
  QCOMPARE( static_cast< const QgsLineString *>( ls->exteriorRing() )->pointN( 2 ), QgsPoint( QgsWkbTypes::PointM, 7, 39, 0, 0 ) );
  QCOMPARE( static_cast< const QgsLineString *>( ls->exteriorRing() )->pointN( 3 ), QgsPoint( QgsWkbTypes::PointM, 2, 20, 0, 0 ) );
  ring.setPoints( QgsPointSequence() << QgsPoint( 11, 12, 13 ) << QgsPoint( 14, 15, 16 ) << QgsPoint( 24, 21, 5 ) << QgsPoint( 11, 12, 13 ) ) ;
  part.clear();
  part.setExteriorRing( ring.clone() );
  c6.addGeometry( part.clone() );
  QCOMPARE( c6.wkbType(), QgsWkbTypes::MultiPolygonM );
  QVERIFY( !c6.is3D() );
  QVERIFY( c6.isMeasure() );
  ls = static_cast< const QgsPolygon * >( c6.geometryN( 2 ) );
  QCOMPARE( static_cast< const QgsLineString *>( ls->exteriorRing() )->pointN( 0 ), QgsPoint( QgsWkbTypes::PointM, 11, 12, 0, 0 ) );
  QCOMPARE( static_cast< const QgsLineString *>( ls->exteriorRing() )->pointN( 1 ), QgsPoint( QgsWkbTypes::PointM, 14, 15, 0, 0 ) );
  QCOMPARE( static_cast< const QgsLineString *>( ls->exteriorRing() )->pointN( 2 ), QgsPoint( QgsWkbTypes::PointM, 24, 21, 0, 0 ) );
  QCOMPARE( static_cast< const QgsLineString *>( ls->exteriorRing() )->pointN( 3 ), QgsPoint( QgsWkbTypes::PointM, 11, 12, 0, 0 ) );

  c6.clear();
  ring.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 5, 50, 1, 4 ) << QgsPoint( QgsWkbTypes::PointZM, 6, 61, 3, 5 )
                  << QgsPoint( QgsWkbTypes::PointZM, 9, 71, 4, 9 ) << QgsPoint( QgsWkbTypes::PointZM, 5, 50, 1, 4 ) ) ;
  part.clear();
  part.setExteriorRing( ring.clone() );
  c6.addGeometry( part.clone() );
  QCOMPARE( c6.wkbType(), QgsWkbTypes::MultiPolygonZM );
  ring.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::Point, 7, 17 ) << QgsPoint( QgsWkbTypes::Point, 3, 13 )
                  << QgsPoint( QgsWkbTypes::Point, 13, 27 ) << QgsPoint( QgsWkbTypes::Point, 7, 17 ) ) ;
  part.clear();
  part.setExteriorRing( ring.clone() );
  c6.addGeometry( part.clone() );
  QCOMPARE( c6.wkbType(), QgsWkbTypes::MultiPolygonZM );
  QVERIFY( c6.isMeasure() );
  QVERIFY( c6.is3D() );
  ls = static_cast< const QgsPolygon * >( c6.geometryN( 0 ) );
  QCOMPARE( static_cast< const QgsLineString *>( ls->exteriorRing() )->pointN( 0 ), QgsPoint( QgsWkbTypes::PointZM, 5, 50, 1, 4 ) );
  QCOMPARE( static_cast< const QgsLineString *>( ls->exteriorRing() )->pointN( 1 ), QgsPoint( QgsWkbTypes::PointZM, 6, 61, 3, 5 ) );
  QCOMPARE( static_cast< const QgsLineString *>( ls->exteriorRing() )->pointN( 2 ), QgsPoint( QgsWkbTypes::PointZM, 9, 71, 4, 9 ) );
  QCOMPARE( static_cast< const QgsLineString *>( ls->exteriorRing() )->pointN( 3 ), QgsPoint( QgsWkbTypes::PointZM, 5, 50, 1, 4 ) );
  ls = static_cast< const QgsPolygon * >( c6.geometryN( 1 ) );
  QCOMPARE( static_cast< const QgsLineString *>( ls->exteriorRing() )->pointN( 0 ), QgsPoint( QgsWkbTypes::PointZM, 7, 17, 0, 0 ) );
  QCOMPARE( static_cast< const QgsLineString *>( ls->exteriorRing() )->pointN( 1 ), QgsPoint( QgsWkbTypes::PointZM, 3, 13, 0, 0 ) );
  QCOMPARE( static_cast< const QgsLineString *>( ls->exteriorRing() )->pointN( 2 ), QgsPoint( QgsWkbTypes::PointZM, 13, 27, 0, 0 ) );
  QCOMPARE( static_cast< const QgsLineString *>( ls->exteriorRing() )->pointN( 3 ), QgsPoint( QgsWkbTypes::PointZM, 7, 17, 0, 0 ) );
  ring.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZ, 77, 87, 7 ) << QgsPoint( QgsWkbTypes::PointZ, 83, 83, 8 )
                  << QgsPoint( QgsWkbTypes::PointZ, 93, 85, 10 ) << QgsPoint( QgsWkbTypes::PointZ, 77, 87, 7 ) ) ;
  part.clear();
  part.setExteriorRing( ring.clone() );
  c6.addGeometry( part.clone() );
  QCOMPARE( c6.wkbType(), QgsWkbTypes::MultiPolygonZM );
  QVERIFY( c6.is3D() );
  QVERIFY( c6.isMeasure() );
  ls = static_cast< const QgsPolygon * >( c6.geometryN( 2 ) );
  QCOMPARE( static_cast< const QgsLineString *>( ls->exteriorRing() )->pointN( 0 ), QgsPoint( QgsWkbTypes::PointZM, 77, 87, 7, 0 ) );
  QCOMPARE( static_cast< const QgsLineString *>( ls->exteriorRing() )->pointN( 1 ), QgsPoint( QgsWkbTypes::PointZM, 83, 83, 8, 0 ) );
  QCOMPARE( static_cast< const QgsLineString *>( ls->exteriorRing() )->pointN( 2 ), QgsPoint( QgsWkbTypes::PointZM, 93, 85, 10, 0 ) );
  QCOMPARE( static_cast< const QgsLineString *>( ls->exteriorRing() )->pointN( 3 ), QgsPoint( QgsWkbTypes::PointZM, 77, 87, 7, 0 ) );
  ring.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointM, 177, 187, 0, 9 ) << QgsPoint( QgsWkbTypes::PointM, 183, 183, 0, 11 )
                  << QgsPoint( QgsWkbTypes::PointM, 185, 193, 0, 13 ) << QgsPoint( QgsWkbTypes::PointM, 177, 187, 0, 9 ) ) ;
  part.clear();
  part.setExteriorRing( ring.clone() );
  c6.addGeometry( part.clone() );
  QCOMPARE( c6.wkbType(), QgsWkbTypes::MultiPolygonZM );
  QVERIFY( c6.is3D() );
  QVERIFY( c6.isMeasure() );
  ls = static_cast< const QgsPolygon * >( c6.geometryN( 3 ) );
  QCOMPARE( static_cast< const QgsLineString *>( ls->exteriorRing() )->pointN( 0 ), QgsPoint( QgsWkbTypes::PointZM, 177, 187, 0, 9 ) );
  QCOMPARE( static_cast< const QgsLineString *>( ls->exteriorRing() )->pointN( 1 ), QgsPoint( QgsWkbTypes::PointZM, 183, 183, 0, 11 ) );
  QCOMPARE( static_cast< const QgsLineString *>( ls->exteriorRing() )->pointN( 2 ), QgsPoint( QgsWkbTypes::PointZM, 185, 193, 0, 13 ) );
  QCOMPARE( static_cast< const QgsLineString *>( ls->exteriorRing() )->pointN( 3 ), QgsPoint( QgsWkbTypes::PointZM, 177, 187, 0, 9 ) );

  //clear
  QgsMultiPolygon c7;
  ring.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 5, 50, 1, 4 ) << QgsPoint( QgsWkbTypes::PointZM, 6, 61, 3, 5 )
                  << QgsPoint( QgsWkbTypes::PointZM, 9, 71, 4, 15 ) << QgsPoint( QgsWkbTypes::PointZM, 5, 71, 4, 6 ) ) ;
  part.clear();
  part.setExteriorRing( ring.clone() );
  c7.addGeometry( part.clone() );
  c7.addGeometry( part.clone() );
  QCOMPARE( c7.numGeometries(), 2 );
  c7.clear();
  QVERIFY( c7.isEmpty() );
  QCOMPARE( c7.numGeometries(), 0 );
  QCOMPARE( c7.nCoordinates(), 0 );
  QCOMPARE( c7.ringCount(), 0 );
  QCOMPARE( c7.partCount(), 0 );
  QVERIFY( !c7.is3D() );
  QVERIFY( !c7.isMeasure() );
  QCOMPARE( c7.wkbType(), QgsWkbTypes::MultiPolygon );

  //clone
  QgsMultiPolygon c11;
  std::unique_ptr< QgsMultiPolygon >cloned( c11.clone() );
  QVERIFY( cloned->isEmpty() );
  c11.addGeometry( part.clone() );
  c11.addGeometry( part.clone() );
  cloned.reset( c11.clone() );
  QCOMPARE( cloned->numGeometries(), 2 );
  ls = static_cast< const QgsPolygon * >( cloned->geometryN( 0 ) );
  QCOMPARE( *ls, part );
  ls = static_cast< const QgsPolygon * >( cloned->geometryN( 1 ) );
  QCOMPARE( *ls, part );

  //copy constructor
  QgsMultiPolygon c12;
  QgsMultiPolygon c13( c12 );
  QVERIFY( c13.isEmpty() );
  c12.addGeometry( part.clone() );
  c12.addGeometry( part.clone() );
  QgsMultiPolygon c14( c12 );
  QCOMPARE( c14.numGeometries(), 2 );
  QCOMPARE( c14.wkbType(), QgsWkbTypes::MultiPolygonZM );
  ls = static_cast< const QgsPolygon * >( c14.geometryN( 0 ) );
  QCOMPARE( *ls, part );
  ls = static_cast< const QgsPolygon * >( c14.geometryN( 1 ) );
  QCOMPARE( *ls, part );

  //assignment operator
  QgsMultiPolygon c15;
  c15 = c13;
  QCOMPARE( c15.numGeometries(), 0 );
  c15 = c14;
  QCOMPARE( c15.numGeometries(), 2 );
  ls = static_cast< const QgsPolygon * >( c15.geometryN( 0 ) );
  QCOMPARE( *ls, part );
  ls = static_cast< const QgsPolygon * >( c15.geometryN( 1 ) );
  QCOMPARE( *ls, part );

  //toCurveType
  std::unique_ptr< QgsMultiSurface > curveType( c12.toCurveType() );
  QCOMPARE( curveType->wkbType(), QgsWkbTypes::MultiSurfaceZM );
  QCOMPARE( curveType->numGeometries(), 2 );
  const QgsPolygon *curve = static_cast< const QgsPolygon * >( curveType->geometryN( 0 ) );
  QCOMPARE( *curve, *static_cast< const QgsPolygon * >( c12.geometryN( 0 ) ) );
  curve = static_cast< const QgsPolygon * >( curveType->geometryN( 1 ) );
  QCOMPARE( *curve, *static_cast< const QgsPolygon * >( c12.geometryN( 1 ) ) );

  //to/fromWKB
  QgsMultiPolygon c16;
  ring.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::Point, 7, 17 ) << QgsPoint( QgsWkbTypes::Point, 3, 13 ) << QgsPoint( QgsWkbTypes::Point, 9, 27 ) << QgsPoint( QgsWkbTypes::Point, 7, 17 ) ) ;
  part.clear();
  part.setExteriorRing( ring.clone() );
  c16.addGeometry( part.clone() );
  ring.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::Point, 27, 37 ) << QgsPoint( QgsWkbTypes::Point, 43, 43 ) << QgsPoint( QgsWkbTypes::Point, 29, 39 ) << QgsPoint( QgsWkbTypes::Point, 27, 37 ) ) ;
  part.clear();
  part.setExteriorRing( ring.clone() );
  c16.addGeometry( part.clone() );
  QByteArray wkb16 = c16.asWkb();
  QgsMultiPolygon c17;
  QgsConstWkbPtr wkb16ptr( wkb16 );
  c17.fromWkb( wkb16ptr );
  QCOMPARE( c17.numGeometries(), 2 );
  QCOMPARE( *static_cast< const QgsPolygon * >( c17.geometryN( 0 ) ), *static_cast< const QgsPolygon * >( c16.geometryN( 0 ) ) );
  QCOMPARE( *static_cast< const QgsPolygon * >( c17.geometryN( 1 ) ), *static_cast< const QgsPolygon * >( c16.geometryN( 1 ) ) );

  //parts with Z
  c16.clear();
  c17.clear();
  ring.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZ, 7, 17, 1 ) << QgsPoint( QgsWkbTypes::PointZ, 3, 13, 4 ) << QgsPoint( QgsWkbTypes::PointZ, 9, 27, 5 )  << QgsPoint( QgsWkbTypes::PointZ, 7, 17, 1 ) ) ;
  part.clear();
  part.setExteriorRing( ring.clone() );
  c16.addGeometry( part.clone() );
  ring.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZ, 27, 37, 2 ) << QgsPoint( QgsWkbTypes::PointZ, 43, 43, 5 ) << QgsPoint( QgsWkbTypes::PointZ, 87, 54, 7 ) << QgsPoint( QgsWkbTypes::PointZ, 27, 37, 2 ) ) ;
  part.clear();
  part.setExteriorRing( ring.clone() );
  c16.addGeometry( part.clone() );
  wkb16 = c16.asWkb();
  QgsConstWkbPtr wkb16ptr2( wkb16 );
  c17.fromWkb( wkb16ptr2 );
  QCOMPARE( c17.numGeometries(), 2 );
  QCOMPARE( c17.wkbType(), QgsWkbTypes::MultiPolygonZ );
  QCOMPARE( *static_cast< const QgsPolygon * >( c17.geometryN( 0 ) ), *static_cast< const QgsPolygon * >( c16.geometryN( 0 ) ) );
  QCOMPARE( *static_cast< const QgsPolygon * >( c17.geometryN( 1 ) ), *static_cast< const QgsPolygon * >( c16.geometryN( 1 ) ) );

  //parts with m
  c16.clear();
  c17.clear();
  ring.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointM, 7, 17, 0, 1 ) << QgsPoint( QgsWkbTypes::PointM, 3, 13, 0, 4 )
                  << QgsPoint( QgsWkbTypes::PointM, 9, 21, 0, 3 ) << QgsPoint( QgsWkbTypes::PointM, 7, 17, 0, 1 ) ) ;
  part.clear();
  part.setExteriorRing( ring.clone() );
  c16.addGeometry( part.clone() );
  ring.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointM, 27, 37, 0, 2 ) << QgsPoint( QgsWkbTypes::PointM, 43, 43, 0, 5 )
                  << QgsPoint( QgsWkbTypes::PointM, 37, 31, 0, 3 ) << QgsPoint( QgsWkbTypes::PointM, 27, 37, 0, 2 ) ) ;
  part.clear();
  part.setExteriorRing( ring.clone() );
  c16.addGeometry( part.clone() );
  wkb16 = c16.asWkb();
  QgsConstWkbPtr wkb16ptr3( wkb16 );
  c17.fromWkb( wkb16ptr3 );
  QCOMPARE( c17.numGeometries(), 2 );
  QCOMPARE( c17.wkbType(), QgsWkbTypes::MultiPolygonM );
  QCOMPARE( *static_cast< const QgsPolygon * >( c17.geometryN( 0 ) ), *static_cast< const QgsPolygon * >( c16.geometryN( 0 ) ) );
  QCOMPARE( *static_cast< const QgsPolygon * >( c17.geometryN( 1 ) ), *static_cast< const QgsPolygon * >( c16.geometryN( 1 ) ) );

  // parts with ZM
  c16.clear();
  c17.clear();
  ring.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 7, 17, 4, 1 ) << QgsPoint( QgsWkbTypes::PointZM, 3, 13, 1, 4 )
                  << QgsPoint( QgsWkbTypes::PointZM, 19, 13, 5, 2 ) << QgsPoint( QgsWkbTypes::PointZM, 7, 17, 4, 1 ) ) ;
  part.clear();
  part.setExteriorRing( ring.clone() );
  c16.addGeometry( part.clone() );
  ring.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 27, 37, 6, 2 ) << QgsPoint( QgsWkbTypes::PointZM, 43, 43, 11, 5 )
                  << QgsPoint( QgsWkbTypes::PointZM, 7, 17, 4, 1 ) << QgsPoint( QgsWkbTypes::PointZM, 27, 37, 6, 2 ) ) ;
  part.clear();
  part.setExteriorRing( ring.clone() );
  c16.addGeometry( part.clone() );
  wkb16 = c16.asWkb();
  QgsConstWkbPtr wkb16ptr4( wkb16 );
  c17.fromWkb( wkb16ptr4 );
  QCOMPARE( c17.numGeometries(), 2 );
  QCOMPARE( c17.wkbType(), QgsWkbTypes::MultiPolygonZM );
  QCOMPARE( *static_cast< const QgsPolygon * >( c17.geometryN( 0 ) ), *static_cast< const QgsPolygon * >( c16.geometryN( 0 ) ) );
  QCOMPARE( *static_cast< const QgsPolygon * >( c17.geometryN( 1 ) ), *static_cast< const QgsPolygon * >( c16.geometryN( 1 ) ) );

  //bad WKB - check for no crash
  c17.clear();
  QgsConstWkbPtr nullPtr( nullptr, 0 );
  QVERIFY( !c17.fromWkb( nullPtr ) );
  QCOMPARE( c17.wkbType(), QgsWkbTypes::MultiPolygon );
  QgsPoint point( 1, 2 );
  QByteArray wkbPoint = point.asWkb();
  QgsConstWkbPtr wkbPointPtr( wkbPoint );
  QVERIFY( !c17.fromWkb( wkbPointPtr ) );
  QCOMPARE( c17.wkbType(), QgsWkbTypes::MultiPolygon );

  //to/from WKT
  QgsMultiPolygon c18;
  ring.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 7, 17, 4, 1 ) << QgsPoint( QgsWkbTypes::PointZM, 3, 13, 1, 4 )
                  << QgsPoint( QgsWkbTypes::PointZM, 13, 19, 3, 10 ) << QgsPoint( QgsWkbTypes::PointZM, 7, 11, 2, 8 ) ) ;
  part.clear();
  part.setExteriorRing( ring.clone() );
  c18.addGeometry( part.clone() );
  ring.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 27, 37, 6, 2 ) << QgsPoint( QgsWkbTypes::PointZM, 43, 43, 11, 5 )
                  << QgsPoint( QgsWkbTypes::PointZM, 17, 49, 31, 53 ) << QgsPoint( QgsWkbTypes::PointZM, 27, 53, 21, 52 ) ) ;
  part.clear();
  part.setExteriorRing( ring.clone() );
  c18.addGeometry( part.clone() );

  QString wkt = c18.asWkt();
  QVERIFY( !wkt.isEmpty() );
  QgsMultiPolygon c19;
  QVERIFY( c19.fromWkt( wkt ) );
  QCOMPARE( c19.numGeometries(), 2 );
  QCOMPARE( *static_cast< const QgsPolygon * >( c19.geometryN( 0 ) ), *static_cast< const QgsPolygon * >( c18.geometryN( 0 ) ) );
  QCOMPARE( *static_cast< const QgsPolygon * >( c19.geometryN( 1 ) ), *static_cast< const QgsPolygon * >( c18.geometryN( 1 ) ) );

  //bad WKT
  QgsMultiPolygon c20;
  QVERIFY( !c20.fromWkt( "Point()" ) );
  QVERIFY( c20.isEmpty() );
  QCOMPARE( c20.numGeometries(), 0 );
  QCOMPARE( c20.wkbType(), QgsWkbTypes::MultiPolygon );

  //as JSON
  QgsMultiPolygon exportC;
  ring.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::Point, 7, 17 ) << QgsPoint( QgsWkbTypes::Point, 3, 13 )
                  << QgsPoint( QgsWkbTypes::Point, 7, 21 ) << QgsPoint( QgsWkbTypes::Point, 7, 17 ) ) ;
  part.clear();
  part.setExteriorRing( ring.clone() );
  exportC.addGeometry( part.clone() );
  ring.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::Point, 27, 37 ) << QgsPoint( QgsWkbTypes::Point, 43, 43 )
                  << QgsPoint( QgsWkbTypes::Point, 41, 39 ) << QgsPoint( QgsWkbTypes::Point, 27, 37 ) ) ;
  part.clear();
  part.setExteriorRing( ring.clone() );
  exportC.addGeometry( part.clone() );

  // GML document for compare
  QDomDocument doc( "gml" );

  // as GML2
  QString expectedSimpleGML2( QStringLiteral( "<MultiPolygon xmlns=\"gml\"><polygonMember xmlns=\"gml\"><Polygon xmlns=\"gml\"><outerBoundaryIs xmlns=\"gml\"><LinearRing xmlns=\"gml\"><coordinates xmlns=\"gml\" cs=\",\" ts=\" \">7,17 3,13 7,21 7,17</coordinates></LinearRing></outerBoundaryIs></Polygon></polygonMember><polygonMember xmlns=\"gml\"><Polygon xmlns=\"gml\"><outerBoundaryIs xmlns=\"gml\"><LinearRing xmlns=\"gml\"><coordinates xmlns=\"gml\" cs=\",\" ts=\" \">27,37 43,43 41,39 27,37</coordinates></LinearRing></outerBoundaryIs></Polygon></polygonMember></MultiPolygon>" ) );
  QString res = elemToString( exportC.asGml2( doc, 1 ) );
  QGSCOMPAREGML( res, expectedSimpleGML2 );
  QString expectedGML2empty( QStringLiteral( "<MultiPolygon xmlns=\"gml\"/>" ) );
  QGSCOMPAREGML( elemToString( QgsMultiPolygon().asGml2( doc ) ), expectedGML2empty );

  //as GML3
  QString expectedSimpleGML3( QStringLiteral( "<MultiPolygon xmlns=\"gml\"><polygonMember xmlns=\"gml\"><Polygon xmlns=\"gml\"><exterior xmlns=\"gml\"><LinearRing xmlns=\"gml\"><posList xmlns=\"gml\" srsDimension=\"2\">7 17 3 13 7 21 7 17</posList></LinearRing></exterior></Polygon></polygonMember><polygonMember xmlns=\"gml\"><Polygon xmlns=\"gml\"><exterior xmlns=\"gml\"><LinearRing xmlns=\"gml\"><posList xmlns=\"gml\" srsDimension=\"2\">27 37 43 43 41 39 27 37</posList></LinearRing></exterior></Polygon></polygonMember></MultiPolygon>" ) );
  res = elemToString( exportC.asGml3( doc ) );
  QCOMPARE( res, expectedSimpleGML3 );
  QString expectedGML3empty( QStringLiteral( "<MultiPolygon xmlns=\"gml\"/>" ) );
  QGSCOMPAREGML( elemToString( QgsMultiPolygon().asGml3( doc ) ), expectedGML3empty );

  // as JSON
  QString expectedSimpleJson( "{\"type\": \"MultiPolygon\", \"coordinates\": [[[ [7, 17], [3, 13], [7, 21], [7, 17]]], [[ [27, 37], [43, 43], [41, 39], [27, 37]]]] }" );
  res = exportC.asJson( 1 );
  QCOMPARE( res, expectedSimpleJson );

  ring.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::Point, 17, 27 ) << QgsPoint( QgsWkbTypes::Point, 18, 28 ) << QgsPoint( QgsWkbTypes::Point, 19, 37 ) << QgsPoint( QgsWkbTypes::Point, 17, 27 ) ) ;
  part.addInteriorRing( ring.clone() );
  exportC.addGeometry( part.clone() );

  QString expectedJsonWithRings( "{\"type\": \"MultiPolygon\", \"coordinates\": [[[ [7, 17], [3, 13], [7, 21], [7, 17]]], [[ [27, 37], [43, 43], [41, 39], [27, 37]]], [[ [27, 37], [43, 43], [41, 39], [27, 37]], [ [17, 27], [18, 28], [19, 37], [17, 27]]]] }" );
  res = exportC.asJson( 1 );
  QCOMPARE( res, expectedJsonWithRings );

  QgsMultiPolygon exportFloat;
  ring.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::Point, 7 / 3.0, 17 / 3.0 ) << QgsPoint( QgsWkbTypes::Point, 3 / 5.0, 13 / 3.0 )
                  << QgsPoint( QgsWkbTypes::Point, 8 / 3.0, 27 / 3.0 ) << QgsPoint( QgsWkbTypes::Point, 7 / 3.0, 17 / 3.0 ) ) ;
  part.clear();
  part.setExteriorRing( ring.clone() );
  exportFloat.addGeometry( part.clone() );
  ring.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::Point, 27 / 3.0, 37 / 9.0 ) << QgsPoint( QgsWkbTypes::Point, 43 / 41.0, 43 / 42.0 ) << QgsPoint( QgsWkbTypes::Point, 27 / 3.0, 37 / 9.0 ) ) ;
  part.clear();
  part.setExteriorRing( ring.clone() );
  exportFloat.addGeometry( part.clone() );

  QString expectedJsonPrec3( QStringLiteral( "{\"type\": \"MultiPolygon\", \"coordinates\": [[[ [2.333, 5.667], [0.6, 4.333], [2.667, 9], [2.333, 5.667]]], [[ [9, 4.111], [1.049, 1.024], [9, 4.111]]]] }" ) );
  res = exportFloat.asJson( 3 );
  QCOMPARE( res, expectedJsonPrec3 );

  // as GML2
  QString expectedGML2prec3( QStringLiteral( "<MultiPolygon xmlns=\"gml\"><polygonMember xmlns=\"gml\"><Polygon xmlns=\"gml\"><outerBoundaryIs xmlns=\"gml\"><LinearRing xmlns=\"gml\"><coordinates xmlns=\"gml\" cs=\",\" ts=\" \">2.333,5.667 0.6,4.333 2.667,9 2.333,5.667</coordinates></LinearRing></outerBoundaryIs></Polygon></polygonMember><polygonMember xmlns=\"gml\"><Polygon xmlns=\"gml\"><outerBoundaryIs xmlns=\"gml\"><LinearRing xmlns=\"gml\"><coordinates xmlns=\"gml\" cs=\",\" ts=\" \">9,4.111 1.049,1.024 9,4.111</coordinates></LinearRing></outerBoundaryIs></Polygon></polygonMember></MultiPolygon>" ) );
  res = elemToString( exportFloat.asGml2( doc, 3 ) );
  QGSCOMPAREGML( res, expectedGML2prec3 );

  //as GML3
  QString expectedGML3prec3( QStringLiteral( "<MultiPolygon xmlns=\"gml\"><polygonMember xmlns=\"gml\"><Polygon xmlns=\"gml\"><exterior xmlns=\"gml\"><LinearRing xmlns=\"gml\"><posList xmlns=\"gml\" srsDimension=\"2\">2.333 5.667 0.6 4.333 2.667 9 2.333 5.667</posList></LinearRing></exterior></Polygon></polygonMember><polygonMember xmlns=\"gml\"><Polygon xmlns=\"gml\"><exterior xmlns=\"gml\"><LinearRing xmlns=\"gml\"><posList xmlns=\"gml\" srsDimension=\"2\">9 4.111 1.049 1.024 9 4.111</posList></LinearRing></exterior></Polygon></polygonMember></MultiPolygon>" ) );
  res = elemToString( exportFloat.asGml3( doc, 3 ) );
  QCOMPARE( res, expectedGML3prec3 );

  // insert geometry
  QgsMultiPolygon rc;
  rc.clear();
  rc.insertGeometry( nullptr, 0 );
  QVERIFY( rc.isEmpty() );
  QCOMPARE( rc.numGeometries(), 0 );
  rc.insertGeometry( nullptr, -1 );
  QVERIFY( rc.isEmpty() );
  QCOMPARE( rc.numGeometries(), 0 );
  rc.insertGeometry( nullptr, 100 );
  QVERIFY( rc.isEmpty() );
  QCOMPARE( rc.numGeometries(), 0 );

  rc.insertGeometry( new QgsPoint(), 0 );
  QVERIFY( rc.isEmpty() );
  QCOMPARE( rc.numGeometries(), 0 );

  rc.insertGeometry( part.clone(), 0 );
  QVERIFY( !rc.isEmpty() );
  QCOMPARE( rc.numGeometries(), 1 );

  // cast
  QVERIFY( !QgsMultiPolygon().cast( nullptr ) );
  QgsMultiPolygon pCast;
  QVERIFY( QgsMultiPolygon().cast( &pCast ) );
  QgsMultiPolygon pCast2;
  pCast2.fromWkt( QStringLiteral( "MultiPolygonZ()" ) );
  QVERIFY( QgsMultiPolygon().cast( &pCast2 ) );
  pCast2.fromWkt( QStringLiteral( "MultiPolygonM()" ) );
  QVERIFY( QgsMultiPolygon().cast( &pCast2 ) );
  pCast2.fromWkt( QStringLiteral( "MultiPolygonZM()" ) );
  QVERIFY( QgsMultiPolygon().cast( &pCast2 ) );


  //boundary
  QgsMultiPolygon multiPolygon1;
  QVERIFY( !multiPolygon1.boundary() );

  QgsLineString ring1;
  ring1.setPoints( QVector<QgsPoint>() << QgsPoint( 0, 0 ) << QgsPoint( 1, 0 ) << QgsPoint( 1, 1 )  << QgsPoint( 0, 0 ) );
  QgsPolygon polygon1;
  polygon1.setExteriorRing( ring1.clone() );
  multiPolygon1.addGeometry( polygon1.clone() );

  std::unique_ptr< QgsAbstractGeometry > boundary( multiPolygon1.boundary() );
  QgsMultiLineString *lineBoundary = dynamic_cast< QgsMultiLineString * >( boundary.get() );
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

  // add polygon with interior rings
  QgsLineString ring2;
  ring2.setPoints( QVector<QgsPoint>() << QgsPoint( 10, 10 ) << QgsPoint( 11, 10 ) << QgsPoint( 11, 11 )  << QgsPoint( 10, 10 ) );
  QgsPolygon polygon2;
  polygon2.setExteriorRing( ring2.clone() );
  QgsLineString boundaryRing1;
  boundaryRing1.setPoints( QVector<QgsPoint>() << QgsPoint( 10.1, 10.1 ) << QgsPoint( 10.2, 10.1 ) << QgsPoint( 10.2, 10.2 )  << QgsPoint( 10.1, 10.1 ) );
  QgsLineString boundaryRing2;
  boundaryRing2.setPoints( QVector<QgsPoint>() << QgsPoint( 10.8, 10.8 ) << QgsPoint( 10.9, 10.8 ) << QgsPoint( 10.9, 10.9 )  << QgsPoint( 10.8, 10.8 ) );
  polygon2.setInteriorRings( QVector< QgsCurve * >() << boundaryRing1.clone() << boundaryRing2.clone() );
  multiPolygon1.addGeometry( polygon2.clone() );

  boundary.reset( multiPolygon1.boundary() );
  QgsMultiLineString *multiLineBoundary( static_cast< QgsMultiLineString * >( boundary.get() ) );
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

  // vertex iterator: 2 polygons (one with just exterior ring, other with two interior rings)
  QgsAbstractGeometry::vertex_iterator it = multiPolygon1.vertices_begin();
  QgsAbstractGeometry::vertex_iterator itEnd = multiPolygon1.vertices_end();
  QCOMPARE( *it, QgsPoint( 0, 0 ) );
  QCOMPARE( it.vertexId(), QgsVertexId( 0, 0, 0 ) );
  ++it;
  QCOMPARE( *it, QgsPoint( 1, 0 ) );
  QCOMPARE( it.vertexId(), QgsVertexId( 0, 0, 1 ) );
  ++it;
  QCOMPARE( *it, QgsPoint( 1, 1 ) );
  QCOMPARE( it.vertexId(), QgsVertexId( 0, 0, 2 ) );
  ++it;
  QCOMPARE( *it, QgsPoint( 0, 0 ) );
  QCOMPARE( it.vertexId(), QgsVertexId( 0, 0, 3 ) );
  ++it;
  // 2nd polygon - exterior ring
  QCOMPARE( *it, QgsPoint( 10, 10 ) );
  QCOMPARE( it.vertexId(), QgsVertexId( 1, 0, 0 ) );
  ++it;
  QCOMPARE( *it, QgsPoint( 11, 10 ) );
  QCOMPARE( it.vertexId(), QgsVertexId( 1, 0, 1 ) );
  ++it;
  QCOMPARE( *it, QgsPoint( 11, 11 ) );
  QCOMPARE( it.vertexId(), QgsVertexId( 1, 0, 2 ) );
  ++it;
  QCOMPARE( *it, QgsPoint( 10, 10 ) );
  QCOMPARE( it.vertexId(), QgsVertexId( 1, 0, 3 ) );
  ++it;
  // 2nd polygon - 1st interior ring
  QCOMPARE( *it, QgsPoint( 10.1, 10.1 ) );
  QCOMPARE( it.vertexId(), QgsVertexId( 1, 1, 0 ) );
  ++it;
  QCOMPARE( *it, QgsPoint( 10.2, 10.1 ) );
  QCOMPARE( it.vertexId(), QgsVertexId( 1, 1, 1 ) );
  ++it;
  QCOMPARE( *it, QgsPoint( 10.2, 10.2 ) );
  QCOMPARE( it.vertexId(), QgsVertexId( 1, 1, 2 ) );
  ++it;
  QCOMPARE( *it, QgsPoint( 10.1, 10.1 ) );
  QCOMPARE( it.vertexId(), QgsVertexId( 1, 1, 3 ) );
  ++it;
  // 2nd polygon - 2nd interior ring
  QCOMPARE( *it, QgsPoint( 10.8, 10.8 ) );
  QCOMPARE( it.vertexId(), QgsVertexId( 1, 2, 0 ) );
  ++it;
  QCOMPARE( *it, QgsPoint( 10.9, 10.8 ) );
  QCOMPARE( it.vertexId(), QgsVertexId( 1, 2, 1 ) );
  ++it;
  QCOMPARE( *it, QgsPoint( 10.9, 10.9 ) );
  QCOMPARE( it.vertexId(), QgsVertexId( 1, 2, 2 ) );
  ++it;
  QCOMPARE( *it, QgsPoint( 10.8, 10.8 ) );
  QCOMPARE( it.vertexId(), QgsVertexId( 1, 2, 3 ) );
  ++it;
  // done!
  QCOMPARE( it, itEnd );

  // Java-style iterator
  QgsVertexIterator it2( &multiPolygon1 );
  QVERIFY( it2.hasNext() );
  QCOMPARE( it2.next(), QgsPoint( 0, 0 ) );
  QVERIFY( it2.hasNext() );
  QCOMPARE( it2.next(), QgsPoint( 1, 0 ) );
  QVERIFY( it2.hasNext() );
  QCOMPARE( it2.next(), QgsPoint( 1, 1 ) );
  QVERIFY( it2.hasNext() );
  QCOMPARE( it2.next(), QgsPoint( 0, 0 ) );
  QVERIFY( it2.hasNext() );
  // 2nd polygon - exterior ring
  QCOMPARE( it2.next(), QgsPoint( 10, 10 ) );
  QVERIFY( it2.hasNext() );
  QCOMPARE( it2.next(), QgsPoint( 11, 10 ) );
  QVERIFY( it2.hasNext() );
  QCOMPARE( it2.next(), QgsPoint( 11, 11 ) );
  QVERIFY( it2.hasNext() );
  QCOMPARE( it2.next(), QgsPoint( 10, 10 ) );
  QVERIFY( it2.hasNext() );
  // 2nd polygon - 1st interior ring
  QCOMPARE( it2.next(), QgsPoint( 10.1, 10.1 ) );
  QVERIFY( it2.hasNext() );
  QCOMPARE( it2.next(), QgsPoint( 10.2, 10.1 ) );
  QVERIFY( it2.hasNext() );
  QCOMPARE( it2.next(), QgsPoint( 10.2, 10.2 ) );
  QVERIFY( it2.hasNext() );
  QCOMPARE( it2.next(), QgsPoint( 10.1, 10.1 ) );
  QVERIFY( it2.hasNext() );
  // 2nd polygon - 2nd interior ring
  QCOMPARE( it2.next(), QgsPoint( 10.8, 10.8 ) );
  QVERIFY( it2.hasNext() );
  QCOMPARE( it2.next(), QgsPoint( 10.9, 10.8 ) );
  QVERIFY( it2.hasNext() );
  QCOMPARE( it2.next(), QgsPoint( 10.9, 10.9 ) );
  QVERIFY( it2.hasNext() );
  QCOMPARE( it2.next(), QgsPoint( 10.8, 10.8 ) );
  QVERIFY( !it2.hasNext() );
}

void TestQgsGeometry::geometryCollection()
{
  //test constructor
  QgsGeometryCollection c1;
  QVERIFY( c1.isEmpty() );
  QCOMPARE( c1.nCoordinates(), 0 );
  QCOMPARE( c1.ringCount(), 0 );
  QCOMPARE( c1.partCount(), 0 );
  QVERIFY( !c1.is3D() );
  QVERIFY( !c1.isMeasure() );
  QCOMPARE( c1.wkbType(), QgsWkbTypes::GeometryCollection );
  QCOMPARE( c1.wktTypeStr(), QString( "GeometryCollection" ) );
  QCOMPARE( c1.geometryType(), QString( "GeometryCollection" ) );
  QCOMPARE( c1.dimension(), 0 );
  QVERIFY( !c1.hasCurvedSegments() );
  QCOMPARE( c1.area(), 0.0 );
  QCOMPARE( c1.perimeter(), 0.0 );
  QCOMPARE( c1.numGeometries(), 0 );
  QVERIFY( !c1.geometryN( 0 ) );
  QVERIFY( !c1.geometryN( -1 ) );
  QCOMPARE( c1.vertexCount( 0, 0 ), 0 );
  QCOMPARE( c1.vertexCount( 0, 1 ), 0 );
  QCOMPARE( c1.vertexCount( 1, 0 ), 0 );

  //addGeometry

  //try with nullptr
  c1.addGeometry( nullptr );
  QVERIFY( c1.isEmpty() );
  QCOMPARE( c1.nCoordinates(), 0 );
  QCOMPARE( c1.ringCount(), 0 );
  QCOMPARE( c1.partCount(), 0 );
  QCOMPARE( c1.numGeometries(), 0 );
  QCOMPARE( c1.wkbType(), QgsWkbTypes::GeometryCollection );
  QVERIFY( !c1.geometryN( 0 ) );
  QVERIFY( !c1.geometryN( -1 ) );

  //valid geometry
  QgsLineString part;
  part.setPoints( QgsPointSequence() << QgsPoint( 0, 0 ) << QgsPoint( 0, 10 ) << QgsPoint( 10, 10 )
                  << QgsPoint( 10, 0 ) << QgsPoint( 0, 0 ) );
  c1.addGeometry( part.clone() );
  QVERIFY( !c1.isEmpty() );
  QCOMPARE( c1.numGeometries(), 1 );
  QCOMPARE( c1.nCoordinates(), 5 );
  QCOMPARE( c1.ringCount(), 1 );
  QCOMPARE( c1.partCount(), 1 );
  QVERIFY( !c1.is3D() );
  QVERIFY( !c1.isMeasure() );
  QCOMPARE( c1.wkbType(), QgsWkbTypes::GeometryCollection );
  QCOMPARE( c1.wktTypeStr(), QString( "GeometryCollection" ) );
  QCOMPARE( c1.geometryType(), QString( "GeometryCollection" ) );
  QCOMPARE( c1.dimension(), 1 );
  QVERIFY( !c1.hasCurvedSegments() );
  QCOMPARE( c1.area(), 0.0 );
  QCOMPARE( c1.perimeter(), 0.0 );
  QVERIFY( c1.geometryN( 0 ) );
  QVERIFY( !c1.geometryN( 100 ) );
  QVERIFY( !c1.geometryN( -1 ) );
  QCOMPARE( c1.vertexCount( 0, 0 ), 5 );
  QCOMPARE( c1.vertexCount( 1, 0 ), 0 );

  //retrieve geometry and check
  QCOMPARE( *( static_cast< const QgsLineString * >( c1.geometryN( 0 ) ) ), part );

  //initial adding of geometry should set z/m type
  part.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZ, 0, 0, 1 )
                  << QgsPoint( QgsWkbTypes::PointZ, 0, 10, 2 ) << QgsPoint( QgsWkbTypes::PointZ, 10, 10, 3 )
                  << QgsPoint( QgsWkbTypes::PointZ, 10, 0, 4 ) << QgsPoint( QgsWkbTypes::PointZ, 0, 0, 1 ) );
  QgsGeometryCollection c2;
  c2.addGeometry( part.clone() );
  //QVERIFY( c2.is3D() ); //no meaning for collections?
  //QVERIFY( !c2.isMeasure() ); //no meaning for collections?
  QCOMPARE( c2.wkbType(), QgsWkbTypes::GeometryCollection );
  QCOMPARE( c2.wktTypeStr(), QString( "GeometryCollection" ) );
  QCOMPARE( c2.geometryType(), QString( "GeometryCollection" ) );
  QCOMPARE( *( static_cast< const QgsLineString * >( c2.geometryN( 0 ) ) ), part );
  QgsGeometryCollection c3;
  part.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointM, 0, 0, 0, 1 )
                  << QgsPoint( QgsWkbTypes::PointM, 0, 10, 0, 2 ) << QgsPoint( QgsWkbTypes::PointM, 10, 10, 0, 3 )
                  << QgsPoint( QgsWkbTypes::PointM, 10, 0, 0, 4 ) << QgsPoint( QgsWkbTypes::PointM, 0, 0, 0, 1 ) );
  c3.addGeometry( part.clone() );
  //QVERIFY( !c3.is3D() ); //no meaning for collections?
  //QVERIFY( c3.isMeasure() ); //no meaning for collections?
  QCOMPARE( c3.wkbType(), QgsWkbTypes::GeometryCollection );
  QCOMPARE( c3.wktTypeStr(), QString( "GeometryCollection" ) );
  QCOMPARE( *( static_cast< const QgsLineString * >( c3.geometryN( 0 ) ) ), part );
  QgsGeometryCollection c4;
  part.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 0, 0, 2, 1 )
                  << QgsPoint( QgsWkbTypes::PointZM, 0, 10, 3, 2 ) << QgsPoint( QgsWkbTypes::PointZM, 10, 10, 5, 3 )
                  << QgsPoint( QgsWkbTypes::PointZM, 10, 0, 0, 4 ) << QgsPoint( QgsWkbTypes::PointZM, 0, 0, 2, 1 ) );
  c4.addGeometry( part.clone() );
  //QVERIFY( c4.is3D() ); //no meaning for collections?
  //QVERIFY( c4.isMeasure() ); //no meaning for collections?
  QCOMPARE( c4.wkbType(), QgsWkbTypes::GeometryCollection );
  QCOMPARE( c4.wktTypeStr(), QString( "GeometryCollection" ) );
  QCOMPARE( *( static_cast< const QgsLineString * >( c4.geometryN( 0 ) ) ), part );

  //add another part
  QgsGeometryCollection c6;
  part.setPoints( QgsPointSequence() << QgsPoint( 0, 0 ) << QgsPoint( 0, 10 ) << QgsPoint( 10, 10 )
                  << QgsPoint( 10, 0 ) << QgsPoint( 0, 0 ) );
  c6.addGeometry( part.clone() );
  QCOMPARE( c6.vertexCount( 0, 0 ), 5 );
  part.setPoints( QgsPointSequence() << QgsPoint( 1, 1 ) << QgsPoint( 1, 9 ) << QgsPoint( 9, 9 )
                  << QgsPoint( 9, 1 ) << QgsPoint( 1, 1 ) );
  c6.addGeometry( part.clone() );
  QCOMPARE( c6.vertexCount( 1, 0 ), 5 );
  QCOMPARE( c6.numGeometries(), 2 );
  QVERIFY( c6.geometryN( 0 ) );
  QCOMPARE( *static_cast< const QgsLineString * >( c6.geometryN( 1 ) ), part );

  QgsCoordinateSequence seq = c6.coordinateSequence();
  QCOMPARE( seq, QgsCoordinateSequence() << ( QgsRingSequence() << ( QgsPointSequence() << QgsPoint( 0, 0 ) << QgsPoint( 0, 10 ) << QgsPoint( 10, 10 )
            << QgsPoint( 10, 0 ) << QgsPoint( 0, 0 ) ) )
            << ( QgsRingSequence() << ( QgsPointSequence() << QgsPoint( 1, 1 ) << QgsPoint( 1, 9 ) << QgsPoint( 9, 9 )
                                        << QgsPoint( 9, 1 ) << QgsPoint( 1, 1 ) ) ) );
  QCOMPARE( c6.nCoordinates(), 10 );


  //clear
  QgsGeometryCollection c7;
  part.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZ, 0, 0, 1 )
                  << QgsPoint( QgsWkbTypes::PointZ, 0, 10, 2 ) << QgsPoint( QgsWkbTypes::PointZ, 10, 10, 3 )
                  << QgsPoint( QgsWkbTypes::PointZ, 10, 0, 4 ) << QgsPoint( QgsWkbTypes::PointZ, 0, 0, 1 ) );
  c7.addGeometry( part.clone() );
  part.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZ, 1, 1, 1 )
                  << QgsPoint( QgsWkbTypes::PointZ, 1, 9, 2 ) << QgsPoint( QgsWkbTypes::PointZ, 9, 9, 3 )
                  << QgsPoint( QgsWkbTypes::PointZ, 9, 1, 4 ) << QgsPoint( QgsWkbTypes::PointZ, 1, 1, 1 ) );
  c7.addGeometry( part.clone() );
  QCOMPARE( c7.numGeometries(), 2 );
  c7.clear();
  QVERIFY( c7.isEmpty() );
  QCOMPARE( c7.numGeometries(), 0 );
  QCOMPARE( c7.nCoordinates(), 0 );
  QCOMPARE( c7.ringCount(), 0 );
  QCOMPARE( c7.partCount(), 0 );
  QVERIFY( !c7.is3D() );
  QVERIFY( !c7.isMeasure() );
  QCOMPARE( c7.wkbType(), QgsWkbTypes::GeometryCollection );

  //clone
  QgsGeometryCollection c11;
  std::unique_ptr< QgsGeometryCollection >cloned( c11.clone() );
  QVERIFY( cloned->isEmpty() );
  part.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 0, 0, 1, 5 )
                  << QgsPoint( QgsWkbTypes::PointZM, 0, 10, 2, 6 ) << QgsPoint( QgsWkbTypes::PointZM, 10, 10, 3, 7 )
                  << QgsPoint( QgsWkbTypes::PointZM, 10, 0, 4, 8 ) << QgsPoint( QgsWkbTypes::PointZM, 0, 0, 1, 9 ) );
  c11.addGeometry( part.clone() );
  QgsLineString part2;
  part2.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 1, 1, 1, 2 )
                   << QgsPoint( QgsWkbTypes::PointZM, 1, 9, 2, 3 ) << QgsPoint( QgsWkbTypes::PointZM, 9, 9, 3, 6 )
                   << QgsPoint( QgsWkbTypes::PointZM, 9, 1, 4, 4 ) << QgsPoint( QgsWkbTypes::PointZM, 1, 1, 1, 7 ) );
  c11.addGeometry( part2.clone() );
  cloned.reset( c11.clone() );
  QCOMPARE( cloned->numGeometries(), 2 );
  QCOMPARE( *static_cast< const QgsLineString * >( cloned->geometryN( 0 ) ), part );
  QCOMPARE( *static_cast< const QgsLineString * >( cloned->geometryN( 1 ) ), part2 );

  //copy constructor
  QgsGeometryCollection c12;
  QgsGeometryCollection c13( c12 );
  QVERIFY( c13.isEmpty() );
  part.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 0, 0, 1, 5 )
                  << QgsPoint( QgsWkbTypes::PointZM, 0, 10, 2, 6 ) << QgsPoint( QgsWkbTypes::PointZM, 10, 10, 3, 7 )
                  << QgsPoint( QgsWkbTypes::PointZM, 10, 0, 4, 8 ) << QgsPoint( QgsWkbTypes::PointZM, 0, 0, 1, 9 ) );
  c12.addGeometry( part.clone() );
  part2.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 1, 1, 1, 2 )
                   << QgsPoint( QgsWkbTypes::PointZM, 1, 9, 2, 3 ) << QgsPoint( QgsWkbTypes::PointZM, 9, 9, 3, 6 )
                   << QgsPoint( QgsWkbTypes::PointZM, 9, 1, 4, 4 ) << QgsPoint( QgsWkbTypes::PointZM, 1, 1, 1, 7 ) );
  c12.addGeometry( part2.clone() );
  QgsGeometryCollection c14( c12 );
  QCOMPARE( c14.numGeometries(), 2 );
  QCOMPARE( *static_cast< const QgsLineString * >( c14.geometryN( 0 ) ), part );
  QCOMPARE( *static_cast< const QgsLineString * >( c14.geometryN( 1 ) ), part2 );

  //assignment operator
  QgsGeometryCollection c15;
  c15 = c13;
  QCOMPARE( c15.numGeometries(), 0 );
  c15 = c14;
  QCOMPARE( c15.numGeometries(), 2 );
  QCOMPARE( *static_cast< const QgsLineString * >( c15.geometryN( 0 ) ), part );
  QCOMPARE( *static_cast< const QgsLineString * >( c15.geometryN( 1 ) ), part2 );

  //equality
  QgsGeometryCollection emptyCollection;
  QVERIFY( !( emptyCollection == c15 ) );
  QVERIFY( emptyCollection != c15 );
  QgsPoint notCollection;
  QVERIFY( !( emptyCollection == notCollection ) );
  QVERIFY( emptyCollection != notCollection );
  QgsMultiPoint mp;
  QgsMultiLineString ml;
  QVERIFY( mp != ml );
  QgsMultiLineString ml2;
  part.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZ, 0, 0, 1 )
                  << QgsPoint( QgsWkbTypes::PointZ, 0, 10, 2 ) << QgsPoint( QgsWkbTypes::PointZ, 10, 10, 3 )
                  << QgsPoint( QgsWkbTypes::PointZ, 10, 0, 4 ) << QgsPoint( QgsWkbTypes::PointZ, 0, 0, 1 ) );
  ml.addGeometry( part.clone() );
  QVERIFY( ml != ml2 );
  part.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZ, 1, 1, 1 )
                  << QgsPoint( QgsWkbTypes::PointZ, 0, 10, 2 ) << QgsPoint( QgsWkbTypes::PointZ, 10, 10, 3 )
                  << QgsPoint( QgsWkbTypes::PointZ, 10, 0, 4 ) << QgsPoint( QgsWkbTypes::PointZ, 0, 0, 1 ) );
  ml2.addGeometry( part.clone() );
  QVERIFY( ml != ml2 );

  //toCurveType
  std::unique_ptr< QgsGeometryCollection > curveType( c12.toCurveType() );
  QCOMPARE( curveType->wkbType(), QgsWkbTypes::GeometryCollection );
  QCOMPARE( curveType->numGeometries(), 2 );
  const QgsCompoundCurve *curve = static_cast< const QgsCompoundCurve * >( curveType->geometryN( 0 ) );
  QCOMPARE( curve->numPoints(), 5 );
  QCOMPARE( curve->vertexAt( QgsVertexId( 0, 0, 0 ) ), QgsPoint( QgsWkbTypes::PointZM, 0, 0, 1, 5 ) );
  QCOMPARE( curve->vertexAt( QgsVertexId( 0, 0, 1 ) ), QgsPoint( QgsWkbTypes::PointZM, 0, 10, 2, 6 ) );
  QCOMPARE( curve->vertexAt( QgsVertexId( 0, 0, 2 ) ), QgsPoint( QgsWkbTypes::PointZM, 10, 10, 3, 7 ) );
  QCOMPARE( curve->vertexAt( QgsVertexId( 0, 0, 3 ) ), QgsPoint( QgsWkbTypes::PointZM, 10, 0, 4, 8 ) );
  QCOMPARE( curve->vertexAt( QgsVertexId( 0, 0, 4 ) ), QgsPoint( QgsWkbTypes::PointZM, 0, 0, 1, 9 ) );
  curve = static_cast< const QgsCompoundCurve * >( curveType->geometryN( 1 ) );
  QCOMPARE( curve->numPoints(), 5 );
  QCOMPARE( curve->vertexAt( QgsVertexId( 0, 0, 0 ) ), QgsPoint( QgsWkbTypes::PointZM, 1, 1, 1, 2 ) );
  QCOMPARE( curve->vertexAt( QgsVertexId( 0, 0, 1 ) ), QgsPoint( QgsWkbTypes::PointZM, 1, 9, 2, 3 ) );
  QCOMPARE( curve->vertexAt( QgsVertexId( 0, 0, 2 ) ), QgsPoint( QgsWkbTypes::PointZM, 9, 9, 3, 6 ) );
  QCOMPARE( curve->vertexAt( QgsVertexId( 0, 0, 3 ) ), QgsPoint( QgsWkbTypes::PointZM, 9, 1, 4, 4 ) );
  QCOMPARE( curve->vertexAt( QgsVertexId( 0, 0, 4 ) ), QgsPoint( QgsWkbTypes::PointZM, 1, 1, 1, 7 ) );

  //to/fromWKB
  QgsGeometryCollection c16;
  part.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::Point, 0, 0 )
                  << QgsPoint( QgsWkbTypes::Point, 0, 10 ) << QgsPoint( QgsWkbTypes::Point, 10, 10 )
                  << QgsPoint( QgsWkbTypes::Point, 10, 0 ) << QgsPoint( QgsWkbTypes::Point, 0, 0 ) );
  c16.addGeometry( part.clone() );
  part2.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::Point, 1, 1 )
                   << QgsPoint( QgsWkbTypes::Point, 1, 9 ) << QgsPoint( QgsWkbTypes::Point, 9, 9 )
                   << QgsPoint( QgsWkbTypes::Point, 9, 1 ) << QgsPoint( QgsWkbTypes::Point, 1, 1 ) );
  c16.addGeometry( part2.clone() );
  QByteArray wkb16 = c16.asWkb();
  QgsGeometryCollection c17;
  QgsConstWkbPtr wkb16ptr( wkb16 );
  c17.fromWkb( wkb16ptr );
  QCOMPARE( c17.numGeometries(), 2 );
  QCOMPARE( *static_cast< const QgsLineString * >( c17.geometryN( 0 ) ), part );
  QCOMPARE( *static_cast< const QgsLineString * >( c17.geometryN( 1 ) ), part2 );

  //parts with Z
  c16.clear();
  c17.clear();
  part.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZ, 0, 0, 1 )
                  << QgsPoint( QgsWkbTypes::PointZ, 0, 10, 2 ) << QgsPoint( QgsWkbTypes::PointZ, 10, 10, 3 )
                  << QgsPoint( QgsWkbTypes::PointZ, 10, 0, 4 ) << QgsPoint( QgsWkbTypes::PointZ, 0, 0, 1 ) );
  c16.addGeometry( part.clone() );
  part2.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZ, 1, 1, 1 )
                   << QgsPoint( QgsWkbTypes::PointZ, 1, 9, 2 ) << QgsPoint( QgsWkbTypes::PointZ, 9, 9, 3 )
                   << QgsPoint( QgsWkbTypes::PointZ, 9, 1, 4 ) << QgsPoint( QgsWkbTypes::PointZ, 1, 1, 1 ) );
  c16.addGeometry( part2.clone() );
  wkb16 = c16.asWkb();
  QgsConstWkbPtr wkb16ptr2( wkb16 );
  c17.fromWkb( wkb16ptr2 );
  QCOMPARE( c17.numGeometries(), 2 );
  QCOMPARE( *static_cast< const QgsLineString * >( c17.geometryN( 0 ) ), part );
  QCOMPARE( *static_cast< const QgsLineString * >( c17.geometryN( 1 ) ), part2 );


  //parts with m
  c16.clear();
  c17.clear();
  part.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointM, 0, 0, 0, 1 )
                  << QgsPoint( QgsWkbTypes::PointM, 0, 10,  0, 2 ) << QgsPoint( QgsWkbTypes::PointM, 10, 10, 0, 3 )
                  << QgsPoint( QgsWkbTypes::PointM, 10, 0,  0, 4 ) << QgsPoint( QgsWkbTypes::PointM, 0, 0, 0, 1 ) );
  c16.addGeometry( part.clone() );
  part2.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointM, 1, 1, 0, 1 )
                   << QgsPoint( QgsWkbTypes::PointM, 1, 9, 0, 2 ) << QgsPoint( QgsWkbTypes::PointM, 9, 9, 0, 3 )
                   << QgsPoint( QgsWkbTypes::PointM, 9, 1, 0, 4 ) << QgsPoint( QgsWkbTypes::PointM, 1, 1, 0, 1 ) );
  c16.addGeometry( part2.clone() );
  wkb16 = c16.asWkb();
  QgsConstWkbPtr wkb16ptr3( wkb16 );
  c17.fromWkb( wkb16ptr3 );
  QCOMPARE( c17.numGeometries(), 2 );
  QCOMPARE( *static_cast< const QgsLineString * >( c17.geometryN( 0 ) ), part );
  QCOMPARE( *static_cast< const QgsLineString * >( c17.geometryN( 1 ) ), part2 );

  // parts with ZM
  c16.clear();
  c17.clear();
  part.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 0, 0, 1, 5 )
                  << QgsPoint( QgsWkbTypes::PointZM, 0, 10, 2, 6 ) << QgsPoint( QgsWkbTypes::PointZM, 10, 10, 3, 7 )
                  << QgsPoint( QgsWkbTypes::PointZM, 10, 0, 4, 8 ) << QgsPoint( QgsWkbTypes::PointZM, 0, 0, 1, 9 ) );
  c16.addGeometry( part.clone() );
  part2.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 1, 1, 1, 2 )
                   << QgsPoint( QgsWkbTypes::PointZM, 1, 9, 2, 3 ) << QgsPoint( QgsWkbTypes::PointZM, 9, 9, 3, 6 )
                   << QgsPoint( QgsWkbTypes::PointZM, 9, 1, 4, 4 ) << QgsPoint( QgsWkbTypes::PointZM, 1, 1, 1, 7 ) );
  c16.addGeometry( part2.clone() );
  wkb16 = c16.asWkb();
  QgsConstWkbPtr wkb16ptr4( wkb16 );
  c17.fromWkb( wkb16ptr4 );
  QCOMPARE( c17.numGeometries(), 2 );
  QCOMPARE( *static_cast< const QgsLineString * >( c17.geometryN( 0 ) ), part );
  QCOMPARE( *static_cast< const QgsLineString * >( c17.geometryN( 1 ) ), part2 );


  //bad WKB - check for no crash
  c17.clear();
  QgsConstWkbPtr nullPtr( nullptr, 0 );
  QVERIFY( !c17.fromWkb( nullPtr ) );
  QCOMPARE( c17.wkbType(), QgsWkbTypes::GeometryCollection );
  QgsPoint point( 1, 2 );
  QByteArray wkbPoint = point.asWkb();
  QgsConstWkbPtr wkbPointPtr( wkbPoint );
  QVERIFY( !c17.fromWkb( wkbPointPtr ) );
  QCOMPARE( c17.wkbType(), QgsWkbTypes::GeometryCollection );

  //to/from WKT
  QgsGeometryCollection c18;
  part.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 0, 0, 1, 5 )
                  << QgsPoint( QgsWkbTypes::PointZM, 0, 10, 2, 6 ) << QgsPoint( QgsWkbTypes::PointZM, 10, 10, 3, 7 )
                  << QgsPoint( QgsWkbTypes::PointZM, 10, 0, 4, 8 ) << QgsPoint( QgsWkbTypes::PointZM, 0, 0, 1, 9 ) );
  c18.addGeometry( part.clone() );
  part2.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 1, 1, 1, 2 )
                   << QgsPoint( QgsWkbTypes::PointZM, 1, 9, 2, 3 ) << QgsPoint( QgsWkbTypes::PointZM, 9, 9, 3, 6 )
                   << QgsPoint( QgsWkbTypes::PointZM, 9, 1, 4, 4 ) << QgsPoint( QgsWkbTypes::PointZM, 1, 1, 1, 7 ) );
  c18.addGeometry( part2.clone() );

  QString wkt = c18.asWkt();
  QVERIFY( !wkt.isEmpty() );
  QgsGeometryCollection c19;
  QVERIFY( c19.fromWkt( wkt ) );
  QCOMPARE( c19.numGeometries(), 2 );
  QCOMPARE( *static_cast< const QgsLineString * >( c19.geometryN( 0 ) ), part );
  QCOMPARE( *static_cast< const QgsLineString * >( c19.geometryN( 1 ) ), part2 );

  //bad WKT
  QgsGeometryCollection c20;
  QVERIFY( !c20.fromWkt( "Point()" ) );
  QVERIFY( c20.isEmpty() );
  QCOMPARE( c20.numGeometries(), 0 );
  QCOMPARE( c20.wkbType(), QgsWkbTypes::GeometryCollection );

  //as JSON
  QgsGeometryCollection exportC;
  part.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::Point, 0, 0 )
                  << QgsPoint( QgsWkbTypes::Point, 0, 10 ) << QgsPoint( QgsWkbTypes::Point, 10, 10 )
                  << QgsPoint( QgsWkbTypes::Point, 10, 0 ) << QgsPoint( QgsWkbTypes::Point, 0, 0 ) );
  exportC.addGeometry( part.clone() );

  // GML document for compare
  QDomDocument doc( "gml" );


  // as GML2
  QString expectedSimpleGML2( QStringLiteral( "<MultiGeometry xmlns=\"gml\"><geometryMember xmlns=\"gml\"><LineString xmlns=\"gml\"><coordinates xmlns=\"gml\" cs=\",\" ts=\" \">0,0 0,10 10,10 10,0 0,0</coordinates></LineString></geometryMember></MultiGeometry>" ) );
  QString res = elemToString( exportC.asGml2( doc ) );
  QGSCOMPAREGML( res, expectedSimpleGML2 );
  QString expectedGML2empty( QStringLiteral( "<MultiGeometry xmlns=\"gml\"/>" ) );
  QGSCOMPAREGML( elemToString( QgsGeometryCollection().asGml2( doc ) ), expectedGML2empty );

  //as GML3
  QString expectedSimpleGML3( QStringLiteral( "<MultiGeometry xmlns=\"gml\"><geometryMember xmlns=\"gml\"><LineString xmlns=\"gml\"><posList xmlns=\"gml\" srsDimension=\"2\">0 0 0 10 10 10 10 0 0 0</posList></LineString></geometryMember></MultiGeometry>" ) );
  res = elemToString( exportC.asGml3( doc ) );
  QCOMPARE( res, expectedSimpleGML3 );
  QString expectedGML3empty( QStringLiteral( "<MultiGeometry xmlns=\"gml\"/>" ) );
  QGSCOMPAREGML( elemToString( QgsGeometryCollection().asGml3( doc ) ), expectedGML3empty );

  // as JSON
  QString expectedSimpleJson( "{\"type\": \"GeometryCollection\", \"geometries\": [{\"type\": \"LineString\", \"coordinates\": [ [0, 0], [0, 10], [10, 10], [10, 0], [0, 0]]}] }" );
  res = exportC.asJson();
  QCOMPARE( res, expectedSimpleJson );

  part.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::Point, 1, 1 )
                  << QgsPoint( QgsWkbTypes::Point, 1, 9 ) << QgsPoint( QgsWkbTypes::Point, 9, 9 )
                  << QgsPoint( QgsWkbTypes::Point, 9, 1 ) << QgsPoint( QgsWkbTypes::Point, 1, 1 ) );
  exportC.addGeometry( part.clone() );

  QString expectedJson( QStringLiteral( "{\"type\": \"GeometryCollection\", \"geometries\": [{\"type\": \"LineString\", \"coordinates\": [ [0, 0], [0, 10], [10, 10], [10, 0], [0, 0]]}, {\"type\": \"LineString\", \"coordinates\": [ [1, 1], [1, 9], [9, 9], [9, 1], [1, 1]]}] }" ) );
  res = exportC.asJson();
  QCOMPARE( res, expectedJson );

  QgsGeometryCollection exportFloat;
  part.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::Point, 10 / 9.0, 10 / 9.0 )
                  << QgsPoint( QgsWkbTypes::Point, 10 / 9.0, 100 / 9.0 ) << QgsPoint( QgsWkbTypes::Point, 100 / 9.0, 100 / 9.0 )
                  << QgsPoint( QgsWkbTypes::Point, 100 / 9.0, 10 / 9.0 ) << QgsPoint( QgsWkbTypes::Point, 10 / 9.0, 10 / 9.0 ) );
  exportFloat.addGeometry( part.clone() );
  part.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::Point, 2 / 3.0, 2 / 3.0 )
                  << QgsPoint( QgsWkbTypes::Point, 2 / 3.0, 4 / 3.0 ) << QgsPoint( QgsWkbTypes::Point, 4 / 3.0, 4 / 3.0 )
                  << QgsPoint( QgsWkbTypes::Point, 4 / 3.0, 2 / 3.0 ) << QgsPoint( QgsWkbTypes::Point, 2 / 3.0, 2 / 3.0 ) );
  exportFloat.addGeometry( part.clone() );

  QString expectedJsonPrec3( QStringLiteral( "{\"type\": \"GeometryCollection\", \"geometries\": [{\"type\": \"LineString\", \"coordinates\": [ [1.111, 1.111], [1.111, 11.111], [11.111, 11.111], [11.111, 1.111], [1.111, 1.111]]}, {\"type\": \"LineString\", \"coordinates\": [ [0.667, 0.667], [0.667, 1.333], [1.333, 1.333], [1.333, 0.667], [0.667, 0.667]]}] }" ) );
  res = exportFloat.asJson( 3 );
  QCOMPARE( res, expectedJsonPrec3 );

  // as GML2
  QString expectedGML2( QStringLiteral( "<MultiGeometry xmlns=\"gml\"><geometryMember xmlns=\"gml\"><LineString xmlns=\"gml\"><coordinates xmlns=\"gml\" cs=\",\" ts=\" \">0,0 0,10 10,10 10,0 0,0</coordinates></LineString></geometryMember><geometryMember xmlns=\"gml\"><LineString xmlns=\"gml\"><coordinates xmlns=\"gml\" cs=\",\" ts=\" \">1,1 1,9 9,9 9,1 1,1</coordinates></LineString></geometryMember></MultiGeometry>" ) );
  res = elemToString( exportC.asGml2( doc ) );
  QGSCOMPAREGML( res, expectedGML2 );
  QString expectedGML2prec3( QStringLiteral( "<MultiGeometry xmlns=\"gml\"><geometryMember xmlns=\"gml\"><LineString xmlns=\"gml\"><coordinates xmlns=\"gml\" cs=\",\" ts=\" \">1.111,1.111 1.111,11.111 11.111,11.111 11.111,1.111 1.111,1.111</coordinates></LineString></geometryMember><geometryMember xmlns=\"gml\"><LineString xmlns=\"gml\"><coordinates xmlns=\"gml\" cs=\",\" ts=\" \">0.667,0.667 0.667,1.333 1.333,1.333 1.333,0.667 0.667,0.667</coordinates></LineString></geometryMember></MultiGeometry>" ) );
  res = elemToString( exportFloat.asGml2( doc, 3 ) );
  QGSCOMPAREGML( res, expectedGML2prec3 );

  //as GML3
  QString expectedGML3( QStringLiteral( "<MultiGeometry xmlns=\"gml\"><geometryMember xmlns=\"gml\"><LineString xmlns=\"gml\"><posList xmlns=\"gml\" srsDimension=\"2\">0 0 0 10 10 10 10 0 0 0</posList></LineString></geometryMember><geometryMember xmlns=\"gml\"><LineString xmlns=\"gml\"><posList xmlns=\"gml\" srsDimension=\"2\">1 1 1 9 9 9 9 1 1 1</posList></LineString></geometryMember></MultiGeometry>" ) );
  res = elemToString( exportC.asGml3( doc ) );
  QCOMPARE( res, expectedGML3 );
  QString expectedGML3prec3( QStringLiteral( "<MultiGeometry xmlns=\"gml\"><geometryMember xmlns=\"gml\"><LineString xmlns=\"gml\"><posList xmlns=\"gml\" srsDimension=\"2\">1.111 1.111 1.111 11.111 11.111 11.111 11.111 1.111 1.111 1.111</posList></LineString></geometryMember><geometryMember xmlns=\"gml\"><LineString xmlns=\"gml\"><posList xmlns=\"gml\" srsDimension=\"2\">0.667 0.667 0.667 1.333 1.333 1.333 1.333 0.667 0.667 0.667</posList></LineString></geometryMember></MultiGeometry>" ) );
  res = elemToString( exportFloat.asGml3( doc, 3 ) );
  QCOMPARE( res, expectedGML3prec3 );

  // remove geometry
  QgsGeometryCollection rc;
  // no crash!
  rc.removeGeometry( -1 );
  rc.removeGeometry( 0 );
  rc.removeGeometry( 100 );
  part.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 0, 0, 1, 5 )
                  << QgsPoint( QgsWkbTypes::PointZM, 0, 10, 2, 6 ) << QgsPoint( QgsWkbTypes::PointZM, 10, 10, 3, 7 )
                  << QgsPoint( QgsWkbTypes::PointZM, 10, 0, 4, 8 ) << QgsPoint( QgsWkbTypes::PointZM, 0, 0, 1, 9 ) );
  rc.addGeometry( part.clone() );
  part2.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 1, 1, 1, 2 )
                   << QgsPoint( QgsWkbTypes::PointZM, 1, 9, 2, 3 ) << QgsPoint( QgsWkbTypes::PointZM, 9, 9, 3, 6 )
                   << QgsPoint( QgsWkbTypes::PointZM, 9, 1, 4, 4 ) << QgsPoint( QgsWkbTypes::PointZM, 1, 1, 1, 7 ) );
  rc.addGeometry( part2.clone() );
  // no crash
  rc.removeGeometry( -1 );
  rc.removeGeometry( 100 );

  rc.removeGeometry( 0 );
  QCOMPARE( rc.numGeometries(), 1 );
  QCOMPARE( *static_cast< const QgsLineString * >( rc.geometryN( 0 ) ), part2 );

  rc.addGeometry( part.clone() );
  rc.removeGeometry( 1 );
  QCOMPARE( rc.numGeometries(), 1 );
  QCOMPARE( *static_cast< const QgsLineString * >( rc.geometryN( 0 ) ), part2 );
  rc.removeGeometry( 0 );
  QCOMPARE( rc.numGeometries(), 0 );


  // insert geometry
  rc.clear();
  rc.insertGeometry( nullptr, 0 );
  QVERIFY( rc.isEmpty() );
  QCOMPARE( rc.numGeometries(), 0 );
  rc.insertGeometry( nullptr, -1 );
  QVERIFY( rc.isEmpty() );
  QCOMPARE( rc.numGeometries(), 0 );
  rc.insertGeometry( nullptr, 100 );
  QVERIFY( rc.isEmpty() );
  QCOMPARE( rc.numGeometries(), 0 );

  rc.insertGeometry( part.clone(), 0 );
  QCOMPARE( rc.numGeometries(), 1 );
  QCOMPARE( *static_cast< const QgsLineString * >( rc.geometryN( 0 ) ), part );
  rc.insertGeometry( part2.clone(), 0 );
  QCOMPARE( rc.numGeometries(), 2 );
  QCOMPARE( *static_cast< const QgsLineString * >( rc.geometryN( 0 ) ), part2 );
  QCOMPARE( *static_cast< const QgsLineString * >( rc.geometryN( 1 ) ), part );
  rc.removeGeometry( 0 );
  rc.insertGeometry( part2.clone(), 1 );
  QCOMPARE( rc.numGeometries(), 2 );
  QCOMPARE( *static_cast< const QgsLineString * >( rc.geometryN( 0 ) ), part );
  QCOMPARE( *static_cast< const QgsLineString * >( rc.geometryN( 1 ) ), part2 );
  rc.removeGeometry( 1 );
  rc.insertGeometry( part2.clone(), 2 );
  QCOMPARE( rc.numGeometries(), 2 );
  QCOMPARE( *static_cast< const QgsLineString * >( rc.geometryN( 0 ) ), part );
  QCOMPARE( *static_cast< const QgsLineString * >( rc.geometryN( 1 ) ), part2 );

  // cast
  QVERIFY( !QgsGeometryCollection().cast( nullptr ) );
  QgsGeometryCollection pCast;
  QVERIFY( QgsGeometryCollection().cast( &pCast ) );
  QgsGeometryCollection pCast2;
  pCast2.fromWkt( QStringLiteral( "GeometryCollectionZ(PolygonZ((0 0 0, 0 1 1, 1 0 2, 0 0 0)))" ) );
  QVERIFY( QgsGeometryCollection().cast( &pCast2 ) );
  pCast2.fromWkt( QStringLiteral( "GeometryCollectionM(PolygonM((0 0 1, 0 1 2, 1 0 3, 0 0 1)))" ) );
  QVERIFY( QgsGeometryCollection().cast( &pCast2 ) );
  pCast2.fromWkt( QStringLiteral( "GeometryCollectionZM(PolygonZM((0 0 0 1, 0 1 1 2, 1 0 2 3, 0 0 0 1)))" ) );
  QVERIFY( QgsGeometryCollection().cast( &pCast2 ) );

  //transform
  //CRS transform
  QgsCoordinateReferenceSystem sourceSrs;
  sourceSrs.createFromSrid( 3994 );
  QgsCoordinateReferenceSystem destSrs;
  destSrs.createFromSrid( 4202 ); // want a transform with ellipsoid change
  QgsCoordinateTransform tr( sourceSrs, destSrs, QgsProject::instance() );

  // 2d CRS transform
  QgsGeometryCollection pTransform;
  QgsLineString l21;
  l21.setPoints( QgsPointSequence() << QgsPoint( 6374985, -3626584 )
                 << QgsPoint( 6274985, -3526584 )
                 << QgsPoint( 6474985, -3526584 )
                 << QgsPoint( 6374985, -3626584 ) );
  pTransform.addGeometry( l21.clone() );
  pTransform.addGeometry( l21.clone() );
  pTransform.transform( tr, QgsCoordinateTransform::ForwardTransform );
  const QgsLineString *extR = static_cast< const QgsLineString * >( pTransform.geometryN( 0 ) );
  QGSCOMPARENEAR( extR->pointN( 0 ).x(), 175.771, 0.001 );
  QGSCOMPARENEAR( extR->pointN( 0 ).y(), -39.724, 0.001 );
  QGSCOMPARENEAR( extR->pointN( 1 ).x(),  174.581448, 0.001 );
  QGSCOMPARENEAR( extR->pointN( 1 ).y(), -38.7999, 0.001 );
  QGSCOMPARENEAR( extR->pointN( 2 ).x(),  176.958633, 0.001 );
  QGSCOMPARENEAR( extR->pointN( 2 ).y(), -38.7999, 0.001 );
  QGSCOMPARENEAR( extR->pointN( 3 ).x(),  175.771, 0.001 );
  QGSCOMPARENEAR( extR->pointN( 3 ).y(), -39.724, 0.001 );
  QGSCOMPARENEAR( extR->boundingBox().xMinimum(), 174.581448, 0.001 );
  QGSCOMPARENEAR( extR->boundingBox().yMinimum(), -39.724, 0.001 );
  QGSCOMPARENEAR( extR->boundingBox().xMaximum(), 176.959, 0.001 );
  QGSCOMPARENEAR( extR->boundingBox().yMaximum(), -38.7999, 0.001 );
  const QgsLineString *intR = static_cast< const QgsLineString * >( pTransform.geometryN( 1 ) );
  QGSCOMPARENEAR( intR->pointN( 0 ).x(), 175.771, 0.001 );
  QGSCOMPARENEAR( intR->pointN( 0 ).y(), -39.724, 0.001 );
  QGSCOMPARENEAR( intR->pointN( 1 ).x(),  174.581448, 0.001 );
  QGSCOMPARENEAR( intR->pointN( 1 ).y(), -38.7999, 0.001 );
  QGSCOMPARENEAR( intR->pointN( 2 ).x(),  176.958633, 0.001 );
  QGSCOMPARENEAR( intR->pointN( 2 ).y(), -38.7999, 0.001 );
  QGSCOMPARENEAR( intR->pointN( 3 ).x(),  175.771, 0.001 );
  QGSCOMPARENEAR( intR->pointN( 3 ).y(), -39.724, 0.001 );
  QGSCOMPARENEAR( intR->boundingBox().xMinimum(), 174.581448, 0.001 );
  QGSCOMPARENEAR( intR->boundingBox().yMinimum(), -39.724, 0.001 );
  QGSCOMPARENEAR( intR->boundingBox().xMaximum(), 176.959, 0.001 );
  QGSCOMPARENEAR( intR->boundingBox().yMaximum(), -38.7999, 0.001 );

  //3d CRS transform
  QgsLineString l22;
  l22.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 6374985, -3626584, 1, 2 )
                 << QgsPoint( QgsWkbTypes::PointZM, 6274985, -3526584, 3, 4 )
                 << QgsPoint( QgsWkbTypes::PointZM, 6474985, -3526584, 5, 6 )
                 << QgsPoint( QgsWkbTypes::PointZM, 6374985, -3626584, 1, 2 ) );
  pTransform.clear();
  pTransform.addGeometry( l22.clone() );
  pTransform.addGeometry( l22.clone() );
  pTransform.transform( tr, QgsCoordinateTransform::ForwardTransform );
  extR = static_cast< const QgsLineString * >( pTransform.geometryN( 0 ) );
  QGSCOMPARENEAR( extR->pointN( 0 ).x(), 175.771, 0.001 );
  QGSCOMPARENEAR( extR->pointN( 0 ).y(), -39.724, 0.001 );
  QGSCOMPARENEAR( extR->pointN( 0 ).z(), 1.0, 0.001 );
  QGSCOMPARENEAR( extR->pointN( 0 ).m(), 2.0, 0.001 );
  QGSCOMPARENEAR( extR->pointN( 1 ).x(),  174.581448, 0.001 );
  QGSCOMPARENEAR( extR->pointN( 1 ).y(), -38.7999, 0.001 );
  QGSCOMPARENEAR( extR->pointN( 1 ).z(), 3.0, 0.001 );
  QGSCOMPARENEAR( extR->pointN( 1 ).m(), 4.0, 0.001 );
  QGSCOMPARENEAR( extR->pointN( 2 ).x(),  176.958633, 0.001 );
  QGSCOMPARENEAR( extR->pointN( 2 ).y(), -38.7999, 0.001 );
  QGSCOMPARENEAR( extR->pointN( 2 ).z(), 5.0, 0.001 );
  QGSCOMPARENEAR( extR->pointN( 2 ).m(), 6.0, 0.001 );
  QGSCOMPARENEAR( extR->pointN( 3 ).x(),  175.771, 0.001 );
  QGSCOMPARENEAR( extR->pointN( 3 ).y(), -39.724, 0.001 );
  QGSCOMPARENEAR( extR->pointN( 3 ).z(), 1.0, 0.001 );
  QGSCOMPARENEAR( extR->pointN( 3 ).m(), 2.0, 0.001 );
  QGSCOMPARENEAR( extR->boundingBox().xMinimum(), 174.581448, 0.001 );
  QGSCOMPARENEAR( extR->boundingBox().yMinimum(), -39.724, 0.001 );
  QGSCOMPARENEAR( extR->boundingBox().xMaximum(), 176.959, 0.001 );
  QGSCOMPARENEAR( extR->boundingBox().yMaximum(), -38.7999, 0.001 );
  intR = static_cast< const QgsLineString * >( pTransform.geometryN( 1 ) );
  QGSCOMPARENEAR( intR->pointN( 0 ).x(), 175.771, 0.001 );
  QGSCOMPARENEAR( intR->pointN( 0 ).y(), -39.724, 0.001 );
  QGSCOMPARENEAR( intR->pointN( 0 ).z(), 1.0, 0.001 );
  QGSCOMPARENEAR( intR->pointN( 0 ).m(), 2.0, 0.001 );
  QGSCOMPARENEAR( intR->pointN( 1 ).x(),  174.581448, 0.001 );
  QGSCOMPARENEAR( intR->pointN( 1 ).y(), -38.7999, 0.001 );
  QGSCOMPARENEAR( intR->pointN( 1 ).z(), 3.0, 0.001 );
  QGSCOMPARENEAR( intR->pointN( 1 ).m(), 4.0, 0.001 );
  QGSCOMPARENEAR( intR->pointN( 2 ).x(),  176.958633, 0.001 );
  QGSCOMPARENEAR( intR->pointN( 2 ).y(), -38.7999, 0.001 );
  QGSCOMPARENEAR( intR->pointN( 2 ).z(), 5.0, 0.001 );
  QGSCOMPARENEAR( intR->pointN( 2 ).m(), 6.0, 0.001 );
  QGSCOMPARENEAR( intR->pointN( 3 ).x(),  175.771, 0.001 );
  QGSCOMPARENEAR( intR->pointN( 3 ).y(), -39.724, 0.001 );
  QGSCOMPARENEAR( intR->pointN( 3 ).z(), 1.0, 0.001 );
  QGSCOMPARENEAR( intR->pointN( 3 ).m(), 2.0, 0.001 );
  QGSCOMPARENEAR( intR->boundingBox().xMinimum(), 174.581448, 0.001 );
  QGSCOMPARENEAR( intR->boundingBox().yMinimum(), -39.724, 0.001 );
  QGSCOMPARENEAR( intR->boundingBox().xMaximum(), 176.959, 0.001 );
  QGSCOMPARENEAR( intR->boundingBox().yMaximum(), -38.7999, 0.001 );

  //reverse transform
  pTransform.transform( tr, QgsCoordinateTransform::ReverseTransform );
  extR = static_cast< const QgsLineString * >( pTransform.geometryN( 0 ) );
  QGSCOMPARENEAR( extR->pointN( 0 ).x(), 6374984, 100 );
  QGSCOMPARENEAR( extR->pointN( 0 ).y(), -3626584, 100 );
  QGSCOMPARENEAR( extR->pointN( 0 ).z(), 1.0, 0.001 );
  QGSCOMPARENEAR( extR->pointN( 0 ).m(), 2.0, 0.001 );
  QGSCOMPARENEAR( extR->pointN( 1 ).x(), 6274984, 100 );
  QGSCOMPARENEAR( extR->pointN( 1 ).y(), -3526584, 100 );
  QGSCOMPARENEAR( extR->pointN( 1 ).z(), 3.0, 0.001 );
  QGSCOMPARENEAR( extR->pointN( 1 ).m(), 4.0, 0.001 );
  QGSCOMPARENEAR( extR->pointN( 2 ).x(),  6474984, 100 );
  QGSCOMPARENEAR( extR->pointN( 2 ).y(), -3526584, 100 );
  QGSCOMPARENEAR( extR->pointN( 2 ).z(), 5.0, 0.001 );
  QGSCOMPARENEAR( extR->pointN( 2 ).m(), 6.0, 0.001 );
  QGSCOMPARENEAR( extR->pointN( 3 ).x(),  6374984, 100 );
  QGSCOMPARENEAR( extR->pointN( 3 ).y(), -3626584, 100 );
  QGSCOMPARENEAR( extR->pointN( 3 ).z(), 1.0, 0.001 );
  QGSCOMPARENEAR( extR->pointN( 3 ).m(), 2.0, 0.001 );
  QGSCOMPARENEAR( extR->boundingBox().xMinimum(), 6274984, 100 );
  QGSCOMPARENEAR( extR->boundingBox().yMinimum(), -3626584, 100 );
  QGSCOMPARENEAR( extR->boundingBox().xMaximum(), 6474984, 100 );
  QGSCOMPARENEAR( extR->boundingBox().yMaximum(), -3526584, 100 );
  intR = static_cast< const QgsLineString * >( pTransform.geometryN( 1 ) );
  QGSCOMPARENEAR( intR->pointN( 0 ).x(), 6374984, 100 );
  QGSCOMPARENEAR( intR->pointN( 0 ).y(), -3626584, 100 );
  QGSCOMPARENEAR( intR->pointN( 0 ).z(), 1.0, 0.001 );
  QGSCOMPARENEAR( intR->pointN( 0 ).m(), 2.0, 0.001 );
  QGSCOMPARENEAR( intR->pointN( 1 ).x(), 6274984, 100 );
  QGSCOMPARENEAR( intR->pointN( 1 ).y(), -3526584, 100 );
  QGSCOMPARENEAR( intR->pointN( 1 ).z(), 3.0, 0.001 );
  QGSCOMPARENEAR( intR->pointN( 1 ).m(), 4.0, 0.001 );
  QGSCOMPARENEAR( intR->pointN( 2 ).x(),  6474984, 100 );
  QGSCOMPARENEAR( intR->pointN( 2 ).y(), -3526584, 100 );
  QGSCOMPARENEAR( intR->pointN( 2 ).z(), 5.0, 0.001 );
  QGSCOMPARENEAR( intR->pointN( 2 ).m(), 6.0, 0.001 );
  QGSCOMPARENEAR( intR->pointN( 3 ).x(),  6374984, 100 );
  QGSCOMPARENEAR( intR->pointN( 3 ).y(), -3626584, 100 );
  QGSCOMPARENEAR( intR->pointN( 3 ).z(), 1.0, 0.001 );
  QGSCOMPARENEAR( intR->pointN( 3 ).m(), 2.0, 0.001 );
  QGSCOMPARENEAR( intR->boundingBox().xMinimum(), 6274984, 100 );
  QGSCOMPARENEAR( intR->boundingBox().yMinimum(), -3626584, 100 );
  QGSCOMPARENEAR( intR->boundingBox().xMaximum(), 6474984, 100 );
  QGSCOMPARENEAR( intR->boundingBox().yMaximum(), -3526584, 100 );

  //z value transform
  pTransform.transform( tr, QgsCoordinateTransform::ForwardTransform, true );
  extR = static_cast< const QgsLineString * >( pTransform.geometryN( 0 ) );
  QGSCOMPARENEAR( extR->pointN( 0 ).z(), -19.249066, 0.001 );
  QGSCOMPARENEAR( extR->pointN( 1 ).z(), -19.148357, 0.001 );
  QGSCOMPARENEAR( extR->pointN( 2 ).z(), -19.092128, 0.001 );
  QGSCOMPARENEAR( extR->pointN( 3 ).z(), -19.249066, 0.001 );
  intR = static_cast< const QgsLineString * >( pTransform.geometryN( 1 ) );
  QGSCOMPARENEAR( intR->pointN( 0 ).z(), -19.249066, 0.001 );
  QGSCOMPARENEAR( intR->pointN( 1 ).z(), -19.148357, 0.001 );
  QGSCOMPARENEAR( intR->pointN( 2 ).z(), -19.092128, 0.001 );
  QGSCOMPARENEAR( intR->pointN( 3 ).z(), -19.249066, 0.001 );
  pTransform.transform( tr, QgsCoordinateTransform::ReverseTransform, true );
  extR = static_cast< const QgsLineString * >( pTransform.geometryN( 0 ) );
  QGSCOMPARENEAR( extR->pointN( 0 ).z(), 1, 0.001 );
  QGSCOMPARENEAR( extR->pointN( 1 ).z(), 3, 0.001 );
  QGSCOMPARENEAR( extR->pointN( 2 ).z(), 5, 0.001 );
  QGSCOMPARENEAR( extR->pointN( 3 ).z(), 1, 0.001 );
  intR = static_cast< const QgsLineString * >( pTransform.geometryN( 1 ) );
  QGSCOMPARENEAR( intR->pointN( 0 ).z(), 1, 0.001 );
  QGSCOMPARENEAR( intR->pointN( 1 ).z(), 3, 0.001 );
  QGSCOMPARENEAR( intR->pointN( 2 ).z(), 5, 0.001 );
  QGSCOMPARENEAR( intR->pointN( 3 ).z(), 1, 0.001 );

  //QTransform transform
  QTransform qtr = QTransform::fromScale( 2, 3 );
  QgsLineString l23;
  l23.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZM, 1, 2, 3, 4 )
                 << QgsPoint( QgsWkbTypes::PointZM, 11, 12, 13, 14 )
                 << QgsPoint( QgsWkbTypes::PointZM, 1, 12, 23, 24 )
                 << QgsPoint( QgsWkbTypes::PointZM, 1, 2, 3, 4 ) );
  QgsGeometryCollection pTransform2;
  pTransform2.addGeometry( l23.clone() );
  pTransform2.addGeometry( l23.clone() );
  pTransform2.transform( qtr, 3, 2, 6, 3 );

  extR = static_cast< const QgsLineString * >( pTransform2.geometryN( 0 ) );
  QGSCOMPARENEAR( extR->pointN( 0 ).x(), 2, 100 );
  QGSCOMPARENEAR( extR->pointN( 0 ).y(), 6, 100 );
  QGSCOMPARENEAR( extR->pointN( 0 ).z(), 9.0, 0.001 );
  QGSCOMPARENEAR( extR->pointN( 0 ).m(), 18.0, 0.001 );
  QGSCOMPARENEAR( extR->pointN( 1 ).x(), 22, 100 );
  QGSCOMPARENEAR( extR->pointN( 1 ).y(), 36, 100 );
  QGSCOMPARENEAR( extR->pointN( 1 ).z(), 29.0, 0.001 );
  QGSCOMPARENEAR( extR->pointN( 1 ).m(), 48.0, 0.001 );
  QGSCOMPARENEAR( extR->pointN( 2 ).x(),  2, 100 );
  QGSCOMPARENEAR( extR->pointN( 2 ).y(), 36, 100 );
  QGSCOMPARENEAR( extR->pointN( 2 ).z(), 49.0, 0.001 );
  QGSCOMPARENEAR( extR->pointN( 2 ).m(), 78.0, 0.001 );
  QGSCOMPARENEAR( extR->pointN( 3 ).x(), 2, 100 );
  QGSCOMPARENEAR( extR->pointN( 3 ).y(), 6, 100 );
  QGSCOMPARENEAR( extR->pointN( 3 ).z(), 9.0, 0.001 );
  QGSCOMPARENEAR( extR->pointN( 3 ).m(), 18.0, 0.001 );
  QGSCOMPARENEAR( extR->boundingBox().xMinimum(), 2, 0.001 );
  QGSCOMPARENEAR( extR->boundingBox().yMinimum(), 6, 0.001 );
  QGSCOMPARENEAR( extR->boundingBox().xMaximum(), 22, 0.001 );
  QGSCOMPARENEAR( extR->boundingBox().yMaximum(), 36, 0.001 );
  intR = static_cast< const QgsLineString * >( pTransform2.geometryN( 1 ) );
  QGSCOMPARENEAR( intR->pointN( 0 ).x(), 2, 100 );
  QGSCOMPARENEAR( intR->pointN( 0 ).y(), 6, 100 );
  QGSCOMPARENEAR( intR->pointN( 0 ).z(), 9.0, 0.001 );
  QGSCOMPARENEAR( intR->pointN( 0 ).m(), 18.0, 0.001 );
  QGSCOMPARENEAR( intR->pointN( 1 ).x(), 22, 100 );
  QGSCOMPARENEAR( intR->pointN( 1 ).y(), 36, 100 );
  QGSCOMPARENEAR( intR->pointN( 1 ).z(), 29.0, 0.001 );
  QGSCOMPARENEAR( intR->pointN( 1 ).m(), 48.0, 0.001 );
  QGSCOMPARENEAR( intR->pointN( 2 ).x(),  2, 100 );
  QGSCOMPARENEAR( intR->pointN( 2 ).y(), 36, 100 );
  QGSCOMPARENEAR( intR->pointN( 2 ).z(), 49.0, 0.001 );
  QGSCOMPARENEAR( intR->pointN( 2 ).m(), 78.0, 0.001 );
  QGSCOMPARENEAR( intR->pointN( 3 ).x(), 2, 100 );
  QGSCOMPARENEAR( intR->pointN( 3 ).y(), 6, 100 );
  QGSCOMPARENEAR( intR->pointN( 3 ).z(), 9.0, 0.001 );
  QGSCOMPARENEAR( intR->pointN( 3 ).m(), 18.0, 0.001 );
  QGSCOMPARENEAR( intR->boundingBox().xMinimum(), 2, 0.001 );
  QGSCOMPARENEAR( intR->boundingBox().yMinimum(), 6, 0.001 );
  QGSCOMPARENEAR( intR->boundingBox().xMaximum(), 22, 0.001 );
  QGSCOMPARENEAR( intR->boundingBox().yMaximum(), 36, 0.001 );


  // closestSegment
  QgsPoint pt;
  QgsVertexId v;
  int leftOf = 0;
  QgsGeometryCollection empty;
  ( void )empty.closestSegment( QgsPoint( 1, 2 ), pt, v ); // empty collection, just want no crash

  QgsGeometryCollection p21;
  QgsLineString p21ls;
  p21ls.setPoints( QgsPointSequence() << QgsPoint( 5, 10 ) << QgsPoint( 7, 12 ) << QgsPoint( 5, 15 ) << QgsPoint( 5, 10 ) );
  p21.addGeometry( p21ls.clone() );
  QGSCOMPARENEAR( p21.closestSegment( QgsPoint( 4, 11 ), pt, v, &leftOf ), 1.0, 0.0001 );
  QGSCOMPARENEAR( pt.x(), 5, 0.01 );
  QGSCOMPARENEAR( pt.y(), 11, 0.01 );
  QCOMPARE( v, QgsVertexId( 0, 0, 3 ) );
  QCOMPARE( leftOf, 1 );
  QGSCOMPARENEAR( p21.closestSegment( QgsPoint( 8, 11 ), pt, v, &leftOf ),  2.0, 0.0001 );
  QGSCOMPARENEAR( pt.x(), 7, 0.01 );
  QGSCOMPARENEAR( pt.y(), 12, 0.01 );
  QCOMPARE( v, QgsVertexId( 0, 0, 1 ) );
  QCOMPARE( leftOf, 1 );
  QGSCOMPARENEAR( p21.closestSegment( QgsPoint( 6, 11.5 ), pt, v, &leftOf ), 0.125000, 0.0001 );
  QGSCOMPARENEAR( pt.x(), 6.25, 0.01 );
  QGSCOMPARENEAR( pt.y(), 11.25, 0.01 );
  QCOMPARE( v, QgsVertexId( 0, 0, 1 ) );
  QCOMPARE( leftOf, -1 );
  QGSCOMPARENEAR( p21.closestSegment( QgsPoint( 7, 16 ), pt, v, &leftOf ), 4.923077, 0.0001 );
  QGSCOMPARENEAR( pt.x(), 5.153846, 0.01 );
  QGSCOMPARENEAR( pt.y(), 14.769231, 0.01 );
  QCOMPARE( v, QgsVertexId( 0, 0, 2 ) );
  QCOMPARE( leftOf, 1 );
  QGSCOMPARENEAR( p21.closestSegment( QgsPoint( 5.5, 13.5 ), pt, v, &leftOf ), 0.173077, 0.0001 );
  QGSCOMPARENEAR( pt.x(), 5.846154, 0.01 );
  QGSCOMPARENEAR( pt.y(), 13.730769, 0.01 );
  QCOMPARE( v, QgsVertexId( 0, 0, 2 ) );
  QCOMPARE( leftOf, -1 );
  // point directly on segment
  QCOMPARE( p21.closestSegment( QgsPoint( 5, 15 ), pt, v, &leftOf ), 0.0 );
  QCOMPARE( pt, QgsPoint( 5, 15 ) );
  QCOMPARE( v, QgsVertexId( 0, 0, 2 ) );
  // with interior ring
  p21ls.setPoints( QgsPointSequence() << QgsPoint( 6, 11.5 ) << QgsPoint( 6.5, 12 ) << QgsPoint( 6, 13 ) << QgsPoint( 6, 11.5 ) );
  p21.addGeometry( p21ls.clone() );
  QGSCOMPARENEAR( p21.closestSegment( QgsPoint( 4, 11 ), pt, v, &leftOf ), 1.0, 0.0001 );
  QGSCOMPARENEAR( pt.x(), 5, 0.01 );
  QGSCOMPARENEAR( pt.y(), 11, 0.01 );
  QCOMPARE( v, QgsVertexId( 0, 0, 3 ) );
  QCOMPARE( leftOf, 1 );
  QGSCOMPARENEAR( p21.closestSegment( QgsPoint( 8, 11 ), pt, v, &leftOf ),  2.0, 0.0001 );
  QGSCOMPARENEAR( pt.x(), 7, 0.01 );
  QGSCOMPARENEAR( pt.y(), 12, 0.01 );
  QCOMPARE( v, QgsVertexId( 0, 0, 1 ) );
  QCOMPARE( leftOf, 1 );
  QGSCOMPARENEAR( p21.closestSegment( QgsPoint( 6, 11.4 ), pt, v, &leftOf ), 0.01, 0.0001 );
  QGSCOMPARENEAR( pt.x(), 6.0, 0.01 );
  QGSCOMPARENEAR( pt.y(), 11.5, 0.01 );
  QCOMPARE( v, QgsVertexId( 1, 0, 1 ) );
  QCOMPARE( leftOf, 1 );
  QGSCOMPARENEAR( p21.closestSegment( QgsPoint( 7, 16 ), pt, v, &leftOf ), 4.923077, 0.0001 );
  QGSCOMPARENEAR( pt.x(), 5.153846, 0.01 );
  QGSCOMPARENEAR( pt.y(), 14.769231, 0.01 );
  QCOMPARE( v, QgsVertexId( 0, 0, 2 ) );
  QCOMPARE( leftOf, 1 );
  QGSCOMPARENEAR( p21.closestSegment( QgsPoint( 5.5, 13.5 ), pt, v, &leftOf ), 0.173077, 0.0001 );
  QGSCOMPARENEAR( pt.x(), 5.846154, 0.01 );
  QGSCOMPARENEAR( pt.y(), 13.730769, 0.01 );
  QCOMPARE( v, QgsVertexId( 0, 0, 2 ) );
  QCOMPARE( leftOf, -1 );
  // point directly on segment
  QCOMPARE( p21.closestSegment( QgsPoint( 6, 13 ), pt, v, &leftOf ), 0.0 );
  QCOMPARE( pt, QgsPoint( 6, 13 ) );
  QCOMPARE( v, QgsVertexId( 1, 0, 2 ) );
  QCOMPARE( leftOf, 0 );

  //nextVertex
  QgsGeometryCollection p22;
  QVERIFY( !p22.nextVertex( v, pt ) );
  v = QgsVertexId( 0, 0, -2 );
  QVERIFY( !p22.nextVertex( v, pt ) );
  v = QgsVertexId( 0, 0, 10 );
  QVERIFY( !p22.nextVertex( v, pt ) );
  QgsLineString lp22;
  lp22.setPoints( QgsPointSequence() << QgsPoint( 1, 2 ) << QgsPoint( 11, 12 ) << QgsPoint( 1, 12 ) << QgsPoint( 1, 2 ) );
  p22.addGeometry( lp22.clone() );
  v = QgsVertexId( 0, 0, 4 ); //out of range
  QVERIFY( !p22.nextVertex( v, pt ) );
  v = QgsVertexId( 0, 0, -5 );
  QVERIFY( p22.nextVertex( v, pt ) );
  v = QgsVertexId( 0, 0, -1 );
  QVERIFY( p22.nextVertex( v, pt ) );
  QCOMPARE( v, QgsVertexId( 0, 0, 0 ) );
  QCOMPARE( pt, QgsPoint( 1, 2 ) );
  QVERIFY( p22.nextVertex( v, pt ) );
  QCOMPARE( v, QgsVertexId( 0, 0, 1 ) );
  QCOMPARE( pt, QgsPoint( 11, 12 ) );
  QVERIFY( p22.nextVertex( v, pt ) );
  QCOMPARE( v, QgsVertexId( 0, 0, 2 ) );
  QCOMPARE( pt, QgsPoint( 1, 12 ) );
  QVERIFY( p22.nextVertex( v, pt ) );
  QCOMPARE( v, QgsVertexId( 0, 0, 3 ) );
  QCOMPARE( pt, QgsPoint( 1, 2 ) );
  v = QgsVertexId( 1, 0, 0 );
  QVERIFY( !p22.nextVertex( v, pt ) );
  v = QgsVertexId( 1, 0, 0 );
  // add another part
  lp22.setPoints( QgsPointSequence() << QgsPoint( 11, 12 ) << QgsPoint( 21, 22 ) << QgsPoint( 11, 22 ) << QgsPoint( 11, 12 ) );
  p22.addGeometry( lp22.clone() );
  v = QgsVertexId( 1, 0, 4 ); //out of range
  QVERIFY( !p22.nextVertex( v, pt ) );
  v = QgsVertexId( 1, 0, -5 );
  QVERIFY( p22.nextVertex( v, pt ) );
  v = QgsVertexId( 1, 0, -1 );
  QVERIFY( p22.nextVertex( v, pt ) );
  QCOMPARE( v, QgsVertexId( 1, 0, 0 ) );
  QCOMPARE( pt, QgsPoint( 11, 12 ) );
  QVERIFY( p22.nextVertex( v, pt ) );
  QCOMPARE( v, QgsVertexId( 1, 0, 1 ) );
  QCOMPARE( pt, QgsPoint( 21, 22 ) );
  QVERIFY( p22.nextVertex( v, pt ) );
  QCOMPARE( v, QgsVertexId( 1, 0, 2 ) );
  QCOMPARE( pt, QgsPoint( 11, 22 ) );
  QVERIFY( p22.nextVertex( v, pt ) );
  QCOMPARE( v, QgsVertexId( 1, 0, 3 ) );
  QCOMPARE( pt, QgsPoint( 11, 12 ) );
  v = QgsVertexId( 2, 0, 0 );
  QVERIFY( !p22.nextVertex( v, pt ) );
  v = QgsVertexId( 1, 1, 0 );
  QVERIFY( p22.nextVertex( v, pt ) );
  QCOMPARE( v, QgsVertexId( 1, 1, 1 ) ); //test that part number is maintained
  QCOMPARE( pt, QgsPoint( 21, 22 ) );


  // dropZValue
  QgsGeometryCollection p23;
  p23.dropZValue();
  QCOMPARE( p23.wkbType(), QgsWkbTypes::GeometryCollection );
  QgsLineString lp23;
  lp23.setPoints( QgsPointSequence() << QgsPoint( 1, 2 ) << QgsPoint( 11, 12 ) << QgsPoint( 1, 12 ) << QgsPoint( 1, 2 ) );
  p23.addGeometry( lp23.clone() );
  p23.addGeometry( lp23.clone() );
  QCOMPARE( p23.wkbType(), QgsWkbTypes::GeometryCollection );
  p23.dropZValue(); // not z
  QCOMPARE( p23.wkbType(), QgsWkbTypes::GeometryCollection );
  QCOMPARE( p23.geometryN( 0 )->wkbType(), QgsWkbTypes::LineString );
  QCOMPARE( static_cast< const QgsLineString *>( p23.geometryN( 0 ) )->pointN( 0 ), QgsPoint( 1, 2 ) );
  QCOMPARE( p23.geometryN( 1 )->wkbType(), QgsWkbTypes::LineString );
  QCOMPARE( static_cast< const QgsLineString *>( p23.geometryN( 1 ) )->pointN( 0 ), QgsPoint( 1, 2 ) );
  // with z
  lp23.setPoints( QgsPointSequence() << QgsPoint( 1, 2, 3 ) << QgsPoint( 11, 12, 13 ) << QgsPoint( 1, 12, 23 ) << QgsPoint( 1, 2, 3 ) );
  p23.clear();
  p23.addGeometry( lp23.clone() );
  p23.addGeometry( lp23.clone() );
  p23.dropZValue();
  QCOMPARE( p23.wkbType(), QgsWkbTypes::GeometryCollection );
  QCOMPARE( p23.geometryN( 0 )->wkbType(), QgsWkbTypes::LineString );
  QCOMPARE( static_cast< const QgsLineString *>( p23.geometryN( 0 ) )->pointN( 0 ), QgsPoint( 1, 2 ) );
  QCOMPARE( p23.geometryN( 1 )->wkbType(), QgsWkbTypes::LineString );
  QCOMPARE( static_cast< const QgsLineString *>( p23.geometryN( 1 ) )->pointN( 0 ), QgsPoint( 1, 2 ) );
  // with zm
  lp23.setPoints( QgsPointSequence() << QgsPoint( 1, 2, 3, 4 ) << QgsPoint( 11, 12, 13, 14 ) << QgsPoint( 1, 12, 23, 24 ) << QgsPoint( 1, 2, 3, 4 ) );
  p23.clear();
  p23.addGeometry( lp23.clone() );
  p23.addGeometry( lp23.clone() );
  p23.dropZValue();
  QCOMPARE( p23.wkbType(), QgsWkbTypes::GeometryCollection );
  QCOMPARE( p23.geometryN( 0 )->wkbType(), QgsWkbTypes::LineStringM );
  QCOMPARE( static_cast< const QgsLineString *>( p23.geometryN( 0 ) )->pointN( 0 ), QgsPoint( QgsWkbTypes::PointM, 1, 2, 0, 4 ) );
  QCOMPARE( p23.geometryN( 1 )->wkbType(), QgsWkbTypes::LineStringM );
  QCOMPARE( static_cast< const QgsLineString *>( p23.geometryN( 1 ) )->pointN( 0 ), QgsPoint( QgsWkbTypes::PointM, 1, 2, 0, 4 ) );


  // dropMValue
  p23.clear();
  p23.dropMValue();
  QCOMPARE( p23.wkbType(), QgsWkbTypes::GeometryCollection );
  lp23.setPoints( QgsPointSequence() << QgsPoint( 1, 2 ) << QgsPoint( 11, 12 ) << QgsPoint( 1, 12 ) << QgsPoint( 1, 2 ) );
  p23.addGeometry( lp23.clone() );
  p23.addGeometry( lp23.clone() );
  QCOMPARE( p23.wkbType(), QgsWkbTypes::GeometryCollection );
  p23.dropMValue(); // not zm
  QCOMPARE( p23.wkbType(), QgsWkbTypes::GeometryCollection );
  QCOMPARE( p23.geometryN( 0 )->wkbType(), QgsWkbTypes::LineString );
  QCOMPARE( static_cast< const QgsLineString *>( p23.geometryN( 0 ) )->pointN( 0 ), QgsPoint( 1, 2 ) );
  QCOMPARE( p23.geometryN( 1 )->wkbType(), QgsWkbTypes::LineString );
  QCOMPARE( static_cast< const QgsLineString *>( p23.geometryN( 1 ) )->pointN( 0 ), QgsPoint( 1, 2 ) );
  // with m
  lp23.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointM,  1, 2, 0, 3 ) << QgsPoint( QgsWkbTypes::PointM, 11, 12, 0, 13 ) << QgsPoint( QgsWkbTypes::PointM, 1, 12, 0, 23 ) << QgsPoint( QgsWkbTypes::PointM,  1, 2, 0, 3 ) );
  p23.clear();
  p23.addGeometry( lp23.clone() );
  p23.addGeometry( lp23.clone() );
  p23.dropMValue();
  QCOMPARE( p23.wkbType(), QgsWkbTypes::GeometryCollection );
  QCOMPARE( p23.geometryN( 0 )->wkbType(), QgsWkbTypes::LineString );
  QCOMPARE( static_cast< const QgsLineString *>( p23.geometryN( 0 ) )->pointN( 0 ), QgsPoint( 1, 2 ) );
  QCOMPARE( p23.geometryN( 1 )->wkbType(), QgsWkbTypes::LineString );
  QCOMPARE( static_cast< const QgsLineString *>( p23.geometryN( 1 ) )->pointN( 0 ), QgsPoint( 1, 2 ) );
  // with zm
  lp23.setPoints( QgsPointSequence() << QgsPoint( 1, 2, 3, 4 ) << QgsPoint( 11, 12, 13, 14 ) << QgsPoint( 1, 12, 23, 24 ) << QgsPoint( 1, 2, 3, 4 ) );
  p23.clear();
  p23.addGeometry( lp23.clone() );
  p23.addGeometry( lp23.clone() );
  p23.dropMValue();
  QCOMPARE( p23.wkbType(), QgsWkbTypes::GeometryCollection );
  QCOMPARE( p23.geometryN( 0 )->wkbType(), QgsWkbTypes::LineStringZ );
  QCOMPARE( static_cast< const QgsLineString *>( p23.geometryN( 0 ) )->pointN( 0 ), QgsPoint( QgsWkbTypes::PointZ, 1, 2, 3 ) );
  QCOMPARE( p23.geometryN( 1 )->wkbType(), QgsWkbTypes::LineStringZ );
  QCOMPARE( static_cast< const QgsLineString *>( p23.geometryN( 1 ) )->pointN( 0 ), QgsPoint( QgsWkbTypes::PointZ, 1, 2, 3 ) );

  //vertexAngle
  QgsGeometryCollection p24;
  ( void )p24.vertexAngle( QgsVertexId() ); //just want no crash
  ( void )p24.vertexAngle( QgsVertexId( 0, 0, 0 ) ); //just want no crash
  ( void )p24.vertexAngle( QgsVertexId( 0, 1, 0 ) ); //just want no crash
  ( void )p24.vertexAngle( QgsVertexId( 1, 0, 0 ) ); //just want no crash
  ( void )p24.vertexAngle( QgsVertexId( -1, 0, 0 ) ); //just want no crash
  QgsLineString l38;
  l38.setPoints( QgsPointSequence() << QgsPoint( 0, 0 ) << QgsPoint( 0.5, 0 ) << QgsPoint( 1, 0 )
                 << QgsPoint( 2, 1 ) << QgsPoint( 1, 2 ) << QgsPoint( 0, 2 ) << QgsPoint( 0, 0 ) );
  p24.addGeometry( l38.clone() );
  QGSCOMPARENEAR( p24.vertexAngle( QgsVertexId( 0, 0, 0 ) ), 2.35619, 0.00001 );
  QGSCOMPARENEAR( p24.vertexAngle( QgsVertexId( 0, 0, 1 ) ), 1.5708, 0.0001 );
  QGSCOMPARENEAR( p24.vertexAngle( QgsVertexId( 0, 0, 2 ) ), 1.17809, 0.00001 );
  QGSCOMPARENEAR( p24.vertexAngle( QgsVertexId( 0, 0, 3 ) ), 0.0, 0.00001 );
  QGSCOMPARENEAR( p24.vertexAngle( QgsVertexId( 0, 0, 4 ) ), 5.10509, 0.00001 );
  QGSCOMPARENEAR( p24.vertexAngle( QgsVertexId( 0, 0, 5 ) ), 3.92699, 0.00001 );
  QGSCOMPARENEAR( p24.vertexAngle( QgsVertexId( 0, 0, 6 ) ), 2.35619, 0.00001 );
  p24.addGeometry( l38.clone() );
  QGSCOMPARENEAR( p24.vertexAngle( QgsVertexId( 1, 0, 0 ) ), 2.35619, 0.00001 );
  QGSCOMPARENEAR( p24.vertexAngle( QgsVertexId( 1, 0, 1 ) ), 1.5708, 0.0001 );
  QGSCOMPARENEAR( p24.vertexAngle( QgsVertexId( 1, 0, 2 ) ), 1.17809, 0.00001 );
  QGSCOMPARENEAR( p24.vertexAngle( QgsVertexId( 1, 0, 3 ) ), 0.0, 0.00001 );
  QGSCOMPARENEAR( p24.vertexAngle( QgsVertexId( 1, 0, 4 ) ), 5.10509, 0.00001 );
  QGSCOMPARENEAR( p24.vertexAngle( QgsVertexId( 1, 0, 5 ) ), 3.92699, 0.00001 );
  QGSCOMPARENEAR( p24.vertexAngle( QgsVertexId( 1, 0, 6 ) ), 2.35619, 0.00001 );

  //insert vertex

  //insert vertex in empty collection
  QgsGeometryCollection p25;
  QVERIFY( !p25.insertVertex( QgsVertexId( 0, 0, 0 ), QgsPoint( 6.0, 7.0 ) ) );
  QVERIFY( !p25.insertVertex( QgsVertexId( 0, 0, 1 ), QgsPoint( 6.0, 7.0 ) ) );
  QVERIFY( !p25.insertVertex( QgsVertexId( 0, 1, 0 ), QgsPoint( 6.0, 7.0 ) ) );
  QVERIFY( !p25.insertVertex( QgsVertexId( 1, 0, 0 ), QgsPoint( 6.0, 7.0 ) ) );
  QVERIFY( p25.isEmpty() );
  l38.setPoints( QgsPointSequence() << QgsPoint( 0, 0 ) << QgsPoint( 0.5, 0 ) << QgsPoint( 1, 0 )
                 << QgsPoint( 2, 1 ) << QgsPoint( 1, 2 ) << QgsPoint( 0, 2 ) << QgsPoint( 0, 0 ) );
  p25.addGeometry( l38.clone() );
  QVERIFY( p25.insertVertex( QgsVertexId( 0, 0, 1 ), QgsPoint( 0.3, 0 ) ) );
  QCOMPARE( p25.nCoordinates(), 8 );
  QCOMPARE( static_cast< const QgsLineString * >( p25.geometryN( 0 ) )->pointN( 0 ), QgsPoint( 0, 0 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p25.geometryN( 0 ) )->pointN( 1 ), QgsPoint( 0.3, 0 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p25.geometryN( 0 ) )->pointN( 2 ), QgsPoint( 0.5, 0 ) );
  QVERIFY( !p25.insertVertex( QgsVertexId( 0, 0, -1 ), QgsPoint( 6.0, 7.0 ) ) );
  QVERIFY( !p25.insertVertex( QgsVertexId( 0, 0, 100 ), QgsPoint( 6.0, 7.0 ) ) );
  QVERIFY( !p25.insertVertex( QgsVertexId( 1, 0, 0 ), QgsPoint( 6.0, 7.0 ) ) );
  // first vertex
  QVERIFY( p25.insertVertex( QgsVertexId( 0, 0, 0 ), QgsPoint( 0, 0.1 ) ) );
  QCOMPARE( p25.nCoordinates(), 9 );
  QCOMPARE( static_cast< const QgsLineString * >( p25.geometryN( 0 ) )->pointN( 0 ), QgsPoint( 0, 0.1 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p25.geometryN( 0 ) )->pointN( 1 ), QgsPoint( 0, 0 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p25.geometryN( 0 ) )->pointN( 2 ), QgsPoint( 0.3, 0 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p25.geometryN( 0 ) )->pointN( 3 ), QgsPoint( 0.5, 0 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p25.geometryN( 0 ) )->pointN( 7 ), QgsPoint( 0, 2 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p25.geometryN( 0 ) )->pointN( 8 ), QgsPoint( 0, 0 ) );
  // last vertex
  QVERIFY( p25.insertVertex( QgsVertexId( 0, 0, 9 ), QgsPoint( 0.1, 0.1 ) ) );
  QCOMPARE( p25.nCoordinates(), 10 );
  QCOMPARE( static_cast< const QgsLineString * >( p25.geometryN( 0 ) )->pointN( 0 ), QgsPoint( 0, 0.1 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p25.geometryN( 0 ) )->pointN( 1 ), QgsPoint( 0, 0 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p25.geometryN( 0 ) )->pointN( 2 ), QgsPoint( 0.3, 0 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p25.geometryN( 0 ) )->pointN( 3 ), QgsPoint( 0.5, 0 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p25.geometryN( 0 ) )->pointN( 8 ), QgsPoint( 0, 0 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p25.geometryN( 0 ) )->pointN( 9 ), QgsPoint( 0.1, 0.1 ) );
  // with second part
  p25.addGeometry( l38.clone() );
  QCOMPARE( p25.nCoordinates(), 17 );
  QVERIFY( p25.insertVertex( QgsVertexId( 1, 0, 1 ), QgsPoint( 0.3, 0 ) ) );
  QCOMPARE( p25.nCoordinates(), 18 );
  QCOMPARE( static_cast< const QgsLineString * >( p25.geometryN( 1 ) )->pointN( 0 ), QgsPoint( 0, 0 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p25.geometryN( 1 ) )->pointN( 1 ), QgsPoint( 0.3, 0 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p25.geometryN( 1 ) )->pointN( 2 ), QgsPoint( 0.5, 0 ) );
  QVERIFY( !p25.insertVertex( QgsVertexId( 1, 0, -1 ), QgsPoint( 6.0, 7.0 ) ) );
  QVERIFY( !p25.insertVertex( QgsVertexId( 1, 0, 100 ), QgsPoint( 6.0, 7.0 ) ) );
  QVERIFY( !p25.insertVertex( QgsVertexId( 2, 0, 0 ), QgsPoint( 6.0, 7.0 ) ) );
  // first vertex in second part
  QVERIFY( p25.insertVertex( QgsVertexId( 1, 0, 0 ), QgsPoint( 0, 0.1 ) ) );
  QCOMPARE( p25.nCoordinates(), 19 );
  QCOMPARE( static_cast< const QgsLineString * >( p25.geometryN( 1 ) )->pointN( 0 ), QgsPoint( 0, 0.1 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p25.geometryN( 1 ) )->pointN( 1 ), QgsPoint( 0, 0 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p25.geometryN( 1 ) )->pointN( 2 ), QgsPoint( 0.3, 0 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p25.geometryN( 1 ) )->pointN( 3 ), QgsPoint( 0.5, 0 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p25.geometryN( 1 ) )->pointN( 7 ), QgsPoint( 0, 2 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p25.geometryN( 1 ) )->pointN( 8 ), QgsPoint( 0, 0 ) );
  // last vertex in second part
  QVERIFY( p25.insertVertex( QgsVertexId( 1, 0, 9 ), QgsPoint( 0.1, 0.1 ) ) );
  QCOMPARE( p25.nCoordinates(), 20 );
  QCOMPARE( static_cast< const QgsLineString * >( p25.geometryN( 1 ) )->pointN( 0 ), QgsPoint( 0, 0.1 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p25.geometryN( 1 ) )->pointN( 1 ), QgsPoint( 0, 0 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p25.geometryN( 1 ) )->pointN( 2 ), QgsPoint( 0.3, 0 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p25.geometryN( 1 ) )->pointN( 3 ), QgsPoint( 0.5, 0 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p25.geometryN( 1 ) )->pointN( 8 ), QgsPoint( 0, 0 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p25.geometryN( 1 ) )->pointN( 9 ), QgsPoint( 0.1, 0.1 ) );

  //move vertex

  //empty collection
  QgsGeometryCollection p26;
  QVERIFY( !p26.moveVertex( QgsVertexId( 0, 0, 0 ), QgsPoint( 6.0, 7.0 ) ) );
  QVERIFY( !p26.moveVertex( QgsVertexId( -1, 0, 0 ), QgsPoint( 6.0, 7.0 ) ) );
  QVERIFY( !p26.moveVertex( QgsVertexId( 1, 0, 0 ), QgsPoint( 6.0, 7.0 ) ) );
  QVERIFY( p26.isEmpty() );

  //valid collection
  l38.setPoints( QgsPointSequence() << QgsPoint( 1, 2 )
                 << QgsPoint( 11, 12 ) << QgsPoint( 21, 22 ) << QgsPoint( 1, 2 ) );
  p26.addGeometry( l38.clone() );
  QVERIFY( p26.moveVertex( QgsVertexId( 0, 0, 0 ), QgsPoint( 6.0, 7.0 ) ) );
  QVERIFY( p26.moveVertex( QgsVertexId( 0, 0, 1 ), QgsPoint( 16.0, 17.0 ) ) );
  QVERIFY( p26.moveVertex( QgsVertexId( 0, 0, 2 ), QgsPoint( 26.0, 27.0 ) ) );
  QCOMPARE( static_cast< const QgsLineString * >( p26.geometryN( 0 ) )->pointN( 0 ), QgsPoint( 6.0, 7.0 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p26.geometryN( 0 ) )->pointN( 1 ), QgsPoint( 16.0, 17.0 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p26.geometryN( 0 ) )->pointN( 2 ), QgsPoint( 26.0, 27.0 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p26.geometryN( 0 ) )->pointN( 3 ), QgsPoint( 1, 2 ) );

  //out of range
  QVERIFY( !p26.moveVertex( QgsVertexId( 0, 0, -1 ), QgsPoint( 3.0, 4.0 ) ) );
  QVERIFY( !p26.moveVertex( QgsVertexId( 0, 0, 10 ), QgsPoint( 3.0, 4.0 ) ) );
  QVERIFY( !p26.moveVertex( QgsVertexId( 1, 0, 0 ), QgsPoint( 3.0, 4.0 ) ) );
  QCOMPARE( static_cast< const QgsLineString * >( p26.geometryN( 0 ) )->pointN( 0 ), QgsPoint( 6.0, 7.0 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p26.geometryN( 0 ) )->pointN( 1 ), QgsPoint( 16.0, 17.0 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p26.geometryN( 0 ) )->pointN( 2 ), QgsPoint( 26.0, 27.0 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p26.geometryN( 0 ) )->pointN( 3 ), QgsPoint( 1.0, 2.0 ) );

  // with second part
  p26.addGeometry( l38.clone() );
  QVERIFY( p26.moveVertex( QgsVertexId( 1, 0, 0 ), QgsPoint( 6.0, 7.0 ) ) );
  QVERIFY( p26.moveVertex( QgsVertexId( 1, 0, 1 ), QgsPoint( 16.0, 17.0 ) ) );
  QVERIFY( p26.moveVertex( QgsVertexId( 1, 0, 2 ), QgsPoint( 26.0, 27.0 ) ) );
  QCOMPARE( static_cast< const QgsLineString * >( p26.geometryN( 1 ) )->pointN( 0 ), QgsPoint( 6.0, 7.0 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p26.geometryN( 1 ) )->pointN( 1 ), QgsPoint( 16.0, 17.0 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p26.geometryN( 1 ) )->pointN( 2 ), QgsPoint( 26.0, 27.0 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p26.geometryN( 1 ) )->pointN( 3 ), QgsPoint( 1.0, 2.0 ) );
  QVERIFY( !p26.moveVertex( QgsVertexId( 1, 0, -1 ), QgsPoint( 3.0, 4.0 ) ) );
  QVERIFY( !p26.moveVertex( QgsVertexId( 1, 0, 10 ), QgsPoint( 3.0, 4.0 ) ) );
  QVERIFY( !p26.moveVertex( QgsVertexId( 2, 0, 0 ), QgsPoint( 3.0, 4.0 ) ) );

  //delete vertex

  //empty collection
  QgsGeometryCollection p27;
  QVERIFY( !p27.deleteVertex( QgsVertexId( 0, 0, 0 ) ) );
  QVERIFY( !p27.deleteVertex( QgsVertexId( 0, 1, 0 ) ) );
  QVERIFY( !p27.deleteVertex( QgsVertexId( 1, 1, 0 ) ) );
  QVERIFY( !p27.deleteVertex( QgsVertexId( -1, 1, 0 ) ) );
  QVERIFY( p27.isEmpty() );

  //valid collection
  l38.setPoints( QgsPointSequence() << QgsPoint( 1, 2 ) << QgsPoint( 5, 2 ) << QgsPoint( 6, 2 ) << QgsPoint( 7, 2 )
                 << QgsPoint( 11, 12 ) << QgsPoint( 21, 22 ) << QgsPoint( 1, 2 ) );

  p27.addGeometry( l38.clone() );
  //out of range vertices
  QVERIFY( !p27.deleteVertex( QgsVertexId( 0, 0, -1 ) ) );
  QVERIFY( !p27.deleteVertex( QgsVertexId( 0, 0, 100 ) ) );
  QVERIFY( !p27.deleteVertex( QgsVertexId( 1, 0, 1 ) ) );

  //valid vertices
  QVERIFY( p27.deleteVertex( QgsVertexId( 0, 0, 1 ) ) );
  QCOMPARE( static_cast< const QgsLineString * >( p27.geometryN( 0 ) )->pointN( 0 ), QgsPoint( 1.0, 2.0 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p27.geometryN( 0 ) )->pointN( 1 ), QgsPoint( 6.0, 2.0 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p27.geometryN( 0 ) )->pointN( 2 ), QgsPoint( 7.0, 2.0 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p27.geometryN( 0 ) )->pointN( 3 ), QgsPoint( 11.0, 12.0 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p27.geometryN( 0 ) )->pointN( 5 ), QgsPoint( 1.0, 2.0 ) );

  // delete first vertex
  QVERIFY( p27.deleteVertex( QgsVertexId( 0, 0, 0 ) ) );
  QCOMPARE( static_cast< const QgsLineString * >( p27.geometryN( 0 ) )->pointN( 0 ), QgsPoint( 6.0, 2.0 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p27.geometryN( 0 ) )->pointN( 1 ), QgsPoint( 7.0, 2.0 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p27.geometryN( 0 ) )->pointN( 2 ), QgsPoint( 11.0, 12.0 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p27.geometryN( 0 ) )->pointN( 3 ), QgsPoint( 21.0, 22.0 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p27.geometryN( 0 ) )->pointN( 4 ), QgsPoint( 1.0, 2.0 ) );

  // delete last vertex
  QVERIFY( p27.deleteVertex( QgsVertexId( 0, 0, 4 ) ) );
  QCOMPARE( static_cast< const QgsLineString * >( p27.geometryN( 0 ) )->pointN( 0 ), QgsPoint( 6.0, 2.0 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p27.geometryN( 0 ) )->pointN( 1 ), QgsPoint( 7.0, 2.0 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p27.geometryN( 0 ) )->pointN( 2 ), QgsPoint( 11.0, 12.0 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p27.geometryN( 0 ) )->pointN( 3 ), QgsPoint( 21.0, 22.0 ) );

  // delete some more vertices - should remove part
  QVERIFY( p27.deleteVertex( QgsVertexId( 0, 0, 0 ) ) );
  QVERIFY( p27.deleteVertex( QgsVertexId( 0, 0, 0 ) ) );
  QVERIFY( p27.deleteVertex( QgsVertexId( 0, 0, 0 ) ) );
  QVERIFY( !p27.geometryN( 0 ) );

  // with two parts
  p27.addGeometry( l38.clone() );
  p27.addGeometry( l38.clone() );

  //out of range vertices
  QVERIFY( !p27.deleteVertex( QgsVertexId( 1, 0, -1 ) ) );
  QVERIFY( !p27.deleteVertex( QgsVertexId( 1, 0, 100 ) ) );
  QVERIFY( !p27.deleteVertex( QgsVertexId( 2, 0, 1 ) ) );

  //valid vertices
  QVERIFY( p27.deleteVertex( QgsVertexId( 1, 0, 1 ) ) );
  QCOMPARE( static_cast< const QgsLineString * >( p27.geometryN( 1 ) )->pointN( 0 ), QgsPoint( 1.0, 2.0 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p27.geometryN( 1 ) )->pointN( 1 ), QgsPoint( 6.0, 2.0 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p27.geometryN( 1 ) )->pointN( 2 ), QgsPoint( 7.0, 2.0 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p27.geometryN( 1 ) )->pointN( 3 ), QgsPoint( 11.0, 12.0 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p27.geometryN( 1 ) )->pointN( 5 ), QgsPoint( 1.0, 2.0 ) );

  // delete first vertex
  QVERIFY( p27.deleteVertex( QgsVertexId( 1, 0, 0 ) ) );
  QCOMPARE( static_cast< const QgsLineString * >( p27.geometryN( 1 ) )->pointN( 0 ), QgsPoint( 6.0, 2.0 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p27.geometryN( 1 ) )->pointN( 1 ), QgsPoint( 7.0, 2.0 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p27.geometryN( 1 ) )->pointN( 2 ), QgsPoint( 11.0, 12.0 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p27.geometryN( 1 ) )->pointN( 3 ), QgsPoint( 21.0, 22.0 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p27.geometryN( 1 ) )->pointN( 4 ), QgsPoint( 1.0, 2.0 ) );

  // delete last vertex
  QVERIFY( p27.deleteVertex( QgsVertexId( 1, 0, 4 ) ) );
  QCOMPARE( static_cast< const QgsLineString * >( p27.geometryN( 1 ) )->pointN( 0 ), QgsPoint( 6.0, 2.0 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p27.geometryN( 1 ) )->pointN( 1 ), QgsPoint( 7.0, 2.0 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p27.geometryN( 1 ) )->pointN( 2 ), QgsPoint( 11.0, 12.0 ) );
  QCOMPARE( static_cast< const QgsLineString * >( p27.geometryN( 1 ) )->pointN( 3 ), QgsPoint( 21.0, 22.0 ) );

  // delete some more vertices - should remove part
  QVERIFY( p27.deleteVertex( QgsVertexId( 1, 0, 1 ) ) );
  QVERIFY( p27.deleteVertex( QgsVertexId( 1, 0, 1 ) ) );
  QVERIFY( p27.deleteVertex( QgsVertexId( 1, 0, 1 ) ) );
  QCOMPARE( p27.numGeometries(), 1 );
  QVERIFY( p27.geometryN( 0 ) );

  // test that second geometry is "promoted" when first is removed
  p27.addGeometry( l38.clone() );
  QVERIFY( p27.deleteVertex( QgsVertexId( 0, 0, 0 ) ) );
  QCOMPARE( p27.numGeometries(), 2 );
  QVERIFY( p27.deleteVertex( QgsVertexId( 0, 0, 0 ) ) );
  QCOMPARE( p27.numGeometries(), 2 );
  QVERIFY( p27.deleteVertex( QgsVertexId( 0, 0, 0 ) ) );
  QCOMPARE( p27.numGeometries(), 2 );
  QVERIFY( p27.deleteVertex( QgsVertexId( 0, 0, 0 ) ) );
  QVERIFY( p27.deleteVertex( QgsVertexId( 0, 0, 0 ) ) );
  QVERIFY( p27.deleteVertex( QgsVertexId( 0, 0, 0 ) ) );
  QCOMPARE( p27.numGeometries(), 1 );
  QVERIFY( p27.geometryN( 0 ) );

  //boundary

  // collections have no boundary defined
  QgsGeometryCollection boundaryCollection;
  QVERIFY( !boundaryCollection.boundary() );
  // add a geometry and retest, should still be undefined
  QgsLineString *lineBoundary = new QgsLineString();
  lineBoundary->setPoints( QVector<QgsPoint>() << QgsPoint( 0, 0 ) << QgsPoint( 1, 0 ) );
  boundaryCollection.addGeometry( lineBoundary );
  QVERIFY( !boundaryCollection.boundary() );

  // segmentize
  QgsGeometryCollection segmentC;
  QgsCircularString toSegment;
  toSegment.setPoints( QgsPointSequence() << QgsPoint( 1, 2 )
                       << QgsPoint( 11, 10 ) << QgsPoint( 21, 2 ) );
  segmentC.addGeometry( toSegment.clone() );
  std::unique_ptr<QgsGeometryCollection> segmentized( static_cast< QgsGeometryCollection * >( segmentC.segmentize() ) );
  const QgsLineString *segmentizedLine = static_cast< const QgsLineString * >( segmentized->geometryN( 0 ) );
  QCOMPARE( segmentizedLine->numPoints(), 156 );
  QCOMPARE( segmentizedLine->vertexCount(), 156 );
  QCOMPARE( segmentizedLine->ringCount(), 1 );
  QCOMPARE( segmentizedLine->partCount(), 1 );
  QCOMPARE( segmentizedLine->wkbType(), QgsWkbTypes::LineString );
  QVERIFY( !segmentizedLine->is3D() );
  QVERIFY( !segmentizedLine->isMeasure() );
  QCOMPARE( segmentizedLine->pointN( 0 ), toSegment.pointN( 0 ) );
  QCOMPARE( segmentizedLine->pointN( segmentizedLine->numPoints() - 1 ), toSegment.pointN( toSegment.numPoints() - 1 ) );

  // hasCurvedSegments
  QgsGeometryCollection c30;
  QVERIFY( !c30.hasCurvedSegments() );
  c30.addGeometry( part.clone() );
  QVERIFY( !c30.hasCurvedSegments() );
  c30.addGeometry( toSegment.clone() );
  QVERIFY( c30.hasCurvedSegments() );


  //adjacent vertices
  QgsGeometryCollection c31;
  QgsLineString vertexLine1;
  QgsVertexId prev( 1, 2, 3 ); // start with something
  QgsVertexId next( 4, 5, 6 );
  c31.adjacentVertices( QgsVertexId( 0, 0, -1 ), prev, next );
  QCOMPARE( prev, QgsVertexId() );
  QCOMPARE( next, QgsVertexId() );
  c31.adjacentVertices( QgsVertexId( -1, 0, -1 ), prev, next );
  QCOMPARE( prev, QgsVertexId() );
  QCOMPARE( next, QgsVertexId() );
  c31.adjacentVertices( QgsVertexId( 10, 0, -1 ), prev, next );
  QCOMPARE( prev, QgsVertexId() );
  QCOMPARE( next, QgsVertexId() );
  vertexLine1.setPoints( QgsPointSequence() << QgsPoint( 1, 2 ) << QgsPoint( 11, 12 ) << QgsPoint( 111, 112 ) );
  c31.addGeometry( vertexLine1.clone() );
  c31.addGeometry( vertexLine1.clone() );
  c31.adjacentVertices( QgsVertexId( -1, 0, -1 ), prev, next );
  QCOMPARE( prev, QgsVertexId() );
  QCOMPARE( next, QgsVertexId() );
  c31.adjacentVertices( QgsVertexId( 10, 0, -1 ), prev, next );
  QCOMPARE( prev, QgsVertexId() );
  QCOMPARE( next, QgsVertexId() );
  c31.adjacentVertices( QgsVertexId( 0, 0, 0 ), prev, next );
  QCOMPARE( prev, QgsVertexId() );
  QCOMPARE( next, QgsVertexId( 0, 0, 1 ) );
  c31.adjacentVertices( QgsVertexId( 0, 0, 1 ), prev, next );
  QCOMPARE( prev, QgsVertexId( 0, 0, 0 ) );
  QCOMPARE( next, QgsVertexId( 0, 0, 2 ) );
  c31.adjacentVertices( QgsVertexId( 1, 0, 1 ), prev, next );
  QCOMPARE( prev, QgsVertexId( 1, 0, 0 ) );
  QCOMPARE( next, QgsVertexId( 1, 0, 2 ) );


  // vertex number
  QgsGeometryCollection c32;
  QgsLineString vertexLine2;
  vertexLine2.setPoints( QgsPointSequence() << QgsPoint( 1, 2 ) << QgsPoint( 11, 12 ) << QgsPoint( 111, 112 ) );
  QCOMPARE( c32.vertexNumberFromVertexId( QgsVertexId( -1, 0, 0 ) ), -1 );
  QCOMPARE( c32.vertexNumberFromVertexId( QgsVertexId( 1, 0, 0 ) ), -1 );
  QCOMPARE( c32.vertexNumberFromVertexId( QgsVertexId( 0, -1, 0 ) ), -1 );
  QCOMPARE( c32.vertexNumberFromVertexId( QgsVertexId( 0, 1, 0 ) ), -1 );
  QCOMPARE( c32.vertexNumberFromVertexId( QgsVertexId( 0, 0, -1 ) ), -1 );
  QCOMPARE( c32.vertexNumberFromVertexId( QgsVertexId( 0, 0, 0 ) ), -1 );
  QCOMPARE( c32.vertexNumberFromVertexId( QgsVertexId( 0, 0, 1 ) ), -1 );
  c32.addGeometry( vertexLine2.clone() );
  QCOMPARE( c32.vertexNumberFromVertexId( QgsVertexId( -1, 0, 0 ) ), -1 );
  QCOMPARE( c32.vertexNumberFromVertexId( QgsVertexId( 1, 0, 0 ) ), -1 );
  QCOMPARE( c32.vertexNumberFromVertexId( QgsVertexId( 0, -1, 0 ) ), -1 );
  QCOMPARE( c32.vertexNumberFromVertexId( QgsVertexId( 0, 1, 0 ) ), -1 );
  QCOMPARE( c32.vertexNumberFromVertexId( QgsVertexId( 0, 0, -1 ) ), -1 );
  QCOMPARE( c32.vertexNumberFromVertexId( QgsVertexId( 0, 0, 0 ) ), 0 );
  QCOMPARE( c32.vertexNumberFromVertexId( QgsVertexId( 0, 0, 1 ) ), 1 );
  QCOMPARE( c32.vertexNumberFromVertexId( QgsVertexId( 0, 0, 2 ) ), 2 );
  QCOMPARE( c32.vertexNumberFromVertexId( QgsVertexId( 0, 0, 3 ) ), -1 );
  c32.addGeometry( vertexLine2.clone() );
  QCOMPARE( c32.vertexNumberFromVertexId( QgsVertexId( 0, 0, 0 ) ), 0 );
  QCOMPARE( c32.vertexNumberFromVertexId( QgsVertexId( 0, 0, 1 ) ), 1 );
  QCOMPARE( c32.vertexNumberFromVertexId( QgsVertexId( 0, 0, 2 ) ), 2 );
  QCOMPARE( c32.vertexNumberFromVertexId( QgsVertexId( 0, 0, 3 ) ), -1 );
  QCOMPARE( c32.vertexNumberFromVertexId( QgsVertexId( 1, 0, 0 ) ), 3 );
  QCOMPARE( c32.vertexNumberFromVertexId( QgsVertexId( 1, 0, 1 ) ), 4 );
  QCOMPARE( c32.vertexNumberFromVertexId( QgsVertexId( 1, 0, 2 ) ), 5 );
  QCOMPARE( c32.vertexNumberFromVertexId( QgsVertexId( 1, 0, 3 ) ), -1 );
  QgsPolygon polyPart;
  vertexLine2.close();
  polyPart.setExteriorRing( vertexLine2.clone() );
  c32.addGeometry( polyPart.clone() );
  QCOMPARE( c32.vertexNumberFromVertexId( QgsVertexId( 0, 0, 0 ) ), 0 );
  QCOMPARE( c32.vertexNumberFromVertexId( QgsVertexId( 0, 0, 1 ) ), 1 );
  QCOMPARE( c32.vertexNumberFromVertexId( QgsVertexId( 0, 0, 2 ) ), 2 );
  QCOMPARE( c32.vertexNumberFromVertexId( QgsVertexId( 0, 0, 3 ) ), -1 );
  QCOMPARE( c32.vertexNumberFromVertexId( QgsVertexId( 1, 0, 0 ) ), 3 );
  QCOMPARE( c32.vertexNumberFromVertexId( QgsVertexId( 1, 0, 1 ) ), 4 );
  QCOMPARE( c32.vertexNumberFromVertexId( QgsVertexId( 1, 0, 2 ) ), 5 );
  QCOMPARE( c32.vertexNumberFromVertexId( QgsVertexId( 1, 0, 3 ) ), -1 );
  QCOMPARE( c32.vertexNumberFromVertexId( QgsVertexId( 2, -1, 0 ) ), -1 );
  QCOMPARE( c32.vertexNumberFromVertexId( QgsVertexId( 2, 1, 0 ) ), -1 );
  QCOMPARE( c32.vertexNumberFromVertexId( QgsVertexId( 2, 0, -1 ) ), -1 );
  QCOMPARE( c32.vertexNumberFromVertexId( QgsVertexId( 2, 0, 0 ) ), 6 );
  QCOMPARE( c32.vertexNumberFromVertexId( QgsVertexId( 2, 0, 1 ) ), 7 );
  QCOMPARE( c32.vertexNumberFromVertexId( QgsVertexId( 2, 0, 2 ) ), 8 );
  QCOMPARE( c32.vertexNumberFromVertexId( QgsVertexId( 2, 0, 3 ) ), 9 );
  QCOMPARE( c32.vertexNumberFromVertexId( QgsVertexId( 2, 0, 4 ) ), -1 );
  polyPart.addInteriorRing( vertexLine2.clone() );
  c32.addGeometry( polyPart.clone() );
  QCOMPARE( c32.vertexNumberFromVertexId( QgsVertexId( 2, 0, 0 ) ), 6 );
  QCOMPARE( c32.vertexNumberFromVertexId( QgsVertexId( 2, 0, 1 ) ), 7 );
  QCOMPARE( c32.vertexNumberFromVertexId( QgsVertexId( 2, 0, 2 ) ), 8 );
  QCOMPARE( c32.vertexNumberFromVertexId( QgsVertexId( 2, 0, 3 ) ), 9 );
  QCOMPARE( c32.vertexNumberFromVertexId( QgsVertexId( 2, 0, 4 ) ), -1 );
  QCOMPARE( c32.vertexNumberFromVertexId( QgsVertexId( 3, -1, 0 ) ), -1 );
  QCOMPARE( c32.vertexNumberFromVertexId( QgsVertexId( 3, 2, 0 ) ), -1 );
  QCOMPARE( c32.vertexNumberFromVertexId( QgsVertexId( 3, 0, 0 ) ), 10 );
  QCOMPARE( c32.vertexNumberFromVertexId( QgsVertexId( 3, 0, 1 ) ), 11 );
  QCOMPARE( c32.vertexNumberFromVertexId( QgsVertexId( 3, 0, 2 ) ), 12 );
  QCOMPARE( c32.vertexNumberFromVertexId( QgsVertexId( 3, 0, 3 ) ), 13 );
  QCOMPARE( c32.vertexNumberFromVertexId( QgsVertexId( 3, 0, 4 ) ), -1 );
  QCOMPARE( c32.vertexNumberFromVertexId( QgsVertexId( 3, 1, 0 ) ), 14 );
  QCOMPARE( c32.vertexNumberFromVertexId( QgsVertexId( 3, 1, 1 ) ), 15 );
  QCOMPARE( c32.vertexNumberFromVertexId( QgsVertexId( 3, 1, 2 ) ), 16 );
  QCOMPARE( c32.vertexNumberFromVertexId( QgsVertexId( 3, 1, 3 ) ), 17 );
  QCOMPARE( c32.vertexNumberFromVertexId( QgsVertexId( 3, 1, 4 ) ), -1 );
  QCOMPARE( c32.vertexNumberFromVertexId( QgsVertexId( 3, 2, 0 ) ), -1 );


  //removeDuplicateNodes
  QgsGeometryCollection gcNodes;
  QgsLineString nodeLine;
  QVERIFY( !gcNodes.removeDuplicateNodes() );
  nodeLine.setPoints( QgsPointSequence() << QgsPoint( 11, 2 ) << QgsPoint( 11, 12 ) << QgsPoint( 111, 12 ) );
  gcNodes.addGeometry( nodeLine.clone() );
  QVERIFY( !gcNodes.removeDuplicateNodes( 0.02 ) );
  QCOMPARE( gcNodes.asWkt(), QStringLiteral( "GeometryCollection (LineString (11 2, 11 12, 111 12))" ) );
  nodeLine.setPoints( QgsPointSequence() << QgsPoint( 11, 2 ) << QgsPoint( 11.01, 1.99 ) << QgsPoint( 11.02, 2.01 )
                      << QgsPoint( 11, 12 ) << QgsPoint( 111, 12 ) << QgsPoint( 111.01, 11.99 ) );
  gcNodes.addGeometry( nodeLine.clone() );
  QVERIFY( gcNodes.removeDuplicateNodes( 0.02 ) );
  QCOMPARE( gcNodes.asWkt( 2 ), QStringLiteral( "GeometryCollection (LineString (11 2, 11 12, 111 12),LineString (11 2, 11 12, 111 12))" ) );

  //swapXy
  QgsGeometryCollection swapCollect;
  QgsLineString swapLine;
  swapCollect.swapXy(); // no crash
  swapLine.setPoints( QgsPointSequence() << QgsPoint( 11, 2, 3, 4, QgsWkbTypes::PointZM ) << QgsPoint( 11, 12, 13, 14, QgsWkbTypes::PointZM ) << QgsPoint( 111, 12, 23, 24, QgsWkbTypes::PointZM ) );
  swapCollect.addGeometry( swapLine.clone() );
  swapCollect.swapXy();
  QCOMPARE( swapCollect.asWkt(), QStringLiteral( "GeometryCollection (LineStringZM (2 11 3 4, 12 11 13 14, 12 111 23 24))" ) );
  swapLine.setPoints( QgsPointSequence() << QgsPoint( 11, 2, 5, 6, QgsWkbTypes::PointZM ) << QgsPoint( 11.01, 1.99, 15, 16, QgsWkbTypes::PointZM ) << QgsPoint( 11.02, 2.01, 25, 26, QgsWkbTypes::PointZM ) );
  swapCollect.addGeometry( swapLine.clone() );
  swapCollect.swapXy();
  QCOMPARE( swapCollect.asWkt( 2 ), QStringLiteral( "GeometryCollection (LineStringZM (11 2 3 4, 11 12 13 14, 111 12 23 24),LineStringZM (2 11 5 6, 1.99 11.01 15 16, 2.01 11.02 25 26))" ) );

  // filter vertices
  QgsGeometryCollection filterCollect;
  auto filter = []( const QgsPoint & point )-> bool
  {
    return point.x() > 5;
  };
  QgsLineString filterLine;
  filterCollect.filterVertices( filter ); // no crash
  filterLine.setPoints( QgsPointSequence() << QgsPoint( 1, 2, 3, 4, QgsWkbTypes::PointZM ) << QgsPoint( 11, 12, 13, 14, QgsWkbTypes::PointZM ) << QgsPoint( 111, 12, 23, 24, QgsWkbTypes::PointZM ) );
  filterCollect.addGeometry( filterLine.clone() );
  filterCollect.filterVertices( filter );
  QCOMPARE( filterCollect.asWkt(), QStringLiteral( "GeometryCollection (LineStringZM (11 12 13 14, 111 12 23 24))" ) );
  filterLine.setPoints( QgsPointSequence() << QgsPoint( 11, 2, 5, 6, QgsWkbTypes::PointZM ) << QgsPoint( 1.01, 1.99, 15, 16, QgsWkbTypes::PointZM ) << QgsPoint( 11.02, 2.01, 25, 26, QgsWkbTypes::PointZM ) );
  filterCollect.addGeometry( filterLine.clone() );
  filterCollect.filterVertices( filter );
  QCOMPARE( filterCollect.asWkt( 2 ), QStringLiteral( "GeometryCollection (LineStringZM (11 12 13 14, 111 12 23 24),LineStringZM (11 2 5 6, 11.02 2.01 25 26))" ) );

  // transform vertices
  QgsGeometryCollection transformCollect;
  auto transform = []( const QgsPoint & point )-> QgsPoint
  {
    return QgsPoint( point.x() + 2, point.y() + 3, point.z() + 4, point.m() + 5 );
  };
  QgsLineString transformLine;
  transformCollect.transformVertices( transform ); // no crash
  transformLine.setPoints( QgsPointSequence() << QgsPoint( 1, 2, 3, 4, QgsWkbTypes::PointZM ) << QgsPoint( 11, 12, 13, 14, QgsWkbTypes::PointZM ) << QgsPoint( 111, 12, 23, 24, QgsWkbTypes::PointZM ) );
  transformCollect.addGeometry( transformLine.clone() );
  transformCollect.transformVertices( transform );
  QCOMPARE( transformCollect.asWkt(), QStringLiteral( "GeometryCollection (LineStringZM (3 5 7 9, 13 15 17 19, 113 15 27 29))" ) );
  transformLine.setPoints( QgsPointSequence() << QgsPoint( 11, 2, 5, 6, QgsWkbTypes::PointZM ) << QgsPoint( 1.01, 1.99, 15, 16, QgsWkbTypes::PointZM ) << QgsPoint( 11.02, 2.01, 25, 26, QgsWkbTypes::PointZM ) );
  transformCollect.addGeometry( transformLine.clone() );
  transformCollect.transformVertices( transform );
  QCOMPARE( transformCollect.asWkt( 2 ), QStringLiteral( "GeometryCollection (LineStringZM (5 8 11 14, 15 18 21 24, 115 18 31 34),LineStringZM (13 5 9 11, 3.01 4.99 19 21, 13.02 5.01 29 31))" ) );

}

void TestQgsGeometry::fromQgsPointXY()
{
  QgsPointXY point( 1.0, 2.0 );
  QgsGeometry result( QgsGeometry::fromPointXY( point ) );
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
  QgsPolylineXY resultLine = result.asPolyline();
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
  QgsPolygonXY resultPolygon = result2.asPolygon();
  QCOMPARE( resultPolygon.size(), 1 );
  QCOMPARE( resultPolygon.at( 0 ).at( 0 ), QgsPointXY( 1.0, 2.0 ) );
  QCOMPARE( resultPolygon.at( 0 ).at( 1 ), QgsPointXY( 4.0, 6.0 ) );
  QCOMPARE( resultPolygon.at( 0 ).at( 2 ), QgsPointXY( 4.0, 3.0 ) );
  QCOMPARE( resultPolygon.at( 0 ).at( 3 ), QgsPointXY( 2.0, 2.0 ) );
  QCOMPARE( resultPolygon.at( 0 ).at( 4 ), QgsPointXY( 1.0, 2.0 ) );
}

void TestQgsGeometry::fromPolyline()
{
  QgsPolyline polyline;
  QgsGeometry fromPolyline = QgsGeometry::fromPolyline( polyline );
  QVERIFY( fromPolyline.isEmpty() );
  QCOMPARE( fromPolyline.wkbType(), QgsWkbTypes::LineString );
  polyline << QgsPoint( 10, 20 ) << QgsPoint( 30, 40 );
  fromPolyline = QgsGeometry::fromPolyline( polyline );
  QCOMPARE( fromPolyline.asWkt(), QStringLiteral( "LineString (10 20, 30 40)" ) );
  QgsPolyline polyline3d;
  polyline3d << QgsPoint( QgsWkbTypes::PointZ, 10, 20, 100 ) << QgsPoint( QgsWkbTypes::PointZ, 30, 40, 200 );
  fromPolyline = QgsGeometry::fromPolyline( polyline3d );
  QCOMPARE( fromPolyline.asWkt(), QStringLiteral( "LineStringZ (10 20 100, 30 40 200)" ) );
  QgsPolyline polylineM;
  polylineM << QgsPoint( QgsWkbTypes::PointM, 10, 20, 0, 100 ) << QgsPoint( QgsWkbTypes::PointM, 30, 40, 0, 200 );
  fromPolyline = QgsGeometry::fromPolyline( polylineM );
  QCOMPARE( fromPolyline.asWkt(), QStringLiteral( "LineStringM (10 20 100, 30 40 200)" ) );
  QgsPolyline polylineZM;
  polylineZM << QgsPoint( QgsWkbTypes::PointZM, 10, 20, 4, 100 ) << QgsPoint( QgsWkbTypes::PointZM, 30, 40, 5, 200 );
  fromPolyline = QgsGeometry::fromPolyline( polylineZM );
  QCOMPARE( fromPolyline.asWkt(), QStringLiteral( "LineStringZM (10 20 4 100, 30 40 5 200)" ) );
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
  QgsPolylineXY testline;
  testline << mPoint1 << mPoint2 << mPoint3;
  QgsGeometry lineGeom( QgsGeometry::fromPolylineXY( testline ) );
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
  QgsGeometry badGeom( QgsGeometry::fromPointXY( mPoint1 ) );
  QPolygonF fromBad = badGeom.asQPolygonF();
  QVERIFY( fromBad.isEmpty() );
}

void TestQgsGeometry::comparePolylines()
{
  QgsPolylineXY line1;
  line1 << mPoint1 << mPoint2 << mPoint3;
  QgsPolylineXY line2;
  line2 << mPoint1 << mPoint2 << mPoint3;
  QVERIFY( QgsGeometry::compare( line1, line2 ) );

  //different number of nodes
  QgsPolylineXY line3;
  line3 << mPoint1 << mPoint2 << mPoint3 << mPoint4;
  QVERIFY( !QgsGeometry::compare( line1, line3 ) );

  //different nodes
  QgsPolylineXY line4;
  line3 << mPoint1 << mPointA << mPoint3 << mPoint4;
  QVERIFY( !QgsGeometry::compare( line3, line4 ) );
}

void TestQgsGeometry::comparePolygons()
{
  QgsPolylineXY ring1;
  ring1 << mPoint1 << mPoint2 << mPoint3 << mPoint1;
  QgsPolylineXY ring2;
  ring2 << mPoint4 << mPointA << mPointB << mPoint4;
  QgsPolygonXY poly1;
  poly1 << ring1 << ring2;
  QgsPolygonXY poly2;
  poly2 << ring1 << ring2;
  QVERIFY( QgsGeometry::compare( poly1, poly2 ) );

  //different number of rings
  QgsPolygonXY poly3;
  poly3 << ring1;
  QVERIFY( !QgsGeometry::compare( poly1, poly3 ) );

  //different rings
  QgsPolygonXY poly4;
  poly4 << ring2;
  QVERIFY( !QgsGeometry::compare( poly3, poly4 ) );
}


// Helper function (in anonymous namespace to prevent possible link with the extirior)
namespace
{
  template<typename T>
  inline void testCreateEmptyWithSameType( bool canBeEmpty = true )
  {
    std::unique_ptr<QgsAbstractGeometry> geom { new T() };
    std::unique_ptr<QgsAbstractGeometry> created { TestQgsGeometry::createEmpty( geom.get() ) };
    if ( canBeEmpty )
    {
      QVERIFY( created->isEmpty() );
    }
    // Check that it is the correct type
    QVERIFY( static_cast<T *>( created.get() ) != nullptr );
  }
}

void TestQgsGeometry::createEmptyWithSameType()
{
  qDebug( "createEmptyWithSameType(): QgsCircularString" );
  testCreateEmptyWithSameType<QgsCircularString>();

  qDebug( "createEmptyWithSameType(): QgsCompoundCurve" );
  testCreateEmptyWithSameType<QgsCompoundCurve>();

  qDebug( "createEmptyWithSameType(): QgsLineString" );
  testCreateEmptyWithSameType<QgsLineString>();


  qDebug( "createEmptyWithSameType(): QgsGeometryCollection" );
  testCreateEmptyWithSameType<QgsGeometryCollection>();

  qDebug( "createEmptyWithSameType(): QgsMultiCurve" );
  testCreateEmptyWithSameType<QgsMultiCurve>();

  qDebug( "createEmptyWithSameType(): QgsMultiLineString" );
  testCreateEmptyWithSameType<QgsMultiLineString>();

  qDebug( "createEmptyWithSameType(): QgsMultiPointV2" );
  testCreateEmptyWithSameType<QgsMultiPoint>();

  qDebug( "createEmptyWithSameType(): QgsMultiSurface" );
  testCreateEmptyWithSameType<QgsMultiSurface>();


  qDebug( "createEmptyWithSameType(): QgsPoint" );
  testCreateEmptyWithSameType<QgsPoint>( false );


  qDebug( "createEmptyWithSameType(): QgsCurvePolygon" );
  testCreateEmptyWithSameType<QgsCurvePolygon>();

  qDebug( "createEmptyWithSameType(): QgsPolygonV2" );
  testCreateEmptyWithSameType<QgsPolygon>();

  qDebug( "createEmptyWithSameType(): QgsTriangle" );
  testCreateEmptyWithSameType<QgsTriangle>();

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

  std::unique_ptr< QgsGeometryEngine > engine( QgsGeometry::createGeometryEngine( mpPolygonGeometryA.constGet() ) );
  QVERIFY( engine->intersects( mpPolygonGeometryB.constGet() ) );
  engine->prepareGeometry();
  QVERIFY( engine->intersects( mpPolygonGeometryB.constGet() ) );

  // should be a single polygon as A intersect B
  QgsGeometry mypIntersectionGeometry  =  mpPolygonGeometryA.intersection( mpPolygonGeometryB );
  qDebug() << "Geometry Type: " << QgsWkbTypes::displayString( mypIntersectionGeometry.wkbType() );
  QVERIFY( mypIntersectionGeometry.wkbType() == QgsWkbTypes::Polygon );
  QgsPolygonXY myPolygon = mypIntersectionGeometry.asPolygon();
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
  QString obtained = geom.asWkt();
  QString expected = QStringLiteral( "LineString (10 -5, 20 -5, 20 5)" );
  QCOMPARE( obtained, expected );
  geom.translate( -10, 5 );
  obtained = geom.asWkt();
  QCOMPARE( obtained, wkt );

  wkt = QStringLiteral( "Polygon ((-2 4, -2 -10, 2 3, -2 4),(1 1, -1 1, -1 -1, 1 1))" );
  geom = QgsGeometry::fromWkt( wkt );
  geom.translate( -2, 10 );
  obtained = geom.asWkt();
  expected = QStringLiteral( "Polygon ((-4 14, -4 0, 0 13, -4 14),(-1 11, -3 11, -3 9, -1 11))" );
  QCOMPARE( obtained, expected );
  geom.translate( 2, -10 );
  obtained = geom.asWkt();
  QCOMPARE( obtained, wkt );

  wkt = QStringLiteral( "Point (40 50)" );
  geom = QgsGeometry::fromWkt( wkt );
  geom.translate( -2, 10 );
  obtained = geom.asWkt();
  expected = QStringLiteral( "Point (38 60)" );
  QCOMPARE( obtained, expected );
  geom.translate( 2, -10 );
  obtained = geom.asWkt();
  QCOMPARE( obtained, wkt );

}

void TestQgsGeometry::rotateCheck1()
{
  QString wkt = QStringLiteral( "LineString (0 0, 10 0, 10 10)" );
  QgsGeometry geom( QgsGeometry::fromWkt( wkt ) );
  geom.rotate( 90, QgsPointXY( 0, 0 ) );
  QString obtained = geom.asWkt();
  QString expected = QStringLiteral( "LineString (0 0, 0 -10, 10 -10)" );
  QCOMPARE( obtained, expected );
  geom.rotate( -90, QgsPointXY( 0, 0 ) );
  obtained = geom.asWkt();
  QCOMPARE( obtained, wkt );

  wkt = QStringLiteral( "Polygon ((-2 4, -2 -10, 2 3, -2 4),(1 1, -1 1, -1 -1, 1 1))" );
  geom = QgsGeometry::fromWkt( wkt );
  geom.rotate( 90, QgsPointXY( 0, 0 ) );
  obtained = geom.asWkt();
  expected = QStringLiteral( "Polygon ((4 2, -10 2, 3 -2, 4 2),(1 -1, 1 1, -1 1, 1 -1))" );
  QCOMPARE( obtained, expected );
  geom.rotate( -90, QgsPointXY( 0, 0 ) );
  obtained = geom.asWkt();
  QCOMPARE( obtained, wkt );

  wkt = QStringLiteral( "Point (40 50)" );
  geom = QgsGeometry::fromWkt( wkt );
  geom.rotate( 90, QgsPointXY( 0, 0 ) );
  obtained = geom.asWkt();
  expected = QStringLiteral( "Point (50 -40)" );
  QCOMPARE( obtained, expected );
  geom.rotate( -90, QgsPointXY( 0, 0 ) );
  obtained = geom.asWkt();
  QCOMPARE( obtained, wkt );
  geom.rotate( 180, QgsPointXY( 40, 0 ) );
  expected = QStringLiteral( "Point (40 -50)" );
  obtained = geom.asWkt();
  QCOMPARE( obtained, expected );
  geom.rotate( 180, QgsPointXY( 40, 0 ) ); // round-trip
  obtained = geom.asWkt();
  QCOMPARE( obtained, wkt );

}

void TestQgsGeometry::unionCheck1()
{
  // should be a multipolygon with 2 parts as A does not intersect C
  QgsGeometry mypUnionGeometry  =  mpPolygonGeometryA.combine( mpPolygonGeometryC );
  qDebug() << "Geometry Type: " << QgsWkbTypes::displayString( mypUnionGeometry.wkbType() );
  QVERIFY( mypUnionGeometry.wkbType() == QgsWkbTypes::MultiPolygon );
  QgsMultiPolygonXY myMultiPolygon = mypUnionGeometry.asMultiPolygon();
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
  QgsPolygonXY myPolygon = mypUnionGeometry.asPolygon();
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
  QgsPolygonXY myPolygon = mypDifferenceGeometry.asPolygon();
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
  QgsPolygonXY myPolygon = mypDifferenceGeometry.asPolygon();
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
  QgsPolygonXY myPolygon = mypBufferGeometry.asPolygon();
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
  QString obtained = result.asWkt();
  QCOMPARE( obtained, wkt );

  //linestring
  wkt = QStringLiteral( "LineString(0 0, 10 0, 10 10, 20 10)" );
  geom = QgsGeometry::fromWkt( wkt );
  result = geom.smooth( 1, 0.25 );
  QgsPolylineXY line = result.asPolyline();
  QgsPolylineXY expectedLine;
  expectedLine << QgsPointXY( 0, 0 ) << QgsPointXY( 7.5, 0 ) << QgsPointXY( 10.0, 2.5 )
               << QgsPointXY( 10.0, 7.5 ) << QgsPointXY( 12.5, 10.0 ) << QgsPointXY( 20.0, 10.0 );
  QVERIFY( QgsGeometry::compare( line, expectedLine ) );

  //linestringM
  wkt = QStringLiteral( "LineStringM(0 0 1, 10 0 2, 10 10 6, 20 10 4)" );
  geom = QgsGeometry::fromWkt( wkt );
  result = geom.smooth( 1, 0.25 );
  QCOMPARE( result.asWkt(), QStringLiteral( "LineStringM (0 0 1, 7.5 0 1.75, 10 2.5 3, 10 7.5 5, 12.5 10 5.5, 20 10 4)" ) );

  //linestringZ
  wkt = QStringLiteral( "LineStringZ(0 0 1, 10 0 2, 10 10 6, 20 10 4)" );
  geom = QgsGeometry::fromWkt( wkt );
  result = geom.smooth( 1, 0.25 );
  QCOMPARE( result.asWkt(), QStringLiteral( "LineStringZ (0 0 1, 7.5 0 1.75, 10 2.5 3, 10 7.5 5, 12.5 10 5.5, 20 10 4)" ) );

  //linestringZM
  wkt = QStringLiteral( "LineStringZM(0 0 1 4, 10 0 2 8, 10 10 6 2, 20 10 4 0)" );
  geom = QgsGeometry::fromWkt( wkt );
  result = geom.smooth( 1, 0.25 );
  QCOMPARE( result.asWkt(), QStringLiteral( "LineStringZM (0 0 1 4, 7.5 0 1.75 7, 10 2.5 3 6.5, 10 7.5 5 3.5, 12.5 10 5.5 1.5, 20 10 4 0)" ) );

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
  QgsMultiPolylineXY multiLine = result.asMultiPolyline();
  QgsMultiPolylineXY expectedMultiline;
  expectedMultiline << ( QgsPolylineXY() << QgsPointXY( 0, 0 ) << QgsPointXY( 7.5, 0 ) << QgsPointXY( 10.0, 2.5 )
                         <<  QgsPointXY( 10.0, 7.5 ) << QgsPointXY( 12.5, 10.0 ) << QgsPointXY( 20.0, 10.0 ) )
                    << ( QgsPolylineXY() << QgsPointXY( 30.0, 30.0 ) << QgsPointXY( 37.5, 30.0 ) << QgsPointXY( 40.0, 32.5 )
                         << QgsPointXY( 40.0, 37.5 ) << QgsPointXY( 42.5, 40.0 ) << QgsPointXY( 50.0, 40.0 ) );
  QVERIFY( QgsGeometry::compare( multiLine, expectedMultiline ) );

  //polygon
  wkt = QStringLiteral( "Polygon ((0 0, 10 0, 10 10, 0 10, 0 0 ),(2 2, 4 2, 4 4, 2 4, 2 2))" );
  geom = QgsGeometry::fromWkt( wkt );
  result = geom.smooth( 1, 0.25 );
  QgsPolygonXY poly = result.asPolygon();
  QgsPolygonXY expectedPolygon;
  expectedPolygon << ( QgsPolylineXY() << QgsPointXY( 2.5, 0 ) << QgsPointXY( 7.5, 0 ) << QgsPointXY( 10.0, 2.5 )
                       <<  QgsPointXY( 10.0, 7.5 ) << QgsPointXY( 7.5, 10.0 ) << QgsPointXY( 2.5, 10.0 ) << QgsPointXY( 0, 7.5 )
                       << QgsPointXY( 0, 2.5 ) << QgsPointXY( 2.5, 0 ) )
                  << ( QgsPolylineXY() << QgsPointXY( 2.5, 2.0 ) << QgsPointXY( 3.5, 2.0 ) << QgsPointXY( 4.0, 2.5 )
                       << QgsPointXY( 4.0, 3.5 ) << QgsPointXY( 3.5, 4.0 ) << QgsPointXY( 2.5, 4.0 )
                       << QgsPointXY( 2.0, 3.5 ) << QgsPointXY( 2.0, 2.5 ) << QgsPointXY( 2.5, 2.0 ) );
  QVERIFY( QgsGeometry::compare( poly, expectedPolygon ) );

  //polygonM
  wkt = QStringLiteral( "PolygonM ((0 0 1, 10 0 4, 10 10 6, 0 10 8, 0 0 1 ),(2 2 3, 4 2 5, 4 4 7, 2 4 9, 2 2 3))" );
  geom = QgsGeometry::fromWkt( wkt );
  result = geom.smooth( 1, 0.25 );
  QCOMPARE( result.asWkt(), QStringLiteral( "PolygonM ((2.5 0 1.75, 7.5 0 3.25, 10 2.5 4.5, 10 7.5 5.5, 7.5 10 6.5, 2.5 10 7.5, 0 7.5 6.25, 0 2.5 2.75, 2.5 0 1.75),(2.5 2 3.5, 3.5 2 4.5, 4 2.5 5.5, 4 3.5 6.5, 3.5 4 7.5, 2.5 4 8.5, 2 3.5 7.5, 2 2.5 4.5, 2.5 2 3.5))" ) );

  //polygonZ
  wkt = QStringLiteral( "PolygonZ ((0 0 1, 10 0 4, 10 10 6, 0 10 8, 0 0 1 ),(2 2 3, 4 2 5, 4 4 7, 2 4 9, 2 2 3))" );
  geom = QgsGeometry::fromWkt( wkt );
  result = geom.smooth( 1, 0.25 );
  QCOMPARE( result.asWkt(), QStringLiteral( "PolygonZ ((2.5 0 1.75, 7.5 0 3.25, 10 2.5 4.5, 10 7.5 5.5, 7.5 10 6.5, 2.5 10 7.5, 0 7.5 6.25, 0 2.5 2.75, 2.5 0 1.75),(2.5 2 3.5, 3.5 2 4.5, 4 2.5 5.5, 4 3.5 6.5, 3.5 4 7.5, 2.5 4 8.5, 2 3.5 7.5, 2 2.5 4.5, 2.5 2 3.5))" ) );

  //polygonZM
  wkt = QStringLiteral( "PolygonZ ((0 0 1 6, 10 0 4 2, 10 10 6 0, 0 10 8 4, 0 0 1 6 ))" );
  geom = QgsGeometry::fromWkt( wkt );
  result = geom.smooth( 1, 0.25 );
  QCOMPARE( result.asWkt(), QStringLiteral( "PolygonZM ((2.5 0 1.75 5, 7.5 0 3.25 3, 10 2.5 4.5 1.5, 10 7.5 5.5 0.5, 7.5 10 6.5 1, 2.5 10 7.5 3, 0 7.5 6.25 4.5, 0 2.5 2.75 5.5, 2.5 0 1.75 5))" ) );

  //polygon with max angle - should be unchanged
  wkt = QStringLiteral( "Polygon ((0 0, 10 0, 10 10, 0 10, 0 0))" );
  geom = QgsGeometry::fromWkt( wkt );
  result = geom.smooth( 1, 0.25, -1, 50 );
  poly = result.asPolygon();
  expectedPolygon.clear();
  expectedPolygon << ( QgsPolylineXY() << QgsPointXY( 0, 0 ) << QgsPointXY( 10, 0 ) << QgsPointXY( 10.0, 10 )
                       <<  QgsPointXY( 0, 10 ) << QgsPointXY( 0, 0 ) );
  QVERIFY( QgsGeometry::compare( poly, expectedPolygon ) );

  //multipolygon)
  wkt = QStringLiteral( "MultiPolygon (((0 0, 10 0, 10 10, 0 10, 0 0 )),((2 2, 4 2, 4 4, 2 4, 2 2)))" );
  geom = QgsGeometry::fromWkt( wkt );
  result = geom.smooth( 1, 0.1 );
  QgsMultiPolygonXY multipoly = result.asMultiPolygon();
  QgsMultiPolygonXY expectedMultiPoly;
  expectedMultiPoly
      << ( QgsPolygonXY() << ( QgsPolylineXY() << QgsPointXY( 1.0, 0 ) << QgsPointXY( 9, 0 ) << QgsPointXY( 10.0, 1 )
                               <<  QgsPointXY( 10.0, 9 ) << QgsPointXY( 9, 10.0 ) << QgsPointXY( 1, 10.0 ) << QgsPointXY( 0, 9 )
                               << QgsPointXY( 0, 1 ) << QgsPointXY( 1, 0 ) ) )
      << ( QgsPolygonXY() << ( QgsPolylineXY() << QgsPointXY( 2.2, 2.0 ) << QgsPointXY( 3.8, 2.0 ) << QgsPointXY( 4.0, 2.2 )
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
  QVector< QgsGeometry > list;
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

  QCOMPARE( geom.constGet()->asWkt(), resultGeometry.constGet()->asWkt() );

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
  QString obtained = geom.asJson();
  QString geojson = QStringLiteral( "{\"type\": \"Point\", \"coordinates\": [40, 50]}" );
  QCOMPARE( obtained, geojson );

  //MultiPoint
  wkt = QStringLiteral( "MultiPoint (0 0, 10 0, 10 10, 20 10)" );
  geom = QgsGeometry::fromWkt( wkt );
  obtained = geom.asJson();
  geojson = QStringLiteral( "{\"type\": \"MultiPoint\", \"coordinates\": [ [0, 0], [10, 0], [10, 10], [20, 10]] }" );
  QCOMPARE( obtained, geojson );

  //Linestring
  wkt = QStringLiteral( "LineString(0 0, 10 0, 10 10, 20 10)" );
  geom = QgsGeometry::fromWkt( wkt );
  obtained = geom.asJson();
  geojson = QStringLiteral( "{\"type\": \"LineString\", \"coordinates\": [ [0, 0], [10, 0], [10, 10], [20, 10]]}" );
  QCOMPARE( obtained, geojson );

  //MultiLineString
  wkt = QStringLiteral( "MultiLineString ((0 0, 10 0, 10 10, 20 10),(30 30, 40 30, 40 40, 50 40))" );
  geom = QgsGeometry::fromWkt( wkt );
  obtained = geom.asJson();
  geojson = QStringLiteral( "{\"type\": \"MultiLineString\", \"coordinates\": [[ [0, 0], [10, 0], [10, 10], [20, 10]], [ [30, 30], [40, 30], [40, 40], [50, 40]]] }" );
  QCOMPARE( obtained, geojson );

  //Polygon
  wkt = QStringLiteral( "Polygon ((0 0, 10 0, 10 10, 0 10, 0 0 ),(2 2, 4 2, 4 4, 2 4, 2 2))" );
  geom = QgsGeometry::fromWkt( wkt );
  obtained = geom.asJson();
  geojson = QStringLiteral( "{\"type\": \"Polygon\", \"coordinates\": [[ [0, 0], [10, 0], [10, 10], [0, 10], [0, 0]], [ [2, 2], [4, 2], [4, 4], [2, 4], [2, 2]]] }" );
  QCOMPARE( obtained, geojson );

  //MultiPolygon
  wkt = QStringLiteral( "MultiPolygon (((0 0, 10 0, 10 10, 0 10, 0 0 )),((2 2, 4 2, 4 4, 2 4, 2 2)))" );
  geom = QgsGeometry::fromWkt( wkt );
  obtained = geom.asJson();
  geojson = QStringLiteral( "{\"type\": \"MultiPolygon\", \"coordinates\": [[[ [0, 0], [10, 0], [10, 10], [0, 10], [0, 0]]], [[ [2, 2], [4, 2], [4, 4], [2, 4], [2, 2]]]] }" );
  QCOMPARE( obtained, geojson );

  // no geometry
  QgsGeometry nullGeom( nullptr );
  obtained = nullGeom.asJson();
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

void TestQgsGeometry::dumpMultiPolygon( QgsMultiPolygonXY &multiPolygon )
{
  qDebug( "Multipolygon Geometry Dump" );
  for ( int i = 0; i < multiPolygon.size(); i++ )
  {
    QgsPolygonXY myPolygon = multiPolygon.at( i );
    qDebug( "\tPolygon in multipolygon: %d", i );
    dumpPolygon( myPolygon );
  }
}

void TestQgsGeometry::dumpPolygon( QgsPolygonXY &polygon )
{
  QVector<QPointF> myPoints;
  for ( int j = 0; j < polygon.size(); j++ )
  {
    QgsPolylineXY myPolyline = polygon.at( j ); //rings of polygon
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

void TestQgsGeometry::dumpPolyline( QgsPolylineXY &polyline )
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
  QString wkt = g14182.asWkt();
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

void TestQgsGeometry::directionNeutralSegmentation()
{
  //Tests, if segmentation of a circularstring is the same in both directions
  QString CWCircularStringWkt( QStringLiteral( "CIRCULARSTRING( 0 0, 0.5 0.5, 0.83 7.33 )" ) );
  QgsCircularString *CWCircularString = static_cast<QgsCircularString *>( QgsGeometryFactory::geomFromWkt( CWCircularStringWkt ).release() );
  QgsLineString *CWLineString = CWCircularString->curveToLine();

  QString CCWCircularStringWkt( QStringLiteral( "CIRCULARSTRING( 0.83 7.33, 0.5 0.5, 0 0 )" ) );
  QgsCircularString *CCWCircularString = static_cast<QgsCircularString *>( QgsGeometryFactory::geomFromWkt( CCWCircularStringWkt ).release() );
  QgsLineString *CCWLineString = CCWCircularString->curveToLine();
  QgsLineString *reversedCCWLineString = CCWLineString->reversed();

  QCOMPARE( CWLineString->asWkt(), reversedCCWLineString->asWkt() );
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
  QgsGeometry degen1 = QgsGeometry::fromWkt( QStringLiteral( "Polygon(( 0 0, 1 0, 2 0, 0 0 ))" ) );
  point = degen1.poleOfInaccessibility( 1 ).asPoint();
  QGSCOMPARENEAR( point.x(), 0, 0.01 );
  QGSCOMPARENEAR( point.y(), 0, 0.01 );

  QgsGeometry degen2 = QgsGeometry::fromWkt( QStringLiteral( "Polygon(( 0 0, 1 0, 1 1 , 1 0, 0 0 ))" ) );
  point = degen2.poleOfInaccessibility( 1 ).asPoint();
  QGSCOMPARENEAR( point.x(), 0, 0.01 );
  QGSCOMPARENEAR( point.y(), 0, 0.01 );

  //empty geometry
  QVERIFY( QgsGeometry().poleOfInaccessibility( 1 ).isNull() );

  // not a polygon
  QgsGeometry lineString = QgsGeometry::fromWkt( QStringLiteral( "LineString(1 0, 2 2 )" ) );
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
  geoms << qMakePair( QStringLiteral( "LINESTRING(0 0)" ),
                      QStringLiteral( "POINT(0 0)" ) );
  // unclosed ring
  geoms << qMakePair( QStringLiteral( "POLYGON((10 22,10 32,20 32,20 22))" ),
                      QStringLiteral( "POLYGON((10 22,10 32,20 32,20 22,10 22))" ) );
  // butterfly polygon (self-intersecting ring)
  geoms << qMakePair( QStringLiteral( "POLYGON((0 0, 10 10, 10 0, 0 10, 0 0))" ),
                      QStringLiteral( "MULTIPOLYGON(((5 5, 0 0, 0 10, 5 5)),((5 5, 10 10, 10 0, 5 5)))" ) );
  // polygon with extra tail (a part of the ring does not form any area)
  geoms << qMakePair( QStringLiteral( "POLYGON((0 0, 1 0, 1 1, 0 1, 0 0, -1 0, 0 0))" ),
                      QStringLiteral( "GEOMETRYCOLLECTION(POLYGON((0 0, 0 1, 1 1, 1 0, 0 0)), LINESTRING(0 0, -1 0))" ) );
  // collection with invalid geometries
  geoms << qMakePair( QStringLiteral( "GEOMETRYCOLLECTION(LINESTRING(0 0, 0 0), POLYGON((0 0, 10 10, 10 0, 0 10, 0 0)), LINESTRING(10 0, 10 10))" ),
                      QStringLiteral( "GEOMETRYCOLLECTION(POINT(0 0), MULTIPOLYGON(((5 5, 0 0, 0 10, 5 5)),((5 5, 10 10, 10 0, 5 5))), LINESTRING(10 0, 10 10)))" ) );
  // null line (#18077)
  geoms << qMakePair( QStringLiteral( "MultiLineString ((356984.0625 6300089, 356984.0625 6300089))" ),
                      QStringLiteral( "MultiPoint ((356984.0625 6300089))" ) );

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
  geoms << qMakePair( QStringLiteral( "LINESTRING(0 0, 1 0, 1 1)" ), true );
  geoms << qMakePair( QStringLiteral( "LINESTRING(0 0, 1 0, 1 1, 0 0)" ), true );  // may be closed (linear ring)
  geoms << qMakePair( QStringLiteral( "LINESTRING(0 0, 1 0, 1 1, 0 -1)" ), false ); // self-intersection
  geoms << qMakePair( QStringLiteral( "LINESTRING(0 0, 1 0, 1 1, 0.5 0, 0 1)" ), false ); // self-tangency
  geoms << qMakePair( QStringLiteral( "POINT(1 1)" ), true ); // points are simple
  geoms << qMakePair( QStringLiteral( "POLYGON((0 0, 1 1, 1 1, 0 0))" ), true ); // polygons are always simple, even if they are invalid
  geoms << qMakePair( QStringLiteral( "MULTIPOINT((1 1), (2 2))" ), true );
  geoms << qMakePair( QStringLiteral( "MULTIPOINT((1 1), (1 1))" ), false );  // must not contain the same point twice
  geoms << qMakePair( QStringLiteral( "MULTILINESTRING((0 0, 1 0), (0 1, 1 1))" ), true );
  geoms << qMakePair( QStringLiteral( "MULTILINESTRING((0 0, 1 0), (0 0, 1 0))" ), true );  // may be touching at endpoints
  geoms << qMakePair( QStringLiteral( "MULTILINESTRING((0 0, 1 1), (0 1, 1 0))" ), false );  // must not intersect each other
  geoms << qMakePair( QStringLiteral( "MULTIPOLYGON(((0 0, 1 1, 1 1, 0 0)),((0 0, 1 1, 1 1, 0 0)))" ), true ); // multi-polygons are always simple

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
  QgsGeometry g2D = QgsGeometry::fromWkt( QStringLiteral( "LINESTRING(10 10, 20 20)" ) );
  QgsGeometry g3D = QgsGeometry::fromWkt( QStringLiteral( "LINESTRINGZ(10 10 1, 20 20 2)" ) );

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
  QCOMPARE( g2D_1.asWkt(), QString( "LineString (10 10, 20 20, 30 30)" ) );

  // prepend with 2D line
  QgsGeometry g2D_2 = g2D;
  res = g2D_2.reshapeGeometry( line2D_2 );
  QCOMPARE( res, 0 );
  QCOMPARE( g2D_2.asWkt(), QString( "LineString (-10 -10, 10 10, 20 20)" ) );

  // append with 3D line
  QgsGeometry g3D_1 = g3D;
  res = g3D_1.reshapeGeometry( line3D_1 );
  QCOMPARE( res, 0 );
  QCOMPARE( g3D_1.asWkt(), QString( "LineStringZ (10 10 1, 20 20 2, 30 30 3)" ) );

  // prepend with 3D line
  QgsGeometry g3D_2 = g3D;
  res = g3D_2.reshapeGeometry( line3D_2 );
  QCOMPARE( res, 0 );
  QCOMPARE( g3D_2.asWkt(), QString( "LineStringZ (-10 -10 -1, 10 10 1, 20 20 2)" ) );
}

void TestQgsGeometry::createCollectionOfType()
{
  std::unique_ptr< QgsGeometryCollection > collect( QgsGeometryFactory::createCollectionOfType( QgsWkbTypes::Unknown ) );
  QVERIFY( !collect );
  collect = QgsGeometryFactory::createCollectionOfType( QgsWkbTypes::Point );
  QCOMPARE( collect->wkbType(), QgsWkbTypes::MultiPoint );
  QVERIFY( dynamic_cast< QgsMultiPoint *>( collect.get() ) );
  collect = QgsGeometryFactory::createCollectionOfType( QgsWkbTypes::PointM );
  QCOMPARE( collect->wkbType(), QgsWkbTypes::MultiPointM );
  QVERIFY( dynamic_cast< QgsMultiPoint *>( collect.get() ) );
  collect = QgsGeometryFactory::createCollectionOfType( QgsWkbTypes::PointZM );
  QCOMPARE( collect->wkbType(), QgsWkbTypes::MultiPointZM );
  QVERIFY( dynamic_cast< QgsMultiPoint *>( collect.get() ) );
  collect = QgsGeometryFactory::createCollectionOfType( QgsWkbTypes::PointZ );
  QCOMPARE( collect->wkbType(), QgsWkbTypes::MultiPointZ );
  QVERIFY( dynamic_cast< QgsMultiPoint *>( collect.get() ) );
  collect = QgsGeometryFactory::createCollectionOfType( QgsWkbTypes::MultiPoint );
  QCOMPARE( collect->wkbType(), QgsWkbTypes::MultiPoint );
  QVERIFY( dynamic_cast< QgsMultiPoint *>( collect.get() ) );
  collect = QgsGeometryFactory::createCollectionOfType( QgsWkbTypes::LineStringZ );
  QCOMPARE( collect->wkbType(), QgsWkbTypes::MultiLineStringZ );
  QVERIFY( dynamic_cast< QgsMultiLineString *>( collect.get() ) );
  collect = QgsGeometryFactory::createCollectionOfType( QgsWkbTypes::PolygonM );
  QCOMPARE( collect->wkbType(), QgsWkbTypes::MultiPolygonM );
  QVERIFY( dynamic_cast< QgsMultiPolygon *>( collect.get() ) );
  collect = QgsGeometryFactory::createCollectionOfType( QgsWkbTypes::GeometryCollectionZ );
  QCOMPARE( collect->wkbType(), QgsWkbTypes::GeometryCollectionZ );
  QVERIFY( dynamic_cast< QgsGeometryCollection *>( collect.get() ) );
  collect = QgsGeometryFactory::createCollectionOfType( QgsWkbTypes::CurvePolygonM );
  QCOMPARE( collect->wkbType(), QgsWkbTypes::MultiSurfaceM );
  QVERIFY( dynamic_cast< QgsMultiSurface *>( collect.get() ) );
}

void TestQgsGeometry::minimalEnclosingCircle()
{
  QgsGeometry geomTest;
  QgsGeometry result, resultTest;
  QgsPointXY center;
  double radius;

  // empty
  result = geomTest.minimalEnclosingCircle( center, radius );
  QCOMPARE( center, QgsPointXY() );
  QCOMPARE( radius, 0.0 );
  QVERIFY( result.isNull() );

  // case 1
  geomTest = QgsGeometry::fromPointXY( QgsPointXY( 5, 5 ) );
  result = geomTest.minimalEnclosingCircle( center, radius );
  QCOMPARE( center, QgsPointXY( 5, 5 ) );
  QCOMPARE( radius, 0.0 );
  resultTest.set( QgsCircle( QgsPoint( center ), radius ).toPolygon( 36 ) );
  QCOMPARE( result.asWkt(), resultTest.asWkt() );

  // case 2
  geomTest = QgsGeometry::fromWkt( QStringLiteral( "MULTIPOINT( 3 8, 7 4 )" ) );
  result = geomTest.minimalEnclosingCircle( center, radius, 6 );
  QGSCOMPARENEARPOINT( center, QgsPointXY( 5, 6 ), 0.0001 );
  QGSCOMPARENEAR( radius, sqrt( 2 ) * 2, 0.0001 );
  QCOMPARE( result.asWkt( 1 ), QStringLiteral( "Polygon ((7 4, 4.3 3.3, 2.3 5.3, 3 8, 5.7 8.7, 7.7 6.7, 7 4))" ) );

  geomTest = QgsGeometry::fromWkt( QStringLiteral( "LINESTRING( 0 5, 2 2, 0 -5, -1 -1 )" ) );
  result = geomTest.minimalEnclosingCircle( center, radius, 6 );
  QGSCOMPARENEARPOINT( center, QgsPointXY( 0, 0 ), 0.0001 );
  QGSCOMPARENEAR( radius, 5, 0.0001 );
  QCOMPARE( result.asWkt( 1 ), QStringLiteral( "Polygon ((0 5, 4.3 2.5, 4.3 -2.5, -0 -5, -4.3 -2.5, -4.3 2.5, 0 5))" ) );

  geomTest = QgsGeometry::fromWkt( QStringLiteral( "MULTIPOINT( 0 5, 2 2, 0 -5, -1 -1 )" ) );
  result = geomTest.minimalEnclosingCircle( center, radius, 6 );
  QGSCOMPARENEARPOINT( center, QgsPointXY( 0, 0 ), 0.0001 );
  QGSCOMPARENEAR( radius, 5, 0.0001 );
  QCOMPARE( result.asWkt( 1 ), QStringLiteral( "Polygon ((0 5, 4.3 2.5, 4.3 -2.5, -0 -5, -4.3 -2.5, -4.3 2.5, 0 5))" ) );

  geomTest = QgsGeometry::fromWkt( QStringLiteral( "POLYGON(( 0 5, 2 2, 0 -5, -1 -1 ))" ) );
  result = geomTest.minimalEnclosingCircle( center, radius, 6 );
  QGSCOMPARENEARPOINT( center, QgsPointXY( 0, 0 ), 0.0001 );
  QGSCOMPARENEAR( radius, 5, 0.0001 );
  resultTest.set( QgsCircle( QgsPoint( center ), radius ).toPolygon( 36 ) );
  QCOMPARE( result.asWkt( 1 ), QStringLiteral( "Polygon ((0 5, 4.3 2.5, 4.3 -2.5, -0 -5, -4.3 -2.5, -4.3 2.5, 0 5))" ) );

  geomTest = QgsGeometry::fromWkt( QStringLiteral( "MULTIPOINT( 0 5, 0 -5, 0 0 )" ) );
  result = geomTest.minimalEnclosingCircle( center, radius, 6 );
  QGSCOMPARENEARPOINT( center, QgsPointXY( 0, 0 ), 0.0001 );
  QGSCOMPARENEAR( radius, 5, 0.0001 );
  resultTest.set( QgsCircle( QgsPoint( center ), radius ).toPolygon( 36 ) );
  QCOMPARE( result.asWkt( 1 ), QStringLiteral( "Polygon ((0 5, 4.3 2.5, 4.3 -2.5, -0 -5, -4.3 -2.5, -4.3 2.5, 0 5))" ) );

  // case 3
  geomTest = QgsGeometry::fromWkt( QStringLiteral( "MULTIPOINT((0 0), (5 5), (0 -5), (0 5), (-5 0))" ) );
  result = geomTest.minimalEnclosingCircle( center, radius, 6 );
  QGSCOMPARENEARPOINT( center, QgsPointXY( 0.8333, 0.8333 ), 0.0001 );
  QGSCOMPARENEAR( radius, 5.8926, 0.0001 );
  resultTest.set( QgsCircle( QgsPoint( center ), radius ).toPolygon( 36 ) );
  QCOMPARE( result.asWkt( 1 ), QStringLiteral( "Polygon ((0.8 6.7, 5.9 3.8, 5.9 -2.1, 0.8 -5.1, -4.3 -2.1, -4.3 3.8, 0.8 6.7))" ) );

}

void TestQgsGeometry::splitGeometry()
{
  QgsGeometry g1 = QgsGeometry::fromWkt( QStringLiteral( "Polygon ((492980.38648063864093274 7082334.45244149677455425, 493082.65415841294452548 7082319.87918917648494244, 492980.38648063858272508 7082334.45244149677455425, 492980.38648063864093274 7082334.45244149677455425))" ) );
  QVector<QgsGeometry> newGeoms;
  QVector<QgsPointXY> testPoints;
  QCOMPARE( g1.splitGeometry( QVector< QgsPointXY >() << QgsPointXY( 493825.46541286131832749, 7082214.02779923938214779 ) << QgsPointXY( 492955.04876351181883365, 7082338.06309300474822521 ),
                              newGeoms, false, testPoints ), QgsGeometry::NothingHappened );
  QVERIFY( newGeoms.isEmpty() );
}

void TestQgsGeometry::snappedToGrid()
{
  qDebug( "SnappedToGrid" );
  // points
  {
    qDebug( "\tPoints:" );
    auto check = []( QgsPoint * _a, QgsPoint const & b )
    {
      std::unique_ptr<QgsPoint> a {_a};
      // because it is to check after snapping, there shouldn't be small precision errors

      qDebug( "\t\tGridified point: %f, %f, %f, %f", a->x(), a->y(), a->z(), a->m() );
      qDebug( "\t\tExpected point: %f, %f, %f, %f", b.x(), b.y(), b.z(), b.m() );
      if ( !std::isnan( b.x() ) )
        QVERIFY( ( float )a->x() == ( float )b.x() );

      if ( !std::isnan( b.y() ) )
        QVERIFY( ( float )a->y() == ( float )b.y() );

      if ( !std::isnan( b.z() ) )
        QVERIFY( ( float )a->z() == ( float )b.z() );

      if ( !std::isnan( b.m() ) )
        QVERIFY( ( float )a->m() == ( float )b.m() );
    };


    check( QgsPoint( 0, 0 ).snappedToGrid( 1, 1 ),
           QgsPoint( 0, 0 ) );

    check( QgsPoint( 1, 2.732 ).snappedToGrid( 1, 1 ),
           QgsPoint( 1, 3 ) );

    check( QgsPoint( 1.3, 6.4 ).snappedToGrid( 1, 1 ),
           QgsPoint( 1, 6 ) );

    check( QgsPoint( 1.3, 6.4 ).snappedToGrid( 1, 0 ),
           QgsPoint( 1, 6.4 ) );


    // multiple checks with the same point
    auto p1 = QgsPoint( 1.38, 2.4432 );

    check( p1.snappedToGrid( 1, 1 ),
           QgsPoint( 1, 2 ) );

    check( p1.snappedToGrid( 1, 0.1 ),
           QgsPoint( 1, 2.4 ) );

    check( p1.snappedToGrid( 1, 0.01 ),
           QgsPoint( 1, 2.44 ) );

    // Let's test more dimensions
    auto p2 = QgsPoint( 4.2134212, 543.1231, 0.123, 12.944145 );

    check( p2.snappedToGrid( 0, 0, 0, 0 ),
           p2 );

    check( p2.snappedToGrid( 0, 0, 1, 1 ),
           QgsPoint( 4.2134212, 543.1231, 0, 13 ) );

    check( p2.snappedToGrid( 1, 0.1, 0.01, 0.001 ),
           QgsPoint( 4, 543.1, 0.12, 12.944 ) );

  }

  // MultiPolygon (testing QgsCollection, QgsCurvePolygon and QgsLineString)
  {
    /*
     * List of tested edge cases:
     *
     * - QgsLineString becoming a point
     * - QgsLineString losing enough points so it is no longer closed
     * - QgsCurvePolygon losing its external ring
     * - QgsCurvePolygon losing an internal ring
     * - QgsCurvePolygon losing all internal rings
     * - QgsCollection losing one of its members
     * - QgsCollection losing all its members
     */

    auto in = QString( "MultiPolygon (((-1.2 -0.87, -0.943 0.8, 0.82 1.4, 1.2 0.9, 0.9 -0.6, -1.2 -0.87),(0.4 0, -0.4 0, 0 0.2, 0.4 0)),((2 0, 2.2 0.2, 2.2 -0.2)),((3 0, 4 0.2, 4 -0.2, 3 0)),((4 8, 3.6 4.1, 0.3 4.9, -0.2 7.8, 4 8),(6.7 7.3, 7 6.4, 5.6 5.9, 6.2 6.8, 6.7 7.3),(6 5.2, 4.9 5.3, 4.8 6.2, 6 5.2)))" );
    auto out = QString( "MultiPolygon (((-1 -1, -1 1, 1 1, 1 -1, -1 -1)),((4 8, 4 4, 0 5, 0 8, 4 8),(7 7, 7 6, 6 6, 6 7, 7 7),(6 5, 5 5, 5 6, 6 5)))" );

    auto inGeom = QgsGeometryFactory::geomFromWkt( in );

    std::unique_ptr<QgsAbstractGeometry> snapped { inGeom->snappedToGrid( 1, 1 ) };
    QCOMPARE( snapped->asWkt( 5 ), out );
  }

  {
    // Curves
    QgsGeometry curve = QgsGeometry::fromWkt( "CircularString( 68.1 415.2, 27.1 505.2, 27.1 406.2 )" );
    std::unique_ptr<QgsAbstractGeometry> snapped { curve.constGet()->snappedToGrid( 1, 1 ) };
    QCOMPARE( snapped->asWkt( 5 ), QStringLiteral( "CircularString (68 415, 27 505, 27 406)" ) );
    snapped.reset( curve.constGet()->snappedToGrid( 10, 1 ) );
    QCOMPARE( snapped->asWkt( 5 ), QStringLiteral( "CircularString (70 415, 30 505, 30 406)" ) );
    snapped.reset( curve.constGet()->snappedToGrid( 1, 10 ) );
    QCOMPARE( snapped->asWkt( 5 ), QStringLiteral( "CircularString (68 420, 27 510, 27 410)" ) );
    snapped.reset( curve.constGet()->snappedToGrid( 0, 0 ) );
    QCOMPARE( snapped->asWkt( 5 ), QStringLiteral( "CircularString (68.1 415.2, 27.1 505.2, 27.1 406.2)" ) );

    curve = QgsGeometry::fromWkt( "CircularString (68.1 415.2, 27.1 505.2, 27.1 406.2, 35.1 410.1, 39.2 403.2)" );
    snapped.reset( curve.constGet()->snappedToGrid( 1, 1 ) );
    QCOMPARE( snapped->asWkt( 5 ), QStringLiteral( "CircularString (68 415, 27 505, 27 406, 35 410, 39 403)" ) );
  }

  {
    // Lines
    QgsGeometry curve = QgsGeometry::fromWkt( "LineString( 68.1 415.2, 27.1 505.2, 27.1 406.2 )" );
    std::unique_ptr<QgsAbstractGeometry> snapped { curve.constGet()->snappedToGrid( 1, 1 ) };
    QCOMPARE( snapped->asWkt( 5 ), QStringLiteral( "LineString (68 415, 27 505, 27 406)" ) );
    snapped.reset( curve.constGet()->snappedToGrid( 10, 1 ) );
    QCOMPARE( snapped->asWkt( 5 ), QStringLiteral( "LineString (70 415, 30 505, 30 406)" ) );
    snapped.reset( curve.constGet()->snappedToGrid( 1, 10 ) );
    QCOMPARE( snapped->asWkt( 5 ), QStringLiteral( "LineString (68 420, 27 510, 27 410)" ) );

    snapped.reset( curve.constGet()->snappedToGrid( 0, 1 ) );
    QCOMPARE( snapped->asWkt( 5 ), QStringLiteral( "LineString (68.1 415, 27.1 505, 27.1 406)" ) );
    snapped.reset( curve.constGet()->snappedToGrid( 1, 0 ) );
    QCOMPARE( snapped->asWkt( 5 ), QStringLiteral( "LineString (68 415.2, 27 505.2, 27 406.2)" ) );

    curve = QgsGeometry::fromWkt( "LineStringZ (68.1 415.2 11.2, 27.1 505.2 23.6, 27.1 406.2 39.9)" );
    snapped.reset( curve.constGet()->snappedToGrid( 1, 1 ) );
    QCOMPARE( snapped->asWkt( 5 ), QStringLiteral( "LineStringZ (68 415 11.2, 27 505 23.6, 27 406 39.9)" ) );
    snapped.reset( curve.constGet()->snappedToGrid( 1, 1, 1 ) );
    QCOMPARE( snapped->asWkt( 5 ), QStringLiteral( "LineStringZ (68 415 11, 27 505 24, 27 406 40)" ) );
    snapped.reset( curve.constGet()->snappedToGrid( 1, 1, 10 ) );
    QCOMPARE( snapped->asWkt( 5 ), QStringLiteral( "LineStringZ (68 415 10, 27 505 20, 27 406 40)" ) );

    curve = QgsGeometry::fromWkt( "LineStringM (68.1 415.2 11.2, 27.1 505.2 23.6, 27.1 406.2 39.9)" );
    snapped.reset( curve.constGet()->snappedToGrid( 1, 1 ) );
    QCOMPARE( snapped->asWkt( 5 ), QStringLiteral( "LineStringM (68 415 11.2, 27 505 23.6, 27 406 39.9)" ) );
    snapped.reset( curve.constGet()->snappedToGrid( 1, 1, 0, 1 ) );
    QCOMPARE( snapped->asWkt( 5 ), QStringLiteral( "LineStringM (68 415 11, 27 505 24, 27 406 40)" ) );
    snapped.reset( curve.constGet()->snappedToGrid( 1, 1, 0, 10 ) );
    QCOMPARE( snapped->asWkt( 5 ), QStringLiteral( "LineStringM (68 415 10, 27 505 20, 27 406 40)" ) );

    curve = QgsGeometry::fromWkt( "LineStringZM (68.1 415.2 11.2 56.7, 27.1 505.2 23.6 49.1, 27.1 406.2 39.9 32.4)" );
    snapped.reset( curve.constGet()->snappedToGrid( 1, 1 ) );
    QCOMPARE( snapped->asWkt( 5 ), QStringLiteral( "LineStringZM (68 415 11.2 56.7, 27 505 23.6 49.1, 27 406 39.9 32.4)" ) );
    snapped.reset( curve.constGet()->snappedToGrid( 1, 1, 1, 1 ) );
    QCOMPARE( snapped->asWkt( 5 ), QStringLiteral( "LineStringZM (68 415 11 57, 27 505 24 49, 27 406 40 32)" ) );
    snapped.reset( curve.constGet()->snappedToGrid( 1, 1, 10, 10 ) );
    QCOMPARE( snapped->asWkt( 5 ), QStringLiteral( "LineStringZM (68 415 10 60, 27 505 20 50, 27 406 40 30)" ) );
  }

  //compound curve
  {
    // Curves
    QgsGeometry curve = QgsGeometry::fromWkt( "CompoundCurve(LineString(59.1 402.1, 68.1 415.2),CircularString( 68.1 415.2, 27.1 505.2, 27.1 406.2))" );
    std::unique_ptr<QgsAbstractGeometry> snapped { curve.constGet()->snappedToGrid( 1, 1 ) };
    QCOMPARE( snapped->asWkt(), QStringLiteral( "CompoundCurve ((59 402, 68 415),CircularString (68 415, 27 505, 27 406))" ) );
  }
}

void TestQgsGeometry::convertGeometryCollectionToSubclass()
{
  QgsGeometry gc0 = QgsGeometry::fromWkt( QStringLiteral( "GeometryCollection(Point(1 1), Polygon((2 2, 3 3, 2 3, 2 2)))" ) );

  QgsGeometry gc1 = gc0;
  QVERIFY( gc1.convertGeometryCollectionToSubclass( QgsWkbTypes::PointGeometry ) );
  QCOMPARE( gc1.asWkt(), QStringLiteral( "MultiPoint ((1 1))" ) );

  QgsGeometry gc2 = gc0;
  QVERIFY( gc2.convertGeometryCollectionToSubclass( QgsWkbTypes::PolygonGeometry ) );
  QCOMPARE( gc2.asWkt(), QStringLiteral( "MultiPolygon (((2 2, 3 3, 2 3, 2 2)))" ) );

  QgsGeometry gc3 = gc0;
  QVERIFY( gc3.convertGeometryCollectionToSubclass( QgsWkbTypes::LineGeometry ) );
  QCOMPARE( gc3.asWkt(), QStringLiteral( "MultiLineString ()" ) );  // I think this is not correct, WKT should be "MultiLineString Empty"
  QVERIFY( gc3.isEmpty() );

  // trying to convert a geometry that is not a geometry collection
  QgsGeometry wrong = QgsGeometry::fromWkt( QStringLiteral( "Point(1 2)" ) );
  QVERIFY( !wrong.convertGeometryCollectionToSubclass( QgsWkbTypes::PolygonGeometry ) );
}

QGSTEST_MAIN( TestQgsGeometry )
#include "testqgsgeometry.moc"
