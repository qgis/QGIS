#include "qgsaimodelrouter.h"

#include "qgsaitoolregistry.h"
#include "qgsapplication.h"
#include "qgsauthmanager.h"
#include "qgsmessagelog.h"
#include "qgsnetworkaccessmanager.h"
#include "qgssettings.h"

#include <QByteArray>
#include <QDateTime>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QRegularExpression>
#include <QStringList>
#include <QUrl>
#include <QUuid>

#include <algorithm>

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

QJsonArray QgsAiModelRouter::buildAnthropicAssistantContent( const QgsAiChatMessage &message ) const
{
  QJsonArray content;
  if ( !message.content.isEmpty() )
  {
    QJsonObject textBlock;
    textBlock.insert( QStringLiteral( "type" ), QStringLiteral( "text" ) );
    textBlock.insert( QStringLiteral( "text" ), message.content );
    content.push_back( textBlock );
  }

  const QVariantList toolCalls = message.metadata.value( QStringLiteral( "tool_calls" ) ).toList();
  for ( const QVariant &call : toolCalls )
  {
    const QVariantMap callMap = call.toMap();
    QJsonObject toolUse;
    toolUse.insert( QStringLiteral( "type" ), QStringLiteral( "tool_use" ) );
    toolUse.insert( QStringLiteral( "id" ), callMap.value( QStringLiteral( "id" ) ).toString() );
    toolUse.insert( QStringLiteral( "name" ), callMap.value( QStringLiteral( "name" ) ).toString() );
    toolUse.insert( QStringLiteral( "input" ), QJsonObject::fromVariantMap( callMap.value( QStringLiteral( "args" ) ).toMap() ) );
    content.push_back( toolUse );
  }
  return content;
}

QJsonArray QgsAiModelRouter::buildAnthropicUserContent( const QgsAiChatMessage &message ) const
{
  QJsonArray content;
  if ( message.role == QgsAiChatRole::Tool )
  {
    QJsonObject toolResult;
    toolResult.insert( QStringLiteral( "type" ), QStringLiteral( "tool_result" ) );
    toolResult.insert( QStringLiteral( "tool_use_id" ), message.metadata.value( QStringLiteral( "tool_call_id" ) ).toString() );
    toolResult.insert( QStringLiteral( "content" ), message.content );
    if ( message.metadata.value( QStringLiteral( "is_error" ) ).toBool() )
      toolResult.insert( QStringLiteral( "is_error" ), true );
    content.push_back( toolResult );
    return content;
  }

  QJsonObject textBlock;
  textBlock.insert( QStringLiteral( "type" ), QStringLiteral( "text" ) );
  textBlock.insert( QStringLiteral( "text" ), message.content );
  content.push_back( textBlock );
  return content;
}

void QgsAiModelRouter::appendOpenAiInputItems( const QgsAiChatMessage &message, QJsonArray &input ) const
{
  // Tool results are emitted as standalone function_call_output items, not as messages.
  if ( message.role == QgsAiChatRole::Tool )
  {
    QJsonObject item;
    item.insert( QStringLiteral( "type" ), QStringLiteral( "function_call_output" ) );
    item.insert( QStringLiteral( "call_id" ), message.metadata.value( QStringLiteral( "tool_call_id" ) ).toString() );
    item.insert( QStringLiteral( "output" ), message.content );
    input.push_back( item );
    return;
  }

  // For an assistant turn that contains tool calls, emit a message item (only if there is text)
  // followed by one function_call item per tool call. Order is preserved so the model sees
  // the same flow on the next turn.
  if ( message.role == QgsAiChatRole::Assistant )
  {
    if ( !message.content.isEmpty() )
    {
      QJsonObject item;
      item.insert( QStringLiteral( "type" ), QStringLiteral( "message" ) );
      item.insert( QStringLiteral( "role" ), QStringLiteral( "assistant" ) );
      QJsonArray content;
      QJsonObject textBlock;
      textBlock.insert( QStringLiteral( "type" ), QStringLiteral( "output_text" ) );
      textBlock.insert( QStringLiteral( "text" ), message.content );
      content.push_back( textBlock );
      item.insert( QStringLiteral( "content" ), content );
      input.push_back( item );
    }

    const QVariantList toolCalls = message.metadata.value( QStringLiteral( "tool_calls" ) ).toList();
    for ( const QVariant &call : toolCalls )
    {
      const QVariantMap callMap = call.toMap();
      QJsonObject item;
      item.insert( QStringLiteral( "type" ), QStringLiteral( "function_call" ) );
      item.insert( QStringLiteral( "call_id" ), callMap.value( QStringLiteral( "id" ) ).toString() );
      item.insert( QStringLiteral( "name" ), callMap.value( QStringLiteral( "name" ) ).toString() );
      const QJsonObject argsObj = QJsonObject::fromVariantMap( callMap.value( QStringLiteral( "args" ) ).toMap() );
      item.insert( QStringLiteral( "arguments" ), QString::fromUtf8( QJsonDocument( argsObj ).toJson( QJsonDocument::Compact ) ) );
      input.push_back( item );
    }
    return;
  }

  // user / system: standard message item.
  QJsonObject item;
  item.insert( QStringLiteral( "type" ), QStringLiteral( "message" ) );
  item.insert( QStringLiteral( "role" ), roleForProvider( Provider::OpenAi, message.role ) );
  QJsonArray content;
  QJsonObject textBlock;
  textBlock.insert( QStringLiteral( "type" ), QStringLiteral( "input_text" ) );
  textBlock.insert( QStringLiteral( "text" ), message.content );
  content.push_back( textBlock );
  item.insert( QStringLiteral( "content" ), content );
  input.push_back( item );
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
      appendOpenAiInputItems( message, input );
    payload.insert( QStringLiteral( "input" ), input );

    if ( mToolRegistry && mToolRegistry->count() > 0 )
    {
      const QJsonArray toolSchemas = mToolRegistry->schemasJsonForFormat( QgsAiToolRegistry::WireFormat::OpenAiResponses );
      if ( !toolSchemas.isEmpty() )
      {
        payload.insert( QStringLiteral( "tools" ), toolSchemas );
        payload.insert( QStringLiteral( "tool_choice" ), QStringLiteral( "auto" ) );
      }
    }
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
      const QString role = ( message.role == QgsAiChatRole::Assistant )
                             ? QStringLiteral( "assistant" )
                             : QStringLiteral( "user" );  // Tool results carry user role with tool_result blocks
      messageObject.insert( QStringLiteral( "role" ), role );

      QJsonArray content = ( message.role == QgsAiChatRole::Assistant )
                             ? buildAnthropicAssistantContent( message )
                             : buildAnthropicUserContent( message );
      messageObject.insert( QStringLiteral( "content" ), content );
      claudeMessages.push_back( messageObject );
    }

    if ( !systemPrompt.isEmpty() )
      payload.insert( QStringLiteral( "system" ), systemPrompt );
    payload.insert( QStringLiteral( "messages" ), claudeMessages );
    payload.insert( QStringLiteral( "max_tokens" ), 4096 );

    if ( mToolRegistry && mToolRegistry->count() > 0 )
    {
      const QJsonArray toolSchemas = mToolRegistry->schemasJsonForFormat( QgsAiToolRegistry::WireFormat::AnthropicTools );
      if ( !toolSchemas.isEmpty() )
        payload.insert( QStringLiteral( "tools" ), toolSchemas );
    }
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
    QgsMessageLog::logMessage(
      QStringLiteral( "No API key configured for %1; request will fail before dispatch." ).arg( providerDisplayName( provider ) ),
      QStringLiteral( "AI" ), Qgis::MessageLevel::Warning, false );
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

  if ( context.provider == Provider::Plan && !isUsablePlanEndpoint( settings.endpoint ) )
  {
    QgsMessageLog::logMessage(
      QStringLiteral( "Plan provider endpoint is a placeholder (%1); refusing to dispatch." ).arg( settings.endpoint ),
      QStringLiteral( "AI" ), Qgis::MessageLevel::Warning, false );
    return false;
  }

  QNetworkRequest request( QUrl( settings.endpoint ) );
  request.setHeader( QNetworkRequest::ContentTypeHeader, QStringLiteral( "application/json" ) );
  QgsSettings appSettings;
  const int configuredTimeoutSeconds = std::max( 10, appSettings.value( QStringLiteral( "ai/network/timeoutSeconds" ), 120 ).toInt() );
  request.setTransferTimeout( configuredTimeoutSeconds * 1000 );

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
  const QByteArray payload = buildRequestPayload( context.provider, context.messages, context.stream );
  QgsMessageLog::logMessage(
    QStringLiteral( "Dispatching request id=%1 provider=%2 endpoint=%3 model=%4 attempt=%5 payloadBytes=%6 stream=%7" )
      .arg( context.requestId, providerDisplayName( context.provider ), settings.endpoint, settings.model )
      .arg( context.attempt ).arg( payload.size() ).arg( context.stream ),
    QStringLiteral( "AI" ), Qgis::MessageLevel::Info, false );
  context.reply = networkManager->post( request, payload );
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
    {
      const QJsonObject delta = object.value( QStringLiteral( "delta" ) ).toObject();
      // text_delta carries assistant text; input_json_delta carries tool args (handled elsewhere).
      if ( delta.value( QStringLiteral( "type" ) ).toString() == QLatin1String( "text_delta" ) )
        return delta.value( QStringLiteral( "text" ) ).toString();
    }
    return QString();
  }

  // OpenAI Responses API streaming: text deltas come as response.output_text.delta
  const QString eventType = object.value( QStringLiteral( "type" ) ).toString();
  if ( eventType == QLatin1String( "response.output_text.delta" ) )
    return object.value( QStringLiteral( "delta" ) ).toString();

  // Fallback to old behaviour for legacy Chat Completions style chunks
  const QString directDelta = object.value( QStringLiteral( "delta" ) ).toString();
  if ( !directDelta.isEmpty() )
    return directDelta;

  const QJsonObject deltaObject = object.value( QStringLiteral( "delta" ) ).toObject();
  if ( !deltaObject.isEmpty() )
    return extractTextRecursive( deltaObject );

  return QString();
}

void QgsAiModelRouter::extractToolCallsFromResponse( Provider provider, const QJsonObject &object, RequestContext &context ) const
{
  if ( provider == Provider::Claude )
  {
    context.stopReason = object.value( QStringLiteral( "stop_reason" ) ).toString();
    const QJsonArray content = object.value( QStringLiteral( "content" ) ).toArray();
    for ( const QJsonValue &item : content )
    {
      const QJsonObject block = item.toObject();
      if ( block.value( QStringLiteral( "type" ) ).toString() != QLatin1String( "tool_use" ) )
        continue;

      PendingToolCall call;
      call.id = block.value( QStringLiteral( "id" ) ).toString();
      call.name = block.value( QStringLiteral( "name" ) ).toString();
      call.argumentsObject = block.value( QStringLiteral( "input" ) ).toObject();
      call.argumentsArePreParsed = true;
      context.toolCalls.append( call );
    }
    return;
  }

  if ( provider == Provider::OpenAi )
  {
    // Top-level "status" or per-output items don't carry stop_reason explicitly here;
    // we infer tool_use by the presence of function_call items.
    const QJsonArray output = object.value( QStringLiteral( "output" ) ).toArray();
    for ( const QJsonValue &item : output )
    {
      const QJsonObject obj = item.toObject();
      if ( obj.value( QStringLiteral( "type" ) ).toString() != QLatin1String( "function_call" ) )
        continue;

      PendingToolCall call;
      call.id = obj.value( QStringLiteral( "call_id" ) ).toString();
      call.name = obj.value( QStringLiteral( "name" ) ).toString();
      call.argumentsRaw = obj.value( QStringLiteral( "arguments" ) ).toString();
      const QJsonDocument doc = QJsonDocument::fromJson( call.argumentsRaw.toUtf8() );
      if ( doc.isObject() )
      {
        call.argumentsObject = doc.object();
        call.argumentsArePreParsed = true;
      }
      context.toolCalls.append( call );
    }

    if ( !context.toolCalls.isEmpty() )
      context.stopReason = QStringLiteral( "tool_use" );
  }
}

void QgsAiModelRouter::absorbStreamEvent( Provider provider, const QJsonObject &object, RequestContext &context )
{
  if ( provider == Provider::Claude )
  {
    const QString eventType = object.value( QStringLiteral( "type" ) ).toString();

    if ( eventType == QLatin1String( "content_block_start" ) )
    {
      const int blockIndex = object.value( QStringLiteral( "index" ) ).toInt();
      const QJsonObject block = object.value( QStringLiteral( "content_block" ) ).toObject();
      if ( block.value( QStringLiteral( "type" ) ).toString() == QLatin1String( "tool_use" ) )
      {
        PendingToolCall call;
        call.id = block.value( QStringLiteral( "id" ) ).toString();
        call.name = block.value( QStringLiteral( "name" ) ).toString();
        context.toolCalls.append( call );
        context.streamItemIndexToToolCall.insert( blockIndex, context.toolCalls.size() - 1 );
      }
    }
    else if ( eventType == QLatin1String( "content_block_delta" ) )
    {
      const QJsonObject delta = object.value( QStringLiteral( "delta" ) ).toObject();
      if ( delta.value( QStringLiteral( "type" ) ).toString() == QLatin1String( "input_json_delta" ) )
      {
        const int blockIndex = object.value( QStringLiteral( "index" ) ).toInt();
        const int toolIndex = context.streamItemIndexToToolCall.value( blockIndex, -1 );
        if ( toolIndex >= 0 && toolIndex < context.toolCalls.size() )
          context.toolCalls[toolIndex].argumentsRaw += delta.value( QStringLiteral( "partial_json" ) ).toString();
      }
    }
    else if ( eventType == QLatin1String( "content_block_stop" ) )
    {
      const int blockIndex = object.value( QStringLiteral( "index" ) ).toInt();
      const int toolIndex = context.streamItemIndexToToolCall.value( blockIndex, -1 );
      if ( toolIndex >= 0 && toolIndex < context.toolCalls.size() )
      {
        PendingToolCall &call = context.toolCalls[toolIndex];
        const QJsonDocument doc = QJsonDocument::fromJson( call.argumentsRaw.toUtf8() );
        if ( doc.isObject() )
        {
          call.argumentsObject = doc.object();
          call.argumentsArePreParsed = true;
        }
      }
    }
    else if ( eventType == QLatin1String( "message_delta" ) )
    {
      const QJsonObject delta = object.value( QStringLiteral( "delta" ) ).toObject();
      const QString reason = delta.value( QStringLiteral( "stop_reason" ) ).toString();
      if ( !reason.isEmpty() )
        context.stopReason = reason;
    }
    return;
  }

  if ( provider == Provider::OpenAi )
  {
    const QString eventType = object.value( QStringLiteral( "type" ) ).toString();

    if ( eventType == QLatin1String( "response.output_item.added" ) )
    {
      const int outputIndex = object.value( QStringLiteral( "output_index" ) ).toInt();
      const QJsonObject item = object.value( QStringLiteral( "item" ) ).toObject();
      if ( item.value( QStringLiteral( "type" ) ).toString() == QLatin1String( "function_call" ) )
      {
        PendingToolCall call;
        call.id = item.value( QStringLiteral( "call_id" ) ).toString();
        call.name = item.value( QStringLiteral( "name" ) ).toString();
        call.argumentsRaw = item.value( QStringLiteral( "arguments" ) ).toString();
        context.toolCalls.append( call );
        context.streamItemIndexToToolCall.insert( outputIndex, context.toolCalls.size() - 1 );
      }
    }
    else if ( eventType == QLatin1String( "response.function_call_arguments.delta" ) )
    {
      const int outputIndex = object.value( QStringLiteral( "output_index" ) ).toInt();
      const int toolIndex = context.streamItemIndexToToolCall.value( outputIndex, -1 );
      if ( toolIndex >= 0 && toolIndex < context.toolCalls.size() )
        context.toolCalls[toolIndex].argumentsRaw += object.value( QStringLiteral( "delta" ) ).toString();
    }
    else if ( eventType == QLatin1String( "response.function_call_arguments.done" ) )
    {
      const int outputIndex = object.value( QStringLiteral( "output_index" ) ).toInt();
      const int toolIndex = context.streamItemIndexToToolCall.value( outputIndex, -1 );
      if ( toolIndex >= 0 && toolIndex < context.toolCalls.size() )
      {
        PendingToolCall &call = context.toolCalls[toolIndex];
        const QString finalArgs = object.value( QStringLiteral( "arguments" ) ).toString();
        if ( !finalArgs.isEmpty() )
          call.argumentsRaw = finalArgs;
        const QJsonDocument doc = QJsonDocument::fromJson( call.argumentsRaw.toUtf8() );
        if ( doc.isObject() )
        {
          call.argumentsObject = doc.object();
          call.argumentsArePreParsed = true;
        }
      }
    }
    else if ( eventType == QLatin1String( "response.completed" ) )
    {
      // If we collected function_calls, stop_reason is conceptually "tool_use".
      if ( !context.toolCalls.isEmpty() )
        context.stopReason = QStringLiteral( "tool_use" );
      else
        context.stopReason = QStringLiteral( "stop" );
    }
  }
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

    const QJsonObject event = doc.object();

    // Update tool-call state regardless of whether the event also carries text.
    absorbStreamEvent( context->provider, event, *context );

    const QString chunk = extractTextFromStreamEvent( context->provider, event );
    if ( chunk.isEmpty() )
      continue;

    context->aggregatedText += chunk;
    emit requestProgress( context->requestId, chunk );
  }
}

QString QgsAiModelRouter::extractErrorMessageFromBody( Provider provider, const QByteArray &body ) const
{
  if ( body.isEmpty() )
    return QString();

  const QJsonDocument doc = QJsonDocument::fromJson( body );
  if ( !doc.isObject() )
    return QString();

  const QJsonObject root = doc.object();
  const QJsonValue errorValue = root.value( QStringLiteral( "error" ) );

  // OpenAI: { "error": { "message": "...", "type": "...", "code": "..." } }
  if ( errorValue.isObject() )
  {
    const QJsonObject errorObj = errorValue.toObject();
    const QString message = errorObj.value( QStringLiteral( "message" ) ).toString();
    const QString type = errorObj.value( QStringLiteral( "type" ) ).toString();
    const QString code = errorObj.value( QStringLiteral( "code" ) ).toString();
    if ( !message.isEmpty() )
    {
      QStringList parts;
      parts << message;
      if ( !type.isEmpty() )
        parts << QStringLiteral( "type=%1" ).arg( type );
      if ( !code.isEmpty() )
        parts << QStringLiteral( "code=%1" ).arg( code );
      return parts.join( QStringLiteral( " · " ) );
    }
  }
  else if ( errorValue.isString() )
  {
    return errorValue.toString();
  }

  // Anthropic: { "type": "error", "error": { "type": "invalid_request_error", "message": "..." } }
  if ( provider == Provider::Claude && root.value( QStringLiteral( "type" ) ).toString() == QLatin1String( "error" ) )
  {
    const QJsonObject errorObj = root.value( QStringLiteral( "error" ) ).toObject();
    const QString message = errorObj.value( QStringLiteral( "message" ) ).toString();
    const QString type = errorObj.value( QStringLiteral( "type" ) ).toString();
    if ( !message.isEmpty() )
      return type.isEmpty() ? message : QStringLiteral( "%1 · type=%2" ).arg( message, type );
  }

  // Plain string fallback
  const QString topLevelMessage = root.value( QStringLiteral( "message" ) ).toString();
  if ( !topLevelMessage.isEmpty() )
    return topLevelMessage;

  return QString();
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

  // Read body once: may contain success text or structured error.
  const QByteArray responseBody = reply->readAll();
  QString responseText = context->aggregatedText;
  if ( responseText.isEmpty() && !responseBody.isEmpty() )
  {
    const QJsonDocument doc = QJsonDocument::fromJson( responseBody );
    if ( doc.isObject() )
    {
      responseText = extractTextFromResponse( context->provider, doc.object() );
      // Non-streaming: also harvest tool calls from the body.
      if ( context->toolCalls.isEmpty() )
        extractToolCallsFromResponse( context->provider, doc.object(), *context );
    }
    else
      responseText = QString::fromUtf8( responseBody );
  }

  const bool success = networkError == QNetworkReply::NoError && ( httpStatus == 0 || ( httpStatus >= 200 && httpStatus < 300 ) );
  const bool wantsToolUse = success && !context->toolCalls.isEmpty();
  QgsMessageLog::logMessage(
    QStringLiteral( "Reply id=%1 provider=%2 httpStatus=%3 networkError=%4 latencyMs=%5 textLen=%6 bodyBytes=%7 success=%8 toolCalls=%9 stopReason=%10" )
      .arg( requestId, providerName ).arg( httpStatus ).arg( static_cast<int>( networkError ) ).arg( latencyMs ).arg( responseText.size() ).arg( responseBody.size() ).arg( success ).arg( context->toolCalls.size() ).arg( context->stopReason ),
    QStringLiteral( "AI" ),
    success ? Qgis::MessageLevel::Info : Qgis::MessageLevel::Warning,
    false );

  if ( wantsToolUse )
  {
    QList<QgsAiToolCall> publicCalls;
    publicCalls.reserve( context->toolCalls.size() );
    for ( const PendingToolCall &pending : context->toolCalls )
    {
      QgsAiToolCall call;
      call.id = pending.id;
      call.name = pending.name;
      call.args = pending.argumentsObject;
      publicCalls.append( call );
    }
    emit toolCallsRequested( requestId, providerName, responseText.trimmed(), publicCalls );
    reply->deleteLater();
    mRequests.remove( requestId );
    return;
  }

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

  QString errorMessage = extractErrorMessageFromBody( context->provider, responseBody );
  if ( errorMessage.isEmpty() )
    errorMessage = reply->errorString();
  if ( errorMessage.isEmpty() )
    errorMessage = responseText;
  errorMessage = sanitizeErrorText( errorMessage );

  emit requestFinished( requestId, false, providerName, QString(), errorMessage, httpStatus, context->attempt - 1, false, latencyMs );
  reply->deleteLater();
  mRequests.remove( requestId );
}

bool QgsAiModelRouter::isUsablePlanEndpoint( const QString &endpoint )
{
  const QString trimmed = endpoint.trimmed();
  if ( trimmed.isEmpty() )
    return false;
  return !trimmed.contains( QLatin1String( "example.invalid" ), Qt::CaseInsensitive );
}

QgsAiModelRouter::Provider QgsAiModelRouter::resolveProvider() const
{
  const ProviderSettings plan = mProviderSettings.value( Provider::Plan );
  if ( plan.enabled && isUsablePlanEndpoint( plan.endpoint ) && hasConfiguredCredential( Provider::Plan ) )
    return Provider::Plan;

  const ProviderSettings openAi = mProviderSettings.value( Provider::OpenAi );
  if ( openAi.enabled && hasConfiguredCredential( Provider::OpenAi ) )
    return Provider::OpenAi;

  const ProviderSettings claude = mProviderSettings.value( Provider::Claude );
  if ( claude.enabled && hasConfiguredCredential( Provider::Claude ) )
    return Provider::Claude;

  return Provider::OpenAi;
}
