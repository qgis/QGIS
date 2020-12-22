/***************************************************************************
  qgsdirectionallightsettings.cpp
  --------------------------------------
  Date                 : June 2020
  Copyright            : (C) 2020 by Belgacem Nedjima
  Email                : gb underscore nedjima at esi dot dz
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsdirectionallightsettings.h"

#include <QDomDocument>

#include "qgssymbollayerutils.h"


QDomElement QgsDirectionalLightSettings::writeXml( QDomDocument &doc ) const
{
  QDomElement elemLight = doc.createElement( QStringLiteral( "directional-light" ) );
  elemLight.setAttribute( QStringLiteral( "x" ), mDirection.x() );
  elemLight.setAttribute( QStringLiteral( "y" ), mDirection.y() );
  elemLight.setAttribute( QStringLiteral( "z" ), mDirection.z() );
  elemLight.setAttribute( QStringLiteral( "color" ), QgsSymbolLayerUtils::encodeColor( mColor ) );
  elemLight.setAttribute( QStringLiteral( "intensity" ), mIntensity );
  return elemLight;
}

void QgsDirectionalLightSettings::readXml( const QDomElement &elem )
{
  mDirection.set( elem.attribute( QStringLiteral( "x" ) ).toFloat(),
                  elem.attribute( QStringLiteral( "y" ) ).toFloat(),
                  elem.attribute( QStringLiteral( "z" ) ).toFloat() );
  mColor = QgsSymbolLayerUtils::decodeColor( elem.attribute( QStringLiteral( "color" ) ) );
  mIntensity = elem.attribute( QStringLiteral( "intensity" ) ).toFloat();
}

bool QgsDirectionalLightSettings::operator==( const QgsDirectionalLightSettings &other )
{
  return mDirection == other.mDirection && mColor == other.mColor && mIntensity == other.mIntensity;
}
