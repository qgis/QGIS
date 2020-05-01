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
#include "qgsnetworkaccessmanager.h"
#include "qgsvectortileutils.h"

QgsVectorTileLoader::QgsVectorTileLoader( const QString &uri, int zoomLevel, const QgsTileRange &range, const QPointF &viewCenter, QgsFeedback *feedback )
  : mEventLoop( new QEventLoop )
  , mFeedback( feedback )
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
  QVector<QgsTileXYZ> tiles = QgsVectorTileUtils::tilesInRange( range, zoomLevel );
  QgsVectorTileUtils::sortTilesByDistanceFromCenter( tiles, viewCenter );
  for ( QgsTileXYZ id : qgis::as_const( tiles ) )
  {
    loadFromNetworkAsync( id, uri );
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

void QgsVectorTileLoader::loadFromNetworkAsync( const QgsTileXYZ &id, const QString &requestUrl )
{
  QString url = QgsVectorTileUtils::formatXYZUrlTemplate( requestUrl, id );
  QNetworkRequest request( url );
  QgsSetRequestInitiatorClass( request, QStringLiteral( "QgsVectorTileLoader" ) );
  QgsSetRequestInitiatorId( request, id.toString() );

  request.setAttribute( static_cast<QNetworkRequest::Attribute>( QNetworkRequest::User + 1 ), id.column() );
  request.setAttribute( static_cast<QNetworkRequest::Attribute>( QNetworkRequest::User + 2 ), id.row() );
  request.setAttribute( static_cast<QNetworkRequest::Attribute>( QNetworkRequest::User + 3 ), id.zoomLevel() );

  request.setAttribute( QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::PreferCache );
  request.setAttribute( QNetworkRequest::CacheSaveControlAttribute, true );

  QNetworkReply *reply = QgsNetworkAccessManager::instance()->get( request );
  connect( reply, &QNetworkReply::finished, this, &QgsVectorTileLoader::tileReplyFinished );

  mReplies << reply;
}

void QgsVectorTileLoader::tileReplyFinished()
{
  QNetworkReply *reply = qobject_cast<QNetworkReply *>( sender() );

  int reqX = reply->request().attribute( static_cast<QNetworkRequest::Attribute>( QNetworkRequest::User + 1 ) ).toInt();
  int reqY = reply->request().attribute( static_cast<QNetworkRequest::Attribute>( QNetworkRequest::User + 2 ) ).toInt();
  int reqZ = reply->request().attribute( static_cast<QNetworkRequest::Attribute>( QNetworkRequest::User + 3 ) ).toInt();
  QgsTileXYZ tileID( reqX, reqY, reqZ );

  if ( reply->error() == QNetworkReply::NoError )
  {
    // TODO: handle redirections?

    QgsDebugMsgLevel( QStringLiteral( "Tile download successful: " ) + tileID.toString(), 2 );
    QByteArray rawData = reply->readAll();
    mReplies.removeOne( reply );
    reply->deleteLater();

    emit tileRequestFinished( QgsVectorTileRawData( tileID, rawData ) );
  }
  else
  {
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
  const QList<QNetworkReply *> replies = mReplies;
  for ( QNetworkReply *reply : replies )
  {
    reply->abort();
  }
}

//////

QList<QgsVectorTileRawData> QgsVectorTileLoader::blockingFetchTileRawData( const QString &sourceType, const QString &sourcePath, int zoomLevel, const QPointF &viewCenter, const QgsTileRange &range )
{
  QList<QgsVectorTileRawData> rawTiles;

  QgsMbTiles mbReader( sourcePath );
  bool isUrl = ( sourceType == QStringLiteral( "xyz" ) );
  if ( !isUrl )
  {
    bool res = mbReader.open();
    Q_ASSERT( res );
  }

  QVector<QgsTileXYZ> tiles = QgsVectorTileUtils::tilesInRange( range, zoomLevel );
  QgsVectorTileUtils::sortTilesByDistanceFromCenter( tiles, viewCenter );
  for ( QgsTileXYZ id : qgis::as_const( tiles ) )
  {
    QByteArray rawData = isUrl ? loadFromNetwork( id, sourcePath ) : loadFromMBTiles( id, mbReader );
    if ( !rawData.isEmpty() )
    {
      rawTiles.append( QgsVectorTileRawData( id, rawData ) );
    }
  }
  return rawTiles;
}

QByteArray QgsVectorTileLoader::loadFromNetwork( const QgsTileXYZ &id, const QString &requestUrl )
{
  QString url = QgsVectorTileUtils::formatXYZUrlTemplate( requestUrl, id );
  QNetworkRequest nr;
  nr.setUrl( QUrl( url ) );
  QgsBlockingNetworkRequest req;
  QgsDebugMsgLevel( QStringLiteral( "Blocking request: " ) + url, 2 );
  QgsBlockingNetworkRequest::ErrorCode errCode = req.get( nr );
  if ( errCode != QgsBlockingNetworkRequest::NoError )
  {
    QgsDebugMsg( QStringLiteral( "Request failed: " ) + url );
    return QByteArray();
  }
  QgsNetworkReplyContent reply = req.reply();
  QgsDebugMsgLevel( QStringLiteral( "Request successful, content size %1" ).arg( reply.content().size() ), 2 );
  return reply.content();
}


QByteArray QgsVectorTileLoader::loadFromMBTiles( const QgsTileXYZ &id, QgsMbTiles &mbTileReader )
{
  // MBTiles uses TMS specs with Y starting at the bottom while XYZ uses Y starting at the top
  int rowTMS = pow( 2, id.zoomLevel() ) - id.row() - 1;
  QByteArray gzippedTileData = mbTileReader.tileData( id.zoomLevel(), id.column(), rowTMS );
  if ( gzippedTileData.isEmpty() )
  {
    QgsDebugMsg( QStringLiteral( "Failed to get tile " ) + id.toString() );
    return QByteArray();
  }

  QByteArray data;
  if ( !QgsMbTiles::decodeGzip( gzippedTileData, data ) )
  {
    QgsDebugMsg( QStringLiteral( "Failed to decompress tile " ) + id.toString() );
    return QByteArray();
  }

  QgsDebugMsgLevel( QStringLiteral( "Tile blob size %1 -> uncompressed size %2" ).arg( gzippedTileData.size() ).arg( data.size() ), 2 );
  return data;
}
