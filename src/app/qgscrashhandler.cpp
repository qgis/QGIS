/***************************************************************************
  qgscrashhandler.cpp - QgsCrashHandler

 ---------------------
 begin                : 23.4.2017
 copyright            : (C) 2017 by Nathan Woodrow
 email                : woodrow.nathan@gmail.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgscrashhandler.h"

#include <QProcess>
#include <QDir>

#include "qgsproject.h"
#include "qgscrashdialog.h"
#include "qgscrashreport.h"
#include "qgsstacktrace.h"

#ifdef Q_OS_WIN
LONG WINAPI QgsCrashHandler::handle( struct _EXCEPTION_POINTERS *ExceptionInfo )
{
  QgsStackLines stack = QgsStackTrace::trace( ExceptionInfo );
  showCrashDialog( stack );

  return EXCEPTION_EXECUTE_HANDLER;
}
#endif

void QgsCrashHandler::showCrashDialog( const QgsStackLines &stack )
{

  QgsCrashDialog dlg( QApplication::activeWindow() );
  QgsCrashReport report;
  report.setStackTrace( stack );
  dlg.setBugReport( report.toString() );
  if ( dlg.exec() )
  {
    restartApplication();
  }
}

void QgsCrashHandler::restartApplication()
{
  QStringList arguments;
  arguments = QCoreApplication::arguments();
  QString path = arguments.at( 0 );
  arguments.removeFirst();
  arguments << QgsProject::instance()->fileName();
  QProcess::startDetached( path, arguments, QDir::toNativeSeparators( QCoreApplication::applicationDirPath() ) );

}
