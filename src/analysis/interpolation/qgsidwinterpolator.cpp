/***************************************************************************
                              qgsidwinterpolator.cpp
                              ----------------------
  begin                : Marco 10, 2008
  copyright            : (C) 2008 by Marco Hugentobler
  email                : marco dot hugentobler at karto dot baug dot ethz dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsidwinterpolator.h"
#include "qgis.h"
#include <cmath>
#include <limits>

QgsIDWInterpolator::QgsIDWInterpolator( const QList<LayerData> &layerData )
  : QgsInterpolator( layerData )
{}

int QgsIDWInterpolator::interpolatePoint( double x, double y, double &result, QgsFeedback *feedback )
{
  if ( !mDataIsCached )
  {
    cacheBaseData( feedback );
  }

  double sumCounter = 0;
  double sumDenominator = 0;

  for ( const QgsInterpolatorVertexData &vertex : qgis::as_const( mCachedBaseData ) )
  {
    double distance = std::sqrt( ( vertex.x - x ) * ( vertex.x - x ) + ( vertex.y - y ) * ( vertex.y - y ) );
    if ( qgsDoubleNear( distance, 0.0 ) )
    {
      result = vertex.z;
      return 0;
    }
    double currentWeight = 1 / ( std::pow( distance, mDistanceCoefficient ) );
    sumCounter += ( currentWeight * vertex.z );
    sumDenominator += currentWeight;
  }

  if ( sumDenominator == 0.0 )
  {
    return 1;
  }

  result = sumCounter / sumDenominator;
  return 0;
}
