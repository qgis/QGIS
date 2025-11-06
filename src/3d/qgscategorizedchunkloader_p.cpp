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
#include "qgschunknode.h"
#include "qgseventtracing.h"
#include "qgsexpressioncontextutils.h"
#include "qgsfeature3dhandler_p.h"
#include "qgsline3dsymbol.h"
#include "qgspoint3dsymbol.h"
#include "qgspolygon3dsymbol.h"
#include "qgsvectorlayer.h"
#include "qgsvectorlayerchunkloader_p.h"
#include "qgsvectorlayerfeatureiterator.h"

#include <QString>
#include <Qt3DCore/QTransform>
#include <QtConcurrentRun>

#include "moc_qgscategorizedchunkloader_p.cpp"

using namespace Qt::StringLiterals;

///@cond PRIVATE


QgsCategorizedChunkLoader::QgsCategorizedChunkLoader( const QgsCategorizedChunkLoaderFactory *factory, QgsChunkNode *node )
  : QgsChunkLoader( node )
  , mFactory( factory )
  , mContext( factory->mRenderContext )
  , mSource( new QgsVectorLayerFeatureSource( factory->mLayer ) )
{
}

const QSet<QString> QgsCategorizedChunkLoader::prepareHandlers( const QgsBox3D &chunkExtent )
{
  QgsVectorLayer *layer = mFactory->mLayer;
  const QString attributeName = mFactory->mAttributeName;

  QSet<QString> attributesNames;

  // prepare the expression
  mAttributeIdx = layer->fields().lookupField( attributeName );
  if ( mAttributeIdx == -1 )
  {
    mExpression.reset( new QgsExpression( attributeName ) );
    mExpression->prepare( &mContext.expressionContext() );
  }

  // build features hash
  mFeaturesHandlerHash.clear();

  for ( const Qgs3DRendererCategory &category : std::as_const( mFactory->mCategories ) )
  {
    if ( !category.renderState() )
    {
      continue;
    }

    const QVariant value = category.value();
    QgsFeature3DHandler *handler = QgsApplication::symbol3DRegistry()->createHandlerForSymbol( layer, category.symbol() );
    if ( value.userType() == QMetaType::Type::QVariantList )
    {
      const QVariantList variantList = value.toList();
      for ( const QVariant &listElt : variantList )
      {
        mFeaturesHandlerHash.insert( listElt.toString(), handler );
      }
    }
    else
    {
      mFeaturesHandlerHash.insert( value.toString(), handler );
    }

    handler->prepare( mContext, attributesNames, chunkExtent );
  }

  attributesNames.insert( attributeName );
  return attributesNames;
}

void QgsCategorizedChunkLoader::processFeature( const QgsFeature &feature ) const
{
  // Get Value for feature
  QgsAttributes attributes = feature.attributes();
  QVariant value;
  if ( mAttributeIdx == -1 )
  {
    Q_ASSERT( mExpression );
    value = mExpression->evaluate( &mContext.expressionContext() );
  }
  else
  {
    value = attributes.value( mAttributeIdx );
  }

  auto handlerIt = mFeaturesHandlerHash.constFind( QgsVariantUtils::isNull( value ) ? QString() : value.toString() );
  if ( handlerIt == mFeaturesHandlerHash.constEnd() )
  {
    if ( mFeaturesHandlerHash.isEmpty() )
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
  handler->processFeature( feature, mContext );
}

void QgsCategorizedChunkLoader::start()
{
  QgsChunkNode *node = chunk();

  QgsVectorLayer *layer = mFactory->mLayer;

  QgsExpressionContext exprContext;
  exprContext.appendScopes( QgsExpressionContextUtils::globalProjectLayerScopes( layer ) );
  exprContext.setFields( layer->fields() );
  mContext.setExpressionContext( exprContext );

  // build the feature handlers
  const QSet<QString> attributesNames = prepareHandlers( node->box3D() );

  // build the feature request
  // only a subset of data to be queried
  const QgsRectangle rect = node->box3D().toRectangle();
  QgsFeatureRequest request;
  request.setDestinationCrs( mContext.crs(), mContext.transformContext() );
  request.setSubsetOfAttributes( attributesNames, layer->fields() );
  request.setFilterRect( rect );

  //
  // this will be run in a background thread
  //
  mFutureWatcher = new QFutureWatcher<void>( this );

  connect( mFutureWatcher, &QFutureWatcher<void>::finished, this, [this] {
    if ( !mCanceled )
      mFactory->mNodesAreLeafs[mNode->tileId().text()] = mNodeIsLeaf;
  } );

  connect( mFutureWatcher, &QFutureWatcher<void>::finished, this, &QgsChunkQueueJob::finished );

  const QFuture<void> future = QtConcurrent::run( [request, this] {
    const QgsEventTracing::ScopedEvent event( u"3D"_s, u"Categorized chunk load"_s );
    QgsFeature feature;
    QgsFeatureIterator featureIt = mSource->getFeatures( request );
    int featureCount = 0;
    bool featureLimitReached = false;
    while ( featureIt.nextFeature( feature ) )
    {
      if ( mCanceled )
      {
        break;
      }

      if ( ++featureCount > mFactory->mMaxFeatures )
      {
        featureLimitReached = true;
        break;
      }

      mContext.expressionContext().setFeature( feature );
      processFeature( feature );
    }
    if ( !featureLimitReached )
    {
      QgsDebugMsgLevel( u"All features fetched for node: %1"_s.arg( mNode->tileId().text() ), 3 );

      if ( featureCount == 0 || std::max<double>( mNode->box3D().width(), mNode->box3D().height() ) < QgsVectorLayer3DTilingSettings::maximumLeafExtent() )
        mNodeIsLeaf = true;
    }
  } );

  // emit finished() as soon as the handler is populated with features
  mFutureWatcher->setFuture( future );
}

QgsCategorizedChunkLoader::~QgsCategorizedChunkLoader()
{
  if ( mFutureWatcher && !mFutureWatcher->isFinished() )
  {
    disconnect( mFutureWatcher, &QFutureWatcher<void>::finished, this, &QgsChunkQueueJob::finished );
    mFutureWatcher->waitForFinished();
  }
}

void QgsCategorizedChunkLoader::cancel()
{
  mCanceled = true;
}

Qt3DCore::QEntity *QgsCategorizedChunkLoader::createEntity( Qt3DCore::QEntity *parent )
{
  long long featureCount = 0;
  for ( const auto featureHandler : mFeaturesHandlerHash )
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
  for ( const auto featureHandler : mFeaturesHandlerHash )
  {
    featureHandler->finalize( entity, mContext );
    if ( featureHandler->zMinimum() < zMin )
      zMin = featureHandler->zMinimum();
    if ( featureHandler->zMaximum() > zMax )
      zMax = featureHandler->zMaximum();
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


QgsCategorizedChunkLoaderFactory::QgsCategorizedChunkLoaderFactory( const Qgs3DRenderContext &context, QgsVectorLayer *vectorLayer, const QgsCategorized3DRenderer *renderer, double zMin, double zMax, int maxFeatures )
  : mRenderContext( context )
  , mLayer( vectorLayer )
  , mCategories( renderer->categories() )
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

  QgsBox3D rootBox3D( context.extent(), zMin, zMax );
  // add small padding to avoid clipping of point features located at the edge of the bounding box
  rootBox3D.grow( 1.0 );

  const float rootError = static_cast<float>( std::max<double>( rootBox3D.width(), rootBox3D.height() ) * QgsVectorLayer3DTilingSettings::tileGeometryErrorRatio() );
  setupQuadtree( rootBox3D, rootError );
}

QgsCategorizedChunkLoaderFactory::~QgsCategorizedChunkLoaderFactory() = default;

QgsChunkLoader *QgsCategorizedChunkLoaderFactory::createChunkLoader( QgsChunkNode *node ) const
{
  return new QgsCategorizedChunkLoader( this, node );
}

bool QgsCategorizedChunkLoaderFactory::canCreateChildren( QgsChunkNode *node )
{
  return mNodesAreLeafs.contains( node->tileId().text() );
}

QVector<QgsChunkNode *> QgsCategorizedChunkLoaderFactory::createChildren( QgsChunkNode *node ) const
{
  if ( mNodesAreLeafs.value( node->tileId().text(), false ) )
    return {};

  return QgsQuadtreeChunkLoaderFactory::createChildren( node );
}


///////////////

QgsCategorizedChunkedEntity::QgsCategorizedChunkedEntity( Qgs3DMapSettings *mapSettings, QgsVectorLayer *vectorLayer, double zMin, double zMax, const QgsVectorLayer3DTilingSettings &tilingSettings, const QgsCategorized3DRenderer *renderer )
  : QgsChunkedEntity( mapSettings,
                      -1, // max. allowed screen error (negative tau means that we need to go until leaves are reached)
                      new QgsCategorizedChunkLoaderFactory( Qgs3DRenderContext::fromMapSettings( mapSettings ), vectorLayer, renderer, zMin, zMax, tilingSettings.maximumChunkFeatures() ), true )
{
  mTransform = new Qt3DCore::QTransform;
  if ( applyTerrainOffset() )
  {
    mTransform->setTranslation( QVector3D( 0.0f, 0.0f, static_cast<float>( mapSettings->terrainSettings()->elevationOffset() ) ) );
  }
  this->addComponent( mTransform );
  connect( mapSettings, &Qgs3DMapSettings::terrainSettingsChanged, this, &QgsCategorizedChunkedEntity::onTerrainElevationOffsetChanged );

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
  QgsCategorizedChunkLoaderFactory *loaderFactory = static_cast<QgsCategorizedChunkLoaderFactory *>( mChunkLoaderFactory );
  if ( loaderFactory )
  {
    for ( const auto &category : loaderFactory->mCategories )
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

void QgsCategorizedChunkedEntity::onTerrainElevationOffsetChanged()
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

QList<QgsRayCastHit> QgsCategorizedChunkedEntity::rayIntersection( const QgsRay3D &ray, const QgsRayCastContext &context ) const
{
  return QgsVectorLayerChunkedEntity::rayIntersection( activeNodes(), mTransform->matrix(), ray, context, mMapSettings->origin() );
}
/// @endcond
