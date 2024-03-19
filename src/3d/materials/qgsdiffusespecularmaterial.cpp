/***************************************************************************
  qgsdiffusespecularematerial.cpp
  --------------------------------------
  Date                 : March 2024
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
#include <Qt3DRender/QRenderPass>
#include <Qt3DRender/QShaderProgram>
#include <Qt3DRender/QTechnique>
#include <Qt3DRender/QTexture>

#include "qgsdiffusespecularmaterial.h"
#include "qgs3dutils.h"

///@cond PRIVATE
QgsDiffuseSpecularMaterial::QgsDiffuseSpecularMaterial( QNode *parent )
  : QMaterial( parent )
  , mEffect( new Qt3DRender::QEffect() )
  , mAmbientParameter( new Qt3DRender::QParameter( QStringLiteral( "ka" ), QColor::fromRgbF( 0.05f, 0.05f, 0.05f, 1.0f ) ) )
  , mDiffuseParameter( new Qt3DRender::QParameter( QStringLiteral( "diffuse" ), QColor::fromRgbF( 0.7f, 0.7f, 0.7f, 1.0f ) ) )
  , mDiffuseTextureParameter( new Qt3DRender::QParameter( QStringLiteral( "diffuseTexture" ), QVariant() ) )
  , mSpecularParameter( new Qt3DRender::QParameter( QStringLiteral( "ks" ), QColor::fromRgbF( 0.01f, 0.01f, 0.01f, 1.0f ) ) )
  , mShininessParameter( new Qt3DRender::QParameter( QStringLiteral( "shininess" ), 150.0f ) )
  , mGL3Technique( new Qt3DRender::QTechnique( this ) )
  , mGL3RenderPass( new Qt3DRender::QRenderPass( this ) )
  , mGL3Shader( new Qt3DRender::QShaderProgram( this ) )
  , mFilterKey( new Qt3DRender::QFilterKey( this ) )
{
  init();
}

QgsDiffuseSpecularMaterial::~QgsDiffuseSpecularMaterial() = default;


void QgsDiffuseSpecularMaterial::init()
{
  connect( mAmbientParameter, &Qt3DRender::QParameter::valueChanged,
           this, &QgsDiffuseSpecularMaterial::handleAmbientChanged );
  connect( mDiffuseParameter, &Qt3DRender::QParameter::valueChanged,
           this, &QgsDiffuseSpecularMaterial::handleDiffuseChanged );
  connect( mSpecularParameter, &Qt3DRender::QParameter::valueChanged,
           this, &QgsDiffuseSpecularMaterial::handleSpecularChanged );
  connect( mShininessParameter, &Qt3DRender::QParameter::valueChanged,
           this, &QgsDiffuseSpecularMaterial::handleShininessChanged );

  mEffect->addParameter( mAmbientParameter );
  mEffect->addParameter( mDiffuseParameter );
  mEffect->addParameter( mSpecularParameter );
  mEffect->addParameter( mShininessParameter );

  mGL3Shader->setVertexShaderCode( Qt3DRender::QShaderProgram::loadSource( QUrl( QStringLiteral( "qrc:/shaders/default.vert" ) ) ) );

  // this inits the fragment shader code
  setDiffuse( mDiffuseParameter->value() );

  mGL3Technique->graphicsApiFilter()->setApi( Qt3DRender::QGraphicsApiFilter::OpenGL );
  mGL3Technique->graphicsApiFilter()->setMajorVersion( 3 );
  mGL3Technique->graphicsApiFilter()->setMinorVersion( 1 );
  mGL3Technique->graphicsApiFilter()->setProfile( Qt3DRender::QGraphicsApiFilter::CoreProfile );

  mFilterKey->setParent( this );
  mFilterKey->setName( QStringLiteral( "renderingStyle" ) );
  mFilterKey->setValue( QStringLiteral( "forward" ) );

  mGL3Technique->addFilterKey( mFilterKey );
  mGL3RenderPass->setShaderProgram( mGL3Shader );
  mGL3Technique->addRenderPass( mGL3RenderPass );
  mEffect->addTechnique( mGL3Technique );

  setEffect( mEffect );
}

void QgsDiffuseSpecularMaterial::setAmbient( const QColor &ambient )
{
  mAmbientParameter->setValue( ambient );
}

void QgsDiffuseSpecularMaterial::setDiffuse( const QVariant &diffuse )
{
  mDiffuseParameter->setValue( diffuse );
  mDiffuseTextureParameter->setValue( diffuse );

  QByteArray fragmentShaderCode = Qt3DRender::QShaderProgram::loadSource( QUrl( QStringLiteral( "qrc:/shaders/diffusespecular.frag" ) ) );
  QByteArray finalFragShaderCode;

  if ( diffuse.value<Qt3DRender::QAbstractTexture *>() )
  {
    mEffect->addParameter( mDiffuseTextureParameter );
    mEffect->removeParameter( mDiffuseParameter );

    finalFragShaderCode = Qgs3DUtils::addDefinesToShaderCode( fragmentShaderCode, QStringList( {"DIFFUSE_TEXTURE"} ) );
  }
  else
  {
    mEffect->removeParameter( mDiffuseTextureParameter );
    mEffect->addParameter( mDiffuseParameter );
    finalFragShaderCode = fragmentShaderCode;
  }

  mGL3Shader->setFragmentShaderCode( finalFragShaderCode );
}

void QgsDiffuseSpecularMaterial::setSpecular( const QColor &specular )
{
  mSpecularParameter->setValue( specular );
}

void QgsDiffuseSpecularMaterial::setShininess( float shininess )
{
  mShininessParameter->setValue( shininess );
}

QColor QgsDiffuseSpecularMaterial::ambient() const
{
  return mAmbientParameter->value().value<QColor>();
}

QVariant QgsDiffuseSpecularMaterial::diffuse() const
{
  return mDiffuseParameter->value();
}

QColor QgsDiffuseSpecularMaterial::specular() const
{
  return mSpecularParameter->value().value<QColor>();
}

float QgsDiffuseSpecularMaterial::shininess() const
{
  return mShininessParameter->value().toFloat();
}

void QgsDiffuseSpecularMaterial::handleAmbientChanged( const QVariant &var )
{
  emit ambientChanged( var.value<QColor>() );
}

void QgsDiffuseSpecularMaterial::handleDiffuseChanged( const QVariant &var )
{
  emit diffuseChanged( var );
}

void QgsDiffuseSpecularMaterial::handleSpecularChanged( const QVariant &var )
{
  emit specularChanged( var.value<QColor>() );
}

void QgsDiffuseSpecularMaterial::handleShininessChanged( const QVariant &var )
{
  emit shininessChanged( var.toFloat() );
}

///@endcond PRIVATE
