/***************************************************************************
    qgswmsconnection.cpp  -  selector for WMS servers, etc.
                             -------------------
    begin                : 3 April 2005
    copyright            :
    original             : (C) 2005 by Brendan Morley email  : morb at ozemail dot com dot au
    wms search           : (C) 2009 Mathias Walker <mwa at sourcepole.ch>, Sourcepole AG
    wms-c support        : (C) 2010 Juergen E. Fischer < jef at norbit dot de >, norBIT GmbH

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "../providers/wms/qgswmsprovider.h"
#include "qgis.h" // GEO_EPSG_CRS_ID
#include "qgsdatasourceuri.h"
#include "qgsproject.h"
#include "qgsproviderregistry.h"
#include "qgswmsconnection.h"
#include "qgsnetworkaccessmanager.h"
#include "qgssettings.h"

QgsWMSConnection::QgsWMSConnection( const QString &connName )
  : mConnName( connName )
{
  QgsDebugMsg( "theConnName = " + connName );

  QgsSettings settings;

  QString key = "qgis/connections-wms/" + mConnName;
  QString credentialsKey = "qgis/WMS/" + mConnName;

  QStringList connStringParts;

  mUri.setParam( QStringLiteral( "url" ), settings.value( key + "/url" ).toString() );

  // Check for credentials and prepend to the connection info
  QString username = settings.value( credentialsKey + "/username" ).toString();
  QString password = settings.value( credentialsKey + "/password" ).toString();
  if ( !username.isEmpty() )
  {
    mUri.setParam( QStringLiteral( "username" ), username );
    mUri.setParam( QStringLiteral( "password" ), password );
  }

  QString authcfg = settings.value( credentialsKey + "/authcfg" ).toString();
  if ( !authcfg.isEmpty() )
  {
    mUri.setParam( QStringLiteral( "authcfg" ), authcfg );
  }

  QString referer = settings.value( key + "/referer" ).toString();
  if ( !referer.isEmpty() )
  {
    mUri.setParam( QStringLiteral( "referer" ), referer );
  }

  bool ignoreGetMap = settings.value( key + "/ignoreGetMapURI", false ).toBool();
  bool ignoreGetFeatureInfo = settings.value( key + "/ignoreGetFeatureInfoURI", false ).toBool();
  bool ignoreAxisOrientation = settings.value( key + "/ignoreAxisOrientation", false ).toBool();
  bool invertAxisOrientation = settings.value( key + "/invertAxisOrientation", false ).toBool();
  bool smoothPixmapTransform = settings.value( key + "/smoothPixmapTransform", false ).toBool();
  QString dpiMode = settings.value( key + "/dpiMode", "all" ).toString();

  if ( ignoreGetMap )
  {
    mUri.setParam( QStringLiteral( "IgnoreGetMapUrl" ), QStringLiteral( "1" ) );
  }

  if ( ignoreGetFeatureInfo )
  {
    mUri.setParam( QStringLiteral( "IgnoreGetFeatureInfoUrl" ), QStringLiteral( "1" ) );
  }

  if ( ignoreAxisOrientation )
  {
    mUri.setParam( QStringLiteral( "IgnoreAxisOrientation" ), QStringLiteral( "1" ) );
  }

  if ( invertAxisOrientation )
  {
    mUri.setParam( QStringLiteral( "InvertAxisOrientation" ), QStringLiteral( "1" ) );
  }

  if ( smoothPixmapTransform )
  {
    mUri.setParam( QStringLiteral( "SmoothPixmapTransform" ), QStringLiteral( "1" ) );
  }

  if ( !dpiMode.isEmpty() )
  {
    mUri.setParam( QStringLiteral( "dpiMode" ), dpiMode );
  }

  QgsDebugMsg( QString( "encodedUri: '%1'." ).arg( QString( mUri.encodedUri() ) ) );
}

QgsWMSConnection::~QgsWMSConnection()
{

}

QgsDataSourceUri QgsWMSConnection::uri()
{
  return mUri;
}

QStringList QgsWMSConnection::connectionList()
{
  QgsSettings settings;
  settings.beginGroup( QStringLiteral( "qgis/connections-wms" ) );
  return settings.childGroups();
}

QString QgsWMSConnection::selectedConnection()
{
  QgsSettings settings;
  return settings.value( QStringLiteral( "qgis/connections-wms/selected" ) ).toString();
}

void QgsWMSConnection::setSelectedConnection( const QString &name )
{
  QgsSettings settings;
  settings.setValue( QStringLiteral( "qgis/connections-wms/selected" ), name );
}

void QgsWMSConnection::deleteConnection( const QString &name )
{
  QgsSettings settings;
  settings.remove( "qgis/connections-wms/" + name );
  settings.remove( "qgis/WMS/" + name );
}
