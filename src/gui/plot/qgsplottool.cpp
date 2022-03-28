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

QgsPlotTool::QgsPlotTool( QgsPlotCanvas *canvas )
  : QObject( canvas )
  , mCanvas( canvas )
{
}

QgsPoint QgsPlotTool::toMapCoordinates( const QgsPointXY &point ) const
{
  return mCanvas->toMapCoordinates( point );
}

QgsPointXY QgsPlotTool::toCanvasCoordinates( const QgsPoint &point ) const
{
  return mCanvas->toCanvasCoordinates( point );
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
  emit activated();
}

void QgsPlotTool::deactivate()
{
  emit deactivated();
}

void QgsPlotTool::clean()
{
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

bool QgsPlotTool::populateContextMenuWithEvent( QMenu *, QgsPlotMouseEvent * )
{
  return false;
}
