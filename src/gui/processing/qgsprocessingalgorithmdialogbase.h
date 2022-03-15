/***************************************************************************
                             qgsprocessingalgorithmdialogbase.h
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

#ifndef QGSPROCESSINGALGORITHMDIALOGBASE_H
#define QGSPROCESSINGALGORITHMDIALOGBASE_H

#include "qgis.h"
#include "qgis_gui.h"
#include "ui_qgsprocessingalgorithmdialogbase.h"
#include "ui_qgsprocessingalgorithmprogressdialogbase.h"
#include "processing/qgsprocessingcontext.h"
#include "processing/qgsprocessingfeedback.h"
#include "qgsprocessingwidgetwrapper.h"

///@cond NOT_STABLE

class QgsProcessingAlgorithm;
class QToolButton;
class QgsProcessingAlgorithmDialogBase;
class QgsMessageBar;
class QgsProcessingAlgRunnerTask;
class QgsTask;

#ifndef SIP_RUN

/**
 * \ingroup gui
 * \brief QgsProcessingFeedback subclass linked to a QgsProcessingAlgorithmDialogBase
 * \note Not stable API
 * \since QGIS 3.0
 */
class QgsProcessingAlgorithmDialogFeedback : public QgsProcessingFeedback
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsProcessingAlgorithmDialogFeedback.
     */
    QgsProcessingAlgorithmDialogFeedback();

  signals:

    void progressTextChanged( const QString &text );
    void errorReported( const QString &text, bool fatalError );
    void warningPushed( const QString &text );
    void infoPushed( const QString &text );
    void commandInfoPushed( const QString &text );
    void debugInfoPushed( const QString &text );
    void consoleInfoPushed( const QString &text );

  public slots:

    void setProgressText( const QString &text ) override;
    void reportError( const QString &error, bool fatalError ) override;
    void pushWarning( const QString &info ) override;
    void pushInfo( const QString &info ) override;
    void pushCommandInfo( const QString &info ) override;
    void pushDebugInfo( const QString &info ) override;
    void pushConsoleInfo( const QString &info ) override;

};
#endif

/**
 * \ingroup gui
 * \brief Base class for processing algorithm dialogs.
 * \note This is not considered stable API and may change in future QGIS versions.
 * \since QGIS 3.0
 */
class GUI_EXPORT QgsProcessingAlgorithmDialogBase : public QDialog, public QgsProcessingParametersGenerator, public QgsProcessingContextGenerator, private Ui::QgsProcessingDialogBase
{
    Q_OBJECT

  public:

    /**
     * Log format options.
     * \since QGIS 3.2
     */
    enum LogFormat
    {
      FormatPlainText, //!< Plain text file (.txt)
      FormatHtml, //!< HTML file (.html)
    };

    /**
     * Dialog modes.
     *
     * \since QGIS 3.24
     */
    enum class DialogMode : int
    {
      Single, //!< Single algorithm execution mode
      Batch, //!< Batch processing mode
    };
    Q_ENUM( QgsProcessingAlgorithmDialogBase::DialogMode )

    /**
     * Constructor for QgsProcessingAlgorithmDialogBase.
     */
    QgsProcessingAlgorithmDialogBase( QWidget *parent SIP_TRANSFERTHIS = nullptr, Qt::WindowFlags flags = Qt::WindowFlags(), QgsProcessingAlgorithmDialogBase::DialogMode mode = QgsProcessingAlgorithmDialogBase::DialogMode::Single );
    ~QgsProcessingAlgorithmDialogBase() override;

    /**
     * Sets the \a algorithm to run in the dialog.
     *
     * Ownership of the algorithm instance is transferred to the dialog.
     *
     * \see algorithm()
     */
    void setAlgorithm( QgsProcessingAlgorithm *algorithm SIP_TRANSFER );

    /**
     * Returns the algorithm running in the dialog.
     * \see setAlgorithm()
     */
    QgsProcessingAlgorithm *algorithm();

    /**
     * Sets the main \a widget for the dialog, usually a panel for configuring algorithm parameters.
     * \see mainWidget()
     */
    void setMainWidget( QgsPanelWidget *widget SIP_TRANSFER );

    /**
     * Returns the main widget for the dialog, usually a panel for configuring algorithm parameters.
     * \see setMainWidget()
     */
    QgsPanelWidget *mainWidget();

    /**
     * Switches the dialog to the log page.
     */
    void showLog();

    /**
     * Returns TRUE if an algorithm was executed in the dialog.
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
     * Creates a new processing feedback object, automatically connected to the appropriate
     * slots in this dialog.
     */
    QgsProcessingFeedback *createFeedback() SIP_FACTORY;

    /**
     * Saves the log contents to a text file (specified by the file \a path), in
     * the given \a format.
     * \see saveLog()
     * \since QGIS 3.2
     */
    void saveLogToFile( const QString &path, LogFormat format = FormatPlainText );

    /**
     * Returns the logging level to use when running algorithms from the dialog.
     *
     * \see setLogLevel()
     * \since QGIS 3.20
     */
    QgsProcessingContext::LogLevel logLevel() const;

    /**
     * Sets the logging \a level to use when running algorithms from the dialog.
     *
     * \see logLevel()
     * \since QGIS 3.20
     */
    void setLogLevel( QgsProcessingContext::LogLevel level );

    /**
     * Sets the parameter \a values to show in the dialog.
     *
     * \since QGIS 3.24
     */
    virtual void setParameters( const QVariantMap &values );

  public slots:

    /**
     * Reports an \a error string to the dialog's log.
     *
     * If \a fatalError is TRUE, the error prevented the algorithm from executing.
     */
    void reportError( const QString &error, bool fatalError );

    /**
     * Pushes a warning information string to the dialog's log.
     */
    void pushWarning( const QString &warning );

    /**
     * Pushes an information string to the dialog's log.
     */
    void pushInfo( const QString &info );

    /**
     * Pushes a debug info string to the dialog's log.
     */
    void pushDebugInfo( const QString &message );

    /**
     * Pushes command info to the dialog's log.
     */
    void pushCommandInfo( const QString &info );

    /**
     * Sets the percentage progress for the dialog, between 0 and 100.
     */
    void setPercentage( double percent );

    /**
     * Sets a progress text message.
     */
    void setProgressText( const QString &text );

    /**
     * Pushes a console info string to the dialog's log.
     */
    void pushConsoleInfo( const QString &info );

    /**
     * Creates a modal progress dialog showing progress and log messages
     * from this dialog.
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
     * Switches the dialog to the parameters page.
     */
    void showParameters();

    void reject() override;

  protected:

    void closeEvent( QCloseEvent *e ) override;

    /**
     * Returns the dialog's run button.
     */
    QPushButton *runButton();

    /**
     * Returns the dialog's cancel button.
     */
    QPushButton *cancelButton();

    /**
     * Returns the dialog's change parameters button.
     */
    QPushButton *changeParametersButton();

    /**
     * Returns the dialog's button box.
     */
    QDialogButtonBox *buttonBox();

    /**
     * Returns the dialog's tab widget.
     */
    QTabWidget *tabWidget();

    /**
     * Clears any current progress from the dialog.
     */
    void clearProgress();

    /**
     * Sets whether the algorithm was executed through the dialog.
     * \see wasExecuted()
     * \see setResults()
     */
    void setExecuted( bool executed );

    /**
     * Sets whether the algorithm was executed through the dialog (no matter the result).
     */
    void setExecutedAnyResult( bool executedAnyResult );

    /**
     * Sets the algorithm results.
     * \see results()
     * \see setExecuted()
     */
    void setResults( const QVariantMap &results );

    /**
     * Displays an info \a message in the dialog's log.
     */
    void setInfo( const QString &message, bool isError = false, bool escapeHtml = true, bool isWarning = false );

    /**
     * Resets the dialog's gui, ready for another algorithm execution.
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
     * Returns the dialog's message bar.
     */
    QgsMessageBar *messageBar();

    /**
     * Hides the short help panel.
     */
    void hideShortHelp();

    /**
     * Sets the current \a task running in the dialog. The task will automatically be started
     * by the dialog. Ownership of \a task is transferred to the dialog.
     */
    void setCurrentTask( QgsProcessingAlgRunnerTask *task SIP_TRANSFER );

    /**
     * Formats an input \a string for display in the log tab.
     *
     * \since QGIS 3.0.1
     */
    static QString formatStringForLog( const QString &string );

    /**
     * Returns TRUE if the dialog is all finalized and can be safely deleted.
     *
     * \since QGIS 3.26
     */
    virtual bool isFinalized();

  signals:

    /**
     * Emitted whenever an algorithm has finished executing in the dialog.
     *
     * \since QGIS 3.14
     */
    void algorithmFinished( bool successful, const QVariantMap &result );

  protected slots:

    /**
     * Called when the algorithm has finished executing.
     */
    virtual void finished( bool successful, const QVariantMap &result, QgsProcessingContext &context, QgsProcessingFeedback *feedback );

    /**
     * Called when the dialog's algorithm should be run. Must be overridden by subclasses.
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

  private:

    DialogMode mMode = DialogMode::Single;

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

    bool mExecuted = false;
    bool mExecutedAnyResult = false;
    QVariantMap mResults;
    QgsPanelWidget *mMainWidget = nullptr;
    std::unique_ptr< QgsProcessingAlgorithm > mAlgorithm;
    QgsProcessingAlgRunnerTask *mAlgorithmTask = nullptr;

    bool mHelpCollapsed = false;

    int mMessageLoggedCount = 0;

    QgsProcessingContext::LogLevel mLogLevel = QgsProcessingContext::DefaultLevel;

    QString formatHelp( QgsProcessingAlgorithm *algorithm );
    void scrollToBottomOfLog();
    void processEvents();

};

#ifndef SIP_RUN

/**
 * \ingroup gui
 * \brief A modal dialog for showing algorithm progress and log messages.
 * \note This is not considered stable API and may change in future QGIS versions.
 * \since QGIS 3.0
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

#endif

///@endcond

#endif // QGSPROCESSINGALGORITHMDIALOGBASE_H
