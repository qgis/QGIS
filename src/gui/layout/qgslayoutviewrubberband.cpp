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
#include <QGraphicsPolygonItem>

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

QPen QgsLayoutViewRubberBand::pen() const
{
  return mPen;
}

void QgsLayoutViewRubberBand::setPen( const QPen &pen )
{
  mPen = pen;
}

QBrush QgsLayoutViewRubberBand::brush() const
{
  return mBrush;
}

void QgsLayoutViewRubberBand::setBrush( const QBrush &brush )
{
  mBrush = brush;
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
  mRubberBandItem->setBrush( brush() );
  mRubberBandItem->setPen( pen() );
  mRubberBandStartPos = position;
  t.translate( position.x(), position.y() );
  mRubberBandItem->setTransform( t );
  mRubberBandItem->setZValue( QgsLayout::ZViewTool );
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

  emit sizeChanged( tr( "width: %1 %3 height: %2 %3" ).arg( newRect.width() ).arg( newRect.height() ).arg( QgsUnitTypes::toAbbreviatedString( layout()->units() ) ) );
}

QRectF QgsLayoutViewRectangularRubberBand::finish( QPointF position, Qt::KeyboardModifiers modifiers )
{
  bool constrainSquare = modifiers & Qt::ShiftModifier;
  bool fromCenter = modifiers & Qt::AltModifier;

  if ( mRubberBandItem )
  {
    layout()->removeItem( mRubberBandItem );
    delete mRubberBandItem;
    mRubberBandItem = nullptr;
  }
  return updateRect( mRubberBandStartPos, position, constrainSquare, fromCenter );
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
  mRubberBandItem->setBrush( brush() );
  mRubberBandItem->setPen( pen() );
  mRubberBandStartPos = position;
  t.translate( position.x(), position.y() );
  mRubberBandItem->setTransform( t );
  mRubberBandItem->setZValue( QgsLayout::ZViewTool );
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

  emit sizeChanged( tr( "width: %1 %3 height: %2 %3" ).arg( newRect.width() ).arg( newRect.height() ).arg( QgsUnitTypes::toAbbreviatedString( layout()->units() ) ) );
}

QRectF QgsLayoutViewEllipticalRubberBand::finish( QPointF position, Qt::KeyboardModifiers modifiers )
{
  bool constrainSquare = modifiers & Qt::ShiftModifier;
  bool fromCenter = modifiers & Qt::AltModifier;

  if ( mRubberBandItem )
  {
    layout()->removeItem( mRubberBandItem );
    delete mRubberBandItem;
    mRubberBandItem = nullptr;
  }
  return updateRect( mRubberBandStartPos, position, constrainSquare, fromCenter );
}

//
// QgsLayoutViewTriangleRubberBand
//

QgsLayoutViewTriangleRubberBand::QgsLayoutViewTriangleRubberBand( QgsLayoutView *view )
  : QgsLayoutViewRubberBand( view )
{

}

QgsLayoutViewTriangleRubberBand *QgsLayoutViewTriangleRubberBand::create( QgsLayoutView *view ) const
{
  return new QgsLayoutViewTriangleRubberBand( view );
}

QgsLayoutViewTriangleRubberBand::~QgsLayoutViewTriangleRubberBand()
{
  if ( mRubberBandItem )
  {
    layout()->removeItem( mRubberBandItem );
    delete mRubberBandItem;
  }
}

void QgsLayoutViewTriangleRubberBand::start( QPointF position, Qt::KeyboardModifiers )
{
  QTransform t;
  mRubberBandItem = new QGraphicsPolygonItem();
  mRubberBandItem->setBrush( brush() );
  mRubberBandItem->setPen( pen() );
  mRubberBandStartPos = position;
  t.translate( position.x(), position.y() );
  mRubberBandItem->setTransform( t );
  mRubberBandItem->setZValue( QgsLayout::ZViewTool );
  layout()->addItem( mRubberBandItem );
  layout()->update();
}

void QgsLayoutViewTriangleRubberBand::update( QPointF position, Qt::KeyboardModifiers modifiers )
{
  if ( !mRubberBandItem )
  {
    return;
  }

  bool constrainSquare = modifiers & Qt::ShiftModifier;
  bool fromCenter = modifiers & Qt::AltModifier;

  QRectF newRect = updateRect( mRubberBandStartPos, position, constrainSquare, fromCenter );

  QPolygonF shapePolygon = QPolygonF() << QPointF( 0, newRect.height() )
                           << QPointF( newRect.width(), newRect.height() )
                           << QPointF( newRect.width() / 2.0, 0 )
                           << QPointF( 0, newRect.height() );

  mRubberBandItem->setPolygon( shapePolygon );
  QTransform t;
  t.translate( newRect.x(), newRect.y() );
  mRubberBandItem->setTransform( t );

  emit sizeChanged( tr( "width: %1 %3 height: %2 %3" ).arg( newRect.width() ).arg( newRect.height() ).arg( QgsUnitTypes::toAbbreviatedString( layout()->units() ) ) );
}

QRectF QgsLayoutViewTriangleRubberBand::finish( QPointF position, Qt::KeyboardModifiers modifiers )
{
  bool constrainSquare = modifiers & Qt::ShiftModifier;
  bool fromCenter = modifiers & Qt::AltModifier;

  if ( mRubberBandItem )
  {
    layout()->removeItem( mRubberBandItem );
    delete mRubberBandItem;
    mRubberBandItem = nullptr;
  }
  return updateRect( mRubberBandStartPos, position, constrainSquare, fromCenter );
}
