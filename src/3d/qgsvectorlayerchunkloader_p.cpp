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

#include "qgsapplication.h"
#include "qgs3dsymbolregistry.h"

#include <QtConcurrent>
#include <Qt3DCore/QTransform>

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

  QgsFeature3DHandler *handler = QgsApplication::symbol3DRegistry()->createHandlerForSymbol( layer, mFactory->mSymbol.get() );
  if ( !handler )
  {
    QgsDebugMsg( QStringLiteral( "Unknown 3D symbol type for vector layer: " ) + mFactory->mSymbol->type() );
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
  const QgsRectangle rect = Qgs3DUtils::worldToMapExtent( node->bbox(), map.origin() );
  req.setFilterRect( rect );

  //
  // this will be run in a background thread
  //
  mFutureWatcher = new QFutureWatcher<void>( this );
  connect( mFutureWatcher, &QFutureWatcher<void>::finished, this, &QgsChunkQueueJob::finished );

  const QFuture<void> future = QtConcurrent::run( [req, this]
  {
    const QgsEventTracing::ScopedEvent e( QStringLiteral( "3D" ), QStringLiteral( "VL chunk load" ) );

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
    return new Qt3DCore::QEntity( parent );  // dummy entity
  }

  if ( mHandler->featureCount() == 0 )
  {
    // an empty node, so we return no entity. This tags the node as having no data and effectively removes it.
    return nullptr;
  }

  Qt3DCore::QEntity *entity = new Qt3DCore::QEntity( parent );
  mHandler->finalize( entity, mContext );

  // fix the vertical range of the node from the estimated vertical range to the true range
  if ( mHandler->zMinimum() != std::numeric_limits<float>::max() && mHandler->zMaximum() != std::numeric_limits<float>::lowest() )
  {
    QgsAABB box = mNode->bbox();
    box.yMin = mHandler->zMinimum();
    box.yMax = mHandler->zMaximum();
    mNode->setExactBbox( box );
    mNode->updateParentBoundingBoxesRecursively();
  }

  return entity;
}


///////////////


QgsVectorLayerChunkLoaderFactory::QgsVectorLayerChunkLoaderFactory( const Qgs3DMapSettings &map, QgsVectorLayer *vl, QgsAbstract3DSymbol *symbol, int leafLevel, double zMin, double zMax )
  : mMap( map )
  , mLayer( vl )
  , mSymbol( symbol->clone() )
  , mLeafLevel( leafLevel )
{
  QgsAABB rootBbox = Qgs3DUtils::layerToWorldExtent( vl->extent(), zMin, zMax, vl->crs(), map.origin(), map.crs(), map.transformContext() );
  // add small padding to avoid clipping of point features located at the edge of the bounding box
  rootBbox.xMin -= 1.0;
  rootBbox.xMax += 1.0;
  rootBbox.yMin -= 1.0;
  rootBbox.yMax += 1.0;
  rootBbox.zMin -= 1.0;
  rootBbox.zMax += 1.0;
  setupQuadtree( rootBbox, -1, leafLevel );  // negative root error means that the node does not contain anything
}

QgsChunkLoader *QgsVectorLayerChunkLoaderFactory::createChunkLoader( QgsChunkNode *node ) const
{
  return new QgsVectorLayerChunkLoader( this, node );
}


///////////////


QgsVectorLayerChunkedEntity::QgsVectorLayerChunkedEntity( QgsVectorLayer *vl, double zMin, double zMax, const QgsVectorLayer3DTilingSettings &tilingSettings, QgsAbstract3DSymbol *symbol, const Qgs3DMapSettings &map )
  : QgsChunkedEntity( -1, // max. allowed screen error (negative tau means that we need to go until leaves are reached)
                      new QgsVectorLayerChunkLoaderFactory( map, vl, symbol, tilingSettings.zoomLevelsCount() - 1, zMin, zMax ), true )
{
  mTransform = new Qt3DCore::QTransform;
  mTransform->setTranslation( QVector3D( 0.0f, map.terrainElevationOffset(), 0.0f ) );
  this->addComponent( mTransform );

  connect( &map, &Qgs3DMapSettings::terrainElevationOffsetChanged, this, &QgsVectorLayerChunkedEntity::onTerrainElevationOffsetChanged );

  setShowBoundingBoxes( tilingSettings.showBoundingBoxes() );
}

QgsVectorLayerChunkedEntity::~QgsVectorLayerChunkedEntity()
{
  // cancel / wait for jobs
  cancelActiveJobs();
}

void QgsVectorLayerChunkedEntity::onTerrainElevationOffsetChanged( float newOffset )
{
  QgsDebugMsgLevel( QStringLiteral( "QgsVectorLayerChunkedEntity::onTerrainElevationOffsetChanged" ), 2 );
  mTransform->setTranslation( QVector3D( 0.0f, newOffset, 0.0f ) );
}

/// @endcond
