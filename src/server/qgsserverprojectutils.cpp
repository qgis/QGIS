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

bool QgsServerProjectUtils::owsServiceCapabilities( const QgsProject &project )
{
  return project.readBoolEntry( QStringLiteral( "WMSServiceCapabilities" ), QStringLiteral( "/" ), false );
}

QString QgsServerProjectUtils::owsServiceTitle( const QgsProject &project )
{
  return project.readEntry( QStringLiteral( "WMSServiceTitle" ), QStringLiteral( "/" ) );
}

QString QgsServerProjectUtils::owsServiceAbstract( const QgsProject &project )
{
  return project.readEntry( QStringLiteral( "WMSServiceAbstract" ), QStringLiteral( "/" ) );
}

QStringList QgsServerProjectUtils::owsServiceKeywords( const QgsProject &project )
{
  return project.readListEntry( QStringLiteral( "WMSKeywordList" ), QStringLiteral( "/" ) );
}

QString QgsServerProjectUtils::owsServiceOnlineResource( const QgsProject &project )
{
  return project.readEntry( QStringLiteral( "WMSOnlineResource" ), QStringLiteral( "/" ) );
}

QString QgsServerProjectUtils::owsServiceContactOrganization( const QgsProject &project )
{
  return project.readEntry( QStringLiteral( "WMSContactOrganization" ), QStringLiteral( "/" ) );
}

QString QgsServerProjectUtils::owsServiceContactPosition( const QgsProject &project )
{
  return project.readEntry( QStringLiteral( "WMSContactPosition" ), QStringLiteral( "/" ) );
}

QString QgsServerProjectUtils::owsServiceContactPerson( const QgsProject &project )
{
  return project.readEntry( QStringLiteral( "WMSContactPerson" ), QStringLiteral( "/" ) );
}

QString QgsServerProjectUtils::owsServiceContactMail( const QgsProject &project )
{
  return project.readEntry( QStringLiteral( "WMSContactMail" ), QStringLiteral( "/" ) );
}

QString QgsServerProjectUtils::owsServiceContactPhone( const QgsProject &project )
{
  return project.readEntry( QStringLiteral( "WMSContactPhone" ), QStringLiteral( "/" ) );
}

QString QgsServerProjectUtils::owsServiceFees( const QgsProject &project )
{
  return project.readEntry( QStringLiteral( "WMSFees" ), QStringLiteral( "/" ) );
}

QString QgsServerProjectUtils::owsServiceAccessConstraints( const QgsProject &project )
{
  return project.readEntry( QStringLiteral( "WMSAccessConstraints" ), QStringLiteral( "/" ) );
}

int QgsServerProjectUtils::wmsMaxWidth( const QgsProject &project )
{
  return project.readNumEntry( QStringLiteral( "WMSMaxWidth" ), QStringLiteral( "/" ), -1 );
}

int QgsServerProjectUtils::wmsMaxHeight( const QgsProject &project )
{
  return project.readNumEntry( QStringLiteral( "WMSMaxHeight" ), QStringLiteral( "/" ), -1 );
}

bool QgsServerProjectUtils::wmsUseLayerIds( const QgsProject &project )
{
  return project.readBoolEntry( QStringLiteral( "WMSUseLayerIDs" ), QStringLiteral( "/" ) );
}

bool QgsServerProjectUtils::wmsInfoFormatSIA2045( const QgsProject &project )
{
  QString sia2045 = project.readEntry( QStringLiteral( "WMSInfoFormatSIA2045" ), QStringLiteral( "/" ), "" );

  if ( sia2045.compare( QLatin1String( "enabled" ), Qt::CaseInsensitive ) == 0
       || sia2045.compare( QLatin1String( "true" ), Qt::CaseInsensitive ) == 0 )
  {
    return true;
  }
  return false;
}

bool QgsServerProjectUtils::wmsInspireActivated( const QgsProject &project )
{
  return project.readBoolEntry( QStringLiteral( "WMSInspire" ), QStringLiteral( "/activated" ) );
}

QString QgsServerProjectUtils::wmsInspireLanguage( const QgsProject &project )
{
  return project.readEntry( QStringLiteral( "WMSInspire" ), QStringLiteral( "/language" ) );
}

QString QgsServerProjectUtils::wmsInspireMetadataUrl( const QgsProject &project )
{
  return project.readEntry( QStringLiteral( "WMSInspire" ), QStringLiteral( "/metadataUrl" ) );
}

QString QgsServerProjectUtils::wmsInspireMetadataUrlType( const QgsProject &project )
{
  return project.readEntry( QStringLiteral( "WMSInspire" ), QStringLiteral( "/metadataUrlType" ) );
}

QString QgsServerProjectUtils::wmsInspireTemporalReference( const QgsProject &project )
{
  return project.readEntry( QStringLiteral( "WMSInspire" ), QStringLiteral( "/temporalReference" ) );
}

QString QgsServerProjectUtils::wmsInspireMetadataDate( const QgsProject &project )
{
  return project.readEntry( QStringLiteral( "WMSInspire" ), QStringLiteral( "/metadataDate" ) );
}

QStringList QgsServerProjectUtils::wmsRestrictedComposers( const QgsProject &project )
{
  return project.readListEntry( QStringLiteral( "WMSRestrictedComposers" ), QStringLiteral( "/" ), QStringList() );
}

QString QgsServerProjectUtils::wmsServiceUrl( const QgsProject &project )
{
  return project.readEntry( QStringLiteral( "WMSUrl" ), QStringLiteral( "/" ), "" );
}

QString QgsServerProjectUtils::wfsServiceUrl( const QgsProject &project )
{
  return project.readEntry( QStringLiteral( "WFSUrl" ), QStringLiteral( "/" ), "" );
}

QStringList QgsServerProjectUtils::wfsLayerIds( const QgsProject &project )
{
  return project.readListEntry( QStringLiteral( "WFSLayers" ), QStringLiteral( "/" ) );
}

int QgsServerProjectUtils::wfsLayerPrecision( const QString &layerId, const QgsProject &project )
{
  return project.readNumEntry( QStringLiteral( "WFSLayersPrecision" ), "/" + layerId, 6 );
}

QStringList QgsServerProjectUtils::wfstUpdateLayerIds( const QgsProject &project )
{
  return project.readListEntry( QStringLiteral( "WFSTLayers" ), QStringLiteral( "Update" ) );
}

QStringList QgsServerProjectUtils::wfstInsertLayerIds( const QgsProject &project )
{
  return project.readListEntry( QStringLiteral( "WFSTLayers" ), QStringLiteral( "Insert" ) );
}

QStringList QgsServerProjectUtils::wfstDeleteLayerIds( const QgsProject &project )
{
  return project.readListEntry( QStringLiteral( "WFSTLayers" ), QStringLiteral( "Delete" ) );
}

QString QgsServerProjectUtils::wcsServiceUrl( const QgsProject &project )
{
  return project.readEntry( QStringLiteral( "WCSUrl" ), QStringLiteral( "/" ), "" );
}

QStringList QgsServerProjectUtils::wcsLayers( const QgsProject &project )
{
  return project.readListEntry( QStringLiteral( "WCSLayers" ), QStringLiteral( "/" ) );
}
