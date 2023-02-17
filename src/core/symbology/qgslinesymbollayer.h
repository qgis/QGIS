/***************************************************************************
 qgslinesymbollayer.h
 ---------------------
 begin                : November 2009
 copyright            : (C) 2009 by Martin Dobias
 email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSLINESYMBOLLAYER_H
#define QGSLINESYMBOLLAYER_H

#include "qgis_core.h"
#include "qgis.h"
#include "qgssymbollayer.h"

#include <QPen>
#include <QVector>

class QgsExpression;
class QgsMarkerSymbol;
class QgsLineSymbol;
class QgsPathResolver;
class QgsColorRamp;

#define DEFAULT_SIMPLELINE_COLOR     QColor(35,35,35)
#define DEFAULT_SIMPLELINE_WIDTH     DEFAULT_LINE_WIDTH
#define DEFAULT_SIMPLELINE_PENSTYLE  Qt::SolidLine
#define DEFAULT_SIMPLELINE_JOINSTYLE Qt::BevelJoin
#define DEFAULT_SIMPLELINE_CAPSTYLE  Qt::SquareCap

/**
 * \ingroup core
 * \class QgsSimpleLineSymbolLayer
 * \brief A simple line symbol layer, which renders lines using a line in a variety of styles (e.g. solid, dotted, dashed).
 */
class CORE_EXPORT QgsSimpleLineSymbolLayer : public QgsLineSymbolLayer
{
  public:

    /**
     * Constructor for QgsSimpleLineSymbolLayer. Creates a simple line
     * symbol in the specified \a color, \a width (in millimeters)
     * and \a penStyle.
     */
    QgsSimpleLineSymbolLayer( const QColor &color = DEFAULT_SIMPLELINE_COLOR,
                              double width = DEFAULT_SIMPLELINE_WIDTH,
                              Qt::PenStyle penStyle = DEFAULT_SIMPLELINE_PENSTYLE );

    ~QgsSimpleLineSymbolLayer() override;

    // static stuff

    /**
     * Creates a new QgsSimpleLineSymbolLayer, using the settings
     * serialized in the \a properties map (corresponding to the output from
     * QgsSimpleLineSymbolLayer::properties() ).
     */
    static QgsSymbolLayer *create( const QVariantMap &properties = QVariantMap() ) SIP_FACTORY;

    /**
     * Creates a new QgsSimpleLineSymbolLayer from an SLD XML DOM \a element.
     */
    static QgsSymbolLayer *createFromSld( QDomElement &element ) SIP_FACTORY;

    QString layerType() const override;
    void startRender( QgsSymbolRenderContext &context ) override;
    void stopRender( QgsSymbolRenderContext &context ) override;
    void renderPolyline( const QPolygonF &points, QgsSymbolRenderContext &context ) override;
    //overridden so that clip path can be set when using draw inside polygon option
    void renderPolygonStroke( const QPolygonF &points, const QVector<QPolygonF> *rings, QgsSymbolRenderContext &context ) override;
    QVariantMap properties() const override;
    QgsSimpleLineSymbolLayer *clone() const override SIP_FACTORY;
    void toSld( QDomDocument &doc, QDomElement &element, const QVariantMap &props ) const override;
    QString ogrFeatureStyle( double mmScaleFactor, double mapUnitScaleFactor ) const override;
    void setOutputUnit( QgsUnitTypes::RenderUnit unit ) override;
    QgsUnitTypes::RenderUnit outputUnit() const override;
    bool usesMapUnits() const override;
    void setMapUnitScale( const QgsMapUnitScale &scale ) override;
    QgsMapUnitScale mapUnitScale() const override;
    double estimateMaxBleed( const QgsRenderContext &context ) const override;
    QVector<qreal> dxfCustomDashPattern( QgsUnitTypes::RenderUnit &unit ) const override;
    Qt::PenStyle dxfPenStyle() const override;
    double dxfWidth( const QgsDxfExport &e, QgsSymbolRenderContext &context ) const override;
    double dxfOffset( const QgsDxfExport &e, QgsSymbolRenderContext &context ) const override;
    QColor dxfColor( QgsSymbolRenderContext &context ) const override;
    bool canCauseArtifactsBetweenAdjacentTiles() const override;

    /**
     * Returns the pen style used to render the line (e.g. solid, dashed, etc).
     *
     * \see setPenStyle()
     */
    Qt::PenStyle penStyle() const { return mPenStyle; }

    /**
     * Sets the pen \a style used to render the line (e.g. solid, dashed, etc).
     *
     * \see penStyle()
     */
    void setPenStyle( Qt::PenStyle style ) { mPenStyle = style; }

    /**
     * Returns the pen join style used to render the line (e.g. miter, bevel, round, etc).
     *
     * \see setPenJoinStyle()
     */
    Qt::PenJoinStyle penJoinStyle() const { return mPenJoinStyle; }

    /**
     * Sets the pen join \a style used to render the line (e.g. miter, bevel, round, etc).
     *
     * \see penJoinStyle()
     */
    void setPenJoinStyle( Qt::PenJoinStyle style ) { mPenJoinStyle = style; }

    /**
     * Returns the pen cap style used to render the line (e.g. flat, square, round, etc).
     *
     * \see setPenCapStyle()
     */
    Qt::PenCapStyle penCapStyle() const { return mPenCapStyle; }

    /**
     * Sets the pen cap \a style used to render the line (e.g. flat, square, round, etc).
     *
     * \see penCapStyle()
     */
    void setPenCapStyle( Qt::PenCapStyle style ) { mPenCapStyle = style; }

    /**
     * Returns TRUE if the line uses a custom dash pattern.
     * \see setUseCustomDashPattern()
     * \see customDashPatternUnit()
     * \see customDashVector()
     */
    bool useCustomDashPattern() const { return mUseCustomDashPattern; }

    /**
     * Sets whether the line uses a custom dash pattern.
     * \see useCustomDashPattern()
     * \see setCustomDashPatternUnit()
     * \see setCustomDashVector()
     */
    void setUseCustomDashPattern( bool b ) { mUseCustomDashPattern = b; }

    /**
     * Sets the \a unit for lengths used in the custom dash pattern.
     * \see customDashPatternUnit()
    */
    void setCustomDashPatternUnit( QgsUnitTypes::RenderUnit unit ) { mCustomDashPatternUnit = unit; }

    /**
     * Returns the units for lengths used in the custom dash pattern.
     * \see setCustomDashPatternUnit()
    */
    QgsUnitTypes::RenderUnit customDashPatternUnit() const { return mCustomDashPatternUnit; }

    /**
     * Returns the map unit scale for lengths used in the custom dash pattern.
     * \see setCustomDashPatternMapUnitScale()
    */
    const QgsMapUnitScale &customDashPatternMapUnitScale() const { return mCustomDashPatternMapUnitScale; }

    /**
     * Sets the map unit \a scale for lengths used in the custom dash pattern.
     * \see customDashPatternMapUnitScale()
    */
    void setCustomDashPatternMapUnitScale( const QgsMapUnitScale &scale ) { mCustomDashPatternMapUnitScale = scale; }

    /**
     * Returns the custom dash vector, which is the pattern of alternating drawn/skipped lengths
     * used while rendering a custom dash pattern.
     *
     * Units for the vector are specified by customDashPatternUnit()
     *
     * This setting is only used when useCustomDashPattern() returns TRUE.
     *
     * \see setCustomDashVector()
     * \see customDashPatternUnit()
     * \see useCustomDashPattern()
     */
    QVector<qreal> customDashVector() const { return mCustomDashVector; }

    /**
     * Sets the custom dash \a vector, which is the pattern of alternating drawn/skipped lengths
     * used while rendering a custom dash pattern.
     *
     * Units for the vector are specified by customDashPatternUnit()
     *
     * This setting is only used when useCustomDashPattern() returns TRUE.
     *
     * \see customDashVector()
     * \see setCustomDashPatternUnit()
     * \see setUseCustomDashPattern()
     */
    void setCustomDashVector( const QVector<qreal> &vector ) { mCustomDashVector = vector; }

    /**
     * Returns the dash pattern offset, which dictates how far along the dash pattern
     * the pattern should start rendering at.
     *
     * Offset units can be retrieved by calling dashPatternOffsetUnit().
     *
     * \see setDashPatternOffset()
     * \see dashPatternOffsetUnit()
     * \see dashPatternOffsetMapUnitScale()
     *
     * \since QGIS 3.16
     */
    double dashPatternOffset() const { return mDashPatternOffset; }

    /**
     * Sets the dash pattern \a offset, which dictates how far along the dash pattern
     * the pattern should start rendering at.
     *
     * Offset units are set via setDashPatternOffsetUnit().
     *
     * \see dashPatternOffset()
     * \see setDashPatternOffsetUnit()
     * \see setDashPatternOffsetMapUnitScale()
     *
     * \since QGIS 3.16
     */
    void setDashPatternOffset( double offset ) { mDashPatternOffset = offset; }

    /**
     * Sets the \a unit for the dash pattern offset.
     *
     * \see dashPatternOffsetUnit()
     * \see setDashPatternOffset()
     * \see setDashPatternOffsetMapUnitScale()
     *
     * \since QGIS 3.16
    */
    void setDashPatternOffsetUnit( QgsUnitTypes::RenderUnit unit ) { mDashPatternOffsetUnit = unit; }

    /**
     * Returns the units for the dash pattern offset.
     *
     * \see setDashPatternOffsetUnit()
     * \see dashPatternOffset()
     * \see dashPatternOffsetMapUnitScale()
     *
     * \since QGIS 3.16
    */
    QgsUnitTypes::RenderUnit dashPatternOffsetUnit() const { return mDashPatternOffsetUnit; }

    /**
     * Returns the map unit scale for the dash pattern offset value.
     *
     * \see setDashPatternOffsetMapUnitScale()
     * \see dashPatternOffsetUnit()
     * \see dashPatternOffset()
     *
     * \since QGIS 3.16
    */
    const QgsMapUnitScale &dashPatternOffsetMapUnitScale() const { return mDashPatternOffsetMapUnitScale; }

    /**
     * Sets the map unit \a scale for the dash pattern offset.
     *
     * \see dashPatternOffsetMapUnitScale()
     * \see setDashPatternOffset()
     * \see setDashPatternOffsetUnit()
     *
     * \since QGIS 3.16
    */
    void setDashPatternOffsetMapUnitScale( const QgsMapUnitScale &scale ) { mDashPatternOffsetMapUnitScale = scale; }

    /**
     * Returns the trim distance for the start of the line, which dictates a length
     * from the start of the line at which the actual rendering should start.
     *
     * Trim units can be retrieved by calling trimDistanceStartUnit().
     *
     * \see setTrimDistanceStart()
     * \see trimDistanceEnd()
     * \see trimDistanceStartUnit()
     * \see trimDistanceStartMapUnitScale()
     *
     * \since QGIS 3.20
     */
    double trimDistanceStart() const { return mTrimDistanceStart; }

    /**
     * Sets the trim \a distance for the start of the line, which dictates a length
     * from the start of the line at which the actual rendering should start.
     *
     * Trim units can be set by calling setTrimDistanceStartUnit().
     *
     * \see trimDistanceStart()
     * \see setTrimDistanceEnd()
     * \see setTrimDistanceStartUnit()
     * \see setTrimDistanceStartMapUnitScale()
     *
     * \since QGIS 3.20
     */
    void setTrimDistanceStart( double distance ) { mTrimDistanceStart = distance; }

    /**
     * Sets the \a unit for the trim distance for the start of the line.
     *
     * \see trimDistanceStartUnit()
     * \see setTrimDistanceEndUnit()
     * \see setTrimDistanceStart()
     * \see setTrimDistanceStartMapUnitScale()
     *
     * \since QGIS 3.20
    */
    void setTrimDistanceStartUnit( QgsUnitTypes::RenderUnit unit ) { mTrimDistanceStartUnit = unit; }

    /**
     * Returns the unit for the trim distance for the start of the line.
     *
     * \see setTrimDistanceStartUnit()
     * \see trimDistanceEndUnit()
     * \see trimDistanceStart()
     * \see trimDistanceStartMapUnitScale()
     *
     * \since QGIS 3.20
    */
    QgsUnitTypes::RenderUnit trimDistanceStartUnit() const { return mTrimDistanceStartUnit; }

    /**
     * Returns the map unit scale for the trim distance for the start of the line.
     *
     * \see setTrimDistanceStartMapUnitScale()
     * \see trimDistanceEndMapUnitScale()
     * \see trimDistanceStart()
     * \see trimDistanceStartUnit()
     *
     * \since QGIS 3.20
    */
    const QgsMapUnitScale &trimDistanceStartMapUnitScale() const { return mTrimDistanceStartMapUnitScale; }

    /**
     * Sets the map unit \a scale for the trim distance for the start of the line.
     *
     * \see trimDistanceStartMapUnitScale()
     * \see setTrimDistanceEndMapUnitScale()
     * \see setTrimDistanceStart()
     * \see setTrimDistanceStartUnit()
     *
     * \since QGIS 3.20
    */
    void setTrimDistanceStartMapUnitScale( const QgsMapUnitScale &scale ) { mTrimDistanceStartMapUnitScale = scale; }

    /**
     * Returns the trim distance for the end of the line, which dictates a length
     * from the end of the line at which the actual rendering should end.
     *
     * Trim units can be retrieved by calling trimDistanceEndUnit().
     *
     * \see setTrimDistanceEnd()
     * \see trimDistanceStart()
     * \see trimDistanceEndUnit()
     * \see trimDistanceEndMapUnitScale()
     *
     * \since QGIS 3.20
     */
    double trimDistanceEnd() const { return mTrimDistanceEnd; }

    /**
     * Sets the trim \a distance for the end of the line, which dictates a length
     * from the end of the line at which the actual rendering should end.
     *
     * Trim units can be set by calling setTrimDistanceEndUnit().
     *
     * \see trimDistanceEnd()
     * \see setTrimDistanceStart()
     * \see setTrimDistanceEndUnit()
     * \see setTrimDistanceEndMapUnitScale()
     *
     * \since QGIS 3.20
     */
    void setTrimDistanceEnd( double distance ) { mTrimDistanceEnd = distance; }

    /**
     * Sets the \a unit for the trim distance for the end of the line.
     *
     * \see trimDistanceEndUnit()
     * \see setTrimDistanceStartUnit()
     * \see setTrimDistanceEnd()
     * \see setTrimDistanceEndMapUnitScale()
     *
     * \since QGIS 3.20
    */
    void setTrimDistanceEndUnit( QgsUnitTypes::RenderUnit unit ) { mTrimDistanceEndUnit = unit; }

    /**
     * Returns the unit for the trim distance for the end of the line.
     *
     * \see setTrimDistanceEndUnit()
     * \see trimDistanceStartUnit()
     * \see trimDistanceEnd()
     * \see trimDistanceEndMapUnitScale()
     *
     * \since QGIS 3.20
    */
    QgsUnitTypes::RenderUnit trimDistanceEndUnit() const { return mTrimDistanceEndUnit; }

    /**
     * Returns the map unit scale for the trim distance for the end of the line.
     *
     * \see setTrimDistanceEndMapUnitScale()
     * \see trimDistanceStartMapUnitScale()
     * \see trimDistanceEnd()
     * \see trimDistanceEndUnit()
     *
     * \since QGIS 3.20
    */
    const QgsMapUnitScale &trimDistanceEndMapUnitScale() const { return mTrimDistanceEndMapUnitScale; }

    /**
     * Sets the map unit \a scale for the trim distance for the end of the line.
     *
     * \see trimDistanceEndMapUnitScale()
     * \see setTrimDistanceStartMapUnitScale()
     * \see setTrimDistanceEnd()
     * \see setTrimDistanceEndUnit()
     *
     * \since QGIS 3.20
    */
    void setTrimDistanceEndMapUnitScale( const QgsMapUnitScale &scale ) { mTrimDistanceEndMapUnitScale = scale; }

    /**
     * Returns TRUE if the line should only be drawn inside polygons, and any portion
     * of the line which falls outside the polygon should be clipped away.
     *
     * This setting only has an effect when the line symbol is being
     * used to render polygon rings.
     *
     * \see setDrawInsidePolygon()
     */
    bool drawInsidePolygon() const { return mDrawInsidePolygon; }

    /**
     * Sets whether the line should only be drawn inside polygons, and any portion
     * of the line which falls outside the polygon should be clipped away.
     *
     * This setting only has an effect when the line symbol is being
     * used to render polygon rings.
     *
     * \see drawInsidePolygon()
     */
    void setDrawInsidePolygon( bool drawInsidePolygon ) { mDrawInsidePolygon = drawInsidePolygon; }

    /**
     * Returns TRUE if dash patterns should be aligned to the start and end of lines, by
     * applying subtle tweaks to the pattern sizing in order to ensure that the end of
     * a line is represented by a complete dash element.
     *
     * \see setAlignDashPattern()
     * \see tweakDashPatternOnCorners()
     * \since QGIS 3.16
     */
    bool alignDashPattern() const;

    /**
     * Sets whether dash patterns should be aligned to the start and end of lines, by
     * applying subtle tweaks to the pattern sizing in order to ensure that the end of
     * a line is represented by a complete dash element.
     *
     * \see alignDashPattern()
     * \see setTweakDashPatternOnCorners()
     * \since QGIS 3.16
     */
    void setAlignDashPattern( bool enabled );

    /**
     * Returns TRUE if dash patterns tweaks should be applied on sharp corners, to ensure
     * that a double-length dash is drawn running into and out of the corner.
     *
     * \note This setting is only applied if alignDashPattern() is TRUE.
     *
     * \see setTweakDashPatternOnCorners()
     * \see alignDashPattern()
     * \since QGIS 3.16
     */
    bool tweakDashPatternOnCorners() const;

    /**
     * Sets whether dash patterns tweaks should be applied on sharp corners, to ensure
     * that a double-length dash is drawn running into and out of the corner.
     *
     * \note This setting is only applied if alignDashPattern() is TRUE.
     *
     * \see tweakDashPatternOnCorners()
     * \see setAlignDashPattern()
     * \since QGIS 3.16
     */
    void setTweakDashPatternOnCorners( bool enabled );

  private:

    Qt::PenStyle mPenStyle = Qt::SolidLine;
    Qt::PenJoinStyle mPenJoinStyle = DEFAULT_SIMPLELINE_JOINSTYLE;
    Qt::PenCapStyle mPenCapStyle = DEFAULT_SIMPLELINE_CAPSTYLE;
    QPen mPen;
    QPen mSelPen;

    bool mUseCustomDashPattern = false;
    QgsUnitTypes::RenderUnit mCustomDashPatternUnit = QgsUnitTypes::RenderMillimeters;
    QgsMapUnitScale mCustomDashPatternMapUnitScale;

    double mDashPatternOffset = 0;
    QgsUnitTypes::RenderUnit mDashPatternOffsetUnit = QgsUnitTypes::RenderMillimeters;
    QgsMapUnitScale mDashPatternOffsetMapUnitScale;

    double mTrimDistanceStart = 0;
    QgsUnitTypes::RenderUnit mTrimDistanceStartUnit = QgsUnitTypes::RenderMillimeters;
    QgsMapUnitScale mTrimDistanceStartMapUnitScale;

    double mTrimDistanceEnd = 0;
    QgsUnitTypes::RenderUnit mTrimDistanceEndUnit = QgsUnitTypes::RenderMillimeters;
    QgsMapUnitScale mTrimDistanceEndMapUnitScale;

    //! Vector with an even number of entries for the
    QVector<qreal> mCustomDashVector;

    bool mAlignDashPattern = false;
    bool mPatternCartographicTweakOnSharpCorners = false;

    bool mDrawInsidePolygon = false;

    //helper functions for data defined symbology
    void applyDataDefinedSymbology( QgsSymbolRenderContext &context, QPen &pen, QPen &selPen, double &offset );
    void drawPathWithDashPatternTweaks( QPainter *painter, const QPolygonF &points, QPen pen ) const;
};

/////////

#define DEFAULT_MARKERLINE_ROTATE     true
#define DEFAULT_MARKERLINE_INTERVAL   3

/**
 * \ingroup core
 * \class QgsTemplatedLineSymbolLayerBase
 *
 * \brief Base class for templated line symbols, e.g. line symbols which draw markers or hash
 * lines at intervals along the line feature.
 *
 * \since QGIS 3.8
 */
class CORE_EXPORT QgsTemplatedLineSymbolLayerBase : public QgsLineSymbolLayer
{
  public:

    /**
     * Constructor for QgsTemplatedLineSymbolLayerBase. Creates a template
     * line placed at the specified \a interval (in millimeters).
     *
     * The \a rotateSymbol argument specifies whether individual symbols
     * should be rotated to match the line segment alignment.
     */
    QgsTemplatedLineSymbolLayerBase( bool rotateSymbol = true,
                                     double interval = 3 );

    ~QgsTemplatedLineSymbolLayerBase() override;

    /**
     * Returns TRUE if the repeating symbols be rotated to match their line segment orientation.
     * \see setRotateSymbols()
     */
    bool rotateSymbols() const { return mRotateSymbols; }

    /**
     * Sets whether the repeating symbols should be rotated to match their line segment orientation.
     * \see rotateSymbols()
     */
    void setRotateSymbols( bool rotate ) { mRotateSymbols = rotate; }

    /**
     * Returns the interval between individual symbols. Units are specified through intervalUnits().
     * \see setInterval()
     * \see intervalUnit()
     */
    double interval() const { return mInterval; }

    /**
     * Sets the interval between individual symbols.
     * \param interval interval size. Units are specified through setIntervalUnit()
     * \see interval()
     * \see setIntervalUnit()
     */
    void setInterval( double interval ) { mInterval = interval; }

    /**
     * Sets the units for the interval between symbols.
     * \param unit interval units
     * \see intervalUnit()
     * \see setInterval()
    */
    void setIntervalUnit( QgsUnitTypes::RenderUnit unit ) { mIntervalUnit = unit; }

    /**
     * Returns the units for the interval between symbols.
     * \see setIntervalUnit()
     * \see interval()
    */
    QgsUnitTypes::RenderUnit intervalUnit() const { return mIntervalUnit; }

    /**
     * Sets the map unit \a scale for the interval between symbols.
     * \see intervalMapUnitScale()
     * \see setIntervalUnit()
     * \see setInterval()
     */
    void setIntervalMapUnitScale( const QgsMapUnitScale &scale ) { mIntervalMapUnitScale = scale; }

    /**
     * Returns the map unit scale for the interval between symbols.
     * \see setIntervalMapUnitScale()
     * \see intervalUnit()
     * \see interval()
     */
    const QgsMapUnitScale &intervalMapUnitScale() const { return mIntervalMapUnitScale; }

    /**
     * Returns the placement of the symbols.
     * \see setPlacement()
     * \deprecated use placements() instead
     */
    Q_DECL_DEPRECATED Qgis::MarkerLinePlacement placement() const SIP_DEPRECATED;

    /**
     * Sets the \a placement of the symbols.
     * \see placement()
     * \deprecated use setPlacements() instead
     */
    Q_DECL_DEPRECATED void setPlacement( Qgis::MarkerLinePlacement placement ) SIP_DEPRECATED;

    /**
     * Returns the placement of the symbols.
     * \see setPlacements()
     * \since QGIS 3.24
     */
    Qgis::MarkerLinePlacements placements() const { return mPlacements; }

    /**
     * Sets the \a placement of the symbols.
     * \see placements()
     * \since QGIS 3.24
     */
    void setPlacements( Qgis::MarkerLinePlacements placements ) { mPlacements = placements; }

    /**
     * Returns TRUE if the placement applies for every part of multi-part feature geometries.
     *
     * The default is TRUE, which means that Qgis::MarkerLinePlacement::FirstVertex or
     * Qgis::MarkerLinePlacement::LastVertex placements will result in a symbol on
     * the first/last vertex of EVERY part of a multipart feature.
     *
     * If FALSE, then Qgis::MarkerLinePlacement::FirstVertex or
     * Qgis::MarkerLinePlacement::LastVertex placements will result in a symbol on
     * the first/last vertex of the overall multipart geometry only.
     *
     * \see setPlaceOnEveryPart()
     * \since QGIS 3.24
     */
    bool placeOnEveryPart() const { return mPlaceOnEveryPart; }

    /**
     * Sets whether the placement applies for every part of multi-part feature geometries.
     *
     * The default is TRUE, which means that Qgis::MarkerLinePlacement::FirstVertex or
     * Qgis::MarkerLinePlacement::LastVertex placements will result in a symbol on
     * the first/last vertex of EVERY part of a multipart feature.
     *
     * If FALSE, then Qgis::MarkerLinePlacement::FirstVertex or
     * Qgis::MarkerLinePlacement::LastVertex placements will result in a symbol on
     * the first/last vertex of the overall multipart geometry only.
     *
     * \see placeOnEveryPart()
     * \since QGIS 3.24
     */
    void setPlaceOnEveryPart( bool respect ) { mPlaceOnEveryPart = respect; }

    /**
     * Returns the offset along the line for the symbol placement. For Interval placements, this is the distance
     * between the start of the line and the first symbol. For FirstVertex and LastVertex placements, this is the
     * distance between the symbol and the start of the line or the end of the line respectively.
     * This setting has no effect for Vertex or CentralPoint placements.
     * \returns The offset along the line. The unit for the offset is retrievable via offsetAlongLineUnit.
     * \see setOffsetAlongLine()
     * \see offsetAlongLineUnit()
     * \see placement()
     */
    double offsetAlongLine() const { return mOffsetAlongLine; }

    /**
     * Sets the the offset along the line for the symbol placement. For Interval placements, this is the distance
     * between the start of the line and the first symbol. For FirstVertex and LastVertex placements, this is the
     * distance between the symbol and the start of the line or the end of the line respectively.
     * This setting has no effect for Vertex or CentralPoint placements.
     * \param offsetAlongLine Distance to offset markers along the line. The offset
     * unit is set via setOffsetAlongLineUnit.
     * \see offsetAlongLine()
     * \see setOffsetAlongLineUnit()
     * \see setPlacement()
     */
    void setOffsetAlongLine( double offsetAlongLine ) { mOffsetAlongLine = offsetAlongLine; }

    /**
     * Returns the unit used for calculating the offset along line for symbols.
     * \returns Offset along line unit type.
     * \see setOffsetAlongLineUnit()
     * \see offsetAlongLine()
     */
    QgsUnitTypes::RenderUnit offsetAlongLineUnit() const { return mOffsetAlongLineUnit; }

    /**
     * Sets the unit used for calculating the offset along line for symbols.
     * \param unit Offset along line unit type.
     * \see offsetAlongLineUnit()
     * \see setOffsetAlongLine()
     */
    void setOffsetAlongLineUnit( QgsUnitTypes::RenderUnit unit ) { mOffsetAlongLineUnit = unit; }

    /**
     * Returns the map unit scale used for calculating the offset in map units along line for symbols.
     * \see setOffsetAlongLineMapUnitScale()
     */
    const QgsMapUnitScale &offsetAlongLineMapUnitScale() const { return mOffsetAlongLineMapUnitScale; }

    /**
     * Sets the map unit \a scale used for calculating the offset in map units along line for symbols.
     * \see offsetAlongLineMapUnitScale()
     */
    void setOffsetAlongLineMapUnitScale( const QgsMapUnitScale &scale ) { mOffsetAlongLineMapUnitScale = scale; }

    /**
     * Returns the length of line over which the line's direction is averaged when
     * calculating individual symbol angles. Longer lengths smooth out angles from jagged lines to a greater extent.
     *
     * Units are retrieved through averageAngleUnit()
     *
     * \see setAverageAngleLength()
     * \see averageAngleUnit()
     * \see averageAngleMapUnitScale()
     */
    double averageAngleLength() const { return mAverageAngleLength; }

    /**
     * Sets the \a length of line over which the line's direction is averaged when
     * calculating individual symbol angles. Longer lengths smooth out angles from jagged lines to a greater extent.
     *
     * Units are set through setAverageAngleUnit()
     *
     * \see averageAngleLength()
     * \see setAverageAngleUnit()
     * \see setAverageAngleMapUnitScale()
     */
    void setAverageAngleLength( double length ) { mAverageAngleLength = length; }

    /**
     * Sets the \a unit for the length over which the line's direction is averaged when
     * calculating individual symbol angles.
     *
     * \see averageAngleUnit()
     * \see setAverageAngleLength()
     * \see setAverageAngleMapUnitScale()
    */
    void setAverageAngleUnit( QgsUnitTypes::RenderUnit unit ) { mAverageAngleLengthUnit = unit; }

    /**
     * Returns the unit for the length over which the line's direction is averaged when
     * calculating individual symbol angles.
     *
     * \see setAverageAngleUnit()
     * \see averageAngleLength()
     * \see averageAngleMapUnitScale()
    */
    QgsUnitTypes::RenderUnit averageAngleUnit() const { return mAverageAngleLengthUnit; }

    /**
     * Sets the map unit \a scale for the length over which the line's direction is averaged when
     * calculating individual symbol angles.
     *
     * \see averageAngleMapUnitScale()
     * \see setAverageAngleLength()
     * \see setAverageAngleUnit()
    */
    void setAverageAngleMapUnitScale( const QgsMapUnitScale &scale ) { mAverageAngleLengthMapUnitScale = scale; }

    /**
     * Returns the map unit scale for the length over which the line's direction is averaged when
     * calculating individual symbol angles.
     *
     * \see setAverageAngleMapUnitScale()
     * \see averageAngleLength()
     * \see averageAngleUnit()
    */
    const QgsMapUnitScale &averageAngleMapUnitScale() const { return mAverageAngleLengthMapUnitScale; }

    void renderPolyline( const QPolygonF &points, QgsSymbolRenderContext &context ) override;
    void renderPolygonStroke( const QPolygonF &points, const QVector<QPolygonF> *rings, QgsSymbolRenderContext &context ) FINAL;
    QgsUnitTypes::RenderUnit outputUnit() const FINAL;
    void setOutputUnit( QgsUnitTypes::RenderUnit unit ) override;
    void setMapUnitScale( const QgsMapUnitScale &scale ) FINAL;
    QgsMapUnitScale mapUnitScale() const FINAL;
    QVariantMap properties() const override;
    bool canCauseArtifactsBetweenAdjacentTiles() const override;

    void startFeatureRender( const QgsFeature &feature, QgsRenderContext &context ) override;
    void stopFeatureRender( const QgsFeature &feature, QgsRenderContext &context ) override;

  protected:

    /**
     * Sets the line \a angle modification for the symbol's angle. This angle is added to
     * the symbol's rotation and data defined rotation before rendering the symbol, and
     * is used for orienting symbols to match the line's angle.
     * \param angle Angle in degrees, valid values are between 0 and 360
     */
    virtual void setSymbolLineAngle( double angle ) = 0;

    /**
     * Returns the symbol's current angle, in degrees clockwise.
     */
    virtual double symbolAngle() const = 0;

    /**
     * Sets the symbol's \a angle, in degrees clockwise.
     */
    virtual void setSymbolAngle( double angle ) = 0;

    /**
     * Renders the templated symbol at the specified \a point, using the given render \a context.
     *
     * The \a feature argument is used to pass the feature currently being rendered (when available).
     *
     * If only a single symbol layer from the symbol should be rendered, it should be specified
     * in the \a layer argument. A \a layer of -1 indicates that all symbol layers should be
     * rendered.
     *
     * If \a selected is TRUE then the symbol will be drawn using the "selected feature"
     * style and colors instead of the symbol's normal style.
     */
    virtual void renderSymbol( const QPointF &point, const QgsFeature *feature, QgsRenderContext &context, int layer = -1, bool selected = false ) = 0;

    /**
     * Copies all common properties of this layer to another templated symbol layer.
     */
    void copyTemplateSymbolProperties( QgsTemplatedLineSymbolLayerBase *destLayer ) const;

    /**
     * Sets all common symbol properties in the \a destLayer, using the settings
     * serialized in the \a properties map.
     */
    static void setCommonProperties( QgsTemplatedLineSymbolLayerBase *destLayer, const QVariantMap &properties );

  private:

    void renderPolylineInterval( const QPolygonF &points, QgsSymbolRenderContext &context, double averageAngleOver );
    void renderPolylineVertex( const QPolygonF &points, QgsSymbolRenderContext &context, Qgis::MarkerLinePlacement placement = Qgis::MarkerLinePlacement::Vertex );
    void renderPolylineCentral( const QPolygonF &points, QgsSymbolRenderContext &context, double averageAngleOver );
    double markerAngle( const QPolygonF &points, bool isRing, int vertex );

    /**
     * Renders a symbol by offsetting a vertex along the line by a specified distance.
     * \param points vertices making up the line
     * \param vertex vertex number to begin offset at
     * \param distance distance to offset from vertex. If distance is positive, offset is calculated
     * moving forward along the line. If distance is negative, offset is calculated moving backward
     * along the line's vertices.
     * \param context render context
     * \param placement marker placement
     * \see setoffsetAlongLine
     * \see setOffsetAlongLineUnit
     */
    void renderOffsetVertexAlongLine( const QPolygonF &points, int vertex, double distance, QgsSymbolRenderContext &context,
                                      Qgis::MarkerLinePlacement placement );


    static void collectOffsetPoints( const QVector< QPointF> &points,
                                     QVector< QPointF> &dest, double intervalPainterUnits, double initialOffset, double initialLag = 0,
                                     int numberPointsRequired = -1 );

    bool mRotateSymbols = true;
    double mInterval = 3;
    QgsUnitTypes::RenderUnit mIntervalUnit = QgsUnitTypes::RenderMillimeters;
    QgsMapUnitScale mIntervalMapUnitScale;
    Qgis::MarkerLinePlacements mPlacements = Qgis::MarkerLinePlacement::Interval;
    double mOffsetAlongLine = 0; //distance to offset along line before marker is drawn
    QgsUnitTypes::RenderUnit mOffsetAlongLineUnit = QgsUnitTypes::RenderMillimeters; //unit for offset along line
    QgsMapUnitScale mOffsetAlongLineMapUnitScale;
    double mAverageAngleLength = 4;
    QgsUnitTypes::RenderUnit mAverageAngleLengthUnit = QgsUnitTypes::RenderMillimeters;
    QgsMapUnitScale mAverageAngleLengthMapUnitScale;
    bool mPlaceOnEveryPart = true;

    bool mRenderingFeature = false;
    bool mHasRenderedFirstPart = false;
    QPointF mFinalVertex;
    bool mCurrentFeatureIsSelected = false;
    double mFeatureSymbolOpacity = 1;

    friend class TestQgsMarkerLineSymbol;

};

/**
 * \ingroup core
 * \class QgsMarkerLineSymbolLayer
 * \brief Line symbol layer type which draws repeating marker symbols along a line feature.
 */
class CORE_EXPORT QgsMarkerLineSymbolLayer : public QgsTemplatedLineSymbolLayerBase
{
  public:

    /**
     * Constructor for QgsMarkerLineSymbolLayer. Creates a marker line
     * with a default marker symbol, placed at the specified \a interval (in millimeters).
     *
     * The \a rotateMarker argument specifies whether individual marker symbols
     * should be rotated to match the line segment alignment.
     */
    QgsMarkerLineSymbolLayer( bool rotateMarker = DEFAULT_MARKERLINE_ROTATE,
                              double interval = DEFAULT_MARKERLINE_INTERVAL );

    ~QgsMarkerLineSymbolLayer() override;

    // static stuff

    /**
     * Creates a new QgsMarkerLineSymbolLayer, using the settings
     * serialized in the \a properties map (corresponding to the output from
     * QgsMarkerLineSymbolLayer::properties() ).
     */
    static QgsSymbolLayer *create( const QVariantMap &properties = QVariantMap() ) SIP_FACTORY;

    /**
     * Creates a new QgsMarkerLineSymbolLayer from an SLD XML DOM \a element.
     */
    static QgsSymbolLayer *createFromSld( QDomElement &element ) SIP_FACTORY;

    // implemented from base classes

    QString layerType() const override;
    void startRender( QgsSymbolRenderContext &context ) override;
    void stopRender( QgsSymbolRenderContext &context ) override;
    QgsMarkerLineSymbolLayer *clone() const override SIP_FACTORY;
    void toSld( QDomDocument &doc, QDomElement &element, const QVariantMap &props ) const override;
    void setColor( const QColor &color ) override;
    QColor color() const override;
    QgsSymbol *subSymbol() override;
    bool setSubSymbol( QgsSymbol *symbol SIP_TRANSFER ) override;
    void setWidth( double width ) override;
    double width() const override;
    double width( const QgsRenderContext &context ) const override;
    double estimateMaxBleed( const QgsRenderContext &context ) const override;
    void setOutputUnit( QgsUnitTypes::RenderUnit unit ) override;
    bool usesMapUnits() const override;
    QSet<QString> usedAttributes( const QgsRenderContext &context ) const override;
    bool hasDataDefinedProperties() const override;
    void setDataDefinedProperty( QgsSymbolLayer::Property key, const QgsProperty &property ) override;

    /**
     * Shall the marker be rotated.
     *
     * \returns TRUE if the marker should be rotated.
     * \deprecated Use rotateSymbols() instead.
     */
    Q_DECL_DEPRECATED bool rotateMarker() const SIP_DEPRECATED { return rotateSymbols(); }

    /**
     * Shall the marker be rotated.
     * \deprecated Use setRotateSymbols() instead.
     */
    Q_DECL_DEPRECATED void setRotateMarker( bool rotate ) SIP_DEPRECATED { setRotateSymbols( rotate ); }

    void renderPolyline( const QPolygonF &points, QgsSymbolRenderContext &context ) override;

  protected:

    std::unique_ptr< QgsMarkerSymbol > mMarker;

    void setSymbolLineAngle( double angle ) override;
    double symbolAngle() const override;
    void setSymbolAngle( double angle ) override;
    void renderSymbol( const QPointF &point, const QgsFeature *feature, QgsRenderContext &context, int layer = -1, bool selected = false ) override;

  private:

#ifdef SIP_RUN
    QgsMarkerLineSymbolLayer( const QgsMarkerLineSymbolLayer &other );
#endif


};


/**
 * \ingroup core
 * \class QgsHashedLineSymbolLayer
 *
 * \brief Line symbol layer type which draws repeating line sections along a line feature.
 *
 * \since QGIS 3.8
 */
class CORE_EXPORT QgsHashedLineSymbolLayer : public QgsTemplatedLineSymbolLayerBase
{
  public:

    /**
     * Constructor for QgsHashedLineSymbolLayer. Creates a line
     * with a default hash symbol, placed at the specified \a interval (in millimeters).
     *
     * The \a rotateSymbol argument specifies whether individual hash symbols
     * should be rotated to match the line segment alignment.
     */
    QgsHashedLineSymbolLayer( bool rotateSymbol = true,
                              double interval = 3 );

    ~QgsHashedLineSymbolLayer() override;

    /**
     * Creates a new QgsHashedLineSymbolLayer, using the settings
     * serialized in the \a properties map (corresponding to the output from
     * QgsHashedLineSymbolLayer::properties() ).
     */
    static QgsSymbolLayer *create( const QVariantMap &properties = QVariantMap() ) SIP_FACTORY;

    QString layerType() const override;
    void startRender( QgsSymbolRenderContext &context ) override;
    void stopRender( QgsSymbolRenderContext &context ) override;
    QVariantMap properties() const override;
    QgsHashedLineSymbolLayer *clone() const override SIP_FACTORY;
    void setColor( const QColor &color ) override;
    QColor color() const override;
    QgsSymbol *subSymbol() override;
    bool setSubSymbol( QgsSymbol *symbol SIP_TRANSFER ) override;
    void setWidth( double width ) override;
    double width() const override;
    double width( const QgsRenderContext &context ) const override;
    double estimateMaxBleed( const QgsRenderContext &context ) const override;
    void setOutputUnit( QgsUnitTypes::RenderUnit unit ) override;
    QSet<QString> usedAttributes( const QgsRenderContext &context ) const override;
    bool hasDataDefinedProperties() const override;
    void setDataDefinedProperty( QgsSymbolLayer::Property key, const QgsProperty &property ) override;
    bool usesMapUnits() const override;

    /**
     * Returns the angle to use when drawing the hashed lines sections, in degrees clockwise.
     *
     * \see setHashAngle()
     */
    double hashAngle() const;

    /**
     * Sets the \a angle to use when drawing the hashed lines sections, in degrees clockwise.
     *
     * \see hashAngle()
     */
    void setHashAngle( double angle );

    /**
     * Returns the length of hash symbols. Units are specified through hashLengthUnits().
     * \see setHashLength()
     * \see hashLengthUnit()
     */
    double hashLength() const { return mHashLength; }

    /**
     * Sets the \a length of hash symbols. Units are specified through setHashLengthUnit()
     * \see hashLength()
     * \see setHashLengthUnit()
     */
    void setHashLength( double length ) { mHashLength = length; }

    /**
     * Sets the \a unit for the length of hash symbols.
     * \see hashLengthUnit()
     * \see setHashLength()
    */
    void setHashLengthUnit( QgsUnitTypes::RenderUnit unit ) { mHashLengthUnit = unit; }

    /**
     * Returns the units for the length of hash symbols.
     * \see setHashLengthUnit()
     * \see hashLength()
    */
    QgsUnitTypes::RenderUnit hashLengthUnit() const { return mHashLengthUnit; }

    /**
     * Sets the map unit \a scale for the hash length.
     * \see hashLengthMapUnitScale()
     * \see setHashLengthUnit()
     * \see setHashLength()
     */
    void setHashLengthMapUnitScale( const QgsMapUnitScale &scale ) { mHashLengthMapUnitScale = scale; }

    /**
     * Returns the map unit scale for the hash length.
     * \see setHashLengthMapUnitScale()
     * \see hashLengthUnit()
     * \see hashLength()
     */
    const QgsMapUnitScale &hashLengthMapUnitScale() const { return mHashLengthMapUnitScale; }

    void renderPolyline( const QPolygonF &points, QgsSymbolRenderContext &context ) override;

  protected:

    void setSymbolLineAngle( double angle ) override;
    double symbolAngle() const override;
    void setSymbolAngle( double angle ) override;
    void renderSymbol( const QPointF &point, const QgsFeature *feature, QgsRenderContext &context, int layer = -1, bool selected = false ) override;

  private:
#ifdef SIP_RUN
    QgsHashedLineSymbolLayer( const QgsHashedLineSymbolLayer &other );
#endif

    std::unique_ptr< QgsLineSymbol > mHashSymbol;

    double mSymbolLineAngle = 0;
    double mSymbolAngle = 0;

    double mHashAngle = 0;
    double mHashLength = 3;
    QgsUnitTypes::RenderUnit mHashLengthUnit = QgsUnitTypes::RenderMillimeters;
    QgsMapUnitScale mHashLengthMapUnitScale;

};


/**
 * \ingroup core
 * \class QgsAbstractBrushedLineSymbolLayer
 *
 * \brief Base class for line symbol layer types which draws line sections using a QBrush.
 *
 * \since QGIS 3.24
 */
class CORE_EXPORT QgsAbstractBrushedLineSymbolLayer : public QgsLineSymbolLayer
{
  public:

    /**
     * Returns the pen join style used to render the line (e.g. miter, bevel, round, etc).
     *
     * \see setPenJoinStyle()
     */
    Qt::PenJoinStyle penJoinStyle() const { return mPenJoinStyle; }

    /**
     * Sets the pen join \a style used to render the line (e.g. miter, bevel, round, etc).
     *
     * \see penJoinStyle()
     */
    void setPenJoinStyle( Qt::PenJoinStyle style ) { mPenJoinStyle = style; }

    /**
     * Returns the pen cap style used to render the line (e.g. flat, square, round, etc).
     *
     * \see setPenCapStyle()
     */
    Qt::PenCapStyle penCapStyle() const { return mPenCapStyle; }

    /**
     * Sets the pen cap \a style used to render the line (e.g. flat, square, round, etc).
     *
     * \see penCapStyle()
     */
    void setPenCapStyle( Qt::PenCapStyle style ) { mPenCapStyle = style; }

  protected:

    /**
     * Renders a polyline of \a points using the specified \a brush.
     */
    void renderPolylineUsingBrush( const QPolygonF &points, QgsSymbolRenderContext &context, const QBrush &brush,
                                   double patternThickness, double patternLength );

    Qt::PenJoinStyle mPenJoinStyle = Qt::PenJoinStyle::RoundJoin;
    Qt::PenCapStyle mPenCapStyle = Qt::PenCapStyle::RoundCap;

  private:
    void renderLine( const QPolygonF &points, QgsSymbolRenderContext &context, const double lineThickness, const double patternLength, const QBrush &sourceBrush );
};



/**
 * \ingroup core
 * \class QgsRasterLineSymbolLayer
 *
 * \brief Line symbol layer type which draws line sections using a raster image file.
 *
 * \since QGIS 3.24
 */
class CORE_EXPORT QgsRasterLineSymbolLayer : public QgsAbstractBrushedLineSymbolLayer
{
  public:

    /**
     * Constructor for QgsRasterLineSymbolLayer, with the specified raster image path.
     */
    QgsRasterLineSymbolLayer( const QString &path = QString() );
    virtual ~QgsRasterLineSymbolLayer();

    /**
     * Creates a new QgsRasterLineSymbolLayer, using the settings
     * serialized in the \a properties map (corresponding to the output from
     * QgsRasterLineSymbolLayer::properties() ).
     */
    static QgsSymbolLayer *create( const QVariantMap &properties = QVariantMap() ) SIP_FACTORY;

    /**
     * Turns relative paths in properties map to absolute when reading and vice versa when writing.
     * Used internally when reading/writing symbols.
     */
    static void resolvePaths( QVariantMap &properties, const QgsPathResolver &pathResolver, bool saving );

    /**
     * Returns the raster image path.
     * \see setPath()
     */
    QString path() const { return mPath; }

    /**
     * Set the raster image \a path.
     * \see path()
     */
    void setPath( const QString &path );

    /**
     * Returns the line opacity.
     * \returns opacity value between 0 (fully transparent) and 1 (fully opaque)
     * \see setOpacity()
     */
    double opacity() const { return mOpacity; }

    /**
     * Set the line opacity.
     * \param opacity opacity value between 0 (fully transparent) and 1 (fully opaque)
     * \see opacity()
     */
    void setOpacity( double opacity ) { mOpacity = opacity; }

    QString layerType() const override;
    void startRender( QgsSymbolRenderContext &context ) override;
    void stopRender( QgsSymbolRenderContext &context ) override;
    void renderPolyline( const QPolygonF &points, QgsSymbolRenderContext &context ) override;
    QVariantMap properties() const override;
    QgsRasterLineSymbolLayer *clone() const override SIP_FACTORY;
    void setOutputUnit( QgsUnitTypes::RenderUnit unit ) override;
    QgsUnitTypes::RenderUnit outputUnit() const override;
    bool usesMapUnits() const override;
    void setMapUnitScale( const QgsMapUnitScale &scale ) override;
    QgsMapUnitScale mapUnitScale() const override;
    double estimateMaxBleed( const QgsRenderContext &context ) const override;
    QColor color() const override;

  protected:
    QString mPath;
    double mOpacity = 1.0;
    QImage mLineImage;

};


/**
 * \ingroup core
 * \class QgsLineburstSymbolLayer
 *
 * \brief Line symbol layer type which draws a gradient pattern perpendicularly along a line.
 *
 * See QgsInterpolatedLineSymbolLayer for a line symbol layer which draws gradients along the length
 * of a line.
 *
 * \since QGIS 3.24
 */
class CORE_EXPORT QgsLineburstSymbolLayer : public QgsAbstractBrushedLineSymbolLayer
{
  public:

    /**
     * Constructor for QgsLineburstSymbolLayer, with the specified start and end gradient colors.
     */
    QgsLineburstSymbolLayer( const QColor &color = DEFAULT_SIMPLELINE_COLOR,
                             const QColor &color2 = Qt::white );
    ~QgsLineburstSymbolLayer() override;

    /**
     * Creates a new QgsLineburstSymbolLayer, using the settings
     * serialized in the \a properties map (corresponding to the output from
     * QgsLineburstSymbolLayer::properties() ).
     */
    static QgsSymbolLayer *create( const QVariantMap &properties = QVariantMap() ) SIP_FACTORY;

    QString layerType() const override;
    void startRender( QgsSymbolRenderContext &context ) override;
    void stopRender( QgsSymbolRenderContext &context ) override;
    void renderPolyline( const QPolygonF &points, QgsSymbolRenderContext &context ) override;
    QVariantMap properties() const override;
    QgsLineburstSymbolLayer *clone() const override SIP_FACTORY;
    void setOutputUnit( QgsUnitTypes::RenderUnit unit ) override;
    QgsUnitTypes::RenderUnit outputUnit() const override;
    bool usesMapUnits() const override;
    void setMapUnitScale( const QgsMapUnitScale &scale ) override;
    QgsMapUnitScale mapUnitScale() const override;
    double estimateMaxBleed( const QgsRenderContext &context ) const override;
    QColor color() const override;

    /**
     * Returns the gradient color mode, which controls how gradient color stops are created.
     *
     * \see setGradientColorType()
     */
    Qgis::GradientColorSource gradientColorType() const { return mGradientColorType; }

    /**
     * Sets the gradient color mode, which controls how gradient color stops are created.
     *
     * \see gradientColorType()
     */
    void setGradientColorType( Qgis::GradientColorSource gradientColorType ) { mGradientColorType = gradientColorType; }

    /**
     * Returns the color ramp used for the gradient line. This is only
     * used if the gradient color type is set to ColorRamp.
     * \see setColorRamp()
     * \see gradientColorType()
     */
    QgsColorRamp *colorRamp();

    /**
     * Sets the color ramp used for the gradient line. This is only
     * used if the gradient color type is set to ColorRamp.
     * \param ramp color ramp. Ownership is transferred.
     * \see colorRamp()
     * \see setGradientColorType()
     */
    void setColorRamp( QgsColorRamp *ramp SIP_TRANSFER );

    /**
     * Returns the color for endpoint of gradient, only used if the gradient color type is set to SimpleTwoColor.
     *
     * \see setColor2()
     */
    QColor color2() const { return mColor2; }

    /**
     * Sets the color for endpoint of gradient, only used if the gradient color type is set to SimpleTwoColor.
     *
     * \see color2()
     */
    void setColor2( const QColor &color2 ) { mColor2 = color2; }

  protected:
    Qgis::GradientColorSource mGradientColorType = Qgis::GradientColorSource::SimpleTwoColor;
    QColor mColor2;
    std::unique_ptr< QgsColorRamp > mGradientRamp;

};

#endif


