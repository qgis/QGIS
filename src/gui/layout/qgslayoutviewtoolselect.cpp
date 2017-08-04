/***************************************************************************
                             qgslayoutviewtoolselect.cpp
                             ---------------------------
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

#include "qgslayoutviewtoolselect.h"
#include "qgslayoutviewmouseevent.h"
#include "qgslayoutview.h"

QgsLayoutViewToolSelect::QgsLayoutViewToolSelect( QgsLayoutView *view )
  : QgsLayoutViewTool( view, tr( "Select" ) )
{
  setCursor( Qt::ArrowCursor );

  mRubberBand.reset( new QgsLayoutViewRectangularRubberBand( view ) );
  mRubberBand->setBrush( QBrush( QColor( 224, 178, 76, 63 ) ) );
  mRubberBand->setPen( QPen( QBrush( QColor( 254, 58, 29, 100 ) ), 0, Qt::DotLine ) );
}

void QgsLayoutViewToolSelect::layoutPressEvent( QgsLayoutViewMouseEvent *event )
{
  if ( event->button() != Qt::LeftButton )
  {
    event->ignore();
    return;
  }

  mIsSelecting = true;
  mMousePressStartPos = event->pos();
  mRubberBand->start( event->layoutPoint(), 0 );
}

void QgsLayoutViewToolSelect::layoutMoveEvent( QgsLayoutViewMouseEvent *event )
{
  if ( mIsSelecting )
  {
    mRubberBand->update( event->layoutPoint(), 0 );
  }
  else
  {
    event->ignore();
  }
}

void QgsLayoutViewToolSelect::layoutReleaseEvent( QgsLayoutViewMouseEvent *event )
{
  if ( !mIsSelecting || event->button() != Qt::LeftButton )
  {
    event->ignore();
    return;
  }

  mIsSelecting = false;
  QRectF rect = mRubberBand->finish( event->layoutPoint(), event->modifiers() );
  Q_UNUSED( rect );
}

void QgsLayoutViewToolSelect::deactivate()
{
  if ( mIsSelecting )
  {
    mRubberBand->finish();
    mIsSelecting = false;
  }
  QgsLayoutViewTool::deactivate();
}
