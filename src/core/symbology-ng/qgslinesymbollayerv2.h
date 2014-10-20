/***************************************************************************
 qgslinesymbollayerv2.h
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

#ifndef QGSLINESYMBOLLAYERV2_H
#define QGSLINESYMBOLLAYERV2_H

#include "qgssymbollayerv2.h"

#include <QPen>
#include <QVector>

class QgsExpression;

#define DEFAULT_SIMPLELINE_COLOR     QColor(0,0,0)
#define DEFAULT_SIMPLELINE_WIDTH     DEFAULT_LINE_WIDTH
#define DEFAULT_SIMPLELINE_PENSTYLE  Qt::SolidLine
#define DEFAULT_SIMPLELINE_JOINSTYLE Qt::BevelJoin
#define DEFAULT_SIMPLELINE_CAPSTYLE  Qt::SquareCap


class CORE_EXPORT QgsSimpleLineSymbolLayerV2 : public QgsLineSymbolLayerV2
{
  public:
    QgsSimpleLineSymbolLayerV2( QColor color = DEFAULT_SIMPLELINE_COLOR,
                                double width = DEFAULT_SIMPLELINE_WIDTH,
                                Qt::PenStyle penStyle = DEFAULT_SIMPLELINE_PENSTYLE );

    // static stuff

    static QgsSymbolLayerV2* create( const QgsStringMap& properties = QgsStringMap() );
    static QgsSymbolLayerV2* createFromSld( QDomElement &element );

    // implemented from base classes

    QString layerType() const;

    void startRender( QgsSymbolV2RenderContext& context );

    void stopRender( QgsSymbolV2RenderContext& context );

    void renderPolyline( const QPolygonF& points, QgsSymbolV2RenderContext& context );

    //overriden so that clip path can be set when using draw inside polygon option
    void renderPolygonOutline( const QPolygonF& points, QList<QPolygonF>* rings, QgsSymbolV2RenderContext& context );

    QgsStringMap properties() const;

    QgsSymbolLayerV2* clone() const;

    void toSld( QDomDocument &doc, QDomElement &element, QgsStringMap props ) const;

    QString ogrFeatureStyle( double mmScaleFactor, double mapUnitScaleFactor ) const;

    void setOutputUnit( QgsSymbolV2::OutputUnit unit );
    QgsSymbolV2::OutputUnit outputUnit() const;

    void setMapUnitScale( const QgsMapUnitScale &scale );
    QgsMapUnitScale mapUnitScale() const;

    double estimateMaxBleed() const;

    // new stuff

    Qt::PenStyle penStyle() const { return mPenStyle; }
    void setPenStyle( Qt::PenStyle style ) { mPenStyle = style; }

    Qt::PenJoinStyle penJoinStyle() const { return mPenJoinStyle; }
    void setPenJoinStyle( Qt::PenJoinStyle style ) { mPenJoinStyle = style; }

    Qt::PenCapStyle penCapStyle() const { return mPenCapStyle; }
    void setPenCapStyle( Qt::PenCapStyle style ) { mPenCapStyle = style; }

    double offset() const { return mOffset; }
    void setOffset( double offset ) { mOffset = offset; }

    void setOffsetUnit( QgsSymbolV2::OutputUnit unit ) { mOffsetUnit = unit; }
    QgsSymbolV2::OutputUnit offsetUnit() const { return mOffsetUnit; }

    void setOffsetMapUnitScale( const QgsMapUnitScale& scale ) { mOffsetMapUnitScale = scale; }
    const QgsMapUnitScale& offsetMapUnitScale() const { return mOffsetMapUnitScale; }

    bool useCustomDashPattern() const { return mUseCustomDashPattern; }
    void setUseCustomDashPattern( bool b ) { mUseCustomDashPattern = b; }

    void setCustomDashPatternUnit( QgsSymbolV2::OutputUnit unit ) { mCustomDashPatternUnit = unit; }
    QgsSymbolV2::OutputUnit customDashPatternUnit() const { return mCustomDashPatternUnit; }

    const QgsMapUnitScale& customDashPatternMapUnitScale() const { return mCustomDashPatternMapUnitScale; }
    void setCustomDashPatternMapUnitScale( const QgsMapUnitScale& scale ) { mCustomDashPatternMapUnitScale = scale; }

    QVector<qreal> customDashVector() const { return mCustomDashVector; }
    void setCustomDashVector( const QVector<qreal>& vector ) { mCustomDashVector = vector; }

    //Returns true if the line should only be drawn inside the polygon
    bool drawInsidePolygon() const { return mDrawInsidePolygon; }
    //Set to true if the line should only be drawn inside the polygon
    void setDrawInsidePolygon( bool drawInsidePolygon ) { mDrawInsidePolygon = drawInsidePolygon; }

    QVector<qreal> dxfCustomDashPattern( QgsSymbolV2::OutputUnit& unit ) const;
    Qt::PenStyle dxfPenStyle() const;

    double dxfWidth( const QgsDxfExport& e, const QgsSymbolV2RenderContext& context ) const;
    QColor dxfColor( const QgsSymbolV2RenderContext& context ) const;

  protected:
    Qt::PenStyle mPenStyle;
    Qt::PenJoinStyle mPenJoinStyle;
    Qt::PenCapStyle mPenCapStyle;
    QPen mPen;
    QPen mSelPen;
    double mOffset;
    QgsSymbolV2::OutputUnit mOffsetUnit;
    QgsMapUnitScale mOffsetMapUnitScale;

    //use a custom dash dot pattern instead of the predefined ones
    bool mUseCustomDashPattern;
    QgsSymbolV2::OutputUnit mCustomDashPatternUnit;
    QgsMapUnitScale mCustomDashPatternMapUnitScale;

    /**Vector with an even number of entries for the */
    QVector<qreal> mCustomDashVector;

    bool mDrawInsidePolygon;

  private:
    //helper functions for data defined symbology
    void applyDataDefinedSymbology( QgsSymbolV2RenderContext& context, QPen& pen, QPen& selPen, double& offset );
};

/////////

#define DEFAULT_MARKERLINE_ROTATE     true
#define DEFAULT_MARKERLINE_INTERVAL   3

class CORE_EXPORT QgsMarkerLineSymbolLayerV2 : public QgsLineSymbolLayerV2
{
  public:
    QgsMarkerLineSymbolLayerV2( bool rotateMarker = DEFAULT_MARKERLINE_ROTATE,
                                double interval = DEFAULT_MARKERLINE_INTERVAL );

    ~QgsMarkerLineSymbolLayerV2();

    enum Placement
    {
      Interval,
      Vertex,
      LastVertex,
      FirstVertex,
      CentralPoint
    };

    // static stuff

    static QgsSymbolLayerV2* create( const QgsStringMap& properties = QgsStringMap() );
    static QgsSymbolLayerV2* createFromSld( QDomElement &element );

    // implemented from base classes

    QString layerType() const;

    void startRender( QgsSymbolV2RenderContext& context );

    void stopRender( QgsSymbolV2RenderContext& context );

    void renderPolyline( const QPolygonF& points, QgsSymbolV2RenderContext& context );

    void renderPolygonOutline( const QPolygonF& points, QList<QPolygonF>* rings, QgsSymbolV2RenderContext& context );

    QgsStringMap properties() const;

    QgsSymbolLayerV2* clone() const;

    void toSld( QDomDocument &doc, QDomElement &element, QgsStringMap props ) const;

    void setColor( const QColor& color );

    QgsSymbolV2* subSymbol();
    bool setSubSymbol( QgsSymbolV2* symbol );

    virtual void setWidth( double width );
    virtual double width() const;

    double estimateMaxBleed() const;

    // new stuff

    bool rotateMarker() const { return mRotateMarker; }
    void setRotateMarker( bool rotate ) { mRotateMarker = rotate; }

    double interval() const { return mInterval; }
    void setInterval( double interval ) { mInterval = interval; }

    double offset() const { return mOffset; }
    void setOffset( double offset ) { mOffset = offset; }

    Placement placement() const { return mPlacement; }
    void setPlacement( Placement p ) { mPlacement = p; }

    /**Returns the offset along the line for the marker placement. For Interval placements, this is the distance
     * between the start of the line and the first marker. For FirstVertex and LastVertex placements, this is the
     * distance between the marker and the start of the line or the end of the line respectively.
     * This setting has no effect for Vertex or CentralPoint placements.
     * @returns The offset along the line. The unit for the offset is retrievable via offsetAlongLineUnit.
     * @note added in 2.3
     * @see setOffsetAlongLine
     * @see offsetAlongLineUnit
     * @see placement
    */
    double offsetAlongLine() const { return mOffsetAlongLine; }

    /**Sets the the offset along the line for the marker placement. For Interval placements, this is the distance
     * between the start of the line and the first marker. For FirstVertex and LastVertex placements, this is the
     * distance between the marker and the start of the line or the end of the line respectively.
     * This setting has no effect for Vertex or CentralPoint placements.
     * @param offsetAlongLine Distance to offset markers along the line. The offset
     * unit is set via setOffsetAlongLineUnit.
     * @note added in 2.3
     * @see offsetAlongLine
     * @see setOffsetAlongLineUnit
     * @see setPlacement
    */
    void setOffsetAlongLine( double offsetAlongLine ) { mOffsetAlongLine = offsetAlongLine; }

    /**Returns the unit used for calculating the offset along line for markers.
     * @returns Offset along line unit type.
     * @see setOffsetAlongLineUnit
     * @see offsetAlongLine
    */
    QgsSymbolV2::OutputUnit offsetAlongLineUnit() const { return mOffsetAlongLineUnit; }

    /**Sets the unit used for calculating the offset along line for markers.
     * @param unit Offset along line unit type.
     * @see offsetAlongLineUnit
     * @see setOffsetAlongLine
    */
    void setOffsetAlongLineUnit( QgsSymbolV2::OutputUnit unit ) { mOffsetAlongLineUnit = unit; }

    /**Returns the map unit scale used for calculating the offset in map units along line for markers.
     * @returns Offset along line map unit scale.
     */
    const QgsMapUnitScale& offsetAlongLineMapUnitScale() const { return mOffsetAlongLineMapUnitScale; }

    /**Sets the map unit scale used for calculating the offset in map units along line for markers.
     * @param scale Offset along line map unit scale.
     */
    void setOffsetAlongLineMapUnitScale( const QgsMapUnitScale& scale ) { mOffsetAlongLineMapUnitScale = scale; }

    void setIntervalUnit( QgsSymbolV2::OutputUnit unit ) { mIntervalUnit = unit; }
    QgsSymbolV2::OutputUnit intervalUnit() const { return mIntervalUnit; }

    void setIntervalMapUnitScale( const QgsMapUnitScale& scale ) { mIntervalMapUnitScale = scale; }
    const QgsMapUnitScale& intervalMapUnitScale() const { return mIntervalMapUnitScale; }

    void setOffsetUnit( QgsSymbolV2::OutputUnit unit ) { mOffsetUnit = unit; }
    QgsSymbolV2::OutputUnit offsetUnit() const { return mOffsetUnit; }

    void setOffsetMapUnitScale( const QgsMapUnitScale& scale ) { mOffsetMapUnitScale = scale; }
    const QgsMapUnitScale& offsetMapUnitScale() const { return mOffsetMapUnitScale; }

    void setOutputUnit( QgsSymbolV2::OutputUnit unit );
    QgsSymbolV2::OutputUnit outputUnit() const;

    void setMapUnitScale( const QgsMapUnitScale& scale );
    QgsMapUnitScale mapUnitScale() const;

  protected:

    void renderPolylineInterval( const QPolygonF& points, QgsSymbolV2RenderContext& context );
    void renderPolylineVertex( const QPolygonF& points, QgsSymbolV2RenderContext& context, Placement placement = Vertex );
    void renderPolylineCentral( const QPolygonF& points, QgsSymbolV2RenderContext& context );
    double markerAngle( const QPolygonF& points, bool isRing, int vertex );

    bool mRotateMarker;
    double mInterval;
    QgsSymbolV2::OutputUnit mIntervalUnit;
    QgsMapUnitScale mIntervalMapUnitScale;
    QgsMarkerSymbolV2* mMarker;
    double mOffset;
    QgsSymbolV2::OutputUnit mOffsetUnit;
    QgsMapUnitScale mOffsetMapUnitScale;
    Placement mPlacement;
    double mOffsetAlongLine; //distance to offset along line before marker is drawn
    QgsSymbolV2::OutputUnit mOffsetAlongLineUnit; //unit for offset along line
    QgsMapUnitScale mOffsetAlongLineMapUnitScale;

  private:

    /**Renders a marker by offseting a vertex along the line by a specified distance.
     * @param points vertices making up the line
     * @param vertex vertex number to begin offset at
     * @param distance distance to offset from vertex. If distance is positive, offset is calculated
     * moving forward along the line. If distance is negative, offset is calculated moving backward
     * along the line's vertices.
     * @param context render context
     * @see setoffsetAlongLine
     * @see setOffsetAlongLineUnit
    */
    void renderOffsetVertexAlongLine( const QPolygonF& points, int vertex, double distance, QgsSymbolV2RenderContext &context );
};

#endif


