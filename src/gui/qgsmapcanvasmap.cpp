/***************************************************************************
    qgsmapcanvasmap.cpp  -  draws the map in map canvas
    ----------------------
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

#include "qgsmapcanvasmap.h"
#include "qgsmaprender.h"

#include <QPainter>

QgsMapCanvasMap::QgsMapCanvasMap(Q3Canvas *canvas, QgsMapRender* render)
  : Q3CanvasRectangle(canvas), mRender(render)
{
  setZ(-10);
  move(0,0);
  resize(QSize(1,1));
}


void QgsMapCanvasMap::drawShape(QPainter & p)
{
  p.drawPixmap(mOffset.x(), mOffset.y(), mPixmap);
}

void QgsMapCanvasMap::resize(QSize size)
{
  setSize(size.width(), size.height());
  mPixmap = QPixmap(size);
  mRender->setOutputSize(size, mPixmap.logicalDpiX());
}

void QgsMapCanvasMap::render()
{
  // use temporary image for rendering
  QImage image(size(), QImage::Format_RGB32);
  
  image.fill(mBgColor.rgb());

  QPainter paint;
  paint.begin(&image);

  // antialiasing
  if (mAntiAliasing)
    paint.setRenderHint(QPainter::Antialiasing);
  
  mRender->render(&paint);

  paint.end();
  
  // convert QImage to QPixmap to acheive faster drawing on screen
  mPixmap = QPixmap::fromImage(image);
}
