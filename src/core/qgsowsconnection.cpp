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

#include "qgsowsconnection.h"

#include "qgis.h"
#include "qgsdatasourceuri.h"
#include "qgshttpheaders.h"
#include "qgslogger.h"
#include "qgssettings.h"
#include "qgssettingsentryenumflag.h"
#include "qgssettingsentryimpl.h"

#include <QNetworkReply>
#include <QNetworkRequest>
#include <QPicture>
#include <QUrl>

#include "moc_qgsowsconnection.cpp"

const QgsSettingsEntryString *QgsXyzConnectionSettings::settingsUrl = new QgsSettingsEntryString( u"url"_s, sTreeXyzConnections, QString() ) ;
const QgsSettingsEntryVariantMap *QgsXyzConnectionSettings::settingsHeaders = new QgsSettingsEntryVariantMap( u"http-header"_s, sTreeXyzConnections ) ;
const QgsSettingsEntryInteger *QgsXyzConnectionSettings::settingsZmin = new QgsSettingsEntryInteger( u"zmin"_s, sTreeXyzConnections, -1 );
const QgsSettingsEntryInteger *QgsXyzConnectionSettings::settingsZmax = new QgsSettingsEntryInteger( u"zmax"_s, sTreeXyzConnections, -1 );
const QgsSettingsEntryDouble *QgsXyzConnectionSettings::settingsTilePixelRatio = new QgsSettingsEntryDouble( u"tile-pixel-ratio"_s, sTreeXyzConnections, 0, u"0 = unknown (not scaled), 1.0 = 256x256, 2.0 = 512x512"_s ) ;
const QgsSettingsEntryBool *QgsXyzConnectionSettings::settingsHidden = new QgsSettingsEntryBool( u"hidden"_s, sTreeXyzConnections, false ) ;
const QgsSettingsEntryString *QgsXyzConnectionSettings::settingsInterpretation = new QgsSettingsEntryString( u"interpretation"_s, sTreeXyzConnections, QString() ) ;
const QgsSettingsEntryString *QgsXyzConnectionSettings::settingsUsername = new QgsSettingsEntryString( u"username"_s, sTreeXyzConnections ) ;
const QgsSettingsEntryString *QgsXyzConnectionSettings::settingsPassword = new QgsSettingsEntryString( u"password"_s, sTreeXyzConnections ) ;
const QgsSettingsEntryString *QgsXyzConnectionSettings::settingsAuthcfg = new QgsSettingsEntryString( u"authcfg"_s, sTreeXyzConnections ) ;


const QgsSettingsEntryString *QgsArcGisConnectionSettings::settingsUrl = new QgsSettingsEntryString( u"url"_s, sTreeConnectionArcgis );
const QgsSettingsEntryString *QgsArcGisConnectionSettings::settingsAuthcfg = new QgsSettingsEntryString( u"authcfg"_s, sTreeConnectionArcgis );
const QgsSettingsEntryString *QgsArcGisConnectionSettings::settingsUsername = new QgsSettingsEntryString( u"username"_s, sTreeConnectionArcgis );
const QgsSettingsEntryString *QgsArcGisConnectionSettings::settingsPassword = new QgsSettingsEntryString( u"password"_s, sTreeConnectionArcgis );
const QgsSettingsEntryVariantMap *QgsArcGisConnectionSettings::settingsHeaders = new QgsSettingsEntryVariantMap( u"http-header"_s, sTreeConnectionArcgis );
const QgsSettingsEntryString *QgsArcGisConnectionSettings::settingsUrlPrefix = new QgsSettingsEntryString( u"urlprefix"_s, sTreeConnectionArcgis );
const QgsSettingsEntryString *QgsArcGisConnectionSettings::settingsContentEndpoint = new QgsSettingsEntryString( u"content-endpoint"_s, sTreeConnectionArcgis );
const QgsSettingsEntryString *QgsArcGisConnectionSettings::settingsCommunityEndpoint = new QgsSettingsEntryString( u"community-endpoint"_s, sTreeConnectionArcgis );


const QgsSettingsEntryString *QgsOwsConnection::settingsUrl = new QgsSettingsEntryString( u"url"_s, sTreeOwsConnections, QString() ) ;
const QgsSettingsEntryVariantMap *QgsOwsConnection::settingsHeaders = new QgsSettingsEntryVariantMap( u"http-header"_s, sTreeOwsConnections ) ;
const QgsSettingsEntryString *QgsOwsConnection::settingsVersion = new QgsSettingsEntryString( u"version"_s, sTreeOwsConnections, QString() ) ;
const QgsSettingsEntryBool *QgsOwsConnection::settingsIgnoreGetMapURI = new QgsSettingsEntryBool( u"ignore-get-map-uri"_s, sTreeOwsConnections, false ) ;
const QgsSettingsEntryBool *QgsOwsConnection::settingsIgnoreGetFeatureInfoURI = new QgsSettingsEntryBool( u"ignore-get-feature-info-uri"_s, sTreeOwsConnections, false ) ;
const QgsSettingsEntryBool *QgsOwsConnection::settingsSmoothPixmapTransform = new QgsSettingsEntryBool( u"smooth-pixmap-transform"_s, sTreeOwsConnections, false ) ;
const QgsSettingsEntryBool *QgsOwsConnection::settingsReportedLayerExtents = new QgsSettingsEntryBool( u"reported-layer-extents"_s, sTreeOwsConnections, false ) ;
const QgsSettingsEntryEnumFlag<Qgis::DpiMode> *QgsOwsConnection::settingsDpiMode = new QgsSettingsEntryEnumFlag<Qgis::DpiMode>( u"dpi-mode"_s, sTreeOwsConnections, Qgis::DpiMode::All, QString(), Qgis::SettingsOption::SaveEnumFlagAsInt ) ;
const QgsSettingsEntryEnumFlag<Qgis::TilePixelRatio> *QgsOwsConnection::settingsTilePixelRatio = new QgsSettingsEntryEnumFlag<Qgis::TilePixelRatio>( u"tile-pixel-ratio"_s, sTreeOwsConnections, Qgis::TilePixelRatio::Undefined, QString(), Qgis::SettingsOption::SaveEnumFlagAsInt ) ;
const QgsSettingsEntryString *QgsOwsConnection::settingsMaxNumFeatures = new QgsSettingsEntryString( u"max-num-features"_s, sTreeOwsConnections ) ;
const QgsSettingsEntryString *QgsOwsConnection::settingsPagesize = new QgsSettingsEntryString( u"page-size"_s, sTreeOwsConnections ) ;
const QgsSettingsEntryString *QgsOwsConnection::settingsPagingEnabled = new QgsSettingsEntryString( u"paging-enabled"_s, sTreeOwsConnections, QString( "default" ) ) ;
const QgsSettingsEntryString *QgsOwsConnection::settingsDefaultFeatureFormat = new QgsSettingsEntryString( u"default-feature-format"_s, sTreeOwsConnections, QString( ) ) ;
const QgsSettingsEntryStringList *QgsOwsConnection::settingsAvailableFeatureFormats = new QgsSettingsEntryStringList( u"available-feature-formats"_s, sTreeOwsConnections, {} ) ;
const QgsSettingsEntryString *QgsOwsConnection::settingsWfsFeatureMode = new QgsSettingsEntryString( u"feature-mode"_s, sTreeOwsConnections, QString( "default" ) ) ;
const QgsSettingsEntryBool *QgsOwsConnection::settingsPreferCoordinatesForWfsT11 = new QgsSettingsEntryBool( u"prefer-coordinates-for-wfs-T11"_s, sTreeOwsConnections, false ) ;
const QgsSettingsEntryBool *QgsOwsConnection::settingsWfsForceInitialGetFeature = new QgsSettingsEntryBool( u"force-initial-get-feature"_s, sTreeOwsConnections, false ) ;
const QgsSettingsEntryString *QgsOwsConnection::settingsDefaultImageFormat = new QgsSettingsEntryString( u"default-image-format"_s, sTreeOwsConnections, QString( ) ) ;
const QgsSettingsEntryStringList *QgsOwsConnection::settingsAvailableImageFormats = new QgsSettingsEntryStringList( u"available-image-formats"_s, sTreeOwsConnections, {} ) ;
const QgsSettingsEntryBool *QgsOwsConnection::settingsIgnoreAxisOrientation = new QgsSettingsEntryBool( u"ignore-axis-orientation"_s, sTreeOwsConnections, false ) ;
const QgsSettingsEntryBool *QgsOwsConnection::settingsInvertAxisOrientation = new QgsSettingsEntryBool( u"invert-axis-orientation"_s, sTreeOwsConnections, false ) ;
const QgsSettingsEntryString *QgsOwsConnection::settingsUsername = new QgsSettingsEntryString( u"username"_s, sTreeOwsConnections ) ;
const QgsSettingsEntryString *QgsOwsConnection::settingsPassword = new QgsSettingsEntryString( u"password"_s, sTreeOwsConnections ) ;
const QgsSettingsEntryString *QgsOwsConnection::settingsAuthCfg = new QgsSettingsEntryString( u"authcfg"_s, sTreeOwsConnections ) ;
const QgsSettingsEntryInteger *QgsOwsConnection::settingsFeatureCount = new QgsSettingsEntryInteger( u"feature-count"_s, sTreeOwsConnections, 10 );
const QgsSettingsEntryEnumFlag<Qgis::HttpMethod> *QgsOwsConnection::settingsPreferredHttpMethod = new QgsSettingsEntryEnumFlag<Qgis::HttpMethod>( u"http-method"_s, sTreeOwsConnections, Qgis::HttpMethod::Get, QString() );

QgsOwsConnection::QgsOwsConnection( const QString &service, const QString &connName )
  : mConnName( connName )
  , mService( service )
{
  QgsDebugMsgLevel( "theConnName = " + connName, 4 );

  const QString url = settingsUrl->value( {mService.toLower(), mConnName} );
  mConnectionInfo = url;
  mUri.setParam( u"url"_s, url );

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

  if ( mService.compare( "WMS"_L1, Qt::CaseInsensitive ) == 0
       || mService.compare( "WCS"_L1, Qt::CaseInsensitive ) == 0 )
  {
    addWmsWcsConnectionSettings( mUri, service, connName );
  }
  else if ( mService.compare( "WFS"_L1, Qt::CaseInsensitive ) == 0 )
  {
    addWfsConnectionSettings( mUri, service, connName );
  }

  QgsDebugMsgLevel( u"encoded uri: '%1'."_s.arg( QString( mUri.encodedUri() ) ), 4 );
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

  if ( settings.value( settingsKey + u"/ignoreGetMapURI"_s, false ).toBool() )
  {
    uri.setParam( u"IgnoreGetMapUrl"_s, u"1"_s );
  }
  if ( settings.value( settingsKey + u"/ignoreGetFeatureInfoURI"_s, false ).toBool() )
  {
    uri.setParam( u"IgnoreGetFeatureInfoUrl"_s, u"1"_s );
  }
  if ( settings.value( settingsKey + u"/smoothPixmapTransform"_s, false ).toBool() )
  {
    uri.setParam( u"SmoothPixmapTransform"_s, u"1"_s );
  }
  if ( settings.value( settingsKey + u"/ignoreReportedLayerExtents"_s, false ).toBool() )
  {
    uri.setParam( u"IgnoreReportedLayerExtents"_s, u"1"_s );
  }
  const QString dpiMode = settings.value( settingsKey + u"/dpiMode"_s, u"all"_s ).toString();
  if ( !dpiMode.isEmpty() )
  {
    uri.setParam( u"dpiMode"_s, dpiMode );
  }
  const QString tilePixelRatio = settings.value( settingsKey + u"/tilePixelRatio"_s, "0" ).toString();
  if ( tilePixelRatio != "0"_L1 )
  {
    uri.setParam( u"tilePixelRatio"_s, tilePixelRatio );
  }

  return uri;
}

QgsDataSourceUri &QgsOwsConnection::addWmsWcsConnectionSettings( QgsDataSourceUri &uri, const QString &service, const QString &connName )
{
  addCommonConnectionSettings( uri, service, connName );

  uri.setHttpHeaders( QgsHttpHeaders( settingsHeaders->value( {service.toLower(), connName} ) ) );

  if ( settingsIgnoreGetMapURI->value( {service.toLower(), connName} ) )
  {
    uri.setParam( u"IgnoreGetMapUrl"_s, u"1"_s );
  }
  if ( settingsIgnoreGetFeatureInfoURI->value( {service.toLower(), connName} ) )
  {
    uri.setParam( u"IgnoreGetFeatureInfoUrl"_s, u"1"_s );
  }
  if ( settingsSmoothPixmapTransform->value( {service.toLower(), connName} ) )
  {
    uri.setParam( u"SmoothPixmapTransform"_s, u"1"_s );
  }
  if ( settingsReportedLayerExtents->value( {service.toLower(), connName} ) )
  {
    uri.setParam( u"IgnoreReportedLayerExtents"_s, u"1"_s );
  }
  if ( settingsDpiMode->exists( {service.toLower(), connName} ) )
  {
    uri.setParam( u"dpiMode"_s, QString::number( static_cast<int>( settingsDpiMode->value( {service.toLower(), connName} ) ) ) );
  }
  if ( settingsTilePixelRatio->exists( {service.toLower(), connName} ) )
  {
    uri.setParam( u"tilePixelRatio"_s, QString::number( static_cast<int>( settingsTilePixelRatio->value( {service.toLower(), connName} ) ) ) );
  }
  if ( settingsFeatureCount->exists( {service.toLower(), connName} ) )
  {
    uri.setParam( u"featureCount"_s, QString::number( settingsFeatureCount->value( {service.toLower(), connName} ) ) );
  }
  // Note: settingsDefaultImageFormat is not part of the connection URI because it's an individual layer setting
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
    uri.setParam( u"version"_s, version );
  }

  const QString maxnumfeatures = settings.value( settingsKey + u"/maxnumfeatures"_s ).toString();
  if ( !maxnumfeatures.isEmpty() )
  {
    uri.setParam( u"maxNumFeatures"_s, maxnumfeatures );
  }

  return uri;
}

QgsDataSourceUri &QgsOwsConnection::addWfsConnectionSettings( QgsDataSourceUri &uri, const QString &service, const QString &connName )
{
  addCommonConnectionSettings( uri, service, connName );

  const QString version = settingsVersion->value( {service.toLower(), connName} );
  if ( !version.isEmpty() )
  {
    uri.setParam( u"version"_s, version );
  }

  const QString maxnumFeatures = settingsMaxNumFeatures->value( {service.toLower(), connName} );
  if ( !maxnumFeatures.isEmpty() )
  {
    uri.setParam( u"maxNumFeatures"_s, maxnumFeatures );
  }

  const Qgis::HttpMethod httpMethod = settingsPreferredHttpMethod->value( {service.toLower(), connName} );
  switch ( httpMethod )
  {
    case Qgis::HttpMethod::Get:
      // default, we don't set to explicitly set
      break;

    case Qgis::HttpMethod::Post:
      uri.setParam( u"httpMethod"_s, u"post"_s );
      break;

    case Qgis::HttpMethod::Head:
    case Qgis::HttpMethod::Put:
    case Qgis::HttpMethod::Delete:
      // not supported
      break;
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

  if ( settings.value( key + u"/ignoreAxisOrientation"_s, false ).toBool() )
  {
    uri.setParam( u"IgnoreAxisOrientation"_s, u"1"_s );
  }
  if ( settings.value( key + u"/invertAxisOrientation"_s, false ).toBool() )
  {
    uri.setParam( u"InvertAxisOrientation"_s, u"1"_s );
  }
}

void QgsOwsConnection::addCommonConnectionSettings( QgsDataSourceUri &uri, const QString &service, const QString &connectionName )
{

  if ( settingsIgnoreAxisOrientation->value( {service.toLower(), connectionName} ) )
  {
    uri.setParam( u"IgnoreAxisOrientation"_s, u"1"_s );
  }
  if ( settingsInvertAxisOrientation->value( {service.toLower(), connectionName} ) )
  {
    uri.setParam( u"InvertAxisOrientation"_s, u"1"_s );
  }
}

void QgsOwsConnection::deleteConnection( const QString &service, const QString &name )
{
  sTreeOwsConnections->deleteItem( name, {service.toLower()} );
}
