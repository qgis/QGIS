class QgsVectorFieldSymbolLayer : QgsMarkerSymbolLayer
{
%TypeHeaderCode
#include <qgsvectorfieldsymbollayer.h>
%End
  public:
    enum VectorFieldType
    {
      Cartesian,
      Polar,
      Height
    };

    enum AngleOrientation
    {
      ClockwiseFromNorth,
      CounterclockwiseFromEast
    };

    enum AngleUnits
    {
      Degrees,
      Radians
    };

    QgsVectorFieldSymbolLayer();
    ~QgsVectorFieldSymbolLayer();

    static QgsSymbolLayer* create( const QgsStringMap& properties = QgsStringMap() );
    static QgsSymbolLayer* createFromSld( QDomElement &element );

    QString layerType() const;

    bool setSubSymbol( QgsSymbol* symbol /Transfer/ );
    QgsSymbol* subSymbol();

    void setColor( const QColor& color );
    virtual QColor color() const;

    void renderPoint( QPointF point, QgsSymbolRenderContext& context );
    void startRender( QgsSymbolRenderContext& context );
    void stopRender( QgsSymbolRenderContext& context );

    virtual QgsVectorFieldSymbolLayer* clone() const /Factory/;
    QgsStringMap properties() const;

    void toSld( QDomDocument& doc, QDomElement &element, const QgsStringMap& props ) const;

    void drawPreviewIcon( QgsSymbolRenderContext& context, QSize size );

    QSet<QString> usedAttributes() const;

    //setters and getters
    void setXAttribute( const QString& attribute );
    QString xAttribute() const;
    void setYAttribute( const QString& attribute );
    QString yAttribute() const;
    void setScale( double s );
    double scale() const;
    void setVectorFieldType( VectorFieldType type );
    VectorFieldType vectorFieldType() const;
    void setAngleOrientation( AngleOrientation orientation );
    AngleOrientation angleOrientation() const;
    void setAngleUnits( AngleUnits units );
    AngleUnits angleUnits() const;

    void setOutputUnit( QgsUnitTypes::RenderUnit unit );
    QgsUnitTypes::RenderUnit outputUnit() const;

    void setMapUnitScale( const QgsMapUnitScale& scale );
    QgsMapUnitScale mapUnitScale() const;

    /** Sets the units for the distance.
     * @param unit distance units
     * @see distanceUnit()
    */
    void setDistanceUnit( QgsUnitTypes::RenderUnit unit );

    /** Returns the units for the distance.
     * @see setDistanceUnit()
    */
    QgsUnitTypes::RenderUnit distanceUnit() const;

    void setDistanceMapUnitScale( const QgsMapUnitScale& scale );
    const QgsMapUnitScale& distanceMapUnitScale() const;

    QRectF bounds( QPointF point, QgsSymbolRenderContext& context );
};
