/***************************************************************************
  qgssunlightsettings.cpp
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

#include "qgssunlightsettings.h"

#include "qgs3dmapsettings.h"
#include "qgscolorutils.h"
#include "qgssunpositioncalculator.h"
#include "qgssymbollayerutils.h"

#include <QDomDocument>
#include <QString>
#include <Qt3DCore/QEntity>
#include <Qt3DExtras/QPhongMaterial>
#include <Qt3DExtras/QSphereMesh>
#include <Qt3DRender/QDirectionalLight>

using namespace Qt::StringLiterals;

Qgis::LightSourceType QgsSunLightSettings::type() const
{
  return Qgis::LightSourceType::Sun;
}

QgsSunLightSettings *QgsSunLightSettings::clone() const
{
  return new QgsSunLightSettings( *this );
}

Qt3DCore::QEntity *QgsSunLightSettings::createEntity( const Qgs3DMapSettings &map, Qt3DCore::QEntity *parent ) const
{
  Qt3DCore::QEntity *lightEntity = new Qt3DCore::QEntity( parent );

  Qt3DRender::QDirectionalLight *light = new Qt3DRender::QDirectionalLight;
  light->setColor( color() );
  light->setIntensity( static_cast< float >( intensity() ) );

  QgsSunPositionResult sunResult;
  try
  {
    sunResult = QgsSunPositionCalculator::calculate( map.extent().center(), map.crs(), map.transformContext(), mSunTime, mReferenceElevation, mAtmosphericPressure, mTemperature );
  }
  catch ( const QgsException & )
  {
    // fallback to straight down (noon) if CRS transformation or math fails
    sunResult.azimuth = 0.0;
    sunResult.apparentElevation = 90.0;
  }

  const float azimuthRad = M_PI * sunResult.azimuth / 180.0;
  const float elevationRad = M_PI * sunResult.apparentElevation / 180.0;
  // convert to vector in z-up world
  const float x = std::cos( elevationRad ) * std::sin( azimuthRad );
  const float y = std::cos( elevationRad ) * std::cos( azimuthRad );
  const float z = std::sin( elevationRad );
  const QVector3D sunVector( x, y, z );
  light->setWorldDirection( -sunVector.normalized() );

  lightEntity->addComponent( light );

  return lightEntity;
}

QDomElement QgsSunLightSettings::writeXml( QDomDocument &doc, const QgsReadWriteContext & ) const
{
  QDomElement elemLight = doc.createElement( u"sun-light"_s );
  elemLight.setAttribute( u"color"_s, QgsColorUtils::colorToString( mColor ) );
  elemLight.setAttribute( u"intensity"_s, mIntensity );
  elemLight.setAttribute( u"sun-time"_s, mSunTime.toString( Qt::ISODate ) );
  elemLight.setAttribute( u"reference-elevation"_s, mReferenceElevation );
  elemLight.setAttribute( u"atmospheric-pressure"_s, mAtmosphericPressure );
  elemLight.setAttribute( u"temperature"_s, mTemperature );

  return elemLight;
}

void QgsSunLightSettings::readXml( const QDomElement &elem, const QgsReadWriteContext & )
{
  mColor = QgsColorUtils::colorFromString( elem.attribute( u"color"_s ) );
  mIntensity = elem.attribute( u"intensity"_s ).toFloat();
  mSunTime = QDateTime::fromString( elem.attribute( u"sun-time"_s ), Qt::ISODate );
  if ( !mSunTime.isValid() )
  {
    mSunTime = QDateTime::currentDateTimeUtc();
  }
  mReferenceElevation = elem.attribute( u"reference-elevation"_s, u"0.0"_s ).toDouble();
  mAtmosphericPressure = elem.attribute( u"atmospheric-pressure"_s, u"1013.25"_s ).toDouble();
  mTemperature = elem.attribute( u"temperature"_s, u"15.0"_s ).toDouble();
}

bool QgsSunLightSettings::operator==( const QgsSunLightSettings &other ) const
{
  return mSunTime == other.mSunTime
         && mColor == other.mColor
         && mIntensity == other.mIntensity
         && qgsDoubleNear( mAtmosphericPressure, other.mAtmosphericPressure )
         && qgsDoubleNear( mTemperature, other.mTemperature )
         && qgsDoubleNear( mReferenceElevation, other.mReferenceElevation );
}
