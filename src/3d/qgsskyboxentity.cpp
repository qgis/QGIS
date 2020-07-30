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
  , mSkyboxTexture( new Qt3DRender::QTextureCubeMap( this ) )
  , mLoadedTexture( new Qt3DRender::QTextureLoader( this ) )
  , mGl3Shader( new Qt3DRender::QShaderProgram( this ) )
  , mGl3Technique( new Qt3DRender::QTechnique( this ) )
  , mFilterKey( new Qt3DRender::QFilterKey( this ) )
  , mGl3RenderPass( new Qt3DRender::QRenderPass( this ) )
  , mMesh( new Qt3DExtras::QCuboidMesh( this ) )
  , mGammaStrengthParameter( new Qt3DRender::QParameter( QStringLiteral( "gammaStrength" ), 0.0f ) )
  , mTextureParameter( new Qt3DRender::QParameter( QStringLiteral( "skyboxTexture" ), mSkyboxTexture, this ) )
  , mPosXImage( new Qt3DRender::QTextureImage( this ) )
  , mPosYImage( new Qt3DRender::QTextureImage( this ) )
  , mPosZImage( new Qt3DRender::QTextureImage( this ) )
  , mNegXImage( new Qt3DRender::QTextureImage( this ) )
  , mNegYImage( new Qt3DRender::QTextureImage( this ) )
  , mNegZImage( new Qt3DRender::QTextureImage( this ) )
  , mExtension( extension )
  , mBaseName( baseName )
  , mHasPendingReloadTextureCall( false )
{
  mLoadedTexture->setGenerateMipMaps( false );
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

  mMesh->setXExtent( 2.0f );
  mMesh->setYExtent( 2.0f );
  mMesh->setZExtent( 2.0f );
  mMesh->setXYMeshResolution( QSize( 2, 2 ) );
  mMesh->setXZMeshResolution( QSize( 2, 2 ) );
  mMesh->setYZMeshResolution( QSize( 2, 2 ) );

  Qt3DCore::QTransform *transform = new Qt3DCore::QTransform( this );
  transform->setTranslation( QVector3D( 0.0f, 0.0f, 0.0f ) );
  transform->setScale( 1.0f );
  addComponent( transform );

  mPosXImage->setFace( Qt3DRender::QTextureCubeMap::CubeMapPositiveX );
  mPosXImage->setMirrored( false );
  mPosYImage->setFace( Qt3DRender::QTextureCubeMap::CubeMapPositiveY );
  mPosYImage->setMirrored( false );
  mPosZImage->setFace( Qt3DRender::QTextureCubeMap::CubeMapPositiveZ );
  mPosZImage->setMirrored( false );
  mNegXImage->setFace( Qt3DRender::QTextureCubeMap::CubeMapNegativeX );
  mNegXImage->setMirrored( false );
  mNegYImage->setFace( Qt3DRender::QTextureCubeMap::CubeMapNegativeY );
  mNegYImage->setMirrored( false );
  mNegZImage->setFace( Qt3DRender::QTextureCubeMap::CubeMapNegativeZ );
  mNegZImage->setMirrored( false );

  mSkyboxTexture->setMagnificationFilter( Qt3DRender::QTextureCubeMap::Linear );
  mSkyboxTexture->setMinificationFilter( Qt3DRender::QTextureCubeMap::Linear );
  mSkyboxTexture->setGenerateMipMaps( false );
  mSkyboxTexture->setWrapMode( Qt3DRender::QTextureWrapMode( Qt3DRender::QTextureWrapMode::Repeat ) );

  mSkyboxTexture->addTextureImage( mPosXImage );
  mSkyboxTexture->addTextureImage( mPosYImage );
  mSkyboxTexture->addTextureImage( mPosZImage );
  mSkyboxTexture->addTextureImage( mNegXImage );
  mSkyboxTexture->addTextureImage( mNegYImage );
  mSkyboxTexture->addTextureImage( mNegZImage );

  addComponent( mMesh );
  addComponent( mMaterial );

  reloadTexture();
}

void QgsSkyboxEntity::reloadTexture()
{
  if ( !mHasPendingReloadTextureCall )
  {
    mHasPendingReloadTextureCall = true;
//        QTimer::singleShot(10, [this] {
    if ( mExtension == QStringLiteral( ".dds" ) )
    {
      mLoadedTexture->setSource( QUrl( mBaseName + mExtension ) );
      mTextureParameter->setValue( QVariant::fromValue( mLoadedTexture ) );
    }
    else
    {
      mPosXImage->setSource( QUrl( mBaseName + QStringLiteral( "_posx" ) + mExtension ) );
      mPosYImage->setSource( QUrl( mBaseName + QStringLiteral( "_posy" ) + mExtension ) );
      mPosZImage->setSource( QUrl( mBaseName + QStringLiteral( "_posz" ) + mExtension ) );
      mNegXImage->setSource( QUrl( mBaseName + QStringLiteral( "_negx" ) + mExtension ) );
      mNegYImage->setSource( QUrl( mBaseName + QStringLiteral( "_negy" ) + mExtension ) );
      mNegZImage->setSource( QUrl( mBaseName + QStringLiteral( "_negz" ) + mExtension ) );
      mTextureParameter->setValue( QVariant::fromValue( mSkyboxTexture ) );
    }
    mHasPendingReloadTextureCall = false;
//        });
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

