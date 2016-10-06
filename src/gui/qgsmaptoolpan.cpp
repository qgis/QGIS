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

#include "qgsmaptoolpan.h"
#include "qgsmapcanvas.h"
#include "qgscursors.h"
#include "qgsmaptopixel.h"
#include <QBitmap>
#include <QCursor>
#include <QMouseEvent>


QgsMapToolPan::QgsMapToolPan( QgsMapCanvas* canvas )
    : QgsMapTool( canvas )
    , mDragging( false )
{
  mToolName = tr( "Pan" );
  // set cursor
  mCursor = QCursor( Qt::OpenHandCursor );
}

void QgsMapToolPan::canvasPressEvent( QgsMapMouseEvent* e )
{
  if ( e->button() == Qt::LeftButton )
    mCanvas->setCursor( QCursor( Qt::ClosedHandCursor ) );
}


void QgsMapToolPan::canvasMoveEvent( QgsMapMouseEvent* e )
{
  if (( e->buttons() & Qt::LeftButton ) )
  {
    mDragging = true;
    // move map and other canvas items
    mCanvas->panAction( e );
  }
}

void QgsMapToolPan::canvasReleaseEvent( QgsMapMouseEvent* e )
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
      mCanvas->setCenter( center );
      mCanvas->refresh();
    }
  }
  mCanvas->setCursor( mCursor );
}
