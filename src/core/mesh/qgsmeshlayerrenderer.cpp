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
#include "qgsmeshvectorrenderer.h"
#include "qgsfillsymbollayer.h"



QgsMeshLayerRenderer::QgsMeshLayerRenderer( QgsMeshLayer *layer, QgsRenderContext &context )
  : QgsMapLayerRenderer( layer->id() )
  , mFeedback( new QgsMeshLayerRendererFeedback )
  , mContext( context )
  , mRendererNativeMeshSettings( layer->rendererNativeMeshSettings() )
  , mRendererTriangularMeshSettings( layer->rendererTriangularMeshSettings() )
  , mRendererScalarSettings( layer-> rendererScalarSettings() )
  , mRendererVectorSettings( layer-> rendererVectorSettings() )
{
  // make copies for mesh data
  Q_ASSERT( layer->nativeMesh() );
  Q_ASSERT( layer->triangularMesh() );
  mNativeMesh = *( layer->nativeMesh() );
  mTriangularMesh = *( layer->triangularMesh() );

  createMeshSymbol( mNativeMeshSymbol, mRendererNativeMeshSettings );
  createMeshSymbol( mTriangularMeshSymbol, mRendererTriangularMeshSettings );

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
  int datasetIndex = layer->activeScalarDataset();
  if ( datasetIndex != NO_ACTIVE_MESH_DATASET )
  {
    const QgsMeshDatasetMetadata metadata = layer->dataProvider()->datasetMetadata( datasetIndex );
    mScalarDataOnVertices = metadata.isOnVertices();
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
  }
}

void QgsMeshLayerRenderer::copyVectorDatasetValues( QgsMeshLayer *layer )
{
  int datasetIndex = layer->activeVectorDataset();
  if ( datasetIndex != NO_ACTIVE_MESH_DATASET )
  {
    const QgsMeshDatasetMetadata metadata = layer->dataProvider()->datasetMetadata( datasetIndex );

    bool isScalar = metadata.isScalar();
    if ( isScalar )
    {
      QgsDebugMsg( "Dataset has no vector values" );
    }
    else
    {
      mVectorDataOnVertices = metadata.isOnVertices();
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
  }
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

  for ( int i = 0; i < faces.size(); ++i )
  {
    if ( mContext.renderingStopped() )
      break;

    const QgsMeshFace &face = faces[i];
    QgsFeature feat;
    feat.setFields( fields );
    QVector<QgsPointXY> ring;
    for ( int j = 0; j < face.size(); ++j )
    {
      int vertex_id = face[j];
      Q_ASSERT( vertex_id < mTriangularMesh.vertices().size() ); //Triangular mesh vertices contains also native mesh vertices
      const QgsPoint &vertex = mTriangularMesh.vertices()[vertex_id];
      ring.append( vertex );
    }
    QgsPolygonXY polygon;
    polygon.append( ring );
    QgsGeometry geom = QgsGeometry::fromPolygonXY( polygon );
    feat.setGeometry( geom );
    renderer.renderFeature( feat, mContext );
  }

  renderer.stopRender( mContext );
}

void QgsMeshLayerRenderer::renderScalarDataset()
{
  if ( mScalarDatasetValues.isEmpty() )
    return;

  // do not render if magnitude is outside of the filtered range (if filtering is enabled)
  double vMin = mRendererScalarSettings.minValue();
  if ( std::isnan( vMin ) )
    vMin = *std::min_element( mScalarDatasetValues.constBegin(), mScalarDatasetValues.constEnd() );


  double vMax = mRendererScalarSettings.maxValue();
  if ( std::isnan( vMax ) )
    vMax = *std::max_element( mScalarDatasetValues.constBegin(), mScalarDatasetValues.constEnd() );

  QList<QgsColorRampShader::ColorRampItem> lst;
  lst << QgsColorRampShader::ColorRampItem( vMin, mRendererScalarSettings.minColor(), QString::number( vMin ) );
  lst << QgsColorRampShader::ColorRampItem( vMax, mRendererScalarSettings.maxColor(), QString::number( vMax ) );

  QgsColorRampShader *fcn = new QgsColorRampShader( vMin, vMax );
  fcn->setColorRampItemList( lst );
  QgsRasterShader *sh = new QgsRasterShader( vMin, vMax );
  sh->setRasterShaderFunction( fcn );  // takes ownership of fcn
  QgsMeshLayerInterpolator interpolator( mTriangularMesh, mScalarDatasetValues, mScalarDataOnVertices, mContext, mOutputSize );
  QgsSingleBandPseudoColorRenderer r( &interpolator, 0, sh );  // takes ownership of sh
  QgsRasterBlock *bl = r.block( 0, mContext.extent(), mOutputSize.width(), mOutputSize.height(), mFeedback.get() );
  Q_ASSERT( bl );

  QImage img = bl->image();

  mContext.painter()->drawImage( 0, 0, img );
  delete bl;
}

void QgsMeshLayerRenderer::renderVectorDataset()
{
  if ( mVectorDatasetValuesX.isEmpty() )
    return;

  if ( mVectorDatasetValuesX.size() != mVectorDatasetValuesY.size() )
    return;

  QgsMeshVectorRenderer renderer( mTriangularMesh,
                                  mVectorDatasetValuesX, mVectorDatasetValuesY, mVectorDatasetValuesMag,
                                  mVectorDataOnVertices, mRendererVectorSettings, mContext, mOutputSize );

  renderer.draw();
}
