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

#include <QColor>

#include "qgscoordinatetransform.h"
#include "qgsmaptopixel.h"
#include "qgsrectangle.h"
#include "qgsvectorsimplifymethod.h"
#include "qgsexpressioncontext.h"

class QPainter;

class QgsAbstractGeometryV2;
class QgsLabelingEngineInterface;
class QgsLabelingEngineV2;
class QgsMapSettings;

/** \ingroup core
 * Contains information about the context of a rendering operation.
 * The context of a rendering operation defines properties such as
 * the conversion ratio between screen and map units, the extents /
 * bounding box to be rendered etc.
 **/
class CORE_EXPORT QgsRenderContext
{
  public:
    QgsRenderContext();
    ~QgsRenderContext();

    /** Enumeration of flags that affect rendering operations.
     * @note added in QGIS 2.14
     */
    enum Flag
    {
      DrawEditingInfo    = 0x01,  //!< Enable drawing of vertex markers for layers in editing mode
      ForceVectorOutput  = 0x02,  //!< Vector graphics should not be cached and drawn as raster images
      UseAdvancedEffects = 0x04,  //!< Enable layer transparency and blending effects
      UseRenderingOptimization = 0x08, //!< Enable vector simplification and other rendering optimizations
      DrawSelection      = 0x10,  //!< Whether vector selections should be shown in the rendered map
    };
    Q_DECLARE_FLAGS( Flags, Flag )

    /** Set combination of flags that will be used for rendering.
     * @note added in QGIS 2.14
     */
    void setFlags( const QgsRenderContext::Flags& flags );

    /** Enable or disable a particular flag (other flags are not affected)
     * @note added in QGIS 2.14
     */
    void setFlag( Flag flag, bool on = true );

    /** Return combination of flags used for rendering.
     * @note added in QGIS 2.14
     */
    Flags flags() const;

    /** Check whether a particular flag is enabled.
     * @note added in QGIS 2.14
     */
    bool testFlag( Flag flag ) const;

    //! create initialized QgsRenderContext instance from given QgsMapSettings
    //! @note added in 2.4
    static QgsRenderContext fromMapSettings( const QgsMapSettings& mapSettings );

    //getters

    QPainter* painter() {return mPainter;}
    const QPainter* constPainter() const { return mPainter; }

    const QgsCoordinateTransform* coordinateTransform() const {return mCoordTransform;}

    const QgsRectangle& extent() const {return mExtent;}

    const QgsMapToPixel& mapToPixel() const {return mMapToPixel;}

    double scaleFactor() const {return mScaleFactor;}

    double rasterScaleFactor() const {return mRasterScaleFactor;}

    bool renderingStopped() const {return mRenderingStopped;}

    bool forceVectorOutput() const;

    /** Returns true if advanced effects such as blend modes such be used
    */
    bool useAdvancedEffects() const;

    /** Used to enable or disable advanced effects such as blend modes
    */
    void setUseAdvancedEffects( bool enabled );

    bool drawEditingInformation() const;

    double rendererScale() const {return mRendererScale;}

    QgsLabelingEngineInterface* labelingEngine() const { return mLabelingEngine; }

    //! Get access to new labeling engine (may be NULL)
    QgsLabelingEngineV2* labelingEngineV2() const { return mLabelingEngine2; }

    QColor selectionColor() const { return mSelectionColor; }

    /** Returns true if vector selections should be shown in the rendered map
     * @returns true if selections should be shown
     * @see setShowSelection
     * @see selectionColor
     * @note Added in QGIS v2.4
    */
    bool showSelection() const;

    //setters

    /** Sets coordinate transformation. QgsRenderContext does not take ownership*/
    void setCoordinateTransform( const QgsCoordinateTransform* t );
    void setMapToPixel( const QgsMapToPixel& mtp ) {mMapToPixel = mtp;}
    void setExtent( const QgsRectangle& extent ) {mExtent = extent;}

    void setDrawEditingInformation( bool b );

    void setRenderingStopped( bool stopped ) {mRenderingStopped = stopped;}
    void setScaleFactor( double factor ) {mScaleFactor = factor;}
    void setRasterScaleFactor( double factor ) {mRasterScaleFactor = factor;}
    void setRendererScale( double scale ) {mRendererScale = scale;}
    void setPainter( QPainter* p ) {mPainter = p;}

    void setForceVectorOutput( bool force );

    void setLabelingEngine( QgsLabelingEngineInterface* iface ) { mLabelingEngine = iface; }
    //! Assign new labeling engine
    void setLabelingEngineV2( QgsLabelingEngineV2* engine2 ) { mLabelingEngine2 = engine2; }
    void setSelectionColor( const QColor& color ) { mSelectionColor = color; }

    /** Sets whether vector selections should be shown in the rendered map
     * @param showSelection set to true if selections should be shown
     * @see showSelection
     * @see setSelectionColor
     * @note Added in QGIS v2.4
    */
    void setShowSelection( const bool showSelection );

    /** Returns true if the rendering optimization (geometry simplification) can be executed
    */
    bool useRenderingOptimization() const;

    void setUseRenderingOptimization( bool enabled );

    //! Added in QGIS v2.4
    const QgsVectorSimplifyMethod& vectorSimplifyMethod() const { return mVectorSimplifyMethod; }
    void setVectorSimplifyMethod( const QgsVectorSimplifyMethod& simplifyMethod ) { mVectorSimplifyMethod = simplifyMethod; }

    /** Sets the expression context. This context is used for all expression evaluation
     * associated with this render context.
     * @see expressionContext()
     * @note added in QGIS 2.12
     */
    void setExpressionContext( const QgsExpressionContext& context ) { mExpressionContext = context; }

    /** Gets the expression context. This context should be used for all expression evaluation
     * associated with this render context.
     * @see setExpressionContext()
     * @note added in QGIS 2.12
     */
    QgsExpressionContext& expressionContext() { return mExpressionContext; }

    /** Gets the expression context (const version). This context should be used for all expression evaluation
     * associated with this render context.
     * @see setExpressionContext()
     * @note added in QGIS 2.12
     * @note not available in Python bindings
     */
    const QgsExpressionContext& expressionContext() const { return mExpressionContext; }

    /** Returns pointer to the unsegmentized geometry*/
    const QgsAbstractGeometryV2* geometry() const { return mGeometry; }
    /** Sets pointer to original (unsegmentized) geometry*/
    void setGeometry( const QgsAbstractGeometryV2* geometry ) { mGeometry = geometry; }

  private:

    Flags mFlags;

    /** Painter for rendering operations*/
    QPainter* mPainter;

    /** For transformation between coordinate systems. Can be 0 if on-the-fly reprojection is not used*/
    const QgsCoordinateTransform* mCoordTransform;

    QgsRectangle mExtent;

    QgsMapToPixel mMapToPixel;

    /** True if the rendering has been canceled*/
    bool mRenderingStopped;

    /** Factor to scale line widths and point marker sizes*/
    double mScaleFactor;

    /** Factor to scale rasters*/
    double mRasterScaleFactor;

    /** Map scale*/
    double mRendererScale;

    /** Labeling engine (can be NULL)*/
    QgsLabelingEngineInterface* mLabelingEngine;

    /** Newer labeling engine implementation (can be NULL) */
    QgsLabelingEngineV2* mLabelingEngine2;

    /** Color used for features that are marked as selected */
    QColor mSelectionColor;

    /** Simplification object which holds the information about how to simplify the features for fast rendering */
    QgsVectorSimplifyMethod mVectorSimplifyMethod;

    /** Expression context */
    QgsExpressionContext mExpressionContext;

    /** Pointer to the (unsegmentized) geometry*/
    const QgsAbstractGeometryV2* mGeometry;
};

Q_DECLARE_OPERATORS_FOR_FLAGS( QgsRenderContext::Flags )

#endif
