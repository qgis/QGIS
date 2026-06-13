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

QSet<QgsAbstractMaterialSettings::Property> QgsMetalRoughMaterialSettings::supportedProperties() const
{
  return { QgsAbstractMaterialSettings::Property::BaseColor, QgsAbstractMaterialSettings::Property::EmissionColor };
}

bool QgsMetalRoughMaterialSettings::supportsTechnique( Qgis::MaterialRenderingTechnique technique )
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

bool QgsMetalRoughMaterialSettings::requiresTangents() const
{
  return mAnisotropy > 0;
}

void QgsMetalRoughMaterialSettings::readXml( const QDomElement &elem, const QgsReadWriteContext &context )
{
  mBaseColor = QgsSymbolLayerUtils::decodeColor( elem.attribute( u"base"_s, u"125,125,125"_s ) );
  if ( elem.hasAttribute( u"emission_color"_s ) )
    mEmissiveColor = QgsSymbolLayerUtils::decodeColor( elem.attribute( u"emission_color"_s ) );
  else
    mEmissiveColor = QColor();
  mEmissionFactor = elem.attribute( u"emission_factor"_s, u"1.0"_s ).toDouble();

  mClearCoatFactor = elem.attribute( u"clear_coat_factor"_s, u"0.0"_s ).toDouble();
  mClearCoatRoughness = elem.attribute( u"clear_coat_roughness"_s, u"0.0"_s ).toDouble();

  mMetalness = elem.attribute( u"metalness"_s, u"0.0"_s ).toDouble();
  mRoughness = elem.attribute( u"roughness"_s, u"0.5"_s ).toDouble();
  mOpacity = elem.attribute( u"opacity"_s, u"1.0"_s ).toDouble();
  mReflectance = elem.attribute( u"reflectance"_s, u"0.5"_s ).toDouble();
  mAnisotropy = elem.attribute( u"anisotropy"_s, u"0.0"_s ).toDouble();
  mAnisotropyRotation = elem.attribute( u"anisotropy_rotation"_s, u"0.0"_s ).toDouble();

  QgsAbstractMaterialSettings::readXml( elem, context );
}

void QgsMetalRoughMaterialSettings::writeXml( QDomElement &elem, const QgsReadWriteContext &context ) const
{
  elem.setAttribute( u"base"_s, QgsSymbolLayerUtils::encodeColor( mBaseColor ) );
  elem.setAttribute( u"metalness"_s, mMetalness );
  elem.setAttribute( u"roughness"_s, mRoughness );
  if ( !qgsDoubleNear( mReflectance, 0.5 ) )
  {
    elem.setAttribute( u"reflectance"_s, mReflectance );
  }
  if ( !qgsDoubleNear( mAnisotropy, 0.0 ) )
  {
    elem.setAttribute( u"anisotropy"_s, mAnisotropy );
  }
  if ( !qgsDoubleNear( mAnisotropyRotation, 0.0 ) )
  {
    elem.setAttribute( u"anisotropy_rotation"_s, mAnisotropyRotation );
  }
  if ( mEmissiveColor.isValid() )
    elem.setAttribute( u"emission_color"_s, QgsSymbolLayerUtils::encodeColor( mEmissiveColor ) );
  if ( !qgsDoubleNear( mEmissionFactor, 1.0 ) )
  {
    elem.setAttribute( u"emission_factor"_s, mEmissionFactor );
  }

  if ( !qgsDoubleNear( mClearCoatFactor, 0.0 ) )
  {
    elem.setAttribute( u"clear_coat_factor"_s, mClearCoatFactor );
  }
  if ( !qgsDoubleNear( mClearCoatRoughness, 0.0 ) )
  {
    elem.setAttribute( u"clear_coat_roughness"_s, mClearCoatRoughness );
  }

  if ( !qgsDoubleNear( mOpacity, 1 ) )
  {
    elem.setAttribute( u"opacity"_s, mOpacity );
  }

  QgsAbstractMaterialSettings::writeXml( elem, context );
}

QColor QgsMetalRoughMaterialSettings::averageColor() const
{
  return baseColor();
}

void QgsMetalRoughMaterialSettings::setColorsFromBase( const QColor &baseColor )
{
  setBaseColor( baseColor );
}
