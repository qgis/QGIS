/***************************************************************************
                             qgscoloreffect.h
                             ----------------
    begin                : March 2015
    copyright            : (C) 2015 Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSCOLOREFFECT_H
#define QGSCOLOREFFECT_H

#include "qgis_core.h"
#include "qgspainteffect.h"
#include "qgsimageoperation.h"
#include "qgis.h"
#include <QPainter>

/**
 * \ingroup core
 * \class QgsColorEffect
 * \brief A paint effect which alters the colors (e.g., brightness, contrast) in a
 * source picture.
 *
 * \since QGIS 2.9
 */

class CORE_EXPORT QgsColorEffect : public QgsPaintEffect
{

  public:

    /**
     * Creates a new QgsColorEffect effect from a properties string map.
     * \param map encoded properties string map
     * \returns new QgsColorEffect
     */
    static QgsPaintEffect *create( const QgsStringMap &map ) SIP_FACTORY;

    QgsColorEffect();

    QString type() const override { return QStringLiteral( "color" ); }
    QgsStringMap properties() const override;
    void readProperties( const QgsStringMap &props ) override;
    QgsColorEffect *clone() const override SIP_FACTORY;

    /**
     * Sets the brightness modification for the effect.
     * \param brightness Valid values are between -255 and 255, where 0 represents
     * no change, negative values indicate darkening and positive values indicate
     * lightening
     * \see setBrightness
     */
    void setBrightness( int brightness ) { mBrightness = qBound( -255, brightness, 255 ); }

    /**
     * Returns the brightness modification for the effect.
     * \returns brightness value. Values are between -255 and 255, where 0 represents
     * no change, negative values indicate darkening and positive values indicate
     * lightening
     * \see setBrightness
     */
    int brightness() const { return mBrightness; }

    /**
     * Sets the contrast modification for the effect.
     * \param contrast Valid values are between -100 and 100, where 0 represents
     * no change, negative values indicate less contrast and positive values indicate
     * greater contrast
     * \see setContrast
     */
    void setContrast( int contrast ) { mContrast = qBound( -100, contrast, 100 ); }

    /**
     * Returns the contrast modification for the effect.
     * \returns contrast value. Values are between -100 and 100, where 0 represents
     * no change, negative values indicate less contrast and positive values indicate
     * greater contrast
     * \see setContrast
     */
    int contrast() const { return mContrast; }

    /**
     * Sets the saturation modification for the effect.
     * \param saturation Valid values are between 0 and 2.0, where 1.0 represents
     * no change, 0.0 represents totally desaturated (grayscale), and positive values indicate
     * greater saturation
     * \see saturation
     */
    void setSaturation( double saturation ) { mSaturation = saturation; }

    /**
     * Returns the saturation modification for the effect.
     * \returns saturation value. Values are between 0 and 2.0, where 1.0 represents
     * no change, 0.0 represents totally desaturated (grayscale), and positive values indicate
     * greater saturation
     * \see setSaturation
     */
    double saturation() const { return mSaturation; }

    /**
     * Sets whether the effect should convert a picture to grayscale.
     * \param grayscaleMode method for grayscale conversion
     * \see grayscaleMode
     */
    void setGrayscaleMode( QgsImageOperation::GrayscaleMode grayscaleMode ) { mGrayscaleMode = grayscaleMode; }

    /**
     * Returns whether the effect will convert a picture to grayscale.
     * \returns method for grayscale conversion
     * \see setGrayscaleMode
     */
    QgsImageOperation::GrayscaleMode grayscaleMode() const { return mGrayscaleMode; }

    /**
     * Sets whether the effect should colorize a picture.
     * \param colorizeOn set to true to enable colorization
     * \see colorizeOn
     * \see setColorizeColor
     * \see setColorizeStrength
     */
    void setColorizeOn( bool colorizeOn ) { mColorizeOn = colorizeOn; }

    /**
     * Returns whether the effect will colorize a picture.
     * \returns true if colorization is enableds
     * \see setColorizeOn
     * \see colorizeColor
     * \see colorizeStrength
     */
    bool colorizeOn() const { return mColorizeOn; }

    /**
     * Sets the color used for colorizing a picture. This is only used if
     *  setColorizeOn() is set to true.
     * \param colorizeColor colorization color
     * \see colorizeColor
     * \see setColorizeOn
     * \see setColorizeStrength
     */
    void setColorizeColor( const QColor &colorizeColor );

    /**
     * Returns the color used for colorizing a picture. This is only used if
     * colorizeOn() is set to true.
     * \returns colorization color
     * \see setColorizeColor
     * \see colorizeOn
     * \see colorizeStrength
     */
    QColor colorizeColor() const { return mColorizeColor; }

    /**
     * Sets the strength for colorizing a picture. This is only used if
     *  setColorizeOn() is set to true.
     * \param colorizeStrength colorization strength, between 0 and 100
     * \see colorizeStrength
     * \see setColorizeOn
     * \see setColorizeColor
     */
    void setColorizeStrength( int colorizeStrength ) { mColorizeStrength = colorizeStrength; }

    /**
     * Returns the strength used for colorizing a picture. This is only used if
     * setColorizeOn() is set to true.
     * \returns colorization strength, between 0 and 100
     * \see setColorizeStrength
     * \see colorizeOn
     * \see colorizeColor
     */
    int colorizeStrength() const { return mColorizeStrength; }

    /**
     * Sets the \a opacity for the effect.
     * \param opacity double between 0 and 1 inclusive, where 0 is fully transparent
     * and 1 is fully opaque
     * \see opacity()
     */
    void setOpacity( const double opacity ) { mOpacity = opacity; }

    /**
     * Returns the opacity for the effect.
     * \returns opacity value between 0 and 1 inclusive, where 0 is fully transparent
     * and 1 is fully opaque.
     * \see setOpacity()
     */
    double opacity() const { return mOpacity; }

    /**
     * Sets the blend mode for the effect
     * \param mode blend mode used for drawing the effect on to a destination
     * paint device
     * \see blendMode
     */
    void setBlendMode( const QPainter::CompositionMode mode ) { mBlendMode = mode; }

    /**
     * Returns the blend mode for the effect
     * \returns blend mode used for drawing the effect on to a destination
     * paint device
     * \see setBlendMode
     */
    QPainter::CompositionMode blendMode() const { return mBlendMode; }

  protected:

    void draw( QgsRenderContext &context ) override;

  private:

    double mOpacity = 1.0;
    QPainter::CompositionMode mBlendMode = QPainter::CompositionMode_SourceOver;
    int mBrightness = 0;
    int mContrast = 0;
    double mSaturation = 1.0;
    QgsImageOperation::GrayscaleMode mGrayscaleMode = QgsImageOperation::GrayscaleOff;
    bool mColorizeOn = false;
    QColor mColorizeColor;
    int mColorizeStrength = 100;
};

#endif // QGSBLUREFFECT_H

