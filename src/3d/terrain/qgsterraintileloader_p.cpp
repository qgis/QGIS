/***************************************************************************
  qgsterraintileloader_p.cpp
  --------------------------------------
  Date                 : July 2017
  Copyright            : (C) 2017 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsterraintileloader_p.h"

#include "qgs3dmapsettings.h"
#include "qgschunknode_p.h"
#include "qgsterrainentity_p.h"
#include "qgsterraingenerator.h"
#include "qgsterraintextureimage_p.h"
#include "qgsterraintexturegenerator_p.h"
#include "qgsterraintileentity_p.h"

#include <Qt3DRender/QTexture>

#if QT_VERSION >= 0x050900
#include <Qt3DExtras/QTextureMaterial>
#else
#include <Qt3DExtras/QDiffuseMapMaterial>
#endif

#include "quantizedmeshterraingenerator.h"

/// @cond PRIVATE

QgsTerrainTileLoader::QgsTerrainTileLoader( QgsTerrainEntity *terrain, QgsChunkNode *node )
  : QgsChunkLoader( node )
  , mTerrain( terrain )
{
  const Qgs3DMapSettings &map = mTerrain->map3D();
  int tx, ty, tz;
#if 0
  if ( map.terrainGenerator->type() == TerrainGenerator::QuantizedMesh )
  {
    // TODO: sort out - should not be here
    QuantizedMeshTerrainGenerator *generator = static_cast<QuantizedMeshTerrainGenerator *>( map.terrainGenerator.get() );
    generator->quadTreeTileToBaseTile( node->x, node->y, node->z, tx, ty, tz );
  }
  else
#endif
  {
    tx = node->tileX();
    ty = node->tileY();
    tz = node->tileZ();
  }

  QgsRectangle extentTerrainCrs = map.terrainGenerator()->tilingScheme().tileToExtent( tx, ty, tz );
  mExtentMapCrs = terrain->terrainToMapTransform().transformBoundingBox( extentTerrainCrs );
  mTileDebugText = QStringLiteral( "%1 | %2 | %3" ).arg( tx ).arg( ty ).arg( tz );
}

void QgsTerrainTileLoader::loadTexture()
{
  connect( mTerrain->textureGenerator(), &QgsTerrainTextureGenerator::tileReady, this, &QgsTerrainTileLoader::onImageReady );
  mTextureJobId = mTerrain->textureGenerator()->render( mExtentMapCrs, mTileDebugText );
}

void QgsTerrainTileLoader::createTextureComponent( QgsTerrainTileEntity *entity )
{
  Qt3DRender::QTexture2D *texture = new Qt3DRender::QTexture2D( entity );
  QgsTerrainTextureImage *textureImage = new QgsTerrainTextureImage( mTextureImage, mExtentMapCrs, mTileDebugText );
  texture->addTextureImage( textureImage );
  texture->setMinificationFilter( Qt3DRender::QTexture2D::Linear );
  texture->setMagnificationFilter( Qt3DRender::QTexture2D::Linear );
  Qt3DExtras::QTextureMaterial *material = nullptr;
#if QT_VERSION >= 0x050900
  material = new Qt3DExtras::QTextureMaterial;
  material->setTexture( texture );
#else
  material = new Qt3DExtras::QDiffuseMapMaterial;
  material->setDiffuse( texture );
  material->setShininess( 1 );
  material->setAmbient( Qt::white );
#endif
  entity->setTextureImage( textureImage );
  entity->addComponent( material ); // takes ownership if the component has no parent
}

void QgsTerrainTileLoader::onImageReady( int jobId, const QImage &image )
{
  if ( mTextureJobId == jobId )
  {
    mTextureImage = image;
    mTextureJobId = -1;
    emit finished();  // TODO: this should be left for derived class!
  }
}

/// @endcond
