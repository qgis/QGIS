/***************************************************************************
                          qgsplottransienttools.cpp
                          ---------------
    begin                : March 2022
    copyright            : (C) 2022 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsplottransienttools.h"
#include "qgsplotcanvas.h"
#include "qgsplotmouseevent.h"

#include <QKeyEvent>

//
// QgsPlotToolTemporaryKeyPan
//

QgsPlotToolTemporaryKeyPan::QgsPlotToolTemporaryKeyPan( QgsPlotCanvas *canvas )
  : QgsPlotTool( canvas, tr( "Pan" ) )
{
  setCursor( Qt::ClosedHandCursor );
}

void QgsPlotToolTemporaryKeyPan::plotMoveEvent( QgsPlotMouseEvent *event )
{
  canvas()->panContentsBy( event->x() - mLastMousePos.x(),
                           event->y() - mLastMousePos.y() );
  mLastMousePos = event->pos();
}

void QgsPlotToolTemporaryKeyPan::keyReleaseEvent( QKeyEvent *event )
{
  if ( event->key() == Qt::Key_Space && !event->isAutoRepeat() )
  {
    canvas()->setTool( mPreviousTool );
  }
}

void QgsPlotToolTemporaryKeyPan::activate()
{
  mLastMousePos = canvas()->mapFromGlobal( QCursor::pos() );
  mPreviousTool = canvas()->tool();
  QgsPlotTool::activate();
}


//
// QgsPlotToolTemporaryMousePan
//

QgsPlotToolTemporaryMousePan::QgsPlotToolTemporaryMousePan( QgsPlotCanvas *canvas )
  : QgsPlotTool( canvas, tr( "Pan" ) )
{
  setCursor( Qt::ClosedHandCursor );
}

void QgsPlotToolTemporaryMousePan::plotMoveEvent( QgsPlotMouseEvent *event )
{
  canvas()->panContentsBy( event->x() - mLastMousePos.x(),
                           event->y() - mLastMousePos.y() );
  mLastMousePos = event->pos();
}

void QgsPlotToolTemporaryMousePan::plotReleaseEvent( QgsPlotMouseEvent *event )
{
  if ( event->button() == Qt::MiddleButton )
  {
    canvas()->setTool( mPreviousTool );
  }
}

void QgsPlotToolTemporaryMousePan::activate()
{
  mLastMousePos = canvas()->mapFromGlobal( QCursor::pos() );
  mPreviousTool = canvas()->tool();
  QgsPlotTool::activate();
}

