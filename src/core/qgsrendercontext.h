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

class QPainter;

class QgsLabelingEngineInterface;
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

    bool forceVectorOutput() const {return mForceVectorOutput;}

    /**Returns true if advanced effects such as blend modes such be used */
    bool useAdvancedEffects() const {return mUseAdvancedEffects;}
    /**Used to enable or disable advanced effects such as blend modes */
    void setUseAdvancedEffects( bool enabled ) { mUseAdvancedEffects = enabled; }

    bool drawEditingInformation() const {return mDrawEditingInformation;}

    double rendererScale() const {return mRendererScale;}

    QgsLabelingEngineInterface* labelingEngine() const { return mLabelingEngine; }

    QColor selectionColor() const { return mSelectionColor; }

    /**Returns true if vector selections should be shown in the rendered map
     * @returns true if selections should be shown
     * @see setShowSelection
     * @see selectionColor
     * @note Added in QGIS v2.4
    */
    bool showSelection() const { return mShowSelection; }

    //setters

    /**Sets coordinate transformation. QgsRenderContext does not take ownership*/
    void setCoordinateTransform( const QgsCoordinateTransform* t );
    void setMapToPixel( const QgsMapToPixel& mtp ) {mMapToPixel = mtp;}
    void setExtent( const QgsRectangle& extent ) {mExtent = extent;}
    void setDrawEditingInformation( bool b ) {mDrawEditingInformation = b;}
    void setRenderingStopped( bool stopped ) {mRenderingStopped = stopped;}
    void setScaleFactor( double factor ) {mScaleFactor = factor;}
    void setRasterScaleFactor( double factor ) {mRasterScaleFactor = factor;}
    void setRendererScale( double scale ) {mRendererScale = scale;}
    void setPainter( QPainter* p ) {mPainter = p;}
    void setForceVectorOutput( bool force ) {mForceVectorOutput = force;}
    void setLabelingEngine( QgsLabelingEngineInterface* iface ) { mLabelingEngine = iface; }
    void setSelectionColor( const QColor& color ) { mSelectionColor = color; }

    /**Sets whether vector selections should be shown in the rendered map
     * @param showSelection set to true if selections should be shown
     * @see showSelection
     * @see setSelectionColor
     * @note Added in QGIS v2.4
    */
    void setShowSelection( const bool showSelection ) { mShowSelection = showSelection; }

    /**Returns true if the rendering optimization (geometry simplification) can be executed*/
    bool useRenderingOptimization() const { return mUseRenderingOptimization; }
    void setUseRenderingOptimization( bool enabled ) { mUseRenderingOptimization = enabled; }

    //! Added in QGIS v2.4
    const QgsVectorSimplifyMethod& vectorSimplifyMethod() const { return mVectorSimplifyMethod; }
    void setVectorSimplifyMethod( const QgsVectorSimplifyMethod& simplifyMethod ) { mVectorSimplifyMethod = simplifyMethod; }

  private:

    /**Painter for rendering operations*/
    QPainter* mPainter;

    /**For transformation between coordinate systems. Can be 0 if on-the-fly reprojection is not used*/
    const QgsCoordinateTransform* mCoordTransform;

    /**True if vertex markers for editing should be drawn*/
    bool mDrawEditingInformation;

    QgsRectangle mExtent;

    /**If true then no rendered vector elements should be cached as image*/
    bool mForceVectorOutput;

    /**Flag if advanced visual effects such as blend modes should be used. True by default*/
    bool mUseAdvancedEffects;

    QgsMapToPixel mMapToPixel;

    /**True if the rendering has been canceled*/
    bool mRenderingStopped;

    /**Factor to scale line widths and point marker sizes*/
    double mScaleFactor;

    /**Factor to scale rasters*/
    double mRasterScaleFactor;

    /**Map scale*/
    double mRendererScale;

    /**Labeling engine (can be NULL)*/
    QgsLabelingEngineInterface* mLabelingEngine;

    /**Whether selection should be shown*/
    bool mShowSelection;

    /**Color used for features that are marked as selected */
    QColor mSelectionColor;

    /**True if the rendering optimization (geometry simplification) can be executed*/
    bool mUseRenderingOptimization;

    /**Simplification object which holds the information about how to simplify the features for fast rendering */
    QgsVectorSimplifyMethod mVectorSimplifyMethod;
};

#endif
