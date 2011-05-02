
#ifndef QGSRENDERERV2_H
#define QGSRENDERERV2_H

#include "qgis.h"

#include <QList>
#include <QString>
#include <QVariant>
#include <QPair>
#include <QPixmap>

class QDomDocument;
class QDomElement;

class QgsSymbolV2;
class QgsRenderContext;
class QgsFeature;
class QgsVectorLayer;

typedef QList<QgsSymbolV2*> QgsSymbolV2List;
typedef QMap<QString, QgsSymbolV2* > QgsSymbolV2Map;

typedef QList< QPair<QString, QPixmap> > QgsLegendSymbologyList;
typedef QList< QPair<QString, QgsSymbolV2*> > QgsLegendSymbolList;

#define RENDERER_TAG_NAME   "renderer-v2"

////////
// symbol levels

class CORE_EXPORT QgsSymbolV2LevelItem
{
  public:
    QgsSymbolV2LevelItem( QgsSymbolV2* symbol, int layer ) : mSymbol( symbol ), mLayer( layer ) {}
    QgsSymbolV2* symbol() { return mSymbol; }
    int layer() { return mLayer; }
  protected:
    QgsSymbolV2* mSymbol;
    int mLayer;
};

// every level has list of items: symbol + symbol layer num
typedef QList< QgsSymbolV2LevelItem > QgsSymbolV2Level;

// this is a list of levels
typedef QList< QgsSymbolV2Level > QgsSymbolV2LevelOrder;


//////////////
// renderers

class CORE_EXPORT QgsFeatureRendererV2
{
  public:
    // renderer takes ownership of its symbols!

    //! return a new renderer - used by default in vector layers
    static QgsFeatureRendererV2* defaultRenderer( QGis::GeometryType geomType );

    QString type() const { return mType; }

    // to be overridden
    virtual QgsSymbolV2* symbolForFeature( QgsFeature& feature ) = 0;

    virtual void startRender( QgsRenderContext& context, const QgsVectorLayer *vlayer ) = 0;

    virtual void stopRender( QgsRenderContext& context ) = 0;

    virtual QList<QString> usedAttributes() = 0;

    virtual ~QgsFeatureRendererV2() {}

    virtual QgsFeatureRendererV2* clone() = 0;

    virtual void renderFeature( QgsFeature& feature, QgsRenderContext& context, int layer = -1, bool selected = false, bool drawVertexMarker = false );

    //! for debugging
    virtual QString dump();

    //! for symbol levels
    virtual QgsSymbolV2List symbols() = 0;

    bool usingSymbolLevels() const { return mUsingSymbolLevels; }
    void setUsingSymbolLevels( bool usingSymbolLevels ) { mUsingSymbolLevels = usingSymbolLevels; }

    //! create a renderer from XML element
    static QgsFeatureRendererV2* load( QDomElement& symbologyElem );

    //! store renderer info to XML element
    virtual QDomElement save( QDomDocument& doc );

    //! return a list of symbology items for the legend
    virtual QgsLegendSymbologyList legendSymbologyItems( QSize iconSize );

    //! return a list of item text / symbol
    //! @note: this method was added in version 1.5
    virtual QgsLegendSymbolList legendSymbolItems();

    //! set type and size of editing vertex markers for subsequent rendering
    void setVertexMarkerAppearance( int type, int size );

  protected:
    QgsFeatureRendererV2( QString type );

    //! render editing vertex marker at specified point
    void renderVertexMarker( QPointF& pt, QgsRenderContext& context );
    //! render editing vertex marker for a polyline
    void renderVertexMarkerPolyline( QPolygonF& pts, QgsRenderContext& context );
    //! render editing vertex marker for a polygon
    void renderVertexMarkerPolygon( QPolygonF& pts, QList<QPolygonF>* rings, QgsRenderContext& context );

    static unsigned char* _getPoint( QPointF& pt, QgsRenderContext& context, unsigned char* wkb );
    static unsigned char* _getLineString( QPolygonF& pts, QgsRenderContext& context, unsigned char* wkb );
    static unsigned char* _getPolygon( QPolygonF& pts, QList<QPolygonF>& holes, QgsRenderContext& context, unsigned char* wkb );

    QString mType;

    bool mUsingSymbolLevels;

    /** The current type of editing marker */
    int mCurrentVertexMarkerType;
    /** The current size of editing marker */
    int mCurrentVertexMarkerSize;
};


#endif
