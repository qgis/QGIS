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


QgsLayoutViewMouseEvent::QgsLayoutViewMouseEvent( QgsLayoutView *view, QMouseEvent *event )
  : QMouseEvent( event->type(), event->pos(), event->button(), event->buttons(), event->modifiers() )
  , mView( view )
{

}

QPointF QgsLayoutViewMouseEvent::layoutPoint() const
{
  return mView->mapToScene( x(), y() );
}
