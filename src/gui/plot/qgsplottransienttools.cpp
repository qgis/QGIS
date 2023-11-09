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
#include "qgsapplication.h"

#include <QKeyEvent>
#include <QApplication>

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


//
// QgsPlotToolTemporaryKeyZoom
//

QgsPlotToolTemporaryKeyZoom::QgsPlotToolTemporaryKeyZoom( QgsPlotCanvas *canvas )
  : QgsPlotToolZoom( canvas )
{
}

void QgsPlotToolTemporaryKeyZoom::plotReleaseEvent( QgsPlotMouseEvent *event )
{
  QgsPlotToolZoom::plotReleaseEvent( event );
  if ( !event->isAccepted() )
    return;

  //end of temporary zoom tool
  if ( mDeactivateOnMouseRelease )
    canvas()->setTool( mPreviousViewTool );
}

void QgsPlotToolTemporaryKeyZoom::keyPressEvent( QKeyEvent *event )
{
  if ( event->isAutoRepeat() )
  {
    event->ignore();
    return;
  }

  //respond to changes in ctrl key status
  if ( !( event->modifiers() & Qt::ControlModifier ) )
  {
    if ( !mMarqueeZoom )
    {
      //space pressed, but control key was released, end of temporary zoom tool
      canvas()->setTool( mPreviousViewTool );
    }
    else
    {
      mDeactivateOnMouseRelease = true;
    }
  }
  else
  {
    //both control and space pressed
    //set cursor to zoom in/out depending on alt key status
    updateCursor( event->modifiers() );
    if ( event->key() == Qt::Key_Space )
    {
      mDeactivateOnMouseRelease = false;
    }
  }
}

void QgsPlotToolTemporaryKeyZoom::keyReleaseEvent( QKeyEvent *event )
{
  if ( event->isAutoRepeat() )
  {
    event->ignore();
    return;
  }

  if ( event->key() == Qt::Key_Space )
  {
    //temporary keyboard-based zoom tool is active and space key has been released
    if ( !mMarqueeZoom )
    {
      //not mid-way through an operation, so immediately switch tool back
      canvas()->setTool( mPreviousViewTool );
    }
    else
    {
      mDeactivateOnMouseRelease = true;
    }
  }
  else
  {
    updateCursor( event->modifiers() );
    event->ignore();
  }
}

void QgsPlotToolTemporaryKeyZoom::activate()
{
  mDeactivateOnMouseRelease = false;
  mPreviousViewTool = canvas()->tool();
  QgsPlotToolZoom::activate();
  updateCursor( QApplication::keyboardModifiers() );
}

void QgsPlotToolTemporaryKeyZoom::updateCursor( Qt::KeyboardModifiers modifiers )
{
  canvas()->viewport()->setCursor( ( modifiers & Qt::AltModifier ) ?
                                   QgsApplication::getThemeCursor( QgsApplication::Cursor::ZoomOut ) :
                                   QgsApplication::getThemeCursor( QgsApplication::Cursor::ZoomIn ) );
}
