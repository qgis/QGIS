/***************************************************************************
    qgsrubberband.cpp - Rubberband widget for drawing multilines and polygons
     --------------------------------------
    Date                 : 07-Jan-2006
    Copyright            : (C) 2006 by Tom Elwertowski
    Email                : telwertowski at users dot sourceforge dot net
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
/* $Id$ */

#include "qgsrubberband.h"
#include <QPainter>

/*!
  \class QgsRubberBand
  \brief The QgsRubberBand class provides a transparent overlay widget
  for tracking the mouse while drawing polylines or polygons.
*/
QgsRubberBand::QgsRubberBand(QWidget * parent, bool isPolygon)
: QWidget(parent), mIsPolygon(isPolygon)
{
  setGeometry(parent->rect()); // this widget is same size as parent
  mPoints.append(QPoint()); // addPoint assumes an initial allocated point
  setColor(QColor(Qt::lightGray));
}

QgsRubberBand::~QgsRubberBand()
{}

/*!
  Set the outline and fill color.
*/
void QgsRubberBand::setColor(const QColor & color)
{
  mPen.setColor(color);
  QColor fillColor(color.red(), color.green(), color.blue(), 63);
  mBrush.setColor(fillColor);
  mBrush.setStyle(Qt::SolidPattern);
}

/*!
  Set the outline width.
*/
void QgsRubberBand::setWidth(int width)
{
  mPen.setWidth(width);
}

/*!
  Remove all points from the shape being created.
*/
void QgsRubberBand::reset(bool isPolygon)
{
  mPoints.resize(1); // addPoint assumes an initial allocated point
  mIsPolygon = isPolygon;
  update();
}

/*!
  Add a point to the shape being created.
*/
void QgsRubberBand::addPoint(const QPoint & p)
{
  mPoints.last() = p; // Current mouse position becomes added point
  mPoints.append(p); // Allocate new point to continue tracking current mouse position
  update();
}

/*!
  Update the line between the last added point and the mouse position.
*/
void QgsRubberBand::movePoint(const QPoint & p)
{
  mPoints.last() = p; // Update current mouse position
  update();
}

/*!
  Draw the shape in response to an update event.
*/
void QgsRubberBand::paintEvent(QPaintEvent * event)
{
  if (mPoints.count() > 1)
  {
    QPainter p(this);
    p.setPen(mPen);
    p.setBrush(mBrush);
    if (mIsPolygon)
    {
      p.drawPolygon(mPoints);
    }
    else
    {
      p.drawPolyline(mPoints);
    }
  }
}
