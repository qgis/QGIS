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
#include "qgsexpressioncontextutils.h"
#include "qgsfeature3dhandler_p.h"
#include "qgsframegraph.h"
#include "qgshighlightmaterial.h"
#include "qgshighlightsrenderview.h"
#include "qgspointcloudlayer3drenderer.h"
#include "qgsrubberband3d.h"
#include "qgsvectorlayer.h"
#include "qgsvectorlayer3drenderer.h"

#include <QTimer>

#include "moc_qgs3dhighlightfeaturehandler.cpp"

Qgs3DHighlightFeatureHandler::Qgs3DHighlightFeatureHandler( Qgs3DMapScene *scene )
  : mScene( scene )
{
}

Qgs3DHighlightFeatureHandler::~Qgs3DHighlightFeatureHandler() = default;

void Qgs3DHighlightFeatureHandler::highlightFeature( const QgsFeature &feature, QgsMapLayer *layer )
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
      QgsVectorLayer *vLayer = static_cast<QgsVectorLayer *>( layer );
      QgsExpressionContext exprContext;
      exprContext.appendScopes( QgsExpressionContextUtils::globalProjectLayerScopes( layer ) );
      exprContext.setFields( vLayer->fields() );
      exprContext.setFeature( feature );
      Qgs3DRenderContext renderContext = Qgs3DRenderContext::fromMapSettings( mScene->mapSettings() );
      renderContext.setExpressionContext( exprContext );

      if ( mHighlightHandlers.contains( layer ) )
      {
        if ( !mHighlightHandlers[layer] )
          return;

        mHighlightHandlers[layer]->processFeature( feature, renderContext );
      }
      else
      {
        QgsVectorLayer3DRenderer *renderer3d = dynamic_cast<QgsVectorLayer3DRenderer *>( layer->renderer3D() );

        // We only support polygon 3d symbol for now
        if ( !renderer3d || !renderer3d->symbol() || renderer3d->symbol()->type() != QLatin1String( "polygon" ) )
        {
          mHighlightHandlers[layer] = nullptr;
          return;
        }

        std::unique_ptr<QgsFeature3DHandler> handler( QgsApplication::symbol3DRegistry()->createHandlerForSymbol( vLayer, renderer3d->symbol() ) );
        if ( !handler )
        {
          QgsDebugError( QStringLiteral( "Unknown 3D symbol type for vector layer: " ) + renderer3d->symbol()->type() );
          mHighlightHandlers[layer] = nullptr;
          return;
        }

        QSet<QString> attributeNames;
        const QgsGeometry geom = feature.geometry();
        const QgsVector3D origin( geom.boundingBox().center().x(), geom.boundingBox().center().y(), 0 );
        if ( !handler->prepare( renderContext, attributeNames, renderContext.origin() ) )
        {
          QgsDebugError( QStringLiteral( "Failed to prepare 3D feature handler!" ) );
          mHighlightHandlers[layer] = nullptr;
          return;
        }

        handler->processFeature( feature, renderContext );
        mHighlightHandlers[layer] = std::move( handler );

        // We'll use a singleshot timer to handle the entity creation once we stop receiving highlighted features
        if ( !mHighlightHandlerTimer )
        {
          mHighlightHandlerTimer = std::make_unique<QTimer>();
          mHighlightHandlerTimer->setSingleShot( true );
          mHighlightHandlerTimer->setInterval( 0 );
          connect( mHighlightHandlerTimer.get(), &QTimer::timeout, this, [this, renderContext] {
            for ( auto it = mHighlightHandlers.cbegin(); it != mHighlightHandlers.cend(); ++it )
            {
              if ( !it->second )
                continue;

              Qgs3DMapSceneEntity *entity = new Qgs3DMapSceneEntity( mScene->mapSettings(), nullptr );
              entity->setObjectName( QStringLiteral( "%1_highlight" ).arg( it->first->name() ) );
              it->second->finalize( entity, renderContext );

              // We need the highlights layer to force rendering on highlights renderview only
              QgsFrameGraph *frameGraph = mScene->engine()->frameGraph();
              entity->addComponent( frameGraph->highlightsRenderView().highlightsLayer() );

              // we remove existing material and replace them with the highlight material
              const QList<QgsMaterial *> materials = entity->findChildren<QgsMaterial *>();
              for ( QgsMaterial *mat : materials )
              {
                // material is not necessarily direct child of entity, so let's grab its parent
                Qt3DCore::QEntity *materialParent = static_cast<Qt3DCore::QEntity *>( mat->parentNode() );
                materialParent->removeComponent( mat );
                materialParent->addComponent( new QgsHighlightMaterial );
              }

              mScene->addSceneEntity( entity );
              mHighlightEntities.append( entity );
            }
          } );
          mHighlightHandlerTimer->start();
        }
      }
      return;
    }

    case Qgis::LayerType::PointCloud:
    {
      const QgsGeometry geom = feature.geometry();
      const QgsPoint pt( geom.vertexAt( 0 ) );

      if ( !mRubberBands.contains( layer ) )
      {
        QgsRubberBand3D *band = new QgsRubberBand3D( *mScene->mapSettings(), mScene->engine(), mScene->engine()->frameGraph()->rubberBandsRootEntity(), Qgis::GeometryType::Point );

        const QgsSettings settings;
        const QColor color = QColor( settings.value( QStringLiteral( "Map/highlight/color" ), Qgis::DEFAULT_HIGHLIGHT_COLOR.name() ).toString() );
        band->setColor( color );
        band->setMarkerType( QgsRubberBand3D::MarkerType::Square );
        if ( QgsPointCloudLayer3DRenderer *pcRenderer = dynamic_cast<QgsPointCloudLayer3DRenderer *>( layer->renderer3D() ) )
        {
          band->setWidth( pcRenderer->symbol()->pointSize() + 1 );
        }
        mRubberBands.insert( layer, band );

        connect( layer, &QgsMapLayer::renderer3DChanged, this, &Qgs3DHighlightFeatureHandler::updateHighlightSizes );
      }
      mRubberBands[layer]->addPoint( pt );
      return;
    }
  }
}

void Qgs3DHighlightFeatureHandler::updateHighlightSizes()
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

void Qgs3DHighlightFeatureHandler::clearHighlights()
{
  mHighlightHandlerTimer.reset();
  mHighlightHandlers.clear();
  // vector layer highlights
  for ( Qt3DCore::QEntity *e : std::as_const( mHighlightEntities ) )
  {
    mScene->removeSceneEntity( static_cast<Qgs3DMapSceneEntity *>( e ) );
  }
  mHighlightEntities.clear();

  // point cloud layer highlights
  for ( auto it = mRubberBands.keyBegin(); it != mRubberBands.keyEnd(); it++ )
  {
    disconnect( it.base().key(), &QgsMapLayer::renderer3DChanged, this, &Qgs3DHighlightFeatureHandler::updateHighlightSizes );
  }

  qDeleteAll( mRubberBands );
  mRubberBands.clear();
}
