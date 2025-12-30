/***************************************************************************
  qgsphongmaterialsettings.cpp
  --------------------------------------
  Date                 : July 2017
  Copyright            : (C) 2017 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsphongmaterialsettings.h"

#include "qgs3dutils.h"
#include "qgscolorutils.h"

#include <QMap>
#include <Qt3DCore/QAttribute>
#include <Qt3DCore/QBuffer>
#include <Qt3DCore/QGeometry>
#include <Qt3DRender/QEffect>
#include <Qt3DRender/QGraphicsApiFilter>
#include <Qt3DRender/QParameter>
#include <Qt3DRender/QTechnique>

QString QgsPhongMaterialSettings::type() const
{
  return u"phong"_s;
}

bool QgsPhongMaterialSettings::supportsTechnique( QgsMaterialSettingsRenderingTechnique technique )
{
  switch ( technique )
  {
    case QgsMaterialSettingsRenderingTechnique::Triangles:
    case QgsMaterialSettingsRenderingTechnique::InstancedPoints:
    case QgsMaterialSettingsRenderingTechnique::Points:
    case QgsMaterialSettingsRenderingTechnique::TrianglesWithFixedTexture:
    case QgsMaterialSettingsRenderingTechnique::TrianglesFromModel:
    case QgsMaterialSettingsRenderingTechnique::TrianglesDataDefined:
      return true;

    case QgsMaterialSettingsRenderingTechnique::Lines:
      return false;
  }
  return false;
}

QgsAbstractMaterialSettings *QgsPhongMaterialSettings::create()
{
  return new QgsPhongMaterialSettings();
}

QgsPhongMaterialSettings *QgsPhongMaterialSettings::clone() const
{
  return new QgsPhongMaterialSettings( *this );
}

bool QgsPhongMaterialSettings::equals( const QgsAbstractMaterialSettings *other ) const
{
  const QgsPhongMaterialSettings *otherPhong = dynamic_cast<const QgsPhongMaterialSettings *>( other );
  if ( !otherPhong )
    return false;

  return *this == *otherPhong;
}

void QgsPhongMaterialSettings::readXml( const QDomElement &elem, const QgsReadWriteContext &context )
{
  mAmbient = QgsColorUtils::colorFromString( elem.attribute( u"ambient"_s, u"25,25,25"_s ) );
  mDiffuse = QgsColorUtils::colorFromString( elem.attribute( u"diffuse"_s, u"178,178,178"_s ) );
  mSpecular = QgsColorUtils::colorFromString( elem.attribute( u"specular"_s, u"255,255,255"_s ) );
  mShininess = elem.attribute( u"shininess"_s ).toDouble();
  mOpacity = elem.attribute( u"opacity"_s, u"1.0"_s ).toDouble();
  mAmbientCoefficient = elem.attribute( u"ka"_s, u"1.0"_s ).toDouble();
  mDiffuseCoefficient = elem.attribute( u"kd"_s, u"1.0"_s ).toDouble();
  mSpecularCoefficient = elem.attribute( u"ks"_s, u"1.0"_s ).toDouble();

  QgsAbstractMaterialSettings::readXml( elem, context );
}

void QgsPhongMaterialSettings::writeXml( QDomElement &elem, const QgsReadWriteContext &context ) const
{
  elem.setAttribute( u"ambient"_s, QgsColorUtils::colorToString( mAmbient ) );
  elem.setAttribute( u"diffuse"_s, QgsColorUtils::colorToString( mDiffuse ) );
  elem.setAttribute( u"specular"_s, QgsColorUtils::colorToString( mSpecular ) );
  elem.setAttribute( u"shininess"_s, mShininess );
  elem.setAttribute( u"opacity"_s, mOpacity );
  elem.setAttribute( u"ka"_s, mAmbientCoefficient );
  elem.setAttribute( u"kd"_s, mDiffuseCoefficient );
  elem.setAttribute( u"ks"_s, mSpecularCoefficient );

  QgsAbstractMaterialSettings::writeXml( elem, context );
}


QgsMaterial *QgsPhongMaterialSettings::toMaterial( QgsMaterialSettingsRenderingTechnique technique, const QgsMaterialContext &context ) const
{
  switch ( technique )
  {
    case QgsMaterialSettingsRenderingTechnique::Triangles:
    case QgsMaterialSettingsRenderingTechnique::InstancedPoints:
    case QgsMaterialSettingsRenderingTechnique::Points:
    case QgsMaterialSettingsRenderingTechnique::TrianglesDataDefined:
    case QgsMaterialSettingsRenderingTechnique::TrianglesWithFixedTexture:
    case QgsMaterialSettingsRenderingTechnique::TrianglesFromModel:
    {
      return buildMaterial( context );
    }

    case QgsMaterialSettingsRenderingTechnique::Lines:
      return nullptr;
  }
  return nullptr;
}

QMap<QString, QString> QgsPhongMaterialSettings::toExportParameters() const
{
  QMap<QString, QString> parameters;
  parameters[u"Kd"_s] = u"%1 %2 %3"_s.arg( mDiffuse.redF() ).arg( mDiffuse.greenF() ).arg( mDiffuse.blueF() );
  parameters[u"Ka"_s] = u"%1 %2 %3"_s.arg( mAmbient.redF() ).arg( mAmbient.greenF() ).arg( mAmbient.blueF() );
  parameters[u"Ks"_s] = u"%1 %2 %3"_s.arg( mSpecular.redF() ).arg( mSpecular.greenF() ).arg( mSpecular.blueF() );
  parameters[u"Ns"_s] = QString::number( mShininess );
  return parameters;
}

void QgsPhongMaterialSettings::addParametersToEffect( Qt3DRender::QEffect *effect, const QgsMaterialContext &materialContext ) const
{
  const QColor ambient = materialContext.isSelected() ? materialContext.selectionColor().darker() : mAmbient;
  const QColor diffuse = materialContext.isSelected() ? materialContext.selectionColor() : mDiffuse;

  Qt3DRender::QParameter *ambientParameter = new Qt3DRender::QParameter( u"ambientColor"_s, QColor::fromRgbF( ambient.redF() * mAmbientCoefficient, ambient.greenF() * mAmbientCoefficient, ambient.blueF() * mAmbientCoefficient ) );
  Qt3DRender::QParameter *diffuseParameter = new Qt3DRender::QParameter( u"diffuseColor"_s, QColor::fromRgbF( diffuse.redF() * mDiffuseCoefficient, diffuse.greenF() * mDiffuseCoefficient, diffuse.blueF() * mDiffuseCoefficient ) );
  Qt3DRender::QParameter *specularParameter = new Qt3DRender::QParameter( u"specularColor"_s, QColor::fromRgbF( mSpecular.redF() * mSpecularCoefficient, mSpecular.greenF() * mSpecularCoefficient, mSpecular.blueF() * mSpecularCoefficient ) );
  Qt3DRender::QParameter *shininessParameter = new Qt3DRender::QParameter( u"shininess"_s, static_cast<float>( mShininess ) );
  Qt3DRender::QParameter *opacityParameter = new Qt3DRender::QParameter( u"opacity"_s, static_cast<float>( mOpacity ) );

  effect->addParameter( ambientParameter );
  effect->addParameter( diffuseParameter );
  effect->addParameter( specularParameter );
  effect->addParameter( shininessParameter );
  effect->addParameter( opacityParameter );
}

QByteArray QgsPhongMaterialSettings::dataDefinedVertexColorsAsByte( const QgsExpressionContext &expressionContext ) const
{
  const QColor ambient = dataDefinedProperties().valueAsColor( QgsAbstractMaterialSettings::Property::Ambient, expressionContext, mAmbient );
  const QColor diffuse = dataDefinedProperties().valueAsColor( QgsAbstractMaterialSettings::Property::Diffuse, expressionContext, mDiffuse );
  const QColor specular = dataDefinedProperties().valueAsColor( QgsAbstractMaterialSettings::Property::Specular, expressionContext, mSpecular );

  QByteArray array;
  if ( mDiffuseCoefficient < 1 || mAmbientCoefficient < 1 || mSpecularCoefficient < 1 )
  {
    // use floats if we are adjusting color component strength, bytes don't
    // give us enough precision
    array.resize( sizeof( float ) * 9 );
    float *fptr = reinterpret_cast<float *>( array.data() );

    *fptr++ = static_cast<float>( diffuse.redF() * mDiffuseCoefficient );
    *fptr++ = static_cast<float>( diffuse.greenF() * mDiffuseCoefficient );
    *fptr++ = static_cast<float>( diffuse.blueF() * mDiffuseCoefficient );

    *fptr++ = static_cast<float>( ambient.redF() * mAmbientCoefficient );
    *fptr++ = static_cast<float>( ambient.greenF() * mAmbientCoefficient );
    *fptr++ = static_cast<float>( ambient.blueF() * mAmbientCoefficient );

    *fptr++ = static_cast<float>( specular.redF() * mSpecularCoefficient );
    *fptr++ = static_cast<float>( specular.greenF() * mSpecularCoefficient );
    *fptr++ = static_cast<float>( specular.blueF() * mSpecularCoefficient );
  }
  else
  {
    array.resize( sizeof( unsigned char ) * 9 );
    unsigned char *ptr = reinterpret_cast<unsigned char *>( array.data() );

    *ptr++ = static_cast<unsigned char>( diffuse.red() );
    *ptr++ = static_cast<unsigned char>( diffuse.green() );
    *ptr++ = static_cast<unsigned char>( diffuse.blue() );

    *ptr++ = static_cast<unsigned char>( ambient.red() );
    *ptr++ = static_cast<unsigned char>( ambient.green() );
    *ptr++ = static_cast<unsigned char>( ambient.blue() );

    *ptr++ = static_cast<unsigned char>( specular.red() );
    *ptr++ = static_cast<unsigned char>( specular.green() );
    *ptr++ = static_cast<unsigned char>( specular.blue() );
  }

  return array;
}

int QgsPhongMaterialSettings::dataDefinedByteStride() const { return 9 * sizeof( unsigned char ); }

void QgsPhongMaterialSettings::applyDataDefinedToGeometry( Qt3DCore::QGeometry *geometry, int vertexCount, const QByteArray &data ) const
{
  Qt3DCore::QBuffer *dataBuffer = new Qt3DCore::QBuffer( geometry );

  // use floats if we are adjusting color component strength, bytes don't
  // give us enough precision
  const bool useFloats = mDiffuseCoefficient < 1 || mAmbientCoefficient < 1 || mSpecularCoefficient < 1;

  Qt3DCore::QAttribute *diffuseAttribute = new Qt3DCore::QAttribute( geometry );
  diffuseAttribute->setName( u"dataDefinedDiffuseColor"_s );
  diffuseAttribute->setVertexBaseType( useFloats ? Qt3DCore::QAttribute::Float : Qt3DCore::QAttribute::UnsignedByte );
  diffuseAttribute->setVertexSize( 3 );
  diffuseAttribute->setAttributeType( Qt3DCore::QAttribute::VertexAttribute );
  diffuseAttribute->setBuffer( dataBuffer );
  diffuseAttribute->setByteStride( 9 * ( useFloats ? sizeof( float ) : sizeof( unsigned char ) ) );
  diffuseAttribute->setByteOffset( 0 );
  diffuseAttribute->setCount( vertexCount );
  geometry->addAttribute( diffuseAttribute );

  Qt3DCore::QAttribute *ambientAttribute = new Qt3DCore::QAttribute( geometry );
  ambientAttribute->setName( u"dataDefinedAmbiantColor"_s );
  ambientAttribute->setVertexBaseType( useFloats ? Qt3DCore::QAttribute::Float : Qt3DCore::QAttribute::UnsignedByte );
  ambientAttribute->setVertexSize( 3 );
  ambientAttribute->setAttributeType( Qt3DCore::QAttribute::VertexAttribute );
  ambientAttribute->setBuffer( dataBuffer );
  ambientAttribute->setByteStride( 9 * ( useFloats ? sizeof( float ) : sizeof( unsigned char ) ) );
  ambientAttribute->setByteOffset( 3 * ( useFloats ? sizeof( float ) : sizeof( unsigned char ) ) );
  ambientAttribute->setCount( vertexCount );
  geometry->addAttribute( ambientAttribute );

  Qt3DCore::QAttribute *specularAttribute = new Qt3DCore::QAttribute( geometry );
  specularAttribute->setName( u"dataDefinedSpecularColor"_s );
  specularAttribute->setVertexBaseType( useFloats ? Qt3DCore::QAttribute::Float : Qt3DCore::QAttribute::UnsignedByte );
  specularAttribute->setVertexSize( 3 );
  specularAttribute->setAttributeType( Qt3DCore::QAttribute::VertexAttribute );
  specularAttribute->setBuffer( dataBuffer );
  specularAttribute->setByteStride( 9 * ( useFloats ? sizeof( float ) : sizeof( unsigned char ) ) );
  specularAttribute->setByteOffset( 6 * ( useFloats ? sizeof( float ) : sizeof( unsigned char ) ) );
  specularAttribute->setCount( vertexCount );
  geometry->addAttribute( specularAttribute );

  dataBuffer->setData( data );
}

QgsMaterial *QgsPhongMaterialSettings::buildMaterial( const QgsMaterialContext &context ) const
{
  QgsMaterial *material = new QgsMaterial;

  Qt3DRender::QEffect *effect = new Qt3DRender::QEffect( material );

  Qt3DRender::QTechnique *technique = new Qt3DRender::QTechnique;
  technique->graphicsApiFilter()->setApi( Qt3DRender::QGraphicsApiFilter::OpenGL );
  technique->graphicsApiFilter()->setProfile( Qt3DRender::QGraphicsApiFilter::CoreProfile );
  technique->graphicsApiFilter()->setMajorVersion( 3 );
  technique->graphicsApiFilter()->setMinorVersion( 3 );
  Qt3DRender::QFilterKey *filterKey = new Qt3DRender::QFilterKey();
  filterKey->setName( u"renderingStyle"_s );
  filterKey->setValue( u"forward"_s );
  technique->addFilterKey( filterKey );

  Qt3DRender::QRenderPass *renderPass = new Qt3DRender::QRenderPass();
  Qt3DRender::QShaderProgram *shaderProgram = new Qt3DRender::QShaderProgram();

  renderPass->setShaderProgram( shaderProgram );
  technique->addRenderPass( renderPass );

  const QByteArray fragmentShaderCode = Qt3DRender::QShaderProgram::loadSource( QUrl( u"qrc:/shaders/phong.frag"_s ) );

  if ( dataDefinedProperties().hasActiveProperties() )
  {
    // Load shader programs
    const QUrl urlVert( u"qrc:/shaders/phongDataDefined.vert"_s );
    shaderProgram->setShaderCode( Qt3DRender::QShaderProgram::Vertex, Qt3DRender::QShaderProgram::loadSource( urlVert ) );
    const QByteArray finalFragmentShaderCode = Qgs3DUtils::addDefinesToShaderCode( fragmentShaderCode, QStringList( { "DATA_DEFINED" } ) );
    shaderProgram->setFragmentShaderCode( finalFragmentShaderCode );
  }
  else
  {
    // Load shader programs
    const QUrl urlVert( u"qrc:/shaders/default.vert"_s );
    shaderProgram->setShaderCode( Qt3DRender::QShaderProgram::Vertex, Qt3DRender::QShaderProgram::loadSource( urlVert ) );
    shaderProgram->setFragmentShaderCode( fragmentShaderCode );

    const QColor ambient = context.isSelected() ? context.selectionColor().darker() : mAmbient;
    const QColor diffuse = context.isSelected() ? context.selectionColor() : mDiffuse;

    effect->addParameter( new Qt3DRender::QParameter( u"ambientColor"_s, QColor::fromRgbF( ambient.redF() * mAmbientCoefficient, ambient.greenF() * mAmbientCoefficient, ambient.blueF() * mAmbientCoefficient ) ) );
    effect->addParameter( new Qt3DRender::QParameter( u"diffuseColor"_s, QColor::fromRgbF( diffuse.redF() * mDiffuseCoefficient, diffuse.greenF() * mDiffuseCoefficient, diffuse.blueF() * mDiffuseCoefficient ) ) );
    effect->addParameter( new Qt3DRender::QParameter( u"specularColor"_s, QColor::fromRgbF( mSpecular.redF() * mSpecularCoefficient, mSpecular.greenF() * mSpecularCoefficient, mSpecular.blueF() * mSpecularCoefficient ) ) );
  }

  effect->addParameter( new Qt3DRender::QParameter( u"shininess"_s, static_cast<float>( mShininess ) ) );
  effect->addParameter( new Qt3DRender::QParameter( u"opacity"_s, static_cast<float>( mOpacity ) ) );

  effect->addTechnique( technique );
  material->setEffect( effect );

  return material;
}
