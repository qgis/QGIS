#include "qgsaimodelrouter.h"

#include "qgsapplication.h"
#include "qgsauthmanager.h"
#include "qgsnetworkaccessmanager.h"
#include "qgssettings.h"

#include <QDateTime>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QRegularExpression>
#include <QUuid>

namespace
{
  QString extractTextRecursive( const QJsonValue &value )
  {
    if ( value.isString() )
      return value.toString();

    if ( value.isArray() )
    {
      QString output;
      const QJsonArray array = value.toArray();
      for ( const QJsonValue &item : array )
      {
        const QString chunk = extractTextRecursive( item );
        if ( !chunk.isEmpty() )
          output += chunk;
      }
      return output;
    }

    if ( value.isObject() )
    {
      const QJsonObject object = value.toObject();
      QString output;
      const QStringList preferredKeys = { QStringLiteral( "output_text" ), QStringLiteral( "text" ), QStringLiteral( "delta" ) };
      for ( const QString &key : preferredKeys )
      {
        if ( object.contains( key ) )
        {
          const QString chunk = extractTextRecursive( object.value( key ) );
          if ( !chunk.isEmpty() )
            output += chunk;
        }
      }
      for ( auto it = object.constBegin(); it != object.constEnd(); ++it )
      {
        if ( preferredKeys.contains( it.key() ) )
          continue;
        const QString chunk = extractTextRecursive( it.value() );
        if ( !chunk.isEmpty() )
          output += chunk;
      }
      return output;
    }

    return QString();
  }
}

QgsAiModelRouter::QgsAiModelRouter( QObject *parent )
  : QObject( parent )
{
  ProviderSettings openAi;
  openAi.endpoint = QStringLiteral( "https://api.openai.com/v1/responses" );
  openAi.model = QStringLiteral( "gpt-4.1-mini" );
  mProviderSettings.insert( Provider::OpenAi, openAi );

  ProviderSettings claude;
  claude.endpoint = QStringLiteral( "https://api.anthropic.com/v1/messages" );
  claude.model = QStringLiteral( "claude-sonnet-4-20250514" );
  mProviderSettings.insert( Provider::Claude, claude );

  ProviderSettings plan;
  plan.endpoint = QStringLiteral( "https://example.invalid/ai/messages" );
  plan.model = QStringLiteral( "managed-plan" );
  mProviderSettings.insert( Provider::Plan, plan );

  loadPersistedProviderSettings();
}

QgsAiModelRouter::ProviderSettings QgsAiModelRouter::providerSettings( Provider provider ) const
{
  return mProviderSettings.value( provider );
}

void QgsAiModelRouter::setProviderSettings( Provider provider, const ProviderSettings &settings )
{
  mProviderSettings.insert( provider, settings );
  persistProviderSettings( provider, settings );
}

QString QgsAiModelRouter::generateRequestId()
{
  return QUuid::createUuid().toString( QUuid::WithoutBraces );
}

QString QgsAiModelRouter::roleForProvider( Provider provider, QgsAiChatRole role ) const
{
  Q_UNUSED( provider )
  switch ( role )
  {
    case QgsAiChatRole::Assistant:
      return QStringLiteral( "assistant" );
    case QgsAiChatRole::System:
      return QStringLiteral( "system" );
    case QgsAiChatRole::Tool:
      return QStringLiteral( "tool" );
    case QgsAiChatRole::User:
    default:
      return QStringLiteral( "user" );
  }
}

QString QgsAiModelRouter::providerDisplayName( Provider provider ) const
{
  switch ( provider )
  {
    case Provider::OpenAi:
      return QStringLiteral( "OpenAI" );
    case Provider::Claude:
      return QStringLiteral( "Claude" );
    case Provider::Plan:
      return QStringLiteral( "Plan Account" );
  }
  return QStringLiteral( "Unknown" );
}

QByteArray QgsAiModelRouter::buildRequestPayload( Provider provider, const QList<QgsAiChatMessage> &messages, bool stream ) const
{
  QJsonObject payload;
  const ProviderSettings settings = providerSettings( provider );
  payload.insert( QStringLiteral( "model" ), settings.model );
  payload.insert( QStringLiteral( "stream" ), stream );

  if ( provider == Provider::OpenAi )
  {
    QJsonArray input;
    for ( const QgsAiChatMessage &message : messages )
    {
      QJsonObject messageObject;
      messageObject.insert( QStringLiteral( "role" ), roleForProvider( provider, message.role ) );

      QJsonArray content;
      QJsonObject textObject;
      textObject.insert( QStringLiteral( "type" ), QStringLiteral( "input_text" ) );
      textObject.insert( QStringLiteral( "text" ), message.content );
      content.push_back( textObject );

      messageObject.insert( QStringLiteral( "content" ), content );
      input.push_back( messageObject );
    }
    payload.insert( QStringLiteral( "input" ), input );
  }
  else if ( provider == Provider::Claude )
  {
    QJsonArray claudeMessages;
    QString systemPrompt;

    for ( const QgsAiChatMessage &message : messages )
    {
      if ( message.role == QgsAiChatRole::System )
      {
        if ( !systemPrompt.isEmpty() )
          systemPrompt += '\n';
        systemPrompt += message.content;
        continue;
      }

      QJsonObject messageObject;
      messageObject.insert( QStringLiteral( "role" ), roleForProvider( provider, message.role ) );

      QJsonArray content;
      QJsonObject textObject;
      textObject.insert( QStringLiteral( "type" ), QStringLiteral( "text" ) );
      textObject.insert( QStringLiteral( "text" ), message.content );
      content.push_back( textObject );
      messageObject.insert( QStringLiteral( "content" ), content );
      claudeMessages.push_back( messageObject );
    }

    if ( !systemPrompt.isEmpty() )
      payload.insert( QStringLiteral( "system" ), systemPrompt );
    payload.insert( QStringLiteral( "messages" ), claudeMessages );
    payload.insert( QStringLiteral( "max_tokens" ), 2048 );
  }
  else
  {
    QJsonArray serializedMessages;
    for ( const QgsAiChatMessage &message : messages )
    {
      QJsonObject messageObject;
      messageObject.insert( QStringLiteral( "role" ), roleForProvider( provider, message.role ) );
      messageObject.insert( QStringLiteral( "content" ), message.content );
      serializedMessages.push_back( messageObject );
    }
    payload.insert( QStringLiteral( "messages" ), serializedMessages );
  }

  return QJsonDocument( payload ).toJson( QJsonDocument::Compact );
}

QString QgsAiModelRouter::sanitizeErrorText( const QString &errorText ) const
{
  QString sanitized = errorText;
  sanitized.replace( QRegularExpression( QStringLiteral( "Bearer\\s+[A-Za-z0-9_\\-\\.~\\+\\/]+=*" ), QRegularExpression::CaseInsensitiveOption ), QStringLiteral( "Bearer [REDACTED]" ) );
  sanitized.replace( QRegularExpression( QStringLiteral( "\\bsk-[A-Za-z0-9\\-_]+\\b" ) ), QStringLiteral( "[REDACTED_OPENAI_KEY]" ) );
  sanitized.replace( QRegularExpression( QStringLiteral( "\\b(x-api-key\\s*[:=]\\s*)([^\\s,;]+)" ), QRegularExpression::CaseInsensitiveOption ), QStringLiteral( "\\1[REDACTED]" ) );
  return sanitized;
}

QString QgsAiModelRouter::authHeaderName( Provider provider ) const
{
  switch ( provider )
  {
    case Provider::Claude:
      return QStringLiteral( "x-api-key" );
    case Provider::OpenAi:
    case Provider::Plan:
      return QStringLiteral( "Authorization" );
  }
  return QStringLiteral( "Authorization" );
}

QString QgsAiModelRouter::authHeaderValue( Provider provider, const QString &secret ) const
{
  if ( provider == Provider::OpenAi || provider == Provider::Plan )
    return QStringLiteral( "Bearer %1" ).arg( secret );
  return secret;
}

QString QgsAiModelRouter::authConfigSettingKey( Provider provider ) const
{
  switch ( provider )
  {
    case Provider::OpenAi:
      return QStringLiteral( "ai/provider/openai/authcfg" );
    case Provider::Claude:
      return QStringLiteral( "ai/provider/claude/authcfg" );
    case Provider::Plan:
      return QStringLiteral( "ai/provider/plan/token" );
  }
  return QString();
}

QString QgsAiModelRouter::providerSettingPrefix( Provider provider ) const
{
  switch ( provider )
  {
    case Provider::OpenAi:
      return QStringLiteral( "ai/provider/openai" );
    case Provider::Claude:
      return QStringLiteral( "ai/provider/claude" );
    case Provider::Plan:
      return QStringLiteral( "ai/provider/plan" );
  }
  return QStringLiteral( "ai/provider/unknown" );
}

QString QgsAiModelRouter::endpointSettingKey( Provider provider ) const
{
  return providerSettingPrefix( provider ) + QStringLiteral( "/endpoint" );
}

QString QgsAiModelRouter::modelSettingKey( Provider provider ) const
{
  return providerSettingPrefix( provider ) + QStringLiteral( "/model" );
}

QString QgsAiModelRouter::enabledSettingKey( Provider provider ) const
{
  return providerSettingPrefix( provider ) + QStringLiteral( "/enabled" );
}

QString QgsAiModelRouter::apiKeySettingKey( Provider provider ) const
{
  return providerSettingPrefix( provider ) + QStringLiteral( "/apiKey" );
}

QString QgsAiModelRouter::planAuthConfigIdSettingKey() const
{
  return QStringLiteral( "ai/provider/plan/authcfg" );
}

QString QgsAiModelRouter::planSessionTokenSettingKey() const
{
  return QStringLiteral( "ai/provider/plan/token" );
}

QString QgsAiModelRouter::storedApiKey( Provider provider ) const
{
  QgsSettings settings;
  const QString savedValue = settings.value( apiKeySettingKey( provider ) ).toString().trimmed();
  if ( !savedValue.isEmpty() )
    return savedValue;

  switch ( provider )
  {
    case Provider::OpenAi:
    {
      const QString envValue = qEnvironmentVariable( "OPENAI_API_KEY" ).trimmed();
      if ( !envValue.isEmpty() )
        return envValue;
      break;
    }
    case Provider::Claude:
    {
      const QString claudeValue = qEnvironmentVariable( "CLAUDE_API_KEY" ).trimmed();
      if ( !claudeValue.isEmpty() )
        return claudeValue;
      const QString anthropicValue = qEnvironmentVariable( "ANTHROPIC_API_KEY" ).trimmed();
      if ( !anthropicValue.isEmpty() )
        return anthropicValue;
      break;
    }
    case Provider::Plan:
      return QString();
  }
  return QString();
}

bool QgsAiModelRouter::hasStoredApiKey( Provider provider ) const
{
  if ( provider == Provider::Plan )
    return false;

  QgsSettings settings;
  return !settings.value( apiKeySettingKey( provider ) ).toString().trimmed().isEmpty();
}

bool QgsAiModelRouter::hasConfiguredCredential( Provider provider ) const
{
  if ( provider == Provider::Plan )
  {
    const ProviderSettings settings = mProviderSettings.value( provider );
    if ( !settings.authConfigId.trimmed().isEmpty() )
      return true;

    QgsAuthManager *authManager = QgsApplication::authManager();
    if ( !authManager )
      return false;

    return !authManager->authSetting( planSessionTokenSettingKey(), QVariant(), true ).toString().trimmed().isEmpty();
  }

  return !storedApiKey( provider ).isEmpty();
}

void QgsAiModelRouter::loadPersistedProviderSettings()
{
  QgsSettings settings;

  const QList<Provider> providers = { Provider::OpenAi, Provider::Claude, Provider::Plan };
  for ( Provider provider : providers )
  {
    ProviderSettings providerSettings = mProviderSettings.value( provider );

    const QString endpoint = settings.value( endpointSettingKey( provider ), providerSettings.endpoint ).toString().trimmed();
    if ( !endpoint.isEmpty() )
      providerSettings.endpoint = endpoint;

    const QString model = settings.value( modelSettingKey( provider ), providerSettings.model ).toString().trimmed();
    if ( !model.isEmpty() )
      providerSettings.model = model;

    if ( provider == Provider::Plan )
    {
      providerSettings.authConfigId = settings.value( planAuthConfigIdSettingKey(), providerSettings.authConfigId ).toString().trimmed();
      providerSettings.enabled = settings.value( enabledSettingKey( provider ), !providerSettings.authConfigId.isEmpty() ).toBool();
    }
    else
    {
      providerSettings.authConfigId.clear();
      providerSettings.enabled = settings.value( enabledSettingKey( provider ), !storedApiKey( provider ).isEmpty() ).toBool();
    }

    mProviderSettings.insert( provider, providerSettings );
  }
}

void QgsAiModelRouter::persistProviderSettings( Provider provider, const ProviderSettings &settings ) const
{
  QgsSettings appSettings;
  appSettings.setValue( endpointSettingKey( provider ), settings.endpoint.trimmed() );
  appSettings.setValue( modelSettingKey( provider ), settings.model.trimmed() );
  appSettings.setValue( enabledSettingKey( provider ), settings.enabled );

  if ( provider == Provider::Plan )
    appSettings.setValue( planAuthConfigIdSettingKey(), settings.authConfigId.trimmed() );
}

QString QgsAiModelRouter::startChatRequest( Provider provider, const QList<QgsAiChatMessage> &messages, bool stream )
{
  RequestContext context;
  context.requestId = generateRequestId();
  context.provider = provider;
  context.messages = messages;
  context.stream = stream;
  context.maxRetries = 1;
  mRequests.insert( context.requestId, context );

  RequestContext &storedContext = mRequests[context.requestId];
  if ( !dispatchRequest( storedContext ) )
  {
    emit requestFinished( storedContext.requestId, false, providerDisplayName( provider ), QString(), QStringLiteral( "Unable to start network request." ), 0, 0, false, 0 );
    mRequests.remove( storedContext.requestId );
  }

  return context.requestId;
}

bool QgsAiModelRouter::hasActiveRequest( const QString &requestId ) const
{
  return mRequests.contains( requestId );
}

void QgsAiModelRouter::cancelRequest( const QString &requestId )
{
  if ( !mRequests.contains( requestId ) )
    return;

  RequestContext &context = mRequests[requestId];
  if ( context.reply )
    context.reply->abort();
}

bool QgsAiModelRouter::storeApiKey( Provider provider, const QString &apiKey, QString *errorMessage )
{
  if ( provider == Provider::Plan )
  {
    if ( errorMessage )
      *errorMessage = QStringLiteral( "Use setPlanSessionToken for plan authentication." );
    return false;
  }

  if ( apiKey.trimmed().isEmpty() )
  {
    if ( errorMessage )
      *errorMessage = QStringLiteral( "API key is empty." );
    return false;
  }

  ProviderSettings settings = mProviderSettings.value( provider );
  QgsSettings appSettings;
  appSettings.setValue( apiKeySettingKey( provider ), apiKey.trimmed() );
  settings.authConfigId.clear();
  settings.enabled = true;
  setProviderSettings( provider, settings );
  return true;
}

bool QgsAiModelRouter::setPlanSessionToken( const QString &token, QString *errorMessage )
{
  if ( token.trimmed().isEmpty() )
  {
    if ( errorMessage )
      *errorMessage = QStringLiteral( "Plan session token is empty." );
    return false;
  }

  QgsAuthManager *authManager = QgsApplication::authManager();
  if ( !authManager )
  {
    if ( errorMessage )
      *errorMessage = QStringLiteral( "Authentication manager is unavailable." );
    return false;
  }

  if ( !authManager->storeAuthSetting( planSessionTokenSettingKey(), token.trimmed(), true ) )
  {
    if ( errorMessage )
      *errorMessage = QStringLiteral( "Unable to store plan session token securely." );
    return false;
  }

  ProviderSettings settings = mProviderSettings.value( Provider::Plan );
  settings.enabled = true;
  setProviderSettings( Provider::Plan, settings );
  return true;
}

void QgsAiModelRouter::setPlanAuthConfigId( const QString &authConfigId )
{
  ProviderSettings settings = mProviderSettings.value( Provider::Plan );
  settings.authConfigId = authConfigId.trimmed();
  settings.enabled = !settings.authConfigId.isEmpty() || settings.enabled;
  setProviderSettings( Provider::Plan, settings );
}

bool QgsAiModelRouter::applyAuthentication( Provider provider, QNetworkRequest &request, QString *errorMessage ) const
{
  const ProviderSettings settings = mProviderSettings.value( provider );
  if ( provider == Provider::Plan )
  {
    QgsAuthManager *authManager = QgsApplication::authManager();
    if ( !authManager )
    {
      if ( errorMessage )
        *errorMessage = QStringLiteral( "Authentication manager is unavailable." );
      return false;
    }

    if ( !settings.authConfigId.isEmpty() )
    {
      if ( !authManager->updateNetworkRequest( request, settings.authConfigId ) )
      {
        if ( errorMessage )
          *errorMessage = QStringLiteral( "Unable to apply plan OAuth authentication." );
        return false;
      }
      return true;
    }

    const QVariant token = authManager->authSetting( planSessionTokenSettingKey(), QVariant(), true );
    const QString tokenString = token.toString();
    if ( tokenString.isEmpty() )
    {
      if ( errorMessage )
        *errorMessage = QStringLiteral( "Missing plan session token. Please login first." );
      return false;
    }

    request.setRawHeader( "Authorization", authHeaderValue( Provider::Plan, tokenString ).toUtf8() );
    return true;
  }

  const QString apiKey = storedApiKey( provider );
  if ( apiKey.isEmpty() )
  {
    if ( errorMessage )
      *errorMessage = QStringLiteral( "No API key configured for %1." ).arg( providerDisplayName( provider ) );
    return false;
  }

  request.setRawHeader( authHeaderName( provider ).toUtf8(), authHeaderValue( provider, apiKey ).toUtf8() );
  return true;
}

QgsAiModelRouter::RequestContext *QgsAiModelRouter::contextFromReply( QNetworkReply *reply )
{
  if ( !reply )
    return nullptr;

  const QString requestId = reply->property( "aiRequestId" ).toString();
  if ( requestId.isEmpty() || !mRequests.contains( requestId ) )
    return nullptr;
  return &mRequests[requestId];
}

bool QgsAiModelRouter::dispatchRequest( RequestContext &context )
{
  const ProviderSettings settings = providerSettings( context.provider );
  if ( settings.endpoint.trimmed().isEmpty() )
    return false;

  QNetworkRequest request( QUrl( settings.endpoint ) );
  request.setHeader( QNetworkRequest::ContentTypeHeader, QStringLiteral( "application/json" ) );
  request.setTransferTimeout( 30000 );

  if ( context.provider == Provider::Claude )
    request.setRawHeader( "anthropic-version", "2023-06-01" );

  QString authError;
  if ( !applyAuthentication( context.provider, request, &authError ) )
    return false;

  QgsNetworkAccessManager *networkManager = QgsNetworkAccessManager::instance();
  if ( !networkManager )
    return false;

  context.startedAtMs = QDateTime::currentMSecsSinceEpoch();
  context.attempt++;
  context.streamingBuffer.clear();
  context.reply = networkManager->post( request, buildRequestPayload( context.provider, context.messages, context.stream ) );
  if ( !context.reply )
    return false;

  context.reply->setProperty( "aiRequestId", context.requestId );
  connect( context.reply, &QNetworkReply::readyRead, this, &QgsAiModelRouter::onReplyReadyRead );
  connect( context.reply, &QNetworkReply::finished, this, &QgsAiModelRouter::onReplyFinished );
  return true;
}

bool QgsAiModelRouter::shouldRetry( int httpStatus, QNetworkReply::NetworkError networkError, int attempt, int maxRetries ) const
{
  if ( attempt > maxRetries )
    return false;

  if ( networkError != QNetworkReply::NoError )
  {
    return networkError == QNetworkReply::TimeoutError
           || networkError == QNetworkReply::TemporaryNetworkFailureError
           || networkError == QNetworkReply::NetworkSessionFailedError;
  }

  return httpStatus == 429 || ( httpStatus >= 500 && httpStatus <= 599 );
}

QString QgsAiModelRouter::extractTextFromResponse( Provider provider, const QJsonObject &object ) const
{
  if ( provider == Provider::Claude )
  {
    const QJsonArray content = object.value( QStringLiteral( "content" ) ).toArray();
    QString text;
    for ( const QJsonValue &item : content )
    {
      const QJsonObject contentObject = item.toObject();
      if ( contentObject.value( QStringLiteral( "type" ) ).toString() == QLatin1String( "text" ) )
        text += contentObject.value( QStringLiteral( "text" ) ).toString();
    }
    if ( !text.isEmpty() )
      return text;
  }

  return extractTextRecursive( object );
}

QString QgsAiModelRouter::extractTextFromStreamEvent( Provider provider, const QJsonObject &object ) const
{
  if ( provider == Provider::Claude )
  {
    const QString eventType = object.value( QStringLiteral( "type" ) ).toString();
    if ( eventType == QLatin1String( "content_block_delta" ) )
      return object.value( QStringLiteral( "delta" ) ).toObject().value( QStringLiteral( "text" ) ).toString();
    return QString();
  }

  const QString directDelta = object.value( QStringLiteral( "delta" ) ).toString();
  if ( !directDelta.isEmpty() )
    return directDelta;

  const QJsonObject deltaObject = object.value( QStringLiteral( "delta" ) ).toObject();
  if ( !deltaObject.isEmpty() )
    return extractTextRecursive( deltaObject );

  return QString();
}

void QgsAiModelRouter::onReplyReadyRead()
{
  QNetworkReply *reply = qobject_cast<QNetworkReply *>( sender() );
  RequestContext *context = contextFromReply( reply );
  if ( !reply || !context )
    return;

  context->streamingBuffer += QString::fromUtf8( reply->readAll() );
  while ( true )
  {
    const int lineEnd = context->streamingBuffer.indexOf( '\n' );
    if ( lineEnd < 0 )
      break;

    const QString rawLine = context->streamingBuffer.left( lineEnd ).trimmed();
    context->streamingBuffer.remove( 0, lineEnd + 1 );
    if ( rawLine.isEmpty() )
      continue;

    QString dataLine = rawLine;
    if ( dataLine.startsWith( QLatin1String( "data:" ) ) )
      dataLine = dataLine.mid( 5 ).trimmed();
    if ( dataLine == QLatin1String( "[DONE]" ) )
      continue;

    const QJsonDocument doc = QJsonDocument::fromJson( dataLine.toUtf8() );
    if ( !doc.isObject() )
      continue;

    const QString chunk = extractTextFromStreamEvent( context->provider, doc.object() );
    if ( chunk.isEmpty() )
      continue;

    context->aggregatedText += chunk;
    emit requestProgress( context->requestId, chunk );
  }
}

void QgsAiModelRouter::onReplyFinished()
{
  QNetworkReply *reply = qobject_cast<QNetworkReply *>( sender() );
  RequestContext *context = contextFromReply( reply );
  if ( !reply || !context )
    return;

  const QString requestId = context->requestId;
  const QString providerName = providerDisplayName( context->provider );
  const int httpStatus = reply->attribute( QNetworkRequest::HttpStatusCodeAttribute ).toInt();
  const QNetworkReply::NetworkError networkError = reply->error();
  const qint64 latencyMs = std::max<qint64>( 0, QDateTime::currentMSecsSinceEpoch() - context->startedAtMs );

  QString responseText = context->aggregatedText;
  if ( responseText.isEmpty() )
  {
    const QByteArray responseBody = reply->readAll();
    const QJsonDocument doc = QJsonDocument::fromJson( responseBody );
    if ( doc.isObject() )
      responseText = extractTextFromResponse( context->provider, doc.object() );
    else
      responseText = QString::fromUtf8( responseBody );
  }

  const bool success = networkError == QNetworkReply::NoError && ( httpStatus == 0 || ( httpStatus >= 200 && httpStatus < 300 ) );
  if ( success )
  {
    emit requestFinished( requestId, true, providerName, responseText.trimmed(), QString(), httpStatus, context->attempt - 1, false, latencyMs );
    reply->deleteLater();
    mRequests.remove( requestId );
    return;
  }

  const bool retriable = shouldRetry( httpStatus, networkError, context->attempt, context->maxRetries );
  if ( retriable )
  {
    reply->deleteLater();
    context->reply = nullptr;
    if ( !dispatchRequest( *context ) )
    {
      emit requestFinished( requestId, false, providerName, QString(), sanitizeErrorText( QStringLiteral( "Retry dispatch failed." ) ), httpStatus, context->attempt - 1, true, latencyMs );
      mRequests.remove( requestId );
    }
    return;
  }

  QString errorMessage = reply->errorString();
  if ( errorMessage.isEmpty() )
    errorMessage = responseText;
  errorMessage = sanitizeErrorText( errorMessage );

  emit requestFinished( requestId, false, providerName, QString(), errorMessage, httpStatus, context->attempt - 1, false, latencyMs );
  reply->deleteLater();
  mRequests.remove( requestId );
}

QgsAiModelRouter::Provider QgsAiModelRouter::resolveProvider() const
{
  const ProviderSettings plan = mProviderSettings.value( Provider::Plan );
  if ( plan.enabled && hasConfiguredCredential( Provider::Plan ) )
    return Provider::Plan;

  const ProviderSettings openAi = mProviderSettings.value( Provider::OpenAi );
  if ( openAi.enabled && hasConfiguredCredential( Provider::OpenAi ) )
    return Provider::OpenAi;

  const ProviderSettings claude = mProviderSettings.value( Provider::Claude );
  if ( claude.enabled && hasConfiguredCredential( Provider::Claude ) )
    return Provider::Claude;

  return Provider::OpenAi;
}
