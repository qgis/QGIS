/***************************************************************************
                         qgspointcloudblockhandle.cpp
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

#include "qgspointcloudblockhandle.h"

#include "qgstiledownloadmanager.h"
#include "qgseptdecoder.h"

//
// QgsPointCloudBlockHandle
//

///@cond PRIVATE

QgsPointCloudBlockHandle::QgsPointCloudBlockHandle( const QString &dataType, const QgsPointCloudAttributeCollection &attributes, const QgsPointCloudAttributeCollection &requestedAttributes, QgsTileDownloadManagerReply *tileDownloadManagerReply )
  : mDataType( dataType ), mAttributes( attributes ), mRequestedAttributes( requestedAttributes ), mTileDownloadManagetReply( tileDownloadManagerReply )
{
  connect( mTileDownloadManagetReply, &QgsTileDownloadManagerReply::finished, this, &QgsPointCloudBlockHandle::blockFinishedLoading );
}

void QgsPointCloudBlockHandle::blockFinishedLoading()
{
  if ( mTileDownloadManagetReply->error() == QNetworkReply::NetworkError::NoError )
  {
    QgsPointCloudBlock *block = nullptr;
    if ( mDataType == QLatin1String( "binary" ) )
    {
      block = QgsEptDecoder::decompressBinary( mTileDownloadManagetReply->data(), mAttributes, mRequestedAttributes );
    }
    else if ( mDataType == QLatin1String( "zstandard" ) )
    {
      block = QgsEptDecoder::decompressZStandard( mTileDownloadManagetReply->data(), mAttributes, mRequestedAttributes );
    }
    else if ( mDataType == QLatin1String( "laszip" ) )
    {
      block = QgsEptDecoder::decompressLaz( mTileDownloadManagetReply->data(), mAttributes, mRequestedAttributes );
    }
    if ( block == nullptr )
      emit blockLoadingFailed( QStringLiteral( "unknown data type %1;" ).arg( mDataType ) +  mTileDownloadManagetReply->errorString() );
    else
      emit blockLoadingSucceeded( block );
  }
  else
  {
    emit blockLoadingFailed( mTileDownloadManagetReply->errorString() );
  }
}

///@endcond
