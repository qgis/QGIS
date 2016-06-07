/***************************************************************************
    qgsrendererv2registry.cpp
    ---------------------
    begin                : November 2009
    copyright            : (C) 2009 by Martin Dobias
    email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgsrendererv2registry.h"

// default renderers
#include "qgssinglesymbolrendererv2.h"
#include "qgscategorizedsymbolrendererv2.h"
#include "qgsgraduatedsymbolrendererv2.h"
#include "qgsrulebasedrendererv2.h"
#include "qgspointdisplacementrenderer.h"
#include "qgsinvertedpolygonrenderer.h"
#include "qgsheatmaprenderer.h"
#include "qgs25drenderer.h"
#include "qgsnullsymbolrenderer.h"
#include "qgsvectorlayer.h"

QgsRendererV2Registry::QgsRendererV2Registry()
{
  // add default renderers
  addRenderer( new QgsRendererV2Metadata( "nullSymbol",
                                          QObject::tr( "No symbols" ),
                                          QgsNullSymbolRenderer::create ) );

  addRenderer( new QgsRendererV2Metadata( "singleSymbol",
                                          QObject::tr( "Single symbol" ),
                                          QgsSingleSymbolRendererV2::create,
                                          QgsSingleSymbolRendererV2::createFromSld ) );

  addRenderer( new QgsRendererV2Metadata( "categorizedSymbol",
                                          QObject::tr( "Categorized" ),
                                          QgsCategorizedSymbolRendererV2::create ) );

  addRenderer( new QgsRendererV2Metadata( "graduatedSymbol",
                                          QObject::tr( "Graduated" ),
                                          QgsGraduatedSymbolRendererV2::create ) );

  addRenderer( new QgsRendererV2Metadata( "RuleRenderer",
                                          QObject::tr( "Rule-based" ),
                                          QgsRuleBasedRendererV2::create,
                                          QgsRuleBasedRendererV2::createFromSld ) );

  addRenderer( new QgsRendererV2Metadata( "pointDisplacement",
                                          QObject::tr( "Point displacement" ),
                                          QgsPointDisplacementRenderer::create,
                                          QIcon(),
                                          nullptr,
                                          QgsRendererV2AbstractMetadata::PointLayer ) );

  addRenderer( new QgsRendererV2Metadata( "invertedPolygonRenderer",
                                          QObject::tr( "Inverted polygons" ),
                                          QgsInvertedPolygonRenderer::create,
                                          QIcon(),
                                          nullptr,
                                          QgsRendererV2AbstractMetadata::PolygonLayer ) );

  addRenderer( new QgsRendererV2Metadata( "heatmapRenderer",
                                          QObject::tr( "Heatmap" ),
                                          QgsHeatmapRenderer::create,
                                          QIcon(),
                                          nullptr,
                                          QgsRendererV2AbstractMetadata::PointLayer ) );


  addRenderer( new QgsRendererV2Metadata( "25dRenderer",
                                          QObject::tr( "2.5 D" ),
                                          Qgs25DRenderer::create,
                                          QIcon(),
                                          nullptr,
                                          QgsRendererV2AbstractMetadata::PolygonLayer ) );
}

QgsRendererV2Registry::~QgsRendererV2Registry()
{
  qDeleteAll( mRenderers );
}

QgsRendererV2Registry* QgsRendererV2Registry::instance()
{
  static QgsRendererV2Registry mInstance;
  return &mInstance;
}


bool QgsRendererV2Registry::addRenderer( QgsRendererV2AbstractMetadata* metadata )
{
  if ( !metadata || mRenderers.contains( metadata->name() ) )
    return false;

  mRenderers[metadata->name()] = metadata;
  mRenderersOrder << metadata->name();
  return true;
}

bool QgsRendererV2Registry::removeRenderer( const QString& rendererName )
{
  if ( !mRenderers.contains( rendererName ) )
    return false;

  delete mRenderers[rendererName];
  mRenderers.remove( rendererName );
  mRenderersOrder.removeAll( rendererName );
  return true;
}

QgsRendererV2AbstractMetadata* QgsRendererV2Registry::rendererMetadata( const QString& rendererName )
{
  return mRenderers.value( rendererName );
}

QgsRendererV2Metadata::~QgsRendererV2Metadata() {}

QStringList QgsRendererV2Registry::renderersList( QgsRendererV2AbstractMetadata::LayerTypes layerTypes ) const
{
  QStringList renderers;
  Q_FOREACH ( const QString& renderer, mRenderersOrder )
  {
    if ( mRenderers.value( renderer )->compatibleLayerTypes() & layerTypes )
      renderers << renderer;
  }
  return renderers;
}

QStringList QgsRendererV2Registry::renderersList( const QgsVectorLayer* layer ) const
{
  QgsRendererV2AbstractMetadata::LayerType layerType = QgsRendererV2AbstractMetadata::All;

  switch ( layer->geometryType() )
  {
    case QGis::Point:
      layerType = QgsRendererV2AbstractMetadata::PointLayer;
      break;

    case QGis::Line:
      layerType = QgsRendererV2AbstractMetadata::LineLayer;
      break;

    case QGis::Polygon:
      layerType = QgsRendererV2AbstractMetadata::PolygonLayer;
      break;

    case QGis::UnknownGeometry:
    case QGis::NoGeometry:
      break;
  }

  return renderersList( layerType );
}
