/***************************************************************************
                       qgsnetworkcontentfetcher.cpp
                             -------------------
    begin                : July, 2014
    copyright            : (C) 2014 by Nyall Dawson
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

#include "qgsnetworkcontentfetcher.h"
#include "qgsnetworkaccessmanager.h"
#include "qgsmessagelog.h"
#include "qgsapplication.h"
#include <QNetworkReply>

QgsNetworkContentFetcher::QgsNetworkContentFetcher()
    : mReply( 0 )
    , mContentLoaded( false )
{

}

QgsNetworkContentFetcher::~QgsNetworkContentFetcher()
{
  delete mReply;
}

void QgsNetworkContentFetcher::fetchContent( const QUrl url )
{
  QUrl nextUrlToFetch = url;
  mContentLoaded = false;

  //get contents
  QNetworkRequest request( nextUrlToFetch );

  mReply = QgsNetworkAccessManager::instance()->get( request );
  connect( mReply, SIGNAL( finished() ), this, SLOT( contentLoaded() ) );
}

QNetworkReply *QgsNetworkContentFetcher::reply()
{
  if ( !mContentLoaded )
  {
    return 0;
  }

  return mReply;
}

QString QgsNetworkContentFetcher::contentAsString() const
{
  if ( !mContentLoaded || !mReply )
  {
    return QString();
  }

  QByteArray array = mReply->readAll();
  return QString( array );
}

void QgsNetworkContentFetcher::contentLoaded( bool ok )
{
  Q_UNUSED( ok );

  if ( mReply->error() != QNetworkReply::NoError )
  {
    QgsMessageLog::logMessage( tr( "HTTP fetch %1 failed with error %2" ).arg( mReply->url().toString() ).arg( mReply->errorString() ) );
    mContentLoaded = true;
    emit finished();
    return;
  }

  QVariant redirect = mReply->attribute( QNetworkRequest::RedirectionTargetAttribute );
  if ( redirect.isNull() )
  {
    //no error or redirect, got target
    QVariant status = mReply->attribute( QNetworkRequest::HttpStatusCodeAttribute );
    if ( !status.isNull() && status.toInt() >= 400 )
    {
      QgsMessageLog::logMessage( tr( "HTTP fetch %1 failed with error %2" ).arg( mReply->url().toString() ).arg( status.toString() ) );
    }
    mContentLoaded = true;
    emit finished();
    return;
  }

  //redirect, so fetch redirect target
  mReply->deleteLater();
  fetchContent( redirect.toUrl() );
}




