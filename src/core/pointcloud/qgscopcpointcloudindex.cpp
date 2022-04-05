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

  mLazFile.reset( new lazperf::reader::generic_file( mCopcFile ) );

  bool success = loadSchema();

  if ( success )
  {
    success = loadHierarchy();
  }

  mIsValid = success;
}

bool QgsCopcPointCloudIndex::loadSchema()
{
  mPointCount = mLazFile->header().point_count;

  mScale = QgsVector3D( mLazFile->header().scale.x, mLazFile->header().scale.y, mLazFile->header().scale.z );
  mOffset = QgsVector3D( mLazFile->header().offset.x, mLazFile->header().offset.y, mLazFile->header().offset.z );

  mOriginalMetadata[ QStringLiteral( "creation_year" ) ] = mLazFile->header().creation.year;
  mOriginalMetadata[ QStringLiteral( "creation_day" ) ] = mLazFile->header().creation.day;
  mOriginalMetadata[ QStringLiteral( "major_version" ) ] = mLazFile->header().version.major;
  mOriginalMetadata[ QStringLiteral( "minor_version" ) ] = mLazFile->header().version.minor;
  mOriginalMetadata[ QStringLiteral( "dataformat_id" ) ] = mLazFile->header().point_format_id;
  mOriginalMetadata[ QStringLiteral( "scale_x" ) ] = mScale.x();
  mOriginalMetadata[ QStringLiteral( "scale_y" ) ] = mScale.y();
  mOriginalMetadata[ QStringLiteral( "scale_z" ) ] = mScale.z();
  mOriginalMetadata[ QStringLiteral( "offset_x" ) ] = mOffset.x();
  mOriginalMetadata[ QStringLiteral( "offset_y" ) ] = mOffset.y();
  mOriginalMetadata[ QStringLiteral( "offset_z" ) ] = mOffset.z();
  mOriginalMetadata[ QStringLiteral( "project_id" ) ] = QString( QByteArray( mLazFile->header().guid, 16 ).toHex() );
  mOriginalMetadata[ QStringLiteral( "system_id" ) ] = QString::fromLocal8Bit( mLazFile->header().system_identifier, 32 );
  mOriginalMetadata[ QStringLiteral( "software_id" ) ] = QString::fromLocal8Bit( mLazFile->header().generating_software, 32 );

  // The COPC format only uses PDRF 6, 7 or 8. So there should be a OGC Coordinate System WKT record
  mWkt = QString();
  std::vector<char> wktRecordData = mLazFile->vlrData( "LASF_Projection", 2112 );
  if ( !wktRecordData.empty() )
  {
    lazperf::wkt_vlr wktVlr;
    wktVlr.fill( wktRecordData.data(), wktRecordData.size() );
    mWkt = QString::fromStdString( wktVlr.wkt );
  }

  mExtent.set( mLazFile->header().minx, mLazFile->header().miny, mLazFile->header().maxx, mLazFile->header().maxy );

  mZMin = mLazFile->header().minz;
  mZMax = mLazFile->header().maxz;


  // Attributes for COPC format
  // COPC supports only PDRF 6, 7 and 8

  QgsPointCloudAttributeCollection attributes;
  attributes.push_back( QgsPointCloudAttribute( "X", QgsPointCloudAttribute::Int32 ) );
  attributes.push_back( QgsPointCloudAttribute( "Y", QgsPointCloudAttribute::Int32 ) );
  attributes.push_back( QgsPointCloudAttribute( "Z", QgsPointCloudAttribute::Int32 ) );
  attributes.push_back( QgsPointCloudAttribute( "Intensity", QgsPointCloudAttribute::UShort ) );
  attributes.push_back( QgsPointCloudAttribute( "ReturnNumber", QgsPointCloudAttribute::Char ) );
  attributes.push_back( QgsPointCloudAttribute( "NumberOfReturns", QgsPointCloudAttribute::Char ) );
  attributes.push_back( QgsPointCloudAttribute( "ScanDirectionFlag", QgsPointCloudAttribute::Char ) );
  attributes.push_back( QgsPointCloudAttribute( "EdgeOfFlightLine", QgsPointCloudAttribute::Char ) );
  attributes.push_back( QgsPointCloudAttribute( "Classification", QgsPointCloudAttribute::Char ) );
  attributes.push_back( QgsPointCloudAttribute( "ScanAngleRank", QgsPointCloudAttribute::Short ) );
  attributes.push_back( QgsPointCloudAttribute( "UserData", QgsPointCloudAttribute::Char ) );
  attributes.push_back( QgsPointCloudAttribute( "PointSourceId", QgsPointCloudAttribute::UShort ) );
  attributes.push_back( QgsPointCloudAttribute( "GpsTime", QgsPointCloudAttribute::Double ) );

  switch ( mLazFile->header().point_format_id )
  {
    case 6:
      break;
    case 7:
      attributes.push_back( QgsPointCloudAttribute( QStringLiteral( "Red" ), QgsPointCloudAttribute::UShort ) );
      attributes.push_back( QgsPointCloudAttribute( QStringLiteral( "Green" ), QgsPointCloudAttribute::UShort ) );
      attributes.push_back( QgsPointCloudAttribute( QStringLiteral( "Blue" ), QgsPointCloudAttribute::UShort ) );
      break;
    case 8:
      attributes.push_back( QgsPointCloudAttribute( QStringLiteral( "Red" ), QgsPointCloudAttribute::UShort ) );
      attributes.push_back( QgsPointCloudAttribute( QStringLiteral( "Green" ), QgsPointCloudAttribute::UShort ) );
      attributes.push_back( QgsPointCloudAttribute( QStringLiteral( "Blue" ), QgsPointCloudAttribute::UShort ) );
      attributes.push_back( QgsPointCloudAttribute( QStringLiteral( "NIR" ), QgsPointCloudAttribute::UShort ) );
      break;
    default:
      return false;
  }

  auto [ebVlr, pointRecordLength] = QgsLazDecoder::extractExtrabytesVlr( mCopcFile );
  QVector<QgsLazDecoder::ExtraBytesAttributeDetails> extrabyteAttributes = QgsLazDecoder::readExtraByteAttributesFromVlr( ebVlr, pointRecordLength );
  for ( QgsLazDecoder::ExtraBytesAttributeDetails attr : extrabyteAttributes )
  {
    attributes.push_back( QgsPointCloudAttribute( attr.attribute, attr.type ) );
  }

  setAttributes( attributes );

  std::vector<char> copcInfoVlrData = mLazFile->vlrData( "copc", 1 );

  lazperf::copc_info_vlr copcInfoVlr;
  copcInfoVlr.fill( copcInfoVlrData.data(), copcInfoVlrData.size() );

  const double xmin = copcInfoVlr.center_x - copcInfoVlr.halfsize;
  const double ymin = copcInfoVlr.center_y - copcInfoVlr.halfsize;
  const double zmin = copcInfoVlr.center_z - copcInfoVlr.halfsize;
  const double xmax = copcInfoVlr.center_x + copcInfoVlr.halfsize;
  const double ymax = copcInfoVlr.center_y + copcInfoVlr.halfsize;
  const double zmax = copcInfoVlr.center_z + copcInfoVlr.halfsize;

  mRootBounds = QgsPointCloudDataBounds(
                  ( xmin - mOffset.x() ) / mScale.x(),
                  ( ymin - mOffset.y() ) / mScale.y(),
                  ( zmin - mOffset.z() ) / mScale.z(),
                  ( xmax - mOffset.x() ) / mScale.x(),
                  ( ymax - mOffset.y() ) / mScale.y(),
                  ( zmax - mOffset.z() ) / mScale.z()
                );

  double calculatedSpan = nodeMapExtent( root() ).width() / copcInfoVlr.spacing;
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

  lazperf::header14 header = mLazFile->header();
  return QgsLazDecoder::decompressCopc( mFileName, header, blockOffset, blockSize, pointCount, attributes(), requestAttributes, scale(), offset(), filterExpression );
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
  return QgsCoordinateReferenceSystem::fromWkt( mWkt );
}

qint64 QgsCopcPointCloudIndex::pointCount() const
{
  return mPointCount;
}

bool QgsCopcPointCloudIndex::loadHierarchy()
{
  std::vector<char> copcInfoVlrData = mLazFile->vlrData( "copc", 1 );

  lazperf::copc_info_vlr copcInfoVlr;
  copcInfoVlr.fill( copcInfoVlrData.data(), copcInfoVlrData.size() );

  QMutexLocker locker( &mHierarchyMutex );
  fetchHierarchyPage( copcInfoVlr.root_hier_offset, copcInfoVlr.root_hier_size );
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
