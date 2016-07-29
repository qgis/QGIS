/***************************************************************************
                          qgshillshadefilter.h  -  description
                          --------------------------------
    begin                : September 26th, 2011
    copyright            : (C) 2011 by Marco Hugentobler
    email                : marco dot hugentobler at sourcepole dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSHILLSHADEFILTER_H
#define QGSHILLSHADEFILTER_H

#include "qgsderivativefilter.h"

/** \ingroup analysis
 * \class QgsHillshadeFilter
 */
class ANALYSIS_EXPORT QgsHillshadeFilter: public QgsDerivativeFilter
{
  public:
    QgsHillshadeFilter( const QString& inputFile, const QString& outputFile, const QString& outputFormat, double lightAzimuth = 300,
                        double lightAngle = 40 );
    ~QgsHillshadeFilter();

    /** Calculates output value from nine input values. The input values and the output value can be equal to the
    nodata value if not present or outside of the border. Must be implemented by subclasses*/
    float processNineCellWindow( float* x11, float* x21, float* x31,
                                 float* x12, float* x22, float* x32,
                                 float* x13, float* x23, float* x33 ) override;

    float lightAzimuth() const { return mLightAzimuth; }
    void setLightAzimuth( float azimuth ) { mLightAzimuth = azimuth; }
    float lightAngle() const { return mLightAngle; }
    void setLightAngle( float angle ) { mLightAngle = angle; }

  private:
    float mLightAzimuth;
    float mLightAngle;
};

#endif // QGSHILLSHADEFILTER_H
