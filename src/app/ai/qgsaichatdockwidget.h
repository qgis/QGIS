/***************************************************************************
    qgsaichatdockwidget.h
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

#ifndef QGSAICHATDOCKWIDGET_H
#define QGSAICHATDOCKWIDGET_H

#include "qgis_app.h"
#include "qgsaiagentsessionmanager.h"
#include "qgsdockwidget.h"

#include <QList>
#include <QPointer>

class QAction;
class QCheckBox;
class QEvent;
class QFrame;
class QHBoxLayout;
class QJsonObject;
class QLabel;
class QListWidget;
class QPushButton;
class QShowEvent;
class QTextEdit;
class QTimer;
class QToolButton;
class QVBoxLayout;

struct QgsAiGisSuggestion;

class QgsAiChatPromptEdit;
class QgsAiLayerIndexCoordinator;
class QgsAiModelRouter;
class QgsAiPlanClient;
class QgsAiReviewPatchEngine;
class QgsScrollArea;

class APP_EXPORT QgsAiChatDockWidget : public QgsDockWidget
{
    Q_OBJECT

  public:
    QgsAiChatDockWidget( QgsAiAgentSessionManager *sessionManager, QgsAiModelRouter *modelRouter, QgsAiReviewPatchEngine *reviewEngine, QWidget *parent = nullptr );

    void setLayerIndexCoordinator( QgsAiLayerIndexCoordinator *coordinator );

  signals:
    void embeddingProviderSettingsChanged();

  public slots:
    void rebuildHistoryMenu();

  public:
    /**
     * Returns true when the user has not yet consented to layer indexing
     * (i.e. attributes + bounding boxes being processed for retrieval).
     * Callers must surface a confirmation dialog before flipping the toggle on.
     */
    static bool requiresLayerIndexingConsent();

    //! Persists the user's explicit acceptance so the consent dialog never re-appears.
    static void recordLayerIndexingConsent();

  protected:
    bool eventFilter( QObject *watched, QEvent *event ) override;
    void showEvent( QShowEvent *event ) override;

  private slots:
    void sendMessage();
    void attachFile();
    void clearFileContext();
    void refreshProposalList();
    void previewProposal();
    void acceptProposal();
    void rejectProposal();
    void acceptPartialProposal();
    void openProviderSettings();
    void onModeSelected( QAction *action );
    void onModelSelected( QAction *action );
    void cancelRunningRequest();
    void onSendOrStopClicked();
    void onNewChatClicked();
    void onHistoryEntryTriggered( QAction *action );
    void reloadTranscriptFromHistory();
    void refreshGisSuggestionCard();

  private:
    QString selectedProposalId() const;
    //! Prompts for the workspace trust decision on the first AI interaction with an undecided workspace.
    void ensureWorkspaceTrustDecision();
    void appendTranscriptMessage( const QString &role, const QString &content );
    void appendTranscriptMessage( const QgsAiChatMessage &message );
    QString renderToolMessageMarkdown( const QgsAiChatMessage &message ) const;
    static QString renderMarkdown( const QString &md );
    QWidget *createMessageWidget(
      const QString &role, const QString &content, const QVariantMap &metadata = QVariantMap(), const QString &messageId = QString(), QgsAiChatRole messageRole = QgsAiChatRole::Assistant
    );
    QWidget *createCollapsibleSection( const QString &title, const QString &content, const QString &language = QString(), bool collapsed = true );
    QWidget *createPlanActionsWidget( const QString &messageId, const QString &planMarkdown, const QVariantMap &metadata );
    QWidget *createQuestionsWidget( const QString &messageId, const QJsonObject &payload, const QVariantMap &metadata );
    QWidget *createToolLimitActionsWidget( const QString &messageId, const QVariantMap &metadata );
    void clearTranscriptWidgets();
    void scrollTranscriptToBottom();
    void setModeLabel( const QString &label );
    void markMessageStatus( const QString &messageId, const QVariantMap &metadata, const QString &key, const QString &value );
    void acceptPlan( const QString &messageId, const QString &planMarkdown, const QVariantMap &metadata );
    QString saveWorkflowPlan( const QString &planMarkdown, const QString &messageId, QString *errorMessage = nullptr ) const;
    QString exportWorkflowReport( const QString &planMarkdown, const QString &messageId, QString *errorMessage = nullptr ) const;
    void dryRunWorkflowPlan( const QString &messageId, const QString &planMarkdown );
    void runWorkflowPlan( const QString &messageId, const QString &planMarkdown );
    void sendPlanRevision( const QString &messageId, const QString &planMarkdown, const QVariantMap &metadata, QTextEdit *revisionEdit );
    void sendQuestionAnswers( const QString &messageId, const QVariantMap &metadata, QWidget *questionsCard );
    void appendStreamChunk( const QString &chunk );
    void closeStreamingAssistantMessage();
    void updateRuntimeState( const QString &state, const QString &detail );
    //! Refreshes the per-session token/cost label in the status row.
    void updateSessionUsage( const QgsAiUsage &total );
    void applyPillStyling();
    void initModeMenu();
    void initModelMenu();
    //! (Re)builds the model picker menu, filtered to currently synced providers. Safe to call repeatedly.
    void rebuildModelMenu();
    void refreshPlanModels();
    void refreshPlanAgentPolicy();
    //! Pill caption for the active model, e.g. "Codex · GPT-5.4 ▾".
    QString modelPillLabel( QgsAiModelRouter::Provider provider, const QString &displayName ) const;
    void updateFileContextChip();
    void updateMentionPopup();
    void hideMentionPopup();
    void insertSelectedMention();
    void insertMentionFile( const QString &relativePath );
    void rebuildAttachmentChips();
    QList<QgsAiChatContextFile> contextFilesForCurrentMessage( const QString &text ) const;
    bool addAttachedFile( const QString &path );
    void setRequestRunning( bool running );
    void maybeShowWelcomeBanner();
    void sendGisSuggestionToChat( const QgsAiGisSuggestion &suggestion );
    void dismissGisSuggestion( const QString &suggestionId );

    struct AttachedFile
    {
        QString filePath;
        bool allowExternal = true;
    };

    QPointer<QgsAiAgentSessionManager> mSessionManager;
    QPointer<QgsAiModelRouter> mModelRouter;
    QPointer<QgsAiPlanClient> mPlanClient;
    QPointer<QgsAiReviewPatchEngine> mReviewEngine;
    QPointer<QgsAiLayerIndexCoordinator> mLayerIndexCoordinator;

    QgsScrollArea *mTranscriptScrollArea = nullptr;
    QWidget *mTranscriptContainer = nullptr;
    QVBoxLayout *mTranscriptLayout = nullptr;
    QgsAiChatPromptEdit *mInputTextEdit = nullptr;

    QToolButton *mNewChatButton = nullptr;
    QToolButton *mHistoryButton = nullptr;
    QToolButton *mModePill = nullptr;
    QToolButton *mModelPill = nullptr;
    QToolButton *mAttachButton = nullptr;
    QToolButton *mSettingsButton = nullptr;
    QToolButton *mSendButton = nullptr;
    QPushButton *mCancelButton = nullptr;

    QWidget *mFileContextChipRow = nullptr;
    QHBoxLayout *mFileContextChipLayout = nullptr;
    QList<AttachedFile> mAttachedFiles;

    QFrame *mGisCardContainer = nullptr;
    QToolButton *mGisCardToggle = nullptr;
    QWidget *mGisCardBody = nullptr;
    QVBoxLayout *mGisCardBodyLayout = nullptr;
    QTimer *mGisCardRefreshTimer = nullptr;

    QFrame *mMentionPopup = nullptr;
    QListWidget *mMentionList = nullptr;
    int mMentionStartPosition = -1;

    QWidget *mReviewContainer = nullptr;
    QListWidget *mProposalList = nullptr;
    QLabel *mReviewStatusLabel = nullptr;
    QLabel *mRuntimeStatusLabel = nullptr;
    QLabel *mUsageLabel = nullptr;

    bool mStreamingInProgress = false;
    QTextEdit *mStreamingTextEdit = nullptr;
    bool mRequestRunning = false;
};

#endif // QGSAICHATDOCKWIDGET_H
