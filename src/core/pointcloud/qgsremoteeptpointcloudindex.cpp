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
#include "qgsfiledownloader.h"

#include "qgsfileutils.h"
#include "qgsapplication.h"
#include "qgspointcloudblockhandle.h"

///@cond PRIVATE

QgsRemoteEptPointCloudIndex::QgsRemoteEptPointCloudIndex() : QgsEptPointCloudIndex()
{
  mTileDownloadManager = QgsApplication::tileDownloadManager();
}

QgsRemoteEptPointCloudIndex::~QgsRemoteEptPointCloudIndex() = default;

QList<IndexedPointCloudNode> QgsRemoteEptPointCloudIndex::nodeChildren( const IndexedPointCloudNode &n ) const
{
  // Potential bug: what if loadNodeHierarchy fails and returns false
  Q_ASSERT( loadNodeHierarchy( n ) );

  QList<IndexedPointCloudNode> lst;
  int d = n.d() + 1;
  int x = n.x() * 2;
  int y = n.y() * 2;
  int z = n.z() * 2;

  for ( int i = 0; i < 8; ++i )
  {
    int dx = i & 1, dy = !!( i & 2 ), dz = !!( i & 4 );
    IndexedPointCloudNode n2( d, x + dx, y + dy, z + dz );
    if ( mHierarchy.contains( n2 ) )
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
  QgsBlockingNetworkRequest::ErrorCode errCode = req.get( nr );
  if ( errCode != QgsBlockingNetworkRequest::NoError )
  {
    QgsDebugMsg( QStringLiteral( "Request failed: " ) + url );
    mIsValid = false;
    return;
  }

  QgsNetworkReplyContent reply = req.reply();
  bool success = loadSchema( reply.content() );

  if ( success )
  {
    // try to import the metadata too!
    QNetworkRequest nr( QStringLiteral( "%1/ept-sources/manifest.json" ).arg( mUrlDirectoryPart ) );

    QgsBlockingNetworkRequest req;
    QgsBlockingNetworkRequest::ErrorCode errCode = req.get( nr );
    if ( errCode == QgsBlockingNetworkRequest::NoError )
    {
      QgsNetworkReplyContent reply = req.reply();
      const QByteArray manifestJson = reply.content();
      loadManifest( manifestJson );
    }
  }

  mIsValid = success;
}

QgsPointCloudBlock *QgsRemoteEptPointCloudIndex::nodeData( const IndexedPointCloudNode &n, const QgsPointCloudRequest &request )
{
  if ( !loadNodeHierarchy( n ) )
    return nullptr;

  QString fileUrl;
  QString fileName;
  if ( mDataType == QLatin1String( "binary" ) )
  {

    fileUrl = QStringLiteral( "%1/ept-data/%2.bin" ).arg( mUrlDirectoryPart, n.toString() );
    fileName = QStringLiteral( "/tmp/%1.bin" ).arg( n.toString() );
  }
  else if ( mDataType == QLatin1String( "zstandard" ) )
  {
    fileUrl = QStringLiteral( "%1/ept-data/%2.zst" ).arg( mUrlDirectoryPart, n.toString() );
    fileName = QStringLiteral( "/tmp/%1.zst" ).arg( n.toString() );
  }
  else if ( mDataType == QLatin1String( "laszip" ) )
  {
    fileUrl = QStringLiteral( "%1/ept-data/%2.laz" ).arg( mUrlDirectoryPart, n.toString() );
    fileName = QStringLiteral( "/tmp/%1.laz" ).arg( n.toString() );
  }
  else
  {
    return nullptr;  // unsupported
  }

  QgsFileDownloader downloader( fileUrl, fileName );

  int timeout = 10000;
  QTimer timer;
  timer.setSingleShot( true );
  QEventLoop loop;

  QUrl downloadedUrl;
  QStringList errorMessages;
  connect( &downloader, &QgsFileDownloader::downloadCompleted, [&]( const QUrl & url )
  {
    downloadedUrl = url;
    loop.quit();
  } );
  connect( &downloader, &QgsFileDownloader::downloadCanceled, &loop, &QEventLoop::quit );
  connect( &downloader, &QgsFileDownloader::downloadError, [&]( QStringList errorMessagesList )
  {
    errorMessages = errorMessagesList;
    loop.exit();
  } );

  connect( &timer, &QTimer::timeout, &loop, &QEventLoop::quit );
  downloader.startDownload();
  timer.start( timeout );
  loop.exec();

  if ( downloadedUrl.isEmpty() )
  {
    qDebug() << "Error downloading " << fileUrl << " : " << errorMessages;
    return nullptr;
  }
  qDebug() << "File downloaded";

  if ( mDataType == QLatin1String( "binary" ) )
  {
    return QgsEptDecoder::decompressBinary( fileName, attributes(), request.attributes() );
  }
  else if ( mDataType == QLatin1String( "zstandard" ) )
  {
    return QgsEptDecoder::decompressZStandard( fileName, attributes(), request.attributes() );
  }
  else if ( mDataType == QLatin1String( "laszip" ) )
  {
    return QgsEptDecoder::decompressLaz( fileName, attributes(), request.attributes() );
  }
  return nullptr;
}

QgsPointCloudBlockHandle *QgsRemoteEptPointCloudIndex::asyncNodeData( const IndexedPointCloudNode &n, const QgsPointCloudRequest &request )
{
  if ( !loadNodeHierarchy( n ) )
    return nullptr;

  QgsPointCloudBlockHandle *handle = nullptr;

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
  QNetworkRequest nr( fileUrl );
  QgsTileDownloadManagerReply *reply = mTileDownloadManager->get( nr );
  handle = new QgsPointCloudBlockHandle( mDataType, attributes(), request.attributes(), reply );
  return handle;
}

int QgsRemoteEptPointCloudIndex::pointCount() const
{
  return mPointCount;
}

bool QgsRemoteEptPointCloudIndex::loadNodeHierarchy( const IndexedPointCloudNode &nodeId ) const
{
  if ( mHierarchy.contains( nodeId ) )
    return true;
  QVector<IndexedPointCloudNode> nodePathToRoot;
  {
    nodePathToRoot.push_back( nodeId );
    IndexedPointCloudNode currentNode = nodeId;
    while ( currentNode.d() != 0 )
    {
      currentNode = currentNode.parentNode();
      nodePathToRoot.push_back( currentNode );
    }
  }

  for ( int i = nodePathToRoot.size() - 1; i >= 0 && mHierarchy.find( nodeId ) == mHierarchy.end(); --i )
  {
    IndexedPointCloudNode node = nodePathToRoot[i];
    //! The hierarchy of the node is found => No need to load its file
    if ( mHierarchy.find( node ) != mHierarchy.end() )
      continue;

    const QString fileUrl = QStringLiteral( "%1/ept-hierarchy/%2.json" ).arg( mUrlDirectoryPart, node.toString() );
    QNetworkRequest nr( fileUrl );

    QgsBlockingNetworkRequest req;
    QgsBlockingNetworkRequest::ErrorCode errCode = req.get( nr );
    if ( errCode != QgsBlockingNetworkRequest::NoError )
    {
      QgsDebugMsgLevel( QStringLiteral( "unable to read hierarchy from file %1" ).arg( fileUrl ), 2 );
      return false;
    }

    QgsNetworkReplyContent reply = req.reply();

    QByteArray dataJsonH = reply.content();
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
      QString nodeIdStr = it.key();
      int nodePointCount = it.value().toInt();
      if ( nodePointCount > 0 )
      {
        IndexedPointCloudNode nodeId = IndexedPointCloudNode::fromString( nodeIdStr );
        mHierarchy[nodeId] = nodePointCount;
      }
    }
  }

  return mHierarchy.contains( nodeId );
}

bool QgsRemoteEptPointCloudIndex::isValid() const
{
  return mIsValid;
}

///@endcond
