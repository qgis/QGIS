/***************************************************************************
  qgspointlightsettings.cpp
  --------------------------------------
  Date                 : November 2018
  Copyright            : (C) 2018 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgspointlightsettings.h"

#include <QDomDocument>

#include "qgssymbollayerutils.h"


QDomElement QgsPointLightSettings::writeXml( QDomDocument &doc ) const
{
  QDomElement elemLight = doc.createElement( QStringLiteral( "point-light" ) );
  elemLight.setAttribute( QStringLiteral( "x" ), mPosition.x() );
  elemLight.setAttribute( QStringLiteral( "y" ), mPosition.y() );
  elemLight.setAttribute( QStringLiteral( "z" ), mPosition.z() );
  elemLight.setAttribute( QStringLiteral( "color" ), QgsSymbolLayerUtils::encodeColor( mColor ) );
  elemLight.setAttribute( QStringLiteral( "intensity" ), mIntensity );
  elemLight.setAttribute( QStringLiteral( "attenuation-0" ), mConstantAttenuation );
  elemLight.setAttribute( QStringLiteral( "attenuation-1" ), mLinearAttenuation );
  elemLight.setAttribute( QStringLiteral( "attenuation-2" ), mQuadraticAttenuation );
  return elemLight;
}

void QgsPointLightSettings::readXml( const QDomElement &elem )
{
  mPosition.set( elem.attribute( QStringLiteral( "x" ) ).toDouble(),
                 elem.attribute( QStringLiteral( "y" ) ).toDouble(),
                 elem.attribute( QStringLiteral( "z" ) ).toDouble() );
  mColor = QgsSymbolLayerUtils::decodeColor( elem.attribute( QStringLiteral( "color" ) ) );
  mIntensity = elem.attribute( QStringLiteral( "intensity" ) ).toFloat();
  mConstantAttenuation = elem.attribute( QStringLiteral( "attenuation-0" ) ).toDouble();
  mLinearAttenuation = elem.attribute( QStringLiteral( "attenuation-1" ) ).toDouble();
  mQuadraticAttenuation = elem.attribute( QStringLiteral( "attenuation-2" ) ).toDouble();
}

bool QgsPointLightSettings::operator==( const QgsPointLightSettings &other )
{
  return mPosition == other.mPosition && mColor == other.mColor && mIntensity == other.mIntensity &&
         mConstantAttenuation == other.mConstantAttenuation && mLinearAttenuation == other.mLinearAttenuation &&
         mQuadraticAttenuation == other.mQuadraticAttenuation;
}
