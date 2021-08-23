/***************************************************************************
                             qgsmodelviewrubberband.cpp
                             ---------------------------
    Date                 : March 2020
    Copyright            : (C) 2020 Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include "qgsmodelviewrubberband.h"
#include "qgsmodelgraphicsview.h"
#include "qgsmodelgraphicsscene.h"
#include <QGraphicsRectItem>

QgsModelViewRubberBand::QgsModelViewRubberBand( QgsModelGraphicsView *view )
  : mView( view )
{

}

QgsModelGraphicsView *QgsModelViewRubberBand::view() const
{
  return mView;
}

QRectF QgsModelViewRubberBand::updateRect( QPointF start, QPointF position, bool constrainSquare, bool fromCenter )
{
  double x = 0;
  double y = 0;
  double width = 0;
  double height = 0;

  const double dx = position.x() - start.x();
  const double dy = position.y() - start.y();

  if ( constrainSquare )
  {
    if ( std::fabs( dx ) > std::fabs( dy ) )
    {
      width = std::fabs( dx );
      height = width;
    }
    else
    {
      height = std::fabs( dy );
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

QPen QgsModelViewRubberBand::pen() const
{
  return mPen;
}

void QgsModelViewRubberBand::setPen( const QPen &pen )
{
  mPen = pen;
}

QBrush QgsModelViewRubberBand::brush() const
{
  return mBrush;
}

void QgsModelViewRubberBand::setBrush( const QBrush &brush )
{
  mBrush = brush;
}


QgsModelViewRectangularRubberBand::QgsModelViewRectangularRubberBand( QgsModelGraphicsView *view )
  : QgsModelViewRubberBand( view )
{
}

QgsModelViewRectangularRubberBand *QgsModelViewRectangularRubberBand::create( QgsModelGraphicsView *view ) const
{
  return new QgsModelViewRectangularRubberBand( view );
}

QgsModelViewRectangularRubberBand::~QgsModelViewRectangularRubberBand()
{
  if ( mRubberBandItem )
  {
    view()->scene()->removeItem( mRubberBandItem );
    delete mRubberBandItem;
  }
}

void QgsModelViewRectangularRubberBand::start( QPointF position, Qt::KeyboardModifiers )
{
  QTransform t;
  mRubberBandItem = new QGraphicsRectItem( 0, 0, 0, 0 );
  mRubberBandItem->setBrush( brush() );
  mRubberBandItem->setPen( pen() );
  mRubberBandStartPos = position;
  t.translate( position.x(), position.y() );
  mRubberBandItem->setTransform( t );
  mRubberBandItem->setZValue( QgsModelGraphicsScene::RubberBand );
  view()->scene()->addItem( mRubberBandItem );
  view()->scene()->update();
}

void QgsModelViewRectangularRubberBand::update( QPointF position, Qt::KeyboardModifiers modifiers )
{
  if ( !mRubberBandItem )
  {
    return;
  }

  const bool constrainSquare = modifiers & Qt::ShiftModifier;
  const bool fromCenter = modifiers & Qt::AltModifier;

  const QRectF newRect = updateRect( mRubberBandStartPos, position, constrainSquare, fromCenter );
  mRubberBandItem->setRect( 0, 0, newRect.width(), newRect.height() );
  QTransform t;
  t.translate( newRect.x(), newRect.y() );
  mRubberBandItem->setTransform( t );
}

QRectF QgsModelViewRectangularRubberBand::finish( QPointF position, Qt::KeyboardModifiers modifiers )
{
  const bool constrainSquare = modifiers & Qt::ShiftModifier;
  const bool fromCenter = modifiers & Qt::AltModifier;

  if ( mRubberBandItem )
  {
    view()->scene()->removeItem( mRubberBandItem );
    delete mRubberBandItem;
    mRubberBandItem = nullptr;
  }
  return updateRect( mRubberBandStartPos, position, constrainSquare, fromCenter );
}
