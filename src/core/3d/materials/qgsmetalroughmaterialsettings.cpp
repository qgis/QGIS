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

#include "qgssymbollayerutils.h"

#include <QString>

using namespace Qt::StringLiterals;

QString QgsMetalRoughMaterialSettings::type() const
{
  return u"metalrough"_s;
}

bool QgsMetalRoughMaterialSettings::supportsTechnique( Qgis::MaterialRenderingTechnique technique )
{
  switch ( technique )
  {
    case Qgis::MaterialRenderingTechnique::Triangles:
    case Qgis::MaterialRenderingTechnique::TrianglesWithFixedTexture:
    case Qgis::MaterialRenderingTechnique::TrianglesFromModel:
    case Qgis::MaterialRenderingTechnique::TrianglesDataDefined:
      return true;

    case Qgis::MaterialRenderingTechnique::Points:
    case Qgis::MaterialRenderingTechnique::Lines:
    case Qgis::MaterialRenderingTechnique::InstancedPoints:
    case Qgis::MaterialRenderingTechnique::Billboards:
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
  mMetalness = elem.attribute( u"metalness"_s, u"0.0"_s ).toDouble();
  mRoughness = elem.attribute( u"roughness"_s, u"0.5"_s ).toDouble();
  mOpacity = elem.attribute( u"opacity"_s, u"1.0"_s ).toDouble();

  QgsAbstractMaterialSettings::readXml( elem, context );
}

void QgsMetalRoughMaterialSettings::writeXml( QDomElement &elem, const QgsReadWriteContext &context ) const
{
  elem.setAttribute( u"base"_s, QgsSymbolLayerUtils::encodeColor( mBaseColor ) );
  elem.setAttribute( u"metalness"_s, mMetalness );
  elem.setAttribute( u"roughness"_s, mRoughness );
  if ( !qgsDoubleNear( mOpacity, 1 ) )
  {
    elem.setAttribute( u"opacity"_s, mOpacity );
  }

  QgsAbstractMaterialSettings::writeXml( elem, context );
}
