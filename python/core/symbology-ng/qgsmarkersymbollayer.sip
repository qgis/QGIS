/** \ingroup core
 * \class QgsSimpleMarkerSymbolLayerBase
 * \brief Abstract base class for simple marker symbol layers. Handles creation of the symbol shapes but
 * leaves the actual drawing of the symbols to subclasses.
 * \note Added in version 2.16
 */
class QgsSimpleMarkerSymbolLayerBase : QgsMarkerSymbolLayer
{
%TypeHeaderCode
#include <qgsmarkersymbollayer.h>
%End

  public:

    //! Marker symbol shapes
    enum Shape
    {
      Square, /*!< Square */
      Diamond, /*!< Diamond */
      Pentagon, /*!< Pentagon */
      Hexagon, /*!< Hexagon */
      Triangle, /*!< Triangle */
      EquilateralTriangle, /*!< Equilateral triangle*/
      Star, /*!< Star*/
      Arrow, /*!< Arrow*/
      Circle, /*!< Circle*/
      Cross, /*!< Cross (lines only)*/
      CrossFill, /*!< Solid filled cross*/
      Cross2, /*!< Rotated cross (lines only), "x" shape*/
      Line, /*!< Vertical line*/
      ArrowHead, /*!< Right facing arrow head (unfilled, lines only)*/
      ArrowHeadFilled, /*!< Right facing filled arrow head*/
      SemiCircle, /*!< Semi circle (top half)*/
      ThirdCircle, /*!< One third circle (top left third)*/
      QuarterCircle, /*!< Quarter circle (top left quarter)*/
      QuarterSquare, /*!< Quarter square (top left quarter)*/
      HalfSquare, /*!< Half square (left half)*/
      DiagonalHalfSquare, /*!< Diagonal half square (bottom left half)*/
      RightHalfTriangle, /*!< Right half of triangle*/
      LeftHalfTriangle, /*!< Left half of triangle*/
    };

    //! Returns a list of all available shape types.
    static QList< QgsSimpleMarkerSymbolLayerBase::Shape > availableShapes();

    /** Returns true if a symbol shape has a fill.
     * @param shape shape to test
     * @returns true if shape uses a fill, or false if shape uses lines only
     */
    static bool shapeIsFilled( Shape shape );

    /** Constructor for QgsSimpleMarkerSymbolLayerBase.
    * @param shape symbol shape for markers
    * @param size symbol size (in mm)
    * @param angle symbol rotation angle
    * @param scaleMethod scaling method for data defined scaling
    */
    QgsSimpleMarkerSymbolLayerBase( Shape shape = Circle,
                                    double size = DEFAULT_SIMPLEMARKER_SIZE,
                                    double angle = DEFAULT_SIMPLEMARKER_ANGLE,
                                    QgsSymbol::ScaleMethod scaleMethod = DEFAULT_SCALE_METHOD );

    /** Returns the shape for the rendered marker symbol.
     * @see setShape()
     */
    Shape shape() const;

    /** Sets the rendered marker shape.
     * @param shape new marker shape
     * @see shape()
     */
    void setShape( Shape shape );

    /** Attempts to decode a string representation of a shape name to the corresponding
     * shape.
     * @param name encoded shape name
     * @param ok if specified, will be set to true if shape was successfully decoded
     * @return decoded name
     * @see encodeShape()
     */
    static Shape decodeShape( const QString &name, bool *ok = 0 );

    /** Encodes a shape to its string representation.
     * @param shape shape to encode
     * @returns encoded string
     * @see decodeShape()
     */
    static QString encodeShape( Shape shape );

    void startRender( QgsSymbolRenderContext &context );
    void stopRender( QgsSymbolRenderContext &context );
    void renderPoint( QPointF point, QgsSymbolRenderContext &context );
    QRectF bounds( QPointF point, QgsSymbolRenderContext &context );

  protected:

    //! Prepares the layer for drawing the specified shape (QPolygonF version)
    bool prepareMarkerShape( Shape shape );

    //! Prepares the layer for drawing the specified shape (QPainterPath version)
    bool prepareMarkerPath( Shape symbol );

    /** Creates a polygon representing the specified shape.
     * @param shape shape to create
     * @param polygon destination polygon for shape
     * @returns true if shape was successfully stored in polygon
     */
    bool shapeToPolygon( Shape shape, QPolygonF &polygon ) const;

    /** Calculates the desired size of the marker, considering data defined size overrides.
     * @param context symbol render context
     * @param hasDataDefinedSize will be set to true if marker uses data defined sizes
     * @returns marker size, in original size units
     */
    double calculateSize( QgsSymbolRenderContext &context, bool &hasDataDefinedSize ) const;

    /** Calculates the marker offset and rotation.
     * @param context symbol render context
     * @param scaledSize size of symbol to render
     * @param hasDataDefinedRotation will be set to true if marker has data defined rotation
     * @param offset will be set to calculated marker offset (in painter units)
     * @param angle will be set to calculated marker angle
     */
    void calculateOffsetAndRotation( QgsSymbolRenderContext &context, double scaledSize, bool &hasDataDefinedRotation, QPointF &offset, double &angle ) const;

  private:

    /** Derived classes must implement draw() to handle drawing the generated shape onto the painter surface.
     * @param context symbol render context
     * @param shape shape to draw
     * @param polygon polygon representing transformed marker shape. May be empty, in which case the shape will be specified
     * in the path argument.
     * @param path transformed painter path representing shape to draw
     */
    virtual void draw( QgsSymbolRenderContext &context, Shape shape, const QPolygonF &polygon, const QPainterPath &path ) = 0;

};

/** \ingroup core
 * \class QgsSimpleMarkerSymbolLayer
 * \brief Simple marker symbol layer, consisting of a rendered shape with solid fill color and an stroke.
 */
class QgsSimpleMarkerSymbolLayer : QgsSimpleMarkerSymbolLayerBase
{
%TypeHeaderCode
#include <qgsmarkersymbollayer.h>
%End

  public:

    /** Constructor for QgsSimpleMarkerSymbolLayer.
    * @param shape symbol shape
    * @param size symbol size (in mm)
    * @param angle symbol rotation angle
    * @param scaleMethod scaling method for data defined scaling
    * @param color fill color for symbol
    * @param strokeColor stroke color for symbol
    * @param penJoinStyle join style for stroke pen
    */
    QgsSimpleMarkerSymbolLayer( Shape shape = Circle,
                                  double size = DEFAULT_SIMPLEMARKER_SIZE,
                                  double angle = DEFAULT_SIMPLEMARKER_ANGLE,
                                  QgsSymbol::ScaleMethod scaleMethod = DEFAULT_SCALE_METHOD,
                                  const QColor& color = DEFAULT_SIMPLEMARKER_COLOR,
                                  const QColor& strokeColor = DEFAULT_SIMPLEMARKER_BORDERCOLOR,
                                  Qt::PenJoinStyle penJoinStyle = DEFAULT_SIMPLEMARKER_JOINSTYLE );

    // static methods

    /** Creates a new QgsSimpleMarkerSymbolLayer.
     * @param properties a property map containing symbol properties (see properties())
     * @returns new QgsSimpleMarkerSymbolLayer
     */
    static QgsSymbolLayer *create( const QgsStringMap &properties = QgsStringMap() ) /Factory/;

    /** Creates a new QgsSimpleMarkerSymbolLayer from an SLD XML element.
     * @param element XML element containing SLD definition of symbol
     * @returns new QgsSimpleMarkerSymbolLayer
     */
    static QgsSymbolLayer *createFromSld( QDomElement &element ) /Factory/;

    // reimplemented from base classes

    QString layerType() const;
    void startRender( QgsSymbolRenderContext &context );
    void renderPoint( QPointF point, QgsSymbolRenderContext &context );
    QgsStringMap properties() const;
    virtual QgsSimpleMarkerSymbolLayer *clone() const /Factory/;
    void writeSldMarker( QDomDocument &doc, QDomElement &element, const QgsStringMap &props ) const;
    QString ogrFeatureStyle( double mmScaleFactor, double mapUnitScaleFactor ) const;
    bool writeDxf( QgsDxfExport &e, double mmMapUnitScaleFactor, const QString &layerName, QgsSymbolRenderContext &context, QPointF shift = QPointF( 0.0, 0.0 ) ) const;
    void setOutputUnit( QgsUnitTypes::RenderUnit unit );
    QgsUnitTypes::RenderUnit outputUnit() const;
    void setMapUnitScale( const QgsMapUnitScale &scale );
    QgsMapUnitScale mapUnitScale() const;
    QRectF bounds( QPointF point, QgsSymbolRenderContext &context );
    QColor fillColor() const;
    void setFillColor( const QColor &color );
    void setColor( const QColor &color );
    virtual QColor color() const;

    // new methods

    /** Returns the marker's stroke color.
     * @see setStrokeColor()
     * @see strokeStyle()
     * @see penJoinStyle()
     */
    QColor strokeColor() const;

    /** Sets the marker's stroke color.
     * @param color stroke color
     * @see strokeColor()
     * @see setStrokeStyle()
     * @see setPenJoinStyle()
     */
    void setStrokeColor( const QColor &color );

    /** Returns the marker's stroke style (e.g., solid, dashed, etc)
     * @note added in 2.4
     * @see setStrokeStyle()
     * @see strokeColor()
     * @see penJoinStyle()
    */
    Qt::PenStyle strokeStyle() const;

    /** Sets the marker's stroke style (e.g., solid, dashed, etc)
     * @param strokeStyle style
     * @note added in 2.4
     * @see strokeStyle()
     * @see setStrokeColor()
     * @see setPenJoinStyle()
    */
    void setStrokeStyle( Qt::PenStyle strokeStyle );

    /** Returns the marker's stroke join style (e.g., miter, bevel, etc).
     * @note added in 2.16
     * @see setPenJoinStyle()
     * @see strokeColor()
     * @see strokeStyle()
    */
    Qt::PenJoinStyle penJoinStyle() const;

    /** Sets the marker's stroke join style (e.g., miter, bevel, etc).
     * @param style join style
     * @note added in 2.16
     * @see penJoinStyle()
     * @see setStrokeColor()
     * @see setStrokeStyle()
    */
    void setPenJoinStyle( Qt::PenJoinStyle style );

    /** Returns the width of the marker's stroke.
     * @see setStrokeWidth()
     * @see strokeWidthUnit()
     * @see strokeWidthMapUnitScale()
     */
    double strokeWidth() const;

    /** Sets the width of the marker's stroke.
     * @param w stroke width. See strokeWidthUnit() for units.
     * @see strokeWidth()
     * @see setStrokeWidthUnit()
     * @see setStrokeWidthMapUnitScale()
     */
    void setStrokeWidth( double w );

    /** Sets the unit for the width of the marker's stroke.
     * @param u stroke width unit
     * @see strokeWidthUnit()
     * @see setStrokeWidth()
     * @see setStrokeWidthMapUnitScale()
     */
    void setStrokeWidthUnit( QgsUnitTypes::RenderUnit u );

    /** Returns the unit for the width of the marker's stroke.
     * @see setStrokeWidthUnit()
     * @see strokeWidth()
     * @see strokeWidthMapUnitScale()
     */
    QgsUnitTypes::RenderUnit strokeWidthUnit() const;

    /** Sets the map scale for the width of the marker's stroke.
     * @param scale stroke width map unit scale
     * @see strokeWidthMapUnitScale()
     * @see setStrokeWidth()
     * @see setStrokeWidthUnit()
     */
    void setStrokeWidthMapUnitScale( const QgsMapUnitScale &scale );

    /** Returns the map scale for the width of the marker's stroke.
     * @see setStrokeWidthMapUnitScale()
     * @see strokeWidth()
     * @see strokeWidthUnit()
     */
    const QgsMapUnitScale &strokeWidthMapUnitScale() const;

  protected:

    /** Draws the marker shape in the specified painter.
     * @param p destination QPainter
     * @param context symbol context
     * @note this method does not handle setting the painter pen or brush to match the symbol's fill or stroke
     */
    void drawMarker( QPainter *p, QgsSymbolRenderContext &context );

    /** Prepares cache image
     * @returns true in case of success, false if cache image size too large
    */
    bool prepareCache( QgsSymbolRenderContext &context );

  private:

    virtual void draw( QgsSymbolRenderContext &context, Shape shape, const QPolygonF &polygon, const QPainterPath &path );
};



/** \ingroup core
 * \class QgsFilledMarkerSymbolLayer
 * \brief Filled marker symbol layer, consisting of a shape which is rendered using a QgsFillSymbol. This allows
 * the symbol to support advanced styling of the interior and stroke of the shape.
 * \note Added in version 2.16
 */
class QgsFilledMarkerSymbolLayer : QgsSimpleMarkerSymbolLayerBase
{
%TypeHeaderCode
#include <qgsmarkersymbollayer.h>
%End
  public:

    /** Constructor for QgsFilledMarkerSymbolLayer.
    * @param shape symbol shape
    * @param size symbol size (in mm)
    * @param angle symbol rotation angle
    * @param scaleMethod size scaling method
    */
    QgsFilledMarkerSymbolLayer( Shape shape = Circle,
                                double size = DEFAULT_SIMPLEMARKER_SIZE,
                                double angle = DEFAULT_SIMPLEMARKER_ANGLE,
                                QgsSymbol::ScaleMethod scaleMethod = DEFAULT_SCALE_METHOD );

    /** Creates a new QgsFilledMarkerSymbolLayer.
     * @param properties a property map containing symbol properties (see properties())
     * @returns new QgsFilledMarkerSymbolLayer
     */
    static QgsSymbolLayer *create( const QgsStringMap &properties = QgsStringMap() ) /Factory/;

    QString layerType() const;
    void startRender( QgsSymbolRenderContext &context );
    void stopRender( QgsSymbolRenderContext &context );
    QgsStringMap properties() const;
    virtual QgsFilledMarkerSymbolLayer *clone() const /Factory/;
    virtual QgsSymbol *subSymbol();
    virtual bool setSubSymbol( QgsSymbol *symbol /Transfer/ );
    virtual double estimateMaxBleed( const QgsRenderContext &context ) const;
    QSet<QString> usedAttributes( const QgsRenderContext &context ) const;
    void setColor( const QColor &c );
    virtual QColor color() const;

  private:

    virtual void draw( QgsSymbolRenderContext &context, Shape shape, const QPolygonF &polygon, const QPainterPath &path );

    QgsFilledMarkerSymbolLayer( const QgsFilledMarkerSymbolLayer& );
};


class QgsSvgMarkerSymbolLayer : QgsMarkerSymbolLayer
{
%TypeHeaderCode
#include <qgsmarkersymbollayer.h>
%End

  public:
    QgsSvgMarkerSymbolLayer( const QString& path,
                               double size = DEFAULT_SVGMARKER_SIZE,
                               double angle = DEFAULT_SVGMARKER_ANGLE,
                               QgsSymbol::ScaleMethod scaleMethod = DEFAULT_SCALE_METHOD );

    // static stuff

    static QgsSymbolLayer* create( const QgsStringMap& properties = QgsStringMap() ) /Factory/;
    static QgsSymbolLayer* createFromSld( QDomElement &element ) /Factory/;
    static void resolvePaths( QgsStringMap& properties, const QgsPathResolver& pathResolver, bool saving );

    // implemented from base classes

    QString layerType() const;

    void startRender( QgsSymbolRenderContext &context );

    void stopRender( QgsSymbolRenderContext &context );

    void renderPoint( QPointF point, QgsSymbolRenderContext &context );

    QgsStringMap properties() const;

    virtual QgsSvgMarkerSymbolLayer *clone() const /Factory/;

    void writeSldMarker( QDomDocument &doc, QDomElement &element, const QgsStringMap &props ) const;

    QString path() const;
    void setPath( const QString &path );

    QColor fillColor() const;
    void setFillColor( const QColor &c );

    QColor strokeColor() const;
    void setStrokeColor( const QColor &c );

    double strokeWidth() const;
    void setStrokeWidth( double w );

    /** Sets the units for the stroke width.
     * @param unit width units
     * @see strokeWidthUnit()
    */
    void setStrokeWidthUnit( QgsUnitTypes::RenderUnit unit );

    /** Returns the units for the stroke width.
     * @see strokeWidthUnit()
    */
    QgsUnitTypes::RenderUnit strokeWidthUnit() const;

    void setStrokeWidthMapUnitScale( const QgsMapUnitScale &scale);
    const QgsMapUnitScale &strokeWidthMapUnitScale() const;

    void setOutputUnit( QgsUnitTypes::RenderUnit unit );
    QgsUnitTypes::RenderUnit outputUnit() const;

    void setMapUnitScale( const QgsMapUnitScale &scale );
    QgsMapUnitScale mapUnitScale() const;

    bool writeDxf( QgsDxfExport &e, double mmMapUnitScaleFactor, const QString &layerName, QgsSymbolRenderContext &context, QPointF shift = QPointF( 0.0, 0.0 ) ) const;

    QRectF bounds( QPointF point, QgsSymbolRenderContext &context );
};

class QgsFontMarkerSymbolLayer : QgsMarkerSymbolLayer
{
%TypeHeaderCode
#include <qgsmarkersymbollayer.h>
%End

  public:
    QgsFontMarkerSymbolLayer( const QString& fontFamily = DEFAULT_FONTMARKER_FONT,
                                QChar chr = DEFAULT_FONTMARKER_CHR,
                                double pointSize = DEFAULT_FONTMARKER_SIZE,
                                const QColor& color = DEFAULT_FONTMARKER_COLOR,
                                double angle = DEFAULT_FONTMARKER_ANGLE );

    ~QgsFontMarkerSymbolLayer();

    // static stuff

    static QgsSymbolLayer *create( const QgsStringMap &properties = QgsStringMap() ) /Factory/;
    static QgsSymbolLayer *createFromSld( QDomElement &element ) /Factory/;

    // implemented from base classes

    QString layerType() const;

    void startRender( QgsSymbolRenderContext &context );

    void stopRender( QgsSymbolRenderContext &context );

    void renderPoint( QPointF point, QgsSymbolRenderContext &context );

    QgsStringMap properties() const;

    virtual QgsFontMarkerSymbolLayer *clone() const /Factory/;

    void writeSldMarker( QDomDocument &doc, QDomElement &element, const QgsStringMap &props ) const;

    // new methods

    QString fontFamily() const;
    void setFontFamily( const QString &family );

    QChar character() const;
    void setCharacter( QChar ch );

    /** Get stroke color.
     * @note added in 2.16 */
    QColor strokeColor() const;
    /** Set stroke color.
     * @note added in 2.16 */
    void setStrokeColor( const QColor &color );

    /** Get stroke width.
     * @note added in 2.16 */
    double strokeWidth() const;
    /** Set stroke width.
     * @note added in 2.16 */
    void setStrokeWidth( double width );

    /** Get stroke width unit.
     * @note added in 2.16 */
    QgsUnitTypes::RenderUnit strokeWidthUnit() const;
    /** Set stroke width unit.
     * @note added in 2.16 */
    void setStrokeWidthUnit( QgsUnitTypes::RenderUnit unit );

    /** Get stroke width map unit scale.
     * @note added in 2.16 */
    const QgsMapUnitScale &strokeWidthMapUnitScale() const;
    /** Set stroke width map unit scale.
     * @note added in 2.16 */
    void setStrokeWidthMapUnitScale( const QgsMapUnitScale &scale );

    /** Get stroke join style.
     * @note added in 2.16 */
    Qt::PenJoinStyle penJoinStyle() const;
    /** Set stroke join style.
     * @note added in 2.16 */
    void setPenJoinStyle( Qt::PenJoinStyle style );

    QRectF bounds( QPointF point, QgsSymbolRenderContext &context );

};
