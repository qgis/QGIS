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
#include <QTextCodec>

QgsNetworkContentFetcher::QgsNetworkContentFetcher()
    : mReply( 0 )
    , mContentLoaded( false )
{

}

QgsNetworkContentFetcher::~QgsNetworkContentFetcher()
{
  if ( mReply && mReply->isRunning() )
  {
    //cancel running request
    mReply->abort();
  }
  if ( mReply )
  {
    mReply->deleteLater();
  }
}

void QgsNetworkContentFetcher::fetchContent( const QUrl url )
{
  QUrl nextUrlToFetch = url;
  mContentLoaded = false;

  //get contents
  QNetworkRequest request( nextUrlToFetch );

  if ( mReply )
  {
    //cancel any in progress requests
    mReply->abort();
    mReply->deleteLater();
    mReply = 0;
  }

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

  //correctly encode reply as unicode
  QTextCodec* codec = codecForHtml( array );
  return codec->toUnicode( array );
}

QTextCodec* QgsNetworkContentFetcher::codecForHtml( QByteArray& array ) const
{
  //QTextCodec::codecForHtml fails to detect "<meta charset="utf-8"/>" type tags
  //see https://bugreports.qt-project.org/browse/QTBUG-41011
  //so test for that ourselves

  //basic check
  QTextCodec* codec = QTextCodec::codecForUtfText( array, 0 );
  if ( codec )
  {
    return codec;
  }

  //check for meta charset tag
  QByteArray header = array.left( 1024 ).toLower();
  int pos = header.indexOf( "meta charset=" );
  if ( pos != -1 )
  {
    pos += int( strlen( "meta charset=" ) ) + 1;
    int pos2 = header.indexOf( '\"', pos );
    QByteArray cs = header.mid( pos, pos2 - pos );
    codec = QTextCodec::codecForName( cs );
    if ( codec )
    {
      return codec;
    }
  }

  //fallback to QTextCodec::codecForHtml
  codec = QTextCodec::codecForHtml( array, codec );
  if ( codec )
  {
    return codec;
  }

  //no luck, default to utf-8
  return QTextCodec::codecForName( "UTF-8" );
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




