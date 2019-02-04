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
 */
class CORE_EXPORT QgsSimpleLineSymbolLayer : public QgsLineSymbolLayer
{
  public:
    QgsSimpleLineSymbolLayer( const QColor &color = DEFAULT_SIMPLELINE_COLOR,
                              double width = DEFAULT_SIMPLELINE_WIDTH,
                              Qt::PenStyle penStyle = DEFAULT_SIMPLELINE_PENSTYLE );

    // static stuff

    static QgsSymbolLayer *create( const QgsStringMap &properties = QgsStringMap() ) SIP_FACTORY;
    static QgsSymbolLayer *createFromSld( QDomElement &element ) SIP_FACTORY;

    // implemented from base classes

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

    // new stuff

    Qt::PenStyle penStyle() const { return mPenStyle; }
    void setPenStyle( Qt::PenStyle style ) { mPenStyle = style; }

    Qt::PenJoinStyle penJoinStyle() const { return mPenJoinStyle; }
    void setPenJoinStyle( Qt::PenJoinStyle style ) { mPenJoinStyle = style; }

    Qt::PenCapStyle penCapStyle() const { return mPenCapStyle; }
    void setPenCapStyle( Qt::PenCapStyle style ) { mPenCapStyle = style; }

    bool useCustomDashPattern() const { return mUseCustomDashPattern; }
    void setUseCustomDashPattern( bool b ) { mUseCustomDashPattern = b; }

    /**
     * Sets the units for lengths used in the custom dash pattern.
     * \param unit length units
     * \see customDashPatternUnit()
    */
    void setCustomDashPatternUnit( QgsUnitTypes::RenderUnit unit ) { mCustomDashPatternUnit = unit; }

    /**
     * Returns the units for lengths used in the custom dash pattern.
     * \see setCustomDashPatternUnit()
    */
    QgsUnitTypes::RenderUnit customDashPatternUnit() const { return mCustomDashPatternUnit; }

    const QgsMapUnitScale &customDashPatternMapUnitScale() const { return mCustomDashPatternMapUnitScale; }
    void setCustomDashPatternMapUnitScale( const QgsMapUnitScale &scale ) { mCustomDashPatternMapUnitScale = scale; }

    QVector<qreal> customDashVector() const { return mCustomDashVector; }
    void setCustomDashVector( const QVector<qreal> &vector ) { mCustomDashVector = vector; }

    /**
     * Returns true if the line should only be drawn inside polygons, and any portion
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

    QVector<qreal> dxfCustomDashPattern( QgsUnitTypes::RenderUnit &unit ) const override;
    Qt::PenStyle dxfPenStyle() const override;

    double dxfWidth( const QgsDxfExport &e, QgsSymbolRenderContext &context ) const override;
    double dxfOffset( const QgsDxfExport &e, QgsSymbolRenderContext &context ) const override;
    QColor dxfColor( QgsSymbolRenderContext &context ) const override;

  protected:
    Qt::PenStyle mPenStyle;
    Qt::PenJoinStyle mPenJoinStyle = DEFAULT_SIMPLELINE_JOINSTYLE;
    Qt::PenCapStyle mPenCapStyle = DEFAULT_SIMPLELINE_CAPSTYLE;
    QPen mPen;
    QPen mSelPen;

    //use a custom dash dot pattern instead of the predefined ones
    bool mUseCustomDashPattern = false;
    QgsUnitTypes::RenderUnit mCustomDashPatternUnit = QgsUnitTypes::RenderMillimeters;
    QgsMapUnitScale mCustomDashPatternMapUnitScale;

    //! Vector with an even number of entries for the
    QVector<qreal> mCustomDashVector;

    bool mDrawInsidePolygon = false;

  private:
    //helper functions for data defined symbology
    void applyDataDefinedSymbology( QgsSymbolRenderContext &context, QPen &pen, QPen &selPen, double &offset );
};

/////////

#define DEFAULT_MARKERLINE_ROTATE     true
#define DEFAULT_MARKERLINE_INTERVAL   3

/**
 * \ingroup core
 * \class QgsMarkerLineSymbolLayer
 */
class CORE_EXPORT QgsMarkerLineSymbolLayer : public QgsLineSymbolLayer
{
  public:
    QgsMarkerLineSymbolLayer( bool rotateMarker = DEFAULT_MARKERLINE_ROTATE,
                              double interval = DEFAULT_MARKERLINE_INTERVAL );

    /**
     * Defines how/where the marker should be placed on the line
     */
    enum Placement
    {
      Interval,
      Vertex,
      LastVertex,
      FirstVertex,
      CentralPoint,
      CurvePoint
    };

    // static stuff

    /**
     * Create a new MarkerLineSymbolLayerV2
     *
     * \param properties A property map to deserialize saved information from properties()
     *
     * \returns A new MarkerLineSymbolLayerV2
     */
    static QgsSymbolLayer *create( const QgsStringMap &properties = QgsStringMap() ) SIP_FACTORY;

    /**
     * Create a new MarkerLineSymbolLayerV2 from SLD
     *
     * \param element An SLD XML DOM element
     *
     * \returns A new MarkerLineSymbolLayerV2
     */
    static QgsSymbolLayer *createFromSld( QDomElement &element ) SIP_FACTORY;

    // implemented from base classes

    QString layerType() const override;

    void startRender( QgsSymbolRenderContext &context ) override;

    void stopRender( QgsSymbolRenderContext &context ) override;

    void renderPolyline( const QPolygonF &points, QgsSymbolRenderContext &context ) override;

    void renderPolygonStroke( const QPolygonF &points, QList<QPolygonF> *rings, QgsSymbolRenderContext &context ) override;

    QgsStringMap properties() const override;

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

    // new stuff

    /**
     * Shall the marker be rotated.
     *
     * \returns True if the marker should be rotated.
     */
    bool rotateMarker() const { return mRotateMarker; }

    /**
     * Shall the marker be rotated.
     */
    void setRotateMarker( bool rotate ) { mRotateMarker = rotate; }

    /**
     * Returns the interval between individual markers. Units are specified through intervalUnits().
     * \see setInterval()
     * \see intervalUnit()
     */
    double interval() const { return mInterval; }

    /**
     * Sets the interval between individual markers.
     * \param interval interval size. Units are specified through setIntervalUnit()
     * \see interval()
     * \see setIntervalUnit()
     */
    void setInterval( double interval ) { mInterval = interval; }

    /**
     * The placement of the markers.
     */
    Placement placement() const { return mPlacement; }

    /**
     * The placement of the markers.
     */
    void setPlacement( Placement p ) { mPlacement = p; }

    /**
     * Returns the offset along the line for the marker placement. For Interval placements, this is the distance
     * between the start of the line and the first marker. For FirstVertex and LastVertex placements, this is the
     * distance between the marker and the start of the line or the end of the line respectively.
     * This setting has no effect for Vertex or CentralPoint placements.
     * \returns The offset along the line. The unit for the offset is retrievable via offsetAlongLineUnit.
     * \see setOffsetAlongLine
     * \see offsetAlongLineUnit
     * \see placement
     * \since QGIS 2.3
     */
    double offsetAlongLine() const { return mOffsetAlongLine; }

    /**
     * Sets the the offset along the line for the marker placement. For Interval placements, this is the distance
     * between the start of the line and the first marker. For FirstVertex and LastVertex placements, this is the
     * distance between the marker and the start of the line or the end of the line respectively.
     * This setting has no effect for Vertex or CentralPoint placements.
     * \param offsetAlongLine Distance to offset markers along the line. The offset
     * unit is set via setOffsetAlongLineUnit.
     * \see offsetAlongLine
     * \see setOffsetAlongLineUnit
     * \see setPlacement
     * \since QGIS 2.3
     */
    void setOffsetAlongLine( double offsetAlongLine ) { mOffsetAlongLine = offsetAlongLine; }

    /**
     * Returns the unit used for calculating the offset along line for markers.
     * \returns Offset along line unit type.
     * \see setOffsetAlongLineUnit
     * \see offsetAlongLine
     */
    QgsUnitTypes::RenderUnit offsetAlongLineUnit() const { return mOffsetAlongLineUnit; }

    /**
     * Sets the unit used for calculating the offset along line for markers.
     * \param unit Offset along line unit type.
     * \see offsetAlongLineUnit
     * \see setOffsetAlongLine
     */
    void setOffsetAlongLineUnit( QgsUnitTypes::RenderUnit unit ) { mOffsetAlongLineUnit = unit; }

    /**
     * Returns the map unit scale used for calculating the offset in map units along line for markers.
     * \returns Offset along line map unit scale.
     */
    const QgsMapUnitScale &offsetAlongLineMapUnitScale() const { return mOffsetAlongLineMapUnitScale; }

    /**
     * Sets the map unit scale used for calculating the offset in map units along line for markers.
     * \param scale Offset along line map unit scale.
     */
    void setOffsetAlongLineMapUnitScale( const QgsMapUnitScale &scale ) { mOffsetAlongLineMapUnitScale = scale; }

    /**
     * Sets the units for the interval between markers.
     * \param unit interval units
     * \see intervalUnit()
     * \see setInterval()
    */
    void setIntervalUnit( QgsUnitTypes::RenderUnit unit ) { mIntervalUnit = unit; }

    /**
     * Returns the units for the interval between markers.
     * \see setIntervalUnit()
     * \see interval()
    */
    QgsUnitTypes::RenderUnit intervalUnit() const { return mIntervalUnit; }

    void setIntervalMapUnitScale( const QgsMapUnitScale &scale ) { mIntervalMapUnitScale = scale; }
    const QgsMapUnitScale &intervalMapUnitScale() const { return mIntervalMapUnitScale; }

    void setOutputUnit( QgsUnitTypes::RenderUnit unit ) override;
    QgsUnitTypes::RenderUnit outputUnit() const override;

    void setMapUnitScale( const QgsMapUnitScale &scale ) override;
    QgsMapUnitScale mapUnitScale() const override;

    QSet<QString> usedAttributes( const QgsRenderContext &context ) const override;
    bool hasDataDefinedProperties() const override;

    void setDataDefinedProperty( QgsSymbolLayer::Property key, const QgsProperty &property ) override;


  protected:

    void renderPolylineInterval( const QPolygonF &points, QgsSymbolRenderContext &context );
    void renderPolylineVertex( const QPolygonF &points, QgsSymbolRenderContext &context, Placement placement = Vertex );
    void renderPolylineCentral( const QPolygonF &points, QgsSymbolRenderContext &context );
    double markerAngle( const QPolygonF &points, bool isRing, int vertex );

    bool mRotateMarker;
    double mInterval;
    QgsUnitTypes::RenderUnit mIntervalUnit;
    QgsMapUnitScale mIntervalMapUnitScale;
    std::unique_ptr< QgsMarkerSymbol > mMarker;
    Placement mPlacement;
    double mOffsetAlongLine; //distance to offset along line before marker is drawn
    QgsUnitTypes::RenderUnit mOffsetAlongLineUnit; //unit for offset along line
    QgsMapUnitScale mOffsetAlongLineMapUnitScale;

  private:

#ifdef SIP_RUN
    QgsMarkerLineSymbolLayer( const QgsMarkerLineSymbolLayer &other );
#endif

    /**
     * Renders a marker by offsetting a vertex along the line by a specified distance.
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
};

#endif


