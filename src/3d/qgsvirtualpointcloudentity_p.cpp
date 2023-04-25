/***************************************************************************
  qgsvirtualpointcloudentity_p.cpp
  --------------------------------------
  Date                 : April 2023
  Copyright            : (C) 2023 by Stefanos Natsis
  Email                : uclaros at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsvirtualpointcloudentity_p.h"
#include "qgspointcloudlayerchunkloader_p.h"

#include <QElapsedTimer>
#include <QVector4D>
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
#include <Qt3DRender/QBuffer>
typedef Qt3DRender::QBuffer Qt3DQBuffer;
#else
#include <Qt3DCore/QBuffer>
typedef Qt3DCore::QBuffer Qt3DQBuffer;
#endif

#include "qgs3dutils.h"
#include "qgschunkboundsentity_p.h"
#include "qgschunklist_p.h"
#include "qgschunkloader_p.h"
#include "qgschunknode_p.h"
#include "qgstessellatedpolygongeometry.h"

#include "qgseventtracing.h"

#include <queue>

///@cond PRIVATE


QgsVirtualPointCloudEntity::QgsVirtualPointCloudEntity( QVector<QgsPointCloudSubIndex> *subIndexes,
    const Qgs3DMapSettings &map,
    const QgsCoordinateTransform &coordinateTransform,
    QgsPointCloud3DSymbol *symbol,
    float maximumScreenSpaceError,
    bool showBoundingBoxes,
    double zValueScale,
    double zValueOffset,
    int pointBudget )
  : Qt3DCore::QEntity( nullptr )
  , mSubIndexes( subIndexes )
  , mMap( map )
  , mCoordinateTransform( coordinateTransform )
  , mZValueScale( zValueScale )
  , mZValueOffset( zValueOffset )
  , mPointBudget( pointBudget )
  , mMaximumScreenSpaceError( maximumScreenSpaceError )
  , mShowBoundingBoxes( showBoundingBoxes )
{
  mSymbol.reset( symbol );
}


QgsVirtualPointCloudEntity::~QgsVirtualPointCloudEntity()
{

}

QList<QgsChunkedEntity *> QgsVirtualPointCloudEntity::chunkedEntities() const
{
  return mChunkedEntities;
}

void QgsVirtualPointCloudEntity::createChunkedEntitiesForLoadedSubIndexes()
{
  if ( !mChunkedEntities.isEmpty() )
    return;

  for ( const auto &si : *mSubIndexes )
  {
    if ( si.index() )
    {
      mChunkedEntities.append( new QgsPointCloudLayerChunkedEntity( si.index(),
                               mMap,
                               mCoordinateTransform,
                               static_cast< QgsPointCloud3DSymbol * >( mSymbol->clone() ),
                               mMaximumScreenSpaceError,
                               mShowBoundingBoxes,
                               mZValueScale,
                               mZValueOffset,
                               mPointBudget ) );
    }
  }
}

void QgsVirtualPointCloudEntity::onSubIndexLoaded( int i )
{
  QgsPointCloudLayerChunkedEntity *newChunkedEntity = new QgsPointCloudLayerChunkedEntity( mSubIndexes->at( i ).index(),
      mMap,
      mCoordinateTransform,
      static_cast< QgsPointCloud3DSymbol * >( mSymbol->clone() ),
      mMaximumScreenSpaceError,
      mShowBoundingBoxes,
      mZValueScale,
      mZValueOffset,
      mPointBudget );
  mChunkedEntities.append( newChunkedEntity );
  emit newEntityCreated( newChunkedEntity );
}

/// @endcond
