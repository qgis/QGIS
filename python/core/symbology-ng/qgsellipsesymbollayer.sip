class QgsEllipseSymbolLayer : QgsMarkerSymbolLayer
{
%TypeHeaderCode
#include <qgsellipsesymbollayer.h>
%End

  public:
    QgsEllipseSymbolLayer();
    ~QgsEllipseSymbolLayer();

    static QgsSymbolLayer* create( const QgsStringMap& properties = QgsStringMap() ) /Factory/;
    static QgsSymbolLayer* createFromSld( QDomElement &element ) /Factory/;

    void renderPoint( QPointF point, QgsSymbolRenderContext& context );
    QString layerType() const;
    void startRender( QgsSymbolRenderContext& context );
    void stopRender( QgsSymbolRenderContext& context );
    virtual QgsEllipseSymbolLayer* clone() const /Factory/;
    QgsStringMap properties() const;

    void toSld( QDomDocument& doc, QDomElement &element, const QgsStringMap& props ) const;
    void writeSldMarker( QDomDocument& doc, QDomElement &element, const QgsStringMap& props ) const;

    bool writeDxf( QgsDxfExport &e, double mmMapUnitScaleFactor, const QString &layerName, QgsSymbolRenderContext &context, QPointF shift = QPointF( 0.0, 0.0 ) ) const;

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

    /** Sets the units for the symbol's width.
     * @param unit symbol units
     * @see symbolWidthUnit()
     * @see setSymbolHeightUnit()
    */
    void setSymbolWidthUnit( QgsUnitTypes::RenderUnit unit );

    /** Returns the units for the symbol's width.
     * @see setSymbolWidthUnit()
     * @see symbolHeightUnit()
    */
    QgsUnitTypes::RenderUnit symbolWidthUnit() const;

    void setSymbolWidthMapUnitScale( const QgsMapUnitScale& scale);
    const QgsMapUnitScale& symbolWidthMapUnitScale() const;

    /** Sets the units for the symbol's height.
     * @param unit symbol units
     * @see symbolHeightUnit()
     * @see setSymbolWidthUnit()
    */
    void setSymbolHeightUnit( QgsUnitTypes::RenderUnit unit );

    /** Returns the units for the symbol's height.
     * @see setSymbolHeightUnit()
     * @see symbolWidthUnit()
    */
    QgsUnitTypes::RenderUnit symbolHeightUnit() const;

    void setSymbolHeightMapUnitScale( const QgsMapUnitScale& scale);
    const QgsMapUnitScale& symbolHeightMapUnitScale() const;

    /** Sets the units for the symbol's outline width.
     * @param unit symbol units
     * @see outlineWidthUnit()
    */
    void setOutlineWidthUnit( QgsUnitTypes::RenderUnit unit );

    /** Returns the units for the symbol's outline width.
     * @see setOutlineWidthUnit()
    */
    QgsUnitTypes::RenderUnit outlineWidthUnit() const;

    void setOutlineWidthMapUnitScale( const QgsMapUnitScale& scale);
    const QgsMapUnitScale& outlineWidthMapUnitScale() const;

    void setOutputUnit( QgsUnitTypes::RenderUnit unit );
    QgsUnitTypes::RenderUnit outputUnit() const;

    void setMapUnitScale( const QgsMapUnitScale& scale );
    QgsMapUnitScale mapUnitScale() const;

    QRectF bounds( QPointF point, QgsSymbolRenderContext& context );
};
