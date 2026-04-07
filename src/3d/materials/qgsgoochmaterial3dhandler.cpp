/***************************************************************************
  qgsgoochmaterial3dhandler.cpp
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

#include "qgsgoochmaterial3dhandler.h"

#include "qgs3dutils.h"
#include "qgsgoochmaterialsettings.h"
#include "qgshighlightmaterial.h"

#include <QString>
#include <QUrl>
#include <Qt3DCore/QAttribute>
#include <Qt3DCore/QBuffer>
#include <Qt3DCore/QGeometry>
#include <Qt3DExtras/QGoochMaterial>
#include <Qt3DRender/QEffect>
#include <Qt3DRender/QGraphicsApiFilter>
#include <Qt3DRender/QParameter>
#include <Qt3DRender/QTechnique>

using namespace Qt::StringLiterals;


QMap<QString, QString> QgsGoochMaterial3DHandler::toExportParameters( const QgsAbstractMaterialSettings * ) const
{
  return QMap<QString, QString>();
}

QgsMaterial *QgsGoochMaterial3DHandler::toMaterial( const QgsAbstractMaterialSettings *settings, Qgis::MaterialRenderingTechnique technique, const QgsMaterialContext &context ) const
{
  switch ( technique )
  {
    case Qgis::MaterialRenderingTechnique::Triangles:
    case Qgis::MaterialRenderingTechnique::TrianglesDataDefined:
    case Qgis::MaterialRenderingTechnique::TrianglesWithFixedTexture:
    case Qgis::MaterialRenderingTechnique::TrianglesFromModel:
    {
      if ( context.isHighlighted() )
      {
        return new QgsHighlightMaterial( technique );
      }

      return buildMaterial( settings, context );
    }

    case Qgis::MaterialRenderingTechnique::Lines:
    case Qgis::MaterialRenderingTechnique::InstancedPoints:
    case Qgis::MaterialRenderingTechnique::Points:
    case Qgis::MaterialRenderingTechnique::Billboards:
      return nullptr;
  }
  return nullptr;
}

void QgsGoochMaterial3DHandler::addParametersToEffect( Qt3DRender::QEffect *, const QgsAbstractMaterialSettings *, const QgsMaterialContext & ) const
{}

QByteArray QgsGoochMaterial3DHandler::dataDefinedVertexColorsAsByte( const QgsAbstractMaterialSettings *settings, const QgsExpressionContext &expressionContext ) const
{
  const QgsGoochMaterialSettings *goochSettings = dynamic_cast< const QgsGoochMaterialSettings * >( settings );
  Q_ASSERT( goochSettings );
  const QgsPropertyCollection &dataDefinedProperties = goochSettings->dataDefinedProperties();
  const QColor diffuse = dataDefinedProperties.valueAsColor( QgsAbstractMaterialSettings::Property::Diffuse, expressionContext, goochSettings->diffuse() );
  const QColor warm = dataDefinedProperties.valueAsColor( QgsAbstractMaterialSettings::Property::Warm, expressionContext, goochSettings->warm() );
  const QColor cool = dataDefinedProperties.valueAsColor( QgsAbstractMaterialSettings::Property::Cool, expressionContext, goochSettings->cool() );
  const QColor specular = dataDefinedProperties.valueAsColor( QgsAbstractMaterialSettings::Property::Specular, expressionContext, goochSettings->specular() );

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

int QgsGoochMaterial3DHandler::dataDefinedByteStride( const QgsAbstractMaterialSettings * ) const
{
  return 12 * sizeof( unsigned char );
}

void QgsGoochMaterial3DHandler::applyDataDefinedToGeometry( const QgsAbstractMaterialSettings *, Qt3DCore::QGeometry *geometry, int vertexCount, const QByteArray &data ) const
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

bool QgsGoochMaterial3DHandler::updatePreviewScene( Qt3DCore::QEntity *sceneRoot, const QgsAbstractMaterialSettings *settings, const QgsMaterialContext & ) const
{
  const QgsGoochMaterialSettings *goochSettings = qgis::down_cast< const QgsGoochMaterialSettings * >( settings );

  QgsMaterial *material = sceneRoot->findChild<QgsMaterial *>();
  if ( material->objectName() != "goochMaterial"_L1 )
    return false;

  Qt3DRender::QEffect *effect = material->effect();

  if ( Qt3DRender::QParameter *p = findParameter( effect, u"kd"_s ) )
    p->setValue( goochSettings->diffuse() );
  if ( Qt3DRender::QParameter *p = findParameter( effect, u"ks"_s ) )
    p->setValue( goochSettings->specular() );
  if ( Qt3DRender::QParameter *p = findParameter( effect, u"kblue"_s ) )
    p->setValue( goochSettings->cool() );
  if ( Qt3DRender::QParameter *p = findParameter( effect, u"kyellow"_s ) )
    p->setValue( goochSettings->warm() );
  if ( Qt3DRender::QParameter *p = findParameter( effect, u"shininess"_s ) )
    p->setValue( goochSettings->shininess() );
  if ( Qt3DRender::QParameter *p = findParameter( effect, u"alpha"_s ) )
    p->setValue( goochSettings->alpha() );
  if ( Qt3DRender::QParameter *p = findParameter( effect, u"beta"_s ) )
    p->setValue( goochSettings->beta() );

  return true;
}

QgsMaterial *QgsGoochMaterial3DHandler::buildMaterial( const QgsAbstractMaterialSettings *settings, const QgsMaterialContext &context ) const
{
  const QgsGoochMaterialSettings *goochSettings = dynamic_cast< const QgsGoochMaterialSettings * >( settings );
  Q_ASSERT( goochSettings );
  const QgsPropertyCollection &dataDefinedProperties = goochSettings->dataDefinedProperties();

  QgsMaterial *material = new QgsMaterial;
  material->setObjectName( u"goochMaterial"_s );

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

  if ( dataDefinedProperties.hasActiveProperties() )
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

    const QColor diffuseColor = context.isSelected() ? context.selectionColor() : goochSettings->diffuse();
    effect->addParameter( new Qt3DRender::QParameter( u"kd"_s, diffuseColor ) );
    effect->addParameter( new Qt3DRender::QParameter( u"ks"_s, goochSettings->specular() ) );
    effect->addParameter( new Qt3DRender::QParameter( u"kblue"_s, goochSettings->cool() ) );
    effect->addParameter( new Qt3DRender::QParameter( u"kyellow"_s, goochSettings->warm() ) );
  }

  renderPass->setShaderProgram( shaderProgram );
  technique->addRenderPass( renderPass );

  technique->addParameter( new Qt3DRender::QParameter( u"shininess"_s, goochSettings->shininess() ) );
  technique->addParameter( new Qt3DRender::QParameter( u"alpha"_s, goochSettings->alpha() ) );
  technique->addParameter( new Qt3DRender::QParameter( u"beta"_s, goochSettings->beta() ) );

  effect->addTechnique( technique );
  material->setEffect( effect );

  return material;
}
