/***************************************************************************
                          qgsplotpantool.cpp
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

#include "qgsplotpantool.h"
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

#if 0
    //get current visible part of scene
    const QRect viewportRect( 0, 0, canvas()->viewport()->width(), canvas()->viewport()->height() );
    QgsRectangle visibleRect = QgsRectangle( canvas()->mapToScene( viewportRect ).boundingRect() );
    const QPointF scenePoint = event->layoutPoint();
    visibleRect.scale( 1, scenePoint.x(), scenePoint.y() );
    const QRectF boundsRect = visibleRect.toRectF();

    //zoom view to fit desired bounds
    canvas()->fitInView( boundsRect, Qt::KeepAspectRatio );
    canvas()->emitZoomLevelChanged();
    canvas()->viewChanged();
#endif
    return;
  }

  if ( !mIsPanning || event->button() != Qt::LeftButton )
  {
    // view()->emitZoomLevelChanged();
    // view()->viewChanged();
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
