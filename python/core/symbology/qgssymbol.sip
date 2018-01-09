/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/symbology/qgssymbol.h                                       *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/






typedef QList<QgsSymbolLayer *> QgsSymbolLayerList;

class QgsSymbol
{

%TypeHeaderCode
#include "qgssymbol.h"
%End
%ConvertToSubClassCode
    switch ( sipCpp->type() )
    {
      case QgsSymbol::Marker: sipType = sipType_QgsMarkerSymbol; break;
      case QgsSymbol::Line: sipType = sipType_QgsLineSymbol; break;
      case QgsSymbol::Fill: sipType = sipType_QgsFillSymbol; break;
      default: sipType = 0; break;
    }
%End
  public:

    enum SymbolType
    {
      Marker,
      Line,
      Fill,
      Hybrid
    };

    enum ScaleMethod
    {
      ScaleArea,
      ScaleDiameter
    };


    enum RenderHint
    {
      DynamicRotation,
    };
    typedef QFlags<QgsSymbol::RenderHint> RenderHints;


    virtual ~QgsSymbol();

    static QgsSymbol *defaultSymbol( QgsWkbTypes::GeometryType geomType ) /Factory/;
%Docstring
return new default symbol for specified geometry type
%End

    SymbolType type() const;


    QgsSymbolLayerList symbolLayers();
%Docstring
Returns list of symbol layers contained in the symbol.

:return: symbol layers list

.. versionadded:: 2.7

.. seealso:: :py:func:`symbolLayer`

.. seealso:: :py:func:`symbolLayerCount`
%End

    QgsSymbolLayer *symbolLayer( int layer );
%Docstring
Returns a specific symbol layers contained in the symbol.

:param layer: layer number

:return: corresponding symbol layer

.. versionadded:: 2.7

.. seealso:: :py:func:`symbolLayers`

.. seealso:: :py:func:`symbolLayerCount`
%End

    int symbolLayerCount() const;
%Docstring
Returns total number of symbol layers contained in the symbol.

:return: count of symbol layers

.. versionadded:: 2.7

.. seealso:: :py:func:`symbolLayers`

.. seealso:: :py:func:`symbolLayer`
%End

    bool insertSymbolLayer( int index, QgsSymbolLayer *layer /Transfer/ );
%Docstring
Insert symbol layer to specified index
Ownership will be transferred.

:param index: The index at which the layer should be added
:param layer: The symbol layer to add

:return: True if the layer is added, False if the index or the layer is bad
%End

    bool appendSymbolLayer( QgsSymbolLayer *layer /Transfer/ );
%Docstring
Append symbol layer at the end of the list
Ownership will be transferred.

:param layer: The layer to add

:return: True if the layer is added, False if the layer is bad
%End

    bool deleteSymbolLayer( int index );
%Docstring
delete symbol layer at specified index
%End

    QgsSymbolLayer *takeSymbolLayer( int index ) /TransferBack/;
%Docstring
Remove symbol layer from the list and return pointer to it.
Ownership is handed to the caller.

:param index: The index of the layer to remove

:return: A pointer to the removed layer
%End

    bool changeSymbolLayer( int index, QgsSymbolLayer *layer /Transfer/ );
%Docstring
delete layer at specified index and set a new one
%End

    void startRender( QgsRenderContext &context, const QgsFields &fields = QgsFields() );
%Docstring
Begins the rendering process for the symbol. This must be called before renderFeature(),
and should be followed by a call to stopRender().

:param context: render context which symbol will be drawn using
:param fields: fields for features to be rendered (usually the associated
vector layer's fields). Required for correct calculation of data defined
overrides.

.. seealso:: :py:func:`stopRender`
%End

    void stopRender( QgsRenderContext &context );
%Docstring
Ends the rendering process. This should be called after rendering all desired features.

:param context: render context, must match the context specified when startRender()
was called.

.. seealso:: :py:func:`startRender`
%End

    void setColor( const QColor &color );
    QColor color() const;

    void drawPreviewIcon( QPainter *painter, QSize size, QgsRenderContext *customContext = 0 );
%Docstring
Draw icon of the symbol that occupyies area given by size using the painter.
Optionally custom context may be given in order to get rendering of symbols that use map units right.

.. versionadded:: 2.6
%End

    void exportImage( const QString &path, const QString &format, QSize size );
%Docstring
export symbol as image format. PNG and SVG supported
%End

    QImage asImage( QSize size, QgsRenderContext *customContext = 0 );
%Docstring
Generate symbol as image
%End

    QImage bigSymbolPreviewImage( QgsExpressionContext *expressionContext = 0 );
%Docstring
Returns a large (roughly 100x100 pixel) preview image for the symbol.

:param expressionContext: optional expression context, for evaluation of
data defined symbol properties
%End

    QString dump() const;

    virtual QgsSymbol *clone() const = 0 /Factory/;
%Docstring
Get a deep copy of this symbol.
Needs to be reimplemented by subclasses.
Ownership is transferred to the caller.
%End

    void toSld( QDomDocument &doc, QDomElement &element, QgsStringMap props ) const;

    QgsUnitTypes::RenderUnit outputUnit() const;
%Docstring
Returns the units to use for sizes and widths within the symbol. Individual
symbol layer definitions will interpret this in different ways, e.g., a marker symbol
may use it to specify the units for the marker size, while a line symbol
may use it to specify the units for the line width.

:return: output unit, or QgsUnitTypes.RenderUnknownUnit if the symbol contains mixed units

.. seealso:: :py:func:`setOutputUnit`
%End

    void setOutputUnit( QgsUnitTypes::RenderUnit unit );
%Docstring
Sets the units to use for sizes and widths within the symbol. Individual
symbol definitions will interpret this in different ways, e.g., a marker symbol
may use it to specify the units for the marker size, while a line symbol
may use it to specify the units for the line width.

:param unit: output units

.. seealso:: :py:func:`outputUnit`
%End

    QgsMapUnitScale mapUnitScale() const;
    void setMapUnitScale( const QgsMapUnitScale &scale );

    qreal opacity() const;
%Docstring
Returns the opacity for the symbol.

:return: opacity value between 0 (fully transparent) and 1 (fully opaque)

.. seealso:: :py:func:`setOpacity`
%End

    void setOpacity( qreal opacity );
%Docstring
Sets the ``opacity`` for the symbol.

:param opacity: opacity value between 0 (fully transparent) and 1 (fully opaque)

.. seealso:: :py:func:`opacity`
%End

    void setRenderHints( RenderHints hints );
%Docstring
Sets rendering hint flags for the symbol.

.. seealso:: :py:func:`renderHints`
%End

    RenderHints renderHints() const;
%Docstring
Returns the rendering hint flags for the symbol.

.. seealso:: :py:func:`setRenderHints`
%End

    void setClipFeaturesToExtent( bool clipFeaturesToExtent );
%Docstring
Sets whether features drawn by the symbol should be clipped to the render context's
extent. If this option is enabled then features which are partially outside the extent
will be clipped. This speeds up rendering of the feature, but may have undesirable
side effects for certain symbol types.

:param clipFeaturesToExtent: set to true to enable clipping (defaults to true)

.. versionadded:: 2.9

.. seealso:: :py:func:`clipFeaturesToExtent`
%End

    bool clipFeaturesToExtent() const;
%Docstring
Returns whether features drawn by the symbol will be clipped to the render context's
extent. If this option is enabled then features which are partially outside the extent
will be clipped. This speeds up rendering of the feature, but may have undesirable
side effects for certain symbol types.

:return: true if features will be clipped

.. versionadded:: 2.9

.. seealso:: :py:func:`setClipFeaturesToExtent`
%End

    QSet<QString> usedAttributes( const QgsRenderContext &context ) const;
%Docstring
Return a list of attributes required to render this feature.
This should include any attributes required by the symbology including
the ones required by expressions.
%End

    bool hasDataDefinedProperties() const;
%Docstring
Returns whether the symbol utilizes any data defined properties.

.. versionadded:: 2.12
%End

    void setLayer( const QgsVectorLayer *layer );
%Docstring

.. note::

   the layer will be NULL after stopRender
%End
    const QgsVectorLayer *layer() const;

    void renderFeature( const QgsFeature &feature, QgsRenderContext &context, int layer = -1, bool selected = false, bool drawVertexMarker = false, int currentVertexMarkerType = 0, int currentVertexMarkerSize = 0 );
%Docstring
Render a feature. Before calling this the startRender() method should be called to initialize
the rendering process. After rendering all features stopRender() must be called.
%End

    QgsSymbolRenderContext *symbolRenderContext();
%Docstring
Returns the symbol render context. Only valid between startRender and stopRender calls.

:return: The symbol render context
%End

  protected:
    QgsSymbol( SymbolType type, const QgsSymbolLayerList &layers /Transfer/ ); // can't be instantiated

    static QPointF _getPoint( QgsRenderContext &context, const QgsPoint &point );
%Docstring
Creates a point in screen coordinates from a QgsPoint in map coordinates
%End

    static QPolygonF _getLineString( QgsRenderContext &context, const QgsCurve &curve, bool clipToExtent = true );
%Docstring
Creates a line string in screen coordinates from a QgsCurve in map coordinates
%End

    static QPolygonF _getPolygonRing( QgsRenderContext &context, const QgsCurve &curve, bool clipToExtent );
%Docstring
Creates a polygon ring in screen coordinates from a QgsCurve in map coordinates
%End

    static void _getPolygon( QPolygonF &pts, QList<QPolygonF> &holes, QgsRenderContext &context, const QgsPolygon &polygon, bool clipToExtent = true );
%Docstring
Creates a polygon in screen coordinates from a QgsPolygonXYin map coordinates
%End

    QgsSymbolLayerList cloneLayers() const /Factory/;
%Docstring
Retrieve a cloned list of all layers that make up this symbol.
Ownership is transferred to the caller.
%End

    void renderUsingLayer( QgsSymbolLayer *layer, QgsSymbolRenderContext &context );
%Docstring
Renders a context using a particular symbol layer without passing in a
geometry. This is used as fallback, if the symbol being rendered is not
compatible with the specified layer. In such a case, this method can be
called and will call the layer's rendering method anyway but the
geometry passed to the layer will be empty.
This is required for layers that generate their own geometry from other
information in the rendering context.
%End

    void renderVertexMarker( QPointF pt, QgsRenderContext &context, int currentVertexMarkerType, int currentVertexMarkerSize );
%Docstring
Render editing vertex marker at specified point

.. versionadded:: 2.16
%End





  private:
    QgsSymbol( const QgsSymbol & );
};

QFlags<QgsSymbol::RenderHint> operator|(QgsSymbol::RenderHint f1, QFlags<QgsSymbol::RenderHint> f2);



class QgsSymbolRenderContext
{

%TypeHeaderCode
#include "qgssymbol.h"
%End
  public:

    QgsSymbolRenderContext( QgsRenderContext &c, QgsUnitTypes::RenderUnit u, qreal opacity = 1.0, bool selected = false, QgsSymbol::RenderHints renderHints = 0, const QgsFeature *f = 0, const QgsFields &fields = QgsFields(), const QgsMapUnitScale &mapUnitScale = QgsMapUnitScale() );
%Docstring
Constructor for QgsSymbolRenderContext

:param c:
:param u:
:param opacity: value between 0 (fully transparent) and 1 (fully opaque)
:param selected: set to true if symbol should be drawn in a "selected" state
:param renderHints: flags controlling rendering behavior
:param f:
:param fields:
:param mapUnitScale:
%End


    QgsRenderContext &renderContext();

    void setOriginalValueVariable( const QVariant &value );
%Docstring
Sets the original value variable value for data defined symbology

:param value: value for original value variable. This usually represents the symbol property value
before any data defined overrides have been applied.

.. versionadded:: 2.12
%End

    QgsUnitTypes::RenderUnit outputUnit() const;
%Docstring
Returns the output unit for the context
%End

    void setOutputUnit( QgsUnitTypes::RenderUnit u );
%Docstring
Sets the output unit for the context
%End

    QgsMapUnitScale mapUnitScale() const;
    void setMapUnitScale( const QgsMapUnitScale &scale );

    qreal opacity() const;
%Docstring
Returns the opacity for the symbol.

:return: opacity value between 0 (fully transparent) and 1 (fully opaque)

.. seealso:: :py:func:`setOpacity`
%End

    void setOpacity( qreal opacity );
%Docstring
Sets the ``opacity`` for the symbol.

:param opacity: opacity value between 0 (fully transparent) and 1 (fully opaque)

.. seealso:: :py:func:`opacity`
%End

    bool selected() const;
    void setSelected( bool selected );

    QgsSymbol::RenderHints renderHints() const;
%Docstring
Returns the rendering hint flags for the symbol.

.. seealso:: :py:func:`setRenderHints`
%End

    void setRenderHints( QgsSymbol::RenderHints hints );
%Docstring
Sets rendering hint flags for the symbol.

.. seealso:: :py:func:`renderHints`
%End

    void setFeature( const QgsFeature *f );
    const QgsFeature *feature() const;
%Docstring
Current feature being rendered - may be null
%End

    void setOriginalGeometryType( QgsWkbTypes::GeometryType type );
%Docstring
Sets the geometry type for the original feature geometry being rendered.

.. seealso:: :py:func:`originalGeometryType`

.. versionadded:: 3.0
%End

    QgsWkbTypes::GeometryType originalGeometryType() const;
%Docstring
Returns the geometry type for the original feature geometry being rendered. This can be
useful if symbol layers alter their appearance based on geometry type - eg offsetting a
simple line style will look different if the simple line is rendering a polygon feature
(a closed buffer) vs a line feature (an unclosed offset line).

.. seealso:: :py:func:`originalGeometryType`

.. versionadded:: 3.0
%End

    QgsFields fields() const;
%Docstring
Fields of the layer. Currently only available in startRender() calls
to allow symbols with data-defined properties prepare the expressions
(other times fields() returns null)

.. versionadded:: 2.4
%End

    int geometryPartCount() const;
%Docstring
Part count of current geometry

.. versionadded:: 2.16
%End

    void setGeometryPartCount( int count );
%Docstring
Sets the part count of current geometry

.. versionadded:: 2.16
%End

    int geometryPartNum() const;
%Docstring
Part number of current geometry

.. versionadded:: 2.16
%End

    void setGeometryPartNum( int num );
%Docstring
Sets the part number of current geometry

.. versionadded:: 2.16
%End

    double outputLineWidth( double width ) const;
    double outputPixelSize( double size ) const;


    QgsExpressionContextScope *expressionContextScope();
%Docstring
This scope is always available when a symbol of this type is being rendered.

:return: An expression scope for details about this symbol
%End

    void setExpressionContextScope( QgsExpressionContextScope *contextScope /Transfer/ );
%Docstring
Set an expression scope for this symbol.

Will take ownership.

:param contextScope: An expression scope for details about this symbol
%End

  private:
    QgsSymbolRenderContext( const QgsSymbolRenderContext &rh ) ;
};





class QgsMarkerSymbol : QgsSymbol
{

%TypeHeaderCode
#include "qgssymbol.h"
%End
  public:

    static QgsMarkerSymbol *createSimple( const QgsStringMap &properties ) /Factory/;
%Docstring
Create a marker symbol with one symbol layer: SimpleMarker with specified properties.
This is a convenience method for easier creation of marker symbols.
%End

    QgsMarkerSymbol( const QgsSymbolLayerList &layers /Transfer/ = QgsSymbolLayerList() );

    void setAngle( double symbolAngle );
%Docstring
Sets the angle for the whole symbol. Individual symbol layer sizes
will be rotated to maintain their current relative angle to the whole symbol angle.

:param symbolAngle: new symbol angle

.. seealso:: :py:func:`angle`
%End

    double angle() const;
%Docstring
Returns the marker angle for the whole symbol. Note that for symbols with
multiple symbol layers, this will correspond just to the angle of
the first symbol layer.

.. versionadded:: 2.16

.. seealso:: :py:func:`setAngle`
%End

    void setDataDefinedAngle( const QgsProperty &property );
%Docstring
Set data defined angle for whole symbol (including all symbol layers).

.. versionadded:: 3.0

.. seealso:: :py:func:`dataDefinedAngle`
%End

    QgsProperty dataDefinedAngle() const;
%Docstring
Returns data defined angle for whole symbol (including all symbol layers).

:return: data defined angle, or invalid property if angle is not set
at the marker level.

.. versionadded:: 3.0

.. seealso:: :py:func:`setDataDefinedAngle`
%End

    void setLineAngle( double lineAngle );
%Docstring
Sets the line angle modification for the symbol's angle. This angle is added to
the marker's rotation and data defined rotation before rendering the symbol, and
is usually used for orienting symbols to match a line's angle.

:param lineAngle: Angle in degrees, valid values are between 0 and 360

.. versionadded:: 2.9
%End

    void setSize( double size );
%Docstring
Sets the size for the whole symbol. Individual symbol layer sizes
will be scaled to maintain their current relative size to the whole symbol size.

:param size: new symbol size

.. seealso:: :py:func:`size`

.. seealso:: :py:func:`setSizeUnit`

.. seealso:: :py:func:`setSizeMapUnitScale`
%End

    double size() const;
%Docstring
Returns the size for the whole symbol, which is the maximum size of
all marker symbol layers in the symbol.

.. seealso:: :py:func:`setSize`

.. seealso:: :py:func:`sizeUnit`

.. seealso:: :py:func:`sizeMapUnitScale`
%End

    void setSizeUnit( QgsUnitTypes::RenderUnit unit );
%Docstring
Sets the size units for the whole symbol (including all symbol layers).

:param unit: size units

.. versionadded:: 2.16

.. seealso:: :py:func:`sizeUnit`

.. seealso:: :py:func:`setSizeMapUnitScale`

.. seealso:: :py:func:`setSize`
%End

    QgsUnitTypes::RenderUnit sizeUnit() const;
%Docstring
Returns the size units for the whole symbol (including all symbol layers).

:return: size units, or mixed units if symbol layers have different units

.. versionadded:: 2.16

.. seealso:: :py:func:`setSizeUnit`

.. seealso:: :py:func:`sizeMapUnitScale`

.. seealso:: :py:func:`size`
%End

    void setSizeMapUnitScale( const QgsMapUnitScale &scale );
%Docstring
Sets the size map unit scale for the whole symbol (including all symbol layers).

:param scale: map unit scale

.. versionadded:: 2.16

.. seealso:: :py:func:`sizeMapUnitScale`

.. seealso:: :py:func:`setSizeUnit`

.. seealso:: :py:func:`setSize`
%End

    QgsMapUnitScale sizeMapUnitScale() const;
%Docstring
Returns the size map unit scale for the whole symbol. Note that for symbols with
multiple symbol layers, this will correspond just to the map unit scale
for the first symbol layer.

.. versionadded:: 2.16

.. seealso:: :py:func:`setSizeMapUnitScale`

.. seealso:: :py:func:`sizeUnit`

.. seealso:: :py:func:`size`
%End

    void setDataDefinedSize( const QgsProperty &property );
%Docstring
Set data defined size for whole symbol (including all symbol layers).

.. versionadded:: 3.0

.. seealso:: :py:func:`dataDefinedSize`
%End

    QgsProperty dataDefinedSize() const;
%Docstring
Returns data defined size for whole symbol (including all symbol layers).

:return: data defined size, or invalid property if size is not set
at the marker level.

.. versionadded:: 3.0

.. seealso:: :py:func:`setDataDefinedSize`
%End

    void setScaleMethod( QgsSymbol::ScaleMethod scaleMethod );
    ScaleMethod scaleMethod();

    void renderPoint( QPointF point, const QgsFeature *f, QgsRenderContext &context, int layer = -1, bool selected = false );

    QRectF bounds( QPointF point, QgsRenderContext &context, const QgsFeature &feature = QgsFeature() ) const;
%Docstring
Returns the approximate bounding box of the marker symbol, which includes the bounding box
of all symbol layers for the symbol. It is recommended to use this method only between startRender()
and stopRender() calls, or data defined rotation and offset will not be correctly calculated.

:param point: location of rendered point in painter units
:param context: render context
:param feature: feature being rendered at point (optional). If not specified, the bounds calculation will not
include data defined parameters such as offset and rotation

:return: approximate symbol bounds, in painter units

.. versionadded:: 2.14
%End

    virtual QgsMarkerSymbol *clone() const /Factory/;


};


class QgsLineSymbol : QgsSymbol
{

%TypeHeaderCode
#include "qgssymbol.h"
%End
  public:

    static QgsLineSymbol *createSimple( const QgsStringMap &properties ) /Factory/;
%Docstring
Create a line symbol with one symbol layer: SimpleLine with specified properties.
This is a convenience method for easier creation of line symbols.
%End

    QgsLineSymbol( const QgsSymbolLayerList &layers /Transfer/ = QgsSymbolLayerList() );

    void setWidth( double width );
    double width() const;

    void setDataDefinedWidth( const QgsProperty &property );
%Docstring
Set data defined width for whole symbol (including all symbol layers).

.. seealso:: :py:func:`dataDefinedWidth`

.. versionadded:: 3.0
%End

    QgsProperty dataDefinedWidth() const;
%Docstring
Returns data defined width for whole symbol (including all symbol layers).

:return: data defined width, or invalid property if size is not set
at the line level. Caller takes responsibility for deleting the returned object.

.. versionadded:: 3.0

.. seealso:: :py:func:`setDataDefinedWidth`
%End

    void renderPolyline( const QPolygonF &points, const QgsFeature *f, QgsRenderContext &context, int layer = -1, bool selected = false );

    virtual QgsLineSymbol *clone() const /Factory/;


};


class QgsFillSymbol : QgsSymbol
{

%TypeHeaderCode
#include "qgssymbol.h"
%End
  public:

    static QgsFillSymbol *createSimple( const QgsStringMap &properties ) /Factory/;
%Docstring
Create a fill symbol with one symbol layer: SimpleFill with specified properties.
This is a convenience method for easier creation of fill symbols.
%End

    QgsFillSymbol( const QgsSymbolLayerList &layers /Transfer/ = QgsSymbolLayerList() );
    void setAngle( double angle );
    void renderPolygon( const QPolygonF &points, QList<QPolygonF> *rings, const QgsFeature *f, QgsRenderContext &context, int layer = -1, bool selected = false );

    virtual QgsFillSymbol *clone() const /Factory/;


};


/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/symbology/qgssymbol.h                                       *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
