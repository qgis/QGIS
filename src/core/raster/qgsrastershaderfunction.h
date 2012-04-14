/* **************************************************************************
                qgsrastershaderfunction.h -  description
                       -------------------
begin                : Fri Dec 28 2007
copyright            : (C) 2007 by Peter J. Ersts
email                : ersts@amnh.org

****************************************************************************/

/* **************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#ifndef QGSRASTERSHADERFUNCTION_H
#define QGSRASTERSHADERFUNCTION_H
/** \ingroup core
 * The raster shade function applies a shader to a pixel at render time -
 * typically used to render grayscale images as false color.
 */

#include <QColor>
#include <QPair>

class CORE_EXPORT QgsRasterShaderFunction
{

  public:
    QgsRasterShaderFunction( double theMinimumValue = 0.0, double theMaximumValue = 255.0 );
    virtual ~QgsRasterShaderFunction() {}

    /** \brief Set the maximum value */
    virtual void setMaximumValue( double );

    /** \brief Return the minimum value */
    virtual void setMinimumValue( double );

    /** \brief generates and new RGB value based on one input value */
    virtual bool shade( double, int*, int*, int* );

    /** \brief generates and new RGB value based on original RGB value */
    virtual bool shade( double, double, double, int*, int*, int* );

    double minimumMaximumRange() const { return mMinimumMaximumRange; }

    double minimumValue() const { return mMinimumValue; }
    double maximumValue() const { return mMaximumValue; }

    virtual void legendSymbologyItems( QList< QPair< QString, QColor > >& symbolItems ) const { Q_UNUSED( symbolItems ); }

  protected:
    /** \brief User defineable maximum value for the shading function */
    double mMaximumValue;

    /** \brief User defineable minimum value for the shading function */
    double mMinimumValue;

    /** \brief Minimum maximum range for the shading function */
    double mMinimumMaximumRange;
};
#endif
