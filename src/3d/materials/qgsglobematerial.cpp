/***************************************************************************
  qgsglobematerial.cpp
  --------------------------------------
  Date                 : April 2025
  Copyright            : (C) 2025 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsglobematerial.h"

#include <QUrl>
#include <Qt3DRender/QEffect>
#include <Qt3DRender/QGraphicsApiFilter>
#include <Qt3DRender/QParameter>
#include <Qt3DRender/QRenderPass>
#include <Qt3DRender/QShaderProgram>
#include <Qt3DRender/QTechnique>
#include <Qt3DRender/QTexture>

#include "moc_qgsglobematerial.cpp"

using namespace Qt::StringLiterals;

///@cond PRIVATE
QgsGlobeMaterial::QgsGlobeMaterial( QNode *parent )
  : QgsMaterial( parent )
  , mTextureParameter( new Qt3DRender::QParameter( u"diffuseTexture"_s, new Qt3DRender::QTexture2D ) )
  , mDiffuseTextureScaleParameter( new Qt3DRender::QParameter( u"texCoordScale"_s, 1.0f ) )
  , mGL3Technique( new Qt3DRender::QTechnique( this ) )
  , mGL3RenderPass( new Qt3DRender::QRenderPass( this ) )
  , mGL3Shader( new Qt3DRender::QShaderProgram( this ) )
  , mFilterKey( new Qt3DRender::QFilterKey( this ) )
{
  init();
}

QgsGlobeMaterial::~QgsGlobeMaterial() = default;


void QgsGlobeMaterial::init()
{
  connect( mTextureParameter, &Qt3DRender::QParameter::valueChanged, this, &QgsGlobeMaterial::handleTextureChanged );

  Qt3DRender::QEffect *effect = new Qt3DRender::QEffect();

  effect->addParameter( mTextureParameter );
  effect->addParameter( mDiffuseTextureScaleParameter );

  mGL3Shader->setFragmentShaderCode( Qt3DRender::QShaderProgram::loadSource( QUrl( u"qrc:/shaders/globe.frag"_s ) ) );
  mGL3Shader->setVertexShaderCode( Qt3DRender::QShaderProgram::loadSource( QUrl( u"qrc:/shaders/default.vert"_s ) ) );

  mGL3Technique->graphicsApiFilter()->setApi( Qt3DRender::QGraphicsApiFilter::OpenGL );
  mGL3Technique->graphicsApiFilter()->setMajorVersion( 3 );
  mGL3Technique->graphicsApiFilter()->setMinorVersion( 1 );
  mGL3Technique->graphicsApiFilter()->setProfile( Qt3DRender::QGraphicsApiFilter::CoreProfile );

  mFilterKey->setParent( this );
  mFilterKey->setName( u"renderingStyle"_s );
  mFilterKey->setValue( u"forward"_s );

  mGL3Technique->addFilterKey( mFilterKey );
  mGL3RenderPass->setShaderProgram( mGL3Shader );
  mGL3Technique->addRenderPass( mGL3RenderPass );
  effect->addTechnique( mGL3Technique );

  setEffect( effect );
}

void QgsGlobeMaterial::setTexture( Qt3DRender::QAbstractTexture *texture )
{
  mTextureParameter->setValue( QVariant::fromValue( texture ) );
}

Qt3DRender::QAbstractTexture *QgsGlobeMaterial::texture() const
{
  return mTextureParameter->value().value<Qt3DRender::QAbstractTexture *>();
}

void QgsGlobeMaterial::handleTextureChanged( const QVariant &var )
{
  emit textureChanged( var.value<Qt3DRender::QAbstractTexture *>() );
}

///@endcond PRIVATE
