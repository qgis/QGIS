/***************************************************************************
    qgsaiagentsessionmanager.h
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

#ifndef QGSAIAGENTSESSIONMANAGER_H
#define QGSAIAGENTSESSIONMANAGER_H

#include "qgis_app.h"
#include "qgsaimodelrouter.h"
#include "qgsaimodels.h"
#include "qgsaitool.h"

#include <QList>
#include <QObject>
#include <QString>

using namespace Qt::StringLiterals;

class QgsAiFileContextProvider;
class QgsAiReviewPatchEngine;
class QgsAiToolRegistry;

struct APP_EXPORT QgsAiChatContextFile
{
    QString filePath;
    QString selectedText;
    bool allowExternal = false;
};

class APP_EXPORT QgsAiAgentSessionManager : public QObject
{
    Q_OBJECT

  public:
    explicit QgsAiAgentSessionManager( QgsAiModelRouter *router, QgsAiFileContextProvider *contextProvider, QgsAiReviewPatchEngine *reviewEngine, QObject *parent = nullptr );

    QStringList availableAgents() const;
    QString activeAgent() const { return mActiveAgent; }
    void setActiveAgent( const QString &agentName );

    QList<QgsAiChatMessage> history() const { return mHistory; }
    void clearHistory();

    void sendUserMessage( const QString &text, const QString &filePath = QString(), const QString &selectedText = QString() );
    void sendUserMessage( const QString &text, const QList<QgsAiChatContextFile> &contextFiles );
    void cancelActiveRequest();
    bool hasActiveRequest() const { return !mActiveRequestId.isEmpty(); }
    QStringList projectFileCandidates( const QString &query, int maxResults = 25 ) const;
    QString resolveProjectFile( const QString &filePath ) const;
    QString workspaceRoot() const;

    void setToolRegistry( QgsAiToolRegistry *registry );
    QgsAiToolRegistry *toolRegistry() const { return mToolRegistry; }

    //! Maximum tool-use rounds the agent will run before bailing out for a single user turn.
    static constexpr int MAX_TOOL_ITERATIONS_PER_TURN = 8;
    //! Rough token budget for the conversation history sent to the provider (excludes system prompt).
    static constexpr int HISTORY_TOKEN_BUDGET = 32768;

  signals:
    void messageAdded( const QgsAiChatMessage &message );
    void proposalCreated( const QString &proposalId );
    void responseChunkReceived( const QString &chunk );
    void requestStateChanged( const QString &state, const QString &detail );
    void requestRunningChanged( bool running );

  private:
    void startProviderAttempt( QgsAiModelRouter::Provider provider );
    QList<QgsAiModelRouter::Provider> providerFallbackOrder() const;
    QString actionableError( const QString &providerName, const QString &errorMessage, int httpStatus ) const;
    QgsAiChatMessage buildAssistantMessage( const QString &text ) const;
    QgsAiChatMessage buildAssistantToolUseMessage( const QString &text, const QList<QgsAiToolCall> &calls ) const;
    QgsAiChatMessage buildToolResultMessage( const QgsAiToolCall &call, const QgsAiToolResult &result ) const;
    QString buildContextSummary( const QList<QgsAiChatContextFile> &contextFiles, bool &contextBlocked ) const;
    bool tryBuildPatchProposal( const QString &text, QgsAiPatchProposal &proposal ) const;
    QString buildSystemPrompt() const;
    //! Per-profile folder where Processing Toolbox picks up user scripts.
    static QString processingScriptsFolder();
    QList<QgsAiChatMessage> trimHistoryByTokenBudget( int budgetTokens ) const;
    QList<QgsAiChatMessage> buildOutgoingMessages() const;
    void onToolCallsRequested( const QString &requestId, const QString &providerName, const QString &assistantText, const QList<QgsAiToolCall> &calls );

    QgsAiModelRouter *mRouter = nullptr;
    QgsAiFileContextProvider *mContextProvider = nullptr;
    QgsAiReviewPatchEngine *mReviewEngine = nullptr;
    QgsAiToolRegistry *mToolRegistry = nullptr;
    QString mActiveAgent = u"planner"_s;
    QList<QgsAiChatMessage> mHistory;
    QList<QgsAiModelRouter::Provider> mPendingProviders;
    QString mActiveRequestId;
    QgsAiModelRouter::Provider mActiveProvider = QgsAiModelRouter::Provider::OpenAi;
    QString mCurrentPrompt;
    QList<QgsAiChatContextFile> mCurrentContextFiles;
    QString mStreamedText;
    int mToolIterations = 0;
};

#endif // QGSAIAGENTSESSIONMANAGER_H
