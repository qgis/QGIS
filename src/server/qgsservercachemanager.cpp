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

//! Returns cached document (or 0 if document not in cache) like capabilities
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

//! Updates or inserts the document in cache like capabilities
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

//! Deletes the cached document
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

//! Deletes all cached documents for a QGIS Project
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

//! Register a new access control filter
void QgsServerCacheManager::registerServerCache( QgsServerCacheFilter *serverCache, int priority )
{
  mPluginsServerCaches->insert( priority, serverCache );
}
