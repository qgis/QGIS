/***************************************************************************
  qgsdemheightmapcache_p.cpp
  --------------------------------------
  Date                 : January 2026
  Copyright            : (C) 2026 By Oslandia
  Email                : benoit dot de dot mezzo at oslandia dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsdemheightmapcache_p.h"

#include "qgs3dutils.h"
#include "qgschunknode.h"
#include "qgsdemterraintileloader_p.h"

#include <QString>

#include "moc_qgsdemheightmapcache_p.cpp"

using namespace Qt::StringLiterals;

QgsDemHeightMapCache::QgsDemHeightMapCache( QgsDemHeightMapGenerator *generator, int resolution, int emitLevel, QgsChunkNode *rootNode )
  : mHeightMapGenerator( generator )
  , mResolution( resolution )
  , mEmitLevel( emitLevel )
  , mRootNode( rootNode )
  , mZMin( Qgs3DUtils::MINIMUM_VECTOR_Z_ESTIMATE )
  , mZMax( Qgs3DUtils::MAXIMUM_VECTOR_Z_ESTIMATE )
{
  connect( mHeightMapGenerator, &QgsDemHeightMapGenerator::heightMapReady, this, &QgsDemHeightMapCache::onHeightMapReceived );
}

size_t QgsDemHeightMapCache::size() const
{
  return mLoaderMap.size();
}

bool QgsDemHeightMapCache::containsTile( const QString &tileText )
{
  return mLoaderMap.contains( tileText );
}

void QgsDemHeightMapCache::setTerrainRootNode( QgsChunkNode *rootNode )
{
  QMutexLocker locker( &mRootNodeMutex );
  mRootNode = rootNode;
}

void QgsDemHeightMapCache::heightMinMax( float &zMin, float &zMax ) const
{
  zMin = mZMin;
  zMax = mZMax;
}


void QgsDemHeightMapCache::onHeightMapReceived( int, const QgsChunkNode *node, const QgsRectangle & /*extent*/, const QByteArray &heightMap )
{
  QMutexLocker locker( &mRootNodeMutex );
  const QgsChunkNodeId nodeId = node->tileId();

  if ( !mLoaderMap.contains( nodeId.text() ) )
  {
    QgsChunkNode *const *children = node->children();
    bool allChildrenInCache = children != nullptr && node->childCount() > 0;
    for ( int i = 0; allChildrenInCache && i < node->childCount(); ++i )
    {
      allChildrenInCache = mLoaderMap.contains( children[i]->tileId().text() );
    }

    if ( !allChildrenInCache )
    {
      // save height map data in cache
      mLoaderMap[nodeId.text()] = heightMap;
      QgsTerrainGenerator::computeHeightMapMinMax( heightMap, mZMin, mZMax );
      cleanup( node );
      if ( nodeId.d >= mEmitLevel )
      {
        // emit only when tile is at the deepest level to update entity elevation with the hi-res tiles
        emit maxResTileReceived( nodeId, node->box3D().toRectangle() );
      }
    }
  }
}

void QgsDemHeightMapCache::cleanup( const QgsChunkNode *currentNode ) const
{
  // check if all sibling are in cache then delete parent height map in cache
  QgsChunkNode *parent = currentNode->parent();

  if ( parent && mLoaderMap.contains( parent->tileId().text() ) && !mLoaderMap[parent->tileId().text()].isEmpty() )
  {
    QgsChunkNode *const *children = parent->children();
    bool deleteParentData = children != nullptr && parent->childCount() > 0;
    for ( int i = 0; deleteParentData && i < parent->childCount(); ++i )
    {
      // if QgsChunkNode state is Loading, this means that the texture has been retrieved
      if ( children[i]->state() != QgsChunkNode::Loading && children[i]->state() != QgsChunkNode::Loaded )
      {
        deleteParentData = false;
      }
    }

    if ( deleteParentData )
    {
      QMutexLocker locker( &mRootNodeMutex );
      // do not really remove the node: make it empty to keep a trace of its existence
      mLoaderMap[parent->tileId().text()] = QByteArray();
    }
  }
}

void QgsDemHeightMapCache::heightAndQualityAt( double x, double y, float &height, int &quality ) const
{
  if ( mRootNode )
  {
    QMutexLocker locker( &mRootNodeMutex );

    // search the best chunk node for x/y
    QgsPoint searched( x, y );
    QgsChunkNode *current = mRootNode;
    QgsChunkNode *bestNode = mRootNode;
    bool deeper = true;

    while ( deeper && current->hasChildrenPopulated() )
    {
      QgsChunkNode *const *children = current->children();
      deeper = false;
      for ( int i = 0; !deeper && i < current->childCount(); ++i )
      {
        if ( children[i]->box3D().contains( searched ) )
        {
          current = children[i];
          if ( mLoaderMap.contains( current->tileId().text() ) && !mLoaderMap[current->tileId().text()].isEmpty() )
            bestNode = current;
          deeper = true;
        }
      }
    }

    QgsChunkNodeId bestTile = bestNode->tileId();

    // search in cache if we know this chunk node/tile
    if ( mLoaderMap.contains( bestTile.text() ) && !mLoaderMap[bestTile.text()].isEmpty() )
    {
      quality = bestTile.d;
      const char *heightMapData = mLoaderMap[bestTile.text()].constData();
      // retrieve height from extracted heightMapData. We do not save parent height map in the cache
      const QgsRectangle extent = current->box3D().toRectangle();

      int cellX = static_cast<int>( lround( ( x - extent.xMinimum() ) / extent.width() * mResolution ) );
      int cellY = static_cast<int>( lround( ( extent.yMaximum() - y ) / extent.height() * mResolution ) );
      cellX = std::clamp( cellX, 0, mResolution - 1 );
      cellY = std::clamp( cellY, 0, mResolution - 1 );

      const float *data = ( const float * ) heightMapData;
      height = data[cellX + cellY * mResolution];
      QgsDebugMsgLevel( u"With cache x:%1, y:%2, z:%3, q:%4"_s.arg( x ).arg( y ).arg( height ).arg( quality ), 2 );
      if ( !std::isnan( height ) && bestTile.d >= mEmitLevel ) // keep good value or continue to fetch coarse one
        return;
    }
  }

  // if we have no rootNode or if we were not able to find better height map data, we fallback to coarse DEM data
  if ( mHeightMapGenerator )
  {
    quality = 0;
    height = mHeightMapGenerator->heightAt( x, y );
    QgsDebugMsgLevel( u"No cache x:%1, y:%2, z:%3, q:%4"_s.arg( x ).arg( y ).arg( height ).arg( quality ), 2 );
  }
  else
  {
    quality = -1;
    height = std::numeric_limits<float>::quiet_NaN();
    QgsDebugMsgLevel( u"No generator x:%1, y:%2, z:%3, q:%4"_s.arg( x ).arg( y ).arg( height ).arg( quality ), 2 );
  }
}
