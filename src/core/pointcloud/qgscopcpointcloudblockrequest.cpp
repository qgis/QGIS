/***************************************************************************
                         qgscopcpointcloudblockrequest.cpp
                         --------------------
    begin                : March 2022
    copyright            : (C) 2022 by Belgacem Nedjima
    email                : belgacem dot nedjima at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgscopcpointcloudblockrequest.h"

#include "qgstiledownloadmanager.h"
#include "qgslazdecoder.h"
#include "qgsapplication.h"
#include "qgsremotecopcpointcloudindex.h"

//
// QgsCopcPointCloudBlockRequest
//

///@cond PRIVATE

QgsCopcPointCloudBlockRequest::QgsCopcPointCloudBlockRequest( const IndexedPointCloudNode &node, const QString &uri,
    const QgsPointCloudAttributeCollection &attributes, const QgsPointCloudAttributeCollection &requestedAttributes,
    const QgsVector3D &scale, const QgsVector3D &offset, const QgsPointCloudExpression &filterExpression,
    uint64_t blockOffset, int32_t blockSize, int pointCount, const QgsLazInfo &lazInfo )
  : QgsPointCloudBlockRequest( node, uri, attributes, requestedAttributes, scale, offset, filterExpression ),
    mBlockOffset( blockOffset ), mBlockSize( blockSize ), mPointCount( pointCount ), mLazInfo( lazInfo )
{
  QNetworkRequest nr( mUri );
  nr.setAttribute( QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::PreferCache );
  nr.setAttribute( QNetworkRequest::CacheSaveControlAttribute, true );

  QByteArray queryRange = QStringLiteral( "bytes=%1-%2" ).arg( mBlockOffset ).arg( ( int64_t ) mBlockOffset + mBlockSize - 1 ).toLocal8Bit();
  nr.setRawHeader( "Range", queryRange );

  mTileDownloadManagerReply.reset( QgsApplication::tileDownloadManager()->get( nr ) );
  connect( mTileDownloadManagerReply.get(), &QgsTileDownloadManagerReply::finished, this, &QgsCopcPointCloudBlockRequest::blockFinishedLoading );
}

void QgsCopcPointCloudBlockRequest::blockFinishedLoading()
{
  mBlock = nullptr;
  QString error;
  if ( mTileDownloadManagerReply->error() == QNetworkReply::NetworkError::NoError )
  {
    if ( mBlockSize != mTileDownloadManagerReply->data().size() )
    {
      error = QStringLiteral( "Returned HTTP range is incorrect, requested %1 bytes but got %2 bytes" ).arg( mBlockSize ).arg( mTileDownloadManagerReply->data().size() );
    }
    else
    {
      try
      {
        mBlock = QgsLazDecoder::decompressCopc( mTileDownloadManagerReply->data(), mLazInfo, mPointCount, mRequestedAttributes, mFilterExpression );
      }
      catch ( std::exception &e )
      {
        error = QStringLiteral( "Decompression error: %1" ).arg( e.what() );
      }
    }
  }
  else
  {
    error = QStringLiteral( "Network request error: %1" ).arg( mTileDownloadManagerReply->errorString() );
  }
  if ( !error.isEmpty() )
  {
    mErrorStr = QStringLiteral( "Error loading point tile %1: \"%2\"" ).arg( mNode.toString() ).arg( error );
  }
  emit finished();
}

///@endcond
