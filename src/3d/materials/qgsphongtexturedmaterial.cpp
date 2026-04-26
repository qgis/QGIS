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

#include "qgsphongtexturedmaterial.h"

#include "qgs3dutils.h"

#include <QString>
#include <QUrl>
#include <Qt3DRender/QEffect>
#include <Qt3DRender/QGraphicsApiFilter>
#include <Qt3DRender/QParameter>
#include <Qt3DRender/QTechnique>

#include "moc_qgsphongtexturedmaterial.cpp"

using namespace Qt::StringLiterals;

///@cond PRIVATE
QgsPhongTexturedMaterial::QgsPhongTexturedMaterial( QNode *parent )
  : QgsMaterial( parent )
  , mAmbientParameter( new Qt3DRender::QParameter( u"ambientColor"_s, QVariant() ) )
  , mDiffuseTextureParameter( new Qt3DRender::QParameter( u"diffuseTexture"_s, QVariant() ) )
  , mDiffuseTextureScaleParameter( new Qt3DRender::QParameter( u"texCoordScale"_s, 1.0f ) )
  , mDiffuseTextureRotationParameter( new Qt3DRender::QParameter( u"texCoordRotation"_s, 0.0f ) )
  , mSpecularParameter( new Qt3DRender::QParameter( u"specularColor"_s, QVariant() ) )
  , mShininessParameter( new Qt3DRender::QParameter( u"shininess"_s, 150.0f ) )
  , mOpacityParameter( new Qt3DRender::QParameter( u"opacity"_s, 1.0f ) )
{
  setAmbient( QColor::fromRgbF( 0.05f, 0.05f, 0.05f, 1.0f ) );
  setSpecular( QColor::fromRgbF( 0.01f, 0.01f, 0.01f, 1.0f ) );
  init();
}

QgsPhongTexturedMaterial::~QgsPhongTexturedMaterial() = default;


void QgsPhongTexturedMaterial::init()
{
  Qt3DRender::QEffect *effect = new Qt3DRender::QEffect();
  effect->addParameter( mAmbientParameter );
  effect->addParameter( mDiffuseTextureParameter );
  effect->addParameter( mDiffuseTextureScaleParameter );
  effect->addParameter( mDiffuseTextureRotationParameter );
  effect->addParameter( mSpecularParameter );
  effect->addParameter( mShininessParameter );
  effect->addParameter( mOpacityParameter );

  Qt3DRender::QShaderProgram *gL3Shader = new Qt3DRender::QShaderProgram();

  const QByteArray vertexShaderCode = Qt3DRender::QShaderProgram::loadSource( QUrl( u"qrc:/shaders/default.vert"_s ) );
  const QByteArray finalVertexShaderCode = Qgs3DUtils::addDefinesToShaderCode( vertexShaderCode, QStringList( { "TEXTURE_ROTATION" } ) );
  gL3Shader->setVertexShaderCode( finalVertexShaderCode );

  gL3Shader->setFragmentShaderCode( Qt3DRender::QShaderProgram::loadSource( QUrl( u"qrc:/shaders/diffuseSpecular.frag"_s ) ) );

  Qt3DRender::QTechnique *gL3Technique = new Qt3DRender::QTechnique();
  gL3Technique->graphicsApiFilter()->setApi( Qt3DRender::QGraphicsApiFilter::OpenGL );
  gL3Technique->graphicsApiFilter()->setMajorVersion( 3 );
  gL3Technique->graphicsApiFilter()->setMinorVersion( 1 );
  gL3Technique->graphicsApiFilter()->setProfile( Qt3DRender::QGraphicsApiFilter::CoreProfile );

  Qt3DRender::QFilterKey *filterKey = new Qt3DRender::QFilterKey( this );
  filterKey->setName( u"renderingStyle"_s );
  filterKey->setValue( u"forward"_s );

  gL3Technique->addFilterKey( filterKey );

  Qt3DRender::QRenderPass *gL3RenderPass = new Qt3DRender::QRenderPass();
  gL3RenderPass->setShaderProgram( gL3Shader );
  gL3Technique->addRenderPass( gL3RenderPass );
  effect->addTechnique( gL3Technique );

  setEffect( effect );
}

void QgsPhongTexturedMaterial::setAmbient( const QColor &ambient )
{
  mAmbientParameter->setValue( Qgs3DUtils::srgbToLinear( ambient ) );
}

void QgsPhongTexturedMaterial::setDiffuseTexture( Qt3DRender::QAbstractTexture *diffuseTexture )
{
  mDiffuseTextureParameter->setValue( QVariant::fromValue( diffuseTexture ) );
}

void QgsPhongTexturedMaterial::setDiffuseTextureScale( float diffuseTextureScale )
{
  mDiffuseTextureScaleParameter->setValue( diffuseTextureScale );
}

void QgsPhongTexturedMaterial::setDiffuseTextureRotation( float textureRotation )
{
  mDiffuseTextureRotationParameter->setValue( textureRotation );
}

void QgsPhongTexturedMaterial::setSpecular( const QColor &specular )
{
  mSpecularParameter->setValue( Qgs3DUtils::srgbToLinear( specular ) );
}

void QgsPhongTexturedMaterial::setShininess( float shininess )
{
  mShininessParameter->setValue( shininess );
}

void QgsPhongTexturedMaterial::setOpacity( float opacity )
{
  mOpacityParameter->setValue( opacity );
}

///@endcond PRIVATE
