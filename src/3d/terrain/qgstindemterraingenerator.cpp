/***************************************************************************
  qgistindemterraingenerator.cpp
  --------------------------------------
  Date                 : october 2019
  Copyright            : (C) 2019 by Vincent Cloarec
  Email                : vcloarec at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QEntity>
#include <Qt3DRender/QGeometryRenderer>
#include <Qt3DExtras/qspheregeometry.h>


#include <Qt3DExtras/QPhongMaterial>
#include <Qt3DExtras/QTextureMaterial>
#include <Qt3DExtras/QTorusMesh>
#include <Qt3DExtras/QMetalRoughMaterial>

#include "qgschunknode_p.h"
#include "qgstindemterraingenerator.h"
#include "qgsterrainentity_p.h"
#include "qgs3dmapsettings.h"
#include "qgstriangularmesh.h"
#include "qgsterraintileentity_p.h"

void QgsTinDemTerrainGenerator::setLayer( QgsMeshLayer *layer )
{
  mLayer = layer;
  updateTilingScheme();
}

QgsChunkLoader *QgsTinDemTerrainGenerator::createChunkLoader( QgsChunkNode *node ) const
{
  return new QgsTinDemTerrainTileLoader( mTerrain, node, mLayer );
}

QgsTerrainGenerator *QgsTinDemTerrainGenerator::clone() const
{
  return new QgsTinDemTerrainGenerator;
}

QgsTerrainGenerator::Type QgsTinDemTerrainGenerator::type() const
{
  return QgsTerrainGenerator::Tin;
}

QgsRectangle QgsTinDemTerrainGenerator::extent() const
{
  if ( mLayer )
    return mLayer->extent();
  else
    return QgsRectangle();
}

void QgsTinDemTerrainGenerator::updateTilingScheme()
{
  if ( mLayer )
    mTerrainTilingScheme = QgsTilingScheme( mLayer->extent(), mLayer->crs() );
  else
    mTerrainTilingScheme = QgsTilingScheme();
}

QgsTinDemTerrainTileLoader::QgsTinDemTerrainTileLoader( QgsTerrainEntity *terrain, QgsChunkNode *node, QgsMeshLayer *layer ):
  QgsTerrainTileLoader( terrain, node ), mLayer( layer ),
  mMeshTile( layer->triangularMesh(),
             terrain->map3D().terrainGenerator()->tilingScheme().
             tileToExtent( node->tileX(),
                           node->tileY(),
                           node->tileZ() ) )
{
  setExtentMapCrs( terrain->terrainToMapTransform().transformBoundingBox( mMeshTile.realTileExtent() ) );
  loadTexture();
}

Qt3DCore::QEntity *QgsTinDemTerrainTileLoader::createEntity( Qt3DCore::QEntity *parent )
{
  QgsTerrainTileEntity *entity = new QgsTerrainTileEntity( parent );

  const Qgs3DMapSettings &map = terrain()->map3D();
  QgsRectangle extent = map.terrainGenerator()->tilingScheme().tileToExtent( mNode->tileX(), mNode->tileY(), mNode->tileZ() ); //node->extent;


  auto triangularMesh = mLayer->triangularMesh();
  QList<int> faces = triangularMesh->faceIndexesForRectangle( extent );
  Qt3DRender::QGeometryRenderer *mesh = new Qt3DRender::QGeometryRenderer;

  mesh->setGeometry( new QgsTinDemTerrainTileGeometry_p( mMeshTile, float( map.terrainVerticalScale() ), entity ) );

  createTextureComponent( entity, map.isTerrainShadingEnabled(), map.terrainShadingMaterial() );

  double x0 =  map.origin().x();
  double y0 =  -map.origin().y();

  Qt3DCore::QTransform *transform = new Qt3DCore::QTransform();
  entity->addComponent( transform );
  transform->setTranslation( QVector3D( float( -x0 ), 0, float( -y0 ) ) );

  mNode->setExactBbox( QgsAABB( float( extent.xMinimum() - x0 ),
                                mMeshTile.zMinimum()*float( map.terrainVerticalScale() ),
                                float( -extent.yMinimum() - y0 ),
                                float( extent.xMaximum() - x0 ),
                                mMeshTile.zMaximum()*float( map.terrainVerticalScale() ),
                                float( -extent.yMaximum() - y0 ) ) );

  entity->addComponent( mesh );
  entity->setEnabled( false );
  entity->setParent( parent );

  return entity;
}
