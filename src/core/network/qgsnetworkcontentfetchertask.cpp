/***************************************************************************
                       qgsnetworkcontentfetchertask.cpp
                             -------------------
    begin                : March 2018
    copyright            : (C) 2018 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsnetworkcontentfetchertask.h"
#include "qgsnetworkcontentfetcher.h"
#include <QEventLoop>

QgsNetworkContentFetcherTask::QgsNetworkContentFetcherTask( const QUrl &url, const QString &authcfg, QgsTask::Flags flags )
  : QgsNetworkContentFetcherTask( QNetworkRequest( url ), authcfg, flags )
{
}

QgsNetworkContentFetcherTask::QgsNetworkContentFetcherTask( const QNetworkRequest &request, const QString &authcfg, QgsTask::Flags flags )
  : QgsTask( tr( "Fetching %1" ).arg( request.url().toString() ), flags )
  , mRequest( request )
  , mAuthcfg( authcfg )
{
}

QgsNetworkContentFetcherTask::~QgsNetworkContentFetcherTask()
{
  if ( mFetcher )
    mFetcher->deleteLater();
}

bool QgsNetworkContentFetcherTask::run()
{
  mFetcher = new QgsNetworkContentFetcher();
  QEventLoop loop;

  // We need to set the event loop (and not 'this') as receiver for all signal to ensure execution
  // in the same thread and in the same order of emission. Indeed 'this' and 'loop' lives in
  // different thread because they have been created in different thread.

  connect( mFetcher, &QgsNetworkContentFetcher::finished, &loop, &QEventLoop::quit );
  connect( mFetcher, &QgsNetworkContentFetcher::downloadProgress, &loop, [ = ]( qint64 bytesReceived, qint64 bytesTotal )
  {
    if ( !isCanceled() && bytesTotal > 0 )
    {
      const int progress = ( bytesReceived * 100 ) / bytesTotal;
      // don't emit 100% progress reports until completely fetched - otherwise we get
      // intermediate 100% reports from redirects
      if ( progress < 100 )
        setProgress( progress );
    }
  } );


  bool hasErrorOccurred = false;
  connect( mFetcher, &QgsNetworkContentFetcher::errorOccurred, &loop, [ &hasErrorOccurred, this ]( QNetworkReply::NetworkError code, const QString & errorMsg )
  {
    hasErrorOccurred = true;
    emit errorOccurred( code, errorMsg );
  } );

  mFetcher->fetchContent( mRequest, mAuthcfg );
  loop.exec();
  if ( !isCanceled() )
    setProgress( 100 );
  emit fetched();

  return !isCanceled() && !hasErrorOccurred;
}

void QgsNetworkContentFetcherTask::cancel()
{
  if ( mFetcher )
    mFetcher->cancel();

  QgsTask::cancel();
}

QNetworkReply *QgsNetworkContentFetcherTask::reply()
{
  return mFetcher ? mFetcher->reply() : nullptr;
}

QString QgsNetworkContentFetcherTask::contentAsString() const
{
  return mFetcher ? mFetcher->contentAsString() : QString();
}
