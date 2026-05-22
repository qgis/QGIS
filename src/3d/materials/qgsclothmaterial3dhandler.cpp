/***************************************************************************
  qgsclothmaterial3dhandler.cpp
  --------------------------------------
  Date                 : May 2026
  Copyright            : (C) 2026 by Nyall Dawson
  Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsclothmaterial3dhandler.h"

#include "qgs3dutils.h"
#include "qgsclothmaterial.h"
#include "qgsclothmaterialsettings.h"
#include "qgshighlightmaterial.h"

#include <QString>
#include <Qt3DCore/QAttribute>
#include <Qt3DCore/QBuffer>
#include <Qt3DCore/QEntity>
#include <Qt3DCore/QGeometry>
#include <Qt3DRender/QParameter>

using namespace Qt::StringLiterals;

QgsMaterial *QgsClothMaterial3DHandler::toMaterial( const QgsAbstractMaterialSettings *settings, Qgis::MaterialRenderingTechnique technique, const QgsMaterialContext &context ) const
{
  const QgsClothMaterialSettings *clothSettings = dynamic_cast< const QgsClothMaterialSettings * >( settings );
  Q_ASSERT( clothSettings );

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

      auto material = new QgsClothMaterial;
      material->setObjectName( u"clothMaterial"_s );
      applySettingsToMaterial( clothSettings, material, context );
      material->setDataDefinedEnabled(
        clothSettings->dataDefinedProperties().isActive( QgsAbstractMaterialSettings::Property::BaseColor )
        || clothSettings->dataDefinedProperties().isActive( QgsAbstractMaterialSettings::Property::SheenColor )
      );

      return material;
    }

    case Qgis::MaterialRenderingTechnique::Lines:
    case Qgis::MaterialRenderingTechnique::InstancedPoints:
    case Qgis::MaterialRenderingTechnique::Points:
    case Qgis::MaterialRenderingTechnique::Billboards:
      return nullptr;
  }
  return nullptr;
}

QgsMaterial *QgsClothMaterial3DHandler::toInstancedMaterial( const QgsAbstractMaterialSettings *settings, const QgsMaterialContext &context, Qgis::InstancedMaterialFlags flags ) const
{
  const QgsClothMaterialSettings *clothSettings = qgis::down_cast< const QgsClothMaterialSettings * >( settings );

  QgsClothMaterial *material = new QgsClothMaterial();
  material->setEnvironmentalLightingEnabled( true );
  material->setInstancingEnabled( true, flags );

  material->setObjectName( u"clothMaterial"_s );
  applySettingsToMaterial( clothSettings, material, context );

  return material;
}

QMap<QString, QString> QgsClothMaterial3DHandler::toExportParameters( const QgsAbstractMaterialSettings * ) const
{
  QMap<QString, QString> parameters;
  return parameters;
}

bool QgsClothMaterial3DHandler::updatePreviewScene( Qt3DCore::QEntity *sceneRoot, const QgsAbstractMaterialSettings *settings, const QgsMaterialContext &context ) const
{
  const QgsClothMaterialSettings *clothSettings = qgis::down_cast< const QgsClothMaterialSettings * >( settings );

  QgsClothMaterial *material = sceneRoot->findChild<QgsClothMaterial *>();
  if ( material->objectName() != "clothMaterial"_L1 )
    return false;

  applySettingsToMaterial( clothSettings, material, context );
  return true;
}

QByteArray QgsClothMaterial3DHandler::dataDefinedVertexColorsAsByte( const QgsAbstractMaterialSettings *settings, const QgsExpressionContext &expressionContext ) const
{
  const QgsClothMaterialSettings *clothSettings = dynamic_cast< const QgsClothMaterialSettings * >( settings );
  Q_ASSERT( clothSettings );
  const QgsPropertyCollection &dataDefinedProperties = clothSettings->dataDefinedProperties();
  const QColor base = Qgs3DUtils::srgbToLinear( dataDefinedProperties.valueAsColor( QgsAbstractMaterialSettings::Property::BaseColor, expressionContext, clothSettings->baseColor() ) );
  const QColor rawSheenColor = dataDefinedProperties.valueAsColor( QgsAbstractMaterialSettings::Property::SheenColor, expressionContext, clothSettings->sheenColor() );
  const QColor sheen = rawSheenColor.isValid() ? Qgs3DUtils::srgbToLinear( rawSheenColor ) : QColor();

  QByteArray array;
  array.resize( sizeof( float ) * 6 );
  float *fptr = reinterpret_cast<float *>( array.data() );

  *fptr++ = base.redF();
  *fptr++ = base.greenF();
  *fptr++ = base.blueF();

  if ( sheen.isValid() )
  {
    *fptr++ = sheen.redF();
    *fptr++ = sheen.greenF();
    *fptr++ = sheen.blueF();
  }
  else
  {
    *fptr++ = 0.0f;
    *fptr++ = 0.0f;
    *fptr++ = 0.0f;
  }
  return array;
}

void QgsClothMaterial3DHandler::applyDataDefinedToGeometry( const QgsAbstractMaterialSettings *, Qt3DCore::QGeometry *geometry, int vertexCount, const QByteArray &data ) const
{
  Qt3DCore::QBuffer *dataBuffer = new Qt3DCore::QBuffer( geometry );

  Qt3DCore::QAttribute *baseColorAttribute = new Qt3DCore::QAttribute( geometry );
  baseColorAttribute->setName( u"dataDefinedBaseColor"_s );
  baseColorAttribute->setVertexBaseType( Qt3DCore::QAttribute::Float );
  baseColorAttribute->setVertexSize( 3 );
  baseColorAttribute->setAttributeType( Qt3DCore::QAttribute::VertexAttribute );
  baseColorAttribute->setBuffer( dataBuffer );
  baseColorAttribute->setByteStride( 6 * sizeof( float ) );
  baseColorAttribute->setByteOffset( 0 );
  baseColorAttribute->setCount( vertexCount );
  geometry->addAttribute( baseColorAttribute );

  Qt3DCore::QAttribute *emissionColorAttribute = new Qt3DCore::QAttribute( geometry );
  emissionColorAttribute->setName( u"dataDefinedSecondaryColor"_s );
  emissionColorAttribute->setVertexBaseType( Qt3DCore::QAttribute::Float );
  emissionColorAttribute->setVertexSize( 3 );
  emissionColorAttribute->setAttributeType( Qt3DCore::QAttribute::VertexAttribute );
  emissionColorAttribute->setBuffer( dataBuffer );
  emissionColorAttribute->setByteStride( 6 * sizeof( float ) );
  emissionColorAttribute->setByteOffset( 3 * sizeof( float ) );
  emissionColorAttribute->setCount( vertexCount );
  geometry->addAttribute( emissionColorAttribute );

  dataBuffer->setData( data );
}

void QgsClothMaterial3DHandler::applySettingsToMaterial( const QgsClothMaterialSettings *clothSettings, QgsClothMaterial *material, const QgsMaterialContext &context )
{
  material->setBaseColor( context.isSelected() ? context.selectionColor() : clothSettings->baseColor() );
  material->setSheenColor( clothSettings->sheenColor() );
  material->setRoughness( static_cast< float >( clothSettings->roughness() ) );
  material->setOpacity( static_cast< float >( clothSettings->opacity() ) );
}
