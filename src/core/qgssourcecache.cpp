/***************************************************************************
                         qgssourcecache.cpp
                         -----------------
    begin                : July 2020
    copyright            : (C) 2020 by Mathieu Pellerin
    email                : nirvn dot asia at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgssourcecache.h"

#include "qgis.h"
#include "qgslogger.h"

#include <QFile>
#include <QBuffer>
#include <QTemporaryDir>

///@cond PRIVATE

QgsSourceCacheEntry::QgsSourceCacheEntry( const QString &path )
  : QgsAbstractContentCacheEntry( path )
{
}

bool QgsSourceCacheEntry::isEqual( const QgsAbstractContentCacheEntry *other ) const
{
  const QgsSourceCacheEntry *otherSource = dynamic_cast< const QgsSourceCacheEntry * >( other );
  // cheapest checks first!
  if ( !otherSource || otherSource->filePath != filePath )
    return false;

  return true;
}

int QgsSourceCacheEntry::dataSize() const
{
  return filePath.size();
}

void QgsSourceCacheEntry::dump() const
{
  QgsDebugMsgLevel( QStringLiteral( "path: %1" ).arg( path ), 3 );
}

///@endcond

QgsSourceCache::QgsSourceCache( QObject *parent )
  : QgsAbstractContentCache< QgsSourceCacheEntry >( parent, QObject::tr( "Source" ) )
{
  temporaryDir.reset( new QTemporaryDir() );

  connect( this, &QgsAbstractContentCacheBase::remoteContentFetched, this, &QgsSourceCache::remoteSourceFetched );
}

QString QgsSourceCache::localFilePath( const QString &path, bool blocking )
{
  const QString file = path.trimmed();
  if ( file.isEmpty() )
    return QString();

  const QMutexLocker locker( &mMutex );

  QgsSourceCacheEntry *currentEntry = findExistingEntry( new QgsSourceCacheEntry( file ) );

  //if current entry's temporary file is empty, create it
  if ( currentEntry->filePath.isEmpty() )
  {
    bool isBroken;
    const QString filePath = fetchSource( file, isBroken, blocking );
    currentEntry->filePath = filePath;
  }

  return currentEntry->filePath;
}

QString QgsSourceCache::fetchSource( const QString &path, bool &isBroken, bool blocking ) const
{
  QString filePath;

  if ( !path.startsWith( QLatin1String( "base64:" ) ) && QFile::exists( path ) )
  {
    filePath = path;
  }
  else
  {
    const QByteArray ba = getContent( path, QByteArray( "broken" ), QByteArray( "fetching" ), blocking );

    if ( ba == "broken" )
    {
      isBroken = true;
    }
    else
    {
      int id = 1;
      filePath = temporaryDir->filePath( QString::number( id ) );
      while ( QFile::exists( filePath ) )
        filePath = temporaryDir->filePath( QString::number( ++id ) );

      QFile file( filePath );
      file.open( QIODevice::WriteOnly );
      file.write( ba );
      file.close();
    }
  }

  return filePath;
}
