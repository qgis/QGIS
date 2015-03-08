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
    : QGraphicsItem()
    , mMapCanvas( mapCanvas )
    , mRectRotation( 0.0 )
    , mPanningOffset( 0, 0 )
    , mItemSize( 0, 0 )
{
  Q_ASSERT( mapCanvas && mapCanvas->scene() );
  mapCanvas->scene()->addItem( this );
}

QgsMapCanvasItem::~QgsMapCanvasItem()
{
  update(); // schedule redraw of canvas
}

void QgsMapCanvasItem::paint( QPainter * painter,
                              const QStyleOptionGraphicsItem * option,
                              QWidget * widget )
{
  Q_UNUSED( option );
  Q_UNUSED( widget );
  if ( mMapCanvas->antiAliasingEnabled() )
  {
    painter->setRenderHint( QPainter::Antialiasing );
  }
  paint( painter ); // call the derived item's drawing routines
}

QgsPoint QgsMapCanvasItem::toMapCoordinates( const QPoint& point ) const
{
  return mMapCanvas->getCoordinateTransform()->toMapCoordinates( point - mPanningOffset );
}


QPointF QgsMapCanvasItem::toCanvasCoordinates( const QgsPoint& point ) const
{
  qreal x = point.x(), y = point.y();
  mMapCanvas->getCoordinateTransform()->transformInPlace( x, y );
  return QPointF( x, y ) + mPanningOffset;
}

QgsRectangle QgsMapCanvasItem::rect() const
{
  return mRect;
}


void QgsMapCanvasItem::setRect( const QgsRectangle& rect, bool resetRotation )
{
  mRect = rect;
  //updatePosition();

  QRectF r; // empty rect by default
  if ( !mRect.isEmpty() )
  {
    // rect encodes origin of the item (xMin,yMax from map to canvas units)
    // and size (rect size / map units per pixel)
    r.setTopLeft( toCanvasCoordinates( QPointF( mRect.xMinimum(), mRect.yMaximum() ) ) );
    const QgsMapToPixel* m2p = mMapCanvas->getCoordinateTransform();
    double res = m2p->mapUnitsPerPixel();
    r.setSize( QSizeF( mRect.width() / res, mRect.height() / res ) );
  }

  // set position in canvas where the item will have coordinate (0,0)
  prepareGeometryChange();
  setPos( r.topLeft() );
  mItemSize = QSizeF( r.width() + 2, r.height() + 2 );

  if ( resetRotation )
  {
    mRectRotation = mMapCanvas->rotation();
    setRotation( 0 );
  }

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
  const QgsMapSettings& ms = mMapCanvas->mapSettings();

  context.setPainter( p );
  context.setRendererScale( mMapCanvas->scale() );
  context.setScaleFactor( ms.outputDpi() / 25.4 );
  context.setRasterScaleFactor( 1.0 );

  context.setForceVectorOutput( true );
  return true;
}

void QgsMapCanvasItem::updatePosition()
{
  // default implementation: recalculate position of the item
  setRect( mRect, false );
  setRotation( mMapCanvas->rotation() - mRectRotation );
}


void QgsMapCanvasItem::setPanningOffset( const QPoint& point )
{
  mPanningOffset = point;
}
