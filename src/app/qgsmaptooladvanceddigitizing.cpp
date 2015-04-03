/***************************************************************************
    qgsmaptooladvanceddigitizing.cpp  - map tool with event in map coordinates
    ----------------------
    begin                : October 2014
    copyright            : (C) Denis Rouzaud
    email                : denis.rouzaud@gmail.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsmapmouseevent.h"
#include "qgsmaptooladvanceddigitizing.h"
#include "qgisapp.h"


QgsMapToolAdvancedDigitizing::QgsMapToolAdvancedDigitizing( QgsMapCanvas* canvas )
    : QgsMapTool( canvas )
    , mCadAllowed( false )
    , mCaptureMode( CapturePoint )
    , mSnapOnPress( false )
    , mSnapOnRelease( false )
    , mSnapOnMove( false )
    , mSnapOnDoubleClick( false )
{
  mCadDockWidget = QgisApp::instance()->cadDockWidget();
}

QgsMapToolAdvancedDigitizing::~QgsMapToolAdvancedDigitizing()
{
}

void QgsMapToolAdvancedDigitizing::canvasPressEvent( QMouseEvent* e )
{
  QgsMapMouseEvent::SnappingMode mode = mSnapOnPress ? QgsMapMouseEvent::SnapProjectConfig : QgsMapMouseEvent::NoSnapping;
  QgsMapMouseEvent* event = new QgsMapMouseEvent( this, e, mode );
  if ( !mCadDockWidget->canvasPressEventFilter( event ) )
  {
    canvasMapPressEvent( event );
  }
  delete event;
}

void QgsMapToolAdvancedDigitizing::canvasReleaseEvent( QMouseEvent* e )
{
  QgsMapMouseEvent::SnappingMode mode = mSnapOnRelease ? QgsMapMouseEvent::SnapProjectConfig : QgsMapMouseEvent::NoSnapping;
  if ( mCadDockWidget->cadEnabled() )
    mode = mCadDockWidget->snappingMode();
  QgsMapMouseEvent* event = new QgsMapMouseEvent( this, e, mode );
  if ( !mCadDockWidget->canvasReleaseEventFilter( event ) )
  {
    canvasMapReleaseEvent( event );
  }
  delete event;
}

void QgsMapToolAdvancedDigitizing::canvasMoveEvent( QMouseEvent* e )
{
  QgsMapMouseEvent::SnappingMode mode = mSnapOnMove ? QgsMapMouseEvent::SnapProjectConfig : QgsMapMouseEvent::NoSnapping;
  if ( mCadDockWidget->cadEnabled() )
    mode = mCadDockWidget->snappingMode();
  QgsMapMouseEvent* event = new QgsMapMouseEvent( this, e, mode );
  if ( !mCadDockWidget->canvasMoveEventFilter( event ) )
  {
    canvasMapMoveEvent( event );
  }
  delete event;
}

void QgsMapToolAdvancedDigitizing::canvasDoubleClickEvent( QMouseEvent* e )
{
  QgsMapMouseEvent::SnappingMode mode = mSnapOnDoubleClick ? QgsMapMouseEvent::SnapProjectConfig : QgsMapMouseEvent::NoSnapping;
  QgsMapMouseEvent* event = new QgsMapMouseEvent( this, e, mode );
  canvasMapDoubleClickEvent( event );
  delete event;
}

void QgsMapToolAdvancedDigitizing::keyPressEvent( QKeyEvent* event )
{
  if ( !mCadDockWidget->canvasKeyPressEventFilter( event ) )
    canvasKeyPressEvent( event );
}

void QgsMapToolAdvancedDigitizing::keyReleaseEvent( QKeyEvent* event )
{
  canvasKeyReleaseEvent( event );
}


void QgsMapToolAdvancedDigitizing::canvasMapPressEvent( QgsMapMouseEvent *e )
{
  Q_UNUSED( e );
}

void QgsMapToolAdvancedDigitizing::canvasMapReleaseEvent( QgsMapMouseEvent *e )
{
  Q_UNUSED( e );
}

void QgsMapToolAdvancedDigitizing::canvasMapMoveEvent( QgsMapMouseEvent *e )
{
  Q_UNUSED( e );
}

void QgsMapToolAdvancedDigitizing::canvasMapDoubleClickEvent( QgsMapMouseEvent *e )
{
  Q_UNUSED( e );
}

void QgsMapToolAdvancedDigitizing::canvasKeyPressEvent( QKeyEvent* e )
{
  Q_UNUSED( e );
}

void QgsMapToolAdvancedDigitizing::canvasKeyReleaseEvent( QKeyEvent* e )
{
  Q_UNUSED( e );
}
