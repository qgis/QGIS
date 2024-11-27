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
#include "qgis_sip.h"
#include <QColor>
#include <QVector>
#include <memory>

#include "qgis.h"
#include "qgsrastershaderfunction.h"
#include "qgsrectangle.h"
#include "qgsreadwritecontext.h"
#include "qgscolorramplegendnodesettings.h"

class QgsColorRamp;
class QgsRasterInterface;

/**
 * \ingroup core
 * \brief A ramp shader will color a raster pixel based on a list of values ranges in a ramp.
 */
class CORE_EXPORT QgsColorRampShader : public QgsRasterShaderFunction
{

  public:

    /**
     * Creates a new color ramp shader.
     * \param minimumValue minimum value for the raster shader
     * \param maximumValue maximum value for the raster shader
     * \param type interpolation type used
     * \param classificationMode method used to classify the color ramp shader
     * \param colorRamp vector color ramp used to classify the color ramp shader. Ownership is transferred to the shader.
     */
    QgsColorRampShader( double minimumValue = 0.0, double maximumValue = 255.0, QgsColorRamp *colorRamp SIP_TRANSFER = nullptr, Qgis::ShaderInterpolationMethod type = Qgis::ShaderInterpolationMethod::Linear, Qgis::ShaderClassificationMethod classificationMode = Qgis::ShaderClassificationMethod::Continuous );

    ~QgsColorRampShader() override;

    QgsColorRampShader( const QgsColorRampShader &other );
    QgsColorRampShader &operator=( const QgsColorRampShader &other );

    bool operator==( const QgsColorRampShader &other ) const
    {
      if ( mColorRampItemList.count() != other.mColorRampItemList.count() ||
           mClassificationMode != other.mClassificationMode ||
           mColorRampType != other.mColorRampType )
      {
        return false;
      }
      for ( int i = 0; i < mColorRampItemList.count(); ++i )
      {
        if ( mColorRampItemList.at( i ) != other.mColorRampItemList.at( i ) ) return false;
      }
      return true;
    }

    bool operator!=( const QgsColorRampShader &other ) const
    {
      return !( *this == other );
    }

    //An entry for classification based upon value.
    //Such a classification is typically used for
    //single band layers where a pixel value represents
    //not a color but a quantity, e.g. temperature or elevation
    struct ColorRampItem
    {

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

      bool operator!=( const QgsColorRampShader::ColorRampItem &other ) const
      {
        return ( color != other.color ) ||
               ( !std::isnan( value ) && !std::isnan( other.value ) && value != other.value ) ||
               ( std::isnan( value ) != std::isnan( other.value ) );
      }
    };

    /**
     * Returns the custom color map.
     *
     * \see setColorRampItemList()
     */
    QList<QgsColorRampShader::ColorRampItem> colorRampItemList() const { return mColorRampItemList.toList(); }

    /**
     * Returns the color ramp interpolation method.
     *
     * \see setColorRampType()
     */
    Qgis::ShaderInterpolationMethod colorRampType() const { return mColorRampType; }

    //! Returns the color ramp type as a string.
    QString colorRampTypeAsQString() const;

    /**
     * Sets a custom color map.
     *
     * \see colorRampItemList()
     */
    void setColorRampItemList( const QList<QgsColorRampShader::ColorRampItem> &list ); //TODO: sort on set

    /**
     * Sets the color ramp interpolation method.
     *
     * \see colorRampType()
     */
    void setColorRampType( Qgis::ShaderInterpolationMethod colorRampType );

    /**
     * Whether the color ramp contains any items
     * \since QGIS 3.4
     */
    bool isEmpty() const;

    /**
     * Returns the source color ramp.
     * \see setSourceColorRamp()
     */
    QgsColorRamp *sourceColorRamp() const;

    /**
     * Creates a gradient color ramp from shader settings.
     * \since QGIS 3.18
     */
    QgsColorRamp *createColorRamp() const SIP_FACTORY;

    /**
     * Set the source color ramp. Ownership is transferred to the shader.
     * \see sourceColorRamp()
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

    bool shade( double value, int *returnRedValue SIP_OUT, int *returnGreenValue SIP_OUT, int *returnBlueValue SIP_OUT, int *returnAlphaValue SIP_OUT ) const override;
    bool shade( double redValue, double greenValue,
                double blueValue, double alphaValue,
                int *returnRedValue SIP_OUT, int *returnGreenValue SIP_OUT,
                int *returnBlueValue SIP_OUT, int *returnAlphaValue SIP_OUT ) const override;
    void legendSymbologyItems( QList< QPair< QString, QColor > > &symbolItems SIP_OUT ) const override;

    /**
     * Writes configuration to a new DOM element
     * \since QGIS 3.4
     */
    QDomElement writeXml( QDomDocument &doc, const QgsReadWriteContext &context = QgsReadWriteContext() ) const;

    /**
     * Reads configuration from the given DOM element
     * \since QGIS 3.4
     */
    void readXml( const QDomElement &elem, const QgsReadWriteContext &context = QgsReadWriteContext() );

    /**
     * Sets the classification mode.
     *
     * \see classificationMode()
     */
    void setClassificationMode( Qgis::ShaderClassificationMethod classificationMode ) { mClassificationMode = classificationMode; }

    /**
     * Returns the classification mode.
     *
     * \see setClassificationMode()
     */
    Qgis::ShaderClassificationMethod classificationMode() const { return mClassificationMode; }

    /**
     * Sets whether the shader should not render values out of range.
     * \param clip set to TRUE to clip values which are out of range.
     * \see clip()
     */
    void setClip( bool clip ) { mClip = clip; }

    /**
     * Returns whether the shader will clip values which are out of range.
     * \see setClip()
     */
    bool clip() const { return mClip; }

    /**
     * Returns the color ramp shader legend settings.
     *
     * \see setLegendSettings()
     * \since QGIS 3.18
     */
    const QgsColorRampLegendNodeSettings *legendSettings() const;

    /**
     * Sets the color ramp shader legend \a settings.
     *
     * Ownership of \a settings is transferred.
     *
     * \see legendSettings()
     * \since QGIS 3.18
     */
    void setLegendSettings( QgsColorRampLegendNodeSettings *settings SIP_TRANSFER );

  protected:

    //! Source color ramp
    std::unique_ptr<QgsColorRamp> mSourceColorRamp;

  private:

    /**
     * This vector holds the information for classification based on values.
     * Each item holds a value, a label and a color. The member
     * mDiscreteClassification holds if one color is applied for all values
     * between two class breaks (TRUE) or if the item values are (linearly)
     * interpolated for values between the item values (FALSE)
    */
    QVector<QgsColorRampShader::ColorRampItem> mColorRampItemList;

    Qgis::ShaderInterpolationMethod mColorRampType = Qgis::ShaderInterpolationMethod::Linear;
    Qgis::ShaderClassificationMethod mClassificationMode = Qgis::ShaderClassificationMethod::Continuous;

    /**
     * Look up table to speed up finding the right color.
     * It is initialized on the first call to shade().
    */
    mutable std::vector<int> mLUT;
    mutable double mLUTOffset = 0.0;
    mutable double mLUTFactor = 1.0;
    mutable bool mLUTInitialized = false;

    //! Do not render values out of range
    bool mClip = false;

    std::unique_ptr< QgsColorRampLegendNodeSettings > mLegendSettings;

};

#endif
