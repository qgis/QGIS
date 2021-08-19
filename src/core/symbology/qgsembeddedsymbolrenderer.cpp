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
#include "qgssymbollayerutils.h"
#include "qgssinglesymbolrenderer.h"
#include "qgssymbol.h"
#include "qgspainteffect.h"

QgsEmbeddedSymbolRenderer::QgsEmbeddedSymbolRenderer( QgsSymbol *defaultSymbol )
  : QgsFeatureRenderer( QStringLiteral( "embeddedSymbol" ) )
  , mDefaultSymbol( defaultSymbol )
{
  Q_ASSERT( mDefaultSymbol );
}

QgsEmbeddedSymbolRenderer::~QgsEmbeddedSymbolRenderer() = default;

QgsSymbol *QgsEmbeddedSymbolRenderer::symbolForFeature( const QgsFeature &feature, QgsRenderContext & ) const
{
  if ( feature.embeddedSymbol() )
    return const_cast< QgsSymbol * >( feature.embeddedSymbol() );
  else
    return mDefaultSymbol.get();
}

QgsSymbol *QgsEmbeddedSymbolRenderer::originalSymbolForFeature( const QgsFeature &feature, QgsRenderContext &context ) const
{
  Q_UNUSED( context )
  if ( feature.embeddedSymbol() )
    return const_cast< QgsSymbol * >( feature.embeddedSymbol() );
  else
    return mDefaultSymbol.get();
}

void QgsEmbeddedSymbolRenderer::startRender( QgsRenderContext &context, const QgsFields &fields )
{
  QgsFeatureRenderer::startRender( context, fields );

  mDefaultSymbol->startRender( context, fields );
}

bool QgsEmbeddedSymbolRenderer::renderFeature( const QgsFeature &feature, QgsRenderContext &context, int layer, bool selected, bool drawVertexMarker )
{
  if ( const QgsSymbol *symbol = feature.embeddedSymbol() )
  {
    std::unique_ptr< QgsSymbol > clone( symbol->clone() );

    clone->startRender( context );
    renderFeatureWithSymbol( feature, clone.get(), context, layer, selected, drawVertexMarker );
    clone->stopRender( context );
  }
  else
  {
    renderFeatureWithSymbol( feature, mDefaultSymbol.get(), context, layer, selected, drawVertexMarker );
  }
  return true;
}

void QgsEmbeddedSymbolRenderer::stopRender( QgsRenderContext &context )
{
  QgsFeatureRenderer::stopRender( context );
  mDefaultSymbol->stopRender( context );
}

QSet<QString> QgsEmbeddedSymbolRenderer::usedAttributes( const QgsRenderContext &context ) const
{
  QSet<QString> attributes;
  if ( mDefaultSymbol )
    attributes.unite( mDefaultSymbol->usedAttributes( context ) );
  return attributes;
}

bool QgsEmbeddedSymbolRenderer::usesEmbeddedSymbols() const
{
  return true;
}

QgsEmbeddedSymbolRenderer *QgsEmbeddedSymbolRenderer::clone() const
{
  QgsEmbeddedSymbolRenderer *r = new QgsEmbeddedSymbolRenderer( mDefaultSymbol->clone() );
  copyRendererData( r );
  return r;
}

QgsFeatureRenderer::Capabilities QgsEmbeddedSymbolRenderer::capabilities()
{
  return SymbolLevels;
}

QgsFeatureRenderer *QgsEmbeddedSymbolRenderer::create( QDomElement &element, const QgsReadWriteContext &context )
{
  QDomElement symbolsElem = element.firstChildElement( QStringLiteral( "symbols" ) );
  if ( symbolsElem.isNull() )
    return nullptr;

  QgsSymbolMap symbolMap = QgsSymbolLayerUtils::loadSymbols( symbolsElem, context );

  if ( !symbolMap.contains( QStringLiteral( "0" ) ) )
    return nullptr;

  QgsEmbeddedSymbolRenderer *r = new QgsEmbeddedSymbolRenderer( symbolMap.take( QStringLiteral( "0" ) ) );
  return r;
}

QgsEmbeddedSymbolRenderer *QgsEmbeddedSymbolRenderer::convertFromRenderer( const QgsFeatureRenderer *renderer )
{
  if ( renderer->type() == QLatin1String( "embeddedSymbol" ) )
  {
    return dynamic_cast<QgsEmbeddedSymbolRenderer *>( renderer->clone() );
  }
  else if ( renderer->type() == QLatin1String( "singleSymbol" ) )
  {
    std::unique_ptr< QgsEmbeddedSymbolRenderer > symbolRenderer = std::make_unique< QgsEmbeddedSymbolRenderer >( static_cast< const QgsSingleSymbolRenderer * >( renderer )->symbol()->clone() );
    renderer->copyRendererData( symbolRenderer.get() );
    return symbolRenderer.release();
  }
  else
  {
    return nullptr;
  }
}

QDomElement QgsEmbeddedSymbolRenderer::save( QDomDocument &doc, const QgsReadWriteContext &context )
{
  QDomElement rendererElem = doc.createElement( RENDERER_TAG_NAME );
  rendererElem.setAttribute( QStringLiteral( "type" ), QStringLiteral( "embeddedSymbol" ) );

  QgsSymbolMap symbols;
  symbols[QStringLiteral( "0" )] = mDefaultSymbol.get();
  const QDomElement symbolsElem = QgsSymbolLayerUtils::saveSymbols( symbols, QStringLiteral( "symbols" ), doc, context );
  rendererElem.appendChild( symbolsElem );

  saveRendererData( doc, rendererElem, context );

  return rendererElem;
}

QgsSymbolList QgsEmbeddedSymbolRenderer::symbols( QgsRenderContext &context ) const
{
  Q_UNUSED( context )
  QgsSymbolList lst;
  lst.append( mDefaultSymbol.get() );
  return lst;
}

QgsSymbol *QgsEmbeddedSymbolRenderer::defaultSymbol() const
{
  return mDefaultSymbol.get();
}

void QgsEmbeddedSymbolRenderer::setDefaultSymbol( QgsSymbol *symbol )
{
  Q_ASSERT( symbol );
  mDefaultSymbol.reset( symbol );
}
