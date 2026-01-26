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

#include <QMap>
#include <Qt3DCore/QAttribute>
#include <Qt3DCore/QBuffer>
#include <Qt3DCore/QGeometry>
#include <Qt3DRender/QEffect>
#include <Qt3DRender/QParameter>
#include <Qt3DRender/QTexture>

QString QgsSimpleLineMaterialSettings::type() const
{
  return u"simpleline"_s;
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
  mAmbient = QgsColorUtils::colorFromString( elem.attribute( u"ambient"_s, u"25,25,25"_s ) );

  QgsAbstractMaterialSettings::readXml( elem, context );
}

void QgsSimpleLineMaterialSettings::writeXml( QDomElement &elem, const QgsReadWriteContext &context ) const
{
  elem.setAttribute( u"ambient"_s, QgsColorUtils::colorToString( mAmbient ) );

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
  parameters[u"Ka"_s] = u"%1 %2 %3"_s.arg( mAmbient.redF() ).arg( mAmbient.greenF() ).arg( mAmbient.blueF() );
  return parameters;
}

void QgsSimpleLineMaterialSettings::addParametersToEffect( Qt3DRender::QEffect *effect, const QgsMaterialContext &materialContext ) const
{
  const QColor ambient = materialContext.isSelected() ? materialContext.selectionColor().darker() : mAmbient;
  Qt3DRender::QParameter *ambientParameter = new Qt3DRender::QParameter( u"ambientColor"_s, ambient );
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

void QgsSimpleLineMaterialSettings::applyDataDefinedToGeometry( Qt3DCore::QGeometry *geometry, int vertexCount, const QByteArray &data ) const
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
