/***************************************************************************
 qgsfillsymbollayerv2.h
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

#ifndef QGSFILLSYMBOLLAYERV2_H
#define QGSFILLSYMBOLLAYERV2_H

#include "qgssymbollayerv2.h"

#define DEFAULT_SIMPLEFILL_COLOR        QColor(0,0,255)
#define DEFAULT_SIMPLEFILL_STYLE        Qt::SolidPattern
#define DEFAULT_SIMPLEFILL_BORDERCOLOR  QColor(0,0,0)
#define DEFAULT_SIMPLEFILL_BORDERSTYLE  Qt::SolidLine
#define DEFAULT_SIMPLEFILL_BORDERWIDTH  DEFAULT_LINE_WIDTH
#define DEFAULT_SIMPLEFILL_JOINSTYLE    Qt::BevelJoin

#define INF 1E20

#include <QPen>
#include <QBrush>

class CORE_EXPORT QgsSimpleFillSymbolLayerV2 : public QgsFillSymbolLayerV2
{
  public:
    QgsSimpleFillSymbolLayerV2( QColor color = DEFAULT_SIMPLEFILL_COLOR,
                                Qt::BrushStyle style = DEFAULT_SIMPLEFILL_STYLE,
                                QColor borderColor = DEFAULT_SIMPLEFILL_BORDERCOLOR,
                                Qt::PenStyle borderStyle = DEFAULT_SIMPLEFILL_BORDERSTYLE,
                                double borderWidth = DEFAULT_SIMPLEFILL_BORDERWIDTH,
                                Qt::PenJoinStyle penJoinStyle = DEFAULT_SIMPLEFILL_JOINSTYLE
                              );

    // static stuff

    static QgsSymbolLayerV2* create( const QgsStringMap& properties = QgsStringMap() );
    static QgsSymbolLayerV2* createFromSld( QDomElement &element );

    // implemented from base classes

    QString layerType() const;

    void startRender( QgsSymbolV2RenderContext& context );

    void stopRender( QgsSymbolV2RenderContext& context );

    void renderPolygon( const QPolygonF& points, QList<QPolygonF>* rings, QgsSymbolV2RenderContext& context );

    QgsStringMap properties() const;

    QgsSymbolLayerV2* clone() const;

    void toSld( QDomDocument &doc, QDomElement &element, QgsStringMap props ) const;

    QString ogrFeatureStyle( double mmScaleFactor, double mapUnitScaleFactor ) const;

    Qt::BrushStyle brushStyle() const { return mBrushStyle; }
    void setBrushStyle( Qt::BrushStyle style ) { mBrushStyle = style; }

    QColor borderColor() const { return mBorderColor; }
    void setBorderColor( QColor borderColor ) { mBorderColor = borderColor; }

    /** Get outline color.
     * @note added in 2.1 */
    QColor outlineColor() const { return borderColor(); }
    /** Set outline color.
     * @note added in 2.1 */
    void setOutlineColor( const QColor& color ) { setBorderColor( color ); }

    /** Get fill color.
     * @note added in 2.1 */
    QColor fillColor() const { return color(); }
    /** Set fill color.
     * @note added in 2.1 */
    void setFillColor( const QColor& color ) { setColor( color ); }

    Qt::PenStyle borderStyle() const { return mBorderStyle; }
    void setBorderStyle( Qt::PenStyle borderStyle ) { mBorderStyle = borderStyle; }

    double borderWidth() const { return mBorderWidth; }
    void setBorderWidth( double borderWidth ) { mBorderWidth = borderWidth; }

    Qt::PenJoinStyle penJoinStyle() const { return mPenJoinStyle; }
    void setPenJoinStyle( Qt::PenJoinStyle style ) { mPenJoinStyle = style; }

    void setOffset( QPointF offset ) { mOffset = offset; }
    QPointF offset() { return mOffset; }

    void setBorderWidthUnit( QgsSymbolV2::OutputUnit unit ) { mBorderWidthUnit = unit; }
    QgsSymbolV2::OutputUnit borderWidthUnit() const { return mBorderWidthUnit; }

    void setBorderWidthMapUnitScale( const QgsMapUnitScale& scale ) { mBorderWidthMapUnitScale = scale; }
    const QgsMapUnitScale& borderWidthMapUnitScale() const { return mBorderWidthMapUnitScale; }

    void setOffsetUnit( QgsSymbolV2::OutputUnit unit ) { mOffsetUnit = unit; }
    QgsSymbolV2::OutputUnit offsetUnit() const { return mOffsetUnit; }

    void setOffsetMapUnitScale( const QgsMapUnitScale& scale ) { mOffsetMapUnitScale = scale; }
    const QgsMapUnitScale& offsetMapUnitScale() const { return mOffsetMapUnitScale; }

    void setOutputUnit( QgsSymbolV2::OutputUnit unit );
    QgsSymbolV2::OutputUnit outputUnit() const;

    void setMapUnitScale( const QgsMapUnitScale &scale );
    QgsMapUnitScale mapUnitScale() const;

    double estimateMaxBleed() const;

    double dxfWidth( const QgsDxfExport& e, const QgsSymbolV2RenderContext& context ) const;
    QColor dxfColor( const QgsSymbolV2RenderContext& context ) const;
    Qt::PenStyle dxfPenStyle() const;
    QColor dxfBrushColor( const QgsSymbolV2RenderContext& context ) const;
    Qt::BrushStyle dxfBrushStyle() const;

  protected:
    QBrush mBrush;
    QBrush mSelBrush;
    Qt::BrushStyle mBrushStyle;
    QColor mBorderColor;
    Qt::PenStyle mBorderStyle;
    double mBorderWidth;
    QgsSymbolV2::OutputUnit mBorderWidthUnit;
    QgsMapUnitScale mBorderWidthMapUnitScale;
    Qt::PenJoinStyle mPenJoinStyle;
    QPen mPen;
    QPen mSelPen;

    QPointF mOffset;
    QgsSymbolV2::OutputUnit mOffsetUnit;
    QgsMapUnitScale mOffsetMapUnitScale;

  private:
    //helper functions for data defined symbology
    void applyDataDefinedSymbology( QgsSymbolV2RenderContext& context, QBrush& brush, QPen& pen, QPen& selPen );
};

class QgsVectorColorRampV2;

class CORE_EXPORT QgsGradientFillSymbolLayerV2 : public QgsFillSymbolLayerV2
{
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

    QgsGradientFillSymbolLayerV2( QColor color = DEFAULT_SIMPLEFILL_COLOR,
                                  QColor color2 = Qt::white,
                                  GradientColorType gradientColorType = SimpleTwoColor,
                                  GradientType gradientType = Linear,
                                  GradientCoordinateMode coordinateMode = Feature,
                                  GradientSpread gradientSpread = Pad
                                );

    virtual ~QgsGradientFillSymbolLayerV2();

    // static stuff

    static QgsSymbolLayerV2* create( const QgsStringMap& properties = QgsStringMap() );

    // implemented from base classes

    QString layerType() const;

    void startRender( QgsSymbolV2RenderContext& context );

    void stopRender( QgsSymbolV2RenderContext& context );

    void renderPolygon( const QPolygonF& points, QList<QPolygonF>* rings, QgsSymbolV2RenderContext& context );

    QgsStringMap properties() const;

    QgsSymbolLayerV2* clone() const;

    double estimateMaxBleed() const;

    /**Type of gradient, eg linear or radial*/
    GradientType gradientType() const { return mGradientType; }
    void setGradientType( GradientType gradientType ) { mGradientType = gradientType; }

    /**Gradient color mode, controls how gradient color stops are created*/
    GradientColorType gradientColorType() const { return mGradientColorType; }
    void setGradientColorType( GradientColorType gradientColorType ) { mGradientColorType = gradientColorType; }

    /**Color ramp used for the gradient fill, only used if the gradient color type is set to ColorRamp*/
    QgsVectorColorRampV2* colorRamp() { return mGradientRamp; }
    void setColorRamp( QgsVectorColorRampV2* ramp );

    /**Color for endpoint of gradient, only used if the gradient color type is set to SimpleTwoColor*/
    QColor color2() const { return mColor2; }
    void setColor2( QColor color2 ) { mColor2 = color2; }

    /**Coordinate mode for gradient. Controls how the gradient stops are positioned.*/
    GradientCoordinateMode coordinateMode() const { return mCoordinateMode; }
    void setCoordinateMode( GradientCoordinateMode coordinateMode ) { mCoordinateMode = coordinateMode; }

    /**Gradient spread mode. Controls how the gradient behaves outside of the predefined stops*/
    GradientSpread gradientSpread() const { return mGradientSpread; }
    void setGradientSpread( GradientSpread gradientSpread ) { mGradientSpread = gradientSpread; }

    /**Starting point of gradient fill, in the range [0,0] - [1,1]*/
    void setReferencePoint1( QPointF referencePoint ) { mReferencePoint1 = referencePoint; }
    QPointF referencePoint1() const { return mReferencePoint1; }

    /**Sets the starting point of the gradient to be the feature centroid*/
    void setReferencePoint1IsCentroid( bool isCentroid ) { mReferencePoint1IsCentroid = isCentroid; }
    bool referencePoint1IsCentroid() const { return mReferencePoint1IsCentroid; }

    /**End point of gradient fill, in the range [0,0] - [1,1]*/
    void setReferencePoint2( QPointF referencePoint ) { mReferencePoint2 = referencePoint; }
    QPointF referencePoint2() const { return mReferencePoint2; }

    /**Sets the end point of the gradient to be the feature centroid*/
    void setReferencePoint2IsCentroid( bool isCentroid ) { mReferencePoint2IsCentroid = isCentroid; }
    bool referencePoint2IsCentroid() const { return mReferencePoint2IsCentroid; }

    /**Rotation angle for gradient fill. Can be used to rotate a gradient around its centre point*/
    void setAngle( double angle ) { mAngle = angle; }
    double angle() const { return mAngle; }

    /**Offset for gradient fill*/
    void setOffset( QPointF offset ) { mOffset = offset; }
    QPointF offset() const { return mOffset; }

    /**Units for gradient fill offset*/
    void setOffsetUnit( QgsSymbolV2::OutputUnit unit ) { mOffsetUnit = unit; }
    QgsSymbolV2::OutputUnit offsetUnit() const { return mOffsetUnit; }

    void setOffsetMapUnitScale( const QgsMapUnitScale& scale ) { mOffsetMapUnitScale = scale; }
    const QgsMapUnitScale& offsetMapUnitScale() const { return mOffsetMapUnitScale; }

    void setOutputUnit( QgsSymbolV2::OutputUnit unit );
    QgsSymbolV2::OutputUnit outputUnit() const;

    void setMapUnitScale( const QgsMapUnitScale &scale );
    QgsMapUnitScale mapUnitScale() const;

  protected:
    QBrush mBrush;
    QBrush mSelBrush;

    GradientColorType mGradientColorType;
    QColor mColor2;
    QgsVectorColorRampV2* mGradientRamp;
    GradientType mGradientType;
    GradientCoordinateMode mCoordinateMode;
    GradientSpread mGradientSpread;

    QPointF mReferencePoint1;
    bool mReferencePoint1IsCentroid;
    QPointF mReferencePoint2;
    bool mReferencePoint2IsCentroid;
    double mAngle;

    QPointF mOffset;
    QgsSymbolV2::OutputUnit mOffsetUnit;
    QgsMapUnitScale mOffsetMapUnitScale;

  private:

    //helper functions for data defined symbology
    void applyDataDefinedSymbology( QgsSymbolV2RenderContext& context, const QPolygonF& points );

    /**Applies the gradient to a brush*/
    void applyGradient( const QgsSymbolV2RenderContext& context, QBrush& brush, const QColor& color, const QColor& color2,
                        const GradientColorType &gradientColorType, QgsVectorColorRampV2 *gradientRamp, const GradientType &gradientType,
                        const GradientCoordinateMode &coordinateMode, const GradientSpread &gradientSpread,
                        const QPointF &referencePoint1, const QPointF &referencePoint2, const double angle );

    /**rotates a reference point by a specified angle around the point (0.5, 0.5)*/
    QPointF rotateReferencePoint( const QPointF & refPoint, double angle );
};

class CORE_EXPORT QgsShapeburstFillSymbolLayerV2 : public QgsFillSymbolLayerV2
{
  public:

    enum ShapeburstColorType
    {
      SimpleTwoColor,
      ColorRamp
    };

    QgsShapeburstFillSymbolLayerV2( QColor color = DEFAULT_SIMPLEFILL_COLOR, QColor color2 = Qt::white,
                                    ShapeburstColorType colorType = SimpleTwoColor,
                                    int blurRadius = 0, bool useWholeShape = true, double maxDistance = 5 );

    virtual ~QgsShapeburstFillSymbolLayerV2();

    // static stuff

    static QgsSymbolLayerV2* create( const QgsStringMap& properties = QgsStringMap() );

    // implemented from base classes

    QString layerType() const;

    void startRender( QgsSymbolV2RenderContext& context );

    void stopRender( QgsSymbolV2RenderContext& context );

    void renderPolygon( const QPolygonF& points, QList<QPolygonF>* rings, QgsSymbolV2RenderContext& context );

    QgsStringMap properties() const;

    QgsSymbolLayerV2* clone() const;

    double estimateMaxBleed() const;

    /**Sets the blur radius, which controls the amount of blurring applied to the fill.
     * @param blurRadius Radius for fill blur. Values between 0 - 17 are valid, where higher values results in a stronger blur. Set to 0 to disable blur.
     * @note added in 2.3
     * @see blurRadius
    */
    void setBlurRadius( int blurRadius ) { mBlurRadius = blurRadius; }
    /**Returns the blur radius, which controls the amount of blurring applied to the fill.
     * @returns Integer representing the radius for fill blur. Higher values indicate a stronger blur. A 0 value indicates that blurring is disabled.
     * @note added in 2.3
     * @see setBlurRadius
    */
    int blurRadius() const { return mBlurRadius; }

    /**Sets whether the shapeburst fill should be drawn using the entire shape.
     * @param useWholeShape Set to true if shapeburst should cover entire shape. If false, setMaxDistance is used to calculate how far from the boundary of the shape should
     * be shaded
     * @note added in 2.3
     * @see useWholeShape
     * @see setMaxDistance
    */
    void setUseWholeShape( bool useWholeShape ) { mUseWholeShape = useWholeShape; }
    /**Returns whether the shapeburst fill is set to cover the entire shape.
     * @returns True if shapeburst fill will cover the entire shape. If false, shapeburst is drawn to a distance of maxDistance from the polygon's boundary.
     * @note added in 2.3
     * @see setUseWholeShape
     * @see maxDistance
    */
    bool useWholeShape() const { return mUseWholeShape; }

    /**Sets the maximum distance to shape inside of the shape from the polygon's boundary.
     * @param maxDistance distance from boundary to shade. setUseWholeShape must be set to false for this parameter to take effect. Distance unit is controlled by setDistanceUnit.
     * @note added in 2.3
     * @see maxDistance
     * @see setUseWholeShape
     * @see setDistanceUnit
    */
    void setMaxDistance( double maxDistance ) { mMaxDistance = maxDistance; }
    /**Returns the maximum distance from the shape's boundary which is shaded. This parameter is only effective if useWholeShape is false.
     * @returns the maximum distance from the polygon's boundary which is shaded. Distance units are indicated by distanceUnit.
     * @note added in 2.3
     * @see useWholeShape
     * @see setMaxDistance
     * @see distanceUnit
    */
    double maxDistance() const { return mMaxDistance; }

    /**Sets the unit for the maximum distance to shade inside of the shape from the polygon's boundary.
     * @param unit distance unit for the maximum distance
     * @note added in 2.3
     * @see setMaxDistance
     * @see distanceUnit
    */
    void setDistanceUnit( QgsSymbolV2::OutputUnit unit ) { mDistanceUnit = unit; }
    /**Returns the unit for the maximum distance to shade inside of the shape from the polygon's boundary.
     * @returns distance unit for the maximum distance
     * @note added in 2.3
     * @see maxDistance
     * @see setDistanceUnit
    */
    QgsSymbolV2::OutputUnit distanceUnit() const { return mDistanceUnit; }

    void setDistanceMapUnitScale( const QgsMapUnitScale& scale ) { mDistanceMapUnitScale = scale; }
    const QgsMapUnitScale& distanceMapUnitScale() const { return mDistanceMapUnitScale; }

    /**Sets the color mode to use for the shapeburst fill. Shapeburst can either be drawn using a QgsVectorColorRampV2 color ramp
     * or by simply specificing a start and end color. setColorType is used to specify which mode to use for the fill.
     * @param colorType color type to use for shapeburst fill
     * @note added in 2.3
     * @see colorType
     * @see setColor
     * @see setColor2
     * @see setColorRamp
    */
    void setColorType( ShapeburstColorType colorType ) { mColorType = colorType; }
    /**Returns the color mode used for the shapeburst fill. Shapeburst can either be drawn using a QgsVectorColorRampV2 color ramp
     * or by simply specificing a start and end color.
     * @returns current color mode used for the shapeburst fill
     * @note added in 2.3
     * @see setColorType
     * @see color
     * @see color2
     * @see colorRamp
    */
    ShapeburstColorType colorType() const { return mColorType; }

    /**Sets the color ramp used to draw the shapeburst fill. Color ramps are only used if setColorType is set ShapeburstColorType::ColorRamp.
     * @param ramp color ramp to use for shapeburst fill
     * @note added in 2.3
     * @see setColorType
     * @see colorRamp
    */
    void setColorRamp( QgsVectorColorRampV2* ramp );
    /**Returns the color ramp used for the shapeburst fill. The color ramp is only used if the colorType is set to ShapeburstColorType::ColorRamp
     * @returns a QgsVectorColorRampV2 color ramp
     * @note added in 2.3
     * @see setColorRamp
     * @see colorType
    */
    QgsVectorColorRampV2* colorRamp() { return mGradientRamp; }

    /**Sets the color for the endpoint of the shapeburst fill. This color is only used if setColorType is set ShapeburstColorType::SimpleTwoColor.
     * @param color2 QColor to use for endpoint of gradient
     * @note added in 2.3
     * @see setColorType
     * @see color2
    */
    void setColor2( QColor color2 ) { mColor2 = color2; }
    /**Returns the color used for the endpoint of the shapeburst fill. This color is only used if the colorType is set to ShapeburstColorType::SimpleTwoColor
     * @returns a QColor indicating the color of the endpoint of the gradient
     * @note added in 2.3
     * @see setColor2
     * @see colorType
    */
    QColor color2() const { return mColor2; }

    /**Sets whether the shapeburst fill should ignore polygon rings when calculating
     * the buffered shading.
     * @param ignoreRings Set to true if buffers should ignore interior rings for polygons.
     * @note added in 2.3
     * @see ignoreRings
    */
    void setIgnoreRings( bool ignoreRings ) { mIgnoreRings = ignoreRings; }
    /**Returns whether the shapeburst fill is set to ignore polygon interior rings.
     * @returns True if the shapeburst fill will ignore interior rings when calculating buffered shading.
     * @note added in 2.3
     * @see setIgnoreRings
    */
    bool ignoreRings() const { return mIgnoreRings; }

    /**Sets the offset for the shapeburst fill.
     * @param offset QPointF indicating the horizontal/vertical offset amount
     * @note added in 2.3
     * @see offset
     * @see setOffsetUnit
    */
    void setOffset( QPointF offset ) { mOffset = offset; }
    /**Returns the offset for the shapeburst fill.
     * @returns a QPointF indicating the horizontal/vertical offset amount
     * @note added in 2.3
     * @see setOffset
     * @see offsetUnit
    */
    QPointF offset() const { return mOffset; }

    /**Sets the units used for the offset for the shapeburst fill.
     * @param unit units for fill offset
     * @note added in 2.3
     * @see setOffset
     * @see offsetUnit
    */
    void setOffsetUnit( QgsSymbolV2::OutputUnit unit ) { mOffsetUnit = unit; }
    /**Returns the units used for the offset of the shapeburst fill.
     * @returns units used for the fill offset
     * @note added in 2.3
     * @see offset
     * @see setOffsetUnit
    */
    QgsSymbolV2::OutputUnit offsetUnit() const { return mOffsetUnit; }

    void setOffsetMapUnitScale( const QgsMapUnitScale& scale ) { mOffsetMapUnitScale = scale; }
    const QgsMapUnitScale& offsetMapUnitScale() const { return mOffsetMapUnitScale; }

    void setOutputUnit( QgsSymbolV2::OutputUnit unit );
    QgsSymbolV2::OutputUnit outputUnit() const;

    void setMapUnitScale( const QgsMapUnitScale &scale );
    QgsMapUnitScale mapUnitScale() const;

  protected:
    QBrush mBrush;
    QBrush mSelBrush;

    int mBlurRadius;

    bool mUseWholeShape;
    double mMaxDistance;
    QgsSymbolV2::OutputUnit mDistanceUnit;
    QgsMapUnitScale mDistanceMapUnitScale;

    ShapeburstColorType mColorType;
    QColor mColor2;
    QgsVectorColorRampV2* mGradientRamp;
    QgsVectorColorRampV2* mTwoColorGradientRamp;

    bool mIgnoreRings;

    QPointF mOffset;
    QgsSymbolV2::OutputUnit mOffsetUnit;
    QgsMapUnitScale mOffsetMapUnitScale;

  private:

    //helper functions for data defined symbology
    void applyDataDefinedSymbology( QgsSymbolV2RenderContext& context, QColor& color, QColor& color2, int& blurRadius, bool& useWholeShape,
                                    double& maxDistance, bool &ignoreRings );

    /* distance transform of a 1d function using squared distance */
    void distanceTransform1d( double *f, int n, int *v, double *z, double *d );
    /* distance transform of 2d function using squared distance */
    void distanceTransform2d( double * im, int width, int height );
    /* distance transform of a binary QImage */
    double * distanceTransform( QImage * im );

    /* fills a QImage with values from an array of doubles containing squared distance transform values */
    void dtArrayToQImage( double * array, QImage *im, QgsVectorColorRampV2* ramp, double layerAlpha = 1, bool useWholeShape = true, int maxPixelDistance = 0 );
};

/**Base class for polygon renderers generating texture images*/
class CORE_EXPORT QgsImageFillSymbolLayer: public QgsFillSymbolLayerV2
{
  public:

    QgsImageFillSymbolLayer();
    virtual ~QgsImageFillSymbolLayer();
    void renderPolygon( const QPolygonF& points, QList<QPolygonF>* rings, QgsSymbolV2RenderContext& context );

    virtual QgsSymbolV2* subSymbol() { return mOutline; }
    virtual bool setSubSymbol( QgsSymbolV2* symbol );

    void setOutlineWidthUnit( QgsSymbolV2::OutputUnit unit ) { mOutlineWidthUnit = unit; }
    QgsSymbolV2::OutputUnit outlineWidthUnit() const { return mOutlineWidthUnit; }

    void setOutlineWidthMapUnitScale( const QgsMapUnitScale& scale ) { mOutlineWidthMapUnitScale = scale; }
    const QgsMapUnitScale& outlineWidthMapUnitScale() const { return mOutlineWidthMapUnitScale; }

    void setOutputUnit( QgsSymbolV2::OutputUnit unit );
    QgsSymbolV2::OutputUnit outputUnit() const;

    void setMapUnitScale( const QgsMapUnitScale& scale );
    QgsMapUnitScale mapUnitScale() const;

    virtual double estimateMaxBleed() const;

    virtual double dxfWidth( const QgsDxfExport& e, const QgsSymbolV2RenderContext& context ) const;
    virtual QColor dxfColor( const QgsSymbolV2RenderContext& context ) const;
    virtual Qt::PenStyle dxfPenStyle() const;

  protected:
    QBrush mBrush;
    double mNextAngle; // mAngle / data defined angle

    /**Outline width*/
    double mOutlineWidth;
    QgsSymbolV2::OutputUnit mOutlineWidthUnit;
    QgsMapUnitScale mOutlineWidthMapUnitScale;

    /**Custom outline*/
    QgsLineSymbolV2* mOutline;

    virtual void applyDataDefinedSettings( const QgsSymbolV2RenderContext& context ) { Q_UNUSED( context ); }
};

/** \ingroup core
 * \class QgsRasterFillSymbolLayer
 * \brief A class for filling symbols with a repeated raster image.
 * \note Added in version 2.7
 */
class CORE_EXPORT QgsRasterFillSymbolLayer: public QgsImageFillSymbolLayer
{
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
    QgsSymbolLayerV2* clone() const;
    virtual double estimateMaxBleed() const;

    //override QgsImageFillSymbolLayer's support for sub symbols
    virtual QgsSymbolV2* subSymbol() { return 0; }
    virtual bool setSubSymbol( QgsSymbolV2* symbol );

    /**Sets the path to the raster image used for the fill.
     * @param imagePath path to image file
     * @see imageFilePath
    */
    void setImageFilePath( const QString& imagePath );
    /**The path to the raster image used for the fill.
     * @returns path to image file
     * @see setImageFilePath
    */
    QString imageFilePath() const { return mImageFilePath; }

    /**Set the coordinate mode for fill. Controls how the top left corner of the image
     * fill is positioned relative to the feature.
     * @param mode coordinate mode
     * @see coordinateMode
    */
    void setCoordinateMode( const FillCoordinateMode mode );
    /**Coordinate mode for fill. Controls how the top left corner of the image
     * fill is positioned relative to the feature.
     * @returns coordinate mode
     * @see setCoordinateMode
    */
    FillCoordinateMode coordinateMode() const { return mCoordinateMode; }

    /**Sets the opacity for the raster image used in the fill.
     * @param alpha opacity value between 0 (fully transparent) and 1 (fully opaque)
     * @see alpha
    */
    void setAlpha( const double alpha );
    /**The opacity for the raster image used in the fill.
     * @returns opacity value between 0 (fully transparent) and 1 (fully opaque)
     * @see setAlpha
    */
    double alpha() const { return mAlpha; }

    /**Sets the offset for the fill.
     * @param offset offset for fill
     * @see offset
     * @see setOffsetUnit
     * @see setOffsetMapUnitScale
    */
    void setOffset( const QPointF& offset ) { mOffset = offset; }
    /**Returns the offset for the fill.
     * @returns offset for fill
     * @see setOffset
     * @see offsetUnit
     * @see offsetMapUnitScale
    */
    QPointF offset() const { return mOffset; }

    /**Sets the units for the fill's offset.
     * @param unit units for offset
     * @see offsetUnit
     * @see setOffset
     * @see setOffsetMapUnitScale
    */
    void setOffsetUnit( const QgsSymbolV2::OutputUnit unit ) { mOffsetUnit = unit; }
    /**Returns the units for the fill's offset.
     * @returns units for offset
     * @see setOffsetUnit
     * @see offset
     * @see offsetMapUnitScale
    */
    QgsSymbolV2::OutputUnit offsetUnit() const { return mOffsetUnit; }

    /**Sets the map unit scale for the fill's offset.
     * @param scale map unit scale for offset
     * @see offsetMapUnitScale
     * @see setOffset
     * @see setOffsetUnit
    */
    void setOffsetMapUnitScale( const QgsMapUnitScale& scale ) { mOffsetMapUnitScale = scale; }
    /**Returns the map unit scale for the fill's offset.
     * @returns map unit scale for offset
     * @see setOffsetMapUnitScale
     * @see offset
     * @see offsetUnit
    */
    const QgsMapUnitScale& offsetMapUnitScale() const { return mOffsetMapUnitScale; }

    /**Sets the width for scaling the image used in the fill. The image's height will also be
     * scaled to maintain the image's aspect ratio.
     * @param width width for scaling the image
     * @see width
     * @see setWidthUnit
     * @see setWidthMapUnitScale
    */
    void setWidth( const double width ) { mWidth = width; }
    /**Returns the width used for scaling the image used in the fill. The image's height is
     * scaled to maintain the image's aspect ratio.
     * @returns width used for scaling the image
     * @see setWidth
     * @see widthUnit
     * @see widthMapUnitScale
    */
    double width() const { return mWidth; }

    /**Sets the units for the image's width.
     * @param unit units for width
     * @see widthUnit
     * @see setWidth
     * @see setWidthMapUnitScale
    */
    void setWidthUnit( const QgsSymbolV2::OutputUnit unit ) { mWidthUnit = unit; }
    /**Returns the units for the image's width.
     * @returns units for width
     * @see setWidthUnit
     * @see width
     * @see widthMapUnitScale
    */
    QgsSymbolV2::OutputUnit widthUnit() const { return mWidthUnit; }

    /**Sets the map unit scale for the image's width.
     * @param scale map unit scale for width
     * @see widthMapUnitScale
     * @see setWidth
     * @see setWidthUnit
    */
    void setWidthMapUnitScale( const QgsMapUnitScale& scale ) { mWidthMapUnitScale = scale; }
    /**Returns the map unit scale for the image's width.
     * @returns map unit scale for width
     * @see setWidthMapUnitScale
     * @see width
     * @see widthUnit
    */
    const QgsMapUnitScale& widthMapUnitScale() const { return mWidthMapUnitScale; }

  protected:

    /**Path to the image file*/
    QString mImageFilePath;
    FillCoordinateMode mCoordinateMode;
    double mAlpha;

    QPointF mOffset;
    QgsSymbolV2::OutputUnit mOffsetUnit;
    QgsMapUnitScale mOffsetMapUnitScale;

    double mWidth;
    QgsSymbolV2::OutputUnit mWidthUnit;
    QgsMapUnitScale mWidthMapUnitScale;

    void applyDataDefinedSettings( const QgsSymbolV2RenderContext& context );

  private:

    /**Applies the image pattern to the brush*/
    void applyPattern( QBrush& brush, const QString& imageFilePath, const double width, const double alpha,
                       const QgsSymbolV2RenderContext& context );
};

/**A class for svg fill patterns. The class automatically scales the pattern to
   the appropriate pixel dimensions of the output device*/
class CORE_EXPORT QgsSVGFillSymbolLayer: public QgsImageFillSymbolLayer
{
  public:
    QgsSVGFillSymbolLayer( const QString& svgFilePath = "", double width = 20, double rotation = 0.0 );
    QgsSVGFillSymbolLayer( const QByteArray& svgData, double width = 20, double rotation = 0.0 );
    ~QgsSVGFillSymbolLayer();

    static QgsSymbolLayerV2* create( const QgsStringMap& properties = QgsStringMap() );
    static QgsSymbolLayerV2* createFromSld( QDomElement &element );

    // implemented from base classes

    QString layerType() const;

    void startRender( QgsSymbolV2RenderContext& context );
    void stopRender( QgsSymbolV2RenderContext& context );

    QgsStringMap properties() const;

    QgsSymbolLayerV2* clone() const;

    void toSld( QDomDocument &doc, QDomElement &element, QgsStringMap props ) const;

    //getters and setters
    void setSvgFilePath( const QString& svgPath );
    QString svgFilePath() const { return mSvgFilePath; }
    void setPatternWidth( double width ) { mPatternWidth = width;}
    double patternWidth() const { return mPatternWidth; }

    void setSvgFillColor( const QColor& c ) { mSvgFillColor = c; }
    QColor svgFillColor() const { return mSvgFillColor; }
    void setSvgOutlineColor( const QColor& c ) { mSvgOutlineColor = c; }
    QColor svgOutlineColor() const { return mSvgOutlineColor; }
    void setSvgOutlineWidth( double w ) { mSvgOutlineWidth = w; }
    double svgOutlineWidth() const { return mSvgOutlineWidth; }

    void setPatternWidthUnit( QgsSymbolV2::OutputUnit unit ) { mPatternWidthUnit = unit; }
    QgsSymbolV2::OutputUnit patternWidthUnit() const { return mPatternWidthUnit; }

    void setPatternWidthMapUnitScale( const QgsMapUnitScale& scale ) { mPatternWidthMapUnitScale = scale; }
    const QgsMapUnitScale& patternWidthMapUnitScale() const { return mPatternWidthMapUnitScale; }

    void setSvgOutlineWidthUnit( QgsSymbolV2::OutputUnit unit ) { mSvgOutlineWidthUnit = unit; }
    QgsSymbolV2::OutputUnit svgOutlineWidthUnit() const { return mSvgOutlineWidthUnit; }

    void setSvgOutlineWidthMapUnitScale( const QgsMapUnitScale& scale ) { mSvgOutlineWidthMapUnitScale = scale; }
    const QgsMapUnitScale& svgOutlineWidthMapUnitScale() const { return mSvgOutlineWidthMapUnitScale; }

    void setOutputUnit( QgsSymbolV2::OutputUnit unit );
    QgsSymbolV2::OutputUnit outputUnit() const;

    void setMapUnitScale( const QgsMapUnitScale &scale );
    QgsMapUnitScale mapUnitScale() const;

  protected:
    /**Width of the pattern (in output units)*/
    double mPatternWidth;
    QgsSymbolV2::OutputUnit mPatternWidthUnit;
    QgsMapUnitScale mPatternWidthMapUnitScale;

    /**SVG data*/
    QByteArray mSvgData;
    /**Path to the svg file (or empty if constructed directly from data)*/
    QString mSvgFilePath;
    /**SVG view box (to keep the aspect ratio */
    QRectF mSvgViewBox;
    /** SVG pattern image */
    QImage* mSvgPattern;

    //param(fill), param(outline), param(outline-width) are going
    //to be replaced in memory
    QColor mSvgFillColor;
    QColor mSvgOutlineColor;
    double mSvgOutlineWidth;
    QgsSymbolV2::OutputUnit mSvgOutlineWidthUnit;
    QgsMapUnitScale mSvgOutlineWidthMapUnitScale;

    void applyDataDefinedSettings( const QgsSymbolV2RenderContext& context );

  private:
    /**Helper function that gets the view box from the byte array*/
    void storeViewBox();
    void setDefaultSvgParams(); //fills mSvgFillColor, mSvgOutlineColor, mSvgOutlineWidth with default values for mSvgFilePath

    /**Applies the svg pattern to the brush*/
    void applyPattern( QBrush& brush, const QString& svgFilePath, double patternWidth, QgsSymbolV2::OutputUnit patternWidthUnit, const QColor& svgFillColor, const QColor& svgOutlineColor,
                       double svgOutlineWidth, QgsSymbolV2::OutputUnit svgOutlineWidthUnit, const QgsSymbolV2RenderContext& context, const QgsMapUnitScale& patternWidthMapUnitScale, const QgsMapUnitScale &svgOutlineWidthMapUnitScale );
};

class CORE_EXPORT QgsLinePatternFillSymbolLayer: public QgsImageFillSymbolLayer
{
  public:
    QgsLinePatternFillSymbolLayer();
    ~QgsLinePatternFillSymbolLayer();

    static QgsSymbolLayerV2* create( const QgsStringMap& properties = QgsStringMap() );
    static QgsSymbolLayerV2* createFromSld( QDomElement &element );

    QString layerType() const;

    void startRender( QgsSymbolV2RenderContext& context );

    void stopRender( QgsSymbolV2RenderContext& context );

    QgsStringMap properties() const;

    QgsSymbolLayerV2* clone() const;

    void toSld( QDomDocument &doc, QDomElement &element, QgsStringMap props ) const;

    double estimateMaxBleed() const;

    QString ogrFeatureStyleWidth( double widthScaleFactor ) const;

    //getters and setters
    void setLineAngle( double a ) { mLineAngle = a; }
    double lineAngle() const { return mLineAngle; }
    void setDistance( double d ) { mDistance = d; }
    double distance() const { return mDistance; }
    void setLineWidth( double w );
    double lineWidth() const { return mLineWidth; }
    void setColor( const QColor& c );
    QColor color() const { return mColor; }
    void setOffset( double offset ) { mOffset = offset; }
    double offset() const { return mOffset; }

    void setDistanceUnit( QgsSymbolV2::OutputUnit unit ) { mDistanceUnit = unit; }
    QgsSymbolV2::OutputUnit distanceUnit() const { return mDistanceUnit; }

    void setDistanceMapUnitScale( const QgsMapUnitScale& scale ) { mDistanceMapUnitScale = scale; }
    const QgsMapUnitScale& distanceMapUnitScale() const { return mDistanceMapUnitScale; }

    void setLineWidthUnit( QgsSymbolV2::OutputUnit unit ) { mLineWidthUnit = unit; }
    QgsSymbolV2::OutputUnit lineWidthUnit() const { return mLineWidthUnit; }

    void setLineWidthMapUnitScale( const QgsMapUnitScale& scale ) { mLineWidthMapUnitScale = scale; }
    const QgsMapUnitScale& lineWidthMapUnitScale() const { return mLineWidthMapUnitScale; }

    void setOffsetUnit( QgsSymbolV2::OutputUnit unit ) { mOffsetUnit = unit; }
    QgsSymbolV2::OutputUnit offsetUnit() const { return mOffsetUnit; }

    void setOffsetMapUnitScale( const QgsMapUnitScale& scale ) { mOffsetMapUnitScale = scale; }
    const QgsMapUnitScale& offsetMapUnitScale() const { return mOffsetMapUnitScale; }

    void setOutputUnit( QgsSymbolV2::OutputUnit unit );
    QgsSymbolV2::OutputUnit outputUnit() const;

    void setMapUnitScale( const QgsMapUnitScale& scale );
    QgsMapUnitScale mapUnitScale() const;

    bool setSubSymbol( QgsSymbolV2* symbol );
    QgsSymbolV2* subSymbol();

  protected:
    /**Distance (in mm or map units) between lines*/
    double mDistance;
    QgsSymbolV2::OutputUnit mDistanceUnit;
    QgsMapUnitScale mDistanceMapUnitScale;
    /**Line width (in mm or map units)*/
    double mLineWidth;
    QgsSymbolV2::OutputUnit mLineWidthUnit;
    QgsMapUnitScale mLineWidthMapUnitScale;
    QColor mColor;
    /**Vector line angle in degrees (0 = horizontal, counterclockwise)*/
    double mLineAngle;
    /**Offset perpendicular to line direction*/
    double mOffset;
    QgsSymbolV2::OutputUnit mOffsetUnit;
    QgsMapUnitScale mOffsetMapUnitScale;

    void applyDataDefinedSettings( const QgsSymbolV2RenderContext& context );

  private:
    /**Applies the svg pattern to the brush*/
    void applyPattern( const QgsSymbolV2RenderContext& context, QBrush& brush, double lineAngle, double distance, double lineWidth, const QColor& color );

    /**Fill line*/
    QgsLineSymbolV2* mFillLineSymbol;
};

class CORE_EXPORT QgsPointPatternFillSymbolLayer: public QgsImageFillSymbolLayer
{
  public:
    QgsPointPatternFillSymbolLayer();
    ~QgsPointPatternFillSymbolLayer();

    static QgsSymbolLayerV2* create( const QgsStringMap& properties = QgsStringMap() );
    static QgsSymbolLayerV2* createFromSld( QDomElement &element );

    QString layerType() const;

    void startRender( QgsSymbolV2RenderContext& context );

    void stopRender( QgsSymbolV2RenderContext& context );

    QgsStringMap properties() const;

    QgsSymbolLayerV2* clone() const;

    void toSld( QDomDocument &doc, QDomElement &element, QgsStringMap props ) const;

    double estimateMaxBleed() const;

    //getters and setters
    double distanceX() const { return mDistanceX; }
    void setDistanceX( double d ) { mDistanceX = d; }

    double distanceY() const { return mDistanceY; }
    void setDistanceY( double d ) { mDistanceY = d; }

    double displacementX() const { return mDisplacementX; }
    void setDisplacementX( double d ) { mDisplacementX = d; }

    double displacementY() const { return mDisplacementY; }
    void setDisplacementY( double d ) { mDisplacementY = d; }

    bool setSubSymbol( QgsSymbolV2* symbol );
    virtual QgsSymbolV2* subSymbol() { return mMarkerSymbol; }

    void setDistanceXUnit( QgsSymbolV2::OutputUnit unit ) { mDistanceXUnit = unit; }
    QgsSymbolV2::OutputUnit distanceXUnit() const { return mDistanceXUnit; }

    void setDistanceXMapUnitScale( const QgsMapUnitScale& scale ) { mDistanceXMapUnitScale = scale; }
    const QgsMapUnitScale& distanceXMapUnitScale() const { return mDistanceXMapUnitScale; }

    void setDistanceYUnit( QgsSymbolV2::OutputUnit unit ) { mDistanceYUnit = unit; }
    QgsSymbolV2::OutputUnit distanceYUnit() const { return mDistanceYUnit; }

    void setDistanceYMapUnitScale( const QgsMapUnitScale& scale ) { mDistanceYMapUnitScale = scale; }
    const QgsMapUnitScale& distanceYMapUnitScale() const { return mDistanceYMapUnitScale; }

    void setDisplacementXUnit( QgsSymbolV2::OutputUnit unit ) { mDisplacementXUnit = unit; }
    QgsSymbolV2::OutputUnit displacementXUnit() const { return mDisplacementXUnit; }

    void setDisplacementXMapUnitScale( const QgsMapUnitScale& scale ) { mDisplacementXMapUnitScale = scale; }
    const QgsMapUnitScale& displacementXMapUnitScale() const { return mDisplacementXMapUnitScale; }

    void setDisplacementYUnit( QgsSymbolV2::OutputUnit unit ) { mDisplacementYUnit = unit; }
    QgsSymbolV2::OutputUnit displacementYUnit() const { return mDisplacementYUnit; }

    void setDisplacementYMapUnitScale( const QgsMapUnitScale& scale ) { mDisplacementYMapUnitScale = scale; }
    const QgsMapUnitScale& displacementYMapUnitScale() const { return mDisplacementYMapUnitScale; }

    void setOutputUnit( QgsSymbolV2::OutputUnit unit );
    QgsSymbolV2::OutputUnit outputUnit() const;

    void setMapUnitScale( const QgsMapUnitScale &scale );
    QgsMapUnitScale mapUnitScale() const;

    virtual QSet<QString> usedAttributes() const;

  protected:
    QgsMarkerSymbolV2* mMarkerSymbol;
    double mDistanceX;
    QgsSymbolV2::OutputUnit mDistanceXUnit;
    QgsMapUnitScale mDistanceXMapUnitScale;
    double mDistanceY;
    QgsSymbolV2::OutputUnit mDistanceYUnit;
    QgsMapUnitScale mDistanceYMapUnitScale;
    double mDisplacementX;
    QgsSymbolV2::OutputUnit mDisplacementXUnit;
    QgsMapUnitScale mDisplacementXMapUnitScale;
    double mDisplacementY;
    QgsSymbolV2::OutputUnit mDisplacementYUnit;
    QgsMapUnitScale mDisplacementYMapUnitScale;

    void applyDataDefinedSettings( const QgsSymbolV2RenderContext& context );

  private:
    void applyPattern( const QgsSymbolV2RenderContext& context, QBrush& brush, double distanceX, double distanceY,
                       double displacementX, double displacementY );
};

class CORE_EXPORT QgsCentroidFillSymbolLayerV2 : public QgsFillSymbolLayerV2
{
  public:
    QgsCentroidFillSymbolLayerV2();
    ~QgsCentroidFillSymbolLayerV2();

    // static stuff

    static QgsSymbolLayerV2* create( const QgsStringMap& properties = QgsStringMap() );
    static QgsSymbolLayerV2* createFromSld( QDomElement &element );

    // implemented from base classes

    QString layerType() const;

    void startRender( QgsSymbolV2RenderContext& context );

    void stopRender( QgsSymbolV2RenderContext& context );

    void renderPolygon( const QPolygonF& points, QList<QPolygonF>* rings, QgsSymbolV2RenderContext& context );

    QgsStringMap properties() const;

    QgsSymbolLayerV2* clone() const;

    void toSld( QDomDocument &doc, QDomElement &element, QgsStringMap props ) const;

    void setColor( const QColor& color );

    QgsSymbolV2* subSymbol();
    bool setSubSymbol( QgsSymbolV2* symbol );

    void setOutputUnit( QgsSymbolV2::OutputUnit unit );
    QgsSymbolV2::OutputUnit outputUnit() const;

    void setMapUnitScale( const QgsMapUnitScale &scale );
    QgsMapUnitScale mapUnitScale() const;

    virtual QSet<QString> usedAttributes() const;

    void setPointOnSurface( bool pointOnSurface ) { mPointOnSurface = pointOnSurface; }
    bool pointOnSurface() const { return mPointOnSurface; }

  protected:
    QgsMarkerSymbolV2* mMarker;
    bool mPointOnSurface;
};

#endif


