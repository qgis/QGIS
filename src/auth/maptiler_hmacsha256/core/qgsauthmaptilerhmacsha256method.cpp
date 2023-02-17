/***************************************************************************
    qgsauthmaptilerhmacsha256method.cpp
    --------------------------
    begin                : January 2022
    copyright            : (C) 2022 by Vincent Cloarec
    author               : Vincent Cloarec
    email                : vcloarec at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsauthmaptilerhmacsha256method.h"

#include <QMessageAuthenticationCode>
#include <QUrlQuery>

#include "qgsauthmanager.h"
#include "qgslogger.h"
#include "qgsapplication.h"

#ifdef HAVE_GUI
#include "qgsauthmaptilerhmacsha256edit.h"
#endif


const QString QgsAuthMapTilerHmacSha256Method::AUTH_METHOD_KEY = QStringLiteral( "MapTilerHmacSha256" );
const QString QgsAuthMapTilerHmacSha256Method::AUTH_METHOD_DESCRIPTION = QStringLiteral( "MapTiler HMAC-SHA256" );
const QString QgsAuthMapTilerHmacSha256Method::AUTH_METHOD_DISPLAY_DESCRIPTION = tr( "MapTiler HMAC SHA256-Signature" );

QMap<QString, QgsAuthMethodConfig> QgsAuthMapTilerHmacSha256Method::sAuthConfigCache = QMap<QString, QgsAuthMethodConfig>();


QgsAuthMapTilerHmacSha256Method::QgsAuthMapTilerHmacSha256Method()
{
  setVersion( 1 );
  setExpansions( QgsAuthMethod::NetworkRequest );
  setDataProviders( QStringList()
                    << QStringLiteral( "wms" )
                    << QStringLiteral( "vectortile" ) );

}

QString QgsAuthMapTilerHmacSha256Method::key() const
{
  return AUTH_METHOD_KEY;
}

QString QgsAuthMapTilerHmacSha256Method::description() const
{
  return AUTH_METHOD_DESCRIPTION;
}

QString QgsAuthMapTilerHmacSha256Method::displayDescription() const
{
  return AUTH_METHOD_DISPLAY_DESCRIPTION;
}

bool QgsAuthMapTilerHmacSha256Method::updateNetworkRequest( QNetworkRequest &request, const QString &authcfg,
    const QString &dataprovider )
{
  Q_UNUSED( dataprovider )
  const QgsAuthMethodConfig config = getMethodConfig( authcfg );
  if ( !config.isValid() )
  {
    QgsDebugMsg( QStringLiteral( "Update request config FAILED for authcfg: %1: config invalid" ).arg( authcfg ) );
    return false;
  }

  const QString token = config.config( QStringLiteral( "token" ) );
  const QStringList splitToken = token.split( '_' );

  if ( splitToken.count() != 2 )
  {
    QgsDebugMsg( QStringLiteral( "Update request config FAILED for authcfg: %1: config invalid" ).arg( authcfg ) );
    return false;
  }

  const QString key = splitToken.at( 0 );
  const QString secret = splitToken.at( 1 );

  QUrl url = request.url();
  QUrlQuery query( url.query() );
  query.removeQueryItem( QStringLiteral( "key" ) );

  QList<QPair<QString, QString> > queryItems = query.queryItems();

  queryItems.append( {QStringLiteral( "key" ), key} );

  query.setQueryItems( queryItems );
  url.setQuery( query );

  QString signature = calculateSignature( secret, url.toEncoded() );
  request.setUrl( QString( url.url() + QStringLiteral( "&signature=" ) + signature ) );

  return true;
}

void QgsAuthMapTilerHmacSha256Method::clearCachedConfig( const QString &authcfg )
{
  removeMethodConfig( authcfg );
}

void QgsAuthMapTilerHmacSha256Method::updateMethodConfig( QgsAuthMethodConfig &mconfig )
{
  if ( mconfig.hasConfig( QStringLiteral( "oldconfigstyle" ) ) )
  {
    QgsDebugMsgLevel( QStringLiteral( "Updating old style auth method config" ), 2 );
  }

  // NOTE: add updates as method version() increases due to config storage changes
}

QgsAuthMethodConfig QgsAuthMapTilerHmacSha256Method::getMethodConfig( const QString &authcfg, bool fullconfig )
{
  const QMutexLocker locker( &mMutex );
  QgsAuthMethodConfig config;

  // check if it is cached
  if ( sAuthConfigCache.contains( authcfg ) )
  {
    config = sAuthConfigCache.value( authcfg );
    QgsDebugMsgLevel( QStringLiteral( "Retrieved config for authcfg: %1" ).arg( authcfg ), 2 );
    return config;
  }

  // else build basic bundle
  if ( !QgsApplication::authManager()->loadAuthenticationConfig( authcfg, config, fullconfig ) )
  {
    QgsDebugMsgLevel( QStringLiteral( "Retrieved config for authcfg: %1" ).arg( authcfg ), 2 );
    return QgsAuthMethodConfig();
  }

  // cache bundle
  putMethodConfig( authcfg, config );

  return config;
}

void QgsAuthMapTilerHmacSha256Method::putMethodConfig( const QString &authcfg, const QgsAuthMethodConfig &mconfig )
{
  const QMutexLocker locker( &mMutex );
  QgsDebugMsgLevel( QStringLiteral( "Putting token config for authcfg: %1" ).arg( authcfg ), 2 );
  sAuthConfigCache.insert( authcfg, mconfig );
}

void QgsAuthMapTilerHmacSha256Method::removeMethodConfig( const QString &authcfg )
{
  const QMutexLocker locker( &mMutex );
  if ( sAuthConfigCache.contains( authcfg ) )
  {
    sAuthConfigCache.remove( authcfg );
    QgsDebugMsgLevel( QStringLiteral( "Removed token config for authcfg: %1" ).arg( authcfg ), 2 );
  }
}

QByteArray QgsAuthMapTilerHmacSha256Method::calculateSignature( const QString &token, const QString &keyedUrl )
{
  QByteArray decodedToken = QByteArray::fromHex( token.toStdString().c_str() );

  QMessageAuthenticationCode authCode( QCryptographicHash::Sha256, decodedToken );
  authCode.addData( keyedUrl.toUtf8() );

  return authCode.result().toBase64( QByteArray::Base64UrlEncoding );
}

#ifdef HAVE_GUI
QWidget *QgsAuthMapTilerHmacSha256Method::editWidget( QWidget *parent ) const
{
  return new QgsAuthMapTilerHmacSha256Edit( parent );
}
#endif

//////////////////////////////////////////////
// Plugin externals
//////////////////////////////////////////////


#ifndef HAVE_STATIC_PROVIDERS
QGISEXTERN QgsAuthMethodMetadata *authMethodMetadataFactory()
{
  return new QgsAuthMapTilerHmacSha256MethodMetadata();
}
#endif
