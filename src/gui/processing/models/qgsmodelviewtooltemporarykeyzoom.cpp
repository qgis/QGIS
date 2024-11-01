/***************************************************************************
                             qgsmodelviewtooltemporarykeyzoom.cpp
                             -------------------------------------
    Date                 : March 2020
    Copyright            : (C) 2020 Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsmodelviewtooltemporarykeyzoom.h"
#include "moc_qgsmodelviewtooltemporarykeyzoom.cpp"
#include "qgsmodelviewmouseevent.h"
#include "qgsmodelgraphicsview.h"
#include "qgsapplication.h"
#include <QScrollBar>
#include <QApplication>

QgsModelViewToolTemporaryKeyZoom::QgsModelViewToolTemporaryKeyZoom( QgsModelGraphicsView *view )
  : QgsModelViewToolZoom( view )
{
}

void QgsModelViewToolTemporaryKeyZoom::modelReleaseEvent( QgsModelViewMouseEvent *event )
{
  QgsModelViewToolZoom::modelReleaseEvent( event );
  if ( !event->isAccepted() )
    return;

  //end of temporary zoom tool
  if ( mDeactivateOnMouseRelease )
    view()->setTool( mPreviousViewTool );
}

void QgsModelViewToolTemporaryKeyZoom::keyPressEvent( QKeyEvent *event )
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
      view()->setTool( mPreviousViewTool );
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

void QgsModelViewToolTemporaryKeyZoom::keyReleaseEvent( QKeyEvent *event )
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
      view()->setTool( mPreviousViewTool );
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

void QgsModelViewToolTemporaryKeyZoom::activate()
{
  mDeactivateOnMouseRelease = false;
  mPreviousViewTool = view()->tool();
  QgsModelViewToolZoom::activate();
  updateCursor( QApplication::keyboardModifiers() );
}

void QgsModelViewToolTemporaryKeyZoom::updateCursor( Qt::KeyboardModifiers modifiers )
{
  view()->viewport()->setCursor( ( modifiers & Qt::AltModifier ) ? QgsApplication::getThemeCursor( QgsApplication::Cursor::ZoomOut ) : QgsApplication::getThemeCursor( QgsApplication::Cursor::ZoomIn ) );
}
