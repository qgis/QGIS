/***************************************************************************
  qgsshadowrenderingframegraph.cpp
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

#include "qgsshadowrenderingframegraph.h"

#include "qgsdirectionallightsettings.h"
#include "qgscameracontroller.h"
#include "qgsrectangle.h"
#include "qgspostprocessingentity.h"
#include "qgspreviewquad.h"

Qt3DRender::QFrameGraphNode *QgsShadowRenderingFrameGraph::constructTexturesPreviewPass()
{
  mPreviewLayerFilter = new Qt3DRender::QLayerFilter;
  mPreviewLayerFilter->addLayer( mPreviewLayer );

  mPreviewRenderStateSet = new Qt3DRender::QRenderStateSet( mPreviewLayerFilter );
  mPreviewDepthTest = new Qt3DRender::QDepthTest;
  mPreviewDepthTest->setDepthFunction( Qt3DRender::QDepthTest::Always );
  mPreviewRenderStateSet->addRenderState( mPreviewDepthTest );
  mPreviewCullFace = new Qt3DRender::QCullFace;
  mPreviewCullFace->setMode( Qt3DRender::QCullFace::NoCulling );
  mPreviewRenderStateSet->addRenderState( mPreviewCullFace );

  return mPreviewLayerFilter;
}

Qt3DRender::QFrameGraphNode *QgsShadowRenderingFrameGraph::constructForwardRenderPass()
{
  mForwardRenderLayerFilter = new Qt3DRender::QLayerFilter( mMainCameraSelector );
  mForwardRenderLayerFilter->addLayer( mForwardRenderLayer );

  mRenderCapture = new Qt3DRender::QRenderCapture( mForwardRenderLayerFilter );

  // TODO: make the width and height change dynamically as the 3D viewer is resized
  int width = 1024;
  int height = 768;

  mForwardColorTexture = new Qt3DRender::QTexture2D;
  mForwardColorTexture->setWidth( width );
  mForwardColorTexture->setHeight( height );
  mForwardColorTexture->setFormat( Qt3DRender::QTexture2D::TextureFormat::RGBA16F );
  mForwardColorTexture->setGenerateMipMaps( false );
  mForwardColorTexture->setMagnificationFilter( Qt3DRender::QTexture2D::Linear );
  mForwardColorTexture->setMinificationFilter( Qt3DRender::QTexture2D::Linear );
  mForwardColorTexture->wrapMode()->setX( Qt3DRender::QTextureWrapMode::ClampToEdge );
  mForwardColorTexture->wrapMode()->setY( Qt3DRender::QTextureWrapMode::ClampToEdge );

  mForwardDepthTexture = new Qt3DRender::QTexture2D;
  mForwardDepthTexture->setWidth( width );
  mForwardDepthTexture->setHeight( height );
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

  mForwardRenderTargetSelector = new Qt3DRender::QRenderTargetSelector( mForwardRenderLayerFilter );
  mForwardRenderTargetSelector->setTarget( mForwardRenderTarget );

  mForwardClearBuffers = new Qt3DRender::QClearBuffers( mForwardRenderTargetSelector );
  mForwardClearBuffers->setClearColor( QColor::fromRgbF( 0.0, 1.0, 0.0, 1.0 ) );
  mForwardClearBuffers->setBuffers( Qt3DRender::QClearBuffers::ColorDepthBuffer );

  mFrustumCulling = new Qt3DRender::QFrustumCulling;
  mFrustumCulling->setParent( mForwardClearBuffers );

  return mForwardRenderLayerFilter;
}

Qt3DRender::QFrameGraphNode *QgsShadowRenderingFrameGraph::constructShadowRenderPass()
{
  mShadowSceneEntitiesFilter = new Qt3DRender::QLayerFilter;
  mShadowSceneEntitiesFilter->addLayer( mCastShadowsLayer );

  mShadowMapTexture = new Qt3DRender::QTexture2D;
  mShadowMapTexture->setWidth( mShadowMapResolution );
  mShadowMapTexture->setHeight( mShadowMapResolution );
  mShadowMapTexture->setFormat( Qt3DRender::QTexture2D::TextureFormat::D32F );
  mShadowMapTexture->setGenerateMipMaps( false );
  mShadowMapTexture->setMagnificationFilter( Qt3DRender::QTexture2D::Linear );
  mShadowMapTexture->setMinificationFilter( Qt3DRender::QTexture2D::Linear );
  mShadowMapTexture->wrapMode()->setX( Qt3DRender::QTextureWrapMode::ClampToEdge );
  mShadowMapTexture->wrapMode()->setY( Qt3DRender::QTextureWrapMode::ClampToEdge );

  mShadowRenderTarget = new Qt3DRender::QRenderTarget;
  mShadowRenderTargetOutput = new Qt3DRender::QRenderTargetOutput;
  mShadowRenderTargetOutput->setAttachmentPoint( Qt3DRender::QRenderTargetOutput::Depth );
  mShadowRenderTargetOutput->setTexture( mShadowMapTexture );
  mShadowRenderTarget->addOutput( mShadowRenderTargetOutput );

  mShadowRenderTargetSelector = new Qt3DRender::QRenderTargetSelector( mShadowSceneEntitiesFilter );
  mShadowRenderTargetSelector->setTarget( mShadowRenderTarget );

  mShadowClearBuffers = new Qt3DRender::QClearBuffers( mShadowRenderTargetSelector );
  mShadowClearBuffers ->setBuffers( Qt3DRender::QClearBuffers::BufferType::ColorDepthBuffer );

  mShadowRenderStateSet = new Qt3DRender::QRenderStateSet( mShadowClearBuffers );
  mShadowDepthTest = new Qt3DRender::QDepthTest;
  mShadowDepthTest->setDepthFunction( Qt3DRender::QDepthTest::Less );
  mShadowRenderStateSet->addRenderState( mShadowDepthTest );
  mShadowCullFace = new Qt3DRender::QCullFace;
  mShadowCullFace->setMode( Qt3DRender::QCullFace::NoCulling );
  mShadowRenderStateSet->addRenderState( mShadowCullFace );

  return mShadowSceneEntitiesFilter;
}

Qt3DRender::QFrameGraphNode *QgsShadowRenderingFrameGraph::constructPostprocessingPass()
{
  mPostprocessPassLayerFilter = new Qt3DRender::QLayerFilter;
  mPostprocessPassLayerFilter->addLayer( mPostprocessPassLayer );

  mPostprocessClearBuffers = new Qt3DRender::QClearBuffers( mPostprocessPassLayerFilter );
  mPostprocessClearBuffers->setClearColor( QColor::fromRgbF( 0.0f, 0.0f, 0.0f ) );


  return mPostprocessPassLayerFilter;
}

QgsShadowRenderingFrameGraph::QgsShadowRenderingFrameGraph( QWindow *window, Qt3DRender::QCamera *mainCamera, Qt3DCore::QEntity *root )
{
  mRootEntity = root;
  mMainCamera = mainCamera;
  mLightCamera = new Qt3DRender::QCamera;

  mPostprocessPassLayer = new Qt3DRender::QLayer;
  mPreviewLayer = new Qt3DRender::QLayer;
  mCastShadowsLayer = new Qt3DRender::QLayer;
  mForwardRenderLayer = new Qt3DRender::QLayer;

#if QT_VERSION >= QT_VERSION_CHECK(5, 10, 0)
  mPostprocessPassLayer->setRecursive( true );
  mPreviewLayer->setRecursive( true );
  mCastShadowsLayer->setRecursive( true );
  mForwardRenderLayer->setRecursive( true );
#endif

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


  mPostprocessingEntity = new QgsPostprocessingEntity( this, mRootEntity );
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
void calculateViewExtent( Qt3DRender::QCamera *camera, float shadowRenderingDistance, float y, float &minX, float &maxX, float &minY, float &maxY, float &minZ, float &maxZ )
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
  maxY = std::numeric_limits<float>::lowest();
  maxZ = std::numeric_limits<float>::lowest();
  minX = std::numeric_limits<float>::max();
  minY = std::numeric_limits<float>::max();
  minZ = std::numeric_limits<float>::max();
  for ( int i = 0; i < viewFrustumPoints.size(); ++i )
  {
    // convert from view port space to world space
    viewFrustumPoints[i] = WorldPosFromDepth(
                             projectionMatrixInv, viewMatrixInv,
                             viewFrustumPoints[i].x(), viewFrustumPoints[i].y(), viewFrustumPoints[i].z() );
    minX = std::min( minX, viewFrustumPoints[i].x() );
    maxX = std::max( maxX, viewFrustumPoints[i].x() );
    minY = std::min( minY, viewFrustumPoints[i].y() );
    maxY = std::max( maxY, viewFrustumPoints[i].y() );
    minZ = std::min( minZ, viewFrustumPoints[i].z() );
    maxZ = std::max( maxZ, viewFrustumPoints[i].z() );
    // find the intersection between the line going from cameraPos to the frustum quad point
    // and the horizontal plane Y=y
    // if the intersection is on the back side of the viewing panel we get a point that is
    // shadowRenderingDistance units in front of the camera
    QVector3D pt = cameraPos;
    QVector3D vect = ( viewFrustumPoints[i] - pt ).normalized();
    float t = ( y - pt.y() ) / vect.y();
    if ( t < 0 )
      t = shadowRenderingDistance;
    else
      t = std::min( t, shadowRenderingDistance );
    viewFrustumPoints[i] = pt + t * vect;
    minX = std::min( minX, viewFrustumPoints[i].x() );
    maxX = std::max( maxX, viewFrustumPoints[i].x() );
    minY = std::min( minY, viewFrustumPoints[i].y() );
    maxY = std::max( maxY, viewFrustumPoints[i].y() );
    minZ = std::min( minZ, viewFrustumPoints[i].z() );
    maxZ = std::max( maxZ, viewFrustumPoints[i].z() );
  }
}

void QgsShadowRenderingFrameGraph::setupDirectionalLight( const QgsDirectionalLightSettings &light, float maximumShadowRenderingDistance )
{
  float minX, maxX, minY, maxY, minZ, maxZ;
  QVector3D lookingAt = mMainCamera->viewCenter();
  float d = 2 * ( mMainCamera->position() - mMainCamera->viewCenter() ).length();

  QVector3D vertical = QVector3D( 0.0f, d, 0.0f );
  QVector3D lightDirection = QVector3D( light.direction().x(), light.direction().y(), light.direction().z() ).normalized();
  calculateViewExtent( mMainCamera, maximumShadowRenderingDistance, lookingAt.y(), minX, maxX, minY, maxY, minZ, maxZ );

  lookingAt = QVector3D( 0.5 * ( minX + maxX ), mMainCamera->viewCenter().y(), 0.5 * ( minZ + maxZ ) );
  QVector3D lightPosition = lookingAt + vertical;
  mLightCamera->setPosition( lightPosition );
  mLightCamera->setViewCenter( lookingAt );
  mLightCamera->setUpVector( QVector3D( 0.0f, 1.0f, 0.0f ) );
  mLightCamera->rotateAboutViewCenter( QQuaternion::rotationTo( vertical.normalized(), -lightDirection.normalized() ) );

  mLightCamera->setProjectionType( Qt3DRender::QCameraLens::ProjectionType::OrthographicProjection );
  mLightCamera->lens()->setOrthographicProjection(
    - 0.7 * ( maxX - minX ), 0.7 * ( maxX - minX ),
    - 0.7 * ( maxZ - minZ ), 0.7 * ( maxZ - minZ ),
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
  if ( mShadowRenderingEnabled )
    mShadowSceneEntitiesFilter->setEnabled( true );
  else
    mShadowSceneEntitiesFilter->setEnabled( false );
}

void QgsShadowRenderingFrameGraph::setShadowBias( float shadowBias )
{
  mShadowBias = shadowBias;
  mPostprocessingEntity->setShadowBias( mShadowBias );
}

void QgsShadowRenderingFrameGraph::setShadowMapResolution( int resolution )
{
  mShadowMapResolution = resolution;
  mShadowMapTexture->setWidth( mShadowMapResolution );
  mShadowMapTexture->setHeight( mShadowMapResolution );
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
