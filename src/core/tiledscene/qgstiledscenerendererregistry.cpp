/***************************************************************************
    qgstiledscenerendererregistry.cpp
    ---------------------
    begin                : August 2023
    copyright            : (C) 2023 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgstiledscenerendererregistry.h"
#include "qgstiledscenerenderer.h"

// default renderers
#include "qgstiledscenetexturerenderer.h"
#include "qgstiledscenewireframerenderer.h"

QgsTiledSceneRendererRegistry::QgsTiledSceneRendererRegistry()
{
  // add default renderers
  addRenderer( new QgsTiledSceneRendererMetadata( QStringLiteral( "texture" ),
               QObject::tr( "Textured" ),
               QgsTiledSceneTextureRenderer::create ) );
  addRenderer( new QgsTiledSceneRendererMetadata( QStringLiteral( "wireframe" ),
               QObject::tr( "Wireframe" ),
               QgsTiledSceneWireframeRenderer::create ) );
}

QgsTiledSceneRendererRegistry::~QgsTiledSceneRendererRegistry()
{
  qDeleteAll( mRenderers );
}

bool QgsTiledSceneRendererRegistry::addRenderer( QgsTiledSceneRendererAbstractMetadata *metadata )
{
  if ( !metadata || mRenderers.contains( metadata->name() ) )
    return false;

  mRenderers[metadata->name()] = metadata;
  mRenderersOrder << metadata->name();
  return true;
}

bool QgsTiledSceneRendererRegistry::removeRenderer( const QString &rendererName )
{
  if ( !mRenderers.contains( rendererName ) )
    return false;

  delete mRenderers[rendererName];
  mRenderers.remove( rendererName );
  mRenderersOrder.removeAll( rendererName );
  return true;
}

QgsTiledSceneRendererAbstractMetadata *QgsTiledSceneRendererRegistry::rendererMetadata( const QString &rendererName )
{
  return mRenderers.value( rendererName );
}

QStringList QgsTiledSceneRendererRegistry::renderersList() const
{
  QStringList renderers;
  for ( const QString &renderer : mRenderersOrder )
  {
    QgsTiledSceneRendererAbstractMetadata *r = mRenderers.value( renderer );
    if ( r )
      renderers << renderer;
  }
  return renderers;
}

QgsTiledSceneRenderer *QgsTiledSceneRendererRegistry::defaultRenderer( const QgsTiledSceneLayer * )
{
  return new QgsTiledSceneTextureRenderer();
}

