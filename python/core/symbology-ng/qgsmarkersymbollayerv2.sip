/** \ingroup core
 * \class QgsSimpleMarkerSymbolLayerBase
 * \brief Abstract base class for simple marker symbol layers. Handles creation of the symbol shapes but
 * leaves the actual drawing of the symbols to subclasses.
 * \note Added in version 2.16
 */
class QgsSimpleMarkerSymbolLayerBase : QgsMarkerSymbolLayerV2
{
%TypeHeaderCode
#include <qgsmarkersymbollayerv2.h>
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
                                    QgsSymbolV2::ScaleMethod scaleMethod = DEFAULT_SCALE_METHOD );

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
    static Shape decodeShape( const QString& name, bool* ok = nullptr );

    /** Encodes a shape to its string representation.
     * @param shape shape to encode
     * @returns encoded string
     * @see decodeShape()
     */
    static QString encodeShape( Shape shape );

    void startRender( QgsSymbolV2RenderContext& context );
    void stopRender( QgsSymbolV2RenderContext& context );
    void renderPoint( QPointF point, QgsSymbolV2RenderContext& context );
    QRectF bounds( QPointF point, QgsSymbolV2RenderContext& context );

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
    double calculateSize( QgsSymbolV2RenderContext& context, bool& hasDataDefinedSize ) const;

    /** Calculates the marker offset and rotation.
     * @param context symbol render context
     * @param scaledSize size of symbol to render
     * @param hasDataDefinedRotation will be set to true if marker has data defined rotation
     * @param offset will be set to calculated marker offset (in painter units)
     * @param angle will be set to calculated marker angle
     */
    void calculateOffsetAndRotation( QgsSymbolV2RenderContext& context, double scaledSize, bool& hasDataDefinedRotation, QPointF& offset, double& angle ) const;

  private:

    /** Derived classes must implement draw() to handle drawing the generated shape onto the painter surface.
     * @param context symbol render context
     * @param shape shape to draw
     * @param polygon polygon representing transformed marker shape. May be empty, in which case the shape will be specified
     * in the path argument.
     * @param path transformed painter path representing shape to draw
     */
    virtual void draw( QgsSymbolV2RenderContext& context, Shape shape, const QPolygonF& polygon, const QPainterPath& path ) = 0;

};

/** \ingroup core
 * \class QgsSimpleMarkerSymbolLayerV2
 * \brief Simple marker symbol layer, consisting of a rendered shape with solid fill color and an outline.
 */
class QgsSimpleMarkerSymbolLayerV2 : QgsSimpleMarkerSymbolLayerBase
{
%TypeHeaderCode
#include <qgsmarkersymbollayerv2.h>
%End

  public:

/** Constructor for QgsSimpleMarkerSymbolLayerV2.
    * @param name symbol name, should be one of "square", "rectangle", "diamond",
    * "pentagon", "hexagon", "triangle", "equilateral_triangle", "star", "arrow",
    * "circle", "cross", "cross_fill", "cross2", "line", "x", "arrowhead", "filled_arrowhead",
    * "semi_circle", "third_circle", "quarter_circle", "quarter_square", "half_square",
    * "diagonal_half_square", "right_half_triangle", "left_half_triangle"
    * @param color fill color for symbol
    * @param borderColor border color for symbol
    * @param size symbol size (in mm)
    * @param angle symbol rotation angle
    * @param scaleMethod scaling method for data defined scaling
    * @param penJoinStyle join style for outline pen
    * @deprecated use variant which accepts QgsSimpleMarkerSymbolLayerBase::Shape instead
    */
    QgsSimpleMarkerSymbolLayerV2( const QString& name,
        const QColor& color = DEFAULT_SIMPLEMARKER_COLOR,
        const QColor& borderColor = DEFAULT_SIMPLEMARKER_BORDERCOLOR,
        double size = DEFAULT_SIMPLEMARKER_SIZE,
        double angle = DEFAULT_SIMPLEMARKER_ANGLE,
        QgsSymbolV2::ScaleMethod scaleMethod = DEFAULT_SCALE_METHOD,
        Qt::PenJoinStyle penJoinStyle = DEFAULT_SIMPLEMARKER_JOINSTYLE ) /Deprecated/;

    /** Constructor for QgsSimpleMarkerSymbolLayerV2.
    * @param shape symbol shape
    * @param size symbol size (in mm)
    * @param angle symbol rotation angle
    * @param scaleMethod scaling method for data defined scaling
    * @param color fill color for symbol
    * @param borderColor border color for symbol
    * @param penJoinStyle join style for outline pen
    */
    QgsSimpleMarkerSymbolLayerV2( Shape shape = Circle,
                                  double size = DEFAULT_SIMPLEMARKER_SIZE,
                                  double angle = DEFAULT_SIMPLEMARKER_ANGLE,
                                  QgsSymbolV2::ScaleMethod scaleMethod = DEFAULT_SCALE_METHOD,
                                  const QColor& color = DEFAULT_SIMPLEMARKER_COLOR,
                                  const QColor& borderColor = DEFAULT_SIMPLEMARKER_BORDERCOLOR,
                                  Qt::PenJoinStyle penJoinStyle = DEFAULT_SIMPLEMARKER_JOINSTYLE );

    // static methods

    /** Creates a new QgsSimpleMarkerSymbolLayerV2.
     * @param properties a property map containing symbol properties (see properties())
     * @returns new QgsSimpleMarkerSymbolLayerV2
     */
    static QgsSymbolLayerV2* create( const QgsStringMap& properties = QgsStringMap() ) /Factory/;

    /** Creates a new QgsSimpleMarkerSymbolLayerV2 from an SLD XML element.
     * @param element XML element containing SLD definition of symbol
     * @returns new QgsSimpleMarkerSymbolLayerV2
     */
    static QgsSymbolLayerV2* createFromSld( QDomElement &element ) /Factory/;

    // reimplemented from base classes

    QString layerType() const;
    void startRender( QgsSymbolV2RenderContext& context );
    void renderPoint( QPointF point, QgsSymbolV2RenderContext& context );
    QgsStringMap properties() const;
    virtual QgsSimpleMarkerSymbolLayerV2* clone() const /Factory/;
    void writeSldMarker( QDomDocument &doc, QDomElement &element, const QgsStringMap& props ) const;
    QString ogrFeatureStyle( double mmScaleFactor, double mapUnitScaleFactor ) const;
    bool writeDxf( QgsDxfExport &e, double mmMapUnitScaleFactor, const QString &layerName, QgsSymbolV2RenderContext &context, QPointF shift = QPointF( 0.0, 0.0 ) ) const;
    void setOutputUnit( QgsSymbolV2::OutputUnit unit );
    QgsSymbolV2::OutputUnit outputUnit() const;
    void setMapUnitScale( const QgsMapUnitScale& scale );
    QgsMapUnitScale mapUnitScale() const;
    QRectF bounds( QPointF point, QgsSymbolV2RenderContext& context );
    QColor outlineColor() const;
    void setOutlineColor( const QColor& color );
    QColor fillColor() const;
    void setFillColor( const QColor& color );
    void setColor( const QColor& color );
    virtual QColor color() const;

    // new methods

    //! @deprecated use shape() instead
    QString name() const /Deprecated/;

    //! @deprecated use setShape() instead
    void setName( const QString& name ) /Deprecated/;

    /** Returns the marker's border color.
     * @see setBorderColor()
     * @see outlineStyle()
     * @see penJoinStyle()
     */
    QColor borderColor() const;

    /** Sets the marker's border color.
     * @param color border color
     * @see borderColor()
     * @see setOutlineStyle()
     * @see setPenJoinStyle()
     */
    void setBorderColor( const QColor& color );

    /** Returns the marker's outline style (eg solid, dashed, etc)
     * @note added in 2.4
     * @see setOutlineStyle()
     * @see borderColor()
     * @see penJoinStyle()
    */
    Qt::PenStyle outlineStyle() const;

    /** Sets the marker's outline style (eg solid, dashed, etc)
     * @param outlineStyle style
     * @note added in 2.4
     * @see outlineStyle()
     * @see setBorderColor()
     * @see setPenJoinStyle()
    */
    void setOutlineStyle( Qt::PenStyle outlineStyle );

    /** Returns the marker's outline join style (eg miter, bevel, etc).
     * @note added in 2.16
     * @see setPenJoinStyle()
     * @see borderColor()
     * @see outlineStyle()
    */
    Qt::PenJoinStyle penJoinStyle() const;

    /** Sets the marker's outline join style (eg miter, bevel, etc).
     * @param style join style
     * @note added in 2.16
     * @see penJoinStyle()
     * @see setBorderColor()
     * @see setOutlineStyle()
    */
    void setPenJoinStyle( Qt::PenJoinStyle style );

    /** Returns the width of the marker's outline.
     * @see setOutlineWidth()
     * @see outlineWidthUnit()
     * @see outlineWidthMapUnitScale()
     */
    double outlineWidth() const;

    /** Sets the width of the marker's outline.
     * @param w outline width. See outlineWidthUnit() for units.
     * @see outlineWidth()
     * @see setOutlineWidthUnit()
     * @see setOutlineWidthMapUnitScale()
     */
    void setOutlineWidth( double w );

    /** Sets the unit for the width of the marker's outline.
     * @param u outline width unit
     * @see outlineWidthUnit()
     * @see setOutlineWidth()
     * @see setOutlineWidthMapUnitScale()
     */
    void setOutlineWidthUnit( QgsSymbolV2::OutputUnit u );

    /** Returns the unit for the width of the marker's outline.
     * @see setOutlineWidthUnit()
     * @see outlineWidth()
     * @see outlineWidthMapUnitScale()
     */
    QgsSymbolV2::OutputUnit outlineWidthUnit() const;

    /** Sets the map scale for the width of the marker's outline.
     * @param scale outline width map unit scale
     * @see outlineWidthMapUnitScale()
     * @see setOutlineWidth()
     * @see setOutlineWidthUnit()
     */
    void setOutlineWidthMapUnitScale( const QgsMapUnitScale& scale );

    /** Returns the map scale for the width of the marker's outline.
     * @see setOutlineWidthMapUnitScale()
     * @see outlineWidth()
     * @see outlineWidthUnit()
     */
    const QgsMapUnitScale& outlineWidthMapUnitScale() const;

  protected:

    /** Draws the marker shape in the specified painter.
     * @param p destination QPainter
     * @param context symbol context
     * @note this method does not handle setting the painter pen or brush to match the symbol's fill or outline
     */
    void drawMarker( QPainter* p, QgsSymbolV2RenderContext& context );

    //! @deprecated will be removed in QGIS 3.0
    bool prepareShape( const QString& name = QString() ) /Deprecated/;
    //! @deprecated will be removed in QGIS 3.0
    bool prepareShape( const QString& name, QPolygonF &polygon ) const /Deprecated/;
    //! @deprecated will be removed in QGIS 3.0
    bool preparePath( QString name = QString() ) /Deprecated/;

    /** Prepares cache image
     * @returns true in case of success, false if cache image size too large
    */
    bool prepareCache( QgsSymbolV2RenderContext& context );

  private:

    virtual void draw( QgsSymbolV2RenderContext& context, Shape shape, const QPolygonF& polygon, const QPainterPath& path );
};



/** \ingroup core
 * \class QgsFilledMarkerSymbolLayer
 * \brief Filled marker symbol layer, consisting of a shape which is rendered using a QgsFillSymbolV2. This allows
 * the symbol to support advanced styling of the interior and outline of the shape.
 * \note Added in version 2.16
 */
class QgsFilledMarkerSymbolLayer : QgsSimpleMarkerSymbolLayerBase
{
%TypeHeaderCode
#include <qgsmarkersymbollayerv2.h>
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
                                QgsSymbolV2::ScaleMethod scaleMethod = DEFAULT_SCALE_METHOD );

    /** Creates a new QgsFilledMarkerSymbolLayer.
     * @param properties a property map containing symbol properties (see properties())
     * @returns new QgsFilledMarkerSymbolLayer
     */
    static QgsSymbolLayerV2* create( const QgsStringMap& properties = QgsStringMap() ) /Factory/;

    QString layerType() const;
    void startRender( QgsSymbolV2RenderContext& context );
    void stopRender( QgsSymbolV2RenderContext& context );
    QgsStringMap properties() const;
    virtual QgsFilledMarkerSymbolLayer* clone() const /Factory/;
    virtual QgsSymbolV2* subSymbol();
    virtual bool setSubSymbol( QgsSymbolV2* symbol /Transfer/ );
    virtual double estimateMaxBleed() const;
    QSet<QString> usedAttributes() const;
    void setColor( const QColor& c );
    virtual QColor color() const;

  private:

    virtual void draw( QgsSymbolV2RenderContext& context, Shape shape, const QPolygonF& polygon, const QPainterPath& path );

    QgsFilledMarkerSymbolLayer( const QgsFilledMarkerSymbolLayer& );
};


class QgsSvgMarkerSymbolLayerV2 : QgsMarkerSymbolLayerV2
{
%TypeHeaderCode
#include <qgsmarkersymbollayerv2.h>
%End

  public:
    QgsSvgMarkerSymbolLayerV2( const QString& name = DEFAULT_SVGMARKER_NAME,
                               double size = DEFAULT_SVGMARKER_SIZE,
                               double angle = DEFAULT_SVGMARKER_ANGLE,
                               QgsSymbolV2::ScaleMethod scaleMethod = DEFAULT_SCALE_METHOD );

    // static stuff

    static QgsSymbolLayerV2* create( const QgsStringMap& properties = QgsStringMap() ) /Factory/;
    static QgsSymbolLayerV2* createFromSld( QDomElement &element ) /Factory/;

    // implemented from base classes

    QString layerType() const;

    void startRender( QgsSymbolV2RenderContext& context );

    void stopRender( QgsSymbolV2RenderContext& context );

    void renderPoint( QPointF point, QgsSymbolV2RenderContext& context );

    QgsStringMap properties() const;

    virtual QgsSvgMarkerSymbolLayerV2* clone() const /Factory/;

    void writeSldMarker( QDomDocument &doc, QDomElement &element, const QgsStringMap& props ) const;

    QString path() const;
    void setPath( const QString& path );

    QColor fillColor() const;
    void setFillColor( const QColor& c );

    QColor outlineColor() const;
    void setOutlineColor( const QColor& c );

    double outlineWidth() const;
    void setOutlineWidth( double w );

    void setOutlineWidthUnit( QgsSymbolV2::OutputUnit unit );
    QgsSymbolV2::OutputUnit outlineWidthUnit() const;

    void setOutlineWidthMapUnitScale( const QgsMapUnitScale& scale);
    const QgsMapUnitScale& outlineWidthMapUnitScale() const;

    void setOutputUnit( QgsSymbolV2::OutputUnit unit );
    QgsSymbolV2::OutputUnit outputUnit() const;

    void setMapUnitScale( const QgsMapUnitScale& scale );
    QgsMapUnitScale mapUnitScale() const;

    bool writeDxf( QgsDxfExport& e, double mmMapUnitScaleFactor, const QString& layerName, QgsSymbolV2RenderContext &context, QPointF shift = QPointF( 0.0, 0.0 ) ) const;

    QRectF bounds( QPointF point, QgsSymbolV2RenderContext& context );
};

class QgsFontMarkerSymbolLayerV2 : QgsMarkerSymbolLayerV2
{
%TypeHeaderCode
#include <qgsmarkersymbollayerv2.h>
%End

  public:
    QgsFontMarkerSymbolLayerV2( const QString& fontFamily = DEFAULT_FONTMARKER_FONT,
                                QChar chr = DEFAULT_FONTMARKER_CHR,
                                double pointSize = DEFAULT_FONTMARKER_SIZE,
                                const QColor& color = DEFAULT_FONTMARKER_COLOR,
                                double angle = DEFAULT_FONTMARKER_ANGLE );

    ~QgsFontMarkerSymbolLayerV2();

    // static stuff

    static QgsSymbolLayerV2* create( const QgsStringMap& properties = QgsStringMap() ) /Factory/;
    static QgsSymbolLayerV2* createFromSld( QDomElement &element ) /Factory/;

    // implemented from base classes

    QString layerType() const;

    void startRender( QgsSymbolV2RenderContext& context );

    void stopRender( QgsSymbolV2RenderContext& context );

    void renderPoint( QPointF point, QgsSymbolV2RenderContext& context );

    QgsStringMap properties() const;

    virtual QgsFontMarkerSymbolLayerV2* clone() const /Factory/;

    void writeSldMarker( QDomDocument &doc, QDomElement &element, const QgsStringMap& props ) const;

    // new methods

    QString fontFamily() const;
    void setFontFamily( const QString& family );

    QChar character() const;
    void setCharacter( QChar ch );

    /** Get outline color.
     * @note added in 2.16 */
    QColor outlineColor() const;
    /** Set outline color.
     * @note added in 2.16 */
    void setOutlineColor( const QColor& color );

    /** Get outline width.
     * @note added in 2.16 */
    double outlineWidth() const;
    /** Set outline width.
     * @note added in 2.16 */
    void setOutlineWidth( double width );

    /** Get outline width unit.
     * @note added in 2.16 */
    QgsSymbolV2::OutputUnit outlineWidthUnit() const;
    /** Set outline width unit.
     * @note added in 2.16 */
    void setOutlineWidthUnit( QgsSymbolV2::OutputUnit unit );

    /** Get outline width map unit scale.
     * @note added in 2.16 */
    const QgsMapUnitScale& outlineWidthMapUnitScale() const;
    /** Set outline width map unit scale.
     * @note added in 2.16 */
    void setOutlineWidthMapUnitScale( const QgsMapUnitScale& scale );

    /** Get outline join style.
     * @note added in 2.16 */
    Qt::PenJoinStyle penJoinStyle() const;
    /** Set outline join style.
     * @note added in 2.16 */
    void setPenJoinStyle( Qt::PenJoinStyle style );

    QRectF bounds( QPointF point, QgsSymbolV2RenderContext& context );

};
