#include "qgsskyboxentity.h"

#include <Qt3DCore/QEntity>
#include <QVector3D>
#include <Qt3DRender/QTexture>
#include <Qt3DExtras/QCuboidMesh>
#include <Qt3DRender/QEffect>
#include <Qt3DRender/QMaterial>
#include <Qt3DRender/QShaderProgram>
#include <Qt3DRender/QFilterKey>
#include <Qt3DRender/QRenderPass>
#include <Qt3DRender/QTechnique>
#include <Qt3DRender/QGraphicsApiFilter>
#include <Qt3DRender/QCullFace>
#include <Qt3DRender/QDepthTest>
#include <Qt3DRender/QSeamlessCubemap>
#include <QTimer>
#include <Qt3DRender/QParameter>
#include <QSurfaceFormat>
#include <Qt3DExtras/QPlaneMesh>
#include <Qt3DExtras/QDiffuseSpecularMaterial>
#include <Qt3DCore/QTransform>
#include <Qt3DRender/QGeometry>
#include <Qt3DRender/QAttribute>

QgsSkyboxEntity::QgsSkyboxEntity( const QString &baseName, const QString &extension, QNode *parent )
  : Qt3DCore::QEntity( parent )
  , mEffect( new Qt3DRender::QEffect( this ) )
  , mMaterial( new Qt3DRender::QMaterial( this ) )
  , mGl3Shader( new Qt3DRender::QShaderProgram( this ) )
  , mGl3Technique( new Qt3DRender::QTechnique( this ) )
  , mFilterKey( new Qt3DRender::QFilterKey( this ) )
  , mGl3RenderPass( new Qt3DRender::QRenderPass( this ) )
  , mMesh( new Qt3DExtras::QCuboidMesh( this ) )
  , mGammaStrengthParameter( new Qt3DRender::QParameter( QStringLiteral( "gammaStrength" ), 0.0f ) )
  , mTextureParameter( new Qt3DRender::QParameter( this ) )
  , mExtension( extension )
  , mBaseName( baseName )
  , mHDRTexturePath( QString() )
  , mIsUsingHDR( false )
{
  mGl3Shader->setVertexShaderCode( Qt3DRender::QShaderProgram::loadSource( QUrl( QStringLiteral( "qrc:/shaders/skybox.vert" ) ) ) );
  mGl3Shader->setFragmentShaderCode( Qt3DRender::QShaderProgram::loadSource( QUrl( QStringLiteral( "qrc:/shaders/skybox.frag" ) ) ) );

  mGl3Technique->graphicsApiFilter()->setApi( Qt3DRender::QGraphicsApiFilter::OpenGL );
  mGl3Technique->graphicsApiFilter()->setMajorVersion( 3 );
  mGl3Technique->graphicsApiFilter()->setMinorVersion( 3 );
  mGl3Technique->graphicsApiFilter()->setProfile( Qt3DRender::QGraphicsApiFilter::CoreProfile );

  mFilterKey->setParent( mEffect );
  mFilterKey->setName( QStringLiteral( "renderingStyle" ) );
  mFilterKey->setValue( QStringLiteral( "forward" ) );

  mGl3Technique->addFilterKey( mFilterKey );

  mGl3RenderPass->setShaderProgram( mGl3Shader );

  Qt3DRender::QCullFace *cullFront = new Qt3DRender::QCullFace();
  cullFront->setMode( Qt3DRender::QCullFace::Front );
  Qt3DRender::QDepthTest *depthTest = new Qt3DRender::QDepthTest();
  depthTest->setDepthFunction( Qt3DRender::QDepthTest::LessOrEqual );
  Qt3DRender::QSeamlessCubemap *seamlessCubemap = new Qt3DRender::QSeamlessCubemap();

  mGl3RenderPass->addRenderState( cullFront );
  mGl3RenderPass->addRenderState( depthTest );
  mGl3RenderPass->addRenderState( seamlessCubemap );

  mGl3Technique->addRenderPass( mGl3RenderPass );

  mEffect->addTechnique( mGl3Technique );

  mMaterial->setEffect( mEffect );
  mMaterial->addParameter( mGammaStrengthParameter );
  mMaterial->addParameter( mTextureParameter );

//  mMesh->setXExtent( 2.0f );
//  mMesh->setYExtent( 2.0f );
//  mMesh->setZExtent( 2.0f );
  mMesh->setXYMeshResolution( QSize( 2, 2 ) );
  mMesh->setXZMeshResolution( QSize( 2, 2 ) );
  mMesh->setYZMeshResolution( QSize( 2, 2 ) );

//   TODO: change the kybox position according to
  Qt3DCore::QTransform *transform = new Qt3DCore::QTransform( this );
  transform->setTranslation( QVector3D( 0.0f, 0.0f, 0.0f ) );
  transform->setScale( 1000.0f );
  addComponent( transform );

  addComponent( mMesh );
  addComponent( mMaterial );

  reloadTexture();
}

QgsSkyboxEntity::QgsSkyboxEntity( const QString &hdrTexturePath, Qt3DCore::QNode *parent )
  : Qt3DCore::QEntity( parent )
  , mEffect( new Qt3DRender::QEffect( this ) )
  , mMaterial( new Qt3DRender::QMaterial( this ) )
  , mGl3Shader( new Qt3DRender::QShaderProgram( this ) )
  , mGl3Technique( new Qt3DRender::QTechnique( this ) )
  , mFilterKey( new Qt3DRender::QFilterKey( this ) )
  , mGl3RenderPass( new Qt3DRender::QRenderPass( this ) )
  , mMesh( new Qt3DExtras::QCuboidMesh( this ) )
  , mGammaStrengthParameter( new Qt3DRender::QParameter( QStringLiteral( "gammaStrength" ), 0.0f ) )
  , mTextureParameter( new Qt3DRender::QParameter( this ) )
  , mHDRTexturePath( hdrTexturePath )
  , mIsUsingHDR( true )
{
  mGl3Shader->setVertexShaderCode( Qt3DRender::QShaderProgram::loadSource( QUrl( QStringLiteral( "qrc:/shaders/hdr_skybox.vert" ) ) ) );
  mGl3Shader->setFragmentShaderCode( Qt3DRender::QShaderProgram::loadSource( QUrl( QStringLiteral( "qrc:/shaders/hdr_skybox.frag" ) ) ) );

  mGl3Technique->graphicsApiFilter()->setApi( Qt3DRender::QGraphicsApiFilter::OpenGL );
  mGl3Technique->graphicsApiFilter()->setMajorVersion( 3 );
  mGl3Technique->graphicsApiFilter()->setMinorVersion( 3 );
  mGl3Technique->graphicsApiFilter()->setProfile( Qt3DRender::QGraphicsApiFilter::CoreProfile );

  mFilterKey->setParent( mEffect );
  mFilterKey->setName( QStringLiteral( "renderingStyle" ) );
  mFilterKey->setValue( QStringLiteral( "forward" ) );

  mGl3Technique->addFilterKey( mFilterKey );

  mGl3RenderPass->setShaderProgram( mGl3Shader );

  Qt3DRender::QCullFace *cullFront = new Qt3DRender::QCullFace();
  cullFront->setMode( Qt3DRender::QCullFace::Front );
  Qt3DRender::QDepthTest *depthTest = new Qt3DRender::QDepthTest();
  depthTest->setDepthFunction( Qt3DRender::QDepthTest::LessOrEqual );
  Qt3DRender::QSeamlessCubemap *seamlessCubemap = new Qt3DRender::QSeamlessCubemap();

  mGl3RenderPass->addRenderState( cullFront );
  mGl3RenderPass->addRenderState( depthTest );
  mGl3RenderPass->addRenderState( seamlessCubemap );

  mGl3Technique->addRenderPass( mGl3RenderPass );

  mEffect->addTechnique( mGl3Technique );

  mMaterial->setEffect( mEffect );
  mMaterial->addParameter( mGammaStrengthParameter );
  mMaterial->addParameter( mTextureParameter );

//  mMesh->setXExtent( 2.0f );
//  mMesh->setYExtent( 2.0f );
//  mMesh->setZExtent( 2.0f );
  mMesh->setXYMeshResolution( QSize( 2, 2 ) );
  mMesh->setXZMeshResolution( QSize( 2, 2 ) );
  mMesh->setYZMeshResolution( QSize( 2, 2 ) );

//   TODO: change the kybox position according to
  Qt3DCore::QTransform *transform = new Qt3DCore::QTransform( this );
  transform->setTranslation( QVector3D( 0.0f, 0.0f, 0.0f ) );
  transform->setScale( 1000.0f );
  addComponent( transform );

  addComponent( mMesh );
  addComponent( mMaterial );

  reloadTexture();
}

void QgsSkyboxEntity::reloadTexture()
{
  if ( mSkyboxTextureLoader != nullptr )
    delete mSkyboxTextureLoader;
  if ( mIsUsingHDR )
  {
    mSkyboxTextureLoader = new QgsHDRSkyboxTextureLoader( mHDRTexturePath, this );
    mTextureParameter->setName( QStringLiteral( "skyboxTexture" ) );
    mTextureParameter->setValue( mSkyboxTextureLoader->getTextureParameter() );
  }
  else if ( mExtension == QStringLiteral( ".dds" ) )
  {
    mSkyboxTextureLoader = new QgsDDSSkyboxLoader( mBaseName, mExtension, this );
    mTextureParameter->setName( QStringLiteral( "skyboxTexture" ) );
    mTextureParameter->setValue( mSkyboxTextureLoader->getTextureParameter() );
  }
  else
  {
    mSkyboxTextureLoader = new QgsSkyboxTextureColloectionLoader( mBaseName, mExtension, this );
    mTextureParameter->setName( QStringLiteral( "skyboxTexture" ) );
    mTextureParameter->setValue( mSkyboxTextureLoader->getTextureParameter() );
  }
}

void QgsSkyboxEntity::setBaseName( const QString &baseName )
{
  if ( baseName != mBaseName )
  {
    mBaseName = baseName;
    emit baseNameChanged( baseName );
    reloadTexture();
  }
}

void QgsSkyboxEntity::setExtension( const QString &extension )
{
  if ( extension != mExtension )
  {
    mExtension = extension;
    emit extensionChanged( extension );
    reloadTexture();
  }
}

void QgsSkyboxEntity::setGammaCorrectEnabled( bool enabled )
{
  if ( enabled != isGammaCorrectEnabled() )
  {
    mGammaStrengthParameter->setValue( enabled ? 1.0f : 0.0f );
    emit gammaCorrectEnabledChanged( enabled );
  }
}

