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

#include <Qt3DCore/QEntity>
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
#include <Qt3DRender/QParameter>
#include <Qt3DRender/QTextureImage>
#include <QUrl>

#include "qgsimagecache.h"
#include "qgsimagetexture.h"
#include "qgsproject.h"

QgsSkyboxEntity::QgsSkyboxEntity( QNode *parent )
  : Qt3DCore::QEntity( parent )
  , mEffect( new Qt3DRender::QEffect( this ) )
  , mMaterial( new Qt3DRender::QMaterial( this ) )
  , mGl3Technique( new Qt3DRender::QTechnique( this ) )
  , mFilterKey( new Qt3DRender::QFilterKey( this ) )
  , mGl3RenderPass( new Qt3DRender::QRenderPass( this ) )
  , mMesh( new Qt3DExtras::QCuboidMesh( this ) )
  , mGammaStrengthParameter( new Qt3DRender::QParameter( QStringLiteral( "gammaStrength" ), 0.0f ) )
  , mTextureParameter( new Qt3DRender::QParameter( this ) )
{
  mGl3Technique->graphicsApiFilter()->setApi( Qt3DRender::QGraphicsApiFilter::OpenGL );
  mGl3Technique->graphicsApiFilter()->setMajorVersion( 3 );
  mGl3Technique->graphicsApiFilter()->setMinorVersion( 3 );
  mGl3Technique->graphicsApiFilter()->setProfile( Qt3DRender::QGraphicsApiFilter::CoreProfile );

  mFilterKey->setParent( mEffect );
  mFilterKey->setName( QStringLiteral( "renderingStyle" ) );
  mFilterKey->setValue( QStringLiteral( "forward" ) );

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

// Panoramic skybox

QgsPanoramicSkyboxEntity::QgsPanoramicSkyboxEntity( const QString &texturePath, QNode *parent )
  : QgsSkyboxEntity( parent )
  , mTexturePath( texturePath )
  , mLoadedTexture( new Qt3DRender::QTextureLoader( parent ) )
  , mGlShader( new Qt3DRender::QShaderProgram( this ) )
{
  mLoadedTexture->setGenerateMipMaps( false );
  mGlShader->setVertexShaderCode( Qt3DRender::QShaderProgram::loadSource( QUrl( QStringLiteral( "qrc:/shaders/skybox.vert" ) ) ) );
  mGlShader->setFragmentShaderCode( Qt3DRender::QShaderProgram::loadSource( QUrl( QStringLiteral( "qrc:/shaders/hdr_skybox.frag" ) ) ) );
  mGl3RenderPass->setShaderProgram( mGlShader );

  mTextureParameter->setName( "skyboxTexture" );
  mTextureParameter->setValue( QVariant::fromValue( mLoadedTexture ) );

  reloadTexture();
}

void QgsPanoramicSkyboxEntity::reloadTexture()
{
  mLoadedTexture->setSource( QUrl::fromUserInput( mTexturePath ) );
}

// 6 faces skybox

QgsCubeFacesSkyboxEntity::QgsCubeFacesSkyboxEntity( const QString &posX, const QString &posY, const QString &posZ, const QString &negX, const QString &negY, const QString &negZ, Qt3DCore::QNode *parent )
  : QgsSkyboxEntity( parent )
  , mGlShader( new Qt3DRender::QShaderProgram() )
  , mCubeMap( new Qt3DRender::QTextureCubeMap( this ) )
{
  init();
  mCubeFacesPaths[Qt3DRender::QTextureCubeMap::CubeMapPositiveX] = posX;
  mCubeFacesPaths[Qt3DRender::QTextureCubeMap::CubeMapPositiveY] = posY;
  mCubeFacesPaths[Qt3DRender::QTextureCubeMap::CubeMapPositiveZ] = posZ;
  mCubeFacesPaths[Qt3DRender::QTextureCubeMap::CubeMapNegativeX] = negX;
  mCubeFacesPaths[Qt3DRender::QTextureCubeMap::CubeMapNegativeY] = negY;
  mCubeFacesPaths[Qt3DRender::QTextureCubeMap::CubeMapNegativeZ] = negZ;
  reloadTexture();
}

void QgsCubeFacesSkyboxEntity::init()
{
  mGlShader->setVertexShaderCode( Qt3DRender::QShaderProgram::loadSource( QUrl( QStringLiteral( "qrc:/shaders/skybox.vert" ) ) ) );
  mGlShader->setFragmentShaderCode( Qt3DRender::QShaderProgram::loadSource( QUrl( QStringLiteral( "qrc:/shaders/skybox.frag" ) ) ) );
  mGl3RenderPass->setShaderProgram( mGlShader );

  mCubeMap->setMagnificationFilter( Qt3DRender::QTextureCubeMap::Linear );
  mCubeMap->setMinificationFilter( Qt3DRender::QTextureCubeMap::Linear );
  mCubeMap->setGenerateMipMaps( false );
  mCubeMap->setWrapMode( Qt3DRender::QTextureWrapMode( Qt3DRender::QTextureWrapMode::Repeat ) );

  mCubeFacesPaths[Qt3DRender::QTextureCubeMap::CubeMapPositiveX] = QString();
  mCubeFacesPaths[Qt3DRender::QTextureCubeMap::CubeMapPositiveY] = QString();
  mCubeFacesPaths[Qt3DRender::QTextureCubeMap::CubeMapPositiveZ] = QString();
  mCubeFacesPaths[Qt3DRender::QTextureCubeMap::CubeMapNegativeX] = QString();
  mCubeFacesPaths[Qt3DRender::QTextureCubeMap::CubeMapNegativeY] = QString();
  mCubeFacesPaths[Qt3DRender::QTextureCubeMap::CubeMapNegativeZ] = QString();

  mTextureParameter->setName( "skyboxTexture" );
  mTextureParameter->setValue( QVariant::fromValue( mCubeMap ) );
}

void QgsCubeFacesSkyboxEntity::reloadTexture()
{
  for ( Qt3DRender::QAbstractTextureImage *textureImage : mFacesTextureImages )
  {
    mCubeMap->removeTextureImage( textureImage );
    delete textureImage;
  }
  mFacesTextureImages.clear();

  for ( auto it = mCubeFacesPaths.begin(); it != mCubeFacesPaths.end(); ++it )
  {
    const Qt3DRender::QTextureCubeMap::CubeMapFace face = it.key();
    const QString texturePath = it.value();
    Qt3DRender::QTextureImage *image = new Qt3DRender::QTextureImage( this );
    image->setFace( face );
    image->setMirrored( false );
    image->setSource( QUrl::fromUserInput( texturePath ) );
    mCubeMap->addTextureImage( image );
    mFacesTextureImages.push_back( image );
  }
}
