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

bool QgsServerProjectUtils::owsServiceCapabilities( const QgsProject& project )
{
  return project.readBoolEntry( QStringLiteral( "WMSServiceCapabilities" ), QStringLiteral( "/" ), false );
}

QString QgsServerProjectUtils::owsServiceTitle( const QgsProject& project )
{
  return project.readEntry( QStringLiteral( "WMSServiceTitle" ), QStringLiteral( "/" ) );
}

QString QgsServerProjectUtils::owsServiceAbstract( const QgsProject& project )
{
  return project.readEntry( QStringLiteral( "WMSServiceAbstract" ), QStringLiteral( "/" ) );
}

QStringList QgsServerProjectUtils::owsServiceKeywords( const QgsProject& project )
{
  return project.readListEntry( QStringLiteral( "WMSKeywordList" ), QStringLiteral( "/" ) );
}

QString QgsServerProjectUtils::owsServiceOnlineResource( const QgsProject& project )
{
  return project.readEntry( QStringLiteral( "WMSOnlineResource" ), QStringLiteral( "/" ) );
}

QString QgsServerProjectUtils::owsServiceContactOrganization( const QgsProject& project )
{
  return project.readEntry( QStringLiteral( "WMSContactOrganization" ), QStringLiteral( "/" ) );
}

QString QgsServerProjectUtils::owsServiceContactPosition( const QgsProject& project )
{
  return project.readEntry( QStringLiteral( "WMSContactPosition" ), QStringLiteral( "/" ) );
}

QString QgsServerProjectUtils::owsServiceContactPerson( const QgsProject& project )
{
  return project.readEntry( QStringLiteral( "WMSContactPerson" ), QStringLiteral( "/" ) );
}

QString QgsServerProjectUtils::owsServiceContactMail( const QgsProject& project )
{
  return project.readEntry( QStringLiteral( "WMSContactMail" ), QStringLiteral( "/" ) );
}

QString QgsServerProjectUtils::owsServiceContactPhone( const QgsProject& project )
{
  return project.readEntry( QStringLiteral( "WMSContactPhone" ), QStringLiteral( "/" ) );
}

QString QgsServerProjectUtils::owsServiceFees( const QgsProject& project )
{
  return project.readEntry( QStringLiteral( "WMSFees" ), QStringLiteral( "/" ) );
}

QString QgsServerProjectUtils::owsServiceAccessConstraints( const QgsProject& project )
{
  return project.readEntry( QStringLiteral( "WMSAccessConstraints" ), QStringLiteral( "/" ) );
}

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

QStringList QgsServerProjectUtils::wcsLayers( const QgsProject& project )
{
  return project.readListEntry( QStringLiteral( "WCSLayers" ), QStringLiteral( "/" ) );
}
