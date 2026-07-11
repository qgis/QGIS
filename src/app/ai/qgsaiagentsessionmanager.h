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

#include "ai/index/qgsaiworkspaceindex.h"
#include "qgis_app.h"
#include "qgsaiagentpolicy.h"
#include "qgsaichathistorystore.h"
#include "qgsaimodelrouter.h"
#include "qgsaimodels.h"
#include "qgsaitool.h"

#include <QList>
#include <QObject>
#include <QString>
#include <QVariantList>

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

/**
 * User-configurable behavior for the agent: rules, skills and a master toggle that
 * gates whether the agent is allowed to perform custom actions (tool use). The
 * settings are persisted in QgsSettings and loaded once at construction time.
 */
struct APP_EXPORT QgsAiAgentBehaviorSettings
{
    static constexpr int DEFAULT_TOOL_CALL_PAUSE_LIMIT = 5;
    static constexpr int MIN_TOOL_CALL_PAUSE_LIMIT = 1;
    static constexpr int MAX_TOOL_CALL_PAUSE_LIMIT = 50;

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
    //! Workspace-relative directory for rules files. Defaults to ".strata/rules".
    QString rulesPath = u".strata/rules"_s;
    //! Workspace-relative directory for skills files. Defaults to ".strata/skills".
    QString skillsPath = u".strata/skills"_s;
    //! Tool-use rounds allowed before pausing for explicit user continuation.
    int maxToolIterationsPerTurn = DEFAULT_TOOL_CALL_PAUSE_LIMIT;
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
    bool updateMessageMetadata( const QString &messageId, const QVariantMap &metadata );

    /**
     * Sets the persistent chat history store. When set, every message appended
     * to the in-memory history is also written to SQLite when the current
     * history scope is persistent. Pass nullptr to disable persistence.
     * Ownership is not transferred.
     */
    void setHistoryStore( QgsAiChatHistoryStore *store ) { mHistoryStore = store; }
    QgsAiChatHistoryStore *historyStore() const { return mHistoryStore; }

    //! Returns the persisted sessions for the current history scope ordered by most recent first.
    QList<QgsAiChatHistoryStore::SessionInfo> listSessions() const;
    //! Returns the active session id, or an empty string if no session has been created yet.
    QString activeSessionId() const { return mActiveSessionId; }
    //! Loads \a sessionId from the store, replacing the in-memory history. Emits historyReplaced().
    void loadSession( const QString &sessionId );

    /**
     * Closes the current session and starts a fresh empty one (the new session row
     * is created lazily on the first user message). Emits historyReplaced().
     */
    void startNewSession();
    //! Renames the active session in the persistent store and emits sessionListChanged().
    void renameActiveSession( const QString &title );
    //! Renames \a sessionId in the persistent store. Emits sessionListChanged().
    void renameSession( const QString &sessionId, const QString &title );
    //! Removes \a sessionId from the store. If it was the active one, starts a new session.
    void deleteSession( const QString &sessionId );

    //! Returns a stable per-project chat history scope key for \a projectFilePath.
    static QString chatHistoryScopeKeyForProjectFile( const QString &projectFilePath );

    //! Applies a per-project chat history scope. Empty scope means unsaved project and disables persistence.
    void setProjectChatHistoryScopeKey( const QString &scopeKey );

    //! Resets the current chat and switches to the unsaved-project memory-only history scope.
    void resetProjectChatHistoryScope();

    //! Returns true when the current chat history scope can persist to SQLite.
    bool hasPersistentChatHistoryScope() const;

    //! Returns the current explicit chat history scope key, or an empty string for unsaved/legacy scopes.
    QString chatHistoryScopeKey() const;

    //! Extracts a fenced ```strata_agent_plan JSON block from assistant text, if present.
    static QJsonObject extractAgentPlanJson( const QString &text );

    //! Validates the agent V2 plan JSON contract used by Plan mode and executable runs.
    static bool validateAgentPlanJson( const QJsonObject &plan, QString *errorMessage = nullptr );

    //! Runtime metadata-only memory for recent agent decisions/tool events.
    QVariantList agentMemoryEvents() const { return mAgentMemory; }

    void sendUserMessage( const QString &text, const QString &filePath = QString(), const QString &selectedText = QString() );
    void sendUserMessage( const QString &text, const QList<QgsAiChatContextFile> &contextFiles );
    bool continueAfterToolLimit( const QString &messageId );
    void cancelActiveRequest();
    bool hasActiveRequest() const { return !mActiveRequestId.isEmpty(); }
    QStringList projectFileCandidates( const QString &query, int maxResults = 25 ) const;
    QString resolveProjectFile( const QString &filePath ) const;
    QString workspaceRoot() const;
    void setWorkspaceRoot( const QString &workspaceRoot );

    //! Returns the file context provider backing this session, used by QgsAiRulesSkillsStore.
    QgsAiFileContextProvider *fileContextProvider() const { return mContextProvider; }

    void setToolRegistry( QgsAiToolRegistry *registry );
    QgsAiToolRegistry *toolRegistry() const { return mToolRegistry; }

    /**
     * Sets the workspace RAG index used to retrieve relevant chunks for each
     * user message before contacting the model. Pass nullptr to disable retrieval.
     */
    void setWorkspaceIndex( QgsAiWorkspaceIndex *index ) { mWorkspaceIndex = index; }
    QgsAiWorkspaceIndex *workspaceIndex() const { return mWorkspaceIndex; }

    //! Maximum number of chunks injected into the system prompt for a single turn.
    static constexpr int RETRIEVAL_TOP_K = 8;
    //! Hard byte cap for the "Retrieved context" block appended to the system prompt.
    static constexpr int RETRIEVAL_BYTE_CAP = 64 * 1024;

    /**
     * Renders \a chunks as a textual block ready to be appended to the system prompt.
     * Truncates with a marker if the total size exceeds \a byteCap. Public for
     * unit-testing the formatting in isolation.
     */
    static QString formatRetrievedContext( const QList<QgsAiWorkspaceIndex::Chunk> &chunks, int byteCap = RETRIEVAL_BYTE_CAP );

    /**
     * Wraps untrusted content (RAG chunks, file snippets, free-text tool output)
     * in an `<untrusted-data source="…">` block, neutralizing any nested wrapper
     * markers so the content cannot escape the block. Public for unit testing.
     */
    static QString wrapUntrusted( const QString &sourceLabel, const QString &text );

    //! Flattens an untrusted label (layer/file name) to a single safe line for the wrapper attribute.
    static QString sanitizeUntrustedLabel( const QString &label );

    /**
     * Returns the current agent behavior settings (rules, skills, custom actions toggle).
     * The values are kept in sync with QgsSettings.
     */
    QgsAiAgentBehaviorSettings agentBehaviorSettings() const { return mBehaviorSettings; }

    /**
     * Persists \a settings in QgsSettings and propagates the master toggle to the
     * model router so subsequent requests reflect the new tool-use policy.
     */
    void setAgentBehaviorSettings( const QgsAiAgentBehaviorSettings &settings );

    //! Applies the managed Strata Cloud policy fetched from `/v1/agents/policy`.
    void setManagedAgentPolicy( const QgsAiManagedAgentPolicy &policy );
    QgsAiManagedAgentPolicy managedAgentPolicy() const { return mManagedAgentPolicy; }

    /**
     * Returns the rules text combined from inline settings and per-file workspace
     * rules. Rules with alwaysApply=true are inlined in full; others contribute only
     * a name/description reference the agent can expand via the read_file tool.
     */
    QString collectRulesContent() const;
    /**
     * Returns the skills text combined from inline settings and a compact
     * name/description index of per-file workspace skills (progressive disclosure —
     * the agent loads a skill's full SKILL.md body on demand via the read_file tool).
     */
    QString collectSkillsContent() const;

    /**
     * Returns the cumulative token/cost accounting for the current session,
     * summed across every model response (including tool-call rounds).
     */
    QgsAiUsage sessionUsage() const { return mSessionUsage; }

    //! Rough token budget for the conversation history sent to the provider (excludes system prompt).
    static constexpr int HISTORY_TOKEN_BUDGET = 32768;

    /**
     * Returns the providers to try for the next request, preferred one first.
     * Only USABLE providers (enabled + configured credentials) are included,
     * so the list may be empty when nothing is configured.
     */
    QList<QgsAiModelRouter::Provider> providerFallbackOrder() const;

  signals:
    void messageAdded( const QgsAiChatMessage &message );
    void proposalCreated( const QString &proposalId );
    void responseChunkReceived( const QString &chunk );
    void requestStateChanged( const QString &state, const QString &detail );
    void requestRunningChanged( bool running );

    /**
     * Emitted whenever the cumulative per-session token/cost accounting changes:
     * after every model response carrying usage, and with an empty total when a
     * new session starts (runtime accumulation only, not persisted).
     */
    void sessionUsageChanged( const QgsAiUsage &total );

    /**
     * Emitted after loadSession() / startNewSession(). The UI should clear the
     * transcript and re-render from history().
     */
    void historyReplaced();

    /**
     * Emitted whenever a session is created, renamed or deleted. The UI should
     * rebuild its history list on the next open.
     */
    void sessionListChanged();

  private:
    void startProviderAttempt( QgsAiModelRouter::Provider provider );
    QString actionableError( const QString &providerName, const QString &errorMessage, int httpStatus ) const;
    QgsAiChatMessage buildAssistantMessage( const QString &text ) const;
    QgsAiChatMessage buildAssistantToolUseMessage( const QString &text, const QList<QgsAiToolCall> &calls ) const;
    QgsAiChatMessage buildToolResultMessage( const QgsAiToolCall &call, const QgsAiToolResult &result ) const;
    QString buildContextSummary( const QList<QgsAiChatContextFile> &contextFiles, bool &contextBlocked ) const;
    bool tryBuildPatchProposal( const QString &text, QgsAiPatchProposal &proposal ) const;
    QString buildSystemPrompt( const QString &extraContext = QString() ) const;
    QString formatAgentMemoryForPrompt() const;
    QString retrieveContextForLastUserMessage() const;
    QStringList allowedToolsForActiveAgent() const;
    bool isToolAllowedForActiveAgent( const QString &toolName ) const;
    void refreshRouterToolPolicy();
    //! Per-profile folder where Processing Toolbox picks up user scripts.
    static QString processingScriptsFolder();
    QList<QgsAiChatMessage> trimHistoryByTokenBudget( int budgetTokens ) const;
    QList<QgsAiChatMessage> buildOutgoingMessages() const;
    void onToolCallsRequested( const QString &requestId, const QString &providerName, const QString &assistantText, const QList<QgsAiToolCall> &calls );
    void rememberAgentEvent( const QString &event, const QVariantMap &metadata );

    void loadPersistedBehaviorSettings();
    void persistBehaviorSettings() const;
    QString readWorkspaceTextFiles( const QString &relativeDir ) const;
    //! Renders enabled rules from the per-file store: full body when alwaysApply, a reference line otherwise.
    QString formatRulesFromStore( const QString &relativeDir ) const;
    //! Renders a compact name/description index of enabled skills from the per-file store.
    QString formatSkillsFromStore( const QString &relativeDir ) const;

    /**
     * Appends \a message to mHistory, persists it (if a store is configured and
     * a session is active), and emits messageAdded().
     */
    void recordHistoryMessage( const QgsAiChatMessage &message );

    /**
     * Ensures there is an active session id. If none, creates a new one using
     * \a firstUserText to derive the title.
     */
    void ensureActiveSession( const QString &firstUserText );
    //! Trims the input to a 50-char single-line title for the session list.
    static QString deriveSessionTitle( const QString &text );
    void resetCurrentSessionState( bool emitHistorySignal );
    void persistCurrentHistoryToStore();

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
    QgsAiManagedAgentPolicy mManagedAgentPolicy;
    QgsAiWorkspaceIndex *mWorkspaceIndex = nullptr;
    QgsAiChatHistoryStore *mHistoryStore = nullptr;
    QString mActiveSessionId;
    int mNextMessageOrdering = 0;
    QgsAiUsage mSessionUsage;
    QVariantList mAgentMemory;
};

#endif // QGSAIAGENTSESSIONMANAGER_H
