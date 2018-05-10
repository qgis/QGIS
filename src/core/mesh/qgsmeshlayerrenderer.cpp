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

QgsMeshLayerRenderer::QgsMeshLayerRenderer( QgsMeshLayer *layer, QgsRenderContext &context )
  : QgsMapLayerRenderer( layer->id() )
  , mContext( context )
{
  // make copies for mesh data
  Q_ASSERT( layer->nativeMesh() );
  Q_ASSERT( layer->triangularMesh() );
  mNativeMesh = *( layer->nativeMesh() );
  mTriangularMesh = *( layer->triangularMesh() );

  // make copies for symbols
  if ( layer->nativeMeshSymbol() )
  {
    mNativeMeshSymbol.reset( layer->nativeMeshSymbol()->clone() );
  }

  if ( layer->triangularMeshSymbol() )
  {
    mTriangularMeshSymbol.reset( layer->triangularMeshSymbol()->clone() );
  }

  copyScalarDatasetValues( layer );
}


void QgsMeshLayerRenderer::copyScalarDatasetValues( QgsMeshLayer *layer )
{
  int datasetIndex = layer->activeScalarDataset();
  if ( datasetIndex != -1 )
  {
    mDataOnVertices = layer->dataProvider()->datasetIsOnVertices( datasetIndex );
    if ( mDataOnVertices )
    {
      int count = mNativeMesh.vertices.count();
      mDatasetValues.resize( count );
      for ( int i = 0; i < count; ++i )
      {
        double v = layer->dataProvider()->datasetValue( datasetIndex, i ).scalar();
        mDatasetValues[i] = v;
      }
    }
    else
    {
      //on faces
      int count = mNativeMesh.faces.count();
      mDatasetValues.resize( count );
      for ( int i = 0; i < count; ++i )
      {
        double v = layer->dataProvider()->datasetValue( datasetIndex, i ).scalar();
        mDatasetValues[i] = v;
      }
    }
  }
}


bool QgsMeshLayerRenderer::render()
{
  renderScalarDataset();

  renderMesh( mNativeMeshSymbol, mNativeMesh.faces ); // native mesh
  renderMesh( mTriangularMeshSymbol, mTriangularMesh.triangles() ); // triangular mesh

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
  if ( mDatasetValues.isEmpty() )
    return;

  // figure out image size
  QgsRectangle extent = mContext.extent();  // this is extent in layer's coordinate system - but we need it in map coordinate system
  QgsMapToPixel mapToPixel = mContext.mapToPixel();
  // TODO: what if OTF reprojection is used - see crayfish layer_renderer.py (_calculate_extent)
  QgsPointXY topleft = mapToPixel.transform( extent.xMinimum(), extent.yMaximum() );
  QgsPointXY bottomright = mapToPixel.transform( extent.xMaximum(), extent.yMinimum() );
  int width = bottomright.x() - topleft.x();
  int height = bottomright.y() - topleft.y();

  double vMin = mDatasetValues[0], vMax = mDatasetValues[0];
  for ( int i = 1; i < mDatasetValues.count(); ++i )
  {
    double v = mDatasetValues[i];
    if ( v < vMin ) vMin = v;
    if ( v > vMax ) vMax = v;
  }

  QList<QgsColorRampShader::ColorRampItem> lst;
  lst << QgsColorRampShader::ColorRampItem( vMin, Qt::red, QString::number( vMin ) );
  lst << QgsColorRampShader::ColorRampItem( vMax, Qt::blue, QString::number( vMax ) );

  QgsColorRampShader *fcn = new QgsColorRampShader( vMin, vMax );
  fcn->setColorRampItemList( lst );
  QgsRasterShader *sh = new QgsRasterShader( 0, 1000 );
  sh->setRasterShaderFunction( fcn );  // takes ownership of fcn
  QgsMeshLayerInterpolator interpolator( mTriangularMesh, mDatasetValues, mDataOnVertices, mContext );
  QgsSingleBandPseudoColorRenderer r( &interpolator, 0, sh );  // takes ownership of sh
  QgsRasterBlock *bl = r.block( 0, extent, width, height );  // TODO: feedback
  Q_ASSERT( bl );

  QImage img = bl->image();

  mContext.painter()->drawImage( 0, 0, img );
  delete bl;
}
