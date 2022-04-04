/***************************************************************************
  qgsvectortileloader.cpp
  --------------------------------------
  Date                 : March 2020
  Copyright            : (C) 2020 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsvectortileloader.h"

#include <QEventLoop>

#include "qgsblockingnetworkrequest.h"
#include "qgslogger.h"
#include "qgsmbtiles.h"
#include "qgsvtpktiles.h"
#include "qgsnetworkaccessmanager.h"
#include "qgsvectortileutils.h"
#include "qgsapplication.h"
#include "qgsauthmanager.h"
#include "qgsmessagelog.h"
#include "qgsziputils.h"

#include "qgstiledownloadmanager.h"

QgsVectorTileLoader::QgsVectorTileLoader( const QString &uri, const QgsTileMatrix &tileMatrix, const QgsTileRange &range, const QPointF &viewCenter, const QString &authid, const QgsHttpHeaders &headers, QgsFeedback *feedback )
  : mEventLoop( new QEventLoop )
  , mFeedback( feedback )
  , mAuthCfg( authid )
  , mHeaders( headers )
{
  if ( feedback )
  {
    connect( feedback, &QgsFeedback::canceled, this, &QgsVectorTileLoader::canceled, Qt::QueuedConnection );

    // rendering could have been canceled before we started to listen to canceled() signal
    // so let's check before doing the download and maybe quit prematurely
    if ( feedback->isCanceled() )
      return;
  }

  QgsDebugMsgLevel( QStringLiteral( "Starting network loader" ), 2 );
  QVector<QgsTileXYZ> tiles = QgsVectorTileUtils::tilesInRange( range, tileMatrix.zoomLevel() );
  QgsVectorTileUtils::sortTilesByDistanceFromCenter( tiles, viewCenter );
  for ( QgsTileXYZ id : std::as_const( tiles ) )
  {
    loadFromNetworkAsync( id, tileMatrix, uri );
  }
}

QgsVectorTileLoader::~QgsVectorTileLoader()
{
  QgsDebugMsgLevel( QStringLiteral( "Terminating network loader" ), 2 );

  if ( !mReplies.isEmpty() )
  {
    // this can happen when the loader is terminated without getting requests finalized
    // (e.g. downloadBlocking() was not called)
    canceled();
  }
}

void QgsVectorTileLoader::downloadBlocking()
{
  if ( mFeedback && mFeedback->isCanceled() )
  {
    QgsDebugMsgLevel( QStringLiteral( "downloadBlocking - not staring event loop - canceled" ), 2 );
    return; // nothing to do
  }

  QgsDebugMsgLevel( QStringLiteral( "Starting event loop with %1 requests" ).arg( mReplies.count() ), 2 );

  mEventLoop->exec( QEventLoop::ExcludeUserInputEvents );

  QgsDebugMsgLevel( QStringLiteral( "downloadBlocking finished" ), 2 );

  Q_ASSERT( mReplies.isEmpty() );
}

void QgsVectorTileLoader::loadFromNetworkAsync( const QgsTileXYZ &id, const QgsTileMatrix &tileMatrix, const QString &requestUrl )
{
  QString url = QgsVectorTileUtils::formatXYZUrlTemplate( requestUrl, id, tileMatrix );
  QNetworkRequest request( url );
  QgsSetRequestInitiatorClass( request, QStringLiteral( "QgsVectorTileLoader" ) );
  QgsSetRequestInitiatorId( request, id.toString() );

  request.setAttribute( static_cast<QNetworkRequest::Attribute>( QNetworkRequest::User + 1 ), id.column() );
  request.setAttribute( static_cast<QNetworkRequest::Attribute>( QNetworkRequest::User + 2 ), id.row() );
  request.setAttribute( static_cast<QNetworkRequest::Attribute>( QNetworkRequest::User + 3 ), id.zoomLevel() );

  request.setAttribute( QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::PreferCache );
  request.setAttribute( QNetworkRequest::CacheSaveControlAttribute, true );

  mHeaders.updateNetworkRequest( request );

  if ( !mAuthCfg.isEmpty() &&  !QgsApplication::authManager()->updateNetworkRequest( request, mAuthCfg ) )
  {
    QgsMessageLog::logMessage( tr( "network request update failed for authentication config" ), tr( "Network" ) );
  }

  QgsTileDownloadManagerReply *reply = QgsApplication::tileDownloadManager()->get( request );
  connect( reply, &QgsTileDownloadManagerReply::finished, this, &QgsVectorTileLoader::tileReplyFinished );
  mReplies << reply;
}

void QgsVectorTileLoader::tileReplyFinished()
{
  QgsTileDownloadManagerReply *reply = qobject_cast<QgsTileDownloadManagerReply *>( sender() );

  int reqX = reply->request().attribute( static_cast<QNetworkRequest::Attribute>( QNetworkRequest::User + 1 ) ).toInt();
  int reqY = reply->request().attribute( static_cast<QNetworkRequest::Attribute>( QNetworkRequest::User + 2 ) ).toInt();
  int reqZ = reply->request().attribute( static_cast<QNetworkRequest::Attribute>( QNetworkRequest::User + 3 ) ).toInt();
  QgsTileXYZ tileID( reqX, reqY, reqZ );

  if ( reply->error() == QNetworkReply::NoError )
  {
    // TODO: handle redirections?

    QgsDebugMsgLevel( QStringLiteral( "Tile download successful: " ) + tileID.toString(), 2 );
    QByteArray rawData = reply->data();
    mReplies.removeOne( reply );
    reply->deleteLater();

    emit tileRequestFinished( QgsVectorTileRawData( tileID, rawData ) );
  }
  else
  {
    if ( reply->error() == QNetworkReply::ContentAccessDenied )
    {
      if ( reply->data().isEmpty() )
        mError = tr( "Access denied" );
      else
        mError = tr( "Access denied: %1" ).arg( QString( reply->data() ) );
    }

    QgsDebugMsg( QStringLiteral( "Tile download failed! " ) + reply->errorString() );
    mReplies.removeOne( reply );
    reply->deleteLater();

    emit tileRequestFinished( QgsVectorTileRawData( tileID, QByteArray() ) );
  }

  if ( mReplies.isEmpty() )
  {
    // exist the event loop
    QMetaObject::invokeMethod( mEventLoop.get(), "quit", Qt::QueuedConnection );
  }
}

void QgsVectorTileLoader::canceled()
{
  QgsDebugMsgLevel( QStringLiteral( "Canceling %1 pending requests" ).arg( mReplies.count() ), 2 );
  qDeleteAll( mReplies );
  mReplies.clear();

  // stop blocking download
  mEventLoop->quit();

}

QString QgsVectorTileLoader::error() const
{
  return mError;
}

//////

QList<QgsVectorTileRawData> QgsVectorTileLoader::blockingFetchTileRawData( const QString &sourceType, const QString &sourcePath, const QgsTileMatrix &tileMatrix, const QPointF &viewCenter, const QgsTileRange &range, const QString &authid, const QgsHttpHeaders &headers, QgsFeedback *feedback )
{
  QList<QgsVectorTileRawData> rawTiles;

  std::unique_ptr< QgsMbTiles > mbReader;
  std::unique_ptr< QgsVtpkTiles > vtpkReader;
  bool isUrl = ( sourceType == QLatin1String( "xyz" ) );
  if ( sourceType == QLatin1String( "vtpk" ) )
  {
    vtpkReader = std::make_unique< QgsVtpkTiles >( sourcePath );
    vtpkReader->open();
  }
  else if ( !isUrl )
  {
    mbReader = std::make_unique< QgsMbTiles >( sourcePath );
    bool res = mbReader->open();
    Q_UNUSED( res )
    Q_ASSERT( res );
  }

  if ( feedback && feedback->isCanceled() )
    return {};

  QVector<QgsTileXYZ> tiles = QgsVectorTileUtils::tilesInRange( range, tileMatrix.zoomLevel() );

  // if a tile matrix results in a HUGE number of tile requests, we skip the sort -- it can be expensive
  if ( tiles.size() < 10000 )
    QgsVectorTileUtils::sortTilesByDistanceFromCenter( tiles, viewCenter );

  rawTiles.reserve( tiles.size() );
  for ( QgsTileXYZ id : std::as_const( tiles ) )
  {
    if ( feedback && feedback->isCanceled() )
      return rawTiles;

    QByteArray rawData = isUrl ? loadFromNetwork( id, tileMatrix, sourcePath, authid, headers, feedback )
                         : ( mbReader ? loadFromMBTiles( id, *mbReader ) : loadFromVtpk( id, *vtpkReader ) );
    if ( !rawData.isEmpty() )
    {
      rawTiles.append( QgsVectorTileRawData( id, rawData ) );
    }
  }
  return rawTiles;
}

QByteArray QgsVectorTileLoader::loadFromNetwork( const QgsTileXYZ &id, const QgsTileMatrix &tileMatrix, const QString &requestUrl, const QString &authid, const QgsHttpHeaders &headers, QgsFeedback *feedback )
{
  QString url = QgsVectorTileUtils::formatXYZUrlTemplate( requestUrl, id, tileMatrix );
  QNetworkRequest nr;
  nr.setUrl( QUrl( url ) );

  headers.updateNetworkRequest( nr );

  QgsBlockingNetworkRequest req;
  req.setAuthCfg( authid );
  QgsDebugMsgLevel( QStringLiteral( "Blocking request: " ) + url, 2 );
  QgsBlockingNetworkRequest::ErrorCode errCode = req.get( nr, false, feedback );
  if ( errCode != QgsBlockingNetworkRequest::NoError )
  {
    QgsDebugMsg( QStringLiteral( "Request failed: " ) + url );
    return QByteArray();
  }
  QgsNetworkReplyContent reply = req.reply();
  QgsDebugMsgLevel( QStringLiteral( "Request successful, content size %1" ).arg( reply.content().size() ), 2 );
  return reply.content();
}


QByteArray QgsVectorTileLoader::loadFromMBTiles( const QgsTileXYZ &id, QgsMbTiles &mbTileReader, QgsFeedback *feedback )
{
  // MBTiles uses TMS specs with Y starting at the bottom while XYZ uses Y starting at the top
  int rowTMS = pow( 2, id.zoomLevel() ) - id.row() - 1;
  QByteArray gzippedTileData = mbTileReader.tileData( id.zoomLevel(), id.column(), rowTMS );
  if ( gzippedTileData.isEmpty() )
  {
    return QByteArray();
  }

  if ( feedback && feedback->isCanceled() )
    return QByteArray();

  QByteArray data;
  if ( !QgsZipUtils::decodeGzip( gzippedTileData, data ) )
  {
    QgsDebugMsg( QStringLiteral( "Failed to decompress tile " ) + id.toString() );
    return QByteArray();
  }

  QgsDebugMsgLevel( QStringLiteral( "Tile blob size %1 -> uncompressed size %2" ).arg( gzippedTileData.size() ).arg( data.size() ), 2 );
  return data;
}

QByteArray QgsVectorTileLoader::loadFromVtpk( const QgsTileXYZ &id, QgsVtpkTiles &vtpkTileReader )
{
  QByteArray tileData = vtpkTileReader.tileData( id.zoomLevel(), id.column(), id.row() );
  if ( tileData.isEmpty() )
  {
    return QByteArray();
  }
  return tileData;
}
