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

#include <QString>
#include <QProcess>
#include <QTcpSocket>
#include <QTextStream>

#include "qgscontexthelp.h"
#include "qgsmessagelog.h"
#include "qgsapplication.h"
#include "qgslogger.h"


QgsContextHelp *QgsContextHelp::gContextHelp = 0;  // Singleton instance

void QgsContextHelp::run( QString context )
{
  if ( !gContextHelp )
  {
    // Create singleton instance if it does not exist
    gContextHelp = new QgsContextHelp();
  }

  gContextHelp->showContext( context );
}

QgsContextHelp::QgsContextHelp()
{
  mProcess = start();
}

QgsContextHelp::~QgsContextHelp()
{
  delete mProcess;
}

QProcess *QgsContextHelp::start()
{
  // Get the path to the help viewer
  QString helpPath = QgsApplication::helpAppPath();
  QgsDebugMsg( QString( "Help path is %1" ).arg( helpPath ) );

  QProcess *process = new QProcess;

  // Delete this object if the process terminates
  connect( process, SIGNAL( finished( int, QProcess::ExitStatus ) ), SLOT( processExited() ) );

  // Delete the process if the application quits
  connect( qApp, SIGNAL( aboutToQuit() ), process, SLOT( terminate() ) );

  connect( process, SIGNAL( error( QProcess::ProcessError ) ), this, SLOT( error( QProcess::ProcessError ) ) );

#ifdef Q_OS_WIN
  if ( QgsApplication::isRunningFromBuildDir() )
  {
    process->setEnvironment( QStringList() << QString( "PATH=%1;%2" ).arg( getenv( "PATH" ) ).arg( QApplication::applicationDirPath() ) );
  }
#endif

  process->start( helpPath, QStringList() );

  return process;
}

void QgsContextHelp::error( QProcess::ProcessError error )
{
  QgsMessageLog::logMessage( tr( "Error starting help viewer [%1]" ).arg( error ), tr( "Context help" ) );
}

void QgsContextHelp::showContext( QString context )
{
  init();

  QString helpContents = gContextHelpTexts.value( context,
                         tr( "<h3>Oops! QGIS can't find help for this form.</h3>"
                             "The help file for %1 was not found for your language<br>"
                             "If you would like to create it, contact the QGIS development team"
                           ).arg( context ) );

  QString myStyle = QgsApplication::reportStyleSheet();
  helpContents = "<head><style>" + myStyle + "</style></head><body>" + helpContents + "</body>\nEOH\n";

  mProcess->write( helpContents.toUtf8() );
}

void QgsContextHelp::processExited()
{
  // Delete this object if the process terminates
  delete gContextHelp;
  gContextHelp = NULL;
}
