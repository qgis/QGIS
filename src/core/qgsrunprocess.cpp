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
#include <QProcess>
#include <QMessageBox>

QgsRunProcess::QgsRunProcess( const QString& action, bool capture )
    : mProcess( NULL ), mOutput( NULL )
{
  // Make up a string from the command and arguments that we'll use
  // for display purposes
  QgsDebugMsg( "Running command: " + action );

  mCommand = action;

  mProcess = new QProcess;

  if ( capture )
  {
    connect( mProcess, SIGNAL( error( QProcess::ProcessError ) ), this, SLOT( processError( QProcess::ProcessError ) ) );
    connect( mProcess, SIGNAL( readyReadStandardOutput() ), this, SLOT( stdoutAvailable() ) );
    connect( mProcess, SIGNAL( readyReadStandardError() ), this, SLOT( stderrAvailable() ) );
    // We only care if the process has finished if we are capturing
    // the output from the process, hence this connect() call is
    // inside the capture if() statement.
    connect( mProcess, SIGNAL( finished( int, QProcess::ExitStatus ) ), this, SLOT( processExit( int, QProcess::ExitStatus ) ) );

    // Use QgsMessageOutput for displaying output to user
    // It will delete itself when the dialog box is closed.
    mOutput = QgsMessageOutput::createMessageOutput();
    mOutput->setTitle( action );
    mOutput->setMessage( tr( "<b>Starting %1...</b>" ).arg( action ), QgsMessageOutput::MessageHtml );
    mOutput->showMessage( false ); // non-blocking

    // get notification of delete if it's derived from QObject
    QObject* mOutputObj = dynamic_cast<QObject *>( mOutput );
    if ( mOutputObj )
    {
      connect( mOutputObj, SIGNAL( destroyed() ), this, SLOT( dialogGone() ) );
    }

    // start the process!
    mProcess->start( action );
  }
  else
  {
    if ( ! mProcess->startDetached( action ) ) // let the program run by itself
    {
      QMessageBox::critical( 0, tr( "Action" ),
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
  QString line( mProcess->readAllStandardOutput() );

  // Add the new output to the dialog box
  mOutput->appendMessage( line );
}

void QgsRunProcess::stderrAvailable()
{
  QString line( mProcess->readAllStandardError() );

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

  if ( mOutput != 0 )
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

  mOutput = 0;

  disconnect( mProcess, SIGNAL( error( QProcess::ProcessError ) ), this, SLOT( processError( QProcess::ProcessError ) ) );
  disconnect( mProcess, SIGNAL( readyReadStandardOutput() ), this, SLOT( stdoutAvailable() ) );
  disconnect( mProcess, SIGNAL( readyReadStandardError() ), this, SLOT( stderrAvailable() ) );
  disconnect( mProcess, SIGNAL( finished( int, QProcess::ExitStatus ) ), this, SLOT( processExit( int, QProcess::ExitStatus ) ) );

  die();
}

void QgsRunProcess::processError( QProcess::ProcessError err )
{
  if ( err == QProcess::FailedToStart )
  {
    QgsMessageOutput* output = mOutput ? mOutput : QgsMessageOutput::createMessageOutput();
    output->setMessage( tr( "Unable to run command %1" ).arg( mCommand ), QgsMessageOutput::MessageText );
    // Didn't work, so no need to hang around
    die();
  }
  else
  {
    QgsDebugMsg( "Got error: " + QString( "%d" ).arg( err ) );
  }
}
