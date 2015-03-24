/***************************************************************************
                             qgsshadoweffect.h
                             -----------------
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
#ifndef QGSSHADOWEFFECT_H
#define QGSSHADOWEFFECT_H

#include "qgspainteffect.h"
#include "qgis.h"
#include "qgssymbolv2.h"
#include <QPainter>

/** \ingroup core
 * \class QgsShadowEffect
 * \brief Base class for paint effects which offset, blurred shadows
 *
 * \note Added in version 2.9
 */

class CORE_EXPORT QgsShadowEffect : public QgsPaintEffect
{

  public:

    QgsShadowEffect();
    virtual ~QgsShadowEffect();

    virtual QgsStringMap properties() const override;
    virtual void readProperties( const QgsStringMap& props ) override;

    /** Sets blur level (strength) for the shadow.
     * @param level blur level. Values between 0 and 16 are valid, with larger
     * values indicating greater blur strength.
     * @see blurLevel
     */
    void setBlurLevel( const int level ) { mBlurLevel = level; }

    /** Returns the blur level (strength) for the shadow.
     * @returns blur level. Value will be between 0 and 16, with larger
     * values indicating greater blur strength.
     * @see setBlurLevel
     */
    int blurLevel() const { return mBlurLevel; }

    /** Sets the angle for offsetting the shadow.
     * @param angle offset angle in degrees clockwise from North
     * @see offsetAngle
     * @see setOffsetDistance
     */
    void setOffsetAngle( const int angle ) { mOffsetAngle = angle; }

    /** Returns the angle used for offsetting the shadow.
     * @returns offset angle in degrees clockwise from North
     * @see setOffsetAngle
     * @see offsetDistance
     */
    int offsetAngle() const { return mOffsetAngle; }

    /** Sets the distance for offsetting the shadow.
     * @param distance offset distance. Units are specified via @link setOffsetUnit @endlink
     * @see offsetDistance
     * @see setOffsetUnit
     * @see setOffsetMapUnitScale
     */
    void setOffsetDistance( const double distance ) { mOffsetDist = distance; }

    /** Returns the distance used for offsetting the shadow.
     * @returns offset distance. Distance units are retreived via @link offsetUnit @endlink
     * @see setOffsetDistance
     * @see offsetUnit
     * @see offsetMapUnitScale
     */
    double offsetDistance() const { return mOffsetDist; }

    /** Sets the units used for the shadow offset distance.
     * @param unit units for offset distance
     * @see offsetUnit
     * @see setOffsetDistance
     * @see setOffsetMapUnitScale
     */
    void setOffsetUnit( const QgsSymbolV2::OutputUnit unit ) { mOffsetUnit = unit; }

    /** Returns the units used for the shadow offset distance.
     * @returns units for offset distance
     * @see setOffsetUnit
     * @see offsetDistance
     * @see offsetMapUnitScale
     */
    QgsSymbolV2::OutputUnit offsetUnit() const { return mOffsetUnit; }

    /** Sets the map unit scale used for the shadow offset distance.
     * @param scale map unit scale for offset distance
     * @see offsetMapUnitScale
     * @see setOffsetDistance
     * @see setOffsetUnit
     */
    void setOffsetMapUnitScale( const QgsMapUnitScale& scale ) { mOffsetMapUnitScale = scale; }

    /** Returns the map unit scale used for the shadow offset distance.
     * @returns map unit scale for offset distance
     * @see setOffsetMapUnitScale
     * @see offsetDistance
     * @see offsetUnit
     */
    const QgsMapUnitScale& offsetMapUnitScale() const { return mOffsetMapUnitScale; }

    /** Sets the color for the shadow.
     * @param color shadow color
     * @see color
     */
    void setColor( const QColor& color ) { mColor = color; }

    /** Returns the color used for the shadow.
     * @returns shadow color
     * @see setColor
     */
    QColor color() const { return mColor; }

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

  protected:

    virtual QRectF boundingRect( const QRectF& rect, const QgsRenderContext& context ) const override;
    virtual void draw( QgsRenderContext& context ) override;

    /** Specifies whether the shadow is drawn outside the picture or within
     * the picture.
     * @returns true if shadow is to be drawn outside the picture, or false
     * to draw shadow within the picture
     */
    virtual bool exteriorShadow() const = 0;

    int mBlurLevel;
    int mOffsetAngle;
    double mOffsetDist;
    QgsSymbolV2::OutputUnit mOffsetUnit;
    QgsMapUnitScale mOffsetMapUnitScale;
    double mTransparency;
    QColor mColor;
    QPainter::CompositionMode mBlendMode;

};


/** \ingroup core
 * \class QgsDropShadowEffect
 * \brief A paint effect which draws an offset and optionally blurred drop shadow
 *
 * \note Added in version 2.9
 */
class CORE_EXPORT QgsDropShadowEffect : public QgsShadowEffect
{

  public:

    /** Creates a new QgsDropShadowEffect effect from a properties string map.
     * @param map encoded properties string map
     * @returns new QgsDropShadowEffect
     */
    static QgsPaintEffect* create( const QgsStringMap& map );

    QgsDropShadowEffect();
    virtual ~QgsDropShadowEffect();

    virtual QString type() const override { return QString( "dropShadow" ); }
    virtual QgsPaintEffect* clone() const override;

  protected:

    virtual bool exteriorShadow() const override { return true; }

};

/** \ingroup core
 * \class QgsInnerShadowEffect
 * \brief A paint effect which draws an offset and optionally blurred drop shadow
 * within a picture.
 *
 * \note Added in version 2.9
 */
class CORE_EXPORT QgsInnerShadowEffect : public QgsShadowEffect
{

  public:

    /** Creates a new QgsInnerShadowEffect effect from a properties string map.
     * @param map encoded properties string map
     * @returns new QgsInnerShadowEffect
     */
    static QgsPaintEffect* create( const QgsStringMap& map );

    QgsInnerShadowEffect();
    virtual ~QgsInnerShadowEffect();

    virtual QString type() const override { return QString( "innerShadow" ); }
    virtual QgsPaintEffect* clone() const override;

  protected:

    virtual bool exteriorShadow() const override { return false; }

};

#endif // QGSSHADOWEFFECT_H

