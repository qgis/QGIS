/***************************************************************************
  qgsmetalroughmaterial3dhandler.cpp
  --------------------------------------
  Date                 : December 2023
  Copyright            : (C) 2023 by Nyall Dawson
  Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsmetalroughmaterial3dhandler.h"

#include "qgs3dutils.h"
#include "qgshighlightmaterial.h"
#include "qgsmetalroughmaterial.h"
#include "qgsmetalroughmaterialsettings.h"

#include <QString>
#include <Qt3DCore/QAttribute>
#include <Qt3DCore/QBuffer>
#include <Qt3DCore/QEntity>
#include <Qt3DCore/QGeometry>
#include <Qt3DRender/QParameter>

using namespace Qt::StringLiterals;

QgsMaterial *QgsMetalRoughMaterial3DHandler::toMaterial( const QgsAbstractMaterialSettings *settings, Qgis::MaterialRenderingTechnique technique, const QgsMaterialContext &context ) const
{
  const QgsMetalRoughMaterialSettings *metalRoughSettings = dynamic_cast< const QgsMetalRoughMaterialSettings * >( settings );
  Q_ASSERT( metalRoughSettings );

  switch ( technique )
  {
    case Qgis::MaterialRenderingTechnique::Triangles:
    case Qgis::MaterialRenderingTechnique::TrianglesDataDefined:
    case Qgis::MaterialRenderingTechnique::TrianglesWithFixedTexture:
    case Qgis::MaterialRenderingTechnique::TrianglesFromModel:
    {
      if ( context.isHighlighted() )
      {
        return new QgsHighlightMaterial();
      }

      QgsMetalRoughMaterial *material = new QgsMetalRoughMaterial( nullptr );
      material->setEnvironmentalLightingEnabled( !context.isPreview() );
      material->setObjectName( u"metalRoughMaterial"_s );
      applySettingsToMaterial( metalRoughSettings, material, context );
      material->setDataDefinedEnabled(
        metalRoughSettings->dataDefinedProperties().isActive( QgsAbstractMaterialSettings::Property::BaseColor )
        || metalRoughSettings->dataDefinedProperties().isActive( QgsAbstractMaterialSettings::Property::EmissionColor )
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

QgsMaterial *QgsMetalRoughMaterial3DHandler::toInstancedMaterial(
  const QgsAbstractMaterialSettings *settings, const QgsMaterialContext &context, Qgis::InstancedMaterialFlags flags, const QMatrix4x4 &transform
) const
{
  const QgsMetalRoughMaterialSettings *metalRoughSettings = qgis::down_cast< const QgsMetalRoughMaterialSettings * >( settings );

  QgsMetalRoughMaterial *material = new QgsMetalRoughMaterial();
  material->setEnvironmentalLightingEnabled( true );
  material->setInstancingEnabled( true, flags );
  material->setInstancingMeshTransform( transform );

  material->setObjectName( u"metalRoughMaterial"_s );
  applySettingsToMaterial( metalRoughSettings, material, context );

  return material;
}

QMap<QString, QString> QgsMetalRoughMaterial3DHandler::toExportParameters( const QgsAbstractMaterialSettings * ) const
{
  QMap<QString, QString> parameters;
  return parameters;
}

bool QgsMetalRoughMaterial3DHandler::updatePreviewScene( Qt3DCore::QEntity *sceneRoot, const QgsAbstractMaterialSettings *settings, const QgsMaterialContext &context ) const
{
  const QgsMetalRoughMaterialSettings *metalRoughSettings = qgis::down_cast< const QgsMetalRoughMaterialSettings * >( settings );

  QgsMetalRoughMaterial *material = sceneRoot->findChild<QgsMetalRoughMaterial *>();
  if ( material->objectName() != "metalRoughMaterial"_L1 )
    return false;

  applySettingsToMaterial( metalRoughSettings, material, context );
  return true;
}

QByteArray QgsMetalRoughMaterial3DHandler::dataDefinedVertexColorsAsByte( const QgsAbstractMaterialSettings *settings, const QgsExpressionContext &expressionContext ) const
{
  const QgsMetalRoughMaterialSettings *metalRoughSettings = dynamic_cast< const QgsMetalRoughMaterialSettings * >( settings );
  Q_ASSERT( metalRoughSettings );
  const QgsPropertyCollection &dataDefinedProperties = metalRoughSettings->dataDefinedProperties();
  const QColor base = Qgs3DUtils::srgbToLinear( dataDefinedProperties.valueAsColor( QgsAbstractMaterialSettings::Property::BaseColor, expressionContext, metalRoughSettings->baseColor() ) );
  const QColor rawEmissionColor = dataDefinedProperties.valueAsColor( QgsAbstractMaterialSettings::Property::EmissionColor, expressionContext, metalRoughSettings->emissionColor() );
  const QColor emission = rawEmissionColor.isValid() ? Qgs3DUtils::srgbToLinear( rawEmissionColor ) : QColor();

  QByteArray array;
  array.resize( sizeof( float ) * 6 );
  float *fptr = reinterpret_cast<float *>( array.data() );

  *fptr++ = base.redF();
  *fptr++ = base.greenF();
  *fptr++ = base.blueF();

  if ( emission.isValid() )
  {
    *fptr++ = emission.redF();
    *fptr++ = emission.greenF();
    *fptr++ = emission.blueF();
  }
  else
  {
    *fptr++ = 0.0f;
    *fptr++ = 0.0f;
    *fptr++ = 0.0f;
  }
  return array;
}

void QgsMetalRoughMaterial3DHandler::applyDataDefinedToGeometry( const QgsAbstractMaterialSettings *, Qt3DCore::QGeometry *geometry, int vertexCount, const QByteArray &data ) const
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
  emissionColorAttribute->setName( u"dataDefinedEmissionColor"_s );
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

void QgsMetalRoughMaterial3DHandler::applySettingsToMaterial( const QgsMetalRoughMaterialSettings *metalRoughSettings, QgsMetalRoughMaterial *material, const QgsMaterialContext &context )
{
  material->setBaseColor( context.isSelected() ? context.selectionColor() : metalRoughSettings->baseColor() );
  material->setEmissionColor( metalRoughSettings->emissionColor().isValid() ? metalRoughSettings->emissionColor() : QColor( 0, 0, 0 ) );
  material->setEmissionFactor( static_cast< float>( metalRoughSettings->emissionFactor() ) );
  material->setClearCoatFactor( static_cast< float >( metalRoughSettings->clearCoatFactor() ) );
  material->setClearCoatRoughness( static_cast< float >( metalRoughSettings->clearCoatRoughness() ) );
  material->setMetalness( static_cast< float >( metalRoughSettings->metalness() ) );
  material->setRoughness( static_cast< float >( metalRoughSettings->roughness() ) );
  material->setReflectance( static_cast< float >( metalRoughSettings->reflectance() ) );
  material->setAnisotropy( static_cast< float >( metalRoughSettings->anisotropy() ) );
  material->setAnisotropyRotation( static_cast< float >( metalRoughSettings->anisotropyRotation() ) );
  material->setOpacity( static_cast< float >( metalRoughSettings->opacity() ) );
}
