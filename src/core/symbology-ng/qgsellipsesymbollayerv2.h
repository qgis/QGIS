#ifndef QGSELLIPSESYMBOLLAYERV2_H
#define QGSELLIPSESYMBOLLAYERV2_H

#include "qgsmarkersymbollayerv2.h"
#include <QPainterPath>

/**A symbol layer for rendering objects with major and minor axis (e.g. ellipse, rectangle )*/
class CORE_EXPORT QgsEllipseSymbolLayerV2: public QgsMarkerSymbolLayerV2
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

    void setSymbolNameField( int index, const QString& field );
    const QPair<int, QString>& symbolNameField() const { return mSymbolNameField; }

    void setSymbolWidth( double w ){ mSymbolWidth = w; }
    double symbolWidth() const { return mSymbolWidth; }

    void setWidthField( int index, const QString& field );
    const QPair<int, QString>& widthField() const { return mWidthField; }

    void setSymbolHeight( double h ){ mSymbolHeight = h; }
    double symbolHeight() const { return mSymbolHeight; }

    void setHeightField( int index, const QString& field );
    const QPair<int, QString>& heightField() const { return mHeightField; }

    void setRotationField( int index, const QString& field );
    const QPair<int, QString>& rotationField() const { return mRotationField; }

    void setOutlineWidth( double w ){ mOutlineWidth = w; }
    double outlineWidth() const { return mOutlineWidth; }

    void setOutlineWidthField( int index, const QString& field );
    const QPair<int, QString>& outlineWidthField() const { return mOutlineWidthField; }

    void setFillColor( const QColor& c ){ mFillColor = c;}
    QColor fillColor() const { return mFillColor; }

    void setFillColorField( int index, const QString& field );
    const QPair<int, QString>& fillColorField() const { return mFillColorField; }

    void setOutlineColor( const QColor& c ){ mOutlineColor = c; }
    QColor outlineColor() const { return mOutlineColor; }

    void setOutlineColorField( int index, const QString& field );
    const QPair<int, QString>& outlineColorField() const { return mOutlineColorField; }

    QSet<QString> usedAttributes() const;

  private:
    QString mSymbolName;
    double mSymbolWidth;
    double mSymbolHeight;
    QColor mFillColor;
    QColor mOutlineColor;
    double mOutlineWidth;

    /**Take width from attribute (-1  if fixed width)*/
    QPair<int, QString> mWidthField;
    /**Take height from attribute (-1 if fixed height)*/
    QPair<int, QString> mHeightField;
    /**Take symbol rotation from attribute (-1 if fixed rotation)*/
    QPair<int, QString> mRotationField;
    /**Take outline width from attribute (-1 if fixed outline width)*/
    QPair<int, QString> mOutlineWidthField;
    /**Take fill color from attribute (-1 if fixed fill color)*/
    QPair<int, QString> mFillColorField;
    /**Take outline color from attribute (-1 if fixed outline color)*/
    QPair<int, QString> mOutlineColorField;
    /**Take shape name from attribute (-1 if fixed shape type)*/
    QPair<int, QString> mSymbolNameField;

    QPainterPath mPainterPath;

    QPen mPen;
    QBrush mBrush;

    /**Setup mPainterPath
      @param feature to render (0 if no data defined rendering)*/
    void preparePath( const QString& symbolName, QgsSymbolV2RenderContext& context, const QgsFeature* f = 0 );

    /**True if this symbol layer uses a data defined property*/
    bool hasDataDefinedProperty() const;
};

#endif // QGSELLIPSESYMBOLLAYERV2_H
