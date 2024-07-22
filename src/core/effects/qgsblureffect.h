/***************************************************************************
                             qgsblureffect.h
                             ---------------
    begin                : December 2014
    copyright            : (C) 2014 Nyall Dawson
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
#ifndef QGSBLUREFFECT_H
#define QGSBLUREFFECT_H

#include "qgis_core.h"
#include "qgspainteffect.h"
#include "qgis_sip.h"
#include "qgis.h"
#include "qgsmapunitscale.h"

#include <QPainter>

/**
 * \ingroup core
 * \class QgsBlurEffect
 * \brief A paint effect which blurs a source picture, using a number of different blur
 * methods.
 *
 */

class CORE_EXPORT QgsBlurEffect : public QgsPaintEffect SIP_NODEFAULTCTORS
{

  public:

    //! Available blur methods (algorithms)
    enum BlurMethod
    {
      StackBlur, //!< Stack blur, a fast but low quality blur. Valid blur level values are between 0 - 16.
      GaussianBlur //!< Gaussian blur, a slower but high quality blur. Blur level values are the distance in pixels for the blur operation.
    };

    /**
     * Creates a new QgsBlurEffect effect from a properties string map.
     * \param map encoded properties string map
     * \returns new QgsBlurEffect
     */
    static QgsPaintEffect *create( const QVariantMap &map ) SIP_FACTORY;

    QgsBlurEffect() = default;

    QString type() const override { return QStringLiteral( "blur" ); }
    QVariantMap properties() const override;
    void readProperties( const QVariantMap &props ) override;
    QgsBlurEffect *clone() const override SIP_FACTORY;

    /**
     * Sets blur level (radius)
     * \param level blur level. Depending on the current blurMethod(), this parameter
     * has different effects
     * \see blurLevel
     * \see setBlurUnit
     * \see setBlurMapUnitScale
     * \see setBlurMethod
     */
    void setBlurLevel( const double level ) { mBlurLevel = level; }

    /**
     * Returns the blur level (radius)
     * \returns blur level. Depending on the current blurMethod(), this parameter
     * has different effects
     * \see setBlurLevel
     * \see blurUnit
     * \see blurMapUnitScale
     * \see blurMethod
     */
    double blurLevel() const { return mBlurLevel; }

    /**
     * Sets the units used for the blur level (radius).
     * \param unit units for blur level
     * \see blurUnit
     * \see setBlurLevel
     * \see setBlurMapUnitScale
     * \since QGIS 3.4.9
     */
    void setBlurUnit( const Qgis::RenderUnit unit ) { mBlurUnit = unit; }

    /**
     * Returns the units used for the blur level (radius).
     * \returns units for blur level
     * \see setBlurUnit
     * \see blurLevel
     * \see blurMapUnitScale
     * \since QGIS 3.4.9
     */
    Qgis::RenderUnit blurUnit() const { return mBlurUnit; }

    /**
     * Sets the map unit scale used for the blur strength (radius).
     * \param scale map unit scale for blur strength
     * \see blurMapUnitScale
     * \see setBlurLevel
     * \see setBlurUnit
     * \since QGIS 3.4.9
     */
    void setBlurMapUnitScale( const QgsMapUnitScale &scale ) { mBlurMapUnitScale = scale; }

    /**
     * Returns the map unit scale used for the blur strength (radius).
     * \returns map unit scale for blur strength
     * \see setBlurMapUnitScale
     * \see blurLevel
     * \see blurUnit
     * \since QGIS 3.4.9
     */
    const QgsMapUnitScale &blurMapUnitScale() const { return mBlurMapUnitScale; }

    /**
     * Sets the blur method (algorithm) to use for performing the blur.
     * \param method blur method
     * \see blurMethod
     */
    void setBlurMethod( const BlurMethod method ) { mBlurMethod = method; }

    /**
     * Returns the blur method (algorithm) used for performing the blur.
     * \returns blur method
     * \see setBlurMethod
     */
    BlurMethod blurMethod() const { return mBlurMethod; }

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
     * and 1 is fully opaque
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
    QRectF boundingRect( const QRectF &rect, const QgsRenderContext &context ) const override;

  private:

    double mBlurLevel = 2.645;
    Qgis::RenderUnit mBlurUnit = Qgis::RenderUnit::Millimeters;
    QgsMapUnitScale mBlurMapUnitScale;
    BlurMethod mBlurMethod = StackBlur;
    double mOpacity = 1.0;
    QPainter::CompositionMode mBlendMode = QPainter::CompositionMode_SourceOver;

    void drawStackBlur( QgsRenderContext &context );
    void drawGaussianBlur( QgsRenderContext &context );
    void drawBlurredImage( QgsRenderContext &context, QImage &image );
};

#endif // QGSBLUREFFECT_H

