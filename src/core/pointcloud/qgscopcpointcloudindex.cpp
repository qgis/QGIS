/***************************************************************************
                         qgscopcpointcloudindex.cpp
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

#include "qgscopcpointcloudindex.h"

#include <fstream>
#include <QFile>
#include <QtDebug>
#include <QQueue>
#include <QMutexLocker>
#include <QJsonDocument>
#include <QJsonObject>
#include <qnamespace.h>

#include "qgsapplication.h"
#include "qgsbox3d.h"
#include "qgscachedpointcloudblockrequest.h"
#include "qgscopcpointcloudblockrequest.h"
#include "qgseptdecoder.h"
#include "qgslazdecoder.h"
#include "qgscoordinatereferencesystem.h"
#include "qgspointcloudblockrequest.h"
#include "qgspointcloudindex.h"
#include "qgspointcloudrequest.h"
#include "qgspointcloudattribute.h"
#include "qgslogger.h"
#include "qgsmessagelog.h"
#include "qgspointcloudexpression.h"

#include "lazperf/vlr.hpp"
#include "qgssetrequestinitiator_p.h"

///@cond PRIVATE

#define PROVIDER_KEY QStringLiteral( "copc" )
#define PROVIDER_DESCRIPTION QStringLiteral( "COPC point cloud provider" )

QgsCopcPointCloudIndex::QgsCopcPointCloudIndex() = default;

QgsCopcPointCloudIndex::~QgsCopcPointCloudIndex() = default;

std::unique_ptr<QgsAbstractPointCloudIndex> QgsCopcPointCloudIndex::clone() const
{
  QgsCopcPointCloudIndex *clone = new QgsCopcPointCloudIndex;
  QMutexLocker locker( &mHierarchyMutex );
  copyCommonProperties( clone );
  return std::unique_ptr<QgsAbstractPointCloudIndex>( clone );
}

void QgsCopcPointCloudIndex::load( const QString &urlString )
{
  QUrl url = urlString;
  // Treat non-URLs as local files
  if ( url.isValid() && ( url.scheme() == "http" || url.scheme() == "https" ) )
    mAccessType = Qgis::PointCloudAccessType::Remote;
  else
  {
    mAccessType = Qgis::PointCloudAccessType::Local;
    mCopcFile.open( QgsLazDecoder::toNativePath( urlString ), std::ios::binary );
    if ( mCopcFile.fail() )
    {
      mError = QObject::tr( "Unable to open %1 for reading" ).arg( urlString );
      mIsValid = false;
      return;
    }
  }
  mUri = urlString;

  if ( mAccessType == Qgis::PointCloudAccessType::Remote )
    mLazInfo.reset( new QgsLazInfo( QgsLazInfo::fromUrl( url ) ) );
  else
    mLazInfo.reset( new QgsLazInfo( QgsLazInfo::fromFile( mCopcFile ) ) );
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
    mError = QObject::tr( "Unable to recognize %1 as a LAZ file: \"%2\"" ).arg( urlString, mLazInfo->error() );
  }
}

bool QgsCopcPointCloudIndex::loadSchema( QgsLazInfo &lazInfo )
{
  QByteArray copcInfoVlrData = lazInfo.vlrData( QStringLiteral( "copc" ), 1 );
  if ( copcInfoVlrData.isEmpty() )
  {
    mError = QObject::tr( "Invalid COPC file" );
    return false;
  }
  mCopcInfoVlr.fill( copcInfoVlrData.data(), copcInfoVlrData.size() );

  mScale = lazInfo.scale();
  mOffset = lazInfo.offset();

  mOriginalMetadata = lazInfo.toMetadata();

  QgsVector3D minCoords = lazInfo.minCoords();
  QgsVector3D maxCoords = lazInfo.maxCoords();
  mExtent.set( minCoords.x(), minCoords.y(), maxCoords.x(), maxCoords.y() );
  mZMin = minCoords.z();
  mZMax = maxCoords.z();

  setAttributes( lazInfo.attributes() );

  const double xmin = mCopcInfoVlr.center_x - mCopcInfoVlr.halfsize;
  const double ymin = mCopcInfoVlr.center_y - mCopcInfoVlr.halfsize;
  const double zmin = mCopcInfoVlr.center_z - mCopcInfoVlr.halfsize;
  const double xmax = mCopcInfoVlr.center_x + mCopcInfoVlr.halfsize;
  const double ymax = mCopcInfoVlr.center_y + mCopcInfoVlr.halfsize;
  const double zmax = mCopcInfoVlr.center_z + mCopcInfoVlr.halfsize;

  mRootBounds = QgsBox3D( xmin, ymin, zmin, xmax, ymax, zmax );

  // TODO: Rounding?
  mSpan = mRootBounds.width() / mCopcInfoVlr.spacing;

#ifdef QGIS_DEBUG
  double dx = xmax - xmin, dy = ymax - ymin, dz = zmax - zmin;
  QgsDebugMsgLevel( QStringLiteral( "lvl0 node size in CRS units: %1 %2 %3" ).arg( dx ).arg( dy ).arg( dz ), 2 );    // all dims should be the same
  QgsDebugMsgLevel( QStringLiteral( "res at lvl0 %1" ).arg( dx / mSpan ), 2 );
  QgsDebugMsgLevel( QStringLiteral( "res at lvl1 %1" ).arg( dx / mSpan / 2 ), 2 );
  QgsDebugMsgLevel( QStringLiteral( "res at lvl2 %1 with node size %2" ).arg( dx / mSpan / 4 ).arg( dx / 4 ), 2 );
#endif

  return true;
}

std::unique_ptr<QgsPointCloudBlock> QgsCopcPointCloudIndex::nodeData( const QgsPointCloudNodeId &n, const QgsPointCloudRequest &request )
{
  if ( QgsPointCloudBlock *cached = getNodeDataFromCache( n, request ) )
  {
    return std::unique_ptr<QgsPointCloudBlock>( cached );
  }

  std::unique_ptr<QgsPointCloudBlock> block;
  if ( mAccessType == Qgis::PointCloudAccessType::Local )
  {
    const bool found = fetchNodeHierarchy( n );
    if ( !found )
      return nullptr;
    mHierarchyMutex.lock();
    int pointCount = mHierarchy.value( n );
    auto [blockOffset, blockSize] = mHierarchyNodePos.value( n );
    mHierarchyMutex.unlock();

    // we need to create a copy of the expression to pass to the decoder
    // as the same QgsPointCloudExpression object mighgt be concurrently
    // used on another thread, for example in a 3d view
    QgsPointCloudExpression filterExpression = mFilterExpression;
    QgsPointCloudAttributeCollection requestAttributes = request.attributes();
    requestAttributes.extend( attributes(), filterExpression.referencedAttributes() );

    QByteArray rawBlockData( blockSize, Qt::Initialization::Uninitialized );
    std::ifstream file( QgsLazDecoder::toNativePath( mUri ), std::ios::binary );
    file.seekg( blockOffset );
    file.read( rawBlockData.data(), blockSize );
    if ( !file )
    {
      QgsDebugError( QStringLiteral( "Could not read file %1" ).arg( mUri ) );
      return nullptr;
    }
    QgsRectangle filterRect = request.filterRect();

    block = QgsLazDecoder::decompressCopc( rawBlockData, *mLazInfo.get(), pointCount, requestAttributes, filterExpression, filterRect );
  }
  else
  {

    std::unique_ptr<QgsPointCloudBlockRequest> blockRequest( asyncNodeData( n, request ) );
    if ( !blockRequest )
      return nullptr;

    QEventLoop loop;
    QObject::connect( blockRequest.get(), &QgsPointCloudBlockRequest::finished, &loop, &QEventLoop::quit );
    loop.exec();

    block = blockRequest->takeBlock();

    if ( !block )
      QgsDebugError( QStringLiteral( "Error downloading node %1 data, error : %2 " ).arg( n.toString(), blockRequest->errorStr() ) );
  }

  storeNodeDataToCache( block.get(), n, request );
  return block;
}

QgsPointCloudBlockRequest *QgsCopcPointCloudIndex::asyncNodeData( const QgsPointCloudNodeId &n, const QgsPointCloudRequest &request )
{
  if ( mAccessType == Qgis::PointCloudAccessType::Local )
    return nullptr; // TODO
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

QgsCoordinateReferenceSystem QgsCopcPointCloudIndex::crs() const
{
  return mLazInfo->crs();
}

qint64 QgsCopcPointCloudIndex::pointCount() const
{
  return mLazInfo->pointCount();
}

bool QgsCopcPointCloudIndex::loadHierarchy() const
{
  fetchHierarchyPage( mCopcInfoVlr.root_hier_offset, mCopcInfoVlr.root_hier_size );
  return true;
}

bool QgsCopcPointCloudIndex::writeStatistics( QgsPointCloudStatistics &stats )
{
  if ( mAccessType == Qgis::PointCloudAccessType::Remote )
  {
    QgsMessageLog::logMessage( QObject::tr( "Can't write statistics to remote file \"%1\"" ).arg( mUri ) );
    return false;
  }

  if ( mLazInfo->version() != qMakePair<uint8_t, uint8_t>( 1, 4 ) )
  {
    // EVLR isn't supported in the first place
    QgsMessageLog::logMessage( QObject::tr( "Can't write statistics to \"%1\": laz version != 1.4" ).arg( mUri ) );
    return false;
  }

  QByteArray statisticsEvlrData = fetchCopcStatisticsEvlrData();
  if ( !statisticsEvlrData.isEmpty() )
  {
    QgsMessageLog::logMessage( QObject::tr( "Can't write statistics to \"%1\": file already contains COPC statistics!" ).arg( mUri ) );
    return false;
  }

  lazperf::evlr_header statsEvlrHeader;
  statsEvlrHeader.user_id = "qgis";
  statsEvlrHeader.record_id = 0;
  statsEvlrHeader.description = "Contains calculated statistics";
  QByteArray statsJson = stats.toStatisticsJson();
  statsEvlrHeader.data_length = statsJson.size();

  // Save the EVLRs to the end of the original file (while erasing the existing EVLRs in the file)
  mCopcFile.close();
  std::fstream copcFile;
  copcFile.open( QgsLazDecoder::toNativePath( mUri ), std::ios_base::binary | std::iostream::in | std::iostream::out );
  if ( copcFile.is_open() && copcFile.good() )
  {
    // Write the new number of EVLRs
    lazperf::header14 header = mLazInfo->header();
    header.evlr_count = header.evlr_count + 1;
    copcFile.seekp( 0 );
    header.write( copcFile );

    // Append EVLR data to the end
    copcFile.seekg( 0, std::ios::end );

    statsEvlrHeader.write( copcFile );
    copcFile.write( statsJson.data(), statsEvlrHeader.data_length );
  }
  else
  {
    QgsMessageLog::logMessage( QObject::tr( "Couldn't open COPC file \"%1\" to write statistics" ).arg( mUri ) );
    return false;
  }
  copcFile.close();
  mCopcFile.open( QgsLazDecoder::toNativePath( mUri ), std::ios::binary );
  return true;
}

QgsPointCloudStatistics QgsCopcPointCloudIndex::metadataStatistics() const
{
  if ( ! mStatistics )
  {
    const QByteArray statisticsEvlrData = fetchCopcStatisticsEvlrData();
    if ( statisticsEvlrData.isEmpty() )
      mStatistics = QgsAbstractPointCloudIndex::metadataStatistics();
    else
      mStatistics = QgsPointCloudStatistics::fromStatisticsJson( statisticsEvlrData );
  }

  return *mStatistics;
}

bool QgsCopcPointCloudIndex::isValid() const
{
  return mIsValid;
}

bool QgsCopcPointCloudIndex::fetchNodeHierarchy( const QgsPointCloudNodeId &n ) const
{
  QMutexLocker locker( &mHierarchyMutex );

  QVector<QgsPointCloudNodeId> ancestors;
  QgsPointCloudNodeId foundRoot = n;
  while ( !mHierarchy.contains( foundRoot ) )
  {
    ancestors.push_front( foundRoot );
    foundRoot = foundRoot.parentNode();
  }
  ancestors.push_front( foundRoot );
  for ( QgsPointCloudNodeId n : ancestors )
  {
    auto hierarchyIt = mHierarchy.constFind( n );
    if ( hierarchyIt == mHierarchy.constEnd() )
      return false;
    int nodesCount = *hierarchyIt;
    if ( nodesCount < 0 )
    {
      auto hierarchyNodePos = mHierarchyNodePos.constFind( n );
      mHierarchyMutex.unlock();
      fetchHierarchyPage( hierarchyNodePos->first, hierarchyNodePos->second );
      mHierarchyMutex.lock();
    }
  }
  return mHierarchy.contains( n );
}

void QgsCopcPointCloudIndex::fetchHierarchyPage( uint64_t offset, uint64_t byteSize ) const
{
  Q_ASSERT( byteSize > 0 );

  QByteArray data = readRange( offset, byteSize );
  if ( data.isEmpty() )
    return;

  populateHierarchy( data.constData(), byteSize );
}

void QgsCopcPointCloudIndex::populateHierarchy( const char *hierarchyPageData, uint64_t byteSize ) const
{
  struct CopcVoxelKey
  {
    int32_t level;
    int32_t x;
    int32_t y;
    int32_t z;
  };

  struct CopcEntry
  {
    CopcVoxelKey key;
    uint64_t offset;
    int32_t byteSize;
    int32_t pointCount;
  };

  QMutexLocker locker( &mHierarchyMutex );

  for ( uint64_t i = 0; i < byteSize; i += sizeof( CopcEntry ) )
  {
    const CopcEntry *entry = reinterpret_cast<const CopcEntry *>( hierarchyPageData + i );
    const QgsPointCloudNodeId nodeId( entry->key.level, entry->key.x, entry->key.y, entry->key.z );
    mHierarchy[nodeId] = entry->pointCount;
    mHierarchyNodePos.insert( nodeId, QPair<uint64_t, int32_t>( entry->offset, entry->byteSize ) );
  }
}

bool QgsCopcPointCloudIndex::hasNode( const QgsPointCloudNodeId &n ) const
{
  return fetchNodeHierarchy( n );
}

QgsPointCloudNode QgsCopcPointCloudIndex::getNode( const QgsPointCloudNodeId &id ) const
{
  bool nodeFound = fetchNodeHierarchy( id );
  Q_ASSERT( nodeFound );

  qint64 pointCount;
  {
    QMutexLocker locker( &mHierarchyMutex );
    pointCount = mHierarchy.value( id, -1 );
  }

  QList<QgsPointCloudNodeId> children;
  children.reserve( 8 );
  const int d = id.d() + 1;
  const int x = id.x() * 2;
  const int y = id.y() * 2;
  const int z = id.z() * 2;

  for ( int i = 0; i < 8; ++i )
  {
    int dx = i & 1, dy = !!( i & 2 ), dz = !!( i & 4 );
    const QgsPointCloudNodeId n2( d, x + dx, y + dy, z + dz );
    bool found = fetchNodeHierarchy( n2 );
    {
      QMutexLocker locker( &mHierarchyMutex );
      if ( found && mHierarchy[id] >= 0 )
        children.append( n2 );
    }
  }

  QgsBox3D bounds = QgsPointCloudNode::bounds( mRootBounds, id );
  return QgsPointCloudNode( id, pointCount, children, bounds.width() / mSpan, bounds );
}

void QgsCopcPointCloudIndex::copyCommonProperties( QgsCopcPointCloudIndex *destination ) const
{
  QgsAbstractPointCloudIndex::copyCommonProperties( destination );

  // QgsCopcPointCloudIndex specific fields
  destination->mIsValid = mIsValid;
  destination->mAccessType = mAccessType;
  destination->mUri = mUri;
  if ( mAccessType == Qgis::PointCloudAccessType::Local )
    destination->mCopcFile.open( QgsLazDecoder::toNativePath( mUri ), std::ios::binary );
  destination->mCopcInfoVlr = mCopcInfoVlr;
  destination->mHierarchyNodePos = mHierarchyNodePos;
  destination->mOriginalMetadata = mOriginalMetadata;
  destination->mLazInfo.reset( new QgsLazInfo( *mLazInfo ) );
}

QByteArray QgsCopcPointCloudIndex::readRange( uint64_t offset, uint64_t length ) const
{
  if ( mAccessType == Qgis::PointCloudAccessType::Local )
  {
    QByteArray buffer( length, Qt::Initialization::Uninitialized );
    mCopcFile.seekg( offset );
    mCopcFile.read( buffer.data(), length );
    if ( mCopcFile.eof() )
      QgsDebugError( QStringLiteral( "Read past end of file (path %1 offset %2 length %3)" ).arg( mUri ).arg( offset ).arg( length ) );
    if ( !mCopcFile )
      QgsDebugError( QStringLiteral( "Error reading %1" ).arg( mUri ) );
    return buffer;
  }
  else
  {
    QNetworkRequest nr = QNetworkRequest( QUrl( mUri ) );
    QgsSetRequestInitiatorClass( nr, QStringLiteral( "QgsCopcPointCloudIndex" ) );
    nr.setAttribute( QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::PreferCache );
    nr.setAttribute( QNetworkRequest::CacheSaveControlAttribute, true );
    QByteArray queryRange = QStringLiteral( "bytes=%1-%2" ).arg( offset ).arg( offset + length - 1 ).toLocal8Bit();
    nr.setRawHeader( "Range", queryRange );

    std::unique_ptr<QgsTileDownloadManagerReply> reply( QgsApplication::tileDownloadManager()->get( nr ) );

    QEventLoop loop;
    QObject::connect( reply.get(), &QgsTileDownloadManagerReply::finished, &loop, &QEventLoop::quit );
    loop.exec();

    if ( reply->error() != QNetworkReply::NoError )
    {
      QgsDebugError( QStringLiteral( "Request failed: %1 (offset %1 length %2)" ).arg( mUri ).arg( offset ).arg( length ) );
      return {};
    }

    return reply->data();
  }
}

QByteArray QgsCopcPointCloudIndex::fetchCopcStatisticsEvlrData() const
{
  uint64_t offset = mLazInfo->firstEvlrOffset();
  uint32_t evlrCount = mLazInfo->evlrCount();

  QByteArray statisticsEvlrData;

  for ( uint32_t i = 0; i < evlrCount; ++i )
  {
    lazperf::evlr_header header;

    QByteArray buffer = readRange( offset, 60 );
    header.fill( buffer.data(), buffer.size() );

    if ( header.user_id == "qgis" && header.record_id == 0 )
    {
      statisticsEvlrData = readRange( offset + 60, header.data_length );
      break;
    }

    offset += 60 + header.data_length;
  }

  return statisticsEvlrData;
}

void QgsCopcPointCloudIndex::reset()
{
  // QgsAbstractPointCloudIndex
  mExtent = QgsRectangle();
  mZMin = 0;
  mZMax = 0;
  mHierarchy.clear();
  mScale = QgsVector3D();
  mOffset = QgsVector3D();
  mRootBounds = QgsBox3D();
  mAttributes = QgsPointCloudAttributeCollection();
  mSpan = 0;
  mError.clear();

  // QgsCopcPointCloudIndex
  mIsValid = false;
  mAccessType = Qgis::PointCloudAccessType::Local;
  mCopcFile.close();
  mOriginalMetadata.clear();
  mStatistics.reset();
  mLazInfo.reset();
  mHierarchyNodePos.clear();
}

QVariantMap QgsCopcPointCloudIndex::extraMetadata() const
{
  return
  {
    { QStringLiteral( "CopcGpsTimeFlag" ), mLazInfo.get()->header().global_encoding & 1 },
  };
}

///@endcond
