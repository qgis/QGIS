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
#include "qgssymbol.h"
#include "qgsmapcanvas.h"
#include "qgsmapsettings.h"
#include "qgsproject.h"
#include "qgsexpressioncontextutils.h"
#include "qgsmarkersymbol.h"
#include "qgslinesymbol.h"

#include <QPainter>
#include <cmath>


//
// QgsMapCanvasSymbolItem
//

QgsMapCanvasSymbolItem::QgsMapCanvasSymbolItem( QgsMapCanvas *canvas )
  : QgsMapCanvasItem( canvas )
  , mOpacityEffect( new QgsDrawSourceEffect() )
{
  setCacheMode( QGraphicsItem::ItemCoordinateCache );
}

QgsMapCanvasSymbolItem::~QgsMapCanvasSymbolItem() = default;

QgsRenderContext QgsMapCanvasSymbolItem::renderContext( QPainter *painter )
{
  QgsExpressionContext context;
  context << QgsExpressionContextUtils::globalScope()
          << QgsExpressionContextUtils::projectScope( QgsProject::instance() )
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
  context.setFields( mFeature.fields() );

  //setup render context
  QgsMapSettings ms = mMapCanvas->mapSettings();
  ms.setExpressionContext( context );
  QgsRenderContext rc = QgsRenderContext::fromMapSettings( ms );
  rc.setPainter( painter );

  return rc;
}

void QgsMapCanvasSymbolItem::paint( QPainter *painter )
{
  if ( !painter )
  {
    return;
  }

  QgsRenderContext rc = renderContext( painter );

  const bool useEffect = !qgsDoubleNear( mOpacityEffect->opacity(), 1.0 );
  if ( useEffect )
  {
    //use a paint effect to reduce opacity. If we directly set the opacity on the painter, then the symbol will NOT
    //be correctly "flattened" and parts of the symbol which should be obscured will show through
    mOpacityEffect->begin( rc );
  }

  mSymbol->startRender( rc, mFeature.fields() );
  renderSymbol( rc, mFeature );
  mSymbol->stopRender( rc );

  if ( useEffect )
  {
    mOpacityEffect->end( rc );
  }
}

void QgsMapCanvasSymbolItem::setSymbol( std::unique_ptr< QgsSymbol > symbol )
{
  mSymbol = std::move( symbol );
}

const QgsSymbol *QgsMapCanvasSymbolItem::symbol() const
{
  return mSymbol.get();
}

void QgsMapCanvasSymbolItem::setFeature( const QgsFeature &feature )
{
  mFeature = feature;
}
void QgsMapCanvasSymbolItem::setOpacity( double opacity )
{
  mOpacityEffect->setOpacity( opacity );
}

double QgsMapCanvasSymbolItem::opacity() const
{
  return mOpacityEffect->opacity();
}


//
// QgsPointMarkerItem
//

QgsMapCanvasMarkerSymbolItem::QgsMapCanvasMarkerSymbolItem( QgsMapCanvas *canvas )
  : QgsMapCanvasSymbolItem( canvas )
{
  setSymbol( std::make_unique< QgsMarkerSymbol >() );
}


void QgsMapCanvasMarkerSymbolItem::setPointLocation( const QgsPointXY &p )
{
  mLocation = toCanvasCoordinates( p );
}

void QgsMapCanvasMarkerSymbolItem::updateSize()
{
  QgsRenderContext rc = renderContext( nullptr );
  markerSymbol()->startRender( rc, mFeature.fields() );
  const QRectF bounds = markerSymbol()->bounds( mLocation, rc, mFeature );
  markerSymbol()->stopRender( rc );
  const QgsRectangle r( mMapCanvas->mapSettings().mapToPixel().toMapCoordinates( static_cast<int>( bounds.x() ),
                        static_cast<int>( bounds.y() ) ),
                        mMapCanvas->mapSettings().mapToPixel().toMapCoordinates( static_cast<int>( bounds.x() + bounds.width() * 2 ),
                            static_cast<int>( bounds.y() + bounds.height() * 2 ) ) );
  setRect( r );
}

void QgsMapCanvasMarkerSymbolItem::renderSymbol( QgsRenderContext &context, const QgsFeature &feature )
{
  markerSymbol()->renderPoint( mLocation - pos(), &feature, context );
}

QgsMarkerSymbol *QgsMapCanvasMarkerSymbolItem::markerSymbol()
{
  QgsMarkerSymbol *marker = dynamic_cast< QgsMarkerSymbol * >( mSymbol.get() );
  Q_ASSERT( marker );
  return marker;
}



//
// QgsLineMarkerItem
//

QgsMapCanvasLineSymbolItem::QgsMapCanvasLineSymbolItem( QgsMapCanvas *canvas )
  : QgsMapCanvasSymbolItem( canvas )
{
  setSymbol( std::make_unique< QgsLineSymbol >() );
}

void QgsMapCanvasLineSymbolItem::setLine( const QPolygonF &line )
{
  mLine = line;
  update();
}

void QgsMapCanvasLineSymbolItem::setLine( const QLineF &line )
{
  mLine.clear();
  mLine << line.p1() << line.p2();
  update();
}

QRectF QgsMapCanvasLineSymbolItem::boundingRect() const
{
  return mMapCanvas->rect();
}

void QgsMapCanvasLineSymbolItem::renderSymbol( QgsRenderContext &context, const QgsFeature &feature )
{
  lineSymbol()->renderPolyline( mLine, &feature, context );
}

QgsLineSymbol *QgsMapCanvasLineSymbolItem::lineSymbol()
{
  QgsLineSymbol *symbol = dynamic_cast< QgsLineSymbol * >( mSymbol.get() );
  Q_ASSERT( symbol );
  return symbol;
}


