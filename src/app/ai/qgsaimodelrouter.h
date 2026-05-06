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

class APP_EXPORT QgsAiModelRouter : public QObject
{
    Q_OBJECT

  public:
    enum class Provider
    {
      OpenAi,
      Claude,
      Plan
    };
    Q_ENUM( Provider )

    struct ProviderSettings
    {
        QString endpoint;
        QString model;
        QString authConfigId;
        bool enabled = false;
    };

    explicit QgsAiModelRouter( QObject *parent = nullptr );

    ProviderSettings providerSettings( Provider provider ) const;
    void setProviderSettings( Provider provider, const ProviderSettings &settings );

    QString startChatRequest( Provider provider, const QList<QgsAiChatMessage> &messages, bool stream = true );
    void cancelRequest( const QString &requestId );
    bool hasActiveRequest( const QString &requestId ) const;

    bool storeApiKey( Provider provider, const QString &apiKey, QString *errorMessage = nullptr );
    bool setPlanSessionToken( const QString &token, QString *errorMessage = nullptr );
    void setPlanAuthConfigId( const QString &authConfigId );

    bool applyAuthentication( Provider provider, QNetworkRequest &request, QString *errorMessage = nullptr ) const;
    Provider resolveProvider() const;
    static bool isUsablePlanEndpoint( const QString &endpoint );

    QString providerDisplayName( Provider provider ) const;
    QByteArray buildRequestPayload( Provider provider, const QList<QgsAiChatMessage> &messages, bool stream ) const;
    QString sanitizeErrorText( const QString &errorText ) const;
    bool hasStoredApiKey( Provider provider ) const;

    /**
     * Sets the tool registry used to advertise tools to the LLM. Pointer is borrowed,
     * caller retains ownership. Pass nullptr to disable tool advertising.
     */
    void setToolRegistry( QgsAiToolRegistry *registry ) { mToolRegistry = registry; }

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

  private slots:
    void onReplyReadyRead();
    void onReplyFinished();

  private:
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
        QMap<int, int> streamItemIndexToToolCall; // Claude content_block index OR OpenAI output_index → toolCalls index
        QNetworkReply *reply = nullptr;
    };

    bool dispatchRequest( RequestContext &context );
    bool shouldRetry( int httpStatus, QNetworkReply::NetworkError networkError, int attempt, int maxRetries ) const;
    QString extractTextFromResponse( Provider provider, const QJsonObject &object ) const;
    QString extractTextFromStreamEvent( Provider provider, const QJsonObject &object ) const;
    void extractToolCallsFromResponse( Provider provider, const QJsonObject &object, RequestContext &context ) const;
    void absorbStreamEvent( Provider provider, const QJsonObject &object, RequestContext &context );
    QString extractErrorMessageFromBody( Provider provider, const QByteArray &body ) const;
    QString roleForProvider( Provider provider, QgsAiChatRole role ) const;
    QJsonArray buildAnthropicAssistantContent( const QgsAiChatMessage &message ) const;
    QJsonArray buildAnthropicUserContent( const QgsAiChatMessage &message ) const;
    void appendOpenAiInputItems( const QgsAiChatMessage &message, QJsonArray &input ) const;

    static QString generateRequestId();
    QString authHeaderName( Provider provider ) const;
    QString authHeaderValue( Provider provider, const QString &secret ) const;
    QString authConfigSettingKey( Provider provider ) const;
    QString providerSettingPrefix( Provider provider ) const;
    QString endpointSettingKey( Provider provider ) const;
    QString modelSettingKey( Provider provider ) const;
    QString enabledSettingKey( Provider provider ) const;
    QString apiKeySettingKey( Provider provider ) const;
    QString planAuthConfigIdSettingKey() const;
    QString planSessionTokenSettingKey() const;
    QString storedApiKey( Provider provider ) const;
    bool hasConfiguredCredential( Provider provider ) const;
    void loadPersistedProviderSettings();
    void persistProviderSettings( Provider provider, const ProviderSettings &settings ) const;
    RequestContext *contextFromReply( QNetworkReply *reply );

    QMap<Provider, ProviderSettings> mProviderSettings;
    QMap<QString, RequestContext> mRequests;
    QgsAiToolRegistry *mToolRegistry = nullptr;
};

#endif // QGSAIMODELROUTER_H
