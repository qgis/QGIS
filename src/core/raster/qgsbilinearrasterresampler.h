/***************************************************************************
                         qgsbilinearrasterresampler.h
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

#ifndef QGSBILINEARRASTERRESAMPLER_H
#define QGSBILINEARRASTERRESAMPLER_H

#include "qgsrasterresampler.h"
#include <QColor>

class QgsBilinearRasterResampler: public QgsRasterResampler
{
  public:
    QgsBilinearRasterResampler();
    ~QgsBilinearRasterResampler();

    void resample( const QImage& srcImage, QImage& dstImage );
    QString type() const { return "bilinear"; }

  private:
    QRgb resampleColorValue( double u, double v, QRgb col1, QRgb col2, QRgb col3, QRgb col4 ) const;
    QRgb resampleColorValue( double u, QRgb col1, QRgb col2 ) const;
    inline double bilinearInterpolation( double u, double v, int val1, int val2, int val3, int val4 ) const;
};

double QgsBilinearRasterResampler::bilinearInterpolation( double u, double v, int val1, int val2, int val3, int val4 ) const
{
  return ( val1 * ( 1 - u ) * ( 1 - v ) + val2 * ( 1 - v ) * u + val4 * ( 1 - u ) * v + val3 * u * v );
}

#endif // QGSBILINEARRASTERRESAMPLER_H
