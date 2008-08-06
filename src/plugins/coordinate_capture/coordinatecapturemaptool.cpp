/***************************************************************************
    coordinatecapturemaptool.cpp  -  map tool for getting click coords
    ---------------------
    begin                : August 2008
    copyright            : (C) 2008 by Tim Sutton
    email                : tim at linfiniti.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
/* $Id$ */

#include "coordinatecapturemaptool.h"
#include "qgscursors.h"
#include "qgsmapcanvas.h"
#include "qgsmaptopixel.h"
#include "qgsrubberband.h"
#include "qgsspatialrefsys.h"

#include <QSettings>
#include <QMessageBox>
#include <QMouseEvent>
#include <QCursor>
#include <QPixmap>

CoordinateCaptureMapTool::CoordinateCaptureMapTool(QgsMapCanvas* thepCanvas)
  : QgsMapTool(thepCanvas)
{
  // set cursor
  QPixmap myIdentifyCursor = QPixmap((const char **) capture_point_cursor);
  mCursor = QCursor(myIdentifyCursor, 1, 1);
  mpMapCanvas = thepCanvas;
  mpRubberBand = new QgsRubberBand(mpMapCanvas,false); //false - not a polygon
  mpRubberBand->setColor(Qt::red);
  mpRubberBand->setWidth(3);
}

CoordinateCaptureMapTool::~CoordinateCaptureMapTool()
{
  if (mpRubberBand != 0)
  delete mpRubberBand;
}

void CoordinateCaptureMapTool::canvasMoveEvent(QMouseEvent * e)
{
}

void CoordinateCaptureMapTool::canvasPressEvent(QMouseEvent * thepEvent)
{
}

void CoordinateCaptureMapTool::canvasReleaseEvent(QMouseEvent * thepEvent)
{
  if(!mpMapCanvas || mpMapCanvas->isDrawing())
  {
    return;
  }

  QgsPoint myPoint = 
    mCanvas->getCoordinateTransform()->toMapCoordinates(thepEvent->x(), thepEvent->y());
  emit pointCaptured(myPoint);
  mpRubberBand->reset(false);
  // convert screen coordinates to map coordinates
  mpRubberBand->addPoint(myPoint,false); //true - update canvas
  mpRubberBand->addPoint(myPoint,false); //true - update canvas
  mpRubberBand->addPoint(myPoint,false); //true - update canvas
  mpRubberBand->show();
}


void CoordinateCaptureMapTool::deactivate()
{
  mpRubberBand->reset(false);
  QgsMapTool::deactivate();
}
