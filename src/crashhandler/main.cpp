/***************************************************************************
                            crssync.cpp
                            -------------------
   begin                : October 2017
   copyright            : (C) 2017 by Nathan Woodrow
   email                : woodrow.nathan@gmail.com
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include <memory>
#include <iostream>
#include <QFile>

#define _NO_CVCONST_H
#define _CRT_STDIO_ISO_WIDE_SPECIFIERS

#include <QApplication>
#include <QMainWindow>
#include "qgscrashdialog.h"
#include "qgsstacktrace.h"
#include "qgscrashreport.h"


int main( int argc, char *argv[] )
{
  if ( argc < 2 )
  {
    std::cout << "QGIS Crash Handler Usage: \n"
              << "qgiscrashhandler {infofile}" << std::endl;
    return -1;
  }

  QApplication app( argc, argv );
  QApplication::setQuitOnLastWindowClosed( true );
  QCoreApplication::setOrganizationName( "QGIS" );
  QCoreApplication::setApplicationName( "QGIS3" );

  QString extraInfoFile = QString( argv[1] );
  std::cout << "Extra Info File: " << extraInfoFile.toUtf8().constData() << std::endl;

  QFile file( extraInfoFile );
  QString processIdString;
  QString threadIdString;
  QString exceptionPointersString;
  QString symbolPaths;
  QString pythonCrashLogFile;
  QString reloadArgs;
  QStringList versionInfo;

  if ( file.open( QIODevice::ReadOnly | QIODevice::Text ) )
  {
    processIdString = file.readLine();
    threadIdString = file.readLine();
    exceptionPointersString = file.readLine();
    symbolPaths = file.readLine();
    pythonCrashLogFile = file.readLine().trimmed();
    reloadArgs = file.readLine();
    // The version info is the last stuff to be in the file until the end
    // bit gross but :)
    QString info = file.readAll();
    versionInfo = info.split( "\n" );
  }

#ifdef MSVC
  DWORD processId;
  DWORD threadId;
  LPEXCEPTION_POINTERS exception;
  processId = processIdString.toULong();
  threadId = threadIdString.toULong();
  sscanf_s( exceptionPointersString.toLocal8Bit().constData(), "%p", &exception );

  std::cout << "Process ID: " << processIdString.toLocal8Bit().constData() << std::endl;
  std::cout << "Thread ID:" << threadIdString.toLocal8Bit().constData() << std::endl;
  std::cout << "Exception Pointer: " << exceptionPointersString.toLocal8Bit().constData() << std::endl;
  std::cout << "Symbol Path :" << symbolPaths.toUtf8().constData() << std::endl;

  std::unique_ptr<QgsStackTrace> stackTrace( QgsStackTrace::trace( processId, threadId, exception, symbolPaths ) );
#endif

  QgsCrashReport report;
  report.setVersionInfo( versionInfo );
#ifdef MSVC
  report.setStackTrace( stackTrace.get() );
#endif
  report.setPythonCrashLogFilePath( pythonCrashLogFile );
  report.exportToCrashFolder();

  QgsCrashDialog dlg;
  dlg.setReloadArgs( reloadArgs );
  dlg.setPythonFault( report.pythonFault() );
  dlg.setBugReport( report.toHtml() );
  dlg.setModal( true );
  dlg.show();
  QApplication::exec();


#ifdef MSVC
  for ( HANDLE threadHandle : stackTrace->threads )
  {
    ResumeThread( threadHandle );
    CloseHandle( threadHandle );
  }
  CloseHandle( stackTrace->process );
#endif

  return 0;
}
