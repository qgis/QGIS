/***************************************************************************
  qgambientocclusionsettings.cpp
  --------------------------------------
  Date                 : June 2022
  Copyright            : (C) 2022 by Belgacem Nedjima
  Email                : belgacem dot nedjima at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsambientocclusionsettings.h"

#include <QDomDocument>

#include "qgsreadwritecontext.h"
#include "qgssymbollayerutils.h"

QgsAmbientOcclusionSettings::QgsAmbientOcclusionSettings( const QgsAmbientOcclusionSettings &other )
  : mEnabled( other.mEnabled )
  , mIntensity( other.mIntensity )
  , mRadius( other.mRadius )
{

}

QgsAmbientOcclusionSettings &QgsAmbientOcclusionSettings::operator=( QgsAmbientOcclusionSettings const &rhs )
{
  mEnabled = rhs.mEnabled;
  mIntensity = rhs.mIntensity;
  mRadius = rhs.mRadius;
  return *this;
}

void QgsAmbientOcclusionSettings::readXml( const QDomElement &element, const QgsReadWriteContext &context )
{
  mEnabled = element.attribute( QStringLiteral( "enabled" ), QStringLiteral( "0" ) ).toInt();
  mIntensity = element.attribute( QStringLiteral( "intensity" ), QStringLiteral( "1.0" ) ).toFloat();
  mRadius = element.attribute( QStringLiteral( "radius" ), QStringLiteral( "10" ) ).toFloat();

  Q_UNUSED( context );
}

void QgsAmbientOcclusionSettings::writeXml( QDomElement &element, const QgsReadWriteContext &context ) const
{
  element.setAttribute( QStringLiteral( "enabled" ), mEnabled );
  element.setAttribute( QStringLiteral( "intensity" ), mIntensity );
  element.setAttribute( QStringLiteral( "radius" ), mRadius );

  Q_UNUSED( context );
}
