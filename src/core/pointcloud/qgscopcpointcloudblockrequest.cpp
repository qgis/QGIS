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

#include "qgsapplication.h"
#include "qgsauthmanager.h"
#include "qgslazdecoder.h"
#include "qgspointcloudindex.h"
#include "qgssetrequestinitiator_p.h"
#include "qgstiledownloadmanager.h"

#include "moc_qgscopcpointcloudblockrequest.cpp"

//
// QgsCopcPointCloudBlockRequest
//

///@cond PRIVATE

QgsCopcPointCloudBlockRequest::QgsCopcPointCloudBlockRequest( const QgsPointCloudNodeId &node, const QString &uri,
    const QgsPointCloudAttributeCollection &attributes, const QgsPointCloudAttributeCollection &requestedAttributes,
    const QgsVector3D &scale, const QgsVector3D &offset, const QgsPointCloudExpression &filterExpression, const QgsRectangle &filterRect,
    uint64_t blockOffset, int32_t blockSize, int pointCount, const QgsLazInfo &lazInfo,
    const QString &authcfg )
  : QgsPointCloudBlockRequest( node, uri, attributes, requestedAttributes, scale, offset, filterExpression, filterRect ),
    mBlockOffset( blockOffset ), mBlockSize( blockSize ), mPointCount( pointCount ), mLazInfo( lazInfo )
{
  // an empty block size will create an invalid range, causing a full request to the server
  Q_ASSERT( mBlockSize > 0 );

  QNetworkRequest nr = QNetworkRequest( QUrl( mUri ) );
  QgsSetRequestInitiatorClass( nr, u"QgsCopcPointCloudBlockRequest"_s );
  QgsSetRequestInitiatorId( nr, node.toString() );
  nr.setAttribute( QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::PreferCache );
  nr.setAttribute( QNetworkRequest::CacheSaveControlAttribute, true );

  QByteArray queryRange = u"bytes=%1-%2"_s.arg( mBlockOffset ).arg( ( int64_t ) mBlockOffset + mBlockSize - 1 ).toLocal8Bit();
  nr.setRawHeader( "Range", queryRange );

  if ( !authcfg.isEmpty() )
    QgsApplication::authManager()->updateNetworkRequest( nr, authcfg );

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
      error = u"Returned HTTP range is incorrect, requested %1 bytes but got %2 bytes"_s.arg( mBlockSize ).arg( mTileDownloadManagerReply->data().size() );
    }
    else
    {
      try
      {
        mBlock = QgsLazDecoder::decompressCopc( mTileDownloadManagerReply->data(), mLazInfo, mPointCount, mRequestedAttributes, mFilterExpression, mFilterRect );
        QgsPointCloudRequest req;
        req.setAttributes( mRequestedAttributes );
        req.setFilterRect( mFilterRect );
        QgsAbstractPointCloudIndex::storeNodeDataToCacheStatic( mBlock.get(), mNode, req, mFilterExpression, mUri );
      }
      catch ( std::exception &e )
      {
        error = u"Decompression error: %1"_s.arg( e.what() );
      }
    }
  }
  else
  {
    error = u"Network request error: %1"_s.arg( mTileDownloadManagerReply->errorString() );
  }
  if ( !error.isEmpty() )
  {
    mErrorStr = u"Error loading point tile %1: \"%2\""_s.arg( mNode.toString(), error );
  }
  emit finished();
}

///@endcond
