/***************************************************************************
                         qgscubicrasterresampler.h
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

#ifndef QGSCUBICRASTERRESAMPLER_H
#define QGSCUBICRASTERRESAMPLER_H

#include "qgsrasterresampler.h"

class QgsCubicRasterResampler: public QgsRasterResampler
{
  public:
    QgsCubicRasterResampler();
    ~QgsCubicRasterResampler();
    void resample( const QImage& srcImage, QImage& dstImage ) const;

  private:
    static void xDerivativeMatrix( int nCols, int nRows, double* matrix, const int* colorMatrix );
    template <class T> void yDerivativeMatrix( int nCols, int nRows, double* dstMatrix, const T* srcMatrix ) const;
    static int cubicInterpolation( double p1, double p2, double p3, double p4, double p1x, double p2x, double p3x, double p4x,
                                   double p1y, double p2y, double p3y, double p4y, double p1xy, double p2xy, double p3xy, double p4xy );
};

template <class T> void QgsCubicRasterResampler::yDerivativeMatrix( int nCols, int nRows, double* dstMatrix, const T* srcMatrix ) const
{
  double val;
  int index = 0;

  for( int i = 0; i < nRows; ++i )
  {
    for( int j = 0; j < nCols; ++j )
    {
      if( i == 0 )
      {
        val = srcMatrix[ index + nRows ] - srcMatrix[ index ];
      }
      else if( i == nRows - 1 )
      {
        val = srcMatrix[ index ] - srcMatrix[ index - nRows ];
      }
      else
      {
        val = ( srcMatrix[ index + nRows ] - srcMatrix[ index - nRows ] ) / 2.0;
      }
      dstMatrix[index] = val;
      ++index;
    }
  }
}

#endif // QGSCUBICRASTERRESAMPLER_H
