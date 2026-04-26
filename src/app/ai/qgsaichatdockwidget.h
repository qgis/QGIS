#ifndef QGSAICHATDOCKWIDGET_H
#define QGSAICHATDOCKWIDGET_H

#include "qgsdockwidget.h"
#include "qgis_app.h"

#include <QPointer>

class QAction;
class QEvent;
class QLabel;
class QListWidget;
class QPushButton;
class QTextEdit;
class QToolButton;

class QgsAiAgentSessionManager;
class QgsAiModelRouter;
class QgsAiReviewPatchEngine;

class APP_EXPORT QgsAiChatDockWidget : public QgsDockWidget
{
    Q_OBJECT

  public:
    QgsAiChatDockWidget( QgsAiAgentSessionManager *sessionManager, QgsAiModelRouter *modelRouter, QgsAiReviewPatchEngine *reviewEngine, QWidget *parent = nullptr );

  protected:
    bool eventFilter( QObject *watched, QEvent *event ) override;

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
    void setRequestRunning( bool running );

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
    QLabel *mFileContextChip = nullptr;
    QToolButton *mFileContextClearBtn = nullptr;
    QString mFileContextPath;

    QWidget *mReviewContainer = nullptr;
    QListWidget *mProposalList = nullptr;
    QLabel *mReviewStatusLabel = nullptr;
    QLabel *mRuntimeStatusLabel = nullptr;

    bool mStreamingInProgress = false;
    int mStreamingContentStartPosition = -1;
    bool mRequestRunning = false;
};

#endif // QGSAICHATDOCKWIDGET_H
