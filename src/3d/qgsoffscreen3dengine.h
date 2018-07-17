#ifndef QGSOFFSCREEN3DENGINE_H
#define QGSOFFSCREEN3DENGINE_H

#include "qgsabstract3dengine.h"

#include <QSize>

class QOffscreenSurface;

namespace Qt3DCore
{
  class QAspectEngine;
  class QNode;
}

namespace Qt3DRender
{
  class QCameraSelector;
  class QClearBuffers;
  class QRenderAspect;
  class QRenderCapture;
  class QRenderCaptureReply;
  class QRenderTarget;
  class QRenderTargetSelector;
  class QRenderTargetOutput;
  class QRenderSurfaceSelector;
  class QTexture2D;
  class QViewport;
}

namespace Qt3DLogic
{
  class QLogicAspect;
}


class _3D_EXPORT QgsOffscreen3DEngine : public QgsAbstract3DEngine
{
    Q_OBJECT
  public:
    QgsOffscreen3DEngine();

    void setClearColor( const QColor &color ) override;
    //void setFrustumCullingEnabled( bool enabled ) override;
    void setRootEntity( Qt3DCore::QEntity *root ) override;

    Qt3DRender::QRenderSettings *renderSettings() override;
    Qt3DRender::QCamera *camera() override;
    QSize size() const override;

  private:
    void createRenderTarget();
    void createFrameGraph();

    void requestRenderCapture();

  private:

    QSize mSize;
    Qt3DRender::QCamera *mCamera = nullptr;
    QOffscreenSurface *mOffscreenSurface = nullptr;
    Qt3DRender::QRenderCaptureReply *mReply = nullptr;

    // basic Qt3D stuff
    Qt3DCore::QAspectEngine *mAspectEngine = nullptr;              // The aspect engine, which holds the scene and related aspects.
    Qt3DRender::QRenderAspect *mRenderAspect = nullptr;            // The render aspect, which deals with rendering the scene.
    Qt3DLogic::QLogicAspect *mLogicAspect = nullptr;               // The logic aspect, which runs jobs to do with synchronising frames.
    Qt3DRender::QRenderSettings *mRenderSettings = nullptr;        // The render settings, which control the general rendering behaviour.
    Qt3DCore::QNode *mSceneRoot = nullptr;                         // The scene root, which becomes a child of the engine's root entity.
    Qt3DCore::QEntity *mRoot = nullptr;

    // render target stuff
    Qt3DRender::QRenderTarget *mTextureTarget = nullptr;
    Qt3DRender::QRenderTargetOutput *mTextureOutput = nullptr;
    Qt3DRender::QTexture2D *mTexture = nullptr;
    Qt3DRender::QRenderTargetOutput *mDepthTextureOutput = nullptr;
    Qt3DRender::QTexture2D *mDepthTexture = nullptr;

    // frame graph stuff
    Qt3DRender::QRenderSurfaceSelector *mSurfaceSelector = nullptr;
    Qt3DRender::QRenderTargetSelector *mRenderTargetSelector = nullptr;
    Qt3DRender::QViewport *mViewport = nullptr;
    Qt3DRender::QClearBuffers *mClearBuffers = nullptr;
    Qt3DRender::QCameraSelector *mCameraSelector = nullptr;
    Qt3DRender::QRenderCapture *mRenderCapture = nullptr;          // The render capture node, which is appended to the frame graph.

};

#endif // QGSOFFSCREEN3DENGINE_H
