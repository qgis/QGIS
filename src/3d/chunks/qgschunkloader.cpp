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
#include "moc_qgschunkloader.cpp"
#include "qgschunknode.h"

#include <QVector>
///@cond PRIVATE

QgsQuadtreeChunkLoaderFactory::QgsQuadtreeChunkLoaderFactory() = default;

QgsQuadtreeChunkLoaderFactory::~QgsQuadtreeChunkLoaderFactory() = default;

void QgsQuadtreeChunkLoaderFactory::setupQuadtree( const QgsBox3D &rootBox3D, float rootError, int maxLevel, const QgsBox3D &clippingBox3D )
{
  mRootBox3D = rootBox3D;
  mRootError = rootError;
  mMaxLevel = maxLevel;
  mClippingBox3D = clippingBox3D;
}

QgsChunkNode *QgsQuadtreeChunkLoaderFactory::createRootNode() const
{
  return new QgsChunkNode( QgsChunkNodeId( 0, 0, 0 ), mRootBox3D, mRootError );
}

QVector<QgsChunkNode *> QgsQuadtreeChunkLoaderFactory::createChildren( QgsChunkNode *node ) const
{
  QVector<QgsChunkNode *> children;

  if ( node->level() >= mMaxLevel )
    return children;

  const QgsChunkNodeId nodeId = node->tileId();
  const float childError = node->error() / 2;
  const QgsBox3D box3D = node->box3D();
  QgsVector3D center = box3D.center();

  for ( int i = 0; i < 4; ++i )
  {
    int dx = i & 1, dy = !!( i & 2 );
    const QgsChunkNodeId childId( nodeId.d + 1, nodeId.x * 2 + dx, nodeId.y * 2 + dy );

    const double chXMin = dx ? center.x() : box3D.xMinimum();
    const double chXMax = dx ? box3D.xMaximum() : center.x();
    const double chYMin = dy ? center.y() : box3D.yMinimum();
    const double chYMax = dy ? box3D.yMaximum() : center.y();
    const double chZMin = box3D.zMinimum();
    const double chZMax = box3D.zMaximum();
    const QgsBox3D childBox3D( chXMin, chYMin, chZMin, chXMax, chYMax, chZMax );

    if ( mClippingBox3D.isEmpty() || childBox3D.intersects( mClippingBox3D ) )
      children << new QgsChunkNode( childId, childBox3D, childError, node );
  }
  return children;
}

/// @endcond
