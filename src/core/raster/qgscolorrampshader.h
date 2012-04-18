/* **************************************************************************
                qgscolorrampshader.h -  description
                       -------------------
begin                : Fri Dec 28 2007
copyright            : (C) 2007 by Peter J. Ersts
email                : ersts@amnh.org

This class is based off of code that was originally written by Marco Hugentobler and
originally part of the larger QgsRasterLayer class
****************************************************************************/

/* **************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSCOLORRAMPSHADER_H
#define QGSCOLORRAMPSHADER_H

#include <QColor>
#include <QMap>

#include "qgsrastershaderfunction.h"

/** \ingroup core
 * A ramp shader will color a raster pixel based on a list of values ranges in a ramp.
 */
class CORE_EXPORT QgsColorRampShader : public QgsRasterShaderFunction
{

  public:
    QgsColorRampShader( double theMinimumValue = 0.0, double theMaximumValue = 255.0 );

    //An entry for classification based upon value.
    //Such a classification is typically used for
    //single band layers where a pixel value represents
    //not a color but a quantity, e.g. temperature or elevation
    struct ColorRampItem
    {
      //! default constructor
      ColorRampItem() {}
      //! convenience constructor - added in v1.6
      ColorRampItem( double val, QColor col, QString lbl = QString() ) : label( lbl ), value( val ), color( col ) {}

      QString label;
      double value;
      QColor color;

      // compare operator for sorting
      bool operator<( const ColorRampItem& other ) const { return value < other.value; }
    };

    enum ColorRamp_TYPE
    {
      INTERPOLATED,
      DISCRETE,
      EXACT
    };

    /** \brief Get the custom colormap*/
    QList<QgsColorRampShader::ColorRampItem> colorRampItemList() const {return mColorRampItemList;}

    /** \brief Get the color ramp type */
    QgsColorRampShader::ColorRamp_TYPE colorRampType() const {return mColorRampType;}

    /** \brief Get the color ramp type as a string */
    QString colorRampTypeAsQString();

    /** \brief Get the maximum size the color cache can be*/
    int maximumColorCacheSize() { return mMaximumColorCacheSize; }

    /** \brief Set custom colormap */
    void setColorRampItemList( const QList<QgsColorRampShader::ColorRampItem>& theList ); //TODO: sort on set

    /** \brief Set the color ramp type*/
    void setColorRampType( QgsColorRampShader::ColorRamp_TYPE theColorRampType );

    /** \brief Set the color ramp type*/
    void setColorRampType( QString );

    /** \brief Set the maximum size the color cache can be */
    void setMaximumColorCacheSize( int theSize ) { mMaximumColorCacheSize = theSize; }

    /** \brief Generates and new RGB value based on one input value */
    bool shade( double, int*, int*, int* );

    /** \brief Generates and new RGB value based on original RGB value */
    bool shade( double, double, double, int*, int*, int* );

    void legendSymbologyItems( QList< QPair< QString, QColor > >& symbolItems ) const;

  private:
    /** Current index from which to start searching the color table*/
    int mCurrentColorRampItemIndex;

    //TODO: Consider pulling this out as a separate class and internally storing as a QMap rather than a QList
    /** This vector holds the information for classification based on values.
     * Each item holds a value, a label and a color. The member
     * mDiscreteClassification holds if one color is applied for all values
     * between two class breaks (true) or if the item values are (linearly)
     * interpolated for values between the item values (false)*/
    QList<QgsColorRampShader::ColorRampItem> mColorRampItemList;

    /** \brief The color ramp type */
    QgsColorRampShader::ColorRamp_TYPE mColorRampType;

    /** \brief Cache of values that have already been looked up */
    QMap<double, QColor> mColorCache;

    /** Maximum size of the color cache. The color cache could eat a ton of
     * memory if you have 32-bit data */
    int mMaximumColorCacheSize;

    /** Gets the color for a pixel value from the classification vector
     * mValueClassification. Assigns the color of the lower class for every
     * pixel between two class breaks.*/
    bool discreteColor( double, int*, int*, int* );

    /** Gets the color for a pixel value from the classification vector
     * mValueClassification. Assigns the color of the exact matching value in
     * the color ramp item list */
    bool exactColor( double, int*, int*, int* );

    /** Gets the color for a pixel value from the classification vector
     * mValueClassification. Interpolates the color between two class breaks
     * linearly.*/
    bool interpolatedColor( double, int*, int*, int* );
};

#endif
