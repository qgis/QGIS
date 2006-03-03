/***************************************************************************
    qgsmaptoolemitpoint.cpp  -  map tool for signaling click on map canvas
    ---------------------
    begin                : January 2006
    copyright            : (C) 2006 by Martin Dobias
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


QgsMapToolEmitPoint::QgsMapToolEmitPoint(QgsMapCanvas* canvas)
  : QgsMapTool(canvas)
{
  mCursor = Qt::CrossCursor;
}
    
    
void QgsMapToolEmitPoint::canvasMoveEvent(QMouseEvent * e)
{
}
  
    
void QgsMapToolEmitPoint::canvasPressEvent(QMouseEvent * e)
{
  QgsPoint idPoint = toMapCoords(e->pos());
  mCanvas->emitPointEvent(idPoint, e->button());
}
  
  
void QgsMapToolEmitPoint::canvasReleaseEvent(QMouseEvent * e)
{
}
