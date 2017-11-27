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

    //! \brief Set the maximum value
    virtual void setMaximumValue( double );

    //! \brief Return the minimum value
    virtual void setMinimumValue( double );

    //! \brief generates and new RGBA value based on one input value
    virtual bool shade( double value,
                        int *returnRedValue SIP_OUT,
                        int *returnGreenValue SIP_OUT,
                        int *returnBlueValue SIP_OUT,
                        int *returnAlpha SIP_OUT );

    //! \brief generates and new RGBA value based on original RGBA value
    virtual bool shade( double redValue,
                        double greenValue,
                        double blueValue,
                        double alphaValue,
                        int *returnRedValue SIP_OUT,
                        int *returnGreenValue SIP_OUT,
                        int *returnBlueValue SIP_OUT,
                        int *returnAlpha SIP_OUT );

    double minimumMaximumRange() const { return mMinimumMaximumRange; }

    double minimumValue() const { return mMinimumValue; }
    double maximumValue() const { return mMaximumValue; }

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
