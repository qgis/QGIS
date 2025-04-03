/***************************************************************************
                         qgsmeshutils.cpp
                         ----------------------------
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

#include "qgsmeshutils.h"
#include "qgsmeshlayer.h"
#include "qgsmaptopixel.h"
#include "qgsrendercontext.h"
#include "qgstriangularmesh.h"
#include "qgsmeshlayerutils.h"
#include "qgsmeshlayerinterpolator.h"
#include "qgsgeometry.h"
#include "qgspolygon.h"
#include "qgslinestring.h"

#include <memory>

QgsRasterBlock *QgsMeshUtils::exportRasterBlock(
  const QgsMeshLayer &layer,
  const QgsMeshDatasetIndex &datasetIndex,
  const QgsCoordinateReferenceSystem &destinationCrs,
  const QgsCoordinateTransformContext &transformContext,
  double mapUnitsPerPixel,
  const QgsRectangle &extent,
  QgsRasterBlockFeedback *feedback )
{
  if ( !layer.dataProvider() )
    return nullptr;

  if ( !datasetIndex.isValid() )
    return nullptr;

  const int widthPixel = static_cast<int>( extent.width() / mapUnitsPerPixel );
  const int heightPixel = static_cast<int>( extent.height() / mapUnitsPerPixel );

  const QgsPointXY center = extent.center();
  const QgsMapToPixel mapToPixel( mapUnitsPerPixel,
                                  center.x(),
                                  center.y(),
                                  widthPixel,
                                  heightPixel,
                                  0 );
  const QgsCoordinateTransform transform( layer.crs(), destinationCrs, transformContext );

  QgsRenderContext renderContext;
  renderContext.setCoordinateTransform( transform );
  renderContext.setMapToPixel( mapToPixel );
  renderContext.setExtent( extent );

  auto nativeMesh = std::make_unique<QgsMesh>();
  layer.dataProvider()->populateMesh( nativeMesh.get() );
  auto triangularMesh = std::make_unique<QgsTriangularMesh>();
  triangularMesh->update( nativeMesh.get(), transform );

  const QgsMeshDatasetGroupMetadata metadata = layer.datasetGroupMetadata( datasetIndex );
  const QgsMeshDatasetGroupMetadata::DataType scalarDataType = QgsMeshLayerUtils::datasetValuesType( metadata.dataType() );
  const int count =  QgsMeshLayerUtils::datasetValuesCount( nativeMesh.get(), scalarDataType );
  const QgsMeshDataBlock vals = QgsMeshLayerUtils::datasetValues(
                                  &layer,
                                  datasetIndex,
                                  0,
                                  count );
  if ( !vals.isValid() )
    return nullptr;

  const QVector<double> datasetValues = QgsMeshLayerUtils::calculateMagnitudes( vals );
  const QgsMeshDataBlock activeFaceFlagValues = layer.areFacesActive(
        datasetIndex,
        0,
        nativeMesh->faces.count() );

  QgsMeshLayerInterpolator interpolator(
    *( triangularMesh.get() ),
    datasetValues,
    activeFaceFlagValues,
    scalarDataType,
    renderContext,
    QSize( widthPixel, heightPixel )
  );

  return interpolator.block( 0, extent, widthPixel, heightPixel, feedback );
}

QgsRasterBlock *QgsMeshUtils::exportRasterBlock(
  const QgsTriangularMesh &triangularMesh,
  const QgsMeshDataBlock &datasetValues,
  const QgsMeshDataBlock &activeFlags,
  const QgsMeshDatasetGroupMetadata::DataType dataType,
  const QgsCoordinateTransform &transform,
  double mapUnitsPerPixel,
  const QgsRectangle &extent,
  QgsRasterBlockFeedback *feedback )
{

  const int widthPixel = static_cast<int>( extent.width() / mapUnitsPerPixel );
  const int heightPixel = static_cast<int>( extent.height() / mapUnitsPerPixel );

  const QgsPointXY center = extent.center();
  const QgsMapToPixel mapToPixel( mapUnitsPerPixel,
                                  center.x(),
                                  center.y(),
                                  widthPixel,
                                  heightPixel,
                                  0 );

  QgsRenderContext renderContext;
  renderContext.setCoordinateTransform( transform );
  renderContext.setMapToPixel( mapToPixel );
  renderContext.setExtent( extent );

  const QVector<double> magnitudes = QgsMeshLayerUtils::calculateMagnitudes( datasetValues );

  QgsMeshLayerInterpolator interpolator(
    triangularMesh,
    magnitudes,
    activeFlags,
    dataType,
    renderContext,
    QSize( widthPixel, heightPixel )
  );

  return interpolator.block( 0, extent, widthPixel, heightPixel, feedback );
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
  auto polygon = std::make_unique< QgsPolygon >();
  polygon->setExteriorRing( new QgsLineString( ring ) );
  return polygon;
}

QgsGeometry QgsMeshUtils::toGeometry( const QgsMeshFace &face, const QVector<QgsMeshVertex> &vertices )
{
  return QgsGeometry( QgsMeshUtils::toPolygon( face, vertices ) );
}

static QSet<int> nativeElementsFromElements( const QList<int> &indexes, const QVector<int> &elementToNativeElements )
{
  QSet<int> nativeElements;
  for ( const int index : indexes )
  {
    if ( index < elementToNativeElements.count() )
    {
      const int nativeIndex = elementToNativeElements[index];
      nativeElements.insert( nativeIndex );
    }
  }
  return nativeElements;
}

QSet<int> QgsMeshUtils::nativeFacesFromTriangles( const QList<int> &triangleIndexes, const QVector<int> &trianglesToNativeFaces )
{
  return nativeElementsFromElements( triangleIndexes, trianglesToNativeFaces );
}

QSet<int> QgsMeshUtils::nativeEdgesFromEdges( const QList<int> &edgesIndexes, const QVector<int> &edgesToNativeEdges )
{
  return nativeElementsFromElements( edgesIndexes, edgesToNativeEdges );
}

static double isLeft2D( const QgsPoint &p1, const QgsPoint &p2, const QgsPoint &p )
{
  return ( p2.x() - p1.x() ) * ( p.y() - p1.y() ) - ( p.x() - p1.x() ) * ( p2.y() - p1.y() );
}

static bool isInTriangle2D( const QgsPoint &p, const QVector<QgsMeshVertex> &triangle )
{
  return ( ( isLeft2D( triangle[2], triangle[0], p ) * isLeft2D( triangle[2], triangle[0], triangle[1] ) >= 0 )
           && ( isLeft2D( triangle[0], triangle[1], p ) * isLeft2D( triangle[0], triangle[1], triangle[2] ) >= 0 )
           && ( isLeft2D( triangle[2], triangle[1], p ) * isLeft2D( triangle[2], triangle[1], triangle[0] ) >= 0 ) );
}

bool QgsMeshUtils::isInTriangleFace( const QgsPointXY point, const QgsMeshFace &face, const QVector<QgsMeshVertex> &vertices )
{
  if ( face.count() != 3 )
    return false;

  QVector<QgsMeshVertex> triangle( 3 );
  for ( int i = 0; i < 3; ++i )
  {
    if ( face[i] >= vertices.count() )
      return false;
    triangle[i] = vertices[face[i]];
  }

  const QgsPoint p( point.x(), point.y() );

  return isInTriangle2D( p, triangle );
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

  const QPointF &pt0 = pX.first();
  QPolygonF localPolygon( pX.count() );
  for ( int i = 0; i < pX.count(); ++i )
    localPolygon[i] = pX.at( i ) - pt0;

  // For all vertices except last
  int i = 0;
  for ( ; i < localPolygon.size() - 1; ++i )
  {
    ENP_centroid_step( localPolygon, cx, cy, signedArea, i, i + 1 );
  }
  // Do last vertex separately to avoid performing an expensive
  // modulus operation in each iteration.
  ENP_centroid_step( localPolygon, cx, cy, signedArea, i, 0 );

  signedArea *= 0.5;
  cx /= ( 6.0 * signedArea );
  cy /= ( 6.0 * signedArea );

  cx = cx + pt0.x();
  cy = cy + pt0.y();
}

QgsMeshVertex QgsMeshUtils::centroid( const QgsMeshFace &face, const QVector<QgsMeshVertex> &vertices )
{
  QVector<QPointF> points( face.size() );
  for ( int j = 0; j < face.size(); ++j )
  {
    int index = face.at( j );
    const QgsMeshVertex &vertex = vertices.at( index ); // we need vertices in map coordinate
    points[j] = vertex.toQPointF();
  }
  QPolygonF poly( points );
  double cx, cy;
  ENP_centroid( poly, cx, cy );
  return QgsMeshVertex( cx, cy );
}

void QgsMeshUtils::setCounterClockwise( QgsMeshFace &triangle, const QgsMeshVertex &v0, const QgsMeshVertex &v1, const QgsMeshVertex &v2 )
{
  //To have consistent clock wise orientation of triangles which is necessary for 3D rendering
  //Check the clock wise, and if it is not counter clock wise, swap indexes to make the oientation counter clock wise
  double ux = v1.x() - v0.x();
  double uy = v1.y() - v0.y();
  double vx = v2.x() - v0.x();
  double vy = v2.y() - v0.y();

  double crossProduct = ux * vy - uy * vx;
  if ( crossProduct < 0 ) //CW -->change the orientation
  {
    std::swap( triangle[1], triangle[2] );
  }
}
