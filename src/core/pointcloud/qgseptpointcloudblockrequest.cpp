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

#include "qgseptpointcloudblockrequest.h"

#include "qgstiledownloadmanager.h"
#include "qgseptdecoder.h"
#include "qgslazdecoder.h"
#include "qgsapplication.h"
#include "qgsremoteeptpointcloudindex.h"

//
// QgsEptPointCloudBlockRequest
//

///@cond PRIVATE

QgsEptPointCloudBlockRequest::QgsEptPointCloudBlockRequest( const IndexedPointCloudNode &node, const QString &uri, const QString &dataType,
    const QgsPointCloudAttributeCollection &attributes, const QgsPointCloudAttributeCollection &requestedAttributes,
    const QgsVector3D &scale, const QgsVector3D &offset, const QgsPointCloudExpression &filterExpression )
  : QgsPointCloudBlockRequest( node, uri, attributes, requestedAttributes, scale, offset, filterExpression ),
    mDataType( dataType )
{
  QNetworkRequest nr( mUri );
  nr.setAttribute( QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::PreferCache );
  nr.setAttribute( QNetworkRequest::CacheSaveControlAttribute, true );
  mTileDownloadManagetReply.reset( QgsApplication::tileDownloadManager()->get( nr ) );
  connect( mTileDownloadManagetReply.get(), &QgsTileDownloadManagerReply::finished, this, &QgsEptPointCloudBlockRequest::blockFinishedLoading );
}

void QgsEptPointCloudBlockRequest::blockFinishedLoading()
{
  mBlock = nullptr;
  if ( mTileDownloadManagetReply->error() == QNetworkReply::NetworkError::NoError )
  {
    bool invalidDataType = false;
    try
    {
      mBlock = nullptr;
      if ( mDataType == QLatin1String( "binary" ) )
      {
        mBlock = QgsEptDecoder::decompressBinary( mTileDownloadManagetReply->data(), mAttributes, mRequestedAttributes, mScale, mOffset, mFilterExpression );
      }
      else if ( mDataType == QLatin1String( "zstandard" ) )
      {
        mBlock = QgsEptDecoder::decompressZStandard( mTileDownloadManagetReply->data(), mAttributes, mRequestedAttributes, mScale, mOffset, mFilterExpression );
      }
      else if ( mDataType == QLatin1String( "laszip" ) )
      {
        mBlock = QgsLazDecoder::decompressLaz( mTileDownloadManagetReply->data(), mAttributes, mRequestedAttributes, mScale, mOffset, mFilterExpression );
      }
      else
      {
        mErrorStr = QStringLiteral( "unknown data type %1;" ).arg( mDataType );
        invalidDataType = true;
      }
    }
    catch ( std::exception &e )
    {
      mErrorStr = QStringLiteral( "Error while decompressing node %1: %2" ).arg( mNode.toString(), e.what() );
    }
    if ( invalidDataType && !mBlock )
      mErrorStr = QStringLiteral( "Error loading point cloud tile: \" %1 \"" ).arg( mTileDownloadManagetReply->errorString() );
  }
  else
  {
    mErrorStr = mTileDownloadManagetReply->errorString();
  }
  emit QgsPointCloudBlockRequest::finished();
}

///@endcond
