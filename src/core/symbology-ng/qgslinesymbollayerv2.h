
#ifndef QGSLINESYMBOLLAYERV2_H
#define QGSLINESYMBOLLAYERV2_H

#include "qgssymbollayerv2.h"

#include <QPen>
#include <QVector>

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

    // implemented from base classes

    QString layerType() const;

    void startRender( QgsSymbolV2RenderContext& context );

    void stopRender( QgsSymbolV2RenderContext& context );

    void renderPolyline( const QPolygonF& points, QgsSymbolV2RenderContext& context );

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

    bool useCustomDashPattern() const { return mUseCustomDashPattern; }
    void setUseCustomDashPattern( bool b ) { mUseCustomDashPattern = b; }

    QVector<qreal> customDashVector() const { return mCustomDashVector; }
    void setCustomDashVector( const QVector<qreal>& vector ) { mCustomDashVector = vector; }

  protected:
    Qt::PenStyle mPenStyle;
    Qt::PenJoinStyle mPenJoinStyle;
    Qt::PenCapStyle mPenCapStyle;
    QPen mPen;
    QPen mSelPen;
    double mOffset;
    //use a custom dash dot pattern instead of the predefined ones
    bool mUseCustomDashPattern;
    /**Vector with an even number of entries for the */
    QVector<qreal> mCustomDashVector;
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
      Vertex
    };

    // static stuff

    static QgsSymbolLayerV2* create( const QgsStringMap& properties = QgsStringMap() );

    // implemented from base classes

    QString layerType() const;

    void startRender( QgsSymbolV2RenderContext& context );

    void stopRender( QgsSymbolV2RenderContext& context );

    void renderPolyline( const QPolygonF& points, QgsSymbolV2RenderContext& context );

    QgsStringMap properties() const;

    QgsSymbolLayerV2* clone() const;

    void setColor( QColor color );

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

  protected:

    void renderPolylineInterval( const QPolygonF& points, QgsSymbolV2RenderContext& context );
    void renderPolylineVertex( const QPolygonF& points, QgsSymbolV2RenderContext& context );

    bool mRotateMarker;
    double mInterval;
    QgsMarkerSymbolV2* mMarker;
    double mOffset;
    Placement mPlacement;
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

    void startRender( QgsSymbolV2RenderContext& context );

    void stopRender( QgsSymbolV2RenderContext& context );

    void renderPolyline( const QPolygonF& points, QgsSymbolV2RenderContext& context );

    QgsStringMap properties() const;

    QgsSymbolLayerV2* clone() const;

  protected:
    QPen mPen;
    QPen mSelPen;

};

#endif
