/***************************************************************************
  qgsphongmaterialsettings.cpp
  --------------------------------------
  Date                 : July 2017
  Copyright            : (C) 2017 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsphongmaterialsettings.h"

#include "qgscolorutils.h"

#include <QMap>
#include <QString>

using namespace Qt::StringLiterals;

QString QgsPhongMaterialSettings::type() const
{
  return u"phong"_s;
}

bool QgsPhongMaterialSettings::supportsTechnique( Qgis::MaterialRenderingTechnique technique )
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

QgsAbstractMaterialSettings *QgsPhongMaterialSettings::create()
{
  return new QgsPhongMaterialSettings();
}

QgsPhongMaterialSettings *QgsPhongMaterialSettings::clone() const
{
  return new QgsPhongMaterialSettings( *this );
}

bool QgsPhongMaterialSettings::equals( const QgsAbstractMaterialSettings *other ) const
{
  const QgsPhongMaterialSettings *otherPhong = dynamic_cast<const QgsPhongMaterialSettings *>( other );
  if ( !otherPhong )
    return false;

  return *this == *otherPhong;
}

QColor QgsPhongMaterialSettings::averageColor() const
{
  const double avgDiffuseFactor = 0.5;
  const double avgSpecularFactor = 0.1;

  double red = mAmbientCoefficient * mAmbient.redF() + mDiffuseCoefficient * avgDiffuseFactor * mDiffuse.redF() + mSpecularCoefficient * avgSpecularFactor * mSpecular.redF();

  double green = mAmbientCoefficient * mAmbient.greenF() + mDiffuseCoefficient * avgDiffuseFactor * mDiffuse.greenF() + mSpecularCoefficient * avgSpecularFactor * mSpecular.greenF();

  double blue = mAmbientCoefficient * mAmbient.blueF() + mDiffuseCoefficient * avgDiffuseFactor * mDiffuse.blueF() + mSpecularCoefficient * avgSpecularFactor * mSpecular.blueF();

  red = std::clamp( red, 0.0, 1.0 );
  green = std::clamp( green, 0.0, 1.0 );
  blue = std::clamp( blue, 0.0, 1.0 );

  return QColor::fromRgbF( static_cast<float>( red ), static_cast<float>( green ), static_cast<float>( blue ), static_cast<float>( mOpacity ) );
}

void QgsPhongMaterialSettings::setColorsFromBase( const QColor &baseColor, float metallic )
{
  metallic = std::clamp( metallic, 0.0f, 1.0f );

  const float baseR = baseColor.redF();
  const float baseG = baseColor.greenF();
  const float baseB = baseColor.blueF();

  // ambient: stable, non-directional lighting
  constexpr float AMBIENT_FACTOR = 0.2f;
  mAmbient = QColor::fromRgbF( baseR * AMBIENT_FACTOR, baseG * AMBIENT_FACTOR, baseB * AMBIENT_FACTOR );

  // F0: Fresnel reflectance at normal incidence
  constexpr float F0_DIELECTRIC = 0.04f;
  constexpr float F0_METALLIC = 0.6f;
  const float ks = F0_DIELECTRIC * ( 1.0f - metallic ) + F0_METALLIC * metallic;
  const float kd = 1.0f - ks;


  mDiffuse = QColor::fromRgbF( kd * baseR, kd * baseG, kd * baseB );

  // specular
  // * Non-metallic surfaces: Independent of base color
  // * Metallic surfaces:
  //   - Reflect their own color
  //   - Linear interpolation from white to base color as metallic increases
  mSpecular = QColor::fromRgbF( ( 1.0f - metallic ) * F0_DIELECTRIC + metallic * baseR, ( 1.0f - metallic ) * F0_DIELECTRIC + metallic * baseG, ( 1.0f - metallic ) * F0_DIELECTRIC + metallic * baseB );

  constexpr float MIN_SHININESS = 32.0f;
  constexpr float MAX_SHININESS = 200.0f;
  mShininess = MIN_SHININESS + metallic * ( MAX_SHININESS - MIN_SHININESS );
}

void QgsPhongMaterialSettings::setColorsFromBase( const QColor &baseColor )
{
  setColorsFromBase( baseColor, 0.0f );
}

void QgsPhongMaterialSettings::readXml( const QDomElement &elem, const QgsReadWriteContext &context )
{
  mAmbient = QgsColorUtils::colorFromString( elem.attribute( u"ambient"_s, u"25,25,25"_s ) );
  mDiffuse = QgsColorUtils::colorFromString( elem.attribute( u"diffuse"_s, u"178,178,178"_s ) );
  mSpecular = QgsColorUtils::colorFromString( elem.attribute( u"specular"_s, u"255,255,255"_s ) );
  mShininess = elem.attribute( u"shininess"_s ).toDouble();
  mOpacity = elem.attribute( u"opacity"_s, u"1.0"_s ).toDouble();
  mAmbientCoefficient = elem.attribute( u"ka"_s, u"1.0"_s ).toDouble();
  mDiffuseCoefficient = elem.attribute( u"kd"_s, u"1.0"_s ).toDouble();
  mSpecularCoefficient = elem.attribute( u"ks"_s, u"1.0"_s ).toDouble();

  QgsAbstractMaterialSettings::readXml( elem, context );
}

void QgsPhongMaterialSettings::writeXml( QDomElement &elem, const QgsReadWriteContext &context ) const
{
  elem.setAttribute( u"ambient"_s, QgsColorUtils::colorToString( mAmbient ) );
  elem.setAttribute( u"diffuse"_s, QgsColorUtils::colorToString( mDiffuse ) );
  elem.setAttribute( u"specular"_s, QgsColorUtils::colorToString( mSpecular ) );
  elem.setAttribute( u"shininess"_s, mShininess );
  elem.setAttribute( u"opacity"_s, mOpacity );
  elem.setAttribute( u"ka"_s, mAmbientCoefficient );
  elem.setAttribute( u"kd"_s, mDiffuseCoefficient );
  elem.setAttribute( u"ks"_s, mSpecularCoefficient );

  QgsAbstractMaterialSettings::writeXml( elem, context );
}
