#include "qgsshadowrenderingframegraph.h"

#include "qgsdirectionallightsettings.h"
#include "qgscameracontroller.h"
#include "qgsrectangle.h"
#include "qgspostprocessingentity.h"
#include "qgspreviewquad.h"

#include <Qt3DRender/QRenderStateSet>
#include <Qt3DRender/QDepthTest>
#include <Qt3DRender>

Qt3DRender::QFrameGraphNode *QgsShadowRenderingFrameGraph::constructTexturesPreviewPass()
{
  Qt3DRender::QLayerFilter *filter = new Qt3DRender::QLayerFilter;
  filter->addLayer( mPreviewLayer );
  filter->setFilterMode( Qt3DRender::QLayerFilter::FilterMode::AcceptAnyMatchingLayers );

  Qt3DRender::QViewport *viewport = new Qt3DRender::QViewport( filter );
  viewport->setNormalizedRect( QRectF( 0.0f, 0.0f, 1.0f, 1.0f ) );

  Qt3DRender::QLayerFilter *layerFilter = new Qt3DRender::QLayerFilter( viewport );
  layerFilter->setObjectName( "preview" );
  layerFilter->addLayer( mPreviewLayer );
  layerFilter->setFilterMode( Qt3DRender::QLayerFilter::FilterMode::AcceptAnyMatchingLayers );

  Qt3DRender::QRenderStateSet *renderStateSet = new Qt3DRender::QRenderStateSet( layerFilter );
  Qt3DRender::QDepthTest *depthTest = new Qt3DRender::QDepthTest;
  depthTest->setDepthFunction( Qt3DRender::QDepthTest::Always );
  renderStateSet->addRenderState( depthTest );
  Qt3DRender::QCullFace *cullFace = new Qt3DRender::QCullFace;
  cullFace->setMode( Qt3DRender::QCullFace::NoCulling );
  renderStateSet->addRenderState( cullFace );

  return filter;
}

Qt3DRender::QFrameGraphNode *QgsShadowRenderingFrameGraph::constructForwardRenderPass()
{
  mSceneEntitiesFilter = new Qt3DRender::QLayerFilter( mMainCameraSelector );

  mSceneEntitiesFilter->addLayer( mPostprocessPassLayer );
  mSceneEntitiesFilter->addLayer( mPreviewLayer );
  mSceneEntitiesFilter->setFilterMode( Qt3DRender::QLayerFilter::FilterMode::DiscardAnyMatchingLayers );

  mForwardColorTexture = new Qt3DRender::QTexture2D;
  mForwardColorTexture->setWidth( 1024 );
  mForwardColorTexture->setHeight( 1024 );
  mForwardColorTexture->setFormat( Qt3DRender::QTexture2D::TextureFormat::RGBA16F );
  mForwardColorTexture->setGenerateMipMaps( false );
  mForwardColorTexture->setMagnificationFilter( Qt3DRender::QTexture2D::Linear );
  mForwardColorTexture->setMinificationFilter( Qt3DRender::QTexture2D::Linear );
  mForwardColorTexture->wrapMode()->setX( Qt3DRender::QTextureWrapMode::ClampToEdge );
  mForwardColorTexture->wrapMode()->setY( Qt3DRender::QTextureWrapMode::ClampToEdge );

  mForwardDepthTexture = new Qt3DRender::QTexture2D;
  mForwardDepthTexture->setWidth( 1024 );
  mForwardDepthTexture->setHeight( 1024 );
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
  mForwardRenderTargetDepthOutput->setTexture( mForwardDepthTexture );
  mForwardRenderTarget->addOutput( mForwardRenderTargetDepthOutput );
  mForwardRenderTargetColorOutput = new Qt3DRender::QRenderTargetOutput;
  mForwardRenderTargetColorOutput->setAttachmentPoint( Qt3DRender::QRenderTargetOutput::Color0 );
  mForwardRenderTargetColorOutput->setTexture( mForwardColorTexture );
  mForwardRenderTarget->addOutput( mForwardRenderTargetColorOutput );

  mForwardRenderTargetSelector = new Qt3DRender::QRenderTargetSelector( mSceneEntitiesFilter );
  mForwardRenderTargetSelector->setTarget( mForwardRenderTarget );

  mForwardClearBuffers = new Qt3DRender::QClearBuffers( mForwardRenderTargetSelector );
  mForwardClearBuffers->setClearColor( QColor::fromRgbF( 0.0, 1.0, 0.0, 1.0 ) );
  mForwardClearBuffers->setBuffers( Qt3DRender::QClearBuffers::ColorDepthBuffer );

  mFrustumCulling = new Qt3DRender::QFrustumCulling;
  mFrustumCulling->setParent( mForwardClearBuffers );

  return mSceneEntitiesFilter;
}

Qt3DRender::QFrameGraphNode *QgsShadowRenderingFrameGraph::constructShadowRenderPass()
{
  Qt3DRender::QLayerFilter *sceneEntitiesFilter = new Qt3DRender::QLayerFilter;
  sceneEntitiesFilter->addLayer( mPostprocessPassLayer );
  sceneEntitiesFilter->addLayer( mPreviewLayer );
  sceneEntitiesFilter->addLayer( mDoNotCastShadowsLayer );
  sceneEntitiesFilter->setFilterMode( Qt3DRender::QLayerFilter::FilterMode::DiscardAnyMatchingLayers );

  mShadowMapTexture = new Qt3DRender::QTexture2D;
  mShadowMapTexture->setWidth( 1024 * 2 );
  mShadowMapTexture->setHeight( 1024 * 2 );
  mShadowMapTexture->setFormat( Qt3DRender::QTexture2D::TextureFormat::D32F );
  mShadowMapTexture->setGenerateMipMaps( false );
  mShadowMapTexture->setMagnificationFilter( Qt3DRender::QTexture2D::Linear );
  mShadowMapTexture->setMinificationFilter( Qt3DRender::QTexture2D::Linear );
  mShadowMapTexture->wrapMode()->setX( Qt3DRender::QTextureWrapMode::ClampToEdge );
  mShadowMapTexture->wrapMode()->setY( Qt3DRender::QTextureWrapMode::ClampToEdge );
//  mShadowMapTexture->setComparisonFunction( Qt3DRender::QTexture2D::ComparisonFunction::CompareLessEqual );
//  mShadowMapTexture->setComparisonMode( Qt3DRender::QTexture2D::ComparisonMode::CompareRefToTexture );

  mShadowRenderTarget = new Qt3DRender::QRenderTarget;
  mShadowRenderTargetOutput = new Qt3DRender::QRenderTargetOutput;
  mShadowRenderTargetOutput->setAttachmentPoint( Qt3DRender::QRenderTargetOutput::Depth );
  mShadowRenderTargetOutput->setTexture( mShadowMapTexture );
  mShadowRenderTarget->addOutput( mShadowRenderTargetOutput );

  mShadowRenderTargetSelector = new Qt3DRender::QRenderTargetSelector( sceneEntitiesFilter );
  mShadowRenderTargetSelector->setTarget( mShadowRenderTarget );

  mShadowClearBuffers = new Qt3DRender::QClearBuffers( mShadowRenderTargetSelector );
  mShadowClearBuffers ->setBuffers( Qt3DRender::QClearBuffers::BufferType::ColorDepthBuffer );

  Qt3DRender::QRenderStateSet *renderStateSet = new Qt3DRender::QRenderStateSet( mShadowClearBuffers );
  Qt3DRender::QPolygonOffset *polygonOffset = new Qt3DRender::QPolygonOffset;
  polygonOffset->setDepthSteps( 2.0f );
  polygonOffset->setScaleFactor( 2.0f );
  renderStateSet->addRenderState( polygonOffset );
  Qt3DRender::QDepthTest *depthTest = new Qt3DRender::QDepthTest;
  depthTest->setDepthFunction( Qt3DRender::QDepthTest::LessOrEqual );
  renderStateSet->addRenderState( depthTest );
  Qt3DRender::QCullFace *cullFace = new Qt3DRender::QCullFace;
  cullFace->setMode( Qt3DRender::QCullFace::Front );
  renderStateSet->addRenderState( cullFace );

  return sceneEntitiesFilter;
}

Qt3DRender::QFrameGraphNode *QgsShadowRenderingFrameGraph::constructPostprocessingPass()
{
  mPostprocessPassLayerFilter = new Qt3DRender::QLayerFilter;
  mPostprocessPassLayerFilter->addLayer( mPostprocessPassLayer );
  mPostprocessPassLayerFilter->setFilterMode( Qt3DRender::QLayerFilter::FilterMode::AcceptAnyMatchingLayers );

  mPostprocessClearBuffers = new Qt3DRender::QClearBuffers( mPostprocessPassLayerFilter );
  mPostprocessClearBuffers->setClearColor( QColor::fromRgbF( 1.0f, 0.0f, 0.0f ) );

  return mPostprocessPassLayerFilter;
}

QgsShadowRenderingFrameGraph::QgsShadowRenderingFrameGraph( QWindow *window, Qt3DRender::QCamera *mainCamera, Qt3DCore::QEntity *root )
{
  mRootEntity = root;
  mMainCamera = mainCamera;
  mLightCamera = new Qt3DRender::QCamera;
  mDoNotCastShadowsLayer = new Qt3DRender::QLayer;
  mDoNotCastShadowsLayer->setRecursive( true );
  mPostprocessPassLayer = new Qt3DRender::QLayer;
  mPostprocessPassLayer->setRecursive( true );
  mPreviewLayer = new Qt3DRender::QLayer;
  mPreviewLayer->setRecursive( true );

  // FrameGraph tree:
  // mRenderSurfaceSelector
  //   mMainViewPort
  //     mMainCameraSelector
  //       forwardRenderPass
  //     mLightCameraSelector
  //       shadowRenderPass
  //       postProcessingPass

  mRenderSurfaceSelector = new Qt3DRender::QRenderSurfaceSelector;
  mRenderSurfaceSelector->setSurface( window );

  mMainViewPort = new Qt3DRender::QViewport( mRenderSurfaceSelector );
  mMainViewPort->setNormalizedRect( QRectF( 0.0f, 0.0f, 1.0f, 1.0f ) );

  mMainCameraSelector = new Qt3DRender::QCameraSelector( mMainViewPort );
  mMainCameraSelector->setCamera( mMainCamera );

  // Forward render
  Qt3DRender::QFrameGraphNode *forwardRenderPass = constructForwardRenderPass();
  forwardRenderPass->setParent( mMainCameraSelector );

  // shadow rendering pass
  mLightCameraSelector = new Qt3DRender::QCameraSelector( mMainViewPort );
  mLightCameraSelector->setCamera( mLightCamera );

  Qt3DRender::QFrameGraphNode *shadowRenderPass = constructShadowRenderPass();
  shadowRenderPass->setParent( mLightCameraSelector );

  // post process
  Qt3DRender::QFrameGraphNode *postprocessingPass = constructPostprocessingPass();
  postprocessingPass->setParent( mLightCameraSelector );

  QString vertexShaderPath = QStringLiteral( "qrc:/shaders/postprocess.vert" );
  QString fragmentShaderPath = QStringLiteral( "qrc:/shaders/postprocess.frag" );
  mPostprocessingEntity = new QgsPostprocessingEntity( this, vertexShaderPath, fragmentShaderPath );
  mPostprocessingEntity->setParent( mRootEntity );
  mPostprocessingEntity->addComponent( mPostprocessPassLayer );

  // textures preview pass
  Qt3DRender::QFrameGraphNode *previewPass = constructTexturesPreviewPass();
  previewPass->setParent( mMainViewPort );
}

void QgsShadowRenderingFrameGraph::addTexturePreviewOverlay( Qt3DRender::QTexture2D *texture, const QPointF &centerNDC, const QSizeF &size, QVector<Qt3DRender::QParameter *> additionalShaderParameters )
{
  QgsPreviewQuad *previewQuad = new QgsPreviewQuad( texture, centerNDC, size, additionalShaderParameters );
  previewQuad->addComponent( mPreviewLayer );
  previewQuad->setParent( mRootEntity );
}

QVector3D WorldPosFromDepth( QMatrix4x4 projMatrixInv, QMatrix4x4 viewMatrixInv, float texCoordX, float texCoordY, float depth )
{
  float z = depth * 2.0 - 1.0;

  QVector4D clipSpacePosition( texCoordX * 2.0 - 1.0, texCoordY * 2.0 - 1.0, z, 1.0 );
  QVector4D viewSpacePosition = projMatrixInv * clipSpacePosition;

  // Perspective division
  viewSpacePosition /= viewSpacePosition.w();
  QVector4D worldSpacePosition =  viewMatrixInv * viewSpacePosition;
  worldSpacePosition /= worldSpacePosition.w();

  return QVector3D( worldSpacePosition.x(), worldSpacePosition.y(), worldSpacePosition.z() );
}

// computes the portion of the Y=y plane the camera is looking at
void calculateViewExtent( Qt3DRender::QCamera *camera, float shadowRenderingDistance, float y, float &minX, float &maxX, float &minZ, float &maxZ )
{
  QVector3D cameraPos = camera->position();
  QMatrix4x4 projectionMatrix = camera->projectionMatrix();
  QMatrix4x4 viewMatrix = camera->viewMatrix();
  QMatrix4x4 projectionMatrixInv = projectionMatrix.inverted();
  QMatrix4x4 viewMatrixInv = viewMatrix.inverted();
  float depth = 1.0f;
  QVector4D viewCenter =  viewMatrix * QVector4D( camera->viewCenter(), 1.0f );
  viewCenter /= viewCenter.w();
  viewCenter = projectionMatrix * viewCenter;
  viewCenter /= viewCenter.w();
  depth = viewCenter.z();
  QVector<QVector3D> viewFrustumPoints =
  {
    QVector3D( 0.0f,  0.0f, depth ),
    QVector3D( 0.0f,  1.0f, depth ),
    QVector3D( 1.0f,  0.0f, depth ),
    QVector3D( 1.0f,  1.0f, depth )
  };
  maxX = std::numeric_limits<float>::lowest();
  maxZ = std::numeric_limits<float>::lowest();
  minX = std::numeric_limits<float>::max();
  minZ = std::numeric_limits<float>::max();
//  float shadowRenderingDistance = 1000.0f;
  for ( int i = 0; i < viewFrustumPoints.size(); ++i )
  {
    // convert from view port space to world space
    viewFrustumPoints[i] = WorldPosFromDepth(
                             projectionMatrixInv, viewMatrixInv,
                             viewFrustumPoints[i].x(), viewFrustumPoints[i].y(), viewFrustumPoints[i].z() );
    // find the intersection between the line going from cameraPos to the frustum quad point
    // and the horizontal plane Y=y
    // if the intersection is on the back side of the viewing panel we get a point that is
    // shadowRenderingDistance units in front of the camera
    QVector3D pt = cameraPos;
    QVector3D vect = ( viewFrustumPoints[i] - pt ).normalized();
    float t = ( y - pt.y() ) / vect.y();
    if ( t < 0 )
      t = shadowRenderingDistance;
    else t = qMin( t, shadowRenderingDistance );
    viewFrustumPoints[i] = pt + t * vect;
    minX = qMin( minX, viewFrustumPoints[i].x() );
    maxX = qMax( maxX, viewFrustumPoints[i].x() );
    minZ = qMin( minZ, viewFrustumPoints[i].z() );
    maxZ = qMax( maxZ, viewFrustumPoints[i].z() );
  }
}

void QgsShadowRenderingFrameGraph::setupDirectionalLight( const QgsDirectionalLightSettings &light, float maximumShadowRenderingDistance )
{
  float minX, maxX, minZ, maxZ;
  QVector3D lookingAt = mMainCamera->viewCenter();//(0.5 * (minX + maxX), mMainCamera->viewCenter().y(), 0.5 * (minZ + maxZ));
  float d = 1.5f * ( mMainCamera->position() - mMainCamera->viewCenter() ).length();

  QVector3D vertical = QVector3D( 0.0f, d, 0.0f );
  QVector3D lightPosition = lookingAt + vertical;
  QVector3D lightDirection = QVector3D( light.direction().x(), light.direction().y(), light.direction().z() ).normalized();
  calculateViewExtent( mMainCamera, maximumShadowRenderingDistance, lookingAt.y(), minX, maxX, minZ, maxZ );

  mLightCamera->setPosition( lightPosition );
  mLightCamera->setViewCenter( lookingAt );
  mLightCamera->setUpVector( QVector3D( 0.0f, 1.0f, 0.0f ) );
  mLightCamera->rotateAboutViewCenter( QQuaternion::rotationTo( vertical.normalized(), -lightDirection.normalized() ) );

  mLightCamera->setProjectionType( Qt3DRender::QCameraLens::ProjectionType::OrthographicProjection );
  mLightCamera->lens()->setOrthographicProjection(
    - 0.75 * ( maxX - minX ), 0.75 * ( maxX - minX ),
    - 0.75 * ( maxZ - minZ ), 0.75 * ( maxZ - minZ ),
    1.0f, 2 * ( lookingAt - lightPosition ).length() );

  mPostprocessingEntity->setupShadowRenderingExtent( minX, maxX, minZ, maxZ );
  mPostprocessingEntity->setupDirectionalLight( lightPosition, lightDirection );
}

void QgsShadowRenderingFrameGraph::setClearColor( const QColor &clearColor )
{
  mForwardClearBuffers->setClearColor( clearColor );
}

void QgsShadowRenderingFrameGraph::setShadowRenderingEnabled( bool enabled )
{
  mShadowRenderingEnabled = enabled;
  mPostprocessingEntity->setShadowRenderingEnabled( mShadowRenderingEnabled );
}

void QgsShadowRenderingFrameGraph::setFrustumCullingEnabled( bool enabled )
{
  if ( enabled == mFrustumCullingEnabled )
    return;
  mFrustumCullingEnabled = enabled;
  if ( mFrustumCullingEnabled )
    mFrustumCulling->setParent( mForwardClearBuffers );
  else
    mFrustumCulling->setParent( ( Qt3DCore::QNode * )nullptr );
}
