/***************************************************************************
    qgsinvertedpolygonrenderer.cpp
    ---------------------
    begin                : April 2014
    copyright            : (C) 2014 Hugo Mercier / Oslandia
    email                : hugo dot mercier at oslandia dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsinvertedpolygonrenderer.h"

#include "qgssymbol.h"
#include "qgssymbollayerutils.h"

#include "qgslogger.h"
#include "qgsfeature.h"
#include "qgsvectorlayer.h"
#include "qgssymbollayer.h"
#include "qgsogcutils.h"
#include "qgspainteffect.h"
#include "qgspainteffectregistry.h"
#include "qgsstyleentityvisitor.h"

#include <QDomDocument>
#include <QDomElement>

QgsInvertedPolygonRenderer::QgsInvertedPolygonRenderer( QgsFeatureRenderer *subRenderer )
  : QgsMergedFeatureRenderer( QStringLiteral( "invertedPolygonRenderer" ), subRenderer )
{
  if ( !subRenderer )
  {
    mSubRenderer.reset( QgsFeatureRenderer::defaultRenderer( QgsWkbTypes::PolygonGeometry ) );
  }
  mOperation = InvertOnly;
}

QString QgsInvertedPolygonRenderer::dump() const
{
  if ( !mSubRenderer )
  {
    return QStringLiteral( "INVERTED: NULL" );
  }
  return "INVERTED [" + mSubRenderer->dump() + ']';
}

QgsInvertedPolygonRenderer *QgsInvertedPolygonRenderer::clone() const
{
  QgsInvertedPolygonRenderer *newRenderer = nullptr;
  if ( !mSubRenderer )
  {
    newRenderer = new QgsInvertedPolygonRenderer( nullptr );
  }
  else
  {
    newRenderer = new QgsInvertedPolygonRenderer( mSubRenderer->clone() );
  }
  newRenderer->setPreprocessingEnabled( preprocessingEnabled() );
  copyRendererData( newRenderer );
  return newRenderer;
}

QgsFeatureRenderer *QgsInvertedPolygonRenderer::create( QDomElement &element, const QgsReadWriteContext &context )
{
  QgsInvertedPolygonRenderer *r = new QgsInvertedPolygonRenderer();
  //look for an embedded renderer <renderer-v2>
  QDomElement embeddedRendererElem = element.firstChildElement( QStringLiteral( "renderer-v2" ) );
  if ( !embeddedRendererElem.isNull() )
  {
    QgsFeatureRenderer *renderer = QgsFeatureRenderer::load( embeddedRendererElem, context );
    r->setEmbeddedRenderer( renderer );
  }
  r->setPreprocessingEnabled( element.attribute( QStringLiteral( "preprocessing" ), QStringLiteral( "0" ) ).toInt() == 1 );
  return r;
}

QDomElement QgsInvertedPolygonRenderer::save( QDomDocument &doc, const QgsReadWriteContext &context )
{
  // clazy:skip

  QDomElement rendererElem = doc.createElement( RENDERER_TAG_NAME );
  rendererElem.setAttribute( QStringLiteral( "type" ), QStringLiteral( "invertedPolygonRenderer" ) );
  rendererElem.setAttribute( QStringLiteral( "preprocessing" ), preprocessingEnabled() ? QStringLiteral( "1" ) : QStringLiteral( "0" ) );

  if ( mSubRenderer )
  {
    const QDomElement embeddedRendererElem = mSubRenderer->save( doc, context );
    rendererElem.appendChild( embeddedRendererElem );
  }

  saveRendererData( doc, rendererElem, context );

  return rendererElem;
}

QgsInvertedPolygonRenderer *QgsInvertedPolygonRenderer::convertFromRenderer( const QgsFeatureRenderer *renderer )
{
  if ( renderer->type() == QLatin1String( "invertedPolygonRenderer" ) )
  {
    return dynamic_cast<QgsInvertedPolygonRenderer *>( renderer->clone() );
  }
  else if ( renderer->type() == QLatin1String( "singleSymbol" ) ||
            renderer->type() == QLatin1String( "categorizedSymbol" ) ||
            renderer->type() == QLatin1String( "graduatedSymbol" ) ||
            renderer->type() == QLatin1String( "RuleRenderer" ) )
  {
    std::unique_ptr< QgsInvertedPolygonRenderer > res = std::make_unique< QgsInvertedPolygonRenderer >( renderer->clone() );
    renderer->copyRendererData( res.get() );
    return res.release();
  }
  else if ( renderer->type() == QLatin1String( "mergedFeatureRenderer" ) )
  {
    std::unique_ptr< QgsInvertedPolygonRenderer > res = std::make_unique< QgsInvertedPolygonRenderer >( renderer->embeddedRenderer() ? renderer->embeddedRenderer()->clone() : nullptr );
    renderer->copyRendererData( res.get() );
    return res.release();
  }
  return nullptr;
}

