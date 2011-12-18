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

void QgsBilinearRasterResampler::resample( const QImage& srcImage, QImage& dstImage ) const
{
  double nSrcPerDstX = ( double ) srcImage.width() / ( double ) dstImage.width();
  double nSrcPerDstY = ( double ) srcImage.height() / ( double ) dstImage.height();

  double currentSrcRow = nSrcPerDstX / 2.0;
  double currentSrcCol;

  QRgb px1, px2, px3, px4;

  for ( int i = 0; i < dstImage.height(); ++i )
  {
    currentSrcCol = nSrcPerDstY / 2.0;
    for ( int j = 0; j < dstImage.width(); ++j )
    {
      px1 = srcImage.pixel( currentSrcCol, currentSrcRow );
      px2 = srcImage.pixel( currentSrcCol + 1, currentSrcRow );
      px3 = srcImage.pixel( currentSrcCol + 1, currentSrcRow + 1 );
      px4 = srcImage.pixel( currentSrcCol, currentSrcRow + 1 );
      double u = currentSrcCol - ( int ) currentSrcCol;
      double v = currentSrcRow - ( int ) currentSrcRow;
      dstImage.setPixel( j, i, resampleColorValue( u, v, px1, px2, px3, px4 ) );
      currentSrcCol += nSrcPerDstX;
    }
    currentSrcRow += nSrcPerDstY;
  }
}

QRgb QgsBilinearRasterResampler::resampleColorValue( double u, double v, QRgb col1, QRgb col2, QRgb col3, QRgb col4 ) const
{
  double r1 = qRed( col1 );
  double g1 = qGreen( col1 );
  double b1 = qBlue( col1 );
  double r2 = qRed( col2 );
  double g2 = qGreen( col2 );
  double b2 = qBlue( col2 );
  double r3 = qRed( col3 );
  double g3 = qGreen( col3 );
  double b3 = qBlue( col3 );
  double r4 = qRed( col4 );
  double g4 = qGreen( col4 );
  double b4 = qBlue( col4 );

  double rt1 = u * r2 + ( 1 - u ) * r1;
  double gt1 = u * g2 + ( 1 - u ) * g1;
  double bt1 = u * b2 + ( 1 - u ) * b1;

  double rt2 = u * r3 + ( 1 - u ) * r4;
  double gt2 = u * g3 + ( 1 - u ) * g4;
  double bt2 = u * b3 + ( 1 - u ) * b4;

  return qRgb( v * rt2 + ( 1 - v ) * rt1, v * gt2 + ( 1 - v ) * gt1,  v * bt2 + ( 1 - v ) * bt1 );
}
