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
#include <cmath>
#include <limits>

QgsIDWInterpolator::QgsIDWInterpolator( const QList<LayerData>& layerData ): QgsInterpolator( layerData ), mDistanceCoefficient( 2.0 )
{

}

QgsIDWInterpolator::QgsIDWInterpolator(): QgsInterpolator( QList<LayerData>() ), mDistanceCoefficient( 2.0 )
{

}

QgsIDWInterpolator::~QgsIDWInterpolator()
{

}

int QgsIDWInterpolator::interpolatePoint( double x, double y, double& result )
{
  if ( !mDataIsCached )
  {
    cacheBaseData();
  }

  double currentWeight;
  double distance;

  double sumCounter = 0;
  double sumDenominator = 0;

  Q_FOREACH ( const vertexData& vertex_it, mCachedBaseData )
  {
    distance = sqrt(( vertex_it.x - x ) * ( vertex_it.x - x ) + ( vertex_it.y - y ) * ( vertex_it.y - y ) );
    if (( distance - 0 ) < std::numeric_limits<double>::min() )
    {
      result = vertex_it.z;
      return 0;
    }
    currentWeight = 1 / ( pow( distance, mDistanceCoefficient ) );
    sumCounter += ( currentWeight * vertex_it.z );
    sumDenominator += currentWeight;
  }

  if ( sumDenominator == 0.0 )
  {
    return 1;
  }

  result = sumCounter / sumDenominator;
  return 0;
}
