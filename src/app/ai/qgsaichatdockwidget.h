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
class QEvent;
class QFrame;
class QHBoxLayout;
class QLabel;
class QListWidget;
class QPushButton;
class QShowEvent;
class QTextEdit;
class QToolButton;

class QgsAiModelRouter;
class QgsAiReviewPatchEngine;

class APP_EXPORT QgsAiChatDockWidget : public QgsDockWidget
{
    Q_OBJECT

  public:
    QgsAiChatDockWidget( QgsAiAgentSessionManager *sessionManager, QgsAiModelRouter *modelRouter, QgsAiReviewPatchEngine *reviewEngine, QWidget *parent = nullptr );

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

  private:
    QString selectedProposalId() const;
    void appendTranscriptLine( const QString &line );
    void appendStreamChunk( const QString &chunk );
    void closeStreamingAssistantMessage();
    void finalizeStreamingAssistantMessage( const QString &finalText );
    void updateRuntimeState( const QString &state, const QString &detail );
    void applyPillStyling();
    void initModeMenu();
    void initModelMenu();
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

    struct AttachedFile
    {
        QString filePath;
        bool allowExternal = true;
    };

    QPointer<QgsAiAgentSessionManager> mSessionManager;
    QPointer<QgsAiModelRouter> mModelRouter;
    QPointer<QgsAiReviewPatchEngine> mReviewEngine;

    QTextEdit *mTranscript = nullptr;
    QTextEdit *mInputTextEdit = nullptr;

    QToolButton *mModePill = nullptr;
    QToolButton *mModelPill = nullptr;
    QToolButton *mAttachButton = nullptr;
    QToolButton *mSettingsButton = nullptr;
    QToolButton *mSendButton = nullptr;
    QPushButton *mCancelButton = nullptr;

    QWidget *mFileContextChipRow = nullptr;
    QHBoxLayout *mFileContextChipLayout = nullptr;
    QList<AttachedFile> mAttachedFiles;

    QFrame *mMentionPopup = nullptr;
    QListWidget *mMentionList = nullptr;
    int mMentionStartPosition = -1;

    QWidget *mReviewContainer = nullptr;
    QListWidget *mProposalList = nullptr;
    QLabel *mReviewStatusLabel = nullptr;
    QLabel *mRuntimeStatusLabel = nullptr;

    bool mStreamingInProgress = false;
    int mStreamingContentStartPosition = -1;
    bool mRequestRunning = false;
};

#endif // QGSAICHATDOCKWIDGET_H
