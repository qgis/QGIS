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

QgsPdalEptGenerationTask::QgsPdalEptGenerationTask( const QString &file ):
  QgsTask( QStringLiteral( "Generate EPT" ) )
{
  //TODO
  mFile = file;
  mOutputDir = "/Users/peter/tmp/ept";
  mUntwineExecutableBinary = "/Users/peter/Projects/pointclouds/build-QGIS-Debug/PlugIns/qgis/untwine";
  mUntwineProcess.reset( new untwine::QgisUntwine( mUntwineExecutableBinary.toStdString() ) );
}

bool QgsPdalEptGenerationTask::run()
{
  std::vector<std::string> files = {mFile.toStdString()};
  mUntwineProcess->start( files, mOutputDir.toStdString() );
  bool success = false;
  int lastPercent = 0;

  qDebug() << "UNTWINE: " << mFile;

  while ( true )
  {
    int percent = mUntwineProcess->progressPercent();
    if ( percent != lastPercent )
    {
      lastPercent = percent;
      setProgress( percent / 100.0 );
      qDebug() << "UNTWINE: progress " << percent;
    }

    // std::string s = mUntwineProcess->progressMessage();
    // std::cerr << "Percent/Msg = " << percent << " / " << s << "!\n";
    if ( isCanceled() )
    {
      mUntwineProcess->stop();
      break;
    }

    if ( !mUntwineProcess->running() )
    {
      success = true;
      break;
    }
  }

  qDebug() << "UNTWINE: done " << mOutputDir;
  return success;
}
