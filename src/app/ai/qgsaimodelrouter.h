/***************************************************************************
    qgsaimodelrouter.h
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

#ifndef QGSAIMODELROUTER_H
#define QGSAIMODELROUTER_H

#include "qgis_app.h"
#include "qgsaimodels.h"

#include <QJsonArray>
#include <QJsonObject>
#include <QMap>
#include <QNetworkReply>
#include <QObject>
#include <QQueue>
#include <QUrl>

class QgsAiToolRegistry;
class QNetworkRequest;
class QTimer;

class APP_EXPORT QgsAiModelRouter : public QObject
{
    Q_OBJECT

  public:
    enum class Provider
    {
      OpenAi,
      Codex,
      Claude,
      OpenRouter,
      Plan
    };
    Q_ENUM( Provider )

    enum class CredentialMode
    {
      ApiKey,
      OAuth
    };
    Q_ENUM( CredentialMode )

    enum class OpenRouterRoutingProfile
    {
      CostOptimized,
      ToolUseOptimized
    };
    Q_ENUM( OpenRouterRoutingProfile )

    struct ProviderSettings
    {
        QString endpoint;
        QString model;
        QString authConfigId;
        CredentialMode credentialMode = CredentialMode::ApiKey;
        bool enabled = false;
        bool autoRouting = true;
    };

    explicit QgsAiModelRouter( QObject *parent = nullptr );

    ProviderSettings providerSettings( Provider provider ) const;
    void setProviderSettings( Provider provider, const ProviderSettings &settings );

    QString startChatRequest( Provider provider, const QList<QgsAiChatMessage> &messages, bool stream = true );
    void cancelRequest( const QString &requestId );
    bool hasActiveRequest( const QString &requestId ) const;

    bool storeApiKey( Provider provider, const QString &apiKey, QString *errorMessage = nullptr );
    bool setCredentialMode( Provider provider, CredentialMode mode, QString *errorMessage = nullptr );
    bool setPlanSessionToken( const QString &token, QString *errorMessage = nullptr );
    bool clearPlanSessionToken( QString *errorMessage = nullptr );
    QString planSessionToken() const;
    void setPlanAuthConfigId( const QString &authConfigId );

    bool applyAuthentication( Provider provider, QNetworkRequest &request, QString *errorMessage = nullptr ) const;
    Provider resolveProvider() const;

    /**
     * Returns true when \a provider is ready to serve a request: enabled, with
     * stored credentials (API key, OAuth token or env fallback) and — for the
     * Plan backend — a usable endpoint. The provider fallback chain only
     * contains usable providers.
     */
    bool isProviderUsable( Provider provider ) const;

    /**
     * Returns true when \a provider is "synced": it has stored credentials
     * (and, for the Plan backend, a usable endpoint), independent of the
     * \c enabled flag or the active selection. Used to filter the model picker
     * so only configured providers' models are offered.
     */
    bool isProviderAvailable( Provider provider ) const;

    /**
     * The provider the user explicitly selected as active. Persisted across
     * sessions in QgsSettings (\c ai/activeProvider). resolveProvider() returns
     * it when usable, otherwise it falls back to the priority chain.
     */
    Provider activeProvider() const;
    void setActiveProvider( Provider provider );

    static bool isUsablePlanEndpoint( const QString &endpoint );

    //! Default Plan chat endpoint used when nothing is configured yet.
    static QString defaultPlanEndpoint();

    QString providerDisplayName( Provider provider ) const;
    QByteArray buildRequestPayload( Provider provider, const QList<QgsAiChatMessage> &messages, bool stream ) const;
    QString sanitizeErrorText( const QString &errorText ) const;
    bool hasStoredApiKey( Provider provider ) const;
    bool hasStoredOAuthRefreshToken( Provider provider ) const;
    void setOpenRouterRoutingProfile( OpenRouterRoutingProfile profile ) { mOpenRouterRoutingProfile = profile; }
    OpenRouterRoutingProfile openRouterRoutingProfile() const { return mOpenRouterRoutingProfile; }

    /**
     * Returns the OpenRouter `provider` routing preferences object. When
     * \a toolsAdvertised is true, `require_parameters` is always requested so
     * OpenRouter only routes to providers that support tool calling.
     */
    QJsonObject openRouterProviderPreferences( bool toolsAdvertised = false ) const;

    /**
     * Sets the tool registry used to advertise tools to the LLM. Pointer is borrowed,
     * caller retains ownership. Pass nullptr to disable tool advertising.
     */
    void setToolRegistry( QgsAiToolRegistry *registry ) { mToolRegistry = registry; }

    /**
     * Master switch for advertising tools to the model. When false, the request payload
     * will not include the `tools`/`tool_choice` fields, so the model cannot request a
     * tool call even if the registry has tools available.
     */
    void setToolUseEnabled( bool enabled ) { mToolUseEnabled = enabled; }
    bool toolUseEnabled() const { return mToolUseEnabled; }

    /**
     * Restricts advertised tools to \a toolNames. An empty list with this filter enabled
     * means no tools are advertised. clearAllowedToolsFilter() restores the default
     * "all available tools" behavior.
     */
    void setAllowedTools( const QStringList &toolNames );
    void clearAllowedToolsFilter();
    QStringList allowedTools() const { return mAllowedTools; }
    bool hasAllowedToolsFilter() const { return mAllowedToolsFilterEnabled; }

    void setAgentMode( const QString &mode ) { mAgentMode = mode.trimmed(); }
    QString agentMode() const { return mAgentMode; }

  signals:
    void requestProgress( const QString &requestId, const QString &chunk );
    void requestFinished(
      const QString &requestId, bool success, const QString &providerName, const QString &responseText, const QString &errorMessage, int httpStatus, int retryCount, bool retriable, qint64 latencyMs
    );

    /**
     * Emitted when the model finishes a turn requesting one or more tool calls
     * (Anthropic stop_reason="tool_use" or OpenAI Responses function_call output items).
     * The session manager is expected to execute the tools and then call
     * \a continueWithToolResults to continue the conversation.
     */
    void toolCallsRequested( const QString &requestId, const QString &providerName, const QString &assistantText, const QList<QgsAiToolCall> &calls );

    /**
     * Emitted once per completed model response carrying token/cost accounting,
     * including responses that end in a tool-call turn. \a model is the model that
     * actually served the response when the provider reports it (OpenRouter routing
     * may differ from the requested model), otherwise the configured model.
     */
    void usageReported( const QString &requestId, const QString &providerName, const QString &model, const QgsAiUsage &usage );

  private slots:
    void onReplyReadyRead();
    void onReplyFinished();

  private:
    //! Wire protocol used to build payloads and parse responses, decoupled from Provider.
    enum class ApiWireFormat
    {
      OpenAiResponses,       //!< OpenAI Responses API (`input` items, `response.*` SSE events)
      OpenAiChatCompletions, //!< OpenAI Chat Completions API (`messages`, `choices[].delta` SSE chunks)
      AnthropicMessages,     //!< Anthropic Messages API
      PlainMessages          //!< Simple `{role, content}` messages (Plan backend)
    };

    struct PendingToolCall
    {
        QString id; // tool_use_id (Anthropic) / call_id (OpenAI)
        QString name;
        QString argumentsRaw;        // accumulated JSON string (OpenAI streams it as deltas)
        QJsonObject argumentsObject; // Anthropic gives us this directly in the final response
        bool argumentsArePreParsed = false;
    };

    struct RequestContext
    {
        QString requestId;
        Provider provider = Provider::OpenAi;
        QList<QgsAiChatMessage> messages;
        bool stream = true;
        int attempt = 0;
        int maxRetries = 1;
        qint64 startedAtMs = 0;
        QString streamingBuffer;
        QString aggregatedText;
        QString stopReason;                       // "end_turn", "tool_use", "stop", etc.
        QList<PendingToolCall> toolCalls;         // collected during streaming/parse
        QMap<int, int> streamItemIndexToToolCall; // Claude content_block index OR OpenAI output_index/tool_calls index → toolCalls index
        QNetworkReply *reply = nullptr;
        QTimer *watchdogTimer = nullptr;
        QString preDispatchError;
        QString midStreamError; // error delivered inside the SSE stream over HTTP 200
        QgsAiUsage usage;       // token/cost accounting harvested from the response
        QString responseModel;  // model that actually served the response (may differ from the requested one under routing)
    };

    bool dispatchRequest( RequestContext &context );
    void finishRequest( const QString &requestId, bool success, const QString &responseText, const QString &errorMessage, int httpStatus, int retryCount, bool retriable, qint64 latencyMs );
    void queueFailedRequestFinish( const QString &requestId, const QString &errorMessage );
    void clearRequestTransport( RequestContext &context );
    void startRequestWatchdog( RequestContext &context, int transferTimeoutSeconds );
    bool shouldRetry( int httpStatus, QNetworkReply::NetworkError networkError, int attempt, int maxRetries ) const;
    QString extractTextFromResponse( Provider provider, const QJsonObject &object ) const;
    QString extractTextFromStreamEvent( Provider provider, const QJsonObject &object ) const;
    void extractToolCallsFromResponse( Provider provider, const QJsonObject &object, RequestContext &context ) const;
    void absorbStreamEvent( Provider provider, const QJsonObject &object, RequestContext &context );
    QString extractErrorMessageFromBody( Provider provider, const QByteArray &body ) const; //#spellok
    QString composeHttpErrorMessage( Provider provider, int httpStatus, const QByteArray &body ) const;
    QString roleForProvider( Provider provider, QgsAiChatRole role ) const;
    QJsonArray buildAnthropicAssistantContent( const QgsAiChatMessage &message ) const;
    QJsonArray buildAnthropicUserContent( const QgsAiChatMessage &message ) const;
    void appendOpenAiInputItems( Provider provider, const QgsAiChatMessage &message, QJsonArray &input ) const;
    void appendOpenAiChatMessages( const QgsAiChatMessage &message, QJsonArray &messages, QJsonArray &pendingImageMessages ) const;
    ApiWireFormat wireFormatForProvider( Provider provider ) const;
    void finalizePendingToolCallArguments( RequestContext &context ) const;

    static QString generateRequestId();
    QString authHeaderName( Provider provider ) const;
    QString authHeaderValue( Provider provider, const QString &secret ) const;
    QString authConfigSettingKey( Provider provider ) const;
    QString providerSettingPrefix( Provider provider ) const;
    QString endpointSettingKey( Provider provider ) const;
    QString modelSettingKey( Provider provider ) const;
    QString enabledSettingKey( Provider provider ) const;
    QString credentialModeSettingKey( Provider provider ) const;
    QString autoRoutingSettingKey( Provider provider ) const;
    QString apiKeySettingKey( Provider provider ) const;
    QString planAuthConfigIdSettingKey() const;
    QString planSessionTokenSettingKey() const;
    QString storedApiKey( Provider provider ) const;
    bool hasConfiguredCredential( Provider provider ) const;
    QString normalizedModelForProvider( Provider provider, const QString &model ) const;
    void loadPersistedProviderSettings();
    void persistProviderSettings( Provider provider, const ProviderSettings &settings ) const;
    RequestContext *contextFromReply( QNetworkReply *reply );

    QMap<Provider, ProviderSettings> mProviderSettings;
    Provider mActiveProvider = Provider::OpenAi;
    QMap<QString, RequestContext> mRequests;
    QgsAiToolRegistry *mToolRegistry = nullptr;
    bool mToolUseEnabled = false;
    bool mAllowedToolsFilterEnabled = false;
    QStringList mAllowedTools;
    QString mAgentMode;
    mutable QString mCodexPromptCacheKey;
    OpenRouterRoutingProfile mOpenRouterRoutingProfile = OpenRouterRoutingProfile::CostOptimized;
};

#endif // QGSAIMODELROUTER_H
