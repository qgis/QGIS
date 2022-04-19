/***************************************************************************
  qgspdaleptgenerationtask.cpp
  ------------------------
  Date                 : December 2020
  Copyright            : (C) 2020 by Peter Petrik
  Email                : zilolv at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgspdaleptgenerationtask.h"

#include <vector>
#include <string>
#include <QDebug>
#include <QThread>
#include <QFileInfo>
#include <QDir>
#include <QProcessEnvironment>

#include "qgsapplication.h"
#include "QgisUntwine.hpp"
#include "qgsmessagelog.h"
#include "qgis.h"

QgsPdalEptGenerationTask::QgsPdalEptGenerationTask( const QString &file, const QString &outputDir, const QString &name )
  : QgsTask( tr( "Indexing Point Cloud (%1)" ).arg( name ) )
  , mOutputDir( outputDir )
  , mFile( file )
{
  mUntwineExecutableBinary = guessUntwineExecutableBinary();
}

bool QgsPdalEptGenerationTask::run()
{
  if ( isCanceled() || !prepareOutputDir() )
    return false;

  if ( isCanceled() || !runUntwine() )
    return false;

  if ( isCanceled() )
    return false;

  cleanTemp();

  return true;
}

void QgsPdalEptGenerationTask::cleanTemp()
{
  QDir tmpDir( mOutputDir + QStringLiteral( "/temp" ) );
  if ( tmpDir.exists() )
  {
    QgsDebugMsgLevel( QStringLiteral( "Removing temporary files in %1" ).arg( tmpDir.dirName() ), 2 );
    tmpDir.removeRecursively();
  }
}

bool QgsPdalEptGenerationTask::runUntwine()
{
  const QFileInfo executable( mUntwineExecutableBinary );
  if ( !executable.isExecutable() )
  {
    QgsMessageLog::logMessage( tr( "Untwine executable not found %1" ).arg( mUntwineExecutableBinary ), QObject::tr( "Point clouds" ), Qgis::MessageLevel::Critical );
    return false;
  }
  else
  {
    QgsDebugMsgLevel( QStringLiteral( "Using executable %1" ).arg( mUntwineExecutableBinary ), 2 );
  }

  untwine::QgisUntwine untwineProcess( mUntwineExecutableBinary.toStdString() );

  untwine::QgisUntwine::Options options;
  // By default Untwine does not calculate stats for attributes, but they are very useful for us:
  // we can use them to set automatically set valid range for the data without having to scan the points again.
  options.push_back( { "stats", std::string() } );

  const std::vector<std::string> files = {mFile.toStdString()};
  untwineProcess.start( files, mOutputDir.toStdString(), options );
  const int lastPercent = 0;
  while ( true )
  {
    QThread::msleep( 100 );
    const int percent = untwineProcess.progressPercent();
    if ( lastPercent != percent )
    {
#ifdef QGISDEBUG
      const QString message = QString::fromStdString( untwineProcess.progressMessage() );
      if ( !message.isEmpty() )
      {
        QgsDebugMsgLevel( message, 2 );
      }
#endif
      setProgress( percent );
    }

    if ( isCanceled() )
    {
      untwineProcess.stop();
      return false;
    }

    if ( !untwineProcess.running() )
    {
      setProgress( 100 );

      if ( !untwineProcess.errorMessage().empty() )
      {
        // TODO: propagate the error message to GUI
        mErrorMessage = QStringLiteral( "Untwine error: %1" ).arg( QString::fromStdString( untwineProcess.errorMessage() ) );
        QgsDebugMsg( mErrorMessage );
        return false;
      }

      return true;
    }
  }
}

QString QgsPdalEptGenerationTask::untwineExecutableBinary() const
{
  return mUntwineExecutableBinary;
}

void QgsPdalEptGenerationTask::setUntwineExecutableBinary( const QString &untwineExecutableBinary )
{
  mUntwineExecutableBinary = untwineExecutableBinary;
}

QString QgsPdalEptGenerationTask::guessUntwineExecutableBinary() const
{
  QString untwineExecutable = QProcessEnvironment::systemEnvironment().value( QStringLiteral( "QGIS_UNTWINE_EXECUTABLE" ) );
  if ( untwineExecutable.isEmpty() )
  {
#if defined(Q_OS_WIN)
    untwineExecutable = QgsApplication::libexecPath() + "untwine.exe";
#else
    untwineExecutable = QgsApplication::libexecPath() + "untwine";
#endif
  }
  return QString( untwineExecutable );
}

QString QgsPdalEptGenerationTask::outputDir() const
{
  return mOutputDir;
}

bool QgsPdalEptGenerationTask::prepareOutputDir()
{
  const QFileInfo fi( mOutputDir + "/ept.json" );
  if ( fi.exists() )
  {
    QgsMessageLog::logMessage( tr( "File %1 is already indexed" ).arg( mFile ), QObject::tr( "Point clouds" ), Qgis::MessageLevel::Info );
    return true;
  }
  else
  {
    if ( QDir( mOutputDir ).exists() )
    {
      if ( !QDir( mOutputDir ).isEmpty() )
      {
        if ( QDir( mOutputDir + "/temp" ).exists() )
        {
          QgsMessageLog::logMessage( tr( "Another indexing process is running (or finished with crash) in directory %1" ).arg( mOutputDir ), QObject::tr( "Point clouds" ), Qgis::MessageLevel::Warning );
          return false;
        }
        else
        {
          QgsMessageLog::logMessage( tr( "Folder %1 is non-empty, but there isn't ept.json present." ).arg( mOutputDir ), QObject::tr( "Point clouds" ), Qgis::MessageLevel::Critical );
          return false;
        }
      }
      else
      {
        // untwine expects that the output directory does not exist at all
        if ( !QDir().rmdir( mOutputDir ) )
        {
          QgsMessageLog::logMessage( tr( "Failed to remove empty directory %1" ).arg( mOutputDir ), QObject::tr( "Point clouds" ), Qgis::MessageLevel::Critical );
          return false;
        }
      }
    }
  }

  return true;
}
