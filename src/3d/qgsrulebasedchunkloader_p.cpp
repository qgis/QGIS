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
#include "qgsabstractterrainsettings.h"
#include "qgschunknode.h"
#include "qgseventtracing.h"
#include "qgsexpressioncontextutils.h"
#include "qgsfeature3dhandler_p.h"
#include "qgsline3dsymbol.h"
#include "qgspoint3dsymbol.h"
#include "qgspolygon3dsymbol.h"
#include "qgsrulebased3drenderer.h"
#include "qgsthreadingutils.h"
#include "qgsvectorlayer.h"
#include "qgsvectorlayerchunkloader_p.h"
#include "qgsvectorlayerfeatureiterator.h"

#include <QMutex>
#include <QString>
#include <Qt3DCore/QTransform>
#include <QtConcurrentRun>

#include "moc_qgsrulebasedchunkloader_p.cpp"

using namespace Qt::StringLiterals;

///@cond PRIVATE


QgsRuleBasedChunkLoader::QgsRuleBasedChunkLoader( const Qgs3DRenderContext &context, QgsVectorLayer *vl, QgsRuleBased3DRenderer::Rule *rootRule, double zMin, double zMax, int maxFeatures )
  : mRenderContext( context )
  , mLayer( vl )
  , mRootRule( rootRule->clone() )
  , mMaxFeatures( maxFeatures )
{
  if ( context.crs().type() == Qgis::CrsType::Geocentric )
  {
    // TODO: add support for handling of vector layers
    // (we're using dummy quadtree here to make sure the empty extent does not break the scene completely)
    QgsDebugError( u"Vector layers in globe scenes are not supported yet!"_s );
    setupQuadtree( QgsBox3D( -7e6, -7e6, -7e6, 7e6, 7e6, 7e6 ), -1, 3 );
    return;
  }

  // choose the smaller root extent between context and mLayer ones:
  QgsRectangle extent = context.extent();
  const QgsRectangle layerExtentInMapCrs = Qgs3DUtils::tryReprojectExtent2D( mLayer->extent(), mLayer->crs(), context.crs(), context.transformContext() );
  if ( layerExtentInMapCrs.isValid() )
  {
    extent = context.extent().intersect( layerExtentInMapCrs );
  }
  if ( extent.isValid() )
  {
    QgsBox3D rootBox3D( extent, zMin, zMax );
    // add small padding to avoid clipping of point features located at the edge of the bounding box
    rootBox3D.grow( 1.0 );

    const float rootError = static_cast<float>( std::max<double>( rootBox3D.width(), rootBox3D.height() ) * QgsVectorLayer3DTilingSettings::tileGeometryErrorRatio() );
    setupQuadtree( rootBox3D, rootError );
  }
}

QgsRuleBasedChunkLoader::~QgsRuleBasedChunkLoader() = default;

QFuture<QgsChunkLoaderResult> QgsRuleBasedChunkLoader::loadChunk( QgsChunkNode *node )
{
  QgsExpressionContext exprContext;
  exprContext.appendScopes( QgsExpressionContextUtils::globalProjectLayerScopes( mLayer ) );
  exprContext.setFields( mLayer->fields() );
  Qgs3DRenderContext renderCtx = mRenderContext; // Copy since we mutate it
  renderCtx.setExpressionContext( exprContext );

  // loader is shared among multiple threads which may be run at the same time
  // so we need a local copy of our rule tree that does not intefere with others
  // (e.g. it happened that filter expressions with invalid syntax would cause
  // nasty crashes when trying to simultaneously record evaluation error)
  std::unique_ptr<QgsRuleBased3DRenderer::Rule> rootRule( mRootRule->clone() );

  QgsRuleBased3DRenderer::RuleToHandlerMap handlers;
  rootRule->createHandlers( mLayer, handlers );

  QSet<QString> attributeNames;
  rootRule->prepare( renderCtx, attributeNames, node->box3D(), handlers );

  // build the feature request
  // only a subset of data to be queried
  const QgsRectangle rect = node->box3D().toRectangle();
  QgsFeatureRequest req;
  req.setDestinationCrs( renderCtx.crs(), renderCtx.transformContext() );
  req.setSubsetOfAttributes( attributeNames, mLayer->fields() );
  req.setFilterRect( rect );

  auto source = std::make_unique<QgsVectorLayerFeatureSource>( mLayer );

  QPointer<QgsRuleBasedChunkLoader> weakThis = this;
  return QtConcurrent::run( [req = std::move( req ), node, handlers, source = std::move( source ), rootRule = std::move( rootRule ), renderCtx, maxFeatures = mMaxFeatures, weakThis](
                              QPromise<QgsChunkLoaderResult> &promise
                            ) mutable {
    const QgsScopedEvent e( u"3D"_s, u"RB chunk load"_s );

    QgsFeature f;
    QgsFeatureIterator fi = source->getFeatures( req );
    int featureCount = 0;
    bool featureLimitReached = false;
    while ( fi.nextFeature( f ) )
    {
      if ( promise.isCanceled() )
        break;

      if ( ++featureCount > maxFeatures )
      {
        featureLimitReached = true;
        break;
      }

      renderCtx.expressionContext().setFeature( f );
      rootRule->registerFeature( f, renderCtx, handlers );
    }

    bool nodeIsLeaf = false;
    if ( !featureLimitReached )
    {
      QgsDebugMsgLevel( u"All features fetched for node: %1"_s.arg( node->tileId().text() ), 3 );

      if ( featureCount == 0 || std::max<double>( node->box3D().width(), node->box3D().height() ) < QgsVectorLayer3DTilingSettings::maximumLeafExtent() )
        nodeIsLeaf = true;
    }

    QgsThreadingUtils::runOnMainThread( [weakThis, nodeIsLeaf, key = node->tileId().text()]() {
      if ( weakThis )
      {
        QMutexLocker<QMutex> locker( &weakThis->mNodesAreLeafsMutex );
        weakThis->mNodesAreLeafs[key] = nodeIsLeaf;
      }
    } );

    promise.addResult( QgsChunkLoaderResult { [handlers, node, renderCtx]( Qt3DCore::QEntity *parent ) -> Qt3DCore::QEntity * {
      QGIS_CHECK_MAIN_THREAD_ACCESS
      long long featureCount = 0;
      for ( auto it = handlers.constBegin(); it != handlers.constEnd(); ++it )
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
      for ( auto it = handlers.constBegin(); it != handlers.constEnd(); ++it )
      {
        QgsFeature3DHandler *handler = it.value();
        handler->finalize( entity, renderCtx );
        if ( handler->zMinimum() < zMin )
          zMin = handler->zMinimum();
        if ( handler->zMaximum() > zMax )
          zMax = handler->zMaximum();
      }
      // fix the vertical range of the node from the estimated vertical range to the true range
      if ( zMin != std::numeric_limits<float>::max() && zMax != std::numeric_limits<float>::lowest() )
      {
        QgsBox3D box = node->box3D();
        box.setZMinimum( zMin );
        box.setZMaximum( zMax );
        node->setExactBox3D( box );
        node->updateParentBoundingBoxesRecursively();
      }

      // TODO: Use smart pointers not to leak handlers if createEntity is never called.
      qDeleteAll( handlers );

      return entity;
    } } );
  } );
}

QFuture<QVector<QgsChunkNode *>> QgsRuleBasedChunkLoader::createChildren( QgsChunkNode *node )
{
  {
    QMutexLocker<QMutex> locker( &mNodesAreLeafsMutex );
    if ( mNodesAreLeafs.value( node->tileId().text(), false ) )
      return QtFuture::makeReadyValueFuture( QVector<QgsChunkNode *> {} );
  }

  return QgsQuadtreeChunkLoader::createChildren( node );
}

///////////////

QgsRuleBasedChunkedEntity::QgsRuleBasedChunkedEntity(
  Qgs3DMapSettings *map, QgsVectorLayer *vl, double zMin, double zMax, const QgsVectorLayer3DTilingSettings &tilingSettings, QgsRuleBased3DRenderer::Rule *rootRule
)
  : QgsAbstractFeatureBasedChunkedEntity( map, 3, new QgsRuleBasedChunkLoader( Qgs3DRenderContext::fromMapSettings( map ), vl, rootRule, zMin, zMax, tilingSettings.maximumChunkFeatures() ), true )
{
  onTerrainElevationOffsetChanged();
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
  QgsRuleBasedChunkLoader *loader = static_cast<QgsRuleBasedChunkLoader *>( mChunkLoader );
  if ( loader )
  {
    QgsRuleBased3DRenderer::Rule *rootRule = loader->mRootRule.get();
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
          QgsDebugMsgLevel( u"QgsRuleBasedChunkedEntityChunkedEntity::applyTerrainOffset, unhandled symbol type %1"_s.arg( symbolType ), 2 );
        }
      }
    }
  }

  return true;
}

/// @endcond
