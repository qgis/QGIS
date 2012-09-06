/***************************************************************************
    qgssymbolv2.h
    ---------------------
    begin                : November 2009
    copyright            : (C) 2009 by Martin Dobias
    email                : wonder.sk at gmail.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSSYMBOLV2_H
#define QGSSYMBOLV2_H

#include "qgis.h"
#include <QList>
#include <QMap>

class QColor;
class QImage;
class QPainter;
class QSize;
class QPointF;
class QPolygonF;

class QDomDocument;
class QDomElement;
//class

class QgsFeature;
class QgsSymbolLayerV2;
class QgsRenderContext;
class QgsVectorLayer;

typedef QMap<QString, QString> QgsStringMap;
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

    enum ScaleMethod
    {
      ScaleArea,
      ScaleDiameter
    };

    //! @note added in 1.5
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


    void startRender( QgsRenderContext& context, const QgsVectorLayer* layer = 0 );
    void stopRender( QgsRenderContext& context );

    void setColor( const QColor& color );
    QColor color();

    void drawPreviewIcon( QPainter* painter, QSize size );

    QImage bigSymbolPreviewImage();

    QString dump();

    virtual QgsSymbolV2* clone() const = 0;

    void toSld( QDomDocument &doc, QDomElement &element, QgsStringMap props ) const;

    OutputUnit outputUnit() const { return mOutputUnit; }
    void setOutputUnit( OutputUnit u ) { mOutputUnit = u; }

    //! Get alpha transparency 1 for opaque, 0 for invisible
    qreal alpha() const { return mAlpha; }
    //! Set alpha transparency 1 for opaque, 0 for invisible
    void setAlpha( qreal alpha ) { mAlpha = alpha; }

    //! @note added in 1.5
    void setRenderHints( int hints ) { mRenderHints = hints; }
    //! @note added in 1.5
    int renderHints() const { return mRenderHints; }

    QSet<QString> usedAttributes() const;

  protected:
    QgsSymbolV2( SymbolType type, QgsSymbolLayerV2List layers ); // can't be instantiated

    QgsSymbolLayerV2List cloneLayers() const;

    //! check whether a symbol layer type can be used within the symbol
    //! (marker-marker, line-line, fill-fill/line)
    //! @note added in 1.7
    bool isSymbolLayerCompatible( SymbolType t );

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
    QgsSymbolV2RenderContext( QgsRenderContext& c, QgsSymbolV2::OutputUnit u , qreal alpha = 1.0, bool selected = false, int renderHints = 0, const QgsFeature* f = 0 );
    ~QgsSymbolV2RenderContext();

    QgsRenderContext& renderContext() { return mRenderContext; }
    //void setRenderContext( QgsRenderContext& c ) { mRenderContext = c;}

    QgsSymbolV2::OutputUnit outputUnit() const { return mOutputUnit; }
    void setOutputUnit( QgsSymbolV2::OutputUnit u ) { mOutputUnit = u; }

    //! Get alpha transparency 1 for opaque, 0 for invisible
    qreal alpha() const { return mAlpha; }
    //! Set alpha transparency 1 for opaque, 0 for invisible
    void setAlpha( qreal alpha ) { mAlpha = alpha; }

    bool selected() const { return mSelected; }
    void setSelected( bool selected ) { mSelected = selected; }

    //! @note added in 1.5
    int renderHints() const { return mRenderHints; }
    //! @note added in 1.5
    void setRenderHints( int hints ) { mRenderHints = hints; }

    void setFeature( const QgsFeature* f ) { mFeature = f; }
    const QgsFeature* feature() const { return mFeature; }

    void setLayer( const QgsVectorLayer* layer ) { mLayer = layer; }
    const QgsVectorLayer* layer() const { return mLayer; }

    // Color used for selections
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
    const QgsFeature* mFeature; //current feature
    const QgsVectorLayer* mLayer; //current vectorlayer
};



//////////////////////



class CORE_EXPORT QgsMarkerSymbolV2 : public QgsSymbolV2
{
  public:
    /** Create a marker symbol with one symbol layer: SimpleMarker with specified properties.
      This is a convenience method for easier creation of marker symbols.
      \note added in v1.7
    */
    static QgsMarkerSymbolV2* createSimple( const QgsStringMap& properties );

    QgsMarkerSymbolV2( QgsSymbolLayerV2List layers = QgsSymbolLayerV2List() );

    void setAngle( double angle );
    double angle();

    void setSize( double size );
    double size();

    void setScaleMethod( QgsSymbolV2::ScaleMethod scaleMethod );
    ScaleMethod scaleMethod();

    void renderPoint( const QPointF& point, const QgsFeature* f, QgsRenderContext& context, int layer = -1, bool selected = false );

    virtual QgsSymbolV2* clone() const;
};



class CORE_EXPORT QgsLineSymbolV2 : public QgsSymbolV2
{
  public:
    /** Create a line symbol with one symbol layer: SimpleLine with specified properties.
      This is a convenience method for easier creation of line symbols.
      \note added in v1.7
    */
    static QgsLineSymbolV2* createSimple( const QgsStringMap& properties );

    QgsLineSymbolV2( QgsSymbolLayerV2List layers = QgsSymbolLayerV2List() );

    void setWidth( double width );
    double width();

    void renderPolyline( const QPolygonF& points, const QgsFeature* f, QgsRenderContext& context, int layer = -1, bool selected = false );

    virtual QgsSymbolV2* clone() const;
};



class CORE_EXPORT QgsFillSymbolV2 : public QgsSymbolV2
{
  public:
    /** Create a fill symbol with one symbol layer: SimpleFill with specified properties.
      This is a convenience method for easier creation of fill symbols.
      \note added in v1.7
    */
    static QgsFillSymbolV2* createSimple( const QgsStringMap& properties );

    QgsFillSymbolV2( QgsSymbolLayerV2List layers = QgsSymbolLayerV2List() );
    void setAngle( double angle );
    void renderPolygon( const QPolygonF& points, QList<QPolygonF>* rings, const QgsFeature* f, QgsRenderContext& context, int layer = -1, bool selected = false );

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
