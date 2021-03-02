/***************************************************************************
    qgsembeddedsymbolrenderer.cpp
    ---------------------
    begin                : March 2021
    copyright            : (C) 2021 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsembeddedsymbolrenderer.h"
#include "qgspainteffectregistry.h"

QgsEmbeddedSymbolRenderer::QgsEmbeddedSymbolRenderer()
  : QgsFeatureRenderer( QStringLiteral( "embeddedSymbol" ) )
{
}

QgsEmbeddedSymbolRenderer::~QgsEmbeddedSymbolRenderer() = default;

QgsSymbol *QgsEmbeddedSymbolRenderer::symbolForFeature( const QgsFeature &feature, QgsRenderContext & ) const
{
  return const_cast< QgsSymbol * >( feature.embeddedSymbol() );
}

QgsSymbol *QgsEmbeddedSymbolRenderer::originalSymbolForFeature( const QgsFeature &feature, QgsRenderContext &context ) const
{
  Q_UNUSED( context )
  return const_cast< QgsSymbol * >( feature.embeddedSymbol() );
}

void QgsEmbeddedSymbolRenderer::startRender( QgsRenderContext &context, const QgsFields &fields )
{
  QgsFeatureRenderer::startRender( context, fields );
}

bool QgsEmbeddedSymbolRenderer::renderFeature( const QgsFeature &feature, QgsRenderContext &context, int layer, bool selected, bool drawVertexMarker )
{
  if ( const QgsSymbol *symbol = feature.embeddedSymbol() )
  {
    std::unique_ptr< QgsSymbol > clone( symbol->clone() );

    clone->startRender( context );
    renderFeatureWithSymbol( feature, clone.get(), context, layer, selected, drawVertexMarker );
    clone->stopRender( context );
    return true;
  }
  else
  {
    return false;
  }
  return true;
}

void QgsEmbeddedSymbolRenderer::stopRender( QgsRenderContext &context )
{
  QgsFeatureRenderer::stopRender( context );
}

QSet<QString> QgsEmbeddedSymbolRenderer::usedAttributes( const QgsRenderContext & ) const
{
  return QSet<QString>();
}

bool QgsEmbeddedSymbolRenderer::usesEmbeddedSymbols() const
{
  return true;
}

QgsEmbeddedSymbolRenderer *QgsEmbeddedSymbolRenderer::clone() const
{
  QgsEmbeddedSymbolRenderer *r = new QgsEmbeddedSymbolRenderer();
  r->setUsingSymbolLevels( usingSymbolLevels() );
  copyRendererData( r );
  return r;
}

QgsFeatureRenderer::Capabilities QgsEmbeddedSymbolRenderer::capabilities()
{
  return SymbolLevels;
}

QgsFeatureRenderer *QgsEmbeddedSymbolRenderer::create( QDomElement &, const QgsReadWriteContext & )
{
  QgsEmbeddedSymbolRenderer *r = new QgsEmbeddedSymbolRenderer();
  return r;
}

QDomElement QgsEmbeddedSymbolRenderer::save( QDomDocument &doc, const QgsReadWriteContext & )
{
  QDomElement rendererElem = doc.createElement( RENDERER_TAG_NAME );
  rendererElem.setAttribute( QStringLiteral( "type" ), QStringLiteral( "embeddedSymbol" ) );
  rendererElem.setAttribute( QStringLiteral( "symbollevels" ), ( mUsingSymbolLevels ? QStringLiteral( "1" ) : QStringLiteral( "0" ) ) );
  rendererElem.setAttribute( QStringLiteral( "forceraster" ), ( mForceRaster ? QStringLiteral( "1" ) : QStringLiteral( "0" ) ) );

  if ( mPaintEffect && !QgsPaintEffectRegistry::isDefaultStack( mPaintEffect ) )
    mPaintEffect->saveProperties( doc, rendererElem );

  if ( !mOrderBy.isEmpty() )
  {
    QDomElement orderBy = doc.createElement( QStringLiteral( "orderby" ) );
    mOrderBy.save( orderBy );
    rendererElem.appendChild( orderBy );
  }
  rendererElem.setAttribute( QStringLiteral( "enableorderby" ), ( mOrderByEnabled ? QStringLiteral( "1" ) : QStringLiteral( "0" ) ) );

  return rendererElem;
}
