/***************************************************************************
    qgsaicodexoauthclient.cpp
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

#include "qgsaicodexoauthclient.h"

#include <algorithm>

#include "qgsaisecretstore.h"
#include "qgsnetworkaccessmanager.h"
#include "qgssettings.h"

#include <QByteArray>
#include <QElapsedTimer>
#include <QEventLoop>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QString>
#include <QTimer>
#include <QUrl>
#include <QUrlQuery>
#include <QVariant>

using namespace Qt::StringLiterals;

namespace
{
  constexpr const char *CODEX_CLIENT_ID = "app_EMoamEEZ73f0CkXaXp7hrann";
  constexpr const char *CODEX_AUTH_ISSUER = "https://auth.openai.com";
  constexpr const char *CODEX_DEVICE_API_BASE = "https://auth.openai.com/api/accounts";
  constexpr const char *CODEX_REDIRECT_URI = "https://auth.openai.com/deviceauth/callback";

  QByteArray base64UrlDecode( QString value )
  {
    value = value.replace( '-'_L1, '+'_L1 ).replace( '_'_L1, '/'_L1 );
    while ( value.size() % 4 != 0 )
      value += '=';
    return QByteArray::fromBase64( value.toUtf8() );
  }

  QJsonObject postJsonBlocking( const QUrl &url, const QJsonObject &payload, int timeoutMs, int &httpStatus, QString *errorMessage )
  {
    httpStatus = 0;
    QgsNetworkAccessManager *nam = QgsNetworkAccessManager::instance();
    if ( !nam )
    {
      if ( errorMessage )
        *errorMessage = u"Network manager is not available."_s;
      return QJsonObject();
    }

    QNetworkRequest request( url );
    request.setHeader( QNetworkRequest::ContentTypeHeader, u"application/json"_s );
    request.setTransferTimeout( timeoutMs );

    QNetworkReply *reply = nam->post( request, QJsonDocument( payload ).toJson( QJsonDocument::Compact ) );
    if ( !reply )
    {
      if ( errorMessage )
        *errorMessage = u"Unable to start OAuth request."_s;
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
        QString detail;
        if ( object.value( u"error"_s ).isString() )
          detail = object.value( u"error"_s ).toString();
        else if ( object.value( u"error"_s ).isObject() )
          detail = object.value( u"error"_s ).toObject().value( u"message"_s ).toString();
        if ( detail.isEmpty() )
          detail = QString::fromUtf8( body.left( 500 ) );
        if ( detail.isEmpty() )
          detail = u"OAuth request failed."_s;
        *errorMessage = u"OAuth request failed (HTTP %1): %2"_s.arg( httpStatus ).arg( detail );
      }
      return QJsonObject();
    }

    return object;
  }

  QJsonObject postFormBlocking( const QUrl &url, const QUrlQuery &form, int timeoutMs, int &httpStatus, QString *errorMessage )
  {
    httpStatus = 0;
    QgsNetworkAccessManager *nam = QgsNetworkAccessManager::instance();
    if ( !nam )
    {
      if ( errorMessage )
        *errorMessage = u"Network manager is not available."_s;
      return QJsonObject();
    }

    QNetworkRequest request( url );
    request.setHeader( QNetworkRequest::ContentTypeHeader, u"application/x-www-form-urlencoded"_s );
    request.setTransferTimeout( timeoutMs );

    QNetworkReply *reply = nam->post( request, form.toString( QUrl::FullyEncoded ).toUtf8() );
    if ( !reply )
    {
      if ( errorMessage )
        *errorMessage = u"Unable to start OAuth token exchange."_s;
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
        if ( detail.isEmpty() )
          detail = object.value( u"error"_s ).toString();
        if ( detail.isEmpty() )
          detail = QString::fromUtf8( body.left( 500 ) );
        *errorMessage = u"OAuth token exchange failed (HTTP %1): %2"_s.arg( httpStatus ).arg( detail );
      }
      return QJsonObject();
    }

    return object;
  }

  bool storeCodexRefreshToken( const QString &refreshToken, QString * )
  {
    // Encrypted vault when usable, cleartext QgsSettings fallback otherwise.
    return QgsAiSecretStore::writeSecret( QgsAiCodexOAuthClient::refreshTokenSettingKey(), refreshToken.trimmed() );
  }

  QString storedCodexRefreshToken()
  {
    return QgsAiSecretStore::readSecret( QgsAiCodexOAuthClient::refreshTokenSettingKey() );
  }

  void sleepWithEvents( int seconds )
  {
    QEventLoop loop;
    QTimer::singleShot( std::max( 1, seconds ) * 1000, &loop, &QEventLoop::quit );
    loop.exec();
  }
} //namespace

bool QgsAiCodexOAuthClient::requestDeviceCode( DeviceCode &deviceCode, QString *errorMessage )
{
  QJsonObject payload;
  payload.insert( u"client_id"_s, QString::fromUtf8( CODEX_CLIENT_ID ) );

  int httpStatus = 0;
  const QJsonObject object = postJsonBlocking( QUrl( QString::fromUtf8( CODEX_DEVICE_API_BASE ) + u"/deviceauth/usercode"_s ), payload, 30000, httpStatus, errorMessage );
  if ( object.isEmpty() )
    return false;

  deviceCode.verificationUrl = QString::fromUtf8( CODEX_AUTH_ISSUER ) + u"/codex/device"_s;
  deviceCode.userCode = object.value( u"user_code"_s ).toString( object.value( u"usercode"_s ).toString() );
  deviceCode.deviceAuthId = object.value( u"device_auth_id"_s ).toString();
  bool ok = false;
  const int interval = object.value( u"interval"_s ).toString().toInt( &ok );
  deviceCode.intervalSeconds = ok ? interval : object.value( u"interval"_s ).toInt( 5 );

  if ( deviceCode.userCode.isEmpty() || deviceCode.deviceAuthId.isEmpty() )
  {
    if ( errorMessage )
      *errorMessage = u"Device authorization response is missing required fields."_s;
    return false;
  }

  return true;
}

bool QgsAiCodexOAuthClient::completeDeviceCodeLogin( const DeviceCode &deviceCode, QString *errorMessage )
{
  QJsonObject pollPayload;
  pollPayload.insert( u"device_auth_id"_s, deviceCode.deviceAuthId );
  pollPayload.insert( u"user_code"_s, deviceCode.userCode );

  QElapsedTimer timer;
  timer.start();
  QJsonObject codeObject;
  while ( timer.elapsed() < 15 * 60 * 1000 )
  {
    int httpStatus = 0;
    QString pollError;
    codeObject = postJsonBlocking( QUrl( QString::fromUtf8( CODEX_DEVICE_API_BASE ) + u"/deviceauth/token"_s ), pollPayload, 30000, httpStatus, &pollError );
    if ( !codeObject.isEmpty() )
      break;

    if ( httpStatus != 403 && httpStatus != 404 )
    {
      if ( errorMessage )
        *errorMessage = pollError;
      return false;
    }
    sleepWithEvents( deviceCode.intervalSeconds );
  }

  if ( codeObject.isEmpty() )
  {
    if ( errorMessage )
      *errorMessage = u"Device authorization timed out."_s;
    return false;
  }

  const QString authorizationCode = codeObject.value( u"authorization_code"_s ).toString();
  const QString codeVerifier = codeObject.value( u"code_verifier"_s ).toString();
  if ( authorizationCode.isEmpty() || codeVerifier.isEmpty() )
  {
    if ( errorMessage )
      *errorMessage = u"Codex device authorization response is missing the authorization code or PKCE verifier."_s;
    return false;
  }

  QUrlQuery form;
  form.addQueryItem( u"grant_type"_s, u"authorization_code"_s );
  form.addQueryItem( u"code"_s, authorizationCode );
  form.addQueryItem( u"redirect_uri"_s, QString::fromUtf8( CODEX_REDIRECT_URI ) );
  form.addQueryItem( u"client_id"_s, QString::fromUtf8( CODEX_CLIENT_ID ) );
  form.addQueryItem( u"code_verifier"_s, codeVerifier );

  int httpStatus = 0;
  const QJsonObject tokenObject = postFormBlocking( QUrl( QString::fromUtf8( CODEX_AUTH_ISSUER ) + u"/oauth/token"_s ), form, 30000, httpStatus, errorMessage );
  if ( tokenObject.isEmpty() )
    return false;

  const QString refreshToken = tokenObject.value( u"refresh_token"_s ).toString();
  if ( refreshToken.isEmpty() )
  {
    if ( errorMessage )
      *errorMessage = u"Codex token response did not include a refresh token."_s;
    return false;
  }

  return storeCodexRefreshToken( refreshToken, errorMessage );
}

bool QgsAiCodexOAuthClient::refreshAccessToken( TokenSet &tokens, QString *errorMessage )
{
  const QString refreshToken = storedCodexRefreshToken();
  if ( refreshToken.isEmpty() )
  {
    if ( errorMessage )
      *errorMessage = u"Missing Codex refresh token. Please sign in with Codex first."_s;
    return false;
  }

  QJsonObject payload;
  payload.insert( u"client_id"_s, QString::fromUtf8( CODEX_CLIENT_ID ) );
  payload.insert( u"grant_type"_s, u"refresh_token"_s );
  payload.insert( u"refresh_token"_s, refreshToken );

  int httpStatus = 0;
  const QJsonObject object = postJsonBlocking( QUrl( QString::fromUtf8( CODEX_AUTH_ISSUER ) + u"/oauth/token"_s ), payload, 30000, httpStatus, errorMessage );
  if ( object.isEmpty() )
    return false;

  tokens.accessToken = object.value( u"access_token"_s ).toString();
  tokens.refreshToken = object.value( u"refresh_token"_s ).toString();
  tokens.idToken = object.value( u"id_token"_s ).toString();
  tokens.chatGptAccountId = extractChatGptAccountId( tokens.idToken );

  if ( tokens.accessToken.isEmpty() || tokens.chatGptAccountId.isEmpty() )
  {
    if ( errorMessage )
      *errorMessage = u"Codex refresh response is missing access token or ChatGPT account id."_s;
    return false;
  }

  if ( !tokens.refreshToken.isEmpty() && tokens.refreshToken != refreshToken )
    return storeCodexRefreshToken( tokens.refreshToken, errorMessage );

  return true;
}

bool QgsAiCodexOAuthClient::hasRefreshToken()
{
  return QgsAiSecretStore::hasSecret( refreshTokenSettingKey() );
}

bool QgsAiCodexOAuthClient::clearRefreshToken( QString * )
{
  QgsAiSecretStore::removeSecret( refreshTokenSettingKey() );
  return true;
}

QString QgsAiCodexOAuthClient::extractChatGptAccountId( const QString &idToken )
{
  const QStringList parts = idToken.split( '.'_L1 );
  if ( parts.size() < 2 )
    return QString();

  const QJsonDocument doc = QJsonDocument::fromJson( base64UrlDecode( parts.at( 1 ) ) );
  if ( !doc.isObject() )
    return QString();

  const QJsonObject root = doc.object();
  const QJsonObject auth = root.value( u"https://api.openai.com/auth"_s ).toObject();
  return auth.value( u"chatgpt_account_id"_s ).toString();
}

QString QgsAiCodexOAuthClient::refreshTokenSettingKey()
{
  return u"ai/provider/codex/oauth/refreshToken"_s;
}
