/***************************************************************************
                qgscoordinatetransform.cpp  -  description
                             -------------------
    begin                : Sat Jun 22 2002
    copyright            : (C) 2002 by Gary E.Sherman
    email                : sherman@mrcc.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include <qstring.h>
#include <qtextstream.h>

#include "qgspoint.h"
#include "qgscoordinatetransform.h"

QgsCoordinateTransform::QgsCoordinateTransform(double mupp=0, double ymax = 0, double ymin=0, double xmin = 0) : 
  mapUnitsPerPixel(mupp), yMax(ymax), yMin(ymin), xMin(xmin){
}
QgsCoordinateTransform::~QgsCoordinateTransform(){
}
QgsPoint QgsCoordinateTransform::transform(QgsPoint p){
  // transform x
  double dx = (p.x() - xMin)/mapUnitsPerPixel;
  double dy = (yMax - (p.y() - yMin))/mapUnitsPerPixel;
  return QgsPoint(dx,dy);
}

QgsPoint QgsCoordinateTransform::transform(double x, double y){
  return(transform(QgsPoint(x,y)));
}
void QgsCoordinateTransform::setMapUnitsPerPixel(double mupp){
  mapUnitsPerPixel = mupp;
}

void QgsCoordinateTransform::setYmax(double ymax){
  yMax = ymax;
}
void QgsCoordinateTransform::setYmin(double ymin){
  yMin = ymin;
}
void QgsCoordinateTransform::setXmin(double xmin){
  xMin = xmin;
}
void QgsCoordinateTransform::setParameters(double mupp, double xmin,
					   double ymin, double ymax){
 mapUnitsPerPixel = mupp;
 xMin = xmin;
 yMin = ymin;
 yMax = ymax;

}
QString QgsCoordinateTransform::showParameters(){
  QString rep;
  QTextOStream(&rep) << "Map units/pixel: " << mapUnitsPerPixel 
		     << " X minimum: " << xMin 
		     << " Y minimum: " << yMin
		     << " Y maximum: " << yMax;
  return rep;
		 
}
