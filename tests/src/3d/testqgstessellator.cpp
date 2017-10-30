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
    return pts[0] == other.pts[0] && pts[1] == other.pts[1] && pts[2] == other.pts[2] &&
           normals[0] == other.normals[0] && normals[1] == other.normals[1] && normals[2] == other.normals[2];
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
    return false;

  // TODO: allow arbitrary order of triangles in output
  const float *dataRaw = data.constData();
  for ( int i = 0; i < expected.count(); ++i )
  {
    const TriangleCoords &exp = expected.at( i );
    TriangleCoords out( dataRaw, withNormals );
    if ( exp != out )
    {
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
 * This is a unit test for the node tool
 */
class TestQgsTessellator : public QObject
{
    Q_OBJECT
  public:
    TestQgsTessellator();

  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.

    void testBasic();

  private:
};

TestQgsTessellator::TestQgsTessellator() = default;


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


QGSTEST_MAIN( TestQgsTessellator )
#include "testqgstessellator.moc"
