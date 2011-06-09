#ifndef QGSELLIPSESYMBOLLAYERV2_H
#define QGSELLIPSESYMBOLLAYERV2_H

#include "qgsmarkersymbollayerv2.h"
#include <QPainterPath>

/**A symbol layer for rendering objects with major and minor axis (e.g. ellipse, rectangle )*/
class QgsEllipseSymbolLayerV2: public QgsMarkerSymbolLayerV2
{
  public:
    QgsEllipseSymbolLayerV2();
    ~QgsEllipseSymbolLayerV2();

    static QgsSymbolLayerV2* create( const QgsStringMap& properties = QgsStringMap() );

    void renderPoint( const QPointF& point, QgsSymbolV2RenderContext& context );
    QString layerType() const;
    void startRender( QgsSymbolV2RenderContext& context );
    void stopRender( QgsSymbolV2RenderContext& context );
    QgsSymbolLayerV2* clone() const;
    QgsStringMap properties() const;

    void setSymbolName( const QString& name ){ mSymbolName = name; }
    QString symbolName() const{ return mSymbolName; }

    void setSymbolWidth( double w ){ mSymbolWidth = w; }
    double symbolWidth() const { return mSymbolWidth; }

    void setDataDefinedWidth( int c ){ mDataDefinedWidth = c; }
    int dataDefinedWidth() const { return mDataDefinedWidth; }

    void setSymbolHeight( double h ){ mSymbolHeight = h; }
    double symbolHeight() const { return mSymbolHeight; }

    void setDataDefinedHeight( int c ){ mDataDefinedHeight = c; }
    int dataDefinedHeight() const { return mDataDefinedHeight; }

    void setOutlineWidth( double w ){ mOutlineWidth = w; }
    double outlineWidth() const { return mOutlineWidth; }

    void setDataDefinedOutlineWidth( int c ){ mDataDefinedOutlineWidth = c; }
    int dataDefinedOutlineWidth() const { return mDataDefinedOutlineWidth; }

    void setFillColor( const QColor& c ){ mFillColor = c;}
    QColor fillColor() const { return mFillColor; }

    void setDataDefinedFillColor( int c ){ mDataDefinedFillColor = c; }
    int dataDefinedFillColor() const { return mDataDefinedFillColor; }

    void setOutlineColor( const QColor& c ){ mOutlineColor = c; }
    QColor outlineColor() const { return mOutlineColor; }

    void setDataDefinedOutlineColor( int c ){ mDataDefinedOutlineColor = c; }
    int dataDefinedOutlineColor() const { return mDataDefinedOutlineColor; }

    QSet<QString> usedAttributes() const;

  private:
    QString mSymbolName;
    double mSymbolWidth;
    /**Take width from attribute (-1  if fixed width)*/
    int mDataDefinedWidth;
    double mSymbolHeight;
    /**Take height from attribute (-1 if fixed height)*/
    int mDataDefinedHeight;
    double mOutlineWidth;
    /**Take outline width from attribute (-1 if fixed outline width)*/
    int mDataDefinedOutlineWidth;
    QColor mFillColor;
    /**Take fill color from attribute (-1 if fixed fill color)*/
    int mDataDefinedFillColor;
    QColor mOutlineColor;
    /**Take outline color from attribute (-1 if fixed outline color)*/
    int mDataDefinedOutlineColor;
    QPainterPath mPainterPath;

    QPen mPen;
    QBrush mBrush;

    /**Setup mPainterPath
      @param feature to render (0 if no data defined rendering)*/
    void preparePath( QgsSymbolV2RenderContext& context, const QgsFeature* f = 0 );

    /**True if this symbol layer uses a data defined property*/
    bool hasDataDefinedProperty() const;
};

#endif // QGSELLIPSESYMBOLLAYERV2_H
