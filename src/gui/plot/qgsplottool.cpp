/***************************************************************************
                          qgsplottool.cpp
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

#include "qgsplottool.h"
#include "qgsplotcanvas.h"
#include "qgsplotmouseevent.h"

#include <QWheelEvent>
#include <QAction>

QgsPlotTool::QgsPlotTool( QgsPlotCanvas *canvas, const QString &name )
  : QObject( canvas )
  , mCanvas( canvas )
  , mToolName( name )
{
  connect( mCanvas, &QgsPlotCanvas::willBeDeleted, this, [ = ]
  {
    mCanvas = nullptr;
  } );
}

QgsPoint QgsPlotTool::toMapCoordinates( const QgsPointXY &point ) const
{
  return mCanvas->toMapCoordinates( point );
}

QgsPointXY QgsPlotTool::toCanvasCoordinates( const QgsPoint &point ) const
{
  return mCanvas->toCanvasCoordinates( point );
}

bool QgsPlotTool::isClickAndDrag( QPoint startViewPoint, QPoint endViewPoint ) const
{
  const int diffX = endViewPoint.x() - startViewPoint.x();
  const int diffY = endViewPoint.y() - startViewPoint.y();
  return std::abs( diffX ) >= 2 || std::abs( diffY ) >= 2;
}

QgsPlotTool::~QgsPlotTool()
{
  if ( mCanvas )
    mCanvas->unsetTool( this );
}

Qgis::PlotToolFlags QgsPlotTool::flags() const
{
  return Qgis::PlotToolFlags();
}

void QgsPlotTool::activate()
{
  // make action and/or button active
  if ( mAction )
    mAction->setChecked( true );

  mCanvas->viewport()->setCursor( mCursor );
  emit activated();
}

void QgsPlotTool::deactivate()
{
  if ( mAction )
    mAction->setChecked( false );

  emit deactivated();
}

bool QgsPlotTool::isActive() const
{
  return mCanvas && mCanvas->tool() == this;
}

void QgsPlotTool::plotMoveEvent( QgsPlotMouseEvent *event )
{
  event->ignore();
}

void QgsPlotTool::plotDoubleClickEvent( QgsPlotMouseEvent *event )
{
  event->ignore();
}

void QgsPlotTool::plotPressEvent( QgsPlotMouseEvent *event )
{
  event->ignore();
}

void QgsPlotTool::plotReleaseEvent( QgsPlotMouseEvent *event )
{
  event->ignore();
}

void QgsPlotTool::wheelEvent( QWheelEvent *event )
{
  event->ignore();
}

void QgsPlotTool::keyPressEvent( QKeyEvent *event )
{
  event->ignore();
}

void QgsPlotTool::keyReleaseEvent( QKeyEvent *event )
{
  event->ignore();
}

bool QgsPlotTool::gestureEvent( QGestureEvent * )
{
  return false;
}

bool QgsPlotTool::canvasToolTipEvent( QHelpEvent * )
{
  return false;
}

QgsPlotCanvas *QgsPlotTool::canvas() const
{
  return mCanvas;
}

void QgsPlotTool::setAction( QAction *action )
{
  mAction = action;
}

QAction *QgsPlotTool::action()
{
  return mAction;
}

void QgsPlotTool::setCursor( const QCursor &cursor )
{
  mCursor = cursor;
}

bool QgsPlotTool::populateContextMenuWithEvent( QMenu *, QgsPlotMouseEvent * )
{
  return false;
}
