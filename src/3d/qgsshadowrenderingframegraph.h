/***************************************************************************
  qgsshadowrenderingframegraph.h
  --------------------------------------
  Date                 : August 2020
  Copyright            : (C) 2020 by Belgacem Nedjima
  Email                : gb underscore nedjima at esi dot dz
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSSHADOWRENDERINGFRAMEGRAPH_H
#define QGSSHADOWRENDERINGFRAMEGRAPH_H

#include <QWindow>
#include <Qt3DRender/QCamera>
#include <Qt3DRender/QRenderSurfaceSelector>
#include <Qt3DRender/QViewport>
#include <Qt3DRender/QCameraSelector>
#include <Qt3DRender/QLayerFilter>
#include <Qt3DRender/QLayer>
#include <Qt3DRender/QRenderTargetSelector>
#include <Qt3DRender/QRenderTarget>
#include <Qt3DRender/QTexture>
#include <Qt3DRender/QClearBuffers>
#include <Qt3DRender/QParameter>
#include <Qt3DRender/QFrustumCulling>
#include <Qt3DRender/QRenderStateSet>
#include <Qt3DRender/QDepthTest>
#include <Qt3DRender/QCullFace>
#include <Qt3DRender/QPolygonOffset>
#include <Qt3DRender/QRenderCapture>

#include "qgspointlightsettings.h"

class QgsDirectionalLightSettings;
class QgsCameraController;
class QgsRectangle;
class QgsPostprocessingEntity;
class QgsPreviewQuad;

#define SIP_NO_FILE

/**
 * \ingroup 3d
 * \brief Container class that holds different objects related to shadow rendering
 *
 * \note Not available in Python bindings
 *
 * \since QGIS 3.16
 */
class QgsShadowRenderingFrameGraph : public Qt3DCore::QEntity
{
  public:
    //! Constructor
    QgsShadowRenderingFrameGraph( QSurface *surface, QSize s, Qt3DRender::QCamera *mainCamera, Qt3DCore::QEntity *root );

    //! Returns the root of the frame graph object
    Qt3DRender::QFrameGraphNode *frameGraphRoot() { return mRenderSurfaceSelector; }

    //! Returns the color texture of the forward rendering pass
    Qt3DRender::QTexture2D *forwardRenderColorTexture() { return mForwardColorTexture; }
    //! Returns the depth texture of the forward rendering pass
    Qt3DRender::QTexture2D *forwardRenderDepthTexture() { return mForwardDepthTexture; }
    //! Returns the shadow map (a depth texture for the shadow rendering pass)
    Qt3DRender::QTexture2D *shadowMapTexture() { return mShadowMapTexture; }

    //! Returns a layer object used to indicate that an entity is to be rendered during the post processing rendering pass
    Qt3DRender::QLayer *postprocessingPassLayer() { return mPostprocessPassLayer; }
    //! Returns a layer object used to indicate that an entity is to be rendered during the preview textures rendering pass
    Qt3DRender::QLayer *previewLayer() { return mPreviewLayer; }
    //! Returns a layer object used to indicate that an entity will cast shadows
    Qt3DRender::QLayer *castShadowsLayer() { return mCastShadowsLayer; }
    //! Returns a layer object used to indicate that an entity will be rendered during the forward rendering pass
    Qt3DRender::QLayer *forwardRenderLayer() { return mForwardRenderLayer; }

    /**
     * Returns a layer object used to indicate that the object is transparent
     * \since QGIS 3.26
     */
    Qt3DRender::QLayer *transparentObjectLayer() { return mTransparentObjectsPassLayer; }

    //! Returns the main camera
    Qt3DRender::QCamera *mainCamera() { return mMainCamera; }
    //! Returns the light camera
    Qt3DRender::QCamera *lightCamera() { return mLightCamera; }
    //! Returns the postprocessing entity
    QgsPostprocessingEntity *postprocessingEntity() { return mPostprocessingEntity; }
    //! Returns the root entity of the entities related to the frame graph (like the post processing entity and preview entity)
    Qt3DCore::QEntity *rootEntity() { return mRootEntity; }

    //! Returns the render capture object used to take an image of the scene
    Qt3DRender::QRenderCapture *renderCapture() { return mRenderCapture; }

    //! Returns the render capture object used to take an image of the depth buffer of the scene
    Qt3DRender::QRenderCapture *depthRenderCapture() { return mDepthRenderCapture; }


    //! Returns whether frustum culling is enabled
    bool frustumCullingEnabled() const { return mFrustumCullingEnabled; }
    //! Sets whether frustum culling is enabled
    void setFrustumCullingEnabled( bool enabled );

    //! Returns whether shadow rendering is enabled
    bool shadowRenderingEnabled() const { return mShadowRenderingEnabled; }
    //! Sets whether the shadow rendering is enabled
    void setShadowRenderingEnabled( bool enabled );

    //! Returns the shadow bias value
    float shadowBias() const { return mShadowBias; }
    //! Sets the shadow bias value
    void setShadowBias( float shadowBias );

    //! Returns the shadow map resolution
    int shadowMapResolution() const { return mShadowMapResolution; }
    //! Sets the resolution of the shadow map
    void setShadowMapResolution( int resolution );

    //! Sets the clear color of the scene (background color)
    void setClearColor( const QColor &clearColor );
    //! Adds an preview entity that shows a texture in real time for debugging purposes
    QgsPreviewQuad *addTexturePreviewOverlay( Qt3DRender::QTexture2D *texture, const QPointF &centerNDC, const QSizeF &size, QVector<Qt3DRender::QParameter *> additionalShaderParameters = QVector<Qt3DRender::QParameter *>() );
    //! Sets shadow rendering to use a directional light
    void setupDirectionalLight( const QgsDirectionalLightSettings &light, float maximumShadowRenderingDistance );
    //! Sets eye dome lighting shading related settings
    void setupEyeDomeLighting( bool enabled, double strength, int distance );
    //! Sets the shadow map debugging view port
    void setupShadowMapDebugging( bool enabled, Qt::Corner corner, double size );
    //! Sets the depth map debugging view port
    void setupDepthMapDebugging( bool enabled, Qt::Corner corner, double size );
    //! Sets the size of the buffers used for rendering
    void setSize( QSize s );

    /**
     * Sets whether it will be possible to render to an image
     * \since QGIS 3.18
     */
    void setRenderCaptureEnabled( bool enabled );

    /**
     * Returns whether it will be possible to render to an image
     * \since QGIS 3.18
     */
    bool renderCaptureEnabled() const { return mRenderCaptureEnabled; }
  private:
    Qt3DRender::QRenderSurfaceSelector *mRenderSurfaceSelector = nullptr;
    Qt3DRender::QViewport *mMainViewPort = nullptr;
    bool mFrustumCullingEnabled = true;

    Qt3DRender::QCamera *mMainCamera = nullptr;
    Qt3DRender::QCamera *mLightCamera = nullptr;

    // Forward rendering pass branch nodes:
    Qt3DRender::QCameraSelector *mMainCameraSelector = nullptr;
    Qt3DRender::QLayerFilter *mForwardRenderLayerFilter = nullptr;
    Qt3DRender::QRenderTargetSelector *mForwardRenderTargetSelector = nullptr;
    Qt3DRender::QClearBuffers *mForwardClearBuffers = nullptr;
    Qt3DRender::QFrustumCulling *mFrustumCulling = nullptr;
    // Forward rendering pass texture related objects:
    Qt3DRender::QTexture2D *mForwardColorTexture = nullptr;
    Qt3DRender::QTexture2D *mForwardDepthTexture = nullptr;

    // Shadow rendering pass branch nodes:
    Qt3DRender::QCameraSelector *mLightCameraSelectorShadowPass = nullptr;
    Qt3DRender::QLayerFilter *mShadowSceneEntitiesFilter = nullptr;
    Qt3DRender::QRenderTargetSelector *mShadowRenderTargetSelector = nullptr;
    Qt3DRender::QClearBuffers *mShadowClearBuffers = nullptr;
    Qt3DRender::QRenderStateSet *mShadowRenderStateSet = nullptr;
    // Shadow rendering pass texture related objects:
    Qt3DRender::QTexture2D *mShadowMapTexture = nullptr;

    // - The depth buffer render pass is made to copy the depth buffer into
    //    an RGB texture that can be captured into a QImage and sent to the CPU for
    //    calculating real 3D points from mouse coordinates (for zoom, rotation, drag..)
    // Depth buffer render pass branch nodes:
    Qt3DRender::QCameraSelector *mDepthRenderCameraSelector = nullptr;
    Qt3DRender::QRenderStateSet *mDepthRenderStateSet = nullptr;;
    Qt3DRender::QLayerFilter *mDepthRenderLayerFilter = nullptr;
    Qt3DRender::QRenderTargetSelector *mDepthRenderCaptureTargetSelector = nullptr;
    Qt3DRender::QRenderCapture *mDepthRenderCapture = nullptr;
    // Depth buffer processing pass texture related objects:
    Qt3DRender::QTexture2D *mDepthRenderCaptureDepthTexture = nullptr;
    Qt3DRender::QTexture2D *mDepthRenderCaptureColorTexture = nullptr;

    // Post processing pass branch nodes:
    Qt3DRender::QCameraSelector *mPostProcessingCameraSelector = nullptr;
    Qt3DRender::QLayerFilter *mPostprocessPassLayerFilter = nullptr;
    Qt3DRender::QClearBuffers *mPostprocessClearBuffers = nullptr;
    Qt3DRender::QRenderTargetSelector *mRenderCaptureTargetSelector = nullptr;
    Qt3DRender::QRenderCapture *mRenderCapture = nullptr;
    // Post processing pass texture related objects:
    Qt3DRender::QTexture2D *mRenderCaptureColorTexture = nullptr;
    Qt3DRender::QTexture2D *mRenderCaptureDepthTexture = nullptr;

    // Texture preview:
    Qt3DRender::QLayerFilter *mPreviewLayerFilter = nullptr;
    Qt3DRender::QRenderStateSet *mPreviewRenderStateSet = nullptr;
    Qt3DRender::QDepthTest *mPreviewDepthTest = nullptr;
    Qt3DRender::QCullFace *mPreviewCullFace = nullptr;

    bool mShadowRenderingEnabled = false;
    float mShadowBias = 0.00001f;
    int mShadowMapResolution = 2048;

    QSize mSize = QSize( 1024, 768 );

    bool mEyeDomeLightingEnabled = false;
    double mEyeDomeLightingStrength = 1000.0;
    int mEyeDomeLightingDistance = 1;

    QgsPreviewQuad *mDebugShadowMapPreviewQuad = nullptr;
    QgsPreviewQuad *mDebugDepthMapPreviewQuad = nullptr;

    QEntity *mDepthRenderQuad = nullptr;

    QVector3D mLightDirection = QVector3D( 0.0, -1.0f, 0.0f );

    Qt3DCore::QEntity *mRootEntity = nullptr;

    Qt3DRender::QLayer *mPostprocessPassLayer = nullptr;
    Qt3DRender::QLayer *mPreviewLayer = nullptr;
    Qt3DRender::QLayer *mForwardRenderLayer = nullptr;
    Qt3DRender::QLayer *mCastShadowsLayer = nullptr;
    Qt3DRender::QLayer *mDepthRenderPassLayer = nullptr;
    Qt3DRender::QLayer *mTransparentObjectsPassLayer = nullptr;

    QgsPostprocessingEntity *mPostprocessingEntity = nullptr;

    QVector<QgsPreviewQuad *> mPreviewQuads;

    Qt3DRender::QFrameGraphNode *constructShadowRenderPass();
    Qt3DRender::QFrameGraphNode *constructForwardRenderPass();
    Qt3DRender::QFrameGraphNode *constructTexturesPreviewPass();
    Qt3DRender::QFrameGraphNode *constructPostprocessingPass();
    Qt3DRender::QFrameGraphNode *constructDepthRenderPass();

    Qt3DCore::QEntity *constructDepthRenderQuad();

    bool mRenderCaptureEnabled = true;

    Q_DISABLE_COPY( QgsShadowRenderingFrameGraph )
};

#endif // QGSSHADOWRENDERINGFRAMEGRAPH_H
