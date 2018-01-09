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

///@cond NOT_STABLE

class QgsProcessingAlgorithm;
class QToolButton;
class QgsProcessingAlgorithmDialogBase;
class QgsMessageBar;

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
    void errorReported( const QString &text );
    void infoPushed( const QString &text );
    void commandInfoPushed( const QString &text );
    void debugInfoPushed( const QString &text );
    void consoleInfoPushed( const QString &text );

  public slots:

    void setProgressText( const QString &text ) override;
    void reportError( const QString &error ) override;
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
class GUI_EXPORT QgsProcessingAlgorithmDialogBase : public QDialog, private Ui::QgsProcessingDialogBase
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsProcessingAlgorithmDialogBase.
     */
    QgsProcessingAlgorithmDialogBase( QWidget *parent = nullptr, Qt::WindowFlags flags = nullptr );

    /**
     * Sets the \a algorithm to run in the dialog.
     * \see algorithm()
     */
    void setAlgorithm( QgsProcessingAlgorithm *algorithm );

    /**
     * Returns the algorithm running in the dialog.
     * \see setAlgorithm()
     */
    QgsProcessingAlgorithm *algorithm();

    /**
     * Sets the main \a widget for the dialog, usually a panel for configuring algorithm parameters.
     * \see mainWidget()
     */
    void setMainWidget( QWidget *widget SIP_TRANSFER );

    /**
     * Returns the main widget for the dialog, usually a panel for configuring algorithm parameters.
     * \see setMainWidget()
     */
    QWidget *mainWidget();

    /**
     * Switches the dialog to the log page.
     */
    void showLog();

    /**
     * Returns true if an algorithm was executed in the dialog.
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
     * Returns the parameter values for the algorithm to run in the dialog.
     */
    virtual QVariantMap getParameterValues() const;

  public slots:

    void accept() override;

    /**
     * Reports an \a error string to the dialog's log.
     */
    void reportError( const QString &error );

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

  protected:

    /**
     * Returns the dialog's run button.
     */
    QPushButton *runButton();

    /**
     * Returns the dialog's cancel button.
     */
    QPushButton *cancelButton();

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
     * Sets the algorithm results.
     * \see results()
     * \see setExecuted()
     */
    void setResults( const QVariantMap &results );

    /**
     * Displays an info \a message in the dialog's log.
     */
    void setInfo( const QString &message, bool isError = false, bool escapeHtml = true );

    /**
     * Resets the dialog's gui, ready for another algorithm execution.
     */
    void resetGui();

    /**
     * Returns the dialog's message bar.
     */
    QgsMessageBar *messageBar();

    /**
     * Hides the short help panel.
     */
    void hideShortHelp();

  protected slots:

    /**
     * Called when the algorithm has finished executing.
     */
    virtual void finished( bool successful, const QVariantMap &result, QgsProcessingContext &context, QgsProcessingFeedback *feedback );

  private slots:

    void openHelp();
    void toggleCollapsed();

    void splitterChanged( int pos, int index );
    void linkClicked( const QUrl &url );

  private:

    QPushButton *mButtonRun = nullptr;
    QPushButton *mButtonClose = nullptr;
    QByteArray mSplitterState;
    QToolButton *mButtonCollapse = nullptr;
    QgsMessageBar *mMessageBar = nullptr;

    bool mExecuted = false;
    QVariantMap mResults;
    QWidget *mMainWidget = nullptr;
    QgsProcessingAlgorithm *mAlgorithm = nullptr;
    bool mHelpCollapsed = false;

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
