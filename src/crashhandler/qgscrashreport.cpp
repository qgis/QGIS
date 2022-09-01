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
#include <QRegularExpression>
#include <QRegularExpressionMatch>

QgsCrashReport::QgsCrashReport()
  : mPythonFault( PythonFault() )
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
  const QString thisCrashID = crashID();
  if ( !thisCrashID.isEmpty() )
    reportData.append( QStringLiteral( "<b>Crash ID</b>: <a href='https://github.com/qgis/QGIS/search?q=%1&type=Issues'>%1</a><br>" ).arg( thisCrashID ) );

  if ( flags().testFlag( QgsCrashReport::Stack ) )
  {
    QStringList pythonStack;
    if ( !mPythonCrashLogFilePath.isEmpty() )
    {
      QFile pythonLog( mPythonCrashLogFilePath );
      if ( pythonLog.open( QIODevice::ReadOnly | QIODevice::Text ) )
      {
        QTextStream inputStream( &pythonLog );
        while ( !inputStream.atEnd() )
        {
          pythonStack.append( inputStream.readLine() );
        }
      }
      pythonLog.close();
    }

    if ( !pythonStack.isEmpty() )
    {
      QString pythonStackString = QStringLiteral( "<b>Python Stack Trace</b><pre>" );

      for ( const QString &line : pythonStack )
      {
        const thread_local QRegularExpression pythonTraceRx( QStringLiteral( "\\s*File\\s+\"(.*)\",\\s+line\\s+(\\d+)" ) );

        const QRegularExpressionMatch fileLineMatch = pythonTraceRx.match( line );
        if ( fileLineMatch.hasMatch() )
        {
          const QString pythonFilePath = fileLineMatch.captured( 1 );
          const int lineNumber = fileLineMatch.captured( 2 ).toInt();
          QFile pythonFile( pythonFilePath );
          if ( pythonFile.open( QIODevice::ReadOnly | QIODevice::Text ) )
          {
            QTextStream inputStream( &pythonFile );
            // read lines till we find target line
            int currentLineNumber = 0;
            QString pythonLine;
            for ( ; currentLineNumber < lineNumber && !inputStream.atEnd(); ++currentLineNumber )
            {
              pythonLine = inputStream.readLine();
            }

            pythonStackString.append( line + '\n' );
            if ( currentLineNumber == lineNumber )
            {
              pythonStackString.append( QStringLiteral( "    " ) + pythonLine.trimmed() + '\n' );
            }
          }
        }
        else
        {
          pythonStackString.append( line + '\n' );
        }
      }
      pythonStackString.append( QStringLiteral( "</pre>" ) );
      reportData.append( pythonStackString );
    }

    reportData.append( QStringLiteral( "<br><b>Stack Trace</b>" ) );
    if ( !mStackTrace || mStackTrace->lines.isEmpty() )
    {
      reportData.append( QStringLiteral( "No stack trace is available." ) );
    }
    else if ( !mStackTrace->symbolsLoaded )
    {
      reportData.append( QStringLiteral( "Stack trace could not be generated due to missing symbols." ) );
    }
    else
    {
      reportData.append( QStringLiteral( "<pre>" ) );
      for ( const QgsStackTrace::StackLine &line : mStackTrace->lines )
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
  for ( const QString &line : std::as_const( reportData ) )
  {
    report += line + "<br>";
  }
  return report;
}

const QString QgsCrashReport::crashID() const
{
  if ( mPythonFault.cause != LikelyPythonFaultCause::NotPython )
    return QString(); // don't report crash IDs for python crashes -- they won't be representative of the cause of the crash

  if ( !mStackTrace )
    return QStringLiteral( "Not available" );

  if ( !mStackTrace->symbolsLoaded || mStackTrace->lines.isEmpty() )
    return QStringLiteral( "ID not generated due to missing information.<br><br> "
                           "Your version of QGIS install might not have debug information included or "
                           "we couldn't get crash information." );

  QString data = QString();

  // Hashes the full stack.
  for ( const QgsStackTrace::StackLine &line : mStackTrace->lines )
  {
#if 0
    QFileInfo fileInfo( line.fileName );
    QString filename( fileInfo.fileName() );
#endif
    data += line.symbolName;
  }

  if ( data.isNull() )
    return QStringLiteral( "ID not generated due to missing information" );

  QString hash = QString( QCryptographicHash::hash( data.toLatin1(), QCryptographicHash::Sha1 ).toHex() );
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

  QString fileName;
  QFile file;

  if ( mStackTrace )
  {
    fileName = folder + "/stack.txt";

    file.setFileName( fileName );
    if ( file.open( QIODevice::WriteOnly | QIODevice::Text ) )
    {
      QTextStream stream( &file );
      stream << mStackTrace->fullStack << Qt::endl;
    }
    file.close();
  }

  fileName = folder + "/report.txt";
  file.setFileName( fileName );
  if ( file.open( QIODevice::WriteOnly | QIODevice::Text ) )
  {
    QTextStream stream( &file );
    stream << htmlToMarkdown( toHtml() ) << Qt::endl;
  }
  file.close();

  if ( !mPythonCrashLogFilePath.isEmpty() )
  {
    fileName = folder + "/python.txt";
    QFile pythonLog( mPythonCrashLogFilePath );
    if ( pythonLog.open( QIODevice::ReadOnly | QIODevice::Text ) )
    {
      QTextStream inputStream( &pythonLog );
      file.setFileName( fileName );
      if ( file.open( QIODevice::WriteOnly | QIODevice::Text ) )
      {
        QTextStream outputStream( &file );

        QString line;
        while ( !inputStream.atEnd() )
        {
          line = inputStream.readLine();
          outputStream << line;
        }
      }
      file.close();
      pythonLog.close();
    }
  }
}

QString QgsCrashReport::crashReportFolder()
{
  return QStandardPaths::standardLocations( QStandardPaths::AppLocalDataLocation ).value( 0 ) +
         "/crashes/" +
         QUuid::createUuid().toString().replace( "{", "" ).replace( "}", "" );
}

void QgsCrashReport::setPythonCrashLogFilePath( const QString &path )
{
  mPythonCrashLogFilePath = path;

  QFile pythonLog( mPythonCrashLogFilePath );
  if ( pythonLog.open( QIODevice::ReadOnly | QIODevice::Text ) )
  {
    QTextStream inputStream( &pythonLog );
    QString line;
    while ( !inputStream.atEnd() )
    {
      line = inputStream.readLine();

      if ( !line.trimmed().isEmpty() && mPythonFault.cause == LikelyPythonFaultCause::NotPython )
        mPythonFault.cause = LikelyPythonFaultCause::Unknown;

      const thread_local QRegularExpression pythonTraceRx( QStringLiteral( "\\s*File\\s+\"(.*)\",\\s+line\\s+(\\d+)" ) );

      const QRegularExpressionMatch fileLineMatch = pythonTraceRx.match( line );
      if ( fileLineMatch.hasMatch() )
      {
        const QString pythonFilePath = fileLineMatch.captured( 1 );
        if ( pythonFilePath.contains( QLatin1String( "profiles" ), Qt::CaseInsensitive )
             && pythonFilePath.contains( QLatin1String( "processing" ), Qt::CaseInsensitive )
             && pythonFilePath.contains( QLatin1String( "scripts" ), Qt::CaseInsensitive ) )
        {
          mPythonFault.cause = LikelyPythonFaultCause::ProcessingScript;
          const QFileInfo fi( pythonFilePath );
          mPythonFault.title = fi.fileName();
          mPythonFault.filePath = pythonFilePath;
        }
        else if ( mPythonFault.cause == LikelyPythonFaultCause::Unknown && pythonFilePath.contains( QLatin1String( "console.py" ), Qt::CaseInsensitive ) )
        {
          mPythonFault.cause = LikelyPythonFaultCause::ConsoleCommand;
        }
        else if ( mPythonFault.cause == LikelyPythonFaultCause::Unknown )
        {
          const thread_local QRegularExpression pluginRx( QStringLiteral( "python[/\\\\]plugins[/\\\\](.*?)[/\\\\]" ) );
          const QRegularExpressionMatch pluginNameMatch = pluginRx.match( pythonFilePath );
          if ( pluginNameMatch.hasMatch() )
          {
            mPythonFault.cause = LikelyPythonFaultCause::Plugin;
            mPythonFault.title = pluginNameMatch.captured( 1 );
            mPythonFault.filePath = pythonFilePath;
          }
        }
      }
    }
  }
}

QString QgsCrashReport::htmlToMarkdown( const QString &html )
{
  // Any changes in this function must be copied to qgsstringutils.cpp too
  QString converted = html;
  converted.replace( QLatin1String( "<br>" ), QLatin1String( "\n" ) );
  converted.replace( QLatin1String( "<b>" ), QLatin1String( "**" ) );
  converted.replace( QLatin1String( "</b>" ), QLatin1String( "**" ) );
  converted.replace( QLatin1String( "<pre>" ), QLatin1String( "\n```\n" ) );
  converted.replace( QLatin1String( "</pre>" ), QLatin1String( "```\n" ) );

  const thread_local QRegularExpression hrefRegEx( QStringLiteral( "<a\\s+href\\s*=\\s*([^<>]*)\\s*>([^<>]*)</a>" ) );

  int offset = 0;
  QRegularExpressionMatch match = hrefRegEx.match( converted );
  while ( match.hasMatch() )
  {
    QString url = match.captured( 1 ).replace( QLatin1String( "\"" ), QString() );
    url.replace( '\'', QString() );
    QString name = match.captured( 2 );
    QString anchor = QStringLiteral( "[%1](%2)" ).arg( name, url );
    converted.replace( match.capturedStart(), match.capturedLength(), anchor );
    offset = match.capturedStart() + anchor.length();
    match = hrefRegEx.match( converted, offset );
  }

  return converted;
}
