/***************************************************************************
                         qgsmesh3dentity.cpp
                         -------------------------
    begin                : january 2020
    copyright            : (C) 2020 by Vincent Cloarec
    email                : vcloarec at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsmesh3dentity_p.h"
#include "moc_qgsmesh3dentity_p.cpp"

#include <Qt3DRender/QGeometryRenderer>

#include "qgsmeshlayer.h"
#include "qgs3drendercontext.h"
#include "qgsmesh3dmaterial_p.h"
#include "qgsgeotransform.h"


QgsMesh3DEntity::QgsMesh3DEntity( const Qgs3DRenderContext &context, const QgsTriangularMesh &triangularMesh, const QgsMesh3DSymbol *symbol )
  : mRenderContext( context )
  , mTriangularMesh( triangularMesh )
  , mSymbol( symbol->clone() )
{}

QgsMeshDataset3DEntity::QgsMeshDataset3DEntity(
  const Qgs3DRenderContext &context,
  const QgsTriangularMesh &triangularMesh,
  QgsMeshLayer *meshLayer,
  const QgsMesh3DSymbol *symbol
)
  : QgsMesh3DEntity( context, triangularMesh, symbol ), mLayerRef( meshLayer )
{}

void QgsMesh3DEntity::build()
{
  buildGeometry();
  applyMaterial();
}

void QgsMeshDataset3DEntity::buildGeometry()
{
  if ( !layer() )
    return;

  QgsPointXY meshCenter = mTriangularMesh.extent().center();
  QgsVector3D chunkOrigin( meshCenter.x(), meshCenter.y(), 0 );

  Qt3DRender::QGeometryRenderer *mesh = new Qt3DRender::QGeometryRenderer;
  mesh->setGeometry( new QgsMeshDataset3DGeometry( mTriangularMesh, layer(), mRenderContext.temporalRange(), chunkOrigin, mRenderContext.extent(), mSymbol.get(), mesh ) );
  addComponent( mesh );

  QgsGeoTransform *transform = new QgsGeoTransform;
  transform->setGeoTranslation( chunkOrigin );
  addComponent( transform );
}

void QgsMeshDataset3DEntity::applyMaterial()
{
  if ( mSymbol->renderingStyle() == QgsMesh3DSymbol::RenderingStyle::ColorRamp2DRendering && layer() )
  {
    const QgsMeshRendererSettings rendererSettings = layer()->rendererSettings();
    const int datasetGroupIndex = rendererSettings.activeScalarDatasetGroup();
    if ( datasetGroupIndex >= 0 )
      mSymbol->setColorRampShader( rendererSettings.scalarSettings( datasetGroupIndex ).colorRampShader() );
  }
  QgsMesh3DMaterial *material = new QgsMesh3DMaterial( layer(), mRenderContext.temporalRange(), mRenderContext.origin(), mSymbol.get(), QgsMesh3DMaterial::ScalarDataSet );
  addComponent( material );
}

QgsMeshLayer *QgsMeshDataset3DEntity::layer() const
{
  return qobject_cast<QgsMeshLayer *>( mLayerRef.layer.data() );
}

QgsMesh3DTerrainTileEntity::QgsMesh3DTerrainTileEntity( const Qgs3DRenderContext &context, const QgsTriangularMesh &triangularMesh, const QgsMesh3DSymbol *symbol, QgsChunkNodeId nodeId, Qt3DCore::QNode *parent )
  : QgsTerrainTileEntity( nodeId, parent )
  , QgsMesh3DEntity( context, triangularMesh, symbol )
{}

void QgsMesh3DTerrainTileEntity::buildGeometry()
{
  QgsPointXY meshCenter = mTriangularMesh.extent().center();
  QgsVector3D chunkOrigin( meshCenter.x(), meshCenter.y(), 0 );

  Qt3DRender::QGeometryRenderer *mesh = new Qt3DRender::QGeometryRenderer;
  mesh->setGeometry( new QgsMeshTerrain3DGeometry( mTriangularMesh, chunkOrigin, mRenderContext.extent(), mSymbol.get()->verticalScale(), mesh ) );
  addComponent( mesh );

  QgsGeoTransform *transform = new QgsGeoTransform;
  transform->setGeoTranslation( chunkOrigin );
  addComponent( transform );
}

void QgsMesh3DTerrainTileEntity::applyMaterial()
{
  QgsMesh3DMaterial *material = new QgsMesh3DMaterial(
    nullptr, QgsDateTimeRange(),
    mRenderContext.origin(),
    mSymbol.get(),
    QgsMesh3DMaterial::ZValue
  );
  addComponent( material );
}
