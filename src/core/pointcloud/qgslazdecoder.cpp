/***************************************************************************
                         qgslazdecoder.cpp
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

#include "qgslazdecoder.h"
#include "qgseptpointcloudindex.h"
#include "qgspointcloudattribute.h"
#include "qgsvector3d.h"
#include "qgsconfig.h"
#include "qgslogger.h"

#include <QFile>
#include <QDir>
#include <iostream>
#include <memory>
#include <cstring>
#include <QElapsedTimer>
#include <QTemporaryFile>
#include <string>

#include <zstd.h>

#include "lazperf/las.hpp"


///@cond PRIVATE

template <typename T>
bool _storeToStream( char *s, size_t position, QgsPointCloudAttribute::DataType type, T value );

bool __serialize( char *data, size_t outputPosition, QgsPointCloudAttribute::DataType outputType,
                  const char *input, QgsPointCloudAttribute::DataType inputType, int inputSize, size_t inputPosition );

// //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template<typename FileType>
std::vector< QgsLazDecoder::RequestedAttributeDetails > __prepareRequestedAttributeDetails( FileType &file, const QgsPointCloudAttributeCollection &requestedAttributes )
{
  const QVector<QgsPointCloudAttribute> requestedAttributesVector = requestedAttributes.attributes();
  QVector<QgsLazDecoder::ExtraBytesAttributeDetails> extrabytesAttr = QgsLazDecoder::readExtraByteAttributes<FileType>( file );

  std::vector< QgsLazDecoder::RequestedAttributeDetails > requestedAttributeDetails;
  requestedAttributeDetails.reserve( requestedAttributesVector.size() );
  for ( const QgsPointCloudAttribute &requestedAttribute : requestedAttributesVector )
  {
    if ( requestedAttribute.name().compare( QLatin1String( "X" ), Qt::CaseInsensitive ) == 0 )
    {
      requestedAttributeDetails.emplace_back( QgsLazDecoder::RequestedAttributeDetails( QgsLazDecoder::LazAttribute::X, requestedAttribute.type(), requestedAttribute.size() ) );
    }
    else if ( requestedAttribute.name().compare( QLatin1String( "Y" ), Qt::CaseInsensitive ) == 0 )
    {
      requestedAttributeDetails.emplace_back( QgsLazDecoder::RequestedAttributeDetails( QgsLazDecoder::LazAttribute::Y, requestedAttribute.type(), requestedAttribute.size() ) );
    }
    else if ( requestedAttribute.name().compare( QLatin1String( "Z" ), Qt::CaseInsensitive ) == 0 )
    {
      requestedAttributeDetails.emplace_back( QgsLazDecoder::RequestedAttributeDetails( QgsLazDecoder::LazAttribute::Z, requestedAttribute.type(), requestedAttribute.size() ) );
    }
    else if ( requestedAttribute.name().compare( QLatin1String( "Classification" ), Qt::CaseInsensitive ) == 0 )
    {
      requestedAttributeDetails.emplace_back( QgsLazDecoder::RequestedAttributeDetails( QgsLazDecoder::LazAttribute::Classification, requestedAttribute.type(), requestedAttribute.size() ) );
    }
    else if ( requestedAttribute.name().compare( QLatin1String( "Intensity" ), Qt::CaseInsensitive ) == 0 )
    {
      requestedAttributeDetails.emplace_back( QgsLazDecoder::RequestedAttributeDetails( QgsLazDecoder::LazAttribute::Intensity, requestedAttribute.type(), requestedAttribute.size() ) );
    }
    else if ( requestedAttribute.name().compare( QLatin1String( "ReturnNumber" ), Qt::CaseInsensitive ) == 0 )
    {
      requestedAttributeDetails.emplace_back( QgsLazDecoder::RequestedAttributeDetails( QgsLazDecoder::LazAttribute::ReturnNumber, requestedAttribute.type(), requestedAttribute.size() ) );
    }
    else if ( requestedAttribute.name().compare( QLatin1String( "NumberOfReturns" ), Qt::CaseInsensitive ) == 0 )
    {
      requestedAttributeDetails.emplace_back( QgsLazDecoder::RequestedAttributeDetails( QgsLazDecoder::LazAttribute::NumberOfReturns, requestedAttribute.type(), requestedAttribute.size() ) );
    }
    else if ( requestedAttribute.name().compare( QLatin1String( "ScanDirectionFlag" ), Qt::CaseInsensitive ) == 0 )
    {
      requestedAttributeDetails.emplace_back( QgsLazDecoder::RequestedAttributeDetails( QgsLazDecoder::LazAttribute::ScanDirectionFlag, requestedAttribute.type(), requestedAttribute.size() ) );
    }
    else if ( requestedAttribute.name().compare( QLatin1String( "EdgeOfFlightLine" ), Qt::CaseInsensitive ) == 0 )
    {
      requestedAttributeDetails.emplace_back( QgsLazDecoder::RequestedAttributeDetails( QgsLazDecoder::LazAttribute::EdgeOfFlightLine, requestedAttribute.type(), requestedAttribute.size() ) );
    }
    else if ( requestedAttribute.name().compare( QLatin1String( "ScanAngleRank" ), Qt::CaseInsensitive ) == 0 )
    {
      requestedAttributeDetails.emplace_back( QgsLazDecoder::RequestedAttributeDetails( QgsLazDecoder::LazAttribute::ScanAngleRank, requestedAttribute.type(), requestedAttribute.size() ) );
    }
    else if ( requestedAttribute.name().compare( QLatin1String( "UserData" ), Qt::CaseInsensitive ) == 0 )
    {
      requestedAttributeDetails.emplace_back( QgsLazDecoder::RequestedAttributeDetails( QgsLazDecoder::LazAttribute::UserData, requestedAttribute.type(), requestedAttribute.size() ) );
    }
    else if ( requestedAttribute.name().compare( QLatin1String( "PointSourceId" ), Qt::CaseInsensitive ) == 0 )
    {
      requestedAttributeDetails.emplace_back( QgsLazDecoder::RequestedAttributeDetails( QgsLazDecoder::LazAttribute::PointSourceId, requestedAttribute.type(), requestedAttribute.size() ) );
    }
    else if ( requestedAttribute.name().compare( QLatin1String( "GpsTime" ), Qt::CaseInsensitive ) == 0 )
    {
      requestedAttributeDetails.emplace_back( QgsLazDecoder::RequestedAttributeDetails( QgsLazDecoder::LazAttribute::GpsTime, requestedAttribute.type(), requestedAttribute.size() ) );
    }
    else if ( requestedAttribute.name().compare( QLatin1String( "Red" ), Qt::CaseInsensitive ) == 0 )
    {
      requestedAttributeDetails.emplace_back( QgsLazDecoder::RequestedAttributeDetails( QgsLazDecoder::LazAttribute::Red, requestedAttribute.type(), requestedAttribute.size() ) );
    }
    else if ( requestedAttribute.name().compare( QLatin1String( "Green" ), Qt::CaseInsensitive ) == 0 )
    {
      requestedAttributeDetails.emplace_back( QgsLazDecoder::RequestedAttributeDetails( QgsLazDecoder::LazAttribute::Green, requestedAttribute.type(), requestedAttribute.size() ) );
    }
    else if ( requestedAttribute.name().compare( QLatin1String( "Blue" ), Qt::CaseInsensitive ) == 0 )
    {
      requestedAttributeDetails.emplace_back( QgsLazDecoder::RequestedAttributeDetails( QgsLazDecoder::LazAttribute::Blue, requestedAttribute.type(), requestedAttribute.size() ) );
    }
    else
    {
      bool foundAttr = false;
      for ( QgsLazDecoder::ExtraBytesAttributeDetails &eba : extrabytesAttr )
      {
        if ( requestedAttribute.name().compare( eba.attribute.trimmed() ) == 0 )
        {
          requestedAttributeDetails.emplace_back( QgsLazDecoder::RequestedAttributeDetails( QgsLazDecoder::LazAttribute::ExtraBytes, eba.type, eba.size, eba.offset ) );
          foundAttr = true;
          break;
        }
      }
      if ( !foundAttr )
      {
        // this can possibly happen -- e.g. if a style built using a different point cloud format references an attribute which isn't available from the laz file
        requestedAttributeDetails.emplace_back( QgsLazDecoder::RequestedAttributeDetails( QgsLazDecoder::LazAttribute::MissingOrUnknown, requestedAttribute.type(), requestedAttribute.size() ) );
      }
    }
  }
  return requestedAttributeDetails;
}

void decodePoint( char *buf, int lasPointFormat, char *dataBuffer, std::size_t &outputOffset, lazperf::las::point10 &p10, lazperf::las::gpstime &gps, lazperf::las::rgb &rgb, lazperf::las::point14 &p14, std::vector< QgsLazDecoder::RequestedAttributeDetails > &requestedAttributeDetails )
{
  bool isLas14 = ( lasPointFormat == 6 || lasPointFormat == 7 || lasPointFormat == 8 );

  switch ( lasPointFormat )
  {
    // LAS 1.2 file support
    case 0: // base
      p10.unpack( buf );
      break;
    case 1: // base + gps time
      p10.unpack( buf );
      gps.unpack( buf + sizeof( lazperf::las::point10 ) );
      break;
    case 2: // base + rgb
      p10.unpack( buf );
      rgb.unpack( buf + sizeof( lazperf::las::point10 ) );
      break;
    case 3: // base + gps time + rgb
      p10.unpack( buf );
      gps.unpack( buf + sizeof( lazperf::las::point10 ) );
      rgb.unpack( buf + sizeof( lazperf::las::point10 ) + sizeof( lazperf::las::gpstime ) );
      break;

    // LAS 1.4 file support
    case 6: // base (includes gps time)
      p14.unpack( buf );
      break;
    case 7: // base + rgb
      p14.unpack( buf );
      rgb.unpack( buf + sizeof( lazperf::las::point14 ) );
      break;
    case 8: // base + rgb + nir
      p14.unpack( buf );
      rgb.unpack( buf + sizeof( lazperf::las::point14 ) );
      // TODO: load NIR channel - need some testing data!
      break;

    default:
      Q_ASSERT( false );  // must not happen - we checked earlier that the format is supported
  }

  for ( const QgsLazDecoder::RequestedAttributeDetails &requestedAttribute : requestedAttributeDetails )
  {
    switch ( requestedAttribute.attribute )
    {
      case QgsLazDecoder::LazAttribute::X:
        _storeToStream<qint32>( dataBuffer, outputOffset, requestedAttribute.type, isLas14 ? p14.x() : p10.x );
        break;
      case QgsLazDecoder::LazAttribute::Y:
        _storeToStream<qint32>( dataBuffer, outputOffset, requestedAttribute.type, isLas14 ? p14.y() : p10.y );
        break;
      case QgsLazDecoder::LazAttribute::Z:
        _storeToStream<qint32>( dataBuffer, outputOffset, requestedAttribute.type, isLas14 ? p14.z() : p10.z );
        break;
      case QgsLazDecoder::LazAttribute::Classification:
        _storeToStream<unsigned char>( dataBuffer, outputOffset, requestedAttribute.type, isLas14 ? p14.classification() : p10.classification );
        break;
      case QgsLazDecoder::LazAttribute::Intensity:
        _storeToStream<unsigned short>( dataBuffer, outputOffset, requestedAttribute.type, isLas14 ? p14.intensity() : p10.intensity );
        break;
      case QgsLazDecoder::LazAttribute::ReturnNumber:
        _storeToStream<unsigned char>( dataBuffer,  outputOffset, requestedAttribute.type, isLas14 ? p14.returnNum() : p10.return_number );
        break;
      case QgsLazDecoder::LazAttribute::NumberOfReturns:
        _storeToStream<unsigned char>( dataBuffer,  outputOffset, requestedAttribute.type, isLas14 ? p14.numReturns() : p10.number_of_returns_of_given_pulse );
        break;
      case QgsLazDecoder::LazAttribute::ScanDirectionFlag:
        _storeToStream<unsigned char>( dataBuffer, outputOffset, requestedAttribute.type, isLas14 ? p14.scanDirFlag() : p10.scan_direction_flag );
        break;
      case QgsLazDecoder::LazAttribute::EdgeOfFlightLine:
        _storeToStream<unsigned char>( dataBuffer, outputOffset, requestedAttribute.type, isLas14 ? p14.eofFlag() : p10.edge_of_flight_line );
        break;
      case QgsLazDecoder::LazAttribute::ScanAngleRank:
        _storeToStream<char>( dataBuffer, outputOffset, requestedAttribute.type, isLas14 ? p14.scanAngle() : p10.scan_angle_rank );
        break;
      case QgsLazDecoder::LazAttribute::UserData:
        _storeToStream<unsigned char>( dataBuffer, outputOffset, requestedAttribute.type, isLas14 ? p14.userData() : p10.user_data );
        break;
      case QgsLazDecoder::LazAttribute::PointSourceId:
        _storeToStream<unsigned short>( dataBuffer, outputOffset, requestedAttribute.type, isLas14 ? p14.pointSourceID() : p10.point_source_ID );
        break;
      case QgsLazDecoder::LazAttribute::GpsTime:
        // lazperf internally stores gps value as int64 field, but in fact it is a double value
        _storeToStream<double>( dataBuffer, outputOffset, requestedAttribute.type,
                                isLas14 ? p14.gpsTime() : *reinterpret_cast<const double *>( reinterpret_cast<const void *>( &gps.value ) ) );
        break;
      case QgsLazDecoder::LazAttribute::Red:
        _storeToStream<unsigned short>( dataBuffer, outputOffset, requestedAttribute.type, rgb.r );
        break;
      case QgsLazDecoder::LazAttribute::Green:
        _storeToStream<unsigned short>( dataBuffer, outputOffset, requestedAttribute.type, rgb.g );
        break;
      case QgsLazDecoder::LazAttribute::Blue:
        _storeToStream<unsigned short>( dataBuffer, outputOffset, requestedAttribute.type, rgb.b );
        break;
      case QgsLazDecoder::LazAttribute::ExtraBytes:
      {
        switch ( requestedAttribute.type )
        {
          case QgsPointCloudAttribute::Char:
            _storeToStream<char>( dataBuffer, outputOffset, requestedAttribute.type, *reinterpret_cast<char * >( &buf[requestedAttribute.offset] ) );
            break;
          case QgsPointCloudAttribute::UChar:
            _storeToStream<unsigned char>( dataBuffer, outputOffset, requestedAttribute.type, *reinterpret_cast<unsigned char * >( &buf[requestedAttribute.offset] ) );
            break;
          case QgsPointCloudAttribute::Short:
            _storeToStream<qint16>( dataBuffer, outputOffset, requestedAttribute.type, *reinterpret_cast<qint16 * >( &buf[requestedAttribute.offset] ) );
            break;
          case QgsPointCloudAttribute::UShort:
            _storeToStream<quint16>( dataBuffer, outputOffset, requestedAttribute.type, *reinterpret_cast<quint16 * >( &buf[requestedAttribute.offset] ) );
            break;
          case QgsPointCloudAttribute::Int32:
            _storeToStream<qint32>( dataBuffer, outputOffset, requestedAttribute.type, *reinterpret_cast<qint32 * >( &buf[requestedAttribute.offset] ) );
            break;
          case QgsPointCloudAttribute::UInt32:
            _storeToStream<quint32>( dataBuffer, outputOffset, requestedAttribute.type, *reinterpret_cast<quint32 * >( &buf[requestedAttribute.offset] ) );
            break;
          case QgsPointCloudAttribute::Int64:
            _storeToStream<qint64>( dataBuffer, outputOffset, requestedAttribute.type, *reinterpret_cast<qint64 * >( &buf[requestedAttribute.offset] ) );
            break;
          case QgsPointCloudAttribute::UInt64:
            _storeToStream<quint64>( dataBuffer, outputOffset, requestedAttribute.type, *reinterpret_cast<quint64 * >( &buf[requestedAttribute.offset] ) );
            break;
          case QgsPointCloudAttribute::Float:
            _storeToStream<float>( dataBuffer, outputOffset, requestedAttribute.type, *reinterpret_cast<float * >( &buf[requestedAttribute.offset] ) );
            break;
          case QgsPointCloudAttribute::Double:
            _storeToStream<double>( dataBuffer, outputOffset, requestedAttribute.type, *reinterpret_cast<double * >( &buf[requestedAttribute.offset] ) );
            break;
        }
      }
      break;
      case QgsLazDecoder::LazAttribute::MissingOrUnknown:
        // just store 0 for unknown/missing attributes
        _storeToStream<unsigned short>( dataBuffer, outputOffset, requestedAttribute.type, 0 );
        break;
    }

    outputOffset += requestedAttribute.size;
  }
}

template<typename FileType>
QgsPointCloudBlock *__decompressLaz( FileType &file, const QgsPointCloudAttributeCollection &attributes, const QgsPointCloudAttributeCollection &requestedAttributes, const QgsVector3D &_scale, const QgsVector3D &_offset, QgsPointCloudExpression &filterExpression )
{
  Q_UNUSED( attributes );
  Q_UNUSED( _scale );
  Q_UNUSED( _offset );

  if ( ! file.good() )
    return nullptr;

#ifdef QGISDEBUG
  QElapsedTimer t;
  t.start();
#endif

  lazperf::reader::generic_file f( file );

  // output file formats from entwine/untwine:
  // - older versions write LAZ 1.2 files with point formats 0, 1, 2 or 3
  // - newer versions write LAZ 1.4 files with point formats 6, 7 or 8

  int lasPointFormat = f.header().pointFormat();
  if ( lasPointFormat != 0 && lasPointFormat != 1 && lasPointFormat != 2 && lasPointFormat != 3 &&
       lasPointFormat != 6 && lasPointFormat != 7 && lasPointFormat != 8 )
  {
    QgsDebugMsg( QStringLiteral( "Unexpected point format record (%1) - only 0, 1, 2, 3, 6, 7, 8 are supported" ).arg( lasPointFormat ) );
    return nullptr;
  }

  const size_t count = f.header().point_count;
  const QgsVector3D scale( f.header().scale.x, f.header().scale.y, f.header().scale.z );
  const QgsVector3D offset( f.header().offset.x, f.header().offset.y, f.header().offset.z );

  QByteArray bufArray( f.header().point_record_length, 0 );
  char *buf = bufArray.data();

  const size_t requestedPointRecordSize = requestedAttributes.pointRecordSize();
  QByteArray data;
  data.resize( requestedPointRecordSize * count );
  char *dataBuffer = data.data();

  std::size_t outputOffset = 0;

  std::unique_ptr< QgsPointCloudBlock > block = std::make_unique< QgsPointCloudBlock >(
        count,
        requestedAttributes,
        data, scale, offset
      );

  int skippedPoints = 0;
  const bool filterIsValid = filterExpression.isValid();
  if ( !filterExpression.prepare( block.get() ) && filterIsValid )
  {
    // skip processing if the expression cannot be prepared
    block->setPointCount( 0 );
    return block.release();
  }

  lazperf::las::point10 p10;
  lazperf::las::gpstime gps;
  lazperf::las::rgb rgb;
  lazperf::las::point14 p14;

  std::vector< QgsLazDecoder::RequestedAttributeDetails > requestedAttributeDetails = __prepareRequestedAttributeDetails( file, requestedAttributes );

  for ( size_t i = 0 ; i < count ; i ++ )
  {
    f.readPoint( buf ); // read the point out

    decodePoint( buf, lasPointFormat, dataBuffer, outputOffset, p10, gps, rgb, p14, requestedAttributeDetails );

    // check if point needs to be filtered out
    if ( filterIsValid )
    {
      // we're always evaluating the last written point in the buffer
      double eval = filterExpression.evaluate( i - skippedPoints );
      if ( !eval || std::isnan( eval ) )
      {
        // if the point is filtered out, rewind the offset so the next point is written over it
        outputOffset -= requestedPointRecordSize;
        ++skippedPoints;
      }
    }
  }

#ifdef QGISDEBUG
  QgsDebugMsgLevel( QStringLiteral( "LAZ-PERF Read through the points in %1 seconds." ).arg( t.elapsed() / 1000. ), 2 );
#endif
  block->setPointCount( count - skippedPoints );
  return block.release();
}

QgsPointCloudBlock *QgsLazDecoder::decompressLaz( const QString &filename,
    const QgsPointCloudAttributeCollection &attributes,
    const QgsPointCloudAttributeCollection &requestedAttributes,
    const QgsVector3D &scale, const QgsVector3D &offset,
    QgsPointCloudExpression &filterExpression )
{
  const QByteArray arr = filename.toUtf8();
  std::ifstream file( arr.constData(), std::ios::binary );

  return __decompressLaz<std::ifstream>( file, attributes, requestedAttributes, scale, offset, filterExpression );
}

QgsPointCloudBlock *QgsLazDecoder::decompressLaz( const QByteArray &byteArrayData,
    const QgsPointCloudAttributeCollection &attributes,
    const QgsPointCloudAttributeCollection &requestedAttributes,
    const QgsVector3D &scale, const QgsVector3D &offset,
    QgsPointCloudExpression &filterExpression )
{
  std::istringstream file( byteArrayData.toStdString() );
  return __decompressLaz<std::istringstream>( file, attributes, requestedAttributes, scale, offset, filterExpression );
}


QgsPointCloudBlock *QgsLazDecoder::decompressCopc( const QString &filename, uint64_t blockOffset, uint64_t blockSize, int32_t pointCount, const QgsPointCloudAttributeCollection &attributes, const QgsPointCloudAttributeCollection &requestedAttributes, const QgsVector3D &_scale, const QgsVector3D &_offset, QgsPointCloudExpression &filterExpression )
{
  Q_UNUSED( attributes );
  Q_UNUSED( _scale );
  Q_UNUSED( _offset );
  std::ifstream file( filename.toStdString(), std::ios::binary );
  lazperf::reader::generic_file f( file );

  // output file formats from entwine/untwine:
  // - older versions write LAZ 1.2 files with point formats 0, 1, 2 or 3
  // - newer versions write LAZ 1.4 files with point formats 6, 7 or 8

  int lasPointFormat = f.header().pointFormat();
  if ( lasPointFormat != 0 && lasPointFormat != 1 && lasPointFormat != 2 && lasPointFormat != 3 &&
       lasPointFormat != 6 && lasPointFormat != 7 && lasPointFormat != 8 )
  {
    QgsDebugMsg( QStringLiteral( "Unexpected point format record (%1) - only 0, 1, 2, 3, 6, 7, 8 are supported" ).arg( lasPointFormat ) );
    return nullptr;
  }

  std::unique_ptr<char> data( new char[ blockSize ] );
  file.seekg( blockOffset );
  file.read( data.get(), blockSize );
  std::unique_ptr<char> decodedData( new char[ f.header().point_record_length ] );

  lazperf::reader::chunk_decompressor decompressor( f.header().pointFormat(), f.header().ebCount(), data.get() );

  const QgsVector3D hScale( f.header().scale.x, f.header().scale.y, f.header().scale.z );
  const QgsVector3D hOffset( f.header().offset.x, f.header().offset.y, f.header().offset.z );

  const size_t requestedPointRecordSize = requestedAttributes.pointRecordSize();
  QByteArray blockData;
  blockData.resize( requestedPointRecordSize * pointCount );
  char *dataBuffer = blockData.data();

  std::size_t outputOffset = 0;

  std::vector< RequestedAttributeDetails > requestedAttributeDetails = __prepareRequestedAttributeDetails( file, requestedAttributes );
  std::unique_ptr< QgsPointCloudBlock > block = std::make_unique< QgsPointCloudBlock >(
        pointCount,
        requestedAttributes,
        blockData, hScale, hOffset
      );

  int skippedPoints = 0;
  const bool filterIsValid = filterExpression.isValid();
  if ( !filterExpression.prepare( block.get() ) && filterIsValid )
  {
    // skip processing if the expression cannot be prepared
    block->setPointCount( 0 );
    return block.release();
  }

  lazperf::las::point10 p10;
  lazperf::las::gpstime gps;
  lazperf::las::rgb rgb;
  lazperf::las::point14 p14;

  for ( int i = 0 ; i < pointCount; ++i )
  {
    decompressor.decompress( decodedData.get() );
    char *buf = decodedData.get();

    decodePoint( buf, lasPointFormat, dataBuffer, outputOffset, p10, gps, rgb, p14, requestedAttributeDetails );

    // check if point needs to be filtered out
    if ( filterIsValid )
    {
      // we're always evaluating the last written point in the buffer
      double eval = filterExpression.evaluate( i - skippedPoints );
      if ( !eval || std::isnan( eval ) )
      {
        // if the point is filtered out, rewind the offset so the next point is written over it
        outputOffset -= requestedPointRecordSize;
        ++skippedPoints;
      }
    }
  }

  block->setPointCount( pointCount - skippedPoints );
  return block.release();
}

///@endcond
