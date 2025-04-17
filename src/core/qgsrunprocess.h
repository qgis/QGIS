/***************************************************************************
                          qgsrunprocess.h

 A class that runs an external program

                             -------------------
    begin                : Jan 2005
    copyright            : (C) 2005 by Gavin Macaulay
    email                : gavin at macaulay dot co dot nz
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSRUNPROCESS_H
#define QGSRUNPROCESS_H

#include <QObject>
#if QT_CONFIG(process)
#include <QProcess>
#endif

#include <QThread>

#include "qgis_core.h"
#include "qgis_sip.h"

class QgsFeedback;
class QgsMessageOutput;

/**
 * \ingroup core
 * \brief Executes an external program/script.
 *
 * It can optionally capture the standard output and error from the
 * process and displays them in a dialog box.
 *
 * On some platforms (e.g. iOS) , the process execution is skipped
 * https://lists.qt-project.org/pipermail/development/2015-July/022205.html
 */
class CORE_EXPORT QgsRunProcess: public QObject SIP_NODEFAULTCTORS
{
    Q_OBJECT

  public:
    // This class deletes itself, so to ensure that it is only created
    // using new, the Named Constructor Idiom is used, and one needs to
    // use the create() static function to get an instance of this class.

    // The action argument contains string with the command.
    // If capture is true, the standard output and error from the process
    // will be sent to QgsMessageOutput - usually a dialog box.
    static QgsRunProcess *create( const QString &action, bool capture ) SIP_FACTORY
    { return new QgsRunProcess( action, capture ); }

    /**
     * Splits the string \a command into a list of tokens, and returns
     * the list.
     *
     * Tokens with spaces can be surrounded by double quotes; three
     * consecutive double quotes represent the quote character itself.
     *
     * \since QGIS 3.18
     */
    static QStringList splitCommand( const QString &command );

  private:
    QgsRunProcess( const QString &action, bool capture ) SIP_FORCE;
    ~QgsRunProcess() override SIP_FORCE;

#if QT_CONFIG(process)
    // Deletes the instance of the class
    void die();

    std::unique_ptr<QProcess> mProcess;
    QgsMessageOutput *mOutput = nullptr;
    QString mCommand;

  public slots:
    void stdoutAvailable();
    void stderrAvailable();
    void processError( QProcess::ProcessError );
    void processExit( int, QProcess::ExitStatus );
    void dialogGone();
#endif // !(QT_CONFIG(process)
};

#if QT_CONFIG(process)

/**
 * \brief A thread safe class for performing blocking (sync) execution of external processes.
 *
 * This class should be used whenever a blocking process run is required. Unlike implementations
 * which rely on QApplication::processEvents() or creation of a QEventLoop, this class is completely
 * thread safe and can be used on either the main thread or background threads without issue.
 *
 * Not available on some platforms (e.g. iOS)
 * https://lists.qt-project.org/pipermail/development/2015-July/022205.html
 *
 * \ingroup core
 * \since QGIS 3.18
 */
class CORE_EXPORT QgsBlockingProcess : public QObject
{
    Q_OBJECT

  public:

    /**
     * Constructor for the given \a program, with the specified list of \a arguments.
     *
     * After construction, call run() to start the process execution.
     */
    QgsBlockingProcess( const QString &program, const QStringList &arguments );

#ifndef SIP_RUN

    /**
     * Sets a \a handler function to call whenever content is written by the process to stdout.
     */
    void setStdOutHandler( const std::function< void( const QByteArray & ) > &handler ) { mStdoutHandler = handler; }
#else

    /**
     * Sets a handler function to call whenever content is written by the process to stdout.
     */
    void setStdOutHandler( SIP_PYCALLABLE / AllowNone / );
    % MethodCode
    Py_BEGIN_ALLOW_THREADS

    sipCpp->setStdOutHandler( [a0]( const QByteArray &arg )
    {
      SIP_BLOCK_THREADS
      Py_XDECREF( sipCallMethod( NULL, a0, "D", &arg, sipType_QByteArray, NULL ) );
      SIP_UNBLOCK_THREADS
    } );

    Py_END_ALLOW_THREADS
    % End
#endif

#ifndef SIP_RUN

    /**
     * Sets a \a handler function to call whenever content is written by the process to stderr.
     */
    void setStdErrHandler( const std::function< void( const QByteArray & ) > &handler ) { mStderrHandler = handler; }
#else

    /**
     * Sets a \a handler function to call whenever content is written by the process to stderr.
     */
    void setStdErrHandler( SIP_PYCALLABLE / AllowNone / );
    % MethodCode
    Py_BEGIN_ALLOW_THREADS

    sipCpp->setStdErrHandler( [a0]( const QByteArray &arg )
    {
      SIP_BLOCK_THREADS
      Py_XDECREF( sipCallMethod( NULL, a0, "D", &arg, sipType_QByteArray, NULL ) );
      SIP_UNBLOCK_THREADS
    } );

    Py_END_ALLOW_THREADS
    % End
#endif

    /**
     * Runs the process, and blocks until execution finishes.
     *
     * The optional \a feedback argument can be used to specify a feedback object for cancellation/process termination.
     *
     * After execution completes, the process' result code will be returned.
     */
    int run( QgsFeedback *feedback = nullptr );

    /**
     * After a call to run(), returns the process' exit status.
     */
    QProcess::ExitStatus exitStatus() const;

    /**
     * After a call to run(), returns the process' reported error.
     *
     * Returns QProcess::UnknownError if no error occurred.
     */
    QProcess::ProcessError processError() const;

  private:

    QString mProcess;
    QStringList mArguments;
    std::function< void( const QByteArray & ) > mStdoutHandler;
    std::function< void( const QByteArray & ) > mStderrHandler;

    QProcess::ExitStatus mExitStatus = QProcess::NormalExit;
    QProcess::ProcessError mProcessError = QProcess::UnknownError;
};

#endif // QT_CONFIG(process)

///@cond PRIVATE
#ifndef SIP_RUN

class ProcessThread : public QThread
{
    Q_OBJECT

  public:
    ProcessThread( const std::function<void()> &function, QObject *parent = nullptr )
      : QThread( parent )
      , mFunction( function )
    {
    }

    void run() override
    {
      mFunction();
    }

  private:
    std::function<void()> mFunction;
};

#endif
///@endcond


#endif
