/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/symbology/qgsmarkersymbollayer.h                            *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/





class QgsSimpleMarkerSymbolLayerBase : QgsMarkerSymbolLayer
{
%Docstring
 Abstract base class for simple marker symbol layers. Handles creation of the symbol shapes but
leaves the actual drawing of the symbols to subclasses.

.. versionadded:: 2.16
%End

%TypeHeaderCode
#include "qgsmarkersymbollayer.h"
%End
  public:

    enum Shape
    {
      Square,
      Diamond,
      Pentagon,
      Hexagon,
      Triangle,
      EquilateralTriangle,
      Star,
      Arrow,
      Circle,
      Cross,
      CrossFill,
      Cross2,
      Line,
      ArrowHead,
      ArrowHeadFilled,
      SemiCircle,
      ThirdCircle,
      QuarterCircle,
      QuarterSquare,
      HalfSquare,
      DiagonalHalfSquare,
      RightHalfTriangle,
      LeftHalfTriangle,
    };

    static QList< QgsSimpleMarkerSymbolLayerBase::Shape > availableShapes();
%Docstring
Returns a list of all available shape types.
%End

    static bool shapeIsFilled( QgsSimpleMarkerSymbolLayerBase::Shape shape );
%Docstring
Returns true if a symbol shape has a fill.

:param shape: shape to test

:return: true if shape uses a fill, or false if shape uses lines only
%End

    QgsSimpleMarkerSymbolLayerBase( QgsSimpleMarkerSymbolLayerBase::Shape shape = Circle,
                                    double size = DEFAULT_SIMPLEMARKER_SIZE,
                                    double angle = DEFAULT_SIMPLEMARKER_ANGLE,
                                    QgsSymbol::ScaleMethod scaleMethod = DEFAULT_SCALE_METHOD );
%Docstring
Constructor for QgsSimpleMarkerSymbolLayerBase.

:param shape: symbol shape for markers
:param size: symbol size (in mm)
:param angle: symbol rotation angle
:param scaleMethod: scaling method for data defined scaling
%End

    QgsSimpleMarkerSymbolLayerBase::Shape shape() const;
%Docstring
Returns the shape for the rendered marker symbol.

.. seealso:: :py:func:`setShape`
%End

    void setShape( QgsSimpleMarkerSymbolLayerBase::Shape shape );
%Docstring
Sets the rendered marker shape.

:param shape: new marker shape

.. seealso:: :py:func:`shape`
%End

    static QgsSimpleMarkerSymbolLayerBase::Shape decodeShape( const QString &name, bool *ok = 0 );
%Docstring
Attempts to decode a string representation of a shape name to the corresponding
shape.

:param name: encoded shape name
:param ok: if specified, will be set to true if shape was successfully decoded

:return: decoded name

.. seealso:: :py:func:`encodeShape`
%End

    static QString encodeShape( QgsSimpleMarkerSymbolLayerBase::Shape shape );
%Docstring
Encodes a shape to its string representation.

:param shape: shape to encode

:return: encoded string

.. seealso:: :py:func:`decodeShape`
%End

    virtual void startRender( QgsSymbolRenderContext &context );

    virtual void stopRender( QgsSymbolRenderContext &context );

    virtual void renderPoint( QPointF point, QgsSymbolRenderContext &context );

    virtual QRectF bounds( QPointF point, QgsSymbolRenderContext &context );


  protected:

    bool prepareMarkerShape( Shape shape );
%Docstring
Prepares the layer for drawing the specified shape (QPolygonF version)

.. note::

   not available in Python bindings
%End

    bool prepareMarkerPath( Shape symbol );
%Docstring
Prepares the layer for drawing the specified shape (QPainterPath version)

.. note::

   not available in Python bindings
%End

    bool shapeToPolygon( Shape shape, QPolygonF &polygon ) const;
%Docstring
Creates a polygon representing the specified shape.

:param shape: shape to create
:param polygon: destination polygon for shape

:return: true if shape was successfully stored in polygon

.. note::

   not available in Python bindings
%End

    double calculateSize( QgsSymbolRenderContext &context, bool &hasDataDefinedSize ) const;
%Docstring
Calculates the desired size of the marker, considering data defined size overrides.

:param context: symbol render context
:param hasDataDefinedSize: will be set to true if marker uses data defined sizes

:return: marker size, in original size units

.. note::

   not available in Python bindings
%End

    void calculateOffsetAndRotation( QgsSymbolRenderContext &context, double scaledSize, bool &hasDataDefinedRotation, QPointF &offset, double &angle ) const;
%Docstring
Calculates the marker offset and rotation.

:param context: symbol render context
:param scaledSize: size of symbol to render
:param hasDataDefinedRotation: will be set to true if marker has data defined rotation
:param offset: will be set to calculated marker offset (in painter units)
:param angle: will be set to calculated marker angle

.. note::

   not available in Python bindings
%End




  private:
    virtual void draw( QgsSymbolRenderContext &context, QgsSimpleMarkerSymbolLayerBase::Shape shape, const QPolygonF &polygon, const QPainterPath &path ) = 0 ;
%Docstring
Derived classes must implement draw() to handle drawing the generated shape onto the painter surface.

:param context: symbol render context
:param shape: shape to draw
:param polygon: polygon representing transformed marker shape. May be empty, in which case the shape will be specified
in the path argument.
:param path: transformed painter path representing shape to draw
%End
};

class QgsSimpleMarkerSymbolLayer : QgsSimpleMarkerSymbolLayerBase
{
%Docstring
 Simple marker symbol layer, consisting of a rendered shape with solid fill color and an stroke.
%End

%TypeHeaderCode
#include "qgsmarkersymbollayer.h"
%End
  public:

    QgsSimpleMarkerSymbolLayer( QgsSimpleMarkerSymbolLayerBase::Shape shape = Circle,
                                double size = DEFAULT_SIMPLEMARKER_SIZE,
                                double angle = DEFAULT_SIMPLEMARKER_ANGLE,
                                QgsSymbol::ScaleMethod scaleMethod = DEFAULT_SCALE_METHOD,
                                const QColor &color = DEFAULT_SIMPLEMARKER_COLOR,
                                const QColor &strokeColor = DEFAULT_SIMPLEMARKER_BORDERCOLOR,
                                Qt::PenJoinStyle penJoinStyle = DEFAULT_SIMPLEMARKER_JOINSTYLE );
%Docstring
Constructor for QgsSimpleMarkerSymbolLayer.

:param shape: symbol shape
:param size: symbol size (in mm)
:param angle: symbol rotation angle
:param scaleMethod: scaling method for data defined scaling
:param color: fill color for symbol
:param strokeColor: stroke color for symbol
:param penJoinStyle: join style for stroke pen
%End


    static QgsSymbolLayer *create( const QgsStringMap &properties = QgsStringMap() ) /Factory/;
%Docstring
Creates a new QgsSimpleMarkerSymbolLayer.

:param properties: a property map containing symbol properties (see properties())

:return: new QgsSimpleMarkerSymbolLayer
%End

    static QgsSymbolLayer *createFromSld( QDomElement &element ) /Factory/;
%Docstring
Creates a new QgsSimpleMarkerSymbolLayer from an SLD XML element.

:param element: XML element containing SLD definition of symbol

:return: new QgsSimpleMarkerSymbolLayer
%End


    virtual QString layerType() const;

    virtual void startRender( QgsSymbolRenderContext &context );

    virtual void renderPoint( QPointF point, QgsSymbolRenderContext &context );

    virtual QgsStringMap properties() const;

    virtual QgsSimpleMarkerSymbolLayer *clone() const /Factory/;

    virtual void writeSldMarker( QDomDocument &doc, QDomElement &element, const QgsStringMap &props ) const;

    virtual QString ogrFeatureStyle( double mmScaleFactor, double mapUnitScaleFactor ) const;

    virtual bool writeDxf( QgsDxfExport &e, double mmMapUnitScaleFactor, const QString &layerName, QgsSymbolRenderContext &context, QPointF shift = QPointF( 0.0, 0.0 ) ) const;

    virtual void setOutputUnit( QgsUnitTypes::RenderUnit unit );

    virtual QgsUnitTypes::RenderUnit outputUnit() const;

    virtual void setMapUnitScale( const QgsMapUnitScale &scale );

    virtual QgsMapUnitScale mapUnitScale() const;

    virtual QRectF bounds( QPointF point, QgsSymbolRenderContext &context );

    virtual QColor fillColor() const;
    virtual void setFillColor( const QColor &color );
    virtual void setColor( const QColor &color );

    virtual QColor color() const;



    virtual QColor strokeColor() const;
%Docstring
Returns the marker's stroke color.

.. seealso:: :py:func:`setStrokeColor`

.. seealso:: :py:func:`strokeStyle`

.. seealso:: :py:func:`penJoinStyle`
%End

    virtual void setStrokeColor( const QColor &color );
%Docstring
Sets the marker's stroke color.

:param color: stroke color

.. seealso:: :py:func:`strokeColor`

.. seealso:: :py:func:`setStrokeStyle`

.. seealso:: :py:func:`setPenJoinStyle`
%End

    Qt::PenStyle strokeStyle() const;
%Docstring
Returns the marker's stroke style (e.g., solid, dashed, etc)

.. versionadded:: 2.4

.. seealso:: :py:func:`setStrokeStyle`

.. seealso:: :py:func:`strokeColor`

.. seealso:: :py:func:`penJoinStyle`
%End

    void setStrokeStyle( Qt::PenStyle strokeStyle );
%Docstring
Sets the marker's stroke style (e.g., solid, dashed, etc)

:param strokeStyle: style

.. versionadded:: 2.4

.. seealso:: :py:func:`strokeStyle`

.. seealso:: :py:func:`setStrokeColor`

.. seealso:: :py:func:`setPenJoinStyle`
%End

    Qt::PenJoinStyle penJoinStyle() const;
%Docstring
Returns the marker's stroke join style (e.g., miter, bevel, etc).

.. versionadded:: 2.16

.. seealso:: :py:func:`setPenJoinStyle`

.. seealso:: :py:func:`strokeColor`

.. seealso:: :py:func:`strokeStyle`
%End

    void setPenJoinStyle( Qt::PenJoinStyle style );
%Docstring
Sets the marker's stroke join style (e.g., miter, bevel, etc).

:param style: join style

.. versionadded:: 2.16

.. seealso:: :py:func:`penJoinStyle`

.. seealso:: :py:func:`setStrokeColor`

.. seealso:: :py:func:`setStrokeStyle`
%End

    double strokeWidth() const;
%Docstring
Returns the width of the marker's stroke.

.. seealso:: :py:func:`setStrokeWidth`

.. seealso:: :py:func:`strokeWidthUnit`

.. seealso:: :py:func:`strokeWidthMapUnitScale`
%End

    void setStrokeWidth( double w );
%Docstring
Sets the width of the marker's stroke.

:param w: stroke width. See strokeWidthUnit() for units.

.. seealso:: :py:func:`strokeWidth`

.. seealso:: :py:func:`setStrokeWidthUnit`

.. seealso:: :py:func:`setStrokeWidthMapUnitScale`
%End

    void setStrokeWidthUnit( QgsUnitTypes::RenderUnit u );
%Docstring
Sets the unit for the width of the marker's stroke.

:param u: stroke width unit

.. seealso:: :py:func:`strokeWidthUnit`

.. seealso:: :py:func:`setStrokeWidth`

.. seealso:: :py:func:`setStrokeWidthMapUnitScale`
%End

    QgsUnitTypes::RenderUnit strokeWidthUnit() const;
%Docstring
Returns the unit for the width of the marker's stroke.

.. seealso:: :py:func:`setStrokeWidthUnit`

.. seealso:: :py:func:`strokeWidth`

.. seealso:: :py:func:`strokeWidthMapUnitScale`
%End

    void setStrokeWidthMapUnitScale( const QgsMapUnitScale &scale );
%Docstring
Sets the map scale for the width of the marker's stroke.

:param scale: stroke width map unit scale

.. seealso:: :py:func:`strokeWidthMapUnitScale`

.. seealso:: :py:func:`setStrokeWidth`

.. seealso:: :py:func:`setStrokeWidthUnit`
%End

    const QgsMapUnitScale &strokeWidthMapUnitScale() const;
%Docstring
Returns the map scale for the width of the marker's stroke.

.. seealso:: :py:func:`setStrokeWidthMapUnitScale`

.. seealso:: :py:func:`strokeWidth`

.. seealso:: :py:func:`strokeWidthUnit`
%End

  protected:

    void drawMarker( QPainter *p, QgsSymbolRenderContext &context );
%Docstring
Draws the marker shape in the specified painter.

:param p: destination QPainter
:param context: symbol context

.. note::

   this method does not handle setting the painter pen or brush to match the symbol's fill or stroke
%End

    bool prepareCache( QgsSymbolRenderContext &context );
%Docstring
Prepares cache image

:return: true in case of success, false if cache image size too large
%End




  private:
    virtual void draw( QgsSymbolRenderContext &context, QgsSimpleMarkerSymbolLayerBase::Shape shape, const QPolygonF &polygon, const QPainterPath &path ) ;

};

class QgsFilledMarkerSymbolLayer : QgsSimpleMarkerSymbolLayerBase
{
%Docstring
 Filled marker symbol layer, consisting of a shape which is rendered using a :py:class:`QgsFillSymbol`. This allows
the symbol to support advanced styling of the interior and stroke of the shape.

.. versionadded:: 2.16
%End

%TypeHeaderCode
#include "qgsmarkersymbollayer.h"
%End
  public:

    QgsFilledMarkerSymbolLayer( QgsSimpleMarkerSymbolLayerBase::Shape shape = Circle,
                                double size = DEFAULT_SIMPLEMARKER_SIZE,
                                double angle = DEFAULT_SIMPLEMARKER_ANGLE,
                                QgsSymbol::ScaleMethod scaleMethod = DEFAULT_SCALE_METHOD );
%Docstring
Constructor for QgsFilledMarkerSymbolLayer.

:param shape: symbol shape
:param size: symbol size (in mm)
:param angle: symbol rotation angle
:param scaleMethod: size scaling method
%End

    static QgsSymbolLayer *create( const QgsStringMap &properties = QgsStringMap() ) /Factory/;
%Docstring
Creates a new QgsFilledMarkerSymbolLayer.

:param properties: a property map containing symbol properties (see properties())

:return: new QgsFilledMarkerSymbolLayer
%End

    virtual QString layerType() const;

    virtual void startRender( QgsSymbolRenderContext &context );

    virtual void stopRender( QgsSymbolRenderContext &context );

    virtual QgsStringMap properties() const;

    virtual QgsFilledMarkerSymbolLayer *clone() const /Factory/;

    virtual QgsSymbol *subSymbol();

    virtual bool setSubSymbol( QgsSymbol *symbol /Transfer/ );

    virtual double estimateMaxBleed( const QgsRenderContext &context ) const;

    virtual QSet<QString> usedAttributes( const QgsRenderContext &context ) const;

    virtual void setColor( const QColor &c );

    virtual QColor color() const;


  private:
    QgsFilledMarkerSymbolLayer( const QgsFilledMarkerSymbolLayer & );
    virtual void draw( QgsSymbolRenderContext &context, QgsSimpleMarkerSymbolLayerBase::Shape shape, const QPolygonF &polygon, const QPainterPath &path ) ;

};



class QgsSvgMarkerSymbolLayer : QgsMarkerSymbolLayer
{

%TypeHeaderCode
#include "qgsmarkersymbollayer.h"
%End
  public:
    QgsSvgMarkerSymbolLayer( const QString &path,
                             double size = DEFAULT_SVGMARKER_SIZE,
                             double angle = DEFAULT_SVGMARKER_ANGLE,
                             QgsSymbol::ScaleMethod scaleMethod = DEFAULT_SCALE_METHOD );
%Docstring
Constructs SVG marker symbol layer with picture from given absolute path to a SVG file
%End


    static QgsSymbolLayer *create( const QgsStringMap &properties = QgsStringMap() ) /Factory/;
    static QgsSymbolLayer *createFromSld( QDomElement &element ) /Factory/;

    static void resolvePaths( QgsStringMap &properties, const QgsPathResolver &pathResolver, bool saving );
%Docstring
Turns relative paths in properties map to absolute when reading and vice versa when writing.
Used internally when reading/writing symbols.

.. versionadded:: 3.0
%End


    virtual QString layerType() const;


    virtual void startRender( QgsSymbolRenderContext &context );


    virtual void stopRender( QgsSymbolRenderContext &context );


    virtual void renderPoint( QPointF point, QgsSymbolRenderContext &context );


    virtual QgsStringMap properties() const;


    virtual QgsSvgMarkerSymbolLayer *clone() const /Factory/;


    virtual void writeSldMarker( QDomDocument &doc, QDomElement &element, const QgsStringMap &props ) const;


    QString path() const;
%Docstring
Returns the marker SVG path.

.. seealso:: :py:func:`setPath`
%End

    void setPath( const QString &path );
%Docstring
Set the marker SVG path.

:param path: SVG path

.. seealso:: :py:func:`path`
%End

    double defaultAspectRatio() const;
%Docstring
Returns the default marker aspect ratio between width and height, 0 if not yet calculated.

.. seealso:: :py:func:`updateDefaultAspectRatio`
%End

    double updateDefaultAspectRatio();
%Docstring
Calculates the default marker aspect ratio between width and height.

:return: the default aspect ratio value

.. seealso:: :py:func:`defaultAspectRatio`
%End

    bool preservedAspectRatio() const;
%Docstring
Returns the preserved aspect ratio value, true if fixed aspect ratio has been lower or equal to 0.

.. seealso:: :py:func:`setPreservedAspectRatio`
%End

    bool setPreservedAspectRatio( bool par );
%Docstring
Set preserved the marker aspect ratio between width and height.

:param par: Preserved Aspect Ratio

:return: the preserved aspect ratio value, true if fixed aspect ratio has been lower or equal to 0

.. seealso:: :py:func:`preservedAspectRatio`
%End

    double fixedAspectRatio() const;
%Docstring
Returns the marker aspect ratio between width and height to be used in rendering,
if the value set is lower or equal to 0 the aspect ratio will be preserved in rendering

.. seealso:: :py:func:`setFixedAspectRatio`
%End

    void setFixedAspectRatio( double ratio );
%Docstring
Set the marker aspect ratio between width and height to be used in rendering,
if the value set is lower or equal to 0 the aspect ratio will be preserved in rendering

:param ratio: Fixed Aspect Ratio

.. seealso:: :py:func:`fixedAspectRatio`
%End

    virtual QColor fillColor() const;
    virtual void setFillColor( const QColor &color );

    virtual QColor strokeColor() const;
    virtual void setStrokeColor( const QColor &c );

    double strokeWidth() const;
    void setStrokeWidth( double w );

    void setStrokeWidthUnit( QgsUnitTypes::RenderUnit unit );
%Docstring
Sets the units for the stroke width.

:param unit: width units

.. seealso:: :py:func:`strokeWidthUnit`
%End

    QgsUnitTypes::RenderUnit strokeWidthUnit() const;
%Docstring
Returns the units for the stroke width.

.. seealso:: :py:func:`strokeWidthUnit`
%End

    void setStrokeWidthMapUnitScale( const QgsMapUnitScale &scale );
    const QgsMapUnitScale &strokeWidthMapUnitScale() const;

    virtual void setOutputUnit( QgsUnitTypes::RenderUnit unit );

    virtual QgsUnitTypes::RenderUnit outputUnit() const;


    virtual void setMapUnitScale( const QgsMapUnitScale &scale );

    virtual QgsMapUnitScale mapUnitScale() const;


    virtual bool writeDxf( QgsDxfExport &e, double mmMapUnitScaleFactor, const QString &layerName, QgsSymbolRenderContext &context, QPointF shift = QPointF( 0.0, 0.0 ) ) const;


    virtual QRectF bounds( QPointF point, QgsSymbolRenderContext &context );


  protected:

    double calculateAspectRatio( QgsSymbolRenderContext &context, double scaledSize, bool &hasDataDefinedAspectRatio ) const;
%Docstring
Calculates the marker aspect ratio between width and height.

:param context: symbol render context
:param scaledSize: size of symbol to render
:param hasDataDefinedAspectRatio: will be set to true if marker has data defined aspectRatio

.. note::

   not available in Python bindings
%End



};





class QgsFontMarkerSymbolLayer : QgsMarkerSymbolLayer
{

%TypeHeaderCode
#include "qgsmarkersymbollayer.h"
%End
  public:
    QgsFontMarkerSymbolLayer( const QString &fontFamily = DEFAULT_FONTMARKER_FONT,
                              QChar chr = DEFAULT_FONTMARKER_CHR,
                              double pointSize = DEFAULT_FONTMARKER_SIZE,
                              const QColor &color = DEFAULT_FONTMARKER_COLOR,
                              double angle = DEFAULT_FONTMARKER_ANGLE );

    ~QgsFontMarkerSymbolLayer();


    static QgsSymbolLayer *create( const QgsStringMap &properties = QgsStringMap() ) /Factory/;
    static QgsSymbolLayer *createFromSld( QDomElement &element ) /Factory/;


    virtual QString layerType() const;


    virtual void startRender( QgsSymbolRenderContext &context );


    virtual void stopRender( QgsSymbolRenderContext &context );


    virtual void renderPoint( QPointF point, QgsSymbolRenderContext &context );


    virtual QgsStringMap properties() const;


    virtual QgsFontMarkerSymbolLayer *clone() const /Factory/;


    virtual void writeSldMarker( QDomDocument &doc, QDomElement &element, const QgsStringMap &props ) const;



    QString fontFamily() const;
    void setFontFamily( const QString &family );

    QChar character() const;
    void setCharacter( QChar ch );

    virtual QColor strokeColor() const;
    virtual void setStrokeColor( const QColor &color );

    double strokeWidth() const;
%Docstring
Get stroke width.

.. versionadded:: 2.16
%End

    void setStrokeWidth( double width );
%Docstring
Set stroke width.

.. versionadded:: 2.16
%End

    QgsUnitTypes::RenderUnit strokeWidthUnit() const;
%Docstring
Get stroke width unit.

.. versionadded:: 2.16
%End

    void setStrokeWidthUnit( QgsUnitTypes::RenderUnit unit );
%Docstring
Set stroke width unit.

.. versionadded:: 2.16
%End

    const QgsMapUnitScale &strokeWidthMapUnitScale() const;
%Docstring
Get stroke width map unit scale.

.. versionadded:: 2.16
%End

    void setStrokeWidthMapUnitScale( const QgsMapUnitScale &scale );
%Docstring
Set stroke width map unit scale.

.. versionadded:: 2.16
%End

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

    virtual QRectF bounds( QPointF point, QgsSymbolRenderContext &context );


  protected:



};




/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/symbology/qgsmarkersymbollayer.h                            *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
