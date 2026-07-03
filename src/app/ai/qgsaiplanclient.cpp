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

#include <algorithm>

#include "qgsapplication.h"
#include "qgsnetworkaccessmanager.h"

#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLocale>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QString>
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

  QString encodedPathSegment( const QString &segment )
  {
    return QString::fromLatin1( QUrl::toPercentEncoding( segment ) );
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

  QgsAiManagedAgentPreset parseAgentPresetObject( const QJsonObject &item )
  {
    QgsAiManagedAgentPreset preset;
    preset.mode = item.value( u"mode"_s ).toString();
    preset.label = item.value( u"label"_s ).toString( preset.mode );
    preset.allowedTools = stringArray( item.value( u"allowedTools"_s ) );
    preset.allowedModels = stringArray( item.value( u"allowedModels"_s ) );
    return preset;
  }

  QJsonObject agentPresetJson( const QgsAiManagedAgentPreset &preset )
  {
    QJsonObject item;
    item.insert( u"mode"_s, preset.mode );
    item.insert( u"label"_s, preset.label );
    item.insert( u"allowedTools"_s, QJsonArray::fromStringList( preset.allowedTools ) );
    item.insert( u"allowedModels"_s, QJsonArray::fromStringList( preset.allowedModels ) );
    return item;
  }

  void applyCachedPolicyFallback( QObject *receiver, const QgsAiManagedAgentPolicy &policy )
  {
    if ( !policy.isEmpty() )
      QMetaObject::invokeMethod(
        receiver,
        [receiver, policy]() {
          if ( QgsAiPlanClient *client = qobject_cast<QgsAiPlanClient *>( receiver ) )
            emit client->agentPolicyReady( policy, true );
        },
        Qt::QueuedConnection
      );
  }

  QString tokenCountLabel( int tokens )
  {
    return QLocale().toString( tokens );
  }
} //namespace

QString QgsAiPlanClient::ModelInfo::displayLabel() const
{
  QString text = label.isEmpty() ? id : label;
  QStringList details;
  if ( contextWindow > 0 )
    details << QObject::tr( "Context window: %1 tokens" ).arg( tokenCountLabel( contextWindow ) );
  if ( inputCredits > 0 || outputCredits > 0 )
    details << QObject::tr( "Cost: input %1 / output %2 credits per 1,000 tokens" ).arg( inputCredits ).arg( outputCredits );
  if ( !details.isEmpty() )
    text += u" - "_s + details.join( " - "_L1 );
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
    parts << QObject::tr( "Context window: %1 tokens. This is the maximum amount of text and context the model can consider at once." ).arg( tokenCountLabel( contextWindow ) );
  if ( inputCredits > 0 || outputCredits > 0 )
  {
    parts << QObject::tr( "Credit cost: %1 credits per 1,000 input tokens; %2 credits per 1,000 output tokens." ).arg( inputCredits ).arg( outputCredits );
    parts << QObject::tr( "Input tokens are prompts and project context sent to the model. Output tokens are the model response." );
  }
  if ( !capabilities.isEmpty() )
    parts << QObject::tr( "Capabilities: %1" ).arg( capabilities.join( ", "_L1 ) );
  if ( !tierAvailability.isEmpty() )
    parts << QObject::tr( "Tiers: %1" ).arg( tierAvailability.join( ", "_L1 ) );
  return parts.join( '\n' );
}

int QgsAiPlanClient::BalanceInfo::usedPercent() const
{
  if ( isUnlimited() )
    return 0;
  const int used = std::max( 0, monthlyCredits - available - held );
  return std::clamp( static_cast<int>( ( static_cast<qint64>( used ) * 100 ) / monthlyCredits ), 0, 100 );
}

QgsAiPlanClient::QgsAiPlanClient( QObject *parent )
  : QObject( parent )
{
  qRegisterMetaType<QgsAiPlanClient::AccountInfo>();
  qRegisterMetaType<QgsAiPlanClient::ModelInfo>();
  qRegisterMetaType<QList<QgsAiPlanClient::ModelInfo>>();
  qRegisterMetaType<QgsAiPlanClient::BalanceInfo>();
  qRegisterMetaType<QgsAiPlanClient::ModelPreferenceInfo>();
  qRegisterMetaType<QList<QgsAiPlanClient::ModelPreferenceInfo>>();
  qRegisterMetaType<QgsAiManagedAgentPreset>();
  qRegisterMetaType<QgsAiManagedAgentPolicy>();
  qRegisterMetaType<QList<QgsAiManagedAgentPreset>>();
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

QList<QgsAiManagedAgentPreset> QgsAiPlanClient::parseAgentsJson( const QByteArray &body )
{
  QList<QgsAiManagedAgentPreset> agents;
  const QJsonObject root = QJsonDocument::fromJson( body ).object();
  const QJsonArray items = root.value( u"items"_s ).toArray();
  agents.reserve( items.size() );
  for ( const QJsonValue &value : items )
  {
    QgsAiManagedAgentPreset preset = parseAgentPresetObject( value.toObject() );
    if ( !preset.mode.isEmpty() )
      agents << preset;
  }
  return agents;
}

QgsAiManagedAgentPolicy QgsAiPlanClient::parseAgentPolicyJson( const QByteArray &body )
{
  QgsAiManagedAgentPolicy policy;
  const QJsonObject root = QJsonDocument::fromJson( body ).object();
  policy.tier = root.value( u"tier"_s ).toString();
  policy.modes = stringArray( root.value( u"modes"_s ) );
  policy.allowedTools = stringArray( root.value( u"allowedTools"_s ) );
  policy.allowedModels = stringArray( root.value( u"allowedModels"_s ) );
  const QJsonArray presets = root.value( u"presets"_s ).toArray();
  policy.presets.reserve( presets.size() );
  for ( const QJsonValue &value : presets )
  {
    QgsAiManagedAgentPreset preset = parseAgentPresetObject( value.toObject() );
    if ( !preset.mode.isEmpty() )
      policy.presets << preset;
  }
  return policy;
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

QgsAiPlanClient::BalanceInfo QgsAiPlanClient::parseBalanceJson( const QByteArray &body )
{
  const QJsonObject root = QJsonDocument::fromJson( body ).object();
  BalanceInfo balance;
  balance.available = root.value( u"available"_s ).toInt();
  balance.held = root.value( u"held"_s ).toInt();
  balance.tier = root.value( u"tier"_s ).toString();
  balance.monthlyCredits = root.value( u"monthlyCredits"_s ).toInt();
  return balance;
}

QList<QgsAiPlanClient::ModelPreferenceInfo> QgsAiPlanClient::parseModelPreferencesJson( const QByteArray &body )
{
  QList<ModelPreferenceInfo> preferences;
  const QJsonObject root = QJsonDocument::fromJson( body ).object();
  const QJsonArray items = root.value( u"items"_s ).toArray();
  preferences.reserve( items.size() );
  for ( const QJsonValue &value : items )
  {
    const QJsonObject item = value.toObject();
    ModelPreferenceInfo info;
    info.modelId = item.value( u"modelId"_s ).toString();
    if ( info.modelId.isEmpty() )
      continue;
    info.enabled = item.value( u"enabled"_s ).toBool( true );
    preferences << info;
  }
  return preferences;
}

QString QgsAiPlanClient::cacheFilePath()
{
  return QDir( QgsApplication::qgisSettingsDirPath() ).filePath( u"strata_plan_models_cache.json"_s );
}

QString QgsAiPlanClient::agentsCacheFilePath()
{
  return QDir( QgsApplication::qgisSettingsDirPath() ).filePath( u"strata_plan_agents_cache.json"_s );
}

QString QgsAiPlanClient::agentPolicyCacheFilePath()
{
  return QDir( QgsApplication::qgisSettingsDirPath() ).filePath( u"strata_plan_agent_policy_cache.json"_s );
}

QString QgsAiPlanClient::modelPreferencesCacheFilePath()
{
  return QDir( QgsApplication::qgisSettingsDirPath() ).filePath( u"strata_plan_model_preferences_cache.json"_s );
}

QList<QgsAiPlanClient::ModelInfo> QgsAiPlanClient::cachedModels()
{
  QFile file( cacheFilePath() );
  if ( !file.open( QIODevice::ReadOnly ) )
    return {};
  return parseModelsJson( file.readAll() );
}

QList<QgsAiManagedAgentPreset> QgsAiPlanClient::cachedAgents()
{
  QFile file( agentsCacheFilePath() );
  if ( !file.open( QIODevice::ReadOnly ) )
    return {};
  return parseAgentsJson( file.readAll() );
}

QgsAiManagedAgentPolicy QgsAiPlanClient::cachedAgentPolicy()
{
  QFile file( agentPolicyCacheFilePath() );
  if ( !file.open( QIODevice::ReadOnly ) )
    return {};
  return parseAgentPolicyJson( file.readAll() );
}

QList<QgsAiPlanClient::ModelPreferenceInfo> QgsAiPlanClient::cachedModelPreferences()
{
  QFile file( modelPreferencesCacheFilePath() );
  if ( !file.open( QIODevice::ReadOnly ) )
    return {};
  return parseModelPreferencesJson( file.readAll() );
}

bool QgsAiPlanClient::isModelDisabled( const QString &modelId )
{
  const QList<ModelPreferenceInfo> preferences = cachedModelPreferences();
  for ( const ModelPreferenceInfo &preference : preferences )
  {
    if ( preference.modelId == modelId )
      return !preference.enabled;
  }
  return false;
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

void QgsAiPlanClient::writeCachedAgents( const QList<QgsAiManagedAgentPreset> &agents )
{
  QJsonArray items;
  for ( const QgsAiManagedAgentPreset &preset : agents )
    items << agentPresetJson( preset );
  QJsonObject root;
  root.insert( u"items"_s, items );

  QFile file( agentsCacheFilePath() );
  if ( file.open( QIODevice::WriteOnly | QIODevice::Truncate ) )
    file.write( QJsonDocument( root ).toJson( QJsonDocument::Compact ) );
}

void QgsAiPlanClient::writeCachedAgentPolicy( const QgsAiManagedAgentPolicy &policy )
{
  QJsonArray presets;
  for ( const QgsAiManagedAgentPreset &preset : policy.presets )
    presets << agentPresetJson( preset );

  QJsonObject root;
  root.insert( u"tier"_s, policy.tier );
  root.insert( u"modes"_s, QJsonArray::fromStringList( policy.modes ) );
  root.insert( u"allowedTools"_s, QJsonArray::fromStringList( policy.allowedTools ) );
  root.insert( u"allowedModels"_s, QJsonArray::fromStringList( policy.allowedModels ) );
  root.insert( u"presets"_s, presets );

  QFile file( agentPolicyCacheFilePath() );
  if ( file.open( QIODevice::WriteOnly | QIODevice::Truncate ) )
    file.write( QJsonDocument( root ).toJson( QJsonDocument::Compact ) );
}

void QgsAiPlanClient::writeCachedModelPreferences( const QList<ModelPreferenceInfo> &preferences )
{
  QJsonArray items;
  for ( const ModelPreferenceInfo &info : preferences )
  {
    QJsonObject item;
    item.insert( u"modelId"_s, info.modelId );
    item.insert( u"enabled"_s, info.enabled );
    items << item;
  }
  QJsonObject root;
  root.insert( u"items"_s, items );

  QFile file( modelPreferencesCacheFilePath() );
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

void QgsAiPlanClient::fetchBalance( const QString &chatEndpoint, const QString &sessionToken )
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

  QNetworkRequest request( apiUrl( apiBase, u"/v1/credits/balance"_s ) );
  request.setRawHeader( "Authorization", ( u"Bearer %1"_s.arg( sessionToken.trimmed() ) ).toUtf8() );
  request.setTransferTimeout( FETCH_TIMEOUT_MS );
  QNetworkReply *reply = networkManager->get( request );
  if ( !reply )
  {
    emit requestFailed( tr( "Unable to start the Plan balance request." ) );
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
    emit balanceReady( parseBalanceJson( body ) );
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

void QgsAiPlanClient::refreshAgents( const QString &chatEndpoint, const QString &sessionToken )
{
  refreshAuthenticatedJson( chatEndpoint, sessionToken, u"/v1/agents"_s );
}

void QgsAiPlanClient::refreshAgentPolicy( const QString &chatEndpoint, const QString &sessionToken )
{
  refreshAuthenticatedJson( chatEndpoint, sessionToken, u"/v1/agents/policy"_s );
}

void QgsAiPlanClient::refreshAuthenticatedJson( const QString &chatEndpoint, const QString &sessionToken, const QString &path )
{
  const QString apiBase = apiBaseForChatEndpoint( chatEndpoint );
  const bool wantsAgents = path.endsWith( "/agents"_L1 );
  if ( apiBase.isEmpty() || sessionToken.trimmed().isEmpty() )
  {
    if ( wantsAgents )
    {
      const QList<QgsAiManagedAgentPreset> cached = cachedAgents();
      if ( !cached.isEmpty() )
        emit agentsReady( cached, true );
    }
    else
      applyCachedPolicyFallback( this, cachedAgentPolicy() );
    if ( apiBase.isEmpty() )
      emit requestFailed( tr( "Plan backend endpoint is not configured." ) );
    return;
  }

  QgsNetworkAccessManager *networkManager = QgsNetworkAccessManager::instance();
  if ( !networkManager )
  {
    if ( wantsAgents )
    {
      const QList<QgsAiManagedAgentPreset> cached = cachedAgents();
      if ( !cached.isEmpty() )
        emit agentsReady( cached, true );
    }
    else
      applyCachedPolicyFallback( this, cachedAgentPolicy() );
    emit requestFailed( tr( "Network manager is not available." ) );
    return;
  }

  QNetworkRequest request( apiUrl( apiBase, path ) );
  request.setRawHeader( "Accept", "application/json" );
  request.setRawHeader( "Authorization", ( u"Bearer %1"_s.arg( sessionToken.trimmed() ) ).toUtf8() );
  request.setTransferTimeout( FETCH_TIMEOUT_MS );
  QNetworkReply *reply = networkManager->get( request );
  if ( !reply )
  {
    emit requestFailed( wantsAgents ? tr( "Unable to start the Plan agents request." ) : tr( "Unable to start the Plan agent policy request." ) );
    return;
  }

  connect( reply, &QNetworkReply::finished, reply, &QObject::deleteLater );
  connect( reply, &QNetworkReply::finished, this, [this, reply, wantsAgents]() {
    const int httpStatus = reply->attribute( QNetworkRequest::HttpStatusCodeAttribute ).toInt();
    const QByteArray body = reply->readAll();
    if ( reply->error() == QNetworkReply::NoError && httpStatus >= 200 && httpStatus < 300 )
    {
      if ( wantsAgents )
      {
        const QList<QgsAiManagedAgentPreset> agents = parseAgentsJson( body );
        if ( !agents.isEmpty() )
        {
          writeCachedAgents( agents );
          emit agentsReady( agents, false );
          return;
        }
      }
      else
      {
        const QgsAiManagedAgentPolicy policy = parseAgentPolicyJson( body );
        if ( !policy.isEmpty() )
        {
          writeCachedAgentPolicy( policy );
          emit agentPolicyReady( policy, false );
          return;
        }
      }
    }

    if ( wantsAgents )
    {
      const QList<QgsAiManagedAgentPreset> cached = cachedAgents();
      if ( !cached.isEmpty() )
        emit agentsReady( cached, true );
    }
    else
      applyCachedPolicyFallback( this, cachedAgentPolicy() );
    emit requestFailed( responseErrorMessage( reply, body ) );
  } );
}

void QgsAiPlanClient::fetchModelPreferences( const QString &chatEndpoint, const QString &sessionToken )
{
  const QString apiBase = apiBaseForChatEndpoint( chatEndpoint );
  if ( apiBase.isEmpty() || sessionToken.trimmed().isEmpty() )
  {
    const QList<ModelPreferenceInfo> cached = cachedModelPreferences();
    if ( !cached.isEmpty() )
      emit modelPreferencesReady( cached, true );
    if ( apiBase.isEmpty() )
      emit requestFailed( tr( "Plan backend endpoint is not configured." ) );
    return;
  }

  QgsNetworkAccessManager *networkManager = QgsNetworkAccessManager::instance();
  if ( !networkManager )
  {
    const QList<ModelPreferenceInfo> cached = cachedModelPreferences();
    if ( !cached.isEmpty() )
      emit modelPreferencesReady( cached, true );
    emit requestFailed( tr( "Network manager is not available." ) );
    return;
  }

  QNetworkRequest request( apiUrl( apiBase, u"/v1/models/preferences"_s ) );
  request.setRawHeader( "Accept", "application/json" );
  request.setRawHeader( "Authorization", ( u"Bearer %1"_s.arg( sessionToken.trimmed() ) ).toUtf8() );
  request.setTransferTimeout( FETCH_TIMEOUT_MS );
  QNetworkReply *reply = networkManager->get( request );
  if ( !reply )
  {
    emit requestFailed( tr( "Unable to start the Plan model preferences request." ) );
    return;
  }

  connect( reply, &QNetworkReply::finished, reply, &QObject::deleteLater );
  connect( reply, &QNetworkReply::finished, this, [this, reply]() {
    const int httpStatus = reply->attribute( QNetworkRequest::HttpStatusCodeAttribute ).toInt();
    const QByteArray body = reply->readAll();
    if ( reply->error() == QNetworkReply::NoError && httpStatus >= 200 && httpStatus < 300 )
    {
      const QList<ModelPreferenceInfo> preferences = parseModelPreferencesJson( body );
      writeCachedModelPreferences( preferences );
      emit modelPreferencesReady( preferences, false );
      return;
    }

    const QList<ModelPreferenceInfo> cached = cachedModelPreferences();
    if ( !cached.isEmpty() )
      emit modelPreferencesReady( cached, true );
    emit requestFailed( responseErrorMessage( reply, body ) );
  } );
}

void QgsAiPlanClient::setModelPreference( const QString &chatEndpoint, const QString &sessionToken, const QString &modelId, bool enabled )
{
  const QString apiBase = apiBaseForChatEndpoint( chatEndpoint );
  if ( apiBase.isEmpty() || sessionToken.trimmed().isEmpty() || modelId.isEmpty() )
  {
    emit modelPreferenceUpdateFailed( modelId, enabled, tr( "Plan endpoint or session token is missing." ) );
    return;
  }

  QgsNetworkAccessManager *networkManager = QgsNetworkAccessManager::instance();
  if ( !networkManager )
  {
    emit modelPreferenceUpdateFailed( modelId, enabled, tr( "Network manager is not available." ) );
    return;
  }

  QJsonObject body;
  body.insert( u"enabled"_s, enabled );

  QNetworkRequest request( apiUrl( apiBase, u"/v1/models/preferences/%1"_s.arg( encodedPathSegment( modelId ) ) ) );
  setJsonHeaders( request );
  request.setRawHeader( "Authorization", ( u"Bearer %1"_s.arg( sessionToken.trimmed() ) ).toUtf8() );
  QNetworkReply *reply = networkManager->sendCustomRequest( request, "PUT", QJsonDocument( body ).toJson( QJsonDocument::Compact ) );
  if ( !reply )
  {
    emit modelPreferenceUpdateFailed( modelId, enabled, tr( "Unable to start the Plan model preference update." ) );
    return;
  }

  connect( reply, &QNetworkReply::finished, reply, &QObject::deleteLater );
  connect( reply, &QNetworkReply::finished, this, [this, reply, modelId, enabled]() {
    const int httpStatus = reply->attribute( QNetworkRequest::HttpStatusCodeAttribute ).toInt();
    const QByteArray responseBody = reply->readAll();
    if ( reply->error() != QNetworkReply::NoError || httpStatus < 200 || httpStatus >= 300 )
    {
      emit modelPreferenceUpdateFailed( modelId, enabled, responseErrorMessage( reply, responseBody ) );
      return;
    }

    QList<ModelPreferenceInfo> cached = cachedModelPreferences();
    bool found = false;
    for ( ModelPreferenceInfo &info : cached )
    {
      if ( info.modelId == modelId )
      {
        info.enabled = enabled;
        found = true;
        break;
      }
    }
    if ( !found )
      cached << ModelPreferenceInfo { modelId, enabled };
    writeCachedModelPreferences( cached );

    emit modelPreferenceUpdated( modelId, enabled );
  } );
}
