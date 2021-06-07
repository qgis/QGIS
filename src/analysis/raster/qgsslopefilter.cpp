/***************************************************************************
                          qgsslopefilter.h  -  description
                          --------------------------------
    begin                : August 7th, 2009
    copyright            : (C) 2009 by Marco Hugentobler
    email                : marco dot hugentobler at karto dot baug dot ethz dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   *
 *  
 *        *
 *                                     *
 *                                                                         *
 ***************************************************************************/

#include "qgsslopefilter.h"
#include <cmath>

QgsSlopeFilter::QgsSlopeFilter( const QString &inputFile, const QString &outputFile, const QString &outputFormat )
  : QgsDerivativeFilter( inputFile, outputFile, outputFormat )
{

}

float QgsSlopeFilter::processNineCellWindow(
  float *x11, float *x21, float *x31,
  float *x12, float *x22, float *x32,
  float *x13, float *x23, float *x33 )
{
  float derX = calcFirstDerX( x11, x21, x31, x12, x22, x32, x13, x23, x33 );
  float derY = calcFirstDerY( x11, x21, x31, x12, x22, x32, x13, x23, x33 );

  if ( derX == mOutputNodataValue || derY == mOutputNodataValue )
  {
    return mOutputNodataValue;
  }

  return std::atan( std::sqrt( derX * derX + derY * derY ) ) * 180.0 / M_PI;
}

