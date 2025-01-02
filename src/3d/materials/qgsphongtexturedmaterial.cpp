/***************************************************************************
  qgsphongtexturedmaterial.cpp
  --------------------------------------
  Date                 : August 2024
  Copyright            : (C) 2024 by Jean Felder
  Email                : jean dot felder at oslandia dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QUrl>

#include <Qt3DRender/QEffect>
#include <Qt3DRender/QGraphicsApiFilter>
#include <Qt3DRender/QParameter>
#include <Qt3DRender/QTechnique>

#include "qgsphongtexturedmaterial.h"
#include "moc_qgsphongtexturedmaterial.cpp"

///@cond PRIVATE
QgsPhongTexturedMaterial::QgsPhongTexturedMaterial( QNode *parent )
  : QgsMaterial( parent )
  , mAmbientParameter( new Qt3DRender::QParameter( QStringLiteral( "ambientColor" ), QColor::fromRgbF( 0.05f, 0.05f, 0.05f, 1.0f ) ) )
  , mDiffuseTextureParameter( new Qt3DRender::QParameter( QStringLiteral( "diffuseTexture" ), QVariant() ) )
  , mDiffuseTextureScaleParameter( new Qt3DRender::QParameter( QStringLiteral( "texCoordScale" ), 1.0f ) )
  , mSpecularParameter( new Qt3DRender::QParameter( QStringLiteral( "specularColor" ), QColor::fromRgbF( 0.01f, 0.01f, 0.01f, 1.0f ) ) )
  , mShininessParameter( new Qt3DRender::QParameter( QStringLiteral( "shininess" ), 150.0f ) )
  , mOpacityParameter( new Qt3DRender::QParameter( QStringLiteral( "opacity" ), 1.0f ) )
{
  init();
}

QgsPhongTexturedMaterial::~QgsPhongTexturedMaterial() = default;


void QgsPhongTexturedMaterial::init()
{
  connect( mAmbientParameter, &Qt3DRender::QParameter::valueChanged, this, &QgsPhongTexturedMaterial::handleAmbientChanged );
  connect( mDiffuseTextureParameter, &Qt3DRender::QParameter::valueChanged, this, &QgsPhongTexturedMaterial::handleDiffuseTextureChanged );
  connect( mDiffuseTextureScaleParameter, &Qt3DRender::QParameter::valueChanged, this, &QgsPhongTexturedMaterial::handleDiffuseTextureScaleChanged );
  connect( mSpecularParameter, &Qt3DRender::QParameter::valueChanged, this, &QgsPhongTexturedMaterial::handleSpecularChanged );
  connect( mShininessParameter, &Qt3DRender::QParameter::valueChanged, this, &QgsPhongTexturedMaterial::handleShininessChanged );
  connect( mOpacityParameter, &Qt3DRender::QParameter::valueChanged, this, &QgsPhongTexturedMaterial::handleOpacityChanged );

  Qt3DRender::QEffect *effect = new Qt3DRender::QEffect();
  effect->addParameter( mAmbientParameter );
  effect->addParameter( mDiffuseTextureParameter );
  effect->addParameter( mDiffuseTextureScaleParameter );
  effect->addParameter( mSpecularParameter );
  effect->addParameter( mShininessParameter );
  effect->addParameter( mOpacityParameter );

  Qt3DRender::QShaderProgram *gL3Shader = new Qt3DRender::QShaderProgram();
  gL3Shader->setVertexShaderCode( Qt3DRender::QShaderProgram::loadSource( QUrl( QStringLiteral( "qrc:/shaders/default.vert" ) ) ) );
  gL3Shader->setFragmentShaderCode( Qt3DRender::QShaderProgram::loadSource( QUrl( QStringLiteral( "qrc:/shaders/diffuseSpecular.frag" ) ) ) );

  Qt3DRender::QTechnique *gL3Technique = new Qt3DRender::QTechnique();
  gL3Technique->graphicsApiFilter()->setApi( Qt3DRender::QGraphicsApiFilter::OpenGL );
  gL3Technique->graphicsApiFilter()->setMajorVersion( 3 );
  gL3Technique->graphicsApiFilter()->setMinorVersion( 1 );
  gL3Technique->graphicsApiFilter()->setProfile( Qt3DRender::QGraphicsApiFilter::CoreProfile );

  Qt3DRender::QFilterKey *filterKey = new Qt3DRender::QFilterKey( this );
  filterKey->setName( QStringLiteral( "renderingStyle" ) );
  filterKey->setValue( QStringLiteral( "forward" ) );

  gL3Technique->addFilterKey( filterKey );

  Qt3DRender::QRenderPass *gL3RenderPass = new Qt3DRender::QRenderPass();
  gL3RenderPass->setShaderProgram( gL3Shader );
  gL3Technique->addRenderPass( gL3RenderPass );
  effect->addTechnique( gL3Technique );

  setEffect( effect );
}

void QgsPhongTexturedMaterial::setAmbient( const QColor &ambient )
{
  mAmbientParameter->setValue( ambient );
}

void QgsPhongTexturedMaterial::setDiffuseTexture( Qt3DRender::QAbstractTexture *diffuseTexture )
{
  mDiffuseTextureParameter->setValue( QVariant::fromValue( diffuseTexture ) );
}

void QgsPhongTexturedMaterial::setDiffuseTextureScale( float diffuseTextureScale )
{
  mDiffuseTextureScaleParameter->setValue( diffuseTextureScale );
}

void QgsPhongTexturedMaterial::setSpecular( const QColor &specular )
{
  mSpecularParameter->setValue( specular );
}

void QgsPhongTexturedMaterial::setShininess( float shininess )
{
  mShininessParameter->setValue( shininess );
}

void QgsPhongTexturedMaterial::setOpacity( float opacity )
{
  mOpacityParameter->setValue( opacity );
}

QColor QgsPhongTexturedMaterial::ambient() const
{
  return mAmbientParameter->value().value<QColor>();
}

Qt3DRender::QAbstractTexture *QgsPhongTexturedMaterial::diffuseTexture() const
{
  return mDiffuseTextureParameter->value().value<Qt3DRender::QAbstractTexture *>();
}

float QgsPhongTexturedMaterial::diffuseTextureScale() const
{
  return mDiffuseTextureScaleParameter->value().toFloat();
}

QColor QgsPhongTexturedMaterial::specular() const
{
  return mSpecularParameter->value().value<QColor>();
}

float QgsPhongTexturedMaterial::shininess() const
{
  return mShininessParameter->value().toFloat();
}

float QgsPhongTexturedMaterial::opacity() const
{
  return mOpacityParameter->value().toFloat();
}

void QgsPhongTexturedMaterial::handleAmbientChanged( const QVariant &var )
{
  emit ambientChanged( var.value<QColor>() );
}

void QgsPhongTexturedMaterial::handleDiffuseTextureChanged( const QVariant &var )
{
  emit diffuseTextureChanged( var.value<Qt3DRender::QAbstractTexture *>() );
}

void QgsPhongTexturedMaterial::handleDiffuseTextureScaleChanged( const QVariant &var )
{
  emit diffuseTextureScaleChanged( var.toFloat() );
}

void QgsPhongTexturedMaterial::handleSpecularChanged( const QVariant &var )
{
  emit specularChanged( var.value<QColor>() );
}

void QgsPhongTexturedMaterial::handleShininessChanged( const QVariant &var )
{
  emit shininessChanged( var.toFloat() );
}

void QgsPhongTexturedMaterial::handleOpacityChanged( const QVariant &var )
{
  emit opacityChanged( var.toFloat() );
}

///@endcond PRIVATE
