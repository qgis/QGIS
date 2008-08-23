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

#include "qgscontrastenhancement.h"

class CORE_EXPORT QgsContrastEnhancementFunction
{

  public:
    QgsContrastEnhancementFunction( QgsContrastEnhancement::QgsRasterDataType, double, double );
    virtual ~QgsContrastEnhancementFunction() {}

    /** \brief Mustator for the maximum value */
    void setMaximumValue( double );
    /** \brief Mutator for the minimum value */
    void setMinimumValue( double );

    /** \brief A customizable method that takes in a double and returns a int between 0 and 255 */
    virtual int enhanceValue( double );
    /** \brief A customicable method to indicate if the pixels is displayable */
    virtual bool isValueInDisplayableRange( double );

  protected:
    /** \brief User defineable maximum value for the band, used for stretching */
    double mMaximumValue;
    /** \brief User defineable minimum value for the band, used for stretching */
    double mMinimumValue;
    /** \brief Minimum maximum range for the band, used for stretching */
    double mMinimumMaximumRange;
    /** \brief Data type of the band */
    QgsContrastEnhancement::QgsRasterDataType mQgsRasterDataType;
};

#endif
