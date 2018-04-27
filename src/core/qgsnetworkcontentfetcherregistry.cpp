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

QgsNetworkContentFetcherRegistry::QgsNetworkContentFetcherRegistry()
  : QObject()
{
}

QgsNetworkContentFetcherRegistry::~QgsNetworkContentFetcherRegistry()
{
  QMap<QUrl, QgsFetchedContent *>::const_iterator it = mFileRegistry.constBegin();
  for ( ; it != mFileRegistry.constEnd(); ++it )
  {
    delete it.value();
  }
  mFileRegistry.clear();
}

const QgsFetchedContent *QgsNetworkContentFetcherRegistry::fetch( const QUrl &url, const FetchingMode fetchingMode )
{
  QMutexLocker locker( &mMutex );
  if ( mFileRegistry.contains( url ) )
  {
    return mFileRegistry.value( url );
  }

  QgsFetchedContent *content = new QgsFetchedContent( nullptr, QgsFetchedContent::NotStarted );

  // start
  QObject::connect( content, &QgsFetchedContent::downloadStarted, this, [ = ]( const bool redownload )
  {
    if ( redownload && content->status() == QgsFetchedContent::Downloading )
    {
      {
        QMutexLocker locker( &mMutex );
        if ( content->mFetchingTask )
          disconnect( content->mFetchingTask, &QgsNetworkContentFetcherTask::fetched, content, &QgsFetchedContent::taskCompleted );
      }
      // no locker when calling cancel!
      content->cancel();
    }
    QMutexLocker locker( &mMutex );
    if ( redownload ||
         content->status() == QgsFetchedContent::NotStarted ||
         content->status() == QgsFetchedContent::Failed )
    {
      content->mFetchingTask = new QgsNetworkContentFetcherTask( url );
      connect( content->mFetchingTask, &QgsNetworkContentFetcherTask::fetched, content, &QgsFetchedContent::taskCompleted );
      QgsApplication::instance()->taskManager()->addTask( content->mFetchingTask );
      content->mStatus = QgsFetchedContent::Downloading;
    }
  } );

  // cancel
  QObject::connect( content, &QgsFetchedContent::cancelTriggered, this, [ = ]()
  {
    QMutexLocker locker( &mMutex );
    if ( content->mFetchingTask && content->mFetchingTask->canCancel() )
    {
      content->mFetchingTask->cancel();
    }
    if ( content->mFile )
    {
      content->mFile->deleteLater();
      content->mFilePath = QString();
    }
  } );

  // finished
  connect( content, &QgsFetchedContent::taskCompleted, this, [ = ]()
  {
    QMutexLocker locker( &mMutex );
    if ( !content->mFetchingTask || !content->mFetchingTask->reply() )
    {
      // if no reply, it has been canceled
      content->mStatus = QgsFetchedContent::Failed;
      content->mError = QNetworkReply::OperationCanceledError;
      content->mFilePath = QString();
    }
    else
    {
      QNetworkReply *reply = content->mFetchingTask->reply();
      if ( reply->error() == QNetworkReply::NoError )
      {
        QTemporaryFile *tf = new QTemporaryFile( QStringLiteral( "XXXXXX" ) );
        content->mFile = tf;
        tf->open();
        content->mFile->write( reply->readAll() );
        // Qt docs notes that on some system if fileName is not called before close, file might get deleted
        content->mFilePath = tf->fileName();
        tf->close();
        content->mStatus = QgsFetchedContent::Finished;
      }
      else
      {
        content->mStatus = QgsFetchedContent::Failed;
        content->mError = reply->error();
        content->mFilePath = QString();
      }
    }
    content->emitFetched();
  } );

  mFileRegistry.insert( url, content );

  if ( fetchingMode == DownloadImmediately )
    content->download();

  return content;
}

QFile *QgsNetworkContentFetcherRegistry::localFile( const QString &filePathOrUrl )
{
  QFile *file = nullptr;
  QString path = filePathOrUrl;

  if ( !QUrl::fromUserInput( filePathOrUrl ).isLocalFile() )
  {
    QMutexLocker locker( &mMutex );
    if ( mFileRegistry.contains( QUrl( path ) ) )
    {
      const QgsFetchedContent *content = mFileRegistry.value( QUrl( path ) );
      if ( content->status() == QgsFetchedContent::Finished && !content->file() )
      {
        file = content->file();
      }
      else
      {
        // if the file is not downloaded yet or has failed, return nullptr
      }
    }
    else
    {
      // if registry doesn't contain the URL, return nullptr
    }
  }
  else
  {
    file = new QFile( filePathOrUrl );
  }
  return file;
}

QString QgsNetworkContentFetcherRegistry::localPath( const QString &filePathOrUrl )
{
  QString path = filePathOrUrl;

  if ( !QUrl::fromUserInput( filePathOrUrl ).isLocalFile() )
  {
    QMutexLocker locker( &mMutex );
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
        path = QString();
      }
    }
    else
    {
      // if registry doesn't contain the URL, keep path unchanged
    }
  }
  return path;
}



