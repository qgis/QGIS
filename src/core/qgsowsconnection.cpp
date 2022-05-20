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
#include "qgshttpheaders.h"

#include <QPicture>
#include <QUrl>
#include <QNetworkRequest>
#include <QNetworkReply>


QgsOwsConnection::QgsOwsConnection( const QString &service, const QString &connName )
  : mConnName( connName )
  , mService( service )
{
  QgsDebugMsgLevel( "theConnName = " + connName, 4 );

  const QString url = settingsConnectionUrl.value( {mService.toLower(), mConnName} );
  mConnectionInfo = url;
  mUri.setParam( QStringLiteral( "url" ), url );

  // Check for credentials and prepend to the connection info
  const QString username = settingsConnectionUsername.value( {mService, mConnName} );
  const QString password = settingsConnectionPassword.value( {mService, mConnName} );
  if ( !username.isEmpty() )
  {
    // check for a password, if none prompt to get it
    mUri.setUsername( username );
    mUri.setPassword( password );
  }

  const QString authcfg = settingsConnectionAuthCfg.value( {mService, mConnName} );
  if ( !authcfg.isEmpty() )
  {
    mUri.setAuthConfigId( authcfg );
  }
  mConnectionInfo.append( ",authcfg=" + authcfg );

  QgsHttpHeaders httpHeaders( QString( "%3/connections-%1/%2/" ).arg( mService.toLower(), mConnName, QgsSettings::Prefix::QGIS ) );
  mUri.setHttpHeaders( httpHeaders );
  const QString referer = httpHeaders[QgsHttpHeaders::KEY_REFERER].toString();
  if ( !referer.isEmpty() )
  {
    mConnectionInfo.append( ",referer=" + referer );
  }

  if ( mService.compare( QLatin1String( "WMS" ), Qt::CaseInsensitive ) == 0
       || mService.compare( QLatin1String( "WCS" ), Qt::CaseInsensitive ) == 0 )
  {
    addWmsWcsConnectionSettings( mUri, service, connName );
  }
  else if ( mService.compare( QLatin1String( "WFS" ), Qt::CaseInsensitive ) == 0 )
  {
    addWfsConnectionSettings( mUri, service, connName );
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
  Q_NOWARN_DEPRECATED_PUSH
  addCommonConnectionSettings( uri, settingsKey );
  Q_NOWARN_DEPRECATED_POP

  const QgsSettings settings;
  uri.httpHeaders().setFromSettings( settings, settingsKey );

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
  if ( settings.value( settingsKey + QStringLiteral( "/ignoreReportedLayerExtents" ), false ).toBool() )
  {
    uri.setParam( QStringLiteral( "IgnoreReportedLayerExtents" ), QStringLiteral( "1" ) );
  }
  const QString dpiMode = settings.value( settingsKey + QStringLiteral( "/dpiMode" ), QStringLiteral( "all" ) ).toString();
  if ( !dpiMode.isEmpty() )
  {
    uri.setParam( QStringLiteral( "dpiMode" ), dpiMode );
  }

  return uri;
}

QgsDataSourceUri &QgsOwsConnection::addWmsWcsConnectionSettings( QgsDataSourceUri &uri, const QString &service, const QString &connName )
{
  addCommonConnectionSettings( uri, service, connName );

  QString settingsKey = QString( "%3/connections-%1/%2/" ).arg( service.toLower(), connName, QgsSettings::Prefix::QGIS );
  const QgsSettings settings;
  uri.httpHeaders().setFromSettings( settings, settingsKey );

  if ( settingsConnectionIgnoreGetMapURI.value( {service.toLower(), connName} ) )
  {
    uri.setParam( QStringLiteral( "IgnoreGetMapUrl" ), QStringLiteral( "1" ) );
  }
  if ( settingsConnectionIgnoreGetFeatureInfoURI.value( {service.toLower(), connName} ) )
  {
    uri.setParam( QStringLiteral( "IgnoreGetFeatureInfoUrl" ), QStringLiteral( "1" ) );
  }
  if ( settingsConnectionSmoothPixmapTransform.value( {service.toLower(), connName} ) )
  {
    uri.setParam( QStringLiteral( "SmoothPixmapTransform" ), QStringLiteral( "1" ) );
  }
  if ( settingsConnectionReportedLayerExtents.value( {service.toLower(), connName} ) )
  {
    uri.setParam( QStringLiteral( "IgnoreReportedLayerExtents" ), QStringLiteral( "1" ) );
  }
  if ( settingsConnectionDpiMode.exists( {service.toLower(), connName} ) )
  {
    uri.setParam( QStringLiteral( "dpiMode" ), QString::number( static_cast<int>( settingsConnectionDpiMode.value( {service.toLower(), connName} ) ) ) );
  }

  return uri;
}

QgsDataSourceUri &QgsOwsConnection::addWfsConnectionSettings( QgsDataSourceUri &uri, const QString &settingsKey )
{
  Q_NOWARN_DEPRECATED_PUSH
  addCommonConnectionSettings( uri, settingsKey );
  Q_NOWARN_DEPRECATED_POP

  const QgsSettings settings;
  const QString version = settings.value( settingsKey + "/version" ).toString();
  if ( !version.isEmpty() )
  {
    uri.setParam( QStringLiteral( "version" ), version );
  }

  const QString maxnumfeatures = settings.value( settingsKey + QStringLiteral( "/maxnumfeatures" ) ).toString();
  if ( !maxnumfeatures.isEmpty() )
  {
    uri.setParam( QStringLiteral( "maxNumFeatures" ), maxnumfeatures );
  }

  return uri;
}

QgsDataSourceUri &QgsOwsConnection::addWfsConnectionSettings( QgsDataSourceUri &uri, const QString &service, const QString &connName )
{
  addCommonConnectionSettings( uri, service, connName );

  const QString version = settingsConnectionVersion.value( {service.toLower(), connName} );
  if ( !version.isEmpty() )
  {
    uri.setParam( QStringLiteral( "version" ), version );
  }

  const QString maxnumFeatures = settingsConnectionMaxNumFeatures.value( {service.toLower(), connName} );
  if ( !maxnumFeatures.isEmpty() )
  {
    uri.setParam( QStringLiteral( "maxNumFeatures" ), maxnumFeatures );
  }

  return uri;
}

QStringList QgsOwsConnection::connectionList( const QString &service )
{
  QgsSettings settings;
  settings.beginGroup( QString( "%1/connections-%2" ).arg( QgsSettings::Prefix::QGIS, service.toLower() ) );
  return settings.childGroups();
}

QString QgsOwsConnection::selectedConnection( const QString &service )
{
  const QgsSettings settings;
  return settings.value( QString( "%1/connections-%2/selected" ).arg( QgsSettings::Prefix::QGIS, service.toLower() ) ).toString();
}

void QgsOwsConnection::setSelectedConnection( const QString &service, const QString &name )
{
  QgsSettings settings;
  settings.setValue( QString( "%1/connections-%2/selected" ).arg( QgsSettings::Prefix::QGIS, service.toLower() ), name );
}

void QgsOwsConnection::addCommonConnectionSettings( QgsDataSourceUri &uri, const QString &key )
{
  const QgsSettings settings;

  if ( settings.value( key + QStringLiteral( "/ignoreAxisOrientation" ), false ).toBool() )
  {
    uri.setParam( QStringLiteral( "IgnoreAxisOrientation" ), QStringLiteral( "1" ) );
  }
  if ( settings.value( key + QStringLiteral( "/invertAxisOrientation" ), false ).toBool() )
  {
    uri.setParam( QStringLiteral( "InvertAxisOrientation" ), QStringLiteral( "1" ) );
  }
}

void QgsOwsConnection::addCommonConnectionSettings( QgsDataSourceUri &uri, const QString &service, const QString &connectionName )
{

  if ( settingsConnectionIgnoreAxisOrientation.value( {service.toLower(), connectionName} ) )
  {
    uri.setParam( QStringLiteral( "IgnoreAxisOrientation" ), QStringLiteral( "1" ) );
  }
  if ( settingsConnectionInvertAxisOrientation.value( {service.toLower(), connectionName} ) )
  {
    uri.setParam( QStringLiteral( "InvertAxisOrientation" ), QStringLiteral( "1" ) );
  }
}

void QgsOwsConnection::deleteConnection( const QString &service, const QString &name )
{
  settingsServiceConnectionDetailsGroup.removeAllSettingsAtBaseKey( {service.toLower(), name} );
  settingsServiceConnectionCredentialsGroup.removeAllSettingsAtBaseKey( {service, name} );
}
