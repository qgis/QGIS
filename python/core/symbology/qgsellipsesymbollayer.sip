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
%Docstring
 :rtype: QgsSymbolLayer
%End
    static QgsSymbolLayer *createFromSld( QDomElement &element ) /Factory/;
%Docstring
 :rtype: QgsSymbolLayer
%End

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
%Docstring
 :rtype: str
%End

    void setSymbolWidth( double w );
    double symbolWidth() const;
%Docstring
 :rtype: float
%End

    void setSymbolHeight( double h );
    double symbolHeight() const;
%Docstring
 :rtype: float
%End

    Qt::PenStyle strokeStyle() const;
%Docstring
 :rtype: Qt.PenStyle
%End
    void setStrokeStyle( Qt::PenStyle strokeStyle );

    Qt::PenJoinStyle penJoinStyle() const;
%Docstring
 Get stroke join style.
.. versionadded:: 2.16
 :rtype: Qt.PenJoinStyle
%End

    void setPenJoinStyle( Qt::PenJoinStyle style );
%Docstring
 Set stroke join style.
.. versionadded:: 2.16
%End

    void setStrokeWidth( double w );
    double strokeWidth() const;
%Docstring
 :rtype: float
%End

    virtual void setFillColor( const QColor &c );
    virtual QColor fillColor() const;

    virtual void setStrokeColor( const QColor &c );
    virtual QColor strokeColor() const;

    void setSymbolWidthUnit( QgsUnitTypes::RenderUnit unit );
%Docstring
 Sets the units for the symbol's width.
 \param unit symbol units
.. seealso:: symbolWidthUnit()
.. seealso:: setSymbolHeightUnit()
%End

    QgsUnitTypes::RenderUnit symbolWidthUnit() const;
%Docstring
 Returns the units for the symbol's width.
.. seealso:: setSymbolWidthUnit()
.. seealso:: symbolHeightUnit()
 :rtype: QgsUnitTypes.RenderUnit
%End

    void setSymbolWidthMapUnitScale( const QgsMapUnitScale &scale );
    const QgsMapUnitScale &symbolWidthMapUnitScale() const;
%Docstring
 :rtype: QgsMapUnitScale
%End

    void setSymbolHeightUnit( QgsUnitTypes::RenderUnit unit );
%Docstring
 Sets the units for the symbol's height.
 \param unit symbol units
.. seealso:: symbolHeightUnit()
.. seealso:: setSymbolWidthUnit()
%End

    QgsUnitTypes::RenderUnit symbolHeightUnit() const;
%Docstring
 Returns the units for the symbol's height.
.. seealso:: setSymbolHeightUnit()
.. seealso:: symbolWidthUnit()
 :rtype: QgsUnitTypes.RenderUnit
%End

    void setSymbolHeightMapUnitScale( const QgsMapUnitScale &scale );
    const QgsMapUnitScale &symbolHeightMapUnitScale() const;
%Docstring
 :rtype: QgsMapUnitScale
%End

    void setStrokeWidthUnit( QgsUnitTypes::RenderUnit unit );
%Docstring
 Sets the units for the symbol's stroke width.
 \param unit symbol units
.. seealso:: strokeWidthUnit()
%End

    QgsUnitTypes::RenderUnit strokeWidthUnit() const;
%Docstring
 Returns the units for the symbol's stroke width.
.. seealso:: setStrokeWidthUnit()
 :rtype: QgsUnitTypes.RenderUnit
%End

    void setStrokeWidthMapUnitScale( const QgsMapUnitScale &scale );
    const QgsMapUnitScale &strokeWidthMapUnitScale() const;
%Docstring
 :rtype: QgsMapUnitScale
%End

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
