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
#include "qgslogger.h"
#include "qgsvectortileutils.h"
#include "qgsapplication.h"
#include "qgsvectortiledataprovider.h"
#include "qgsfeedback.h"
#include "qgstiledownloadmanager.h"

#include <QEventLoop>


QgsVectorTileLoader::QgsVectorTileLoader( const QgsVectorTileDataProvider *provider, const QgsTileMatrixSet &tileMatrixSet, const QgsTileRange &range, int zoomLevel, const QPointF &viewCenter, QgsFeedback *feedback, Qgis::RendererUsage usage )
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
  QVector<QgsTileXYZ> tiles = tileMatrixSet.tilesInRange( range, zoomLevel );
  QgsVectorTileUtils::sortTilesByDistanceFromCenter( tiles, viewCenter );
  for ( QgsTileXYZ id : std::as_const( tiles ) )
  {
    loadFromNetworkAsync( id, tileMatrixSet, provider, usage );
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

void QgsVectorTileLoader::loadFromNetworkAsync( const QgsTileXYZ &id, const QgsTileMatrixSet &tileMatrixSet, const QgsVectorTileDataProvider *provider, Qgis::RendererUsage usage )
{
  QNetworkRequest request = provider->tileRequest( tileMatrixSet, id, usage );

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

    QgsDebugError( QStringLiteral( "Tile download failed! " ) + reply->errorString() );
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

QList<QgsVectorTileRawData> QgsVectorTileLoader::blockingFetchTileRawData( const QgsVectorTileDataProvider *provider, const QgsTileMatrixSet &tileMatrixSet, const QPointF &viewCenter, const QgsTileRange &range, int zoomLevel, QgsFeedback *feedback, Qgis::RendererUsage usage )
{
  if ( feedback && feedback->isCanceled() )
    return {};

  QVector<QgsTileXYZ> tiles = tileMatrixSet.tilesInRange( range, zoomLevel );

  // if a tile matrix results in a HUGE number of tile requests, we skip the sort -- it can be expensive
  if ( tiles.size() < 10000 )
    QgsVectorTileUtils::sortTilesByDistanceFromCenter( tiles, viewCenter );

  return provider->readTiles( tileMatrixSet, tiles, feedback, usage );
}
