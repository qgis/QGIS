/***************************************************************************
  qgsgoochmaterialsettings.cpp
  --------------------------------------
  Date                 : July 2020
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

#include "qgsgoochmaterialsettings.h"

#include "qgscolorutils.h"

#include <QString>
#include <QUrl>

using namespace Qt::StringLiterals;

QString QgsGoochMaterialSettings::type() const
{
  return u"gooch"_s;
}

QgsAbstractMaterialSettings *QgsGoochMaterialSettings::create()
{
  return new QgsGoochMaterialSettings();
}

bool QgsGoochMaterialSettings::supportsTechnique( Qgis::MaterialRenderingTechnique technique )
{
  switch ( technique )
  {
    case Qgis::MaterialRenderingTechnique::Triangles:
    case Qgis::MaterialRenderingTechnique::TrianglesWithFixedTexture:
    case Qgis::MaterialRenderingTechnique::TrianglesFromModel:
    case Qgis::MaterialRenderingTechnique::TrianglesDataDefined:
      return true;

    case Qgis::MaterialRenderingTechnique::Lines:
    case Qgis::MaterialRenderingTechnique::InstancedPoints:
    case Qgis::MaterialRenderingTechnique::Points:
    case Qgis::MaterialRenderingTechnique::Billboards:
      return false;
  }
  return false;
}

QgsGoochMaterialSettings *QgsGoochMaterialSettings::clone() const
{
  return new QgsGoochMaterialSettings( *this );
}

bool QgsGoochMaterialSettings::equals( const QgsAbstractMaterialSettings *other ) const
{
  const QgsGoochMaterialSettings *otherGooch = dynamic_cast<const QgsGoochMaterialSettings *>( other );
  if ( !otherGooch )
    return false;

  return *this == *otherGooch;
}

QColor QgsGoochMaterialSettings::averageColor() const
{
  const double kDiffuse = 0.8;
  const double kSpecular = 0.2;

  double red = 0.5 * kDiffuse * ( ( mCool.redF() + mAlpha * mDiffuse.redF() ) + ( mWarm.redF() + mBeta * mDiffuse.redF() ) ) + kSpecular * mSpecular.redF();
  double green = 0.5 * kDiffuse * ( ( mCool.greenF() + mAlpha * mDiffuse.greenF() ) + ( mWarm.greenF() + mBeta * mDiffuse.greenF() ) ) + kSpecular * mSpecular.greenF();
  double blue = 0.5 * kDiffuse * ( ( mCool.blueF() + mAlpha * mDiffuse.blueF() ) + ( mWarm.blueF() + mBeta * mDiffuse.blueF() ) ) + kSpecular * mSpecular.blueF();

  red = std::clamp( red, 0.0, 1.0 );
  green = std::clamp( green, 0.0, 1.0 );
  blue = std::clamp( blue, 0.0, 1.0 );

  return QColor::fromRgbF( static_cast<float>( red ), static_cast<float>( green ), static_cast<float>( blue ) );
}

void QgsGoochMaterialSettings::readXml( const QDomElement &elem, const QgsReadWriteContext &context )
{
  mWarm = QgsColorUtils::colorFromString( elem.attribute( u"warm"_s, u"107,0,107"_s ) );
  mCool = QgsColorUtils::colorFromString( elem.attribute( u"cool"_s, u"255,130,0"_s ) );
  mDiffuse = QgsColorUtils::colorFromString( elem.attribute( u"diffuse"_s, u"178,178,178"_s ) );
  mSpecular = QgsColorUtils::colorFromString( elem.attribute( u"specular"_s ) );
  mShininess = elem.attribute( u"shininess2"_s, u"100"_s ).toDouble();
  mAlpha = elem.attribute( u"alpha"_s, u"0.25"_s ).toDouble();
  mBeta = elem.attribute( u"beta"_s, u"0.5"_s ).toDouble();

  QgsAbstractMaterialSettings::readXml( elem, context );
}

void QgsGoochMaterialSettings::writeXml( QDomElement &elem, const QgsReadWriteContext &context ) const
{
  elem.setAttribute( u"warm"_s, QgsColorUtils::colorToString( mWarm ) );
  elem.setAttribute( u"cool"_s, QgsColorUtils::colorToString( mCool ) );
  elem.setAttribute( u"diffuse"_s, QgsColorUtils::colorToString( mDiffuse ) );
  elem.setAttribute( u"specular"_s, QgsColorUtils::colorToString( mSpecular ) );
  elem.setAttribute( u"shininess2"_s, mShininess );
  elem.setAttribute( u"alpha"_s, mAlpha );
  elem.setAttribute( u"beta"_s, mBeta );

  QgsAbstractMaterialSettings::writeXml( elem, context );
}
