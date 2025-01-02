/***************************************************************************
  qgsnullmaterialsettings.cpp
  --------------------------------------
  Date                 : November 2020
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

#include "qgsnullmaterialsettings.h"
#include "qgsmaterial.h"
#include <QMap>


QString QgsNullMaterialSettings::type() const
{
  return QStringLiteral( "null" );
}

bool QgsNullMaterialSettings::supportsTechnique( QgsMaterialSettingsRenderingTechnique technique )
{
  switch ( technique )
  {
    case QgsMaterialSettingsRenderingTechnique::TrianglesFromModel:
      return true;

    case QgsMaterialSettingsRenderingTechnique::Triangles:
    case QgsMaterialSettingsRenderingTechnique::InstancedPoints:
    case QgsMaterialSettingsRenderingTechnique::Points:
    case QgsMaterialSettingsRenderingTechnique::TrianglesWithFixedTexture:
    case QgsMaterialSettingsRenderingTechnique::TrianglesDataDefined:
    case QgsMaterialSettingsRenderingTechnique::Lines:
      return false;
  }
  return false;
}

QgsAbstractMaterialSettings *QgsNullMaterialSettings::create()
{
  return new QgsNullMaterialSettings();
}

QgsNullMaterialSettings *QgsNullMaterialSettings::clone() const
{
  return new QgsNullMaterialSettings( *this );
}

bool QgsNullMaterialSettings::equals( const QgsAbstractMaterialSettings *other ) const
{
  const QgsNullMaterialSettings *otherNull = dynamic_cast<const QgsNullMaterialSettings *>( other );
  if ( !otherNull )
    return false;

  return true;
}

QgsMaterial *QgsNullMaterialSettings::toMaterial( QgsMaterialSettingsRenderingTechnique, const QgsMaterialContext & ) const
{
  return nullptr;
}

QMap<QString, QString> QgsNullMaterialSettings::toExportParameters() const
{
  QMap<QString, QString> parameters;
  return parameters;
}

void QgsNullMaterialSettings::addParametersToEffect( Qt3DRender::QEffect *, const QgsMaterialContext & ) const
{
}
