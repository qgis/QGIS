/***************************************************************************
                          qgsrunprocess.cpp

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

#include "qgsrunprocess.h"

#include "qgslogger.h"
#include "qgsmessageoutput.h"
#include "qgsfeedback.h"
#include "qgsapplication.h"
#include "qgis.h"
#include <QProcess>
#include <QTextCodec>
#include <QMessageBox>
#include <QApplication>

#if QT_CONFIG(process)
QgsRunProcess::QgsRunProcess( const QString &action, bool capture )

{
  // Make up a string from the command and arguments that we'll use
  // for display purposes
  QgsDebugMsg( "Running command: " + action );

  mCommand = action;

#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
  QStringList arguments = QProcess::splitCommand( action );
  const QString command = arguments.value( 0 );
  if ( !arguments.isEmpty() )
    arguments.removeFirst();
#endif

  mProcess = new QProcess;

  if ( capture )
  {
    connect( mProcess, &QProcess::errorOccurred, this, &QgsRunProcess::processError );
    connect( mProcess, &QProcess::readyReadStandardOutput, this, &QgsRunProcess::stdoutAvailable );
    connect( mProcess, &QProcess::readyReadStandardError, this, &QgsRunProcess::stderrAvailable );
    // We only care if the process has finished if we are capturing
    // the output from the process, hence this connect() call is
    // inside the capture if() statement.
    connect( mProcess, static_cast < void ( QProcess::* )( int,  QProcess::ExitStatus ) >( &QProcess::finished ), this, &QgsRunProcess::processExit );

    // Use QgsMessageOutput for displaying output to user
    // It will delete itself when the dialog box is closed.
    mOutput = QgsMessageOutput::createMessageOutput();
    mOutput->setTitle( action );
    mOutput->setMessage( tr( "<b>Starting %1…</b>" ).arg( action ), QgsMessageOutput::MessageHtml );
    mOutput->showMessage( false ); // non-blocking

    // get notification of delete if it's derived from QObject
    QObject *mOutputObj = dynamic_cast<QObject *>( mOutput );
    if ( mOutputObj )
    {
      connect( mOutputObj, &QObject::destroyed, this, &QgsRunProcess::dialogGone );
    }

    // start the process!
#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
    mProcess->start( action );
#else
    mProcess->start( command, arguments );
#endif
  }
  else
  {
#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
    if ( ! mProcess->startDetached( action ) ) // let the program run by itself
#else
    if ( ! QProcess::startDetached( command, arguments ) ) // let the program run by itself
#endif
    {
      QMessageBox::critical( nullptr, tr( "Action" ),
                             tr( "Unable to run command\n%1" ).arg( action ),
                             QMessageBox::Ok, Qt::NoButton );
    }
    // We're not capturing the output from the process, so we don't
    // need to exist anymore.
    die();
  }
}

QgsRunProcess::~QgsRunProcess()
{
  delete mProcess;
}

void QgsRunProcess::die()
{
  // safe way to do "delete this" for QObjects
  deleteLater();
}

void QgsRunProcess::stdoutAvailable()
{
  const QByteArray bytes( mProcess->readAllStandardOutput() );
  QTextCodec *codec = QTextCodec::codecForLocale();
  const QString line( codec->toUnicode( bytes ) );

  // Add the new output to the dialog box
  mOutput->appendMessage( line );
}

void QgsRunProcess::stderrAvailable()
{
  const QByteArray bytes( mProcess->readAllStandardOutput() );
  QTextCodec *codec = QTextCodec::codecForLocale();
  const QString line( codec->toUnicode( bytes ) );

  // Add the new output to the dialog box, but color it red
  mOutput->appendMessage( "<font color=red>" + line + "</font>" );
}

void QgsRunProcess::processExit( int, QProcess::ExitStatus )
{
  // Because we catch the dialog box going (the dialogGone()
  // function), and delete this instance, control will only pass to
  // this function if the dialog box still exists when the process
  // exits, so it's always safe to use the pointer to the dialog box
  // (unless it was never created in the first case, which is what the
  // test against 0 is for).

  if ( mOutput )
  {
    mOutput->appendMessage( "<b>" + tr( "Done" ) + "</b>" );
  }

  // Since the dialog box takes care of deleting itself, and the
  // process has gone, there's no need for this instance to stay
  // around, so we disappear too.
  die();
}

void QgsRunProcess::dialogGone()
{
  // The dialog has gone, so the user is no longer interested in the
  // output from the process. Since the process will run happily
  // without the QProcess object, this instance and its data can then
  // go too, but disconnect the signals to prevent further functions in this
  // class being called after it has been deleted (Qt seems not to be
  // disconnecting them itself)

  mOutput = nullptr;

  disconnect( mProcess, &QProcess::errorOccurred, this, &QgsRunProcess::processError );
  disconnect( mProcess, &QProcess::readyReadStandardOutput, this, &QgsRunProcess::stdoutAvailable );
  disconnect( mProcess, &QProcess::readyReadStandardError, this, &QgsRunProcess::stderrAvailable );
  disconnect( mProcess, static_cast < void ( QProcess::* )( int, QProcess::ExitStatus ) >( &QProcess::finished ), this, &QgsRunProcess::processExit );

  die();
}

void QgsRunProcess::processError( QProcess::ProcessError err )
{
  if ( err == QProcess::FailedToStart )
  {
    QgsMessageOutput *output = mOutput ? mOutput : QgsMessageOutput::createMessageOutput();
    output->setMessage( tr( "Unable to run command %1" ).arg( mCommand ), QgsMessageOutput::MessageText );
    // Didn't work, so no need to hang around
    die();
  }
  else
  {
    QgsDebugMsg( "Got error: " + QString( "%d" ).arg( err ) );
  }
}

QStringList QgsRunProcess::splitCommand( const QString &command )
{
#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
  return QProcess::splitCommand( command );
#else
  // taken from Qt 5.15's implementation
  QStringList args;
  QString tmp;
  int quoteCount = 0;
  bool inQuote = false;

  // handle quoting. tokens can be surrounded by double quotes
  // "hello world". three consecutive double quotes represent
  // the quote character itself.
  for ( int i = 0; i < command.size(); ++i )
  {
    if ( command.at( i ) == QLatin1Char( '"' ) )
    {
      ++quoteCount;
      if ( quoteCount == 3 )
      {
        // third consecutive quote
        quoteCount = 0;
        tmp += command.at( i );
      }
      continue;
    }
    if ( quoteCount )
    {
      if ( quoteCount == 1 )
        inQuote = !inQuote;
      quoteCount = 0;
    }
    if ( !inQuote && command.at( i ).isSpace() )
    {
      if ( !tmp.isEmpty() )
      {
        args += tmp;
        tmp.clear();
      }
    }
    else
    {
      tmp += command.at( i );
    }
  }
  if ( !tmp.isEmpty() )
    args += tmp;

  return args;
#endif
}
#else
QgsRunProcess::QgsRunProcess( const QString &action, bool )
{
  Q_UNUSED( action )
  QgsDebugMsg( "Skipping command: " + action );
}

QgsRunProcess::~QgsRunProcess()
{
}

QStringList QgsRunProcess::splitCommand( const QString & )
{
  return QStringList();
}
#endif


//
// QgsBlockingProcess
//

#if QT_CONFIG(process)
QgsBlockingProcess::QgsBlockingProcess( const QString &process, const QStringList &arguments )
  : QObject()
  , mProcess( process )
  , mArguments( arguments )
{

}

int QgsBlockingProcess::run( QgsFeedback *feedback )
{
  const bool requestMadeFromMainThread = QThread::currentThread() == QCoreApplication::instance()->thread();

  int result = 0;
  QProcess::ExitStatus exitStatus = QProcess::NormalExit;
  QProcess::ProcessError error = QProcess::UnknownError;

  const std::function<void()> runFunction = [ this, &result, &exitStatus, &error, feedback]()
  {
    // this function will always be run in worker threads -- either the blocking call is being made in a worker thread,
    // or the blocking call has been made from the main thread and we've fired up a new thread for this function
    Q_ASSERT( QThread::currentThread() != QgsApplication::instance()->thread() );

    QProcess p;
    const QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    p.setProcessEnvironment( env );

    QEventLoop loop;
    // connecting to aboutToQuit avoids an on-going process to remain stalled
    // when QThreadPool::globalInstance()->waitForDone()
    // is called at process termination
    connect( qApp, &QCoreApplication::aboutToQuit, &loop, &QEventLoop::quit, Qt::DirectConnection );

    if ( feedback )
      QObject::connect( feedback, &QgsFeedback::canceled, &p, [ &p]
    {
#ifdef Q_OS_WIN
      // From the qt docs:
      // "Console applications on Windows that do not run an event loop, or whose
      // event loop does not handle the WM_CLOSE message, can only be terminated by calling kill()."
      p.kill();
#else
      p.terminate();
#endif
    } );
    connect( &p, qOverload< int, QProcess::ExitStatus >( &QProcess::finished ), this, [&loop, &result, &exitStatus]( int res, QProcess::ExitStatus st )
    {
      result = res;
      exitStatus = st;
      loop.quit();
    }, Qt::DirectConnection );

    connect( &p, &QProcess::readyReadStandardOutput, &p, [&p, this]
    {
      const QByteArray ba = p.readAllStandardOutput();
      mStdoutHandler( ba );
    } );
    connect( &p, &QProcess::readyReadStandardError, &p, [&p, this]
    {
      const QByteArray ba = p.readAllStandardError();
      mStderrHandler( ba );
    } );
    p.start( mProcess, mArguments, QProcess::Unbuffered | QProcess::ReadWrite );
    if ( !p.waitForStarted() )
    {
      result = 1;
      exitStatus = QProcess::NormalExit;
      error = p.error();
    }
    else
    {
      loop.exec();
    }

    mStdoutHandler( p.readAllStandardOutput() );
    mStderrHandler( p.readAllStandardError() );
  };

  if ( requestMadeFromMainThread )
  {
    std::unique_ptr<ProcessThread> processThread = std::make_unique<ProcessThread>( runFunction );
    processThread->start();
    // wait for thread to gracefully exit
    processThread->wait();
  }
  else
  {
    runFunction();
  }

  mExitStatus = exitStatus;
  mProcessError = error;
  return result;
}

QProcess::ExitStatus QgsBlockingProcess::exitStatus() const
{
  return mExitStatus;
};

QProcess::ProcessError QgsBlockingProcess::processError() const
{
  return mProcessError;
};
#endif // QT_CONFIG(process)
