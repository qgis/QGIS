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
  double mx = x * mapUnitsPerPixel + xMin;
  double my = -1 * ((y - yMax) * mapUnitsPerPixel - yMin);
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
  mapUnitsPerPixel = mupp;
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
  mapUnitsPerPixel = mupp;
  xMin = xmin;
  yMin = ymin;
  yMax = ymax;

}

QString QgsMapToPixel::showParameters()
{
  QString rep;
  QTextOStream(&rep) << "Map units/pixel: " << mapUnitsPerPixel
    << " X minimum: " << xMin << " Y minimum: " << yMin << " Y maximum: " << yMax;
  return rep;

}

bool QgsMapToPixel::trimLine(const QgsPoint& from, const QgsPoint& to, 
			     QgsPoint& tFrom, QgsPoint& tTo)
{
  // The limits for X11 screen coordinates. Actual value is 32767, but
  // we allow a little bit of space for rounding errors in the
  // calculations below.
  static const double minX = -32760;
  static const double maxX =  32760;
  static const double minY = -32760;
  static const double maxY =  32760;
  static const double SMALL_NUM = 1e-6;

  // To determine the intersection between a line given by the points
  // A and B, and the line given by the points C and D, calculate
  //
  //     (Ay - Cy)(Dx - Cx) - (Ax - Cx)(Dy - Cy)
  // r = -----------------------------------------
  //     (Bx - Ax)(Dy - Cy) - (By - Ay)(Dx - Cx)
  //
  //     (Ay - Cy)(Bx - Ax) - (Ax - Cx)(By - Ay)
  // s = -----------------------------------------
  //     (Bx - Ax)(Dy - Cy) - (By - Ay)(Dx - Cx)
  //
  // If the demoninator is 0, the lines are parallel, and don't
  // intersect, and can be left untrimmed
  // If the numerator is 0 too, the lines are collinear and can be
  // left untrimmed.
  // 
  // Note that the demoninator is the same for r and s.
  //
  // If the two points that define the line segment are actually at
  // the same place, the denominator will be zero and the points will
  // be left alone as per parallel lines. This shouldn't happen anyway
  // because if the line is just a point, and it's within the visible
  // window on screen, the coordinates will be within the limits and
  // the first if() test below will fail and not attempt any trimming.
  //
  // If 0 >= r <= 1 and 0 >= s <= 1 then the line segments intersect,
  // and the intersection point P is given by:
  //
  // P = A + r*(B-A)
  //
  // Also do a check to see if the line segment is inside or outside
  // the limits. If outside, return false to let the caller know that
  // the shouldn't plot the line segment. 
  // 
  // This function has more than one exit point - as soon as the code
  // determines that it has trimmed the lines appropriately, it exits,
  // to reduce CPU effort.

  // Once we have adjusted both the to and from points, there is no
  // point in doing further checks, so return from the function
  // if that occurs.
  bool toDone = false, fromDone = false;

  // Check for the need to trim first
  if (from.x() < minX || from.x() > maxX || to.x() < minX || to.x() > maxX ||
      from.y() < minY || from.y() > maxY || to.y() < minY || to.y() > maxY)
  {
    tFrom = from;
    tTo = to;
    
    // Check the top boundary
    double r_n = (from.y() -     minY)  * (maxX - minX);
    double   d = - (to.y() - from.y())  * (maxX - minX);
    double s_n;

    if (fabs(d) > SMALL_NUM && fabs(r_n) > SMALL_NUM)
    {
      s_n = (from.y() -     minY)  * (to.x() - from.x()) 
          - (from.x() -     minX)  * (to.y() - from.y());
      double r_nOverd = r_n / d;
      double s_nOverd = s_n / d;
      if (r_nOverd >= 0.0 && r_nOverd <= 1.0 &&
	  s_nOverd >= 0.0 && s_nOverd <= 1.0)
      {
	// intersects the top line. Trim back.
	// work out which end to trim
	if (from.y() <= minY) // trim from
	{
	  fromDone = true;
	  tFrom.setX(from.x() + r_nOverd*(to.x() - from.x()));
	  tFrom.setY(from.y() + r_nOverd*(to.y() - from.y()));
	}
	else // trim to
	{
	  toDone = true;
	  tTo.setX(from.x() + r_nOverd*(to.x() - from.x()));
	  tTo.setY(from.y() + r_nOverd*(to.y() - from.y()));
	}
      }
    }

    // the right border
    r_n = -(from.x() - maxX)     * (maxY   - minY);
      d =  (to.x()   - from.x()) * (maxY   - minY);

    if (fabs(d) > SMALL_NUM && fabs(r_n) > SMALL_NUM)
    {
      s_n = (from.y()  - minY)     * (to.x() - from.x()) 
          - (from.x()  - maxX)     * (to.y() - from.y());
      double r_nOverd = r_n / d;
      double s_nOverd = s_n / d;
      if (r_nOverd >= 0.0 && r_nOverd <= 1.0 &&
	  s_nOverd >= 0.0 && s_nOverd <= 1.0)
      {
	// intersects the bottom line. Trim back.
	// work out which end to trim
	if (from.x() >= maxX) // trim from
	{
	  fromDone = true;
	  tFrom.setX(from.x() + r_nOverd*(to.x() - from.x()));
	  tFrom.setY(from.y() + r_nOverd*(to.y() - from.y()));
	}
	else // trim to
	{
	  toDone = true;
	  tTo.setX(from.x() + r_nOverd*(to.x() - from.x()));
	  tTo.setY(from.y() + r_nOverd*(to.y() - from.y()));
	}
      }
    }

    // Done both ends of the line, so leave.
    if (toDone && fromDone)
      return true;

    // the left border
    r_n = - (from.x() - minX)     * (maxY   - minY);
      d =   (to.x()   - from.x()) * (maxY   - minY);

    if (fabs(d) > SMALL_NUM && fabs(r_n) > SMALL_NUM)
    {
      s_n =   (from.y() - minY)     * (to.x() - from.x()) 
	    - (from.x() - minX)     * (to.y() - from.y());
      double r_nOverd = r_n / d;
      double s_nOverd = s_n / d;
      if (r_nOverd >= 0.0 && r_nOverd <= 1.0 &&
	  s_nOverd >= 0.0 && s_nOverd <= 1.0)
      {
	// intersects the left line. Trim back.
	// work out which end to trim
	if (from.x() <= minX) // trim from
	{
	  fromDone = true;
	  tFrom.setX(from.x() + r_nOverd*(to.x() - from.x()));
	  tFrom.setY(from.y() + r_nOverd*(to.y() - from.y()));
	}
	else // trim to
	{
	  toDone = true;
	  tTo.setX(from.x() + r_nOverd*(to.x() - from.x()));
	  tTo.setY(from.y() + r_nOverd*(to.y() - from.y()));
	}
      }
    }

    // Done both ends of the line, so leave.
    if (toDone && fromDone)
      return true;

    // the bottom border
    r_n = (from.y() - maxY)     * (maxX   - minX);
      d = - (to.y() - from.y()) * (maxX   - minX);

    if (fabs(d) > SMALL_NUM && fabs(r_n) > SMALL_NUM)
    {
      s_n = (from.y() - maxY)     * (to.x() - from.x())
          - (from.x() - minX)     * (to.y() - from.y());
      double r_nOverd = r_n/d;
      double s_nOverd = s_n/d;
      if (r_nOverd >= 0.0 && r_nOverd <= 1.0 &&
	  s_nOverd >= 0.0 && s_nOverd <= 1.0)
      {
	// intersects the bottom line. Trim back.
	// work out which end to trim
	if (from.y() >= maxY) // trim from
	{
	  fromDone = true;
	  tFrom.setX(from.x() + r_nOverd*(to.x() - from.x()));
	  tFrom.setY(from.y() + r_nOverd*(to.y() - from.y()));
	}
	else // trim to
	{
	  toDone = true;
	  tTo.setX(from.x() + r_nOverd*(to.x() - from.x()));
	  tTo.setY(from.y() + r_nOverd*(to.y() - from.y()));
	}
      }
    }
    // If the line hasn't been trimmed yet, it is entirely outside the
    // boundary, so tell the calling code.
    if (!toDone && !fromDone)
      return false;
  }
  else
  {
    // The entire line is visible on screen, so do nothing.
    tFrom = from;
    tTo = to;
  }

  // Too verbose for QGISDEBUG, but handy sometimes.
  /*  
  std::cerr << "Point 1 trimmed from " << from.x() << ", " << from.y() 
	    << " to " << tFrom.x() << ", " << tFrom.y() << '\n'
	    << "Point 2 trimmed from " << to.x() << ", " << to.y() 
	    << " to " << tTo.x() << ", " << tTo.y() << "\n\n";
  */
  return true;
}
