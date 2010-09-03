/***************************************************************************
                          qgsclipper.cpp  -  a class that clips line
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

// Where has all the code gone?

// It's been inlined, so its in the qgsclipper.h file.

// But the static members must be initialised outside the class! (or GCC 4 dies)

// Qt also does clipping when the coordinates go over +/- 32767
// moreover from Qt 4.6, Qt clips also when the width/height of a painter path
// is more than 32767. Since we want to avoid clipping by Qt (because it is slow)
// we set coordinate limit to less than 32767 / 2
const double QgsClipper::MAX_X =  16000;
const double QgsClipper::MIN_X = -16000;
const double QgsClipper::MAX_Y =  16000;
const double QgsClipper::MIN_Y = -16000;

const double QgsClipper::SMALL_NUM = 1e-12;
