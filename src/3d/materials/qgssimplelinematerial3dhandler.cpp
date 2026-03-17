/***************************************************************************
  qgssimplelinematerial3dhandler.cpp
  --------------------------------------
  Date                 : August 2020
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

#include "qgssimplelinematerial3dhandler.h"

#include "qgslinematerial_p.h"
#include "qgssimplelinematerialsettings.h"

#include <QMap>
#include <QString>
#include <Qt3DCore/QAttribute>
#include <Qt3DCore/QBuffer>
#include <Qt3DCore/QGeometry>
#include <Qt3DRender/QEffect>
#include <Qt3DRender/QParameter>
#include <Qt3DRender/QTexture>

using namespace Qt::StringLiterals;


QgsMaterial *QgsSimpleLineMaterial3DHandler::toMaterial( const QgsAbstractMaterialSettings *settings, Qgis::MaterialRenderingTechnique technique, const QgsMaterialContext &context ) const
{
  const QgsSimpleLineMaterialSettings *lineSettings = dynamic_cast< const QgsSimpleLineMaterialSettings * >( settings );
  Q_ASSERT( lineSettings );

  switch ( technique )
  {
    case Qgis::MaterialRenderingTechnique::Triangles:
    case Qgis::MaterialRenderingTechnique::InstancedPoints:
    case Qgis::MaterialRenderingTechnique::Points:
    case Qgis::MaterialRenderingTechnique::TrianglesWithFixedTexture:
    case Qgis::MaterialRenderingTechnique::TrianglesFromModel:
    case Qgis::MaterialRenderingTechnique::TrianglesDataDefined:
      return nullptr;

    case Qgis::MaterialRenderingTechnique::Lines:
    {
      if ( context.isHighlighted() )
      {
        // QgsHighlightMaterial does not support lines
        return nullptr;
      }

      QgsLineMaterial *mat = new QgsLineMaterial;
      if ( !context.isSelected() )
      {
        mat->setLineColor( lineSettings->ambient() );
        mat->setUseVertexColors( lineSettings->dataDefinedProperties().isActive( QgsAbstractMaterialSettings::Property::Ambient ) );
      }
      else
      {
        // update the material with selection colors
        mat->setLineColor( context.selectionColor() );
        mat->setUseVertexColors( false );
      }
      return mat;
    }
  }
  return nullptr;
}

QMap<QString, QString> QgsSimpleLineMaterial3DHandler::toExportParameters( const QgsAbstractMaterialSettings *settings ) const
{
  const QgsSimpleLineMaterialSettings *lineSettings = dynamic_cast< const QgsSimpleLineMaterialSettings * >( settings );
  Q_ASSERT( lineSettings );

  QMap<QString, QString> parameters;
  parameters[u"Ka"_s] = u"%1 %2 %3"_s.arg( lineSettings->ambient().redF() ).arg( lineSettings->ambient().greenF() ).arg( lineSettings->ambient().blueF() );
  return parameters;
}

void QgsSimpleLineMaterial3DHandler::addParametersToEffect( Qt3DRender::QEffect *effect, const QgsAbstractMaterialSettings *settings, const QgsMaterialContext &materialContext ) const
{
  const QgsSimpleLineMaterialSettings *lineSettings = dynamic_cast< const QgsSimpleLineMaterialSettings * >( settings );
  Q_ASSERT( lineSettings );

  const QColor ambient = materialContext.isSelected() ? materialContext.selectionColor().darker() : lineSettings->ambient();
  Qt3DRender::QParameter *ambientParameter = new Qt3DRender::QParameter( u"ambientColor"_s, ambient );
  effect->addParameter( ambientParameter );
}

QByteArray QgsSimpleLineMaterial3DHandler::dataDefinedVertexColorsAsByte( const QgsAbstractMaterialSettings *settings, const QgsExpressionContext &expressionContext ) const
{
  const QgsSimpleLineMaterialSettings *lineSettings = dynamic_cast< const QgsSimpleLineMaterialSettings * >( settings );
  Q_ASSERT( lineSettings );

  const QColor ambient = lineSettings->dataDefinedProperties().valueAsColor( QgsAbstractMaterialSettings::Property::Ambient, expressionContext, lineSettings->ambient() );

  QByteArray array;
  array.resize( sizeof( unsigned char ) * 3 );
  unsigned char *fptr = reinterpret_cast<unsigned char *>( array.data() );

  *fptr++ = static_cast<unsigned char>( ambient.red() );
  *fptr++ = static_cast<unsigned char>( ambient.green() );
  *fptr++ = static_cast<unsigned char>( ambient.blue() );
  return array;
}

void QgsSimpleLineMaterial3DHandler::applyDataDefinedToGeometry( const QgsAbstractMaterialSettings *, Qt3DCore::QGeometry *geometry, int vertexCount, const QByteArray &data ) const
{
  Qt3DCore::QBuffer *dataBuffer = new Qt3DCore::QBuffer( geometry );

  Qt3DCore::QAttribute *colorAttribute = new Qt3DCore::QAttribute( geometry );
  colorAttribute->setName( u"dataDefinedColor"_s );
  colorAttribute->setVertexBaseType( Qt3DCore::QAttribute::UnsignedByte );
  colorAttribute->setVertexSize( 3 );
  colorAttribute->setAttributeType( Qt3DCore::QAttribute::VertexAttribute );
  colorAttribute->setBuffer( dataBuffer );
  colorAttribute->setByteStride( 3 * sizeof( unsigned char ) );
  colorAttribute->setByteOffset( 0 );
  colorAttribute->setCount( vertexCount );
  geometry->addAttribute( colorAttribute );

  dataBuffer->setData( data );
}
