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

void QgsCubicRasterResampler::resample( const QImage& srcImage, QImage& dstImage )
{
  int nCols = srcImage.width();
  int nRows = srcImage.height();

  int pos = 0;
  QRgb px;
  int* redMatrix = new int[ nCols * nRows ];
  int* greenMatrix = new int[ nCols * nRows ];
  int* blueMatrix = new int[ nCols * nRows ];

  for ( int i = 0; i < nRows; ++i )
  {
    for ( int j = 0; j < nCols; ++j )
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
  yDerivativeMatrix( nCols, nRows, yDerivativeMatrixRed, redMatrix );
  double* yDerivativeMatrixGreen = new double[ nCols * nRows ];
  yDerivativeMatrix( nCols, nRows, yDerivativeMatrixGreen, greenMatrix );
  double* yDerivativeMatrixBlue = new double[ nCols * nRows ];
  yDerivativeMatrix( nCols, nRows, yDerivativeMatrixBlue, blueMatrix );

  //compute output
  double nSrcPerDstX = ( double ) srcImage.width() / ( double ) dstImage.width();
  double nSrcPerDstY = ( double ) srcImage.height() / ( double ) dstImage.height();

  double currentSrcRow = nSrcPerDstX / 2.0 - 0.5;
  double currentSrcCol;
  int currentSrcColInt;
  int currentSrcRowInt;
  int lastSrcColInt = -1;
  int lastSrcRowInt = -1;

  int r, g, b;
  //bernstein polynomials
  double bp0u, bp1u, bp2u, bp3u, bp0v, bp1v, bp2v, bp3v;

  for ( int i = 0; i < dstImage.height(); ++i )
  {
    currentSrcRowInt = ( int ) currentSrcRow;

    currentSrcCol = nSrcPerDstY / 2.0 - 0.5;
    for ( int j = 0; j < dstImage.width(); ++j )
    {
      double u = currentSrcCol - ( int )currentSrcCol;
      double v = currentSrcRow - ( int )currentSrcRow;

      //out of bounds check
      currentSrcColInt = ( int )currentSrcCol;
      if ( currentSrcColInt < 0 || currentSrcColInt > srcImage.width() - 2
           || currentSrcRowInt < 0 || currentSrcRowInt > srcImage.height() - 2 )
      {
        lastSrcColInt = currentSrcColInt;
        currentSrcCol += nSrcPerDstX;
        dstImage.setPixel( j, i, qRgb( 0, 0, 255 ) );
        continue;
      }

      //first update the control points if necessary
      if ( currentSrcColInt != lastSrcColInt || currentSrcRowInt != lastSrcRowInt )
      {
        calculateControlPoints( nCols, nRows, currentSrcRowInt, currentSrcColInt, redMatrix, greenMatrix, blueMatrix,
                                xDerivativeMatrixRed, xDerivativeMatrixGreen, xDerivativeMatrixBlue,
                                yDerivativeMatrixRed, yDerivativeMatrixGreen, yDerivativeMatrixBlue );
      }

      //bernstein polynomials
      bp0u = calcBernsteinPoly( 3, 0, u ); bp1u = calcBernsteinPoly( 3, 1, u );
      bp2u = calcBernsteinPoly( 3, 2, u ); bp3u = calcBernsteinPoly( 3, 3, u );
      bp0v = calcBernsteinPoly( 3, 0, v ); bp1v = calcBernsteinPoly( 3, 1, v );
      bp2v = calcBernsteinPoly( 3, 2, v ); bp3v = calcBernsteinPoly( 3, 3, v );

      //then calculate value based on bernstein form of Bezier patch
      //todo: move into function
      r = bp0u * bp0v * cRed00 +
          bp1u * bp0v * cRed10 +
          bp2u * bp0v * cRed20 +
          bp3u * bp0v * cRed30 +
          bp0u * bp1v * cRed01 +
          bp1u * bp1v * cRed11 +
          bp2u * bp1v * cRed21 +
          bp3u * bp1v * cRed31 +
          bp0u * bp2v * cRed02 +
          bp1u * bp2v * cRed12 +
          bp2u * bp2v * cRed22 +
          bp3u * bp2v * cRed32 +
          bp0u * bp3v * cRed03 +
          bp1u * bp3v * cRed13 +
          bp2u * bp3v * cRed23 +
          bp3u * bp3v * cRed33;

      g = bp0u * bp0v * cGreen00 +
          bp1u * bp0v * cGreen10 +
          bp2u * bp0v * cGreen20 +
          bp3u * bp0v * cGreen30 +
          bp0u * bp1v * cGreen01 +
          bp1u * bp1v * cGreen11 +
          bp2u * bp1v * cGreen21 +
          bp3u * bp1v * cGreen31 +
          bp0u * bp2v * cGreen02 +
          bp1u * bp2v * cGreen12 +
          bp2u * bp2v * cGreen22 +
          bp3u * bp2v * cGreen32 +
          bp0u * bp3v * cGreen03 +
          bp1u * bp3v * cGreen13 +
          bp2u * bp3v * cGreen23 +
          bp3u * bp3v * cGreen33;

      b = bp0u * bp0v * cBlue00 +
          bp1u * bp0v * cBlue10 +
          bp2u * bp0v * cBlue20 +
          bp3u * bp0v * cBlue30 +
          bp0u * bp1v * cBlue01 +
          bp1u * bp1v * cBlue11 +
          bp2u * bp1v * cBlue21 +
          bp3u * bp1v * cBlue31 +
          bp0u * bp2v * cBlue02 +
          bp1u * bp2v * cBlue12 +
          bp2u * bp2v * cBlue22 +
          bp3u * bp2v * cBlue32 +
          bp0u * bp3v * cBlue03 +
          bp1u * bp3v * cBlue13 +
          bp2u * bp3v * cBlue23 +
          bp3u * bp3v * cBlue33;

      dstImage.setPixel( j, i, qRgb( r, g, b ) );
      lastSrcColInt = currentSrcColInt;
      currentSrcCol += nSrcPerDstX;
    }
    lastSrcRowInt = currentSrcRowInt;
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
}

void QgsCubicRasterResampler::xDerivativeMatrix( int nCols, int nRows, double* matrix, const int* colorMatrix )
{
  double val;
  int index = 0;

  for ( int i = 0; i < nRows; ++i )
  {
    for ( int j = 0; j < nCols; ++j )
    {
      if ( j < 1 )
      {
        val = colorMatrix[index + 1] - colorMatrix[index];
      }
      else if ( j == ( nCols - 1 ) )
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

void QgsCubicRasterResampler::yDerivativeMatrix( int nCols, int nRows, double* matrix, const int* colorMatrix )
{
  double val;
  int index = 0;

  for ( int i = 0; i < nRows; ++i )
  {
    for ( int j = 0; j < nCols; ++j )
    {
      if ( i == 0 )
      {
        val = colorMatrix[ index + nRows ] - colorMatrix[ index ];
      }
      else if ( i == nRows - 1 )
      {
        val = colorMatrix[ index ] - colorMatrix[ index - nRows ];
      }
      else
      {
        val = ( colorMatrix[ index + nRows ] - colorMatrix[ index - nRows ] ) / 2.0;
      }
      matrix[index] = val;
      ++index;
    }
  }
}

void QgsCubicRasterResampler::calculateControlPoints( int nCols, int nRows, int currentRow, int currentCol, int* redMatrix, int* greenMatrix, int* blueMatrix,
    double* xDerivativeMatrixRed, double* xDerivativeMatrixGreen, double* xDerivativeMatrixBlue,
    double* yDerivativeMatrixRed, double* yDerivativeMatrixGreen, double* yDerivativeMatrixBlue )
{
  int idx00 = currentRow * nCols + currentCol;
  int idx10 = idx00 + 1;
  int idx01 = idx00 + nCols;
  int idx11 = idx01 + 1;

  //corner points
  cRed00 = redMatrix[idx00]; cGreen00 = greenMatrix[idx00]; cBlue00 = blueMatrix[idx00];
  cRed30 = redMatrix[idx10]; cGreen30 = greenMatrix[idx10]; cBlue30 = blueMatrix[idx10];
  cRed03 = redMatrix[idx01]; cGreen03 = greenMatrix[idx01]; cBlue03 = blueMatrix[idx01];
  cRed33 = redMatrix[idx11]; cGreen33 = greenMatrix[idx11]; cBlue33 = blueMatrix[idx11];

  //control points near c00
  cRed10 = cRed00 + 0.3 * xDerivativeMatrixRed[idx00]; cGreen10 = cGreen00 + 0.3 * xDerivativeMatrixGreen[idx00]; cBlue10 = cBlue00 + 0.3 * xDerivativeMatrixBlue[idx00];
  cRed01 = cRed00 + 0.3 * yDerivativeMatrixRed[idx00]; cGreen01 = cGreen00 + 0.3 * yDerivativeMatrixGreen[idx00]; cBlue01 = cBlue00 + 0.3 * yDerivativeMatrixBlue[idx00];
  cRed11 = cRed10 + 0.3 * yDerivativeMatrixRed[idx00]; cGreen11 = cGreen10 + 0.3 * yDerivativeMatrixGreen[idx00]; cBlue11 = cBlue10 + 0.3 * yDerivativeMatrixBlue[idx00];

  //control points near c30
  cRed20 = cRed30 - 0.3 * xDerivativeMatrixRed[idx10]; cGreen20 = cGreen30 - 0.3 * xDerivativeMatrixGreen[idx10]; cBlue20 = cBlue30 - 0.3 * xDerivativeMatrixBlue[idx10];
  cRed31 = cRed30 + 0.3 * yDerivativeMatrixRed[idx10]; cGreen31 = cGreen30 + 0.3 * yDerivativeMatrixGreen[idx10]; cBlue31 = cBlue30 + 0.3 * yDerivativeMatrixBlue[idx10];
  cRed21 = cRed20 + 0.3 * yDerivativeMatrixRed[idx10]; cGreen21 = cGreen20 + 0.3 * yDerivativeMatrixGreen[idx10]; cBlue21 = cBlue20 + 0.3 * yDerivativeMatrixBlue[idx10];

  //control points near c03
  cRed13 = cRed03 + 0.3 * xDerivativeMatrixRed[idx01]; cGreen13 = cGreen03 + 0.3 * xDerivativeMatrixGreen[idx01]; cBlue13 = cBlue03 + 0.3 * xDerivativeMatrixBlue[idx01];
  cRed02 = cRed03 - 0.3 * yDerivativeMatrixRed[idx01]; cGreen02 = cGreen03 - 0.3 * yDerivativeMatrixGreen[idx01]; cBlue02 = cBlue03 - 0.3 * yDerivativeMatrixBlue[idx01];
  cRed12 = cRed02 + 0.3 * xDerivativeMatrixRed[idx01]; cGreen12 = cGreen02 + 0.3 * xDerivativeMatrixGreen[idx01]; cBlue12 = cBlue02 + 0.3 * xDerivativeMatrixBlue[idx01];

  //control points near c33
  cRed23 = cRed33 - 0.3 * xDerivativeMatrixRed[idx11]; cGreen23 = cGreen33 - 0.3 * xDerivativeMatrixGreen[idx11]; cBlue23 = cBlue33 - 0.3 * xDerivativeMatrixBlue[idx11];
  cRed32 = cRed33 - 0.3 * yDerivativeMatrixRed[idx11]; cGreen32 = cGreen33 - 0.3 * yDerivativeMatrixGreen[idx11]; cBlue32 = cBlue33 - 0.3 * yDerivativeMatrixBlue[idx11];
  cRed22 = cRed32 - 0.3 * xDerivativeMatrixRed[idx11]; cGreen22 = cGreen32 - 0.3 * xDerivativeMatrixGreen[idx11]; cBlue22 = cBlue32 - 0.3 * xDerivativeMatrixBlue[idx11];
}

double QgsCubicRasterResampler::calcBernsteinPoly( int n, int i, double t )
{
  if ( i < 0 )
  {
    return 0;
  }

  return lower( n, i )*power( t, i )*power(( 1 - t ), ( n - i ) );
}

int QgsCubicRasterResampler::lower( int n, int i )
{
  if ( i >= 0 && i <= n )
  {
    return faculty( n ) / ( faculty( i )*faculty( n - i ) );
  }
  else
  {
    return 0;
  }
}

double QgsCubicRasterResampler::power( double a, int b )
{
  if ( b == 0 )
  {
    return 1;
  }
  double tmp = a;
  for ( int i = 2; i <= qAbs(( double )b ); i++ )
  {

    a *= tmp;
  }
  if ( b > 0 )
  {
    return a;
  }
  else
  {
    return ( 1.0 / a );
  }
}

int QgsCubicRasterResampler::faculty( int n )
{
  if ( n < 0 )//Is faculty also defined for negative integers?
  {
    return 0;
  }
  int i;
  int result = n;

  if ( n == 0 || n == 1 )
    {return 1;}//faculty of 0 is 1!

  for ( i = n - 1; i >= 2; i-- )
  {
    result *= i;
  }
  return result;
}
