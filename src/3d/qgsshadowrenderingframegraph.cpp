#include "qgsshadowrenderingframegraph.h"

#include "qgsdirectionallightsettings.h"
#include "qgscameracontroller.h"
#include "qgsrectangle.h"
#include "qgspostprocessingentity.h"

#include <Qt3DRender/QRenderStateSet>
#include <Qt3DRender/QDepthTest>
#include <Qt3DRender>

Qt3DRender::QFrameGraphNode *QgsShadowRenderingFrameGraph::constructPreviewSubTree()
{
  // Frame graph subtree:
  // layerFilter
  //   viewport
  Qt3DRender::QLayerFilter *filter = new Qt3DRender::QLayerFilter;
  filter->addLayer( mPreviewLayer );
  filter->setFilterMode( Qt3DRender::QLayerFilter::FilterMode::AcceptAnyMatchingLayers );

  Qt3DRender::QViewport *viewport = new Qt3DRender::QViewport( filter );
  viewport->setNormalizedRect( QRectF( 0.7f, 0.0f, 0.3f, 0.3f ) );
  viewport->setGamma( 2.5f );

  Qt3DRender::QLayerFilter *layerFilter = new Qt3DRender::QLayerFilter( viewport );
  layerFilter->setObjectName( "preview" );
  layerFilter->addLayer( mPreviewLayer );
  layerFilter->setFilterMode( Qt3DRender::QLayerFilter::FilterMode::AcceptAnyMatchingLayers );


//  Qt3DRender::QDebugOverlay *debugOverlay = new Qt3DRender::QDebugOverlay(viewport);
  Qt3DRender::QRenderStateSet *renderStateSet = new Qt3DRender::QRenderStateSet( layerFilter );
  Qt3DRender::QDepthTest *depthTest = new Qt3DRender::QDepthTest;
  depthTest->setDepthFunction( Qt3DRender::QDepthTest::Always );
  renderStateSet->addRenderState( depthTest );
  Qt3DRender::QCullFace *cullFace = new Qt3DRender::QCullFace;
  cullFace->setMode( Qt3DRender::QCullFace::NoCulling );
  renderStateSet->addRenderState( cullFace );

//  Qt3DRender::QCameraSelector* cameraSelector = new Qt3DRender::QCameraSelector(renderStateSet);
//  cameraSelector->setCamera(nullptr);

//  Qt3DRender::QClearBuffers* clearBufferts = new Qt3DRender::QClearBuffers(renderStateSet);
//  clearBufferts->setClearColor(QColor(0.0f, 0.0f, 1.0f));
//  clearBufferts->setBuffers(Qt3DRender::QClearBuffers::ColorBuffer);

  return filter;
}

Qt3DRender::QFrameGraphNode *QgsShadowRenderingFrameGraph::constructShadowRenderPass()
{
  mDoNotCastShadowsLayer = new Qt3DRender::QLayer;
  mDoNotCastShadowsLayer->setRecursive( true );

  Qt3DRender::QLayerFilter *sceneEntitiesFilter = new Qt3DRender::QLayerFilter;
  sceneEntitiesFilter->addLayer( mPostprocessPassLayer );
  sceneEntitiesFilter->addLayer( mPreviewLayer );
  sceneEntitiesFilter->addLayer( mDoNotCastShadowsLayer );
  sceneEntitiesFilter->setFilterMode( Qt3DRender::QLayerFilter::FilterMode::DiscardAnyMatchingLayers );

  mShadowMapTexture = new Qt3DRender::QTexture2D;
  mShadowMapTexture->setWidth( 1024 * 2 );
  mShadowMapTexture->setHeight( 1024 * 2 );
  mShadowMapTexture->setFormat( Qt3DRender::QTexture2D::TextureFormat::DepthFormat );
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
//  Qt3DRender::QPolygonOffset *polygonOffset = new Qt3DRender::QPolygonOffset;
//  polygonOffset->setDepthSteps(8);
//  polygonOffset->setScaleFactor(4);
//  renderStateSet->addRenderState(polygonOffset);
  Qt3DRender::QDepthTest *depthTest = new Qt3DRender::QDepthTest;
  depthTest->setDepthFunction( Qt3DRender::QDepthTest::LessOrEqual );
  renderStateSet->addRenderState( depthTest );
  Qt3DRender::QCullFace *cullFace = new Qt3DRender::QCullFace;
  cullFace->setMode( Qt3DRender::QCullFace::NoCulling );
  renderStateSet->addRenderState( cullFace );

  return sceneEntitiesFilter;
}

QgsShadowRenderingFrameGraph::QgsShadowRenderingFrameGraph( QWindow *window, Qt3DRender::QCamera *mainCamera )
{
  mMainCamera = mainCamera;
  mLightCamera = new Qt3DRender::QCamera;
  mLightCamera->tiltAboutViewCenter( -90.0f );

//  QObject::connect( mMainCamera, &Qt3DRender::QCamera::viewCenterChanged, [&]() {
//    updateDirectionalLightParameters();
//  } );

  // Frame graph:
  // mRenderSurfaceSelector
  //   mMainViewPort
  //     mMainCameraSelector
  //       mSceneEntitiesFilter
  //         mForwardRenderTargetSelector
  //           mForwardClearBuffers
  //       mPostprocessPassLayerFilter
  //         mPostprocessClearBuffers
  //       mPreviewPassLayerFilter

  mRenderSurfaceSelector = new Qt3DRender::QRenderSurfaceSelector;
  mRenderSurfaceSelector->setSurface( window );

  mMainViewPort = new Qt3DRender::QViewport( mRenderSurfaceSelector );
  mMainViewPort->setNormalizedRect( QRectF( 0.0f, 0.0f, 1.0f, 1.0f ) );

  mMainCameraSelector = new Qt3DRender::QCameraSelector( mMainViewPort );
  mMainCameraSelector->setCamera( mMainCamera );
//  mMainCameraSelector->setCamera( mLightCamera );

  // Forward render
  mSceneEntitiesFilter = new Qt3DRender::QLayerFilter( mMainCameraSelector );
  mPostprocessPassLayer = new Qt3DRender::QLayer;//(mSceneEntitiesFilter);
  mPostprocessPassLayer->setRecursive( true );
  mPreviewLayer = new Qt3DRender::QLayer;
  mPreviewLayer->setRecursive( true );

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

  // shadow rendering pass
  mLightCameraSelector = new Qt3DRender::QCameraSelector( mMainViewPort );
  mLightCameraSelector->setCamera( mLightCamera );

  Qt3DRender::QFrameGraphNode *shadowRenderPass = constructShadowRenderPass();
  shadowRenderPass->setParent( mLightCameraSelector );

  // post process
  mPostprocessPassLayerFilter = new Qt3DRender::QLayerFilter( mLightCameraSelector );
  mPostprocessPassLayerFilter->addLayer( mPostprocessPassLayer );
  mPostprocessPassLayerFilter->setFilterMode( Qt3DRender::QLayerFilter::FilterMode::AcceptAnyMatchingLayers );

  mPostprocessClearBuffers = new Qt3DRender::QClearBuffers( mPostprocessPassLayerFilter );
  mPostprocessClearBuffers->setClearColor( QColor::fromRgbF( 1.0f, 0.0f, 0.0f ) );

  //
  Qt3DRender::QFrameGraphNode *preview = constructPreviewSubTree();
  preview->setParent( mMainViewPort );

  QString vertexShaderPath = QStringLiteral( "qrc:/shaders/postprocess.vert" );
  QString fragmentShaderPath = QStringLiteral( "qrc:/shaders/postprocess.frag" );
  mPostprocessingEntity = new QgsPostprocessingEntity( this, vertexShaderPath, fragmentShaderPath );
}

void QgsShadowRenderingFrameGraph::updateDirectionalLightParameters( QgsRectangle extent )
{
  qDebug() << "Camera position: " << mMainCamera->position();
  qDebug() << "View center: " << mMainCamera->viewCenter();
  qDebug() << "Aspect ratio: " << mMainCamera->aspectRatio();
  qDebug() << "FoV: " << mMainCamera->fieldOfView();
  qDebug() << "Near: " << mMainCamera->nearPlane();
  qDebug() << "Far: " << mMainCamera->farPlane();
  qDebug() << "--------------------------";

//Camera position:  QVector3D(-89687.8, 128337, -2458.87)
//View center:  QVector3D(-89687.8, 14.2109, -2458.87)
//Aspect ratio:  2.47539
//FoV:  45
//Near:  54873.4
//Far:  256673

  QgsPointXY extentCenter = extent.center();
//  qDebug() << extent;
//  qDebug() << extent.width() << " " << extent.height();

  float mainCameraFarPlane = mMainCamera->farPlane();
  QgsVector3D lightDirection = QgsVector3D( 0.0f, -1.0f, 0.0f ); //mLightDirection;
  QgsVector3D mainCameraViewCenter = mMainCamera->viewCenter();//(extentCenter.x(), 0.0f, extentCenter.y());//mMainCamera->viewCenter();
  float d = 200.0f;//shadowDistance;//0.5 * qMax(extent.width(), extent.height());// mMainCamera->farPlane();

  QVector3D lightPosition( mainCameraViewCenter.x(), mainCameraViewCenter.y() + d, mainCameraViewCenter.z() );
  QVector3D lookingAt( mainCameraViewCenter.x(), mainCameraViewCenter.y(), mainCameraViewCenter.z() );
//  lightDirection.normalize();
//  float x_diff = lightDirection.x() * mainCameraFarPlane;
//  float z_diff = lightDirection.z() * mainCameraFarPlane;
//  lightPosition += QVector3D(x_diff, 0.0f, z_diff);
//   mLightCamera->lens()->setPerspectiveProjection()


  mLightCamera->setProjectionType( Qt3DRender::QCameraLens::PerspectiveProjection );
  mLightCamera->lens()->setPerspectiveProjection( 90.0f, mMainCamera->aspectRatio(), 1.0f, 2 * d );

  mLightCamera->setPosition( lightPosition );
  mLightCamera->setViewCenter( lookingAt );
//  mLightCamera->lens()->setOrthographicProjection(-d, d, -d, d, 1, 128337 * 1.5);
//  mLightCamera->rotateAboutViewCenter(
//  mLightCamera->tilt(0.0f);
//  mLightCamera->setPosition(QVector3D(extentCenter.x(), mMainCamera->farPlane(), extentCenter.y()));
//  mLightCamera->setViewCenter(QVector3D(0.0f, 1000.0f, 0.0f));
//  mLightCamera->setPosition(QVector3D(extentCenter.x(), qMax(extent.width(), extent.height()) * 2.0f, extentCenter.y()));
//  mLightCamera->viewSphere(mMainCamera->viewCenter(), qMax(extent.width(), extent.height()));
//  mLightCamera->tiltAboutViewCenter(90.0f);
//  mLightCamera->rotateAboutViewCenter(QQuaternion::fromAxisAndAngle(QVector3D(0.0f, 0.0f, -1.0f), 90.0f));

//  mLightCamera->setPosition(lightPosition);
//  mLightCamera->setViewCenter(QVector3D(mainCameraViewCenter.x(), mainCameraViewCenter.y(), mainCameraViewCenter.z()));
//  mLightCamera->lens()->setOrthographicProjection(
//    extent.xMinimum(), extent.xMaximum(),
//    extent.yMinimum(), extent.yMaximum(),
//    10000, mainCameraFarPlane);
}

void QgsShadowRenderingFrameGraph::setupDirectionalLights( const QgsDirectionalLightSettings &light )
{
  QgsVector3D lightDirection = light.direction();
  lightDirection.normalize();
  mLightDirection = QVector3D( lightDirection.x(), lightDirection.y(), lightDirection.z() );
//  updateDirectionalLightParameters();
}

static double viewDistance( Qt3DRender::QCamera *camera )
{
//  double farPlane = camera->farPlane();
  double farPlane = ( camera->position() - camera->viewCenter() ).length();
  double fov = camera->fieldOfView();
  double aspectRatio = camera->aspectRatio();

  double viewDistance = 2.0f * farPlane * tanf( qDegreesToRadians( fov / 2.0f ) );
  return qMax( viewDistance, viewDistance * aspectRatio );
}

QVector3D WorldPosFromDepth( QMatrix4x4 projMatrixInv, QMatrix4x4 viewMatrixInv, float texCoordX, float texCoordY, float depth )
{
  float z = depth * 2.0 - 1.0;

  QVector4D clipSpacePosition( texCoordX * 2.0 - 1.0, texCoordY * 2.0 - 1.0, z, 1.0 );
  QVector4D viewSpacePosition = projMatrixInv * clipSpacePosition;

  // Perspective division
  viewSpacePosition /= viewSpacePosition.w();
//    return viewSpacePosition.xyz;
  QVector4D worldSpacePosition =  viewMatrixInv * viewSpacePosition;
  worldSpacePosition /= worldSpacePosition.w();

  return QVector3D( worldSpacePosition.x(), worldSpacePosition.y(), worldSpacePosition.z() );
}

struct ViewBoundingBox
{
  float minX;
  float maxX;
  float minY;
  float maxY;
  float minZ;
  float maxZ;
};

void calculateExtent( Qt3DRender::QCamera *camera, float &minX, float &maxX, float &minZ, float &maxZ )
{
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
  qDebug() << "Depth " << depth;
  QVector<QVector3D> viewFrustrumPoints =
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
  for ( int i = 0; i < viewFrustrumPoints.size(); ++i )
  {
    viewFrustrumPoints[i] = WorldPosFromDepth(
                              projectionMatrixInv, viewMatrixInv,
                              viewFrustrumPoints[i].x(), viewFrustrumPoints[i].y(), viewFrustrumPoints[i].z() );
    maxX = qMax( maxX, viewFrustrumPoints[i].x() );
    maxZ = qMax( maxZ, viewFrustrumPoints[i].z() );
    minX = qMin( minX, viewFrustrumPoints[i].x() );
    minZ = qMin( minZ, viewFrustrumPoints[i].z() );
  }
}

ViewBoundingBox calculateExtent( Qt3DRender::QCamera *camera )
{
  ViewBoundingBox boundingBox;
  float &minX = boundingBox.minX;
  float &maxX = boundingBox.maxX;
  float &minY = boundingBox.minY;
  float &maxY = boundingBox.maxY;
  float &minZ = boundingBox.minZ;
  float &maxZ = boundingBox.maxZ;
  minX = std::numeric_limits<float>::max();
  minY = std::numeric_limits<float>::max();
  minZ = std::numeric_limits<float>::max();
  maxX = std::numeric_limits<float>::lowest();
  maxY = std::numeric_limits<float>::lowest();
  maxZ = std::numeric_limits<float>::lowest();
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
  qDebug() << "Depth " << depth;
  QVector<QVector3D> viewFrustrumPoints =
  {
    QVector3D( 0.0f,  0.0f, depth ),
    QVector3D( 0.0f,  1.0f, depth ),
    QVector3D( 1.0f,  0.0f, depth ),
    QVector3D( 1.0f,  1.0f, depth ),
    QVector3D( 0.0f,  0.0f, 0.01f ),
    QVector3D( 0.0f,  1.0f, 0.01f ),
    QVector3D( 1.0f,  0.0f, 0.01f ),
    QVector3D( 1.0f,  1.0f, 0.01f ),
  };
  minX = std::numeric_limits<float>::max();
  minY = std::numeric_limits<float>::max();
  minZ = std::numeric_limits<float>::max();
  maxX = std::numeric_limits<float>::lowest();
  maxY = std::numeric_limits<float>::lowest();
  maxZ = std::numeric_limits<float>::lowest();
  for ( int i = 0; i < viewFrustrumPoints.size(); ++i )
  {
    viewFrustrumPoints[i] = WorldPosFromDepth(
                              projectionMatrixInv, viewMatrixInv,
                              viewFrustrumPoints[i].x(), viewFrustrumPoints[i].y(), viewFrustrumPoints[i].z() );
    maxX = qMax( maxX, viewFrustrumPoints[i].x() );
    minX = qMin( minX, viewFrustrumPoints[i].x() );
    maxY = qMax( maxY, viewFrustrumPoints[i].y() );
    minY = qMin( minY, viewFrustrumPoints[i].y() );
    maxZ = qMax( maxZ, viewFrustrumPoints[i].z() );
    minZ = qMin( minZ, viewFrustrumPoints[i].z() );
  }
  return boundingBox;
}

void setupViewFrustrum( Qt3DRender::QCamera *camera, QVector3D cameraPos, QVector3D viewCenter, double renderDistance )
{
  camera->setPosition( cameraPos );
  camera->setUpVector( QVector3D( 0.0f, 1.0f, 0.0f ) );
  QVector<QVector3D> frustrumPoints =
  {
    QVector3D( viewCenter.x() - renderDistance, viewCenter.y(), viewCenter.z() - renderDistance ),
    QVector3D( viewCenter.x() - renderDistance, viewCenter.y(), viewCenter.z() + renderDistance ),
    QVector3D( viewCenter.x() + renderDistance, viewCenter.y(), viewCenter.z() - renderDistance ),
    QVector3D( viewCenter.x() + renderDistance, viewCenter.y(), viewCenter.z() + renderDistance )
  };

  QVector3D P = cameraPos;
  QVector3D C = viewCenter;
  QVector3D PC = C - P;
  QVector<QVector3D> castedPoints;
  int farthestPoint = 0;
  float farthestPointD = 0.0;
  for ( QVector3D Q : frustrumPoints )
  {
    QVector3D PQ = Q - P;
    float t = QVector3D::dotProduct( PQ, PC );
    castedPoints.push_back( P + t * PC );
    float d = ( castedPoints.back() - P ).length();
    if ( d > farthestPointD )
    {
      farthestPointD = d;
      farthestPoint = castedPoints.size() - 1;
    }
  }

  QVector3D refPoint = castedPoints[farthestPoint];
  camera->setViewCenter( refPoint );

  QVector4D upVector( 0.0f, 1.0f, 0.0f, 0.0f );
  upVector = camera->viewMatrix().inverted() * upVector;
//  camera->
//  camera->setUpVector(QVector3D(upVector.x(), upVector.y(), upVector.z()));

  qDebug() << "Frustrum points: ";
  float minX = std::numeric_limits<float>::max();
  float maxX = std::numeric_limits<float>::lowest();
  float minY = std::numeric_limits<float>::max();
  float maxY = std::numeric_limits<float>::lowest();
  float minZ = std::numeric_limits<float>::max();
  float maxZ = std::numeric_limits<float>::lowest();
  for ( int i = 0; i < frustrumPoints.size(); ++i )
  {
    QVector4D p = camera->viewMatrix() * QVector4D( frustrumPoints[i], 1.0f );
    p /= p.w();
    frustrumPoints[i] = QVector3D( p.x(), p.y(), p.z() );
    qDebug() << frustrumPoints[i];
    minX = qMin( minX, frustrumPoints[i].x() );
    maxX = qMax( maxX, frustrumPoints[i].x() );
    minY = qMin( minY, frustrumPoints[i].y() );
    maxY = qMax( maxY, frustrumPoints[i].y() );
    minZ = qMin( minZ, frustrumPoints[i].z() );
    maxZ = qMax( maxZ, frustrumPoints[i].z() );
  }

  camera->lens()->setOrthographicProjection( minX, maxX, minY, maxY, 1.0f, 1000.0f );
  return;

  qDebug() << "Center point: " << camera->viewMatrix() * QVector4D( viewCenter, 1.0f );
//  qDebug() << "--------------" ;
//  float refPointToCameraPos = (refPoint - P).length();
//  for (int i = 0; i < frustrumPoints.size(); ++i) {
//    float f = (castedPoints[i] - P).length() / refPointToCameraPos;
////    frustrumPoints[i] = P + f * (frustrumPoints[i] - P);
//    frustrumPoints[i].setX(f * frustrumPoints[i].x());
//    frustrumPoints[i].setY(f * frustrumPoints[i].y());
//    qDebug() << frustrumPoints[i];
//  }
  qDebug() << "Z span: " << minZ << " " << maxZ;

  for ( int i = 0; i < frustrumPoints.size(); ++i )
  {
    frustrumPoints[i].setZ( maxZ );
  }

  double farPlane = maxZ;
  double fovy = qRadiansToDegrees( 2.0f * atan2( maxX, farPlane ) );
  qDebug() << "FoV: " << fovy;
  double aspectRatio = maxX / maxY;
  camera->setProjectionType( Qt3DRender::QCameraLens::PerspectiveProjection );
  camera->lens()->setPerspectiveProjection( fovy, aspectRatio, 1.0f, 1.0f * farPlane );
  return;

  QVector<QVector3D> newFrustrumPoints =
  {
    QVector3D( viewCenter.x() - renderDistance, viewCenter.y(), viewCenter.z() - renderDistance ),
    QVector3D( viewCenter.x() - renderDistance, viewCenter.y(), viewCenter.z() + renderDistance ),
    QVector3D( viewCenter.x() + renderDistance, viewCenter.y(), viewCenter.z() - renderDistance ),
    QVector3D( viewCenter.x() + renderDistance, viewCenter.y(), viewCenter.z() + renderDistance )
  };

  float shadowDist = 0.0f;
  for ( int i = 0; i < frustrumPoints.size(); ++i )
  {
    shadowDist = qMax( shadowDist, ( frustrumPoints[i] - refPoint ).length() );
  }

//  shadowDist *= 2.0f;
//  double farPlane = 1.2 * (refPoint - cameraPos).length();
//  double fovy = qRadiansToDegrees( 2.0f * atan2f(shadowDist / 2.0f, farPlane) );
//  qDebug() << "FoV " << fovy;
//  fovy = qMin(60.0, fovy);
//  camera->setProjectionType(Qt3DRender::QCameraLens::PerspectiveProjection);
//  camera->lens()->setPerspectiveProjection(fovy, 1.0f, 1.0f, farPlane);

//  QVector4D viewCenterViewSpace4D = camera->viewMatrix() * QVector4D(refPoint.x(), refPoint.y(), refPoint.z(), 1.0f);
//  viewCenterViewSpace4D /= viewCenterViewSpace4D.w();
//  QVector4D positionViewSpace4D = camera->viewMatrix() * QVector4D(cameraPos.x(), cameraPos.y(), cameraPos.z(), 1.0f);
//  positionViewSpace4D /= positionViewSpace4D.w();
//  QVector3D viewCenterViewSpace(viewCenterViewSpace4D.x(), viewCenterViewSpace4D.y(), viewCenterViewSpace4D.z());
//  QVector3D positionViewSpace(positionViewSpace4D.x(), positionViewSpace4D.y(), positionViewSpace4D.z());

//  float farPlane = (viewCenterViewSpace - positionViewSpace).length();
//  double fov = qRadiansToDegrees( 2.0f * atan2f(shadowDist / 2.0f, farPlane) );
//  fov = qMin(90.0, fov);

////  mLightCamera->viewSphere(lookingAt, shadow_render_distance);

//  camera->setProjectionType(Qt3DRender::QCameraLens::PerspectiveProjection);
//  camera->lens()->setPerspectiveProjection(fov, 1.0f, 1.0f, farPlane);
}

void setupViewFrustrum2( Qt3DRender::QCamera *camera, QVector3D cameraPos, QVector3D viewCenter, double renderDistance )
{
  camera->setPosition( cameraPos );
  camera->setViewCenter( viewCenter );
//  camera->setUpVector(QVector3D(0.0f, 1.0f, 0.0f));
  QVector<QVector4D> frustrumPoints =
  {
    QVector4D( viewCenter.x() - renderDistance, viewCenter.y(), viewCenter.z() - renderDistance, 1.0f ),
    QVector4D( viewCenter.x() - renderDistance, viewCenter.y(), viewCenter.z() + renderDistance, 1.0f ),
    QVector4D( viewCenter.x() + renderDistance, viewCenter.y(), viewCenter.z() - renderDistance, 1.0f ),
    QVector4D( viewCenter.x() + renderDistance, viewCenter.y(), viewCenter.z() + renderDistance, 1.0f )
  };
  float maxDistToCameraPos = 0.0f;
  float viewCenterToCameraPos = ( cameraPos - viewCenter ).length();
  for ( QVector4D fp : frustrumPoints )
  {
    float d = ( QVector3D( fp.x(), fp.y(), fp.z() ) - cameraPos ).length();
    maxDistToCameraPos = qMax( maxDistToCameraPos, d );
  }
//  for (int i = 0; i < frustrumPoints.size(); ++i) {
//    frustrumPoints[i] = cameraPos + (maxDistToCameraPos / maxDistToCameraPos)
//  }


  float maxZ = std::numeric_limits<float>::lowest();
  for ( int i = 0; i < frustrumPoints.size(); ++i )
  {
    frustrumPoints[i] = camera->viewMatrix() * frustrumPoints[i];
    frustrumPoints[i] /= frustrumPoints[i].w();
    maxZ = qMax( maxZ, frustrumPoints[i].z() );
  }
  for ( int i = 0; i < frustrumPoints.size(); ++i )
  {
    frustrumPoints[i].setZ( maxZ );
  }
  QVector3D center( 0.0f, 0.0f, 0.0f );
  for ( int i = 0; i < frustrumPoints.size(); ++i )
  {
    frustrumPoints[i] = camera->viewMatrix().inverted() * frustrumPoints[i];
    frustrumPoints[i] /= frustrumPoints[i].w();
    center += QVector3D( frustrumPoints[i].x(), frustrumPoints[i].y(), frustrumPoints[i].z() );
  }
  center /= 4.0f;
//  camera->setViewCenter(center);
  float d = 0.0f;
  for ( QVector4D fp : frustrumPoints )
  {
    d = qMax( d, ( QVector3D( fp.x(), fp.y(), fp.z() ) - center ).length() );
  }
  d *= 2.0f;
  float farPlane = 2.0f * ( center - cameraPos ).length();
  float distanceToEdge = ( QVector3D( frustrumPoints[0].x(), frustrumPoints[0].y(), frustrumPoints[0].z() ) - cameraPos ).length();
//  qDebug() << "Fov : " << fov;
//  float farPlane = 0.5f * d / tan(qDegreesToRadians(0.5 * fov));
  // fov = 2 * atan(d / 2)
  double fov = qRadiansToDegrees( 2.0f * atan2f( d / 2.0f, farPlane ) );
  camera->setProjectionType( Qt3DRender::QCameraLens::PerspectiveProjection );
  camera->lens()->setPerspectiveProjection( fov, 1.0f, 1.0f, farPlane );

//  for (QVector4D p : frustrumPoints) qDebug() << p;
}

void QgsShadowRenderingFrameGraph::setupPointLight( const QgsPointLightSettings &light )
{
  QVector3D mainCameraViewCenter = mMainCamera->viewCenter();
  QVector3D lightPosition( light.position().x(), light.position().y(), light.position().z() );

//  calculateExtent(mMainCamera, minX, maxX, minZ, maxZ);

  QVector3D lookingAt = mainCameraViewCenter;//((minX + maxX) / 2.0f, mainCameraViewCenter.y(), (minZ + maxZ) / 2.0f);
//  double farPlane = 1.2 * (lookingAt - lightPosition).length();

//  double shadow_render_distance = 1000.0f;//viewDistance(mMainCamera);
//  double fov = qRadiansToDegrees( 2.0f * atan2f(shadow_render_distance / 2.0f, farPlane) );
//  fov = qMax(fov, 90.0);
//  mLightCamera->setUpVector(QVector3D(0.0f, 1.0f, 0.0f));


//  mLightCamera->setPosition(lightPosition);
//  mLightCamera->setViewCenter(lookingAt);
//  mLightCamera->setUpVector(QVector3D(0.0f, 1.0f, 0.0f));


//  QVector4D viewCenterViewSpace4D = mLightCamera->viewMatrix() * QVector4D(lightPosition.x(), lightPosition.y(), lightPosition.z(), 1.0f);
//  viewCenterViewSpace4D /= viewCenterViewSpace4D.w();
//  QVector4D positionViewSpace4D = mLightCamera->viewMatrix() * QVector4D(lookingAt.x(), lookingAt.y(), lookingAt.z(), 1.0f);
//  positionViewSpace4D /= positionViewSpace4D.w();
//  QVector3D viewCenterViewSpace(viewCenterViewSpace4D.x(), viewCenterViewSpace4D.y(), viewCenterViewSpace4D.z());
//  QVector3D positionViewSpace(positionViewSpace4D.x(), positionViewSpace4D.y(), positionViewSpace4D.z());

//  double shadow_render_distance = 1000.0f;
//  float farPlane = shadow_render_distance;//2.0f * (viewCenterViewSpace - positionViewSpace).length();
//  double fov = qRadiansToDegrees( 2.0f * atan2f(shadow_render_distance / 2.0f, farPlane) );
//  fov = qMin(90.0, fov);

////  mLightCamera->viewSphere(lookingAt, shadow_render_distance);

//  mLightCamera->setProjectionType(Qt3DRender::QCameraLens::PerspectiveProjection);
//  mLightCamera->lens()->setPerspectiveProjection(fov, 1.0f, 1.0f, farPlane);

////  mLightCamera->setProjectionType(Qt3DRender::QCameraLens::OrthographicProjection);
////  float d = 100.0f;
////  mLightCamera->lens()->setOrthographicProjection(-d, d, -d, d, 1.0f, shadowDistance);
  float shadowRenderingDistance = 150.0f;
  setupViewFrustrum( mLightCamera, lightPosition, lookingAt, shadowRenderingDistance );

  float minX = mainCameraViewCenter.x() - shadowRenderingDistance;
  float maxX = mainCameraViewCenter.x() + shadowRenderingDistance;
  float minZ = mainCameraViewCenter.z() - shadowRenderingDistance;
  float maxZ = mainCameraViewCenter.z() + shadowRenderingDistance;

//  calculateExtent(mLightCamera, minX, maxX, minZ, maxZ);

  mPostprocessingEntity->setupShadowRenderingExtent( minX, maxX, minZ, maxZ );
//  mPostprocessingEntity->setupShadowRenderingExtent(-100000.0f, 100000.0f, -100000.0f, 100000.0f);
}

ViewBoundingBox transformToViewSpace( ViewBoundingBox bb, Qt3DRender::QCamera *camera )
{
  ViewBoundingBox bb2;
  bb2.minX = std::numeric_limits<float>::max();
  bb2.minY = std::numeric_limits<float>::max();
  bb2.minZ = std::numeric_limits<float>::max();
  bb2.maxX = std::numeric_limits<float>::lowest();
  bb2.maxY = std::numeric_limits<float>::lowest();
  bb2.maxZ = std::numeric_limits<float>::lowest();
  QVector<QVector3D> points;
  for ( int i = 0; i < 8; ++i )
  {
    int a = i & 1, b = i & 2, c = i & 4;
    points.push_back( QVector3D( a > 0 ? bb.minX : bb.maxX, 0.0f/*b > 0 ? bb.minY : bb.maxY*/, c > 0 ? bb.minZ : bb.maxZ ) );
  }
  QMatrix4x4 viewMatrix = camera->viewMatrix();
  for ( int i = 0; i < points.size(); ++i )
  {
    QVector4D p = viewMatrix * QVector4D( points[i], 1.0f );
    p /= p.w();
    bb2.minX = qMin( bb2.minX, p.x() );
    bb2.minY = qMin( bb2.minY, p.y() );
    bb2.minZ = qMin( bb2.minZ, p.z() );
    bb2.maxX = qMax( bb2.maxX, p.x() );
    bb2.maxY = qMax( bb2.maxY, p.y() );
    bb2.maxZ = qMax( bb2.maxZ, p.z() );
  }
  return bb2;
}

QVector3D intersectionWithY0Plane( QVector3D pt, QVector3D vect )
{
  float t = - pt.y() / vect.y();
  return QVector3D( pt.x() + t * vect.x(), 0.0f, pt.z() + t * vect.z() );
}

// computes the portion of the y = 0 plane the camera is looking at
void calculateViewExtent( Qt3DRender::QCamera *camera, float y, QVector3D lightPos, float &minX, float &maxX, float &minZ, float &maxZ )
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
//  depth = viewCenter.z();
  QVector<QVector3D> viewFrustrumPoints =
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
  float shadowRenderingDistance = 500.0f;
  for ( int i = 0; i < viewFrustrumPoints.size(); ++i )
  {
    viewFrustrumPoints[i] = WorldPosFromDepth(
                              projectionMatrixInv, viewMatrixInv,
                              viewFrustrumPoints[i].x(), viewFrustrumPoints[i].y(), viewFrustrumPoints[i].z() );
    QVector3D pt = cameraPos;
    QVector3D vect = ( viewFrustrumPoints[i] - pt ).normalized();
//    QVector3D p = intersectionWithY0Plane(cameraPos, (viewFrustrumPoints[i] - cameraPos).normalized());
    float t = ( y - pt.y() ) / vect.y();
    qDebug() << "t: " << t;
    if ( t < 0 ) qDebug() << "Negative";
    if ( t < 0 )
      t = shadowRenderingDistance;
//    viewFrustrumPoints[i] = QVector3D(pt.x() + t * vect.x(), 0.0f, pt.z() + t * vect.z());
    viewFrustrumPoints[i] = pt + t * vect;
    minX = qMin( minX, viewFrustrumPoints[i].x() );
    maxX = qMax( maxX, viewFrustrumPoints[i].x() );
    minZ = qMin( minZ, viewFrustrumPoints[i].z() );
    maxZ = qMax( maxZ, viewFrustrumPoints[i].z() );
  }
}

//void QgsShadowRenderingFrameGraph::setupDirectionalLight(const QgsDirectionalLightSettings &light) {
//  QVector3D lookingAt = mMainCamera->viewCenter();
////  ViewBoundingBox boundingBox = calculateExtent(mMainCamera);
//  float shadowRenderingDistance = 300.0f;
//  QVector3D vertical = QVector3D(0.0f, 2 * shadowRenderingDistance, 0.0f);
//  QVector3D lightPosition = lookingAt + vertical;
//  QVector3D lightDirection = QVector3D(light.direction().x(), light.direction().y(), light.direction().z()).normalized();
//  lightDirection *= 2 * shadowRenderingDistance;

//  mLightCamera->setPosition(lightPosition);
//  mLightCamera->setViewCenter(lookingAt);
//  mLightCamera->setUpVector(QVector3D(0.0f, 1.0f, 0.0f));
//  mLightCamera->rotateAboutViewCenter(QQuaternion::rotationTo(-vertical.normalized(), lightDirection.normalized()));

////  ViewBoundingBox boundingBox2 = transformToViewSpace(boundingBox, mLightCamera);
//  mLightCamera->setProjectionType(Qt3DRender::QCameraLens::ProjectionType::OrthographicProjection);
//  mLightCamera->lens()->setOrthographicProjection(
//    -shadowRenderingDistance * 1.2f, shadowRenderingDistance * 1.2f,
//    -shadowRenderingDistance * 1.2f, shadowRenderingDistance * 1.2f,
//    0.1f, 3 * shadowRenderingDistance);
////  mLightCamera->lens()->setOrthographicProjection(
////    boundingBox2.minX, boundingBox2.maxX,
////    boundingBox2.minZ, boundingBox2.maxZ,
////    1.0f, 1000.0f);//boundingBox2.minZ, boundingBox2.maxZ);

////  mPostprocessingEntity->setupShadowRenderingExtent(boundingBox.minX, boundingBox.maxX, boundingBox.minY, boundingBox.maxY);
//  mPostprocessingEntity->setupShadowRenderingExtent(
//    lookingAt.x()-shadowRenderingDistance, lookingAt.x()+shadowRenderingDistance,
//    lookingAt.z()-shadowRenderingDistance, lookingAt.z()+shadowRenderingDistance);
//  mPostprocessingEntity->setupDirectionalLight(lightPosition, lightDirection);
//}


void QgsShadowRenderingFrameGraph::setupDirectionalLight( const QgsDirectionalLightSettings &light, QgsRectangle extent )
{
//  ViewBoundingBox boundingBox = calculateExtent(mMainCamera);
  float minX, maxX, minZ, maxZ;
  float shadowRenderingDistance = 100.0f;
//  minX = qMin(minX, mMainCamera->viewCenter().x() - shadowRenderingDistance);
//  maxX = qMax(maxX, mMainCamera->viewCenter().x() + shadowRenderingDistance);
//  minZ = qMin(minZ, mMainCamera->viewCenter().z() - shadowRenderingDistance);
//  maxZ = qMax(maxZ, mMainCamera->viewCenter().z() + shadowRenderingDistance);
//  minX = qMin(minX, (float)extent.xMinimum());
//  maxX = qMax(maxX, (float)extent.xMaximum());
//  minZ = qMin(minZ, (float)extent.yMinimum());
//  maxZ = qMax(maxZ, (float)extent.yMaximum());
  QVector3D lookingAt = mMainCamera->viewCenter();//(0.5 * (minX + maxX), mMainCamera->viewCenter().y(), 0.5 * (minZ + maxZ));
  float d = 2 * ( mMainCamera->position() - mMainCamera->viewCenter() ).length();

  QVector3D vertical = QVector3D( 0.0f, d, 0.0f );
  QVector3D lightPosition = lookingAt + vertical;
  QVector3D lightDirection = QVector3D( light.direction().x(), light.direction().y(), light.direction().z() ).normalized();
  calculateViewExtent( mMainCamera, lookingAt.y(), lightPosition, minX, maxX, minZ, maxZ );

//  lightDirection *= 2 * shadowRenderingDistance;

  mLightCamera->setPosition( lightPosition );
  mLightCamera->setViewCenter( lookingAt );
//  mLightCamera->setUpVector(mMainCamera->upVector());
  mLightCamera->setUpVector( QVector3D( 0.0f, 1.0f, 0.0f ) );
  mLightCamera->rotateAboutViewCenter( QQuaternion::rotationTo( -vertical.normalized(), lightDirection.normalized() ) );

//  ViewBoundingBox boundingBox2 = transformToViewSpace(boundingBox, mLightCamera);
  mLightCamera->setProjectionType( Qt3DRender::QCameraLens::ProjectionType::OrthographicProjection );
  mLightCamera->lens()->setOrthographicProjection(
    - 0.5 * ( maxX - minX ), 0.5 * ( maxX - minX ),
    - 0.5 * ( maxZ - minZ ), 0.5 * ( maxZ - minZ ),
    1.0f, 2 * ( lookingAt - lightPosition ).length() );

//  mPostprocessingEntity->setupShadowRenderingExtent(boundingBox.minX, boundingBox.maxX, boundingBox.minY, boundingBox.maxY);
  mPostprocessingEntity->setupShadowRenderingExtent( minX, maxX, minZ, maxZ );
  mPostprocessingEntity->setupDirectionalLight( lightPosition, lightDirection );
}

