/***************************************************************************
                          qgsaspectfilter.h  -  description
                          ---------------------------------
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

#ifndef QGSASPECTFILTER_H
#define QGSASPECTFILTER_H

#include "qgsderivativefilter.h"
#include "qgis_analysis.h"

/**
 * \ingroup analysis
 * \brief Calculates aspect values in a window of 3x3 cells based on first order derivatives in x- and y- directions.
 *
 * Direction is clockwise starting from north.
*/
class ANALYSIS_EXPORT QgsAspectFilter : public QgsDerivativeFilter
{
  public:
    QgsAspectFilter( const QString &inputFile, const QString &outputFile, const QString &outputFormat );

    float processNineCellWindow( float *x11, float *x21, float *x31, float *x12, float *x22, float *x32, float *x13, float *x23, float *x33 ) override;


#ifdef HAVE_OPENCL
  private:
    const QString openClProgramBaseName() const override
    {
      return QStringLiteral( "aspect" );
    }
#endif
};

#endif // QGSASPECTFILTER_H
