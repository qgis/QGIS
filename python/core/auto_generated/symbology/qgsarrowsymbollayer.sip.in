/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/symbology/qgsarrowsymbollayer.h                             *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/





class QgsArrowSymbolLayer : QgsLineSymbolLayer
{
%Docstring
Line symbol layer used for representing lines as arrows.

.. versionadded:: 2.16
%End

%TypeHeaderCode
#include "qgsarrowsymbollayer.h"
%End
  public:
    QgsArrowSymbolLayer();
%Docstring
Simple constructor
%End

    static QgsSymbolLayer *create( const QgsStringMap &properties = QgsStringMap() ) /Factory/;
%Docstring
Create a new QgsArrowSymbolLayer

:param properties: A property map to deserialize saved information from properties()

:return: A new QgsArrowSymbolLayer
%End

    virtual QgsArrowSymbolLayer *clone() const /Factory/;

    virtual QgsSymbol *subSymbol();
    virtual bool setSubSymbol( QgsSymbol *symbol /Transfer/ );

    virtual QSet<QString> usedAttributes( const QgsRenderContext &context ) const;

    virtual bool hasDataDefinedProperties() const;


    double arrowWidth() const;
%Docstring
Gets current arrow width
%End
    void setArrowWidth( double width );
%Docstring
Sets the arrow width
%End
    QgsUnitTypes::RenderUnit arrowWidthUnit() const;
%Docstring
Gets the unit for the arrow width
%End
    void setArrowWidthUnit( QgsUnitTypes::RenderUnit unit );
%Docstring
Sets the unit for the arrow width
%End
    QgsMapUnitScale arrowWidthUnitScale() const;
%Docstring
Gets the scale for the arrow width
%End
    void setArrowWidthUnitScale( const QgsMapUnitScale &scale );
%Docstring
Sets the scale for the arrow width
%End

    double arrowStartWidth() const;
%Docstring
Gets current arrow start width. Only meaningful for single headed arrows
%End
    void setArrowStartWidth( double width );
%Docstring
Sets the arrow start width
%End
    QgsUnitTypes::RenderUnit arrowStartWidthUnit() const;
%Docstring
Gets the unit for the arrow start width
%End
    void setArrowStartWidthUnit( QgsUnitTypes::RenderUnit unit );
%Docstring
Sets the unit for the arrow start width
%End
    QgsMapUnitScale arrowStartWidthUnitScale() const;
%Docstring
Gets the scale for the arrow start width
%End
    void setArrowStartWidthUnitScale( const QgsMapUnitScale &scale );
%Docstring
Sets the scale for the arrow start width
%End

    double headLength() const;
%Docstring
Gets the current arrow head length
%End
    void setHeadLength( double length );
%Docstring
Sets the arrow head length
%End
    QgsUnitTypes::RenderUnit headLengthUnit() const;
%Docstring
Gets the unit for the head length
%End
    void setHeadLengthUnit( QgsUnitTypes::RenderUnit unit );
%Docstring
Sets the unit for the head length
%End
    QgsMapUnitScale headLengthUnitScale() const;
%Docstring
Gets the scale for the head length
%End
    void setHeadLengthUnitScale( const QgsMapUnitScale &scale );
%Docstring
Sets the scale for the head length
%End

    double headThickness() const;
%Docstring
Gets the current arrow head height
%End
    void setHeadThickness( double thickness );
%Docstring
Sets the arrow head height
%End
    QgsUnitTypes::RenderUnit headThicknessUnit() const;
%Docstring
Gets the unit for the head height
%End
    void setHeadThicknessUnit( QgsUnitTypes::RenderUnit unit );
%Docstring
Sets the unit for the head height
%End
    QgsMapUnitScale headThicknessUnitScale() const;
%Docstring
Gets the scale for the head height
%End
    void setHeadThicknessUnitScale( const QgsMapUnitScale &scale );
%Docstring
Sets the scale for the head height
%End

    bool isCurved() const;
%Docstring
Returns whether it is a curved arrow or a straight one
%End
    void setIsCurved( bool isCurved );
%Docstring
Sets whether it is a curved arrow or a straight one
%End

    bool isRepeated() const;
%Docstring
Returns whether the arrow is repeated along the line or not
%End
    void setIsRepeated( bool isRepeated );
%Docstring
Sets whether the arrow is repeated along the line
%End

    enum HeadType
    {
      HeadSingle,
      HeadReversed,
      HeadDouble
    };

    HeadType headType() const;
%Docstring
Gets the current head type
%End
    void setHeadType( HeadType type );
%Docstring
Sets the head type
%End

    enum ArrowType
    {
      ArrowPlain,
      ArrowLeftHalf,
      ArrowRightHalf
    };

    ArrowType arrowType() const;
%Docstring
Gets the current arrow type
%End
    void setArrowType( ArrowType type );
%Docstring
Sets the arrow type
%End

    virtual QgsStringMap properties() const;

    virtual QString layerType() const;

    virtual void startRender( QgsSymbolRenderContext &context );

    virtual void stopRender( QgsSymbolRenderContext &context );

    virtual void renderPolyline( const QPolygonF &points, QgsSymbolRenderContext &context );

    virtual void setColor( const QColor &c );

    virtual QColor color() const;


  private:
    QgsArrowSymbolLayer( const QgsArrowSymbolLayer & );
};



/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/symbology/qgsarrowsymbollayer.h                             *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
