/***************************************************************************
     testqgstessellator.cpp
     ----------------------
    Date                 : October 2017
    Copyright            : (C) 2017 by Martin Dobias
    Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgstest.h"

#include <QVector3D>

#include "qgspoint.h"
#include "qgspolygon.h"
#include "qgstessellator.h"
#include "qgsmultipolygon.h"
#include "qgslogger.h"
#include "qgsgeometry.h"

static bool qgsVectorNear( const QVector3D &v1, const QVector3D &v2, double eps )
{
  return qgsDoubleNear( v1.x(), v2.x(), eps ) && qgsDoubleNear( v1.y(), v2.y(), eps ) && qgsDoubleNear( v1.z(), v2.z(), eps );
}

/**
 * Simple structure to record an expected triangle from tessellator.
 * Triangle vertices are expected to be in counter-clockwise order.
 */
struct TriangleCoords
{
  //! Constructs from expected triangle coordinates
  TriangleCoords( const QVector3D &a, const QVector3D &b, const QVector3D &c,
                  const QVector3D &na = QVector3D(), const QVector3D &nb = QVector3D(), const QVector3D &nc = QVector3D() )
  {
    pts[0] = a; pts[1] = b; pts[2] = c;
    normals[0] = na; normals[1] = nb; normals[2] = nc;
  }

  //! Constructs from tessellator output. Note: tessellator outputs (X,-Z,Y) tuples for (X,Y,Z) input coords
  TriangleCoords( const float *data, bool withNormal )
  {
#define FLOAT3_TO_VECTOR(x)  QVector3D( data[0], -data[2], data[1] )

    pts[0] = FLOAT3_TO_VECTOR( data ); data += 3;
    if ( withNormal ) { normals[0] = FLOAT3_TO_VECTOR( data ); data += 3; }

    pts[1] = FLOAT3_TO_VECTOR( data ); data += 3;
    if ( withNormal ) { normals[1] = FLOAT3_TO_VECTOR( data ); data += 3; }

    pts[2] = FLOAT3_TO_VECTOR( data ); data += 3;
    if ( withNormal ) { normals[2] = FLOAT3_TO_VECTOR( data ); data += 3; }
  }

  //! Compares two triangles
  bool operator==( const TriangleCoords &other ) const
  {
    // TODO: allow that the two triangles have coordinates shifted (but still in the same order)
    const double eps = 1e-6;
    return qgsVectorNear( pts[0], other.pts[0], eps ) &&
           qgsVectorNear( pts[1], other.pts[1], eps ) &&
           qgsVectorNear( pts[2], other.pts[2], eps ) &&
           qgsVectorNear( normals[0], other.normals[0], eps ) &&
           qgsVectorNear( normals[1], other.normals[1], eps ) &&
           qgsVectorNear( normals[2], other.normals[2], eps );
  }

  bool operator!=( const TriangleCoords &other ) const
  {
    return !operator==( other );
  }

  void dump() const
  {
    qDebug() << pts[0] << pts[1] << pts[2] << normals[0] << normals[1] << normals[2];
  }

  QVector3D pts[3];
  QVector3D normals[3];
};


bool checkTriangleOutput( const QVector<float> &data, bool withNormals, const QList<TriangleCoords> &expected )
{
  const int valuesPerTriangle = withNormals ? 18 : 9;
  if ( data.count() != expected.count() * valuesPerTriangle )
  {
    qDebug() << "expected" << expected.count() << "triangles, got" << data.count() / valuesPerTriangle;
    return false;
  }

  // TODO: allow arbitrary order of triangles in output
  const float *dataRaw = data.constData();
  for ( int i = 0; i < expected.count(); ++i )
  {
    const TriangleCoords &exp = expected.at( i );
    const TriangleCoords out( dataRaw, withNormals );
    if ( exp != out )
    {
      qDebug() << i;
      qDebug() << "expected:";
      exp.dump();
      qDebug() << "got:";
      out.dump();
      return false;
    }
    dataRaw += withNormals ? 18 : 9;
  }
  return true;
}


/**
 * \ingroup UnitTests
 * This is a unit test for the vertex tool
 */
class TestQgsTessellator : public QObject
{
    Q_OBJECT
  public:
    TestQgsTessellator() = default;

  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.

    void testBasic();
    void testWalls();
    void testBackEdges();
    void asMultiPolygon();
    void testBadCoordinates();
    void testIssue17745();
    void testCrashSelfIntersection();
    void testCrashEmptyPolygon();
    void testBoundsScaling();
    void testNoZ();
    void testTriangulationDoesNotCrash();
    void testCrash2DTriangle();
    void narrowPolygon();

  private:
};

//runs before all tests
void TestQgsTessellator::initTestCase()
{
}

//runs after all tests
void TestQgsTessellator::cleanupTestCase()
{
}

void TestQgsTessellator::testBasic()
{
  QgsPolygon polygon;
  polygon.fromWkt( "POLYGON((1 1, 2 1, 3 2, 1 2, 1 1))" );

  QgsPolygon polygonZ;
  polygonZ.fromWkt( "POLYGONZ((1 1 3, 2 1 3, 3 2 3, 1 2 3, 1 1 3))" );

  QList<TriangleCoords> tc;
  tc << TriangleCoords( QVector3D( 1, 2, 0 ), QVector3D( 2, 1, 0 ), QVector3D( 3, 2, 0 ) );
  tc << TriangleCoords( QVector3D( 1, 2, 0 ), QVector3D( 1, 1, 0 ), QVector3D( 2, 1, 0 ) );

  QList<TriangleCoords> tcZ;
  tcZ << TriangleCoords( QVector3D( 1, 2, 3 ), QVector3D( 2, 1, 3 ), QVector3D( 3, 2, 3 ) );
  tcZ << TriangleCoords( QVector3D( 1, 2, 3 ), QVector3D( 1, 1, 3 ), QVector3D( 2, 1, 3 ) );

  const QVector3D up( 0, 0, 1 );  // surface normal pointing straight up
  QList<TriangleCoords> tcNormals;
  tcNormals << TriangleCoords( QVector3D( 1, 2, 0 ), QVector3D( 2, 1, 0 ), QVector3D( 3, 2, 0 ), up, up, up );
  tcNormals << TriangleCoords( QVector3D( 1, 2, 0 ), QVector3D( 1, 1, 0 ), QVector3D( 2, 1, 0 ), up, up, up );

  QList<TriangleCoords> tcNormalsZ;
  tcNormalsZ << TriangleCoords( QVector3D( 1, 2, 3 ), QVector3D( 2, 1, 3 ), QVector3D( 3, 2, 3 ), up, up, up );
  tcNormalsZ << TriangleCoords( QVector3D( 1, 2, 3 ), QVector3D( 1, 1, 3 ), QVector3D( 2, 1, 3 ), up, up, up );

  // without normals

  QgsTessellator t( 0, 0, false );
  t.addPolygon( polygon, 0 );
  QVERIFY( checkTriangleOutput( t.data(), false, tc ) );

  QCOMPARE( t.zMinimum(), 0 );
  QCOMPARE( t.zMaximum(), 0 );

  QgsTessellator tZ( 0, 0, false );
  tZ.addPolygon( polygonZ, 0 );
  QVERIFY( checkTriangleOutput( tZ.data(), false, tcZ ) );

  QCOMPARE( tZ.zMinimum(), 3 );
  QCOMPARE( tZ.zMaximum(), 3 );

  // with normals

  QgsTessellator tN( 0, 0, true );
  tN.addPolygon( polygon, 0 );
  QVERIFY( checkTriangleOutput( tN.data(), true, tcNormals ) );

  QCOMPARE( tN.zMinimum(), 0 );
  QCOMPARE( tN.zMaximum(), 0 );

  QgsTessellator tNZ( 0, 0, true );
  tNZ.addPolygon( polygonZ, 0 );
  QVERIFY( checkTriangleOutput( tNZ.data(), true, tcNormalsZ ) );

  QCOMPARE( tNZ.zMinimum(), 3 );
  QCOMPARE( tNZ.zMaximum(), 3 );
}

void TestQgsTessellator::testWalls()
{
  QgsPolygon rect;
  rect.fromWkt( "POLYGON((0 0, 3 0, 3 2, 0 2, 0 0))" );

  const QVector3D zPos( 0, 0, 1 );
  const QVector3D xPos( 1, 0, 0 );
  const QVector3D yPos( 0, 1, 0 );
  const QVector3D xNeg( -1, 0, 0 );
  const QVector3D yNeg( 0, -1, 0 );

  QList<TriangleCoords> tcRect;
  tcRect << TriangleCoords( QVector3D( 0, 2, 1 ), QVector3D( 3, 0, 1 ), QVector3D( 3, 2, 1 ), zPos, zPos, zPos );
  tcRect << TriangleCoords( QVector3D( 0, 2, 1 ), QVector3D( 0, 0, 1 ), QVector3D( 3, 0, 1 ), zPos, zPos, zPos );
  tcRect << TriangleCoords( QVector3D( 0, 0, 1 ), QVector3D( 0, 2, 1 ), QVector3D( 0, 0, 0 ), xNeg, xNeg, xNeg );
  tcRect << TriangleCoords( QVector3D( 0, 0, 0 ), QVector3D( 0, 2, 1 ), QVector3D( 0, 2, 0 ), xNeg, xNeg, xNeg );
  tcRect << TriangleCoords( QVector3D( 0, 2, 1 ), QVector3D( 3, 2, 1 ), QVector3D( 0, 2, 0 ), yPos, yPos, yPos );
  tcRect << TriangleCoords( QVector3D( 0, 2, 0 ), QVector3D( 3, 2, 1 ), QVector3D( 3, 2, 0 ), yPos, yPos, yPos );
  tcRect << TriangleCoords( QVector3D( 3, 2, 1 ), QVector3D( 3, 0, 1 ), QVector3D( 3, 2, 0 ), xPos, xPos, xPos );
  tcRect << TriangleCoords( QVector3D( 3, 2, 0 ), QVector3D( 3, 0, 1 ), QVector3D( 3, 0, 0 ), xPos, xPos, xPos );
  tcRect << TriangleCoords( QVector3D( 3, 0, 1 ), QVector3D( 0, 0, 1 ), QVector3D( 3, 0, 0 ), yNeg, yNeg, yNeg );
  tcRect << TriangleCoords( QVector3D( 3, 0, 0 ), QVector3D( 0, 0, 1 ), QVector3D( 0, 0, 0 ), yNeg, yNeg, yNeg );

  QgsTessellator tRect( 0, 0, true );
  tRect.addPolygon( rect, 1 );
  QVERIFY( checkTriangleOutput( tRect.data(), true, tcRect ) );
  QCOMPARE( tRect.zMinimum(), 0 );
  QCOMPARE( tRect.zMaximum(), 1 );

  // try to extrude a polygon with reverse (clock-wise) order of vertices and check it is still fine

  QgsPolygon rectRev;
  rectRev.fromWkt( "POLYGON((0 0, 0 2, 3 2, 3 0, 0 0))" );

  QgsTessellator tRectRev( 0, 0, true );
  tRectRev.addPolygon( rectRev, 1 );
  QVERIFY( checkTriangleOutput( tRectRev.data(), true, tcRect ) );
  QCOMPARE( tRectRev.zMinimum(), 0 );
  QCOMPARE( tRectRev.zMaximum(), 1 );

  // this is a more complicated polygon with Z coordinates where the "roof" is not in one plane

  QgsPolygon polygonZ;
  polygonZ.fromWkt( "POLYGONZ((1 1 1, 2 1 2, 3 2 3, 1 2 4, 1 1 1))" );

  QList<TriangleCoords> tc;

  tc << TriangleCoords( QVector3D( 1, 2, 14 ), QVector3D( 2, 1, 12 ), QVector3D( 3, 2, 13 ) );
  tc << TriangleCoords( QVector3D( 1, 2, 14 ), QVector3D( 1, 1, 11 ), QVector3D( 2, 1, 12 ) );

  tc << TriangleCoords( QVector3D( 1, 1, 11 ), QVector3D( 1, 2, 14 ), QVector3D( 1, 1, 1 ) );
  tc << TriangleCoords( QVector3D( 1, 1, 1 ), QVector3D( 1, 2, 14 ), QVector3D( 1, 2, 4 ) );
  tc << TriangleCoords( QVector3D( 1, 2, 14 ), QVector3D( 3, 2, 13 ), QVector3D( 1, 2, 4 ) );
  tc << TriangleCoords( QVector3D( 1, 2, 4 ), QVector3D( 3, 2, 13 ), QVector3D( 3, 2, 3 ) );
  tc << TriangleCoords( QVector3D( 3, 2, 13 ), QVector3D( 2, 1, 12 ), QVector3D( 3, 2, 3 ) );
  tc << TriangleCoords( QVector3D( 3, 2, 3 ), QVector3D( 2, 1, 12 ), QVector3D( 2, 1, 2 ) );
  tc << TriangleCoords( QVector3D( 2, 1, 12 ), QVector3D( 1, 1, 11 ), QVector3D( 2, 1, 2 ) );
  tc << TriangleCoords( QVector3D( 2, 1, 2 ), QVector3D( 1, 1, 11 ), QVector3D( 1, 1, 1 ) );

  QgsTessellator tZ( 0, 0, false );
  tZ.addPolygon( polygonZ, 10 );
  QVERIFY( checkTriangleOutput( tZ.data(), false, tc ) );

  QCOMPARE( tZ.zMinimum(), 1 );
  QCOMPARE( tZ.zMaximum(), 14 );
}

void TestQgsTessellator::testBackEdges()
{
  QgsPolygon polygon;
  polygon.fromWkt( "POLYGON((1 1, 2 1, 3 2, 1 2, 1 1))" );

  const QVector3D up( 0, 0, 1 );  // surface normal pointing straight up
  const QVector3D dn( 0, 0, -1 );  // surface normal pointing straight down for back faces
  QList<TriangleCoords> tcNormals;
  tcNormals << TriangleCoords( QVector3D( 1, 2, 0 ), QVector3D( 2, 1, 0 ), QVector3D( 3, 2, 0 ), up, up, up );
  tcNormals << TriangleCoords( QVector3D( 3, 2, 0 ), QVector3D( 2, 1, 0 ), QVector3D( 1, 2, 0 ), dn, dn, dn );
  tcNormals << TriangleCoords( QVector3D( 1, 2, 0 ), QVector3D( 1, 1, 0 ), QVector3D( 2, 1, 0 ), up, up, up );
  tcNormals << TriangleCoords( QVector3D( 2, 1, 0 ), QVector3D( 1, 1, 0 ), QVector3D( 1, 2, 0 ), dn, dn, dn );

  QgsTessellator tN( 0, 0, true, false, true );
  tN.addPolygon( polygon, 0 );
  QVERIFY( checkTriangleOutput( tN.data(), true, tcNormals ) );

  QCOMPARE( tN.zMinimum(), 0 );
  QCOMPARE( tN.zMaximum(), 0 );
}

void TestQgsTessellator::asMultiPolygon()
{
  QgsPolygon polygon;
  polygon.fromWkt( "POLYGON((1 1, 2 1, 3 2, 1 2, 1 1))" );

  QgsPolygon polygonZ;
  polygonZ.fromWkt( "POLYGONZ((1 1 1, 2 1 2, 3 2 3, 1 2 4, 1 1 1))" );

  QgsTessellator t( 0, 0, false );
  t.addPolygon( polygon, 0 );
  QCOMPARE( t.asMultiPolygon()->asWkt(), QStringLiteral( "MultiPolygonZ (((1 2 0, 2 1 0, 3 2 0, 1 2 0)),((1 2 0, 1 1 0, 2 1 0, 1 2 0)))" ) );

  QgsTessellator t2( 0, 0, false );
  t2.addPolygon( polygonZ, 0 );
  QCOMPARE( t2.asMultiPolygon()->asWkt(), QStringLiteral( "MultiPolygonZ (((1 2 4, 2 1 2, 3 2 3, 1 2 4)),((1 2 4, 1 1 1, 2 1 2, 1 2 4)))" ) );
}

void TestQgsTessellator::testBadCoordinates()
{
  // check with a vertical "wall" polygon - if the Z coordinates are ignored,
  // the polygon may be incorrectly considered as having close/repeated coordinates
  QList<TriangleCoords> tcZ;
  tcZ << TriangleCoords( QVector3D( 1, 2, 2 ), QVector3D( 2, 1, 1 ), QVector3D( 2, 1, 2 ) );
  tcZ << TriangleCoords( QVector3D( 1, 2, 2 ), QVector3D( 1, 2, 1 ), QVector3D( 2, 1, 1 ) );

  QgsPolygon polygonZ;
  polygonZ.fromWkt( "POLYGONZ((1 2 1, 2 1 1, 2 1 2, 1 2 2, 1 2 1))" );

  QgsTessellator tZ( 0, 0, false );
  tZ.addPolygon( polygonZ, 0 );
  QVERIFY( checkTriangleOutput( tZ.data(), false, tcZ ) );

  QCOMPARE( tZ.zMinimum(), 1 );
  QCOMPARE( tZ.zMaximum(), 2 );

  // triangulation would crash for me with this polygon if there is no simplification
  // to remove the coordinates that are very close to each other
  QgsPolygon polygon;
  polygon.fromWkt( "POLYGON((1 1, 2 1, 2.0000001 1.0000001, 2.0000002 1.0000001, 2.0000001 1.0000002, 2.0000002 1.0000002, 3 2, 1 2, 1 1))" );

  QList<TriangleCoords> tc;
  tc << TriangleCoords( QVector3D( 1, 2, 0 ), QVector3D( 2, 1, 0 ), QVector3D( 3, 2, 0 ) );
  tc << TriangleCoords( QVector3D( 1, 2, 0 ), QVector3D( 1, 1, 0 ), QVector3D( 2, 1, 0 ) );

  QgsTessellator t( 0, 0, false );
  t.addPolygon( polygon, 0 );
  QVERIFY( checkTriangleOutput( t.data(), false, tc ) );

  QCOMPARE( t.zMinimum(), 0 );
  QCOMPARE( t.zMaximum(), 0 );
}

void TestQgsTessellator::testIssue17745()
{
  // this is a rectangular polygon with collinear points that would crash poly2tri if coordinates do not get rounded a bit

  QgsTessellator t( 0, 0, true );
  QgsPolygon p;
  const bool resWktRead = p.fromWkt( "Polygon((0 0, 1 1e-15, 4 0, 4 5, 1 5, 0 5, 0 0))" );
  QVERIFY( resWktRead );

  t.addPolygon( p, 0 );   // must not crash - that's all we test here
}

void TestQgsTessellator::testCrashSelfIntersection()
{
  // this is a polygon where we get self-intersecting exterior ring that would crash poly2tri if not skipped

  QgsTessellator t( 0, 0, true );
  QgsPolygon p;
  const bool resWktRead = p.fromWkt( "PolygonZ ((-744809.80499999970197678 -1042371.96730000153183937 260.460968017578125, -744809.80299999937415123 -1042371.92199999839067459 260.460968017578125, -744810.21599999815225601 -1042381.09099999815225601 260.460968017578125, -744810.21499999985098839 -1042381.0689999982714653 260.460968017578125, -744812.96469999849796295 -1042375.32499999925494194 263.734283447265625, -744809.80499999970197678 -1042371.96730000153183937 260.460968017578125))" );

  QVERIFY( resWktRead );

  t.addPolygon( p, 0 );   // must not crash - that's all we test here
}

void TestQgsTessellator::testCrashEmptyPolygon()
{
  // this is a polygon that goes through GEOS simplification which throws an exception (and produces null geometry)

  QgsTessellator t( 0, 0, true );
  QgsPolygon p;
  const bool resWktRead = p.fromWkt( "PolygonZ ((0 0 0, 0 0 0, 0 0 0))" );
  QVERIFY( resWktRead );

  t.addPolygon( p, 0 );  // must not crash - that's all we test here
}

void TestQgsTessellator::testBoundsScaling()
{
  QgsPolygon polygon;
  polygon.fromWkt( "POLYGON((1 1, 1.00000001 1, 1.00000001 1.00000001, 1 1.0000000001, 1 1))" );

  QList<TriangleCoords> tc;
  tc << TriangleCoords( QVector3D( 0, 1e-10f, 0 ), QVector3D( 1e-08f, 0, 0 ), QVector3D( 1e-08f, 1e-08f, 0 ), QVector3D( 0, 0, 1 ), QVector3D( 0, 0, 1 ), QVector3D( 0, 0, 1 ) );
  tc << TriangleCoords( QVector3D( 0, 1e-10f, 0 ), QVector3D( 0, 0, 0 ), QVector3D( 1e-08f, 0, 0 ), QVector3D( 0, 0, 1 ), QVector3D( 0, 0, 1 ), QVector3D( 0, 0, 1 ) );

  // without using bounds -- numerically unstable, expect no result
  QgsTessellator t( 0, 0, true );
  t.addPolygon( polygon, 0 );
  QCOMPARE( t.data().size(), 0 );

  // using bounds scaling, expect good result
  QgsTessellator t2( polygon.boundingBox(), true );
  t2.addPolygon( polygon, 0 );
  QVERIFY( checkTriangleOutput( t2.data(), true, tc ) );
  QCOMPARE( t2.zMinimum(), 0 );
  QCOMPARE( t2.zMaximum(), 0 );
}

void TestQgsTessellator::testNoZ()
{
  // test tessellation with no z support
  QgsPolygon polygonZ;
  polygonZ.fromWkt( "POLYGONZ((1 1 1, 2 1 1, 3 2 1, 1 2 1, 1 1 1))" );

  QList<TriangleCoords> tc;
  tc << TriangleCoords( QVector3D( 0, 1, 0 ), QVector3D( 1, 0, 0 ), QVector3D( 2, 1, 0 ) );
  tc << TriangleCoords( QVector3D( 0, 1, 0 ), QVector3D( 0, 0, 0 ), QVector3D( 1, 0, 0 ) );

  QgsTessellator t( polygonZ.boundingBox(), false, false, false, true );
  t.addPolygon( polygonZ, 0 );
  QVERIFY( checkTriangleOutput( t.data(), false, tc ) );
  QCOMPARE( t.zMinimum(), 0 );
  QCOMPARE( t.zMaximum(), 0 );
}

void TestQgsTessellator::testTriangulationDoesNotCrash()
{
  // a commit in poly2tri has caused a crashing regression - https://github.com/jhasse/poly2tri/issues/11
  // this code only makes sure that the crash does not come back during another update of poly2tri
  QgsPolygon polygon;
  polygon.fromWkt( "Polygon((0 0, -5 -3e-10, -10 -2e-10, -10 -4, 0 -4))" );
  QgsTessellator t( 0, 0, true );
  t.addPolygon( polygon, 0 );
}

void TestQgsTessellator::testCrash2DTriangle()
{
  // test tessellation of a 2D triangle - https://github.com/qgis/QGIS/issues/36024
  QgsPolygon polygon;
  polygon.fromWkt( "Polygon((0 0, 42 0, 42 42, 0 0))" );
  // must not be declared with mNoz = true
  QgsTessellator t( 0, 0, true );
  t.addPolygon( polygon, 0 ); // must not crash - that's all we test here
}

void TestQgsTessellator::narrowPolygon()
{
  // test that tessellation of a long narrow polygon results in a "nice" tessellation (i.e. not lots of long thin triangles)
  // refs https://github.com/qgis/QGIS/issues/37077
  QgsPolygon polygon;
  polygon.fromWkt( "Polygon ((383393.53728186257649213 4902093.79335568379610777, 383383.73728171654511243 4902092.99335567187517881, 383375.25399118528002873 4902092.8891992112621665, 383368.08741026872303337 4902093.48088630195707083, 383362.87084332120139152 4902093.91129046399146318, 383359.60429034277331084 4902094.18041169829666615, 383357.23274148383643478 4902093.6530067715793848, 383355.75619674433255568 4902092.32907568290829659, 383355.57501344417687505 4902090.56084021460264921, 383356.68919158342760056 4902088.34830036386847496, 383361.07830215193098411 4902086.48156050778925419, 383368.74234514962881804 4902084.96062064450234175, 383380.44909519288921729 4902084.10479906108230352, 383396.19855228182859719 4902083.91409575659781694, 383406.97328086948255077 4902084.21874411031603813, 383412.77328095590928569 4902085.01874412223696709, 383416.70496230950811878 4902086.31405469868332148, 383418.76832493022084236 4902088.1046758396551013, 383419.69647055567475036 4902089.60464367642998695, 383419.48939918575342745 4902090.81395820714533329, 383418.40130056580528617 4902092.01381655503064394, 383416.43217469577211887 4902093.20421871729195118, 383411.19502930447924882 4902093.89790377207100391, 383402.68986439192667603 4902094.09487171657383442, 383393.53728186257649213 4902093.79335568379610777))" );
  QgsTessellator t( polygon.boundingBox(), false );
  t.addPolygon( polygon, 0 );
  QgsGeometry res( t.asMultiPolygon() );
  res.translate( polygon.boundingBox().xMinimum(), polygon.boundingBox().yMinimum() );
  QgsDebugMsg( res.asWkt( 0 ) );
  QCOMPARE( res.asWkt( 0 ), QStringLiteral( "MultiPolygonZ (((383357 4902094 0, 383356 4902092 0, 383356 4902091 0, 383357 4902094 0)),((383357 4902088 0, 383357 4902094 0, 383356 4902091 0, 383357 4902088 0)),((383357 4902088 0, 383361 4902086 0, 383357 4902094 0, 383357 4902088 0)),((383357 4902094 0, 383361 4902086 0, 383360 4902094 0, 383357 4902094 0)),((383363 4902094 0, 383360 4902094 0, 383361 4902086 0, 383363 4902094 0)),((383368 4902093 0, 383363 4902094 0, 383361 4902086 0, 383368 4902093 0)),((383368 4902093 0, 383361 4902086 0, 383369 4902085 0, 383368 4902093 0)),((383368 4902093 0, 383369 4902085 0, 383375 4902093 0, 383368 4902093 0)),((383375 4902093 0, 383369 4902085 0, 383380 4902084 0, 383375 4902093 0)),((383375 4902093 0, 383380 4902084 0, 383384 4902093 0, 383375 4902093 0)),((383384 4902093 0, 383380 4902084 0, 383396 4902084 0, 383384 4902093 0)),((383394 4902094 0, 383384 4902093 0, 383396 4902084 0, 383394 4902094 0)),((383403 4902094 0, 383394 4902094 0, 383396 4902084 0, 383403 4902094 0)),((383396 4902084 0, 383407 4902084 0, 383403 4902094 0, 383396 4902084 0)),((383411 4902094 0, 383403 4902094 0, 383407 4902084 0, 383411 4902094 0)),((383411 4902094 0, 383407 4902084 0, 383413 4902085 0, 383411 4902094 0)),((383411 4902094 0, 383413 4902085 0, 383416 4902093 0, 383411 4902094 0)),((383416 4902093 0, 383413 4902085 0, 383417 4902086 0, 383416 4902093 0)),((383419 4902088 0, 383416 4902093 0, 383417 4902086 0, 383419 4902088 0)),((383418 4902092 0, 383416 4902093 0, 383419 4902088 0, 383418 4902092 0)),((383418 4902092 0, 383419 4902088 0, 383419 4902091 0, 383418 4902092 0)),((383419 4902091 0, 383419 4902088 0, 383420 4902090 0, 383419 4902091 0)))" ) );
}

QGSTEST_MAIN( TestQgsTessellator )
#include "testqgstessellator.moc"
