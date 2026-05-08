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
class QgsMapLayer;
class QgsVectorLayer;
class QgsRasterLayer;

struct APP_EXPORT QgsAiChatContextFile
{
    QString filePath;
    QString selectedText;
    bool allowExternal = false;
};

/**
 * User-configurable behaviour for the agent: rules, skills and a master toggle that
 * gates whether the agent is allowed to perform custom actions (tool use). The
 * settings are persisted in QgsSettings and loaded once at construction time.
 */
struct APP_EXPORT QgsAiAgentBehaviorSettings
{
    //! Master toggle. When false the agent must not use any custom tool/action.
    bool allowCustomActions = false;
    //! Inline rules text injected into the system prompt.
    QString rulesText;
    //! Inline skills text injected into the system prompt.
    QString skillsText;
    //! When true, also load .md/.txt files from rulesPath inside the workspace.
    bool loadWorkspaceRules = true;
    //! When true, also load .md/.txt files from skillsPath inside the workspace.
    bool loadWorkspaceSkills = true;
    //! Workspace-relative directory for rules files. Defaults to ".qgis_ai/rules".
    QString rulesPath = u".qgis_ai/rules"_s;
    //! Workspace-relative directory for skills files. Defaults to ".qgis_ai/skills".
    QString skillsPath = u".qgis_ai/skills"_s;
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

    /**
     * Returns the current agent behaviour settings (rules, skills, custom actions toggle).
     * The values are kept in sync with QgsSettings.
     */
    QgsAiAgentBehaviorSettings agentBehaviorSettings() const { return mBehaviorSettings; }

    /**
     * Persists \a settings in QgsSettings and propagates the master toggle to the
     * model router so subsequent requests reflect the new tool-use policy.
     */
    void setAgentBehaviorSettings( const QgsAiAgentBehaviorSettings &settings );

    //! Returns the rules text combined from inline settings and workspace files.
    QString collectRulesContent() const;
    //! Returns the skills text combined from inline settings and workspace files.
    QString collectSkillsContent() const;

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
    QString buildLayerDataDump() const;
    QString dumpVectorLayer( QgsVectorLayer *layer ) const;
    QString dumpRasterLayer( QgsRasterLayer *layer ) const;
    //! Per-profile folder where Processing Toolbox picks up user scripts.
    static QString processingScriptsFolder();
    QList<QgsAiChatMessage> trimHistoryByTokenBudget( int budgetTokens ) const;
    QList<QgsAiChatMessage> buildOutgoingMessages() const;
    void onToolCallsRequested( const QString &requestId, const QString &providerName, const QString &assistantText, const QList<QgsAiToolCall> &calls );

    void loadPersistedBehaviorSettings();
    void persistBehaviorSettings() const;
    QString readWorkspaceTextFiles( const QString &relativeDir ) const;

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
    QgsAiAgentBehaviorSettings mBehaviorSettings;
};

#endif // QGSAIAGENTSESSIONMANAGER_H
