/***************************************************************************
                          qgsrect.cpp  -  description
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
#include <iostream>
#include <qstring.h>
#include "qgspoint.h"
#include "qgsrect.h"

QgsRect::QgsRect(QgsPoint const & p1, QgsPoint const & p2)
{
  xmin = p1.x();
  xmax = p2.x();
  ymin = p1.y();
  ymax = p2.y();
  normalize();
}

QgsRect::QgsRect(const QgsRect &r){
  xmin = r.xMin();
  ymin = r.yMin();
  xmax = r.xMax();
  ymax = r.yMax();
}

void QgsRect::normalize()
{
  double temp;
  if (xmin > xmax)
    {
      temp = xmin;
      xmin = xmax;
      xmax = temp;
    }
  if (ymin > ymax)
    {
      temp = ymin;
      ymin = ymax;
      ymax = temp;
    }
}
void QgsRect::scale(double scaleFactor, QgsPoint * cp)
{
  // scale from the center
  double centerX, centerY;
  if (cp)
    {
      centerX = cp->x();
      centerY = cp->y();
  } else
    {
      centerX = xmin + width() / 2;
      centerY = ymin + height() / 2;
    }
  double newWidth = width() * scaleFactor;
  double newHeight = height() * scaleFactor;
  xmin = centerX - newWidth / 2.0;
  xmax = centerX + newWidth / 2.0;
  ymin = centerY - newHeight / 2.0;
  ymax = centerY + newHeight / 2.0;
}
void QgsRect::expand(double scaleFactor, QgsPoint * cp)
{
  // scale from the center
  double centerX, centerY;
  if (cp)
    {
      centerX = cp->x();
      centerY = cp->y();
  } else
    {
      centerX = xmin + width() / 2;
      centerY = ymin + height() / 2;
    }

  double newWidth = width() * scaleFactor;
  double newHeight = height() * scaleFactor;
  xmin = centerX - newWidth;
  xmax = centerX + newWidth;
  ymin = centerY - newHeight;
  ymax = centerY + newHeight;
}

QgsRect QgsRect::intersect(QgsRect * rect)
{
  QgsRect intersection = QgsRect();
  
  intersection.setXmin(xmin > rect->xMin()?xmin:rect->xMin());
  intersection.setXmax(xmax < rect->xMax()?xmax:rect->xMax());
  intersection.setYmin(ymin > rect->yMin()?ymin:rect->yMin());
  intersection.setYmax(ymax < rect->yMax()?ymax:rect->yMax());
  return intersection;
}

void QgsRect::combineExtentWith(QgsRect * rect)
{
 
  xmin = ( (xmin < rect->xMin())? xmin : rect->xMin() );
  xmax = ( (xmax > rect->xMax())? xmax : rect->xMax() );

  ymin = ( (ymin < rect->yMin())? ymin : rect->yMin() );
  ymax = ( (ymax > rect->yMax())? ymax : rect->yMax() );

}

bool QgsRect::isEmpty()
{
  if (xmax <= xmin || ymax <= ymin)
    {
      return TRUE;
  } else
    {
      return FALSE;
    }
}
// Return a string representation of the rectangle with high precision
QString QgsRect::stringRep() const
{
  QString tmp;
  QString rep = tmp.sprintf("%16f %16f,%16f %16f", xmin, ymin, xmax, ymax);
  return rep;
}

// overloaded version of above fn to allow precision to be set
// Return a string representation of the rectangle with high precision
QString QgsRect::stringRep(int thePrecision) const
{
  
  QString rep = QString::number(xmin,'f',thePrecision) + 
                QString(",") +
                QString::number(ymin,'f',thePrecision) +
                QString(" : ") +
                QString::number(xmax,'f',thePrecision) +
                QString(",") +
                QString::number(ymax,'f',thePrecision) ;
#ifdef QGISDEBUG
  std::cout << "Extents : " << rep << std::endl;
#endif    
  return rep;
}
// Return the rectangle as a set of polygon coordinates
QString QgsRect::asPolygon() const
{
  QString tmp;
  QString rep = tmp.sprintf("%16f %16f,%16f %16f,%16f %16f,%16f %16f,%16f %16f",
    xmin, ymin, xmin, ymax, xmax, ymax, xmax, ymin, xmin, ymin);
    return rep;
    
}
bool QgsRect::operator==(const QgsRect & r1)
{
  return (r1.xMax() == this->xMax() && r1.xMin() == this->xMin() && r1.yMax() == this->yMax() && r1.yMin() == this->yMin());
}

QgsRect & QgsRect::operator=(const QgsRect & r)
{
  if (&r != this)
    {
      xmax = r.xMax();
      xmin = r.xMin();
      ymax = r.yMax();
      ymin = r.yMin();
    }
  return *this;


}
