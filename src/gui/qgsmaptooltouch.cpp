/***************************************************************************
    qgsmaptooltouch.cpp  -  map tool for zooming and panning using qgestures
    ----------------------
    begin                : February 2012
    copyright            : (C) 2012 by Marco Bernasocchi
    email                : marco at bernawebdesign.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsmaptooltouch.h"
#include "qgsmapcanvas.h"
#include "qgscursors.h"
#include "qgsmaptopixel.h"
#include <QBitmap>
#include <QCursor>
#include <QMouseEvent>
#include <qgslogger.h>


QgsMapToolTouch::QgsMapToolTouch( QgsMapCanvas* canvas )
    : QgsMapTool( canvas ), mDragging( false ), mPinching( false )
{
  // set cursor
//  QBitmap panBmp = QBitmap::fromData( QSize( 16, 16 ), pan_bits );
//  QBitmap panBmpMask = QBitmap::fromData( QSize( 16, 16 ), pan_mask_bits );
//  mCursor = QCursor( panBmp, panBmpMask, 5, 5 );
}

QgsMapToolTouch::~QgsMapToolTouch()
{
  mCanvas->ungrabGesture( Qt::PinchGesture );
}

void QgsMapToolTouch::activate()
{
  mCanvas->grabGesture( Qt::PinchGesture );
  QgsMapTool::activate();
}

void QgsMapToolTouch::deactivate()
{
  mCanvas->ungrabGesture( Qt::PinchGesture );
  QgsMapTool::deactivate();
}

void QgsMapToolTouch::canvasMoveEvent( QMouseEvent * e )
{
  if ( !mPinching )
  {
    if (( e->buttons() & Qt::LeftButton ) )
    {
      mDragging = true;
      // move map and other canvas items
      mCanvas->panAction( e );
    }
  }
}

void QgsMapToolTouch::canvasReleaseEvent( QMouseEvent * e )
{
  if ( !mPinching )
  {
    if ( e->button() == Qt::LeftButton )
    {
      if ( mDragging )
      {
        mCanvas->panActionEnd( e->pos() );
        mDragging = false;
      }
      else // add pan to mouse cursor
      {
        // transform the mouse pos to map coordinates
        QgsPoint center = mCanvas->getCoordinateTransform()->toMapPoint( e->x(), e->y() );
        mCanvas->setExtent( QgsRectangle( center, center ) );
        mCanvas->refresh();
      }
    }
  }
}

void QgsMapToolTouch::canvasDoubleClickEvent( QMouseEvent *e )
{
  if ( !mPinching )
  {
    mCanvas->zoomWithCenter( e->x(), e->y(), true );
  }
}

bool QgsMapToolTouch::gestureEvent( QGestureEvent *event )
{
  qDebug() << "gesture " << event;
  if ( QGesture *gesture = event->gesture( Qt::PinchGesture ) )
  {
    mPinching = true;
    pinchTriggered( static_cast<QPinchGesture *>( gesture ) );
  }
  return true;
}


void QgsMapToolTouch::pinchTriggered( QPinchGesture *gesture )
{
  if ( gesture->state() == Qt::GestureFinished )
  {
    //a very small totalScaleFactor indicates a two finger tap (pinch gesture without pinching)
    if ( 0.98 < gesture->totalScaleFactor()  && gesture->totalScaleFactor() < 1.02 )
    {
      mCanvas->zoomOut();
    }
    else
    {
      //Transfor global coordinates to widget coordinates
      QPoint pos = gesture->centerPoint().toPoint();
      pos = mCanvas->mapFromGlobal( pos );
      // transform the mouse pos to map coordinates
      QgsPoint center  = mCanvas->getCoordinateTransform()->toMapPoint( pos.x(), pos.y() );
      QgsRectangle r = mCanvas->extent();
      r.scale( 1 / gesture->totalScaleFactor(), &center );
      mCanvas->setExtent( r );
      mCanvas->refresh();
    }
    mPinching = false;
  }
}
