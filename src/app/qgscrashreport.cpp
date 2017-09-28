/***************************************************************************
  qgscrashreport.cpp - QgsCrashReport

 ---------------------
 begin                : 16.4.2017
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
#include "qgscrashreport.h"

#include <QSysInfo>
#include <QFileInfo>
#include <QCryptographicHash>

#include "qgis.h"
#include "gdal_version.h"
#include "ogr_core.h"

QgsCrashReport::QgsCrashReport()
{
  setFlags( QgsCrashReport::All );
}

void QgsCrashReport::setFlags( QgsCrashReport::Flags flags )
{
  mFlags = flags;
}

const QString QgsCrashReport::toHtml() const
{
  QStringList reportData;
  reportData.append( "<b>Crash ID</b>: " + crashID() );

  if ( flags().testFlag( QgsCrashReport::Stack ) )
  {
    reportData.append( QStringLiteral( "<br>" ) );
    reportData.append( QStringLiteral( "<b>Stack Trace</b>" ) );
    if ( mStackTrace.isEmpty() )
    {
      reportData.append( QStringLiteral( "Stack trace unable to be generated." ) );
    }
    else
    {
      reportData.append( QStringLiteral( "<pre>" ) );
      Q_FOREACH ( const QgsStackTrace::StackLine &line, mStackTrace )
      {
        QFileInfo fileInfo( line.fileName );
        QString filename( fileInfo.fileName() );
        reportData.append( QStringLiteral( "(%1) %2 %3:%4" ).arg( line.moduleName, line.symbolName, filename, line.lineNumber ) );
      }
      reportData.append( QStringLiteral( "</pre>" ) );
    }
  }

#if 0
  if ( flags().testFlag( QgsCrashReport::Plugins ) )
  {
    reportData.append( "<br>" );
    reportData.append( "<b>Plugins</b>" );
    // TODO Get plugin info
  }

  if ( flags().testFlag( QgsCrashReport::ProjectDetails ) )
  {
    reportData.append( "<br>" );
    reportData.append( "<b>Project Info</b>" );
    // TODO Get project details
  }
#endif

  if ( flags().testFlag( QgsCrashReport::QgisInfo ) )
  {
    reportData.append( QStringLiteral( "<br>" ) );
    reportData.append( QStringLiteral( "<b>QGIS Info</b>" ) );
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
  }

  if ( flags().testFlag( QgsCrashReport::SystemInfo ) )
  {
    reportData.append( QStringLiteral( "<br>" ) );
    reportData.append( QStringLiteral( "<b>System Info</b>" ) );
    reportData.append( QStringLiteral( "CPU Type: %1" ).arg( QSysInfo::currentCpuArchitecture() ) );
    reportData.append( QStringLiteral( "Kernel Type: %1" ).arg( QSysInfo::kernelType() ) );
    reportData.append( QStringLiteral( "Kernel Version: %1" ).arg( QSysInfo::kernelVersion() ) );
  }

  QString report;
  Q_FOREACH ( const QString &line, reportData )
  {
    report += line + "<br>";
  }
  return report;
}

const QString QgsCrashReport::crashID() const
{
  if ( mStackTrace.isEmpty() )
    return QStringLiteral( "ID not generated due to missing information\n\n Your version of QGIS install might not have debug information included." );

  QString data = QString();

  // Hashes the full stack.
  Q_FOREACH ( const QgsStackTrace::StackLine &line, mStackTrace )
  {
#if 0
    QFileInfo fileInfo( line.fileName );
    QString filename( fileInfo.fileName() );
#endif
    data += line.symbolName;
  }

  if ( data.isNull() )
    return QStringLiteral( "ID not generated due to missing information" );

  QString hash = QString( QCryptographicHash::hash( data.toAscii(), QCryptographicHash::Sha1 ).toHex() );
  return hash;
}
