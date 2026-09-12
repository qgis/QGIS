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
#include "qgsthreadingutils.h"

#include <QVector>

#include "moc_qgschunkloader.cpp"

///@cond PRIVATE

const QgsChunkLoaderResult QgsChunkLoaderResult::sEmpty = QgsChunkLoaderResult( []( Qt3DCore::QEntity * ) {
  QGIS_CHECK_MAIN_THREAD_ACCESS
  return nullptr;
} );

QFuture<QgsChunkLoaderResult> QgsChunkLoader::updateChunk( QgsChunkNode *node )
{
  return loadChunk( node );
}

QgsQuadtreeChunkLoader::QgsQuadtreeChunkLoader() = default;

QgsQuadtreeChunkLoader::~QgsQuadtreeChunkLoader() = default;

void QgsQuadtreeChunkLoader::setupQuadtree( const QgsBox3D &rootBox3D, float rootError, int maxLevel, const QgsBox3D &clippingBox3D )
{
  mRootBox3D = rootBox3D;
  mRootError = rootError;
  mMaxLevel = maxLevel;
  mClippingBox3D = clippingBox3D;
}

QgsChunkNode *QgsQuadtreeChunkLoader::createRootNode() const
{
  return new QgsChunkNode( QgsChunkNodeId( 0, 0, 0 ), mRootBox3D, mRootError );
}

QFuture<QVector<QgsChunkNode *>> QgsQuadtreeChunkLoader::createChildren( QgsChunkNode *node )
{
  QVector<QgsChunkNode *> children;

  // If there is a max level set, we should respect that
  if ( mMaxLevel != -1 && node->level() >= mMaxLevel )
    return QtFuture::makeReadyValueFuture( children );

  const QgsBox3D box3D = node->box3D();

  // Nodes without extent cannot have children
  if ( box3D.isNull() )
    return QtFuture::makeReadyValueFuture( children );

  const QgsChunkNodeId nodeId = node->tileId();
  const float childError = node->error() / 2;
  QgsVector3D center = box3D.center();

  // if box is very wide (or very thin) we will only split it in one direction so the children
  // are not as wide/thin and are closer to squares
  const bool skipDx = box3D.height() > box3D.width() * 2;
  const bool skipDy = box3D.width() > box3D.height() * 2;
  for ( int i = 0; i < 4; ++i )
  {
    int dx = i & 1, dy = !!( i & 2 );

    if ( dx && skipDx )
      continue;
    if ( dy && skipDy )
      continue;

    const QgsChunkNodeId childId( nodeId.d + 1, nodeId.x * 2 + dx, nodeId.y * 2 + dy );

    const double chXMin = dx ? center.x() : box3D.xMinimum();
    const double chXMax = dx || skipDx ? box3D.xMaximum() : center.x();
    const double chYMin = dy ? center.y() : box3D.yMinimum();
    const double chYMax = dy || skipDy ? box3D.yMaximum() : center.y();
    const double chZMin = box3D.zMinimum();
    const double chZMax = box3D.zMaximum();
    const QgsBox3D childBox3D( chXMin, chYMin, chZMin, chXMax, chYMax, chZMax );

    // When there's a clipping box defined, only keep intersecting children. Note that a QgsBox3D.isEmpty() == true when is2d() == true
    if ( mClippingBox3D.isNull() || childBox3D.intersects( mClippingBox3D ) )
      children << new QgsChunkNode( childId, childBox3D, childError, node );
  }
  return QtFuture::makeReadyValueFuture( children );
}

/// @endcond
