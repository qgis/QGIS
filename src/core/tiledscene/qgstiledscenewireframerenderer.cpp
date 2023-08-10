/***************************************************************************
                         qgstiledscenewireframerenderer.h
                         --------------------
    begin                : August 2023
    copyright            : (C) 2023 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgstiledscenewireframerenderer.h"
#include "qgsfillsymbol.h"
#include "qgsfillsymbollayer.h"
#include "qgssymbollayerutils.h"

QgsTiledSceneWireframeRenderer::QgsTiledSceneWireframeRenderer()
{
  QVariantMap properties;
  properties.insert( QStringLiteral( "color" ), QStringLiteral( "white" ) );
  properties.insert( QStringLiteral( "style" ), QStringLiteral( "solid" ) );
  properties.insert( QStringLiteral( "style_border" ), QStringLiteral( "solid" ) );
  properties.insert( QStringLiteral( "color_border" ), QStringLiteral( "black" ) );
  properties.insert( QStringLiteral( "width_border" ), QStringLiteral( "0.3" ) );
  properties.insert( QStringLiteral( "joinstyle" ), QStringLiteral( "miter" ) );

  mFillSymbol.reset( QgsFillSymbol::createSimple( properties ) );
}

QgsTiledSceneWireframeRenderer::~QgsTiledSceneWireframeRenderer() = default;

QString QgsTiledSceneWireframeRenderer::type() const
{
  return QStringLiteral( "wireframe" );
}

QgsTiledSceneRenderer *QgsTiledSceneWireframeRenderer::clone() const
{
  std::unique_ptr< QgsTiledSceneWireframeRenderer > res = std::make_unique< QgsTiledSceneWireframeRenderer >();

  res->setFillSymbol( mFillSymbol->clone() );

  copyCommonProperties( res.get() );

  return res.release();
}

QgsTiledSceneRenderer *QgsTiledSceneWireframeRenderer::create( QDomElement &element, const QgsReadWriteContext &context )
{
  std::unique_ptr< QgsTiledSceneWireframeRenderer > r = std::make_unique< QgsTiledSceneWireframeRenderer >();
  {
    const QDomElement fillSymbolElem = element.firstChildElement( QStringLiteral( "fillSymbol" ) );
    if ( !fillSymbolElem.isNull() )
    {
      const QDomElement symbolElem = fillSymbolElem.firstChildElement( QStringLiteral( "symbol" ) );
      std::unique_ptr< QgsFillSymbol > fillSymbol( QgsSymbolLayerUtils::loadSymbol<QgsFillSymbol>( symbolElem, context ) );
      if ( fillSymbol )
        r->mFillSymbol = std::move( fillSymbol );
    }
  }

  r->restoreCommonProperties( element, context );
  return r.release();
}

QgsFillSymbol *QgsTiledSceneWireframeRenderer::fillSymbol() const
{
  return mFillSymbol.get();
}

void QgsTiledSceneWireframeRenderer::setFillSymbol( QgsFillSymbol *symbol )
{
  mFillSymbol.reset( symbol );
}

QDomElement QgsTiledSceneWireframeRenderer::save( QDomDocument &doc, const QgsReadWriteContext &context ) const
{
  QDomElement rendererElem = doc.createElement( QStringLiteral( "renderer" ) );

  rendererElem.setAttribute( QStringLiteral( "type" ), QStringLiteral( "wireframe" ) );

  {
    QDomElement fillSymbolElem = doc.createElement( QStringLiteral( "fillSymbol" ) );
    const QDomElement symbolElement = QgsSymbolLayerUtils::saveSymbol( QString(),
                                      mFillSymbol.get(),
                                      doc,
                                      context );
    fillSymbolElem.appendChild( symbolElement );
    rendererElem.appendChild( fillSymbolElem );
  }

  saveCommonProperties( rendererElem, context );

  return rendererElem;
}

void QgsTiledSceneWireframeRenderer::renderTriangle( QgsTiledSceneRenderContext &context, const QPolygonF &triangle )
{
  mFillSymbol->renderPolygon( triangle, nullptr, nullptr, context.renderContext() );
}

void QgsTiledSceneWireframeRenderer::startRender( QgsTiledSceneRenderContext &context )
{
  mFillSymbol->startRender( context.renderContext() );
}

void QgsTiledSceneWireframeRenderer::stopRender( QgsTiledSceneRenderContext &context )
{
  mFillSymbol->stopRender( context.renderContext() );
}
