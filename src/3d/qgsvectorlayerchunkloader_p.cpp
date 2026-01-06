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

#include "qgs3dsymbolregistry.h"
#include "qgs3dutils.h"
#include "qgsabstract3dsymbol.h"
#include "qgsabstractterrainsettings.h"
#include "qgsabstractvectorlayer3drenderer.h"
#include "qgsapplication.h"
#include "qgschunknode.h"
#include "qgseventtracing.h"
#include "qgsexpressioncontextutils.h"
#include "qgsgeotransform.h"
#include "qgsline3dsymbol.h"
#include "qgslogger.h"
#include "qgspoint3dsymbol.h"
#include "qgspolygon3dsymbol.h"
#include "qgsray3d.h"
#include "qgsraycastcontext.h"
#include "qgsraycastingutils.h"
#include "qgstessellatedpolygongeometry.h"
#include "qgsvectorlayer.h"
#include "qgsvectorlayerfeatureiterator.h"

#include <Qt3DCore/QTransform>
#include <QtConcurrent>

#include "moc_qgsvectorlayerchunkloader_p.cpp"

///@cond PRIVATE


QgsVectorLayerChunkLoader::QgsVectorLayerChunkLoader( const QgsVectorLayerChunkLoaderFactory *factory, QgsChunkNode *node )
  : QgsChunkLoader( node )
  , mFactory( factory )
  , mRenderContext( factory->mRenderContext )
  , mSource( new QgsVectorLayerFeatureSource( factory->mLayer ) )
{
}

void QgsVectorLayerChunkLoader::start()
{
  QgsChunkNode *node = chunk();
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
    QgsDebugError( u"Unknown 3D symbol type for vector layer: "_s + mFactory->mSymbol->type() );
    return;
  }
  mHandler.reset( handler );

  // only a subset of data to be queried
  const QgsRectangle rect = node->box3D().toRectangle();
  // origin for coordinates of the chunk - it is kind of arbitrary, but it should be
  // picked so that the coordinates are relatively small to avoid numerical precision issues
  QgsVector3D chunkOrigin( rect.center().x(), rect.center().y(), 0 );

  QgsExpressionContext exprContext;
  exprContext.appendScopes( QgsExpressionContextUtils::globalProjectLayerScopes( layer ) );
  exprContext.setFields( layer->fields() );
  mRenderContext.setExpressionContext( exprContext );

  QSet<QString> attributeNames;
  if ( !mHandler->prepare( mRenderContext, attributeNames, chunkOrigin ) )
  {
    QgsDebugError( u"Failed to prepare 3D feature handler!"_s );
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

  const QFuture<void> future = QtConcurrent::run( [req = std::move( req ), this] {
    const QgsEventTracing::ScopedEvent e( u"3D"_s, u"VL chunk load"_s );

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
  if ( context.crs().type() == Qgis::CrsType::Geocentric )
  {
    // TODO: add support for handling of vector layers
    // (we're using dummy quadtree here to make sure the empty extent does not break the scene completely)
    QgsDebugError( u"Vector layers in globe scenes are not supported yet!"_s );
    setupQuadtree( QgsBox3D( -1e7, -1e7, -1e7, 1e7, 1e7, 1e7 ), -1, leafLevel );
    return;
  }

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
    mTransform->setTranslation( QVector3D( 0.0f, 0.0f, static_cast<float>( map->terrainSettings()->elevationOffset() ) ) );
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
      QgsDebugMsgLevel( u"QgsVectorLayerChunkedEntity::applyTerrainOffset, unhandled symbol type %1"_s.arg( symbolType ), 2 );
    }
  }

  return true;
}

void QgsVectorLayerChunkedEntity::onTerrainElevationOffsetChanged()
{
  QgsDebugMsgLevel( u"QgsVectorLayerChunkedEntity::onTerrainElevationOffsetChanged"_s, 2 );
  float newOffset = static_cast<float>( qobject_cast<Qgs3DMapSettings *>( sender() )->terrainSettings()->elevationOffset() );
  if ( !applyTerrainOffset() )
  {
    newOffset = 0.0;
  }
  mTransform->setTranslation( QVector3D( 0.0f, 0.0f, newOffset ) );
}

QList<QgsRayCastHit> QgsVectorLayerChunkedEntity::rayIntersection( const QgsRay3D &ray, const QgsRayCastContext &context ) const
{
  return QgsVectorLayerChunkedEntity::rayIntersection( activeNodes(), mTransform->matrix(), ray, context, mMapSettings->origin() );
}

QList<QgsRayCastHit> QgsVectorLayerChunkedEntity::rayIntersection( const QList<QgsChunkNode *> &activeNodes, const QMatrix4x4 &transformMatrix, const QgsRay3D &ray, const QgsRayCastContext &context, const QgsVector3D &origin )
{
  Q_UNUSED( context )
  QgsDebugMsgLevel( u"Ray cast on vector layer"_s, 2 );
#ifdef QGISDEBUG
  int nodeUsed = 0;
  int nodesAll = 0;
  int hits = 0;
  int ignoredGeometries = 0;
#endif
  QList<QgsRayCastHit> result;

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

        // the node geometry has been translated by chunkOrigin
        // This translation is stored in the QTransform component
        // this needs to be taken into account to get the whole transformation
        const QMatrix4x4 nodeTransformMatrix = node->entity()->findChild<QgsGeoTransform *>()->matrix();
        const QMatrix4x4 fullTransformMatrix = transformMatrix * nodeTransformMatrix;
        if ( QgsRayCastingUtils::rayMeshIntersection( rend, ray, context.maximumDistance(), fullTransformMatrix, nodeIntPoint, triangleIndex ) )
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
    QgsRayCastHit hit;
    hit.setDistance( minDist );
    hit.setMapCoordinates( Qgs3DUtils::worldToMapCoordinates( intersectionPoint, origin ) );
    hit.setProperties( { { u"fid"_s, nearestFid } } );
    result.append( hit );
  }
  QgsDebugMsgLevel( u"Active Nodes: %1, checked nodes: %2, hits found: %3, incompatible geometries: %4"_s.arg( nodesAll ).arg( nodeUsed ).arg( hits ).arg( ignoredGeometries ), 2 );
  return result;
}

/// @endcond
