/***************************************************************************
  qgsdemterraingenerator.cpp
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

#include "qgsdemterraingenerator.h"

#include <limits>

#include "qgs3drendercontext.h"
#include "qgs3dutils.h"
#include "qgscoordinatetransform.h"
#include "qgsdemterraintilegeometry_p.h"
#include "qgsdemterraintileloader_p.h"
#include "qgsgeotransform.h"
#include "qgsrasterlayer.h"
#include "qgsterrainentity.h"

#include <QEntity>
#include <QFuture>
#include <QGeometryRenderer>

#include "moc_qgsdemterraingenerator.cpp"

QgsTerrainGenerator *QgsDemTerrainGenerator::create()
{
  return new QgsDemTerrainGenerator();
}

QgsDemTerrainGenerator::QgsDemTerrainGenerator() = default;

QgsDemTerrainGenerator::~QgsDemTerrainGenerator()
{}

void QgsDemTerrainGenerator::setLayer( QgsRasterLayer *layer )
{
  mLayer = layer;
  updateGenerator();
}

QgsRasterLayer *QgsDemTerrainGenerator::layer() const
{
  return mLayer;
}

void QgsDemTerrainGenerator::setCrs( const QgsCoordinateReferenceSystem &crs, const QgsCoordinateTransformContext &context )
{
  mCrs = crs;
  mTransformContext = context;
  updateGenerator();
}

QgsTerrainGenerator *QgsDemTerrainGenerator::clone() const
{
  QgsDemTerrainGenerator *cloned = new QgsDemTerrainGenerator;
  cloned->setTerrain( mTerrain );
  cloned->mCrs = mCrs;
  cloned->mLayer = mLayer;
  cloned->mResolution = mResolution;
  cloned->mSkirtHeight = mSkirtHeight;
  cloned->mExtent = mExtent;
  cloned->updateGenerator();
  return cloned;
}

QgsTerrainGenerator::Type QgsDemTerrainGenerator::type() const
{
  return QgsTerrainGenerator::Dem;
}

QgsRectangle QgsDemTerrainGenerator::rootChunkExtent() const
{
  return mTerrainTilingScheme.tileToExtent( 0, 0, 0 );
}

float QgsDemTerrainGenerator::heightAt( double x, double y, const Qgs3DRenderContext &context ) const
{
  Q_UNUSED( context )
  if ( mHeightMapGenerator )
    return mHeightMapGenerator->heightAt( x, y );
  else
    return std::numeric_limits<float>::quiet_NaN();
}

static void heightMapMinMax( const QByteArray &heightMap, float &zMin, float &zMax )
{
  const float *zBits = ( const float * ) heightMap.constData();
  int zCount = heightMap.count() / sizeof( float );
  bool first = true;

  zMin = zMax = std::numeric_limits<float>::quiet_NaN();
  for ( int i = 0; i < zCount; ++i )
  {
    float z = zBits[i];
    if ( std::isnan( z ) )
      continue;
    if ( first )
    {
      zMin = zMax = z;
      first = false;
    }
    zMin = std::min( zMin, z );
    zMax = std::max( zMax, z );
  }
}

QFuture<QgsChunkLoaderResult> QgsDemTerrainGenerator::loadChunk( QgsChunkNode *node )
{
  return QtFuture::whenAll( mHeightMapGenerator->render( node->tileId() ), loadTextureResources( node ) )
    // TODO(dvdkon): Create our own method that returns a tuple of values instead of list of futures? This is kind of stupid.
    .then( this, [this, node]( QList<std::variant<QFuture<QByteArray>, QFuture<QgsTerrainGenerator::TerrainTextureResources>>> results ) {
      QByteArray heightMap = std::get<0>( results[0] ).result();
      TerrainTextureResources textureResources = std::get<1>( results[1] ).result();
      return QgsChunkLoaderResult { [this, heightMap, node, textureResources]( Qt3DCore::QEntity *parent ) -> Qt3DCore::QEntity * {
        float zMin, zMax;
        heightMapMinMax( heightMap, zMin, zMax );

        if ( std::isnan( zMin ) || std::isnan( zMax ) )
        {
          // no data available for this tile
          return nullptr;
        }

        Qgs3DMapSettings *map = mTerrain->mapSettings();
        Qgs3DRenderContext context = Qgs3DRenderContext::fromMapSettings( map );
        QgsChunkNodeId nodeId = node->tileId();
        QgsRectangle extent = map->terrainGenerator()->tilingScheme().tileToExtent( nodeId );
        double side = extent.width();

        QgsTerrainTileEntity *entity = new QgsTerrainTileEntity( nodeId );

        // create geometry renderer

        Qt3DRender::QGeometryRenderer *mesh = new Qt3DRender::QGeometryRenderer;
        mesh->setGeometry( new DemTerrainTileGeometry( mResolution, side, map->terrainSettings()->verticalScale(), mSkirtHeight, heightMap, mesh ) );
        entity->addComponent( mesh ); // takes ownership if the component has no parent

        // create material

        createTextureComponent( textureResources, entity, map->isTerrainShadingEnabled(), map->terrainShadingMaterial(), !map->layers().empty(), context );

        // create transform
        QgsGeoTransform *transform = new QgsGeoTransform;
        transform->setGeoTranslation( QgsVector3D( extent.xMinimum(), extent.yMinimum(), 0 ) );
        entity->addComponent( transform );

        // clang-format off
          node->setExactBox3D(
            QgsBox3D( extent.xMinimum(), extent.yMinimum(), zMin * map->terrainSettings()->verticalScale(),
                      extent.xMinimum() + side, extent.yMinimum() + side, zMax * map->terrainSettings()->verticalScale() )
          );
        // clang-format on
        node->updateParentBoundingBoxesRecursively();

        entity->setParent( parent );
        return entity;
      } };
    } );
}

void QgsDemTerrainGenerator::setExtent( const QgsRectangle &extent )
{
  if ( mExtent == extent )
    return;

  mExtent = extent;
  updateGenerator();
}

void QgsDemTerrainGenerator::updateGenerator()
{
  QgsRasterLayer *dem = layer();
  if ( dem && mCrs.isValid() )
  {
    QgsRectangle layerExtent = Qgs3DUtils::tryReprojectExtent2D( mLayer->extent(), mLayer->crs(), mCrs, mTransformContext );
    // no need to have an mExtent larger than the actual layer's extent
    const QgsRectangle intersectExtent = mExtent.intersect( layerExtent );

    mTerrainTilingScheme = QgsTilingScheme( intersectExtent, mCrs );
    mHeightMapGenerator = std::make_unique<QgsDemHeightMapGenerator>( dem, mTerrainTilingScheme, mResolution, mTransformContext );

    mIsValid = true;
  }
  else
  {
    mTerrainTilingScheme = QgsTilingScheme();
    mHeightMapGenerator.reset();
    mIsValid = false;
  }
}

QgsTerrainGenerator::Capabilities QgsDemTerrainGenerator::capabilities() const
{
  return QgsTerrainGenerator::Capability::SupportsTileResolution;
}
