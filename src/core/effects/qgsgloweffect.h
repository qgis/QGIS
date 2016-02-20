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

#include "qgspainteffect.h"
#include "qgssymbolv2.h"
#include <QPainter>

class QgsVectorColorRampV2;

/** \ingroup core
 * \class QgsGlowEffect
 * \brief Base class for paint effect which draw a glow inside or outside a
 * picture.
 *
 * \note Added in version 2.9
 */

class CORE_EXPORT QgsGlowEffect : public QgsPaintEffect
{

  public:

    /** Color sources for the glow */
    enum GlowColorType
    {
      SingleColor, /*!< use a single color and fade the color to totally transparent */
      ColorRamp /*!< use colors from a color ramp */
    };

    QgsGlowEffect();
    QgsGlowEffect( const QgsGlowEffect& other );
    virtual ~QgsGlowEffect();

    virtual QgsStringMap properties() const override;
    virtual void readProperties( const QgsStringMap& props ) override;

    /** Sets the spread distance for drawing the glow effect.
     * @param spread spread distance. Units are specified via @link setSpreadUnit @endlink
     * @see spread
     * @see setSpreadUnit
     * @see setSpreadMapUnitScale
     */
    void setSpread( const double spread ) { mSpread = spread; }

    /** Returns the spread distance used for drawing the glow effect.
     * @returns spread distance. Units are retrieved via @link spreadUnit @endlink
     * @see setSpread
     * @see spreadUnit
     * @see spreadMapUnitScale
     */
    double spread() const { return mSpread; }

    /** Sets the units used for the glow spread distance.
     * @param unit units for spread distance
     * @see spreadUnit
     * @see setSpread
     * @see setSpreadMapUnitScale
     */
    void setSpreadUnit( const QgsSymbolV2::OutputUnit unit ) { mSpreadUnit = unit; }

    /** Returns the units used for the glow spread distance.
     * @returns units for spread distance
     * @see setSpreadUnit
     * @see spread
     * @see spreadMapUnitScale
     */
    QgsSymbolV2::OutputUnit spreadUnit() const { return mSpreadUnit; }

    /** Sets the map unit scale used for the spread distance.
     * @param scale map unit scale for spread distance
     * @see spreadMapUnitScale
     * @see setSpread
     * @see setSpreadUnit
     */
    void setSpreadMapUnitScale( const QgsMapUnitScale& scale ) { mSpreadMapUnitScale = scale; }

    /** Returns the map unit scale used for the spread distance.
     * @returns map unit scale for spread distance
     * @see setSpreadMapUnitScale
     * @see spread
     * @see spreadUnit
     */
    const QgsMapUnitScale& spreadMapUnitScale() const { return mSpreadMapUnitScale; }

    /** Sets blur level (strength) for the glow. This can be used to smooth the
     * output from the glow effect.
     * @param level blur level. Values between 0 and 16 are valid, with larger
     * values indicating greater blur strength.
     * @see blurLevel
     */
    void setBlurLevel( const int level ) { mBlurLevel = level; }

    /** Returns the blur level (strength) for the glow.
     * @returns blur level. Value will be between 0 and 16, with larger
     * values indicating greater blur strength.
     * @see setBlurLevel
     */
    int blurLevel() const { return mBlurLevel; }

    /** Sets the transparency for the effect
     * @param transparency double between 0 and 1 inclusive, where 0 is fully opaque
     * and 1 is fully transparent
     * @see transparency
     */
    void setTransparency( const double transparency ) { mTransparency = transparency; }

    /** Returns the transparency for the effect
     * @returns transparency value between 0 and 1 inclusive, where 0 is fully opaque
     * and 1 is fully transparent
     * @see setTransparency
     */
    double transparency() const { return mTransparency; }

    /** Sets the color for the glow. This only applies if the @link colorType @endlink
     * is set to SingleColor. The glow will fade between the specified color and
     * a totally transparent version of the color.
     * @param color glow color
     * @see color
     * @see setColorType
     */
    void setColor( const QColor& color ) { mColor = color; }

    /** Returns the color for the glow. This only applies if the @link colorType @endlink
     * is set to SingleColor. The glow will fade between the specified color and
     * a totally transparent version of the color.
     * @returns glow color
     * @see setColor
     * @see colorType
     */
    QColor color() const { return mColor; }

    /** Sets the color ramp for the glow. This only applies if the @link colorType @endlink
     * is set to ColorRamp. The glow will utilise colors from the ramp.
     * @param ramp color ramp for glow. Ownership of the ramp is transferred to the effect.
     * @see ramp
     * @see setColorType
     */
    void setRamp( QgsVectorColorRampV2* ramp );

    /** Returns the color ramp used for the glow. This only applies if the @link colorType @endlink
     * is set to ColorRamp. The glow will utilise colors from the ramp.
     * @returns color ramp for glow
     * @see setRamp
     * @see colorType
     */
    QgsVectorColorRampV2* ramp() const { return mRamp; }

    /** Sets the blend mode for the effect
     * @param mode blend mode used for drawing the effect on to a destination
     * paint device
     * @see blendMode
     */
    void setBlendMode( const QPainter::CompositionMode mode ) { mBlendMode = mode; }

    /** Returns the blend mode for the effect
     * @returns blend mode used for drawing the effect on to a destination
     * paint device
     * @see setBlendMode
     */
    QPainter::CompositionMode blendMode() const { return mBlendMode; }

    /** Sets the color mode to use for the glow. The glow can either be drawn using a QgsVectorColorRampV2
     * color ramp or by simply specificing a single color. setColorType is used to specify which mode to use
     * for the glow.
     * @param colorType color type to use for glow
     * @see colorType
     * @see setColor
     * @see setRamp
     */
    void setColorType( GlowColorType colorType ) { mColorType = colorType; }

    /** Returns the color mode used for the glow. The glow can either be drawn using a QgsVectorColorRampV2
     * color ramp or by specificing a single color.
     * @returns current color mode used for the glow
     * @see setColorType
     * @see color
     * @see ramp
     */
    GlowColorType colorType() const { return mColorType; }

    QgsGlowEffect& operator=( const QgsGlowEffect& rhs );

  protected:

    virtual QRectF boundingRect( const QRectF& rect, const QgsRenderContext& context ) const override;
    virtual void draw( QgsRenderContext& context ) override;

    /** Specifies whether the glow is drawn outside the picture or within
     * the picture.
     * @returns true if glow is to be drawn outside the picture, or false
     * to draw glow within the picture
     */
    virtual bool shadeExterior() const = 0;

    double mSpread;
    QgsSymbolV2::OutputUnit mSpreadUnit;
    QgsMapUnitScale mSpreadMapUnitScale;
    QgsVectorColorRampV2* mRamp;
    int mBlurLevel;
    double mTransparency;
    QColor mColor;
    QPainter::CompositionMode mBlendMode;
    GlowColorType mColorType;

};


/** \ingroup core
 * \class QgsOuterGlowEffect
 * \brief A paint effect which draws a glow outside of a picture.
 *
 * \note Added in version 2.9
 */

class CORE_EXPORT QgsOuterGlowEffect : public QgsGlowEffect
{

  public:

    /** Creates a new QgsOuterGlowEffect effect from a properties string map.
     * @param map encoded properties string map
     * @returns new QgsOuterGlowEffect
     */
    static QgsPaintEffect* create( const QgsStringMap& map );

    QgsOuterGlowEffect();
    virtual ~QgsOuterGlowEffect();

    virtual QString type() const override { return QString( "outerGlow" ); }
    virtual QgsOuterGlowEffect* clone() const override;

  protected:

    virtual bool shadeExterior() const override { return true; }

};


/** \ingroup core
 * \class QgsInnerGlowEffect
 * \brief A paint effect which draws a glow within a picture.
 *
 * \note Added in version 2.9
 */

class CORE_EXPORT QgsInnerGlowEffect : public QgsGlowEffect
{

  public:

    /** Creates a new QgsInnerGlowEffect effect from a properties string map.
     * @param map encoded properties string map
     * @returns new QgsInnerGlowEffect
     */
    static QgsPaintEffect* create( const QgsStringMap& map );

    QgsInnerGlowEffect();
    virtual ~QgsInnerGlowEffect();

    virtual QString type() const override { return QString( "innerGlow" ); }
    virtual QgsInnerGlowEffect* clone() const override;

  protected:

    virtual bool shadeExterior() const override { return false; }

};

#endif // QGSGLOWEFFECT_H

