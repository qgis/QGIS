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
#include <iostream>

QgsArchive::QgsArchive()
  : mDir( new QTemporaryDir() )
{
}

QgsArchive::QgsArchive( const QgsArchive &other )
  : mFiles( other.mFiles )
  ,  mDir( new QTemporaryDir() )
  , mFilename( other.mFilename )
{
}

QgsArchive &QgsArchive::operator=( const QgsArchive &other )
{
  if ( this != &other )
  {
    mFilename = other.mFilename;
    mFiles = other.mFiles;
  }

  return *this;
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

bool QgsArchive::zip()
{
  if ( mFilename.isEmpty() )
    return false;
  else
    return zip( mFilename );
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

bool QgsArchive::unzip()
{
  if ( mFilename.isEmpty() )
    return false;
  else
    return unzip( mFilename );
}

bool QgsArchive::unzip( const QString &filename )
{
  clear();
  mFilename = filename;
  return QgsZipUtils::unzip( filename, mDir->path(), mFiles );
}

void QgsArchive::addFile( const QString &file )
{
  mFiles.append( file );
}

QString QgsArchive::filename() const
{
  return mFilename;
}

void QgsArchive::setFileName( const QString &filename )
{
  mFilename = filename;
}

QStringList QgsArchive::files() const
{
  return mFiles;
}

QString QgsProjectArchive::projectFile() const
{
  Q_FOREACH ( const QString &file, mFiles )
  {
    QFileInfo fileInfo( file );
    if ( fileInfo.suffix().compare( QLatin1String( "qgs" ), Qt::CaseInsensitive ) == 0 )
      return file;
  }

  return QString();
}

bool QgsProjectArchive::unzip( const QString &filename )
{
  if ( QgsArchive::unzip( filename ) )
    return ! projectFile().isEmpty();
  else
    return false;
}

bool QgsProjectArchive::clearProjectFile()
{
  bool rc = false;
  QString file = projectFile();

  if ( !file.isEmpty() && QFile::exists( file ) )
    rc = QFile::remove( file );

  if ( rc )
    mFiles.removeOne( file );

  return rc;
}
