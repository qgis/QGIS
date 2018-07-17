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
#include <cmath>

QgsHillshadeFilter::QgsHillshadeFilter( const QString &inputFile, const QString &outputFile, const QString &outputFormat, double lightAzimuth,
                                        double lightAngle )
  : QgsDerivativeFilter( inputFile, outputFile, outputFormat )
  , mLightAzimuth( static_cast<float>( lightAzimuth ) )
  , mLightAngle( static_cast<float>( lightAngle ) )
  , mCosZenithRad( std::cos( static_cast<float>( lightAngle * M_PI ) / 180.0f ) )
  , mSinZenithRad( std::sin( static_cast<float>( lightAngle * M_PI ) / 180.0f ) )
  , mAzimuthRad( static_cast<float>( lightAzimuth * M_PI ) / 180.0f )
{
}

float QgsHillshadeFilter::processNineCellWindow( float *x11, float *x21, float *x31,
    float *x12, float *x22, float *x32,
    float *x13, float *x23, float *x33 )
{

  float derX = calcFirstDerX( x11, x21, x31, x12, x22, x32, x13, x23, x33 );
  float derY = calcFirstDerY( x11, x21, x31, x12, x22, x32, x13, x23, x33 );

  if ( derX == mOutputNodataValue || derY == mOutputNodataValue )
  {
    return mOutputNodataValue;
  }

  float slope_rad = std::atan( std::sqrt( derX * derX + derY * derY ) );
  float aspect_rad = 0;
  if ( derX == 0 && derY == 0 ) //aspect undefined, take a neutral value. Better solutions?
  {
    aspect_rad = mAzimuthRad / 2.0f;
  }
  else
  {
    aspect_rad = M_PI + std::atan2( derX, derY );
  }
  return std::max( 0.0f, 255.0f * ( ( mCosZenithRad * std::cos( slope_rad ) ) +
                                    ( mSinZenithRad * std::sin( slope_rad ) *
                                      std::cos( mAzimuthRad - aspect_rad ) ) ) );
}

void QgsHillshadeFilter::setLightAzimuth( float azimuth )
{
  mLightAzimuth = azimuth;
  mAzimuthRad = azimuth * static_cast<float>( M_PI ) / 180.0f;
}

void QgsHillshadeFilter::setLightAngle( float angle )
{
  mLightAngle = angle;
  mCosZenithRad = std::cos( angle * static_cast<float>( M_PI ) / 180.0f );
  mSinZenithRad = std::sin( angle * static_cast<float>( M_PI ) / 180.0f );
}

<<<<<<< a73bbbad21629d81b9b1d4217a096a930473eb5c
#ifdef HAVE_OPENCL

void QgsHillshadeFilter::addExtraRasterParams( std::vector<float> &params )
{

  params.push_back( mCosZenithRad ); // cos_zenith_rad 5
  params.push_back( mSinZenithRad ); // sin_zenith_rad 6
  params.push_back( mAzimuthRad ); // azimuth_rad 7

}

#endif
=======
void QgsHillshadeFilter::addExtraRasterParams( std::vector<float> &params )
{
  float azimuthRad =  -1 * mLightAzimuth * M_PI / 180.0;
  float zenithRad = std::max( 0.0f, 90.0f - mLightAngle ) * M_PI / 180.0;
  float cosZenithRad = std::cos( zenithRad );
  float cos_az_mul_cos_alt_mul_z = std::cos( azimuthRad ) * cosZenithRad * mZFactor;
  float sin_az_mul_cos_alt_mul_z = std::sin( azimuthRad ) * cosZenithRad * mZFactor;
  float cos_az_mul_cos_alt_mul_z_mul_254 = 254.0 * cos_az_mul_cos_alt_mul_z;
  float sin_az_mul_cos_alt_mul_z_mul_254 = 254.0 * sin_az_mul_cos_alt_mul_z;
  float square_z = mZFactor * mZFactor;
  float sin_altRadians_mul_254 = 254.0 * std::sin( zenithRad );

  // For fast formula from GDAL DEM
  params.push_back( cos_az_mul_cos_alt_mul_z_mul_254 ); // 5
  params.push_back( sin_az_mul_cos_alt_mul_z_mul_254 ); // 6
  params.push_back( square_z ); // 7
  params.push_back( sin_altRadians_mul_254 ); // 8

}
>>>>>>> [opencl] Use fast formula for hillshade
