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
#include <QFile>
#include <QtDebug>
#include <QQueue>
#include <QMutexLocker>

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

///@cond PRIVATE

#define PROVIDER_KEY QStringLiteral( "copc" )
#define PROVIDER_DESCRIPTION QStringLiteral( "COPC point cloud provider" )

QgsCopcPointCloudIndex::QgsCopcPointCloudIndex() = default;

QgsCopcPointCloudIndex::~QgsCopcPointCloudIndex() = default;

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
    QgsMessageLog::logMessage( tr( "Unable to recognize %1 as a LAZ file: \"%2\"" ).arg( fileName ).arg( mLazInfo->error() ) );
    return;
  }

  loadHierarchy();
}

bool QgsCopcPointCloudIndex::loadSchema( QgsLazInfo &lazInfo )
{
  QByteArray copcInfoVlrData = lazInfo.vlrData( "copc", 1 );
  if ( copcInfoVlrData.isEmpty() )
  {
    QgsDebugMsg( "Invalid COPC file" );
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
  int pointCount = mHierarchy[n];
  auto [blockOffset, blockSize] = mHierarchyNodePos[n];
  mHierarchyMutex.unlock();

  // we need to create a copy of the expression to pass to the decoder
  // as the same QgsPointCloudExpression object mighgt be concurrently
  // used on another thread, for example in a 3d view
  QgsPointCloudExpression filterExpression = mFilterExpression;
  QgsPointCloudAttributeCollection requestAttributes = request.attributes();
  requestAttributes.extend( attributes(), filterExpression.referencedAttributes() );

  QByteArray rawBlockData( blockSize, Qt::Initialization::Uninitialized );
  std::ifstream file( mFileName.toStdString(), std::ios::binary );
  file.seekg( blockOffset );
  file.read( rawBlockData.data(), blockSize );

  return QgsLazDecoder::decompressCopc( rawBlockData, *mLazInfo.get(), pointCount, requestAttributes, filterExpression );
}

QgsPointCloudBlockRequest *QgsCopcPointCloudIndex::asyncNodeData( const IndexedPointCloudNode &n, const QgsPointCloudRequest &request )
{
  Q_UNUSED( n );
  Q_UNUSED( request );
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
    if ( !mHierarchy.contains( n ) )
      return false;
    int nodesCount = mHierarchy[n];
    if ( nodesCount < 0 )
    {
      fetchHierarchyPage( mHierarchyNodePos[n].first, mHierarchyNodePos[n].second );
    }
  }
  return true;
}

void QgsCopcPointCloudIndex::fetchHierarchyPage( uint64_t offset, uint64_t byteSize ) const
{
  mCopcFile.seekg( offset );
  std::unique_ptr<char> data( new char[ byteSize ] );
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
  const bool found = mHierarchy.contains( n ) && mHierarchy[n] > 0;
  mHierarchyMutex.unlock();
  return found;
}

QList<IndexedPointCloudNode> QgsCopcPointCloudIndex::nodeChildren( const IndexedPointCloudNode &n ) const
{
  fetchNodeHierarchy( n );

  mHierarchyMutex.lock();
  Q_ASSERT( mHierarchy.contains( n ) );
  QList<IndexedPointCloudNode> lst;
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

///@endcond
