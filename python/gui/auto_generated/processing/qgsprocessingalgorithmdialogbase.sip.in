/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/processing/qgsprocessingalgorithmdialogbase.h                *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/






class QgsProcessingAlgorithmDialogBase : QDialog
{
%Docstring
Base class for processing algorithm dialogs.

.. note::

   This is not considered stable API and may change in future QGIS versions.

.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgsprocessingalgorithmdialogbase.h"
%End
  public:

    enum LogFormat
    {
      FormatPlainText,
      FormatHtml,
    };

    QgsProcessingAlgorithmDialogBase( QWidget *parent /TransferThis/ = 0, Qt::WindowFlags flags = 0 );
%Docstring
Constructor for QgsProcessingAlgorithmDialogBase.
%End
    ~QgsProcessingAlgorithmDialogBase();

    void setAlgorithm( QgsProcessingAlgorithm *algorithm /Transfer/ );
%Docstring
Sets the ``algorithm`` to run in the dialog.

Ownership of the algorithm instance is transferred to the dialog.

.. seealso:: :py:func:`algorithm`
%End

    QgsProcessingAlgorithm *algorithm();
%Docstring
Returns the algorithm running in the dialog.

.. seealso:: :py:func:`setAlgorithm`
%End

    void setMainWidget( QWidget *widget /Transfer/ );
%Docstring
Sets the main ``widget`` for the dialog, usually a panel for configuring algorithm parameters.

.. seealso:: :py:func:`mainWidget`
%End

    QWidget *mainWidget();
%Docstring
Returns the main widget for the dialog, usually a panel for configuring algorithm parameters.

.. seealso:: :py:func:`setMainWidget`
%End

    void showLog();
%Docstring
Switches the dialog to the log page.
%End

    bool wasExecuted() const;
%Docstring
Returns ``True`` if an algorithm was executed in the dialog.

.. seealso:: :py:func:`results`

.. seealso:: :py:func:`setExecuted`
%End

    QVariantMap results() const;
%Docstring
Returns the results returned by the algorithm executed.

.. seealso:: :py:func:`wasExecuted`

.. seealso:: :py:func:`setResults`
%End

    QgsProcessingFeedback *createFeedback() /Factory/;
%Docstring
Creates a new processing feedback object, automatically connected to the appropriate
slots in this dialog.
%End

    virtual QVariantMap getParameterValues() const;
%Docstring
Returns the parameter values for the algorithm to run in the dialog.
%End

    void saveLogToFile( const QString &path, LogFormat format = FormatPlainText );
%Docstring
Saves the log contents to a text file (specified by the file ``path``), in
the given ``format``.

.. seealso:: :py:func:`saveLog`

.. versionadded:: 3.2
%End

  public slots:

    void reportError( const QString &error, bool fatalError );
%Docstring
Reports an ``error`` string to the dialog's log.

If ``fatalError`` is ``True``, the error prevented the algorithm from executing.
%End

    void pushInfo( const QString &info );
%Docstring
Pushes an information string to the dialog's log.
%End

    void pushDebugInfo( const QString &message );
%Docstring
Pushes a debug info string to the dialog's log.
%End

    void pushCommandInfo( const QString &info );
%Docstring
Pushes command info to the dialog's log.
%End

    void setPercentage( double percent );
%Docstring
Sets the percentage progress for the dialog, between 0 and 100.
%End

    void setProgressText( const QString &text );
%Docstring
Sets a progress text message.
%End

    void pushConsoleInfo( const QString &info );
%Docstring
Pushes a console info string to the dialog's log.
%End

    QDialog *createProgressDialog();
%Docstring
Creates a modal progress dialog showing progress and log messages
from this dialog.
%End

    void clearLog();
%Docstring
Clears the current log contents.

.. versionadded:: 3.2
%End

    void saveLog();
%Docstring
Opens a dialog allowing users to save the current log contents.

.. seealso:: :py:func:`saveLogToFile`

.. versionadded:: 3.2
%End

    void copyLogToClipboard();
%Docstring
Copies the current log contents to the clipboard.

.. versionadded:: 3.2
%End

  protected:

    virtual void closeEvent( QCloseEvent *e );


    QPushButton *runButton();
%Docstring
Returns the dialog's run button.
%End

    QPushButton *cancelButton();
%Docstring
Returns the dialog's cancel button.
%End

    QDialogButtonBox *buttonBox();
%Docstring
Returns the dialog's button box.
%End

    QTabWidget *tabWidget();
%Docstring
Returns the dialog's tab widget.
%End

    void clearProgress();
%Docstring
Clears any current progress from the dialog.
%End

    void setExecuted( bool executed );
%Docstring
Sets whether the algorithm was executed through the dialog.

.. seealso:: :py:func:`wasExecuted`

.. seealso:: :py:func:`setResults`
%End

    void setResults( const QVariantMap &results );
%Docstring
Sets the algorithm results.

.. seealso:: :py:func:`results`

.. seealso:: :py:func:`setExecuted`
%End

    void setInfo( const QString &message, bool isError = false, bool escapeHtml = true );
%Docstring
Displays an info ``message`` in the dialog's log.
%End

    void resetGui();
%Docstring
Resets the dialog's gui, ready for another algorithm execution.
%End

    QgsMessageBar *messageBar();
%Docstring
Returns the dialog's message bar.
%End

    void hideShortHelp();
%Docstring
Hides the short help panel.
%End

    void setCurrentTask( QgsProcessingAlgRunnerTask *task /Transfer/ );
%Docstring
Sets the current ``task`` running in the dialog. The task will automatically be started
by the dialog. Ownership of ``task`` is transferred to the dialog.
%End

    static QString formatStringForLog( const QString &string );
%Docstring
Formats an input ``string`` for display in the log tab.

.. versionadded:: 3.0.1
%End

  protected slots:

    virtual void finished( bool successful, const QVariantMap &result, QgsProcessingContext &context, QgsProcessingFeedback *feedback );
%Docstring
Called when the algorithm has finished executing.
%End

    virtual void runAlgorithm();
%Docstring
Called when the dialog's algorithm should be run. Must be overridden by subclasses.
%End

};



/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/processing/qgsprocessingalgorithmdialogbase.h                *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
