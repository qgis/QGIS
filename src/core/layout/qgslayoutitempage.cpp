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
#include <QPainter>

QgsLayoutItemPage::QgsLayoutItemPage( QgsLayout *layout )
  : QgsLayoutItem( layout )
{

}

void QgsLayoutItemPage::draw( QgsRenderContext &context, const QStyleOptionGraphicsItem *itemStyle )
{
#if 0
  Q_UNUSED( itemStyle );
  Q_UNUSED( pWidget );
  if ( !painter || !mComposition || !mComposition->pagesVisible() )
  {
    return;
  }

  //setup painter scaling to dots so that raster symbology is drawn to scale
  double dotsPerMM = painter->device()->logicalDpiX() / 25.4;

  //setup render context
  QgsRenderContext context = QgsComposerUtils::createRenderContextForComposition( mComposition, painter );
  context.setForceVectorOutput( true );

  QgsExpressionContext expressionContext = createExpressionContext();
  context.setExpressionContext( expressionContext );

  painter->save();

  if ( mComposition->plotStyle() ==  QgsComposition::Preview )
  {
    //if in preview mode, draw page border and shadow so that it's
    //still possible to tell where pages with a transparent style begin and end
    painter->setRenderHint( QPainter::Antialiasing, false );

    //shadow
    painter->setBrush( QBrush( QColor( 150, 150, 150 ) ) );
    painter->setPen( Qt::NoPen );
    painter->drawRect( QRectF( 1, 1, rect().width() + 1, rect().height() + 1 ) );

    //page area
    painter->setBrush( QColor( 215, 215, 215 ) );
    QPen pagePen = QPen( QColor( 100, 100, 100 ), 0 );
    pagePen.setCosmetic( true );
    painter->setPen( pagePen );
    painter->drawRect( QRectF( 0, 0, rect().width(), rect().height() ) );
  }

  painter->scale( 1 / dotsPerMM, 1 / dotsPerMM ); // scale painter from mm to dots

  painter->setRenderHint( QPainter::Antialiasing );
  mComposition->pageStyleSymbol()->startRender( context );

  calculatePageMargin();
  QPolygonF pagePolygon = QPolygonF( QRectF( mPageMargin * dotsPerMM, mPageMargin * dotsPerMM,
                                     ( rect().width() - 2 * mPageMargin ) * dotsPerMM, ( rect().height() - 2 * mPageMargin ) * dotsPerMM ) );
  QList<QPolygonF> rings; //empty list

  mComposition->pageStyleSymbol()->renderPolygon( pagePolygon, &rings, nullptr, context );
  mComposition->pageStyleSymbol()->stopRender( context );
  painter->restore();
#endif
}
