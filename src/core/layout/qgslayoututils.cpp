/***************************************************************************
                              qgslayoututils.cpp
                              ------------------
    begin                : July 2017
    copyright            : (C) 2017 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgslayoututils.h"
#include <math.h>

double QgsLayoutUtils::normalizedAngle( const double angle, const bool allowNegative )
{
  double clippedAngle = angle;
  if ( clippedAngle >= 360.0 || clippedAngle <= -360.0 )
  {
    clippedAngle = fmod( clippedAngle, 360.0 );
  }
  if ( !allowNegative && clippedAngle < 0.0 )
  {
    clippedAngle += 360.0;
  }
  return clippedAngle;
}
