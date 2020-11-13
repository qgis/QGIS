/***************************************************************************
    qgspointcloudrendererregistry.cpp
    ---------------------
    begin                : November 2020
    copyright            : (C) 2020 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgspointcloudrendererregistry.h"
#include "qgspointcloudrenderer.h"

// default renderers
#include "qgspointcloudrgbrenderer.h"

#include "qgspointcloudlayer.h"

QgsPointCloudRendererRegistry::QgsPointCloudRendererRegistry()
{
  // add default renderers
  addRenderer( new QgsPointCloudRendererMetadata( QStringLiteral( "rgb" ),
               QObject::tr( "RGB" ),
               QgsPointCloudRgbRenderer::create ) );
  addRenderer( new QgsPointCloudRendererMetadata( QStringLiteral( "dummy" ),
               QObject::tr( "Dummy" ),
               QgsDummyPointCloudRenderer::create ) );
}

QgsPointCloudRendererRegistry::~QgsPointCloudRendererRegistry()
{
  qDeleteAll( mRenderers );
}

bool QgsPointCloudRendererRegistry::addRenderer( QgsPointCloudRendererAbstractMetadata *metadata )
{
  if ( !metadata || mRenderers.contains( metadata->name() ) )
    return false;

  mRenderers[metadata->name()] = metadata;
  mRenderersOrder << metadata->name();
  return true;
}

bool QgsPointCloudRendererRegistry::removeRenderer( const QString &rendererName )
{
  if ( !mRenderers.contains( rendererName ) )
    return false;

  delete mRenderers[rendererName];
  mRenderers.remove( rendererName );
  mRenderersOrder.removeAll( rendererName );
  return true;
}

QgsPointCloudRendererAbstractMetadata *QgsPointCloudRendererRegistry::rendererMetadata( const QString &rendererName )
{
  return mRenderers.value( rendererName );
}

QStringList QgsPointCloudRendererRegistry::renderersList() const
{
  QStringList renderers;
  for ( const QString &renderer : mRenderersOrder )
  {
    QgsPointCloudRendererAbstractMetadata *r = mRenderers.value( renderer );
    if ( r )
      renderers << renderer;
  }
  return renderers;
}

QgsPointCloudRenderer *QgsPointCloudRendererRegistry::defaultRenderer( const QgsPointCloudAttributeCollection &attributes )
{
  //if red/green/blue attributes are present, then default to a RGB renderer
  if ( attributes.indexOf( QStringLiteral( "Red" ) ) >= 0 && attributes.indexOf( QStringLiteral( "Green" ) ) >= 0 && attributes.indexOf( QStringLiteral( "Blue" ) ) >= 0 )
    return new QgsPointCloudRgbRenderer();

  return new QgsDummyPointCloudRenderer();
}

