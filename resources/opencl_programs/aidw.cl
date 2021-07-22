/***************************************************************************
  aidw.cl - AIDW Interpolation

  IDW calculator kernel

 ---------------------
 begin                : 19.7.2021
 copyright            : (C) 2021 by Alessandro Pasotti
 email                : elpaso at itopen dot it
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

// Double precision is required
#pragma OPENCL EXTENSION cl_khr_fp64 : enable

// Calculate the distances
__kernel void idw(
    double xStep,// constant
    double yStep,// constant
    double xMin, // constant
    double yMin, // variable on each iteration
    int width,   // column count constant
    unsigned long featureCount, // constant
    double coefficient, // constant
    __global double *dataBuffer,   // data buffer array of (x, y, z) double records
    __global double *resultBuffer  // result buffer array of doubles
)
{

  // Get the index of the current element
  const int i = get_global_id(0);

  const double2 cellCenter = (double2)( xMin + xStep * i, yMin );

  double sumCounter = 0;
  double sumDenominator = 0;

  for( int featureIdx = 0; featureIdx < featureCount; ++featureIdx )
  {
    const ulong index = 3 * featureIdx;
    const double2 dataPoint = (double2)( dataBuffer[ index ], dataBuffer[ index + 1 ]);
    const double z =  dataBuffer[ index + 2 ];
    // printf( "Data: %f %f %f\ncellCenter: %f %f\n", dataPoint.x, dataPoint.y, z, cellCenter.x, cellCenter.y);
    const double dist = distance( cellCenter, dataPoint );
    if ( dist != 0 )
    {
      const double currentWeight = (double) 1.0 / native_powr( dist, coefficient );
      sumCounter += ( currentWeight * z );
      sumDenominator += currentWeight;
      // printf( "Dist: %f Current w: %f, z: %f\n", dist, currentWeight, z );
    }
    else
    {
      // printf( "Result Z (%f, %f) %f\n", cellCenter.x, cellCenter.y, z );
      resultBuffer[ i ] = z;
      return ;
    }
  }

  if ( sumDenominator != 0.0 )
  {
    // printf( "Result %d (%f, %f) %f\n", i, cellCenter.x, cellCenter.y, sumCounter / sumDenominator );
    resultBuffer[ i ] = sumCounter / sumDenominator;
  }
  else
  {
    resultBuffer[ i ] = -9999; // Set nodata
  }

}
