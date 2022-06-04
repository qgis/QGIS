/***************************************************************************
                         qgsrangerequestcache.cpp
                         --------------------
    begin                : April 2022
    copyright            : (C) 2022 by Belgacem Nedjima
    email                : belgacem dot nedjima at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsrangerequestcache.h"

#include <QtDebug>
#include <QFile>
#include <QDir>
#include <QDateTime>

QgsRangeRequestCache::QgsRangeRequestCache()
{

}

QByteArray QgsRangeRequestCache::entry( const QNetworkRequest &request )
{
  QString hash = rangeFileName( request );
  QByteArray arr = readFile( hash );
  return arr;
}

bool QgsRangeRequestCache::hasEntry( const QNetworkRequest &request )
{
  QDir dir( mCacheDir );
  return dir.exists( rangeFileName( request ) );
}

void QgsRangeRequestCache::registerEntry( const QNetworkRequest &request, QByteArray data )
{
  QString hash = rangeFileName( request );
  writeFile( hash, data );
  expire();
}

void QgsRangeRequestCache::clear()
{
  QDir dir( mCacheDir );
  for ( QFileInfo info : dir.entryInfoList() )
  {
    removeFile( info.filePath() );
  }
}

bool QgsRangeRequestCache::setCacheDirectory( const QString &path )
{
  QString cachePath = path;
  if ( !cachePath.endsWith( QDir::separator() ) )
  {
    cachePath.push_back( QDir::separator() );
  }
  mCacheDir = cachePath;

  if ( !QDir().mkpath( mCacheDir ) )
  {
    mError = QObject::tr( "Unable to create cache directory \"%1\"" ).arg( mCacheDir );
    return false;
  }
  return true;
}

void QgsRangeRequestCache::setCacheSize( qint64 cacheSize )
{
  mMaxDataSize = cacheSize;
  expire();
}

QString QgsRangeRequestCache::rangeFileName( const QNetworkRequest &request ) const
{
  return mCacheDir + QStringLiteral( "%1-%2" ).arg( qHash( request.url().toString() ) ).arg( QString::fromUtf8( request.rawHeader( "Range" ) ) );
}

QByteArray QgsRangeRequestCache::readFile( const QString &fileName )
{
  QFile file( fileName );
  if ( !file.open( QFile::OpenModeFlag::ReadOnly ) )
  {
    mError = QObject::tr( "Unable to read cache file \"%1\"" ).arg( fileName );
    return QByteArray();
  }
  return file.readAll();
}

bool QgsRangeRequestCache::writeFile( const QString &fileName, QByteArray data )
{
  QFile file( fileName );
  if ( !file.open( QFile::OpenModeFlag::WriteOnly ) )
  {
    mError = QObject::tr( "Unable to open cache file \"%1\"" ).arg( fileName );
    return false;
  }
  qint64 written = file.write( data );
  file.close();
  if ( written != data.size() )
  {
    mError = QObject::tr( "Unable to write to cache file \"%1\", error: \"%2\"" ).arg( fileName, file.errorString() );
    return false;
  }
  return true;
}

bool QgsRangeRequestCache::removeFile( const QString &fileName )
{
  if ( fileName.isEmpty() )
    return false;
  bool wasRemoved = QFile::remove( fileName );
  if ( !wasRemoved )
  {
    mError = QObject::tr( "Unable to remove cache file \"%1\"" ).arg( fileName );
  }
  return wasRemoved;
}

void QgsRangeRequestCache::expire()
{
  QFileInfoList filesList = cacheEntries();
  qint64 totalSize = 0;
  for ( QFileInfo &info : filesList )
  {
    totalSize += info.size();
  }
  while ( totalSize > mMaxDataSize )
  {
    QFileInfo info = filesList.back();
    filesList.pop_back();
    totalSize -= info.size();
    removeFile( info.filePath() );
  }
}

QFileInfoList QgsRangeRequestCache::cacheEntries()
{
  QStringList list;
  QDir dir( mCacheDir );
  QFileInfoList filesList = dir.entryInfoList( QDir::Filter::Files, QDir::SortFlags() );
  std::sort( filesList.begin(), filesList.end(), []( QFileInfo & f1, QFileInfo & f2 )
  {
    QDateTime t1 = f1.fileTime( QFile::FileTime::FileAccessTime );
    if ( !t1.isValid() )
      t1 = f1.fileTime( QFile::FileTime::FileBirthTime );
    QDateTime t2 = f2.fileTime( QFile::FileTime::FileAccessTime );
    if ( !t2.isValid() )
      t2 = f2.fileTime( QFile::FileTime::FileBirthTime );
    return t1 > t2;
  } );
  return filesList;
}
