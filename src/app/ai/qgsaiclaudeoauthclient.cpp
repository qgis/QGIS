/***************************************************************************
    qgsaiclaudeoauthclient.cpp
    ---------------------
    begin                : May 2026
    copyright            : (C) 2026 by Francesco Mazzi
    email                : francemazzi at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsaiclaudeoauthclient.h"

#include "qgsapplication.h"
#include "qgsauthmanager.h"
#include "qgsnetworkaccessmanager.h"

#include <QCryptographicHash>
#include <QEventLoop>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QRandomGenerator>
#include <QTimer>
#include <QUrlQuery>
#include <QVariant>

using namespace Qt::StringLiterals;

namespace
{
  constexpr const char *CLAUDE_CLIENT_ID = "9d1c250a-e61b-44d9-88ed-5944d1962f5e";
  constexpr const char *CLAUDE_AUTHORIZE_URL = "https://claude.ai/oauth/authorize";
  constexpr const char *CLAUDE_TOKEN_URL = "https://console.anthropic.com/v1/oauth/token";
  constexpr const char *CLAUDE_REDIRECT_URI = "https://console.anthropic.com/oauth/code/callback";
  constexpr const char *CLAUDE_SCOPE = "org:create_api_key user:profile user:inference";

  QString base64Url( const QByteArray &value )
  {
    return QString::fromLatin1( value.toBase64( QByteArray::Base64UrlEncoding | QByteArray::OmitTrailingEquals ) );
  }

  QString randomBase64Url( int byteCount )
  {
    QByteArray bytes;
    bytes.resize( byteCount );
    for ( int i = 0; i < byteCount; ++i )
      bytes[i] = static_cast<char>( QRandomGenerator::global()->bounded( 256 ) );
    return base64Url( bytes );
  }

  QJsonObject postJsonBlocking( const QJsonObject &payload, int timeoutMs, int &httpStatus, QString *errorMessage )
  {
    httpStatus = 0;
    QgsNetworkAccessManager *nam = QgsNetworkAccessManager::instance();
    if ( !nam )
    {
      if ( errorMessage )
        *errorMessage = QStringLiteral( "Network manager is not available." );
      return QJsonObject();
    }

    QNetworkRequest request( QUrl( QString::fromUtf8( CLAUDE_TOKEN_URL ) ) );
    request.setHeader( QNetworkRequest::ContentTypeHeader, QStringLiteral( "application/json" ) );
    request.setTransferTimeout( timeoutMs );

    QNetworkReply *reply = nam->post( request, QJsonDocument( payload ).toJson( QJsonDocument::Compact ) );
    if ( !reply )
    {
      if ( errorMessage )
        *errorMessage = QStringLiteral( "Unable to start Claude OAuth request." );
      return QJsonObject();
    }

    QEventLoop loop;
    QTimer timer;
    timer.setSingleShot( true );
    QObject::connect( &timer, &QTimer::timeout, &loop, &QEventLoop::quit );
    QObject::connect( reply, &QNetworkReply::finished, &loop, &QEventLoop::quit );
    timer.start( timeoutMs );
    loop.exec();

    if ( timer.isActive() )
      timer.stop();
    else
      reply->abort();

    httpStatus = reply->attribute( QNetworkRequest::HttpStatusCodeAttribute ).toInt();
    const QByteArray body = reply->readAll();
    const QNetworkReply::NetworkError networkError = reply->error();
    reply->deleteLater();

    const QJsonDocument doc = QJsonDocument::fromJson( body );
    const QJsonObject object = doc.isObject() ? doc.object() : QJsonObject();
    if ( networkError != QNetworkReply::NoError || httpStatus < 200 || httpStatus >= 300 )
    {
      if ( errorMessage )
      {
        QString detail = object.value( u"error_description"_s ).toString();
        if ( detail.isEmpty() && object.value( u"error"_s ).isObject() )
          detail = object.value( u"error"_s ).toObject().value( u"message"_s ).toString();
        if ( detail.isEmpty() )
          detail = object.value( u"error"_s ).toString();
        if ( detail.isEmpty() )
          detail = QString::fromUtf8( body.left( 500 ) );
        *errorMessage = QStringLiteral( "Claude OAuth request failed (HTTP %1): %2" ).arg( httpStatus ).arg( detail );
      }
      return QJsonObject();
    }

    return object;
  }

  bool storeRefreshToken( const QString &refreshToken, QString *errorMessage )
  {
    QgsAuthManager *authManager = QgsApplication::authManager();
    if ( !authManager )
    {
      if ( errorMessage )
        *errorMessage = QStringLiteral( "Authentication manager is unavailable." );
      return false;
    }
    if ( !authManager->storeAuthSetting( QgsAiClaudeOAuthClient::refreshTokenSettingKey(), refreshToken.trimmed(), true ) )
    {
      if ( errorMessage )
        *errorMessage = QStringLiteral( "Unable to store Claude refresh token securely." );
      return false;
    }
    return true;
  }

  QString storedRefreshToken()
  {
    QgsAuthManager *authManager = QgsApplication::authManager();
    if ( !authManager )
      return QString();
    return authManager->authSetting( QgsAiClaudeOAuthClient::refreshTokenSettingKey(), QVariant(), true ).toString().trimmed();
  }
}

QgsAiClaudeOAuthClient::AuthorizationRequest QgsAiClaudeOAuthClient::buildAuthorizationRequest()
{
  return buildAuthorizationRequest( QString::fromUtf8( CLAUDE_REDIRECT_URI ) );
}

QgsAiClaudeOAuthClient::AuthorizationRequest QgsAiClaudeOAuthClient::buildAuthorizationRequest( const QString &redirectUri )
{
  AuthorizationRequest request;
  request.codeVerifier = randomBase64Url( 32 );
  request.state = randomBase64Url( 32 );
  request.redirectUri = redirectUri.trimmed().isEmpty() ? QString::fromUtf8( CLAUDE_REDIRECT_URI ) : redirectUri.trimmed();

  const QString codeChallenge = base64Url( QCryptographicHash::hash( request.codeVerifier.toUtf8(), QCryptographicHash::Sha256 ) );
  QUrl url( QString::fromUtf8( CLAUDE_AUTHORIZE_URL ) );
  QUrlQuery query;
  query.addQueryItem( u"code"_s, u"true"_s );
  query.addQueryItem( u"response_type"_s, u"code"_s );
  query.addQueryItem( u"client_id"_s, QString::fromUtf8( CLAUDE_CLIENT_ID ) );
  query.addQueryItem( u"redirect_uri"_s, request.redirectUri );
  query.addQueryItem( u"code_challenge"_s, codeChallenge );
  query.addQueryItem( u"code_challenge_method"_s, u"S256"_s );
  query.addQueryItem( u"state"_s, request.state );
  query.addQueryItem( u"scope"_s, QString::fromUtf8( CLAUDE_SCOPE ) );
  url.setQuery( query );
  request.authorizationUrl = url;
  return request;
}

bool QgsAiClaudeOAuthClient::exchangeAuthorizationCode( const QString &authorizationCode, const QString &codeVerifier, const QString &redirectUri, QString *errorMessage )
{
  QString code = authorizationCode.trimmed();
  if ( code.startsWith( "http://"_L1 ) || code.startsWith( "https://"_L1 ) )
  {
    const QUrl callbackUrl( code );
    code = QUrlQuery( callbackUrl ).queryItemValue( u"code"_s ).trimmed();
  }

  if ( code.isEmpty() )
  {
    if ( errorMessage )
      *errorMessage = QStringLiteral( "Authorization code is empty." );
    return false;
  }

  QJsonObject payload;
  payload.insert( u"grant_type"_s, u"authorization_code"_s );
  payload.insert( u"code"_s, code );
  payload.insert( u"code_verifier"_s, codeVerifier );
  payload.insert( u"client_id"_s, QString::fromUtf8( CLAUDE_CLIENT_ID ) );
  payload.insert( u"redirect_uri"_s, redirectUri );

  int httpStatus = 0;
  const QJsonObject object = postJsonBlocking( payload, 30000, httpStatus, errorMessage );
  const QString refreshToken = object.value( u"refresh_token"_s ).toString();
  if ( refreshToken.isEmpty() )
  {
    if ( errorMessage )
      *errorMessage = QStringLiteral( "Claude OAuth response did not include a refresh token." );
    return false;
  }
  return storeRefreshToken( refreshToken, errorMessage );
}

bool QgsAiClaudeOAuthClient::refreshAccessToken( TokenSet &tokens, QString *errorMessage )
{
  const QString refreshToken = storedRefreshToken();
  if ( refreshToken.isEmpty() )
  {
    if ( errorMessage )
      *errorMessage = QStringLiteral( "Missing Claude refresh token. Please sign in with Claude first." );
    return false;
  }

  QJsonObject payload;
  payload.insert( u"grant_type"_s, u"refresh_token"_s );
  payload.insert( u"refresh_token"_s, refreshToken );
  payload.insert( u"client_id"_s, QString::fromUtf8( CLAUDE_CLIENT_ID ) );

  int httpStatus = 0;
  const QJsonObject object = postJsonBlocking( payload, 30000, httpStatus, errorMessage );
  if ( object.isEmpty() )
    return false;

  tokens.accessToken = object.value( u"access_token"_s ).toString();
  tokens.refreshToken = object.value( u"refresh_token"_s ).toString();
  if ( tokens.accessToken.isEmpty() )
  {
    if ( errorMessage )
      *errorMessage = QStringLiteral( "Claude refresh response is missing access token." );
    return false;
  }

  if ( !tokens.refreshToken.isEmpty() && tokens.refreshToken != refreshToken )
    return storeRefreshToken( tokens.refreshToken, errorMessage );

  return true;
}

bool QgsAiClaudeOAuthClient::hasRefreshToken()
{
  return !storedRefreshToken().isEmpty();
}

bool QgsAiClaudeOAuthClient::clearRefreshToken( QString *errorMessage )
{
  QgsAuthManager *authManager = QgsApplication::authManager();
  if ( !authManager )
  {
    if ( errorMessage )
      *errorMessage = QStringLiteral( "Authentication manager is unavailable." );
    return false;
  }
  if ( !hasRefreshToken() )
    return true;
  return authManager->removeAuthSetting( refreshTokenSettingKey() );
}

QString QgsAiClaudeOAuthClient::refreshTokenSettingKey()
{
  return QStringLiteral( "ai/provider/claude/oauth/refreshToken" );
}
