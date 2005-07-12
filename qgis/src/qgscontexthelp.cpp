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
#include <iostream>
#include <qstring.h>
#include <qdir.h>
#include <qprocess.h>
#include <qsocket.h>
#include <qapplication.h>
#include "qgscontexthelp.h"

// Note: QGSCONTEXTHELP_REUSE must be defined (or not) in qgscontexthelp.h.
// The flag determines if an existing viewer process should be reused or
// terminated and restarted in order to make the viewer be the top window.

QgsContextHelp *QgsContextHelp::gContextHelp = NULL;  // Singleton instance

void QgsContextHelp::run(int contextId)
{
  if (gContextHelp == NULL)
  {
    // Create singleton instance if it does not exist
    gContextHelp = new QgsContextHelp(contextId);
  }
  else
  {
    gContextHelp->showContext(contextId);
  }
}

QgsContextHelp::QgsContextHelp(int contextId)
{
  mProcess = start(contextId);
#ifdef QGSCONTEXTHELP_REUSE
  // Create socket to communicate with process
  mSocket = new QSocket(this);
  connect(mProcess, SIGNAL(readyReadStdout()), SLOT(readPort()));
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

QProcess *QgsContextHelp::start(int contextId)
{
  // Assume minimum Qt 3.2 version and use the API to get the path
  // path to the help viewer
  QString helpPath = qApp->applicationDirPath(); 
#ifdef Q_OS_MACX
  helpPath += "/bin/qgis_help.app/Contents/MacOS";
#endif
  helpPath += "/qgis_help";
#ifdef QGISDEBUG
  std::cout << "Help path is " << helpPath.local8Bit() << std::endl; 
#endif

  QProcess *process = new QProcess(helpPath);
  QString arg1;
  arg1.setNum(contextId);
  process->addArgument(arg1);
  process->start();

  // Delete this object if the process terminates
  connect(process, SIGNAL(processExited()), SLOT(processExited()));

  // Delete the process if the application quits
  connect(qApp, SIGNAL(aboutToQuit()), process, SLOT(tryTerminate()));

  return process;
}

void QgsContextHelp::readPort()
{
#ifdef QGSCONTEXTHELP_REUSE
  // Get port and connect socket to process
  QString p = mProcess->readLineStdout();
  Q_UINT16 port = p.toUShort();
  mSocket->connectToHost("localhost", port);
  disconnect(mProcess, SIGNAL(readyReadStdout()), this, SLOT(readPort()));
#endif
}

void QgsContextHelp::showContext(int contextId)
{
  // Refresh help process with new context
#ifdef QGSCONTEXTHELP_REUSE
  // Send context to process
  QTextStream os(mSocket);
  os << contextId << "\n";
#ifdef QGISDEBUG
  std::cout << "Sending help process context " << contextId << std::endl; 
#endif
#else
  // Should be NULL here unless previous process termination failed
  // (if it did fail, we abandon the process and delete the object reference)
  delete mNextProcess;
  // Start new help viewer process (asynchronous)
  mNextProcess = start(contextId);
  // Terminate existing help viewer process (asynchronous)
  mProcess->tryTerminate();
#endif
}

void QgsContextHelp::processExited()
{
#ifndef QGSCONTEXTHELP_REUSE
  if (mNextProcess)
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
