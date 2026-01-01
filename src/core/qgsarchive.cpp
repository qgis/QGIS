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

#include "qgsauxiliarystorage.h"
#include "qgsmessagelog.h"
#include "qgsziputils.h"

#ifdef Q_OS_WIN
#include <windows.h>
#endif

#include <QStandardPaths>
#include <QUuid>
#include <memory>

QgsArchive::QgsArchive()
  : mDir( new QTemporaryDir() )
{
}

QgsArchive::QgsArchive( const QgsArchive &other )
  : mFiles( other.mFiles )
  , mDir( new QTemporaryDir() )
{
}

QgsArchive &QgsArchive::operator=( const QgsArchive &other )
{
  if ( this != &other )
  {
    mFiles = other.mFiles;
    mDir = std::make_unique<QTemporaryDir>( );
  }

  return *this;
}

QString QgsArchive::dir() const
{
  return mDir->path();
}

void QgsArchive::clear()
{
  mDir = std::make_unique<QTemporaryDir>( );
  mFiles.clear();
}

bool QgsArchive::zip( const QString &filename )
{
  const QString tempPath( QDir::temp().absoluteFilePath( u"qgis-project-XXXXXX.zip"_s ) );

  // zip content
  if ( ! QgsZipUtils::zip( tempPath, mFiles, true ) )
  {
    const QString err = QObject::tr( "Unable to zip content" );
    QgsMessageLog::logMessage( err, u"QgsArchive"_s );
    return false;
  }

  QString target {filename};

  // remove existing zip file
  if ( QFile::exists( target ) )
  {
    // If symlink -> we want to write to its target instead
    const QFileInfo targetFileInfo( target );
    target = targetFileInfo.canonicalFilePath();
    // If target still exists, remove (might not exist if was a dangling symlink)
    if ( QFile::exists( target ) )
      QFile::remove( target );
  }

#ifdef Q_OS_WIN
  // Clear temporary flag (see GH #32118)
  DWORD dwAttrs;
#ifdef UNICODE
  dwAttrs = GetFileAttributes( qUtf16Printable( tempPath ) );
  SetFileAttributes( qUtf16Printable( tempPath ), dwAttrs & ~ FILE_ATTRIBUTE_TEMPORARY );
#else
  dwAttrs = GetFileAttributes( tempPath.toLocal8Bit( ).data( ) );
  SetFileAttributes( tempPath.toLocal8Bit( ).data( ), dwAttrs & ~ FILE_ATTRIBUTE_TEMPORARY );
#endif

#endif // Q_OS_WIN

  // save zip archive
  if ( !QFile::rename( tempPath, target ) )
  {
    const QString err = QObject::tr( "Unable to save zip file '%1'" ).arg( target );
    QgsMessageLog::logMessage( err, u"QgsArchive"_s );
    return false;
  }

  return true;
}

bool QgsArchive::unzip( const QString &filename )
{
  clear();
  return QgsZipUtils::unzip( filename, mDir->path(), mFiles );
}

void QgsArchive::addFile( const QString &file )
{
  mFiles.append( file );
}

bool QgsArchive::removeFile( const QString &file )
{
  bool rc = false;

  if ( !file.isEmpty() && mFiles.contains( file ) && QFile::exists( file ) )
    rc = QFile::remove( file );

  mFiles.removeOne( file );

  return rc;
}

QStringList QgsArchive::files() const
{
  return mFiles;
}

bool QgsArchive::exists() const
{
  return QFileInfo::exists( mDir->path() );
}

QString QgsProjectArchive::projectFile() const
{
  const auto constFiles = files();
  for ( const QString &file : constFiles )
  {
    const QFileInfo fileInfo( file );
    if ( fileInfo.suffix().compare( "qgs"_L1, Qt::CaseInsensitive ) == 0 )
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
  return removeFile( projectFile() );
}

QString QgsProjectArchive::auxiliaryStorageFile() const
{
  const QString extension = QgsAuxiliaryStorage::extension();

  const QStringList fileList = files();
  for ( const QString &file : fileList )
  {
    const QFileInfo fileInfo( file );
    if ( fileInfo.suffix().compare( extension, Qt::CaseInsensitive ) == 0 )
      return file;
  }

  return QString();
}
