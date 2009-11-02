/***************************************************************************
                          qgstotalcurvaturefilter.h  -  description
                             -------------------
    begin                : August 21th, 2009
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

#include "qgstotalcurvaturefilter.h"

QgsTotalCurvatureFilter::QgsTotalCurvatureFilter( const QString& inputFile, const QString& outputFile, const QString& outputFormat ): \
    QgsNineCellFilter( inputFile, outputFile, outputFormat )
{

}

QgsTotalCurvatureFilter::~QgsTotalCurvatureFilter()
{

}

float QgsTotalCurvatureFilter::processNineCellWindow( float* x11, float* x21, float* x31, float* x12, \
    float* x22, float* x32, float* x13, float* x23, float* x33 )
{
  //return nodata if one value is the nodata value
  if ( *x11 == mInputNodataValue || *x21 == mInputNodataValue || *x31 == mInputNodataValue || *x12 == mInputNodataValue \
       || *x22 == mInputNodataValue || *x32 == mInputNodataValue || *x13 == mInputNodataValue || *x23 == mInputNodataValue \
       || *x33 == mInputNodataValue )
  {
    return mOutputNodataValue;
  }

  double cellSizeAvg = ( mCellSizeX + mCellSizeY ) / 2.0;
  double dxx = ( *x32 - 2 * *x22 + *x12 ) / ( mCellSizeX * mCellSizeX );
  double dyy = ( -*x11 + *x31 + *x13 - *x33 ) / ( 4 * cellSizeAvg * cellSizeAvg );
  double dxy = ( *x21 - 2 * *x22 + *x23 ) / ( mCellSizeY * mCellSizeY );

  return dxx*dxx + 2*dxy*dxy + dyy*dyy;
}
