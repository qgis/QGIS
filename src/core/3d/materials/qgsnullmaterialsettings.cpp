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

#include <QMap>
#include <QString>

using namespace Qt::StringLiterals;

QString QgsNullMaterialSettings::type() const
{
  return u"null"_s;
}

bool QgsNullMaterialSettings::supportsTechnique( Qgis::MaterialRenderingTechnique technique )
{
  switch ( technique )
  {
    case Qgis::MaterialRenderingTechnique::TrianglesFromModel:
      return true;

    case Qgis::MaterialRenderingTechnique::Triangles:
    case Qgis::MaterialRenderingTechnique::InstancedPoints:
    case Qgis::MaterialRenderingTechnique::Points:
    case Qgis::MaterialRenderingTechnique::TrianglesWithFixedTexture:
    case Qgis::MaterialRenderingTechnique::TrianglesDataDefined:
    case Qgis::MaterialRenderingTechnique::Lines:
    case Qgis::MaterialRenderingTechnique::Billboards:
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

QColor QgsNullMaterialSettings::averageColor() const
{
  return QColor();
}

void QgsNullMaterialSettings::setColorsFromBase( const QColor &baseColor )
{
  Q_UNUSED( baseColor )
}
