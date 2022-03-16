/***************************************************************************
                         qgsremoteeptpointcloudindex.cpp
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

#include "qgsremoteeptpointcloudindex.h"

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

#include "qgseptdecoder.h"
#include "qgscoordinatereferencesystem.h"
#include "qgspointcloudrequest.h"
#include "qgspointcloudattribute.h"
#include "qgslogger.h"
#include "qgsfeedback.h"
#include "qgsmessagelog.h"

#include "qgstiledownloadmanager.h"
#include "qgsblockingnetworkrequest.h"

#include "qgsfileutils.h"
#include "qgsapplication.h"
#include "qgspointcloudblockrequest.h"
#include "qgspointcloudexpression.h"

///@cond PRIVATE

QgsRemoteEptPointCloudIndex::QgsRemoteEptPointCloudIndex() : QgsEptPointCloudIndex()
{
  mHierarchyNodes.insert( IndexedPointCloudNode( 0, 0, 0, 0 ) );
}

QgsRemoteEptPointCloudIndex::~QgsRemoteEptPointCloudIndex() = default;

QList<IndexedPointCloudNode> QgsRemoteEptPointCloudIndex::nodeChildren( const IndexedPointCloudNode &n ) const
{
  QList<IndexedPointCloudNode> lst;
  if ( !loadNodeHierarchy( n ) )
    return lst;

  const int d = n.d() + 1;
  const int x = n.x() * 2;
  const int y = n.y() * 2;
  const int z = n.z() * 2;

  for ( int i = 0; i < 8; ++i )
  {
    int dx = i & 1, dy = !!( i & 2 ), dz = !!( i & 4 );
    const IndexedPointCloudNode n2( d, x + dx, y + dy, z + dz );
    if ( loadNodeHierarchy( n2 ) )
      lst.append( n2 );
  }
  return lst;
}

void QgsRemoteEptPointCloudIndex::load( const QString &url )
{
  mUrl = QUrl( url );

  QStringList splitUrl = url.split( '/' );

  mUrlFileNamePart = splitUrl.back();
  splitUrl.pop_back();
  mUrlDirectoryPart = splitUrl.join( '/' );

  QNetworkRequest nr( url );

  QgsBlockingNetworkRequest req;
  const QgsBlockingNetworkRequest::ErrorCode errCode = req.get( nr );
  if ( errCode != QgsBlockingNetworkRequest::NoError )
  {
    QgsDebugMsg( QStringLiteral( "Request failed: " ) + url );
    mIsValid = false;
    return;
  }

  const QgsNetworkReplyContent reply = req.reply();
  mIsValid = loadSchema( reply.content() );
}

QgsPointCloudBlock *QgsRemoteEptPointCloudIndex::nodeData( const IndexedPointCloudNode &n, const QgsPointCloudRequest &request )
{
  std::unique_ptr<QgsPointCloudBlockRequest> blockRequest( asyncNodeData( n, request ) );
  if ( !blockRequest )
    return nullptr;

  QEventLoop loop;
  connect( blockRequest.get(), &QgsPointCloudBlockRequest::finished, &loop, &QEventLoop::quit );
  loop.exec();

  if ( !blockRequest->block() )
  {
    QgsDebugMsg( QStringLiteral( "Error downloading node %1 data, error : %2 " ).arg( n.toString(), blockRequest->errorStr() ) );
  }

  return blockRequest->block();
}

QgsPointCloudBlockRequest *QgsRemoteEptPointCloudIndex::asyncNodeData( const IndexedPointCloudNode &n, const QgsPointCloudRequest &request )
{
  if ( !loadNodeHierarchy( n ) )
    return nullptr;

  QString fileUrl;
  if ( mDataType == QLatin1String( "binary" ) )
  {
    fileUrl = QStringLiteral( "%1/ept-data/%2.bin" ).arg( mUrlDirectoryPart, n.toString() );
  }
  else if ( mDataType == QLatin1String( "zstandard" ) )
  {
    fileUrl = QStringLiteral( "%1/ept-data/%2.zst" ).arg( mUrlDirectoryPart, n.toString() );
  }
  else if ( mDataType == QLatin1String( "laszip" ) )
  {
    fileUrl = QStringLiteral( "%1/ept-data/%2.laz" ).arg( mUrlDirectoryPart, n.toString() );
  }
  else
  {
    return nullptr;
  }

  // we need to create a copy of the expression to pass to the decoder
  // as the same QgsPointCloudExpression object might be concurrently
  // used on another thread, for example in a 3d view
  QgsPointCloudExpression filterExpression = mFilterExpression;
  QgsPointCloudAttributeCollection requestAttributes = request.attributes();
  requestAttributes.extend( attributes(), filterExpression.referencedAttributes() );
  return new QgsPointCloudBlockRequest( n, fileUrl, mDataType, attributes(), requestAttributes, scale(), offset(), filterExpression );
}

bool QgsRemoteEptPointCloudIndex::hasNode( const IndexedPointCloudNode &n ) const
{
  return loadNodeHierarchy( n );
}

bool QgsRemoteEptPointCloudIndex::loadNodeHierarchy( const IndexedPointCloudNode &nodeId ) const
{
  mHierarchyMutex.lock();
  bool found = mHierarchy.contains( nodeId );
  mHierarchyMutex.unlock();
  if ( found )
    return true;

  QVector<IndexedPointCloudNode> nodePathToRoot;
  {
    IndexedPointCloudNode currentNode = nodeId;
    do
    {
      nodePathToRoot.push_back( currentNode );
      currentNode = currentNode.parentNode();
    }
    while ( currentNode.d() >= 0 );
  }

  for ( int i = nodePathToRoot.size() - 1; i >= 0 && !mHierarchy.contains( nodeId ); --i )
  {
    const IndexedPointCloudNode node = nodePathToRoot[i];
    //! The hierarchy of the node is found => No need to load its file
    mHierarchyMutex.lock();
    const bool foundInHierarchy = mHierarchy.contains( node );
    const bool foundInHierarchyNodes = mHierarchyNodes.contains( node );
    mHierarchyMutex.unlock();
    if ( foundInHierarchy )
      continue;

    if ( !foundInHierarchyNodes )
      continue;

    const QString fileUrl = QStringLiteral( "%1/ept-hierarchy/%2.json" ).arg( mUrlDirectoryPart, node.toString() );
    QNetworkRequest nr( fileUrl );

    nr.setAttribute( QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::PreferCache );
    nr.setAttribute( QNetworkRequest::CacheSaveControlAttribute, true );

    QgsBlockingNetworkRequest req;
    const QgsBlockingNetworkRequest::ErrorCode errCode = req.get( nr );
    if ( errCode != QgsBlockingNetworkRequest::NoError )
    {
      QgsDebugMsgLevel( QStringLiteral( "unable to read hierarchy from file %1" ).arg( fileUrl ), 2 );
      return false;
    }

    const QgsNetworkReplyContent reply = req.reply();

    const QByteArray dataJsonH = reply.content();
    QJsonParseError errH;
    const QJsonDocument docH = QJsonDocument::fromJson( dataJsonH, &errH );
    if ( errH.error != QJsonParseError::NoError )
    {
      QgsDebugMsgLevel( QStringLiteral( "QJsonParseError when reading hierarchy from file %1" ).arg( fileUrl ), 2 );
      return false;
    }

    const QJsonObject rootHObj = docH.object();
    for ( auto it = rootHObj.constBegin(); it != rootHObj.constEnd(); ++it )
    {
      const QString nodeIdStr = it.key();
      const int nodePointCount = it.value().toInt();
      const IndexedPointCloudNode nodeId = IndexedPointCloudNode::fromString( nodeIdStr );
      mHierarchyMutex.lock();
      if ( nodePointCount > 0 )
        mHierarchy[nodeId] = nodePointCount;
      else if ( nodePointCount == -1 )
        mHierarchyNodes.insert( nodeId );
      mHierarchyMutex.unlock();
    }
  }

  mHierarchyMutex.lock();
  found = mHierarchy.contains( nodeId );
  mHierarchyMutex.unlock();

  return found;
}

bool QgsRemoteEptPointCloudIndex::isValid() const
{
  return mIsValid;
}

///@endcond
