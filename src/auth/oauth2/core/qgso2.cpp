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
#include "moc_qgso2.cpp"

#include "o0globals.h"
#include "o0settingsstore.h"
#include "o2replyserver.h"
#include "qgsauthoauth2config.h"
#include "qgslogger.h"
#include "qgsnetworkaccessmanager.h"
#include "qgssetrequestinitiator_p.h"
#include "qgsblockingnetworkrequest.h"

#include <QDir>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSettings>
#include <QUrl>
#include <QUrlQuery>
#include <QCryptographicHash>
#include <QRandomGenerator>

QString QgsO2::O2_OAUTH2_STATE = QStringLiteral( "state" );

QgsO2::QgsO2( const QString &authcfg, QgsAuthOAuth2Config *oauth2config, QObject *parent, QNetworkAccessManager *manager )
  : O2( parent, manager )
  , mTokenCacheFile( QString() )
  , mAuthcfg( authcfg )
  , mOAuth2Config( oauth2config )
{
  static std::once_flag initialized;
  std::call_once( initialized, [=]() {
    setLoggingFunction( []( const QString &message, LogLevel level ) {
#ifdef QGISDEBUG
      switch ( level )
      {
        case O0BaseAuth::LogLevel::Debug:
          QgsDebugMsgLevel( message, 2 );
          break;
        case O0BaseAuth::LogLevel::Warning:
        case O0BaseAuth::LogLevel::Critical:
          QgsDebugError( message );
          break;
      }
#else
                                         ( void ) message;
                                         ( void ) level;
#endif
    } );
  } );

  if ( mOAuth2Config )
    mOAuth2Config->setParent( this );

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
      QgsDebugError( QStringLiteral( "Could not remove temp token cache file: %1" ).arg( mTokenCacheFile ) );
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
  const QString localpolicy = QStringLiteral( "http://%1:% 1/%2" ).arg( mOAuth2Config->redirectHost(), mOAuth2Config->redirectUrl() ).replace( QLatin1String( "% 1" ), QLatin1String( "%1" ) );
  QgsDebugMsgLevel( QStringLiteral( "localpolicy(w/port): %1" ).arg( localpolicy.arg( mOAuth2Config->redirectPort() ) ), 2 );
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

  switch ( mOAuth2Config->grantFlow() )
  {
    case QgsAuthOAuth2Config::GrantFlow::Pkce:
      setGrantFlow( O2::GrantFlowPkce );
      setRequestUrl( mOAuth2Config->requestUrl() );
      setClientId( mOAuth2Config->clientId() );
      // No client secret with PKCE
      //setClientSecret( mOAuth2Config->clientSecret() );

      break;
    case QgsAuthOAuth2Config::GrantFlow::AuthCode:
      setGrantFlow( O2::GrantFlowAuthorizationCode );
      setRequestUrl( mOAuth2Config->requestUrl() );
      setClientId( mOAuth2Config->clientId() );
      setClientSecret( mOAuth2Config->clientSecret() );

      break;
    case QgsAuthOAuth2Config::GrantFlow::Implicit:
      setGrantFlow( O2::GrantFlowImplicit );
      setRequestUrl( mOAuth2Config->requestUrl() );
      setClientId( mOAuth2Config->clientId() );

      break;
    case QgsAuthOAuth2Config::GrantFlow::ResourceOwner:
      setGrantFlow( O2::GrantFlowResourceOwnerPasswordCredentials );
      setClientId( mOAuth2Config->clientId() );
      setClientSecret( mOAuth2Config->clientSecret() );
      setUsername( mOAuth2Config->username() );
      setPassword( mOAuth2Config->password() );

      break;
  }

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
    setReplyContent( QString::fromUtf8( verhtml.readAll() )
                       .replace( QLatin1String( "{{ H2_TITLE }}" ), tr( "QGIS OAuth2 verification has finished" ) )
                       .replace( QLatin1String( "{{ H3_TITLE }}" ), tr( "If you have not been returned to QGIS, bring the application to the forefront." ) )
                       .replace( QLatin1String( "{{ CLOSE_WINDOW }}" ), tr( "Close window" ) )
                       .toUtf8()
    );
  }
}

bool QgsO2::isLocalHost( const QUrl redirectUrl ) const
{
  const QString hostName = redirectUrl.host();
  if ( hostName == QLatin1String( "localhost" ) || hostName == QLatin1String( "127.0.0.1" ) || hostName == QLatin1String( "[::1]" ) )
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
  Q_ASSERT( thread() == QThread::currentThread() );

  QgsDebugMsgLevel( QStringLiteral( "QgsO2::link" ), 4 );

  // Create the reply server if it doesn't exist
  // and we don't use an external web interceptor
  if ( !useExternalWebInterceptor_ )
  {
    if ( replyServer() == NULL )
    {
      O2ReplyServer *replyServer = new O2ReplyServer( this );
      connect( replyServer, &O2ReplyServer::verificationReceived, this, &QgsO2::onVerificationReceived );
      connect( replyServer, &O2ReplyServer::serverClosed, this, &QgsO2::serverHasClosed );
      setReplyServer( replyServer );
    }
  }

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

  if ( grantFlow_ == GrantFlowAuthorizationCode || grantFlow_ == GrantFlowImplicit || grantFlow_ == GrantFlowPkce )
  {
    if ( useExternalWebInterceptor_ )
    {
      // Save redirect URI, as we have to reuse it when requesting the access token
      redirectUri_ = localhostPolicy_.arg( localPort() );
    }
    else
    {
      if ( mIsLocalHost )
      {
        if ( !replyServer()->isListening() )
        {
          // Start listening to authentication replies
          if ( replyServer()->listen( QHostAddress::Any, localPort_ ) )
          {
            QgsDebugMsgLevel( QStringLiteral( "O2::link: Reply server listening on port %1" ).arg( localPort() ), 2 );
          }
          else
          {
            QgsDebugError( QStringLiteral( "O2::link: Reply server failed to start listening on port %1" ).arg( localPort() ) );
            emit linkingFailed();
            return;
          }
        }

        // Save redirect URI, as we have to reuse it when requesting the access token
        redirectUri_ = localhostPolicy_.arg( replyServer()->serverPort() );
      }
    }
    // Assemble initial authentication URL
    QList<QPair<QString, QString>> parameters;
    parameters.append( qMakePair( QString( O2_OAUTH2_RESPONSE_TYPE ), ( grantFlow_ == GrantFlowAuthorizationCode || grantFlow_ == GrantFlowPkce ) ? QString( O2_OAUTH2_GRANT_TYPE_CODE ) : QString( O2_OAUTH2_GRANT_TYPE_TOKEN ) ) );
    parameters.append( qMakePair( QString( O2_OAUTH2_CLIENT_ID ), clientId_ ) );
    parameters.append( qMakePair( QString( O2_OAUTH2_REDIRECT_URI ), redirectUri_ ) );

    if ( grantFlow_ == GrantFlowPkce )
    {
      pkceCodeVerifier_ = ( QUuid::createUuid().toString( QUuid::WithoutBraces ) + QUuid::createUuid().toString( QUuid::WithoutBraces ) ).toLatin1();
      pkceCodeChallenge_ = QCryptographicHash::hash( pkceCodeVerifier_, QCryptographicHash::Sha256 ).toBase64( QByteArray::Base64UrlEncoding | QByteArray::OmitTrailingEquals );
      parameters.append( qMakePair( QString( O2_OAUTH2_PKCE_CODE_CHALLENGE_PARAM ), pkceCodeChallenge_ ) );
      parameters.append( qMakePair( QString( O2_OAUTH2_PKCE_CODE_CHALLENGE_METHOD_PARAM ), QString( O2_OAUTH2_PKCE_CODE_CHALLENGE_METHOD_S256 ) ) );
    }

    if ( !scope_.isEmpty() )
      parameters.append( qMakePair( QString( O2_OAUTH2_SCOPE ), scope_ ) );
    if ( !state_.isEmpty() )
      parameters.append( qMakePair( O2_OAUTH2_STATE, state_ ) );
    if ( !apiKey_.isEmpty() )
      parameters.append( qMakePair( QString( O2_OAUTH2_API_KEY ), apiKey_ ) );

    for ( auto iter = extraReqParams_.constBegin(); iter != extraReqParams_.constEnd(); ++iter )
    {
      parameters.append( qMakePair( iter.key(), iter.value().toString() ) );
    }

    // Show authentication URL with a web browser
    QUrl url( requestUrl_ );
    QUrlQuery query( url );
    query.setQueryItems( parameters );
    url.setQuery( query );
    QgsDebugMsgLevel( QStringLiteral( "QgsO2::link: Emit openBrowser %1" ).arg( url.toString() ), 4 );
    QgsNetworkAccessManager::instance()->requestAuthOpenBrowser( url );
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
    if ( !scope_.isEmpty() )
      parameters.append( O0RequestParameter( O2_OAUTH2_SCOPE, scope_.toUtf8() ) );
    if ( !apiKey_.isEmpty() )
      parameters.append( O0RequestParameter( O2_OAUTH2_API_KEY, apiKey_.toUtf8() ) );

    for ( auto iter = extraReqParams_.constBegin(); iter != extraReqParams_.constEnd(); ++iter )
    {
      parameters.append( O0RequestParameter( iter.key().toUtf8(), iter.value().toString().toUtf8() ) );
    }

    const QByteArray payload = O0BaseAuth::createQueryParameters( parameters );

    const QUrl url( tokenUrl_ );
    QNetworkRequest tokenRequest( url );
    QgsSetRequestInitiatorClass( tokenRequest, QStringLiteral( "QgsO2" ) );
    tokenRequest.setHeader( QNetworkRequest::ContentTypeHeader, QLatin1String( "application/x-www-form-urlencoded" ) );
    QNetworkReply *tokenReply = getManager()->post( tokenRequest, payload );

    connect( tokenReply, &QNetworkReply::finished, this, &QgsO2::onTokenReplyFinished, Qt::QueuedConnection );
    connect( tokenReply, &QNetworkReply::errorOccurred, this, &QgsO2::onTokenReplyError, Qt::QueuedConnection );
  }
}

void QgsO2::setState( const QString & )
{
  state_ = QString::number( QRandomGenerator::system()->generate() );
  Q_EMIT stateChanged();
}

void QgsO2::onVerificationReceived( QMap<QString, QString> response )
{
  QgsDebugMsgLevel( QStringLiteral( "QgsO2::onVerificationReceived: Emitting closeBrowser()" ), 4 );
  QgsNetworkAccessManager::instance()->requestAuthCloseBrowser();

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
          QgsDebugMsgLevel( QStringLiteral( "QgsO2::onVerificationReceived: Verification failed: (Response returned wrong state)" ), 3 );
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

  if ( grantFlow_ == GrantFlowAuthorizationCode || grantFlow_ == GrantFlowPkce )
  {
    // Exchange access code for access/refresh tokens
    QString query;
    if ( !apiKey_.isEmpty() )
      query = QStringLiteral( "?=%1" ).arg( QString( O2_OAUTH2_API_KEY ), apiKey_ );
    QNetworkRequest tokenRequest( QUrl( tokenUrl_.toString() + query ) );
    QgsSetRequestInitiatorClass( tokenRequest, QStringLiteral( "QgsO2" ) );
    tokenRequest.setHeader( QNetworkRequest::ContentTypeHeader, O2_MIME_TYPE_XFORM );
    QMap<QString, QString> parameters;
    parameters.insert( O2_OAUTH2_GRANT_TYPE_CODE, code() );
    parameters.insert( O2_OAUTH2_CLIENT_ID, clientId_ );
    //No client secret with PKCE
    if ( grantFlow_ != GrantFlowPkce )
    {
      parameters.insert( O2_OAUTH2_CLIENT_SECRET, clientSecret_ );
    }
    parameters.insert( O2_OAUTH2_REDIRECT_URI, redirectUri_ );
    parameters.insert( O2_OAUTH2_GRANT_TYPE, O2_AUTHORIZATION_CODE );
    if ( grantFlow() == GrantFlowPkce )
    {
      parameters.insert( O2_OAUTH2_PKCE_CODE_VERIFIER_PARAM, pkceCodeVerifier_ );
    }
    const QByteArray data = buildRequestBody( parameters );
    QNetworkReply *tokenReply = getManager()->post( tokenRequest, data );
    timedReplies_.add( tokenReply );
    connect( tokenReply, &QNetworkReply::finished, this, &QgsO2::onTokenReplyFinished, Qt::QueuedConnection );
    connect( tokenReply, &QNetworkReply::errorOccurred, this, &QgsO2::onTokenReplyError, Qt::QueuedConnection );
  }
  else if ( grantFlow_ == GrantFlowImplicit )
  {
    // Check for mandatory tokens
    if ( response.contains( O2_OAUTH2_ACCESS_TOKEN ) )
    {
      QgsDebugMsgLevel( QStringLiteral( "O2::onVerificationReceived: Access token returned for implicit flow" ), 2 );
      setToken( response.value( O2_OAUTH2_ACCESS_TOKEN ) );
      if ( response.contains( O2_OAUTH2_EXPIRES_IN ) )
      {
        bool ok = false;
        const int expiresIn = response.value( O2_OAUTH2_EXPIRES_IN ).toInt( &ok );
        if ( ok )
        {
          QgsDebugMsgLevel( QStringLiteral( "O2::onVerificationReceived: Token expires in %1 seconds" ).arg( expiresIn ), 2 );
          setExpires( QDateTime::currentMSecsSinceEpoch() / 1000 + static_cast<qint64>( expiresIn ) );
        }
      }
      setLinked( true );
      Q_EMIT linkingSucceeded();
    }
    else
    {
      QgsDebugError( QStringLiteral( "O2::onVerificationReceived: Access token missing from response for implicit flow" ) );
      Q_EMIT linkingFailed();
    }
  }
  else
  {
    setToken( response.value( O2_OAUTH2_ACCESS_TOKEN ) );
    setRefreshToken( response.value( O2_OAUTH2_REFRESH_TOKEN ) );
  }
}

QNetworkAccessManager *QgsO2::getManager()
{
  return QgsNetworkAccessManager::instance();
}

/// Parse JSON data into a QVariantMap
static QVariantMap parseTokenResponse( const QByteArray &data )
{
  QJsonParseError err;
  const QJsonDocument doc = QJsonDocument::fromJson( data, &err );
  if ( err.error != QJsonParseError::NoError )
  {
    QgsDebugError( QStringLiteral( "parseTokenResponse: Failed to parse token response due to err: %1" ).arg( err.errorString() ) );
    return QVariantMap();
  }

  if ( !doc.isObject() )
  {
    QgsDebugError( QStringLiteral( "parseTokenResponse: Token response is not an object" ) );
    return QVariantMap();
  }

  return doc.object().toVariantMap();
}

// Code adapted from O2::refresh(), but using QgsBlockingNetworkRequest
void QgsO2::refreshSynchronous()
{
  QgsDebugMsgLevel( QStringLiteral( "O2::refresh: Token: ... %1" ).arg( refreshToken().right( 7 ) ), 2 );

  if ( refreshToken().isEmpty() )
  {
    QgsDebugError( QStringLiteral( "O2::refresh: No refresh token" ) );
    onRefreshError( QNetworkReply::AuthenticationRequiredError );
    return;
  }
  if ( refreshTokenUrl_.isEmpty() )
  {
    QgsDebugError( QStringLiteral( "O2::refresh: Refresh token URL not set" ) );
    onRefreshError( QNetworkReply::AuthenticationRequiredError );
    return;
  }

  QNetworkRequest refreshRequest( refreshTokenUrl_ );
  refreshRequest.setHeader( QNetworkRequest::ContentTypeHeader, O2_MIME_TYPE_XFORM );
  QMap<QString, QString> parameters;
  parameters.insert( O2_OAUTH2_CLIENT_ID, clientId_ );
  // No secret with PKCE
  if ( grantFlow_ != GrantFlowPkce )
  {
    parameters.insert( O2_OAUTH2_CLIENT_SECRET, clientSecret_ );
  }
  parameters.insert( O2_OAUTH2_REFRESH_TOKEN, refreshToken() );
  parameters.insert( O2_OAUTH2_GRANT_TYPE, O2_OAUTH2_REFRESH_TOKEN );

  const QByteArray data = buildRequestBody( parameters );

  QgsBlockingNetworkRequest blockingRequest;
  const QgsBlockingNetworkRequest::ErrorCode errCode = blockingRequest.post( refreshRequest, data, true );
  if ( errCode == QgsBlockingNetworkRequest::NoError )
  {
    const QByteArray reply = blockingRequest.reply().content();
    const QVariantMap tokens = parseTokenResponse( reply );
    if ( tokens.contains( QStringLiteral( "error" ) ) )
    {
      QgsDebugError( QStringLiteral( "Error refreshing token %1" ).arg( tokens.value( QStringLiteral( "error" ) ).toMap().value( QStringLiteral( "message" ) ).toString().toLocal8Bit().constData() ) );
      unlink();
    }
    else
    {
      setToken( tokens.value( O2_OAUTH2_ACCESS_TOKEN ).toString() );
      const int expiresIn = tokens.value( O2_OAUTH2_EXPIRES_IN ).toInt();
      setExpires( QDateTime::currentMSecsSinceEpoch() / 1000 + static_cast<qint64>( expiresIn ) );
      const QString refreshToken = tokens.value( O2_OAUTH2_REFRESH_TOKEN ).toString();
      if ( !refreshToken.isEmpty() )
        setRefreshToken( refreshToken );
      setLinked( true );
      QgsDebugMsgLevel( QStringLiteral( "New token expires in %1 seconds" ).arg( expiresIn ), 2 );
      emit linkingSucceeded();
    }
    emit refreshFinished( QNetworkReply::NoError );
  }
  else
  {
    unlink();
    QgsDebugError( QStringLiteral( "O2::onRefreshFinished: Error %1" ).arg( blockingRequest.errorMessage() ) );
    emit refreshFinished( blockingRequest.reply().error() );
  }
}

void QgsO2::computeExpirationDelay()
{
  const qint64 lExpires = expires();
  mExpirationDelay = static_cast<int>( lExpires > 0 ? lExpires - static_cast<qint64>( QDateTime::currentMSecsSinceEpoch() / 1000 ) : 0 );
}
