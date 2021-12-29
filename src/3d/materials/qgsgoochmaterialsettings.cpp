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

#include "qgssymbollayerutils.h"
#include "qgslinematerial_p.h"
#include <Qt3DExtras/QGoochMaterial>
#include <Qt3DRender/QAttribute>
#include <Qt3DRender/QBuffer>
#include <Qt3DRender/QGeometry>
#include <Qt3DRender/QParameter>
#include <Qt3DRender/QEffect>
#include <Qt3DRender/QTechnique>
#include <Qt3DRender/QGraphicsApiFilter>
#include <QUrl>

QString QgsGoochMaterialSettings::type() const
{
  return QStringLiteral( "gooch" );
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

void QgsGoochMaterialSettings::readXml( const QDomElement &elem, const QgsReadWriteContext &context )
{
  mWarm = QgsSymbolLayerUtils::decodeColor( elem.attribute( QStringLiteral( "warm" ), QStringLiteral( "107,0,107" ) ) );
  mCool = QgsSymbolLayerUtils::decodeColor( elem.attribute( QStringLiteral( "cool" ), QStringLiteral( "255,130,0" ) ) );
  mDiffuse = QgsSymbolLayerUtils::decodeColor( elem.attribute( QStringLiteral( "diffuse" ), QStringLiteral( "178,178,178" ) ) );
  mSpecular = QgsSymbolLayerUtils::decodeColor( elem.attribute( QStringLiteral( "specular" ) ) );
  mShininess = elem.attribute( QStringLiteral( "shininess2" ), QStringLiteral( "100" ) ).toFloat();
  mAlpha = elem.attribute( QStringLiteral( "alpha" ), QStringLiteral( "0.25" ) ).toFloat();
  mBeta = elem.attribute( QStringLiteral( "beta" ), QStringLiteral( "0.5" ) ).toFloat();

  QgsAbstractMaterialSettings::readXml( elem, context );
}

void QgsGoochMaterialSettings::writeXml( QDomElement &elem, const QgsReadWriteContext &context ) const
{
  elem.setAttribute( QStringLiteral( "warm" ), QgsSymbolLayerUtils::encodeColor( mWarm ) );
  elem.setAttribute( QStringLiteral( "cool" ), QgsSymbolLayerUtils::encodeColor( mCool ) );
  elem.setAttribute( QStringLiteral( "diffuse" ), QgsSymbolLayerUtils::encodeColor( mDiffuse ) );
  elem.setAttribute( QStringLiteral( "specular" ), QgsSymbolLayerUtils::encodeColor( mSpecular ) );
  elem.setAttribute( QStringLiteral( "shininess2" ), mShininess );
  elem.setAttribute( QStringLiteral( "alpha" ), mAlpha );
  elem.setAttribute( QStringLiteral( "beta" ), mBeta );

  QgsAbstractMaterialSettings::writeXml( elem, context );
}

QMap<QString, QString> QgsGoochMaterialSettings::toExportParameters() const
{
  return QMap<QString, QString>();
}

Qt3DRender::QMaterial *QgsGoochMaterialSettings::toMaterial( QgsMaterialSettingsRenderingTechnique technique, const QgsMaterialContext &context ) const
{
  switch ( technique )
  {
    case QgsMaterialSettingsRenderingTechnique::Triangles:
    case QgsMaterialSettingsRenderingTechnique::TrianglesDataDefined:
    case QgsMaterialSettingsRenderingTechnique::TrianglesWithFixedTexture:
    case QgsMaterialSettingsRenderingTechnique::TrianglesFromModel:
    {
      if ( dataDefinedProperties().hasActiveProperties() )
        return dataDefinedMaterial();
      Qt3DExtras::QGoochMaterial *material  = new Qt3DExtras::QGoochMaterial;
      material->setDiffuse( mDiffuse );
      material->setWarm( mWarm );
      material->setCool( mCool );

      material->setSpecular( mSpecular );
      material->setShininess( mShininess );
      material->setAlpha( mAlpha );
      material->setBeta( mBeta );

      if ( context.isSelected() )
      {
        // update the material with selection colors
        material->setDiffuse( context.selectionColor() );
      }
      return material;
    }

    case QgsMaterialSettingsRenderingTechnique::Lines:
    case QgsMaterialSettingsRenderingTechnique::InstancedPoints:
    case QgsMaterialSettingsRenderingTechnique::Points:
      return nullptr;
  }
  return nullptr;
}

void QgsGoochMaterialSettings::addParametersToEffect( Qt3DRender::QEffect * ) const
{
}

QByteArray QgsGoochMaterialSettings::dataDefinedVertexColorsAsByte( const QgsExpressionContext &expressionContext ) const
{

  const QColor diffuse = dataDefinedProperties().valueAsColor( Diffuse, expressionContext, mDiffuse );
  const QColor warm = dataDefinedProperties().valueAsColor( Warm, expressionContext, mWarm );
  const QColor cool = dataDefinedProperties().valueAsColor( Cool, expressionContext, mCool );
  const QColor specular = dataDefinedProperties().valueAsColor( Specular, expressionContext, mSpecular );


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

void QgsGoochMaterialSettings::applyDataDefinedToGeometry( Qt3DRender::QGeometry *geometry, int vertexCount, const QByteArray &data ) const
{
  Qt3DRender::QBuffer *dataBuffer = new Qt3DRender::QBuffer( geometry );

  Qt3DRender::QAttribute *diffuseAttribute = new Qt3DRender::QAttribute( geometry );
  diffuseAttribute->setName( QStringLiteral( "dataDefinedDiffuseColor" ) );
  diffuseAttribute->setVertexBaseType( Qt3DRender::QAttribute::UnsignedByte );
  diffuseAttribute->setVertexSize( 3 );
  diffuseAttribute->setAttributeType( Qt3DRender::QAttribute::VertexAttribute );
  diffuseAttribute->setBuffer( dataBuffer );
  diffuseAttribute->setByteStride( 12 * sizeof( unsigned char ) );
  diffuseAttribute->setByteOffset( 0 );
  diffuseAttribute->setCount( vertexCount );
  geometry->addAttribute( diffuseAttribute );

  Qt3DRender::QAttribute *warmAttribute = new Qt3DRender::QAttribute( geometry );
  warmAttribute->setName( QStringLiteral( "dataDefinedWarmColor" ) );
  warmAttribute->setVertexBaseType( Qt3DRender::QAttribute::UnsignedByte );
  warmAttribute->setVertexSize( 3 );
  warmAttribute->setAttributeType( Qt3DRender::QAttribute::VertexAttribute );
  warmAttribute->setBuffer( dataBuffer );
  warmAttribute->setByteStride( 12 * sizeof( unsigned char ) );
  warmAttribute->setByteOffset( 3 * sizeof( unsigned char ) );
  warmAttribute->setCount( vertexCount );
  geometry->addAttribute( warmAttribute
                        );

  Qt3DRender::QAttribute *coolAttribute = new Qt3DRender::QAttribute( geometry );
  coolAttribute->setName( QStringLiteral( "dataDefinedCoolColor" ) );
  coolAttribute->setVertexBaseType( Qt3DRender::QAttribute::UnsignedByte );
  coolAttribute->setVertexSize( 3 );
  coolAttribute->setAttributeType( Qt3DRender::QAttribute::VertexAttribute );
  coolAttribute->setBuffer( dataBuffer );
  coolAttribute->setByteStride( 12 * sizeof( unsigned char ) );
  coolAttribute->setByteOffset( 6 * sizeof( unsigned char ) );
  coolAttribute->setCount( vertexCount );
  geometry->addAttribute( coolAttribute );


  Qt3DRender::QAttribute *specularAttribute = new Qt3DRender::QAttribute( geometry );
  specularAttribute->setName( QStringLiteral( "dataDefinedSpecularColor" ) );
  specularAttribute->setVertexBaseType( Qt3DRender::QAttribute::UnsignedByte );
  specularAttribute->setVertexSize( 3 );
  specularAttribute->setAttributeType( Qt3DRender::QAttribute::VertexAttribute );
  specularAttribute->setBuffer( dataBuffer );
  specularAttribute->setByteStride( 12 * sizeof( unsigned char ) );
  specularAttribute->setByteOffset( 9 * sizeof( unsigned char ) );
  specularAttribute->setCount( vertexCount );
  geometry->addAttribute( specularAttribute );

  dataBuffer->setData( data );
}

Qt3DRender::QMaterial *QgsGoochMaterialSettings::dataDefinedMaterial() const
{
  Qt3DRender::QMaterial *material = new Qt3DRender::QMaterial;

  Qt3DRender::QEffect *eff = new Qt3DRender::QEffect( material );

  Qt3DRender::QTechnique *technique = new Qt3DRender::QTechnique;
  technique->graphicsApiFilter()->setApi( Qt3DRender::QGraphicsApiFilter::OpenGL );
  technique->graphicsApiFilter()->setProfile( Qt3DRender::QGraphicsApiFilter::CoreProfile );
  technique->graphicsApiFilter()->setMajorVersion( 3 );
  technique->graphicsApiFilter()->setMinorVersion( 3 );
  Qt3DRender::QFilterKey *filterKey = new Qt3DRender::QFilterKey();
  filterKey->setName( QStringLiteral( "renderingStyle" ) );
  filterKey->setValue( QStringLiteral( "forward" ) );
  technique->addFilterKey( filterKey );

  Qt3DRender::QRenderPass *renderPass = new Qt3DRender::QRenderPass();
  Qt3DRender::QShaderProgram *shaderProgram = new Qt3DRender::QShaderProgram();

  //Load shader programs
  const QUrl urlVert( QStringLiteral( "qrc:/shaders/goochDataDefined.vert" ) );
  shaderProgram->setShaderCode( Qt3DRender::QShaderProgram::Vertex, Qt3DRender::QShaderProgram::loadSource( urlVert ) );
  const QUrl urlFrag( QStringLiteral( "qrc:/shaders/goochDataDefined.frag" ) );
  shaderProgram->setShaderCode( Qt3DRender::QShaderProgram::Fragment, Qt3DRender::QShaderProgram::loadSource( urlFrag ) );

  renderPass->setShaderProgram( shaderProgram );
  technique->addRenderPass( renderPass );

  technique->addParameter( new Qt3DRender::QParameter( QStringLiteral( "shininess" ), mShininess ) );
  technique->addParameter( new Qt3DRender::QParameter( QStringLiteral( "alpha" ), mAlpha ) );
  technique->addParameter( new Qt3DRender::QParameter( QStringLiteral( "beta" ), mBeta ) );

  eff->addTechnique( technique );
  material->setEffect( eff );

  return material;
}
