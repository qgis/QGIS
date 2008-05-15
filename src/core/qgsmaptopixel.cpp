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
#include "qgsmaptopixel.h"
#include <QPoint>
#include <QTextStream>

QgsMapToPixel::QgsMapToPixel(double mupp, 
				    double ymax,
				    double ymin, 
				    double xmin)
  : mMapUnitsPerPixel(mupp), 
     yMax(ymax), 
     yMin(ymin), 
     xMin(xmin),
     xMax(0)                   // XXX wasn't originally specified?  Why?
{
}

QgsMapToPixel::~QgsMapToPixel()
{
}

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


QgsPoint QgsMapToPixel::transform(double x, double y)
{
  transformInPlace(x,y);
  return QgsPoint(x,y);
}

QgsPoint QgsMapToPixel::transform(const QgsPoint& p)
{
  double dx = p.x();
  double dy = p.y();
  transformInPlace(dx, dy);

  //std::cerr << "Point to pixel...X : " << p.x() << "-->" << dx << ", Y: " << p.y() << " -->" << dy << std::endl;
  return QgsPoint(dx, dy);
}

void QgsMapToPixel::transform(QgsPoint* p)
{   
  double x = p->x();
  double y = p->y();
  transformInPlace(x, y);

#ifdef QGISDEBUG 
    //std::cerr << "Point to pixel...X : " << p->x() << "-->" << x << ", Y: " << p->y() << " -->" << y << std::endl;
#endif     
  p->set(x,y);
}

void QgsMapToPixel::transformInPlace(double& x, double& y)
{
  x = (x - xMin) / mMapUnitsPerPixel;
  y = yMax - (y - yMin) / mMapUnitsPerPixel;
}

void QgsMapToPixel::transformInPlace(std::vector<double>& x, 
					    std::vector<double>& y)
{
  assert(x.size() == y.size());
  for (unsigned int i = 0; i < x.size(); ++i)
    transformInPlace(x[i], y[i]);
}
