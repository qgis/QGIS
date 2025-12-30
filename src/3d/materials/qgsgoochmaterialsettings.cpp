/***************************************************************************
  qgsgoochmaterialsettings.cpp
  --------------------------------------
  Date                 : July 2020
  Copyright            : (C) 2020 by Nyall Dawson
  Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsgoochmaterialsettings.h"

#include "qgs3dutils.h"
#include "qgscolorutils.h"

#include <QUrl>
#include <Qt3DCore/QAttribute>
#include <Qt3DCore/QBuffer>
#include <Qt3DCore/QGeometry>
#include <Qt3DExtras/QGoochMaterial>
#include <Qt3DRender/QEffect>
#include <Qt3DRender/QGraphicsApiFilter>
#include <Qt3DRender/QParameter>
#include <Qt3DRender/QTechnique>

QString QgsGoochMaterialSettings::type() const
{
  return u"gooch"_s;
}

QgsAbstractMaterialSettings *QgsGoochMaterialSettings::create()
{
  return new QgsGoochMaterialSettings();
}

bool QgsGoochMaterialSettings::supportsTechnique( QgsMaterialSettingsRenderingTechnique technique )
{
  switch ( technique )
  {
    case QgsMaterialSettingsRenderingTechnique::Triangles:
    case QgsMaterialSettingsRenderingTechnique::TrianglesWithFixedTexture:
    case QgsMaterialSettingsRenderingTechnique::TrianglesFromModel:
    case QgsMaterialSettingsRenderingTechnique::TrianglesDataDefined:
      return true;

    case QgsMaterialSettingsRenderingTechnique::Lines:
    case QgsMaterialSettingsRenderingTechnique::InstancedPoints:
    case QgsMaterialSettingsRenderingTechnique::Points:
      return false;
  }
  return false;
}

QgsGoochMaterialSettings *QgsGoochMaterialSettings::clone() const
{
  return new QgsGoochMaterialSettings( *this );
}

bool QgsGoochMaterialSettings::equals( const QgsAbstractMaterialSettings *other ) const
{
  const QgsGoochMaterialSettings *otherGooch = dynamic_cast<const QgsGoochMaterialSettings *>( other );
  if ( !otherGooch )
    return false;

  return *this == *otherGooch;
}

void QgsGoochMaterialSettings::readXml( const QDomElement &elem, const QgsReadWriteContext &context )
{
  mWarm = QgsColorUtils::colorFromString( elem.attribute( u"warm"_s, u"107,0,107"_s ) );
  mCool = QgsColorUtils::colorFromString( elem.attribute( u"cool"_s, u"255,130,0"_s ) );
  mDiffuse = QgsColorUtils::colorFromString( elem.attribute( u"diffuse"_s, u"178,178,178"_s ) );
  mSpecular = QgsColorUtils::colorFromString( elem.attribute( u"specular"_s ) );
  mShininess = elem.attribute( u"shininess2"_s, u"100"_s ).toDouble();
  mAlpha = elem.attribute( u"alpha"_s, u"0.25"_s ).toDouble();
  mBeta = elem.attribute( u"beta"_s, u"0.5"_s ).toDouble();

  QgsAbstractMaterialSettings::readXml( elem, context );
}

void QgsGoochMaterialSettings::writeXml( QDomElement &elem, const QgsReadWriteContext &context ) const
{
  elem.setAttribute( u"warm"_s, QgsColorUtils::colorToString( mWarm ) );
  elem.setAttribute( u"cool"_s, QgsColorUtils::colorToString( mCool ) );
  elem.setAttribute( u"diffuse"_s, QgsColorUtils::colorToString( mDiffuse ) );
  elem.setAttribute( u"specular"_s, QgsColorUtils::colorToString( mSpecular ) );
  elem.setAttribute( u"shininess2"_s, mShininess );
  elem.setAttribute( u"alpha"_s, mAlpha );
  elem.setAttribute( u"beta"_s, mBeta );

  QgsAbstractMaterialSettings::writeXml( elem, context );
}

QMap<QString, QString> QgsGoochMaterialSettings::toExportParameters() const
{
  return QMap<QString, QString>();
}

QgsMaterial *QgsGoochMaterialSettings::toMaterial( QgsMaterialSettingsRenderingTechnique technique, const QgsMaterialContext &context ) const
{
  switch ( technique )
  {
    case QgsMaterialSettingsRenderingTechnique::Triangles:
    case QgsMaterialSettingsRenderingTechnique::TrianglesDataDefined:
    case QgsMaterialSettingsRenderingTechnique::TrianglesWithFixedTexture:
    case QgsMaterialSettingsRenderingTechnique::TrianglesFromModel:
    {
      return buildMaterial( context );
    }

    case QgsMaterialSettingsRenderingTechnique::Lines:
    case QgsMaterialSettingsRenderingTechnique::InstancedPoints:
    case QgsMaterialSettingsRenderingTechnique::Points:
      return nullptr;
  }
  return nullptr;
}

void QgsGoochMaterialSettings::addParametersToEffect( Qt3DRender::QEffect *, const QgsMaterialContext & ) const
{
}

QByteArray QgsGoochMaterialSettings::dataDefinedVertexColorsAsByte( const QgsExpressionContext &expressionContext ) const
{
  const QColor diffuse = dataDefinedProperties().valueAsColor( QgsAbstractMaterialSettings::Property::Diffuse, expressionContext, mDiffuse );
  const QColor warm = dataDefinedProperties().valueAsColor( QgsAbstractMaterialSettings::Property::Warm, expressionContext, mWarm );
  const QColor cool = dataDefinedProperties().valueAsColor( QgsAbstractMaterialSettings::Property::Cool, expressionContext, mCool );
  const QColor specular = dataDefinedProperties().valueAsColor( QgsAbstractMaterialSettings::Property::Specular, expressionContext, mSpecular );


  QByteArray array;
  array.resize( sizeof( unsigned char ) * 12 );
  unsigned char *fptr = reinterpret_cast<unsigned char *>( array.data() );

  *fptr++ = static_cast<unsigned char>( diffuse.red() );
  *fptr++ = static_cast<unsigned char>( diffuse.green() );
  *fptr++ = static_cast<unsigned char>( diffuse.blue() );

  *fptr++ = static_cast<unsigned char>( warm.red() );
  *fptr++ = static_cast<unsigned char>( warm.green() );
  *fptr++ = static_cast<unsigned char>( warm.blue() );

  *fptr++ = static_cast<unsigned char>( cool.red() );
  *fptr++ = static_cast<unsigned char>( cool.green() );
  *fptr++ = static_cast<unsigned char>( cool.blue() );

  *fptr++ = static_cast<unsigned char>( specular.red() );
  *fptr++ = static_cast<unsigned char>( specular.green() );
  *fptr++ = static_cast<unsigned char>( specular.blue() );

  return array;
}

int QgsGoochMaterialSettings::dataDefinedByteStride() const
{
  return 12 * sizeof( unsigned char );
}

void QgsGoochMaterialSettings::applyDataDefinedToGeometry( Qt3DCore::QGeometry *geometry, int vertexCount, const QByteArray &data ) const
{
  Qt3DCore::QBuffer *dataBuffer = new Qt3DCore::QBuffer( geometry );

  Qt3DCore::QAttribute *diffuseAttribute = new Qt3DCore::QAttribute( geometry );
  diffuseAttribute->setName( u"dataDefinedDiffuseColor"_s );
  diffuseAttribute->setVertexBaseType( Qt3DCore::QAttribute::UnsignedByte );
  diffuseAttribute->setVertexSize( 3 );
  diffuseAttribute->setAttributeType( Qt3DCore::QAttribute::VertexAttribute );
  diffuseAttribute->setBuffer( dataBuffer );
  diffuseAttribute->setByteStride( 12 * sizeof( unsigned char ) );
  diffuseAttribute->setByteOffset( 0 );
  diffuseAttribute->setCount( vertexCount );
  geometry->addAttribute( diffuseAttribute );

  Qt3DCore::QAttribute *warmAttribute = new Qt3DCore::QAttribute( geometry );
  warmAttribute->setName( u"dataDefinedWarmColor"_s );
  warmAttribute->setVertexBaseType( Qt3DCore::QAttribute::UnsignedByte );
  warmAttribute->setVertexSize( 3 );
  warmAttribute->setAttributeType( Qt3DCore::QAttribute::VertexAttribute );
  warmAttribute->setBuffer( dataBuffer );
  warmAttribute->setByteStride( 12 * sizeof( unsigned char ) );
  warmAttribute->setByteOffset( 3 * sizeof( unsigned char ) );
  warmAttribute->setCount( vertexCount );
  geometry->addAttribute( warmAttribute );

  Qt3DCore::QAttribute *coolAttribute = new Qt3DCore::QAttribute( geometry );
  coolAttribute->setName( u"dataDefinedCoolColor"_s );
  coolAttribute->setVertexBaseType( Qt3DCore::QAttribute::UnsignedByte );
  coolAttribute->setVertexSize( 3 );
  coolAttribute->setAttributeType( Qt3DCore::QAttribute::VertexAttribute );
  coolAttribute->setBuffer( dataBuffer );
  coolAttribute->setByteStride( 12 * sizeof( unsigned char ) );
  coolAttribute->setByteOffset( 6 * sizeof( unsigned char ) );
  coolAttribute->setCount( vertexCount );
  geometry->addAttribute( coolAttribute );


  Qt3DCore::QAttribute *specularAttribute = new Qt3DCore::QAttribute( geometry );
  specularAttribute->setName( u"dataDefinedSpecularColor"_s );
  specularAttribute->setVertexBaseType( Qt3DCore::QAttribute::UnsignedByte );
  specularAttribute->setVertexSize( 3 );
  specularAttribute->setAttributeType( Qt3DCore::QAttribute::VertexAttribute );
  specularAttribute->setBuffer( dataBuffer );
  specularAttribute->setByteStride( 12 * sizeof( unsigned char ) );
  specularAttribute->setByteOffset( 9 * sizeof( unsigned char ) );
  specularAttribute->setCount( vertexCount );
  geometry->addAttribute( specularAttribute );

  dataBuffer->setData( data );
}

QgsMaterial *QgsGoochMaterialSettings::buildMaterial( const QgsMaterialContext &context ) const
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

  const QByteArray fragmentShaderCode = Qt3DRender::QShaderProgram::loadSource( QUrl( u"qrc:/shaders/gooch.frag"_s ) );

  if ( dataDefinedProperties().hasActiveProperties() )
  {
    //Load shader programs
    const QUrl urlVert( u"qrc:/shaders/goochDataDefined.vert"_s );
    shaderProgram->setShaderCode( Qt3DRender::QShaderProgram::Vertex, Qt3DRender::QShaderProgram::loadSource( urlVert ) );

    const QByteArray finalFragmentShaderCode = Qgs3DUtils::addDefinesToShaderCode( fragmentShaderCode, QStringList( { "DATA_DEFINED" } ) );
    shaderProgram->setFragmentShaderCode( finalFragmentShaderCode );
  }
  else
  {
    //Load shader programs
    const QUrl urlVert( u"qrc:/shaders/default.vert"_s );
    shaderProgram->setShaderCode( Qt3DRender::QShaderProgram::Vertex, Qt3DRender::QShaderProgram::loadSource( urlVert ) );
    shaderProgram->setFragmentShaderCode( fragmentShaderCode );

    const QColor diffuseColor = context.isSelected() ? context.selectionColor() : mDiffuse;
    effect->addParameter( new Qt3DRender::QParameter( u"kd"_s, diffuseColor ) );
    effect->addParameter( new Qt3DRender::QParameter( u"ks"_s, mSpecular ) );
    effect->addParameter( new Qt3DRender::QParameter( u"kblue"_s, mCool ) );
    effect->addParameter( new Qt3DRender::QParameter( u"kyellow"_s, mWarm ) );
  }

  renderPass->setShaderProgram( shaderProgram );
  technique->addRenderPass( renderPass );

  technique->addParameter( new Qt3DRender::QParameter( u"shininess"_s, mShininess ) );
  technique->addParameter( new Qt3DRender::QParameter( u"alpha"_s, mAlpha ) );
  technique->addParameter( new Qt3DRender::QParameter( u"beta"_s, mBeta ) );

  effect->addTechnique( technique );
  material->setEffect( effect );

  return material;
}
