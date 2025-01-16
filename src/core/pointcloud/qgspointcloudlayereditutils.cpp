/***************************************************************************
    qgspointcloudlayereditutils.cpp
    ---------------------
    begin                : December 2024
    copyright            : (C) 2024 by Stefanos Natsis
    email                : uclaros at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgspointcloudlayereditutils.h"
#include "qgspointcloudlayer.h"
#include "qgslazdecoder.h"
#include "qgscopcpointcloudindex.h"
#include "qgspointcloudeditingindex.h"

#include <lazperf/readers.hpp>
#include <lazperf/writers.hpp>


QgsPointCloudLayerEditUtils::QgsPointCloudLayerEditUtils( QgsPointCloudLayer *layer )
  : mIndex( layer->index() )
{
}

bool QgsPointCloudLayerEditUtils::changeAttributeValue( const QgsPointCloudNodeId &n, const QVector<int> &pts, const QgsPointCloudAttribute &attribute, double value )
{
  // Cannot allow x,y,z editing as points may get moved outside the node extents
  if ( attribute.name().compare( QLatin1String( "X" ), Qt::CaseInsensitive ) == 0 ||
       attribute.name().compare( QLatin1String( "Y" ), Qt::CaseInsensitive ) == 0 ||
       attribute.name().compare( QLatin1String( "Z" ), Qt::CaseInsensitive ) == 0 )
    return false;

  if ( !n.isValid() || !mIndex.hasNode( n ) ) // todo: should not have to check if n.isValid
    return false;

  const QgsPointCloudAttributeCollection attributeCollection = mIndex.attributes();

  int attributeOffset;
  const QgsPointCloudAttribute *at = attributeCollection.find( attribute.name(), attributeOffset );

  if ( !at ||
       at->size() != attribute.size() ||
       at->type() != attribute.type() )
  {
    return false;
  }

  if ( !isAttributeValueValid( attribute, value ) )
  {
    return false;
  }

  const QSet<int> uniquePoints( pts.constBegin(), pts.constEnd() );
  QVector<int> sortedPoints( uniquePoints.constBegin(), uniquePoints.constEnd() );
  std::sort( sortedPoints.begin(), sortedPoints.end() );

  if ( sortedPoints.constFirst() < 0 ||
       sortedPoints.constLast() > mIndex.getNode( n ).pointCount() )
    return false;

  QgsPointCloudEditingIndex *editIndex = static_cast<QgsPointCloudEditingIndex *>( mIndex.get() );
  QgsCopcPointCloudIndex *copcIndex = static_cast<QgsCopcPointCloudIndex *>( editIndex->mIndex.get() );

  QByteArray chunkData;
  if ( editIndex->mEditedNodeData.contains( n ) )
  {
    chunkData = editIndex->mEditedNodeData[n];
  }
  else
  {
    QPair<uint64_t, int32_t> offsetSizePair = copcIndex->mHierarchyNodePos[n];
    chunkData = copcIndex->readRange( offsetSizePair.first, offsetSizePair.second );
  }

  QByteArray data = updateChunkValues( copcIndex, chunkData, *at, value, n, pts );

  return mIndex.updateNodeData( {{n, data}} );
}


static void updatePoint( char *pointBuffer, int pointFormat, const QString &attributeName, double newValue )
{
  if ( attributeName == QLatin1String( "Intensity" ) )  // unsigned short
  {
    quint16 newValueShort = static_cast<quint16>( newValue );
    memcpy( pointBuffer + 12, &newValueShort, sizeof( qint16 ) );
  }
  else if ( attributeName == QLatin1String( "ReturnNumber" ) )  // bits 0-3
  {
    uchar newByteValue = static_cast<uchar>( newValue ) & 0xf;
    pointBuffer[14] = static_cast<char>( ( pointBuffer[14] & 0xf0 ) | newByteValue );
  }
  else if ( attributeName == QLatin1String( "NumberOfReturns" ) )  // bits 4-7
  {
    uchar newByteValue = ( static_cast<uchar>( newValue ) & 0xf ) << 4;
    pointBuffer[14] = static_cast<char>( ( pointBuffer[14] & 0xf ) | newByteValue );
  }
  else if ( attributeName == QLatin1String( "Synthetic" ) )  // bit 0
  {
    uchar newByteValue = ( static_cast<uchar>( newValue ) & 0x1 );
    pointBuffer[15] = static_cast<char>( ( pointBuffer[15] & 0xfe ) | newByteValue );
  }
  else if ( attributeName == QLatin1String( "KeyPoint" ) )  // bit 1
  {
    uchar newByteValue = ( static_cast<uchar>( newValue ) & 0x1 ) << 1;
    pointBuffer[15] = static_cast<char>( ( pointBuffer[15] & 0xfd ) | newByteValue );
  }
  else if ( attributeName == QLatin1String( "Withheld" ) )  // bit 2
  {
    uchar newByteValue = ( static_cast<uchar>( newValue ) & 0x1 ) << 2;
    pointBuffer[15] = static_cast<char>( ( pointBuffer[15] & 0xfb ) | newByteValue );
  }
  else if ( attributeName == QLatin1String( "Overlap" ) )  // bit 3
  {
    uchar newByteValue = ( static_cast<uchar>( newValue ) & 0x1 ) << 3;
    pointBuffer[15] = static_cast<char>( ( pointBuffer[15] & 0xf7 ) | newByteValue );
  }
  else if ( attributeName == QLatin1String( "ScannerChannel" ) )  // bits 4-5
  {
    uchar newByteValue = ( static_cast<uchar>( newValue ) & 0x3 ) << 4;
    pointBuffer[15] = static_cast<char>( ( pointBuffer[15] & 0xcf ) | newByteValue );
  }
  else if ( attributeName == QLatin1String( "ScanDirectionFlag" ) )  // bit 6
  {
    uchar newByteValue = ( static_cast<uchar>( newValue ) & 0x1 ) << 6;
    pointBuffer[15] = static_cast<char>( ( pointBuffer[15] & 0xbf ) | newByteValue );
  }
  else if ( attributeName == QLatin1String( "EdgeOfFlightLine" ) )  // bit 7
  {
    uchar newByteValue = ( static_cast<uchar>( newValue ) & 0x1 ) << 7;
    pointBuffer[15] = static_cast<char>( ( pointBuffer[15] & 0x7f ) | newByteValue );
  }
  else if ( attributeName == QLatin1String( "Classification" ) )  // unsigned char
  {
    pointBuffer[16] = static_cast<char>( static_cast<uchar>( newValue ) );
  }
  else if ( attributeName == QLatin1String( "UserData" ) )  // unsigned char
  {
    pointBuffer[17] = static_cast<char>( static_cast<uchar>( newValue ) );
  }
  else if ( attributeName == QLatin1String( "ScanAngleRank" ) )  // short
  {
    qint16 newValueShort = static_cast<qint16>( newValue );
    memcpy( pointBuffer + 18, &newValueShort, sizeof( qint16 ) );
  }
  else if ( attributeName == QLatin1String( "PointSourceId" ) )  // unsigned short
  {
    quint16 newValueShort = static_cast<quint16>( newValue );
    memcpy( pointBuffer + 20, &newValueShort, sizeof( quint16 ) );
  }
  else if ( attributeName == QLatin1String( "GpsTime" ) )  // double
  {
    memcpy( pointBuffer + 22, &newValue, sizeof( double ) );
  }
  else if ( pointFormat == 7 || pointFormat == 8 )
  {
    if ( attributeName == QLatin1String( "Red" ) )  // unsigned short
    {
      quint16 newValueShort = static_cast<quint16>( newValue );
      memcpy( pointBuffer + 30, &newValueShort, sizeof( quint16 ) );
    }
    else if ( attributeName == QLatin1String( "Green" ) )  // unsigned short
    {
      quint16 newValueShort = static_cast<quint16>( newValue );
      memcpy( pointBuffer + 32, &newValueShort, sizeof( quint16 ) );
    }
    else if ( attributeName == QLatin1String( "Blue" ) )  // unsigned short
    {
      quint16 newValueShort = static_cast<quint16>( newValue );
      memcpy( pointBuffer + 34, &newValueShort, sizeof( quint16 ) );
    }
    else if ( pointFormat == 8 )
    {
      if ( attributeName == QLatin1String( "Infrared" ) )  // unsigned short
      {
        quint16 newValueShort = static_cast<quint16>( newValue );
        memcpy( pointBuffer + 36, &newValueShort, sizeof( quint16 ) );
      }
    }
  }
}


QByteArray QgsPointCloudLayerEditUtils::updateChunkValues( QgsCopcPointCloudIndex *copcIndex, const QByteArray &chunkData, const QgsPointCloudAttribute &attribute, double newValue, const QgsPointCloudNodeId &n, const QVector<int> &pointIndices )
{
  Q_ASSERT( copcIndex->mHierarchy.contains( n ) );
  Q_ASSERT( copcIndex->mHierarchyNodePos.contains( n ) );

  int pointCount = copcIndex->mHierarchy[n];

  lazperf::header14 header = copcIndex->mLazInfo->header();

  lazperf::reader::chunk_decompressor decompressor( header.pointFormat(), header.ebCount(), chunkData.constData() );
  lazperf::writer::chunk_compressor compressor( header.pointFormat(), header.ebCount() );

  std::unique_ptr<char []> decodedData( new char[ header.point_record_length ] );

  // only PDRF 6/7/8 is allowed by COPC
  Q_ASSERT( header.pointFormat() == 6 || header.pointFormat() == 7 || header.pointFormat() == 8 );

  QSet<int> pointIndicesSet( pointIndices.constBegin(), pointIndices.constEnd() );

  QString attributeName = attribute.name();

  for ( int i = 0 ; i < pointCount; ++i )
  {
    decompressor.decompress( decodedData.get() );
    char *buf = decodedData.get();

    if ( pointIndicesSet.contains( i ) )
    {
      // TODO: support for extrabytes attributes
      updatePoint( buf, header.point_format_id, attributeName, newValue );
    }

    compressor.compress( decodedData.get() );
  }

  std::vector<unsigned char> data = compressor.done();
  return QByteArray( ( const char * ) data.data(), ( int ) data.size() ); // QByteArray makes a deep copy
}


QByteArray QgsPointCloudLayerEditUtils::dataForAttributes( const QgsPointCloudAttributeCollection &allAttributes, const QByteArray &data, const QgsPointCloudRequest &request )
{
  const QVector<QgsPointCloudAttribute> attributes = allAttributes.attributes();
  const int nPoints = data.size() / allAttributes.pointRecordSize();
  const char *ptr = data.data();

  QByteArray outData;
  for ( int i = 0; i < nPoints; ++i )
  {
    for ( const QgsPointCloudAttribute &attr : attributes )
    {
      if ( request.attributes().indexOf( attr.name() ) >= 0 )
      {
        outData.append( ptr, attr.size() );
      }
      ptr += attr.size();
    }
  }

  //
  Q_ASSERT( nPoints == outData.size() / request.attributes().pointRecordSize() );

  return outData;
}

bool QgsPointCloudLayerEditUtils::isAttributeValueValid( const QgsPointCloudAttribute &attribute, double value )
{
  const QString name = attribute.name().toUpper();

  if ( name == QLatin1String( "INTENSITY" ) )
    return value >= 0 && value <= 65535;
  if ( name == QLatin1String( "RETURNNUMBER" ) )
    return value >= 0 && value <= 15;
  if ( name == QLatin1String( "NUMBEROFRETURNS" ) )
    return value >= 0 && value <= 15;
  if ( name == QLatin1String( "SCANCHANNEL" ) )
    return value >= 0 && value <= 3;
  if ( name == QLatin1String( "SCANDIRECTIONFLAG" ) )
    return value >= 0 && value <= 1;
  if ( name == QLatin1String( "EDGEOFFLIGHTLINE" ) )
    return value >= 0 && value <= 1;
  if ( name == QLatin1String( "CLASSIFICATION" ) )
    return value >= 0 && value <= 255;
  if ( name == QLatin1String( "USERDATA" ) )
    return value >= 0 && value <= 255;
  if ( name == QLatin1String( "SCANANGLE" ) )
    return value >= -30'000 && value <= 30'000;
  if ( name == QLatin1String( "POINTSOURCEID" ) )
    return value >= 0 && value <= 65535;
  if ( name == QLatin1String( "GPSTIME" ) )
    return value >= 0;
  if ( name == QLatin1String( "SYNTHETIC" ) )
    return value >= 0 && value <= 1;
  if ( name == QLatin1String( "KEYPOINT" ) )
    return value >= 0 && value <= 1;
  if ( name == QLatin1String( "WITHHELD" ) )
    return value >= 0 && value <= 1;
  if ( name == QLatin1String( "OVERLAP" ) )
    return value >= 0 && value <= 1;
  if ( name == QLatin1String( "RED" ) )
    return value >= 0 && value <= 65535;
  if ( name == QLatin1String( "GREEN" ) )
    return value >= 0 && value <= 65535;
  if ( name == QLatin1String( "BLUE" ) )
    return value >= 0 && value <= 65535;

  return true;
}
