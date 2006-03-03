/***************************************************************************
    qgsvertexmarker.cpp  - canvas item which shows a simple vertex marker
    ---------------------
    begin                : February 2006
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

#include <QPainter>

#include "qgsvertexmarker.h"


QgsVertexMarker::QgsVertexMarker(QgsMapCanvas* mapCanvas)
  : QgsMapCanvasItem(mapCanvas)
{
  mIconSize = 10;
  mIconType = ICON_X;
  mResizeType = ResizeManual;
}

void QgsVertexMarker::setIconType(int type)
{
  mIconType = type;
}
    
void QgsVertexMarker::setIconSize(int iconSize)
{
  mIconSize = iconSize;
}
    
void QgsVertexMarker::setCenter(const QgsPoint& point)
{
   mCenter = point;
   updatePosition();
   updateCanvas();
}


void QgsVertexMarker::drawShape(QPainter & p)
{
  QPoint pt = toCanvasCoords(mCenter);
  int x = pt.x(), y = pt.y();
  int s = (mIconSize - 1) / 2;
  
  // draw cross
  p.setPen(QColor(255,0,0));
  switch (mIconType)
  {
    case ICON_NONE:
      break;
      
    case ICON_CROSS:
      p.drawLine(x-s, y, x+s, y);
      p.drawLine(x, y-s, x, y+s);
      break;
      
    case ICON_X:
      p.drawLine(x-s, y-s, x+s, y+s);
      p.drawLine(x-s, y+s, x+s, y-s);
      break;
      
    case ICON_BOX:
      p.drawLine(x-s, y-s, x+s, y-s);
      p.drawLine(x+s, y-s, x+s, y+s);
      p.drawLine(x+s, y+s, x-s, y+s);
      p.drawLine(x-s, y+s, x-s, y-s);
      break;
  
  }
}
    

void QgsVertexMarker::updatePositionManual()
{
  QPoint pt = toCanvasCoords(mCenter);
  int s = (mIconSize - 1) / 2;
  move(pt.x() - s, pt.y() - s);
  setSize(mIconSize, mIconSize);
  show();
}


