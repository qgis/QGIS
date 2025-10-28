/***************************************************************************
    qgsmathutils.cpp
    ----------------------
    begin                : July 2025
    copyright            : (C) 2025 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include "qgsmathutils.h"
#include "qgis.h"
#include "moc_qgsmathutils.cpp"

void QgsMathUtils::doubleToRational( double value, qlonglong &numerator, qlonglong &denominator, double tolerance, int maxIterations )
{
  // This method uses the "continued fraction" algorithm to calculate approximate rational fractions

  if ( qgsDoubleNear( value, 0.0 ) )
  {
    numerator = 0;
    denominator = 1;
    return;
  }

  const int sign = ( value > 0 ) ? 1 : -1;
  const double x = std::abs( value );

  // convergents:
  // numerator continuants
  long long previousAConvergent = 0;
  long long currentAConvergent = 1;
  // denominator continuants
  long long previousBConvergent = 1;
  long long currentBConvergent = 0;

  double fractionalPart = x;
  const double relativeTolerance = tolerance * x;

  for ( int i = 0; i < maxIterations; ++i )
  {
    long long a = static_cast< long long >( std::floor( fractionalPart ) );

    // guard against overflows before continuing, if so, abort early
    if ( currentAConvergent != 0 && a > ( std::numeric_limits<long long>::max() - previousAConvergent ) / currentAConvergent )
    {
      break;
    }
    if ( currentBConvergent != 0 && a > ( std::numeric_limits<long long>::max() - previousBConvergent ) / currentBConvergent )
    {
      break;
    }

    long long nextHConvergent = a * currentAConvergent + previousAConvergent;
    long long nextKConvergent = a * currentBConvergent + previousBConvergent;
    previousAConvergent = currentAConvergent;
    previousBConvergent = currentBConvergent;
    currentAConvergent = nextHConvergent ;
    currentBConvergent = nextKConvergent;

    // is approximation within the specified tolerance?
    if ( currentBConvergent != 0 && std::abs( x - static_cast<double>( currentAConvergent ) / static_cast<double>( currentBConvergent ) ) <= relativeTolerance )
    {
      // if so, we've found our answer...
      break;
    }

    // update the fractional part for the next iteration
    const double remainder = fractionalPart - static_cast< double >( a );
    if ( qgsDoubleNear( remainder, 0.0 ) )
    {
      // found exact representation
      break;
    }
    fractionalPart = 1.0 / remainder;
  }

  numerator = sign * currentAConvergent;
  denominator = currentBConvergent;
}
