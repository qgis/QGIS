/***************************************************************************
                          qgsderivativefilter.h  -  description
                          ---------------------
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

#ifndef QGSDERIVATIVEFILTER_H
#define QGSDERIVATIVEFILTER_H

#include "qgsninecellfilter.h"

/**Adds the ability to calculate derivatives in x- and y-directions. Needs to be subclassed (e.g. for slope and aspect)*/
class QgsDerivativeFilter: public QgsNineCellFilter
{
  public:
    QgsDerivativeFilter( const QString& inputFile, const QString& outputFile, const QString& outputFormat );
    virtual ~QgsDerivativeFilter();
    //to be implemented by subclasses
    virtual float processNineCellWindow( float* x11, float* x21, float* x31,
                                         float* x12, float* x22, float* x32,
                                         float* x13, float* x23, float* x33 ) = 0;

  protected:
    /**Calculates the first order derivative in x-direction according to Horn (1981)*/
    float calcFirstDerX( float* x11, float* x21, float* x31, float* x12, float* x22, float* x32, float* x13, float* x23, float* x33 );
    /**Calculates the first order derivative in y-direction according to Horn (1981)*/
    float calcFirstDerY( float* x11, float* x21, float* x31, float* x12, float* x22, float* x32, float* x13, float* x23, float* x33 );
};

#endif // QGSDERIVATIVEFILTER_H
