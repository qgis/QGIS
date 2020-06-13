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
#include "qgsmeshlayerutils.h"
#include "meshOptimizer/meshoptimizer.h"

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
    if ( !( std::isnan( mTriangularMesh.vertex( ear[0] ).x() )  ||
            std::isnan( mTriangularMesh.vertex( ear[1] ).x() )  ||
            std::isnan( mTriangularMesh.vertex( ear[2] ).x() ) ) )
    {
      mTriangularMesh.faces.push_back( ear );
      mTrianglesToNativeFaces.push_back( nativeIndex );
    }
    --vertexCount;
  }

  const QgsMeshFace triangle = { face[1], face[2], face[0] };
  if ( !( std::isnan( mTriangularMesh.vertex( triangle[0] ).x() )  ||
          std::isnan( mTriangularMesh.vertex( triangle[1] ).x() )  ||
          std::isnan( mTriangularMesh.vertex( triangle[2] ).x() ) ) )
  {
    mTriangularMesh.faces.push_back( triangle );
    mTrianglesToNativeFaces.push_back( nativeIndex );
  }
}

double QgsTriangularMesh::averageTriangleSize() const
{
  return mAverageTriangleSize;
}

QgsTriangularMesh::~QgsTriangularMesh() = default;
QgsTriangularMesh::QgsTriangularMesh() = default;

bool QgsTriangularMesh::update( QgsMesh *nativeMesh, const QgsCoordinateTransform &transform )
{
  Q_ASSERT( nativeMesh );

  // FIND OUT IF UPDATE IS NEEDED
  if ( mTriangularMesh.vertices.size() >= nativeMesh->vertices.size() &&
       mTriangularMesh.faces.size() >= nativeMesh->faces.size() &&
       mTriangularMesh.edges.size() == nativeMesh->edges.size() &&
       ( ( !mCoordinateTransform.isValid() && !transform.isValid() ) ||
         ( mCoordinateTransform.sourceCrs() == transform.sourceCrs() &&
           mCoordinateTransform.destinationCrs() == transform.destinationCrs() &&
           mCoordinateTransform.isValid() == transform.isValid() ) ) )
    return false;

  // CLEAN-UP
  mTriangularMesh.vertices.clear();
  mTriangularMesh.faces.clear();
  mTriangularMesh.edges.clear();
  mTrianglesToNativeFaces.clear();
  mEdgesToNativeEdges.clear();
  mNativeMeshFaceCentroids.clear();
  mNativeMeshEdgeCentroids.clear();

  // TRANSFORM VERTICES
  mCoordinateTransform = transform;
  mTriangularMesh.vertices.resize( nativeMesh->vertices.size() );
  mExtent.setMinimal();
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
        mExtent.include( mapPoint );
      }
      catch ( QgsCsException &cse )
      {
        Q_UNUSED( cse )
        QgsDebugMsg( QStringLiteral( "Caught CRS exception %1" ).arg( cse.what() ) );
        mTriangularMesh.vertices[i] = QgsMeshVertex();
      }
    }
    else
    {
      mTriangularMesh.vertices[i] = vertex;
      mExtent.include( vertex );
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
  mSpatialFaceIndex = QgsMeshSpatialIndex( mTriangularMesh, nullptr, QgsMesh::ElementType::Face );

  // SET ALL TRIANGLE CCW AND COMPUTE AVERAGE SIZE
  finalizeTriangles();

  // CREATE EDGES
  // remove all edges with invalid vertices
  const QVector<QgsMeshEdge> edges = nativeMesh->edges;
  for ( int nativeIndex = 0; nativeIndex < edges.size(); ++nativeIndex )
  {
    const QgsMeshEdge &edge = edges.at( nativeIndex );
    if ( !( std::isnan( mTriangularMesh.vertex( edge.first ).x() )  ||
            std::isnan( mTriangularMesh.vertex( edge.second ).x() ) ) )
    {
      mTriangularMesh.edges.push_back( edge );
      mEdgesToNativeEdges.push_back( nativeIndex );
    }
  }

  // CALCULATE CENTROIDS
  mNativeMeshEdgeCentroids.resize( nativeMesh->edgeCount() );
  for ( int i = 0; i < nativeMesh->edgeCount(); ++i )
  {
    const QgsMeshEdge &edge = nativeMesh->edges.at( i ) ;
    const QgsPoint &a = mTriangularMesh.vertices[edge.first];
    const QgsPoint &b = mTriangularMesh.vertices[edge.second];
    mNativeMeshEdgeCentroids[i] = QgsMeshVertex( ( a.x() + b.x() ) / 2.0, ( a.y() + b.y() ) / 2.0, ( a.z() + b.z() ) / 2.0 );
  }

  // CALCULATE SPATIAL INDEX
  mSpatialEdgeIndex = QgsMeshSpatialIndex( mTriangularMesh, nullptr, QgsMesh::ElementType::Edge );

  return true;
}

void QgsTriangularMesh::finalizeTriangles()
{
  mAverageTriangleSize = 0;
  for ( int i = 0; i < mTriangularMesh.faceCount(); ++i )
  {
    QgsMeshFace &face = mTriangularMesh.faces[i];

    const QgsMeshVertex &v0 = mTriangularMesh.vertex( face[0] );
    const QgsMeshVertex &v1 = mTriangularMesh.vertex( face[1] );
    const QgsMeshVertex &v2 = mTriangularMesh.vertex( face[2] );

    QgsRectangle bbox = QgsMeshLayerUtils::triangleBoundingBox( v0, v1, v2 );

    mAverageTriangleSize += std::fmax( bbox.width(), bbox.height() );

    //To have consistent clock wise orientation of triangles which is necessary for 3D rendering
    //Check the clock wise, and if it is not counter clock wise, swap indexes to make the oientation counter clock wise
    double ux = v1.x() - v0.x();
    double uy = v1.y() - v0.y();
    double vx = v2.x() - v0.x();
    double vy = v2.y() - v0.y();

    double crossProduct = ux * vy - uy * vx;
    if ( crossProduct < 0 ) //CW -->change the orientation
    {
      std::swap( face[1], face[2] );
    }
  }
  mAverageTriangleSize /= mTriangularMesh.faceCount();
}

QgsRectangle QgsTriangularMesh::extent() const
{
  return mExtent;
}

int QgsTriangularMesh::levelOfDetail() const
{
  return mLod;
}

bool QgsTriangularMesh::contains( const QgsMesh::ElementType &type ) const
{
  switch ( type )
  {
    case QgsMesh::ElementType::Vertex:
      return mTriangularMesh.vertexCount() != 0;
    case QgsMesh::ElementType::Edge:
      return mTriangularMesh.edgeCount() != 0;
    case QgsMesh::ElementType::Face:
      return mTriangularMesh.faceCount() != 0;
  }

  return false;
}

const QVector<QgsMeshVertex> &QgsTriangularMesh::vertices() const
{
  return mTriangularMesh.vertices;
}

const QVector<QgsMeshFace> &QgsTriangularMesh::triangles() const
{
  return mTriangularMesh.faces;
}

const QVector<QgsMeshEdge> &QgsTriangularMesh::edges() const
{
  return mTriangularMesh.edges;
}

const QVector<QgsMeshVertex> &QgsTriangularMesh::centroids() const
{
  return faceCentroids();
}

const QVector<QgsMeshVertex> &QgsTriangularMesh::faceCentroids() const
{
  return mNativeMeshFaceCentroids;
}

const QVector<QgsMeshVertex> &QgsTriangularMesh::edgeCentroids() const
{
  return mNativeMeshEdgeCentroids;
}

const QVector<int> &QgsTriangularMesh::trianglesToNativeFaces() const
{
  return mTrianglesToNativeFaces;
}

const QVector<int> &QgsTriangularMesh::edgesToNativeEdges() const
{
  return mEdgesToNativeEdges;
}

int QgsTriangularMesh::faceIndexForPoint( const QgsPointXY &point ) const
{
  const QList<int> faceIndexes = mSpatialFaceIndex.intersects( QgsRectangle( point, point ) );
  for ( const int faceIndex : faceIndexes )
  {
    const QgsMeshFace &face = mTriangularMesh.faces.at( faceIndex );
    const QgsGeometry geom = QgsMeshUtils::toGeometry( face, mTriangularMesh.vertices );
    if ( geom.contains( &point ) )
      return faceIndex;
  }
  return -1;
}

int QgsTriangularMesh::faceIndexForPoint_v2( const QgsPointXY &point ) const
{
  const QList<int> faceIndexes = mSpatialFaceIndex.intersects( QgsRectangle( point, point ) );

  for ( const int faceIndex : faceIndexes )
  {
    const QgsMeshFace &face = mTriangularMesh.faces.at( faceIndex );
    if ( QgsMeshUtils::isInTriangleFace( point, face, mTriangularMesh.vertices ) )
      return faceIndex;
  }
  return -1;
}

QList<int> QgsTriangularMesh::faceIndexesForRectangle( const QgsRectangle &rectangle ) const
{
  return mSpatialFaceIndex.intersects( rectangle );
}

QList<int> QgsTriangularMesh::edgeIndexesForRectangle( const QgsRectangle &rectangle ) const
{
  return mSpatialEdgeIndex.intersects( rectangle );
}

QVector<QVector3D> QgsTriangularMesh::vertexNormals( float vertScale ) const
{
  QVector<QVector3D> normales( vertices().count(), QVector3D( 0, 0, 0 ) );

  for ( const auto &face : triangles() )
  {
    for ( int i = 0; i < 3; i++ )
    {
      int index1( face.at( i ) );
      int index2( face.at( ( i + 1 ) % 3 ) );
      int index3( face.at( ( i + 2 ) % 3 ) );

      const QgsMeshVertex &vert( vertices().at( index1 ) );
      const QgsMeshVertex &otherVert1( vertices().at( index2 ) );
      const QgsMeshVertex &otherVert2( vertices().at( index3 ) );

      QVector3D v1( float( otherVert1.x() - vert.x() ), float( otherVert1.y() - vert.y() ), vertScale * float( otherVert1.z() - vert.z() ) );
      QVector3D v2( float( otherVert2.x() - vert.x() ), float( otherVert2.y() - vert.y() ), vertScale * float( otherVert2.z() - vert.z() ) );

      normales[index1] += QVector3D::crossProduct( v1, v2 );
    }
  }
  return normales;
}

QVector<QgsTriangularMesh *> QgsTriangularMesh::simplifyMesh( double reductionFactor, int minimumTrianglesCount ) const
{
  QVector<QgsTriangularMesh *> simplifiedMeshes;

  if ( mTriangularMesh.edgeCount() != 0 )
    return simplifiedMeshes;

  if ( !( reductionFactor > 1 ) )
    return simplifiedMeshes;

  size_t verticesCount = size_t( mTriangularMesh.vertices.count() );

  unsigned int baseIndexCount = mTriangularMesh.faceCount() * 3;

  QVector<unsigned int> indexes( mTriangularMesh.faces.count() * 3 );
  for ( int i = 0; i < mTriangularMesh.faceCount(); ++i )
  {
    const QgsMeshFace &f = mTriangularMesh.face( i );
    for ( int j = 0; j < 3; ++j )
      indexes[i * 3 + j] = f.at( j );
  }

  QVector<float> vertices( mTriangularMesh.vertices.count() * 3 );
  for ( int i = 0; i < mTriangularMesh.vertices.count(); ++i )
  {
    const QgsMeshVertex &v = mTriangularMesh.vertex( i );
    vertices[i * 3] = v.x() ;
    vertices[i * 3 + 1] = v.y() ;
    vertices[i * 3 + 2] = v.z() ;
  }

  int path = 0;
  while ( true )
  {
    QgsTriangularMesh *simplifiedMesh = new QgsTriangularMesh( *this );
    size_t maxNumberOfIndexes = baseIndexCount / pow( reductionFactor, path + 1 );

    if ( indexes.size() <= int( maxNumberOfIndexes ) )
      break;

    QVector<unsigned int> returnIndexes( indexes.size() );
    //returned size could be different than goal size but not than the input indexes count
    size_t size = meshopt_simplifySloppy(
                    returnIndexes.data(),
                    indexes.data(),
                    indexes.size(),
                    vertices.data(),
                    verticesCount,
                    sizeof( float ) * 3,
                    maxNumberOfIndexes );


    returnIndexes.resize( size );

    if ( size == 0 || int( size ) >= indexes.size() )
    {
      QgsDebugMsg( QStringLiteral( "Mesh simplification failed after %1 path" ).arg( path + 1 ) );
      break;
    }

    QgsMesh newMesh;
    newMesh.vertices = mTriangularMesh.vertices;

    newMesh.faces.resize( returnIndexes.size() / 3 );
    for ( int i = 0; i < newMesh.faces.size(); ++i )
    {
      QgsMeshFace f( 3 );
      for ( size_t j = 0; j < 3 ; ++j )
        f[j] = returnIndexes.at( i * 3 + j ) ;
      newMesh.faces[i ] = f;
    }

    simplifiedMesh->mTriangularMesh = newMesh;
    simplifiedMesh->mSpatialFaceIndex = QgsMeshSpatialIndex( simplifiedMesh->mTriangularMesh );
    simplifiedMesh->finalizeTriangles();
    simplifiedMeshes.push_back( simplifiedMesh );

    QgsDebugMsg( QStringLiteral( "Simplified mesh created with %1 triangles" ).arg( newMesh.faceCount() ) );

    simplifiedMesh->mTrianglesToNativeFaces = QVector<int>( simplifiedMesh->triangles().count(), 0 );
    for ( int i = 0; i < simplifiedMesh->mTrianglesToNativeFaces.count(); ++i )
    {
      QgsMeshFace triangle = simplifiedMesh->triangles().at( i );
      double x = 0;
      double y = 0;
      for ( size_t j = 0; j < 3 ; ++j )
      {
        x += mTriangularMesh.vertex( triangle[j] ).x();
        y += mTriangularMesh.vertex( triangle[j] ).y();
      }
      x /= 3;
      y /= 3;
      QgsPoint centroid( x, y );
      int indexInBaseMesh = faceIndexForPoint_v2( centroid );

      if ( indexInBaseMesh == -1 )
      {
        // sometime the centroid of simplified mesh could be outside the base mesh,
        // so try with vertices of the simplified triangle
        int j = 0;
        while ( indexInBaseMesh == -1 && j < 3 )
          indexInBaseMesh = faceIndexForPoint_v2( mTriangularMesh.vertex( triangle[j++] ) );
      }

      if ( indexInBaseMesh > -1 && indexInBaseMesh < mTrianglesToNativeFaces.count() )
        simplifiedMesh->mTrianglesToNativeFaces[i] = mTrianglesToNativeFaces[indexInBaseMesh];
    }

    simplifiedMesh->mLod = path + 1;
    simplifiedMesh->mBaseTriangularMesh = this;

    if ( simplifiedMesh->triangles().count() <  minimumTrianglesCount )
      break;

    indexes = returnIndexes;
    ++path;
  }

  return simplifiedMeshes;
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

static QSet<int> _nativeElementsFromElements( const QList<int> &indexes, const QVector<int> &elementToNativeElements )
{
  QSet<int> nativeElements;
  for ( const int index : indexes )
  {
    const int nativeIndex = elementToNativeElements[index];
    nativeElements.insert( nativeIndex );
  }
  return nativeElements;
}

QSet<int> QgsMeshUtils::nativeFacesFromTriangles( const QList<int> &triangleIndexes, const QVector<int> &trianglesToNativeFaces )
{
  return _nativeElementsFromElements( triangleIndexes, trianglesToNativeFaces );
}

QSet<int> QgsMeshUtils::nativeEdgesFromEdges( const QList<int> &edgesIndexes, const QVector<int> &edgesToNativeEdges )
{
  return _nativeElementsFromElements( edgesIndexes, edgesToNativeEdges );
}


static double _isLeft2D( const QgsPoint &p1, const QgsPoint &p2, const QgsPoint &p )
{
  return ( p2.x() - p1.x() ) * ( p.y() - p1.y() ) - ( p.x() - p1.x() ) * ( p2.y() - p1.y() );
}

static bool _isInTriangle2D( const QgsPoint &p, const QVector<QgsMeshVertex> &triangle )
{
  return ( ( _isLeft2D( triangle[2], triangle[0], p ) * _isLeft2D( triangle[2], triangle[0], triangle[1] ) >= 0 )
           && ( _isLeft2D( triangle[0], triangle[1], p ) * _isLeft2D( triangle[0], triangle[1], triangle[2] ) >= 0 )
           && ( _isLeft2D( triangle[2], triangle[1], p ) * _isLeft2D( triangle[2], triangle[1], triangle[0] ) >= 0 ) );
}

bool QgsMeshUtils::isInTriangleFace( const QgsPointXY point, const QgsMeshFace &face, const QVector<QgsMeshVertex> &vertices )
{
  if ( face.count() != 3 )
    return false;

  QVector<QgsMeshVertex> triangle( 3 );
  for ( int i = 0; i < 3; ++i )
  {
    if ( face[i] > vertices.count() )
      return false;
    triangle[i] = vertices[face[i]];
  }

  const QgsPoint p( point.x(), point.y() );

  return _isInTriangle2D( p, triangle );
}

QSet<int> QgsMeshUtils::nativeVerticesFromTriangles( const QList<int> &triangleIndexes, const QVector<QgsMeshFace> &triangles )
{
  QSet<int> uniqueVertices;
  for ( int triangleIndex : triangleIndexes )
  {
    const QgsMeshFace triangle = triangles[triangleIndex];
    for ( int i : triangle )
    {
      uniqueVertices.insert( i );
    }
  }
  return uniqueVertices;
}

QSet<int> QgsMeshUtils::nativeVerticesFromEdges( const QList<int> &edgesIndexes, const QVector<QgsMeshEdge> &edges )
{
  QSet<int> uniqueVertices;
  for ( int edgeIndex : edgesIndexes )
  {
    const QgsMeshEdge edge = edges[edgeIndex];
    uniqueVertices.insert( edge.first );
    uniqueVertices.insert( edge.second );
  }
  return uniqueVertices;
}
