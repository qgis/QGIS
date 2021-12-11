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

#include <qgslogger.h>
#include <iostream>
#include "qgscrashhandler.h"
#include "qgsapplication.h"
#include "qgsproject.h"

#include <gdal.h>

#include <QTextStream>
#include <QProcess>
#include <QDir>
#include <QStandardPaths>
#include <QUuid>

QString QgsCrashHandler::sPythonCrashLogFile;

#ifdef _MSC_VER
LONG WINAPI QgsCrashHandler::handle( LPEXCEPTION_POINTERS exception )
{
  DWORD processID = GetCurrentProcessId();
  DWORD threadID = GetCurrentThreadId();

  QString symbolPath;
  if ( !QgsApplication::isRunningFromBuildDir() )
  {
    symbolPath = QStringLiteral( "%1\\pdb;http://msdl.microsoft.com/download/symbols;http://download.osgeo.org/osgeo4w/%2/symstores/%3" )
                 .arg( getenv( "QGIS_PREFIX_PATH" ) )
                 .arg( QSysInfo::WordSize == 64 ? QStringLiteral( "x86_64" ) : QStringLiteral( "x86" ) )
                 .arg( QFileInfo( getenv( "QGIS_PREFIX_PATH" ) ).baseName() );
  }
  else
  {
    symbolPath = QStringLiteral( "%1;%2;http://msdl.microsoft.com/download/symbols" )
                 .arg( getenv( "QGIS_PDB_PATH" ) )
                 .arg( QgsApplication::applicationDirPath() );
  }

  QString ptrStr = QString( "0x%1" ).arg( ( quintptr )exception, QT_POINTER_SIZE * 2, 16, QChar( '0' ) );

  handleCrash( processID, threadID, symbolPath, ptrStr );
  return TRUE;
}
#else
void QgsCrashHandler::handle( int )
{
  handleCrash( QCoreApplication::applicationPid(), 0, QString(), QString() );
}
#endif

void QgsCrashHandler::handleCrash( int processID, int threadID,
                                   const QString &symbolPath,
                                   const QString &ptrStr )
{
  QString fileName = QStandardPaths::standardLocations( QStandardPaths::TempLocation ).at( 0 ) + "/qgis-crash-info-" + QString::number( processID );
  QgsDebugMsg( fileName );

  QStringList arguments;
  arguments = QCoreApplication::arguments();
  // TODO In future this needs to be moved out into a "session state" file because we can't trust this is valid in
  // a crash.
  QString projectFile = QgsProject::instance()->fileName();
  if ( !projectFile.isEmpty() )
    // quote project file path to avoid issues if it has spaces
    arguments << QStringLiteral( "\"%1\"" ).arg( projectFile );

  QStringList reportData;
  reportData.append( QStringLiteral( "QGIS Version: %1" ).arg( Qgis::version() ) );

  if ( QString( Qgis::devVersion() ) == QLatin1String( "exported" ) )
  {
    reportData.append( QStringLiteral( "QGIS code branch: Release %1.%2" )
                       .arg( Qgis::versionInt() / 10000 ).arg( Qgis::versionInt() / 100 % 100 ) );
  }
  else
  {
    reportData.append( QStringLiteral( "QGIS code revision: %1" ).arg( Qgis::devVersion() ) );
  }

  reportData.append( QStringLiteral( "Compiled against Qt: %1" ).arg( QT_VERSION_STR ) );
  reportData.append( QStringLiteral( "Running against Qt: %1" ).arg( qVersion() ) );

  reportData.append( QStringLiteral( "Compiled against GDAL: %1" ).arg( GDAL_RELEASE_NAME ) );
  reportData.append( QStringLiteral( "Running against GDAL: %1" ).arg( GDALVersionInfo( "RELEASE_NAME" ) ) );

  QFile file( fileName );
  if ( file.open( QIODevice::WriteOnly | QIODevice::Text ) )
  {
    QTextStream stream( &file );
    stream << QString::number( processID ) << endl;
    stream << QString::number( threadID ) << endl;
    stream << ptrStr << endl;
    stream << symbolPath << endl;
    stream << sPythonCrashLogFile << endl;
    stream << arguments.join( ' ' ) << endl;
    stream << reportData.join( '\n' ) << endl;
  }

  file.close();
  QStringList args;
  args << fileName;

  QString prefixPath( getenv( "QGIS_PREFIX_PATH" ) ? getenv( "QGIS_PREFIX_PATH" ) : QApplication::applicationDirPath() );
#ifdef MSVC
  QString path = prefixPath + QStringLiteral( "/qgiscrashhandler.exe" );
#else
  QString path = prefixPath + QStringLiteral( "/qgiscrashhandler" );
#endif
  QgsDebugMsg( path );
  QProcess::execute( path, args );
}
