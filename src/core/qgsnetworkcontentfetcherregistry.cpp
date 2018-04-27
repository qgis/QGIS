/***************************************************************************
                       qgsnetworkcontentfetcherregistry.cpp
                             -------------------
    begin                : April, 2018
    copyright            : (C) 2018 by Denis Rouzaud
    email                : denis@opengis.ch

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsnetworkcontentfetcherregistry.h"

#include "qgsapplication.h"
#include "qgsnetworkcontentfetchertask.h"

QgsNetworkContentFetcherRegistry::QgsNetworkContentFetcherRegistry()
  : QObject()
{
}

QgsNetworkContentFetcherRegistry::~QgsNetworkContentFetcherRegistry()
{
  QMap<QUrl, QgsFetchedContent *>::const_iterator it = mFileRegistry.constBegin();
  for ( ; it != mFileRegistry.constEnd(); ++it )
  {
    it.value()->mFile->close();
    delete it.value()->mFile;
  }
  mFileRegistry.clear();
}

const QgsFetchedContent *QgsNetworkContentFetcherRegistry::fetch( const QUrl &url, const FetchingMode &fetchingMode )
{
  if ( mFileRegistry.contains( url ) )
  {
    return mFileRegistry.value( url );
  }

  QgsFetchedContent *content = new QgsFetchedContent( nullptr, QgsFetchedContent::NotStarted );
  QgsNetworkContentFetcherTask *fetcher = new QgsNetworkContentFetcherTask( url );

  QObject::connect( content, &QgsFetchedContent::downloadStarted, this, [ = ]( const bool redownload )
  {
    QMutexLocker locker( &mMutex );
    if ( mFileRegistry.contains( url ) && redownload )
    {
      const QgsFetchedContent *content = mFileRegistry[url];
      if ( mFileRegistry.value( url )->status() == QgsFetchedContent::Downloading && content->mFetchingTask )
      {
        content->mFetchingTask->cancel();
      }
      if ( content->mFile )
      {
        content->mFile->deleteLater();
        mFileRegistry[url]->setFilePath( QStringLiteral() );
      }
    }
    if ( ( mFileRegistry.contains( url ) && mFileRegistry.value( url )->status() == QgsFetchedContent::NotStarted ) || redownload )
    {
      QgsApplication::instance()->taskManager()->addTask( fetcher );
    }
  } );

  QObject::connect( fetcher, &QgsNetworkContentFetcherTask::fetched, this, [ = ]()
  {
    QMutexLocker locker( &mMutex );
    QNetworkReply *reply = fetcher->reply();
    QgsFetchedContent *content = mFileRegistry.value( url );
    if ( reply->error() == QNetworkReply::NoError )
    {
      QTemporaryFile *tf = new QTemporaryFile( QStringLiteral( "XXXXXX" ) );
      content->setFile( tf );
      tf->open();
      content->mFile->write( reply->readAll() );
      // Qt docs notes that on some system if fileName is not called before close, file might get deleted
      content->setFilePath( tf->fileName() );
      tf->close();
      content->setStatus( QgsFetchedContent::Finished );
    }
    else
    {
      content->setStatus( QgsFetchedContent::Failed );
      content->setError( reply->error() );
      content->setFilePath( QStringLiteral() );
    }
    content->emitFetched();
  } );

  if ( fetchingMode == DownloadImmediately )
    content->download();
  mFileRegistry.insert( url, content );

  return content;
}

QString QgsNetworkContentFetcherRegistry::localPath( const QString &filePathOrUrl )
{
  QString path = filePathOrUrl;

  if ( !QUrl::fromUserInput( filePathOrUrl ).isLocalFile() )
  {
    if ( mFileRegistry.contains( QUrl( path ) ) )
    {
      const QgsFetchedContent *content = mFileRegistry.value( QUrl( path ) );
      if ( content->status() == QgsFetchedContent::Finished && !content->filePath().isEmpty() )
      {
        path = content->filePath();
      }
      else
      {
        // if the file is not downloaded yet or has failed, return empty string
        path = QStringLiteral();
      }
    }
    else
    {
      // if registry doesn't contain the URL, keep path unchanged
    }
  }
  return path;
}



