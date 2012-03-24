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
    static QgsSymbolLayerV2* createFromSld( QDomElement &element );

    void renderPoint( const QPointF& point, QgsSymbolV2RenderContext& context );
    QString layerType() const;
    void startRender( QgsSymbolV2RenderContext& context );
    void stopRender( QgsSymbolV2RenderContext& context );
    QgsSymbolLayerV2* clone() const;
    QgsStringMap properties() const;

    void toSld( QDomDocument& doc, QDomElement &element, QgsStringMap props ) const;
    void writeSldMarker( QDomDocument& doc, QDomElement &element, QgsStringMap props ) const;

    void setSymbolName( const QString& name ) { mSymbolName = name; }
    QString symbolName() const { return mSymbolName; }

    void setSymbolNameField( const QString& field ) { mSymbolNameField = field; }
    const QString& symbolNameField() const { return mSymbolNameField; }

    void setSymbolWidth( double w ) { mSymbolWidth = w; }
    double symbolWidth() const { return mSymbolWidth; }

    void setWidthField( const QString& field ) { mWidthField = field; }
    const QString& widthField() const { return mWidthField; }

    void setSymbolHeight( double h ) { mSymbolHeight = h; }
    double symbolHeight() const { return mSymbolHeight; }

    void setHeightField( const QString& field ) { mHeightField = field; }
    const QString& heightField() const { return mHeightField; }

    void setRotationField( const QString& field ) { mRotationField = field; }
    const QString& rotationField() const { return mRotationField; }

    void setOutlineWidth( double w ) { mOutlineWidth = w; }
    double outlineWidth() const { return mOutlineWidth; }

    void setOutlineWidthField( const QString& field ) { mOutlineWidthField = field; }
    const QString& outlineWidthField() const { return mOutlineWidthField; }

    void setFillColor( const QColor& c ) { mFillColor = c;}
    QColor fillColor() const { return mFillColor; }

    void setFillColorField( const QString& field ) { mFillColorField = field; }
    const QString& fillColorField() const { return mFillColorField; }

    void setOutlineColor( const QColor& c ) { mOutlineColor = c; }
    QColor outlineColor() const { return mOutlineColor; }

    void setOutlineColorField( const QString& field ) { mOutlineColorField = field; }
    const QString& outlineColorField() const { return mOutlineColorField; }

    QSet<QString> usedAttributes() const;

  private:
    QString mSymbolName;
    double mSymbolWidth;
    double mSymbolHeight;
    QColor mFillColor;
    QColor mOutlineColor;
    double mOutlineWidth;

#if 0
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
#endif //0

    //data defined property fields
    QString mWidthField;
    QString mHeightField;
    QString mRotationField;
    QString mOutlineWidthField;
    QString mFillColorField;
    QString mOutlineColorField;
    QString mSymbolNameField;

    //field indices for data defined properties
    //resolved in startRender method
    int mWidthIndex;
    int mHeightIndex;
    int mRotationIndex;
    int mOutlineWidthIndex;
    int mFillColorIndex;
    int mOutlineColorIndex;
    int mSymbolNameIndex;

    QPainterPath mPainterPath;

    QPen mPen;
    QBrush mBrush;

    /**Setup mPainterPath
      @param symbolName name of symbol
      @param context render context
      @param f feature f to render (0 if no data defined rendering)*/
    void preparePath( const QString& symbolName, QgsSymbolV2RenderContext& context, const QgsFeature* f = 0 );

    /**True if this symbol layer uses a data defined property*/
    bool hasDataDefinedProperty() const;
};

#endif // QGSELLIPSESYMBOLLAYERV2_H
