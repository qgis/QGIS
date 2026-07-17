/***************************************************************************
    qgsunlitmaterial.cpp
    ---------------------
    begin                : June 2026
    copyright            : (C) 2026 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsunlitmaterial.h"

#include "qgs3dutils.h"
#include "qgssettings.h"

#include <QColor>
#include <QString>
#include <QUrl>
#include <Qt3DRender/QEffect>
#include <Qt3DRender/QGraphicsApiFilter>
#include <Qt3DRender/QParameter>
#include <Qt3DRender/QRenderPass>
#include <Qt3DRender/QShaderProgram>
#include <Qt3DRender/QTechnique>

#include "moc_qgsunlitmaterial.cpp"

using namespace Qt::StringLiterals;

///@cond PRIVATE

QgsUnlitMaterial::QgsUnlitMaterial( QNode *parent )
  : QgsMaterial( parent )
  , mTransformParameter( new Qt3DRender::QParameter( u"meshMatrix"_s, QVariant::fromValue( QMatrix4x4() ), this ) )
  , mNormalTransformParameter( new Qt3DRender::QParameter( u"meshNormalMatrix"_s, QVariant::fromValue( QMatrix3x3() ), this ) )
{
  init();
}

QgsUnlitMaterial::~QgsUnlitMaterial() = default;

void QgsUnlitMaterial::setColor( const QColor &color )
{
  const QColor linearColor = Qgs3DUtils::srgbToLinear( color );
  mColorParameter->setValue( linearColor );
}

void QgsUnlitMaterial::init()
{
  Qt3DRender::QEffect *effect = new Qt3DRender::QEffect;
  Qt3DRender::QTechnique *technique = new Qt3DRender::QTechnique;
  technique->graphicsApiFilter()->setApi( Qt3DRender::QGraphicsApiFilter::OpenGL );
  technique->graphicsApiFilter()->setProfile( Qt3DRender::QGraphicsApiFilter::CoreProfile );
  technique->graphicsApiFilter()->setMajorVersion( 4 );
  technique->graphicsApiFilter()->setMinorVersion( 3 );

  Qt3DRender::QRenderPass *pass = new Qt3DRender::QRenderPass;

  mShaderProgram = new Qt3DRender::QShaderProgram();
  pass->setShaderProgram( mShaderProgram );

  mColorParameter = new Qt3DRender::QParameter( u"color"_s, Qgs3DUtils::srgbToLinear( QColor( 255, 255, 255 ) ) );
  pass->addParameter( mColorParameter );

  technique->addRenderPass( pass );
  effect->addTechnique( technique );
  effect->addParameter( mTransformParameter );
  effect->addParameter( mNormalTransformParameter );
  setEffect( effect );

  updateShaders();
}

void QgsUnlitMaterial::setInstancingEnabled( bool enabled, Qgis::InstancedMaterialFlags flags )
{
  mInstanced = enabled;
  mInstanceFlags = flags;
  updateShaders();
}

void QgsUnlitMaterial::setInstancingMeshTransform( const QMatrix4x4 &transform )
{
  const QMatrix3x3 normalTransform = transform.normalMatrix();
  mTransformParameter->setValue( QVariant::fromValue( transform ) );
  mNormalTransformParameter->setValue( QVariant::fromValue( normalTransform ) );
}

void QgsUnlitMaterial::setDataDefinedEnabled( bool enabled )
{
  if ( enabled != mDataDefinedEnabled )
  {
    mDataDefinedEnabled = enabled;
    updateShaders();
  }
}

void QgsUnlitMaterial::updateShaders()
{
  if ( mInstanced )
  {
    QStringList defines;
    if ( mInstanceFlags.testFlag( Qgis::InstancedMaterialFlag::DataDefinedScale ) )
      defines << u"USE_INSTANCE_SCALE"_s;
    if ( mInstanceFlags.testFlag( Qgis::InstancedMaterialFlag::DataDefinedRotation ) )
      defines << u"USE_INSTANCE_ROTATION"_s;
    const QByteArray vertCode = Qt3DRender::QShaderProgram::loadSource( QUrl( u"qrc:/shaders/instanced.vert"_s ) );
    mShaderProgram->setShaderCode( Qt3DRender::QShaderProgram::Vertex, Qgs3DUtils::addDefinesToShaderCode( vertCode, defines ) );
    mShaderProgram->setFragmentShaderCode( Qt3DRender::QShaderProgram::loadSource( QUrl( u"qrc:/shaders/singlecolor.frag"_s ) ) );
  }
  else if ( mDataDefinedEnabled )
  {
    mShaderProgram->setShaderCode( Qt3DRender::QShaderProgram::Vertex, Qt3DRender::QShaderProgram::loadSource( QUrl( u"qrc:/shaders/singlecolorDataDefined.vert"_s ) ) );
    const QByteArray fragCode = Qt3DRender::QShaderProgram::loadSource( QUrl( u"qrc:/shaders/singlecolor.frag"_s ) );
    mShaderProgram->setFragmentShaderCode( Qgs3DUtils::addDefinesToShaderCode( fragCode, QStringList( { u"DATA_DEFINED"_s } ) ) );
  }
  else
  {
    mShaderProgram->setShaderCode( Qt3DRender::QShaderProgram::Vertex, Qt3DRender::QShaderProgram::loadSource( QUrl( u"qrc:/shaders/default.vert"_s ) ) );
    mShaderProgram->setFragmentShaderCode( Qt3DRender::QShaderProgram::loadSource( QUrl( u"qrc:/shaders/singlecolor.frag"_s ) ) );
  }
}

///@endcond PRIVATE
