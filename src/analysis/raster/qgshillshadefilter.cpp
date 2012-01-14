/***************************************************************************
                          qgshillshadefilter.h  -  description
                          --------------------------------
    begin                : September 26th, 2011
    copyright            : (C) 2011 by Marco Hugentobler
    email                : marco dot hugentobler at sourcepole dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgshillshadefilter.h"

QgsHillshadeFilter::QgsHillshadeFilter( const QString& inputFile, const QString& outputFile, const QString& outputFormat, double lightAzimuth,
                                        double lightAngle )
    : QgsDerivativeFilter( inputFile, outputFile, outputFormat )
    , mLightAzimuth( lightAzimuth )
    , mLightAngle( lightAngle )
{
}

QgsHillshadeFilter::~QgsHillshadeFilter()
{
}

float QgsHillshadeFilter::processNineCellWindow( float* x11, float* x21, float* x31,
    float* x12, float* x22, float* x32,
    float* x13, float* x23, float* x33 )
{
  float derX = calcFirstDerX( x11, x21, x31, x12, x22, x32, x13, x23, x33 );
  float derY = calcFirstDerY( x11, x21, x31, x12, x22, x32, x13, x23, x33 );

  if ( derX == mOutputNodataValue || derY == mOutputNodataValue )
  {
    return mOutputNodataValue;
  }

  float zenith_rad = mLightAngle * M_PI / 180.0;
  float slope_rad = atan( sqrt( derX * derX + derY * derY ) );
  float azimuth_rad = mLightAzimuth * M_PI / 180.0;
  float aspect_rad = 0;
  if ( derX == 0 && derY == 0 ) //aspect undefined, take a neutral value. Better solutions?
  {
    aspect_rad = azimuth_rad / 2.0;
  }
  else
  {
    aspect_rad = M_PI + atan2( derX, derY );
  }
  return qMax( 0.0, 255.0 * (( cos( zenith_rad ) * cos( slope_rad ) ) + ( sin( zenith_rad ) * sin( slope_rad ) * cos( azimuth_rad - aspect_rad ) ) ) );
}
