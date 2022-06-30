/***************************************************************************
  qgsssaosettings.cpp
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

#include "qgsssaosettings.h"

#include <QDomDocument>

#include "qgsreadwritecontext.h"
#include "qgssymbollayerutils.h"

QgsSsaoSettings::QgsSsaoSettings( const QgsSsaoSettings &other )
  : mEnabled( other.mEnabled )
  , mBlurEnabled( other.mBlurEnabled )
  , mShadingFactor( other.mShadingFactor )
  , mDistanceAttenuationFactor( other.mDistanceAttenuationFactor )
  , mRadiusParameter( other.mRadiusParameter )
{

}

QgsSsaoSettings &QgsSsaoSettings::operator=( QgsSsaoSettings const &rhs )
{
  mEnabled = rhs.mEnabled;
  mBlurEnabled = rhs.mBlurEnabled;
  mShadingFactor = rhs.mShadingFactor;
  mDistanceAttenuationFactor = rhs.mDistanceAttenuationFactor;
  mRadiusParameter = rhs.mRadiusParameter;
  return *this;
}

void QgsSsaoSettings::readXml( const QDomElement &element, const QgsReadWriteContext &context )
{
  mEnabled = element.attribute( QStringLiteral( "enabled" ), QStringLiteral( "0" ) ).toInt();
  mBlurEnabled = element.attribute( QStringLiteral( "blur-enabled" ), QStringLiteral( "1" ) ).toInt();
  mShadingFactor = element.attribute( QStringLiteral( "shading-factor" ), QStringLiteral( "50.0" ) ).toFloat();
  mDistanceAttenuationFactor = element.attribute( QStringLiteral( "distance-attenuation-factor" ), QStringLiteral( "500.0" ) ).toFloat();
  mRadiusParameter = element.attribute( QStringLiteral( "radius-parameter" ), QStringLiteral( "0.05" ) ).toFloat();

  Q_UNUSED( context );
}

void QgsSsaoSettings::writeXml( QDomElement &element, const QgsReadWriteContext &context ) const
{
  element.setAttribute( QStringLiteral( "enabled" ), mEnabled );
  element.setAttribute( QStringLiteral( "blur-enabled" ), mBlurEnabled );
  element.setAttribute( QStringLiteral( "shading-factor" ), mShadingFactor );
  element.setAttribute( QStringLiteral( "distance-attenuation-factor" ), mDistanceAttenuationFactor );
  element.setAttribute( QStringLiteral( "radius-parameter" ), mRadiusParameter );

  Q_UNUSED( context );
}
