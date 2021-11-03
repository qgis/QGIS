/***************************************************************************
                         qgsmeshlayerinterpolator.cpp
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

///@cond PRIVATE

#include <memory>
#include <limits>

#include "qgsmeshlayerinterpolator.h"

#include "qgis.h"
#include "qgsrasterinterface.h"
#include "qgsmaptopixel.h"
#include "qgsvector.h"
#include "qgspoint.h"
#include "qgspointxy.h"
#include "qgsmeshlayerutils.h"
#include "qgsmeshlayer.h"
#include "qgscoordinatetransformcontext.h"
#include "qgscoordinatetransform.h"
#include "qgsmeshdataprovider.h"
#include "qgsrendercontext.h"

QgsMeshLayerInterpolator::QgsMeshLayerInterpolator(
  const QgsTriangularMesh &m,
  const QVector<double> &datasetValues,
  const QgsMeshDataBlock &activeFaceFlagValues,
  QgsMeshDatasetGroupMetadata::DataType dataType,
  const QgsRenderContext &context,
  const QSize &size )
  : mTriangularMesh( m ),
    mDatasetValues( datasetValues ),
    mActiveFaceFlagValues( activeFaceFlagValues ),
    mContext( context ),
    mDataType( dataType ),
    mOutputSize( size )
{
}

QgsMeshLayerInterpolator::~QgsMeshLayerInterpolator() = default;

QgsRasterInterface *QgsMeshLayerInterpolator::clone() const
{
  assert( false ); // we should not need this (hopefully)
  return nullptr;
}

Qgis::DataType QgsMeshLayerInterpolator::dataType( int ) const
{
  return Qgis::DataType::Float64;
}

int QgsMeshLayerInterpolator::bandCount() const
{
  return 1;
}

QgsRasterBlock *QgsMeshLayerInterpolator::block( int, const QgsRectangle &extent, int width, int height, QgsRasterBlockFeedback *feedback )
{
  std::unique_ptr<QgsRasterBlock> outputBlock( new QgsRasterBlock( Qgis::DataType::Float64, width, height ) );
  const double noDataValue = std::numeric_limits<double>::quiet_NaN();
  outputBlock->setNoDataValue( noDataValue );
  outputBlock->setIsNoData();  // assume initially that all values are unset
  double *data = reinterpret_cast<double *>( outputBlock->bits() );

  QList<int> spatialIndexTriangles;
  int indexCount;
  if ( mSpatialIndexActive )
  {
    spatialIndexTriangles = mTriangularMesh.faceIndexesForRectangle( extent );
    indexCount = spatialIndexTriangles.count();
  }
  else
  {
    indexCount = mTriangularMesh.triangles().count();
  }

  if ( mTriangularMesh.contains( QgsMesh::ElementType::Edge ) )
  {
    return outputBlock.release();
  }

  const QVector<QgsMeshVertex> &vertices = mTriangularMesh.vertices();

  // currently expecting that triangulation does not add any new extra vertices on the way
  if ( mDataType == QgsMeshDatasetGroupMetadata::DataType::DataOnVertices )
    Q_ASSERT( mDatasetValues.count() == mTriangularMesh.vertices().count() );

  for ( int i = 0; i < indexCount; ++i )
  {
    if ( feedback && feedback->isCanceled() )
      break;

    if ( mContext.renderingStopped() )
      break;

    int triangleIndex;
    if ( mSpatialIndexActive )
      triangleIndex = spatialIndexTriangles[i];
    else
      triangleIndex = i;

    const QgsMeshFace &face = mTriangularMesh.triangles()[triangleIndex];

    if ( face.isEmpty() )
      continue;

    const int v1 = face[0], v2 = face[1], v3 = face[2];
    const QgsPointXY &p1 = vertices[v1], &p2 = vertices[v2], &p3 = vertices[v3];

    const int nativeFaceIndex = mTriangularMesh.trianglesToNativeFaces()[triangleIndex];
    const bool isActive = mActiveFaceFlagValues.active( nativeFaceIndex );
    if ( !isActive )
      continue;

    const QgsRectangle bbox = QgsMeshLayerUtils::triangleBoundingBox( p1, p2, p3 );
    if ( !extent.intersects( bbox ) )
      continue;

    // Get the BBox of the element in pixels
    int topLim, bottomLim, leftLim, rightLim;
    QgsMeshLayerUtils::boundingBoxToScreenRectangle( mContext.mapToPixel(), mOutputSize, bbox, leftLim, rightLim, topLim, bottomLim );

    double value( 0 ), value1( 0 ), value2( 0 ), value3( 0 );
    const int faceIdx = mTriangularMesh.trianglesToNativeFaces()[triangleIndex];

    if ( mDataType == QgsMeshDatasetGroupMetadata::DataType::DataOnVertices )
    {
      value1 = mDatasetValues[v1];
      value2 = mDatasetValues[v2];
      value3 = mDatasetValues[v3];
    }
    else
      value = mDatasetValues[faceIdx];

    // interpolate in the bounding box of the face
    for ( int j = topLim; j <= bottomLim; j++ )
    {
      double *line = data + ( j * width );
      for ( int k = leftLim; k <= rightLim; k++ )
      {
        double val;
        const QgsPointXY p = mContext.mapToPixel().toMapCoordinates( k, j );
        if ( mDataType == QgsMeshDatasetGroupMetadata::DataType::DataOnVertices )
          val = QgsMeshLayerUtils::interpolateFromVerticesData(
                  p1,
                  p2,
                  p3,
                  value1,
                  value2,
                  value3,
                  p );
        else
        {
          val = QgsMeshLayerUtils::interpolateFromFacesData(
                  p1,
                  p2,
                  p3,
                  value,
                  p
                );
        }
        if ( !std::isnan( val ) )
        {
          line[k] = val;
          outputBlock->setIsData( j, k );
        }
      }
    }

  }

  return outputBlock.release();
}

void QgsMeshLayerInterpolator::setSpatialIndexActive( bool active ) {mSpatialIndexActive = active;}

///@endcond

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

  std::unique_ptr<QgsMesh> nativeMesh = std::make_unique<QgsMesh>();
  layer.dataProvider()->populateMesh( nativeMesh.get() );
  std::unique_ptr<QgsTriangularMesh> triangularMesh = std::make_unique<QgsTriangularMesh>();
  triangularMesh->update( nativeMesh.get(), transform );

  const QgsMeshDatasetGroupMetadata metadata = layer.dataProvider()->datasetGroupMetadata( datasetIndex );
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
  const QgsMeshDataBlock activeFaceFlagValues = layer.dataProvider()->areFacesActive(
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
