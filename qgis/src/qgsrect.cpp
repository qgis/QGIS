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

QgsRect::QgsRect(QgsPoint p1, QgsPoint p2)
{
  xmin = p1.x();
  xmax = p2.x();
  ymin = p1.y();
  ymax = p2.y();
  normalize();
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
  intersection.setXmin(xmin > ? rect->xMin());
  intersection.setYmin(ymin > ? rect->yMin());
  intersection.setXmax(xmax < ? rect->xMax());
  intersection.setYmax(ymax < ? rect->yMax());
  return intersection;
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

QString QgsRect::stringRep() const const
{
  QString tmp;
  QString rep = tmp.setNum(xmin);;
  rep += " ";
  rep += tmp.setNum(ymin);
  rep += ",";
  rep += tmp.setNum(xmax);
  rep += " ";
  rep += tmp.setNum(ymax);
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
