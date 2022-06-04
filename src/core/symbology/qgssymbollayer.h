/***************************************************************************
 qgssymbollayer.h
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
#ifndef QGSSYMBOLLAYER_H
#define QGSSYMBOLLAYER_H

#define DEG2RAD(x)    ((x)*M_PI/180)
#define DEFAULT_SCALE_METHOD              Qgis::ScaleMethod::ScaleDiameter

#include "qgis_core.h"
#include "qgis.h"
#include "qgsfields.h"
#include "qgspropertycollection.h"
#include "qgssymbolrendercontext.h"

#include <QColor>
#include <QMap>
#include <QPointF>
#include <QSet>
#include <QDomDocument>
#include <QDomElement>
#include <QPainterPath>
#include <QImage>

class QPainter;
class QSize;
class QPolygonF;

class QgsDxfExport;
class QgsExpression;
class QgsRenderContext;
class QgsPaintEffect;
class QgsSymbolLayerReference;

#ifndef SIP_RUN
typedef QMap<QString, QString> QgsStringMap;
#endif

/**
 * \ingroup core
 * \class QgsSymbolLayer
 */
class CORE_EXPORT QgsSymbolLayer
{
#ifdef SIP_RUN
#include <qgslinesymbollayer.h>
#endif


#ifdef SIP_RUN
    SIP_CONVERT_TO_SUBCLASS_CODE
    switch ( sipCpp->type() )
    {
      case Qgis::SymbolType::Marker:
        if ( sipCpp->layerType() == "EllipseMarker" )
          sipType = sipType_QgsEllipseSymbolLayer;
        else if ( sipCpp->layerType() == "FontMarker" )
          sipType = sipType_QgsFontMarkerSymbolLayer;
        else if ( sipCpp->layerType() == "SimpleMarker" )
          sipType = sipType_QgsSimpleMarkerSymbolLayer;
        else if ( sipCpp->layerType() == "FilledMarker" )
          sipType = sipType_QgsFilledMarkerSymbolLayer;
        else if ( sipCpp->layerType() == "SvgMarker" )
          sipType = sipType_QgsSvgMarkerSymbolLayer;
        else if ( sipCpp->layerType() == "RasterMarker" )
          sipType = sipType_QgsRasterMarkerSymbolLayer;
        else if ( sipCpp->layerType() == "AnimatedMarker" )
          sipType = sipType_QgsAnimatedMarkerSymbolLayer;
        else if ( sipCpp->layerType() == "VectorField" )
          sipType = sipType_QgsVectorFieldSymbolLayer;
        else if ( sipCpp->layerType() == "MaskMarker" )
          sipType = sipType_QgsMaskMarkerSymbolLayer;
        else
          sipType = sipType_QgsMarkerSymbolLayer;
        break;

      case Qgis::SymbolType::Line:
        if ( sipCpp->layerType() == "MarkerLine" )
          sipType = sipType_QgsMarkerLineSymbolLayer;
        else if ( sipCpp->layerType() == "SimpleLine" )
          sipType = sipType_QgsSimpleLineSymbolLayer;
        else if ( sipCpp->layerType() == "HashLine" )
          sipType = sipType_QgsHashedLineSymbolLayer;
        else if ( sipCpp->layerType() == "ArrowLine" )
          sipType = sipType_QgsArrowSymbolLayer;
        else if ( sipCpp->layerType() == "InterpolatedLine" )
          sipType = sipType_QgsInterpolatedLineSymbolLayer;
        else if ( sipCpp->layerType() == "RasterLine" )
          sipType = sipType_QgsRasterLineSymbolLayer;
        else if ( sipCpp->layerType() == "Lineburst" )
          sipType = sipType_QgsLineburstSymbolLayer;
        else
          sipType = sipType_QgsLineSymbolLayer;
        break;

      case Qgis::SymbolType::Fill:
        if ( sipCpp->layerType() == "SimpleFill" )
          sipType = sipType_QgsSimpleFillSymbolLayer;
        else if ( sipCpp->layerType() == "LinePatternFill" )
          sipType = sipType_QgsLinePatternFillSymbolLayer;
        else if ( sipCpp->layerType() == "PointPatternFill" )
          sipType = sipType_QgsPointPatternFillSymbolLayer;
        else if ( sipCpp->layerType() == "SVGFill" )
          sipType = sipType_QgsSVGFillSymbolLayer;
        else if ( sipCpp->layerType() == "RasterFill" )
          sipType = sipType_QgsRasterFillSymbolLayer;
        else if ( sipCpp->layerType() == "CentroidFill" )
          sipType = sipType_QgsCentroidFillSymbolLayer;
        else if ( sipCpp->layerType() == "GradientFill" )
          sipType = sipType_QgsGradientFillSymbolLayer;
        else if ( sipCpp->layerType() == "ShapeburstFill" )
          sipType = sipType_QgsShapeburstFillSymbolLayer;
        else if ( sipCpp->layerType() == "RandomMarkerFill" )
          sipType = sipType_QgsRandomMarkerFillSymbolLayer;
        else
          sipType = sipType_QgsFillSymbolLayer;
        break;

      case Qgis::SymbolType::Hybrid:
        sipType = sipType_QgsGeometryGeneratorSymbolLayer;
        break;
    }
    SIP_END
#endif
  public:

    /**
     * Data definable properties.
     * \since QGIS 3.0
     */
    enum Property
    {
      PropertySize = 0, //!< Symbol size
      PropertyAngle, //!< Symbol angle
      PropertyName, //!< Name, eg shape name for simple markers
      PropertyFillColor, //!< Fill color
      PropertyStrokeColor, //!< Stroke color
      PropertyStrokeWidth, //!< Stroke width
      PropertyStrokeStyle, //!< Stroke style (eg solid, dashed)
      PropertyOffset, //!< Symbol offset
      PropertyCharacter, //!< Character, eg for font marker symbol layers
      PropertyWidth, //!< Symbol width
      PropertyHeight, //!< Symbol height
      PropertyPreserveAspectRatio, //!< Preserve aspect ratio between width and height
      PropertyFillStyle, //!< Fill style (eg solid, dots)
      PropertyJoinStyle, //!< Line join style
      PropertySecondaryColor, //!< Secondary color (eg for gradient fills)
      PropertyLineAngle, //!< Line angle, or angle of hash lines for hash line symbols
      PropertyLineDistance, //!< Distance between lines, or length of lines for hash line symbols
      PropertyGradientType, //!< Gradient fill type
      PropertyCoordinateMode, //!< Gradient coordinate mode
      PropertyGradientSpread, //!< Gradient spread mode
      PropertyGradientReference1X, //!< Gradient reference point 1 x
      PropertyGradientReference1Y, //!< Gradient reference point 1 y
      PropertyGradientReference2X, //!< Gradient reference point 2 x
      PropertyGradientReference2Y, //!< Gradient reference point 2 y
      PropertyGradientReference1IsCentroid, //!< Gradient reference point 1 is centroid
      PropertyGradientReference2IsCentroid, //!< Gradient reference point 2 is centroid
      PropertyBlurRadius, //!< Shapeburst blur radius
      PropertyShapeburstUseWholeShape, //!< Shapeburst use whole shape
      PropertyShapeburstMaxDistance, //!< Shapeburst fill from edge distance
      PropertyShapeburstIgnoreRings, //!< Shapeburst ignore rings
      PropertyFile, //!< Filename, eg for svg files
      PropertyDistanceX, //!< Horizontal distance between points
      PropertyDistanceY, //!< Vertical distance between points
      PropertyDisplacementX, //!< Horizontal displacement
      PropertyDisplacementY, //!< Vertical displacement
      PropertyOpacity, //!< Opacity
      PropertyCustomDash, //!< Custom dash pattern
      PropertyCapStyle, //!< Line cap style
      PropertyPlacement, //!< Line marker placement
      PropertyInterval, //!< Line marker interval
      PropertyOffsetAlongLine, //!< Offset along line
      PropertyAverageAngleLength, //!< Length to average symbol angles over
      PropertyHorizontalAnchor, //!< Horizontal anchor point
      PropertyVerticalAnchor, //!< Vertical anchor point
      PropertyLayerEnabled, //!< Whether symbol layer is enabled
      PropertyArrowWidth, //!< Arrow tail width
      PropertyArrowStartWidth, //!< Arrow tail start width
      PropertyArrowHeadLength, //!< Arrow head length
      PropertyArrowHeadThickness, //!< Arrow head thickness
      PropertyArrowHeadType, //!< Arrow head type
      PropertyArrowType, //!< Arrow type
      PropertyOffsetX, //!< Horizontal offset
      PropertyOffsetY, //!< Vertical offset
      PropertyPointCount, //!< Point count
      PropertyRandomSeed, //!< Random number seed
      PropertyClipPoints, //!< Whether markers should be clipped to polygon boundaries
      PropertyDensityArea, //!< Density area
      PropertyFontFamily, //!< Font family
      PropertyFontStyle, //!< Font style
      PropertyDashPatternOffset, //!< Dash pattern offset,
      PropertyTrimStart, //!< Trim distance from start of line (since QGIS 3.20)
      PropertyTrimEnd, //!< Trim distance from end of line (since QGIS 3.20)
      PropertyLineStartWidthValue, //!< Start line width for interpolated line renderer (since QGIS 3.22)
      PropertyLineEndWidthValue, //!< End line width for interpolated line renderer (since QGIS 3.22)
      PropertyLineStartColorValue, //!< Start line color for interpolated line renderer (since QGIS 3.22)
      PropertyLineEndColorValue, //!< End line color for interpolated line renderer (since QGIS 3.22)
      PropertyMarkerClipping, //!< Marker clipping mode (since QGIS 3.24)
      PropertyRandomOffsetX, //!< Random offset X (since QGIS 3.24)
      PropertyRandomOffsetY, //!< Random offset Y (since QGIS 3.24)
      PropertyLineClipping, //!< Line clipping mode (since QGIS 3.24)
    };

    /**
     * Returns the symbol layer property definitions.
     * \since QGIS 3.0
     */
    static const QgsPropertiesDefinition &propertyDefinitions();

    virtual ~QgsSymbolLayer();

    //! QgsSymbolLayer cannot be copied
    QgsSymbolLayer( const QgsSymbolLayer &other ) = delete;

    //! QgsSymbolLayer cannot be copied
    QgsSymbolLayer &operator=( const QgsSymbolLayer &other ) = delete;

    /**
     * Returns flags which control the symbol layer's behavior.
     *
     * \since QGIS 3.22
     */
    virtual Qgis::SymbolLayerFlags flags() const;

    /**
     * Returns TRUE if symbol layer is enabled and will be drawn.
     * \see setEnabled()
     * \since QGIS 3.0
     */
    bool enabled() const { return mEnabled; }

    /**
     * Sets whether symbol layer is enabled and should be drawn. Disabled
     * layers are not drawn, but remain part of the symbol and can be re-enabled
     * when desired.
     * \see enabled()
     * \since QGIS 3.0
     */
    void setEnabled( bool enabled ) { mEnabled = enabled; }

    /**
     * Returns the "representative" color of the symbol layer.
     *
     * Depending on the symbol layer type, this will have different meaning. For instance, a line
     * symbol layer will generally return the stroke color of the layer, while a fill symbol layer
     * will return the "fill" color instead of stroke.
     *
     * Some symbol layer types will return an invalid QColor if they have no representative
     * color associated (e.g. raster image based symbol layers).
     *
     * \see setColor()
     * \see strokeColor()
     * \see fillColor()
     */
    virtual QColor color() const;

    /**
     * Sets the "representative" color for the symbol layer.
     *
     * Depending on the symbol layer type, this will have different meaning. For instance, a line
     * symbol layer will generally set the stroke color of the layer, while a fill symbol layer
     * will set the "fill" color instead of stroke.
     *
     * \see color()
     * \see setStrokeColor()
     * \see setFillColor()
     */
    virtual void setColor( const QColor &color );

    /**
     * Sets the stroke \a color for the symbol layer.
     *
     * This property is not supported by all symbol layer types, only those with a stroke component.
     *
     * \see strokeColor()
     * \see setColor()
     * \see setFillColor()
     *
     * \since QGIS 2.1
    */
    virtual void setStrokeColor( const QColor &color );

    /**
     * Returns the stroke color for the symbol layer.
     *
     * This property is not supported by all symbol layer types, only those with a stroke component. Symbol
     * layers without a stroke component will return an invalid QColor.
     *
     * \see setStrokeColor()
     * \see color()
     * \see fillColor()
     *
     * \since QGIS 2.1
    */
    virtual QColor strokeColor() const;

    /**
     * Sets the fill \a color for the symbol layer.
     *
     * This property is not supported by all symbol layer types, only those with a fill component.
     *
     * \see fillColor()
     * \see setColor()
     * \see setStrokeColor()
     *
     * \since QGIS 2.1
    */
    virtual void setFillColor( const QColor &color );

    /**
     * Returns the fill color for the symbol layer.
     *
     * This property is not supported by all symbol layer types, only those with a fill component. Symbol
     * layers without a fill component will return an invalid QColor.
     *
     * \see setFillColor()
     * \see color()
     * \see strokeColor()
     *
     * \since QGIS 2.1
    */
    virtual QColor fillColor() const;

    /**
     * Returns a string that represents this layer type. Used for serialization.
     * Should match with the string used to register this symbol layer in the registry.
     */
    virtual QString layerType() const = 0;

    /**
     * Called before a set of rendering operations commences on the supplied render \a context.
     *
     * This is always followed by a call to stopRender() after all rendering operations
     * have been completed.
     *
     * Subclasses can use this method to prepare for a set of rendering operations, e.g. by
     * pre-evaluating paths or images to render, and performing other one-time optimisations.
     *
     * \see startFeatureRender()
     * \see stopRender()
     */
    virtual void startRender( QgsSymbolRenderContext &context ) = 0;

    /**
     * Called after a set of rendering operations has finished on the supplied render \a context.
     *
     * This is always preceded by a call to startRender() before all rendering operations
     * are commenced.
     *
     * Subclasses can use this method to cleanup after a set of rendering operations.
     *
     * \see startRender()
     * \see stopFeatureRender()
     */
    virtual void stopRender( QgsSymbolRenderContext &context ) = 0;

    /**
     * Called before the layer will be rendered for a particular \a feature.
     *
     * This is always followed by a call to stopFeatureRender() after the feature
     * has been completely rendered (i.e. all parts have been rendered).
     *
     * The default implementation does nothing.
     *
     * \note In some circumstances, startFeatureRender() and stopFeatureRender() may not be called
     * before a symbol layer is rendered. E.g., when a symbol layer is being rendered in isolation
     * and not as a result of rendering a feature (for instance, when rendering a legend patch or other
     * non-feature based shape).
     *
     * \see stopFeatureRender()
     * \see startRender()
     *
     * \since QGIS 3.12
     */
    virtual void startFeatureRender( const QgsFeature &feature, QgsRenderContext &context );

    /**
     * Called after the layer has been rendered for a particular \a feature.
     *
     * This is always preceded by a call to startFeatureRender() just before the feature
     * will be rendered.
     *
     * The default implementation does nothing.
     *
     * \note In some circumstances, startFeatureRender() and stopFeatureRender() may not be called
     * before a symbol layer is rendered. E.g., when a symbol layer is being rendered in isolation
     * and not as a result of rendering a feature (for instance, when rendering a legend patch or other
     * non-feature based shape).
     *
     * \see startFeatureRender()
     * \see stopRender()
     *
     * \since QGIS 3.12
     */
    virtual void stopFeatureRender( const QgsFeature &feature, QgsRenderContext &context );

    /**
     * Shall be reimplemented by subclasses to create a deep copy of the instance.
     */
    virtual QgsSymbolLayer *clone() const = 0 SIP_FACTORY;

    //! Saves the symbol layer as SLD
    virtual void toSld( QDomDocument &doc, QDomElement &element, const QVariantMap &props ) const
    { Q_UNUSED( props ) element.appendChild( doc.createComment( QStringLiteral( "SymbolLayerV2 %1 not implemented yet" ).arg( layerType() ) ) ); }

    virtual QString ogrFeatureStyle( double mmScaleFactor, double mapUnitScaleFactor ) const { Q_UNUSED( mmScaleFactor ) Q_UNUSED( mapUnitScaleFactor ); return QString(); }

    /**
     * Should be reimplemented by subclasses to return a string map that
     * contains the configuration information for the symbol layer. This
     * is used to serialize a symbol layer perstistently.
     */
    virtual QVariantMap properties() const = 0;

    virtual void drawPreviewIcon( QgsSymbolRenderContext &context, QSize size ) = 0;

    /**
     * Returns the symbol's sub symbol, if present.
     */
    virtual QgsSymbol *subSymbol();

    //! Sets layer's subsymbol. takes ownership of the passed symbol
    virtual bool setSubSymbol( QgsSymbol *symbol SIP_TRANSFER );

    Qgis::SymbolType type() const { return mType; }

    //! Returns if the layer can be used below the specified symbol
    virtual bool isCompatibleWithSymbol( QgsSymbol *symbol ) const;

    /**
     * Returns TRUE if the symbol layer rendering can cause visible artifacts across a single feature
     * when the feature is rendered as a series of adjacent map tiles each containing a portion of the feature's geometry.
     *
     * The default implementation returns FALSE.
     *
     * \since QGIS 3.18
     */
    virtual bool canCauseArtifactsBetweenAdjacentTiles() const;

    /**
     * Sets whether the layer's colors are locked.
     *
     * If \a locked is TRUE then the symbol layer colors are locked and the layer will ignore any symbol-level color changes.
     *
     * \see isLocked()
     */
    void setLocked( bool locked ) { mLocked = locked; }

    /**
     * Returns TRUE if the symbol layer colors are locked and the layer will ignore any symbol-level color changes.
     *
     * \see setLocked()
     */
    bool isLocked() const { return mLocked; }

    /**
     * Returns the estimated maximum distance which the layer style will bleed outside
     * the drawn shape when drawn in the specified /a context. For example, polygons
     * drawn with an stroke will draw half the width
     * of the stroke outside of the polygon. This amount is estimated, since it may
     * be affected by data defined symbology rules.
    */
    virtual double estimateMaxBleed( const QgsRenderContext &context ) const { Q_UNUSED( context ) return 0; }

    /**
     * Sets the units to use for sizes and widths within the symbol layer. Individual
     * symbol layer subclasses will interpret this in different ways, e.g., a marker symbol
     * layer may use it to specify the units for the marker size, while a line symbol
     * layer may use it to specify the units for the line width.
     * \param unit output units
     * \see outputUnit()
     */
    virtual void setOutputUnit( QgsUnitTypes::RenderUnit unit ) { Q_UNUSED( unit ) }

    /**
     * Returns the units to use for sizes and widths within the symbol layer. Individual
     * symbol layer subclasses will interpret this in different ways, e.g., a marker symbol
     * layer may use it to specify the units for the marker size, while a line symbol
     * layer may use it to specify the units for the line width.
     * \returns output unit, or QgsUnitTypes::RenderUnknownUnit if the symbol layer contains mixed units
     * \see setOutputUnit()
     */
    virtual QgsUnitTypes::RenderUnit outputUnit() const { return QgsUnitTypes::RenderUnknownUnit; }

    /**
     * Returns TRUE if the symbol layer has any components which use map unit based sizes.
     *
     * \since QGIS 3.18
     */
    virtual bool usesMapUnits() const;

    virtual void setMapUnitScale( const QgsMapUnitScale &scale ) { Q_UNUSED( scale ) }
    virtual QgsMapUnitScale mapUnitScale() const { return QgsMapUnitScale(); }

    /**
     * Specifies the rendering pass in which this symbol layer should be rendered.
     * The lower the number, the lower the symbol will be rendered.
     * 0: first pass, 1: second pass, ...
     * Defaults to 0
     */
    void setRenderingPass( int renderingPass );

    /**
     * Specifies the rendering pass in which this symbol layer should be rendered.
     * The lower the number, the lower the symbol will be rendered.
     * 0: first pass, 1: second pass, ...
     * Defaults to 0
     */
    int renderingPass() const;

    /**
     * Returns the set of attributes referenced by the layer. This includes attributes
     * required by any data defined properties associated with the layer.
     */
    virtual QSet<QString> usedAttributes( const QgsRenderContext &context ) const;

    /**
     * Sets a data defined property for the layer. Any existing property with the same key
     * will be overwritten.
     * \see dataDefinedProperties()
     * \see Property
     * \since QGIS 3.0
     */
    virtual void setDataDefinedProperty( Property key, const QgsProperty &property );

    //! write as DXF
    virtual bool writeDxf( QgsDxfExport &e, double mmMapUnitScaleFactor, const QString &layerName, QgsSymbolRenderContext &context, QPointF shift = QPointF( 0.0, 0.0 ) ) const;

    //! Gets line width
    virtual double dxfWidth( const QgsDxfExport &e, QgsSymbolRenderContext &context ) const;

    //! Gets offset
    virtual double dxfOffset( const QgsDxfExport &e, QgsSymbolRenderContext &context ) const;

    //! Gets color
    virtual QColor dxfColor( QgsSymbolRenderContext &context ) const;

    //! Gets angle
    virtual double dxfAngle( QgsSymbolRenderContext &context ) const;

    //! Gets dash pattern
    virtual QVector<qreal> dxfCustomDashPattern( QgsUnitTypes::RenderUnit &unit ) const;

    //! Gets pen style
    virtual Qt::PenStyle dxfPenStyle() const;

    //! Gets brush/fill color
    virtual QColor dxfBrushColor( QgsSymbolRenderContext &context ) const;

    //! Gets brush/fill style
    virtual Qt::BrushStyle dxfBrushStyle() const;

    /**
     * Returns the current paint effect for the layer.
     * \returns paint effect
     * \see setPaintEffect
     * \since QGIS 2.9
     */
    QgsPaintEffect *paintEffect() const;

    /**
     * Sets the current paint effect for the layer.
     * \param effect paint effect. Ownership is transferred to the layer.
     * \see paintEffect
     * \since QGIS 2.9
     */
    void setPaintEffect( QgsPaintEffect *effect SIP_TRANSFER );

    /**
     * Prepares all data defined property expressions for evaluation. This should
     * be called prior to evaluating data defined properties.
     * \param context symbol render context
     * \since QGIS 2.12
     */
    virtual void prepareExpressions( const QgsSymbolRenderContext &context );

    /**
     * Returns a reference to the symbol layer's property collection, used for data defined overrides.
     * \see setDataDefinedProperties()
     * \see Property
     * \since QGIS 3.0
     */
    QgsPropertyCollection &dataDefinedProperties() { return mDataDefinedProperties; }

    /**
     * Returns a reference to the symbol layer's property collection, used for data defined overrides.
     * \see setDataDefinedProperties()
     * \since QGIS 3.0
     */
    const QgsPropertyCollection &dataDefinedProperties() const { return mDataDefinedProperties; } SIP_SKIP

    /**
     * Sets the symbol layer's property collection, used for data defined overrides.
     * \param collection property collection. Existing properties will be replaced.
     * \see dataDefinedProperties()
     * \since QGIS 3.0
     */
    void setDataDefinedProperties( const QgsPropertyCollection &collection ) { mDataDefinedProperties = collection; }

    /**
     * Returns TRUE if the symbol layer (or any of its sub-symbols) contains data defined properties.
     *
     * \since QGIS 3.4.5
     */
    virtual bool hasDataDefinedProperties() const;

    /**
     * Returns masks defined by this symbol layer.
     * This is a list of symbol layers of other layers that should be occluded.
     * \since QGIS 3.12
     */
    virtual QList<QgsSymbolLayerReference> masks() const;

    /**
     * Prepares all mask internal objects according to what is defined in \a context
     * This should be called prior to calling startRender() method.
     * \since QGIS 3.26
     */
    virtual void prepareMasks( const QgsSymbolRenderContext &context );

  protected:

    /**
     * Constructor for QgsSymbolLayer.
     * \param type specifies the associated symbol type
     * \param locked if TRUE, then symbol layer colors will be locked and will ignore any symbol-level color changes.
     */
    QgsSymbolLayer( Qgis::SymbolType type, bool locked = false );

    Qgis::SymbolType mType;

    //! True if layer is enabled and should be drawn
    bool mEnabled = true;

    bool mLocked = false;
    QColor mColor;
    int mRenderingPass = 0;

    QgsPropertyCollection mDataDefinedProperties;

    std::unique_ptr< QgsPaintEffect > mPaintEffect;
    QgsFields mFields;

    // clip path to be used during rendering
    QPainterPath mClipPath;

    // Configuration of selected symbology implementation
    //! Whether styles for selected features ignore symbol alpha
    static const bool SELECTION_IS_OPAQUE = true;
    //! Whether fill styles for selected features also highlight symbol stroke
    static const bool SELECT_FILL_BORDER = false;
    //! Whether fill styles for selected features uses symbol layer style
    static const bool SELECT_FILL_STYLE = false;

    /**
     * Restores older data defined properties from string map.
     * \since QGIS 3.0
     */
    void restoreOldDataDefinedProperties( const QVariantMap &stringMap );

    /**
     * Copies all data defined properties of this layer to another symbol layer.
     * \param destLayer destination layer
     */
    void copyDataDefinedProperties( QgsSymbolLayer *destLayer ) const;

    /**
     * Copies paint effect of this layer to another symbol layer
     * \param destLayer destination layer
     * \since QGIS 2.9
     */
    void copyPaintEffect( QgsSymbolLayer *destLayer ) const;

  private:
    static void initPropertyDefinitions();

    //! Property definitions
    static QgsPropertiesDefinition sPropertyDefinitions;

#ifdef SIP_RUN
    QgsSymbolLayer( const QgsSymbolLayer &other );
#endif

};

//////////////////////

/**
 * \ingroup core
 * \class QgsMarkerSymbolLayer
 * \brief Abstract base class for marker symbol layers.
 */
class CORE_EXPORT QgsMarkerSymbolLayer : public QgsSymbolLayer
{
  public:

    //! Symbol horizontal anchor points
    enum HorizontalAnchorPoint
    {
      Left, //!< Align to left side of symbol
      HCenter, //!< Align to horizontal center of symbol
      Right, //!< Align to right side of symbol
    };

    //! Symbol vertical anchor points
    enum VerticalAnchorPoint
    {
      Top, //!< Align to top of symbol
      VCenter, //!< Align to vertical center of symbol
      Bottom, //!< Align to bottom of symbol
    };

    //! QgsMarkerSymbolLayer cannot be copied
    QgsMarkerSymbolLayer( const QgsMarkerSymbolLayer &other ) = delete;

    //! QgsMarkerSymbolLayer cannot be copied
    QgsMarkerSymbolLayer &operator=( const QgsMarkerSymbolLayer &other ) = delete;

    void startRender( QgsSymbolRenderContext &context ) override;

    void stopRender( QgsSymbolRenderContext &context ) override;

    /**
     * Renders a marker at the specified point. Derived classes must implement this to
     * handle drawing the point.
     * \param point position at which to render point, in painter units
     * \param context symbol render context
     */
    virtual void renderPoint( QPointF point, QgsSymbolRenderContext &context ) = 0;

    void drawPreviewIcon( QgsSymbolRenderContext &context, QSize size ) override;

    /**
     * Sets the rotation angle for the marker.
     * \param angle angle in degrees clockwise from north.
     * \see angle()
     * \see setLineAngle()
     */
    void setAngle( double angle ) { mAngle = angle; }

    /**
     * Returns the rotation angle for the marker, in degrees clockwise from north.
     * \see setAngle()
     */
    double angle() const { return mAngle; }

    /**
     * Sets the line angle modification for the symbol's angle. This angle is added to
     * the marker's rotation and data defined rotation before rendering the symbol, and
     * is usually used for orienting symbols to match a line's angle.
     * \param lineAngle Angle in degrees clockwise from north, valid values are between 0 and 360
     * \see setAngle()
     * \see angle()
     * \since QGIS 2.9
     */
    void setLineAngle( double lineAngle ) { mLineAngle = lineAngle; }

    /**
     * Sets the symbol size.
     * \param size symbol size. Units are specified by sizeUnit().
     * \see size()
     * \see setSizeUnit()
     * \see setSizeMapUnitScale()
     */
    virtual void setSize( double size ) { mSize = size; }

    /**
     * Returns the symbol size. Units are specified by sizeUnit().
     * \see setSize()
     * \see sizeUnit()
     * \see sizeMapUnitScale()
     */
    double size() const { return mSize; }

    /**
     * Sets the units for the symbol's size.
     * \param unit size units
     * \see sizeUnit()
     * \see setSize()
     * \see setSizeMapUnitScale()
     */
    void setSizeUnit( QgsUnitTypes::RenderUnit unit ) { mSizeUnit = unit; }

    /**
     * Returns the units for the symbol's size.
     * \see setSizeUnit()
     * \see size()
     * \see sizeMapUnitScale()
     */
    QgsUnitTypes::RenderUnit sizeUnit() const { return mSizeUnit; }

    /**
     * Sets the map unit scale for the symbol's size.
     * \param scale size map unit scale
     * \see sizeMapUnitScale()
     * \see setSize()
     * \see setSizeUnit()
     */
    void setSizeMapUnitScale( const QgsMapUnitScale &scale ) { mSizeMapUnitScale = scale; }

    /**
     * Returns the map unit scale for the symbol's size.
     * \see setSizeMapUnitScale()
     * \see size()
     * \see sizeUnit()
     */
    const QgsMapUnitScale &sizeMapUnitScale() const { return mSizeMapUnitScale; }

    /**
     * Sets the method to use for scaling the marker's size.
     * \param scaleMethod scale method
     * \see scaleMethod()
     */
    void setScaleMethod( Qgis::ScaleMethod scaleMethod ) { mScaleMethod = scaleMethod; }

    /**
     * Returns the method to use for scaling the marker's size.
     * \see setScaleMethod()
     */
    Qgis::ScaleMethod scaleMethod() const { return mScaleMethod; }

    /**
     * Sets the marker's offset, which is the horizontal and vertical displacement which the rendered marker
     * should have from the original feature's geometry.
     * \param offset marker offset. Units are specified by offsetUnit()
     * \see offset()
     * \see setOffsetUnit()
     * \see setOffsetMapUnitScale()
     */
    void setOffset( QPointF offset ) { mOffset = offset; }

    /**
     * Returns the marker's offset, which is the horizontal and vertical displacement which the rendered marker
     * will have from the original feature's geometry. Units are specified by offsetUnit().
     * \see setOffset()
     * \see offsetUnit()
     * \see offsetMapUnitScale()
     */
    QPointF offset() const { return mOffset; }

    /**
     * Sets the units for the symbol's offset.
     * \param unit offset units
     * \see offsetUnit()
     * \see setOffset()
     * \see setOffsetMapUnitScale()
     */
    void setOffsetUnit( QgsUnitTypes::RenderUnit unit ) { mOffsetUnit = unit; }

    /**
     * Returns the units for the symbol's offset.
     * \see setOffsetUnit()
     * \see offset()
     * \see offsetMapUnitScale()
     */
    QgsUnitTypes::RenderUnit offsetUnit() const { return mOffsetUnit; }

    /**
     * Sets the map unit scale for the symbol's offset.
     * \param scale offset map unit scale
     * \see offsetMapUnitScale()
     * \see setOffset()
     * \see setOffsetUnit()
     */
    void setOffsetMapUnitScale( const QgsMapUnitScale &scale ) { mOffsetMapUnitScale = scale; }

    /**
     * Returns the map unit scale for the symbol's offset.
     * \see setOffsetMapUnitScale()
     * \see offset()
     * \see offsetUnit()
     */
    const QgsMapUnitScale &offsetMapUnitScale() const { return mOffsetMapUnitScale; }

    /**
     * Sets the horizontal anchor point for positioning the symbol.
     * \param h anchor point. Symbol will be drawn so that the horizontal anchor point is aligned with
     * the marker's desired location.
     * \see horizontalAnchorPoint()
     * \see setVerticalAnchorPoint()
     */
    void setHorizontalAnchorPoint( HorizontalAnchorPoint h ) { mHorizontalAnchorPoint = h; }

    /**
     * Returns the horizontal anchor point for positioning the symbol. The symbol will be drawn so that
     * the horizontal anchor point is aligned with the marker's desired location.
     * \see setHorizontalAnchorPoint()
     * \see verticalAnchorPoint()
     */
    HorizontalAnchorPoint horizontalAnchorPoint() const { return mHorizontalAnchorPoint; }

    /**
     * Sets the vertical anchor point for positioning the symbol.
     * \param v anchor point. Symbol will be drawn so that the vertical anchor point is aligned with
     * the marker's desired location.
     * \see verticalAnchorPoint()
     * \see setHorizontalAnchorPoint()
     */
    void setVerticalAnchorPoint( VerticalAnchorPoint v ) { mVerticalAnchorPoint = v; }

    /**
     * Returns the vertical anchor point for positioning the symbol. The symbol will be drawn so that
     * the vertical anchor point is aligned with the marker's desired location.
     * \see setVerticalAnchorPoint()
     * \see horizontalAnchorPoint()
     */
    VerticalAnchorPoint verticalAnchorPoint() const { return mVerticalAnchorPoint; }

    void toSld( QDomDocument &doc, QDomElement &element, const QVariantMap &props ) const override;

    /**
     * Writes the symbol layer definition as a SLD XML element.
     * \param doc XML document
     * \param element parent XML element
     * \param props symbol layer definition (see properties())
     */
    virtual void writeSldMarker( QDomDocument &doc, QDomElement &element, const QVariantMap &props ) const
    { Q_UNUSED( props ) element.appendChild( doc.createComment( QStringLiteral( "QgsMarkerSymbolLayer %1 not implemented yet" ).arg( layerType() ) ) ); }

    void setOutputUnit( QgsUnitTypes::RenderUnit unit ) override;
    QgsUnitTypes::RenderUnit outputUnit() const override;
    void setMapUnitScale( const QgsMapUnitScale &scale ) override;
    QgsMapUnitScale mapUnitScale() const override;

    /**
     * Returns the approximate bounding box of the marker symbol layer, taking into account
     * any data defined overrides and offsets which are set for the marker layer.
     * \returns approximate symbol bounds, in painter units
     * \since QGIS 2.14
     */
    virtual QRectF bounds( QPointF point, QgsSymbolRenderContext &context ) = 0;

  protected:

    /**
     * Constructor for QgsMarkerSymbolLayer.
     * \param locked set to TRUE to lock symbol color
     */
    QgsMarkerSymbolLayer( bool locked = false );

    /**
     * Calculates the required marker offset, including both the symbol offset
     * and any displacement required to align with the marker's anchor point.
     * \param context symbol render context
     * \param offsetX will be set to required horizontal offset (in painter units)
     * \param offsetY will be set to required vertical offset (in painter units)
     */
    void markerOffset( QgsSymbolRenderContext &context, double &offsetX, double &offsetY ) const;

    /**
     * Calculates the required marker offset, including both the symbol offset
     * and any displacement required to align with the marker's anchor point.
     * \param context symbol render context
     * \param width marker width
     * \param height marker height
     * \param offsetX will be set to required horizontal offset (in painter units)
     * \param offsetY will be set to required vertical offset (in painter units)
     * \note available in Python as markerOffsetWithWidthAndHeight
     */
    void markerOffset( QgsSymbolRenderContext &context, double width, double height, double &offsetX, double &offsetY ) const SIP_PYNAME( markerOffsetWithWidthAndHeight );

    //! \note available in Python bindings as markerOffset2
    void markerOffset( QgsSymbolRenderContext &context, double width, double height,
                       QgsUnitTypes::RenderUnit widthUnit, QgsUnitTypes::RenderUnit heightUnit,
                       double &offsetX, double &offsetY,
                       const QgsMapUnitScale &widthMapUnitScale, const QgsMapUnitScale &heightMapUnitScale ) const SIP_PYNAME( markerOffset2 );

    /**
     * Adjusts a marker offset to account for rotation.
     * \param offset offset prior to rotation
     * \param angle rotation angle in degrees clockwise from north
     * \returns adjusted offset
     */
    static QPointF _rotatedOffset( QPointF offset, double angle );

    //! Marker rotation angle, in degrees clockwise from north
    double mAngle = 0;
    //! Line rotation angle (see setLineAngle() for details)
    double mLineAngle = 0;
    //! Marker size
    double mSize = 2.0;
    //! Marker size unit
    QgsUnitTypes::RenderUnit mSizeUnit = QgsUnitTypes::RenderMillimeters;
    //! Marker size map unit scale
    QgsMapUnitScale mSizeMapUnitScale;
    //! Marker offset
    QPointF mOffset;
    //! Offset units
    QgsUnitTypes::RenderUnit mOffsetUnit = QgsUnitTypes::RenderMillimeters;
    //! Offset map unit scale
    QgsMapUnitScale mOffsetMapUnitScale;
    //! Marker size scaling method
    Qgis::ScaleMethod mScaleMethod = Qgis::ScaleMethod::ScaleDiameter;
    //! Horizontal anchor point
    HorizontalAnchorPoint mHorizontalAnchorPoint = HCenter;
    //! Vertical anchor point
    VerticalAnchorPoint mVerticalAnchorPoint = VCenter;

  private:
    static QgsMarkerSymbolLayer::HorizontalAnchorPoint decodeHorizontalAnchorPoint( const QString &str );
    static QgsMarkerSymbolLayer::VerticalAnchorPoint decodeVerticalAnchorPoint( const QString &str );

#ifdef SIP_RUN
    QgsMarkerSymbolLayer( const QgsMarkerSymbolLayer &other );
#endif
};

/**
 * \ingroup core
 * \class QgsLineSymbolLayer
 */
class CORE_EXPORT QgsLineSymbolLayer : public QgsSymbolLayer
{
  public:

    //! Options for filtering rings when the line symbol layer is being used to render a polygon's rings.
    enum RenderRingFilter
    {
      AllRings, //!< Render both exterior and interior rings
      ExteriorRingOnly, //!< Render the exterior ring only
      InteriorRingsOnly, //!< Render the interior rings only
    };

    //! QgsLineSymbolLayer cannot be copied
    QgsLineSymbolLayer( const QgsLineSymbolLayer &other ) = delete;

    //! QgsLineSymbolLayer cannot be copied
    QgsLineSymbolLayer &operator=( const QgsLineSymbolLayer &other ) = delete;

    void setOutputUnit( QgsUnitTypes::RenderUnit unit ) override;
    QgsUnitTypes::RenderUnit outputUnit() const override;
    void setMapUnitScale( const QgsMapUnitScale &scale ) override;
    QgsMapUnitScale mapUnitScale() const override;
    void drawPreviewIcon( QgsSymbolRenderContext &context, QSize size ) override;
    double dxfWidth( const QgsDxfExport &e, QgsSymbolRenderContext &context ) const override;

    /**
     * Renders the line symbol layer along the line joining \a points, using the given render \a context.
     * \see renderPolygonStroke()
     */
    virtual void renderPolyline( const QPolygonF &points, QgsSymbolRenderContext &context ) = 0;

    /**
     * Renders the line symbol layer along the outline of polygon, using the given render \a context.
     *
     * The exterior ring of the polygon is specified in \a points. Optionally, interior
     * rings are set via the \a rings argument.
     *
     * \see renderPolyline()
     */
    virtual void renderPolygonStroke( const QPolygonF &points, const QVector<QPolygonF> *rings, QgsSymbolRenderContext &context );

    /**
     * Sets the \a width of the line symbol layer.
     *
     * Calling this method updates the width of the line symbol layer, without
     * changing the existing width units. It has different effects depending
     * on the line symbol layer subclass, e.g. for a simple line layer it
     * changes the stroke width of the line, for a marker line layer it
     * changes the size of the markers used to draw the line.
     *
     * \see width()
     * \warning Since the width units vary, this method is useful for changing the
     * relative width of a line symbol layer only.
     */
    virtual void setWidth( double width ) { mWidth = width; }

    /**
     * Returns the estimated width for the line symbol layer.
     *
     * \warning This returned value is inaccurate if the symbol layer has sub-symbols with
     * different width units. Use the overload accepting a QgsRenderContext
     * argument instead for accurate sizes in this case.
     *
     * \see setWidth()
     */
    virtual double width() const { return mWidth; }

    /**
     * Returns the line symbol layer width, in painter units.
     *
     * This method returns an accurate width by calculating the actual rendered
     * width of the symbol layer using the provided render \a context.
     *
     * \see setWidth()
     *
     * \since QGIS 3.4.5
     */
    virtual double width( const QgsRenderContext &context ) const;

    /**
     * Returns the line's offset.
     *
     * Offset units can be retrieved by calling offsetUnit().
     *
     * \see setOffset()
     * \see offsetUnit()
     * \see offsetMapUnitScale()
     */
    double offset() const { return mOffset; }

    /**
     * Sets the line's \a offset.
     *
     * Offset units are set via setOffsetUnit().
     *
     * \see offset()
     * \see setOffsetUnit()
     * \see setOffsetMapUnitScale()
     */
    void setOffset( double offset ) { mOffset = offset; }

    /**
     * Sets the \a unit for the line's offset.
     * \see offsetUnit()
     * \see setOffset()
     * \see setOffsetMapUnitScale()
    */
    void setOffsetUnit( QgsUnitTypes::RenderUnit unit ) { mOffsetUnit = unit; }

    /**
     * Returns the units for the line's offset.
     * \see setOffsetUnit()
     * \see offset()
     * \see offsetMapUnitScale()
    */
    QgsUnitTypes::RenderUnit offsetUnit() const { return mOffsetUnit; }

    /**
     * Sets the map unit \a scale for the line's offset.
     * \see offsetMapUnitScale()
     * \see setOffset()
     * \see setOffsetUnit()
    */
    void setOffsetMapUnitScale( const QgsMapUnitScale &scale ) { mOffsetMapUnitScale = scale; }

    /**
     * Returns the map unit scale for the line's offset.
     * \see setOffsetMapUnitScale()
     * \see offset()
     * \see offsetUnit()
    */
    const QgsMapUnitScale &offsetMapUnitScale() const { return mOffsetMapUnitScale; }

    // TODO QGIS 4.0 - setWidthUnit(), widthUnit(), setWidthUnitScale(), widthUnitScale()
    // only apply to simple line symbol layers and do not belong here.

    /**
     * Sets the units for the line's width.
     * \param unit width units
     * \see widthUnit()
    */
    void setWidthUnit( QgsUnitTypes::RenderUnit unit ) { mWidthUnit = unit; }

    /**
     * Returns the units for the line's width.
     * \see setWidthUnit()
    */
    QgsUnitTypes::RenderUnit widthUnit() const { return mWidthUnit; }

    void setWidthMapUnitScale( const QgsMapUnitScale &scale ) { mWidthMapUnitScale = scale; }
    const QgsMapUnitScale &widthMapUnitScale() const { return mWidthMapUnitScale; }

    /**
     * Returns the line symbol layer's ring filter, which controls which rings are
     * rendered when the line symbol is being used to draw a polygon's rings.
     *
     * This setting has no effect when the line symbol is not being rendered
     * for a polygon.
     *
     * \see setRingFilter()
     * \since QGIS 3.6
     */
    RenderRingFilter ringFilter() const;

    /**
     * Sets the line symbol layer's ring \a filter, which controls which rings are
     * rendered when the line symbol is being used to draw a polygon's rings.
     *
     * This setting has no effect when the line symbol is not being rendered
     * for a polygon.
     *
     * \see ringFilter()
     * \since QGIS 3.6
     */
    void setRingFilter( QgsLineSymbolLayer::RenderRingFilter filter );

  protected:
    QgsLineSymbolLayer( bool locked = false );

    double mWidth = 0;
    QgsUnitTypes::RenderUnit mWidthUnit = QgsUnitTypes::RenderMillimeters;
    QgsMapUnitScale mWidthMapUnitScale;
    double mOffset = 0;
    QgsUnitTypes::RenderUnit mOffsetUnit = QgsUnitTypes::RenderMillimeters;
    QgsMapUnitScale mOffsetMapUnitScale;

    RenderRingFilter mRingFilter = AllRings;

  private:
#ifdef SIP_RUN
    QgsLineSymbolLayer( const QgsLineSymbolLayer &other );
#endif
};

/**
 * \ingroup core
 * \class QgsFillSymbolLayer
 */
class CORE_EXPORT QgsFillSymbolLayer : public QgsSymbolLayer
{
  public:

    //! QgsFillSymbolLayer cannot be copied
    QgsFillSymbolLayer( const QgsFillSymbolLayer &other ) = delete;

    //! QgsFillSymbolLayer cannot be copied
    QgsFillSymbolLayer &operator=( const QgsFillSymbolLayer &other ) = delete;

    /**
     * Renders the fill symbol layer for the polygon whose outer ring is defined by \a points, using the given render \a context.
     *
     * The \a rings argument optionally specifies a list of polygon rings to render as holes.
     */
    virtual void renderPolygon( const QPolygonF &points, const QVector<QPolygonF> *rings, QgsSymbolRenderContext &context ) = 0;

    void drawPreviewIcon( QgsSymbolRenderContext &context, QSize size ) override;

    void setAngle( double angle ) { mAngle = angle; }
    double angle() const { return mAngle; }

  protected:
    QgsFillSymbolLayer( bool locked = false );
    //! Default method to render polygon
    void _renderPolygon( QPainter *p, const QPolygonF &points, const QVector<QPolygonF> *rings, QgsSymbolRenderContext &context );

    double mAngle = 0.0;

  private:
#ifdef SIP_RUN
    QgsFillSymbolLayer( const QgsFillSymbolLayer &other );
#endif
};

class QgsSymbolLayerWidget;  // why does SIP fail, when this isn't here

#endif
