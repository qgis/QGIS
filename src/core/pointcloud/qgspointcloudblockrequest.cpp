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

//
// QgsPointCloudBlockRequest
//

///@cond PRIVATE

QgsPointCloudBlockRequest::QgsPointCloudBlockRequest( const QString &dataType, const QgsPointCloudAttributeCollection &attributes, const QgsPointCloudAttributeCollection &requestedAttributes, QgsTileDownloadManagerReply *tileDownloadManagerReply )
  : mDataType( dataType ), mAttributes( attributes ), mRequestedAttributes( requestedAttributes ), mTileDownloadManagetReply( tileDownloadManagerReply )
{
  connect( tileDownloadManagerReply, &QgsTileDownloadManagerReply::finished, this, &QgsPointCloudBlockRequest::blockFinishedLoading );
}

QgsPointCloudBlock *QgsPointCloudBlockRequest::block()
{
  return mBlock.get();
}

QString QgsPointCloudBlockRequest::errorStr()
{
  return mErrorStr;
}

void QgsPointCloudBlockRequest::blockFinishedLoading()
{
  mBlock.reset( nullptr );
  if ( mTileDownloadManagetReply->error() == QNetworkReply::NetworkError::NoError )
  {
    if ( mDataType == QLatin1String( "binary" ) )
    {
      mBlock.reset( QgsEptDecoder::decompressBinary( mTileDownloadManagetReply->data(), mAttributes, mRequestedAttributes ) );
    }
    else if ( mDataType == QLatin1String( "zstandard" ) )
    {
      mBlock.reset( QgsEptDecoder::decompressZStandard( mTileDownloadManagetReply->data(), mAttributes, mRequestedAttributes ) );
    }
    else if ( mDataType == QLatin1String( "laszip" ) )
    {
      mBlock.reset( QgsEptDecoder::decompressLaz( mTileDownloadManagetReply->data(), mAttributes, mRequestedAttributes ) );
    }

    if ( !mBlock.get() )
      mErrorStr = QStringLiteral( "unknown data type %1;" ).arg( mDataType ) +  mTileDownloadManagetReply->errorString();
  }
  else
  {
    mErrorStr = mTileDownloadManagetReply->errorString();
  }
  emit finished();
}

///@endcond
