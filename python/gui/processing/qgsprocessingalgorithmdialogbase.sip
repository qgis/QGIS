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

    QgsProcessingAlgorithmDialogBase( QWidget *parent = 0, Qt::WindowFlags flags = 0 );
%Docstring
 Constructor for QgsProcessingAlgorithmDialogBase.
%End

    void setAlgorithm( QgsProcessingAlgorithm *algorithm );
%Docstring
 Sets the ``algorithm`` to run in the dialog.
.. seealso:: :py:func:`algorithm()`
%End

    QgsProcessingAlgorithm *algorithm();
%Docstring
 Returns the algorithm running in the dialog.
.. seealso:: :py:func:`setAlgorithm()`
 :rtype: QgsProcessingAlgorithm
%End

    void setMainWidget( QWidget *widget /Transfer/ );
%Docstring
 Sets the main ``widget`` for the dialog, usually a panel for configuring algorithm parameters.
.. seealso:: :py:func:`mainWidget()`
%End

    QWidget *mainWidget();
%Docstring
 Returns the main widget for the dialog, usually a panel for configuring algorithm parameters.
.. seealso:: :py:func:`setMainWidget()`
 :rtype: QWidget
%End

    void showLog();
%Docstring
 Switches the dialog to the log page.
%End

    bool wasExecuted() const;
%Docstring
 Returns true if an algorithm was executed in the dialog.
 :rtype: bool
%End

    QgsProcessingFeedback *createFeedback() /Factory/;
%Docstring
 Creates a new processing feedback object, automatically connected to the appropriate
 slots in this dialog.
 :rtype: QgsProcessingFeedback
%End

    virtual QVariantMap getParameterValues() const;
%Docstring
 Returns the parameter values for the algorithm to run in the dialog.
 :rtype: QVariantMap
%End

  public slots:

    virtual void accept();

    virtual void reject();


    void reportError( const QString &error );
%Docstring
 Reports an ``error`` string to the dialog's log.
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

  protected:

    virtual void closeEvent( QCloseEvent *e );


    QPushButton *runButton();
%Docstring
 Returns the dialog's run button.
 :rtype: QPushButton
%End

    QPushButton *cancelButton();
%Docstring
 Returns the dialog's cancel button.
 :rtype: QPushButton
%End

    QDialogButtonBox *buttonBox();
%Docstring
 Returns the dialog's button box.
 :rtype: QDialogButtonBox
%End

    QTabWidget *tabWidget();
%Docstring
 Returns the dialog's tab widget.
 :rtype: QTabWidget
%End

    void clearProgress();
%Docstring
 Clears any current progress from the dialog.
%End

    void setExecuted( bool executed );
%Docstring
 Sets whether the algorithm was executed through the dialog.
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
 :rtype: QgsMessageBar
%End

    void hideShortHelp();
%Docstring
 Hides the short help panel.
%End

  protected slots:

    virtual void finished( bool successful, const QVariantMap &result, QgsProcessingContext &context, QgsProcessingFeedback *feedback );
%Docstring
 Called when the algorithm has finished executing.
%End

};


/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/processing/qgsprocessingalgorithmdialogbase.h                *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
