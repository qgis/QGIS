/***************************************************************************
          qgsrasterrange.h
     --------------------------------------
    Date                 : Oct 9, 2012
    Copyright            : (C) 2012 by Radim Blazek
    email                : radim dot blazek at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <limits>
#include "qgis.h"
#include "qgsrasterrange.h"

QgsRasterRange::QgsRasterRange()
  : mMin ( std::numeric_limits<double>::quiet_NaN() )
  , mMax ( std::numeric_limits<double>::quiet_NaN() )
{
}

QgsRasterRange::QgsRasterRange( double theMin, double theMax  )
  : mMin ( theMin )
  , mMax ( theMax )
{
}

QgsRasterRange::~QgsRasterRange()
{
}

bool QgsRasterRange::contains( double value, const QgsRasterRangeList &rangeList )
{
  foreach ( QgsRasterRange::QgsRasterRange range, rangeList )
  {
    if (( value >= range.mMin && value <= range.mMax ) ||
        doubleNear( value, range.mMin ) ||
        doubleNear( value, range.mMax ) )
    {
      return true;
    }
  }
  return false;
}

