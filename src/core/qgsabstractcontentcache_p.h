/***************************************************************************
                         qgsabstractcontentcache_p.h
                         ---------------
    begin                : February 2024
    copyright            : (C) 2024 by Matthias Kuhn
    email                : matthias@opengis.ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSABSTRACTCONTENTCACHE_P_H
#define QGSABSTRACTCONTENTCACHE_P_H

#include "qgsabstractcontentcache.h"
#include "qgssetrequestinitiator_p.h"

template<class T>
QByteArray QgsAbstractContentCache<T>::getContent( const QString &path, const QByteArray &missingContent, const QByteArray &fetchingContent, bool blocking ) const
{
  // is it a path to local file?
  QFile file( path );
  if ( file.exists() )
  {
    if ( file.open( QIODevice::ReadOnly ) )
    {
      return file.readAll();
    }
    else
    {
      return missingContent;
    }
  }

  // maybe it's an embedded base64 string
  if ( path.startsWith( QLatin1String( "base64:" ), Qt::CaseInsensitive ) )
  {
    const QByteArray base64 = path.mid( 7 ).toLocal8Bit(); // strip 'base64:' prefix
    return QByteArray::fromBase64( base64, QByteArray::OmitTrailingEquals );
  }

  // maybe it's a url...
  if ( !path.contains( QLatin1String( "://" ) ) ) // otherwise short, relative SVG paths might be considered URLs
  {
    return missingContent;
  }

  const QUrl url( path );
  if ( !url.isValid() )
  {
    return missingContent;
  }

  // check whether it's a url pointing to a local file
  if ( url.scheme().compare( QLatin1String( "file" ), Qt::CaseInsensitive ) == 0 )
  {
    file.setFileName( url.toLocalFile() );
    if ( file.exists() )
    {
      if ( file.open( QIODevice::ReadOnly ) )
      {
        return file.readAll();
      }
    }

    // not found...
    return missingContent;
  }

  const QMutexLocker locker( &mMutex );

  // already a request in progress for this url
  if ( mPendingRemoteUrls.contains( path ) )
  {
    // it's a non blocking request so return fetching content
    if ( !blocking )
    {
      return fetchingContent;
    }

    // it's a blocking request so try to find the task and wait for task finished
    const auto constActiveTasks = QgsApplication::taskManager()->activeTasks();
    for ( QgsTask *task : constActiveTasks )
    {
      // the network content fetcher task's description ends with the path
      if ( !task->description().endsWith( path ) )
      {
        continue;
      }

      // cast task to network content fetcher task
      QgsNetworkContentFetcherTask *ncfTask = qobject_cast<QgsNetworkContentFetcherTask *>( task );
      if ( ncfTask )
      {
        // wait for task finished
        if ( waitForTaskFinished( ncfTask ) )
        {
          if ( mRemoteContentCache.contains( path ) )
          {
            // We got the file!
            return *mRemoteContentCache[ path ];
          }
        }
      }
      // task found, no needs to continue
      break;
    }
    // if no content returns the content is probably in remote content cache
    // or a new task will be created
  }

  if ( mRemoteContentCache.contains( path ) )
  {
    // already fetched this content - phew. Just return what we already got.
    return *mRemoteContentCache[ path ];
  }

  mPendingRemoteUrls.insert( path );
  //fire up task to fetch content in background
  QNetworkRequest request( url );
  QgsSetRequestInitiatorClass( request, QStringLiteral( "QgsAbstractContentCache<%1>" ).arg( mTypeString ) );
  request.setAttribute( QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::PreferCache );
  request.setAttribute( QNetworkRequest::CacheSaveControlAttribute, true );

  QgsNetworkContentFetcherTask *task = new QgsNetworkContentFetcherTask( request );
  connect( task, &QgsNetworkContentFetcherTask::fetched, this, [this, task, path, missingContent]
  {
    const QMutexLocker locker( &mMutex );

    QNetworkReply *reply = task->reply();
    if ( !reply )
    {
      // canceled
      QMetaObject::invokeMethod( const_cast< QgsAbstractContentCacheBase * >( qobject_cast< const QgsAbstractContentCacheBase * >( this ) ), "onRemoteContentFetched", Qt::QueuedConnection, Q_ARG( QString, path ), Q_ARG( bool, false ) );
      return;
    }

    if ( reply->error() != QNetworkReply::NoError )
    {
      QgsMessageLog::logMessage( tr( "%3 request failed [error: %1 - url: %2]" ).arg( reply->errorString(), path, mTypeString ), mTypeString );
      return;
    }

    bool ok = true;

    const QVariant status = reply->attribute( QNetworkRequest::HttpStatusCodeAttribute );
    if ( !QgsVariantUtils::isNull( status ) && status.toInt() >= 400 )
    {
      const QVariant phrase = reply->attribute( QNetworkRequest::HttpReasonPhraseAttribute );
      QgsMessageLog::logMessage( tr( "%4 request error [status: %1 - reason phrase: %2] for %3" ).arg( status.toInt() ).arg( phrase.toString(), path, mTypeString ), mTypeString );
      mRemoteContentCache.insert( path, new QByteArray( missingContent ) );
      ok = false;
    }

    if ( !checkReply( reply, path ) )
    {
      mRemoteContentCache.insert( path, new QByteArray( missingContent ) );
      ok = false;
    }

    if ( ok )
    {
      // read the content data
      const QByteArray ba = reply->readAll();

      // because of the fragility listed below in waitForTaskFinished, this slot may get called twice. In that case
      // the second time will have an empty reply (we've already read it all...)
      if ( !ba.isEmpty() )
        mRemoteContentCache.insert( path, new QByteArray( ba ) );
    }
    QMetaObject::invokeMethod( const_cast< QgsAbstractContentCacheBase * >( qobject_cast< const QgsAbstractContentCacheBase * >( this ) ), "onRemoteContentFetched", Qt::QueuedConnection, Q_ARG( QString, path ), Q_ARG( bool, true ) );
  } );

  QgsApplication::taskManager()->addTask( task );

  // if blocking, wait for finished
  if ( blocking )
  {
    if ( waitForTaskFinished( task ) )
    {
      if ( mRemoteContentCache.contains( path ) )
      {
        // We got the file!
        return *mRemoteContentCache[ path ];
      }
    }
  }
  return fetchingContent;
}

#endif // QGSABSTRACTCONTENTCACHE_P_H
