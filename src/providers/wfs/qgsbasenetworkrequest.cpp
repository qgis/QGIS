/***************************************************************************
    qgsbasenetworkrequest.cpp
    ---------------------
    begin                : February 2016
    copyright            : (C) 2011 by Martin Dobias
                           (C) 2016 by Even Rouault
    email                : wonder dot sk at gmail dot com
                           even.rouault at spatialys.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsbasenetworkrequest.h"
#include "qgslogger.h"
#include "qgsmessagelog.h"
#include "qgsnetworkaccessmanager.h"
#include "qgsapplication.h"

#include <QEventLoop>
#include <QNetworkCacheMetaData>
#include <QCryptographicHash> // just for testing file:// fake_qgis_http_endpoint hack
#include <QFuture>
#include <QtConcurrent>

QgsBaseNetworkRequest::QgsBaseNetworkRequest( const QgsAuthorizationSettings &auth, const QString &translatedComponent )
  : mAuth( auth )
  , mTranslatedComponent( translatedComponent )
{
  connect( QgsNetworkAccessManager::instance(), qgis::overload< QNetworkReply *>::of( &QgsNetworkAccessManager::requestTimedOut ), this, &QgsBaseNetworkRequest::requestTimedOut );
}

QgsBaseNetworkRequest::~QgsBaseNetworkRequest()
{
  abort();
}

void QgsBaseNetworkRequest::requestTimedOut( QNetworkReply *reply )
{
  if ( reply == mReply )
    mTimedout = true;
}

bool QgsBaseNetworkRequest::sendGET( const QUrl &url, const QString &acceptHeader, bool synchronous, bool forceRefresh, bool cache )
{
  abort(); // cancel previous
  mIsAborted = false;
  mTimedout = false;
  mGotNonEmptyResponse = false;

  mErrorMessage.clear();
  mErrorCode = QgsBaseNetworkRequest::NoError;
  mForceRefresh = forceRefresh;
  mResponse.clear();

  QUrl modifiedUrl( url );

  // Specific code for testing
  if ( modifiedUrl.toString().contains( QLatin1String( "fake_qgis_http_endpoint" ) ) )
  {
    // Just for testing with local files instead of http:// resources
    QString modifiedUrlString = modifiedUrl.toString();
    // Qt5 does URL encoding from some reason (of the FILTER parameter for example)
    modifiedUrlString = QUrl::fromPercentEncoding( modifiedUrlString.toUtf8() );
    QgsDebugMsgLevel( QStringLiteral( "Get %1" ).arg( modifiedUrlString ), 4 );
    modifiedUrlString = modifiedUrlString.mid( QStringLiteral( "http://" ).size() );
#ifdef Q_OS_WIN
    // Passing "urls" like "http://c:/path" to QUrl 'eats' the : after c,
    // so we must restore it
    if ( modifiedUrlString[1] == '/' )
    {
      modifiedUrlString = modifiedUrlString[0] + ":/" + modifiedUrlString.mid( 2 );
    }
#endif

    // For REST API using URL subpaths, normalize the subpaths
    int afterEndpointStartPos = modifiedUrlString.indexOf( "fake_qgis_http_endpoint" ) + strlen( "fake_qgis_http_endpoint" );
    QString afterEndpointStart = modifiedUrlString.mid( afterEndpointStartPos );
    afterEndpointStart.replace( QLatin1String( "/" ), QLatin1String( "_" ) );
    modifiedUrlString = modifiedUrlString.mid( 0, afterEndpointStartPos ) + afterEndpointStart;

    if ( !acceptHeader.isEmpty() )
    {
      if ( modifiedUrlString.indexOf( '?' ) > 0 )
      {
        modifiedUrlString += QStringLiteral( "&Accept=" ) + acceptHeader;
      }
      else
      {
        modifiedUrlString += QStringLiteral( "?Accept=" ) + acceptHeader;
      }
    }
    auto posQuotationMark = modifiedUrlString.indexOf( '?' );
    if ( posQuotationMark > 0 )
    {
      QString args = modifiedUrlString.mid( posQuotationMark );
      if ( modifiedUrlString.size() > 256 )
      {
        QgsDebugMsgLevel( QStringLiteral( "Args before MD5: %1" ).arg( args ), 4 );
        args = QCryptographicHash::hash( args.toUtf8(), QCryptographicHash::Md5 ).toHex();
      }
      else
      {
        args.replace( '?', '_' );
        args.replace( '&', '_' );
        args.replace( '<', '_' );
        args.replace( '>', '_' );
        args.replace( '\'', '_' );
        args.replace( '\"', '_' );
        args.replace( ' ', '_' );
        args.replace( ':', '_' );
        args.replace( '/', '_' );
        args.replace( '\n', '_' );
      }
      modifiedUrlString = modifiedUrlString.mid( 0, modifiedUrlString.indexOf( '?' ) ) + args;
    }
    QgsDebugMsgLevel( QStringLiteral( "Get %1 (after laundering)" ).arg( modifiedUrlString ), 4 );
    modifiedUrl = QUrl::fromLocalFile( modifiedUrlString );
  }

  QgsDebugMsgLevel( QStringLiteral( "Calling: %1" ).arg( modifiedUrl.toDisplayString( ) ), 4 );

  QNetworkRequest request( modifiedUrl );
  if ( !acceptHeader.isEmpty() )
  {
    request.setRawHeader( "Accept", acceptHeader.toUtf8() );
  }

  QgsSetRequestInitiatorClass( request, QStringLiteral( "QgsBaseNetworkRequest" ) );
  if ( !mAuth.setAuthorization( request ) )
  {
    mErrorCode = QgsBaseNetworkRequest::NetworkError;
    mErrorMessage = errorMessageFailedAuth();
    logMessageIfEnabled();
    return false;
  }

  if ( cache )
  {
    request.setAttribute( QNetworkRequest::CacheLoadControlAttribute, forceRefresh ? QNetworkRequest::AlwaysNetwork : QNetworkRequest::PreferCache );
    request.setAttribute( QNetworkRequest::CacheSaveControlAttribute, true );
  }

  QWaitCondition waitCondition;
  QMutex waitConditionMutex;

  bool threadFinished = false;
  bool success = false;

  std::function<void()> downloaderFunction = [ this, request, synchronous, &waitConditionMutex, &waitCondition, &threadFinished, &success ]()
  {
    if ( QThread::currentThread() != QgsApplication::instance()->thread() )
      QgsNetworkAccessManager::instance( Qt::DirectConnection );

    success = true;
    mReply = QgsNetworkAccessManager::instance()->get( request );

    if ( !mAuth.setAuthorizationReply( mReply ) )
    {
      mErrorCode = QgsBaseNetworkRequest::NetworkError;
      mErrorMessage = errorMessageFailedAuth();
      logMessageIfEnabled();
      waitCondition.wakeAll();
      success = false;
    }
    else
    {
      // We are able to use direct connection here, because we
      // * either run on the thread mReply lives in, so DirectConnection is standard and safe anyway
      // * or the owner thread of mReply is currently not doing anything because it's blocked in future.waitForFinished() (if it is the main thread)
      connect( mReply, &QNetworkReply::finished, this, &QgsBaseNetworkRequest::replyFinished, Qt::DirectConnection );
      connect( mReply, &QNetworkReply::downloadProgress, this, &QgsBaseNetworkRequest::replyProgress, Qt::DirectConnection );
      connect( mReply, &QNetworkReply::readyRead, this, &QgsBaseNetworkRequest::replyReadyRead, Qt::DirectConnection );

      if ( synchronous )
      {
        auto resumeMainThread = [&waitConditionMutex, &waitCondition]()
        {
          // when this method is called we have "produced" a single authentication request -- so the buffer is now full
          // and it's time for the "consumer" (main thread) to do its part
          waitConditionMutex.lock();
          waitCondition.wakeAll();
          waitConditionMutex.unlock();

          // note that we don't need to handle waking this thread back up - that's done automatically by QgsNetworkAccessManager
        };

        connect( QgsNetworkAccessManager::instance(), &QgsNetworkAccessManager::authRequestOccurred, this, resumeMainThread, Qt::DirectConnection );
        connect( QgsNetworkAccessManager::instance(), &QgsNetworkAccessManager::proxyAuthenticationRequired, this, resumeMainThread, Qt::DirectConnection );

#ifndef QT_NO_SSL
        connect( QgsNetworkAccessManager::instance(), &QgsNetworkAccessManager::sslErrorsOccurred, this, resumeMainThread, Qt::DirectConnection );
#endif
        QEventLoop loop;
        connect( this, &QgsBaseNetworkRequest::downloadFinished, &loop, &QEventLoop::quit, Qt::DirectConnection );
        loop.exec();
      }
    }
    waitConditionMutex.lock();
    threadFinished = true;
    waitCondition.wakeAll();
    waitConditionMutex.unlock();
  };

  if ( synchronous && QThread::currentThread() == QApplication::instance()->thread() )
  {
    std::unique_ptr<DownloaderThread> downloaderThread = qgis::make_unique<DownloaderThread>( downloaderFunction );
    downloaderThread->start();

    while ( true )
    {
      waitConditionMutex.lock();
      if ( threadFinished )
      {
        waitConditionMutex.unlock();
        break;
      }
      waitCondition.wait( &waitConditionMutex );

      // If the downloader thread wakes us (the main thread) up and is not yet finished
      // then it has "produced" an authentication request which we need to now "consume".
      // The processEvents() call gives the auth manager the chance to show a dialog and
      // once done with that, we can wake the downloaderThread again and continue the download.
      if ( !threadFinished )
      {
        waitConditionMutex.unlock();

        QgsApplication::instance()->processEvents();
        // we don't need to wake up the worker thread - it will automatically be woken when
        // the auth request has been dealt with by QgsNetworkAccessManager
      }
      else
      {
        waitConditionMutex.unlock();
      }
    }
    // wait for thread to gracefully exit
    downloaderThread->wait();
  }
  else
  {
    downloaderFunction();
  }
  return success && mErrorMessage.isEmpty();
}

bool QgsBaseNetworkRequest::sendPOST( const QUrl &url, const QString &contentTypeHeader, const QByteArray &data )
{
  abort(); // cancel previous
  mIsAborted = false;
  mTimedout = false;
  mGotNonEmptyResponse = false;

  mErrorMessage.clear();
  mErrorCode = QgsBaseNetworkRequest::NoError;
  mForceRefresh = true;
  mResponse.clear();

  if ( url.toEncoded().contains( "fake_qgis_http_endpoint" ) )
  {
    // Hack for testing purposes
    QUrl modifiedUrl( url );
    modifiedUrl.addQueryItem( QStringLiteral( "POSTDATA" ), QString::fromUtf8( data ) );
    return sendGET( modifiedUrl, QString(), true, true, false );
  }

  QNetworkRequest request( url );
  QgsSetRequestInitiatorClass( request, QStringLiteral( "QgsBaseNetworkRequest" ) );
  if ( !mAuth.setAuthorization( request ) )
  {
    mErrorCode = QgsBaseNetworkRequest::NetworkError;
    mErrorMessage = errorMessageFailedAuth();
    logMessageIfEnabled();
    return false;
  }
  request.setHeader( QNetworkRequest::ContentTypeHeader, contentTypeHeader );

  mReply = QgsNetworkAccessManager::instance()->post( request, data );
  if ( !mAuth.setAuthorizationReply( mReply ) )
  {
    mErrorCode = QgsBaseNetworkRequest::NetworkError;
    mErrorMessage = errorMessageFailedAuth();
    logMessageIfEnabled();
    return false;
  }
  connect( mReply, &QNetworkReply::finished, this, &QgsBaseNetworkRequest::replyFinished );
  connect( mReply, &QNetworkReply::downloadProgress, this, &QgsBaseNetworkRequest::replyProgress );
  connect( mReply, &QNetworkReply::readyRead, this, &QgsBaseNetworkRequest::replyReadyRead );

  QEventLoop loop;
  connect( this, &QgsBaseNetworkRequest::downloadFinished, &loop, &QEventLoop::quit );
  loop.exec( QEventLoop::ExcludeUserInputEvents );

  return mErrorMessage.isEmpty();
}

void QgsBaseNetworkRequest::abort()
{
  mIsAborted = true;
  if ( mReply )
  {
    mReply->deleteLater();
    mReply = nullptr;
  }
}

void QgsBaseNetworkRequest::replyReadyRead( )
{
  mGotNonEmptyResponse = true;
}

void QgsBaseNetworkRequest::replyProgress( qint64 bytesReceived, qint64 bytesTotal )
{
  QgsDebugMsgLevel( QStringLiteral( "%1 of %2 bytes downloaded." ).arg( bytesReceived ).arg( bytesTotal < 0 ? QStringLiteral( "unknown number of" ) : QString::number( bytesTotal ) ), 4 );

  if ( !mIsAborted && mReply )
  {
    if ( mReply->error() == QNetworkReply::NoError )
    {
      QVariant redirect = mReply->attribute( QNetworkRequest::RedirectionTargetAttribute );
      if ( !redirect.isNull() )
      {
        // We don't want to emit downloadProgress() for a redirect
        return;
      }
    }
  }

  emit downloadProgress( bytesReceived, bytesTotal );
}

void QgsBaseNetworkRequest::replyFinished()
{
  if ( !mIsAborted && mReply )
  {
    if ( mReply->error() == QNetworkReply::NoError )
    {
      QgsDebugMsgLevel( QStringLiteral( "reply OK" ), 4 );
      QVariant redirect = mReply->attribute( QNetworkRequest::RedirectionTargetAttribute );
      if ( !redirect.isNull() )
      {
        QgsDebugMsgLevel( QStringLiteral( "Request redirected." ), 4 );

        const QUrl &toUrl = redirect.toUrl();
        mReply->request();
        if ( toUrl == mReply->url() )
        {
          mErrorMessage = tr( "Redirect loop detected: %1" ).arg( toUrl.toString() );
          logMessageIfEnabled();
          mResponse.clear();
        }
        else
        {
          QNetworkRequest request( toUrl );
          QgsSetRequestInitiatorClass( request, QStringLiteral( "QgsBaseNetworkRequest" ) );
          if ( !mAuth.setAuthorization( request ) )
          {
            mResponse.clear();
            mErrorMessage = errorMessageFailedAuth();
            mErrorCode = QgsBaseNetworkRequest::NetworkError;
            logMessageIfEnabled();
            emit downloadFinished();
            return;
          }
          request.setAttribute( QNetworkRequest::CacheLoadControlAttribute, mForceRefresh ? QNetworkRequest::AlwaysNetwork : QNetworkRequest::PreferCache );
          request.setAttribute( QNetworkRequest::CacheSaveControlAttribute, true );

          mReply->deleteLater();
          mReply = nullptr;

          QgsDebugMsgLevel( QStringLiteral( "redirected: %1 forceRefresh=%2" ).arg( redirect.toString() ).arg( mForceRefresh ), 4 );
          mReply = QgsNetworkAccessManager::instance()->get( request );
          if ( !mAuth.setAuthorizationReply( mReply ) )
          {
            mResponse.clear();
            mErrorMessage = errorMessageFailedAuth();
            mErrorCode = QgsBaseNetworkRequest::NetworkError;
            logMessageIfEnabled();
            emit downloadFinished();
            return;
          }
          connect( mReply, &QNetworkReply::finished, this, &QgsBaseNetworkRequest::replyFinished, Qt::DirectConnection );
          connect( mReply, &QNetworkReply::downloadProgress, this, &QgsBaseNetworkRequest::replyProgress, Qt::DirectConnection );
          connect( mReply, &QNetworkReply::readyRead, this, &QgsBaseNetworkRequest::replyReadyRead, Qt::DirectConnection );
          return;
        }
      }
      else
      {
        const QgsNetworkAccessManager *nam = QgsNetworkAccessManager::instance();

        if ( nam->cache() )
        {
          QgsDebugMsgLevel( QStringLiteral( "request url:%1" ).arg( mReply->request().url().toString() ), 4 );
          QNetworkCacheMetaData cmd = nam->cache()->metaData( mReply->request().url() );

          QNetworkCacheMetaData::RawHeaderList hl;
          const auto constRawHeaders = cmd.rawHeaders();
          for ( const QNetworkCacheMetaData::RawHeader &h : constRawHeaders )
          {
            if ( h.first != "Cache-Control" )
              hl.append( h );
          }
          cmd.setRawHeaders( hl );

          QgsDebugMsgLevel( QStringLiteral( "expirationDate:%1" ).arg( cmd.expirationDate().toString() ), 4 );
          if ( cmd.expirationDate().isNull() )
          {
            cmd.setExpirationDate( QDateTime::currentDateTime().addSecs( defaultExpirationInSec() ) );
          }

          nam->cache()->updateMetaData( cmd );
        }
        else
        {
          QgsDebugMsgLevel( QStringLiteral( "No cache!" ), 4 );
        }

#ifdef QGISDEBUG
        bool fromCache = mReply->attribute( QNetworkRequest::SourceIsFromCacheAttribute ).toBool();
        QgsDebugMsgLevel( QStringLiteral( "Reply was cached: %1" ).arg( fromCache ), 4 );
#endif

        mResponse = mReply->readAll();

        if ( mResponse.isEmpty() && !mGotNonEmptyResponse )
        {
          mErrorMessage = tr( "empty response: %1" ).arg( mReply->errorString() );
          mErrorCode = QgsBaseNetworkRequest::ServerExceptionError;
          logMessageIfEnabled();
        }
      }
    }
    else
    {
      mErrorMessage = errorMessageWithReason( mReply->errorString() );
      mErrorCode = QgsBaseNetworkRequest::ServerExceptionError;
      logMessageIfEnabled();
      mResponse.clear();
    }
  }
  if ( mTimedout )
    mErrorCode = QgsBaseNetworkRequest::TimeoutError;

  if ( mReply )
  {
    mReply->deleteLater();
    mReply = nullptr;
  }

  emit downloadFinished();
}

QString QgsBaseNetworkRequest::errorMessageFailedAuth()
{
  return errorMessageWithReason( tr( "network request update failed for authentication config" ) );
}

void QgsBaseNetworkRequest::logMessageIfEnabled()
{
  if ( mLogErrors )
    QgsMessageLog::logMessage( mErrorMessage, mTranslatedComponent );
}
