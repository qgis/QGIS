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

#ifndef QGSFILLSYMBOLLAYERV2_H
#define QGSFILLSYMBOLLAYERV2_H

#include "qgis_core.h"
#include "qgis.h"
#include "qgssymbollayer.h"

#define DEFAULT_SIMPLEFILL_COLOR        QColor(0,0,255)
#define DEFAULT_SIMPLEFILL_STYLE        Qt::SolidPattern
#define DEFAULT_SIMPLEFILL_BORDERCOLOR  QColor(0,0,0)
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

    virtual ~QgsGradientFillSymbolLayer();

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
    void setColorRamp( QgsColorRamp *ramp );

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
                        QPointF referencePoint1, QPointF referencePoint2, const double angle );

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

    virtual ~QgsShapeburstFillSymbolLayer();

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
     * \since QGIS 2.3
     * \see blurRadius
     */
    void setBlurRadius( int blurRadius ) { mBlurRadius = blurRadius; }

    /**
     * Returns the blur radius, which controls the amount of blurring applied to the fill.
     * \returns Integer representing the radius for fill blur. Higher values indicate a stronger blur. A 0 value indicates that blurring is disabled.
     * \since QGIS 2.3
     * \see setBlurRadius
     */
    int blurRadius() const { return mBlurRadius; }

    /**
     * Sets whether the shapeburst fill should be drawn using the entire shape.
     * \param useWholeShape Set to true if shapeburst should cover entire shape. If false, setMaxDistance is used to calculate how far from the boundary of the shape should
     * be shaded
     * \since QGIS 2.3
     * \see useWholeShape
     * \see setMaxDistance
     */
    void setUseWholeShape( bool useWholeShape ) { mUseWholeShape = useWholeShape; }

    /**
     * Returns whether the shapeburst fill is set to cover the entire shape.
     * \returns True if shapeburst fill will cover the entire shape. If false, shapeburst is drawn to a distance of maxDistance from the polygon's boundary.
     * \since QGIS 2.3
     * \see setUseWholeShape
     * \see maxDistance
     */
    bool useWholeShape() const { return mUseWholeShape; }

    /**
     * Sets the maximum distance to shape inside of the shape from the polygon's boundary.
     * \param maxDistance distance from boundary to shade. setUseWholeShape must be set to false for this parameter to take effect. Distance unit is controlled by setDistanceUnit.
     * \since QGIS 2.3
     * \see maxDistance
     * \see setUseWholeShape
     * \see setDistanceUnit
     */
    void setMaxDistance( double maxDistance ) { mMaxDistance = maxDistance; }

    /**
     * Returns the maximum distance from the shape's boundary which is shaded. This parameter is only effective if useWholeShape is false.
     * \returns the maximum distance from the polygon's boundary which is shaded. Distance units are indicated by distanceUnit.
     * \since QGIS 2.3
     * \see useWholeShape
     * \see setMaxDistance
     * \see distanceUnit
     */
    double maxDistance() const { return mMaxDistance; }

    /**
     * Sets the unit for the maximum distance to shade inside of the shape from the polygon's boundary.
     * \param unit distance unit for the maximum distance
     * \since QGIS 2.3
     * \see setMaxDistance
     * \see distanceUnit
     */
    void setDistanceUnit( QgsUnitTypes::RenderUnit unit ) { mDistanceUnit = unit; }

    /**
     * Returns the unit for the maximum distance to shade inside of the shape from the polygon's boundary.
     * \returns distance unit for the maximum distance
     * \since QGIS 2.3
     * \see maxDistance
     * \see setDistanceUnit
     */
    QgsUnitTypes::RenderUnit distanceUnit() const { return mDistanceUnit; }

    void setDistanceMapUnitScale( const QgsMapUnitScale &scale ) { mDistanceMapUnitScale = scale; }
    const QgsMapUnitScale &distanceMapUnitScale() const { return mDistanceMapUnitScale; }

    /**
     * Sets the color mode to use for the shapeburst fill. Shapeburst can either be drawn using a QgsColorRamp color ramp
     * or by simply specificing a start and end color. setColorType is used to specify which mode to use for the fill.
     * \param colorType color type to use for shapeburst fill
     * \since QGIS 2.3
     * \see colorType
     * \see setColor
     * \see setColor2
     * \see setColorRamp
     */
    void setColorType( ShapeburstColorType colorType ) { mColorType = colorType; }

    /**
     * Returns the color mode used for the shapeburst fill. Shapeburst can either be drawn using a QgsColorRamp color ramp
     * or by simply specificing a start and end color.
     * \returns current color mode used for the shapeburst fill
     * \since QGIS 2.3
     * \see setColorType
     * \see color
     * \see color2
     * \see colorRamp
     */
    ShapeburstColorType colorType() const { return mColorType; }

    /**
     * Sets the color ramp used to draw the shapeburst fill. Color ramps are only used if setColorType is set ShapeburstColorType::ColorRamp.
     * \param ramp color ramp to use for shapeburst fill
     * \since QGIS 2.3
     * \see setColorType
     * \see colorRamp
     */
    void setColorRamp( QgsColorRamp *ramp );

    /**
     * Returns the color ramp used for the shapeburst fill. The color ramp is only used if the colorType is set to ShapeburstColorType::ColorRamp
     * \returns a QgsColorRamp color ramp
     * \since QGIS 2.3
     * \see setColorRamp
     * \see colorType
     */
    QgsColorRamp *colorRamp() { return mGradientRamp; }

    /**
     * Sets the color for the endpoint of the shapeburst fill. This color is only used if setColorType is set ShapeburstColorType::SimpleTwoColor.
     * \param color2 QColor to use for endpoint of gradient
     * \since QGIS 2.3
     * \see setColorType
     * \see color2
     */
    void setColor2( const QColor &color2 ) { mColor2 = color2; }

    /**
     * Returns the color used for the endpoint of the shapeburst fill. This color is only used if the colorType is set to ShapeburstColorType::SimpleTwoColor
     * \returns a QColor indicating the color of the endpoint of the gradient
     * \since QGIS 2.3
     * \see setColor2
     * \see colorType
     */
    QColor color2() const { return mColor2; }

    /**
     * Sets whether the shapeburst fill should ignore polygon rings when calculating
     * the buffered shading.
     * \param ignoreRings Set to true if buffers should ignore interior rings for polygons.
     * \since QGIS 2.3
     * \see ignoreRings
     */
    void setIgnoreRings( bool ignoreRings ) { mIgnoreRings = ignoreRings; }

    /**
     * Returns whether the shapeburst fill is set to ignore polygon interior rings.
     * \returns True if the shapeburst fill will ignore interior rings when calculating buffered shading.
     * \since QGIS 2.3
     * \see setIgnoreRings
     */
    bool ignoreRings() const { return mIgnoreRings; }

    /**
     * Sets the offset for the shapeburst fill.
     * \param offset QPointF indicating the horizontal/vertical offset amount
     * \since QGIS 2.3
     * \see offset
     * \see setOffsetUnit
     */
    void setOffset( QPointF offset ) { mOffset = offset; }

    /**
     * Returns the offset for the shapeburst fill.
     * \returns a QPointF indicating the horizontal/vertical offset amount
     * \since QGIS 2.3
     * \see setOffset
     * \see offsetUnit
     */
    QPointF offset() const { return mOffset; }

    /**
     * Sets the units used for the offset for the shapeburst fill.
     * \param unit units for fill offset
     * \since QGIS 2.3
     * \see setOffset
     * \see offsetUnit
     */
    void setOffsetUnit( QgsUnitTypes::RenderUnit unit ) { mOffsetUnit = unit; }

    /**
     * Returns the units used for the offset of the shapeburst fill.
     * \returns units used for the fill offset
     * \since QGIS 2.3
     * \see offset
     * \see setOffsetUnit
     */
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

    int mBlurRadius;

    bool mUseWholeShape;
    double mMaxDistance;
    QgsUnitTypes::RenderUnit mDistanceUnit = QgsUnitTypes::RenderMillimeters;
    QgsMapUnitScale mDistanceMapUnitScale;

    ShapeburstColorType mColorType;
    QColor mColor2;
    QgsColorRamp *mGradientRamp = nullptr;
    QgsColorRamp *mTwoColorGradientRamp = nullptr;

    bool mIgnoreRings = false;

    QPointF mOffset;
    QgsUnitTypes::RenderUnit mOffsetUnit = QgsUnitTypes::RenderMillimeters;
    QgsMapUnitScale mOffsetMapUnitScale;

  private:

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
};

/**
 * \ingroup core
 * Base class for polygon renderers generating texture images*/
class CORE_EXPORT QgsImageFillSymbolLayer: public QgsFillSymbolLayer
{
  public:

    QgsImageFillSymbolLayer();
    void renderPolygon( const QPolygonF &points, QList<QPolygonF> *rings, QgsSymbolRenderContext &context ) override;

    virtual QgsSymbol *subSymbol() override { return mStroke.get(); }
    virtual bool setSubSymbol( QgsSymbol *symbol SIP_TRANSFER ) override;

    /**
     * Sets the units for the symbol's stroke width.
     * \param unit symbol units
     * \see strokeWidthUnit()
    */
    void setStrokeWidthUnit( QgsUnitTypes::RenderUnit unit ) { mStrokeWidthUnit = unit; }

    /**
     * Returns the units for the symbol's stroke width.
     * \see setStrokeWidthUnit()
    */
    QgsUnitTypes::RenderUnit strokeWidthUnit() const { return mStrokeWidthUnit; }

    void setStrokeWidthMapUnitScale( const QgsMapUnitScale &scale ) { mStrokeWidthMapUnitScale = scale; }
    const QgsMapUnitScale &strokeWidthMapUnitScale() const { return mStrokeWidthMapUnitScale; }

    void setOutputUnit( QgsUnitTypes::RenderUnit unit ) override;
    QgsUnitTypes::RenderUnit outputUnit() const override;

    void setMapUnitScale( const QgsMapUnitScale &scale ) override;
    QgsMapUnitScale mapUnitScale() const override;

    virtual double estimateMaxBleed( const QgsRenderContext &context ) const override;

    double dxfWidth( const QgsDxfExport &e, QgsSymbolRenderContext &context ) const override;
    QColor dxfColor( QgsSymbolRenderContext &context ) const override;

    Qt::PenStyle dxfPenStyle() const override;

    QSet<QString> usedAttributes( const QgsRenderContext &context ) const override;

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

    enum FillCoordinateMode
    {
      Feature,
      Viewport
    };

    QgsRasterFillSymbolLayer( const QString &imageFilePath = QString() );

    static QgsSymbolLayer *create( const QgsStringMap &properties = QgsStringMap() ) SIP_FACTORY;

    // implemented from base classes
    QString layerType() const override;
    void renderPolygon( const QPolygonF &points, QList<QPolygonF> *rings, QgsSymbolRenderContext &context ) override;
    void startRender( QgsSymbolRenderContext &context ) override;
    void stopRender( QgsSymbolRenderContext &context ) override;
    QgsStringMap properties() const override;
    QgsRasterFillSymbolLayer *clone() const override SIP_FACTORY;
    virtual double estimateMaxBleed( const QgsRenderContext &context ) const override;

    //override QgsImageFillSymbolLayer's support for sub symbols
    virtual QgsSymbol *subSymbol() override { return nullptr; }
    virtual bool setSubSymbol( QgsSymbol *symbol SIP_TRANSFER ) override;

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
    void setCoordinateMode( const FillCoordinateMode mode );

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
    void setOpacity( const double opacity );

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

    void applyDataDefinedSettings( QgsSymbolRenderContext &context ) override;

  private:

    //! Applies the image pattern to the brush
    void applyPattern( QBrush &brush, const QString &imageFilePath, const double width, const double opacity,
                       const QgsSymbolRenderContext &context );
};

/**
 * \ingroup core
 * A class for svg fill patterns. The class automatically scales the pattern to
   the appropriate pixel dimensions of the output device*/
class CORE_EXPORT QgsSVGFillSymbolLayer: public QgsImageFillSymbolLayer
{
  public:
    //! Constructs SVG fill symbol layer with picture from given absolute path to a SVG file
    QgsSVGFillSymbolLayer( const QString &svgFilePath, double width = 20, double rotation = 0.0 );
    QgsSVGFillSymbolLayer( const QByteArray &svgData, double width = 20, double rotation = 0.0 );
    ~QgsSVGFillSymbolLayer();

    static QgsSymbolLayer *create( const QgsStringMap &properties = QgsStringMap() ) SIP_FACTORY;
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

    //getters and setters
    void setSvgFilePath( const QString &svgPath );
    QString svgFilePath() const { return mSvgFilePath; }
    void setPatternWidth( double width ) { mPatternWidth = width;}
    double patternWidth() const { return mPatternWidth; }

    void setSvgFillColor( const QColor &c ) { setColor( c );  }
    QColor svgFillColor() const { return color(); }

    void setSvgStrokeColor( const QColor &c ) { mSvgStrokeColor = c; }
    QColor svgStrokeColor() const { return mSvgStrokeColor; }
    void setSvgStrokeWidth( double w ) { mSvgStrokeWidth = w; }
    double svgStrokeWidth() const { return mSvgStrokeWidth; }

    /**
     * Sets the units for the width of the SVG images in the pattern.
     * \param unit width units
     * \see patternWidthUnit()
    */
    void setPatternWidthUnit( QgsUnitTypes::RenderUnit unit ) { mPatternWidthUnit = unit; }

    /**
     * Returns the units for the width of the SVG images in the pattern.
     * \see setPatternWidthUnit()
    */
    QgsUnitTypes::RenderUnit patternWidthUnit() const { return mPatternWidthUnit; }

    void setPatternWidthMapUnitScale( const QgsMapUnitScale &scale ) { mPatternWidthMapUnitScale = scale; }
    const QgsMapUnitScale &patternWidthMapUnitScale() const { return mPatternWidthMapUnitScale; }

    /**
     * Sets the units for the stroke width.
     * \param unit width units
     * \see svgStrokeWidthUnit()
    */
    void setSvgStrokeWidthUnit( QgsUnitTypes::RenderUnit unit ) { mSvgStrokeWidthUnit = unit; }

    /**
     * Returns the units for the stroke width.
     * \see setSvgStrokeWidthUnit()
    */
    QgsUnitTypes::RenderUnit svgStrokeWidthUnit() const { return mSvgStrokeWidthUnit; }

    void setSvgStrokeWidthMapUnitScale( const QgsMapUnitScale &scale ) { mSvgStrokeWidthMapUnitScale = scale; }
    const QgsMapUnitScale &svgStrokeWidthMapUnitScale() const { return mSvgStrokeWidthMapUnitScale; }

    void setOutputUnit( QgsUnitTypes::RenderUnit unit ) override;
    QgsUnitTypes::RenderUnit outputUnit() const override;

    void setMapUnitScale( const QgsMapUnitScale &scale ) override;
    QgsMapUnitScale mapUnitScale() const override;

  protected:
    //! Width of the pattern (in output units)
    double mPatternWidth;
    QgsUnitTypes::RenderUnit mPatternWidthUnit;
    QgsMapUnitScale mPatternWidthMapUnitScale;

    //! SVG data
    QByteArray mSvgData;
    //! Path to the svg file (or empty if constructed directly from data)
    QString mSvgFilePath;
    //! SVG view box (to keep the aspect ratio
    QRectF mSvgViewBox;
    //! SVG pattern image
    QImage *mSvgPattern = nullptr;

    //param(fill), param(stroke), param(stroke-width) are going
    //to be replaced in memory
    QColor mSvgStrokeColor;
    double mSvgStrokeWidth;
    QgsUnitTypes::RenderUnit mSvgStrokeWidthUnit;
    QgsMapUnitScale mSvgStrokeWidthMapUnitScale;

    void applyDataDefinedSettings( QgsSymbolRenderContext &context ) override;

  private:
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
 */
class CORE_EXPORT QgsLinePatternFillSymbolLayer: public QgsImageFillSymbolLayer
{
  public:
    QgsLinePatternFillSymbolLayer();
    ~QgsLinePatternFillSymbolLayer();

    static QgsSymbolLayer *create( const QgsStringMap &properties = QgsStringMap() ) SIP_FACTORY;
    static QgsSymbolLayer *createFromSld( QDomElement &element ) SIP_FACTORY;

    QString layerType() const override;

    void startRender( QgsSymbolRenderContext &context ) override;

    void stopRender( QgsSymbolRenderContext &context ) override;

    QgsStringMap properties() const override;

    QgsLinePatternFillSymbolLayer *clone() const override SIP_FACTORY;

    void toSld( QDomDocument &doc, QDomElement &element, const QgsStringMap &props ) const override;

    double estimateMaxBleed( const QgsRenderContext &context ) const override;

    QString ogrFeatureStyleWidth( double widthScaleFactor ) const;

    //getters and setters
    void setLineAngle( double a ) { mLineAngle = a; }
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

    void setLineWidth( double w );
    double lineWidth() const { return mLineWidth; }
    void setColor( const QColor &c ) override;
    QColor color() const override;
    void setOffset( double offset ) { mOffset = offset; }
    double offset() const { return mOffset; }

    /**
     * Sets the units for the distance between lines in the fill pattern.
     * \param unit distance units
     * \see distanceUnit()
     * \see setDistance()
    */
    void setDistanceUnit( QgsUnitTypes::RenderUnit unit ) { mDistanceUnit = unit; }

    /**
     * Returns the units for the distance between lines in the fill pattern.
     * \see setDistanceUnit()
     * \see distance()
    */
    QgsUnitTypes::RenderUnit distanceUnit() const { return mDistanceUnit; }

    void setDistanceMapUnitScale( const QgsMapUnitScale &scale ) { mDistanceMapUnitScale = scale; }
    const QgsMapUnitScale &distanceMapUnitScale() const { return mDistanceMapUnitScale; }

    /**
     * Sets the units for the line's width.
     * \param unit width units
     * \see lineWidthUnit()
    */
    void setLineWidthUnit( QgsUnitTypes::RenderUnit unit ) { mLineWidthUnit = unit; }

    /**
     * Returns the units for the line's width.
     * \see setLineWidthUnit()
    */
    QgsUnitTypes::RenderUnit lineWidthUnit() const { return mLineWidthUnit; }

    void setLineWidthMapUnitScale( const QgsMapUnitScale &scale ) { mLineWidthMapUnitScale = scale; }
    const QgsMapUnitScale &lineWidthMapUnitScale() const { return mLineWidthMapUnitScale; }

    /**
     * Sets the units for the line pattern's offset.
     * \param unit offset units
     * \see offsetUnit()
    */
    void setOffsetUnit( QgsUnitTypes::RenderUnit unit ) { mOffsetUnit = unit; }

    /**
     * Returns the units for the line pattern's offset.
     * \see setOffsetUnit()
    */
    QgsUnitTypes::RenderUnit offsetUnit() const { return mOffsetUnit; }

    void setOffsetMapUnitScale( const QgsMapUnitScale &scale ) { mOffsetMapUnitScale = scale; }
    const QgsMapUnitScale &offsetMapUnitScale() const { return mOffsetMapUnitScale; }

    void setOutputUnit( QgsUnitTypes::RenderUnit unit ) override;
    QgsUnitTypes::RenderUnit outputUnit() const override;

    void setMapUnitScale( const QgsMapUnitScale &scale ) override;
    QgsMapUnitScale mapUnitScale() const override;

    bool setSubSymbol( QgsSymbol *symbol SIP_TRANSFER ) override;
    QgsSymbol *subSymbol() override;

    QSet<QString> usedAttributes( const QgsRenderContext &context ) const override;

  protected:
    //! Distance (in mm or map units) between lines
    double mDistance = 5.0;
    QgsUnitTypes::RenderUnit mDistanceUnit = QgsUnitTypes::RenderMillimeters;
    QgsMapUnitScale mDistanceMapUnitScale;
    //! Line width (in mm or map units)
    double mLineWidth = 0;
    QgsUnitTypes::RenderUnit mLineWidthUnit = QgsUnitTypes::RenderMillimeters;
    QgsMapUnitScale mLineWidthMapUnitScale;
    QColor mColor;
    //! Vector line angle in degrees (0 = horizontal, counterclockwise)
    double mLineAngle = 45.0;
    //! Offset perpendicular to line direction
    double mOffset = 0.0;
    QgsUnitTypes::RenderUnit mOffsetUnit = QgsUnitTypes::RenderMillimeters;
    QgsMapUnitScale mOffsetMapUnitScale;

    void applyDataDefinedSettings( QgsSymbolRenderContext &context ) override;

  private:
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
    ~QgsPointPatternFillSymbolLayer();

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

    bool setSubSymbol( QgsSymbol *symbol SIP_TRANSFER ) override;
    virtual QgsSymbol *subSymbol() override { return mMarkerSymbol; }

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

    void setOutputUnit( QgsUnitTypes::RenderUnit unit ) override;
    QgsUnitTypes::RenderUnit outputUnit() const override;

    void setMapUnitScale( const QgsMapUnitScale &scale ) override;
    QgsMapUnitScale mapUnitScale() const override;

    virtual QSet<QString> usedAttributes( const QgsRenderContext &context ) const override;
    void setColor( const QColor &c ) override;
    virtual QColor color() const override;

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

    void applyDataDefinedSettings( QgsSymbolRenderContext &context ) override;

  private:
#ifdef SIP_RUN
    QgsPointPatternFillSymbolLayer( const QgsPointPatternFillSymbolLayer &other );
#endif

    void applyPattern( const QgsSymbolRenderContext &context, QBrush &brush, double distanceX, double distanceY,
                       double displacementX, double displacementY );
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

    virtual QSet<QString> usedAttributes( const QgsRenderContext &context ) const override;

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


