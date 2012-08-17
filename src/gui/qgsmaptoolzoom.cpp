/***************************************************************************
    qgsmaptoolzoom.cpp  -  map tool for zooming
    ----------------------
    begin                : January 2006
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

#include "qgsmaptoolzoom.h"
#include "qgsmapcanvas.h"
#include "qgsmaptopixel.h"
#include "qgscursors.h"
#include "qgsrubberband.h"

#include <QMouseEvent>
#include <QRect>
#include <QCursor>
#include <QPixmap>
#include "qgslogger.h"


QgsMapToolZoom::QgsMapToolZoom( QgsMapCanvas* canvas, bool zoomOut )
    : QgsMapTool( canvas ), mZoomOut( zoomOut ), mDragging( false ), mRubberBand( 0 )
{
  // set the cursor
  QPixmap myZoomQPixmap = QPixmap(( const char ** )( zoomOut ? zoom_out : zoom_in ) );
  mCursor = QCursor( myZoomQPixmap, 7, 7 );
}

QgsMapToolZoom::~QgsMapToolZoom()
{
  delete mRubberBand;
}


void QgsMapToolZoom::canvasMoveEvent( QMouseEvent * e )
{
  if ( !( e->buttons() & Qt::LeftButton ) )
    return;

  if ( !mDragging )
  {
    mDragging = true;
    delete mRubberBand;
    mRubberBand = new QgsRubberBand( mCanvas, true );
    mZoomRect.setTopLeft( e->pos() );
  }
  mZoomRect.setBottomRight( e->pos() );
  if ( mRubberBand )
  {
    mRubberBand->setToCanvasRectangle( mZoomRect );
    mRubberBand->show();
  }
}


void QgsMapToolZoom::canvasPressEvent( QMouseEvent * e )
{
  if ( e->button() != Qt::LeftButton )
    return;

  mZoomRect.setRect( 0, 0, 0, 0 );
}


void QgsMapToolZoom::canvasReleaseEvent( QMouseEvent * e )
{
  if ( e->button() != Qt::LeftButton )
    return;

  // We are not really dragging in this case. This is sometimes caused by
  // a pen based computer reporting a press, move, and release, all the
  // one point.
  if ( mDragging && ( mZoomRect.topLeft() == mZoomRect.bottomRight() ) )
  {
    mDragging = false;
    delete mRubberBand;
    mRubberBand = 0;
  }

  if ( mDragging )
  {
    mDragging = false;
    delete mRubberBand;
    mRubberBand = 0;

    // store the rectangle
    mZoomRect.setRight( e->pos().x() );
    mZoomRect.setBottom( e->pos().y() );

    const QgsMapToPixel* coordinateTransform = mCanvas->getCoordinateTransform();

    // set the extent to the zoomBox
    QgsPoint ll = coordinateTransform->toMapCoordinates( mZoomRect.left(), mZoomRect.bottom() );
    QgsPoint ur = coordinateTransform->toMapCoordinates( mZoomRect.right(), mZoomRect.top() );

    QgsRectangle r;
    r.setXMinimum( ll.x() );
    r.setYMinimum( ll.y() );
    r.setXMaximum( ur.x() );
    r.setYMaximum( ur.y() );
    r.normalize();

    // prevent zooming to an empty extent
    if ( r.width() == 0 || r.height() == 0 )
    {
      return;
    }

    if ( mZoomOut )
    {
      QgsPoint cer = r.center();
      QgsRectangle extent = mCanvas->extent();

      double sf;
      if ( mZoomRect.width() > mZoomRect.height() )
      {
        sf = extent.width() / r.width();
      }
      else
      {
        sf = extent.height() / r.height();
      }
      sf = sf / 2.0;
      r.scale( sf );

      QgsDebugMsg( QString( "Extent scaled by %1 to %2" ).arg( sf ).arg( r.toString().toLocal8Bit().constData() ) );
      QgsDebugMsg( QString( "Center of currentExtent after scaling is %1" ).arg( r.center().toString().toLocal8Bit().constData() ) );

    }

    mCanvas->setExtent( r );
    mCanvas->refresh();
  }
  else // not dragging
  {
    // change to zoom in/out by the default multiple
    mCanvas->zoomWithCenter( e->x(), e->y(), !mZoomOut );
  }
}

void QgsMapToolZoom::deactivate()
{
  delete mRubberBand;
  mRubberBand = 0;
}
