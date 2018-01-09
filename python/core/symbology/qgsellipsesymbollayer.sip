/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/symbology/qgsellipsesymbollayer.h                           *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/




class QgsEllipseSymbolLayer: QgsMarkerSymbolLayer
{
%Docstring
A symbol layer for rendering objects with major and minor axis (e.g. ellipse, rectangle )*
%End

%TypeHeaderCode
#include "qgsellipsesymbollayer.h"
%End
  public:
    QgsEllipseSymbolLayer();

    static QgsSymbolLayer *create( const QgsStringMap &properties = QgsStringMap() ) /Factory/;
    static QgsSymbolLayer *createFromSld( QDomElement &element ) /Factory/;

    virtual void renderPoint( QPointF point, QgsSymbolRenderContext &context );

    virtual QString layerType() const;

    virtual void startRender( QgsSymbolRenderContext &context );

    virtual void stopRender( QgsSymbolRenderContext &context );

    virtual QgsEllipseSymbolLayer *clone() const /Factory/;

    virtual QgsStringMap properties() const;


    virtual void toSld( QDomDocument &doc, QDomElement &element, const QgsStringMap &props ) const;

    virtual void writeSldMarker( QDomDocument &doc, QDomElement &element, const QgsStringMap &props ) const;


    virtual bool writeDxf( QgsDxfExport &e, double mmMapUnitScaleFactor, const QString &layerName, QgsSymbolRenderContext &context, QPointF shift = QPointF( 0.0, 0.0 ) ) const;


    void setSymbolName( const QString &name );
    QString symbolName() const;

    void setSymbolWidth( double w );
    double symbolWidth() const;

    void setSymbolHeight( double h );
    double symbolHeight() const;

    Qt::PenStyle strokeStyle() const;
    void setStrokeStyle( Qt::PenStyle strokeStyle );

    Qt::PenJoinStyle penJoinStyle() const;
%Docstring
Get stroke join style.

.. versionadded:: 2.16
%End

    void setPenJoinStyle( Qt::PenJoinStyle style );
%Docstring
Set stroke join style.

.. versionadded:: 2.16
%End

    void setStrokeWidth( double w );
    double strokeWidth() const;

    virtual void setFillColor( const QColor &c );
    virtual QColor fillColor() const;

    virtual void setStrokeColor( const QColor &c );
    virtual QColor strokeColor() const;

    void setSymbolWidthUnit( QgsUnitTypes::RenderUnit unit );
%Docstring
Sets the units for the symbol's width.

:param unit: symbol units

.. seealso:: :py:func:`symbolWidthUnit`

.. seealso:: :py:func:`setSymbolHeightUnit`
%End

    QgsUnitTypes::RenderUnit symbolWidthUnit() const;
%Docstring
Returns the units for the symbol's width.

.. seealso:: :py:func:`setSymbolWidthUnit`

.. seealso:: :py:func:`symbolHeightUnit`
%End

    void setSymbolWidthMapUnitScale( const QgsMapUnitScale &scale );
    const QgsMapUnitScale &symbolWidthMapUnitScale() const;

    void setSymbolHeightUnit( QgsUnitTypes::RenderUnit unit );
%Docstring
Sets the units for the symbol's height.

:param unit: symbol units

.. seealso:: :py:func:`symbolHeightUnit`

.. seealso:: :py:func:`setSymbolWidthUnit`
%End

    QgsUnitTypes::RenderUnit symbolHeightUnit() const;
%Docstring
Returns the units for the symbol's height.

.. seealso:: :py:func:`setSymbolHeightUnit`

.. seealso:: :py:func:`symbolWidthUnit`
%End

    void setSymbolHeightMapUnitScale( const QgsMapUnitScale &scale );
    const QgsMapUnitScale &symbolHeightMapUnitScale() const;

    void setStrokeWidthUnit( QgsUnitTypes::RenderUnit unit );
%Docstring
Sets the units for the symbol's stroke width.

:param unit: symbol units

.. seealso:: :py:func:`strokeWidthUnit`
%End

    QgsUnitTypes::RenderUnit strokeWidthUnit() const;
%Docstring
Returns the units for the symbol's stroke width.

.. seealso:: :py:func:`setStrokeWidthUnit`
%End

    void setStrokeWidthMapUnitScale( const QgsMapUnitScale &scale );
    const QgsMapUnitScale &strokeWidthMapUnitScale() const;

    virtual void setOutputUnit( QgsUnitTypes::RenderUnit unit );

    virtual QgsUnitTypes::RenderUnit outputUnit() const;


    virtual void setMapUnitScale( const QgsMapUnitScale &scale );

    virtual QgsMapUnitScale mapUnitScale() const;


    virtual QRectF bounds( QPointF point, QgsSymbolRenderContext &context );


};




/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/symbology/qgsellipsesymbollayer.h                           *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
