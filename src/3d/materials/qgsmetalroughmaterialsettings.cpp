/***************************************************************************
  qgsmetalroughmaterialsettings.cpp
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

#include "qgsmetalroughmaterialsettings.h"

#include "qgsmetalroughmaterial.h"
#include "qgssymbollayerutils.h"

QString QgsMetalRoughMaterialSettings::type() const
{
  return u"metalrough"_s;
}

bool QgsMetalRoughMaterialSettings::supportsTechnique( QgsMaterialSettingsRenderingTechnique technique )
{
  switch ( technique )
  {
    case QgsMaterialSettingsRenderingTechnique::Triangles:
    case QgsMaterialSettingsRenderingTechnique::TrianglesWithFixedTexture:
    case QgsMaterialSettingsRenderingTechnique::TrianglesFromModel:
    case QgsMaterialSettingsRenderingTechnique::TrianglesDataDefined:
      return true;

    case QgsMaterialSettingsRenderingTechnique::Points:
    case QgsMaterialSettingsRenderingTechnique::Lines:
    case QgsMaterialSettingsRenderingTechnique::InstancedPoints:
      return false;
  }
  return false;
}

QgsAbstractMaterialSettings *QgsMetalRoughMaterialSettings::create()
{
  return new QgsMetalRoughMaterialSettings();
}

QgsMetalRoughMaterialSettings *QgsMetalRoughMaterialSettings::clone() const
{
  return new QgsMetalRoughMaterialSettings( *this );
}

bool QgsMetalRoughMaterialSettings::equals( const QgsAbstractMaterialSettings *other ) const
{
  const QgsMetalRoughMaterialSettings *otherMetal = dynamic_cast<const QgsMetalRoughMaterialSettings *>( other );
  if ( !otherMetal )
    return false;

  return *this == *otherMetal;
}

void QgsMetalRoughMaterialSettings::readXml( const QDomElement &elem, const QgsReadWriteContext &context )
{
  mBaseColor = QgsSymbolLayerUtils::decodeColor( elem.attribute( u"base"_s, u"125,125,125"_s ) );
  mMetalness = elem.attribute( u"metalness"_s ).toDouble();
  mRoughness = elem.attribute( u"roughness"_s ).toDouble();

  QgsAbstractMaterialSettings::readXml( elem, context );
}

void QgsMetalRoughMaterialSettings::writeXml( QDomElement &elem, const QgsReadWriteContext &context ) const
{
  elem.setAttribute( u"base"_s, QgsSymbolLayerUtils::encodeColor( mBaseColor ) );
  elem.setAttribute( u"metalness"_s, mMetalness );
  elem.setAttribute( u"roughness"_s, mRoughness );

  QgsAbstractMaterialSettings::writeXml( elem, context );
}

QgsMaterial *QgsMetalRoughMaterialSettings::toMaterial( QgsMaterialSettingsRenderingTechnique technique, const QgsMaterialContext &context ) const
{
  switch ( technique )
  {
    case QgsMaterialSettingsRenderingTechnique::Triangles:
    case QgsMaterialSettingsRenderingTechnique::TrianglesDataDefined:
    case QgsMaterialSettingsRenderingTechnique::TrianglesWithFixedTexture:
    case QgsMaterialSettingsRenderingTechnique::TrianglesFromModel:
    {
      QgsMetalRoughMaterial *material = new QgsMetalRoughMaterial;
      material->setBaseColor( context.isSelected() ? context.selectionColor() : mBaseColor );
      material->setMetalness( mMetalness );
      material->setRoughness( mRoughness );
      return material;
    }

    case QgsMaterialSettingsRenderingTechnique::Lines:
    case QgsMaterialSettingsRenderingTechnique::InstancedPoints:
    case QgsMaterialSettingsRenderingTechnique::Points:
      return nullptr;
  }
  return nullptr;
}

QMap<QString, QString> QgsMetalRoughMaterialSettings::toExportParameters() const
{
  QMap<QString, QString> parameters;
  return parameters;
}

void QgsMetalRoughMaterialSettings::addParametersToEffect( Qt3DRender::QEffect *, const QgsMaterialContext & ) const
{
}
