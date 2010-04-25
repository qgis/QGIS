
#ifndef QGSSYMBOLV2_H
#define QGSSYMBOLV2_H

#include "qgis.h"
#include <QList>

class QColor;
class QImage;
class QPainter;
class QSize;
class QPointF;
class QPolygonF;
//class

class QgsSymbolLayerV2;
class QgsRenderContext;

typedef QList<QgsSymbolLayerV2*> QgsSymbolLayerV2List;

class CORE_EXPORT QgsSymbolV2
{
  public:

    enum OutputUnit
    {
      MM,
      MapUnit
    };

    enum SymbolType
    {
      Marker,
      Line,
      Fill
    };

    enum RenderHint
    {
      DataDefinedSizeScale = 1,
      DataDefinedRotation = 2
    };

    virtual ~QgsSymbolV2();

    //! return new default symbol for specified geometry type
    static QgsSymbolV2* defaultSymbol( QGis::GeometryType geomType );

    SymbolType type() const { return mType; }

    // symbol layers handling

    QgsSymbolLayerV2* symbolLayer( int layer );

    int symbolLayerCount() { return mLayers.count(); }

    //! insert symbol layer to specified index
    bool insertSymbolLayer( int index, QgsSymbolLayerV2* layer );

    //! append symbol layer at the end of the list
    bool appendSymbolLayer( QgsSymbolLayerV2* layer );

    //! delete symbol layer at specified index
    bool deleteSymbolLayer( int index );

    //! remove symbol layer from the list and return pointer to it
    QgsSymbolLayerV2* takeSymbolLayer( int index );

    //! delete layer at specified index and set a new one
    bool changeSymbolLayer( int index, QgsSymbolLayerV2* layer );


    void startRender( QgsRenderContext& context );
    void stopRender( QgsRenderContext& context );

    void setColor( const QColor& color );
    QColor color();

    void drawPreviewIcon( QPainter* painter, QSize size );

    QImage bigSymbolPreviewImage();

    QString dump();

    virtual QgsSymbolV2* clone() const = 0;

    OutputUnit outputUnit() const { return mOutputUnit; }
    void setOutputUnit( OutputUnit u ) { mOutputUnit = u; }

    qreal alpha() const { return mAlpha; }
    void setAlpha( qreal alpha ) { mAlpha = alpha; }

    void setRenderHints( int hints ) { mRenderHints = hints; }
    int renderHints() { return mRenderHints; }

  protected:
    QgsSymbolV2( SymbolType type, QgsSymbolLayerV2List layers ); // can't be instantiated

    QgsSymbolLayerV2List cloneLayers() const;

    SymbolType mType;
    QgsSymbolLayerV2List mLayers;

    OutputUnit mOutputUnit;

    /**Symbol opacity (in the range 0 - 1)*/
    qreal mAlpha;

    int mRenderHints;
};

///////////////////////

class CORE_EXPORT QgsSymbolV2RenderContext
{
  public:
    QgsSymbolV2RenderContext( QgsRenderContext& c, QgsSymbolV2::OutputUnit u , qreal alpha = 1.0, bool selected = false, int renderHints = 0 );
    ~QgsSymbolV2RenderContext();

    QgsRenderContext& renderContext() { return mRenderContext; }
    //void setRenderContext( QgsRenderContext& c ) { mRenderContext = c;}

    QgsSymbolV2::OutputUnit outputUnit() const { return mOutputUnit; }
    void setOutputUnit( QgsSymbolV2::OutputUnit u ) { mOutputUnit = u; }

    qreal alpha() const { return mAlpha; }
    void setAlpha( qreal alpha ) { mAlpha = alpha; }

    bool selected() const { return mSelected; }
    void setSelected( bool selected ) { mSelected = selected; }

    int renderHints() const { return mRenderHints; }
    void setRenderHints( int hints ) { mRenderHints = hints; }

    // Colour used for selections

    static QColor selectionColor();

    double outputLineWidth( double width ) const;
    double outputPixelSize( double size ) const;

    // workaround for sip 4.7. Don't use assignment - will fail with assertion error
    QgsSymbolV2RenderContext& operator=( const QgsSymbolV2RenderContext& );

  private:
    QgsRenderContext& mRenderContext;
    QgsSymbolV2::OutputUnit mOutputUnit;
    qreal mAlpha;
    bool mSelected;
    int mRenderHints;
};



//////////////////////



class CORE_EXPORT QgsMarkerSymbolV2 : public QgsSymbolV2
{
  public:
    QgsMarkerSymbolV2( QgsSymbolLayerV2List layers = QgsSymbolLayerV2List() );

    void setAngle( double angle );
    double angle();

    void setSize( double size );
    double size();

    void renderPoint( const QPointF& point, QgsRenderContext& context, int layer = -1, bool selected = false );

    virtual QgsSymbolV2* clone() const;
};



class CORE_EXPORT QgsLineSymbolV2 : public QgsSymbolV2
{
  public:
    QgsLineSymbolV2( QgsSymbolLayerV2List layers = QgsSymbolLayerV2List() );

    void setWidth( double width );
    double width();

    void renderPolyline( const QPolygonF& points, QgsRenderContext& context, int layer = -1, bool selected = false );

    virtual QgsSymbolV2* clone() const;
};



class CORE_EXPORT QgsFillSymbolV2 : public QgsSymbolV2
{
  public:
    QgsFillSymbolV2( QgsSymbolLayerV2List layers = QgsSymbolLayerV2List() );

    void renderPolygon( const QPolygonF& points, QList<QPolygonF>* rings, QgsRenderContext& context, int layer = -1, bool selected = false );

    virtual QgsSymbolV2* clone() const;
};

#endif


/*

QgsSymbolV2* ps = new QgsPointSymbol();

// ----

sl = QgsSymbolLayerV2Registry::instance()->createSymbolLayer("SimpleLine", { "color", "..." })

// (or)

sl = QgsSymbolLayerV2Registry::defaultSymbolLayer(QgsSymbolV2::Line)

// (or)

QgsSymbolLayerV2* sl = new QgsSimpleLineSymbolLayer(x,y,z);
QgsLineSymbol* s = new LineSymbol( [ sl ] );

// ----

rend = QgsSingleSymbolRenderer( new LineSymbol() );
*/
