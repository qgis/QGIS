/***************************************************************************
                             qgsmodelviewtoolpan.cpp
                             ------------------------------------
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

#include "qgsmodelviewtoolpan.h"
#include "qgsmodelviewmouseevent.h"
#include "qgsmodelgraphicsview.h"
#include <QScrollBar>

QgsModelViewToolPan::QgsModelViewToolPan( QgsModelGraphicsView *view )
  : QgsModelViewTool( view, tr( "Pan" ) )
{
  setCursor( Qt::OpenHandCursor );
}

void QgsModelViewToolPan::modelPressEvent( QgsModelViewMouseEvent *event )
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

void QgsModelViewToolPan::modelMoveEvent( QgsModelViewMouseEvent *event )
{
  if ( !mIsPanning )
  {
    event->ignore();
    return;
  }

  view()->horizontalScrollBar()->setValue( view()->horizontalScrollBar()->value() - ( event->x() - mLastMousePos.x() ) );
  view()->verticalScrollBar()->setValue( view()->verticalScrollBar()->value() - ( event->y() - mLastMousePos.y() ) );
  mLastMousePos = event->pos();
}

void QgsModelViewToolPan::modelReleaseEvent( QgsModelViewMouseEvent *event )
{
  const bool clickOnly = !isClickAndDrag( mMousePressStartPos, event->pos() );

  if ( event->button() == Qt::MiddleButton && clickOnly )
  {
    //middle mouse button click = recenter on point

    //get current visible part of scene
    const QRect viewportRect( 0, 0, view()->viewport()->width(), view()->viewport()->height() );
    QgsRectangle visibleRect = QgsRectangle( view()->mapToScene( viewportRect ).boundingRect() );
    const QPointF scenePoint = event->modelPoint();
    visibleRect.scale( 1, scenePoint.x(), scenePoint.y() );
    const QRectF boundsRect = visibleRect.toRectF();

    //zoom view to fit desired bounds
    view()->fitInView( boundsRect, Qt::KeepAspectRatio );
    return;
  }

  if ( !mIsPanning || event->button() != Qt::LeftButton )
  {
    event->ignore();
    return;
  }

  mIsPanning = false;
  view()->setCursor( Qt::OpenHandCursor );
}

void QgsModelViewToolPan::deactivate()
{
  mIsPanning = false;
  QgsModelViewTool::deactivate();
}
