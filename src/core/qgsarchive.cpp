/***************************************************************************
                            qgsarchive.cpp
                           ----------------

    begin                : July 07, 2017
    copyright            : (C) 2017 by Paul Blottiere
    email                : paul.blottiere@oslandia.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsarchive.h"
#include "qgsziputils.h"
#include "qgsmessagelog.h"

QgsArchive::QgsArchive()
  : mDir( new QTemporaryDir() )
{
}

QgsArchive::~QgsArchive()
{
}

QString QgsArchive::dir() const
{
  return mDir->path();
}

void QgsArchive::clear()
{
  mDir.reset( new QTemporaryDir() );
  mFilename.clear();
  mFiles.clear();
}

bool QgsArchive::zip( const QString &filename )
{
  // create a temporary path
  QTemporaryFile tmpFile;
  tmpFile.open();
  tmpFile.close();

  // zip content
  if ( ! QgsZipUtils::zip( tmpFile.fileName(), mFiles ) )
  {
    QString err = QObject::tr( "Unable to zip content" );
    QgsMessageLog::logMessage( err, QStringLiteral( "QgsArchive" ) );
    return false;
  }

  // remove existing zip file
  if ( QFile::exists( filename ) )
    QFile::remove( filename );

  // save zip archive
  if ( ! tmpFile.rename( filename ) )
  {
    QString err = QObject::tr( "Unable to save zip file '%1'" ).arg( filename );
    QgsMessageLog::logMessage( err, QStringLiteral( "QgsArchive" ) );
    return false;
  }

  // keep the zip filename
  tmpFile.setAutoRemove( false );
  mFilename = filename;

  return true;
}

bool QgsArchive::unzip( const QString &filename )
{
  clear();

  QgsZipUtils::unzip( filename, mDir->path(), mFiles );
  mFilename = filename;

  return ! projectFile().isEmpty();
}

void QgsArchive::addFile( const QString &file )
{
  mFiles.append( file );
}

QString QgsArchive::filename() const
{
  return mFilename;
}

QString QgsArchive::projectFile() const
{
  Q_FOREACH ( const QString &file, mFiles )
  {
    QFileInfo fileInfo( file );
    if ( "qgs" == fileInfo.suffix().toLower() )
      return file;
  }

  return QString();
}

QStringList QgsArchive::files() const
{
  return mFiles;
}
