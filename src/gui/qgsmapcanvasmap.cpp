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
  p->drawPixmap( 0, 0, mPixmap );
}

QRectF QgsMapCanvasMap::boundingRect() const
{
  return QRectF( 0, 0, mPixmap.width(), mPixmap.height() );
}


void QgsMapCanvasMap::resize( QSize size )
{
  QgsDebugMsg( QString( "resizing to %1x%2" ).arg( size.width() ).arg( size.height() ) );
  prepareGeometryChange(); // to keep QGraphicsScene indexes up to date on size change

  mPixmap = QPixmap( size );
  mPixmap.fill( mBgColor.rgb() );
  mImage = QImage( size, QImage::Format_RGB32 ); // temporary image - build it here so it is available when switching from QPixmap to QImage rendering
  mCanvas->mapRenderer()->setOutputSize( size, mPixmap.logicalDpiX() );
}

void QgsMapCanvasMap::setPanningOffset( const QPoint& point )
{
  mOffset = point;
  setPos( mOffset );
}

void QgsMapCanvasMap::render()
{
  if ( mUseQImageToRender )
  {
    // use temporary image for rendering
    mImage.fill( mBgColor.rgb() );

    // clear the pixmap so that old map won't be displayed while rendering
    // TODO: do the canvas updates wisely -> this wouldn't be needed
    mPixmap = QPixmap( mImage.size() );
    mPixmap.fill( mBgColor.rgb() );

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
    mPixmap = QPixmap::fromImage( mImage );
  }
  else
  {
    mPixmap.fill( mBgColor.rgb() );
    QPainter paint;
    paint.begin( &mPixmap );
    // Clip our drawing to the QPixmap
    paint.setClipRect( mPixmap.rect() );

    // antialiasing
    if ( mAntiAliasing )
      paint.setRenderHint( QPainter::Antialiasing );

    mCanvas->mapRenderer()->render( &paint );
    paint.end();
  }
  update();
}

QPaintDevice& QgsMapCanvasMap::paintDevice()
{
  return mPixmap;
}

void QgsMapCanvasMap::updateContents()
{
  // make sure we're using current contents
  if ( mUseQImageToRender )
    mPixmap = QPixmap::fromImage( mImage );

  // trigger update of this item
  update();
}
