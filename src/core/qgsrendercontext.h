/***************************************************************************
                              qgsrendercontext.h
                              ------------------
  begin                : March 16, 2008
  copyright            : (C) 2008 by Marco Hugentobler
  email                : marco dot hugentobler at karto dot baug dot ethz dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSRENDERCONTEXT_H
#define QGSRENDERCONTEXT_H

#include "qgis_core.h"
#include "qgis_sip.h"
#include <QColor>
#include <QPainter>
#include <memory>

#include "qgscoordinatetransform.h"
#include "qgsexpressioncontext.h"
#include "qgsfeaturefilterprovider.h"
#include "qgslabelsink.h"
#include "qgsmaptopixel.h"
#include "qgsmapunitscale.h"
#include "qgsrectangle.h"
#include "qgsvectorsimplifymethod.h"
#include "qgsdistancearea.h"
#include "qgscoordinatetransformcontext.h"
#include "qgspathresolver.h"
#include "qgstemporalrangeobject.h"

class QPainter;
class QgsAbstractGeometry;
class QgsLabelingEngine;
class QgsMapSettings;
class QgsRenderedFeatureHandlerInterface;
class QgsSymbolLayer;
class QgsMaskIdProvider;
class QgsMapClippingRegion;


/**
 * \ingroup core
 * \brief Contains information about the context of a rendering operation.
 *
 * The context of a rendering operation defines properties such as
 * the conversion ratio between screen and map units, the extents
 * to be rendered etc.
 */
class CORE_EXPORT QgsRenderContext : public QgsTemporalRangeObject
{
  public:
    QgsRenderContext();
    ~QgsRenderContext() override;

    QgsRenderContext( const QgsRenderContext &rh );
    QgsRenderContext &operator=( const QgsRenderContext &rh );

    /**
     * Set combination of flags that will be used for rendering.
     * \since QGIS 2.14
     */
    void setFlags( Qgis::RenderContextFlags flags );

    /**
     * Enable or disable a particular flag (other flags are not affected)
     * \since QGIS 2.14
     */
    void setFlag( Qgis::RenderContextFlag flag, bool on = true );

    /**
     * Returns combination of flags used for rendering.
     * \since QGIS 2.14
     */
    Qgis::RenderContextFlags flags() const;

    /**
     * Check whether a particular flag is enabled.
     * \since QGIS 2.14
     */
    bool testFlag( Qgis::RenderContextFlag flag ) const;

    /**
     * create initialized QgsRenderContext instance from given QgsMapSettings
     * \since QGIS 2.4
     */
    static QgsRenderContext fromMapSettings( const QgsMapSettings &mapSettings );

    /**
     * Creates a default render context given a pixel based QPainter destination.
     * If no painter is specified or the painter has no device, then a default
     * DPI of 88 will be assumed.
     * \since QGIS 3.0
     */
    static QgsRenderContext fromQPainter( QPainter *painter );

    //getters

    /**
     * Returns the destination QPainter for the render operation.
     * \see setPainter()
    */
    QPainter *painter() {return mPainter;}

#ifndef SIP_RUN

    /**
     * Returns the const destination QPainter for the render operation.
     * \see setPainter()
    * \since QGIS 3.12
    */
    const QPainter *painter() const { return mPainter; }
#endif

    /**
     * Sets relevant flags on a destination \a painter, using the flags and settings
     * currently defined for the render context.
     *
     * If no \a painter is specified, then the flags will be applied to the render
     * context's painter().
     *
     * \since QGIS 3.16
     */
    void setPainterFlagsUsingContext( QPainter *painter = nullptr ) const;

    /**
     * Returns a mask QPainter for the render operation.
     * Multiple mask painters can be defined, each with a unique identifier.
     * nullptr is returned if a mask painter with the given identifier does not exist.
     * This is currently used to implement selective masking.
     * \see setMaskPainter()
     * \see currentMaskId()
     * \since QGIS 3.12
     */
    QPainter *maskPainter( int id = 0 ) { return mMaskPainter.value( id, nullptr ); }

    /**
     * When rendering a map layer in a second pass (for selective masking),
     * some symbol layers may be disabled.
     *
     * Returns the list of disabled symbol layers.
     * \see setDisabledSymbolLayers()
     * \see isSymbolLayerEnabled()
     * \since QGIS 3.12
     */
    QSet<const QgsSymbolLayer *> disabledSymbolLayers() const { return mDisabledSymbolLayers; }

    /**
     * When rendering a map layer in a second pass (for selective masking),
     * some symbol layers may be disabled.
     *
     * Checks whether a given symbol layer has been disabled for the current pass.
     * \see setDisabledSymbolLayers()
     * \see disabledSymbolLayers()
     * \since QGIS 3.12
     */
    bool isSymbolLayerEnabled( const QgsSymbolLayer *layer ) const { return ! mDisabledSymbolLayers.contains( layer ); }

    /**
     * Returns the current coordinate transform for the context.
     *
     * This represents the coordinate transform required to transform a layer
     * which is being rendered back to the CRS of the rendered map. If no coordinate
     * transformation is required, or the render context is not associated with
     * a map layer render, then an invalid coordinate transformation is returned.
     *
     * \see setCoordinateTransform()
     */
    QgsCoordinateTransform coordinateTransform() const {return mCoordTransform;}

    /**
     * A general purpose distance and area calculator, capable of performing ellipsoid based calculations.
     * \since QGIS 3.0
     */
    const QgsDistanceArea &distanceArea() const { return mDistanceArea; }

    /**
     * Returns the context's coordinate transform context, which stores various
     * information regarding which datum transforms should be used when transforming points
     * from a source to destination coordinate reference system.
     *
     * \see setTransformContext()
     * \since QGIS 3.0
     */
    QgsCoordinateTransformContext transformContext() const;

    /**
     * Sets the context's coordinate transform \a context, which stores various
     * information regarding which datum transforms should be used when transforming points
     * from a source to destination coordinate reference system.
     *
     * \see transformContext()
     * \since QGIS 3.0
     */
    void setTransformContext( const QgsCoordinateTransformContext &context );

    /**
     * Returns the path resolver for conversion between relative and absolute paths
     * during rendering operations, e.g. for resolving relative symbol paths.
     *
     * \see setPathResolver()
     * \since QGIS 3.0
     */
    const QgsPathResolver &pathResolver() const { return mPathResolver; }

    /**
     * Sets the path \a resolver for conversion between relative and absolute paths
     * during rendering operations, e.g. for resolving relative symbol paths.
     *
     * \see pathResolver()
     * \since QGIS 3.0
     */
    void setPathResolver( const QgsPathResolver &resolver ) { mPathResolver = resolver; }

    /**
     * When rendering a map layer, calling this method returns the "clipping"
     * extent for the layer (in the layer's CRS).
     *
     * This extent is a "worst-case" scenario, which is guaranteed to cover the complete
     * visible portion of the layer when it is rendered to a map. It is often larger
     * than the actual visible portion of that layer.
     *
     * \warning For some layers, depending on the visible extent and the coordinate
     * transforms involved, this extent will represent the entire globe. This method
     * should never be used to determine the actual visible extent of a map render.
     *
     * \see setExtent()
     * \see mapExtent()
     */
    const QgsRectangle &extent() const { return mExtent; }

    /**
     * Returns the original extent of the map being rendered.
     *
     * Unlike extent(), this extent is always in the final destination CRS for the map
     * render and represents the exact bounds of the map being rendered.
     *
     * \see extent()
     * \see setMapExtent()
     * \since QGIS 3.4.8
     */
    QgsRectangle mapExtent() const { return mOriginalMapExtent; }

    /**
     * Returns the context's map to pixel transform, which transforms between map coordinates and device coordinates.
     *
     * \see setMapToPixel()
     */
    const QgsMapToPixel &mapToPixel() const {return mMapToPixel;}

    /**
     * Returns the scaling factor for the render to convert painter units
     * to physical sizes. This is usually equal to the number of pixels
     * per millimeter.
     * \see setScaleFactor()
     */
    double scaleFactor() const {return mScaleFactor;}

    /**
     * Returns the targeted DPI for rendering.
     *
     * \see setDpiTarget()
     * \since QGIS 3.20
     */
    double dpiTarget() const {return mDpiTarget;}

    /**
     * Returns TRUE if the rendering operation has been stopped and any ongoing
     * rendering should be canceled immediately.
     *
     * \note Since QGIS 3.22 the feedback() member exists as an alternative means of cancellation support.
     *
     * \see setRenderingStopped()
     * \see feedback()
     */
    bool renderingStopped() const {return mRenderingStopped;}

    /**
     * Attach a \a feedback object that can be queried regularly during rendering to check
     * if rendering should be canceled.
     *
     * Ownership of \a feedback is NOT transferred, and the caller must take care that it exists
     * for the lifetime of the render context.
     *
     * \see feedback()
     *
     * \since QGIS 3.22
     */
    void setFeedback( QgsFeedback *feedback );

    /**
     * Returns the feedback object that can be queried regularly during rendering to check
     * if rendering should be canceled, if set. Maybe be NULLPTR.
     *
     * \see setFeedback()
     *
     * \since QGIS 3.22
     */
    QgsFeedback *feedback() const;

    /**
     * Returns TRUE if rendering operations should use vector operations instead
     * of any faster raster shortcuts.
     *
     * \see setForceVectorOutput()
     */
    bool forceVectorOutput() const;

    /**
     * Returns TRUE if advanced effects such as blend modes such be used
     *
     * \see setUseAdvancedEffects()
     */
    bool useAdvancedEffects() const;

    /**
     * Used to enable or disable advanced effects such as blend modes
     *
     * \see useAdvancedEffects()
     */
    void setUseAdvancedEffects( bool enabled );

    /**
     * Returns TRUE if edit markers should be drawn during the render operation.
     *
     * \see setDrawEditingInformation()
     */
    bool drawEditingInformation() const;

    /**
     * Returns the renderer map scale. This will match the desired scale denominator
     * for the rendered map, eg 1000.0 for a 1:1000 map render.
     * \see setRendererScale()
     */
    double rendererScale() const {return mRendererScale;}


    /**
     * Returns the symbology reference scale.
     *
     * This represents the desired scale denominator for the rendered map, eg 1000.0 for a 1:1000 map render.
     * A value of -1 indicates that symbology scaling by reference scale is disabled.
     *
     * The symbology reference scale is an optional property which specifies the reference
     * scale at which symbology in paper units (such a millimeters or points) is fixed
     * to. For instance, if the scale is 1000 then a 2mm thick line will be rendered at
     * exactly 2mm thick when a map is rendered at 1:1000, or 1mm thick when rendered at 1:2000, or 4mm thick at 1:500.
     *
     * \see setSymbologyReferenceScale()
     * \see rendererScale()
     * \since QGIS 3.22
     */
    double symbologyReferenceScale() const { return mSymbologyReferenceScale; }

    /**
     * Gets access to new labeling engine (may be NULLPTR).
     * \note Not available in Python bindings.
     */
    QgsLabelingEngine *labelingEngine() const { return mLabelingEngine; } SIP_SKIP

    /**
     * Returns the associated label sink, or NULLPTR if not set.
     * \note Not available in Python bindings.
     * \since QGIS 3.24
     */
    QgsLabelSink *labelSink() const { return mLabelSink; } SIP_SKIP

    /**
     * Returns the color to use when rendering selected features.
     *
     * \see setSelectionColor()
     */
    QColor selectionColor() const { return mSelectionColor; }

    /**
     * Returns TRUE if vector selections should be shown in the rendered map
     * \returns TRUE if selections should be shown
     * \see setShowSelection
     * \see selectionColor
     * \since QGIS v2.4
     */
    bool showSelection() const;

    //setters

    /**
     * Sets the current coordinate transform for the context.
     *
     * This represents the coordinate transform required to transform the layer
     * which is being rendered back to the CRS of the rendered map.
     *
     * Set to an invalid QgsCoordinateTransform to indicate that no transformation is required.
     *
     * \see coordinateTransform()
     */
    void setCoordinateTransform( const QgsCoordinateTransform &t );

    /**
     * Sets the context's map to pixel transform, which transforms between map coordinates and device coordinates.
     *
     * \see mapToPixel()
     */
    void setMapToPixel( const QgsMapToPixel &mtp ) {mMapToPixel = mtp;}

    /**
     * When rendering a map layer, calling this method sets the "clipping"
     * extent for the layer (in the layer's CRS).
     *
     * This extent should be a "worst-case" scenario, which is guaranteed to
     * completely cover the entire visible portion of the layer when it is rendered
     * to the map. It may be larger than the actual visible area, but MUST contain at least the
     * entire visible area.
     *
     * \see setExtent()
     * \see setMapExtent()
     */
    void setExtent( const QgsRectangle &extent ) {mExtent = extent;}

    /**
     * Sets the original \a extent of the map being rendered.
     *
     * Unlike setExtent(), this extent is always in the final destination CRS for the map
     * render and represents the exact bounds of the map being rendered.
     *
     * \see mapExtent()
     * \see setExtent()
     * \since QGIS 3.4.8
     */
    void setMapExtent( const QgsRectangle &extent ) { mOriginalMapExtent = extent; }

    /**
     * Sets whether edit markers should be drawn during the render operation.
     *
     * \see drawEditingInformation()
     */
    void setDrawEditingInformation( bool b );

    /**
     * Sets whether the rendering operation has been \a stopped and any ongoing
     * rendering should be canceled immediately.
     *
     * \note Since QGIS 3.22 the feedback() member exists as an alternative means of cancellation support.
     *
     * \see renderingStopped()
     * \see feedback()
     * \see setFeedback()
     */
    void setRenderingStopped( bool stopped ) {mRenderingStopped = stopped;}

    /**
     * A general purpose distance and area calculator, capable of performing ellipsoid based calculations.
     * Will be used to convert meter distances to active MapUnit values for QgsUnitTypes::RenderMetersInMapUnits
     * \since QGIS 3.0
     */
    void setDistanceArea( const QgsDistanceArea &distanceArea ) {mDistanceArea = distanceArea ;}

    /**
     * Sets the scaling factor for the render to convert painter units
     * to physical sizes. This should usually be equal to the number of pixels
     * per millimeter.
     * \see scaleFactor()
     */
    void setScaleFactor( double factor ) {mScaleFactor = factor;}

    /**
     * Sets the targeted \a dpi for rendering.
     *
     * \see dpiTarget()
     * \since QGIS 3.20
     */
    void setDpiTarget( double dpi ) {mDpiTarget = dpi;}

    /**
     * Sets the renderer map scale. This should match the desired scale denominator
     * for the rendered map, eg 1000.0 for a 1:1000 map render.
     * \see rendererScale()
     */
    void setRendererScale( double scale ) {mRendererScale = scale;}

    /**
     * Sets the symbology reference \a scale.
     *
     * This should match the desired scale denominator for the rendered map, eg 1000.0 for a 1:1000 map render.
     * Set to -1 to disable symbology scaling by reference scale.
     *
     * The symbology reference scale is an optional property which specifies the reference
     * scale at which symbology in paper units (such a millimeters or points) is fixed
     * to. For instance, if \a scale is set to 1000 then a 2mm thick line will be rendered at
     * exactly 2mm thick when a map is rendered at 1:1000, or 1mm thick when rendered at 1:2000, or 4mm thick at 1:500.
     *
     * \see symbologyReferenceScale()
     * \see rendererScale()
     * \since QGIS 3.22
     */
    void setSymbologyReferenceScale( double scale ) { mSymbologyReferenceScale = scale; }

    /**
     * Sets the destination QPainter for the render operation. Ownership of the painter
     * is not transferred and the QPainter destination must stay alive for the duration
     * of any rendering operations.
     * \see painter()
     */
    void setPainter( QPainter *p ) {mPainter = p;}

    /**
     * Sets a mask QPainter for the render operation. Ownership of the painter
     * is not transferred and the QPainter must stay alive for the duration
     * of any rendering operations.
     * Multiple mask painters can be defined and the second parameter gives a unique identifier to each one.
     * \see maskPainter()
     */
    void setMaskPainter( QPainter *p, int id = 0 ) { mMaskPainter[id] = p; }

    /**
     * When rendering a map layer in a second pass (for selective masking),
     * some symbol layers may be disabled.
     *
     * Sets the list of disabled symbol layers.
     * \see disabledSymbolLayers()
     * \see isSymbolLayerEnabled()
     * \since QGIS 3.12
     */
    void setDisabledSymbolLayers( const QSet<const QgsSymbolLayer *> &symbolLayers ) { mDisabledSymbolLayers = symbolLayers; }

    /**
     * Sets whether rendering operations should use vector operations instead
     * of any faster raster shortcuts.
     *
     * \see forceVectorOutput()
     */
    void setForceVectorOutput( bool force );

    /**
     * Assigns the labeling engine
     * \note Not available in Python bindings.
     */
    void setLabelingEngine( QgsLabelingEngine *engine ) { mLabelingEngine = engine; } SIP_SKIP

    /**
     * Assigns the label sink which will take over responsibility for handling labels.
     * \note Ownership is not transferred and the sink must exist for the lifetime of the map rendering job.
     * \note Not available in Python bindings.
     * \since QGIS 3.24
     */
    void setLabelSink( QgsLabelSink *sink ) { mLabelSink = sink; } SIP_SKIP

    /**
     * Sets the \a color to use when rendering selected features.
     *
     * \see selectionColor()
     */
    void setSelectionColor( const QColor &color ) { mSelectionColor = color; }

    /**
     * Sets whether vector selections should be shown in the rendered map
     * \param showSelection set to TRUE if selections should be shown
     * \see showSelection
     * \see setSelectionColor
     * \since QGIS v2.4
     */
    void setShowSelection( bool showSelection );

    /**
     * Returns TRUE if the rendering optimization (geometry simplification) can be executed
     *
     * \see setUseRenderingOptimization()
     */
    bool useRenderingOptimization() const;

    /**
     * Sets whether the rendering optimization (geometry simplification) should be executed
     *
     * \see useRenderingOptimization()
     */
    void setUseRenderingOptimization( bool enabled );

    /**
     * Returns the simplification settings to use when rendering vector layers.
     *
     * The default is to use no simplification.
     *
     * \see setVectorSimplifyMethod()
     * \since QGIS 2.4
     */
    const QgsVectorSimplifyMethod &vectorSimplifyMethod() const { return mVectorSimplifyMethod; }

    /**
     * Sets the simplification setting to use when rendering vector layers.
     *
     * This can be used to specify simplification methods to apply during map exports and renders,
     * e.g. to allow vector layers to be simplified to an appropriate maximum level of detail
     * during PDF exports or to speed up layer rendering
     *
     * The default is to use no simplification.
     *
     * \see vectorSimplifyMethod()
     *
     * \since QGIS 2.4
     */
    void setVectorSimplifyMethod( const QgsVectorSimplifyMethod &simplifyMethod ) { mVectorSimplifyMethod = simplifyMethod; }

    /**
     * Sets the expression context. This context is used for all expression evaluation
     * associated with this render context.
     * \see expressionContext()
     * \since QGIS 2.12
     */
    void setExpressionContext( const QgsExpressionContext &context ) { mExpressionContext = context; }

    /**
     * Gets the expression context. This context should be used for all expression evaluation
     * associated with this render context.
     * \see setExpressionContext()
     * \since QGIS 2.12
     */
    QgsExpressionContext &expressionContext() { return mExpressionContext; }

    /**
     * Gets the expression context (const version). This context should be used for all expression evaluation
     * associated with this render context.
     * \see setExpressionContext()
     * \note not available in Python bindings
     * \since QGIS 2.12
     */
    const QgsExpressionContext &expressionContext() const { return mExpressionContext; } SIP_SKIP

    //! Returns pointer to the unsegmentized geometry
    const QgsAbstractGeometry *geometry() const { return mGeometry; }
    //! Sets pointer to original (unsegmentized) geometry
    void setGeometry( const QgsAbstractGeometry *geometry ) { mGeometry = geometry; }

    /**
     * Set a filter feature provider used for additional filtering of rendered features.
     * \param ffp the filter feature provider
     * \see featureFilterProvider()
     * \since QGIS 2.14
     */
    void setFeatureFilterProvider( const QgsFeatureFilterProvider *ffp );

    /**
     * Gets the filter feature provider used for additional filtering of rendered features.
     * \returns the filter feature provider
     * \see setFeatureFilterProvider()
     * \since QGIS 2.14
     */
    const QgsFeatureFilterProvider *featureFilterProvider() const;

    /**
     * Sets the segmentation tolerance applied when rendering curved geometries
     * \param tolerance the segmentation tolerance
     * \see segmentationTolerance()
     * \see segmentationToleranceType()
     */
    void setSegmentationTolerance( double tolerance ) { mSegmentationTolerance = tolerance; }

    /**
     * Gets the segmentation tolerance applied when rendering curved geometries
     * \see setSegmentationTolerance()
     */
    double segmentationTolerance() const { return mSegmentationTolerance; }

    /**
     * Sets segmentation tolerance type (maximum angle or maximum difference between curve and approximation)
     * \param type the segmentation tolerance typename
     * \see segmentationToleranceType()
     * \see segmentationTolerance()
     */
    void setSegmentationToleranceType( QgsAbstractGeometry::SegmentationToleranceType type ) { mSegmentationToleranceType = type; }

    /**
     * Gets segmentation tolerance type (maximum angle or maximum difference between curve and approximation)
     * \see setSegmentationToleranceType()
     */
    QgsAbstractGeometry::SegmentationToleranceType segmentationToleranceType() const { return mSegmentationToleranceType; }

    // Conversions

    /**
     * Converts a size from the specified units to painter units (pixels). The conversion respects the limits
     * specified by the optional scale parameter.
     *
     * Since QGIS 3.22 the optional \a property argument can be used to specify the associated property. This
     * is used in some contexts to refine the converted size. For example, a Qgis::RenderSubcomponentProperty::BlurSize
     * property will be limited to a suitably fast range when the render context has the Qgis::RenderContextFlag::RenderSymbolPreview set.
     *
     * \see convertToMapUnits()
     * \since QGIS 3.0
     */
    double convertToPainterUnits( double size, QgsUnitTypes::RenderUnit unit, const QgsMapUnitScale &scale = QgsMapUnitScale(), Qgis::RenderSubcomponentProperty property = Qgis::RenderSubcomponentProperty::Generic ) const;

    /**
     * Converts a size from the specified units to map units. The conversion respects the limits
     * specified by the optional scale parameter.
     * \see convertToPainterUnits()
     * \since QGIS 3.0
     */
    double convertToMapUnits( double size, QgsUnitTypes::RenderUnit unit, const QgsMapUnitScale &scale = QgsMapUnitScale() ) const;

    /**
     * Converts a size from map units to the specified units.
     * \see convertToMapUnits()
     * \since QGIS 3.0
     */
    double convertFromMapUnits( double sizeInMapUnits, QgsUnitTypes::RenderUnit outputUnit ) const;

    /**
     * Convert meter distances to active MapUnit values for QgsUnitTypes::RenderMetersInMapUnits
     * \note
      * When the sourceCrs() is geographic, the center of the Extent will be used
     * \since QGIS 3.0
     */
    double convertMetersToMapUnits( double meters ) const;

    /**
     * Returns the text render format, which dictates how text is rendered (e.g. as paths or real text objects).
     *
     * \see setTextRenderFormat()
     * \since QGIS 3.4.3
     */
    Qgis::TextRenderFormat textRenderFormat() const
    {
      return mTextRenderFormat;
    }

    /**
     * Sets the text render \a format, which dictates how text is rendered (e.g. as paths or real text objects).
     *
     * \see textRenderFormat()
     * \since QGIS 3.4.3
     */
    void setTextRenderFormat( Qgis::TextRenderFormat format )
    {
      mTextRenderFormat = format;
    }

    /**
     * Returns the list of rendered feature handlers to use while rendering map layers.
     * \see hasRenderedFeatureHandlers()
     * \since QGIS 3.10
     */
    QList<QgsRenderedFeatureHandlerInterface *> renderedFeatureHandlers() const;

    /**
     * Returns TRUE if the context has any rendered feature handlers.
     * \see renderedFeatureHandlers()
     * \since QGIS 3.10
     */
    bool hasRenderedFeatureHandlers() const { return mHasRenderedFeatureHandlers; }

    /**
     * Attaches a mask id provider to the context. It will allow some rendering operations to set the current mask id
     * based on the context (label layer names and label rules for instance).
     * \see QgsMaskIdProvider
     * \see setCurrentMaskId()
     * \see maskIdProvider()
     * \since QGIS 3.12
     */
    void setMaskIdProvider( QgsMaskIdProvider *provider ) { mMaskIdProvider = provider; }

    /**
     * Returns the mask id provider attached to the context.
     * \see setMaskIdProvider()
     * \since QGIS 3.12
     */
    const QgsMaskIdProvider *maskIdProvider() const { return mMaskIdProvider; }

    /**
     * Stores a mask id as the "current" one.
     * \see currentMaskId()
     * \since QGIS 3.12
     */
    void setCurrentMaskId( int id ) { mCurrentMaskId = id; }

    /**
     * Returns the current mask id, which can be used with maskPainter()
     * \see setCurrentMaskId()
     * \see maskPainter()
     * \since QGIS 3.12
     */
    int currentMaskId() const { return mCurrentMaskId; }

    /**
     * Sets GUI preview mode.
     * GUI preview mode is used to change the behavior of some renderings when
     * they are done to preview of symbology in the GUI.
     * This is especially used to display mask symbol layers rather than painting them
     * in a mask painter, which is not meant to be visible, by definition.
     * \see isGuiPreview
     * \since QGIS 3.12
     */
    void setIsGuiPreview( bool preview ) { mIsGuiPreview = preview; }

    /**
     * Returns the Gui preview mode.
     * GUI preview mode is used to change the behavior of some renderings when
     * they are done to preview of symbology in the GUI.
     * This is especially used to display mask symbol layers rather than painting them
     * in a mask painter, which is not meant to be visible, by definition.
     * \see isGuiPreview
     * \see setIsGuiPreview
     * \since QGIS 3.12
     */
    bool isGuiPreview() const { return mIsGuiPreview; }

    /**
     * Gets custom rendering flags. Layers might honour these to alter their rendering.
     * \returns a map of custom flags
     * \see setCustomRenderingFlag()
     * \since QGIS 3.12
     */
    QVariantMap customRenderingFlags() const { return mCustomRenderingFlags; }

    /**
     * Sets a custom rendering flag. Layers might honour these to alter their rendering.
     * \param flag the flag name
     * \param value the flag value
     * \see customRenderingFlags()
     * \since QGIS 3.12
     */
    void setCustomRenderingFlag( const QString &flag, const QVariant &value ) { mCustomRenderingFlags[flag] = value; }

    /**
     * Clears the specified custom rendering flag.
     * \param flag the flag name
     * \see setCustomRenderingFlag()
     * \since QGIS 3.12
     */
    void clearCustomRenderingFlag( const QString &flag ) { mCustomRenderingFlags.remove( flag ); }

    /**
     * Returns the list of clipping regions to apply during the render.
     *
     * These regions are always in the final destination CRS for the map.
     *
     * \since QGIS 3.16
     */
    QList< QgsMapClippingRegion > clippingRegions() const;

    /**
     * Returns the geometry to use to clip features at render time.
     *
     * When vector features are rendered, they should be clipped to this geometry.
     *
     * \warning The clipping must take effect for rendering the feature's symbol only,
     * and should never be applied directly to the feature being rendered. Doing so would
     * impact the results of rendering rules which rely on feature geometry, such as
     * a rule-based renderer using the feature's area.
     *
     * \see setFeatureClipGeometry()
     *
     * \since QGIS 3.16
     */
    QgsGeometry featureClipGeometry() const;

    /**
     * Sets a \a geometry to use to clip features at render time.
     *
     * \note This is not usually set directly, but rather specified by calling QgsMapSettings:addClippingRegion()
     * prior to constructing a QgsRenderContext.
     *
     * \see featureClipGeometry()
     * \since QGIS 3.16
     */
    void setFeatureClipGeometry( const QgsGeometry &geometry );

    /**
     * Returns the texture origin, which should be used as a brush transform when
     * rendering using QBrush objects.
     *
     * \see setTextureOrigin()
     * \since QGIS 3.16
     */
    QPointF textureOrigin() const;

    /**
     * Sets the texture \a origin, which should be used as a brush transform when
     * rendering using QBrush objects.
     *
     * \see textureOrigin()
     * \since QGIS 3.16
     */
    void setTextureOrigin( const QPointF &origin );

    /**
     * Returns the range of z-values which should be rendered.
     *
     * \see setZRange()
     * \since QGIS 3.18
     */
    QgsDoubleRange zRange() const;

    /**
     * Sets the \a range of z-values which should be rendered.
     *
     * \see zRange()
     * \since QGIS 3.18
     */
    void setZRange( const QgsDoubleRange &range );

    /**
     * Returns the size of the resulting rendered image, in pixels.
     *
     * \see deviceOutputSize()
     * \see setOutputSize()
     *
     * \since QGIS 3.22
     */
    QSize outputSize() const;

    /**
     * Sets the \a size of the resulting rendered image, in pixels.
     *
     * \see outputSize()
     * \since QGIS 3.22
     */
    void setOutputSize( QSize size );

    /**
     * Returns the device pixel ratio.
     *
     * Common values are 1 for normal-dpi displays and 2 for high-dpi "retina" displays.
     *
     * \see setDevicePixelRatio()
     * \since QGIS 3.22
     */
    float devicePixelRatio() const;

    /**
     * Sets the device pixel \a ratio.
     *
     * Common values are 1 for normal-dpi displays and 2 for high-dpi "retina" displays.
     *
     * \see devicePixelRatio()
     * \since QGIS 3.22
     */
    void setDevicePixelRatio( float ratio );

    /**
     * Returns the device output size of the render.
     *
     * This is equivalent to the output size multiplicated by the device pixel ratio.
     *
     * \see outputSize()
     * \see devicePixelRatio()
     * \see setOutputSize()
     *
     * \since QGIS 3.22
     */
    QSize deviceOutputSize() const;

    /**
     * Sets QImage \a format which should be used for QImages created
     * during rendering.
     *
     * \see imageFormat()
     * \since QGIS 3.22
     */
    void setImageFormat( QImage::Format format ) { mImageFormat = format; }

    /**
     * Returns the QImage format which should be used for QImages created
     * during rendering.
     *
     * \see setImageFormat
     * \since QGIS 3.22
     */
    QImage::Format imageFormat() const { return mImageFormat; }

    /**
    * Returns the renderer usage
    *
    * \see setRendererUsage()
    * \since QGIS 3.24
    */
    Qgis::RendererUsage rendererUsage() const {return mRendererUsage;}

    /**
    * Sets the renderer usage
    *
    * \note This usage not alter how the map gets rendered but the intention is that data provider
    * knows the context of rendering and may report that to the backend.
    *
    * \see rendererUsage()
    * \since QGIS 3.24
    */
    void setRendererUsage( Qgis::RendererUsage usage ) {mRendererUsage = usage;}

    /**
     * Returns the frame rate of the map, for maps which are part of an animation.
     *
     * Returns -1 if the map is not associated with an animation.
     *
     * \see setFrameRate()
     * \since QGIS 3.26
     */
    double frameRate() const;

    /**
     * Sets the frame \a rate of the map (in frames per second), for maps which are part of an animation.
     *
     * Defaults to -1 if the map is not associated with an animation.
     *
     * \see frameRate()
     * \since QGIS 3.26
     */
    void setFrameRate( double rate );

    /**
     * Returns the current frame number of the map (in frames per second), for maps which are part of an animation.
     *
     * Returns -1 if the map is not associated with an animation.
     *
     * \see setCurrentFrame()
     * \since QGIS 3.26
     */
    long long currentFrame() const;

    /**
     * Sets the current \a frame of the map, for maps which are part of an animation.
     *
     * Defaults to -1 if the map is not associated with an animation.
     *
     * \see currentFrame()
     * \since QGIS 3.26
     */
    void setCurrentFrame( long long frame );

  private:

    Qgis::RenderContextFlags mFlags;

    //! Painter for rendering operations
    QPainter *mPainter = nullptr;

    /**
     * Mask painters for selective masking.
     * Multiple mask painters can be defined for a rendering. The map key is a unique identifier for each mask painter.
     * \see mMaskIdProvider
     * \since QGIS 3.12
     */
    QMap<int, QPainter *> mMaskPainter;

    /**
     * Pointer to a mask id provider
     * \see QgsMaskIdProvider
     * \since QGIS 3.12
     */
    QgsMaskIdProvider *mMaskIdProvider = nullptr;

    /**
     * Current mask identifier
     * \since QGIS 3.12
     */
    int mCurrentMaskId = -1;

    /**
     * Whether we are rendering a preview of a symbol / label
     * \since QGIS 3.12
     */
    bool mIsGuiPreview = false;

    //! For transformation between coordinate systems. Can be invalid if on-the-fly reprojection is not used
    QgsCoordinateTransform mCoordTransform;

    /**
     * A general purpose distance and area calculator, capable of performing ellipsoid based calculations.
     * Will be used to convert meter distances to active MapUnit values for QgsUnitTypes::RenderMetersInMapUnits
     * \since QGIS 3.0
     */
    QgsDistanceArea mDistanceArea;

    QgsRectangle mExtent;
    QgsRectangle mOriginalMapExtent;

    QgsMapToPixel mMapToPixel;

    //! True if the rendering has been canceled
    bool mRenderingStopped = false;

    //! Optional feedback object, as an alternative for mRenderingStopped for cancellation support
    QgsFeedback *mFeedback = nullptr;

    //! Factor to scale line widths and point marker sizes
    double mScaleFactor = 1.0;

    //! Targeted DPI
    double mDpiTarget = -1.0;

    //! Map scale
    double mRendererScale = 1.0;

    double mSymbologyReferenceScale = -1;

    //! Labeling engine implementation (can be NULLPTR)
    QgsLabelingEngine *mLabelingEngine = nullptr;

    //! Label sink (can be NULLPTR)
    QgsLabelSink *mLabelSink = nullptr;

    //! Color used for features that are marked as selected
    QColor mSelectionColor;

    //! Simplification object which holds the information about how to simplify the features for fast rendering
    QgsVectorSimplifyMethod mVectorSimplifyMethod;

    //! Expression context
    QgsExpressionContext mExpressionContext;

    //! Pointer to the (unsegmentized) geometry
    const QgsAbstractGeometry *mGeometry = nullptr;

    //! The feature filter provider
    std::unique_ptr< QgsFeatureFilterProvider > mFeatureFilterProvider;

    double mSegmentationTolerance = M_PI_2 / 90;

    QgsAbstractGeometry::SegmentationToleranceType mSegmentationToleranceType = QgsAbstractGeometry::MaximumAngle;

    QgsCoordinateTransformContext mTransformContext;

    QgsPathResolver mPathResolver;

    Qgis::TextRenderFormat mTextRenderFormat = Qgis::TextRenderFormat::AlwaysOutlines;
    QList< QgsRenderedFeatureHandlerInterface * > mRenderedFeatureHandlers;
    bool mHasRenderedFeatureHandlers = false;
    QVariantMap mCustomRenderingFlags;

    QSet<const QgsSymbolLayer *> mDisabledSymbolLayers;

    QList< QgsMapClippingRegion > mClippingRegions;
    QgsGeometry mFeatureClipGeometry;

    QPointF mTextureOrigin;

    QgsDoubleRange mZRange;

    QSize mSize;
    float mDevicePixelRatio = 1.0;
    QImage::Format mImageFormat = QImage::Format_ARGB32_Premultiplied;

    Qgis::RendererUsage mRendererUsage = Qgis::RendererUsage::Unknown;

    double mFrameRate = -1;
    long long mCurrentFrame = -1;

#ifdef QGISDEBUG
    bool mHasTransformContext = false;
#endif
};

#ifndef SIP_RUN

/**
 * \ingroup core
 *
 * \brief Scoped object for temporary replacement of a QgsRenderContext destination painter.
 *
 * Temporarily swaps out the destination QPainter object for a QgsRenderContext for the lifetime of the object,
 * before replacing it to the original painter on destruction.
 *
 * \note Not available in Python bindings
 * \since QGIS 3.14
 */
class QgsScopedRenderContextPainterSwap
{
  public:

    /**
     * Constructor for QgsScopedRenderContextPainterSwap.
     *
     * Swaps the destination painter for \a context (set QgsRenderContext::setPainter() ) to
     * \a temporaryPainter.
     */
    QgsScopedRenderContextPainterSwap( QgsRenderContext &context, QPainter *temporaryPainter )
      : mContext( context )
      , mPreviousPainter( context.painter() )
    {
      mContext.setPainter( temporaryPainter );
    }

    /**
     * Resets the destination painter for the context back to the original QPainter object.
     */
    void reset()
    {
      if ( !mReleased )
      {
        mContext.setPainter( mPreviousPainter );
        mReleased = true;
      }
    }

    /**
     * Returns the destination painter for the context back to the original QPainter object.
     */
    ~QgsScopedRenderContextPainterSwap()
    {
      reset();
    }

  private:

    QgsRenderContext &mContext;
    QPainter *mPreviousPainter = nullptr;
    bool mReleased = false;
};


/**
 * \ingroup core
 *
 * \brief Scoped object for temporary scaling of a QgsRenderContext for millimeter based rendering.
 *
 * Temporarily scales the destination QPainter for a QgsRenderContext to use millimeter based units for the lifetime of the object,
 * before returning it to pixel based units on destruction.
 *
 * \note Not available in Python bindings
 * \since QGIS 3.14
 */
class QgsScopedRenderContextScaleToMm
{
  public:

    /**
     * Constructor for QgsScopedRenderContextScaleToMm.
     *
     * Rescales the destination painter (see QgsRenderContext::painter() ) to use millimeter based units.
     *
     * \warning It is the caller's responsibility to ensure that \a context is initially scaled to use pixel based units!
     */
    QgsScopedRenderContextScaleToMm( QgsRenderContext &context )
      : mContext( context )
    {
      if ( mContext.painter() )
        mContext.painter()->scale( mContext.scaleFactor(), mContext.scaleFactor() );
    }

    /**
     * Returns the destination painter back to pixel based units.
     */
    ~QgsScopedRenderContextScaleToMm()
    {
      if ( mContext.painter() )
        mContext.painter()->scale( 1.0 / mContext.scaleFactor(), 1.0 / mContext.scaleFactor() );
    }

  private:

    QgsRenderContext &mContext;
};


/**
 * \ingroup core
 *
 * \brief Scoped object for temporary scaling of a QgsRenderContext for pixel based rendering.
 *
 * Temporarily scales the destination QPainter for a QgsRenderContext to use pixel based units for the lifetime of the object,
 * before returning it to millimeter based units on destruction.
 *
 * \note Not available in Python bindings
 * \since QGIS 3.14
 */
class QgsScopedRenderContextScaleToPixels
{
  public:

    /**
     * Constructor for QgsScopedRenderContextScaleToPixels.
     *
     * Rescales the destination painter (see QgsRenderContext::painter() ) to use pixel based units.
     *
     * \warning It is the caller's responsibility to ensure that \a context is initially scaled to use millimeter based units!
     */
    QgsScopedRenderContextScaleToPixels( QgsRenderContext &context )
      : mContext( context )
    {
      if ( mContext.painter() )
        mContext.painter()->scale( 1.0 / mContext.scaleFactor(), 1.0 / mContext.scaleFactor() );
    }

    /**
     * Returns the destination painter back to millimeter based units.
     */
    ~QgsScopedRenderContextScaleToPixels()
    {
      if ( mContext.painter() )
        mContext.painter()->scale( mContext.scaleFactor(), mContext.scaleFactor() );
    }

  private:

    QgsRenderContext &mContext;
};


/**
 * \ingroup core
 *
 * \brief Scoped object for saving and restoring a QPainter object's state.
 *
 * Temporarily saves the QPainter state for the lifetime of the object, before restoring it
 * on destruction.
 *
 * \note Not available in Python bindings
 * \since QGIS 3.16
 */
class QgsScopedQPainterState
{
  public:

    /**
     * Constructor for QgsScopedQPainterState.
     *
     * Saves the specified \a painter state.
     */
    QgsScopedQPainterState( QPainter *painter )
      : mPainter( painter )
    {
      mPainter->save();
    }

    /**
     * Restores the painter back to its original state.
     */
    ~QgsScopedQPainterState()
    {
      mPainter->restore();
    }

  private:

    QPainter *mPainter = nullptr;
};


/**
 * \ingroup core
 *
 * \brief Scoped object for temporary override of the symbologyReferenceScale property of a QgsRenderContext.
 *
 * Temporarily changes the symbologyReferenceScale, before returning it to the original value on destruction.
 *
 * \note Not available in Python bindings
 * \since QGIS 3.22
 */
class QgsScopedRenderContextReferenceScaleOverride
{
  public:

    /**
     * Constructor for QgsScopedRenderContextReferenceScaleOverride.
     *
     * Temporarily sets the render \a context symbologyReferenceScale to \a scale for the lifetime of this object.
     */
    QgsScopedRenderContextReferenceScaleOverride( QgsRenderContext &context, double scale )
      : mContext( &context )
      , mOriginalScale( context.symbologyReferenceScale() )
    {
      mContext->setSymbologyReferenceScale( scale );
    }

    /**
     * Move constructor.
     */
    QgsScopedRenderContextReferenceScaleOverride( QgsScopedRenderContextReferenceScaleOverride &&o ) noexcept
      : mContext( o.mContext )
      , mOriginalScale( o.mOriginalScale )
    {
      o.mContext = nullptr;
    }

    /**
     * Returns the render context back to the original reference scale.
     */
    ~QgsScopedRenderContextReferenceScaleOverride()
    {
      if ( mContext )
        mContext->setSymbologyReferenceScale( mOriginalScale );
    }

  private:

    QgsRenderContext *mContext = nullptr;
    double mOriginalScale = 0;
};


#endif

#endif
