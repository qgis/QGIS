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
#include "moc_qgsmetalroughmaterial.cpp"
#include "qgs3dutils.h"
#include <Qt3DRender/QParameter>
#include <Qt3DRender/QRenderPass>
#include <Qt3DRender/QTechnique>
#include <Qt3DRender/QTexture>
#include <Qt3DRender/QAbstractTexture>
#include <Qt3DRender/QShaderProgramBuilder>
#include <Qt3DRender/QGraphicsApiFilter>

///@cond PRIVATE
QgsMetalRoughMaterial::QgsMetalRoughMaterial( QNode *parent )
  : QgsMaterial( parent )
  , mBaseColorParameter( new Qt3DRender::QParameter( QStringLiteral( "baseColor" ), QColor( "grey" ), this ) )
  , mMetalnessParameter( new Qt3DRender::QParameter( QStringLiteral( "metalness" ), 0.0f, this ) )
  , mRoughnessParameter( new Qt3DRender::QParameter( QStringLiteral( "roughness" ), 0.0f, this ) )
  , mBaseColorMapParameter( new Qt3DRender::QParameter( QStringLiteral( "baseColorMap" ), QVariant(), this ) )
  , mMetalnessMapParameter( new Qt3DRender::QParameter( QStringLiteral( "metalnessMap" ), QVariant(), this ) )
  , mRoughnessMapParameter( new Qt3DRender::QParameter( QStringLiteral( "roughnessMap" ), QVariant(), this ) )
  , mAmbientOcclusionMapParameter( new Qt3DRender::QParameter( QStringLiteral( "ambientOcclusionMap" ), QVariant(), this ) )
  , mNormalMapParameter( new Qt3DRender::QParameter( QStringLiteral( "normalMap" ), QVariant(), this ) )
  , mTextureScaleParameter( new Qt3DRender::QParameter( QStringLiteral( "texCoordScale" ), 1.0f, this ) )
  , mMetalRoughEffect( new Qt3DRender::QEffect( this ) )
  , mMetalRoughGL3Technique( new Qt3DRender::QTechnique( this ) )
  , mMetalRoughGL3RenderPass( new Qt3DRender::QRenderPass( this ) )
  , mMetalRoughGL3Shader( new Qt3DRender::QShaderProgram( this ) )
  , mFilterKey( new Qt3DRender::QFilterKey( this ) )
{
  init();
}

QgsMetalRoughMaterial::~QgsMetalRoughMaterial() = default;

QVariant QgsMetalRoughMaterial::baseColor() const
{
  return mBaseColorParameter->value();
}

QVariant QgsMetalRoughMaterial::metalness() const
{
  return mMetalnessParameter->value();
}

QVariant QgsMetalRoughMaterial::roughness() const
{
  return mRoughnessParameter->value();
}

QVariant QgsMetalRoughMaterial::ambientOcclusion() const
{
  return mAmbientOcclusionMapParameter->value();
}

QVariant QgsMetalRoughMaterial::normal() const
{
  return mNormalMapParameter->value();
}

float QgsMetalRoughMaterial::textureScale() const
{
  return mTextureScaleParameter->value().toFloat();
}

void QgsMetalRoughMaterial::setBaseColor( const QVariant &baseColor )
{
  mBaseColorParameter->setValue( baseColor );
  mBaseColorMapParameter->setValue( baseColor );
  bool oldUsingBaseColorMap = mUsingBaseColorMap;

  if ( baseColor.value<Qt3DRender::QAbstractTexture *>() )
  {
    mUsingBaseColorMap = true;
    mMetalRoughEffect->addParameter( mBaseColorMapParameter );
    if ( mMetalRoughEffect->parameters().contains( mBaseColorParameter ) )
      mMetalRoughEffect->removeParameter( mBaseColorParameter );
  }
  else
  {
    mUsingBaseColorMap = false;
    if ( mMetalRoughEffect->parameters().contains( mBaseColorMapParameter ) )
      mMetalRoughEffect->removeParameter( mBaseColorMapParameter );
    mMetalRoughEffect->addParameter( mBaseColorParameter );
  }

  if ( oldUsingBaseColorMap != mUsingBaseColorMap )
    updateFragmentShader();
}

void QgsMetalRoughMaterial::setMetalness( const QVariant &metalness )
{
  mMetalnessParameter->setValue( metalness );
  mMetalnessMapParameter->setValue( metalness );
  bool oldUsingMetalnessMap = mUsingMetalnessMap;

  if ( metalness.value<Qt3DRender::QAbstractTexture *>() )
  {
    mUsingMetalnessMap = true;
    mMetalRoughEffect->addParameter( mMetalnessMapParameter );
    if ( mMetalRoughEffect->parameters().contains( mMetalnessParameter ) )
      mMetalRoughEffect->removeParameter( mMetalnessParameter );
  }
  else
  {
    mUsingMetalnessMap = false;
    if ( mMetalRoughEffect->parameters().contains( mMetalnessMapParameter ) )
      mMetalRoughEffect->removeParameter( mMetalnessMapParameter );
    mMetalRoughEffect->addParameter( mMetalnessParameter );
  }

  if ( oldUsingMetalnessMap != mUsingMetalnessMap )
    updateFragmentShader();
}

void QgsMetalRoughMaterial::setRoughness( const QVariant &roughness )
{
  mRoughnessParameter->setValue( roughness );
  mRoughnessMapParameter->setValue( roughness );
  bool oldUsingRoughnessMap = mUsingRoughnessMap;

  if ( roughness.value<Qt3DRender::QAbstractTexture *>() )
  {
    mUsingRoughnessMap = true;
    mMetalRoughEffect->addParameter( mRoughnessMapParameter );
    if ( mMetalRoughEffect->parameters().contains( mRoughnessParameter ) )
      mMetalRoughEffect->removeParameter( mRoughnessParameter );
  }
  else
  {
    mUsingRoughnessMap = false;
    if ( mMetalRoughEffect->parameters().contains( mRoughnessMapParameter ) )
      mMetalRoughEffect->removeParameter( mRoughnessMapParameter );
    mMetalRoughEffect->addParameter( mRoughnessParameter );
  }

  if ( oldUsingRoughnessMap != mUsingRoughnessMap )
    updateFragmentShader();
}

void QgsMetalRoughMaterial::setAmbientOcclusion( const QVariant &ambientOcclusion )
{
  mAmbientOcclusionMapParameter->setValue( ambientOcclusion );
  bool oldUsingAmbientOcclusionMap = mUsingAmbientOcclusionMap;

  if ( ambientOcclusion.value<Qt3DRender::QAbstractTexture *>() )
  {
    mUsingAmbientOcclusionMap = true;
    mMetalRoughEffect->addParameter( mAmbientOcclusionMapParameter );
  }
  else
  {
    mUsingAmbientOcclusionMap = false;
    if ( mMetalRoughEffect->parameters().contains( mAmbientOcclusionMapParameter ) )
      mMetalRoughEffect->removeParameter( mAmbientOcclusionMapParameter );
  }

  if ( oldUsingAmbientOcclusionMap != mUsingAmbientOcclusionMap )
    updateFragmentShader();
}

void QgsMetalRoughMaterial::setNormal( const QVariant &normal )
{
  mNormalMapParameter->setValue( normal );
  bool oldUsingNormalMap = mUsingNormalMap;

  if ( normal.value<Qt3DRender::QAbstractTexture *>() )
  {
    mUsingNormalMap = true;
    mMetalRoughEffect->addParameter( mNormalMapParameter );
  }
  else
  {
    mUsingNormalMap = false;
    if ( mMetalRoughEffect->parameters().contains( mNormalMapParameter ) )
      mMetalRoughEffect->removeParameter( mNormalMapParameter );
  }

  if ( oldUsingNormalMap != mUsingNormalMap )
    updateFragmentShader();
}

void QgsMetalRoughMaterial::setTextureScale( float textureScale )
{
  mTextureScaleParameter->setValue( textureScale );
}

void QgsMetalRoughMaterial::init()
{
  QObject::connect( mBaseColorParameter, &Qt3DRender::QParameter::valueChanged, this, &QgsMetalRoughMaterial::baseColorChanged );
  QObject::connect( mMetalnessParameter, &Qt3DRender::QParameter::valueChanged, this, &QgsMetalRoughMaterial::metalnessChanged );
  QObject::connect( mRoughnessParameter, &Qt3DRender::QParameter::valueChanged, this, &QgsMetalRoughMaterial::roughnessChanged );
  QObject::connect( mAmbientOcclusionMapParameter, &Qt3DRender::QParameter::valueChanged, this, &QgsMetalRoughMaterial::ambientOcclusionChanged );
  QObject::connect( mNormalMapParameter, &Qt3DRender::QParameter::valueChanged, this, &QgsMetalRoughMaterial::normalChanged );
  connect( mTextureScaleParameter, &Qt3DRender::QParameter::valueChanged, this, &QgsMetalRoughMaterial::handleTextureScaleChanged );

  mMetalRoughGL3Shader->setVertexShaderCode( Qt3DRender::QShaderProgram::loadSource( QUrl( QStringLiteral( "qrc:/shaders/default.vert" ) ) ) );

  updateFragmentShader();

  mMetalRoughGL3Technique->graphicsApiFilter()->setApi( Qt3DRender::QGraphicsApiFilter::OpenGL );
  mMetalRoughGL3Technique->graphicsApiFilter()->setMajorVersion( 3 );
  mMetalRoughGL3Technique->graphicsApiFilter()->setMinorVersion( 1 );
  mMetalRoughGL3Technique->graphicsApiFilter()->setProfile( Qt3DRender::QGraphicsApiFilter::CoreProfile );

  mFilterKey->setParent( this );
  mFilterKey->setName( QStringLiteral( "renderingStyle" ) );
  mFilterKey->setValue( QStringLiteral( "forward" ) );

  mMetalRoughGL3Technique->addFilterKey( mFilterKey );
  mMetalRoughGL3RenderPass->setShaderProgram( mMetalRoughGL3Shader );
  mMetalRoughGL3Technique->addRenderPass( mMetalRoughGL3RenderPass );
  mMetalRoughEffect->addTechnique( mMetalRoughGL3Technique );

  // Given parameters a parent
  mBaseColorMapParameter->setParent( mMetalRoughEffect );
  mMetalnessMapParameter->setParent( mMetalRoughEffect );
  mRoughnessMapParameter->setParent( mMetalRoughEffect );

  mMetalRoughEffect->addParameter( mBaseColorParameter );
  mMetalRoughEffect->addParameter( mMetalnessParameter );
  mMetalRoughEffect->addParameter( mRoughnessParameter );
  mMetalRoughEffect->addParameter( mTextureScaleParameter );

  setEffect( mMetalRoughEffect );
}

void QgsMetalRoughMaterial::updateFragmentShader()
{
  // pre-process fragment shader and add #defines based on whether using maps for some properties
  QByteArray fragmentShaderCode = Qt3DRender::QShaderProgram::loadSource( QUrl( QStringLiteral( "qrc:/shaders/metalrough.frag" ) ) );
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

  if ( mFlatShading )
    defines += "FLAT_SHADING";

  QByteArray finalShaderCode = Qgs3DUtils::addDefinesToShaderCode( fragmentShaderCode, defines );
  mMetalRoughGL3Shader->setFragmentShaderCode( finalShaderCode );
}

void QgsMetalRoughMaterial::handleTextureScaleChanged( const QVariant &var )
{
  emit textureScaleChanged( var.toFloat() );
}

bool QgsMetalRoughMaterial::flatShadingEnabled() const
{
  return mFlatShading;
}

void QgsMetalRoughMaterial::setFlatShadingEnabled( bool enabled )
{
  if ( enabled != mFlatShading )
  {
    mFlatShading = enabled;
    updateFragmentShader();
  }
}

///@endcond PRIVATE
