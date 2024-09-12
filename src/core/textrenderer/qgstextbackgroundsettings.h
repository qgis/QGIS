/***************************************************************************
  qgstextbackgroundsettings.h
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

#ifndef QGSTEXTBACKGROUNDSETTINGS_H
#define QGSTEXTBACKGROUNDSETTINGS_H

#include "qgis_sip.h"
#include "qgis_core.h"
#include "qgis.h"
#include "qgsmapunitscale.h"

#include <QString>
#include <QPointF>
#include <QSizeF>
#include <QPainter>
#include <QDomElement>
#include <QSharedDataPointer>

class QgsMarkerSymbol;
class QgsFillSymbol;
class QgsPaintEffect;
class QgsVectorLayer;
class QgsReadWriteContext;
class QgsPropertyCollection;
class QgsTextBackgroundSettingsPrivate;

/**
 * \class QgsTextBackgroundSettings
  * \ingroup core
  * \brief Container for settings relating to a text background object.
  * \note QgsTextBackgroundSettings objects are implicitly shared.
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
      ShapeSVG, //!< SVG file
      ShapeMarkerSymbol, //!< Marker symbol
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

    QgsTextBackgroundSettings( const QgsTextBackgroundSettings &other );

    QgsTextBackgroundSettings &operator=( const QgsTextBackgroundSettings &other );

    ~QgsTextBackgroundSettings();

    bool operator==( const QgsTextBackgroundSettings &other ) const;
    bool operator!=( const QgsTextBackgroundSettings &other ) const;

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
     * Returns the marker symbol to be rendered in the background. Ownership remains with
     * the background settings.
     * \note This is only used when the type() is QgsTextBackgroundSettings::ShapeMarkerSymbol.
     * \see setMarkerSymbol()
     * \since QGIS 3.10
     */
    QgsMarkerSymbol *markerSymbol() const;

    /**
     * Sets the current marker \a symbol for the background shape. Ownership is transferred
     * to the background settings.
     * \note This is only used when the type() is QgsTextBackgroundSettings::ShapeMarkerSymbol.
     * \see markerSymbol()
     * \since QGIS 3.10
     */
    void setMarkerSymbol( QgsMarkerSymbol *symbol SIP_TRANSFER );

    /**
     * Returns the fill symbol to be rendered in the background. Ownership remains with
     * the background settings.
     * \note This is only used when the type() is QgsTextBackgroundSettings::ShapeRectangle,
     * QgsTextBackgroundSettings::ShapeSquare, QgsTextBackgroundSettings::ShapeCircle or
     * QgsTextBackgroundSettings::ShapeEllipse
     * \see setFillSymbol()
     * \since QGIS 3.20
     */
    QgsFillSymbol *fillSymbol() const;

    /**
     * Sets the current fill \a symbol for the background shape. Ownership is transferred
     * to the background settings.
     * \note This is only used when the type() is QgsTextBackgroundSettings::ShapeRectangle,
     * QgsTextBackgroundSettings::ShapeSquare, QgsTextBackgroundSettings::ShapeCircle or
     * QgsTextBackgroundSettings::ShapeEllipse
     * \see fillSymbol()
     * \since QGIS 3.20
     */
    void setFillSymbol( QgsFillSymbol *symbol SIP_TRANSFER );

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
    Qgis::RenderUnit sizeUnit() const;

    /**
     * Sets the units used for the shape's size. This value has no meaning if the sizeType() is set to
     * QgsTextBackgroundSettings::SizePercent.
     * \param unit size units
     * \see sizeUnit()
     * \see setSizeType()
     * \see setSize()
     */
    void setSizeUnit( Qgis::RenderUnit unit );

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
    Qgis::RenderUnit offsetUnit() const;

    /**
     * Sets the units used for the shape's offset.
     * \param units offset units
     * \see offsetUnit()
     * \see setOffset()
     */
    void setOffsetUnit( Qgis::RenderUnit units );

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
    Qgis::RenderUnit radiiUnit() const;

    /**
     * Sets the units used for the shape's radii.
     * \param units radii units
     * \see radiiUnit()
     * \see setRadii()
     */
    void setRadiiUnit( Qgis::RenderUnit units );

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
     * \note As of QGIS 3.20, using this function is only recommended for SVG backgrounds, while
     * other background types should be configured through their symbols.
     */
    QColor fillColor() const;

    /**
     * Sets the color used for filing the background shape.
     * \param color background color
     * \see fillColor()
     * \see setStrokeColor()
     * \note As of QGIS 3.20, using this function is only recommended for SVG backgrounds, while
     * other background types should be configured through their symbols.
     */
    void setFillColor( const QColor &color );

    /**
     * Returns the color used for outlining the background shape.
     * \see setStrokeColor()
     * \see fillColor()
     * \note As of QGIS 3.20, using this function is only recommended for SVG backgrounds, while
     * other background types should be configured through their symbols.
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
     * \note As of QGIS 3.20, using this function is only recommended for SVG backgrounds, while
     * other background types should be configured through their symbols.
     */
    void setStrokeWidth( double width );

    /**
     * Returns the units used for the shape's stroke width.
     * \see setStrokeWidthUnit()
     * \see strokeWidth()
     */
    Qgis::RenderUnit strokeWidthUnit() const;

    /**
     * Sets the units used for the shape's stroke width.
     * \param units stroke width units
     * \see strokeWidthUnit()
     * \see setStrokeWidth()
     * \note As of QGIS 3.20, using this function is only recommended for SVG backgrounds, while
     * other background types should be configured through their symbols.
     */
    void setStrokeWidthUnit( Qgis::RenderUnit units );

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
     * \note As of QGIS 3.20, using this function is only recommended for SVG backgrounds, while
     * other background types should be configured through their symbols.
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
     * \note As of QGIS 3.20, using this function is only recommended for SVG backgrounds, while
     * other background types should be configured through their symbols.
     */
    void setJoinStyle( Qt::PenJoinStyle style );

    /**
     * Returns the current paint effect for the background shape.
     * \returns paint effect
     * \see setPaintEffect()
     */
    const QgsPaintEffect *paintEffect() const;

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

    /**
     * Updates the format by evaluating current values of data defined properties.
     * \note Since QGIS 3.20, data defined fill color, stroke color, stroke width, and
     * pen join style will only modify SVG backgrounds. For other background types, the
     * data defined properties within symbols are to be used.
     * \since QGIS 3.10
     */
    void updateDataDefinedProperties( QgsRenderContext &context, const QgsPropertyCollection &properties );

    /**
     * Upgrade data defined properties when reading a project file saved in QGIS prior to version 3.20.
     * \since QGIS 3.20
     */
    void upgradeDataDefinedProperties( QgsPropertyCollection &properties ) SIP_SKIP;

    /**
     * Returns all field names referenced by the configuration (e.g. from data defined properties).
     * \since QGIS 3.14
     */
    QSet<QString> referencedFields( const QgsRenderContext &context ) const;

  private:

    QSharedDataPointer<QgsTextBackgroundSettingsPrivate> d;

};

#endif // QGSTEXTBACKGROUNDSETTINGS_H
