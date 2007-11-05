/***************************************************************************
    qgsmaptoolcapture.cpp  -  map tool for capturing points, lines, polygons
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

#include "qgsapplication.h"
#include "qgsattributedialog.h"
#include "qgscoordinatetransform.h"
#include "qgsfield.h"
#include "qgsmaptoolcapture.h"
#include "qgsmapcanvas.h"
#include "qgsmaprender.h"
#include "qgsmaptopixel.h"
#include "qgsfeature.h"
#include "qgsproject.h"
#include "qgsrubberband.h"
#include "qgsvectorlayer.h"
#include "qgsvectordataprovider.h"
#include "qgscursors.h"
#include <QCursor>
#include <QPixmap>
#include <QMessageBox>


QgsMapToolCapture::QgsMapToolCapture(QgsMapCanvas* canvas, enum CaptureTool tool)
  : QgsMapTool(canvas), mTool(tool), mRubberBand(0)
{
  mCapturing = FALSE;
  
  QPixmap mySelectQPixmap = QPixmap((const char **) capture_point_cursor);
  mCursor = QCursor(mySelectQPixmap, 8, 8);
}

QgsMapToolCapture::~QgsMapToolCapture()
{
  delete mRubberBand;
  mRubberBand = 0;
}

void QgsMapToolCapture::canvasMoveEvent(QMouseEvent * e)
{

  if (mCapturing)
  {
    // show the rubber-band from the last click
    QgsVectorLayer *vlayer = dynamic_cast <QgsVectorLayer*>(mCanvas->currentLayer());
    double tolerance  = QgsProject::instance()->readDoubleEntry("Digitizing","/Tolerance",0);
    QgsPoint mapPoint;
    QgsPoint layerPoint = toLayerCoords(vlayer, e->pos());
    vlayer->snapPoint(layerPoint, tolerance); //show snapping during dragging
    //now we need to know the map coordinates of the snapped point for the rubber band
    mapPoint = toMapCoords(vlayer, layerPoint);
    mRubberBand->movePoint(mapPoint); //does only work if coordinate reprojection is not enabled
  }

} // mouseMoveEvent


void QgsMapToolCapture::canvasPressEvent(QMouseEvent * e)
{
  // nothing to be done
}


void QgsMapToolCapture::renderComplete()
{
}

void QgsMapToolCapture::deactivate()
{
  delete mRubberBand;
  mRubberBand = 0;
}

int QgsMapToolCapture::addVertex(const QPoint& p)
{
  QgsVectorLayer *vlayer = dynamic_cast <QgsVectorLayer*>(mCanvas->currentLayer());
  
  if (!vlayer)
    {
      return 1;
    }

  if (!mRubberBand)
    {
      mRubberBand = new QgsRubberBand(mCanvas, mTool == CapturePolygon);
      QgsProject* project = QgsProject::instance();
      QColor color(
		   project->readNumEntry("Digitizing", "/LineColorRedPart", 255),
		   project->readNumEntry("Digitizing", "/LineColorGreenPart", 0),
		   project->readNumEntry("Digitizing", "/LineColorBluePart", 0));
      mRubberBand->setColor(color);
      mRubberBand->setWidth(project->readNumEntry("Digitizing", "/LineWidth", 1));
      mRubberBand->show();
    }
      
  QgsPoint mapPoint;
  QgsPoint digitisedPoint;
  try
    {
      digitisedPoint = toLayerCoords(vlayer, p); //todo: handle coordinate transform exception
    }
  catch(QgsCsException &cse)
    {
      UNUSED(cse); // unused
      return 2; //cannot reproject point to layer coordinate system
    }

  //snap point
  double tolerance  = QgsProject::instance()->readDoubleEntry("Digitizing","/Tolerance",0);
  vlayer->snapPoint(digitisedPoint, tolerance);
  mapPoint = toMapCoords(vlayer, digitisedPoint);
  
  mCaptureList.push_back(digitisedPoint);
  mRubberBand->addPoint(mapPoint);

  return 0;
}
