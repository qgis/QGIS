/***************************************************************************
                         qgstiledscenetexturerenderer.h
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

#include "qgstiledscenetexturerenderer.h"
#include "qgspainting.h"

QgsTiledSceneTextureRenderer::QgsTiledSceneTextureRenderer()
{

}

QString QgsTiledSceneTextureRenderer::type() const
{
  return QStringLiteral( "texture" );
}

QgsTiledSceneRenderer *QgsTiledSceneTextureRenderer::clone() const
{
  std::unique_ptr< QgsTiledSceneTextureRenderer > res = std::make_unique< QgsTiledSceneTextureRenderer >();

  copyCommonProperties( res.get() );

  return res.release();
}

QgsTiledSceneRenderer *QgsTiledSceneTextureRenderer::create( QDomElement &element, const QgsReadWriteContext &context )
{
  std::unique_ptr< QgsTiledSceneTextureRenderer > r = std::make_unique< QgsTiledSceneTextureRenderer >();

  r->restoreCommonProperties( element, context );

  return r.release();
}

QDomElement QgsTiledSceneTextureRenderer::save( QDomDocument &doc, const QgsReadWriteContext &context ) const
{
  QDomElement rendererElem = doc.createElement( QStringLiteral( "renderer" ) );

  rendererElem.setAttribute( QStringLiteral( "type" ), QStringLiteral( "texture" ) );

  saveCommonProperties( rendererElem, context );

  return rendererElem;
}

Qgis::TiledSceneRendererFlags QgsTiledSceneTextureRenderer::flags() const
{
  return Qgis::TiledSceneRendererFlag::RequiresTextures;
}

void QgsTiledSceneTextureRenderer::renderTriangle( QgsTiledSceneRenderContext &context, const QPolygonF &triangle )
{
  if ( context.textureImage().isNull() )
    return;

  float textureX1;
  float textureY1;
  float textureX2;
  float textureY2;
  float textureX3;
  float textureY3;
  context.textureCoordinates( textureX1, textureY1, textureX2, textureY2, textureX3, textureY3 );

  QPainter *painter = context.renderContext().painter();
  painter->setPen( Qt::NoPen );

  QgsPainting::drawTriangleUsingTexture(
    painter,
    triangle, context.textureImage(),
    textureX1, textureY1,
    textureX2, textureY2,
    textureX3, textureY3
  );
}
