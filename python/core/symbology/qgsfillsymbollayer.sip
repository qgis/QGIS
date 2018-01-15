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
    static QgsSymbolLayer *createFromSld( QDomElement &element ) /Factory/;


    virtual QString layerType() const;


    virtual void startRender( QgsSymbolRenderContext &context );


    virtual void stopRender( QgsSymbolRenderContext &context );


    virtual void renderPolygon( const QPolygonF &points, QList<QPolygonF> *rings, QgsSymbolRenderContext &context );


    virtual QgsStringMap properties() const;


    virtual QgsSimpleFillSymbolLayer *clone() const /Factory/;


    virtual void toSld( QDomDocument &doc, QDomElement &element, const QgsStringMap &props ) const;


    virtual QString ogrFeatureStyle( double mmScaleFactor, double mapUnitScaleFactor ) const;


    Qt::BrushStyle brushStyle() const;
    void setBrushStyle( Qt::BrushStyle style );

    virtual QColor strokeColor() const;
    virtual void setStrokeColor( const QColor &strokeColor );

    virtual QColor fillColor() const;
    virtual void setFillColor( const QColor &color );

    Qt::PenStyle strokeStyle() const;
    void setStrokeStyle( Qt::PenStyle strokeStyle );

    double strokeWidth() const;
    void setStrokeWidth( double strokeWidth );

    Qt::PenJoinStyle penJoinStyle() const;
    void setPenJoinStyle( Qt::PenJoinStyle style );

    void setOffset( QPointF offset );
    QPointF offset();

    void setStrokeWidthUnit( QgsUnitTypes::RenderUnit unit );
%Docstring
Sets the units for the width of the fill's stroke.

:param unit: width units

.. seealso:: :py:func:`strokeWidthUnit`
%End

    QgsUnitTypes::RenderUnit strokeWidthUnit() const;
%Docstring
Returns the units for the width of the fill's stroke.

.. seealso:: :py:func:`setStrokeWidthUnit`
%End

    void setStrokeWidthMapUnitScale( const QgsMapUnitScale &scale );
    const QgsMapUnitScale &strokeWidthMapUnitScale() const;

    void setOffsetUnit( QgsUnitTypes::RenderUnit unit );
%Docstring
Sets the units for the fill's offset.

:param unit: offset units

.. seealso:: :py:func:`offsetUnit`
%End

    QgsUnitTypes::RenderUnit offsetUnit() const;
%Docstring
Returns the units for the fill's offset.

.. seealso:: :py:func:`setOffsetUnit`
%End

    void setOffsetMapUnitScale( const QgsMapUnitScale &scale );
    const QgsMapUnitScale &offsetMapUnitScale() const;

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

    ~QgsGradientFillSymbolLayer();


    static QgsSymbolLayer *create( const QgsStringMap &properties = QgsStringMap() ) /Factory/;


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
%End
    void setGradientType( GradientType gradientType );

    GradientColorType gradientColorType() const;
%Docstring
Gradient color mode, controls how gradient color stops are created
%End
    void setGradientColorType( GradientColorType gradientColorType );

    QgsColorRamp *colorRamp();
%Docstring
Returns the color ramp used for the gradient fill. This is only
used if the gradient color type is set to ColorRamp.

.. seealso:: :py:func:`setColorRamp`

.. seealso:: :py:func:`gradientColorType`
%End

    void setColorRamp( QgsColorRamp *ramp );
%Docstring
Sets the color ramp used for the gradient fill. This is only
used if the gradient color type is set to ColorRamp.

:param ramp: color ramp. Ownership is transferred.

.. seealso:: :py:func:`colorRamp`

.. seealso:: :py:func:`setGradientColorType`
%End

    QColor color2() const;
%Docstring
Color for endpoint of gradient, only used if the gradient color type is set to SimpleTwoColor
%End
    void setColor2( const QColor &color2 );

    GradientCoordinateMode coordinateMode() const;
%Docstring
Coordinate mode for gradient. Controls how the gradient stops are positioned.
%End
    void setCoordinateMode( GradientCoordinateMode coordinateMode );

    GradientSpread gradientSpread() const;
%Docstring
Gradient spread mode. Controls how the gradient behaves outside of the predefined stops
%End
    void setGradientSpread( GradientSpread gradientSpread );

    void setReferencePoint1( QPointF referencePoint );
%Docstring
Starting point of gradient fill, in the range [0,0] - [1,1]
%End
    QPointF referencePoint1() const;

    void setReferencePoint1IsCentroid( bool isCentroid );
%Docstring
Sets the starting point of the gradient to be the feature centroid
%End
    bool referencePoint1IsCentroid() const;

    void setReferencePoint2( QPointF referencePoint );
%Docstring
End point of gradient fill, in the range [0,0] - [1,1]
%End
    QPointF referencePoint2() const;

    void setReferencePoint2IsCentroid( bool isCentroid );
%Docstring
Sets the end point of the gradient to be the feature centroid
%End
    bool referencePoint2IsCentroid() const;

    void setOffset( QPointF offset );
%Docstring
Offset for gradient fill
%End
    QPointF offset() const;

    void setOffsetUnit( QgsUnitTypes::RenderUnit unit );
%Docstring
Units for gradient fill offset
%End
    QgsUnitTypes::RenderUnit offsetUnit() const;

    void setOffsetMapUnitScale( const QgsMapUnitScale &scale );
    const QgsMapUnitScale &offsetMapUnitScale() const;

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

    ~QgsShapeburstFillSymbolLayer();


    static QgsSymbolLayer *create( const QgsStringMap &properties = QgsStringMap() ) /Factory/;


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

:param blurRadius: Radius for fill blur. Values between 0 - 17 are valid, where higher values results in a stronger blur. Set to 0 to disable blur.

.. versionadded:: 2.3

.. seealso:: :py:func:`blurRadius`
%End

    int blurRadius() const;
%Docstring
Returns the blur radius, which controls the amount of blurring applied to the fill.

:return: Integer representing the radius for fill blur. Higher values indicate a stronger blur. A 0 value indicates that blurring is disabled.

.. versionadded:: 2.3

.. seealso:: :py:func:`setBlurRadius`
%End

    void setUseWholeShape( bool useWholeShape );
%Docstring
Sets whether the shapeburst fill should be drawn using the entire shape.

:param useWholeShape: Set to true if shapeburst should cover entire shape. If false, setMaxDistance is used to calculate how far from the boundary of the shape should
be shaded

.. versionadded:: 2.3

.. seealso:: :py:func:`useWholeShape`

.. seealso:: :py:func:`setMaxDistance`
%End

    bool useWholeShape() const;
%Docstring
Returns whether the shapeburst fill is set to cover the entire shape.

:return: True if shapeburst fill will cover the entire shape. If false, shapeburst is drawn to a distance of maxDistance from the polygon's boundary.

.. versionadded:: 2.3

.. seealso:: :py:func:`setUseWholeShape`

.. seealso:: :py:func:`maxDistance`
%End

    void setMaxDistance( double maxDistance );
%Docstring
Sets the maximum distance to shape inside of the shape from the polygon's boundary.

:param maxDistance: distance from boundary to shade. setUseWholeShape must be set to false for this parameter to take effect. Distance unit is controlled by setDistanceUnit.

.. versionadded:: 2.3

.. seealso:: :py:func:`maxDistance`

.. seealso:: :py:func:`setUseWholeShape`

.. seealso:: :py:func:`setDistanceUnit`
%End

    double maxDistance() const;
%Docstring
Returns the maximum distance from the shape's boundary which is shaded. This parameter is only effective if useWholeShape is false.

:return: the maximum distance from the polygon's boundary which is shaded. Distance units are indicated by distanceUnit.

.. versionadded:: 2.3

.. seealso:: :py:func:`useWholeShape`

.. seealso:: :py:func:`setMaxDistance`

.. seealso:: :py:func:`distanceUnit`
%End

    void setDistanceUnit( QgsUnitTypes::RenderUnit unit );
%Docstring
Sets the unit for the maximum distance to shade inside of the shape from the polygon's boundary.

:param unit: distance unit for the maximum distance

.. versionadded:: 2.3

.. seealso:: :py:func:`setMaxDistance`

.. seealso:: :py:func:`distanceUnit`
%End

    QgsUnitTypes::RenderUnit distanceUnit() const;
%Docstring
Returns the unit for the maximum distance to shade inside of the shape from the polygon's boundary.

:return: distance unit for the maximum distance

.. versionadded:: 2.3

.. seealso:: :py:func:`maxDistance`

.. seealso:: :py:func:`setDistanceUnit`
%End

    void setDistanceMapUnitScale( const QgsMapUnitScale &scale );
    const QgsMapUnitScale &distanceMapUnitScale() const;

    void setColorType( ShapeburstColorType colorType );
%Docstring
Sets the color mode to use for the shapeburst fill. Shapeburst can either be drawn using a QgsColorRamp color ramp
or by simply specificing a start and end color. setColorType is used to specify which mode to use for the fill.

:param colorType: color type to use for shapeburst fill

.. versionadded:: 2.3

.. seealso:: :py:func:`colorType`

.. seealso:: :py:func:`setColor`

.. seealso:: :py:func:`setColor2`

.. seealso:: :py:func:`setColorRamp`
%End

    ShapeburstColorType colorType() const;
%Docstring
Returns the color mode used for the shapeburst fill. Shapeburst can either be drawn using a QgsColorRamp color ramp
or by simply specificing a start and end color.

:return: current color mode used for the shapeburst fill

.. versionadded:: 2.3

.. seealso:: :py:func:`setColorType`

.. seealso:: :py:func:`color`

.. seealso:: :py:func:`color2`

.. seealso:: :py:func:`colorRamp`
%End

    void setColorRamp( QgsColorRamp *ramp );
%Docstring
Sets the color ramp used to draw the shapeburst fill. Color ramps are only used if setColorType is set ShapeburstColorType.ColorRamp.

:param ramp: color ramp to use for shapeburst fill

.. versionadded:: 2.3

.. seealso:: :py:func:`setColorType`

.. seealso:: :py:func:`colorRamp`
%End

    QgsColorRamp *colorRamp();
%Docstring
Returns the color ramp used for the shapeburst fill. The color ramp is only used if the colorType is set to ShapeburstColorType.ColorRamp

:return: a QgsColorRamp color ramp

.. versionadded:: 2.3

.. seealso:: :py:func:`setColorRamp`

.. seealso:: :py:func:`colorType`
%End

    void setColor2( const QColor &color2 );
%Docstring
Sets the color for the endpoint of the shapeburst fill. This color is only used if setColorType is set ShapeburstColorType.SimpleTwoColor.

:param color2: QColor to use for endpoint of gradient

.. versionadded:: 2.3

.. seealso:: :py:func:`setColorType`

.. seealso:: :py:func:`color2`
%End

    QColor color2() const;
%Docstring
Returns the color used for the endpoint of the shapeburst fill. This color is only used if the colorType is set to ShapeburstColorType.SimpleTwoColor

:return: a QColor indicating the color of the endpoint of the gradient

.. versionadded:: 2.3

.. seealso:: :py:func:`setColor2`

.. seealso:: :py:func:`colorType`
%End

    void setIgnoreRings( bool ignoreRings );
%Docstring
Sets whether the shapeburst fill should ignore polygon rings when calculating
the buffered shading.

:param ignoreRings: Set to true if buffers should ignore interior rings for polygons.

.. versionadded:: 2.3

.. seealso:: :py:func:`ignoreRings`
%End

    bool ignoreRings() const;
%Docstring
Returns whether the shapeburst fill is set to ignore polygon interior rings.

:return: True if the shapeburst fill will ignore interior rings when calculating buffered shading.

.. versionadded:: 2.3

.. seealso:: :py:func:`setIgnoreRings`
%End

    void setOffset( QPointF offset );
%Docstring
Sets the offset for the shapeburst fill.

:param offset: QPointF indicating the horizontal/vertical offset amount

.. versionadded:: 2.3

.. seealso:: :py:func:`offset`

.. seealso:: :py:func:`setOffsetUnit`
%End

    QPointF offset() const;
%Docstring
Returns the offset for the shapeburst fill.

:return: a QPointF indicating the horizontal/vertical offset amount

.. versionadded:: 2.3

.. seealso:: :py:func:`setOffset`

.. seealso:: :py:func:`offsetUnit`
%End

    void setOffsetUnit( QgsUnitTypes::RenderUnit unit );
%Docstring
Sets the units used for the offset for the shapeburst fill.

:param unit: units for fill offset

.. versionadded:: 2.3

.. seealso:: :py:func:`setOffset`

.. seealso:: :py:func:`offsetUnit`
%End

    QgsUnitTypes::RenderUnit offsetUnit() const;
%Docstring
Returns the units used for the offset of the shapeburst fill.

:return: units used for the fill offset

.. versionadded:: 2.3

.. seealso:: :py:func:`offset`

.. seealso:: :py:func:`setOffsetUnit`
%End

    void setOffsetMapUnitScale( const QgsMapUnitScale &scale );
    const QgsMapUnitScale &offsetMapUnitScale() const;

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

:param unit: symbol units

.. seealso:: :py:func:`strokeWidthUnit`
%End

    QgsUnitTypes::RenderUnit strokeWidthUnit() const;
%Docstring
Returns the units for the symbol's stroke width.

.. seealso:: :py:func:`setStrokeWidthUnit`
%End

    void setStrokeWidthMapUnitScale( const QgsMapUnitScale &scale );
    const QgsMapUnitScale &strokeWidthMapUnitScale() const;

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

:param imagePath: path to image file

.. seealso:: :py:func:`imageFilePath`
%End

    QString imageFilePath() const;
%Docstring
The path to the raster image used for the fill.

:return: path to image file

.. seealso:: :py:func:`setImageFilePath`
%End

    void setCoordinateMode( const FillCoordinateMode mode );
%Docstring
Set the coordinate mode for fill. Controls how the top left corner of the image
fill is positioned relative to the feature.

:param mode: coordinate mode

.. seealso:: :py:func:`coordinateMode`
%End

    FillCoordinateMode coordinateMode() const;
%Docstring
Coordinate mode for fill. Controls how the top left corner of the image
fill is positioned relative to the feature.

:return: coordinate mode

.. seealso:: :py:func:`setCoordinateMode`
%End

    void setOpacity( const double opacity );
%Docstring
Sets the ``opacity`` for the raster image used in the fill.

:param opacity: opacity value between 0 (fully transparent) and 1 (fully opaque)

.. seealso:: :py:func:`opacity`
%End

    double opacity() const;
%Docstring
Returns the opacity for the raster image used in the fill.

:return: opacity value between 0 (fully transparent) and 1 (fully opaque)

.. seealso:: :py:func:`setOpacity`
%End

    void setOffset( QPointF offset );
%Docstring
Sets the offset for the fill.

:param offset: offset for fill

.. seealso:: :py:func:`offset`

.. seealso:: :py:func:`setOffsetUnit`

.. seealso:: :py:func:`setOffsetMapUnitScale`
%End

    QPointF offset() const;
%Docstring
Returns the offset for the fill.

:return: offset for fill

.. seealso:: :py:func:`setOffset`

.. seealso:: :py:func:`offsetUnit`

.. seealso:: :py:func:`offsetMapUnitScale`
%End

    void setOffsetUnit( const QgsUnitTypes::RenderUnit unit );
%Docstring
Sets the units for the fill's offset.

:param unit: units for offset

.. seealso:: :py:func:`offsetUnit`

.. seealso:: :py:func:`setOffset`

.. seealso:: :py:func:`setOffsetMapUnitScale`
%End

    QgsUnitTypes::RenderUnit offsetUnit() const;
%Docstring
Returns the units for the fill's offset.

:return: units for offset

.. seealso:: :py:func:`setOffsetUnit`

.. seealso:: :py:func:`offset`

.. seealso:: :py:func:`offsetMapUnitScale`
%End

    void setOffsetMapUnitScale( const QgsMapUnitScale &scale );
%Docstring
Sets the map unit scale for the fill's offset.

:param scale: map unit scale for offset

.. seealso:: :py:func:`offsetMapUnitScale`

.. seealso:: :py:func:`setOffset`

.. seealso:: :py:func:`setOffsetUnit`
%End

    const QgsMapUnitScale &offsetMapUnitScale() const;
%Docstring
Returns the map unit scale for the fill's offset.

:return: map unit scale for offset

.. seealso:: :py:func:`setOffsetMapUnitScale`

.. seealso:: :py:func:`offset`

.. seealso:: :py:func:`offsetUnit`
%End

    void setWidth( const double width );
%Docstring
Sets the width for scaling the image used in the fill. The image's height will also be
scaled to maintain the image's aspect ratio.

:param width: width for scaling the image

.. seealso:: :py:func:`width`

.. seealso:: :py:func:`setWidthUnit`

.. seealso:: :py:func:`setWidthMapUnitScale`
%End

    double width() const;
%Docstring
Returns the width used for scaling the image used in the fill. The image's height is
scaled to maintain the image's aspect ratio.

:return: width used for scaling the image

.. seealso:: :py:func:`setWidth`

.. seealso:: :py:func:`widthUnit`

.. seealso:: :py:func:`widthMapUnitScale`
%End

    void setWidthUnit( const QgsUnitTypes::RenderUnit unit );
%Docstring
Sets the units for the image's width.

:param unit: units for width

.. seealso:: :py:func:`widthUnit`

.. seealso:: :py:func:`setWidth`

.. seealso:: :py:func:`setWidthMapUnitScale`
%End

    QgsUnitTypes::RenderUnit widthUnit() const;
%Docstring
Returns the units for the image's width.

:return: units for width

.. seealso:: :py:func:`setWidthUnit`

.. seealso:: :py:func:`width`

.. seealso:: :py:func:`widthMapUnitScale`
%End

    void setWidthMapUnitScale( const QgsMapUnitScale &scale );
%Docstring
Sets the map unit scale for the image's width.

:param scale: map unit scale for width

.. seealso:: :py:func:`widthMapUnitScale`

.. seealso:: :py:func:`setWidth`

.. seealso:: :py:func:`setWidthUnit`
%End

    const QgsMapUnitScale &widthMapUnitScale() const;
%Docstring
Returns the map unit scale for the image's width.

:return: map unit scale for width

.. seealso:: :py:func:`setWidthMapUnitScale`

.. seealso:: :py:func:`width`

.. seealso:: :py:func:`widthUnit`
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


    virtual QgsStringMap properties() const;


    virtual QgsSVGFillSymbolLayer *clone() const /Factory/;


    virtual void toSld( QDomDocument &doc, QDomElement &element, const QgsStringMap &props ) const;


    void setSvgFilePath( const QString &svgPath );
    QString svgFilePath() const;
    void setPatternWidth( double width );
    double patternWidth() const;

    void setSvgFillColor( const QColor &c );
    QColor svgFillColor() const;

    void setSvgStrokeColor( const QColor &c );
    QColor svgStrokeColor() const;
    void setSvgStrokeWidth( double w );
    double svgStrokeWidth() const;

    void setPatternWidthUnit( QgsUnitTypes::RenderUnit unit );
%Docstring
Sets the units for the width of the SVG images in the pattern.

:param unit: width units

.. seealso:: :py:func:`patternWidthUnit`
%End

    QgsUnitTypes::RenderUnit patternWidthUnit() const;
%Docstring
Returns the units for the width of the SVG images in the pattern.

.. seealso:: :py:func:`setPatternWidthUnit`
%End

    void setPatternWidthMapUnitScale( const QgsMapUnitScale &scale );
    const QgsMapUnitScale &patternWidthMapUnitScale() const;

    void setSvgStrokeWidthUnit( QgsUnitTypes::RenderUnit unit );
%Docstring
Sets the units for the stroke width.

:param unit: width units

.. seealso:: :py:func:`svgStrokeWidthUnit`
%End

    QgsUnitTypes::RenderUnit svgStrokeWidthUnit() const;
%Docstring
Returns the units for the stroke width.

.. seealso:: :py:func:`setSvgStrokeWidthUnit`
%End

    void setSvgStrokeWidthMapUnitScale( const QgsMapUnitScale &scale );
    const QgsMapUnitScale &svgStrokeWidthMapUnitScale() const;

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
    static QgsSymbolLayer *createFromSld( QDomElement &element ) /Factory/;

    virtual QString layerType() const;


    virtual void startRender( QgsSymbolRenderContext &context );


    virtual void stopRender( QgsSymbolRenderContext &context );


    virtual QgsStringMap properties() const;


    virtual QgsLinePatternFillSymbolLayer *clone() const /Factory/;


    virtual void toSld( QDomDocument &doc, QDomElement &element, const QgsStringMap &props ) const;


    virtual double estimateMaxBleed( const QgsRenderContext &context ) const;


    QString ogrFeatureStyleWidth( double widthScaleFactor ) const;

    void setLineAngle( double a );
    double lineAngle() const;

    void setDistance( double d );
%Docstring
Sets the distance between lines in the fill pattern.

:param d: distance. Units are specified by setDistanceUnit()

.. seealso:: :py:func:`distance`

.. seealso:: :py:func:`setDistanceUnit`
%End

    double distance() const;
%Docstring
Returns the distance between lines in the fill pattern. Units are retrieved by distanceUnit().

.. seealso:: :py:func:`setDistance`

.. seealso:: :py:func:`distanceUnit`
%End

    void setLineWidth( double w );
    double lineWidth() const;
    virtual void setColor( const QColor &c );

    virtual QColor color() const;

    void setOffset( double offset );
    double offset() const;

    void setDistanceUnit( QgsUnitTypes::RenderUnit unit );
%Docstring
Sets the units for the distance between lines in the fill pattern.

:param unit: distance units

.. seealso:: :py:func:`distanceUnit`

.. seealso:: :py:func:`setDistance`
%End

    QgsUnitTypes::RenderUnit distanceUnit() const;
%Docstring
Returns the units for the distance between lines in the fill pattern.

.. seealso:: :py:func:`setDistanceUnit`

.. seealso:: :py:func:`distance`
%End

    void setDistanceMapUnitScale( const QgsMapUnitScale &scale );
    const QgsMapUnitScale &distanceMapUnitScale() const;

    void setLineWidthUnit( QgsUnitTypes::RenderUnit unit );
%Docstring
Sets the units for the line's width.

:param unit: width units

.. seealso:: :py:func:`lineWidthUnit`
%End

    QgsUnitTypes::RenderUnit lineWidthUnit() const;
%Docstring
Returns the units for the line's width.

.. seealso:: :py:func:`setLineWidthUnit`
%End

    void setLineWidthMapUnitScale( const QgsMapUnitScale &scale );
    const QgsMapUnitScale &lineWidthMapUnitScale() const;

    void setOffsetUnit( QgsUnitTypes::RenderUnit unit );
%Docstring
Sets the units for the line pattern's offset.

:param unit: offset units

.. seealso:: :py:func:`offsetUnit`
%End

    QgsUnitTypes::RenderUnit offsetUnit() const;
%Docstring
Returns the units for the line pattern's offset.

.. seealso:: :py:func:`setOffsetUnit`
%End

    void setOffsetMapUnitScale( const QgsMapUnitScale &scale );
    const QgsMapUnitScale &offsetMapUnitScale() const;

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
    static QgsSymbolLayer *createFromSld( QDomElement &element ) /Factory/;

    virtual QString layerType() const;


    virtual void startRender( QgsSymbolRenderContext &context );


    virtual void stopRender( QgsSymbolRenderContext &context );


    virtual QgsStringMap properties() const;


    virtual QgsPointPatternFillSymbolLayer *clone() const /Factory/;


    virtual void toSld( QDomDocument &doc, QDomElement &element, const QgsStringMap &props ) const;


    virtual double estimateMaxBleed( const QgsRenderContext &context ) const;


    double distanceX() const;
    void setDistanceX( double d );

    double distanceY() const;
    void setDistanceY( double d );

    double displacementX() const;
    void setDisplacementX( double d );

    double displacementY() const;
    void setDisplacementY( double d );

    virtual bool setSubSymbol( QgsSymbol *symbol /Transfer/ );

    virtual QgsSymbol *subSymbol();

    void setDistanceXUnit( QgsUnitTypes::RenderUnit unit );
%Docstring
Sets the units for the horizontal distance between points in the pattern.

:param unit: distance units

.. seealso:: :py:func:`distanceXUnit`

.. seealso:: :py:func:`setDistanceYUnit`
%End

    QgsUnitTypes::RenderUnit distanceXUnit() const;
%Docstring
Returns the units for the horizontal distance between points in the pattern.

.. seealso:: :py:func:`setDistanceXUnit`

.. seealso:: :py:func:`distanceYUnit`
%End

    void setDistanceXMapUnitScale( const QgsMapUnitScale &scale );
    const QgsMapUnitScale &distanceXMapUnitScale() const;

    void setDistanceYUnit( QgsUnitTypes::RenderUnit unit );
%Docstring
Sets the units for the vertical distance between points in the pattern.

:param unit: distance units

.. seealso:: :py:func:`distanceYUnit`

.. seealso:: :py:func:`setDistanceXUnit`
%End

    QgsUnitTypes::RenderUnit distanceYUnit() const;
%Docstring
Returns the units for the vertical distance between points in the pattern.

.. seealso:: :py:func:`setDistanceYUnit`

.. seealso:: :py:func:`distanceXUnit`
%End

    void setDistanceYMapUnitScale( const QgsMapUnitScale &scale );
    const QgsMapUnitScale &distanceYMapUnitScale() const;

    void setDisplacementXUnit( QgsUnitTypes::RenderUnit unit );
%Docstring
Sets the units for the horizontal displacement between rows in the pattern.

:param unit: displacement units

.. seealso:: :py:func:`displacementXUnit`

.. seealso:: :py:func:`setDisplacementYUnit`
%End

    QgsUnitTypes::RenderUnit displacementXUnit() const;
%Docstring
Returns the units for the horizontal displacement between rows in the pattern.

.. seealso:: :py:func:`setDisplacementXUnit`

.. seealso:: :py:func:`displacementYUnit`
%End

    void setDisplacementXMapUnitScale( const QgsMapUnitScale &scale );
    const QgsMapUnitScale &displacementXMapUnitScale() const;

    void setDisplacementYUnit( QgsUnitTypes::RenderUnit unit );
%Docstring
Sets the units for the vertical displacement between rows in the pattern.

:param unit: displacement units

.. seealso:: :py:func:`displacementYUnit`

.. seealso:: :py:func:`setDisplacementXUnit`
%End

    QgsUnitTypes::RenderUnit displacementYUnit() const;
%Docstring
Returns the units for the vertical displacement between rows in the pattern.

.. seealso:: :py:func:`setDisplacementYUnit`

.. seealso:: :py:func:`displacementXUnit`
%End

    void setDisplacementYMapUnitScale( const QgsMapUnitScale &scale );
    const QgsMapUnitScale &displacementYMapUnitScale() const;

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
    static QgsSymbolLayer *createFromSld( QDomElement &element ) /Factory/;


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

    void setPointOnAllParts( bool pointOnAllParts );
%Docstring
Sets whether a point is drawn for all parts or only on the biggest part of multi-part features.

.. versionadded:: 2.16
%End

    bool pointOnAllParts() const;
%Docstring
Returns whether a point is drawn for all parts or only on the biggest part of multi-part features.

.. versionadded:: 2.16
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
