/***************************************************************************
  qgstextrenderer.h
  -----------------
   begin                : September 2015
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

#ifndef QGSTEXTRENDERER_H
#define QGSTEXTRENDERER_H

#include "qgis_sip.h"
#include "qgis_core.h"
#include "qgsmapunitscale.h"
#include "qgsunittypes.h"

#include <QSharedData>
#include <QPainter>
#include <QPicture>
#include <QDomElement>

class QgsReadWriteContext;
class QgsTextBufferSettingsPrivate;
class QgsTextBackgroundSettingsPrivate;
class QgsTextShadowSettingsPrivate;
class QgsTextSettingsPrivate;
class QgsVectorLayer;
class QgsPaintEffect;

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

  private:

    QSharedDataPointer<QgsTextBufferSettingsPrivate> d;

};

/**
 * \class QgsTextBackgroundSettings
  * \ingroup core
  * Container for settings relating to a text background object.
  * \note QgsTextBackgroundSettings objects are implicitly shared.
  * \since QGIS 3.0
 */

class CORE_EXPORT QgsTextBackgroundSettings
{
  public:

    /**
     * Background shape types.
     */
    enum ShapeType
    {
      ShapeRectangle = 0, //!< Rectangle
      ShapeSquare, //!< Square - buffered sizes only
      ShapeEllipse, //!< Ellipse
      ShapeCircle, //!< Circle
      ShapeSVG //!< SVG file
    };

    /**
     * Methods for determining the background shape size.
     */
    enum SizeType
    {
      SizeBuffer = 0, //!< Shape size is determined by adding a buffer margin around text
      SizeFixed, //!< Fixed size
      SizePercent //!< Shape size is determined by percent of text size
    };

    /**
     * Methods for determining the rotation of the background shape.
     */
    enum RotationType
    {
      RotationSync = 0, //!< Shape rotation is synced with text rotation
      RotationOffset, //!< Shape rotation is offset from text rotation
      RotationFixed //!< Shape rotation is a fixed angle
    };

    QgsTextBackgroundSettings();

    /**
     * Copy constructor.
     * \param other source QgsTextBackgroundSettings
     */
    QgsTextBackgroundSettings( const QgsTextBackgroundSettings &other );

    QgsTextBackgroundSettings &operator=( const QgsTextBackgroundSettings &other );

    ~QgsTextBackgroundSettings();

    /**
     * Returns whether the background is enabled.
     * \see setEnabled()
     */
    bool enabled() const;

    /**
     * Sets whether the text background will be drawn.
     * \param enabled set to TRUE to draw background
     * \see enabled()
     */
    void setEnabled( bool enabled );

    /**
     * Returns the type of background shape (e.g., square, ellipse, SVG).
     * \see setType()
     */
    ShapeType type() const;

    /**
     * Sets the type of background shape to draw (e.g., square, ellipse, SVG).
     * \param type shape type
     * \see type()
     */
    void setType( ShapeType type );

    /**
     * Returns the absolute path to the background SVG file, if set.
     * \see setSvgFile()
     */
    QString svgFile() const;

    /**
     * Sets the path to the background SVG file. This is only used if type() is set to
     * QgsTextBackgroundSettings::ShapeSVG. The path must be absolute.
     * \param file Absolute SVG file path
     * \see svgFile()
     */
    void setSvgFile( const QString &file );

    /**
     * Returns the method used to determine the size of the background shape (e.g., fixed size or buffer
     * around text).
     * \see setSizeType()
     * \see size()
     */
    SizeType sizeType() const;

    /**
     * Sets the method used to determine the size of the background shape (e.g., fixed size or buffer
     * around text).
     * \param type size method
     * \see sizeType()
     * \see setSize()
     */
    void setSizeType( SizeType type );

    /**
     * Returns the size of the background shape. The meaning of the size depends on the current sizeType(),
     * e.g., for size types of QgsTextBackgroundSettings::SizeFixed the size will represent the actual width and
     * height of the shape, for QgsTextBackgroundSettings::SizeBuffer the size will represent the horizontal
     * and vertical margins to add to the text when calculating the size of the shape.
     * \see setSize()
     * \see sizeType()
     */
    QSizeF size() const;

    /**
     * Sets the size of the background shape. The meaning of the size depends on the current sizeType(),
     * e.g., for size types of QgsTextBackgroundSettings::SizeFixed the size will represent the actual width and
     * height of the shape, for QgsTextBackgroundSettings::SizeBuffer the size will represent the horizontal
     * and vertical margins to add to the text when calculating the size of the shape.
     * \param size QSizeF representing horizontal and vertical size components for shape
     * \see size()
     * \see setSizeType()
     */
    void setSize( QSizeF size );

    /**
     * Returns the units used for the shape's size. This value has no meaning if the sizeType() is set to
     * QgsTextBackgroundSettings::SizePercent.
     * \see setSizeUnit()
     * \see sizeType()
     * \see size()
     */
    QgsUnitTypes::RenderUnit sizeUnit() const;

    /**
     * Sets the units used for the shape's size. This value has no meaning if the sizeType() is set to
     * QgsTextBackgroundSettings::SizePercent.
     * \param unit size units
     * \see sizeUnit()
     * \see setSizeType()
     * \see setSize()
     */
    void setSizeUnit( QgsUnitTypes::RenderUnit unit );

    /**
     * Returns the map unit scale object for the shape size. This is only used if the
     * sizeUnit() is set to QgsUnitTypes::RenderMapUnit.
     * \see setSizeMapUnitScale()
     * \see sizeUnit()
     */
    QgsMapUnitScale sizeMapUnitScale() const;

    /**
     * Sets the map unit scale object for the shape size. This is only used if the
     * sizeUnit() is set to QgsUnitTypes::RenderMapUnit.
     * \param scale scale for shape size
     * \see sizeMapUnitScale()
     * \see setSizeUnit()
     */
    void setSizeMapUnitScale( const QgsMapUnitScale &scale );

    /**
     * Returns the method used for rotating the background shape.
     * \see setRotationType()
     * \see rotation()
     */
    RotationType rotationType() const;

    /**
     * Sets the method used for rotating the background shape.
     * \param type rotation method
     * \see rotationType()
     * \see setRotation()
     */
    void setRotationType( RotationType type );

    /**
     * Returns the rotation for the background shape, in degrees clockwise.
     * \see rotationType()
     * \see setRotation()
     */
    double rotation() const;

    /**
     * Sets the \a rotation for the background shape, in degrees clockwise.
     * \see rotation()
     * \see setRotationType()
     */
    void setRotation( double rotation );

    /**
     * Returns the offset used for drawing the background shape. Units are determined
     * via offsetUnit().
     * \see setOffset()
     * \see offsetUnit()
     */
    QPointF offset() const;

    /**
     * Sets the offset used for drawing the background shape. Units are specified using
     * setOffsetUnit().
     * \param offset offset for shape
     * \see offset()
     * \see setOffsetUnit()
     */
    void setOffset( QPointF offset );

    /**
     * Returns the units used for the shape's offset.
     * \see setOffsetUnit()
     * \see offset()
     */
    QgsUnitTypes::RenderUnit offsetUnit() const;

    /**
     * Sets the units used for the shape's offset.
     * \param units offset units
     * \see offsetUnit()
     * \see setOffset()
     */
    void setOffsetUnit( QgsUnitTypes::RenderUnit units );

    /**
     * Returns the map unit scale object for the shape offset. This is only used if the
     * offsetUnit() is set to QgsUnitTypes::RenderMapUnit.
     * \see setOffsetMapUnitScale()
     * \see offsetUnit()
     */
    QgsMapUnitScale offsetMapUnitScale() const;

    /**
     * Sets the map unit scale object for the shape offset. This is only used if the
     * offsetUnit() is set to QgsUnitTypes::RenderMapUnit.
     * \param scale scale for shape offset
     * \see offsetMapUnitScale()
     * \see setOffsetUnit()
     */
    void setOffsetMapUnitScale( const QgsMapUnitScale &scale );

    /**
     * Returns the radii used for rounding the corners of shapes. Units are retrieved
     * through radiiUnit().
     * \see setRadii()
     * \see radiiUnit()
     */
    QSizeF radii() const;

    /**
     * Sets the radii used for rounding the corners of shapes. This is only used if
     * type() is set to QgsTextBackgroundSettings::ShapeRectangle or QgsTextBackgroundSettings::ShapeSquare.
     * \param radii QSizeF representing horizontal and vertical radii for rounded corners. Units are
     * specified through setRadiiUnit()
     * \see radii()
     * \see setRadiiUnit()
     */
    void setRadii( QSizeF radii );

    /**
     * Returns the units used for the shape's radii.
     * \see setRadiiUnit()
     * \see radii()
     */
    QgsUnitTypes::RenderUnit radiiUnit() const;

    /**
     * Sets the units used for the shape's radii.
     * \param units radii units
     * \see radiiUnit()
     * \see setRadii()
     */
    void setRadiiUnit( QgsUnitTypes::RenderUnit units );

    /**
     * Returns the map unit scale object for the shape radii. This is only used if the
     * radiiUnit() is set to QgsUnitTypes::RenderMapUnit.
     * \see setRadiiMapUnitScale()
     * \see radiiUnit()
     */
    QgsMapUnitScale radiiMapUnitScale() const;

    /**
     * Sets the map unit scale object for the shape radii. This is only used if the
     * radiiUnit() is set to QgsUnitTypes::RenderMapUnit.
     * \param scale scale for shape radii
     * \see radiiMapUnitScale()
     * \see setRadiiUnit()
     */
    void setRadiiMapUnitScale( const QgsMapUnitScale &scale );

    /**
     * Returns the background shape's opacity. The opacity is a double value between 0 (fully transparent) and 1 (totally
     * opaque).
     * \see setOpacity()
     */
    double opacity() const;

    /**
     * Sets the background shape's opacity.
     * \param opacity opacity as a double value between 0 (fully transparent) and 1 (totally
     * opaque)
     * \see opacity()
     */
    void setOpacity( double opacity );

    /**
     * Returns the blending mode used for drawing the background shape.
     * \see setBlendMode()
     */
    QPainter::CompositionMode blendMode() const;

    /**
     * Sets the blending mode used for drawing the background shape.
     * \param mode blending mode
     * \see blendMode()
     */
    void setBlendMode( QPainter::CompositionMode mode );

    /**
     * Returns the color used for filing the background shape.
     * \see setFillColor()
     * \see strokeColor()
     */
    QColor fillColor() const;

    /**
     * Sets the color used for filing the background shape.
     * \param color background color
     * \see fillColor()
     * \see setStrokeColor()
     */
    void setFillColor( const QColor &color );

    /**
     * Returns the color used for outlining the background shape.
     * \see setStrokeColor()
     * \see fillColor()
     */
    QColor strokeColor() const;

    /**
     * Sets the color used for outlining the background shape.
     * \param color stroke color
     * \see strokeColor()
     * \see setFillColor()
     */
    void setStrokeColor( const QColor &color );

    /**
     * Returns the width of the shape's stroke (stroke). Units are retrieved through
     * strokeWidthUnit().
     * \see setStrokeWidth()
     * \see strokeWidthUnit()
     */
    double strokeWidth() const;

    /**
     * Sets the width of the shape's stroke (stroke). Units are specified through
     * setStrokeWidthUnit().
     * \see strokeWidth()
     * \see setStrokeWidthUnit()
     */
    void setStrokeWidth( double width );

    /**
     * Returns the units used for the shape's stroke width.
     * \see setStrokeWidthUnit()
     * \see strokeWidth()
     */
    QgsUnitTypes::RenderUnit strokeWidthUnit() const;

    /**
     * Sets the units used for the shape's stroke width.
     * \param units stroke width units
     * \see strokeWidthUnit()
     * \see setStrokeWidth()
     */
    void setStrokeWidthUnit( QgsUnitTypes::RenderUnit units );

    /**
     * Returns the map unit scale object for the shape stroke width. This is only used if the
     * strokeWidthUnit() is set to QgsUnitTypes::RenderMapUnit.
     * \see setStrokeWidthMapUnitScale()
     * \see strokeWidthUnit()
     */
    QgsMapUnitScale strokeWidthMapUnitScale() const;

    /**
     * Sets the map unit scale object for the shape stroke width. This is only used if the
     * strokeWidthUnit() is set to QgsUnitTypes::RenderMapUnit.
     * \param scale scale for shape stroke width
     * \see strokeWidthMapUnitScale()
     * \see setStrokeWidthUnit()
     */
    void setStrokeWidthMapUnitScale( const QgsMapUnitScale &scale );

    /**
     * Returns the join style used for drawing the background shape.
     * \see setJoinStyle
     */
    Qt::PenJoinStyle joinStyle() const;

    /**
     * Sets the join style used for drawing the background shape.
     * \param style join style
     * \see joinStyle()
     */
    void setJoinStyle( Qt::PenJoinStyle style );

    /**
     * Returns the current paint effect for the background shape.
     * \returns paint effect
     * \see setPaintEffect()
     */
    QgsPaintEffect *paintEffect() const;

    /**
     * Sets the current paint \a effect for the background shape.
     * \param effect paint effect. Ownership is transferred to the background settings.
     * \see paintEffect()
     */
    void setPaintEffect( QgsPaintEffect *effect SIP_TRANSFER );

    /**
     * Reads settings from a layer's custom properties (for QGIS 2.x projects).
     * \param layer source vector layer
     */
    void readFromLayer( QgsVectorLayer *layer );

    /**
     * Read settings from a DOM element.
     * \see writeXml()
     */
    void readXml( const QDomElement &elem, const QgsReadWriteContext &context );

    /**
     * Write settings into a DOM element.
     * \see readXml()
     */
    QDomElement writeXml( QDomDocument &doc, const QgsReadWriteContext &context ) const;

  private:

    QSharedDataPointer<QgsTextBackgroundSettingsPrivate> d;

};

/**
 * \class QgsTextShadowSettings
  * \ingroup core
  * Container for settings relating to a text shadow.
  * \note QgsTextShadowSettings objects are implicitly shared.
  * \since QGIS 3.0
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

    /**
     * Copy constructor.
     * \param other source QgsTextShadowSettings
     */
    QgsTextShadowSettings( const QgsTextShadowSettings &other );

    QgsTextShadowSettings &operator=( const QgsTextShadowSettings &other );

    ~QgsTextShadowSettings();

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
    QgsUnitTypes::RenderUnit offsetUnit() const;

    /**
     * Sets the units used for the shadow's offset.
     * \param units shadow distance units
     * \see offsetUnit()
     * \see setOffsetDistance()
     */
    void setOffsetUnit( QgsUnitTypes::RenderUnit units );

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
    QgsUnitTypes::RenderUnit blurRadiusUnit() const;

    /**
     * Sets the units used for the shadow's blur radius.
     * \param units shadow blur radius units
     * \see blurRadiusUnit()
     * \see setBlurRadius()
     */
    void setBlurRadiusUnit( QgsUnitTypes::RenderUnit units );

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

  private:

    QSharedDataPointer<QgsTextShadowSettingsPrivate> d;

};


/**
 * \class QgsTextFormat
  * \ingroup core
  * Container for all settings relating to text rendering.
  * \note QgsTextFormat objects are implicitly shared.
  * \since QGIS 3.0
 */

class CORE_EXPORT QgsTextFormat
{
  public:

    QgsTextFormat();

    /**
     * Copy constructor.
     * \param other source QgsTextFormat
     */
    QgsTextFormat( const QgsTextFormat &other );

    QgsTextFormat &operator=( const QgsTextFormat &other );

    ~QgsTextFormat();

    /**
     * Returns a reference to the text buffer settings.
     * \see setBuffer()
     */
    QgsTextBufferSettings &buffer() { return mBufferSettings; }

    /**
     * Returns a reference to the text buffer settings.
     * \see setBuffer()
     */
    SIP_SKIP QgsTextBufferSettings buffer() const { return mBufferSettings; }

    /**
     * Sets the text's buffer settings.
     * \param bufferSettings buffer settings
     * \see buffer()
     */
    void setBuffer( const QgsTextBufferSettings &bufferSettings ) { mBufferSettings = bufferSettings; }

    /**
     * Returns a reference to the text background settings.
     * \see setBackground()
     */
    QgsTextBackgroundSettings &background() { return mBackgroundSettings; }

    /**
     * Returns a reference to the text background settings.
     * \see setBackground()
     */
    SIP_SKIP QgsTextBackgroundSettings background() const { return mBackgroundSettings; }

    /**
     * Sets the text's background settings.q
     * \param backgroundSettings background settings
     * \see background()
     */
    void setBackground( const QgsTextBackgroundSettings &backgroundSettings ) { mBackgroundSettings = backgroundSettings; }

    /**
     * Returns a reference to the text drop shadow settings.
     * \see setShadow()
     */
    QgsTextShadowSettings &shadow() { return mShadowSettings; }

    /**
     * Returns a reference to the text drop shadow settings.
     * \see setShadow()
     */
    SIP_SKIP QgsTextShadowSettings shadow() const { return mShadowSettings; }

    /**
     * Sets the text's drop shadow settings.
     * \param shadowSettings shadow settings
     * \see shadow()
     */
    void setShadow( const QgsTextShadowSettings &shadowSettings ) { mShadowSettings = shadowSettings; }

    /**
     * Returns the font used for rendering text. Note that the size of the font
     * is not used, and size() should be called instead to determine the size
     * of rendered text.
     * \see scaledFont()
     * \see setFont()
     * \see namedStyle()
     * \see toQFont()
     */
    QFont font() const;

    /**
     * Returns a font with the size scaled to match the format's size settings (including
     * units and map unit scale) for a specified render context.
     * \param context destination render context
     * \returns font with scaled size
     * \see font()
     * \see size()
     */
    QFont scaledFont( const QgsRenderContext &context ) const;

    /**
     * Sets the font used for rendering text. Note that the size of the font
     * is not used, and setSize() should be called instead to explicitly set the size
     * of rendered text.
     * \param font desired font
     * \see font()
     * \see setNamedStyle()
     * \see fromQFont()
     */
    void setFont( const QFont &font );

    /**
     * Returns the named style for the font used for rendering text (e.g., "bold").
     * \see setNamedStyle()
     * \see font()
     */
    QString namedStyle() const;

    /**
     * Sets the named style for the font used for rendering text.
     * \param style named style, e.g., "bold"
     * \see namedStyle()
     * \see setFont()
     */
    void setNamedStyle( const QString &style );

    /**
     * Returns the size for rendered text. Units are retrieved using sizeUnit().
     * \see setSize()
     * \see sizeUnit()
     */
    double size() const;

    /**
     * Sets the size for rendered text.
     * \param size size of rendered text. Units are set using setSizeUnit()
     * \see size()
     * \see setSizeUnit()
     */
    void setSize( double size );

    /**
     * Returns the units for the size of rendered text.
     * \see size()
     * \see setSizeUnit()
     * \see sizeMapUnitScale()
     */
    QgsUnitTypes::RenderUnit sizeUnit() const;

    /**
     * Sets the units for the size of rendered text.
     * \param unit size units
     * \see setSize()
     * \see sizeUnit()
     * \see setSizeMapUnitScale()
     */
    void setSizeUnit( QgsUnitTypes::RenderUnit unit );

    /**
     * Returns the map unit scale object for the size. This is only used if the
     * sizeUnit() is set to QgsUnitTypes::RenderMapUnit.
     * \see setSizeMapUnitScale()
     * \see sizeUnit()
     */
    QgsMapUnitScale sizeMapUnitScale() const;

    /**
     * Sets the map unit scale object for the size. This is only used if the
     * sizeUnit() is set to QgsUnitTypes::RenderMapUnit.
     * \see sizeMapUnitScale()
     * \see setSizeUnit()
     */
    void setSizeMapUnitScale( const QgsMapUnitScale &scale );

    /**
     * Returns the color that text will be rendered in.
     * \see setColor()
     */
    QColor color() const;

    /**
     * Sets the color that text will be rendered in.
     * \param color text color
     * \see color()
     */
    void setColor( const QColor &color );

    /**
     * Returns the text's opacity. The opacity is a double value between 0 (fully transparent) and 1 (totally
     * opaque).
     * \see setOpacity()
     */
    double opacity() const;

    /**
     * Sets the text's opacity.
     * \param opacity opacity as a double value between 0 (fully transparent) and 1 (totally
     * opaque)
     * \see opacity()
     */
    void setOpacity( double opacity );

    /**
     * Returns the blending mode used for drawing the text.
     * \see setBlendMode()
     */
    QPainter::CompositionMode blendMode() const;

    /**
     * Sets the blending mode used for drawing the text.
     * \param mode blending mode
     * \see blendMode()
     */
    void setBlendMode( QPainter::CompositionMode mode );

    /**
     * Returns the line height for text. This is a number between
     * 0.0 and 10.0 representing the leading between lines as a
     * multiplier of line height.
     * \see setLineHeight()
     */
    double lineHeight() const;

    /**
     * Sets the line height for text.
     * \param height a number between
     * 0.0 and 10.0 representing the leading between lines as a
     * multiplier of line height.
     * \see lineHeight()
     */
    void setLineHeight( double height );

    /**
     * Reads settings from a layer's custom properties (for QGIS 2.x projects).
     * \param layer source vector layer
     */
    void readFromLayer( QgsVectorLayer *layer );

    /**
     * Read settings from a DOM element.
     * \see writeXml()
     */
    void readXml( const QDomElement &elem, const QgsReadWriteContext &context );

    /**
     * Write settings into a DOM element.
     * \see readXml()
     */
    QDomElement writeXml( QDomDocument &doc, const QgsReadWriteContext &context ) const;

    /**
     * Returns new mime data representing the text format settings.
     * Caller takes responsibility for deleting the returned object.
     * \see fromMimeData()
     */
    QMimeData *toMimeData() const SIP_FACTORY;

    /**
     * Returns a text format matching the settings from an input \a font.
     * Unlike setFont(), this method also handles the size and size units
     * from \a font.
     * \see toQFont()
     * \since QGIS 3.2
     */
    static QgsTextFormat fromQFont( const QFont &font );

    /**
     * Returns a QFont matching the relevant settings from this text format.
     * Unlike font(), this method also handles the size and size units
     * from the text format.
     * \see fromQFont()
     * \since QGIS 3.2
     */
    QFont toQFont() const;

    /**
     * Attempts to parse the provided mime \a data as a QgsTextFormat.
     * If data can be parsed as a text format, \a ok will be set to TRUE.
     * \see toMimeData()
     */
    static QgsTextFormat fromMimeData( const QMimeData *data, bool *ok SIP_OUT = nullptr );

    /**
     * Returns TRUE if any component of the font format requires advanced effects
     * such as blend modes, which require output in raster formats to be fully respected.
     */
    bool containsAdvancedEffects() const;

    /**
     * Returns TRUE if the specified font was found on the system, or FALSE
     * if the font was not found and a replacement was used instead.
     * \see resolvedFontFamily()
     */
    bool fontFound() const { return mTextFontFound; }

    /**
     * Returns the family for the resolved font, ie if the specified font
     * was not found on the system this will return the name of the replacement
     * font.
     * \see fontFound()
     */
    QString resolvedFontFamily() const { return mTextFontFamily; }

  private:

    QgsTextBufferSettings mBufferSettings;
    QgsTextBackgroundSettings mBackgroundSettings;
    QgsTextShadowSettings mShadowSettings;

    QString mTextFontFamily;
    bool mTextFontFound = true;

    QSharedDataPointer<QgsTextSettingsPrivate> d;

};

/**
 * \class QgsTextRenderer
  * \ingroup core
  * Handles rendering text using rich formatting options, including drop shadows, buffers
  * and background shapes.
  * \since QGIS 3.0
 */

class CORE_EXPORT QgsTextRenderer
{
  public:

    //! Draw mode to calculate width and height
    enum DrawMode
    {
      Rect = 0, //!< Text within rectangle draw mode
      Point, //!< Text at point of origin draw mode
      Label, //!< Label-specific draw mode
    };

    //! Components of text
    enum TextPart
    {
      Text = 0, //!< Text component
      Buffer, //!< Buffer component
      Background, //!< Background shape
      Shadow, //!< Drop shadow
    };

    //! Horizontal alignment
    enum HAlignment
    {
      AlignLeft = 0, //!< Left align
      AlignCenter, //!< Center align
      AlignRight, //!< Right align
    };

    /**
     * Calculates pixel size (considering output size should be in pixel or map units, scale factors and optionally oversampling)
     * \param size size to convert
     * \param c rendercontext
     * \param unit size units
     * \param mapUnitScale a mapUnitScale clamper
     * \returns font pixel size
     */
    static int sizeToPixel( double size, const QgsRenderContext &c, QgsUnitTypes::RenderUnit unit, const QgsMapUnitScale &mapUnitScale = QgsMapUnitScale() );

    // TODO QGIS 4.0 -- remove drawAsOutlines from below methods!

    /**
     * Draws text within a rectangle using the specified settings.
     * \param rect destination rectangle for text
     * \param rotation text rotation
     * \param alignment horizontal alignment
     * \param textLines list of lines of text to draw
     * \param context render context
     * \param format text format
     * \param drawAsOutlines set to FALSE to render text as text. This allows outputs to
     * formats like SVG to maintain text as text objects, but at the cost of degraded
     * rendering and may result in side effects like misaligned text buffers. This setting is deprecated and has no effect
     * as of QGIS 3.4.3 and the text format should be set using QgsRenderContext::setTextRenderFormat() instead.
     */
    static void drawText( const QRectF &rect, double rotation, HAlignment alignment, const QStringList &textLines,
                          QgsRenderContext &context, const QgsTextFormat &format,
                          bool drawAsOutlines = true );

    /**
     * Draws text at a point origin using the specified settings.
     * \param point origin of text
     * \param rotation text rotation
     * \param alignment horizontal alignment
     * \param textLines list of lines of text to draw
     * \param context render context
     * \param format text format
     * \param drawAsOutlines set to FALSE to render text as text. This allows outputs to
     * formats like SVG to maintain text as text objects, but at the cost of degraded
     * rendering and may result in side effects like misaligned text buffers. This setting is deprecated and has no effect
     * as of QGIS 3.4.3 and the text format should be set using QgsRenderContext::setTextRenderFormat() instead.
     */
    static void drawText( QPointF point, double rotation, HAlignment alignment, const QStringList &textLines,
                          QgsRenderContext &context, const QgsTextFormat &format,
                          bool drawAsOutlines = true );

    /**
     * Draws a single component of rendered text using the specified settings.
     * \param rect destination rectangle for text
     * \param rotation text rotation
     * \param alignment horizontal alignment
     * \param textLines list of lines of text to draw
     * \param context render context
     * \param format text format
     * \param part component of text to draw. Note that Shadow parts cannot be drawn
     * individually and instead are drawn with their associated part (e.g., drawn together
     * with the text or background parts)
     * \param drawAsOutlines set to FALSE to render text as text. This allows outputs to
     * formats like SVG to maintain text as text objects, but at the cost of degraded
     * rendering and may result in side effects like misaligned text buffers. This setting is deprecated and has no effect
     * as of QGIS 3.4.3 and the text format should be set using QgsRenderContext::setTextRenderFormat() instead.
     */
    static void drawPart( const QRectF &rect, double rotation, HAlignment alignment, const QStringList &textLines,
                          QgsRenderContext &context, const QgsTextFormat &format,
                          TextPart part, bool drawAsOutlines = true );

    /**
     * Draws a single component of rendered text using the specified settings.
     * \param origin origin for start of text. Y coordinate will be used as baseline.
     * \param rotation text rotation
     * \param alignment horizontal alignment
     * \param textLines list of lines of text to draw
     * \param context render context
     * \param format text format
     * \param part component of text to draw. Note that Shadow parts cannot be drawn
     * individually and instead are drawn with their associated part (e.g., drawn together
     * with the text or background parts)
     * \param drawAsOutlines set to FALSE to render text as text. This allows outputs to
     * formats like SVG to maintain text as text objects, but at the cost of degraded
     * rendering and may result in side effects like misaligned text buffers. This setting is deprecated and has no effect
     * as of QGIS 3.4.3 and the text format should be set using QgsRenderContext::setTextRenderFormat() instead.
     */
    static void drawPart( QPointF origin, double rotation, HAlignment alignment, const QStringList &textLines,
                          QgsRenderContext &context, const QgsTextFormat &format,
                          TextPart part, bool drawAsOutlines = true );

    /**
     * Returns the font metrics for the given text \a format, when rendered
     * in the specified render \a context. The font metrics will take into account
     * all scaling required by the render context.
     * \since QGIS 3.2
     */
    static QFontMetricsF fontMetrics( QgsRenderContext &context, const QgsTextFormat &format );

    /**
     * Returns the width of a text based on a given format.
     * \param context render context
     * \param format text format
     * \param textLines list of lines of text to calculate width from
     * \param fontMetrics font metrics
     */
    static double textWidth( const QgsRenderContext &context, const QgsTextFormat &format, const QStringList &textLines,
                             QFontMetricsF *fontMetrics = nullptr );

    /**
     * Returns the height of a text based on a given format.
     * \param context render context
     * \param format text format
     * \param textLines list of lines of text to calculate width from
     * \param mode draw mode
     * \param fontMetrics font metrics
     */
    static double textHeight( const QgsRenderContext &context, const QgsTextFormat &format, const QStringList &textLines, DrawMode mode,
                              QFontMetricsF *fontMetrics = nullptr );

  private:

    struct Component
    {
      //! Component text
      QString text;
      //! Current origin point for painting (generally current painter rotation point)
      QPointF origin;
      //! Whether to translate the painter to supplied origin
      bool useOrigin = false;
      //! Any rotation to be applied to painter (in radians)
      double rotation = 0.0;
      //! Any rotation to be applied to painter (in radians) after initial rotation
      double rotationOffset = 0.0;
      //! Current center point of label component, after rotation
      QPointF center;
      //! Width and height of label component, transformed and ready for painting
      QSizeF size;
      //! Any translation offsets to be applied before painting, transformed and ready for painting
      QPointF offset;
      //! A stored QPicture of painting for the component
      QPicture picture;

      /**
       * Buffer for component to accommodate graphic items ignored by QPicture,
       * e.g. half-width of an applied QPen, which would extend beyond boundingRect() of QPicture
       */
      double pictureBuffer = 0.0;
      //! A ratio of native painter dpi and that of rendering context's painter
      double dpiRatio = 1.0;
      //! Horizontal alignment
      HAlignment hAlign = AlignLeft;
    };

    static void drawBuffer( QgsRenderContext &context,
                            const Component &component,
                            const QgsTextFormat &format );

    static void drawBackground( QgsRenderContext &context,
                                Component component,
                                const QgsTextFormat &format,
                                const QStringList &textLines,
                                DrawMode mode = Rect );

    static void drawShadow( QgsRenderContext &context,
                            const Component &component,
                            const QgsTextFormat &format );

    static void drawText( QgsRenderContext &context,
                          const Component &component,
                          const QgsTextFormat &format );

    static void drawTextInternal( TextPart drawType,
                                  QgsRenderContext &context,
                                  const QgsTextFormat &format,
                                  const Component &component,
                                  const QStringList &textLines,
                                  const QFontMetricsF *fontMetrics,
                                  HAlignment alignment,
                                  DrawMode mode = Rect );

    friend class QgsVectorLayerLabelProvider;
    friend class QgsLabelPreview;

    static QgsTextFormat updateShadowPosition( const QgsTextFormat &format );


};

#endif // QGSTEXTRENDERER_H
