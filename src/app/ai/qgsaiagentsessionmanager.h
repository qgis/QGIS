#ifndef QGSAIAGENTSESSIONMANAGER_H
#define QGSAIAGENTSESSIONMANAGER_H

#include "qgsaimodelrouter.h"
#include "qgsaimodels.h"
#include "qgis_app.h"

#include <QList>
#include <QObject>

class QgsAiFileContextProvider;
class QgsAiReviewPatchEngine;

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
    QString buildContextSummary( const QList<QgsAiChatContextFile> &contextFiles, bool &contextBlocked ) const;
    bool tryBuildPatchProposal( const QString &text, QgsAiPatchProposal &proposal ) const;

    QgsAiModelRouter *mRouter = nullptr;
    QgsAiFileContextProvider *mContextProvider = nullptr;
    QgsAiReviewPatchEngine *mReviewEngine = nullptr;
    QString mActiveAgent = QStringLiteral( "planner" );
    QList<QgsAiChatMessage> mHistory;
    QList<QgsAiModelRouter::Provider> mPendingProviders;
    QString mActiveRequestId;
    QString mCurrentPrompt;
    QList<QgsAiChatContextFile> mCurrentContextFiles;
    QString mStreamedText;
};

#endif // QGSAIAGENTSESSIONMANAGER_H
