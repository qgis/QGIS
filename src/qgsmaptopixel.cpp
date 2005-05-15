/***************************************************************************
                qgsmaptopixel.cpp  -  description
                             -------------------
    begin                : Sat Jun 22 2002
    copyright            : (C) 2002 by Gary E.Sherman
    email                : sherman at mrcc.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
 /* $Id$ */
#include <qstring.h>
#include <qtextstream.h>
#include <qpoint.h>
#include "qgsmaptopixel.h"

QgsPoint QgsMapToPixel::toMapPoint(int x, int y)
{
  double mx = x * mMapUnitsPerPixel + xMin;
  double my = -1 * ((y - yMax) * mMapUnitsPerPixel - yMin);
  return QgsPoint(mx, my);
}

QgsPoint QgsMapToPixel::toMapCoordinates(QPoint p)
{
  QgsPoint mapPt = toMapPoint(p.x(), p.y());
  return QgsPoint(mapPt);
}

QgsPoint QgsMapToPixel::toMapCoordinates(int x, int y)
{
  return toMapPoint(x, y);
}

void QgsMapToPixel::setMapUnitsPerPixel(double mupp)
{
  mMapUnitsPerPixel = mupp;
}

double QgsMapToPixel::mapUnitsPerPixel()
{
  return mMapUnitsPerPixel;
}

void QgsMapToPixel::setYmax(double ymax)
{
  yMax = ymax;
}

void QgsMapToPixel::setYmin(double ymin)
{
  yMin = ymin;
}

void QgsMapToPixel::setXmin(double xmin)
{
  xMin = xmin;
}

void QgsMapToPixel::setParameters(double mupp, double xmin, double ymin, double ymax)
{
  mMapUnitsPerPixel = mupp;
  xMin = xmin;
  yMin = ymin;
  yMax = ymax;

}

QString QgsMapToPixel::showParameters()
{
  QString rep;
  QTextOStream(&rep) << "Map units/pixel: " << mMapUnitsPerPixel
    << " X minimum: " << xMin << " Y minimum: " << yMin << " Y maximum: " << yMax;
  return rep;

}
