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

    // geometry types
    void multiCurve();
    void multiSurface();
    void multiPolygon();
    void geometryCollection();

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
  QString expectedSimpleJson( "{\"coordinates\":[[[7.0,17.0],[6.9,17.0],[6.9,17.0],[6.8,17.0],[6.8,17.1],[6.7,17.1],[6.7,17.1],[6.6,17.1],[6.6,17.1],[6.5,17.1],[6.5,17.1],[6.4,17.1],[6.4,17.1],[6.3,17.1],[6.2,17.2],[6.2,17.2],[6.1,17.2],[6.1,17.2],[6.0,17.2],[6.0,17.2],[5.9,17.2],[5.9,17.2],[5.8,17.2],[5.7,17.2],[5.7,17.1],[5.6,17.1],[5.6,17.1],[5.5,17.1],[5.5,17.1],[5.4,17.1],[5.4,17.1],[5.3,17.1],[5.3,17.1],[5.2,17.1],[5.2,17.0],[5.1,17.0],[5.0,17.0],[5.0,17.0],[4.9,17.0],[4.9,17.0],[4.8,16.9],[4.8,16.9],[4.7,16.9],[4.7,16.9],[4.6,16.9],[4.6,16.8],[4.5,16.8],[4.5,16.8],[4.4,16.8],[4.4,16.7],[4.3,16.7],[4.3,16.7],[4.3,16.6],[4.2,16.6],[4.2,16.6],[4.1,16.5],[4.1,16.5],[4.0,16.5],[4.0,16.4],[3.9,16.4],[3.9,16.4],[3.9,16.3],[3.8,16.3],[3.8,16.3],[3.7,16.2],[3.7,16.2],[3.7,16.1],[3.6,16.1],[3.6,16.1],[3.6,16.0],[3.5,16.0],[3.5,15.9],[3.5,15.9],[3.4,15.8],[3.4,15.8],[3.4,15.7],[3.3,15.7],[3.3,15.7],[3.3,15.6],[3.2,15.6],[3.2,15.5],[3.2,15.5],[3.2,15.4],[3.1,15.4],[3.1,15.3],[3.1,15.3],[3.1,15.2],[3.1,15.2],[3.0,15.1],[3.0,15.1],[3.0,15.0],[3.0,15.0],[3.0,14.9],[3.0,14.8],[2.9,14.8],[2.9,14.7],[2.9,14.7],[2.9,14.6],[2.9,14.6],[2.9,14.5],[2.9,14.5],[2.9,14.4],[2.9,14.4],[2.9,14.3],[2.8,14.2],[2.8,14.2],[2.8,14.1],[2.8,14.1],[2.8,14.0],[2.8,14.0],[2.8,13.9],[2.8,13.9],[2.8,13.8],[2.8,13.8],[2.9,13.7],[2.9,13.6],[2.9,13.6],[2.9,13.5],[2.9,13.5],[2.9,13.4],[2.9,13.4],[2.9,13.3],[2.9,13.3],[2.9,13.2],[3.0,13.2],[3.0,13.1],[3.0,13.0],[3.0,13.0],[3.0,12.9],[3.0,12.9],[3.1,12.8],[3.1,12.8],[3.1,12.7],[3.1,12.7],[3.1,12.6],[3.2,12.6],[3.2,12.5],[3.2,12.5],[3.2,12.4],[3.3,12.4],[3.3,12.3],[3.3,12.3],[3.4,12.3],[3.4,12.2],[3.4,12.2],[3.5,12.1],[3.5,12.1],[3.5,12.0],[3.6,12.0],[3.6,11.9],[3.6,11.9],[3.7,11.9],[3.7,11.8],[3.7,11.8],[3.8,11.7],[3.8,11.7],[3.9,11.7],[3.9,11.6],[3.9,11.6],[4.0,11.6],[4.0,11.5],[4.1,11.5],[4.1,11.5],[4.2,11.4],[4.2,11.4],[4.3,11.4],[4.3,11.3],[4.3,11.3],[4.4,11.3],[4.4,11.2],[4.5,11.2],[4.5,11.2],[4.6,11.2],[4.6,11.1],[4.7,11.1],[4.7,11.1],[4.8,11.1],[4.8,11.1],[4.9,11.0],[4.9,11.0],[5.0,11.0],[5.0,11.0],[5.1,11.0],[5.2,11.0],[5.2,10.9],[5.3,10.9],[5.3,10.9],[5.4,10.9],[5.4,10.9],[5.5,10.9],[5.5,10.9],[5.6,10.9],[5.6,10.9],[5.7,10.9],[5.7,10.8],[5.8,10.8],[5.9,10.8],[5.9,10.8],[6.0,10.8],[6.0,10.8],[6.1,10.8],[6.1,10.8],[6.2,10.8],[6.2,10.8],[6.3,10.9],[6.4,10.9],[6.4,10.9],[6.5,10.9],[6.5,10.9],[6.6,10.9],[6.6,10.9],[6.7,10.9],[6.7,10.9],[6.8,10.9],[6.8,11.0],[6.9,11.0],[6.9,11.0],[7.0,11.0]],[[27.0,37.0],[27.1,36.9],[27.2,36.8],[27.3,36.6],[27.4,36.5],[27.5,36.4],[27.6,36.3],[27.7,36.2],[27.8,36.0],[27.9,35.9],[28.0,35.8],[28.1,35.7],[28.2,35.6],[28.3,35.5],[28.4,35.4],[28.5,35.3],[28.7,35.2],[28.8,35.1],[28.9,35.0],[29.0,34.9],[29.1,34.8],[29.3,34.7],[29.4,34.6],[29.5,34.6],[29.7,34.5],[29.8,34.4],[29.9,34.3],[30.1,34.3],[30.2,34.2],[30.3,34.1],[30.5,34.0],[30.6,34.0],[30.7,33.9],[30.9,33.9],[31.0,33.8],[31.2,33.7],[31.3,33.7],[31.5,33.6],[31.6,33.6],[31.8,33.6],[31.9,33.5],[32.1,33.5],[32.2,33.4],[32.4,33.4],[32.5,33.4],[32.7,33.3],[32.8,33.3],[33.0,33.3],[33.1,33.3],[33.3,33.2],[33.4,33.2],[33.6,33.2],[33.7,33.2],[33.9,33.2],[34.0,33.2],[34.2,33.2],[34.3,33.2],[34.5,33.2],[34.6,33.2],[34.8,33.2],[34.9,33.2],[35.1,33.2],[35.3,33.3],[35.4,33.3],[35.6,33.3],[35.7,33.3],[35.9,33.3],[36.0,33.4],[36.2,33.4],[36.3,33.4],[36.5,33.5],[36.6,33.5],[36.8,33.6],[36.9,33.6],[37.1,33.7],[37.2,33.7],[37.3,33.8],[37.5,33.8],[37.6,33.9],[37.8,33.9],[37.9,34.0],[38.0,34.1],[38.2,34.1],[38.3,34.2],[38.5,34.3],[38.6,34.3],[38.7,34.4],[38.9,34.5],[39.0,34.6],[39.1,34.7],[39.2,34.7],[39.4,34.8],[39.5,34.9],[39.6,35.0],[39.7,35.1],[39.9,35.2],[40.0,35.3],[40.1,35.4],[40.2,35.5],[40.3,35.6],[40.4,35.7],[40.5,35.8],[40.6,35.9],[40.7,36.1],[40.8,36.2],[40.9,36.3],[41.0,36.4],[41.1,36.5],[41.2,36.6],[41.3,36.8],[41.4,36.9],[41.5,37.0],[41.6,37.1],[41.7,37.3],[41.8,37.4],[41.8,37.5],[41.9,37.7],[42.0,37.8],[42.1,37.9],[42.1,38.1],[42.2,38.2],[42.3,38.3],[42.3,38.5],[42.4,38.6],[42.4,38.8],[42.5,38.9],[42.6,39.1],[42.6,39.2],[42.6,39.4],[42.7,39.5],[42.7,39.6],[42.8,39.8],[42.8,39.9],[42.8,40.1],[42.9,40.2],[42.9,40.4],[42.9,40.5],[43.0,40.7],[43.0,40.9],[43.0,41.0],[43.0,41.2],[43.0,41.3],[43.0,41.5],[43.0,41.6],[43.1,41.8],[43.1,41.9],[43.1,42.1],[43.1,42.2],[43.0,42.4],[43.0,42.5],[43.0,42.7],[43.0,42.8],[43.0,43.0],[43.0,43.1],[43.0,43.3],[42.9,43.5],[42.9,43.6],[42.9,43.8],[42.8,43.9],[42.8,44.1],[42.8,44.2],[42.7,44.4],[42.7,44.5],[42.6,44.6],[42.6,44.8],[42.6,44.9],[42.5,45.1],[42.4,45.2],[42.4,45.4],[42.3,45.5],[42.3,45.7],[42.2,45.8],[42.1,45.9],[42.1,46.1],[42.0,46.2],[41.9,46.3],[41.8,46.5],[41.8,46.6],[41.7,46.7],[41.6,46.9],[41.5,47.0],[41.4,47.1],[41.3,47.2],[41.2,47.4],[41.1,47.5],[41.0,47.6],[40.9,47.7],[40.8,47.8],[40.7,47.9],[40.6,48.1],[40.5,48.2],[40.4,48.3],[40.3,48.4],[40.2,48.5],[40.1,48.6],[40.0,48.7],[39.9,48.8],[39.7,48.9],[39.6,49.0],[39.5,49.1],[39.4,49.2],[39.2,49.3],[39.1,49.3],[39.0,49.4],[38.9,49.5],[38.7,49.6],[38.6,49.7],[38.5,49.7],[38.3,49.8],[38.2,49.9],[38.0,49.9],[37.9,50.0],[37.8,50.1],[37.6,50.1],[37.5,50.2],[37.3,50.2],[37.2,50.3],[37.1,50.3],[36.9,50.4],[36.8,50.4],[36.6,50.5],[36.5,50.5],[36.3,50.6],[36.2,50.6],[36.0,50.6],[35.9,50.7],[35.7,50.7],[35.6,50.7],[35.4,50.7],[35.3,50.7],[35.1,50.8],[34.9,50.8],[34.8,50.8],[34.6,50.8],[34.5,50.8],[34.3,50.8],[34.2,50.8],[34.0,50.8],[33.9,50.8],[33.7,50.8],[33.6,50.8],[33.4,50.8],[33.3,50.8],[33.1,50.7],[33.0,50.7],[32.8,50.7],[32.7,50.7],[32.5,50.6],[32.4,50.6],[32.2,50.6],[32.1,50.5],[31.9,50.5],[31.8,50.4],[31.6,50.4],[31.5,50.4],[31.3,50.3],[31.2,50.3],[31.0,50.2],[30.9,50.1],[30.7,50.1],[30.6,50.0],[30.5,50.0],[30.3,49.9],[30.2,49.8],[30.1,49.7],[29.9,49.7],[29.8,49.6],[29.7,49.5],[29.5,49.4],[29.4,49.4],[29.3,49.3],[29.1,49.2],[29.0,49.1],[28.9,49.0],[28.8,48.9],[28.7,48.8],[28.5,48.7],[28.4,48.6],[28.3,48.5],[28.2,48.4],[28.1,48.3],[28.0,48.2],[27.9,48.1],[27.8,48.0],[27.7,47.8],[27.6,47.7],[27.5,47.6],[27.4,47.5],[27.3,47.4],[27.2,47.2],[27.1,47.1],[27.0,47.0]]],\"type\":\"MultiLineString\"}" );
  res = exportC.asJson( 1 );
  QCOMPARE( res, expectedSimpleJson );

  QgsMultiCurve exportFloat;
  part.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::Point, 7 / 3.0, 17 / 3.0 ) << QgsPoint( QgsWkbTypes::Point, 3 / 5.0, 13 / 3.0 ) << QgsPoint( QgsWkbTypes::Point, 7 / 3.0, 11 / 3.0 ) ) ;
  exportFloat.addGeometry( part.clone() );
  part.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::Point, 27 / 3.0, 37 / 9.0 ) << QgsPoint( QgsWkbTypes::Point, 43 / 41.0, 43 / 42.0 ) << QgsPoint( QgsWkbTypes::Point, 27 / 3.0, 1 / 3.0 ) ) ;
  exportFloat.addGeometry( part.clone() );

  QString expectedJsonPrec3( QStringLiteral( "{\"coordinates\":[[[2.333,5.667],[2.316,5.677],[2.298,5.687],[2.28,5.697],[2.262,5.707],[2.244,5.716],[2.226,5.725],[2.207,5.734],[2.188,5.742],[2.17,5.75],[2.151,5.757],[2.131,5.765],[2.112,5.772],[2.093,5.778],[2.074,5.785],[2.054,5.79],[2.034,5.796],[2.015,5.801],[1.995,5.806],[1.975,5.811],[1.955,5.815],[1.935,5.819],[1.915,5.822],[1.894,5.826],[1.874,5.828],[1.854,5.831],[1.834,5.833],[1.813,5.835],[1.793,5.836],[1.773,5.837],[1.752,5.838],[1.732,5.838],[1.711,5.838],[1.691,5.838],[1.67,5.837],[1.65,5.836],[1.63,5.834],[1.609,5.833],[1.589,5.83],[1.569,5.828],[1.548,5.825],[1.528,5.822],[1.508,5.818],[1.488,5.814],[1.468,5.81],[1.448,5.805],[1.428,5.801],[1.409,5.795],[1.389,5.79],[1.37,5.784],[1.35,5.777],[1.331,5.771],[1.312,5.764],[1.293,5.756],[1.274,5.749],[1.255,5.741],[1.236,5.732],[1.218,5.724],[1.199,5.715],[1.181,5.705],[1.163,5.696],[1.145,5.686],[1.128,5.676],[1.11,5.665],[1.093,5.654],[1.076,5.643],[1.059,5.632],[1.042,5.62],[1.025,5.608],[1.009,5.596],[0.993,5.583],[0.977,5.57],[0.962,5.557],[0.946,5.543],[0.931,5.53],[0.916,5.516],[0.901,5.502],[0.887,5.487],[0.873,5.473],[0.859,5.458],[0.845,5.442],[0.832,5.427],[0.819,5.411],[0.806,5.395],[0.793,5.379],[0.781,5.363],[0.769,5.346],[0.757,5.33],[0.746,5.313],[0.735,5.296],[0.724,5.278],[0.713,5.261],[0.703,5.243],[0.693,5.225],[0.684,5.207],[0.674,5.189],[0.666,5.171],[0.657,5.152],[0.649,5.133],[0.641,5.115],[0.633,5.096],[0.626,5.077],[0.619,5.057],[0.612,5.038],[0.606,5.019],[0.6,4.999],[0.594,4.979],[0.589,4.96],[0.584,4.94],[0.579,4.92],[0.575,4.9],[0.571,4.88],[0.568,4.86],[0.564,4.84],[0.562,4.819],[0.559,4.799],[0.557,4.779],[0.555,4.759],[0.554,4.738],[0.553,4.718],[0.552,4.697],[0.552,4.677],[0.552,4.656],[0.552,4.636],[0.553,4.616],[0.554,4.595],[0.555,4.575],[0.557,4.554],[0.559,4.534],[0.562,4.514],[0.564,4.494],[0.568,4.473],[0.571,4.453],[0.575,4.433],[0.579,4.413],[0.584,4.393],[0.589,4.374],[0.594,4.354],[0.6,4.334],[0.606,4.315],[0.612,4.295],[0.619,4.276],[0.626,4.257],[0.633,4.238],[0.641,4.219],[0.649,4.2],[0.657,4.181],[0.666,4.163],[0.674,4.144],[0.684,4.126],[0.693,4.108],[0.703,4.09],[0.713,4.073],[0.724,4.055],[0.735,4.038],[0.746,4.021],[0.757,4.004],[0.769,3.987],[0.781,3.97],[0.793,3.954],[0.806,3.938],[0.819,3.922],[0.832,3.906],[0.845,3.891],[0.859,3.876],[0.873,3.861],[0.887,3.846],[0.901,3.832],[0.916,3.817],[0.931,3.804],[0.946,3.79],[0.962,3.776],[0.977,3.763],[0.993,3.75],[1.009,3.738],[1.025,3.726],[1.042,3.713],[1.059,3.702],[1.076,3.69],[1.093,3.679],[1.11,3.668],[1.128,3.658],[1.145,3.648],[1.163,3.638],[1.181,3.628],[1.199,3.619],[1.218,3.61],[1.236,3.601],[1.255,3.593],[1.274,3.585],[1.293,3.577],[1.312,3.57],[1.331,3.563],[1.35,3.556],[1.37,3.55],[1.389,3.544],[1.409,3.538],[1.428,3.533],[1.448,3.528],[1.468,3.523],[1.488,3.519],[1.508,3.515],[1.528,3.511],[1.548,3.508],[1.569,3.505],[1.589,3.503],[1.609,3.501],[1.63,3.499],[1.65,3.497],[1.67,3.496],[1.691,3.496],[1.711,3.495],[1.732,3.495],[1.752,3.496],[1.773,3.496],[1.793,3.497],[1.813,3.499],[1.834,3.5],[1.854,3.503],[1.874,3.505],[1.894,3.508],[1.915,3.511],[1.935,3.514],[1.955,3.518],[1.975,3.523],[1.995,3.527],[2.015,3.532],[2.034,3.537],[2.054,3.543],[2.074,3.549],[2.093,3.555],[2.112,3.562],[2.131,3.569],[2.151,3.576],[2.17,3.584],[2.188,3.592],[2.207,3.6],[2.226,3.608],[2.244,3.617],[2.262,3.627],[2.28,3.636],[2.298,3.646],[2.316,3.656],[2.333,3.667]],[[9.0,4.111],[8.966,4.178],[8.932,4.244],[8.896,4.309],[8.859,4.374],[8.821,4.438],[8.782,4.502],[8.742,4.565],[8.7,4.627],[8.658,4.688],[8.614,4.749],[8.57,4.809],[8.524,4.868],[8.477,4.926],[8.43,4.983],[8.381,5.04],[8.331,5.096],[8.281,5.151],[8.229,5.205],[8.177,5.258],[8.124,5.31],[8.069,5.361],[8.014,5.411],[7.958,5.461],[7.901,5.509],[7.844,5.556],[7.785,5.603],[7.726,5.648],[7.666,5.692],[7.605,5.735],[7.543,5.777],[7.481,5.818],[7.418,5.858],[7.354,5.897],[7.29,5.935],[7.225,5.971],[7.159,6.007],[7.093,6.041],[7.026,6.074],[6.958,6.106],[6.89,6.137],[6.822,6.167],[6.753,6.195],[6.683,6.222],[6.613,6.248],[6.543,6.273],[6.472,6.296],[6.401,6.319],[6.329,6.34],[6.257,6.36],[6.185,6.378],[6.112,6.395],[6.04,6.411],[5.966,6.426],[5.893,6.44],[5.819,6.452],[5.746,6.463],[5.672,6.472],[5.597,6.48],[5.523,6.487],[5.449,6.493],[5.374,6.498],[5.3,6.501],[5.225,6.503],[5.15,6.503],[5.076,6.502],[5.001,6.5],[4.927,6.497],[4.852,6.492],[4.778,6.486],[4.704,6.479],[4.629,6.47],[4.555,6.46],[4.482,6.449],[4.408,6.437],[4.335,6.423],[4.262,6.408],[4.189,6.392],[4.116,6.374],[4.044,6.355],[3.972,6.335],[3.901,6.314],[3.83,6.292],[3.759,6.268],[3.688,6.243],[3.619,6.217],[3.549,6.189],[3.48,6.16],[3.412,6.131],[3.344,6.1],[3.277,6.067],[3.21,6.034],[3.144,5.999],[3.078,5.964],[3.013,5.927],[2.949,5.889],[2.886,5.85],[2.823,5.81],[2.761,5.768],[2.699,5.726],[2.638,5.683],[2.578,5.638],[2.519,5.593],[2.461,5.546],[2.403,5.499],[2.347,5.45],[2.291,5.401],[2.236,5.35],[2.182,5.299],[2.129,5.246],[2.076,5.193],[2.025,5.139],[1.975,5.084],[1.925,5.028],[1.877,4.971],[1.829,4.914],[1.783,4.855],[1.738,4.796],[1.693,4.736],[1.65,4.675],[1.608,4.614],[1.567,4.551],[1.527,4.488],[1.488,4.425],[1.45,4.36],[1.413,4.295],[1.378,4.23],[1.343,4.164],[1.31,4.097],[1.278,4.029],[1.247,3.961],[1.217,3.893],[1.189,3.824],[1.161,3.755],[1.135,3.685],[1.11,3.614],[1.087,3.544],[1.064,3.472],[1.043,3.401],[1.023,3.329],[1.004,3.257],[0.987,3.184],[0.971,3.111],[0.956,3.038],[0.942,2.965],[0.93,2.891],[0.919,2.817],[0.909,2.743],[0.901,2.669],[0.894,2.595],[0.888,2.52],[0.883,2.446],[0.88,2.371],[0.878,2.297],[0.878,2.222],[0.878,2.148],[0.88,2.073],[0.883,1.998],[0.888,1.924],[0.894,1.85],[0.901,1.775],[0.909,1.701],[0.919,1.627],[0.93,1.553],[0.942,1.48],[0.956,1.406],[0.971,1.333],[0.987,1.26],[1.004,1.188],[1.023,1.116],[1.043,1.044],[1.064,0.972],[1.087,0.901],[1.11,0.83],[1.135,0.76],[1.161,0.69],[1.189,0.62],[1.217,0.551],[1.247,0.483],[1.278,0.415],[1.31,0.348],[1.343,0.281],[1.378,0.215],[1.413,0.149],[1.45,0.084],[1.488,0.02],[1.527,-0.044],[1.567,-0.107],[1.608,-0.169],[1.65,-0.231],[1.693,-0.291],[1.738,-0.351],[1.783,-0.411],[1.829,-0.469],[1.877,-0.527],[1.925,-0.584],[1.975,-0.639],[2.025,-0.694],[2.076,-0.749],[2.129,-0.802],[2.182,-0.854],[2.236,-0.906],[2.291,-0.956],[2.347,-1.006],[2.403,-1.054],[2.461,-1.102],[2.519,-1.148],[2.578,-1.194],[2.638,-1.238],[2.699,-1.282],[2.761,-1.324],[2.823,-1.365],[2.886,-1.405],[2.949,-1.444],[3.013,-1.482],[3.078,-1.519],[3.144,-1.555],[3.21,-1.589],[3.277,-1.623],[3.344,-1.655],[3.412,-1.686],[3.48,-1.716],[3.549,-1.745],[3.619,-1.772],[3.688,-1.798],[3.759,-1.823],[3.83,-1.847],[3.901,-1.87],[3.972,-1.891],[4.044,-1.911],[4.116,-1.93],[4.189,-1.947],[4.262,-1.964],[4.335,-1.979],[4.408,-1.992],[4.482,-2.005],[4.555,-2.016],[4.629,-2.026],[4.704,-2.034],[4.778,-2.042],[4.852,-2.048],[4.927,-2.052],[5.001,-2.056],[5.076,-2.058],[5.15,-2.059],[5.225,-2.058],[5.3,-2.056],[5.374,-2.053],[5.449,-2.049],[5.523,-2.043],[5.597,-2.036],[5.672,-2.028],[5.746,-2.018],[5.819,-2.007],[5.893,-1.995],[5.966,-1.982],[6.04,-1.967],[6.112,-1.951],[6.185,-1.934],[6.257,-1.915],[6.329,-1.895],[6.401,-1.874],[6.472,-1.852],[6.543,-1.829],[6.613,-1.804],[6.683,-1.778],[6.753,-1.751],[6.822,-1.722],[6.89,-1.693],[6.958,-1.662],[7.026,-1.63],[7.093,-1.597],[7.159,-1.562],[7.225,-1.527],[7.29,-1.49],[7.354,-1.453],[7.418,-1.414],[7.481,-1.374],[7.543,-1.333],[7.605,-1.291],[7.666,-1.248],[7.726,-1.203],[7.785,-1.158],[7.844,-1.112],[7.901,-1.065],[7.958,-1.016],[8.014,-0.967],[8.069,-0.917],[8.124,-0.865],[8.177,-0.813],[8.229,-0.76],[8.281,-0.706],[8.331,-0.651],[8.381,-0.596],[8.43,-0.539],[8.477,-0.482],[8.524,-0.423],[8.57,-0.364],[8.614,-0.304],[8.658,-0.244],[8.7,-0.182],[8.742,-0.12],[8.782,-0.057],[8.821,0.006],[8.859,0.07],[8.896,0.135],[8.932,0.201],[8.966,0.267],[9.0,0.333]]],\"type\":\"MultiLineString\"}" ) );
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

  //asKML
  QString expectedKml( QStringLiteral( "<MultiGeometry><LineString><altitudeMode>clampToGround</altitudeMode><coordinates>7,17,0 6.9,17,0 6.9,17,0 6.8,17,0 6.8,17.1,0 6.7,17.1,0 6.7,17.1,0 6.6,17.1,0 6.6,17.1,0 6.5,17.1,0 6.5,17.1,0 6.4,17.1,0 6.4,17.1,0 6.3,17.1,0 6.2,17.2,0 6.2,17.2,0 6.1,17.2,0 6.1,17.2,0 6,17.2,0 6,17.2,0 5.9,17.2,0 5.9,17.2,0 5.8,17.2,0 5.7,17.2,0 5.7,17.1,0 5.6,17.1,0 5.6,17.1,0 5.5,17.1,0 5.5,17.1,0 5.4,17.1,0 5.4,17.1,0 5.3,17.1,0 5.3,17.1,0 5.2,17.1,0 5.2,17,0 5.1,17,0 5,17,0 5,17,0 4.9,17,0 4.9,17,0 4.8,16.9,0 4.8,16.9,0 4.7,16.9,0 4.7,16.9,0 4.6,16.9,0 4.6,16.8,0 4.5,16.8,0 4.5,16.8,0 4.4,16.8,0 4.4,16.7,0 4.3,16.7,0 4.3,16.7,0 4.3,16.6,0 4.2,16.6,0 4.2,16.6,0 4.1,16.5,0 4.1,16.5,0 4,16.5,0 4,16.4,0 3.9,16.4,0 3.9,16.4,0 3.9,16.3,0 3.8,16.3,0 3.8,16.3,0 3.7,16.2,0 3.7,16.2,0 3.7,16.1,0 3.6,16.1,0 3.6,16.1,0 3.6,16,0 3.5,16,0 3.5,15.9,0 3.5,15.9,0 3.4,15.8,0 3.4,15.8,0 3.4,15.7,0 3.3,15.7,0 3.3,15.7,0 3.3,15.6,0 3.2,15.6,0 3.2,15.5,0 3.2,15.5,0 3.2,15.4,0 3.1,15.4,0 3.1,15.3,0 3.1,15.3,0 3.1,15.2,0 3.1,15.2,0 3,15.1,0 3,15.1,0 3,15,0 3,15,0 3,14.9,0 3,14.8,0 2.9,14.8,0 2.9,14.7,0 2.9,14.7,0 2.9,14.6,0 2.9,14.6,0 2.9,14.5,0 2.9,14.5,0 2.9,14.4,0 2.9,14.4,0 2.9,14.3,0 2.8,14.2,0 2.8,14.2,0 2.8,14.1,0 2.8,14.1,0 2.8,14,0 2.8,14,0 2.8,13.9,0 2.8,13.9,0 2.8,13.8,0 2.8,13.8,0 2.9,13.7,0 2.9,13.6,0 2.9,13.6,0 2.9,13.5,0 2.9,13.5,0 2.9,13.4,0 2.9,13.4,0 2.9,13.3,0 2.9,13.3,0 2.9,13.2,0 3,13.2,0 3,13.1,0 3,13,0 3,13,0 3,12.9,0 3,12.9,0 3.1,12.8,0 3.1,12.8,0 3.1,12.7,0 3.1,12.7,0 3.1,12.6,0 3.2,12.6,0 3.2,12.5,0 3.2,12.5,0 3.2,12.4,0 3.3,12.4,0 3.3,12.3,0 3.3,12.3,0 3.4,12.3,0 3.4,12.2,0 3.4,12.2,0 3.5,12.1,0 3.5,12.1,0 3.5,12,0 3.6,12,0 3.6,11.9,0 3.6,11.9,0 3.7,11.9,0 3.7,11.8,0 3.7,11.8,0 3.8,11.7,0 3.8,11.7,0 3.9,11.7,0 3.9,11.6,0 3.9,11.6,0 4,11.6,0 4,11.5,0 4.1,11.5,0 4.1,11.5,0 4.2,11.4,0 4.2,11.4,0 4.3,11.4,0 4.3,11.3,0 4.3,11.3,0 4.4,11.3,0 4.4,11.2,0 4.5,11.2,0 4.5,11.2,0 4.6,11.2,0 4.6,11.1,0 4.7,11.1,0 4.7,11.1,0 4.8,11.1,0 4.8,11.1,0 4.9,11,0 4.9,11,0 5,11,0 5,11,0 5.1,11,0 5.2,11,0 5.2,10.9,0 5.3,10.9,0 5.3,10.9,0 5.4,10.9,0 5.4,10.9,0 5.5,10.9,0 5.5,10.9,0 5.6,10.9,0 5.6,10.9,0 5.7,10.9,0 5.7,10.8,0 5.8,10.8,0 5.9,10.8,0 5.9,10.8,0 6,10.8,0 6,10.8,0 6.1,10.8,0 6.1,10.8,0 6.2,10.8,0 6.2,10.8,0 6.3,10.9,0 6.4,10.9,0 6.4,10.9,0 6.5,10.9,0 6.5,10.9,0 6.6,10.9,0 6.6,10.9,0 6.7,10.9,0 6.7,10.9,0 6.8,10.9,0 6.8,11,0 6.9,11,0 6.9,11,0 7,11,0</coordinates></LineString><LineString><altitudeMode>clampToGround</altitudeMode><coordinates>27,37,0 27.1,36.9,0 27.2,36.8,0 27.3,36.6,0 27.4,36.5,0 27.5,36.4,0 27.6,36.3,0 27.7,36.2,0 27.8,36,0 27.9,35.9,0 28,35.8,0 28.1,35.7,0 28.2,35.6,0 28.3,35.5,0 28.4,35.4,0 28.5,35.3,0 28.7,35.2,0 28.8,35.1,0 28.9,35,0 29,34.9,0 29.1,34.8,0 29.3,34.7,0 29.4,34.6,0 29.5,34.6,0 29.7,34.5,0 29.8,34.4,0 29.9,34.3,0 30.1,34.3,0 30.2,34.2,0 30.3,34.1,0 30.5,34,0 30.6,34,0 30.7,33.9,0 30.9,33.9,0 31,33.8,0 31.2,33.7,0 31.3,33.7,0 31.5,33.6,0 31.6,33.6,0 31.8,33.6,0 31.9,33.5,0 32.1,33.5,0 32.2,33.4,0 32.4,33.4,0 32.5,33.4,0 32.7,33.3,0 32.8,33.3,0 33,33.3,0 33.1,33.3,0 33.3,33.2,0 33.4,33.2,0 33.6,33.2,0 33.7,33.2,0 33.9,33.2,0 34,33.2,0 34.2,33.2,0 34.3,33.2,0 34.5,33.2,0 34.6,33.2,0 34.8,33.2,0 34.9,33.2,0 35.1,33.2,0 35.3,33.3,0 35.4,33.3,0 35.6,33.3,0 35.7,33.3,0 35.9,33.3,0 36,33.4,0 36.2,33.4,0 36.3,33.4,0 36.5,33.5,0 36.6,33.5,0 36.8,33.6,0 36.9,33.6,0 37.1,33.7,0 37.2,33.7,0 37.3,33.8,0 37.5,33.8,0 37.6,33.9,0 37.8,33.9,0 37.9,34,0 38,34.1,0 38.2,34.1,0 38.3,34.2,0 38.5,34.3,0 38.6,34.3,0 38.7,34.4,0 38.9,34.5,0 39,34.6,0 39.1,34.7,0 39.2,34.7,0 39.4,34.8,0 39.5,34.9,0 39.6,35,0 39.7,35.1,0 39.9,35.2,0 40,35.3,0 40.1,35.4,0 40.2,35.5,0 40.3,35.6,0 40.4,35.7,0 40.5,35.8,0 40.6,35.9,0 40.7,36.1,0 40.8,36.2,0 40.9,36.3,0 41,36.4,0 41.1,36.5,0 41.2,36.6,0 41.3,36.8,0 41.4,36.9,0 41.5,37,0 41.6,37.1,0 41.7,37.3,0 41.8,37.4,0 41.8,37.5,0 41.9,37.7,0 42,37.8,0 42.1,37.9,0 42.1,38.1,0 42.2,38.2,0 42.3,38.3,0 42.3,38.5,0 42.4,38.6,0 42.4,38.8,0 42.5,38.9,0 42.6,39.1,0 42.6,39.2,0 42.6,39.4,0 42.7,39.5,0 42.7,39.6,0 42.8,39.8,0 42.8,39.9,0 42.8,40.1,0 42.9,40.2,0 42.9,40.4,0 42.9,40.5,0 43,40.7,0 43,40.9,0 43,41,0 43,41.2,0 43,41.3,0 43,41.5,0 43,41.6,0 43.1,41.8,0 43.1,41.9,0 43.1,42.1,0 43.1,42.2,0 43,42.4,0 43,42.5,0 43,42.7,0 43,42.8,0 43,43,0 43,43.1,0 43,43.3,0 42.9,43.5,0 42.9,43.6,0 42.9,43.8,0 42.8,43.9,0 42.8,44.1,0 42.8,44.2,0 42.7,44.4,0 42.7,44.5,0 42.6,44.6,0 42.6,44.8,0 42.6,44.9,0 42.5,45.1,0 42.4,45.2,0 42.4,45.4,0 42.3,45.5,0 42.3,45.7,0 42.2,45.8,0 42.1,45.9,0 42.1,46.1,0 42,46.2,0 41.9,46.3,0 41.8,46.5,0 41.8,46.6,0 41.7,46.7,0 41.6,46.9,0 41.5,47,0 41.4,47.1,0 41.3,47.2,0 41.2,47.4,0 41.1,47.5,0 41,47.6,0 40.9,47.7,0 40.8,47.8,0 40.7,47.9,0 40.6,48.1,0 40.5,48.2,0 40.4,48.3,0 40.3,48.4,0 40.2,48.5,0 40.1,48.6,0 40,48.7,0 39.9,48.8,0 39.7,48.9,0 39.6,49,0 39.5,49.1,0 39.4,49.2,0 39.2,49.3,0 39.1,49.3,0 39,49.4,0 38.9,49.5,0 38.7,49.6,0 38.6,49.7,0 38.5,49.7,0 38.3,49.8,0 38.2,49.9,0 38,49.9,0 37.9,50,0 37.8,50.1,0 37.6,50.1,0 37.5,50.2,0 37.3,50.2,0 37.2,50.3,0 37.1,50.3,0 36.9,50.4,0 36.8,50.4,0 36.6,50.5,0 36.5,50.5,0 36.3,50.6,0 36.2,50.6,0 36,50.6,0 35.9,50.7,0 35.7,50.7,0 35.6,50.7,0 35.4,50.7,0 35.3,50.7,0 35.1,50.8,0 34.9,50.8,0 34.8,50.8,0 34.6,50.8,0 34.5,50.8,0 34.3,50.8,0 34.2,50.8,0 34,50.8,0 33.9,50.8,0 33.7,50.8,0 33.6,50.8,0 33.4,50.8,0 33.3,50.8,0 33.1,50.7,0 33,50.7,0 32.8,50.7,0 32.7,50.7,0 32.5,50.6,0 32.4,50.6,0 32.2,50.6,0 32.1,50.5,0 31.9,50.5,0 31.8,50.4,0 31.6,50.4,0 31.5,50.4,0 31.3,50.3,0 31.2,50.3,0 31,50.2,0 30.9,50.1,0 30.7,50.1,0 30.6,50,0 30.5,50,0 30.3,49.9,0 30.2,49.8,0 30.1,49.7,0 29.9,49.7,0 29.8,49.6,0 29.7,49.5,0 29.5,49.4,0 29.4,49.4,0 29.3,49.3,0 29.1,49.2,0 29,49.1,0 28.9,49,0 28.8,48.9,0 28.7,48.8,0 28.5,48.7,0 28.4,48.6,0 28.3,48.5,0 28.2,48.4,0 28.1,48.3,0 28,48.2,0 27.9,48.1,0 27.8,48,0 27.7,47.8,0 27.6,47.7,0 27.5,47.6,0 27.4,47.5,0 27.3,47.4,0 27.2,47.2,0 27.1,47.1,0 27,47,0</coordinates></LineString></MultiGeometry>" ) );
  QCOMPARE( exportC.asKml( 1 ), expectedKml );
  QString expectedKmlPrec3( QStringLiteral( "<MultiGeometry><LineString><altitudeMode>clampToGround</altitudeMode><coordinates>2.333,5.667,0 2.316,5.677,0 2.298,5.687,0 2.28,5.697,0 2.262,5.707,0 2.244,5.716,0 2.226,5.725,0 2.207,5.734,0 2.188,5.742,0 2.17,5.75,0 2.151,5.757,0 2.131,5.765,0 2.112,5.772,0 2.093,5.778,0 2.074,5.785,0 2.054,5.79,0 2.034,5.796,0 2.015,5.801,0 1.995,5.806,0 1.975,5.811,0 1.955,5.815,0 1.935,5.819,0 1.915,5.822,0 1.894,5.826,0 1.874,5.828,0 1.854,5.831,0 1.834,5.833,0 1.813,5.835,0 1.793,5.836,0 1.773,5.837,0 1.752,5.838,0 1.732,5.838,0 1.711,5.838,0 1.691,5.838,0 1.67,5.837,0 1.65,5.836,0 1.63,5.834,0 1.609,5.833,0 1.589,5.83,0 1.569,5.828,0 1.548,5.825,0 1.528,5.822,0 1.508,5.818,0 1.488,5.814,0 1.468,5.81,0 1.448,5.805,0 1.428,5.801,0 1.409,5.795,0 1.389,5.79,0 1.37,5.784,0 1.35,5.777,0 1.331,5.771,0 1.312,5.764,0 1.293,5.756,0 1.274,5.749,0 1.255,5.741,0 1.236,5.732,0 1.218,5.724,0 1.199,5.715,0 1.181,5.705,0 1.163,5.696,0 1.145,5.686,0 1.128,5.676,0 1.11,5.665,0 1.093,5.654,0 1.076,5.643,0 1.059,5.632,0 1.042,5.62,0 1.025,5.608,0 1.009,5.596,0 0.993,5.583,0 0.977,5.57,0 0.962,5.557,0 0.946,5.543,0 0.931,5.53,0 0.916,5.516,0 0.901,5.502,0 0.887,5.487,0 0.873,5.473,0 0.859,5.458,0 0.845,5.442,0 0.832,5.427,0 0.819,5.411,0 0.806,5.395,0 0.793,5.379,0 0.781,5.363,0 0.769,5.346,0 0.757,5.33,0 0.746,5.313,0 0.735,5.296,0 0.724,5.278,0 0.713,5.261,0 0.703,5.243,0 0.693,5.225,0 0.684,5.207,0 0.674,5.189,0 0.666,5.171,0 0.657,5.152,0 0.649,5.133,0 0.641,5.115,0 0.633,5.096,0 0.626,5.077,0 0.619,5.057,0 0.612,5.038,0 0.606,5.019,0 0.6,4.999,0 0.594,4.979,0 0.589,4.96,0 0.584,4.94,0 0.579,4.92,0 0.575,4.9,0 0.571,4.88,0 0.568,4.86,0 0.564,4.84,0 0.562,4.819,0 0.559,4.799,0 0.557,4.779,0 0.555,4.759,0 0.554,4.738,0 0.553,4.718,0 0.552,4.697,0 0.552,4.677,0 0.552,4.656,0 0.552,4.636,0 0.553,4.616,0 0.554,4.595,0 0.555,4.575,0 0.557,4.554,0 0.559,4.534,0 0.562,4.514,0 0.564,4.494,0 0.568,4.473,0 0.571,4.453,0 0.575,4.433,0 0.579,4.413,0 0.584,4.393,0 0.589,4.374,0 0.594,4.354,0 0.6,4.334,0 0.606,4.315,0 0.612,4.295,0 0.619,4.276,0 0.626,4.257,0 0.633,4.238,0 0.641,4.219,0 0.649,4.2,0 0.657,4.181,0 0.666,4.163,0 0.674,4.144,0 0.684,4.126,0 0.693,4.108,0 0.703,4.09,0 0.713,4.073,0 0.724,4.055,0 0.735,4.038,0 0.746,4.021,0 0.757,4.004,0 0.769,3.987,0 0.781,3.97,0 0.793,3.954,0 0.806,3.938,0 0.819,3.922,0 0.832,3.906,0 0.845,3.891,0 0.859,3.876,0 0.873,3.861,0 0.887,3.846,0 0.901,3.832,0 0.916,3.817,0 0.931,3.804,0 0.946,3.79,0 0.962,3.776,0 0.977,3.763,0 0.993,3.75,0 1.009,3.738,0 1.025,3.726,0 1.042,3.713,0 1.059,3.702,0 1.076,3.69,0 1.093,3.679,0 1.11,3.668,0 1.128,3.658,0 1.145,3.648,0 1.163,3.638,0 1.181,3.628,0 1.199,3.619,0 1.218,3.61,0 1.236,3.601,0 1.255,3.593,0 1.274,3.585,0 1.293,3.577,0 1.312,3.57,0 1.331,3.563,0 1.35,3.556,0 1.37,3.55,0 1.389,3.544,0 1.409,3.538,0 1.428,3.533,0 1.448,3.528,0 1.468,3.523,0 1.488,3.519,0 1.508,3.515,0 1.528,3.511,0 1.548,3.508,0 1.569,3.505,0 1.589,3.503,0 1.609,3.501,0 1.63,3.499,0 1.65,3.497,0 1.67,3.496,0 1.691,3.496,0 1.711,3.495,0 1.732,3.495,0 1.752,3.496,0 1.773,3.496,0 1.793,3.497,0 1.813,3.499,0 1.834,3.5,0 1.854,3.503,0 1.874,3.505,0 1.894,3.508,0 1.915,3.511,0 1.935,3.514,0 1.955,3.518,0 1.975,3.523,0 1.995,3.527,0 2.015,3.532,0 2.034,3.537,0 2.054,3.543,0 2.074,3.549,0 2.093,3.555,0 2.112,3.562,0 2.131,3.569,0 2.151,3.576,0 2.17,3.584,0 2.188,3.592,0 2.207,3.6,0 2.226,3.608,0 2.244,3.617,0 2.262,3.627,0 2.28,3.636,0 2.298,3.646,0 2.316,3.656,0 2.333,3.667,0</coordinates></LineString><LineString><altitudeMode>clampToGround</altitudeMode><coordinates>9,4.111,0 8.966,4.178,0 8.932,4.244,0 8.896,4.309,0 8.859,4.374,0 8.821,4.438,0 8.782,4.502,0 8.742,4.565,0 8.7,4.627,0 8.658,4.688,0 8.614,4.749,0 8.57,4.809,0 8.524,4.868,0 8.477,4.926,0 8.43,4.983,0 8.381,5.04,0 8.331,5.096,0 8.281,5.151,0 8.229,5.205,0 8.177,5.258,0 8.124,5.31,0 8.069,5.361,0 8.014,5.411,0 7.958,5.461,0 7.901,5.509,0 7.844,5.556,0 7.785,5.603,0 7.726,5.648,0 7.666,5.692,0 7.605,5.735,0 7.543,5.777,0 7.481,5.818,0 7.418,5.858,0 7.354,5.897,0 7.29,5.935,0 7.225,5.971,0 7.159,6.007,0 7.093,6.041,0 7.026,6.074,0 6.958,6.106,0 6.89,6.137,0 6.822,6.167,0 6.753,6.195,0 6.683,6.222,0 6.613,6.248,0 6.543,6.273,0 6.472,6.296,0 6.401,6.319,0 6.329,6.34,0 6.257,6.36,0 6.185,6.378,0 6.112,6.395,0 6.04,6.411,0 5.966,6.426,0 5.893,6.44,0 5.819,6.452,0 5.746,6.463,0 5.672,6.472,0 5.597,6.48,0 5.523,6.487,0 5.449,6.493,0 5.374,6.498,0 5.3,6.501,0 5.225,6.503,0 5.15,6.503,0 5.076,6.502,0 5.001,6.5,0 4.927,6.497,0 4.852,6.492,0 4.778,6.486,0 4.704,6.479,0 4.629,6.47,0 4.555,6.46,0 4.482,6.449,0 4.408,6.437,0 4.335,6.423,0 4.262,6.408,0 4.189,6.392,0 4.116,6.374,0 4.044,6.355,0 3.972,6.335,0 3.901,6.314,0 3.83,6.292,0 3.759,6.268,0 3.688,6.243,0 3.619,6.217,0 3.549,6.189,0 3.48,6.16,0 3.412,6.131,0 3.344,6.1,0 3.277,6.067,0 3.21,6.034,0 3.144,5.999,0 3.078,5.964,0 3.013,5.927,0 2.949,5.889,0 2.886,5.85,0 2.823,5.81,0 2.761,5.768,0 2.699,5.726,0 2.638,5.683,0 2.578,5.638,0 2.519,5.593,0 2.461,5.546,0 2.403,5.499,0 2.347,5.45,0 2.291,5.401,0 2.236,5.35,0 2.182,5.299,0 2.129,5.246,0 2.076,5.193,0 2.025,5.139,0 1.975,5.084,0 1.925,5.028,0 1.877,4.971,0 1.829,4.914,0 1.783,4.855,0 1.738,4.796,0 1.693,4.736,0 1.65,4.675,0 1.608,4.614,0 1.567,4.551,0 1.527,4.488,0 1.488,4.425,0 1.45,4.36,0 1.413,4.295,0 1.378,4.23,0 1.343,4.164,0 1.31,4.097,0 1.278,4.029,0 1.247,3.961,0 1.217,3.893,0 1.189,3.824,0 1.161,3.755,0 1.135,3.685,0 1.11,3.614,0 1.087,3.544,0 1.064,3.472,0 1.043,3.401,0 1.023,3.329,0 1.004,3.257,0 0.987,3.184,0 0.971,3.111,0 0.956,3.038,0 0.942,2.965,0 0.93,2.891,0 0.919,2.817,0 0.909,2.743,0 0.901,2.669,0 0.894,2.595,0 0.888,2.52,0 0.883,2.446,0 0.88,2.371,0 0.878,2.297,0 0.878,2.222,0 0.878,2.148,0 0.88,2.073,0 0.883,1.998,0 0.888,1.924,0 0.894,1.85,0 0.901,1.775,0 0.909,1.701,0 0.919,1.627,0 0.93,1.553,0 0.942,1.48,0 0.956,1.406,0 0.971,1.333,0 0.987,1.26,0 1.004,1.188,0 1.023,1.116,0 1.043,1.044,0 1.064,0.972,0 1.087,0.901,0 1.11,0.83,0 1.135,0.76,0 1.161,0.69,0 1.189,0.62,0 1.217,0.551,0 1.247,0.483,0 1.278,0.415,0 1.31,0.348,0 1.343,0.281,0 1.378,0.215,0 1.413,0.149,0 1.45,0.084,0 1.488,0.02,0 1.527,-0.044,0 1.567,-0.107,0 1.608,-0.169,0 1.65,-0.231,0 1.693,-0.291,0 1.738,-0.351,0 1.783,-0.411,0 1.829,-0.469,0 1.877,-0.527,0 1.925,-0.584,0 1.975,-0.639,0 2.025,-0.694,0 2.076,-0.749,0 2.129,-0.802,0 2.182,-0.854,0 2.236,-0.906,0 2.291,-0.956,0 2.347,-1.006,0 2.403,-1.054,0 2.461,-1.102,0 2.519,-1.148,0 2.578,-1.194,0 2.638,-1.238,0 2.699,-1.282,0 2.761,-1.324,0 2.823,-1.365,0 2.886,-1.405,0 2.949,-1.444,0 3.013,-1.482,0 3.078,-1.519,0 3.144,-1.555,0 3.21,-1.589,0 3.277,-1.623,0 3.344,-1.655,0 3.412,-1.686,0 3.48,-1.716,0 3.549,-1.745,0 3.619,-1.772,0 3.688,-1.798,0 3.759,-1.823,0 3.83,-1.847,0 3.901,-1.87,0 3.972,-1.891,0 4.044,-1.911,0 4.116,-1.93,0 4.189,-1.947,0 4.262,-1.964,0 4.335,-1.979,0 4.408,-1.992,0 4.482,-2.005,0 4.555,-2.016,0 4.629,-2.026,0 4.704,-2.034,0 4.778,-2.042,0 4.852,-2.048,0 4.927,-2.052,0 5.001,-2.056,0 5.076,-2.058,0 5.15,-2.059,0 5.225,-2.058,0 5.3,-2.056,0 5.374,-2.053,0 5.449,-2.049,0 5.523,-2.043,0 5.597,-2.036,0 5.672,-2.028,0 5.746,-2.018,0 5.819,-2.007,0 5.893,-1.995,0 5.966,-1.982,0 6.04,-1.967,0 6.112,-1.951,0 6.185,-1.934,0 6.257,-1.915,0 6.329,-1.895,0 6.401,-1.874,0 6.472,-1.852,0 6.543,-1.829,0 6.613,-1.804,0 6.683,-1.778,0 6.753,-1.751,0 6.822,-1.722,0 6.89,-1.693,0 6.958,-1.662,0 7.026,-1.63,0 7.093,-1.597,0 7.159,-1.562,0 7.225,-1.527,0 7.29,-1.49,0 7.354,-1.453,0 7.418,-1.414,0 7.481,-1.374,0 7.543,-1.333,0 7.605,-1.291,0 7.666,-1.248,0 7.726,-1.203,0 7.785,-1.158,0 7.844,-1.112,0 7.901,-1.065,0 7.958,-1.016,0 8.014,-0.967,0 8.069,-0.917,0 8.124,-0.865,0 8.177,-0.813,0 8.229,-0.76,0 8.281,-0.706,0 8.331,-0.651,0 8.381,-0.596,0 8.43,-0.539,0 8.477,-0.482,0 8.524,-0.423,0 8.57,-0.364,0 8.614,-0.304,0 8.658,-0.244,0 8.7,-0.182,0 8.742,-0.12,0 8.782,-0.057,0 8.821,0.006,0 8.859,0.07,0 8.896,0.135,0 8.932,0.201,0 8.966,0.267,0 9,0.333,0</coordinates></LineString></MultiGeometry>" ) );
  QCOMPARE( exportFloat.asKml( 3 ), expectedKmlPrec3 );

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
  boundaryLine1.setPoints( QgsPointSequence() << QgsPoint( 0, 0 ) << QgsPoint( 1, 0 ) << QgsPoint( 1, 1 ) );
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
  boundaryLine2.setPoints( QgsPointSequence() << QgsPoint( 10, 10 ) << QgsPoint( 11, 10 ) << QgsPoint( 11, 11 ) );
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
  boundaryLine3.setPoints( QgsPointSequence() << QgsPoint( 20, 20 ) << QgsPoint( 21, 20 ) <<  QgsPoint( 20, 20 ) );
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
  boundaryLine4.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZ, 0, 0, 10 ) << QgsPoint( QgsWkbTypes::PointZ, 1, 0, 15 ) << QgsPoint( QgsWkbTypes::PointZ, 1, 1, 20 ) );
  QgsCircularString boundaryLine5;
  boundaryLine5.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZ, 10, 10, 100 ) << QgsPoint( QgsWkbTypes::PointZ, 10, 20, 150 ) << QgsPoint( QgsWkbTypes::PointZ, 20, 20, 200 ) );
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

  // segmentize
  QgsMultiCurve multiCurve2;
  QgsCompoundCurve compoundCurve2;
  QgsCircularString circularString2;
  circularString2.setPoints( QgsPointSequence() << QgsPoint( 1, 2 ) << QgsPoint( 11, 10 ) << QgsPoint( 21, 2 ) );
  compoundCurve2.addCurve( circularString2.clone() );
  multiCurve2.addGeometry( compoundCurve2.clone() );
  QgsMultiLineString *segmentized2 = static_cast<QgsMultiLineString *>( multiCurve2.segmentize() );
  QCOMPARE( segmentized2->vertexCount(), 156 );
  QCOMPARE( segmentized2->partCount(), 1 );
  QVERIFY( !segmentized2->is3D() );
  QVERIFY( !segmentized2->isMeasure() );
  QCOMPARE( segmentized2->wkbType(),  QgsWkbTypes::Type::MultiLineString );
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
  QgsLineString lineRing;
  lineRing.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::Point, 7, 17 ) << QgsPoint( QgsWkbTypes::Point, 7, 13 )  << QgsPoint( QgsWkbTypes::Point, 3, 13 ) << QgsPoint( QgsWkbTypes::Point, 7, 17 ) ) ;
  part.clear();
  part.setExteriorRing( lineRing.clone() );
  exportC.addGeometry( part.clone() );
  lineRing.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::Point, 27, 37 ) << QgsPoint( QgsWkbTypes::Point, 27, 43 ) << QgsPoint( QgsWkbTypes::Point, 43, 43 )  << QgsPoint( QgsWkbTypes::Point, 27, 37 ) ) ;
  part.clear();
  part.setExteriorRing( lineRing.clone() );
  exportC.addGeometry( part.clone() );

  // GML document for compare
  QDomDocument doc( "gml" );

  // as GML2
  QString expectedSimpleGML2( QStringLiteral( "<MultiPolygon xmlns=\"gml\"><polygonMember xmlns=\"gml\"><Polygon xmlns=\"gml\"><outerBoundaryIs xmlns=\"gml\"><LinearRing xmlns=\"gml\"><coordinates xmlns=\"gml\" cs=\",\" ts=\" \">7,17 7,13 3,13 7,17</coordinates></LinearRing></outerBoundaryIs></Polygon></polygonMember><polygonMember xmlns=\"gml\"><Polygon xmlns=\"gml\"><outerBoundaryIs xmlns=\"gml\"><LinearRing xmlns=\"gml\"><coordinates xmlns=\"gml\" cs=\",\" ts=\" \">27,37 27,43 43,43 27,37</coordinates></LinearRing></outerBoundaryIs></Polygon></polygonMember></MultiPolygon>" ) );
  QString res = elemToString( exportC.asGml2( doc, 1 ) );
  QGSCOMPAREGML( res, expectedSimpleGML2 );
  QString expectedGML2empty( QStringLiteral( "<MultiPolygon xmlns=\"gml\"/>" ) );
  QGSCOMPAREGML( elemToString( QgsMultiSurface().asGml2( doc ) ), expectedGML2empty );

  //as GML3

  QString expectedSimpleGML3( QStringLiteral( "<MultiSurface xmlns=\"gml\"><surfaceMember xmlns=\"gml\"><Polygon xmlns=\"gml\"><exterior xmlns=\"gml\"><LinearRing xmlns=\"gml\"><posList xmlns=\"gml\" srsDimension=\"2\">7 17 7 13 3 13 7 17</posList></LinearRing></exterior></Polygon></surfaceMember><surfaceMember xmlns=\"gml\"><Polygon xmlns=\"gml\"><exterior xmlns=\"gml\"><LinearRing xmlns=\"gml\"><posList xmlns=\"gml\" srsDimension=\"2\">27 37 27 43 43 43 27 37</posList></LinearRing></exterior></Polygon></surfaceMember></MultiSurface>" ) );
  res = elemToString( exportC.asGml3( doc ) );
  QCOMPARE( res, expectedSimpleGML3 );
  QString expectedGML3empty( QStringLiteral( "<MultiSurface xmlns=\"gml\"/>" ) );
  QGSCOMPAREGML( elemToString( QgsMultiSurface().asGml3( doc ) ), expectedGML3empty );

  // as JSON
  QString expectedSimpleJson( "{\"coordinates\":[[[[7.0,17.0],[7.0,13.0],[3.0,13.0],[7.0,17.0]]],[[[27.0,37.0],[27.0,43.0],[43.0,43.0],[27.0,37.0]]]],\"type\":\"MultiPolygon\"}" );
  res = exportC.asJson( 1 );
  QCOMPARE( res, expectedSimpleJson );

  lineRing.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::Point, 17, 27 ) << QgsPoint( QgsWkbTypes::Point, 17, 28 ) << QgsPoint( QgsWkbTypes::Point, 18, 28 )  << QgsPoint( QgsWkbTypes::Point, 17, 27 ) ) ;
  part.addInteriorRing( lineRing.clone() );
  exportC.addGeometry( part.clone() );

  QString expectedJsonWithRings( "{\"coordinates\":[[[[7.0,17.0],[7.0,13.0],[3.0,13.0],[7.0,17.0]]],[[[27.0,37.0],[27.0,43.0],[43.0,43.0],[27.0,37.0]]],[[[27.0,37.0],[27.0,43.0],[43.0,43.0],[27.0,37.0]],[[17.0,27.0],[17.0,28.0],[18.0,28.0],[17.0,27.0]]]],\"type\":\"MultiPolygon\"}" );
  res = exportC.asJson( 1 );
  QCOMPARE( res, expectedJsonWithRings );

  QgsMultiSurface exportFloat;
  lineRing.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::Point, 0.1234, 0.1234 ) << QgsPoint( QgsWkbTypes::Point, 0.1234, 1.2344 ) << QgsPoint( QgsWkbTypes::Point, 1.2344, 1.2344 ) << QgsPoint( QgsWkbTypes::Point, 0.1234, 0.1234 ) ) ;
  part.clear();
  part.setExteriorRing( lineRing.clone() );
  exportFloat.addGeometry( part.clone() );

  QString expectedJsonPrec3( QStringLiteral( "{\"coordinates\":[[[[0.123,0.123],[0.123,1.234],[1.234,1.234],[0.123,0.123]]]],\"type\":\"MultiPolygon\"}" ) );
  res = exportFloat.asJson( 3 );
  QCOMPARE( res, expectedJsonPrec3 );

  // as GML2
  QString expectedGML2prec3( QStringLiteral( "<MultiPolygon xmlns=\"gml\"><polygonMember xmlns=\"gml\"><Polygon xmlns=\"gml\"><outerBoundaryIs xmlns=\"gml\"><LinearRing xmlns=\"gml\"><coordinates xmlns=\"gml\" cs=\",\" ts=\" \">0.123,0.123 0.123,1.234 1.234,1.234 0.123,0.123</coordinates></LinearRing></outerBoundaryIs></Polygon></polygonMember></MultiPolygon>" ) );
  res = elemToString( exportFloat.asGml2( doc, 3 ) );
  QGSCOMPAREGML( res, expectedGML2prec3 );

  //as GML3
  QString expectedGML3prec3( QStringLiteral( "<MultiSurface xmlns=\"gml\"><surfaceMember xmlns=\"gml\"><Polygon xmlns=\"gml\"><exterior xmlns=\"gml\"><LinearRing xmlns=\"gml\"><posList xmlns=\"gml\" srsDimension=\"2\">0.123 0.123 0.123 1.234 1.234 1.234 0.123 0.123</posList></LinearRing></exterior></Polygon></surfaceMember></MultiSurface>" ) );
  res = elemToString( exportFloat.asGml3( doc, 3 ) );
  QCOMPARE( res, expectedGML3prec3 );

  //asKML
  QString expectedKmlPrec3( QStringLiteral( "<MultiGeometry><Polygon><outerBoundaryIs><LinearRing><altitudeMode>clampToGround</altitudeMode><coordinates>0.123,0.123,0 0.123,1.234,0 1.234,1.234,0 0.123,0.123,0</coordinates></LinearRing></outerBoundaryIs></Polygon></MultiGeometry>" ) );
  QCOMPARE( exportFloat.asKml( 3 ), expectedKmlPrec3 );

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
  boundaryLine1.setPoints( QgsPointSequence() << QgsPoint( 0, 0 ) << QgsPoint( 1, 0 ) << QgsPoint( 0, 0 ) );
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
  boundaryLine2.setPoints( QgsPointSequence() << QgsPoint( 10, 10 ) << QgsPoint( 11, 10 ) << QgsPoint( 10, 10 ) );
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
  boundaryLine4.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZ, 0, 0, 10 ) << QgsPoint( QgsWkbTypes::PointZ, 1, 0, 15 ) << QgsPoint( QgsWkbTypes::PointZ, 0, 0, 10 ) );
  part.clear();
  part.setExteriorRing( boundaryLine4.clone() );
  QgsCircularString boundaryLine5;
  boundaryLine5.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZ, 10, 10, 100 ) << QgsPoint( QgsWkbTypes::PointZ, 10, 20, 150 ) << QgsPoint( QgsWkbTypes::PointZ, 10, 10, 100 ) );
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
  QCOMPARE( wkb16.size(), c16.wkbSize() );
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
  QString expectedSimpleJson( "{\"coordinates\":[[[[7.0,17.0],[3.0,13.0],[7.0,21.0],[7.0,17.0]]],[[[27.0,37.0],[43.0,43.0],[41.0,39.0],[27.0,37.0]]]],\"type\":\"MultiPolygon\"}" );
  res = exportC.asJson( 1 );
  QCOMPARE( res, expectedSimpleJson );

  ring.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::Point, 17, 27 ) << QgsPoint( QgsWkbTypes::Point, 18, 28 ) << QgsPoint( QgsWkbTypes::Point, 19, 37 ) << QgsPoint( QgsWkbTypes::Point, 17, 27 ) ) ;
  part.addInteriorRing( ring.clone() );
  exportC.addGeometry( part.clone() );

  QString expectedJsonWithRings( "{\"coordinates\":[[[[7.0,17.0],[3.0,13.0],[7.0,21.0],[7.0,17.0]]],[[[27.0,37.0],[43.0,43.0],[41.0,39.0],[27.0,37.0]]],[[[27.0,37.0],[43.0,43.0],[41.0,39.0],[27.0,37.0]],[[17.0,27.0],[18.0,28.0],[19.0,37.0],[17.0,27.0]]]],\"type\":\"MultiPolygon\"}" );
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

  QString expectedJsonPrec3( "{\"coordinates\":[[[[2.333,5.667],[0.6,4.333],[2.667,9.0],[2.333,5.667]]],[[[9.0,4.111],[1.049,1.024],[9.0,4.111]]]],\"type\":\"MultiPolygon\"}" );
  res = exportFloat.asJson( 3 );
  QCOMPARE( res, expectedJsonPrec3 );

  // as GML2
  QString expectedGML2prec3( "<MultiPolygon xmlns=\"gml\"><polygonMember xmlns=\"gml\"><Polygon xmlns=\"gml\"><outerBoundaryIs xmlns=\"gml\"><LinearRing xmlns=\"gml\"><coordinates xmlns=\"gml\" cs=\",\" ts=\" \">2.333,5.667 0.6,4.333 2.667,9 2.333,5.667</coordinates></LinearRing></outerBoundaryIs></Polygon></polygonMember><polygonMember xmlns=\"gml\"><Polygon xmlns=\"gml\"><outerBoundaryIs xmlns=\"gml\"><LinearRing xmlns=\"gml\"><coordinates xmlns=\"gml\" cs=\",\" ts=\" \">9,4.111 1.049,1.024 9,4.111</coordinates></LinearRing></outerBoundaryIs></Polygon></polygonMember></MultiPolygon>" );
  res = elemToString( exportFloat.asGml2( doc, 3 ) );
  QGSCOMPAREGML( res, expectedGML2prec3 );

  //as GML3
  QString expectedGML3prec3( "<MultiPolygon xmlns=\"gml\"><polygonMember xmlns=\"gml\"><Polygon xmlns=\"gml\"><exterior xmlns=\"gml\"><LinearRing xmlns=\"gml\"><posList xmlns=\"gml\" srsDimension=\"2\">2.333 5.667 0.6 4.333 2.667 9 2.333 5.667</posList></LinearRing></exterior></Polygon></polygonMember><polygonMember xmlns=\"gml\"><Polygon xmlns=\"gml\"><exterior xmlns=\"gml\"><LinearRing xmlns=\"gml\"><posList xmlns=\"gml\" srsDimension=\"2\">9 4.111 1.049 1.024 9 4.111</posList></LinearRing></exterior></Polygon></polygonMember></MultiPolygon>" );
  res = elemToString( exportFloat.asGml3( doc, 3 ) );
  QCOMPARE( res, expectedGML3prec3 );

  //asKML
  QString expectedKmlPrec3( "<MultiGeometry><Polygon><outerBoundaryIs><LinearRing><altitudeMode>clampToGround</altitudeMode><coordinates>2.333,5.667,0 0.6,4.333,0 2.667,9,0 2.333,5.667,0</coordinates></LinearRing></outerBoundaryIs></Polygon><Polygon><outerBoundaryIs><LineString><altitudeMode>clampToGround</altitudeMode><coordinates>9,4.111,0 1.049,1.024,0 9,4.111,0</coordinates></LineString></outerBoundaryIs></Polygon></MultiGeometry>" );
  QCOMPARE( exportFloat.asKml( 3 ), expectedKmlPrec3 );


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
  ring1.setPoints( QgsPointSequence() << QgsPoint( 0, 0 ) << QgsPoint( 1, 0 ) << QgsPoint( 1, 1 )  << QgsPoint( 0, 0 ) );
  QgsPolygon polygon1;
  polygon1.setExteriorRing( ring1.clone() );
  multiPolygon1.addGeometry( polygon1.clone() );

  std::unique_ptr< QgsAbstractGeometry > boundary( multiPolygon1.boundary() );
  QgsMultiLineString *lineBoundary = dynamic_cast< QgsMultiLineString * >( boundary.get() );
  QVERIFY( lineBoundary );
  QCOMPARE( lineBoundary->numGeometries(), 1 );
  QCOMPARE( qgis::down_cast< QgsLineString * >( lineBoundary->geometryN( 0 ) )->numPoints(), 4 );
  QCOMPARE( qgis::down_cast< QgsLineString * >( lineBoundary->geometryN( 0 ) )->xAt( 0 ), 0.0 );
  QCOMPARE( qgis::down_cast< QgsLineString * >( lineBoundary->geometryN( 0 ) )->xAt( 1 ), 1.0 );
  QCOMPARE( qgis::down_cast< QgsLineString * >( lineBoundary->geometryN( 0 ) )->xAt( 2 ), 1.0 );
  QCOMPARE( qgis::down_cast< QgsLineString * >( lineBoundary->geometryN( 0 ) )->xAt( 3 ), 0.0 );
  QCOMPARE( qgis::down_cast< QgsLineString * >( lineBoundary->geometryN( 0 ) )->yAt( 0 ), 0.0 );
  QCOMPARE( qgis::down_cast< QgsLineString * >( lineBoundary->geometryN( 0 ) )->yAt( 1 ), 0.0 );
  QCOMPARE( qgis::down_cast< QgsLineString * >( lineBoundary->geometryN( 0 ) )->yAt( 2 ), 1.0 );
  QCOMPARE( qgis::down_cast< QgsLineString * >( lineBoundary->geometryN( 0 ) )->yAt( 3 ), 0.0 );

  // add polygon with interior rings
  QgsLineString ring2;
  ring2.setPoints( QgsPointSequence() << QgsPoint( 10, 10 ) << QgsPoint( 11, 10 ) << QgsPoint( 11, 11 )  << QgsPoint( 10, 10 ) );
  QgsPolygon polygon2;
  polygon2.setExteriorRing( ring2.clone() );
  QgsLineString boundaryRing1;
  boundaryRing1.setPoints( QgsPointSequence() << QgsPoint( 10.1, 10.1 ) << QgsPoint( 10.2, 10.1 ) << QgsPoint( 10.2, 10.2 )  << QgsPoint( 10.1, 10.1 ) );
  QgsLineString boundaryRing2;
  boundaryRing2.setPoints( QgsPointSequence() << QgsPoint( 10.8, 10.8 ) << QgsPoint( 10.9, 10.8 ) << QgsPoint( 10.9, 10.9 )  << QgsPoint( 10.8, 10.8 ) );
  polygon2.setInteriorRings( QVector< QgsCurve * >() << boundaryRing1.clone() << boundaryRing2.clone() );
  multiPolygon1.addGeometry( polygon2.clone() );

  boundary.reset( multiPolygon1.boundary() );
  QgsMultiLineString *multiLineBoundary( static_cast< QgsMultiLineString * >( boundary.get() ) );
  QVERIFY( multiLineBoundary );
  QCOMPARE( multiLineBoundary->numGeometries(), 4 );
  QCOMPARE( qgis::down_cast< QgsLineString * >( multiLineBoundary->geometryN( 0 ) )->numPoints(), 4 );
  QCOMPARE( qgis::down_cast< QgsLineString * >( multiLineBoundary->geometryN( 0 ) )->xAt( 0 ), 0.0 );
  QCOMPARE( qgis::down_cast< QgsLineString * >( multiLineBoundary->geometryN( 0 ) )->xAt( 1 ), 1.0 );
  QCOMPARE( qgis::down_cast< QgsLineString * >( multiLineBoundary->geometryN( 0 ) )->xAt( 2 ), 1.0 );
  QCOMPARE( qgis::down_cast< QgsLineString * >( multiLineBoundary->geometryN( 0 ) )->xAt( 3 ), 0.0 );
  QCOMPARE( qgis::down_cast< QgsLineString * >( multiLineBoundary->geometryN( 0 ) )->yAt( 0 ), 0.0 );
  QCOMPARE( qgis::down_cast< QgsLineString * >( multiLineBoundary->geometryN( 0 ) )->yAt( 1 ), 0.0 );
  QCOMPARE( qgis::down_cast< QgsLineString * >( multiLineBoundary->geometryN( 0 ) )->yAt( 2 ), 1.0 );
  QCOMPARE( qgis::down_cast< QgsLineString * >( multiLineBoundary->geometryN( 0 ) )->yAt( 3 ), 0.0 );
  QCOMPARE( qgis::down_cast< QgsLineString * >( multiLineBoundary->geometryN( 1 ) )->numPoints(), 4 );
  QCOMPARE( qgis::down_cast< QgsLineString * >( multiLineBoundary->geometryN( 1 ) )->xAt( 0 ), 10.0 );
  QCOMPARE( qgis::down_cast< QgsLineString * >( multiLineBoundary->geometryN( 1 ) )->xAt( 1 ), 11.0 );
  QCOMPARE( qgis::down_cast< QgsLineString * >( multiLineBoundary->geometryN( 1 ) )->xAt( 2 ), 11.0 );
  QCOMPARE( qgis::down_cast< QgsLineString * >( multiLineBoundary->geometryN( 1 ) )->xAt( 3 ), 10.0 );
  QCOMPARE( qgis::down_cast< QgsLineString * >( multiLineBoundary->geometryN( 1 ) )->yAt( 0 ), 10.0 );
  QCOMPARE( qgis::down_cast< QgsLineString * >( multiLineBoundary->geometryN( 1 ) )->yAt( 1 ), 10.0 );
  QCOMPARE( qgis::down_cast< QgsLineString * >( multiLineBoundary->geometryN( 1 ) )->yAt( 2 ), 11.0 );
  QCOMPARE( qgis::down_cast< QgsLineString * >( multiLineBoundary->geometryN( 1 ) )->yAt( 3 ), 10.0 );
  QCOMPARE( qgis::down_cast< QgsLineString * >( multiLineBoundary->geometryN( 2 ) )->numPoints(), 4 );
  QCOMPARE( qgis::down_cast< QgsLineString * >( multiLineBoundary->geometryN( 2 ) )->xAt( 0 ), 10.1 );
  QCOMPARE( qgis::down_cast< QgsLineString * >( multiLineBoundary->geometryN( 2 ) )->xAt( 1 ), 10.2 );
  QCOMPARE( qgis::down_cast< QgsLineString * >( multiLineBoundary->geometryN( 2 ) )->xAt( 2 ), 10.2 );
  QCOMPARE( qgis::down_cast< QgsLineString * >( multiLineBoundary->geometryN( 2 ) )->xAt( 3 ), 10.1 );
  QCOMPARE( qgis::down_cast< QgsLineString * >( multiLineBoundary->geometryN( 2 ) )->yAt( 0 ), 10.1 );
  QCOMPARE( qgis::down_cast< QgsLineString * >( multiLineBoundary->geometryN( 2 ) )->yAt( 1 ), 10.1 );
  QCOMPARE( qgis::down_cast< QgsLineString * >( multiLineBoundary->geometryN( 2 ) )->yAt( 2 ), 10.2 );
  QCOMPARE( qgis::down_cast< QgsLineString * >( multiLineBoundary->geometryN( 2 ) )->yAt( 3 ), 10.1 );
  QCOMPARE( qgis::down_cast< QgsLineString * >( multiLineBoundary->geometryN( 3 ) )->numPoints(), 4 );
  QCOMPARE( qgis::down_cast< QgsLineString * >( multiLineBoundary->geometryN( 3 ) )->xAt( 0 ), 10.8 );
  QCOMPARE( qgis::down_cast< QgsLineString * >( multiLineBoundary->geometryN( 3 ) )->xAt( 1 ), 10.9 );
  QCOMPARE( qgis::down_cast< QgsLineString * >( multiLineBoundary->geometryN( 3 ) )->xAt( 2 ), 10.9 );
  QCOMPARE( qgis::down_cast< QgsLineString * >( multiLineBoundary->geometryN( 3 ) )->xAt( 3 ), 10.8 );
  QCOMPARE( qgis::down_cast< QgsLineString * >( multiLineBoundary->geometryN( 3 ) )->yAt( 0 ), 10.8 );
  QCOMPARE( qgis::down_cast< QgsLineString * >( multiLineBoundary->geometryN( 3 ) )->yAt( 1 ), 10.8 );
  QCOMPARE( qgis::down_cast< QgsLineString * >( multiLineBoundary->geometryN( 3 ) )->yAt( 2 ), 10.9 );
  QCOMPARE( qgis::down_cast< QgsLineString * >( multiLineBoundary->geometryN( 3 ) )->yAt( 3 ), 10.8 );

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

  //removeDuplicateNodes
  QgsMultiPolygon dn1;
  QVERIFY( dn1.fromWkt( QStringLiteral( "MultiPolygonZ (((0 0 0, 10 10 0, 11 9 0, 9 8 0, 9 8 0, 1 -1 0, 0 0 0)),((7 -1 0, 12 7 0, 13 6 0, 13 6 0, 8 -3 0, 7 -1 0)))" ) ) );
  QCOMPARE( dn1.numGeometries(), 2 );
  // First call should remove all duplicate nodes (one per part)
  QVERIFY( dn1.removeDuplicateNodes( 0.001, false ) );
  QVERIFY( !dn1.removeDuplicateNodes( 0.001, false ) );
  QCOMPARE( dn1.asWkt(), QStringLiteral( "MultiPolygonZ (((0 0 0, 10 10 0, 11 9 0, 9 8 0, 1 -1 0, 0 0 0)),((7 -1 0, 12 7 0, 13 6 0, 8 -3 0, 7 -1 0)))" ) );

  // test centroid of empty multipolygon
  QgsMultiPolygon empty;
  QCOMPARE( empty.centroid().asWkt(), QStringLiteral( "Point EMPTY" ) );
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
  part.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZ, 0, 0, 1 )
                  << QgsPoint( QgsWkbTypes::PointZ, 0, 10, 2 ) << QgsPoint( QgsWkbTypes::PointZ, 10, 10, 3 )
                  << QgsPoint( QgsWkbTypes::PointZ, 10, 0, 4 ) << QgsPoint( QgsWkbTypes::PointZ, 0, 0, 1 ) );
  ml.addGeometry( part.clone() );
  QgsMultiLineString ml2;
  QVERIFY( ml != ml2 );
  part.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::PointZ, 1, 1, 1 )
                  << QgsPoint( QgsWkbTypes::PointZ, 0, 10, 2 ) << QgsPoint( QgsWkbTypes::PointZ, 10, 10, 3 )
                  << QgsPoint( QgsWkbTypes::PointZ, 10, 0, 4 ) << QgsPoint( QgsWkbTypes::PointZ, 0, 0, 1 ) );
  ml2.addGeometry( part.clone() );
  QVERIFY( ml != ml2 );

  QgsMultiLineString ml3;
  ml3.addGeometry( part.clone() );
  QVERIFY( ml2 == ml3 );

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
  QCOMPARE( wkb16.size(), c16.wkbSize() );
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
  QString expectedSimpleJson( "{\"geometries\":[{\"coordinates\":[[0.0,0.0],[0.0,10.0],[10.0,10.0],[10.0,0.0],[0.0,0.0]],\"type\":\"LineString\"}],\"type\":\"GeometryCollection\"}" );
  res = exportC.asJson();
  QCOMPARE( res, expectedSimpleJson );

  part.setPoints( QgsPointSequence() << QgsPoint( QgsWkbTypes::Point, 1, 1 )
                  << QgsPoint( QgsWkbTypes::Point, 1, 9 ) << QgsPoint( QgsWkbTypes::Point, 9, 9 )
                  << QgsPoint( QgsWkbTypes::Point, 9, 1 ) << QgsPoint( QgsWkbTypes::Point, 1, 1 ) );
  exportC.addGeometry( part.clone() );

  QString expectedJson( "{\"geometries\":[{\"coordinates\":[[0.0,0.0],[0.0,10.0],[10.0,10.0],[10.0,0.0],[0.0,0.0]],\"type\":\"LineString\"},{\"coordinates\":[[1.0,1.0],[1.0,9.0],[9.0,9.0],[9.0,1.0],[1.0,1.0]],\"type\":\"LineString\"}],\"type\":\"GeometryCollection\"}" );
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

  QString expectedJsonPrec3( "{\"geometries\":[{\"coordinates\":[[1.111,1.111],[1.111,11.111],[11.111,11.111],[11.111,1.111],[1.111,1.111]],\"type\":\"LineString\"},{\"coordinates\":[[0.667,0.667],[0.667,1.333],[1.333,1.333],[1.333,0.667],[0.667,0.667]],\"type\":\"LineString\"}],\"type\":\"GeometryCollection\"}" );
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

  //asKML
  QString expectedKml( QStringLiteral( "<MultiGeometry><LinearRing><altitudeMode>clampToGround</altitudeMode><coordinates>0,0,0 0,10,0 10,10,0 10,0,0 0,0,0</coordinates></LinearRing><LinearRing><altitudeMode>clampToGround</altitudeMode><coordinates>1,1,0 1,9,0 9,9,0 9,1,0 1,1,0</coordinates></LinearRing></MultiGeometry>" ) );
  QCOMPARE( exportC.asKml(), expectedKml );
  QString expectedKmlPrec3( QStringLiteral( "<MultiGeometry><LinearRing><altitudeMode>clampToGround</altitudeMode><coordinates>1.111,1.111,0 1.111,11.111,0 11.111,11.111,0 11.111,1.111,0 1.111,1.111,0</coordinates></LinearRing><LinearRing><altitudeMode>clampToGround</altitudeMode><coordinates>0.667,0.667,0 0.667,1.333,0 1.333,1.333,0 1.333,0.667,0 0.667,0.667,0</coordinates></LinearRing></MultiGeometry>" ) );
  QCOMPARE( exportFloat.asKml( 3 ), expectedKmlPrec3 );


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
  QgsCoordinateReferenceSystem sourceSrs( QStringLiteral( "EPSG:3994" ) );
  QgsCoordinateReferenceSystem destSrs( QStringLiteral( "EPSG:4202" ) ); // want a transform with ellipsoid change
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

#if PROJ_VERSION_MAJOR<6 // note - z value transform doesn't currently work with proj 6+, because we don't yet support compound CRS definitions
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
#endif

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
  lineBoundary->setPoints( QgsPointSequence() << QgsPoint( 0, 0 ) << QgsPoint( 1, 0 ) );
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
  QVERIFY( !gcNodes.removeDuplicateNodes( 0.02 ) );
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

  // transform using class
  TestTransformer transformer;
  QgsGeometryCollection transformCollect2;
  transformCollect2.transform( &transformer ); // no crash
  transformLine.setPoints( QgsPointSequence() << QgsPoint( 1, 2, 3, 4, QgsWkbTypes::PointZM ) << QgsPoint( 11, 12, 13, 14, QgsWkbTypes::PointZM ) << QgsPoint( 111, 12, 23, 24, QgsWkbTypes::PointZM ) );
  transformCollect2.addGeometry( transformLine.clone() );
  QVERIFY( transformCollect2.transform( &transformer ) );
  QCOMPARE( transformCollect2.asWkt(), QStringLiteral( "GeometryCollection (LineStringZM (3 16 8 3, 33 26 18 13, 333 26 28 23))" ) );
  transformLine.setPoints( QgsPointSequence() << QgsPoint( 11, 2, 5, 6, QgsWkbTypes::PointZM ) << QgsPoint( 1.01, 1.99, 15, 16, QgsWkbTypes::PointZM ) << QgsPoint( 11.02, 2.01, 25, 26, QgsWkbTypes::PointZM ) );
  transformCollect2.addGeometry( transformLine.clone() );
  QVERIFY( transformCollect2.transform( &transformer ) );
  QCOMPARE( transformCollect2.asWkt( 2 ), QStringLiteral( "GeometryCollection (LineStringZM (9 30 13 2, 99 40 23 12, 999 40 33 22),LineStringZM (33 16 10 5, 3.03 15.99 20 15, 33.06 16.01 30 25))" ) );

  TestFailTransformer failTransformer;
  QVERIFY( !transformCollect2.transform( &failTransformer ) );
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
    for ( QString ext : extensionZM )
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

          QgsGeometry gWkt = QgsGeometry().fromWkt( generatedWkt );
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
    QgsPolylineXY myPolyline = polygon.at( j ); //rings of polygon
    qDebug( "\t\tRing in polygon: %d", j );

    for ( int k = 0; k < myPolyline.size(); k++ )
    {
      QgsPointXY myPoint = myPolyline.at( k );
      qDebug( "\t\t\tPoint in ring %d : %s", k, myPoint.toString().toLocal8Bit().constData() );
    }
  }
}

void TestQgsGeometry::paintPolygon( QgsPolygonXY &polygon )
{
  QVector<QPointF> myPoints;
  for ( int j = 0; j < polygon.size(); j++ )
  {
    QgsPolylineXY myPolyline = polygon.at( j ); //rings of polygon
    for ( int k = 0; k < myPolyline.size(); k++ )
    {
      QgsPointXY myPoint = myPolyline.at( k );
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
