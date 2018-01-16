/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/symbology/qgssymbollayer.h                                  *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/






class QgsSymbolLayer
{

%TypeHeaderCode
#include "qgssymbollayer.h"
#include <qgslinesymbollayer.h>
%End
%ConvertToSubClassCode
    switch ( sipCpp->type() )
    {
      case QgsSymbol::Marker:
        if ( sipCpp->layerType() == "EllipseMarker" )
          sipType = sipType_QgsEllipseSymbolLayer;
        else if ( sipCpp->layerType() == "FontMarker" )
          sipType = sipType_QgsFontMarkerSymbolLayer;
        else if ( sipCpp->layerType() == "SimpleMarker" )
          sipType = sipType_QgsSimpleMarkerSymbolLayer;
        else if ( sipCpp->layerType() == "FilledMarker" )
          sipType = sipType_QgsFilledMarkerSymbolLayer;
        else if ( sipCpp->layerType() == "SvgMarker" )
          sipType = sipType_QgsSvgMarkerSymbolLayer;
        else if ( sipCpp->layerType() == "VectorField" )
          sipType = sipType_QgsVectorFieldSymbolLayer;
        else
          sipType = sipType_QgsMarkerSymbolLayer;
        break;

      case QgsSymbol::Line:
        if ( sipCpp->layerType() == "MarkerLine" )
          sipType = sipType_QgsMarkerLineSymbolLayer;
        else if ( sipCpp->layerType() == "SimpleLine" )
          sipType = sipType_QgsSimpleLineSymbolLayer;
        else if ( sipCpp->layerType() == "ArrowLine" )
          sipType = sipType_QgsArrowSymbolLayer;
        else
          sipType = sipType_QgsLineSymbolLayer;
        break;

      case QgsSymbol::Fill:
        if ( sipCpp->layerType() == "SimpleFill" )
          sipType = sipType_QgsSimpleFillSymbolLayer;
        else if ( sipCpp->layerType() == "LinePatternFill" )
          sipType = sipType_QgsLinePatternFillSymbolLayer;
        else if ( sipCpp->layerType() == "PointPatternFill" )
          sipType = sipType_QgsPointPatternFillSymbolLayer;
        else if ( sipCpp->layerType() == "SVGFill" )
          sipType = sipType_QgsSVGFillSymbolLayer;
        else if ( sipCpp->layerType() == "RasterFill" )
          sipType = sipType_QgsRasterFillSymbolLayer;
        else if ( sipCpp->layerType() == "CentroidFill" )
          sipType = sipType_QgsCentroidFillSymbolLayer;
        else if ( sipCpp->layerType() == "GradientFill" )
          sipType = sipType_QgsGradientFillSymbolLayer;
        else if ( sipCpp->layerType() == "ShapeburstFill" )
          sipType = sipType_QgsShapeburstFillSymbolLayer;
        else
          sipType = sipType_QgsFillSymbolLayer;
        break;

      case QgsSymbol::Hybrid:
        sipType = sipType_QgsGeometryGeneratorSymbolLayer;
        break;
    }
%End
  public:

    enum Property
    {
      PropertySize,
      PropertyAngle,
      PropertyName,
      PropertyFillColor,
      PropertyStrokeColor,
      PropertyStrokeWidth,
      PropertyStrokeStyle,
      PropertyOffset,
      PropertyCharacter,
      PropertyWidth,
      PropertyHeight,
      PropertyPreserveAspectRatio,
      PropertyFillStyle,
      PropertyJoinStyle,
      PropertySecondaryColor,
      PropertyLineAngle,
      PropertyLineDistance,
      PropertyGradientType,
      PropertyCoordinateMode,
      PropertyGradientSpread,
      PropertyGradientReference1X,
      PropertyGradientReference1Y,
      PropertyGradientReference2X,
      PropertyGradientReference2Y,
      PropertyGradientReference1IsCentroid,
      PropertyGradientReference2IsCentroid,
      PropertyBlurRadius,
      PropertyShapeburstUseWholeShape,
      PropertyShapeburstMaxDistance,
      PropertyShapeburstIgnoreRings,
      PropertyFile,
      PropertyDistanceX,
      PropertyDistanceY,
      PropertyDisplacementX,
      PropertyDisplacementY,
      PropertyOpacity,
      PropertyCustomDash,
      PropertyCapStyle,
      PropertyPlacement,
      PropertyInterval,
      PropertyOffsetAlongLine,
      PropertyHorizontalAnchor,
      PropertyVerticalAnchor,
      PropertyLayerEnabled,
      PropertyArrowWidth,
      PropertyArrowStartWidth,
      PropertyArrowHeadLength,
      PropertyArrowHeadThickness,
      PropertyArrowHeadType,
      PropertyArrowType,
    };

    static const QgsPropertiesDefinition &propertyDefinitions();
%Docstring
Returns the symbol layer property definitions.

.. versionadded:: 3.0
%End

    virtual ~QgsSymbolLayer();

    bool enabled() const;
%Docstring
Returns true if symbol layer is enabled and will be drawn.

.. versionadded:: 3.0

.. seealso:: :py:func:`setEnabled`
%End

    void setEnabled( bool enabled );
%Docstring
Sets whether symbol layer is enabled and should be drawn. Disabled
layers are not drawn, but remain part of the symbol and can be re-enabled
when desired.

.. versionadded:: 3.0

.. seealso:: :py:func:`enabled`
%End

    virtual QColor color() const;
%Docstring
The fill color.
%End

    virtual void setColor( const QColor &color );
%Docstring
The fill color.
%End

    virtual void setStrokeColor( const QColor &color );
%Docstring
Set stroke color. Supported by marker and fill layers.

.. versionadded:: 2.1
%End

    virtual QColor strokeColor() const;
%Docstring
Get stroke color. Supported by marker and fill layers.

.. versionadded:: 2.1
%End

    virtual void setFillColor( const QColor &color );
%Docstring
Set fill color. Supported by marker and fill layers.

.. versionadded:: 2.1
%End

    virtual QColor fillColor() const;
%Docstring
Get fill color. Supported by marker and fill layers.

.. versionadded:: 2.1
%End

    virtual QString layerType() const = 0;
%Docstring
Returns a string that represents this layer type. Used for serialization.
Should match with the string used to register this symbol layer in the registry.
%End

    virtual void startRender( QgsSymbolRenderContext &context ) = 0;
    virtual void stopRender( QgsSymbolRenderContext &context ) = 0;

    virtual QgsSymbolLayer *clone() const = 0 /Factory/;
%Docstring
Shall be reimplemented by subclasses to create a deep copy of the instance.
%End

    virtual void toSld( QDomDocument &doc, QDomElement &element, const QgsStringMap &props ) const;

    virtual QString ogrFeatureStyle( double mmScaleFactor, double mapUnitScaleFactor ) const;

    virtual QgsStringMap properties() const = 0;
%Docstring
Should be reimplemented by subclasses to return a string map that
contains the configuration information for the symbol layer. This
is used to serialize a symbol layer perstistently.
%End

    virtual void drawPreviewIcon( QgsSymbolRenderContext &context, QSize size ) = 0;

    virtual QgsSymbol *subSymbol();
%Docstring
Returns the symbol's sub symbol, if present.
%End

    virtual bool setSubSymbol( QgsSymbol *symbol /Transfer/ );
%Docstring
set layer's subsymbol. takes ownership of the passed symbol
%End

    QgsSymbol::SymbolType type() const;

    virtual bool isCompatibleWithSymbol( QgsSymbol *symbol ) const;
%Docstring
Returns if the layer can be used below the specified symbol
%End

    void setLocked( bool locked );
    bool isLocked() const;

    virtual double estimateMaxBleed( const QgsRenderContext &context ) const;
%Docstring
Returns the estimated maximum distance which the layer style will bleed outside
the drawn shape when drawn in the specified /a context. For example, polygons
drawn with an stroke will draw half the width
of the stroke outside of the polygon. This amount is estimated, since it may
be affected by data defined symbology rules.*
%End

    virtual void setOutputUnit( QgsUnitTypes::RenderUnit unit );
%Docstring
Sets the units to use for sizes and widths within the symbol layer. Individual
symbol layer subclasses will interpret this in different ways, e.g., a marker symbol
layer may use it to specify the units for the marker size, while a line symbol
layer may use it to specify the units for the line width.

:param unit: output units

.. seealso:: :py:func:`outputUnit`
%End

    virtual QgsUnitTypes::RenderUnit outputUnit() const;
%Docstring
Returns the units to use for sizes and widths within the symbol layer. Individual
symbol layer subclasses will interpret this in different ways, e.g., a marker symbol
layer may use it to specify the units for the marker size, while a line symbol
layer may use it to specify the units for the line width.

:return: output unit, or QgsUnitTypes.RenderUnknownUnit if the symbol layer contains mixed units

.. seealso:: :py:func:`setOutputUnit`
%End

    virtual void setMapUnitScale( const QgsMapUnitScale &scale );
    virtual QgsMapUnitScale mapUnitScale() const;

    void setRenderingPass( int renderingPass );
    int renderingPass() const;

    virtual QSet<QString> usedAttributes( const QgsRenderContext &context ) const;
%Docstring
Returns the set of attributes referenced by the layer. This includes attributes
required by any data defined properties associated with the layer.
%End

    virtual void setDataDefinedProperty( Property key, const QgsProperty &property );
%Docstring
Sets a data defined property for the layer. Any existing property with the same key
will be overwritten.

.. versionadded:: 3.0

.. seealso:: :py:func:`getDataDefinedProperty`

.. seealso:: :py:func:`removeDataDefinedProperty`
%End

    virtual bool writeDxf( QgsDxfExport &e, double mmMapUnitScaleFactor, const QString &layerName, QgsSymbolRenderContext &context, QPointF shift = QPointF( 0.0, 0.0 ) ) const;
%Docstring
write as DXF
%End

    virtual double dxfWidth( const QgsDxfExport &e, QgsSymbolRenderContext &context ) const;
%Docstring
get line width
%End

    virtual double dxfOffset( const QgsDxfExport &e, QgsSymbolRenderContext &context ) const;
%Docstring
get offset
%End

    virtual QColor dxfColor( QgsSymbolRenderContext &context ) const;
%Docstring
get color
%End

    virtual double dxfAngle( QgsSymbolRenderContext &context ) const;
%Docstring
get angle
%End

    virtual QVector<qreal> dxfCustomDashPattern( QgsUnitTypes::RenderUnit &unit ) const;
%Docstring
get dash pattern
%End

    virtual Qt::PenStyle dxfPenStyle() const;
%Docstring
get pen style
%End

    virtual QColor dxfBrushColor( QgsSymbolRenderContext &context ) const;
%Docstring
get brush/fill color
%End

    virtual Qt::BrushStyle dxfBrushStyle() const;
%Docstring
get brush/fill style
%End

    QgsPaintEffect *paintEffect() const;
%Docstring
Returns the current paint effect for the layer.

:return: paint effect

.. versionadded:: 2.9

.. seealso:: :py:func:`setPaintEffect`
%End

    void setPaintEffect( QgsPaintEffect *effect /Transfer/ );
%Docstring
Sets the current paint effect for the layer.

:param effect: paint effect. Ownership is transferred to the layer.

.. versionadded:: 2.9

.. seealso:: :py:func:`paintEffect`
%End

    virtual void prepareExpressions( const QgsSymbolRenderContext &context );
%Docstring
Prepares all data defined property expressions for evaluation. This should
be called prior to evaluating data defined properties.

:param context: symbol render context

.. versionadded:: 2.12
%End

    QgsPropertyCollection &dataDefinedProperties();
%Docstring
Returns a reference to the symbol layer's property collection, used for data defined overrides.

.. versionadded:: 3.0

.. seealso:: :py:func:`setProperties`
%End


    void setDataDefinedProperties( const QgsPropertyCollection &collection );
%Docstring
Sets the symbol layer's property collection, used for data defined overrides.

:param collection: property collection. Existing properties will be replaced.

.. versionadded:: 3.0

.. seealso:: :py:func:`properties`
%End

  protected:

    QgsSymbolLayer( QgsSymbol::SymbolType type, bool locked = false );







    void restoreOldDataDefinedProperties( const QgsStringMap &stringMap );
%Docstring
Restores older data defined properties from string map.

.. versionadded:: 3.0
%End

    void copyDataDefinedProperties( QgsSymbolLayer *destLayer ) const;
%Docstring
Copies all data defined properties of this layer to another symbol layer.

:param destLayer: destination layer
%End

    void copyPaintEffect( QgsSymbolLayer *destLayer ) const;
%Docstring
Copies paint effect of this layer to another symbol layer

:param destLayer: destination layer

.. versionadded:: 2.9
%End

};


class QgsMarkerSymbolLayer : QgsSymbolLayer
{
%Docstring
 Abstract base class for marker symbol layers.
%End

%TypeHeaderCode
#include "qgssymbollayer.h"
%End
  public:

    enum HorizontalAnchorPoint
    {
      Left,
      HCenter,
      Right,
    };

    enum VerticalAnchorPoint
    {
      Top,
      VCenter,
      Bottom,
    };

    virtual void startRender( QgsSymbolRenderContext &context );


    virtual void renderPoint( QPointF point, QgsSymbolRenderContext &context ) = 0;
%Docstring
Renders a marker at the specified point. Derived classes must implement this to
handle drawing the point.

:param point: position at which to render point, in painter units
:param context: symbol render context
%End

    virtual void drawPreviewIcon( QgsSymbolRenderContext &context, QSize size );


    void setAngle( double angle );
%Docstring
Sets the rotation angle for the marker.

:param angle: angle in degrees clockwise from north.

.. seealso:: :py:func:`angle`

.. seealso:: :py:func:`setLineAngle`
%End

    double angle() const;
%Docstring
Returns the rotation angle for the marker, in degrees clockwise from north.

.. seealso:: :py:func:`setAngle`
%End

    void setLineAngle( double lineAngle );
%Docstring
Sets the line angle modification for the symbol's angle. This angle is added to
the marker's rotation and data defined rotation before rendering the symbol, and
is usually used for orienting symbols to match a line's angle.

:param lineAngle: Angle in degrees clockwise from north, valid values are between 0 and 360

.. versionadded:: 2.9

.. seealso:: :py:func:`setAngle`

.. seealso:: :py:func:`angle`
%End

    void setSize( double size );
%Docstring
Sets the symbol size.

:param size: symbol size. Units are specified by sizeUnit().

.. seealso:: :py:func:`size`

.. seealso:: :py:func:`setSizeUnit`

.. seealso:: :py:func:`setSizeMapUnitScale`
%End

    double size() const;
%Docstring
Returns the symbol size. Units are specified by sizeUnit().

.. seealso:: :py:func:`setSize`

.. seealso:: :py:func:`sizeUnit`

.. seealso:: :py:func:`sizeUnitMapScale`
%End

    void setSizeUnit( QgsUnitTypes::RenderUnit unit );
%Docstring
Sets the units for the symbol's size.

:param unit: size units

.. seealso:: :py:func:`sizeUnit`

.. seealso:: :py:func:`setSize`

.. seealso:: :py:func:`setSizeMapUnitScale`
%End

    QgsUnitTypes::RenderUnit sizeUnit() const;
%Docstring
Returns the units for the symbol's size.

.. seealso:: :py:func:`setSizeUnit`

.. seealso:: :py:func:`size`

.. seealso:: :py:func:`sizeMapUnitScale`
%End

    void setSizeMapUnitScale( const QgsMapUnitScale &scale );
%Docstring
Sets the map unit scale for the symbol's size.

:param scale: size map unit scale

.. seealso:: :py:func:`sizeMapUnitScale`

.. seealso:: :py:func:`setSize`

.. seealso:: :py:func:`setSizeUnit`
%End

    const QgsMapUnitScale &sizeMapUnitScale() const;
%Docstring
Returns the map unit scale for the symbol's size.

.. seealso:: :py:func:`setSizeMapUnitScale`

.. seealso:: :py:func:`size`

.. seealso:: :py:func:`sizeUnit`
%End

    void setScaleMethod( QgsSymbol::ScaleMethod scaleMethod );
%Docstring
Sets the method to use for scaling the marker's size.

:param scaleMethod: scale method

.. seealso:: :py:func:`scaleMethod`
%End

    QgsSymbol::ScaleMethod scaleMethod() const;
%Docstring
Returns the method to use for scaling the marker's size.

.. seealso:: :py:func:`setScaleMethod`
%End

    void setOffset( QPointF offset );
%Docstring
Sets the marker's offset, which is the horizontal and vertical displacement which the rendered marker
should have from the original feature's geometry.

:param offset: marker offset. Units are specified by offsetUnit()

.. seealso:: :py:func:`offset`

.. seealso:: :py:func:`setOffsetUnit`

.. seealso:: :py:func:`setOffsetMapUnitScale`
%End

    QPointF offset() const;
%Docstring
Returns the marker's offset, which is the horizontal and vertical displacement which the rendered marker
will have from the original feature's geometry. Units are specified by offsetUnit().

.. seealso:: :py:func:`setOffset`

.. seealso:: :py:func:`offsetUnit`

.. seealso:: :py:func:`offsetMapUnitScale`
%End

    void setOffsetUnit( QgsUnitTypes::RenderUnit unit );
%Docstring
Sets the units for the symbol's offset.

:param unit: offset units

.. seealso:: :py:func:`offsetUnit`

.. seealso:: :py:func:`setOffset`

.. seealso:: :py:func:`setOffsetMapUnitScale`
%End

    QgsUnitTypes::RenderUnit offsetUnit() const;
%Docstring
Returns the units for the symbol's offset.

.. seealso:: :py:func:`setOffsetUnit`

.. seealso:: :py:func:`offset`

.. seealso:: :py:func:`offsetMapUnitScale`
%End

    void setOffsetMapUnitScale( const QgsMapUnitScale &scale );
%Docstring
Sets the map unit scale for the symbol's offset.

:param scale: offset map unit scale

.. seealso:: :py:func:`offsetMapUnitScale`

.. seealso:: :py:func:`setOffset`

.. seealso:: :py:func:`setOffsetUnit`
%End

    const QgsMapUnitScale &offsetMapUnitScale() const;
%Docstring
Returns the map unit scale for the symbol's offset.

.. seealso:: :py:func:`setOffsetMapUnitScale`

.. seealso:: :py:func:`offset`

.. seealso:: :py:func:`offsetUnit`
%End

    void setHorizontalAnchorPoint( HorizontalAnchorPoint h );
%Docstring
Sets the horizontal anchor point for positioning the symbol.

:param h: anchor point. Symbol will be drawn so that the horizontal anchor point is aligned with
the marker's desired location.

.. seealso:: :py:func:`horizontalAnchorPoint`

.. seealso:: :py:func:`setVerticalAnchorPoint`
%End

    HorizontalAnchorPoint horizontalAnchorPoint() const;
%Docstring
Returns the horizontal anchor point for positioning the symbol. The symbol will be drawn so that
the horizontal anchor point is aligned with the marker's desired location.

.. seealso:: :py:func:`setHorizontalAnchorPoint`

.. seealso:: :py:func:`verticalAnchorPoint`
%End

    void setVerticalAnchorPoint( VerticalAnchorPoint v );
%Docstring
Sets the vertical anchor point for positioning the symbol.

:param v: anchor point. Symbol will be drawn so that the vertical anchor point is aligned with
the marker's desired location.

.. seealso:: :py:func:`verticalAnchorPoint`

.. seealso:: :py:func:`setHorizontalAnchorPoint`
%End

    VerticalAnchorPoint verticalAnchorPoint() const;
%Docstring
Returns the vertical anchor point for positioning the symbol. The symbol will be drawn so that
the vertical anchor point is aligned with the marker's desired location.

.. seealso:: :py:func:`setVerticalAnchorPoint`

.. seealso:: :py:func:`horizontalAnchorPoint`
%End

    virtual void toSld( QDomDocument &doc, QDomElement &element, const QgsStringMap &props ) const;


    virtual void writeSldMarker( QDomDocument &doc, QDomElement &element, const QgsStringMap &props ) const;
%Docstring
Writes the symbol layer definition as a SLD XML element.

:param doc: XML document
:param element: parent XML element
:param props: symbol layer definition (see properties())
%End

    virtual void setOutputUnit( QgsUnitTypes::RenderUnit unit );

    virtual QgsUnitTypes::RenderUnit outputUnit() const;

    virtual void setMapUnitScale( const QgsMapUnitScale &scale );

    virtual QgsMapUnitScale mapUnitScale() const;


    virtual QRectF bounds( QPointF point, QgsSymbolRenderContext &context ) = 0;
%Docstring
Returns the approximate bounding box of the marker symbol layer, taking into account
any data defined overrides and offsets which are set for the marker layer.

:return: approximate symbol bounds, in painter units

.. versionadded:: 2.14
%End

  protected:

    QgsMarkerSymbolLayer( bool locked = false );
%Docstring
Constructor for QgsMarkerSymbolLayer.

:param locked: set to true to lock symbol color
%End

    void markerOffset( QgsSymbolRenderContext &context, double &offsetX, double &offsetY ) const;
%Docstring
Calculates the required marker offset, including both the symbol offset
and any displacement required to align with the marker's anchor point.

:param context: symbol render context
:param offsetX: will be set to required horizontal offset (in painter units)
:param offsetY: will be set to required vertical offset (in painter units)
%End

    void markerOffset( QgsSymbolRenderContext &context, double width, double height, double &offsetX, double &offsetY ) const /PyName=markerOffsetWithWidthAndHeight/;
%Docstring
Calculates the required marker offset, including both the symbol offset
and any displacement required to align with the marker's anchor point.

:param context: symbol render context
:param width: marker width
:param height: marker height
:param offsetX: will be set to required horizontal offset (in painter units)
:param offsetY: will be set to required vertical offset (in painter units)

.. note::

   available in Python as markerOffsetWithWidthAndHeight
%End

    void markerOffset( QgsSymbolRenderContext &context, double width, double height,
                       QgsUnitTypes::RenderUnit widthUnit, QgsUnitTypes::RenderUnit heightUnit,
                       double &offsetX, double &offsetY,
                       const QgsMapUnitScale &widthMapUnitScale, const QgsMapUnitScale &heightMapUnitScale ) const /PyName=markerOffset2/;
%Docstring

.. note::

   available in Python bindings as markerOffset2
%End

    static QPointF _rotatedOffset( QPointF offset, double angle );
%Docstring
Adjusts a marker offset to account for rotation.

:param offset: offset prior to rotation
:param angle: rotation angle in degrees clockwise from north

:return: adjusted offset
%End


};

class QgsLineSymbolLayer : QgsSymbolLayer
{

%TypeHeaderCode
#include "qgssymbollayer.h"
%End
  public:
    virtual void renderPolyline( const QPolygonF &points, QgsSymbolRenderContext &context ) = 0;

    virtual void renderPolygonStroke( const QPolygonF &points, QList<QPolygonF> *rings, QgsSymbolRenderContext &context );

    virtual void setWidth( double width );
    virtual double width() const;

    double offset() const;
    void setOffset( double offset );

    void setWidthUnit( QgsUnitTypes::RenderUnit unit );
%Docstring
Sets the units for the line's width.

:param unit: width units

.. seealso:: :py:func:`widthUnit`
%End

    QgsUnitTypes::RenderUnit widthUnit() const;
%Docstring
Returns the units for the line's width.

.. seealso:: :py:func:`setWidthUnit`
%End

    void setWidthMapUnitScale( const QgsMapUnitScale &scale );
    const QgsMapUnitScale &widthMapUnitScale() const;

    void setOffsetUnit( QgsUnitTypes::RenderUnit unit );
%Docstring
Sets the units for the line's offset.

:param unit: offset units

.. seealso:: :py:func:`offsetUnit`
%End

    QgsUnitTypes::RenderUnit offsetUnit() const;
%Docstring
Returns the units for the line's offset.

.. seealso:: :py:func:`setOffsetUnit`
%End

    void setOffsetMapUnitScale( const QgsMapUnitScale &scale );
    const QgsMapUnitScale &offsetMapUnitScale() const;

    virtual void setOutputUnit( QgsUnitTypes::RenderUnit unit );

    virtual QgsUnitTypes::RenderUnit outputUnit() const;


    virtual void setMapUnitScale( const QgsMapUnitScale &scale );

    virtual QgsMapUnitScale mapUnitScale() const;


    virtual void drawPreviewIcon( QgsSymbolRenderContext &context, QSize size );


    virtual double dxfWidth( const QgsDxfExport &e, QgsSymbolRenderContext &context ) const;


  protected:
    QgsLineSymbolLayer( bool locked = false );

};

class QgsFillSymbolLayer : QgsSymbolLayer
{

%TypeHeaderCode
#include "qgssymbollayer.h"
%End
  public:
    virtual void renderPolygon( const QPolygonF &points, QList<QPolygonF> *rings, QgsSymbolRenderContext &context ) = 0;

    virtual void drawPreviewIcon( QgsSymbolRenderContext &context, QSize size );


    void setAngle( double angle );
    double angle() const;

  protected:
    QgsFillSymbolLayer( bool locked = false );
    void _renderPolygon( QPainter *p, const QPolygonF &points, const QList<QPolygonF> *rings, QgsSymbolRenderContext &context );
%Docstring
Default method to render polygon
%End

};




/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/symbology/qgssymbollayer.h                                  *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
