/***************************************************************************
                       qgsnetworkcontentfetcherregistry.cpp
                             -------------------
    begin                : April, 2018
    copyright            : (C) 2018 by Denis Rouzaud
    email                : denis.rouzaud@gmail.com

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
  QMap<QUrl, FetchedContent>::const_iterator it = mFileRegistry.constBegin();
  for ( ; it != mFileRegistry.constEnd(); ++it )
  {
    it.value().mFile->deleteLater();
  }
}

const QgsNetworkContentFetcherTask *QgsNetworkContentFetcherRegistry::fetch( const QUrl &url, const bool reload )
{
  if ( mFileRegistry.contains( url ) )
  {
    if ( !reload )
    {
      return mFileRegistry.value( url ).mFetchingTask;
    }
    else
    {
      FetchedContent content = mFileRegistry.take( url );
      if ( content.mFetchingTask )
      {
        content.mFetchingTask->cancel();
      }
      if ( content.mFile )
      {
        content.mFile->deleteLater();
      }
    }
  }

  QgsNetworkContentFetcherTask *fetcher = new QgsNetworkContentFetcherTask( url );
  QgsApplication::instance()->taskManager()->addTask( fetcher );
  mFileRegistry.insert( url,
                        FetchedContent( nullptr,
                                        QgsNetworkContentFetcherRegistry::Downloading ) );

  QObject::connect( fetcher, &QgsNetworkContentFetcherTask::fetched, this, [ = ]()
  {
    QNetworkReply *reply = fetcher->reply();
    FetchedContent content = mFileRegistry.take( url );
    if ( reply->error() == QNetworkReply::NoError )
    {
      content.setFile( new QTemporaryFile( QStringLiteral( "XXXXXX" ) ) );
      content.mFile->write( reply->readAll() );
      content.setStatus( Finished );
    }
    else
    {
      content.setStatus( Failed );
      content.setError( reply->error() );
    }
    mFileRegistry.insert( url, content );
  } );


  return fetcher;
}



QgsNetworkContentFetcherRegistry::FetchedContent QgsNetworkContentFetcherRegistry::file( const QUrl &url )
{
  return mFileRegistry.value( url );
}
