/***************************************************************************
    qgsmaptoolpan.h  -  map tool for panning in map canvas
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

#include "qgsmaptoolpan.h"
#include "qgsmapcanvas.h"
#include "qgscursors.h"
#include <QBitmap>
#include <QCursor>
#include <QMouseEvent>


QgsMapToolPan::QgsMapToolPan(QgsMapCanvas* canvas)
  : QgsMapTool(canvas), mDragging(FALSE)
{
  // set cursor
  QBitmap panBmp = QBitmap::fromData( QSize(16, 16), pan_bits);
  QBitmap panBmpMask = QBitmap::fromData( QSize(16, 16), pan_mask_bits);
  mCursor = QCursor(panBmp, panBmpMask, 5, 5);
}

    
void QgsMapToolPan::canvasMoveEvent(QMouseEvent * e)
{
  if (mDragging && (e->buttons() & Qt::LeftButton))
  {
    // move map and other canvas items
    mCanvas->panAction(e);
  }
}
  
    
void QgsMapToolPan::canvasPressEvent(QMouseEvent * e)
{
  if (e->button() == Qt::LeftButton)
  {
    mDragging = TRUE;
  }
}
  
    
void QgsMapToolPan::canvasReleaseEvent(QMouseEvent * e)
{
  if (mDragging && e->button() == Qt::LeftButton)
  {
    mCanvas->panActionEnd(e->pos());
    mDragging = FALSE;
  }
}
