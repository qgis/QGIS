/***************************************************************************
                         qgstriangularmesh.cpp
                         ---------------------
    begin                : April 2018
    copyright            : (C) 2018 by Peter Petrik
    email                : zilolv at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <memory>
#include <QList>
#include "qgspolygon.h"
#include "qgslinestring.h"
#include "qgstriangularmesh.h"
#include "qgsrendercontext.h"
#include "qgscoordinatetransform.h"
#include "qgsgeometry.h"
#include "qgsrectangle.h"
#include "qgslogger.h"
#include "qgsmeshspatialindex.h"

static void ENP_centroid_step( const QPolygonF &pX, double &cx, double &cy, double &signedArea, int i, int i1 )
{
  double x0 = 0.0; // Current vertex X
  double y0 = 0.0; // Current vertex Y
  double x1 = 0.0; // Next vertex X
  double y1 = 0.0; // Next vertex Y
  double a = 0.0;  // Partial signed area

  x0 = pX[i].x();
  y0 = pX[i].y();
  x1 = pX[i1].x();
  y1 = pX[i1].y();
  a = x0 * y1 - x1 * y0;
  signedArea += a;
  cx += ( x0 + x1 ) * a;
  cy += ( y0 + y1 ) * a;
}

static void ENP_centroid( const QPolygonF &pX, double &cx, double &cy )
{
  // http://stackoverflow.com/questions/2792443/finding-the-centroid-of-a-polygon/2792459#2792459
  cx = 0;
  cy = 0;

  if ( pX.isEmpty() )
    return;

  double signedArea = 0.0;

  // For all vertices except last
  int i = 0;
  for ( ; i < pX.size() - 1; ++i )
  {
    ENP_centroid_step( pX, cx, cy, signedArea, i, i + 1 );
  }
  // Do last vertex separately to avoid performing an expensive
  // modulus operation in each iteration.
  ENP_centroid_step( pX, cx, cy, signedArea, i, 0 );

  signedArea *= 0.5;
  cx /= ( 6.0 * signedArea );
  cy /= ( 6.0 * signedArea );
}


void QgsTriangularMesh::triangulate( const QgsMeshFace &face, int nativeIndex )
{
  int vertexCount = face.size();
  if ( vertexCount < 3 )
    return;

  while ( vertexCount > 3 )
  {
    // clip one ear from last 2 and first vertex
    const QgsMeshFace ear = { face[vertexCount - 2], face[vertexCount - 1], face[0] };
    mTriangularMesh.faces.push_back( ear );
    mTrianglesToNativeFaces.push_back( nativeIndex );
    --vertexCount;
  }

  const QgsMeshFace triangle = { face[1], face[2], face[0] };
  mTriangularMesh.faces.push_back( triangle );
  mTrianglesToNativeFaces.push_back( nativeIndex );
}

QgsTriangularMesh::~QgsTriangularMesh() = default;
QgsTriangularMesh::QgsTriangularMesh() = default;

void QgsTriangularMesh::update( QgsMesh *nativeMesh, QgsRenderContext *context )
{
  Q_ASSERT( nativeMesh );
  Q_ASSERT( context );

  // FIND OUT IF UPDATE IS NEEDED
  if ( mTriangularMesh.vertices.size() >= nativeMesh->vertices.size() &&
       mTriangularMesh.faces.size() >= nativeMesh->faces.size() &&
       mCoordinateTransform.sourceCrs() == context->coordinateTransform().sourceCrs() &&
       mCoordinateTransform.destinationCrs() == context->coordinateTransform().destinationCrs() )
    return;

  // CLEAN-UP
  mTriangularMesh.vertices.clear();
  mTriangularMesh.faces.clear();
  mTrianglesToNativeFaces.clear();
  mNativeMeshFaceCentroids.clear();

  // TRANSFORM VERTICES
  mCoordinateTransform = context->coordinateTransform();
  mTriangularMesh.vertices.resize( nativeMesh->vertices.size() );
  for ( int i = 0; i < nativeMesh->vertices.size(); ++i )
  {
    const QgsMeshVertex &vertex = nativeMesh->vertices.at( i );
    if ( mCoordinateTransform.isValid() )
    {
      try
      {
        QgsPointXY mapPoint = mCoordinateTransform.transform( QgsPointXY( vertex.x(), vertex.y() ) );
        QgsMeshVertex mapVertex( mapPoint );
        mapVertex.addZValue( vertex.z() );
        mapVertex.setM( vertex.m() );
        mTriangularMesh.vertices[i] = mapVertex;
      }
      catch ( QgsCsException &cse )
      {
        Q_UNUSED( cse );
        QgsDebugMsg( QStringLiteral( "Caught CRS exception %1" ).arg( cse.what() ) );
        mTriangularMesh.vertices[i] = vertex;
      }
    }
    else
    {
      mTriangularMesh.vertices[i] = vertex;
    }
  }

  // CREATE TRIANGULAR MESH
  for ( int i = 0; i < nativeMesh->faces.size(); ++i )
  {
    const QgsMeshFace &face = nativeMesh->faces.at( i ) ;
    triangulate( face, i );
  }

  // CALCULATE CENTROIDS
  mNativeMeshFaceCentroids.resize( nativeMesh->faces.size() );
  for ( int i = 0; i < nativeMesh->faces.size(); ++i )
  {
    const QgsMeshFace &face = nativeMesh->faces.at( i ) ;
    QVector<QPointF> points;
    points.reserve( face.size() );
    for ( int j = 0; j < face.size(); ++j )
    {
      int index = face.at( j );
      const QgsMeshVertex &vertex = mTriangularMesh.vertices[index]; // we need projected vertices
      points.push_back( vertex.toQPointF() );
    }
    QPolygonF poly( points );
    double cx, cy;
    ENP_centroid( poly, cx, cy );
    mNativeMeshFaceCentroids[i] = QgsMeshVertex( cx, cy );
  }

  // CALCULATE SPATIAL INDEX
  mSpatialIndex = QgsMeshSpatialIndex( mTriangularMesh );
}

const QVector<QgsMeshVertex> &QgsTriangularMesh::vertices() const
{
  return mTriangularMesh.vertices;
}

const QVector<QgsMeshFace> &QgsTriangularMesh::triangles() const
{
  return mTriangularMesh.faces;
}

const QVector<QgsMeshVertex> &QgsTriangularMesh::centroids() const
{
  return mNativeMeshFaceCentroids;
}

const QVector<int> &QgsTriangularMesh::trianglesToNativeFaces() const
{
  return mTrianglesToNativeFaces;
}

int QgsTriangularMesh::faceIndexForPoint( const QgsPointXY &point ) const
{
  const QList<int> faceIndexes = mSpatialIndex.intersects( QgsRectangle( point, point ) );
  for ( const int faceIndex : faceIndexes )
  {
    const QgsMeshFace &face = mTriangularMesh.faces.at( faceIndex );
    const QgsGeometry geom = QgsMeshUtils::toGeometry( face, mTriangularMesh.vertices );
    if ( geom.contains( &point ) )
      return faceIndex;
  }
  return -1;
}

QList<int> QgsTriangularMesh::faceIndexesForRectangle( const QgsRectangle &rectangle ) const
{
  return mSpatialIndex.intersects( rectangle );
}

std::unique_ptr< QgsPolygon > QgsMeshUtils::toPolygon( const QgsMeshFace &face, const QVector<QgsMeshVertex> &vertices )
{
  QVector<QgsPoint> ring;
  for ( int j = 0; j < face.size(); ++j )
  {
    int vertexId = face[j];
    Q_ASSERT( vertexId < vertices.size() );
    const QgsPoint &vertex = vertices[vertexId];
    ring.append( vertex );
  }
  std::unique_ptr< QgsPolygon > polygon = qgis::make_unique< QgsPolygon >();
  polygon->setExteriorRing( new QgsLineString( ring ) );
  return polygon;
}

QgsGeometry QgsMeshUtils::toGeometry( const QgsMeshFace &face, const QVector<QgsMeshVertex> &vertices )
{
  return QgsGeometry( QgsMeshUtils::toPolygon( face, vertices ) );
}

QList<int> QgsMeshUtils::nativeFacesFromTriangles( const QList<int> &triangleIndexes, const QVector<int> &trianglesToNativeFaces )
{
  QSet<int> nativeFaces;
  for ( const int triangleIndex : triangleIndexes )
  {
    const int nativeIndex = trianglesToNativeFaces[triangleIndex];
    nativeFaces.insert( nativeIndex );
  }
  return nativeFaces.toList();
}
