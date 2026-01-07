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

#include <memory>

#include "qgs3dutils.h"
#include "qgsbox3d.h"
#include "qgschunknode.h"
#include "qgseventtracing.h"
#include "qgslogger.h"
#include "qgspointcloud3dsymbol.h"
#include "qgspointcloud3dsymbol_p.h"
#include "qgspointcloudattribute.h"
#include "qgspointcloudindex.h"
#include "qgspointcloudlayer3drenderer.h"
#include "qgspointcloudrequest.h"
#include "qgsray3d.h"
#include "qgsraycastcontext.h"
#include "qgsraycastingutils.h"

#include <QPointSize>
#include <Qt3DCore/QAttribute>
#include <Qt3DRender/QGraphicsApiFilter>
#include <Qt3DRender/QShaderProgram>
#include <Qt3DRender/QTechnique>
#include <QtConcurrent>

#include "moc_qgspointcloudlayerchunkloader_p.cpp"

///@cond PRIVATE


///////////////

QgsPointCloudLayerChunkLoader::QgsPointCloudLayerChunkLoader( const QgsPointCloudLayerChunkLoaderFactory *factory, QgsChunkNode *node, std::unique_ptr<QgsPointCloud3DSymbol> symbol, const QgsCoordinateTransform &coordinateTransform, double zValueScale, double zValueOffset )
  : QgsChunkLoader( node )
  , mFactory( factory )
  , mContext( factory->mRenderContext, coordinateTransform, std::move( symbol ), zValueScale, zValueOffset )
{
}

void QgsPointCloudLayerChunkLoader::start()
{
  QgsChunkNode *node = chunk();
  QgsPointCloudIndex pc = mFactory->mPointCloudIndex;
  mContext.setAttributes( pc.attributes() );

  const QgsChunkNodeId nodeId = node->tileId();
  const QgsPointCloudNodeId pcNode( nodeId.d, nodeId.x, nodeId.y, nodeId.z );

  Q_ASSERT( pc.hasNode( pcNode ) );

  QgsDebugMsgLevel( u"loading entity %1"_s.arg( node->tileId().text() ), 2 );

  // suppress false positive clang tidy warning
  // NOLINTBEGIN(bugprone-branch-clone)
  if ( mContext.symbol()->symbolType() == "single-color"_L1 )
    mHandler = std::make_unique<QgsSingleColorPointCloud3DSymbolHandler>();
  else if ( mContext.symbol()->symbolType() == "color-ramp"_L1 )
    mHandler = std::make_unique<QgsColorRampPointCloud3DSymbolHandler>();
  else if ( mContext.symbol()->symbolType() == "rgb"_L1 )
    mHandler = std::make_unique<QgsRGBPointCloud3DSymbolHandler>();
  else if ( mContext.symbol()->symbolType() == "classification"_L1 )
  {
    mHandler = std::make_unique<QgsClassificationPointCloud3DSymbolHandler>();
    const QgsClassificationPointCloud3DSymbol *classificationSymbol = dynamic_cast<const QgsClassificationPointCloud3DSymbol *>( mContext.symbol() );
    mContext.setFilteredOutCategories( classificationSymbol->getFilteredOutCategories() );
  }
  // NOLINTEND(bugprone-branch-clone)

  //
  // this will be run in a background thread
  //
  mFutureWatcher = new QFutureWatcher<void>( this );
  connect( mFutureWatcher, &QFutureWatcher<void>::finished, this, &QgsChunkQueueJob::finished );

  const QgsBox3D box3D = node->box3D();
  const QFuture<void> future = QtConcurrent::run( [pc = std::move( pc ), pcNode, box3D, this] {
    const QgsEventTracing::ScopedEvent e( u"3D"_s, u"PC chunk load"_s );

    if ( mContext.isCanceled() )
    {
      QgsDebugMsgLevel( u"canceled"_s, 2 );
      return;
    }

    QgsPointCloudIndex pc2 = pc; // Copy to discard const
    mHandler->processNode( pc2, pcNode, mContext );

    if ( mContext.isCanceled() )
    {
      QgsDebugMsgLevel( u"canceled"_s, 2 );
      return;
    }

    if ( mContext.symbol()->renderAsTriangles() )
      mHandler->triangulate( pc2, pcNode, mContext, box3D );
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
  QgsPointCloudIndex pc = mFactory->mPointCloudIndex;
  const QgsChunkNodeId nodeId = mNode->tileId();
  const QgsPointCloudNodeId pcNode( nodeId.d, nodeId.x, nodeId.y, nodeId.z );
  Q_ASSERT( pc.hasNode( pcNode ) );

  Qt3DCore::QEntity *entity = new Qt3DCore::QEntity( parent );
  mHandler->finalize( entity, mContext );
  return entity;
}

///////////////


QgsPointCloudLayerChunkLoaderFactory::QgsPointCloudLayerChunkLoaderFactory( const Qgs3DRenderContext &context, const QgsCoordinateTransform &coordinateTransform, QgsPointCloudIndex pc, QgsPointCloud3DSymbol *symbol, double zValueScale, double zValueOffset, int pointBudget )
  : mRenderContext( context )
  , mCoordinateTransform( coordinateTransform )
  , mPointCloudIndex( std::move( pc ) )
  , mZValueScale( zValueScale )
  , mZValueOffset( zValueOffset )
  , mPointBudget( pointBudget )
{
  mSymbol.reset( symbol );

  if ( context.crs().type() != Qgis::CrsType::Geocentric ) // extent is not used for globe
  {
    try
    {
      mExtent = mCoordinateTransform.transformBoundingBox( mRenderContext.extent(), Qgis::TransformDirection::Reverse );
    }
    catch ( const QgsCsException & )
    {
      // bad luck, can't reproject for some reason
      QgsDebugError( u"Transformation of extent failed."_s );
    }
  }
}

QgsChunkLoader *QgsPointCloudLayerChunkLoaderFactory::createChunkLoader( QgsChunkNode *node ) const
{
  const QgsChunkNodeId id = node->tileId();

  Q_ASSERT( mPointCloudIndex.hasNode( QgsPointCloudNodeId( id.d, id.x, id.y, id.z ) ) );
  QgsPointCloud3DSymbol *symbol = static_cast<QgsPointCloud3DSymbol *>( mSymbol->clone() );
  return new QgsPointCloudLayerChunkLoader( this, node, std::unique_ptr<QgsPointCloud3DSymbol>( symbol ), mCoordinateTransform, mZValueScale, mZValueOffset );
}

int QgsPointCloudLayerChunkLoaderFactory::primitivesCount( QgsChunkNode *node ) const
{
  const QgsChunkNodeId id = node->tileId();
  const QgsPointCloudNodeId n( id.d, id.x, id.y, id.z );
  Q_ASSERT( mPointCloudIndex.hasNode( n ) );
  return mPointCloudIndex.getNode( n ).pointCount();
}


static QgsBox3D nodeBoundsToBox3D( QgsBox3D nodeBounds, const QgsCoordinateTransform &coordinateTransform, double zValueOffset, double zValueScale )
{
  QgsVector3D extentMin3D( nodeBounds.xMinimum(), nodeBounds.yMinimum(), nodeBounds.zMinimum() * zValueScale + zValueOffset );
  QgsVector3D extentMax3D( nodeBounds.xMaximum(), nodeBounds.yMaximum(), nodeBounds.zMaximum() * zValueScale + zValueOffset );
  QgsCoordinateTransform extentTransform = coordinateTransform;
  extentTransform.setBallparkTransformsAreAppropriate( true );
  try
  {
    extentMin3D = extentTransform.transform( extentMin3D );
    extentMax3D = extentTransform.transform( extentMax3D );
  }
  catch ( QgsCsException & )
  {
    QgsDebugError( u"Error transforming node bounds coordinate"_s );
  }
  return QgsBox3D( extentMin3D.x(), extentMin3D.y(), extentMin3D.z(), extentMax3D.x(), extentMax3D.y(), extentMax3D.z() );
}


QgsChunkNode *QgsPointCloudLayerChunkLoaderFactory::createRootNode() const
{
  const QgsPointCloudNode pcNode = mPointCloudIndex.getNode( mPointCloudIndex.root() );
  const QgsBox3D rootNodeBounds = pcNode.bounds();
  QgsBox3D rootNodeBox3D = nodeBoundsToBox3D( rootNodeBounds, mCoordinateTransform, mZValueOffset, mZValueScale );

  const float error = pcNode.error();
  QgsChunkNode *node = new QgsChunkNode( QgsChunkNodeId( 0, 0, 0, 0 ), rootNodeBox3D, error );
  node->setRefinementProcess( mSymbol->renderAsTriangles() ? Qgis::TileRefinementProcess::Replacement : Qgis::TileRefinementProcess::Additive );
  return node;
}

QVector<QgsChunkNode *> QgsPointCloudLayerChunkLoaderFactory::createChildren( QgsChunkNode *node ) const
{
  QVector<QgsChunkNode *> children;
  const QgsChunkNodeId nodeId = node->tileId();
  const float childError = node->error() / 2;

  for ( int i = 0; i < 8; ++i )
  {
    int dx = i & 1, dy = !!( i & 2 ), dz = !!( i & 4 );
    const QgsChunkNodeId childId( nodeId.d + 1, nodeId.x * 2 + dx, nodeId.y * 2 + dy, nodeId.z * 2 + dz );
    const QgsPointCloudNodeId childPcId( childId.d, childId.x, childId.y, childId.z );
    if ( !mPointCloudIndex.hasNode( childPcId ) )
      continue;
    const QgsPointCloudNode childNode = mPointCloudIndex.getNode( childPcId );
    const QgsBox3D childBounds = childNode.bounds();
    if ( !mExtent.isEmpty() && !childBounds.toRectangle().intersects( mExtent ) )
      continue;

    QgsBox3D childBox3D = nodeBoundsToBox3D( childBounds, mCoordinateTransform, mZValueOffset, mZValueScale );

    QgsChunkNode *child = new QgsChunkNode( childId, childBox3D, childError, node );
    child->setRefinementProcess( mSymbol->renderAsTriangles() ? Qgis::TileRefinementProcess::Replacement : Qgis::TileRefinementProcess::Additive );
    children << child;
  }
  return children;
}

///////////////


static QgsChunkNode *findChunkNodeFromNodeId( QgsChunkNode *rootNode, QgsPointCloudNodeId nodeId )
{
  // find path from the node to the root
  QVector<QgsPointCloudNodeId> parentIds;
  while ( nodeId.d() > 0 )
  {
    parentIds << nodeId;
    nodeId = nodeId.parentNode();
  }

  // now descend from the root to the node in the QgsChunkNode hierarchy
  QgsChunkNode *chunk = rootNode;
  while ( !parentIds.empty() )
  {
    QgsPointCloudNodeId p = parentIds.takeLast();
    QgsChunkNodeId childNodeId( p.d(), p.x(), p.y(), p.z() );

    if ( !chunk->hasChildrenPopulated() )
      return nullptr;

    QgsChunkNode *chunkChild = nullptr;
    QgsChunkNode *const *children = chunk->children();
    for ( int i = 0; i < chunk->childCount(); ++i )
    {
      if ( children[i]->tileId() == childNodeId )
      {
        chunkChild = children[i];
        break;
      }
    }
    Q_ASSERT( chunkChild );
    chunk = chunkChild;
  }
  return chunk;
}


QgsPointCloudLayerChunkedEntity::QgsPointCloudLayerChunkedEntity( Qgs3DMapSettings *map, QgsPointCloudLayer *pcl, QgsPointCloudIndex index, const QgsCoordinateTransform &coordinateTransform, QgsPointCloud3DSymbol *symbol, float maximumScreenSpaceError, bool showBoundingBoxes, double zValueScale, double zValueOffset, int pointBudget )
  : QgsChunkedEntity( map, maximumScreenSpaceError, new QgsPointCloudLayerChunkLoaderFactory( Qgs3DRenderContext::fromMapSettings( map ), coordinateTransform, std::move( index ), symbol, zValueScale, zValueOffset, pointBudget ), true, pointBudget )
  , mLayer( pcl )
{
  setShowBoundingBoxes( showBoundingBoxes );

  if ( pcl->supportsEditing() )
  {
    // when editing starts or stops, we need to update our index to use the editing index (or not)
    connect( pcl, &QgsPointCloudLayer::editingStarted, this, &QgsPointCloudLayerChunkedEntity::updateIndex );
    connect( pcl, &QgsPointCloudLayer::editingStopped, this, &QgsPointCloudLayerChunkedEntity::updateIndex );

    mChunkUpdaterFactory = std::make_unique<QgsChunkUpdaterFactory>( mChunkLoaderFactory );

    connect( pcl, &QgsPointCloudLayer::chunkAttributeValuesChanged, this, [this]( const QgsPointCloudNodeId &n ) {
      QgsChunkNode *node = findChunkNodeFromNodeId( mRootNode, n );
      if ( node )
      {
        updateNodes( QList<QgsChunkNode *>() << node, mChunkUpdaterFactory.get() );
      }
    } );
  }
}

QgsPointCloudLayerChunkedEntity::~QgsPointCloudLayerChunkedEntity()
{
  // cancel / wait for jobs
  cancelActiveJobs();
}

void QgsPointCloudLayerChunkedEntity::updateIndex()
{
  static_cast<QgsPointCloudLayerChunkLoaderFactory *>( mChunkLoaderFactory )->mPointCloudIndex = mLayer->index();
}

QList<QgsRayCastHit> QgsPointCloudLayerChunkedEntity::rayIntersection( const QgsRay3D &ray, const QgsRayCastContext &context ) const
{
  QList<QgsRayCastHit> result;
  QgsPointCloudLayerChunkLoaderFactory *factory = static_cast<QgsPointCloudLayerChunkLoaderFactory *>( mChunkLoaderFactory );

  const QgsPointCloud3DSymbol *symbol = factory->mSymbol.get();
  // Symbol can be null in case of no rendering enabled
  if ( !symbol )
    return result;

  // transform ray
  const QgsVector3D rayOriginMapCoords = factory->mRenderContext.worldToMapCoordinates( ray.origin() );
  const QgsVector3D pointMapCoords = factory->mRenderContext.worldToMapCoordinates( ray.origin() + ray.origin().length() * ray.direction().normalized() );
  QgsVector3D rayDirectionMapCoords = pointMapCoords - rayOriginMapCoords;
  rayDirectionMapCoords.normalize();

  // We're using the angle as a tolerance, effectively meaning we're fetching points intersecting a cone.
  // This may be revisited to use a cylinder instead, if the balance between near/far points does not scale
  // well with different point sizes, screen sizes and fov values.
  const double limitAngle = context.angleThreshold() * M_PI / 180.;

  // adjust ray to elevation properties
  const QgsVector3D adjustedRayOrigin = QgsVector3D( rayOriginMapCoords.x(), rayOriginMapCoords.y(), ( rayOriginMapCoords.z() - factory->mZValueOffset ) / factory->mZValueScale );
  QgsVector3D adjustedRayDirection = QgsVector3D( rayDirectionMapCoords.x(), rayDirectionMapCoords.y(), rayDirectionMapCoords.z() / factory->mZValueScale );
  adjustedRayDirection.normalize();

  QgsPointCloudIndex index = factory->mPointCloudIndex;

  const QgsPointCloudAttributeCollection attributeCollection = index.attributes();
  QgsPointCloudRequest request;
  request.setAttributes( attributeCollection );

  double minDist = -1.;
  const QList<QgsChunkNode *> activeNodes = this->activeNodes();
  for ( QgsChunkNode *node : activeNodes )
  {
    const QgsChunkNodeId id = node->tileId();
    const QgsPointCloudNodeId n( id.d, id.x, id.y, id.z );

    if ( !index.hasNode( n ) )
      continue;

    const QgsAABB nodeBbox = Qgs3DUtils::mapToWorldExtent( node->box3D(), mMapSettings->origin() );
    if ( !QgsRayCastingUtils::rayBoxIntersection( ray, nodeBbox ) )
      continue;

    std::unique_ptr<QgsPointCloudBlock> block( index.nodeData( n, request ) );
    if ( !block )
      continue;

    const QgsVector3D blockScale = block->scale();
    const QgsVector3D blockOffset = block->offset();

    const char *ptr = block->data();
    const QgsPointCloudAttributeCollection blockAttributes = block->attributes();
    const std::size_t recordSize = blockAttributes.pointRecordSize();
    int xOffset = 0, yOffset = 0, zOffset = 0;
    const QgsPointCloudAttribute::DataType xType = blockAttributes.find( u"X"_s, xOffset )->type();
    const QgsPointCloudAttribute::DataType yType = blockAttributes.find( u"Y"_s, yOffset )->type();
    const QgsPointCloudAttribute::DataType zType = blockAttributes.find( u"Z"_s, zOffset )->type();
    for ( int i = 0; i < block->pointCount(); ++i )
    {
      double x, y, z;
      QgsPointCloudAttribute::getPointXYZ( ptr, i, recordSize, xOffset, xType, yOffset, yType, zOffset, zType, blockScale, blockOffset, x, y, z );
      const QgsVector3D point( x, y, z );

      // check whether point is in front of the ray
      // similar to QgsRay3D::isInFront(), but using doubles
      QgsVector3D vectorToPoint = point - adjustedRayOrigin;
      vectorToPoint.normalize();
      if ( QgsVector3D::dotProduct( vectorToPoint, adjustedRayDirection ) < 0.0 )
        continue;

      // calculate the angle between the point and the projected point
      // similar to QgsRay3D::angleToPoint(), but using doubles
      const QgsVector3D projPoint = adjustedRayOrigin + adjustedRayDirection * QgsVector3D::dotProduct( point - adjustedRayOrigin, adjustedRayDirection );

      const double d1 = projPoint.distance( adjustedRayOrigin );
      const double d2 = projPoint.distance( point );
      const double angle = std::atan2( d2, d1 );

      if ( angle > limitAngle )
        continue;

      const double dist = rayOriginMapCoords.distance( point );

      if ( minDist < 0 || dist < minDist )
      {
        minDist = dist;
      }
      else if ( context.singleResult() )
      {
        continue;
      }

      // Note : applying elevation properties is done in fromPointCloudIdentificationToIdentifyResults
      QVariantMap pointAttr = QgsPointCloudAttribute::getAttributeMap( ptr, i * recordSize, blockAttributes );
      pointAttr[u"X"_s] = x;
      pointAttr[u"Y"_s] = y;
      pointAttr[u"Z"_s] = z;

      QgsRayCastHit hit;
      hit.setDistance( dist );
      hit.setMapCoordinates( point );
      hit.setProperties( pointAttr );
      if ( context.singleResult() )
        result.clear();
      result.append( hit );
    }
  }
  return result;
}
/// @endcond
