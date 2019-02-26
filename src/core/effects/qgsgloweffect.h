/***************************************************************************
                             qgsgloweffect.h
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
#ifndef QGSGLOWEFFECT_H
#define QGSGLOWEFFECT_H

#include "qgis_core.h"
#include "qgis.h"
#include "qgspainteffect.h"
#include "qgssymbol.h"
#include "qgscolorramp.h"

#include <QPainter>


/**
 * \ingroup core
 * \class QgsGlowEffect
 * \brief Base class for paint effect which draw a glow inside or outside a
 * picture.
 *
 * \since QGIS 2.9
 */

class CORE_EXPORT QgsGlowEffect : public QgsPaintEffect
{

  public:

    //! Color sources for the glow
    enum GlowColorType
    {
      SingleColor, //!< Use a single color and fade the color to totally transparent
      ColorRamp //!< Use colors from a color ramp
    };

    QgsGlowEffect();
    QgsGlowEffect( const QgsGlowEffect &other );
    ~QgsGlowEffect() override;

    QgsStringMap properties() const override;
    void readProperties( const QgsStringMap &props ) override;

    /**
     * Sets the spread distance for drawing the glow effect.
     * \param spread spread distance. Units are specified via setSpreadUnit()
     * \see spread
     * \see setSpreadUnit
     * \see setSpreadMapUnitScale
     */
    void setSpread( const double spread ) { mSpread = spread; }

    /**
     * Returns the spread distance used for drawing the glow effect.
     * \returns spread distance. Units are retrieved via spreadUnit()
     * \see setSpread
     * \see spreadUnit
     * \see spreadMapUnitScale
     */
    double spread() const { return mSpread; }

    /**
     * Sets the units used for the glow spread distance.
     * \param unit units for spread distance
     * \see spreadUnit
     * \see setSpread
     * \see setSpreadMapUnitScale
     */
    void setSpreadUnit( const QgsUnitTypes::RenderUnit unit ) { mSpreadUnit = unit; }

    /**
     * Returns the units used for the glow spread distance.
     * \returns units for spread distance
     * \see setSpreadUnit
     * \see spread
     * \see spreadMapUnitScale
     */
    QgsUnitTypes::RenderUnit spreadUnit() const { return mSpreadUnit; }

    /**
     * Sets the map unit scale used for the spread distance.
     * \param scale map unit scale for spread distance
     * \see spreadMapUnitScale
     * \see setSpread
     * \see setSpreadUnit
     */
    void setSpreadMapUnitScale( const QgsMapUnitScale &scale ) { mSpreadMapUnitScale = scale; }

    /**
     * Returns the map unit scale used for the spread distance.
     * \returns map unit scale for spread distance
     * \see setSpreadMapUnitScale
     * \see spread
     * \see spreadUnit
     */
    const QgsMapUnitScale &spreadMapUnitScale() const { return mSpreadMapUnitScale; }

    /**
     * Sets blur level (strength) for the glow. This can be used to smooth the
     * output from the glow effect.
     * \param level blur level. Values between 0 and 16 are valid, with larger
     * values indicating greater blur strength.
     * \see blurLevel
     */
    void setBlurLevel( const int level ) { mBlurLevel = level; }

    /**
     * Returns the blur level (strength) for the glow.
     * \returns blur level. Value will be between 0 and 16, with larger
     * values indicating greater blur strength.
     * \see setBlurLevel
     */
    int blurLevel() const { return mBlurLevel; }

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
     * \see setOpacity().
     */
    double opacity() const { return mOpacity; }

    /**
     * Sets the color for the glow. This only applies if the colorType()
     * is set to SingleColor. The glow will fade between the specified color and
     * a totally transparent version of the color.
     * \param color glow color
     * \see color
     * \see setColorType
     */
    void setColor( const QColor &color ) { mColor = color; }

    /**
     * Returns the color for the glow. This only applies if the colorType()
     * is set to SingleColor. The glow will fade between the specified color and
     * a totally transparent version of the color.
     * \returns glow color
     * \see setColor
     * \see colorType
     */
    QColor color() const { return mColor; }

    /**
     * Sets the color ramp for the glow. This only applies if the colorType()
     * is set to ColorRamp. The glow will utilize colors from the ramp.
     * \param ramp color ramp for glow. Ownership of the ramp is transferred to the effect.
     * \see ramp
     * \see setColorType
     */
    void setRamp( QgsColorRamp *ramp SIP_TRANSFER );

    /**
     * Returns the color ramp used for the glow. This only applies if the colorType()
     * is set to ColorRamp. The glow will utilize colors from the ramp.
     * \returns color ramp for glow
     * \see setRamp
     * \see colorType
     */
    QgsColorRamp *ramp() const { return mRamp; }

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

    /**
     * Sets the color mode to use for the glow. The glow can either be drawn using a QgsColorRamp
     * color ramp or by simply specificing a single color. setColorType is used to specify which mode to use
     * for the glow.
     * \param colorType color type to use for glow
     * \see colorType
     * \see setColor
     * \see setRamp
     */
    void setColorType( GlowColorType colorType ) { mColorType = colorType; }

    /**
     * Returns the color mode used for the glow. The glow can either be drawn using a QgsColorRamp
     * color ramp or by specificing a single color.
     * \returns current color mode used for the glow
     * \see setColorType
     * \see color
     * \see ramp
     */
    GlowColorType colorType() const { return mColorType; }

    QgsGlowEffect &operator=( const QgsGlowEffect &rhs );

  protected:

    QRectF boundingRect( const QRectF &rect, const QgsRenderContext &context ) const override;
    void draw( QgsRenderContext &context ) override;

    /**
     * Specifies whether the glow is drawn outside the picture or within
     * the picture.
     * \returns TRUE if glow is to be drawn outside the picture, or FALSE
     * to draw glow within the picture
     */
    virtual bool shadeExterior() const = 0;

    double mSpread = 2.0;
    QgsUnitTypes::RenderUnit mSpreadUnit = QgsUnitTypes::RenderMillimeters;
    QgsMapUnitScale mSpreadMapUnitScale;
    QgsColorRamp *mRamp = nullptr;
    int mBlurLevel = 3;
    double mOpacity = 0.5;
    QColor mColor;
    QPainter::CompositionMode mBlendMode = QPainter::CompositionMode_SourceOver;
    GlowColorType mColorType = SingleColor;

};


/**
 * \ingroup core
 * \class QgsOuterGlowEffect
 * \brief A paint effect which draws a glow outside of a picture.
 *
 * \since QGIS 2.9
 */

class CORE_EXPORT QgsOuterGlowEffect : public QgsGlowEffect
{

  public:

    /**
     * Creates a new QgsOuterGlowEffect effect from a properties string map.
     * \param map encoded properties string map
     * \returns new QgsOuterGlowEffect
     */
    static QgsPaintEffect *create( const QgsStringMap &map ) SIP_FACTORY;

    QgsOuterGlowEffect();

    QString type() const override { return QStringLiteral( "outerGlow" ); }
    QgsOuterGlowEffect *clone() const override SIP_FACTORY;

  protected:

    bool shadeExterior() const override { return true; }

};


/**
 * \ingroup core
 * \class QgsInnerGlowEffect
 * \brief A paint effect which draws a glow within a picture.
 *
 * \since QGIS 2.9
 */

class CORE_EXPORT QgsInnerGlowEffect : public QgsGlowEffect
{

  public:

    /**
     * Creates a new QgsInnerGlowEffect effect from a properties string map.
     * \param map encoded properties string map
     * \returns new QgsInnerGlowEffect
     */
    static QgsPaintEffect *create( const QgsStringMap &map ) SIP_FACTORY;

    QgsInnerGlowEffect();

    QString type() const override { return QStringLiteral( "innerGlow" ); }
    QgsInnerGlowEffect *clone() const override SIP_FACTORY;

  protected:

    bool shadeExterior() const override { return false; }

};

#endif // QGSGLOWEFFECT_H

