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

#ifndef QGSCLIPPER_H
#define QGSCLIPPER_H

#include "qgspoint.h"
#include "qpointarray.h"

// The functions in this class are likely to be called from within a
// render loop and hence need to as CPU efficient as possible. 

// The main purpose of the functions in this class are to trim lines
// and polygons to lie within a rectangular region. This is necessary
// for drawing items to an X11 display which have a limit on the
// magnitude of the screen coordinates (+/- 32768, i.e. 16 bit integer).

class QgsClipper
{
 public:

  // Coordinates of the rectangular box that we trim to.
  //
  // These are the limits for X11 screen coordinates. The actual
  // values are +/-32767, but we allow a little bit of space for
  // rounding errors. 
  static const double minX = -32760;
  static const double maxX =  32760;
  static const double minY = -32760;
  static const double maxY =  32760;

  // Used when testing for equivalance to 0.0
  static const double SMALL_NUM = 1e-6;

  // A handy way to refer to the four boundaries
  enum boundary {Xmax, Xmin, Ymax, Ymin};

  // Trims the given line segment to a rectangular box. Returns the
  // trimmed line segement in tFrom and tTo.
  static bool trimLine(const QgsPoint& from, const QgsPoint& to, 
		       QgsPoint& tFrom, QgsPoint& tTo);

  // Trims the given polygon to a rectangular box. Returns the trimmed
  // polygon in the given QPointArray.
  static void trimPolygon(QPointArray* pa);

 private:

  // Trims the given polygon to the given boundary. Returns the
  // trimmed polygon in the out pointer.
  static void trimPolygonToBoundary(QPointArray* in,
				    QPointArray* out,
				    boundary b);

  // Determines if a point is inside or outside a boundary
  static bool inside(QPointArray* pa, int p, boundary b);

  // Calculates the intersection point between a line defined by a
  // line segment and a boundary
  static QPoint intersect(QPointArray* pa, int i1, int i2, boundary b);

};


#endif
