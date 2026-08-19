/***************************************************************************
  qgscategorizedchunkloader_p.cpp
  --------------------------------------
  Date                 : November 2025
  Copyright            : (C) 2025 by Jean Felder
  Email                : jean dot felder at oslandia dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgscategorizedchunkloader_p.h"

#include "qgs3dsymbolregistry.h"
#include "qgs3dutils.h"
#include "qgsabstractterrainsettings.h"
#include "qgsapplication.h"
#include "qgscategorized3drenderer.h"
#include "qgscategorizedsymbolutils.h"
#include "qgschunkloader.h"
#include "qgschunknode.h"
#include "qgseventtracing.h"
#include "qgsexpressioncontextutils.h"
#include "qgsfeature3dhandler_p.h"
#include "qgsfields.h"
#include "qgsline3dsymbol.h"
#include "qgspoint3dsymbol.h"
#include "qgspolygon3dsymbol.h"
#include "qgsvectorlayer.h"
#include "qgsvectorlayerfeatureiterator.h"

#include <QMutex>
#include <QString>
#include <Qt3DCore/QTransform>
#include <QtConcurrentRun>

#include "moc_qgscategorizedchunkloader_p.cpp"

using namespace Qt::StringLiterals;

///@cond PRIVATE

QgsCategorizedChunkLoader::QgsCategorizedChunkLoader( const Qgs3DRenderContext &context, QgsVectorLayer *vectorLayer, const QgsCategorized3DRenderer *renderer, double zMin, double zMax, int maxFeatures )
  : mRenderContext( context )
  , mLayer( vectorLayer )
  , mCategories( &renderer->categories() )
  , mAttributeName( renderer->classAttribute() )
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
  if ( context.extent().contains( mLayer->extent() ) )
  {
    extent = mLayer->extent();
  }
  QgsBox3D rootBox3D( extent, zMin, zMax );

  // add small padding to avoid clipping of point features located at the edge of the bounding box
  rootBox3D.grow( 1.0 );

  const float rootError = static_cast<float>( std::max<double>( rootBox3D.width(), rootBox3D.height() ) * QgsVectorLayer3DTilingSettings::tileGeometryErrorRatio() );
  setupQuadtree( rootBox3D, rootError );
}

QgsCategorizedChunkLoader::~QgsCategorizedChunkLoader() = default;

struct ChunkLoadingContext
{
    // TODO(dvdkon): we need shared_ptr, because we capture this in
    // std::function. We could get around this with std::move_only_function
    // (C++23 feature) or by returning a custom allocated object with virtual
    // methods.
    QgsChunkNode *node;
    std::shared_ptr<QgsVectorLayerFeatureSource> source;
    Qgs3DRenderContext renderCtx;
    std::vector<std::shared_ptr<QgsFeature3DHandler>> handlers;
    //! hashtable for faster access to symbols
    QHash<QString, QgsFeature3DHandler *> featuresHandlerHash;
    std::shared_ptr<QgsExpression> expression;
    int attributeIdx = -1;
};

static void processFeature( ChunkLoadingContext &ctx, const QgsFeature &feature )
{
  ctx.renderCtx.expressionContext().setFeature( feature );

  // Get Value for feature
  QgsAttributes attributes = feature.attributes();
  QVariant value;
  if ( ctx.attributeIdx == -1 )
  {
    value = ctx.expression->evaluate( &ctx.renderCtx.expressionContext() );
  }
  else
  {
    value = attributes.value( ctx.attributeIdx );
  }

  auto handlerIt = ctx.featuresHandlerHash.constFind( QgsVariantUtils::isNull( value ) ? QString() : value.toString() );
  if ( handlerIt == ctx.featuresHandlerHash.constEnd() )
  {
    if ( ctx.featuresHandlerHash.isEmpty() )
    {
      QgsDebugError( u"There are no hashed symbols!"_s );
    }
    else
    {
      QgsDebugMsgLevel( u"Attribute value not found: %1"_s.arg( value.toString() ), 3 );
    }
    return;
  }

  QgsFeature3DHandler *handler = *handlerIt;
  handler->processFeature( feature, ctx.renderCtx );
}


QFuture<QgsChunkLoaderResult> QgsCategorizedChunkLoader::loadChunk( QgsChunkNode *node )
{
  ChunkLoadingContext ctx;
  ctx.renderCtx = mRenderContext; // Copy for constness
  ctx.source = std::make_unique<QgsVectorLayerFeatureSource>( mLayer );

  QgsExpressionContext exprContext;
  exprContext.appendScopes( QgsExpressionContextUtils::globalProjectLayerScopes( mLayer ) );
  exprContext.setFields( mLayer->fields() );
  ctx.renderCtx.setExpressionContext( exprContext );

  // build the feature handlers
  QSet<QString> attributesNames;

  // prepare the expression
  ctx.attributeIdx = mLayer->fields().lookupField( mAttributeName );
  if ( ctx.attributeIdx == -1 )
  {
    ctx.expression.reset( new QgsExpression( mAttributeName ) );
    ctx.expression->prepare( &ctx.renderCtx.expressionContext() );
  }

  // build features hash
  for ( const Qgs3DRendererCategory &category : *mCategories )
  {
    if ( !category.renderState() )
    {
      continue;
    }

    const QVariant value = category.value();
    std::unique_ptr<QgsFeature3DHandler> handler( QgsApplication::symbol3DRegistry()->createHandlerForSymbol( mLayer, category.symbol() ) );
    ctx.handlers.push_back( std::move( handler ) );
    QgsFeature3DHandler *handlerPtr = ctx.handlers.back().get();
    if ( value.userType() == QMetaType::Type::QVariantList )
    {
      const QVariantList variantList = value.toList();
      for ( const QVariant &listElt : variantList )
      {
        ctx.featuresHandlerHash.insert( listElt.toString(), handlerPtr );
      }
    }
    else
    {
      ctx.featuresHandlerHash.insert( value.toString(), handlerPtr );
    }

    handlerPtr->prepare( ctx.renderCtx, attributesNames, node->box3D() );
  }
  attributesNames.insert( mAttributeName );

  // build the feature request
  // only a subset of data to be queried
  const QgsRectangle rect = node->box3D().toRectangle();
  QgsFeatureRequest request;
  request.setDestinationCrs( ctx.renderCtx.crs(), ctx.renderCtx.transformContext() );
  request.setSubsetOfAttributes( attributesNames, mLayer->fields() );
  request.setFilterRect( rect );

  const QString rendererFilter = QgsCategorizedSymbolUtils<QgsCategorized3DRenderer>::buildCategorizedFilter( mAttributeName, mLayer->fields(), *mCategories );
  if ( !rendererFilter.isEmpty() )
  {
    request.setFilterExpression( rendererFilter );
  }

  return QtConcurrent::run( [request, this, ctx = std::move( ctx )]( QPromise<QgsChunkLoaderResult> &promise ) mutable {
    const QgsScopedEvent event( u"3D"_s, u"Categorized chunk load"_s );
    QgsFeature feature;
    QgsFeatureIterator featureIt = ctx.source->getFeatures( request );
    int featureCount = 0;
    bool featureLimitReached = false;
    while ( featureIt.nextFeature( feature ) )
    {
      if ( promise.isCanceled() )
      {
        break;
      }

      if ( ++featureCount > mMaxFeatures )
      {
        featureLimitReached = true;
        break;
      }

      processFeature( ctx, feature );
    }
    bool mNodeIsLeaf = false;
    if ( !featureLimitReached )
    {
      QgsDebugMsgLevel( u"All features fetched for node: %1"_s.arg( ctx.node->tileId().text() ), 3 );

      if ( featureCount == 0 || std::max<double>( ctx.node->box3D().width(), ctx.node->box3D().height() ) < QgsVectorLayer3DTilingSettings::maximumLeafExtent() )
        mNodeIsLeaf = true;
    }
    {
      QMutexLocker<QMutex> locker( &this->mNodesAreLeafsMutex );
      mNodesAreLeafs[ctx.node->tileId().text()] = mNodeIsLeaf;
    }

    promise.addResult( QgsChunkLoaderResult( [ctx = std::move( ctx )]( Qt3DCore::QEntity *parent ) -> Qt3DCore::QEntity * {
      long long featureCount = 0;
      for ( const auto &featureHandler : ctx.handlers )
      {
        featureCount += featureHandler->featureCount();
      }
      if ( featureCount == 0 )
      {
        // an empty node, so we return no entity. This tags the node as having no data and effectively removes it.
        return nullptr;
      }

      Qt3DCore::QEntity *entity = new Qt3DCore::QEntity( parent );
      float zMin = std::numeric_limits<float>::max();
      float zMax = std::numeric_limits<float>::lowest();
      for ( const auto &featureHandler : ctx.handlers )
      {
        featureHandler->finalize( entity, ctx.renderCtx );
        if ( featureHandler->zMinimum() < zMin )
        {
          zMin = featureHandler->zMinimum();
        }
        if ( featureHandler->zMaximum() > zMax )
        {
          zMax = featureHandler->zMaximum();
        }
      }

      // fix the vertical range of the node from the estimated vertical range to the true range
      if ( zMin != std::numeric_limits<float>::max() && zMax != std::numeric_limits<float>::lowest() )
      {
        QgsBox3D box = ctx.node->box3D();
        box.setZMinimum( zMin );
        box.setZMaximum( zMax );
        ctx.node->setExactBox3D( box );
        ctx.node->updateParentBoundingBoxesRecursively();
      }

      return entity;
    } ) );
  } );
}

bool QgsCategorizedChunkLoader::canCreateChildren( QgsChunkNode *node )
{
  return mNodesAreLeafs.contains( node->tileId().text() );
}

QVector<QgsChunkNode *> QgsCategorizedChunkLoader::createChildren( QgsChunkNode *node ) const
{
  if ( mNodesAreLeafs.value( node->tileId().text(), false ) )
    return {};

  return QgsQuadtreeChunkLoader::createChildren( node );
}


///////////////

QgsCategorizedChunkedEntity::QgsCategorizedChunkedEntity(
  Qgs3DMapSettings *mapSettings, QgsVectorLayer *vectorLayer, double zMin, double zMax, const QgsVectorLayer3DTilingSettings &tilingSettings, const QgsCategorized3DRenderer *renderer
)
  : QgsAbstractFeatureBasedChunkedEntity(
      mapSettings,
      -1, // max. allowed screen error (negative tau means that we need to go until leaves are reached)
      new QgsCategorizedChunkLoader( Qgs3DRenderContext::fromMapSettings( mapSettings ), vectorLayer, renderer, zMin, zMax, tilingSettings.maximumChunkFeatures() ),
      true
    )
{
  onTerrainElevationOffsetChanged();
  setShowBoundingBoxes( tilingSettings.showBoundingBoxes() );
}

QgsCategorizedChunkedEntity::~QgsCategorizedChunkedEntity()
{
  // cancel / wait for jobs
  cancelActiveJobs();
}

// if the AltitudeClamping is `Absolute`, do not apply the offset
bool QgsCategorizedChunkedEntity::applyTerrainOffset() const
{
  QgsCategorizedChunkLoader *loader = static_cast<QgsCategorizedChunkLoader *>( mChunkLoader );
  if ( loader )
  {
    for ( const auto &category : *loader->mCategories )
    {
      const QgsAbstract3DSymbol *symbol = category.symbol();
      if ( category.symbol() )
      {
        QString symbolType = symbol->type();
        if ( symbolType == "line" )
        {
          const QgsLine3DSymbol *lineSymbol = static_cast<const QgsLine3DSymbol *>( symbol );
          if ( lineSymbol && lineSymbol->altitudeClamping() == Qgis::AltitudeClamping::Absolute )
          {
            return false;
          }
        }
        else if ( symbolType == "point" )
        {
          const QgsPoint3DSymbol *pointSymbol = static_cast<const QgsPoint3DSymbol *>( symbol );
          if ( pointSymbol && pointSymbol->altitudeClamping() == Qgis::AltitudeClamping::Absolute )
          {
            return false;
          }
        }
        else if ( symbolType == "polygon" )
        {
          const QgsPolygon3DSymbol *polygonSymbol = static_cast<const QgsPolygon3DSymbol *>( symbol );
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
