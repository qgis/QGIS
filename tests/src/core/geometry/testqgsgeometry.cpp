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
#include "qgsproject.h"
#include "qgsgeos.h"
#include "qgsreferencedgeometry.h"

//qgs unit test utility class
#include "qgsrenderchecker.h"

#include "testtransformer.h"
#include "testgeometryutils.h"

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
    void cleanup();// will be called after every testfunction.
    void copy();
    void assignment();
    void asVariant(); //test conversion to and from a QVariant
    void referenced();
    void isEmpty();
    void isValid();
    void equality();
    void vertexIterator();
    void partIterator();

    void geos();

    void curveIndexOf_data();
    void curveIndexOf();
    void splitCurve_data();
    void splitCurve();

    void fromQgsPointXY();
    void fromQPoint();
    void fromQPolygonF();
    void fromPolyline();
    void asQPointF();
    void asQPolygonF();

    void comparePolylines();
    void comparePolygons();

    void createEmptyWithSameType();

    void compareTo_data();
    void compareTo();

    void scroll_data();
    void scroll();

    void normalize_data();
    void normalize();

    void simplifiedTypeRef_data();
    void simplifiedTypeRef();

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
    void shortestLineEmptyGeometry();

    void unaryUnion();

    void dataStream();

    void exportToGeoJSON();

    void wkbInOut();

    void directionNeutralSegmentation();
    void poleOfInaccessibility();

    void makeValid_data();
    void makeValid();

    void isSimple_data();
    void isSimple();

    void reshapeGeometryLineMerge();
    void createCollectionOfType();

    void orientedMinimumBoundingBox( );
    void minimalEnclosingCircle( );
    void splitGeometry();
    void snappedToGrid();

    void convertGeometryCollectionToSubclass();

    void emptyJson();

    void testRandomPointsInPolygon();

    void wktParser();

  private:
    //! Must be called before each render test
    void initPainterTest();

    //! A helper method to do a render check to see if the geometry op is as expected
    bool renderCheck( const QString &testName, const QString &comment = QString(), int mismatchCount = 0 );
    //! A helper method to dump to qdebug the geometry of a multipolygon
    void dumpMultiPolygon( QgsMultiPolygonXY &multiPolygon );
    void paintMultiPolygon( QgsMultiPolygonXY &multiPolygon );
    //! A helper method to dump to qdebug the geometry of a polygon
    void dumpPolygon( QgsPolygonXY &polygon );
    void paintPolygon( QgsPolygonXY &polygon );
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
  delete mpPainter;
  mpPainter = nullptr;

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

void TestQgsGeometry::initPainterTest()
{
  delete mpPainter;
  mpPainter = nullptr;
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


  //set the pen to a different color -
  //any test outs will be drawn in pen2
  mpPainter->setPen( mPen2 );
  mpPainter->setBrush( myBrush );
}

void TestQgsGeometry::cleanup()
{
  // will be called after every testfunction.
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
  QVERIFY( !var.canConvert< QgsReferencedGeometry >() );

  QgsGeometry fromVar = qvariant_cast<QgsGeometry>( var );
  QCOMPARE( fromVar.constGet()->vertexAt( QgsVertexId( 0, 0, 0 ) ).x(), 1.0 );
  QCOMPARE( fromVar.constGet()->vertexAt( QgsVertexId( 0, 0, 0 ) ).y(), 2.0 );

  //also check copying variant
  const QVariant &var2 = var;
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


void TestQgsGeometry::referenced()
{
  QgsReferencedGeometry geom1 = QgsReferencedGeometry( QgsGeometry::fromPointXY( QgsPointXY( 1, 2 ) ), QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:3111" ) ) );
  QCOMPARE( geom1.crs().authid(), QStringLiteral( "EPSG:3111" ) );
  geom1.setCrs( QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:28356" ) ) );
  QCOMPARE( geom1.crs().authid(), QStringLiteral( "EPSG:28356" ) );

  //convert to and from a QVariant
  QVariant var = QVariant::fromValue( geom1 );
  QVERIFY( var.isValid() );

  QVERIFY( var.canConvert< QgsReferencedGeometry >() );

  QgsReferencedGeometry geom2 = qvariant_cast<QgsReferencedGeometry>( var );
  QCOMPARE( geom2.asWkt(), geom1.asWkt() );
  QCOMPARE( geom2.crs().authid(), QStringLiteral( "EPSG:28356" ) );
}


void TestQgsGeometry::isEmpty()
{
  QgsGeometry geom;
  QVERIFY( geom.isNull() );

  geom.set( new QgsPoint() );
  QVERIFY( geom.isEmpty() );

  geom.set( new QgsPoint( 1.0, 2.0 ) );
  QVERIFY( !geom.isNull() );

  geom.set( nullptr );
  QVERIFY( geom.isNull() );

  QgsGeometryCollection collection;
  QVERIFY( collection.isEmpty() );
}

void TestQgsGeometry::isValid()
{
  QString error;
  // LineString
  QgsLineString line;
  QVERIFY( line.isValid( error ) );
  line.addVertex( QgsPoint( 0, 0 ) );
  QVERIFY( !line.isValid( error ) );
  QCOMPARE( error, QStringLiteral( "LineString has less than 2 points and is not empty." ) );
  line.addVertex( QgsPoint( 1, 1 ) );
  QVERIFY( line.isValid( error ) );

  // CircularString
  QgsCircularString circ;
  QVERIFY( circ.isValid( error ) );
  QgsPointSequence pts = QgsPointSequence() << QgsPoint( 0, 0 ) << QgsPoint( 2.5, 2.3 );
  circ.setPoints( pts );
  QVERIFY( !circ.isValid( error ) );
  QCOMPARE( error, QStringLiteral( "CircularString has less than 3 points and is not empty." ) );
  pts.append( QgsPoint( 5, 5 ) );
  circ.setPoints( pts );
  QVERIFY( circ.isValid( error ) );

  // CompoundCurve
  QgsCompoundCurve curve;
  QVERIFY( curve.isValid( error ) );
  curve.addCurve( line.clone() );
  QVERIFY( curve.isValid( error ) );
  curve.addCurve( circ.clone() );
  QVERIFY( curve.isValid( error ) );
  QgsLineString invalidLine;
  invalidLine.addVertex( QgsPoint( 0, 0 ) );
  curve.addCurve( invalidLine.clone() );
  QVERIFY( !curve.isValid( error ) );
  QCOMPARE( error, QStringLiteral( "Curve[3]: LineString has less than 2 points and is not empty." ) );
}

void TestQgsGeometry::equality()
{
  // null geometries
  QVERIFY( !QgsGeometry().equals( QgsGeometry() ) );

  // compare to null
  QgsGeometry g1( std::make_unique< QgsPoint >( 1.0, 2.0 ) );
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

  QgsGeometry emptyGeom = QgsGeometry::fromWkt( "LINESTRING EMPTY" );
  QgsVertexIterator it3 = emptyGeom.vertices();
  QVERIFY( !it3.hasNext() );
  emptyGeom = QgsGeometry::fromWkt( "POINT EMPTY" );
  QgsVertexIterator it4 = emptyGeom.vertices();
  QVERIFY( !it4.hasNext() );
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
  // geom2 should have adetached, geom should be unaffected by change
  QCOMPARE( geom.asWkt(), QStringLiteral( "Point (1 2)" ) );

  // See test_qgsgeometry.py for geometry-type specific checks!
}

void TestQgsGeometry::geos()
{
  // test GEOS conversion utils

  // empty parts should NOT be added to a GEOS collection -- it can cause crashes in GEOS
  QgsMultiPolygon polyWithEmptyParts;
  geos::unique_ptr asGeos( QgsGeos::asGeos( &polyWithEmptyParts ) );
  QgsGeometry res( QgsGeos::fromGeos( asGeos.get() ) );
  QCOMPARE( res.asWkt(), QStringLiteral( "MultiPolygon EMPTY" ) );
  polyWithEmptyParts.addGeometry( new QgsPolygon( new QgsLineString() ) );
  polyWithEmptyParts.addGeometry( new QgsPolygon( new QgsLineString( QVector< QgsPoint >() << QgsPoint( 0, 0 ) << QgsPoint( 0, 1 ) << QgsPoint( 1, 1 ) << QgsPoint( 0, 0 ) ) ) );
  polyWithEmptyParts.addGeometry( new QgsPolygon( new QgsLineString() ) );
  polyWithEmptyParts.addGeometry( new QgsPolygon( new QgsLineString( QVector< QgsPoint >() << QgsPoint( 10, 0 ) << QgsPoint( 10, 1 ) << QgsPoint( 11, 1 ) << QgsPoint( 10, 0 ) ) ) );
  asGeos = QgsGeos::asGeos( &polyWithEmptyParts );
  QCOMPARE( GEOSGetNumGeometries_r( QgsGeos::getGEOSHandler(), asGeos.get() ), 2 );
  res = QgsGeometry( QgsGeos::fromGeos( asGeos.get() ) );
  QCOMPARE( res.asWkt(), QStringLiteral( "MultiPolygon (((0 0, 0 1, 1 1, 0 0)),((10 0, 10 1, 11 1, 10 0)))" ) );

  // Empty geometry
  QgsPoint point;
  asGeos = QgsGeos::asGeos( &point );
  // should be treated as a null geometry, not an empty point in order to maintain api compatibility with
  // earlier QGIS 3.x releases
  QVERIFY( !QgsGeos::fromGeos( asGeos.get() ) );
}

void TestQgsGeometry::curveIndexOf_data()
{
  QTest::addColumn<QString>( "curve" );
  QTest::addColumn<QString>( "point" );
  QTest::addColumn<int>( "expected" );

  QTest::newRow( "linestring empty" ) << QStringLiteral( "LINESTRING()" ) << QStringLiteral( "Point( 1 2 )" ) << -1;
  QTest::newRow( "linestring one point no match" ) << QStringLiteral( "LINESTRING( 1 3 )" ) << QStringLiteral( "Point( 1 2 )" ) << -1;
  QTest::newRow( "linestring one point match" ) << QStringLiteral( "LINESTRING( 1 2 )" ) << QStringLiteral( "Point( 1 2 )" ) << 0;
  QTest::newRow( "linestring match 0" ) << QStringLiteral( "LINESTRING( 1 2, 2 3, 3 4)" ) << QStringLiteral( "Point( 1 2 )" ) << 0;
  QTest::newRow( "linestring match 1" ) << QStringLiteral( "LINESTRING( 1 2, 2 3, 3 4)" ) << QStringLiteral( "Point( 2 3 )" ) << 1;
  QTest::newRow( "linestring match 2" ) << QStringLiteral( "LINESTRING( 1 2, 2 3, 3 4)" ) << QStringLiteral( "Point( 3 4 )" ) << 2;
  QTest::newRow( "linestring no match" ) << QStringLiteral( "LINESTRING( 1 2, 2 3, 3 4)" ) << QStringLiteral( "Point( 3 3 )" ) << -1;
  QTest::newRow( "linestringz no match" ) << QStringLiteral( "LINESTRINGZ( 1 2 11, 2 3 12, 3 4 13)" ) << QStringLiteral( "PointZ( 2 3 11)" ) << -1;
  QTest::newRow( "linestringz match" ) << QStringLiteral( "LINESTRINGZ( 1 2 11, 2 3 12, 3 4 13)" ) << QStringLiteral( "PointZ( 2 3 12)" ) << 1;
  QTest::newRow( "linestringm no match" ) << QStringLiteral( "LINESTRINGM( 1 2 11, 2 3 12, 3 4 13)" ) << QStringLiteral( "PointM( 2 3 11)" ) << -1;
  QTest::newRow( "linestringm match" ) << QStringLiteral( "LINESTRINGM( 1 2 11, 2 3 12, 3 4 13)" ) << QStringLiteral( "PointM( 2 3 12)" ) << 1;
  QTest::newRow( "linestringzm no match z" ) << QStringLiteral( "LINESTRINGZM( 1 2 11 21, 2 3 12 22, 3 4 13 23)" ) << QStringLiteral( "PointM( 2 3 11 22)" ) << -1;
  QTest::newRow( "linestringzm no match m" ) << QStringLiteral( "LINESTRINGZM( 1 2 11 21, 2 3 12 22, 3 4 13 23)" ) << QStringLiteral( "PointM( 2 3 12 23)" ) << -1;
  QTest::newRow( "linestringzm match" ) << QStringLiteral( "LINESTRINGZM( 1 2 11 21, 2 3 12 22, 3 4 13 23)" ) << QStringLiteral( "PointZM( 2 3 12 22)" ) << 1;

  QTest::newRow( "circularstring empty" ) << QStringLiteral( "CIRCULARSTRING()" ) << QStringLiteral( "Point( 1 2 )" ) << -1;
  QTest::newRow( "circularstring match 0" ) << QStringLiteral( "CIRCULARSTRING( 1 2, 2 3, 3 4)" ) << QStringLiteral( "Point( 1 2 )" ) << 0;
  // should not match -- we only consider segment points, not curve points
  QTest::newRow( "circularstring match 1" ) << QStringLiteral( "CIRCULARSTRING( 1 2, 2 3, 3 4)" ) << QStringLiteral( "Point( 2 3 )" ) << -1;
  QTest::newRow( "circularstring match 2" ) << QStringLiteral( "CIRCULARSTRING( 1 2, 2 3, 3 4)" ) << QStringLiteral( "Point( 3 4 )" ) << 2;
  QTest::newRow( "circularstring no match" ) << QStringLiteral( "CIRCULARSTRING( 1 2, 2 3, 3 4)" ) << QStringLiteral( "Point( 3 3 )" ) << -1;
  QTest::newRow( "circularstringz no match" ) << QStringLiteral( "CIRCULARSTRINGZ( 1 2 11, 2 3 12, 3 4 13)" ) << QStringLiteral( "PointZ( 3 4 12)" ) << -1;
  QTest::newRow( "circularstringz match" ) << QStringLiteral( "CIRCULARSTRINGZ( 1 2 11, 2 3 12, 3 4 13)" ) << QStringLiteral( "PointZ( 3 4 13)" ) << 2;
  QTest::newRow( "circularstringm no match" ) << QStringLiteral( "CIRCULARSTRINGM( 1 2 11, 2 3 12, 3 4 13)" ) << QStringLiteral( "PointM( 3 4 12)" ) << -1;
  QTest::newRow( "circularstringm match" ) << QStringLiteral( "CIRCULARSTRINGM( 1 2 11, 2 3 12, 3 4 13)" ) << QStringLiteral( "PointM( 3 4 13)" ) << 2;
  QTest::newRow( "circularstringzm no match z" ) << QStringLiteral( "CIRCULARSTRINGZM( 1 2 11 21, 2 3 12 22, 3 4 13 23)" ) << QStringLiteral( "PointM( 3 4 14 23)" ) << -1;
  QTest::newRow( "circularstringzm no match m" ) << QStringLiteral( "CIRCULARSTRINGZM( 1 2 11 21, 2 3 12 22, 3 4 13 23)" ) << QStringLiteral( "PointM( 3 4 13 24)" ) << -1;
  QTest::newRow( "circularstringzm match" ) << QStringLiteral( "CIRCULARSTRINGZM( 1 2 11 21, 2 3 12 22, 3 4 13 23)" ) << QStringLiteral( "PointZM( 3 4 13 23)" ) << 2;

  QTest::newRow( "compound curve empty" ) << QStringLiteral( "COMPOUNDCURVE()" ) << QStringLiteral( "Point( 1 2 )" ) << -1;
  QTest::newRow( "compound curve no match" ) << QStringLiteral( "COMPOUNDCURVE((1 1, 2 3, 3 4),(3 4, 5 6, 7 8))" ) << QStringLiteral( "Point( 1 21 )" ) << -1;
  QTest::newRow( "compound curve match 0" ) << QStringLiteral( "COMPOUNDCURVE((1 1, 2 3, 3 4),(3 4, 5 6, 7 8))" ) << QStringLiteral( "Point( 1 1 )" ) << 0;
  QTest::newRow( "compound curve match 1" ) << QStringLiteral( "COMPOUNDCURVE((1 1, 2 3, 3 4),(3 4, 5 6, 7 8))" ) << QStringLiteral( "Point( 2 3 )" ) << 1;
  QTest::newRow( "compound curve match 2" ) << QStringLiteral( "COMPOUNDCURVE((1 1, 2 3, 3 4),(3 4, 5 6, 7 8))" ) << QStringLiteral( "Point( 3 4 )" ) << 2;
  QTest::newRow( "compound curve match 3" ) << QStringLiteral( "COMPOUNDCURVE((1 1, 2 3, 3 4),(3 4, 5 6, 7 8))" ) << QStringLiteral( "Point( 5 6 )" ) << 3;
  QTest::newRow( "compound curve match 4" ) << QStringLiteral( "COMPOUNDCURVE((1 1, 2 3, 3 4),(3 4, 5 6, 7 8))" ) << QStringLiteral( "Point( 7 8 )" ) << 4;
  QTest::newRow( "compound curve match 5" ) << QStringLiteral( "COMPOUNDCURVE((1 1, 2 3, 3 4),(3 4, 5 6, 7 8),(7 8, 9 10))" ) << QStringLiteral( "Point( 9 10 )" ) << 5;
  QTest::newRow( "compound curve circular string match" ) << QStringLiteral( "COMPOUNDCURVE((1 1, 2 3, 3 4),CIRCULARSTRING(3 4, 5 6, 7 8))" ) << QStringLiteral( "Point( 7 8 )" ) << 4;
  QTest::newRow( "compound curve circular string no match" ) << QStringLiteral( "COMPOUNDCURVE((1 1, 2 3, 3 4),CIRCULARSTRING(3 4, 5 6, 7 8))" ) << QStringLiteral( "Point( 5 6 )" ) << -1;
  QTest::newRow( "compound curve z match" ) << QStringLiteral( "COMPOUNDCURVEZ((1 1 11, 2 3 12, 3 4 13),(3 4 13, 5 6 14, 7 8 15))" ) << QStringLiteral( "PointZ( 7 8 15)" ) << 4;
  QTest::newRow( "compound curve z nomatch" ) << QStringLiteral( "COMPOUNDCURVEZ((1 1 11, 2 3 12, 3 4 13),(3 4 13, 5 6 14, 7 8 15))" ) << QStringLiteral( "PointZ( 7 8 16)" ) << -1;
  QTest::newRow( "compound curve m match" ) << QStringLiteral( "COMPOUNDCURVEM((1 1 11, 2 3 12, 3 4 13),(3 4 13, 5 6 14, 7 8 15))" ) << QStringLiteral( "PointM( 7 8 15)" ) << 4;
  QTest::newRow( "compound curve m nomatch" ) << QStringLiteral( "COMPOUNDCURVEM((1 1 11, 2 3 12, 3 4 13),(3 4 13, 5 6 14, 7 8 15))" ) << QStringLiteral( "PointM( 7 8 16)" ) << -1;
  QTest::newRow( "compound curve zm match" ) << QStringLiteral( "COMPOUNDCURVEZM((1 1 11 22, 2 3 12 23, 3 4 13 24),(3 4 13 24, 5 6 14 25, 7 8 15 26))" ) << QStringLiteral( "PointZM( 7 8 15 26)" ) << 4;
  QTest::newRow( "compound curve zm nomatch z" ) << QStringLiteral( "COMPOUNDCURVEZM((1 1 11 22, 2 3 12 23, 3 4 13 24),(3 4 13 24, 5 6 14 25, 7 8 15 26))" ) << QStringLiteral( "PointZM( 7 8 16 26)" ) << -1;
  QTest::newRow( "compound curve zm nomatch m" ) << QStringLiteral( "COMPOUNDCURVEZM((1 1 11 22, 2 3 12 23, 3 4 13 24),(3 4 13 24, 5 6 14 25, 7 8 15 26))" ) << QStringLiteral( "PointZM( 7 8 15 27)" ) << -1;
}

void TestQgsGeometry::curveIndexOf()
{
  QFETCH( QString, curve );
  QFETCH( QString, point );
  QFETCH( int, expected );

  QgsGeometry g = QgsGeometry::fromWkt( curve );
  QgsPoint p;
  p.fromWkt( point );
  QCOMPARE( qgsgeometry_cast< const QgsCurve * >( g.constGet() )->indexOf( p ), expected );
}

void TestQgsGeometry::splitCurve_data()
{
  QTest::addColumn<QString>( "wkt" );
  QTest::addColumn<int>( "vertex" );
  QTest::addColumn<QString>( "curve1" );
  QTest::addColumn<QString>( "curve2" );

  QTest::newRow( "linestring empty" ) << QStringLiteral( "LINESTRING()" ) << 0 << QStringLiteral( "LineString EMPTY" ) << QStringLiteral( "LineString EMPTY" );
  QTest::newRow( "linestring empty 1" ) << QStringLiteral( "LINESTRING()" ) << 1 << QStringLiteral( "LineString EMPTY" ) << QStringLiteral( "LineString EMPTY" );
  QTest::newRow( "linestring one vertex 0" ) << QStringLiteral( "LINESTRING( 1 2)" ) << 0 << QStringLiteral( "LineString (1 2)" ) << QStringLiteral( "LineString EMPTY" );
  QTest::newRow( "linestring one vertex 1 " ) << QStringLiteral( "LINESTRING( 1 2)" ) << 1 << QStringLiteral( "LineString (1 2)" ) << QStringLiteral( "LineString EMPTY" );
  QTest::newRow( "linestring one vertex 2" ) << QStringLiteral( "LINESTRING( 1 2)" ) << 2 << QStringLiteral( "LineString (1 2)" ) << QStringLiteral( "LineString EMPTY" );
  QTest::newRow( "linestring two 0" ) << QStringLiteral( "LINESTRING( 1 2, 2 3)" ) << 0 << QStringLiteral( "LineString EMPTY" ) << QStringLiteral( "LineString (1 2, 2 3)" );
  QTest::newRow( "linestring two 1" ) << QStringLiteral( "LINESTRING( 1 2, 2 3)" ) << 1 << QStringLiteral( "LineString (1 2, 2 3)" ) << QStringLiteral( "LineString EMPTY" );
  QTest::newRow( "linestring two 2" ) << QStringLiteral( "LINESTRING( 1 2, 2 3)" ) << 2 << QStringLiteral( "LineString (1 2, 2 3)" ) << QStringLiteral( "LineString EMPTY" );
  QTest::newRow( "linestring three 0" ) << QStringLiteral( "LINESTRING( 1 2, 2 3, 3 4)" ) << 0 << QStringLiteral( "LineString EMPTY" ) << QStringLiteral( "LineString (1 2, 2 3, 3 4)" );
  QTest::newRow( "linestring three 1" ) << QStringLiteral( "LINESTRING( 1 2, 2 3, 3 4)" ) << 1 << QStringLiteral( "LineString (1 2, 2 3)" ) << QStringLiteral( "LineString (2 3, 3 4)" );
  QTest::newRow( "linestring three 2" ) << QStringLiteral( "LINESTRING( 1 2, 2 3, 3 4)" ) << 2 << QStringLiteral( "LineString (1 2, 2 3, 3 4)" ) << QStringLiteral( "LineString EMPTY" );
  QTest::newRow( "linestring three 3" ) << QStringLiteral( "LINESTRING( 1 2, 2 3, 3 4)" ) << 3 << QStringLiteral( "LineString (1 2, 2 3, 3 4)" ) << QStringLiteral( "LineString EMPTY" );
  QTest::newRow( "linestring four 0" ) << QStringLiteral( "LINESTRING( 1 2, 2 3, 3 4, 5 6)" ) << 0 << QStringLiteral( "LineString EMPTY" ) << QStringLiteral( "LineString (1 2, 2 3, 3 4, 5 6)" );
  QTest::newRow( "linestring four 1" ) << QStringLiteral( "LINESTRING( 1 2, 2 3, 3 4, 5 6)" ) << 1 << QStringLiteral( "LineString (1 2, 2 3)" ) << QStringLiteral( "LineString (2 3, 3 4, 5 6)" );
  QTest::newRow( "linestring four 2" ) << QStringLiteral( "LINESTRING( 1 2, 2 3, 3 4, 5 6)" ) << 2 << QStringLiteral( "LineString (1 2, 2 3, 3 4)" ) << QStringLiteral( "LineString (3 4, 5 6)" );
  QTest::newRow( "linestring four 3" ) << QStringLiteral( "LINESTRING( 1 2, 2 3, 3 4, 5 6)" ) << 3 << QStringLiteral( "LineString (1 2, 2 3, 3 4, 5 6)" ) << QStringLiteral( "LineString EMPTY" );
  QTest::newRow( "linestringz" ) << QStringLiteral( "LINESTRINGZ( 1 2 11, 2 3 12, 3 4 13, 5 6 14)" ) << 1 << QStringLiteral( "LineStringZ (1 2 11, 2 3 12)" ) << QStringLiteral( "LineStringZ (2 3 12, 3 4 13, 5 6 14)" );
  QTest::newRow( "linestringm" ) << QStringLiteral( "LINESTRINGM( 1 2 11, 2 3 12, 3 4 13, 5 6 14)" ) << 1 << QStringLiteral( "LineStringM (1 2 11, 2 3 12)" ) << QStringLiteral( "LineStringM (2 3 12, 3 4 13, 5 6 14)" );
  QTest::newRow( "linestringzm" ) << QStringLiteral( "LINESTRINGZM( 1 2 11 21, 2 3 12 22, 3 4 13 23, 5 6 14 24)" ) << 1 << QStringLiteral( "LineStringZM (1 2 11 21, 2 3 12 22)" ) << QStringLiteral( "LineStringZM (2 3 12 22, 3 4 13 23, 5 6 14 24)" );

  QTest::newRow( "CircularString empty" ) << QStringLiteral( "CircularString()" ) << 0 << QStringLiteral( "CircularString EMPTY" ) << QStringLiteral( "CircularString EMPTY" );
  QTest::newRow( "CircularString empty 1" ) << QStringLiteral( "CircularString()" ) << 1 << QStringLiteral( "CircularString EMPTY" ) << QStringLiteral( "CircularString EMPTY" );
  QTest::newRow( "CircularString one vertex 0" ) << QStringLiteral( "CircularString( 1 2)" ) << 0 << QStringLiteral( "CircularString (1 2)" ) << QStringLiteral( "CircularString EMPTY" );
  QTest::newRow( "CircularString one vertex 1 " ) << QStringLiteral( "CircularString( 1 2)" ) << 1 << QStringLiteral( "CircularString (1 2)" ) << QStringLiteral( "CircularString EMPTY" );
  QTest::newRow( "CircularString one vertex 2" ) << QStringLiteral( "CircularString( 1 2)" ) << 2 << QStringLiteral( "CircularString (1 2)" ) << QStringLiteral( "CircularString EMPTY" );
  QTest::newRow( "CircularString three 0" ) << QStringLiteral( "CircularString( 1 2, 2 3, 3 4)" ) << 0 << QStringLiteral( "CircularString EMPTY" ) << QStringLiteral( "CircularString (1 2, 2 3, 3 4)" );
  QTest::newRow( "CircularString three 1" ) << QStringLiteral( "CircularString( 1 2, 2 3, 3 4)" ) << 1 << QStringLiteral( "CircularString (1 2, 2 3)" ) << QStringLiteral( "CircularString (2 3, 3 4)" );
  QTest::newRow( "CircularString three 2" ) << QStringLiteral( "CircularString( 1 2, 2 3, 3 4)" ) << 2 << QStringLiteral( "CircularString (1 2, 2 3, 3 4)" ) << QStringLiteral( "CircularString EMPTY" );
  QTest::newRow( "CircularString three 3" ) << QStringLiteral( "CircularString( 1 2, 2 3, 3 4)" ) << 3 << QStringLiteral( "CircularString (1 2, 2 3, 3 4)" ) << QStringLiteral( "CircularString EMPTY" );
  QTest::newRow( "CircularString five 0" ) << QStringLiteral( "CircularString( 1 2, 2 3, 3 4, 5 6, 7 8)" ) << 0 << QStringLiteral( "CircularString EMPTY" ) << QStringLiteral( "CircularString (1 2, 2 3, 3 4, 5 6, 7 8)" );
  QTest::newRow( "CircularString five 1" ) << QStringLiteral( "CircularString( 1 2, 2 3, 3 4, 5 6, 7 8)" ) << 1 << QStringLiteral( "CircularString (1 2, 2 3)" ) << QStringLiteral( "CircularString (2 3, 3 4, 5 6, 7 8)" );
  QTest::newRow( "CircularString five 2" ) << QStringLiteral( "CircularString( 1 2, 2 3, 3 4, 5 6, 7 8)" ) << 2 << QStringLiteral( "CircularString (1 2, 2 3, 3 4)" ) << QStringLiteral( "CircularString (3 4, 5 6, 7 8)" );
  QTest::newRow( "CircularString five 3" ) << QStringLiteral( "CircularString( 1 2, 2 3, 3 4, 5 6, 7 8)" ) << 3 << QStringLiteral( "CircularString (1 2, 2 3, 3 4, 5 6)" ) << QStringLiteral( "CircularString (5 6, 7 8)" );
  QTest::newRow( "CircularString five 4" ) << QStringLiteral( "CircularString( 1 2, 2 3, 3 4, 5 6, 7 8)" ) << 4 << QStringLiteral( "CircularString (1 2, 2 3, 3 4, 5 6, 7 8)" ) << QStringLiteral( "CircularString EMPTY" );
  QTest::newRow( "CircularStringz" ) << QStringLiteral( "CircularStringZ( 1 2 11, 2 3 12, 3 4 13, 5 6 14, 7 8 15)" ) << 1 << QStringLiteral( "CircularStringZ (1 2 11, 2 3 12)" ) << QStringLiteral( "CircularStringZ (2 3 12, 3 4 13, 5 6 14, 7 8 15)" );
  QTest::newRow( "CircularStringm" ) << QStringLiteral( "CircularStringM( 1 2 11, 2 3 12, 3 4 13, 5 6 14, 7 8 15)" ) << 1 << QStringLiteral( "CircularStringM (1 2 11, 2 3 12)" ) << QStringLiteral( "CircularStringM (2 3 12, 3 4 13, 5 6 14, 7 8 15)" );
  QTest::newRow( "CircularStringzm" ) << QStringLiteral( "CircularStringZM( 1 2 11 21, 2 3 12 22, 3 4 13 23, 5 6 14 24, 7 8 15 25)" ) << 1 << QStringLiteral( "CircularStringZM (1 2 11 21, 2 3 12 22)" ) << QStringLiteral( "CircularStringZM (2 3 12 22, 3 4 13 23, 5 6 14 24, 7 8 15 25)" );

  QTest::newRow( "CompoundCurve empty" ) << QStringLiteral( "CompoundCurve()" ) << 0 << QStringLiteral( "CompoundCurve EMPTY" ) << QStringLiteral( "CompoundCurve EMPTY" );
  QTest::newRow( "CompoundCurve 0" ) << QStringLiteral( "CompoundCurve((1 2, 3 4, 5 6),CircularString(5 6, 7 8, 9 10, 11 12, 13 14))" ) << 0
                                     << QStringLiteral( "CompoundCurve EMPTY" ) << QStringLiteral( "CompoundCurve ((1 2, 3 4, 5 6),CircularString (5 6, 7 8, 9 10, 11 12, 13 14))" );
  QTest::newRow( "CompoundCurve 1" ) << QStringLiteral( "CompoundCurve((1 2, 3 4, 5 6),CircularString(5 6, 7 8, 9 10, 11 12, 13 14))" ) << 1
                                     << QStringLiteral( "CompoundCurve ((1 2, 3 4))" ) << QStringLiteral( "CompoundCurve ((3 4, 5 6),CircularString (5 6, 7 8, 9 10, 11 12, 13 14))" );
  QTest::newRow( "CompoundCurve 2" ) << QStringLiteral( "CompoundCurve((1 2, 3 4, 5 6),CircularString(5 6, 7 8, 9 10, 11 12, 13 14))" ) << 2
                                     << QStringLiteral( "CompoundCurve ((1 2, 3 4, 5 6))" ) << QStringLiteral( "CompoundCurve (CircularString (5 6, 7 8, 9 10, 11 12, 13 14))" );
  QTest::newRow( "CompoundCurve 4" ) << QStringLiteral( "CompoundCurve((1 2, 3 4, 5 6),CircularString(5 6, 7 8, 9 10, 11 12, 13 14))" ) << 4
                                     << QStringLiteral( "CompoundCurve ((1 2, 3 4, 5 6),CircularString (5 6, 7 8, 9 10))" ) << QStringLiteral( "CompoundCurve (CircularString (9 10, 11 12, 13 14))" );
  QTest::newRow( "CompoundCurve 6" ) << QStringLiteral( "CompoundCurve((1 2, 3 4, 5 6), CircularString(5 6, 7 8, 9 10, 11 12, 13 14))" ) << 6
                                     << QStringLiteral( "CompoundCurve ((1 2, 3 4, 5 6),CircularString (5 6, 7 8, 9 10, 11 12, 13 14))" ) << QStringLiteral( "CompoundCurve EMPTY" );
  QTest::newRow( "CompoundCurve 10" ) << QStringLiteral( "CompoundCurve((1 2, 3 4, 5 6), CircularString(5 6, 7 8, 9 10, 11 12, 13 14))" ) << 10
                                      << QStringLiteral( "CompoundCurve ((1 2, 3 4, 5 6),CircularString (5 6, 7 8, 9 10, 11 12, 13 14))" ) << QStringLiteral( "CompoundCurve EMPTY" );
  QTest::newRow( "CompoundCurve three parts 6" ) << QStringLiteral( "CompoundCurve((1 2, 3 4, 5 6), CircularString(5 6, 7 8, 9 10, 11 12, 13 14), (13 14, 15 16, 17 18))" ) << 6
      << QStringLiteral( "CompoundCurve ((1 2, 3 4, 5 6),CircularString (5 6, 7 8, 9 10, 11 12, 13 14))" ) << QStringLiteral( "CompoundCurve ((13 14, 15 16, 17 18))" );
  QTest::newRow( "CompoundCurve three parts 7" ) << QStringLiteral( "CompoundCurve((1 2, 3 4, 5 6), CircularString(5 6, 7 8, 9 10, 11 12, 13 14), (13 14, 15 16, 17 18))" ) << 7
      << QStringLiteral( "CompoundCurve ((1 2, 3 4, 5 6),CircularString (5 6, 7 8, 9 10, 11 12, 13 14),(13 14, 15 16))" ) << QStringLiteral( "CompoundCurve ((15 16, 17 18))" );
  QTest::newRow( "CompoundCurve three parts 7" ) << QStringLiteral( "CompoundCurve((1 2, 3 4, 5 6), CircularString(5 6, 7 8, 9 10, 11 12, 13 14), (13 14, 15 16, 17 18))" ) << 8
      << QStringLiteral( "CompoundCurve ((1 2, 3 4, 5 6),CircularString (5 6, 7 8, 9 10, 11 12, 13 14),(13 14, 15 16, 17 18))" ) << QStringLiteral( "CompoundCurve EMPTY" );
  QTest::newRow( "CompoundCurve three parts 1" ) << QStringLiteral( "CompoundCurve((1 2, 3 4, 5 6), CircularString(5 6, 7 8, 9 10, 11 12, 13 14), (13 14, 15 16, 17 18))" ) << 1
      << QStringLiteral( "CompoundCurve ((1 2, 3 4))" ) << QStringLiteral( "CompoundCurve ((3 4, 5 6),CircularString (5 6, 7 8, 9 10, 11 12, 13 14),(13 14, 15 16, 17 18))" );
}

void TestQgsGeometry::splitCurve()
{
  QFETCH( QString, wkt );
  QFETCH( int, vertex );
  QFETCH( QString, curve1 );
  QFETCH( QString, curve2 );

  QgsGeometry curve( QgsGeometry::fromWkt( wkt ) );
  auto [p1, p2] = qgsgeometry_cast< const QgsCurve * >( curve.constGet() )->splitCurveAtVertex( vertex );
  QCOMPARE( p1->asWkt(), curve1 );
  QCOMPARE( p2->asWkt(), curve2 );
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
  QgsPointXY point1 = QgsPointXY( 20.0, 20.0 );
  QgsPointXY point2 = QgsPointXY( 80.0, 20.0 );
  QgsPointXY point3 = QgsPointXY( 80.0, 80.0 );
  QgsPointXY point4 = QgsPointXY( 20.0, 80.0 );

  QgsGeometry polygonGeometryA = QgsGeometry::fromPolygonXY( QgsPolygonXY() << ( QgsPolylineXY() << point1 << point2 << point3 << point4 << point1 ) );

  //test polygon
  QPolygonF fromPoly = polygonGeometryA.asQPolygonF();
  QVERIFY( fromPoly.isClosed() );
  QCOMPARE( fromPoly.size(), 5 );
  QCOMPARE( fromPoly.at( 0 ).x(), point1.x() );
  QCOMPARE( fromPoly.at( 0 ).y(), point1.y() );
  QCOMPARE( fromPoly.at( 1 ).x(), point2.x() );
  QCOMPARE( fromPoly.at( 1 ).y(), point2.y() );
  QCOMPARE( fromPoly.at( 2 ).x(), point3.x() );
  QCOMPARE( fromPoly.at( 2 ).y(), point3.y() );
  QCOMPARE( fromPoly.at( 3 ).x(), point4.x() );
  QCOMPARE( fromPoly.at( 3 ).y(), point4.y() );
  QCOMPARE( fromPoly.at( 4 ).x(), point1.x() );
  QCOMPARE( fromPoly.at( 4 ).y(), point1.y() );

  //test polyline
  QgsPolylineXY testline;
  testline << point1 << point2 << point3;
  QgsGeometry lineGeom( QgsGeometry::fromPolylineXY( testline ) );
  QPolygonF fromLine = lineGeom.asQPolygonF();
  QVERIFY( !fromLine.isClosed() );
  QCOMPARE( fromLine.size(), 3 );
  QCOMPARE( fromLine.at( 0 ).x(), point1.x() );
  QCOMPARE( fromLine.at( 0 ).y(), point1.y() );
  QCOMPARE( fromLine.at( 1 ).x(), point2.x() );
  QCOMPARE( fromLine.at( 1 ).y(), point2.y() );
  QCOMPARE( fromLine.at( 2 ).x(), point3.x() );
  QCOMPARE( fromLine.at( 2 ).y(), point3.y() );

  //test a bad geometry
  QgsGeometry badGeom( QgsGeometry::fromPointXY( point1 ) );
  QPolygonF fromBad = badGeom.asQPolygonF();
  QVERIFY( fromBad.isEmpty() );

  // test a multipolygon
  QPolygonF res = QgsGeometry::fromWkt( QStringLiteral( "MultiPolygon (((0 0, 10 0, 10 10, 0 10, 0 0 )),((2 2, 4 2, 4 4, 2 4, 2 2)))" ) ).asQPolygonF();
  QVERIFY( res.isClosed() );
  QCOMPARE( res.size(), 5 );
  QCOMPARE( res.at( 0 ).x(), 0.0 );
  QCOMPARE( res.at( 0 ).y(), 0.0 );
  QCOMPARE( res.at( 1 ).x(), 10.0 );
  QCOMPARE( res.at( 1 ).y(), 0.0 );
  QCOMPARE( res.at( 2 ).x(), 10.0 );
  QCOMPARE( res.at( 2 ).y(), 10.0 );
  QCOMPARE( res.at( 3 ).x(), 0.0 );
  QCOMPARE( res.at( 3 ).y(), 10.0 );
  QCOMPARE( res.at( 4 ).x(), 0.0 );
  QCOMPARE( res.at( 4 ).y(), 0.0 );

  // test a multilinestring
  res = QgsGeometry::fromWkt( QStringLiteral( "MultiLineString((0 0, 10 0, 10 10, 0 10 ),(2 2, 4 2, 4 4, 2 4))" ) ).asQPolygonF();
  QVERIFY( !res.isClosed() );
  QCOMPARE( res.size(), 4 );
  QCOMPARE( res.at( 0 ).x(), 0.0 );
  QCOMPARE( res.at( 0 ).y(), 0.0 );
  QCOMPARE( res.at( 1 ).x(), 10.0 );
  QCOMPARE( res.at( 1 ).y(), 0.0 );
  QCOMPARE( res.at( 2 ).x(), 10.0 );
  QCOMPARE( res.at( 2 ).y(), 10.0 );
  QCOMPARE( res.at( 3 ).x(), 0.0 );
  QCOMPARE( res.at( 3 ).y(), 10.0 );
}

void TestQgsGeometry::comparePolylines()
{
  QgsPointXY point1 = QgsPointXY( 20.0, 20.0 );
  QgsPointXY point2 = QgsPointXY( 80.0, 20.0 );
  QgsPointXY point3 = QgsPointXY( 80.0, 80.0 );
  QgsPointXY point4 = QgsPointXY( 20.0, 80.0 );
  QgsPointXY pointA = QgsPointXY( 40.0, 40.0 );

  QgsPolylineXY line1;
  line1 << point1 << point2 << point3;
  QgsPolylineXY line2;
  line2 << point1 << point2 << point3;
  QVERIFY( QgsGeometry::compare( line1, line2 ) );

  //different number of nodes
  QgsPolylineXY line3;
  line3 << point1 << point2 << point3 << point4;
  QVERIFY( !QgsGeometry::compare( line1, line3 ) );

  //different nodes
  QgsPolylineXY line4;
  line3 << point1 << pointA << point3 << point4;
  QVERIFY( !QgsGeometry::compare( line3, line4 ) );
}

void TestQgsGeometry::comparePolygons()
{
  QgsPointXY point1 = QgsPointXY( 20.0, 20.0 );
  QgsPointXY point2 = QgsPointXY( 80.0, 20.0 );
  QgsPointXY point3 = QgsPointXY( 80.0, 80.0 );
  QgsPointXY point4 = QgsPointXY( 20.0, 80.0 );
  QgsPointXY pointA = QgsPointXY( 40.0, 40.0 );
  QgsPointXY pointB = QgsPointXY( 100.0, 40.0 );

  QgsPolylineXY ring1;
  ring1 << point1 << point2 << point3 << point1;
  QgsPolylineXY ring2;
  ring2 << point4 << pointA << pointB << point4;
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

#if defined(__clang__) || defined(__GNUG__)
#include <cxxabi.h>
#include <ctime>

QString generateSpacesString( int numberOfSpace )
{
  QStringList spaces;
  spaces << QString( " " ) << QString( "\t" ) << QString( "\n" ) << QString( "\r" );

  QString ret;
  for ( int i = 0; i < numberOfSpace; ++i )
  {
    int r = rand() % ( spaces.count() );
    ret += spaces.at( r );
  }
  return ret;
}

#endif
// Helper function (in anonymous namespace to prevent possible link with the extirior)
namespace
{
  template<typename T>
  inline void testCreateEmptyWithSameType()
  {
    std::unique_ptr<QgsAbstractGeometry> geom { new T() };
    std::unique_ptr<QgsAbstractGeometry> created { TestQgsGeometry::createEmpty( geom.get() ) };
    QVERIFY( created->isEmpty() );
#if defined(__clang__) || defined(__GNUG__)
    srand( ( unsigned )time( NULL ) );

    const std::type_info  &ti = typeid( T );
    int status;
    char *realname = abi::__cxa_demangle( ti.name(), 0, 0, &status );

    QString type = realname;
// remove Qgs prefix
    type = type.right( type.count() - 3 );
    QStringList extensionZM;
    extensionZM << QString() << QString( "Z" ) << QString( "M" ) << QString( "ZM" );
    for ( const QString &ext : extensionZM )
    {
      QString wkt = type + ext;
      QString result = wkt + QLatin1String( " EMPTY" );

      QStringList emptyStringList;
      emptyStringList << QString( "EMPTY" ) << QString( "Empty" ) << QString( "empty" ) << QString( "EmptY" ) << QString( "EmPtY" );
      for ( int i = 0 ; i < 10 ; ++i )
      {
        QString spacesBefore = generateSpacesString( i );
        QString spacesMiddle = i == 0 ? QStringLiteral( " " ) : generateSpacesString( i );
        QString spacesAfter = generateSpacesString( i );
        for ( int j = 0; j < emptyStringList.count() ; ++j )
        {
          QString generatedWkt = spacesBefore + wkt + spacesMiddle + emptyStringList.at( j ) + spacesAfter;

          QgsGeometry gWkt = QgsGeometry::fromWkt( generatedWkt );
          QVERIFY( gWkt.asWkt().compare( result, Qt::CaseInsensitive ) == 0 );

          QVERIFY( geom->fromWkt( generatedWkt ) );
          QVERIFY( geom->asWkt().compare( result, Qt::CaseInsensitive ) == 0 );
        }
      }
    }
    free( realname );
#endif


    // Check that it is the correct type
    QVERIFY( static_cast<T *>( created.get() ) != nullptr );
  }
}

void TestQgsGeometry::createEmptyWithSameType()
{
  testCreateEmptyWithSameType<QgsCircularString>();
  testCreateEmptyWithSameType<QgsCompoundCurve>();
  testCreateEmptyWithSameType<QgsLineString>();
  testCreateEmptyWithSameType<QgsGeometryCollection>();
  testCreateEmptyWithSameType<QgsMultiCurve>();
  testCreateEmptyWithSameType<QgsMultiLineString>();
  testCreateEmptyWithSameType<QgsMultiPoint>();
  testCreateEmptyWithSameType<QgsMultiSurface>();
  testCreateEmptyWithSameType<QgsPoint>();
  testCreateEmptyWithSameType<QgsCurvePolygon>();
  testCreateEmptyWithSameType<QgsPolygon>();
  testCreateEmptyWithSameType<QgsTriangle>();
}

void TestQgsGeometry::compareTo_data()
{
  QTest::addColumn<QString>( "geom1" );
  QTest::addColumn<QString>( "geom2" );
  QTest::addColumn<int>( "expected" );

  QTest::newRow( "point vs multipoint" ) << QStringLiteral( "POINT( 1 2 )" ) << QStringLiteral( "MULTIPOINT((1 2))" ) << -1;
  QTest::newRow( "multipoint vs point" ) << QStringLiteral( "MULTIPOINT((1 2))" ) << QStringLiteral( "POINT( 1 2 )" ) << 1;
  QTest::newRow( "empty point vs empty point" ) << QStringLiteral( "POINT EMPTY" ) << QStringLiteral( "POINT EMPTY" ) << 0;
  QTest::newRow( "empty point vs non-empty point" ) << QStringLiteral( "POINT EMPTY" ) << QStringLiteral( "POINT (1 2)" ) << -1;
  QTest::newRow( "non-empty point vs empty point" ) << QStringLiteral( "POINT (1 2)" ) << QStringLiteral( "POINT EMPTY" ) << 1;
  QTest::newRow( "point vs point" ) << QStringLiteral( "POINT( 1 2 )" ) << QStringLiteral( "POINT(1 2)" ) << 0;
  QTest::newRow( "point vs point greater x" ) << QStringLiteral( "POINT( 2 2 )" ) << QStringLiteral( "POINT(1 2)" ) << 1;
  QTest::newRow( "point vs point lesser x" ) << QStringLiteral( "POINT( 0 2 )" ) << QStringLiteral( "POINT(1 2)" ) << -1;
  QTest::newRow( "point vs point greater y" ) << QStringLiteral( "POINT( 1 3 )" ) << QStringLiteral( "POINT(1 2)" ) << 1;
  QTest::newRow( "point vs point lesser y" ) << QStringLiteral( "POINT( 1 1 )" ) << QStringLiteral( "POINT(1 2)" ) << -1;
  QTest::newRow( "point vs point first with z" ) << QStringLiteral( "POINTZ( 2 1 3 )" ) << QStringLiteral( "POINT(2 1)" ) << 1;
  QTest::newRow( "point vs point second with z" ) << QStringLiteral( "POINT( 2 1 )" ) << QStringLiteral( "POINTZ(2 1 3)" ) << -1;
  QTest::newRow( "point vs point equal z" ) << QStringLiteral( "POINTZ( 2 1 3 )" ) << QStringLiteral( "POINTZ(2 1 3)" ) << 0;
  QTest::newRow( "point vs point greater z" ) << QStringLiteral( "POINTZ( 2 1 4 )" ) << QStringLiteral( "POINTZ(2 1 3)" ) << 1;
  QTest::newRow( "point vs point lower z" ) << QStringLiteral( "POINTZ(2 1 2)" ) << QStringLiteral( "POINTZ(2 1 3)" ) << -1;
  QTest::newRow( "point vs point first with m" ) << QStringLiteral( "POINTM( 2 1 3 )" ) << QStringLiteral( "POINT(2 1)" ) << 1;
  QTest::newRow( "point vs point second with m" ) << QStringLiteral( "POINT( 2 1 )" ) << QStringLiteral( "POINTM(2 1 3)" ) << -1;
  QTest::newRow( "point vs point equal m" ) << QStringLiteral( "POINTM( 2 1 3 )" ) << QStringLiteral( "POINTM(2 1 3)" ) << 0;
  QTest::newRow( "point vs point greater m" ) << QStringLiteral( "POINTM( 2 1 4 )" ) << QStringLiteral( "POINTM(2 1 3)" ) << 1;
  QTest::newRow( "point vs point lower m" ) << QStringLiteral( "POINTM(2 1 2)" ) << QStringLiteral( "POINTM(2 1 3)" ) << -1;
  QTest::newRow( "point vs point first with z second with m" ) << QStringLiteral( "POINTZ( 2 1 3 )" ) << QStringLiteral( "POINTM(2 1 3)" ) << 1;
  QTest::newRow( "point vs point first with m second with z" ) << QStringLiteral( "POINTM( 2 1 3 )" ) << QStringLiteral( "POINTZ(2 1 3)" ) << -1;
  QTest::newRow( "point vs point zm" ) << QStringLiteral( "POINTZM( 2 1 3 4 )" ) << QStringLiteral( "POINTZM(2 1 4 4)" ) << -1;
  QTest::newRow( "point vs point zm second" ) << QStringLiteral( "POINTZM( 2 1 3 4 )" ) << QStringLiteral( "POINTM(2 1 3 5)" ) << -1;
  QTest::newRow( "linestring empty vs polygon empty" ) << QStringLiteral( "LINESTRING()" ) << QStringLiteral( "POLYGON(())" ) << -1;
  QTest::newRow( "polygon empty vs linesting empty" ) << QStringLiteral( "POLYGON(())" ) << QStringLiteral( "LINESTRING(())" ) << 1;
  QTest::newRow( "linestring empty vs non empty" ) << QStringLiteral( "LINESTRING()" ) << QStringLiteral( "LINESTRING(1 1, 2 2)" ) << -1;
  QTest::newRow( "linestring non empty vs empty" ) << QStringLiteral( "LINESTRING( 1 1, 2 2)" ) << QStringLiteral( "LINESTRING()" ) << 1;
  QTest::newRow( "linestring empty vs empty" ) << QStringLiteral( "LINESTRING()" ) << QStringLiteral( "LINESTRING()" ) << 0;
  QTest::newRow( "linestring equal" ) << QStringLiteral( "LINESTRING( 1 1, 2 2)" ) << QStringLiteral( "LINESTRING( 1 1, 2 2)" ) << 0;
  QTest::newRow( "linestring longer vs shorter" ) << QStringLiteral( "LINESTRING( 1 1, 2 2, 3 3)" ) << QStringLiteral( "LINESTRING( 1 1, 2 2)" ) << 1;
  QTest::newRow( "linestring shorter vs longer" ) << QStringLiteral( "LINESTRING( 1 1, 2 2)" ) << QStringLiteral( "LINESTRING( 1 1, 2 2, 3 3)" ) << -1;
  QTest::newRow( "linestring x less" ) << QStringLiteral( "LINESTRING( 1 1, 1 2)" ) << QStringLiteral( "LINESTRING( 1 1, 2 2)" ) << -1;
  QTest::newRow( "linestring x greater" ) << QStringLiteral( "LINESTRING( 1 1, 3 2)" ) << QStringLiteral( "LINESTRING( 1 1, 2 2)" ) << 1;
  QTest::newRow( "linestring y less" ) << QStringLiteral( "LINESTRING( 1 1, 1 0)" ) << QStringLiteral( "LINESTRING( 1 1, 1 2)" ) << -1;
  QTest::newRow( "linestring y greater" ) << QStringLiteral( "LINESTRING( 1 1, 3 2)" ) << QStringLiteral( "LINESTRING( 1 1, 3 1)" ) << 1;
  QTest::newRow( "linestring z vs no z" ) << QStringLiteral( "LINESTRINGZ( 1 1 3, 2 2 4)" ) << QStringLiteral( "LINESTRING( 1 1, 2 2)" ) << 1;
  QTest::newRow( "linestring no z vs z" ) << QStringLiteral( "LINESTRING( 1 1, 2 2)" ) << QStringLiteral( "LINESTRINGZ( 1 1 3, 2 2 4)" ) << -1;
  QTest::newRow( "linestring z vs z" ) << QStringLiteral( "LINESTRINGZ( 1 1 3, 2 2 4)" ) << QStringLiteral( "LINESTRINGZ( 1 1 3, 2 2 4)" ) << 0;
  QTest::newRow( "linestring z less" ) << QStringLiteral( "LINESTRINGZ( 1 1 3, 2 2 0)" ) << QStringLiteral( "LINESTRINGZ( 1 1 3, 2 2 4)" ) << -1;
  QTest::newRow( "linestring z greater" ) << QStringLiteral( "LINESTRINGZ( 1 1 3, 2 2 6)" ) << QStringLiteral( "LINESTRINGZ( 1 1 3, 2 2 4)" ) << 1;
  QTest::newRow( "linestring m vs no m" ) << QStringLiteral( "LINESTRINGM( 1 1 3, 2 2 4)" ) << QStringLiteral( "LINESTRING( 1 1, 2 2)" ) << 1;
  QTest::newRow( "linestring no m vs m" ) << QStringLiteral( "LINESTRING( 1 1, 2 2)" ) << QStringLiteral( "LINESTRINGM( 1 1 3, 2 2 4)" ) << -1;
  QTest::newRow( "linestring m vs m" ) << QStringLiteral( "LINESTRINGM( 1 1 3, 2 2 4)" ) << QStringLiteral( "LINESTRINGM( 1 1 3, 2 2 4)" ) << 0;
  QTest::newRow( "linestring m less" ) << QStringLiteral( "LINESTRINGM( 1 1 3, 2 2 0)" ) << QStringLiteral( "LINESTRINGM( 1 1 3, 2 2 4)" ) << -1;
  QTest::newRow( "linestring m greater" ) << QStringLiteral( "LINESTRINGM( 1 1 3, 2 2 6)" ) << QStringLiteral( "LINESTRINGM( 1 1 3, 2 2 4)" ) << 1;
  QTest::newRow( "linestring first with z vs second with m" ) << QStringLiteral( "LINESTRINGZ( 1 1 3, 2 2 4)" ) << QStringLiteral( "LINESTRINGM( 1 1 3, 2 2 4)" ) << 1;
  QTest::newRow( "linestring first with m vs second with z" ) << QStringLiteral( "LINESTRINGM( 1 1 3, 2 2 4)" ) << QStringLiteral( "LINESTRINGZ( 1 1 3, 2 2 4)" ) << -1;
  QTest::newRow( "linestring zm" ) << QStringLiteral( "LINESTRINGZM( 1 1 3 5, 2 2 4 6)" ) << QStringLiteral( "LINESTRINGZM( 1 1 3 5, 2 2 4 6)" ) << 0;
  QTest::newRow( "linestring zm second" ) << QStringLiteral( "LINESTRINGZM( 1 1 3 5, 2 2 4 5)" ) << QStringLiteral( "LINESTRINGZM( 1 1 3 5, 2 2 4 6)" ) << -1;
  QTest::newRow( "linestring zm third" ) << QStringLiteral( "LINESTRINGZM( 1 1 3 5, 2 2 4 7)" ) << QStringLiteral( "LINESTRINGZM( 1 1 3 5, 2 2 4 3)" ) << 1;
  QTest::newRow( "linestring vs circular string" ) << QStringLiteral( "LINESTRING( 1 1, 2 1, 2 3)" ) << QStringLiteral( "CIRCULARSTRING( 1 1, 2 1, 2 3)" ) << -1;
  QTest::newRow( "circular string vs linestring" ) << QStringLiteral( "CIRCULARSTRING( 1 1, 2 1, 2 3)" )  << QStringLiteral( "LINESTRING( 1 1, 2 1, 2 3)" ) << 1;
  QTest::newRow( "circular string empty vs non empty" ) << QStringLiteral( "CIRCULARSTRING()" ) << QStringLiteral( "CIRCULARSTRING(1 1, 2 2, 2 3)" ) << -1;
  QTest::newRow( "circular string non empty vs empty" ) << QStringLiteral( "CIRCULARSTRING( 1 1, 2 2, 2 3)" ) << QStringLiteral( "CIRCULARSTRING()" ) << 1;
  QTest::newRow( "circular string empty vs empty" ) << QStringLiteral( "CIRCULARSTRING()" ) << QStringLiteral( "CIRCULARSTRING()" ) << 0;
  QTest::newRow( "circular string equal" ) << QStringLiteral( "CIRCULARSTRING( 1 1, 2 2, 2 3)" ) << QStringLiteral( "CIRCULARSTRING( 1 1, 2 2, 2 3)" ) << 0;
  QTest::newRow( "circular string longer vs shorter" ) << QStringLiteral( "CIRCULARSTRING( 1 1, 2 2, 2 3, 4 3, 4 4)" ) << QStringLiteral( "CIRCULARSTRING( 1 1, 2 2, 2 3)" ) << 1;
  QTest::newRow( "circular string shorter vs longer" ) << QStringLiteral( "CIRCULARSTRING( 1 1, 2 2, 2 3)" ) << QStringLiteral( "CIRCULARSTRING( 1 1, 2 2, 2 3, 4 3, 4 4)" ) << -1;
  QTest::newRow( "circular string x less" ) << QStringLiteral( "CIRCULARSTRING( 1 1, 1 2, 2 2)" ) << QStringLiteral( "CIRCULARSTRING( 1 1, 1 2, 4 2)" ) << -1;
  QTest::newRow( "circular string x greater" ) << QStringLiteral( "CIRCULARSTRING( 1 1, 1 2, 4 2)" ) << QStringLiteral( "CIRCULARSTRING( 1 1, 1 2, 2 2)" ) << 1;
  QTest::newRow( "circular string y less" ) << QStringLiteral( "CIRCULARSTRING( 1 1, 1 0, 2 1)" ) << QStringLiteral( "CIRCULARSTRING( 1 1, 1 2, 2 1)" ) << -1;
  QTest::newRow( "circular string y greater" ) << QStringLiteral( "CIRCULARSTRING( 1 1, 3 2, 2 1)" ) << QStringLiteral( "CIRCULARSTRING( 1 1, 3 1, 2 1)" ) << 1;
  QTest::newRow( "circular string z vs no z" ) << QStringLiteral( "CIRCULARSTRINGZ( 1 1 3, 2 2 4, 2 3 4)" ) << QStringLiteral( "CIRCULARSTRING( 1 1, 2 2, 2 3)" ) << 1;
  QTest::newRow( "circular string no z vs z" ) << QStringLiteral( "CIRCULARSTRING( 1 1, 2 2, 2 3)" ) << QStringLiteral( "CIRCULARSTRINGZ( 1 1 3, 2 2 4, 2 3 4)" ) << -1;
  QTest::newRow( "circular string z vs z" ) << QStringLiteral( "CIRCULARSTRINGZ( 1 1 3, 2 2 4, 2 3 5)" ) << QStringLiteral( "CIRCULARSTRINGZ( 1 1 3, 2 2 4, 2 3 5)" ) << 0;
  QTest::newRow( "circular string z less" ) << QStringLiteral( "CIRCULARSTRINGZ( 1 1 3, 2 2 0, 2 3 5)" ) << QStringLiteral( "CIRCULARSTRINGZ( 1 1 3, 2 2 4, 2 3 5)" ) << -1;
  QTest::newRow( "circular string z greater" ) << QStringLiteral( "CIRCULARSTRINGZ( 1 1 3, 2 2 6, 2 3 5)" ) << QStringLiteral( "CIRCULARSTRINGZ( 1 1 3, 2 2 4, 2 3 5)" ) << 1;
  QTest::newRow( "circular string m vs no m" ) << QStringLiteral( "CIRCULARSTRINGM( 1 1 3, 2 2 4, 2 3 5)" ) << QStringLiteral( "CIRCULARSTRING( 1 1, 2 2, 2 3)" ) << 1;
  QTest::newRow( "circular string no m vs m" ) << QStringLiteral( "CIRCULARSTRING( 1 1, 2 2, 2 3)" ) << QStringLiteral( "CIRCULARSTRINGM( 1 1 3, 2 2 4, 2 3 5)" ) << -1;
  QTest::newRow( "circular string m vs m" ) << QStringLiteral( "CIRCULARSTRINGM( 1 1 3, 2 2 4, 2 3 5)" ) << QStringLiteral( "CIRCULARSTRINGM( 1 1 3, 2 2 4, 2 3 5)" ) << 0;
  QTest::newRow( "circular string m less" ) << QStringLiteral( "CIRCULARSTRINGM( 1 1 3, 2 2 0, 2 3 5)" ) << QStringLiteral( "CIRCULARSTRINGM( 1 1 3, 2 2 4, 2 3 5)" ) << -1;
  QTest::newRow( "circular string m greater" ) << QStringLiteral( "CIRCULARSTRINGM( 1 1 3, 2 2 6, 2 3 5)" ) << QStringLiteral( "CIRCULARSTRINGM( 1 1 3, 2 2 4, 2 3 5)" ) << 1;
  QTest::newRow( "circular string first with z vs second with m" ) << QStringLiteral( "CIRCULARSTRINGZ( 1 1 3, 2 2 4, 2 3 5)" ) << QStringLiteral( "CIRCULARSTRINGM( 1 1 3, 2 2 4, 2 3 5)" ) << 1;
  QTest::newRow( "circular string first with m vs second with z" ) << QStringLiteral( "CIRCULARSTRINGM( 1 1 3, 2 2 4, 2 3 5)" ) << QStringLiteral( "CIRCULARSTRINGZ( 1 1 3, 2 2 4, 2 3 5)" ) << -1;
  QTest::newRow( "circular string zm" ) << QStringLiteral( "CIRCULARSTRINGZM( 1 1 3 5, 2 2 4 6, 2 3 5 6)" ) << QStringLiteral( "CIRCULARSTRINGZM( 1 1 3 5, 2 2 4 6, 2 3 5 6)" ) << 0;
  QTest::newRow( "circular string zm second" ) << QStringLiteral( "CIRCULARSTRINGZM( 1 1 3 5, 2 2 4 5, 2 3 5 6)" ) << QStringLiteral( "CIRCULARSTRINGZM( 1 1 3 5, 2 2 4 6, 2 3 5 6)" ) << -1;
  QTest::newRow( "circular string zm third" ) << QStringLiteral( "CIRCULARSTRINGZM( 1 1 3 5, 2 2 4 7, 2 3 5 6)" ) << QStringLiteral( "CIRCULARSTRINGZM( 1 1 3 5, 2 2 4 3, 2 3 5 6)" ) << 1;

  QTest::newRow( "linestring vs compound curve" ) << QStringLiteral( "LINESTRING( 1 1, 2 1, 2 3)" ) << QStringLiteral( "COMPOUNDCURVE((1 1, 2 1, 2 3))" ) << -1;
  QTest::newRow( "compound curve vs linestring" ) << QStringLiteral( "COMPOUNDCURVE(( 1 1, 2 1, 2 3 ))" )  << QStringLiteral( "LINESTRING( 1 1, 2 1, 2 3)" ) << 1;
  QTest::newRow( "compound curve empty vs non empty" ) << QStringLiteral( "COMPOUNDCURVE()" ) << QStringLiteral( "COMPOUNDCURVE((1 1, 2 2, 2 3))" ) << -1;
  QTest::newRow( "compound curve non empty vs empty" ) << QStringLiteral( "COMPOUNDCURVE( (1 1, 2 2, 2 3))" ) << QStringLiteral( "COMPOUNDCURVE()" ) << 1;
  QTest::newRow( "compound curve empty vs empty" ) << QStringLiteral( "COMPOUNDCURVE()" ) << QStringLiteral( "COMPOUNDCURVE()" ) << 0;
  QTest::newRow( "compound curve equal" ) << QStringLiteral( "COMPOUNDCURVE((1 1, 2 2, 2 3))" ) << QStringLiteral( "COMPOUNDCURVE((1 1, 2 2, 2 3))" ) << 0;
  QTest::newRow( "compound curve longer vs shorter" ) << QStringLiteral( "COMPOUNDCURVE(( 1 1, 2 2, 2 3, 4 3, 4 4))" ) << QStringLiteral( "COMPOUNDCURVE((1 1, 2 2, 2 3))" ) << 1;
  QTest::newRow( "compound curve shorter vs longer" ) << QStringLiteral( "COMPOUNDCURVE(( 1 1, 2 2, 2 3))" ) << QStringLiteral( "COMPOUNDCURVE(( 1 1, 2 2, 2 3, 4 3, 4 4))" ) << -1;
  QTest::newRow( "compound curve x less" ) << QStringLiteral( "COMPOUNDCURVE(( 1 1, 1 2, 2 2))" ) << QStringLiteral( "COMPOUNDCURVE(( 1 1, 1 2, 4 2))" ) << -1;
  QTest::newRow( "compound curve x greater" ) << QStringLiteral( "COMPOUNDCURVE(( 1 1, 1 2, 4 2))" ) << QStringLiteral( "COMPOUNDCURVE(( 1 1, 1 2, 2 2))" ) << 1;
  QTest::newRow( "compound curve y less" ) << QStringLiteral( "COMPOUNDCURVE(( 1 1, 1 0, 2 1))" ) << QStringLiteral( "COMPOUNDCURVE(( 1 1, 1 2, 2 1))" ) << -1;
  QTest::newRow( "compound curve y greater" ) << QStringLiteral( "COMPOUNDCURVE(( 1 1, 3 2, 2 1))" ) << QStringLiteral( "COMPOUNDCURVE(( 1 1, 3 1, 2 1))" ) << 1;
  QTest::newRow( "compound curve z vs no z" ) << QStringLiteral( "COMPOUNDCURVEZ(( 1 1 3, 2 2 4, 2 3 4))" ) << QStringLiteral( "COMPOUNDCURVE(( 1 1, 2 2, 2 3))" ) << 1;
  QTest::newRow( "compound curve no z vs z" ) << QStringLiteral( "COMPOUNDCURVE(( 1 1, 2 2, 2 3))" ) << QStringLiteral( "COMPOUNDCURVEZ(( 1 1 3, 2 2 4, 2 3 4))" ) << -1;
  QTest::newRow( "compound curve z vs z" ) << QStringLiteral( "COMPOUNDCURVEZ(( 1 1 3, 2 2 4, 2 3 5))" ) << QStringLiteral( "COMPOUNDCURVEZ(( 1 1 3, 2 2 4, 2 3 5))" ) << 0;
  QTest::newRow( "compound curve z less" ) << QStringLiteral( "COMPOUNDCURVEZ(( 1 1 3, 2 2 0, 2 3 5))" ) << QStringLiteral( "COMPOUNDCURVEZ(( 1 1 3, 2 2 4, 2 3 5))" ) << -1;
  QTest::newRow( "compound curve z greater" ) << QStringLiteral( "COMPOUNDCURVEZ(( 1 1 3, 2 2 6, 2 3 5))" ) << QStringLiteral( "COMPOUNDCURVEZ(( 1 1 3, 2 2 4, 2 3 5))" ) << 1;
  QTest::newRow( "compound curve m vs no m" ) << QStringLiteral( "COMPOUNDCURVEM(( 1 1 3, 2 2 4, 2 3 5))" ) << QStringLiteral( "COMPOUNDCURVE(( 1 1, 2 2, 2 3))" ) << 1;
  QTest::newRow( "compound curve no m vs m" ) << QStringLiteral( "COMPOUNDCURVE(( 1 1, 2 2, 2 3))" ) << QStringLiteral( "COMPOUNDCURVEM(( 1 1 3, 2 2 4, 2 3 5))" ) << -1;
  QTest::newRow( "compound curve m vs m" ) << QStringLiteral( "COMPOUNDCURVEM(( 1 1 3, 2 2 4, 2 3 5))" ) << QStringLiteral( "COMPOUNDCURVEM(( 1 1 3, 2 2 4, 2 3 5))" ) << 0;
  QTest::newRow( "compound curve m less" ) << QStringLiteral( "COMPOUNDCURVEM(( 1 1 3, 2 2 0, 2 3 5))" ) << QStringLiteral( "COMPOUNDCURVEM(( 1 1 3, 2 2 4, 2 3 5))" ) << -1;
  QTest::newRow( "compound curve m greater" ) << QStringLiteral( "COMPOUNDCURVEM(( 1 1 3, 2 2 6, 2 3 5))" ) << QStringLiteral( "COMPOUNDCURVEM(( 1 1 3, 2 2 4, 2 3 5))" ) << 1;
  QTest::newRow( "compound curve first with z vs second with m" ) << QStringLiteral( "COMPOUNDCURVEZ(( 1 1 3, 2 2 4, 2 3 5))" ) << QStringLiteral( "COMPOUNDCURVEM(( 1 1 3, 2 2 4, 2 3 5))" ) << 1;
  QTest::newRow( "compound curve first with m vs second with z" ) << QStringLiteral( "COMPOUNDCURVEM(( 1 1 3, 2 2 4, 2 3 5))" ) << QStringLiteral( "COMPOUNDCURVEZ(( 1 1 3, 2 2 4, 2 3 5))" ) << -1;
  QTest::newRow( "compound curve zm" ) << QStringLiteral( "COMPOUNDCURVEZM(( 1 1 3 5, 2 2 4 6, 2 3 5 6))" ) << QStringLiteral( "COMPOUNDCURVEZM(( 1 1 3 5, 2 2 4 6, 2 3 5 6))" ) << 0;
  QTest::newRow( "compound curve zm second" ) << QStringLiteral( "COMPOUNDCURVEZM(( 1 1 3 5, 2 2 4 5, 2 3 5 6))" ) << QStringLiteral( "COMPOUNDCURVEZM(( 1 1 3 5, 2 2 4 6, 2 3 5 6))" ) << -1;
  QTest::newRow( "compound curve zm third" ) << QStringLiteral( "COMPOUNDCURVEZM(( 1 1 3 5, 2 2 4 7, 2 3 5 6))" ) << QStringLiteral( "COMPOUNDCURVEZM(( 1 1 3 5, 2 2 4 3, 2 3 5 6))" ) << 1;

  // same code is used for all CurvePolygon derivates!
  QTest::newRow( "polygon empty vs non empty" ) << QStringLiteral( "POLYGON()" ) << QStringLiteral( "POLYGON((1 1, 2 2, 2 3, 1 1))" ) << -1;
  QTest::newRow( "polygon non empty vs empty" ) << QStringLiteral( "POLYGON( (1 1, 2 2, 2 3, 1 1))" ) << QStringLiteral( "POLYGON()" ) << 1;
  QTest::newRow( "polygon equal" ) << QStringLiteral( "POLYGON( (1 1, 2 2, 2 3, 1 1))" ) << QStringLiteral( "POLYGON((1 1, 2 2, 2 3, 1 1))" ) << 0;
  QTest::newRow( "polygon exterior less" ) << QStringLiteral( "POLYGON( (1 1, 2 2, 2 3, 1 1))" ) << QStringLiteral( "POLYGON((1 10, 2 12, 2 13, 1 10))" ) << -1;
  QTest::newRow( "polygon exterior greater" ) << QStringLiteral( "POLYGON( (1 11, 2 12, 2 13, 1 11))" ) << QStringLiteral( "POLYGON((1 1, 2 1, 2 1, 1 1))" ) << 1;
  QTest::newRow( "polygon less rings" ) << QStringLiteral( "POLYGON( (1 1, 2 2, 2 3, 1 1))" ) << QStringLiteral( "POLYGON((1 1, 2 2, 2 3, 1 1),(1.1 1.1, 1.2 1.1, 1.1 1.2, 1.1 1.1))" ) << -1;
  QTest::newRow( "polygon less equal" ) << QStringLiteral( "POLYGON( (1 1, 2 2, 2 3, 1 1),(1.1 1.1, 1.2 1.1, 1.1 1.2, 1.1 1.1))" ) << QStringLiteral( "POLYGON((1 1, 2 2, 2 3, 1 1),(1.1 1.1, 1.2 1.1, 1.1 1.2, 1.1 1.1))" ) << 0;
  QTest::newRow( "polygon less more" ) << QStringLiteral( "POLYGON( (1 1, 2 2, 2 3, 1 1),(1.1 1.1, 1.2 1.1, 1.1 1.2, 1.1 1.1))" ) << QStringLiteral( "POLYGON((1 1, 2 2, 2 3, 1 1))" ) << 1;
  QTest::newRow( "polygon ring lesser" ) << QStringLiteral( "POLYGON( (1 1, 2 2, 2 3, 1 1),(1.1 1.1, 1.2 1.1, 1.1 1.2, 1.1 1.1))" ) << QStringLiteral( "POLYGON((1 1, 2 2, 2 3, 1 1),(1.1 1.1, 1.2 1.1, 1.1 1.3, 1.1 1.1))" ) << -1;
  QTest::newRow( "polygon ring greater" ) << QStringLiteral( "POLYGON( (1 1, 2 2, 2 3, 1 1),(1.1 1.1, 1.2 1.1, 1.1 1.3, 1.1 1.1))" ) << QStringLiteral( "POLYGON((1 1, 2 2, 2 3, 1 1),(1.1 1.1, 1.2 1.1, 1.1 1.2, 1.1 1.1))" ) << 1;

  // same code is used for all geometry collection derivates!
  QTest::newRow( "collection empty vs non empty" ) << QStringLiteral( "GEOMETRYCOLLECTION()" ) << QStringLiteral( "GEOMETRYCOLLECTION(LINESTRING(1 1, 2 2, 2 3, 1 1))" ) << -1;
  QTest::newRow( "collection non empty vs empty" ) << QStringLiteral( "GEOMETRYCOLLECTION( LINESTRING(1 1, 2 2, 2 3, 1 1))" ) << QStringLiteral( "GEOMETRYCOLLECTION()" ) << 1;
  QTest::newRow( "collection equal" ) << QStringLiteral( "GEOMETRYCOLLECTION( LINESTRING(1 1, 2 2, 2 3, 1 1))" ) << QStringLiteral( "GEOMETRYCOLLECTION(LINESTRING(1 1, 2 2, 2 3, 1 1))" ) << 0;
  QTest::newRow( "collection different types" ) << QStringLiteral( "GEOMETRYCOLLECTION( LINESTRING(1 1, 2 2, 2 3))" ) << QStringLiteral( "GEOMETRYCOLLECTION(CIRCULARSTRING(1 1, 2 2, 2 3))" ) << -1;
  QTest::newRow( "collection same" ) << QStringLiteral( "GEOMETRYCOLLECTION( LINESTRING(1 1, 2 2, 2 3), POINT( 1 3))" ) << QStringLiteral( "GEOMETRYCOLLECTION(LINESTRING(1 1, 2 2, 2 3), POINT( 1 3)" ) << 0;
  QTest::newRow( "collection less members" ) << QStringLiteral( "GEOMETRYCOLLECTION( LINESTRING(1 1, 2 2, 2 3))" ) << QStringLiteral( "GEOMETRYCOLLECTION(LINESTRING(1 1, 2 2, 2 3), POINT( 1 3)" ) << -1;
  QTest::newRow( "collection more members" ) << QStringLiteral( "GEOMETRYCOLLECTION( LINESTRING(1 1, 2 2, 2 3), POINT( 1 3))" ) << QStringLiteral( "GEOMETRYCOLLECTION(POINT( 1 3)" ) << 1;
  QTest::newRow( "collection member less" ) << QStringLiteral( "GEOMETRYCOLLECTION( LINESTRING(1 1, 2 2, 2 3), POINT( 1 3))" ) << QStringLiteral( "GEOMETRYCOLLECTION(LINESTRING(1 1, 2 2, 2 3), POINT( 1 4)" ) << -1;
  QTest::newRow( "collection member greater" ) << QStringLiteral( "GEOMETRYCOLLECTION( LINESTRING(1 1, 2 2, 2 3), POINT( 1 3))" ) << QStringLiteral( "GEOMETRYCOLLECTION(LINESTRING(1 1, 2 2, 2 3), POINT( 1 1)" ) << 1;
}

void TestQgsGeometry::compareTo()
{
  QFETCH( QString, geom1 );
  QFETCH( QString, geom2 );
  QFETCH( int, expected );

  QCOMPARE( QgsGeometry::fromWkt( geom1 ).get()->compareTo( QgsGeometry::fromWkt( geom2 ).get() ), expected );
}

void TestQgsGeometry::scroll_data()
{
  QTest::addColumn<QString>( "curve" );
  QTest::addColumn<int>( "vertex" );
  QTest::addColumn<QString>( "expected" );

  QTest::newRow( "linestring empty" ) << QStringLiteral( "LINESTRING()" ) << 2 << QStringLiteral( "LineString EMPTY" );
  QTest::newRow( "linestring no matching point" ) << QStringLiteral( "LINESTRING( 1 1, 1 2, 1 3)" ) << 4 << QStringLiteral( "LineString (1 1, 1 2, 1 3)" );
  QTest::newRow( "linestring one vertex" ) << QStringLiteral( "LINESTRING( 1 1)" ) << 0 << QStringLiteral( "LineString (1 1)" );
  QTest::newRow( "linestring match first" ) << QStringLiteral( "LINESTRING( 1 1, 1 2, 1 3, 1 4, 1 1)" ) << 0 << QStringLiteral( "LineString (1 1, 1 2, 1 3, 1 4, 1 1)" );
  QTest::newRow( "linestring match second" ) << QStringLiteral( "LINESTRING( 1 1, 1 2, 1 3, 1 4, 1 1)" ) << 1 << QStringLiteral( "LineString (1 2, 1 3, 1 4, 1 1, 1 2)" );
  QTest::newRow( "linestring match third" ) << QStringLiteral( "LINESTRING( 1 1, 1 2, 1 3, 1 4, 1 1)" ) << 2 << QStringLiteral( "LineString (1 3, 1 4, 1 1, 1 2, 1 3)" );
  QTest::newRow( "linestring match forth" ) << QStringLiteral( "LINESTRING( 1 1, 1 2, 1 3, 1 4, 1 1)" ) << 3 << QStringLiteral( "LineString (1 4, 1 1, 1 2, 1 3, 1 4)" );
  QTest::newRow( "linestring match last" ) << QStringLiteral( "LINESTRING( 1 1, 1 2, 1 3, 1 4, 1 1)" ) << 4 << QStringLiteral( "LineString (1 1, 1 2, 1 3, 1 4, 1 1)" );
  QTest::newRow( "linestringz" ) << QStringLiteral( "LINESTRINGZ( 1 1 3 , 1 2 4, 1 3 5 , 1 4 6, 1 1 3)" ) << 2 << QStringLiteral( "LineStringZ (1 3 5, 1 4 6, 1 1 3, 1 2 4, 1 3 5)" );
  QTest::newRow( "linestringm" ) << QStringLiteral( "LINESTRINGM( 1 1 3 , 1 2 4, 1 3 5 , 1 4 6, 1 1 3)" ) << 2 << QStringLiteral( "LineStringM (1 3 5, 1 4 6, 1 1 3, 1 2 4, 1 3 5)" );
  QTest::newRow( "linestringzm" ) << QStringLiteral( "LINESTRINGZM( 1 1 3 5, 1 2 4 6, 1 3 5 7, 1 4 6 8, 1 1 3 5)" ) << 2 << QStringLiteral( "LineStringZM (1 3 5 7, 1 4 6 8, 1 1 3 5, 1 2 4 6, 1 3 5 7)" );

  QTest::newRow( "circularstring empty" ) << QStringLiteral( "CIRCULARSTRING()" ) << 2 << QStringLiteral( "CircularString EMPTY" );
  QTest::newRow( "circularstring no matching point" ) << QStringLiteral( "CIRCULARSTRING( 1 1, 1 2, 1 3)" ) << 4 << QStringLiteral( "CircularString (1 1, 1 2, 1 3)" );
  // technically not valid, but we don't want a crash
  QTest::newRow( "circularstring one vertex" ) << QStringLiteral( "CIRCULARSTRING( 1 1)" ) << 0 << QStringLiteral( "CircularString (1 1)" );
  QTest::newRow( "circularstring match first" ) << QStringLiteral( "CIRCULARSTRING( 1 1, 2 1, 2 2, 1 4, 1 1)" ) << 0 << QStringLiteral( "CircularString (1 1, 2 1, 2 2, 1 4, 1 1)" );
  QTest::newRow( "circularstring match third" ) << QStringLiteral( "CIRCULARSTRING( 1 1, 2 1, 2 2, 1 4, 1 1)" ) << 2 << QStringLiteral( "CircularString (2 2, 1 4, 1 1, 2 1, 2 2)" );
  QTest::newRow( "circularstring match 5th" ) << QStringLiteral( "CIRCULARSTRING( 1 1, 2 1, 2 2, 3 4, 2 4, 1 4, 1 1)" ) << 4 << QStringLiteral( "CircularString (2 4, 1 4, 1 1, 2 1, 2 2, 3 4, 2 4)" );
  QTest::newRow( "circularstring match last" ) << QStringLiteral( "CIRCULARSTRING( 1 1, 2 1, 2 2, 3 4, 2 4, 1 4, 1 1)" ) << 6 << QStringLiteral( "CircularString (1 1, 2 1, 2 2, 3 4, 2 4, 1 4, 1 1)" );
  QTest::newRow( "circularstringz" ) << QStringLiteral( "CIRCULARSTRINGZ( 1 1 3 , 2 1 4, 2 2 5 , 1 4 6, 1 1 3)" ) << 2 << QStringLiteral( "CircularStringZ (2 2 5, 1 4 6, 1 1 3, 2 1 4, 2 2 5)" );
  QTest::newRow( "circularstringm" ) << QStringLiteral( "CIRCULARSTRINGM( 1 1 3 , 2 1 4, 2 2 5 , 1 4 6, 1 1 3)" ) << 2 << QStringLiteral( "CircularStringM (2 2 5, 1 4 6, 1 1 3, 2 1 4, 2 2 5)" );
  QTest::newRow( "circularstringzm" ) << QStringLiteral( "CIRCULARSTRINGZM( 1 1 3 5, 2 1 4 6, 2 2 5 7, 1 4 6 8, 1 1 3 5)" ) << 2 << QStringLiteral( "CircularStringZM (2 2 5 7, 1 4 6 8, 1 1 3 5, 2 1 4 6, 2 2 5 7)" );

  QTest::newRow( "compoundcurve empty" ) << QStringLiteral( "COMPOUNDCURVE()" ) << 2 << QStringLiteral( "CompoundCurve EMPTY" );
  QTest::newRow( "compoundcurve no matching point" ) << QStringLiteral( "COMPOUNDCURVE(( 1 1, 1 2, 1 3))" ) << 4 << QStringLiteral( "CompoundCurve ((1 1, 1 2, 1 3))" );
  QTest::newRow( "compoundcurve one vertex" ) << QStringLiteral( "COMPOUNDCURVE(( 1 1))" ) << 0 << QStringLiteral( "CompoundCurve ((1 1))" );
  QTest::newRow( "compoundcurve match first" ) << QStringLiteral( "COMPOUNDCURVE(( 1 1, 1 2),( 1 2, 1 3, 1 4, 1 1))" ) << 0 << QStringLiteral( "CompoundCurve ((1 1, 1 2),(1 2, 1 3, 1 4, 1 1))" );
  QTest::newRow( "compoundcurve match second" ) << QStringLiteral( "COMPOUNDCURVE(( 1 1, 1 2),( 1 2, 1 3, 1 4, 1 1))" ) << 1 << QStringLiteral( "CompoundCurve ((1 2, 1 3, 1 4, 1 1),(1 1, 1 2))" );
  QTest::newRow( "compoundcurve match third" ) << QStringLiteral( "COMPOUNDCURVE(( 1 1, 1 2),( 1 2, 1 3, 1 4, 1 1))" ) << 2 << QStringLiteral( "CompoundCurve ((1 3, 1 4, 1 1),(1 1, 1 2),(1 2, 1 3))" );
  QTest::newRow( "compoundcurve match forth" ) << QStringLiteral( "COMPOUNDCURVE(( 1 1, 1 2),( 1 2, 1 3, 1 4, 1 1))" ) << 3 << QStringLiteral( "CompoundCurve ((1 4, 1 1),(1 1, 1 2),(1 2, 1 3, 1 4))" );
  QTest::newRow( "compoundcurve match last" ) << QStringLiteral( "COMPOUNDCURVE(( 1 1, 1 2),( 1 2, 1 3, 1 4, 1 1))" ) << 4 << QStringLiteral( "CompoundCurve ((1 1, 1 2),(1 2, 1 3, 1 4, 1 1))" );
  QTest::newRow( "compoundcurvez" ) << QStringLiteral( "COMPOUNDCURVEZ(( 1 1 3 , 1 2 4, 1 3 5 , 1 4 6, 1 1 3))" ) << 2 << QStringLiteral( "CompoundCurveZ ((1 3 5, 1 4 6, 1 1 3),(1 1 3, 1 2 4, 1 3 5))" );
  QTest::newRow( "compoundcurvem" ) << QStringLiteral( "COMPOUNDCURVEM(( 1 1 3 , 1 2 4, 1 3 5 , 1 4 6, 1 1 3))" ) << 2 << QStringLiteral( "CompoundCurveM ((1 3 5, 1 4 6, 1 1 3),(1 1 3, 1 2 4, 1 3 5))" );
  QTest::newRow( "compoundcurvezm" ) << QStringLiteral( "COMPOUNDCURVEZM(( 1 1 3 5, 1 2 4 6, 1 3 5 7, 1 4 6 8, 1 1 3 5))" ) << 2 << QStringLiteral( "CompoundCurveZM ((1 3 5 7, 1 4 6 8, 1 1 3 5),(1 1 3 5, 1 2 4 6, 1 3 5 7))" );
}

void TestQgsGeometry::scroll()
{
  QFETCH( QString, curve );
  QFETCH( int, vertex );
  QFETCH( QString, expected );

  QgsGeometry geom = QgsGeometry::fromWkt( curve );
  qgsgeometry_cast< QgsCurve * >( geom.get() )->scroll( vertex );

  QCOMPARE( geom.get()->asWkt(), expected );
}

void TestQgsGeometry::normalize_data()
{
  QTest::addColumn<QString>( "wkt" );
  QTest::addColumn<QString>( "expected" );

  QTest::newRow( "empty" ) << QString() << QString();
  QTest::newRow( "point empty" ) << QStringLiteral( "POINT EMPTY" )  << QStringLiteral( "Point EMPTY" );
  QTest::newRow( "point" ) << QStringLiteral( "POINT (1 2)" )  << QStringLiteral( "Point (1 2)" );
  // same code paths are used for all curve derivatives
  QTest::newRow( "line empty" ) << QStringLiteral( "LINESTRING EMPTY" )  << QStringLiteral( "LineString EMPTY" );
  QTest::newRow( "line open" ) << QStringLiteral( "LINESTRING (1 1, 1 2, 1 3)" )  << QStringLiteral( "LineString (1 1, 1 2, 1 3)" );
  QTest::newRow( "line closed" ) << QStringLiteral( "LINESTRING (1 1, 1 0, 0 0, 0 1, 1 1)" )  << QStringLiteral( "LineString (0 0, 0 1, 1 1, 1 0, 0 0)" );
  QTest::newRow( "circular string closed" ) << QStringLiteral( "CIRCULARSTRINGZM (1 1 11 21, 1 0 12 22, 0 0 13 23, 0 1 14 24, 1 1 11 21)" )  << QStringLiteral( "CircularStringZM (0 0 13 23, 0 1 14 24, 1 1 11 21, 1 0 12 22, 0 0 13 23)" );
  // same code paths are used for all curve polygon derivatives
  QTest::newRow( "polygon empty" ) << QStringLiteral( "POLYGON EMPTY" )  << QStringLiteral( "Polygon EMPTY" );
  QTest::newRow( "polygon exterior" ) << QStringLiteral( "POLYGON ((1 1, 0 1, 0 0, 1 0, 1 1))" )  << QStringLiteral( "Polygon ((0 0, 0 1, 1 1, 1 0, 0 0))" );
  QTest::newRow( "polygon rings" ) << QStringLiteral( "POLYGON ((1 1, 0 1, 0 0, 1 0, 1 1),(0.1 0.2,  0.2 0.2, 0.2 0.1, 0.1 0.1, 0.1 0.2),(0.8 0.8, 0.8 0.7, 0.7 0.7, 0.7 0.8, 0.8 0.8))" )  << QStringLiteral( "Polygon ((0 0, 0 1, 1 1, 1 0, 0 0),(0.7 0.7, 0.8 0.7, 0.8 0.8, 0.7 0.8, 0.7 0.7),(0.1 0.1, 0.2 0.1, 0.2 0.2, 0.1 0.2, 0.1 0.1))" );
  // same code paths are used for all collection derivatives
  QTest::newRow( "collection empty" ) << QStringLiteral( "GEOMETRYCOLLECTION EMPTY" )  << QStringLiteral( "GeometryCollection EMPTY" );
  QTest::newRow( "multipoint" ) << QStringLiteral( "MULTIPOINT(1 1, 3 4, 1 3, 2 2)" )  << QStringLiteral( "MultiPoint ((3 4),(2 2),(1 3),(1 1))" );
}

void TestQgsGeometry::normalize()
{
  QFETCH( QString, wkt );
  QFETCH( QString, expected );

  QgsGeometry geom = QgsGeometry::fromWkt( wkt );
  geom.normalize();

  QCOMPARE( geom.asWkt( 1 ), expected );
}


void TestQgsGeometry::simplifiedTypeRef_data()
{
  QTest::addColumn<QString>( "wkt" );
  QTest::addColumn<QString>( "expected" );

  QTest::newRow( "point empty" ) << QStringLiteral( "POINT EMPTY" )  << QStringLiteral( "Point EMPTY" );
  QTest::newRow( "point" ) << QStringLiteral( "POINT (1 2)" )  << QStringLiteral( "Point (1 2)" );
  QTest::newRow( "line empty" ) << QStringLiteral( "LINESTRING EMPTY" )  << QStringLiteral( "LineString EMPTY" );
  QTest::newRow( "line" ) << QStringLiteral( "LINESTRING (1 1, 1 2, 1 3)" )  << QStringLiteral( "LineString (1 1, 1 2, 1 3)" );
  QTest::newRow( "circular string" ) << QStringLiteral( "CIRCULARSTRINGZM (1 1 11 21, 1 0 12 22, 0 0 13 23, 0 1 14 24, 1 1 11 21)" )  << QStringLiteral( "CircularStringZM (1 1 11 21, 1 0 12 22, 0 0 13 23, 0 1 14 24, 1 1 11 21)" );
  QTest::newRow( "compound curve empty" ) << QStringLiteral( "COMPOUNDCURVE EMPTY" )  << QStringLiteral( "CompoundCurve EMPTY" );
  QTest::newRow( "compound curve one curve" ) << QStringLiteral( "COMPOUNDCURVE ((1 1, 1 2, 2 3))" )  << QStringLiteral( "LineString (1 1, 1 2, 2 3)" );
  QTest::newRow( "compound curve two curves" ) << QStringLiteral( "COMPOUNDCURVE ((1 1, 1 2, 2 3),(2 3, 4 4))" )  << QStringLiteral( "CompoundCurve ((1 1, 1 2, 2 3),(2 3, 4 4))" );
  QTest::newRow( "polygon empty" ) << QStringLiteral( "POLYGON EMPTY" )  << QStringLiteral( "Polygon EMPTY" );
  QTest::newRow( "polygon exterior" ) << QStringLiteral( "POLYGON ((1 1, 0 1, 0 0, 1 0, 1 1))" )  << QStringLiteral( "Polygon ((1 1, 0 1, 0 0, 1 0, 1 1))" );
  QTest::newRow( "collection empty" ) << QStringLiteral( "GEOMETRYCOLLECTION EMPTY" )  << QStringLiteral( "GeometryCollection EMPTY" );
  QTest::newRow( "multipoint one point" ) << QStringLiteral( "MULTIPOINT(1 1)" )  << QStringLiteral( "Point (1 1)" );
  QTest::newRow( "multipoint" ) << QStringLiteral( "MULTIPOINT(1 1, 3 4, 1 3, 2 2)" )  << QStringLiteral( "MultiPoint ((1 1),(3 4),(1 3),(2 2))" );
}

void TestQgsGeometry::simplifiedTypeRef()
{
  QFETCH( QString, wkt );
  QFETCH( QString, expected );

  QgsGeometry geom = QgsGeometry::fromWkt( wkt );
  QCOMPARE( geom.constGet()->simplifiedTypeRef()->asWkt( 1 ), expected );
}

// MK, Disabled 14.11.2014
// Too unclear what exactly should be tested and which variations are allowed for the line
#if 0
void TestQgsGeometry::simplifyCheck1()
{
  initPainterTest();
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
  initPainterTest();
  QVERIFY( mpPolygonGeometryA.intersects( mpPolygonGeometryB ) );

  std::unique_ptr< QgsGeometryEngine > engine( QgsGeometry::createGeometryEngine( mpPolygonGeometryA.constGet() ) );
  QVERIFY( engine->intersects( mpPolygonGeometryB.constGet() ) );
  engine->prepareGeometry();
  QVERIFY( engine->intersects( mpPolygonGeometryB.constGet() ) );

  // should be a single polygon as A intersect B
  QgsGeometry mypIntersectionGeometry  =  mpPolygonGeometryA.intersection( mpPolygonGeometryB );
  QVERIFY( mypIntersectionGeometry.wkbType() == QgsWkbTypes::Polygon );
  QgsPolygonXY myPolygon = mypIntersectionGeometry.asPolygon();
  QVERIFY( myPolygon.size() > 0 ); //check that the union created a feature
  paintPolygon( myPolygon );
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
  initPainterTest();
  // should be a multipolygon with 2 parts as A does not intersect C
  QgsGeometry mypUnionGeometry  =  mpPolygonGeometryA.combine( mpPolygonGeometryC );
  QVERIFY( mypUnionGeometry.wkbType() == QgsWkbTypes::MultiPolygon );
  QgsMultiPolygonXY myMultiPolygon = mypUnionGeometry.asMultiPolygon();
  QVERIFY( myMultiPolygon.size() > 0 ); //check that the union did not fail
  paintMultiPolygon( myMultiPolygon );
  QVERIFY( renderCheck( "geometry_unionCheck1", "Checking A union C produces 2 polys" ) );
}

void TestQgsGeometry::unionCheck2()
{
  initPainterTest();
  // should be a single polygon as A intersect B
  QgsGeometry mypUnionGeometry  =  mpPolygonGeometryA.combine( mpPolygonGeometryB );
  QVERIFY( mypUnionGeometry.wkbType() == QgsWkbTypes::Polygon );
  QgsPolygonXY myPolygon = mypUnionGeometry.asPolygon();
  QVERIFY( myPolygon.size() > 0 ); //check that the union created a feature
  paintPolygon( myPolygon );
  QVERIFY( renderCheck( "geometry_unionCheck2", "Checking A union B produces single union poly" ) );
}

void TestQgsGeometry::differenceCheck1()
{
  initPainterTest();
  // should be same as A since A does not intersect C so diff is 100% of A
  QgsGeometry mypDifferenceGeometry( mpPolygonGeometryA.difference( mpPolygonGeometryC ) );
  QVERIFY( mypDifferenceGeometry.wkbType() == QgsWkbTypes::Polygon );
  QgsPolygonXY myPolygon = mypDifferenceGeometry.asPolygon();
  QVERIFY( myPolygon.size() > 0 ); //check that the union did not fail
  paintPolygon( myPolygon );
  QVERIFY( renderCheck( "geometry_differenceCheck1", "Checking (A - C) = A" ) );
}

void TestQgsGeometry::differenceCheck2()
{
  initPainterTest();
  // should be a single polygon as (A - B) = subset of A
  QgsGeometry mypDifferenceGeometry( mpPolygonGeometryA.difference( mpPolygonGeometryB ) );
  QVERIFY( mypDifferenceGeometry.wkbType() == QgsWkbTypes::Polygon );
  QgsPolygonXY myPolygon = mypDifferenceGeometry.asPolygon();
  QVERIFY( myPolygon.size() > 0 ); //check that the union created a feature
  paintPolygon( myPolygon );
  QVERIFY( renderCheck( "geometry_differenceCheck2", "Checking (A - B) = subset of A" ) );
}
void TestQgsGeometry::bufferCheck()
{
  initPainterTest();
  // should be a single polygon
  QgsGeometry mypBufferGeometry( mpPolygonGeometryB.buffer( 10, 10 ) );
  QVERIFY( mypBufferGeometry.wkbType() == QgsWkbTypes::Polygon );
  QgsPolygonXY myPolygon = mypBufferGeometry.asPolygon();
  QVERIFY( myPolygon.size() > 0 ); //check that the buffer created a feature
  paintPolygon( myPolygon );
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

  // curved geometry
  wkt = QStringLiteral( "CurvePolygon (CompoundCurve (CircularString (-70.75639028391421448 42.11076979194393743, -70.75300889449444242 42.10738840252416537, -70.75639028391421448 42.10400701310439331, -70.75977167333398654 42.10738840252416537, -70.75639028391421448 42.11076979194393743)))	1" );
  geom = QgsGeometry::fromWkt( wkt );
  result = geom.smooth( 3 );
  QCOMPARE( result.wkbType(), QgsWkbTypes::Polygon );
}

void TestQgsGeometry::shortestLineEmptyGeometry()
{
  // test calculating distance to an empty geometry
  // refs https://github.com/qgis/QGIS/issues/41968
  QgsGeometry geom1( QgsGeometry::fromWkt( QStringLiteral( "Polygon ((0 0, 10 0, 10 10, 0 10, 0 0 ))" ) ) );
  QgsGeometry geom2( new QgsPoint() );
  QgsGeos geos( geom1.constGet() );
  QVERIFY( geos.shortestLine( geom2.constGet() ).isNull() );
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
  QString geojson = QStringLiteral( "{\"coordinates\":[40.0,50.0],\"type\":\"Point\"}" );
  QCOMPARE( obtained, geojson );

  //MultiPoint
  wkt = QStringLiteral( "MultiPoint (0 0, 10 0, 10 10, 20 10)" );
  geom = QgsGeometry::fromWkt( wkt );
  obtained = geom.asJson();
  geojson = QStringLiteral( "{\"coordinates\":[[0.0,0.0],[10.0,0.0],[10.0,10.0],[20.0,10.0]],\"type\":\"MultiPoint\"}" );
  QCOMPARE( obtained, geojson );

  //Linestring
  wkt = QStringLiteral( "LineString(0 0, 10 0, 10 10, 20 10)" );
  geom = QgsGeometry::fromWkt( wkt );
  obtained = geom.asJson();
  geojson = QStringLiteral( "{\"coordinates\":[[0.0,0.0],[10.0,0.0],[10.0,10.0],[20.0,10.0]],\"type\":\"LineString\"}" );
  QCOMPARE( obtained, geojson );

  //MultiLineString
  wkt = QStringLiteral( "MultiLineString ((0 0, 10 0, 10 10, 20 10),(30 30, 40 30, 40 40, 50 40))" );
  geom = QgsGeometry::fromWkt( wkt );
  obtained = geom.asJson();
  geojson = QStringLiteral( "{\"coordinates\":[[[0.0,0.0],[10.0,0.0],[10.0,10.0],[20.0,10.0]],[[30.0,30.0],[40.0,30.0],[40.0,40.0],[50.0,40.0]]],\"type\":\"MultiLineString\"}" );
  QCOMPARE( obtained, geojson );

  //Polygon
  wkt = QStringLiteral( "Polygon ((0 0, 10 0, 10 10, 0 10, 0 0 ),(2 2, 4 2, 4 4, 2 4, 2 2))" );
  geom = QgsGeometry::fromWkt( wkt );
  obtained = geom.asJson();
  geojson = QStringLiteral( "{\"coordinates\":[[[0.0,0.0],[10.0,0.0],[10.0,10.0],[0.0,10.0],[0.0,0.0]],[[2.0,2.0],[4.0,2.0],[4.0,4.0],[2.0,4.0],[2.0,2.0]]],\"type\":\"Polygon\"}" );
  QCOMPARE( obtained, geojson );

  //MultiPolygon
  wkt = QStringLiteral( "MultiPolygon (((0 0, 10 0, 10 10, 0 10, 0 0 )),((2 2, 4 2, 4 4, 2 4, 2 2)))" );
  geom = QgsGeometry::fromWkt( wkt );
  obtained = geom.asJson();
  geojson = QStringLiteral( "{\"coordinates\":[[[[0.0,0.0],[10.0,0.0],[10.0,10.0],[0.0,10.0],[0.0,0.0]]],[[[2.0,2.0],[4.0,2.0],[4.0,4.0],[2.0,4.0],[2.0,2.0]]]],\"type\":\"MultiPolygon\"}" );
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

void TestQgsGeometry::paintMultiPolygon( QgsMultiPolygonXY &multiPolygon )
{
  for ( int i = 0; i < multiPolygon.size(); i++ )
  {
    QgsPolygonXY myPolygon = multiPolygon.at( i );
    paintPolygon( myPolygon );
  }
}

void TestQgsGeometry::dumpPolygon( QgsPolygonXY &polygon )
{
  for ( int j = 0; j < polygon.size(); j++ )
  {
    const QgsPolylineXY &myPolyline = polygon.at( j ); //rings of polygon
    qDebug( "\t\tRing in polygon: %d", j );

    for ( int k = 0; k < myPolyline.size(); k++ )
    {
      const QgsPointXY &myPoint = myPolyline.at( k );
      qDebug( "\t\t\tPoint in ring %d : %s", k, myPoint.toString().toLocal8Bit().constData() );
    }
  }
}

void TestQgsGeometry::paintPolygon( QgsPolygonXY &polygon )
{
  QVector<QPointF> myPoints;
  for ( int j = 0; j < polygon.size(); j++ )
  {
    const QgsPolylineXY &myPolyline = polygon.at( j ); //rings of polygon
    for ( int k = 0; k < myPolyline.size(); k++ )
    {
      const QgsPointXY &myPoint = myPolyline.at( k );
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
    const QgsPointXY &myPoint = polyline.at( j );
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

void TestQgsGeometry::wkbInOut()
{
  // Premature end of WKB
  // See https://github.com/qgis/QGIS/issues/22184
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

void TestQgsGeometry::makeValid_data()
{
  QTest::addColumn<QString>( "input" );
  QTest::addColumn<QString>( "expected" );

  QTest::newRow( "dimension collapse" ) << QStringLiteral( "LINESTRING(0 0)" ) << QStringLiteral( "" );
  QTest::newRow( "unclosed ring" ) << QStringLiteral( "POLYGON((10 22,10 32,20 32,20 22))" ) << QStringLiteral( "Polygon ((10 22, 10 32, 20 32, 20 22, 10 22))" );
  QTest::newRow( "butterfly polygon (self-intersecting ring)" ) << QStringLiteral( "POLYGON((0 0, 10 10, 10 0, 0 10, 0 0))" ) << QStringLiteral( "MultiPolygon (((5 5, 10 10, 10 0, 5 5)),((0 0, 0 10, 5 5, 0 0)))" );
  QTest::newRow( "polygon with extra tail (a part of the ring does not form any area)" ) << QStringLiteral( "POLYGON((0 0, 1 0, 1 1, 0 1, 0 0, -1 0, 0 0))" ) << QStringLiteral( "GeometryCollection (Polygon ((0 0, 0 1, 1 1, 1 0, 0 0)),LineString (0 0, -1 0))" );
  QTest::newRow( "collection with invalid geometries" ) << QStringLiteral( "GEOMETRYCOLLECTION(LINESTRING(0 0, 0 0), POLYGON((0 0, 10 10, 10 0, 0 10, 0 0)), LINESTRING(10 0, 10 10))" ) << QStringLiteral( "GeometryCollection (MultiPolygon (((5 5, 10 10, 10 0, 5 5)),((0 0, 0 10, 5 5, 0 0))),LineString (10 0, 10 10),Point (0 0))" );
  QTest::newRow( "null line (#18077)" ) << QStringLiteral( "MultiLineString ((356984.0625 6300089, 356984.0625 6300089))" ) << QStringLiteral( "Point (356984.0625 6300089)" );
}

void TestQgsGeometry::makeValid()
{
  QFETCH( QString, input );
  QFETCH( QString, expected );

  QgsGeometry gInput = QgsGeometry::fromWkt( input );
  QVERIFY( !gInput.isNull() );

  QgsGeometry gValid = gInput.makeValid();
  gValid.normalize();
  QCOMPARE( gValid.asWkt(), expected );
}

void TestQgsGeometry::isSimple_data()
{
  QTest::addColumn<QString>( "wkt" );
  QTest::addColumn<bool>( "simple" );

  QTest::newRow( "linestring" ) << QStringLiteral( "LINESTRING(0 0, 1 0, 1 1)" ) << true;
  QTest::newRow( "may be closed (linear ring)" ) << QStringLiteral( "LINESTRING(0 0, 1 0, 1 1, 0 0)" ) << true;
  QTest::newRow( "self-intersection" ) << QStringLiteral( "LINESTRING(0 0, 1 0, 1 1, 0 -1)" ) << false;
  QTest::newRow( "self-tangency" ) << QStringLiteral( "LINESTRING(0 0, 1 0, 1 1, 0.5 0, 0 1)" ) << false;
  QTest::newRow( "points are simple" ) << QStringLiteral( "POINT(1 1)" ) << true;
  QTest::newRow( "multipoint" ) << QStringLiteral( "MULTIPOINT((1 1), (2 2))" ) << true;
  QTest::newRow( "must not contain the same point twice" ) << QStringLiteral( "MULTIPOINT((1 1), (1 1))" ) << false;
  QTest::newRow( "multiline string simple" ) << QStringLiteral( "MULTILINESTRING((0 0, 1 0), (0 1, 1 1))" ) << true;
  QTest::newRow( "may be touching at endpoints" ) << QStringLiteral( "MULTILINESTRING((0 0, 1 0), (0 0, 1 0))" ) << true;
  QTest::newRow( "must not intersect each other" ) << QStringLiteral( "MULTILINESTRING((0 0, 1 1), (0 1, 1 0))" ) << false;
}

void TestQgsGeometry::isSimple()
{
  QFETCH( QString, wkt );
  QFETCH( bool, simple );

  QgsGeometry gInput = QgsGeometry::fromWkt( wkt );
  QVERIFY( !gInput.isNull() );

  bool res = gInput.isSimple();
  QCOMPARE( res, simple );
}

void TestQgsGeometry::reshapeGeometryLineMerge()
{
  int res;
  QgsGeometry g2D = QgsGeometry::fromWkt( QStringLiteral( "LINESTRING(10 10, 20 20)" ) );
  QgsGeometry g3D = QgsGeometry::fromWkt( QStringLiteral( "LINESTRINGZ(10 10 1, 20 20 2)" ) );

  // prepare 2D reshaping line
  QgsPointSequence v2D_1, v2D_2;
  v2D_1 << QgsPoint( 20, 20 ) << QgsPoint( 30, 30 );
  v2D_2 << QgsPoint( 10, 10 ) << QgsPoint( -10, -10 );
  QgsLineString line2D_1( v2D_1 ), line2D_2( v2D_2 );

  // prepare 3D reshaping line
  QgsPointSequence v3D_1, v3D_2;
  v3D_1 << QgsPoint( QgsWkbTypes::PointZ, 20, 20, 2 ) << QgsPoint( QgsWkbTypes::PointZ, 30, 30, 3 );
  v3D_2 << QgsPoint( QgsWkbTypes::PointZ, 10, 10, 1 ) << QgsPoint( QgsWkbTypes::PointZ, -10, -10, -1 );
  QgsLineString line3D_1( v3D_1 ), line3D_2( v3D_2 );

  // append with 2D line
  QgsGeometry g2D_1 = g2D;
  res = static_cast< int >( g2D_1.reshapeGeometry( line2D_1 ) );
  QCOMPARE( res, 0 );
  QCOMPARE( g2D_1.asWkt(), QString( "LineString (10 10, 20 20, 30 30)" ) );

  // prepend with 2D line
  QgsGeometry g2D_2 = g2D;
  res = static_cast< int >( g2D_2.reshapeGeometry( line2D_2 ) );
  QCOMPARE( res, 0 );
  QCOMPARE( g2D_2.asWkt(), QString( "LineString (-10 -10, 10 10, 20 20)" ) );

  // append with 3D line
  QgsGeometry g3D_1 = g3D;
  res = static_cast< int >( g3D_1.reshapeGeometry( line3D_1 ) );
  QCOMPARE( res, 0 );
  QCOMPARE( g3D_1.asWkt(), QString( "LineStringZ (10 10 1, 20 20 2, 30 30 3)" ) );

  // prepend with 3D line
  QgsGeometry g3D_2 = g3D;
  res = static_cast< int >( g3D_2.reshapeGeometry( line3D_2 ) );
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

void TestQgsGeometry::orientedMinimumBoundingBox()
{
  QgsGeometry geomTest;
  QgsGeometry result, resultTest;
  // empty
  result = geomTest.orientedMinimumBoundingBox( );
  QVERIFY( result.isEmpty() );

  // oriented rectangle
  geomTest = QgsGeometry::fromWkt( QStringLiteral( " Polygon(0 0, 5 5, -2.07106781186547462 12.07106781186547551, -7.07106781186547462 7.07106781186547551, 0 0) " ) );
  result = geomTest.orientedMinimumBoundingBox( );
  QgsPolygonXY geomXY, resultXY;
  geomXY = geomTest.asPolygon();
  resultXY = result.asPolygon();

  QCOMPARE( geomXY.count(), resultXY.count() );
  // can't strictly compare, use tolerance
  for ( int i = 0 ; i < geomXY.count() ; ++i )
  {
    QVERIFY( geomXY.at( 0 ).at( i ).compare( resultXY.at( 0 ).at( i ), 1E-8 ) );
  }

  // Issue https://github.com/qgis/QGIS/issues/33532
  geomTest = QgsGeometry::fromWkt( QStringLiteral( " Polygon ((264 -525, 248 -521, 244 -519, 233 -508, 231 -504, 210 -445, 196 -396, 180 -332, 178 -322, 176 -310, 174 -296, 174 -261, 176 -257, 178 -255, 183 -251, 193 -245, 197 -243, 413 -176, 439 -168, 447 -166, 465 -164, 548 -164, 552 -166, 561 -175, 567 -187, 602 -304, 618 -379, 618 -400, 616 -406, 612 -414, 606 -420, 587 -430, 575 -436, 547 -446, 451 -474, 437 -478, 321 -511, 283 -521, 275 -523, 266 -525, 264 -525)) " ) );
  result = geomTest.orientedMinimumBoundingBox( );
  QString resultTestWKT = QStringLiteral( "Polygon ((635.86 -420.08, 552.66 -134.85, 153.5 -251.27, 236.69 -536.51, 635.86 -420.08))" );
  QCOMPARE( result.asWkt( 2 ), resultTestWKT );

  // Issue https://github.com/qgis/QGIS/issues/47726
  geomTest = QgsGeometry::fromWkt( QStringLiteral( "MultiPolygon (((-57 -30, -56.5 -30, -56 -30, -55.5 -30, -55.5 -29.6667, -55.5 -29.333, -55.5 -29, -56 -29, -56.5 -29, -57 -29, -57 -29.3333, -57 -29.6666, -57 -30)))" ) );
  result = geomTest.orientedMinimumBoundingBox( );
  resultTestWKT = QStringLiteral( "Polygon ((-57 -30, -55.5 -30, -55.5 -29, -57 -29, -57 -30))" );
  QCOMPARE( result.asWkt( 2 ), resultTestWKT );

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
  QCOMPARE( result.asWkt( 1 ), QStringLiteral( "Polygon ((0 5, 4.3 2.5, 4.3 -2.5, 0 -5, -4.3 -2.5, -4.3 2.5, 0 5))" ) );

  geomTest = QgsGeometry::fromWkt( QStringLiteral( "MULTIPOINT( 0 5, 2 2, 0 -5, -1 -1 )" ) );
  result = geomTest.minimalEnclosingCircle( center, radius, 6 );
  QGSCOMPARENEARPOINT( center, QgsPointXY( 0, 0 ), 0.0001 );
  QGSCOMPARENEAR( radius, 5, 0.0001 );
  QCOMPARE( result.asWkt( 1 ), QStringLiteral( "Polygon ((0 5, 4.3 2.5, 4.3 -2.5, 0 -5, -4.3 -2.5, -4.3 2.5, 0 5))" ) );

  geomTest = QgsGeometry::fromWkt( QStringLiteral( "POLYGON(( 0 5, 2 2, 0 -5, -1 -1 ))" ) );
  result = geomTest.minimalEnclosingCircle( center, radius, 6 );
  QGSCOMPARENEARPOINT( center, QgsPointXY( 0, 0 ), 0.0001 );
  QGSCOMPARENEAR( radius, 5, 0.0001 );
  resultTest.set( QgsCircle( QgsPoint( center ), radius ).toPolygon( 36 ) );
  QCOMPARE( result.asWkt( 1 ), QStringLiteral( "Polygon ((0 5, 4.3 2.5, 4.3 -2.5, 0 -5, -4.3 -2.5, -4.3 2.5, 0 5))" ) );

  geomTest = QgsGeometry::fromWkt( QStringLiteral( "MULTIPOINT( 0 5, 0 -5, 0 0 )" ) );
  result = geomTest.minimalEnclosingCircle( center, radius, 6 );
  QGSCOMPARENEARPOINT( center, QgsPointXY( 0, 0 ), 0.0001 );
  QGSCOMPARENEAR( radius, 5, 0.0001 );
  resultTest.set( QgsCircle( QgsPoint( center ), radius ).toPolygon( 36 ) );
  QCOMPARE( result.asWkt( 1 ), QStringLiteral( "Polygon ((0 5, 4.3 2.5, 4.3 -2.5, 0 -5, -4.3 -2.5, -4.3 2.5, 0 5))" ) );

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
  QVector<QgsGeometry> newGeoms;
  QgsPointSequence testPoints;
  qDebug() << GEOSversion() << "\n";
  QgsGeometry g1 = QgsGeometry::fromWkt( QStringLiteral( "Polygon ((492980.38648063864093274 7082334.45244149677455425, 493082.65415841294452548 7082319.87918917648494244, 492980.38648063858272508 7082334.45244149677455425, 492980.38648063864093274 7082334.45244149677455425))" ) );
  if ( GEOS_VERSION_MAJOR == 3 && GEOS_VERSION_MINOR < 9 )
  {
    QCOMPARE( g1.splitGeometry( QgsPointSequence() << QgsPoint( 493825.46541286131832749, 7082214.02779923938214779 ) << QgsPoint( 492955.04876351181883365, 7082338.06309300474822521 ),
                                newGeoms, false, testPoints ), Qgis::GeometryOperationResult::NothingHappened );
  }
  else
  {
    QCOMPARE( g1.splitGeometry( QgsPointSequence() << QgsPoint( 493825.46541286131832749, 7082214.02779923938214779 ) << QgsPoint( 492955.04876351181883365, 7082338.06309300474822521 ),
                                newGeoms, false, testPoints ), Qgis::GeometryOperationResult::InvalidBaseGeometry );
  }
  QVERIFY( newGeoms.isEmpty() );

  // Bug https://github.com/qgis/QGIS/issues/33489
  QgsGeometry g2 = QgsGeometry::fromWkt( "CompoundCurveZ ((2749546.2003820720128715 1262904.45356595050543547 100, 2749557.82053794478997588 1262920.05570670193992555 200))" );
  testPoints.clear();
  newGeoms.clear();
  QCOMPARE( g2.splitGeometry( QgsPointSequence() << QgsPoint( 2749544.19, 1262914.79, 0 ) << QgsPoint( 2749557.64, 1262897.30, 0 ), newGeoms, false, testPoints ), Qgis::GeometryOperationResult::Success );
  QVERIFY( newGeoms.count() == 1 );
  QCOMPARE( newGeoms[0].asWkt( 2 ), QStringLiteral( "LineStringZ (2749549.12 1262908.38 125.14, 2749557.82 1262920.06 200)" ) );

  // This should obviously not crash
  g2 = QgsGeometry::fromWkt( "CompoundCurve ((1 1, 2 2, 3 3))" );
  testPoints.clear();
  newGeoms.clear();
  QCOMPARE( g2.splitGeometry( QgsPointSequence() << QgsPoint( 2, 2 ), newGeoms, false, testPoints ), Qgis::GeometryOperationResult::Success );
  QVERIFY( newGeoms.count() == 1 );
  QCOMPARE( newGeoms[0].asWkt( 2 ), QStringLiteral( "LineString (2 2, 3 3)" ) );

  // Do not split on self-intersections - https://github.com/qgis/QGIS/issues/14070
  g2 = QgsGeometry::fromWkt( "LineString (0 0, 10 0, 10 2, 6 2, 6 -2, 3 -2, 3 2, 0 2, 0 0)" );
  testPoints.clear();
  newGeoms.clear();
  QCOMPARE( g2.splitGeometry( QgsPointSequence() << QgsPoint( 0, 1 ) << QgsPoint( 11, 1 ) << QgsPoint( 11, -1 ) << QgsPoint( 0, -1 ), newGeoms, false, testPoints ), Qgis::GeometryOperationResult::Success );
  QCOMPARE( newGeoms.count(), 6 );
  QCOMPARE( newGeoms[0].asWkt( 2 ), QStringLiteral( "LineString (10 1, 10 2, 6 2, 6 1)" ) );
  QCOMPARE( newGeoms[1].asWkt( 2 ), QStringLiteral( "LineString (6 1, 6 -1)" ) );
  QCOMPARE( newGeoms[2].asWkt( 2 ), QStringLiteral( "LineString (6 -1, 6 -2, 3 -2, 3 -1)" ) );
  QCOMPARE( newGeoms[3].asWkt( 2 ), QStringLiteral( "LineString (3 -1, 3 1)" ) );
  QCOMPARE( newGeoms[4].asWkt( 2 ), QStringLiteral( "LineString (3 1, 3 2, 0 2, 0 1)" ) );
  QCOMPARE( newGeoms[5].asWkt( 2 ), QStringLiteral( "LineString (0 1, 0 0)" ) );

  // Same, but with a single split point on an existing vertex
  g2 = QgsGeometry::fromWkt( "LineString (0 0, 10 0, 10 2, 6 2, 6 -2, 3 -2, 3 2, 0 2, 0 0)" );
  testPoints.clear();
  newGeoms.clear();
  QCOMPARE( g2.splitGeometry( QgsPointSequence() << QgsPoint( 6, 2 ), newGeoms, false, testPoints ), Qgis::GeometryOperationResult::Success );
  QCOMPARE( newGeoms.count(), 1 );
  QCOMPARE( newGeoms[0].asWkt( 2 ), QStringLiteral( "LineString (6 2, 6 -2, 3 -2, 3 2, 0 2, 0 0)" ) );

  // Test split geometry with topological editing
  QVector<QgsPointXY> testPointsXY;
  testPoints.clear();
  newGeoms.clear();
  g1 = QgsGeometry::fromWkt( QStringLiteral( "Polygon ((1.0 1.0, 1.0 100.0, 100.0 100.0, 100.0 1.0, 1.0 1.0))" ) );
  QCOMPARE( g1.splitGeometry( QgsPointSequence() << QgsPoint( 0.0, 42.0 ) << QgsPoint( 101.0, 42.0 ), newGeoms, true, testPoints ), Qgis::GeometryOperationResult::Success );
  QCOMPARE( newGeoms.count(), 1 );
  QCOMPARE( testPoints.count(), 2 );
  QgsGeometry::convertPointList( testPoints, testPointsXY );
  QVERIFY( QgsGeometry::fromWkt( QStringLiteral( "Linestring (1.0 42.0, 100.0 42.0)" ) ).touches( QgsGeometry::fromPointXY( testPointsXY.at( 0 ) ) ) );
  QVERIFY( QgsGeometry::fromWkt( QStringLiteral( "Linestring (1.0 42.0, 100.0 42.0)" ) ).touches( QgsGeometry::fromPointXY( testPointsXY.at( 1 ) ) ) );

  testPointsXY.clear();
  testPoints.clear();
  newGeoms.clear();
  g1 = QgsGeometry::fromWkt( QStringLiteral( "Linestring (1.0 1.0, 1.0 100.0, 100.0 100.0, 100.0 1.0, 1.0 1.0)" ) );
  QCOMPARE( g1.splitGeometry( QgsPointSequence() << QgsPoint( 0.0, 42.0 ) << QgsPoint( 101.0, 42.0 ), newGeoms, true, testPoints ), Qgis::GeometryOperationResult::Success );
  QCOMPARE( newGeoms.count(), 2 );
  QCOMPARE( testPoints.count(), 2 );
  QgsGeometry::convertPointList( testPoints, testPointsXY );
  QVERIFY( QgsGeometry::fromWkt( QStringLiteral( "Linestring (1.0 42.0, 100.0 42.0)" ) ).touches( QgsGeometry::fromPointXY( testPointsXY.at( 0 ) ) ) );
  QVERIFY( QgsGeometry::fromWkt( QStringLiteral( "Linestring (1.0 42.0, 100.0 42.0)" ) ).touches( QgsGeometry::fromPointXY( testPointsXY.at( 1 ) ) ) );

  // Test split parts with topological editing
  testPointsXY.clear();
  testPoints.clear();
  newGeoms.clear();
  g1 = QgsGeometry::fromWkt( QStringLiteral( "Polygon ((1.0 1.0, 1.0 100.0, 100.0 100.0, 100.0 1.0, 1.0 1.0))" ) );
  QCOMPARE( g1.splitGeometry( QgsPointSequence() << QgsPoint( 0.0, 42.0 ) << QgsPoint( 101.0, 42.0 ), newGeoms, true, testPoints, false ), Qgis::GeometryOperationResult::Success );
  QCOMPARE( newGeoms.count(), 2 );
  QCOMPARE( testPoints.count(), 2 );
  QgsGeometry::convertPointList( testPoints, testPointsXY );
  QVERIFY( QgsGeometry::fromWkt( QStringLiteral( "Linestring (1.0 42.0, 100.0 42.0)" ) ).touches( QgsGeometry::fromPointXY( testPointsXY.at( 0 ) ) ) );
  QVERIFY( QgsGeometry::fromWkt( QStringLiteral( "Linestring (1.0 42.0, 100.0 42.0)" ) ).touches( QgsGeometry::fromPointXY( testPointsXY.at( 1 ) ) ) );

  testPointsXY.clear();
  testPoints.clear();
  newGeoms.clear();
  g1 = QgsGeometry::fromWkt( QStringLiteral( "Linestring (1.0 1.0, 1.0 100.0, 100.0 100.0, 100.0 1.0, 1.0 1.0)" ) );
  QCOMPARE( g1.splitGeometry( QgsPointSequence() << QgsPoint( 0.0, 42.0 ) << QgsPoint( 101.0, 42.0 ), newGeoms, true, testPoints, false ), Qgis::GeometryOperationResult::Success );
  QCOMPARE( newGeoms.count(), 3 );
  QCOMPARE( testPoints.count(), 2 );
  QgsGeometry::convertPointList( testPoints, testPointsXY );
  QVERIFY( QgsGeometry::fromWkt( QStringLiteral( "Linestring (1.0 42.0, 100.0 42.0)" ) ).touches( QgsGeometry::fromPointXY( testPointsXY.at( 0 ) ) ) );
  QVERIFY( QgsGeometry::fromWkt( QStringLiteral( "Linestring (1.0 42.0, 100.0 42.0)" ) ).touches( QgsGeometry::fromPointXY( testPointsXY.at( 1 ) ) ) );

  // Repeat previous tests with QVector<QgsPointXY> instead of QgsPointSequence
  // Those tests are for the deprecated QgsGeometry::splitGeometry() variant and should be removed in QGIS 4.0
  Q_NOWARN_DEPRECATED_PUSH
  testPointsXY.clear();
  newGeoms.clear();
  g1 = QgsGeometry::fromWkt( QStringLiteral( "Polygon ((1.0 1.0, 1.0 100.0, 100.0 100.0, 100.0 1.0, 1.0 1.0))" ) );
  QCOMPARE( g1.splitGeometry( QgsPolylineXY() << QgsPointXY( 0.0, 42.0 ) << QgsPointXY( 101.0, 42.0 ), newGeoms, true, testPointsXY ), Qgis::GeometryOperationResult::Success );
  QCOMPARE( newGeoms.count(), 1 );
  QCOMPARE( testPointsXY.count(), 2 );
  QVERIFY( QgsGeometry::fromWkt( QStringLiteral( "Linestring (1.0 42.0, 100.0 42.0)" ) ).touches( QgsGeometry::fromPointXY( testPointsXY.at( 0 ) ) ) );
  QVERIFY( QgsGeometry::fromWkt( QStringLiteral( "Linestring (1.0 42.0, 100.0 42.0)" ) ).touches( QgsGeometry::fromPointXY( testPointsXY.at( 1 ) ) ) );

  testPointsXY.clear();
  newGeoms.clear();
  g1 = QgsGeometry::fromWkt( QStringLiteral( "Linestring (1.0 1.0, 1.0 100.0, 100.0 100.0, 100.0 1.0, 1.0 1.0)" ) );
  QCOMPARE( g1.splitGeometry( QgsPolylineXY() << QgsPointXY( 0.0, 42.0 ) << QgsPointXY( 101.0, 42.0 ), newGeoms, true, testPointsXY ), Qgis::GeometryOperationResult::Success );
  QCOMPARE( newGeoms.count(), 2 );
  QCOMPARE( testPointsXY.count(), 2 );
  QVERIFY( QgsGeometry::fromWkt( QStringLiteral( "Linestring (1.0 42.0, 100.0 42.0)" ) ).touches( QgsGeometry::fromPointXY( testPointsXY.at( 0 ) ) ) );
  QVERIFY( QgsGeometry::fromWkt( QStringLiteral( "Linestring (1.0 42.0, 100.0 42.0)" ) ).touches( QgsGeometry::fromPointXY( testPointsXY.at( 1 ) ) ) );

  // Test split parts with topological editing
  testPointsXY.clear();
  newGeoms.clear();
  g1 = QgsGeometry::fromWkt( QStringLiteral( "Polygon ((1.0 1.0, 1.0 100.0, 100.0 100.0, 100.0 1.0, 1.0 1.0))" ) );
  QCOMPARE( g1.splitGeometry( QgsPolylineXY() << QgsPointXY( 0.0, 42.0 ) << QgsPointXY( 101.0, 42.0 ), newGeoms, true, testPointsXY, false ), Qgis::GeometryOperationResult::Success );
  QCOMPARE( newGeoms.count(), 2 );
  QCOMPARE( testPointsXY.count(), 2 );
  QVERIFY( QgsGeometry::fromWkt( QStringLiteral( "Linestring (1.0 42.0, 100.0 42.0)" ) ).touches( QgsGeometry::fromPointXY( testPointsXY.at( 0 ) ) ) );
  QVERIFY( QgsGeometry::fromWkt( QStringLiteral( "Linestring (1.0 42.0, 100.0 42.0)" ) ).touches( QgsGeometry::fromPointXY( testPointsXY.at( 1 ) ) ) );

  testPointsXY.clear();
  newGeoms.clear();
  g1 = QgsGeometry::fromWkt( QStringLiteral( "Linestring (1.0 1.0, 1.0 100.0, 100.0 100.0, 100.0 1.0, 1.0 1.0)" ) );
  QCOMPARE( g1.splitGeometry( QgsPolylineXY() << QgsPointXY( 0.0, 42.0 ) << QgsPointXY( 101.0, 42.0 ), newGeoms, true, testPointsXY, false ), Qgis::GeometryOperationResult::Success );
  QCOMPARE( newGeoms.count(), 3 );
  QCOMPARE( testPointsXY.count(), 2 );
  QVERIFY( QgsGeometry::fromWkt( QStringLiteral( "Linestring (1.0 42.0, 100.0 42.0)" ) ).touches( QgsGeometry::fromPointXY( testPointsXY.at( 0 ) ) ) );
  QVERIFY( QgsGeometry::fromWkt( QStringLiteral( "Linestring (1.0 42.0, 100.0 42.0)" ) ).touches( QgsGeometry::fromPointXY( testPointsXY.at( 1 ) ) ) );
  Q_NOWARN_DEPRECATED_POP
}

void TestQgsGeometry::snappedToGrid()
{
  // points
  {
    auto check = []( QgsPoint * _a, QgsPoint const & b )
    {
      std::unique_ptr<QgsPoint> a {_a};
      // because it is to check after snapping, there shouldn't be small precision errors

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
  QCOMPARE( gc3.asWkt(), QStringLiteral( "MultiLineString EMPTY" ) );
  QVERIFY( gc3.isEmpty() );

  // trying to convert a geometry that is not a geometry collection
  QgsGeometry wrong = QgsGeometry::fromWkt( QStringLiteral( "Point(1 2)" ) );
  QVERIFY( !wrong.convertGeometryCollectionToSubclass( QgsWkbTypes::PolygonGeometry ) );
}

void TestQgsGeometry::emptyJson()
{
  QString expected;
  expected = QStringLiteral( "{\"coordinates\":[],\"type\":\"LineString\"}" );
  QCOMPARE( QgsCircularString().asJson(), expected );
  QCOMPARE( QgsCompoundCurve().asJson(), expected );
  QCOMPARE( QgsLineString().asJson(), expected );

  expected = QStringLiteral( "{\"geometries\":[],\"type\":\"GeometryCollection\"}" );
  QCOMPARE( QgsGeometryCollection().asJson(), expected );

  expected = QStringLiteral( "{\"coordinates\":[],\"type\":\"MultiLineString\"}" );
  QCOMPARE( QgsMultiCurve().asJson(), expected );
  QCOMPARE( QgsMultiLineString().asJson(), expected );

  expected = QStringLiteral( "{\"coordinates\":[],\"type\":\"MultiPoint\"}" );
  QCOMPARE( QgsMultiPoint().asJson(), expected );

  expected = QStringLiteral( "{\"coordinates\":[],\"type\":\"MultiPolygon\"}" );
  QCOMPARE( QgsMultiSurface().asJson(), expected );

  expected = QStringLiteral( "{\"coordinates\":[],\"type\":\"Point\"}" );
  QCOMPARE( QgsPoint().asJson(), expected );

  expected = QStringLiteral( "{\"coordinates\":[],\"type\":\"Polygon\"}" );
  QCOMPARE( QgsCurvePolygon().asJson(), expected );
  QCOMPARE( QgsPolygon().asJson(), expected );
  QCOMPARE( QgsTriangle().asJson(), expected );
}

void TestQgsGeometry::testRandomPointsInPolygon()
{
  // null geometry
  QVector< QgsPointXY > points = QgsGeometry().randomPointsInPolygon( 100 );
  QVERIFY( points.empty() );

  // not polygon geometry
  points = QgsGeometry::fromWkt( QStringLiteral( "Point( 4 5 )" ) ).randomPointsInPolygon( 100 );
  QVERIFY( points.empty() );
  points = QgsGeometry::fromWkt( QStringLiteral( "LineString( 4 5, 6 7 )" ) ).randomPointsInPolygon( 100 );
  QVERIFY( points.empty() );

  // zero point count
  points = QgsGeometry::fromWkt( QStringLiteral( "Polygon(( 5 15, 10 15, 10 20, 5 20, 5 15 ))" ) ).randomPointsInPolygon( 0 );
  QVERIFY( points.empty() );

  // valid polygon
  QgsGeometry g = QgsGeometry::fromWkt( QStringLiteral( "Polygon(( 5 15, 10 15, 10 20, 5 20, 5 15 ), (6 16, 8 16, 8 18, 6 16 ))" ) );
  points = g.randomPointsInPolygon( 10000 );
  QCOMPARE( points.count(), 10000 );
  for ( const QgsPointXY &p : std::as_const( points ) )
    QVERIFY( g.intersects( QgsGeometry::fromPointXY( p ) ) );

  // valid multipolygon
  g = QgsGeometry::fromWkt( QStringLiteral( "MultiPolygon((( 5 15, 10 15, 10 20, 5 20, 5 15 ), (6 16, 8 16, 8 18, 6 16 )), (( 105 115, 110 115, 110 120, 105 120, 105 115 ), (106 116, 108 116, 108 118, 106 116 )))" ) );
  points = g.randomPointsInPolygon( 10000 );
  QCOMPARE( points.count(), 10000 );
  bool foundp1Point = false;
  bool foundp2Point = false;
  for ( const QgsPointXY &p : std::as_const( points ) )
  {
    QVERIFY( g.intersects( QgsGeometry::fromPointXY( p ) ) );
    foundp1Point |= p.x() < 100;
    foundp2Point |= p.x() > 100;
  }
  QVERIFY( foundp1Point );
  QVERIFY( foundp2Point );

  // with seed
  g = QgsGeometry::fromWkt( QStringLiteral( "MultiPolygon((( 5 15, 10 15, 10 20, 5 20, 5 15 ), (6 16, 8 16, 8 18, 6 16 )), (( 105 115, 110 115, 110 120, 105 120, 105 115 ), (106 116, 108 116, 108 118, 106 116 )))" ) );
  QVector< QgsPointXY > points1 = g.randomPointsInPolygon( 100, 200 );
  QCOMPARE( points1.count(), 100 );
  QVector< QgsPointXY > points2 = g.randomPointsInPolygon( 100, 200 );
  QCOMPARE( points2.count(), 100 );
  QCOMPARE( points1, points2 );

  // no seed
  points1 = g.randomPointsInPolygon( 100 );
  QCOMPARE( points1.count(), 100 );
  points2 = g.randomPointsInPolygon( 100 );
  QCOMPARE( points2.count(), 100 );
  QVERIFY( points1 != points2 );

  // with filter
  points = g.randomPointsInPolygon( 10000, []( const QgsPointXY & p )->bool
  {
    return p.x() > 100;
  } );
  QCOMPARE( points.count(), 10000 );
  foundp1Point = false;
  foundp2Point = false;
  for ( const QgsPointXY &p : std::as_const( points ) )
  {
    QVERIFY( g.intersects( QgsGeometry::fromPointXY( p ) ) );
    foundp1Point |= p.x() < 100;
    foundp2Point |= p.x() > 100;
  }
  QVERIFY( !foundp1Point );
  QVERIFY( foundp2Point );
}

void TestQgsGeometry::wktParser()
{
  // POINT
  // unbalanced parentheses
  QVERIFY( QgsPoint().fromWkt( "POINT((0 1)" ) );
  QVERIFY( QgsPoint().fromWkt( "POINT(0 1) )" ) );
  QVERIFY( QgsPoint().fromWkt( "POINT ((0 1)" ) );
  QVERIFY( QgsPoint().fromWkt( "POINT (0 1) )" ) );
  // extra parentheses
  QVERIFY( QgsPoint().fromWkt( "POINT ( (5 1) )" ) );
  // not a number
  QVERIFY( ! QgsPoint().fromWkt( "POINT (a, b)" ) );
  QVERIFY( ! QgsPoint().fromWkt( "POINT (a b)" ) );
  QVERIFY( ! QgsPoint().fromWkt( "POINT((0, 1)" ) );
  QVERIFY( ! QgsPoint().fromWkt( "POINT(0, 1) )" ) );
  QVERIFY( ! QgsPoint().fromWkt( "POINT ((0, 1)" ) );
  QVERIFY( ! QgsPoint().fromWkt( "POINT (0, 1) )" ) );
  QVERIFY( ! QgsPoint().fromWkt( "POINT ( (5, 1) )" ) );

  // valid
  QgsPoint p;
  QVERIFY( p.fromWkt( "POINT (5 1)" ) );
  QCOMPARE( p.asWkt(), QStringLiteral( "Point (5 1)" ) );
  // extra parentheses
  QVERIFY( p.fromWkt( "POINT ( (5 1) )" ) );
  // unbalanced parentheses
  QVERIFY( p.fromWkt( "POINT ( (5 1)" ) );
  QCOMPARE( p.asWkt(), QStringLiteral( "Point (5 1)" ) );
  QVERIFY( p.fromWkt( "POINT (5 1) )" ) );
  QCOMPARE( p.asWkt(), QStringLiteral( "Point (5 1)" ) );

  QVERIFY( p.fromWkt( "POINT (5.1234 1.4321)" ) );
  QCOMPARE( p.asWkt( 4 ), QStringLiteral( "Point (5.1234 1.4321)" ) );
  QVERIFY( p.fromWkt( "POINT (-5.1234 -1.4321)" ) );
  QCOMPARE( p.asWkt( 4 ), QStringLiteral( "Point (-5.1234 -1.4321)" ) );
  QVERIFY( p.fromWkt( "POINT (-12e4 -1.4e-1)" ) );
  QCOMPARE( p.asWkt( 2 ), QStringLiteral( "Point (-120000 -0.14)" ) );

  QVERIFY( p.fromWkt( "POINT ( )" ) );
  QCOMPARE( p.asWkt(), QStringLiteral( "Point EMPTY" ) );
  QVERIFY( ! p.fromWkt( "POINT (a, b)" ) );
  QCOMPARE( p.asWkt(), QStringLiteral( "Point EMPTY" ) );
  // LINESTRING
  QVERIFY( QgsLineString().fromWkt( "LineString(0 1, 1 2) )" ) );
  QVERIFY( QgsLineString().fromWkt( "LineString (0 1, 1 2) )" ) );
  QVERIFY( QgsLineString().fromWkt( "LineString ( (0 1) )" ) );
  QVERIFY( QgsLineString().fromWkt( "LineString ( ( 0 1, 2 3 ) )" ) );
  QVERIFY( QgsMultiLineString().fromWkt( "MULTILINESTRING ((((1 2, 2 4, 4 5))" ) );
  QVERIFY( QgsMultiLineString().fromWkt( "MULTILINESTRING((((1 2, 2 4, 4 5))" ) );
  // not a number
  QVERIFY( ! QgsLineString().fromWkt( "LineString(0, 1) )" ) );
  QVERIFY( ! QgsLineString().fromWkt( "LineString (0, 1) )" ) );
  QVERIFY( ! QgsLineString().fromWkt( "LineString (a b, 3 4)" ) );
  QVERIFY( ! QgsMultiLineString().fromWkt( "MULTILINESTRING ((1 2, 2 a, 4 b))" ) );
  // valid
  QgsLineString l;
  QVERIFY( l.fromWkt( "LINESTRING ( 1 2, 3 4 )" ) );
  QCOMPARE( l.asWkt(), QStringLiteral( "LineString (1 2, 3 4)" ) );
  // unbalanced parentheses
  QVERIFY( l.fromWkt( "LINESTRING ( ( 1 2, 3 4 )" ) );
  QCOMPARE( l.asWkt(), QStringLiteral( "LineString (1 2, 3 4)" ) );
  QVERIFY( l.fromWkt( "LINESTRING ( 1 2, 3 4 ) )" ) );
  QCOMPARE( l.asWkt(), QStringLiteral( "LineString (1 2, 3 4)" ) );
  // extra parentheses
  QVERIFY( l.fromWkt( "LINESTRING( ( 1 2, 3 4 ) )" ) );
  QCOMPARE( l.asWkt(), QStringLiteral( "LineString (1 2, 3 4)" ) );
  // empty
  QVERIFY( l.fromWkt( "LINESTRING (  )" ) );
  QCOMPARE( l.asWkt(), QStringLiteral( "LineString EMPTY" ) );
  QVERIFY( ! l.fromWkt( "LINESTRING ( a b, 2 3  )" ) );
  QCOMPARE( l.asWkt(), QStringLiteral( "LineString EMPTY" ) );
  QVERIFY( l.fromWkt( "LINESTRING ( 1e4 -2, 3 +4.5 )" ) );
  QCOMPARE( l.asWkt( 1 ), QStringLiteral( "LineString (10000 -2, 3 4.5)" ) );
  QgsMultiLineString m;
  m.fromWkt( "MULTILINESTRING ((1 2, 2 3, 4 5))" ) ;
  QVERIFY( m.fromWkt( "MULTILINESTRING ((1 2, 2 3, 4 5))" ) );
  QCOMPARE( m.asWkt( 1 ), QStringLiteral( "MultiLineString ((1 2, 2 3, 4 5))" ) );

  // TRIANGLE
  // unbalanced parenthesis
  QVERIFY( QgsTriangle().fromWkt( "Triangle( (0 1, 1 2, 3 3) )) " ) );
  QVERIFY( QgsTriangle().fromWkt( "Triangle ( (0 1, 1 2, 3 3) )) " ) );
  QVERIFY( ! QgsTriangle().fromWkt( "Triangle(0 1, 1 2, 3 3) )" ) );
  QVERIFY( ! QgsTriangle().fromWkt( "Triangle (0 1, 1 2, 3 3) )" ) );
  // not a number
  QVERIFY( ! QgsTriangle().fromWkt( "Triangle ( (0 a, b 2, 3 3 )) " ) );
  QVERIFY( ! QgsTriangle().fromWkt( "Triangle ( (0 a, b 2, 3 3, 0 a )) " ) );
  // incorrect number of points
  QVERIFY( ! QgsTriangle().fromWkt( "Triangle (( 0 0, 1 1))" ) );
  QVERIFY( ! QgsTriangle().fromWkt( "Triangle (( 0 0, 1 1, 1 0, 2 3, 4 5))" ) );
  QVERIFY( ! QgsTriangle().fromWkt( "Triangle (( 0 1, 2 3, 3 4, 1 0))" ) ); // four points but start and last points are different
  // valid
  QgsTriangle t;
  QVERIFY( t.fromWkt( "TRIANGLE(( 0 1, 2 3, 3 4))" ) );
  QCOMPARE( t.asWkt(), QStringLiteral( "Triangle ((0 1, 2 3, 3 4))" ) );
  QVERIFY( t.fromWkt( "TRIANGLE(( 0 1, 2 3, 3 4, 0 1))" ) );
  QCOMPARE( t.asWkt(), QStringLiteral( "Triangle ((0 1, 2 3, 3 4, 0 1))" ) );
  QVERIFY( t.fromWkt( "TRIANGLE(( 0 1, 2 3, 3 4, 0 1))" ) );
  QCOMPARE( t.asWkt(), QStringLiteral( "Triangle ((0 1, 2 3, 3 4, 0 1))" ) );
  QVERIFY( ! t.fromWkt( "TRIANGLE(( 0 1, 2 3, 3 4, 0 3))" ) );
  QCOMPARE( t.asWkt(), QStringLiteral( "Triangle EMPTY" ) );
  QVERIFY( ! t.fromWkt( "TRIANGLE(( 0 1, 2 3, 3 4, 0 3, 4 5))" ) );
  QCOMPARE( t.asWkt(), QStringLiteral( "Triangle EMPTY" ) );
  QVERIFY( ! t.fromWkt( "TRIANGLE(( 0 1, 2 3 ))" ) );
  QCOMPARE( t.asWkt(), QStringLiteral( "Triangle EMPTY" ) );
  QVERIFY( t.fromWkt( "TRIANGLE((0 1e3, -2 3, +3 4, 0 1e3))" ) );
  QCOMPARE( t.asWkt(), QStringLiteral( "Triangle ((0 1000, -2 3, 3 4, 0 1000))" ) );

  // POLYGON
  // unbalanced parenthesis
  QVERIFY( QgsPolygon().fromWkt( "Polygon( (0 1, 1 2, 3 3) )) " ) );
  QVERIFY( QgsPolygon().fromWkt( "Polygon ( (0 1, 1 2, 3 3) )) " ) );
  QVERIFY( ! QgsPolygon().fromWkt( "Polygon(0 1, 1 2, 3 3) )" ) );
  QVERIFY( ! QgsPolygon().fromWkt( "Polygon (0 1, 1 2, 3 3) )" ) );
  // not a number
  QVERIFY( ! QgsPolygon().fromWkt( "Polygon ( (0 a, b 2, 3 3 )) " ) );
  QVERIFY( ! QgsPolygon().fromWkt( "Polygon ( (0 a, b 2, 3 3, 0 a )) " ) );
  // valid
  QgsPolygon poly;
  QVERIFY( poly.fromWkt( "Polygon(( 0 1, 2 3, 3 4))" ) );
  QCOMPARE( poly.asWkt(), QStringLiteral( "Polygon ((0 1, 2 3, 3 4))" ) ); // TODO: A polygon must be closed
  QVERIFY( poly.fromWkt( "Polygon(( 0 1, 2 3, 3 4, 0 1))" ) );
  QCOMPARE( poly.asWkt(), QStringLiteral( "Polygon ((0 1, 2 3, 3 4, 0 1))" ) );
  QVERIFY( poly.fromWkt( "Polygon((0 1e3, -2 3, +3 4, 0 1))" ) );
  QCOMPARE( poly.asWkt(), QStringLiteral( "Polygon ((0 1000, -2 3, 3 4, 0 1))" ) );

  // Circular string
  // unbalanced parenthesis
  QVERIFY( QgsCircularString().fromWkt( "CircularString(0 1, 1 2, 3 3) )" ) );
  QVERIFY( QgsCircularString().fromWkt( "CircularString (0 1, 1 2, 3 3) )" ) );
  QVERIFY( QgsCircularString().fromWkt( "CircularString( (0 1, 1 2, 3 3) )) " ) );
  QVERIFY( QgsCircularString().fromWkt( "CircularString ( (0 1, 1 2, 3 3) )) " ) );
  // not a number
  QVERIFY( ! QgsCircularString().fromWkt( "CircularString (0 a, b 2, 3 3) " ) );
  QVERIFY( ! QgsCircularString().fromWkt( "CircularString (0 a, b 2, 3 3, 0 a) " ) );
  // valid
  QgsCircularString c;
  QVERIFY( c.fromWkt( "CircularString( 0 1, 2 3, 3 4)" ) );
  QCOMPARE( c.asWkt(), QStringLiteral( "CircularString (0 1, 2 3, 3 4)" ) );
  QVERIFY( c.fromWkt( "CircularString(0 1e3, -2 3, +3 4)" ) );
  QCOMPARE( c.asWkt(), QStringLiteral( "CircularString (0 1000, -2 3, 3 4)" ) );

  QVERIFY( c.fromWkt( "CircularString ((0 0,1 1,2 0))" ) ); // Added from an old test with an invalid wkt, but allowed in QGIS https://github.com/qgis/QGIS/pull/38439/files/59aab9dc9cc58bdc98e6d8091840bc129564ed2f#diff-fe3aa1328ee04f0eb00a1b1d59c0ea71L4247
  QCOMPARE( c.asWkt(), QStringLiteral( "CircularString (0 0, 1 1, 2 0)" ) );

  // multipoint
  QgsMultiPoint mp;
  QVERIFY( ! mp.fromWkt( "MULTIPOINT (((10 40), (40 30), (20 20), (30 10))" ) );
  QVERIFY( ! mp.fromWkt( "MULTIPOINT (((10 40), (40 30), (xx20 20), (30 10))" ) );
  QVERIFY( ! mp.fromWkt( "MULTIPOINT ((10 40, 40 30, 20 20, 30 10)" ) );
  QVERIFY( mp.fromWkt( "MULTIPOINT ((10 40), (40 30), (20 20), (30 10))))" ) );
  QVERIFY( mp.fromWkt( "MULTIPOINT ( )" ) );
  QVERIFY( mp.fromWkt( "MULTIPOINT EMPTY" ) );
  QVERIFY( mp.fromWkt( "MULTIPOINT ((10 40), (40 30), (20 20), (30 10))" ) );
  QCOMPARE( mp.asWkt(), QStringLiteral( "MultiPoint ((10 40),(40 30),(20 20),(30 10))" ) );
  QVERIFY( mp.fromWkt( "MULTIPOINT (10 40, 40 30, 20 20, 30 10)" ) );
  QCOMPARE( mp.asWkt(), QStringLiteral( "MultiPoint ((10 40),(40 30),(20 20),(30 10))" ) );
  QVERIFY( mp.fromWkt( "MULTIPOINT ((10 40), (40 30), (20 20), (30 10))))" ) );
  QCOMPARE( mp.asWkt(), QStringLiteral( "MultiPoint ((10 40),(40 30),(20 20),(30 10))" ) );
  QVERIFY( mp.fromWkt( "MULTIPOINT ( )" ) );
  QCOMPARE( mp.asWkt(), QStringLiteral( "MultiPoint EMPTY" ) );
  QVERIFY( mp.fromWkt( "MULTIPOINT EMPTY" ) );
  QCOMPARE( mp.asWkt(), QStringLiteral( "MultiPoint EMPTY" ) );
  // Added from an old test with an invalid wkt, but allowed in QGIS https://github.com/qgis/QGIS/pull/38439/files/59aab9dc9cc58bdc98e6d8091840bc129564ed2f#diff-4444b5a772b35be43721b71a4b95d785R50
  QVERIFY( mp.fromWkt( "MULTIPOINT(0 20,20 20))" ) );
  QCOMPARE( mp.asWkt(), QStringLiteral( "MultiPoint ((0 20),(20 20))" ) );

  // compoundcurve
  QgsCompoundCurve cc;
  QVERIFY( ! cc.fromWkt( "COMPOUNDCURVE((CIRCULARSTRING( 0 0, 1 1, 2 2))" ) );
  QVERIFY( cc.fromWkt( "COMPOUNDCURVE(CIRCULARSTRING( 0 0, 1 1, 2 2)))" ) );
  QCOMPARE( cc.asWkt(), QStringLiteral( "CompoundCurve (CircularString (0 0, 1 1, 2 2))" ) );
  QVERIFY( cc.fromWkt( "COMPOUNDCURVE(CIRCULARSTRING( 0 0, 1 1, 2 2))" ) );
  QCOMPARE( cc.asWkt(), QStringLiteral( "CompoundCurve (CircularString (0 0, 1 1, 2 2))" ) );
  QVERIFY( cc.fromWkt( "COMPOUNDCURVE(CIRCULARSTRING( 0 0, 1 1, 2 2), LINESTRING((2 2, 3 3)))" ) );
  QCOMPARE( cc.asWkt(), QStringLiteral( "CompoundCurve (CircularString (0 0, 1 1, 2 2),(2 2, 3 3))" ) );
  QVERIFY( cc.fromWkt( "COMPOUNDCURVE ( )" ) );
  QCOMPARE( cc.asWkt(), QStringLiteral( "CompoundCurve EMPTY" ) );
  QVERIFY( cc.fromWkt( "COMPOUNDCURVE EMPTY" ) );
  QCOMPARE( cc.asWkt(), QStringLiteral( "CompoundCurve EMPTY" ) );
  // geometrycollection
  QgsGeometryCollection gc;
  QVERIFY( gc.fromWkt( "GeometryCollection((CIRCULARSTRING( 0 0, 1 1, 2 2))" ) );
  QCOMPARE( gc.asWkt(), QStringLiteral( "GeometryCollection (GeometryCollection (CircularString (0 0, 1 1, 2 2)))" ) );
  QVERIFY( gc.fromWkt( "GeometryCollection(CIRCULARSTRING( 0 0, 1 1, 2 2)))" ) );
  QCOMPARE( gc.asWkt(), QStringLiteral( "GeometryCollection (CircularString (0 0, 1 1, 2 2))" ) );
  QVERIFY( gc.fromWkt( "GeometryCollection(CIRCULARSTRING( 0 0, 1 1, 2 2))" ) );
  QCOMPARE( gc.asWkt(), QStringLiteral( "GeometryCollection (CircularString (0 0, 1 1, 2 2))" ) );
  QVERIFY( gc.fromWkt( "GeometryCollection(CIRCULARSTRING( 0 0, 1 1, 2 2), LINESTRING((2 2, 3 3)))" ) );
  QCOMPARE( gc.asWkt(), QStringLiteral( "GeometryCollection (CircularString (0 0, 1 1, 2 2),LineString (2 2, 3 3))" ) );
  QVERIFY( gc.fromWkt( "GeometryCollection ( )" ) );
  QCOMPARE( gc.asWkt(), QStringLiteral( "GeometryCollection EMPTY" ) );
  QVERIFY( gc.fromWkt( "GeometryCollection EMPTY" ) );
  QCOMPARE( gc.asWkt(), QStringLiteral( "GeometryCollection EMPTY" ) );
  // curvepolygon
  QgsCurvePolygon cp;
  QVERIFY( ! cp.fromWkt( "CurvePolygon((CIRCULARSTRING( 0 0, 1 1, 2 2))" ) );
  QVERIFY( cp.fromWkt( "CurvePolygon(CIRCULARSTRING( 0 0, 1 1, 2 2)))" ) );
  QCOMPARE( cp.asWkt(), QStringLiteral( "CurvePolygon (CircularString (0 0, 1 1, 2 2))" ) );
  QVERIFY( cp.fromWkt( "CurvePolygon(CIRCULARSTRING( 0 0, 1 1, 2 2))" ) );
  QCOMPARE( cp.asWkt(), QStringLiteral( "CurvePolygon (CircularString (0 0, 1 1, 2 2))" ) );
  QVERIFY( cp.fromWkt( "CurvePolygon(CIRCULARSTRING( 0 0, 1 1, 2 2), LINESTRING((2 2, 3 3)))" ) );
  QCOMPARE( cp.asWkt(), QStringLiteral( "CurvePolygon (CircularString (0 0, 1 1, 2 2),(2 2, 3 3))" ) );
  QVERIFY( cp.fromWkt( "CurvePolygon ( )" ) );
  QCOMPARE( cp.asWkt(), QStringLiteral( "CurvePolygon EMPTY" ) );
  QVERIFY( cp.fromWkt( "CurvePolygon EMPTY" ) );
  QCOMPARE( cp.asWkt(), QStringLiteral( "CurvePolygon EMPTY" ) );
  // multicurve
  QgsMultiCurve mc;
  QVERIFY( ! mc.fromWkt( "MultiCurve((CIRCULARSTRING( 0 0, 1 1, 2 2))" ) );
  QVERIFY( mc.fromWkt( "MultiCurve(CIRCULARSTRING( 0 0, 1 1, 2 2)))" ) );
  QCOMPARE( mc.asWkt(), QStringLiteral( "MultiCurve (CircularString (0 0, 1 1, 2 2))" ) );
  QVERIFY( mc.fromWkt( "MultiCurve(CIRCULARSTRING( 0 0, 1 1, 2 2))" ) );
  QCOMPARE( mc.asWkt(), QStringLiteral( "MultiCurve (CircularString (0 0, 1 1, 2 2))" ) );
  QVERIFY( mc.fromWkt( "MultiCurve(CIRCULARSTRING( 0 0, 1 1, 2 2), LINESTRING((2 2, 3 3)))" ) );
  QCOMPARE( mc.asWkt(), QStringLiteral( "MultiCurve (CircularString (0 0, 1 1, 2 2),LineString (2 2, 3 3))" ) );
  QVERIFY( mc.fromWkt( "MultiCurve ( )" ) );
  QCOMPARE( mc.asWkt(), QStringLiteral( "MultiCurve EMPTY" ) );
  QVERIFY( mc.fromWkt( "MultiCurve EMPTY" ) );
  QCOMPARE( mc.asWkt(), QStringLiteral( "MultiCurve EMPTY" ) );
  // multisurface
  QgsMultiSurface ms;
  QVERIFY( ! ms.fromWkt( "MultiSurface((Polygon( 0 0, 1 1, 2 2))" ) );
  QVERIFY( ms.fromWkt( "MultiSurface(Polygon(( 0 0, 1 1, 2 2)))" ) );
  QCOMPARE( ms.asWkt(), QStringLiteral( "MultiSurface (Polygon ((0 0, 1 1, 2 2)))" ) );
  QVERIFY( ms.fromWkt( "MultiSurface(Polygon(( 0 0, 1 1, 2 2)))" ) );
  QCOMPARE( ms.asWkt(), QStringLiteral( "MultiSurface (Polygon ((0 0, 1 1, 2 2)))" ) );
  QVERIFY( ms.fromWkt( "MultiSurface(Polygon(( 0 0, 1 1, 2 2)), Polygon((2 2, 3 3, 3 2)))" ) );
  QCOMPARE( ms.asWkt(), QStringLiteral( "MultiSurface (Polygon ((0 0, 1 1, 2 2)),Polygon ((2 2, 3 3, 3 2)))" ) );
  QVERIFY( ms.fromWkt( "MultiSurface ( )" ) );
  QCOMPARE( ms.asWkt(), QStringLiteral( "MultiSurface EMPTY" ) );
  QVERIFY( ms.fromWkt( "MultiSurface EMPTY" ) );
  QCOMPARE( ms.asWkt(), QStringLiteral( "MultiSurface EMPTY" ) );
  // multipolygon
  QgsMultiPolygon mpoly;
  QVERIFY( ! mpoly.fromWkt( "MultiPolygon((Polygon( 0 0, 1 1, 2 2))" ) );
  QVERIFY( mpoly.fromWkt( "MultiPolygon(Polygon(( 0 0, 1 1, 2 2)))" ) );
  QCOMPARE( mpoly.asWkt(), QStringLiteral( "MultiPolygon (((0 0, 1 1, 2 2)))" ) );
  QVERIFY( mpoly.fromWkt( "MultiPolygon(Polygon(( 0 0, 1 1, 2 2)))" ) );
  QCOMPARE( mpoly.asWkt(), QStringLiteral( "MultiPolygon (((0 0, 1 1, 2 2)))" ) );
  QVERIFY( mpoly.fromWkt( "MultiPolygon(Polygon(( 0 0, 1 1, 2 2)), Polygon((2 2, 3 3, 3 2)))" ) );
  QCOMPARE( mpoly.asWkt(), QStringLiteral( "MultiPolygon (((0 0, 1 1, 2 2)),((2 2, 3 3, 3 2)))" ) );
  QVERIFY( mpoly.fromWkt( "MultiPolygon ( )" ) );
  QCOMPARE( mpoly.asWkt(), QStringLiteral( "MultiPolygon EMPTY" ) );
  QVERIFY( mpoly.fromWkt( "MultiPolygon EMPTY" ) );
  QCOMPARE( mpoly.asWkt(), QStringLiteral( "MultiPolygon EMPTY" ) );

  // multilinestring
  QgsMultiLineString mline;
  QVERIFY( ! mline.fromWkt( "MultiLineString((LineString( 0 0, 1 1, 2 2))" ) );
  QVERIFY( mline.fromWkt( "MultiLineString(LineString(( 0 0, 1 1, 2 2)))" ) );
  QCOMPARE( mline.asWkt(), QStringLiteral( "MultiLineString ((0 0, 1 1, 2 2))" ) );
  QVERIFY( mline.fromWkt( "MultiLineString(LineString(( 0 0, 1 1, 2 2)))" ) );
  QCOMPARE( mline.asWkt(), QStringLiteral( "MultiLineString ((0 0, 1 1, 2 2))" ) );
  QVERIFY( mline.fromWkt( "MultiLineString(LineString(( 0 0, 1 1, 2 2)), LineString((2 2, 3 3, 3 2)))" ) );
  QCOMPARE( mline.asWkt(), QStringLiteral( "MultiLineString ((0 0, 1 1, 2 2),(2 2, 3 3, 3 2))" ) );
  QVERIFY( mline.fromWkt( "MultiLineString ( )" ) );
  QCOMPARE( mline.asWkt(), QStringLiteral( "MultiLineString EMPTY" ) );
  QVERIFY( mline.fromWkt( "MultiLineString EMPTY" ) );
  QCOMPARE( mline.asWkt(), QStringLiteral( "MultiLineString EMPTY" ) );
}
QGSTEST_MAIN( TestQgsGeometry )
#include "testqgsgeometry.moc"
