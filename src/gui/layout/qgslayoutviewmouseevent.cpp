/***************************************************************************
                             qgslayoutviewmouseevent.cpp
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


#include "qgslayoutviewmouseevent.h"
#include "qgslayoutview.h"
#include "qgslayout.h"

QgsLayoutViewMouseEvent::QgsLayoutViewMouseEvent( QgsLayoutView *view, QMouseEvent *event, bool snap )
  : QMouseEvent( event->type(), event->pos(), event->button(), event->buttons(), event->modifiers() )
  , mView( view )
{
  mLayoutPoint = mView->mapToScene( x(), y() );
  if ( snap && mView->currentLayout() )
  {
    mSnappedPoint = mView->currentLayout()->snapper().snapPoint( mLayoutPoint, mView->transform().m11(), mSnapped );
  }
  else
  {
    mSnappedPoint = mLayoutPoint;
  }
}

void QgsLayoutViewMouseEvent::snapPoint( QGraphicsLineItem *horizontalSnapLine, QGraphicsLineItem *verticalSnapLine, const QList<QgsLayoutItem *> &excludeItems )
{
  if ( mView->currentLayout() )
  {
    mSnappedPoint = mView->currentLayout()->snapper().snapPoint( mLayoutPoint, mView->transform().m11(), mSnapped, horizontalSnapLine, verticalSnapLine, &excludeItems );
  }
}

QPointF QgsLayoutViewMouseEvent::layoutPoint() const
{
  return mLayoutPoint;
}
