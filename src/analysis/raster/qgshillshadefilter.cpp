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
<<<<<<< 573283f0dcf022e84bd615e84fd2656043a9722b
  , mLightAzimuth( static_cast<float>( lightAzimuth ) )
  , mLightAngle( static_cast<float>( lightAngle ) )
  , mCosZenithRad( std::cos( static_cast<float>( lightAngle * M_PI ) / 180.0f ) )
  , mSinZenithRad( std::sin( static_cast<float>( lightAngle * M_PI ) / 180.0f ) )
  , mAzimuthRad( static_cast<float>( lightAzimuth * M_PI ) / 180.0f )
=======
  , mLightAzimuth( lightAzimuth )
  , mLightAngle( lightAngle )
<<<<<<< 8f40129d09776c7fe96b88604405a908595f9ede
  , mCosZenithRad( std::cos( mLightAngle * M_PI / 180.0 ) )
  , mSinZenithRad( std::sin( mLightAngle * M_PI / 180.0 ) )
  , mAzimuthRad( mLightAzimuth * M_PI / 180.0 )
>>>>>>> [opencl] Fix small OpenCL alg issues
=======
  , mCosZenithRad( std::cos( mLightAngle * M_PI / 180.0f ) )
  , mSinZenithRad( std::sin( mLightAngle * M_PI / 180.0f ) )
  , mAzimuthRad( mLightAzimuth * M_PI / 180.0f )
>>>>>>> [opencl] Small optimization in hillshade
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
<<<<<<< 573283f0dcf022e84bd615e84fd2656043a9722b
    aspect_rad = mAzimuthRad / 2.0f;
=======
    aspect_rad = mAzimuthRad / 2.0;
>>>>>>> [opencl] Fix small OpenCL alg issues
  }
  else
  {
    aspect_rad = M_PI + std::atan2( derX, derY );
  }
<<<<<<< 573283f0dcf022e84bd615e84fd2656043a9722b
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
=======
  return std::max( 0.0, 255.0 * ( ( mCosZenithRad * std::cos( slope_rad ) ) +
                                  ( mSinZenithRad * std::sin( slope_rad ) *
                                    std::cos( mAzimuthRad - aspect_rad ) ) ) );
}

#ifdef HAVE_OPENCL

>>>>>>> [opencl] Fix small OpenCL alg issues
void QgsHillshadeFilter::addExtraRasterParams( std::vector<float> &params )
{

  params.push_back( mCosZenithRad ); // cos_zenith_rad 5
  params.push_back( mSinZenithRad ); // sin_zenith_rad 6
  params.push_back( mAzimuthRad ); // azimuth_rad 7

}
<<<<<<< 573283f0dcf022e84bd615e84fd2656043a9722b
>>>>>>> [opencl] Use fast formula for hillshade
=======

#endif
>>>>>>> [opencl] Fix small OpenCL alg issues
