/***************************************************************************
  qgsforwardrenderview.h
  --------------------------------------
  Date                 : June 2024
  Copyright            : (C) 2024 by Benoit De Mezzo and (C) 2020 by Belgacem Nedjima
  Email                : benoit dot de dot mezzo at oslandia dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSFORWARDRENDERVIEW_H
#define QGSFORWARDRENDERVIEW_H

#include "qgsabstractrenderview.h"

namespace Qt3DRender
{
  class QRenderSettings;
  class QLayer;
  class QSubtreeEnabler;
  class QTexture2D;
  class QCamera;
  class QCameraSelector;
  class QLayerFilter;
  class QRenderTargetSelector;
  class QRenderTarget;
  class QClearBuffers;
  class QFrustumCulling;
  class QRenderStateSet;
  class QDebugOverlay;
} // namespace Qt3DRender

#define SIP_NO_FILE

/**
 * \ingroup qgis_3d
 * \brief Container class that holds different objects related to forward rendering
 * \note Not available in Python bindings
 *
 * \since QGIS 3.44
 */
class QgsForwardRenderView : public QgsAbstractRenderView
{
  public:
    //! Constructor with 3D scene camera
    QgsForwardRenderView( const QString &viewName, Qt3DRender::QCamera *mainCamera );

    //! Returns a layer object used to indicate that the object is transparent
    Qt3DRender::QLayer *renderLayer() { return mRenderLayer; }

    //! Returns a layer object used to indicate that the object is transparent
    Qt3DRender::QLayer *transparentObjectLayer() { return mTransparentObjectsLayer; }

    //! Sets the clear color of the scene (background color)
    void setClearColor( const QColor &clearColor );

    //! Returns whether frustum culling is enabled
    bool isFrustumCullingEnabled() const { return mFrustumCullingEnabled; }
    //! Sets whether frustum culling is enabled
    void setFrustumCullingEnabled( bool enabled );

    //! Sets whether debug overlay is enabled
    void setDebugOverlayEnabled( bool enabled );

    //! Returns current render target selector
    Qt3DRender::QRenderTargetSelector *renderTargetSelector() { return mRenderTargetSelector; }

    virtual void updateWindowResize( int width, int height ) override;

    //! Returns forward depth texture
    Qt3DRender::QTexture2D *depthTexture() const;

    //! Returns forward color texture
    Qt3DRender::QTexture2D *colorTexture() const;

    /**
     * Setups \a nrClipPlanes clip planes in the forward pass to enable OpenGL clipping.
     * If \a nrClipPlanes is equal to 0, the clipping is disabled.
     *
     * \see removeClipPlanes()
     * \since QGIS 3.40
    */
    void addClipPlanes( int nrClipPlanes );

    /**
     * Disables OpenGL clipping
     *
     * \see addClipPlanes()
     * \since QGIS 3.40
    */
    void removeClipPlanes();

  private:
    Qt3DRender::QCamera *mMainCamera = nullptr;

    Qt3DRender::QCameraSelector *mMainCameraSelector = nullptr;
    Qt3DRender::QLayerFilter *mLayerFilter = nullptr;
    Qt3DRender::QRenderTargetSelector *mRenderTargetSelector = nullptr;

    // clip planes render state
    Qt3DRender::QRenderStateSet *mClipRenderStateSet = nullptr;

    Qt3DRender::QLayer *mRenderLayer = nullptr;
    Qt3DRender::QLayer *mTransparentObjectsLayer = nullptr;
    Qt3DRender::QClearBuffers *mClearBuffers = nullptr;
    bool mFrustumCullingEnabled = true;
    Qt3DRender::QFrustumCulling *mFrustumCulling = nullptr;
    // Forward rendering pass texture related objects:
    Qt3DRender::QTexture2D *mColorTexture = nullptr;
    Qt3DRender::QTexture2D *mDepthTexture = nullptr;
    // QDebugOverlay added in the forward pass
#if QT_VERSION >= QT_VERSION_CHECK( 5, 15, 0 )
    Qt3DRender::QDebugOverlay *mDebugOverlay = nullptr;
#endif

    /**
     * Builds the three forward passes needed by forward: one for solid objects, followed by two for transparent objects
     */
    void buildRenderPasses();

    /**
     * Build color and depth textures and add then to a new rendertarget
     */
    Qt3DRender::QRenderTarget *buildTextures();
};

#endif // QGSFORWARDRENDERVIEW_H
