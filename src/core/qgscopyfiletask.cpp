/***************************************************************************
  qgscopyfiletask.cpp
  --------------------------------------
  Date                 : March 2021
  Copyright            : (C) 2021 by Julien Cabieces
  Email                : julien dot cabieces at oslandia dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgscopyfiletask.h"

#include <QFile>
#include <QFileInfo>
#include <QDir>

QgsCopyFileTask::QgsCopyFileTask( const QString &source, const QString &destination )
  : mSource( source ),
    mDestination( destination )
{
}

bool QgsCopyFileTask::run()
{
  QFile fileSource( mSource );
  if ( !fileSource.exists() )
  {
    mErrorString = tr( "Source file '%1' does not exist" ).arg( mSource );
    return false;
  }

  if ( QFileInfo( mDestination ).isDir() )
  {
    mDestination = QDir( mDestination ).absoluteFilePath( QFileInfo( fileSource ).fileName() );
  }

  QFile fileDestination( mDestination );
  if ( fileDestination.exists() )
  {
    mErrorString = tr( "Destination file '%1' already exist" ).arg( mDestination );
    return false;
  }

  const QDir destinationDir = QFileInfo( mDestination ).absoluteDir();
  if ( !destinationDir.exists() )
  {
    mErrorString = tr( "Destination directory '%1' does not exist" ).arg( destinationDir.absolutePath() );
    return false;
  }

  fileSource.open( QIODevice::ReadOnly );
  fileDestination.open( QIODevice::WriteOnly );

  const int size = fileSource.size();
  const int chunkSize = std::clamp( size / 100, 1024, 1024 * 1024 );

  int bytesRead = 0;
  std::vector<char> data( chunkSize );
  while ( true )
  {
    const int len = fileSource.read( data.data(), chunkSize );
    if ( len == -1 )
    {
      mErrorString = tr( "Fail reading from '%1'" ).arg( mSource );
      return false;
    }

    // finish reading
    if ( !len )
      break;

    if ( fileDestination.write( data.data(), len ) != len )
    {
      mErrorString = tr( "Fail writing to '%1'" ).arg( mDestination );
      return false;
    }

    bytesRead += len;
    setProgress( static_cast<double>( bytesRead ) / size );
  }

  setProgress( 100 );

  fileSource.close();
  fileDestination.close();

  return true;
}

const QString &QgsCopyFileTask::errorString() const
{
  return mErrorString;
}

const QString &QgsCopyFileTask::destination() const
{
  return mDestination;
}
