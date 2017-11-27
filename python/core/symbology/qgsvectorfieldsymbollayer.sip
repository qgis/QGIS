/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/symbology/qgsvectorfieldsymbollayer.h                       *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/




class QgsVectorFieldSymbolLayer: QgsMarkerSymbolLayer
{
%Docstring
 A symbol layer class for displaying displacement arrows based on point layer attributes*
%End

%TypeHeaderCode
#include "qgsvectorfieldsymbollayer.h"
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

    static QgsSymbolLayer *create( const QgsStringMap &properties = QgsStringMap() );
%Docstring
 :rtype: QgsSymbolLayer
%End
    static QgsSymbolLayer *createFromSld( QDomElement &element );
%Docstring
 :rtype: QgsSymbolLayer
%End

    virtual QString layerType() const;

    virtual bool setSubSymbol( QgsSymbol *symbol /Transfer/ );

    virtual QgsSymbol *subSymbol();

    virtual void setColor( const QColor &color );

    virtual QColor color() const;

    virtual void renderPoint( QPointF point, QgsSymbolRenderContext &context );

    virtual void startRender( QgsSymbolRenderContext &context );

    virtual void stopRender( QgsSymbolRenderContext &context );


    virtual QgsVectorFieldSymbolLayer *clone() const /Factory/;

    virtual QgsStringMap properties() const;


    virtual void toSld( QDomDocument &doc, QDomElement &element, const QgsStringMap &props ) const;


    virtual void drawPreviewIcon( QgsSymbolRenderContext &context, QSize size );


    virtual QSet<QString> usedAttributes( const QgsRenderContext &context ) const;


    void setXAttribute( const QString &attribute );
    QString xAttribute() const;
%Docstring
 :rtype: str
%End
    void setYAttribute( const QString &attribute );
    QString yAttribute() const;
%Docstring
 :rtype: str
%End
    void setScale( double s );
    double scale() const;
%Docstring
 :rtype: float
%End
    void setVectorFieldType( VectorFieldType type );
    VectorFieldType vectorFieldType() const;
%Docstring
 :rtype: VectorFieldType
%End
    void setAngleOrientation( AngleOrientation orientation );
    AngleOrientation angleOrientation() const;
%Docstring
 :rtype: AngleOrientation
%End
    void setAngleUnits( AngleUnits units );
    AngleUnits angleUnits() const;
%Docstring
 :rtype: AngleUnits
%End

    virtual void setOutputUnit( QgsUnitTypes::RenderUnit unit );

    virtual QgsUnitTypes::RenderUnit outputUnit() const;


    virtual void setMapUnitScale( const QgsMapUnitScale &scale );

    virtual QgsMapUnitScale mapUnitScale() const;


    void setDistanceUnit( QgsUnitTypes::RenderUnit unit );
%Docstring
 Sets the units for the distance.
 \param unit distance units
.. seealso:: distanceUnit()
%End

    QgsUnitTypes::RenderUnit distanceUnit() const;
%Docstring
 Returns the units for the distance.
.. seealso:: setDistanceUnit()
 :rtype: QgsUnitTypes.RenderUnit
%End

    void setDistanceMapUnitScale( const QgsMapUnitScale &scale );
    const QgsMapUnitScale &distanceMapUnitScale() const;
%Docstring
 :rtype: QgsMapUnitScale
%End

    virtual QRectF bounds( QPointF, QgsSymbolRenderContext & );

  private:
    QgsVectorFieldSymbolLayer( const QgsVectorFieldSymbolLayer &other );
};



/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/symbology/qgsvectorfieldsymbollayer.h                       *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
