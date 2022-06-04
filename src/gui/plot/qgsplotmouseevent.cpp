/***************************************************************************
                          qgsplotmouseevent.cpp
                          ---------------
    begin                : March 2022
    copyright            : (C) 2022 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgsplotmouseevent.h"
#include "qgsplotcanvas.h"

QgsPlotMouseEvent::QgsPlotMouseEvent( QgsPlotCanvas *canvas, QMouseEvent *event )
  : QgsPlotMouseEvent( canvas, event->type(), event->pos(), event->button(), event->buttons(), event->modifiers() )
{
}

QgsPlotMouseEvent::QgsPlotMouseEvent( QgsPlotCanvas *canvas, QEvent::Type type, QPoint pos, Qt::MouseButton button, Qt::MouseButtons buttons, Qt::KeyboardModifiers modifiers )
  : QMouseEvent( type, pos, button, buttons, modifiers )
  , mCanvas( canvas )
  , mMapPoint( mCanvas->toMapCoordinates( pos ) )
{

}

QgsPoint QgsPlotMouseEvent::mapPoint() const
{
  return mMapPoint;
}

QgsPointXY QgsPlotMouseEvent::snappedPoint()
{
  if ( mHasCachedSnapResult )
    return mSnappedPoint;

  snapPoint();
  return mSnappedPoint;
}

bool QgsPlotMouseEvent::isSnapped()
{
  if ( mHasCachedSnapResult )
    return mIsSnapped;

  snapPoint();
  return mIsSnapped;
}

void QgsPlotMouseEvent::snapPoint()
{
  mHasCachedSnapResult = true;

  const QgsPointXY result = mCanvas->snapToPlot( pos() );
  mIsSnapped = !result.isEmpty();
  mSnappedPoint = !mIsSnapped ? pos() : result;
}
