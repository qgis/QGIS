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
#include "qgssymbollayerutils.h"
#include "qgslinesymbol.h"

QgsTiledSceneWireframeRenderer::QgsTiledSceneWireframeRenderer()
{
  mFillSymbol.reset( createDefaultFillSymbol() );
  mLineSymbol.reset( createDefaultLineSymbol() );
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
  res->setLineSymbol( mLineSymbol->clone() );
  res->setUseTextureColors( mUseTextureColors );

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
  {
    const QDomElement lineSymbolElem = element.firstChildElement( QStringLiteral( "lineSymbol" ) );
    if ( !lineSymbolElem.isNull() )
    {
      const QDomElement symbolElem = lineSymbolElem.firstChildElement( QStringLiteral( "symbol" ) );
      std::unique_ptr< QgsLineSymbol > lineSymbol( QgsSymbolLayerUtils::loadSymbol<QgsLineSymbol>( symbolElem, context ) );
      if ( lineSymbol )
        r->mLineSymbol = std::move( lineSymbol );
    }
  }

  r->setUseTextureColors( element.attribute( QStringLiteral( "useTextureColors" ), QStringLiteral( "0" ) ).toInt() );

  r->restoreCommonProperties( element, context );
  return r.release();
}

QgsFillSymbol *QgsTiledSceneWireframeRenderer::createDefaultFillSymbol()
{
  QVariantMap properties;
  properties.insert( QStringLiteral( "color" ), QStringLiteral( "white" ) );
  properties.insert( QStringLiteral( "style" ), QStringLiteral( "solid" ) );
  properties.insert( QStringLiteral( "style_border" ), QStringLiteral( "solid" ) );
  properties.insert( QStringLiteral( "color_border" ), QStringLiteral( "black" ) );
  properties.insert( QStringLiteral( "width_border" ), QStringLiteral( "0.3" ) );
  properties.insert( QStringLiteral( "joinstyle" ), QStringLiteral( "miter" ) );

  return QgsFillSymbol::createSimple( properties );
}

QgsFillSymbol *QgsTiledSceneWireframeRenderer::fillSymbol() const
{
  return mFillSymbol.get();
}

void QgsTiledSceneWireframeRenderer::setFillSymbol( QgsFillSymbol *symbol )
{
  mFillSymbol.reset( symbol );
}

QgsLineSymbol *QgsTiledSceneWireframeRenderer::createDefaultLineSymbol()
{
  QVariantMap properties;
  properties.insert( QStringLiteral( "color" ), QStringLiteral( "red" ) );

  return QgsLineSymbol::createSimple( properties );
}

QgsLineSymbol *QgsTiledSceneWireframeRenderer::lineSymbol() const
{
  return mLineSymbol.get();
}

void QgsTiledSceneWireframeRenderer::setLineSymbol( QgsLineSymbol *symbol )
{
  mLineSymbol.reset( symbol );
}

bool QgsTiledSceneWireframeRenderer::useTextureColors() const
{
  return mUseTextureColors;
}

void QgsTiledSceneWireframeRenderer::setUseTextureColors( bool newUseTextureColors )
{
  mUseTextureColors = newUseTextureColors;
}

QDomElement QgsTiledSceneWireframeRenderer::save( QDomDocument &doc, const QgsReadWriteContext &context ) const
{
  QDomElement rendererElem = doc.createElement( QStringLiteral( "renderer" ) );

  rendererElem.setAttribute( QStringLiteral( "type" ), QStringLiteral( "wireframe" ) );
  rendererElem.setAttribute( QStringLiteral( "useTextureColors" ), mUseTextureColors ? QStringLiteral( "1" ) : QStringLiteral( "0" ) );

  {
    QDomElement fillSymbolElem = doc.createElement( QStringLiteral( "fillSymbol" ) );
    const QDomElement symbolElement = QgsSymbolLayerUtils::saveSymbol( QString(),
                                      mFillSymbol.get(),
                                      doc,
                                      context );
    fillSymbolElem.appendChild( symbolElement );
    rendererElem.appendChild( fillSymbolElem );
  }
  {
    QDomElement lineSymbolElem = doc.createElement( QStringLiteral( "lineSymbol" ) );
    const QDomElement symbolElement = QgsSymbolLayerUtils::saveSymbol( QString(),
                                      mLineSymbol.get(),
                                      doc,
                                      context );
    lineSymbolElem.appendChild( symbolElement );
    rendererElem.appendChild( lineSymbolElem );
  }
  saveCommonProperties( rendererElem, context );

  return rendererElem;
}

void QgsTiledSceneWireframeRenderer::renderTriangle( QgsTiledSceneRenderContext &context, const QPolygonF &triangle )
{
  if ( mUseTextureColors )
  {
    std::unique_ptr< QgsFillSymbol > s( mFillSymbol->clone() );
    const QImage textureImage = context.textureImage();
    if ( !textureImage.isNull() )
    {
      float textureX1;
      float textureY1;
      float textureX2;
      float textureY2;
      float textureX3;
      float textureY3;
      context.textureCoordinates( textureX1, textureY1, textureX2, textureY2, textureX3, textureY3 );

      const QColor centerColor( textureImage.pixelColor(
                                  static_cast<int>( ( ( textureX1 + textureX2 + textureX3 ) / 3 ) * ( textureImage.width() - 1 ) ),
                                  static_cast< int >( ( ( textureY1 + textureY2 + textureY3 ) / 3 ) * ( textureImage.height() - 1 ) ) )
                              );
      s->setColor( centerColor );
    }
    s->startRender( context.renderContext() );
    s->renderPolygon( triangle, nullptr, nullptr, context.renderContext() );
    s->stopRender( context.renderContext() );
  }
  else
  {
    mFillSymbol->renderPolygon( triangle, nullptr, nullptr, context.renderContext() );
  }
}

void QgsTiledSceneWireframeRenderer::renderLine( QgsTiledSceneRenderContext &context, const QPolygonF &line )
{
  mLineSymbol->renderPolyline( line, nullptr, context.renderContext() );
}

void QgsTiledSceneWireframeRenderer::startRender( QgsTiledSceneRenderContext &context )
{
  QgsTiledSceneRenderer::startRender( context );

  if ( !mUseTextureColors )
    mFillSymbol->startRender( context.renderContext() );

  mLineSymbol->startRender( context.renderContext() );
}

void QgsTiledSceneWireframeRenderer::stopRender( QgsTiledSceneRenderContext &context )
{
  if ( !mUseTextureColors )
    mFillSymbol->stopRender( context.renderContext() );

  mLineSymbol->stopRender( context.renderContext() );

  QgsTiledSceneRenderer::stopRender( context );
}

Qgis::TiledSceneRendererFlags QgsTiledSceneWireframeRenderer::flags() const
{
  Qgis::TiledSceneRendererFlags flags = Qgis::TiledSceneRendererFlag::RendersTriangles |
                                        Qgis::TiledSceneRendererFlag::RendersLines;

  if ( mUseTextureColors )
    flags |= Qgis::TiledSceneRendererFlag::RequiresTextures;

  return flags;
}
