/***************************************************************************
  qgspdalindexingtask.cpp
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

#include "qgspdalindexingtask.h"
#include "moc_qgspdalindexingtask.cpp"

#include <vector>
#include <string>
#include <QDebug>
#include <QThread>
#include <QFileInfo>
#include <QDir>
#include <QProcessEnvironment>
#include <QTemporaryDir>

#include "qgsapplication.h"
#include "QgisUntwine.hpp"
#include "qgis.h"

QgsPdalIndexingTask::QgsPdalIndexingTask( const QString &file, const QString &outputPath, const QString &name )
  : QgsTask( tr( "Creating indexed COPC (%1)" ).arg( name ) )
  , mOutputPath( outputPath )
  , mFile( file )
{
  mUntwineExecutableBinary = guessUntwineExecutableBinary();
}

bool QgsPdalIndexingTask::run()
{
  if ( isCanceled() || !prepareOutputPath() )
    return false;

  const bool result = runUntwine();

  cleanup();

  return result;
}

void QgsPdalIndexingTask::cleanup()
{
  QFile::remove( QStringLiteral( "%1-indexing" ).arg( mFile ) );
}

bool QgsPdalIndexingTask::runUntwine()
{
  const QFileInfo executable( mUntwineExecutableBinary );
  if ( !executable.isExecutable() )
  {
    mErrorMessage = tr( "Untwine executable not found %1" ).arg( mUntwineExecutableBinary );
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
  options.push_back( { "temp_dir", mTempDir->path().toStdString() } );

  const std::vector<std::string> files = { mFile.toStdString() };
  untwineProcess.start( files, mOutputPath.toStdString(), options );
  int lastPercent = 0;
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
      lastPercent = percent;
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
        mErrorMessage = tr( "COPC creation failed: %1" ).arg( QString::fromStdString( untwineProcess.errorMessage() ) );
        QgsDebugError( mErrorMessage );
        return false;
      }

      return true;
    }
  }
}

QString QgsPdalIndexingTask::untwineExecutableBinary() const
{
  return mUntwineExecutableBinary;
}

void QgsPdalIndexingTask::setUntwineExecutableBinary( const QString &untwineExecutableBinary )
{
  mUntwineExecutableBinary = untwineExecutableBinary;
}

QString QgsPdalIndexingTask::guessUntwineExecutableBinary() const
{
  QString untwineExecutable = QProcessEnvironment::systemEnvironment().value( QStringLiteral( "QGIS_UNTWINE_EXECUTABLE" ) );
  if ( untwineExecutable.isEmpty() )
  {
#if defined( Q_OS_WIN )
    untwineExecutable = QgsApplication::libexecPath() + "untwine.exe";
#else
    untwineExecutable = QgsApplication::libexecPath() + "untwine";
#endif
  }
  return QString( untwineExecutable );
}

QString QgsPdalIndexingTask::outputPath() const
{
  return mOutputPath;
}

bool QgsPdalIndexingTask::prepareOutputPath()
{
  const QFileInfo fi( mOutputPath );
  if ( fi.exists() )
  {
    mErrorMessage = tr( "File %1 is already indexed" ).arg( mFile );
    return false;
  }

  mTempDir = std::make_unique< QTemporaryDir >();
  if ( !mTempDir->isValid() )
  {
    mErrorMessage = tr( "Directory is not writable: %1" ).arg( mTempDir->path() );
    return false;
  }

  QFile marker( QStringLiteral( "%1-indexing" ).arg( mFile ) );
  if ( marker.exists() )
  {
    mErrorMessage = tr( "Another indexing process is running (or finished with crash) for file %1" ).arg( mFile );
    return false;
  }

  // this check is last so we only create the marker file if no error occured
  if ( !marker.open( QIODevice::WriteOnly ) )
  {
    mErrorMessage = tr( "Directory is not writable: %1" ).arg( fi.canonicalPath() );
    return false;
  }

  return true;
}
