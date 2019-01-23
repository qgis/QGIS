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
#include <memory>

#include "qgscoordinatetransform.h"
#include "qgsexpressioncontext.h"
#include "qgsfeaturefilterprovider.h"
#include "qgsmaptopixel.h"
#include "qgsmapunitscale.h"
#include "qgsrectangle.h"
#include "qgsvectorsimplifymethod.h"
#include "qgsdistancearea.h"
#include "qgscoordinatetransformcontext.h"
#include "qgspathresolver.h"

class QPainter;
class QgsAbstractGeometry;
class QgsLabelingEngine;
class QgsMapSettings;


/**
 * \ingroup core
 * Contains information about the context of a rendering operation.
 * The context of a rendering operation defines properties such as
 * the conversion ratio between screen and map units, the extents
 * to be rendered etc.
 **/
class CORE_EXPORT QgsRenderContext
{
  public:
    QgsRenderContext();

    QgsRenderContext( const QgsRenderContext &rh );
    QgsRenderContext &operator=( const QgsRenderContext &rh );

    /**
     * Enumeration of flags that affect rendering operations.
     * \since QGIS 2.14
     */
    enum Flag
    {
      DrawEditingInfo          = 0x01,  //!< Enable drawing of vertex markers for layers in editing mode
      ForceVectorOutput        = 0x02,  //!< Vector graphics should not be cached and drawn as raster images
      UseAdvancedEffects       = 0x04,  //!< Enable layer opacity and blending effects
      UseRenderingOptimization = 0x08,  //!< Enable vector simplification and other rendering optimizations
      DrawSelection            = 0x10,  //!< Whether vector selections should be shown in the rendered map
      DrawSymbolBounds         = 0x20,  //!< Draw bounds of symbols (for debugging/testing)
      RenderMapTile            = 0x40,  //!< Draw map such that there are no problems between adjacent tiles
      Antialiasing             = 0x80,  //!< Use antialiasing while drawing
      RenderPartialOutput      = 0x100, //!< Whether to make extra effort to update map image with partially rendered layers (better for interactive map canvas). Added in QGIS 3.0
      RenderPreviewJob         = 0x200, //!< Render is a 'canvas preview' render, and shortcuts should be taken to ensure fast rendering
    };
    Q_DECLARE_FLAGS( Flags, Flag )

    /**
     * Options for rendering text.
     * \since QGIS 3.4.3
     */
    enum TextRenderFormat
    {
      // refs for below dox: https://github.com/qgis/QGIS/pull/1286#issuecomment-39806854
      // https://github.com/qgis/QGIS/pull/8573#issuecomment-445585826

      /**
       * Always render text using path objects (AKA outlines/curves).
       *
       * This setting guarantees the best quality rendering, even when using a raster paint surface
       * (where sub-pixel path based text rendering is superior to sub-pixel text-based rendering).
       * The downside is that text is converted to paths only, so users cannot open created vector
       * outputs for post-processing in other applications and retain text editability.
       *
       * This setting also guarantees complete compatibility with the full range of formatting options available
       * through QgsTextRenderer and QgsTextFormat, some of which may not be possible to reproduce when using
       * a vector-based paint surface and TextFormatAlwaysText mode.
       *
       * A final benefit to this setting is that vector exports created using text as outlines do
       * not require all users to have the original fonts installed in order to display the
       * text in its original style.
       */
      TextFormatAlwaysOutlines,

      /**
       * Always render text as text objects.
       *
       * While this mode preserves text objects as text for post-processing in external vector editing applications,
       * it can result in rendering artifacts or poor quality rendering, depending on the text format settings.
       *
       * Even with raster based paint devices, TextFormatAlwaysText can result in inferior rendering quality
       * to TextFormatAlwaysOutlines.
       *
       * When rendering using TextFormatAlwaysText to a vector based device (e.g. PDF or SVG), care must be
       * taken to ensure that the required fonts are available to users when opening the created files,
       * or default fallback fonts will be used to display the output instead. (Although PDF exports MAY
       * automatically embed some fonts when possible, depending on the user's platform).
       */
      TextFormatAlwaysText,
    };

    /**
     * Set combination of flags that will be used for rendering.
     * \since QGIS 2.14
     */
    void setFlags( QgsRenderContext::Flags flags );

    /**
     * Enable or disable a particular flag (other flags are not affected)
     * \since QGIS 2.14
     */
    void setFlag( Flag flag, bool on = true );

    /**
     * Returns combination of flags used for rendering.
     * \since QGIS 2.14
     */
    Flags flags() const;

    /**
     * Check whether a particular flag is enabled.
     * \since QGIS 2.14
     */
    bool testFlag( Flag flag ) const;

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
     */
    const QgsRectangle &extent() const { return mExtent; }

    const QgsMapToPixel &mapToPixel() const {return mMapToPixel;}

    /**
     * Returns the scaling factor for the render to convert painter units
     * to physical sizes. This is usually equal to the number of pixels
     * per millimeter.
     * \see setScaleFactor()
     */
    double scaleFactor() const {return mScaleFactor;}

    bool renderingStopped() const {return mRenderingStopped;}

    bool forceVectorOutput() const;

    /**
     * Returns true if advanced effects such as blend modes such be used
     */
    bool useAdvancedEffects() const;

    /**
     * Used to enable or disable advanced effects such as blend modes
     */
    void setUseAdvancedEffects( bool enabled );

    bool drawEditingInformation() const;

    /**
     * Returns the renderer map scale. This will match the desired scale denominator
     * for the rendered map, eg 1000.0 for a 1:1000 map render.
     * \see setRendererScale()
     */
    double rendererScale() const {return mRendererScale;}

    /**
     * Gets access to new labeling engine (may be nullptr)
     * \note not available in Python bindings
     */
    QgsLabelingEngine *labelingEngine() const { return mLabelingEngine; } SIP_SKIP

    QColor selectionColor() const { return mSelectionColor; }

    /**
     * Returns true if vector selections should be shown in the rendered map
     * \returns true if selections should be shown
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
     */
    void setExtent( const QgsRectangle &extent ) {mExtent = extent;}

    void setDrawEditingInformation( bool b );

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
     * Sets the renderer map scale. This should match the desired scale denominator
     * for the rendered map, eg 1000.0 for a 1:1000 map render.
     * \see rendererScale()
     */
    void setRendererScale( double scale ) {mRendererScale = scale;}

    /**
     * Sets the destination QPainter for the render operation. Ownership of the painter
     * is not transferred and the QPainter destination must stay alive for the duration
     * of any rendering operations.
     * \see painter()
     */
    void setPainter( QPainter *p ) {mPainter = p;}

    void setForceVectorOutput( bool force );

    /**
     * Assign new labeling engine
     * \note not available in Python bindings
     */
    void setLabelingEngine( QgsLabelingEngine *engine2 ) { mLabelingEngine = engine2; } SIP_SKIP
    void setSelectionColor( const QColor &color ) { mSelectionColor = color; }

    /**
     * Sets whether vector selections should be shown in the rendered map
     * \param showSelection set to true if selections should be shown
     * \see showSelection
     * \see setSelectionColor
     * \since QGIS v2.4
     */
    void setShowSelection( bool showSelection );

    /**
     * Returns true if the rendering optimization (geometry simplification) can be executed
     */
    bool useRenderingOptimization() const;

    void setUseRenderingOptimization( bool enabled );

    //! Added in QGIS v2.4
    const QgsVectorSimplifyMethod &vectorSimplifyMethod() const { return mVectorSimplifyMethod; }
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
    \param tolerance the segmentation tolerance*/
    void setSegmentationTolerance( double tolerance ) { mSegmentationTolerance = tolerance; }
    //! Gets the segmentation tolerance applied when rendering curved geometries
    double segmentationTolerance() const { return mSegmentationTolerance; }

    /**
     * Sets segmentation tolerance type (maximum angle or maximum difference between curve and approximation)
    \param type the segmentation tolerance typename*/
    void setSegmentationToleranceType( QgsAbstractGeometry::SegmentationToleranceType type ) { mSegmentationToleranceType = type; }
    //! Gets segmentation tolerance type (maximum angle or maximum difference between curve and approximation)
    QgsAbstractGeometry::SegmentationToleranceType segmentationToleranceType() const { return mSegmentationToleranceType; }

    // Conversions

    /**
     * Converts a size from the specified units to painter units (pixels). The conversion respects the limits
     * specified by the optional scale parameter.
     * \see convertToMapUnits()
     * \since QGIS 3.0
     */
    double convertToPainterUnits( double size, QgsUnitTypes::RenderUnit unit, const QgsMapUnitScale &scale = QgsMapUnitScale() ) const;

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
    TextRenderFormat textRenderFormat() const
    {
      return mTextRenderFormat;
    }

    /**
     * Sets the text render \a format, which dictates how text is rendered (e.g. as paths or real text objects).
     *
     * \see textRenderFormat()
     * \since QGIS 3.4.3
     */
    void setTextRenderFormat( TextRenderFormat format )
    {
      mTextRenderFormat = format;
    }

  private:

    Flags mFlags;

    //! Painter for rendering operations
    QPainter *mPainter = nullptr;

    //! For transformation between coordinate systems. Can be invalid if on-the-fly reprojection is not used
    QgsCoordinateTransform mCoordTransform;

    /**
     * A general purpose distance and area calculator, capable of performing ellipsoid based calculations.
     * Will be used to convert meter distances to active MapUnit values for QgsUnitTypes::RenderMetersInMapUnits
     * \since QGIS 3.0
     */
    QgsDistanceArea mDistanceArea;

    QgsRectangle mExtent;

    QgsMapToPixel mMapToPixel;

    //! True if the rendering has been canceled
    bool mRenderingStopped = false;

    //! Factor to scale line widths and point marker sizes
    double mScaleFactor = 1.0;

    //! Map scale
    double mRendererScale = 1.0;

    //! Newer labeling engine implementation (can be nullptr)
    QgsLabelingEngine *mLabelingEngine = nullptr;

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

    TextRenderFormat mTextRenderFormat = TextFormatAlwaysOutlines;

#ifdef QGISDEBUG
    bool mHasTransformContext = false;
#endif
};

Q_DECLARE_OPERATORS_FOR_FLAGS( QgsRenderContext::Flags )

#endif
