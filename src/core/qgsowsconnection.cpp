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
#include "qgsowsconnection.h"
#include "moc_qgsowsconnection.cpp"
#include "qgssettings.h"
#include "qgshttpheaders.h"
#include "qgssettingsentryimpl.h"
#include "qgssettingsentryenumflag.h"

#include <QPicture>
#include <QUrl>
#include <QNetworkRequest>
#include <QNetworkReply>


const QgsSettingsEntryString *QgsXyzConnectionSettings::settingsUrl = new QgsSettingsEntryString( QStringLiteral( "url" ), sTreeXyzConnections, QString() ) ;
const QgsSettingsEntryVariantMap *QgsXyzConnectionSettings::settingsHeaders = new QgsSettingsEntryVariantMap( QStringLiteral( "http-header" ), sTreeXyzConnections ) ;
const QgsSettingsEntryInteger *QgsXyzConnectionSettings::settingsZmin = new QgsSettingsEntryInteger( QStringLiteral( "zmin" ), sTreeXyzConnections, -1 );
const QgsSettingsEntryInteger *QgsXyzConnectionSettings::settingsZmax = new QgsSettingsEntryInteger( QStringLiteral( "zmax" ), sTreeXyzConnections, -1 );
const QgsSettingsEntryDouble *QgsXyzConnectionSettings::settingsTilePixelRatio = new QgsSettingsEntryDouble( QStringLiteral( "tile-pixel-ratio" ), sTreeXyzConnections, 0, QStringLiteral( "0 = unknown (not scaled), 1.0 = 256x256, 2.0 = 512x512" ) ) ;
const QgsSettingsEntryBool *QgsXyzConnectionSettings::settingsHidden = new QgsSettingsEntryBool( QStringLiteral( "hidden" ), sTreeXyzConnections, false ) ;
const QgsSettingsEntryString *QgsXyzConnectionSettings::settingsInterpretation = new QgsSettingsEntryString( QStringLiteral( "interpretation" ), sTreeXyzConnections, QString() ) ;
const QgsSettingsEntryString *QgsXyzConnectionSettings::settingsUsername = new QgsSettingsEntryString( QStringLiteral( "username" ), sTreeXyzConnections ) ;
const QgsSettingsEntryString *QgsXyzConnectionSettings::settingsPassword = new QgsSettingsEntryString( QStringLiteral( "password" ), sTreeXyzConnections ) ;
const QgsSettingsEntryString *QgsXyzConnectionSettings::settingsAuthcfg = new QgsSettingsEntryString( QStringLiteral( "authcfg" ), sTreeXyzConnections ) ;


const QgsSettingsEntryString *QgsArcGisConnectionSettings::settingsUrl = new QgsSettingsEntryString( QStringLiteral( "url" ), sTreeConnectionArcgis );
const QgsSettingsEntryString *QgsArcGisConnectionSettings::settingsAuthcfg = new QgsSettingsEntryString( QStringLiteral( "authcfg" ), sTreeConnectionArcgis );
const QgsSettingsEntryString *QgsArcGisConnectionSettings::settingsUsername = new QgsSettingsEntryString( QStringLiteral( "username" ), sTreeConnectionArcgis );
const QgsSettingsEntryString *QgsArcGisConnectionSettings::settingsPassword = new QgsSettingsEntryString( QStringLiteral( "password" ), sTreeConnectionArcgis );
const QgsSettingsEntryVariantMap *QgsArcGisConnectionSettings::settingsHeaders = new QgsSettingsEntryVariantMap( QStringLiteral( "http-header" ), sTreeConnectionArcgis );
const QgsSettingsEntryString *QgsArcGisConnectionSettings::settingsUrlPrefix = new QgsSettingsEntryString( QStringLiteral( "urlprefix" ), sTreeConnectionArcgis );
const QgsSettingsEntryString *QgsArcGisConnectionSettings::settingsContentEndpoint = new QgsSettingsEntryString( QStringLiteral( "content-endpoint" ), sTreeConnectionArcgis );
const QgsSettingsEntryString *QgsArcGisConnectionSettings::settingsCommunityEndpoint = new QgsSettingsEntryString( QStringLiteral( "community-endpoint" ), sTreeConnectionArcgis );


const QgsSettingsEntryString *QgsOwsConnection::settingsUrl = new QgsSettingsEntryString( QStringLiteral( "url" ), sTreeOwsConnections, QString() ) ;
const QgsSettingsEntryVariantMap *QgsOwsConnection::settingsHeaders = new QgsSettingsEntryVariantMap( QStringLiteral( "http-header" ), sTreeOwsConnections ) ;
const QgsSettingsEntryString *QgsOwsConnection::settingsVersion = new QgsSettingsEntryString( QStringLiteral( "version" ), sTreeOwsConnections, QString() ) ;
const QgsSettingsEntryBool *QgsOwsConnection::settingsIgnoreGetMapURI = new QgsSettingsEntryBool( QStringLiteral( "ignore-get-map-uri" ), sTreeOwsConnections, false ) ;
const QgsSettingsEntryBool *QgsOwsConnection::settingsIgnoreGetFeatureInfoURI = new QgsSettingsEntryBool( QStringLiteral( "ignore-get-feature-info-uri" ), sTreeOwsConnections, false ) ;
const QgsSettingsEntryBool *QgsOwsConnection::settingsSmoothPixmapTransform = new QgsSettingsEntryBool( QStringLiteral( "smooth-pixmap-transform" ), sTreeOwsConnections, false ) ;
const QgsSettingsEntryBool *QgsOwsConnection::settingsReportedLayerExtents = new QgsSettingsEntryBool( QStringLiteral( "reported-layer-extents" ), sTreeOwsConnections, false ) ;
const QgsSettingsEntryEnumFlag<Qgis::DpiMode> *QgsOwsConnection::settingsDpiMode = new QgsSettingsEntryEnumFlag<Qgis::DpiMode>( QStringLiteral( "dpi-mode" ), sTreeOwsConnections, Qgis::DpiMode::All, QString(), Qgis::SettingsOption::SaveEnumFlagAsInt ) ;
const QgsSettingsEntryEnumFlag<Qgis::TilePixelRatio> *QgsOwsConnection::settingsTilePixelRatio = new QgsSettingsEntryEnumFlag<Qgis::TilePixelRatio>( QStringLiteral( "tile-pixel-ratio" ), sTreeOwsConnections, Qgis::TilePixelRatio::Undefined, QString(), Qgis::SettingsOption::SaveEnumFlagAsInt ) ;
const QgsSettingsEntryString *QgsOwsConnection::settingsMaxNumFeatures = new QgsSettingsEntryString( QStringLiteral( "max-num-features" ), sTreeOwsConnections ) ;
const QgsSettingsEntryString *QgsOwsConnection::settingsPagesize = new QgsSettingsEntryString( QStringLiteral( "page-size" ), sTreeOwsConnections ) ;
const QgsSettingsEntryString *QgsOwsConnection::settingsPagingEnabled = new QgsSettingsEntryString( QStringLiteral( "paging-enabled" ), sTreeOwsConnections, QString( "default" ) ) ;
const QgsSettingsEntryBool *QgsOwsConnection::settingsPreferCoordinatesForWfsT11 = new QgsSettingsEntryBool( QStringLiteral( "prefer-coordinates-for-wfs-T11" ), sTreeOwsConnections, false ) ;
const QgsSettingsEntryBool *QgsOwsConnection::settingsIgnoreAxisOrientation = new QgsSettingsEntryBool( QStringLiteral( "ignore-axis-orientation" ), sTreeOwsConnections, false ) ;
const QgsSettingsEntryBool *QgsOwsConnection::settingsInvertAxisOrientation = new QgsSettingsEntryBool( QStringLiteral( "invert-axis-orientation" ), sTreeOwsConnections, false ) ;
const QgsSettingsEntryString *QgsOwsConnection::settingsUsername = new QgsSettingsEntryString( QStringLiteral( "username" ), sTreeOwsConnections ) ;
const QgsSettingsEntryString *QgsOwsConnection::settingsPassword = new QgsSettingsEntryString( QStringLiteral( "password" ), sTreeOwsConnections ) ;
const QgsSettingsEntryString *QgsOwsConnection::settingsAuthCfg = new QgsSettingsEntryString( QStringLiteral( "authcfg" ), sTreeOwsConnections ) ;
const QgsSettingsEntryInteger *QgsOwsConnection::settingsFeatureCount = new QgsSettingsEntryInteger( QStringLiteral( "feature-count" ), sTreeOwsConnections, 10 );

QgsOwsConnection::QgsOwsConnection( const QString &service, const QString &connName )
  : mConnName( connName )
  , mService( service )
{
  QgsDebugMsgLevel( "theConnName = " + connName, 4 );

  const QString url = settingsUrl->value( {mService.toLower(), mConnName} );
  mConnectionInfo = url;
  mUri.setParam( QStringLiteral( "url" ), url );

  // Check for credentials and prepend to the connection info
  const QString username = settingsUsername->value( {mService.toLower(), mConnName} );
  const QString password = settingsPassword->value( {mService.toLower(), mConnName} );
  if ( !username.isEmpty() )
  {
    // check for a password, if none prompt to get it
    mUri.setUsername( username );
    mUri.setPassword( password );
  }

  const QString authcfg = settingsAuthCfg->value( {mService.toLower(), mConnName} );
  if ( !authcfg.isEmpty() )
  {
    mUri.setAuthConfigId( authcfg );
  }
  mConnectionInfo.append( ",authcfg=" + authcfg );

  QgsHttpHeaders httpHeaders( settingsHeaders->value( {mService.toLower(), mConnName} ) );
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
  const QString tilePixelRatio = settings.value( settingsKey + QStringLiteral( "/tilePixelRatio" ), "0" ).toString();
  if ( tilePixelRatio != QLatin1String( "0" ) )
  {
    uri.setParam( QStringLiteral( "tilePixelRatio" ), tilePixelRatio );
  }

  return uri;
}

QgsDataSourceUri &QgsOwsConnection::addWmsWcsConnectionSettings( QgsDataSourceUri &uri, const QString &service, const QString &connName )
{
  addCommonConnectionSettings( uri, service, connName );

  uri.setHttpHeaders( QgsHttpHeaders( settingsHeaders->value( {service.toLower(), connName} ) ) );

  if ( settingsIgnoreGetMapURI->value( {service.toLower(), connName} ) )
  {
    uri.setParam( QStringLiteral( "IgnoreGetMapUrl" ), QStringLiteral( "1" ) );
  }
  if ( settingsIgnoreGetFeatureInfoURI->value( {service.toLower(), connName} ) )
  {
    uri.setParam( QStringLiteral( "IgnoreGetFeatureInfoUrl" ), QStringLiteral( "1" ) );
  }
  if ( settingsSmoothPixmapTransform->value( {service.toLower(), connName} ) )
  {
    uri.setParam( QStringLiteral( "SmoothPixmapTransform" ), QStringLiteral( "1" ) );
  }
  if ( settingsReportedLayerExtents->value( {service.toLower(), connName} ) )
  {
    uri.setParam( QStringLiteral( "IgnoreReportedLayerExtents" ), QStringLiteral( "1" ) );
  }
  if ( settingsDpiMode->exists( {service.toLower(), connName} ) )
  {
    uri.setParam( QStringLiteral( "dpiMode" ), QString::number( static_cast<int>( settingsDpiMode->value( {service.toLower(), connName} ) ) ) );
  }
  if ( settingsTilePixelRatio->exists( {service.toLower(), connName} ) )
  {
    uri.setParam( QStringLiteral( "tilePixelRatio" ), QString::number( static_cast<int>( settingsTilePixelRatio->value( {service.toLower(), connName} ) ) ) );
  }
  if ( settingsFeatureCount->exists( {service.toLower(), connName} ) )
  {
    uri.setParam( QStringLiteral( "featureCount" ), QString::number( settingsFeatureCount->value( {service.toLower(), connName} ) ) );
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

  const QString version = settingsVersion->value( {service.toLower(), connName} );
  if ( !version.isEmpty() )
  {
    uri.setParam( QStringLiteral( "version" ), version );
  }

  const QString maxnumFeatures = settingsMaxNumFeatures->value( {service.toLower(), connName} );
  if ( !maxnumFeatures.isEmpty() )
  {
    uri.setParam( QStringLiteral( "maxNumFeatures" ), maxnumFeatures );
  }

  return uri;
}

QStringList QgsOwsConnection::connectionList( const QString &service )
{
  return QgsOwsConnection::sTreeOwsConnections->items( {service.toLower()} );
}

QString QgsOwsConnection::selectedConnection( const QString &service )
{
  return QgsOwsConnection::sTreeOwsConnections->selectedItem( {service.toLower()} );
}

void QgsOwsConnection::setSelectedConnection( const QString &service, const QString &name )
{
  QgsOwsConnection::sTreeOwsConnections->setSelectedItem( name, {service.toLower()} );
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

  if ( settingsIgnoreAxisOrientation->value( {service.toLower(), connectionName} ) )
  {
    uri.setParam( QStringLiteral( "IgnoreAxisOrientation" ), QStringLiteral( "1" ) );
  }
  if ( settingsInvertAxisOrientation->value( {service.toLower(), connectionName} ) )
  {
    uri.setParam( QStringLiteral( "InvertAxisOrientation" ), QStringLiteral( "1" ) );
  }
}

void QgsOwsConnection::deleteConnection( const QString &service, const QString &name )
{
  sTreeOwsConnections->deleteItem( name, {service.toLower()} );
}
