class QgsEllipseSymbolLayerV2 : QgsMarkerSymbolLayerV2
{
%TypeHeaderCode
#include <qgsellipsesymbollayerv2.h>
%End

  public:
    QgsEllipseSymbolLayerV2();
    ~QgsEllipseSymbolLayerV2();

    static QgsSymbolLayerV2* create( const QgsStringMap& properties = QgsStringMap() ) /Factory/;
    static QgsSymbolLayerV2* createFromSld( QDomElement &element ) /Factory/;

    void renderPoint( const QPointF& point, QgsSymbolV2RenderContext& context );
    QString layerType() const;
    void startRender( QgsSymbolV2RenderContext& context );
    void stopRender( QgsSymbolV2RenderContext& context );
    QgsSymbolLayerV2* clone() const /Factory/;
    QgsStringMap properties() const;

    void toSld( QDomDocument& doc, QDomElement &element, QgsStringMap props ) const;
    void writeSldMarker( QDomDocument& doc, QDomElement &element, QgsStringMap props ) const;

    void setSymbolName( const QString& name );
    QString symbolName() const;

    void setSymbolNameField( const QString& field );
    const QString& symbolNameField() const;

    void setSymbolWidth( double w );
    double symbolWidth() const;

    void setWidthField( const QString& field );
    const QString& widthField() const;

    void setSymbolHeight( double h );
    double symbolHeight() const;

    void setHeightField( const QString& field );
    const QString& heightField() const;

    void setRotationField( const QString& field );
    const QString& rotationField() const;

    void setOutlineWidth( double w );
    double outlineWidth() const;

    void setOutlineWidthField( const QString& field );
    const QString& outlineWidthField() const;

    void setFillColor( const QColor& c );
    QColor fillColor() const;

    void setFillColorField( const QString& field );
    const QString& fillColorField() const;

    void setOutlineColor( const QColor& c );
    QColor outlineColor() const;

    void setOutlineColorField( const QString& field );
    const QString& outlineColorField() const;

    QSet<QString> usedAttributes() const;

};
