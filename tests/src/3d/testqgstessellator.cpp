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
  int valuesPerTriangle = withNormals ? 18 : 9;
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
    TriangleCoords out( dataRaw, withNormals );
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
  polygonZ.fromWkt( "POLYGONZ((1 1 0, 2 1 0, 3 2 0, 1 2 0, 1 1 0))" );

  QList<TriangleCoords> tc;
  tc << TriangleCoords( QVector3D( 1, 2, 0 ), QVector3D( 2, 1, 0 ), QVector3D( 3, 2, 0 ) );
  tc << TriangleCoords( QVector3D( 1, 2, 0 ), QVector3D( 1, 1, 0 ), QVector3D( 2, 1, 0 ) );

  QVector3D up( 0, 0, 1 );  // surface normal pointing straight up
  QList<TriangleCoords> tcNormals;
  tcNormals << TriangleCoords( QVector3D( 1, 2, 0 ), QVector3D( 2, 1, 0 ), QVector3D( 3, 2, 0 ), up, up, up );
  tcNormals << TriangleCoords( QVector3D( 1, 2, 0 ), QVector3D( 1, 1, 0 ), QVector3D( 2, 1, 0 ), up, up, up );

  // without normals

  QgsTessellator t( 0, 0, false );
  t.addPolygon( polygon, 0 );
  QVERIFY( checkTriangleOutput( t.data(), false, tc ) );

  QgsTessellator tZ( 0, 0, false );
  tZ.addPolygon( polygonZ, 0 );
  QVERIFY( checkTriangleOutput( tZ.data(), false, tc ) );

  // with normals

  QgsTessellator tN( 0, 0, true );
  tN.addPolygon( polygon, 0 );
  QVERIFY( checkTriangleOutput( tN.data(), true, tcNormals ) );

  QgsTessellator tNZ( 0, 0, true );
  tNZ.addPolygon( polygonZ, 0 );
  QVERIFY( checkTriangleOutput( tNZ.data(), true, tcNormals ) );
}

void TestQgsTessellator::testWalls()
{
  QgsPolygon rect;
  rect.fromWkt( "POLYGON((0 0, 3 0, 3 2, 0 2, 0 0))" );

  QVector3D zPos( 0, 0, 1 );
  QVector3D xPos( 1, 0, 0 );
  QVector3D yPos( 0, 1, 0 );
  QVector3D xNeg( -1, 0, 0 );
  QVector3D yNeg( 0, -1, 0 );

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

  // try to extrude a polygon with reverse (clock-wise) order of vertices and check it is still fine

  QgsPolygon rectRev;
  rectRev.fromWkt( "POLYGON((0 0, 0 2, 3 2, 3 0, 0 0))" );

  QgsTessellator tRectRev( 0, 0, true );
  tRectRev.addPolygon( rectRev, 1 );
  QVERIFY( checkTriangleOutput( tRectRev.data(), true, tcRect ) );

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
}

void TestQgsTessellator::testBackEdges()
{
  QgsPolygon polygon;
  polygon.fromWkt( "POLYGON((1 1, 2 1, 3 2, 1 2, 1 1))" );

  QVector3D up( 0, 0, 1 );  // surface normal pointing straight up
  QVector3D dn( 0, 0, -1 );  // surface normal pointing straight down for back faces
  QList<TriangleCoords> tcNormals;
  tcNormals << TriangleCoords( QVector3D( 1, 2, 0 ), QVector3D( 2, 1, 0 ), QVector3D( 3, 2, 0 ), up, up, up );
  tcNormals << TriangleCoords( QVector3D( 3, 2, 0 ), QVector3D( 2, 1, 0 ), QVector3D( 1, 2, 0 ), dn, dn, dn );
  tcNormals << TriangleCoords( QVector3D( 1, 2, 0 ), QVector3D( 1, 1, 0 ), QVector3D( 2, 1, 0 ), up, up, up );
  tcNormals << TriangleCoords( QVector3D( 2, 1, 0 ), QVector3D( 1, 1, 0 ), QVector3D( 1, 2, 0 ), dn, dn, dn );

  QgsTessellator tN( 0, 0, true, false, true );
  tN.addPolygon( polygon, 0 );
  QVERIFY( checkTriangleOutput( tN.data(), true, tcNormals ) );
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
}

void TestQgsTessellator::testIssue17745()
{
  // this is a rectangular polygon with collinear points that would crash poly2tri if coordinates do not get rounded a bit

  QgsTessellator t( 0, 0, true );
  QgsPolygon p;
  bool resWktRead = p.fromWkt( "Polygon((0 0, 1 1e-15, 4 0, 4 5, 1 5, 0 5, 0 0))" );
  QVERIFY( resWktRead );

  t.addPolygon( p, 0 );   // must not crash - that's all we test here
}

void TestQgsTessellator::testCrashSelfIntersection()
{
  // this is a polygon where we get self-intersecting exterior ring that would crash poly2tri if not skipped

  QgsTessellator t( 0, 0, true );
  QgsPolygon p;
  bool resWktRead = p.fromWkt( "PolygonZ ((-744809.80499999970197678 -1042371.96730000153183937 260.460968017578125, -744809.80299999937415123 -1042371.92199999839067459 260.460968017578125, -744810.21599999815225601 -1042381.09099999815225601 260.460968017578125, -744810.21499999985098839 -1042381.0689999982714653 260.460968017578125, -744812.96469999849796295 -1042375.32499999925494194 263.734283447265625, -744809.80499999970197678 -1042371.96730000153183937 260.460968017578125))" );

  QVERIFY( resWktRead );

  t.addPolygon( p, 0 );   // must not crash - that's all we test here
}

void TestQgsTessellator::testCrashEmptyPolygon()
{
  // this is a polygon that goes through GEOS simplification which throws an exception (and produces null geometry)

  QgsTessellator t( 0, 0, true );
  QgsPolygon p;
  bool resWktRead = p.fromWkt( "PolygonZ ((0 0 0, 0 0 0, 0 0 0))" );
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
}


QGSTEST_MAIN( TestQgsTessellator )
#include "testqgstessellator.moc"
