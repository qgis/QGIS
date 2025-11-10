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
#include "moc_qgsrulebasedchunkloader_p.cpp"
#include "qgsvectorlayerchunkloader_p.h"

#include "qgs3dutils.h"
#include "qgsline3dsymbol.h"
#include "qgspoint3dsymbol.h"
#include "qgspolygon3dsymbol.h"
#include "qgschunknode.h"
#include "qgseventtracing.h"

#include "qgsvectorlayer.h"
#include "qgsvectorlayerfeatureiterator.h"

#include "qgsrulebased3drenderer.h"
#include "qgstessellatedpolygongeometry.h"
#include "qgsabstractterrainsettings.h"
#include "qgsexpressioncontextutils.h"
#include "qgsfeature3dhandler_p.h"

#include <QtConcurrent>
#include <Qt3DCore/QTransform>

///@cond PRIVATE


QgsRuleBasedChunkLoader::QgsRuleBasedChunkLoader( const QgsRuleBasedChunkLoaderFactory *factory, QgsChunkNode *node )
  : QgsChunkLoader( node )
  , mFactory( factory )
  , mContext( factory->mRenderContext )
  , mSource( new QgsVectorLayerFeatureSource( factory->mLayer ) )
{
}

void QgsRuleBasedChunkLoader::start()
{
  QgsChunkNode *node = chunk();

  QgsVectorLayer *layer = mFactory->mLayer;

  // only a subset of data to be queried
  const QgsRectangle rect = node->box3D().toRectangle();
  // origin for coordinates of the chunk - it is kind of arbitrary, but it should be
  // picked so that the coordinates are relatively small to avoid numerical precision issues
  QgsVector3D chunkOrigin( rect.center().x(), rect.center().y(), 0 );

  QgsExpressionContext exprContext;
  exprContext.appendScopes( QgsExpressionContextUtils::globalProjectLayerScopes( layer ) );
  exprContext.setFields( layer->fields() );
  mContext.setExpressionContext( exprContext );

  // factory is shared among multiple loaders which may be run at the same time
  // so we need a local copy of our rule tree that does not intefere with others
  // (e.g. it happened that filter expressions with invalid syntax would cause
  // nasty crashes when trying to simultaneously record evaluation error)
  mRootRule.reset( mFactory->mRootRule->clone() );

  mRootRule->createHandlers( layer, mHandlers );

  QSet<QString> attributeNames;
  mRootRule->prepare( mContext, attributeNames, chunkOrigin, mHandlers );

  // build the feature request
  QgsFeatureRequest req;
  req.setDestinationCrs( mContext.crs(), mContext.transformContext() );
  req.setSubsetOfAttributes( attributeNames, layer->fields() );
  req.setFilterRect( rect );

  //
  // this will be run in a background thread
  //
  mFutureWatcher = new QFutureWatcher<void>( this );

  connect( mFutureWatcher, &QFutureWatcher<void>::finished, this, [this] {
    if ( !mCanceled )
      mFactory->mNodesAreLeafs[mNode->tileId().text()] = mNodeIsLeaf;
  } );

  connect( mFutureWatcher, &QFutureWatcher<void>::finished, this, &QgsChunkQueueJob::finished );

  const QFuture<void> future = QtConcurrent::run( [req = std::move( req ), this] {
    const QgsEventTracing::ScopedEvent e( QStringLiteral( "3D" ), QStringLiteral( "RB chunk load" ) );

    QgsFeature f;
    QgsFeatureIterator fi = mSource->getFeatures( req );
    int fc = 0;
    bool featureLimitReached = false;
    while ( fi.nextFeature( f ) )
    {
      if ( mCanceled )
        break;

      if ( ++fc > mFactory->mMaxFeatures )
      {
        featureLimitReached = true;
        break;
      }

      mContext.expressionContext().setFeature( f );
      mRootRule->registerFeature( f, mContext, mHandlers );
    }
    if ( !featureLimitReached )
    {
      QgsDebugMsgLevel( QStringLiteral( "All features fetched for node: %1" ).arg( mNode->tileId().text() ), 3 );
      mNodeIsLeaf = true;
    }
  } );

  // emit finished() as soon as the handler is populated with features
  mFutureWatcher->setFuture( future );
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
  long long featureCount = 0;
  for ( auto it = mHandlers.constBegin(); it != mHandlers.constEnd(); ++it )
  {
    featureCount += it.value()->featureCount();
  }
  if ( featureCount == 0 )
  {
    // an empty node, so we return no entity. This tags the node as having no data and effectively removes it.
    return nullptr;
  }

  Qt3DCore::QEntity *entity = new Qt3DCore::QEntity( parent );
  float zMin = std::numeric_limits<float>::max();
  float zMax = std::numeric_limits<float>::lowest();
  for ( auto it = mHandlers.constBegin(); it != mHandlers.constEnd(); ++it )
  {
    QgsFeature3DHandler *handler = it.value();
    handler->finalize( entity, mContext );
    if ( handler->zMinimum() < zMin )
      zMin = handler->zMinimum();
    if ( handler->zMaximum() > zMax )
      zMax = handler->zMaximum();
  }

  // fix the vertical range of the node from the estimated vertical range to the true range
  if ( zMin != std::numeric_limits<float>::max() && zMax != std::numeric_limits<float>::lowest() )
  {
    QgsBox3D box = mNode->box3D();
    box.setZMinimum( zMin );
    box.setZMaximum( zMax );
    mNode->setExactBox3D( box );
    mNode->updateParentBoundingBoxesRecursively();
  }

  return entity;
}


///////////////


QgsRuleBasedChunkLoaderFactory::QgsRuleBasedChunkLoaderFactory( const Qgs3DRenderContext &context, QgsVectorLayer *vl, QgsRuleBased3DRenderer::Rule *rootRule, double zMin, double zMax, int maxFeatures )
  : mRenderContext( context )
  , mLayer( vl )
  , mRootRule( rootRule->clone() )
  , mMaxFeatures( maxFeatures )
{
  if ( context.crs().type() == Qgis::CrsType::Geocentric )
  {
    // TODO: add support for handling of vector layers
    // (we're using dummy quadtree here to make sure the empty extent does not break the scene completely)
    QgsDebugError( QStringLiteral( "Vector layers in globe scenes are not supported yet!" ) );
    setupQuadtree( QgsBox3D( -1e7, -1e7, -1e7, 1e7, 1e7, 1e7 ), -1, 3 );
    return;
  }

  const QgsBox3D rootBox3D( context.extent(), zMin, zMax );
  const float rootError = static_cast<float>( std::max<double>( rootBox3D.width(), rootBox3D.height() ) );
  setupQuadtree( rootBox3D, rootError );
}

QgsRuleBasedChunkLoaderFactory::~QgsRuleBasedChunkLoaderFactory() = default;

QgsChunkLoader *QgsRuleBasedChunkLoaderFactory::createChunkLoader( QgsChunkNode *node ) const
{
  return new QgsRuleBasedChunkLoader( this, node );
}

bool QgsRuleBasedChunkLoaderFactory::canCreateChildren( QgsChunkNode *node )
{
  return mNodesAreLeafs.contains( node->tileId().text() );
}

QVector<QgsChunkNode *> QgsRuleBasedChunkLoaderFactory::createChildren( QgsChunkNode *node ) const
{
  if ( mNodesAreLeafs.value( node->tileId().text(), false ) )
    return {};

  return QgsQuadtreeChunkLoaderFactory::createChildren( node );
}

///////////////

QgsRuleBasedChunkedEntity::QgsRuleBasedChunkedEntity( Qgs3DMapSettings *map, QgsVectorLayer *vl, double zMin, double zMax, const QgsVectorLayer3DTilingSettings &tilingSettings, QgsRuleBased3DRenderer::Rule *rootRule )
  : QgsChunkedEntity( map, tilingSettings.maximumScreenError(), new QgsRuleBasedChunkLoaderFactory( Qgs3DRenderContext::fromMapSettings( map ), vl, rootRule, zMin, zMax, tilingSettings.maximumChunkFeatures() ), true )
{
  mTransform = new Qt3DCore::QTransform;
  if ( applyTerrainOffset() )
  {
    mTransform->setTranslation( QVector3D( 0.0f, 0.0f, static_cast<float>( map->terrainSettings()->elevationOffset() ) ) );
  }
  this->addComponent( mTransform );
  connect( map, &Qgs3DMapSettings::terrainSettingsChanged, this, &QgsRuleBasedChunkedEntity::onTerrainElevationOffsetChanged );

  setShowBoundingBoxes( tilingSettings.showBoundingBoxes() );
}

QgsRuleBasedChunkedEntity::~QgsRuleBasedChunkedEntity()
{
  // cancel / wait for jobs
  cancelActiveJobs();
}

// if the AltitudeClamping is `Absolute`, do not apply the offset
bool QgsRuleBasedChunkedEntity::applyTerrainOffset() const
{
  QgsRuleBasedChunkLoaderFactory *loaderFactory = static_cast<QgsRuleBasedChunkLoaderFactory *>( mChunkLoaderFactory );
  if ( loaderFactory )
  {
    QgsRuleBased3DRenderer::Rule *rootRule = loaderFactory->mRootRule.get();
    const QgsRuleBased3DRenderer::RuleList rules = rootRule->children();
    for ( const auto &rule : rules )
    {
      if ( rule->symbol() )
      {
        QString symbolType = rule->symbol()->type();
        if ( symbolType == "line" )
        {
          QgsLine3DSymbol *lineSymbol = static_cast<QgsLine3DSymbol *>( rule->symbol() );
          if ( lineSymbol && lineSymbol->altitudeClamping() == Qgis::AltitudeClamping::Absolute )
          {
            return false;
          }
        }
        else if ( symbolType == "point" )
        {
          QgsPoint3DSymbol *pointSymbol = static_cast<QgsPoint3DSymbol *>( rule->symbol() );
          if ( pointSymbol && pointSymbol->altitudeClamping() == Qgis::AltitudeClamping::Absolute )
          {
            return false;
          }
        }
        else if ( symbolType == "polygon" )
        {
          QgsPolygon3DSymbol *polygonSymbol = static_cast<QgsPolygon3DSymbol *>( rule->symbol() );
          if ( polygonSymbol && polygonSymbol->altitudeClamping() == Qgis::AltitudeClamping::Absolute )
          {
            return false;
          }
        }
        else
        {
          QgsDebugMsgLevel( QStringLiteral( "QgsRuleBasedChunkedEntityChunkedEntity::applyTerrainOffset, unhandled symbol type %1" ).arg( symbolType ), 2 );
        }
      }
    }
  }

  return true;
}

void QgsRuleBasedChunkedEntity::onTerrainElevationOffsetChanged()
{
  const float previousOffset = mTransform->translation()[1];
  float newOffset = static_cast<float>( qobject_cast<Qgs3DMapSettings *>( sender() )->terrainSettings()->elevationOffset() );
  if ( !applyTerrainOffset() )
  {
    newOffset = 0.0;
  }

  if ( newOffset != previousOffset )
  {
    mTransform->setTranslation( QVector3D( 0.0f, 0.0f, newOffset ) );
  }
}

QList<QgsRayCastHit> QgsRuleBasedChunkedEntity::rayIntersection( const QgsRay3D &ray, const QgsRayCastContext &context ) const
{
  return QgsVectorLayerChunkedEntity::rayIntersection( activeNodes(), mTransform->matrix(), ray, context, mMapSettings->origin() );
}
/// @endcond
