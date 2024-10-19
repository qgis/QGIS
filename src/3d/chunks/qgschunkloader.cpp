/***************************************************************************
  qgschunkloader.cpp
  --------------------------------------
  Date                 : October 2020
  Copyright            : (C) 2020 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgschunkloader.h"
#include "qgschunknode.h"

#include <QVector>
///@cond PRIVATE

QgsQuadtreeChunkLoaderFactory::QgsQuadtreeChunkLoaderFactory() = default;

QgsQuadtreeChunkLoaderFactory::~QgsQuadtreeChunkLoaderFactory() = default;

void QgsQuadtreeChunkLoaderFactory::setupQuadtree( const QgsAABB &rootBbox, float rootError, int maxLevel, const QgsAABB &clippingBbox )
{
  mRootBbox = rootBbox;
  mRootError = rootError;
  mMaxLevel = maxLevel;
  mClippingBbox = clippingBbox;
}

QgsChunkNode *QgsQuadtreeChunkLoaderFactory::createRootNode() const
{
  return new QgsChunkNode( QgsChunkNodeId( 0, 0, 0 ), mRootBbox, mRootError );
}

QVector<QgsChunkNode *> QgsQuadtreeChunkLoaderFactory::createChildren( QgsChunkNode *node ) const
{
  QVector<QgsChunkNode *> children;

  if ( node->level() >= mMaxLevel )
    return children;

  const QgsChunkNodeId nodeId = node->tileId();
  const float childError = node->error() / 2;
  const QgsAABB bbox = node->bbox();
  float xc = bbox.xCenter(), zc = bbox.zCenter();

  for ( int i = 0; i < 4; ++i )
  {
    int dx = i & 1, dy = !!( i & 2 );
    const QgsChunkNodeId childId( nodeId.d + 1, nodeId.x * 2 + dx, nodeId.y * 2 + ( dy ? 0 : 1 ) );  // TODO: inverse dy?
    // the Y and Z coordinates below are intentionally flipped, because
    // in chunk node IDs the X,Y axes define horizontal plane,
    // while in our 3D scene the X,Z axes define the horizontal plane
    const float chXMin = dx ? xc : bbox.xMin;
    const float chXMax = dx ? bbox.xMax : xc;
    const float chZMin = dy ? zc : bbox.zMin;
    const float chZMax = dy ? bbox.zMax : zc;
    const float chYMin = bbox.yMin;
    const float chYMax = bbox.yMax;
    const QgsAABB childBbox = QgsAABB( chXMin, chYMin, chZMin, chXMax, chYMax, chZMax );
    if ( mClippingBbox.isEmpty() || childBbox.intersects( mClippingBbox ) )
      children << new QgsChunkNode( childId, childBbox, childError, node );
  }
  return children;
}

/// @endcond
