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
#include "qgsauthmanager.h"
#include <QNetworkReply>
#include <QTextCodec>

QgsNetworkContentFetcher::~QgsNetworkContentFetcher()
{
  if ( mReply && mReply->isRunning() )
  {
    //cancel running request
    mReply->abort();
  }
  delete mReply;
}

void QgsNetworkContentFetcher::fetchContent( const QUrl &url, const QString &authcfg )
{
  QNetworkRequest req( url );
  QgsSetRequestInitiatorClass( req, QStringLiteral( "QgsNetworkContentFetcher" ) );

  fetchContent( req, authcfg );
}

void QgsNetworkContentFetcher::fetchContent( const QNetworkRequest &r, const QString &authcfg )
{
  QNetworkRequest request( r );

  mAuthCfg = authcfg;
  if ( !mAuthCfg.isEmpty() )
  {
    QgsApplication::authManager()->updateNetworkRequest( request, mAuthCfg );
  }

  mContentLoaded = false;
  mIsCanceled = false;

  if ( mReply )
  {
    //cancel any in progress requests
    mReply->abort();
    mReply->deleteLater();
    mReply = nullptr;
  }

  mReply = QgsNetworkAccessManager::instance()->get( request );
  if ( !mAuthCfg.isEmpty() )
  {
    QgsApplication::authManager()->updateNetworkReply( mReply, mAuthCfg );
  }
  mReply->setParent( nullptr ); // we don't want thread locale QgsNetworkAccessManagers to delete the reply - we want ownership of it to belong to this object
  connect( mReply, &QNetworkReply::finished, this, [ = ] { contentLoaded(); } );
  connect( mReply, &QNetworkReply::downloadProgress, this, &QgsNetworkContentFetcher::downloadProgress );

  auto onError = [ = ]( QNetworkReply::NetworkError code )
  {
    // could have been canceled in the meantime
    if ( mReply )
      emit errorOccurred( code, mReply->errorString() );
  };

#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
  connect( mReply, qOverload<QNetworkReply::NetworkError>( &QNetworkReply::error ), this, onError );
#else
  connect( mReply, &QNetworkReply::errorOccurred, this, onError );
#endif
}

QNetworkReply *QgsNetworkContentFetcher::reply()
{
  if ( !mContentLoaded )
  {
    return nullptr;
  }

  return mReply;
}

QString QgsNetworkContentFetcher::contentDispositionFilename() const
{
  return mReply ? QgsNetworkReplyContent::extractFilenameFromContentDispositionHeader( mReply ) : QString();
}

QString QgsNetworkContentFetcher::contentAsString() const
{
  if ( !mContentLoaded || !mReply )
  {
    return QString();
  }

  QByteArray array = mReply->readAll();

  //correctly encode reply as unicode
  QTextCodec *codec = codecForHtml( array );
  return codec->toUnicode( array );
}

void QgsNetworkContentFetcher::cancel()
{
  mIsCanceled = true;

  if ( mReply )
  {
    //cancel any in progress requests
    mReply->abort();
    mReply->deleteLater();
    mReply = nullptr;
  }
}

bool QgsNetworkContentFetcher::wasCanceled() const
{
  return mIsCanceled;
}

QTextCodec *QgsNetworkContentFetcher::codecForHtml( QByteArray &array ) const
{
  //QTextCodec::codecForHtml fails to detect "<meta charset="utf-8"/>" type tags
  //see https://bugreports.qt.io/browse/QTBUG-41011
  //so test for that ourselves

  //basic check
  QTextCodec *codec = QTextCodec::codecForUtfText( array, nullptr );
  if ( codec )
  {
    return codec;
  }

  //check for meta charset tag
  const QByteArray header = array.left( 1024 ).toLower();
  int pos = header.indexOf( "meta charset=" );
  if ( pos != -1 )
  {
    pos += int( strlen( "meta charset=" ) ) + 1;
    const int pos2 = header.indexOf( '\"', pos );
    const QByteArray cs = header.mid( pos, pos2 - pos );
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
  Q_UNUSED( ok )

  if ( mIsCanceled )
  {
    emit finished();
    return;
  }

  if ( mReply->error() != QNetworkReply::NoError )
  {
    QgsMessageLog::logMessage( tr( "HTTP fetch %1 failed with error %2" ).arg( mReply->url().toString(), mReply->errorString() ) );
    mContentLoaded = true;
    emit finished();
    return;
  }

  const QVariant redirect = mReply->attribute( QNetworkRequest::RedirectionTargetAttribute );
  if ( redirect.isNull() )
  {
    //no error or redirect, got target
    const QVariant status = mReply->attribute( QNetworkRequest::HttpStatusCodeAttribute );
    if ( !status.isNull() && status.toInt() >= 400 )
    {
      QgsMessageLog::logMessage( tr( "HTTP fetch %1 failed with error %2" ).arg( mReply->url().toString(), status.toString() ) );
    }
    mContentLoaded = true;
    emit finished();
    return;
  }

  //redirect, so fetch redirect target
  mReply->deleteLater();
  fetchContent( redirect.toUrl(), mAuthCfg );
}
