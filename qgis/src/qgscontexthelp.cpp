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

  mProcess = new QProcess(helpPath);
  QString arg1;
  arg1.setNum(contextId);
  mProcess->addArgument(arg1);
  mProcess->start();

  // Create socket to communicate with process
  mSocket = new QSocket(this);
  connect(mProcess, SIGNAL(readyReadStdout()), SLOT(readPort()));

  // Delete this object if the process terminates
  connect(mProcess, SIGNAL(processExited()), SLOT(processExited()));

  // Delete the process if the application quits
  connect(qApp, SIGNAL(aboutToQuit()), mProcess, SLOT(tryTerminate()));
}

QgsContextHelp::~QgsContextHelp()
{
  delete mSocket;
  delete mProcess;
}

void QgsContextHelp::readPort()
{
  // Get port and connect socket to process
  QString p = mProcess->readLineStdout();
  Q_UINT16 port = p.toUShort();
  mSocket->connectToHost("localhost", port);
  disconnect(mProcess, SIGNAL(readyReadStdout()), this, SLOT(readPort()));
}

void QgsContextHelp::showContext(int contextId)
{
  // Send context to process
  QTextStream os(mSocket);
  os << contextId << "\n";
#ifdef QGISDEBUG
  std::cout << "Sending help process context " << contextId << std::endl; 
#endif
}

void QgsContextHelp::processExited()
{
  // Delete this object if the process terminates
  delete gContextHelp;
  gContextHelp = NULL;
}
