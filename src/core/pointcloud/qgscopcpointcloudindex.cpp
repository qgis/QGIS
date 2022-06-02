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

#include "qgseptdecoder.h"
#include "qgslazdecoder.h"
#include "qgscoordinatereferencesystem.h"
#include "qgspointcloudrequest.h"
#include "qgspointcloudattribute.h"
#include "qgslogger.h"
#include "qgsfeedback.h"
#include "qgsmessagelog.h"
#include "qgspointcloudexpression.h"

#include "lazperf/lazperf.hpp"
#include "lazperf/readers.hpp"
#include "lazperf/vlr.hpp"

///@cond PRIVATE

#define PROVIDER_KEY QStringLiteral( "copc" )
#define PROVIDER_DESCRIPTION QStringLiteral( "COPC point cloud provider" )

QgsCopcPointCloudIndex::QgsCopcPointCloudIndex() = default;

QgsCopcPointCloudIndex::~QgsCopcPointCloudIndex() = default;

std::unique_ptr<QgsPointCloudIndex> QgsCopcPointCloudIndex::clone() const
{
  QgsCopcPointCloudIndex *clone = new QgsCopcPointCloudIndex;
  QMutexLocker locker( &mHierarchyMutex );
  copyCommonProperties( clone );
  return std::unique_ptr<QgsPointCloudIndex>( clone );
}

void QgsCopcPointCloudIndex::load( const QString &fileName )
{
  mFileName = fileName;
  mCopcFile.open( fileName.toStdString(), std::ios::binary );

  if ( !mCopcFile.is_open() || !mCopcFile.good() )
  {
    QgsMessageLog::logMessage( tr( "Unable to open %1 for reading" ).arg( fileName ) );
    mIsValid = false;
    return;
  }

  mLazInfo.reset( new QgsLazInfo( QgsLazInfo::fromFile( mCopcFile ) ) );
  mIsValid = mLazInfo->isValid();
  if ( mIsValid )
  {
    mIsValid = loadSchema( *mLazInfo.get() );
  }
  if ( !mIsValid )
  {
    QgsMessageLog::logMessage( tr( "Unable to recognize %1 as a LAZ file: \"%2\"" ).arg( fileName, mLazInfo->error() ) );
    return;
  }

  loadHierarchy();
}

bool QgsCopcPointCloudIndex::loadSchema( QgsLazInfo &lazInfo )
{
  QByteArray copcInfoVlrData = lazInfo.vlrData( QStringLiteral( "copc" ), 1 );
  if ( copcInfoVlrData.isEmpty() )
  {
    QgsDebugMsg( QStringLiteral( "Invalid COPC file" ) );
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

  mRootBounds = QgsPointCloudDataBounds(
                  ( xmin - mOffset.x() ) / mScale.x(),
                  ( ymin - mOffset.y() ) / mScale.y(),
                  ( zmin - mOffset.z() ) / mScale.z(),
                  ( xmax - mOffset.x() ) / mScale.x(),
                  ( ymax - mOffset.y() ) / mScale.y(),
                  ( zmax - mOffset.z() ) / mScale.z()
                );

  double calculatedSpan = nodeMapExtent( root() ).width() / mCopcInfoVlr.spacing;
  mSpan = calculatedSpan;

#ifdef QGIS_DEBUG
  double dx = xmax - xmin, dy = ymax - ymin, dz = zmax - zmin;
  QgsDebugMsgLevel( QStringLiteral( "lvl0 node size in CRS units: %1 %2 %3" ).arg( dx ).arg( dy ).arg( dz ), 2 );    // all dims should be the same
  QgsDebugMsgLevel( QStringLiteral( "res at lvl0 %1" ).arg( dx / mSpan ), 2 );
  QgsDebugMsgLevel( QStringLiteral( "res at lvl1 %1" ).arg( dx / mSpan / 2 ), 2 );
  QgsDebugMsgLevel( QStringLiteral( "res at lvl2 %1 with node size %2" ).arg( dx / mSpan / 4 ).arg( dx / 4 ), 2 );
#endif

  return true;
}

QgsPointCloudBlock *QgsCopcPointCloudIndex::nodeData( const IndexedPointCloudNode &n, const QgsPointCloudRequest &request )
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
  std::ifstream file( QgsLazDecoder::toNativePath( mFileName ), std::ios::binary );
  file.seekg( blockOffset );
  file.read( rawBlockData.data(), blockSize );

  return QgsLazDecoder::decompressCopc( rawBlockData, *mLazInfo.get(), pointCount, requestAttributes, filterExpression );
}

QgsPointCloudBlockRequest *QgsCopcPointCloudIndex::asyncNodeData( const IndexedPointCloudNode &n, const QgsPointCloudRequest &request )
{
  Q_UNUSED( n )
  Q_UNUSED( request )
  Q_ASSERT( false );
  return nullptr; // unsupported
}

QgsCoordinateReferenceSystem QgsCopcPointCloudIndex::crs() const
{
  return mLazInfo->crs();
}

qint64 QgsCopcPointCloudIndex::pointCount() const
{
  return mLazInfo->pointCount();
}

bool QgsCopcPointCloudIndex::loadHierarchy()
{
  QMutexLocker locker( &mHierarchyMutex );
  fetchHierarchyPage( mCopcInfoVlr.root_hier_offset, mCopcInfoVlr.root_hier_size );
  return true;
}

bool QgsCopcPointCloudIndex::writeStatistics( QgsPointCloudStatistics &stats )
{
  if ( mLazInfo->version() != qMakePair<uint8_t, uint8_t>( 1, 4 ) )
  {
    // EVLR isn't supported in the first place
    QgsMessageLog::logMessage( tr( "Can't write statistics to \"%1\": laz version != 1.4" ).arg( mFileName ) );
    return false;
  }

  QByteArray statisticsEvlrData = fetchCopcStatisticsEvlrData();
  if ( !statisticsEvlrData.isEmpty() )
  {
    QgsMessageLog::logMessage( tr( "Can't write statistics to \"%1\": file already contains COPC statistics!" ).arg( mFileName ) );
    return false;
  }

  lazperf::evlr_header statsEvlrHeader;
  statsEvlrHeader.user_id = "qgis";
  statsEvlrHeader.record_id = 0;
  statsEvlrHeader.description = "Contains calculated statistics";
  QByteArray statsJson = stats.toStatisticsJson();
  statsEvlrHeader.data_length = statsJson.size();

  // Save the EVLRs to the end of the original file (while erasing the exisitng EVLRs in the file)
  mCopcFile.close();
  std::fstream copcFile;
  copcFile.open( QgsLazDecoder::toNativePath( mFileName ), std::ios_base::binary | std::iostream::in | std::iostream::out );
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
    QgsMessageLog::logMessage( tr( "Couldn't open COPC file \"%1\" to write statistics" ).arg( mFileName ) );
    return false;
  }
  copcFile.close();
  mCopcFile.open( QgsLazDecoder::toNativePath( mFileName ), std::ios::binary );
  return true;
}

QgsPointCloudStatistics QgsCopcPointCloudIndex::readStatistics()
{
  QByteArray statisticsEvlrData = fetchCopcStatisticsEvlrData();

  if ( statisticsEvlrData.isEmpty() )
  {
    return QgsPointCloudStatistics();
  }

  return QgsPointCloudStatistics::fromStatisticsJson( statisticsEvlrData );
}

bool QgsCopcPointCloudIndex::isValid() const
{
  return mIsValid;
}

bool QgsCopcPointCloudIndex::fetchNodeHierarchy( const IndexedPointCloudNode &n ) const
{
  QMutexLocker locker( &mHierarchyMutex );

  QVector<IndexedPointCloudNode> ancestors;
  IndexedPointCloudNode foundRoot = n;
  while ( !mHierarchy.contains( foundRoot ) )
  {
    ancestors.push_front( foundRoot );
    foundRoot = foundRoot.parentNode();
  }
  ancestors.push_front( foundRoot );
  for ( IndexedPointCloudNode n : ancestors )
  {
    auto hierarchyIt = mHierarchy.constFind( n );
    if ( hierarchyIt == mHierarchy.constEnd() )
      return false;
    int nodesCount = *hierarchyIt;
    if ( nodesCount < 0 )
    {
      auto hierarchyNodePos = mHierarchyNodePos.constFind( n );
      fetchHierarchyPage( hierarchyNodePos->first, hierarchyNodePos->second );
    }
  }
  return true;
}

void QgsCopcPointCloudIndex::fetchHierarchyPage( uint64_t offset, uint64_t byteSize ) const
{
  mCopcFile.seekg( offset );
  std::unique_ptr<char []> data( new char[ byteSize ] );
  mCopcFile.read( data.get(), byteSize );

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

  for ( uint64_t i = 0; i < byteSize; i += sizeof( CopcEntry ) )
  {
    CopcEntry *entry = reinterpret_cast<CopcEntry *>( data.get() + i );
    const IndexedPointCloudNode nodeId( entry->key.level, entry->key.x, entry->key.y, entry->key.z );
    mHierarchy[nodeId] = entry->pointCount;
    mHierarchyNodePos.insert( nodeId, QPair<uint64_t, int32_t>( entry->offset, entry->byteSize ) );
  }
}

bool QgsCopcPointCloudIndex::hasNode( const IndexedPointCloudNode &n ) const
{
  fetchNodeHierarchy( n );
  mHierarchyMutex.lock();

  auto it = mHierarchy.constFind( n );
  const bool found = it != mHierarchy.constEnd() && *it  > 0;
  mHierarchyMutex.unlock();
  return found;
}

QList<IndexedPointCloudNode> QgsCopcPointCloudIndex::nodeChildren( const IndexedPointCloudNode &n ) const
{
  fetchNodeHierarchy( n );

  mHierarchyMutex.lock();

  auto hierarchyIt = mHierarchy.constFind( n );
  Q_ASSERT( hierarchyIt != mHierarchy.constEnd() );
  QList<IndexedPointCloudNode> lst;
  lst.reserve( 8 );
  const int d = n.d() + 1;
  const int x = n.x() * 2;
  const int y = n.y() * 2;
  const int z = n.z() * 2;
  mHierarchyMutex.unlock();

  for ( int i = 0; i < 8; ++i )
  {
    int dx = i & 1, dy = !!( i & 2 ), dz = !!( i & 4 );
    const IndexedPointCloudNode n2( d, x + dx, y + dy, z + dz );
    if ( fetchNodeHierarchy( n2 ) && mHierarchy[n] > 0 )
      lst.append( n2 );
  }
  return lst;
}

void QgsCopcPointCloudIndex::copyCommonProperties( QgsCopcPointCloudIndex *destination ) const
{
  QgsPointCloudIndex::copyCommonProperties( destination );

  // QgsCopcPointCloudIndex specific fields
  destination->mIsValid = mIsValid;
  destination->mFileName = mFileName;
  destination->mCopcFile.open( mFileName.toStdString(), std::ios::binary );
  destination->mCopcInfoVlr = mCopcInfoVlr;
  destination->mHierarchyNodePos = mHierarchyNodePos;
  destination->mOriginalMetadata = mOriginalMetadata;
  destination->mLazInfo.reset( new QgsLazInfo( *mLazInfo ) );
}

QByteArray QgsCopcPointCloudIndex::fetchCopcStatisticsEvlrData()
{
  uint64_t offset = mLazInfo->firstEvlrOffset();
  uint32_t evlrCount = mLazInfo->evlrCount();

  QByteArray statisticsEvlrData;

  for ( uint32_t i = 0; i < evlrCount; ++i )
  {
    lazperf::evlr_header header;
    mCopcFile.seekg( offset );
    char buffer[60];
    mCopcFile.read( buffer, 60 );
    header.fill( buffer, 60 );

    // UserID: "qgis", record id: 0
    if ( header.user_id == "qgis" && header.record_id == 0 )
    {
      statisticsEvlrData = QByteArray( header.data_length, Qt::Initialization::Uninitialized );
      mCopcFile.read( statisticsEvlrData.data(), header.data_length );
      break;
    }

    offset += 60 + header.data_length;
  }

  return statisticsEvlrData;
}

///@endcond
