/***************************************************************************
                             qgslayoutviewtooltemporarykeypan.cpp
                             ------------------------------------
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

#include "qgslayoutviewtooltemporarykeypan.h"
#include "moc_qgslayoutviewtooltemporarykeypan.cpp"
#include "qgslayoutviewmouseevent.h"
#include "qgslayoutview.h"
#include <QScrollBar>

QgsLayoutViewToolTemporaryKeyPan::QgsLayoutViewToolTemporaryKeyPan( QgsLayoutView *view )
  : QgsLayoutViewTool( view, tr( "Pan" ) )
{
  setCursor( Qt::ClosedHandCursor );
}

void QgsLayoutViewToolTemporaryKeyPan::layoutMoveEvent( QgsLayoutViewMouseEvent *event )
{
  view()->horizontalScrollBar()->setValue( view()->horizontalScrollBar()->value() - ( event->x() - mLastMousePos.x() ) );
  view()->verticalScrollBar()->setValue( view()->verticalScrollBar()->value() - ( event->y() - mLastMousePos.y() ) );
  mLastMousePos = event->pos();
  view()->viewChanged();
}

void QgsLayoutViewToolTemporaryKeyPan::keyReleaseEvent( QKeyEvent *event )
{
  if ( event->key() == Qt::Key_Space && !event->isAutoRepeat() )
  {
    view()->setTool( mPreviousViewTool );
  }
}

void QgsLayoutViewToolTemporaryKeyPan::activate()
{
  mLastMousePos = view()->mapFromGlobal( QCursor::pos() );
  mPreviousViewTool = view()->tool();
  QgsLayoutViewTool::activate();
}
