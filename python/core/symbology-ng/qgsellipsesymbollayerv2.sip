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

    void setSymbolWidth( double w );
    double symbolWidth() const;

    void setSymbolHeight( double h );
    double symbolHeight() const;

    void setOutlineWidth( double w );
    double outlineWidth() const;

    void setFillColor( const QColor& c );
    QColor fillColor() const;

    void setOutlineColor( const QColor& c );
    QColor outlineColor() const;

    void setSymbolWidthUnit( QgsSymbolV2::OutputUnit unit );
    QgsSymbolV2::OutputUnit symbolWidthUnit() const;

    void setSymbolHeightUnit( QgsSymbolV2::OutputUnit unit );
    QgsSymbolV2::OutputUnit symbolHeightUnit() const;

    void setOutlineWidthUnit( QgsSymbolV2::OutputUnit unit );
    QgsSymbolV2::OutputUnit outlineWidthUnit() const;

    void setOutputUnit( QgsSymbolV2::OutputUnit unit );
    QgsSymbolV2::OutputUnit outputUnit() const;

    QSet<QString> usedAttributes() const;

};
