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

#include <limits>

class QgsContrastEnhancementFunction;

/** \ingroup core
 * Manipulates raster pixel values so that they enhanceContrast or clip into a
 * specified numerical range according to the specified
 * ContrastEnhancementAlgorithm.
 */
class CORE_EXPORT QgsContrastEnhancement
{

  public:

    /** \brief This enumerator describes the types of contrast enhancement algorithms that can be used.  */
    enum ContrastEnhancementAlgorithm
    {
      NoEnhancement,                  //this should be the default color scaling algorithm
      StretchToMinimumMaximum,        //linear histogram enhanceContrast
      StretchAndClipToMinimumMaximum,
      ClipToMinimumMaximum,
      UserDefinedEnhancement
    };

    /** These are exactly the same as GDAL pixel data types
     ** This was added so that the python bindings could be built,
     ** which initially was a problem because GDALDataType was passed
     ** around as an argument to numerous method, including the constructor.
     **
     ** It seems like there should be a better way to do this...
     */
    enum QgsRasterDataType
    {
      QGS_Unknown = 0,
      /*! Eight bit unsigned integer */           QGS_Byte = 1,
      /*! Sixteen bit unsigned integer */         QGS_UInt16 = 2,
      /*! Sixteen bit signed integer */           QGS_Int16 = 3,
      /*! Thirty two bit unsigned integer */      QGS_UInt32 = 4,
      /*! Thirty two bit signed integer */        QGS_Int32 = 5,
      /*! Thirty two bit floating point */        QGS_Float32 = 6,
      /*! Sixty four bit floating point */        QGS_Float64 = 7,
      /*! Complex Int16 */                        QGS_CInt16 = 8,
      /*! Complex Int32 */                        QGS_CInt32 = 9,
      /*! Complex Float32 */                      QGS_CFloat32 = 10,
      /*! Complex Float64 */                      QGS_CFloat64 = 11,
      QGS_TypeCount = 12          /* maximum type # + 1 */
    };

    QgsContrastEnhancement( QgsContrastEnhancement::QgsRasterDataType theDatatype = QGS_Byte );
    ~QgsContrastEnhancement();

    /*
     *
     * Static methods
     *
     */
    /** \brief Helper function that returns the maximum possible value for a GDAL data type */
    static double maximumValuePossible( QgsRasterDataType );

    /** \brief Helper function that returns the minimum possible value for a GDAL data type */
    static double minimumValuePossible( QgsRasterDataType );

    /*
     *
     * Non-Static Inline methods
     *
     */
    /** \brief Return the maximum value for the contrast enhancement range. */
    double maximumValue() const { return mMaximumValue; }

    /** \brief Return the minimum value for the contrast enhancement range. */
    double minimumValue() const { return mMinimumValue; }

    ContrastEnhancementAlgorithm contrastEnhancementAlgorithm() const { return mContrastEnhancementAlgorithm; }

    /*
     *
     * Non-Static methods
     *
     */
    /** \brief Apply the contrast enhancement to a value. Return values are 0 - 254, -1 means the pixel was clipped and should not be displayed */
    int enhanceContrast( double );

    /** \brief Return true if pixel is in stretable range, false if pixel is outside of range (i.e., clipped) */
    bool isValueInDisplayableRange( double );

    /** \brief Set the contrast enhancement algorithm */
    void setContrastEnhancementAlgorithm( ContrastEnhancementAlgorithm, bool generateTable = true );

    /** \brief A public method that allows the user to set their own custom contrast enhancment function */
    void setContrastEnhancementFunction( QgsContrastEnhancementFunction* );

    /** \brief Set the maximum value for the contrast enhancement range. */
    void setMaximumValue( double, bool generateTable = true );

    /** \brief Return the minimum value for the contrast enhancement range. */
    void setMinimumValue( double, bool generateTable = true );

  private:
    /** \brief Current contrast enhancement algorithm */
    ContrastEnhancementAlgorithm mContrastEnhancementAlgorithm;

    /** \brief Pointer to the contrast enhancement function */
    QgsContrastEnhancementFunction* mContrastEnhancementFunction;

    /** \brief Flag indicating if the lookup table needs to be regenerated */
    bool mEnhancementDirty;

    /** \brief Scalar so that values can be used as array indicies */
    double mLookupTableOffset;

    /** \brief Pointer to the lookup table */
    int *mLookupTable;

    /** \brief User defineable minimum value for the band, used for enhanceContrasting */
    double mMinimumValue;

    /** \brief user defineable maximum value for the band, used for enhanceContrasting */
    double mMaximumValue;

    /** \brief Data type of the band */
    QgsRasterDataType mRasterDataType;

    /** \brief Maximum range of values for a given data type */
    double mRasterDataTypeRange;



    /** \brief Method to generate a new lookup table */
    bool generateLookupTable();

    /** \brief Method to calculate the actual enhanceContrasted value(s) */
    int calculateContrastEnhancementValue( double );
};

#endif
