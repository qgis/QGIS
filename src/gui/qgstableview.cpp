/***************************************************************************
    qgstableview.cpp
    ---------------
    begin                : March 2022
    copyright            : (C) 2022 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgstableview.h"

#include <QWheelEvent>

QgsTableView::QgsTableView( QWidget *parent )
  : QTableView( parent )
{

}

void QgsTableView::wheelEvent( QWheelEvent *event )
{
  if ( event->modifiers() & Qt::ShiftModifier )
  {
    // a wheel event with the shift modifier switches a vertical scroll to a horizontal scroll (or vice versa)
    const QPoint invertedPixelDelta = QPoint( event->pixelDelta().y(), event->pixelDelta().x() );
    const QPoint invertedAngleDelta = QPoint( event->angleDelta().y(), event->angleDelta().x() );
    QWheelEvent axisSwappedScrollEvent( event->posF(), event->globalPosF(),
                                        invertedPixelDelta, invertedAngleDelta,
                                        event->buttons(), event->modifiers() & ~Qt::ShiftModifier, event->phase(),
                                        event->inverted(), event->source() );
    QTableView::wheelEvent( &axisSwappedScrollEvent );
  }
  else
  {
    QTableView::wheelEvent( event );
  }
}
