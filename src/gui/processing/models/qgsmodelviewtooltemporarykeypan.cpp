/***************************************************************************
                             qgsmodelviewtooltemporarykeypan.cpp
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

#include "qgsmodelviewtooltemporarykeypan.h"
#include "moc_qgsmodelviewtooltemporarykeypan.cpp"
#include "qgsmodelviewmouseevent.h"
#include "qgsmodelgraphicsview.h"
#include <QScrollBar>

QgsModelViewToolTemporaryKeyPan::QgsModelViewToolTemporaryKeyPan( QgsModelGraphicsView *view )
  : QgsModelViewTool( view, tr( "Pan" ) )
{
  setCursor( Qt::ClosedHandCursor );
}

void QgsModelViewToolTemporaryKeyPan::modelMoveEvent( QgsModelViewMouseEvent *event )
{
  view()->horizontalScrollBar()->setValue( view()->horizontalScrollBar()->value() - ( event->x() - mLastMousePos.x() ) );
  view()->verticalScrollBar()->setValue( view()->verticalScrollBar()->value() - ( event->y() - mLastMousePos.y() ) );
  mLastMousePos = event->pos();
}

void QgsModelViewToolTemporaryKeyPan::keyReleaseEvent( QKeyEvent *event )
{
  if ( event->key() == Qt::Key_Space && !event->isAutoRepeat() )
  {
    view()->setTool( mPreviousViewTool );
  }
}

void QgsModelViewToolTemporaryKeyPan::activate()
{
  mLastMousePos = view()->mapFromGlobal( QCursor::pos() );
  mPreviousViewTool = view()->tool();
  QgsModelViewTool::activate();
}
