/***************************************************************************
    qgsmaptool.h  -  base class for map canvas tools
    ----------------------
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

#include "qgsmaptool.h"
#include "qgsmapcanvas.h"
#include "qgsmaptopixel.h"
#include <QAction>

QgsMapTool::QgsMapTool(QgsMapCanvas* canvas)
  : mCanvas(canvas), mCursor(Qt::CrossCursor), mAction(NULL)
{
}


QgsMapTool::~QgsMapTool()
{
}


QgsPoint QgsMapTool::toMapCoords(const QPoint& point)
{
  return mCanvas->getCoordinateTransform()->toMapCoordinates(point);
}


QPoint QgsMapTool::toCanvasCoords(const QgsPoint& point)
{
  double x = point.x(), y = point.y();
  mCanvas->getCoordinateTransform()->transformInPlace(x,y);
  return QPoint((int)(x+0.5), (int)(y+0.5)); // round the values
}


void QgsMapTool::activate()
{
  // make action active
  if (mAction)
    mAction->setOn(true);
  
  // set cursor (map tools usually set it in constructor
  mCanvas->setCursor(mCursor);
}
    

void QgsMapTool::deactivate()
{
}
