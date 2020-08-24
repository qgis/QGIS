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

#include "qgspointlightsettings.h"

class QgsDirectionalLightSettings;
class QgsCameraController;
class QgsRectangle;
class QgsPostprocessingEntity;
class QgsPreviewQuad;

class QgsShadowRenderingFrameGraph
{
  public:
    QgsShadowRenderingFrameGraph( QWindow *window, Qt3DRender::QCamera *mainCamera, Qt3DCore::QEntity *root );

    Qt3DRender::QFrameGraphNode *getFrameGraphRoot() { return mRenderSurfaceSelector; }
    Qt3DRender::QTexture2D *forwardRenderColorTexture() { return mForwardColorTexture; }
    Qt3DRender::QTexture2D *forwardRenderDepthTexture() { return mForwardDepthTexture; }
    Qt3DRender::QTexture2D *shadowMapTexture() { return mShadowMapTexture; }
    Qt3DRender::QLayer *postprocessingPassLayer() { return mPostprocessPassLayer; }
    Qt3DRender::QCamera *mainCamera() { return mMainCamera; }
    Qt3DRender::QCamera *lightCamera() { return mLightCamera; }
    Qt3DRender::QLayer *previewLayer() { return mPreviewLayer; }
    Qt3DRender::QLayer *doNotCastShadowsLayerLayer() { return mDoNotCastShadowsLayer; }
    QgsPostprocessingEntity *postprocessingEntity() { return mPostprocessingEntity; }
    Qt3DCore::QEntity *rootEntity() { return mRootEntity; }

    bool frustumCullingEnabled() { return mFrustumCullingEnabled; }
    void setFrustumCullingEnabled( bool enabled );
    void setShadowRenderingEnabled( bool enabled );
    bool shadowRenderingEnabled() { return mShadowRenderingEnabled; }

    void setClearColor( const QColor &clearColor );
    void addTexturePreviewOverlay( Qt3DRender::QTexture2D *texture, const QPointF &centerNDC, const QSizeF &size, QVector<Qt3DRender::QParameter *> additionalShaderParameters = QVector<Qt3DRender::QParameter *>() );
    void setupDirectionalLight( const QgsDirectionalLightSettings &light, float maximumShadowRenderingDistance );
  private:
    Qt3DRender::QRenderSurfaceSelector *mRenderSurfaceSelector = nullptr;
    Qt3DRender::QViewport *mMainViewPort = nullptr;
    Qt3DRender::QCameraSelector *mMainCameraSelector = nullptr;
    Qt3DRender::QLayerFilter *mSceneEntitiesFilter = nullptr;
    Qt3DRender::QRenderTargetSelector *mForwardRenderTargetSelector = nullptr;
    Qt3DRender::QRenderTarget *mForwardRenderTarget = nullptr;
    Qt3DRender::QRenderTargetOutput *mForwardRenderTargetColorOutput = nullptr;
    Qt3DRender::QRenderTargetOutput *mForwardRenderTargetDepthOutput = nullptr;
    Qt3DRender::QTexture2D *mForwardColorTexture = nullptr;
    Qt3DRender::QTexture2D *mForwardDepthTexture = nullptr;
    Qt3DRender::QClearBuffers *mForwardClearBuffers = nullptr;

    Qt3DRender::QCamera *mMainCamera = nullptr;
    Qt3DRender::QLayer *mPostprocessPassLayer = nullptr;
    Qt3DRender::QLayerFilter *mPostprocessPassLayerFilter = nullptr;
    Qt3DRender::QClearBuffers *mPostprocessClearBuffers = nullptr;

    Qt3DRender::QRenderTargetSelector *mShadowRenderTargetSelector = nullptr;
    Qt3DRender::QRenderTarget *mShadowRenderTarget = nullptr;
    Qt3DRender::QRenderTargetOutput *mShadowRenderTargetOutput = nullptr;
    Qt3DRender::QTexture2D *mShadowMapTexture = nullptr;
    Qt3DRender::QClearBuffers *mShadowClearBuffers = nullptr;
    Qt3DRender::QCamera *mLightCamera = nullptr;
    Qt3DRender::QCameraSelector *mLightCameraSelector = nullptr;
    Qt3DRender::QFrustumCulling *mFrustumCulling = nullptr;
    bool mFrustumCullingEnabled = true;
    bool mShadowRenderingEnabled = false;

    QVector3D mLightDirection = QVector3D( 0.0, -1.0f, 0.0f );

    Qt3DCore::QEntity *mRootEntity = nullptr;

    Qt3DRender::QLayer *mPreviewLayer = nullptr;
    Qt3DRender::QLayer *mDoNotCastShadowsLayer = nullptr;
    QgsPostprocessingEntity *mPostprocessingEntity = nullptr;

    QVector<QgsPreviewQuad *> mPreviewQuads;

    Qt3DRender::QFrameGraphNode *constructShadowRenderPass();
    Qt3DRender::QFrameGraphNode *constructForwardRenderPass();
    Qt3DRender::QFrameGraphNode *constructTexturesPreviewPass();
    Qt3DRender::QFrameGraphNode *constructPostprocessingPass();
};

#endif // QGSSHADOWRENDERINGFRAMEGRAPH_H
