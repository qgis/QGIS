/***************************************************************************
  qgspointcloudlayerchunkloader_p.cpp
  --------------------------------------
  Date                 : October 2020
  Copyright            : (C) 2020 by Peter Petrik
  Email                : zilolv dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgspointcloudlayerchunkloader_p.h"

#include "qgs3dutils.h"
#include "qgspointcloudlayer3drenderer.h"
#include "qgschunknode_p.h"
#include "qgslogger.h"
#include "qgspointcloudlayer.h"
#include "qgspointcloudindex.h"
#include "qgseventtracing.h"

#include "qgspoint3dsymbol.h"
#include "qgsphongmaterialsettings.h"

#include "qgspointcloud3dsymbol.h"
#include "qgsapplication.h"
#include "qgs3dsymbolregistry.h"
#include "qgspointcloudattribute.h"
#include "qgspointcloudrequest.h"
#include "qgspointcloud3dsymbol_p.h"

#include <QtConcurrent>
#include <Qt3DRender/QAttribute>
#include <Qt3DRender/QTechnique>
#include <Qt3DRender/QShaderProgram>
#include <Qt3DRender/QGraphicsApiFilter>
#include <QPointSize>

///@cond PRIVATE


///////////////

QgsPointCloudLayerChunkLoader::QgsPointCloudLayerChunkLoader( const QgsPointCloudLayerChunkLoaderFactory *factory, QgsChunkNode *node, std::unique_ptr< QgsPointCloud3DSymbol > symbol,
    const QgsCoordinateTransform &coordinateTransform, double zValueScale, double zValueOffset )
  : QgsChunkLoader( node )
  , mFactory( factory )
  , mContext( factory->mMap, coordinateTransform, std::move( symbol ), zValueScale, zValueOffset )
{

  QgsPointCloudIndex *pc = mFactory->mPointCloudIndex;
  mContext.setAttributes( pc->attributes() );

  const QgsChunkNodeId nodeId = node->tileId();
  const IndexedPointCloudNode pcNode( nodeId.d, nodeId.x, nodeId.y, nodeId.z );

  Q_ASSERT( pc->hasNode( pcNode ) );

  QgsDebugMsgLevel( QStringLiteral( "loading entity %1" ).arg( node->tileId().text() ), 2 );

  if ( mContext.symbol()->symbolType() == QLatin1String( "single-color" ) )
    mHandler.reset( new QgsSingleColorPointCloud3DSymbolHandler() );
  else if ( mContext.symbol()->symbolType() == QLatin1String( "color-ramp" ) )
    mHandler.reset( new QgsColorRampPointCloud3DSymbolHandler() );
  else if ( mContext.symbol()->symbolType() == QLatin1String( "rgb" ) )
    mHandler.reset( new QgsRGBPointCloud3DSymbolHandler() );
  else if ( mContext.symbol()->symbolType() == QLatin1String( "classification" ) )
  {
    mHandler.reset( new QgsClassificationPointCloud3DSymbolHandler() );
    const QgsClassificationPointCloud3DSymbol *classificationSymbol = dynamic_cast<const QgsClassificationPointCloud3DSymbol *>( mContext.symbol() );
    mContext.setFilteredOutCategories( classificationSymbol->getFilteredOutCategories() );
  }

  //
  // this will be run in a background thread
  //
  mFutureWatcher = new QFutureWatcher<void>( this );
  connect( mFutureWatcher, &QFutureWatcher<void>::finished, this, &QgsChunkQueueJob::finished );

  const QgsAABB bbox = node->bbox();
  const QFuture<void> future = QtConcurrent::run( [pc, pcNode, bbox, this]
  {
    const QgsEventTracing::ScopedEvent e( QStringLiteral( "3D" ), QStringLiteral( "PC chunk load" ) );

    if ( mContext.isCanceled() )
    {
      QgsDebugMsgLevel( QStringLiteral( "canceled" ), 2 );
      return;
    }

    mHandler->processNode( pc, pcNode, mContext );
    if ( mContext.symbol()->renderAsTriangles() )
      mHandler->triangulate( pc, pcNode, mContext, bbox );
  } );

  // emit finished() as soon as the handler is populated with features
  mFutureWatcher->setFuture( future );
}

QgsPointCloudLayerChunkLoader::~QgsPointCloudLayerChunkLoader()
{
  if ( mFutureWatcher && !mFutureWatcher->isFinished() )
  {
    disconnect( mFutureWatcher, &QFutureWatcher<void>::finished, this, &QgsChunkQueueJob::finished );
    mContext.cancelRendering();
    mFutureWatcher->waitForFinished();
  }
}

void QgsPointCloudLayerChunkLoader::cancel()
{
  mContext.cancelRendering();
}

Qt3DCore::QEntity *QgsPointCloudLayerChunkLoader::createEntity( Qt3DCore::QEntity *parent )
{
  QgsPointCloudIndex *pc = mFactory->mPointCloudIndex;
  const QgsChunkNodeId nodeId = mNode->tileId();
  const IndexedPointCloudNode pcNode( nodeId.d, nodeId.x, nodeId.y, nodeId.z );
  Q_ASSERT( pc->hasNode( pcNode ) );

  Qt3DCore::QEntity *entity = new Qt3DCore::QEntity( parent );
  mHandler->finalize( entity, mContext );
  return entity;
}

///////////////


QgsPointCloudLayerChunkLoaderFactory::QgsPointCloudLayerChunkLoaderFactory( const Qgs3DMapSettings &map, const QgsCoordinateTransform &coordinateTransform, QgsPointCloudIndex *pc, QgsPointCloud3DSymbol *symbol,
    double zValueScale, double zValueOffset, int pointBudget )
  : mMap( map )
  , mCoordinateTransform( coordinateTransform )
  , mPointCloudIndex( pc )
  , mZValueScale( zValueScale )
  , mZValueOffset( zValueOffset )
  , mPointBudget( pointBudget )
{
  mSymbol.reset( symbol );
}

QgsChunkLoader *QgsPointCloudLayerChunkLoaderFactory::createChunkLoader( QgsChunkNode *node ) const
{
  const QgsChunkNodeId id = node->tileId();

  Q_ASSERT( mPointCloudIndex->hasNode( IndexedPointCloudNode( id.d, id.x, id.y, id.z ) ) );
  QgsPointCloud3DSymbol *symbol = static_cast< QgsPointCloud3DSymbol * >( mSymbol->clone() );
  return new QgsPointCloudLayerChunkLoader( this, node, std::unique_ptr< QgsPointCloud3DSymbol >( symbol ), mCoordinateTransform, mZValueScale, mZValueOffset );
}

int QgsPointCloudLayerChunkLoaderFactory::primitivesCount( QgsChunkNode *node ) const
{
  const QgsChunkNodeId id = node->tileId();
  const IndexedPointCloudNode n( id.d, id.x, id.y, id.z );
  Q_ASSERT( mPointCloudIndex->hasNode( n ) );
  return mPointCloudIndex->nodePointCount( n );
}

QgsAABB nodeBoundsToAABB( QgsPointCloudDataBounds nodeBounds, QgsVector3D offset, QgsVector3D scale, const Qgs3DMapSettings &map, const QgsCoordinateTransform &coordinateTransform, double zValueOffset );

QgsChunkNode *QgsPointCloudLayerChunkLoaderFactory::createRootNode() const
{
  const QgsAABB bbox = nodeBoundsToAABB( mPointCloudIndex->nodeBounds( IndexedPointCloudNode( 0, 0, 0, 0 ) ), mPointCloudIndex->offset(), mPointCloudIndex->scale(), mMap, mCoordinateTransform, mZValueOffset );
  const float error = mPointCloudIndex->nodeError( IndexedPointCloudNode( 0, 0, 0, 0 ) );
  return new QgsChunkNode( QgsChunkNodeId( 0, 0, 0, 0 ), bbox, error );
}

QVector<QgsChunkNode *> QgsPointCloudLayerChunkLoaderFactory::createChildren( QgsChunkNode *node ) const
{
  QVector<QgsChunkNode *> children;
  const QgsChunkNodeId nodeId = node->tileId();
  const QgsAABB bbox = node->bbox();
  const float childError = node->error() / 2;
  float xc = bbox.xCenter(), yc = bbox.yCenter(), zc = bbox.zCenter();

  for ( int i = 0; i < 8; ++i )
  {
    int dx = i & 1, dy = !!( i & 2 ), dz = !!( i & 4 );
    const QgsChunkNodeId childId( nodeId.d + 1, nodeId.x * 2 + dx, nodeId.y * 2 + dy, nodeId.z * 2 + dz );

    if ( !mPointCloudIndex->hasNode( IndexedPointCloudNode( childId.d, childId.x, childId.y, childId.z ) ) )
      continue;

    // the Y and Z coordinates below are intentionally flipped, because
    // in chunk node IDs the X,Y axes define horizontal plane,
    // while in our 3D scene the X,Z axes define the horizontal plane
    const float chXMin = dx ? xc : bbox.xMin;
    const float chXMax = dx ? bbox.xMax : xc;
    // Z axis: values are increasing to the south
    const float chZMin = !dy ? zc : bbox.zMin;
    const float chZMax = !dy ? bbox.zMax : zc;
    const float chYMin = dz ? yc : bbox.yMin;
    const float chYMax = dz ? bbox.yMax : yc;
    children << new QgsChunkNode( childId, QgsAABB( chXMin, chYMin, chZMin, chXMax, chYMax, chZMax ), childError, node );
  }
  return children;
}

///////////////


QgsAABB nodeBoundsToAABB( QgsPointCloudDataBounds nodeBounds, QgsVector3D offset, QgsVector3D scale, const Qgs3DMapSettings &map, const QgsCoordinateTransform &coordinateTransform, double zValueOffset )
{
  QgsVector3D extentMin3D( nodeBounds.xMin() * scale.x() + offset.x(), nodeBounds.yMin() * scale.y() + offset.y(), nodeBounds.zMin() * scale.z() + offset.z() + zValueOffset );
  QgsVector3D extentMax3D( nodeBounds.xMax() * scale.x() + offset.x(), nodeBounds.yMax() * scale.y() + offset.y(), nodeBounds.zMax() * scale.z() + offset.z() + zValueOffset );
  QgsCoordinateTransform extentTransform = coordinateTransform;
  extentTransform.setBallparkTransformsAreAppropriate( true );
  try
  {
    extentMin3D = extentTransform.transform( extentMin3D );
    extentMax3D = extentTransform.transform( extentMax3D );
  }
  catch ( QgsCsException & )
  {
    QgsDebugMsg( QStringLiteral( "Error transforming node bounds coordinate" ) );
  }
  const QgsVector3D worldExtentMin3D = Qgs3DUtils::mapToWorldCoordinates( extentMin3D, map.origin() );
  const QgsVector3D worldExtentMax3D = Qgs3DUtils::mapToWorldCoordinates( extentMax3D, map.origin() );
  QgsAABB rootBbox( worldExtentMin3D.x(), worldExtentMin3D.y(), worldExtentMin3D.z(),
                    worldExtentMax3D.x(), worldExtentMax3D.y(), worldExtentMax3D.z() );
  return rootBbox;
}


QgsPointCloudLayerChunkedEntity::QgsPointCloudLayerChunkedEntity( QgsPointCloudIndex *pc, const Qgs3DMapSettings &map,
    const QgsCoordinateTransform &coordinateTransform, QgsPointCloud3DSymbol *symbol,
    float maximumScreenSpaceError, bool showBoundingBoxes,
    double zValueScale, double zValueOffset,
    int pointBudget )
  : QgsChunkedEntity( maximumScreenSpaceError,
                      new QgsPointCloudLayerChunkLoaderFactory( map, coordinateTransform, pc, symbol, zValueScale, zValueOffset, pointBudget ), true, pointBudget )
{
  setUsingAdditiveStrategy( !symbol->renderAsTriangles() );
  setShowBoundingBoxes( showBoundingBoxes );
}

QgsPointCloudLayerChunkedEntity::~QgsPointCloudLayerChunkedEntity()
{
  // cancel / wait for jobs
  cancelActiveJobs();
}

/// @endcond
