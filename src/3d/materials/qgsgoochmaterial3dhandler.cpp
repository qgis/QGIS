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
#include "qgsgoochmaterial.h"
#include "qgsgoochmaterialsettings.h"
#include "qgshighlightmaterial.h"

#include <QString>
#include <Qt3DCore/QAttribute>
#include <Qt3DCore/QBuffer>
#include <Qt3DCore/QGeometry>

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
  const QColor diffuse = Qgs3DUtils::srgbToLinear( dataDefinedProperties.valueAsColor( QgsAbstractMaterialSettings::Property::Diffuse, expressionContext, goochSettings->diffuse() ) );
  const QColor warm = Qgs3DUtils::srgbToLinear( dataDefinedProperties.valueAsColor( QgsAbstractMaterialSettings::Property::Warm, expressionContext, goochSettings->warm() ) );
  const QColor cool = Qgs3DUtils::srgbToLinear( dataDefinedProperties.valueAsColor( QgsAbstractMaterialSettings::Property::Cool, expressionContext, goochSettings->cool() ) );
  const QColor specular = Qgs3DUtils::srgbToLinear( dataDefinedProperties.valueAsColor( QgsAbstractMaterialSettings::Property::Specular, expressionContext, goochSettings->specular() ) );

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

  QgsGoochMaterial *material = sceneRoot->findChild<QgsGoochMaterial *>();
  if ( !material || material->objectName() != "goochMaterial"_L1 )
    return false;

  applySettingsToMaterial( goochSettings, material );

  return true;
}

QgsMaterial *QgsGoochMaterial3DHandler::buildMaterial( const QgsAbstractMaterialSettings *settings, const QgsMaterialContext &context ) const
{
  const QgsGoochMaterialSettings *goochSettings = dynamic_cast< const QgsGoochMaterialSettings * >( settings );
  Q_ASSERT( goochSettings );
  const QgsPropertyCollection &dataDefinedProperties = goochSettings->dataDefinedProperties();

  QgsGoochMaterial *material = new QgsGoochMaterial;
  material->setObjectName( u"goochMaterial"_s );

  applySettingsToMaterial( goochSettings, material );
  if ( context.isSelected() )
    material->setDiffuse( context.selectionColor() );
  material->setDataDefinedEnabled( dataDefinedProperties.hasActiveProperties() );

  return material;
}

void QgsGoochMaterial3DHandler::applySettingsToMaterial( const QgsGoochMaterialSettings *settings, QgsGoochMaterial *material )
{
  material->setDiffuse( settings->diffuse() );
  material->setSpecular( settings->specular() );
  material->setCool( settings->cool() );
  material->setWarm( settings->warm() );
  material->setShininess( static_cast<float>( settings->shininess() ) );
  material->setAlpha( static_cast<float>( settings->alpha() ) );
  material->setBeta( static_cast<float>( settings->beta() ) );
}
