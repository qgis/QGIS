#ifndef QGSAGENTPLUGIN_H
#define QGSAGENTPLUGIN_H

#include "../qgisplugin.h"

#include <QByteArray>
#include <QDockWidget>
#include <QHash>
#include <QJsonObject>
#include <QObject>
#include <QPointer>
#include <QProcess>
#include <QSet>
#include <QVector>

class QAction;
class QCheckBox;
class QComboBox;
class QLabel;
class QMovie;
class QTextBrowser;
class QTextEdit;
class QTcpServer;
class QTcpSocket;
class QToolButton;
class QTimer;
class QUrl;

class QgisInterface;

class QgsAgentServer : public QObject
{
    Q_OBJECT

  public:
    explicit QgsAgentServer( QgisInterface *interface, QObject *parent = nullptr );

    bool start();
    quint16 port() const;
    QString token() const;
    void setConfirmActions( bool confirm );

  signals:
    void activity( const QString &message );
    void layersLoaded( const QStringList &layerIds, const QString &description );

  private slots:
    void acceptConnection();
    void readRequest();

  private:
    QJsonObject executeTool( const QString &tool, const QJsonObject &arguments );
    QJsonObject projectSummary() const;
    QJsonObject inspectLayer( const QJsonObject &arguments ) const;
    QJsonObject listProcessingAlgorithms( const QJsonObject &arguments ) const;
    QJsonObject runProcessingAlgorithm( const QJsonObject &arguments );
    QJsonObject layerDetails( class QgsMapLayer *layer ) const;
    QJsonObject algorithmDetails( const class QgsProcessingAlgorithm *algorithm ) const;
    void sendResponse( QTcpSocket *socket, int statusCode, const QJsonObject &body );

    QgisInterface *mInterface = nullptr;
    QTcpServer *mServer = nullptr;
    QHash<QTcpSocket *, QByteArray> mBuffers;
    QString mToken;
    bool mConfirmActions = true;
};

class QgsAgentDockWidget : public QDockWidget
{
    Q_OBJECT

  public:
    explicit QgsAgentDockWidget( QgisInterface *interface, QWidget *parent = nullptr );
    ~QgsAgentDockWidget() override;

  private slots:
    void submitTask();
    void stopTask();
    void agentStarted();
    void readAgentOutput();
    void readAgentError();
    void agentFinished( int exitCode, QProcess::ExitStatus status );
    void showServerActivity( const QString &message );
    void switchProvider();
    void switchModel();
    void handleTranscriptLink( const QUrl &url );
    void primaryActionTriggered();
    void recordLoadedLayers( const QStringList &layerIds, const QString &description );

  private:
    bool eventFilter( QObject *watched, QEvent *event ) override;

    struct TranscriptBlock
    {
      QString role;
      QString message;
      QStringList undoLayerIds;
      bool undone = false;
      int turnIndex = -1;
      bool userBlock = false;
      bool withdrawn = false;
    };

    void startAgentSession();
    void shutdownAgentSession();
    qint64 sendRequest( const QString &method, const QJsonObject &params = QJsonObject() );
    void sendNotification( const QString &method, const QJsonObject &params = QJsonObject() );
    void sendResponse( const QJsonValue &id, const QJsonObject &result );
    void sendErrorResponse( const QJsonValue &id, int code, const QString &message );
    void handleAgentFrame( const QJsonObject &frame );
    void handleAgentResponse( const QJsonObject &frame );
    void handleAgentNotification( const QString &method, const QJsonObject &params );
    void handleAgentRequest( const QJsonObject &frame );
    void setAgentReady( const QString &threadId );
    void finishTurn( const QString &status );
    void appendMessage( const QString &role, const QString &message );
    void appendMessage( const QString &role, const QString &message, const QStringList &undoLayerIds );
    void appendMessage( const QString &role, const QString &message, const QStringList &undoLayerIds, int turnIndex, bool userBlock );
    void renderTranscript();
    void resetTurnStreaming();
    void commitStreamingAnswer();
    void commitStreamingReasoning();
    void populateProviderOptions();
    void populateModelOptions();
    void positionActionButton();
    void updateActionButton();
    void undoTranscriptBlock( int blockIndex );
    void withdrawTurn( int turnIndex );
    QString agentExecutable( const QString &provider ) const;
    QString agentExecutable() const;
    QString agentHome() const;
    QString agentName() const;
    QString selectedModel() const;
    QString pythonExecutable() const;
    QString bridgePath() const;
    QString adapterPath() const;
    QString connectionFilePath() const;
    QString agentWorkspace() const;
    QString agentInstructions() const;

    QgisInterface *mInterface = nullptr;
    QgsAgentServer *mServer = nullptr;
    QTextBrowser *mTranscript = nullptr;
    QTextEdit *mTaskInput = nullptr;
    QComboBox *mProviderCombo = nullptr;
    QComboBox *mModelCombo = nullptr;
    QCheckBox *mConfirmActions = nullptr;
    QToolButton *mActionButton = nullptr;
    QLabel *mStatusLabel = nullptr;
    QMovie *mLoadingMovie = nullptr;
    QProcess *mProcess = nullptr;
    QTimer *mTimeoutTimer = nullptr;
    QByteArray mOutputBuffer;
    QHash<qint64, QString> mPendingRequests;
    qint64 mNextRequestId = 1;
    QString mThreadId;
    QString mTurnId;
    QVector<TranscriptBlock> mTranscriptBlocks;
    QString mStreamingAnswer;
    QString mStreamingAnswerItemId;
    QString mStreamingReasoning;
    QString mStreamingReasoningItemId;
    QStringList mPlanSteps;
    QStringList mCompletedToolActivities;
    QStringList mCurrentTurnUndoLayerIds;
    QStringList mCurrentTurnUndoDescriptions;
    int mCurrentTurnUserBlockIndex = -1;
    int mCurrentTurnIndex = -1;
    int mNextTurnIndex = 1;
    QHash<QString, QString> mRunningToolActivities;
    QSet<int> mWithdrawnTurnIndexes;
    QString mSessionModel;
    bool mAgentReady = false;
    bool mTurnActive = false;
    bool mShuttingDown = false;
};

class QgsAgentPlugin : public QObject, public QgisPlugin
{
    Q_OBJECT

  public:
    explicit QgsAgentPlugin( QgisInterface *interface );

    void initGui() override;
    void unload() override;

  private slots:
    void showPanel();

  private:
    QgisInterface *mInterface = nullptr;
    QAction *mAction = nullptr;
    QPointer<QgsAgentDockWidget> mDock;
};

#endif
