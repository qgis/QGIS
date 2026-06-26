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
#include <Qt3DRender/QShaderProgram>
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
  , mDiffuseTextureOffsetParameter( new Qt3DRender::QParameter( u"texCoordOffset"_s, QVariant::fromValue( QVector2D( 0, 0 ) ), this ) )
  , mSpecularParameter( new Qt3DRender::QParameter( u"specularColor"_s, QVariant() ) )
  , mShininessParameter( new Qt3DRender::QParameter( u"shininess"_s, 150.0f ) )
  , mOpacityParameter( new Qt3DRender::QParameter( u"opacity"_s, 1.0f ) )
  , mEffect( new Qt3DRender::QEffect( this ) )
  , mGL3Technique( new Qt3DRender::QTechnique( this ) )
  , mGL3RenderPass( new Qt3DRender::QRenderPass( this ) )
  , mShaderProgram( new Qt3DRender::QShaderProgram( this ) )
  , mFilterKey( new Qt3DRender::QFilterKey( this ) )
  , mTransformParameter( new Qt3DRender::QParameter( u"meshMatrix"_s, QVariant::fromValue( QMatrix4x4() ), this ) )
  , mNormalTransformParameter( new Qt3DRender::QParameter( u"meshNormalMatrix"_s, QVariant::fromValue( QMatrix3x3() ), this ) )
{
  setAmbient( QColor::fromRgbF( 0.05f, 0.05f, 0.05f, 1.0f ) );
  setSpecular( QColor::fromRgbF( 0.01f, 0.01f, 0.01f, 1.0f ) );
  init();
}

QgsPhongTexturedMaterial::~QgsPhongTexturedMaterial() = default;


void QgsPhongTexturedMaterial::init()
{
  updateShaders();

  mGL3Technique->graphicsApiFilter()->setApi( Qt3DRender::QGraphicsApiFilter::OpenGL );
  mGL3Technique->graphicsApiFilter()->setMajorVersion( 3 );
  mGL3Technique->graphicsApiFilter()->setMinorVersion( 1 );
  mGL3Technique->graphicsApiFilter()->setProfile( Qt3DRender::QGraphicsApiFilter::CoreProfile );

  mFilterKey->setParent( this );
  mFilterKey->setName( u"renderingStyle"_s );
  mFilterKey->setValue( u"forward"_s );

  mGL3Technique->addFilterKey( mFilterKey );
  mGL3RenderPass->setShaderProgram( mShaderProgram );
  mGL3Technique->addRenderPass( mGL3RenderPass );
  mEffect->addTechnique( mGL3Technique );

  mShaderProgram->setFragmentShaderCode( Qt3DRender::QShaderProgram::loadSource( QUrl( u"qrc:/shaders/diffuseSpecular.frag"_s ) ) );

  mEffect->addParameter( mAmbientParameter );
  mEffect->addParameter( mDiffuseTextureParameter );
  mEffect->addParameter( mDiffuseTextureScaleParameter );
  mEffect->addParameter( mDiffuseTextureRotationParameter );
  mEffect->addParameter( mDiffuseTextureOffsetParameter );
  mEffect->addParameter( mSpecularParameter );
  mEffect->addParameter( mShininessParameter );
  mEffect->addParameter( mOpacityParameter );
  mEffect->addParameter( mTransformParameter );
  mEffect->addParameter( mNormalTransformParameter );

  setEffect( mEffect );
}

void QgsPhongTexturedMaterial::setInstancingEnabled( bool enabled, Qgis::InstancedMaterialFlags flags )
{
  mInstanced = enabled;
  mInstanceFlags = flags;

  updateShaders();
}

void QgsPhongTexturedMaterial::updateShaders()
{
  const QByteArray fragCode = Qt3DRender::QShaderProgram::loadSource( QUrl( u"qrc:/shaders/diffuseSpecular.frag"_s ) );

  if ( mInstanced )
  {
    QStringList defines = { u"HAS_TEXTURE"_s };
    if ( mInstanceFlags.testFlag( Qgis::InstancedMaterialFlag::DataDefinedScale ) )
      defines << u"USE_INSTANCE_SCALE"_s;
    if ( mInstanceFlags.testFlag( Qgis::InstancedMaterialFlag::DataDefinedRotation ) )
      defines << u"USE_INSTANCE_ROTATION"_s;
    const QByteArray vertCode = Qt3DRender::QShaderProgram::loadSource( QUrl( u"qrc:/shaders/instanced.vert"_s ) );
    mShaderProgram->setVertexShaderCode( Qgs3DUtils::addDefinesToShaderCode( vertCode, defines ) );
  }
  else
  {
    QByteArray vertexCode = Qt3DRender::QShaderProgram::loadSource( QUrl( u"qrc:/shaders/default.vert"_s ) );
    QStringList defines { u"TEXTURE_ROTATION"_s, u"TEXTURE_OFFSET"_s };
    if ( mDataDefinedTextureTransformEnabled )
      defines << u"DATA_DEFINED_TEXTURE_TRANSFORMS"_s;

    vertexCode = Qgs3DUtils::addDefinesToShaderCode( vertexCode, defines );
    mShaderProgram->setVertexShaderCode( vertexCode );
  }
  mShaderProgram->setFragmentShaderCode( fragCode );
}

void QgsPhongTexturedMaterial::setInstancingMeshTransform( const QMatrix4x4 &transform )
{
  const QMatrix3x3 normalTransform = transform.normalMatrix();
  mTransformParameter->setValue( QVariant::fromValue( transform ) );
  mNormalTransformParameter->setValue( QVariant::fromValue( normalTransform ) );
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

void QgsPhongTexturedMaterial::setDiffuseTextureOffset( float textureOffsetX, float textureOffsetY )
{
  mDiffuseTextureOffsetParameter->setValue( QVariant::fromValue( QVector2D( textureOffsetX, textureOffsetY ) ) );
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

void QgsPhongTexturedMaterial::setDataDefinedTextureTransformEnabled( bool enabled )
{
  if ( enabled == mDataDefinedTextureTransformEnabled )
    return;

  mDataDefinedTextureTransformEnabled = enabled;
  updateShaders();
}

///@endcond PRIVATE
