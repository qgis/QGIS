/***************************************************************************
    begin                : August 1, 2016
    copyright            : (C) 2016 by Monsanto Company, USA
    author               : Larry Shaffer, Boundless Spatial
    email                : lshaffer at boundlessgeo dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgso2.h"

#include "o0globals.h"
#include "o0settingsstore.h"
#include "qgsapplication.h"
#include "qgsauthoauth2config.h"
#include "qgslogger.h"

#include <QDir>
#include <QSettings>
#include <QUrl>


QgsO2::QgsO2( const QString &authcfg, QgsAuthOAuth2Config *oauth2config,
              QObject *parent, QNetworkAccessManager *manager )
  : O2( parent, manager )
  , mTokenCacheFile( QString::null )
  , mAuthcfg( authcfg )
  , mOAuth2Config( oauth2config )
{
  initOAuthConfig();
}

QgsO2::~QgsO2()
{
  // FIXME: This crashes app on QgsApplication destruction
  //        Verify that objects are actually being deleted via QgsAuthManager's destruction
  //mOAuth2Config->deleteLater();

  if ( mTokenCacheFile.startsWith( QgsAuthOAuth2Config::tokenCacheDirectory( true ) )
       && QFile::exists( mTokenCacheFile ) )
  {
    if ( !QFile::remove( mTokenCacheFile ) )
    {
      QgsDebugMsg( QStringLiteral( "Could not remove temp token cache file: %1" ).arg( mTokenCacheFile ) );
    }
  }
}

void QgsO2::initOAuthConfig()
{
  if ( !mOAuth2Config )
  {
    return;
  }

  // common properties to all grant flows
  QString localpolicy = QStringLiteral( "http://127.0.0.1:% 1/%1" ).arg( mOAuth2Config->redirectUrl() ).replace( QStringLiteral( "% 1" ), QStringLiteral( "%1" ) );
  QgsDebugMsg( QStringLiteral( "localpolicy(w/port): %1" ).arg( localpolicy.arg( mOAuth2Config->redirectPort() ) ) );
  setLocalhostPolicy( localpolicy );
  setLocalPort( mOAuth2Config->redirectPort() );

  setTokenUrl( mOAuth2Config->tokenUrl() );
  setRefreshTokenUrl( mOAuth2Config->refreshTokenUrl() );

  setScope( mOAuth2Config->scope() );
  // TODO: add support to O2 (or this class?) for state query param

  // common optional properties
  setApiKey( mOAuth2Config->apiKey() );
  setExtraRequestParams( mOAuth2Config->queryPairs() );

  O2::GrantFlow o2flow;
  switch ( mOAuth2Config->grantFlow() )
  {
    case QgsAuthOAuth2Config::AuthCode:
      o2flow = O2::GrantFlowAuthorizationCode;
      setRequestUrl( mOAuth2Config->requestUrl() );
      setClientId( mOAuth2Config->clientId() );
      setClientSecret( mOAuth2Config->clientSecret() );

      break;
    case QgsAuthOAuth2Config::Implicit:
      o2flow = O2::GrantFlowImplicit;
      setRequestUrl( mOAuth2Config->requestUrl() );
      setClientId( mOAuth2Config->clientId() );

      break;
    case QgsAuthOAuth2Config::ResourceOwner:
      o2flow = O2::GrantFlowResourceOwnerPasswordCredentials;
      setClientId( mOAuth2Config->clientId() );
      setClientSecret( mOAuth2Config->clientSecret() );
      setUsername( mOAuth2Config->username() );
      setPassword( mOAuth2Config->password() );

      break;
  }
  setGrantFlow( o2flow );

  setSettingsStore( mOAuth2Config->persistToken() );

  setVerificationResponseContent();
}

void QgsO2::setSettingsStore( bool persist )
{
  mTokenCacheFile = QgsAuthOAuth2Config::tokenCachePath( mAuthcfg, !persist );

  QSettings *settings = new QSettings( mTokenCacheFile, QSettings::IniFormat );
  O0SettingsStore *store = new O0SettingsStore( settings, O2_ENCRYPTION_KEY );
  store->setGroupKey( QStringLiteral( "authcfg_%1" ).arg( mAuthcfg ) );
  setStore( store );
}

void QgsO2::setVerificationResponseContent()
{
  QFile verhtml( QStringLiteral( ":/oauth2method/oauth2_verification_finished.html" ) );
  if ( verhtml.open( QIODevice::ReadOnly | QIODevice::Text ) )
  {
    setReplyContent( verhtml.readAll() );
  }
}

// slot
void QgsO2::clearProperties()
{
  // TODO: clear object properties
}
