/***************************************************************************
                          qgsslopefilter.h  -  description
                          --------------------------------
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

#ifndef QGSSLOPEFILTER_H
#define QGSSLOPEFILTER_H

#include "qgsderivativefilter.h"

/**Calculates slope values in a window of 3x3 cells based on first order derivatives in x- and y- directions*/
class ANALYSIS_EXPORT QgsSlopeFilter: public QgsDerivativeFilter
{
  public:
    QgsSlopeFilter( const QString& inputFile, const QString& outputFile, const QString& outputFormat );
    ~QgsSlopeFilter();

    /**Calculates output value from nine input values. The input values and the output value can be equal to the
      nodata value if not present or outside of the border. Must be implemented by subclasses*/
    float processNineCellWindow( float* x11, float* x21, float* x31,
                                 float* x12, float* x22, float* x32,
                                 float* x13, float* x23, float* x33 );
};

#endif // QGSSLOPEFILTER_H
