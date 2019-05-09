/***************************************************************************
 qgsfillsymbollayer.h
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

#ifndef QGSFILLSYMBOLLAYER_H
#define QGSFILLSYMBOLLAYER_H

#include "qgis_core.h"
#include "qgis.h"
#include "qgssymbollayer.h"

#define DEFAULT_SIMPLEFILL_COLOR        QColor(0,0,255)
#define DEFAULT_SIMPLEFILL_STYLE        Qt::SolidPattern
#define DEFAULT_SIMPLEFILL_BORDERCOLOR  QColor( 35, 35, 35 )
#define DEFAULT_SIMPLEFILL_BORDERSTYLE  Qt::SolidLine
#define DEFAULT_SIMPLEFILL_BORDERWIDTH  DEFAULT_LINE_WIDTH
#define DEFAULT_SIMPLEFILL_JOINSTYLE    Qt::BevelJoin

#define INF 1E20

#include <QPen>
#include <QBrush>

/**
 * \ingroup core
 * \class QgsSimpleFillSymbolLayer
 */
class CORE_EXPORT QgsSimpleFillSymbolLayer : public QgsFillSymbolLayer
{
  public:
    QgsSimpleFillSymbolLayer( const QColor &color = DEFAULT_SIMPLEFILL_COLOR,
                              Qt::BrushStyle style = DEFAULT_SIMPLEFILL_STYLE,
                              const QColor &strokeColor = DEFAULT_SIMPLEFILL_BORDERCOLOR,
                              Qt::PenStyle strokeStyle = DEFAULT_SIMPLEFILL_BORDERSTYLE,
                              double strokeWidth = DEFAULT_SIMPLEFILL_BORDERWIDTH,
                              Qt::PenJoinStyle penJoinStyle = DEFAULT_SIMPLEFILL_JOINSTYLE
                            );

    // static stuff

    static QgsSymbolLayer *create( const QgsStringMap &properties = QgsStringMap() ) SIP_FACTORY;
    static QgsSymbolLayer *createFromSld( QDomElement &element ) SIP_FACTORY;

    // implemented from base classes

    QString layerType() const override;

    void startRender( QgsSymbolRenderContext &context ) override;

    void stopRender( QgsSymbolRenderContext &context ) override;

    void renderPolygon( const QPolygonF &points, QList<QPolygonF> *rings, QgsSymbolRenderContext &context ) override;

    QgsStringMap properties() const override;

    QgsSimpleFillSymbolLayer *clone() const override SIP_FACTORY;

    void toSld( QDomDocument &doc, QDomElement &element, const QgsStringMap &props ) const override;

    QString ogrFeatureStyle( double mmScaleFactor, double mapUnitScaleFactor ) const override;

    Qt::BrushStyle brushStyle() const { return mBrushStyle; }
    void setBrushStyle( Qt::BrushStyle style ) { mBrushStyle = style; }

    QColor strokeColor() const override { return mStrokeColor; }
    void setStrokeColor( const QColor &strokeColor ) override { mStrokeColor = strokeColor; }

    QColor fillColor() const override { return color(); }
    void setFillColor( const QColor &color ) override { setColor( color ); }

    Qt::PenStyle strokeStyle() const { return mStrokeStyle; }
    void setStrokeStyle( Qt::PenStyle strokeStyle ) { mStrokeStyle = strokeStyle; }

    double strokeWidth() const { return mStrokeWidth; }
    void setStrokeWidth( double strokeWidth ) { mStrokeWidth = strokeWidth; }

    Qt::PenJoinStyle penJoinStyle() const { return mPenJoinStyle; }
    void setPenJoinStyle( Qt::PenJoinStyle style ) { mPenJoinStyle = style; }

    void setOffset( QPointF offset ) { mOffset = offset; }
    QPointF offset() { return mOffset; }

    /**
     * Sets the units for the width of the fill's stroke.
     * \param unit width units
     * \see strokeWidthUnit()
    */
    void setStrokeWidthUnit( QgsUnitTypes::RenderUnit unit ) { mStrokeWidthUnit = unit; }

    /**
     * Returns the units for the width of the fill's stroke.
     * \see setStrokeWidthUnit()
    */
    QgsUnitTypes::RenderUnit strokeWidthUnit() const { return mStrokeWidthUnit; }

    void setStrokeWidthMapUnitScale( const QgsMapUnitScale &scale ) { mStrokeWidthMapUnitScale = scale; }
    const QgsMapUnitScale &strokeWidthMapUnitScale() const { return mStrokeWidthMapUnitScale; }

    /**
     * Sets the units for the fill's offset.
     * \param unit offset units
     * \see offsetUnit()
    */
    void setOffsetUnit( QgsUnitTypes::RenderUnit unit ) { mOffsetUnit = unit; }

    /**
     * Returns the units for the fill's offset.
     * \see setOffsetUnit()
    */
    QgsUnitTypes::RenderUnit offsetUnit() const { return mOffsetUnit; }

    void setOffsetMapUnitScale( const QgsMapUnitScale &scale ) { mOffsetMapUnitScale = scale; }
    const QgsMapUnitScale &offsetMapUnitScale() const { return mOffsetMapUnitScale; }

    void setOutputUnit( QgsUnitTypes::RenderUnit unit ) override;
    QgsUnitTypes::RenderUnit outputUnit() const override;

    void setMapUnitScale( const QgsMapUnitScale &scale ) override;
    QgsMapUnitScale mapUnitScale() const override;

    double estimateMaxBleed( const QgsRenderContext &context ) const override;

    double dxfWidth( const QgsDxfExport &e, QgsSymbolRenderContext &context ) const override;
    QColor dxfColor( QgsSymbolRenderContext &context ) const override;
    double dxfAngle( QgsSymbolRenderContext &context ) const override;

    Qt::PenStyle dxfPenStyle() const override;
    QColor dxfBrushColor( QgsSymbolRenderContext &context ) const override;
    Qt::BrushStyle dxfBrushStyle() const override;

  protected:
    QBrush mBrush;
    QBrush mSelBrush;
    Qt::BrushStyle mBrushStyle;
    QColor mStrokeColor;
    Qt::PenStyle mStrokeStyle;
    double mStrokeWidth;
    QgsUnitTypes::RenderUnit mStrokeWidthUnit = QgsUnitTypes::RenderMillimeters;
    QgsMapUnitScale mStrokeWidthMapUnitScale;
    Qt::PenJoinStyle mPenJoinStyle;
    QPen mPen;
    QPen mSelPen;

    QPointF mOffset;
    QgsUnitTypes::RenderUnit mOffsetUnit = QgsUnitTypes::RenderMillimeters;
    QgsMapUnitScale mOffsetMapUnitScale;

  private:
    //helper functions for data defined symbology
    void applyDataDefinedSymbology( QgsSymbolRenderContext &context, QBrush &brush, QPen &pen, QPen &selPen );
};

class QgsColorRamp;

/**
 * \ingroup core
 * \class QgsGradientFillSymbolLayer
 */
class CORE_EXPORT QgsGradientFillSymbolLayer : public QgsFillSymbolLayer
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

    QgsGradientFillSymbolLayer( const QColor &color = DEFAULT_SIMPLEFILL_COLOR,
                                const QColor &color2 = Qt::white,
                                GradientColorType gradientColorType = SimpleTwoColor,
                                GradientType gradientType = Linear,
                                GradientCoordinateMode coordinateMode = Feature,
                                GradientSpread gradientSpread = Pad
                              );

    ~QgsGradientFillSymbolLayer() override;

    // static stuff

    static QgsSymbolLayer *create( const QgsStringMap &properties = QgsStringMap() ) SIP_FACTORY;

    // implemented from base classes

    QString layerType() const override;

    void startRender( QgsSymbolRenderContext &context ) override;

    void stopRender( QgsSymbolRenderContext &context ) override;

    void renderPolygon( const QPolygonF &points, QList<QPolygonF> *rings, QgsSymbolRenderContext &context ) override;

    QgsStringMap properties() const override;

    QgsGradientFillSymbolLayer *clone() const override SIP_FACTORY;

    double estimateMaxBleed( const QgsRenderContext &context ) const override;

    //! Type of gradient, e.g., linear or radial
    GradientType gradientType() const { return mGradientType; }
    void setGradientType( GradientType gradientType ) { mGradientType = gradientType; }

    //! Gradient color mode, controls how gradient color stops are created
    GradientColorType gradientColorType() const { return mGradientColorType; }
    void setGradientColorType( GradientColorType gradientColorType ) { mGradientColorType = gradientColorType; }

    /**
     * Returns the color ramp used for the gradient fill. This is only
     * used if the gradient color type is set to ColorRamp.
     * \see setColorRamp()
     * \see gradientColorType()
     */
    QgsColorRamp *colorRamp() { return mGradientRamp; }

    /**
     * Sets the color ramp used for the gradient fill. This is only
     * used if the gradient color type is set to ColorRamp.
     * \param ramp color ramp. Ownership is transferred.
     * \see colorRamp()
     * \see setGradientColorType()
     */
    void setColorRamp( QgsColorRamp *ramp SIP_TRANSFER );

    //! Color for endpoint of gradient, only used if the gradient color type is set to SimpleTwoColor
    QColor color2() const { return mColor2; }
    void setColor2( const QColor &color2 ) { mColor2 = color2; }

    //! Coordinate mode for gradient. Controls how the gradient stops are positioned.
    GradientCoordinateMode coordinateMode() const { return mCoordinateMode; }
    void setCoordinateMode( GradientCoordinateMode coordinateMode ) { mCoordinateMode = coordinateMode; }

    //! Gradient spread mode. Controls how the gradient behaves outside of the predefined stops
    GradientSpread gradientSpread() const { return mGradientSpread; }
    void setGradientSpread( GradientSpread gradientSpread ) { mGradientSpread = gradientSpread; }

    //! Starting point of gradient fill, in the range [0,0] - [1,1]
    void setReferencePoint1( QPointF referencePoint ) { mReferencePoint1 = referencePoint; }
    QPointF referencePoint1() const { return mReferencePoint1; }

    //! Sets the starting point of the gradient to be the feature centroid
    void setReferencePoint1IsCentroid( bool isCentroid ) { mReferencePoint1IsCentroid = isCentroid; }
    bool referencePoint1IsCentroid() const { return mReferencePoint1IsCentroid; }

    //! End point of gradient fill, in the range [0,0] - [1,1]
    void setReferencePoint2( QPointF referencePoint ) { mReferencePoint2 = referencePoint; }
    QPointF referencePoint2() const { return mReferencePoint2; }

    //! Sets the end point of the gradient to be the feature centroid
    void setReferencePoint2IsCentroid( bool isCentroid ) { mReferencePoint2IsCentroid = isCentroid; }
    bool referencePoint2IsCentroid() const { return mReferencePoint2IsCentroid; }

    //! Offset for gradient fill
    void setOffset( QPointF offset ) { mOffset = offset; }
    QPointF offset() const { return mOffset; }

    //! Units for gradient fill offset
    void setOffsetUnit( QgsUnitTypes::RenderUnit unit ) { mOffsetUnit = unit; }
    QgsUnitTypes::RenderUnit offsetUnit() const { return mOffsetUnit; }

    void setOffsetMapUnitScale( const QgsMapUnitScale &scale ) { mOffsetMapUnitScale = scale; }
    const QgsMapUnitScale &offsetMapUnitScale() const { return mOffsetMapUnitScale; }

    void setOutputUnit( QgsUnitTypes::RenderUnit unit ) override;
    QgsUnitTypes::RenderUnit outputUnit() const override;

    void setMapUnitScale( const QgsMapUnitScale &scale ) override;
    QgsMapUnitScale mapUnitScale() const override;

  protected:
    QBrush mBrush;
    QBrush mSelBrush;

    GradientColorType mGradientColorType;
    QColor mColor2;
    QgsColorRamp *mGradientRamp = nullptr;
    GradientType mGradientType;
    GradientCoordinateMode mCoordinateMode;
    GradientSpread mGradientSpread;

    QPointF mReferencePoint1;
    bool mReferencePoint1IsCentroid = false;
    QPointF mReferencePoint2;
    bool mReferencePoint2IsCentroid = false;

    QPointF mOffset;
    QgsUnitTypes::RenderUnit mOffsetUnit = QgsUnitTypes::RenderMillimeters;
    QgsMapUnitScale mOffsetMapUnitScale;

  private:

    //helper functions for data defined symbology
    void applyDataDefinedSymbology( QgsSymbolRenderContext &context, const QPolygonF &points );

    //! Applies the gradient to a brush
    void applyGradient( const QgsSymbolRenderContext &context, QBrush &brush, const QColor &color, const QColor &color2,
                        GradientColorType gradientColorType, QgsColorRamp *gradientRamp, GradientType gradientType,
                        GradientCoordinateMode coordinateMode, GradientSpread gradientSpread,
                        QPointF referencePoint1, QPointF referencePoint2, double angle );

    //! Rotates a reference point by a specified angle around the point (0.5, 0.5)
    QPointF rotateReferencePoint( QPointF refPoint, double angle );
};

/**
 * \ingroup core
 * \class QgsShapeburstFillSymbolLayer
 */
class CORE_EXPORT QgsShapeburstFillSymbolLayer : public QgsFillSymbolLayer
{
  public:

    enum ShapeburstColorType
    {
      SimpleTwoColor,
      ColorRamp
    };

    QgsShapeburstFillSymbolLayer( const QColor &color = DEFAULT_SIMPLEFILL_COLOR, const QColor &color2 = Qt::white,
                                  ShapeburstColorType colorType = SimpleTwoColor,
                                  int blurRadius = 0, bool useWholeShape = true, double maxDistance = 5 );

    ~QgsShapeburstFillSymbolLayer() override;

    /**
     * QgsShapeburstFillSymbolLayer cannot be copied.
     * \see clone()
     */
    QgsShapeburstFillSymbolLayer( const QgsShapeburstFillSymbolLayer &other ) = delete;

    /**
     * QgsShapeburstFillSymbolLayer cannot be copied.
     * \see clone()
     */
    QgsShapeburstFillSymbolLayer &operator=( const QgsShapeburstFillSymbolLayer &other ) = delete;

    // static stuff

    static QgsSymbolLayer *create( const QgsStringMap &properties = QgsStringMap() ) SIP_FACTORY;

    // implemented from base classes

    QString layerType() const override;

    void startRender( QgsSymbolRenderContext &context ) override;

    void stopRender( QgsSymbolRenderContext &context ) override;

    void renderPolygon( const QPolygonF &points, QList<QPolygonF> *rings, QgsSymbolRenderContext &context ) override;

    QgsStringMap properties() const override;

    QgsShapeburstFillSymbolLayer *clone() const override SIP_FACTORY;

    double estimateMaxBleed( const QgsRenderContext &context ) const override;

    /**
     * Sets the blur radius, which controls the amount of blurring applied to the fill.
     * \param blurRadius Radius for fill blur. Values between 0 - 17 are valid, where higher values results in a stronger blur. Set to 0 to disable blur.
     * \see blurRadius
     * \since QGIS 2.3
     */
    void setBlurRadius( int blurRadius ) { mBlurRadius = blurRadius; }

    /**
     * Returns the blur radius, which controls the amount of blurring applied to the fill.
     * \returns Integer representing the radius for fill blur. Higher values indicate a stronger blur. A 0 value indicates that blurring is disabled.
     * \see setBlurRadius
     * \since QGIS 2.3
     */
    int blurRadius() const { return mBlurRadius; }

    /**
     * Sets whether the shapeburst fill should be drawn using the entire shape.
     * \param useWholeShape Set to TRUE if shapeburst should cover entire shape. If FALSE, setMaxDistance is used to calculate how far from the boundary of the shape should
     * be shaded
     * \see useWholeShape
     * \see setMaxDistance
     * \since QGIS 2.3
     */
    void setUseWholeShape( bool useWholeShape ) { mUseWholeShape = useWholeShape; }

    /**
     * Returns whether the shapeburst fill is set to cover the entire shape.
     * \returns TRUE if shapeburst fill will cover the entire shape. If FALSE, shapeburst is drawn to a distance of maxDistance from the polygon's boundary.
     * \see setUseWholeShape
     * \see maxDistance
     * \since QGIS 2.3
     */
    bool useWholeShape() const { return mUseWholeShape; }

    /**
     * Sets the maximum distance to shape inside of the shape from the polygon's boundary.
     * \param maxDistance distance from boundary to shade. setUseWholeShape must be set to FALSE for this parameter to take effect. Distance unit is controlled by setDistanceUnit.
     * \see maxDistance
     * \see setUseWholeShape
     * \see setDistanceUnit
     * \since QGIS 2.3
     */
    void setMaxDistance( double maxDistance ) { mMaxDistance = maxDistance; }

    /**
     * Returns the maximum distance from the shape's boundary which is shaded. This parameter is only effective if useWholeShape is FALSE.
     * \returns the maximum distance from the polygon's boundary which is shaded. Distance units are indicated by distanceUnit.
     * \see useWholeShape
     * \see setMaxDistance
     * \see distanceUnit
     * \since QGIS 2.3
     */
    double maxDistance() const { return mMaxDistance; }

    /**
     * Sets the unit for the maximum distance to shade inside of the shape from the polygon's boundary.
     * \param unit distance unit for the maximum distance
     * \see setMaxDistance
     * \see distanceUnit
     * \since QGIS 2.3
     */
    void setDistanceUnit( QgsUnitTypes::RenderUnit unit ) { mDistanceUnit = unit; }

    /**
     * Returns the unit for the maximum distance to shade inside of the shape from the polygon's boundary.
     * \returns distance unit for the maximum distance
     * \see maxDistance
     * \see setDistanceUnit
     * \since QGIS 2.3
     */
    QgsUnitTypes::RenderUnit distanceUnit() const { return mDistanceUnit; }

    void setDistanceMapUnitScale( const QgsMapUnitScale &scale ) { mDistanceMapUnitScale = scale; }
    const QgsMapUnitScale &distanceMapUnitScale() const { return mDistanceMapUnitScale; }

    /**
     * Sets the color mode to use for the shapeburst fill. Shapeburst can either be drawn using a QgsColorRamp color ramp
     * or by simply specificing a start and end color. setColorType is used to specify which mode to use for the fill.
     * \param colorType color type to use for shapeburst fill
     * \see colorType
     * \see setColor
     * \see setColor2
     * \see setColorRamp
     * \since QGIS 2.3
     */
    void setColorType( ShapeburstColorType colorType ) { mColorType = colorType; }

    /**
     * Returns the color mode used for the shapeburst fill. Shapeburst can either be drawn using a QgsColorRamp color ramp
     * or by simply specificing a start and end color.
     * \returns current color mode used for the shapeburst fill
     * \see setColorType
     * \see color
     * \see color2
     * \see colorRamp
     * \since QGIS 2.3
     */
    ShapeburstColorType colorType() const { return mColorType; }

    /**
     * Sets the color ramp used to draw the shapeburst fill. Color ramps are only used if setColorType is set ShapeburstColorType::ColorRamp.
     * \param ramp color ramp to use for shapeburst fill
     * \see setColorType
     * \see colorRamp
     * \since QGIS 2.3
     */
    void setColorRamp( QgsColorRamp *ramp );

    /**
     * Returns the color ramp used for the shapeburst fill. The color ramp is only used if the colorType is set to ShapeburstColorType::ColorRamp
     * \returns a QgsColorRamp color ramp
     * \see setColorRamp
     * \see colorType
     * \since QGIS 2.3
     */
    QgsColorRamp *colorRamp() { return mGradientRamp.get(); }

    /**
     * Sets the color for the endpoint of the shapeburst fill. This color is only used if setColorType is set ShapeburstColorType::SimpleTwoColor.
     * \param color2 QColor to use for endpoint of gradient
     * \see setColorType
     * \see color2
     * \since QGIS 2.3
     */
    void setColor2( const QColor &color2 ) { mColor2 = color2; }

    /**
     * Returns the color used for the endpoint of the shapeburst fill. This color is only used if the colorType is set to ShapeburstColorType::SimpleTwoColor
     * \returns a QColor indicating the color of the endpoint of the gradient
     * \see setColor2
     * \see colorType
     * \since QGIS 2.3
     */
    QColor color2() const { return mColor2; }

    /**
     * Sets whether the shapeburst fill should ignore polygon rings when calculating
     * the buffered shading.
     * \param ignoreRings Set to TRUE if buffers should ignore interior rings for polygons.
     * \see ignoreRings
     * \since QGIS 2.3
     */
    void setIgnoreRings( bool ignoreRings ) { mIgnoreRings = ignoreRings; }

    /**
     * Returns whether the shapeburst fill is set to ignore polygon interior rings.
     * \returns TRUE if the shapeburst fill will ignore interior rings when calculating buffered shading.
     * \see setIgnoreRings
     * \since QGIS 2.3
     */
    bool ignoreRings() const { return mIgnoreRings; }

    /**
     * Sets the offset for the shapeburst fill.
     * \param offset QPointF indicating the horizontal/vertical offset amount
     * \see offset
     * \see setOffsetUnit
     * \since QGIS 2.3
     */
    void setOffset( QPointF offset ) { mOffset = offset; }

    /**
     * Returns the offset for the shapeburst fill.
     * \returns a QPointF indicating the horizontal/vertical offset amount
     * \see setOffset
     * \see offsetUnit
     * \since QGIS 2.3
     */
    QPointF offset() const { return mOffset; }

    /**
     * Sets the units used for the offset for the shapeburst fill.
     * \param unit units for fill offset
     * \see setOffset
     * \see offsetUnit
     * \since QGIS 2.3
     */
    void setOffsetUnit( QgsUnitTypes::RenderUnit unit ) { mOffsetUnit = unit; }

    /**
     * Returns the units used for the offset of the shapeburst fill.
     * \returns units used for the fill offset
     * \see offset
     * \see setOffsetUnit
     * \since QGIS 2.3
     */
    QgsUnitTypes::RenderUnit offsetUnit() const { return mOffsetUnit; }

    void setOffsetMapUnitScale( const QgsMapUnitScale &scale ) { mOffsetMapUnitScale = scale; }
    const QgsMapUnitScale &offsetMapUnitScale() const { return mOffsetMapUnitScale; }

    void setOutputUnit( QgsUnitTypes::RenderUnit unit ) override;
    QgsUnitTypes::RenderUnit outputUnit() const override;

    void setMapUnitScale( const QgsMapUnitScale &scale ) override;
    QgsMapUnitScale mapUnitScale() const override;

  private:
    QBrush mBrush;
    QBrush mSelBrush;

    int mBlurRadius = 0;

    bool mUseWholeShape = true;
    double mMaxDistance = 5.0;
    QgsUnitTypes::RenderUnit mDistanceUnit = QgsUnitTypes::RenderMillimeters;
    QgsMapUnitScale mDistanceMapUnitScale;

    ShapeburstColorType mColorType = SimpleTwoColor;
    QColor mColor2;

    bool mIgnoreRings = false;

    QPointF mOffset;
    QgsUnitTypes::RenderUnit mOffsetUnit = QgsUnitTypes::RenderMillimeters;
    QgsMapUnitScale mOffsetMapUnitScale;

    std::unique_ptr< QgsColorRamp > mGradientRamp;

    //helper functions for data defined symbology
    void applyDataDefinedSymbology( QgsSymbolRenderContext &context, QColor &color, QColor &color2, int &blurRadius, bool &useWholeShape,
                                    double &maxDistance, bool &ignoreRings );

    /* distance transform of a 1d function using squared distance */
    void distanceTransform1d( double *f, int n, int *v, double *z, double *d );
    /* distance transform of 2d function using squared distance */
    void distanceTransform2d( double *im, int width, int height );
    /* distance transform of a binary QImage */
    double *distanceTransform( QImage *im );

    /* fills a QImage with values from an array of doubles containing squared distance transform values */
    void dtArrayToQImage( double *array, QImage *im, QgsColorRamp *ramp, double layerAlpha = 1, bool useWholeShape = true, int maxPixelDistance = 0 );

#ifdef SIP_RUN
    QgsShapeburstFillSymbolLayer( const QgsShapeburstFillSymbolLayer &other );
#endif
};

/**
 * \ingroup core
 * Base class for polygon renderers generating texture images*/
class CORE_EXPORT QgsImageFillSymbolLayer: public QgsFillSymbolLayer
{
  public:

    QgsImageFillSymbolLayer();
    void renderPolygon( const QPolygonF &points, QList<QPolygonF> *rings, QgsSymbolRenderContext &context ) override;

    QgsSymbol *subSymbol() override { return mStroke.get(); }
    bool setSubSymbol( QgsSymbol *symbol SIP_TRANSFER ) override;

    /**
     * Sets the \a units fo the symbol's stroke width.
     * \see strokeWidthUnit()
     * \see setStrokeWidthMapUnitScale()
    */
    void setStrokeWidthUnit( QgsUnitTypes::RenderUnit unit ) { mStrokeWidthUnit = unit; }

    /**
     * Returns the units for the symbol's stroke width.
     * \see setStrokeWidthUnit()
     * \see strokeWidthMapUnitScale()
    */
    QgsUnitTypes::RenderUnit strokeWidthUnit() const { return mStrokeWidthUnit; }

    /**
     * Sets the stroke width map unit \a scale.
     *
     * \see strokeWidthMapUnitScale()
     * \see setStrokeWidthUnit()
     */
    void setStrokeWidthMapUnitScale( const QgsMapUnitScale &scale ) { mStrokeWidthMapUnitScale = scale; }

    /**
     * Returns the stroke width map unit scale.
     *
     * \see setStrokeWidthMapUnitScale()
     * \see strokeWidthUnit()
     *
     * \since QGIS 2.16
    */
    const QgsMapUnitScale &strokeWidthMapUnitScale() const { return mStrokeWidthMapUnitScale; }

    void setOutputUnit( QgsUnitTypes::RenderUnit unit ) override;
    QgsUnitTypes::RenderUnit outputUnit() const override;
    void setMapUnitScale( const QgsMapUnitScale &scale ) override;
    QgsMapUnitScale mapUnitScale() const override;
    double estimateMaxBleed( const QgsRenderContext &context ) const override;
    double dxfWidth( const QgsDxfExport &e, QgsSymbolRenderContext &context ) const override;
    QColor dxfColor( QgsSymbolRenderContext &context ) const override;
    Qt::PenStyle dxfPenStyle() const override;
    QSet<QString> usedAttributes( const QgsRenderContext &context ) const override;
    bool hasDataDefinedProperties() const override;

  protected:
    QBrush mBrush;
    double mNextAngle = 0.0; // mAngle / data defined angle

    //! Stroke width
    double mStrokeWidth = 0.0;
    QgsUnitTypes::RenderUnit mStrokeWidthUnit = QgsUnitTypes::RenderMillimeters;
    QgsMapUnitScale mStrokeWidthMapUnitScale;

    //! Custom stroke
    std::unique_ptr< QgsLineSymbol > mStroke;

    virtual void applyDataDefinedSettings( QgsSymbolRenderContext &context ) { Q_UNUSED( context ); }

  private:
#ifdef SIP_RUN
    QgsImageFillSymbolLayer( const QgsImageFillSymbolLayer &other );
#endif
};

/**
 * \ingroup core
 * \class QgsRasterFillSymbolLayer
 * \brief A class for filling symbols with a repeated raster image.
 * \since QGIS 2.7
 */
class CORE_EXPORT QgsRasterFillSymbolLayer: public QgsImageFillSymbolLayer
{
  public:

    //! Fill coordinate modes, dictates fill tiling behavior
    enum FillCoordinateMode
    {
      Feature, //!< Tiling is based on feature bounding box
      Viewport, //!< Tiling is based on complete map viewport
    };

    /**
     * Constructor for QgsRasterFillSymbolLayer, using a raster fill from the
     * specified \a imageFilePath.
     */
    QgsRasterFillSymbolLayer( const QString &imageFilePath = QString() );

    /**
     * Creates a new QgsRasterFillSymbolLayer from a \a properties map. The caller takes
     * ownership of the returned object.
     */
    static QgsSymbolLayer *create( const QgsStringMap &properties = QgsStringMap() ) SIP_FACTORY;

    /**
     * Turns relative paths in properties map to absolute when reading and vice versa when writing.
     * Used internally when reading/writing symbols.
     * \since QGIS 3.0
     */
    static void resolvePaths( QgsStringMap &properties, const QgsPathResolver &pathResolver, bool saving );

    // implemented from base classes
    QString layerType() const override;
    void renderPolygon( const QPolygonF &points, QList<QPolygonF> *rings, QgsSymbolRenderContext &context ) override;
    void startRender( QgsSymbolRenderContext &context ) override;
    void stopRender( QgsSymbolRenderContext &context ) override;
    QgsStringMap properties() const override;
    QgsRasterFillSymbolLayer *clone() const override SIP_FACTORY;
    double estimateMaxBleed( const QgsRenderContext &context ) const override;

    //override QgsImageFillSymbolLayer's support for sub symbols
    QgsSymbol *subSymbol() override { return nullptr; }
    bool setSubSymbol( QgsSymbol *symbol SIP_TRANSFER ) override;

    /**
     * Sets the path to the raster image used for the fill.
     * \param imagePath path to image file
     * \see imageFilePath
     */
    void setImageFilePath( const QString &imagePath );

    /**
     * The path to the raster image used for the fill.
     * \returns path to image file
     * \see setImageFilePath
     */
    QString imageFilePath() const { return mImageFilePath; }

    /**
     * Set the coordinate mode for fill. Controls how the top left corner of the image
     * fill is positioned relative to the feature.
     * \param mode coordinate mode
     * \see coordinateMode
     */
    void setCoordinateMode( FillCoordinateMode mode );

    /**
     * Coordinate mode for fill. Controls how the top left corner of the image
     * fill is positioned relative to the feature.
     * \returns coordinate mode
     * \see setCoordinateMode
     */
    FillCoordinateMode coordinateMode() const { return mCoordinateMode; }

    /**
     * Sets the \a opacity for the raster image used in the fill.
     * \param opacity opacity value between 0 (fully transparent) and 1 (fully opaque)
     * \see opacity()
     */
    void setOpacity( double opacity );

    /**
     * Returns the opacity for the raster image used in the fill.
     * \returns opacity value between 0 (fully transparent) and 1 (fully opaque)
     * \see setOpacity()
     */
    double opacity() const { return mOpacity; }

    /**
     * Sets the offset for the fill.
     * \param offset offset for fill
     * \see offset
     * \see setOffsetUnit
     * \see setOffsetMapUnitScale
     */
    void setOffset( QPointF offset ) { mOffset = offset; }

    /**
     * Returns the offset for the fill.
     * \returns offset for fill
     * \see setOffset
     * \see offsetUnit
     * \see offsetMapUnitScale
     */
    QPointF offset() const { return mOffset; }

    /**
     * Sets the units for the fill's offset.
     * \param unit units for offset
     * \see offsetUnit
     * \see setOffset
     * \see setOffsetMapUnitScale
     */
    void setOffsetUnit( const QgsUnitTypes::RenderUnit unit ) { mOffsetUnit = unit; }

    /**
     * Returns the units for the fill's offset.
     * \returns units for offset
     * \see setOffsetUnit
     * \see offset
     * \see offsetMapUnitScale
     */
    QgsUnitTypes::RenderUnit offsetUnit() const { return mOffsetUnit; }

    /**
     * Sets the map unit scale for the fill's offset.
     * \param scale map unit scale for offset
     * \see offsetMapUnitScale
     * \see setOffset
     * \see setOffsetUnit
     */
    void setOffsetMapUnitScale( const QgsMapUnitScale &scale ) { mOffsetMapUnitScale = scale; }

    /**
     * Returns the map unit scale for the fill's offset.
     * \returns map unit scale for offset
     * \see setOffsetMapUnitScale
     * \see offset
     * \see offsetUnit
     */
    const QgsMapUnitScale &offsetMapUnitScale() const { return mOffsetMapUnitScale; }

    /**
     * Sets the width for scaling the image used in the fill. The image's height will also be
     * scaled to maintain the image's aspect ratio.
     * \param width width for scaling the image
     * \see width
     * \see setWidthUnit
     * \see setWidthMapUnitScale
     */
    void setWidth( const double width ) { mWidth = width; }

    /**
     * Returns the width used for scaling the image used in the fill. The image's height is
     * scaled to maintain the image's aspect ratio.
     * \returns width used for scaling the image
     * \see setWidth
     * \see widthUnit
     * \see widthMapUnitScale
     */
    double width() const { return mWidth; }

    /**
     * Sets the units for the image's width.
     * \param unit units for width
     * \see widthUnit
     * \see setWidth
     * \see setWidthMapUnitScale
     */
    void setWidthUnit( const QgsUnitTypes::RenderUnit unit ) { mWidthUnit = unit; }

    /**
     * Returns the units for the image's width.
     * \returns units for width
     * \see setWidthUnit
     * \see width
     * \see widthMapUnitScale
     */
    QgsUnitTypes::RenderUnit widthUnit() const { return mWidthUnit; }

    /**
     * Sets the map unit scale for the image's width.
     * \param scale map unit scale for width
     * \see widthMapUnitScale
     * \see setWidth
     * \see setWidthUnit
     */
    void setWidthMapUnitScale( const QgsMapUnitScale &scale ) { mWidthMapUnitScale = scale; }

    /**
     * Returns the map unit scale for the image's width.
     * \returns map unit scale for width
     * \see setWidthMapUnitScale
     * \see width
     * \see widthUnit
     */
    const QgsMapUnitScale &widthMapUnitScale() const { return mWidthMapUnitScale; }

  protected:

    void applyDataDefinedSettings( QgsSymbolRenderContext &context ) override;

  private:

    //! Path to the image file
    QString mImageFilePath;
    FillCoordinateMode mCoordinateMode = QgsRasterFillSymbolLayer::Feature;
    double mOpacity = 1.0;

    QPointF mOffset;
    QgsUnitTypes::RenderUnit mOffsetUnit = QgsUnitTypes::RenderMillimeters;
    QgsMapUnitScale mOffsetMapUnitScale;

    double mWidth = 0.0;
    QgsUnitTypes::RenderUnit mWidthUnit = QgsUnitTypes::RenderPixels;
    QgsMapUnitScale mWidthMapUnitScale;

    //! Applies the image pattern to the brush
    void applyPattern( QBrush &brush, const QString &imageFilePath, double width, double opacity,
                       const QgsSymbolRenderContext &context );
};

/**
 * \ingroup core
 * A class for filling symbols with a repeated SVG file.
*/
class CORE_EXPORT QgsSVGFillSymbolLayer: public QgsImageFillSymbolLayer
{
  public:

    /**
     * Constructor for QgsSVGFillSymbolLayer, using the SVG picture at the specified absolute file path.
     */
    QgsSVGFillSymbolLayer( const QString &svgFilePath, double width = 20, double rotation = 0.0 );

    /**
     * Constructor for QgsSVGFillSymbolLayer, using the specified SVG picture data.
     */
    QgsSVGFillSymbolLayer( const QByteArray &svgData, double width = 20, double rotation = 0.0 );

    /**
     * Creates a new QgsSVGFillSymbolLayer from a \a properties map. The caller takes
     * ownership of the returned object.
     */
    static QgsSymbolLayer *create( const QgsStringMap &properties = QgsStringMap() ) SIP_FACTORY;

    /**
     * Creates a new QgsSVGFillSymbolLayer from a SLD \a element. The caller takes
     * ownership of the returned object.
     */
    static QgsSymbolLayer *createFromSld( QDomElement &element ) SIP_FACTORY;

    /**
     * Turns relative paths in properties map to absolute when reading and vice versa when writing.
     * Used internally when reading/writing symbols.
     * \since QGIS 3.0
     */
    static void resolvePaths( QgsStringMap &properties, const QgsPathResolver &pathResolver, bool saving );

    // implemented from base classes

    QString layerType() const override;
    void startRender( QgsSymbolRenderContext &context ) override;
    void stopRender( QgsSymbolRenderContext &context ) override;
    QgsStringMap properties() const override;
    QgsSVGFillSymbolLayer *clone() const override SIP_FACTORY;
    void toSld( QDomDocument &doc, QDomElement &element, const QgsStringMap &props ) const override;

    /**
     * Sets the path to the SVG file to render in the fill.
     *
     * This is usually an absolute file path. Other supported options include
     * - relative paths to folders from the user's SVG search paths
     * - base64 encoded content, prefixed with a 'base64:' string
     * - http(s) paths
     *
     * \see svgFilePath()
     */
    void setSvgFilePath( const QString &svgPath );

    /**
     * Returns the path to the SVG file used to render the fill.
     *
     * \see setSvgFilePath()
     */
    QString svgFilePath() const { return mSvgFilePath; }

    /**
     * Sets the \a width to render the SVG content as within the fill (i.e. the pattern repeat/tile size).
     *
     * Units are specified by setPatternWidthUnit()
     *
     * \see patternWidth()
     * \see setPatternWidthUnit()
     * \see setPatternWidthMapUnitScale()
     */
    void setPatternWidth( double width ) { mPatternWidth = width;}

    /**
     * Returns the width of the rendered SVG content within the fill (i.e. the pattern repeat/tile size).
     *
     * Units are retrieved by patternWidthUnit()
     *
     * \see setPatternWidth()
     * \see patternWidthUnit()
     * \see patternWidthMapUnitScale()
     */
    double patternWidth() const { return mPatternWidth; }

    /**
     * Sets the fill color used for rendering the SVG content.
     *
     * Fill color is only supported for parametrized SVG files. Color opacity is
     * ignored if the SVG file does not support parametrized fill opacity.
     *
     * \see svgFillColor()
     * \see setSvgStrokeColor()
     */
    void setSvgFillColor( const QColor &c ) { setColor( c );  }

    /**
     * Returns the fill color used for rendering the SVG content.
     *
     * Fill color is only supported for parametrized SVG files.
     *
     * \see setSvgFillColor()
     * \see svgStrokeColor()
     */
    QColor svgFillColor() const { return color(); }

    /**
     * Sets the stroke color used for rendering the SVG content.
     *
     * Stroke color is only supported for parametrized SVG files. Color opacity is
     * ignored if the SVG file does not support parametrized outline opacity.
     *
     * \see svgStrokeColor()
     * \see setSvgFillColor()
     */
    void setSvgStrokeColor( const QColor &c ) { mSvgStrokeColor = c; }

    /**
     * Returns the stroke color used for rendering the SVG content.
     *
     * Stroke color is only supported for parametrized SVG files.
     *
     * \see setSvgStrokeColor()
     * \see svgFillColor()
     */
    QColor svgStrokeColor() const { return mSvgStrokeColor; }

    /**
     * Sets the stroke width used for rendering the SVG content.
     *
     * Stroke width is only supported for parametrized SVG files. Units are
     * specified via setSvgStrokeWidthUnit()
     *
     * \see svgStrokeWidth()
     * \see setSvgStrokeWidthUnit()
     * \see setSvgStrokeWidthMapUnitScale()
     */
    void setSvgStrokeWidth( double w ) { mSvgStrokeWidth = w; }

    /**
     * Returns the stroke width used for rendering the SVG content.
     *
     * Stroke width is only supported for parametrized SVG files. Units are
     * retrieved via setSvgStrokeWidthUnit()
     *
     * \see setSvgStrokeWidth()
     * \see svgStrokeWidthUnit()
     * \see svgStrokeWidthMapUnitScale()
     */
    double svgStrokeWidth() const { return mSvgStrokeWidth; }

    /**
     * Sets the \a unit for the width of the SVG images in the pattern.
     *
     * \see patternWidthUnit()
     * \see setPatternWidth()
     * \see setPatternWidthMapUnitScale()
    */
    void setPatternWidthUnit( QgsUnitTypes::RenderUnit unit ) { mPatternWidthUnit = unit; }

    /**
     * Returns the units for the width of the SVG images in the pattern.
     *
     * \see setPatternWidthUnit()
     * \see patternWidth()
     * \see patternWidthMapUnitScale()
    */
    QgsUnitTypes::RenderUnit patternWidthUnit() const { return mPatternWidthUnit; }

    /**
     * Sets the map unit \a scale for the pattern's width.
     *
     * \see patternWidthMapUnitScale()
     * \see setPatternWidth()
     * \see setPatternWidthUnit()
     */
    void setPatternWidthMapUnitScale( const QgsMapUnitScale &scale ) { mPatternWidthMapUnitScale = scale; }

    /**
     * Returns the map unit scale for the pattern's width.
     *
     * \see setPatternWidthMapUnitScale()
     * \see patternWidth()
     * \see patternWidthUnit()
     */
    const QgsMapUnitScale &patternWidthMapUnitScale() const { return mPatternWidthMapUnitScale; }

    /**
     * Sets the \a unit for the stroke width.
     *
     * \see svgStrokeWidthUnit()
     * \see setSvgStrokeWidth()
     * \see setSvgStrokeWidthMapUnitScale()
    */
    void setSvgStrokeWidthUnit( QgsUnitTypes::RenderUnit unit ) { mSvgStrokeWidthUnit = unit; }

    /**
     * Returns the units for the stroke width.
     *
     * \see setSvgStrokeWidthUnit()
     * \see svgStrokeWidth()
     * \see svgStrokeWidthMapUnitScale()
    */
    QgsUnitTypes::RenderUnit svgStrokeWidthUnit() const { return mSvgStrokeWidthUnit; }

    /**
     * Sets the map unit \a scale for the pattern's stroke.
     *
     * \see svgStrokeWidthMapUnitScale()
     * \see setSvgStrokeWidth()
     * \see setSvgStrokeWidthUnit()
     */
    void setSvgStrokeWidthMapUnitScale( const QgsMapUnitScale &scale ) { mSvgStrokeWidthMapUnitScale = scale; }

    /**
     * Returns the map unit scale for the pattern's stroke.
     *
     * \see setSvgStrokeWidthMapUnitScale()
     * \see svgStrokeWidth()
     * \see svgStrokeWidthUnit()
     */
    const QgsMapUnitScale &svgStrokeWidthMapUnitScale() const { return mSvgStrokeWidthMapUnitScale; }

    void setOutputUnit( QgsUnitTypes::RenderUnit unit ) override;
    QgsUnitTypes::RenderUnit outputUnit() const override;

    void setMapUnitScale( const QgsMapUnitScale &scale ) override;
    QgsMapUnitScale mapUnitScale() const override;

  protected:

    void applyDataDefinedSettings( QgsSymbolRenderContext &context ) override;

  private:

    //! Width of the pattern (in output units)
    double mPatternWidth = 20;
    QgsUnitTypes::RenderUnit mPatternWidthUnit = QgsUnitTypes::RenderMillimeters;
    QgsMapUnitScale mPatternWidthMapUnitScale;

    //! SVG data
    QByteArray mSvgData;
    //! Path to the svg file (or empty if constructed directly from data)
    QString mSvgFilePath;
    //! SVG view box (to keep the aspect ratio
    QRectF mSvgViewBox;

    //param(fill), param(stroke), param(stroke-width) are going
    //to be replaced in memory
    QColor mSvgStrokeColor = QColor( 35, 35, 35 );
    double mSvgStrokeWidth = 0.2;
    QgsUnitTypes::RenderUnit mSvgStrokeWidthUnit = QgsUnitTypes::RenderMillimeters;
    QgsMapUnitScale mSvgStrokeWidthMapUnitScale;

    //! Helper function that gets the view box from the byte array
    void storeViewBox();
    void setDefaultSvgParams(); //fills mSvgFillColor, mSvgStrokeColor, mSvgStrokeWidth with default values for mSvgFilePath

    //! Applies the svg pattern to the brush
    void applyPattern( QBrush &brush, const QString &svgFilePath, double patternWidth, QgsUnitTypes::RenderUnit patternWidthUnit, const QColor &svgFillColor, const QColor &svgStrokeColor,
                       double svgStrokeWidth, QgsUnitTypes::RenderUnit svgStrokeWidthUnit, const QgsSymbolRenderContext &context, const QgsMapUnitScale &patternWidthMapUnitScale, const QgsMapUnitScale &svgStrokeWidthMapUnitScale );
};

/**
 * \ingroup core
 * \class QgsLinePatternFillSymbolLayer
 * A symbol fill consisting of repeated parallel lines.
 */
class CORE_EXPORT QgsLinePatternFillSymbolLayer: public QgsImageFillSymbolLayer
{
  public:
    QgsLinePatternFillSymbolLayer();
    ~QgsLinePatternFillSymbolLayer() override;

    /**
     * Creates a new QgsLinePatternFillSymbolLayer from a \a properties map. The caller takes
     * ownership of the returned object.
     */
    static QgsSymbolLayer *create( const QgsStringMap &properties = QgsStringMap() ) SIP_FACTORY;

    /**
     * Creates a new QgsLinePatternFillSymbolLayer from a SLD \a element. The caller takes
     * ownership of the returned object.
     */
    static QgsSymbolLayer *createFromSld( QDomElement &element ) SIP_FACTORY;

    QString layerType() const override;
    void startRender( QgsSymbolRenderContext &context ) override;
    void stopRender( QgsSymbolRenderContext &context ) override;
    QgsStringMap properties() const override;
    QgsLinePatternFillSymbolLayer *clone() const override SIP_FACTORY;
    void toSld( QDomDocument &doc, QDomElement &element, const QgsStringMap &props ) const override;
    double estimateMaxBleed( const QgsRenderContext &context ) const override;

    QString ogrFeatureStyleWidth( double widthScaleFactor ) const;

    /**
     * Sets the angle for the parallel lines used to fill the symbol.
     *
     * Angles are in degrees, clockwise from North.
     *
     * \see lineAngle()
     */
    void setLineAngle( double a ) { mLineAngle = a; }

    /**
     * Returns the angle for the parallel lines used to fill the symbol.
     *
     * Angles are in degrees, clockwise from North.
     *
     * \see setLineAngle()
     */
    double lineAngle() const { return mLineAngle; }

    /**
     * Sets the distance between lines in the fill pattern.
     * \param d distance. Units are specified by setDistanceUnit()
     * \see distance()
     * \see setDistanceUnit()
    */
    void setDistance( double d ) { mDistance = d; }

    /**
     * Returns the distance between lines in the fill pattern. Units are retrieved by distanceUnit().
     * \see setDistance()
     * \see distanceUnit()
    */
    double distance() const { return mDistance; }

    /**
     * Sets the width of the line subsymbol used to render the parallel lines
     * in the fill.
     *
     * \see lineWidth()
     */
    void setLineWidth( double w );

    /**
     * Returns the width of the line subsymbol used to render the parallel lines
     * in the fill.
     *
     * \see setLineWidth()
     */
    double lineWidth() const { return mLineWidth; }

    void setColor( const QColor &c ) override;
    QColor color() const override;

    /**
     * Sets the \a offset distance for lines within the fill, which is
     * the distance to offset the parallel lines from their normal
     * position.
     *
     * Units are specified via setOffsetUnit().
     *
     * \see offset()
     * \see setOffsetUnit()
     * \see setOffsetMapUnitScale()
     */
    void setOffset( double offset ) { mOffset = offset; }

    /**
     * Returns the offset distance for lines within the fill, which is
     * the distance to offset the parallel lines from their normal
     * position.
     *
     * Units are retrieved via offsetUnit().
     *
     * \see setOffset()
     * \see offsetUnit()
     * \see offsetMapUnitScale()
     */
    double offset() const { return mOffset; }

    /**
     * Sets the \a unit for the distance between lines in the fill pattern.
     *
     * \see distanceUnit()
     * \see setDistance()
    */
    void setDistanceUnit( QgsUnitTypes::RenderUnit unit ) { mDistanceUnit = unit; }

    /**
     * Returns the units for the distance between lines in the fill pattern.
     *
     * \see setDistanceUnit()
     * \see distance()
    */
    QgsUnitTypes::RenderUnit distanceUnit() const { return mDistanceUnit; }

    /**
     * Sets the map unit \a scale for the pattern's line distance.
     *
     * \see distanceMapUnitScale()
     * \see setDistance()
     * \see setDistanceUnit()
     */
    void setDistanceMapUnitScale( const QgsMapUnitScale &scale ) { mDistanceMapUnitScale = scale; }

    /**
     * Returns the map unit scale for the pattern's line distance.
     *
     * \see setDistanceMapUnitScale()
     * \see distance()
     * \see distanceUnit()
     */
    const QgsMapUnitScale &distanceMapUnitScale() const { return mDistanceMapUnitScale; }

    /**
     * Sets the \a unit for the line's width.
     *
     * \see lineWidthUnit()
    */
    void setLineWidthUnit( QgsUnitTypes::RenderUnit unit ) { mLineWidthUnit = unit; }

    /**
     * Returns the units for the line's width.
     *
     * \see setLineWidthUnit()
    */
    QgsUnitTypes::RenderUnit lineWidthUnit() const { return mLineWidthUnit; }

    /**
     * Sets the map unit \a scale for the pattern's line width.
     *
     * \see lineWidthMapUnitScale()
     * \see setLineWidth()
     * \see setLineWidthUnit()
     */
    void setLineWidthMapUnitScale( const QgsMapUnitScale &scale ) { mLineWidthMapUnitScale = scale; }

    /**
     * Returns the map unit scale for the pattern's line width.
     *
     * \see setLineWidthMapUnitScale()
     * \see lineWidth()
     * \see lineWidthUnit()
     */
    const QgsMapUnitScale &lineWidthMapUnitScale() const { return mLineWidthMapUnitScale; }

    /**
     * Sets the \a unit for the line pattern's offset.

     * \see offsetUnit()
    */
    void setOffsetUnit( QgsUnitTypes::RenderUnit unit ) { mOffsetUnit = unit; }

    /**
     * Returns the units for the line pattern's offset.
     *
     * \see setOffsetUnit()
    */
    QgsUnitTypes::RenderUnit offsetUnit() const { return mOffsetUnit; }

    /**
     * Sets the map unit \a scale for the pattern's line offset.
     *
     * \see offsetMapUnitScale()
     * \see setOffset()
     * \see setOffsetUnit()
     */
    void setOffsetMapUnitScale( const QgsMapUnitScale &scale ) { mOffsetMapUnitScale = scale; }

    /**
     * Returns the map unit scale for the pattern's line offset.
     *
     * \see setOffsetMapUnitScale()
     * \see offset()
     * \see offsetUnit()
     */
    const QgsMapUnitScale &offsetMapUnitScale() const { return mOffsetMapUnitScale; }

    void setOutputUnit( QgsUnitTypes::RenderUnit unit ) override;
    QgsUnitTypes::RenderUnit outputUnit() const override;
    void setMapUnitScale( const QgsMapUnitScale &scale ) override;
    QgsMapUnitScale mapUnitScale() const override;
    bool setSubSymbol( QgsSymbol *symbol SIP_TRANSFER ) override;
    QgsSymbol *subSymbol() override;
    QSet<QString> usedAttributes( const QgsRenderContext &context ) const override;
    bool hasDataDefinedProperties() const override;

  protected:

    void applyDataDefinedSettings( QgsSymbolRenderContext &context ) override;

  private:
    //! Distance (in mm or map units) between lines
    double mDistance = 5.0;
    QgsUnitTypes::RenderUnit mDistanceUnit = QgsUnitTypes::RenderMillimeters;
    QgsMapUnitScale mDistanceMapUnitScale;
    //! Line width (in mm or map units)
    double mLineWidth = 0;
    QgsUnitTypes::RenderUnit mLineWidthUnit = QgsUnitTypes::RenderMillimeters;
    QgsMapUnitScale mLineWidthMapUnitScale;
    //! Vector line angle in degrees (0 = horizontal, counterclockwise)
    double mLineAngle = 45.0;
    //! Offset perpendicular to line direction
    double mOffset = 0.0;
    QgsUnitTypes::RenderUnit mOffsetUnit = QgsUnitTypes::RenderMillimeters;
    QgsMapUnitScale mOffsetMapUnitScale;

#ifdef SIP_RUN
    QgsLinePatternFillSymbolLayer( const QgsLinePatternFillSymbolLayer &other );
#endif

    //! Applies the svg pattern to the brush
    void applyPattern( const QgsSymbolRenderContext &context, QBrush &brush, double lineAngle, double distance );

    //! Fill line
    QgsLineSymbol *mFillLineSymbol = nullptr;
};

/**
 * \ingroup core
 * \class QgsPointPatternFillSymbolLayer
 */
class CORE_EXPORT QgsPointPatternFillSymbolLayer: public QgsImageFillSymbolLayer
{
  public:
    QgsPointPatternFillSymbolLayer();
    ~QgsPointPatternFillSymbolLayer() override;

    static QgsSymbolLayer *create( const QgsStringMap &properties = QgsStringMap() ) SIP_FACTORY;
    static QgsSymbolLayer *createFromSld( QDomElement &element ) SIP_FACTORY;

    QString layerType() const override;

    void startRender( QgsSymbolRenderContext &context ) override;

    void stopRender( QgsSymbolRenderContext &context ) override;

    QgsStringMap properties() const override;

    QgsPointPatternFillSymbolLayer *clone() const override SIP_FACTORY;

    void toSld( QDomDocument &doc, QDomElement &element, const QgsStringMap &props ) const override;

    double estimateMaxBleed( const QgsRenderContext &context ) const override;

    //getters and setters
    double distanceX() const { return mDistanceX; }
    void setDistanceX( double d ) { mDistanceX = d; }

    double distanceY() const { return mDistanceY; }
    void setDistanceY( double d ) { mDistanceY = d; }

    double displacementX() const { return mDisplacementX; }
    void setDisplacementX( double d ) { mDisplacementX = d; }

    double displacementY() const { return mDisplacementY; }
    void setDisplacementY( double d ) { mDisplacementY = d; }

    /**
     * Sets the horizontal offset values for points in the pattern.
     * \param offset offset value
     * \see offsetX()
     * \see setOffsetY()
     * \since QGIS 3.8
    */
    void setOffsetX( double offset ) { mOffsetX = offset; }

    /**
     * Returns the horizontal offset values for points in the pattern.
     * \see setOffsetX()
     * \see offsetY()
     * \since QGIS 3.8
    */
    double offsetX() const { return mOffsetX; }

    /**
     * Sets the vertical offset values for points in the pattern.
     * \param offset offset value
     * \see offsetY()
     * \see setOffsetX()
     * \since QGIS 3.8
    */
    void setOffsetY( double offset ) { mOffsetY = offset; }

    /**
     * Returns the vertical offset values for points in the pattern.
     * \see setOffsetY()
     * \see offsetX()
     * \since QGIS 3.8
    */
    double offsetY() const { return mOffsetY; }

    bool setSubSymbol( QgsSymbol *symbol SIP_TRANSFER ) override;
    QgsSymbol *subSymbol() override { return mMarkerSymbol; }

    /**
     * Sets the units for the horizontal distance between points in the pattern.
     * \param unit distance units
     * \see distanceXUnit()
     * \see setDistanceYUnit()
    */
    void setDistanceXUnit( QgsUnitTypes::RenderUnit unit ) { mDistanceXUnit = unit; }

    /**
     * Returns the units for the horizontal distance between points in the pattern.
     * \see setDistanceXUnit()
     * \see distanceYUnit()
    */
    QgsUnitTypes::RenderUnit distanceXUnit() const { return mDistanceXUnit; }

    void setDistanceXMapUnitScale( const QgsMapUnitScale &scale ) { mDistanceXMapUnitScale = scale; }
    const QgsMapUnitScale &distanceXMapUnitScale() const { return mDistanceXMapUnitScale; }

    /**
     * Sets the units for the vertical distance between points in the pattern.
     * \param unit distance units
     * \see distanceYUnit()
     * \see setDistanceXUnit()
    */
    void setDistanceYUnit( QgsUnitTypes::RenderUnit unit ) { mDistanceYUnit = unit; }

    /**
     * Returns the units for the vertical distance between points in the pattern.
     * \see setDistanceYUnit()
     * \see distanceXUnit()
    */
    QgsUnitTypes::RenderUnit distanceYUnit() const { return mDistanceYUnit; }

    void setDistanceYMapUnitScale( const QgsMapUnitScale &scale ) { mDistanceYMapUnitScale = scale; }
    const QgsMapUnitScale &distanceYMapUnitScale() const { return mDistanceYMapUnitScale; }

    /**
     * Sets the units for the horizontal displacement between rows in the pattern.
     * \param unit displacement units
     * \see displacementXUnit()
     * \see setDisplacementYUnit()
    */
    void setDisplacementXUnit( QgsUnitTypes::RenderUnit unit ) { mDisplacementXUnit = unit; }

    /**
     * Returns the units for the horizontal displacement between rows in the pattern.
     * \see setDisplacementXUnit()
     * \see displacementYUnit()
    */
    QgsUnitTypes::RenderUnit displacementXUnit() const { return mDisplacementXUnit; }

    void setDisplacementXMapUnitScale( const QgsMapUnitScale &scale ) { mDisplacementXMapUnitScale = scale; }
    const QgsMapUnitScale &displacementXMapUnitScale() const { return mDisplacementXMapUnitScale; }

    /**
     * Sets the units for the vertical displacement between rows in the pattern.
     * \param unit displacement units
     * \see displacementYUnit()
     * \see setDisplacementXUnit()
    */
    void setDisplacementYUnit( QgsUnitTypes::RenderUnit unit ) { mDisplacementYUnit = unit; }

    /**
     * Returns the units for the vertical displacement between rows in the pattern.
     * \see setDisplacementYUnit()
     * \see displacementXUnit()
    */
    QgsUnitTypes::RenderUnit displacementYUnit() const { return mDisplacementYUnit; }

    void setDisplacementYMapUnitScale( const QgsMapUnitScale &scale ) { mDisplacementYMapUnitScale = scale; }
    const QgsMapUnitScale &displacementYMapUnitScale() const { return mDisplacementYMapUnitScale; }

    /**
     * Sets the units for the horizontal offset between rows in the pattern.
     * \param unit offset units
     * \see offsetXUnit()
     * \see setOffsetYUnit()
     * \since QGIS 3.8
    */
    void setOffsetXUnit( QgsUnitTypes::RenderUnit unit ) { mOffsetXUnit = unit; }

    /**
     * Returns the units for the horizontal offset for rows in the pattern.
     * \see setOffsetXUnit()
     * \see offsetYUnit()
     * \since QGIS 3.8
    */
    QgsUnitTypes::RenderUnit offsetXUnit() const { return mOffsetXUnit; }

    /**
     * Sets the unit scale for the horizontal offset for rows in the pattern.
     * \param scale offset unit scale
     * \see offsetXMapUnitScale()
     * \see setOffsetYMapUnitScale()
     * \since QGIS 3.8
    */
    void setOffsetXMapUnitScale( const QgsMapUnitScale &scale ) { mOffsetXMapUnitScale = scale; }

    /**
     * Returns the unit scale for the horizontal offset for rows in the pattern.
     * \see setOffsetXMapUnitScale()
     * \see offsetYMapUnitScale()
     * \since QGIS 3.8
    */
    const QgsMapUnitScale &offsetXMapUnitScale() const { return mOffsetXMapUnitScale; }

    /**
     * Sets the units for the vertical offset for rows in the pattern.
     * \param unit offset units
     * \see offsetYUnit()
     * \see setOffsetXUnit()
     * \since QGIS 3.8
    */
    void setOffsetYUnit( QgsUnitTypes::RenderUnit unit ) { mOffsetYUnit = unit; }

    /**
     * Returns the units for the vertical offset for rows in the pattern.
     * \see setOffsetYUnit()
     * \see offsetXUnit()
     * \since QGIS 3.8
    */
    QgsUnitTypes::RenderUnit offsetYUnit() const { return mOffsetYUnit; }

    /**
     * Sets the unit scale for the vertical offset for rows in the pattern.
     * \param scale offset unit scale
     * \see offsetYMapUnitScale()
     * \see setOffsetXMapUnitScale()
     * \since QGIS 3.8
    */
    void setOffsetYMapUnitScale( const QgsMapUnitScale &scale ) { mOffsetYMapUnitScale = scale; }

    /**
     * Returns the unit scale for the vertical offset between rows in the pattern.
     * \see setOffsetYMapUnitScale()
     * \see offsetXMapUnitScale()
     * \since QGIS 3.8
    */
    const QgsMapUnitScale &offsetYMapUnitScale() const { return mOffsetYMapUnitScale; }

    void setOutputUnit( QgsUnitTypes::RenderUnit unit ) override;
    QgsUnitTypes::RenderUnit outputUnit() const override;

    void setMapUnitScale( const QgsMapUnitScale &scale ) override;
    QgsMapUnitScale mapUnitScale() const override;

    QSet<QString> usedAttributes( const QgsRenderContext &context ) const override;
    bool hasDataDefinedProperties() const override;
    void setColor( const QColor &c ) override;
    QColor color() const override;

  protected:
    QgsMarkerSymbol *mMarkerSymbol = nullptr;
    double mDistanceX = 15;
    QgsUnitTypes::RenderUnit mDistanceXUnit = QgsUnitTypes::RenderMillimeters;
    QgsMapUnitScale mDistanceXMapUnitScale;
    double mDistanceY = 15;
    QgsUnitTypes::RenderUnit mDistanceYUnit = QgsUnitTypes::RenderMillimeters;
    QgsMapUnitScale mDistanceYMapUnitScale;
    double mDisplacementX = 0;
    QgsUnitTypes::RenderUnit mDisplacementXUnit = QgsUnitTypes::RenderMillimeters;
    QgsMapUnitScale mDisplacementXMapUnitScale;
    double mDisplacementY = 0;
    QgsUnitTypes::RenderUnit mDisplacementYUnit = QgsUnitTypes::RenderMillimeters;
    QgsMapUnitScale mDisplacementYMapUnitScale;
    double mOffsetX = 0;
    QgsUnitTypes::RenderUnit mOffsetXUnit = QgsUnitTypes::RenderMillimeters;
    QgsMapUnitScale mOffsetXMapUnitScale;
    double mOffsetY = 0;
    QgsUnitTypes::RenderUnit mOffsetYUnit = QgsUnitTypes::RenderMillimeters;
    QgsMapUnitScale mOffsetYMapUnitScale;

    void applyDataDefinedSettings( QgsSymbolRenderContext &context ) override;

  private:
#ifdef SIP_RUN
    QgsPointPatternFillSymbolLayer( const QgsPointPatternFillSymbolLayer &other );
#endif

    void applyPattern( const QgsSymbolRenderContext &context, QBrush &brush, double distanceX, double distanceY,
                       double displacementX, double displacementY, double offsetX, double offsetY );
};

/**
 * \ingroup core
 * \class QgsCentroidFillSymbolLayer
 */
class CORE_EXPORT QgsCentroidFillSymbolLayer : public QgsFillSymbolLayer
{
  public:
    QgsCentroidFillSymbolLayer();

    // static stuff

    static QgsSymbolLayer *create( const QgsStringMap &properties = QgsStringMap() ) SIP_FACTORY;
    static QgsSymbolLayer *createFromSld( QDomElement &element ) SIP_FACTORY;

    // implemented from base classes

    QString layerType() const override;

    void startRender( QgsSymbolRenderContext &context ) override;

    void stopRender( QgsSymbolRenderContext &context ) override;

    void renderPolygon( const QPolygonF &points, QList<QPolygonF> *rings, QgsSymbolRenderContext &context ) override;

    QgsStringMap properties() const override;

    QgsCentroidFillSymbolLayer *clone() const override SIP_FACTORY;

    void toSld( QDomDocument &doc, QDomElement &element, const QgsStringMap &props ) const override;

    void setColor( const QColor &color ) override;
    QColor color() const override;

    QgsSymbol *subSymbol() override;
    bool setSubSymbol( QgsSymbol *symbol SIP_TRANSFER ) override;

    void setOutputUnit( QgsUnitTypes::RenderUnit unit ) override;
    QgsUnitTypes::RenderUnit outputUnit() const override;

    void setMapUnitScale( const QgsMapUnitScale &scale ) override;
    QgsMapUnitScale mapUnitScale() const override;

    QSet<QString> usedAttributes( const QgsRenderContext &context ) const override;
    bool hasDataDefinedProperties() const override;

    void setPointOnSurface( bool pointOnSurface ) { mPointOnSurface = pointOnSurface; }
    bool pointOnSurface() const { return mPointOnSurface; }

    /**
     * Sets whether a point is drawn for all parts or only on the biggest part of multi-part features.
     * \since QGIS 2.16 */
    void setPointOnAllParts( bool pointOnAllParts ) { mPointOnAllParts = pointOnAllParts; }

    /**
     * Returns whether a point is drawn for all parts or only on the biggest part of multi-part features.
     * \since QGIS 2.16 */
    bool pointOnAllParts() const { return mPointOnAllParts; }

  protected:
    std::unique_ptr< QgsMarkerSymbol > mMarker;
    bool mPointOnSurface = false;
    bool mPointOnAllParts = true;

    QgsFeatureId mCurrentFeatureId = -1;
    int mBiggestPartIndex = -1;

  private:
#ifdef SIP_RUN
    QgsCentroidFillSymbolLayer( const QgsCentroidFillSymbolLayer &other );
#endif
};

#endif


