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

/**
 * \ingroup core
 * The raster shade function applies a shader to a pixel at render time -
 * typically used to render grayscale images as false color.
 */

#include "qgis_core.h"
#include "qgis_sip.h"
#include <QColor>
#include <QPair>

class CORE_EXPORT QgsRasterShaderFunction
{
#ifdef SIP_RUN
#include <qgscolorrampshader.h>
#endif


#ifdef SIP_RUN
    SIP_CONVERT_TO_SUBCLASS_CODE
    if ( dynamic_cast<QgsColorRampShader *>( sipCpp ) != NULL )
      sipType = sipType_QgsColorRampShader;
    else
      sipType = 0;
    SIP_END
#endif

  public:
    QgsRasterShaderFunction( double minimumValue = 0.0, double maximumValue = 255.0 );
    virtual ~QgsRasterShaderFunction() = default;

    /**
     * Sets the maximum \a value for the raster shader.
     * \see setMinimumValue()
     * \see maximumValue()
    */
    virtual void setMaximumValue( double value );

    /**
     * Sets the minimum \a value for the raster shader.
     * \see setMaximumValue()
     * \see minimumValue()
    */
    virtual void setMinimumValue( double value );

    /**
     * Generates an new RGBA value based on one input \a value.
     * \param value The original value to base a new RGBA value on
     * \param returnRedValue The red component of the new RGBA value
     * \param returnGreenValue The green component of the new RGBA value
     * \param returnBlueValue The blue component of the new RGBA value
     * \param returnAlpha The alpha component of the new RGBA value
     * \return TRUE if the return values are valid otherwise FALSE
    */
    virtual bool shade( double value,
                        int *returnRedValue SIP_OUT,
                        int *returnGreenValue SIP_OUT,
                        int *returnBlueValue SIP_OUT,
                        int *returnAlpha SIP_OUT ) const;

    /**
     * Generates an new RGBA value based on an original RGBA value.
     *
     * \param redValue The red component of the original value to base a new RGBA value on
     * \param greenValue The green component of the original value to base a new RGBA value on
     * \param blueValue The blue component of the original value to base a new RGBA value on
     * \param alphaValue The alpha component of the original value to base a new RGBA value on
     * \param returnRedValue The red component of the new RGBA value
     * \param returnGreenValue The green component of the new RGBA value
     * \param returnBlueValue The blue component of the new RGBA value
     * \param returnAlpha The alpha component of the new RGBA value
     * \return TRUE if the return values are valid otherwise FALSE
    */
    virtual bool shade( double redValue,
                        double greenValue,
                        double blueValue,
                        double alphaValue,
                        int *returnRedValue SIP_OUT,
                        int *returnGreenValue SIP_OUT,
                        int *returnBlueValue SIP_OUT,
                        int *returnAlpha SIP_OUT ) const;

    double minimumMaximumRange() const { return mMinimumMaximumRange; }

    /**
     * Returns the maximum value for the raster shader.
     * \see setMaximumValue()
     * \see minimumValue()
    */
    double minimumValue() const { return mMinimumValue; }

    /**
     * Returns the minimum value for the raster shader.
     * \see setMinimumValue()
     * \see maximumValue()
    */
    double maximumValue() const { return mMaximumValue; }

    /**
     * Returns legend symbology items if provided by renderer.
     */
    virtual void legendSymbologyItems( QList< QPair< QString, QColor > > &symbolItems SIP_OUT ) const { Q_UNUSED( symbolItems ); }

  protected:
    //! \brief User defineable maximum value for the shading function
    double mMaximumValue;

    //! \brief User defineable minimum value for the shading function
    double mMinimumValue;

    //! \brief Minimum maximum range for the shading function
    double mMinimumMaximumRange;
};
#endif
