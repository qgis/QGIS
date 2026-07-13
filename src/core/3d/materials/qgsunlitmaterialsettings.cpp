/***************************************************************************
  qgsunlitmaterialsettings.cpp
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

#include "qgsunlitmaterialsettings.h"

#include "qgscolorutils.h"

#include <QMap>
#include <QString>

using namespace Qt::StringLiterals;

QString QgsUnlitMaterialSettings::type() const
{
  return u"unlit"_s;
}

bool QgsUnlitMaterialSettings::supportsTechnique( Qgis::MaterialRenderingTechnique technique )
{
  switch ( technique )
  {
    case Qgis::MaterialRenderingTechnique::Triangles:
    case Qgis::MaterialRenderingTechnique::InstancedPoints:
    case Qgis::MaterialRenderingTechnique::Points:
    case Qgis::MaterialRenderingTechnique::TrianglesWithFixedTexture:
    case Qgis::MaterialRenderingTechnique::TrianglesFromModel:
    case Qgis::MaterialRenderingTechnique::TrianglesDataDefined:
      return true;

    case Qgis::MaterialRenderingTechnique::Lines:
    case Qgis::MaterialRenderingTechnique::Billboards:
      return false;
  }
  return false;
}

QgsAbstractMaterialSettings *QgsUnlitMaterialSettings::create()
{
  return new QgsUnlitMaterialSettings();
}

QgsUnlitMaterialSettings *QgsUnlitMaterialSettings::clone() const
{
  return new QgsUnlitMaterialSettings( *this );
}

bool QgsUnlitMaterialSettings::equals( const QgsAbstractMaterialSettings *other ) const
{
  const QgsUnlitMaterialSettings *otherLine = dynamic_cast<const QgsUnlitMaterialSettings *>( other );
  if ( !otherLine )
    return false;

  return *this == *otherLine;
}

QSet<QgsAbstractMaterialSettings::Property> QgsUnlitMaterialSettings::supportedProperties() const
{
  return { QgsAbstractMaterialSettings::Property::BaseColor };
}

void QgsUnlitMaterialSettings::readXml( const QDomElement &elem, const QgsReadWriteContext &context )
{
  mColor = QgsColorUtils::colorFromString( elem.attribute( u"color"_s, QgsColorUtils::colorToString( mColor ) ) );

  QgsAbstractMaterialSettings::readXml( elem, context );
}

void QgsUnlitMaterialSettings::writeXml( QDomElement &elem, const QgsReadWriteContext &context ) const
{
  elem.setAttribute( u"color"_s, QgsColorUtils::colorToString( mColor ) );

  QgsAbstractMaterialSettings::writeXml( elem, context );
}

QColor QgsUnlitMaterialSettings::averageColor() const
{
  return mColor;
}

void QgsUnlitMaterialSettings::setColorsFromBase( const QColor &baseColor )
{
  mColor = baseColor;
}
