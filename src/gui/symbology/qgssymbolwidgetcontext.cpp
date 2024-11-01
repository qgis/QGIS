/***************************************************************************
    qgssymbolwidgetcontext.cpp
    --------------------------
    begin                : September 2016
    copyright            : (C) 2016 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgssymbolwidgetcontext.h"
#include "qgsmapcanvas.h"
#include "qgsmessagebar.h"
#include "qgsproject.h"
#include "qgsexpressioncontextutils.h"
#include "qgstemporalcontroller.h"

QgsSymbolWidgetContext::QgsSymbolWidgetContext( const QgsSymbolWidgetContext &other )
  : mMapCanvas( other.mMapCanvas )
  , mMessageBar( other.mMessageBar )
  , mAdditionalScopes( other.mAdditionalScopes )
  , mSymbolType( other.mSymbolType )
{
  if ( other.mExpressionContext )
  {
    mExpressionContext.reset( new QgsExpressionContext( *other.mExpressionContext ) );
  }
}

QgsSymbolWidgetContext &QgsSymbolWidgetContext::operator=( const QgsSymbolWidgetContext &other )
{
  mMapCanvas = other.mMapCanvas;
  mMessageBar = other.mMessageBar;
  mAdditionalScopes = other.mAdditionalScopes;
  mSymbolType = other.mSymbolType;
  if ( other.mExpressionContext )
  {
    mExpressionContext.reset( new QgsExpressionContext( *other.mExpressionContext ) );
  }
  else
  {
    mExpressionContext.reset();
  }
  return *this;
}

void QgsSymbolWidgetContext::setMapCanvas( QgsMapCanvas *canvas )
{
  mMapCanvas = canvas;
}

QgsMapCanvas *QgsSymbolWidgetContext::mapCanvas() const
{
  return mMapCanvas;
}

void QgsSymbolWidgetContext::setMessageBar( QgsMessageBar *bar )
{
  mMessageBar = bar;
}

QgsMessageBar *QgsSymbolWidgetContext::messageBar() const
{
  return mMessageBar;
}

void QgsSymbolWidgetContext::setExpressionContext( QgsExpressionContext *context )
{
  if ( context )
    mExpressionContext.reset( new QgsExpressionContext( *context ) );
  else
    mExpressionContext.reset();
}

QgsExpressionContext *QgsSymbolWidgetContext::expressionContext() const
{
  return mExpressionContext.get();
}

void QgsSymbolWidgetContext::setAdditionalExpressionContextScopes( const QList<QgsExpressionContextScope> &scopes )
{
  mAdditionalScopes = scopes;
}

QList<QgsExpressionContextScope> QgsSymbolWidgetContext::additionalExpressionContextScopes() const
{
  return mAdditionalScopes;
}

QList<QgsExpressionContextScope *> QgsSymbolWidgetContext::globalProjectAtlasMapLayerScopes( const QgsMapLayer *layer ) const
{
  QList<QgsExpressionContextScope *> scopes;
  scopes << QgsExpressionContextUtils::globalScope()
         << QgsExpressionContextUtils::projectScope( QgsProject::instance() )
         << QgsExpressionContextUtils::atlasScope( nullptr );
  if ( mMapCanvas )
  {
    scopes << QgsExpressionContextUtils::mapSettingsScope( mMapCanvas->mapSettings() )
           << mMapCanvas->defaultExpressionContextScope()
           << new QgsExpressionContextScope( mMapCanvas->expressionContextScope() );

    if ( const QgsExpressionContextScopeGenerator *generator = dynamic_cast<const QgsExpressionContextScopeGenerator *>( mMapCanvas->temporalController() ) )
    {
      scopes << generator->createExpressionContextScope();
    }
  }
  else
  {
    scopes << QgsExpressionContextUtils::mapSettingsScope( QgsMapSettings() );
  }
  if ( layer )
    scopes << QgsExpressionContextUtils::layerScope( layer );
  return scopes;
}

Qgis::SymbolType QgsSymbolWidgetContext::symbolType() const
{
  return mSymbolType;
}

void QgsSymbolWidgetContext::setSymbolType( Qgis::SymbolType type )
{
  mSymbolType = type;
}
