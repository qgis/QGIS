/***************************************************************************
  qgstextshadowsettings.h
  -----------------
   begin                : May 2020
   copyright            : (C) Nyall Dawson
   email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSTEXTSHADOWSETTINGS_H
#define QGSTEXTSHADOWSETTINGS_H

#include "qgsmapunitscale.h"

#include <QSharedDataPointer>
#include <QPainter>
#include <QDomElement>

class QgsTextShadowSettingsPrivate;
class QgsVectorLayer;
class QgsPropertyCollection;

/**
 * \class QgsTextShadowSettings
  * \ingroup core
  * \brief Container for settings relating to a text shadow.
  * \note QgsTextShadowSettings objects are implicitly shared.
 */
class CORE_EXPORT QgsTextShadowSettings
{
  public:

    /**
     * Placement positions for text shadow.
     */
    enum ShadowPlacement
    {
      ShadowLowest = 0, //!< Draw shadow below all text components
      ShadowText, //!< Draw shadow under text
      ShadowBuffer, //!< Draw shadow under buffer
      ShadowShape //!< Draw shadow under background shape
    };

    QgsTextShadowSettings();
    QgsTextShadowSettings( const QgsTextShadowSettings &other );

    QgsTextShadowSettings &operator=( const QgsTextShadowSettings &other );

    ~QgsTextShadowSettings();

    bool operator==( const QgsTextShadowSettings &other ) const;
    bool operator!=( const QgsTextShadowSettings &other ) const;

    /**
     * Returns whether the shadow is enabled.
     * \see setEnabled()
     */
    bool enabled() const;

    /**
     * Sets whether the text shadow will be drawn.
     * \param enabled set to TRUE to draw shadow
     * \see enabled()
     */
    void setEnabled( bool enabled );

    /**
     * Returns the placement for the drop shadow. The placement determines
     * both the z-order stacking position for the shadow and the what shape (e.g., text,
     * background shape) is used for casting the shadow.
     * \see setShadowPlacement()
     */
    QgsTextShadowSettings::ShadowPlacement shadowPlacement() const;

    /**
     * Sets the placement for the drop shadow. The placement determines
     * both the z-order stacking position for the shadow and the what shape (e.g., text,
     * background shape) is used for casting the shadow.
     * \param placement shadow placement
     * \see shadowPlacement()
     */
    void setShadowPlacement( QgsTextShadowSettings::ShadowPlacement placement );

    /**
     * Returns the angle for offsetting the position of the shadow from the text.
     * \see setOffsetAngle
     * \see offsetDistance()
     */
    int offsetAngle() const;

    /**
     * Sets the angle for offsetting the position of the shadow from the text.
     * \param angle offset angle in degrees
     * \see offsetAngle()
     * \see setOffsetDistance()
     */
    void setOffsetAngle( int angle );

    /**
     * Returns the distance for offsetting the position of the shadow from the text. Offset units
     * are retrieved via offsetUnit().
     * \see setOffsetDistance()
     * \see offsetUnit()
     */
    double offsetDistance() const;

    /**
     * Sets the distance for offsetting the position of the shadow from the text. Offset units
     * are specified via setOffsetUnit().
     * \param distance offset distance
     * \see offsetDistance()
     * \see setOffsetUnit()
     */
    void setOffsetDistance( double distance );

    /**
     * Returns the units used for the shadow's offset.
     * \see setOffsetUnit()
     * \see offsetDistance()
     */
    Qgis::RenderUnit offsetUnit() const;

    /**
     * Sets the units used for the shadow's offset.
     * \param units shadow distance units
     * \see offsetUnit()
     * \see setOffsetDistance()
     */
    void setOffsetUnit( Qgis::RenderUnit units );

    /**
     * Returns the map unit scale object for the shadow offset distance. This is only used if the
     * offsetUnit() is set to QgsUnitTypes::RenderMapUnit.
     * \see setOffsetMapUnitScale()
     * \see offsetUnit()
     */
    QgsMapUnitScale offsetMapUnitScale() const;

    /**
     * Sets the map unit scale object for the shadow offset distance. This is only used if the
     * offsetUnit() is set to QgsUnitTypes::RenderMapUnit.
     * \param scale scale for shadow offset
     * \see offsetMapUnitScale()
     * \see setOffsetUnit()
     */
    void setOffsetMapUnitScale( const QgsMapUnitScale &scale );

    /**
     * Returns TRUE if the global shadow offset will be used.
     * \see setOffsetGlobal()
     */
    bool offsetGlobal() const;

    /**
     * Sets whether the global shadow offset should be used.
     * \param global set to TRUE to use global shadow offset.
     */
    void setOffsetGlobal( bool global );

    /**
     * Returns the blur radius for the shadow. Radius units are retrieved via blurRadiusUnits().
     * \see setBlurRadius()
     * \see blurRadiusUnit()
     */
    double blurRadius() const;

    /**
     * Sets the blur radius for the shadow. Radius units are specified via setBlurRadiusUnits().
     * \param blurRadius blur radius
     * \see blurRadius()
     * \see setBlurRadiusUnit()
     */
    void setBlurRadius( double blurRadius );

    /**
     * Returns the units used for the shadow's blur radius.
     * \see setBlurRadiusUnit()
     * \see blurRadius()
     */
    Qgis::RenderUnit blurRadiusUnit() const;

    /**
     * Sets the units used for the shadow's blur radius.
     * \param units shadow blur radius units
     * \see blurRadiusUnit()
     * \see setBlurRadius()
     */
    void setBlurRadiusUnit( Qgis::RenderUnit units );

    /**
     * Returns the map unit scale object for the shadow blur radius. This is only used if the
     * blurRadiusUnit() is set to QgsUnitTypes::RenderMapUnit.
     * \see setBlurRadiusMapUnitScale()
     * \see blurRadiusUnit()
     */
    QgsMapUnitScale blurRadiusMapUnitScale() const;

    /**
     * Sets the map unit scale object for the shadow blur radius. This is only used if the
     * blurRadiusUnit() is set to QgsUnitTypes::RenderMapUnit.
     * \param scale scale for shadow blur radius
     * \see blurRadiusMapUnitScale()
     * \see setBlurRadiusUnit()
     */
    void setBlurRadiusMapUnitScale( const QgsMapUnitScale &scale );

    /**
     * Returns whether only the alpha channel for the shadow will be blurred.
     * \see setBlurAlphaOnly()
     */
    bool blurAlphaOnly() const;

    /**
     * Sets whether only the alpha channel for the shadow should be blurred.
     * \param alphaOnly set to TRUE to blur only the alpha channel. If FALSE, all channels (including
     * red, green and blue channel) will be blurred.
     * \see blurAlphaOnly()
     */
    void setBlurAlphaOnly( bool alphaOnly );

    /**
     * Returns the shadow's opacity. The opacity is a double value between 0 (fully transparent) and 1 (totally
     * opaque).
     * \see setOpacity()
     */
    double opacity() const;

    /**
     * Sets the shadow's opacity.
     * \param opacity opacity as a double value between 0 (fully transparent) and 1 (totally
     * opaque)
     * \see opacity()
     */
    void setOpacity( double opacity );

    /**
     * Returns the scaling used for the drop shadow (in percentage of original size).
     * \see setScale()
     */
    int scale() const;

    /**
     * Sets the scaling used for the drop shadow (in percentage of original size).
     * \param scale scale percent for drop shadow
     * \see scale()
     */
    void setScale( int scale );

    /**
     * Returns the color of the drop shadow.
     * \see setColor()
     */
    QColor color() const;

    /**
     * Sets the color for the drop shadow.
     * \param color shadow color
     * \see color()
     */
    void setColor( const QColor &color );

    /**
     * Returns the blending mode used for drawing the drop shadow.
     * \see setBlendMode()
     */
    QPainter::CompositionMode blendMode() const;

    /**
     * Sets the blending mode used for drawing the drop shadow.
     * \param mode blending mode
     * \see blendMode()
     */
    void setBlendMode( QPainter::CompositionMode mode );

    /**
     * Reads settings from a layer's custom properties (for QGIS 2.x projects).
     * \param layer source vector layer
     */
    void readFromLayer( QgsVectorLayer *layer );

    /**
     * Read settings from a DOM element.
     * \see writeXml()
     */
    void readXml( const QDomElement &elem );

    /**
     * Write settings into a DOM element.
     * \see readXml()
     */
    QDomElement writeXml( QDomDocument &doc ) const;

    /**
     * Updates the format by evaluating current values of data defined properties.
     * \since QGIS 3.10
     */
    void updateDataDefinedProperties( QgsRenderContext &context, const QgsPropertyCollection &properties );

    /**
     * Returns all field names referenced by the configuration (e.g. from data defined properties).
     * \since QGIS 3.14
     */
    QSet<QString> referencedFields( const QgsRenderContext &context ) const;

  private:

    QSharedDataPointer<QgsTextShadowSettingsPrivate> d;

};

#endif // QGSTEXTSHADOWSETTINGS_H
