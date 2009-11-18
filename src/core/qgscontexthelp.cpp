/***************************************************************************
                          qgscontexthelp.cpp
                    Display context help for a dialog
                             -------------------
    begin                : 2005-06-19
    copyright            : (C) 2005 by Gary E.Sherman
    email                : sherman at mrcc.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
/* $Id$ */

#include <QString>
#include <QProcess>
#include <QTcpSocket>
#include <QTextStream>

#include "qgscontexthelp.h"
#include "qgsapplication.h"
#include "qgslogger.h"


// Note: QGSCONTEXTHELP_REUSE must be defined (or not) in qgscontexthelp.h.
// The flag determines if an existing viewer process should be reused or
// terminated and restarted in order to make the viewer be the top window.

QgsContextHelp *QgsContextHelp::gContextHelp = NULL;  // Singleton instance

void QgsContextHelp::run( QString context )
{
  if ( gContextHelp == NULL )
  {
    // Create singleton instance if it does not exist
    gContextHelp = new QgsContextHelp( context );
  }
  else
  {
    gContextHelp->showContext( context );
  }
}

QgsContextHelp::QgsContextHelp( QString context )
{
  mProcess = start( context );
#ifdef QGSCONTEXTHELP_REUSE
  // Create socket to communicate with process
  mSocket = new QTcpSocket( this );
  connect( mProcess, SIGNAL( readyReadStandardOutput() ), SLOT( readPort() ) );
#else
  // Placeholder for new process if terminating and restarting
  mNextProcess = NULL;
#endif
}

QgsContextHelp::~QgsContextHelp()
{
#ifdef QGSCONTEXTHELP_REUSE
  delete mSocket;
#else
  // Should be NULL here unless previous process termination failed
  delete mNextProcess;
#endif
  delete mProcess;
}

QProcess *QgsContextHelp::start( QString context )
{
  // Get the path to the help viewer
  QString helpPath = QgsApplication::helpAppPath();
  QgsDebugMsg( QString( "Help path is %1" ).arg( helpPath ) );

  QProcess *process = new QProcess;
  process->start( helpPath, QStringList( context ) );

  // Delete this object if the process terminates
  connect( process, SIGNAL( finished( int, QProcess::ExitStatus ) ), SLOT( processExited() ) );

  // Delete the process if the application quits
  connect( qApp, SIGNAL( aboutToQuit() ), process, SLOT( terminate() ) );

  return process;
}

void QgsContextHelp::readPort()
{
#ifdef QGSCONTEXTHELP_REUSE
  // Get port and connect socket to process
  QString p = mProcess->readAllStandardOutput();
  quint16 port = p.toUShort();
  mSocket->connectToHost( "localhost", port );
  disconnect( mProcess, SIGNAL( readyReadStandardOutput() ), this, SLOT( readPort() ) );
#endif
}

void QgsContextHelp::showContext( QString context )
{
  // Refresh help process with new context
#ifdef QGSCONTEXTHELP_REUSE
  // Send context to process
  QTextStream os( mSocket );
  os << contextId << "\n";
  QgsDebugMsg( QString( "Sending help process context %1" ).arg( contextId ) );
#else
  // Should be NULL here unless previous process termination failed
  // (if it did fail, we abandon the process and delete the object reference)
  delete mNextProcess;
  // Start new help viewer process (asynchronous)
  mNextProcess = start( context );
  // Terminate existing help viewer process (asynchronous)
  mProcess->terminate();
#endif
}

void QgsContextHelp::processExited()
{
#ifndef QGSCONTEXTHELP_REUSE
  if ( mNextProcess )
  {
    // New process becomes current process when prior process terminates
    delete mProcess;
    mProcess = mNextProcess;
    mNextProcess = NULL;
  }
  else
#endif
  {
    // Delete this object if the process terminates
    delete gContextHelp;
    gContextHelp = NULL;
  }
}
