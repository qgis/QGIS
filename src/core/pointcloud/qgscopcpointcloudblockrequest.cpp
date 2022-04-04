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
#include "qgseptdecoder.h"
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
    uint64_t blockOffset, int32_t blockSize, int pointCount, QByteArray lazHeader, QByteArray extraBytesData )
  : QgsPointCloudBlockRequest( node, uri, QStringLiteral( "copc" ), attributes, requestedAttributes, scale, offset, filterExpression ),
    mBlockOffset( blockOffset ), mBlockSize( blockSize ), mPointCount( pointCount ), mLazHeader( lazHeader ), mExtrabytesData( extraBytesData )
{
}

void QgsCopcPointCloudBlockRequest::startRequest()
{
  QNetworkRequest nr( mUri );
  nr.setAttribute( QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::AlwaysNetwork );
  nr.setAttribute( QNetworkRequest::CacheSaveControlAttribute, false );

  QByteArray queryRange = QStringLiteral( "bytes=%1-%2" ).arg( mBlockOffset ).arg( ( int64_t ) mBlockOffset + mBlockSize - 1 ).toLocal8Bit();
  nr.setRawHeader( "Range", queryRange );

  mTileDownloadManagetReply.reset( QgsApplication::tileDownloadManager()->get( nr ) );
  connect( mTileDownloadManagetReply.get(), &QgsTileDownloadManagerReply::finished, this, &QgsCopcPointCloudBlockRequest::blockFinishedLoading );
}


void QgsCopcPointCloudBlockRequest::blockFinishedLoading()
{
  mBlock = nullptr;
  if ( mTileDownloadManagetReply->error() == QNetworkReply::NetworkError::NoError )
  {
    try
    {
      mBlock = nullptr;
#ifdef HAVE_COPC
      std::istringstream file( mLazHeader.toStdString() );
      lazperf::header14 header = lazperf::header14::create( file );
      if ( mBlockSize != mTileDownloadManagetReply->data().size() )
      {
        QString err = QStringLiteral( "Failed to load node %1 properly %2" ).arg( mNode.toString() ).arg( QString::fromStdString( mTileDownloadManagetReply->request().rawHeader( "Range" ).toStdString() ) );
      }
      else
      {
        lazperf::eb_vlr ebVlr;
        ebVlr.fill( mExtrabytesData.data(), mExtrabytesData.size() );
        mBlock = QgsLazDecoder::decompressCopc( mTileDownloadManagetReply->data(), header, ebVlr, mPointCount, mAttributes, mRequestedAttributes, mScale, mOffset, mFilterExpression );
      }
#endif
    }
    catch ( std::exception &e )
    {
      mErrorStr = QStringLiteral( "Error while decompressing node %1: %2" ).arg( mNode.toString(), e.what() );
    }
    if ( !mBlock )
      mErrorStr = QStringLiteral( "Error loading point cloud tile: \" %1 \"" ).arg( mTileDownloadManagetReply->errorString() );
  }
  else
  {
    mErrorStr = mTileDownloadManagetReply->errorString();
  }
  emit finished();
}

///@endcond
