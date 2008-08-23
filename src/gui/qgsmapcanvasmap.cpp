/***************************************************************************
    qgsmapcanvasmap.cpp  -  draws the map in map canvas
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

#include "qgslogger.h"
#include "qgsmapcanvas.h"
#include "qgsmapcanvasmap.h"
#include "qgsmaprenderer.h"

#include <QPainter>

QgsMapCanvasMap::QgsMapCanvasMap( QgsMapCanvas* canvas )
    : mCanvas( canvas )
{
  setZValue( -10 );
  setPos( 0, 0 );
  resize( QSize( 1, 1 ) );
  mUseQImageToRender = false;
}

void QgsMapCanvasMap::paint( QPainter* p, const QStyleOptionGraphicsItem*, QWidget* )
{
  //refreshes the canvas map with the current offscreen image
  if ( mUseQImageToRender )
  {
    mPixmap = QPixmap::fromImage( mImage );
  }
  p->drawPixmap( 0, 0, mPixmap );
}

QRectF QgsMapCanvasMap::boundingRect() const
{
  return QRectF( 0, 0, mPixmap.width(), mPixmap.height() );
}


void QgsMapCanvasMap::resize( QSize size )
{
  mPixmap = QPixmap( size );
  mCanvas->mapRenderer()->setOutputSize( size, mPixmap.logicalDpiX() );
}

void QgsMapCanvasMap::setPanningOffset( const QPoint& point )
{
  mOffset = point;
  setPos( mOffset );
}

void QgsMapCanvasMap::render()
{
  // Rendering to a QImage gives incorrectly filled polygons in some
  // cases (as at Qt4.1.4), but it is the only renderer that supports
  // anti-aliasing, so we provide the means to swap between QImage and
  // QPixmap.

  if ( mUseQImageToRender )
  {
    // use temporary image for rendering
    mImage = QImage( boundingRect().size().toSize(), QImage::Format_RGB32 );

    mImage.fill( mBgColor.rgb() );

    QPainter paint;
    paint.begin( &mImage );
    // Clip drawing to the QImage
    paint.setClipRect( mImage.rect() );

    // antialiasing
    if ( mAntiAliasing )
      paint.setRenderHint( QPainter::Antialiasing );

    mCanvas->mapRenderer()->render( &paint );

    paint.end();

    // convert QImage to QPixmap to acheive faster drawing on screen
    //mPixmap = QPixmap::fromImage(image);
  }
  else
  {
    mPixmap.fill( mBgColor.rgb() );
    QPainter paint;
    paint.begin( &mPixmap );
    // Clip our drawing to the QPixmap
    paint.setClipRect( mPixmap.rect() );
    mCanvas->mapRenderer()->render( &paint );
    paint.end();
  }
  update();
}

QPaintDevice& QgsMapCanvasMap::paintDevice()
{
  if ( mUseQImageToRender )
  {
    return mImage;
  }
  else
  {
    return mPixmap;
  }
}
