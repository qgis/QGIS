/***************************************************************************
                          qgstotalcurvaturefilter.h  -  description
                             -------------------
    begin                : August 21th, 2009
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

#ifndef QGSTOTALCURVATUREFILTER_H
#define QGSTOTALCURVATUREFILTER_H

#include "qgsninecellfilter.h"
#include "qgis_analysis.h"

/**
 * \ingroup analysis
 * \brief Calculates total curvature as described by Wilson, Gallant (2000): terrain analysis.
*/
class ANALYSIS_EXPORT QgsTotalCurvatureFilter : public QgsNineCellFilter
{
  public:
    QgsTotalCurvatureFilter( const QString &inputFile, const QString &outputFile, const QString &outputFormat );

  protected:
    float processNineCellWindow( float *x11, float *x21, float *x31, float *x12, float *x22, float *x32, float *x13, float *x23, float *x33 ) override;
};

#endif // QGSTOTALCURVATUREFILTER_H
