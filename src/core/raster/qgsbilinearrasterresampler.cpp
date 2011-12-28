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
#include <cmath>

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
  int currentSrcRowInt, currentSrcColInt;
  double u, v;

  QRgb px1, px2, px3, px4;

  for ( int i = 0; i < dstImage.height(); ++i )
  {
    currentSrcCol = nSrcPerDstX / 2.0 - 0.5;
    currentSrcRowInt = floor( currentSrcRow );
    v = currentSrcRow - currentSrcRowInt;

    for ( int j = 0; j < dstImage.width(); ++j )
    {
      currentSrcColInt = floor( currentSrcCol );
      u = currentSrcCol - currentSrcColInt;

      //handle eight edge-cases
      if( currentSrcRowInt < 0 || currentSrcRowInt >= (srcImage.height() - 1) || currentSrcColInt < 0 || currentSrcColInt >= (srcImage.width() - 1) )
      {
        //pixels at the border of the source image needs to be handled in a special way
        if( currentSrcRowInt < 0 && currentSrcColInt < 0 )
        {
          dstImage.setPixel( j, i, srcImage.pixel(0, 0) );
        }
        else if( currentSrcRowInt < 0 && currentSrcColInt >= ( srcImage.width() - 1 ) )
        {
          dstImage.setPixel( j, i, srcImage.pixel( srcImage.width() - 1, 0 ) );
        }
        else if( currentSrcRowInt >= (srcImage.height() - 1) && currentSrcColInt >= ( srcImage.width() - 1 ) )
        {
          dstImage.setPixel( j, i, srcImage.pixel( srcImage.width() - 1, srcImage.height() - 1 ) );
        }
        else if( currentSrcRowInt >= (srcImage.height() - 1) && currentSrcColInt < 0 )
        {
          dstImage.setPixel( j, i, srcImage.pixel( 0, srcImage.height() - 1 ) );
        }
        else if( currentSrcRowInt < 0 )
        {
          px1 = srcImage.pixel( currentSrcColInt, 0 );
          px2 = srcImage.pixel( currentSrcColInt + 1, 0 );
          dstImage.setPixel( j, i, resampleColorValue( u, px1, px2 ) );
        }
        else if( currentSrcRowInt >= (srcImage.height() - 1) )
        {
          px1 = srcImage.pixel( currentSrcColInt, srcImage.height() - 1 );
          px2 = srcImage.pixel( currentSrcColInt + 1, srcImage.height() - 1 );
          dstImage.setPixel( j, i, resampleColorValue( u, px1, px2 ) );
        }
        else if( currentSrcColInt < 0 )
        {
          px1 = srcImage.pixel( 0, currentSrcRowInt );
          px2 = srcImage.pixel( 0, currentSrcRowInt + 1);
          dstImage.setPixel( j, i, resampleColorValue( v, px1, px2 ) );
        }
        else if( currentSrcColInt >= ( srcImage.width() - 1 ) )
        {
          px1 = srcImage.pixel( srcImage.width() - 1, currentSrcRowInt );
          px2 = srcImage.pixel( srcImage.width() - 1, currentSrcRowInt + 1);
          dstImage.setPixel( j, i, resampleColorValue( v, px1, px2 ) );
        }
        currentSrcCol += nSrcPerDstX;
        continue;
      }

      px1 = srcImage.pixel( currentSrcColInt, currentSrcRowInt );
      px2 = srcImage.pixel( currentSrcColInt + 1, currentSrcRowInt );
      px3 = srcImage.pixel( currentSrcColInt + 1, currentSrcRowInt + 1 );
      px4 = srcImage.pixel( currentSrcColInt, currentSrcRowInt + 1 );
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
