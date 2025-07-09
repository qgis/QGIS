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
#include "moc_qgsdemterraingenerator.cpp"

#include "qgsdemterraintileloader_p.h"

#include "qgs3dutils.h"
#include "qgsrasterlayer.h"
#include "qgscoordinatetransform.h"

QgsTerrainGenerator *QgsDemTerrainGenerator::create()
{
  return new QgsDemTerrainGenerator();
}

QgsDemTerrainGenerator::~QgsDemTerrainGenerator()
{
  delete mHeightMapGenerator;
}

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

void QgsDemTerrainGenerator::onHeightMapReceived( int, const QgsChunkNode *node, const QgsRectangle &extent, const QByteArray &heightMap )
{
  QMutexLocker locker( &mRootNodeMutex );

  const QgsChunkNodeId nodeId = node->tileId();

  // save height map data in cache
  mLoaderMap[nodeId.text()] = heightMap;
  cleanupHeightMapCache( node );
  //  if ( tileId.d >= mMaxLevel / 2 )
  {
    // emit only when tile is at the deepest level to update entity elevation with the hi-res tiles
    emit maxResTileReceived( nodeId, extent );
  }
}

void QgsDemTerrainGenerator::cleanupHeightMapCache( const QgsChunkNode *currentNode ) const
{
  // check if all sibling are ok then delete parent height map in cache
  QgsChunkNode *parent = currentNode->parent();

  if ( parent && mLoaderMap.contains( parent->tileId().text() ) )
  {
    QgsChunkNode *const *children = parent->children();
    bool deleteParentData = true;
    for ( int i = 0; i < parent->childCount(); ++i )
    {
      // if QgsChunkNode state is Loading, this means that the texture has been retrieved
      // C VRAI CA ???
      if ( children[i]->state() != QgsChunkNode::Loading && children[i]->state() != QgsChunkNode::Loaded )
      {
        deleteParentData = false;
        break;
      }
    }

    if ( deleteParentData )
    {
      mLoaderMap.remove( parent->tileId().text() );
    }
  }
}

float QgsDemTerrainGenerator::heightAt( double x, double y, const Qgs3DRenderContext &context ) const
{
  Q_UNUSED( context )
  if ( mRootNode != nullptr )
  {
    QMutexLocker locker( &mRootNodeMutex );

    // search the best chunk node for x/y
    QgsPoint searched( x, y );
    QgsChunkNode *current = mRootNode;
    QgsChunkNode *found = mRootNode;
    while ( current->hasChildrenPopulated() )
    {
      QgsChunkNode *const *children = current->children();
      bool deeper = false;
      for ( int i = 0; !deeper && i < current->childCount(); ++i )
      {
        if ( children[i]->box3D().contains( searched ) )
        {
          found = children[i];
          current = children[i];
          deeper = true;
        }
      }
      if ( !deeper )
      {
        break;
      }
    }

    QString foundKey = found->tileId().text();

    // search in cache if we know this chunk node/tile
    const char *heightMapData = nullptr;
    if ( mLoaderMap.contains( foundKey ) )
    {
      heightMapData = mLoaderMap[foundKey].constData();
    }
    else
    {
      // try to load height map from the current tile loader
      QgsDemTerrainTileLoader *loader = dynamic_cast<QgsDemTerrainTileLoader *>( found->loader() );
      if ( loader && !loader->heightMap().isEmpty() )
      {
        mLoaderMap.insert( foundKey, loader->heightMap() );
        cleanupHeightMapCache( found );
        heightMapData = loader->heightMap();
      }
      else // no load in current tile, check for map in parents
      {
        QgsChunkNode *parent = found->parent();
        while ( parent )
        {
          loader = dynamic_cast<QgsDemTerrainTileLoader *>( parent->loader() );
          if ( loader && !loader->heightMap().isEmpty() )
          {
            heightMapData = loader->heightMap();
            break;
          }
          else
            parent = parent->parent();
        }
      }
    }

    // retrieve height from extracted heightMapData. We do not save parent height map in the cache
    if ( heightMapData )
    {
      const QgsRectangle extent = found->box3D().toRectangle();

      int cellX = ( int ) ( ( x - extent.xMinimum() ) / extent.width() * mResolution + .5f );
      int cellY = ( int ) ( ( extent.yMaximum() - y ) / extent.height() * mResolution + .5f );
      cellX = std::clamp( cellX, 0, mResolution - 1 );
      cellY = std::clamp( cellY, 0, mResolution - 1 );

      const float *data = ( const float * ) heightMapData;

      return data[cellX + cellY * mResolution];
    }
  }

  // if we have no rootNode or if we were not able to find better height map data, we fallback to coarse DEM data
  if ( mHeightMapGenerator )
  {
    return mHeightMapGenerator->heightAt( x, y );
  }
  else
  {
    return 0;
  }
}

QgsChunkLoader *QgsDemTerrainGenerator::createChunkLoader( QgsChunkNode *node ) const
{
  // save root node for futur height map searches
  if ( node->parent() == nullptr )
  {
    QMutexLocker locker( &mRootNodeMutex );
    mRootNode = node;
  }

  QString key = node->tileId().text();
  // A bit of a hack to make cloning terrain generator work properly
  if ( mTerrain )
    return new QgsDemTerrainTileLoader( mTerrain, node, const_cast<QgsDemTerrainGenerator *>( this ) );
  return nullptr;
}

void QgsDemTerrainGenerator::setExtent( const QgsRectangle &extent )
{
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
    delete mHeightMapGenerator;
    mHeightMapGenerator = new QgsDemHeightMapGenerator( dem, mTerrainTilingScheme, mResolution, mTransformContext );
    connect( mHeightMapGenerator, &QgsDemHeightMapGenerator::heightMapReady, this, &QgsDemTerrainGenerator::onHeightMapReceived );

    mIsValid = true;
  }
  else
  {
    mTerrainTilingScheme = QgsTilingScheme();
    delete mHeightMapGenerator;
    mHeightMapGenerator = nullptr;
    mIsValid = false;
  }
}
