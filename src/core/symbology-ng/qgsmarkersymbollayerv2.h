/***************************************************************************
 qgsmarkersymbollayerv2.h
 ---------------------
 begin                : November 2009
 copyright            : (C) 2009 by Martin Dobias
 email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSMARKERSYMBOLLAYERV2_H
#define QGSMARKERSYMBOLLAYERV2_H

#include "qgssymbollayerv2.h"
#include "qgsvectorlayer.h"

#define DEFAULT_SIMPLEMARKER_NAME         "circle"
#define DEFAULT_SIMPLEMARKER_COLOR        QColor(255,0,0)
#define DEFAULT_SIMPLEMARKER_BORDERCOLOR  QColor(0,0,0)
#define DEFAULT_SIMPLEMARKER_JOINSTYLE    Qt::BevelJoin
#define DEFAULT_SIMPLEMARKER_SIZE         DEFAULT_POINT_SIZE
#define DEFAULT_SIMPLEMARKER_ANGLE        0

#include <QPen>
#include <QBrush>
#include <QPicture>
#include <QPolygonF>
#include <QFont>

/** \ingroup core
 * \class QgsSimpleMarkerSymbolLayerBase
 * \brief Abstract base class for simple marker symbol layers. Handles creation of the symbol shapes but
 * leaves the actual drawing of the symbols to subclasses.
 * \note Added in version 2.16
 */
class CORE_EXPORT QgsSimpleMarkerSymbolLayerBase : public QgsMarkerSymbolLayerV2
{
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
    static QList< Shape > availableShapes();

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
    Shape shape() const { return mShape; }

    /** Sets the rendered marker shape.
     * @param shape new marker shape
     * @see shape()
     */
    void setShape( Shape shape ) { mShape = shape; }

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

    void startRender( QgsSymbolV2RenderContext& context ) override;
    void stopRender( QgsSymbolV2RenderContext& context ) override;
    void renderPoint( QPointF point, QgsSymbolV2RenderContext& context ) override;
    QRectF bounds( QPointF point, QgsSymbolV2RenderContext& context ) override;

  protected:

    //! Prepares the layer for drawing the specified shape (QPolygonF version)
    //! @note not available in Python bindings
    bool prepareMarkerShape( Shape shape );

    //! Prepares the layer for drawing the specified shape (QPainterPath version)
    //! @note not available in Python bindings
    bool prepareMarkerPath( Shape symbol );

    /** Creates a polygon representing the specified shape.
     * @param shape shape to create
     * @param polygon destination polygon for shape
     * @returns true if shape was successfully stored in polygon
     * @note not available in Python bindings
     */
    bool shapeToPolygon( Shape shape, QPolygonF &polygon ) const;

    /** Calculates the desired size of the marker, considering data defined size overrides.
     * @param context symbol render context
     * @param hasDataDefinedSize will be set to true if marker uses data defined sizes
     * @returns marker size, in original size units
     * @note not available in Python bindings
     */
    double calculateSize( QgsSymbolV2RenderContext& context, bool& hasDataDefinedSize ) const;

    /** Calculates the marker offset and rotation.
     * @param context symbol render context
     * @param scaledSize size of symbol to render
     * @param hasDataDefinedRotation will be set to true if marker has data defined rotation
     * @param offset will be set to calculated marker offset (in painter units)
     * @param angle will be set to calculated marker angle
     * @note not available in Python bindings
     */
    void calculateOffsetAndRotation( QgsSymbolV2RenderContext& context, double scaledSize, bool& hasDataDefinedRotation, QPointF& offset, double& angle ) const;

    //! Polygon of points in shape. If polygon is empty then shape is using mPath.
    QPolygonF mPolygon;

    //! Painter path representing shape. If mPolygon is empty then the shape is stored in mPath.
    QPainterPath mPath;

    //! Symbol shape
    Shape mShape;

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
class CORE_EXPORT QgsSimpleMarkerSymbolLayerV2 : public QgsSimpleMarkerSymbolLayerBase
{
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
    Q_DECL_DEPRECATED QgsSimpleMarkerSymbolLayerV2( const QString& name,
        const QColor& color = DEFAULT_SIMPLEMARKER_COLOR,
        const QColor& borderColor = DEFAULT_SIMPLEMARKER_BORDERCOLOR,
        double size = DEFAULT_SIMPLEMARKER_SIZE,
        double angle = DEFAULT_SIMPLEMARKER_ANGLE,
        QgsSymbolV2::ScaleMethod scaleMethod = DEFAULT_SCALE_METHOD,
        Qt::PenJoinStyle penJoinStyle = DEFAULT_SIMPLEMARKER_JOINSTYLE );

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
    static QgsSymbolLayerV2* create( const QgsStringMap& properties = QgsStringMap() );

    /** Creates a new QgsSimpleMarkerSymbolLayerV2 from an SLD XML element.
     * @param element XML element containing SLD definition of symbol
     * @returns new QgsSimpleMarkerSymbolLayerV2
     */
    static QgsSymbolLayerV2* createFromSld( QDomElement &element );

    // reimplemented from base classes

    QString layerType() const override;
    void startRender( QgsSymbolV2RenderContext& context ) override;
    void renderPoint( QPointF point, QgsSymbolV2RenderContext& context ) override;
    QgsStringMap properties() const override;
    QgsSimpleMarkerSymbolLayerV2* clone() const override;
    void writeSldMarker( QDomDocument &doc, QDomElement &element, const QgsStringMap& props ) const override;
    QString ogrFeatureStyle( double mmScaleFactor, double mapUnitScaleFactor ) const override;
    bool writeDxf( QgsDxfExport &e, double mmMapUnitScaleFactor, const QString &layerName, QgsSymbolV2RenderContext &context, QPointF shift = QPointF( 0.0, 0.0 ) ) const override;
    void setOutputUnit( QgsSymbolV2::OutputUnit unit ) override;
    QgsSymbolV2::OutputUnit outputUnit() const override;
    void setMapUnitScale( const QgsMapUnitScale& scale ) override;
    QgsMapUnitScale mapUnitScale() const override;
    QRectF bounds( QPointF point, QgsSymbolV2RenderContext& context ) override;
    QColor outlineColor() const override { return borderColor(); }
    void setOutlineColor( const QColor& color ) override { setBorderColor( color ); }
    QColor fillColor() const override { return mColor; }
    void setFillColor( const QColor& color ) override { mColor = color; }
    void setColor( const QColor& color ) override;
    virtual QColor color() const override;

    // new methods

    //! @deprecated use shape() instead
    Q_DECL_DEPRECATED QString name() const { return encodeShape( mShape ); }

    //! @deprecated use setShape() instead
    Q_DECL_DEPRECATED void setName( const QString& name ) { mName = name; mShape = decodeShape( name ); }

    /** Returns the marker's border color.
     * @see setBorderColor()
     * @see outlineStyle()
     * @see penJoinStyle()
     */
    QColor borderColor() const { return mBorderColor; }

    /** Sets the marker's border color.
     * @param color border color
     * @see borderColor()
     * @see setOutlineStyle()
     * @see setPenJoinStyle()
     */
    void setBorderColor( const QColor& color ) { mBorderColor = color; }

    /** Returns the marker's outline style (eg solid, dashed, etc)
     * @note added in 2.4
     * @see setOutlineStyle()
     * @see borderColor()
     * @see penJoinStyle()
    */
    Qt::PenStyle outlineStyle() const { return mOutlineStyle; }

    /** Sets the marker's outline style (eg solid, dashed, etc)
     * @param outlineStyle style
     * @note added in 2.4
     * @see outlineStyle()
     * @see setBorderColor()
     * @see setPenJoinStyle()
    */
    void setOutlineStyle( Qt::PenStyle outlineStyle ) { mOutlineStyle = outlineStyle; }

    /** Returns the marker's outline join style (eg miter, bevel, etc).
     * @note added in 2.16
     * @see setPenJoinStyle()
     * @see borderColor()
     * @see outlineStyle()
    */
    Qt::PenJoinStyle penJoinStyle() const { return mPenJoinStyle; }

    /** Sets the marker's outline join style (eg miter, bevel, etc).
     * @param style join style
     * @note added in 2.16
     * @see penJoinStyle()
     * @see setBorderColor()
     * @see setOutlineStyle()
    */
    void setPenJoinStyle( Qt::PenJoinStyle style ) { mPenJoinStyle = style; }

    /** Returns the width of the marker's outline.
     * @see setOutlineWidth()
     * @see outlineWidthUnit()
     * @see outlineWidthMapUnitScale()
     */
    double outlineWidth() const { return mOutlineWidth; }

    /** Sets the width of the marker's outline.
     * @param w outline width. See outlineWidthUnit() for units.
     * @see outlineWidth()
     * @see setOutlineWidthUnit()
     * @see setOutlineWidthMapUnitScale()
     */
    void setOutlineWidth( double w ) { mOutlineWidth = w; }

    /** Sets the unit for the width of the marker's outline.
     * @param u outline width unit
     * @see outlineWidthUnit()
     * @see setOutlineWidth()
     * @see setOutlineWidthMapUnitScale()
     */
    void setOutlineWidthUnit( QgsSymbolV2::OutputUnit u ) { mOutlineWidthUnit = u; }

    /** Returns the unit for the width of the marker's outline.
     * @see setOutlineWidthUnit()
     * @see outlineWidth()
     * @see outlineWidthMapUnitScale()
     */
    QgsSymbolV2::OutputUnit outlineWidthUnit() const { return mOutlineWidthUnit; }

    /** Sets the map scale for the width of the marker's outline.
     * @param scale outline width map unit scale
     * @see outlineWidthMapUnitScale()
     * @see setOutlineWidth()
     * @see setOutlineWidthUnit()
     */
    void setOutlineWidthMapUnitScale( const QgsMapUnitScale& scale ) { mOutlineWidthMapUnitScale = scale; }

    /** Returns the map scale for the width of the marker's outline.
     * @see setOutlineWidthMapUnitScale()
     * @see outlineWidth()
     * @see outlineWidthUnit()
     */
    const QgsMapUnitScale& outlineWidthMapUnitScale() const { return mOutlineWidthMapUnitScale; }

  protected:

    /** Draws the marker shape in the specified painter.
     * @param p destination QPainter
     * @param context symbol context
     * @note this method does not handle setting the painter pen or brush to match the symbol's fill or outline
     */
    void drawMarker( QPainter* p, QgsSymbolV2RenderContext& context );

    //! @deprecated will be removed in QGIS 3.0
    Q_DECL_DEPRECATED bool prepareShape( const QString& name = QString() );
    //! @deprecated will be removed in QGIS 3.0
    Q_DECL_DEPRECATED bool prepareShape( const QString& name, QPolygonF &polygon ) const;
    //! @deprecated will be removed in QGIS 3.0
    Q_DECL_DEPRECATED bool preparePath( QString name = QString() );

    /** Prepares cache image
     * @returns true in case of success, false if cache image size too large
    */
    bool prepareCache( QgsSymbolV2RenderContext& context );

    //! Outline color
    QColor mBorderColor;
    //! Outline style
    Qt::PenStyle mOutlineStyle;
    //! Outline width
    double mOutlineWidth;
    //! Outline width units
    QgsSymbolV2::OutputUnit mOutlineWidthUnit;
    //! Outline width map unit scale
    QgsMapUnitScale mOutlineWidthMapUnitScale;
    //! Outline pen join style
    Qt::PenJoinStyle mPenJoinStyle;
    //! QPen corresponding to marker's outline style
    QPen mPen;
    //! QBrush corresponding to marker's fill style
    QBrush mBrush;

    //TODO QGIS 3.0 - remove
    QString mName;

    //! Cached image of marker, if using cached version
    QImage mCache;
    //! QPen to use as outline of selected symbols
    QPen mSelPen;
    //! QBrush to use as fill of selected symbols
    QBrush mSelBrush;
    //! Cached image of selected marker, if using cached version
    QImage mSelCache;
    //! True if using cached images of markers for drawing. This is faster, but cannot
    //! be used when data defined properties are present
    bool mUsingCache;
    //! Maximum width/height of cache image
    static const int mMaximumCacheWidth = 3000;

  private:

    virtual void draw( QgsSymbolV2RenderContext& context, Shape shape, const QPolygonF& polygon, const QPainterPath& path ) override;
};

/** \ingroup core
 * \class QgsFilledMarkerSymbolLayer
 * \brief Filled marker symbol layer, consisting of a shape which is rendered using a QgsFillSymbolV2. This allows
 * the symbol to support advanced styling of the interior and outline of the shape.
 * \note Added in version 2.16
 */
class CORE_EXPORT QgsFilledMarkerSymbolLayer : public QgsSimpleMarkerSymbolLayerBase
{
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
    static QgsSymbolLayerV2* create( const QgsStringMap& properties = QgsStringMap() );

    QString layerType() const override;
    void startRender( QgsSymbolV2RenderContext& context ) override;
    void stopRender( QgsSymbolV2RenderContext& context ) override;
    QgsStringMap properties() const override;
    QgsFilledMarkerSymbolLayer* clone() const override;
    virtual QgsSymbolV2* subSymbol() override;
    virtual bool setSubSymbol( QgsSymbolV2* symbol ) override;
    virtual double estimateMaxBleed() const override;
    QSet<QString> usedAttributes() const override;
    void setColor( const QColor& c ) override;
    virtual QColor color() const override;

  private:

    virtual void draw( QgsSymbolV2RenderContext& context, Shape shape, const QPolygonF& polygon, const QPainterPath& path ) override;

    //! Fill subsymbol
    QScopedPointer< QgsFillSymbolV2 > mFill;
};

//////////

#define DEFAULT_SVGMARKER_NAME         "/crosses/Star1.svg"
#define DEFAULT_SVGMARKER_SIZE         2*DEFAULT_POINT_SIZE
#define DEFAULT_SVGMARKER_ANGLE        0

/** \ingroup core
 * \class QgsSvgMarkerSymbolLayerV2
 */
class CORE_EXPORT QgsSvgMarkerSymbolLayerV2 : public QgsMarkerSymbolLayerV2
{
  public:
    QgsSvgMarkerSymbolLayerV2( const QString& name = DEFAULT_SVGMARKER_NAME,
                               double size = DEFAULT_SVGMARKER_SIZE,
                               double angle = DEFAULT_SVGMARKER_ANGLE,
                               QgsSymbolV2::ScaleMethod scaleMethod = DEFAULT_SCALE_METHOD );

    // static stuff

    static QgsSymbolLayerV2* create( const QgsStringMap& properties = QgsStringMap() );
    static QgsSymbolLayerV2* createFromSld( QDomElement &element );

    // implemented from base classes

    QString layerType() const override;

    void startRender( QgsSymbolV2RenderContext& context ) override;

    void stopRender( QgsSymbolV2RenderContext& context ) override;

    void renderPoint( QPointF point, QgsSymbolV2RenderContext& context ) override;

    QgsStringMap properties() const override;

    QgsSvgMarkerSymbolLayerV2* clone() const override;

    void writeSldMarker( QDomDocument &doc, QDomElement &element, const QgsStringMap& props ) const override;

    QString path() const { return mPath; }
    void setPath( const QString& path );

    QColor fillColor() const override { return color(); }
    void setFillColor( const QColor& color ) override { setColor( color ); }

    QColor outlineColor() const override { return mOutlineColor; }
    void setOutlineColor( const QColor& c ) override { mOutlineColor = c; }

    double outlineWidth() const { return mOutlineWidth; }
    void setOutlineWidth( double w ) { mOutlineWidth = w; }

    void setOutlineWidthUnit( QgsSymbolV2::OutputUnit unit ) { mOutlineWidthUnit = unit; }
    QgsSymbolV2::OutputUnit outlineWidthUnit() const { return mOutlineWidthUnit; }

    void setOutlineWidthMapUnitScale( const QgsMapUnitScale& scale ) { mOutlineWidthMapUnitScale = scale; }
    const QgsMapUnitScale& outlineWidthMapUnitScale() const { return mOutlineWidthMapUnitScale; }

    void setOutputUnit( QgsSymbolV2::OutputUnit unit ) override;
    QgsSymbolV2::OutputUnit outputUnit() const override;

    void setMapUnitScale( const QgsMapUnitScale& scale ) override;
    QgsMapUnitScale mapUnitScale() const override;

    bool writeDxf( QgsDxfExport& e, double mmMapUnitScaleFactor, const QString& layerName, QgsSymbolV2RenderContext &context, QPointF shift = QPointF( 0.0, 0.0 ) ) const override;

    QRectF bounds( QPointF point, QgsSymbolV2RenderContext& context ) override;

  protected:
    QString mPath;

    //param(fill), param(outline), param(outline-width) are going
    //to be replaced in memory
    QColor mOutlineColor;
    double mOutlineWidth;
    QgsSymbolV2::OutputUnit mOutlineWidthUnit;
    QgsMapUnitScale mOutlineWidthMapUnitScale;

  private:
    double calculateSize( QgsSymbolV2RenderContext& context, bool& hasDataDefinedSize ) const;
    void calculateOffsetAndRotation( QgsSymbolV2RenderContext& context, double scaledSize, QPointF& offset, double& angle ) const;

};


//////////

#define POINT2MM(x) ( (x) * 25.4 / 72 ) // point is 1/72 of inch
#define MM2POINT(x) ( (x) * 72 / 25.4 )

#define DEFAULT_FONTMARKER_FONT   "Dingbats"
#define DEFAULT_FONTMARKER_CHR    QChar('A')
#define DEFAULT_FONTMARKER_SIZE   POINT2MM(12)
#define DEFAULT_FONTMARKER_COLOR  QColor(Qt::black)
#define DEFAULT_FONTMARKER_BORDERCOLOR  QColor(Qt::white)
#define DEFAULT_FONTMARKER_JOINSTYLE    Qt::MiterJoin
#define DEFAULT_FONTMARKER_ANGLE  0

/** \ingroup core
 * \class QgsFontMarkerSymbolLayerV2
 */
class CORE_EXPORT QgsFontMarkerSymbolLayerV2 : public QgsMarkerSymbolLayerV2
{
  public:
    QgsFontMarkerSymbolLayerV2( const QString& fontFamily = DEFAULT_FONTMARKER_FONT,
                                QChar chr = DEFAULT_FONTMARKER_CHR,
                                double pointSize = DEFAULT_FONTMARKER_SIZE,
                                const QColor& color = DEFAULT_FONTMARKER_COLOR,
                                double angle = DEFAULT_FONTMARKER_ANGLE );

    ~QgsFontMarkerSymbolLayerV2();

    // static stuff

    static QgsSymbolLayerV2* create( const QgsStringMap& properties = QgsStringMap() );
    static QgsSymbolLayerV2* createFromSld( QDomElement &element );

    // implemented from base classes

    QString layerType() const override;

    void startRender( QgsSymbolV2RenderContext& context ) override;

    void stopRender( QgsSymbolV2RenderContext& context ) override;

    void renderPoint( QPointF point, QgsSymbolV2RenderContext& context ) override;

    QgsStringMap properties() const override;

    QgsFontMarkerSymbolLayerV2* clone() const override;

    void writeSldMarker( QDomDocument &doc, QDomElement &element, const QgsStringMap& props ) const override;

    // new methods

    QString fontFamily() const { return mFontFamily; }
    void setFontFamily( const QString& family ) { mFontFamily = family; }

    QChar character() const { return mChr; }
    void setCharacter( QChar ch ) { mChr = ch; }

    /** Get outline color.
     * @note added in 2.16 */
    QColor outlineColor() const override { return mOutlineColor; }
    /** Set outline color.
     * @note added in 2.16 */
    void setOutlineColor( const QColor& color ) override { mOutlineColor = color; }

    /** Get outline width.
     * @note added in 2.16 */
    double outlineWidth() const { return mOutlineWidth; }
    /** Set outline width.
     * @note added in 2.16 */
    void setOutlineWidth( double width ) { mOutlineWidth = width; }

    /** Get outline width unit.
     * @note added in 2.16 */
    QgsSymbolV2::OutputUnit outlineWidthUnit() const { return mOutlineWidthUnit; }
    /** Set outline width unit.
     * @note added in 2.16 */
    void setOutlineWidthUnit( QgsSymbolV2::OutputUnit unit ) { mOutlineWidthUnit = unit; }

    /** Get outline width map unit scale.
     * @note added in 2.16 */
    const QgsMapUnitScale& outlineWidthMapUnitScale() const { return mOutlineWidthMapUnitScale; }
    /** Set outline width map unit scale.
     * @note added in 2.16 */
    void setOutlineWidthMapUnitScale( const QgsMapUnitScale& scale ) { mOutlineWidthMapUnitScale = scale; }

    /** Get outline join style.
     * @note added in 2.16 */
    Qt::PenJoinStyle penJoinStyle() const { return mPenJoinStyle; }
    /** Set outline join style.
     * @note added in 2.16 */
    void setPenJoinStyle( Qt::PenJoinStyle style ) { mPenJoinStyle = style; }

    QRectF bounds( QPointF point, QgsSymbolV2RenderContext& context ) override;

  protected:

    QString mFontFamily;
    QFontMetrics* mFontMetrics;
    QChar mChr;

    double mChrWidth;
    QPointF mChrOffset;
    QFont mFont;
    double mOrigSize;

  private:

    QColor mOutlineColor;
    double mOutlineWidth;
    QgsSymbolV2::OutputUnit mOutlineWidthUnit;
    QgsMapUnitScale mOutlineWidthMapUnitScale;
    Qt::PenJoinStyle mPenJoinStyle;

    QPen mPen;
    QBrush mBrush;

    QString characterToRender( QgsSymbolV2RenderContext& context, QPointF& charOffset, double& charWidth );
    void calculateOffsetAndRotation( QgsSymbolV2RenderContext& context, double scaledSize, bool& hasDataDefinedRotation, QPointF& offset, double& angle ) const;
    double calculateSize( QgsSymbolV2RenderContext& context );
};


#endif


