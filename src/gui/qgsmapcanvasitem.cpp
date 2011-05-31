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


#include "qgsmapcanvasitem.h"
#include "qgsmapcanvas.h"
#include "qgsmaprenderer.h"
#include "qgsmaptopixel.h"
#include "qgsrendercontext.h"
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
  update(); // schedule redraw of canvas
}

void QgsMapCanvasItem::paint( QPainter * painter,
                              const QStyleOptionGraphicsItem * option,
                              QWidget * widget )
{
  if ( mMapCanvas->antiAliasingEnabled() )
  {
    painter->setRenderHint( QPainter::Antialiasing );
  }
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


QgsRectangle QgsMapCanvasItem::rect() const
{
  return mRect;
}


void QgsMapCanvasItem::setRect( const QgsRectangle& rect )
{
  mRect = rect;
  //updatePosition();

  QRectF r; // empty rect by default
  if ( !mRect.isEmpty() )
  {
    r.setTopLeft( toCanvasCoordinates( QgsPoint( mRect.xMinimum(), mRect.yMinimum() ) ) );
    r.setBottomRight( toCanvasCoordinates( QgsPoint( mRect.xMaximum(), mRect.yMaximum() ) ) );
    r = r.normalized();
  }

  // set position in canvas where the item will have coordinate (0,0)
  prepareGeometryChange();
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

bool QgsMapCanvasItem::setRenderContextVariables( QPainter* p, QgsRenderContext& context ) const
{
  if ( !mMapCanvas || !p )
  {
    return false;
  }
  QgsMapRenderer* mapRenderer = mMapCanvas->mapRenderer();
  if ( !mapRenderer )
  {
    return false;
  }

  context.setPainter( p );
  context.setRendererScale( mMapCanvas->scale() );

  int dpi = mapRenderer->outputDpi();
  int painterDpi = p->device()->logicalDpiX();
  double scaleFactor = 1.0;
  double rasterScaleFactor = 1.0;

  //little trick to find out if painting origines from composer or main map canvas
  if ( data( 1 ).toString() == "composer" )
  {
    rasterScaleFactor = painterDpi / 25.4;
    scaleFactor = dpi / 25.4;
  }
  else
  {
    if ( mapRenderer->outputUnits() == QgsMapRenderer::Millimeters )
    {
      scaleFactor = dpi / 25.4;
    }
  }
  context.setScaleFactor( scaleFactor );
  context.setRasterScaleFactor( rasterScaleFactor );
  return true;
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
