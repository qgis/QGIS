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
#include "qgssettings.h"

#include <QCryptographicHash>
#include <QEventLoop>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QRandomGenerator>
#include <QString>
#include <QTimer>
#include <QUrlQuery>
#include <QVariant>

using namespace Qt::StringLiterals;

namespace
{
  constexpr const char *CLAUDE_CLIENT_ID = "9d1c250a-e61b-44d9-88ed-5944d1962f5e";
  constexpr const char *CLAUDE_AUTHORIZE_URL = "https://platform.claude.com/oauth/authorize";
  constexpr const char *CLAUDE_TOKEN_URL = "https://platform.claude.com/v1/oauth/token";
  constexpr const char *CLAUDE_REDIRECT_URI = "https://platform.claude.com/oauth/code/callback";
  constexpr const char *CLAUDE_SCOPE = "user:inference user:profile user:sessions:claude_code user:mcp_servers user:file_upload";
  constexpr int CLAUDE_TOKEN_EXCHANGE_TIMEOUT_MS = 120000;
  constexpr int CLAUDE_TOKEN_REFRESH_TIMEOUT_MS = 30000;

  QString &tokenUrlOverride()
  {
    static QString override;
    return override;
  }

  QString normalizedOAuthInput( const QString &input )
  {
    QString normalized = input.trimmed();
    normalized.replace( '\r', '\n' );
    const int newlineIndex = normalized.indexOf( '\n' );
    if ( newlineIndex >= 0 )
      normalized = normalized.left( newlineIndex ).trimmed();
    return normalized;
  }

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

  QString tokenEndpointUrl()
  {
    const QString override = tokenUrlOverride().trimmed();
    return override.isEmpty() ? QString::fromUtf8( CLAUDE_TOKEN_URL ) : override;
  }

  QString formatTokenResponseError( const QJsonObject &object, const QByteArray &rawBody )
  {
    if ( !object.value( u"access_token"_s ).toString().isEmpty() )
      return u"Claude OAuth token response is incomplete: access token was returned but refresh token is missing."_s;

    if ( object.isEmpty() )
    {
      const QString bodyExcerpt = QString::fromUtf8( rawBody.left( 200 ) ).trimmed();
      if ( !bodyExcerpt.isEmpty() )
        return u"Claude OAuth token response did not include a refresh token (response body: %1)."_s.arg( bodyExcerpt );
      return u"Claude OAuth token response did not include a refresh token (empty response body)."_s;
    }

    return u"Claude OAuth response did not include a refresh token."_s;
  }

  QJsonObject postJsonBlocking( const QJsonObject &payload, int timeoutMs, int &httpStatus, QByteArray &rawBody, QString *errorMessage )
  {
    httpStatus = 0;
    rawBody.clear();
    QgsNetworkAccessManager *nam = QgsNetworkAccessManager::instance();
    if ( !nam )
    {
      if ( errorMessage )
        *errorMessage = u"Network manager is not available."_s;
      return QJsonObject();
    }

    const QUrl endpoint( tokenEndpointUrl() );
    QNetworkRequest request( endpoint );
    request.setHeader( QNetworkRequest::ContentTypeHeader, u"application/json"_s );
    request.setRawHeader( "Accept", "application/json" );
    request.setRawHeader( "anthropic-beta", "oauth-2025-04-20" );
    request.setTransferTimeout( timeoutMs );

    QNetworkReply *reply = nam->post( request, QJsonDocument( payload ).toJson( QJsonDocument::Compact ) );
    if ( !reply )
    {
      if ( errorMessage )
        *errorMessage = u"Unable to start Claude OAuth request."_s;
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
    rawBody = reply->readAll();
    const QNetworkReply::NetworkError networkError = reply->error();
    reply->deleteLater();

    const QJsonDocument doc = QJsonDocument::fromJson( rawBody );
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
          detail = QString::fromUtf8( rawBody.left( 500 ) );
        *errorMessage = u"Claude OAuth request failed (HTTP %1): %2"_s.arg( httpStatus ).arg( detail );
      }
      return QJsonObject();
    }

    return object;
  }

  QString refreshTokenPresenceFlagKey()
  {
    // Non-secret flag mirroring whether a refresh token exists in the encrypted
    // auth manager vault. Reading the flag does not unlock the vault, so UI
    // surfaces (e.g. "Signed in / Not signed in" labels) can be rendered
    // without prompting for the QGIS master password.
    return u"ai/provider/claude/oauth/has_refresh_token"_s;
  }

  bool storeRefreshToken( const QString &refreshToken, QString *errorMessage )
  {
    QgsAuthManager *authManager = QgsApplication::authManager();
    if ( !authManager )
    {
      if ( errorMessage )
        *errorMessage = u"Authentication manager is unavailable."_s;
      return false;
    }
    if ( !authManager->storeAuthSetting( QgsAiClaudeOAuthClient::refreshTokenSettingKey(), refreshToken.trimmed(), true ) )
    {
      if ( errorMessage )
        *errorMessage = u"Unable to store Claude refresh token securely."_s;
      return false;
    }
    QgsSettings().setValue( refreshTokenPresenceFlagKey(), true );
    return true;
  }

  QString storedRefreshToken()
  {
    QgsAuthManager *authManager = QgsApplication::authManager();
    if ( !authManager )
      return QString();
    return authManager->authSetting( QgsAiClaudeOAuthClient::refreshTokenSettingKey(), QVariant(), true ).toString().trimmed();
  }
} //namespace

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

QString QgsAiClaudeOAuthClient::authorizationCodeFromInput( const QString &input )
{
  const QString normalized = normalizedOAuthInput( input );

  if ( normalized.startsWith( "http://"_L1 ) || normalized.startsWith( "https://"_L1 ) )
  {
    const QUrl callbackUrl( normalized );
    QString parsedCode = QUrlQuery( callbackUrl ).queryItemValue( u"code"_s ).trimmed();
    if ( parsedCode.isEmpty() )
      parsedCode = QUrlQuery( callbackUrl.fragment() ).queryItemValue( u"code"_s ).trimmed();
    if ( parsedCode.isEmpty() && !callbackUrl.fragment().contains( '='_L1 ) )
      parsedCode = callbackUrl.fragment().trimmed();
    return parsedCode;
  }

  const QString queryCode = QUrlQuery( normalized ).queryItemValue( u"code"_s ).trimmed();
  if ( !queryCode.isEmpty() )
    return queryCode;

  const int fragmentIndex = normalized.indexOf( '#'_L1 );
  if ( fragmentIndex > 0 )
    return normalized.left( fragmentIndex ).trimmed();

  return normalized;
}

QString QgsAiClaudeOAuthClient::authorizationStateFromInput( const QString &input )
{
  const QString normalized = normalizedOAuthInput( input );

  if ( normalized.startsWith( "http://"_L1 ) || normalized.startsWith( "https://"_L1 ) )
  {
    const QUrl callbackUrl( normalized );
    QString parsedState = QUrlQuery( callbackUrl ).queryItemValue( u"state"_s ).trimmed();
    if ( parsedState.isEmpty() )
      parsedState = QUrlQuery( callbackUrl.fragment() ).queryItemValue( u"state"_s ).trimmed();
    return parsedState;
  }

  const QString queryState = QUrlQuery( normalized ).queryItemValue( u"state"_s ).trimmed();
  if ( !queryState.isEmpty() )
    return queryState;

  const int fragmentIndex = normalized.indexOf( '#'_L1 );
  if ( fragmentIndex > 0 && fragmentIndex < normalized.size() - 1 )
    return normalized.mid( fragmentIndex + 1 ).trimmed();

  return QString();
}

bool QgsAiClaudeOAuthClient::exchangeAuthorizationCode( const QString &authorizationCode, const QString &codeVerifier, const QString &redirectUri, const QString &expectedState, QString *errorMessage )
{
  const QString code = authorizationCodeFromInput( authorizationCode );
  if ( code.isEmpty() )
  {
    if ( errorMessage )
      *errorMessage = u"Authorization code is empty."_s;
    return false;
  }

  QString resolvedState = authorizationStateFromInput( authorizationCode ).trimmed();
  if ( resolvedState.isEmpty() )
    resolvedState = expectedState.trimmed();
  if ( resolvedState.isEmpty() )
  {
    if ( errorMessage )
      *errorMessage = u"OAuth state is missing. Paste the full callback URL or code#state value from Claude."_s;
    return false;
  }

  QJsonObject payload;
  payload.insert( u"grant_type"_s, u"authorization_code"_s );
  payload.insert( u"code"_s, code );
  payload.insert( u"state"_s, resolvedState );
  payload.insert( u"code_verifier"_s, codeVerifier );
  payload.insert( u"client_id"_s, QString::fromUtf8( CLAUDE_CLIENT_ID ) );
  payload.insert( u"redirect_uri"_s, redirectUri );

  int httpStatus = 0;
  QByteArray rawBody;
  QString requestError;
  const QJsonObject object = postJsonBlocking( payload, CLAUDE_TOKEN_EXCHANGE_TIMEOUT_MS, httpStatus, rawBody, &requestError );
  if ( object.isEmpty() && !requestError.isEmpty() )
  {
    if ( errorMessage )
      *errorMessage = requestError;
    return false;
  }

  const QString refreshToken = object.value( u"refresh_token"_s ).toString();
  if ( refreshToken.isEmpty() )
  {
    if ( errorMessage )
      *errorMessage = formatTokenResponseError( object, rawBody );
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
      *errorMessage = u"Missing Claude refresh token. Please sign in with Claude first."_s;
    return false;
  }

  QJsonObject payload;
  payload.insert( u"grant_type"_s, u"refresh_token"_s );
  payload.insert( u"refresh_token"_s, refreshToken );
  payload.insert( u"client_id"_s, QString::fromUtf8( CLAUDE_CLIENT_ID ) );

  int httpStatus = 0;
  QByteArray rawBody;
  const QJsonObject object = postJsonBlocking( payload, CLAUDE_TOKEN_REFRESH_TIMEOUT_MS, httpStatus, rawBody, errorMessage );
  if ( object.isEmpty() )
    return false;

  tokens.accessToken = object.value( u"access_token"_s ).toString();
  tokens.refreshToken = object.value( u"refresh_token"_s ).toString();
  if ( tokens.accessToken.isEmpty() )
  {
    if ( errorMessage )
      *errorMessage = u"Claude refresh response is missing access token."_s;
    return false;
  }

  if ( !tokens.refreshToken.isEmpty() && tokens.refreshToken != refreshToken )
    return storeRefreshToken( tokens.refreshToken, errorMessage );

  return true;
}

bool QgsAiClaudeOAuthClient::hasRefreshToken()
{
  return QgsSettings().value( refreshTokenPresenceFlagKey(), false ).toBool();
}

bool QgsAiClaudeOAuthClient::clearRefreshToken( QString *errorMessage )
{
  QgsAuthManager *authManager = QgsApplication::authManager();
  if ( !authManager )
  {
    if ( errorMessage )
      *errorMessage = u"Authentication manager is unavailable."_s;
    return false;
  }
  if ( !hasRefreshToken() )
    return true;
  if ( !authManager->removeAuthSetting( refreshTokenSettingKey() ) )
    return false;
  QgsSettings().remove( refreshTokenPresenceFlagKey() );
  return true;
}

QString QgsAiClaudeOAuthClient::refreshTokenSettingKey()
{
  return u"ai/provider/claude/oauth/refreshToken"_s;
}

void QgsAiClaudeOAuthClient::setTokenUrlForTesting( const QString &url )
{
  tokenUrlOverride() = url.trimmed();
}

void QgsAiClaudeOAuthClient::clearTokenUrlForTesting()
{
  tokenUrlOverride().clear();
}
