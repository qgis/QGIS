/***************************************************************************
    qgsaiplanclient.cpp
    ---------------------
    begin                : July 2026
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

#include "qgsaiplanclient.h"

#include "qgsapplication.h"
#include "qgsnetworkaccessmanager.h"

#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QUrl>

#include "moc_qgsaiplanclient.cpp"

using namespace Qt::StringLiterals;

namespace
{
  constexpr int FETCH_TIMEOUT_MS = 20000;

  QUrl apiUrl( const QString &apiBase, const QString &path )
  {
    return QUrl( apiBase + path );
  }

  void setJsonHeaders( QNetworkRequest &request )
  {
    request.setHeader( QNetworkRequest::ContentTypeHeader, u"application/json"_s );
    request.setRawHeader( "Accept", "application/json" );
    request.setTransferTimeout( FETCH_TIMEOUT_MS );
  }

  QString responseErrorMessage( QNetworkReply *reply, const QByteArray &body )
  {
    const int httpStatus = reply ? reply->attribute( QNetworkRequest::HttpStatusCodeAttribute ).toInt() : 0;
    const QJsonObject root = QJsonDocument::fromJson( body ).object();
    const QJsonValue error = root.value( u"error"_s );
    QString message = root.value( u"message"_s ).toString();
    if ( message.isEmpty() && error.isObject() )
      message = error.toObject().value( u"message"_s ).toString();
    if ( message.isEmpty() && error.isString() )
      message = error.toString();
    if ( message.isEmpty() && reply )
      message = reply->errorString();
    return httpStatus > 0 ? QObject::tr( "Strata Cloud request failed (HTTP %1): %2" ).arg( httpStatus ).arg( message ) : message;
  }

  QStringList stringArray( const QJsonValue &value )
  {
    QStringList out;
    const QJsonArray array = value.toArray();
    out.reserve( array.size() );
    for ( const QJsonValue &item : array )
    {
      const QString text = item.toString().trimmed();
      if ( !text.isEmpty() )
        out << text;
    }
    return out;
  }
} //namespace

QString QgsAiPlanClient::ModelInfo::displayLabel() const
{
  QString text = label.isEmpty() ? id : label;
  if ( contextWindow > 0 )
    text += u" - %1k ctx"_s.arg( contextWindow / 1000 );
  if ( inputCredits > 0 || outputCredits > 0 )
    text += u" - %1/%2 cr"_s.arg( inputCredits ).arg( outputCredits );
  return text;
}

QString QgsAiPlanClient::ModelInfo::tooltip() const
{
  QStringList parts;
  if ( !id.isEmpty() )
    parts << QObject::tr( "Model: %1" ).arg( id );
  if ( !provider.isEmpty() )
    parts << QObject::tr( "Provider: %1" ).arg( provider );
  if ( contextWindow > 0 )
    parts << QObject::tr( "Context window: %1 tokens" ).arg( contextWindow );
  if ( inputCredits > 0 || outputCredits > 0 )
    parts << QObject::tr( "Credits: %1 input / %2 output" ).arg( inputCredits ).arg( outputCredits );
  if ( !capabilities.isEmpty() )
    parts << QObject::tr( "Capabilities: %1" ).arg( capabilities.join( u", "_s ) );
  if ( !tierAvailability.isEmpty() )
    parts << QObject::tr( "Tiers: %1" ).arg( tierAvailability.join( u", "_s ) );
  return parts.join( '\n' );
}

QgsAiPlanClient::QgsAiPlanClient( QObject *parent )
  : QObject( parent )
{
  qRegisterMetaType<QgsAiPlanClient::AccountInfo>();
  qRegisterMetaType<QgsAiPlanClient::ModelInfo>();
  qRegisterMetaType<QList<QgsAiPlanClient::ModelInfo>>();
}

QString QgsAiPlanClient::apiBaseForChatEndpoint( const QString &chatEndpoint )
{
  QUrl url( chatEndpoint.trimmed() );
  if ( !url.isValid() || url.scheme().isEmpty() || url.host().isEmpty() )
    return QString();

  url.setPath( QString() );
  url.setQuery( QString() );
  url.setFragment( QString() );
  QString base = url.toString();
  if ( base.endsWith( '/' ) )
    base.chop( 1 );
  return base;
}

QList<QgsAiPlanClient::ModelInfo> QgsAiPlanClient::parseModelsJson( const QByteArray &body )
{
  QList<ModelInfo> models;
  const QJsonObject root = QJsonDocument::fromJson( body ).object();
  const QJsonArray items = root.value( u"items"_s ).toArray();
  models.reserve( items.size() );
  for ( const QJsonValue &value : items )
  {
    const QJsonObject item = value.toObject();
    ModelInfo info;
    info.id = item.value( u"id"_s ).toString();
    if ( info.id.isEmpty() )
      continue;
    info.label = item.value( u"label"_s ).toString( info.id );
    info.provider = item.value( u"provider"_s ).toString();
    info.contextWindow = item.value( u"contextWindow"_s ).toInt();
    const QJsonObject price = item.value( u"priceInCredits"_s ).toObject();
    info.inputCredits = price.value( u"input"_s ).toInt();
    info.outputCredits = price.value( u"output"_s ).toInt();
    info.capabilities = stringArray( item.value( u"capabilities"_s ) );
    info.tierAvailability = stringArray( item.value( u"tierAvailability"_s ) );
    models << info;
  }
  return models;
}

QgsAiPlanClient::AccountInfo QgsAiPlanClient::parseMeJson( const QByteArray &body )
{
  const QJsonObject root = QJsonDocument::fromJson( body ).object();
  AccountInfo account;
  account.id = root.value( u"id"_s ).toString();
  account.email = root.value( u"email"_s ).toString();
  account.tier = root.value( u"tier"_s ).toString();
  return account;
}

QString QgsAiPlanClient::cacheFilePath()
{
  return QDir( QgsApplication::qgisSettingsDirPath() ).filePath( u"strata_plan_models_cache.json"_s );
}

QList<QgsAiPlanClient::ModelInfo> QgsAiPlanClient::cachedModels()
{
  QFile file( cacheFilePath() );
  if ( !file.open( QIODevice::ReadOnly ) )
    return {};
  return parseModelsJson( file.readAll() );
}

void QgsAiPlanClient::writeCachedModels( const QList<ModelInfo> &models )
{
  QJsonArray items;
  for ( const ModelInfo &info : models )
  {
    QJsonObject item;
    item.insert( u"id"_s, info.id );
    item.insert( u"label"_s, info.label );
    item.insert( u"provider"_s, info.provider );
    item.insert( u"contextWindow"_s, info.contextWindow );
    item.insert( u"capabilities"_s, QJsonArray::fromStringList( info.capabilities ) );
    item.insert( u"tierAvailability"_s, QJsonArray::fromStringList( info.tierAvailability ) );
    QJsonObject price;
    price.insert( u"input"_s, info.inputCredits );
    price.insert( u"output"_s, info.outputCredits );
    item.insert( u"priceInCredits"_s, price );
    items << item;
  }
  QJsonObject root;
  root.insert( u"items"_s, items );

  QFile file( cacheFilePath() );
  if ( file.open( QIODevice::WriteOnly | QIODevice::Truncate ) )
    file.write( QJsonDocument( root ).toJson( QJsonDocument::Compact ) );
}

void QgsAiPlanClient::login( const QString &chatEndpoint, const QString &email, const QString &password )
{
  authenticate( chatEndpoint, email, password, false );
}

void QgsAiPlanClient::registerAccount( const QString &chatEndpoint, const QString &email, const QString &password )
{
  authenticate( chatEndpoint, email, password, true );
}

void QgsAiPlanClient::authenticate( const QString &chatEndpoint, const QString &email, const QString &password, bool createAccount )
{
  const QString apiBase = apiBaseForChatEndpoint( chatEndpoint );
  if ( apiBase.isEmpty() )
  {
    emit requestFailed( tr( "Plan backend endpoint is not configured." ) );
    return;
  }
  if ( email.trimmed().isEmpty() || password.isEmpty() )
  {
    emit requestFailed( tr( "Email and password are required." ) );
    return;
  }

  QgsNetworkAccessManager *networkManager = QgsNetworkAccessManager::instance();
  if ( !networkManager )
  {
    emit requestFailed( tr( "Network manager is not available." ) );
    return;
  }

  QJsonObject body;
  body.insert( u"email"_s, email.trimmed() );
  body.insert( u"password"_s, password );

  QNetworkRequest request( apiUrl( apiBase, createAccount ? u"/v1/auth/register"_s : u"/v1/auth/login"_s ) );
  setJsonHeaders( request );
  QNetworkReply *reply = networkManager->post( request, QJsonDocument( body ).toJson( QJsonDocument::Compact ) );
  if ( !reply )
  {
    emit requestFailed( tr( "Unable to start the Plan login request." ) );
    return;
  }

  connect( reply, &QNetworkReply::finished, reply, &QObject::deleteLater );
  connect( reply, &QNetworkReply::finished, this, [this, reply, apiBase]() {
    const int httpStatus = reply->attribute( QNetworkRequest::HttpStatusCodeAttribute ).toInt();
    const QByteArray body = reply->readAll();
    if ( reply->error() != QNetworkReply::NoError || httpStatus < 200 || httpStatus >= 300 )
    {
      emit requestFailed( responseErrorMessage( reply, body ) );
      return;
    }

    const QString accessToken = QJsonDocument::fromJson( body ).object().value( u"accessToken"_s ).toString();
    if ( accessToken.isEmpty() )
    {
      emit requestFailed( tr( "Plan login response did not include an access token." ) );
      return;
    }
    requestDesktopToken( apiBase, accessToken );
  } );
}

void QgsAiPlanClient::requestDesktopToken( const QString &apiBase, const QString &accessToken )
{
  QgsNetworkAccessManager *networkManager = QgsNetworkAccessManager::instance();
  if ( !networkManager )
  {
    emit requestFailed( tr( "Network manager is not available." ) );
    return;
  }

  QJsonObject body;
  body.insert( u"name"_s, u"Strata Desktop"_s );
  QNetworkRequest request( apiUrl( apiBase, u"/v1/auth/token"_s ) );
  setJsonHeaders( request );
  request.setRawHeader( "Authorization", ( u"Bearer %1"_s.arg( accessToken ) ).toUtf8() );
  QNetworkReply *reply = networkManager->post( request, QJsonDocument( body ).toJson( QJsonDocument::Compact ) );
  if ( !reply )
  {
    emit requestFailed( tr( "Unable to mint the Plan desktop token." ) );
    return;
  }

  connect( reply, &QNetworkReply::finished, reply, &QObject::deleteLater );
  connect( reply, &QNetworkReply::finished, this, [this, reply]() {
    const int httpStatus = reply->attribute( QNetworkRequest::HttpStatusCodeAttribute ).toInt();
    const QByteArray body = reply->readAll();
    if ( reply->error() != QNetworkReply::NoError || httpStatus < 200 || httpStatus >= 300 )
    {
      emit requestFailed( responseErrorMessage( reply, body ) );
      return;
    }

    const QString token = QJsonDocument::fromJson( body ).object().value( u"token"_s ).toString();
    if ( token.isEmpty() )
    {
      emit requestFailed( tr( "Plan token response did not include a desktop token." ) );
      return;
    }
    emit desktopTokenReady( token );
  } );
}

void QgsAiPlanClient::fetchMe( const QString &chatEndpoint, const QString &sessionToken )
{
  const QString apiBase = apiBaseForChatEndpoint( chatEndpoint );
  if ( apiBase.isEmpty() || sessionToken.trimmed().isEmpty() )
  {
    emit requestFailed( tr( "Plan endpoint or session token is missing." ) );
    return;
  }

  QgsNetworkAccessManager *networkManager = QgsNetworkAccessManager::instance();
  if ( !networkManager )
  {
    emit requestFailed( tr( "Network manager is not available." ) );
    return;
  }

  QNetworkRequest request( apiUrl( apiBase, u"/v1/auth/me"_s ) );
  request.setRawHeader( "Authorization", ( u"Bearer %1"_s.arg( sessionToken.trimmed() ) ).toUtf8() );
  request.setTransferTimeout( FETCH_TIMEOUT_MS );
  QNetworkReply *reply = networkManager->get( request );
  if ( !reply )
  {
    emit requestFailed( tr( "Unable to start the Plan profile request." ) );
    return;
  }

  connect( reply, &QNetworkReply::finished, reply, &QObject::deleteLater );
  connect( reply, &QNetworkReply::finished, this, [this, reply]() {
    const int httpStatus = reply->attribute( QNetworkRequest::HttpStatusCodeAttribute ).toInt();
    const QByteArray body = reply->readAll();
    if ( reply->error() != QNetworkReply::NoError || httpStatus < 200 || httpStatus >= 300 )
    {
      emit requestFailed( responseErrorMessage( reply, body ) );
      return;
    }
    emit accountReady( parseMeJson( body ) );
  } );
}

void QgsAiPlanClient::refreshModels( const QString &chatEndpoint )
{
  const QString apiBase = apiBaseForChatEndpoint( chatEndpoint );
  if ( apiBase.isEmpty() )
  {
    const QList<ModelInfo> cached = cachedModels();
    if ( !cached.isEmpty() )
      emit modelsReady( cached, true );
    else
      emit requestFailed( tr( "Plan backend endpoint is not configured." ) );
    return;
  }

  QgsNetworkAccessManager *networkManager = QgsNetworkAccessManager::instance();
  if ( !networkManager )
  {
    const QList<ModelInfo> cached = cachedModels();
    if ( !cached.isEmpty() )
      emit modelsReady( cached, true );
    else
      emit requestFailed( tr( "Network manager is not available." ) );
    return;
  }

  QNetworkRequest request( apiUrl( apiBase, u"/v1/models"_s ) );
  request.setRawHeader( "Accept", "application/json" );
  request.setTransferTimeout( FETCH_TIMEOUT_MS );
  QNetworkReply *reply = networkManager->get( request );
  if ( !reply )
  {
    emit requestFailed( tr( "Unable to start the Plan model catalog request." ) );
    return;
  }

  connect( reply, &QNetworkReply::finished, reply, &QObject::deleteLater );
  connect( reply, &QNetworkReply::finished, this, [this, reply]() {
    const int httpStatus = reply->attribute( QNetworkRequest::HttpStatusCodeAttribute ).toInt();
    const QByteArray body = reply->readAll();
    if ( reply->error() == QNetworkReply::NoError && httpStatus >= 200 && httpStatus < 300 )
    {
      const QList<ModelInfo> models = parseModelsJson( body );
      if ( !models.isEmpty() )
      {
        writeCachedModels( models );
        emit modelsReady( models, false );
        return;
      }
    }

    const QList<ModelInfo> cached = cachedModels();
    if ( !cached.isEmpty() )
      emit modelsReady( cached, true );
    emit requestFailed( responseErrorMessage( reply, body ) );
  } );
}
