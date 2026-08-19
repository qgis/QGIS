/***************************************************************************
    qgs3dhighlightfeaturehandler.cpp
    ---------------------
    begin                : December 2025
    copyright            : (C) 2025 by Stefanos Natsis
    email                : uclaros at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgs3dhighlightfeaturehandler.h"

#include "qgs3dmapscene.h"
#include "qgs3dmapsceneentity.h"
#include "qgs3dsymbolregistry.h"
#include "qgsabstract3dengine.h"
#include "qgsapplication.h"
#include "qgscategorized3drenderer.h"
#include "qgsexpressioncontextutils.h"
#include "qgsfeature3dhandler_p.h"
#include "qgsframegraph.h"
#include "qgshighlightsrenderview.h"
#include "qgspointcloudlayer3drenderer.h"
#include "qgsrubberband3d.h"
#include "qgsvariantutils.h"
#include "qgsvectorlayer.h"
#include "qgsvectorlayer3drenderer.h"

#include <QSet>
#include <QString>
#include <QTimer>

#include "moc_qgs3dhighlightfeaturehandler.cpp"

using namespace Qt::StringLiterals;

Qgs3DHighlightFeatureHandler::Qgs3DHighlightFeatureHandler( Qgs3DMapScene *scene )
  : mScene( scene )
{}

Qgs3DHighlightFeatureHandler::~Qgs3DHighlightFeatureHandler()
{
  qDeleteAll( mHighlightHandlers );
  for ( auto it = mHighlightRuleBasedHandlers.constBegin(); it != mHighlightRuleBasedHandlers.constEnd(); ++it )
  {
    qDeleteAll( it.value() );
  }
  for ( auto it = mHighlightCategorizedHandlers.constBegin(); it != mHighlightCategorizedHandlers.constEnd(); ++it )
  {
    QSet<QgsFeature3DHandler *> uniqueHandlers;
    for ( QgsFeature3DHandler *handler : it.value() )
      uniqueHandlers.insert( handler );
    qDeleteAll( uniqueHandlers );
  }
}

void Qgs3DHighlightFeatureHandler::highlightFeature( QgsFeature feature, QgsMapLayer *layer )
{
  switch ( layer->type() )
  {
    // we only support point clouds and vector for now
    case Qgis::LayerType::Raster:
    case Qgis::LayerType::Plugin:
    case Qgis::LayerType::Mesh:
    case Qgis::LayerType::VectorTile:
    case Qgis::LayerType::Annotation:
    case Qgis::LayerType::Group:
    case Qgis::LayerType::TiledScene:
      return;

    case Qgis::LayerType::Vector:
    {
      QgsAbstract3DRenderer *renderer = layer->renderer3D();
      if ( !renderer )
        return;

      QgsVectorLayer *vLayer = static_cast<QgsVectorLayer *>( layer );
      QgsExpressionContext exprContext;
      exprContext.appendScopes( QgsExpressionContextUtils::globalProjectLayerScopes( layer ) );
      exprContext.setFields( vLayer->fields() );
      exprContext.setFeature( feature );
      Qgs3DRenderContext renderContext = Qgs3DRenderContext::fromMapSettings( mScene->mapSettings() );
      renderContext.setExpressionContext( exprContext );

      if ( renderContext.crs() != layer->crs() )
      {
        QgsCoordinateTransform ct( layer->crs(), renderContext.crs(), renderContext.transformContext() );
        QgsGeometry geom = feature.geometry();
        try
        {
          geom.transform( ct );
        }
        catch ( QgsCsException & )
        {
          QgsDebugError( u"Could not reproject identified feature to 3d view crs"_s );
          return;
        }
        feature.setGeometry( geom );
        exprContext.setFeature( feature );
      }

      if ( renderer->type() == "vector"_L1 )
      {
        if ( !mHighlightHandlers.contains( layer ) )
        {
          QgsVectorLayer3DRenderer *renderer3d = static_cast<QgsVectorLayer3DRenderer *>( renderer );

          if ( !renderer3d || !renderer3d->symbol() )
          {
            return;
          }

          std::unique_ptr<QgsFeature3DHandler> handler( QgsApplication::symbol3DRegistry()->createHandlerForSymbol( vLayer, renderer3d->symbol() ) );
          if ( !handler )
          {
            QgsDebugError( u"Unknown 3D symbol type for vector layer: "_s + renderer3d->symbol()->type() );
            return;
          }

          QSet<QString> attributeNames;
          const QgsGeometry geom = feature.geometry();
          // We want to ignore the geometry's elevation and use a box with symmetric +Z/-Z extents so the chunk origin has Z == 0.
          QgsBox3D box = geom.boundingBox().toBox3d( 0, 0 );
          if ( !handler->prepare( renderContext, attributeNames, box ) )
          {
            QgsDebugError( u"Failed to prepare 3D feature handler!"_s );
            return;
          }

          mHighlightHandlers[layer] = handler.release();
        }

        mHighlightHandlers[layer]->processFeature( feature, renderContext );
      }
      else if ( renderer->type() == "rulebased"_L1 )
      {
        QgsRuleBased3DRenderer *ruleBasedRenderer = static_cast<QgsRuleBased3DRenderer *>( renderer );
        QgsRuleBased3DRenderer::Rule *rootRule = ruleBasedRenderer->rootRule();
        if ( !mHighlightRuleBasedHandlers.contains( layer ) )
        {
          QgsRuleBased3DRenderer::RuleToHandlerMap handlers;
          rootRule->createHandlers( vLayer, handlers );
          QSet<QString> attributeNames;
          const QgsGeometry geom = feature.geometry();
          // We want to ignore the geometry's elevation and use a box with symmetric +Z/-Z extents so the chunk origin has Z == 0.
          QgsBox3D box = geom.boundingBox().toBox3d( 0, 0 );
          rootRule->prepare( renderContext, attributeNames, box, handlers );
          mHighlightRuleBasedHandlers[layer] = handlers;
        }

        rootRule->registerFeature( feature, renderContext, mHighlightRuleBasedHandlers[layer] );
      }
      else if ( renderer->type() == "categorized"_L1 )
      {
        QgsCategorized3DRenderer *categorizedRenderer = static_cast<QgsCategorized3DRenderer *>( renderer );
        if ( !mHighlightCategorizedHandlers.contains( layer ) )
        {
          QHash<QString, QgsFeature3DHandler *> handlersHash;
          const QgsGeometry geom = feature.geometry();
          // We want to ignore the geometry's elevation and use a box with symmetric +Z/-Z extents so the chunk origin has Z == 0.
          QgsBox3D box = geom.boundingBox().toBox3d( 0, 0 );
          QSet<QString> attributeNames;

          for ( const Qgs3DRendererCategory &category : categorizedRenderer->categories() )
          {
            if ( !category.renderState() || !category.symbol() )
              continue;

            std::unique_ptr<QgsFeature3DHandler> handler( QgsApplication::symbol3DRegistry()->createHandlerForSymbol( vLayer, category.symbol() ) );
            if ( !handler )
              continue;

            handler->prepare( renderContext, attributeNames, box );

            const QVariant value = category.value();
            if ( value.userType() == QMetaType::Type::QVariantList )
            {
              const QVariantList variantList = value.toList();
              if ( variantList.isEmpty() )
                continue;

              QgsFeature3DHandler *handlerPtr = handler.release();
              for ( const QVariant &listElt : variantList )
              {
                handlersHash.insert( listElt.toString(), handlerPtr );
              }
            }
            else
            {
              QgsFeature3DHandler *handlerPtr = handler.release();
              handlersHash.insert( QgsVariantUtils::isNull( value ) ? QString() : value.toString(), handlerPtr );
            }
          }
          mHighlightCategorizedHandlers[layer] = handlersHash;
        }

        QVariant value;
        int attrIdx = vLayer->fields().lookupField( categorizedRenderer->classAttribute() );
        if ( attrIdx == -1 )
        {
          QgsExpression expr( categorizedRenderer->classAttribute() );
          expr.prepare( &exprContext );
          value = expr.evaluate( &exprContext );
        }
        else
        {
          value = feature.attributes().value( attrIdx );
        }

        QString valueStr = QgsVariantUtils::isNull( value ) ? QString() : value.toString();
        auto handlerIt = mHighlightCategorizedHandlers[layer].constFind( valueStr );
        if ( handlerIt != mHighlightCategorizedHandlers[layer].constEnd() )
        {
          ( *handlerIt )->processFeature( feature, renderContext );
        }
      }

      // We'll use a singleshot timer to handle the entity creation, it will be served once we stop receiving highlighted features
      if ( !mHighlightHandlerTimer && ( !mHighlightHandlers.isEmpty() || !mHighlightRuleBasedHandlers.isEmpty() || !mHighlightCategorizedHandlers.isEmpty() ) )
      {
        mHighlightHandlerTimer = std::make_unique<QTimer>();
        mHighlightHandlerTimer->setSingleShot( true );
        mHighlightHandlerTimer->setInterval( 0 );
        connect( mHighlightHandlerTimer.get(), &QTimer::timeout, this, [this, renderContext] {
          for ( auto it = mHighlightHandlers.constBegin(); it != mHighlightHandlers.constEnd(); ++it )
          {
            Qgs3DMapSceneEntity *entity = new Qgs3DMapSceneEntity( mScene->mapSettings(), nullptr );
            entity->setObjectName( u"%1_highlight"_s.arg( it.key()->name() ) );
            QgsFeature3DHandler *handler = it.value();
            handler->setHighlightingEnabled( true );
            handler->finalize( entity, renderContext );

            finalizeAndAddToScene( entity );
          }

          for ( auto rulesIt = mHighlightRuleBasedHandlers.constBegin(); rulesIt != mHighlightRuleBasedHandlers.constEnd(); ++rulesIt )
          {
            Qgs3DMapSceneEntity *entity = new Qgs3DMapSceneEntity( mScene->mapSettings(), nullptr );
            entity->setObjectName( u"%1_highlight"_s.arg( rulesIt.key()->name() ) );

            for ( auto handlersIt = rulesIt.value().constBegin(); handlersIt != rulesIt.value().constEnd(); ++handlersIt )
            {
              QgsFeature3DHandler *handler = handlersIt.value();
              handler->setHighlightingEnabled( true );
              handler->finalize( entity, renderContext );
            }

            finalizeAndAddToScene( entity );
          }

          for ( auto catIt = mHighlightCategorizedHandlers.constBegin(); catIt != mHighlightCategorizedHandlers.constEnd(); ++catIt )
          {
            Qgs3DMapSceneEntity *entity = new Qgs3DMapSceneEntity( mScene->mapSettings(), nullptr );
            entity->setObjectName( u"%1_highlight"_s.arg( catIt.key()->name() ) );

            QSet<QgsFeature3DHandler *> uniqueHandlers;
            for ( QgsFeature3DHandler *handler : catIt.value() )
            {
              uniqueHandlers.insert( handler );
            }

            for ( QgsFeature3DHandler *handler : std::as_const( uniqueHandlers ) )
            {
              handler->setHighlightingEnabled( true );
              handler->finalize( entity, renderContext );
            }

            finalizeAndAddToScene( entity );
          }
        } );
        mHighlightHandlerTimer->start();
      }

      return;
    }

    case Qgis::LayerType::PointCloud:
    {
      const QgsGeometry geom = feature.geometry();
      const QgsPoint pt( geom.vertexAt( 0 ) );

      if ( !mRubberBands.contains( layer ) )
      {
        QgsRubberBand3D *band = new QgsRubberBand3D( *mScene->mapSettings(), mScene->engine(), Qgis::GeometryType::Point );

        const QgsSettings settings;
        const QColor color = QColor( settings.value( u"Map/highlight/color"_s, Qgis::DEFAULT_HIGHLIGHT_COLOR.name() ).toString() );
        band->setColor( color );
        band->setMarkerType( QgsRubberBand3D::MarkerType::Square );
        if ( QgsPointCloudLayer3DRenderer *pcRenderer = dynamic_cast<QgsPointCloudLayer3DRenderer *>( layer->renderer3D() ) )
        {
          band->setWidth( pcRenderer->symbol()->pointSize() + 1 );
        }
        mRubberBands.insert( layer, band );

        connect( layer, &QgsMapLayer::renderer3DChanged, this, &Qgs3DHighlightFeatureHandler::onRenderer3DChanged );
      }
      mRubberBands[layer]->addPoint( pt );
      return;
    }
  }
}

void Qgs3DHighlightFeatureHandler::onRenderer3DChanged()
{
  if ( QgsMapLayer *layer = qobject_cast<QgsMapLayer *>( sender() ) )
  {
    if ( QgsPointCloudLayer3DRenderer *rnd = dynamic_cast<QgsPointCloudLayer3DRenderer *>( layer->renderer3D() ) )
    {
      if ( mRubberBands.contains( layer ) )
      {
        mRubberBands[layer]->setWidth( rnd->symbol()->pointSize() + 1 );
      }
    }
  }
}

void Qgs3DHighlightFeatureHandler::finalizeAndAddToScene( Qgs3DMapSceneEntity *entity )
{
  // We need the highlights layer to force rendering on highlights renderview only
  QgsFrameGraph *frameGraph = mScene->engine()->frameGraph();
  entity->addComponent( frameGraph->highlightsRenderView().highlightsLayer() );

  mScene->addSceneEntity( entity );
  mHighlightEntities.append( entity );
}

void Qgs3DHighlightFeatureHandler::clearHighlights()
{
  mHighlightHandlerTimer.reset();
  qDeleteAll( mHighlightHandlers );
  mHighlightHandlers.clear();
  for ( auto it = mHighlightRuleBasedHandlers.constBegin(); it != mHighlightRuleBasedHandlers.constEnd(); ++it )
  {
    qDeleteAll( it.value() );
  }
  mHighlightRuleBasedHandlers.clear();
  for ( auto it = mHighlightCategorizedHandlers.constBegin(); it != mHighlightCategorizedHandlers.constEnd(); ++it )
  {
    QSet<QgsFeature3DHandler *> uniqueHandlers;
    for ( QgsFeature3DHandler *handler : it.value() )
      uniqueHandlers.insert( handler );
    qDeleteAll( uniqueHandlers );
  }
  mHighlightCategorizedHandlers.clear();
  // vector layer highlights
  for ( Qt3DCore::QEntity *e : std::as_const( mHighlightEntities ) )
  {
    mScene->removeSceneEntity( static_cast<Qgs3DMapSceneEntity *>( e ) );
  }
  mHighlightEntities.clear();

  // point cloud layer highlights
  for ( auto it = mRubberBands.keyBegin(); it != mRubberBands.keyEnd(); it++ )
  {
    disconnect( it.base().key(), &QgsMapLayer::renderer3DChanged, this, &Qgs3DHighlightFeatureHandler::onRenderer3DChanged );
  }

  qDeleteAll( mRubberBands );
  mRubberBands.clear();
}
