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

#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QUuid>
#include <QStandardPaths>
#include <QSysInfo>
#include <QFileInfo>
#include <QCryptographicHash>

QgsCrashReport::QgsCrashReport()
{
  setFlags( QgsCrashReport::All );
}

void QgsCrashReport::setFlags( QgsCrashReport::Flags flags )
{
  mFlags = flags;
}

const QString QgsCrashReport::toMarkdown()
{
  QString markdown = toHtml();
  markdown.replace( QLatin1String( "<br>" ), QLatin1String( "\n" ) );
  markdown.replace( QLatin1String( "<b>" ), QLatin1String( "*" ) );
  markdown.replace( QLatin1String( "</b>" ), QLatin1String( "*" ) );
  markdown.replace( QLatin1String( "QGIS code revision: " ), QLatin1String( "QGIS code revision: commit:" ) );
  return markdown;
}

const QString QgsCrashReport::toHtml() const
{
  QStringList reportData;
  reportData.append( "<b>Crash ID</b>: " + crashID() );

  if ( flags().testFlag( QgsCrashReport::Stack ) )
  {
    reportData.append( QStringLiteral( "<br>" ) );
    reportData.append( QStringLiteral( "<b>Stack Trace</b>" ) );
    if ( mStackTrace->lines.isEmpty() )
    {
      reportData.append( QStringLiteral( "Stack trace could not be generated." ) );
    }
    else
    {
      reportData.append( QStringLiteral( "<pre>" ) );
      Q_FOREACH ( const QgsStackTrace::StackLine &line, mStackTrace->lines )
      {
        QFileInfo fileInfo( line.fileName );
        QString filename( fileInfo.fileName() );
        reportData.append( QStringLiteral( "%2 %3:%4" ).arg( line.symbolName, filename, line.lineNumber ) );
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
    reportData.append( mVersionInfo );
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
  if ( !mStackTrace->symbolsLoaded || mStackTrace->lines.isEmpty() )
    return QStringLiteral( "ID not generated due to missing information.<br><br> "
                           "Your version of QGIS install might not have debug information included or "
                           "we couldn't get crash information." );

  QString data = QString();

  // Hashes the full stack.
  Q_FOREACH ( const QgsStackTrace::StackLine &line, mStackTrace->lines )
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


void QgsCrashReport::exportToCrashFolder()
{
  QString folder = QgsCrashReport::crashReportFolder();
  QDir dir( folder );
  if ( !dir.exists() )
  {
    QDir().mkpath( folder );
  }

  QString fileName = folder + "/stack.txt";

  QFile file( fileName );
  if ( file.open( QIODevice::WriteOnly | QIODevice::Text ) )
  {
    QTextStream stream( &file );
    stream << mStackTrace->fullStack << endl;
  }
  file.close();

  fileName = folder + "/report.txt";

  file.setFileName( fileName );
  if ( file.open( QIODevice::WriteOnly | QIODevice::Text ) )
  {
    QTextStream stream( &file );
    stream << toMarkdown() << endl;
  }
  file.close();
}

QString QgsCrashReport::crashReportFolder()
{
  return QStandardPaths::standardLocations( QStandardPaths::AppLocalDataLocation ).value( 0 ) +
         "/crashes/" +
         QUuid::createUuid().toString().replace( "{", "" ).replace( "}", "" );
}
