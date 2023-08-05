/***************************************************************************
                         qgstiledscenetexturecolorrenderer.h
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

#include "qgstiledscenetexturecolorrenderer.h"

QgsTiledSceneTextureColorRenderer::QgsTiledSceneTextureColorRenderer()
{

}

QString QgsTiledSceneTextureColorRenderer::type() const
{
  return QStringLiteral( "texturecolor" );
}

QgsTiledSceneRenderer *QgsTiledSceneTextureColorRenderer::clone() const
{
  std::unique_ptr< QgsTiledSceneTextureColorRenderer > res = std::make_unique< QgsTiledSceneTextureColorRenderer >();

  copyCommonProperties( res.get() );

  return res.release();
}

QgsTiledSceneRenderer *QgsTiledSceneTextureColorRenderer::create( QDomElement &element, const QgsReadWriteContext &context )
{
  std::unique_ptr< QgsTiledSceneTextureColorRenderer > r = std::make_unique< QgsTiledSceneTextureColorRenderer >();

  r->restoreCommonProperties( element, context );

  return r.release();
}

QDomElement QgsTiledSceneTextureColorRenderer::save( QDomDocument &doc, const QgsReadWriteContext &context ) const
{
  QDomElement rendererElem = doc.createElement( QStringLiteral( "renderer" ) );

  rendererElem.setAttribute( QStringLiteral( "type" ), QStringLiteral( "texturecolor" ) );

  saveCommonProperties( rendererElem, context );

  return rendererElem;
}

Qgis::TiledSceneRendererFlags QgsTiledSceneTextureColorRenderer::flags() const
{
  return Qgis::TiledSceneRendererFlag::RequiresTextures;
}

void QgsTiledSceneTextureColorRenderer::renderTriangle( QgsTiledSceneRenderContext &context, const QPolygonF &triangle )
{
  const QImage textureImage = context.textureImage();
  if ( textureImage.isNull() )
    return;

  double textureX1;
  double textureY1;
  double textureX2;
  double textureY2;
  double textureX3;
  double textureY3;
  context.textureCoordinates( textureX1, textureY1, textureX2, textureY2, textureX3, textureY3 );

  const QColor centerColor( textureImage.pixelColor(
                              ( ( textureX1 + textureX2 + textureX3 ) / 3 ) * ( textureImage.width() - 1 ),
                              ( ( textureY1 + textureY2 + textureY3 ) / 3 ) * ( textureImage.height() - 1 ) )
                          );
  QBrush b( centerColor );
  QPainter *painter = context.renderContext().painter();
  painter->setPen( Qt::NoPen );
  painter->setBrush( b );
  painter->drawPolygon( triangle );
}
