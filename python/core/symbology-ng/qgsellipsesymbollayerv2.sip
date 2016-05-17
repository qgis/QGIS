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

    void renderPoint( QPointF point, QgsSymbolV2RenderContext& context );
    QString layerType() const;
    void startRender( QgsSymbolV2RenderContext& context );
    void stopRender( QgsSymbolV2RenderContext& context );
    virtual QgsEllipseSymbolLayerV2* clone() const /Factory/;
    QgsStringMap properties() const;

    void toSld( QDomDocument& doc, QDomElement &element, const QgsStringMap& props ) const;
    void writeSldMarker( QDomDocument& doc, QDomElement &element, const QgsStringMap& props ) const;

    bool writeDxf( QgsDxfExport &e, double mmMapUnitScaleFactor, const QString &layerName, QgsSymbolV2RenderContext &context, QPointF shift = QPointF( 0.0, 0.0 ) ) const;

    void setSymbolName( const QString& name );
    QString symbolName() const;

    void setSymbolWidth( double w );
    double symbolWidth() const;

    void setSymbolHeight( double h );
    double symbolHeight() const;

    Qt::PenStyle outlineStyle() const;
    void setOutlineStyle( Qt::PenStyle outlineStyle );

    /** Get outline join style.
     * @note added in 2.16 */
    Qt::PenJoinStyle penJoinStyle() const;
    /** Set outline join style.
     * @note added in 2.16 */
    void setPenJoinStyle( Qt::PenJoinStyle style );

    void setOutlineWidth( double w );
    double outlineWidth() const;

    void setFillColor( const QColor& c );
    QColor fillColor() const;

    void setOutlineColor( const QColor& c );
    QColor outlineColor() const;

    void setSymbolWidthUnit( QgsSymbolV2::OutputUnit unit );
    QgsSymbolV2::OutputUnit symbolWidthUnit() const;

    void setSymbolWidthMapUnitScale( const QgsMapUnitScale& scale);
    const QgsMapUnitScale& symbolWidthMapUnitScale() const;

    void setSymbolHeightUnit( QgsSymbolV2::OutputUnit unit );
    QgsSymbolV2::OutputUnit symbolHeightUnit() const;

    void setSymbolHeightMapUnitScale( const QgsMapUnitScale& scale);
    const QgsMapUnitScale& symbolHeightMapUnitScale() const;

    void setOutlineWidthUnit( QgsSymbolV2::OutputUnit unit );
    QgsSymbolV2::OutputUnit outlineWidthUnit() const;

    void setOutlineWidthMapUnitScale( const QgsMapUnitScale& scale);
    const QgsMapUnitScale& outlineWidthMapUnitScale() const;

    void setOutputUnit( QgsSymbolV2::OutputUnit unit );
    QgsSymbolV2::OutputUnit outputUnit() const;

    void setMapUnitScale( const QgsMapUnitScale& scale );
    QgsMapUnitScale mapUnitScale() const;

    QRectF bounds( QPointF point, QgsSymbolV2RenderContext& context );
};
