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

#include "qgsrasterrange.h"

QgsRasterRange::QgsRasterRange( double min, double max, BoundsType bounds )
  : mMin( min )
  , mMax( max )
  , mType( bounds )
{
}

bool QgsRasterRange::contains( double value, const QgsRasterRangeList &rangeList )
{
  Q_FOREACH ( QgsRasterRange range, rangeList )
  {
    if ( ( value >= range.mMin && value <= range.mMax ) ||
         qgsDoubleNear( value, range.mMin ) ||
         qgsDoubleNear( value, range.mMax ) )
    {
      return true;
    }
  }
  return false;
}

