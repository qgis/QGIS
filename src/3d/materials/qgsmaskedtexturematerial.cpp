/***************************************************************************
  qgsmaskedtexturematerial.cpp
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

#include "qgsmaskedtexturematerial.h"

///@cond PRIVATE
QgsMaskedTextureMaterial::QgsMaskedTextureMaterial( QNode *parent )
  : QMaterial( parent )
  , mTextureParameter( new Qt3DRender::QParameter( QStringLiteral( "diffuseTexture" ), new Qt3DRender::QTexture2D ) )
  , mGL3Technique( new Qt3DRender::QTechnique( this ) )
  , mGL3RenderPass( new Qt3DRender::QRenderPass( this ) )
  , mGL3Shader( new Qt3DRender::QShaderProgram( this ) )
  , mFilterKey( new Qt3DRender::QFilterKey( this ) )
{
  init();
}

QgsMaskedTextureMaterial::~QgsMaskedTextureMaterial() = default;


void QgsMaskedTextureMaterial::init()
{
  connect( mTextureParameter, &Qt3DRender::QParameter::valueChanged,
           this, &QgsMaskedTextureMaterial::handleTextureChanged );

  Qt3DRender::QEffect *effect = new Qt3DRender::QEffect();

  effect->addParameter( mTextureParameter );

  mGL3Shader->setFragmentShaderCode( Qt3DRender::QShaderProgram::loadSource( QUrl( QStringLiteral( "qrc:/shaders/maskedtexture.frag" ) ) ) );
  mGL3Shader->setVertexShaderCode( Qt3DRender::QShaderProgram::loadSource( QUrl( QStringLiteral( "qrc:/shaders/maskedtexture.vert" ) ) ) );

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
  effect->addTechnique( mGL3Technique );

  setEffect( effect );
}

void QgsMaskedTextureMaterial::setTexture( Qt3DRender::QAbstractTexture *texture )
{
  mTextureParameter->setValue( QVariant::fromValue( texture ) );
}

Qt3DRender::QAbstractTexture *QgsMaskedTextureMaterial::texture() const
{
  return mTextureParameter->value().value<Qt3DRender::QAbstractTexture *>();
}

void QgsMaskedTextureMaterial::handleTextureChanged( const QVariant &var )
{
  emit textureChanged( var.value<Qt3DRender::QAbstractTexture *>() );
}

///@endcond PRIVATE
