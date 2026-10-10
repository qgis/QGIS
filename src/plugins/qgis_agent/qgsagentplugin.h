#ifndef QGSAGENTPLUGIN_H
#define QGSAGENTPLUGIN_H

#include "../qgisplugin.h"

#include <QByteArray>
#include <QDateTime>
#include <QDockWidget>
#include <QHash>
#include <QJsonArray>
#include <QJsonObject>
#include <QObject>
#include <QPointer>
#include <QProcess>
#include <QRectF>
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
    void selectionChanged( const QString &layerId, const QStringList &previousFeatureIds );
    void canvasExtentChanged( double xMinimum, double yMinimum, double xMaximum, double yMaximum );
    void layerEdited( const QString &layerId );
    void fileCreated( const QString &path );

  private slots:
    void acceptConnection();
    void readRequest();

  private:
    QJsonObject executeTool( const QString &tool, const QJsonObject &arguments );
    QJsonObject projectSummary() const;
    QJsonObject inspectLayer( const QJsonObject &arguments ) const;
    QJsonObject listDataSourceLayers( const QJsonObject &arguments ) const;
    QJsonObject loadLayer( const QJsonObject &arguments );
    QJsonObject exportLayer( const QJsonObject &arguments );
    QJsonObject exportMapLayout( const QJsonObject &arguments );
    QJsonObject saveProject( const QJsonObject &arguments );
    QJsonObject queryFeatures( const QJsonObject &arguments ) const;
    QJsonObject fieldStatistics( const QJsonObject &arguments ) const;
    QJsonObject validateExpression( const QJsonObject &arguments ) const;
    QJsonObject selectFeatures( const QJsonObject &arguments );
    QJsonObject clearSelection( const QJsonObject &arguments );
    QJsonObject mapCanvasState() const;
    QJsonObject zoomToLayer( const QJsonObject &arguments );
    QJsonObject inspectRaster( const QJsonObject &arguments ) const;
    QJsonObject suggestRasterThreshold( const QJsonObject &arguments ) const;
    QJsonObject qualityCheck( const QJsonObject &arguments ) const;
    QJsonObject editVectorLayer( const QJsonObject &arguments );
    QJsonObject saveWorkflow( const QJsonObject &arguments ) const;
    QJsonObject listWorkflows() const;
    QJsonObject runWorkflow( const QJsonObject &arguments );
    QJsonObject runBatchWorkflow( const QJsonObject &arguments );
    QJsonObject listProcessingAlgorithms( const QJsonObject &arguments ) const;
    QJsonObject runProcessingAlgorithm( const QJsonObject &arguments );
    QJsonObject runProcessingAlgorithmInternal( const QJsonObject &arguments, bool confirm );
    QJsonObject recentDiagnostics( const QJsonObject &arguments ) const;
    QJsonObject downloadRemoteFile( const QJsonObject &arguments );
    QJsonObject geocodePlace( const QJsonObject &arguments );
    QJsonObject queryOverpass( const QJsonObject &arguments );
    QJsonObject searchStac( const QJsonObject &arguments );
    void recordToolDiagnostic( const QString &tool, const QJsonObject &arguments, const QJsonObject &result );
    class QgsMapLayer *findLayer( const QString &reference, QString &error ) const;
    bool confirmWriteAction( const QString &title, const QString &details ) const;
    QJsonObject layerDetails( class QgsMapLayer *layer ) const;
    QJsonObject algorithmDetails( const class QgsProcessingAlgorithm *algorithm ) const;
    void sendResponse( QTcpSocket *socket, int statusCode, const QJsonObject &body );

    QgisInterface *mInterface = nullptr;
    QTcpServer *mServer = nullptr;
    QHash<QTcpSocket *, QByteArray> mBuffers;
    QString mToken;
    bool mConfirmActions = true;
    bool mToolRequestActive = false;
    QDateTime mLastGeocodeRequest;
    QJsonArray mRecentToolDiagnostics;
    QJsonArray mRecentQgisMessages;
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
    void recordSelectionChange( const QString &layerId, const QStringList &previousFeatureIds );
    void recordCanvasExtentChange( double xMinimum, double yMinimum, double xMaximum, double yMaximum );
    void recordLayerEdit( const QString &layerId );
    void recordCreatedFile( const QString &path );

  private:
    bool eventFilter( QObject *watched, QEvent *event ) override;

    struct TranscriptBlock
    {
      TranscriptBlock( const QString &roleValue, const QString &messageValue, const QStringList &undoLayerIdsValue, bool undoneValue, int turnIndexValue, bool userBlockValue, bool withdrawnValue )
        : role( roleValue )
        , message( messageValue )
        , undoLayerIds( undoLayerIdsValue )
        , undone( undoneValue )
        , turnIndex( turnIndexValue )
        , userBlock( userBlockValue )
        , withdrawn( withdrawnValue )
      {
      }

      QString role;
      QString message;
      QStringList undoLayerIds;
      bool undone = false;
      int turnIndex = -1;
      bool userBlock = false;
      bool withdrawn = false;
      QHash<QString, QStringList> undoSelections;
      QHash<QString, int> undoEditCounts;
      QStringList undoFiles;
      QRectF undoCanvasExtent;
      bool hasUndoCanvasExtent = false;
    };

    void startAgentSession();
    void shutdownAgentSession();
    void startAgentThread();
    void startCurrentTurnRequest();
    bool recoverMissingThread( const QString &message );
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
    void renderExecutionLog();
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
    QToolButton *mExecutionToggle = nullptr;
    QTextBrowser *mExecutionLog = nullptr;
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
    QHash<QString, int> mTurnIndexes;
    QSet<QString> mCommittedAgentMessageIds;
    qint64 mNextRequestId = 1;
    QString mThreadId;
    QString mTurnId;
    QString mCurrentTaskText;
    QVector<TranscriptBlock> mTranscriptBlocks;
    QString mStreamingAnswer;
    QString mStreamingAnswerItemId;
    int mStreamingAnswerTurnIndex = -1;
    QString mStreamingReasoning;
    QString mStreamingReasoningItemId;
    QStringList mPlanSteps;
    QStringList mCompletedToolActivities;
    QStringList mCurrentTurnUndoLayerIds;
    QStringList mCurrentTurnUndoDescriptions;
    QHash<QString, QStringList> mCurrentTurnUndoSelections;
    QHash<QString, int> mCurrentTurnUndoEditCounts;
    QStringList mCurrentTurnUndoFiles;
    QRectF mCurrentTurnUndoCanvasExtent;
    bool mCurrentTurnHasUndoCanvasExtent = false;
    int mCurrentTurnUserBlockIndex = -1;
    int mCurrentTurnIndex = -1;
    int mNextTurnIndex = 1;
    QHash<QString, QString> mRunningToolActivities;
    QSet<int> mWithdrawnTurnIndexes;
    QString mSessionModel;
    bool mAgentReady = false;
    bool mTurnActive = false;
    bool mShuttingDown = false;
    bool mRecoveringThread = false;
    int mCurrentTurnRetryCount = 0;
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
