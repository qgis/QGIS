/***************************************************************************
  qgsspatialindexutils.cpp
  ------------------------
  Date                 : December 2019
  Copyright            : (C) 2019 by Nyall Dawson
  Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsspatialindexutils.h"
#include "qgsrectangle.h"
#include <spatialindex/SpatialIndex.h>

SpatialIndex::Region QgsSpatialIndexUtils::rectangleToRegion( const QgsRectangle &rectangle )
{
  double pt1[2] = { rectangle.xMinimum(), rectangle.yMinimum() };
  double pt2[2] = { rectangle.xMaximum(), rectangle.yMaximum() };
  return SpatialIndex::Region( pt1, pt2, 2 );
}
