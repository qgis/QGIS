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

#define DEFAULT_SIMPLELINE_COLOR     QColor(35,35,35)
#define DEFAULT_SIMPLELINE_WIDTH     DEFAULT_LINE_WIDTH
#define DEFAULT_SIMPLELINE_PENSTYLE  Qt::SolidLine
#define DEFAULT_SIMPLELINE_JOINSTYLE Qt::BevelJoin
#define DEFAULT_SIMPLELINE_CAPSTYLE  Qt::SquareCap

/**
 * \ingroup core
 * \class QgsSimpleLineSymbolLayer
 * A simple line symbol layer, which renders lines using a line in a variety of styles (e.g. solid, dotted, dashed).
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

    // static stuff

    /**
     * Creates a new QgsSimpleLineSymbolLayer, using the settings
     * serialized in the \a properties map (corresponding to the output from
     * QgsSimpleLineSymbolLayer::properties() ).
     */
    static QgsSymbolLayer *create( const QgsStringMap &properties = QgsStringMap() ) SIP_FACTORY;

    /**
     * Creates a new QgsSimpleLineSymbolLayer from an SLD XML DOM \a element.
     */
    static QgsSymbolLayer *createFromSld( QDomElement &element ) SIP_FACTORY;

    QString layerType() const override;
    void startRender( QgsSymbolRenderContext &context ) override;
    void stopRender( QgsSymbolRenderContext &context ) override;
    void renderPolyline( const QPolygonF &points, QgsSymbolRenderContext &context ) override;
    //overridden so that clip path can be set when using draw inside polygon option
    void renderPolygonStroke( const QPolygonF &points, QList<QPolygonF> *rings, QgsSymbolRenderContext &context ) override;
    QgsStringMap properties() const override;
    QgsSimpleLineSymbolLayer *clone() const override SIP_FACTORY;
    void toSld( QDomDocument &doc, QDomElement &element, const QgsStringMap &props ) const override;
    QString ogrFeatureStyle( double mmScaleFactor, double mapUnitScaleFactor ) const override;
    void setOutputUnit( QgsUnitTypes::RenderUnit unit ) override;
    QgsUnitTypes::RenderUnit outputUnit() const override;
    void setMapUnitScale( const QgsMapUnitScale &scale ) override;
    QgsMapUnitScale mapUnitScale() const override;
    double estimateMaxBleed( const QgsRenderContext &context ) const override;
    QVector<qreal> dxfCustomDashPattern( QgsUnitTypes::RenderUnit &unit ) const override;
    Qt::PenStyle dxfPenStyle() const override;
    double dxfWidth( const QgsDxfExport &e, QgsSymbolRenderContext &context ) const override;
    double dxfOffset( const QgsDxfExport &e, QgsSymbolRenderContext &context ) const override;
    QColor dxfColor( QgsSymbolRenderContext &context ) const override;

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

  private:

    Qt::PenStyle mPenStyle = Qt::SolidLine;
    Qt::PenJoinStyle mPenJoinStyle = DEFAULT_SIMPLELINE_JOINSTYLE;
    Qt::PenCapStyle mPenCapStyle = DEFAULT_SIMPLELINE_CAPSTYLE;
    QPen mPen;
    QPen mSelPen;

    bool mUseCustomDashPattern = false;
    QgsUnitTypes::RenderUnit mCustomDashPatternUnit = QgsUnitTypes::RenderMillimeters;
    QgsMapUnitScale mCustomDashPatternMapUnitScale;

    //! Vector with an even number of entries for the
    QVector<qreal> mCustomDashVector;

    bool mDrawInsidePolygon = false;

    //helper functions for data defined symbology
    void applyDataDefinedSymbology( QgsSymbolRenderContext &context, QPen &pen, QPen &selPen, double &offset );
};

/////////

#define DEFAULT_MARKERLINE_ROTATE     true
#define DEFAULT_MARKERLINE_INTERVAL   3

/**
 * \ingroup core
 * \class QgsTemplatedLineSymbolLayerBase
 *
 * Base class for templated line symbols, e.g. line symbols which draw markers or hash
 * lines at intervals along the line feature.
 *
 * \since QGIS 3.8
 */
class CORE_EXPORT QgsTemplatedLineSymbolLayerBase : public QgsLineSymbolLayer
{
  public:

    /**
     * Defines how/where the templated symbol should be placed on the line.
     */
    enum Placement
    {
      Interval, //!< Place symbols at regular intervals
      Vertex, //!< Place symbols on every vertex in the line
      LastVertex, //!< Place symbols on the last vertex in the line
      FirstVertex, //!< Place symbols on the first vertex in the line
      CentralPoint, //!< Place symbols at the mid point of the line
      CurvePoint, //!< Place symbols at every virtual curve point in the line (used when rendering curved geometry types only)
    };

    /**
     * Constructor for QgsTemplatedLineSymbolLayerBase. Creates a template
     * line placed at the specified \a interval (in millimeters).
     *
     * The \a rotateSymbol argument specifies whether individual symbols
     * should be rotated to match the line segment alignment.
     */
    QgsTemplatedLineSymbolLayerBase( bool rotateSymbol = true,
                                     double interval = 3 );

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
     */
    Placement placement() const { return mPlacement; }

    /**
     * Sets the \a placement of the symbols.
     * \see placement()
     */
    void setPlacement( Placement placement ) { mPlacement = placement; }

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

    void renderPolyline( const QPolygonF &points, QgsSymbolRenderContext &context ) FINAL;
    void renderPolygonStroke( const QPolygonF &points, QList<QPolygonF> *rings, QgsSymbolRenderContext &context ) FINAL;
    QgsUnitTypes::RenderUnit outputUnit() const FINAL;
    void setMapUnitScale( const QgsMapUnitScale &scale ) FINAL;
    QgsMapUnitScale mapUnitScale() const FINAL;
    QgsStringMap properties() const override;

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
     * If \a selected is true then the symbol will be drawn using the "selected feature"
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
    static void setCommonProperties( QgsTemplatedLineSymbolLayerBase *destLayer, const QgsStringMap &properties );

  private:

    void renderPolylineInterval( const QPolygonF &points, QgsSymbolRenderContext &context, double averageAngleOver );
    void renderPolylineVertex( const QPolygonF &points, QgsSymbolRenderContext &context, QgsTemplatedLineSymbolLayerBase::Placement placement = QgsTemplatedLineSymbolLayerBase::Vertex );
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
     * \see setoffsetAlongLine
     * \see setOffsetAlongLineUnit
     */
    void renderOffsetVertexAlongLine( const QPolygonF &points, int vertex, double distance, QgsSymbolRenderContext &context );


    static void collectOffsetPoints( const QVector< QPointF> &points,
                                     QVector< QPointF> &dest, double intervalPainterUnits, double initialOffset, double initialLag = 0,
                                     int numberPointsRequired = -1 );

    bool mRotateSymbols = true;
    double mInterval = 3;
    QgsUnitTypes::RenderUnit mIntervalUnit = QgsUnitTypes::RenderMillimeters;
    QgsMapUnitScale mIntervalMapUnitScale;
    Placement mPlacement = Interval;
    double mOffsetAlongLine = 0; //distance to offset along line before marker is drawn
    QgsUnitTypes::RenderUnit mOffsetAlongLineUnit = QgsUnitTypes::RenderMillimeters; //unit for offset along line
    QgsMapUnitScale mOffsetAlongLineMapUnitScale;
    double mAverageAngleLength = 4;
    QgsUnitTypes::RenderUnit mAverageAngleLengthUnit = QgsUnitTypes::RenderMillimeters;
    QgsMapUnitScale mAverageAngleLengthMapUnitScale;

    friend class TestQgsMarkerLineSymbol;

};

/**
 * \ingroup core
 * \class QgsMarkerLineSymbolLayer
 * Line symbol layer type which draws repeating marker symbols along a line feature.
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

    // static stuff

    /**
     * Creates a new QgsMarkerLineSymbolLayer, using the settings
     * serialized in the \a properties map (corresponding to the output from
     * QgsMarkerLineSymbolLayer::properties() ).
     */
    static QgsSymbolLayer *create( const QgsStringMap &properties = QgsStringMap() ) SIP_FACTORY;

    /**
     * Creates a new QgsMarkerLineSymbolLayer from an SLD XML DOM \a element.
     */
    static QgsSymbolLayer *createFromSld( QDomElement &element ) SIP_FACTORY;

    // implemented from base classes

    QString layerType() const override;
    void startRender( QgsSymbolRenderContext &context ) override;
    void stopRender( QgsSymbolRenderContext &context ) override;
    QgsMarkerLineSymbolLayer *clone() const override SIP_FACTORY;
    void toSld( QDomDocument &doc, QDomElement &element, const QgsStringMap &props ) const override;
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
 * Line symbol layer type which draws repeating line sections along a line feature.
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

    /**
     * Creates a new QgsHashedLineSymbolLayer, using the settings
     * serialized in the \a properties map (corresponding to the output from
     * QgsHashedLineSymbolLayer::properties() ).
     */
    static QgsSymbolLayer *create( const QgsStringMap &properties = QgsStringMap() ) SIP_FACTORY;

    QString layerType() const override;
    void startRender( QgsSymbolRenderContext &context ) override;
    void stopRender( QgsSymbolRenderContext &context ) override;
    QgsStringMap properties() const override;
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

#endif


