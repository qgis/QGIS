/***************************************************************************
                             qgsprocessingalgorithmwidgetbase.h
                             ----------------------------------
    Date                 : November 2017
    Copyright            : (C) 2017 Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSPROCESSINGALGORITHMWIDGETBASE_H
#define QGSPROCESSINGALGORITHMWIDGETBASE_H

#include "ui_qgsprocessingalgorithmdialogbase.h"
#include "ui_qgsprocessingalgorithmprogressdialogbase.h"
#include "ui_qgsprocessingcontextoptionsbase.h"

#include "qgis.h"
#include "qgis_gui.h"
#include "qgsprocessingcontext.h"
#include "qgsprocessingfeedback.h"
#include "qgsprocessingwidgetwrapper.h"

#include <QThread>

///@cond NOT_STABLE

class QgsProcessingAlgorithm;
class QToolButton;
class QgsProcessingContextOptionsWidget;
class QgsMessageBar;
class QgsProcessingAlgRunnerTask;
class QgsTask;

/**
 * \ingroup gui
 * \brief Factory for creating QgsProcessingFeedback objects.
 * \note This is not considered stable API and may change in future QGIS versions.
 * \since QGIS 4.2
 */
class GUI_EXPORT QgsProcessingFeedbackGenerator
{
  public:
    virtual ~QgsProcessingFeedbackGenerator();

    /**
     * Creates a new QgsProcessingFeedback object, and transfers ownership to the caller.
     */
    virtual QgsProcessingFeedback *createFeedback() = 0 SIP_FACTORY;
};


/**
 * \ingroup gui
 * \brief Base class for widgets which contain settings for running Processing algorithms.
 * \note This is not considered stable API and may change in future QGIS versions.
 */
class GUI_EXPORT QgsProcessingAlgorithmWidgetBase : public QDialog, public QgsProcessingParametersGenerator, public QgsProcessingContextGenerator, private Ui::QgsProcessingDialogBase
{
    Q_OBJECT

  public:
    /**
     * Log format options.
     * \since QGIS 3.2
     */
    enum class LogFormat : int
    {
      FormatPlainText, //!< Plain text file (.txt)
      FormatHtml,      //!< HTML file (.html)
    };
    Q_ENUM( QgsProcessingAlgorithmWidgetBase::LogFormat )

    /**
     * Widget modes.
     *
     * \since QGIS 3.24
     */
    enum class WidgetMode : int
    {
      Single, //!< Single algorithm execution mode
      Batch,  //!< Batch processing mode
    };
    Q_ENUM( QgsProcessingAlgorithmWidgetBase::WidgetMode )

    /**
     * Constructor for QgsProcessingAlgorithmWidgetBase.
     */
    QgsProcessingAlgorithmWidgetBase( QWidget *parent SIP_TRANSFERTHIS = nullptr, QgsProcessingAlgorithmWidgetBase::WidgetMode mode = QgsProcessingAlgorithmWidgetBase::WidgetMode::Single );
    ~QgsProcessingAlgorithmWidgetBase() override;

    /**
     * Sets the \a algorithm to run in the widget.
     *
     * Ownership of the algorithm instance is transferred to the widget.
     *
     * \see algorithm()
     */
    void setAlgorithm( QgsProcessingAlgorithm *algorithm SIP_TRANSFER );

    /**
     * Returns the algorithm running in the widget.
     * \see setAlgorithm()
     */
    QgsProcessingAlgorithm *algorithm();

    /**
     * Sets the main \a widget for the widget, usually a panel for configuring algorithm parameters.
     * \see mainWidget()
     */
    void setMainWidget( QgsPanelWidget *widget SIP_TRANSFER );

    /**
     * Returns the main widget for the widget, usually a panel for configuring algorithm parameters.
     * \see setMainWidget()
     */
    QgsPanelWidget *mainWidget();

    /**
     * Shows and raises the widget
     *
     * \since QGIS 4.2
     */
    void showWidget();

    /**
     * Switches the widget to the log page.
     */
    void showLog();

    /**
     * Returns TRUE if an algorithm was executed in the widget.
     * \see results()
     * \see setExecuted()
     */
    bool wasExecuted() const { return mExecuted; }

    /**
     * Returns the results returned by the algorithm executed.
     * \see wasExecuted()
     * \see setResults()
     */
    QVariantMap results() const { return mResults; }

    /**
     * Registers a \a generator for creating QgsProcessingFeedback objects for use
     * in the widget.
     *
     * Ownership of \a generator is not transferred, the caller is responsible for
     * ensuring that it exists for the lifetime of the widget.
     *
     * If no generator is registered then a default QgsProcessingFeedback object will be
     * created.
     *
     * \see createFeedback()
     * \since QGIS 4.2
     */
    void registerProcessingFeedbackGenerator( QgsProcessingFeedbackGenerator *generator );

    /**
     * Creates a new processing feedback object, automatically connected to the appropriate
     * slots in this widget.
     *
     * \see registerProcessingFeedbackFactory()
     */
    QgsProcessingFeedback *createFeedback() SIP_FACTORY;

    /**
     * Saves the log contents to a text file (specified by the file \a path), in
     * the given \a format.
     * \see saveLog()
     * \since QGIS 3.2
     */
    void saveLogToFile( const QString &path, QgsProcessingAlgorithmWidgetBase::LogFormat format = QgsProcessingAlgorithmWidgetBase::LogFormat::FormatPlainText );

    /**
     * Returns the logging level to use when running algorithms from the widget.
     *
     * \see setLogLevel()
     * \since QGIS 3.20
     */
    Qgis::ProcessingLogLevel logLevel() const;

    /**
     * Sets the logging \a level to use when running algorithms from the widget.
     *
     * \see logLevel()
     * \since QGIS 3.20
     */
    void setLogLevel( Qgis::ProcessingLogLevel level );

    /**
     * Sets the parameter \a values to show in the widget.
     *
     * \since QGIS 3.24
     */
    virtual void setParameters( const QVariantMap &values );

  public slots:

    /**
     * Reports an \a error string to the widget's log.
     *
     * If \a fatalError is TRUE, the error prevented the algorithm from executing.
     */
    void reportError( const QString &error, bool fatalError );

    /**
     * Pushes a warning information string to the widget's log.
     */
    void pushWarning( const QString &warning );

    /**
     * Pushes an information string to the widget's log.
     */
    void pushInfo( const QString &info );

    /**
     * Pushes a pre-formatted message to the widget's log
     *
     * This can be used to push formatted HTML messages to the widget.
     *
     * \since QGIS 3.36
     */
    void pushFormattedMessage( const QString &html );

    /**
     * Pushes a debug info string to the widget's log.
     */
    void pushDebugInfo( const QString &message );

    /**
     * Pushes command info to the widget's log.
     */
    void pushCommandInfo( const QString &info );

    /**
     * Sets the percentage progress for the widget, between 0 and 100.
     */
    void setPercentage( double percent );

    /**
     * Sets a progress text message.
     */
    void setProgressText( const QString &text );

    /**
     * Pushes a console info string to the widget's log.
     */
    void pushConsoleInfo( const QString &info );

    /**
     * Creates a modal progress dialog showing progress and log messages
     * from this widget.
     */
    QDialog *createProgressDialog();

    /**
     * Clears the current log contents.
     * \since QGIS 3.2
     */
    void clearLog();

    /**
     * Opens a dialog allowing users to save the current log contents.
     * \see saveLogToFile()
     * \since QGIS 3.2
     */
    void saveLog();

    /**
     * Copies the current log contents to the clipboard.
     * \since QGIS 3.2
     */
    void copyLogToClipboard();

    /**
     * Switches the widget to the parameters page.
     */
    void showParameters();

    /**
     * Returns TRUE if the widget is currently running an algorithm.
     *
     * If the algorithm has not yet started or has finished executing then FALSE will be returned.
     *
     * \note Unlike the isFinalized() method, isRunning() only provides
     * information about the state of widget and is not tied to the
     * deletion mechanisms of the widget.
     *
     * \see isFinalized()
     *
     * \since QGIS 4.2
     */
    virtual bool isRunning();

    /**
     * Cancels any algorithm currently running through the widget.
     *
     * \note The cancellation is non-blocking, and may take some time to complete. Some algorithms
     * ignore cancellation requests entirely. This method returns immediately after requesting
     * the cancellation, but the algorithm may still be running after return.
     *
     * \since QGIS 4.2
     */
    void cancel();

    /**
     * Forces the widget to close by detaching any running task from the widget THEN closing the widget.
     *
     * \since QGIS 4.2
     */
    void forceClose();

    void reject() override;

  protected:
    void closeEvent( QCloseEvent *e ) override;

    /**
     * Returns the widget's run button.
     */
    QPushButton *runButton();

    /**
     * Returns the widget's cancel button.
     */
    QPushButton *cancelButton();

    /**
     * Returns the widget's change parameters button.
     */
    QPushButton *changeParametersButton();

    /**
     * Returns the widget's button box.
     */
    QDialogButtonBox *buttonBox();

    /**
     * Returns the widget's tab widget.
     */
    QTabWidget *tabWidget();

    /**
     * Clears any current progress from the widget.
     */
    void clearProgress();

    /**
     * Sets whether the algorithm was executed through the widget.
     * \see wasExecuted()
     * \see setResults()
     */
    void setExecuted( bool executed );

    /**
     * Sets whether the algorithm was executed through the widget (no matter the result).
     */
    void setExecutedAnyResult( bool executedAnyResult );

    /**
     * Sets the algorithm results.
     * \see results()
     * \see setExecuted()
     */
    void setResults( const QVariantMap &results );

    /**
     * Displays an info \a message in the widget's log.
     */
    void setInfo( const QString &message, bool isError = false, bool escapeHtml = true, bool isWarning = false );

    /**
     * Resets the widget's GUI, ready for another algorithm execution.
     */
    void resetGui();

    /**
     * For subclasses to register their own GUI controls to be reset, ready
     * for another algorithm execution.
     */
    virtual void resetAdditionalGui();

    /**
     * Sets visibility for mutually exclusive buttons Run and Change Parameters.
     */
    void updateRunButtonVisibility();

    /**
     * Blocks run and changeParameters buttons and parameters tab while the
     * algorithm is running.
     */
    void blockControlsWhileRunning();

    /**
     * For subclasses to register their own GUI controls to be blocked while
     * the algorithm is running.
     */
    virtual void blockAdditionalControlsWhileRunning();

    /**
     * Returns the widget's message bar.
     */
    QgsMessageBar *messageBar();

    /**
     * Hides the short help panel.
     */
    void hideShortHelp();

    /**
     * Sets the current \a task running in the widget. The task will automatically be started
     * by the widget. Ownership of \a task is transferred to the widget.
     */
    void setCurrentTask( QgsProcessingAlgRunnerTask *task SIP_TRANSFER );

    /**
     * Formats an input \a string for display in the log tab.
     *
     */
    static QString formatStringForLog( const QString &string );

    /**
     * Returns TRUE if the widget is all finalized and can be safely deleted.
     *
     * \since QGIS 3.26
     */
    virtual bool isFinalized();

    /**
     * Applies any defined overrides for Processing context settings to the specified \a context.
     *
     * This allows the widget to override default Processing settings for an individual algorithm execution.
     *
     * \since QGIS 3.32
     */
    void applyContextOverrides( QgsProcessingContext *context );

  signals:

    /**
     * Emitted when the algorithm is about to run in the specified \a context.
     *
     * This signal can be used to tweak the \a context prior to the algorithm execution.
     *
     * \since QGIS 3.38
     */
    void algorithmAboutToRun( QgsProcessingContext *context );

    /**
     * Emitted whenever an algorithm has finished executing in the widget.
     *
     * \since QGIS 3.14
     */
    void algorithmFinished( bool successful, const QVariantMap &result );

    /**
     * Emitted when the widget requests that the algorithm is canceled.
     *
     * \note Not available in Python bindings
     * \since QGIS 4.2
     */
    SIP_SKIP void cancelRequested();

  protected slots:

    /**
     * Called when the algorithm has finished executing.
     */
    virtual void finished( bool successful, const QVariantMap &result, QgsProcessingContext &context, QgsProcessingFeedback *feedback );

    /**
     * Called when the widget's algorithm should be run. Must be overridden by subclasses.
     */
    virtual void runAlgorithm();

    /**
     * Called when an algorithm task has completed.
     *
     * \since QGIS 3.26
     */
    virtual void algExecuted( bool successful, const QVariantMap &results );

  private slots:

    void openHelp();
    void toggleCollapsed();

    void splitterChanged( int pos, int index );
    void mTabWidget_currentChanged( int index );
    void linkClicked( const QUrl &url );
    void taskTriggered( QgsTask *task );
    void closeClicked();
    void urlClicked( const QUrl &url );

  private:
    /**
     * Disconnects from the current task running in the widget.
     *
     * The task will continue to execute in the task manager, but the widget
     * will no longer respond to it.
     */
    void disconnectCurrentTask();

    WidgetMode mMode = WidgetMode::Single;

    QPushButton *mButtonRun = nullptr;
    QPushButton *mButtonClose = nullptr;
    QPushButton *mButtonChangeParameters = nullptr;
    QByteArray mSplitterState;
    QToolButton *mButtonCollapse = nullptr;
    QgsMessageBar *mMessageBar = nullptr;
    QPushButton *mAdvancedButton = nullptr;
    QMenu *mAdvancedMenu = nullptr;
    QAction *mCopyAsQgisProcessCommand = nullptr;
    QAction *mPasteJsonAction = nullptr;
    QAction *mContextSettingsAction = nullptr;

    bool mExecuted = false;
    bool mExecutedAnyResult = false;
    QVariantMap mResults;
    QgsPanelWidget *mMainWidget = nullptr;
    QgsProcessingFeedbackGenerator *mFeedbackFactory = nullptr;
    std::unique_ptr<QgsProcessingAlgorithm> mAlgorithm;
    QgsProcessingAlgRunnerTask *mAlgorithmTask = nullptr;

    bool mHelpCollapsed = false;

    int mMessageLoggedCount = 0;

    Qgis::ProcessingLogLevel mLogLevel = Qgis::ProcessingLogLevel::DefaultLevel;

    QPointer<QgsProcessingContextOptionsWidget> mContextOptionsWidget;
    bool mOverrideDefaultContextSettings = false;
    Qgis::InvalidGeometryCheck mGeometryCheck = Qgis::InvalidGeometryCheck::AbortOnInvalid;
    Qgis::DistanceUnit mDistanceUnits = Qgis::DistanceUnit::Unknown;
    Qgis::AreaUnit mAreaUnits = Qgis::AreaUnit::Unknown;
    QString mTemporaryFolderOverride;
    int mMaximumThreads = QThread::idealThreadCount();

    QString formatHelp( QgsProcessingAlgorithm *algorithm );
    void scrollToBottomOfLog();
    void processEvents();
};

#ifndef SIP_RUN

/**
 * \ingroup gui
 * \brief A modal dialog for showing algorithm progress and log messages.
 * \note This is not considered stable API and may change in future QGIS versions.
 */
class QgsProcessingAlgorithmProgressDialog : public QDialog, private Ui::QgsProcessingProgressDialogBase
{
    Q_OBJECT

  public:
    /**
     * Constructor for QgsProcessingAlgorithmProgressDialog.
     */
    QgsProcessingAlgorithmProgressDialog( QWidget *parent = nullptr );

    /**
     * Returns the dialog's progress bar.
     */
    QProgressBar *progressBar();

    /**
     * Returns the dialog's cancel button.
     */
    QPushButton *cancelButton();

    /**
     * Returns the dialog's text log.
     */
    QTextEdit *logTextEdit();

  public slots:

    void reject() override;
};

/**
 * \ingroup gui
 * \brief Widget for configuring settings for a Processing context.
 * \note Not stable API
 * \since QGIS 3.32
 */
class GUI_EXPORT QgsProcessingContextOptionsWidget : public QgsPanelWidget, private Ui::QgsProcessingContextOptionsBase
{
    Q_OBJECT

  public:
    /**
     * Constructor for QgsProcessingContextOptionsWidget, with the specified \a parent widget.
     */
    QgsProcessingContextOptionsWidget( QWidget *parent SIP_TRANSFERTHIS = nullptr );

    /**
     * Sets the widget state using the specified \a context.
     */
    void setFromContext( const QgsProcessingContext *context );

    /**
     * Returns the invalid geometry check selected in the widget.
     */
    Qgis::InvalidGeometryCheck invalidGeometryCheck() const;

    /**
     * Returns the distance unit selected in the widget.
     */
    Qgis::DistanceUnit distanceUnit() const;

    /**
     * Returns the area unit selected in the widget.
     */
    Qgis::AreaUnit areaUnit() const;

    /**
     * Returns the optional temporary folder override location.
     */
    QString temporaryFolder();

    /**
     * Returns the number of threads to use selected in the widget.
     */
    int maximumThreads() const;

    /**
     * Sets the log \a level to shown in the widget.
     *
     * \since QGIS 3.34
     */
    void setLogLevel( Qgis::ProcessingLogLevel level );

    /**
     * Returns the logging level selected in the widget.
     *
     * \since QGIS 3.34
     */
    Qgis::ProcessingLogLevel logLevel() const;
};

#endif

///@endcond

#endif // QGSPROCESSINGALGORITHMWIDGETBASE_H
