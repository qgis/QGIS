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
#include "qgis.h"
#include <QPainter>

/**
 * \ingroup core
 * \class QgsBlurEffect
 * \brief A paint effect which blurs a source picture, using a number of different blur
 * methods.
 *
 * \since QGIS 2.9
 */

class CORE_EXPORT QgsBlurEffect : public QgsPaintEffect
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
    static QgsPaintEffect *create( const QgsStringMap &map ) SIP_FACTORY;

    /**
     * Constructor for QgsBlurEffect.
     */
    QgsBlurEffect() = default;

    QString type() const override { return QStringLiteral( "blur" ); }
    QgsStringMap properties() const override;
    void readProperties( const QgsStringMap &props ) override;
    QgsBlurEffect *clone() const override SIP_FACTORY;

    /**
     * Sets blur level (strength)
     * \param level blur level. Depending on the current blurMethod(), this parameter
     * has different effects
     * \see blurLevel
     * \see blurMethod
     */
    void setBlurLevel( const int level ) { mBlurLevel = level; }

    /**
     * Returns the blur level (strength)
     * \returns blur level. Depending on the current blurMethod(), this parameter
     * has different effects
     * \see setBlurLevel
     * \see blurMethod
     */
    int blurLevel() const { return mBlurLevel; }

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

    int mBlurLevel = 10;
    BlurMethod mBlurMethod = StackBlur;
    double mOpacity = 1.0;
    QPainter::CompositionMode mBlendMode = QPainter::CompositionMode_SourceOver;

    void drawStackBlur( QgsRenderContext &context );
    void drawGaussianBlur( QgsRenderContext &context );
    void drawBlurredImage( QgsRenderContext &context, QImage &image );
};

#endif // QGSBLUREFFECT_H

