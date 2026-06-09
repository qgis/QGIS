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
#include <Qt3DRender/QSeamlessCubemap>
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
  , mReflectanceParameter( new Qt3DRender::QParameter( u"reflectance"_s, 0.5f, this ) )
  , mAnisotropyParameter( new Qt3DRender::QParameter( u"anisotropy"_s, 0.0f, this ) )
  , mAnisotropyRotationParameter( new Qt3DRender::QParameter( u"anisotropyRotation"_s, 0.0f, this ) )
  , mBaseColorMapParameter( new Qt3DRender::QParameter( u"baseColorMap"_s, QVariant(), this ) )
  , mMetalnessMapParameter( new Qt3DRender::QParameter( u"metalnessMap"_s, QVariant(), this ) )
  , mRoughnessMapParameter( new Qt3DRender::QParameter( u"roughnessMap"_s, QVariant(), this ) )
  , mAmbientOcclusionMapParameter( new Qt3DRender::QParameter( u"ambientOcclusionMap"_s, QVariant(), this ) )
  , mNormalMapParameter( new Qt3DRender::QParameter( u"normalMap"_s, QVariant(), this ) )
  , mHeightMapParameter( new Qt3DRender::QParameter( u"heightMap"_s, QVariant(), this ) )
  , mParallaxScaleParameter( new Qt3DRender::QParameter( u"parallaxScale"_s, 0.1f, this ) )
  , mEmissionMapParameter( new Qt3DRender::QParameter( u"emissionMap"_s, QVariant(), this ) )
  , mEmissiveColorParameter( new Qt3DRender::QParameter( u"emissiveColor"_s, Qgs3DUtils::srgbToLinear( QColor( 0, 0, 0 ) ), this ) )
  , mEmissionFactorParameter( new Qt3DRender::QParameter( u"emissiveFactor"_s, 1.0f, this ) )
  , mClearCoatFactorParameter( new Qt3DRender::QParameter( u"clearCoatFactor"_s, 0.0f, this ) )
  , mClearCoatRoughnessParameter( new Qt3DRender::QParameter( u"clearCoatRoughness"_s, 0.0f, this ) )
  , mTextureScaleParameter( new Qt3DRender::QParameter( u"texCoordScale"_s, 1.0f, this ) )
  , mTextureRotationParameter( new Qt3DRender::QParameter( u"texCoordRotation"_s, 0.0f, this ) )
  , mTextureOffsetParameter( new Qt3DRender::QParameter( u"texCoordOffset"_s, QVariant::fromValue( QVector2D( 0, 0 ) ), this ) )
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
  {
    updateShaders();
  }
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
  {
    updateShaders();
  }
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
  {
    updateShaders();
  }
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
  {
    updateShaders();
  }
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
  {
    updateShaders();
  }
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
  {
    updateShaders();
  }
}

void QgsMetalRoughMaterial::setReflectance( float reflectance )
{
  mReflectanceParameter->setValue( QVariant::fromValue( reflectance ) );
}

void QgsMetalRoughMaterial::setAnisotropy( float anisotropy )
{
  const bool oldUsingAnisotropy = mMetalRoughEffect->parameters().contains( mAnisotropyParameter );
  mAnisotropyParameter->setValue( anisotropy );
  const bool newUsingAnisotropy = anisotropy > 0;
  if ( newUsingAnisotropy )
  {
    if ( !oldUsingAnisotropy )
    {
      mMetalRoughEffect->addParameter( mAnisotropyParameter );
      mMetalRoughEffect->addParameter( mAnisotropyRotationParameter );
    }
  }
  else if ( oldUsingAnisotropy )
  {
    mMetalRoughEffect->removeParameter( mAnisotropyParameter );
    mMetalRoughEffect->removeParameter( mAnisotropyRotationParameter );
  }

  if ( oldUsingAnisotropy != newUsingAnisotropy )
  {
    updateShaders();
  }
}

void QgsMetalRoughMaterial::setAnisotropyRotation( float rotation )
{
  mAnisotropyRotationParameter->setValue( M_PI * rotation / 180.0 );
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
  {
    updateShaders();
  }
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
  {
    updateShaders();
  }
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
  {
    updateShaders();
  }
}

void QgsMetalRoughMaterial::setParallaxScale( double scale )
{
  mParallaxScaleParameter->setValue( scale );
}

void QgsMetalRoughMaterial::setEmissionColor( const QColor &color )
{
  mEmissiveColorParameter->setValue( Qgs3DUtils::srgbToLinear( color ) );
  const bool oldUsingEmissionMap = mUsingEmissionMap;

  mUsingEmissionMap = false;
  if ( mMetalRoughEffect->parameters().contains( mEmissionMapParameter ) )
    mMetalRoughEffect->removeParameter( mEmissionMapParameter );
  mMetalRoughEffect->addParameter( mEmissiveColorParameter );

  if ( oldUsingEmissionMap != mUsingEmissionMap )
  {
    updateShaders();
  }
}

void QgsMetalRoughMaterial::setEmissionTexture( Qt3DRender::QAbstractTexture *emission )
{
  const bool oldUsingEmissionMap = mUsingEmissionMap;

  if ( emission )
  {
    mEmissionMapParameter->setValue( QVariant::fromValue( emission ) );
    mUsingEmissionMap = true;
    mMetalRoughEffect->addParameter( mEmissionMapParameter );
    if ( mMetalRoughEffect->parameters().contains( mEmissiveColorParameter ) )
      mMetalRoughEffect->removeParameter( mEmissiveColorParameter );
  }
  else
  {
    mEmissionMapParameter->setValue( QVariant() );
    mUsingEmissionMap = false;
    if ( mMetalRoughEffect->parameters().contains( mEmissionMapParameter ) )
      mMetalRoughEffect->removeParameter( mEmissionMapParameter );
    mMetalRoughEffect->addParameter( mEmissiveColorParameter );
  }

  if ( oldUsingEmissionMap != mUsingEmissionMap )
  {
    updateShaders();
  }
}

void QgsMetalRoughMaterial::setEmissionFactor( double factor )
{
  mEmissionFactorParameter->setValue( factor );
}

void QgsMetalRoughMaterial::setClearCoatFactor( float factor )
{
  mClearCoatFactorParameter->setValue( factor );
  const bool oldUsingClearCoat = mMetalRoughEffect->parameters().contains( mClearCoatFactorParameter );
  const bool newUsingClearCoat = factor > 0;
  if ( newUsingClearCoat )
  {
    if ( !oldUsingClearCoat )
    {
      mMetalRoughEffect->addParameter( mClearCoatFactorParameter );
      mMetalRoughEffect->addParameter( mClearCoatRoughnessParameter );
    }
  }
  else if ( oldUsingClearCoat )
  {
    mMetalRoughEffect->removeParameter( mClearCoatFactorParameter );
    mMetalRoughEffect->removeParameter( mClearCoatRoughnessParameter );
  }

  if ( oldUsingClearCoat != newUsingClearCoat )
  {
    updateShaders();
  }
}

void QgsMetalRoughMaterial::setClearCoatRoughness( float roughness )
{
  mClearCoatRoughnessParameter->setValue( roughness );
}

void QgsMetalRoughMaterial::setTextureScale( float textureScale )
{
  mTextureScaleParameter->setValue( textureScale );
}

void QgsMetalRoughMaterial::setTextureRotation( float textureRotation )
{
  mTextureRotationParameter->setValue( textureRotation );
}

void QgsMetalRoughMaterial::setTextureOffset( float textureOffsetX, float textureOffsetY )
{
  mTextureOffsetParameter->setValue( QVariant::fromValue( QVector2D( textureOffsetX, textureOffsetY ) ) );
}

void QgsMetalRoughMaterial::init()
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
  mMetalRoughEffect->addTechnique( mMetalRoughGL3Technique );

  // ensure IBL cubemaps are seamless -- this should be safe to do here, the only cubemap
  // lookups happening in the metalrough shader is for IBL
  mMetalRoughGL3RenderPass->addRenderState( new Qt3DRender::QSeamlessCubemap( this ) );

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
  mMetalRoughEffect->addParameter( mReflectanceParameter );
  mMetalRoughEffect->addParameter( mParallaxScaleParameter );
  mMetalRoughEffect->addParameter( mEmissiveColorParameter );
  mMetalRoughEffect->addParameter( mEmissionFactorParameter );
  mMetalRoughEffect->addParameter( mTextureScaleParameter );
  mMetalRoughEffect->addParameter( mTextureRotationParameter );
  mMetalRoughEffect->addParameter( mTextureOffsetParameter );
  mMetalRoughEffect->addParameter( mOpacityParameter );

  setEffect( mMetalRoughEffect );

  updateShaders();
}

void QgsMetalRoughMaterial::updateShaders()
{
  QByteArray fragmentShaderCode = Qt3DRender::QShaderProgram::loadSource( QUrl( u"qrc:/shaders/metalrough.frag"_s ) );

  // pre-process fragment shader and add #defines based on whether using maps for some properties
  QStringList fragShaderDefines;
  if ( mUsingBaseColorMap )
    fragShaderDefines += "BASE_COLOR_MAP";
  if ( mUsingMetalnessMap )
    fragShaderDefines += "METALNESS_MAP";
  if ( mUsingRoughnessMap )
    fragShaderDefines += "ROUGHNESS_MAP";
  if ( mUsingAmbientOcclusionMap )
    fragShaderDefines += "AMBIENT_OCCLUSION_MAP";
  if ( mUsingNormalMap )
    fragShaderDefines += "NORMAL_MAP";
  if ( mUsingHeightMap )
    fragShaderDefines += "HEIGHT_MAP";
  if ( mUsingEmissionMap )
    fragShaderDefines += "EMISSION_MAP";
  if ( mFlatShading )
    fragShaderDefines += "FLAT_SHADING";
  if ( mMetalRoughEffect->parameters().contains( mAnisotropyParameter ) )
    fragShaderDefines += "ANISOTROPY";
  if ( mMetalRoughEffect->parameters().contains( mClearCoatFactorParameter ) )
    fragShaderDefines += "CLEAR_COAT";
  if ( mEnableEnvironmentalLighting )
    fragShaderDefines += "ENABLE_IBL";

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
    fragShaderDefines += "DATA_DEFINED";
    mMetalRoughGL3Shader->setShaderCode( Qt3DRender::QShaderProgram::Vertex, Qt3DRender::QShaderProgram::loadSource( QUrl( u"qrc:/shaders/metalroughDataDefined.vert"_s ) ) );
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

  const QByteArray finalShaderCode = Qgs3DUtils::addDefinesToShaderCode( fragmentShaderCode, fragShaderDefines );
  mMetalRoughGL3Shader->setFragmentShaderCode( finalShaderCode );
}

void QgsMetalRoughMaterial::setFlatShadingEnabled( bool enabled )
{
  if ( enabled != mFlatShading )
  {
    mFlatShading = enabled;
    updateShaders();
  }
}

void QgsMetalRoughMaterial::setOpacity( float opacity )
{
  mOpacityParameter->setValue( opacity );
}

void QgsMetalRoughMaterial::setDataDefinedEnabled( bool enabled )
{
  if ( enabled != mDataDefinedEnabled )
  {
    mDataDefinedEnabled = enabled;
    updateShaders();
  }
}

void QgsMetalRoughMaterial::setDataDefinedTextureTransformEnabled( bool enabled )
{
  if ( enabled != mDataDefinedTextureTransformEnabled )
  {
    mDataDefinedTextureTransformEnabled = enabled;
    updateShaders();
  }
}

void QgsMetalRoughMaterial::setEnvironmentalLightingEnabled( bool enabled )
{
  if ( enabled != mEnableEnvironmentalLighting )
  {
    mEnableEnvironmentalLighting = enabled;
    updateShaders();
  }
}

void QgsMetalRoughMaterial::setInstancingEnabled( bool enabled, Qgis::InstancedMaterialFlags flags, const QMatrix3x3 &axisTransform, const QMatrix4x4 &nodeTransform )
{
  mInstanced = enabled;
  mInstanceFlags = flags;

  if ( mInstanced )
  {
    const QMatrix3x3 nodeNormalTransform = nodeTransform.normalMatrix();

    if ( !mNodeTransformParameter )
    {
      mNodeTransformParameter = new Qt3DRender::QParameter( u"nodeTransform"_s, QVariant::fromValue( nodeTransform ), this );
      addParameter( mNodeTransformParameter );
    }
    else
      mNodeTransformParameter->setValue( QVariant::fromValue( nodeTransform ) );

    if ( !mAxisTransformParameter )
    {
      mAxisTransformParameter = new Qt3DRender::QParameter( u"axisTransform"_s, QVariant::fromValue( axisTransform ), this );
      addParameter( mAxisTransformParameter );
    }
    else
      mAxisTransformParameter->setValue( QVariant::fromValue( axisTransform ) );

    if ( !mNodeNormalTransformParameter )
    {
      mNodeNormalTransformParameter = new Qt3DRender::QParameter( u"nodeNormalTransform"_s, QVariant::fromValue( nodeNormalTransform ), this );
      addParameter( mNodeNormalTransformParameter );
    }
    else
      mNodeNormalTransformParameter->setValue( QVariant::fromValue( nodeNormalTransform ) );

    QStringList defines = { u"HAS_TEXTURE"_s, u"HAS_TANGENT"_s };
    if ( mInstanceFlags.testFlag( Qgis::InstancedMaterialFlag::DataDefinedScale ) )
      defines << u"USE_INSTANCE_SCALE"_s;
    if ( mInstanceFlags.testFlag( Qgis::InstancedMaterialFlag::DataDefinedRotation ) )
      defines << u"USE_INSTANCE_ROTATION"_s;
    const QByteArray vertCode = Qt3DRender::QShaderProgram::loadSource( QUrl( u"qrc:/shaders/instanced.vert"_s ) );
    mMetalRoughGL3Shader->setVertexShaderCode( Qgs3DUtils::addDefinesToShaderCode( vertCode, defines ) );
  }
  else
  {
    const QByteArray vertCode = Qt3DRender::QShaderProgram::loadSource( QUrl( u"qrc:/shaders/default.vert"_s ) );
    mMetalRoughGL3Shader->setVertexShaderCode( Qgs3DUtils::addDefinesToShaderCode( vertCode, { u"TEXTURE_ROTATION"_s } ) );
  }
  updateShaders();
}

///@endcond PRIVATE
