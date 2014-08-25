
typedef QList<QgsSymbolLayerV2*> QgsSymbolLayerV2List;

class QgsSymbolV2
{
%TypeHeaderCode
#include <qgssymbolv2.h>
%End

%ConvertToSubClassCode
  switch (sipCpp->type())
  {
    case QgsSymbolV2::Marker: sipClass = sipClass_QgsMarkerSymbolV2; break;
    case QgsSymbolV2::Line: sipClass = sipClass_QgsLineSymbolV2; break;
    case QgsSymbolV2::Fill: sipClass = sipClass_QgsFillSymbolV2; break;
    default: sipClass = 0; break;
  }
%End

  public:

    enum OutputUnit
    {
      MM,
      MapUnit,
      Mixed //mixed units in symbol layers
    };

    enum SymbolType
    {
      Marker,
      Line,
      Fill
    };

    enum ScaleMethod
    {
      ScaleArea,
      ScaleDiameter
    };

    //! @note added in 1.5
    enum RenderHint
    {
      DataDefinedSizeScale,
      DataDefinedRotation
    };

    virtual ~QgsSymbolV2();

    //! return new default symbol for specified geometry type
    static QgsSymbolV2* defaultSymbol( QGis::GeometryType geomType ) /Factory/;

    SymbolType type() const;

    // symbol layers handling

    QgsSymbolLayerV2* symbolLayer( int layer );

    int symbolLayerCount();

    //! insert symbol layer to specified index
    bool insertSymbolLayer( int index, QgsSymbolLayerV2* layer /Transfer/ );

    //! append symbol layer at the end of the list
    bool appendSymbolLayer( QgsSymbolLayerV2* layer /Transfer/ );

    //! delete symbol layer at specified index
    bool deleteSymbolLayer( int index );

    //! remove symbol layer from the list and return pointer to it
    QgsSymbolLayerV2* takeSymbolLayer( int index ) /TransferBack/;

    //! delete layer at specified index and set a new one
    bool changeSymbolLayer( int index, QgsSymbolLayerV2 *layer /Transfer/ );

    void startRender( QgsRenderContext& context, const QgsFields* fields = 0 );
    void stopRender( QgsRenderContext& context );

    void setColor( const QColor& color );
    QColor color() const;

    //! Draw icon of the symbol that occupyies area given by size using the painter.
    //! Optionally custom context may be given in order to get rendering of symbols that use map units right.
    //! @note customContext parameter added in 2.6
    void drawPreviewIcon( QPainter* painter, QSize size, QgsRenderContext* customContext = 0 );

    QImage bigSymbolPreviewImage();

    QString dump() const;

    virtual QgsSymbolV2* clone() const = 0 /Factory/;

    void toSld( QDomDocument &doc, QDomElement &element, QgsStringMap props ) const;

    OutputUnit outputUnit() const;
    void setOutputUnit( OutputUnit u );

    QgsMapUnitScale mapUnitScale() const;
    void setMapUnitScale( const QgsMapUnitScale& scale );

    //! Get alpha transparency 1 for opaque, 0 for invisible
    qreal alpha() const;
    //! Set alpha transparency 1 for opaque, 0 for invisible
    void setAlpha( qreal alpha );

    //! @note added in 1.5
    void setRenderHints( int hints );
    //! @note added in 1.5
    int renderHints() const;

    QSet<QString> usedAttributes() const;

    void setLayer( const QgsVectorLayer* layer );
    const QgsVectorLayer* layer() const;

  protected:
    QgsSymbolV2( SymbolType type, QgsSymbolLayerV2List layers /Transfer/ ); // can't be instantiated

    QgsSymbolLayerV2List cloneLayers() const /Factory/;

    //! check whether a symbol layer type can be used within the symbol
    //! (marker-marker, line-line, fill-fill/line)
    //! @note added in 1.7
    bool isSymbolLayerCompatible( SymbolType t );
};

///////////////////////

class QgsSymbolV2RenderContext
{
%TypeHeaderCode
#include <qgssymbolv2.h>
%End

  public:
    QgsSymbolV2RenderContext( QgsRenderContext& c, QgsSymbolV2::OutputUnit u , qreal alpha = 1.0, bool selected = false, int renderHints = 0, const QgsFeature* f = 0, const QgsFields* fields = 0, const QgsMapUnitScale& mapUnitScale = QgsMapUnitScale() );
    ~QgsSymbolV2RenderContext();

    QgsRenderContext& renderContext();
    //void setRenderContext( QgsRenderContext& c );

    QgsSymbolV2::OutputUnit outputUnit() const;
    void setOutputUnit( QgsSymbolV2::OutputUnit u );

    QgsMapUnitScale mapUnitScale() const;
    void setMapUnitScale( const QgsMapUnitScale& scale);

    //! Get alpha transparency 1 for opaque, 0 for invisible
    qreal alpha() const;
    //! Set alpha transparency 1 for opaque, 0 for invisible
    void setAlpha( qreal alpha );

    bool selected() const;
    void setSelected( bool selected );

    //! @note added in 1.5
    int renderHints() const;
    //! @note added in 1.5
    void setRenderHints( int hints );

    void setFeature( const QgsFeature* f );
    //! Current feature being rendered - may be null
    const QgsFeature* feature() const;

    //! Fields of the layer. Currently only available in startRender() calls
    //! to allow symbols with data-defined properties prepare the expressions
    //! (other times fields() returns null)
    //! @note added in 2.4
    const QgsFields* fields() const;

    double outputLineWidth( double width ) const;
    double outputPixelSize( double size ) const;

    // workaround for sip 4.7. Don't use assignment - will fail with assertion error
    // QgsSymbolV2RenderContext& operator=( const QgsSymbolV2RenderContext& );
};



//////////////////////



class QgsMarkerSymbolV2 : QgsSymbolV2
{
%TypeHeaderCode
#include <qgssymbolv2.h>
%End

  public:
    /** Create a marker symbol with one symbol layer: SimpleMarker with specified properties.
      This is a convenience method for easier creation of marker symbols.
      \note added in v1.7
    */
    static QgsMarkerSymbolV2* createSimple( const QgsStringMap& properties ) /Factory/;

    QgsMarkerSymbolV2( QgsSymbolLayerV2List layers /Transfer/ = QgsSymbolLayerV2List() );

    void setAngle( double angle );
    double angle();

    void setSize( double size );
    double size();

    void setScaleMethod( QgsSymbolV2::ScaleMethod scaleMethod );
    ScaleMethod scaleMethod();

    void renderPoint( const QPointF& point, const QgsFeature* f, QgsRenderContext& context, int layer = -1, bool selected = false );

    virtual QgsSymbolV2* clone() const /Factory/;
};



class QgsLineSymbolV2 : QgsSymbolV2
{
%TypeHeaderCode
#include <qgssymbolv2.h>
%End

  public:
    /** Create a line symbol with one symbol layer: SimpleLine with specified properties.
      This is a convenience method for easier creation of line symbols.
      \note added in v1.7
    */
    static QgsLineSymbolV2* createSimple( const QgsStringMap& properties ) /Factory/;

    QgsLineSymbolV2( QgsSymbolLayerV2List layers /Transfer/ = QgsSymbolLayerV2List() );

    void setWidth( double width );
    double width();

    void renderPolyline( const QPolygonF& points, const QgsFeature* f, QgsRenderContext& context, int layer = -1, bool selected = false );

    virtual QgsSymbolV2* clone() const /Factory/;
};



class QgsFillSymbolV2 : QgsSymbolV2
{
%TypeHeaderCode
#include <qgssymbolv2.h>
%End

  public:
    /** Create a fill symbol with one symbol layer: SimpleFill with specified properties.
      This is a convenience method for easier creation of fill symbols.
      \note added in v1.7
    */
    static QgsFillSymbolV2* createSimple( const QgsStringMap& properties ) /Factory/;

    QgsFillSymbolV2( QgsSymbolLayerV2List layers /Transfer/ = QgsSymbolLayerV2List() );

    void setAngle( double angle );
    void renderPolygon( const QPolygonF& points, QList<QPolygonF>* rings, const QgsFeature* f, QgsRenderContext& context, int layer = -1, bool selected = false );

    virtual QgsSymbolV2* clone() const /Factory/;
};
