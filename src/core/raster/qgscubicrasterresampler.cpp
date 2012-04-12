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
#include <cmath>

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
  int* alphaMatrix = new int[ nCols * nRows ];

  for ( int i = 0; i < nRows; ++i )
  {
    for ( int j = 0; j < nCols; ++j )
    {
      px = srcImage.pixel( j, i );
      redMatrix[pos] = qRed( px );
      greenMatrix[pos] = qGreen( px );
      blueMatrix[pos] = qBlue( px );
      alphaMatrix[pos] = qAlpha( px );
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
  double* xDerivativeMatrixAlpha = new double[ nCols * nRows ];
  xDerivativeMatrix( nCols, nRows, xDerivativeMatrixAlpha, alphaMatrix );

  //derivative y
  double* yDerivativeMatrixRed = new double[ nCols * nRows ];
  yDerivativeMatrix( nCols, nRows, yDerivativeMatrixRed, redMatrix );
  double* yDerivativeMatrixGreen = new double[ nCols * nRows ];
  yDerivativeMatrix( nCols, nRows, yDerivativeMatrixGreen, greenMatrix );
  double* yDerivativeMatrixBlue = new double[ nCols * nRows ];
  yDerivativeMatrix( nCols, nRows, yDerivativeMatrixBlue, blueMatrix );
  double* yDerivativeMatrixAlpha = new double[ nCols * nRows ];
  yDerivativeMatrix( nCols, nRows, yDerivativeMatrixAlpha, alphaMatrix );

  //compute output
  double nSrcPerDstX = ( double ) srcImage.width() / ( double ) dstImage.width();
  double nSrcPerDstY = ( double ) srcImage.height() / ( double ) dstImage.height();

  double currentSrcRow = nSrcPerDstY / 2.0 - 0.5;
  double currentSrcCol;
  int currentSrcColInt;
  int currentSrcRowInt;
  int lastSrcColInt = -1;
  int lastSrcRowInt = -1;

  int r, g, b, a;
  //bernstein polynomials
  double bp0u, bp1u, bp2u, bp3u, bp0v, bp1v, bp2v, bp3v;
  double u, v;

  for ( int i = 0; i < dstImage.height(); ++i )
  {
    currentSrcRowInt = floor( currentSrcRow );
    v = currentSrcRow - currentSrcRowInt;

    currentSrcCol = nSrcPerDstX / 2.0 - 0.5;
    for ( int j = 0; j < dstImage.width(); ++j )
    {
      currentSrcColInt = floor( currentSrcCol );
      u = currentSrcCol - currentSrcColInt;

      //handle eight edge-cases
      if ( currentSrcRowInt < 0 || currentSrcRowInt >= ( srcImage.height() - 1 ) || currentSrcColInt < 0 || currentSrcColInt >= ( srcImage.width() - 1 ) )
      {
        QRgb px1, px2;
        //pixels at the border of the source image needs to be handled in a special way
        if ( currentSrcRowInt < 0 && currentSrcColInt < 0 )
        {
          dstImage.setPixel( j, i, srcImage.pixel( 0, 0 ) );
        }
        else if ( currentSrcRowInt < 0 && currentSrcColInt >= ( srcImage.width() - 1 ) )
        {
          dstImage.setPixel( j, i, srcImage.pixel( srcImage.width() - 1, 0 ) );
        }
        else if ( currentSrcRowInt >= ( srcImage.height() - 1 ) && currentSrcColInt >= ( srcImage.width() - 1 ) )
        {
          dstImage.setPixel( j, i, srcImage.pixel( srcImage.width() - 1, srcImage.height() - 1 ) );
        }
        else if ( currentSrcRowInt >= ( srcImage.height() - 1 ) && currentSrcColInt < 0 )
        {
          dstImage.setPixel( j, i, srcImage.pixel( 0, srcImage.height() - 1 ) );
        }
        else if ( currentSrcRowInt < 0 )
        {
          px1 = srcImage.pixel( currentSrcColInt, 0 );
          px2 = srcImage.pixel( currentSrcColInt + 1, 0 );
          dstImage.setPixel( j, i, curveInterpolation( px1, px2, u, xDerivativeMatrixRed[ currentSrcColInt ], xDerivativeMatrixGreen[ currentSrcColInt ],
                             xDerivativeMatrixBlue[ currentSrcColInt ], xDerivativeMatrixAlpha[ currentSrcColInt ], xDerivativeMatrixRed[ currentSrcColInt + 1 ], xDerivativeMatrixGreen[ currentSrcColInt + 1 ],
                             xDerivativeMatrixBlue[ currentSrcColInt + 1 ], xDerivativeMatrixAlpha[ currentSrcColInt + 1 ] ) );
        }
        else if ( currentSrcRowInt >= ( srcImage.height() - 1 ) )
        {
          int idx = ( srcImage.height() - 1 ) * srcImage.width() + currentSrcColInt;
          px1 = srcImage.pixel( currentSrcColInt, srcImage.height() - 1 );
          px2 = srcImage.pixel( currentSrcColInt + 1, srcImage.height() - 1 );
          dstImage.setPixel( j, i, curveInterpolation( px1, px2, u, xDerivativeMatrixRed[ idx ], xDerivativeMatrixGreen[ idx ], xDerivativeMatrixBlue[idx],
                             xDerivativeMatrixAlpha[idx], xDerivativeMatrixRed[ idx + 1 ], xDerivativeMatrixGreen[ idx + 1 ], xDerivativeMatrixBlue[idx + 1],
                             xDerivativeMatrixAlpha[idx + 1] ) );
        }
        else if ( currentSrcColInt < 0 )
        {
          int idx1 = currentSrcRowInt * srcImage.width();
          int idx2 = idx1 + srcImage.width();
          px1 = srcImage.pixel( 0, currentSrcRowInt );
          px2 = srcImage.pixel( 0, currentSrcRowInt + 1 );
          dstImage.setPixel( j, i, curveInterpolation( px1, px2, v, yDerivativeMatrixRed[ idx1 ], yDerivativeMatrixGreen[ idx1 ], yDerivativeMatrixBlue[ idx1],
                             yDerivativeMatrixAlpha[ idx1], yDerivativeMatrixRed[ idx2 ], yDerivativeMatrixGreen[ idx2 ], yDerivativeMatrixBlue[ idx2],
                             yDerivativeMatrixAlpha[ idx2] ) );
        }
        else if ( currentSrcColInt >= ( srcImage.width() - 1 ) )
        {
          int idx1 = currentSrcRowInt * srcImage.width() + srcImage.width() - 1;
          int idx2 = idx1 + srcImage.width();
          px1 = srcImage.pixel( srcImage.width() - 1, currentSrcRowInt );
          px2 = srcImage.pixel( srcImage.width() - 1, currentSrcRowInt + 1 );
          dstImage.setPixel( j, i, curveInterpolation( px1, px2, v, yDerivativeMatrixRed[ idx1 ], yDerivativeMatrixGreen[ idx1 ], yDerivativeMatrixBlue[ idx1],
                             yDerivativeMatrixAlpha[ idx1], yDerivativeMatrixRed[ idx2 ], yDerivativeMatrixGreen[ idx2 ], yDerivativeMatrixBlue[ idx2],
                             yDerivativeMatrixAlpha[ idx2] ) );
        }
        currentSrcCol += nSrcPerDstX;
        continue;
      }

      //first update the control points if necessary
      if ( currentSrcColInt != lastSrcColInt || currentSrcRowInt != lastSrcRowInt )
      {
        calculateControlPoints( nCols, nRows, currentSrcRowInt, currentSrcColInt, redMatrix, greenMatrix, blueMatrix, alphaMatrix,
                                xDerivativeMatrixRed, xDerivativeMatrixGreen, xDerivativeMatrixBlue, xDerivativeMatrixAlpha,
                                yDerivativeMatrixRed, yDerivativeMatrixGreen, yDerivativeMatrixBlue, yDerivativeMatrixAlpha );
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

      a = bp0u * bp0v * cAlpha00 +
          bp1u * bp0v * cAlpha10 +
          bp2u * bp0v * cAlpha20 +
          bp3u * bp0v * cAlpha30 +
          bp0u * bp1v * cAlpha01 +
          bp1u * bp1v * cAlpha11 +
          bp2u * bp1v * cAlpha21 +
          bp3u * bp1v * cAlpha31 +
          bp0u * bp2v * cAlpha02 +
          bp1u * bp2v * cAlpha12 +
          bp2u * bp2v * cAlpha22 +
          bp3u * bp2v * cAlpha32 +
          bp0u * bp3v * cAlpha03 +
          bp1u * bp3v * cAlpha13 +
          bp2u * bp3v * cAlpha23 +
          bp3u * bp3v * cAlpha33;

      dstImage.setPixel( j, i, qRgba( r, g, b, a ) );
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
  double val = 0;
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
  double val = 0;
  int index = 0;

  for ( int i = 0; i < nRows; ++i )
  {
    for ( int j = 0; j < nCols; ++j )
    {
      if ( i == 0 )
      {
        val = colorMatrix[ index + nCols ] - colorMatrix[ index ];
      }
      else if ( i == ( nRows - 1 ) )
      {
        val = colorMatrix[ index ] - colorMatrix[ index - nCols ];
      }
      else
      {
        val = ( colorMatrix[ index + nCols ] - colorMatrix[ index - nCols ] ) / 2.0;
      }
      matrix[index] = val;
      ++index;
    }
  }
}

void QgsCubicRasterResampler::calculateControlPoints( int nCols, int nRows, int currentRow, int currentCol, int* redMatrix, int* greenMatrix, int* blueMatrix,
    int* alphaMatrix, double* xDerivativeMatrixRed, double* xDerivativeMatrixGreen, double* xDerivativeMatrixBlue, double* xDerivativeMatrixAlpha,
    double* yDerivativeMatrixRed, double* yDerivativeMatrixGreen, double* yDerivativeMatrixBlue, double* yDerivativeMatrixAlpha )
{
  Q_UNUSED(nRows);
  int idx00 = currentRow * nCols + currentCol;
  int idx10 = idx00 + 1;
  int idx01 = idx00 + nCols;
  int idx11 = idx01 + 1;

  //corner points
  cRed00 = redMatrix[idx00]; cGreen00 = greenMatrix[idx00]; cBlue00 = blueMatrix[idx00]; cAlpha00 = alphaMatrix[idx00];
  cRed30 = redMatrix[idx10]; cGreen30 = greenMatrix[idx10]; cBlue30 = blueMatrix[idx10]; cAlpha30 = alphaMatrix[idx10];
  cRed03 = redMatrix[idx01]; cGreen03 = greenMatrix[idx01]; cBlue03 = blueMatrix[idx01]; cAlpha03 = alphaMatrix[idx01];
  cRed33 = redMatrix[idx11]; cGreen33 = greenMatrix[idx11]; cBlue33 = blueMatrix[idx11]; cAlpha33 = alphaMatrix[idx11];

  //control points near c00
  cRed10 = cRed00 + 0.333 * xDerivativeMatrixRed[idx00]; cGreen10 = cGreen00 + 0.333 * xDerivativeMatrixGreen[idx00];
  cBlue10 = cBlue00 + 0.333 * xDerivativeMatrixBlue[idx00];cAlpha10 = cAlpha00 + 0.333 * xDerivativeMatrixAlpha[idx00];
  cRed01 = cRed00 + 0.333 * yDerivativeMatrixRed[idx00]; cGreen01 = cGreen00 + 0.333 * yDerivativeMatrixGreen[idx00];
  cBlue01 = cBlue00 + 0.333 * yDerivativeMatrixBlue[idx00];cAlpha01 = cAlpha00 + 0.333 * yDerivativeMatrixAlpha[idx00];
  cRed11 = cRed10 + 0.333 * yDerivativeMatrixRed[idx00]; cGreen11 = cGreen10 + 0.333 * yDerivativeMatrixGreen[idx00];
  cBlue11 = cBlue10 + 0.333 * yDerivativeMatrixBlue[idx00];cAlpha11 = cAlpha10 + 0.333 * yDerivativeMatrixAlpha[idx00];

  //control points near c30
  cRed20 = cRed30 - 0.333 * xDerivativeMatrixRed[idx10]; cGreen20 = cGreen30 - 0.333 * xDerivativeMatrixGreen[idx10];
  cBlue20 = cBlue30 - 0.333 * xDerivativeMatrixBlue[idx10]; cAlpha20 = cAlpha30 - 0.333 * xDerivativeMatrixAlpha[idx10];
  cRed31 = cRed30 + 0.333 * yDerivativeMatrixRed[idx10]; cGreen31 = cGreen30 + 0.333 * yDerivativeMatrixGreen[idx10];
  cBlue31 = cBlue30 + 0.333 * yDerivativeMatrixBlue[idx10]; cAlpha31 = cAlpha30 + 0.333 * yDerivativeMatrixAlpha[idx10];
  cRed21 = cRed20 + 0.333 * yDerivativeMatrixRed[idx10]; cGreen21 = cGreen20 + 0.333 * yDerivativeMatrixGreen[idx10];
  cBlue21 = cBlue20 + 0.333 * yDerivativeMatrixBlue[idx10]; cAlpha21 = cAlpha20 + 0.333 * yDerivativeMatrixAlpha[idx10];

  //control points near c03
  cRed13 = cRed03 + 0.333 * xDerivativeMatrixRed[idx01]; cGreen13 = cGreen03 + 0.333 * xDerivativeMatrixGreen[idx01];
  cBlue13 = cBlue03 + 0.333 * xDerivativeMatrixBlue[idx01]; cAlpha13 = cAlpha03 + 0.333 * xDerivativeMatrixAlpha[idx01];
  cRed02 = cRed03 - 0.333 * yDerivativeMatrixRed[idx01]; cGreen02 = cGreen03 - 0.333 * yDerivativeMatrixGreen[idx01];
  cBlue02 = cBlue03 - 0.333 * yDerivativeMatrixBlue[idx01]; cAlpha02 = cAlpha03 - 0.333 * yDerivativeMatrixAlpha[idx01];
  cRed12 = cRed02 + 0.333 * xDerivativeMatrixRed[idx01]; cGreen12 = cGreen02 + 0.333 * xDerivativeMatrixGreen[idx01];
  cBlue12 = cBlue02 + 0.333 * xDerivativeMatrixBlue[idx01]; cAlpha12 = cAlpha02 + 0.333 * xDerivativeMatrixAlpha[idx01];

  //control points near c33
  cRed23 = cRed33 - 0.333 * xDerivativeMatrixRed[idx11]; cGreen23 = cGreen33 - 0.333 * xDerivativeMatrixGreen[idx11];
  cBlue23 = cBlue33 - 0.333 * xDerivativeMatrixBlue[idx11]; cAlpha23 = cAlpha33 - 0.333 * xDerivativeMatrixAlpha[idx11];
  cRed32 = cRed33 - 0.333 * yDerivativeMatrixRed[idx11]; cGreen32 = cGreen33 - 0.333 * yDerivativeMatrixGreen[idx11];
  cBlue32 = cBlue33 - 0.333 * yDerivativeMatrixBlue[idx11]; cAlpha32 = cAlpha33 - 0.333 * yDerivativeMatrixAlpha[idx11];
  cRed22 = cRed32 - 0.333 * xDerivativeMatrixRed[idx11]; cGreen22 = cGreen32 - 0.333 * xDerivativeMatrixGreen[idx11];
  cBlue22 = cBlue32 - 0.333 * xDerivativeMatrixBlue[idx11]; cAlpha22 = cAlpha32 - 0.333 * xDerivativeMatrixAlpha[idx11];
}

QRgb QgsCubicRasterResampler::curveInterpolation( QRgb pt1, QRgb pt2, double t, double d1red, double d1green, double d1blue, double d1alpha,
    double d2red, double d2green, double d2blue, double d2alpha )
{
  //control points
  double p0r = qRed( pt1 ); double p1r = p0r + 0.333 * d1red; double p3r = qRed( pt2 ); double p2r = p3r - 0.333 * d2red;
  double p0g = qGreen( pt1 ); double p1g = p0g + 0.333 * d1green; double p3g = qGreen( pt2 ); double p2g = p3g - 0.333 * d2green;
  double p0b = qBlue( pt1 ); double p1b = p0b + 0.333 * d1blue; double p3b = qBlue( pt2 ); double p2b = p3b - 0.333 * d2blue;
  double p0a = qAlpha( pt1 ); double p1a = p0a + 0.333 * d1alpha; double p3a = qAlpha( pt2 ); double p2a = p3a - 0.333 * d2alpha;

  //bernstein polynomials
  double bp0 = calcBernsteinPoly( 3, 0, t );
  double bp1 = calcBernsteinPoly( 3, 1, t );
  double bp2 = calcBernsteinPoly( 3, 2, t );
  double bp3 = calcBernsteinPoly( 3, 3, t );

  int red = bp0 * p0r + bp1 * p1r + bp2 * p2r + bp3 * p3r;
  int green = bp0 * p0g + bp1 * p1g + bp2 * p2g + bp3 * p3g;
  int blue = bp0 * p0b + bp1 * p1b + bp2 * p2b + bp3 * p3b;
  int alpha = bp0 * p0a + bp1 * p1a + bp2 * p2a + bp3 * p3a;

  return qRgba( red, green, blue, alpha );
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
