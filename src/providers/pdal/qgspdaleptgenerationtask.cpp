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
    QgsMessageLog::logMessage( tr( "Removing temporary files in %1" ).arg( tmpDir.dirName() ), QObject::tr( "Point clouds" ), Qgis::MessageLevel::Info );
    tmpDir.removeRecursively();
  }
}

bool QgsPdalEptGenerationTask::runUntwine()
{
  QFileInfo executable( mUntwineExecutableBinary );
  if ( !executable.isExecutable() )
  {
    QgsMessageLog::logMessage( tr( "Untwine executable not found %1" ).arg( mUntwineExecutableBinary ), QObject::tr( "Point clouds" ), Qgis::MessageLevel::Critical );
    return false;
  }
  else
  {
    QgsMessageLog::logMessage( tr( "Using executable %1" ).arg( mUntwineExecutableBinary ), QObject::tr( "Point clouds" ), Qgis::MessageLevel::Info );
  }

  untwine::QgisUntwine untwineProcess( mUntwineExecutableBinary.toStdString() );

  untwine::QgisUntwine::Options options;
  // By default Untwine does not calculate stats for attributes, but they are very useful for us:
  // we can use them to set automatically set valid range for the data without having to scan the points again.
  options.push_back( { "stats", std::string() } );

  std::vector<std::string> files = {mFile.toStdString()};
  untwineProcess.start( files, mOutputDir.toStdString(), options );
  int lastPercent = 0;
  while ( true )
  {
    QThread::msleep( 100 );
    int percent = untwineProcess.progressPercent();
    if ( lastPercent != percent )
    {
      QString message = QString::fromStdString( untwineProcess.progressMessage() );
      if ( !message.isEmpty() )
        QgsMessageLog::logMessage( message, QObject::tr( "Point clouds" ), Qgis::MessageLevel::Info );

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
  QFileInfo fi( mOutputDir + "/ept.json" );
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
    }
    else
    {
      bool success = QDir().mkdir( mOutputDir );
      if ( success )
      {
        QgsMessageLog::logMessage( tr( "Created output directory %1" ).arg( mOutputDir ), QObject::tr( "Point clouds" ), Qgis::MessageLevel::Info );
      }
      else
      {
        QgsMessageLog::logMessage( tr( "Unable to create output directory %1" ).arg( mOutputDir ), QObject::tr( "Point clouds" ), Qgis::MessageLevel::Critical );
        return false;
      }
    }
  }

  return true;
}
