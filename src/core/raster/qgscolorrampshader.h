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

#include "qgis_core.h"
#include "qgis.h"
#include <QColor>
#include <QVector>
#include <memory>

#include "qgscolorramp.h"
#include "qgsrasterinterface.h"
#include "qgsrastershaderfunction.h"
#include "qgsrectangle.h"

/**
 * \ingroup core
 * A ramp shader will color a raster pixel based on a list of values ranges in a ramp.
 */
class CORE_EXPORT QgsColorRampShader : public QgsRasterShaderFunction
{

  public:

    //! Supported methods for color interpolation.
    enum Type
    {
      Interpolated, //!< Interpolates the color between two class breaks linearly.
      Discrete,     //!< Assigns the color of the higher class for every pixel between two class breaks.
      Exact         //!< Assigns the color of the exact matching value in the color ramp item list
    };

    //! Classification modes used to create the color ramp shader
    enum ClassificationMode
    {
      Continuous = 1, //!< Uses breaks from color palette
      EqualInterval = 2, //!< Uses equal interval
      Quantile = 3 //!< Uses quantile (i.e. equal pixel) count
    };

    /**
     * Creates a new color ramp shader.
     * \param minimumValue minimum value for the raster shader
     * \param maximumValue maximum value for the raster shader
     * \param type interpolation type used
     * \param classificationMode method used to classify the color ramp shader
     * \param colorRamp vector color ramp used to classify the color ramp shader. Ownership is transferred to the shader.
     * \returns new QgsColorRampShader
     */
    QgsColorRampShader( double minimumValue = 0.0, double maximumValue = 255.0, QgsColorRamp *colorRamp SIP_TRANSFER = nullptr, Type type = Interpolated, ClassificationMode classificationMode = Continuous );

    /**
     * Copy constructor
     */
    QgsColorRampShader( const QgsColorRampShader &other );

    /**
     * Assignment operator
     */
    QgsColorRampShader &operator=( const QgsColorRampShader &other );

    //An entry for classification based upon value.
    //Such a classification is typically used for
    //single band layers where a pixel value represents
    //not a color but a quantity, e.g. temperature or elevation
    struct ColorRampItem
    {
      //! default constructor
      ColorRampItem() = default;
      //! convenience constructor
      ColorRampItem( double val, const QColor &col, const QString &lbl = QString() )
        : label( lbl )
        , value( val )
        , color( col )
      {}

      QString label;
      double value = 0;
      QColor color;

      // compare operator for sorting
      bool operator<( const QgsColorRampShader::ColorRampItem &other ) const { return value < other.value; }
    };

    //! Returns the custom colormap.
    QList<QgsColorRampShader::ColorRampItem> colorRampItemList() const { return mColorRampItemList.toList(); }

    //! Returns the color ramp type.
    Type colorRampType() const { return mColorRampType; }

    //! Returns the color ramp type as a string.
    QString colorRampTypeAsQString() const;

    //! Sets a custom colormap
    void setColorRampItemList( const QList<QgsColorRampShader::ColorRampItem> &list ); //TODO: sort on set

    //! Sets the color ramp type
    void setColorRampType( QgsColorRampShader::Type colorRampType );

    /**
     * Whether the color ramp contains any items
     * \since QGIS 3.4
     */
    bool isEmpty() const;

    /**
     * Gets the source color ramp
     * \see setSourceColorRamp()
     * \since QGIS 3.0
     */
    QgsColorRamp *sourceColorRamp() const SIP_FACTORY;

    /**
     * Set the source color ramp. Ownership is transferred to the shader.
     * \see sourceColorRamp()
     * \since QGIS 3.0
     */
    void setSourceColorRamp( QgsColorRamp *colorramp SIP_TRANSFER );

    //! Sets the color ramp type
    void setColorRampType( const QString &type );

    /**
     * Classify color ramp shader
     * \param classes number of classes
     * \param band raster band used in classification (only used in quantile mode)
     * \param extent extent used in classification (only used in quantile mode)
     * \param input raster input used in classification (only used in quantile mode)
     */
    void classifyColorRamp( int classes = 0, int band = -1, const QgsRectangle &extent = QgsRectangle(), QgsRasterInterface *input = nullptr );

    /**
     * Classify color ramp shader
     * \param band raster band used in classification (quantile mode only)
     * \param extent extent used in classification (quantile mode only)
     * \param input raster input used in classification (quantile mode only)
     */
    void classifyColorRamp( int band = -1, const QgsRectangle &extent = QgsRectangle(), QgsRasterInterface *input = nullptr ) SIP_PYNAME( classifyColorRampV2 );

    //! \brief Generates and new RGB value based on one input value
    bool shade( double value, int *returnRedValue SIP_OUT, int *returnGreenValue SIP_OUT, int *returnBlueValue SIP_OUT, int *returnAlphaValue SIP_OUT ) const override;

    //! \brief Generates and new RGB value based on original RGB value
    bool shade( double redValue, double greenValue,
                double blueValue, double alphaValue,
                int *returnRedValue SIP_OUT, int *returnGreenValue SIP_OUT,
                int *returnBlueValue SIP_OUT, int *returnAlphaValue SIP_OUT ) const override;

    void legendSymbologyItems( QList< QPair< QString, QColor > > &symbolItems SIP_OUT ) const override;

    /**
     * Writes configuration to a new DOM element
     * \since QGIS 3.4
     */
    QDomElement writeXml( QDomDocument &doc ) const;

    /**
     * Reads configuration from the given DOM element
     * \since QGIS 3.4
     */
    void readXml( const QDomElement &elem );

    //! Sets classification mode
    void setClassificationMode( ClassificationMode classificationMode ) { mClassificationMode = classificationMode; }

    //! Returns the classification mode
    ClassificationMode classificationMode() const { return mClassificationMode; }

    /**
     * Sets whether the shader should not render values out of range.
     * \param clip set to true to clip values which are out of range.
     * \see clip()
     */
    void setClip( bool clip ) { mClip = clip; }

    /**
     * Returns whether the shader will clip values which are out of range.
     * \see setClip()
     */
    bool clip() const { return mClip; }

  protected:

    //! Source color ramp
    std::unique_ptr<QgsColorRamp> mSourceColorRamp;

  private:

    /**
     * This vector holds the information for classification based on values.
     * Each item holds a value, a label and a color. The member
     * mDiscreteClassification holds if one color is applied for all values
     * between two class breaks (true) or if the item values are (linearly)
     * interpolated for values between the item values (false)*/
    QVector<QgsColorRampShader::ColorRampItem> mColorRampItemList;

    Type mColorRampType;
    ClassificationMode mClassificationMode;

    /**
     * Look up table to speed up finding the right color.
      * It is initialized on the first call to shade(). */
    mutable QVector<int> mLUT;
    mutable double mLUTOffset = 0.0;
    mutable double mLUTFactor = 1.0;
    mutable bool mLUTInitialized = false;

    //! Do not render values out of range
    bool mClip = false;
};

#endif
