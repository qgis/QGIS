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

#include "qgssymbollayerutils.h"
#include "qgsapplication.h"
#include "qgsimagecache.h"
#include <Qt3DExtras/QDiffuseSpecularMaterial>
#include <Qt3DRender/QAttribute>
#include <Qt3DRender/QBuffer>
#include <Qt3DRender/QGeometry>
#include <Qt3DRender/QParameter>
#include <Qt3DRender/QEffect>
#include <Qt3DRender/QTechnique>
#include <Qt3DRender/QGraphicsApiFilter>
#include <QMap>


QString QgsPhongMaterialSettings::type() const
{
  return QStringLiteral( "phong" );
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

void QgsPhongMaterialSettings::readXml( const QDomElement &elem, const QgsReadWriteContext &context )
{
  mAmbient = QgsSymbolLayerUtils::decodeColor( elem.attribute( QStringLiteral( "ambient" ), QStringLiteral( "25,25,25" ) ) );
  mDiffuse = QgsSymbolLayerUtils::decodeColor( elem.attribute( QStringLiteral( "diffuse" ), QStringLiteral( "178,178,178" ) ) );
  mSpecular = QgsSymbolLayerUtils::decodeColor( elem.attribute( QStringLiteral( "specular" ), QStringLiteral( "255,255,255" ) ) );
  mShininess = elem.attribute( QStringLiteral( "shininess" ) ).toFloat();
  mOpacity = elem.attribute( QStringLiteral( "opacity" ), QStringLiteral( "1.0" ) ).toFloat();

  QgsAbstractMaterialSettings::readXml( elem, context );
}

void QgsPhongMaterialSettings::writeXml( QDomElement &elem, const QgsReadWriteContext &context ) const
{
  elem.setAttribute( QStringLiteral( "ambient" ), QgsSymbolLayerUtils::encodeColor( mAmbient ) );
  elem.setAttribute( QStringLiteral( "diffuse" ), QgsSymbolLayerUtils::encodeColor( mDiffuse ) );
  elem.setAttribute( QStringLiteral( "specular" ), QgsSymbolLayerUtils::encodeColor( mSpecular ) );
  elem.setAttribute( QStringLiteral( "shininess" ), mShininess );
  elem.setAttribute( QStringLiteral( "opacity" ), mOpacity );

  QgsAbstractMaterialSettings::writeXml( elem, context );
}


Qt3DRender::QMaterial *QgsPhongMaterialSettings::toMaterial( QgsMaterialSettingsRenderingTechnique technique, const QgsMaterialContext &context ) const
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
      if ( dataDefinedProperties().hasActiveProperties() )
        return dataDefinedMaterial();

      int opacity = mOpacity * 255;
      Qt3DExtras::QDiffuseSpecularMaterial *material  = new Qt3DExtras::QDiffuseSpecularMaterial;
      material->setDiffuse( QColor( mDiffuse.red(), mDiffuse.green(), mDiffuse.blue(), opacity ) );
      material->setAmbient( QColor( mAmbient.red(), mAmbient.green(), mAmbient.blue(), opacity ) );
      material->setSpecular( QColor( mSpecular.red(), mSpecular.green(), mSpecular.blue(), opacity ) );
      material->setShininess( mShininess );
      if ( mOpacity != 1 )
      {
        material->setAlphaBlendingEnabled( true );
      }

      if ( context.isSelected() )
      {
        // update the material with selection colors
        material->setDiffuse( context.selectionColor() );
        material->setAmbient( context.selectionColor().darker() );
      }
      return material;
    }

    case QgsMaterialSettingsRenderingTechnique::Lines:
      return nullptr;
  }
  return nullptr;
}

QMap<QString, QString> QgsPhongMaterialSettings::toExportParameters() const
{
  QMap<QString, QString> parameters;
  parameters[ QStringLiteral( "Kd" ) ] = QStringLiteral( "%1 %2 %3" ).arg( mDiffuse.redF() ).arg( mDiffuse.greenF() ).arg( mDiffuse.blueF() );
  parameters[ QStringLiteral( "Ka" ) ] = QStringLiteral( "%1 %2 %3" ).arg( mAmbient.redF() ).arg( mAmbient.greenF() ).arg( mAmbient.blueF() );
  parameters[ QStringLiteral( "Ks" ) ] = QStringLiteral( "%1 %2 %3" ).arg( mSpecular.redF() ).arg( mSpecular.greenF() ).arg( mSpecular.blueF() );
  parameters[ QStringLiteral( "Ns" ) ] = QString::number( mShininess );
  return parameters;
}

void QgsPhongMaterialSettings::addParametersToEffect( Qt3DRender::QEffect *effect ) const
{
  Qt3DRender::QParameter *ambientParameter = new Qt3DRender::QParameter( QStringLiteral( "ka" ), QColor::fromRgbF( 0.05f, 0.05f, 0.05f, 1.0f ) );
  Qt3DRender::QParameter *diffuseParameter = new Qt3DRender::QParameter( QStringLiteral( "kd" ), QColor::fromRgbF( 0.7f, 0.7f, 0.7f, 1.0f ) );
  Qt3DRender::QParameter *specularParameter = new Qt3DRender::QParameter( QStringLiteral( "ks" ), QColor::fromRgbF( 0.01f, 0.01f, 0.01f, 1.0f ) );
  Qt3DRender::QParameter *shininessParameter = new Qt3DRender::QParameter( QStringLiteral( "shininess" ), 150.0f );

  diffuseParameter->setValue( mDiffuse );
  ambientParameter->setValue( mAmbient );
  specularParameter->setValue( mSpecular );
  shininessParameter->setValue( mShininess );

  effect->addParameter( ambientParameter );
  effect->addParameter( diffuseParameter );
  effect->addParameter( specularParameter );
  effect->addParameter( shininessParameter );
}

QByteArray QgsPhongMaterialSettings::dataDefinedVertexColorsAsByte( const QgsExpressionContext &expressionContext ) const
{
  const QColor ambient = dataDefinedProperties().valueAsColor( Ambient, expressionContext, mAmbient );
  const QColor diffuse = dataDefinedProperties().valueAsColor( Diffuse, expressionContext, mDiffuse );
  const QColor specular = dataDefinedProperties().valueAsColor( Specular, expressionContext, mSpecular );

  QByteArray array;
  array.resize( sizeof( unsigned char ) * 9 );
  unsigned char *fptr = reinterpret_cast<unsigned char *>( array.data() );

  *fptr++ = static_cast<unsigned char>( diffuse.red() );
  *fptr++ = static_cast<unsigned char>( diffuse.green() );
  *fptr++ = static_cast<unsigned char>( diffuse.blue() );

  *fptr++ =  static_cast<unsigned char>( ambient.red() );
  *fptr++ =  static_cast<unsigned char>( ambient.green() );
  *fptr++ =  static_cast<unsigned char>( ambient.blue() );

  *fptr++ =  static_cast<unsigned char>( specular.red() );
  *fptr++ =  static_cast<unsigned char>( specular.green() );
  *fptr++ =  static_cast<unsigned char>( specular.blue() );

  return array;
}

int QgsPhongMaterialSettings::dataDefinedByteStride() const {return 9 * sizeof( unsigned char );}

void QgsPhongMaterialSettings::applyDataDefinedToGeometry( Qt3DRender::QGeometry *geometry, int vertexCount, const QByteArray &data ) const
{
  Qt3DRender::QBuffer *dataBuffer = new Qt3DRender::QBuffer( geometry );

  Qt3DRender::QAttribute *diffuseAttribute = new Qt3DRender::QAttribute( geometry );
  diffuseAttribute->setName( QStringLiteral( "dataDefinedDiffuseColor" ) );
  diffuseAttribute->setVertexBaseType( Qt3DRender::QAttribute::UnsignedByte );
  diffuseAttribute->setVertexSize( 3 );
  diffuseAttribute->setAttributeType( Qt3DRender::QAttribute::VertexAttribute );
  diffuseAttribute->setBuffer( dataBuffer );
  diffuseAttribute->setByteStride( 9 * sizeof( unsigned char ) );
  diffuseAttribute->setByteOffset( 0 );
  diffuseAttribute->setCount( vertexCount );
  geometry->addAttribute( diffuseAttribute );

  Qt3DRender::QAttribute *ambientAttribute = new Qt3DRender::QAttribute( geometry );
  ambientAttribute->setName( QStringLiteral( "dataDefinedAmbiantColor" ) );
  ambientAttribute->setVertexBaseType( Qt3DRender::QAttribute::UnsignedByte );
  ambientAttribute->setVertexSize( 3 );
  ambientAttribute->setAttributeType( Qt3DRender::QAttribute::VertexAttribute );
  ambientAttribute->setBuffer( dataBuffer );
  ambientAttribute->setByteStride( 9 * sizeof( unsigned char ) );
  ambientAttribute->setByteOffset( 3 * sizeof( unsigned char ) );
  ambientAttribute->setCount( vertexCount );
  geometry->addAttribute( ambientAttribute );

  Qt3DRender::QAttribute *specularAttribute = new Qt3DRender::QAttribute( geometry );
  specularAttribute->setName( QStringLiteral( "dataDefinedSpecularColor" ) );
  specularAttribute->setVertexBaseType( Qt3DRender::QAttribute::UnsignedByte );
  specularAttribute->setVertexSize( 3 );
  specularAttribute->setAttributeType( Qt3DRender::QAttribute::VertexAttribute );
  specularAttribute->setBuffer( dataBuffer );
  specularAttribute->setByteStride( 9 * sizeof( unsigned char ) );
  specularAttribute->setByteOffset( 6 * sizeof( unsigned char ) );
  specularAttribute->setCount( vertexCount );
  geometry->addAttribute( specularAttribute );

  dataBuffer->setData( data );
}

Qt3DRender::QMaterial *QgsPhongMaterialSettings::dataDefinedMaterial() const
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
  const QUrl urlVert( QStringLiteral( "qrc:/shaders/phongDataDefined.vert" ) );
  shaderProgram->setShaderCode( Qt3DRender::QShaderProgram::Vertex, Qt3DRender::QShaderProgram::loadSource( urlVert ) );
  const QUrl urlFrag( QStringLiteral( "qrc:/shaders/phongDataDefined.frag" ) );
  shaderProgram->setShaderCode( Qt3DRender::QShaderProgram::Fragment, Qt3DRender::QShaderProgram::loadSource( urlFrag ) );

  renderPass->setShaderProgram( shaderProgram );
  technique->addRenderPass( renderPass );

  technique->addParameter( new Qt3DRender::QParameter( QStringLiteral( "shininess" ), mShininess ) );

  eff->addTechnique( technique );
  material->setEffect( eff );

  return material;
}
