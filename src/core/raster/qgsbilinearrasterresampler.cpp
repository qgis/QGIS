/***************************************************************************
                         qgsbilinearrasterresampler.cpp
                         ------------------------------
    begin                : December 2011
    copyright            : (C) 2011 by Marco Hugentobler
    email                : marco at sourcepole dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsbilinearrasterresampler.h"
#include <QImage>

QgsBilinearRasterResampler::QgsBilinearRasterResampler()
{
}

QgsBilinearRasterResampler::~QgsBilinearRasterResampler()
{
}

void QgsBilinearRasterResampler::resample( const QImage& srcImage, QImage& dstImage )
{
  double nSrcPerDstX = ( double ) srcImage.width() / ( double ) dstImage.width();
  double nSrcPerDstY = ( double ) srcImage.height() / ( double ) dstImage.height();

  double currentSrcRow = nSrcPerDstY / 2.0 - 0.5;
  double currentSrcCol;

  QRgb px1, px2, px3, px4;

  for ( int i = 0; i < dstImage.height(); ++i )
  {
    //avoid access to invalid rows
    if ( currentSrcRow < 0 )
    {
      currentSrcRow += nSrcPerDstY;
    }

    currentSrcCol = nSrcPerDstX / 2.0 - 0.5;
    for ( int j = 0; j < dstImage.width(); ++j )
    {
      double u = currentSrcCol - ( int )currentSrcCol;
      double v = currentSrcRow - ( int )currentSrcRow;
      if ( currentSrcCol > srcImage.width() - 2 )
      {
        //resample in one direction only
        px1 = srcImage.pixel( currentSrcCol, currentSrcRow );
        px2 = srcImage.pixel( currentSrcCol, currentSrcRow + 1 );
        dstImage.setPixel( j, i, resampleColorValue( v, px1, px2 ) );
        currentSrcCol += nSrcPerDstX;
        continue;
      }
      else if ( currentSrcRow > srcImage.height() - 2 )
      {
        px1 = srcImage.pixel( currentSrcCol, currentSrcRow );
        px2 = srcImage.pixel( currentSrcCol + 1, currentSrcRow );
        dstImage.setPixel( j, i, resampleColorValue( u, px1, px2 ) );
        currentSrcCol += nSrcPerDstX;
        continue;
      }
      px1 = srcImage.pixel( currentSrcCol, currentSrcRow );
      px2 = srcImage.pixel( currentSrcCol + 1, currentSrcRow );
      px3 = srcImage.pixel( currentSrcCol + 1, currentSrcRow + 1 );
      px4 = srcImage.pixel( currentSrcCol, currentSrcRow + 1 );
      dstImage.setPixel( j, i, resampleColorValue( u, v, px1, px2, px3, px4 ) );
      currentSrcCol += nSrcPerDstX;
    }
    currentSrcRow += nSrcPerDstY;
  }
}

QRgb QgsBilinearRasterResampler::resampleColorValue( double u, double v, QRgb col1, QRgb col2, QRgb col3, QRgb col4 ) const
{
  int red = bilinearInterpolation( u, v, qRed( col1 ), qRed( col2 ), qRed( col3 ), qRed( col4 ) );
  int green = bilinearInterpolation( u, v, qGreen( col1 ), qGreen( col2 ), qGreen( col3 ), qGreen( col4 ) );
  int blue = bilinearInterpolation( u, v, qBlue( col1 ), qBlue( col2 ), qBlue( col3 ), qBlue( col4 ) );
  return qRgb( red, green, blue );
}

QRgb QgsBilinearRasterResampler::resampleColorValue( double u, QRgb col1, QRgb col2 ) const
{
  return qRgb( qRed( col1 ) * ( 1 - u ) + qRed( col2 ) * u, qGreen( col1 ) * ( 1 - u ) + qGreen( col2 ) * u, qBlue( col1 ) * ( 1 - u ) + qBlue( col2 ) * u );
}
