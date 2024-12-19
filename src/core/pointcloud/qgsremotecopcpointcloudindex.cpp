/***************************************************************************
                         qgsremotecopcpointcloudindex.cpp
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

#include "qgsremotecopcpointcloudindex.h"

#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTime>
#include <QtDebug>
#include <QQueue>
#include <QTimer>

#include "qgspointcloudrequest.h"
#include "qgspointcloudattribute.h"
#include "qgslogger.h"
#include "qgstiledownloadmanager.h"
#include "qgsapplication.h"
#include "qgscopcpointcloudblockrequest.h"
#include "qgscachedpointcloudblockrequest.h"
#include "qgspointcloudexpression.h"
#include "qgsnetworkaccessmanager.h"
#include "qgssetrequestinitiator_p.h"

///@cond PRIVATE

QgsRemoteCopcPointCloudIndex::QgsRemoteCopcPointCloudIndex() = default;

QgsRemoteCopcPointCloudIndex::~QgsRemoteCopcPointCloudIndex() = default;

std::unique_ptr<QgsPointCloudIndex> QgsRemoteCopcPointCloudIndex::clone() const
{
  QgsRemoteCopcPointCloudIndex *clone = new QgsRemoteCopcPointCloudIndex;
  QMutexLocker locker( &mHierarchyMutex );
  copyCommonProperties( clone );
  return std::unique_ptr<QgsPointCloudIndex>( clone );
}

void QgsRemoteCopcPointCloudIndex::load( const QString &uri )
{
  mUri = uri;
  QUrl url( uri );
  mLazInfo.reset( new QgsLazInfo( QgsLazInfo::fromUrl( url ) ) );
  mIsValid = mLazInfo->isValid();
  if ( mIsValid )
  {
    mIsValid = loadSchema( *mLazInfo.get() );
    if ( mIsValid )
    {
      loadHierarchy();
    }
  }
  if ( !mIsValid )
  {
    mError = tr( "Unable to recognize %1 as a LAZ file: \"%2\"" ).arg( uri, mLazInfo->error() );
  }
}

std::unique_ptr<QgsPointCloudBlock> QgsRemoteCopcPointCloudIndex::nodeData( const IndexedPointCloudNode &n, const QgsPointCloudRequest &request )
{
  if ( QgsPointCloudBlock *cached = getNodeDataFromCache( n, request ) )
  {
    return std::unique_ptr<QgsPointCloudBlock>( cached );
  }

  std::unique_ptr<QgsPointCloudBlockRequest> blockRequest( asyncNodeData( n, request ) );
  if ( !blockRequest )
    return nullptr;

  QEventLoop loop;
  connect( blockRequest.get(), &QgsPointCloudBlockRequest::finished, &loop, &QEventLoop::quit );
  loop.exec();

  std::unique_ptr<QgsPointCloudBlock> block = blockRequest->takeBlock();

  if ( !block )
  {
    QgsDebugError( QStringLiteral( "Error downloading node %1 data, error : %2 " ).arg( n.toString(), blockRequest->errorStr() ) );
  }

  storeNodeDataToCache( block.get(), n, request );
  return block;
}

QgsPointCloudBlockRequest *QgsRemoteCopcPointCloudIndex::asyncNodeData( const IndexedPointCloudNode &n, const QgsPointCloudRequest &request )
{
  if ( QgsPointCloudBlock *cached = getNodeDataFromCache( n, request ) )
  {
    return new QgsCachedPointCloudBlockRequest( cached,  n, mUri, attributes(), request.attributes(),
           scale(), offset(), mFilterExpression, request.filterRect() );
  }

  if ( !fetchNodeHierarchy( n ) )
    return nullptr;
  QMutexLocker locker( &mHierarchyMutex );

  // we need to create a copy of the expression to pass to the decoder
  // as the same QgsPointCloudExpression object might be concurrently
  // used on another thread, for example in a 3d view
  QgsPointCloudExpression filterExpression = mFilterExpression;
  QgsPointCloudAttributeCollection requestAttributes = request.attributes();
  requestAttributes.extend( attributes(), filterExpression.referencedAttributes() );
  auto [ blockOffset, blockSize ] = mHierarchyNodePos.value( n );
  int pointCount = mHierarchy.value( n );

  return new QgsCopcPointCloudBlockRequest( n, mUri, attributes(), requestAttributes,
         scale(), offset(), filterExpression, request.filterRect(),
         blockOffset, blockSize, pointCount, *mLazInfo.get() );
}

bool QgsRemoteCopcPointCloudIndex::isValid() const
{
  return mIsValid;
}

void QgsRemoteCopcPointCloudIndex::fetchHierarchyPage( uint64_t offset, uint64_t byteSize ) const
{
  Q_ASSERT( byteSize > 0 );

  QNetworkRequest nr = QNetworkRequest( QUrl( mUri ) );
  QgsSetRequestInitiatorClass( nr, QStringLiteral( "QgsRemoteCopcPointCloudIndex" ) );
  nr.setAttribute( QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::PreferCache );
  nr.setAttribute( QNetworkRequest::CacheSaveControlAttribute, true );
  QByteArray queryRange = QStringLiteral( "bytes=%1-%2" ).arg( offset ).arg( offset + byteSize - 1 ).toLocal8Bit();
  nr.setRawHeader( "Range", queryRange );

  std::unique_ptr<QgsTileDownloadManagerReply> reply( QgsApplication::tileDownloadManager()->get( nr ) );

  QEventLoop loop;
  connect( reply.get(), &QgsTileDownloadManagerReply::finished, &loop, &QEventLoop::quit );
  loop.exec();

  if ( reply->error() != QNetworkReply::NoError )
  {
    QgsDebugError( QStringLiteral( "Request failed: " ) + mUri );
    return;
  }

  QByteArray data = reply->data();

  populateHierarchy( data.constData(), byteSize );
}

void QgsRemoteCopcPointCloudIndex::copyCommonProperties( QgsRemoteCopcPointCloudIndex *destination ) const
{
  QgsCopcPointCloudIndex::copyCommonProperties( destination );

  // QgsRemoteCopcPointCloudIndex specific fields
  destination->mHierarchyNodes = mHierarchyNodes;
}

///@endcond
