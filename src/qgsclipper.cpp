/***************************************************************************
                          qgsclipper.h  -  a class that clips line
                          segments and polygons
                             -------------------
    begin                : March 2004
    copyright            : (C) 2005 by Gavin Macaulay
    email                : 
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

#include "qgsclipper.h"

#include <iostream>

bool QgsClipper::trimLine(const QgsPoint& from, const QgsPoint& to, 
			  QgsPoint& tFrom, QgsPoint& tTo)
{
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
    double r_n = (from.y() -     minY) * (maxX - minX);
    double dTB = - (to.y() - from.y()) * (maxX - minX);
    double s_n;

    if (std::abs(dTB) > SMALL_NUM && std::abs(r_n) > SMALL_NUM)
    {
      s_n = (from.y() -     minY)  * (to.x() - from.x()) 
          - (from.x() -     minX)  * (to.y() - from.y());
      double r_nOverd = r_n / dTB;
      double s_nOverd = s_n / dTB;

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
    r_n = -(from.x() - maxX) * (maxY - minY);
    double dLR =  (to.x()   - from.x()) * (maxY   - minY);

    if (std::abs(dLR) > SMALL_NUM && std::abs(r_n) > SMALL_NUM)
    {
      s_n = (from.y()  - minY)     * (to.x() - from.x()) 
          - (from.x()  - maxX)     * (to.y() - from.y());
      double r_nOverd = r_n / dLR;
      double s_nOverd = s_n / dLR;

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
    r_n = - (from.x() - minX) * (maxY - minY);
    // d is the same as dLR

    if (std::abs(dLR) > SMALL_NUM && std::abs(r_n) > SMALL_NUM)
    {
      s_n =   (from.y() - minY)     * (to.x() - from.x()) 
	    - (from.x() - minX)     * (to.y() - from.y());

      double r_nOverd = r_n / dLR;
      double s_nOverd = s_n / dLR;

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
    r_n = (from.y() - maxY) * (maxX - minX);
    // d is the same as for the top boundary check

    if (std::abs(dTB) > SMALL_NUM && std::abs(r_n) > SMALL_NUM)
    {
      s_n = (from.y() - maxY)     * (to.x() - from.x())
          - (from.x() - minX)     * (to.y() - from.y());
      double r_nOverd = r_n/dTB;
      double s_nOverd = s_n/dTB;

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


// Trim the polygon using Sutherland and Hodgman's
// polygon-clipping algorithm. See J. D. Foley, A. van Dam,
// S. K. Feiner, and J. F. Hughes, Computer Graphics, Principles and
// Practice. Addison-Wesley Systems Programming Series,
// Addison-Wesley, 2nd ed., 1991. 

// I understand that this is not the most efficient algorithm, but is
// one (the only?) that is guaranteed to always give the correct
// result. 

void QgsClipper::trimPolygon(QPointArray* pa)
{
  // std::cerr << "Trimming polygon...\n";

  // Trim the polygon against the four boundaries in turn.
  QPointArray* tmp = new QPointArray();
  trimPolygonToBoundary(pa, tmp, Xmax);
  pa->resize(0);
  trimPolygonToBoundary(tmp, pa, Ymax);
  tmp->resize(0);
  trimPolygonToBoundary(pa, tmp, Xmin);
  pa->resize(0);
  trimPolygonToBoundary(tmp, pa, Ymin);
  delete tmp;
  /*
  for (int i = 0; i < pa->size(); ++i)
    std::cerr << pa->point(i).x() << ", "
	      << pa->point(i).y() << "; ";
  std::cerr <<'\n';
  */
}

// An auxilary function that is part of the polygon trimming
// code. Will trim the given polygon to the given boundary and return
// the trimmed polygon in the out pointer. Uses Sutherland and
// Hodgman's polygon-clipping algorithm.

void QgsClipper::trimPolygonToBoundary(QPointArray* in, 
				       QPointArray* out, boundary b)
{
  QPoint i;

  int i1 = in->size()-1; // start with last point

  // and compare to the first point initially.
  for (int i2 = 0; i2 < in->size() ; ++i2)
    { // look at each edge of the polygon in turn
    if (inside(in, i2, b)) // end point of edge is inside boundary
    {
      if (inside(in, i1, b)) // edge is entirely inside boundary, so pass-thru
	out->putPoints(out->size(), 1, in->point(i2).x(), in->point(i2).y());
      else
      {
	// edge crosses into the boundary, so trim back to the boundary, and
	// store both ends of the new edge
	i = intersect(in, i1, i2, b);
 	out->putPoints(out->size(), 1, i.x(), i.y());
	out->putPoints(out->size(), 1, in->point(i2).x(), in->point(i2).y());
      }
    }
    else // end point of edge is outside boundary
    {
      if (inside(in, i1, b))
      { // start point is in boundary, so need to trim back
	i = intersect(in, i1, i2, b);
	out->putPoints(out->size(), 1, i.x(), i.y());
      }
    }
    i1 = i2;
  }
  //  std::cerr << in->size() << " -> " << out->size() << '\n';
}


// An auxilary function to trimPolygonToBoundarY() that returns
// whether a point is inside or outside the given boundary. 

bool QgsClipper::inside(QPointArray* pa, int p, boundary b)
{
  switch (b)
  {
  case Xmax: // x < maxX is inside
    if ((pa->point(p)).x() < maxX)
      return true;
    break;
  case Xmin: // x > minX is inside
    if ((pa->point(p)).x() > minX)
      return true;
    break;
  case Ymax: // y < maxY is inside
    if ((pa->point(p)).y() < maxY)
      return true;
    break;
  case Ymin: // y > minY is inside
    if ((pa->point(p)).y() > minY)
      return true;
    break;
  }
  return false;
}


// An auxilary function to trimPolygonToBoundarY() that calculates and
// returns the intersection of the line defined by the given points
// and the given boundary.

QPoint QgsClipper::intersect(QPointArray* pa, int i1, int i2, boundary b)
{
  // This function assumes that the two given points cross the given
  // boundary. Making this assumption allows some optimisations.

  QPoint p1 = pa->point(i1);
  QPoint p2 = pa->point(i2);
  QPoint p;
  double r_n, r_d;

  /*
  std::cerr << pa->point(i1).x() << ", " << pa->point(i1).y() << ", "
	    << pa->point(i2).x() << ", " << pa->point(i2).y() << '\n';
  */

  switch (b)
  {
  case Xmax: // x = maxX boundary
    r_n = -(p1.x() - maxX)   * (maxY - minY);
    r_d =  (p2.x() - p1.x()) * (maxY - minY);
    break;
  case Xmin: // x = minX boundary
    r_n = -(p1.x() - minX)   * (maxY - minY);
    r_d =  (p2.x() - p1.x()) * (maxY - minY);
    break;
  case Ymax: // y = maxY boundary
    r_n =   (p1.y() - maxY)   * (maxX - minX);
    r_d = - (p2.y() - p1.y()) * (maxX - minX);
    break;
  case Ymin: // y = minY boundary
    r_n =   (p1.y() - minY)   * (maxX - minX);
    r_d = - (p2.y() - p1.y()) * (maxX - minX);
    break;
  }

  if (std::abs(r_d) > SMALL_NUM && std::abs(r_n) > SMALL_NUM)
  { // they cross
    double r = r_n / r_d;
    p.setX(static_cast<int>(round(p1.x() + r*(p2.x() - p1.x()))));
    p.setY(static_cast<int>(round(p1.y() + r*(p2.y() - p1.y()))));
  }
  else
  {
    // Should never get here, but if we do for some reason, cause a
    // clunk because something else is wrong if we do.
    Q_ASSERT(std::abs(r_d) > SMALL_NUM && std::abs(r_n) > SMALL_NUM);
  }

  return p;
}


