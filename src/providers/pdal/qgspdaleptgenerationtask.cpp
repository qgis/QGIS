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
#include "QgisUntwine.hpp"
#include <vector>
#include <string>
#include <QDebug>
#include <QThread>
#include <QFileInfo>
#include <QDir>
#include "qgsapplication.h"
#include <QProcessEnvironment>

QgsPdalEptGenerationTask::QgsPdalEptGenerationTask( const QString &file ):
  QgsTask( QStringLiteral( "Generate EPT Index" ) )
{
  mFile = file;

  QFileInfo fi( file );
  const QDir directory = fi.absoluteDir();

  // TODO multiplatform
  mOutputDir = directory.absolutePath() + QString( "/ept_" ) + fi.baseName();
  mUntwineExecutableBinary = guessUntwineExecutableBinary();
}

bool QgsPdalEptGenerationTask::run()
{
  QFileInfo fi( mOutputDir + "/ept.json" );
  if ( fi.exists() )
  {
    qDebug() << "untwine: already indexed";
    setProgress( 100 );
    return true;
  }
  else
  {
    if ( QDir( mOutputDir ).exists() )
    {
      qDebug() << "untwine: folder exists but no ept.json inside";
      return false;
    }
    else
    {
      qDebug() << "untwine: creating new dir " << mOutputDir;
      bool success = QDir().mkdir( mOutputDir );
      if ( !success )
        return false;
    }
  }

  QFileInfo executable( mUntwineExecutableBinary );
  if ( !executable.isExecutable() )
  {
    qDebug() << "untwine: executable not found " << mUntwineExecutableBinary;
    return false;
  }

  untwine::QgisUntwine untwineProcess( mUntwineExecutableBinary.toStdString() );

  std::vector<std::string> files = {mFile.toStdString()};
  untwineProcess.start( files, mOutputDir.toStdString() );
  bool success = false;
  int lastPercent = 0;
  setProgress( lastPercent );
  qDebug() << "untwine: start" << mFile;
  while ( true )
  {
    QThread::msleep( 100 );
    int percent = untwineProcess.progressPercent();
    if ( lastPercent != percent )
    {
      setProgress( percent / 100.0 );
      qDebug() << "untwine: progress" << percent;
    }

    if ( isCanceled() )
    {
      untwineProcess.stop();
      break;
    }

    if ( !untwineProcess.running() )
    {
      success = true;
      setProgress( 1 );
      break;
    }
  }

  qDebug() << "untwine: done " << mOutputDir << success;
  return success;
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
    qDebug() << "using untwine executable " << untwineExecutable << " defined from libexecPath()";
  }
  else
  {
    qDebug() << "using untwine executable " << untwineExecutable << " defined from env. QGIS_UNTWINE_EXECUTABLE";
  }
  return QString( "/Users/peter/Projects/pointclouds/build-QGIS-Debug/PlugIns/qgis/untwine" );
}
