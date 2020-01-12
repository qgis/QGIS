/***************************************************************************
  qgsrulebasedchunkloader_p.cpp
  --------------------------------------
  Date                 : November 2019
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

#include "qgsrulebasedchunkloader_p.h"

#include "qgs3dutils.h"
#include "qgschunknode_p.h"
#include "qgspolygon3dsymbol_p.h"
#include "qgseventtracing.h"

#include "qgsvectorlayer.h"
#include "qgsvectorlayerfeatureiterator.h"

#include "qgsrulebased3drenderer.h"

#include <QtConcurrent>

///@cond PRIVATE


QgsRuleBasedChunkLoader::QgsRuleBasedChunkLoader( const QgsRuleBasedChunkLoaderFactory *factory, QgsChunkNode *node )
  : QgsChunkLoader( node )
  , mFactory( factory )
  , mContext( factory->mMap )
  , mSource( new QgsVectorLayerFeatureSource( factory->mLayer ) )
{
  if ( node->level() < mFactory->mLeafLevel )
  {
    QTimer::singleShot( 0, this, &QgsRuleBasedChunkLoader::finished );
    return;
  }

  QgsVectorLayer *layer = mFactory->mLayer;
  const Qgs3DMapSettings &map = mFactory->mMap;

  QgsExpressionContext exprContext( Qgs3DUtils::globalProjectLayerExpressionContext( layer ) );
  exprContext.setFields( layer->fields() );
  mContext.setExpressionContext( exprContext );

  // factory is shared among multiple loaders which may be run at the same time
  // so we need a local copy of our rule tree that does not intefere with others
  // (e.g. it happened that filter expressions with invalid syntax would cause
  // nasty crashes when trying to simultaneously record evaluation error)
  mRootRule.reset( mFactory->mRootRule->clone() );

  mRootRule->createHandlers( layer, mHandlers );

  QSet<QString> attributeNames;
  mRootRule->prepare( mContext, attributeNames, mHandlers );

  // build the feature request
  QgsFeatureRequest req;
  req.setDestinationCrs( map.crs(), map.transformContext() );
  req.setSubsetOfAttributes( attributeNames, layer->fields() );

  // only a subset of data to be queried
  QgsRectangle rect = Qgs3DUtils::worldToLayerExtent( node->bbox(), mFactory->mLayer->crs(), map.origin(), map.crs(), map.transformContext() );
  req.setFilterRect( rect );

  //
  // this will be run in a background thread
  //

  QFuture<void> future = QtConcurrent::run( [req, this]
  {
    QgsEventTracing::ScopedEvent e( "3D", "RB chunk load" );

    QgsFeature f;
    QgsFeatureIterator fi = mSource->getFeatures( req );
    while ( fi.nextFeature( f ) )
    {
      if ( mCanceled )
        break;
      mContext.expressionContext().setFeature( f );
      mRootRule->registerFeature( f, mContext, mHandlers );
    }
  } );

  // emit finished() as soon as the handler is populated with features
  mFutureWatcher = new QFutureWatcher<void>( this );
  mFutureWatcher->setFuture( future );
  connect( mFutureWatcher, &QFutureWatcher<void>::finished, this, &QgsChunkQueueJob::finished );
}

QgsRuleBasedChunkLoader::~QgsRuleBasedChunkLoader()
{
  if ( mFutureWatcher && !mFutureWatcher->isFinished() )
  {
    disconnect( mFutureWatcher, &QFutureWatcher<void>::finished, this, &QgsChunkQueueJob::finished );
    mFutureWatcher->waitForFinished();
  }

  qDeleteAll( mHandlers );
  mHandlers.clear();
}

void QgsRuleBasedChunkLoader::cancel()
{
  mCanceled = true;
}

Qt3DCore::QEntity *QgsRuleBasedChunkLoader::createEntity( Qt3DCore::QEntity *parent )
{
  if ( mNode->level() < mFactory->mLeafLevel )
  {
    return new Qt3DCore::QEntity( parent );  // dummy entity
  }

  float zMin = std::numeric_limits<float>::max();
  float zMax = std::numeric_limits<float>::min();

  Qt3DCore::QEntity *entity = new Qt3DCore::QEntity( parent );
  for ( QgsFeature3DHandler *handler : mHandlers.values() )
  {
    handler->finalize( entity, mContext );
    if ( handler->zMinimum() < zMin )
      zMin = handler->zMinimum();
    if ( handler->zMaximum() > zMax )
      zMax = handler->zMaximum();
  }

  // fix the vertical range of the node from the estimated vertical range to the true range
  if ( zMin != std::numeric_limits<float>::max() && zMax != std::numeric_limits<float>::min() )
  {
    QgsAABB box = mNode->bbox();
    box.yMin = zMin;
    box.yMax = zMax;
    mNode->setExactBbox( box );
  }

  return entity;
}


///////////////


QgsRuleBasedChunkLoaderFactory::QgsRuleBasedChunkLoaderFactory( const Qgs3DMapSettings &map, QgsVectorLayer *vl, QgsRuleBased3DRenderer::Rule *rootRule, int leafLevel )
  : mMap( map )
  , mLayer( vl )
  , mRootRule( rootRule->clone() )
  , mLeafLevel( leafLevel )
{
}

QgsRuleBasedChunkLoaderFactory::~QgsRuleBasedChunkLoaderFactory()
{
}

QgsChunkLoader *QgsRuleBasedChunkLoaderFactory::createChunkLoader( QgsChunkNode *node ) const
{
  return new QgsRuleBasedChunkLoader( this, node );
}


///////////////

QgsRuleBasedChunkedEntity::QgsRuleBasedChunkedEntity( QgsVectorLayer *vl, double zMin, double zMax, const QgsVectorLayer3DTilingSettings &tilingSettings, QgsRuleBased3DRenderer::Rule *rootRule, const Qgs3DMapSettings &map )
  : QgsChunkedEntity( Qgs3DUtils::layerToWorldExtent( vl->extent(), zMin, zMax, vl->crs(), map.origin(), map.crs(), map.transformContext() ),
                      -1,  // rootError  TODO: negative error should mean that the node does not contain anything
                      -1, // tau = max. allowed screen error. TODO: negative tau should mean that we need to go until leaves are reached
                      tilingSettings.zoomLevelsCount() - 1,
                      new QgsRuleBasedChunkLoaderFactory( map, vl, rootRule, tilingSettings.zoomLevelsCount() - 1 ) )
{
  setShowBoundingBoxes( true );
}

QgsRuleBasedChunkedEntity::~QgsRuleBasedChunkedEntity()
{
  // cancel / wait for jobs
  cancelActiveJobs();
}

/// @endcond
