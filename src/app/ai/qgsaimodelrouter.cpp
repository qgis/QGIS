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
#include "qgsaisecretstore.h"
#include "qgsaitoolregistry.h"
#include "qgsaivisualcontextutils.h"
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
#include <QMetaEnum>
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
  // OpenRouter production endpoint (Chat Completions). The Responses endpoint is
  // beta and kept only as an escape hatch for explicitly configured endpoints.
  constexpr const char *OPENROUTER_DEFAULT_ENDPOINT = "https://openrouter.ai/api/v1/chat/completions";
  constexpr const char *OPENROUTER_LEGACY_RESPONSES_ENDPOINT = "https://openrouter.ai/api/v1/responses";
  // Pinned default: a strong tool-calling model beats openrouter/auto for the agent loop.
  constexpr const char *OPENROUTER_DEFAULT_MODEL = "anthropic/claude-sonnet-4.6";
  constexpr const char *OPENROUTER_AUTO_MODEL = "openrouter/auto";
  // App attribution headers recommended by OpenRouter.
  constexpr const char *OPENROUTER_REFERER = "https://github.com/francemazzi/strata";
  constexpr const char *OPENROUTER_APP_TITLE = "Strata";

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

  /**
   * Parses a `usage` JSON object into QgsAiUsage, accepting both the Chat
   * Completions naming (prompt_tokens/completion_tokens) and the Responses /
   * Anthropic naming (input_tokens/output_tokens).
   */
  QgsAiUsage parseUsageObject( const QJsonObject &usage )
  {
    QgsAiUsage parsed;
    parsed.promptTokens = usage.value( u"prompt_tokens"_s ).toVariant().toLongLong();
    if ( parsed.promptTokens == 0 )
      parsed.promptTokens = usage.value( u"input_tokens"_s ).toVariant().toLongLong();
    parsed.completionTokens = usage.value( u"completion_tokens"_s ).toVariant().toLongLong();
    if ( parsed.completionTokens == 0 )
      parsed.completionTokens = usage.value( u"output_tokens"_s ).toVariant().toLongLong();
    parsed.totalTokens = usage.value( u"total_tokens"_s ).toVariant().toLongLong();
    if ( parsed.totalTokens == 0 )
      parsed.totalTokens = parsed.promptTokens + parsed.completionTokens;
    parsed.cachedTokens = usage.value( u"prompt_tokens_details"_s ).toObject().value( u"cached_tokens"_s ).toVariant().toLongLong();
    if ( parsed.cachedTokens == 0 )
      parsed.cachedTokens = usage.value( u"input_tokens_details"_s ).toObject().value( u"cached_tokens"_s ).toVariant().toLongLong();
    if ( parsed.cachedTokens == 0 )
      parsed.cachedTokens = usage.value( u"cache_read_input_tokens"_s ).toVariant().toLongLong();
    parsed.reasoningTokens = usage.value( u"completion_tokens_details"_s ).toObject().value( u"reasoning_tokens"_s ).toVariant().toLongLong();
    if ( parsed.reasoningTokens == 0 )
      parsed.reasoningTokens = usage.value( u"output_tokens_details"_s ).toObject().value( u"reasoning_tokens"_s ).toVariant().toLongLong();
    parsed.costUsd = usage.value( u"cost"_s ).toDouble();
    return parsed;
  }

  //! Overwrites \a target fields with non-zero values from \a parsed (cumulative streaming updates).
  void mergeUsage( QgsAiUsage &target, const QgsAiUsage &parsed )
  {
    if ( parsed.promptTokens > 0 )
      target.promptTokens = parsed.promptTokens;
    if ( parsed.completionTokens > 0 )
      target.completionTokens = parsed.completionTokens;
    if ( parsed.totalTokens > 0 )
      target.totalTokens = parsed.totalTokens;
    if ( parsed.cachedTokens > 0 )
      target.cachedTokens = parsed.cachedTokens;
    if ( parsed.reasoningTokens > 0 )
      target.reasoningTokens = parsed.reasoningTokens;
    if ( parsed.costUsd > 0 )
      target.costUsd = parsed.costUsd;

    // Anthropic streams report input and output tokens in separate events, so a
    // partial event's synthesized total must never shadow the accumulated one.
    target.totalTokens = std::max( target.totalTokens, target.promptTokens + target.completionTokens );
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

  struct VisualContextImagePayload
  {
      QString mimeType;
      QString base64Data;
  };

  bool readImageFileBase64( const QString &imagePath, const QString &preferredMimeType, QString &mimeType, QString &base64Data )
  {
    QFile file( imagePath );
    if ( !file.open( QIODevice::ReadOnly ) )
      return false;

    const QByteArray bytes = file.readAll();
    if ( bytes.isEmpty() )
      return false;

    mimeType = preferredMimeType.trimmed().isEmpty() ? u"image/png"_s : preferredMimeType;
    base64Data = QString::fromLatin1( bytes.toBase64() );
    return true;
  }

  QList<VisualContextImagePayload> readVisualContextImages( const QgsAiChatMessage &message )
  {
    QList<VisualContextImagePayload> images;
    if ( !QgsAiVisualContextUtils::hasVisualConsent() )
      return images;

    QStringList imagePaths;
    QStringList mimeTypes;

    if ( message.role == QgsAiChatRole::Tool )
    {
      const QString imagePath = message.metadata.value( u"visual_context_image_path"_s ).toString();
      if ( !imagePath.isEmpty() )
      {
        imagePaths << imagePath;
        mimeTypes << message.metadata.value( u"visual_context_mime_type"_s, u"image/png"_s ).toString();
      }
    }
    else if ( message.role == QgsAiChatRole::User )
    {
      imagePaths = message.metadata.value( u"attached_image_paths"_s ).toStringList();
      const QVariantList storedMimes = message.metadata.value( u"attached_image_mime_types"_s ).toList();
      mimeTypes.reserve( storedMimes.size() );
      for ( const QVariant &mime : storedMimes )
        mimeTypes << mime.toString();
    }

    for ( int i = 0; i < imagePaths.size(); ++i )
    {
      VisualContextImagePayload payload;
      const QString preferredMime = i < mimeTypes.size() ? mimeTypes.at( i ) : u"image/png"_s;
      if ( readImageFileBase64( imagePaths.at( i ), preferredMime, payload.mimeType, payload.base64Data ) )
        images << payload;
    }

    return images;
  }

  bool readVisualContextImage( const QgsAiChatMessage &message, QString &mimeType, QString &base64Data )
  {
    const QList<VisualContextImagePayload> images = readVisualContextImages( message );
    if ( images.isEmpty() )
      return false;

    mimeType = images.first().mimeType;
    base64Data = images.first().base64Data;
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
  openRouter.endpoint = QString::fromUtf8( OPENROUTER_DEFAULT_ENDPOINT );
  openRouter.model = QString::fromUtf8( OPENROUTER_DEFAULT_MODEL );
  openRouter.credentialMode = CredentialMode::ApiKey;
  openRouter.autoRouting = true;
  mProviderSettings.insert( Provider::OpenRouter, openRouter );

  ProviderSettings plan;
  plan.endpoint = defaultPlanEndpoint();
  plan.model = u"managed-plan"_s;
  mProviderSettings.insert( Provider::Plan, plan );

  loadPersistedProviderSettings();
  QgsAiSecretStore::migrateLegacySecrets();
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
    return trimmed.isEmpty() ? QString::fromUtf8( OPENROUTER_DEFAULT_MODEL ) : trimmed;

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

  const QList<VisualContextImagePayload> images = readVisualContextImages( message );
  for ( const VisualContextImagePayload &image : images )
  {
    QJsonObject source;
    source.insert( u"type"_s, u"base64"_s );
    source.insert( u"media_type"_s, image.mimeType );
    source.insert( u"data"_s, image.base64Data );

    QJsonObject imageBlock;
    imageBlock.insert( u"type"_s, u"image"_s );
    imageBlock.insert( u"source"_s, source );
    content.push_back( imageBlock );
  }
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

  if ( provider == Provider::OpenAi || provider == Provider::Codex || provider == Provider::OpenRouter )
  {
    const QList<VisualContextImagePayload> images = readVisualContextImages( message );
    for ( const VisualContextImagePayload &image : images )
    {
      QJsonObject imageBlock;
      imageBlock.insert( u"type"_s, u"input_image"_s );
      imageBlock.insert( u"image_url"_s, u"data:%1;base64,%2"_s.arg( image.mimeType, image.base64Data ) );
      content.push_back( imageBlock );
    }
  }

  item.insert( u"content"_s, content );
  input.push_back( item );
}

void QgsAiModelRouter::appendOpenAiChatMessages( const QgsAiChatMessage &message, QJsonArray &messages, QJsonArray &pendingImageMessages ) const
{
  // Tool results are emitted as role:"tool" messages keyed by tool_call_id.
  if ( message.role == QgsAiChatRole::Tool )
  {
    QJsonObject item;
    item.insert( u"role"_s, u"tool"_s );
    item.insert( u"tool_call_id"_s, message.metadata.value( u"tool_call_id"_s ).toString() );
    item.insert( u"content"_s, message.content );
    messages.push_back( item );

    QString mimeType;
    QString base64Data;
    if ( readVisualContextImage( message, mimeType, base64Data ) )
    {
      // Chat Completions uses a nested image_url object, unlike Responses' input_image.
      // The image rides as a user message, which must NOT be interleaved between the
      // tool messages of a parallel tool-call turn (every tool message has to directly
      // follow the assistant tool_calls message) — the caller flushes pendingImageMessages
      // only once the tool-result block ends.
      QJsonObject imageMessage;
      imageMessage.insert( u"role"_s, u"user"_s );

      QJsonArray content;
      QJsonObject textBlock;
      textBlock.insert( u"type"_s, u"text"_s );
      textBlock.insert( u"text"_s, u"Visual context image captured from the QGIS map canvas for the preceding tool result."_s );
      content.push_back( textBlock );

      QJsonObject imageUrl;
      imageUrl.insert( u"url"_s, u"data:%1;base64,%2"_s.arg( mimeType, base64Data ) );
      QJsonObject imageBlock;
      imageBlock.insert( u"type"_s, u"image_url"_s );
      imageBlock.insert( u"image_url"_s, imageUrl );
      content.push_back( imageBlock );

      imageMessage.insert( u"content"_s, content );
      pendingImageMessages.push_back( imageMessage );
    }
    return;
  }

  // Assistant turn: text content plus any tool calls in the nested function format.
  if ( message.role == QgsAiChatRole::Assistant )
  {
    QJsonObject item;
    item.insert( u"role"_s, u"assistant"_s );

    const QVariantList toolCalls = message.metadata.value( u"tool_calls"_s ).toList();
    if ( !toolCalls.isEmpty() )
    {
      QJsonArray callsArray;
      for ( const QVariant &call : toolCalls )
      {
        const QVariantMap callMap = call.toMap();
        QJsonObject function;
        function.insert( u"name"_s, callMap.value( u"name"_s ).toString() );
        const QJsonObject argsObj = QJsonObject::fromVariantMap( callMap.value( u"args"_s ).toMap() );
        function.insert( u"arguments"_s, QString::fromUtf8( QJsonDocument( argsObj ).toJson( QJsonDocument::Compact ) ) );

        QJsonObject callObj;
        callObj.insert( u"id"_s, callMap.value( u"id"_s ).toString() );
        callObj.insert( u"type"_s, u"function"_s );
        callObj.insert( u"function"_s, function );
        callsArray.push_back( callObj );
      }
      item.insert( u"tool_calls"_s, callsArray );
      // Content is null when the assistant turn carried only tool calls.
      item.insert( u"content"_s, message.content.isEmpty() ? QJsonValue() : QJsonValue( message.content ) );
    }
    else
    {
      item.insert( u"content"_s, message.content );
    }
    messages.push_back( item );
    return;
  }

  // user / system: plain {role, content} message, or multipart when images are attached.
  const QList<VisualContextImagePayload> images = readVisualContextImages( message );
  QJsonObject item;
  item.insert( u"role"_s, roleForProvider( Provider::OpenRouter, message.role ) );
  if ( images.isEmpty() )
  {
    item.insert( u"content"_s, message.content );
    messages.push_back( item );
    return;
  }

  QJsonArray content;
  QJsonObject textBlock;
  textBlock.insert( u"type"_s, u"text"_s );
  textBlock.insert( u"text"_s, message.content );
  content.push_back( textBlock );

  for ( const VisualContextImagePayload &image : images )
  {
    QJsonObject imageUrl;
    imageUrl.insert( u"url"_s, u"data:%1;base64,%2"_s.arg( image.mimeType, image.base64Data ) );
    QJsonObject imageBlock;
    imageBlock.insert( u"type"_s, u"image_url"_s );
    imageBlock.insert( u"image_url"_s, imageUrl );
    content.push_back( imageBlock );
  }

  item.insert( u"content"_s, content );
  messages.push_back( item );
}

QgsAiModelRouter::ApiWireFormat QgsAiModelRouter::wireFormatForProvider( Provider provider ) const
{
  switch ( provider )
  {
    case Provider::OpenAi:
    case Provider::Codex:
      return ApiWireFormat::OpenAiResponses;
    case Provider::Claude:
      return ApiWireFormat::AnthropicMessages;
    case Provider::OpenRouter:
    {
      // Production path is Chat Completions; a custom endpoint ending in
      // /responses keeps the legacy OpenAI Responses wire format.
      const QString endpointPath = QUrl( mProviderSettings.value( provider ).endpoint ).path();
      return endpointPath.endsWith( "/responses"_L1 ) ? ApiWireFormat::OpenAiResponses : ApiWireFormat::OpenAiChatCompletions;
    }
    case Provider::Plan:
      return ApiWireFormat::PlainMessages;
  }
  return ApiWireFormat::PlainMessages;
}

QByteArray QgsAiModelRouter::buildRequestPayload( Provider provider, const QList<QgsAiChatMessage> &messages, bool stream ) const
{
  QJsonObject payload;
  const ProviderSettings settings = providerSettings( provider );
  const QString model = normalizedModelForProvider( provider, settings.model );
  payload.insert( u"model"_s, model );
  payload.insert( u"stream"_s, stream );
  if ( provider == Provider::Plan && !mAgentMode.isEmpty() )
    payload.insert( u"agent_mode"_s, mAgentMode );

  const ApiWireFormat wireFormat = wireFormatForProvider( provider );

  if ( wireFormat == ApiWireFormat::OpenAiChatCompletions )
  {
    // OpenRouter production path: Chat Completions wire format.
    QJsonArray chatMessages;
    QJsonArray pendingImageMessages;
    for ( const QgsAiChatMessage &message : messages )
    {
      // Flush deferred visual-context images only once the tool-result block ends,
      // preserving the assistant→tool adjacency required by Chat Completions.
      if ( message.role != QgsAiChatRole::Tool && !pendingImageMessages.isEmpty() )
      {
        for ( const QJsonValue &imageMessage : std::as_const( pendingImageMessages ) )
          chatMessages.push_back( imageMessage );
        pendingImageMessages = QJsonArray();
      }
      appendOpenAiChatMessages( message, chatMessages, pendingImageMessages );
    }
    for ( const QJsonValue &imageMessage : std::as_const( pendingImageMessages ) )
      chatMessages.push_back( imageMessage );
    payload.insert( u"messages"_s, chatMessages );

    QJsonArray toolSchemas;
    if ( mToolUseEnabled && mToolRegistry && mToolRegistry->count() > 0 )
    {
      if ( !mAllowedToolsFilterEnabled )
        toolSchemas = mToolRegistry->schemasJsonForFormat( QgsAiToolRegistry::WireFormat::OpenAiChatCompletions );
      else if ( !mAllowedTools.isEmpty() )
        toolSchemas = mToolRegistry->schemasJsonForFormat( QgsAiToolRegistry::WireFormat::OpenAiChatCompletions, mAllowedTools );
    }
    if ( !toolSchemas.isEmpty() )
    {
      payload.insert( u"tools"_s, toolSchemas );
      payload.insert( u"tool_choice"_s, u"auto"_s );
    }

    QgsSettings appSettings;
    const int maxTokens = std::max( 256, appSettings.value( u"ai/provider/openrouter/maxTokens"_s, 8192 ).toInt() );
    payload.insert( u"max_tokens"_s, maxTokens );

    if ( settings.autoRouting )
    {
      payload.insert( u"provider"_s, openRouterProviderPreferences( !toolSchemas.isEmpty() ) );

      // Pinned model with automatic failover to the auto router if it is unavailable.
      if ( model != QString::fromUtf8( OPENROUTER_AUTO_MODEL ) )
      {
        QJsonArray fallbackModels;
        fallbackModels.append( model );
        fallbackModels.append( QString::fromUtf8( OPENROUTER_AUTO_MODEL ) );
        payload.insert( u"models"_s, fallbackModels );
      }
    }
  }
  else if ( wireFormat == ApiWireFormat::OpenAiResponses )
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
      // Legacy Responses escape hatch (custom endpoint ending in /responses).
      if ( !toolSchemas.isEmpty() )
      {
        payload.insert( u"tools"_s, toolSchemas );
        payload.insert( u"tool_choice"_s, u"auto"_s );
      }

      if ( settings.autoRouting )
        payload.insert( u"provider"_s, openRouterProviderPreferences( !toolSchemas.isEmpty() ) );
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

QJsonObject QgsAiModelRouter::openRouterProviderPreferences( bool toolsAdvertised ) const
{
  QJsonObject provider;
  provider.insert( u"data_collection"_s, u"deny"_s );
  provider.insert( u"allow_fallbacks"_s, true );

  // Whenever tools are advertised, only route to providers that support every
  // request parameter — a cheap model that drops `tools` breaks the agent loop.
  if ( toolsAdvertised || mOpenRouterRoutingProfile == OpenRouterRoutingProfile::ToolUseOptimized )
    provider.insert( u"require_parameters"_s, true );

  if ( mOpenRouterRoutingProfile == OpenRouterRoutingProfile::CostOptimized )
  {
    QgsSettings settings;
    const QString sort = settings.value( u"ai/provider/openrouter/sort"_s, u"price"_s ).toString().trimmed().toLower();
    if ( sort == "price"_L1 || sort == "throughput"_L1 )
      provider.insert( u"sort"_s, sort );
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
  QStringList envFallbacks;
  switch ( provider )
  {
    case Provider::OpenAi:
      envFallbacks << u"OPENAI_API_KEY"_s;
      break;
    case Provider::Claude:
      envFallbacks << u"CLAUDE_API_KEY"_s << u"ANTHROPIC_API_KEY"_s;
      break;
    case Provider::OpenRouter:
      envFallbacks << u"OPENROUTER_API_KEY"_s;
      break;
    case Provider::Codex:
    case Provider::Plan:
      return QString();
  }

  return QgsAiSecretStore::readSecret( apiKeySettingKey( provider ), envFallbacks );
}

bool QgsAiModelRouter::hasStoredApiKey( Provider provider ) const
{
  if ( provider == Provider::Plan || provider == Provider::Codex )
    return false;

  return QgsAiSecretStore::hasSecret( apiKeySettingKey( provider ) );
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

    if ( QgsAiSecretStore::hasSecret( planSessionTokenSettingKey() ) )
      return true;

    // Legacy: tokens stored straight in the auth vault (no presence flag) by
    // older builds. existsAuthSetting() never decrypts, so it cannot trigger
    // the master password prompt.
    QgsAuthManager *authManager = QgsApplication::authManager();
    return authManager && !authManager->isDisabled() && authManager->existsAuthSetting( planSessionTokenSettingKey() );
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

    // Migrate away from the historical example.invalid placeholder so fresh
    // and previously-unconfigured profiles can log in without touching the
    // Advanced endpoint field.
    if ( provider == Provider::Plan && providerSettings.endpoint.contains( "example.invalid"_L1, Qt::CaseInsensitive ) )
      providerSettings.endpoint = defaultPlanEndpoint();

    const QString model = settings.value( modelSettingKey( provider ), providerSettings.model ).toString().trimmed();
    if ( !model.isEmpty() )
      providerSettings.model = normalizedModelForProvider( provider, model );

    const QString credentialMode = settings.value( credentialModeSettingKey( provider ), providerSettings.credentialMode == CredentialMode::OAuth ? u"oauth"_s : u"apiKey"_s ).toString().trimmed();
    providerSettings.credentialMode = credentialMode.compare( u"oauth"_s, Qt::CaseInsensitive ) == 0 ? CredentialMode::OAuth : CredentialMode::ApiKey;
    if ( provider == Provider::Codex )
      providerSettings.credentialMode = CredentialMode::OAuth;
    if ( provider == Provider::OpenRouter )
    {
      providerSettings.autoRouting = settings.value( autoRoutingSettingKey( provider ), providerSettings.autoRouting ).toBool();

      // One-time migration away from the beta Responses endpoint and the
      // unpredictable auto-routed default model. Runs once: an explicit user
      // choice made afterwards (including openrouter/auto) is respected forever.
      const QString migrationFlagKey = providerSettingPrefix( provider ) + u"/defaultModelMigrated_v1"_s;
      if ( !settings.value( migrationFlagKey, false ).toBool() )
      {
        if ( providerSettings.endpoint == QString::fromUtf8( OPENROUTER_LEGACY_RESPONSES_ENDPOINT ) )
        {
          providerSettings.endpoint = QString::fromUtf8( OPENROUTER_DEFAULT_ENDPOINT );
          settings.setValue( endpointSettingKey( provider ), providerSettings.endpoint );
          QgsMessageLog::logMessage( u"Migrated OpenRouter endpoint from beta Responses API to Chat Completions."_s, u"AI"_s, Qgis::MessageLevel::Info, false );
        }
        // Only re-pin the model on official openrouter.ai endpoints: on a custom
        // gateway an explicit openrouter/auto choice may be deliberate.
        const bool officialEndpoint = QUrl( providerSettings.endpoint ).host().compare( u"openrouter.ai"_s, Qt::CaseInsensitive ) == 0;
        if ( officialEndpoint && providerSettings.model == QString::fromUtf8( OPENROUTER_AUTO_MODEL ) )
        {
          providerSettings.model = QString::fromUtf8( OPENROUTER_DEFAULT_MODEL );
          settings.setValue( modelSettingKey( provider ), providerSettings.model );
          QgsMessageLog::logMessage( u"Migrated OpenRouter default model from openrouter/auto to %1."_s.arg( providerSettings.model ), u"AI"_s, Qgis::MessageLevel::Info, false );
        }
        settings.setValue( migrationFlagKey, true );
      }
    }

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

  // Restore the explicit active selection (runs after the per-provider loop, so
  // it is unaffected by the OpenRouter one-time migration above). A not-yet-synced
  // value is fine: resolveProvider() falls back when the active provider is unusable.
  const QString activeKey = settings.value( u"ai/activeProvider"_s ).toString().trimmed();
  if ( !activeKey.isEmpty() )
  {
    bool ok = false;
    const int value = QMetaEnum::fromType<Provider>().keyToValue( activeKey.toUtf8().constData(), &ok );
    if ( ok )
      mActiveProvider = static_cast<Provider>( value );
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
  QgsSettings appSettings;
  context.maxRetries = std::clamp( appSettings.value( u"ai/network/maxRetries"_s, 2 ).toInt(), 0, 5 );
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
  QgsAiSecretStore::writeSecret( apiKeySettingKey( provider ), apiKey.trimmed() );
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

  // Never-prompt policy: the secret store uses the vault only when it is
  // already unlocked, otherwise it falls back to cleartext settings instead of
  // triggering the master password dialog.
  if ( !QgsAiSecretStore::writeSecret( planSessionTokenSettingKey(), token.trimmed() ) )
  {
    if ( errorMessage )
      *errorMessage = u"Unable to store plan session token."_s;
    return false;
  }

  ProviderSettings settings = mProviderSettings.value( Provider::Plan );
  settings.enabled = true;
  setProviderSettings( Provider::Plan, settings );
  return true;
}

bool QgsAiModelRouter::clearPlanSessionToken( QString *errorMessage )
{
  Q_UNUSED( errorMessage )

  QgsAiSecretStore::removeSecret( planSessionTokenSettingKey() );

  // Legacy: tokens stored straight in the auth vault (no presence flag) by
  // older builds. removeAuthSetting() never decrypts, so it cannot trigger
  // the master password prompt.
  QgsAuthManager *authManager = QgsApplication::authManager();
  if ( authManager && !authManager->isDisabled() && authManager->existsAuthSetting( planSessionTokenSettingKey() ) )
    authManager->removeAuthSetting( planSessionTokenSettingKey() );

  ProviderSettings settings = mProviderSettings.value( Provider::Plan );
  if ( settings.authConfigId.isEmpty() )
    settings.enabled = false;
  setProviderSettings( Provider::Plan, settings );
  return true;
}

QString QgsAiModelRouter::planSessionToken() const
{
  // Never-prompt read: vault only when already unlocked, cleartext fallback
  // otherwise. A locked vault therefore reads as "no token" instead of
  // popping the master password dialog from UI refresh paths.
  return QgsAiSecretStore::readSecret( planSessionTokenSettingKey(), { u"STRATA_PLAN_TOKEN"_s } );
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

    // Never-prompt read: a locked vault reads as "no token" instead of
    // triggering the master password dialog mid-request.
    const QString tokenString = planSessionToken();
    if ( tokenString.trimmed().isEmpty() )
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

  if ( context.provider == Provider::OpenRouter )
  {
    // App attribution headers recommended by OpenRouter.
    request.setRawHeader( "HTTP-Referer", OPENROUTER_REFERER );
    request.setRawHeader( "X-Title", OPENROUTER_APP_TITLE );
    if ( context.stream )
      request.setRawHeader( "Accept", "text/event-stream" );
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
  // Reset per-attempt state so a retry doesn't inherit partial data from a failed attempt.
  context.streamingBuffer.clear();
  context.aggregatedText.clear();
  context.stopReason.clear();
  context.toolCalls.clear();
  context.streamItemIndexToToolCall.clear();
  context.midStreamError.clear();
  context.usage = QgsAiUsage();
  context.responseModel.clear();
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

  // Check the HTTP status first: Qt also sets a NetworkError for HTTP >= 400
  // replies (e.g. ServiceUnavailableError for 503), so testing networkError
  // first would make the status-based retry unreachable.
  if ( httpStatus == 408 || httpStatus == 429 || ( httpStatus >= 500 && httpStatus <= 599 ) )
    return true;

  if ( networkError != QNetworkReply::NoError )
  {
    return networkError == QNetworkReply::TimeoutError || networkError == QNetworkReply::TemporaryNetworkFailureError || networkError == QNetworkReply::NetworkSessionFailedError;
  }

  return false;
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

  // Chat Completions: text lives at choices[0].message.content. Return without
  // the recursive fallback, which would leak tool_calls JSON into chat text.
  if ( wireFormatForProvider( provider ) == ApiWireFormat::OpenAiChatCompletions )
  {
    const QJsonArray choices = object.value( u"choices"_s ).toArray();
    if ( !choices.isEmpty() )
      return choices.first().toObject().value( u"message"_s ).toObject().value( u"content"_s ).toString();
    return QString();
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

  // Chat Completions streaming: text deltas come as choices[0].delta.content.
  // Must return before the recursive fallback below, which would otherwise leak
  // tool-call argument fragments into the chat text.
  if ( wireFormatForProvider( provider ) == ApiWireFormat::OpenAiChatCompletions )
  {
    const QJsonArray choices = object.value( u"choices"_s ).toArray();
    if ( choices.isEmpty() )
      return QString();
    return choices.first().toObject().value( u"delta"_s ).toObject().value( u"content"_s ).toString();
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

void QgsAiModelRouter::finalizePendingToolCallArguments( RequestContext &context ) const
{
  for ( PendingToolCall &call : context.toolCalls )
  {
    if ( call.argumentsArePreParsed )
      continue;
    const QJsonDocument doc = QJsonDocument::fromJson( call.argumentsRaw.toUtf8() );
    if ( doc.isObject() )
    {
      call.argumentsObject = doc.object();
      call.argumentsArePreParsed = true;
    }
  }
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

  const ApiWireFormat wireFormat = wireFormatForProvider( provider );

  if ( wireFormat == ApiWireFormat::OpenAiChatCompletions )
  {
    const QJsonArray choices = object.value( u"choices"_s ).toArray();
    if ( choices.isEmpty() )
      return;

    const QJsonObject choice = choices.first().toObject();
    const QJsonArray toolCalls = choice.value( u"message"_s ).toObject().value( u"tool_calls"_s ).toArray();
    for ( const QJsonValue &item : toolCalls )
    {
      const QJsonObject obj = item.toObject();
      const QJsonObject function = obj.value( u"function"_s ).toObject();

      PendingToolCall call;
      call.id = obj.value( u"id"_s ).toString();
      call.name = function.value( u"name"_s ).toString();
      call.argumentsRaw = function.value( u"arguments"_s ).toString();
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
    else if ( !choice.value( u"finish_reason"_s ).toString().isEmpty() )
      context.stopReason = u"stop"_s;
    return;
  }

  if ( wireFormat == ApiWireFormat::OpenAiResponses )
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

    if ( eventType == "message_start"_L1 )
    {
      const QJsonObject message = object.value( u"message"_s ).toObject();
      mergeUsage( context.usage, parseUsageObject( message.value( u"usage"_s ).toObject() ) );
      if ( !message.value( u"model"_s ).toString().isEmpty() )
        context.responseModel = message.value( u"model"_s ).toString();
    }
    else if ( eventType == "content_block_start"_L1 )
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
      mergeUsage( context.usage, parseUsageObject( object.value( u"usage"_s ).toObject() ) );
    }
    return;
  }

  const ApiWireFormat wireFormat = wireFormatForProvider( provider );

  if ( wireFormat == ApiWireFormat::OpenAiChatCompletions )
  {
    // Usage and the routed model ride on top-level fields; the final usage chunk
    // may carry an empty choices array, so harvest them before the choices check.
    if ( object.contains( u"usage"_s ) )
      mergeUsage( context.usage, parseUsageObject( object.value( u"usage"_s ).toObject() ) );
    if ( !object.value( u"model"_s ).toString().isEmpty() )
      context.responseModel = object.value( u"model"_s ).toString();

    const QJsonArray choices = object.value( u"choices"_s ).toArray();
    if ( choices.isEmpty() )
      return;

    const QJsonObject choice = choices.first().toObject();
    const QJsonObject delta = choice.value( u"delta"_s ).toObject();

    // Tool call fragments accumulate per tool_calls[].index across chunks.
    const QJsonArray toolCallDeltas = delta.value( u"tool_calls"_s ).toArray();
    for ( const QJsonValue &value : toolCallDeltas )
    {
      const QJsonObject callDelta = value.toObject();
      const int callIndex = callDelta.value( u"index"_s ).toInt();
      int toolIndex = context.streamItemIndexToToolCall.value( callIndex, -1 );
      if ( toolIndex < 0 )
      {
        context.toolCalls.append( PendingToolCall() );
        toolIndex = context.toolCalls.size() - 1;
        context.streamItemIndexToToolCall.insert( callIndex, toolIndex );
      }

      PendingToolCall &call = context.toolCalls[toolIndex];
      const QString id = callDelta.value( u"id"_s ).toString();
      if ( !id.isEmpty() )
        call.id = id;
      const QJsonObject function = callDelta.value( u"function"_s ).toObject();
      call.name += function.value( u"name"_s ).toString();
      call.argumentsRaw += function.value( u"arguments"_s ).toString();
    }

    const QString finishReason = choice.value( u"finish_reason"_s ).toString();
    if ( !finishReason.isEmpty() )
    {
      if ( finishReason == "error"_L1 )
      {
        if ( context.midStreamError.isEmpty() )
          context.midStreamError = u"The provider reported a mid-stream error (finish_reason=error)."_s;
      }
      else if ( finishReason == "length"_L1 && !context.toolCalls.isEmpty() )
      {
        // max_tokens hit in the middle of a tool call: the accumulated arguments
        // JSON is truncated and must never be dispatched as empty args.
        if ( context.midStreamError.isEmpty() )
          context.midStreamError = u"The response hit the max_tokens limit while emitting a tool call; the tool arguments were truncated."_s;
      }
      else if ( finishReason == "tool_calls"_L1 || !context.toolCalls.isEmpty() )
      {
        context.stopReason = u"tool_use"_s;
        finalizePendingToolCallArguments( context );
      }
      else
      {
        context.stopReason = u"stop"_s;
      }
    }
    return;
  }

  if ( wireFormat == ApiWireFormat::OpenAiResponses )
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

      const QJsonObject response = object.value( u"response"_s ).toObject();
      mergeUsage( context.usage, parseUsageObject( response.value( u"usage"_s ).toObject() ) );
      if ( !response.value( u"model"_s ).toString().isEmpty() )
        context.responseModel = response.value( u"model"_s ).toString();
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

    // SSE comment lines (e.g. OpenRouter's ": OPENROUTER PROCESSING" keep-alives) are not data.
    if ( rawLine.startsWith( ':'_L1 ) )
      continue;

    QString dataLine = rawLine;
    const bool isDataLine = dataLine.startsWith( "data:"_L1 );
    if ( isDataLine )
      dataLine = dataLine.mid( 5 ).trimmed();
    if ( dataLine == "[DONE]"_L1 )
      continue;

    const QJsonDocument doc = QJsonDocument::fromJson( dataLine.toUtf8() );
    if ( !doc.isObject() )
    {
      // A corrupted data frame can silently lose tool-call arguments: log it.
      if ( isDataLine )
        QgsMessageLog::logMessage( u"Skipping malformed SSE data line for request id=%1: %2"_s.arg( context->requestId, sanitizeErrorText( dataLine.left( 200 ) ) ), u"AI"_s, Qgis::MessageLevel::Warning, false );
      continue;
    }

    const QJsonObject event = doc.object();

    // Providers can deliver errors inside the stream over HTTP 200 — both as a
    // top-level error object (OpenRouter chat/completions) and as a typed error
    // event (Anthropic / OpenAI Responses).
    if ( context->midStreamError.isEmpty() )
    {
      const QJsonValue errorValue = event.value( u"error"_s );
      if ( errorValue.isObject() )
      {
        const QJsonObject errorObj = errorValue.toObject();
        QString message = errorObj.value( u"message"_s ).toString();
        if ( message.isEmpty() && errorObj.contains( u"code"_s ) )
          message = u"Provider returned error code %1."_s.arg( errorObj.value( u"code"_s ).toVariant().toString() );
        if ( !message.isEmpty() )
          context->midStreamError = message;
      }
      else if ( errorValue.isString() && !errorValue.toString().isEmpty() )
      {
        context->midStreamError = errorValue.toString();
      }
      else if ( event.value( u"type"_s ).toString() == "error"_L1 && !event.value( u"message"_s ).toString().isEmpty() )
      {
        context->midStreamError = event.value( u"message"_s ).toString();
      }
    }

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

QString QgsAiModelRouter::composeHttpErrorMessage( Provider provider, int httpStatus, const QByteArray &body ) const
{
  const QString bodyMessage = extractErrorMessageFromBody( provider, body ); //#spellok

  QString prefix;
  switch ( httpStatus )
  {
    case 401:
      prefix = tr( "Authentication failed — the API key is invalid or expired. Check Provider Settings." );
      break;
    case 402:
      prefix = provider == Provider::OpenRouter ? tr( "Insufficient credits on your OpenRouter account — top up at openrouter.ai/credits." )
                                                : tr( "Payment required — the account has insufficient credits." );
      break;
    case 403:
    {
      prefix = tr( "The request was refused (forbidden)." );
      // OpenRouter moderation errors carry metadata.reasons and a flagged_input excerpt.
      const QJsonDocument doc = QJsonDocument::fromJson( body );
      if ( doc.isObject() )
      {
        const QJsonObject metadata = doc.object().value( u"error"_s ).toObject().value( u"metadata"_s ).toObject();
        const QJsonArray reasons = metadata.value( u"reasons"_s ).toArray();
        if ( !reasons.isEmpty() )
        {
          QStringList reasonList;
          for ( const QJsonValue &reason : reasons )
            reasonList << reason.toString();
          prefix = tr( "The request was flagged by moderation: %1." ).arg( reasonList.join( ", "_L1 ) );
          const QString flagged = metadata.value( u"flagged_input"_s ).toString();
          if ( !flagged.isEmpty() )
            prefix += u" "_s + tr( "Flagged input: \"%1\"" ).arg( flagged.left( 100 ) );
        }
      }
      break;
    }
    case 408:
      prefix = tr( "The provider timed out before completing the response." );
      break;
    case 429:
      prefix = tr( "Rate limited by the provider — wait a moment before retrying." );
      break;
    case 502:
      // The routing-specific wording only makes sense for OpenRouter.
      prefix = provider == Provider::OpenRouter ? tr( "The upstream model provider failed to return a valid response — retry, or switch model." )
                                                : tr( "The provider returned an invalid upstream response (HTTP 502) — retry shortly." );
      break;
    case 503:
      prefix = provider == Provider::OpenRouter ? tr( "No provider currently matches the requested model and routing requirements — try a different model or relax the routing preferences." )
                                                : tr( "The provider is temporarily unavailable (HTTP 503) — retry shortly." );
      break;
    default:
      break;
  }

  if ( prefix.isEmpty() )
    return bodyMessage;
  return bodyMessage.isEmpty() ? prefix : prefix + u" · "_s + bodyMessage;
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
      // Non-streaming: also harvest tool calls and usage accounting from the body.
      if ( context->toolCalls.isEmpty() )
        extractToolCallsFromResponse( context->provider, doc.object(), *context );
      mergeUsage( context->usage, parseUsageObject( doc.object().value( u"usage"_s ).toObject() ) );
      if ( !doc.object().value( u"model"_s ).toString().isEmpty() )
        context->responseModel = doc.object().value( u"model"_s ).toString();
    }
    else
      responseText = QString::fromUtf8( responseBody );
  }

  // Providers can deliver errors inside the SSE stream over HTTP 200.
  const bool success = networkError == QNetworkReply::NoError && ( httpStatus == 0 || ( httpStatus >= 200 && httpStatus < 300 ) ) && context->midStreamError.isEmpty();

  // Safety net: a stream can end without its finish chunk (connection cut, or a
  // malformed final SSE line). Parse any pending tool-call arguments now, and
  // never dispatch a call whose accumulated arguments could not be parsed —
  // executing tools with silently-emptied args is worse than failing the turn.
  finalizePendingToolCallArguments( *context );
  bool hasUnparsedToolArguments = false;
  for ( const PendingToolCall &pending : std::as_const( context->toolCalls ) )
  {
    if ( !pending.argumentsArePreParsed && !pending.argumentsRaw.trimmed().isEmpty() )
    {
      hasUnparsedToolArguments = true;
      break;
    }
  }

  const bool wantsToolUse = success && !context->toolCalls.isEmpty() && !hasUnparsedToolArguments;
  const bool retriable = !success && shouldRetry( httpStatus, networkError, context->attempt, context->maxRetries );
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

  // Report usage only when this attempt is final — a retried attempt would
  // otherwise double-count tokens in the per-session accumulation.
  if ( context->usage.isValid() && !retriable )
  {
    const QString servedModel = !context->responseModel.isEmpty() ? context->responseModel : providerSettings( context->provider ).model;
    QgsMessageLog::logMessage(
      u"Usage id=%1 provider=%2 model=%3 promptTokens=%4 completionTokens=%5 cachedTokens=%6 reasoningTokens=%7 costUsd=%8"_s.arg( requestId, providerName, servedModel )
        .arg( context->usage.promptTokens )
        .arg( context->usage.completionTokens )
        .arg( context->usage.cachedTokens )
        .arg( context->usage.reasoningTokens )
        .arg( context->usage.costUsd, 0, 'f', 6 ),
      u"AI"_s,
      Qgis::MessageLevel::Info,
      false
    );
    emit usageReported( requestId, providerName, servedModel, context->usage );
  }

  if ( success && !context->toolCalls.isEmpty() && hasUnparsedToolArguments )
  {
    finishRequest( requestId, false, QString(), tr( "The model response ended with incomplete tool-call arguments (stream truncated); the turn was aborted." ), httpStatus, context->attempt - 1, false, latencyMs );
    return;
  }

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

  if ( retriable )
  {
    // Honor the server's Retry-After header (seconds form, clamped) when present;
    // otherwise back off linearly with the attempt count.
    int retryAfterSeconds = -1;
    if ( reply->hasRawHeader( "Retry-After" ) )
    {
      bool parsedOk = false;
      const int parsed = QString::fromLatin1( reply->rawHeader( "Retry-After" ) ).trimmed().toInt( &parsedOk );
      if ( parsedOk && parsed >= 0 )
        retryAfterSeconds = parsed;
    }
    const int backoffMs = retryAfterSeconds >= 0 ? std::min( retryAfterSeconds, 30 ) * 1000 : 1000 * context->attempt;

    clearRequestTransport( *context );
    QgsMessageLog::
      logMessage( u"Request id=%1 provider=%2 retrying in %3 ms (attempt %4 of %5)."_s.arg( requestId, providerName ).arg( backoffMs ).arg( context->attempt + 1 ).arg( context->maxRetries + 1 ), u"AI"_s, Qgis::MessageLevel::Info, false );
    QTimer::singleShot( backoffMs, this, [this, requestId, httpStatus]() {
      // The request may have been canceled while waiting for the backoff.
      if ( !mRequests.contains( requestId ) )
        return;

      RequestContext &retryContext = mRequests[requestId];
      if ( !dispatchRequest( retryContext ) )
      {
        const QString retryError = !retryContext.preDispatchError.isEmpty() ? retryContext.preDispatchError : u"Retry dispatch failed."_s;
        finishRequest( requestId, false, QString(), sanitizeErrorText( retryError ), httpStatus, retryContext.attempt - 1, true, 0 );
      }
    } );
    return;
  }

  QString errorMessage = context->midStreamError;
  if ( errorMessage.isEmpty() )
    errorMessage = composeHttpErrorMessage( context->provider, httpStatus, responseBody );
  if ( errorMessage.isEmpty() )
    errorMessage = reply->errorString();
  if ( errorMessage.isEmpty() )
    errorMessage = responseText;
  errorMessage = sanitizeErrorText( errorMessage );

  finishRequest( requestId, false, QString(), errorMessage, httpStatus, context->attempt - 1, false, latencyMs );
}

QString QgsAiModelRouter::defaultPlanEndpoint()
{
  // Development default pointing at a local strata-be; swap to the production
  // URL before release.
  return u"http://localhost:3001/ai/messages"_s;
}

bool QgsAiModelRouter::isUsablePlanEndpoint( const QString &endpoint )
{
  const QString trimmed = endpoint.trimmed();
  if ( trimmed.isEmpty() )
    return false;
  return !trimmed.contains( "example.invalid"_L1, Qt::CaseInsensitive );
}

bool QgsAiModelRouter::isProviderUsable( Provider provider ) const
{
  const ProviderSettings settings = mProviderSettings.value( provider );
  if ( !settings.enabled || !hasConfiguredCredential( provider ) )
    return false;
  // The Plan backend additionally needs a real endpoint (the default is a placeholder).
  if ( provider == Provider::Plan && !isUsablePlanEndpoint( settings.endpoint ) )
    return false;
  return true;
}

bool QgsAiModelRouter::isProviderAvailable( Provider provider ) const
{
  if ( !hasConfiguredCredential( provider ) )
    return false;
  // The Plan backend additionally needs a real endpoint (the default is a placeholder).
  if ( provider == Provider::Plan && !isUsablePlanEndpoint( mProviderSettings.value( provider ).endpoint ) )
    return false;
  return true;
}

QgsAiModelRouter::Provider QgsAiModelRouter::activeProvider() const
{
  return mActiveProvider;
}

void QgsAiModelRouter::setActiveProvider( Provider provider )
{
  mActiveProvider = provider;
  const char *key = QMetaEnum::fromType<Provider>().valueToKey( static_cast<int>( provider ) );
  if ( key )
  {
    QgsSettings settings;
    settings.setValue( u"ai/activeProvider"_s, QString::fromUtf8( key ) );
  }
}

QgsAiModelRouter::Provider QgsAiModelRouter::resolveProvider() const
{
  // Honor the user's explicit choice when it is actually usable; otherwise fall
  // back through the priority chain so a stale/unconfigured selection never
  // strands the assistant.
  if ( isProviderUsable( mActiveProvider ) )
    return mActiveProvider;
  for ( Provider provider : { Provider::Plan, Provider::Codex, Provider::OpenRouter, Provider::OpenAi, Provider::Claude } )
  {
    if ( isProviderUsable( provider ) )
      return provider;
  }
  return Provider::OpenAi;
}
