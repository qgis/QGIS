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
#include "moc_qgseptpointcloudblockrequest.cpp"

#include "qgstiledownloadmanager.h"
#include "qgseptdecoder.h"
#include "qgslazdecoder.h"
#include "qgsapplication.h"
#include "qgsnetworkaccessmanager.h"
#include "qgssetrequestinitiator_p.h"

//
// QgsEptPointCloudBlockRequest
//

///@cond PRIVATE

QgsEptPointCloudBlockRequest::QgsEptPointCloudBlockRequest( const QgsPointCloudNodeId &node, const QString &uri, const QString &dataType,
    const QgsPointCloudAttributeCollection &attributes, const QgsPointCloudAttributeCollection &requestedAttributes,
    const QgsVector3D &scale, const QgsVector3D &offset, const QgsPointCloudExpression &filterExpression, const QgsRectangle &filterRect )
  : QgsPointCloudBlockRequest( node, uri, attributes, requestedAttributes, scale, offset, filterExpression, filterRect ),
    mDataType( dataType )
{
  QNetworkRequest nr = QNetworkRequest( QUrl( mUri ) );
  QgsSetRequestInitiatorClass( nr, QStringLiteral( "QgsEptPointCloudBlockRequest" ) );
  QgsSetRequestInitiatorId( nr, node.toString() );
  nr.setAttribute( QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::PreferCache );
  nr.setAttribute( QNetworkRequest::CacheSaveControlAttribute, true );
  mTileDownloadManagerReply.reset( QgsApplication::tileDownloadManager()->get( nr ) );
  connect( mTileDownloadManagerReply.get(), &QgsTileDownloadManagerReply::finished, this, &QgsEptPointCloudBlockRequest::blockFinishedLoading );
}

void QgsEptPointCloudBlockRequest::blockFinishedLoading()
{
  mBlock = nullptr;
  QString error;
  if ( mTileDownloadManagerReply->error() == QNetworkReply::NetworkError::NoError )
  {
    try
    {
      mBlock = nullptr;
      if ( mDataType == QLatin1String( "binary" ) )
      {
        mBlock = QgsEptDecoder::decompressBinary( mTileDownloadManagerReply->data(), mAttributes, mRequestedAttributes, mScale, mOffset, mFilterExpression, mFilterRect );
      }
      else if ( mDataType == QLatin1String( "zstandard" ) )
      {
        mBlock = QgsEptDecoder::decompressZStandard( mTileDownloadManagerReply->data(), mAttributes, mRequestedAttributes, mScale, mOffset, mFilterExpression, mFilterRect );
      }
      else if ( mDataType == QLatin1String( "laszip" ) )
      {
        mBlock = QgsLazDecoder::decompressLaz( mTileDownloadManagerReply->data(), mRequestedAttributes, mFilterExpression, mFilterRect );
      }
      else
      {
        error = QStringLiteral( "Unknown data type %1;" ).arg( mDataType );
      }
      if ( mBlock )
      {
        QgsPointCloudRequest req;
        req.setAttributes( mRequestedAttributes );
        req.setFilterRect( mFilterRect );
        QgsAbstractPointCloudIndex::storeNodeDataToCacheStatic( mBlock.get(), mNode, req, mFilterExpression, mUri );
      }
    }
    catch ( std::exception &e )
    {
      error = QStringLiteral( "Decompression error: %1" ).arg( e.what() );
    }
  }
  else
  {
    error = QStringLiteral( "Network request error: %1" ).arg( mTileDownloadManagerReply->errorString() );
  }
  if ( !error.isEmpty() )
  {
    mErrorStr = QStringLiteral( "Error loading point cloud tile %1: \" %2 \"" ).arg( mNode.toString(), error );
  }
  emit finished();
}

///@endcond
