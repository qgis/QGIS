/***************************************************************************
  qgsunlitmaterial3dhandler.cpp
  --------------------------------------
  Date                 : June 2026
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

#include "qgsunlitmaterial3dhandler.h"

#include "qgs3d.h"
#include "qgs3dutils.h"
#include "qgsunlitmaterial.h"
#include "qgsunlitmaterialsettings.h"

#include <QMap>
#include <QString>
#include <Qt3DCore/QAttribute>
#include <Qt3DCore/QBuffer>
#include <Qt3DCore/QGeometry>
#include <Qt3DRender/QEffect>
#include <Qt3DRender/QGraphicsApiFilter>
#include <Qt3DRender/QParameter>
#include <Qt3DRender/QTechnique>

using namespace Qt::StringLiterals;


QgsMaterial *QgsUnlitMaterial3DHandler::toMaterial( const QgsAbstractMaterialSettings *settings, Qgis::MaterialRenderingTechnique technique, const QgsMaterialContext &context ) const
{
  switch ( technique )
  {
    case Qgis::MaterialRenderingTechnique::InstancedPoints:
    {
      Q_ASSERT( false );
      return nullptr;
    }
    case Qgis::MaterialRenderingTechnique::Triangles:
    case Qgis::MaterialRenderingTechnique::Points:
    case Qgis::MaterialRenderingTechnique::TrianglesDataDefined:
    case Qgis::MaterialRenderingTechnique::TrianglesWithFixedTexture:
    case Qgis::MaterialRenderingTechnique::TrianglesFromModel:
    {
      if ( context.isHighlighted() )
      {
        return Qgs3D::createHighlightMaterial();
      }

      const QgsUnlitMaterialSettings *unlitSettings = dynamic_cast< const QgsUnlitMaterialSettings * >( settings );
      Q_ASSERT( unlitSettings );

      auto material = new QgsUnlitMaterial();
      material->setObjectName( u"unlitMaterial"_s );

      const QgsPropertyCollection &dataDefinedProperties = unlitSettings->dataDefinedProperties();
      const bool dataDefined = dataDefinedProperties.isActive( QgsAbstractMaterialSettings::Property::BaseColor );
      if ( !dataDefined )
      {
        const QColor color = context.isSelected() ? context.selectionColor() : unlitSettings->color();
        material->setColor( color );
      }
      material->setDataDefinedEnabled( dataDefined );
      return material;
    }

    case Qgis::MaterialRenderingTechnique::Lines:
    case Qgis::MaterialRenderingTechnique::Billboards:
      return nullptr;
  }
  return nullptr;
}

QMap<QString, QString> QgsUnlitMaterial3DHandler::toExportParameters( const QgsAbstractMaterialSettings * ) const
{
  return {};
}

QByteArray QgsUnlitMaterial3DHandler::dataDefinedVertexColorsAsByte( const QgsAbstractMaterialSettings *settings, const QgsExpressionContext &expressionContext ) const
{
  auto unlitSettings = dynamic_cast< const QgsUnlitMaterialSettings * >( settings );
  Q_ASSERT( unlitSettings );

  const QColor color = Qgs3DUtils::srgbToLinear( unlitSettings->dataDefinedProperties().valueAsColor( QgsAbstractMaterialSettings::Property::BaseColor, expressionContext, unlitSettings->color() ) );

  QByteArray array;
  array.resize( sizeof( float ) * 3 );
  float *fptr = reinterpret_cast<float *>( array.data() );

  *fptr++ = color.redF();
  *fptr++ = color.greenF();
  *fptr++ = color.blueF();

  return array;
}

void QgsUnlitMaterial3DHandler::applyDataDefinedToGeometry( const QgsAbstractMaterialSettings *, Qt3DCore::QGeometry *geometry, int vertexCount, const QByteArray &data ) const
{
  Qt3DCore::QBuffer *dataBuffer = new Qt3DCore::QBuffer( geometry );

  Qt3DCore::QAttribute *baseColorAttribute = new Qt3DCore::QAttribute( geometry );
  baseColorAttribute->setName( u"dataDefinedBaseColor"_s );
  baseColorAttribute->setVertexBaseType( Qt3DCore::QAttribute::Float );
  baseColorAttribute->setVertexSize( 3 );
  baseColorAttribute->setAttributeType( Qt3DCore::QAttribute::VertexAttribute );
  baseColorAttribute->setBuffer( dataBuffer );
  baseColorAttribute->setByteStride( 3 * sizeof( float ) );
  baseColorAttribute->setByteOffset( 0 );
  baseColorAttribute->setCount( vertexCount );
  geometry->addAttribute( baseColorAttribute );

  dataBuffer->setData( data );
}

bool QgsUnlitMaterial3DHandler::updatePreviewScene( Qt3DCore::QEntity *sceneRoot, const QgsAbstractMaterialSettings *settings, const QgsMaterialContext & ) const
{
  auto unlitSettings = qgis::down_cast< const QgsUnlitMaterialSettings * >( settings );

  QgsUnlitMaterial *material = sceneRoot->findChild<QgsUnlitMaterial *>();
  if ( !material || material->objectName() != "unlitMaterial"_L1 )
    return false;

  material->setColor( unlitSettings->color() );

  return true;
}

QgsMaterial *QgsUnlitMaterial3DHandler::toInstancedMaterial(
  const QgsAbstractMaterialSettings *settings, const QgsMaterialContext &context, Qgis::InstancedMaterialFlags flags, const QMatrix4x4 &transform
) const
{
  auto unlitSettings = qgis::down_cast< const QgsUnlitMaterialSettings * >( settings );

  auto material = new QgsUnlitMaterial();
  material->setInstancingEnabled( true, flags );
  material->setInstancingMeshTransform( transform );
  material->setObjectName( u"unlitMaterial"_s );

  const QColor color = context.isSelected() ? context.selectionColor() : unlitSettings->color();
  material->setColor( color );

  return material;
}
