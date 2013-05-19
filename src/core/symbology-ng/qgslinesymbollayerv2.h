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

    QgsStringMap properties() const;

    QgsSymbolLayerV2* clone() const;

    void toSld( QDomDocument &doc, QDomElement &element, QgsStringMap props ) const;

    QString ogrFeatureStyle( double mmScaleFactor, double mapUnitScaleFactor ) const;

    void setOutputUnit( QgsSymbolV2::OutputUnit unit );
    QgsSymbolV2::OutputUnit outputUnit() const;

    // new stuff

    Qt::PenStyle penStyle() const { return mPenStyle; }
    void setPenStyle( Qt::PenStyle style ) { mPenStyle = style; }

    Qt::PenJoinStyle penJoinStyle() const { return mPenJoinStyle; }
    void setPenJoinStyle( Qt::PenJoinStyle style ) { mPenJoinStyle = style; }

    Qt::PenCapStyle penCapStyle() const { return mPenCapStyle; }
    void setPenCapStyle( Qt::PenCapStyle style ) { mPenCapStyle = style; }

    double offset() const { return mOffset; }
    void setOffset( double offset ) { mOffset = offset; }

    QgsSymbolV2::OutputUnit offsetUnit() const { return mOffsetUnit; }
    void setOffsetUnit( QgsSymbolV2::OutputUnit unit ) { mOffsetUnit = unit; }

    bool useCustomDashPattern() const { return mUseCustomDashPattern; }
    void setUseCustomDashPattern( bool b ) { mUseCustomDashPattern = b; }

    QgsSymbolV2::OutputUnit customDashPatternUnit() const { return mCustomDashPatternUnit; }
    void setCustomDashPatternUnit( QgsSymbolV2::OutputUnit unit ) { mCustomDashPatternUnit = unit; }

    QVector<qreal> customDashVector() const { return mCustomDashVector; }
    void setCustomDashVector( const QVector<qreal>& vector ) { mCustomDashVector = vector; }

  protected:
    Qt::PenStyle mPenStyle;
    Qt::PenJoinStyle mPenJoinStyle;
    Qt::PenCapStyle mPenCapStyle;
    QPen mPen;
    QPen mSelPen;
    double mOffset;
    QgsSymbolV2::OutputUnit mOffsetUnit;

    //use a custom dash dot pattern instead of the predefined ones
    bool mUseCustomDashPattern;
    QgsSymbolV2::OutputUnit mCustomDashPatternUnit;

    /**Vector with an even number of entries for the */
    QVector<qreal> mCustomDashVector;

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

    QgsStringMap properties() const;

    QgsSymbolLayerV2* clone() const;

    void toSld( QDomDocument &doc, QDomElement &element, QgsStringMap props ) const;

    void setColor( const QColor& color );

    QgsSymbolV2* subSymbol();
    bool setSubSymbol( QgsSymbolV2* symbol );

    virtual void setWidth( double width );
    virtual double width() const;

    // new stuff

    bool rotateMarker() const { return mRotateMarker; }
    void setRotateMarker( bool rotate ) { mRotateMarker = rotate; }

    double interval() const { return mInterval; }
    void setInterval( double interval ) { mInterval = interval; }

    double offset() const { return mOffset; }
    void setOffset( double offset ) { mOffset = offset; }

    Placement placement() const { return mPlacement; }
    void setPlacement( Placement p ) { mPlacement = p; }

    QgsSymbolV2::OutputUnit intervalUnit() const { return mIntervalUnit; }
    void setIntervalUnit( QgsSymbolV2::OutputUnit unit ) { mIntervalUnit = unit; }

    QgsSymbolV2::OutputUnit offsetUnit() const { return mOffsetUnit; }
    void setOffsetUnit( QgsSymbolV2::OutputUnit unit ) { mOffsetUnit = unit; }

    void setOutputUnit( QgsSymbolV2::OutputUnit unit );
    QgsSymbolV2::OutputUnit outputUnit() const;

  protected:

    void renderPolylineInterval( const QPolygonF& points, QgsSymbolV2RenderContext& context );
    void renderPolylineVertex( const QPolygonF& points, QgsSymbolV2RenderContext& context, Placement placement = Vertex );
    void renderPolylineCentral( const QPolygonF& points, QgsSymbolV2RenderContext& context );

    bool mRotateMarker;
    double mInterval;
    QgsSymbolV2::OutputUnit mIntervalUnit;
    QgsMarkerSymbolV2* mMarker;
    double mOffset;
    QgsSymbolV2::OutputUnit mOffsetUnit;
    Placement mPlacement;
};

/////////

#define DEFAULT_LINEDECORATION_COLOR  QColor(0,0,0)
#define DEFAULT_LINEDECORATION_WIDTH  DEFAULT_LINE_WIDTH

class CORE_EXPORT QgsLineDecorationSymbolLayerV2 : public QgsLineSymbolLayerV2
{
  public:
    QgsLineDecorationSymbolLayerV2( QColor color = DEFAULT_LINEDECORATION_COLOR,
                                    double width = DEFAULT_LINEDECORATION_WIDTH );

    ~QgsLineDecorationSymbolLayerV2();

    // static stuff

    static QgsSymbolLayerV2* create( const QgsStringMap& properties = QgsStringMap() );

    // implemented from base classes

    QString layerType() const;

    void startRender( QgsSymbolV2RenderContext& context );

    void stopRender( QgsSymbolV2RenderContext& context );

    void renderPolyline( const QPolygonF& points, QgsSymbolV2RenderContext& context );

    QgsStringMap properties() const;

    QgsSymbolLayerV2* clone() const;

    void toSld( QDomDocument &doc, QDomElement &element, QgsStringMap props ) const;

    void setOutputUnit( QgsSymbolV2::OutputUnit unit );
    QgsSymbolV2::OutputUnit outputUnit() const;

  protected:
    QPen mPen;
    QPen mSelPen;

};

#endif
