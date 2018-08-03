/***************************************************************************
                        qgsservercachemanager.cpp
                        -------------------------

  begin                : 2018-07-05
  copyright            : (C) 2018 by René-Luc D'Hont
  email                : rldhont at 3liz dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsservercachemanager.h"

QByteArray QgsServerCacheManager::getCachedDocument( const QgsProject *project, const QgsServerRequest &request, const QString &key ) const
{
  QgsServerCacheFilterMap::const_iterator scIterator;
  for ( scIterator = mPluginsServerCaches->constBegin(); scIterator != mPluginsServerCaches->constEnd(); ++scIterator )
  {
    QByteArray content = scIterator.value()->getCachedDocument( project, request, key );
    if ( !content.isEmpty() )
    {
      return content;
    }
  }
  return QByteArray();
}

bool QgsServerCacheManager::setCachedDocument( const QDomDocument *doc, const QgsProject *project, const QgsServerRequest &request, const QString &key ) const
{
  QgsServerCacheFilterMap::const_iterator scIterator;
  for ( scIterator = mPluginsServerCaches->constBegin(); scIterator != mPluginsServerCaches->constEnd(); ++scIterator )
  {
    if ( scIterator.value()->setCachedDocument( doc, project, request, key ) )
    {
      return true;
    }
  }
  return false;
}

bool QgsServerCacheManager::deleteCachedDocument( const QgsProject *project, const QgsServerRequest &request, const QString &key ) const
{
  QgsServerCacheFilterMap::const_iterator scIterator;
  for ( scIterator = mPluginsServerCaches->constBegin(); scIterator != mPluginsServerCaches->constEnd(); ++scIterator )
  {
    if ( scIterator.value()->deleteCachedDocument( project, request, key ) )
    {
      return true;
    }
  }
  return false;
}

bool QgsServerCacheManager::deleteCachedDocuments( const QgsProject *project ) const
{
  QgsServerCacheFilterMap::const_iterator scIterator;
  for ( scIterator = mPluginsServerCaches->constBegin(); scIterator != mPluginsServerCaches->constEnd(); ++scIterator )
  {
    if ( scIterator.value()->deleteCachedDocuments( project ) )
    {
      return true;
    }
  }
  return false;
}

QByteArray QgsServerCacheManager::getCachedImage( const QgsProject *project, const QgsServerRequest &request, const QString &key ) const
{
  QgsServerCacheFilterMap::const_iterator scIterator;
  for ( scIterator = mPluginsServerCaches->constBegin(); scIterator != mPluginsServerCaches->constEnd(); ++scIterator )
  {
    QByteArray content = scIterator.value()->getCachedImage( project, request, key );
    if ( !content.isEmpty() )
    {
      return content;
    }
  }
  return QByteArray();
}

bool QgsServerCacheManager::setCachedImage( const QByteArray *img, const QgsProject *project, const QgsServerRequest &request, const QString &key ) const
{
  QgsServerCacheFilterMap::const_iterator scIterator;
  for ( scIterator = mPluginsServerCaches->constBegin(); scIterator != mPluginsServerCaches->constEnd(); ++scIterator )
  {
    if ( scIterator.value()->setCachedImage( img, project, request, key ) )
    {
      return true;
    }
  }
  return false;
}

bool QgsServerCacheManager::deleteCachedImage( const QgsProject *project, const QgsServerRequest &request, const QString &key ) const
{
  QgsServerCacheFilterMap::const_iterator scIterator;
  for ( scIterator = mPluginsServerCaches->constBegin(); scIterator != mPluginsServerCaches->constEnd(); ++scIterator )
  {
    if ( scIterator.value()->deleteCachedImage( project, request, key ) )
    {
      return true;
    }
  }
  return false;
}

bool QgsServerCacheManager::deleteCachedImages( const QgsProject *project ) const
{
  QgsServerCacheFilterMap::const_iterator scIterator;
  for ( scIterator = mPluginsServerCaches->constBegin(); scIterator != mPluginsServerCaches->constEnd(); ++scIterator )
  {
    if ( scIterator.value()->deleteCachedImages( project ) )
    {
      return true;
    }
  }
  return false;
}

void QgsServerCacheManager::registerServerCache( QgsServerCacheFilter *serverCache, int priority )
{
  mPluginsServerCaches->insert( priority, serverCache );
}
