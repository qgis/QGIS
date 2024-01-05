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
#include <Qt3DRender/QParameter>
#include <Qt3DRender/QRenderPass>
#include <Qt3DRender/QTechnique>
#include <Qt3DRender/QTexture>
#include <Qt3DRender/QAbstractTexture>
#include <Qt3DRender/QEffect>
#include <Qt3DRender/QShaderProgramBuilder>
#include <Qt3DRender/QGraphicsApiFilter>

///@cond PRIVATE
QgsMetalRoughMaterial::QgsMetalRoughMaterial( QNode *parent )
  : QMaterial( parent )
  , mBaseColorParameter( new Qt3DRender::QParameter( QStringLiteral( "baseColor" ), QColor( "grey" ) ) )
  , mMetalnessParameter( new Qt3DRender::QParameter( QStringLiteral( "metalness" ), 0.0f ) )
  , mRoughnessParameter( new Qt3DRender::QParameter( QStringLiteral( "roughness" ), 0.0f ) )
  , mBaseColorMapParameter( new Qt3DRender::QParameter( QStringLiteral( "baseColorMap" ), QVariant() ) )
  , mMetalnessMapParameter( new Qt3DRender::QParameter( QStringLiteral( "metalnessMap" ), QVariant() ) )
  , mRoughnessMapParameter( new Qt3DRender::QParameter( QStringLiteral( "roughnessMap" ), QVariant() ) )
  , mAmbientOcclusionMapParameter( new Qt3DRender::QParameter( QStringLiteral( "ambientOcclusionMap" ), QVariant() ) )
  , mNormalMapParameter( new Qt3DRender::QParameter( QStringLiteral( "normalMap" ), QVariant() ) )
  , mTextureScaleParameter( new Qt3DRender::QParameter( QStringLiteral( "texCoordScale" ), 1.0f ) )
  , mMetalRoughEffect( new Qt3DRender::QEffect() )
  , mMetalRoughGL3Technique( new Qt3DRender::QTechnique() )
  , mMetalRoughGL3RenderPass( new Qt3DRender::QRenderPass() )
  , mMetalRoughGL3Shader( new Qt3DRender::QShaderProgram() )
  , mMetalRoughGL3ShaderBuilder( new Qt3DRender::QShaderProgramBuilder() )
  , mFilterKey( new Qt3DRender::QFilterKey )
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

  auto layers = mMetalRoughGL3ShaderBuilder->enabledLayers();
  if ( baseColor.value<Qt3DRender::QAbstractTexture *>() )
  {
    layers.removeAll( QStringLiteral( "baseColor" ) );
    layers.append( QStringLiteral( "baseColorMap" ) );
    mMetalRoughEffect->addParameter( mBaseColorMapParameter );
    if ( mMetalRoughEffect->parameters().contains( mBaseColorParameter ) )
      mMetalRoughEffect->removeParameter( mBaseColorParameter );
  }
  else
  {
    layers.removeAll( QStringLiteral( "baseColorMap" ) );
    layers.append( QStringLiteral( "baseColor" ) );
    if ( mMetalRoughEffect->parameters().contains( mBaseColorMapParameter ) )
      mMetalRoughEffect->removeParameter( mBaseColorMapParameter );
    mMetalRoughEffect->addParameter( mBaseColorParameter );
  }
  updateLayersOnTechnique( layers );
}

void QgsMetalRoughMaterial::setMetalness( const QVariant &metalness )
{
  mMetalnessParameter->setValue( metalness );
  mMetalnessMapParameter->setValue( metalness );

  auto layers = mMetalRoughGL3ShaderBuilder->enabledLayers();
  if ( metalness.value<Qt3DRender::QAbstractTexture *>() )
  {
    layers.removeAll( QStringLiteral( "metalness" ) );
    layers.append( QStringLiteral( "metalnessMap" ) );
    mMetalRoughEffect->addParameter( mMetalnessMapParameter );
    if ( mMetalRoughEffect->parameters().contains( mMetalnessParameter ) )
      mMetalRoughEffect->removeParameter( mMetalnessParameter );
  }
  else
  {
    layers.removeAll( QStringLiteral( "metalnessMap" ) );
    layers.append( QStringLiteral( "metalness" ) );
    if ( mMetalRoughEffect->parameters().contains( mMetalnessMapParameter ) )
      mMetalRoughEffect->removeParameter( mMetalnessMapParameter );
    mMetalRoughEffect->addParameter( mMetalnessParameter );
  }
  updateLayersOnTechnique( layers );
}

void QgsMetalRoughMaterial::setRoughness( const QVariant &roughness )
{
  mRoughnessParameter->setValue( roughness );
  mRoughnessMapParameter->setValue( roughness );

  auto layers = mMetalRoughGL3ShaderBuilder->enabledLayers();
  if ( roughness.value<Qt3DRender::QAbstractTexture *>() )
  {
    layers.removeAll( QStringLiteral( "roughness" ) );
    layers.append( QStringLiteral( "roughnessMap" ) );
    mMetalRoughEffect->addParameter( mRoughnessMapParameter );
    if ( mMetalRoughEffect->parameters().contains( mRoughnessParameter ) )
      mMetalRoughEffect->removeParameter( mRoughnessParameter );
  }
  else
  {
    layers.removeAll( QStringLiteral( "roughnessMap" ) );
    layers.append( QStringLiteral( "roughness" ) );
    if ( mMetalRoughEffect->parameters().contains( mRoughnessMapParameter ) )
      mMetalRoughEffect->removeParameter( mRoughnessMapParameter );
    mMetalRoughEffect->addParameter( mRoughnessParameter );
  }
  updateLayersOnTechnique( layers );
}

void QgsMetalRoughMaterial::setAmbientOcclusion( const QVariant &ambientOcclusion )
{
  mAmbientOcclusionMapParameter->setValue( ambientOcclusion );

  auto layers = mMetalRoughGL3ShaderBuilder->enabledLayers();
  if ( ambientOcclusion.value<Qt3DRender::QAbstractTexture *>() )
  {
    layers.removeAll( QStringLiteral( "ambientOcclusion" ) );
    layers.append( QStringLiteral( "ambientOcclusionMap" ) );
    mMetalRoughEffect->addParameter( mAmbientOcclusionMapParameter );
  }
  else
  {
    layers.removeAll( QStringLiteral( "ambientOcclusionMap" ) );
    layers.append( QStringLiteral( "ambientOcclusion" ) );
    if ( mMetalRoughEffect->parameters().contains( mAmbientOcclusionMapParameter ) )
      mMetalRoughEffect->removeParameter( mAmbientOcclusionMapParameter );
  }
  updateLayersOnTechnique( layers );
}

void QgsMetalRoughMaterial::setNormal( const QVariant &normal )
{
  mNormalMapParameter->setValue( normal );

  auto layers = mMetalRoughGL3ShaderBuilder->enabledLayers();
  if ( normal.value<Qt3DRender::QAbstractTexture *>() )
  {
    layers.removeAll( QStringLiteral( "normal" ) );
    layers.append( QStringLiteral( "normalMap" ) );
    mMetalRoughEffect->addParameter( mNormalMapParameter );
  }
  else
  {
    layers.removeAll( QStringLiteral( "normalMap" ) );
    layers.append( QStringLiteral( "normal" ) );
    if ( mMetalRoughEffect->parameters().contains( mNormalMapParameter ) )
      mMetalRoughEffect->removeParameter( mNormalMapParameter );
  }
  updateLayersOnTechnique( layers );
}

void QgsMetalRoughMaterial::setTextureScale( float textureScale )
{
  mTextureScaleParameter->setValue( textureScale );
}

void QgsMetalRoughMaterial::init()
{
  QObject::connect( mBaseColorParameter, &Qt3DRender::QParameter::valueChanged,
                    this, &QgsMetalRoughMaterial::baseColorChanged );
  QObject::connect( mMetalnessParameter, &Qt3DRender::QParameter::valueChanged,
                    this, &QgsMetalRoughMaterial::metalnessChanged );
  QObject::connect( mRoughnessParameter, &Qt3DRender::QParameter::valueChanged,
                    this, &QgsMetalRoughMaterial::roughnessChanged );
  QObject::connect( mAmbientOcclusionMapParameter, &Qt3DRender::QParameter::valueChanged,
                    this, &QgsMetalRoughMaterial::roughnessChanged );
  QObject::connect( mNormalMapParameter, &Qt3DRender::QParameter::valueChanged,
                    this, &QgsMetalRoughMaterial::normalChanged );
  connect( mTextureScaleParameter, &Qt3DRender::QParameter::valueChanged,
           this, &QgsMetalRoughMaterial::handleTextureScaleChanged );

  mMetalRoughGL3Shader->setVertexShaderCode( Qt3DRender::QShaderProgram::loadSource( QUrl( QStringLiteral( "qrc:/shaders/default.vert" ) ) ) );

  mMetalRoughGL3ShaderBuilder->setParent( this );
  mMetalRoughGL3ShaderBuilder->setShaderProgram( mMetalRoughGL3Shader );
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
  mMetalRoughGL3ShaderBuilder->setFragmentShaderGraph( QUrl( QStringLiteral( "qrc:/shaders/metalrough.frag.json" ) ) );
#else
  mMetalRoughGL3ShaderBuilder->setFragmentShaderGraph( QUrl( QStringLiteral( "qrc:/shaders/metalrough.qt6.frag.json" ) ) );
#endif

  mMetalRoughGL3ShaderBuilder->setEnabledLayers( {QStringLiteral( "baseColor" ),
      QStringLiteral( "metalness" ),
      QStringLiteral( "roughness" ),
      QStringLiteral( "ambientOcclusion" ),
      QStringLiteral( "normal" )} );

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

void QgsMetalRoughMaterial::handleTextureScaleChanged( const QVariant &var )
{
  emit textureScaleChanged( var.toFloat() );
}

void QgsMetalRoughMaterial::updateLayersOnTechnique( const QStringList &layers )
{
  mMetalRoughGL3ShaderBuilder->setEnabledLayers( layers );
}
///@endcond PRIVATE
