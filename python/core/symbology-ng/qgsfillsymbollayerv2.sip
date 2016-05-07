class QgsSimpleFillSymbolLayerV2 : QgsFillSymbolLayerV2
{
%TypeHeaderCode
#include <qgsfillsymbollayerv2.h>
%End

  public:
    QgsSimpleFillSymbolLayerV2( const QColor& color = DEFAULT_SIMPLEFILL_COLOR,
                                Qt::BrushStyle style = DEFAULT_SIMPLEFILL_STYLE,
                                const QColor& borderColor = DEFAULT_SIMPLEFILL_BORDERCOLOR,
                                Qt::PenStyle borderStyle = DEFAULT_SIMPLEFILL_BORDERSTYLE,
                                double borderWidth = DEFAULT_SIMPLEFILL_BORDERWIDTH,
                                Qt::PenJoinStyle penJoinStyle = DEFAULT_SIMPLEFILL_JOINSTYLE
                              );

    // static stuff

    static QgsSymbolLayerV2* create( const QgsStringMap& properties = QgsStringMap() ) /Factory/;
    static QgsSymbolLayerV2* createFromSld( QDomElement &element ) /Factory/;

    // implemented from base classes

    QString layerType() const;

    void startRender( QgsSymbolV2RenderContext& context );

    void stopRender( QgsSymbolV2RenderContext& context );

    void renderPolygon( const QPolygonF& points, QList<QPolygonF>* rings, QgsSymbolV2RenderContext& context );

    QgsStringMap properties() const;

    virtual QgsSimpleFillSymbolLayerV2* clone() const /Factory/;

    void toSld( QDomDocument &doc, QDomElement &element, const QgsStringMap& props ) const;

    QString ogrFeatureStyle( double mmScaleFactor, double mapUnitScaleFactor ) const;

    Qt::BrushStyle brushStyle() const;
    void setBrushStyle( Qt::BrushStyle style );

    QColor borderColor() const;
    void setBorderColor( const QColor& borderColor );

    /** Get outline color.
     * @note added in 2.1 */
    QColor outlineColor() const;
    /** Set outline color.
     * @note added in 2.1 */
    void setOutlineColor( const QColor& color );

    /** Get fill color.
     * @note added in 2.1 */
    QColor fillColor() const;
    /** Set fill color.
     * @note added in 2.1 */
    void setFillColor( const QColor& color );

    Qt::PenStyle borderStyle() const;
    void setBorderStyle( Qt::PenStyle borderStyle );

    double borderWidth() const;
    void setBorderWidth( double borderWidth );

    Qt::PenJoinStyle penJoinStyle() const;
    void setPenJoinStyle( Qt::PenJoinStyle style );

    void setOffset( QPointF offset );
    QPointF offset();

    void setBorderWidthUnit( QgsSymbolV2::OutputUnit unit );
    QgsSymbolV2::OutputUnit borderWidthUnit() const;

    void setBorderWidthMapUnitScale( const QgsMapUnitScale& scale);
    const QgsMapUnitScale& borderWidthMapUnitScale() const;

    void setOffsetUnit( QgsSymbolV2::OutputUnit unit );
    QgsSymbolV2::OutputUnit offsetUnit() const;

    void setOffsetMapUnitScale( const QgsMapUnitScale& scale);
    const QgsMapUnitScale& offsetMapUnitScale() const;

    void setOutputUnit( QgsSymbolV2::OutputUnit unit );
    QgsSymbolV2::OutputUnit outputUnit() const;

    void setMapUnitScale( const QgsMapUnitScale &scale );
    QgsMapUnitScale mapUnitScale() const;

    double estimateMaxBleed() const;

    double dxfWidth( const QgsDxfExport& e, QgsSymbolV2RenderContext& context ) const;
    QColor dxfColor( QgsSymbolV2RenderContext& context ) const;
    Qt::PenStyle dxfPenStyle() const;
    QColor dxfBrushColor( QgsSymbolV2RenderContext& context ) const;
    Qt::BrushStyle dxfBrushStyle() const;
};

class QgsGradientFillSymbolLayerV2 : QgsFillSymbolLayerV2
{
%TypeHeaderCode
#include <qgsfillsymbollayerv2.h>
%End
  public:

    enum GradientColorType
    {
      SimpleTwoColor,
      ColorRamp
    };

    enum GradientType
    {
      Linear,
      Radial,
      Conical
    };

    enum GradientCoordinateMode
    {
      Feature,
      Viewport
    };

    enum GradientSpread
    {
      Pad,
      Reflect,
      Repeat
    };

    QgsGradientFillSymbolLayerV2( const QColor& color = DEFAULT_SIMPLEFILL_COLOR,
                                  const QColor& color2 = Qt::white,
                                  GradientColorType gradientColorType = SimpleTwoColor,
                                  GradientType gradientType = Linear,
                                  GradientCoordinateMode coordinateMode = Feature,
                                  GradientSpread gradientSpread = Pad
                                );

    virtual ~QgsGradientFillSymbolLayerV2();

    // static stuff

    static QgsSymbolLayerV2* create( const QgsStringMap& properties = QgsStringMap() ) /Factory/;

    // implemented from base classes

    QString layerType() const;

    void startRender( QgsSymbolV2RenderContext& context );

    void stopRender( QgsSymbolV2RenderContext& context );

    void renderPolygon( const QPolygonF& points, QList<QPolygonF>* rings, QgsSymbolV2RenderContext& context );

    QgsStringMap properties() const;

    virtual QgsGradientFillSymbolLayerV2* clone() const /Factory/;

    double estimateMaxBleed() const;

    /** Type of gradient, eg linear or radial*/
    GradientType gradientType() const;
    void setGradientType( GradientType gradientType );

    /** Gradient color mode, controls how gradient color stops are created*/
    GradientColorType gradientColorType() const;
    void setGradientColorType( GradientColorType gradientColorType );

    /** Color ramp used for the gradient fill, only used if the gradient color type is set to ColorRamp*/
    QgsVectorColorRampV2* colorRamp();
    void setColorRamp( QgsVectorColorRampV2* ramp );

    /** Color for endpoint of gradient, only used if the gradient color type is set to SimpleTwoColor*/
    QColor color2() const;
    void setColor2( const QColor& color2 );

    /** Coordinate mode for gradient. Controls how the gradient stops are positioned.*/
    GradientCoordinateMode coordinateMode() const;
    void setCoordinateMode( GradientCoordinateMode coordinateMode );

    /** Gradient spread mode. Controls how the gradient behaves outside of the predefined stops*/
    GradientSpread gradientSpread() const;
    void setGradientSpread( GradientSpread gradientSpread );

    /** Starting point of gradient fill, in the range [0,0] - [1,1]*/
    void setReferencePoint1( QPointF referencePoint );
    QPointF referencePoint1() const;

    /** Sets the starting point of the gradient to be the feature centroid*/
    void setReferencePoint1IsCentroid( bool isCentroid );
    bool referencePoint1IsCentroid() const;

    /** End point of gradient fill, in the range [0,0] - [1,1]*/
    void setReferencePoint2( QPointF referencePoint );
    QPointF referencePoint2() const;

    /** Sets the end point of the gradient to be the feature centroid*/
    void setReferencePoint2IsCentroid( bool isCentroid );
    bool referencePoint2IsCentroid() const;

    /** Offset for gradient fill*/
    void setOffset( QPointF offset );
    QPointF offset() const;

    /** Units for gradient fill offset*/
    void setOffsetUnit( QgsSymbolV2::OutputUnit unit );
    QgsSymbolV2::OutputUnit offsetUnit() const;

    void setOffsetMapUnitScale( const QgsMapUnitScale& scale);
    const QgsMapUnitScale& offsetMapUnitScale() const;

    void setOutputUnit( QgsSymbolV2::OutputUnit unit );
    QgsSymbolV2::OutputUnit outputUnit() const;

    void setMapUnitScale( const QgsMapUnitScale &scale );
    QgsMapUnitScale mapUnitScale() const;

};

class QgsShapeburstFillSymbolLayerV2 : QgsFillSymbolLayerV2
{
%TypeHeaderCode
#include <qgsfillsymbollayerv2.h>
%End
  public:

    enum ShapeburstColorType
    {
      SimpleTwoColor,
      ColorRamp
    };

    QgsShapeburstFillSymbolLayerV2( const QColor& color = DEFAULT_SIMPLEFILL_COLOR, const QColor& color2 = Qt::white,
                                    ShapeburstColorType colorType = SimpleTwoColor,
                                    int blurRadius = 0, bool useWholeShape = true, double maxDistance = 5 );

    virtual ~QgsShapeburstFillSymbolLayerV2();

    // static stuff

    static QgsSymbolLayerV2* create( const QgsStringMap& properties = QgsStringMap() ) /Factory/;

    // implemented from base classes

    QString layerType() const;

    void startRender( QgsSymbolV2RenderContext& context );

    void stopRender( QgsSymbolV2RenderContext& context );

    void renderPolygon( const QPolygonF& points, QList<QPolygonF>* rings, QgsSymbolV2RenderContext& context );

    QgsStringMap properties() const;

    virtual QgsShapeburstFillSymbolLayerV2* clone() const /Factory/;

    double estimateMaxBleed() const;

    /** Sets the blur radius, which controls the amount of blurring applied to the fill.
     * @param blurRadius Radius for fill blur. Values between 0 - 17 are valid, where higher values results in a stronger blur. Set to 0 to disable blur.
     * @note added in 2.3
     * @see blurRadius
     */
    void setBlurRadius( int blurRadius );
    /** Returns the blur radius, which controls the amount of blurring applied to the fill.
     * @returns Integer representing the radius for fill blur. Higher values indicate a stronger blur. A 0 value indicates that blurring is disabled.
     * @note added in 2.3
     * @see setBlurRadius
     */
    int blurRadius() const;

    /** Sets whether the shapeburst fill should be drawn using the entire shape.
     * @param useWholeShape Set to true if shapeburst should cover entire shape. If false, setMaxDistance is used to calculate how far from the boundary of the shape should
     * be shaded
     * @note added in 2.3
     * @see useWholeShape
     * @see setMaxDistance
     */
    void setUseWholeShape( bool useWholeShape );
    /** Returns whether the shapeburst fill is set to cover the entire shape.
     * @returns True if shapeburst fill will cover the entire shape. If false, shapeburst is drawn to a distance of maxDistance from the polygon's boundary.
     * @note added in 2.3
     * @see setUseWholeShape
     * @see maxDistance
     */
    bool useWholeShape() const;

    /** Sets the maximum distance to shape inside of the shape from the polygon's boundary.
     * @param maxDistance distance from boundary to shade. setUseWholeShape must be set to false for this parameter to take effect. Distance unit is controlled by setDistanceUnit.
     * @note added in 2.3
     * @see maxDistance
     * @see setUseWholeShape
     * @see setDistanceUnit
     */
    void setMaxDistance( double maxDistance );
    /** Returns the maximum distance from the shape's boundary which is shaded. This parameter is only effective if useWholeShape is false.
     * @returns the maximum distance from the polygon's boundary which is shaded. Distance units are indicated by distanceUnit.
     * @note added in 2.3
     * @see useWholeShape
     * @see setMaxDistance
     * @see distanceUnit
     */
    double maxDistance() const;

    /** Sets the unit for the maximum distance to shade inside of the shape from the polygon's boundary.
     * @param unit distance unit for the maximum distance
     * @note added in 2.3
     * @see setMaxDistance
     * @see distanceUnit
     */
    void setDistanceUnit( QgsSymbolV2::OutputUnit unit );
    /** Returns the unit for the maximum distance to shade inside of the shape from the polygon's boundary.
     * @returns distance unit for the maximum distance
     * @note added in 2.3
     * @see maxDistance
     * @see setDistanceUnit
     */
    QgsSymbolV2::OutputUnit distanceUnit() const;

    void setDistanceMapUnitScale( const QgsMapUnitScale& scale );
    const QgsMapUnitScale& distanceMapUnitScale() const;

    /** Sets the color mode to use for the shapeburst fill. Shapeburst can either be drawn using a QgsVectorColorRampV2 color ramp
     * or by simply specificing a start and end color. setColorType is used to specify which mode to use for the fill.
     * @param colorType color type to use for shapeburst fill
     * @note added in 2.3
     * @see colorType
     * @see setColor
     * @see setColor2
     * @see setColorRamp
     */
    void setColorType( ShapeburstColorType colorType );
    /** Returns the color mode used for the shapeburst fill. Shapeburst can either be drawn using a QgsVectorColorRampV2 color ramp
     * or by simply specificing a start and end color.
     * @returns current color mode used for the shapeburst fill
     * @note added in 2.3
     * @see setColorType
     * @see color
     * @see color2
     * @see colorRamp
     */
    ShapeburstColorType colorType() const;

    /** Sets the color ramp used to draw the shapeburst fill. Color ramps are only used if setColorType is set ShapeburstColorType::ColorRamp.
     * @param ramp color ramp to use for shapeburst fill
     * @note added in 2.3
     * @see setColorType
     * @see colorRamp
     */
    void setColorRamp( QgsVectorColorRampV2* ramp );
    /** Returns the color ramp used for the shapeburst fill. The color ramp is only used if the colorType is set to ShapeburstColorType::ColorRamp
     * @returns a QgsVectorColorRampV2 color ramp
     * @note added in 2.3
     * @see setColorRamp
     * @see colorType
     */
    QgsVectorColorRampV2* colorRamp();

    /** Sets the color for the endpoint of the shapeburst fill. This color is only used if setColorType is set ShapeburstColorType::SimpleTwoColor.
     * @param color2 QColor to use for endpoint of gradient
     * @note added in 2.3
     * @see setColorType
     * @see color2
     */
    void setColor2( const QColor& color2 );
    /** Returns the color used for the endpoint of the shapeburst fill. This color is only used if the colorType is set to ShapeburstColorType::SimpleTwoColor
     * @returns a QColor indicating the color of the endpoint of the gradient
     * @note added in 2.3
     * @see setColor2
     * @see colorType
     */
    QColor color2() const;

    /** Sets whether the shapeburst fill should ignore polygon rings when calculating
     * the buffered shading.
     * @param ignoreRings Set to true if buffers should ignore interior rings for polygons.
     * @note added in 2.3
     * @see ignoreRings
     */
    void setIgnoreRings( bool ignoreRings );
    /** Returns whether the shapeburst fill is set to ignore polygon interior rings.
     * @returns True if the shapeburst fill will ignore interior rings when calculating buffered shading.
     * @note added in 2.3
     * @see setIgnoreRings
     */
    bool ignoreRings() const;

    /** Sets the offset for the shapeburst fill.
     * @param offset QPointF indicating the horizontal/vertical offset amount
     * @note added in 2.3
     * @see offset
     * @see setOffsetUnit
     */
    void setOffset( QPointF offset );
    /** Returns the offset for the shapeburst fill.
     * @returns a QPointF indicating the horizontal/vertical offset amount
     * @note added in 2.3
     * @see setOffset
     * @see offsetUnit
     */
    QPointF offset() const;

    /** Sets the units used for the offset for the shapeburst fill.
     * @param unit units for fill offset
     * @note added in 2.3
     * @see setOffset
     * @see offsetUnit
     */
    void setOffsetUnit( QgsSymbolV2::OutputUnit unit );
    /** Returns the units used for the offset of the shapeburst fill.
     * @returns units used for the fill offset
     * @note added in 2.3
     * @see offset
     * @see setOffsetUnit
     */
    QgsSymbolV2::OutputUnit offsetUnit() const;

    void setOffsetMapUnitScale( const QgsMapUnitScale& scale);
    const QgsMapUnitScale& offsetMapUnitScale() const;

    void setOutputUnit( QgsSymbolV2::OutputUnit unit );
    QgsSymbolV2::OutputUnit outputUnit() const;

    void setMapUnitScale( const QgsMapUnitScale &scale );
    QgsMapUnitScale mapUnitScale() const;
};

/** Base class for polygon renderers generating texture images*/
class QgsImageFillSymbolLayer: QgsFillSymbolLayerV2
{
%TypeHeaderCode
#include <qgsfillsymbollayerv2.h>
%End

  public:
    QgsImageFillSymbolLayer();
    virtual ~QgsImageFillSymbolLayer();
    void renderPolygon( const QPolygonF& points, QList<QPolygonF>* rings, QgsSymbolV2RenderContext& context );

    virtual QgsSymbolV2* subSymbol();
    virtual bool setSubSymbol( QgsSymbolV2* symbol /Transfer/ );

    void setOutlineWidthUnit( QgsSymbolV2::OutputUnit unit );
    QgsSymbolV2::OutputUnit outlineWidthUnit() const;

    void setOutlineWidthMapUnitScale( const QgsMapUnitScale& scale);
    const QgsMapUnitScale& outlineWidthMapUnitScale() const;

    void setOutputUnit( QgsSymbolV2::OutputUnit unit );
    QgsSymbolV2::OutputUnit outputUnit() const;

    void setMapUnitScale( const QgsMapUnitScale& scale );
    QgsMapUnitScale mapUnitScale() const;

    virtual double estimateMaxBleed() const;

    virtual double dxfWidth( const QgsDxfExport& e, QgsSymbolV2RenderContext& context ) const;
    virtual QColor dxfColor( QgsSymbolV2RenderContext& context ) const;
    virtual Qt::PenStyle dxfPenStyle() const;

    QSet<QString> usedAttributes() const;

  protected:

    virtual void applyDataDefinedSettings( QgsSymbolV2RenderContext& context );
};

/** \ingroup core
 * \class QgsRasterFillSymbolLayer
 * \brief A class for filling symbols with a repeated raster image.
 * \note Added in version 2.7
 */
class QgsRasterFillSymbolLayer: QgsImageFillSymbolLayer
{
%TypeHeaderCode
#include <qgsfillsymbollayerv2.h>
%End
  public:

    enum FillCoordinateMode
    {
      Feature,
      Viewport
    };

    QgsRasterFillSymbolLayer( const QString& imageFilePath = QString() );
    ~QgsRasterFillSymbolLayer();

    static QgsSymbolLayerV2* create( const QgsStringMap& properties = QgsStringMap() );

    // implemented from base classes
    QString layerType() const;
    void renderPolygon( const QPolygonF& points, QList<QPolygonF>* rings, QgsSymbolV2RenderContext& context );
    void startRender( QgsSymbolV2RenderContext& context );
    void stopRender( QgsSymbolV2RenderContext& context );
    QgsStringMap properties() const;
    virtual QgsRasterFillSymbolLayer* clone() const;
    virtual double estimateMaxBleed() const;

    //override QgsImageFillSymbolLayer's support for sub symbols
    virtual QgsSymbolV2* subSymbol();
    virtual bool setSubSymbol( QgsSymbolV2* symbol /Transfer/ );

    /** Sets the path to the raster image used for the fill.
     * @param imagePath path to image file
     * @see imageFilePath
     */
    void setImageFilePath( const QString& imagePath );
    /** The path to the raster image used for the fill.
     * @returns path to image file
     * @see setImageFilePath
     */
    QString imageFilePath() const;

    /** Set the coordinate mode for fill. Controls how the top left corner of the image
     * fill is positioned relative to the feature.
     * @param mode coordinate mode
     * @see coordinateMode
     */
    void setCoordinateMode( const FillCoordinateMode mode );
    /** Coordinate mode for fill. Controls how the top left corner of the image
     * fill is positioned relative to the feature.
     * @returns coordinate mode
     * @see setCoordinateMode
     */
    FillCoordinateMode coordinateMode() const;

    /** Sets the opacity for the raster image used in the fill.
     * @param alpha opacity value between 0 (fully transparent) and 1 (fully opaque)
     * @see alpha
     */
    void setAlpha( const double alpha );
    /** The opacity for the raster image used in the fill.
     * @returns opacity value between 0 (fully transparent) and 1 (fully opaque)
     * @see setAlpha
     */
    double alpha() const;

    /** Sets the offset for the fill.
     * @param offset offset for fill
     * @see offset
     * @see setOffsetUnit
     * @see setOffsetMapUnitScale
     */
    void setOffset( QPointF offset );
    /** Returns the offset for the fill.
     * @returns offset for fill
     * @see setOffset
     * @see offsetUnit
     * @see offsetMapUnitScale
     */
    QPointF offset() const;

    /** Sets the units for the fill's offset.
     * @param unit units for offset
     * @see offsetUnit
     * @see setOffset
     * @see setOffsetMapUnitScale
     */
    void setOffsetUnit( const QgsSymbolV2::OutputUnit unit );
    /** Returns the units for the fill's offset.
     * @returns units for offset
     * @see setOffsetUnit
     * @see offset
     * @see offsetMapUnitScale
     */
    QgsSymbolV2::OutputUnit offsetUnit() const;

    /** Sets the map unit scale for the fill's offset.
     * @param scale map unit scale for offset
     * @see offsetMapUnitScale
     * @see setOffset
     * @see setOffsetUnit
     */
    void setOffsetMapUnitScale( const QgsMapUnitScale& scale );
    /** Returns the map unit scale for the fill's offset.
     * @returns map unit scale for offset
     * @see setOffsetMapUnitScale
     * @see offset
     * @see offsetUnit
     */
    const QgsMapUnitScale& offsetMapUnitScale() const;

    /** Sets the width for scaling the image used in the fill. The image's height will also be
     * scaled to maintain the image's aspect ratio.
     * @param width width for scaling the image
     * @see width
     * @see setWidthUnit
     * @see setWidthMapUnitScale
     */
    void setWidth( const double width );
    /** Returns the width used for scaling the image used in the fill. The image's height is
     * scaled to maintain the image's aspect ratio.
     * @returns width used for scaling the image
     * @see setWidth
     * @see widthUnit
     * @see widthMapUnitScale
     */
    double width() const;

    /** Sets the units for the image's width.
     * @param unit units for width
     * @see widthUnit
     * @see setWidth
     * @see setWidthMapUnitScale
     */
    void setWidthUnit( const QgsSymbolV2::OutputUnit unit );
    /** Returns the units for the image's width.
     * @returns units for width
     * @see setWidthUnit
     * @see width
     * @see widthMapUnitScale
     */
    QgsSymbolV2::OutputUnit widthUnit() const;

    /** Sets the map unit scale for the image's width.
     * @param scale map unit scale for width
     * @see widthMapUnitScale
     * @see setWidth
     * @see setWidthUnit
     */
    void setWidthMapUnitScale( const QgsMapUnitScale& scale );
    /** Returns the map unit scale for the image's width.
     * @returns map unit scale for width
     * @see setWidthMapUnitScale
     * @see width
     * @see widthUnit
     */
    const QgsMapUnitScale& widthMapUnitScale() const;

  protected:

    void applyDataDefinedSettings( QgsSymbolV2RenderContext& context );

};

/** A class for svg fill patterns. The class automatically scales the pattern to
   the appropriate pixel dimensions of the output device*/
class QgsSVGFillSymbolLayer: QgsImageFillSymbolLayer
{
%TypeHeaderCode
#include <qgsfillsymbollayerv2.h>
%End

  public:
    QgsSVGFillSymbolLayer( const QString& svgFilePath = "", double width = 20, double rotation = 0.0 );
    QgsSVGFillSymbolLayer( const QByteArray& svgData, double width = 20, double rotation = 0.0 );
    ~QgsSVGFillSymbolLayer();

    static QgsSymbolLayerV2* create( const QgsStringMap& properties = QgsStringMap() ) /Factory/;
    static QgsSymbolLayerV2* createFromSld( QDomElement &element ) /Factory/;

    // implemented from base classes

    QString layerType() const;

    void startRender( QgsSymbolV2RenderContext& context );
    void stopRender( QgsSymbolV2RenderContext& context );

    QgsStringMap properties() const;

    virtual QgsSVGFillSymbolLayer* clone() const /Factory/;

    void toSld( QDomDocument &doc, QDomElement &element, const QgsStringMap& props ) const;

    //getters and setters
    void setSvgFilePath( const QString& svgPath );
    QString svgFilePath() const;
    void setPatternWidth( double width );
    double patternWidth() const;

    void setSvgFillColor( const QColor& c );
    QColor svgFillColor() const;
    void setSvgOutlineColor( const QColor& c );
    QColor svgOutlineColor() const;
    void setSvgOutlineWidth( double w );
    double svgOutlineWidth() const;

    void setPatternWidthUnit( QgsSymbolV2::OutputUnit unit );
    QgsSymbolV2::OutputUnit patternWidthUnit() const;

    void setPatternWidthMapUnitScale( const QgsMapUnitScale& scale);
    const QgsMapUnitScale& patternWidthMapUnitScale() const;

    void setSvgOutlineWidthUnit( QgsSymbolV2::OutputUnit unit );
    QgsSymbolV2::OutputUnit svgOutlineWidthUnit() const;

    void setSvgOutlineWidthMapUnitScale( const QgsMapUnitScale& scale);
    const QgsMapUnitScale& svgOutlineWidthMapUnitScale() const;

    void setOutputUnit( QgsSymbolV2::OutputUnit unit );
    QgsSymbolV2::OutputUnit outputUnit() const;

    void setMapUnitScale( const QgsMapUnitScale &scale );
    QgsMapUnitScale mapUnitScale() const;

  protected:
    void applyDataDefinedSettings( QgsSymbolV2RenderContext& context );
};

class QgsLinePatternFillSymbolLayer: QgsImageFillSymbolLayer
{
%TypeHeaderCode
#include <qgsfillsymbollayerv2.h>
%End
  public:
    QgsLinePatternFillSymbolLayer();
    ~QgsLinePatternFillSymbolLayer();

    static QgsSymbolLayerV2* create( const QgsStringMap& properties = QgsStringMap() ) /Factory/;
    static QgsSymbolLayerV2* createFromSld( QDomElement &element ) /Factory/;

    QString layerType() const;

    void startRender( QgsSymbolV2RenderContext& context );

    void stopRender( QgsSymbolV2RenderContext& context );

    QgsStringMap properties() const;

    virtual QgsLinePatternFillSymbolLayer* clone() const /Factory/;

    void toSld( QDomDocument &doc, QDomElement &element, const QgsStringMap& props ) const;

    double estimateMaxBleed() const;

    QString ogrFeatureStyleWidth( double widthScaleFactor ) const;

    //getters and setters
    void setLineAngle( double a );
    double lineAngle() const;
    void setDistance( double d );
    double distance() const;
    void setLineWidth( double w );
    double lineWidth() const;
    void setColor( const QColor& c );
    QColor color() const;
    void setOffset( double offset );
    double offset() const;

    void setDistanceUnit( QgsSymbolV2::OutputUnit unit );
    QgsSymbolV2::OutputUnit distanceUnit() const;

    void setDistanceMapUnitScale( const QgsMapUnitScale& scale);
    const QgsMapUnitScale& distanceMapUnitScale() const;

    void setLineWidthUnit( QgsSymbolV2::OutputUnit unit );
    QgsSymbolV2::OutputUnit lineWidthUnit() const;

    void setLineWidthMapUnitScale( const QgsMapUnitScale& scale);
    const QgsMapUnitScale& lineWidthMapUnitScale() const;

    void setOffsetUnit( QgsSymbolV2::OutputUnit unit );
    QgsSymbolV2::OutputUnit offsetUnit() const;

    void setOffsetMapUnitScale( const QgsMapUnitScale& scale);
    const QgsMapUnitScale& offsetMapUnitScale() const;

    void setOutputUnit( QgsSymbolV2::OutputUnit unit );
    QgsSymbolV2::OutputUnit outputUnit() const;

    void setMapUnitScale( const QgsMapUnitScale& scale );
    QgsMapUnitScale mapUnitScale() const;

    bool setSubSymbol( QgsSymbolV2* symbol /Transfer/ );
    QgsSymbolV2* subSymbol();

    QSet<QString> usedAttributes() const;

};

class QgsPointPatternFillSymbolLayer : QgsImageFillSymbolLayer
{
%TypeHeaderCode
#include <qgsfillsymbollayerv2.h>
%End
  public:
    QgsPointPatternFillSymbolLayer();
    ~QgsPointPatternFillSymbolLayer();

    static QgsSymbolLayerV2* create( const QgsStringMap& properties = QgsStringMap() ) /Factory/;
    static QgsSymbolLayerV2* createFromSld( QDomElement &element ) /Factory/;

    QString layerType() const;

    void startRender( QgsSymbolV2RenderContext& context );

    void stopRender( QgsSymbolV2RenderContext& context );

    QgsStringMap properties() const;

    virtual QgsPointPatternFillSymbolLayer* clone() const /Factory/;

    void toSld( QDomDocument &doc, QDomElement &element, const QgsStringMap& props ) const;

    double estimateMaxBleed() const;

    //getters and setters
    double distanceX() const;
    void setDistanceX( double d );

    double distanceY() const;
    void setDistanceY( double d );

    double displacementX() const;
    void setDisplacementX( double d );

    double displacementY() const;
    void setDisplacementY( double d );

    bool setSubSymbol( QgsSymbolV2* symbol /Transfer/ );
    virtual QgsSymbolV2* subSymbol();

    void setDistanceXUnit( QgsSymbolV2::OutputUnit unit );
    QgsSymbolV2::OutputUnit distanceXUnit() const;

    void setDistanceXMapUnitScale( const QgsMapUnitScale& scale);
    const QgsMapUnitScale& distanceXMapUnitScale() const;

    void setDistanceYUnit( QgsSymbolV2::OutputUnit unit );
    QgsSymbolV2::OutputUnit distanceYUnit() const;

    void setDistanceYMapUnitScale( const QgsMapUnitScale& scale);
    const QgsMapUnitScale& distanceYMapUnitScale() const;

    void setDisplacementXUnit( QgsSymbolV2::OutputUnit unit );
    QgsSymbolV2::OutputUnit displacementXUnit() const;

    void setDisplacementXMapUnitScale( const QgsMapUnitScale& scale);
    const QgsMapUnitScale& displacementXMapUnitScale() const;

    void setDisplacementYUnit( QgsSymbolV2::OutputUnit unit );
    QgsSymbolV2::OutputUnit displacementYUnit() const;

    void setDisplacementYMapUnitScale( const QgsMapUnitScale& scale);
    const QgsMapUnitScale& displacementYMapUnitScale() const;

    void setOutputUnit( QgsSymbolV2::OutputUnit unit );
    QgsSymbolV2::OutputUnit outputUnit() const;

    void setMapUnitScale( const QgsMapUnitScale &scale );
    QgsMapUnitScale mapUnitScale() const;

    virtual QSet<QString> usedAttributes() const;

    void setColor( const QColor& c );
    virtual QColor color() const;

  protected:
    void applyDataDefinedSettings( QgsSymbolV2RenderContext& context );
};

class QgsCentroidFillSymbolLayerV2 : QgsFillSymbolLayerV2
{
%TypeHeaderCode
#include <qgsfillsymbollayerv2.h>
%End

  public:
    QgsCentroidFillSymbolLayerV2();
    ~QgsCentroidFillSymbolLayerV2();

    // static stuff

    static QgsSymbolLayerV2* create( const QgsStringMap& properties = QgsStringMap() ) /Factory/;
    static QgsSymbolLayerV2* createFromSld( QDomElement &element ) /Factory/;

    // implemented from base classes

    QString layerType() const;

    void startRender( QgsSymbolV2RenderContext& context );

    void stopRender( QgsSymbolV2RenderContext& context );

    void renderPolygon( const QPolygonF& points, QList<QPolygonF>* rings, QgsSymbolV2RenderContext& context );

    QgsStringMap properties() const;

    virtual QgsCentroidFillSymbolLayerV2* clone() const /Factory/;

    void toSld( QDomDocument &doc, QDomElement &element, const QgsStringMap& props ) const;

    void setColor( const QColor& color );
    QColor color() const;

    QgsSymbolV2* subSymbol();
    bool setSubSymbol( QgsSymbolV2* symbol /Transfer/ );

    void setOutputUnit( QgsSymbolV2::OutputUnit unit );
    QgsSymbolV2::OutputUnit outputUnit() const;

    void setMapUnitScale(const QgsMapUnitScale &scale);
    QgsMapUnitScale mapUnitScale() const;

    virtual QSet<QString> usedAttributes() const;

    void setPointOnSurface( bool pointOnSurface );
    bool pointOnSurface() const;

    /** Sets whether a point is drawn for all parts or only on the biggest part of multi-part features.
     * @note added in 2.16 */
    void setPointOnAllParts( bool pointOnAllParts );
    /** Returns whether a point is drawn for all parts or only on the biggest part of multi-part features.
     * @note added in 2.16 */
    bool pointOnAllParts() const;
};
