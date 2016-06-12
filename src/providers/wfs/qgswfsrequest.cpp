/***************************************************************************
    qgswfsrequest.cpp
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

#include "qgswfsrequest.h"
#include "qgslogger.h"
#include "qgsmessagelog.h"
#include "qgsnetworkaccessmanager.h"

#include <QEventLoop>
#include <QNetworkCacheMetaData>
#include <QCryptographicHash> // just for testin file:// fake_qgis_http_endpoint hack

QgsWFSRequest::QgsWFSRequest( const QString& theUri )
    : mUri( theUri )
    , mReply( nullptr )
    , mErrorCode( QgsWFSRequest::NoError )
    , mIsAborted( false )
    , mForceRefresh( false )
    , mTimedout( false )
    , mGotNonEmptyResponse( false )
{
  QgsDebugMsg( "theUri = " + theUri );
  connect( QgsNetworkAccessManager::instance(), SIGNAL( requestTimedOut( QNetworkReply* ) ), this, SLOT( requestTimedOut( QNetworkReply* ) ) );
}

QgsWFSRequest::~QgsWFSRequest()
{
  abort();
}

void QgsWFSRequest::requestTimedOut( QNetworkReply* reply )
{
  if ( reply == mReply )
    mTimedout = true;
}

bool QgsWFSRequest::sendGET( const QUrl& url, bool synchronous, bool forceRefresh, bool cache )
{
  abort(); // cancel previous
  mIsAborted = false;
  mTimedout = false;
  mGotNonEmptyResponse = false;

  mErrorMessage.clear();
  mErrorCode = QgsWFSRequest::NoError;
  mForceRefresh = forceRefresh;
  mResponse.clear();

  QUrl modifiedUrl( url );
  if ( modifiedUrl.toString().contains( "fake_qgis_http_endpoint" ) )
  {
    // Just for testing with local files instead of http:// ressources
    QString modifiedUrlString = modifiedUrl.toString();
    // Qt5 does URL encoding from some reason (of the FILTER parameter for example)
    modifiedUrlString = QUrl::fromPercentEncoding( modifiedUrlString.toUtf8() );
    QgsDebugMsg( QString( "Get %1" ).arg( modifiedUrlString ) );
    modifiedUrlString = modifiedUrlString.mid( QString( "http://" ).size() );
    QString args = modifiedUrlString.mid( modifiedUrlString.indexOf( '?' ) );
    if ( modifiedUrlString.size() > 256 )
    {
      args = QCryptographicHash::hash( args.toUtf8(), QCryptographicHash::Md5 ).toHex();
    }
    else
    {
      args.replace( "?", "_" );
      args.replace( "&", "_" );
      args.replace( "<", "_" );
      args.replace( ">", "_" );
      args.replace( "'", "_" );
      args.replace( "\"", "_" );
      args.replace( " ", "_" );
      args.replace( ":", "_" );
      args.replace( "/", "_" );
      args.replace( "\n", "_" );
    }
#ifdef Q_OS_WIN
    // Passing "urls" like "http://c:/path" to QUrl 'eats' the : after c,
    // so we must restore it
    if ( modifiedUrlString[1] == '/' )
    {
      modifiedUrlString = modifiedUrlString[0] + ":/" + modifiedUrlString.mid( 2 );
    }
#endif
    modifiedUrlString = modifiedUrlString.mid( 0, modifiedUrlString.indexOf( '?' ) ) + args;
    QgsDebugMsg( QString( "Get %1 (after laundering)" ).arg( modifiedUrlString ) );
    modifiedUrl = QUrl::fromLocalFile( modifiedUrlString );
  }

  QNetworkRequest request( modifiedUrl );
  if ( !mUri.auth().setAuthorization( request ) )
  {
    mErrorCode = QgsWFSRequest::NetworkError;
    mErrorMessage = errorMessageFailedAuth();
    QgsMessageLog::logMessage( mErrorMessage, tr( "WFS" ) );
    return false;
  }

  if ( cache )
  {
    request.setAttribute( QNetworkRequest::CacheLoadControlAttribute, forceRefresh ? QNetworkRequest::AlwaysNetwork : QNetworkRequest::PreferCache );
    request.setAttribute( QNetworkRequest::CacheSaveControlAttribute, true );
  }

  mReply = QgsNetworkAccessManager::instance()->get( request );
  connect( mReply, SIGNAL( finished() ), this, SLOT( replyFinished() ) );
  connect( mReply, SIGNAL( downloadProgress( qint64, qint64 ) ), this, SLOT( replyProgress( qint64, qint64 ) ) );

  if ( !synchronous )
    return true;

  QEventLoop loop;
  connect( this, SIGNAL( downloadFinished() ), &loop, SLOT( quit() ) );
  loop.exec( QEventLoop::ExcludeUserInputEvents );

  return mErrorMessage.isEmpty();
}

bool QgsWFSRequest::sendPOST( const QUrl& url, const QString& contentTypeHeader, const QByteArray& data )
{
  abort(); // cancel previous
  mIsAborted = false;
  mTimedout = false;
  mGotNonEmptyResponse = false;

  mErrorMessage.clear();
  mErrorCode = QgsWFSRequest::NoError;
  mForceRefresh = true;
  mResponse.clear();

  if ( url.toEncoded().contains( "fake_qgis_http_endpoint" ) )
  {
    // Hack for testing purposes
    QUrl modifiedUrl( url );
    modifiedUrl.addQueryItem( "POSTDATA", QString::fromUtf8( data ) );
    return sendGET( modifiedUrl, true, true, false );
  }

  QNetworkRequest request( url );
  if ( !mUri.auth().setAuthorization( request ) )
  {
    mErrorCode = QgsWFSRequest::NetworkError;
    mErrorMessage = errorMessageFailedAuth();
    QgsMessageLog::logMessage( mErrorMessage, tr( "WFS" ) );
    return false;
  }
  request.setHeader( QNetworkRequest::ContentTypeHeader, contentTypeHeader );

  mReply = QgsNetworkAccessManager::instance()->post( request, data );
  connect( mReply, SIGNAL( finished() ), this, SLOT( replyFinished() ) );
  connect( mReply, SIGNAL( downloadProgress( qint64, qint64 ) ), this, SLOT( replyProgress( qint64, qint64 ) ) );

  QEventLoop loop;
  connect( this, SIGNAL( downloadFinished() ), &loop, SLOT( quit() ) );
  loop.exec( QEventLoop::ExcludeUserInputEvents );

  return mErrorMessage.isEmpty();
}

void QgsWFSRequest::abort()
{
  mIsAborted = true;
  if ( mReply )
  {
    mReply->deleteLater();
    mReply = nullptr;
  }
}

void QgsWFSRequest::replyProgress( qint64 bytesReceived, qint64 bytesTotal )
{
  QgsDebugMsg( tr( "%1 of %2 bytes downloaded." ).arg( bytesReceived ).arg( bytesTotal < 0 ? QString( "unknown number of" ) : QString::number( bytesTotal ) ) );

  if ( bytesReceived != 0 )
    mGotNonEmptyResponse = true;

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

void QgsWFSRequest::replyFinished()
{
  if ( !mIsAborted && mReply )
  {
    if ( mReply->error() == QNetworkReply::NoError )
    {
      QgsDebugMsg( "reply ok" );
      QVariant redirect = mReply->attribute( QNetworkRequest::RedirectionTargetAttribute );
      if ( !redirect.isNull() )
      {
        QgsDebugMsg( "Request redirected." );

        const QUrl& toUrl = redirect.toUrl();
        mReply->request();
        if ( toUrl == mReply->url() )
        {
          mErrorMessage = tr( "Redirect loop detected: %1" ).arg( toUrl.toString() );
          QgsMessageLog::logMessage( mErrorMessage, tr( "WFS" ) );
          mResponse.clear();
        }
        else
        {
          QNetworkRequest request( toUrl );
          if ( !mUri.auth().setAuthorization( request ) )
          {
            mResponse.clear();
            mErrorMessage = errorMessageFailedAuth();
            mErrorCode = QgsWFSRequest::NetworkError;
            QgsMessageLog::logMessage( mErrorMessage, tr( "WFS" ) );
            emit downloadFinished();
            return;
          }
          request.setAttribute( QNetworkRequest::CacheLoadControlAttribute, mForceRefresh ? QNetworkRequest::AlwaysNetwork : QNetworkRequest::PreferCache );
          request.setAttribute( QNetworkRequest::CacheSaveControlAttribute, true );

          mReply->deleteLater();
          mReply = nullptr;

          QgsDebugMsg( QString( "redirected: %1 forceRefresh=%2" ).arg( redirect.toString() ).arg( mForceRefresh ) );
          mReply = QgsNetworkAccessManager::instance()->get( request );
          connect( mReply, SIGNAL( finished() ), this, SLOT( replyFinished() ) );
          connect( mReply, SIGNAL( downloadProgress( qint64, qint64 ) ), this, SLOT( replyProgress( qint64, qint64 ) ) );
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

          QgsDebugMsg( QString( "expirationDate:%1" ).arg( cmd.expirationDate().toString() ) );
          if ( cmd.expirationDate().isNull() )
          {
            cmd.setExpirationDate( QDateTime::currentDateTime().addSecs( defaultExpirationInSec() ) );
          }

          nam->cache()->updateMetaData( cmd );
        }
        else
        {
          QgsDebugMsg( "No cache!" );
        }

#ifdef QGISDEBUG
        bool fromCache = mReply->attribute( QNetworkRequest::SourceIsFromCacheAttribute ).toBool();
        QgsDebugMsg( QString( "Reply was cached: %1" ).arg( fromCache ) );
#endif

        mResponse = mReply->readAll();

        if ( mResponse.isEmpty() && !mGotNonEmptyResponse )
        {
          mErrorMessage = tr( "empty response: %1" ).arg( mReply->errorString() );
          mErrorCode = QgsWFSRequest::ServerExceptionError;
          QgsMessageLog::logMessage( mErrorMessage, tr( "WFS" ) );
        }
      }
    }
    else
    {
      mErrorMessage = errorMessageWithReason( mReply->errorString() );
      mErrorCode = QgsWFSRequest::ServerExceptionError;
      QgsMessageLog::logMessage( mErrorMessage, tr( "WFS" ) );
      mResponse.clear();
    }
  }
  if ( mTimedout )
    mErrorCode = QgsWFSRequest::TimeoutError;

  if ( mReply )
  {
    mReply->deleteLater();
    mReply = nullptr;
  }

  emit downloadFinished();
}

QString QgsWFSRequest::errorMessageFailedAuth()
{
  return errorMessageWithReason( tr( "network request update failed for authentication config" ) );
}
