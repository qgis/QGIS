/***************************************************************************
  qgspbrmaterial.cpp
  --------------------------------------
  Date                 : April 2026
  Copyright            : (C) 2026 by Nyall Dawson
  Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgspbrmaterial.h"

#include "qgs3dutils.h"

#include <QString>
#include <Qt3DRender/QAbstractTexture>
#include <Qt3DRender/QGraphicsApiFilter>
#include <Qt3DRender/QParameter>
#include <Qt3DRender/QRenderPass>
#include <Qt3DRender/QSeamlessCubemap>
#include <Qt3DRender/QShaderProgramBuilder>
#include <Qt3DRender/QTechnique>
#include <Qt3DRender/QTexture>

#include "moc_qgspbrmaterial.cpp"

using namespace Qt::StringLiterals;

///@cond PRIVATE
QgsPBRMaterial::QgsPBRMaterial( QNode *parent )
  : QgsMaterial( parent )
  , mEffect( new Qt3DRender::QEffect( this ) )
  , mBaseColorParameter( new Qt3DRender::QParameter( u"baseColor"_s, Qgs3DUtils::srgbToLinear( QColor( "grey" ) ), this ) )
  , mRoughnessParameter( new Qt3DRender::QParameter( u"roughness"_s, 0.0f, this ) )
  , mBaseColorMapParameter( new Qt3DRender::QParameter( u"baseColorMap"_s, QVariant(), this ) )
  , mRoughnessMapParameter( new Qt3DRender::QParameter( u"roughnessMap"_s, QVariant(), this ) )
  , mAmbientOcclusionMapParameter( new Qt3DRender::QParameter( u"ambientOcclusionMap"_s, QVariant(), this ) )
  , mNormalMapParameter( new Qt3DRender::QParameter( u"normalMap"_s, QVariant(), this ) )
  , mHeightMapParameter( new Qt3DRender::QParameter( u"heightMap"_s, QVariant(), this ) )
  , mParallaxScaleParameter( new Qt3DRender::QParameter( u"parallaxScale"_s, 0.1f, this ) )
  , mTextureScaleParameter( new Qt3DRender::QParameter( u"texCoordScale"_s, 1.0f, this ) )
  , mTextureRotationParameter( new Qt3DRender::QParameter( u"texCoordRotation"_s, 0.0f, this ) )
  , mTextureOffsetParameter( new Qt3DRender::QParameter( u"texCoordOffset"_s, QVariant::fromValue( QVector2D( 0, 0 ) ), this ) )
  , mOpacityParameter( new Qt3DRender::QParameter( u"opacity"_s, 1.0f ) )
  , mMetalRoughGL3Technique( new Qt3DRender::QTechnique( this ) )
  , mMetalRoughGL3RenderPass( new Qt3DRender::QRenderPass( this ) )
  , mMetalRoughGL3Shader( new Qt3DRender::QShaderProgram( this ) )
  , mFilterKey( new Qt3DRender::QFilterKey( this ) )
{}

QgsPBRMaterial::~QgsPBRMaterial() = default;

void QgsPBRMaterial::setBaseColor( const QColor &baseColor )
{
  mBaseColorParameter->setValue( Qgs3DUtils::srgbToLinear( baseColor ) );
  bool oldUsingBaseColorMap = mUsingBaseColorMap;

  mUsingBaseColorMap = false;
  if ( mEffect->parameters().contains( mBaseColorMapParameter ) )
    mEffect->removeParameter( mBaseColorMapParameter );
  mEffect->addParameter( mBaseColorParameter );

  if ( oldUsingBaseColorMap != mUsingBaseColorMap )
  {
    updateShaders();
  }
}

void QgsPBRMaterial::setBaseColorTexture( Qt3DRender::QAbstractTexture *baseColor )
{
  mBaseColorMapParameter->setValue( QVariant::fromValue( baseColor ) );
  bool oldUsingBaseColorMap = mUsingBaseColorMap;

  mUsingBaseColorMap = true;
  mEffect->addParameter( mBaseColorMapParameter );
  if ( mEffect->parameters().contains( mBaseColorParameter ) )
    mEffect->removeParameter( mBaseColorParameter );

  if ( oldUsingBaseColorMap != mUsingBaseColorMap )
  {
    updateShaders();
  }
}

void QgsPBRMaterial::setRoughness( float roughness )
{
  mRoughnessParameter->setValue( roughness );
  bool oldUsingRoughnessMap = mUsingRoughnessMap;

  mUsingRoughnessMap = false;
  if ( mEffect->parameters().contains( mRoughnessMapParameter ) )
    mEffect->removeParameter( mRoughnessMapParameter );
  mEffect->addParameter( mRoughnessParameter );

  if ( oldUsingRoughnessMap != mUsingRoughnessMap )
  {
    updateShaders();
  }
}

void QgsPBRMaterial::setRoughnessTexture( Qt3DRender::QAbstractTexture *roughness )
{
  mRoughnessMapParameter->setValue( QVariant::fromValue( roughness ) );
  bool oldUsingRoughnessMap = mUsingRoughnessMap;

  mUsingRoughnessMap = true;
  mEffect->addParameter( mRoughnessMapParameter );
  if ( mEffect->parameters().contains( mRoughnessParameter ) )
    mEffect->removeParameter( mRoughnessParameter );

  if ( oldUsingRoughnessMap != mUsingRoughnessMap )
  {
    updateShaders();
  }
}

void QgsPBRMaterial::setAmbientOcclusionTexture( Qt3DRender::QAbstractTexture *ambientOcclusion )
{
  bool oldUsingAmbientOcclusionMap = mUsingAmbientOcclusionMap;

  if ( ambientOcclusion )
  {
    mAmbientOcclusionMapParameter->setValue( QVariant::fromValue( ambientOcclusion ) );
    mUsingAmbientOcclusionMap = true;
    mEffect->addParameter( mAmbientOcclusionMapParameter );
  }
  else
  {
    mAmbientOcclusionMapParameter->setValue( QVariant() );
    mUsingAmbientOcclusionMap = false;
    if ( mEffect->parameters().contains( mAmbientOcclusionMapParameter ) )
      mEffect->removeParameter( mAmbientOcclusionMapParameter );
  }

  if ( oldUsingAmbientOcclusionMap != mUsingAmbientOcclusionMap )
  {
    updateShaders();
  }
}

void QgsPBRMaterial::setNormalTexture( Qt3DRender::QAbstractTexture *normal )
{
  bool oldUsingNormalMap = mUsingNormalMap;

  if ( normal )
  {
    mNormalMapParameter->setValue( QVariant::fromValue( normal ) );
    mUsingNormalMap = true;
    mEffect->addParameter( mNormalMapParameter );
  }
  else
  {
    mNormalMapParameter->setValue( QVariant() );
    mUsingNormalMap = false;
    if ( mEffect->parameters().contains( mNormalMapParameter ) )
      mEffect->removeParameter( mNormalMapParameter );
  }

  if ( oldUsingNormalMap != mUsingNormalMap )
  {
    updateShaders();
  }
}

void QgsPBRMaterial::setHeightTexture( Qt3DRender::QAbstractTexture *height )
{
  bool oldUsingHeightMap = mUsingHeightMap;

  if ( height )
  {
    mHeightMapParameter->setValue( QVariant::fromValue( height ) );
    mUsingHeightMap = true;
    mEffect->addParameter( mHeightMapParameter );
  }
  else
  {
    mHeightMapParameter->setValue( QVariant() );
    mUsingHeightMap = false;
    if ( mEffect->parameters().contains( mHeightMapParameter ) )
      mEffect->removeParameter( mHeightMapParameter );
  }

  if ( oldUsingHeightMap != mUsingHeightMap )
  {
    updateShaders();
  }
}

void QgsPBRMaterial::setParallaxScale( double scale )
{
  mParallaxScaleParameter->setValue( scale );
}

void QgsPBRMaterial::setTextureScale( float textureScale )
{
  mTextureScaleParameter->setValue( textureScale );
}

void QgsPBRMaterial::setTextureRotation( float textureRotation )
{
  mTextureRotationParameter->setValue( textureRotation );
}

void QgsPBRMaterial::setTextureOffset( float textureOffsetX, float textureOffsetY )
{
  mTextureOffsetParameter->setValue( QVariant::fromValue( QVector2D( textureOffsetX, textureOffsetY ) ) );
}

void QgsPBRMaterial::initMaterial()
{
  mMetalRoughGL3Technique->graphicsApiFilter()->setApi( Qt3DRender::QGraphicsApiFilter::OpenGL );
  mMetalRoughGL3Technique->graphicsApiFilter()->setMajorVersion( 3 );
  mMetalRoughGL3Technique->graphicsApiFilter()->setMinorVersion( 3 );
  mMetalRoughGL3Technique->graphicsApiFilter()->setProfile( Qt3DRender::QGraphicsApiFilter::CoreProfile );

  mFilterKey->setParent( this );
  mFilterKey->setName( u"renderingStyle"_s );
  mFilterKey->setValue( u"forward"_s );

  mMetalRoughGL3Technique->addFilterKey( mFilterKey );
  mMetalRoughGL3RenderPass->setShaderProgram( mMetalRoughGL3Shader );
  mMetalRoughGL3Technique->addRenderPass( mMetalRoughGL3RenderPass );
  mEffect->addTechnique( mMetalRoughGL3Technique );

  // ensure IBL cubemaps are seamless -- this should be safe to do here, the only cubemap
  // lookups happening in the metalrough shader is for IBL
  mMetalRoughGL3RenderPass->addRenderState( new Qt3DRender::QSeamlessCubemap( this ) );

  // Given parameters a parent
  mBaseColorMapParameter->setParent( mEffect );
  mRoughnessMapParameter->setParent( mEffect );
  mNormalMapParameter->setParent( mEffect );
  mHeightMapParameter->setParent( mEffect );
  mAmbientOcclusionMapParameter->setParent( mEffect );

  mEffect->addParameter( mBaseColorParameter );
  mEffect->addParameter( mRoughnessParameter );
  mEffect->addParameter( mParallaxScaleParameter );
  mEffect->addParameter( mTextureScaleParameter );
  mEffect->addParameter( mTextureRotationParameter );
  mEffect->addParameter( mTextureOffsetParameter );
  mEffect->addParameter( mOpacityParameter );

  setEffect( mEffect );

  updateShaders();
}

QStringList QgsPBRMaterial::fragmentShaderDefines() const
{
  QStringList defines;
  if ( mUsingBaseColorMap )
    defines << "BASE_COLOR_MAP";
  if ( mUsingRoughnessMap )
    defines << "ROUGHNESS_MAP";
  if ( mUsingAmbientOcclusionMap )
    defines << "AMBIENT_OCCLUSION_MAP";
  if ( mUsingNormalMap )
    defines << "NORMAL_MAP";
  if ( mUsingHeightMap )
    defines << "HEIGHT_MAP";
  if ( mFlatShading )
    defines << "FLAT_SHADING";
  if ( mDataDefinedEnabled )
    defines << "DATA_DEFINED";
  if ( mEnableEnvironmentalLighting )
    defines << "ENABLE_IBL";
  if ( !mInstanced && mDataDefinedEnabled )
    defines << "DATA_DEFINED";

  return defines;
}

void QgsPBRMaterial::updateShaders()
{
  QByteArray fragmentShaderCode = Qt3DRender::QShaderProgram::loadSource( QUrl( u"qrc:/shaders/pbr.frag"_s ) );

  // pre-process fragment shader and add #defines based on whether using maps for some properties
  if ( mInstanced )
  {
    QStringList defines = { u"HAS_TEXTURE"_s, u"HAS_TANGENT"_s };
    if ( mInstanceFlags.testFlag( Qgis::InstancedMaterialFlag::DataDefinedScale ) )
      defines << u"USE_INSTANCE_SCALE"_s;
    if ( mInstanceFlags.testFlag( Qgis::InstancedMaterialFlag::DataDefinedRotation ) )
      defines << u"USE_INSTANCE_ROTATION"_s;
    const QByteArray vertCode = Qt3DRender::QShaderProgram::loadSource( QUrl( u"qrc:/shaders/instanced.vert"_s ) );
    mMetalRoughGL3Shader->setVertexShaderCode( Qgs3DUtils::addDefinesToShaderCode( vertCode, defines ) );
  }
  else if ( mDataDefinedEnabled )
  {
    mMetalRoughGL3Shader->setShaderCode( Qt3DRender::QShaderProgram::Vertex, Qt3DRender::QShaderProgram::loadSource( QUrl( u"qrc:/shaders/pbrDataDefined.vert"_s ) ) );
  }
  else
  {
    const QByteArray vertexShaderCode = Qt3DRender::QShaderProgram::loadSource( QUrl( u"qrc:/shaders/default.vert"_s ) );
    QStringList defines { u"TEXTURE_ROTATION"_s, u"TEXTURE_OFFSET"_s };
    if ( mDataDefinedTextureTransformEnabled )
      defines << u"DATA_DEFINED_TEXTURE_TRANSFORMS"_s;

    const QByteArray finalVertexShaderCode = Qgs3DUtils::addDefinesToShaderCode( vertexShaderCode, defines );
    mMetalRoughGL3Shader->setVertexShaderCode( finalVertexShaderCode );
  }

  const QByteArray finalShaderCode = Qgs3DUtils::addDefinesToShaderCode( fragmentShaderCode, fragmentShaderDefines() );
  mMetalRoughGL3Shader->setFragmentShaderCode( finalShaderCode );
}

void QgsPBRMaterial::setFlatShadingEnabled( bool enabled )
{
  if ( enabled != mFlatShading )
  {
    mFlatShading = enabled;
    updateShaders();
  }
}

void QgsPBRMaterial::setOpacity( float opacity )
{
  mOpacityParameter->setValue( opacity );
}

void QgsPBRMaterial::setDataDefinedEnabled( bool enabled )
{
  if ( enabled != mDataDefinedEnabled )
  {
    mDataDefinedEnabled = enabled;
    updateShaders();
  }
}

void QgsPBRMaterial::setInstancingEnabled( bool enabled, Qgis::InstancedMaterialFlags flags )
{
  mInstanced = enabled;
  mInstanceFlags = flags;
  updateShaders();
}

void QgsPBRMaterial::setDataDefinedTextureTransformEnabled( bool enabled )
{
  if ( enabled != mDataDefinedTextureTransformEnabled )
  {
    mDataDefinedTextureTransformEnabled = enabled;
    updateShaders();
  }
}

void QgsPBRMaterial::setEnvironmentalLightingEnabled( bool enabled )
{
  if ( enabled != mEnableEnvironmentalLighting )
  {
    mEnableEnvironmentalLighting = enabled;
    updateShaders();
  }
}

///@endcond PRIVATE
