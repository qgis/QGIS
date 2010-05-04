#ifndef QGSSYMBOLLAYERV2_H
#define QGSSYMBOLLAYERV2_H

#include <QMap>

#include <QColor>
#include <QPointF>

#include "qgssymbolv2.h"

#include "qgssymbollayerv2utils.h" // QgsStringMap

class QPainter;
class QSize;
class QPolygonF;

class QgsRenderContext;
class QgsSymbolV2;


class CORE_EXPORT QgsSymbolLayerV2
{
  public:

    // not necessarily supported by all symbol layers...
    virtual void setColor( const QColor& color ) { mColor = color; }
    virtual QColor color() const { return mColor; }

    virtual ~QgsSymbolLayerV2() {}

    virtual QString layerType() const = 0;

    virtual void startRender( QgsSymbolV2RenderContext& context ) = 0;
    virtual void stopRender( QgsSymbolV2RenderContext& context ) = 0;

    virtual QgsSymbolLayerV2* clone() const = 0;

    virtual QgsStringMap properties() const = 0;

    virtual void drawPreviewIcon( QgsSymbolV2RenderContext& context, QSize size ) = 0;

    virtual QgsSymbolV2* subSymbol() { return NULL; }
    // set layer's subsymbol. takes ownership of the passed symbol
    virtual bool setSubSymbol( QgsSymbolV2* symbol ) { delete symbol; return false; }

    QgsSymbolV2::SymbolType type() const { return mType; }

    void setLocked( bool locked ) { mLocked = locked; }
    bool isLocked() const { return mLocked; }

    // used only with rending with symbol levels is turned on (0 = first pass, 1 = second, ...)
    void setRenderingPass( int renderingPass ) { mRenderingPass = renderingPass; }
    int renderingPass() const { return mRenderingPass; }

  protected:
    QgsSymbolLayerV2( QgsSymbolV2::SymbolType type, bool locked = false )
        : mType( type ), mLocked( locked ), mRenderingPass( 0 ) {}

    QgsSymbolV2::SymbolType mType;
    bool mLocked;
    QColor mColor;
    int mRenderingPass;

    // Configuration of selected symbology implementation
    static const bool selectionIsOpaque = true;  // Selection ignores symbol alpha
    static const bool selectFillBorder = false;  // Fill symbol layer also selects border symbology
    static const bool selectFillStyle = false;   // Fill symbol uses symbol layer style..

};

//////////////////////

class CORE_EXPORT QgsMarkerSymbolLayerV2 : public QgsSymbolLayerV2
{
  public:
    virtual void renderPoint( const QPointF& point, QgsSymbolV2RenderContext& context ) = 0;

    void drawPreviewIcon( QgsSymbolV2RenderContext& context, QSize size );

    void setAngle( double angle ) { mAngle = angle; }
    double angle() const { return mAngle; }

    void setSize( double size ) { mSize = size; }
    double size() const { return mSize; }

    void setOffset( QPointF offset ) { mOffset = offset; }
    QPointF offset() { return mOffset; }

  protected:
    QgsMarkerSymbolLayerV2( bool locked = false );

    double mAngle;
    double mSize;
    QPointF mOffset;
};

class CORE_EXPORT QgsLineSymbolLayerV2 : public QgsSymbolLayerV2
{
  public:
    virtual void renderPolyline( const QPolygonF& points, QgsSymbolV2RenderContext& context ) = 0;

    virtual void setWidth( double width ) { mWidth = width; }
    virtual double width() const { return mWidth; }

    void drawPreviewIcon( QgsSymbolV2RenderContext& context, QSize size );

  protected:
    QgsLineSymbolLayerV2( bool locked = false );

    double mWidth;
};

class CORE_EXPORT QgsFillSymbolLayerV2 : public QgsSymbolLayerV2
{
  public:
    virtual void renderPolygon( const QPolygonF& points, QList<QPolygonF>* rings, QgsSymbolV2RenderContext& context ) = 0;

    void drawPreviewIcon( QgsSymbolV2RenderContext& context, QSize size );

  protected:
    QgsFillSymbolLayerV2( bool locked = false );
    /**Default method to render polygon*/
    void _renderPolygon( QPainter* p, const QPolygonF& points, const QList<QPolygonF>* rings );
};

class QgsSymbolLayerV2Widget;

#endif
