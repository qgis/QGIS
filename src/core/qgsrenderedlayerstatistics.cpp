/***************************************************************************
    qgsrenderedlayerstatistics.cpp
    ----------------
    copyright            : (C) 2024 by Jean Felder
    email                : jean dot felder at oslandia dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsrenderedlayerstatistics.h"

QgsRenderedLayerStatistics::QgsRenderedLayerStatistics( const QString &layerId, const QList<double> &minimum, const QList<double> &maximum )
  : QgsRenderedItemDetails( layerId )
  , mMin( minimum )
  , mMax( maximum )
{

}

QgsRenderedLayerStatistics::QgsRenderedLayerStatistics( const QString &layerId, double minimum, double maximum )
  : QgsRenderedItemDetails( layerId )
  , mMin( {minimum} )
, mMax( {maximum} )
{

}

QList<double> QgsRenderedLayerStatistics::minimum() const
{
  return mMin;
}

double QgsRenderedLayerStatistics::minimum( int index ) const
{
  if ( index < 0 || index >= mMin.size() )
  {
    return std::numeric_limits<double>::quiet_NaN();
  }

  return mMin.at( index );
}

QList<double> QgsRenderedLayerStatistics::maximum() const
{
  return mMax;
}

double QgsRenderedLayerStatistics::maximum( int index ) const
{
  if ( index < 0 || index >= mMax.size() )
  {
    return std::numeric_limits<double>::quiet_NaN();
  }

  return mMax.at( index );
}


void QgsRenderedLayerStatistics::setMinimum( QList<double> &minimum )
{
  mMin = minimum;
}

bool QgsRenderedLayerStatistics::setMinimum( int index, double minimum )
{
  if ( index < 0 || index >= mMax.size() )
  {
    return false;
  }

  mMin[index] = minimum;
  return true;
}

void QgsRenderedLayerStatistics::setMaximum( QList<double> &maximum )
{
  mMax = maximum;
}

bool QgsRenderedLayerStatistics::setMaximum( int index, double maximum )
{
  if ( index < 0 || index >= mMax.size() )
  {
    return false;
  }

  mMax[index] = maximum;
  return true;
}
