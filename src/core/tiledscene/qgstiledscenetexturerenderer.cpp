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
