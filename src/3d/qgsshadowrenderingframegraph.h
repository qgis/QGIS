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


class QgsShadowRenderingFrameGraph
{
  public:
    QgsShadowRenderingFrameGraph(QWindow *window, Qt3DRender::QCamera *mainCamera);

    Qt3DRender::QFrameGraphNode *getFrameGraphRoot() { return mRenderSurfaceSelector; }
    Qt3DRender::QTexture2D *forwardRenderColorTexture() { return mForwardColorTexture; }
    Qt3DRender::QTexture2D *forwardRenderDepthTexture() { return mForwardDepthTexture; }
    Qt3DRender::QLayer *postprocessingPassLayer() { return mPostprocessPassLayer; }
    Qt3DRender::QCamera *mainCamera() { return mMainCamera; }
  private:
    Qt3DRender::QRenderSurfaceSelector *mRenderSurfaceSelector = nullptr;
    Qt3DRender::QViewport *mMainViewPort = nullptr;
    Qt3DRender::QCameraSelector *mMainCameraSelector = nullptr;
    Qt3DRender::QLayerFilter *mSceneEntitiesFilter = nullptr;
    Qt3DRender::QRenderTargetSelector *mForwardRenderTargetSelector = nullptr;
    Qt3DRender::QRenderTarget *mForwardRenderTarget = nullptr;
    Qt3DRender::QRenderTargetOutput* mForwardRenderTargetColorOutput = nullptr;
    Qt3DRender::QRenderTargetOutput* mForwardRenderTargetDepthOutput = nullptr;
    Qt3DRender::QTexture2D *mForwardColorTexture = nullptr;
    Qt3DRender::QTexture2D *mForwardDepthTexture = nullptr;
    Qt3DRender::QClearBuffers *mForwardClearBuffers = nullptr;

    Qt3DRender::QCamera *mMainCamera = nullptr;
    Qt3DRender::QLayer *mPostprocessPassLayer = nullptr;
    Qt3DRender::QLayerFilter *mPostprocessPassLayerFilter = nullptr;
    Qt3DRender::QClearBuffers *mPostprocessClearBuffers = nullptr;
};

#endif // QGSSHADOWRENDERINGFRAMEGRAPH_H
