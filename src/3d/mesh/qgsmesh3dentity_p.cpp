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

#include <Qt3DRender/QGeometryRenderer>

#include "qgsmeshlayer.h"
#include "qgs3dmapsettings.h"
#include "qgsmesh3dmaterial_p.h"



QgsMesh3dEntity::QgsMesh3dEntity( const Qgs3DMapSettings &map,
                                  const QgsTriangularMesh &triangularMesh,
                                  const QgsMesh3DSymbol *symbol )
  : mMapSettings( map )
  , mTriangularMesh( triangularMesh )
  , mSymbol( symbol->clone() )
{}

QgsMeshDataset3dEntity::QgsMeshDataset3dEntity(
  const Qgs3DMapSettings &map,
  const QgsTriangularMesh &triangularMesh,
  QgsMeshLayer *meshLayer,
  const QgsMesh3DSymbol *symbol )
  : QgsMesh3dEntity( map, triangularMesh, symbol ),
    mLayerRef( meshLayer )
{}

void QgsMesh3dEntity::build()
{
  buildGeometry();
  applyMaterial();
}

void QgsMeshDataset3dEntity::buildGeometry()
{
  if ( !layer() )
    return;

  Qt3DRender::QGeometryRenderer *mesh = new Qt3DRender::QGeometryRenderer;
  mesh->setGeometry( new QgsMeshDataset3dGeometry( mTriangularMesh, layer(), mMapSettings.temporalRange(), mMapSettings.origin(), mSymbol.get(), mesh ) );
  addComponent( mesh );
}

void QgsMeshDataset3dEntity::applyMaterial()
{
  if ( mSymbol->renderingStyle() == QgsMesh3DSymbol::ColorRamp2DRendering && layer() )
  {
    const QgsMeshRendererSettings rendererSettings = layer()->rendererSettings();
    const int datasetGroupIndex = rendererSettings.activeScalarDatasetGroup();
    if ( datasetGroupIndex >= 0 )
      mSymbol->setColorRampShader( rendererSettings.scalarSettings( datasetGroupIndex ).colorRampShader() );
  }
  QgsMesh3dMaterial *material = new QgsMesh3dMaterial( layer(), mMapSettings.temporalRange(), mMapSettings.origin(), mSymbol.get(), QgsMesh3dMaterial::ScalarDataSet );
  addComponent( material );
}

QgsMeshLayer *QgsMeshDataset3dEntity::layer() const
{
  return qobject_cast<QgsMeshLayer *>( mLayerRef.layer.data() );
}

QgsMesh3dTerrainTileEntity::QgsMesh3dTerrainTileEntity(
  const Qgs3DMapSettings &map,
  const QgsTriangularMesh &triangularMesh,
  const QgsMesh3DSymbol *symbol,
  QgsChunkNodeId nodeId,
  Qt3DCore::QNode *parent )
  : QgsTerrainTileEntity( nodeId, parent )
  , QgsMesh3dEntity( map, triangularMesh, symbol )
{}

void QgsMesh3dTerrainTileEntity::buildGeometry()
{
  Qt3DRender::QGeometryRenderer *mesh = new Qt3DRender::QGeometryRenderer;
  mesh->setGeometry( new QgsMeshTerrain3dGeometry( mTriangularMesh, mMapSettings.origin(), mSymbol.get()->verticalScale(), mesh ) );
  addComponent( mesh );
}

void QgsMesh3dTerrainTileEntity::applyMaterial()
{
  QgsMesh3dMaterial *material = new QgsMesh3dMaterial(
    nullptr, QgsDateTimeRange(),
    mMapSettings.origin(),
    mSymbol.get(),
    QgsMesh3dMaterial::ZValue );
  addComponent( material );
}

