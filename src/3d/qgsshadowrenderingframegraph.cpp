#include "qgsshadowrenderingframegraph.h"

QgsShadowRenderingFrameGraph::QgsShadowRenderingFrameGraph(QWindow *window, Qt3DRender::QCamera *mainCamera)
{
  mMainCamera = mainCamera;

  // Frame graph:
  // mRenderSurfaceSelector
  //   mMainViewPort
  //     mMainCameraSelector
  //       mSceneEntitiesFilter
  //         mForwardRenderTargetSelector
  //           mForwardClearBuffers
  //       mPostprocessPassLayerFilter
  //         mPostprocessClearBuffers

  mRenderSurfaceSelector = new Qt3DRender::QRenderSurfaceSelector;
  mRenderSurfaceSelector->setSurface(window);

  mForwardClearBuffers = new Qt3DRender::QClearBuffers(mRenderSurfaceSelector);
  mForwardClearBuffers->setClearColor(QColor::fromRgbF(1.0f, 1.0f, 1.0f));
  mForwardClearBuffers->setBuffers(Qt3DRender::QClearBuffers::ColorDepthBuffer);

  mMainViewPort = new Qt3DRender::QViewport(mForwardClearBuffers);
  mMainViewPort->setNormalizedRect(QRectF(0.0f, 0.0f, 1.0f, 1.0f));

  mMainCameraSelector = new Qt3DRender::QCameraSelector(mMainViewPort);
  mMainCameraSelector->setCamera(mainCamera);

  // Forward render
  mSceneEntitiesFilter = new Qt3DRender::QLayerFilter(mMainCameraSelector);
  mPostprocessPassLayer = new Qt3DRender::QLayer;//(mSceneEntitiesFilter);
  mPostprocessPassLayer->setRecursive(true);

  mSceneEntitiesFilter->addLayer(mPostprocessPassLayer);
  mSceneEntitiesFilter->setFilterMode(Qt3DRender::QLayerFilter::FilterMode::DiscardAnyMatchingLayers);

  mForwardColorTexture = new Qt3DRender::QTexture2D;
  mForwardColorTexture->setWidth(1024);
  mForwardColorTexture->setHeight(1024);
  mForwardColorTexture->setFormat( Qt3DRender::QTexture2D::TextureFormat::RGBA16F );
  mForwardColorTexture->setGenerateMipMaps( false );
  mForwardColorTexture->setMagnificationFilter( Qt3DRender::QTexture2D::Linear );
  mForwardColorTexture->setMinificationFilter( Qt3DRender::QTexture2D::Linear );
  mForwardColorTexture->wrapMode()->setX( Qt3DRender::QTextureWrapMode::ClampToEdge );
  mForwardColorTexture->wrapMode()->setY( Qt3DRender::QTextureWrapMode::ClampToEdge );

  mForwardDepthTexture = new Qt3DRender::QTexture2D;
  mForwardDepthTexture->setWidth(1024);
  mForwardDepthTexture->setHeight(1024);
  mForwardDepthTexture->setFormat( Qt3DRender::QTexture2D::TextureFormat::DepthFormat );
  mForwardDepthTexture->setGenerateMipMaps( false );
  mForwardDepthTexture->setMagnificationFilter( Qt3DRender::QTexture2D::Linear );
  mForwardDepthTexture->setMinificationFilter( Qt3DRender::QTexture2D::Linear );
  mForwardDepthTexture->wrapMode()->setX( Qt3DRender::QTextureWrapMode::ClampToEdge );
  mForwardDepthTexture->wrapMode()->setY( Qt3DRender::QTextureWrapMode::ClampToEdge );
  mForwardDepthTexture->setComparisonFunction( Qt3DRender::QTexture2D::ComparisonFunction::CompareLessEqual );
  mForwardDepthTexture->setComparisonMode( Qt3DRender::QTexture2D::ComparisonMode::CompareRefToTexture );

  mForwardRenderTarget = new Qt3DRender::QRenderTarget;
  mForwardRenderTargetDepthOutput = new Qt3DRender::QRenderTargetOutput;
  mForwardRenderTargetDepthOutput->setAttachmentPoint( Qt3DRender::QRenderTargetOutput::Depth );
  mForwardRenderTargetDepthOutput->setTexture(mForwardDepthTexture);
  mForwardRenderTarget->addOutput(mForwardRenderTargetDepthOutput);
  mForwardRenderTargetColorOutput = new Qt3DRender::QRenderTargetOutput;
  mForwardRenderTargetColorOutput->setAttachmentPoint( Qt3DRender::QRenderTargetOutput::Color0 );
  mForwardRenderTargetColorOutput->setTexture(mForwardColorTexture);
  mForwardRenderTarget->addOutput(mForwardRenderTargetColorOutput);

  mForwardRenderTargetSelector = new Qt3DRender::QRenderTargetSelector(mSceneEntitiesFilter);
  mForwardRenderTargetSelector->setTarget(mForwardRenderTarget);

  mForwardClearBuffers = new Qt3DRender::QClearBuffers(mForwardRenderTargetSelector);
  mForwardClearBuffers->setClearColor( QColor::fromRgbF(0.0, 1.0, 0.0, 1.0) );
  mForwardClearBuffers->setBuffers(Qt3DRender::QClearBuffers::ColorDepthBuffer);

  mPostprocessPassLayerFilter = new Qt3DRender::QLayerFilter(mMainCameraSelector);
  mPostprocessPassLayerFilter->addLayer(mPostprocessPassLayer);
  mPostprocessPassLayerFilter->setFilterMode(Qt3DRender::QLayerFilter::FilterMode::AcceptAnyMatchingLayers);

  mPostprocessClearBuffers = new Qt3DRender::QClearBuffers(mPostprocessPassLayerFilter);
  mPostprocessClearBuffers->setClearColor(QColor::fromRgbF(0.0f, 0.0f, 1.0f));
}
