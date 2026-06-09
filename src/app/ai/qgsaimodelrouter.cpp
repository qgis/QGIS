/***************************************************************************
    qgsaimodelrouter.cpp
    ---------------------
    begin                : April 2026
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

#include "qgsaimodelrouter.h"

#include <algorithm>

#include "qgsaiclaudeoauthclient.h"
#include "qgsaicodexoauthclient.h"
#include "qgsaitoolregistry.h"
#include "qgsapplication.h"
#include "qgsauthmanager.h"
#include "qgsmessagelog.h"
#include "qgsnetworkaccessmanager.h"
#include "qgssettings.h"

#include <QByteArray>
#include <QDateTime>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QRegularExpression>
#include <QString>
#include <QStringList>
#include <QTimer>
#include <QUrl>
#include <QUuid>

#include "moc_qgsaimodelrouter.cpp"

using namespace Qt::StringLiterals;

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
      const QStringList preferredKeys = { u"output_text"_s, u"text"_s, u"delta"_s };
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

  bool aiFullToolLogDetails()
  {
    QgsSettings settings;
    if ( settings.contains( u"strata/log/full_tool_details"_s ) )
      return settings.value( u"strata/log/full_tool_details"_s, false ).toBool();
    return settings.value( u"geoai/log/full_tool_details"_s, false ).toBool();
  }

  QString endpointForLog( const QString &endpoint )
  {
    if ( aiFullToolLogDetails() )
      return endpoint;

    const QUrl url( endpoint );
    if ( url.isValid() && !url.host().isEmpty() )
      return url.host();
    return u"(custom endpoint)"_s;
  }

  bool readVisualContextImage( const QgsAiChatMessage &message, QString &mimeType, QString &base64Data )
  {
    QgsSettings settings;
    const bool hasConsent = settings.contains( u"strata/visual_context/image_send_consent"_s ) ? settings.value( u"strata/visual_context/image_send_consent"_s, false ).toBool()
                                                                                               : settings.value( u"geoai/visual_context/image_send_consent"_s, false ).toBool();
    if ( !hasConsent )
      return false;

    const QString imagePath = message.metadata.value( u"visual_context_image_path"_s ).toString();
    if ( imagePath.isEmpty() )
      return false;

    QFile file( imagePath );
    if ( !file.open( QIODevice::ReadOnly ) )
      return false;

    const QByteArray bytes = file.readAll();
    if ( bytes.isEmpty() )
      return false;

    mimeType = message.metadata.value( u"visual_context_mime_type"_s, u"image/png"_s ).toString();
    if ( mimeType.isEmpty() )
      mimeType = u"image/png"_s;
    base64Data = QString::fromLatin1( bytes.toBase64() );
    return true;
  }
} //namespace

QgsAiModelRouter::QgsAiModelRouter( QObject *parent )
  : QObject( parent )
{
  ProviderSettings openAi;
  openAi.endpoint = u"https://api.openai.com/v1/responses"_s;
  openAi.model = u"gpt-4.1-mini"_s;
  openAi.credentialMode = CredentialMode::ApiKey;
  mProviderSettings.insert( Provider::OpenAi, openAi );

  ProviderSettings codex;
  codex.endpoint = u"https://chatgpt.com/backend-api/codex/responses"_s;
  codex.model = u"gpt-5.4"_s;
  codex.credentialMode = CredentialMode::OAuth;
  mProviderSettings.insert( Provider::Codex, codex );

  ProviderSettings claude;
  claude.endpoint = u"https://api.anthropic.com/v1/messages"_s;
  claude.model = u"claude-sonnet-4-20250514"_s;
  claude.credentialMode = CredentialMode::ApiKey;
  mProviderSettings.insert( Provider::Claude, claude );

  ProviderSettings openRouter;
  openRouter.endpoint = u"https://openrouter.ai/api/v1/responses"_s;
  openRouter.model = u"openrouter/auto"_s;
  openRouter.credentialMode = CredentialMode::ApiKey;
  openRouter.autoRouting = true;
  mProviderSettings.insert( Provider::OpenRouter, openRouter );

  ProviderSettings plan;
  plan.endpoint = u"https://example.invalid/ai/messages"_s;
  plan.model = u"managed-plan"_s;
  mProviderSettings.insert( Provider::Plan, plan );

  loadPersistedProviderSettings();
}

QgsAiModelRouter::ProviderSettings QgsAiModelRouter::providerSettings( Provider provider ) const
{
  return mProviderSettings.value( provider );
}

void QgsAiModelRouter::setAllowedTools( const QStringList &toolNames )
{
  mAllowedToolsFilterEnabled = true;
  mAllowedTools = toolNames;
  mAllowedTools.removeDuplicates();
}

void QgsAiModelRouter::clearAllowedToolsFilter()
{
  mAllowedToolsFilterEnabled = false;
  mAllowedTools.clear();
}

void QgsAiModelRouter::setProviderSettings( Provider provider, const ProviderSettings &settings )
{
  ProviderSettings normalizedSettings = settings;
  normalizedSettings.model = normalizedModelForProvider( provider, normalizedSettings.model );
  if ( provider == Provider::Codex )
    normalizedSettings.credentialMode = CredentialMode::OAuth;
  mProviderSettings.insert( provider, normalizedSettings );
  persistProviderSettings( provider, normalizedSettings );
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
      return u"assistant"_s;
    case QgsAiChatRole::System:
      return u"system"_s;
    case QgsAiChatRole::Tool:
      return u"tool"_s;
    case QgsAiChatRole::User:
    default:
      return u"user"_s;
  }
}

QString QgsAiModelRouter::providerDisplayName( Provider provider ) const
{
  switch ( provider )
  {
    case Provider::OpenAi:
      return u"OpenAI"_s;
    case Provider::Codex:
      return u"Codex"_s;
    case Provider::Claude:
      return u"Claude"_s;
    case Provider::OpenRouter:
      return u"OpenRouter"_s;
    case Provider::Plan:
      return u"Plan Account"_s;
  }
  return u"Unknown"_s;
}

QString QgsAiModelRouter::normalizedModelForProvider( Provider provider, const QString &model ) const
{
  const QString trimmed = model.trimmed();
  if ( provider == Provider::OpenRouter )
    return trimmed.isEmpty() ? u"openrouter/auto"_s : trimmed;

  if ( provider != Provider::Codex )
    return trimmed;

  // Slugs the chatgpt.com Codex backend currently accepts for ChatGPT-auth users.
  // The backend rejects unknown slugs (e.g. gpt-5.5) with HTTP 400.
  const QStringList allowedCodexModels = { u"gpt-5.4"_s, u"gpt-5.4-mini"_s, u"gpt-5.3-codex"_s, u"gpt-5.2"_s, u"gpt-5-codex"_s };
  if ( allowedCodexModels.contains( trimmed ) )
    return trimmed;

  return u"gpt-5.4"_s;
}

QJsonArray QgsAiModelRouter::buildAnthropicAssistantContent( const QgsAiChatMessage &message ) const
{
  QJsonArray content;
  if ( !message.content.isEmpty() )
  {
    QJsonObject textBlock;
    textBlock.insert( u"type"_s, u"text"_s );
    textBlock.insert( u"text"_s, message.content );
    content.push_back( textBlock );
  }

  const QVariantList toolCalls = message.metadata.value( u"tool_calls"_s ).toList();
  for ( const QVariant &call : toolCalls )
  {
    const QVariantMap callMap = call.toMap();
    QJsonObject toolUse;
    toolUse.insert( u"type"_s, u"tool_use"_s );
    toolUse.insert( u"id"_s, callMap.value( u"id"_s ).toString() );
    toolUse.insert( u"name"_s, callMap.value( u"name"_s ).toString() );
    toolUse.insert( u"input"_s, QJsonObject::fromVariantMap( callMap.value( u"args"_s ).toMap() ) );
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
    toolResult.insert( u"type"_s, u"tool_result"_s );
    toolResult.insert( u"tool_use_id"_s, message.metadata.value( u"tool_call_id"_s ).toString() );

    QString mimeType;
    QString base64Data;
    if ( readVisualContextImage( message, mimeType, base64Data ) )
    {
      QJsonArray resultContent;

      QJsonObject textBlock;
      textBlock.insert( u"type"_s, u"text"_s );
      textBlock.insert( u"text"_s, message.content );
      resultContent.push_back( textBlock );

      QJsonObject source;
      source.insert( u"type"_s, u"base64"_s );
      source.insert( u"media_type"_s, mimeType );
      source.insert( u"data"_s, base64Data );

      QJsonObject imageBlock;
      imageBlock.insert( u"type"_s, u"image"_s );
      imageBlock.insert( u"source"_s, source );
      resultContent.push_back( imageBlock );

      toolResult.insert( u"content"_s, resultContent );
    }
    else
    {
      toolResult.insert( u"content"_s, message.content );
    }

    if ( message.metadata.value( u"is_error"_s ).toBool() )
      toolResult.insert( u"is_error"_s, true );
    content.push_back( toolResult );
    return content;
  }

  QJsonObject textBlock;
  textBlock.insert( u"type"_s, u"text"_s );
  textBlock.insert( u"text"_s, message.content );
  content.push_back( textBlock );
  return content;
}

void QgsAiModelRouter::appendOpenAiInputItems( Provider provider, const QgsAiChatMessage &message, QJsonArray &input ) const
{
  // Tool results are emitted as standalone function_call_output items, not as messages.
  if ( message.role == QgsAiChatRole::Tool )
  {
    QJsonObject item;
    item.insert( u"type"_s, u"function_call_output"_s );
    item.insert( u"call_id"_s, message.metadata.value( u"tool_call_id"_s ).toString() );
    item.insert( u"output"_s, message.content );
    input.push_back( item );

    if ( provider == Provider::OpenAi || provider == Provider::Codex || provider == Provider::OpenRouter )
    {
      QString mimeType;
      QString base64Data;
      if ( readVisualContextImage( message, mimeType, base64Data ) )
      {
        QJsonObject imageMessage;
        imageMessage.insert( u"type"_s, u"message"_s );
        imageMessage.insert( u"role"_s, u"user"_s );

        QJsonArray content;
        QJsonObject textBlock;
        textBlock.insert( u"type"_s, u"input_text"_s );
        textBlock.insert( u"text"_s, u"Visual context image captured from the QGIS map canvas for the preceding tool result."_s );
        content.push_back( textBlock );

        QJsonObject imageBlock;
        imageBlock.insert( u"type"_s, u"input_image"_s );
        imageBlock.insert( u"image_url"_s, u"data:%1;base64,%2"_s.arg( mimeType, base64Data ) );
        content.push_back( imageBlock );

        imageMessage.insert( u"content"_s, content );
        input.push_back( imageMessage );
      }
    }
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
      item.insert( u"type"_s, u"message"_s );
      item.insert( u"role"_s, u"assistant"_s );
      QJsonArray content;
      QJsonObject textBlock;
      textBlock.insert( u"type"_s, u"output_text"_s );
      textBlock.insert( u"text"_s, message.content );
      content.push_back( textBlock );
      item.insert( u"content"_s, content );
      input.push_back( item );
    }

    const QVariantList toolCalls = message.metadata.value( u"tool_calls"_s ).toList();
    for ( const QVariant &call : toolCalls )
    {
      const QVariantMap callMap = call.toMap();
      QJsonObject item;
      item.insert( u"type"_s, u"function_call"_s );
      item.insert( u"call_id"_s, callMap.value( u"id"_s ).toString() );
      item.insert( u"name"_s, callMap.value( u"name"_s ).toString() );
      const QJsonObject argsObj = QJsonObject::fromVariantMap( callMap.value( u"args"_s ).toMap() );
      item.insert( u"arguments"_s, QString::fromUtf8( QJsonDocument( argsObj ).toJson( QJsonDocument::Compact ) ) );
      input.push_back( item );
    }
    return;
  }

  // user / system: standard message item.
  QJsonObject item;
  item.insert( u"type"_s, u"message"_s );
  item.insert( u"role"_s, roleForProvider( Provider::OpenAi, message.role ) );
  QJsonArray content;
  QJsonObject textBlock;
  textBlock.insert( u"type"_s, u"input_text"_s );
  textBlock.insert( u"text"_s, message.content );
  content.push_back( textBlock );
  item.insert( u"content"_s, content );
  input.push_back( item );
}

QByteArray QgsAiModelRouter::buildRequestPayload( Provider provider, const QList<QgsAiChatMessage> &messages, bool stream ) const
{
  QJsonObject payload;
  const ProviderSettings settings = providerSettings( provider );
  payload.insert( u"model"_s, normalizedModelForProvider( provider, settings.model ) );
  payload.insert( u"stream"_s, stream );

  if ( provider == Provider::OpenAi || provider == Provider::Codex || provider == Provider::OpenRouter )
  {
    QJsonArray input;
    QString codexInstructions;
    for ( const QgsAiChatMessage &message : messages )
    {
      // The Codex backend expects system prompts in a top-level `instructions`
      // field — system items inside `input` are rejected.
      if ( provider == Provider::Codex && message.role == QgsAiChatRole::System )
      {
        if ( !codexInstructions.isEmpty() )
          codexInstructions += '\n';
        codexInstructions += message.content;
        continue;
      }
      appendOpenAiInputItems( provider, message, input );
    }
    payload.insert( u"input"_s, input );

    QJsonArray toolSchemas;
    if ( mToolUseEnabled && mToolRegistry && mToolRegistry->count() > 0 )
    {
      if ( !mAllowedToolsFilterEnabled )
        toolSchemas = mToolRegistry->schemasJsonForFormat( QgsAiToolRegistry::WireFormat::OpenAiResponses );
      else if ( !mAllowedTools.isEmpty() )
        toolSchemas = mToolRegistry->schemasJsonForFormat( QgsAiToolRegistry::WireFormat::OpenAiResponses, mAllowedTools );
    }

    if ( provider == Provider::Codex )
    {
      // The chatgpt.com Codex backend mirrors the schema codex_cli_rs sends and
      // accepts tool fields only when tool schemas are intentionally advertised.
      if ( !codexInstructions.isEmpty() )
        payload.insert( u"instructions"_s, codexInstructions );
      if ( !toolSchemas.isEmpty() )
      {
        payload.insert( u"tools"_s, toolSchemas );
        payload.insert( u"tool_choice"_s, u"auto"_s );
        payload.insert( u"parallel_tool_calls"_s, false );
      }
      payload.insert( u"reasoning"_s, QJsonValue() );
      payload.insert( u"store"_s, false );
      payload.insert( u"include"_s, QJsonArray() );
      if ( mCodexPromptCacheKey.isEmpty() )
        mCodexPromptCacheKey = QUuid::createUuid().toString( QUuid::WithoutBraces );
      payload.insert( u"prompt_cache_key"_s, mCodexPromptCacheKey );
    }
    else if ( provider == Provider::OpenRouter )
    {
      if ( !toolSchemas.isEmpty() )
      {
        payload.insert( u"tools"_s, toolSchemas );
        payload.insert( u"tool_choice"_s, u"auto"_s );
      }

      if ( settings.autoRouting )
        payload.insert( u"provider"_s, openRouterProviderPreferences() );
    }
    else if ( !toolSchemas.isEmpty() )
    {
      payload.insert( u"tools"_s, toolSchemas );
      payload.insert( u"tool_choice"_s, u"auto"_s );
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
      const QString role = ( message.role == QgsAiChatRole::Assistant ) ? u"assistant"_s : u"user"_s; // Tool results carry user role with tool_result blocks
      messageObject.insert( u"role"_s, role );

      QJsonArray content = ( message.role == QgsAiChatRole::Assistant ) ? buildAnthropicAssistantContent( message ) : buildAnthropicUserContent( message );
      messageObject.insert( u"content"_s, content );
      claudeMessages.push_back( messageObject );
    }

    if ( !systemPrompt.isEmpty() )
      payload.insert( u"system"_s, systemPrompt );
    payload.insert( u"messages"_s, claudeMessages );
    payload.insert( u"max_tokens"_s, 4096 );

    if ( mToolUseEnabled && mToolRegistry && mToolRegistry->count() > 0 )
    {
      QJsonArray toolSchemas;
      if ( !mAllowedToolsFilterEnabled )
        toolSchemas = mToolRegistry->schemasJsonForFormat( QgsAiToolRegistry::WireFormat::AnthropicTools );
      else if ( !mAllowedTools.isEmpty() )
        toolSchemas = mToolRegistry->schemasJsonForFormat( QgsAiToolRegistry::WireFormat::AnthropicTools, mAllowedTools );
      if ( !toolSchemas.isEmpty() )
        payload.insert( u"tools"_s, toolSchemas );
    }
  }
  else
  {
    QJsonArray serializedMessages;
    for ( const QgsAiChatMessage &message : messages )
    {
      QJsonObject messageObject;
      messageObject.insert( u"role"_s, roleForProvider( provider, message.role ) );
      messageObject.insert( u"content"_s, message.content );
      serializedMessages.push_back( messageObject );
    }
    payload.insert( u"messages"_s, serializedMessages );
  }

  return QJsonDocument( payload ).toJson( QJsonDocument::Compact );
}

QString QgsAiModelRouter::sanitizeErrorText( const QString &errorText ) const
{
  QString sanitized = errorText;
  sanitized.replace( QRegularExpression( u"Bearer\\s+[A-Za-z0-9_\\-\\.~\\+\\/]+=*"_s, QRegularExpression::CaseInsensitiveOption ), u"Bearer [REDACTED]"_s );
  sanitized.replace( QRegularExpression( u"\\bsk-or-[A-Za-z0-9\\-_]+\\b"_s ), u"[REDACTED_OPENROUTER_KEY]"_s );
  sanitized.replace( QRegularExpression( u"\\bsk-[A-Za-z0-9\\-_]+\\b"_s ), u"[REDACTED_OPENAI_KEY]"_s );
  sanitized.replace( QRegularExpression( u"\\b(x-api-key\\s*[:=]\\s*)([^\\s,;]+)"_s, QRegularExpression::CaseInsensitiveOption ), u"\\1[REDACTED]"_s );
  return sanitized;
}

QJsonObject QgsAiModelRouter::openRouterProviderPreferences() const
{
  QJsonObject provider;
  provider.insert( u"data_collection"_s, u"deny"_s );
  provider.insert( u"allow_fallbacks"_s, true );

  switch ( mOpenRouterRoutingProfile )
  {
    case OpenRouterRoutingProfile::ToolUseOptimized:
      provider.insert( u"require_parameters"_s, true );
      break;
    case OpenRouterRoutingProfile::CostOptimized:
    default:
      provider.insert( u"sort"_s, u"price"_s );
      break;
  }

  return provider;
}

QString QgsAiModelRouter::authHeaderName( Provider provider ) const
{
  switch ( provider )
  {
    case Provider::Claude:
      return u"x-api-key"_s;
    case Provider::Codex:
    case Provider::OpenAi:
    case Provider::OpenRouter:
    case Provider::Plan:
      return u"Authorization"_s;
  }
  return u"Authorization"_s;
}

QString QgsAiModelRouter::authHeaderValue( Provider provider, const QString &secret ) const
{
  if ( provider == Provider::OpenAi || provider == Provider::Codex || provider == Provider::OpenRouter || provider == Provider::Plan )
    return u"Bearer %1"_s.arg( secret );
  return secret;
}

QString QgsAiModelRouter::authConfigSettingKey( Provider provider ) const
{
  switch ( provider )
  {
    case Provider::OpenAi:
      return u"ai/provider/openai/authcfg"_s;
    case Provider::Codex:
      return u"ai/provider/codex/authcfg"_s;
    case Provider::Claude:
      return u"ai/provider/claude/authcfg"_s;
    case Provider::OpenRouter:
      return u"ai/provider/openrouter/authcfg"_s;
    case Provider::Plan:
      return u"ai/provider/plan/token"_s;
  }
  return QString();
}

QString QgsAiModelRouter::providerSettingPrefix( Provider provider ) const
{
  switch ( provider )
  {
    case Provider::OpenAi:
      return u"ai/provider/openai"_s;
    case Provider::Codex:
      return u"ai/provider/codex"_s;
    case Provider::Claude:
      return u"ai/provider/claude"_s;
    case Provider::OpenRouter:
      return u"ai/provider/openrouter"_s;
    case Provider::Plan:
      return u"ai/provider/plan"_s;
  }
  return u"ai/provider/unknown"_s;
}

QString QgsAiModelRouter::endpointSettingKey( Provider provider ) const
{
  return providerSettingPrefix( provider ) + u"/endpoint"_s;
}

QString QgsAiModelRouter::modelSettingKey( Provider provider ) const
{
  return providerSettingPrefix( provider ) + u"/model"_s;
}

QString QgsAiModelRouter::enabledSettingKey( Provider provider ) const
{
  return providerSettingPrefix( provider ) + u"/enabled"_s;
}

QString QgsAiModelRouter::credentialModeSettingKey( Provider provider ) const
{
  return providerSettingPrefix( provider ) + u"/credentialMode"_s;
}

QString QgsAiModelRouter::autoRoutingSettingKey( Provider provider ) const
{
  return providerSettingPrefix( provider ) + u"/autoRouting"_s;
}

QString QgsAiModelRouter::apiKeySettingKey( Provider provider ) const
{
  return providerSettingPrefix( provider ) + u"/apiKey"_s;
}

QString QgsAiModelRouter::planAuthConfigIdSettingKey() const
{
  return u"ai/provider/plan/authcfg"_s;
}

QString QgsAiModelRouter::planSessionTokenSettingKey() const
{
  return u"ai/provider/plan/token"_s;
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
    case Provider::Codex:
      return QString();
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
    case Provider::OpenRouter:
    {
      const QString envValue = qEnvironmentVariable( "OPENROUTER_API_KEY" ).trimmed();
      if ( !envValue.isEmpty() )
        return envValue;
      break;
    }
    case Provider::Plan:
      return QString();
  }
  return QString();
}

bool QgsAiModelRouter::hasStoredApiKey( Provider provider ) const
{
  if ( provider == Provider::Plan || provider == Provider::Codex )
    return false;

  QgsSettings settings;
  return !settings.value( apiKeySettingKey( provider ) ).toString().trimmed().isEmpty();
}

bool QgsAiModelRouter::hasStoredOAuthRefreshToken( Provider provider ) const
{
  if ( provider == Provider::Codex )
    return QgsAiCodexOAuthClient::hasRefreshToken();
  if ( provider == Provider::Claude )
    return QgsAiClaudeOAuthClient::hasRefreshToken();
  return false;
}

bool QgsAiModelRouter::hasConfiguredCredential( Provider provider ) const
{
  if ( provider == Provider::Codex )
    return QgsAiCodexOAuthClient::hasRefreshToken();

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

  const ProviderSettings settings = mProviderSettings.value( provider );
  if ( provider == Provider::Claude && settings.credentialMode == CredentialMode::OAuth )
    return QgsAiClaudeOAuthClient::hasRefreshToken();

  return !storedApiKey( provider ).isEmpty();
}

void QgsAiModelRouter::loadPersistedProviderSettings()
{
  QgsSettings settings;

  const QList<Provider> providers = { Provider::OpenAi, Provider::Codex, Provider::Claude, Provider::OpenRouter, Provider::Plan };
  for ( Provider provider : providers )
  {
    ProviderSettings providerSettings = mProviderSettings.value( provider );

    const QString endpoint = settings.value( endpointSettingKey( provider ), providerSettings.endpoint ).toString().trimmed();
    if ( !endpoint.isEmpty() )
      providerSettings.endpoint = endpoint;

    const QString model = settings.value( modelSettingKey( provider ), providerSettings.model ).toString().trimmed();
    if ( !model.isEmpty() )
      providerSettings.model = normalizedModelForProvider( provider, model );

    const QString credentialMode = settings.value( credentialModeSettingKey( provider ), providerSettings.credentialMode == CredentialMode::OAuth ? u"oauth"_s : u"apiKey"_s ).toString().trimmed();
    providerSettings.credentialMode = credentialMode.compare( u"oauth"_s, Qt::CaseInsensitive ) == 0 ? CredentialMode::OAuth : CredentialMode::ApiKey;
    if ( provider == Provider::Codex )
      providerSettings.credentialMode = CredentialMode::OAuth;
    if ( provider == Provider::OpenRouter )
      providerSettings.autoRouting = settings.value( autoRoutingSettingKey( provider ), providerSettings.autoRouting ).toBool();

    if ( provider == Provider::Plan )
    {
      providerSettings.authConfigId = settings.value( planAuthConfigIdSettingKey(), providerSettings.authConfigId ).toString().trimmed();
      providerSettings.enabled = settings.value( enabledSettingKey( provider ), !providerSettings.authConfigId.isEmpty() ).toBool();
    }
    else if ( provider == Provider::OpenAi || provider == Provider::OpenRouter )
    {
      providerSettings.authConfigId.clear();
      providerSettings.enabled = settings.value( enabledSettingKey( provider ), !storedApiKey( provider ).isEmpty() ).toBool();
    }
    else
    {
      providerSettings.authConfigId.clear();
      const bool hasCredential = provider == Provider::Codex                                ? QgsAiCodexOAuthClient::hasRefreshToken()
                                 : providerSettings.credentialMode == CredentialMode::OAuth ? QgsAiClaudeOAuthClient::hasRefreshToken()
                                                                                            : !storedApiKey( provider ).isEmpty();
      providerSettings.enabled = settings.value( enabledSettingKey( provider ), hasCredential ).toBool();
    }

    mProviderSettings.insert( provider, providerSettings );
  }
}

void QgsAiModelRouter::persistProviderSettings( Provider provider, const ProviderSettings &settings ) const
{
  QgsSettings appSettings;
  appSettings.setValue( endpointSettingKey( provider ), settings.endpoint.trimmed() );
  appSettings.setValue( modelSettingKey( provider ), normalizedModelForProvider( provider, settings.model ).trimmed() );
  appSettings.setValue( enabledSettingKey( provider ), settings.enabled );
  appSettings.setValue( credentialModeSettingKey( provider ), settings.credentialMode == CredentialMode::OAuth ? u"oauth"_s : u"apiKey"_s );
  if ( provider == Provider::OpenRouter )
    appSettings.setValue( autoRoutingSettingKey( provider ), settings.autoRouting );

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
    const QString fallbackError = u"Unable to start network request."_s;
    const QString error = !storedContext.preDispatchError.isEmpty() ? storedContext.preDispatchError : fallbackError;
    queueFailedRequestFinish( storedContext.requestId, error );
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
  {
    context.reply->abort();
  }
  else
  {
    clearRequestTransport( context );
    mRequests.remove( requestId );
  }
}

bool QgsAiModelRouter::storeApiKey( Provider provider, const QString &apiKey, QString *errorMessage )
{
  if ( provider == Provider::Plan )
  {
    if ( errorMessage )
      *errorMessage = u"Use setPlanSessionToken for plan authentication."_s;
    return false;
  }
  if ( provider == Provider::Codex )
  {
    if ( errorMessage )
      *errorMessage = u"Use Codex login for Codex authentication."_s;
    return false;
  }

  if ( apiKey.trimmed().isEmpty() )
  {
    if ( errorMessage )
      *errorMessage = u"API key is empty."_s;
    return false;
  }

  ProviderSettings settings = mProviderSettings.value( provider );
  QgsSettings appSettings;
  appSettings.setValue( apiKeySettingKey( provider ), apiKey.trimmed() );
  settings.authConfigId.clear();
  settings.credentialMode = CredentialMode::ApiKey;
  settings.enabled = true;
  setProviderSettings( provider, settings );
  return true;
}

bool QgsAiModelRouter::setCredentialMode( Provider provider, CredentialMode mode, QString *errorMessage )
{
  if ( provider == Provider::Plan )
  {
    if ( errorMessage )
      *errorMessage = u"Plan authentication mode is configured through its authcfg or session token."_s;
    return false;
  }
  if ( provider == Provider::Codex && mode != CredentialMode::OAuth )
  {
    if ( errorMessage )
      *errorMessage = u"Codex only supports OAuth login."_s;
    return false;
  }

  ProviderSettings settings = mProviderSettings.value( provider );
  settings.credentialMode = mode;
  const bool hasCredential = mode == CredentialMode::OAuth ? hasStoredOAuthRefreshToken( provider ) : !storedApiKey( provider ).isEmpty();
  settings.enabled = hasCredential || settings.enabled;
  setProviderSettings( provider, settings );
  return true;
}

bool QgsAiModelRouter::setPlanSessionToken( const QString &token, QString *errorMessage )
{
  if ( token.trimmed().isEmpty() )
  {
    if ( errorMessage )
      *errorMessage = u"Plan session token is empty."_s;
    return false;
  }

  QgsAuthManager *authManager = QgsApplication::authManager();
  if ( !authManager )
  {
    if ( errorMessage )
      *errorMessage = u"Authentication manager is unavailable."_s;
    return false;
  }

  if ( !authManager->storeAuthSetting( planSessionTokenSettingKey(), token.trimmed(), true ) )
  {
    if ( errorMessage )
      *errorMessage = u"Unable to store plan session token securely."_s;
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
        *errorMessage = u"Authentication manager is unavailable."_s;
      return false;
    }

    if ( !settings.authConfigId.isEmpty() )
    {
      if ( !authManager->updateNetworkRequest( request, settings.authConfigId ) )
      {
        if ( errorMessage )
          *errorMessage = u"Unable to apply plan OAuth authentication."_s;
        return false;
      }
      return true;
    }

    const QVariant token = authManager->authSetting( planSessionTokenSettingKey(), QVariant(), true );
    const QString tokenString = token.toString();
    if ( tokenString.isEmpty() )
    {
      if ( errorMessage )
        *errorMessage = u"Missing plan session token. Please login first."_s;
      return false;
    }

    request.setRawHeader( "Authorization", authHeaderValue( Provider::Plan, tokenString ).toUtf8() );
    return true;
  }

  if ( provider == Provider::Codex )
  {
    QgsAiCodexOAuthClient::TokenSet tokens;
    if ( !QgsAiCodexOAuthClient::refreshAccessToken( tokens, errorMessage ) )
      return false;

    request.setRawHeader( "Authorization", authHeaderValue( Provider::Codex, tokens.accessToken ).toUtf8() );
    request.setRawHeader( "ChatGPT-Account-ID", tokens.chatGptAccountId.toUtf8() );
    request.setRawHeader( "Accept", "text/event-stream" );
    request.setRawHeader( "originator", "codex_cli_rs" );
    request.setRawHeader( "version", "0.128.0" );
    return true;
  }

  if ( provider == Provider::Claude && settings.credentialMode == CredentialMode::OAuth )
  {
    QgsAiClaudeOAuthClient::TokenSet tokens;
    if ( !QgsAiClaudeOAuthClient::refreshAccessToken( tokens, errorMessage ) )
      return false;

    request.setRawHeader( "Authorization", authHeaderValue( Provider::OpenAi, tokens.accessToken ).toUtf8() );
    return true;
  }

  const QString apiKey = storedApiKey( provider );
  if ( apiKey.isEmpty() )
  {
    QgsMessageLog::logMessage( u"No API key configured for %1; request will fail before dispatch."_s.arg( providerDisplayName( provider ) ), u"AI"_s, Qgis::MessageLevel::Warning, false );
    if ( errorMessage )
      *errorMessage = u"No API key configured for %1."_s.arg( providerDisplayName( provider ) );
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
  {
    context.preDispatchError = u"%1 endpoint is not configured. Check Provider Settings."_s.arg( providerDisplayName( context.provider ) );
    return false;
  }

  if ( context.provider == Provider::Plan && !isUsablePlanEndpoint( settings.endpoint ) )
  {
    QgsMessageLog::logMessage( u"Plan provider endpoint is a placeholder (%1); refusing to dispatch."_s.arg( settings.endpoint ), u"AI"_s, Qgis::MessageLevel::Warning, false );
    context.preDispatchError = u"Plan Account endpoint is not configured. Configure a usable endpoint and session token in Provider Settings."_s;
    return false;
  }

  QNetworkRequest request( QUrl( settings.endpoint ) );
  request.setHeader( QNetworkRequest::ContentTypeHeader, u"application/json"_s );
  QgsSettings appSettings;
  const int configuredTimeoutSeconds = std::max( 10, appSettings.value( u"ai/network/timeoutSeconds"_s, 120 ).toInt() );
  request.setTransferTimeout( configuredTimeoutSeconds * 1000 );

  if ( context.provider == Provider::Claude )
  {
    request.setRawHeader( "anthropic-version", "2023-06-01" );
    if ( settings.credentialMode == CredentialMode::OAuth )
      request.setRawHeader( "anthropic-beta", "oauth-2025-04-20" );
  }

  QString authenticationError;
  if ( !applyAuthentication( context.provider, request, &authenticationError ) )
  {
    if ( !authenticationError.isEmpty() )
    {
      QgsMessageLog::logMessage( u"Authentication failed for %1: %2"_s.arg( providerDisplayName( context.provider ), authenticationError ), u"AI"_s, Qgis::MessageLevel::Warning, false );
      context.preDispatchError = authenticationError;
    }
    return false;
  }

  QgsNetworkAccessManager *networkManager = QgsNetworkAccessManager::instance();
  if ( !networkManager )
    context.preDispatchError = u"Network manager is not available."_s;
  if ( !networkManager )
    return false;

  context.startedAtMs = QDateTime::currentMSecsSinceEpoch();
  context.attempt++;
  context.streamingBuffer.clear();
  const QByteArray payload = buildRequestPayload( context.provider, context.messages, context.stream );
  QgsMessageLog::logMessage(
    u"Dispatching request id=%1 provider=%2 endpointHost=%3 model=%4 attempt=%5 payloadBytes=%6 stream=%7"_s
      .arg( context.requestId, providerDisplayName( context.provider ), endpointForLog( settings.endpoint ), settings.model )
      .arg( context.attempt )
      .arg( payload.size() )
      .arg( context.stream ),
    u"AI"_s,
    Qgis::MessageLevel::Info,
    false
  );
  context.reply = networkManager->post( request, payload );
  if ( !context.reply )
    context.preDispatchError = u"Unable to start network request for %1."_s.arg( providerDisplayName( context.provider ) );
  if ( !context.reply )
    return false;

  context.reply->setProperty( "aiRequestId", context.requestId );
  connect( context.reply, &QNetworkReply::readyRead, this, &QgsAiModelRouter::onReplyReadyRead );
  connect( context.reply, &QNetworkReply::finished, this, &QgsAiModelRouter::onReplyFinished );
  startRequestWatchdog( context, configuredTimeoutSeconds );
  return true;
}

void QgsAiModelRouter::queueFailedRequestFinish( const QString &requestId, const QString &errorMessage )
{
  QTimer::singleShot( 0, this, [this, requestId, errorMessage]() { finishRequest( requestId, false, QString(), sanitizeErrorText( errorMessage ), 0, 0, false, 0 ); } );
}

void QgsAiModelRouter::clearRequestTransport( RequestContext &context )
{
  if ( context.watchdogTimer )
  {
    context.watchdogTimer->stop();
    context.watchdogTimer->deleteLater();
    context.watchdogTimer = nullptr;
  }

  if ( context.reply )
  {
    context.reply->deleteLater();
    context.reply = nullptr;
  }
}

void QgsAiModelRouter::finishRequest( const QString &requestId, bool success, const QString &responseText, const QString &errorMessage, int httpStatus, int retryCount, bool retriable, qint64 latencyMs )
{
  if ( !mRequests.contains( requestId ) )
    return;

  RequestContext &context = mRequests[requestId];
  const QString providerName = providerDisplayName( context.provider );
  clearRequestTransport( context );
  mRequests.remove( requestId );

  emit requestFinished( requestId, success, providerName, responseText, errorMessage, httpStatus, retryCount, retriable, latencyMs );
}

void QgsAiModelRouter::startRequestWatchdog( RequestContext &context, int transferTimeoutSeconds )
{
  QgsSettings appSettings;
  const int watchdogSeconds = appSettings.value( u"ai/network/watchdogSeconds"_s, transferTimeoutSeconds + 30 ).toInt();
  if ( watchdogSeconds <= 0 )
    return;

  QTimer *timer = new QTimer( this );
  timer->setSingleShot( true );
  timer->setInterval( std::max( 10, watchdogSeconds ) * 1000 );
  const QString requestId = context.requestId;
  connect( timer, &QTimer::timeout, this, [this, requestId, watchdogSeconds]() {
    if ( !mRequests.contains( requestId ) )
      return;

    RequestContext &timedOutContext = mRequests[requestId];
    const QString providerName = providerDisplayName( timedOutContext.provider );
    const qint64 latencyMs = timedOutContext.startedAtMs > 0 ? std::max<qint64>( 0, QDateTime::currentMSecsSinceEpoch() - timedOutContext.startedAtMs ) : 0;
    QgsMessageLog::logMessage( u"Request id=%1 provider=%2 watchdog timed out after %3 seconds."_s.arg( requestId, providerName ).arg( watchdogSeconds ), u"AI"_s, Qgis::MessageLevel::Warning, false );
    const int retryCount = timedOutContext.attempt - 1;
    finishRequest( requestId, false, QString(), u"Network request watchdog timed out after %1 seconds."_s.arg( watchdogSeconds ), 0, retryCount, false, latencyMs );
  } );
  context.watchdogTimer = timer;
  timer->start();
}

bool QgsAiModelRouter::shouldRetry( int httpStatus, QNetworkReply::NetworkError networkError, int attempt, int maxRetries ) const
{
  if ( attempt > maxRetries )
    return false;

  if ( networkError != QNetworkReply::NoError )
  {
    return networkError == QNetworkReply::TimeoutError || networkError == QNetworkReply::TemporaryNetworkFailureError || networkError == QNetworkReply::NetworkSessionFailedError;
  }

  return httpStatus == 429 || ( httpStatus >= 500 && httpStatus <= 599 );
}

QString QgsAiModelRouter::extractTextFromResponse( Provider provider, const QJsonObject &object ) const
{
  if ( provider == Provider::Claude )
  {
    const QJsonArray content = object.value( u"content"_s ).toArray();
    QString text;
    for ( const QJsonValue &item : content )
    {
      const QJsonObject contentObject = item.toObject();
      if ( contentObject.value( u"type"_s ).toString() == "text"_L1 )
        text += contentObject.value( u"text"_s ).toString();
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
    const QString eventType = object.value( u"type"_s ).toString();
    if ( eventType == "content_block_delta"_L1 )
    {
      const QJsonObject delta = object.value( u"delta"_s ).toObject();
      // text_delta carries assistant text; input_json_delta carries tool args (handled elsewhere).
      if ( delta.value( u"type"_s ).toString() == "text_delta"_L1 )
        return delta.value( u"text"_s ).toString();
    }
    return QString();
  }

  // OpenAI Responses API streaming: text deltas come as response.output_text.delta
  const QString eventType = object.value( u"type"_s ).toString();
  if ( eventType == "response.output_text.delta"_L1 )
    return object.value( u"delta"_s ).toString();

  // Fallback to old behavior for legacy Chat Completions style chunks
  const QString directDelta = object.value( u"delta"_s ).toString();
  if ( !directDelta.isEmpty() )
    return directDelta;

  const QJsonObject deltaObject = object.value( u"delta"_s ).toObject();
  if ( !deltaObject.isEmpty() )
    return extractTextRecursive( deltaObject );

  return QString();
}

void QgsAiModelRouter::extractToolCallsFromResponse( Provider provider, const QJsonObject &object, RequestContext &context ) const
{
  if ( provider == Provider::Claude )
  {
    context.stopReason = object.value( u"stop_reason"_s ).toString();
    const QJsonArray content = object.value( u"content"_s ).toArray();
    for ( const QJsonValue &item : content )
    {
      const QJsonObject block = item.toObject();
      if ( block.value( u"type"_s ).toString() != "tool_use"_L1 )
        continue;

      PendingToolCall call;
      call.id = block.value( u"id"_s ).toString();
      call.name = block.value( u"name"_s ).toString();
      call.argumentsObject = block.value( u"input"_s ).toObject();
      call.argumentsArePreParsed = true;
      context.toolCalls.append( call );
    }
    return;
  }

  if ( provider == Provider::OpenAi || provider == Provider::Codex || provider == Provider::OpenRouter )
  {
    // Top-level "status" or per-output items don't carry stop_reason explicitly here;
    // we infer tool_use by the presence of function_call items.
    const QJsonArray output = object.value( u"output"_s ).toArray();
    for ( const QJsonValue &item : output )
    {
      const QJsonObject obj = item.toObject();
      if ( obj.value( u"type"_s ).toString() != "function_call"_L1 )
        continue;

      PendingToolCall call;
      call.id = obj.value( u"call_id"_s ).toString();
      call.name = obj.value( u"name"_s ).toString();
      call.argumentsRaw = obj.value( u"arguments"_s ).toString();
      const QJsonDocument doc = QJsonDocument::fromJson( call.argumentsRaw.toUtf8() );
      if ( doc.isObject() )
      {
        call.argumentsObject = doc.object();
        call.argumentsArePreParsed = true;
      }
      context.toolCalls.append( call );
    }

    if ( !context.toolCalls.isEmpty() )
      context.stopReason = u"tool_use"_s;
  }
}

void QgsAiModelRouter::absorbStreamEvent( Provider provider, const QJsonObject &object, RequestContext &context )
{
  if ( provider == Provider::Claude )
  {
    const QString eventType = object.value( u"type"_s ).toString();

    if ( eventType == "content_block_start"_L1 )
    {
      const int blockIndex = object.value( u"index"_s ).toInt();
      const QJsonObject block = object.value( u"content_block"_s ).toObject();
      if ( block.value( u"type"_s ).toString() == "tool_use"_L1 )
      {
        PendingToolCall call;
        call.id = block.value( u"id"_s ).toString();
        call.name = block.value( u"name"_s ).toString();
        context.toolCalls.append( call );
        context.streamItemIndexToToolCall.insert( blockIndex, context.toolCalls.size() - 1 );
      }
    }
    else if ( eventType == "content_block_delta"_L1 )
    {
      const QJsonObject delta = object.value( u"delta"_s ).toObject();
      if ( delta.value( u"type"_s ).toString() == "input_json_delta"_L1 )
      {
        const int blockIndex = object.value( u"index"_s ).toInt();
        const int toolIndex = context.streamItemIndexToToolCall.value( blockIndex, -1 );
        if ( toolIndex >= 0 && toolIndex < context.toolCalls.size() )
          context.toolCalls[toolIndex].argumentsRaw += delta.value( u"partial_json"_s ).toString();
      }
    }
    else if ( eventType == "content_block_stop"_L1 )
    {
      const int blockIndex = object.value( u"index"_s ).toInt();
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
    else if ( eventType == "message_delta"_L1 )
    {
      const QJsonObject delta = object.value( u"delta"_s ).toObject();
      const QString reason = delta.value( u"stop_reason"_s ).toString();
      if ( !reason.isEmpty() )
        context.stopReason = reason;
    }
    return;
  }

  if ( provider == Provider::OpenAi || provider == Provider::Codex || provider == Provider::OpenRouter )
  {
    const QString eventType = object.value( u"type"_s ).toString();

    if ( eventType == "response.output_item.added"_L1 )
    {
      const int outputIndex = object.value( u"output_index"_s ).toInt();
      const QJsonObject item = object.value( u"item"_s ).toObject();
      if ( item.value( u"type"_s ).toString() == "function_call"_L1 )
      {
        PendingToolCall call;
        call.id = item.value( u"call_id"_s ).toString();
        call.name = item.value( u"name"_s ).toString();
        call.argumentsRaw = item.value( u"arguments"_s ).toString();
        context.toolCalls.append( call );
        context.streamItemIndexToToolCall.insert( outputIndex, context.toolCalls.size() - 1 );
      }
    }
    else if ( eventType == "response.function_call_arguments.delta"_L1 )
    {
      const int outputIndex = object.value( u"output_index"_s ).toInt();
      const int toolIndex = context.streamItemIndexToToolCall.value( outputIndex, -1 );
      if ( toolIndex >= 0 && toolIndex < context.toolCalls.size() )
        context.toolCalls[toolIndex].argumentsRaw += object.value( u"delta"_s ).toString();
    }
    else if ( eventType == "response.function_call_arguments.done"_L1 )
    {
      const int outputIndex = object.value( u"output_index"_s ).toInt();
      const int toolIndex = context.streamItemIndexToToolCall.value( outputIndex, -1 );
      if ( toolIndex >= 0 && toolIndex < context.toolCalls.size() )
      {
        PendingToolCall &call = context.toolCalls[toolIndex];
        const QString finalArgs = object.value( u"arguments"_s ).toString();
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
    else if ( eventType == "response.completed"_L1 )
    {
      // If we collected function_calls, stop_reason is conceptually "tool_use".
      if ( !context.toolCalls.isEmpty() )
        context.stopReason = u"tool_use"_s;
      else
        context.stopReason = u"stop"_s;
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
    if ( dataLine.startsWith( "data:"_L1 ) )
      dataLine = dataLine.mid( 5 ).trimmed();
    if ( dataLine == "[DONE]"_L1 )
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

QString QgsAiModelRouter::extractErrorMessageFromBody( Provider provider, const QByteArray &body ) const //#spellok
{
  if ( body.isEmpty() )
    return QString();

  const QJsonDocument doc = QJsonDocument::fromJson( body );
  if ( !doc.isObject() )
    return QString();

  const QJsonObject root = doc.object();
  const QJsonValue errorValue = root.value( u"error"_s );

  // OpenAI: { "error": { "message": "...", "type": "...", "code": "..." } }
  if ( errorValue.isObject() )
  {
    const QJsonObject errorObj = errorValue.toObject();
    const QString message = errorObj.value( u"message"_s ).toString();
    const QString type = errorObj.value( u"type"_s ).toString();
    const QString code = errorObj.value( u"code"_s ).toString();
    if ( !message.isEmpty() )
    {
      QStringList parts;
      parts << message;
      if ( !type.isEmpty() )
        parts << u"type=%1"_s.arg( type );
      if ( !code.isEmpty() )
        parts << u"code=%1"_s.arg( code );
      return parts.join( u" · "_s );
    }
  }
  else if ( errorValue.isString() )
  {
    return errorValue.toString();
  }

  // Anthropic: { "type": "error", "error": { "type": "invalid_request_error", "message": "..." } }
  if ( provider == Provider::Claude && root.value( u"type"_s ).toString() == "error"_L1 )
  {
    const QJsonObject errorObj = root.value( u"error"_s ).toObject();
    const QString message = errorObj.value( u"message"_s ).toString();
    const QString type = errorObj.value( u"type"_s ).toString();
    if ( !message.isEmpty() )
      return type.isEmpty() ? message : u"%1 · type=%2"_s.arg( message, type );
  }

  // Plain string fallback
  const QString topLevelMessage = root.value( u"message"_s ).toString();
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
  // For streamed error responses the server may deliver the JSON error in a
  // single chunk that readyRead already drained into streamingBuffer; fall
  // back to it so we don't lose the error message.
  QByteArray responseBody = reply->readAll();
  if ( responseBody.isEmpty() && !context->streamingBuffer.isEmpty() )
    responseBody = context->streamingBuffer.toUtf8();
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
    u"Reply id=%1 provider=%2 httpStatus=%3 networkError=%4 latencyMs=%5 textLen=%6 bodyBytes=%7 success=%8 toolCalls=%9 stopReason=%10"_s.arg( requestId, providerName )
      .arg( httpStatus )
      .arg( static_cast<int>( networkError ) )
      .arg( latencyMs )
      .arg( responseText.size() )
      .arg( responseBody.size() )
      .arg( success )
      .arg( context->toolCalls.size() )
      .arg( context->stopReason ),
    u"AI"_s,
    success ? Qgis::MessageLevel::Info : Qgis::MessageLevel::Warning,
    false
  );

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
    clearRequestTransport( *context );
    mRequests.remove( requestId );
    emit toolCallsRequested( requestId, providerName, responseText.trimmed(), publicCalls );
    return;
  }

  if ( success )
  {
    finishRequest( requestId, true, responseText.trimmed(), QString(), httpStatus, context->attempt - 1, false, latencyMs );
    return;
  }

  const bool retriable = shouldRetry( httpStatus, networkError, context->attempt, context->maxRetries );
  if ( retriable )
  {
    clearRequestTransport( *context );
    if ( !dispatchRequest( *context ) )
    {
      const QString retryError = !context->preDispatchError.isEmpty() ? context->preDispatchError : u"Retry dispatch failed."_s;
      finishRequest( requestId, false, QString(), sanitizeErrorText( retryError ), httpStatus, context->attempt - 1, true, latencyMs );
    }
    return;
  }

  QString errorMessage = extractErrorMessageFromBody( context->provider, responseBody ); //#spellok
  if ( errorMessage.isEmpty() )
    errorMessage = reply->errorString();
  if ( errorMessage.isEmpty() )
    errorMessage = responseText;
  errorMessage = sanitizeErrorText( errorMessage );

  finishRequest( requestId, false, QString(), errorMessage, httpStatus, context->attempt - 1, false, latencyMs );
}

bool QgsAiModelRouter::isUsablePlanEndpoint( const QString &endpoint )
{
  const QString trimmed = endpoint.trimmed();
  if ( trimmed.isEmpty() )
    return false;
  return !trimmed.contains( "example.invalid"_L1, Qt::CaseInsensitive );
}

QgsAiModelRouter::Provider QgsAiModelRouter::resolveProvider() const
{
  const ProviderSettings plan = mProviderSettings.value( Provider::Plan );
  if ( plan.enabled && isUsablePlanEndpoint( plan.endpoint ) && hasConfiguredCredential( Provider::Plan ) )
    return Provider::Plan;

  const ProviderSettings codex = mProviderSettings.value( Provider::Codex );
  if ( codex.enabled && hasConfiguredCredential( Provider::Codex ) )
    return Provider::Codex;

  const ProviderSettings openRouter = mProviderSettings.value( Provider::OpenRouter );
  if ( openRouter.enabled && hasConfiguredCredential( Provider::OpenRouter ) )
    return Provider::OpenRouter;

  const ProviderSettings openAi = mProviderSettings.value( Provider::OpenAi );
  if ( openAi.enabled && hasConfiguredCredential( Provider::OpenAi ) )
    return Provider::OpenAi;

  const ProviderSettings claude = mProviderSettings.value( Provider::Claude );
  if ( claude.enabled && hasConfiguredCredential( Provider::Claude ) )
    return Provider::Claude;

  return Provider::OpenAi;
}
