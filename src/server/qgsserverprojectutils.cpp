/***************************************************************************
                              qgsserverprojectutils.cpp
                              -------------------------
  begin                : December 19, 2016
  copyright            : (C) 2016 by Paul Blottiere
  email                : paul dot blottiere at oslandia dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsserverprojectutils.h"

int QgsServerProjectUtils::wmsMaxWidth( const QgsProject& project )
{
  return project.readNumEntry( QStringLiteral( "WMSMaxWidth" ), QStringLiteral( "/" ), -1 );
}

int QgsServerProjectUtils::wmsMaxHeight( const QgsProject& project )
{
  return project.readNumEntry( QStringLiteral( "WMSMaxHeight" ), QStringLiteral( "/" ), -1 );
}

QString QgsServerProjectUtils::wmsServiceUrl( const QgsProject& project )
{
  return project.readEntry( QStringLiteral( "WMSUrl" ), QStringLiteral( "/" ), "" );
}

QString QgsServerProjectUtils::wfsServiceUrl( const QgsProject& project )
{
  return project.readEntry( QStringLiteral( "WFSUrl" ), QStringLiteral( "/" ), "" );
}

QString QgsServerProjectUtils::wcsServiceUrl( const QgsProject& project )
{
  return project.readEntry( QStringLiteral( "WCSUrl" ), QStringLiteral( "/" ), "" );
}
