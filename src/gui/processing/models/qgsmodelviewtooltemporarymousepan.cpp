/***************************************************************************
                             qgsmodelviewtooltemporarymousepan.cpp
                             --------------------------------------
    Date                 : March 2020
    Copyright            : (C) 2020 Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   *
 *  
 *        *
 *                                     *
 *                                                                         *
 ***************************************************************************/

#include "qgsmodelviewtooltemporarymousepan.h"
#include "qgsmodelviewmouseevent.h"
#include "qgsmodelgraphicsview.h"
#include <QScrollBar>

QgsModelViewToolTemporaryMousePan::QgsModelViewToolTemporaryMousePan( QgsModelGraphicsView *view )
  : QgsModelViewTool( view, tr( "Pan" ) )
{
  setCursor( Qt::ClosedHandCursor );
}

void QgsModelViewToolTemporaryMousePan::modelMoveEvent( QgsModelViewMouseEvent *event )
{
  view()->horizontalScrollBar()->setValue( view()->horizontalScrollBar()->value() - ( event->x() - mLastMousePos.x() ) );
  view()->verticalScrollBar()->setValue( view()->verticalScrollBar()->value() - ( event->y() - mLastMousePos.y() ) );
  mLastMousePos = event->pos();
}

void QgsModelViewToolTemporaryMousePan::modelReleaseEvent( QgsModelViewMouseEvent *event )
{
  if ( event->button() == Qt::MidButton )
  {
    view()->setTool( mPreviousViewTool );
  }
}

void QgsModelViewToolTemporaryMousePan::activate()
{
  mLastMousePos = view()->mapFromGlobal( QCursor::pos() );
  mPreviousViewTool = view()->tool();
  QgsModelViewTool::activate();
}
