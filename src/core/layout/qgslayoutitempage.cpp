/***************************************************************************
                              qgslayoutitempage.cpp
                             ---------------------
    begin                : July 2017
    copyright            : (C) 2017 by Nyall Dawson
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

#include "qgslayoutitempage.h"
#include "qgslayout.h"
#include "qgslayoututils.h"
#include "qgssymbollayerutils.h"
#include <QPainter>

#define SHADOW_WIDTH_PIXELS 5
QgsLayoutItemPage::QgsLayoutItemPage( QgsLayout *layout )
  : QgsLayoutItem( layout )
{

}

void QgsLayoutItemPage::draw( QgsRenderContext &context, const QStyleOptionGraphicsItem * )
{
  if ( !context.painter() || !mLayout /*|| !mLayout->pagesVisible() */ )
  {
    return;
  }

  double scale = context.convertToPainterUnits( 1, QgsUnitTypes::RenderMillimeters );

  QgsExpressionContext expressionContext = createExpressionContext();
  context.setExpressionContext( expressionContext );

  QPainter *painter = context.painter();
  painter->save();

#if 0 //TODO
  if ( mComposition->plotStyle() ==  QgsComposition::Preview )
#endif
  {
    //if in preview mode, draw page border and shadow so that it's
    //still possible to tell where pages with a transparent style begin and end
    painter->setRenderHint( QPainter::Antialiasing, false );

    //shadow
    painter->setBrush( QBrush( QColor( 150, 150, 150 ) ) );
    painter->setPen( Qt::NoPen );
    painter->drawRect( QRectF( SHADOW_WIDTH_PIXELS, SHADOW_WIDTH_PIXELS, rect().width() * scale + SHADOW_WIDTH_PIXELS, rect().height() * scale + SHADOW_WIDTH_PIXELS ) );

    //page area
    painter->setBrush( QColor( 215, 215, 215 ) );
    QPen pagePen = QPen( QColor( 100, 100, 100 ), 0 );
    pagePen.setCosmetic( true );
    painter->setPen( pagePen );
    painter->drawRect( QRectF( 0, 0, scale * rect().width(), scale * rect().height() ) );
  }

  std::unique_ptr< QgsFillSymbol > symbol( mLayout->pageCollection()->pageStyleSymbol()->clone() );
  symbol->startRender( context );

  //get max bleed from symbol
  double maxBleedPixels = QgsSymbolLayerUtils::estimateMaxSymbolBleed( symbol.get(), context );

  //Now subtract 1 pixel to prevent semi-transparent borders at edge of solid page caused by
  //anti-aliased painting. This may cause a pixel to be cropped from certain edge lines/symbols,
  //but that can be counteracted by adding a dummy transparent line symbol layer with a wider line width
  maxBleedPixels--;

  QPolygonF pagePolygon = QPolygonF( QRectF( maxBleedPixels, maxBleedPixels,
                                     ( rect().width() * scale - 2 * maxBleedPixels ), ( rect().height() * scale - 2 * maxBleedPixels ) ) );
  QList<QPolygonF> rings; //empty list

  symbol->renderPolygon( pagePolygon, &rings, nullptr, context );
  symbol->stopRender( context );

  painter->restore();
}
