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
#include "qgseventtracing.h"
#include "qgsabstractvectorlayer3drenderer.h" // for QgsVectorLayer3DTilingSettings

#include "qgspoint3dsymbol.h"

#include "qgsapplication.h"
#include "qgs3dsymbolregistry.h"

#include <QtConcurrent>

///@cond PRIVATE

QgsPointCloud3DSymbolHandler::QgsPointCloud3DSymbolHandler()
{

}

bool QgsPointCloud3DSymbolHandler::prepare( const Qgs3DRenderContext &context )
{
  return true;
}

void QgsPointCloud3DSymbolHandler::processFeature( const QVector3D &point, const Qgs3DRenderContext &context )
{
  mZMin = std::min( mZMin, point.z() );
  mZMax = std::max( mZMax, point.z() );
  outNormal.positions.push_back( point );
}

void QgsPointCloud3DSymbolHandler::finalize( Qt3DCore::QEntity *parent, const Qgs3DRenderContext &context )
{
  makeEntity( parent, context, outNormal, false );
  // makeEntity( parent, context, outSelected, true );

  // updateZRangeFromPositions( outNormal.positions );
  // updateZRangeFromPositions( outSelected.positions );

  // the elevation offset is applied separately in QTransform added to sub-entities
  //float symbolHeight = mSymbol->transform().data()[13];
  //mZMin += symbolHeight;
  //mZMax += symbolHeight;
}

void QgsPointCloud3DSymbolHandler::makeEntity( Qt3DCore::QEntity *parent, const Qgs3DRenderContext &context, QgsPointCloud3DSymbolHandler::PointData &out, bool selected )
{
  if ( out.positions.empty() )
    return;

  // build the default material
  QgsMaterialContext materialContext;
  materialContext.setIsSelected( selected );
  materialContext.setSelectionColor( context.map().selectionColor() );
  /*
  Qt3DRender::QMaterial *mat = symbol->material()->toMaterial( QgsMaterialSettingsRenderingTechnique::Triangles, materialContext );

  // get nodes
  for ( const QVector3D &position : positions )
  {
    const QString source = QgsApplication::instance()->sourceCache()->localFilePath( symbol->shapeProperties()[QStringLiteral( "model" )].toString() );
    if ( !source.isEmpty() )
    {
      // build the entity
      Qt3DCore::QEntity *entity = new Qt3DCore::QEntity;

      QUrl url = QUrl::fromLocalFile( source );
      Qt3DRender::QMesh *mesh = new Qt3DRender::QMesh;
      mesh->setSource( url );

      entity->addComponent( mesh );
      entity->addComponent( mat );
      entity->addComponent( transform( position, symbol ) );
      entity->setParent( parent );

  // cppcheck wrongly believes entity will leak
  // cppcheck-suppress memleak
    }
  }
  */
}


QgsPointCloudLayerChunkLoader::QgsPointCloudLayerChunkLoader( const QgsPointCloudLayerChunkLoaderFactory *factory, QgsChunkNode *node )
  : QgsChunkLoader( node )
  , mFactory( factory )
  , mContext( factory->mMap )
    // , mSource( new QgsPointCloudLayerFeatureSource( factory->mLayer ) )
{
  if ( node->level() < mFactory->mLeafLevel )
  {
    QTimer::singleShot( 0, this, &QgsPointCloudLayerChunkLoader::finished );
    return;
  }

  QgsPointCloudLayer *layer = mFactory->mLayer;
  const Qgs3DMapSettings &map = mFactory->mMap;

  QgsPointCloud3DSymbolHandler *handler = new QgsPointCloud3DSymbolHandler;
  mHandler.reset( handler );

  //QgsExpressionContext exprContext( Qgs3DUtils::globalProjectLayerExpressionContext( layer ) );
  //exprContext.setFields( layer->fields() );
  //mContext.setExpressionContext( exprContext );

  //QSet<QString> attributeNames;
  //if ( !mHandler->prepare( mContext, attributeNames ) )
  // {
  //  QgsDebugMsg( QStringLiteral( "Failed to prepare 3D feature handler!" ) );
  //  return;
  //}

  // build the feature request
  //QgsFeatureRequest req;
  //req.setDestinationCrs( map.crs(), map.transformContext() );
  //req.setSubsetOfAttributes( attributeNames, layer->fields() );

  // only a subset of data to be queried
  //QgsRectangle rect = Qgs3DUtils::worldToMapExtent( node->bbox(), map.origin() );
  //req.setFilterRect( rect );

  //
  // this will be run in a background thread
  //
  /*
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
        // mHandler->processFeature( f, mContext );
      }
    } );

    // emit finished() as soon as the handler is populated with features
    mFutureWatcher = new QFutureWatcher<void>( this );
    mFutureWatcher->setFuture( future );
    connect( mFutureWatcher, &QFutureWatcher<void>::finished, this, &QgsChunkQueueJob::finished );
  */
}

QgsPointCloudLayerChunkLoader::~QgsPointCloudLayerChunkLoader()
{
  if ( mFutureWatcher && !mFutureWatcher->isFinished() )
  {
    disconnect( mFutureWatcher, &QFutureWatcher<void>::finished, this, &QgsChunkQueueJob::finished );
    mFutureWatcher->waitForFinished();
  }
}

void QgsPointCloudLayerChunkLoader::cancel()
{
  mCanceled = true;
}

Qt3DCore::QEntity *QgsPointCloudLayerChunkLoader::createEntity( Qt3DCore::QEntity *parent )
{
  if ( mNode->level() < mFactory->mLeafLevel )
  {
    return new Qt3DCore::QEntity( parent );  // dummy entity
  }

  Qt3DCore::QEntity *entity = new Qt3DCore::QEntity( parent );
  // mHandler->finalize( entity, mContext );
  return entity;
}


///////////////


QgsPointCloudLayerChunkLoaderFactory::QgsPointCloudLayerChunkLoaderFactory( const Qgs3DMapSettings &map, QgsPointCloudLayer *vl, int leafLevel )
  : mMap( map )
  , mLayer( vl )
  , mLeafLevel( leafLevel )
{
}

QgsChunkLoader *QgsPointCloudLayerChunkLoaderFactory::createChunkLoader( QgsChunkNode *node ) const
{
  return new QgsPointCloudLayerChunkLoader( this, node );
}


///////////////

QgsPointCloudLayerChunkedEntity::QgsPointCloudLayerChunkedEntity( QgsPointCloudLayer *vl, double zMin, double zMax, const Qgs3DMapSettings &map )
  : QgsChunkedEntity( Qgs3DUtils::layerToWorldExtent( vl->extent(), zMin, zMax, vl->crs(), map.origin(), map.crs(), map.transformContext() ),
                      -1, // rootError (negative error means that the node does not contain anything)
                      -1, // max. allowed screen error (negative tau means that we need to go until leaves are reached)
                      QgsVectorLayer3DTilingSettings().zoomLevelsCount() - 1,
                      new QgsPointCloudLayerChunkLoaderFactory( map, vl, QgsVectorLayer3DTilingSettings().zoomLevelsCount() - 1 ), true )
{
  setShowBoundingBoxes( QgsVectorLayer3DTilingSettings().showBoundingBoxes() );
}

QgsPointCloudLayerChunkedEntity::~QgsPointCloudLayerChunkedEntity()
{
  // cancel / wait for jobs
  cancelActiveJobs();
}

/// @endcond
