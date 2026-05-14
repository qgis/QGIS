/***************************************************************************
  qgsphongmaterial.cpp
  --------------------------------------
  Date                 : April 2026
  Copyright            : (C) 2026 by Dominik Cindrić
  Email                : viper dot miniq at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsphongmaterial.h"

#include "qgs3dutils.h"

#include <QString>
#include <QUrl>
#include <Qt3DRender/QEffect>
#include <Qt3DRender/QFilterKey>
#include <Qt3DRender/QGraphicsApiFilter>
#include <Qt3DRender/QParameter>
#include <Qt3DRender/QRenderPass>
#include <Qt3DRender/QShaderProgram>
#include <Qt3DRender/QTechnique>

#include "moc_qgsphongmaterial.cpp"

using namespace Qt::StringLiterals;

///@cond PRIVATE
QgsPhongMaterial::QgsPhongMaterial( QNode *parent )
  : QgsMaterial( parent )
  , mAmbientParameter( new Qt3DRender::QParameter( u"ambientColor"_s, QVariant() ) )
  , mDiffuseParameter( new Qt3DRender::QParameter( u"diffuseColor"_s, QVariant() ) )
  , mSpecularParameter( new Qt3DRender::QParameter( u"specularColor"_s, QVariant() ) )
  , mShininessParameter( new Qt3DRender::QParameter( u"shininess"_s, 0.0f ) )
  , mOpacityParameter( new Qt3DRender::QParameter( u"opacity"_s, 1.0f ) )
{
  setAmbient( QColor::fromRgbF( 0.1f, 0.1f, 0.1f, 1.0f ) );
  setDiffuse( QColor::fromRgbF( 0.7f, 0.7f, 0.7f, 1.0f ) );
  setSpecular( QColor::fromRgbF( 1.0f, 1.0f, 1.0f, 1.0f ) );
  init();
}

QgsPhongMaterial::~QgsPhongMaterial() = default;

void QgsPhongMaterial::init()
{
  Qt3DRender::QEffect *effect = new Qt3DRender::QEffect();
  effect->addParameter( mAmbientParameter );
  effect->addParameter( mDiffuseParameter );
  effect->addParameter( mSpecularParameter );
  effect->addParameter( mShininessParameter );
  effect->addParameter( mOpacityParameter );

  mShaderProgram = new Qt3DRender::QShaderProgram();

  Qt3DRender::QRenderPass *renderPass = new Qt3DRender::QRenderPass();
  renderPass->setShaderProgram( mShaderProgram );

  Qt3DRender::QFilterKey *filterKey = new Qt3DRender::QFilterKey();
  filterKey->setName( u"renderingStyle"_s );
  filterKey->setValue( u"forward"_s );

  Qt3DRender::QTechnique *technique = new Qt3DRender::QTechnique();
  technique->graphicsApiFilter()->setApi( Qt3DRender::QGraphicsApiFilter::OpenGL );
  technique->graphicsApiFilter()->setProfile( Qt3DRender::QGraphicsApiFilter::CoreProfile );
  technique->graphicsApiFilter()->setMajorVersion( 3 );
  technique->graphicsApiFilter()->setMinorVersion( 3 );
  technique->addFilterKey( filterKey );
  technique->addRenderPass( renderPass );

  effect->addTechnique( technique );
  setEffect( effect );

  updateShaders();
}

void QgsPhongMaterial::updateShaders()
{
  const QByteArray fragCode = Qt3DRender::QShaderProgram::loadSource( QUrl( u"qrc:/shaders/phong.frag"_s ) );

  if ( mDataDefinedEnabled )
  {
    mShaderProgram->setShaderCode( Qt3DRender::QShaderProgram::Vertex, Qt3DRender::QShaderProgram::loadSource( QUrl( u"qrc:/shaders/phongDataDefined.vert"_s ) ) );
    mShaderProgram->setFragmentShaderCode( Qgs3DUtils::addDefinesToShaderCode( fragCode, QStringList( { u"DATA_DEFINED"_s } ) ) );
  }
  else
  {
    mShaderProgram->setShaderCode( Qt3DRender::QShaderProgram::Vertex, Qt3DRender::QShaderProgram::loadSource( QUrl( u"qrc:/shaders/default.vert"_s ) ) );
    mShaderProgram->setFragmentShaderCode( fragCode );
  }
}

void QgsPhongMaterial::setDataDefinedEnabled( bool enabled )
{
  if ( enabled != mDataDefinedEnabled )
  {
    mDataDefinedEnabled = enabled;
    updateShaders();
  }
}

void QgsPhongMaterial::setAmbient( const QColor &ambient, float scaleFactor )
{
  const QColor color = Qgs3DUtils::srgbToLinear( ambient );
  mAmbientParameter->setValue( QColor::fromRgbF( color.redF() * scaleFactor, color.greenF() * scaleFactor, color.blueF() * scaleFactor ) );
}

void QgsPhongMaterial::setDiffuse( const QColor &diffuse, float scaleFactor )
{
  const QColor color = Qgs3DUtils::srgbToLinear( diffuse );
  mDiffuseParameter->setValue( QColor::fromRgbF( color.redF() * scaleFactor, color.greenF() * scaleFactor, color.blueF() * scaleFactor ) );
}

void QgsPhongMaterial::setSpecular( const QColor &specular, float scaleFactor )
{
  const QColor color = Qgs3DUtils::srgbToLinear( specular );
  mSpecularParameter->setValue( QColor::fromRgbF( color.redF() * scaleFactor, color.greenF() * scaleFactor, color.blueF() * scaleFactor ) );
}

void QgsPhongMaterial::setShininess( float shininess )
{
  mShininessParameter->setValue( shininess );
}

void QgsPhongMaterial::setOpacity( float opacity )
{
  mOpacityParameter->setValue( opacity );
}

///@endcond PRIVATE
