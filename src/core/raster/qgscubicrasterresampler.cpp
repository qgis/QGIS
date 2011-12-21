/***************************************************************************
                         qgscubicrasterresampler.cpp
                         ----------------------------
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

#include "qgscubicrasterresampler.h"
#include <QImage>

QgsCubicRasterResampler::QgsCubicRasterResampler()
{
}

QgsCubicRasterResampler::~QgsCubicRasterResampler()
{
}

void QgsCubicRasterResampler::resample( const QImage& srcImage, QImage& dstImage ) const
{
  int nCols = srcImage.width();
  int nRows = srcImage.height();

  int pos = 0;
  QRgb px;
  int* redMatrix = new int[ nCols * nRows ];
  int* greenMatrix = new int[ nCols * nRows ];
  int* blueMatrix = new int[ nCols * nRows ];

  for( int i = 0; i < nRows; ++i )
  {
    for( int j = 0; j < nCols; ++j )
    {
      px = srcImage.pixel( j, i );
      redMatrix[pos] = qRed( px );
      greenMatrix[pos] = qGreen( px );
      blueMatrix[pos] = qBlue( px );
      ++pos;
    }
  }

  //derivative x
  double* xDerivativeMatrixRed = new double[ nCols * nRows ];
  xDerivativeMatrix( nCols, nRows, xDerivativeMatrixRed, redMatrix );
  double* xDerivativeMatrixGreen = new double[ nCols * nRows ];
  xDerivativeMatrix( nCols, nRows, xDerivativeMatrixGreen, greenMatrix );
  double* xDerivativeMatrixBlue = new double[ nCols * nRows ];
  xDerivativeMatrix( nCols, nRows, xDerivativeMatrixBlue, blueMatrix );

  //derivative y
  double* yDerivativeMatrixRed = new double[ nCols * nRows ];
  yDerivativeMatrix<int>( nCols, nRows, yDerivativeMatrixRed, redMatrix );
  double* yDerivativeMatrixGreen = new double[ nCols * nRows ];
  yDerivativeMatrix<int>( nCols, nRows, yDerivativeMatrixGreen, greenMatrix );
  double* yDerivativeMatrixBlue = new double[ nCols * nRows ];
  yDerivativeMatrix<int>( nCols, nRows, yDerivativeMatrixBlue, blueMatrix );

  //derivative xy
  double* xyDerivativeMatrixRed = new double[ nCols * nRows ];
  yDerivativeMatrix<double>( nCols, nRows, xyDerivativeMatrixRed, xDerivativeMatrixRed );
  double* xyDerivativeMatrixGreen = new double[ nCols * nRows ];
  yDerivativeMatrix<double>( nCols, nRows, xyDerivativeMatrixGreen, xDerivativeMatrixGreen );
  double* xyDerivativeMatrixBlue = new double[ nCols * nRows ];
  yDerivativeMatrix<double>( nCols, nRows, xyDerivativeMatrixBlue, xDerivativeMatrixBlue );

  //compute output
  double nSrcPerDstX = ( double ) srcImage.width() / ( double ) dstImage.width();
  double nSrcPerDstY = ( double ) srcImage.height() / ( double ) dstImage.height();

  double currentSrcRow = nSrcPerDstX / 2.0 - 0.5;
  double currentSrcCol;
  int currentSrcColInt;
  int currentSrcRowInt;

  int r, g, b;
  int index;

  for ( int i = 0; i < dstImage.height(); ++i )
  {
    currentSrcRowInt = (int) currentSrcRow;

    currentSrcCol = nSrcPerDstY / 2.0 - 0.5;
    for( int j = 0; j < dstImage.width(); ++j )
    {
      //out of bounds check
      currentSrcColInt = (int)currentSrcCol;
      if( currentSrcColInt < 0 || currentSrcColInt > srcImage.width() - 2
        || currentSrcRowInt < 0 || currentSrcRowInt > srcImage.height() - 2 )
      {
        currentSrcCol += nSrcPerDstX;
        ++index;
        dstImage.setPixel( j, i, qRgb( 0, 0, 255 ) );
        continue;
      }


      //interpolation
      index = currentSrcRowInt * nCols + currentSrcColInt;
      r = cubicInterpolation( redMatrix[index], redMatrix[index + 1], redMatrix[index + nCols + 1 ], redMatrix[ index + nCols ],
                              xDerivativeMatrixRed[index], xDerivativeMatrixRed[index + 1], xDerivativeMatrixRed[index + nCols + 1 ], xDerivativeMatrixRed[ index + nCols ],
                              yDerivativeMatrixRed[index], yDerivativeMatrixRed[index + 1], yDerivativeMatrixRed[index + nCols + 1 ], yDerivativeMatrixRed[ index + nCols ],
                              xyDerivativeMatrixRed[index], xyDerivativeMatrixRed[index + 1], xyDerivativeMatrixRed[index + nCols + 1 ], xyDerivativeMatrixRed[ index + nCols ] );
      g = cubicInterpolation( greenMatrix[index], greenMatrix[index + 1], greenMatrix[index + nCols + 1 ], greenMatrix[ index + nCols ],
                              xDerivativeMatrixGreen[index], xDerivativeMatrixGreen[index + 1], xDerivativeMatrixGreen[index + nCols + 1 ], xDerivativeMatrixGreen[ index + nCols ],
                              yDerivativeMatrixGreen[index], yDerivativeMatrixGreen[index + 1], yDerivativeMatrixGreen[index + nCols + 1 ], yDerivativeMatrixGreen[ index + nCols ],
                              xyDerivativeMatrixGreen[index], xyDerivativeMatrixGreen[index + 1], xyDerivativeMatrixGreen[index + nCols + 1 ], xyDerivativeMatrixGreen[ index + nCols ] );
      b = cubicInterpolation( blueMatrix[index], blueMatrix[index + 1], blueMatrix[index + nCols + 1 ], blueMatrix[ index + nCols ],
                              xDerivativeMatrixBlue[index], xDerivativeMatrixBlue[index + 1], xDerivativeMatrixBlue[index + nCols + 1 ], xDerivativeMatrixBlue[ index + nCols ],
                              yDerivativeMatrixBlue[index], yDerivativeMatrixBlue[index + 1], yDerivativeMatrixBlue[index + nCols + 1 ], yDerivativeMatrixBlue[ index + nCols ],
                              xyDerivativeMatrixBlue[index], xyDerivativeMatrixBlue[index + 1], xyDerivativeMatrixBlue[index + nCols + 1 ], xyDerivativeMatrixBlue[ index + nCols ] );

      dstImage.setPixel( j, i, qRgb( 255, 0, 0 ) );
      currentSrcCol += nSrcPerDstX;
    }
    currentSrcRow += nSrcPerDstY;
  }


  //cleanup memory
  delete[] redMatrix;
  delete[] greenMatrix;
  delete[] blueMatrix;
  delete[] xDerivativeMatrixRed;
  delete[] xDerivativeMatrixGreen;
  delete[] xDerivativeMatrixBlue;
  delete[] yDerivativeMatrixRed;
  delete[] yDerivativeMatrixGreen;
  delete[] yDerivativeMatrixBlue;
  delete[] xyDerivativeMatrixRed;
  delete[] xyDerivativeMatrixGreen;
  delete[] xyDerivativeMatrixBlue;
}

void QgsCubicRasterResampler::xDerivativeMatrix( int nCols, int nRows, double* matrix, const int* colorMatrix )
{
  double val;
  int index = 0;

  for( int i = 0; i < nRows; ++i )
  {
    for( int j = 0; j < nCols; ++j )
    {
      if( j < 1 )
      {
        val = colorMatrix[index + 1] - colorMatrix[index];
      }
      else if( j == ( nCols - 1 ) )
      {
        val = colorMatrix[index] - colorMatrix[ index - 1 ];
      }
      else
      {
        val = ( colorMatrix[index + 1] - colorMatrix[index - 1] ) / 2.0;
      }
      matrix[index] = val;
      ++index;
    }
  }
}

int QgsCubicRasterResampler::cubicInterpolation( double p1, double p2, double p3, double p4, double p1x, double p2x, double p3x, double p4x,
                                   double p1y, double p2y, double p3y, double p4y, double p1xy, double p2xy, double p3xy, double p4xy )
{
#if 0
  //calculate 16 coefficients
  double a00 = p1 - 3 * p4 + 2* p3 - 3 * p1y + 9 * p4y - 6 * p3y + 2 * p1xy - 6 * p4xy+ 4 * p3xy;
  double a10 = 3 * p4 - 2 * p3 - 9* p4y + 6 * p3y + 6 * p4xy - 4 * p3xy;
  double a20 = 3 * p1y - 9 * p4y + 6 * p3y - 2 * p1xy + 6 * p4xy - 4 * p3xy;
  double a30 = 9 * p4y - 6 * p3y - 6 * p4xy + 4 * p3xy;
  double a01 = p2 - 2 * p4 + p3 - 3 * p2y + 6 * p4y -3 * p3y + 2 * p2xy - 4 * p4xy + 2 * p3xy;
  double a11 = - p4 + p3 + 3 * p4y + -3 * p3y - 2 * p4xy;
  double a21 = 3 * p2y - 6 * p4y + 3 * p3y -2 * p2xy + 4 * p4xy - 2 * p3xy;
  double a31 = - 3 * p4y + 3 * p3y + 2 * p4xy -2 * p3xy;
  double a02 = p1x - 3 * p4x + 2 * p3x - 2 * p1y + 6 * p4y - 4 * p3y + p1xy - 3 * p4xy + 2 * p3xy;
  double a12 = 3 * p4x - 2 * p3x - 6 * p4y + 4 * p3y + 3 * p4xy - 2 * p3xy;
  double a22 = - p1y + 3 * p4y - 2 * p3y + p1xy - 3 * p4xy + 2 * p3xy;
  double a32 = -3 * p4y + 2 * p3y + 3 * p4xy - 2 * p3xy;
  double a03 = p2x - 2 * p4x + p3x - 2 * p4y + 4 * p4y - 2 * p3y + p2xy - 2 * p4xy + p3xy;
  double a13 = - p4x + p3x + 2 * p4y - 2 * p3y - p4xy + p3xy;
#endif //0
  return 0;
}
