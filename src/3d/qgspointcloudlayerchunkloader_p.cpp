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
#include "qgspointcloudindex.h"
#include "qgspointcloudrequest.h"
#include "qgseventtracing.h"


#include "qgspointcloud3dsymbol.h"
#include "qgspointcloudattribute.h"
#include "qgspointcloud3dsymbol_p.h"

#include <QtConcurrent>
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
#include <Qt3DRender/QAttribute>
#else
#include <Qt3DCore/QAttribute>
#endif
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

  try
  {
    mExtent = mCoordinateTransform.transformBoundingBox( mMap.extent(), Qgis::TransformDirection::Reverse );
  }
  catch ( const QgsCsException & )
  {
    // bad luck, can't reproject for some reason
    QgsDebugMsg( QStringLiteral( "Transformation of extent failed." ) );
  }
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
    if ( !mExtent.isEmpty() &&
         !mPointCloudIndex->nodeMapExtent( IndexedPointCloudNode( childId.d, childId.x, childId.y, childId.z ) ).intersects( mExtent ) )
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

QVector<RayHit> QgsPointCloudLayerChunkedEntity::rayIntersection( const QgsRay3D &ray, const RayCastContext &context ) const
{

  QgsPointCloudLayerChunkLoaderFactory *factory = static_cast<QgsPointCloudLayerChunkLoaderFactory *>( mChunkLoaderFactory );

  QVector<RayHit> result;
  // transform ray
  const QgsVector3D originMapCoords = factory->mMap.worldToMapCoordinates( ray.origin() );
  const QgsVector3D pointMapCoords = factory->mMap.worldToMapCoordinates( ray.origin() + ray.origin().length() * ray.direction().normalized() );
  QgsVector3D directionMapCoords = pointMapCoords - originMapCoords;
  directionMapCoords.normalize();

  const QVector3D rayOriginMapCoords( originMapCoords.x(), originMapCoords.y(), originMapCoords.z() );
  const QVector3D rayDirectionMapCoords( directionMapCoords.x(), directionMapCoords.y(), directionMapCoords.z() );

//  const QSize size = mEngine->size();
//  const int screenSizePx = std::max( size.width(), size.height() ); // TODO: is this correct? (see sceneState_)
  const int screenSizePx = std::max( context.screenWidth, context.screenHeight ); // TODO: is this correct? (see sceneState_)

  const QgsPointCloud3DSymbol *symbol = factory->mSymbol.get();
  // Symbol can be null in case of no rendering enabled
  if ( !symbol )
    return result;
  const double pointSize = symbol->pointSize();

  const double limitAngle = 2 * pointSize / screenSizePx * factory->mMap.fieldOfView();

  // adjust ray to elevation properties
  const QVector3D adjutedRayOrigin = QVector3D( rayOriginMapCoords.x(), rayOriginMapCoords.y(), ( rayOriginMapCoords.z() -  factory->mZValueOffset ) / factory->mZValueScale );
  QVector3D adjutedRayDirection = QVector3D( rayDirectionMapCoords.x(), rayDirectionMapCoords.y(), rayDirectionMapCoords.z() / factory->mZValueScale );
  adjutedRayDirection.normalize();

  const QgsRay3D layerRay( adjutedRayOrigin, adjutedRayDirection );

  QgsPointCloudIndex *index = factory->mPointCloudIndex;

  const QgsPointCloudAttributeCollection attributeCollection = index->attributes();
  QgsPointCloudRequest request;
  request.setAttributes( attributeCollection );

  const QList<QgsChunkNode *> activeNodes = this->activeNodes();
  for ( const auto &node : activeNodes )
  {
    const QgsChunkNodeId id = node->tileId();
    const IndexedPointCloudNode n( id.d, id.x, id.y, id.z );

    if ( !index->hasNode( n ) )
      continue;

    if ( !ray.intersects( Qgs3DUtils::aabbToBox( node->bbox() ) ) )
      continue;

    std::unique_ptr<QgsPointCloudBlock> block( index->nodeData( n, request ) );
    if ( !block )
      continue;

    const QgsVector3D blockScale = block->scale();
    const QgsVector3D blockOffset = block->offset();

    const char *ptr = block->data();
    const QgsPointCloudAttributeCollection blockAttributes = block->attributes();
    const std::size_t recordSize = blockAttributes.pointRecordSize();
    int xOffset = 0, yOffset = 0, zOffset = 0;
    const QgsPointCloudAttribute::DataType xType = blockAttributes.find( QStringLiteral( "X" ), xOffset )->type();
    const QgsPointCloudAttribute::DataType yType = blockAttributes.find( QStringLiteral( "Y" ), yOffset )->type();
    const QgsPointCloudAttribute::DataType zType = blockAttributes.find( QStringLiteral( "Z" ), zOffset )->type();
    for ( int i = 0; i < block->pointCount(); ++i )
    {
      double x, y, z;
      QgsPointCloudAttribute::getPointXYZ( ptr, i, recordSize, xOffset, xType, yOffset, yType, zOffset, zType, blockScale, blockOffset, x, y, z );
      const QVector3D point( x, y, z );

      // check whether point is in front of the ray
      if ( !layerRay.isInFront( point ) )
        continue;

      // calculate the angle between the point and the projected point
      if ( layerRay.angleToPoint( point ) > limitAngle )
        continue;

      // Note : applying elevation properties is done in fromPointCloudIdentificationToIdentifyResults
      QVariantMap pointAttr = QgsPointCloudAttribute::getAttributeMap( ptr, i * recordSize, blockAttributes );
      pointAttr[ QStringLiteral( "X" ) ] = x;
      pointAttr[ QStringLiteral( "Y" ) ] = y;
      pointAttr[ QStringLiteral( "Z" ) ] = z;

      RayHit hit( 0, QVector3D( x, y, z ), QgsFeatureId(), pointAttr/*, layer */ );
      result.append( hit );
    }
  }
  return result;
}
/// @endcond
