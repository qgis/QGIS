/***************************************************************************
                          qgsaspectfilter.cpp  -  description
                          -----------------------------------
    begin                : August 7th, 2009
    copyright            : (C) 2009 by Marco Hugentobler
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

#include "qgsaspectfilter.h"

QgsAspectFilter::QgsAspectFilter( const QString& inputFile, const QString& outputFile, const QString& outputFormat )
    : QgsDerivativeFilter( inputFile, outputFile, outputFormat )
{

}

QgsAspectFilter::~QgsAspectFilter()
{

}

float QgsAspectFilter::processNineCellWindow(
  float* x11, float* x21, float* x31,
  float* x12, float* x22, float* x32,
  float* x13, float* x23, float* x33 )
{
  float derX = calcFirstDerX( x11, x21, x31, x12, x22, x32, x13, x23, x33 );
  float derY = calcFirstDerY( x11, x21, x31, x12, x22, x32, x13, x23, x33 );

  if ( derX == mOutputNodataValue ||
       derY == mOutputNodataValue ||
       ( derX == 0.0 && derY == 0.0 ) )
  {
    return mOutputNodataValue;
  }
  else
  {
    return 180.0 + atan2( derX, derY ) * 180.0 / M_PI;
  }
}

