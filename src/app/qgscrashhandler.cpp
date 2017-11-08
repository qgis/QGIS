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

#ifdef _MSC_VER
LONG WINAPI QgsCrashHandler::handle( LPEXCEPTION_POINTERS exception )
{
  QgsDebugMsg( "CRASH!!!" );

  DWORD processID = GetCurrentProcessId();
  DWORD threadID = GetCurrentThreadId();

  QString symbolPath;
  if ( !QgsApplication::isRunningFromBuildDir() )
  {
    symbolPath = getenv( "QGIS_PREFIX_PATH" );
    symbolPath = symbolPath + "\\pdb;http://msdl.microsoft.com/download/symbols;http://download.osgeo.org/osgeo4w/symstore";
  }
  else
  {
    QString pdbPath = getenv( "QGIS_PDB_PATH" );
    QString appPath = QgsApplication::applicationDirPath();
    symbolPath += QString( "%1;%2;http://msdl.microsoft.com/download/symbols;http://download.osgeo.org/osgeo4w/symstore" )
                  .arg( appPath )
                  .arg( pdbPath );
  }

  QString ptrStr = QString( "0x%1" ).arg( ( quintptr )exception,
                                          QT_POINTER_SIZE * 2, 16, QChar( '0' ) );
  QString fileName = QStandardPaths::standardLocations( QStandardPaths::TempLocation ).at( 0 ) + "/qgis-crash-info-" + QString::number( processID );
  QgsDebugMsg( fileName );

  QStringList arguments;
  arguments = QCoreApplication::arguments();
  // TODO In future this needs to be moved out into a "session state" file because we can't trust this is valid in
  // a crash.
  arguments << QgsProject::instance()->fileName();

  QStringList reportData;
  reportData.append( QStringLiteral( "QGIS Version: %1" ).arg( Qgis::QGIS_VERSION ) );

  if ( QString( Qgis::QGIS_DEV_VERSION ) == QLatin1String( "exported" ) )
  {
    reportData.append( QStringLiteral( "QGIS code branch: Release %1.%2" )
                       .arg( Qgis::QGIS_VERSION_INT / 10000 ).arg( Qgis::QGIS_VERSION_INT / 100 % 100 ) );
  }
  else
  {
    reportData.append( QStringLiteral( "QGIS code revision: %1" ).arg( Qgis::QGIS_DEV_VERSION ) );
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
    stream << arguments.join( " " ) << endl;
    stream << reportData.join( "\n" ) << endl;
  }

  file.close();
  QStringList args;
  args << fileName;

  QString prefixPath( getenv( "QGIS_PREFIX_PATH" ) ? getenv( "QGIS_PREFIX_PATH" ) : QApplication::applicationDirPath() );
  QString path = prefixPath + "/qgiscrashhandler.exe";
  QgsDebugMsg( path );
  QProcess::execute( path, args );

  return TRUE;
}
#endif
