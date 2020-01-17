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

#include "qgs3dutils.h"
#include "qgsabstractvectorlayer3drenderer.h"
#include "qgschunknode_p.h"
#include "qgspolygon3dsymbol_p.h"
#include "qgseventtracing.h"
#include "qgslogger.h"
#include "qgsvectorlayer.h"
#include "qgsvectorlayerfeatureiterator.h"

#include "qgsline3dsymbol.h"
#include "qgspoint3dsymbol.h"
#include "qgspolygon3dsymbol.h"

#include "qgsline3dsymbol_p.h"
#include "qgspoint3dsymbol_p.h"
#include "qgspolygon3dsymbol_p.h"

#include <QtConcurrent>

///@cond PRIVATE


QgsVectorLayerChunkLoader::QgsVectorLayerChunkLoader( const QgsVectorLayerChunkLoaderFactory *factory, QgsChunkNode *node )
  : QgsChunkLoader( node )
  , mFactory( factory )
  , mContext( factory->mMap )
  , mSource( new QgsVectorLayerFeatureSource( factory->mLayer ) )
{
  if ( node->level() < mFactory->mLeafLevel )
  {
    QTimer::singleShot( 0, this, &QgsVectorLayerChunkLoader::finished );
    return;
  }

  QgsVectorLayer *layer = mFactory->mLayer;
  const Qgs3DMapSettings &map = mFactory->mMap;

  QgsFeature3DHandler *handler = nullptr;
  QString symbolType = mFactory->mSymbol->type();
  if ( symbolType == QLatin1String( "polygon" ) )
    handler = Qgs3DSymbolImpl::handlerForPolygon3DSymbol( layer, *static_cast<QgsPolygon3DSymbol *>( mFactory->mSymbol.get() ) );
  else if ( symbolType == QLatin1String( "point" ) )
    handler = Qgs3DSymbolImpl::handlerForPoint3DSymbol( layer, *static_cast<QgsPoint3DSymbol *>( mFactory->mSymbol.get() ) );
  else if ( symbolType == QLatin1String( "line" ) )
    handler = Qgs3DSymbolImpl::handlerForLine3DSymbol( layer, *static_cast<QgsLine3DSymbol *>( mFactory->mSymbol.get() ) );
  else
  {
    QgsDebugMsg( QStringLiteral( "Unknown 3D symbol type for vector layer: " ) + symbolType );
    return;
  }
  mHandler.reset( handler );

  QgsExpressionContext exprContext( Qgs3DUtils::globalProjectLayerExpressionContext( layer ) );
  exprContext.setFields( layer->fields() );
  mContext.setExpressionContext( exprContext );

  QSet<QString> attributeNames;
  if ( !mHandler->prepare( mContext, attributeNames ) )
  {
    QgsDebugMsg( QStringLiteral( "Failed to prepare 3D feature handler!" ) );
    return;
  }

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
    QgsEventTracing::ScopedEvent e( QStringLiteral( "3D" ), QStringLiteral( "VL chunk load" ) );

    QgsFeature f;
    QgsFeatureIterator fi = mSource->getFeatures( req );
    while ( fi.nextFeature( f ) )
    {
      if ( mCanceled )
        break;
      mContext.expressionContext().setFeature( f );
      mHandler->processFeature( f, mContext );
    }
  } );

  // emit finished() as soon as the handler is populated with features
  mFutureWatcher = new QFutureWatcher<void>( this );
  mFutureWatcher->setFuture( future );
  connect( mFutureWatcher, &QFutureWatcher<void>::finished, this, &QgsChunkQueueJob::finished );
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
    return new Qt3DCore::QEntity( parent );  // dummy entity
  }

  Qt3DCore::QEntity *entity = new Qt3DCore::QEntity( parent );
  mHandler->finalize( entity, mContext );
  return entity;
}


///////////////


QgsVectorLayerChunkLoaderFactory::QgsVectorLayerChunkLoaderFactory( const Qgs3DMapSettings &map, QgsVectorLayer *vl, QgsAbstract3DSymbol *symbol, int leafLevel )
  : mMap( map )
  , mLayer( vl )
  , mSymbol( symbol->clone() )
  , mLeafLevel( leafLevel )
{
}

QgsChunkLoader *QgsVectorLayerChunkLoaderFactory::createChunkLoader( QgsChunkNode *node ) const
{
  return new QgsVectorLayerChunkLoader( this, node );
}


///////////////


QgsVectorLayerChunkedEntity::QgsVectorLayerChunkedEntity( QgsVectorLayer *vl, double zMin, double zMax, const QgsVectorLayer3DTilingSettings &tilingSettings, QgsAbstract3DSymbol *symbol, const Qgs3DMapSettings &map )
  : QgsChunkedEntity( Qgs3DUtils::layerToWorldExtent( vl->extent(), zMin, zMax, vl->crs(), map.origin(), map.crs(), map.transformContext() ),
                      -1, // rootError (negative error means that the node does not contain anything)
                      -1, // max. allowed screen error (negative tau means that we need to go until leaves are reached)
                      tilingSettings.zoomLevelsCount() - 1,
                      new QgsVectorLayerChunkLoaderFactory( map, vl, symbol, tilingSettings.zoomLevelsCount() - 1 ) )
{
  setShowBoundingBoxes( tilingSettings.showBoundingBoxes() );
}

QgsVectorLayerChunkedEntity::~QgsVectorLayerChunkedEntity()
{
  // cancel / wait for jobs
  cancelActiveJobs();
}

/// @endcond
