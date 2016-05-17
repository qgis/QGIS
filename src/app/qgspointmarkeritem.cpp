/***************************************************************************
    qgspointmarkeritem.cpp
    ----------------------
    begin                : April 2016
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

#include "qgspointmarkeritem.h"
#include "qgssymbolv2.h"
#include "qgsmapcanvas.h"
#include "qgsmapsettings.h"
#include <QPainter>
#include <cmath>

QgsPointMarkerItem::QgsPointMarkerItem( QgsMapCanvas* canvas )
    : QgsMapCanvasItem( canvas )
    , mOpacityEffect( new QgsDrawSourceEffect() )
{
  setCacheMode( QGraphicsItem::ItemCoordinateCache );
}

QgsRenderContext QgsPointMarkerItem::renderContext( QPainter* painter )
{
  QgsExpressionContext context;
  context << QgsExpressionContextUtils::globalScope()
  << QgsExpressionContextUtils::projectScope()
  << QgsExpressionContextUtils::atlasScope( nullptr );
  if ( mMapCanvas )
  {
    context << QgsExpressionContextUtils::mapSettingsScope( mMapCanvas->mapSettings() )
    << new QgsExpressionContextScope( mMapCanvas->expressionContextScope() );
  }
  else
  {
    context << QgsExpressionContextUtils::mapSettingsScope( QgsMapSettings() );
  }
  //context << QgsExpressionContextUtils::layerScope( mLayer );
  context.setFeature( mFeature );

  //setup render context
  QgsMapSettings ms = mMapCanvas->mapSettings();
  ms.setExpressionContext( context );
  QgsRenderContext rc = QgsRenderContext::fromMapSettings( ms );
  rc.setPainter( painter );

  return rc;
}

void QgsPointMarkerItem::paint( QPainter * painter )
{
  if ( !painter )
  {
    return;
  }

  QgsRenderContext rc = renderContext( painter );

  bool useEffect = !qgsDoubleNear( mOpacityEffect->transparency(), 0.0 );
  if ( useEffect )
  {
    //use a paint effect to reduce opacity. If we directly set the opacity on the painter, then the symbol will NOT
    //be correctly "flattened" and parts of the symbol which should be obscured will show through
    mOpacityEffect->begin( rc );
  }

  mMarkerSymbol->startRender( rc, mFeature.fields() );
  mMarkerSymbol->renderPoint( mLocation - pos(), &mFeature, rc );
  mMarkerSymbol->stopRender( rc );

  if ( useEffect )
  {
    mOpacityEffect->end( rc );
  }
}

void QgsPointMarkerItem::setPointLocation( const QgsPoint& p )
{
  mLocation = toCanvasCoordinates( p );
}

void QgsPointMarkerItem::setSymbol( QgsMarkerSymbolV2 *symbol )
{
  mMarkerSymbol.reset( symbol );
}

QgsMarkerSymbolV2*QgsPointMarkerItem::symbol()
{
  return mMarkerSymbol.data();
}

void QgsPointMarkerItem::setFeature( const QgsFeature& feature )
{
  mFeature = feature;
}

void QgsPointMarkerItem::updateSize()
{
  QgsRenderContext rc = renderContext( nullptr );
  mMarkerSymbol->startRender( rc, mFeature.fields() );
  QRectF bounds =  mMarkerSymbol->bounds( mLocation, rc, mFeature );
  mMarkerSymbol->stopRender( rc );
  QgsRectangle r( mMapCanvas->mapSettings().mapToPixel().toMapCoordinates( bounds.x(), bounds.y() ),
                  mMapCanvas->mapSettings().mapToPixel().toMapCoordinates( bounds.x() + bounds.width() * 2, bounds.y() + bounds.height() * 2 ) );
  setRect( r );
}

void QgsPointMarkerItem::setTransparency( double transparency )
{
  mOpacityEffect->setTransparency( transparency );
}

double QgsPointMarkerItem::transparency() const
{
  return mOpacityEffect->transparency();
}

