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

  // Trims the given line segment to a rectangular box. Returns the
  // trimmed line segement in tFrom and tTo.
  static bool trimLine(const QgsPoint& from, const QgsPoint& to, 
		       QgsPoint& tFrom, QgsPoint& tTo);

  // Trims the given polygon to a rectangular box. Returns the trimmed
  // polygon in the given QPointArray.
  //  static void trimPolygon(QPointArray* pa);

 private:

};


#endif
