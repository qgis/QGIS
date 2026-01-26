/***************************************************************************
    qgsauthplanetarycomputermethod.cpp
    ------------------------
    begin                : August 2025
    copyright            : (C) 2025 by Stefanos Natsis
    author               : Stefanos Natsis
    email                : uclaros at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsauthplanetarycomputermethod.h"

#include <nlohmann/json.hpp>

#include "qgsapplication.h"
#include "qgsauthmanager.h"
#include "qgsauthmethodregistry.h"
#include "qgsblockingnetworkrequest.h"
#include "qgslogger.h"
#include "qgsnetworkaccessmanager.h"

#include "moc_qgsauthplanetarycomputermethod.cpp"

#ifdef HAVE_GUI
#include "qgsauthplanetarycomputeredit.h"
#endif

#include <QNetworkProxy>
#include <QMutexLocker>
#include <QUuid>
#include <QUrlQuery>


const QString QgsAuthPlanetaryComputerMethod::AUTH_METHOD_KEY = u"PlanetaryComputer"_s;
const QString QgsAuthPlanetaryComputerMethod::AUTH_METHOD_DESCRIPTION = u"Microsoft Planetary Computer"_s;
const QString QgsAuthPlanetaryComputerMethod::AUTH_METHOD_DISPLAY_DESCRIPTION = tr( "Microsoft Planetary Computer" );

const QString QgsAuthPlanetaryComputerMethod::OPEN_SAS_SIGN_URL = u"https://planetarycomputer.microsoft.com/api/sas/v1/sign?href="_s;
const QString QgsAuthPlanetaryComputerMethod::PRO_SAS_SIGN_URL = u"%1://%2/sas/sign?api-version=2025-04-30-preview&href="_s;
const QString QgsAuthPlanetaryComputerMethod::BLOB_STORAGE_DOMAIN = u".blob.core.windows.net"_s;

QMap<QString, QgsAuthMethodConfig> QgsAuthPlanetaryComputerMethod::sAuthConfigCache = QMap<QString, QgsAuthMethodConfig>();
QMap<QString, QgsAuthPlanetaryComputerMethod::SasToken> QgsAuthPlanetaryComputerMethod::sSasTokensCache = QMap<QString, QgsAuthPlanetaryComputerMethod::SasToken>();


QgsAuthPlanetaryComputerMethod::QgsAuthPlanetaryComputerMethod()
{
  setExpansions( QgsAuthMethod::NetworkRequest | QgsAuthMethod::DataSourceUri );
  setDataProviders( { u"gdal"_s, u"copc"_s, u"stac"_s } );

  mOauth2 = QgsApplication::authManager()->authMethod( u"OAuth2"_s );
}

QString QgsAuthPlanetaryComputerMethod::key() const
{
  return AUTH_METHOD_KEY;
}

QString QgsAuthPlanetaryComputerMethod::description() const
{
  return AUTH_METHOD_DESCRIPTION;
}

QString QgsAuthPlanetaryComputerMethod::displayDescription() const
{
  return AUTH_METHOD_DISPLAY_DESCRIPTION;
}

bool QgsAuthPlanetaryComputerMethod::updateNetworkRequest( QNetworkRequest &request, const QString &authcfg, const QString &dataprovider )
{
  Q_UNUSED( dataprovider )

  const QgsAuthMethodConfig config = getMethodConfig( authcfg );
  if ( !config.isValid() )
  {
    QgsDebugError( u"Update request config FAILED for authcfg: %1: config invalid"_s.arg( authcfg ) );
    return false;
  }

  const bool planetaryComputerPro = config.config( u"serverType"_s ) == "pro"_L1;
  const bool needsSasSigning = request.url().host().endsWith( BLOB_STORAGE_DOMAIN );

  // Planetary Computer Pro requests need to be updated using oauth2
  // Requests to blob storage should update the url to include the sas token, but not have the oauth2 token header
  if ( needsSasSigning )
  {
    QString uri = request.url().toString();
    updateUri( uri, config, authcfg );
    const QUrl url( uri );
    request.setUrl( url );
  }
  else if ( planetaryComputerPro )
  {
    mOauth2->updateNetworkRequest( request, authcfg );
  }

  return true;
}

bool QgsAuthPlanetaryComputerMethod::updateDataSourceUriItems( QStringList &connectionItems, const QString &authcfg, const QString &dataprovider )
{
  Q_UNUSED( dataprovider )

  const QgsAuthMethodConfig config = getMethodConfig( authcfg );
  if ( !config.isValid() )
  {
    QgsDebugError( u"Update URI items FAILED for authcfg: %1: config invalid"_s.arg( authcfg ) );
    return false;
  }

  updateUri( connectionItems.first(), config, authcfg );

  return true;
}

void QgsAuthPlanetaryComputerMethod::clearCachedConfig( const QString &authcfg )
{
  removeMethodConfig( authcfg );
}

void QgsAuthPlanetaryComputerMethod::updateMethodConfig( QgsAuthMethodConfig &config )
{
  Q_UNUSED( config );
  // NOTE: add updates as method version() increases due to config storage changes
}

QgsAuthMethodConfig QgsAuthPlanetaryComputerMethod::getMethodConfig( const QString &authcfg, bool fullconfig )
{
  const QMutexLocker locker( &mMutex );
  QgsAuthMethodConfig config;

  // check if it is cached
  if ( sAuthConfigCache.contains( authcfg ) )
  {
    config = sAuthConfigCache.value( authcfg );
    QgsDebugMsgLevel( u"Retrieved config for authcfg: %1"_s.arg( authcfg ), 2 );
    return config;
  }

  // else build basic bundle
  if ( !QgsApplication::authManager()->loadAuthenticationConfig( authcfg, config, fullconfig ) )
  {
    QgsDebugError( u"Retrieve config FAILED for authcfg: %1"_s.arg( authcfg ) );
    return QgsAuthMethodConfig();
  }

  // cache bundle
  putMethodConfig( authcfg, config );

  return config;
}

void QgsAuthPlanetaryComputerMethod::putMethodConfig( const QString &authcfg, const QgsAuthMethodConfig &config )
{
  const QMutexLocker locker( &mMutex );
  QgsDebugMsgLevel( u"Putting Planetary Computer config for authcfg: %1"_s.arg( authcfg ), 2 );
  sAuthConfigCache.insert( authcfg, config );
}

void QgsAuthPlanetaryComputerMethod::removeMethodConfig( const QString &authcfg )
{
  const QMutexLocker locker( &mMutex );
  if ( sAuthConfigCache.contains( authcfg ) )
  {
    sAuthConfigCache.remove( authcfg );
    QMutableMapIterator< QString, SasToken > it( sSasTokensCache );
    while ( it.hasNext() )
    {
      if ( it.key().startsWith( authcfg ) )
        it.remove();
    }
    QgsDebugMsgLevel( u"Removed Planetary Computer config for authcfg: %1"_s.arg( authcfg ), 2 );
  }
}

void QgsAuthPlanetaryComputerMethod::updateUri( QString &uri, const QgsAuthMethodConfig &config, const QString &authcfg )
{
  const bool isPro = config.config( u"serverType"_s ) == "pro"_L1;
  QString signUrl;
  if ( isPro )
  {
    const QUrl rootUrl( config.config( u"rootUrl"_s ) );
    signUrl = PRO_SAS_SIGN_URL.arg( rootUrl.scheme(), rootUrl.host() );
  }
  else
  {
    signUrl = OPEN_SAS_SIGN_URL;
  }

  // We trim the vsicurl prefix from the uri before creating the url, we'll add it back after fetching the token if needed
  const bool isVsi = uri.startsWith( "/vsicurl/"_L1 );
  if ( isVsi )
  {
    uri.remove( 0, 9 );
  }

  QUrl url( uri );
  const QString token( sasTokenForUrl( url, signUrl, authcfg, isPro ) );

  if ( !token.isEmpty() )
  {
    const QString query( url.query() );
    if ( query.isEmpty() )
      url.setQuery( token );
    else
      url.setQuery( u"%1&%2"_s.arg( query, token ) );

    if ( isVsi )
      uri = u"/vsicurl/%1"_s.arg( url.toString() );
    else
      uri = url.toString();
  }
}

QString QgsAuthPlanetaryComputerMethod::sasTokenForUrl( const QUrl &url, const QString &signUrl, const QString &authcfg, bool isPro )
{
  QString token;

  if ( !url.host().endsWith( BLOB_STORAGE_DOMAIN ) )
    return token;

  const QString account = url.host().remove( BLOB_STORAGE_DOMAIN );
  const QStringList path = url.path().split( '/', Qt::SkipEmptyParts );
  if ( path.isEmpty() )
    return token;

  const QString container = path.constFirst();

  SasToken sas = retrieveSasToken( authcfg, account, container );

  if ( sas.isValid() )
    return sas.token;

  const QString requestUrl = signUrl + url.toString();
  QNetworkRequest request = QNetworkRequest( requestUrl );
  // for planetary computer pro we need to apply the oauth2 token first
  if ( isPro )
  {
    mOauth2->updateNetworkRequest( request, authcfg );
  }
  QgsNetworkReplyContent content = QgsNetworkAccessManager::instance()->blockingGet( request );

  if ( content.error() != QNetworkReply::NoError )
  {
    QgsDebugError( u"Error getting SAS token"_s );
    return token;
  }

  try
  {
    nlohmann::json j = nlohmann::json::parse( content.content() );
    // The collectionId signing endpoints return the plain SAS token
    if ( j.contains( "token" ) )
    {
      token = QString::fromStdString( j.at( "token" ) );
      sas.token = token;
    }
    // The url signing endpoints return the complete SAS signed url, so we need to extract the token
    else if ( j.contains( "href" ) )
    {
      QString href = QString::fromStdString( j.at( "href" ) );
      token = href.remove( url.toString() + '?' );
      sas.token = token;
    }
    // All endpoints return the UTC expiry date
    if ( j.contains( "msft:expiry" ) )
    {
      const QString expiry = QString::fromStdString( j.at( "msft:expiry" ) );
      sas.expiry = QDateTime::fromString( expiry, Qt::ISODate );
    }
    if ( sas.isValid() )
      storeSasToken( authcfg, account, container, sas );
  }
  catch ( nlohmann::json::exception &ex )
  {
    QgsDebugError( u"Error parsing SAS token reply : %1"_s.arg( ex.what() ) );
  }

  return token;
}

void QgsAuthPlanetaryComputerMethod::storeSasToken( const QString &authcfg, const QString &account, const QString &container, const SasToken &token )
{
  const QMutexLocker locker( &mMutex );
  sSasTokensCache.insert( u"%1/%2/%3"_s.arg( authcfg, account, container ), token );
}

QgsAuthPlanetaryComputerMethod::SasToken QgsAuthPlanetaryComputerMethod::retrieveSasToken( const QString &authcfg, const QString &account, const QString &container )
{
  const QMutexLocker locker( &mMutex );
  SasToken sas = sSasTokensCache.value( u"%1/%2/%3"_s.arg( authcfg, account, container ) );
  return sas;
}

#ifdef HAVE_GUI
QWidget *QgsAuthPlanetaryComputerMethod::editWidget( QWidget *parent ) const
{
  return new QgsAuthPlanetaryComputerEdit( parent );
}
#endif

//////////////////////////////////////////////
// Plugin externals
//////////////////////////////////////////////


#ifndef HAVE_STATIC_PROVIDERS
QGISEXTERN QgsAuthMethodMetadata *authMethodMetadataFactory()
{
  return new QgsAuthPlanetaryComputerMethodMetadata();
}
#endif
