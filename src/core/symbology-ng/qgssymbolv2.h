
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

    enum SymbolType
    {
      Marker,
      Line,
      Fill
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

  protected:
    QgsSymbolV2( SymbolType type, QgsSymbolLayerV2List layers ); // can't be instantiated

    QgsSymbolLayerV2List cloneLayers() const;

    SymbolType mType;
    QgsSymbolLayerV2List mLayers;
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

    void renderPoint( const QPointF& point, QgsRenderContext& context, int layer = -1 );

    virtual QgsSymbolV2* clone() const;
};



class CORE_EXPORT QgsLineSymbolV2 : public QgsSymbolV2
{
  public:
    QgsLineSymbolV2( QgsSymbolLayerV2List layers = QgsSymbolLayerV2List() );

    void setWidth( double width );
    double width();

    void renderPolyline( const QPolygonF& points, QgsRenderContext& context, int layer = -1 );

    virtual QgsSymbolV2* clone() const;
};



class CORE_EXPORT QgsFillSymbolV2 : public QgsSymbolV2
{
  public:
    QgsFillSymbolV2( QgsSymbolLayerV2List layers = QgsSymbolLayerV2List() );

    void renderPolygon( const QPolygonF& points, QList<QPolygonF>* rings, QgsRenderContext& context, int layer = -1 );

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
