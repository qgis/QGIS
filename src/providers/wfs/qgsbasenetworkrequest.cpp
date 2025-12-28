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
#include "qgssetrequestinitiator_p.h"
#include "qgssettings.h"
#include "qgstestutils.h"
#include "qgsvariantutils.h"

#include <QCache>
#include <QEventLoop>
#include <QFuture>
#include <QNetworkCacheMetaData>
#include <QtConcurrent>

#include "moc_qgsbasenetworkrequest.cpp"

static QMutex gMemoryCacheMmutex;
static QCache<QUrl, std::pair<QDateTime, QByteArray>> gCache( 10 * 1024 * 1024 );

static QByteArray getFromMemoryCache( const QUrl &url )
{
  QMutexLocker lock( &gMemoryCacheMmutex );
  const std::pair<QDateTime, QByteArray> *entry = gCache.object( url );
  if ( entry )
  {
    QgsSettings s;
    const int delayOfCachingInSecs = s.value( u"qgis/wfsMemoryCacheDelay"_s, 60 ).toInt();
    if ( entry->first.secsTo( QDateTime::currentDateTime() ) < delayOfCachingInSecs )
    {
      QgsDebugMsgLevel( u"Reusing cached response from memory cache for %1"_s.arg( url.toString() ), 4 );
      return entry->second;
    }
  }
  return QByteArray();
}

static void insertIntoMemoryCache( const QUrl &url, const QByteArray &response )
{
  QMutexLocker lock( &gMemoryCacheMmutex );
  if ( response.size() <= gCache.maxCost() )
  {
    std::pair<QDateTime, QByteArray> *entry = new std::pair<QDateTime, QByteArray>();
    entry->first = QDateTime::currentDateTime();
    entry->second = response;
    gCache.insert( url, entry, response.size() );
  }
}

QgsBaseNetworkRequest::QgsBaseNetworkRequest( const QgsAuthorizationSettings &auth, const QString &translatedComponent )
  : mAuth( auth )
  , mTranslatedComponent( translatedComponent )
{
  connect( QgsNetworkAccessManager::instance(), qOverload<QNetworkReply *>( &QgsNetworkAccessManager::requestTimedOut ), this, &QgsBaseNetworkRequest::requestTimedOut );
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

bool QgsBaseNetworkRequest::sendGET( const QUrl &url, const QString &acceptHeader, bool synchronous, bool forceRefresh, bool cache, const QList<QNetworkReply::RawHeaderPair> &extraHeaders )
{
  abort(); // cancel previous
  mIsAborted = false;
  mTimedout = false;
  mGotNonEmptyResponse = false;

  mErrorMessage.clear();
  mErrorCode = QgsBaseNetworkRequest::NoError;
  mForceRefresh = forceRefresh;
  mResponse.clear();
  mResponseHeaders.clear();

  if ( synchronous )
  {
    mResponse = getFromMemoryCache( url );
    if ( !mResponse.isEmpty() )
    {
      emit downloadProgress( mResponse.size(), mResponse.size() );
      emit downloadFinished();
      return true;
    }
  }

  QUrl modifiedUrl( url );

  // Specific code for testing
  if ( modifiedUrl.toString().contains( "fake_qgis_http_endpoint"_L1 ) )
  {
    mIsSimulatedMode = true;

    // Just for testing with local files instead of http:// resources
    QString modifiedUrlString;

    if ( modifiedUrl.toString().contains( "fake_qgis_http_endpoint_encoded_query"_L1 ) )
    {
      // Get encoded representation (used by test_provider_oapif.py testSimpleQueryableFiltering())
      modifiedUrlString = modifiedUrl.toEncoded();
    }
    else
    {
      // Get representation with percent decoding (easier for WFS filtering)
      modifiedUrlString = QUrl::fromPercentEncoding( modifiedUrl.toString().toUtf8() );
    }

    if ( !acceptHeader.isEmpty() )
    {
      if ( modifiedUrlString.indexOf( '?' ) > 0 )
      {
        modifiedUrlString += u"&Accept="_s + acceptHeader;
      }
      else
      {
        modifiedUrlString += u"?Accept="_s + acceptHeader;
      }
    }
    for ( const QNetworkReply::RawHeaderPair &headerPair : extraHeaders )
    {
      if ( modifiedUrlString.indexOf( '?' ) > 0 )
      {
        modifiedUrlString += QLatin1Char( '&' );
      }
      else
      {
        modifiedUrlString += QLatin1Char( '?' );
      }
      modifiedUrlString += QString::fromUtf8( headerPair.first ) + u"="_s + QString::fromUtf8( headerPair.second );
    }

    QgsDebugMsgLevel( u"Get %1"_s.arg( modifiedUrlString ), 4 );
    modifiedUrlString = modifiedUrlString.mid( u"http://"_s.size() );
#ifdef Q_OS_WIN
    // Passing "urls" like "http://c:/path" to QUrl 'eats' the : after c,
    // so we must restore it
    if ( modifiedUrlString[1] == '/' )
    {
      modifiedUrlString = modifiedUrlString[0] + ":/" + modifiedUrlString.mid( 2 );
    }
#endif

    const auto posQuotationMark = modifiedUrlString.indexOf( '?' );
    if ( posQuotationMark > 0 )
    {
      modifiedUrlString = QgsTestUtils::sanitizeFakeHttpEndpoint( modifiedUrlString );
    }
    QgsDebugMsgLevel( u"Get %1 (after laundering)"_s.arg( modifiedUrlString ), 4 );
    modifiedUrl = QUrl::fromLocalFile( modifiedUrlString );
  }
  else
  {
    // Some servers don't like spaces not encoded
    // e.g the following fails because of the space after fes%3AFilter and before xmlns%31:fes
    // but works if replacing it with %20
    // http://geocloud.vd.dk/NR/wfs?SERVICE=WFS&REQUEST=GetFeature&VERSION=2.0.0&TYPENAMES=NR:nr_byggelinjer&STARTINDEX=0&COUNT=1&SRSNAME=urn:ogc:def:crs:EPSG::25832&FILTER=%3Cfes%3AFilter xmlns%3Afes%3D%22http%3A%2F%2Fwww.opengis.net%2Ffes%2F2.0%22%20xmlns%3Agml%3D%22http%3A%2F%2Fwww.opengis.net%2Fgml%2F3.2%22%3E%0A%3Cfes%3ADisjoint%3E%0A%3Cfes%3AValueReference%3EGEOMETRY%3C%2Ffes%3AValueReference%3E%0A%3Cgml%3APoint%20srsName%3D%22urn%3Aogc%3Adef%3Acrs%3AEPSG%3A%3A25832%22%20gml%3Aid%3D%22qgis_id_geom_1%22%3E%0A%3Cgml%3Apos%20srsDimension%3D%222%22%3E0%200%3C%2Fgml%3Apos%3E%0A%3C%2Fgml%3APoint%3E%0A%3C%2Ffes%3ADisjoint%3E%0A%3C%2Ffes%3AFilter%3E%0A&SORTBY=BESTEMMELSEN
    modifiedUrl = modifiedUrl.adjusted( QUrl::EncodeSpaces );
  }

  QgsDebugMsgLevel( u"Calling: %1"_s.arg( modifiedUrl.toDisplayString( QUrl::EncodeSpaces ) ), 4 );

  QNetworkRequest request( modifiedUrl );

  mRequestHeaders = extraHeaders;
  if ( !acceptHeader.isEmpty() )
  {
    mRequestHeaders << QNetworkReply::RawHeaderPair( "Accept", acceptHeader.toUtf8() );
  }

  for ( const QNetworkReply::RawHeaderPair &headerPair : std::as_const( mRequestHeaders ) )
    request.setRawHeader( headerPair.first, headerPair.second );

  QgsSetRequestInitiatorClass( request, u"QgsBaseNetworkRequest"_s );
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

  const bool success = issueRequest( request, QByteArray( "GET" ), nullptr, synchronous );
  if ( !success || !mErrorMessage.isEmpty() )
  {
    return false;
  }

  if ( synchronous )
  {
    // Insert response of requests GetCapabilities, DescribeFeatureType or GetFeature
    // with a COUNT=1 into a short-lived memory cache, as they are emitted
    // repeatedly in interactive scenarios when adding a WFS layer.
    QString urlString = url.toString();
    if ( urlString.contains( u"REQUEST=GetCapabilities"_s ) || urlString.contains( u"REQUEST=DescribeFeatureType"_s ) || ( urlString.contains( u"REQUEST=GetFeature"_s ) && urlString.contains( u"COUNT=1"_s ) ) )
    {
      QgsSettings s;
      if ( s.value( u"qgis/wfsMemoryCacheAllowed"_s, true ).toBool() )
      {
        insertIntoMemoryCache( url, mResponse );
      }
    }
  }

  return true;
}

bool QgsBaseNetworkRequest::issueRequest( QNetworkRequest &request, const QByteArray &verb, const QByteArray *data, bool synchronous )
{
  QWaitCondition waitCondition;
  QMutex waitConditionMutex;

  bool threadFinished = false;
  bool success = false;

  request.setAttribute( QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::ManualRedirectPolicy );

  const std::function<void()> downloaderFunction = [this, request, synchronous, data, &verb, &waitConditionMutex, &waitCondition, &threadFinished, &success]() {
    if ( QThread::currentThread() != QApplication::instance()->thread() )
      QgsNetworkAccessManager::instance( Qt::DirectConnection );

    success = true;
    if ( verb == QByteArray( "GET" ) )
      mReply = QgsNetworkAccessManager::instance()->get( request );
    else if ( verb == QByteArray( "POST" ) )
      mReply = QgsNetworkAccessManager::instance()->post( request, *data );
    else if ( verb == QByteArray( "PUT" ) )
      mReply = QgsNetworkAccessManager::instance()->put( request, *data );
    else if ( verb == QByteArray( "PATCH" ) )
      mReply = QgsNetworkAccessManager::instance()->sendCustomRequest( request, verb, *data );
    else
      mReply = QgsNetworkAccessManager::instance()->sendCustomRequest( request, verb );

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
        auto resumeMainThread = [&waitConditionMutex, &waitCondition]() {
          // when this method is called we have "produced" a single authentication request -- so the buffer is now full
          // and it's time for the "consumer" (main thread) to do its part
          waitConditionMutex.lock();
          waitCondition.wakeAll();
          waitConditionMutex.unlock();

          // note that we don't need to handle waking this thread back up - that's done automatically by QgsNetworkAccessManager
        };

        QMetaObject::Connection authRequestConnection = connect( QgsNetworkAccessManager::instance(), &QgsNetworkAccessManager::authRequestOccurred, this, resumeMainThread, Qt::DirectConnection );
        QMetaObject::Connection proxyAuthenticationConnection = connect( QgsNetworkAccessManager::instance(), &QgsNetworkAccessManager::proxyAuthenticationRequired, this, resumeMainThread, Qt::DirectConnection );
#ifndef QT_NO_SSL
        QMetaObject::Connection sslErrorConnection = connect( QgsNetworkAccessManager::instance(), &QgsNetworkAccessManager::sslErrorsOccurred, this, resumeMainThread, Qt::DirectConnection );
#endif

        QEventLoop loop;
        connect( this, &QgsBaseNetworkRequest::downloadFinished, &loop, &QEventLoop::quit, Qt::DirectConnection );
        loop.exec();

        // event loop exited - need to disconnect as to not leave functor hanging to receive signals in future
        disconnect( authRequestConnection );
        disconnect( proxyAuthenticationConnection );
#ifndef QT_NO_SSL
        disconnect( sslErrorConnection );
#endif
      }
    }
    waitConditionMutex.lock();
    threadFinished = true;
    waitCondition.wakeAll();
    waitConditionMutex.unlock();
  };

  if ( synchronous && QThread::currentThread() == QApplication::instance()->thread() )
  {
    auto downloaderThread = std::make_unique<_DownloaderThread>( downloaderFunction );
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

        QApplication::processEvents();
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

  return success;
}

void QgsBaseNetworkRequest::extractResponseHeadersForUnitTests()
{
  if ( mFakeResponseHasHeaders )
  {
    // Expect the file content to be formatted like:
    // header1: value1\r\n
    // headerN: valueN\r\n
    // \r\n
    // content
    int from = 0;
    while ( true )
    {
      const int pos = static_cast<int>( mResponse.indexOf( QByteArray( "\r\n" ), from ) );
      if ( pos < 0 )
      {
        break;
      }
      QByteArray line = mResponse.mid( from, pos - from );
      const int posColon = static_cast<int>( line.indexOf( QByteArray( ":" ) ) );
      if ( posColon > 0 )
      {
        mResponseHeaders.append( QNetworkReply::RawHeaderPair( line.mid( 0, posColon ), line.mid( posColon + 1 ).trimmed() ) );
      }
      from = pos + 2;
      if ( from + 2 < mResponse.size() && mResponse[from] == '\r' && mResponse[from] == '\n' )
      {
        from += 2;
        break;
      }
    }
    mResponse = mResponse.mid( from );
  }
}

bool QgsBaseNetworkRequest::sendPOSTOrPUTOrPATCH( const QUrl &url, const QByteArray &verb, const QString &contentTypeHeader, const QByteArray &data, bool synchronous, const QList<QNetworkReply::RawHeaderPair> &extraHeaders )
{
  abort(); // cancel previous
  mIsAborted = false;
  mTimedout = false;
  mGotNonEmptyResponse = false;

  mErrorMessage.clear();
  mErrorCode = QgsBaseNetworkRequest::NoError;
  mForceRefresh = true;
  mResponse.clear();
  mResponseHeaders.clear();

  if ( url.toEncoded().contains( "fake_qgis_http_endpoint" ) )
  {
    mIsSimulatedMode = true;
    // Hack for testing purposes
    QUrl modifiedUrl( url );
    QUrlQuery query( modifiedUrl );
    query.addQueryItem( QString( QString::fromUtf8( verb ) + u"DATA"_s ), QString::fromUtf8( data ) );
    modifiedUrl.setQuery( query );
    QList<QNetworkReply::RawHeaderPair> extraHeadersModified( extraHeaders );
    if ( mFakeURLIncludesContentType && !contentTypeHeader.isEmpty() )
    {
      extraHeadersModified.append( QNetworkReply::RawHeaderPair( QByteArray( "Content-Type" ), contentTypeHeader.toUtf8() ) );
    }
    bool ret = sendGET( modifiedUrl, QString(), true, true, false, extraHeadersModified );

    extractResponseHeadersForUnitTests();
    return ret;
  }

  QNetworkRequest request( url );
  QgsSetRequestInitiatorClass( request, u"QgsBaseNetworkRequest"_s );
  if ( !mAuth.setAuthorization( request ) )
  {
    mErrorCode = QgsBaseNetworkRequest::NetworkError;
    mErrorMessage = errorMessageFailedAuth();
    logMessageIfEnabled();
    return false;
  }

  mRequestHeaders = extraHeaders;
  mRequestHeaders << QNetworkReply::RawHeaderPair( "Content-Type", contentTypeHeader.toUtf8() );

  for ( const QNetworkReply::RawHeaderPair &headerPair : std::as_const( mRequestHeaders ) )
    request.setRawHeader( headerPair.first, headerPair.second );

  if ( !issueRequest( request, verb, &data, synchronous ) )
  {
    return false;
  }

  return mErrorMessage.isEmpty();
}

bool QgsBaseNetworkRequest::sendPOST( const QUrl &url, const QString &contentTypeHeader, const QByteArray &data, bool synchronous, const QList<QNetworkReply::RawHeaderPair> &extraHeaders )
{
  return sendPOSTOrPUTOrPATCH( url, QByteArray( "POST" ), contentTypeHeader, data, synchronous, extraHeaders );
}

bool QgsBaseNetworkRequest::sendPUT( const QUrl &url, const QString &contentTypeHeader, const QByteArray &data, const QList<QNetworkReply::RawHeaderPair> &extraHeaders )
{
  return sendPOSTOrPUTOrPATCH( url, QByteArray( "PUT" ), contentTypeHeader, data, true /*synchronous*/, extraHeaders );
}

bool QgsBaseNetworkRequest::sendPATCH( const QUrl &url, const QString &contentTypeHeader, const QByteArray &data, const QList<QNetworkReply::RawHeaderPair> &extraHeaders )
{
  return sendPOSTOrPUTOrPATCH( url, QByteArray( "PATCH" ), contentTypeHeader, data, true /*synchronous*/, extraHeaders );
}

QStringList QgsBaseNetworkRequest::sendOPTIONS( const QUrl &url )
{
  abort(); // cancel previous
  mIsAborted = false;
  mTimedout = false;
  mGotNonEmptyResponse = false;
  mEmptyResponseIsValid = true;

  mErrorMessage.clear();
  mErrorCode = QgsBaseNetworkRequest::NoError;
  mForceRefresh = true;
  mResponse.clear();
  mResponseHeaders.clear();

  QByteArray allowValue;
  if ( url.toEncoded().contains( "fake_qgis_http_endpoint" ) )
  {
    mIsSimulatedMode = true;

    // Hack for testing purposes
    QUrl modifiedUrl( url );
    QUrlQuery query( modifiedUrl );
    query.addQueryItem( u"VERB"_s, u"OPTIONS"_s );
    modifiedUrl.setQuery( query );
    if ( !sendGET( modifiedUrl, QString(), true, true, false ) )
      return QStringList();
    allowValue = mResponse;
  }
  else
  {
    QNetworkRequest request( url );
    QgsSetRequestInitiatorClass( request, u"QgsBaseNetworkRequest"_s );
    if ( !mAuth.setAuthorization( request ) )
    {
      mErrorCode = QgsBaseNetworkRequest::NetworkError;
      mErrorMessage = errorMessageFailedAuth();
      logMessageIfEnabled();
      return QStringList();
    }

    if ( !issueRequest( request, QByteArray( "OPTIONS" ), /*data=*/nullptr, /*synchronous=*/true ) )
    {
      return QStringList();
    }

    for ( const auto &headerKeyValue : mResponseHeaders )
    {
      if ( headerKeyValue.first.compare( QByteArray( "Allow" ), Qt::CaseInsensitive ) == 0 )
      {
        allowValue = headerKeyValue.second;
        break;
      }
    }
  }

  QStringList res;
  QStringList l = QString::fromLatin1( allowValue ).split( QLatin1Char( ',' ) );
  for ( const QString &s : l )
  {
    res.append( s.trimmed() );
  }
  return res;
}

bool QgsBaseNetworkRequest::sendDELETE( const QUrl &url )
{
  abort(); // cancel previous
  mIsAborted = false;
  mTimedout = false;
  mGotNonEmptyResponse = false;
  mEmptyResponseIsValid = true;

  mErrorMessage.clear();
  mErrorCode = QgsBaseNetworkRequest::NoError;
  mForceRefresh = true;
  mResponse.clear();
  mResponseHeaders.clear();

  if ( url.toEncoded().contains( "fake_qgis_http_endpoint" ) )
  {
    mIsSimulatedMode = true;

    // Hack for testing purposes
    QUrl modifiedUrl( url );
    QUrlQuery query( modifiedUrl );
    query.addQueryItem( u"VERB"_s, QString::fromUtf8( "DELETE" ) );
    modifiedUrl.setQuery( query );
    return sendGET( modifiedUrl, QString(), true, true, false );
  }

  QNetworkRequest request( url );
  QgsSetRequestInitiatorClass( request, u"QgsBaseNetworkRequest"_s );
  if ( !mAuth.setAuthorization( request ) )
  {
    mErrorCode = QgsBaseNetworkRequest::NetworkError;
    mErrorMessage = errorMessageFailedAuth();
    logMessageIfEnabled();
    return false;
  }

  if ( !issueRequest( request, QByteArray( "DELETE" ), nullptr, /*synchronous=*/true ) )
  {
    return false;
  }

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

void QgsBaseNetworkRequest::replyReadyRead()
{
  mGotNonEmptyResponse = true;
}

void QgsBaseNetworkRequest::replyProgress( qint64 bytesReceived, qint64 bytesTotal )
{
  QgsDebugMsgLevel( u"%1 of %2 bytes downloaded."_s.arg( bytesReceived ).arg( bytesTotal < 0 ? u"unknown number of"_s : QString::number( bytesTotal ) ), 4 );

  if ( !mIsAborted && mReply )
  {
    if ( mReply->error() == QNetworkReply::NoError )
    {
      const QVariant redirect = mReply->attribute( QNetworkRequest::RedirectionTargetAttribute );
      if ( !QgsVariantUtils::isNull( redirect ) )
      {
        // We don't want to emit downloadProgress() for a redirect
        return;
      }
    }

    if ( !mIsSimulatedMode && mResponseHeaders.empty() )
      mResponseHeaders = mReply->rawHeaderPairs();
  }

  emit downloadProgress( bytesReceived, bytesTotal );
}

void QgsBaseNetworkRequest::replyFinished()
{
  if ( !mIsAborted && mReply )
  {
    if ( mReply->error() == QNetworkReply::NoError )
    {
      QgsDebugMsgLevel( u"reply OK"_s, 4 );
      const QVariant redirect = mReply->attribute( QNetworkRequest::RedirectionTargetAttribute );
      if ( !QgsVariantUtils::isNull( redirect ) )
      {
        QgsDebugMsgLevel( u"Request redirected."_s, 4 );

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
          QgsSetRequestInitiatorClass( request, u"QgsBaseNetworkRequest"_s );
          if ( !mAuth.setAuthorization( request ) )
          {
            mResponse.clear();
            mErrorMessage = errorMessageFailedAuth();
            mErrorCode = QgsBaseNetworkRequest::NetworkError;
            logMessageIfEnabled();
            emit downloadFinished();
            return;
          }
          request.setAttribute( QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::ManualRedirectPolicy );
          request.setAttribute( QNetworkRequest::CacheLoadControlAttribute, mForceRefresh ? QNetworkRequest::AlwaysNetwork : QNetworkRequest::PreferCache );
          request.setAttribute( QNetworkRequest::CacheSaveControlAttribute, true );

          for ( const QNetworkReply::RawHeaderPair &headerPair : std::as_const( mRequestHeaders ) )
            request.setRawHeader( headerPair.first, headerPair.second );

          mReply->deleteLater();
          mReply = nullptr;

          QgsDebugMsgLevel( u"redirected: %1 forceRefresh=%2"_s.arg( redirect.toString() ).arg( mForceRefresh ), 4 );
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
          QgsDebugMsgLevel( u"request url:%1"_s.arg( mReply->request().url().toString() ), 4 );
          QNetworkCacheMetaData cmd = nam->cache()->metaData( mReply->request().url() );

          QNetworkCacheMetaData::RawHeaderList hl;
          const auto constRawHeaders = cmd.rawHeaders();
          for ( const QNetworkCacheMetaData::RawHeader &h : constRawHeaders )
          {
            if ( h.first != "Cache-Control" )
              hl.append( h );
          }
          cmd.setRawHeaders( hl );

          QgsDebugMsgLevel( u"expirationDate:%1"_s.arg( cmd.expirationDate().toString() ), 4 );
          if ( cmd.expirationDate().isNull() )
          {
            cmd.setExpirationDate( QDateTime::currentDateTime().addSecs( defaultExpirationInSec() ) );
          }

          nam->cache()->updateMetaData( cmd );
        }
        else
        {
          QgsDebugMsgLevel( u"No cache!"_s, 4 );
        }

#ifdef QGISDEBUG
        const bool fromCache = mReply->attribute( QNetworkRequest::SourceIsFromCacheAttribute ).toBool();
        QgsDebugMsgLevel( u"Reply was cached: %1"_s.arg( fromCache ), 4 );
#endif

        mResponse = mReply->readAll();

        if ( mIsSimulatedMode )
          extractResponseHeadersForUnitTests();

        if ( mResponse.isEmpty() && !mGotNonEmptyResponse && !mEmptyResponseIsValid )
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
      const QString replyContent = mReply->readAll();
      QDomDocument exceptionDoc;
      QString errorMsg;
      if ( exceptionDoc.setContent( replyContent, true, &errorMsg ) )
      {
        const QDomElement exceptionElem = exceptionDoc.documentElement();
        if ( !exceptionElem.isNull() && exceptionElem.tagName() == "ExceptionReport"_L1 )
        {
          const QDomElement exception = exceptionElem.firstChildElement( u"Exception"_s );
          mErrorMessage = tr( "WFS exception report (code=%1 text=%2)" )
                            .arg( exception.attribute( u"exceptionCode"_s, tr( "missing" ) ), exception.firstChildElement( u"ExceptionText"_s ).text() );
        }
      }
      mErrorCode = QgsBaseNetworkRequest::ServerExceptionError;
      logMessageIfEnabled();
      mResponse.clear();
    }
  }
  if ( mTimedout )
    mErrorCode = QgsBaseNetworkRequest::TimeoutError;

  if ( mReply )
  {
    if ( !mIsSimulatedMode && mResponseHeaders.empty() )
      mResponseHeaders = mReply->rawHeaderPairs();

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
