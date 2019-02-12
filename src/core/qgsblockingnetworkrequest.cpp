/***************************************************************************
    qgsblockingnetworkrequest.cpp
    -----------------------------
    begin                : November 2018
    copyright            : (C) 2018 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsblockingnetworkrequest.h"
#include "qgslogger.h"
#include "qgsapplication.h"
#include "qgsnetworkaccessmanager.h"
#include "qgsauthmanager.h"
#include "qgsmessagelog.h"
#include "qgsfeedback.h"
#include <QUrl>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QMutex>
#include <QWaitCondition>
#include <QNetworkCacheMetaData>
#include <QAuthenticator>

QgsBlockingNetworkRequest::QgsBlockingNetworkRequest()
{
  connect( QgsNetworkAccessManager::instance(), qgis::overload< QNetworkReply * >::of( &QgsNetworkAccessManager::requestTimedOut ), this, &QgsBlockingNetworkRequest::requestTimedOut );
}

QgsBlockingNetworkRequest::~QgsBlockingNetworkRequest()
{
  abort();
}

void QgsBlockingNetworkRequest::requestTimedOut( QNetworkReply *reply )
{
  if ( reply == mReply )
    mTimedout = true;
}

QString QgsBlockingNetworkRequest::authCfg() const
{
  return mAuthCfg;
}

void QgsBlockingNetworkRequest::setAuthCfg( const QString &authCfg )
{
  mAuthCfg = authCfg;
}

QgsBlockingNetworkRequest::ErrorCode QgsBlockingNetworkRequest::get( QNetworkRequest &request, bool forceRefresh, QgsFeedback *feedback )
{
  return doRequest( Get, request, forceRefresh, feedback );
}

QgsBlockingNetworkRequest::ErrorCode QgsBlockingNetworkRequest::post( QNetworkRequest &request, const QByteArray &data, bool forceRefresh, QgsFeedback *feedback )
{
  mPostData = data;
  return doRequest( Post, request, forceRefresh, feedback );
}

QgsBlockingNetworkRequest::ErrorCode QgsBlockingNetworkRequest::doRequest( QgsBlockingNetworkRequest::Method method, QNetworkRequest &request, bool forceRefresh, QgsFeedback *feedback )
{
  mMethod = method;
  mFeedback = feedback;

  abort(); // cancel previous
  mIsAborted = false;
  mTimedout = false;
  mGotNonEmptyResponse = false;

  mErrorMessage.clear();
  mErrorCode = NoError;
  mForceRefresh = forceRefresh;
  mReplyContent.clear();

  if ( !mAuthCfg.isEmpty() &&  !QgsApplication::authManager()->updateNetworkRequest( request, mAuthCfg ) )
  {
    mErrorCode = NetworkError;
    mErrorMessage = errorMessageFailedAuth();
    QgsMessageLog::logMessage( mErrorMessage, tr( "Network" ) );
    return NetworkError;
  }

  QgsDebugMsgLevel( QStringLiteral( "Calling: %1" ).arg( request.url().toString() ), 2 );

  request.setAttribute( QNetworkRequest::CacheLoadControlAttribute, forceRefresh ? QNetworkRequest::AlwaysNetwork : QNetworkRequest::PreferCache );
  request.setAttribute( QNetworkRequest::CacheSaveControlAttribute, true );

  QWaitCondition authRequestBufferNotEmpty;
  QMutex waitConditionMutex;

  bool threadFinished = false;
  bool success = false;

  const bool requestMadeFromMainThread = QThread::currentThread() == QApplication::instance()->thread();

  if ( mFeedback )
    connect( mFeedback, &QgsFeedback::canceled, this, &QgsBlockingNetworkRequest::abort );

  std::function<void()> downloaderFunction = [ this, request, &waitConditionMutex, &authRequestBufferNotEmpty, &threadFinished, &success, requestMadeFromMainThread ]()
  {
    // this function will always be run in worker threads -- either the blocking call is being made in a worker thread,
    // or the blocking call has been made from the main thread and we've fired up a new thread for this function
    Q_ASSERT( QThread::currentThread() != QgsApplication::instance()->thread() );

    QgsNetworkAccessManager::instance( Qt::DirectConnection );

    success = true;

    switch ( mMethod )
    {
      case Get:
        mReply = QgsNetworkAccessManager::instance()->get( request );
        break;

      case Post:
        mReply = QgsNetworkAccessManager::instance()->post( request, mPostData );
        break;
    };

    if ( mFeedback )
      connect( mFeedback, &QgsFeedback::canceled, mReply, &QNetworkReply::abort );

    if ( !mAuthCfg.isEmpty() && !QgsApplication::authManager()->updateNetworkReply( mReply, mAuthCfg ) )
    {
      mErrorCode = NetworkError;
      mErrorMessage = errorMessageFailedAuth();
      QgsMessageLog::logMessage( mErrorMessage, tr( "Network" ) );
      if ( requestMadeFromMainThread )
        authRequestBufferNotEmpty.wakeAll();
      success = false;
    }
    else
    {
      // We are able to use direct connection here, because we
      // * either run on the thread mReply lives in, so DirectConnection is standard and safe anyway
      // * or the owner thread of mReply is currently not doing anything because it's blocked in future.waitForFinished() (if it is the main thread)
      connect( mReply, &QNetworkReply::finished, this, &QgsBlockingNetworkRequest::replyFinished, Qt::DirectConnection );
      connect( mReply, &QNetworkReply::downloadProgress, this, &QgsBlockingNetworkRequest::replyProgress, Qt::DirectConnection );

      auto resumeMainThread = [&waitConditionMutex, &authRequestBufferNotEmpty ]()
      {
        // when this method is called we have "produced" a single authentication request -- so the buffer is now full
        // and it's time for the "consumer" (main thread) to do its part
        waitConditionMutex.lock();
        authRequestBufferNotEmpty.wakeAll();
        waitConditionMutex.unlock();

        // note that we don't need to handle waking this thread back up - that's done automatically by QgsNetworkAccessManager
      };

      if ( requestMadeFromMainThread )
      {
        connect( QgsNetworkAccessManager::instance(), &QgsNetworkAccessManager::authRequestOccurred, this, resumeMainThread, Qt::DirectConnection );
        connect( QgsNetworkAccessManager::instance(), &QgsNetworkAccessManager::proxyAuthenticationRequired, this, resumeMainThread, Qt::DirectConnection );

#ifndef QT_NO_SSL
        connect( QgsNetworkAccessManager::instance(), &QgsNetworkAccessManager::sslErrorsOccurred, this, resumeMainThread, Qt::DirectConnection );
#endif
      }
      QEventLoop loop;
      connect( this, &QgsBlockingNetworkRequest::downloadFinished, &loop, &QEventLoop::quit, Qt::DirectConnection );
      loop.exec();
    }

    if ( requestMadeFromMainThread )
    {
      waitConditionMutex.lock();
      threadFinished = true;
      authRequestBufferNotEmpty.wakeAll();
      waitConditionMutex.unlock();
    }
  };

  if ( requestMadeFromMainThread )
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
      authRequestBufferNotEmpty.wait( &waitConditionMutex );

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
  return mErrorCode;
}

void QgsBlockingNetworkRequest::abort()
{
  mIsAborted = true;
  if ( mReply )
  {
    mReply->deleteLater();
    mReply = nullptr;
  }
}

void QgsBlockingNetworkRequest::replyProgress( qint64 bytesReceived, qint64 bytesTotal )
{
  QgsDebugMsgLevel( QStringLiteral( "%1 of %2 bytes downloaded." ).arg( bytesReceived ).arg( bytesTotal < 0 ? QStringLiteral( "unknown number of" ) : QString::number( bytesTotal ) ), 2 );

  if ( bytesReceived != 0 )
    mGotNonEmptyResponse = true;

  if ( !mIsAborted && mReply && ( !mFeedback || !mFeedback->isCanceled() ) )
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

void QgsBlockingNetworkRequest::replyFinished()
{
  if ( !mIsAborted && mReply )
  {
    if ( mReply->error() == QNetworkReply::NoError && ( !mFeedback || !mFeedback->isCanceled() ) )
    {
      QgsDebugMsgLevel( QStringLiteral( "reply OK" ), 2 );
      QVariant redirect = mReply->attribute( QNetworkRequest::RedirectionTargetAttribute );
      if ( !redirect.isNull() )
      {
        QgsDebugMsgLevel( QStringLiteral( "Request redirected." ), 2 );

        const QUrl &toUrl = redirect.toUrl();
        mReply->request();
        if ( toUrl == mReply->url() )
        {
          mErrorMessage = tr( "Redirect loop detected: %1" ).arg( toUrl.toString() );
          QgsMessageLog::logMessage( mErrorMessage, tr( "Network" ) );
          mReplyContent.clear();
        }
        else
        {
          QNetworkRequest request( toUrl );

          if ( !mAuthCfg.isEmpty() && !QgsApplication::authManager()->updateNetworkRequest( request, mAuthCfg ) )
          {
            mReplyContent.clear();
            mErrorMessage = errorMessageFailedAuth();
            mErrorCode = NetworkError;
            QgsMessageLog::logMessage( mErrorMessage, tr( "Network" ) );
            emit downloadFinished();
            return;
          }

          request.setAttribute( QNetworkRequest::CacheLoadControlAttribute, mForceRefresh ? QNetworkRequest::AlwaysNetwork : QNetworkRequest::PreferCache );
          request.setAttribute( QNetworkRequest::CacheSaveControlAttribute, true );

          mReply->deleteLater();
          mReply = nullptr;

          QgsDebugMsgLevel( QStringLiteral( "redirected: %1 forceRefresh=%2" ).arg( redirect.toString() ).arg( mForceRefresh ), 2 );
          switch ( mMethod )
          {
            case Get:
              mReply = QgsNetworkAccessManager::instance()->get( request );
              break;

            case Post:
              mReply = QgsNetworkAccessManager::instance()->post( request, mPostData );
              break;
          };

          if ( mFeedback )
            connect( mFeedback, &QgsFeedback::canceled, mReply, &QNetworkReply::abort );

          if ( !mAuthCfg.isEmpty() && !QgsApplication::authManager()->updateNetworkReply( mReply, mAuthCfg ) )
          {
            mReplyContent.clear();
            mErrorMessage = errorMessageFailedAuth();
            mErrorCode = NetworkError;
            QgsMessageLog::logMessage( mErrorMessage, tr( "Network" ) );
            emit downloadFinished();
            return;
          }

          connect( mReply, &QNetworkReply::finished, this, &QgsBlockingNetworkRequest::replyFinished, Qt::DirectConnection );
          connect( mReply, &QNetworkReply::downloadProgress, this, &QgsBlockingNetworkRequest::replyProgress, Qt::DirectConnection );
          return;
        }
      }
      else
      {
        const QgsNetworkAccessManager *nam = QgsNetworkAccessManager::instance();

        if ( nam->cache() )
        {
          QNetworkCacheMetaData cmd = nam->cache()->metaData( mReply->request().url() );

          QNetworkCacheMetaData::RawHeaderList hl;
          Q_FOREACH ( const QNetworkCacheMetaData::RawHeader &h, cmd.rawHeaders() )
          {
            if ( h.first != "Cache-Control" )
              hl.append( h );
          }
          cmd.setRawHeaders( hl );

          QgsDebugMsgLevel( QStringLiteral( "expirationDate:%1" ).arg( cmd.expirationDate().toString() ), 2 );
          if ( cmd.expirationDate().isNull() )
          {
            cmd.setExpirationDate( QDateTime::currentDateTime().addSecs( mExpirationSec ) );
          }

          nam->cache()->updateMetaData( cmd );
        }
        else
        {
          QgsDebugMsgLevel( QStringLiteral( "No cache!" ), 2 );
        }

#ifdef QGISDEBUG
        bool fromCache = mReply->attribute( QNetworkRequest::SourceIsFromCacheAttribute ).toBool();
        QgsDebugMsgLevel( QStringLiteral( "Reply was cached: %1" ).arg( fromCache ), 2 );
#endif

        mReplyContent = QgsNetworkReplyContent( mReply );
        const QByteArray content = mReply->readAll();
        if ( content.isEmpty() && !mGotNonEmptyResponse )
        {
          mErrorMessage = tr( "empty response: %1" ).arg( mReply->errorString() );
          mErrorCode = ServerExceptionError;
          QgsMessageLog::logMessage( mErrorMessage, tr( "Network" ) );
        }
        mReplyContent.setContent( content );
      }
    }
    else
    {
      mErrorMessage = mReply->errorString();
      mErrorCode = ServerExceptionError;
      QgsMessageLog::logMessage( mErrorMessage, tr( "Network" ) );
      mReplyContent = QgsNetworkReplyContent( mReply );
    }
  }
  if ( mTimedout )
    mErrorCode = TimeoutError;

  if ( mReply )
  {
    mReply->deleteLater();
    mReply = nullptr;
  }

  emit downloadFinished();
}

QString QgsBlockingNetworkRequest::errorMessageFailedAuth()
{
  return tr( "network request update failed for authentication config" );
}
