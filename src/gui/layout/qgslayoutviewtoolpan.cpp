/***************************************************************************
                             qgslayoutviewtoolpan.cpp
                             ------------------------
    Date                 : July 2017
    Copyright            : (C) 2017 Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgslayoutviewtoolpan.h"
#include "qgslayoutviewmouseevent.h"
#include "qgslayoutview.h"
#include <QScrollBar>

QgsLayoutViewToolPan::QgsLayoutViewToolPan( QgsLayoutView *view )
  : QgsLayoutViewTool( view, tr( "Pan" ) )
{
  setCursor( Qt::OpenHandCursor );
}

void QgsLayoutViewToolPan::layoutPressEvent( QgsLayoutViewMouseEvent *event )
{
  mMousePressStartPos = event->pos();

  if ( event->button() != Qt::LeftButton )
  {
    event->ignore();
    return;
  }

  mIsPanning = true;
  mLastMousePos = event->pos();
  view()->setCursor( Qt::ClosedHandCursor );
}

void QgsLayoutViewToolPan::layoutMoveEvent( QgsLayoutViewMouseEvent *event )
{
  if ( !mIsPanning )
  {
    event->ignore();
    return;
  }

  view()->horizontalScrollBar()->setValue( view()->horizontalScrollBar()->value() - ( event->x() - mLastMousePos.x() ) );
  view()->verticalScrollBar()->setValue( view()->verticalScrollBar()->value() - ( event->y() - mLastMousePos.y() ) );
  mLastMousePos = event->pos();
  view()->viewChanged();
}

void QgsLayoutViewToolPan::layoutReleaseEvent( QgsLayoutViewMouseEvent *event )
{
  const bool clickOnly = !isClickAndDrag( mMousePressStartPos, event->pos() );

  if ( event->button() == Qt::MiddleButton && clickOnly )
  {
    //middle mouse button click = recenter on point

    //get current visible part of scene
    const QRect viewportRect( 0, 0, view()->viewport()->width(), view()->viewport()->height() );
    QgsRectangle visibleRect = QgsRectangle( view()->mapToScene( viewportRect ).boundingRect() );
    const QPointF scenePoint = event->layoutPoint();
    visibleRect.scale( 1, scenePoint.x(), scenePoint.y() );
    const QRectF boundsRect = visibleRect.toRectF();

    //zoom view to fit desired bounds
    view()->fitInView( boundsRect, Qt::KeepAspectRatio );
    view()->emitZoomLevelChanged();
    view()->viewChanged();
    return;
  }

  if ( !mIsPanning || event->button() != Qt::LeftButton )
  {
    view()->emitZoomLevelChanged();
    view()->viewChanged();
    event->ignore();
    return;
  }

  mIsPanning = false;
  view()->setCursor( Qt::OpenHandCursor );
}

void QgsLayoutViewToolPan::deactivate()
{
  mIsPanning = false;
  QgsLayoutViewTool::deactivate();
}
