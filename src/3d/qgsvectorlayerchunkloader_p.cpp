/***************************************************************************
  qgsvectorlayerchunkloader_p.cpp
  --------------------------------------
  Date                 : July 2019
  Copyright            : (C) 2019 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsvectorlayerchunkloader_p.h"
#include "moc_qgsvectorlayerchunkloader_p.cpp"
#include "qgs3dutils.h"
#include "qgsline3dsymbol.h"
#include "qgspoint3dsymbol.h"
#include "qgspolygon3dsymbol.h"
#include "qgsraycastingutils_p.h"
#include "qgsabstractvectorlayer3drenderer.h"
#include "qgstessellatedpolygongeometry.h"
#include "qgschunknode.h"
#include "qgseventtracing.h"
#include "qgslogger.h"
#include "qgsvectorlayer.h"
#include "qgsvectorlayerfeatureiterator.h"
#include "qgsapplication.h"
#include "qgs3dsymbolregistry.h"
#include "qgsabstract3dsymbol.h"
#include "qgsabstractterrainsettings.h"

#include <QtConcurrent>
#include <Qt3DCore/QTransform>

///@cond PRIVATE


QgsVectorLayerChunkLoader::QgsVectorLayerChunkLoader( const QgsVectorLayerChunkLoaderFactory *factory, QgsChunkNode *node )
  : QgsChunkLoader( node )
  , mFactory( factory )
  , mRenderContext( factory->mRenderContext )
  , mSource( new QgsVectorLayerFeatureSource( factory->mLayer ) )
{
  if ( node->level() < mFactory->mLeafLevel )
  {
    QTimer::singleShot( 0, this, &QgsVectorLayerChunkLoader::finished );
    return;
  }

  QgsVectorLayer *layer = mFactory->mLayer;
  mLayerName = mFactory->mLayer->name();

  QgsFeature3DHandler *handler = QgsApplication::symbol3DRegistry()->createHandlerForSymbol( layer, mFactory->mSymbol.get() );
  if ( !handler )
  {
    QgsDebugError( QStringLiteral( "Unknown 3D symbol type for vector layer: " ) + mFactory->mSymbol->type() );
    return;
  }
  mHandler.reset( handler );

  // only a subset of data to be queried
  const QgsRectangle rect = node->box3D().toRectangle();
  // origin for coordinates of the chunk - it is kind of arbitrary, but it should be
  // picked so that the coordinates are relatively small to avoid numerical precision issues
  QgsVector3D chunkOrigin( rect.center().x(), rect.center().y(), 0 );

  QgsExpressionContext exprContext( Qgs3DUtils::globalProjectLayerExpressionContext( layer ) );
  exprContext.setFields( layer->fields() );
  mRenderContext.setExpressionContext( exprContext );

  QSet<QString> attributeNames;
  if ( !mHandler->prepare( mRenderContext, attributeNames, chunkOrigin ) )
  {
    QgsDebugError( QStringLiteral( "Failed to prepare 3D feature handler!" ) );
    return;
  }

  // build the feature request
  QgsFeatureRequest req;
  req.setCoordinateTransform(
    QgsCoordinateTransform( layer->crs3D(), mRenderContext.crs(), mRenderContext.transformContext() )
  );
  req.setSubsetOfAttributes( attributeNames, layer->fields() );
  req.setFilterRect( rect );

  //
  // this will be run in a background thread
  //
  mFutureWatcher = new QFutureWatcher<void>( this );
  connect( mFutureWatcher, &QFutureWatcher<void>::finished, this, &QgsChunkQueueJob::finished );

  const QFuture<void> future = QtConcurrent::run( [req, this] {
    const QgsEventTracing::ScopedEvent e( QStringLiteral( "3D" ), QStringLiteral( "VL chunk load" ) );

    QgsFeature f;
    QgsFeatureIterator fi = mSource->getFeatures( req );
    while ( fi.nextFeature( f ) )
    {
      if ( mCanceled )
        break;
      mRenderContext.expressionContext().setFeature( f );
      mHandler->processFeature( f, mRenderContext );
    }
  } );

  // emit finished() as soon as the handler is populated with features
  mFutureWatcher->setFuture( future );
}

QgsVectorLayerChunkLoader::~QgsVectorLayerChunkLoader()
{
  if ( mFutureWatcher && !mFutureWatcher->isFinished() )
  {
    disconnect( mFutureWatcher, &QFutureWatcher<void>::finished, this, &QgsChunkQueueJob::finished );
    mFutureWatcher->waitForFinished();
  }
}

void QgsVectorLayerChunkLoader::cancel()
{
  mCanceled = true;
}

Qt3DCore::QEntity *QgsVectorLayerChunkLoader::createEntity( Qt3DCore::QEntity *parent )
{
  if ( mNode->level() < mFactory->mLeafLevel )
  {
    Qt3DCore::QEntity *entity = new Qt3DCore::QEntity( parent ); // dummy entity
    entity->setObjectName( mLayerName + "_CONTAINER_" + mNode->tileId().text() );
    return entity;
  }

  if ( mHandler->featureCount() == 0 )
  {
    // an empty node, so we return no entity. This tags the node as having no data and effectively removes it.
    // we just make sure first that its initial estimated vertical range does not affect its parents' bboxes calculation
    mNode->setExactBox3D( QgsBox3D() );
    mNode->updateParentBoundingBoxesRecursively();
    return nullptr;
  }

  Qt3DCore::QEntity *entity = new Qt3DCore::QEntity( parent );
  entity->setObjectName( mLayerName + "_" + mNode->tileId().text() );
  mHandler->finalize( entity, mRenderContext );

  // fix the vertical range of the node from the estimated vertical range to the true range
  if ( mHandler->zMinimum() != std::numeric_limits<float>::max() && mHandler->zMaximum() != std::numeric_limits<float>::lowest() )
  {
    QgsBox3D box = mNode->box3D();
    box.setZMinimum( mHandler->zMinimum() );
    box.setZMaximum( mHandler->zMaximum() );
    mNode->setExactBox3D( box );
    mNode->updateParentBoundingBoxesRecursively();
  }

  return entity;
}


///////////////


QgsVectorLayerChunkLoaderFactory::QgsVectorLayerChunkLoaderFactory( const Qgs3DRenderContext &context, QgsVectorLayer *vl, QgsAbstract3DSymbol *symbol, int leafLevel, double zMin, double zMax )
  : mRenderContext( context )
  , mLayer( vl )
  , mSymbol( symbol->clone() )
  , mLeafLevel( leafLevel )
{
  QgsBox3D rootBox3D( context.extent(), zMin, zMax );
  // add small padding to avoid clipping of point features located at the edge of the bounding box
  rootBox3D.grow( 1.0 );
  setupQuadtree( rootBox3D, -1, leafLevel ); // negative root error means that the node does not contain anything
}

QgsChunkLoader *QgsVectorLayerChunkLoaderFactory::createChunkLoader( QgsChunkNode *node ) const
{
  return new QgsVectorLayerChunkLoader( this, node );
}


///////////////


QgsVectorLayerChunkedEntity::QgsVectorLayerChunkedEntity( Qgs3DMapSettings *map, QgsVectorLayer *vl, double zMin, double zMax, const QgsVectorLayer3DTilingSettings &tilingSettings, QgsAbstract3DSymbol *symbol )
  : QgsChunkedEntity( map,
                      -1, // max. allowed screen error (negative tau means that we need to go until leaves are reached)
                      new QgsVectorLayerChunkLoaderFactory( Qgs3DRenderContext::fromMapSettings( map ), vl, symbol, tilingSettings.zoomLevelsCount() - 1, zMin, zMax ), true )
{
  mTransform = new Qt3DCore::QTransform;
  if ( applyTerrainOffset() )
  {
    mTransform->setTranslation( QVector3D( 0.0f, static_cast<float>( map->terrainSettings()->elevationOffset() ), 0.0f ) );
  }
  this->addComponent( mTransform );

  connect( map, &Qgs3DMapSettings::terrainSettingsChanged, this, &QgsVectorLayerChunkedEntity::onTerrainElevationOffsetChanged );

  setShowBoundingBoxes( tilingSettings.showBoundingBoxes() );
}

QgsVectorLayerChunkedEntity::~QgsVectorLayerChunkedEntity()
{
  // cancel / wait for jobs
  cancelActiveJobs();
}

// if the AltitudeClamping is `Absolute`, do not apply the offset
bool QgsVectorLayerChunkedEntity::applyTerrainOffset() const
{
  QgsVectorLayerChunkLoaderFactory *loaderFactory = static_cast<QgsVectorLayerChunkLoaderFactory *>( mChunkLoaderFactory );
  if ( loaderFactory )
  {
    QString symbolType = loaderFactory->mSymbol.get()->type();
    if ( symbolType == "line" )
    {
      QgsLine3DSymbol *lineSymbol = static_cast<QgsLine3DSymbol *>( loaderFactory->mSymbol.get() );
      if ( lineSymbol && lineSymbol->altitudeClamping() == Qgis::AltitudeClamping::Absolute )
      {
        return false;
      }
    }
    else if ( symbolType == "point" )
    {
      QgsPoint3DSymbol *pointSymbol = static_cast<QgsPoint3DSymbol *>( loaderFactory->mSymbol.get() );
      if ( pointSymbol && pointSymbol->altitudeClamping() == Qgis::AltitudeClamping::Absolute )
      {
        return false;
      }
    }
    else if ( symbolType == "polygon" )
    {
      QgsPolygon3DSymbol *polygonSymbol = static_cast<QgsPolygon3DSymbol *>( loaderFactory->mSymbol.get() );
      if ( polygonSymbol && polygonSymbol->altitudeClamping() == Qgis::AltitudeClamping::Absolute )
      {
        return false;
      }
    }
    else
    {
      QgsDebugMsgLevel( QStringLiteral( "QgsVectorLayerChunkedEntity::applyTerrainOffset, unhandled symbol type %1" ).arg( symbolType ), 2 );
    }
  }

  return true;
}

void QgsVectorLayerChunkedEntity::onTerrainElevationOffsetChanged()
{
  QgsDebugMsgLevel( QStringLiteral( "QgsVectorLayerChunkedEntity::onTerrainElevationOffsetChanged" ), 2 );
  float newOffset = static_cast<float>( qobject_cast<Qgs3DMapSettings *>( sender() )->terrainSettings()->elevationOffset() );
  if ( !applyTerrainOffset() )
  {
    newOffset = 0.0;
  }
  mTransform->setTranslation( QVector3D( 0.0f, newOffset, 0.0f ) );
}

QVector<QgsRayCastingUtils::RayHit> QgsVectorLayerChunkedEntity::rayIntersection( const QgsRayCastingUtils::Ray3D &ray, const QgsRayCastingUtils::RayCastContext &context ) const
{
  return QgsVectorLayerChunkedEntity::rayIntersection( activeNodes(), mTransform->matrix(), ray, context, mMapSettings->origin() );
}

QVector<QgsRayCastingUtils::RayHit> QgsVectorLayerChunkedEntity::rayIntersection( const QList<QgsChunkNode *> &activeNodes, const QMatrix4x4 &transformMatrix, const QgsRayCastingUtils::Ray3D &ray, const QgsRayCastingUtils::RayCastContext &context, const QgsVector3D &origin )
{
  Q_UNUSED( context )
  QgsDebugMsgLevel( QStringLiteral( "Ray cast on vector layer" ), 2 );
#ifdef QGISDEBUG
  int nodeUsed = 0;
  int nodesAll = 0;
  int hits = 0;
  int ignoredGeometries = 0;
#endif
  QVector<QgsRayCastingUtils::RayHit> result;

  float minDist = -1;
  QVector3D intersectionPoint;
  QgsFeatureId nearestFid = FID_NULL;

  for ( QgsChunkNode *node : activeNodes )
  {
#ifdef QGISDEBUG
    nodesAll++;
#endif

    QgsAABB nodeBbox = Qgs3DUtils::mapToWorldExtent( node->box3D(), origin );

    if ( node->entity() && ( minDist < 0 || nodeBbox.distanceFromPoint( ray.origin() ) < minDist ) && QgsRayCastingUtils::rayBoxIntersection( ray, nodeBbox ) )
    {
#ifdef QGISDEBUG
      nodeUsed++;
#endif
      const QList<Qt3DRender::QGeometryRenderer *> rendLst = node->entity()->findChildren<Qt3DRender::QGeometryRenderer *>();
      for ( const auto &rend : rendLst )
      {
        auto *geom = rend->geometry();
        QgsTessellatedPolygonGeometry *polygonGeom = qobject_cast<QgsTessellatedPolygonGeometry *>( geom );
        if ( !polygonGeom )
        {
#ifdef QGISDEBUG
          ignoredGeometries++;
#endif
          continue; // other QGeometry types are not supported for now
        }

        QVector3D nodeIntPoint;
        int triangleIndex = -1;

        if ( QgsRayCastingUtils::rayMeshIntersection( rend, ray, transformMatrix, nodeIntPoint, triangleIndex ) )
        {
#ifdef QGISDEBUG
          hits++;
#endif
          float dist = ( ray.origin() - nodeIntPoint ).length();
          if ( minDist < 0 || dist < minDist )
          {
            minDist = dist;
            intersectionPoint = nodeIntPoint;
            nearestFid = polygonGeom->triangleIndexToFeatureId( triangleIndex );
          }
        }
      }
    }
  }
  if ( !FID_IS_NULL( nearestFid ) )
  {
    QgsRayCastingUtils::RayHit hit( minDist, intersectionPoint, nearestFid );
    result.append( hit );
  }
  QgsDebugMsgLevel( QStringLiteral( "Active Nodes: %1, checked nodes: %2, hits found: %3, incompatible geometries: %4" ).arg( nodesAll ).arg( nodeUsed ).arg( hits ).arg( ignoredGeometries ), 2 );
  return result;
}

/// @endcond
