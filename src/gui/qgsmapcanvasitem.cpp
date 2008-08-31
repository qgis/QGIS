/***************************************************************************
    qgsmapcanvasitem.h  - base class for map canvas items
    ----------------------
    begin                : February 2006
    copyright            : (C) 2006 by Martin Dobias
    email                : wonder.sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
/* $Id$ */


#include "qgsmapcanvasitem.h"
#include "qgsmapcanvas.h"
#include "qgsmaptopixel.h"
#include <QGraphicsScene>
#include <QRect>
#include <QPen>
#include <QBrush>
#include <QPainter>
#include "qgslogger.h"

QgsMapCanvasItem::QgsMapCanvasItem( QgsMapCanvas* mapCanvas )
    : QGraphicsItem( 0, mapCanvas->scene() ), mMapCanvas( mapCanvas ),
    mPanningOffset( 0, 0 ), mItemSize( 0, 0 )
{
}

QgsMapCanvasItem::~QgsMapCanvasItem()
{
  update(); // shedule redraw of canvas
}

void QgsMapCanvasItem::paint( QPainter * painter,
                              const QStyleOptionGraphicsItem * option,
                              QWidget * widget )
{
  paint( painter ); // call the derived item's drawing routines
}

QgsPoint QgsMapCanvasItem::toMapCoordinates( const QPoint& point )
{
  return mMapCanvas->getCoordinateTransform()->toMapCoordinates( point - mPanningOffset );
}


QPointF QgsMapCanvasItem::toCanvasCoordinates( const QgsPoint& point )
{
  double x = point.x(), y = point.y();
  mMapCanvas->getCoordinateTransform()->transformInPlace( x, y );
  return QPointF( x, y ) + mPanningOffset;
}


QgsRect QgsMapCanvasItem::rect() const
{
  return mRect;
}


void QgsMapCanvasItem::setRect( const QgsRect& rect )
{
  mRect = rect;
  //updatePosition();

  QRectF r; // empty rect by default
  if ( !mRect.isEmpty() )
  {
    r.setTopLeft( toCanvasCoordinates( QgsPoint( mRect.xMin(), mRect.yMin() ) ) );
    r.setBottomRight( toCanvasCoordinates( QgsPoint( mRect.xMax(), mRect.yMax() ) ) );
    r = r.normalized();
  }

  // update the point prior to changing its position
  update();

  // set position in canvas where the item will have coordinate (0,0)
  setPos( r.topLeft() );
  mItemSize = QSizeF( r.width() + 2, r.height() + 2 );

  // QgsDebugMsg(QString("[%1,%2]-[%3x%4]").arg((int) r.left()).arg((int) r.top()).arg((int) r.width()).arg((int) r.height()));

  update();
}

QRectF QgsMapCanvasItem::boundingRect() const
{
  return QRectF( QPointF( -1, -1 ), mItemSize );
}


void QgsMapCanvasItem::updateCanvas()
{
  update();
  // porting: update below should not be needed anymore
  //mMapCanvas->scene()->update(); //Contents();
}

void QgsMapCanvasItem::updatePosition()
{
  // default implementation: recalculate position of the item
  setRect( mRect );
}


void QgsMapCanvasItem::setPanningOffset( const QPoint& point )
{
  mPanningOffset = point;
}
