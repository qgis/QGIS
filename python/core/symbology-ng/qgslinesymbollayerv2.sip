class QgsSimpleLineSymbolLayerV2 : QgsLineSymbolLayerV2
{
%TypeHeaderCode
#include <qgslinesymbollayerv2.h>
%End

  public:
    QgsSimpleLineSymbolLayerV2( QColor color = DEFAULT_SIMPLELINE_COLOR,
                                double width = DEFAULT_SIMPLELINE_WIDTH,
                                Qt::PenStyle penStyle = DEFAULT_SIMPLELINE_PENSTYLE );

    // static stuff

    static QgsSymbolLayerV2* create( const QgsStringMap& properties = QgsStringMap() ) /Factory/;
    static QgsSymbolLayerV2* createFromSld( QDomElement &element ) /Factory/;

    // implemented from base classes

    QString layerType() const;

    void startRender( QgsSymbolV2RenderContext& context );

    void stopRender( QgsSymbolV2RenderContext& context );

    void renderPolyline( const QPolygonF& points, QgsSymbolV2RenderContext& context );

    QgsStringMap properties() const;

    QgsSymbolLayerV2* clone() const /Factory/;

    void toSld( QDomDocument &doc, QDomElement &element, QgsStringMap props ) const;

    // new stuff

    Qt::PenStyle penStyle() const;
    void setPenStyle( Qt::PenStyle style );

    Qt::PenJoinStyle penJoinStyle() const;
    void setPenJoinStyle( Qt::PenJoinStyle style );

    Qt::PenCapStyle penCapStyle() const;
    void setPenCapStyle( Qt::PenCapStyle style );

    double offset() const;
    void setOffset( double offset );

    QgsSymbolV2::OutputUnit offsetUnit() const;
    void setOffsetUnit( QgsSymbolV2::OutputUnit unit );

    bool useCustomDashPattern() const;
    void setUseCustomDashPattern( bool b );

    QgsSymbolV2::OutputUnit customDashPatternUnit() const;
    void setCustomDashPatternUnit( QgsSymbolV2::OutputUnit unit );

    QVector<qreal> customDashVector() const;
    void setCustomDashVector( const QVector<qreal>& vector );
};

/////////

class QgsMarkerLineSymbolLayerV2 : QgsLineSymbolLayerV2
{
%TypeHeaderCode
#include <qgslinesymbollayerv2.h>
%End

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

    static QgsSymbolLayerV2* create( const QgsStringMap& properties = QgsStringMap() ) /Factory/;
    static QgsSymbolLayerV2* createFromSld( QDomElement &element ) /Factory/;

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

    bool rotateMarker() const;
    void setRotateMarker( bool rotate );

    double interval() const;
    void setInterval( double interval );

    double offset() const;
    void setOffset( double offset );

    QgsSymbolV2::OutputUnit intervalUnit() const;
    void setIntervalUnit( QgsSymbolV2::OutputUnit unit );

    QgsSymbolV2::OutputUnit offsetUnit() const;
    void setOffsetUnit( QgsSymbolV2::OutputUnit unit );

    void setOutputUnit( QgsSymbolV2::OutputUnit unit );
    QgsSymbolV2::OutputUnit outputUnit() const;

    Placement placement() const;
    void setPlacement( Placement p );

  protected:

    void renderPolylineInterval( const QPolygonF& points, QgsSymbolV2RenderContext& context );
    void renderPolylineVertex( const QPolygonF& points, QgsSymbolV2RenderContext& context );
    void renderPolylineCentral( const QPolygonF& points, QgsSymbolV2RenderContext& context );
};

/////////

class QgsLineDecorationSymbolLayerV2 : QgsLineSymbolLayerV2
{
%TypeHeaderCode
#include <qgslinesymbollayerv2.h>
%End

  public:
    QgsLineDecorationSymbolLayerV2( QColor color = DEFAULT_LINEDECORATION_COLOR,
                                    double width = DEFAULT_LINEDECORATION_WIDTH );

    ~QgsLineDecorationSymbolLayerV2();

    // static stuff

    static QgsSymbolLayerV2* create( const QgsStringMap& properties = QgsStringMap() ) /Factory/;

    // implemented from base classes

    QString layerType() const;

    void startRender( QgsSymbolV2RenderContext& context );

    void stopRender( QgsSymbolV2RenderContext& context );

    void renderPolyline( const QPolygonF& points, QgsSymbolV2RenderContext& context );

    QgsStringMap properties() const;

    QgsSymbolLayerV2* clone() const /Factory/;

    void toSld( QDomDocument &doc, QDomElement &element, QgsStringMap props ) const;

    void setOutputUnit( QgsSymbolV2::OutputUnit unit );
    QgsSymbolV2::OutputUnit outputUnit() const;
};
