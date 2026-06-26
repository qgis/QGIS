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

#include "qgs3dutils.h"
#include "qgsapplication.h"
#include "qgsenvironmentlight.h"
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
#include <QtConcurrent/QtConcurrentMap>

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
  Qgis::SkyboxCubeMapping mapping,
  const QString &posX,
  const QString &posY,
  const QString &posZ,
  const QString &negX,
  const QString &negY,
  const QString &negZ,
  bool enableEnvironmentalLighting,
  Qt3DCore::QNode *parent
)
  : QgsSkyboxEntity( parent )
  , mMappingType( mapping )
  , mSourcePosX( posX )
  , mSourcePosY( posY )
  , mSourcePosZ( posZ )
  , mSourceNegX( negX )
  , mSourceNegY( negY )
  , mSourceNegZ( negZ )
  , mEnableEnvironmentalLighting( enableEnvironmentalLighting )
  , mGlShader( new Qt3DRender::QShaderProgram() )
{
  init();
  reloadTexture();
}

namespace
{
  QVector3D getCubeMapDirection( int face, float u, float v )
  {
    switch ( face )
    {
      case Qt3DRender::QTextureCubeMap::CubeMapFace::CubeMapPositiveX:
        return QVector3D( 1.0f, -v, -u );
      case Qt3DRender::QTextureCubeMap::CubeMapFace::CubeMapNegativeX:
        return QVector3D( -1.0f, -v, u );
      case Qt3DRender::QTextureCubeMap::CubeMapFace::CubeMapPositiveY:
        return QVector3D( u, 1.0f, v );
      case Qt3DRender::QTextureCubeMap::CubeMapFace::CubeMapNegativeY:
        return QVector3D( u, -1.0f, -v );
      case Qt3DRender::QTextureCubeMap::CubeMapFace::CubeMapPositiveZ:
        return QVector3D( u, -v, 1.0f );
      case Qt3DRender::QTextureCubeMap::CubeMapFace::CubeMapNegativeZ:
        return QVector3D( -u, -v, -1.0f );
      default:
        break;
    }
    return QVector3D();
  }


  struct FaceData
  {
      FaceData( Qt3DRender::QTextureCubeMap::CubeMapFace faceIndex, const QImage &image )
        : faceIndex( faceIndex )
        , image( image )
      {}
      Qt3DRender::QTextureCubeMap::CubeMapFace faceIndex;
      QImage image;
  };

  struct SHFaceResult
  {
      QVector<QVector3D> coeffs = QVector<QVector3D>( 9, QVector3D( 0.0f, 0.0f, 0.0f ) );
      float totalWeight = 0.0f;
  };

  SHFaceResult computeFaceSH( const FaceData &data )
  {
    SHFaceResult result;

    constexpr int WIDTH = 32;
    constexpr int HEIGHT = 32;

    QImage img = data.image;
    if ( img.isNull() )
      return result;

    bool isSrgb = true;
    switch ( img.format() )
    {
      case QImage::Format_RGBA32FPx4:
      case QImage::Format_RGBA32FPx4_Premultiplied:
      case QImage::Format_RGBX32FPx4:
      case QImage::Format_RGBA16FPx4:
      case QImage::Format_RGBA16FPx4_Premultiplied:
      case QImage::Format_RGBX16FPx4:
        // float based image formats won't be in sRGB color space
        isSrgb = false;
        break;
      default:
        break;
    }

    if ( img.format() != QImage::Format_RGBA32FPx4 )
    {
      // convert image to float
      img = img.convertToFormat( QImage::Format_RGBA32FPx4 );
    }

    QImage scaledImage = img.scaled( WIDTH, HEIGHT, Qt::IgnoreAspectRatio, Qt::SmoothTransformation );
    for ( int y = 0; y < HEIGHT; ++y )
    {
      const float v = ( ( static_cast< float >( y ) + 0.5f ) / static_cast< float >( HEIGHT ) ) * 2.0f - 1.0f;
      const float *line = reinterpret_cast<const float *>( scaledImage.constScanLine( y ) );
      for ( int x = 0; x < WIDTH; ++x )
      {
        // map pixel coordinate to [-1, 1] range
        const float u = ( ( static_cast< float >( x ) + 0.5f ) / static_cast< float >( WIDTH ) ) * 2.0f - 1.0f;
        const QVector3D cubemapDir = getCubeMapDirection( data.faceIndex, u, v ).normalized();
        const QVector3D dir( cubemapDir.x(), -cubemapDir.z(), cubemapDir.y() );

        // solid angle weighting (pixels near the corners of the cube appear smaller)
        float weight = 1.0f / std::pow( 1.0f + u * u + v * v, 1.5f );
        result.totalWeight += weight;

        QColor color = QColor::fromRgbF( line[x * 4 + 0], line[x * 4 + 1], line[x * 4 + 2], 1 );
        if ( isSrgb )
        {
          color = Qgs3DUtils::srgbToLinear( color );
        }
        const QVector3D weightedColor( color.redF() * weight, color.greenF() * weight, color.blueF() * weight );

        constexpr float Y00 = 0.282095f;
        constexpr float Y11 = 0.488603f;
        constexpr float Y22 = 1.092548f;
        constexpr float Y20 = 0.315392f;
        constexpr float Y22_2 = 0.546274f;

        result.coeffs[0] += weightedColor * Y00;
        result.coeffs[1] += weightedColor * ( Y11 * dir.y() );
        result.coeffs[2] += weightedColor * ( Y11 * dir.z() );
        result.coeffs[3] += weightedColor * ( Y11 * dir.x() );
        result.coeffs[4] += weightedColor * ( Y22 * dir.x() * dir.y() );
        result.coeffs[5] += weightedColor * ( Y22 * dir.y() * dir.z() );
        result.coeffs[6] += weightedColor * ( Y20 * ( 3.0f * dir.z() * dir.z() - 1.0f ) );
        result.coeffs[7] += weightedColor * ( Y22 * dir.x() * dir.z() );
        result.coeffs[8] += weightedColor * ( Y22_2 * ( dir.x() * dir.x() - dir.y() * dir.y() ) );
      }
    }

    return result;
  }

  void reduceSH( SHFaceResult &finalResult, const SHFaceResult &partialResult )
  {
    for ( int i = 0; i < 9; ++i )
    {
      finalResult.coeffs[i] += partialResult.coeffs[i];
    }
    finalResult.totalWeight += partialResult.totalWeight;
  }

  QVector<QVector3D> computeSphericalHarmonics( const QList<FaceData> &faceDataList )
  {
    // this isn't too expensive to calculate, but it's also TRIVIAL to calculate in parallel and WILL
    // be done on the main thread, so let's do that...
    QFuture<SHFaceResult> future = QtConcurrent::mappedReduced( faceDataList, computeFaceSH, reduceSH );
    future.waitForFinished();
    SHFaceResult combinedResult = future.result();

    if ( combinedResult.totalWeight > 0 )
    {
      const float norm = static_cast< float >( 4.0 / combinedResult.totalWeight );
      for ( int i = 0; i < 9; ++i )
      {
        combinedResult.coeffs[i] *= norm;
      }
    }

    return combinedResult.coeffs;
  }

} //namespace
void QgsCubeFacesSkyboxEntity::updateEnvironmentLight( QgsEnvironmentLight *envLight ) const
{
  if ( !mEnableEnvironmentalLighting )
  {
    envLight->setMode( QgsEnvironmentLight::Mode::Disabled );
    return;
  }

  auto *lightCubeMap = new Qt3DRender::QTextureCubeMap( envLight );
  lightCubeMap->setMagnificationFilter( Qt3DRender::QTextureCubeMap::Linear );
  lightCubeMap->setMinificationFilter( Qt3DRender::QTextureCubeMap::LinearMipMapLinear );
  lightCubeMap->setGenerateMipMaps( true );
  lightCubeMap->setWrapMode( Qt3DRender::QTextureWrapMode( Qt3DRender::QTextureWrapMode::ClampToEdge ) );

  int maxSize = 0;
  for ( const QString &texturePath : { mSourcePosX, mSourcePosY, mSourcePosZ, mSourceNegX, mSourceNegY, mSourceNegZ } )
  {
    const QSize size = QgsApplication::imageCache()->originalSize( texturePath, true );
    maxSize = std::max( maxSize, std::max( size.width(), size.height() ) );
  }

  const QMap<Qt3DRender::QTextureCubeMap::CubeMapFace, FaceTransformation> faceConfigs = generateFaceTransformation();
  const QSize faceSize( maxSize, maxSize );

  QList<FaceData> faceDataList;
  for ( auto it = faceConfigs.begin(); it != faceConfigs.end(); ++it )
  {
    const Qt3DRender::QTextureCubeMap::CubeMapFace face = it.key();
    const FaceTransformation &config = it.value();

    bool fitsInCache = false;
    // TODO -- this force converts to Format_ARGB32_Premultiplied -- we need a flag to avoid that, as potentially these are HDR images...
    const QImage textureSourceImage = QgsApplication::imageCache()->pathAsImage( config.path, faceSize, true, 1.0, fitsInCache );
    ( void ) fitsInCache;
    QImage finalImage = textureSourceImage;
    if ( finalImage.isNull() )
    {
      finalImage = QImage( faceSize.width(), faceSize.height(), QImage::Format_RGB32 );
      finalImage.fill( Qt::white );
    }
    else if ( config.mirrorHorizontal || config.mirrorVertical )
    {
      finalImage = finalImage.mirrored( config.mirrorHorizontal, config.mirrorVertical );
    }

    bool requiresConversionToRgb = false;
    Qt3DRender::QAbstractTexture::TextureFormat textureFormat = Qgs3DUtils::determineTextureFormat( finalImage.format(), true, requiresConversionToRgb );
    lightCubeMap->setFormat( textureFormat );
    if ( requiresConversionToRgb )
    {
      finalImage.convertTo( QImage::Format::Format_ARGB32_Premultiplied );
    }

    auto textureImage = new QgsImageTexture( finalImage, lightCubeMap );
    textureImage->setFace( face );
    lightCubeMap->addTextureImage( textureImage );

    if ( !textureSourceImage.isNull() )
    {
      faceDataList.append( FaceData( face, finalImage ) );
    }
  }

  const QVector<QVector3D> shCoeffs = faceDataList.isEmpty() ? QVector<QVector3D>( 9, QVector3D( 0, 0, 0 ) ) : computeSphericalHarmonics( faceDataList );

  int mipLevels = 1;
  if ( maxSize > 0 )
  {
    mipLevels = static_cast<int>( std::floor( std::log2( maxSize ) ) ) + 1;
  }

  envLight->setSpecularMap( lightCubeMap, mipLevels );
  envLight->setSphericalHarmonics( shCoeffs );
  envLight->setMode( QgsEnvironmentLight::Mode::SpecularMapWithSphericalHarmonics );
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

    bool requiresConversionToRgb = false;
    Qt3DRender::QAbstractTexture::TextureFormat textureFormat = Qgs3DUtils::determineTextureFormat( finalImage.format(), true, requiresConversionToRgb );
    newCubeMap->setFormat( textureFormat );
    if ( requiresConversionToRgb )
    {
      finalImage.convertTo( QImage::Format::Format_ARGB32_Premultiplied );
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
