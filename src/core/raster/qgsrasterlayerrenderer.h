/***************************************************************************
  qgsrasterlayerrenderer.h
  --------------------------------------
  Date                 : December 2013
  Copyright            : (C) 2013 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSRASTERLAYERRENDERER_H
#define QGSRASTERLAYERRENDERER_H

#define SIP_NO_FILE

#include "qgsmapclippingregion.h"
#include "qgsmaplayerrenderer.h"

#include <functional>

class QPainter;

class QgsMapToPixel;
class QgsRasterLayer;
class QgsRasterPipe;
struct QgsRasterViewPort;
class QgsRenderContext;

class QgsRasterLayerRenderer;
class QgsRasterLayerLabelProvider;

#include "qgsrasterinterface.h"

///@cond PRIVATE

/**
 * \ingroup core
 * \brief Specific internal feedback class to provide preview of raster layer rendering.
 * \note not available in Python bindings
 */
class CORE_EXPORT QgsRasterLayerRendererFeedback : public QgsRasterBlockFeedback
{
    Q_OBJECT

  public:
    //! Create feedback object based on our layer renderer
    explicit QgsRasterLayerRendererFeedback( QgsRasterLayerRenderer *r );

    //! when notified of new data in data provider it launches a preview draw of the raster
    void onNewData() override;
  private:
    QgsRasterLayerRenderer *mR = nullptr;   //!< Parent renderer instance
    int mMinimalPreviewInterval = 250;  //!< In milliseconds
    QTime mLastPreview;           //!< When last preview has been generated
};

///@endcond

/**
 * \ingroup core
 * \brief Implementation of threaded rendering for raster layers.
 *
 * \note not available in Python bindings
 */
class CORE_EXPORT QgsRasterLayerRenderer : public QgsMapLayerRenderer
{
  public:
    QgsRasterLayerRenderer( QgsRasterLayer *layer, QgsRenderContext &rendererContext );
    ~QgsRasterLayerRenderer() override;

    bool render() override;
    QgsFeedback *feedback() const override;
    bool forceRasterRender() const override;

    /**
     * \brief GPU renderer factory function type
     *
     * Factory function that attempts GPU-accelerated rendering.
     * Returns TRUE if rendering succeeded via GPU, FALSE to fallback to CPU.
     *
     * \since QGIS 3.40
     */
    using GpuRendererFactory = std::function<bool(
      QgsRenderContext &context,
      QgsRasterViewPort *viewport,
      QgsRasterPipe *pipe,
      QgsFeedback *feedback
    )>;

    /**
     * \brief Set GPU renderer factory
     *
     * Registers a factory function that will be called to attempt GPU rendering
     * before falling back to CPU. Set to nullptr to disable GPU rendering.
     *
     * This allows GUI code to inject GPU rendering capability into core rendering.
     *
     * \param factory GPU renderer factory function, or nullptr to disable
     * \since QGIS 3.40
     */
    static void setGpuRendererFactory( GpuRendererFactory factory );

    /**
     * \brief Check if GPU rendering is available
     * \returns TRUE if a GPU renderer factory is registered
     * \since QGIS 3.40
     */
    static bool gpuRenderingAvailable();

  private:

    void prepareLabeling( QgsRasterLayer *layer );
    void drawLabeling();

    QString mLayerName;
    QgsRasterViewPort *mRasterViewPort = nullptr;

    double mLayerOpacity = 1.0;
    std::unique_ptr<QgsRasterPipe> mPipe;

    Qgis::RasterProviderCapabilities mProviderCapabilities;
    Qgis::RasterInterfaceCapabilities mInterfaceCapabilities;

    //! feedback class for cancellation and preview generation
    QgsRasterLayerRendererFeedback *mFeedback = nullptr;

    QList< QgsMapClippingRegion > mClippingRegions;

    double mElevationScale = 1.0;
    double mElevationOffset = 0.0;
    int mElevationBand = 0;
    bool mDrawElevationMap = false;

    bool mEnableProfile = false;
    quint64 mPreparationTime = 0;

    // may be NULLPTR. no need to delete: if exists it is owned by labeling engine
    QgsRasterLayerLabelProvider *mLabelProvider = nullptr;

    void drawElevationMap();

    //! Static GPU renderer factory
    static GpuRendererFactory sGpuRendererFactory;

    //! Whether to attempt GPU rendering for this layer
    bool mTryGpuPath = false;

    friend class QgsRasterLayerRendererFeedback;
};


#endif // QGSRASTERLAYERRENDERER_H
