/***************************************************************************
  qgsmetalroughtexturedmaterialsettings.cpp
  --------------------------------------
  Date                 : April 2026
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

#include "qgsmetalroughtexturedmaterialsettings.h"

#include "qgssymbollayerutils.h"

#include <QString>

using namespace Qt::StringLiterals;

QString QgsMetalRoughTexturedMaterialSettings::type() const
{
  return u"metalroughtextured"_s;
}

bool QgsMetalRoughTexturedMaterialSettings::supportsTechnique( Qgis::MaterialRenderingTechnique technique )
{
  switch ( technique )
  {
    case Qgis::MaterialRenderingTechnique::Triangles:
    case Qgis::MaterialRenderingTechnique::TrianglesDataDefined: //technique is supported but color can't be datadefined
      return true;

    case Qgis::MaterialRenderingTechnique::Points:
    case Qgis::MaterialRenderingTechnique::TrianglesWithFixedTexture:
    case Qgis::MaterialRenderingTechnique::TrianglesFromModel:
    case Qgis::MaterialRenderingTechnique::InstancedPoints:
    case Qgis::MaterialRenderingTechnique::Lines:
    case Qgis::MaterialRenderingTechnique::Billboards:
      return false;
  }
  return false;
}

QgsAbstractMaterialSettings *QgsMetalRoughTexturedMaterialSettings::create()
{
  return new QgsMetalRoughTexturedMaterialSettings();
}

QgsMetalRoughTexturedMaterialSettings *QgsMetalRoughTexturedMaterialSettings::clone() const
{
  return new QgsMetalRoughTexturedMaterialSettings( *this );
}

bool QgsMetalRoughTexturedMaterialSettings::equals( const QgsAbstractMaterialSettings *other ) const
{
  const QgsMetalRoughTexturedMaterialSettings *otherMetal = dynamic_cast<const QgsMetalRoughTexturedMaterialSettings *>( other );
  if ( !otherMetal )
    return false;

  return *this == *otherMetal;
}

bool QgsMetalRoughTexturedMaterialSettings::requiresTextureCoordinates() const
{
  return true;
}

bool QgsMetalRoughTexturedMaterialSettings::requiresTangents() const
{
  return !mNormalTexturePath.isEmpty() || !mHeightTexturePath.isEmpty();
}

void QgsMetalRoughTexturedMaterialSettings::readXml( const QDomElement &elem, const QgsReadWriteContext &context )
{
  mBaseColorTexturePath = elem.attribute( u"base_color_texture_path"_s, QString() );
  mMetalnessTexturePath = elem.attribute( u"metalness_texture_path"_s, QString() );
  mRoughnessTexturePath = elem.attribute( u"roughness_texture_path"_s, QString() );
  mNormalTexturePath = elem.attribute( u"normal_texture_path"_s, QString() );
  mHeightTexturePath = elem.attribute( u"height_texture_path"_s, QString() );
  mAmbientOcclusionTexturePath = elem.attribute( u"ambient_occlusion_texture_path"_s, QString() );
  mEmissionTexturePath = elem.attribute( u"emission_texture_path"_s, QString() );
  mEmissionFactor = elem.attribute( u"emission_factor"_s, QString( "1.0" ) ).toDouble();
  mParallaxScale = elem.attribute( u"parallax_scale"_s, QString( "0.1" ) ).toDouble();
  mTextureScale = elem.attribute( u"texture_scale"_s, QString( "1.0" ) ).toDouble();
  mTextureRotation = elem.attribute( u"texture_rotation"_s, QString( "0.0" ) ).toDouble();
  mOpacity = elem.attribute( u"opacity"_s, u"1.0"_s ).toDouble();

  QgsAbstractMaterialSettings::readXml( elem, context );
}

void QgsMetalRoughTexturedMaterialSettings::writeXml( QDomElement &elem, const QgsReadWriteContext &context ) const
{
  elem.setAttribute( u"base_color_texture_path"_s, mBaseColorTexturePath );
  elem.setAttribute( u"metalness_texture_path"_s, mMetalnessTexturePath );
  elem.setAttribute( u"roughness_texture_path"_s, mRoughnessTexturePath );
  elem.setAttribute( u"normal_texture_path"_s, mNormalTexturePath );
  elem.setAttribute( u"parallax_scale"_s, mParallaxScale );
  elem.setAttribute( u"height_texture_path"_s, mHeightTexturePath );
  elem.setAttribute( u"emission_texture_path"_s, mEmissionTexturePath );
  elem.setAttribute( u"ambient_occlusion_texture_path"_s, mAmbientOcclusionTexturePath );
  if ( !qgsDoubleNear( mEmissionFactor, 1.0 ) )
    elem.setAttribute( u"emission_factor"_s, mEmissionFactor );
  elem.setAttribute( u"texture_scale"_s, mTextureScale );
  elem.setAttribute( u"texture_rotation"_s, mTextureRotation );
  if ( !qgsDoubleNear( mOpacity, 1 ) )
    elem.setAttribute( u"opacity"_s, mOpacity );

  QgsAbstractMaterialSettings::writeXml( elem, context );
}
