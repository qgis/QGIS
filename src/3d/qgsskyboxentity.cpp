/***************************************************************************
  qgsskyboxentity.cpp
  --------------------------------------
  Date                 : August 2020
  Copyright            : (C) 2020 by Belgacem Nedjima
  Email                : gb uderscore nedjima at esi dot dz
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsskyboxentity.h"

#include "qgsapplication.h"
#include "qgsimagecache.h"
#include "qgsimagetexture.h"

#include <QString>
#include <QUrl>
#include <Qt3DCore/QEntity>
#include <Qt3DExtras/QCuboidMesh>
#include <Qt3DRender/QCullFace>
#include <Qt3DRender/QDepthTest>
#include <Qt3DRender/QEffect>
#include <Qt3DRender/QFilterKey>
#include <Qt3DRender/QGraphicsApiFilter>
#include <Qt3DRender/QMaterial>
#include <Qt3DRender/QParameter>
#include <Qt3DRender/QRenderPass>
#include <Qt3DRender/QSeamlessCubemap>
#include <Qt3DRender/QShaderProgram>
#include <Qt3DRender/QTechnique>
#include <Qt3DRender/QTextureImage>

#include "moc_qgsskyboxentity.cpp"

using namespace Qt::StringLiterals;

QgsSkyboxEntity::QgsSkyboxEntity( QNode *parent )
  : Qt3DCore::QEntity( parent )
  , mEffect( new Qt3DRender::QEffect( this ) )
  , mMaterial( new Qt3DRender::QMaterial( this ) )
  , mGl3Technique( new Qt3DRender::QTechnique( this ) )
  , mFilterKey( new Qt3DRender::QFilterKey( this ) )
  , mGl3RenderPass( new Qt3DRender::QRenderPass( this ) )
  , mMesh( new Qt3DExtras::QCuboidMesh( this ) )
  , mGammaStrengthParameter( new Qt3DRender::QParameter( u"gammaStrength"_s, 0.0f ) )
  , mTextureParameter( new Qt3DRender::QParameter( this ) )
{
  mGl3Technique->graphicsApiFilter()->setApi( Qt3DRender::QGraphicsApiFilter::OpenGL );
  mGl3Technique->graphicsApiFilter()->setMajorVersion( 3 );
  mGl3Technique->graphicsApiFilter()->setMinorVersion( 3 );
  mGl3Technique->graphicsApiFilter()->setProfile( Qt3DRender::QGraphicsApiFilter::CoreProfile );

  mFilterKey->setParent( mEffect );
  mFilterKey->setName( u"renderingStyle"_s );
  mFilterKey->setValue( u"forward"_s );

  mGl3Technique->addFilterKey( mFilterKey );

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

  mMesh->setXYMeshResolution( QSize( 2, 2 ) );
  mMesh->setXZMeshResolution( QSize( 2, 2 ) );
  mMesh->setYZMeshResolution( QSize( 2, 2 ) );

  addComponent( mMesh );
  addComponent( mMaterial );
}

#if ENABLE_PANORAMIC_SKYBOX
// Panoramic skybox

QgsPanoramicSkyboxEntity::QgsPanoramicSkyboxEntity( const QString &texturePath, QNode *parent )
  : QgsSkyboxEntity( parent )
  , mTexturePath( texturePath )
  , mLoadedTexture( new Qt3DRender::QTextureLoader( parent ) )
  , mGlShader( new Qt3DRender::QShaderProgram( this ) )
{
  mLoadedTexture->setGenerateMipMaps( false );
  mGlShader->setVertexShaderCode( Qt3DRender::QShaderProgram::loadSource( QUrl( u"qrc:/shaders/skybox.vert"_s ) ) );
  mGlShader->setFragmentShaderCode( Qt3DRender::QShaderProgram::loadSource( QUrl( u"qrc:/shaders/hdr_skybox.frag"_s ) ) );
  mGl3RenderPass->setShaderProgram( mGlShader );

  mTextureParameter->setName( "skyboxTexture" );
  mTextureParameter->setValue( QVariant::fromValue( mLoadedTexture ) );

  reloadTexture();
}

void QgsPanoramicSkyboxEntity::reloadTexture()
{
  mLoadedTexture->setSource( QUrl::fromUserInput( mTexturePath ) );
}

#endif

// 6 faces skybox

QgsCubeFacesSkyboxEntity::QgsCubeFacesSkyboxEntity(
  Qgis::SkyboxCubeMapping mapping, const QString &posX, const QString &posY, const QString &posZ, const QString &negX, const QString &negY, const QString &negZ, Qt3DCore::QNode *parent
)
  : QgsSkyboxEntity( parent )
  , mMappingType( mapping )
  , mSourcePosX( posX )
  , mSourcePosY( posY )
  , mSourcePosZ( posZ )
  , mSourceNegX( negX )
  , mSourceNegY( negY )
  , mSourceNegZ( negZ )
  , mGlShader( new Qt3DRender::QShaderProgram() )
{
  init();
  reloadTexture();
}


void QgsCubeFacesSkyboxEntity::init()
{
  mGlShader->setVertexShaderCode( Qt3DRender::QShaderProgram::loadSource( QUrl( u"qrc:/shaders/skybox.vert"_s ) ) );
  mGlShader->setFragmentShaderCode( Qt3DRender::QShaderProgram::loadSource( QUrl( u"qrc:/shaders/skybox.frag"_s ) ) );
  mGl3RenderPass->setShaderProgram( mGlShader );

  mTextureParameter->setName( "skyboxTexture" );
}

void QgsCubeFacesSkyboxEntity::reloadTexture()
{
  auto *newCubeMap = new Qt3DRender::QTextureCubeMap( this );
  newCubeMap->setMagnificationFilter( Qt3DRender::QTextureCubeMap::Linear );
  newCubeMap->setMinificationFilter( Qt3DRender::QTextureCubeMap::Linear );
  newCubeMap->setGenerateMipMaps( false );
  newCubeMap->setWrapMode( Qt3DRender::QTextureWrapMode( Qt3DRender::QTextureWrapMode::ClampToEdge ) );
  newCubeMap->setFormat( Qt3DRender::QAbstractTexture::SRGB8_Alpha8 );

  // all faces must have the SAME size, so take the maximum size from the input images
  int maxSize = 0;
  for ( const QString &texturePath : { mSourcePosX, mSourcePosY, mSourcePosZ, mSourceNegX, mSourceNegY, mSourceNegZ } )
  {
    const QSize size = QgsApplication::imageCache()->originalSize( texturePath, true );
    maxSize = std::max( maxSize, std::max( size.width(), size.height() ) );
  }

  QList<Qt3DRender::QAbstractTextureImage *> newFaces;
  const QMap<Qt3DRender::QTextureCubeMap::CubeMapFace, FaceTransformation> faceConfigs = generateFaceTransformation();
  const QSize faceSize( maxSize, maxSize );
  for ( auto it = faceConfigs.begin(); it != faceConfigs.end(); ++it )
  {
    const Qt3DRender::QTextureCubeMap::CubeMapFace face = it.key();
    const FaceTransformation &config = it.value();

    bool fitsInCache = false;
    const QImage textureSourceImage = QgsApplication::imageCache()->pathAsImage( config.path, faceSize, true, 1.0, fitsInCache );
    ( void ) fitsInCache;
    QImage finalImage = textureSourceImage;
    if ( finalImage.isNull() )
    {
      finalImage = QImage( faceSize.width(), faceSize.height(), QImage::Format_RGB32 );
      finalImage.fill( Qt::white );
      QPainter p;
      p.begin( &finalImage );
      //draw a checkerboard background for missing texture images
      uchar pixDataRGB[] = { 150, 150, 150, 255, 100, 100, 100, 255, 100, 100, 100, 255, 150, 150, 150, 255 };
      const QImage img( pixDataRGB, 2, 2, 8, QImage::Format_ARGB32 );
      const QPixmap pix = QPixmap::fromImage( img.scaled( 8, 8 ) );
      QBrush checkerBrush;
      checkerBrush.setTexture( pix );
      p.fillRect( finalImage.rect(), checkerBrush );
      p.end();
    }
    else if ( config.mirrorHorizontal || config.mirrorVertical )
    {
      finalImage = finalImage.mirrored( config.mirrorHorizontal, config.mirrorVertical );
    }

    auto textureImage = new QgsImageTexture( finalImage, newCubeMap );
    textureImage->setFace( face );
    newCubeMap->addTextureImage( textureImage );
    newFaces.push_back( textureImage );
  }

  mTextureParameter->setValue( QVariant::fromValue( newCubeMap ) );

  if ( mCubeMap )
  {
    mCubeMap->deleteLater();
  }
  mCubeMap = newCubeMap;
  mFacesTextureImages = newFaces;
}

QMap<Qt3DRender::QAbstractTexture::CubeMapFace, QgsCubeFacesSkyboxEntity::FaceTransformation> QgsCubeFacesSkyboxEntity::generateFaceTransformation() const
{
  QMap<Qt3DRender::QTextureCubeMap::CubeMapFace, FaceTransformation> faceConfigs;
  switch ( mMappingType )
  {
    case Qgis::SkyboxCubeMapping::NativeZUp:
      faceConfigs[Qt3DRender::QTextureCubeMap::CubeMapPositiveX] = { mSourcePosX, false, false };
      faceConfigs[Qt3DRender::QTextureCubeMap::CubeMapNegativeX] = { mSourceNegX, false, false };
      faceConfigs[Qt3DRender::QTextureCubeMap::CubeMapPositiveY] = { mSourcePosY, false, false };
      faceConfigs[Qt3DRender::QTextureCubeMap::CubeMapNegativeY] = { mSourceNegY, false, false };
      faceConfigs[Qt3DRender::QTextureCubeMap::CubeMapPositiveZ] = { mSourcePosZ, false, false };
      faceConfigs[Qt3DRender::QTextureCubeMap::CubeMapNegativeZ] = { mSourceNegZ, false, false };
      break;

    case Qgis::SkyboxCubeMapping::OpenGLYUp:
      faceConfigs[Qt3DRender::QTextureCubeMap::CubeMapPositiveX] = { mSourcePosX, false, false };
      faceConfigs[Qt3DRender::QTextureCubeMap::CubeMapNegativeX] = { mSourceNegX, false, false };
      faceConfigs[Qt3DRender::QTextureCubeMap::CubeMapPositiveY] = { mSourceNegZ, false, false };
      faceConfigs[Qt3DRender::QTextureCubeMap::CubeMapNegativeY] = { mSourcePosZ, false, false };
      faceConfigs[Qt3DRender::QTextureCubeMap::CubeMapPositiveZ] = { mSourcePosY, false, false };
      faceConfigs[Qt3DRender::QTextureCubeMap::CubeMapNegativeZ] = { mSourceNegY, false, false };
      break;

    case Qgis::SkyboxCubeMapping::GodotYUp:
      faceConfigs[Qt3DRender::QTextureCubeMap::CubeMapPositiveX] = { mSourcePosX, false, true };
      faceConfigs[Qt3DRender::QTextureCubeMap::CubeMapNegativeX] = { mSourceNegX, false, true };
      faceConfigs[Qt3DRender::QTextureCubeMap::CubeMapPositiveY] = { mSourceNegZ, false, true };
      faceConfigs[Qt3DRender::QTextureCubeMap::CubeMapNegativeY] = { mSourcePosZ, false, true };
      faceConfigs[Qt3DRender::QTextureCubeMap::CubeMapPositiveZ] = { mSourcePosY, false, true };
      faceConfigs[Qt3DRender::QTextureCubeMap::CubeMapNegativeZ] = { mSourceNegY, false, true };
      break;

    case Qgis::SkyboxCubeMapping::UnrealEngineZUp:
      faceConfigs[Qt3DRender::QTextureCubeMap::CubeMapPositiveX] = { mSourcePosY, true, false };
      faceConfigs[Qt3DRender::QTextureCubeMap::CubeMapNegativeX] = { mSourceNegY, true, false };
      faceConfigs[Qt3DRender::QTextureCubeMap::CubeMapPositiveY] = { mSourcePosX, true, false };
      faceConfigs[Qt3DRender::QTextureCubeMap::CubeMapNegativeY] = { mSourceNegX, true, false };
      faceConfigs[Qt3DRender::QTextureCubeMap::CubeMapPositiveZ] = { mSourcePosZ, true, false };
      faceConfigs[Qt3DRender::QTextureCubeMap::CubeMapNegativeZ] = { mSourceNegZ, true, false };
      break;

    case Qgis::SkyboxCubeMapping::LeftHandedYUpMirrored:
      faceConfigs[Qt3DRender::QTextureCubeMap::CubeMapPositiveX] = { mSourcePosX, true, false };
      faceConfigs[Qt3DRender::QTextureCubeMap::CubeMapNegativeX] = { mSourceNegX, true, false };
      faceConfigs[Qt3DRender::QTextureCubeMap::CubeMapPositiveY] = { mSourcePosZ, true, false };
      faceConfigs[Qt3DRender::QTextureCubeMap::CubeMapNegativeY] = { mSourceNegZ, true, false };
      faceConfigs[Qt3DRender::QTextureCubeMap::CubeMapPositiveZ] = { mSourcePosY, true, false };
      faceConfigs[Qt3DRender::QTextureCubeMap::CubeMapNegativeZ] = { mSourceNegY, true, false };
      break;
  }

  return faceConfigs;
}
