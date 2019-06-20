/***************************************************************************
    qgsowsconnection.cpp  -  selector for WMS servers, etc.
                             -------------------
    begin                : 3 April 2005
    copyright            :
    original             : (C) 2005 by Brendan Morley email  : morb at ozemail dot com dot au
    wms search           : (C) 2009 Mathias Walker <mwa at sourcepole.ch>, Sourcepole AG
    wms-c support        : (C) 2010 Juergen E. Fischer < jef at norbit dot de >, norBIT GmbH
    generalized          : (C) 2012 Radim Blazek, based on qgswmsconnection.cpp

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgis.h" // GEO_EPSG_CRS_ID
#include "qgsdatasourceuri.h"
#include "qgslogger.h"
#include "qgsproject.h"
#include "qgsproviderregistry.h"
#include "qgsowsconnection.h"
#include "qgssettings.h"

#include <QInputDialog>
#include <QMessageBox>
#include <QPicture>
#include <QUrl>
#include <QNetworkRequest>
#include <QNetworkReply>

QgsOwsConnection::QgsOwsConnection( const QString &service, const QString &connName )
  : mConnName( connName )
  , mService( service )
{
  QgsDebugMsgLevel( "theConnName = " + connName, 4 );

  QgsSettings settings;

  QString key = "qgis/connections-" + mService.toLower() + '/' + mConnName;
  QString credentialsKey = "qgis/" + mService + '/' + mConnName;

  mConnectionInfo = settings.value( key + "/url" ).toString();
  mUri.setParam( QStringLiteral( "url" ), settings.value( key + "/url" ).toString() );

  // Check for credentials and prepend to the connection info
  QString username = settings.value( credentialsKey + "/username" ).toString();
  QString password = settings.value( credentialsKey + "/password" ).toString();
  if ( !username.isEmpty() )
  {
    // check for a password, if none prompt to get it
    mUri.setParam( QStringLiteral( "username" ), username );
    mUri.setParam( QStringLiteral( "password" ), password );
  }

  QString authcfg = settings.value( credentialsKey + "/authcfg" ).toString();
  if ( !authcfg.isEmpty() )
  {
    mUri.setParam( QStringLiteral( "authcfg" ), authcfg );
  }
  mConnectionInfo.append( ",authcfg=" + authcfg );

  const QString referer = settings.value( key + "/referer" ).toString();
  if ( !referer.isEmpty() )
  {
    mUri.setParam( QStringLiteral( "referer" ), referer );
    mConnectionInfo.append( ",referer=" + referer );
  }

  if ( mService.compare( QLatin1String( "WMS" ), Qt::CaseInsensitive ) == 0
       || mService.compare( QLatin1String( "WCS" ), Qt::CaseInsensitive ) == 0 )
  {
    addWmsWcsConnectionSettings( mUri, key );
  }
  else if ( mService.compare( QLatin1String( "WFS" ), Qt::CaseInsensitive ) == 0 )
  {
    addWfsConnectionSettings( mUri, key );
  }

  QgsDebugMsgLevel( QStringLiteral( "encoded uri: '%1'." ).arg( QString( mUri.encodedUri() ) ), 4 );
}

QString QgsOwsConnection::connectionName() const
{
  return mConnName;
}

QString QgsOwsConnection::connectionInfo() const
{
  return mConnectionInfo;
}

QString QgsOwsConnection::service() const
{
  return mService;
}

QgsDataSourceUri QgsOwsConnection::uri() const
{
  return mUri;
}

QgsDataSourceUri &QgsOwsConnection::addWmsWcsConnectionSettings( QgsDataSourceUri &uri, const QString &settingsKey )
{
  addCommonConnectionSettings( uri, settingsKey );

  QgsSettings settings;
  QString referer = settings.value( settingsKey + "/referer" ).toString();
  if ( !referer.isEmpty() )
  {
    uri.setParam( QStringLiteral( "referer" ), referer );
  }
  if ( settings.value( settingsKey + QStringLiteral( "/ignoreGetMapURI" ), false ).toBool() )
  {
    uri.setParam( QStringLiteral( "IgnoreGetMapUrl" ), QStringLiteral( "1" ) );
  }
  if ( settings.value( settingsKey + QStringLiteral( "/ignoreGetFeatureInfoURI" ), false ).toBool() )
  {
    uri.setParam( QStringLiteral( "IgnoreGetFeatureInfoUrl" ), QStringLiteral( "1" ) );
  }
  if ( settings.value( settingsKey + QStringLiteral( "/smoothPixmapTransform" ), false ).toBool() )
  {
    uri.setParam( QStringLiteral( "SmoothPixmapTransform" ), QStringLiteral( "1" ) );
  }
  QString dpiMode = settings.value( settingsKey + QStringLiteral( "/dpiMode" ), QStringLiteral( "all" ) ).toString();
  if ( !dpiMode.isEmpty() )
  {
    uri.setParam( QStringLiteral( "dpiMode" ), dpiMode );
  }

  return uri;
}

QgsDataSourceUri &QgsOwsConnection::addWfsConnectionSettings( QgsDataSourceUri &uri, const QString &settingsKey )
{
  addCommonConnectionSettings( uri, settingsKey );

  QgsSettings settings;
  QString version = settings.value( settingsKey + "/version" ).toString();
  if ( !version.isEmpty() )
  {
    uri.setParam( QStringLiteral( "version" ), version );
  }

  QString maxnumfeatures = settings.value( settingsKey + QStringLiteral( "/maxnumfeatures" ) ).toString();
  if ( !maxnumfeatures.isEmpty() )
  {
    uri.setParam( QStringLiteral( "maxNumFeatures" ), maxnumfeatures );
  }

  return uri;
}

QStringList QgsOwsConnection::connectionList( const QString &service )
{
  QgsSettings settings;
  settings.beginGroup( "qgis/connections-" + service.toLower() );
  return settings.childGroups();
}

QString QgsOwsConnection::selectedConnection( const QString &service )
{
  QgsSettings settings;
  return settings.value( "qgis/connections-" + service.toLower() + "/selected" ).toString();
}

void QgsOwsConnection::setSelectedConnection( const QString &service, const QString &name )
{
  QgsSettings settings;
  settings.setValue( "qgis/connections-" + service.toLower() + "/selected", name );
}

void QgsOwsConnection::addCommonConnectionSettings( QgsDataSourceUri &uri, const QString &key )
{
  QgsSettings settings;

  if ( settings.value( key + QStringLiteral( "/ignoreAxisOrientation" ), false ).toBool() )
  {
    uri.setParam( QStringLiteral( "IgnoreAxisOrientation" ), QStringLiteral( "1" ) );
  }
  if ( settings.value( key + QStringLiteral( "/invertAxisOrientation" ), false ).toBool() )
  {
    uri.setParam( QStringLiteral( "InvertAxisOrientation" ), QStringLiteral( "1" ) );
  }
}

void QgsOwsConnection::deleteConnection( const QString &service, const QString &name )
{
  QgsSettings settings;
  settings.remove( "qgis/connections-" + service.toLower() + '/' + name );
  settings.remove( "qgis/" + service + '/' + name );
}
