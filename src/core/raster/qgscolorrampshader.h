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
      QString label;
      double value;
      QColor color;
    };

    enum COLOR_RAMP_TYPE
    {
      INTERPOLATED,
      DISCRETE,
      EXACT
    };

    bool generateShadedValue( double, int*, int*, int* );
    bool generateShadedValue( double, double, double, int*, int*, int* );

    /** Get the custom colormap*/
    QList<QgsColorRampShader::ColorRampItem> getColorRampItemList() const {return mColorRampItemList;}

    /**Get the color ramp type */
    QgsColorRampShader::COLOR_RAMP_TYPE getColorRampType() {return mColorRampType;}
    QString getColorRampTypeAsQString();

    /**Get the maximum size the color cache can be*/
    int getMaximumColorCacheSize() { return mMaximumColorCacheSize; }

    /**Set custom colormap */
    void setColorRampItemList( const QList<QgsColorRampShader::ColorRampItem>& theList ) { mColorRampItemList = theList; } //TODO: sort on set

    /**Set the color ramp type*/
    void setColorRampType( QgsColorRampShader::COLOR_RAMP_TYPE theColorRampType );

    /**Set the color ramp type*/
    void setColorRampType( QString );

    /**Set the maximum size the color cache can be */
    void setMaximumColorCacheSize( int theSize ) { mMaximumColorCacheSize = theSize; }



  private:
    /**Gets the color for a pixel value from the classification vector mValueClassification. Assigns the color of the lower class for every pixel between two class breaks.*/
    bool getDiscreteColor( double, int*, int*, int* );
    /**Gets the color for a pixel value from the classification vector mValueClassification. Assigns the color of the exact matching value in the color ramp item list */
    bool getExactColor( double, int*, int*, int* );
    /**Gets the color for a pixel value from the classification vector mValueClassification. Interpolates the color between two class breaks linearly.*/
    bool getInterpolatedColor( double, int*, int*, int* );

    /**This vector holds the information for classification based on values. Each item holds a value, a label and a color. The member mDiscreteClassification holds if one color is applied for all values between two class breaks (true) or if the item values are (linearly) interpolated for values between the item values (false)*/
    QList<QgsColorRampShader::ColorRampItem> mColorRampItemList;

    QgsColorRampShader::COLOR_RAMP_TYPE mColorRampType;
    QMap<double, QColor> mColorCache;

    /** Maximum size of the color cache. The color cache could eat a ton of memory if you have 32-bit data */
    int mMaximumColorCacheSize;
    /** Current index to start searching the color table*/
    int mCurrentColorRampItemIndex;
};

#endif
