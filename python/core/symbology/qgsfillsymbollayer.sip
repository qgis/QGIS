/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/symbology/qgsfillsymbollayer.h                              *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/






class QgsSimpleFillSymbolLayer : QgsFillSymbolLayer
{

%TypeHeaderCode
#include "qgsfillsymbollayer.h"
%End
  public:
    QgsSimpleFillSymbolLayer( const QColor &color = DEFAULT_SIMPLEFILL_COLOR,
                              Qt::BrushStyle style = DEFAULT_SIMPLEFILL_STYLE,
                              const QColor &strokeColor = DEFAULT_SIMPLEFILL_BORDERCOLOR,
                              Qt::PenStyle strokeStyle = DEFAULT_SIMPLEFILL_BORDERSTYLE,
                              double strokeWidth = DEFAULT_SIMPLEFILL_BORDERWIDTH,
                              Qt::PenJoinStyle penJoinStyle = DEFAULT_SIMPLEFILL_JOINSTYLE
                            );


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


    virtual void renderPolygon( const QPolygonF &points, QList<QPolygonF> *rings, QgsSymbolRenderContext &context );


    virtual QgsStringMap properties() const;


    virtual QgsSimpleFillSymbolLayer *clone() const /Factory/;


    virtual void toSld( QDomDocument &doc, QDomElement &element, const QgsStringMap &props ) const;


    virtual QString ogrFeatureStyle( double mmScaleFactor, double mapUnitScaleFactor ) const;


    Qt::BrushStyle brushStyle() const;
%Docstring
 :rtype: Qt.BrushStyle
%End
    void setBrushStyle( Qt::BrushStyle style );

    virtual QColor strokeColor() const;
    virtual void setStrokeColor( const QColor &strokeColor );

    virtual QColor fillColor() const;
    virtual void setFillColor( const QColor &color );

    Qt::PenStyle strokeStyle() const;
%Docstring
 :rtype: Qt.PenStyle
%End
    void setStrokeStyle( Qt::PenStyle strokeStyle );

    double strokeWidth() const;
%Docstring
 :rtype: float
%End
    void setStrokeWidth( double strokeWidth );

    Qt::PenJoinStyle penJoinStyle() const;
%Docstring
 :rtype: Qt.PenJoinStyle
%End
    void setPenJoinStyle( Qt::PenJoinStyle style );

    void setOffset( QPointF offset );
    QPointF offset();
%Docstring
 :rtype: QPointF
%End

    void setStrokeWidthUnit( QgsUnitTypes::RenderUnit unit );
%Docstring
 Sets the units for the width of the fill's stroke.
 \param unit width units
.. seealso:: strokeWidthUnit()
%End

    QgsUnitTypes::RenderUnit strokeWidthUnit() const;
%Docstring
 Returns the units for the width of the fill's stroke.
.. seealso:: setStrokeWidthUnit()
 :rtype: QgsUnitTypes.RenderUnit
%End

    void setStrokeWidthMapUnitScale( const QgsMapUnitScale &scale );
    const QgsMapUnitScale &strokeWidthMapUnitScale() const;
%Docstring
 :rtype: QgsMapUnitScale
%End

    void setOffsetUnit( QgsUnitTypes::RenderUnit unit );
%Docstring
 Sets the units for the fill's offset.
 \param unit offset units
.. seealso:: offsetUnit()
%End

    QgsUnitTypes::RenderUnit offsetUnit() const;
%Docstring
 Returns the units for the fill's offset.
.. seealso:: setOffsetUnit()
 :rtype: QgsUnitTypes.RenderUnit
%End

    void setOffsetMapUnitScale( const QgsMapUnitScale &scale );
    const QgsMapUnitScale &offsetMapUnitScale() const;
%Docstring
 :rtype: QgsMapUnitScale
%End

    virtual void setOutputUnit( QgsUnitTypes::RenderUnit unit );

    virtual QgsUnitTypes::RenderUnit outputUnit() const;


    virtual void setMapUnitScale( const QgsMapUnitScale &scale );

    virtual QgsMapUnitScale mapUnitScale() const;


    virtual double estimateMaxBleed( const QgsRenderContext &context ) const;


    virtual double dxfWidth( const QgsDxfExport &e, QgsSymbolRenderContext &context ) const;

    virtual QColor dxfColor( QgsSymbolRenderContext &context ) const;

    virtual double dxfAngle( QgsSymbolRenderContext &context ) const;


    virtual Qt::PenStyle dxfPenStyle() const;

    virtual QColor dxfBrushColor( QgsSymbolRenderContext &context ) const;

    virtual Qt::BrushStyle dxfBrushStyle() const;


  protected:


};


class QgsGradientFillSymbolLayer : QgsFillSymbolLayer
{

%TypeHeaderCode
#include "qgsfillsymbollayer.h"
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

    QgsGradientFillSymbolLayer( const QColor &color = DEFAULT_SIMPLEFILL_COLOR,
                                const QColor &color2 = Qt::white,
                                GradientColorType gradientColorType = SimpleTwoColor,
                                GradientType gradientType = Linear,
                                GradientCoordinateMode coordinateMode = Feature,
                                GradientSpread gradientSpread = Pad
                              );

    virtual ~QgsGradientFillSymbolLayer();


    static QgsSymbolLayer *create( const QgsStringMap &properties = QgsStringMap() ) /Factory/;
%Docstring
 :rtype: QgsSymbolLayer
%End


    virtual QString layerType() const;


    virtual void startRender( QgsSymbolRenderContext &context );


    virtual void stopRender( QgsSymbolRenderContext &context );


    virtual void renderPolygon( const QPolygonF &points, QList<QPolygonF> *rings, QgsSymbolRenderContext &context );


    virtual QgsStringMap properties() const;


    virtual QgsGradientFillSymbolLayer *clone() const /Factory/;


    virtual double estimateMaxBleed( const QgsRenderContext &context ) const;


    GradientType gradientType() const;
%Docstring
Type of gradient, e.g., linear or radial
 :rtype: GradientType
%End
    void setGradientType( GradientType gradientType );

    GradientColorType gradientColorType() const;
%Docstring
Gradient color mode, controls how gradient color stops are created
 :rtype: GradientColorType
%End
    void setGradientColorType( GradientColorType gradientColorType );

    QgsColorRamp *colorRamp();
%Docstring
 Returns the color ramp used for the gradient fill. This is only
 used if the gradient color type is set to ColorRamp.
.. seealso:: setColorRamp()
.. seealso:: gradientColorType()
 :rtype: QgsColorRamp
%End

    void setColorRamp( QgsColorRamp *ramp );
%Docstring
 Sets the color ramp used for the gradient fill. This is only
 used if the gradient color type is set to ColorRamp.
 \param ramp color ramp. Ownership is transferred.
.. seealso:: colorRamp()
.. seealso:: setGradientColorType()
%End

    QColor color2() const;
%Docstring
Color for endpoint of gradient, only used if the gradient color type is set to SimpleTwoColor
 :rtype: QColor
%End
    void setColor2( const QColor &color2 );

    GradientCoordinateMode coordinateMode() const;
%Docstring
Coordinate mode for gradient. Controls how the gradient stops are positioned.
 :rtype: GradientCoordinateMode
%End
    void setCoordinateMode( GradientCoordinateMode coordinateMode );

    GradientSpread gradientSpread() const;
%Docstring
Gradient spread mode. Controls how the gradient behaves outside of the predefined stops
 :rtype: GradientSpread
%End
    void setGradientSpread( GradientSpread gradientSpread );

    void setReferencePoint1( QPointF referencePoint );
%Docstring
Starting point of gradient fill, in the range [0,0] - [1,1]
%End
    QPointF referencePoint1() const;
%Docstring
 :rtype: QPointF
%End

    void setReferencePoint1IsCentroid( bool isCentroid );
%Docstring
Sets the starting point of the gradient to be the feature centroid
%End
    bool referencePoint1IsCentroid() const;
%Docstring
 :rtype: bool
%End

    void setReferencePoint2( QPointF referencePoint );
%Docstring
End point of gradient fill, in the range [0,0] - [1,1]
%End
    QPointF referencePoint2() const;
%Docstring
 :rtype: QPointF
%End

    void setReferencePoint2IsCentroid( bool isCentroid );
%Docstring
Sets the end point of the gradient to be the feature centroid
%End
    bool referencePoint2IsCentroid() const;
%Docstring
 :rtype: bool
%End

    void setOffset( QPointF offset );
%Docstring
Offset for gradient fill
%End
    QPointF offset() const;
%Docstring
 :rtype: QPointF
%End

    void setOffsetUnit( QgsUnitTypes::RenderUnit unit );
%Docstring
Units for gradient fill offset
%End
    QgsUnitTypes::RenderUnit offsetUnit() const;
%Docstring
 :rtype: QgsUnitTypes.RenderUnit
%End

    void setOffsetMapUnitScale( const QgsMapUnitScale &scale );
    const QgsMapUnitScale &offsetMapUnitScale() const;
%Docstring
 :rtype: QgsMapUnitScale
%End

    virtual void setOutputUnit( QgsUnitTypes::RenderUnit unit );

    virtual QgsUnitTypes::RenderUnit outputUnit() const;


    virtual void setMapUnitScale( const QgsMapUnitScale &scale );

    virtual QgsMapUnitScale mapUnitScale() const;


  protected:




};

class QgsShapeburstFillSymbolLayer : QgsFillSymbolLayer
{

%TypeHeaderCode
#include "qgsfillsymbollayer.h"
%End
  public:

    enum ShapeburstColorType
    {
      SimpleTwoColor,
      ColorRamp
    };

    QgsShapeburstFillSymbolLayer( const QColor &color = DEFAULT_SIMPLEFILL_COLOR, const QColor &color2 = Qt::white,
                                  ShapeburstColorType colorType = SimpleTwoColor,
                                  int blurRadius = 0, bool useWholeShape = true, double maxDistance = 5 );

    virtual ~QgsShapeburstFillSymbolLayer();


    static QgsSymbolLayer *create( const QgsStringMap &properties = QgsStringMap() ) /Factory/;
%Docstring
 :rtype: QgsSymbolLayer
%End


    virtual QString layerType() const;


    virtual void startRender( QgsSymbolRenderContext &context );


    virtual void stopRender( QgsSymbolRenderContext &context );


    virtual void renderPolygon( const QPolygonF &points, QList<QPolygonF> *rings, QgsSymbolRenderContext &context );


    virtual QgsStringMap properties() const;


    virtual QgsShapeburstFillSymbolLayer *clone() const /Factory/;


    virtual double estimateMaxBleed( const QgsRenderContext &context ) const;


    void setBlurRadius( int blurRadius );
%Docstring
 Sets the blur radius, which controls the amount of blurring applied to the fill.
 \param blurRadius Radius for fill blur. Values between 0 - 17 are valid, where higher values results in a stronger blur. Set to 0 to disable blur.
.. versionadded:: 2.3
.. seealso:: blurRadius
%End

    int blurRadius() const;
%Docstring
 Returns the blur radius, which controls the amount of blurring applied to the fill.
 :return: Integer representing the radius for fill blur. Higher values indicate a stronger blur. A 0 value indicates that blurring is disabled.
.. versionadded:: 2.3
.. seealso:: setBlurRadius
 :rtype: int
%End

    void setUseWholeShape( bool useWholeShape );
%Docstring
 Sets whether the shapeburst fill should be drawn using the entire shape.
 \param useWholeShape Set to true if shapeburst should cover entire shape. If false, setMaxDistance is used to calculate how far from the boundary of the shape should
 be shaded
.. versionadded:: 2.3
.. seealso:: useWholeShape
.. seealso:: setMaxDistance
%End

    bool useWholeShape() const;
%Docstring
 Returns whether the shapeburst fill is set to cover the entire shape.
 :return: True if shapeburst fill will cover the entire shape. If false, shapeburst is drawn to a distance of maxDistance from the polygon's boundary.
.. versionadded:: 2.3
.. seealso:: setUseWholeShape
.. seealso:: maxDistance
 :rtype: bool
%End

    void setMaxDistance( double maxDistance );
%Docstring
 Sets the maximum distance to shape inside of the shape from the polygon's boundary.
 \param maxDistance distance from boundary to shade. setUseWholeShape must be set to false for this parameter to take effect. Distance unit is controlled by setDistanceUnit.
.. versionadded:: 2.3
.. seealso:: maxDistance
.. seealso:: setUseWholeShape
.. seealso:: setDistanceUnit
%End

    double maxDistance() const;
%Docstring
 Returns the maximum distance from the shape's boundary which is shaded. This parameter is only effective if useWholeShape is false.
 :return: the maximum distance from the polygon's boundary which is shaded. Distance units are indicated by distanceUnit.
.. versionadded:: 2.3
.. seealso:: useWholeShape
.. seealso:: setMaxDistance
.. seealso:: distanceUnit
 :rtype: float
%End

    void setDistanceUnit( QgsUnitTypes::RenderUnit unit );
%Docstring
 Sets the unit for the maximum distance to shade inside of the shape from the polygon's boundary.
 \param unit distance unit for the maximum distance
.. versionadded:: 2.3
.. seealso:: setMaxDistance
.. seealso:: distanceUnit
%End

    QgsUnitTypes::RenderUnit distanceUnit() const;
%Docstring
 Returns the unit for the maximum distance to shade inside of the shape from the polygon's boundary.
 :return: distance unit for the maximum distance
.. versionadded:: 2.3
.. seealso:: maxDistance
.. seealso:: setDistanceUnit
 :rtype: QgsUnitTypes.RenderUnit
%End

    void setDistanceMapUnitScale( const QgsMapUnitScale &scale );
    const QgsMapUnitScale &distanceMapUnitScale() const;
%Docstring
 :rtype: QgsMapUnitScale
%End

    void setColorType( ShapeburstColorType colorType );
%Docstring
 Sets the color mode to use for the shapeburst fill. Shapeburst can either be drawn using a QgsColorRamp color ramp
 or by simply specificing a start and end color. setColorType is used to specify which mode to use for the fill.
 \param colorType color type to use for shapeburst fill
.. versionadded:: 2.3
.. seealso:: colorType
.. seealso:: setColor
.. seealso:: setColor2
.. seealso:: setColorRamp
%End

    ShapeburstColorType colorType() const;
%Docstring
 Returns the color mode used for the shapeburst fill. Shapeburst can either be drawn using a QgsColorRamp color ramp
 or by simply specificing a start and end color.
 :return: current color mode used for the shapeburst fill
.. versionadded:: 2.3
.. seealso:: setColorType
.. seealso:: color
.. seealso:: color2
.. seealso:: colorRamp
 :rtype: ShapeburstColorType
%End

    void setColorRamp( QgsColorRamp *ramp );
%Docstring
 Sets the color ramp used to draw the shapeburst fill. Color ramps are only used if setColorType is set ShapeburstColorType.ColorRamp.
 \param ramp color ramp to use for shapeburst fill
.. versionadded:: 2.3
.. seealso:: setColorType
.. seealso:: colorRamp
%End

    QgsColorRamp *colorRamp();
%Docstring
 Returns the color ramp used for the shapeburst fill. The color ramp is only used if the colorType is set to ShapeburstColorType.ColorRamp
 :return: a QgsColorRamp color ramp
.. versionadded:: 2.3
.. seealso:: setColorRamp
.. seealso:: colorType
 :rtype: QgsColorRamp
%End

    void setColor2( const QColor &color2 );
%Docstring
 Sets the color for the endpoint of the shapeburst fill. This color is only used if setColorType is set ShapeburstColorType.SimpleTwoColor.
 \param color2 QColor to use for endpoint of gradient
.. versionadded:: 2.3
.. seealso:: setColorType
.. seealso:: color2
%End

    QColor color2() const;
%Docstring
 Returns the color used for the endpoint of the shapeburst fill. This color is only used if the colorType is set to ShapeburstColorType.SimpleTwoColor
 :return: a QColor indicating the color of the endpoint of the gradient
.. versionadded:: 2.3
.. seealso:: setColor2
.. seealso:: colorType
 :rtype: QColor
%End

    void setIgnoreRings( bool ignoreRings );
%Docstring
 Sets whether the shapeburst fill should ignore polygon rings when calculating
 the buffered shading.
 \param ignoreRings Set to true if buffers should ignore interior rings for polygons.
.. versionadded:: 2.3
.. seealso:: ignoreRings
%End

    bool ignoreRings() const;
%Docstring
 Returns whether the shapeburst fill is set to ignore polygon interior rings.
 :return: True if the shapeburst fill will ignore interior rings when calculating buffered shading.
.. versionadded:: 2.3
.. seealso:: setIgnoreRings
 :rtype: bool
%End

    void setOffset( QPointF offset );
%Docstring
 Sets the offset for the shapeburst fill.
 \param offset QPointF indicating the horizontal/vertical offset amount
.. versionadded:: 2.3
.. seealso:: offset
.. seealso:: setOffsetUnit
%End

    QPointF offset() const;
%Docstring
 Returns the offset for the shapeburst fill.
 :return: a QPointF indicating the horizontal/vertical offset amount
.. versionadded:: 2.3
.. seealso:: setOffset
.. seealso:: offsetUnit
 :rtype: QPointF
%End

    void setOffsetUnit( QgsUnitTypes::RenderUnit unit );
%Docstring
 Sets the units used for the offset for the shapeburst fill.
 \param unit units for fill offset
.. versionadded:: 2.3
.. seealso:: setOffset
.. seealso:: offsetUnit
%End

    QgsUnitTypes::RenderUnit offsetUnit() const;
%Docstring
 Returns the units used for the offset of the shapeburst fill.
 :return: units used for the fill offset
.. versionadded:: 2.3
.. seealso:: offset
.. seealso:: setOffsetUnit
 :rtype: QgsUnitTypes.RenderUnit
%End

    void setOffsetMapUnitScale( const QgsMapUnitScale &scale );
    const QgsMapUnitScale &offsetMapUnitScale() const;
%Docstring
 :rtype: QgsMapUnitScale
%End

    virtual void setOutputUnit( QgsUnitTypes::RenderUnit unit );

    virtual QgsUnitTypes::RenderUnit outputUnit() const;


    virtual void setMapUnitScale( const QgsMapUnitScale &scale );

    virtual QgsMapUnitScale mapUnitScale() const;


  protected:






};

class QgsImageFillSymbolLayer: QgsFillSymbolLayer
{
%Docstring
 Base class for polygon renderers generating texture images*
%End

%TypeHeaderCode
#include "qgsfillsymbollayer.h"
%End
  public:

    QgsImageFillSymbolLayer();
    virtual void renderPolygon( const QPolygonF &points, QList<QPolygonF> *rings, QgsSymbolRenderContext &context );


    virtual QgsSymbol *subSymbol();
    virtual bool setSubSymbol( QgsSymbol *symbol /Transfer/ );

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


    virtual double estimateMaxBleed( const QgsRenderContext &context ) const;

    virtual double dxfWidth( const QgsDxfExport &e, QgsSymbolRenderContext &context ) const;

    virtual QColor dxfColor( QgsSymbolRenderContext &context ) const;


    virtual Qt::PenStyle dxfPenStyle() const;


    virtual QSet<QString> usedAttributes( const QgsRenderContext &context ) const;


  protected:



    virtual void applyDataDefinedSettings( QgsSymbolRenderContext &context );
%Docstring
Custom stroke
%End

  private:
    QgsImageFillSymbolLayer( const QgsImageFillSymbolLayer &other );
};

class QgsRasterFillSymbolLayer: QgsImageFillSymbolLayer
{
%Docstring
 A class for filling symbols with a repeated raster image.
.. versionadded:: 2.7
%End

%TypeHeaderCode
#include "qgsfillsymbollayer.h"
%End
  public:

    enum FillCoordinateMode
    {
      Feature,
      Viewport
    };

    QgsRasterFillSymbolLayer( const QString &imageFilePath = QString() );

    static QgsSymbolLayer *create( const QgsStringMap &properties = QgsStringMap() ) /Factory/;
%Docstring
 :rtype: QgsSymbolLayer
%End

    virtual QString layerType() const;

    virtual void renderPolygon( const QPolygonF &points, QList<QPolygonF> *rings, QgsSymbolRenderContext &context );

    virtual void startRender( QgsSymbolRenderContext &context );

    virtual void stopRender( QgsSymbolRenderContext &context );

    virtual QgsStringMap properties() const;

    virtual QgsRasterFillSymbolLayer *clone() const /Factory/;

    virtual double estimateMaxBleed( const QgsRenderContext &context ) const;

    virtual QgsSymbol *subSymbol();
    virtual bool setSubSymbol( QgsSymbol *symbol /Transfer/ );

    void setImageFilePath( const QString &imagePath );
%Docstring
 Sets the path to the raster image used for the fill.
 \param imagePath path to image file
.. seealso:: imageFilePath
%End

    QString imageFilePath() const;
%Docstring
 The path to the raster image used for the fill.
 :return: path to image file
.. seealso:: setImageFilePath
 :rtype: str
%End

    void setCoordinateMode( const FillCoordinateMode mode );
%Docstring
 Set the coordinate mode for fill. Controls how the top left corner of the image
 fill is positioned relative to the feature.
 \param mode coordinate mode
.. seealso:: coordinateMode
%End

    FillCoordinateMode coordinateMode() const;
%Docstring
 Coordinate mode for fill. Controls how the top left corner of the image
 fill is positioned relative to the feature.
 :return: coordinate mode
.. seealso:: setCoordinateMode
 :rtype: FillCoordinateMode
%End

    void setOpacity( const double opacity );
%Docstring
 Sets the ``opacity`` for the raster image used in the fill.
 \param opacity opacity value between 0 (fully transparent) and 1 (fully opaque)
.. seealso:: opacity()
%End

    double opacity() const;
%Docstring
 Returns the opacity for the raster image used in the fill.
 :return: opacity value between 0 (fully transparent) and 1 (fully opaque)
.. seealso:: setOpacity()
 :rtype: float
%End

    void setOffset( QPointF offset );
%Docstring
 Sets the offset for the fill.
 \param offset offset for fill
.. seealso:: offset
.. seealso:: setOffsetUnit
.. seealso:: setOffsetMapUnitScale
%End

    QPointF offset() const;
%Docstring
 Returns the offset for the fill.
 :return: offset for fill
.. seealso:: setOffset
.. seealso:: offsetUnit
.. seealso:: offsetMapUnitScale
 :rtype: QPointF
%End

    void setOffsetUnit( const QgsUnitTypes::RenderUnit unit );
%Docstring
 Sets the units for the fill's offset.
 \param unit units for offset
.. seealso:: offsetUnit
.. seealso:: setOffset
.. seealso:: setOffsetMapUnitScale
%End

    QgsUnitTypes::RenderUnit offsetUnit() const;
%Docstring
 Returns the units for the fill's offset.
 :return: units for offset
.. seealso:: setOffsetUnit
.. seealso:: offset
.. seealso:: offsetMapUnitScale
 :rtype: QgsUnitTypes.RenderUnit
%End

    void setOffsetMapUnitScale( const QgsMapUnitScale &scale );
%Docstring
 Sets the map unit scale for the fill's offset.
 \param scale map unit scale for offset
.. seealso:: offsetMapUnitScale
.. seealso:: setOffset
.. seealso:: setOffsetUnit
%End

    const QgsMapUnitScale &offsetMapUnitScale() const;
%Docstring
 Returns the map unit scale for the fill's offset.
 :return: map unit scale for offset
.. seealso:: setOffsetMapUnitScale
.. seealso:: offset
.. seealso:: offsetUnit
 :rtype: QgsMapUnitScale
%End

    void setWidth( const double width );
%Docstring
 Sets the width for scaling the image used in the fill. The image's height will also be
 scaled to maintain the image's aspect ratio.
 \param width width for scaling the image
.. seealso:: width
.. seealso:: setWidthUnit
.. seealso:: setWidthMapUnitScale
%End

    double width() const;
%Docstring
 Returns the width used for scaling the image used in the fill. The image's height is
 scaled to maintain the image's aspect ratio.
 :return: width used for scaling the image
.. seealso:: setWidth
.. seealso:: widthUnit
.. seealso:: widthMapUnitScale
 :rtype: float
%End

    void setWidthUnit( const QgsUnitTypes::RenderUnit unit );
%Docstring
 Sets the units for the image's width.
 \param unit units for width
.. seealso:: widthUnit
.. seealso:: setWidth
.. seealso:: setWidthMapUnitScale
%End

    QgsUnitTypes::RenderUnit widthUnit() const;
%Docstring
 Returns the units for the image's width.
 :return: units for width
.. seealso:: setWidthUnit
.. seealso:: width
.. seealso:: widthMapUnitScale
 :rtype: QgsUnitTypes.RenderUnit
%End

    void setWidthMapUnitScale( const QgsMapUnitScale &scale );
%Docstring
 Sets the map unit scale for the image's width.
 \param scale map unit scale for width
.. seealso:: widthMapUnitScale
.. seealso:: setWidth
.. seealso:: setWidthUnit
%End

    const QgsMapUnitScale &widthMapUnitScale() const;
%Docstring
 Returns the map unit scale for the image's width.
 :return: map unit scale for width
.. seealso:: setWidthMapUnitScale
.. seealso:: width
.. seealso:: widthUnit
 :rtype: QgsMapUnitScale
%End

  protected:




    virtual void applyDataDefinedSettings( QgsSymbolRenderContext &context );

%Docstring
Path to the image file
%End

};

class QgsSVGFillSymbolLayer: QgsImageFillSymbolLayer
{
%Docstring
 A class for svg fill patterns. The class automatically scales the pattern to
the appropriate pixel dimensions of the output device*
%End

%TypeHeaderCode
#include "qgsfillsymbollayer.h"
%End
  public:
    QgsSVGFillSymbolLayer( const QString &svgFilePath, double width = 20, double rotation = 0.0 );
%Docstring
Constructs SVG fill symbol layer with picture from given absolute path to a SVG file
%End
    QgsSVGFillSymbolLayer( const QByteArray &svgData, double width = 20, double rotation = 0.0 );
    ~QgsSVGFillSymbolLayer();

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


    virtual QgsStringMap properties() const;


    virtual QgsSVGFillSymbolLayer *clone() const /Factory/;


    virtual void toSld( QDomDocument &doc, QDomElement &element, const QgsStringMap &props ) const;


    void setSvgFilePath( const QString &svgPath );
    QString svgFilePath() const;
%Docstring
 :rtype: str
%End
    void setPatternWidth( double width );
    double patternWidth() const;
%Docstring
 :rtype: float
%End

    void setSvgFillColor( const QColor &c );
    QColor svgFillColor() const;
%Docstring
 :rtype: QColor
%End

    void setSvgStrokeColor( const QColor &c );
    QColor svgStrokeColor() const;
%Docstring
 :rtype: QColor
%End
    void setSvgStrokeWidth( double w );
    double svgStrokeWidth() const;
%Docstring
 :rtype: float
%End

    void setPatternWidthUnit( QgsUnitTypes::RenderUnit unit );
%Docstring
 Sets the units for the width of the SVG images in the pattern.
 \param unit width units
.. seealso:: patternWidthUnit()
%End

    QgsUnitTypes::RenderUnit patternWidthUnit() const;
%Docstring
 Returns the units for the width of the SVG images in the pattern.
.. seealso:: setPatternWidthUnit()
 :rtype: QgsUnitTypes.RenderUnit
%End

    void setPatternWidthMapUnitScale( const QgsMapUnitScale &scale );
    const QgsMapUnitScale &patternWidthMapUnitScale() const;
%Docstring
 :rtype: QgsMapUnitScale
%End

    void setSvgStrokeWidthUnit( QgsUnitTypes::RenderUnit unit );
%Docstring
 Sets the units for the stroke width.
 \param unit width units
.. seealso:: svgStrokeWidthUnit()
%End

    QgsUnitTypes::RenderUnit svgStrokeWidthUnit() const;
%Docstring
 Returns the units for the stroke width.
.. seealso:: setSvgStrokeWidthUnit()
 :rtype: QgsUnitTypes.RenderUnit
%End

    void setSvgStrokeWidthMapUnitScale( const QgsMapUnitScale &scale );
    const QgsMapUnitScale &svgStrokeWidthMapUnitScale() const;
%Docstring
 :rtype: QgsMapUnitScale
%End

    virtual void setOutputUnit( QgsUnitTypes::RenderUnit unit );

    virtual QgsUnitTypes::RenderUnit outputUnit() const;


    virtual void setMapUnitScale( const QgsMapUnitScale &scale );

    virtual QgsMapUnitScale mapUnitScale() const;


  protected:



    virtual void applyDataDefinedSettings( QgsSymbolRenderContext &context );


};

class QgsLinePatternFillSymbolLayer: QgsImageFillSymbolLayer
{

%TypeHeaderCode
#include "qgsfillsymbollayer.h"
%End
  public:
    QgsLinePatternFillSymbolLayer();
    ~QgsLinePatternFillSymbolLayer();

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


    virtual QgsStringMap properties() const;


    virtual QgsLinePatternFillSymbolLayer *clone() const /Factory/;


    virtual void toSld( QDomDocument &doc, QDomElement &element, const QgsStringMap &props ) const;


    virtual double estimateMaxBleed( const QgsRenderContext &context ) const;


    QString ogrFeatureStyleWidth( double widthScaleFactor ) const;
%Docstring
 :rtype: str
%End

    void setLineAngle( double a );
    double lineAngle() const;
%Docstring
 :rtype: float
%End

    void setDistance( double d );
%Docstring
 Sets the distance between lines in the fill pattern.
 \param d distance. Units are specified by setDistanceUnit()
.. seealso:: distance()
.. seealso:: setDistanceUnit()
%End

    double distance() const;
%Docstring
 Returns the distance between lines in the fill pattern. Units are retrieved by distanceUnit().
.. seealso:: setDistance()
.. seealso:: distanceUnit()
 :rtype: float
%End

    void setLineWidth( double w );
    double lineWidth() const;
%Docstring
 :rtype: float
%End
    virtual void setColor( const QColor &c );

    virtual QColor color() const;

    void setOffset( double offset );
    double offset() const;
%Docstring
 :rtype: float
%End

    void setDistanceUnit( QgsUnitTypes::RenderUnit unit );
%Docstring
 Sets the units for the distance between lines in the fill pattern.
 \param unit distance units
.. seealso:: distanceUnit()
.. seealso:: setDistance()
%End

    QgsUnitTypes::RenderUnit distanceUnit() const;
%Docstring
 Returns the units for the distance between lines in the fill pattern.
.. seealso:: setDistanceUnit()
.. seealso:: distance()
 :rtype: QgsUnitTypes.RenderUnit
%End

    void setDistanceMapUnitScale( const QgsMapUnitScale &scale );
    const QgsMapUnitScale &distanceMapUnitScale() const;
%Docstring
 :rtype: QgsMapUnitScale
%End

    void setLineWidthUnit( QgsUnitTypes::RenderUnit unit );
%Docstring
 Sets the units for the line's width.
 \param unit width units
.. seealso:: lineWidthUnit()
%End

    QgsUnitTypes::RenderUnit lineWidthUnit() const;
%Docstring
 Returns the units for the line's width.
.. seealso:: setLineWidthUnit()
 :rtype: QgsUnitTypes.RenderUnit
%End

    void setLineWidthMapUnitScale( const QgsMapUnitScale &scale );
    const QgsMapUnitScale &lineWidthMapUnitScale() const;
%Docstring
 :rtype: QgsMapUnitScale
%End

    void setOffsetUnit( QgsUnitTypes::RenderUnit unit );
%Docstring
 Sets the units for the line pattern's offset.
 \param unit offset units
.. seealso:: offsetUnit()
%End

    QgsUnitTypes::RenderUnit offsetUnit() const;
%Docstring
 Returns the units for the line pattern's offset.
.. seealso:: setOffsetUnit()
 :rtype: QgsUnitTypes.RenderUnit
%End

    void setOffsetMapUnitScale( const QgsMapUnitScale &scale );
    const QgsMapUnitScale &offsetMapUnitScale() const;
%Docstring
 :rtype: QgsMapUnitScale
%End

    virtual void setOutputUnit( QgsUnitTypes::RenderUnit unit );

    virtual QgsUnitTypes::RenderUnit outputUnit() const;


    virtual void setMapUnitScale( const QgsMapUnitScale &scale );

    virtual QgsMapUnitScale mapUnitScale() const;


    virtual bool setSubSymbol( QgsSymbol *symbol /Transfer/ );

    virtual QgsSymbol *subSymbol();


    virtual QSet<QString> usedAttributes( const QgsRenderContext &context ) const;


  protected:

    virtual void applyDataDefinedSettings( QgsSymbolRenderContext &context );

%Docstring
Offset perpendicular to line direction
%End

  private:
    QgsLinePatternFillSymbolLayer( const QgsLinePatternFillSymbolLayer &other );
};

class QgsPointPatternFillSymbolLayer: QgsImageFillSymbolLayer
{

%TypeHeaderCode
#include "qgsfillsymbollayer.h"
%End
  public:
    QgsPointPatternFillSymbolLayer();
    ~QgsPointPatternFillSymbolLayer();

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


    virtual QgsStringMap properties() const;


    virtual QgsPointPatternFillSymbolLayer *clone() const /Factory/;


    virtual void toSld( QDomDocument &doc, QDomElement &element, const QgsStringMap &props ) const;


    virtual double estimateMaxBleed( const QgsRenderContext &context ) const;


    double distanceX() const;
%Docstring
 :rtype: float
%End
    void setDistanceX( double d );

    double distanceY() const;
%Docstring
 :rtype: float
%End
    void setDistanceY( double d );

    double displacementX() const;
%Docstring
 :rtype: float
%End
    void setDisplacementX( double d );

    double displacementY() const;
%Docstring
 :rtype: float
%End
    void setDisplacementY( double d );

    virtual bool setSubSymbol( QgsSymbol *symbol /Transfer/ );

    virtual QgsSymbol *subSymbol();

    void setDistanceXUnit( QgsUnitTypes::RenderUnit unit );
%Docstring
 Sets the units for the horizontal distance between points in the pattern.
 \param unit distance units
.. seealso:: distanceXUnit()
.. seealso:: setDistanceYUnit()
%End

    QgsUnitTypes::RenderUnit distanceXUnit() const;
%Docstring
 Returns the units for the horizontal distance between points in the pattern.
.. seealso:: setDistanceXUnit()
.. seealso:: distanceYUnit()
 :rtype: QgsUnitTypes.RenderUnit
%End

    void setDistanceXMapUnitScale( const QgsMapUnitScale &scale );
    const QgsMapUnitScale &distanceXMapUnitScale() const;
%Docstring
 :rtype: QgsMapUnitScale
%End

    void setDistanceYUnit( QgsUnitTypes::RenderUnit unit );
%Docstring
 Sets the units for the vertical distance between points in the pattern.
 \param unit distance units
.. seealso:: distanceYUnit()
.. seealso:: setDistanceXUnit()
%End

    QgsUnitTypes::RenderUnit distanceYUnit() const;
%Docstring
 Returns the units for the vertical distance between points in the pattern.
.. seealso:: setDistanceYUnit()
.. seealso:: distanceXUnit()
 :rtype: QgsUnitTypes.RenderUnit
%End

    void setDistanceYMapUnitScale( const QgsMapUnitScale &scale );
    const QgsMapUnitScale &distanceYMapUnitScale() const;
%Docstring
 :rtype: QgsMapUnitScale
%End

    void setDisplacementXUnit( QgsUnitTypes::RenderUnit unit );
%Docstring
 Sets the units for the horizontal displacement between rows in the pattern.
 \param unit displacement units
.. seealso:: displacementXUnit()
.. seealso:: setDisplacementYUnit()
%End

    QgsUnitTypes::RenderUnit displacementXUnit() const;
%Docstring
 Returns the units for the horizontal displacement between rows in the pattern.
.. seealso:: setDisplacementXUnit()
.. seealso:: displacementYUnit()
 :rtype: QgsUnitTypes.RenderUnit
%End

    void setDisplacementXMapUnitScale( const QgsMapUnitScale &scale );
    const QgsMapUnitScale &displacementXMapUnitScale() const;
%Docstring
 :rtype: QgsMapUnitScale
%End

    void setDisplacementYUnit( QgsUnitTypes::RenderUnit unit );
%Docstring
 Sets the units for the vertical displacement between rows in the pattern.
 \param unit displacement units
.. seealso:: displacementYUnit()
.. seealso:: setDisplacementXUnit()
%End

    QgsUnitTypes::RenderUnit displacementYUnit() const;
%Docstring
 Returns the units for the vertical displacement between rows in the pattern.
.. seealso:: setDisplacementYUnit()
.. seealso:: displacementXUnit()
 :rtype: QgsUnitTypes.RenderUnit
%End

    void setDisplacementYMapUnitScale( const QgsMapUnitScale &scale );
    const QgsMapUnitScale &displacementYMapUnitScale() const;
%Docstring
 :rtype: QgsMapUnitScale
%End

    virtual void setOutputUnit( QgsUnitTypes::RenderUnit unit );

    virtual QgsUnitTypes::RenderUnit outputUnit() const;


    virtual void setMapUnitScale( const QgsMapUnitScale &scale );

    virtual QgsMapUnitScale mapUnitScale() const;


    virtual QSet<QString> usedAttributes( const QgsRenderContext &context ) const;
    virtual void setColor( const QColor &c );

    virtual QColor color() const;

  protected:

    virtual void applyDataDefinedSettings( QgsSymbolRenderContext &context );


  private:
    QgsPointPatternFillSymbolLayer( const QgsPointPatternFillSymbolLayer &other );
};

class QgsCentroidFillSymbolLayer : QgsFillSymbolLayer
{

%TypeHeaderCode
#include "qgsfillsymbollayer.h"
%End
  public:
    QgsCentroidFillSymbolLayer();


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


    virtual void renderPolygon( const QPolygonF &points, QList<QPolygonF> *rings, QgsSymbolRenderContext &context );


    virtual QgsStringMap properties() const;


    virtual QgsCentroidFillSymbolLayer *clone() const /Factory/;


    virtual void toSld( QDomDocument &doc, QDomElement &element, const QgsStringMap &props ) const;


    virtual void setColor( const QColor &color );

    virtual QColor color() const;


    virtual QgsSymbol *subSymbol();

    virtual bool setSubSymbol( QgsSymbol *symbol /Transfer/ );


    virtual void setOutputUnit( QgsUnitTypes::RenderUnit unit );

    virtual QgsUnitTypes::RenderUnit outputUnit() const;


    virtual void setMapUnitScale( const QgsMapUnitScale &scale );

    virtual QgsMapUnitScale mapUnitScale() const;


    virtual QSet<QString> usedAttributes( const QgsRenderContext &context ) const;

    void setPointOnSurface( bool pointOnSurface );
    bool pointOnSurface() const;
%Docstring
 :rtype: bool
%End

    void setPointOnAllParts( bool pointOnAllParts );
%Docstring
 Sets whether a point is drawn for all parts or only on the biggest part of multi-part features.
.. versionadded:: 2.16
%End

    bool pointOnAllParts() const;
%Docstring
 Returns whether a point is drawn for all parts or only on the biggest part of multi-part features.
.. versionadded:: 2.16
 :rtype: bool
%End

  protected:


  private:
    QgsCentroidFillSymbolLayer( const QgsCentroidFillSymbolLayer &other );
};



/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/symbology/qgsfillsymbollayer.h                              *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
