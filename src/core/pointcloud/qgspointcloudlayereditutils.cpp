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

#include "lazperf/readers.hpp"
#include "lazperf/writers.hpp"
#include "qgscopcpointcloudindex.h"
#include "qgseventtracing.h"
#include "qgslazdecoder.h"

#include <QMutex>

static void updatePoint( char *pointBuffer, int pointFormat, const QString &attributeName, double newValue )
{
  if ( attributeName == "Intensity"_L1 )  // unsigned short
  {
    quint16 newValueShort = static_cast<quint16>( newValue );
    memcpy( pointBuffer + 12, &newValueShort, sizeof( qint16 ) );
  }
  else if ( attributeName == "ReturnNumber"_L1 )  // bits 0-3
  {
    uchar newByteValue = static_cast<uchar>( newValue ) & 0xf;
    pointBuffer[14] = static_cast<char>( ( pointBuffer[14] & 0xf0 ) | newByteValue );
  }
  else if ( attributeName == "NumberOfReturns"_L1 )  // bits 4-7
  {
    uchar newByteValue = ( static_cast<uchar>( newValue ) & 0xf ) << 4;
    pointBuffer[14] = static_cast<char>( ( pointBuffer[14] & 0xf ) | newByteValue );
  }
  else if ( attributeName == "Synthetic"_L1 )  // bit 0
  {
    uchar newByteValue = ( static_cast<uchar>( newValue ) & 0x1 );
    pointBuffer[15] = static_cast<char>( ( pointBuffer[15] & 0xfe ) | newByteValue );
  }
  else if ( attributeName == "KeyPoint"_L1 )  // bit 1
  {
    uchar newByteValue = ( static_cast<uchar>( newValue ) & 0x1 ) << 1;
    pointBuffer[15] = static_cast<char>( ( pointBuffer[15] & 0xfd ) | newByteValue );
  }
  else if ( attributeName == "Withheld"_L1 )  // bit 2
  {
    uchar newByteValue = ( static_cast<uchar>( newValue ) & 0x1 ) << 2;
    pointBuffer[15] = static_cast<char>( ( pointBuffer[15] & 0xfb ) | newByteValue );
  }
  else if ( attributeName == "Overlap"_L1 )  // bit 3
  {
    uchar newByteValue = ( static_cast<uchar>( newValue ) & 0x1 ) << 3;
    pointBuffer[15] = static_cast<char>( ( pointBuffer[15] & 0xf7 ) | newByteValue );
  }
  else if ( attributeName == "ScannerChannel"_L1 )  // bits 4-5
  {
    uchar newByteValue = ( static_cast<uchar>( newValue ) & 0x3 ) << 4;
    pointBuffer[15] = static_cast<char>( ( pointBuffer[15] & 0xcf ) | newByteValue );
  }
  else if ( attributeName == "ScanDirectionFlag"_L1 )  // bit 6
  {
    uchar newByteValue = ( static_cast<uchar>( newValue ) & 0x1 ) << 6;
    pointBuffer[15] = static_cast<char>( ( pointBuffer[15] & 0xbf ) | newByteValue );
  }
  else if ( attributeName == "EdgeOfFlightLine"_L1 )  // bit 7
  {
    uchar newByteValue = ( static_cast<uchar>( newValue ) & 0x1 ) << 7;
    pointBuffer[15] = static_cast<char>( ( pointBuffer[15] & 0x7f ) | newByteValue );
  }
  else if ( attributeName == "Classification"_L1 )  // unsigned char
  {
    pointBuffer[16] = static_cast<char>( static_cast<uchar>( newValue ) );
  }
  else if ( attributeName == "UserData"_L1 )  // unsigned char
  {
    pointBuffer[17] = static_cast<char>( static_cast<uchar>( newValue ) );
  }
  else if ( attributeName == "ScanAngleRank"_L1 )  // short
  {
    qint16 newValueShort = static_cast<qint16>( std::round( newValue / 0.006 ) );  // copc stores angle in 0.006deg increments
    memcpy( pointBuffer + 18, &newValueShort, sizeof( qint16 ) );
  }
  else if ( attributeName == "PointSourceId"_L1 )  // unsigned short
  {
    quint16 newValueShort = static_cast<quint16>( newValue );
    memcpy( pointBuffer + 20, &newValueShort, sizeof( quint16 ) );
  }
  else if ( attributeName == "GpsTime"_L1 )  // double
  {
    memcpy( pointBuffer + 22, &newValue, sizeof( double ) );
  }
  else if ( pointFormat == 7 || pointFormat == 8 )
  {
    if ( attributeName == "Red"_L1 )  // unsigned short
    {
      quint16 newValueShort = static_cast<quint16>( newValue );
      memcpy( pointBuffer + 30, &newValueShort, sizeof( quint16 ) );
    }
    else if ( attributeName == "Green"_L1 )  // unsigned short
    {
      quint16 newValueShort = static_cast<quint16>( newValue );
      memcpy( pointBuffer + 32, &newValueShort, sizeof( quint16 ) );
    }
    else if ( attributeName == "Blue"_L1 )  // unsigned short
    {
      quint16 newValueShort = static_cast<quint16>( newValue );
      memcpy( pointBuffer + 34, &newValueShort, sizeof( quint16 ) );
    }
    else if ( pointFormat == 8 )
    {
      if ( attributeName == "Infrared"_L1 )  // unsigned short
      {
        quint16 newValueShort = static_cast<quint16>( newValue );
        memcpy( pointBuffer + 36, &newValueShort, sizeof( quint16 ) );
      }
    }
  }
}


QByteArray QgsPointCloudLayerEditUtils::updateChunkValues( QgsCopcPointCloudIndex *copcIndex, const QByteArray &chunkData, const QgsPointCloudAttribute &attribute, const QgsPointCloudNodeId &n, const QHash<int, double> &pointValues, std::optional<double> newValue )
{
  QgsEventTracing::ScopedEvent _trace( u"PointCloud"_s, u"QgsPointCloudLayerEditUtils::updateChunkValues"_s );

  int pointCount;

  {
    QMutexLocker locker( &copcIndex->mHierarchyMutex );

    Q_ASSERT( copcIndex->mHierarchy.contains( n ) );
    Q_ASSERT( copcIndex->mHierarchyNodePos.contains( n ) );

    pointCount = copcIndex->mHierarchy[n];
  }

  lazperf::header14 header = copcIndex->mLazInfo->header();

  lazperf::reader::chunk_decompressor decompressor( header.pointFormat(), header.ebCount(), chunkData.constData() );
  lazperf::writer::chunk_compressor compressor( header.pointFormat(), header.ebCount() );

  std::unique_ptr<char[]> decodedData( new char[header.point_record_length] );

  // only PDRF 6/7/8 is allowed by COPC
  Q_ASSERT( header.pointFormat() == 6 || header.pointFormat() == 7 || header.pointFormat() == 8 );

  QString attributeName = attribute.name();

  for ( int i = 0; i < pointCount; ++i )
  {
    decompressor.decompress( decodedData.get() );
    char *buf = decodedData.get();

    if ( pointValues.contains( i ) )
    {
      // TODO: support for extrabytes attributes
      updatePoint( buf, header.point_format_id, attributeName, newValue ? *newValue : pointValues[i] );
    }

    compressor.compress( decodedData.get() );
  }

  std::vector<unsigned char> data = compressor.done();
  return QByteArray( ( const char * ) data.data(), ( int ) data.size() ); // QByteArray makes a deep copy
}

bool QgsPointCloudLayerEditUtils::isAttributeValueValid( const QgsPointCloudAttribute &attribute, double value )
{
  const QString name = attribute.name().toUpper();

  if ( name == "INTENSITY"_L1 )
    return value >= 0 && value <= 65535;
  if ( name == "RETURNNUMBER"_L1 )
    return value >= 0 && value <= 15;
  if ( name == "NUMBEROFRETURNS"_L1 )
    return value >= 0 && value <= 15;
  if ( name == "SCANNERCHANNEL"_L1 )
    return value >= 0 && value <= 3;
  if ( name == "SCANDIRECTIONFLAG"_L1 )
    return value >= 0 && value <= 1;
  if ( name == "EDGEOFFLIGHTLINE"_L1 )
    return value >= 0 && value <= 1;
  if ( name == "CLASSIFICATION"_L1 )
    return value >= 0 && value <= 255;
  if ( name == "USERDATA"_L1 )
    return value >= 0 && value <= 255;
  if ( name == "SCANANGLERANK"_L1 )
    return value >= -180 && value <= 180;
  if ( name == "POINTSOURCEID"_L1 )
    return value >= 0 && value <= 65535;
  if ( name == "GPSTIME"_L1 )
    return value >= 0;
  if ( name == "SYNTHETIC"_L1 )
    return value >= 0 && value <= 1;
  if ( name == "KEYPOINT"_L1 )
    return value >= 0 && value <= 1;
  if ( name == "WITHHELD"_L1 )
    return value >= 0 && value <= 1;
  if ( name == "OVERLAP"_L1 )
    return value >= 0 && value <= 1;
  if ( name == "RED"_L1 )
    return value >= 0 && value <= 65535;
  if ( name == "GREEN"_L1 )
    return value >= 0 && value <= 65535;
  if ( name == "BLUE"_L1 )
    return value >= 0 && value <= 65535;
  if ( name == "INFRARED"_L1 )
    return value >= 0 && value <= 65535;

  return true;
}
