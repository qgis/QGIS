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


QgsMapToolAdvancedDigitizing::QgsMapToolAdvancedDigitizing( QgsMapCanvas* canvas, QgsAdvancedDigitizingDockWidget* cadDockWidget )
    : QgsMapToolEdit( canvas )
    , mCaptureMode( CapturePoint )
    , mSnapOnPress( false )
    , mSnapOnRelease( false )
    , mSnapOnMove( false )
    , mSnapOnDoubleClick( false )
    , mCadDockWidget( cadDockWidget )
{
}

QgsMapToolAdvancedDigitizing::~QgsMapToolAdvancedDigitizing()
{
}

void QgsMapToolAdvancedDigitizing::canvasPressEvent( QgsMapMouseEvent* e )
{
  snap( e );
  if ( !mCadDockWidget->canvasPressEvent( e ) )
    cadCanvasPressEvent( e );
}

void QgsMapToolAdvancedDigitizing::canvasReleaseEvent( QgsMapMouseEvent* e )
{
  snap( e );
  if ( !mCadDockWidget->canvasReleaseEvent( e, mCaptureMode == CaptureLine || mCaptureMode == CapturePolygon ) )
    cadCanvasReleaseEvent( e );
}

void QgsMapToolAdvancedDigitizing::canvasMoveEvent( QgsMapMouseEvent* e )
{
  snap( e );
  if ( !mCadDockWidget->canvasMoveEvent( e ) )
    cadCanvasMoveEvent( e );
}

void QgsMapToolAdvancedDigitizing::activate()
{
  QgsMapToolEdit::activate();
  connect( mCadDockWidget, SIGNAL( pointChanged( QgsPoint ) ), this, SLOT( cadPointChanged( QgsPoint ) ) );
  mCadDockWidget->enable();
}

void QgsMapToolAdvancedDigitizing::deactivate()
{
  QgsMapToolEdit::deactivate();
  disconnect( mCadDockWidget, SIGNAL( pointChanged( QgsPoint ) ), this, SLOT( cadPointChanged( QgsPoint ) ) );
  mCadDockWidget->disable();
}

void QgsMapToolAdvancedDigitizing::cadPointChanged( const QgsPoint& point )
{
  QgsMapMouseEvent fakeEvent( mCanvas, QMouseEvent::Move, QPoint( 0, 0 ) );
  fakeEvent.setMapPoint( point );
  canvasMoveEvent( &fakeEvent );
}

void QgsMapToolAdvancedDigitizing::snap( QgsMapMouseEvent* e )
{
  if ( !mCadDockWidget->cadEnabled() )
    e->snapPoint( QgsMapMouseEvent::SnapProjectConfig );
}
