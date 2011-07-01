/***************************************************************************
                         qgscomposeritemgroup.cpp
                         ------------------------
    begin                : 2nd June 2008
    copyright            : (C) 2008 by Marco Hugentobler
    email                : marco dot hugentobler at karto dot baug dot ethz dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgscomposeritemgroup.h"
#include "qgscomposition.h"
#include <QPen>
#include <QPainter>

QgsComposerItemGroup::QgsComposerItemGroup( QgsComposition* c ): QgsComposerItem( c )
{
  setZValue( 90 );
  show();
}

QgsComposerItemGroup::~QgsComposerItemGroup()
{
  QSet<QgsComposerItem*>::iterator itemIt = mItems.begin();
  for ( ; itemIt != mItems.end(); ++itemIt )
  {
    if ( *itemIt )
    {
      mComposition->removeItem( *itemIt );
      ( *itemIt )->setFlag( QGraphicsItem::ItemIsSelectable, true );
    }
  }
}

void QgsComposerItemGroup::addItem( QgsComposerItem* item )
{
  if ( !item )
  {
    return;
  }

  if ( mItems.contains( item ) )
  {
    return;
  }
  mItems.insert( item );
  item->setSelected( false );
  item->setFlag( QGraphicsItem::ItemIsSelectable, false ); //item in groups cannot be selected

  //update extent (which is in scene coordinates)
  double minXItem = item->transform().dx();
  double minYItem = item->transform().dy();
  double maxXItem = minXItem + item->rect().width();
  double maxYItem = minYItem + item->rect().height();

  if ( mSceneBoundingRectangle.isEmpty() ) //we add the first item
  {
    mSceneBoundingRectangle.setLeft( minXItem );
    mSceneBoundingRectangle.setTop( minYItem );
    mSceneBoundingRectangle.setRight( maxXItem );
    mSceneBoundingRectangle.setBottom( maxYItem );
  }

  else
  {
    if ( minXItem < mSceneBoundingRectangle.left() )
    {
      mSceneBoundingRectangle.setLeft( minXItem );
    }
    if ( minYItem < mSceneBoundingRectangle.top() )
    {
      mSceneBoundingRectangle.setTop( minYItem );
    }
    if ( maxXItem > mSceneBoundingRectangle.right() )
    {
      mSceneBoundingRectangle.setRight( maxXItem );
    }
    if ( maxYItem > mSceneBoundingRectangle.bottom() )
    {
      mSceneBoundingRectangle.setBottom( maxYItem );
    }
  }

  QgsComposerItem::setSceneRect( mSceneBoundingRectangle ); //call method of superclass to avoid repositioning of items

}

void QgsComposerItemGroup::removeItems()
{
  QSet<QgsComposerItem*>::iterator item_it = mItems.begin();
  for ( ; item_it != mItems.end(); ++item_it )
  {
    ( *item_it )->setFlag( QGraphicsItem::ItemIsSelectable, true ); //enable item selection again
    ( *item_it )->setSelected( true );
  }
  mItems.clear();
}

void QgsComposerItemGroup::paint( QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget )
{
  Q_UNUSED( option );
  Q_UNUSED( widget );
  drawFrame( painter );
  if ( isSelected() )
  {
    drawSelectionBoxes( painter );
  }
}

void QgsComposerItemGroup::setSceneRect( const QRectF& rectangle )
{
  //calculate values between 0 and 1 for boundaries of all contained items, depending on their positions in the item group rectangle.
  //then position the item boundaries in the new item group rect such that these values are the same
  double xLeftCurrent = transform().dx();
  double xRightCurrent = xLeftCurrent + rect().width();
  double yTopCurrent = transform().dy();
  double yBottomCurrent = yTopCurrent + rect().height();

  double xItemLeft, xItemRight, yItemTop, yItemBottom;
  double xItemLeftNew, xItemRightNew, yItemTopNew, yItemBottomNew;
  double xParamLeft, xParamRight, yParamTop, yParamBottom;


  QSet<QgsComposerItem*>::iterator item_it = mItems.begin();
  for ( ; item_it != mItems.end(); ++item_it )
  {
    xItemLeft = ( *item_it )->transform().dx();
    xItemRight = xItemLeft + ( *item_it )->rect().width();
    yItemTop = ( *item_it )->transform().dy();
    yItemBottom = yItemTop + ( *item_it )->rect().height();

    xParamLeft = ( xItemLeft - xLeftCurrent ) / ( xRightCurrent - xLeftCurrent );
    xParamRight = ( xItemRight - xLeftCurrent ) / ( xRightCurrent - xLeftCurrent );
    yParamTop = ( yItemTop - yTopCurrent ) / ( yBottomCurrent - yTopCurrent );
    yParamBottom = ( yItemBottom - yTopCurrent ) / ( yBottomCurrent - yTopCurrent );

    xItemLeftNew = xParamLeft * rectangle.right()  + ( 1 - xParamLeft ) * rectangle.left();
    xItemRightNew = xParamRight * rectangle.right() + ( 1 - xParamRight ) * rectangle.left();
    yItemTopNew = yParamTop * rectangle.bottom() + ( 1 - yParamTop ) * rectangle.top();
    yItemBottomNew = yParamBottom * rectangle.bottom() + ( 1 - yParamBottom ) * rectangle.top();

    ( *item_it )->setSceneRect( QRectF( xItemLeftNew, yItemTopNew, xItemRightNew - xItemLeftNew, yItemBottomNew - yItemTopNew ) );
  }
  QgsComposerItem::setSceneRect( rectangle );
}

void QgsComposerItemGroup::drawFrame( QPainter* p )
{
  if ( !mComposition )
  {
    return;
  }

  if ( mFrame && mComposition->plotStyle() == QgsComposition::Preview )
  {
    QPen newPen( pen() );
    newPen.setStyle( Qt::DashLine );
    newPen.setColor( QColor( 128, 128, 128, 128 ) );
    p->setPen( newPen );
    p->setRenderHint( QPainter::Antialiasing, true );
    p->drawRect( QRectF( 0, 0, rect().width(), rect().height() ) );
  }
}
