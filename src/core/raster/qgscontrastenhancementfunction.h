/* **************************************************************************
              qgscontrastenhancementfunction.h -  description
                       -------------------
begin                : Fri Nov 16 2007
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

#ifndef QGSCONTRASTENHANCEMENTFUNCTION_H
#define QGSCONTRASTENHANCEMENTFUNCTION_H

#include "qgis_core.h"
#include "qgis.h"

/**
 * \ingroup core
 * A contrast enhancement function is the base class for all raster contrast enhancements.
 *
 * The purpose of a contrast enhancement is to enhanceContrast or clip a pixel value into
 * a specified bounding range.
 */
class CORE_EXPORT QgsContrastEnhancementFunction
{

  public:
    QgsContrastEnhancementFunction( Qgis::DataType, double, double );
    QgsContrastEnhancementFunction( const QgsContrastEnhancementFunction &f );
    virtual ~QgsContrastEnhancementFunction() = default;

    /**
     * A customizable method that takes in a double \a value and returns a int between 0 and 255.
     */
    virtual int enhance( double value );

    /**
     * A customizable method to indicate if a pixel's value is within the displayable range.
     */
    virtual bool isValueInDisplayableRange( double value );

    /**
     * Sets the maximum \a value.
     * \see maximumValue()
     * \see setMinimumValue()
     */
    void setMaximumValue( double value );

    /**
     * Sets the minimum \a value.
     * \see minimumValue()
     * \see setMaximumValue()
     */
    void setMinimumValue( double value );

    /**
     * Returns the maximum value.
     * \see setMaximumValue()
     * \see minimumValue()
     * \since QGIS 3.2
     */
    double maximumValue() const { return mMaximumValue; }

    /**
     * Returns the minimum value.
     * \see setMinimumValue()
     * \see maximumValue()
     * \since QGIS 3.2
     */
    double minimumValue() const { return mMinimumValue; }

  protected:
    //! \brief User defineable maximum value for the band, used for enhanceContrasting
    double mMaximumValue;

    //! \brief User defineable minimum value for the band, used for enhanceContrasting
    double mMinimumValue;

    //! \brief Minimum maximum range for the band, used for enhanceContrasting
    double mMinimumMaximumRange;

    //! \brief Data type of the band
    Qgis::DataType mQgsRasterDataType;
};

#endif
