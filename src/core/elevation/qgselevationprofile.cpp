/***************************************************************************
    qgselevationprofile.cpp
    ------------------
    Date                 : July 2025
    Copyright            : (C) 2025 Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgselevationprofile.h"
#include "qgsproject.h"
#include "moc_qgselevationprofile.cpp"

QgsElevationProfile::QgsElevationProfile( QgsProject *project )
  : mProject( project )
{

}

QDomElement QgsElevationProfile::writeXml( QDomDocument &document, const QgsReadWriteContext & ) const
{
  QDomElement profileElem = document.createElement( QStringLiteral( "ElevationProfile" ) );
  profileElem.setAttribute( QStringLiteral( "name" ), mName );
  return profileElem;
}

bool QgsElevationProfile::readXml( const QDomElement &element, const QDomDocument &, const QgsReadWriteContext & )
{
  if ( element.nodeName() != QLatin1String( "ElevationProfile" ) )
  {
    return false;
  }

  setName( element.attribute( QStringLiteral( "name" ) ) );

  return true;
}

void QgsElevationProfile::setName( const QString &name )
{
  if ( name == mName )
    return;

  mName = name;
  emit nameChanged( mName );
}
