/***************************************************************************
  qgstextbuffersettings.h
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

#ifndef QGSTEXTBUFFERSETTINGS_H
#define QGSTEXTBUFFERSETTINGS_H

#include "qgis_sip.h"
#include "qgis_core.h"
#include "qgsmapunitscale.h"
#include "qgsunittypes.h"

#include <QSharedData>
#include <QPainter>
#include <QDomElement>

class QgsReadWriteContext;
class QgsTextBufferSettingsPrivate;
class QgsVectorLayer;
class QgsPaintEffect;
class QgsPropertyCollection;

/**
 * \class QgsTextBufferSettings
  * \ingroup core
  * Container for settings relating to a text buffer.
  * \note QgsTextBufferSettings objects are implicitly shared.
  * \since QGIS 3.0
 */
class CORE_EXPORT QgsTextBufferSettings
{
  public:

    QgsTextBufferSettings();

    /**
     * Copy constructor.
     * \param other source settings
     */
    QgsTextBufferSettings( const QgsTextBufferSettings &other );

    /**
     * Copy constructor.
     * \param other source QgsTextBufferSettings
     */
    QgsTextBufferSettings &operator=( const QgsTextBufferSettings &other );

    ~QgsTextBufferSettings();

    bool operator==( const QgsTextBufferSettings &other ) const;
    bool operator!=( const QgsTextBufferSettings &other ) const;

    /**
     * Returns whether the buffer is enabled.
     * \see setEnabled()
     */
    bool enabled() const;

    /**
     * Sets whether the text buffer will be drawn.
     * \param enabled set to TRUE to draw buffer
     * \see enabled()
     */
    void setEnabled( bool enabled );

    /**
     * Returns the size of the buffer.
     * \see sizeUnit()
     * \see setSize()
     */
    double size() const;

    /**
     * Sets the size of the buffer. The size units are specified using setSizeUnit().
     * \param size buffer size
     * \see size()
     * \see setSizeUnit()
     */
    void setSize( double size );

    /**
     * Returns the units for the buffer size.
     * \see size()
     * \see setSizeUnit()
     */
    QgsUnitTypes::RenderUnit sizeUnit() const;

    /**
     * Sets the units used for the buffer size.
     * \param unit size unit
     * \see setSize()
     * \see sizeUnit()
     */
    void setSizeUnit( QgsUnitTypes::RenderUnit unit );

    /**
     * Returns the map unit scale object for the buffer size. This is only used if the
     * buffer size is set to QgsUnitTypes::RenderMapUnit.
     * \see setSizeMapUnitScale()
     * \see sizeUnit()
     */
    QgsMapUnitScale sizeMapUnitScale() const;

    /**
     * Sets the map unit scale object for the buffer size. This is only used if the
     * buffer size is set to QgsUnitTypes::RenderMapUnit.
     * \param scale scale for buffer size
     * \see sizeMapUnitScale()
     * \see setSizeUnit()
     */
    void setSizeMapUnitScale( const QgsMapUnitScale &scale );

    /**
     * Returns the color of the buffer.
     * \see setColor()
     */
    QColor color() const;

    /**
     * Sets the color for the buffer.
     * \param color buffer color
     * \see color()
     */
    void setColor( const QColor &color );

    /**
     * Returns whether the interior of the buffer will be filled in. If FALSE, only the stroke
     * of the text will be drawn as the buffer. The effect of this setting is only visible for
     * semi-transparent text.
     * \see setFillBufferInterior()
     */
    bool fillBufferInterior() const;

    /**
     * Sets whether the interior of the buffer will be filled in.
     * \param fill set to FALSE to drawn only the stroke of the text as the buffer, or TRUE to also
     * shade the area inside the text. The effect of this setting is only visible for semi-transparent text.
     * \see fillBufferInterior()
     */
    void setFillBufferInterior( bool fill );

    /**
     * Returns the buffer opacity. The opacity is a double value between 0 (fully transparent) and 1 (totally
     * opaque).
     * \see setOpacity()
     */
    double opacity() const;

    /**
     * Sets the buffer opacity.
     * \param opacity opacity as a double value between 0 (fully transparent) and 1 (totally
     * opaque)
     * \see opacity()
     */
    void setOpacity( double opacity );

    /**
     * Returns the buffer join style.
     * \see setJoinStyle
     */
    Qt::PenJoinStyle joinStyle() const;

    /**
     * Sets the join style used for drawing the buffer.
     * \param style join style
     * \see joinStyle()
     */
    void setJoinStyle( Qt::PenJoinStyle style );

    /**
     * Returns the blending mode used for drawing the buffer.
     * \see setBlendMode()
     */
    QPainter::CompositionMode blendMode() const;

    /**
     * Sets the blending mode used for drawing the buffer.
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
     * Returns the current paint effect for the buffer.
     * \returns paint effect
     * \see setPaintEffect()
     */
    QgsPaintEffect *paintEffect() const;

    /**
     * Sets the current paint \a effect for the buffer.
     * \param effect paint effect. Ownership is transferred to the buffer settings.
     * \see paintEffect()
     */
    void setPaintEffect( QgsPaintEffect *effect SIP_TRANSFER );

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

    QSharedDataPointer<QgsTextBufferSettingsPrivate> d;

};

#endif // QGSTEXTBUFFERSETTINGS_H
