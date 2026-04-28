/***************************************************************************
  qgsphongtexturedmaterialsettings.cpp
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

#include "qgsphongtexturedmaterialsettings.h"

#include "qgscolorutils.h"
#include "qgsphongmaterialsettings.h"

#include <QMap>
#include <QString>

using namespace Qt::StringLiterals;

QString QgsPhongTexturedMaterialSettings::type() const
{
  return u"phongtextured"_s;
}

bool QgsPhongTexturedMaterialSettings::supportsTechnique( Qgis::MaterialRenderingTechnique technique )
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

QgsAbstractMaterialSettings *QgsPhongTexturedMaterialSettings::create()
{
  return new QgsPhongTexturedMaterialSettings();
}

QgsPhongTexturedMaterialSettings *QgsPhongTexturedMaterialSettings::clone() const
{
  return new QgsPhongTexturedMaterialSettings( *this );
}

bool QgsPhongTexturedMaterialSettings::equals( const QgsAbstractMaterialSettings *other ) const
{
  const QgsPhongTexturedMaterialSettings *otherPhong = dynamic_cast<const QgsPhongTexturedMaterialSettings *>( other );
  if ( !otherPhong )
    return false;

  return *this == *otherPhong;
}

bool QgsPhongTexturedMaterialSettings::requiresTextureCoordinates() const
{
  return !mDiffuseTexturePath.isEmpty();
}

double QgsPhongTexturedMaterialSettings::textureRotation() const
{
  return mTextureRotation;
}

void QgsPhongTexturedMaterialSettings::readXml( const QDomElement &elem, const QgsReadWriteContext &context )
{
  mAmbient = QgsColorUtils::colorFromString( elem.attribute( u"ambient"_s, u"25,25,25"_s ) );
  mSpecular = QgsColorUtils::colorFromString( elem.attribute( u"specular"_s, u"255,255,255"_s ) );
  mShininess = elem.attribute( u"shininess"_s ).toDouble();
  mOpacity = elem.attribute( u"opacity"_s, u"1.0"_s ).toDouble();
  mDiffuseTexturePath = elem.attribute( u"diffuse_texture_path"_s, QString() );
  mTextureScale = elem.attribute( u"texture_scale"_s, QString( "1.0" ) ).toDouble();
  mTextureRotation = elem.attribute( u"texture-rotation"_s, QString( "0.0" ) ).toDouble();

  QgsAbstractMaterialSettings::readXml( elem, context );
}

void QgsPhongTexturedMaterialSettings::writeXml( QDomElement &elem, const QgsReadWriteContext &context ) const
{
  elem.setAttribute( u"ambient"_s, QgsColorUtils::colorToString( mAmbient ) );
  elem.setAttribute( u"specular"_s, QgsColorUtils::colorToString( mSpecular ) );
  elem.setAttribute( u"shininess"_s, mShininess );
  elem.setAttribute( u"opacity"_s, mOpacity );
  elem.setAttribute( u"diffuse_texture_path"_s, mDiffuseTexturePath );
  elem.setAttribute( u"texture_scale"_s, mTextureScale );
  elem.setAttribute( u"texture-rotation"_s, mTextureRotation );

  QgsAbstractMaterialSettings::writeXml( elem, context );
}
