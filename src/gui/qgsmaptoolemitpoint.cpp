/***************************************************************************
    qgsmaptoolemitpoint.cpp  -  map tool that emits a signal on click
    ---------------------
    begin                : June 2007
    copyright            : (C) 2007 by Martin Dobias
    email                : wonder.sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
/* $Id$ */


#include "qgsmaptoolemitpoint.h"
#include "qgsmapcanvas.h"
#include <QMouseEvent>


QgsMapToolEmitPoint::QgsMapToolEmitPoint( QgsMapCanvas* canvas )
    : QgsMapTool( canvas )
{
}

void QgsMapToolEmitPoint::canvasMoveEvent( QMouseEvent * e )
{
}

void QgsMapToolEmitPoint::canvasPressEvent( QMouseEvent * e )
{
  QgsPoint pnt = toMapCoordinates( e->pos() );
  emit canvasClicked( pnt, e->button() );
}

void QgsMapToolEmitPoint::canvasReleaseEvent( QMouseEvent * e )
{
}
