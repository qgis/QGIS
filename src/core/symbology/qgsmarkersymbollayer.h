/***************************************************************************
 qgsmarkersymbollayer.h
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

#ifndef QGSMARKERSYMBOLLAYER_H
#define QGSMARKERSYMBOLLAYER_H

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgssymbollayer.h"

#define DEFAULT_SIMPLEMARKER_NAME         "circle"
#define DEFAULT_SIMPLEMARKER_COLOR        QColor(255,0,0)
#define DEFAULT_SIMPLEMARKER_BORDERCOLOR  QColor( 35, 35, 35 )
#define DEFAULT_SIMPLEMARKER_JOINSTYLE    Qt::BevelJoin
#define DEFAULT_SIMPLEMARKER_SIZE         DEFAULT_POINT_SIZE
#define DEFAULT_SIMPLEMARKER_ANGLE        0

#include <QPen>
#include <QBrush>
#include <QPicture>
#include <QPolygonF>
#include <QFont>

class QgsFillSymbol;
class QgsPathResolver;

/**
 * \ingroup core
 * \class QgsSimpleMarkerSymbolLayerBase
 * \brief Abstract base class for simple marker symbol layers. Handles creation of the symbol shapes but
 * leaves the actual drawing of the symbols to subclasses.
 * \since QGIS 2.16
 */
class CORE_EXPORT QgsSimpleMarkerSymbolLayerBase : public QgsMarkerSymbolLayer
{

  public:

    //! Returns a list of all available shape types.
    static QList< Qgis::MarkerShape > availableShapes();

    /**
     * Returns TRUE if a symbol shape has a fill.
     * \param shape shape to test
     * \returns TRUE if shape uses a fill, or FALSE if shape uses lines only
     */
    static bool shapeIsFilled( Qgis::MarkerShape shape );

    /**
     * Constructor for QgsSimpleMarkerSymbolLayerBase.
    * \param shape symbol shape for markers
    * \param size symbol size (in mm)
    * \param angle symbol rotation angle
    * \param scaleMethod scaling method for data defined scaling
    */
    QgsSimpleMarkerSymbolLayerBase( Qgis::MarkerShape shape = Qgis::MarkerShape::Circle,
                                    double size = DEFAULT_SIMPLEMARKER_SIZE,
                                    double angle = DEFAULT_SIMPLEMARKER_ANGLE,
                                    Qgis::ScaleMethod scaleMethod = DEFAULT_SCALE_METHOD );

    ~QgsSimpleMarkerSymbolLayerBase() override;

    /**
     * Returns the shape for the rendered marker symbol.
     * \see setShape()
     */
    Qgis::MarkerShape shape() const { return mShape; }

    /**
     * Sets the rendered marker shape.
     * \param shape new marker shape
     * \see shape()
     */
    void setShape( Qgis::MarkerShape shape ) { mShape = shape; }

    /**
     * Attempts to decode a string representation of a shape name to the corresponding
     * shape.
     * \param name encoded shape name
     * \param ok if specified, will be set to TRUE if shape was successfully decoded
     * \returns decoded name
     * \see encodeShape()
     */
    static Qgis::MarkerShape decodeShape( const QString &name, bool *ok = nullptr );

    /**
     * Encodes a shape to its string representation.
     * \param shape shape to encode
     * \returns encoded string
     * \see decodeShape()
     */
    static QString encodeShape( Qgis::MarkerShape shape );

    void startRender( QgsSymbolRenderContext &context ) override;
    void stopRender( QgsSymbolRenderContext &context ) override;
    void renderPoint( QPointF point, QgsSymbolRenderContext &context ) override;
    QRectF bounds( QPointF point, QgsSymbolRenderContext &context ) override;

  protected:

    /**
     * Prepares the layer for drawing the specified shape (QPolygonF version)
     */
    bool prepareMarkerShape( Qgis::MarkerShape shape );

    /**
     * Prepares the layer for drawing the specified shape (QPainterPath version)
     */
    bool prepareMarkerPath( Qgis::MarkerShape symbol );

    /**
     * Creates a polygon representing the specified shape.
     * \param shape shape to create
     * \param polygon destination polygon for shape
     * \returns TRUE if shape was successfully stored in polygon
     */
    bool shapeToPolygon( Qgis::MarkerShape shape, QPolygonF &polygon ) const;

    /**
     * Calculates the desired size of the marker, considering data defined size overrides.
     * \param context symbol render context
     * \param hasDataDefinedSize will be set to TRUE if marker uses data defined sizes
     * \returns marker size, in original size units
     */
    double calculateSize( QgsSymbolRenderContext &context, bool &hasDataDefinedSize ) const;

    /**
     * Calculates the marker offset and rotation.
     * \param context symbol render context
     * \param scaledSize size of symbol to render
     * \param hasDataDefinedRotation will be set to TRUE if marker has data defined rotation
     * \param offset will be set to calculated marker offset (in painter units)
     * \param angle will be set to calculated marker angle
     */
    void calculateOffsetAndRotation( QgsSymbolRenderContext &context, double scaledSize, bool &hasDataDefinedRotation, QPointF &offset, double &angle ) const;

    //! Polygon of points in shape. If polygon is empty then shape is using mPath.
    QPolygonF mPolygon;

    //! Painter path representing shape. If mPolygon is empty then the shape is stored in mPath.
    QPainterPath mPath;

    //! Symbol shape
    Qgis::MarkerShape mShape;

  private:

    /**
     * Derived classes must implement draw() to handle drawing the generated shape onto the painter surface.
     * \param context symbol render context
     * \param shape shape to draw
     * \param polygon polygon representing transformed marker shape. May be empty, in which case the shape will be specified
     * in the path argument.
     * \param path transformed painter path representing shape to draw
     */
    virtual void draw( QgsSymbolRenderContext &context, Qgis::MarkerShape shape, const QPolygonF &polygon, const QPainterPath &path ) = 0 SIP_FORCE;
};

/**
 * \ingroup core
 * \class QgsSimpleMarkerSymbolLayer
 * \brief Simple marker symbol layer, consisting of a rendered shape with solid fill color and an stroke.
 */
class CORE_EXPORT QgsSimpleMarkerSymbolLayer : public QgsSimpleMarkerSymbolLayerBase
{
  public:

    /**
     * Constructor for QgsSimpleMarkerSymbolLayer.
    * \param shape symbol shape
    * \param size symbol size (in mm)
    * \param angle symbol rotation angle
    * \param scaleMethod scaling method for data defined scaling
    * \param color fill color for symbol
    * \param strokeColor stroke color for symbol
    * \param penJoinStyle join style for stroke pen
    */
    QgsSimpleMarkerSymbolLayer( Qgis::MarkerShape shape = Qgis::MarkerShape::Circle,
                                double size = DEFAULT_SIMPLEMARKER_SIZE,
                                double angle = DEFAULT_SIMPLEMARKER_ANGLE,
                                Qgis::ScaleMethod scaleMethod = DEFAULT_SCALE_METHOD,
                                const QColor &color = DEFAULT_SIMPLEMARKER_COLOR,
                                const QColor &strokeColor = DEFAULT_SIMPLEMARKER_BORDERCOLOR,
                                Qt::PenJoinStyle penJoinStyle = DEFAULT_SIMPLEMARKER_JOINSTYLE );

    ~QgsSimpleMarkerSymbolLayer() override;

    // static methods

    /**
     * Creates a new QgsSimpleMarkerSymbolLayer.
     * \param properties a property map containing symbol properties (see properties())
     * \returns new QgsSimpleMarkerSymbolLayer
     */
    static QgsSymbolLayer *create( const QVariantMap &properties = QVariantMap() ) SIP_FACTORY;

    /**
     * Creates a new QgsSimpleMarkerSymbolLayer from an SLD XML element.
     * \param element XML element containing SLD definition of symbol
     * \returns new QgsSimpleMarkerSymbolLayer
     */
    static QgsSymbolLayer *createFromSld( QDomElement &element ) SIP_FACTORY;

    // reimplemented from base classes

    QString layerType() const override;
    void startRender( QgsSymbolRenderContext &context ) override;
    void renderPoint( QPointF point, QgsSymbolRenderContext &context ) override;
    QVariantMap properties() const override;
    QgsSimpleMarkerSymbolLayer *clone() const override SIP_FACTORY;
    void writeSldMarker( QDomDocument &doc, QDomElement &element, const QVariantMap &props ) const override;
    QString ogrFeatureStyle( double mmScaleFactor, double mapUnitScaleFactor ) const override;
    bool writeDxf( QgsDxfExport &e, double mmMapUnitScaleFactor, const QString &layerName, QgsSymbolRenderContext &context, QPointF shift = QPointF( 0.0, 0.0 ) ) const override;
    void setOutputUnit( QgsUnitTypes::RenderUnit unit ) override;
    QgsUnitTypes::RenderUnit outputUnit() const override;
    void setMapUnitScale( const QgsMapUnitScale &scale ) override;
    QgsMapUnitScale mapUnitScale() const override;
    bool usesMapUnits() const override;
    QRectF bounds( QPointF point, QgsSymbolRenderContext &context ) override;
    QColor fillColor() const override { return mColor; }
    void setFillColor( const QColor &color ) override { mColor = color; }
    void setColor( const QColor &color ) override;
    QColor color() const override;

    // new methods

    /**
     * Returns the marker's stroke color.
     * \see setStrokeColor()
     * \see strokeStyle()
     * \see penJoinStyle()
     */
    QColor strokeColor() const override { return mStrokeColor; }

    /**
     * Sets the marker's stroke color.
     * \param color stroke color
     * \see strokeColor()
     * \see setStrokeStyle()
     * \see setPenJoinStyle()
     */
    void setStrokeColor( const QColor &color ) override { mStrokeColor = color; }

    /**
     * Returns the marker's stroke style (e.g., solid, dashed, etc)
     * \see setStrokeStyle()
     * \see strokeColor()
     * \see penJoinStyle()
     * \since QGIS 2.4
    */
    Qt::PenStyle strokeStyle() const { return mStrokeStyle; }

    /**
     * Sets the marker's stroke style (e.g., solid, dashed, etc)
     * \param strokeStyle style
     * \see strokeStyle()
     * \see setStrokeColor()
     * \see setPenJoinStyle()
     * \since QGIS 2.4
    */
    void setStrokeStyle( Qt::PenStyle strokeStyle ) { mStrokeStyle = strokeStyle; }

    /**
     * Returns the marker's stroke join style (e.g., miter, bevel, etc).
     * \see setPenJoinStyle()
     * \see penCapStyle()
     * \see strokeColor()
     * \see strokeStyle()
     * \since QGIS 2.16
    */
    Qt::PenJoinStyle penJoinStyle() const { return mPenJoinStyle; }

    /**
     * Sets the marker's stroke join style (e.g., miter, bevel, etc).
     * \param style join style
     * \see penJoinStyle()
     * \see setPenCapStyle()
     * \see setStrokeColor()
     * \see setStrokeStyle()
     * \since QGIS 2.16
    */
    void setPenJoinStyle( Qt::PenJoinStyle style ) { mPenJoinStyle = style; }

    /**
     * Returns the marker's stroke cap style (e.g., flat, round, etc).
     * \see setPenCapStyle()
     * \see penJoinStyle()
     * \see strokeColor()
     * \see strokeStyle()
     * \since QGIS 3.20
    */
    Qt::PenCapStyle penCapStyle() const { return mPenCapStyle; }

    /**
     * Sets the marker's stroke cap \a style (e.g., flat, round, etc).
     * \see penCapStyle()
     * \see penJoinStyle()
     * \see setStrokeColor()
     * \see setStrokeStyle()
     * \since QGIS 3.20
    */
    void setPenCapStyle( Qt::PenCapStyle style ) { mPenCapStyle = style; }

    /**
     * Returns the width of the marker's stroke.
     * \see setStrokeWidth()
     * \see strokeWidthUnit()
     * \see strokeWidthMapUnitScale()
     */
    double strokeWidth() const { return mStrokeWidth; }

    /**
     * Sets the width of the marker's stroke.
     * \param w stroke width. See strokeWidthUnit() for units.
     * \see strokeWidth()
     * \see setStrokeWidthUnit()
     * \see setStrokeWidthMapUnitScale()
     */
    void setStrokeWidth( double w ) { mStrokeWidth = w; }

    /**
     * Sets the unit for the width of the marker's stroke.
     * \param u stroke width unit
     * \see strokeWidthUnit()
     * \see setStrokeWidth()
     * \see setStrokeWidthMapUnitScale()
     */
    void setStrokeWidthUnit( QgsUnitTypes::RenderUnit u ) { mStrokeWidthUnit = u; }

    /**
     * Returns the unit for the width of the marker's stroke.
     * \see setStrokeWidthUnit()
     * \see strokeWidth()
     * \see strokeWidthMapUnitScale()
     */
    QgsUnitTypes::RenderUnit strokeWidthUnit() const { return mStrokeWidthUnit; }

    /**
     * Sets the map scale for the width of the marker's stroke.
     * \param scale stroke width map unit scale
     * \see strokeWidthMapUnitScale()
     * \see setStrokeWidth()
     * \see setStrokeWidthUnit()
     */
    void setStrokeWidthMapUnitScale( const QgsMapUnitScale &scale ) { mStrokeWidthMapUnitScale = scale; }

    /**
     * Returns the map scale for the width of the marker's stroke.
     * \see setStrokeWidthMapUnitScale()
     * \see strokeWidth()
     * \see strokeWidthUnit()
     */
    const QgsMapUnitScale &strokeWidthMapUnitScale() const { return mStrokeWidthMapUnitScale; }

  protected:

    /**
     * Draws the marker shape in the specified painter.
     * \param p destination QPainter
     * \param context symbol context
     * \note this method does not handle setting the painter pen or brush to match the symbol's fill or stroke
     */
    void drawMarker( QPainter *p, QgsSymbolRenderContext &context );

    /**
     * Prepares cache image
     * \returns TRUE in case of success, FALSE if cache image size too large
    */
    bool prepareCache( QgsSymbolRenderContext &context );

    //! Stroke color
    QColor mStrokeColor;
    //! Stroke style
    Qt::PenStyle mStrokeStyle = Qt::SolidLine;
    //! Stroke width
    double mStrokeWidth = 0;
    //! Stroke width units
    QgsUnitTypes::RenderUnit mStrokeWidthUnit = QgsUnitTypes::RenderMillimeters;
    //! Stroke width map unit scale
    QgsMapUnitScale mStrokeWidthMapUnitScale;
    //! Stroke pen join style
    Qt::PenJoinStyle mPenJoinStyle;
    //! Stroke pen cap style
    Qt::PenCapStyle mPenCapStyle = Qt::SquareCap;
    //! QPen corresponding to marker's stroke style
    QPen mPen;
    //! QBrush corresponding to marker's fill style
    QBrush mBrush;

    //! Cached image of marker, if using cached version
    QImage mCache;
    //! QPen to use as stroke of selected symbols
    QPen mSelPen;
    //! QBrush to use as fill of selected symbols
    QBrush mSelBrush;
    //! Cached image of selected marker, if using cached version
    QImage mSelCache;

    /**
     * TRUE if using cached images of markers for drawing. This is faster, but cannot
     * be used when data defined properties are present
     */
    bool mUsingCache = false;

    //! Maximum width/height of cache image
    static const int MAXIMUM_CACHE_WIDTH = 3000;

  private:
    // cppcheck-suppress unusedPrivateFunction
    void draw( QgsSymbolRenderContext &context, Qgis::MarkerShape shape, const QPolygonF &polygon, const QPainterPath &path ) override SIP_FORCE;

    double mCachedOpacity = 1.0;

};

/**
 * \ingroup core
 * \class QgsFilledMarkerSymbolLayer
 * \brief Filled marker symbol layer, consisting of a shape which is rendered using a QgsFillSymbol. This allows
 * the symbol to support advanced styling of the interior and stroke of the shape.
 * \since QGIS 2.16
 */
class CORE_EXPORT QgsFilledMarkerSymbolLayer : public QgsSimpleMarkerSymbolLayerBase
{
  public:

    /**
     * Constructor for QgsFilledMarkerSymbolLayer.
    * \param shape symbol shape
    * \param size symbol size (in mm)
    * \param angle symbol rotation angle
    * \param scaleMethod size scaling method
    */
    QgsFilledMarkerSymbolLayer( Qgis::MarkerShape shape = Qgis::MarkerShape::Circle,
                                double size = DEFAULT_SIMPLEMARKER_SIZE,
                                double angle = DEFAULT_SIMPLEMARKER_ANGLE,
                                Qgis::ScaleMethod scaleMethod = DEFAULT_SCALE_METHOD );

    ~QgsFilledMarkerSymbolLayer() override;

    /**
     * Creates a new QgsFilledMarkerSymbolLayer.
     * \param properties a property map containing symbol properties (see properties())
     * \returns new QgsFilledMarkerSymbolLayer
     */
    static QgsSymbolLayer *create( const QVariantMap &properties = QVariantMap() ) SIP_FACTORY;

    QString layerType() const override;
    void startRender( QgsSymbolRenderContext &context ) override;
    void stopRender( QgsSymbolRenderContext &context ) override;
    QVariantMap properties() const override;
    QgsFilledMarkerSymbolLayer *clone() const override SIP_FACTORY;
    QgsSymbol *subSymbol() override;
    bool setSubSymbol( QgsSymbol *symbol SIP_TRANSFER ) override;
    double estimateMaxBleed( const QgsRenderContext &context ) const override;
    QSet<QString> usedAttributes( const QgsRenderContext &context ) const override;
    bool hasDataDefinedProperties() const override;
    void setColor( const QColor &c ) override;
    QColor color() const override;
    bool usesMapUnits() const override;

  private:
#ifdef SIP_RUN
    QgsFilledMarkerSymbolLayer( const QgsFilledMarkerSymbolLayer & );
#endif

    // cppcheck-suppress unusedPrivateFunction
    void draw( QgsSymbolRenderContext &context, Qgis::MarkerShape shape, const QPolygonF &polygon, const QPainterPath &path ) override SIP_FORCE;

    //! Fill subsymbol
    std::unique_ptr< QgsFillSymbol > mFill;
};

//////////

#define DEFAULT_SVGMARKER_SIZE         2*DEFAULT_POINT_SIZE
#define DEFAULT_SVGMARKER_ANGLE        0

/**
 * \ingroup core
 * \class QgsSvgMarkerSymbolLayer
 */
class CORE_EXPORT QgsSvgMarkerSymbolLayer : public QgsMarkerSymbolLayer
{
  public:
    //! Constructs SVG marker symbol layer with picture from given absolute path to a SVG file
    QgsSvgMarkerSymbolLayer( const QString &path,
                             double size = DEFAULT_SVGMARKER_SIZE,
                             double angle = DEFAULT_SVGMARKER_ANGLE,
                             Qgis::ScaleMethod scaleMethod = DEFAULT_SCALE_METHOD );

    ~QgsSvgMarkerSymbolLayer() override;

    // static stuff

    //! Creates the symbol
    static QgsSymbolLayer *create( const QVariantMap &properties = QVariantMap() ) SIP_FACTORY;
    static QgsSymbolLayer *createFromSld( QDomElement &element ) SIP_FACTORY;

    /**
     * Turns relative paths in properties map to absolute when reading and vice versa when writing.
     * Used internally when reading/writing symbols.
     * \since QGIS 3.0
     */
    static void resolvePaths( QVariantMap &properties, const QgsPathResolver &pathResolver, bool saving );

    // implemented from base classes

    QString layerType() const override;

    void startRender( QgsSymbolRenderContext &context ) override;

    void stopRender( QgsSymbolRenderContext &context ) override;

    void renderPoint( QPointF point, QgsSymbolRenderContext &context ) override;

    QVariantMap properties() const override;
    bool usesMapUnits() const override;

    QgsSvgMarkerSymbolLayer *clone() const override SIP_FACTORY;

    void writeSldMarker( QDomDocument &doc, QDomElement &element, const QVariantMap &props ) const override;

    /**
     * Returns the marker SVG path.
     * \see setPath()
     */
    QString path() const { return mPath; }

    /**
     * Set the marker SVG path.
     * \param path SVG path
     * \see path()
     */
    void setPath( const QString &path );

    /**
     * Returns the default marker aspect ratio between width and height, 0 if not yet calculated.
     * \see updateDefaultAspectRatio()
     */
    double defaultAspectRatio() const { return mDefaultAspectRatio; }

    /**
     * Calculates the default marker aspect ratio between width and height.
     * \returns the default aspect ratio value
     * \see defaultAspectRatio()
     */
    double updateDefaultAspectRatio();

    /**
     * Returns the preserved aspect ratio value, TRUE if fixed aspect ratio has been lower or equal to 0.
     * \see setPreservedAspectRatio()
     */
    bool preservedAspectRatio() const { return mFixedAspectRatio <= 0.0; }

    /**
     * Set preserved the marker aspect ratio between width and height.
     * \param par Preserved Aspect Ratio
     * \returns the preserved aspect ratio value, TRUE if fixed aspect ratio has been lower or equal to 0
     * \see preservedAspectRatio()
     */
    bool setPreservedAspectRatio( bool par );

    /**
     * Returns the marker aspect ratio between width and height to be used in rendering,
     * if the value set is lower or equal to 0 the aspect ratio will be preserved in rendering
     * \see setFixedAspectRatio() QgsSvgCache
     */
    double fixedAspectRatio() const { return mFixedAspectRatio; }

    /**
     * Set the marker aspect ratio between width and height to be used in rendering,
     * if the value set is lower or equal to 0 the aspect ratio will be preserved in rendering
     * \param ratio Fixed Aspect Ratio
     * \see fixedAspectRatio() QgsSvgCache
     */
    void setFixedAspectRatio( double ratio ) { mFixedAspectRatio = ratio; }

    QColor fillColor() const override { return color(); }
    void setFillColor( const QColor &color ) override { setColor( color ); }

    QColor strokeColor() const override { return mStrokeColor; }
    void setStrokeColor( const QColor &c ) override { mStrokeColor = c; }

    double strokeWidth() const { return mStrokeWidth; }
    void setStrokeWidth( double w ) { mStrokeWidth = w; }

    /**
     * Returns the dynamic SVG parameters
     * \since QGIS 3.18
     */
    QMap<QString, QgsProperty> parameters() const { return mParameters; }

    /**
     * Sets the dynamic SVG parameters
     * \since QGIS 3.18
     */
    void setParameters( const QMap<QString, QgsProperty> &parameters );

    /**
     * Sets the units for the stroke width.
     * \param unit width units
     * \see strokeWidthUnit()
    */
    void setStrokeWidthUnit( QgsUnitTypes::RenderUnit unit ) { mStrokeWidthUnit = unit; }

    /**
     * Returns the units for the stroke width.
     * \see strokeWidthUnit()
    */
    QgsUnitTypes::RenderUnit strokeWidthUnit() const { return mStrokeWidthUnit; }

    void setStrokeWidthMapUnitScale( const QgsMapUnitScale &scale ) { mStrokeWidthMapUnitScale = scale; }
    const QgsMapUnitScale &strokeWidthMapUnitScale() const { return mStrokeWidthMapUnitScale; }

    void setOutputUnit( QgsUnitTypes::RenderUnit unit ) override;
    QgsUnitTypes::RenderUnit outputUnit() const override;

    void setMapUnitScale( const QgsMapUnitScale &scale ) override;
    QgsMapUnitScale mapUnitScale() const override;

    bool writeDxf( QgsDxfExport &e, double mmMapUnitScaleFactor, const QString &layerName, QgsSymbolRenderContext &context, QPointF shift = QPointF( 0.0, 0.0 ) ) const override;

    QRectF bounds( QPointF point, QgsSymbolRenderContext &context ) override;

    void prepareExpressions( const QgsSymbolRenderContext &context ) override;

    QSet<QString> usedAttributes( const QgsRenderContext &context ) const override;

  protected:

    /**
     * Calculates the marker aspect ratio between width and height.
     * \param context symbol render context
     * \param scaledSize size of symbol to render
     * \param hasDataDefinedAspectRatio will be set to TRUE if marker has data defined aspectRatio
     */
    double calculateAspectRatio( QgsSymbolRenderContext &context, double scaledSize, bool &hasDataDefinedAspectRatio ) const;

    QString mPath;

    //! The marker default aspect ratio
    double mDefaultAspectRatio = 0.0;
    //! The marker fixed aspect ratio
    double mFixedAspectRatio = 0.0;
    //param(fill), param(stroke), param(stroke-width) are going
    //to be replaced in memory
    bool mHasFillParam = false;
    QColor mStrokeColor;
    double mStrokeWidth;
    QMap<QString, QgsProperty> mParameters;

    QgsUnitTypes::RenderUnit mStrokeWidthUnit;
    QgsMapUnitScale mStrokeWidthMapUnitScale;

  private:
    double calculateSize( QgsSymbolRenderContext &context, bool &hasDataDefinedSize ) const;
    void calculateOffsetAndRotation( QgsSymbolRenderContext &context, double scaledWidth, double scaledHeight, QPointF &offset, double &angle ) const;

};


//////////

#define DEFAULT_RASTERMARKER_SIZE         2*DEFAULT_POINT_SIZE
#define DEFAULT_RASTERMARKER_ANGLE        0

/**
 * \ingroup core
 * \class QgsRasterMarkerSymbolLayer
 * \brief Raster marker symbol layer class.
 * \since QGIS 3.6
 */
class CORE_EXPORT QgsRasterMarkerSymbolLayer : public QgsMarkerSymbolLayer
{
  public:
    //! Constructs raster marker symbol layer with picture from given absolute path to a raster image file
    QgsRasterMarkerSymbolLayer( const QString &path = QString(),
                                double size = DEFAULT_SVGMARKER_SIZE,
                                double angle = DEFAULT_SVGMARKER_ANGLE,
                                Qgis::ScaleMethod scaleMethod = DEFAULT_SCALE_METHOD );

    ~QgsRasterMarkerSymbolLayer() override;

    // static stuff

    /**
     * Creates a raster marker symbol layer from a string map of properties.
     * \param properties QVariantMap properties object
     */
    static QgsSymbolLayer *create( const QVariantMap &properties = QVariantMap() ) SIP_FACTORY;

    /**
     * Turns relative paths in properties map to absolute when reading and vice versa when writing.
     * Used internally when reading/writing symbols.
     * \since QGIS 3.0
     */
    static void resolvePaths( QVariantMap &properties, const QgsPathResolver &pathResolver, bool saving );

    // implemented from base classes

    QString layerType() const override;

    void renderPoint( QPointF point, QgsSymbolRenderContext &context ) override;
    QVariantMap properties() const override;
    QgsRasterMarkerSymbolLayer *clone() const override SIP_FACTORY;
    bool usesMapUnits() const override;
    QColor color() const override;

    /**
     * Calculates the marker aspect ratio between width and height.
     * \param context symbol render context
     * \param scaledSize size of symbol to render
     * \param hasDataDefinedAspectRatio will be set to TRUE if marker has data defined aspectRatio
     */
    double calculateAspectRatio( QgsSymbolRenderContext &context, double scaledSize, bool &hasDataDefinedAspectRatio ) const;

    /**
     * Returns the marker raster image path.
     * \see setPath()
     */
    QString path() const { return mPath; }

    /**
     * Set the marker raster image path.
     * \param path raster image path
     * \see path()
     */
    void setPath( const QString &path );

    /**
     * Returns the marker opacity.
     * \returns opacity value between 0 (fully transparent) and 1 (fully opaque)
     * \see setOpacity()
     */
    double opacity() const { return mOpacity; }

    /**
     * Set the marker opacity.
     * \param opacity opacity value between 0 (fully transparent) and 1 (fully opaque)
     * \see opacity()
     */
    void setOpacity( double opacity ) { mOpacity = opacity; }

    /**
     * Returns the default marker aspect ratio between width and height, 0 if not yet calculated.
     * \see updateDefaultAspectRatio()
     */
    double defaultAspectRatio() const { return mDefaultAspectRatio; }

    /**
     * Calculates the default marker aspect ratio between width and height.
     * \returns the default aspect ratio value
     * \see defaultAspectRatio()
     */
    double updateDefaultAspectRatio();

    /**
     * Returns the preserved aspect ratio value, TRUE if fixed aspect ratio has been lower or equal to 0.
     * \see setPreservedAspectRatio()
     */
    bool preservedAspectRatio() const { return mFixedAspectRatio <= 0.0; }

    /**
     * Set preserved the marker aspect ratio between width and height.
     * \param par Preserved Aspect Ratio
     * \returns the preserved aspect ratio value, TRUE if fixed aspect ratio has been lower or equal to 0
     * \see preservedAspectRatio()
     */
    bool setPreservedAspectRatio( bool par );

    /**
     * Returns the marker aspect ratio between width and height to be used in rendering,
     * if the value set is lower or equal to 0 the aspect ratio will be preserved in rendering
     * \see setFixedAspectRatio() QgsSvgCache
     */
    double fixedAspectRatio() const { return mFixedAspectRatio; }

    /**
     * Set the marker aspect ratio between width and height to be used in rendering,
     * if the value set is lower or equal to 0 the aspect ratio will be preserved in rendering
     * \param ratio Fixed Aspect Ratio
     * \see fixedAspectRatio() QgsSvgCache
     */
    void setFixedAspectRatio( double ratio ) { mFixedAspectRatio = ratio; }

    void setMapUnitScale( const QgsMapUnitScale &scale ) override;
    QgsMapUnitScale mapUnitScale() const override;

    QRectF bounds( QPointF point, QgsSymbolRenderContext &context ) override;

  protected:

    /**
     * Sets common class properties from a \a properties map.
     *
     * \since QGIS 3.26
     */
    void setCommonProperties( const QVariantMap &properties );

    /**
     * Copies common properties to another layer.
     *
     * \since QGIS 3.26
     */
    void copyCommonProperties( QgsRasterMarkerSymbolLayer *other ) const;

    /**
     * Fetches the image to render.
     *
     * \note Not available in Python bindings
     */
    virtual QImage fetchImage( QgsRenderContext &context, const QString &path, QSize size, bool preserveAspectRatio, double opacity ) const SIP_SKIP;

    QString mPath;
    //! The marker default opacity
    double mOpacity = 1.0;
    //! The marker default aspect ratio
    double mDefaultAspectRatio = 0.0;
    //! The marker fixed aspect ratio
    double mFixedAspectRatio = 0.0;

  private:
    double calculateSize( QgsSymbolRenderContext &context, bool &hasDataDefinedSize ) const;
    void calculateOffsetAndRotation( QgsSymbolRenderContext &context, double scaledWidth, double scaledHeight, QPointF &offset, double &angle ) const;

};


//////////

#define POINT2MM(x) ( (x) * 25.4 / 72 ) // point is 1/72 of inch
#define MM2POINT(x) ( (x) * 72 / 25.4 )

#define DEFAULT_FONTMARKER_FONT   "Dingbats"
#define DEFAULT_FONTMARKER_CHR    QChar('A')
#define DEFAULT_FONTMARKER_SIZE   POINT2MM(12)
#define DEFAULT_FONTMARKER_COLOR  QColor( 35, 35, 35 )
#define DEFAULT_FONTMARKER_BORDERCOLOR  QColor(Qt::white)
#define DEFAULT_FONTMARKER_JOINSTYLE    Qt::MiterJoin
#define DEFAULT_FONTMARKER_ANGLE  0

/**
 * \ingroup core
 * \class QgsFontMarkerSymbolLayer
 */
class CORE_EXPORT QgsFontMarkerSymbolLayer : public QgsMarkerSymbolLayer
{
  public:

    //! Constructs a font marker symbol layer.
    QgsFontMarkerSymbolLayer( const QString &fontFamily = DEFAULT_FONTMARKER_FONT,
                              QString chr = DEFAULT_FONTMARKER_CHR,
                              double pointSize = DEFAULT_FONTMARKER_SIZE,
                              const QColor &color = DEFAULT_FONTMARKER_COLOR,
                              double angle = DEFAULT_FONTMARKER_ANGLE );

    ~QgsFontMarkerSymbolLayer() override;

    // static stuff

    /**
     * Creates a new QgsFontMarkerSymbolLayer from a property map (see properties())
     */
    static QgsSymbolLayer *create( const QVariantMap &properties = QVariantMap() ) SIP_FACTORY;

    /**
     * Creates a new QgsFontMarkerSymbolLayer from an SLD XML \a element.
     */
    static QgsSymbolLayer *createFromSld( QDomElement &element ) SIP_FACTORY;

    /**
     * Resolves fonts from a \a properties map, raising warnings in the specified \a context if the
     * required fonts are not available on the system.
     *
     * \since QGIS 3.20
     */
    static void resolveFonts( const QVariantMap &properties, const QgsReadWriteContext &context );

    // implemented from base classes

    QString layerType() const override;

    void startRender( QgsSymbolRenderContext &context ) override;

    void stopRender( QgsSymbolRenderContext &context ) override;

    void renderPoint( QPointF point, QgsSymbolRenderContext &context ) override;

    QVariantMap properties() const override;

    QgsFontMarkerSymbolLayer *clone() const override SIP_FACTORY;

    void writeSldMarker( QDomDocument &doc, QDomElement &element, const QVariantMap &props ) const override;
    bool usesMapUnits() const override;

    // new methods

    /**
     * Returns the font family name for the associated font which will be used to render the point.
     *
     * \see setFontFamily()
     */
    QString fontFamily() const { return mFontFamily; }

    /**
     * Sets the font \a family for the font which will be used to render the point.
     *
     * \see fontFamily()
     */
    void setFontFamily( const QString &family ) { mFontFamily = family; }

    /**
     * Returns the font style for the associated font which will be used to render the point.
     *
     * \see setFontStyle()
     * \since QGIS 3.14
     */
    QString fontStyle() const { return mFontStyle; }

    /**
     * Sets the font \a style for the font which will be used to render the point.
     *
     * \see fontStyle()
     * \since QGIS 3.14
     */
    void setFontStyle( const QString &style ) { mFontStyle = style; }

    /**
     * Returns the character(s) used when rendering points.
     *
     * \see setCharacter()
     */
    QString character() const { return mString; }

    /**
     * Sets the character(s) used when rendering points.
     *
     * \see character()
     */
    void setCharacter( QString chr ) { mString = chr; }

    QColor strokeColor() const override { return mStrokeColor; }
    void setStrokeColor( const QColor &color ) override { mStrokeColor = color; }

    /**
     * Returns the marker's stroke width. Units are retrieved by strokeWidthUnit()
     *
     * \see setStrokeWidth()
     * \see strokeWidthUnit()
     * \see strokeWidthMapUnitScale()
     *
     * \since QGIS 2.16
    */
    double strokeWidth() const { return mStrokeWidth; }

    /**
     * Set's the marker's stroke \a width. Units are set by setStrokeWidthUnit().
     *
     * \see strokeWidth()
     * \see setStrokeWidthUnit()
     * \see setStrokeWidthMapUnitScale()
     *
     * \since QGIS 2.16
    */
    void setStrokeWidth( double width ) { mStrokeWidth = width; }

    /**
     * Returns the stroke width unit.
     *
     * \see setStrokeWidthUnit()
     * \see strokeWidth()
     * \see strokeWidthMapUnitScale()
     *
     * \since QGIS 2.16
    */
    QgsUnitTypes::RenderUnit strokeWidthUnit() const { return mStrokeWidthUnit; }

    /**
     * Sets the stroke width \a unit.
     *
     * \see strokeWidthUnit()
     * \see setStrokeWidth()
     * \see setStrokeWidthMapUnitScale()
     * \since QGIS 2.16
    */
    void setStrokeWidthUnit( QgsUnitTypes::RenderUnit unit ) { mStrokeWidthUnit = unit; }

    /**
     * Returns the stroke width map unit scale.
     *
     * \see setStrokeWidthMapUnitScale()
     * \see strokeWidth()
     * \see strokeWidthUnit()
     *
     * \since QGIS 2.16
    */
    const QgsMapUnitScale &strokeWidthMapUnitScale() const { return mStrokeWidthMapUnitScale; }

    /**
     * Sets the stroke width map unit \a scale.
     *
     * \see strokeWidthMapUnitScale()
     * \see setStrokeWidth()
     * \see setStrokeWidthUnit()
     *
     * \since QGIS 2.16
     */
    void setStrokeWidthMapUnitScale( const QgsMapUnitScale &scale ) { mStrokeWidthMapUnitScale = scale; }

    /**
     * Returns the stroke join style.
     *
     * \see setPenJoinStyle()
     *
     * \since QGIS 2.16
    */
    Qt::PenJoinStyle penJoinStyle() const { return mPenJoinStyle; }

    /**
     * Sets the stroke join \a style.
     *
     * \see penJoinStyle()
     *
     * \since QGIS 2.16
    */
    void setPenJoinStyle( Qt::PenJoinStyle style ) { mPenJoinStyle = style; }

    QRectF bounds( QPointF point, QgsSymbolRenderContext &context ) override;

  private:

    QString mFontFamily;
    QString mFontStyle;
    QFont mFont;
    std::unique_ptr< QFontMetrics >mFontMetrics;

    QString mString;

    double mChrWidth = 0;
    QPointF mChrOffset;
    //! Scaling for font sizes, used if font size grows too large
    double mFontSizeScale = 1.0;
    double mOrigSize;

    QColor mStrokeColor;
    double mStrokeWidth;
    QgsUnitTypes::RenderUnit mStrokeWidthUnit;
    QgsMapUnitScale mStrokeWidthMapUnitScale;
    Qt::PenJoinStyle mPenJoinStyle;

    QPen mPen;
    QBrush mBrush;

    bool mUseCachedPath = false;
    QPainterPath mCachedPath;

    // If font has a zero (or nearly zero) size, we skip rendering altogether..
    bool mNonZeroFontSize = true;

    QString characterToRender( QgsSymbolRenderContext &context, QPointF &charOffset, double &charWidth );
    void calculateOffsetAndRotation( QgsSymbolRenderContext &context, double scaledSize, bool &hasDataDefinedRotation, QPointF &offset, double &angle ) const;
    double calculateSize( QgsSymbolRenderContext &context );
};


/**
 * \ingroup core
 * \class QgsAnimatedMarkerSymbolLayer
 * \brief Animated marker symbol layer class.
 *
 * The QgsAnimatedMarkerSymbolLayer class renders frames from an animated raster image source (e.g.
 * an animated GIF) during temporal animations or map exports.
 *
 * \since QGIS 3.26
 */
class CORE_EXPORT QgsAnimatedMarkerSymbolLayer : public QgsRasterMarkerSymbolLayer
{
  public:

    /**
     * Constructor for animated marker symbol layer using the specified source image \a path.
     */
    QgsAnimatedMarkerSymbolLayer( const QString &path = QString(),
                                  double size = DEFAULT_RASTERMARKER_SIZE,
                                  double angle = DEFAULT_RASTERMARKER_ANGLE );

    ~QgsAnimatedMarkerSymbolLayer() override;

    // static stuff

    /**
     * Creates an animated marker symbol layer from a string map of \a properties.
     */
    static QgsSymbolLayer *create( const QVariantMap &properties = QVariantMap() ) SIP_FACTORY;

    // implemented from base classes

    QString layerType() const override;
    QVariantMap properties() const override;
    QgsAnimatedMarkerSymbolLayer *clone() const override SIP_FACTORY;

    /**
     * Sets the marker frame \a rate in frame per second.
     *
     * \see frameRate()
     */
    void setFrameRate( double rate ) { mFrameRateFps = rate; }

    /**
     * Returns the marker frame rate in frame per second.
     *
     * \see setFrameRate()
     */
    double frameRate() const { return mFrameRateFps; }

    void startRender( QgsSymbolRenderContext &context ) override;

  protected:
    QImage fetchImage( QgsRenderContext &context, const QString &path, QSize size, bool preserveAspectRatio, double opacity ) const override SIP_SKIP;

  private:
    double mFrameRateFps = 10;
    bool mStaticPath = false;
    mutable QSet< QString > mPreparedPaths;

};

// clazy:excludeall=qstring-allocations

#endif


