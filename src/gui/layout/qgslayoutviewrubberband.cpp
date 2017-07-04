/***************************************************************************
                             qgslayoutviewrubberband.cpp
                             ---------------------------
    Date                 : July 2017
    Copyright            : (C) 2017 Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include "qgslayoutviewrubberband.h"
#include "qgslayout.h"
#include "qgslayoutview.h"
#include <QGraphicsRectItem>
#include <QGraphicsEllipseItem>

QgsLayoutViewRubberBand::QgsLayoutViewRubberBand( QgsLayoutView *view )
  : mView( view )
{

}

QgsLayoutView *QgsLayoutViewRubberBand::view() const
{
  return mView;
}

QgsLayout *QgsLayoutViewRubberBand::layout() const
{
  return mView->currentLayout();
}

QRectF QgsLayoutViewRubberBand::updateRect( QPointF start, QPointF position, bool constrainSquare, bool fromCenter )
{
  double x = 0;
  double y = 0;
  double width = 0;
  double height = 0;

  double dx = position.x() - start.x();
  double dy = position.y() - start.y();

  if ( constrainSquare )
  {
    if ( fabs( dx ) > fabs( dy ) )
    {
      width = fabs( dx );
      height = width;
    }
    else
    {
      height = fabs( dy );
      width = height;
    }

    x = start.x() - ( ( dx < 0 ) ? width : 0 );
    y = start.y() - ( ( dy < 0 ) ? height : 0 );
  }
  else
  {
    //not constraining
    if ( dx < 0 )
    {
      x = position.x();
      width = -dx;
    }
    else
    {
      x = start.x();
      width = dx;
    }

    if ( dy < 0 )
    {
      y = position.y();
      height = -dy;
    }
    else
    {
      y = start.y();
      height = dy;
    }
  }

  if ( fromCenter )
  {
    x = start.x() - width;
    y = start.y() - height;
    width *= 2.0;
    height *= 2.0;
  }

  return QRectF( x, y, width, height );
}


QgsLayoutViewRectangularRubberBand::QgsLayoutViewRectangularRubberBand( QgsLayoutView *view )
  : QgsLayoutViewRubberBand( view )
{
}

QgsLayoutViewRectangularRubberBand *QgsLayoutViewRectangularRubberBand::create( QgsLayoutView *view ) const
{
  return new QgsLayoutViewRectangularRubberBand( view );
}

QgsLayoutViewRectangularRubberBand::~QgsLayoutViewRectangularRubberBand()
{
  if ( mRubberBandItem )
  {
    layout()->removeItem( mRubberBandItem );
    delete mRubberBandItem;
  }
}

void QgsLayoutViewRectangularRubberBand::start( QPointF position, Qt::KeyboardModifiers )
{
  QTransform t;
  mRubberBandItem = new QGraphicsRectItem( 0, 0, 0, 0 );
  mRubberBandItem->setBrush( Qt::NoBrush );
  mRubberBandItem->setPen( QPen( QBrush( QColor( 227, 22, 22, 200 ) ), 0 ) );
  mRubberBandStartPos = position;
  t.translate( position.x(), position.y() );
  mRubberBandItem->setTransform( t );
  mRubberBandItem->setZValue( QgsLayout::ZMapTool );
  layout()->addItem( mRubberBandItem );
  layout()->update();
}

void QgsLayoutViewRectangularRubberBand::update( QPointF position, Qt::KeyboardModifiers modifiers )
{
  if ( !mRubberBandItem )
  {
    return;
  }

  bool constrainSquare = modifiers & Qt::ShiftModifier;
  bool fromCenter = modifiers & Qt::AltModifier;

  QRectF newRect = updateRect( mRubberBandStartPos, position, constrainSquare, fromCenter );
  mRubberBandItem->setRect( 0, 0, newRect.width(), newRect.height() );
  QTransform t;
  t.translate( newRect.x(), newRect.y() );
  mRubberBandItem->setTransform( t );
}

void QgsLayoutViewRectangularRubberBand::finish( QPointF, Qt::KeyboardModifiers )
{
  if ( mRubberBandItem )
  {
    layout()->removeItem( mRubberBandItem );
    delete mRubberBandItem;
    mRubberBandItem = nullptr;
  }
}

QgsLayoutViewEllipticalRubberBand::QgsLayoutViewEllipticalRubberBand( QgsLayoutView *view )
  : QgsLayoutViewRubberBand( view )
{

}

QgsLayoutViewEllipticalRubberBand *QgsLayoutViewEllipticalRubberBand::create( QgsLayoutView *view ) const
{
  return new QgsLayoutViewEllipticalRubberBand( view );
}

QgsLayoutViewEllipticalRubberBand::~QgsLayoutViewEllipticalRubberBand()
{
  if ( mRubberBandItem )
  {
    layout()->removeItem( mRubberBandItem );
    delete mRubberBandItem;
  }
}

void QgsLayoutViewEllipticalRubberBand::start( QPointF position, Qt::KeyboardModifiers )
{
  QTransform t;
  mRubberBandItem = new QGraphicsEllipseItem( 0, 0, 0, 0 );
  mRubberBandItem->setBrush( Qt::NoBrush );
  mRubberBandItem->setPen( QPen( QBrush( QColor( 227, 22, 22, 200 ) ), 0 ) );
  mRubberBandStartPos = position;
  t.translate( position.x(), position.y() );
  mRubberBandItem->setTransform( t );
  mRubberBandItem->setZValue( QgsLayout::ZMapTool );
  layout()->addItem( mRubberBandItem );
  layout()->update();
}

void QgsLayoutViewEllipticalRubberBand::update( QPointF position, Qt::KeyboardModifiers modifiers )
{
  if ( !mRubberBandItem )
  {
    return;
  }

  bool constrainSquare = modifiers & Qt::ShiftModifier;
  bool fromCenter = modifiers & Qt::AltModifier;

  QRectF newRect = updateRect( mRubberBandStartPos, position, constrainSquare, fromCenter );
  mRubberBandItem->setRect( 0, 0, newRect.width(), newRect.height() );
  QTransform t;
  t.translate( newRect.x(), newRect.y() );
  mRubberBandItem->setTransform( t );
}

void QgsLayoutViewEllipticalRubberBand::finish( QPointF, Qt::KeyboardModifiers )
{
  if ( mRubberBandItem )
  {
    layout()->removeItem( mRubberBandItem );
    delete mRubberBandItem;
    mRubberBandItem = nullptr;
  }
}
