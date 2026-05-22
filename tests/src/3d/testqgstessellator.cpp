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

#include "qgsgeometry.h"
#include "qgslogger.h"
#include "qgsmultipolygon.h"
#include "qgspolygon.h"
#include "qgstessellator.h"
#include "qgstest.h"

#include <QString>
#include <QVector3D>
#include <QVector4D>

using namespace Qt::StringLiterals;

static bool qgsVectorNear( const QVector3D &v1, const QVector3D &v2, double eps )
{
  return qgsDoubleNear( v1.x(), v2.x(), eps ) && qgsDoubleNear( v1.y(), v2.y(), eps ) && qgsDoubleNear( v1.z(), v2.z(), eps );
}
static bool qgsVectorNear( const QVector4D &v1, const QVector4D &v2, double eps )
{
  return qgsDoubleNear( v1.x(), v2.x(), eps ) && qgsDoubleNear( v1.y(), v2.y(), eps ) && qgsDoubleNear( v1.z(), v2.z(), eps ) && qgsDoubleNear( v1.w(), v2.w(), eps );
}

/**
 * Simple structure to record an expected triangle from tessellator.
 * Triangle vertices are expected to be in counter-clockwise order.
 */
struct TriangleCoords
{
    //! Constructs from expected triangle coordinates
    TriangleCoords(
      const QVector3D &a,
      const QVector3D &b,
      const QVector3D &c,
      const QVector3D &na = QVector3D(),
      const QVector3D &nb = QVector3D(),
      const QVector3D &nc = QVector3D(),
      const QVector4D &ta = QVector4D(),
      const QVector4D &tb = QVector4D(),
      const QVector4D &tc = QVector4D()
    )
    {
      pts[0] = a;
      pts[1] = b;
      pts[2] = c;
      normals[0] = na;
      normals[1] = nb;
      normals[2] = nc;
      tangents[0] = ta;
      tangents[1] = tb;
      tangents[2] = tc;
    }


    //! Compares two triangles
    bool operator==( const TriangleCoords &other ) const
    {
      // TODO: allow that the two triangles have coordinates shifted (but still in the same order)
      const double eps = 1e-6;
      return qgsVectorNear( pts[0], other.pts[0], eps )
             && qgsVectorNear( pts[1], other.pts[1], eps )
             && qgsVectorNear( pts[2], other.pts[2], eps )
             && qgsVectorNear( normals[0], other.normals[0], eps )
             && qgsVectorNear( normals[1], other.normals[1], eps )
             && qgsVectorNear( normals[2], other.normals[2], eps )
             && qgsVectorNear( tangents[0], other.tangents[0], eps )
             && qgsVectorNear( tangents[1], other.tangents[1], eps )
             && qgsVectorNear( tangents[2], other.tangents[2], eps );
    }

    bool operator!=( const TriangleCoords &other ) const { return !operator==( other ); }

    bool operator<( const TriangleCoords &other ) const
    {
      const double eps = 1e-6;

      for ( int i = 0; i < 3; ++i )
      {
        if ( !qgsVectorNear( pts[i], other.pts[i], eps ) )
        {
          if ( !qgsDoubleNear( pts[i].x(), other.pts[i].x(), eps ) )
            return pts[i].x() < other.pts[i].x();
          if ( !qgsDoubleNear( pts[i].y(), other.pts[i].y(), eps ) )
            return pts[i].y() < other.pts[i].y();
          if ( !qgsDoubleNear( pts[i].z(), other.pts[i].z(), eps ) )
            return pts[i].z() < other.pts[i].z();
        }
      }

      for ( int i = 0; i < 3; ++i )
      {
        if ( !qgsVectorNear( normals[i], other.normals[i], eps ) )
        {
          if ( !qgsDoubleNear( normals[i].x(), other.normals[i].x(), eps ) )
            return normals[i].x() < other.normals[i].x();
          if ( !qgsDoubleNear( normals[i].y(), other.normals[i].y(), eps ) )
            return normals[i].y() < other.normals[i].y();
          if ( !qgsDoubleNear( normals[i].z(), other.normals[i].z(), eps ) )
            return normals[i].z() < other.normals[i].z();
        }
      }

      for ( int i = 0; i < 3; ++i )
      {
        if ( !qgsVectorNear( tangents[i], other.tangents[i], eps ) )
        {
          if ( !qgsDoubleNear( tangents[i].x(), other.tangents[i].x(), eps ) )
            return tangents[i].x() < other.tangents[i].x();
          if ( !qgsDoubleNear( tangents[i].y(), other.tangents[i].y(), eps ) )
            return tangents[i].y() < other.tangents[i].y();
          if ( !qgsDoubleNear( tangents[i].z(), other.tangents[i].z(), eps ) )
            return tangents[i].z() < other.tangents[i].z();
          if ( !qgsDoubleNear( tangents[i].w(), other.tangents[i].w(), eps ) )
            return tangents[i].w() < other.tangents[i].w();
        }
      }
      return false;
    }

    void dump() const { qDebug() << pts[0] << pts[1] << pts[2] << normals[0] << normals[1] << normals[2] << tangents[0] << tangents[1] << tangents[2]; }

    QVector3D pts[3];
    QVector3D normals[3];
    QVector4D tangents[3];
};

QList<TriangleCoords> extractTriangles( QgsTessellator &tessellator, bool withNormals, bool withTangents = false )
{
  QList<TriangleCoords> triangles;

  const QByteArray vertexBuffer = tessellator.vertexBuffer();
  const QByteArray indexBuffer = tessellator.indexBuffer();

  if ( vertexBuffer.isEmpty() || indexBuffer.isEmpty() )
    return triangles;

  const int vertexStride = tessellator.stride();
  const int indexStride = tessellator.indexStride();

  const char *vertices = vertexBuffer.constData();
  const char *indices = indexBuffer.constData();

  qsizetype triangleCount = indexBuffer.size() / static_cast<qsizetype>( 3 * indexStride );
  triangles.reserve( triangleCount );

  for ( qsizetype i = 0; i < triangleCount; i++ )
  {
    size_t index0 = 0;
    size_t index1 = 0;
    size_t index2 = 0;
    std::memcpy( &index0, indices + ( 3 * i + 0 ) * indexStride, indexStride );
    std::memcpy( &index1, indices + ( 3 * i + 1 ) * indexStride, indexStride );
    std::memcpy( &index2, indices + ( 3 * i + 2 ) * indexStride, indexStride );

    const float *vertex0 = reinterpret_cast<const float *>( vertices + index0 * vertexStride );
    const float *vertex1 = reinterpret_cast<const float *>( vertices + index1 * vertexStride );
    const float *vertex2 = reinterpret_cast<const float *>( vertices + index2 * vertexStride );

    QVector3D p0( vertex0[0], vertex0[1], vertex0[2] );
    QVector3D p1( vertex1[0], vertex1[1], vertex1[2] );
    QVector3D p2( vertex2[0], vertex2[1], vertex2[2] );

    if ( withNormals && withTangents )
    {
      QVector3D n0( vertex0[3], vertex0[4], vertex0[5] );
      QVector3D n1( vertex1[3], vertex1[4], vertex1[5] );
      QVector3D n2( vertex2[3], vertex2[4], vertex2[5] );

      QVector4D t0( vertex0[6], vertex0[7], vertex0[8], vertex0[9] );
      QVector4D t1( vertex1[6], vertex1[7], vertex1[8], vertex1[9] );
      QVector4D t2( vertex2[6], vertex2[7], vertex2[8], vertex2[9] );

      triangles.append( TriangleCoords( p0, p1, p2, n0, n1, n2, t0, t1, t2 ) );
    }
    else if ( withNormals )
    {
      QVector3D n0( vertex0[3], vertex0[4], vertex0[5] );
      QVector3D n1( vertex1[3], vertex1[4], vertex1[5] );
      QVector3D n2( vertex2[3], vertex2[4], vertex2[5] );

      triangles.append( TriangleCoords( p0, p1, p2, n0, n1, n2 ) );
    }
    else
    {
      triangles.append( TriangleCoords( p0, p1, p2 ) );
    }
  }

  return triangles;
}

bool checkTriangleOutput( const QList<TriangleCoords> &output, const QList<TriangleCoords> &expected )
{
  if ( output.size() != expected.size() )
  {
    qDebug() << "Expected size " << expected.size() << " got " << output.size();
    return false;
  }

  QList<TriangleCoords> sortedExpected = expected;
  std::sort( sortedExpected.begin(), sortedExpected.end() );

  QList<TriangleCoords> sortedOutput = output;
  std::sort( sortedOutput.begin(), sortedOutput.end() );

  for ( int i = 0; i < expected.count(); ++i )
  {
    const TriangleCoords &exp = sortedExpected.at( i );
    const TriangleCoords &out = sortedOutput.at( i );

    if ( exp != out )
    {
      qDebug() << i;
      qDebug() << "expected:";
      exp.dump();
      qDebug() << "got:";
      out.dump();
      return false;
    }
  }
  return true;
};


/**
 * \ingroup UnitTests
 * This is a unit test for the vertex tool
 */
class TestQgsTessellator : public QgsTest
{
    Q_OBJECT
  public:
    TestQgsTessellator()
      : QgsTest( u"Test QgsTessellator"_s ) {};

  private slots:
    void initTestCase();    // will be called before the first testfunction is executed.
    void cleanupTestCase(); // will be called after the last testfunction was executed.

    void testBasic();
    void testBasicClockwise();
    void testWalls();
    void testWallsTangents();
    void testBackEdges();
    void test2DTriangle();
    void test3DTriangle();
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
    void testOutputZUp();
    void testDuplicatePoints();
    void testRoofs();
    void testFloorsAndRoofs();

  private:
};

//runs before all tests
void TestQgsTessellator::initTestCase()
{}

//runs after all tests
void TestQgsTessellator::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestQgsTessellator::testBasic()
{
  QgsPolygon polygon;
  polygon.fromWkt( "POLYGON((1 1, 2 1, 3 2, 1 2, 1 1))" );

  QgsPolygon polygonZ;
  polygonZ.fromWkt( "POLYGONZ((1 1 3, 2 1 3, 3 2 3, 1 2 3, 1 1 3))" );

  QList<TriangleCoords> trianglesCD;
  trianglesCD << TriangleCoords( QVector3D( 1, 2, 0 ), QVector3D( 2, 1, 0 ), QVector3D( 3, 2, 0 ) );
  trianglesCD << TriangleCoords( QVector3D( 1, 2, 0 ), QVector3D( 1, 1, 0 ), QVector3D( 2, 1, 0 ) );

  QList<TriangleCoords> trianglesZCD;
  trianglesZCD << TriangleCoords( QVector3D( 1, 2, 3 ), QVector3D( 2, 1, 3 ), QVector3D( 3, 2, 3 ) );
  trianglesZCD << TriangleCoords( QVector3D( 1, 2, 3 ), QVector3D( 1, 1, 3 ), QVector3D( 2, 1, 3 ) );

  QList<TriangleCoords> trianglesEarcut;
  trianglesEarcut << TriangleCoords( QVector3D( 3, 2, 0 ), QVector3D( 1, 2, 0 ), QVector3D( 1, 1, 0 ) );
  trianglesEarcut << TriangleCoords( QVector3D( 1, 1, 0 ), QVector3D( 2, 1, 0 ), QVector3D( 3, 2, 0 ) );

  QList<TriangleCoords> trianglesZEarcut;
  trianglesZEarcut << TriangleCoords( QVector3D( 3, 2, 3 ), QVector3D( 1, 2, 3 ), QVector3D( 1, 1, 3 ) );
  trianglesZEarcut << TriangleCoords( QVector3D( 1, 1, 3 ), QVector3D( 2, 1, 3 ), QVector3D( 3, 2, 3 ) );

  const QVector3D up( 0, 0, 1 ); // surface normal pointing straight up
  QList<TriangleCoords> trianglesNormalsCD;
  trianglesNormalsCD << TriangleCoords( QVector3D( 1, 2, 0 ), QVector3D( 2, 1, 0 ), QVector3D( 3, 2, 0 ), up, up, up );
  trianglesNormalsCD << TriangleCoords( QVector3D( 1, 2, 0 ), QVector3D( 1, 1, 0 ), QVector3D( 2, 1, 0 ), up, up, up );

  QList<TriangleCoords> trianglesNormalsZCD;
  trianglesNormalsZCD << TriangleCoords( QVector3D( 1, 2, 3 ), QVector3D( 2, 1, 3 ), QVector3D( 3, 2, 3 ), up, up, up );
  trianglesNormalsZCD << TriangleCoords( QVector3D( 1, 2, 3 ), QVector3D( 1, 1, 3 ), QVector3D( 2, 1, 3 ), up, up, up );

  QList<TriangleCoords> trianglesNormalsTangentsCD;
  const QVector4D alongX( 1, 0, 0, 1 );
  trianglesNormalsTangentsCD << TriangleCoords( QVector3D( 1, 2, 0 ), QVector3D( 2, 1, 0 ), QVector3D( 3, 2, 0 ), up, up, up, alongX, alongX, alongX );
  trianglesNormalsTangentsCD << TriangleCoords( QVector3D( 1, 2, 0 ), QVector3D( 1, 1, 0 ), QVector3D( 2, 1, 0 ), up, up, up, alongX, alongX, alongX );

  QList<TriangleCoords> trianglesNormalsTangentsZCD;
  trianglesNormalsTangentsZCD << TriangleCoords( QVector3D( 1, 2, 3 ), QVector3D( 2, 1, 3 ), QVector3D( 3, 2, 3 ), up, up, up, alongX, alongX, alongX );
  trianglesNormalsTangentsZCD << TriangleCoords( QVector3D( 1, 2, 3 ), QVector3D( 1, 1, 3 ), QVector3D( 2, 1, 3 ), up, up, up, alongX, alongX, alongX );

  QList<TriangleCoords> trianglesNormalsEarcut;
  trianglesNormalsEarcut << TriangleCoords( QVector3D( 3, 2, 0 ), QVector3D( 1, 2, 0 ), QVector3D( 1, 1, 0 ), up, up, up );
  trianglesNormalsEarcut << TriangleCoords( QVector3D( 1, 1, 0 ), QVector3D( 2, 1, 0 ), QVector3D( 3, 2, 0 ), up, up, up );

  QList<TriangleCoords> trianglesNormalsZEarcut;
  trianglesNormalsZEarcut << TriangleCoords( QVector3D( 3, 2, 3 ), QVector3D( 1, 2, 3 ), QVector3D( 1, 1, 3 ), up, up, up );
  trianglesNormalsZEarcut << TriangleCoords( QVector3D( 1, 1, 3 ), QVector3D( 2, 1, 3 ), QVector3D( 3, 2, 3 ), up, up, up );
  // without normals

  QgsTessellator tesCD;
  tesCD.setOrigin( QgsVector3D( 0, 0, 0 ) );
  tesCD.setAddNormals( false );
  tesCD.addPolygon( polygon, 0 );
  QVERIFY( checkTriangleOutput( extractTriangles( tesCD, false ), trianglesCD ) );

  QCOMPARE( tesCD.zMinimum(), 0 );
  QCOMPARE( tesCD.zMaximum(), 0 );

  QgsTessellator tesZCD;
  tesZCD.setAddNormals( false );
  tesZCD.addPolygon( polygonZ, 0 );
  QVERIFY( checkTriangleOutput( extractTriangles( tesZCD, false ), trianglesZCD ) );

  QgsTessellator tesEarcut;
  tesEarcut.setOrigin( QgsVector3D( 0, 0, 0 ) );
  tesEarcut.setAddNormals( false );
  tesEarcut.setTriangulationAlgorithm( Qgis::TriangulationAlgorithm::Earcut );
  tesEarcut.addPolygon( polygon, 0 );
  QVERIFY( checkTriangleOutput( extractTriangles( tesEarcut, false ), trianglesEarcut ) );

  QCOMPARE( tesEarcut.zMinimum(), 0 );
  QCOMPARE( tesEarcut.zMaximum(), 0 );

  QgsTessellator tesZEarcut;
  tesZEarcut.setAddNormals( false );
  tesZEarcut.setTriangulationAlgorithm( Qgis::TriangulationAlgorithm::Earcut );
  tesZEarcut.addPolygon( polygonZ, 0 );
  QVERIFY( checkTriangleOutput( extractTriangles( tesZEarcut, false ), trianglesZEarcut ) );

  QCOMPARE( tesZEarcut.zMinimum(), 3 );
  QCOMPARE( tesZEarcut.zMaximum(), 3 );
  // with normals

  QgsTessellator tesNormalsCD;
  tesNormalsCD.setAddNormals( true );
  tesNormalsCD.addPolygon( polygon, 0 );
  QVERIFY( checkTriangleOutput( extractTriangles( tesNormalsCD, true ), trianglesNormalsCD ) );
  QCOMPARE( tesNormalsCD.zMinimum(), 0 );
  QCOMPARE( tesNormalsCD.zMaximum(), 0 );

  QgsTessellator tesNormalsZCD;
  tesNormalsZCD.setAddNormals( true );
  tesNormalsZCD.addPolygon( polygonZ, 0 );
  QVERIFY( checkTriangleOutput( extractTriangles( tesNormalsZCD, true ), trianglesNormalsZCD ) );

  QCOMPARE( tesNormalsZCD.zMinimum(), 3 );
  QCOMPARE( tesNormalsZCD.zMaximum(), 3 );

  // with tangents
  QgsTessellator tesNormalsTangentsCD;
  tesNormalsTangentsCD.setAddNormals( true );
  tesNormalsTangentsCD.setAddTangents( true );
  tesNormalsTangentsCD.addPolygon( polygon, 0 );
  QVERIFY( checkTriangleOutput( extractTriangles( tesNormalsTangentsCD, true, true ), trianglesNormalsTangentsCD ) );
  QCOMPARE( tesNormalsTangentsCD.zMinimum(), 0 );
  QCOMPARE( tesNormalsTangentsCD.zMaximum(), 0 );

  QgsTessellator tesNormalsTangentsZCD;
  tesNormalsTangentsZCD.setAddNormals( true );
  tesNormalsTangentsZCD.setAddTangents( true );
  tesNormalsTangentsZCD.addPolygon( polygonZ, 0 );
  QVERIFY( checkTriangleOutput( extractTriangles( tesNormalsTangentsZCD, true, true ), trianglesNormalsTangentsZCD ) );
  QCOMPARE( tesNormalsTangentsZCD.zMinimum(), 3 );
  QCOMPARE( tesNormalsTangentsZCD.zMaximum(), 3 );

  QgsTessellator tesNormalsEarcut;
  tesNormalsEarcut.setAddNormals( true );
  tesNormalsEarcut.setTriangulationAlgorithm( Qgis::TriangulationAlgorithm::Earcut );
  tesNormalsEarcut.addPolygon( polygon, 0 );
  QVERIFY( checkTriangleOutput( extractTriangles( tesNormalsEarcut, true ), trianglesNormalsEarcut ) );

  QCOMPARE( tesNormalsEarcut.zMinimum(), 0 );
  QCOMPARE( tesNormalsEarcut.zMaximum(), 0 );

  QgsTessellator tesNormalsZEarcut;
  tesNormalsZEarcut.setAddNormals( true );
  tesNormalsZEarcut.setTriangulationAlgorithm( Qgis::TriangulationAlgorithm::Earcut );
  tesNormalsZEarcut.addPolygon( polygonZ, 0 );
  QVERIFY( checkTriangleOutput( extractTriangles( tesNormalsZEarcut, true ), trianglesNormalsZEarcut ) );

  QCOMPARE( tesNormalsZEarcut.zMinimum(), 3 );
  QCOMPARE( tesNormalsZEarcut.zMaximum(), 3 );
  // with invert normals enabled, normals point down and triangles are reversed to clockwise
  const QVector3D down( 0, 0, -1 ); // surface normal pointing straight down
  QList<TriangleCoords> trianglesInvertedNormalsCD;
  trianglesInvertedNormalsCD << TriangleCoords( QVector3D( 2, 1, 0 ), QVector3D( 1, 1, 0 ), QVector3D( 1, 2, 0 ), down, down, down );
  trianglesInvertedNormalsCD << TriangleCoords( QVector3D( 3, 2, 0 ), QVector3D( 2, 1, 0 ), QVector3D( 1, 2, 0 ), down, down, down );

  QList<TriangleCoords> trianglesInvertedNormalsZCD;
  trianglesInvertedNormalsZCD << TriangleCoords( QVector3D( 2, 1, 3 ), QVector3D( 1, 1, 3 ), QVector3D( 1, 2, 3 ), down, down, down );
  trianglesInvertedNormalsZCD << TriangleCoords( QVector3D( 3, 2, 3 ), QVector3D( 2, 1, 3 ), QVector3D( 1, 2, 3 ), down, down, down );

  QList<TriangleCoords> trianglesInvertedNormalsEarcut;
  trianglesInvertedNormalsEarcut << TriangleCoords( QVector3D( 2, 1, 0 ), QVector3D( 1, 1, 0 ), QVector3D( 1, 2, 0 ), down, down, down );
  trianglesInvertedNormalsEarcut << TriangleCoords( QVector3D( 1, 2, 0 ), QVector3D( 3, 2, 0 ), QVector3D( 2, 1, 0 ), down, down, down );

  QList<TriangleCoords> trianglesInvertedNormalsZEarcut;
  trianglesInvertedNormalsZEarcut << TriangleCoords( QVector3D( 1, 2, 3 ), QVector3D( 3, 2, 3 ), QVector3D( 2, 1, 3 ), down, down, down );
  trianglesInvertedNormalsZEarcut << TriangleCoords( QVector3D( 2, 1, 3 ), QVector3D( 1, 1, 3 ), QVector3D( 1, 2, 3 ), down, down, down );

  QgsTessellator tesInvertedNormalsCD;
  tesInvertedNormalsCD.setAddNormals( true );
  tesInvertedNormalsCD.setInvertNormals( true );
  tesInvertedNormalsCD.addPolygon( polygon, 0 );
  QVERIFY( checkTriangleOutput( extractTriangles( tesInvertedNormalsCD, true ), trianglesInvertedNormalsCD ) );

  QCOMPARE( tesInvertedNormalsCD.zMinimum(), 0 );
  QCOMPARE( tesInvertedNormalsCD.zMaximum(), 0 );

  QgsTessellator tesInvertedNormalsZCD;
  tesInvertedNormalsZCD.setExtrusionFaces( Qgis::ExtrusionFace::Walls | Qgis::ExtrusionFace::Roof );
  tesInvertedNormalsZCD.setAddNormals( true );
  tesInvertedNormalsZCD.setInvertNormals( true );
  tesInvertedNormalsZCD.addPolygon( polygonZ, 0 );
  QVERIFY( checkTriangleOutput( extractTriangles( tesInvertedNormalsZCD, true ), trianglesInvertedNormalsZCD ) );

  QCOMPARE( tesInvertedNormalsZCD.zMinimum(), 3 );
  QCOMPARE( tesInvertedNormalsZCD.zMaximum(), 3 );

  QgsTessellator tesInvertedNormalsEarcut;
  tesInvertedNormalsEarcut.setAddNormals( true );
  tesInvertedNormalsEarcut.setInvertNormals( true );
  tesInvertedNormalsEarcut.setTriangulationAlgorithm( Qgis::TriangulationAlgorithm::Earcut );
  tesInvertedNormalsEarcut.addPolygon( polygon, 0 );
  QVERIFY( checkTriangleOutput( extractTriangles( tesInvertedNormalsEarcut, true ), trianglesInvertedNormalsEarcut ) );

  QCOMPARE( tesInvertedNormalsEarcut.zMinimum(), 0 );
  QCOMPARE( tesInvertedNormalsEarcut.zMaximum(), 0 );

  QgsTessellator tesInvertedNormalsZEarcut;
  tesInvertedNormalsZEarcut.setAddNormals( true );
  tesInvertedNormalsZEarcut.setInvertNormals( true );
  tesInvertedNormalsZEarcut.setTriangulationAlgorithm( Qgis::TriangulationAlgorithm::Earcut );
  tesInvertedNormalsZEarcut.addPolygon( polygonZ, 0 );
  QVERIFY( checkTriangleOutput( extractTriangles( tesInvertedNormalsZEarcut, true ), trianglesInvertedNormalsZEarcut ) );

  QCOMPARE( tesInvertedNormalsZEarcut.zMinimum(), 3 );
  QCOMPARE( tesInvertedNormalsZEarcut.zMaximum(), 3 );
}

void TestQgsTessellator::testBasicClockwise()
{
  // Clockwise polygons are facing down, not up, even when flat
  // The triangles also face down and are clockwise
  QgsPolygon polygon;
  polygon.fromWkt( "POLYGON((1 1, 1 2, 3 2, 2 1, 1 1))" );

  QgsPolygon polygonZ;
  polygonZ.fromWkt( "POLYGONZ((1 1 3, 1 2 3, 3 2 3, 2 1 3, 1 1 3))" );

  QList<TriangleCoords> trianglesCD;
  trianglesCD << TriangleCoords( QVector3D( 2, 1, 0 ), QVector3D( 1, 1, 0 ), QVector3D( 1, 2, 0 ) );
  trianglesCD << TriangleCoords( QVector3D( 3, 2, 0 ), QVector3D( 2, 1, 0 ), QVector3D( 1, 2, 0 ) );

  QList<TriangleCoords> trianglesZCD;
  trianglesZCD << TriangleCoords( QVector3D( 2, 1, 3 ), QVector3D( 1, 1, 3 ), QVector3D( 1, 2, 3 ) );
  trianglesZCD << TriangleCoords( QVector3D( 3, 2, 3 ), QVector3D( 2, 1, 3 ), QVector3D( 1, 2, 3 ) );

  QList<TriangleCoords> trianglesEarcut;
  trianglesEarcut << TriangleCoords( QVector3D( 3, 2, 0 ), QVector3D( 2, 1, 0 ), QVector3D( 1, 1, 0 ) );
  trianglesEarcut << TriangleCoords( QVector3D( 1, 1, 0 ), QVector3D( 1, 2, 0 ), QVector3D( 3, 2, 0 ) );

  QList<TriangleCoords> trianglesZEarcut;
  trianglesZEarcut << TriangleCoords( QVector3D( 3, 2, 3 ), QVector3D( 2, 1, 3 ), QVector3D( 1, 1, 3 ) );
  trianglesZEarcut << TriangleCoords( QVector3D( 1, 1, 3 ), QVector3D( 1, 2, 3 ), QVector3D( 3, 2, 3 ) );

  const QVector3D down( 0, 0, -1 ); // surface normal pointing straight down
  QList<TriangleCoords> trianglesNormalsCD;
  trianglesNormalsCD << TriangleCoords( QVector3D( 2, 1, 0 ), QVector3D( 1, 1, 0 ), QVector3D( 1, 2, 0 ), down, down, down );
  trianglesNormalsCD << TriangleCoords( QVector3D( 3, 2, 0 ), QVector3D( 2, 1, 0 ), QVector3D( 1, 2, 0 ), down, down, down );

  QList<TriangleCoords> trianglesNormalsZCD;
  trianglesNormalsZCD << TriangleCoords( QVector3D( 2, 1, 3 ), QVector3D( 1, 1, 3 ), QVector3D( 1, 2, 3 ), down, down, down );
  trianglesNormalsZCD << TriangleCoords( QVector3D( 3, 2, 3 ), QVector3D( 2, 1, 3 ), QVector3D( 1, 2, 3 ), down, down, down );

  const QVector4D alongX( 1, 0, 0, 1 );
  QList<TriangleCoords> trianglesNormalsTangentsCD;
  trianglesNormalsTangentsCD << TriangleCoords( QVector3D( 2, 1, 0 ), QVector3D( 1, 1, 0 ), QVector3D( 1, 2, 0 ), down, down, down, alongX, alongX, alongX );
  trianglesNormalsTangentsCD << TriangleCoords( QVector3D( 3, 2, 0 ), QVector3D( 2, 1, 0 ), QVector3D( 1, 2, 0 ), down, down, down, alongX, alongX, alongX );

  QList<TriangleCoords> trianglesNormalsTangentsZCD;
  trianglesNormalsTangentsZCD << TriangleCoords( QVector3D( 2, 1, 3 ), QVector3D( 1, 1, 3 ), QVector3D( 1, 2, 3 ), down, down, down, alongX, alongX, alongX );
  trianglesNormalsTangentsZCD << TriangleCoords( QVector3D( 3, 2, 3 ), QVector3D( 2, 1, 3 ), QVector3D( 1, 2, 3 ), down, down, down, alongX, alongX, alongX );

  QList<TriangleCoords> trianglesNormalsEarcut;
  trianglesNormalsEarcut << TriangleCoords( QVector3D( 3, 2, 0 ), QVector3D( 2, 1, 0 ), QVector3D( 1, 1, 0 ), down, down, down );
  trianglesNormalsEarcut << TriangleCoords( QVector3D( 1, 1, 0 ), QVector3D( 1, 2, 0 ), QVector3D( 3, 2, 0 ), down, down, down );

  QList<TriangleCoords> trianglesNormalsZEarcut;
  trianglesNormalsZEarcut << TriangleCoords( QVector3D( 3, 2, 3 ), QVector3D( 2, 1, 3 ), QVector3D( 1, 1, 3 ), down, down, down );
  trianglesNormalsZEarcut << TriangleCoords( QVector3D( 1, 1, 3 ), QVector3D( 1, 2, 3 ), QVector3D( 3, 2, 3 ), down, down, down );

  // without normals

  QgsTessellator tesCD;
  tesCD.addPolygon( polygon, 0 );
  QVERIFY( checkTriangleOutput( extractTriangles( tesCD, false ), trianglesCD ) );

  QCOMPARE( tesCD.zMinimum(), 0 );
  QCOMPARE( tesCD.zMaximum(), 0 );

  QgsTessellator tesZCD;
  tesZCD.addPolygon( polygonZ, 0 );
  QVERIFY( checkTriangleOutput( extractTriangles( tesZCD, false ), trianglesZCD ) );

  QCOMPARE( tesZCD.zMinimum(), 3 );
  QCOMPARE( tesZCD.zMaximum(), 3 );
  // with normals

  QgsTessellator tesN;
  tesN.setAddNormals( true );
  tesN.addPolygon( polygon, 0 );
  QVERIFY( checkTriangleOutput( extractTriangles( tesN, true ), trianglesNormalsCD ) );

  QCOMPARE( tesN.zMinimum(), 0 );
  QCOMPARE( tesN.zMaximum(), 0 );

  QgsTessellator tesNZ;
  tesNZ.setAddNormals( true );
  tesNZ.addPolygon( polygonZ, 0 );
  QVERIFY( checkTriangleOutput( extractTriangles( tesNZ, true ), trianglesNormalsZCD ) );

  QCOMPARE( tesNZ.zMinimum(), 3 );
  QCOMPARE( tesNZ.zMaximum(), 3 );

  // with tangents

  QgsTessellator tesNT;
  tesNT.setAddNormals( true );
  tesNT.setAddTangents( true );
  tesNT.addPolygon( polygon, 0 );
  QVERIFY( checkTriangleOutput( extractTriangles( tesNT, true, true ), trianglesNormalsTangentsCD ) );
  QCOMPARE( tesNT.zMinimum(), 0 );
  QCOMPARE( tesNT.zMaximum(), 0 );

  QgsTessellator tesNZT;
  tesNZT.setAddNormals( true );
  tesNZT.setAddTangents( true );
  tesNZT.addPolygon( polygonZ, 0 );
  QVERIFY( checkTriangleOutput( extractTriangles( tesNZT, true, true ), trianglesNormalsTangentsZCD ) );
  QCOMPARE( tesNZT.zMinimum(), 3 );
  QCOMPARE( tesNZT.zMaximum(), 3 );

  QgsTessellator tesEarcut;
  tesEarcut.setAddNormals( true );
  tesEarcut.setTriangulationAlgorithm( Qgis::TriangulationAlgorithm::Earcut );
  tesEarcut.addPolygon( polygon, 0 );
  QVERIFY( checkTriangleOutput( extractTriangles( tesEarcut, true ), trianglesNormalsEarcut ) );

  QCOMPARE( tesEarcut.zMinimum(), 0 );
  QCOMPARE( tesEarcut.zMaximum(), 0 );

  QgsTessellator tesZEarcut;
  tesZEarcut.setAddNormals( true );
  tesZEarcut.setTriangulationAlgorithm( Qgis::TriangulationAlgorithm::Earcut );
  tesZEarcut.addPolygon( polygonZ, 0 );
  QVERIFY( checkTriangleOutput( extractTriangles( tesZEarcut, true ), trianglesNormalsZEarcut ) );

  QCOMPARE( tesZEarcut.zMinimum(), 3 );
  QCOMPARE( tesZEarcut.zMaximum(), 3 );

  // with invert normals enabled, normals point up and triangles are reversed to counter clockwise
  const QVector3D up( 0, 0, 1 ); // surface normal pointing straight up
  QList<TriangleCoords> trianglesInvertedNormalsCD;
  trianglesInvertedNormalsCD << TriangleCoords( QVector3D( 1, 2, 0 ), QVector3D( 2, 1, 0 ), QVector3D( 3, 2, 0 ), up, up, up );
  trianglesInvertedNormalsCD << TriangleCoords( QVector3D( 1, 2, 0 ), QVector3D( 1, 1, 0 ), QVector3D( 2, 1, 0 ), up, up, up );

  QList<TriangleCoords> trianglesInvertedNormalsZCD;
  trianglesInvertedNormalsZCD << TriangleCoords( QVector3D( 1, 2, 3 ), QVector3D( 2, 1, 3 ), QVector3D( 3, 2, 3 ), up, up, up );
  trianglesInvertedNormalsZCD << TriangleCoords( QVector3D( 1, 2, 3 ), QVector3D( 1, 1, 3 ), QVector3D( 2, 1, 3 ), up, up, up );

  QList<TriangleCoords> trianglesInvertedNormalsEarcut;
  trianglesInvertedNormalsEarcut << TriangleCoords( QVector3D( 2, 1, 0 ), QVector3D( 3, 2, 0 ), QVector3D( 1, 2, 0 ), up, up, up );
  trianglesInvertedNormalsEarcut << TriangleCoords( QVector3D( 1, 2, 0 ), QVector3D( 1, 1, 0 ), QVector3D( 2, 1, 0 ), up, up, up );

  QList<TriangleCoords> trianglesInvertedNormalsZEarcut;
  trianglesInvertedNormalsZEarcut << TriangleCoords( QVector3D( 1, 2, 3 ), QVector3D( 1, 1, 3 ), QVector3D( 2, 1, 3 ), up, up, up );
  trianglesInvertedNormalsZEarcut << TriangleCoords( QVector3D( 2, 1, 3 ), QVector3D( 3, 2, 3 ), QVector3D( 1, 2, 3 ), up, up, up );

  QgsTessellator tesInvertedNormalsCD;
  tesInvertedNormalsCD.setAddNormals( true );
  tesInvertedNormalsCD.setInvertNormals( true );
  tesInvertedNormalsCD.addPolygon( polygon, 0 );
  QVERIFY( checkTriangleOutput( extractTriangles( tesInvertedNormalsCD, true ), trianglesInvertedNormalsCD ) );

  QCOMPARE( tesInvertedNormalsCD.zMinimum(), 0 );
  QCOMPARE( tesInvertedNormalsCD.zMaximum(), 0 );

  QgsTessellator tesInvertedNormalsZCD;
  tesInvertedNormalsZCD.setAddNormals( true );
  tesInvertedNormalsZCD.setInvertNormals( true );
  tesInvertedNormalsZCD.addPolygon( polygonZ, 0 );
  QVERIFY( checkTriangleOutput( extractTriangles( tesInvertedNormalsZCD, true ), trianglesInvertedNormalsZCD ) );

  QCOMPARE( tesInvertedNormalsZCD.zMinimum(), 3 );
  QCOMPARE( tesInvertedNormalsZCD.zMaximum(), 3 );

  QgsTessellator tesInvertedNormalsEarcut;
  tesInvertedNormalsEarcut.setAddNormals( true );
  tesInvertedNormalsEarcut.setInvertNormals( true );
  tesInvertedNormalsEarcut.setTriangulationAlgorithm( Qgis::TriangulationAlgorithm::Earcut );
  tesInvertedNormalsEarcut.addPolygon( polygon, 0 );
  QVERIFY( checkTriangleOutput( extractTriangles( tesInvertedNormalsEarcut, true ), trianglesInvertedNormalsEarcut ) );

  QCOMPARE( tesInvertedNormalsEarcut.zMinimum(), 0 );
  QCOMPARE( tesInvertedNormalsEarcut.zMaximum(), 0 );

  QgsTessellator tesInvertedNormalsZEarcut;
  tesInvertedNormalsZEarcut.setAddNormals( true );
  tesInvertedNormalsZEarcut.setInvertNormals( true );
  tesInvertedNormalsZEarcut.setTriangulationAlgorithm( Qgis::TriangulationAlgorithm::Earcut );
  tesInvertedNormalsZEarcut.addPolygon( polygonZ, 0 );
  QVERIFY( checkTriangleOutput( extractTriangles( tesInvertedNormalsZEarcut, true ), trianglesInvertedNormalsZEarcut ) );

  QCOMPARE( tesInvertedNormalsZEarcut.zMinimum(), 3 );
  QCOMPARE( tesInvertedNormalsZEarcut.zMaximum(), 3 );
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

  QgsTessellator tRect;
  tRect.setAddNormals( true );
  tRect.addPolygon( rect, 1 );
  QVERIFY( checkTriangleOutput( extractTriangles( tRect, true ), tcRect ) );
  QCOMPARE( tRect.zMinimum(), 0 );
  QCOMPARE( tRect.zMaximum(), 1 );

  // try to extrude a polygon with reverse (clock-wise) order of vertices and check it is still fine

  QgsPolygon rectRev;
  rectRev.fromWkt( "POLYGON((0 0, 0 2, 3 2, 3 0, 0 0))" );

  QgsTessellator tRectRev;
  tRectRev.setAddNormals( true );
  tRectRev.addPolygon( rectRev, 1 );
  QVERIFY( checkTriangleOutput( extractTriangles( tRectRev, true ), tcRect ) );
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

  QgsTessellator tZ;
  tZ.addPolygon( polygonZ, 10 );
  QVERIFY( checkTriangleOutput( extractTriangles( tZ, false ), tc ) );

  QCOMPARE( tZ.zMinimum(), 1 );
  QCOMPARE( tZ.zMaximum(), 14 );
}

void TestQgsTessellator::testWallsTangents()
{
  QgsPolygon rect;
  rect.fromWkt( "POLYGON((0 0, 3 0, 3 2, 0 2, 0 0))" );

  const QVector3D zPos( 0, 0, 1 );
  const QVector3D xPos( 1, 0, 0 );
  const QVector3D yPos( 0, 1, 0 );
  const QVector3D xNeg( -1, 0, 0 );
  const QVector3D yNeg( 0, -1, 0 );

  QList<TriangleCoords> tcRect;
  tcRect << TriangleCoords( QVector3D( 0, 2, 1 ), QVector3D( 3, 0, 1 ), QVector3D( 3, 2, 1 ), zPos, zPos, zPos, QVector4D( 1, 0, 0, 1 ), QVector4D( 1, 0, 0, 1 ), QVector4D( 1, 0, 0, 1 ) );
  tcRect << TriangleCoords( QVector3D( 0, 2, 1 ), QVector3D( 0, 0, 1 ), QVector3D( 3, 0, 1 ), zPos, zPos, zPos, QVector4D( 1, 0, 0, 1 ), QVector4D( 1, 0, 0, 1 ), QVector4D( 1, 0, 0, 1 ) );
  tcRect << TriangleCoords( QVector3D( 0, 0, 1 ), QVector3D( 0, 2, 1 ), QVector3D( 0, 0, 0 ), xNeg, xNeg, xNeg, QVector4D( 0, -1, 0, 1 ), QVector4D( 0, -1, 0, 1 ), QVector4D( 0, -1, 0, 1 ) );
  tcRect << TriangleCoords( QVector3D( 0, 0, 0 ), QVector3D( 0, 2, 1 ), QVector3D( 0, 2, 0 ), xNeg, xNeg, xNeg, QVector4D( 0, -1, 0, 1 ), QVector4D( 0, -1, 0, 1 ), QVector4D( 0, -1, 0, 1 ) );
  tcRect << TriangleCoords( QVector3D( 0, 2, 1 ), QVector3D( 3, 2, 1 ), QVector3D( 0, 2, 0 ), yPos, yPos, yPos, QVector4D( -1, 0, 0, 1 ), QVector4D( -1, 0, 0, 1 ), QVector4D( -1, 0, 0, 1 ) );
  tcRect << TriangleCoords( QVector3D( 0, 2, 0 ), QVector3D( 3, 2, 1 ), QVector3D( 3, 2, 0 ), yPos, yPos, yPos, QVector4D( -1, 0, 0, 1 ), QVector4D( -1, 0, 0, 1 ), QVector4D( -1, 0, 0, 1 ) );
  tcRect << TriangleCoords( QVector3D( 3, 2, 1 ), QVector3D( 3, 0, 1 ), QVector3D( 3, 2, 0 ), xPos, xPos, xPos, QVector4D( 0, 1, 0, 1 ), QVector4D( 0, 1, 0, 1 ), QVector4D( 0, 1, 0, 1 ) );
  tcRect << TriangleCoords( QVector3D( 3, 2, 0 ), QVector3D( 3, 0, 1 ), QVector3D( 3, 0, 0 ), xPos, xPos, xPos, QVector4D( 0, 1, 0, 1 ), QVector4D( 0, 1, 0, 1 ), QVector4D( 0, 1, 0, 1 ) );
  tcRect << TriangleCoords( QVector3D( 3, 0, 1 ), QVector3D( 0, 0, 1 ), QVector3D( 3, 0, 0 ), yNeg, yNeg, yNeg, QVector4D( 1, 0, 0, 1 ), QVector4D( 1, 0, 0, 1 ), QVector4D( 1, 0, 0, 1 ) );
  tcRect << TriangleCoords( QVector3D( 3, 0, 0 ), QVector3D( 0, 0, 1 ), QVector3D( 0, 0, 0 ), yNeg, yNeg, yNeg, QVector4D( 1, 0, 0, 1 ), QVector4D( 1, 0, 0, 1 ), QVector4D( 1, 0, 0, 1 ) );

  QgsTessellator tRect;
  tRect.setAddNormals( true );
  tRect.setAddTangents( true );
  tRect.addPolygon( rect, 1 );
  QVERIFY( checkTriangleOutput( extractTriangles( tRect, true, true ), tcRect ) );
  QCOMPARE( tRect.zMinimum(), 0 );
  QCOMPARE( tRect.zMaximum(), 1 );

  // try to extrude a polygon with reverse (clock-wise) order of vertices and check it is still fine

  QgsPolygon rectRev;
  rectRev.fromWkt( "POLYGON((0 0, 0 2, 3 2, 3 0, 0 0))" );

  QgsTessellator tRectRev;
  tRectRev.setAddNormals( true );
  tRectRev.setAddTangents( true );
  tRectRev.addPolygon( rectRev, 1 );
  QVERIFY( checkTriangleOutput( extractTriangles( tRectRev, true, true ), tcRect ) );
  QCOMPARE( tRectRev.zMinimum(), 0 );
  QCOMPARE( tRectRev.zMaximum(), 1 );
}

void TestQgsTessellator::testBackEdges()
{
  QgsPolygon rect;
  rect.fromWkt( "POLYGON((0 0, 3 0, 3 2, 0 2, 0 0))" );

  const QVector3D zPos( 0, 0, 1 );
  const QVector3D zNeg( 0, 0, -1 );
  const QVector3D xPos( 1, 0, 0 );
  const QVector3D xNeg( -1, 0, 0 );
  const QVector3D yPos( 0, 1, 0 );
  const QVector3D yNeg( 0, -1, 0 );

  QList<TriangleCoords> tc;

  tc << TriangleCoords( QVector3D( 0, 2, 1 ), QVector3D( 3, 0, 1 ), QVector3D( 3, 2, 1 ), zPos, zPos, zPos );
  tc << TriangleCoords( QVector3D( 0, 2, 1 ), QVector3D( 0, 0, 1 ), QVector3D( 3, 0, 1 ), zPos, zPos, zPos );
  tc << TriangleCoords( QVector3D( 3, 2, 1 ), QVector3D( 3, 0, 1 ), QVector3D( 0, 2, 1 ), zNeg, zNeg, zNeg );
  tc << TriangleCoords( QVector3D( 3, 0, 1 ), QVector3D( 0, 0, 1 ), QVector3D( 0, 2, 1 ), zNeg, zNeg, zNeg );

  tc << TriangleCoords( QVector3D( 0, 2, 0 ), QVector3D( 3, 0, 0 ), QVector3D( 3, 2, 0 ), zPos, zPos, zPos );
  tc << TriangleCoords( QVector3D( 0, 2, 0 ), QVector3D( 0, 0, 0 ), QVector3D( 3, 0, 0 ), zPos, zPos, zPos );
  tc << TriangleCoords( QVector3D( 3, 2, 0 ), QVector3D( 3, 0, 0 ), QVector3D( 0, 2, 0 ), zNeg, zNeg, zNeg );
  tc << TriangleCoords( QVector3D( 3, 0, 0 ), QVector3D( 0, 0, 0 ), QVector3D( 0, 2, 0 ), zNeg, zNeg, zNeg );

  tc << TriangleCoords( QVector3D( 0, 0, 1 ), QVector3D( 0, 2, 1 ), QVector3D( 0, 0, 0 ), xNeg, xNeg, xNeg );
  tc << TriangleCoords( QVector3D( 0, 0, 0 ), QVector3D( 0, 2, 1 ), QVector3D( 0, 2, 0 ), xNeg, xNeg, xNeg );
  tc << TriangleCoords( QVector3D( 0, 2, 1 ), QVector3D( 0, 0, 1 ), QVector3D( 0, 2, 0 ), xPos, xPos, xPos );
  tc << TriangleCoords( QVector3D( 0, 2, 0 ), QVector3D( 0, 0, 1 ), QVector3D( 0, 0, 0 ), xPos, xPos, xPos );

  tc << TriangleCoords( QVector3D( 0, 2, 1 ), QVector3D( 3, 2, 1 ), QVector3D( 0, 2, 0 ), yPos, yPos, yPos );
  tc << TriangleCoords( QVector3D( 0, 2, 0 ), QVector3D( 3, 2, 1 ), QVector3D( 3, 2, 0 ), yPos, yPos, yPos );
  tc << TriangleCoords( QVector3D( 3, 2, 1 ), QVector3D( 0, 2, 1 ), QVector3D( 3, 2, 0 ), yNeg, yNeg, yNeg );
  tc << TriangleCoords( QVector3D( 3, 2, 0 ), QVector3D( 0, 2, 1 ), QVector3D( 0, 2, 0 ), yNeg, yNeg, yNeg );

  tc << TriangleCoords( QVector3D( 3, 2, 1 ), QVector3D( 3, 0, 1 ), QVector3D( 3, 2, 0 ), xPos, xPos, xPos );
  tc << TriangleCoords( QVector3D( 3, 2, 0 ), QVector3D( 3, 0, 1 ), QVector3D( 3, 0, 0 ), xPos, xPos, xPos );
  tc << TriangleCoords( QVector3D( 3, 0, 1 ), QVector3D( 3, 2, 1 ), QVector3D( 3, 0, 0 ), xNeg, xNeg, xNeg );
  tc << TriangleCoords( QVector3D( 3, 0, 0 ), QVector3D( 3, 2, 1 ), QVector3D( 3, 2, 0 ), xNeg, xNeg, xNeg );

  tc << TriangleCoords( QVector3D( 3, 0, 1 ), QVector3D( 0, 0, 1 ), QVector3D( 3, 0, 0 ), yNeg, yNeg, yNeg );
  tc << TriangleCoords( QVector3D( 3, 0, 0 ), QVector3D( 0, 0, 1 ), QVector3D( 0, 0, 0 ), yNeg, yNeg, yNeg );
  tc << TriangleCoords( QVector3D( 0, 0, 1 ), QVector3D( 3, 0, 1 ), QVector3D( 0, 0, 0 ), yPos, yPos, yPos );
  tc << TriangleCoords( QVector3D( 0, 0, 0 ), QVector3D( 3, 0, 1 ), QVector3D( 3, 0, 0 ), yPos, yPos, yPos );

  QgsTessellator t;
  t.setAddNormals( true );
  t.setBackFacesEnabled( true );
  t.setExtrusionFaces( Qgis::ExtrusionFace::Walls | Qgis::ExtrusionFace::Roof | Qgis::ExtrusionFace::Floor );
  t.addPolygon( rect, 1 );
  QVERIFY( checkTriangleOutput( extractTriangles( t, true ), tc ) );
}


void TestQgsTessellator::test2DTriangle()
{
  QgsPolygon polygon;
  polygon.fromWkt( "POLYGON((1 1, 2 1, 1 2, 1 1))" );

  const QVector3D up( 0, 0, 1 );    // surface normal pointing straight up
  const QVector3D left( -1, 0, 0 ); // surface normal pointing straight down for back faces
  const QVector3D ne( 0.707107, 0.707107, 0 );
  const QVector3D bt( 0, -1, 0 ); // surface normal pointing straight down for back faces

  {
    // NO extrusion
    QList<TriangleCoords> trianglesNormalsCD;
    trianglesNormalsCD << TriangleCoords( QVector3D( 1, 1, 0 ), QVector3D( 2, 1, 0 ), QVector3D( 1, 2, 0 ), up, up, up );

    QgsTessellator tessellatorNormalsCD;
    tessellatorNormalsCD.setAddNormals( true );
    tessellatorNormalsCD.addPolygon( polygon, 0 );
    QVERIFY( checkTriangleOutput( extractTriangles( tessellatorNormalsCD, true ), trianglesNormalsCD ) );

    QCOMPARE( tessellatorNormalsCD.zMinimum(), 0 );
    QCOMPARE( tessellatorNormalsCD.zMaximum(), 0 );

    QList<TriangleCoords> trianglesNormalsEarcut;
    trianglesNormalsEarcut << TriangleCoords( QVector3D( 1, 1, 0 ), QVector3D( 2, 1, 0 ), QVector3D( 1, 2, 0 ), up, up, up );

    QgsTessellator tessellatorNormalsEarcut;
    tessellatorNormalsEarcut.setAddNormals( true );
    tessellatorNormalsEarcut.setTriangulationAlgorithm( Qgis::TriangulationAlgorithm::Earcut );
    tessellatorNormalsEarcut.addPolygon( polygon, 0 );
    QVERIFY( checkTriangleOutput( extractTriangles( tessellatorNormalsEarcut, true ), trianglesNormalsEarcut ) );

    QCOMPARE( tessellatorNormalsEarcut.zMinimum(), 0 );
    QCOMPARE( tessellatorNormalsEarcut.zMaximum(), 0 );
  }

  {
    // WITH extrusion
    QList<TriangleCoords> trianglesNormalsCD;
    trianglesNormalsCD << TriangleCoords( QVector3D( 1, 1, 7 ), QVector3D( 2, 1, 7 ), QVector3D( 1, 2, 7 ), up, up, up );

    trianglesNormalsCD << TriangleCoords( QVector3D( 1, 1, 7 ), QVector3D( 1, 2, 7 ), QVector3D( 1, 1, 0 ), left, left, left );
    trianglesNormalsCD << TriangleCoords( QVector3D( 1, 1, 0 ), QVector3D( 1, 2, 7 ), QVector3D( 1, 2, 0 ), left, left, left );
    trianglesNormalsCD << TriangleCoords( QVector3D( 1, 2, 7 ), QVector3D( 2, 1, 7 ), QVector3D( 1, 2, 0 ), ne, ne, ne );
    trianglesNormalsCD << TriangleCoords( QVector3D( 1, 2, 0 ), QVector3D( 2, 1, 7 ), QVector3D( 2, 1, 0 ), ne, ne, ne );

    trianglesNormalsCD << TriangleCoords( QVector3D( 2, 1, 7 ), QVector3D( 1, 1, 7 ), QVector3D( 2, 1, 0 ), bt, bt, bt );
    trianglesNormalsCD << TriangleCoords( QVector3D( 2, 1, 0 ), QVector3D( 1, 1, 7 ), QVector3D( 1, 1, 0 ), bt, bt, bt );

    QList<TriangleCoords> trianglesNormalsEarcut;
    trianglesNormalsEarcut << TriangleCoords( QVector3D( 1, 1, 7 ), QVector3D( 2, 1, 7 ), QVector3D( 1, 2, 7 ), up, up, up );

    trianglesNormalsEarcut << TriangleCoords( QVector3D( 1, 1, 7 ), QVector3D( 1, 2, 7 ), QVector3D( 1, 1, 0 ), left, left, left );
    trianglesNormalsEarcut << TriangleCoords( QVector3D( 1, 1, 0 ), QVector3D( 1, 2, 7 ), QVector3D( 1, 2, 0 ), left, left, left );

    trianglesNormalsEarcut << TriangleCoords( QVector3D( 1, 2, 7 ), QVector3D( 2, 1, 7 ), QVector3D( 1, 2, 0 ), ne, ne, ne );
    trianglesNormalsEarcut << TriangleCoords( QVector3D( 1, 2, 0 ), QVector3D( 2, 1, 7 ), QVector3D( 2, 1, 0 ), ne, ne, ne );

    trianglesNormalsEarcut << TriangleCoords( QVector3D( 2, 1, 7 ), QVector3D( 1, 1, 7 ), QVector3D( 2, 1, 0 ), bt, bt, bt );
    trianglesNormalsEarcut << TriangleCoords( QVector3D( 2, 1, 0 ), QVector3D( 1, 1, 7 ), QVector3D( 1, 1, 0 ), bt, bt, bt );

    QgsTessellator tesNormalsCD;
    tesNormalsCD.setAddNormals( true );
    tesNormalsCD.addPolygon( polygon, 7 );
    QVERIFY( checkTriangleOutput( extractTriangles( tesNormalsCD, true ), trianglesNormalsCD ) );

    QCOMPARE( tesNormalsCD.zMinimum(), 0 );
    QCOMPARE( tesNormalsCD.zMaximum(), 7 );
  }
}

void TestQgsTessellator::test3DTriangle()
{
  QgsPolygon polygon;
  polygon.fromWkt( "POLYGONZ((1 1 5, 2 1 5, 1 2 5, 1 1 5))" );

  const QVector3D up( 0, 0, 1 );    // surface normal pointing straight up
  const QVector3D left( -1, 0, 0 ); // surface normal pointing straight down for back faces
  const QVector3D ne( 0.707107, 0.707107, 0 );
  const QVector3D bt( 0, -1, 0 ); // surface normal pointing straight down for back faces

  {
    // NO extrusion
    QList<TriangleCoords> tcNormals;
    tcNormals << TriangleCoords( QVector3D( 1, 1, 5 ), QVector3D( 2, 1, 5 ), QVector3D( 1, 2, 5 ), up, up, up );

    QgsTessellator tN;
    tN.setAddNormals( true );
    tN.addPolygon( polygon, 0 );
    QVERIFY( checkTriangleOutput( extractTriangles( tN, true ), tcNormals ) );

    QCOMPARE( tN.zMinimum(), 5 );
    QCOMPARE( tN.zMaximum(), 5 );
  }

  {
    // WITH extrusion
    QList<TriangleCoords> trianglesNormals;
    trianglesNormals << TriangleCoords( QVector3D( 1, 1, 5 + 7 ), QVector3D( 2, 1, 5 + 7 ), QVector3D( 1, 2, 5 + 7 ), up, up, up );

    trianglesNormals << TriangleCoords( QVector3D( 1, 1, 5 + 7 ), QVector3D( 1, 2, 5 + 7 ), QVector3D( 1, 1, 5 ), left, left, left );
    trianglesNormals << TriangleCoords( QVector3D( 1, 1, 5 ), QVector3D( 1, 2, 5 + 7 ), QVector3D( 1, 2, 5 ), left, left, left );

    trianglesNormals << TriangleCoords( QVector3D( 1, 2, 5 + 7 ), QVector3D( 2, 1, 5 + 7 ), QVector3D( 1, 2, 5 ), ne, ne, ne );
    trianglesNormals << TriangleCoords( QVector3D( 1, 2, 5 ), QVector3D( 2, 1, 5 + 7 ), QVector3D( 2, 1, 5 ), ne, ne, ne );

    trianglesNormals << TriangleCoords( QVector3D( 2, 1, 5 + 7 ), QVector3D( 1, 1, 5 + 7 ), QVector3D( 2, 1, 5 ), bt, bt, bt );
    trianglesNormals << TriangleCoords( QVector3D( 2, 1, 5 ), QVector3D( 1, 1, 5 + 7 ), QVector3D( 1, 1, 5 ), bt, bt, bt );

    QList<TriangleCoords> trianglesNormalsEarcut;
    trianglesNormalsEarcut << TriangleCoords( QVector3D( 1, 1, 5 + 7 ), QVector3D( 2, 1, 5 + 7 ), QVector3D( 1, 2, 5 + 7 ), up, up, up );

    trianglesNormalsEarcut << TriangleCoords( QVector3D( 1, 1, 5 + 7 ), QVector3D( 1, 2, 5 + 7 ), QVector3D( 1, 1, 5 ), left, left, left );
    trianglesNormalsEarcut << TriangleCoords( QVector3D( 1, 1, 5 ), QVector3D( 1, 2, 5 + 7 ), QVector3D( 1, 2, 5 ), left, left, left );

    trianglesNormalsEarcut << TriangleCoords( QVector3D( 1, 2, 5 + 7 ), QVector3D( 2, 1, 5 + 7 ), QVector3D( 1, 2, 5 ), ne, ne, ne );
    trianglesNormalsEarcut << TriangleCoords( QVector3D( 1, 2, 5 ), QVector3D( 2, 1, 5 + 7 ), QVector3D( 2, 1, 5 ), ne, ne, ne );

    trianglesNormalsEarcut << TriangleCoords( QVector3D( 2, 1, 5 + 7 ), QVector3D( 1, 1, 5 + 7 ), QVector3D( 2, 1, 5 ), bt, bt, bt );
    trianglesNormalsEarcut << TriangleCoords( QVector3D( 2, 1, 5 ), QVector3D( 1, 1, 5 + 7 ), QVector3D( 1, 1, 5 ), bt, bt, bt );

    QgsTessellator tesNormalsEarcut;
    tesNormalsEarcut.setAddNormals( true );
    tesNormalsEarcut.setTriangulationAlgorithm( Qgis::TriangulationAlgorithm::Earcut );
    tesNormalsEarcut.addPolygon( polygon, 7 );
    QVERIFY( checkTriangleOutput( extractTriangles( tesNormalsEarcut, true ), trianglesNormalsEarcut ) );

    QCOMPARE( tesNormalsEarcut.zMinimum(), 5 );
    QCOMPARE( tesNormalsEarcut.zMaximum(), 5 + 7 );
  }
}

void TestQgsTessellator::asMultiPolygon()
{
  QgsPolygon polygon;
  polygon.fromWkt( "POLYGON((1 1, 2 1, 3 2, 1 2, 1 1))" );

  QgsPolygon polygonZ;
  polygonZ.fromWkt( "POLYGONZ((1 1 1, 2 1 2, 3 2 3, 1 2 4, 1 1 1))" );

  QgsTessellator t;
  t.addPolygon( polygon, 0 );
  QCOMPARE( t.asMultiPolygon()->asWkt(), u"MultiPolygon Z (((1 2 0, 2 1 0, 3 2 0, 1 2 0)),((1 2 0, 1 1 0, 2 1 0, 1 2 0)))"_s );

  QgsTessellator t2;
  t2.addPolygon( polygonZ, 0 );
  QCOMPARE( t2.asMultiPolygon()->asWkt( 6 ), u"MultiPolygon Z (((1 2 4, 2 1 2, 3 2 3, 1 2 4)),((1 2 4, 1 1 1, 2 1 2, 1 2 4)))"_s );
}

void TestQgsTessellator::testBadCoordinates()
{
  // check with a vertical "wall" polygon - if the Z coordinates are ignored,
  // the polygon may be incorrectly considered as having close/repeated coordinates
  QgsPolygon polygonZ;
  polygonZ.fromWkt( "POLYGONZ((1 2 1, 2 1 1, 2 1 2, 1 2 2, 1 2 1))" );

  QList<TriangleCoords> trianglesZCD;
  trianglesZCD << TriangleCoords( QVector3D( 1, 2, 2 ), QVector3D( 2, 1, 1 ), QVector3D( 2, 1, 2 ) );
  trianglesZCD << TriangleCoords( QVector3D( 1, 2, 2 ), QVector3D( 1, 2, 1 ), QVector3D( 2, 1, 1 ) );

  QList<TriangleCoords> trianglesZEarcut;
  trianglesZEarcut << TriangleCoords( QVector3D( 2, 1, 2 ), QVector3D( 1, 2, 2 ), QVector3D( 1, 2, 1 ) );
  trianglesZEarcut << TriangleCoords( QVector3D( 1, 2, 1 ), QVector3D( 2, 1, 1 ), QVector3D( 2, 1, 2 ) );

  QgsTessellator tesZCD;
  tesZCD.addPolygon( polygonZ, 0 );
  QVERIFY( checkTriangleOutput( extractTriangles( tesZCD, false ), trianglesZCD ) );

  QCOMPARE( tesZCD.zMinimum(), 1.0f );
  QCOMPARE( tesZCD.zMaximum(), 2.0f );

  QgsTessellator tesZEarcut;
  tesZEarcut.setTriangulationAlgorithm( Qgis::TriangulationAlgorithm::Earcut );
  tesZEarcut.addPolygon( polygonZ, 0 );

  QVERIFY( checkTriangleOutput( extractTriangles( tesZEarcut, false ), trianglesZEarcut ) );

  QCOMPARE( tesZEarcut.zMinimum(), 1.0f );
  QCOMPARE( tesZEarcut.zMaximum(), 2.0f );

  // triangulation would crash for me with this polygon if there is no simplification
  // to remove the coordinates that are very close to each other
  QgsPolygon polygon;
  polygon.fromWkt( "POLYGON((1 1, 2 1, 2.0000001 1.0000001, 2.0000002 1.0000001, 2.0000001 1.0000002, 2.0000002 1.0000002, 3 2, 1 2, 1 1))" );

  QList<TriangleCoords> trianglesCD;
  trianglesCD << TriangleCoords( QVector3D( 1, 2, 0 ), QVector3D( 2, 1, 0 ), QVector3D( 3, 2, 0 ) );
  trianglesCD << TriangleCoords( QVector3D( 1, 2, 0 ), QVector3D( 1, 1, 0 ), QVector3D( 2, 1, 0 ) );

  QList<TriangleCoords> trianglesEarcut;
  trianglesEarcut << TriangleCoords( QVector3D( 3, 2, 0 ), QVector3D( 1, 2, 0 ), QVector3D( 1, 1, 0 ) );
  trianglesEarcut << TriangleCoords( QVector3D( 1, 1, 0 ), QVector3D( 2, 1, 0 ), QVector3D( 3, 2, 0 ) );

  QgsTessellator tesCD;
  tesCD.addPolygon( polygon, 0 );
  QVERIFY( checkTriangleOutput( extractTriangles( tesCD, false ), trianglesCD ) );

  QCOMPARE( tesCD.zMinimum(), 0 );
  QCOMPARE( tesCD.zMaximum(), 0 );

  QgsTessellator tesEarcut;
  tesEarcut.setTriangulationAlgorithm( Qgis::TriangulationAlgorithm::Earcut );
  tesEarcut.addPolygon( polygon, 0 );
  QVERIFY( checkTriangleOutput( extractTriangles( tesEarcut, false ), trianglesEarcut ) );

  QCOMPARE( tesEarcut.zMinimum(), 0 );
  QCOMPARE( tesEarcut.zMaximum(), 0 );
}

void TestQgsTessellator::testIssue17745()
{
  // this is a rectangular polygon with collinear points that would crash poly2tri if coordinates do not get rounded a bit

  QgsTessellator t;
  t.setAddNormals( true );
  QgsPolygon p;
  const bool resWktRead = p.fromWkt( "Polygon((0 0, 1 1e-15, 4 0, 4 5, 1 5, 0 5, 0 0))" );
  QVERIFY( resWktRead );

  t.addPolygon( p, 0 ); // must not crash - that's all we test here

  // we also test earcut not crashing
  t.setTriangulationAlgorithm( Qgis::TriangulationAlgorithm::Earcut );
  t.addPolygon( p, 0 );
}

void TestQgsTessellator::testCrashSelfIntersection()
{
  // this is a polygon where we get self-intersecting exterior ring that would crash poly2tri if not skipped

  QgsTessellator t;
  t.setAddNormals( true );
  QgsPolygon p;
  const bool resWktRead = p.fromWkt(
    "PolygonZ ((-744809.80499999970197678 -1042371.96730000153183937 260.460968017578125, -744809.80299999937415123 -1042371.92199999839067459 260.460968017578125, -744810.21599999815225601 "
    "-1042381.09099999815225601 260.460968017578125, -744810.21499999985098839 -1042381.0689999982714653 260.460968017578125, -744812.96469999849796295 -1042375.32499999925494194 "
    "263.734283447265625, -744809.80499999970197678 -1042371.96730000153183937 260.460968017578125))"
  );

  QVERIFY( resWktRead );

  t.addPolygon( p, 0 ); // must not crash - that's all we test here

  // we also test earcut not crashing
  t.setTriangulationAlgorithm( Qgis::TriangulationAlgorithm::Earcut );
  t.addPolygon( p, 0 );
}

void TestQgsTessellator::testCrashEmptyPolygon()
{
  // this is a polygon that goes through GEOS simplification which throws an exception (and produces null geometry)

  QgsTessellator t;
  t.setAddNormals( true );
  QgsPolygon p;
  const bool resWktRead = p.fromWkt( "PolygonZ ((0 0 0, 0 0 0, 0 0 0))" );
  QVERIFY( resWktRead );

  t.addPolygon( p, 0 ); // must not crash - that's all we test here
}

void TestQgsTessellator::testBoundsScaling()
{
  QgsPolygon polygon;
  polygon.fromWkt( "POLYGON((1 1, 1.00000001 1, 1.00000001 1.00000001, 1 1.0000000001, 1 1))" );

  QList<TriangleCoords> tc;
  tc << TriangleCoords( QVector3D( 0, 1e-10f, 0 ), QVector3D( 1e-08f, 0, 0 ), QVector3D( 1e-08f, 1e-08f, 0 ), QVector3D( 0, 0, 1 ), QVector3D( 0, 0, 1 ), QVector3D( 0, 0, 1 ) );
  tc << TriangleCoords( QVector3D( 0, 1e-10f, 0 ), QVector3D( 0, 0, 0 ), QVector3D( 1e-08f, 0, 0 ), QVector3D( 0, 0, 1 ), QVector3D( 0, 0, 1 ), QVector3D( 0, 0, 1 ) );

  // without using bounds -- numerically unstable, expect no result
  QgsTessellator t;
  t.setAddNormals( true );
  t.addPolygon( polygon, 0 );
  QCOMPARE( t.uniqueVertexCount(), 0 );

  // using bounds scaling, expect good result
  QgsTessellator t2;
  t2.setBounds( polygon.boundingBox() );
  t2.setAddNormals( true );
  t2.addPolygon( polygon, 0 );
  QVERIFY( checkTriangleOutput( extractTriangles( t2, true ), tc ) );
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

  QgsTessellator t;
  t.setBounds( polygonZ.boundingBox() );
  t.setInputZValueIgnored( true );
  t.addPolygon( polygonZ, 0 );
  QVERIFY( checkTriangleOutput( extractTriangles( t, false ), tc ) );
  QCOMPARE( t.zMinimum(), 0 );
  QCOMPARE( t.zMaximum(), 0 );
}

void TestQgsTessellator::testTriangulationDoesNotCrash()
{
  // a commit in poly2tri has caused a crashing regression - https://github.com/jhasse/poly2tri/issues/11
  // this code only makes sure that the crash does not come back during another update of poly2tri
  QgsPolygon polygon;
  polygon.fromWkt( "Polygon((0 0, -5 -3e-10, -10 -2e-10, -10 -4, 0 -4))" );
  QgsTessellator t;
  t.setAddNormals( true );
  t.addPolygon( polygon, 0 );

  // let's also test earcut here
  t.setTriangulationAlgorithm( Qgis::TriangulationAlgorithm::Earcut );
  t.addPolygon( polygon, 0 );
}

void TestQgsTessellator::testCrash2DTriangle()
{
  // test tessellation of a 2D triangle - https://github.com/qgis/QGIS/issues/36024
  QgsPolygon polygon;
  polygon.fromWkt( "Polygon((0 0, 42 0, 42 42, 0 0))" );
  // must not be declared with mNoz = true
  QgsTessellator t;
  t.setAddNormals( true );
  t.addPolygon( polygon, 0 ); // must not crash - that's all we test here

  // let's also test earcut here
  t.setTriangulationAlgorithm( Qgis::TriangulationAlgorithm::Earcut );
  t.addPolygon( polygon, 0 );
}

void TestQgsTessellator::narrowPolygon()
{
  // test that tessellation of a long narrow polygon results in a "nice" tessellation (i.e. not lots of long thin triangles)
  // refs https://github.com/qgis/QGIS/issues/37077
  QgsPolygon polygon;
  polygon.fromWkt(
    "Polygon ((383393.53728186257649213 4902093.79335568379610777, 383383.73728171654511243 4902092.99335567187517881, 383375.25399118528002873 4902092.8891992112621665, 383368.08741026872303337 "
    "4902093.48088630195707083, 383362.87084332120139152 4902093.91129046399146318, 383359.60429034277331084 4902094.18041169829666615, 383357.23274148383643478 4902093.6530067715793848, "
    "383355.75619674433255568 4902092.32907568290829659, 383355.57501344417687505 4902090.56084021460264921, 383356.68919158342760056 4902088.34830036386847496, 383361.07830215193098411 "
    "4902086.48156050778925419, 383368.74234514962881804 4902084.96062064450234175, 383380.44909519288921729 4902084.10479906108230352, 383396.19855228182859719 4902083.91409575659781694, "
    "383406.97328086948255077 4902084.21874411031603813, 383412.77328095590928569 4902085.01874412223696709, 383416.70496230950811878 4902086.31405469868332148, 383418.76832493022084236 "
    "4902088.1046758396551013, 383419.69647055567475036 4902089.60464367642998695, 383419.48939918575342745 4902090.81395820714533329, 383418.40130056580528617 4902092.01381655503064394, "
    "383416.43217469577211887 4902093.20421871729195118, 383411.19502930447924882 4902093.89790377207100391, 383402.68986439192667603 4902094.09487171657383442, 383393.53728186257649213 "
    "4902093.79335568379610777))"
  );
  QgsTessellator t;
  t.setBounds( polygon.boundingBox() );
  t.addPolygon( polygon, 0 );
  QgsGeometry res( t.asMultiPolygon() );
  res.translate( polygon.boundingBox().xMinimum(), polygon.boundingBox().yMinimum() );
  QgsDebugMsgLevel( res.asWkt( 0 ), 1 );
  QCOMPARE(
    res.asWkt( 0 ),
    u"MultiPolygon Z (((383357 4902094 0, 383356 4902092 0, 383356 4902091 0, 383357 4902094 0)),((383357 4902088 0, 383357 4902094 0, 383356 4902091 0, 383357 4902088 0)),((383357 4902088 0, 383361 4902086 0, 383357 4902094 0, 383357 4902088 0)),((383357 4902094 0, 383361 4902086 0, 383360 4902094 0, 383357 4902094 0)),((383363 4902094 0, 383360 4902094 0, 383361 4902086 0, 383363 4902094 0)),((383368 4902093 0, 383363 4902094 0, 383361 4902086 0, 383368 4902093 0)),((383368 4902093 0, 383361 4902086 0, 383369 4902085 0, 383368 4902093 0)),((383368 4902093 0, 383369 4902085 0, 383375 4902093 0, 383368 4902093 0)),((383375 4902093 0, 383369 4902085 0, 383380 4902084 0, 383375 4902093 0)),((383375 4902093 0, 383380 4902084 0, 383384 4902093 0, 383375 4902093 0)),((383384 4902093 0, 383380 4902084 0, 383396 4902084 0, 383384 4902093 0)),((383394 4902094 0, 383384 4902093 0, 383396 4902084 0, 383394 4902094 0)),((383403 4902094 0, 383394 4902094 0, 383396 4902084 0, 383403 4902094 0)),((383396 4902084 0, 383407 4902084 0, 383403 4902094 0, 383396 4902084 0)),((383411 4902094 0, 383403 4902094 0, 383407 4902084 0, 383411 4902094 0)),((383411 4902094 0, 383407 4902084 0, 383413 4902085 0, 383411 4902094 0)),((383411 4902094 0, 383413 4902085 0, 383416 4902093 0, 383411 4902094 0)),((383416 4902093 0, 383413 4902085 0, 383417 4902086 0, 383416 4902093 0)),((383419 4902088 0, 383416 4902093 0, 383417 4902086 0, 383419 4902088 0)),((383418 4902092 0, 383416 4902093 0, 383419 4902088 0, 383418 4902092 0)),((383418 4902092 0, 383419 4902088 0, 383419 4902091 0, 383418 4902092 0)),((383419 4902091 0, 383419 4902088 0, 383420 4902090 0, 383419 4902091 0)))"_s
  );
}

void TestQgsTessellator::testOutputZUp()
{
  QgsPolygon polygon;
  polygon.fromWkt( "POLYGON((1 1, 2 1, 3 2, 1 2, 1 1))" );

  QgsTessellator tZUp;
  tZUp.setAddNormals( true );
  tZUp.addPolygon( polygon, 0 );

  QList<TriangleCoords> expectedOutputZUp;
  expectedOutputZUp << TriangleCoords( QVector3D( 1, 2, 0 ), QVector3D( 2, 1, 0 ), QVector3D( 3, 2, 0 ), QVector3D( 0, 0, 1 ), QVector3D( 0, 0, 1 ), QVector3D( 0, 0, 1 ) );
  expectedOutputZUp << TriangleCoords( QVector3D( 1, 2, 0 ), QVector3D( 1, 1, 0 ), QVector3D( 2, 1, 0 ), QVector3D( 0, 0, 1 ), QVector3D( 0, 0, 1 ), QVector3D( 0, 0, 1 ) );

  QVERIFY( checkTriangleOutput( extractTriangles( tZUp, true ), expectedOutputZUp ) );
}

void TestQgsTessellator::testDuplicatePoints()
{
  QgsPolygon polygonZ;
  polygonZ.fromWkt( "POLYGONZ((1 1 3, 1 1 3, 2 1 3, 2 1 3, 3 2 3, 1 2 3, 1 1 3))" );

  QList<TriangleCoords> trianglesZCD;
  trianglesZCD << TriangleCoords( QVector3D( 1, 2, 3 ), QVector3D( 2, 1, 3 ), QVector3D( 3, 2, 3 ) );
  trianglesZCD << TriangleCoords( QVector3D( 1, 2, 3 ), QVector3D( 1, 1, 3 ), QVector3D( 2, 1, 3 ) );

  QList<TriangleCoords> trianglesZEarcut;
  trianglesZEarcut << TriangleCoords( QVector3D( 3, 2, 3 ), QVector3D( 1, 2, 3 ), QVector3D( 1, 1, 3 ) );
  trianglesZEarcut << TriangleCoords( QVector3D( 1, 1, 3 ), QVector3D( 2, 1, 3 ), QVector3D( 3, 2, 3 ) );

  QgsTessellator tesZCD;
  tesZCD.addPolygon( polygonZ, 0 );
  QVERIFY( checkTriangleOutput( extractTriangles( tesZCD, false ), trianglesZCD ) );

  QgsTessellator tesZEarcut;
  tesZEarcut.setTriangulationAlgorithm( Qgis::TriangulationAlgorithm::Earcut );
  tesZEarcut.addPolygon( polygonZ, 0 );
  QVERIFY( checkTriangleOutput( extractTriangles( tesZEarcut, false ), trianglesZEarcut ) );
}

void TestQgsTessellator::testRoofs()
{
  QgsPolygon polygon;
  polygon.fromWkt( "POLYGON((1 1, 2 1, 3 2, 1 2, 1 1))" );

  QgsPolygon polygonZflat;
  polygonZflat.fromWkt( "POLYGONZ((1 1 3, 2 1 3, 3 2 3, 1 2 3, 1 1 3))" );

  // this is a more complicated polygon with Z coordinates where the "roof" is not in one plane
  QgsPolygon polygonZnonFlat;
  polygonZnonFlat.fromWkt( "POLYGONZ((1 1 1, 2 1 2, 3 2 3, 1 2 4, 1 1 1))" );

  {
    // polygon CD
    QList<TriangleCoords> tc;
    tc << TriangleCoords( QVector3D( 1, 2, 10 ), QVector3D( 1, 1, 10 ), QVector3D( 2, 1, 10 ) );
    tc << TriangleCoords( QVector3D( 1, 2, 10 ), QVector3D( 2, 1, 10 ), QVector3D( 3, 2, 10 ) );

    QgsTessellator t;
    t.setExtrusionFaces( Qgis::ExtrusionFace::Roof );
    t.setTriangulationAlgorithm( Qgis::TriangulationAlgorithm::ConstrainedDelaunay );
    t.addPolygon( polygon, 10 );

    QVERIFY( checkTriangleOutput( extractTriangles( t, false ), tc ) );
  }

  {
    // polygon Earcut
    QList<TriangleCoords> tc;
    tc << TriangleCoords( QVector3D( 1, 1, 10 ), QVector3D( 2, 1, 10 ), QVector3D( 3, 2, 10 ) );
    tc << TriangleCoords( QVector3D( 3, 2, 10 ), QVector3D( 1, 2, 10 ), QVector3D( 1, 1, 10 ) );

    QgsTessellator t;
    t.setExtrusionFaces( Qgis::ExtrusionFace::Roof );
    t.setTriangulationAlgorithm( Qgis::TriangulationAlgorithm::Earcut );
    t.addPolygon( polygon, 10 );

    QVERIFY( checkTriangleOutput( extractTriangles( t, false ), tc ) );
  }

  {
    // flat polygon Z CD
    QList<TriangleCoords> tc;
    tc << TriangleCoords( QVector3D( 1, 2, 13 ), QVector3D( 1, 1, 13 ), QVector3D( 2, 1, 13 ) );
    tc << TriangleCoords( QVector3D( 1, 2, 13 ), QVector3D( 2, 1, 13 ), QVector3D( 3, 2, 13 ) );

    QgsTessellator t;
    t.setExtrusionFaces( Qgis::ExtrusionFace::Roof );
    t.setTriangulationAlgorithm( Qgis::TriangulationAlgorithm::ConstrainedDelaunay );
    t.addPolygon( polygonZflat, 10 );

    QVERIFY( checkTriangleOutput( extractTriangles( t, false ), tc ) );
  }

  {
    // flat polygon Z Earcut
    QList<TriangleCoords> tc;
    tc << TriangleCoords( QVector3D( 1, 1, 13 ), QVector3D( 2, 1, 13 ), QVector3D( 3, 2, 13 ) );
    tc << TriangleCoords( QVector3D( 3, 2, 13 ), QVector3D( 1, 2, 13 ), QVector3D( 1, 1, 13 ) );

    QgsTessellator t;
    t.setExtrusionFaces( Qgis::ExtrusionFace::Roof );
    t.setTriangulationAlgorithm( Qgis::TriangulationAlgorithm::Earcut );
    t.addPolygon( polygonZflat, 10 );

    QVERIFY( checkTriangleOutput( extractTriangles( t, false ), tc ) );
  }

  {
    // polygon Z CD
    QList<TriangleCoords> tc;
    tc << TriangleCoords( QVector3D( 1, 2, 14 ), QVector3D( 2, 1, 12 ), QVector3D( 3, 2, 13 ) );
    tc << TriangleCoords( QVector3D( 1, 2, 14 ), QVector3D( 1, 1, 11 ), QVector3D( 2, 1, 12 ) );

    QgsTessellator t;
    t.setExtrusionFaces( Qgis::ExtrusionFace::Roof );
    t.setTriangulationAlgorithm( Qgis::TriangulationAlgorithm::ConstrainedDelaunay );
    t.addPolygon( polygonZnonFlat, 10 );

    QVERIFY( checkTriangleOutput( extractTriangles( t, false ), tc ) );
  }

  {
    // polygon Z Earcut
    QList<TriangleCoords> tc;
    tc << TriangleCoords( QVector3D( 3, 2, 13 ), QVector3D( 1, 2, 14 ), QVector3D( 1, 1, 11 ) );
    tc << TriangleCoords( QVector3D( 1, 1, 11 ), QVector3D( 2, 1, 12 ), QVector3D( 3, 2, 13 ) );

    QgsTessellator t;
    t.setExtrusionFaces( Qgis::ExtrusionFace::Roof );
    t.setTriangulationAlgorithm( Qgis::TriangulationAlgorithm::Earcut );
    t.addPolygon( polygonZnonFlat, 10 );

    QVERIFY( checkTriangleOutput( extractTriangles( t, false ), tc ) );
  }
}

void TestQgsTessellator::testFloorsAndRoofs()
{
  QgsPolygon polygon;
  polygon.fromWkt( "POLYGON((1 1, 2 1, 3 2, 1 2, 1 1))" );

  QgsPolygon polygonZflat;
  polygonZflat.fromWkt( "POLYGONZ((1 1 3, 2 1 3, 3 2 3, 1 2 3, 1 1 3))" );

  // this is a more complicated polygon with Z coordinates where the "roof" is not in one plane
  QgsPolygon polygonZnonFlat;
  polygonZnonFlat.fromWkt( "POLYGONZ((1 1 1, 2 1 2, 3 2 3, 1 2 4, 1 1 1))" );

  {
    // polygon CD
    QList<TriangleCoords> tc;
    tc << TriangleCoords( QVector3D( 1, 2, 0 ), QVector3D( 1, 1, 0 ), QVector3D( 2, 1, 0 ) );
    tc << TriangleCoords( QVector3D( 1, 2, 0 ), QVector3D( 2, 1, 0 ), QVector3D( 3, 2, 0 ) );
    tc << TriangleCoords( QVector3D( 1, 2, 10 ), QVector3D( 1, 1, 10 ), QVector3D( 2, 1, 10 ) );
    tc << TriangleCoords( QVector3D( 1, 2, 10 ), QVector3D( 2, 1, 10 ), QVector3D( 3, 2, 10 ) );

    QgsTessellator t;
    t.setExtrusionFaces( Qgis::ExtrusionFace::Floor | Qgis::ExtrusionFace::Roof );
    t.setTriangulationAlgorithm( Qgis::TriangulationAlgorithm::ConstrainedDelaunay );
    t.addPolygon( polygon, 10 );

    QVERIFY( checkTriangleOutput( extractTriangles( t, false ), tc ) );
  }

  {
    // polygon Earcut
    QList<TriangleCoords> tc;
    tc << TriangleCoords( QVector3D( 1, 1, 0 ), QVector3D( 2, 1, 0 ), QVector3D( 3, 2, 0 ) );
    tc << TriangleCoords( QVector3D( 3, 2, 0 ), QVector3D( 1, 2, 0 ), QVector3D( 1, 1, 0 ) );
    tc << TriangleCoords( QVector3D( 1, 1, 10 ), QVector3D( 2, 1, 10 ), QVector3D( 3, 2, 10 ) );
    tc << TriangleCoords( QVector3D( 3, 2, 10 ), QVector3D( 1, 2, 10 ), QVector3D( 1, 1, 10 ) );

    QgsTessellator t;
    t.setExtrusionFaces( Qgis::ExtrusionFace::Floor | Qgis::ExtrusionFace::Roof );
    t.setTriangulationAlgorithm( Qgis::TriangulationAlgorithm::Earcut );
    t.addPolygon( polygon, 10 );

    QVERIFY( checkTriangleOutput( extractTriangles( t, false ), tc ) );
  }

  {
    // flat polygon Z CD
    QList<TriangleCoords> tc;
    tc << TriangleCoords( QVector3D( 1, 2, 3 ), QVector3D( 1, 1, 3 ), QVector3D( 2, 1, 3 ) );
    tc << TriangleCoords( QVector3D( 1, 2, 3 ), QVector3D( 2, 1, 3 ), QVector3D( 3, 2, 3 ) );
    tc << TriangleCoords( QVector3D( 1, 2, 13 ), QVector3D( 1, 1, 13 ), QVector3D( 2, 1, 13 ) );
    tc << TriangleCoords( QVector3D( 1, 2, 13 ), QVector3D( 2, 1, 13 ), QVector3D( 3, 2, 13 ) );

    QgsTessellator t;
    t.setExtrusionFaces( Qgis::ExtrusionFace::Floor | Qgis::ExtrusionFace::Roof );
    t.setTriangulationAlgorithm( Qgis::TriangulationAlgorithm::ConstrainedDelaunay );
    t.addPolygon( polygonZflat, 10 );

    QVERIFY( checkTriangleOutput( extractTriangles( t, false ), tc ) );
  }

  {
    // flat polygon Z Earcut
    QList<TriangleCoords> tc;
    tc << TriangleCoords( QVector3D( 1, 1, 3 ), QVector3D( 2, 1, 3 ), QVector3D( 3, 2, 3 ) );
    tc << TriangleCoords( QVector3D( 3, 2, 3 ), QVector3D( 1, 2, 3 ), QVector3D( 1, 1, 3 ) );
    tc << TriangleCoords( QVector3D( 1, 1, 13 ), QVector3D( 2, 1, 13 ), QVector3D( 3, 2, 13 ) );
    tc << TriangleCoords( QVector3D( 3, 2, 13 ), QVector3D( 1, 2, 13 ), QVector3D( 1, 1, 13 ) );

    QgsTessellator t;
    t.setExtrusionFaces( Qgis::ExtrusionFace::Floor | Qgis::ExtrusionFace::Roof );
    t.setTriangulationAlgorithm( Qgis::TriangulationAlgorithm::Earcut );
    t.addPolygon( polygonZflat, 10 );

    QVERIFY( checkTriangleOutput( extractTriangles( t, false ), tc ) );
  }

  {
    // polygon Z CD
    QList<TriangleCoords> tc;
    tc << TriangleCoords( QVector3D( 1, 2, 4 ), QVector3D( 2, 1, 2 ), QVector3D( 3, 2, 3 ) );
    tc << TriangleCoords( QVector3D( 1, 2, 4 ), QVector3D( 1, 1, 1 ), QVector3D( 2, 1, 2 ) );
    tc << TriangleCoords( QVector3D( 1, 2, 14 ), QVector3D( 2, 1, 12 ), QVector3D( 3, 2, 13 ) );
    tc << TriangleCoords( QVector3D( 1, 2, 14 ), QVector3D( 1, 1, 11 ), QVector3D( 2, 1, 12 ) );

    QgsTessellator t;
    t.setExtrusionFaces( Qgis::ExtrusionFace::Floor | Qgis::ExtrusionFace::Roof );
    t.setTriangulationAlgorithm( Qgis::TriangulationAlgorithm::ConstrainedDelaunay );
    t.addPolygon( polygonZnonFlat, 10 );

    QVERIFY( checkTriangleOutput( extractTriangles( t, false ), tc ) );
  }

  {
    // polygon Z Earcut
    QList<TriangleCoords> tc;
    tc << TriangleCoords( QVector3D( 3, 2, 3 ), QVector3D( 1, 2, 4 ), QVector3D( 1, 1, 1 ) );
    tc << TriangleCoords( QVector3D( 1, 1, 1 ), QVector3D( 2, 1, 2 ), QVector3D( 3, 2, 3 ) );
    tc << TriangleCoords( QVector3D( 3, 2, 13 ), QVector3D( 1, 2, 14 ), QVector3D( 1, 1, 11 ) );
    tc << TriangleCoords( QVector3D( 1, 1, 11 ), QVector3D( 2, 1, 12 ), QVector3D( 3, 2, 13 ) );

    QgsTessellator t;
    t.setExtrusionFaces( Qgis::ExtrusionFace::Floor | Qgis::ExtrusionFace::Roof );
    t.setTriangulationAlgorithm( Qgis::TriangulationAlgorithm::Earcut );
    t.addPolygon( polygonZnonFlat, 10 );

    QVERIFY( checkTriangleOutput( extractTriangles( t, false ), tc ) );
  }
}

QGSTEST_MAIN( TestQgsTessellator )
#include "testqgstessellator.moc"
