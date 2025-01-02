/***************************************************************************
  qgsflatterraingenerator.cpp
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

#include "qgsflatterraingenerator.h"
#include "moc_qgsflatterraingenerator.cpp"

#include <Qt3DRender/QGeometryRenderer>

#include "qgs3dmapsettings.h"
#include "qgschunknode.h"
#include "qgsgeotransform.h"
#include "qgsterrainentity.h"
#include "qgsterraintileentity_p.h"

/// @cond PRIVATE


//---------------


FlatTerrainChunkLoader::FlatTerrainChunkLoader( QgsTerrainEntity *terrain, QgsChunkNode *node )
  : QgsTerrainTileLoader( terrain, node )
{
  loadTexture();
}


Qt3DCore::QEntity *FlatTerrainChunkLoader::createEntity( Qt3DCore::QEntity *parent )
{
  QgsTerrainTileEntity *entity = new QgsTerrainTileEntity( mNode->tileId() );

  // make geometry renderer

  // simple quad geometry shared by all tiles
  // QPlaneGeometry by default is 1x1 with mesh resolution QSize(2,2), centered at 0
  // TODO: the geometry could be shared inside Terrain instance (within terrain-generator specific data?)
  mTileGeometry = new Qt3DExtras::QPlaneGeometry;

  Qt3DRender::QGeometryRenderer *mesh = new Qt3DRender::QGeometryRenderer;
  mesh->setGeometry( mTileGeometry ); // takes ownership if the component has no parent
  entity->addComponent( mesh );       // takes ownership if the component has no parent

  // create material

  const Qgs3DMapSettings *map = terrain()->mapSettings();

  // create transform

  QgsGeoTransform *transform = nullptr;
  transform = new QgsGeoTransform();
  entity->addComponent( transform );

  // set up transform according to the extent covered by the quad geometry
  const QgsBox3D box3D = mNode->box3D();
  const QgsBox3D mapFullBox3D( map->extent(), box3D.zMinimum(), box3D.zMaximum() );

  const QgsBox3D commonExtent( std::max( box3D.xMinimum(), mapFullBox3D.xMinimum() ), std::max( box3D.yMinimum(), mapFullBox3D.yMinimum() ), box3D.zMinimum(), std::min( box3D.xMaximum(), mapFullBox3D.xMaximum() ), std::min( box3D.yMaximum(), mapFullBox3D.yMaximum() ), box3D.zMaximum() );
  const double xSide = commonExtent.width();
  const double ySide = commonExtent.height();
  const double xMin = commonExtent.xMinimum();
  const double yMin = commonExtent.yMinimum();

  transform->setRotation( QQuaternion::fromAxisAndAngle( QVector3D( 1, 0, 0 ), 90 ) ); // QPlaneGeometry uses XZ as the base plane
  transform->setScale3D( QVector3D( static_cast<float>( xSide ), 1, static_cast<float>( ySide ) ) );
  transform->setGeoTranslation( QgsVector3D( xMin + xSide / 2, yMin + ySide / 2, 0 ) );

  createTextureComponent( entity, map->isTerrainShadingEnabled(), map->terrainShadingMaterial(), !map->layers().empty() );

  entity->setParent( parent );
  return entity;
}

/// @endcond

// ---------------

QgsTerrainGenerator *QgsFlatTerrainGenerator::create()
{
  return new QgsFlatTerrainGenerator();
}

QgsChunkLoader *QgsFlatTerrainGenerator::createChunkLoader( QgsChunkNode *node ) const
{
  return new FlatTerrainChunkLoader( mTerrain, node );
}

QgsTerrainGenerator *QgsFlatTerrainGenerator::clone() const
{
  QgsFlatTerrainGenerator *cloned = new QgsFlatTerrainGenerator;
  cloned->mCrs = mCrs;
  cloned->mExtent = mExtent;
  cloned->updateTilingScheme();
  return cloned;
}

QgsTerrainGenerator::Type QgsFlatTerrainGenerator::type() const
{
  return QgsTerrainGenerator::Flat;
}

QgsRectangle QgsFlatTerrainGenerator::rootChunkExtent() const
{
  return mTerrainTilingScheme.tileToExtent( 0, 0, 0 );
}

void QgsFlatTerrainGenerator::rootChunkHeightRange( float &hMin, float &hMax ) const
{
  hMin = 0;
  hMax = 0;
}

void QgsFlatTerrainGenerator::setCrs( const QgsCoordinateReferenceSystem &crs, const QgsCoordinateTransformContext & )
{
  mCrs = crs;
  updateTilingScheme();
}

void QgsFlatTerrainGenerator::setExtent( const QgsRectangle &extent )
{
  if ( mExtent == extent )
    return;

  mExtent = extent;
  updateTilingScheme();
}

void QgsFlatTerrainGenerator::updateTilingScheme()
{
  // the real extent will be a square where the given extent fully fits
  mTerrainTilingScheme = QgsTilingScheme( mExtent, mCrs );
}
