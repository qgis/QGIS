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

#include "qgsmeshlayerrenderer.h"

#include "qgsfield.h"
#include "qgslogger.h"
#include "qgsmeshlayer.h"
#include "qgspointxy.h"
#include "qgsrenderer.h"
#include "qgssinglesymbolrenderer.h"
#include "qgssymbol.h"
#include "qgssinglebandpseudocolorrenderer.h"
#include "qgsrastershader.h"
#include "qgsmeshlayerinterpolator.h"
#include "qgsmeshlayerutils.h"
#include "qgsmeshvectorrenderer.h"
#include "qgsfillsymbollayer.h"
#include "qgssettings.h"
#include "qgsstyle.h"


QgsMeshLayerRenderer::QgsMeshLayerRenderer( QgsMeshLayer *layer, QgsRenderContext &context )
  : QgsMapLayerRenderer( layer->id() )
  , mFeedback( new QgsMeshLayerRendererFeedback )
  , mContext( context )
  , mRendererSettings( layer->rendererSettings() )
{
  // make copies for mesh data
  Q_ASSERT( layer->nativeMesh() );
  Q_ASSERT( layer->triangularMesh() );
  Q_ASSERT( layer->rendererCache() );
  Q_ASSERT( layer->dataProvider() );

  mNativeMesh = *( layer->nativeMesh() );
  mTriangularMesh = *( layer->triangularMesh() );

  createMeshSymbol( mNativeMeshSymbol, mRendererSettings.nativeMeshSettings() );
  createMeshSymbol( mTriangularMeshSymbol, mRendererSettings.triangularMeshSettings() );

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
  QgsRectangle extent = mContext.extent();  // this is extent in layer's coordinate system - but we need it in map coordinate system
  QgsMapToPixel mapToPixel = mContext.mapToPixel();
  QgsPointXY topleft = mapToPixel.transform( extent.xMinimum(), extent.yMaximum() );
  QgsPointXY bottomright = mapToPixel.transform( extent.xMaximum(), extent.yMinimum() );
  int width = int( bottomright.x() - topleft.x() );
  int height = int( bottomright.y() - topleft.y() );
  mOutputSize = QSize( width, height );
}

void QgsMeshLayerRenderer::createMeshSymbol( std::unique_ptr<QgsSymbol> &symbol,
    const QgsMeshRendererMeshSettings &settings )
{
  if ( settings.isEnabled() )
  {
    QgsSymbolLayerList l1;
    l1 << new QgsSimpleFillSymbolLayer( Qt::white,
                                        Qt::NoBrush,
                                        settings.color(),
                                        Qt::SolidLine,
                                        settings.lineWidth() );
    symbol.reset( new QgsFillSymbol( l1 ) );
  }
}


void QgsMeshLayerRenderer::copyScalarDatasetValues( QgsMeshLayer *layer )
{
  const QgsMeshDatasetIndex datasetIndex = mRendererSettings.activeScalarDataset();

  // Find out if we can use cache up to date. If yes, use it and return
  const int datasetGroupCount = layer->dataProvider()->datasetGroupCount();
  QgsMeshLayerRendererCache *cache = layer->rendererCache();
  if ( ( cache->mDatasetGroupsCount == datasetGroupCount ) &&
       ( cache->mActiveScalarDatasetIndex == datasetIndex ) )
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
    int count;
    if ( mScalarDataOnVertices )
      count = mNativeMesh.vertices.count();
    else
      count = mNativeMesh.faces.count();

    mScalarDatasetValues.resize( count );
    for ( int i = 0; i < count; ++i )
    {
      double v = layer->dataProvider()->datasetValue( datasetIndex, i ).scalar();
      mScalarDatasetValues[i] = v;
    }

    // populate face active flag, always defined on faces
    mScalarActiveFaceFlagValues.resize( mNativeMesh.faces.count() );
    for ( int i = 0; i < mNativeMesh.faces.count(); ++i )
    {
      bool active = layer->dataProvider()->isFaceActive( datasetIndex, i );
      mScalarActiveFaceFlagValues[i] = active;
    }

    QgsMeshLayerUtils::calculateMinimumMaximum( mScalarDatasetMinimum, mScalarDatasetMaximum, mScalarDatasetValues );
  }

  // update cache
  cache->mDatasetGroupsCount = datasetGroupCount;
  cache->mActiveScalarDatasetIndex = datasetIndex;
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
    mVectorDatasetValuesX = cache->mVectorDatasetValuesX;
    mVectorDatasetValuesY = cache->mVectorDatasetValuesY;
    mVectorDatasetValuesMag = cache->mVectorDatasetValuesMag;
    mVectorDatasetMagMinimum = cache->mVectorDatasetMagMinimum;
    mVectorDatasetMagMaximum = cache->mVectorDatasetMagMaximum;
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
      QgsDebugMsg( "Dataset has no vector values" );
    }
    else
    {
      mVectorDataOnVertices = metadata.dataType() == QgsMeshDatasetGroupMetadata::DataOnVertices;
      int count;
      if ( mVectorDataOnVertices )
        count = mNativeMesh.vertices.count();
      else
        count = mNativeMesh.faces.count();

      mVectorDatasetValuesX.resize( count );
      mVectorDatasetValuesY.resize( count );
      mVectorDatasetValuesMag.resize( count );
      for ( int i = 0; i < count; ++i )
      {
        double x = layer->dataProvider()->datasetValue( datasetIndex, i ).x();
        mVectorDatasetValuesX[i] = x;

        double y = layer->dataProvider()->datasetValue( datasetIndex, i ).y();
        mVectorDatasetValuesY[i] = y;

        double mag = layer->dataProvider()->datasetValue( datasetIndex, i ).scalar();
        mVectorDatasetValuesMag[i] = mag;
      }
    }

    QgsMeshLayerUtils::calculateMinimumMaximum( mVectorDatasetMagMinimum, mVectorDatasetMagMaximum, mVectorDatasetValuesMag );
  }

  // update cache
  cache->mDatasetGroupsCount = datasetGroupCount;
  cache->mActiveVectorDatasetIndex = datasetIndex;
  cache->mVectorDatasetValuesX = mVectorDatasetValuesX;
  cache->mVectorDatasetValuesY = mVectorDatasetValuesY;
  cache->mVectorDatasetValuesMag = mVectorDatasetValuesMag;
  cache->mVectorDatasetMagMinimum = mVectorDatasetMagMinimum;
  cache->mVectorDatasetMagMaximum = mVectorDatasetMagMaximum;
  cache->mVectorDataOnVertices = mVectorDataOnVertices;
}

bool QgsMeshLayerRenderer::render()
{

  renderScalarDataset();

  renderMesh( mNativeMeshSymbol, mNativeMesh.faces ); // native mesh
  renderMesh( mTriangularMeshSymbol, mTriangularMesh.triangles() ); // triangular mesh

  renderVectorDataset();

  return true;
}

void QgsMeshLayerRenderer::renderMesh( const std::unique_ptr<QgsSymbol> &symbol, const QVector<QgsMeshFace> &faces )
{
  if ( !symbol )
    return;

  QgsFields fields;
  QgsSingleSymbolRenderer renderer( symbol->clone() );
  renderer.startRender( mContext, fields );
  const QVector<QgsMeshVertex> &vertices = mTriangularMesh.vertices(); //Triangular mesh vertices contains also native mesh vertices

  for ( int i = 0; i < faces.size(); ++i )
  {
    if ( mContext.renderingStopped() )
      break;

    const QgsMeshFace &face = faces[i];
    QgsFeature feat;
    feat.setFields( fields );
    QgsGeometry geom = QgsMeshUtils::toGeometry( face, vertices );
    const QgsRectangle bbox = geom.boundingBox();
    if ( !mContext.extent().intersects( bbox ) )
      continue;

    feat.setGeometry( geom );
    renderer.renderFeature( feat, mContext );
  }

  renderer.stopRender( mContext );
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

  const QgsMeshRendererScalarSettings scalarSettings = mRendererSettings.scalarSettings( index.group() );
  QgsColorRampShader *fcn = new QgsColorRampShader( scalarSettings.colorRampShader() );
  QgsRasterShader *sh = new QgsRasterShader();
  sh->setRasterShaderFunction( fcn );  // takes ownership of fcn
  QgsMeshLayerInterpolator interpolator( mTriangularMesh, mScalarDatasetValues, mScalarActiveFaceFlagValues,
                                         mScalarDataOnVertices, mContext, mOutputSize );
  QgsSingleBandPseudoColorRenderer renderer( &interpolator, 0, sh );  // takes ownership of sh
  renderer.setClassificationMin( scalarSettings.classificationMinimum() );
  renderer.setClassificationMax( scalarSettings.classificationMaximum() );
  renderer.setOpacity( scalarSettings.opacity() );

  std::unique_ptr<QgsRasterBlock> bl( renderer.block( 0, mContext.extent(), mOutputSize.width(), mOutputSize.height(), mFeedback.get() ) );
  QImage img = bl->image();

  mContext.painter()->drawImage( 0, 0, img );
}

void QgsMeshLayerRenderer::renderVectorDataset()
{
  QgsMeshDatasetIndex index = mRendererSettings.activeVectorDataset();
  if ( !index.isValid() )
    return;

  if ( mVectorDatasetValuesX.isEmpty() )
    return;

  if ( std::isnan( mVectorDatasetMagMinimum ) || std::isnan( mVectorDatasetMagMaximum ) )
    return; // only NODATA values

  if ( mVectorDatasetValuesX.size() != mVectorDatasetValuesY.size() )
    return;

  QgsMeshVectorRenderer renderer( mTriangularMesh,
                                  mVectorDatasetValuesX, mVectorDatasetValuesY, mVectorDatasetValuesMag,
                                  mVectorDatasetMagMinimum, mVectorDatasetMagMaximum,
                                  mVectorDataOnVertices, mRendererSettings.vectorSettings( index.group() ), mContext, mOutputSize );

  renderer.draw();
}
