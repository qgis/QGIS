/***************************************************************************
    qgsmaptoolpan.h  -  map tool for panning in map canvas
    ---------------------
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

#include <QBitmap>
#include <QCursor>
#include "qgsmaptoolpan.h"
#include "qgsmapcanvas.h"
#include "qgsmaptopixel.h"
#include "qgsmapmouseevent.h"
#include "qgsproject.h"
#include "qgslogger.h"


QgsMapToolPan::QgsMapToolPan( QgsMapCanvas *canvas )
  : QgsMapTool( canvas )
  , mDragging( false )
{
  mToolName = tr( "Pan" );
  // set cursor
  mCursor = QCursor( Qt::OpenHandCursor );
}

QgsMapToolPan::~QgsMapToolPan()
{
  mCanvas->ungrabGesture( Qt::PinchGesture );
}

void QgsMapToolPan::activate()
{
  mCanvas->grabGesture( Qt::PinchGesture );
  QgsMapTool::activate();
}

void QgsMapToolPan::deactivate()
{
  mCanvas->ungrabGesture( Qt::PinchGesture );
  QgsMapTool::deactivate();
}

QgsMapTool::Flags QgsMapToolPan::flags() const
{
  return QgsMapTool::Transient | QgsMapTool::AllowZoomRect | QgsMapTool::ShowContextMenu;
}

void QgsMapToolPan::canvasPressEvent( QgsMapMouseEvent *e )
{
  if ( e->button() == Qt::LeftButton )
  {
    mCanvas->setCursor( QCursor( Qt::ClosedHandCursor ) );
    mCanvas->panActionStart( e->pos() );
  }
}


void QgsMapToolPan::canvasMoveEvent( QgsMapMouseEvent *e )
{
  if ( !mPinching )
  {
    if ( ( e->buttons() & Qt::LeftButton ) )
    {
      mDragging = true;
      // move map and other canvas items
      mCanvas->panAction( e );
    }
  }
}

void QgsMapToolPan::canvasReleaseEvent( QgsMapMouseEvent *e )
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
        if ( mCanvas->allowInteraction( QgsMapCanvasInteractionBlocker::Interaction::MapPanOnSingleClick ) )
        {
          // transform the mouse pos to map coordinates
          const QgsPointXY prevCenter = mCanvas->center();
          const QgsPointXY center = mCanvas->getCoordinateTransform()->toMapCoordinates( e->x(), e->y() );
          mCanvas->setCenter( center );
          mCanvas->refresh();

          QgsDistanceArea da;
          da.setEllipsoid( QgsProject::instance()->ellipsoid() );
          da.setSourceCrs( mCanvas->mapSettings().destinationCrs(), QgsProject::instance()->transformContext() );
          try
          {
            emit panDistanceBearingChanged( da.measureLine( center, prevCenter ), da.lengthUnits(), da.bearing( center, prevCenter ) * 180 / M_PI );
          }
          catch ( QgsCsException & )
          {}
        }
      }
    }
  }
  mCanvas->setCursor( mCursor );
}

void QgsMapToolPan::canvasDoubleClickEvent( QgsMapMouseEvent *e )
{
  if ( !QTouchDevice::devices().isEmpty() && !mPinching )
  {
    mCanvas->zoomWithCenter( e->x(), e->y(), true );
  }
}

bool QgsMapToolPan::gestureEvent( QGestureEvent *event )
{
  if ( QTouchDevice::devices().isEmpty() )
    return true; // no touch support

  if ( QGesture *gesture = event->gesture( Qt::PinchGesture ) )
  {
    mPinching = true;
    pinchTriggered( static_cast<QPinchGesture *>( gesture ) );
  }
  return true;
}

void QgsMapToolPan::pinchTriggered( QPinchGesture *gesture )
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
      const QgsPointXY center  = mCanvas->getCoordinateTransform()->toMapCoordinates( pos.x(), pos.y() );
      QgsRectangle r = mCanvas->extent();
      r.scale( 1 / gesture->totalScaleFactor(), &center );
      mCanvas->setExtent( r );
      mCanvas->refresh();
    }
    mPinching = false;
  }
}
