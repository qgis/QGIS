/***************************************************************************
                         qgsmeshlayerrenderer.cpp
                         ------------------------
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
#include <QSet>
#include <QPair>

#include "qgsmeshlayerrenderer.h"

#include "qgsfield.h"
#include "qgslogger.h"
#include "qgsmeshlayer.h"
#include "qgspointxy.h"
#include "qgsrenderer.h"
#include "qgssinglebandpseudocolorrenderer.h"
#include "qgsrastershader.h"
#include "qgsmeshlayerinterpolator.h"
#include "qgsmeshlayerutils.h"
#include "qgsmeshvectorrenderer.h"
#include "qgsfillsymbollayer.h"
#include "qgssettings.h"
#include "qgsstyle.h"


QgsMeshLayerRenderer::QgsMeshLayerRenderer( QgsMeshLayer *layer, QgsRenderContext &context )
  : QgsMapLayerRenderer( layer->id(), &context )
  , mFeedback( new QgsMeshLayerRendererFeedback )
  , mRendererSettings( layer->rendererSettings() )
{
  // make copies for mesh data
  Q_ASSERT( layer->nativeMesh() );
  Q_ASSERT( layer->triangularMesh() );
  Q_ASSERT( layer->rendererCache() );
  Q_ASSERT( layer->dataProvider() );

  mNativeMesh = *( layer->nativeMesh() );
  mTriangularMesh = *( layer->triangularMesh() );

  copyScalarDatasetValues( layer );
  copyVectorDatasetValues( layer );

  calculateOutputSize();
}

QgsFeedback *QgsMeshLayerRenderer::feedback() const
{
  return mFeedback.get();
}

void QgsMeshLayerRenderer::calculateOutputSize()
{
  // figure out image size
  QgsRenderContext &context = *renderContext();
  QgsRectangle extent = context.extent();  // this is extent in layer's coordinate system - but we need it in map coordinate system
  QgsMapToPixel mapToPixel = context.mapToPixel();
  QgsPointXY topleft = mapToPixel.transform( extent.xMinimum(), extent.yMaximum() );
  QgsPointXY bottomright = mapToPixel.transform( extent.xMaximum(), extent.yMinimum() );
  int width = int( bottomright.x() - topleft.x() );
  int height = int( bottomright.y() - topleft.y() );
  mOutputSize = QSize( width, height );
}

void QgsMeshLayerRenderer::copyScalarDatasetValues( QgsMeshLayer *layer )
{
  const QgsMeshDatasetIndex datasetIndex = mRendererSettings.activeScalarDataset();

  // Find out if we can use cache up to date. If yes, use it and return
  const int datasetGroupCount = layer->dataProvider()->datasetGroupCount();
  const QgsMeshRendererScalarSettings::DataInterpolationMethod method = mRendererSettings.scalarSettings( datasetIndex.group() ).dataInterpolationMethod();
  QgsMeshLayerRendererCache *cache = layer->rendererCache();
  if ( ( cache->mDatasetGroupsCount == datasetGroupCount ) &&
       ( cache->mActiveScalarDatasetIndex == datasetIndex ) &&
       ( cache->mDataInterpolationMethod ==  method ) )
  {
    mScalarDatasetValues = cache->mScalarDatasetValues;
    mScalarActiveFaceFlagValues = cache->mScalarActiveFaceFlagValues;
    mScalarDataOnVertices = cache->mScalarDataOnVertices;
    mScalarDatasetMinimum = cache->mScalarDatasetMinimum;
    mScalarDatasetMaximum = cache->mScalarDatasetMaximum;
    return;
  }

  // Cache is not up-to-date, gather data
  if ( datasetIndex.isValid() )
  {
    const QgsMeshDatasetGroupMetadata metadata = layer->dataProvider()->datasetGroupMetadata( datasetIndex );
    mScalarDataOnVertices = metadata.dataType() == QgsMeshDatasetGroupMetadata::DataOnVertices;

    // populate scalar values
    QgsMeshDataBlock vals = layer->dataProvider()->datasetValues(
                              datasetIndex,
                              0,
                              mScalarDataOnVertices ? mNativeMesh.vertices.count() : mNativeMesh.faces.count() );

    // vals could be scalar or vectors, for contour rendering we want always magnitude
    mScalarDatasetValues = QgsMeshLayerUtils::calculateMagnitudes( vals );

    // populate face active flag, always defined on faces
    mScalarActiveFaceFlagValues = layer->dataProvider()->areFacesActive(
                                    datasetIndex,
                                    0,
                                    mNativeMesh.faces.count() );

    // for data on faces, there could be request to interpolate the data to vertices
    if ( ( !mScalarDataOnVertices ) && ( method != QgsMeshRendererScalarSettings::None ) )
    {
      mScalarDataOnVertices = true;
      mScalarDatasetValues = QgsMeshLayerUtils::interpolateFromFacesData(
                               mScalarDatasetValues,
                               &mNativeMesh,
                               &mTriangularMesh,
                               &mScalarActiveFaceFlagValues,
                               method
                             );
    }

    const QgsMeshDatasetMetadata datasetMetadata = layer->dataProvider()->datasetMetadata( datasetIndex );
    mScalarDatasetMinimum = datasetMetadata.minimum();
    mScalarDatasetMaximum = datasetMetadata.maximum();
  }

  // update cache
  cache->mDatasetGroupsCount = datasetGroupCount;
  cache->mActiveScalarDatasetIndex = datasetIndex;
  cache->mDataInterpolationMethod = method;
  cache->mScalarDatasetValues = mScalarDatasetValues;
  cache->mScalarActiveFaceFlagValues = mScalarActiveFaceFlagValues;
  cache->mScalarDataOnVertices = mScalarDataOnVertices;
  cache->mScalarDatasetMinimum = mScalarDatasetMinimum;
  cache->mScalarDatasetMaximum = mScalarDatasetMaximum;
}

void QgsMeshLayerRenderer::copyVectorDatasetValues( QgsMeshLayer *layer )
{
  const QgsMeshDatasetIndex datasetIndex = mRendererSettings.activeVectorDataset();

  // Find out if we can use cache up to date. If yes, use it and return
  const int datasetGroupCount = layer->dataProvider()->datasetGroupCount();
  QgsMeshLayerRendererCache *cache = layer->rendererCache();
  if ( ( cache->mDatasetGroupsCount == datasetGroupCount ) &&
       ( cache->mActiveVectorDatasetIndex == datasetIndex ) )
  {
    mVectorDatasetValues = cache->mVectorDatasetValues;
    mVectorDatasetValuesMag = cache->mVectorDatasetValuesMag;
    mVectorDatasetMagMinimum = cache->mVectorDatasetMagMinimum;
    mVectorDatasetMagMaximum = cache->mVectorDatasetMagMaximum;
    mVectorDatasetGroupMagMinimum = cache->mVectorDatasetMagMinimum;
    mVectorDatasetGroupMagMaximum = cache->mVectorDatasetMagMaximum;
    mVectorDataOnVertices = cache->mVectorDataOnVertices;
    return;
  }


  // Cache is not up-to-date, gather data
  if ( datasetIndex.isValid() )
  {
    const QgsMeshDatasetGroupMetadata metadata = layer->dataProvider()->datasetGroupMetadata( datasetIndex );

    bool isScalar = metadata.isScalar();
    if ( isScalar )
    {
      QgsDebugMsg( QStringLiteral( "Dataset has no vector values" ) );
    }
    else
    {
      mVectorDataOnVertices = metadata.dataType() == QgsMeshDatasetGroupMetadata::DataOnVertices;
      mVectorDatasetGroupMagMinimum = metadata.minimum();
      mVectorDatasetGroupMagMaximum = metadata.maximum();

      int count;
      if ( mVectorDataOnVertices )
        count = mNativeMesh.vertices.count();
      else
        count = mNativeMesh.faces.count();


      mVectorDatasetValues = layer->dataProvider()->datasetValues(
                               datasetIndex,
                               0,
                               count );

      mVectorDatasetValuesMag = QgsMeshLayerUtils::calculateMagnitudes( mVectorDatasetValues );

      const QgsMeshDatasetMetadata datasetMetadata = layer->dataProvider()->datasetMetadata( datasetIndex );
      mVectorDatasetMagMinimum = datasetMetadata.minimum();
      mVectorDatasetMagMaximum = datasetMetadata.maximum();
    }
  }

  // update cache
  cache->mDatasetGroupsCount = datasetGroupCount;
  cache->mActiveVectorDatasetIndex = datasetIndex;
  cache->mVectorDatasetValues = mVectorDatasetValues;
  cache->mVectorDatasetValuesMag = mVectorDatasetValuesMag;
  cache->mVectorDatasetMagMinimum = mVectorDatasetMagMinimum;
  cache->mVectorDatasetMagMaximum = mVectorDatasetMagMaximum;
  cache->mVectorDatasetGroupMagMinimum = mVectorDatasetMagMinimum;
  cache->mVectorDatasetGroupMagMaximum = mVectorDatasetMagMaximum;
  cache->mVectorDataOnVertices = mVectorDataOnVertices;
}

bool QgsMeshLayerRenderer::render()
{
  renderScalarDataset();
  renderMesh();
  renderVectorDataset();
  return true;
}

void QgsMeshLayerRenderer::renderMesh()
{
  if ( !mRendererSettings.nativeMeshSettings().isEnabled() &&
       !mRendererSettings.triangularMeshSettings().isEnabled() )
    return;

  // triangular mesh
  const QList<int> trianglesInExtent = mTriangularMesh.faceIndexesForRectangle( renderContext()->extent() );
  if ( mRendererSettings.triangularMeshSettings().isEnabled() )
  {
    renderMesh( mRendererSettings.triangularMeshSettings(),
                mTriangularMesh.triangles(),
                trianglesInExtent );
  }

  // native mesh
  if ( mRendererSettings.nativeMeshSettings().isEnabled() )
  {
    const QList<int> nativeFacesInExtent = QgsMeshUtils::nativeFacesFromTriangles( trianglesInExtent,
                                           mTriangularMesh.trianglesToNativeFaces() );
    renderMesh( mRendererSettings.nativeMeshSettings(),
                mNativeMesh.faces,
                nativeFacesInExtent );
  }
};

void QgsMeshLayerRenderer::renderMesh( const QgsMeshRendererMeshSettings &settings, const QVector<QgsMeshFace> &faces, const QList<int> &facesInExtent )
{
  Q_ASSERT( settings.isEnabled() );

  QgsRenderContext &context = *renderContext();
  // Set up the render configuration options
  QPainter *painter = context.painter();
  painter->save();
  if ( context.flags() & QgsRenderContext::Antialiasing )
    painter->setRenderHint( QPainter::Antialiasing, true );

  QPen pen = painter->pen();
  pen.setCapStyle( Qt::FlatCap );
  pen.setJoinStyle( Qt::MiterJoin );

  double penWidth = context.convertToPainterUnits( settings.lineWidth(),
                    QgsUnitTypes::RenderUnit::RenderMillimeters );
  pen.setWidthF( penWidth );
  pen.setColor( settings.color() );
  painter->setPen( pen );

  const QVector<QgsMeshVertex> &vertices = mTriangularMesh.vertices(); //Triangular mesh vertices contains also native mesh vertices
  QSet<QPair<int, int>> drawnEdges;

  for ( const int i : facesInExtent )
  {
    if ( context.renderingStopped() )
      break;

    const QgsMeshFace &face = faces[i];
    if ( face.size() < 2 )
      continue;

    for ( int j = 0; j < face.size(); ++j )
    {
      const int startVertexId = face[j];
      const int endVertexId = face[( j + 1 ) % face.size()];
      const QPair<int, int> thisEdge( startVertexId, endVertexId );
      const QPair<int, int> thisEdgeReversed( endVertexId, startVertexId );
      if ( drawnEdges.contains( thisEdge ) || drawnEdges.contains( thisEdgeReversed ) )
        continue;
      drawnEdges.insert( thisEdge );
      drawnEdges.insert( thisEdgeReversed );

      const QgsMeshVertex &startVertex = vertices[startVertexId];
      const QgsMeshVertex &endVertex = vertices[endVertexId];
      const QgsPointXY lineStart = context.mapToPixel().transform( startVertex.x(), startVertex.y() );
      const QgsPointXY lineEnd = context.mapToPixel().transform( endVertex.x(), endVertex.y() );
      painter->drawLine( lineStart.toQPointF(), lineEnd.toQPointF() );
    }
  }

  painter->restore();
}

void QgsMeshLayerRenderer::renderScalarDataset()
{
  if ( mScalarDatasetValues.isEmpty() )
    return; // activeScalarDataset == NO_ACTIVE_MESH_DATASET

  if ( std::isnan( mScalarDatasetMinimum ) || std::isnan( mScalarDatasetMaximum ) )
    return; // only NODATA values

  QgsMeshDatasetIndex index = mRendererSettings.activeScalarDataset();
  if ( !index.isValid() )
    return; // no shader

  QgsRenderContext &context = *renderContext();
  const QgsMeshRendererScalarSettings scalarSettings = mRendererSettings.scalarSettings( index.group() );
  QgsColorRampShader *fcn = new QgsColorRampShader( scalarSettings.colorRampShader() );
  QgsRasterShader *sh = new QgsRasterShader();
  sh->setRasterShaderFunction( fcn );  // takes ownership of fcn
  QgsMeshLayerInterpolator interpolator( mTriangularMesh,
                                         mScalarDatasetValues,
                                         mScalarActiveFaceFlagValues,
                                         mScalarDataOnVertices,
                                         context,
                                         mOutputSize );
  QgsSingleBandPseudoColorRenderer renderer( &interpolator, 0, sh );  // takes ownership of sh
  renderer.setClassificationMin( scalarSettings.classificationMinimum() );
  renderer.setClassificationMax( scalarSettings.classificationMaximum() );
  renderer.setOpacity( scalarSettings.opacity() );

  std::unique_ptr<QgsRasterBlock> bl( renderer.block( 0, context.extent(), mOutputSize.width(), mOutputSize.height(), mFeedback.get() ) );
  QImage img = bl->image();

  context.painter()->drawImage( 0, 0, img );
}

void QgsMeshLayerRenderer::renderVectorDataset()
{
  QgsMeshDatasetIndex index = mRendererSettings.activeVectorDataset();
  if ( !index.isValid() )
    return;

  if ( !mVectorDatasetValues.isValid() )
    return; // no data at all

  if ( std::isnan( mVectorDatasetMagMinimum ) || std::isnan( mVectorDatasetMagMaximum ) )
    return; // only NODATA values

  QgsMeshVectorRenderer renderer( mTriangularMesh,
                                  mVectorDatasetValues,
                                  mVectorDatasetValuesMag,
                                  mVectorDatasetMagMinimum,
                                  mVectorDatasetMagMaximum,
                                  mVectorDataOnVertices,
                                  mRendererSettings.vectorSettings( index.group() ),
                                  *renderContext(),
                                  mOutputSize );

  renderer.draw();
}
