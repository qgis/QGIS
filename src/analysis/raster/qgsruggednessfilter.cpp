/***************************************************************************
                          qgsruggednessfilter.cpp  -  description
                          -----------------------
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

#include "qgsruggednessfilter.h"

QgsRuggednessFilter::QgsRuggednessFilter( const QString& inputFile, const QString& outputFile, const QString& outputFormat ): QgsNineCellFilter( inputFile, outputFile, outputFormat )
{

}

QgsRuggednessFilter::QgsRuggednessFilter(): QgsNineCellFilter( "", "", "" )
{

}


QgsRuggednessFilter::~QgsRuggednessFilter()
{

}

float QgsRuggednessFilter::processNineCellWindow( float* x11, float* x21, float* x31,
    float* x12, float* x22, float* x32, float* x13, float* x23, float* x33 )
{
  //the formula would be that easy without nodata values...
  /*
    //return *x22; //test: write the raster value of the middle cell
    float diff1 = *x11 - *x22;
    float diff2 = *x21 - *x22;
    float diff3 = *x31 - *x22;
    float diff4 = *x12 - *x22;
    float diff5 = *x32 - *x22;
    float diff6 = *x13 - *x22;
    float diff7 = *x23 - *x22;
    float diff8 = *x33 - *x22;
    return sqrt(diff1 * diff1 + diff2 * diff2 + diff3 * diff3 + diff4 * diff4 + diff5 * diff5 + diff6 * diff6 + diff7 * diff7 + diff8 * diff8);
   */

  if ( *x22 == mInputNodataValue )
  {
    return mOutputNodataValue;
  }

  double sum = 0;
  if ( *x11 != mInputNodataValue )
  {
    sum += ( *x11 - *x22 ) * ( *x11 - *x22 );
  }
  if ( *x21 != mInputNodataValue )
  {
    sum += ( *x21 - *x22 ) * ( *x21 - *x22 );
  }
  if ( *x31 != mInputNodataValue )
  {
    sum += ( *x31 - *x22 ) * ( *x31 - *x22 );
  }
  if ( *x12 != mInputNodataValue )
  {
    sum += ( *x12 - *x22 ) * ( *x12 - *x22 );
  }
  if ( *x32 != mInputNodataValue )
  {
    sum += ( *x32 - *x22 ) * ( *x32 - *x22 );
  }
  if ( *x13 != mInputNodataValue )
  {
    sum += ( *x13 - *x22 ) * ( *x13 - *x22 );
  }
  if ( *x23 != mInputNodataValue )
  {
    sum += ( *x23 - *x22 ) * ( *x23 - *x22 );
  }
  if ( *x33 != mInputNodataValue )
  {
    sum += ( *x33 - *x22 ) * ( *x33 - *x22 );
  }

  return sqrt( sum );
}

