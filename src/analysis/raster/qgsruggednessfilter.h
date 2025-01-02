/***************************************************************************
                          qgsruggednessfilter.h  -  description
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

#ifndef QGSRUGGEDNESSFILTER_H
#define QGSRUGGEDNESSFILTER_H

#include "qgsninecellfilter.h"
#include "qgis_analysis.h"

/**
 * \ingroup analysis
 * \brief Calculates the ruggedness index based on a 3x3 moving window.
 *
 * Algorithm from Riley et al. 1999: A terrain ruggedness index that quantifies topographic heterogeneity
*/
class ANALYSIS_EXPORT QgsRuggednessFilter : public QgsNineCellFilter
{
  public:
    QgsRuggednessFilter( const QString &inputFile, const QString &outputFile, const QString &outputFormat );

  protected:
    float processNineCellWindow( float *x11, float *x21, float *x31, float *x12, float *x22, float *x32, float *x13, float *x23, float *x33 ) override;

#ifdef HAVE_OPENCL
  private:
    QgsRuggednessFilter();

    virtual const QString openClProgramBaseName() const override
    {
      return QStringLiteral( "ruggedness" );
    }
#endif
};

#endif // QGSRUGGEDNESSFILTER_H
