
#ifndef QGSLINESYMBOLLAYERV2_H
#define QGSLINESYMBOLLAYERV2_H

#include "qgssymbollayerv2.h"

#include <QPen>

#define DEFAULT_SIMPLELINE_COLOR     QColor(0,0,0)
#define DEFAULT_SIMPLELINE_WIDTH     1
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

    // implemented from base classes

    QString layerType() const;

    void startRender( QgsRenderContext& context );

    void stopRender( QgsRenderContext& context );

    void renderPolyline( const QPolygonF& points, QgsRenderContext& context );

    QgsStringMap properties() const;

    QgsSymbolLayerV2* clone() const;

    // new stuff

    Qt::PenStyle penStyle() const { return mPenStyle; }
    void setPenStyle( Qt::PenStyle style ) { mPenStyle = style; }

    Qt::PenJoinStyle penJoinStyle() const { return mPenJoinStyle; }
    void setPenJoinStyle( Qt::PenJoinStyle style ) { mPenJoinStyle = style; }

    Qt::PenCapStyle penCapStyle() const { return mPenCapStyle; }
    void setPenCapStyle( Qt::PenCapStyle style ) { mPenCapStyle = style; }

    double offset() const { return mOffset; }
    void setOffset( double offset ) { mOffset = offset; }

  protected:
    Qt::PenStyle mPenStyle;
    Qt::PenJoinStyle mPenJoinStyle;
    Qt::PenCapStyle mPenCapStyle;
    QPen mPen;
    double mOffset;
};

/////////

#define DEFAULT_MARKERLINE_ROTATE     true
#define DEFAULT_MARKERLINE_INTERVAL   10

class CORE_EXPORT QgsMarkerLineSymbolLayerV2 : public QgsLineSymbolLayerV2
{
  public:
    QgsMarkerLineSymbolLayerV2( bool rotateMarker = DEFAULT_MARKERLINE_ROTATE,
                                double interval = DEFAULT_MARKERLINE_INTERVAL );

    ~QgsMarkerLineSymbolLayerV2();

    // static stuff

    static QgsSymbolLayerV2* create( const QgsStringMap& properties = QgsStringMap() );

    // implemented from base classes

    QString layerType() const;

    void startRender( QgsRenderContext& context );

    void stopRender( QgsRenderContext& context );

    void renderPolyline( const QPolygonF& points, QgsRenderContext& context );

    QgsStringMap properties() const;

    QgsSymbolLayerV2* clone() const;

    void setColor( QColor color );

    QgsSymbolV2* subSymbol();
    bool setSubSymbol( QgsSymbolV2* symbol );

    // new stuff

    bool rotateMarker() const { return mRotateMarker; }
    void setRotateMarker( bool rotate ) { mRotateMarker = rotate; }

    double interval() const { return mInterval; }
    void setInterval( double interval ) { mInterval = interval; }

    double offset() const { return mOffset; }
    void setOffset( double offset ) { mOffset = offset; }

  protected:

    void renderPolylineNoOffset( const QPolygonF& points, QgsRenderContext& context );

    bool mRotateMarker;
    double mInterval;
    QgsMarkerSymbolV2* mMarker;
    double mOffset;
};

/////////

#define DEFAULT_LINEDECORATION_COLOR  QColor(0,0,0)

class CORE_EXPORT QgsLineDecorationSymbolLayerV2 : public QgsLineSymbolLayerV2
{
  public:
    QgsLineDecorationSymbolLayerV2( QColor color = DEFAULT_LINEDECORATION_COLOR );

    ~QgsLineDecorationSymbolLayerV2();

    // static stuff

    static QgsSymbolLayerV2* create( const QgsStringMap& properties = QgsStringMap() );

    // implemented from base classes

    QString layerType() const;

    void startRender( QgsRenderContext& context );

    void stopRender( QgsRenderContext& context );

    void renderPolyline( const QPolygonF& points, QgsRenderContext& context );

    QgsStringMap properties() const;

    QgsSymbolLayerV2* clone() const;

  protected:
    QPen mPen;

};

#endif
