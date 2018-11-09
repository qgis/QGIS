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
#include "o2replyserver.h"
#include "qgsapplication.h"
#include "qgsauthoauth2config.h"
#include "qgslogger.h"

#include <QDir>
#include <QSettings>
#include <QUrl>


QString QgsO2::O2_OAUTH2_STATE = QStringLiteral( "state" );

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
  mIsLocalHost = isLocalHost( QUrl( localpolicy.arg( mOAuth2Config->redirectPort() ) ) );

  setTokenUrl( mOAuth2Config->tokenUrl() );
  // refresh token url is marked as optional -- we use the token url if user has not specified a specific refresh URL
  setRefreshTokenUrl( !mOAuth2Config->refreshTokenUrl().isEmpty() ? mOAuth2Config->refreshTokenUrl() : mOAuth2Config->tokenUrl() );

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

bool QgsO2::isLocalHost( const QUrl redirectUrl ) const
{
  QString hostName = redirectUrl.host();
  if ( hostName == QStringLiteral( "localhost" ) || hostName == QStringLiteral( "127.0.0.1" ) || hostName == QStringLiteral( "[::1]" ) )
  {
    return true;
  }
  return false;
}

// slot
void QgsO2::clearProperties()
{
  // TODO: clear object properties
}

void QgsO2::onSetAuthCode( const QString &code )
{
  setCode( code );
  onVerificationReceived( QMap<QString, QString>() );
}

void QgsO2::link()
{
  QgsDebugMsgLevel( QStringLiteral( "QgsO2::link" ), 4 );

  if ( linked() )
  {
    QgsDebugMsgLevel( QStringLiteral( "QgsO2::link: Linked already" ), 4 );
    emit linkingSucceeded();
    return;
  }

  setLinked( false );
  setToken( QString() );
  setTokenSecret( QString() );
  setExtraTokens( QVariantMap() );
  setRefreshToken( QString() );
  setExpires( 0 );

  if ( grantFlow_ == GrantFlowAuthorizationCode || grantFlow_ == GrantFlowImplicit )
  {
    if ( mIsLocalHost )
    {
      // Start listening to authentication replies
      replyServer_->listen( QHostAddress::Any, localPort_ );

      // Save redirect URI, as we have to reuse it when requesting the access token
      redirectUri_ = localhostPolicy_.arg( replyServer_->serverPort() );
    }
    // Assemble initial authentication URL
    QList<QPair<QString, QString> > parameters;
    parameters.append( qMakePair( QString( O2_OAUTH2_RESPONSE_TYPE ), ( grantFlow_ == GrantFlowAuthorizationCode ) ?
                                  QString( O2_OAUTH2_GRANT_TYPE_CODE ) :
                                  QString( O2_OAUTH2_GRANT_TYPE_TOKEN ) ) );
    parameters.append( qMakePair( QString( O2_OAUTH2_CLIENT_ID ), clientId_ ) );
    parameters.append( qMakePair( QString( O2_OAUTH2_REDIRECT_URI ), redirectUri_ ) );
    parameters.append( qMakePair( QString( O2_OAUTH2_SCOPE ), scope_ ) );
    parameters.append( qMakePair( O2_OAUTH2_STATE, state_ ) );
    parameters.append( qMakePair( QString( O2_OAUTH2_API_KEY ), apiKey_ ) );

    for ( QVariantMap::const_iterator iter = extraReqParams_.begin(); iter != extraReqParams_.end(); ++iter )
    {
      parameters.append( qMakePair( iter.key(), iter.value().toString() ) );
    }

    // Show authentication URL with a web browser
    QUrl url( requestUrl_ );
    QUrlQuery query( url );
    query.setQueryItems( parameters );
    url.setQuery( query );
    QgsDebugMsgLevel( QStringLiteral( "QgsO2::link: Emit openBrowser %1" ).arg( url.toString() ), 4 );
    emit openBrowser( url );
    if ( !mIsLocalHost )
    {
      emit getAuthCode();
    }
  }
  else if ( grantFlow_ == GrantFlowResourceOwnerPasswordCredentials )
  {
    QList<O0RequestParameter> parameters;
    parameters.append( O0RequestParameter( O2_OAUTH2_CLIENT_ID, clientId_.toUtf8() ) );
    parameters.append( O0RequestParameter( O2_OAUTH2_CLIENT_SECRET, clientSecret_.toUtf8() ) );
    parameters.append( O0RequestParameter( O2_OAUTH2_USERNAME, username_.toUtf8() ) );
    parameters.append( O0RequestParameter( O2_OAUTH2_PASSWORD, password_.toUtf8() ) );
    parameters.append( O0RequestParameter( O2_OAUTH2_GRANT_TYPE, O2_OAUTH2_GRANT_TYPE_PASSWORD ) );
    parameters.append( O0RequestParameter( O2_OAUTH2_SCOPE, scope_.toUtf8() ) );
    parameters.append( O0RequestParameter( O2_OAUTH2_API_KEY, apiKey_.toUtf8() ) );


    for ( QVariantMap::const_iterator iter = extraReqParams_.begin(); iter != extraReqParams_.end(); ++iter )
    {
      parameters.append( O0RequestParameter( iter.key().toUtf8(), iter.value().toString().toUtf8() ) );
    }


    QByteArray payload = O0BaseAuth::createQueryParameters( parameters );

    QUrl url( tokenUrl_ );
    QNetworkRequest tokenRequest( url );
    tokenRequest.setHeader( QNetworkRequest::ContentTypeHeader, QLatin1Literal( "application/x-www-form-urlencoded" ) );
    QNetworkReply *tokenReply = manager_->post( tokenRequest, payload );

    connect( tokenReply, SIGNAL( finished() ), this, SLOT( onTokenReplyFinished() ), Qt::QueuedConnection );
    connect( tokenReply, SIGNAL( error( QNetworkReply::NetworkError ) ), this, SLOT( onTokenReplyError( QNetworkReply::NetworkError ) ), Qt::QueuedConnection );
  }
}


void QgsO2::setState( const QString & )
{
  qsrand( QTime::currentTime().msec() );
  state_ = QString::number( qrand() );
  Q_EMIT stateChanged();
}


void QgsO2::onVerificationReceived( QMap<QString, QString> response )
{
  QgsDebugMsgLevel( QStringLiteral( "QgsO2::onVerificationReceived: Emitting closeBrowser()" ), 4 );
  emit closeBrowser();

  if ( mIsLocalHost )
  {
    if ( response.contains( QStringLiteral( "error" ) ) )
    {
      QgsDebugMsgLevel( QStringLiteral( "QgsO2::onVerificationReceived: Verification failed: %1" ).arg( response["error"] ), 4 );
      emit linkingFailed();
      return;
    }

    if ( !state_.isEmpty() )
    {
      if ( response.contains( QStringLiteral( "state" ) ) )
      {
        if ( response.value( QStringLiteral( "state" ), QStringLiteral( "ignore" ) ) != state_ )
        {
          QgsDebugMsgLevel( QStringLiteral( "QgsO2::onVerificationReceived: Verification failed: (Response returned wrong state)" ), 3 ) ;
          emit linkingFailed();
          return;
        }
      }
      else
      {
        QgsDebugMsgLevel( QStringLiteral( "QgsO2::onVerificationReceived: Verification failed: (Response does not contain state)" ), 3 );
        emit linkingFailed();
        return;
      }
    }
    // Save access code
    setCode( response.value( QString( O2_OAUTH2_GRANT_TYPE_CODE ) ) );
  }

  if ( grantFlow_ == GrantFlowAuthorizationCode )
  {

    // Exchange access code for access/refresh tokens
    QString query;
    if ( !apiKey_.isEmpty() )
      query = QStringLiteral( "?=%1" ).arg( QString( O2_OAUTH2_API_KEY ), apiKey_ );
    QNetworkRequest tokenRequest( QUrl( tokenUrl_.toString() + query ) );
    tokenRequest.setHeader( QNetworkRequest::ContentTypeHeader, O2_MIME_TYPE_XFORM );
    QMap<QString, QString> parameters;
    parameters.insert( O2_OAUTH2_GRANT_TYPE_CODE, code() );
    parameters.insert( O2_OAUTH2_CLIENT_ID, clientId_ );
    parameters.insert( O2_OAUTH2_CLIENT_SECRET, clientSecret_ );
    parameters.insert( O2_OAUTH2_REDIRECT_URI, redirectUri_ );
    parameters.insert( O2_OAUTH2_GRANT_TYPE, O2_AUTHORIZATION_CODE );
    QByteArray data = buildRequestBody( parameters );
    QNetworkReply *tokenReply = manager_->post( tokenRequest, data );
    timedReplies_.add( tokenReply );
    connect( tokenReply, &QNetworkReply::finished, this, &QgsO2::onTokenReplyFinished, Qt::QueuedConnection );
    connect( tokenReply, qgis::overload<QNetworkReply::NetworkError>::of( &QNetworkReply::error ), this, &QgsO2::onTokenReplyError, Qt::QueuedConnection );
  }
  else if ( grantFlow_ == GrantFlowImplicit )
  {
    // Check for mandatory tokens
    if ( response.contains( O2_OAUTH2_ACCESS_TOKEN ) )
    {
      qDebug() << "O2::onVerificationReceived: Access token returned for implicit flow";
      setToken( response.value( O2_OAUTH2_ACCESS_TOKEN ) );
      if ( response.contains( O2_OAUTH2_EXPIRES_IN ) )
      {
        bool ok = false;
        int expiresIn = response.value( O2_OAUTH2_EXPIRES_IN ).toInt( &ok );
        if ( ok )
        {
          qDebug() << "O2::onVerificationReceived: Token expires in" << expiresIn << "seconds";
          setExpires( QDateTime::currentMSecsSinceEpoch() / 1000 + expiresIn );
        }
      }
      setLinked( true );
      Q_EMIT linkingSucceeded();
    }
    else
    {
      qWarning() << "O2::onVerificationReceived: Access token missing from response for implicit flow";
      Q_EMIT linkingFailed();
    }
  }
  else
  {
    setToken( response.value( O2_OAUTH2_ACCESS_TOKEN ) );
    setRefreshToken( response.value( O2_OAUTH2_REFRESH_TOKEN ) );
  }
}
