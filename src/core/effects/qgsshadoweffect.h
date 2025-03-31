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

#include "qgis_core.h"
#include "qgspainteffect.h"
#include "qgis_sip.h"
#include "qgsmapunitscale.h"
#include "qgis.h"

#include <QPainter>

/**
 * \ingroup core
 * \class QgsShadowEffect
 * \brief Base class for paint effects which render offset, blurred shadows.
 */
class CORE_EXPORT QgsShadowEffect : public QgsPaintEffect SIP_NODEFAULTCTORS
{

  public:

    QgsShadowEffect();

    QVariantMap properties() const override;
    void readProperties( const QVariantMap &props ) override;

    /**
     * Sets blur level (radius) for the shadow.
     * \param level blur level.
     * values indicating greater blur strength.
     * \see blurLevel
     * \see setBlurUnit
     * \see setBlurMapUnitScale
     */
    void setBlurLevel( const double level ) { mBlurLevel = level; }

    /**
     * Returns the blur level (radius) for the shadow.
     * \returns blur level.
     * values indicating greater blur strength.
     * \see setBlurLevel
     * \see blurUnit
     * \see blurMapUnitScale
     */
    double blurLevel() const { return mBlurLevel; }

    /**
     * Sets the units used for the shadow blur level (radius).
     * \param unit units for blur level
     * \see blurUnit
     * \see setBlurLevel
     * \see setBlurMapUnitScale
     * \since QGIS 3.4.9
     */
    void setBlurUnit( const Qgis::RenderUnit unit ) { mBlurUnit = unit; }

    /**
     * Returns the units used for the shadow blur level (radius).
     * \returns units for blur level
     * \see setBlurUnit
     * \see blurLevel
     * \see blurMapUnitScale
     * \since QGIS 3.4.9
     */
    Qgis::RenderUnit blurUnit() const { return mBlurUnit; }

    /**
     * Sets the map unit scale used for the shadow blur strength (radius).
     * \param scale map unit scale for blur strength
     * \see blurMapUnitScale
     * \see setBlurLevel
     * \see setBlurUnit
     * \since QGIS 3.4.9
     */
    void setBlurMapUnitScale( const QgsMapUnitScale &scale ) { mBlurMapUnitScale = scale; }

    /**
     * Returns the map unit scale used for the shadow blur strength (radius).
     * \returns map unit scale for blur strength
     * \see setBlurMapUnitScale
     * \see blurLevel
     * \see blurUnit
     * \since QGIS 3.4.9
     */
    const QgsMapUnitScale &blurMapUnitScale() const { return mBlurMapUnitScale; }

    /**
     * Sets the angle for offsetting the shadow.
     * \param angle offset angle in degrees clockwise from North
     * \see offsetAngle
     * \see setOffsetDistance
     */
    void setOffsetAngle( const int angle ) { mOffsetAngle = angle; }

    /**
     * Returns the angle used for offsetting the shadow.
     * \returns offset angle in degrees clockwise from North
     * \see setOffsetAngle
     * \see offsetDistance
     */
    int offsetAngle() const { return mOffsetAngle; }

    /**
     * Sets the distance for offsetting the shadow.
     * \param distance offset distance. Units are specified via setOffsetUnit()
     * \see offsetDistance
     * \see setOffsetUnit
     * \see setOffsetMapUnitScale
     */
    void setOffsetDistance( const double distance ) { mOffsetDist = distance; }

    /**
     * Returns the distance used for offsetting the shadow.
     * \returns offset distance. Distance units are retrieved via offsetUnit()
     * \see setOffsetDistance
     * \see offsetUnit
     * \see offsetMapUnitScale
     */
    double offsetDistance() const { return mOffsetDist; }

    /**
     * Sets the units used for the shadow offset distance.
     * \param unit units for offset distance
     * \see offsetUnit
     * \see setOffsetDistance
     * \see setOffsetMapUnitScale
     */
    void setOffsetUnit( const Qgis::RenderUnit unit ) { mOffsetUnit = unit; }

    /**
     * Returns the units used for the shadow offset distance.
     * \returns units for offset distance
     * \see setOffsetUnit
     * \see offsetDistance
     * \see offsetMapUnitScale
     */
    Qgis::RenderUnit offsetUnit() const { return mOffsetUnit; }

    /**
     * Sets the map unit scale used for the shadow offset distance.
     * \param scale map unit scale for offset distance
     * \see offsetMapUnitScale
     * \see setOffsetDistance
     * \see setOffsetUnit
     */
    void setOffsetMapUnitScale( const QgsMapUnitScale &scale ) { mOffsetMapUnitScale = scale; }

    /**
     * Returns the map unit scale used for the shadow offset distance.
     * \returns map unit scale for offset distance
     * \see setOffsetMapUnitScale
     * \see offsetDistance
     * \see offsetUnit
     */
    const QgsMapUnitScale &offsetMapUnitScale() const { return mOffsetMapUnitScale; }

    /**
     * Sets the color for the shadow.
     * \param color shadow color
     * \see color
     */
    void setColor( const QColor &color ) { mColor = color; }

    /**
     * Returns the color used for the shadow.
     * \returns shadow color
     * \see setColor
     */
    QColor color() const { return mColor; }

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

    QRectF boundingRect( const QRectF &rect, const QgsRenderContext &context ) const override;
    void draw( QgsRenderContext &context ) override;

    /**
     * Specifies whether the shadow is drawn outside the picture or within
     * the picture.
     * \returns TRUE if shadow is to be drawn outside the picture, or FALSE
     * to draw shadow within the picture
     */
    virtual bool exteriorShadow() const = 0;

    double mBlurLevel = 2.645;
    Qgis::RenderUnit mBlurUnit = Qgis::RenderUnit::Millimeters;
    QgsMapUnitScale mBlurMapUnitScale;
    int mOffsetAngle = 135;
    double mOffsetDist = 2.0;
    Qgis::RenderUnit mOffsetUnit = Qgis::RenderUnit::Millimeters;
    QgsMapUnitScale mOffsetMapUnitScale;
    double mOpacity = 1.0;
    QColor mColor;
    QPainter::CompositionMode mBlendMode = QPainter::CompositionMode_Multiply;

};


/**
 * \ingroup core
 * \class QgsDropShadowEffect
 * \brief A paint effect which draws an offset and optionally blurred drop shadow.
 *
 */
class CORE_EXPORT QgsDropShadowEffect : public QgsShadowEffect SIP_NODEFAULTCTORS
{

  public:

    /**
     * Creates a new QgsDropShadowEffect effect from a properties string map.
     * \param map encoded properties string map
     * \returns new QgsDropShadowEffect
     */
    static QgsPaintEffect *create( const QVariantMap &map ) SIP_FACTORY;

    QgsDropShadowEffect();

    QString type() const override;
    QgsDropShadowEffect *clone() const override SIP_FACTORY;

  protected:

    bool exteriorShadow() const override;

};

/**
 * \ingroup core
 * \class QgsInnerShadowEffect
 * \brief A paint effect which draws an offset and optionally blurred drop shadow
 * within a picture.
 *
 */
class CORE_EXPORT QgsInnerShadowEffect : public QgsShadowEffect SIP_NODEFAULTCTORS
{

  public:

    /**
     * Creates a new QgsInnerShadowEffect effect from a properties string map.
     * \param map encoded properties string map
     * \returns new QgsInnerShadowEffect
     */
    static QgsPaintEffect *create( const QVariantMap &map ) SIP_FACTORY;

    QgsInnerShadowEffect();

    QString type() const override;
    QgsInnerShadowEffect *clone() const override SIP_FACTORY;

  protected:

    bool exteriorShadow() const override;

};

#endif // QGSSHADOWEFFECT_H

