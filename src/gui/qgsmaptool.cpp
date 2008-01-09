/***************************************************************************
    qgsmaptool.cpp  -  base class for map canvas tools
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

#include "qgslogger.h"
#include "qgsmaptool.h"
#include "qgsmapcanvas.h"
#include "qgsmaprender.h"
#include "qgsmaptopixel.h"
#include <QAction>
#include <QAbstractButton>

QgsMapTool::QgsMapTool(QgsMapCanvas* canvas)
  : QObject(canvas), mCanvas(canvas), mCursor(Qt::CrossCursor), mAction(NULL), mButton(NULL)
{
}


QgsMapTool::~QgsMapTool()
{
  mCanvas->unsetMapTool(this);
}


QgsPoint QgsMapTool::toMapCoords(const QPoint& point)
{
  return mCanvas->getCoordinateTransform()->toMapCoordinates(point);
}


QgsPoint QgsMapTool::toLayerCoords(QgsMapLayer* layer, const QPoint& point)
{
  QgsPoint pt = toMapCoords(point);
  return toLayerCoords(layer, pt);
}

QgsPoint QgsMapTool::toLayerCoords(QgsMapLayer* layer, const QgsPoint& point)
{
  return mCanvas->mapRender()->outputCoordsToLayerCoords(layer, point);
}

QgsPoint QgsMapTool::toMapCoords(QgsMapLayer* layer, const QgsPoint& point)
{
  return mCanvas->mapRender()->layerCoordsToOutputCoords(layer, point);
}

QgsRect QgsMapTool::toLayerCoords(QgsMapLayer* layer, const QgsRect& rect)
{
  return mCanvas->mapRender()->outputCoordsToLayerCoords(layer, rect);
}

QPoint QgsMapTool::toCanvasCoords(const QgsPoint& point)
{
  double x = point.x(), y = point.y();
  mCanvas->getCoordinateTransform()->transformInPlace(x,y);
  return QPoint((int)(x+0.5), (int)(y+0.5)); // round the values
}


void QgsMapTool::activate()
{
  // make action and/or button active
  if (mAction)
    mAction->setChecked(true);
  if (mButton)
    mButton->setChecked(true);
  
  // set cursor (map tools usually set it in constructor)
  mCanvas->setCursor(mCursor);
  QgsDebugMsg("Cursor has been set");
}
    

void QgsMapTool::deactivate()
{
  if (mAction)
    mAction->setChecked(false);
  if (mButton)
    mButton->setChecked(false);
}

void QgsMapTool::setAction(QAction* action)
{
  mAction = action;
}

QAction* QgsMapTool::action()
{
  return mAction;
}

void QgsMapTool::setButton(QAbstractButton* button)
{
  mButton = button;
}

QAbstractButton* QgsMapTool::button()
{
  return mButton;
}

    
void QgsMapTool::canvasMoveEvent(QMouseEvent *)
{
}

void QgsMapTool::canvasPressEvent(QMouseEvent *)
{
}

void QgsMapTool::canvasReleaseEvent(QMouseEvent *)
{
}

void QgsMapTool::keyPressEvent(QKeyEvent* e)
{
}

void QgsMapTool::renderComplete()
{
}

bool QgsMapTool::isZoomTool()
{
  return false;
}

QgsMapCanvas* QgsMapTool::canvas()
{
  return mCanvas;
}
