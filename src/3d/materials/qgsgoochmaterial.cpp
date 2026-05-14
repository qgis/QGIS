/***************************************************************************
  qgsgoochmaterial.cpp
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

#include "qgsgoochmaterial.h"

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

#include "moc_qgsgoochmaterial.cpp"

using namespace Qt::StringLiterals;

///@cond PRIVATE
QgsGoochMaterial::QgsGoochMaterial( QNode *parent )
  : QgsMaterial( parent )
  , mDiffuseParameter( new Qt3DRender::QParameter( u"kd"_s, QVariant() ) )
  , mSpecularParameter( new Qt3DRender::QParameter( u"ks"_s, QVariant() ) )
  , mWarmParameter( new Qt3DRender::QParameter( u"kyellow"_s, QVariant() ) )
  , mCoolParameter( new Qt3DRender::QParameter( u"kblue"_s, QVariant() ) )
  , mShininessParameter( new Qt3DRender::QParameter( u"shininess"_s, 100.0f ) )
  , mAlphaParameter( new Qt3DRender::QParameter( u"alpha"_s, 0.25f ) )
  , mBetaParameter( new Qt3DRender::QParameter( u"beta"_s, 0.5f ) )
{
  setDiffuse( QColor::fromRgbF( 0.7f, 0.7f, 0.7f, 1.0f ) );
  setSpecular( QColor::fromRgbF( 1.0f, 1.0f, 1.0f, 1.0f ) );
  setWarm( QColor::fromRgbF( 0.42f, 0.0f, 0.42f, 1.0f ) );
  setCool( QColor::fromRgbF( 1.0f, 0.51f, 0.0f, 1.0f ) );
  init();
}

QgsGoochMaterial::~QgsGoochMaterial() = default;

void QgsGoochMaterial::init()
{
  Qt3DRender::QEffect *effect = new Qt3DRender::QEffect();
  effect->addParameter( mDiffuseParameter );
  effect->addParameter( mSpecularParameter );
  effect->addParameter( mWarmParameter );
  effect->addParameter( mCoolParameter );
  effect->addParameter( mShininessParameter );
  effect->addParameter( mAlphaParameter );
  effect->addParameter( mBetaParameter );

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

void QgsGoochMaterial::updateShaders()
{
  const QByteArray fragCode = Qt3DRender::QShaderProgram::loadSource( QUrl( u"qrc:/shaders/gooch.frag"_s ) );

  if ( mDataDefinedEnabled )
  {
    mShaderProgram->setShaderCode( Qt3DRender::QShaderProgram::Vertex, Qt3DRender::QShaderProgram::loadSource( QUrl( u"qrc:/shaders/goochDataDefined.vert"_s ) ) );
    mShaderProgram->setFragmentShaderCode( Qgs3DUtils::addDefinesToShaderCode( fragCode, QStringList( { u"DATA_DEFINED"_s } ) ) );
  }
  else
  {
    mShaderProgram->setShaderCode( Qt3DRender::QShaderProgram::Vertex, Qt3DRender::QShaderProgram::loadSource( QUrl( u"qrc:/shaders/default.vert"_s ) ) );
    mShaderProgram->setFragmentShaderCode( fragCode );
  }
}

void QgsGoochMaterial::setDataDefinedEnabled( bool enabled )
{
  if ( enabled != mDataDefinedEnabled )
  {
    mDataDefinedEnabled = enabled;
    updateShaders();
  }
}

void QgsGoochMaterial::setDiffuse( const QColor &diffuse )
{
  mDiffuseParameter->setValue( Qgs3DUtils::srgbToLinear( diffuse ) );
}

void QgsGoochMaterial::setSpecular( const QColor &specular )
{
  mSpecularParameter->setValue( Qgs3DUtils::srgbToLinear( specular ) );
}

void QgsGoochMaterial::setWarm( const QColor &warm )
{
  mWarmParameter->setValue( Qgs3DUtils::srgbToLinear( warm ) );
}

void QgsGoochMaterial::setCool( const QColor &cool )
{
  mCoolParameter->setValue( Qgs3DUtils::srgbToLinear( cool ) );
}

void QgsGoochMaterial::setShininess( float shininess )
{
  mShininessParameter->setValue( shininess );
}

void QgsGoochMaterial::setAlpha( float alpha )
{
  mAlphaParameter->setValue( alpha );
}

void QgsGoochMaterial::setBeta( float beta )
{
  mBetaParameter->setValue( beta );
}

///@endcond PRIVATE
