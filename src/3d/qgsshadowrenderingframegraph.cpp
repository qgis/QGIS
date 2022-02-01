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
#include "qgs3dutils.h"

#include <Qt3DRender/QAttribute>
#include <Qt3DRender/QBuffer>
#include <Qt3DRender/QTechnique>
#include <Qt3DRender/QGraphicsApiFilter>


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
  mMainCameraSelector = new Qt3DRender::QCameraSelector;
  mMainCameraSelector->setCamera( mMainCamera );

  mForwardRenderLayerFilter = new Qt3DRender::QLayerFilter( mMainCameraSelector );
  mForwardRenderLayerFilter->addLayer( mForwardRenderLayer );

  mForwardColorTexture = new Qt3DRender::QTexture2D;
  mForwardColorTexture->setWidth( mSize.width() );
  mForwardColorTexture->setHeight( mSize.height() );
  mForwardColorTexture->setFormat( Qt3DRender::QAbstractTexture::RGB8_UNorm );
  mForwardColorTexture->setGenerateMipMaps( false );
  mForwardColorTexture->setMagnificationFilter( Qt3DRender::QTexture2D::Linear );
  mForwardColorTexture->setMinificationFilter( Qt3DRender::QTexture2D::Linear );
  mForwardColorTexture->wrapMode()->setX( Qt3DRender::QTextureWrapMode::ClampToEdge );
  mForwardColorTexture->wrapMode()->setY( Qt3DRender::QTextureWrapMode::ClampToEdge );

  mForwardDepthTexture = new Qt3DRender::QTexture2D;
  mForwardDepthTexture->setWidth( mSize.width() );
  mForwardDepthTexture->setHeight( mSize.height() );
  mForwardDepthTexture->setFormat( Qt3DRender::QTexture2D::TextureFormat::DepthFormat );
  mForwardDepthTexture->setGenerateMipMaps( false );
  mForwardDepthTexture->setMagnificationFilter( Qt3DRender::QTexture2D::Linear );
  mForwardDepthTexture->setMinificationFilter( Qt3DRender::QTexture2D::Linear );
  mForwardDepthTexture->wrapMode()->setX( Qt3DRender::QTextureWrapMode::ClampToEdge );
  mForwardDepthTexture->wrapMode()->setY( Qt3DRender::QTextureWrapMode::ClampToEdge );

  Qt3DRender::QRenderTarget *forwardRenderTarget = new Qt3DRender::QRenderTarget;
  Qt3DRender::QRenderTargetOutput *forwardRenderTargetDepthOutput = new Qt3DRender::QRenderTargetOutput;
  forwardRenderTargetDepthOutput->setAttachmentPoint( Qt3DRender::QRenderTargetOutput::Depth );
  forwardRenderTargetDepthOutput->setTexture( mForwardDepthTexture );
  forwardRenderTarget->addOutput( forwardRenderTargetDepthOutput );
  Qt3DRender::QRenderTargetOutput *forwardRenderTargetColorOutput = new Qt3DRender::QRenderTargetOutput;
  forwardRenderTargetColorOutput->setAttachmentPoint( Qt3DRender::QRenderTargetOutput::Color0 );
  forwardRenderTargetColorOutput->setTexture( mForwardColorTexture );
  forwardRenderTarget->addOutput( forwardRenderTargetColorOutput );

  mForwardRenderTargetSelector = new Qt3DRender::QRenderTargetSelector( mForwardRenderLayerFilter );
  mForwardRenderTargetSelector->setTarget( forwardRenderTarget );

  mForwardClearBuffers = new Qt3DRender::QClearBuffers( mForwardRenderTargetSelector );
  mForwardClearBuffers->setClearColor( QColor::fromRgbF( 0.0, 0.0, 1.0, 1.0 ) );
  mForwardClearBuffers->setBuffers( Qt3DRender::QClearBuffers::ColorDepthBuffer );
  mForwardClearBuffers->setClearDepthValue( 1.0f );

  Qt3DRender::QRenderStateSet *forwaredRenderStateSet = new Qt3DRender::QRenderStateSet( mForwardClearBuffers );

  Qt3DRender::QDepthTest *depthTest = new Qt3DRender::QDepthTest;
  depthTest->setDepthFunction( Qt3DRender::QDepthTest::Less );
  forwaredRenderStateSet->addRenderState( depthTest );

  Qt3DRender::QCullFace *cullFace = new Qt3DRender::QCullFace;
  cullFace->setMode( Qt3DRender::QCullFace::CullingMode::Back );
  forwaredRenderStateSet->addRenderState( cullFace );

  mFrustumCulling = new Qt3DRender::QFrustumCulling( forwaredRenderStateSet );

  return mMainCameraSelector;
}

Qt3DRender::QFrameGraphNode *QgsShadowRenderingFrameGraph::constructShadowRenderPass()
{
  mLightCameraSelectorShadowPass = new Qt3DRender::QCameraSelector;
  mLightCameraSelectorShadowPass->setCamera( mLightCamera );

  mShadowSceneEntitiesFilter = new Qt3DRender::QLayerFilter( mLightCameraSelectorShadowPass );
  mShadowSceneEntitiesFilter->addLayer( mCastShadowsLayer );

  mShadowMapTexture = new Qt3DRender::QTexture2D;
  mShadowMapTexture->setWidth( mShadowMapResolution );
  mShadowMapTexture->setHeight( mShadowMapResolution );
  mShadowMapTexture->setFormat( Qt3DRender::QTexture2D::TextureFormat::DepthFormat );
  mShadowMapTexture->setGenerateMipMaps( false );
  mShadowMapTexture->setMagnificationFilter( Qt3DRender::QTexture2D::Linear );
  mShadowMapTexture->setMinificationFilter( Qt3DRender::QTexture2D::Linear );
  mShadowMapTexture->wrapMode()->setX( Qt3DRender::QTextureWrapMode::ClampToEdge );
  mShadowMapTexture->wrapMode()->setY( Qt3DRender::QTextureWrapMode::ClampToEdge );

  Qt3DRender::QRenderTarget *shadowRenderTarget = new Qt3DRender::QRenderTarget;
  Qt3DRender::QRenderTargetOutput *shadowRenderTargetOutput = new Qt3DRender::QRenderTargetOutput;
  shadowRenderTargetOutput->setAttachmentPoint( Qt3DRender::QRenderTargetOutput::Depth );
  shadowRenderTargetOutput->setTexture( mShadowMapTexture );
  shadowRenderTarget->addOutput( shadowRenderTargetOutput );

  mShadowRenderTargetSelector = new Qt3DRender::QRenderTargetSelector( mShadowSceneEntitiesFilter );
  mShadowRenderTargetSelector->setTarget( shadowRenderTarget );

  mShadowClearBuffers = new Qt3DRender::QClearBuffers( mShadowRenderTargetSelector );
  mShadowClearBuffers->setBuffers( Qt3DRender::QClearBuffers::BufferType::ColorDepthBuffer );
  mShadowClearBuffers->setClearColor( QColor::fromRgbF( 0.0f, 1.0f, 0.0f ) );

  mShadowRenderStateSet = new Qt3DRender::QRenderStateSet( mShadowClearBuffers );

  Qt3DRender::QDepthTest *shadowDepthTest = new Qt3DRender::QDepthTest;
  shadowDepthTest->setDepthFunction( Qt3DRender::QDepthTest::Less );
  mShadowRenderStateSet->addRenderState( shadowDepthTest );

  Qt3DRender::QCullFace *shadowCullFace = new Qt3DRender::QCullFace;
  shadowCullFace->setMode( Qt3DRender::QCullFace::CullingMode::Front );
  mShadowRenderStateSet->addRenderState( shadowCullFace );

  Qt3DRender::QPolygonOffset *polygonOffset = new Qt3DRender::QPolygonOffset;
  polygonOffset->setDepthSteps( 4.0 );
  polygonOffset->setScaleFactor( 1.1 );
  mShadowRenderStateSet->addRenderState( polygonOffset );

  return mLightCameraSelectorShadowPass;
}

Qt3DRender::QFrameGraphNode *QgsShadowRenderingFrameGraph::constructPostprocessingPass()
{
  mPostProcessingCameraSelector = new Qt3DRender::QCameraSelector;
  mPostProcessingCameraSelector->setCamera( mLightCamera );

  mPostprocessPassLayerFilter = new Qt3DRender::QLayerFilter( mPostProcessingCameraSelector );
  mPostprocessPassLayerFilter->addLayer( mPostprocessPassLayer );

  mPostprocessClearBuffers = new Qt3DRender::QClearBuffers( mPostprocessPassLayerFilter );

  mRenderCaptureTargetSelector = new Qt3DRender::QRenderTargetSelector( mPostprocessClearBuffers );

  Qt3DRender::QRenderTarget *renderTarget = new Qt3DRender::QRenderTarget( mRenderCaptureTargetSelector );

  // The lifetime of the objects created here is managed
  // automatically, as they become children of this object.

  // Create a render target output for rendering color.
  Qt3DRender::QRenderTargetOutput *colorOutput = new Qt3DRender::QRenderTargetOutput( renderTarget );
  colorOutput->setAttachmentPoint( Qt3DRender::QRenderTargetOutput::Color0 );

  // Create a texture to render into.
  mRenderCaptureColorTexture = new Qt3DRender::QTexture2D( colorOutput );
  mRenderCaptureColorTexture->setSize( mSize.width(), mSize.height() );
  mRenderCaptureColorTexture->setFormat( Qt3DRender::QAbstractTexture::RGB8_UNorm );
  mRenderCaptureColorTexture->setMinificationFilter( Qt3DRender::QAbstractTexture::Linear );
  mRenderCaptureColorTexture->setMagnificationFilter( Qt3DRender::QAbstractTexture::Linear );

  // Hook the texture up to our output, and the output up to this object.
  colorOutput->setTexture( mRenderCaptureColorTexture );
  renderTarget->addOutput( colorOutput );

  Qt3DRender::QRenderTargetOutput *depthOutput = new Qt3DRender::QRenderTargetOutput( renderTarget );

  depthOutput->setAttachmentPoint( Qt3DRender::QRenderTargetOutput::Depth );
  mRenderCaptureDepthTexture = new Qt3DRender::QTexture2D( depthOutput );
  mRenderCaptureDepthTexture->setSize( mSize.width(), mSize.height() );
  mRenderCaptureDepthTexture->setFormat( Qt3DRender::QAbstractTexture::DepthFormat );
  mRenderCaptureDepthTexture->setMinificationFilter( Qt3DRender::QAbstractTexture::Linear );
  mRenderCaptureDepthTexture->setMagnificationFilter( Qt3DRender::QAbstractTexture::Linear );
  mRenderCaptureDepthTexture->setComparisonFunction( Qt3DRender::QAbstractTexture::CompareLessEqual );
  mRenderCaptureDepthTexture->setComparisonMode( Qt3DRender::QAbstractTexture::CompareRefToTexture );

  depthOutput->setTexture( mRenderCaptureDepthTexture );
  renderTarget->addOutput( depthOutput );

  mRenderCaptureTargetSelector->setTarget( renderTarget );

  mRenderCapture = new Qt3DRender::QRenderCapture( mRenderCaptureTargetSelector );

  return mPostProcessingCameraSelector;
}

Qt3DRender::QFrameGraphNode *QgsShadowRenderingFrameGraph::constructDepthRenderPass()
{
  // depth buffer render to copy pass

  mDepthRenderCameraSelector = new Qt3DRender::QCameraSelector;
  mDepthRenderCameraSelector->setCamera( mMainCamera );

  mDepthRenderStateSet = new Qt3DRender::QRenderStateSet( mDepthRenderCameraSelector );

  Qt3DRender::QDepthTest *depthRenderDepthTest = new Qt3DRender::QDepthTest;
  depthRenderDepthTest->setDepthFunction( Qt3DRender::QDepthTest::Always );;
  Qt3DRender::QCullFace *depthRenderCullFace = new Qt3DRender::QCullFace;
  depthRenderCullFace->setMode( Qt3DRender::QCullFace::NoCulling );

  mDepthRenderStateSet->addRenderState( depthRenderDepthTest );
  mDepthRenderStateSet->addRenderState( depthRenderCullFace );

  mDepthRenderLayerFilter = new Qt3DRender::QLayerFilter( mDepthRenderStateSet );
  mDepthRenderLayerFilter->addLayer( mDepthRenderPassLayer );

  mDepthRenderCaptureTargetSelector = new Qt3DRender::QRenderTargetSelector( mDepthRenderLayerFilter );
  Qt3DRender::QRenderTarget *depthRenderTarget = new Qt3DRender::QRenderTarget( mDepthRenderCaptureTargetSelector );

  // The lifetime of the objects created here is managed
  // automatically, as they become children of this object.

  // Create a render target output for rendering color.
  Qt3DRender::QRenderTargetOutput *colorOutput = new Qt3DRender::QRenderTargetOutput( depthRenderTarget );
  colorOutput->setAttachmentPoint( Qt3DRender::QRenderTargetOutput::Color0 );

  // Create a texture to render into.
  mDepthRenderCaptureColorTexture = new Qt3DRender::QTexture2D( colorOutput );
  mDepthRenderCaptureColorTexture->setSize( mSize.width(), mSize.height() );
  mDepthRenderCaptureColorTexture->setFormat( Qt3DRender::QAbstractTexture::RGB8_UNorm );
  mDepthRenderCaptureColorTexture->setMinificationFilter( Qt3DRender::QAbstractTexture::Linear );
  mDepthRenderCaptureColorTexture->setMagnificationFilter( Qt3DRender::QAbstractTexture::Linear );

  // Hook the texture up to our output, and the output up to this object.
  colorOutput->setTexture( mDepthRenderCaptureColorTexture );
  depthRenderTarget->addOutput( colorOutput );

  Qt3DRender::QRenderTargetOutput *depthOutput = new Qt3DRender::QRenderTargetOutput( depthRenderTarget );

  depthOutput->setAttachmentPoint( Qt3DRender::QRenderTargetOutput::Depth );
  mDepthRenderCaptureDepthTexture = new Qt3DRender::QTexture2D( depthOutput );
  mDepthRenderCaptureDepthTexture->setSize( mSize.width(), mSize.height() );
  mDepthRenderCaptureDepthTexture->setFormat( Qt3DRender::QAbstractTexture::DepthFormat );
  mDepthRenderCaptureDepthTexture->setMinificationFilter( Qt3DRender::QAbstractTexture::Linear );
  mDepthRenderCaptureDepthTexture->setMagnificationFilter( Qt3DRender::QAbstractTexture::Linear );
  mDepthRenderCaptureDepthTexture->setComparisonFunction( Qt3DRender::QAbstractTexture::CompareLessEqual );
  mDepthRenderCaptureDepthTexture->setComparisonMode( Qt3DRender::QAbstractTexture::CompareRefToTexture );

  depthOutput->setTexture( mDepthRenderCaptureDepthTexture );
  depthRenderTarget->addOutput( depthOutput );

  mDepthRenderCaptureTargetSelector->setTarget( depthRenderTarget );

  // Note: We do not a clear buffers node since we are drawing a quad that will override the buffer's content anyway
  mDepthRenderCapture = new Qt3DRender::QRenderCapture( mDepthRenderCaptureTargetSelector );

  return mDepthRenderCameraSelector;
}

Qt3DCore::QEntity *QgsShadowRenderingFrameGraph::constructDepthRenderQuad()
{
  Qt3DCore::QEntity *quad = new Qt3DCore::QEntity;
  quad->setObjectName( "depthRenderQuad" );

  Qt3DRender::QGeometry *geom = new Qt3DRender::QGeometry;
  Qt3DRender::QAttribute *positionAttribute = new Qt3DRender::QAttribute;
  const QVector<float> vert = { -1.0f, -1.0f, 1.0f, /**/ 1.0f, -1.0f, 1.0f, /**/ -1.0f,  1.0f, 1.0f, /**/ -1.0f,  1.0f, 1.0f, /**/ 1.0f, -1.0f, 1.0f, /**/ 1.0f,  1.0f, 1.0f };

  const QByteArray vertexArr( ( const char * ) vert.constData(), vert.size() * sizeof( float ) );
  Qt3DRender::QBuffer *vertexBuffer = nullptr;
  vertexBuffer = new Qt3DRender::QBuffer( this );
  vertexBuffer->setData( vertexArr );

  positionAttribute->setName( Qt3DRender::QAttribute::defaultPositionAttributeName() );
  positionAttribute->setVertexBaseType( Qt3DRender::QAttribute::Float );
  positionAttribute->setVertexSize( 3 );
  positionAttribute->setAttributeType( Qt3DRender::QAttribute::VertexAttribute );
  positionAttribute->setBuffer( vertexBuffer );
  positionAttribute->setByteOffset( 0 );
  positionAttribute->setByteStride( 3 * sizeof( float ) );
  positionAttribute->setCount( 6 );

  geom->addAttribute( positionAttribute );

  Qt3DRender::QGeometryRenderer *renderer = new Qt3DRender::QGeometryRenderer;
  renderer->setPrimitiveType( Qt3DRender::QGeometryRenderer::PrimitiveType::Triangles );
  renderer->setGeometry( geom );

  quad->addComponent( renderer );

  QMatrix4x4 modelMatrix;
  modelMatrix.setToIdentity();

  // construct material

  Qt3DRender::QMaterial *material = new Qt3DRender::QMaterial;
  Qt3DRender::QParameter *textureParameter = new Qt3DRender::QParameter( "depthTexture", mForwardDepthTexture );
  Qt3DRender::QParameter *textureTransformParameter = new Qt3DRender::QParameter( "modelMatrix", QVariant::fromValue( modelMatrix ) );
  material->addParameter( textureParameter );
  material->addParameter( textureTransformParameter );

  Qt3DRender::QEffect *effect = new Qt3DRender::QEffect;

  Qt3DRender::QTechnique *technique = new Qt3DRender::QTechnique;

  Qt3DRender::QGraphicsApiFilter *graphicsApiFilter = technique->graphicsApiFilter();
  graphicsApiFilter->setApi( Qt3DRender::QGraphicsApiFilter::Api::OpenGL );
  graphicsApiFilter->setProfile( Qt3DRender::QGraphicsApiFilter::OpenGLProfile::CoreProfile );
  graphicsApiFilter->setMajorVersion( 1 );
  graphicsApiFilter->setMinorVersion( 5 );

  Qt3DRender::QRenderPass *renderPass = new Qt3DRender::QRenderPass;

  Qt3DRender::QShaderProgram *shader = new Qt3DRender::QShaderProgram;
  shader->setVertexShaderCode( Qt3DRender::QShaderProgram::loadSource( QUrl( "qrc:/shaders/depth_render.vert" ) ) );
  shader->setFragmentShaderCode( Qt3DRender::QShaderProgram::loadSource( QUrl( "qrc:/shaders/depth_render.frag" ) ) );
  renderPass->setShaderProgram( shader );

  technique->addRenderPass( renderPass );

  effect->addTechnique( technique );
  material->setEffect( effect );

  quad->addComponent( material );

  return quad;
}

QgsShadowRenderingFrameGraph::QgsShadowRenderingFrameGraph( QSurface *surface, QSize s, Qt3DRender::QCamera *mainCamera, Qt3DCore::QEntity *root )
  : Qt3DCore::QEntity( root )
  , mSize( s )
{
  mRootEntity = root;
  mMainCamera = mainCamera;
  mLightCamera = new Qt3DRender::QCamera;

  mPostprocessPassLayer = new Qt3DRender::QLayer;
  mPreviewLayer = new Qt3DRender::QLayer;
  mCastShadowsLayer = new Qt3DRender::QLayer;
  mForwardRenderLayer = new Qt3DRender::QLayer;
  mDepthRenderPassLayer = new Qt3DRender::QLayer;


  mPostprocessPassLayer->setRecursive( true );
  mPreviewLayer->setRecursive( true );
  mCastShadowsLayer->setRecursive( true );
  mForwardRenderLayer->setRecursive( true );
  mDepthRenderPassLayer->setRecursive( true );

  mRenderSurfaceSelector = new Qt3DRender::QRenderSurfaceSelector;

  QObject *surfaceObj = dynamic_cast< QObject *  >( surface );
  Q_ASSERT( surfaceObj );

  mRenderSurfaceSelector->setSurface( surfaceObj );
  mRenderSurfaceSelector->setExternalRenderTargetSize( mSize );

  mMainViewPort = new Qt3DRender::QViewport( mRenderSurfaceSelector );
  mMainViewPort->setNormalizedRect( QRectF( 0.0f, 0.0f, 1.0f, 1.0f ) );

  // Forward render
  Qt3DRender::QFrameGraphNode *forwardRenderPass = constructForwardRenderPass();
  forwardRenderPass->setParent( mMainViewPort );

  // shadow rendering pass

  Qt3DRender::QFrameGraphNode *shadowRenderPass = constructShadowRenderPass();
  shadowRenderPass->setParent( mMainViewPort );

  // depth buffer processing
  Qt3DRender::QFrameGraphNode *depthBufferProcessingPass = constructDepthRenderPass();
  depthBufferProcessingPass->setParent( mMainViewPort );

  // post process
  Qt3DRender::QFrameGraphNode *postprocessingPass = constructPostprocessingPass();
  postprocessingPass->setParent( mMainViewPort );

  // textures preview pass
  Qt3DRender::QFrameGraphNode *previewPass = constructTexturesPreviewPass();
  previewPass->setParent( mMainViewPort );

  mPostprocessingEntity = new QgsPostprocessingEntity( this, mRootEntity );
  mPostprocessingEntity->addComponent( mPostprocessPassLayer );

  Qt3DRender::QParameter *depthMapIsDepthParam = new Qt3DRender::QParameter( "isDepth", true );
  Qt3DRender::QParameter *shadowMapIsDepthParam = new Qt3DRender::QParameter( "isDepth", true );

  mDebugDepthMapPreviewQuad = this->addTexturePreviewOverlay( mForwardDepthTexture, QPointF( 0.9f, 0.9f ), QSizeF( 0.1, 0.1 ), QVector<Qt3DRender::QParameter *> { depthMapIsDepthParam } );
  mDebugShadowMapPreviewQuad = this->addTexturePreviewOverlay( mShadowMapTexture, QPointF( 0.9f, 0.9f ), QSizeF( 0.1, 0.1 ), QVector<Qt3DRender::QParameter *> { shadowMapIsDepthParam } );
  mDebugDepthMapPreviewQuad->setEnabled( false );
  mDebugShadowMapPreviewQuad->setEnabled( false );

  mDepthRenderQuad = constructDepthRenderQuad();
  mDepthRenderQuad->addComponent( mDepthRenderPassLayer );
  mDepthRenderQuad->setParent( mRootEntity );
}

QgsPreviewQuad *QgsShadowRenderingFrameGraph::addTexturePreviewOverlay( Qt3DRender::QTexture2D *texture, const QPointF &centerTexCoords, const QSizeF &sizeTexCoords, QVector<Qt3DRender::QParameter *> additionalShaderParameters )
{
  QgsPreviewQuad *previewQuad = new QgsPreviewQuad( texture, centerTexCoords, sizeTexCoords, additionalShaderParameters );
  previewQuad->addComponent( mPreviewLayer );
  previewQuad->setParent( mRootEntity );
  mPreviewQuads.push_back( previewQuad );
  return previewQuad;
}

// computes the portion of the Y=y plane the camera is looking at
void calculateViewExtent( Qt3DRender::QCamera *camera, float shadowRenderingDistance, float y, float &minX, float &maxX, float &minY, float &maxY, float &minZ, float &maxZ )
{
  const QVector3D cameraPos = camera->position();
  const QMatrix4x4 projectionMatrix = camera->projectionMatrix();
  const QMatrix4x4 viewMatrix = camera->viewMatrix();
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
    QVector3D( 1.0f,  1.0f, depth ),
    QVector3D( 0.0f,  0.0f, 0 ),
    QVector3D( 0.0f,  1.0f, 0 ),
    QVector3D( 1.0f,  0.0f, 0 ),
    QVector3D( 1.0f,  1.0f, 0 )
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
    viewFrustumPoints[i] = viewFrustumPoints[i].unproject( viewMatrix, projectionMatrix, QRect( 0, 0, 1, 1 ) );
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
    const QVector3D pt = cameraPos;
    const QVector3D vect = ( viewFrustumPoints[i] - pt ).normalized();
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
  const float d = 2 * ( mMainCamera->position() - mMainCamera->viewCenter() ).length();

  const QVector3D vertical = QVector3D( 0.0f, d, 0.0f );
  const QVector3D lightDirection = QVector3D( light.direction().x(), light.direction().y(), light.direction().z() ).normalized();
  calculateViewExtent( mMainCamera, maximumShadowRenderingDistance, lookingAt.y(), minX, maxX, minY, maxY, minZ, maxZ );

  lookingAt = QVector3D( 0.5 * ( minX + maxX ), mMainCamera->viewCenter().y(), 0.5 * ( minZ + maxZ ) );
  const QVector3D lightPosition = lookingAt + vertical;
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

void QgsShadowRenderingFrameGraph::setupEyeDomeLighting( bool enabled, double strength, int distance )
{
  mEyeDomeLightingEnabled = enabled;
  mEyeDomeLightingStrength = strength;
  mEyeDomeLightingDistance = distance;
  mPostprocessingEntity->setEyeDomeLightingEnabled( enabled );
  mPostprocessingEntity->setEyeDomeLightingStrength( strength );
  mPostprocessingEntity->setEyeDomeLightingDistance( distance );
}

void QgsShadowRenderingFrameGraph::setupShadowMapDebugging( bool enabled, Qt::Corner corner, double size )
{
  mDebugShadowMapPreviewQuad->setEnabled( enabled );
  if ( enabled )
  {
    switch ( corner )
    {
      case Qt::Corner::TopRightCorner:
        mDebugShadowMapPreviewQuad->setViewPort( QPointF( 1.0f - size / 2, 0.0f + size / 2 ), 0.5 * QSizeF( size, size ) );
        break;
      case Qt::Corner::TopLeftCorner:
        mDebugShadowMapPreviewQuad->setViewPort( QPointF( 0.0f + size / 2, 0.0f + size / 2 ), 0.5 * QSizeF( size, size ) );
        break;
      case Qt::Corner::BottomRightCorner:
        mDebugShadowMapPreviewQuad->setViewPort( QPointF( 1.0f - size / 2, 1.0f - size / 2 ), 0.5 * QSizeF( size, size ) );
        break;
      case Qt::Corner::BottomLeftCorner:
        mDebugShadowMapPreviewQuad->setViewPort( QPointF( 0.0f + size / 2, 1.0f - size / 2 ), 0.5 * QSizeF( size, size ) );
        break;
    }
  }
}

void QgsShadowRenderingFrameGraph::setupDepthMapDebugging( bool enabled, Qt::Corner corner, double size )
{
  mDebugDepthMapPreviewQuad->setEnabled( enabled );

  if ( enabled )
  {
    switch ( corner )
    {
      case Qt::Corner::TopRightCorner:
        mDebugDepthMapPreviewQuad->setViewPort( QPointF( 1.0f - size / 2, 0.0f + size / 2 ), 0.5 * QSizeF( size, size ) );
        break;
      case Qt::Corner::TopLeftCorner:
        mDebugDepthMapPreviewQuad->setViewPort( QPointF( 0.0f + size / 2, 0.0f + size / 2 ), 0.5 * QSizeF( size, size ) );
        break;
      case Qt::Corner::BottomRightCorner:
        mDebugDepthMapPreviewQuad->setViewPort( QPointF( 1.0f - size / 2, 1.0f - size / 2 ), 0.5 * QSizeF( size, size ) );
        break;
      case Qt::Corner::BottomLeftCorner:
        mDebugDepthMapPreviewQuad->setViewPort( QPointF( 0.0f + size / 2, 1.0f - size / 2 ), 0.5 * QSizeF( size, size ) );
        break;
    }
  }
}

void QgsShadowRenderingFrameGraph::setSize( QSize s )
{
  mSize = s;
  mForwardColorTexture->setSize( mSize.width(), mSize.height() );
  mForwardDepthTexture->setSize( mSize.width(), mSize.height() );
  mRenderCaptureColorTexture->setSize( mSize.width(), mSize.height() );
  mRenderCaptureDepthTexture->setSize( mSize.width(), mSize.height() );
  mDepthRenderCaptureDepthTexture->setSize( mSize.width(), mSize.height() );
  mDepthRenderCaptureColorTexture->setSize( mSize.width(), mSize.height() );
  mRenderSurfaceSelector->setExternalRenderTargetSize( mSize );
}

void QgsShadowRenderingFrameGraph::setRenderCaptureEnabled( bool enabled )
{
  if ( enabled == mRenderCaptureEnabled )
    return;
  mRenderCaptureEnabled = enabled;
  mRenderCaptureTargetSelector->setEnabled( mRenderCaptureEnabled );
}
