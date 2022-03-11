/***************************************************************************
                         qgspointcloudblockrequest.cpp
                         --------------------
    begin                : March 2021
    copyright            : (C) 2021 by Belgacem Nedjima
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

#include "qgspointcloudblockrequest.h"

#include "qgstiledownloadmanager.h"
#include "qgseptdecoder.h"
#include "qgsapplication.h"

//
// QgsPointCloudBlockRequest
//

///@cond PRIVATE

QgsPointCloudBlockRequest::QgsPointCloudBlockRequest( const IndexedPointCloudNode &node, const QString &Uri, const QString &dataType,
    const QgsPointCloudAttributeCollection &attributes, const QgsPointCloudAttributeCollection &requestedAttributes,
    const QgsVector3D &scale, const QgsVector3D &offset, const QgsPointCloudExpression &filterExpression )
  : mNode( node ), mDataType( dataType ),
    mAttributes( attributes ), mRequestedAttributes( requestedAttributes ),
    mScale( scale ), mOffset( offset ), mFilterExpression( filterExpression )
{
  QNetworkRequest nr( Uri );
  nr.setAttribute( QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::PreferCache );
  nr.setAttribute( QNetworkRequest::CacheSaveControlAttribute, true );
  mTileDownloadManagetReply.reset( QgsApplication::tileDownloadManager()->get( nr ) );
  connect( mTileDownloadManagetReply.get(), &QgsTileDownloadManagerReply::finished, this, &QgsPointCloudBlockRequest::blockFinishedLoading );
}

QgsPointCloudBlock *QgsPointCloudBlockRequest::block()
{
  return mBlock;
}

QString QgsPointCloudBlockRequest::errorStr()
{
  return mErrorStr;
}

void QgsPointCloudBlockRequest::blockFinishedLoading()
{
  mBlock = nullptr;
  if ( mTileDownloadManagetReply->error() == QNetworkReply::NetworkError::NoError )
  {
    bool invalidDataType = false;
    try
    {
      mBlock = nullptr;
#ifdef WITH_EPT
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
        mBlock = QgsEptDecoder::decompressLaz( mTileDownloadManagetReply->data(), mAttributes, mRequestedAttributes, mScale, mOffset, mFilterExpression );
      }
      else
      {
        mErrorStr = QStringLiteral( "unknown data type %1;" ).arg( mDataType );
        invalidDataType = true;
      }
#endif
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
  emit finished();
}

///@endcond
