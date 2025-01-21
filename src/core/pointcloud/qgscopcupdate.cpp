/***************************************************************************
    qgscopcupdate.cpp
    ---------------------
    begin                : January 2025
    copyright            : (C) 2025 by Martin Dobias
    email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgscopcupdate.h"

#include "qgslazdecoder.h"

#include <fstream>
#include <iostream>

#include <lazperf/header.hpp>
#include <lazperf/vlr.hpp>
#include <lazperf/Extractor.hpp>
#include <lazperf/filestream.hpp>
#include <lazperf/readers.hpp>
#include <lazperf/writers.hpp>


//! Keeps one entry of COPC hierarchy
struct HierarchyEntry
{
  //! Key of the data to which this entry corresponds
  QgsPointCloudNodeId key;

  /**
   * Absolute offset to the data chunk if the pointCount > 0.
   * Absolute offset to a child hierarchy page if the pointCount is -1.
   * 0 if the pointCount is 0.
   */
  uint64_t offset;

  /**
   * Size of the data chunk in bytes (compressed size) if the pointCount > 0.
   * Size of the hierarchy page if the pointCount is -1.
   * 0 if the pointCount is 0.
   */
  int32_t byteSize;

  /**
   * If > 0, represents the number of points in the data chunk.
   * If -1, indicates the information for this octree node is found in another hierarchy page.
   * If 0, no point data exists for this key, though may exist for child entries.
   */
  int32_t pointCount;
};

typedef QVector<HierarchyEntry> HierarchyEntries;


HierarchyEntries getHierarchyPage( std::ifstream &file, uint64_t offset, uint64_t size )
{
  HierarchyEntries page;
  std::vector<char> buf( 32 );
  int numEntries = static_cast<int>( size / 32 );
  file.seekg( static_cast<int64_t>( offset ) );
  while ( numEntries-- )
  {
    file.read( buf.data(), static_cast<long>( buf.size() ) );
    lazperf::LeExtractor s( buf.data(), buf.size() );

    HierarchyEntry e;
    int d, x, y, z;
    s >> d >> x >> y >> z;
    s >> e.offset >> e.byteSize >> e.pointCount;
    e.key = QgsPointCloudNodeId( d, x, y, z );

    page.push_back( e );
  }
  return page;
}


bool QgsCopcUpdate::write( const QString &outputFilename, const QHash<QgsPointCloudNodeId, UpdatedChunk> &updatedChunks )
{
  std::ofstream m_f;
  m_f.open( QgsLazDecoder::toNativePath( outputFilename ), std::ios::out | std::ios::binary );

  // write header and all VLRs all the way to point offset
  // (then we patch what we need)
  mFile.seekg( 0 );
  std::vector<char> allHeaderData;
  allHeaderData.resize( mHeader.point_offset );
  mFile.read( allHeaderData.data(), static_cast<long>( allHeaderData.size() ) );
  m_f.write( allHeaderData.data(), static_cast<long>( allHeaderData.size() ) );

  m_f.write( "XXXXXXXX", 8 ); // placeholder for chunk table offset

  uint64_t currentChunkOffset = mHeader.point_offset + 8;
  mFile.seekg( static_cast<long>( currentChunkOffset ) ); // this is where first chunk starts

  // now, let's write chunks:
  // - iterate through original chunk table, write out chunks
  //   - if chunk is updated, use that instead
  //   - keep updating hierarchy as we go
  //   - keep updating chunk table as we go

  QHash<QgsPointCloudNodeId, uint64_t> voxelToNewOffset;

  int chIndex = 0;
  for ( lazperf::chunk ch : mChunks )
  {
    Q_ASSERT( mOffsetToVoxel.contains( currentChunkOffset ) );
    QgsPointCloudNodeId n = mOffsetToVoxel[currentChunkOffset];

    uint64_t newOffset = m_f.tellp();
    voxelToNewOffset[n] = newOffset;

    // check whether the chunk is modified
    if ( updatedChunks.contains( n ) )
    {
      const UpdatedChunk &updatedChunk = updatedChunks[n];

      // use updated one and skip in the original file
      mFile.seekg( static_cast<long>( mFile.tellg() ) + static_cast<long>( ch.offset ) );

      m_f.write( updatedChunk.chunkData.constData(), updatedChunk.chunkData.size() );

      // update sizes
      mChunks[chIndex].offset = updatedChunk.chunkData.size();
    }
    else
    {
      // use as is
      std::vector<char> originalChunkData;
      originalChunkData.resize( ch.offset );
      mFile.read( originalChunkData.data(), static_cast<long>( originalChunkData.size() ) );
      m_f.write( originalChunkData.data(), static_cast<long>( originalChunkData.size() ) );
    }

    currentChunkOffset += ch.offset;
    ++chIndex;
  }

  // write chunk table: size in bytes + point count of each chunk

  const uint64_t newChunkTableOffset = m_f.tellp();

  m_f.write( "\0\0\0\0", 4 ); // chunk table version
  m_f.write( reinterpret_cast<const char *>( &mChunkCount ), sizeof( mChunkCount ) );

  lazperf::OutFileStream outStream( m_f );
  lazperf::compress_chunk_table( outStream.cb(), mChunks, true );

  // update hierarchy

  // NOTE: one big assumption we're doing here is that existing hierarchy pages
  // are packed one after another, with no gaps. if that's not the case, things
  // will break apart

  const long hierPositionShift = static_cast<long>( m_f.tellp() ) + 60 - static_cast<long>( mHierarchyOffset );

  HierarchyEntry *oldCopcHierarchyBlobEntries = reinterpret_cast<HierarchyEntry *>( mHierarchyBlob.data() );
  const int nEntries = static_cast<int>( mHierarchyBlob.size() / 32 );
  for ( int i = 0; i < nEntries; ++i )
  {
    HierarchyEntry &e = oldCopcHierarchyBlobEntries[i];
    if ( e.pointCount > 0 )
    {
      // update entry to new offset
      Q_ASSERT( voxelToNewOffset.contains( e.key ) );
      e.offset = voxelToNewOffset[e.key];

      if ( updatedChunks.contains( e.key ) )
      {
        uint64_t newByteSize = updatedChunks[e.key].chunkData.size();
        e.byteSize = static_cast<int>( newByteSize );
      }
    }
    else if ( e.pointCount < 0 )
    {
      // move hierarchy pages to new offset
      e.offset += hierPositionShift;
    }
    else  // pointCount == 0
    {
      // nothing to do - byte size and offset should be zero
    }

  }

  // write hierarchy eVLR

  const uint64_t newEvlrOffset = m_f.tellp();

  lazperf::evlr_header outCopcHierEvlr;
  outCopcHierEvlr.reserved = 0;
  outCopcHierEvlr.user_id = "copc";
  outCopcHierEvlr.record_id = 1000;
  outCopcHierEvlr.data_length = mHierarchyBlob.size();
  outCopcHierEvlr.description = "EPT Hierarchy";

  outCopcHierEvlr.write( m_f );
  m_f.write( mHierarchyBlob.data(), static_cast<long>( mHierarchyBlob.size() ) );

  // write other eVLRs

  for ( size_t i = 0; i < mEvlrHeaders.size(); ++i )
  {
    lazperf::evlr_header evlrHeader = mEvlrHeaders[i];
    std::vector<char> evlrBody = mEvlrData[i];

    evlrHeader.write( m_f );
    m_f.write( evlrBody.data(), static_cast<long>( evlrBody.size() ) );
  }

  // patch header

  m_f.seekp( 235 );
  m_f.write( reinterpret_cast<const char *>( &newEvlrOffset ), 8 );

  const uint64_t newRootHierOffset = mCopcVlr.root_hier_offset + hierPositionShift;
  m_f.seekp( 469 );
  m_f.write( reinterpret_cast<const char *>( &newRootHierOffset ), 8 );

  m_f.seekp( mHeader.point_offset );
  m_f.write( reinterpret_cast<const char *>( &newChunkTableOffset ), 8 );

  return true;
}



bool QgsCopcUpdate::read( const QString &inputFilename )
{
  mInputFilename = inputFilename;

  mFile.open( QgsLazDecoder::toNativePath( inputFilename ), std::ios::binary | std::ios::in );
  if ( mFile.fail() )
  {
    mErrorMessage = QStringLiteral( "Could not open file for reading: %1" ).arg( inputFilename );
    return false;
  }

  if ( !readHeader() )
    return false;

  readChunkTable();
  readHierarchy();

  return true;
}


bool QgsCopcUpdate::readHeader()
{
  // read header and COPC VLR
  mHeader = lazperf::header14::create( mFile );
  if ( !mFile )
  {
    mErrorMessage = QStringLiteral( "Error reading COPC header" );
    return false;
  }

  lazperf::vlr_header vh = lazperf::vlr_header::create( mFile );
  mCopcVlr = lazperf::copc_info_vlr::create( mFile );

  int baseCount = lazperf::baseCount( mHeader.point_format_id );
  if ( baseCount == 0 )
  {
    mErrorMessage = QStringLiteral( "Bad point record format: %1" ).arg( mHeader.point_format_id );
    return false;
  }

  return true;
}


void QgsCopcUpdate::readChunkTable()
{
  uint64_t chunkTableOffset;

  mFile.seekg( mHeader.point_offset );
  mFile.read( reinterpret_cast<char *>( &chunkTableOffset ), sizeof( chunkTableOffset ) );
  mFile.seekg( static_cast<long>( chunkTableOffset ) + 4 ); // The first 4 bytes are the version, then the chunk count.
  mFile.read( reinterpret_cast<char *>( &mChunkCount ), sizeof( mChunkCount ) );

  //
  // read chunk table
  //

  bool variable = true;

  // TODO: not sure why, but after decompress_chunk_table() the input stream seems to be dead, so we create a temporary one
  std::ifstream copcFileTmp;
  copcFileTmp.open( QgsLazDecoder::toNativePath( mInputFilename ), std::ios::binary | std::ios::in );
  copcFileTmp.seekg( mFile.tellg() );
  lazperf::InFileStream copcInFileStream( copcFileTmp );

  mChunks = lazperf::decompress_chunk_table( copcInFileStream.cb(), mChunkCount, variable );
  std::vector<lazperf::chunk> chunksWithAbsoluteOffsets;
  uint64_t nextChunkOffset = mHeader.point_offset + 8;
  for ( lazperf::chunk ch : mChunks )
  {
    chunksWithAbsoluteOffsets.push_back( {nextChunkOffset, ch.count} );
    nextChunkOffset += ch.offset;
  }
}


void QgsCopcUpdate::readHierarchy()
{
  // get all hierarchy pages

  HierarchyEntries childEntriesToProcess;
  childEntriesToProcess.push_back( HierarchyEntry
  {
    QgsPointCloudNodeId( 0, 0, 0, 0 ),
    mCopcVlr.root_hier_offset,
    static_cast<int32_t>( mCopcVlr.root_hier_size ),
    -1 } );

  while ( !childEntriesToProcess.empty() )
  {
    HierarchyEntry childEntry = childEntriesToProcess.back();
    childEntriesToProcess.pop_back();

    HierarchyEntries page = getHierarchyPage( mFile, childEntry.offset, childEntry.byteSize );

    for ( const HierarchyEntry &e : page )
    {
      if ( e.pointCount > 0 ) // it's a non-empty node
      {
        Q_ASSERT( !mOffsetToVoxel.contains( e.offset ) );
        mOffsetToVoxel[e.offset] = e.key;
      }
      else if ( e.pointCount < 0 ) // referring to a child page
      {
        childEntriesToProcess.push_back( e );
      }
    }
  }

  lazperf::evlr_header evlr1;
  mFile.seekg( static_cast<long>( mHeader.evlr_offset ) );

  mHierarchyOffset = 0;  // where the hierarchy eVLR payload starts

  for ( uint32_t i = 0; i < mHeader.evlr_count; ++i )
  {
    evlr1.read( mFile );
    if ( evlr1.user_id == "copc" && evlr1.record_id == 1000 )
    {
      mHierarchyBlob.resize( evlr1.data_length );
      mHierarchyOffset = mFile.tellg();
      mFile.read( mHierarchyBlob.data(), static_cast<long>( evlr1.data_length ) );
    }
    else
    {
      // keep for later
      mEvlrHeaders.push_back( evlr1 );
      std::vector<char> evlrBlob;
      evlrBlob.resize( evlr1.data_length );
      mFile.read( evlrBlob.data(), static_cast<long>( evlrBlob.size() ) );
      mEvlrData.push_back( evlrBlob );
    }
  }

  Q_ASSERT( !mHierarchyBlob.empty() );
}


bool QgsCopcUpdate::writeUpdatedFile( const QString &inputFilename,
                                      const QString &outputFilename,
                                      const QHash<QgsPointCloudNodeId, UpdatedChunk> &updatedChunks,
                                      QString *errorMessage )
{
  QgsCopcUpdate copcUpdate;
  if ( !copcUpdate.read( inputFilename ) )
  {
    if ( errorMessage )
      *errorMessage = copcUpdate.errorMessage();
    return false;
  }

  if ( !copcUpdate.write( outputFilename, updatedChunks ) )
  {
    if ( errorMessage )
      *errorMessage = copcUpdate.errorMessage();
    return false;
  }

  return true;
}
