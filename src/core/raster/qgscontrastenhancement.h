/* **************************************************************************
                qgscontrastenhancement.h -  description
                       -------------------
begin                : Mon Oct 22 2007
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

#ifndef QGSCONTRASTENHANCEMENT_H
#define QGSCONTRASTENHANCEMENT_H

#include "qgis_core.h"
#include <limits>

#include "qgis.h"
#include "qgsraster.h"
#include <memory>

class QgsContrastEnhancementFunction;
class QDomDocument;
class QDomElement;
class QString;

/**
 * \ingroup core
 * Manipulates raster pixel values so that they enhanceContrast or clip into a
 * specified numerical range according to the specified
 * ContrastEnhancementAlgorithm.
 */
class CORE_EXPORT QgsContrastEnhancement
{

  public:

    //! \brief This enumerator describes the types of contrast enhancement algorithms that can be used.
    enum ContrastEnhancementAlgorithm
    {
      NoEnhancement,                  //this should be the default color scaling algorithm
      StretchToMinimumMaximum,        //linear histogram enhanceContrast
      StretchAndClipToMinimumMaximum,
      ClipToMinimumMaximum,
      UserDefinedEnhancement
    };

    QgsContrastEnhancement( Qgis::DataType datatype = Qgis::Byte );
    QgsContrastEnhancement( const QgsContrastEnhancement &ce );
    ~QgsContrastEnhancement();

    const QgsContrastEnhancement &operator=( const QgsContrastEnhancement & ) = delete;

    /*
     *
     * Static methods
     *
     */

    /**
     * Helper function that returns the maximum possible value for a GDAL data type.
     */
    static double maximumValuePossible( Qgis::DataType );

    /**
     * Helper function that returns the minimum possible value for a GDAL data type.
     */
    static double minimumValuePossible( Qgis::DataType );

    /**
     * Returns a string to serialize ContrastEnhancementAlgorithm.
     */
    static QString contrastEnhancementAlgorithmString( ContrastEnhancementAlgorithm algorithm );

    /**
     * Deserialize ContrastEnhancementAlgorithm.
     */
    static ContrastEnhancementAlgorithm contrastEnhancementAlgorithmFromString( const QString &contrastEnhancementString );

    /*
     *
     * Non-Static Inline methods
     *
     */
    //! Returns the maximum value for the contrast enhancement range.
    double maximumValue() const { return mMaximumValue; }

    //! Returns the minimum value for the contrast enhancement range.
    double minimumValue() const { return mMinimumValue; }

    ContrastEnhancementAlgorithm contrastEnhancementAlgorithm() const { return mContrastEnhancementAlgorithm; }

    /*
     *
     * Non-Static methods
     *
     */

    /**
     * Applies the contrast enhancement to a \a value. Return values are 0 - 254, -1 means the pixel was clipped and should not be displayed.
     */
    int enhanceContrast( double value );

    /**
     * Returns true if a pixel \a value is in displayable range, false if pixel
     * is outside of range (i.e. clipped).
     */
    bool isValueInDisplayableRange( double value );

    /**
     * Sets the contrast enhancement \a algorithm.
     *
     * The \a generateTable parameter is optional and is for performance improvements.
     * If you know you are immediately going to set the Minimum or Maximum value, you
     * can elect to not generate the lookup tale. By default it will be generated.
    */
    void setContrastEnhancementAlgorithm( ContrastEnhancementAlgorithm algorithm, bool generateTable = true );

    /**
     * Allows the user to set their own custom contrast enhancement \a function. Ownership of
     * \a function is transferred.
    */
    void setContrastEnhancementFunction( QgsContrastEnhancementFunction *function SIP_TRANSFER );

    /**
     * Sets the maximum \a value for the contrast enhancement range.
     *
     * The \a generateTable parameter is optional and is for performance improvements.
     * If you know you are immediately going to set the minimum value or the contrast
     * enhancement algorithm, you can elect to not generate the lookup table.
     * By default it will be generated.
     *
     * \see setMinimumValue()
    */
    void setMaximumValue( double value, bool generateTable = true );

    /**
     * Sets the minimum \a value for the contrast enhancement range.
     *
     * The \a generateTable parameter is optional and is for performance improvements.
     * If you know you are immediately going to set the maximum value or the contrast
     * enhancement algorithm, you can elect to not generate the lookup table.
     * By default it will be generated.
     *
     * \see setMaximumValue()
    */
    void setMinimumValue( double value, bool generateTable = true );

    void writeXml( QDomDocument &doc, QDomElement &parentElem ) const;

    void readXml( const QDomElement &elem );

  private:
#ifdef SIP_RUN
    const QgsContrastEnhancement &operator=( const QgsContrastEnhancement & );
#endif

    //! \brief Current contrast enhancement algorithm
    ContrastEnhancementAlgorithm mContrastEnhancementAlgorithm = NoEnhancement;

    //! \brief Pointer to the contrast enhancement function
    std::unique_ptr< QgsContrastEnhancementFunction > mContrastEnhancementFunction;

    //! \brief Flag indicating if the lookup table needs to be regenerated
    bool mEnhancementDirty = false;

    //! \brief Scalar so that values can be used as array indices
    double mLookupTableOffset;

    //! \brief Pointer to the lookup table
    int *mLookupTable = nullptr;

    //! \brief User defineable minimum value for the band, used for enhanceContrasting
    double mMinimumValue;

    //! \brief user defineable maximum value for the band, used for enhanceContrasting
    double mMaximumValue;

    //! \brief Data type of the band
    Qgis::DataType mRasterDataType;

    //! \brief Maximum range of values for a given data type
    double mRasterDataTypeRange;

    //! Generates a new lookup table
    bool generateLookupTable();

    //! \brief Method to calculate the actual enhanceContrasted value(s)
    int calculateContrastEnhancementValue( double );

};

#endif
