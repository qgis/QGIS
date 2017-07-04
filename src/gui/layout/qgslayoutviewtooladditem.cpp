/***************************************************************************
                             qgslayoutviewtooladditem.cpp
                             ----------------------------
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

#include "qgslayoutviewtooladditem.h"
#include "qgsapplication.h"
#include "qgscursors.h"
#include "qgslayoutview.h"
#include "qgslayout.h"
#include "qgslayoutitemregistry.h"
#include "qgslayoutviewmouseevent.h"
#include "qgslogger.h"
#include <QGraphicsRectItem>
#include <QPen>
#include <QBrush>
#include <QMouseEvent>

QgsLayoutViewToolAddItem::QgsLayoutViewToolAddItem( QgsLayoutView *view )
  : QgsLayoutViewTool( view, tr( "Add item" ) )
{
  QPixmap crosshairQPixmap = QPixmap( ( const char ** )( cross_hair_cursor ) );
  setCursor( QCursor( crosshairQPixmap, 8, 8 ) );
}

void QgsLayoutViewToolAddItem::setItemType( int itemType )
{
  mItemType = itemType;
}

void QgsLayoutViewToolAddItem::layoutPressEvent( QgsLayoutViewMouseEvent *event )
{
  QTransform t;
  mRubberBandItem = new QGraphicsRectItem( 0, 0, 0, 0 );
  mRubberBandItem->setBrush( Qt::NoBrush );
  mRubberBandItem->setPen( QPen( QBrush( QColor( 227, 22, 22, 200 ) ), 0 ) );
  mMousePressStartPos = event->pos();
  mRubberBandStartPos = QPointF( event->layoutPoint().x(), event->layoutPoint().y() );
  t.translate( event->x(), event->y() );
  mRubberBandItem->setTransform( t );
  mRubberBandItem->setZValue( QgsLayout::ZMapTool );
  layout()->addItem( mRubberBandItem );
  layout()->update();
}

void QgsLayoutViewToolAddItem::layoutMoveEvent( QgsLayoutViewMouseEvent *event )
{
  updateRubberBandRect( event->layoutPoint(), event->modifiers() & Qt::ShiftModifier, event->modifiers() & Qt::AltModifier );
}

void QgsLayoutViewToolAddItem::layoutReleaseEvent( QgsLayoutViewMouseEvent *event )
{
  if ( event->button() != Qt::LeftButton )
  {
    return;
  }

  // click? or click-and-drag?
  QPoint mousePressStopPoint = event->pos();
  int diffX = mousePressStopPoint.x() - mMousePressStartPos.x();
  int diffY = mousePressStopPoint.y() - mMousePressStartPos.y();
  bool clickOnly = false;
  if ( qAbs( diffX ) < 2 && qAbs( diffY ) < 2 )
  {
    clickOnly = true;
  }

  if ( mRubberBandItem )
  {
    layout()->removeItem( mRubberBandItem );
    delete mRubberBandItem;
    mRubberBandItem = nullptr;
  }

  QgsLogger::debug( QStringLiteral( "creating new %1 item  " ).arg( QgsApplication::layoutItemRegistry()->itemMetadata( mItemType )->visibleName() ) );
}

void QgsLayoutViewToolAddItem::updateRubberBandRect( QPointF pos, const bool constrainSquare, const bool fromCenter )
{
  if ( !mRubberBandItem )
  {
    return;
  }

  double x = 0;
  double y = 0;
  double width = 0;
  double height = 0;

  double dx = pos.x() - mRubberBandStartPos.x();
  double dy = pos.y() - mRubberBandStartPos.y();

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

    x = mRubberBandStartPos.x() - ( ( dx < 0 ) ? width : 0 );
    y = mRubberBandStartPos.y() - ( ( dy < 0 ) ? height : 0 );
  }
  else
  {
    //not constraining
    if ( dx < 0 )
    {
      x = pos.x();
      width = -dx;
    }
    else
    {
      x = mRubberBandStartPos.x();
      width = dx;
    }

    if ( dy < 0 )
    {
      y = pos.y();
      height = -dy;
    }
    else
    {
      y = mRubberBandStartPos.y();
      height = dy;
    }
  }

  if ( fromCenter )
  {
    x = mRubberBandStartPos.x() - width;
    y = mRubberBandStartPos.y() - height;
    width *= 2.0;
    height *= 2.0;
  }

  mRubberBandItem->setRect( 0, 0, width, height );
  QTransform t;
  t.translate( x, y );
  mRubberBandItem->setTransform( t );
}

int QgsLayoutViewToolAddItem::itemType() const
{
  return mItemType;
}
