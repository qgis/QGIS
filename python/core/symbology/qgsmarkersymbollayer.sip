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
 :rtype: list of QgsSimpleMarkerSymbolLayerBase.Shape
%End

    static bool shapeIsFilled( QgsSimpleMarkerSymbolLayerBase::Shape shape );
%Docstring
 Returns true if a symbol shape has a fill.
 \param shape shape to test
 :return: true if shape uses a fill, or false if shape uses lines only
 :rtype: bool
%End

    QgsSimpleMarkerSymbolLayerBase( QgsSimpleMarkerSymbolLayerBase::Shape shape = Circle,
                                    double size = DEFAULT_SIMPLEMARKER_SIZE,
                                    double angle = DEFAULT_SIMPLEMARKER_ANGLE,
                                    QgsSymbol::ScaleMethod scaleMethod = DEFAULT_SCALE_METHOD );
%Docstring
 Constructor for QgsSimpleMarkerSymbolLayerBase.
 \param shape symbol shape for markers
 \param size symbol size (in mm)
 \param angle symbol rotation angle
 \param scaleMethod scaling method for data defined scaling
%End

    QgsSimpleMarkerSymbolLayerBase::Shape shape() const;
%Docstring
 Returns the shape for the rendered marker symbol.
.. seealso:: setShape()
 :rtype: QgsSimpleMarkerSymbolLayerBase.Shape
%End

    void setShape( QgsSimpleMarkerSymbolLayerBase::Shape shape );
%Docstring
 Sets the rendered marker shape.
 \param shape new marker shape
.. seealso:: shape()
%End

    static QgsSimpleMarkerSymbolLayerBase::Shape decodeShape( const QString &name, bool *ok = 0 );
%Docstring
 Attempts to decode a string representation of a shape name to the corresponding
 shape.
 \param name encoded shape name
 \param ok if specified, will be set to true if shape was successfully decoded
 :return: decoded name
.. seealso:: encodeShape()
 :rtype: QgsSimpleMarkerSymbolLayerBase.Shape
%End

    static QString encodeShape( QgsSimpleMarkerSymbolLayerBase::Shape shape );
%Docstring
 Encodes a shape to its string representation.
 \param shape shape to encode
 :return: encoded string
.. seealso:: decodeShape()
 :rtype: str
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
 :rtype: bool
%End

    bool prepareMarkerPath( Shape symbol );
%Docstring
 Prepares the layer for drawing the specified shape (QPainterPath version)
.. note::

   not available in Python bindings
 :rtype: bool
%End

    bool shapeToPolygon( Shape shape, QPolygonF &polygon ) const;
%Docstring
 Creates a polygon representing the specified shape.
 \param shape shape to create
 \param polygon destination polygon for shape
 :return: true if shape was successfully stored in polygon
.. note::

   not available in Python bindings
 :rtype: bool
%End

    double calculateSize( QgsSymbolRenderContext &context, bool &hasDataDefinedSize ) const;
%Docstring
 Calculates the desired size of the marker, considering data defined size overrides.
 \param context symbol render context
 \param hasDataDefinedSize will be set to true if marker uses data defined sizes
 :return: marker size, in original size units
.. note::

   not available in Python bindings
 :rtype: float
%End

    void calculateOffsetAndRotation( QgsSymbolRenderContext &context, double scaledSize, bool &hasDataDefinedRotation, QPointF &offset, double &angle ) const;
%Docstring
 Calculates the marker offset and rotation.
 \param context symbol render context
 \param scaledSize size of symbol to render
 \param hasDataDefinedRotation will be set to true if marker has data defined rotation
 \param offset will be set to calculated marker offset (in painter units)
 \param angle will be set to calculated marker angle
.. note::

   not available in Python bindings
%End




  private:
    virtual void draw( QgsSymbolRenderContext &context, QgsSimpleMarkerSymbolLayerBase::Shape shape, const QPolygonF &polygon, const QPainterPath &path ) = 0 ;
%Docstring
 Derived classes must implement draw() to handle drawing the generated shape onto the painter surface.
 \param context symbol render context
 \param shape shape to draw
 \param polygon polygon representing transformed marker shape. May be empty, in which case the shape will be specified
 in the path argument.
 \param path transformed painter path representing shape to draw
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
 \param shape symbol shape
 \param size symbol size (in mm)
 \param angle symbol rotation angle
 \param scaleMethod scaling method for data defined scaling
 \param color fill color for symbol
 \param strokeColor stroke color for symbol
 \param penJoinStyle join style for stroke pen
%End


    static QgsSymbolLayer *create( const QgsStringMap &properties = QgsStringMap() ) /Factory/;
%Docstring
 Creates a new QgsSimpleMarkerSymbolLayer.
 \param properties a property map containing symbol properties (see properties())
 :return: new QgsSimpleMarkerSymbolLayer
 :rtype: QgsSymbolLayer
%End

    static QgsSymbolLayer *createFromSld( QDomElement &element ) /Factory/;
%Docstring
 Creates a new QgsSimpleMarkerSymbolLayer from an SLD XML element.
 \param element XML element containing SLD definition of symbol
 :return: new QgsSimpleMarkerSymbolLayer
 :rtype: QgsSymbolLayer
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
.. seealso:: setStrokeColor()
.. seealso:: strokeStyle()
.. seealso:: penJoinStyle()
 :rtype: QColor
%End

    virtual void setStrokeColor( const QColor &color );
%Docstring
 Sets the marker's stroke color.
 \param color stroke color
.. seealso:: strokeColor()
.. seealso:: setStrokeStyle()
.. seealso:: setPenJoinStyle()
%End

    Qt::PenStyle strokeStyle() const;
%Docstring
 Returns the marker's stroke style (e.g., solid, dashed, etc)
.. versionadded:: 2.4
.. seealso:: setStrokeStyle()
.. seealso:: strokeColor()
.. seealso:: penJoinStyle()
 :rtype: Qt.PenStyle
%End

    void setStrokeStyle( Qt::PenStyle strokeStyle );
%Docstring
 Sets the marker's stroke style (e.g., solid, dashed, etc)
 \param strokeStyle style
.. versionadded:: 2.4
.. seealso:: strokeStyle()
.. seealso:: setStrokeColor()
.. seealso:: setPenJoinStyle()
%End

    Qt::PenJoinStyle penJoinStyle() const;
%Docstring
 Returns the marker's stroke join style (e.g., miter, bevel, etc).
.. versionadded:: 2.16
.. seealso:: setPenJoinStyle()
.. seealso:: strokeColor()
.. seealso:: strokeStyle()
 :rtype: Qt.PenJoinStyle
%End

    void setPenJoinStyle( Qt::PenJoinStyle style );
%Docstring
 Sets the marker's stroke join style (e.g., miter, bevel, etc).
 \param style join style
.. versionadded:: 2.16
.. seealso:: penJoinStyle()
.. seealso:: setStrokeColor()
.. seealso:: setStrokeStyle()
%End

    double strokeWidth() const;
%Docstring
 Returns the width of the marker's stroke.
.. seealso:: setStrokeWidth()
.. seealso:: strokeWidthUnit()
.. seealso:: strokeWidthMapUnitScale()
 :rtype: float
%End

    void setStrokeWidth( double w );
%Docstring
 Sets the width of the marker's stroke.
 \param w stroke width. See strokeWidthUnit() for units.
.. seealso:: strokeWidth()
.. seealso:: setStrokeWidthUnit()
.. seealso:: setStrokeWidthMapUnitScale()
%End

    void setStrokeWidthUnit( QgsUnitTypes::RenderUnit u );
%Docstring
 Sets the unit for the width of the marker's stroke.
 \param u stroke width unit
.. seealso:: strokeWidthUnit()
.. seealso:: setStrokeWidth()
.. seealso:: setStrokeWidthMapUnitScale()
%End

    QgsUnitTypes::RenderUnit strokeWidthUnit() const;
%Docstring
 Returns the unit for the width of the marker's stroke.
.. seealso:: setStrokeWidthUnit()
.. seealso:: strokeWidth()
.. seealso:: strokeWidthMapUnitScale()
 :rtype: QgsUnitTypes.RenderUnit
%End

    void setStrokeWidthMapUnitScale( const QgsMapUnitScale &scale );
%Docstring
 Sets the map scale for the width of the marker's stroke.
 \param scale stroke width map unit scale
.. seealso:: strokeWidthMapUnitScale()
.. seealso:: setStrokeWidth()
.. seealso:: setStrokeWidthUnit()
%End

    const QgsMapUnitScale &strokeWidthMapUnitScale() const;
%Docstring
 Returns the map scale for the width of the marker's stroke.
.. seealso:: setStrokeWidthMapUnitScale()
.. seealso:: strokeWidth()
.. seealso:: strokeWidthUnit()
 :rtype: QgsMapUnitScale
%End

  protected:

    void drawMarker( QPainter *p, QgsSymbolRenderContext &context );
%Docstring
 Draws the marker shape in the specified painter.
 \param p destination QPainter
 \param context symbol context
.. note::

   this method does not handle setting the painter pen or brush to match the symbol's fill or stroke
%End

    bool prepareCache( QgsSymbolRenderContext &context );
%Docstring
 Prepares cache image
 :return: true in case of success, false if cache image size too large
 :rtype: bool
%End




  private:
    virtual void draw( QgsSymbolRenderContext &context, QgsSimpleMarkerSymbolLayerBase::Shape shape, const QPolygonF &polygon, const QPainterPath &path ) ;
};

class QgsFilledMarkerSymbolLayer : QgsSimpleMarkerSymbolLayerBase
{
%Docstring
 Filled marker symbol layer, consisting of a shape which is rendered using a QgsFillSymbol. This allows
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
 \param shape symbol shape
 \param size symbol size (in mm)
 \param angle symbol rotation angle
 \param scaleMethod size scaling method
%End

    static QgsSymbolLayer *create( const QgsStringMap &properties = QgsStringMap() ) /Factory/;
%Docstring
 Creates a new QgsFilledMarkerSymbolLayer.
 \param properties a property map containing symbol properties (see properties())
 :return: new QgsFilledMarkerSymbolLayer
 :rtype: QgsSymbolLayer
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
%Docstring
 :rtype: QgsSymbolLayer
%End
    static QgsSymbolLayer *createFromSld( QDomElement &element ) /Factory/;
%Docstring
 :rtype: QgsSymbolLayer
%End

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
.. seealso:: setPath()
 :rtype: str
%End

    void setPath( const QString &path );
%Docstring
 Set the marker SVG path.
 \param path SVG path
.. seealso:: path()
%End

    double defaultAspectRatio() const;
%Docstring
 Returns the default marker aspect ratio between width and height, 0 if not yet calculated.
.. seealso:: updateDefaultAspectRatio()
 :rtype: float
%End

    double updateDefaultAspectRatio();
%Docstring
 Calculates the default marker aspect ratio between width and height.
 :return: the default aspect ratio value
.. seealso:: defaultAspectRatio()
 :rtype: float
%End

    bool preservedAspectRatio() const;
%Docstring
 Returns the preserved aspect ratio value, true if fixed aspect ratio has been lower or equal to 0.
.. seealso:: setPreservedAspectRatio()
 :rtype: bool
%End

    bool setPreservedAspectRatio( bool par );
%Docstring
 Set preserved the marker aspect ratio between width and height.
 \param par Preserved Aspect Ratio
 :return: the preserved aspect ratio value, true if fixed aspect ratio has been lower or equal to 0
.. seealso:: preservedAspectRatio()
 :rtype: bool
%End

    double fixedAspectRatio() const;
%Docstring
 Returns the marker aspect ratio between width and height to be used in rendering,
 if the value set is lower or equal to 0 the aspect ratio will be preserved in rendering
.. seealso:: setFixedAspectRatio() QgsSvgCache
 :rtype: float
%End

    void setFixedAspectRatio( double ratio );
%Docstring
 Set the marker aspect ratio between width and height to be used in rendering,
 if the value set is lower or equal to 0 the aspect ratio will be preserved in rendering
 \param ratio Fixed Aspect Ratio
.. seealso:: fixedAspectRatio() QgsSvgCache
%End

    virtual QColor fillColor() const;
    virtual void setFillColor( const QColor &color );

    virtual QColor strokeColor() const;
    virtual void setStrokeColor( const QColor &c );

    double strokeWidth() const;
%Docstring
 :rtype: float
%End
    void setStrokeWidth( double w );

    void setStrokeWidthUnit( QgsUnitTypes::RenderUnit unit );
%Docstring
 Sets the units for the stroke width.
 \param unit width units
.. seealso:: strokeWidthUnit()
%End

    QgsUnitTypes::RenderUnit strokeWidthUnit() const;
%Docstring
 Returns the units for the stroke width.
.. seealso:: strokeWidthUnit()
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


    virtual bool writeDxf( QgsDxfExport &e, double mmMapUnitScaleFactor, const QString &layerName, QgsSymbolRenderContext &context, QPointF shift = QPointF( 0.0, 0.0 ) ) const;


    virtual QRectF bounds( QPointF point, QgsSymbolRenderContext &context );


  protected:

    double calculateAspectRatio( QgsSymbolRenderContext &context, double scaledSize, bool &hasDataDefinedAspectRatio ) const;
%Docstring
 Calculates the marker aspect ratio between width and height.
 \param context symbol render context
 \param scaledSize size of symbol to render
 \param hasDataDefinedAspectRatio will be set to true if marker has data defined aspectRatio
.. note::

   not available in Python bindings
 :rtype: float
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
%Docstring
 :rtype: QgsSymbolLayer
%End
    static QgsSymbolLayer *createFromSld( QDomElement &element ) /Factory/;
%Docstring
 :rtype: QgsSymbolLayer
%End


    virtual QString layerType() const;


    virtual void startRender( QgsSymbolRenderContext &context );


    virtual void stopRender( QgsSymbolRenderContext &context );


    virtual void renderPoint( QPointF point, QgsSymbolRenderContext &context );


    virtual QgsStringMap properties() const;


    virtual QgsFontMarkerSymbolLayer *clone() const /Factory/;


    virtual void writeSldMarker( QDomDocument &doc, QDomElement &element, const QgsStringMap &props ) const;



    QString fontFamily() const;
%Docstring
 :rtype: str
%End
    void setFontFamily( const QString &family );

    QChar character() const;
%Docstring
 :rtype: QChar
%End
    void setCharacter( QChar ch );

    virtual QColor strokeColor() const;
    virtual void setStrokeColor( const QColor &color );

    double strokeWidth() const;
%Docstring
 Get stroke width.
.. versionadded:: 2.16
 :rtype: float
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
 :rtype: QgsUnitTypes.RenderUnit
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
 :rtype: QgsMapUnitScale
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
 :rtype: Qt.PenJoinStyle
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
