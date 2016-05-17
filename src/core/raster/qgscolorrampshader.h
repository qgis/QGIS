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
#include <QVector>

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
      ColorRampItem() : value( 0 ) {}
      //! convenience constructor
      ColorRampItem( double val, const QColor& col, const QString& lbl = QString() )
          : label( lbl )
          , value( val )
          , color( col )
      {}

      QString label;
      double value;
      QColor color;

      // compare operator for sorting
      bool operator<( const ColorRampItem& other ) const { return value < other.value; }
    };

    /** Supported methods for color interpolation. */
    enum ColorRamp_TYPE
    {
      INTERPOLATED, //!< Interpolates the color between two class breaks linearly.
      DISCRETE,     //!< Assigns the color of the higher class for every pixel between two class breaks.
      EXACT         //!< Assigns the color of the exact matching value in the color ramp item list
    };

    /** \brief Get the custom colormap*/
    QList<QgsColorRampShader::ColorRampItem> colorRampItemList() const {return mColorRampItemList.toList();}

    /** \brief Get the color ramp type */
    QgsColorRampShader::ColorRamp_TYPE colorRampType() const {return mColorRampType;}

    /** \brief Get the color ramp type as a string */
    QString colorRampTypeAsQString();

    /** \brief Get the maximum size the color cache can be
     * @deprecated will be removed in QGIS 3.0. Color cache is not used anymore.
     */
    Q_DECL_DEPRECATED int maximumColorCacheSize() { return 0; }

    /** \brief Set custom colormap */
    void setColorRampItemList( const QList<QgsColorRampShader::ColorRampItem>& theList ); //TODO: sort on set

    /** \brief Set the color ramp type*/
    void setColorRampType( QgsColorRampShader::ColorRamp_TYPE theColorRampType );

    /** \brief Set the color ramp type*/
    void setColorRampType( const QString& theType );

    /** \brief Set the maximum size the color cache can be
     * @deprecated will be removed in QGIS 3.0. Color cache is not used anymore.
     */
    Q_DECL_DEPRECATED void setMaximumColorCacheSize( int theSize ) { Q_UNUSED( theSize ); }

    /** \brief Generates and new RGB value based on one input value */
    bool shade( double, int*, int*, int*, int* ) override;

    /** \brief Generates and new RGB value based on original RGB value */
    bool shade( double, double, double, double, int*, int*, int*, int* ) override;

    void legendSymbologyItems( QList< QPair< QString, QColor > >& symbolItems ) const override;

    /** Sets whether the shader should not render values out of range.
     * @param clip set to true to clip values which are out of range.
     * @see clip()
     */
    void setClip( bool clip ) { mClip = clip; }

    /** Returns whether the shader will clip values which are out of range.
     * @see setClip()
     */
    bool clip() const { return mClip; }

  private:
    /** This vector holds the information for classification based on values.
     * Each item holds a value, a label and a color. The member
     * mDiscreteClassification holds if one color is applied for all values
     * between two class breaks (true) or if the item values are (linearly)
     * interpolated for values between the item values (false)*/
    QVector<QgsColorRampShader::ColorRampItem> mColorRampItemList;

    /** \brief The color ramp type */
    QgsColorRampShader::ColorRamp_TYPE mColorRampType;

    /** Look up table to speed up finding the right color.
      * It is initialized on the first call to shade(). */
    QVector<int> mLUT;
    double mLUTOffset;
    double mLUTFactor;
    bool mLUTInitialized;

    /** Do not render values out of range */
    bool mClip;
};

#endif
