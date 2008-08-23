/* **************************************************************************
                qgsfreakoutshader.h -  description
                       -------------------
begin                : Fri Dec 28 2007
copyright            : (C) 2007 by Peter J. Ersts
email                : ersts@amnh.org

This class contains code that was originally part of the larger QgsRasterLayer
class originally created circa 2004 by T.Sutton, Gary E.Sherman, Steve Halasz
****************************************************************************/

/* **************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSFREAKOUTSHADER_H
#define QGSFREAKOUTSHADER_H

#include "qgsrastershaderfunction.h"

class CORE_EXPORT QgsFreakOutShader : public QgsRasterShaderFunction
{

  public:
    QgsFreakOutShader( double theMinimumValue = 0.0, double theMaximumValue = 255.0 );

    bool generateShadedValue( double, int*, int*, int* );
    bool generateShadedValue( double, double, double, int*, int*, int* );
    /** \brief Set the maximum value */
    void setMaximumValue( double );
    /** \brief Return the minimum value */
    void setMinimumValue( double );

  private:
    void setClassBreaks();

    double mBreakSize;
    double mClassBreakMin1;
    double mClassBreakMax1;
    double mClassBreakMin2;
    double mClassBreakMax2;
    double mClassBreakMin3;
};

#endif
