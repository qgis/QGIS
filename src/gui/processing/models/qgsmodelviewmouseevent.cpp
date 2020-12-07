/***************************************************************************
                             qgsmodelviewmouseevent.cpp
                             ---------------------------
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

#include "qgsmodelviewmouseevent.h"
#include "qgsmodelgraphicsview.h"
#include "qgsmodelsnapper.h"

QgsModelViewMouseEvent::QgsModelViewMouseEvent( QgsModelGraphicsView *view, QMouseEvent *event, bool snap )
  : QMouseEvent( event->type(), event->pos(), event->button(), event->buttons(), event->modifiers() )
  , mView( view )
{
  mModelPoint = mView->mapToScene( x(), y() );

  if ( snap && mView->snapper() )
  {
    mSnappedPoint = mView->snapper()->snapPoint( mModelPoint, mView->transform().m11(), mSnapped );
  }
  else
  {
    mSnappedPoint = mModelPoint;
  }
}

void QgsModelViewMouseEvent::snapPoint()
{
  if ( mView->snapper() )
  {
    mSnappedPoint = mView->snapper()->snapPoint( mModelPoint, mView->transform().m11(), mSnapped );
  }
}

QPointF QgsModelViewMouseEvent::modelPoint() const
{
  return mModelPoint;
}
