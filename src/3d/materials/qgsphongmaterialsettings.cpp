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
#include "qgscolorutils.h"

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
#include <Qt3DRender/QAttribute>
#include <Qt3DRender/QBuffer>
#include <Qt3DRender/QGeometry>

typedef Qt3DRender::QAttribute Qt3DQAttribute;
typedef Qt3DRender::QBuffer Qt3DQBuffer;
typedef Qt3DRender::QGeometry Qt3DQGeometry;
#else
#include <Qt3DCore/QAttribute>
#include <Qt3DCore/QBuffer>
#include <Qt3DCore/QGeometry>

typedef Qt3DCore::QAttribute Qt3DQAttribute;
typedef Qt3DCore::QBuffer Qt3DQBuffer;
typedef Qt3DCore::QGeometry Qt3DQGeometry;
#endif
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
  mAmbient = QgsColorUtils::colorFromString( elem.attribute( QStringLiteral( "ambient" ), QStringLiteral( "25,25,25" ) ) );
  mDiffuse = QgsColorUtils::colorFromString( elem.attribute( QStringLiteral( "diffuse" ), QStringLiteral( "178,178,178" ) ) );
  mSpecular = QgsColorUtils::colorFromString( elem.attribute( QStringLiteral( "specular" ), QStringLiteral( "255,255,255" ) ) );
  mShininess = elem.attribute( QStringLiteral( "shininess" ) ).toDouble();
  mOpacity = elem.attribute( QStringLiteral( "opacity" ), QStringLiteral( "1.0" ) ).toDouble();
  mAmbientCoefficient = elem.attribute( QStringLiteral( "ka" ), QStringLiteral( "1.0" ) ).toDouble();
  mDiffuseCoefficient = elem.attribute( QStringLiteral( "kd" ), QStringLiteral( "1.0" ) ).toDouble();
  mSpecularCoefficient = elem.attribute( QStringLiteral( "ks" ), QStringLiteral( "1.0" ) ).toDouble();

  QgsAbstractMaterialSettings::readXml( elem, context );
}

void QgsPhongMaterialSettings::writeXml( QDomElement &elem, const QgsReadWriteContext &context ) const
{
  elem.setAttribute( QStringLiteral( "ambient" ), QgsColorUtils::colorToString( mAmbient ) );
  elem.setAttribute( QStringLiteral( "diffuse" ), QgsColorUtils::colorToString( mDiffuse ) );
  elem.setAttribute( QStringLiteral( "specular" ), QgsColorUtils::colorToString( mSpecular ) );
  elem.setAttribute( QStringLiteral( "shininess" ), mShininess );
  elem.setAttribute( QStringLiteral( "opacity" ), mOpacity );
  elem.setAttribute( QStringLiteral( "ka" ), mAmbientCoefficient );
  elem.setAttribute( QStringLiteral( "kd" ), mDiffuseCoefficient );
  elem.setAttribute( QStringLiteral( "ks" ), mSpecularCoefficient );

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
      else
        return constantColorMaterial( context );
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
  Qt3DRender::QParameter *ambientParameter = new Qt3DRender::QParameter( QStringLiteral( "ambientColor" ),
      QColor::fromRgbF( mAmbient.redF() * mAmbientCoefficient,
                        mAmbient.greenF() * mAmbientCoefficient,
                        mAmbient.blueF() * mAmbientCoefficient ) );
  Qt3DRender::QParameter *diffuseParameter = new Qt3DRender::QParameter( QStringLiteral( "diffuseColor" ),
      QColor::fromRgbF( mDiffuse.redF() * mDiffuseCoefficient,
                        mDiffuse.greenF() * mDiffuseCoefficient,
                        mDiffuse.blueF() * mDiffuseCoefficient ) );
  Qt3DRender::QParameter *specularParameter = new Qt3DRender::QParameter( QStringLiteral( "specularColor" ),
      QColor::fromRgbF( mSpecular.redF() * mSpecularCoefficient,
                        mSpecular.greenF() * mSpecularCoefficient,
                        mSpecular.blueF() * mSpecularCoefficient ) );
  Qt3DRender::QParameter *shininessParameter = new Qt3DRender::QParameter( QStringLiteral( "shininess" ), static_cast< float >( mShininess ) );
  Qt3DRender::QParameter *opacityParameter = new Qt3DRender::QParameter( QStringLiteral( "opacity" ), static_cast< float >( mOpacity ) );

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

    *fptr++ =  static_cast<float>( ambient.redF() * mAmbientCoefficient );
    *fptr++ =  static_cast<float>( ambient.greenF() * mAmbientCoefficient );
    *fptr++ =  static_cast<float>( ambient.blueF() * mAmbientCoefficient );

    *fptr++ =  static_cast<float>( specular.redF() * mSpecularCoefficient );
    *fptr++ =  static_cast<float>( specular.greenF() * mSpecularCoefficient );
    *fptr++ =  static_cast<float>( specular.blueF() * mSpecularCoefficient );
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

int QgsPhongMaterialSettings::dataDefinedByteStride() const {return 9 * sizeof( unsigned char );}

void QgsPhongMaterialSettings::applyDataDefinedToGeometry( Qt3DQGeometry *geometry, int vertexCount, const QByteArray &data ) const
{
  Qt3DQBuffer *dataBuffer = new Qt3DQBuffer( geometry );

  // use floats if we are adjusting color component strength, bytes don't
  // give us enough precision
  const bool useFloats = mDiffuseCoefficient < 1 || mAmbientCoefficient < 1 || mSpecularCoefficient < 1;

  Qt3DQAttribute *diffuseAttribute = new Qt3DQAttribute( geometry );
  diffuseAttribute->setName( QStringLiteral( "dataDefinedDiffuseColor" ) );
  diffuseAttribute->setVertexBaseType( useFloats ? Qt3DQAttribute::Float : Qt3DQAttribute::UnsignedByte );
  diffuseAttribute->setVertexSize( 3 );
  diffuseAttribute->setAttributeType( Qt3DQAttribute::VertexAttribute );
  diffuseAttribute->setBuffer( dataBuffer );
  diffuseAttribute->setByteStride( 9 * ( useFloats ? sizeof( float ) : sizeof( unsigned char ) ) );
  diffuseAttribute->setByteOffset( 0 );
  diffuseAttribute->setCount( vertexCount );
  geometry->addAttribute( diffuseAttribute );

  Qt3DQAttribute *ambientAttribute = new Qt3DQAttribute( geometry );
  ambientAttribute->setName( QStringLiteral( "dataDefinedAmbiantColor" ) );
  ambientAttribute->setVertexBaseType( useFloats ? Qt3DQAttribute::Float : Qt3DQAttribute::UnsignedByte );
  ambientAttribute->setVertexSize( 3 );
  ambientAttribute->setAttributeType( Qt3DQAttribute::VertexAttribute );
  ambientAttribute->setBuffer( dataBuffer );
  ambientAttribute->setByteStride( 9 * ( useFloats ? sizeof( float ) : sizeof( unsigned char ) ) );
  ambientAttribute->setByteOffset( 3 * ( useFloats ? sizeof( float ) : sizeof( unsigned char ) ) );
  ambientAttribute->setCount( vertexCount );
  geometry->addAttribute( ambientAttribute );

  Qt3DQAttribute *specularAttribute = new Qt3DQAttribute( geometry );
  specularAttribute->setName( QStringLiteral( "dataDefinedSpecularColor" ) );
  specularAttribute->setVertexBaseType( useFloats ? Qt3DQAttribute::Float : Qt3DQAttribute::UnsignedByte );
  specularAttribute->setVertexSize( 3 );
  specularAttribute->setAttributeType( Qt3DQAttribute::VertexAttribute );
  specularAttribute->setBuffer( dataBuffer );
  specularAttribute->setByteStride( 9 * ( useFloats ? sizeof( float ) : sizeof( unsigned char ) ) );
  specularAttribute->setByteOffset( 6 * ( useFloats ? sizeof( float ) : sizeof( unsigned char ) ) );
  specularAttribute->setCount( vertexCount );
  geometry->addAttribute( specularAttribute );

  dataBuffer->setData( data );
}

Qt3DRender::QMaterial *QgsPhongMaterialSettings::constantColorMaterial( const QgsMaterialContext &context ) const
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
  const QUrl urlVert( QStringLiteral( "qrc:/shaders/phongConstant.vert" ) );
  shaderProgram->setShaderCode( Qt3DRender::QShaderProgram::Vertex, Qt3DRender::QShaderProgram::loadSource( urlVert ) );
  const QUrl urlFrag( QStringLiteral( "qrc:/shaders/phongConstant.frag" ) );
  shaderProgram->setShaderCode( Qt3DRender::QShaderProgram::Fragment, Qt3DRender::QShaderProgram::loadSource( urlFrag ) );

  renderPass->setShaderProgram( shaderProgram );
  technique->addRenderPass( renderPass );

  const QColor ambient = context.isSelected() ? context.selectionColor().darker() : mAmbient;
  const QColor diffuse = context.isSelected() ? context.selectionColor() : mDiffuse;

  eff->addParameter( new Qt3DRender::QParameter( QStringLiteral( "shininess" ),  static_cast< float >( mShininess ) ) );
  eff->addParameter( new Qt3DRender::QParameter( QStringLiteral( "opacity" ),  static_cast< float >( mOpacity ) ) );
  eff->addParameter( new Qt3DRender::QParameter( QStringLiteral( "ambientColor" ),
                     QColor::fromRgbF( ambient.redF() * mAmbientCoefficient,
                                       ambient.greenF() * mAmbientCoefficient,
                                       ambient.blueF() * mAmbientCoefficient ) ) );
  eff->addParameter( new Qt3DRender::QParameter( QStringLiteral( "diffuseColor" ),
                     QColor::fromRgbF( diffuse.redF() * mDiffuseCoefficient,
                                       diffuse.greenF() * mDiffuseCoefficient,
                                       diffuse.blueF() * mDiffuseCoefficient ) ) );
  eff->addParameter( new Qt3DRender::QParameter( QStringLiteral( "specularColor" ),
                     QColor::fromRgbF( mSpecular.redF() * mSpecularCoefficient,
                                       mSpecular.greenF() * mSpecularCoefficient,
                                       mSpecular.blueF() * mSpecularCoefficient ) ) );

  eff->addTechnique( technique );
  material->setEffect( eff );

  return material;
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

  eff->addParameter( new Qt3DRender::QParameter( QStringLiteral( "shininess" ),  static_cast< float >( mShininess ) ) );
  eff->addParameter( new Qt3DRender::QParameter( QStringLiteral( "opacity" ), static_cast< float >( mOpacity ) ) );

  eff->addTechnique( technique );
  material->setEffect( eff );

  return material;
}
