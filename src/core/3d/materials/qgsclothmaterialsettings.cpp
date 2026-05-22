/***************************************************************************
  qgsclothmaterialsettings.cpp
  --------------------------------------
  Date                 : May 2026
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

#include "qgsclothmaterialsettings.h"

#include "qgssymbollayerutils.h"

#include <QString>

using namespace Qt::StringLiterals;

QString QgsClothMaterialSettings::type() const
{
  return u"cloth"_s;
}

QSet<QgsAbstractMaterialSettings::Property> QgsClothMaterialSettings::supportedProperties() const
{
  return { QgsAbstractMaterialSettings::Property::BaseColor, QgsAbstractMaterialSettings::Property::SheenColor };
}

bool QgsClothMaterialSettings::supportsTechnique( Qgis::MaterialRenderingTechnique technique )
{
  switch ( technique )
  {
    case Qgis::MaterialRenderingTechnique::Triangles:
    case Qgis::MaterialRenderingTechnique::TrianglesWithFixedTexture:
    case Qgis::MaterialRenderingTechnique::TrianglesFromModel:
    case Qgis::MaterialRenderingTechnique::TrianglesDataDefined:
    case Qgis::MaterialRenderingTechnique::InstancedPoints:
      return true;

    case Qgis::MaterialRenderingTechnique::Points:
    case Qgis::MaterialRenderingTechnique::Lines:
    case Qgis::MaterialRenderingTechnique::Billboards:
      return false;
  }
  return false;
}

QgsAbstractMaterialSettings *QgsClothMaterialSettings::create()
{
  return new QgsClothMaterialSettings();
}

QgsClothMaterialSettings *QgsClothMaterialSettings::clone() const
{
  return new QgsClothMaterialSettings( *this );
}

bool QgsClothMaterialSettings::equals( const QgsAbstractMaterialSettings *other ) const
{
  const QgsClothMaterialSettings *otherMetal = dynamic_cast<const QgsClothMaterialSettings *>( other );
  if ( !otherMetal )
    return false;

  return *this == *otherMetal;
}

void QgsClothMaterialSettings::readXml( const QDomElement &elem, const QgsReadWriteContext &context )
{
  mBaseColor = QgsSymbolLayerUtils::decodeColor( elem.attribute( u"base"_s, u"125,125,125"_s ) );
  mSheenColor = QgsSymbolLayerUtils::decodeColor( elem.attribute( u"sheen_color"_s, u"255,255,255"_s ) );
  mRoughness = elem.attribute( u"roughness"_s, u"0.5"_s ).toDouble();
  mOpacity = elem.attribute( u"opacity"_s, u"1.0"_s ).toDouble();

  QgsAbstractMaterialSettings::readXml( elem, context );
}

void QgsClothMaterialSettings::writeXml( QDomElement &elem, const QgsReadWriteContext &context ) const
{
  elem.setAttribute( u"base"_s, QgsSymbolLayerUtils::encodeColor( mBaseColor ) );
  elem.setAttribute( u"sheen_color"_s, QgsSymbolLayerUtils::encodeColor( mSheenColor ) );
  elem.setAttribute( u"roughness"_s, mRoughness );
  if ( !qgsDoubleNear( mOpacity, 1 ) )
  {
    elem.setAttribute( u"opacity"_s, mOpacity );
  }

  QgsAbstractMaterialSettings::writeXml( elem, context );
}

QColor QgsClothMaterialSettings::averageColor() const
{
  return baseColor();
}

void QgsClothMaterialSettings::setColorsFromBase( const QColor &baseColor )
{
  setBaseColor( baseColor );
}
