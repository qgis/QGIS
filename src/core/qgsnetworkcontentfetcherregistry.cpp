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

QgsNetworkContentFetcherRegistry::~QgsNetworkContentFetcherRegistry()
{
  QMap<QString, QgsFetchedContent *>::const_iterator it = mFileRegistry.constBegin();
  for ( ; it != mFileRegistry.constEnd(); ++it )
  {
    delete it.value();
  }
  mFileRegistry.clear();
}

const QgsFetchedContent *QgsNetworkContentFetcherRegistry::fetch( const QString &url, const FetchingMode fetchingMode )
{

  if ( mFileRegistry.contains( url ) )
  {
    return mFileRegistry.value( url );
  }

  QgsFetchedContent *content = new QgsFetchedContent( url, nullptr, QgsFetchedContent::NotStarted );

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
    if ( mFileRegistry.contains( path ) )
    {
      const QgsFetchedContent *content = mFileRegistry.value( path );
      if ( content && content->status() == QgsFetchedContent::Finished && content->file() )
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
    if ( mFileRegistry.contains( path ) )
    {
      const QgsFetchedContent *content = mFileRegistry.value( path );
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




void QgsFetchedContent::download( bool redownload )
{

  if ( redownload && status() == QgsFetchedContent::Downloading )
  {
    {
      if ( mFetchingTask )
        disconnect( mFetchingTask, &QgsNetworkContentFetcherTask::taskCompleted, this, &QgsFetchedContent::taskCompleted );
    }
    cancel();
  }
  if ( redownload ||
       status() == QgsFetchedContent::NotStarted ||
       status() == QgsFetchedContent::Failed )
  {
    mFetchingTask = new QgsNetworkContentFetcherTask( mUrl );
    // use taskCompleted which is main thread rather than fetched signal in worker thread
    connect( mFetchingTask, &QgsNetworkContentFetcherTask::taskCompleted, this, &QgsFetchedContent::taskCompleted );
    QgsApplication::instance()->taskManager()->addTask( mFetchingTask );
    mStatus = QgsFetchedContent::Downloading;
  }

}

void QgsFetchedContent::cancel()
{
  if ( mFetchingTask && mFetchingTask->canCancel() )
  {
    mFetchingTask->cancel();
  }
  if ( mFile )
  {
    mFile->deleteLater();
    mFilePath = QString();
  }
}


void QgsFetchedContent::taskCompleted()
{
  if ( !mFetchingTask || !mFetchingTask->reply() )
  {
    // if no reply, it has been canceled
    mStatus = QgsFetchedContent::Failed;
    mError = QNetworkReply::OperationCanceledError;
    mFilePath = QString();
  }
  else
  {
    QNetworkReply *reply = mFetchingTask->reply();
    if ( reply->error() == QNetworkReply::NoError )
    {
      QTemporaryFile *tf = new QTemporaryFile( QStringLiteral( "XXXXXX" ) );
      mFile = tf;
      tf->open();
      mFile->write( reply->readAll() );
      // Qt docs notes that on some system if fileName is not called before close, file might get deleted
      mFilePath = tf->fileName();
      tf->close();
      mStatus = QgsFetchedContent::Finished;
    }
    else
    {
      mStatus = QgsFetchedContent::Failed;
      mError = reply->error();
      mFilePath = QString();
    }
  }

  emit fetched();
}
