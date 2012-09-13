/***************************************************************************
    qgsrendererv2registry.cpp
    ---------------------
    begin                : November 2009
    copyright            : (C) 2009 by Martin Dobias
    email                : wonder.sk at gmail.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgsrendererv2registry.h"

// default renderers
#include "qgssinglesymbolrendererv2.h"
#include "qgscategorizedsymbolrendererv2.h"
#include "qgsgraduatedsymbolrendererv2.h"
#include "qgsrulebasedrendererv2.h"
#include "qgspointdisplacementrenderer.h"

QgsRendererV2Registry* QgsRendererV2Registry::mInstance = NULL;

QgsRendererV2Registry::QgsRendererV2Registry()
{
  // add default renderers
  addRenderer( new QgsRendererV2Metadata( "singleSymbol",
                                          QObject::tr( "Single Symbol" ),
                                          QgsSingleSymbolRendererV2::create,
                                          QgsSingleSymbolRendererV2::createFromSld ) );

  addRenderer( new QgsRendererV2Metadata( "categorizedSymbol",
                                          QObject::tr( "Categorized" ),
                                          QgsCategorizedSymbolRendererV2::create ) );
  addRenderer( new QgsRendererV2Metadata( "graduatedSymbol",
                                          QObject::tr( "Graduated" ),
                                          QgsGraduatedSymbolRendererV2::create ) );

  addRenderer( new QgsRendererV2Metadata( "RuleRenderer",
                                          QObject::tr( "Rule-based" ),
                                          QgsRuleBasedRendererV2::create,
                                          QgsRuleBasedRendererV2::createFromSld ) );

  addRenderer( new QgsRendererV2Metadata( "pointDisplacement",
                                          QObject::tr( "Point displacement" ),
                                          QgsPointDisplacementRenderer::create ) );
}

QgsRendererV2Registry::~QgsRendererV2Registry()
{
  foreach ( QString name, mRenderers.keys() )
  {
    delete mRenderers[name];
  }
  mRenderers.clear();
}

QgsRendererV2Registry* QgsRendererV2Registry::instance()
{
  if ( !mInstance )
    mInstance = new QgsRendererV2Registry();

  return mInstance;
}


bool QgsRendererV2Registry::addRenderer( QgsRendererV2AbstractMetadata* metadata )
{
  if ( metadata == NULL || mRenderers.contains( metadata->name() ) )
    return false;

  mRenderers[metadata->name()] = metadata;
  mRenderersOrder << metadata->name();
  return true;
}

bool QgsRendererV2Registry::removeRenderer( QString rendererName )
{
  if ( !mRenderers.contains( rendererName ) )
    return false;

  delete mRenderers[rendererName];
  mRenderers.remove( rendererName );
  mRenderersOrder.removeAll( rendererName );
  return true;
}

QgsRendererV2AbstractMetadata* QgsRendererV2Registry::rendererMetadata( QString rendererName )
{
  return mRenderers.value( rendererName );
}

QStringList QgsRendererV2Registry::renderersList()
{
  return mRenderersOrder;
}
