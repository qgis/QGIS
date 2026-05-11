/***************************************************************************
  qgsmetalroughmaterial.cpp
  --------------------------------------
  Date                 : December 2023
  Copyright            : (C) 2023 by Nyall Dawson
  Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsmetalroughmaterial.h"

#include "qgs3dutils.h"

#include <QString>
#include <Qt3DRender/QAbstractTexture>
#include <Qt3DRender/QGraphicsApiFilter>
#include <Qt3DRender/QParameter>
#include <Qt3DRender/QRenderPass>
#include <Qt3DRender/QShaderProgramBuilder>
#include <Qt3DRender/QTechnique>
#include <Qt3DRender/QTexture>

#include "moc_qgsmetalroughmaterial.cpp"

using namespace Qt::StringLiterals;

///@cond PRIVATE
QgsMetalRoughMaterial::QgsMetalRoughMaterial( QNode *parent )
  : QgsMaterial( parent )
  , mBaseColorParameter( new Qt3DRender::QParameter( u"baseColor"_s, Qgs3DUtils::srgbToLinear( QColor( "grey" ) ), this ) )
  , mMetalnessParameter( new Qt3DRender::QParameter( u"metalness"_s, 0.0f, this ) )
  , mRoughnessParameter( new Qt3DRender::QParameter( u"roughness"_s, 0.0f, this ) )
  , mBaseColorMapParameter( new Qt3DRender::QParameter( u"baseColorMap"_s, QVariant(), this ) )
  , mMetalnessMapParameter( new Qt3DRender::QParameter( u"metalnessMap"_s, QVariant(), this ) )
  , mRoughnessMapParameter( new Qt3DRender::QParameter( u"roughnessMap"_s, QVariant(), this ) )
  , mAmbientOcclusionMapParameter( new Qt3DRender::QParameter( u"ambientOcclusionMap"_s, QVariant(), this ) )
  , mNormalMapParameter( new Qt3DRender::QParameter( u"normalMap"_s, QVariant(), this ) )
  , mHeightMapParameter( new Qt3DRender::QParameter( u"heightMap"_s, QVariant(), this ) )
  , mParallaxScaleParameter( new Qt3DRender::QParameter( u"parallaxScale"_s, 0.1f, this ) )
  , mEmissionMapParameter( new Qt3DRender::QParameter( u"emissionMap"_s, QVariant(), this ) )
  , mEmissionFactorParameter( new Qt3DRender::QParameter( u"emissiveFactor"_s, 1.0f, this ) )
  , mTextureScaleParameter( new Qt3DRender::QParameter( u"texCoordScale"_s, 1.0f, this ) )
  , mTextureRotationParameter( new Qt3DRender::QParameter( u"texCoordRotation"_s, 0.0f, this ) )
  , mOpacityParameter( new Qt3DRender::QParameter( u"opacity"_s, 1.0f ) )
  , mMetalRoughEffect( new Qt3DRender::QEffect( this ) )
  , mMetalRoughGL3Technique( new Qt3DRender::QTechnique( this ) )
  , mMetalRoughGL3RenderPass( new Qt3DRender::QRenderPass( this ) )
  , mMetalRoughGL3Shader( new Qt3DRender::QShaderProgram( this ) )
  , mFilterKey( new Qt3DRender::QFilterKey( this ) )
{
  init();
}

QgsMetalRoughMaterial::~QgsMetalRoughMaterial() = default;

void QgsMetalRoughMaterial::setBaseColor( const QColor &baseColor )
{
  mBaseColorParameter->setValue( Qgs3DUtils::srgbToLinear( baseColor ) );
  bool oldUsingBaseColorMap = mUsingBaseColorMap;

  mUsingBaseColorMap = false;
  if ( mMetalRoughEffect->parameters().contains( mBaseColorMapParameter ) )
    mMetalRoughEffect->removeParameter( mBaseColorMapParameter );
  mMetalRoughEffect->addParameter( mBaseColorParameter );

  if ( oldUsingBaseColorMap != mUsingBaseColorMap )
    updateFragmentShader();
}

void QgsMetalRoughMaterial::setBaseColorTexture( Qt3DRender::QAbstractTexture *baseColor )
{
  mBaseColorMapParameter->setValue( QVariant::fromValue( baseColor ) );
  bool oldUsingBaseColorMap = mUsingBaseColorMap;

  mUsingBaseColorMap = true;
  mMetalRoughEffect->addParameter( mBaseColorMapParameter );
  if ( mMetalRoughEffect->parameters().contains( mBaseColorParameter ) )
    mMetalRoughEffect->removeParameter( mBaseColorParameter );

  if ( oldUsingBaseColorMap != mUsingBaseColorMap )
    updateFragmentShader();
}

void QgsMetalRoughMaterial::setMetalness( float metalness )
{
  mMetalnessParameter->setValue( metalness );
  bool oldUsingMetalnessMap = mUsingMetalnessMap;

  mUsingMetalnessMap = false;
  if ( mMetalRoughEffect->parameters().contains( mMetalnessMapParameter ) )
    mMetalRoughEffect->removeParameter( mMetalnessMapParameter );
  mMetalRoughEffect->addParameter( mMetalnessParameter );

  if ( oldUsingMetalnessMap != mUsingMetalnessMap )
    updateFragmentShader();
}

void QgsMetalRoughMaterial::setMetalnessTexture( Qt3DRender::QAbstractTexture *metalness )
{
  mMetalnessMapParameter->setValue( QVariant::fromValue( metalness ) );
  bool oldUsingMetalnessMap = mUsingMetalnessMap;

  mUsingMetalnessMap = true;
  mMetalRoughEffect->addParameter( mMetalnessMapParameter );
  if ( mMetalRoughEffect->parameters().contains( mMetalnessParameter ) )
    mMetalRoughEffect->removeParameter( mMetalnessParameter );

  if ( oldUsingMetalnessMap != mUsingMetalnessMap )
    updateFragmentShader();
}

void QgsMetalRoughMaterial::setRoughness( float roughness )
{
  mRoughnessParameter->setValue( roughness );
  bool oldUsingRoughnessMap = mUsingRoughnessMap;

  mUsingRoughnessMap = false;
  if ( mMetalRoughEffect->parameters().contains( mRoughnessMapParameter ) )
    mMetalRoughEffect->removeParameter( mRoughnessMapParameter );
  mMetalRoughEffect->addParameter( mRoughnessParameter );

  if ( oldUsingRoughnessMap != mUsingRoughnessMap )
    updateFragmentShader();
}

void QgsMetalRoughMaterial::setRoughnessTexture( Qt3DRender::QAbstractTexture *roughness )
{
  mRoughnessMapParameter->setValue( QVariant::fromValue( roughness ) );
  bool oldUsingRoughnessMap = mUsingRoughnessMap;

  mUsingRoughnessMap = true;
  mMetalRoughEffect->addParameter( mRoughnessMapParameter );
  if ( mMetalRoughEffect->parameters().contains( mRoughnessParameter ) )
    mMetalRoughEffect->removeParameter( mRoughnessParameter );

  if ( oldUsingRoughnessMap != mUsingRoughnessMap )
    updateFragmentShader();
}

void QgsMetalRoughMaterial::setAmbientOcclusionTexture( Qt3DRender::QAbstractTexture *ambientOcclusion )
{
  bool oldUsingAmbientOcclusionMap = mUsingAmbientOcclusionMap;

  if ( ambientOcclusion )
  {
    mAmbientOcclusionMapParameter->setValue( QVariant::fromValue( ambientOcclusion ) );
    mUsingAmbientOcclusionMap = true;
    mMetalRoughEffect->addParameter( mAmbientOcclusionMapParameter );
  }
  else
  {
    mAmbientOcclusionMapParameter->setValue( QVariant() );
    mUsingAmbientOcclusionMap = false;
    if ( mMetalRoughEffect->parameters().contains( mAmbientOcclusionMapParameter ) )
      mMetalRoughEffect->removeParameter( mAmbientOcclusionMapParameter );
  }

  if ( oldUsingAmbientOcclusionMap != mUsingAmbientOcclusionMap )
    updateFragmentShader();
}

void QgsMetalRoughMaterial::setNormalTexture( Qt3DRender::QAbstractTexture *normal )
{
  bool oldUsingNormalMap = mUsingNormalMap;

  if ( normal )
  {
    mNormalMapParameter->setValue( QVariant::fromValue( normal ) );
    mUsingNormalMap = true;
    mMetalRoughEffect->addParameter( mNormalMapParameter );
  }
  else
  {
    mNormalMapParameter->setValue( QVariant() );
    mUsingNormalMap = false;
    if ( mMetalRoughEffect->parameters().contains( mNormalMapParameter ) )
      mMetalRoughEffect->removeParameter( mNormalMapParameter );
  }

  if ( oldUsingNormalMap != mUsingNormalMap )
    updateFragmentShader();
}

void QgsMetalRoughMaterial::setHeightTexture( Qt3DRender::QAbstractTexture *height )
{
  bool oldUsingHeightMap = mUsingHeightMap;

  if ( height )
  {
    mHeightMapParameter->setValue( QVariant::fromValue( height ) );
    mUsingHeightMap = true;
    mMetalRoughEffect->addParameter( mHeightMapParameter );
  }
  else
  {
    mHeightMapParameter->setValue( QVariant() );
    mUsingHeightMap = false;
    if ( mMetalRoughEffect->parameters().contains( mHeightMapParameter ) )
      mMetalRoughEffect->removeParameter( mHeightMapParameter );
  }

  if ( oldUsingHeightMap != mUsingHeightMap )
    updateFragmentShader();
}

void QgsMetalRoughMaterial::setParallaxScale( double scale )
{
  mParallaxScaleParameter->setValue( scale );
}

void QgsMetalRoughMaterial::setEmissionTexture( Qt3DRender::QAbstractTexture *emission )
{
  bool oldUsingEmissionMap = mUsingEmissionMap;

  if ( emission )
  {
    mEmissionMapParameter->setValue( QVariant::fromValue( emission ) );
    mUsingEmissionMap = true;
    mMetalRoughEffect->addParameter( mEmissionMapParameter );
  }
  else
  {
    mEmissionMapParameter->setValue( QVariant() );
    mUsingEmissionMap = false;
    if ( mMetalRoughEffect->parameters().contains( mEmissionMapParameter ) )
      mMetalRoughEffect->removeParameter( mEmissionMapParameter );
  }

  if ( oldUsingEmissionMap != mUsingEmissionMap )
    updateFragmentShader();
}

void QgsMetalRoughMaterial::setEmissionFactor( double factor )
{
  mEmissionFactorParameter->setValue( factor );
}

void QgsMetalRoughMaterial::setTextureScale( float textureScale )
{
  mTextureScaleParameter->setValue( textureScale );
}

void QgsMetalRoughMaterial::setTextureRotation( float textureRotation )
{
  mTextureRotationParameter->setValue( textureRotation );
}

void QgsMetalRoughMaterial::init()
{
  const QByteArray vertexShaderCode = Qt3DRender::QShaderProgram::loadSource( QUrl( u"qrc:/shaders/default.vert"_s ) );
  const QByteArray finalVertexShaderCode = Qgs3DUtils::addDefinesToShaderCode( vertexShaderCode, QStringList( { "TEXTURE_ROTATION" } ) );
  mMetalRoughGL3Shader->setVertexShaderCode( finalVertexShaderCode );

  updateFragmentShader();

  mMetalRoughGL3Technique->graphicsApiFilter()->setApi( Qt3DRender::QGraphicsApiFilter::OpenGL );
  mMetalRoughGL3Technique->graphicsApiFilter()->setMajorVersion( 3 );
  mMetalRoughGL3Technique->graphicsApiFilter()->setMinorVersion( 1 );
  mMetalRoughGL3Technique->graphicsApiFilter()->setProfile( Qt3DRender::QGraphicsApiFilter::CoreProfile );

  mFilterKey->setParent( this );
  mFilterKey->setName( u"renderingStyle"_s );
  mFilterKey->setValue( u"forward"_s );

  mMetalRoughGL3Technique->addFilterKey( mFilterKey );
  mMetalRoughGL3RenderPass->setShaderProgram( mMetalRoughGL3Shader );
  mMetalRoughGL3Technique->addRenderPass( mMetalRoughGL3RenderPass );
  mMetalRoughEffect->addTechnique( mMetalRoughGL3Technique );

  // Given parameters a parent
  mBaseColorMapParameter->setParent( mMetalRoughEffect );
  mMetalnessMapParameter->setParent( mMetalRoughEffect );
  mRoughnessMapParameter->setParent( mMetalRoughEffect );
  mNormalMapParameter->setParent( mMetalRoughEffect );
  mHeightMapParameter->setParent( mMetalRoughEffect );
  mAmbientOcclusionMapParameter->setParent( mMetalRoughEffect );
  mEmissionMapParameter->setParent( mMetalRoughEffect );

  mMetalRoughEffect->addParameter( mBaseColorParameter );
  mMetalRoughEffect->addParameter( mMetalnessParameter );
  mMetalRoughEffect->addParameter( mRoughnessParameter );
  mMetalRoughEffect->addParameter( mParallaxScaleParameter );
  mMetalRoughEffect->addParameter( mEmissionFactorParameter );
  mMetalRoughEffect->addParameter( mTextureScaleParameter );
  mMetalRoughEffect->addParameter( mTextureRotationParameter );
  mMetalRoughEffect->addParameter( mOpacityParameter );

  setEffect( mMetalRoughEffect );
}

void QgsMetalRoughMaterial::updateFragmentShader()
{
  // pre-process fragment shader and add #defines based on whether using maps for some properties
  QByteArray fragmentShaderCode = Qt3DRender::QShaderProgram::loadSource( QUrl( u"qrc:/shaders/metalrough.frag"_s ) );
  QStringList defines;
  if ( mUsingBaseColorMap )
    defines += "BASE_COLOR_MAP";
  if ( mUsingMetalnessMap )
    defines += "METALNESS_MAP";
  if ( mUsingRoughnessMap )
    defines += "ROUGHNESS_MAP";
  if ( mUsingAmbientOcclusionMap )
    defines += "AMBIENT_OCCLUSION_MAP";
  if ( mUsingNormalMap )
    defines += "NORMAL_MAP";
  if ( mUsingHeightMap )
    defines += "HEIGHT_MAP";
  if ( mUsingEmissionMap )
    defines += "EMISSION_MAP";

  if ( mFlatShading )
    defines += "FLAT_SHADING";

  QByteArray finalShaderCode = Qgs3DUtils::addDefinesToShaderCode( fragmentShaderCode, defines );
  mMetalRoughGL3Shader->setFragmentShaderCode( finalShaderCode );
}

void QgsMetalRoughMaterial::setFlatShadingEnabled( bool enabled )
{
  if ( enabled != mFlatShading )
  {
    mFlatShading = enabled;
    updateFragmentShader();
  }
}

void QgsMetalRoughMaterial::setOpacity( float opacity )
{
  mOpacityParameter->setValue( opacity );
}

///@endcond PRIVATE
