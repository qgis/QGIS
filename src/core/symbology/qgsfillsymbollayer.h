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

class QgsMarkerSymbol;
class QgsLineSymbol;
class QgsPathResolver;

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

    ~QgsSimpleFillSymbolLayer() override;

    // static stuff

    /**
     * Creates a new QgsSimpleFillSymbolLayer using the specified \a properties map containing symbol properties (see properties()).
     *
     * Caller takes ownership of the returned symbol layer.
     */
    static QgsSymbolLayer *create( const QVariantMap &properties = QVariantMap() ) SIP_FACTORY;
    static QgsSymbolLayer *createFromSld( QDomElement &element ) SIP_FACTORY;

    // implemented from base classes

    QString layerType() const override;

    void startRender( QgsSymbolRenderContext &context ) override;

    void stopRender( QgsSymbolRenderContext &context ) override;

    void renderPolygon( const QPolygonF &points, const QVector<QPolygonF> *rings, QgsSymbolRenderContext &context ) override;

    QVariantMap properties() const override;

    QgsSimpleFillSymbolLayer *clone() const override SIP_FACTORY;

    void toSld( QDomDocument &doc, QDomElement &element, const QVariantMap &props ) const override;

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

    /**
     * Sets an \a offset by which polygons will be translated during rendering.
     *
     * Units are specified by offsetUnit().
     *
     * \see offset()
     * \see setOffsetUnit()
     */
    void setOffset( QPointF offset ) { mOffset = offset; }

    /**
     * Returns the offset by which polygons will be translated during rendering.
     *
     * Units are specified by offsetUnit().
     *
     * \see setOffset()
     * \see offsetUnit()
     */
    QPointF offset() const { return mOffset; }

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
     * Sets the \a unit for the fill's offset.
     * \see offset()
     * \see offsetUnit()
    */
    void setOffsetUnit( QgsUnitTypes::RenderUnit unit ) { mOffsetUnit = unit; }

    /**
     * Returns the units for the fill's offset.
     * \see setOffsetUnit()
     * \see offset()
    */
    QgsUnitTypes::RenderUnit offsetUnit() const { return mOffsetUnit; }

    /**
     * Sets the map unit \a scale for the fill's offset.
     * \see setOffset()
     * \see offsetMapUnitScale()
    */
    void setOffsetMapUnitScale( const QgsMapUnitScale &scale ) { mOffsetMapUnitScale = scale; }

    /**
     * Returns the map unit scale for the fill's offset.
     * \see offset()
     * \see setOffsetMapUnitScale()
    */
    const QgsMapUnitScale &offsetMapUnitScale() const { return mOffsetMapUnitScale; }

    void setOutputUnit( QgsUnitTypes::RenderUnit unit ) override;
    QgsUnitTypes::RenderUnit outputUnit() const override;
    bool usesMapUnits() const override;

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

    /**
     * Constructor for QgsGradientFillSymbolLayer.
     */
    QgsGradientFillSymbolLayer( const QColor &color = DEFAULT_SIMPLEFILL_COLOR,
                                const QColor &color2 = Qt::white,
                                Qgis::GradientColorSource gradientColorType = Qgis::GradientColorSource::SimpleTwoColor,
                                Qgis::GradientType gradientType = Qgis::GradientType::Linear,
                                Qgis::SymbolCoordinateReference coordinateMode = Qgis::SymbolCoordinateReference::Feature,
                                Qgis::GradientSpread gradientSpread = Qgis::GradientSpread::Pad
                              );

    ~QgsGradientFillSymbolLayer() override;

    // static stuff

    /**
     * Creates a new QgsGradientFillSymbolLayer using the specified \a properties map containing symbol properties (see properties()).
     *
     * Caller takes ownership of the returned symbol layer.
     */
    static QgsSymbolLayer *create( const QVariantMap &properties = QVariantMap() ) SIP_FACTORY;

    // implemented from base classes

    QString layerType() const override;
    void startRender( QgsSymbolRenderContext &context ) override;
    void stopRender( QgsSymbolRenderContext &context ) override;
    void renderPolygon( const QPolygonF &points, const QVector<QPolygonF> *rings, QgsSymbolRenderContext &context ) override;
    QVariantMap properties() const override;
    QgsGradientFillSymbolLayer *clone() const override SIP_FACTORY;
    double estimateMaxBleed( const QgsRenderContext &context ) const override;
    bool canCauseArtifactsBetweenAdjacentTiles() const override;

    /**
     * Returns the type of gradient, e.g., linear or radial.
     *
     * \see setGradientType()
     */
    Qgis::GradientType gradientType() const { return mGradientType; }

    /**
     * Sets the type of gradient, e.g., linear or radial.
     *
     * \see gradientType()
     */
    void setGradientType( Qgis::GradientType gradientType ) { mGradientType = gradientType; }

    /**
     * Returns the gradient color mode, which controls how gradient color stops are created.
     *
     * \see setGradientColorType()
     */
    Qgis::GradientColorSource gradientColorType() const { return mGradientColorType; }

    /**
     * Sets the gradient color mode, which controls how gradient color stops are created.
     *
     * \see gradientColorType()
     */
    void setGradientColorType( Qgis::GradientColorSource gradientColorType ) { mGradientColorType = gradientColorType; }

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

    /**
     * Returns the color for endpoint of gradient, only used if the gradient color type is set to SimpleTwoColor.
     *
     * \see setColor2()
     */
    QColor color2() const { return mColor2; }

    /**
     * Sets the color for endpoint of gradient, only used if the gradient color type is set to SimpleTwoColor.
     *
     * \see color2()
     */
    void setColor2( const QColor &color2 ) { mColor2 = color2; }

    /**
     * Returns the coordinate mode for gradient, which controls how the gradient stops are positioned.
     *
     * \see setCoordinateMode()
     */
    Qgis::SymbolCoordinateReference coordinateMode() const { return mCoordinateMode; }

    /**
     * Sets the coordinate mode for gradient, which controls how the gradient stops are positioned.
     *
     * \see coordinateMode()
     */
    void setCoordinateMode( Qgis::SymbolCoordinateReference coordinateMode ) { mCoordinateMode = coordinateMode; }

    /**
     * Returns the gradient spread mode, which controls how the gradient behaves outside of the predefined stops.
     *
     * \see setGradientSpread()
     */
    Qgis::GradientSpread gradientSpread() const { return mGradientSpread; }

    /**
     * Sets the gradient spread mode, which controls how the gradient behaves outside of the predefined stops.
     *
     * \see gradientSpread()
     */
    void setGradientSpread( Qgis::GradientSpread gradientSpread ) { mGradientSpread = gradientSpread; }

    /**
     * Sets the starting point of gradient fill, in the range [0,0] - [1,1].
     *
     * \see referencePoint1()
     */
    void setReferencePoint1( QPointF referencePoint ) { mReferencePoint1 = referencePoint; }

    /**
     * Returns the starting point of gradient fill, in the range [0,0] - [1,1].
     *
     * \see setReferencePoint1()
     */
    QPointF referencePoint1() const { return mReferencePoint1; }

    /**
     * Sets whether the starting point for the gradient is taken from the feature centroid.
     *
     * \see referencePoint1IsCentroid()
     */
    void setReferencePoint1IsCentroid( bool isCentroid ) { mReferencePoint1IsCentroid = isCentroid; }

    /**
     * Returns whether the starting point for the gradient is taken from the feature centroid.
     *
     * \see setReferencePoint1IsCentroid()
     */
    bool referencePoint1IsCentroid() const { return mReferencePoint1IsCentroid; }

    /**
     * Sets the end point of gradient fill, in the range [0,0] - [1,1].
     *
     * \see referencePoint2()
     */
    void setReferencePoint2( QPointF referencePoint ) { mReferencePoint2 = referencePoint; }

    /**
     * Returns the end point of gradient fill, in the range [0,0] - [1,1].
     *
     * \see setReferencePoint2()
     */
    QPointF referencePoint2() const { return mReferencePoint2; }

    /**
     * Sets whether the end point for the gradient is taken from the feature centroid.
     *
     * \see referencePoint2IsCentroid()
     */
    void setReferencePoint2IsCentroid( bool isCentroid ) { mReferencePoint2IsCentroid = isCentroid; }


    /**
     * Returns whether the end point for the gradient is taken from the feature centroid.
     *
     * \see setReferencePoint2IsCentroid()
     */
    bool referencePoint2IsCentroid() const { return mReferencePoint2IsCentroid; }

    /**
     * Sets an \a offset by which polygons will be translated during rendering.
     *
     * Units are specified by offsetUnit().
     *
     * \see offset()
     * \see setOffsetUnit()
     */
    void setOffset( QPointF offset ) { mOffset = offset; }

    /**
     * Returns the offset by which polygons will be translated during rendering.
     *
     * Units are specified by offsetUnit().
     *
     * \see setOffset()
     * \see offsetUnit()
     */
    QPointF offset() const { return mOffset; }

    /**
     * Sets the \a unit for the fill's offset.
     * \see offset()
     * \see offsetUnit()
    */
    void setOffsetUnit( QgsUnitTypes::RenderUnit unit ) { mOffsetUnit = unit; }

    /**
     * Returns the units for the fill's offset.
     * \see setOffsetUnit()
     * \see offset()
    */
    QgsUnitTypes::RenderUnit offsetUnit() const { return mOffsetUnit; }

    /**
     * Sets the map unit \a scale for the fill's offset.
     * \see setOffset()
     * \see offsetMapUnitScale()
    */
    void setOffsetMapUnitScale( const QgsMapUnitScale &scale ) { mOffsetMapUnitScale = scale; }

    /**
     * Returns the map unit scale for the fill's offset.
     * \see offset()
     * \see setOffsetMapUnitScale()
    */
    const QgsMapUnitScale &offsetMapUnitScale() const { return mOffsetMapUnitScale; }

    void setOutputUnit( QgsUnitTypes::RenderUnit unit ) override;
    QgsUnitTypes::RenderUnit outputUnit() const override;
    bool usesMapUnits() const override;

    void setMapUnitScale( const QgsMapUnitScale &scale ) override;
    QgsMapUnitScale mapUnitScale() const override;

  protected:
    QBrush mBrush;
    QBrush mSelBrush;

    Qgis::GradientColorSource mGradientColorType;
    QColor mColor2;
    QgsColorRamp *mGradientRamp = nullptr;
    Qgis::GradientType mGradientType;
    Qgis::SymbolCoordinateReference mCoordinateMode;
    Qgis::GradientSpread mGradientSpread;

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
                        Qgis::GradientColorSource gradientColorType, QgsColorRamp *gradientRamp, Qgis::GradientType gradientType,
                        Qgis::SymbolCoordinateReference coordinateMode, Qgis::GradientSpread gradientSpread,
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

    /**
     * Constructor for QgsShapeburstFillSymbolLayer.
     */
    QgsShapeburstFillSymbolLayer( const QColor &color = DEFAULT_SIMPLEFILL_COLOR, const QColor &color2 = Qt::white,
                                  Qgis::GradientColorSource colorType = Qgis::GradientColorSource::SimpleTwoColor,
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

    /**
     * Creates a new QgsShapeburstFillSymbolLayer using the specified \a properties map containing symbol properties (see properties()).
     *
     * Caller takes ownership of the returned symbol layer.
     */
    static QgsSymbolLayer *create( const QVariantMap &properties = QVariantMap() ) SIP_FACTORY;

    // implemented from base classes

    QString layerType() const override;
    void startRender( QgsSymbolRenderContext &context ) override;
    void stopRender( QgsSymbolRenderContext &context ) override;
    void renderPolygon( const QPolygonF &points, const QVector<QPolygonF> *rings, QgsSymbolRenderContext &context ) override;
    QVariantMap properties() const override;
    QgsShapeburstFillSymbolLayer *clone() const override SIP_FACTORY;
    double estimateMaxBleed( const QgsRenderContext &context ) const override;
    bool canCauseArtifactsBetweenAdjacentTiles() const override;

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
    void setColorType( Qgis::GradientColorSource colorType ) { mColorType = colorType; }

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
    Qgis::GradientColorSource colorType() const { return mColorType; }

    /**
     * Sets the color \a ramp used to draw the shapeburst fill. Color ramps are only used if setColorType is set ShapeburstColorType::ColorRamp.
     *
     * Ownership of \a ramp is transferred to the fill.
     *
     * \see setColorType
     * \see colorRamp
     * \since QGIS 2.3
     */
    void setColorRamp( QgsColorRamp *ramp SIP_TRANSFER );

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
    bool usesMapUnits() const override;

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

    Qgis::GradientColorSource mColorType = Qgis::GradientColorSource::SimpleTwoColor;
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
    void distanceTransform2d( double *im, int width, int height, QgsRenderContext &context );
    /* distance transform of a binary QImage */
    double *distanceTransform( QImage *im, QgsRenderContext &context );

    /* fills a QImage with values from an array of doubles containing squared distance transform values */
    void dtArrayToQImage( double *array, QImage *im, QgsColorRamp *ramp, QgsRenderContext &context, bool useWholeShape = true, int maxPixelDistance = 0 );

#ifdef SIP_RUN
    QgsShapeburstFillSymbolLayer( const QgsShapeburstFillSymbolLayer &other );
#endif
};

/**
 * \ingroup core
 * \brief Base class for polygon renderers generating texture images
*/
class CORE_EXPORT QgsImageFillSymbolLayer: public QgsFillSymbolLayer
{
  public:

    QgsImageFillSymbolLayer();
    ~QgsImageFillSymbolLayer() override;

    void renderPolygon( const QPolygonF &points, const QVector<QPolygonF> *rings, QgsSymbolRenderContext &context ) override;

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

    /**
     * Sets the coordinate reference mode for fill which controls how the top left corner of the image
     * fill is positioned relative to the feature.
     * \param coordinateReference coordinate reference mode
     * \see coordinateReference
     * \since QGIS 3.24
     */
    void setCoordinateReference( Qgis::SymbolCoordinateReference coordinateReference ) { mCoordinateReference = coordinateReference; }

    /**
     * Returns the coordinate reference mode for fill which controls how the top left corner of the image
     * fill is positioned relative to the feature.
     * \returns coordinate reference mode
     * \see setCoordinateReference
     * \since QGIS 3.24
     */
    Qgis::SymbolCoordinateReference coordinateReference() const { return mCoordinateReference; }

    void setOutputUnit( QgsUnitTypes::RenderUnit unit ) override;
    QgsUnitTypes::RenderUnit outputUnit() const override;
    void setMapUnitScale( const QgsMapUnitScale &scale ) override;
    QgsMapUnitScale mapUnitScale() const override;
    double dxfWidth( const QgsDxfExport &e, QgsSymbolRenderContext &context ) const override;
    Qt::PenStyle dxfPenStyle() const override;
    QVariantMap properties() const override;

  protected:
    QBrush mBrush;
    Qgis::SymbolCoordinateReference mCoordinateReference = Qgis::SymbolCoordinateReference::Feature;
    double mNextAngle = 0.0; // mAngle / data defined angle

    //! Stroke width
    double mStrokeWidth = 0.0;
    QgsUnitTypes::RenderUnit mStrokeWidthUnit = QgsUnitTypes::RenderMillimeters;
    QgsMapUnitScale mStrokeWidthMapUnitScale;

    /**
     * Applies data defined settings prior to generating the fill symbol brush.
     */
    virtual void applyDataDefinedSettings( QgsSymbolRenderContext &context ) { Q_UNUSED( context ) }

    /**
     * Returns TRUE if the image brush should be transformed using the render context's texture origin.
     *
     * \since QGIS 3.16
     */
    virtual bool applyBrushTransformFromContext( QgsSymbolRenderContext *context = nullptr ) const;

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

    /**
     * Constructor for QgsRasterFillSymbolLayer, using a raster fill from the
     * specified \a imageFilePath.
     */
    QgsRasterFillSymbolLayer( const QString &imageFilePath = QString() );

    ~QgsRasterFillSymbolLayer() override;

    /**
     * Creates a new QgsRasterFillSymbolLayer from a \a properties map. The caller takes
     * ownership of the returned object.
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
    void renderPolygon( const QPolygonF &points, const QVector<QPolygonF> *rings, QgsSymbolRenderContext &context ) override;
    void startRender( QgsSymbolRenderContext &context ) override;
    void stopRender( QgsSymbolRenderContext &context ) override;
    QVariantMap properties() const override;
    QgsRasterFillSymbolLayer *clone() const override SIP_FACTORY;
    double estimateMaxBleed( const QgsRenderContext &context ) const override;
    bool usesMapUnits() const override;
    QColor color() const override;

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
    void setCoordinateMode( Qgis::SymbolCoordinateReference mode );

    /**
     * Coordinate mode for fill. Controls how the top left corner of the image
     * fill is positioned relative to the feature.
     * \returns coordinate mode
     * \see setCoordinateMode
     */
    Qgis::SymbolCoordinateReference coordinateMode() const { return mCoordinateMode; }

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
    bool applyBrushTransformFromContext( QgsSymbolRenderContext *context = nullptr ) const override;
  private:

    //! Path to the image file
    QString mImageFilePath;
    Qgis::SymbolCoordinateReference mCoordinateMode = Qgis::SymbolCoordinateReference::Feature;
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
 * \brief A class for filling symbols with a repeated SVG file.
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

    ~QgsSVGFillSymbolLayer() override;

    /**
     * Creates a new QgsSVGFillSymbolLayer from a \a properties map. The caller takes
     * ownership of the returned object.
     */
    static QgsSymbolLayer *create( const QVariantMap &properties = QVariantMap() ) SIP_FACTORY;

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
    static void resolvePaths( QVariantMap &properties, const QgsPathResolver &pathResolver, bool saving );

    // implemented from base classes

    QString layerType() const override;
    void startRender( QgsSymbolRenderContext &context ) override;
    void stopRender( QgsSymbolRenderContext &context ) override;
    void renderPolygon( const QPolygonF &points, const QVector<QPolygonF> *rings, QgsSymbolRenderContext &context ) override;
    QVariantMap properties() const override;
    QgsSVGFillSymbolLayer *clone() const override SIP_FACTORY;
    void toSld( QDomDocument &doc, QDomElement &element, const QVariantMap &props ) const override;
    bool usesMapUnits() const override;
    QgsSymbol *subSymbol() override;
    bool setSubSymbol( QgsSymbol *symbol SIP_TRANSFER ) override;
    double estimateMaxBleed( const QgsRenderContext &context ) const override;
    QColor dxfColor( QgsSymbolRenderContext &context ) const override;
    QSet<QString> usedAttributes( const QgsRenderContext &context ) const override;
    bool hasDataDefinedProperties() const override;

    /**
     * Sets the path to the SVG file to render in the fill.
     *
     * This is usually an absolute file path. Other supported options include
     *
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

  protected:

    void applyDataDefinedSettings( QgsSymbolRenderContext &context ) override;

  private:

    //! Width of the pattern (in output units)
    double mPatternWidth = 20;
    QgsUnitTypes::RenderUnit mPatternWidthUnit = QgsUnitTypes::RenderMillimeters;
    QgsMapUnitScale mPatternWidthMapUnitScale;
    QMap<QString, QgsProperty> mParameters;

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

    //! Custom stroke -- here for historic reasons only
    std::unique_ptr< QgsLineSymbol > mStroke;

    //! Helper function that gets the view box from the byte array
    void storeViewBox();
    void setDefaultSvgParams(); //fills mSvgFillColor, mSvgStrokeColor, mSvgStrokeWidth with default values for mSvgFilePath

    //! Applies the svg pattern to the brush
    void applyPattern( QBrush &brush, const QString &svgFilePath, double patternWidth, QgsUnitTypes::RenderUnit patternWidthUnit, const QColor &svgFillColor, const QColor &svgStrokeColor,
                       double svgStrokeWidth, QgsUnitTypes::RenderUnit svgStrokeWidthUnit, const QgsSymbolRenderContext &context, const QgsMapUnitScale &patternWidthMapUnitScale, const QgsMapUnitScale &svgStrokeWidthMapUnitScale,
                       const QgsStringMap svgParameters );
};

/**
 * \ingroup core
 * \class QgsLinePatternFillSymbolLayer
 * \brief A symbol fill consisting of repeated parallel lines.
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
    static QgsSymbolLayer *create( const QVariantMap &properties = QVariantMap() ) SIP_FACTORY;

    /**
     * Creates a new QgsLinePatternFillSymbolLayer from a SLD \a element. The caller takes
     * ownership of the returned object.
     */
    static QgsSymbolLayer *createFromSld( QDomElement &element ) SIP_FACTORY;

    QString layerType() const override;
    void startRender( QgsSymbolRenderContext &context ) override;
    void stopRender( QgsSymbolRenderContext &context ) override;
    void renderPolygon( const QPolygonF &points, const QVector<QPolygonF> *rings, QgsSymbolRenderContext &context ) override;
    QVariantMap properties() const override;
    QgsLinePatternFillSymbolLayer *clone() const override SIP_FACTORY;
    void toSld( QDomDocument &doc, QDomElement &element, const QVariantMap &props ) const override;
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

    /**
     * Returns the line clipping mode, which defines how lines are clipped at the edges of shapes.
     *
     * \see setClipMode()
     * \since QGIS 3.24
     */
    Qgis::LineClipMode clipMode() const { return mClipMode; }

    /**
     * Sets the line clipping \a mode, which defines how lines are clipped at the edges of shapes.
     *
     * \see clipMode()
     * \since QGIS 3.24
     */
    void setClipMode( Qgis::LineClipMode mode ) { mClipMode = mode; }

    void setOutputUnit( QgsUnitTypes::RenderUnit unit ) override;
    QgsUnitTypes::RenderUnit outputUnit() const override;
    bool usesMapUnits() const override;
    void setMapUnitScale( const QgsMapUnitScale &scale ) override;
    QgsMapUnitScale mapUnitScale() const override;
    bool setSubSymbol( QgsSymbol *symbol SIP_TRANSFER ) override;
    QgsSymbol *subSymbol() override;
    QSet<QString> usedAttributes( const QgsRenderContext &context ) const override;
    bool hasDataDefinedProperties() const override;
    void startFeatureRender( const QgsFeature &feature, QgsRenderContext &context ) override;
    void stopFeatureRender( const QgsFeature &feature, QgsRenderContext &context ) override;

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

    bool mRenderUsingLines = false;

#ifdef SIP_RUN
    QgsLinePatternFillSymbolLayer( const QgsLinePatternFillSymbolLayer &other );
#endif

    //! Applies the svg pattern to the brush
    void applyPattern( const QgsSymbolRenderContext &context, QBrush &brush, double lineAngle, double distance );

    //! Fill line
    std::unique_ptr< QgsLineSymbol > mFillLineSymbol;

    Qgis::LineClipMode mClipMode = Qgis::LineClipMode::ClipPainterOnly;
};

/**
 * \ingroup core
 * \class QgsPointPatternFillSymbolLayer
 * \brief A fill symbol layer which fills polygon shapes with repeating marker symbols.
 */
class CORE_EXPORT QgsPointPatternFillSymbolLayer: public QgsImageFillSymbolLayer
{
  public:
    QgsPointPatternFillSymbolLayer();
    ~QgsPointPatternFillSymbolLayer() override;

    /**
     * Creates a new QgsPointPatternFillSymbolLayer using the specified \a properties map containing symbol properties (see properties()).
     *
     * Caller takes ownership of the returned symbol layer.
     */
    static QgsSymbolLayer *create( const QVariantMap &properties = QVariantMap() ) SIP_FACTORY;
    static QgsSymbolLayer *createFromSld( QDomElement &element ) SIP_FACTORY;

    QString layerType() const override;
    void startRender( QgsSymbolRenderContext &context ) override;
    void stopRender( QgsSymbolRenderContext &context ) override;
    void startFeatureRender( const QgsFeature &feature, QgsRenderContext &context ) override;
    void stopFeatureRender( const QgsFeature &feature, QgsRenderContext &context ) override;
    void renderPolygon( const QPolygonF &points, const QVector<QPolygonF> *rings, QgsSymbolRenderContext &context ) override;
    QVariantMap properties() const override;
    QgsPointPatternFillSymbolLayer *clone() const override SIP_FACTORY;
    void toSld( QDomDocument &doc, QDomElement &element, const QVariantMap &props ) const override;
    double estimateMaxBleed( const QgsRenderContext &context ) const override;
    bool setSubSymbol( QgsSymbol *symbol SIP_TRANSFER ) override;
    QgsSymbol *subSymbol() override;
    void setOutputUnit( QgsUnitTypes::RenderUnit unit ) override;
    QgsUnitTypes::RenderUnit outputUnit() const override;
    bool usesMapUnits() const override;
    void setMapUnitScale( const QgsMapUnitScale &scale ) override;
    QgsMapUnitScale mapUnitScale() const override;
    QSet<QString> usedAttributes( const QgsRenderContext &context ) const override;
    bool hasDataDefinedProperties() const override;
    void setColor( const QColor &c ) override;
    QColor color() const override;

    /**
     * Returns the horizontal distance between rendered markers in the fill.
     *
     * Units are retrieved via distanceXUnit().
     *
     * \see setDistanceX()
     * \see distanceXUnit()
     * \see distanceXMapUnitScale()
     */
    double distanceX() const { return mDistanceX; }

    /**
     * Sets the horizontal distance between rendered markers in the fill.
     *
     * Units are set via setDistanceXUnit().
     *
     * \see distanceX()
     * \see setDistanceXUnit()
     * \see setDistanceXMapUnitScale()
     */
    void setDistanceX( double d ) { mDistanceX = d; }

    /**
     * Sets the \a unit for the horizontal distance between points in the pattern.
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

    /**
     * Sets the map unit \a scale for the horizontal distance between points in the pattern.
     *
     * \see distanceXMapUnitScale()
     * \see setDistanceX()
     */
    void setDistanceXMapUnitScale( const QgsMapUnitScale &scale ) { mDistanceXMapUnitScale = scale; }

    /**
     * Returns the map unit scale for the horizontal distance between points in the pattern.
     *
     * \see setDistanceXMapUnitScale()
     * \see distanceX()
     */
    const QgsMapUnitScale &distanceXMapUnitScale() const { return mDistanceXMapUnitScale; }

    /**
     * Returns the vertical distance between rendered markers in the fill.
     *
     * Units are retrieved via distanceYUnit().
     *
     * \see setDistanceY()
     * \see distanceYUnit()
     * \see distanceYMapUnitScale()
     */
    double distanceY() const { return mDistanceY; }

    /**
     * Sets the vertical distance between rendered markers in the fill.
     *
     * Units are set via setDistanceYUnit().
     *
     * \see distanceY()
     * \see setDistanceYUnit()
     * \see setDistanceYMapUnitScale()
     */
    void setDistanceY( double d ) { mDistanceY = d; }

    /**
     * Sets the \a unit for the vertical distance between points in the pattern.
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

    /**
     * Sets the map unit \a scale for the vertical distance between points in the pattern.
     *
     * \see distanceYMapUnitScale()
     * \see setDistanceY()
     */
    void setDistanceYMapUnitScale( const QgsMapUnitScale &scale ) { mDistanceYMapUnitScale = scale; }

    /**
     * Returns the map unit scale for the vertical distance between points in the pattern.
     *
     * \see setDistanceYMapUnitScale()
     * \see distanceY()
     */
    const QgsMapUnitScale &distanceYMapUnitScale() const { return mDistanceYMapUnitScale; }

    /**
     * Returns the horizontal displacement for odd numbered rows in the pattern.
     *
     * Units are retrieved via displacementXUnit().
     *
     * \see setDisplacementX()
     * \see displacementXUnit()
     * \see displacementXMapUnitScale()
     */
    double displacementX() const { return mDisplacementX; }

    /**
     * Sets the horizontal displacement for odd numbered rows in the pattern.
     *
     * Units are set via setDisplacementXUnit().
     *
     * \see displacementX()
     * \see setDisplacementXUnit()
     * \see setDisplacementXMapUnitScale()
     */
    void setDisplacementX( double d ) { mDisplacementX = d; }

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

    /**
     * Sets the map unit \a scale for the horizontal displacement between odd numbered rows in the pattern.
     *
     * \see displacementXMapUnitScale()
     * \see setDisplacementX()
     */
    void setDisplacementXMapUnitScale( const QgsMapUnitScale &scale ) { mDisplacementXMapUnitScale = scale; }

    /**
     * Returns the map unit scale for the horizontal displacement between odd numbered rows in the pattern.
     *
     * \see setDisplacementXMapUnitScale()
     * \see displacementX()
     */
    const QgsMapUnitScale &displacementXMapUnitScale() const { return mDisplacementXMapUnitScale; }

    /**
     * Returns the vertical displacement for odd numbered columns in the pattern.
     *
     * Units are retrieved via displacementYUnit().
     *
     * \see setDisplacementY()
     * \see displacementYUnit()
     * \see displacementYMapUnitScale()
     */
    double displacementY() const { return mDisplacementY; }

    /**
     * Sets the vertical displacement for odd numbered columns in the pattern.
     *
     * Units are set via setDisplacementYUnit().
     *
     * \see displacementY()
     * \see setDisplacementYUnit()
     * \see setDisplacementYMapUnitScale()
     */
    void setDisplacementY( double d ) { mDisplacementY = d; }

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

    /**
     * Sets the map unit \a scale for the vertical displacement between odd numbered columns in the pattern.
     *
     * \see displacementYMapUnitScale()
     * \see setDisplacementY()
     */
    void setDisplacementYMapUnitScale( const QgsMapUnitScale &scale ) { mDisplacementYMapUnitScale = scale; }

    /**
     * Returns the map unit scale for the vertical displacement between odd numbered columns in the pattern.
     *
     * \see setDisplacementYMapUnitScale()
     * \see displacementY()
     */
    const QgsMapUnitScale &displacementYMapUnitScale() const { return mDisplacementYMapUnitScale; }

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

    /**
     * Returns the marker clipping mode, which defines how markers are clipped at the edges of shapes.
     *
     * \see setClipMode()
     * \since QGIS 3.24
     */
    Qgis::MarkerClipMode clipMode() const { return mClipMode; }

    /**
     * Sets the marker clipping \a mode, which defines how markers are clipped at the edges of shapes.
     *
     * \see clipMode()
     * \since QGIS 3.24
     */
    void setClipMode( Qgis::MarkerClipMode mode ) { mClipMode = mode; }

    /**
     * Sets the maximum horizontal random \a deviation of points in the pattern.
     *
     * Units are set via setMaximumRandomDeviationXUnit().
     *
     * \see maximumRandomDeviationX()
     * \see setMaximumRandomDeviationY()
     * \since QGIS 3.24
    */
    void setMaximumRandomDeviationX( double deviation ) { mRandomDeviationX = deviation; }

    /**
     * Returns the maximum horizontal random deviation of points in the pattern.
     *
     * Units are retrieved via maximumRandomDeviationXUnit().
     *
     * \see setMaximumRandomDeviationX()
     * \see maximumRandomDeviationY()
     * \since QGIS 3.24
    */
    double maximumRandomDeviationX() const { return mRandomDeviationX; }

    /**
     * Sets the maximum vertical random \a deviation of points in the pattern.
     *
     * Units are set via setMaximumRandomDeviationYUnit().
     *
     * \see maximumRandomDeviationY()
     * \see setMaximumRandomDeviationX()
     * \since QGIS 3.24
    */
    void setMaximumRandomDeviationY( double deviation ) { mRandomDeviationY = deviation; }

    /**
     * Returns the maximum vertical random deviation of points in the pattern.
     *
     * Units are retrieved via maximumRandomDeviationYUnit().
     *
     * \see setMaximumRandomDeviationY()
     * \see maximumRandomDeviationX()
     * \since QGIS 3.24
    */
    double maximumRandomDeviationY() const { return mRandomDeviationY; }

    /**
     * Sets the \a unit for the horizontal random deviation of points in the pattern.
     *
     * \see randomDeviationXUnit()
     * \see setRandomDeviationYUnit()
     * \since QGIS 3.24
    */
    void setRandomDeviationXUnit( QgsUnitTypes::RenderUnit unit ) { mRandomDeviationXUnit = unit; }

    /**
     * Returns the units for the horizontal random deviation of points in the pattern.
     * \see setRandomDeviationXUnit()
     * \see randomDeviationYUnit()
     *
     * \since QGIS 3.24
    */
    QgsUnitTypes::RenderUnit randomDeviationXUnit() const { return mRandomDeviationXUnit; }

    /**
     * Sets the \a unit for the vertical random deviation of points in the pattern.
     *
     * \see randomDeviationYUnit()
     * \see setRandomDeviationXUnit()
     * \since QGIS 3.24
    */
    void setRandomDeviationYUnit( QgsUnitTypes::RenderUnit unit ) { mRandomDeviationYUnit = unit; }

    /**
     * Returns the units for the vertical random deviation of points in the pattern.
     *
     * \see setRandomDeviationYUnit()
     * \see randomDeviationXUnit()
     *
     * \since QGIS 3.24
    */
    QgsUnitTypes::RenderUnit randomDeviationYUnit() const { return mRandomDeviationYUnit; }

    /**
     * Returns the unit scale for the horizontal random deviation of points in the pattern.
     *
     * \see setRandomDeviationXMapUnitScale()
     * \see randomDeviationYMapUnitScale()
     * \since QGIS 3.24
    */
    const QgsMapUnitScale &randomDeviationXMapUnitScale() const { return mRandomDeviationXMapUnitScale; }

    /**
     * Returns the unit scale for the vertical random deviation of points in the pattern.
     *
     * \see setRandomDeviationXMapUnitScale()
     * \see randomDeviationXMapUnitScale()
     * \since QGIS 3.24
    */
    const QgsMapUnitScale &randomDeviationYMapUnitScale() const { return mRandomDeviationYMapUnitScale; }

    /**
     * Sets the unit \a scale for the horizontal random deviation of points in the pattern.
     *
     * \see randomDeviationXMapUnitScale()
     * \see setRandomDeviationYMapUnitScale()
     * \since QGIS 3.24
    */
    void setRandomDeviationXMapUnitScale( const QgsMapUnitScale &scale ) { mRandomDeviationXMapUnitScale = scale; }

    /**
     * Sets the unit \a scale for the vertical random deviation of points in the pattern.
     *
     * \see randomDeviationYMapUnitScale()
     * \see setRandomDeviationXMapUnitScale()
     * \since QGIS 3.24
    */
    void setRandomDeviationYMapUnitScale( const QgsMapUnitScale &scale ) { mRandomDeviationYMapUnitScale = scale; }

    /**
     * Returns the random number seed to use when randomly shifting points, or 0 if
     * a truly random sequence will be used (causing points to appear in different locations with every map refresh).
     * \see setSeed()
     * \since QGIS 3.24
     */
    unsigned long seed() const { return mSeed; }

    /**
     * Sets the random number \a seed to use when randomly shifting points, or 0 if
     * a truly random sequence will be used on every rendering (causing points to appear
     * in different locations with every map refresh).
     *
     * \see seed()
     * \since QGIS 3.24
     */
    void setSeed( unsigned long seed ) { mSeed = seed; }

    /**
     * Returns the rotation angle of the pattern, in degrees clockwise.
     *
     * \see setAngle()
     * \since QGIS 3.24
     */
    double angle() const { return mAngle; }

    /**
     * Sets the rotation \a angle of the pattern, in degrees clockwise.
     *
     * \see angle()
     * \since QGIS 3.24
     */
    void setAngle( double angle ) { mAngle = angle; }

  protected:
    std::unique_ptr< QgsMarkerSymbol > mMarkerSymbol;
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

    double mRandomDeviationX = 0;
    QgsUnitTypes::RenderUnit mRandomDeviationXUnit = QgsUnitTypes::RenderMillimeters;
    QgsMapUnitScale mRandomDeviationXMapUnitScale;
    double mRandomDeviationY = 0;
    QgsUnitTypes::RenderUnit mRandomDeviationYUnit = QgsUnitTypes::RenderMillimeters;
    QgsMapUnitScale mRandomDeviationYMapUnitScale;
    unsigned long mSeed = 0;

    double mAngle = 0;

    void applyDataDefinedSettings( QgsSymbolRenderContext &context ) override;

  private:
#ifdef SIP_RUN
    QgsPointPatternFillSymbolLayer( const QgsPointPatternFillSymbolLayer &other );
#endif

    void applyPattern( const QgsSymbolRenderContext &context, QBrush &brush, double distanceX, double distanceY,
                       double displacementX, double displacementY, double offsetX, double offsetY );

    Qgis::MarkerClipMode mClipMode = Qgis::MarkerClipMode::Shape;

    bool mRenderUsingMarkers = false;
};

/**
 * \ingroup core
 * \class QgsRandomMarkerFillSymbolLayer
 *
 * \brief A fill symbol layer which places markers at random locations within polygons.
 *
 * \since QGIS 3.12
 */
class CORE_EXPORT QgsRandomMarkerFillSymbolLayer : public QgsFillSymbolLayer
{
  public:

    /**
     * Constructor for QgsRandomMarkerFillSymbolLayer, with the specified \a pointCount.
     *
     * Optionally a specific random number \a seed can be used when generating points. A \a seed of 0 indicates that
     * a truly random sequence will be used on every rendering, causing points to appear in different locations with every map refresh.
     */
    QgsRandomMarkerFillSymbolLayer( int pointCount = 10, Qgis::PointCountMethod method = Qgis::PointCountMethod::Absolute, double densityArea = 250.0, unsigned long seed = 0 );

    ~QgsRandomMarkerFillSymbolLayer() override;

    /**
     * Creates a new QgsRandomMarkerFillSymbolLayer using the specified \a properties map containing symbol properties (see properties()).
     *
     * Caller takes ownership of the returned symbol layer.
     */
    static QgsSymbolLayer *create( const QVariantMap &properties = QVariantMap() ) SIP_FACTORY;

    QString layerType() const override;
    void startRender( QgsSymbolRenderContext &context ) override;
    void stopRender( QgsSymbolRenderContext &context ) override;
    void renderPolygon( const QPolygonF &points, const QVector<QPolygonF> *rings, QgsSymbolRenderContext &context ) override;
    QVariantMap properties() const override;
    QgsRandomMarkerFillSymbolLayer *clone() const override SIP_FACTORY;
    bool canCauseArtifactsBetweenAdjacentTiles() const override;

    void setColor( const QColor &color ) override;
    QColor color() const override;

    QgsSymbol *subSymbol() override;
    bool setSubSymbol( QgsSymbol *symbol SIP_TRANSFER ) override;

    void setOutputUnit( QgsUnitTypes::RenderUnit unit ) override;
    QgsUnitTypes::RenderUnit outputUnit() const override;
    bool usesMapUnits() const override;

    void setMapUnitScale( const QgsMapUnitScale &scale ) override;
    QgsMapUnitScale mapUnitScale() const override;

    QSet<QString> usedAttributes( const QgsRenderContext &context ) const override;
    bool hasDataDefinedProperties() const override;

    /**
     * Returns the count of random points to render in the fill.
     *
     * \see setPointCount()
     */
    int pointCount() const;

    /**
     * Sets the \a count of random points to render in the fill.
     *
     * \see pointCount()
     */
    void setPointCount( int count );

    /**
     * Returns the random number seed to use when generating points, or 0 if
     * a truly random sequence will be used (causing points to appear in different locations with every map refresh).
     * \see setSeed()
     */
    unsigned long seed() const;

    /**
     * Sets the random number \a seed to use when generating points, or 0 if
     * a truly random sequence will be used on every rendering (causing points to appear
     * in different locations with every map refresh).
     *
     * \see seed()
     */
    void setSeed( unsigned long seed );

    /**
     * Returns TRUE if point markers should be clipped to the polygon boundary.
     *
     * \see setClipPoints()
     */
    bool clipPoints() const;

    /**
     * Sets whether point markers should be \a clipped to the polygon boundary.
     *
     * \see clipPoints()
     */
    void setClipPoints( bool clipped );

    /**
     * Returns the count method used to randomly fill the polygon.
     *
     * \see setCountMethod()
     */
    Qgis::PointCountMethod countMethod() const;

    /**
     * Sets the count \a method used to randomly fill the polygon.
     *
     * \see countMethod()
     */
    void setCountMethod( Qgis::PointCountMethod method );

    /**
     * Returns the density area used to count the number of points to randomly fill the polygon.
     *
     * Only used when the count method is set to QgsRandomMarkerFillSymbolLayer::DensityBasedCount.
     *
     * Units are specified by setDensityAreaUnit().
     *
     * \see setDensityArea()
     */
    double densityArea() const;

    /**
     * Sets the density \a area used to count the number of points to randomly fill the polygon.
     *
     * \see densityArea()
     */
    void setDensityArea( double area );

    /**
     * Sets the units for the density area.
     * \param unit width units
     * \see densityAreaUnit()
    */
    void setDensityAreaUnit( QgsUnitTypes::RenderUnit unit ) { mDensityAreaUnit = unit; }

    /**
     * Returns the units for the density area.
     * \see setDensityAreaUnit()
    */
    QgsUnitTypes::RenderUnit densityAreaUnit() const { return mDensityAreaUnit; }

    /**
     * Sets the map scale for the density area.
     * \param scale density area map unit scale
     * \see densityAreaUnitScale()
     * \see setDensityArea()
     * \see setDensityAreaUnit()
     */
    void setDensityAreaUnitScale( const QgsMapUnitScale &scale ) { mDensityAreaUnitScale = scale; }

    /**
     * Returns the map scale for the density area.
     * \see setDensityAreaUnitScale()
     * \see densityArea()
     * \see densityAreaUnit()
     */
    const QgsMapUnitScale &densityAreaUnitScale() const { return mDensityAreaUnitScale; }

    void startFeatureRender( const QgsFeature &feature, QgsRenderContext &context ) override;
    void stopFeatureRender( const QgsFeature &feature, QgsRenderContext &context ) override;

  private:
#ifdef SIP_RUN
    QgsRandomMarkerFillSymbolLayer( const QgsRandomMarkerFillSymbolLayer &other );
#endif

    struct Part
    {
      QPolygonF exterior;
      QVector<QPolygonF> rings;
    };

    QVector< Part > mCurrentParts;

    void render( QgsRenderContext &context, const QVector< Part > &parts, const QgsFeature &feature, bool selected );

    std::unique_ptr< QgsMarkerSymbol > mMarker;
    Qgis::PointCountMethod mCountMethod = Qgis::PointCountMethod::Absolute;
    int mPointCount = 10;
    double mDensityArea = 250.0;
    QgsUnitTypes::RenderUnit mDensityAreaUnit = QgsUnitTypes::RenderMillimeters;
    QgsMapUnitScale mDensityAreaUnitScale;
    unsigned long mSeed = 0;
    bool mClipPoints = false;

    bool mRenderingFeature = false;
    double mFeatureSymbolOpacity = 1;
};


/**
 * \ingroup core
 * \class QgsCentroidFillSymbolLayer
 */
class CORE_EXPORT QgsCentroidFillSymbolLayer : public QgsFillSymbolLayer
{
  public:
    QgsCentroidFillSymbolLayer();
    ~QgsCentroidFillSymbolLayer() override;

    // static stuff

    /**
     * Creates a new QgsCentroidFillSymbolLayer using the specified \a properties map containing symbol properties (see properties()).
     *
     * Caller takes ownership of the returned symbol layer.
     */
    static QgsSymbolLayer *create( const QVariantMap &properties = QVariantMap() ) SIP_FACTORY;
    static QgsSymbolLayer *createFromSld( QDomElement &element ) SIP_FACTORY;

    // implemented from base classes

    QString layerType() const override;
    void startRender( QgsSymbolRenderContext &context ) override;
    void stopRender( QgsSymbolRenderContext &context ) override;
    void renderPolygon( const QPolygonF &points, const QVector<QPolygonF> *rings, QgsSymbolRenderContext &context ) override;
    QVariantMap properties() const override;
    QgsCentroidFillSymbolLayer *clone() const override SIP_FACTORY;
    void toSld( QDomDocument &doc, QDomElement &element, const QVariantMap &props ) const override;
    void setColor( const QColor &color ) override;
    QColor color() const override;
    QgsSymbol *subSymbol() override;
    bool setSubSymbol( QgsSymbol *symbol SIP_TRANSFER ) override;
    void setOutputUnit( QgsUnitTypes::RenderUnit unit ) override;
    QgsUnitTypes::RenderUnit outputUnit() const override;
    bool usesMapUnits() const override;
    void setMapUnitScale( const QgsMapUnitScale &scale ) override;
    QgsMapUnitScale mapUnitScale() const override;
    QSet<QString> usedAttributes( const QgsRenderContext &context ) const override;
    bool hasDataDefinedProperties() const override;
    bool canCauseArtifactsBetweenAdjacentTiles() const override;

    void setPointOnSurface( bool pointOnSurface ) { mPointOnSurface = pointOnSurface; }
    bool pointOnSurface() const { return mPointOnSurface; }

    /**
     * Sets whether a point is drawn for all parts or only on the biggest part of multi-part features.
     * \see pointOnAllParts()
     * \since QGIS 2.16
    */
    void setPointOnAllParts( bool pointOnAllParts ) { mPointOnAllParts = pointOnAllParts; }

    /**
     * Returns whether a point is drawn for all parts or only on the biggest part of multi-part features.
     * \see setPointOnAllParts()
     * \since QGIS 2.16
    */
    bool pointOnAllParts() const { return mPointOnAllParts; }

    /**
     * Returns TRUE if point markers should be clipped to the polygon boundary.
     *
     * \see setClipPoints()
     * \since 3.14
     */
    bool clipPoints() const { return mClipPoints; }

    /**
     * Sets whether point markers should be \a clipped to the polygon boundary.
     *
     * \see clipPoints()
     * \since 3.14
     */
    void setClipPoints( bool clipPoints ) { mClipPoints = clipPoints; }

    /**
     * Returns TRUE if point markers should be clipped to the current part boundary only.
     *
     * \see setClipPoints()
     * \since 3.14
     */
    bool clipOnCurrentPartOnly() const { return mClipOnCurrentPartOnly; }

    /**
     * Sets whether point markers should be \a clipped to the current part boundary only.
     *
     * \see clipOnCurrentPartOnly()
     * \since 3.14
     */
    void setClipOnCurrentPartOnly( bool clipOnCurrentPartOnly ) { mClipOnCurrentPartOnly = clipOnCurrentPartOnly; }

    void startFeatureRender( const QgsFeature &feature, QgsRenderContext &context ) override;
    void stopFeatureRender( const QgsFeature &feature, QgsRenderContext &context ) override;

  protected:

    std::unique_ptr< QgsMarkerSymbol > mMarker;
    bool mPointOnSurface = false;
    bool mPointOnAllParts = true;
    bool mClipPoints = false;
    bool mClipOnCurrentPartOnly = false;

    bool mRenderingFeature = false;
    double mFeatureSymbolOpacity = 1;

  private:
#ifdef SIP_RUN
    QgsCentroidFillSymbolLayer( const QgsCentroidFillSymbolLayer &other );
#endif
    struct Part
    {
      QPolygonF exterior;
      QVector<QPolygonF> rings;
    };

    void render( QgsRenderContext &context, const QVector<Part> &parts, const QgsFeature &feature, bool selected );
    QVector<Part> mCurrentParts;
};

#endif


