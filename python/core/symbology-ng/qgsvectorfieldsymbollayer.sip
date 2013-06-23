class QgsVectorFieldSymbolLayer : QgsMarkerSymbolLayerV2
{
%TypeHeaderCode
#include <qgsvectorfieldsymbollayer.h>
%End
  public:
    enum VectorFieldType
    {
      Cartesian = 0,
      Polar,
      Height
    };

    enum AngleOrientation
    {
      ClockwiseFromNorth = 0,
      CounterclockwiseFromEast
    };

    enum AngleUnits
    {
      Degrees = 0,
      Radians
    };

    QgsVectorFieldSymbolLayer();
    ~QgsVectorFieldSymbolLayer();

    static QgsSymbolLayerV2* create( const QgsStringMap& properties = QgsStringMap() );
    static QgsSymbolLayerV2* createFromSld( QDomElement &element );

    QString layerType() const;

    bool setSubSymbol( QgsSymbolV2* symbol /Transfer/ );
    QgsSymbolV2* subSymbol();

    void renderPoint( const QPointF& point, QgsSymbolV2RenderContext& context );
    void startRender( QgsSymbolV2RenderContext& context );
    void stopRender( QgsSymbolV2RenderContext& context );

    QgsSymbolLayerV2* clone() const /Factory/;
    QgsStringMap properties() const;

    void toSld( QDomDocument& doc, QDomElement &element, QgsStringMap props ) const;

    void drawPreviewIcon( QgsSymbolV2RenderContext& context, QSize size );

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

    void setOutputUnit( QgsSymbolV2::OutputUnit unit );
    QgsSymbolV2::OutputUnit outputUnit() const;

    void setDistanceUnit( QgsSymbolV2::OutputUnit unit );
    QgsSymbolV2::OutputUnit distanceUnit() const;
};
