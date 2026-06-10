/***************************************************************************
    qgshighlightmaterial.cpp
    ---------------------
    begin                : December 2025
    copyright            : (C) 2025 by Stefanos Natsis
    email                : uclaros at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgshighlightmaterial.h"

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

#include "moc_qgshighlightmaterial.cpp"

using namespace Qt::StringLiterals;

///@cond PRIVATE

QgsHighlightMaterial::QgsHighlightMaterial( QNode *parent )
  : QgsMaterial( parent )
  , mTransformParameter( new Qt3DRender::QParameter( u"nodeTransform"_s, QVariant::fromValue( QMatrix4x4() ), this ) )
  , mNormalTransformParameter( new Qt3DRender::QParameter( u"normalTransform"_s, QVariant::fromValue( QMatrix3x3() ), this ) )
{
  init();
}

QgsHighlightMaterial::~QgsHighlightMaterial() = default;

void QgsHighlightMaterial::init()
{
  Qt3DRender::QEffect *effect = new Qt3DRender::QEffect;
  Qt3DRender::QTechnique *technique = new Qt3DRender::QTechnique;
  technique->graphicsApiFilter()->setApi( Qt3DRender::QGraphicsApiFilter::OpenGL );
  technique->graphicsApiFilter()->setProfile( Qt3DRender::QGraphicsApiFilter::CoreProfile );
  technique->graphicsApiFilter()->setMajorVersion( 3 );
  technique->graphicsApiFilter()->setMinorVersion( 3 );

  Qt3DRender::QRenderPass *pass = new Qt3DRender::QRenderPass;

  mShaderProgram = new Qt3DRender::QShaderProgram();
  pass->setShaderProgram( mShaderProgram );

  mShaderProgram->setFragmentShaderCode( Qt3DRender::QShaderProgram::loadSource( QUrl( u"qrc:/shaders/singlecolor.frag"_s ) ) );

  const QgsSettings settings;
  const float alpha = settings.value( u"Map/highlight/colorAlpha"_s, Qgis::DEFAULT_HIGHLIGHT_COLOR.alpha() ).toFloat() / 255.f;
  QColor color = QColor( settings.value( u"Map/highlight/color"_s, Qgis::DEFAULT_HIGHLIGHT_COLOR.name() ).toString() );
  color.setAlphaF( alpha );
  Qt3DRender::QParameter *colorParam = new Qt3DRender::QParameter( u"color"_s, Qgs3DUtils::srgbToLinear( color ) );
  pass->addParameter( colorParam );

  technique->addRenderPass( pass );
  effect->addTechnique( technique );
  effect->addParameter( mTransformParameter );
  effect->addParameter( mNormalTransformParameter );
  setEffect( effect );


  updateShaders();
}

void QgsHighlightMaterial::setInstancingEnabled( bool enabled, Qgis::InstancedMaterialFlags flags )
{
  mInstanced = enabled;
  mInstanceFlags = flags;
  updateShaders();
}

void QgsHighlightMaterial::setInstancingMeshTransform( const QMatrix4x4 &transform )
{
  const QMatrix3x3 normalTransform = transform.normalMatrix();
  mTransformParameter->setValue( QVariant::fromValue( transform ) );
  mNormalTransformParameter->setValue( QVariant::fromValue( normalTransform ) );
}

void QgsHighlightMaterial::updateShaders()
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
  }
  else
  {
    mShaderProgram->setShaderCode( Qt3DRender::QShaderProgram::Vertex, Qt3DRender::QShaderProgram::loadSource( QUrl( u"qrc:/shaders/default.vert"_s ) ) );
  }
}

///@endcond PRIVATE
