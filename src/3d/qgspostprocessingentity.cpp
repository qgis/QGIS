#include "qgspostprocessingentity.h"

#include <Qt3DRender/QGeometry>
#include <Qt3DRender/QAttribute>
#include <Qt3DRender/QBuffer>
#include <Qt3DRender/QGeometryRenderer>
#include <Qt3DRender/QParameter>
#include <Qt3DRender/QTechnique>
#include <Qt3DRender/QGraphicsApiFilter>
#include <Qt3DRender/QDepthTest>

#include "qgsshadowrenderingframegraph.h"

QgsPostprocessingEntity::QgsPostprocessingEntity( QgsShadowRenderingFrameGraph *frameGraph, const QString &vertexShaderPath, const QString &fragmentShaderPath, QNode *parent )
  : Qt3DCore::QEntity( parent )
{
  Qt3DRender::QGeometry *geom = new Qt3DRender::QGeometry( this );
  Qt3DRender::QAttribute *positionAttribute = new Qt3DRender::QAttribute( this );
  QVector<float> vert =
  {
    -1.0f, -1.0f, 0.0f,
      1.0f, -1.0f, 0.0f,
      -1.0f,  1.0f, 0.0f,
      -1.0f,  1.0f, 0.0f,
      1.0f, -1.0f, 0.0f,
      1.0f,  1.0f, 0.0f
    };

  QByteArray vertexArr( ( const char * ) vert.constData(), vert.size() * sizeof( float ) );
  Qt3DRender::QBuffer *vertexBuffer = new Qt3DRender::QBuffer( this );
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

  Qt3DRender::QGeometryRenderer *renderer = new Qt3DRender::QGeometryRenderer( this );
  renderer->setPrimitiveType( Qt3DRender::QGeometryRenderer::PrimitiveType::Triangles );
  renderer->setGeometry( geom );

  addComponent( renderer );

  mMaterial = new Qt3DRender::QMaterial( this );
  mColorTextureParameter = new Qt3DRender::QParameter( "colorTexture", frameGraph->forwardRenderColorTexture() );
  mDepthTextureParameter = new Qt3DRender::QParameter( "depthTexture", frameGraph->forwardRenderDepthTexture() );
  mShadowMapParameter = new Qt3DRender::QParameter( "shadowTexture", frameGraph->shadowMapTexture() );
  mMaterial->addParameter( mColorTextureParameter );
  mMaterial->addParameter( mDepthTextureParameter );
  mMaterial->addParameter( mShadowMapParameter );

  mMainCamera = frameGraph->mainCamera();
  mLightCamera = frameGraph->lightCamera();

  mFarPlaneParameter = new Qt3DRender::QParameter( "farPlane", mMainCamera->farPlane() );
  mMaterial->addParameter( mFarPlaneParameter );
  connect( mMainCamera, &Qt3DRender::QCamera::farPlaneChanged, [&]( float farPlane )
  {
    mFarPlaneParameter->setValue( farPlane );
  } );
  mNearPlaneParameter = new Qt3DRender::QParameter( "nearPlane", mMainCamera->nearPlane() );
  mMaterial->addParameter( mNearPlaneParameter );
  connect( mMainCamera, &Qt3DRender::QCamera::nearPlaneChanged, [&]( float nearPlane )
  {
    mNearPlaneParameter->setValue( nearPlane );
  } );

  mLightFarPlaneParameter = new Qt3DRender::QParameter( "lightFarPlane", mLightCamera->farPlane() );
  mMaterial->addParameter( mLightFarPlaneParameter );
  connect( mLightCamera, &Qt3DRender::QCamera::farPlaneChanged, [&]( float farPlane )
  {
    mLightFarPlaneParameter->setValue( farPlane );
  } );
  mLightNearPlaneParameter = new Qt3DRender::QParameter( "lightNearPlane", mLightCamera->nearPlane() );
  mMaterial->addParameter( mLightNearPlaneParameter );
  connect( mLightCamera, &Qt3DRender::QCamera::nearPlaneChanged, [&]( float nearPlane )
  {
    mLightNearPlaneParameter->setValue( nearPlane );
  } );

  mMainCameraViewMatrixParameter = new Qt3DRender::QParameter( "cameraView", mMainCamera->viewMatrix() );
  mMaterial->addParameter( mMainCameraViewMatrixParameter );
  mMainCameraProjMatrixParameter = new Qt3DRender::QParameter( "cameraProj", mMainCamera->projectionMatrix() );
  mMaterial->addParameter( mMainCameraProjMatrixParameter );
  connect( mMainCamera, &Qt3DRender::QCamera::projectionMatrixChanged, [&]( const QMatrix4x4 & projectionMatrix )
  {
    mMainCameraProjMatrixParameter->setValue( projectionMatrix );
  } );
  connect( mMainCamera, &Qt3DRender::QCamera::viewMatrixChanged, [&]()
  {
    mMainCameraViewMatrixParameter->setValue( mMainCamera->viewMatrix() );
  } );

  mShadowMinX = new Qt3DRender::QParameter( "shadowMinX", QVariant::fromValue( 0.0f ) );
  mShadowMaxX = new Qt3DRender::QParameter( "shadowMaxX", QVariant::fromValue( 0.0f ) );
  mShadowMinZ = new Qt3DRender::QParameter( "shadowMinZ", QVariant::fromValue( 0.0f ) );
  mShadowMaxZ = new Qt3DRender::QParameter( "shadowMaxZ", QVariant::fromValue( 0.0f ) );
  mMaterial->addParameter( mShadowMinX );
  mMaterial->addParameter( mShadowMaxX );
  mMaterial->addParameter( mShadowMinZ );
  mMaterial->addParameter( mShadowMaxZ );

  mLightPosition = new Qt3DRender::QParameter( "lightPosition", QVariant::fromValue( QVector3D() ) );
  mLightDirection = new Qt3DRender::QParameter( "lightDirection", QVariant::fromValue( QVector3D() ) );

  mEffect = new Qt3DRender::QEffect( this );
  Qt3DRender::QTechnique *technique = new Qt3DRender::QTechnique( this );
  Qt3DRender::QGraphicsApiFilter *graphicsApiFilter = technique->graphicsApiFilter();
  graphicsApiFilter->setApi( Qt3DRender::QGraphicsApiFilter::Api::OpenGL );
  graphicsApiFilter->setProfile( Qt3DRender::QGraphicsApiFilter::OpenGLProfile::CoreProfile );
  graphicsApiFilter->setMajorVersion( 1 );
  graphicsApiFilter->setMinorVersion( 5 );
  Qt3DRender::QRenderPass *renderPass = new Qt3DRender::QRenderPass( this );
  Qt3DRender::QShaderProgram *shader = new Qt3DRender::QShaderProgram( this );

  shader->setVertexShaderCode( Qt3DRender::QShaderProgram::loadSource( QUrl( vertexShaderPath ) ) );
  shader->setFragmentShaderCode( Qt3DRender::QShaderProgram::loadSource( QUrl( fragmentShaderPath ) ) );
  renderPass->setShaderProgram( shader );

  Qt3DRender::QDepthTest *depthTest = new Qt3DRender::QDepthTest( this );
  depthTest->setDepthFunction( Qt3DRender::QDepthTest::Always );

  renderPass->addRenderState( depthTest );

  technique->addRenderPass( renderPass );

  mEffect->addTechnique( technique );
  mMaterial->setEffect( mEffect );

  addComponent( mMaterial );

  addComponent( frameGraph->postprocessingPassLayer() );
}

void QgsPostprocessingEntity::setupShadowRenderingExtent( float minX, float maxX, float minZ, float maxZ )
{
  mShadowMinX->setValue( minX );
  mShadowMaxX->setValue( maxX );
  mShadowMinZ->setValue( minZ );
  mShadowMaxZ->setValue( maxZ );
}

void QgsPostprocessingEntity::setupDirectionalLight( QVector3D position, QVector3D direction )
{
  mLightPosition->setValue( QVariant::fromValue( position ) );
  mLightDirection->setValue( QVariant::fromValue( direction.normalized() ) );
}
