/***************************************************************************
  qgssimplelinematerialsettings.cpp
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

#include "qgssimplelinematerialsettings.h"
#include "qgscolorutils.h"
#include "qgslinematerial_p.h"

#include <Qt3DRender/QTexture>
#include <Qt3DRender/QParameter>
#include <Qt3DRender/QEffect>
#include <QMap>

#if QT_VERSION < QT_VERSION_CHECK( 6, 0, 0 )
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

QString QgsSimpleLineMaterialSettings::type() const
{
  return QStringLiteral( "simpleline" );
}

bool QgsSimpleLineMaterialSettings::supportsTechnique( QgsMaterialSettingsRenderingTechnique technique )
{
  switch ( technique )
  {
    case QgsMaterialSettingsRenderingTechnique::Lines:
      return true;

    case QgsMaterialSettingsRenderingTechnique::Triangles:
    case QgsMaterialSettingsRenderingTechnique::InstancedPoints:
    case QgsMaterialSettingsRenderingTechnique::Points:
    case QgsMaterialSettingsRenderingTechnique::TrianglesWithFixedTexture:
    case QgsMaterialSettingsRenderingTechnique::TrianglesFromModel:
    case QgsMaterialSettingsRenderingTechnique::TrianglesDataDefined:
      return false;
  }
  return false;
}

QgsAbstractMaterialSettings *QgsSimpleLineMaterialSettings::create()
{
  return new QgsSimpleLineMaterialSettings();
}

QgsSimpleLineMaterialSettings *QgsSimpleLineMaterialSettings::clone() const
{
  return new QgsSimpleLineMaterialSettings( *this );
}

bool QgsSimpleLineMaterialSettings::equals( const QgsAbstractMaterialSettings *other ) const
{
  const QgsSimpleLineMaterialSettings *otherLine = dynamic_cast<const QgsSimpleLineMaterialSettings *>( other );
  if ( !otherLine )
    return false;

  return *this == *otherLine;
}

void QgsSimpleLineMaterialSettings::readXml( const QDomElement &elem, const QgsReadWriteContext &context )
{
  mAmbient = QgsColorUtils::colorFromString( elem.attribute( QStringLiteral( "ambient" ), QStringLiteral( "25,25,25" ) ) );

  QgsAbstractMaterialSettings::readXml( elem, context );
}

void QgsSimpleLineMaterialSettings::writeXml( QDomElement &elem, const QgsReadWriteContext &context ) const
{
  elem.setAttribute( QStringLiteral( "ambient" ), QgsColorUtils::colorToString( mAmbient ) );

  QgsAbstractMaterialSettings::writeXml( elem, context );
}

QgsMaterial *QgsSimpleLineMaterialSettings::toMaterial( QgsMaterialSettingsRenderingTechnique technique, const QgsMaterialContext &context ) const
{
  switch ( technique )
  {
    case QgsMaterialSettingsRenderingTechnique::Triangles:
    case QgsMaterialSettingsRenderingTechnique::InstancedPoints:
    case QgsMaterialSettingsRenderingTechnique::Points:
    case QgsMaterialSettingsRenderingTechnique::TrianglesWithFixedTexture:
    case QgsMaterialSettingsRenderingTechnique::TrianglesFromModel:
    case QgsMaterialSettingsRenderingTechnique::TrianglesDataDefined:
      return nullptr;

    case QgsMaterialSettingsRenderingTechnique::Lines:
    {
      QgsLineMaterial *mat = new QgsLineMaterial;
      if ( !context.isSelected() )
      {
        mat->setLineColor( mAmbient );
        mat->setUseVertexColors( dataDefinedProperties().isActive( QgsAbstractMaterialSettings::Property::Ambient ) );
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

QMap<QString, QString> QgsSimpleLineMaterialSettings::toExportParameters() const
{
  QMap<QString, QString> parameters;
  parameters[QStringLiteral( "Ka" )] = QStringLiteral( "%1 %2 %3" ).arg( mAmbient.redF() ).arg( mAmbient.greenF() ).arg( mAmbient.blueF() );
  return parameters;
}

void QgsSimpleLineMaterialSettings::addParametersToEffect( Qt3DRender::QEffect *effect, const QgsMaterialContext &materialContext ) const
{
  const QColor ambient = materialContext.isSelected() ? materialContext.selectionColor().darker() : mAmbient;
  Qt3DRender::QParameter *ambientParameter = new Qt3DRender::QParameter( QStringLiteral( "ambientColor" ), ambient );
  effect->addParameter( ambientParameter );
}

QByteArray QgsSimpleLineMaterialSettings::dataDefinedVertexColorsAsByte( const QgsExpressionContext &expressionContext ) const
{
  const QColor ambient = dataDefinedProperties().valueAsColor( QgsAbstractMaterialSettings::Property::Ambient, expressionContext, mAmbient );

  QByteArray array;
  array.resize( sizeof( unsigned char ) * 3 );
  unsigned char *fptr = reinterpret_cast<unsigned char *>( array.data() );

  *fptr++ = static_cast<unsigned char>( ambient.red() );
  *fptr++ = static_cast<unsigned char>( ambient.green() );
  *fptr++ = static_cast<unsigned char>( ambient.blue() );
  return array;
}

void QgsSimpleLineMaterialSettings::applyDataDefinedToGeometry( Qt3DQGeometry *geometry, int vertexCount, const QByteArray &data ) const
{
  Qt3DQBuffer *dataBuffer = new Qt3DQBuffer( geometry );

  Qt3DQAttribute *colorAttribute = new Qt3DQAttribute( geometry );
  colorAttribute->setName( QStringLiteral( "dataDefinedColor" ) );
  colorAttribute->setVertexBaseType( Qt3DQAttribute::UnsignedByte );
  colorAttribute->setVertexSize( 3 );
  colorAttribute->setAttributeType( Qt3DQAttribute::VertexAttribute );
  colorAttribute->setBuffer( dataBuffer );
  colorAttribute->setByteStride( 3 * sizeof( unsigned char ) );
  colorAttribute->setByteOffset( 0 );
  colorAttribute->setCount( vertexCount );
  geometry->addAttribute( colorAttribute );

  dataBuffer->setData( data );
}
