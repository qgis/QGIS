/***************************************************************************
                             qgslayoutguidecollection.cpp
                             ----------------------------
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

#include "qgslayoutguidecollection.h"
#include "qgslayout.h"
#include <QGraphicsLineItem>


//
// QgsLayoutGuide
//

QgsLayoutGuide::QgsLayoutGuide( QgsLayout *layout, Orientation orientation, const QgsLayoutMeasurement &position )
  : QObject( layout )
  , mOrientation( orientation )
  , mPosition( position )
  , mLayout( layout )
  , mLineItem( new QGraphicsLineItem() )
{
  mLineItem->hide();
  mLineItem->setZValue( QgsLayout::ZGuide );
  mLayout->addItem( mLineItem.get() );

  QPen linePen( Qt::SolidLine );
  linePen.setColor( Qt::red );
  // use a pen width of 0, since this activates a cosmetic pen
  // which doesn't scale with the composer and keeps a constant size
  linePen.setWidthF( 0 );
  mLineItem->setPen( linePen );
}

QgsLayoutMeasurement QgsLayoutGuide::position() const
{
  return mPosition;
}

void QgsLayoutGuide::setPosition( const QgsLayoutMeasurement &position )
{
  mPosition = position;
  update();
  emit positionChanged();
}

int QgsLayoutGuide::page() const
{
  return mPage;
}

void QgsLayoutGuide::setPage( int page )
{
  mPage = page;
  update();
}

void QgsLayoutGuide::update()
{
  // first find matching page
  if ( mPage >= mLayout->pageCollection()->pageCount() )
  {
    mLineItem->hide();
    return;
  }

  QgsLayoutItemPage *page = mLayout->pageCollection()->page( mPage );
  mLineItem->setParentItem( page );
  double layoutPos = mLayout->convertToLayoutUnits( mPosition );
  switch ( mOrientation )
  {
    case Horizontal:
      if ( layoutPos > page->rect().width() )
      {
        mLineItem->hide();
      }
      else
      {
        mLineItem->setLine( 0, layoutPos, page->rect().width(), layoutPos );
        mLineItem->show();
      }

      break;

    case Vertical:
      if ( layoutPos > page->rect().height() )
      {
        mLineItem->hide();
      }
      else
      {
        mLineItem->setLine( layoutPos, 0, layoutPos, page->rect().height() );
        mLineItem->show();
      }

      break;
  }
}

QGraphicsLineItem *QgsLayoutGuide::item()
{
  return mLineItem.get();
}

double QgsLayoutGuide::layoutPosition() const
{
  switch ( mOrientation )
  {
    case Horizontal:
      return mLineItem->line().y1();

    case Vertical:
      return mLineItem->line().x1();
  }
  return -999; // avoid warning
}

QgsLayoutGuide::Orientation QgsLayoutGuide::orientation() const
{
  return mOrientation;
}
