/***************************************************************************
                          qgsplottoolpan.cpp
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

#include "qgsplottoolpan.h"
#include "qgsplotcanvas.h"
#include "qgsplotmouseevent.h"

QgsPlotToolPan::QgsPlotToolPan( QgsPlotCanvas *canvas )
  : QgsPlotTool( canvas, tr( "Pan" ) )
{
  setCursor( Qt::OpenHandCursor );
}

Qgis::PlotToolFlags QgsPlotToolPan::flags() const
{
  return Qgis::PlotToolFlag::ShowContextMenu;
}

void QgsPlotToolPan::plotMoveEvent( QgsPlotMouseEvent *event )
{
  if ( !mIsPanning )
  {
    event->ignore();
    return;
  }

  mCanvas->panContentsBy( event->x() - mLastMousePos.x(), event->y() - mLastMousePos.y() );
  mLastMousePos = event->pos();
}

void QgsPlotToolPan::plotPressEvent( QgsPlotMouseEvent *event )
{
  mMousePressStartPos = event->pos();

  if ( event->button() != Qt::LeftButton )
  {
    event->ignore();
    return;
  }

  mIsPanning = true;
  mLastMousePos = event->pos();
  mCanvas->viewport()->setCursor( Qt::ClosedHandCursor );
}

void QgsPlotToolPan::plotReleaseEvent( QgsPlotMouseEvent *event )
{
  const bool clickOnly = !isClickAndDrag( mMousePressStartPos, event->pos() );

  if ( event->button() == Qt::MiddleButton && clickOnly )
  {
    //middle mouse button click = recenter on point
    mCanvas->centerPlotOn( event->pos().x(), event->pos().y() );
    return;
  }

  if ( !mIsPanning || event->button() != Qt::LeftButton )
  {
    event->ignore();
    return;
  }

  mIsPanning = false;
  canvas()->viewport()->setCursor( Qt::OpenHandCursor );
}

void QgsPlotToolPan::deactivate()
{
  mIsPanning = false;
  QgsPlotTool::deactivate();
}
