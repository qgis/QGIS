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
#include "qgslazinfo.h"

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
#include <lazperf/lazperf.hpp>

#if defined(_MSC_VER)
#define UNICODE
#include <locale>
#include <codecvt>
#endif

///@cond PRIVATE

template <typename T>
bool _lazStoreToStream( char *s, size_t position, QgsPointCloudAttribute::DataType type, T value )
{
  switch ( type )
  {
    case QgsPointCloudAttribute::Char:
    {
      const char val = char( value );
      s[position] = val;
      break;
    }
    case QgsPointCloudAttribute::UChar:
    {
      const unsigned char val = ( unsigned char )( value );
      s[position] = val;
      break;
    }

    case QgsPointCloudAttribute::Short:
    {
      short val = short( value );
      memcpy( s + position, reinterpret_cast<char * >( &val ), sizeof( short ) );
      break;
    }
    case QgsPointCloudAttribute::UShort:
    {
      unsigned short val = static_cast< unsigned short>( value );
      memcpy( s + position, reinterpret_cast< char * >( &val ), sizeof( unsigned short ) );
      break;
    }

    case QgsPointCloudAttribute::Int32:
    {
      qint32 val = qint32( value );
      memcpy( s + position, reinterpret_cast< char * >( &val ), sizeof( qint32 ) );
      break;
    }
    case QgsPointCloudAttribute::UInt32:
    {
      quint32 val = quint32( value );
      memcpy( s + position, reinterpret_cast< char * >( &val ), sizeof( quint32 ) );
      break;
    }

    case QgsPointCloudAttribute::Int64:
    {
      qint64 val = qint64( value );
      memcpy( s + position, reinterpret_cast< char * >( &val ), sizeof( qint64 ) );
      break;
    }
    case QgsPointCloudAttribute::UInt64:
    {
      quint64 val = quint64( value );
      memcpy( s + position, reinterpret_cast< char * >( &val ), sizeof( quint64 ) );
      break;
    }

    case QgsPointCloudAttribute::Float:
    {
      float val = float( value );
      memcpy( s + position, reinterpret_cast< char * >( &val ),  sizeof( float ) );
      break;
    }
    case QgsPointCloudAttribute::Double:
    {
      double val = double( value );
      memcpy( s + position, reinterpret_cast< char * >( &val ), sizeof( double ) );
      break;
    }
  }

  return true;
}

bool _lazSerialize( char *data, size_t outputPosition, QgsPointCloudAttribute::DataType outputType,
                    const char *input, QgsPointCloudAttribute::DataType inputType, int inputSize, size_t inputPosition )
{
  if ( outputType == inputType )
  {
    memcpy( data + outputPosition, input + inputPosition, inputSize );
    return true;
  }

  switch ( inputType )
  {
    case QgsPointCloudAttribute::Char:
    {
      const char val = *( input + inputPosition );
      return _lazStoreToStream<char>( data, outputPosition, outputType, val );
    }
    case QgsPointCloudAttribute::UChar:
    {
      const unsigned char val = *( input + inputPosition );
      return _lazStoreToStream<unsigned char>( data, outputPosition, outputType, val );
    }
    case QgsPointCloudAttribute::Short:
    {
      const short val = *reinterpret_cast< const short * >( input + inputPosition );
      return _lazStoreToStream<short>( data, outputPosition, outputType, val );
    }
    case QgsPointCloudAttribute::UShort:
    {
      const unsigned short val = *reinterpret_cast< const unsigned short * >( input + inputPosition );
      return _lazStoreToStream<unsigned short>( data, outputPosition, outputType, val );
    }
    case QgsPointCloudAttribute::Int32:
    {
      const qint32 val = *reinterpret_cast<const qint32 * >( input + inputPosition );
      return _lazStoreToStream<qint32>( data, outputPosition, outputType, val );
    }
    case QgsPointCloudAttribute::UInt32:
    {
      const quint32 val = *reinterpret_cast<const quint32 * >( input + inputPosition );
      return _lazStoreToStream<quint32>( data, outputPosition, outputType, val );
    }
    case QgsPointCloudAttribute::Int64:
    {
      const qint64 val = *reinterpret_cast<const qint64 * >( input + inputPosition );
      return _lazStoreToStream<qint64>( data, outputPosition, outputType, val );
    }
    case QgsPointCloudAttribute::UInt64:
    {
      const quint64 val = *reinterpret_cast<const quint64 * >( input + inputPosition );
      return _lazStoreToStream<quint64>( data, outputPosition, outputType, val );
    }
    case QgsPointCloudAttribute::Float:
    {
      const float val = *reinterpret_cast< const float * >( input + inputPosition );
      return _lazStoreToStream<float>( data, outputPosition, outputType, val );
    }
    case QgsPointCloudAttribute::Double:
    {
      const double val = *reinterpret_cast< const double * >( input + inputPosition );
      return _lazStoreToStream<double>( data, outputPosition, outputType, val );
    }
  }
  return true;
}

// //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::vector< QgsLazDecoder::RequestedAttributeDetails > __prepareRequestedAttributeDetails( const QgsPointCloudAttributeCollection &requestedAttributes, QVector<QgsLazInfo::ExtraBytesAttributeDetails> &extrabytesAttr )
{
  const QVector<QgsPointCloudAttribute> requestedAttributesVector = requestedAttributes.attributes();

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
    else if ( requestedAttribute.name().compare( QLatin1String( "ScannerChannel" ), Qt::CaseInsensitive ) == 0 )
    {
      requestedAttributeDetails.emplace_back( QgsLazDecoder::RequestedAttributeDetails( QgsLazDecoder::LazAttribute::ScannerChannel, requestedAttribute.type(), requestedAttribute.size() ) );
    }
    else if ( requestedAttribute.name().compare( QLatin1String( "ClassificationFlags" ), Qt::CaseInsensitive ) == 0 )
    {
      requestedAttributeDetails.emplace_back( QgsLazDecoder::RequestedAttributeDetails( QgsLazDecoder::LazAttribute::ClassificationFlags, requestedAttribute.type(), requestedAttribute.size() ) );
    }
    else if ( requestedAttribute.name().compare( QLatin1String( "Infrared" ), Qt::CaseInsensitive ) == 0 )
    {
      requestedAttributeDetails.emplace_back( QgsLazDecoder::RequestedAttributeDetails( QgsLazDecoder::LazAttribute::NIR, requestedAttribute.type(), requestedAttribute.size() ) );
    }
    else
    {
      bool foundAttr = false;
      for ( QgsLazInfo::ExtraBytesAttributeDetails &eba : extrabytesAttr )
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

void decodePoint( char *buf, int lasPointFormat, char *dataBuffer, std::size_t &outputOffset, std::vector< QgsLazDecoder::RequestedAttributeDetails > &requestedAttributeDetails )
{
  lazperf::las::point10 p10;
  lazperf::las::gpstime gps;
  lazperf::las::rgb rgb;
  lazperf::las::nir14 nir;
  lazperf::las::point14 p14;

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
      nir.unpack( buf + sizeof( lazperf::las::point14 ) + sizeof( lazperf::las::rgb ) );
      break;

    default:
      Q_ASSERT( false );  // must not happen - we checked earlier that the format is supported
  }

  for ( const QgsLazDecoder::RequestedAttributeDetails &requestedAttribute : requestedAttributeDetails )
  {
    switch ( requestedAttribute.attribute )
    {
      case QgsLazDecoder::LazAttribute::X:
        _lazStoreToStream<qint32>( dataBuffer, outputOffset, requestedAttribute.type, isLas14 ? p14.x() : p10.x );
        break;
      case QgsLazDecoder::LazAttribute::Y:
        _lazStoreToStream<qint32>( dataBuffer, outputOffset, requestedAttribute.type, isLas14 ? p14.y() : p10.y );
        break;
      case QgsLazDecoder::LazAttribute::Z:
        _lazStoreToStream<qint32>( dataBuffer, outputOffset, requestedAttribute.type, isLas14 ? p14.z() : p10.z );
        break;
      case QgsLazDecoder::LazAttribute::Classification:
        _lazStoreToStream<unsigned char>( dataBuffer, outputOffset, requestedAttribute.type, isLas14 ? p14.classification() : p10.classification );
        break;
      case QgsLazDecoder::LazAttribute::Intensity:
        _lazStoreToStream<unsigned short>( dataBuffer, outputOffset, requestedAttribute.type, isLas14 ? p14.intensity() : p10.intensity );
        break;
      case QgsLazDecoder::LazAttribute::ReturnNumber:
        _lazStoreToStream<unsigned char>( dataBuffer,  outputOffset, requestedAttribute.type, isLas14 ? p14.returnNum() : p10.return_number );
        break;
      case QgsLazDecoder::LazAttribute::NumberOfReturns:
        _lazStoreToStream<unsigned char>( dataBuffer,  outputOffset, requestedAttribute.type, isLas14 ? p14.numReturns() : p10.number_of_returns_of_given_pulse );
        break;
      case QgsLazDecoder::LazAttribute::ScanDirectionFlag:
        _lazStoreToStream<unsigned char>( dataBuffer, outputOffset, requestedAttribute.type, isLas14 ? p14.scanDirFlag() : p10.scan_direction_flag );
        break;
      case QgsLazDecoder::LazAttribute::EdgeOfFlightLine:
        _lazStoreToStream<unsigned char>( dataBuffer, outputOffset, requestedAttribute.type, isLas14 ? p14.eofFlag() : p10.edge_of_flight_line );
        break;
      case QgsLazDecoder::LazAttribute::ScanAngleRank:
        _lazStoreToStream<char>( dataBuffer, outputOffset, requestedAttribute.type, isLas14 ? p14.scanAngle() : p10.scan_angle_rank );
        break;
      case QgsLazDecoder::LazAttribute::UserData:
        _lazStoreToStream<unsigned char>( dataBuffer, outputOffset, requestedAttribute.type, isLas14 ? p14.userData() : p10.user_data );
        break;
      case QgsLazDecoder::LazAttribute::PointSourceId:
        _lazStoreToStream<unsigned short>( dataBuffer, outputOffset, requestedAttribute.type, isLas14 ? p14.pointSourceID() : p10.point_source_ID );
        break;
      case QgsLazDecoder::LazAttribute::GpsTime:
        // lazperf internally stores gps value as int64 field, but in fact it is a double value
        _lazStoreToStream<double>( dataBuffer, outputOffset, requestedAttribute.type,
                                   isLas14 ? p14.gpsTime() : *reinterpret_cast<const double *>( reinterpret_cast<const void *>( &gps.value ) ) );
        break;
      case QgsLazDecoder::LazAttribute::Red:
        _lazStoreToStream<unsigned short>( dataBuffer, outputOffset, requestedAttribute.type, rgb.r );
        break;
      case QgsLazDecoder::LazAttribute::Green:
        _lazStoreToStream<unsigned short>( dataBuffer, outputOffset, requestedAttribute.type, rgb.g );
        break;
      case QgsLazDecoder::LazAttribute::Blue:
        _lazStoreToStream<unsigned short>( dataBuffer, outputOffset, requestedAttribute.type, rgb.b );
        break;
      case QgsLazDecoder::LazAttribute::ScannerChannel:
        _lazStoreToStream<char>( dataBuffer, outputOffset, requestedAttribute.type, p14.scannerChannel() );
        break;
      case QgsLazDecoder::LazAttribute::ClassificationFlags:
        _lazStoreToStream<char>( dataBuffer, outputOffset, requestedAttribute.type, p14.classFlags() );
        break;
      case QgsLazDecoder::LazAttribute::NIR:
      {
        if ( lasPointFormat == 8 || lasPointFormat == 10 )
        {
          _lazStoreToStream<unsigned short>( dataBuffer, outputOffset, requestedAttribute.type, nir.val );
        }
        else
        {
          // just store 0 for missing attributes
          _lazStoreToStream<unsigned short>( dataBuffer, outputOffset, requestedAttribute.type, 0 );
        }
        break;

      }
      case QgsLazDecoder::LazAttribute::ExtraBytes:
      {
        switch ( requestedAttribute.type )
        {
          case QgsPointCloudAttribute::Char:
            _lazStoreToStream<char>( dataBuffer, outputOffset, requestedAttribute.type, *reinterpret_cast<char * >( &buf[requestedAttribute.offset] ) );
            break;
          case QgsPointCloudAttribute::UChar:
            _lazStoreToStream<unsigned char>( dataBuffer, outputOffset, requestedAttribute.type, *reinterpret_cast<unsigned char * >( &buf[requestedAttribute.offset] ) );
            break;
          case QgsPointCloudAttribute::Short:
            _lazStoreToStream<qint16>( dataBuffer, outputOffset, requestedAttribute.type, *reinterpret_cast<qint16 * >( &buf[requestedAttribute.offset] ) );
            break;
          case QgsPointCloudAttribute::UShort:
            _lazStoreToStream<quint16>( dataBuffer, outputOffset, requestedAttribute.type, *reinterpret_cast<quint16 * >( &buf[requestedAttribute.offset] ) );
            break;
          case QgsPointCloudAttribute::Int32:
            _lazStoreToStream<qint32>( dataBuffer, outputOffset, requestedAttribute.type, *reinterpret_cast<qint32 * >( &buf[requestedAttribute.offset] ) );
            break;
          case QgsPointCloudAttribute::UInt32:
            _lazStoreToStream<quint32>( dataBuffer, outputOffset, requestedAttribute.type, *reinterpret_cast<quint32 * >( &buf[requestedAttribute.offset] ) );
            break;
          case QgsPointCloudAttribute::Int64:
            _lazStoreToStream<qint64>( dataBuffer, outputOffset, requestedAttribute.type, *reinterpret_cast<qint64 * >( &buf[requestedAttribute.offset] ) );
            break;
          case QgsPointCloudAttribute::UInt64:
            _lazStoreToStream<quint64>( dataBuffer, outputOffset, requestedAttribute.type, *reinterpret_cast<quint64 * >( &buf[requestedAttribute.offset] ) );
            break;
          case QgsPointCloudAttribute::Float:
            _lazStoreToStream<float>( dataBuffer, outputOffset, requestedAttribute.type, *reinterpret_cast<float * >( &buf[requestedAttribute.offset] ) );
            break;
          case QgsPointCloudAttribute::Double:
            _lazStoreToStream<double>( dataBuffer, outputOffset, requestedAttribute.type, *reinterpret_cast<double * >( &buf[requestedAttribute.offset] ) );
            break;
        }
      }
      break;
      case QgsLazDecoder::LazAttribute::MissingOrUnknown:
        // just store 0 for unknown/missing attributes
        _lazStoreToStream<unsigned short>( dataBuffer, outputOffset, requestedAttribute.type, 0 );
        break;
    }

    outputOffset += requestedAttribute.size;
  }
}

template<typename FileType>
QgsPointCloudBlock *__decompressLaz( FileType &file, const QgsPointCloudAttributeCollection &requestedAttributes, QgsPointCloudExpression &filterExpression )
{
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

  std::vector<char> rawExtrabytes = f.vlrData( "LASF_Spec", 4 );
  QVector<QgsLazInfo::ExtraBytesAttributeDetails> extrabyteAttributesDetails = QgsLazInfo::parseExtrabytes( rawExtrabytes.data(), rawExtrabytes.size(), f.header().point_record_length );
  std::vector< QgsLazDecoder::RequestedAttributeDetails > requestedAttributeDetails = __prepareRequestedAttributeDetails( requestedAttributes, extrabyteAttributesDetails );

  for ( size_t i = 0 ; i < count ; i ++ )
  {
    f.readPoint( buf ); // read the point out

    decodePoint( buf, lasPointFormat, dataBuffer, outputOffset, requestedAttributeDetails );

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
    const QgsPointCloudAttributeCollection &requestedAttributes,
    QgsPointCloudExpression &filterExpression )
{
  std::ifstream file( toNativePath( filename ), std::ios::binary );

  return __decompressLaz<std::ifstream>( file, requestedAttributes, filterExpression );
}

QgsPointCloudBlock *QgsLazDecoder::decompressLaz( const QByteArray &byteArrayData,
    const QgsPointCloudAttributeCollection &requestedAttributes,
    QgsPointCloudExpression &filterExpression )
{
  std::istringstream file( byteArrayData.toStdString() );
  return __decompressLaz<std::istringstream>( file, requestedAttributes, filterExpression );
}

QgsPointCloudBlock *QgsLazDecoder::decompressCopc( const QByteArray &data, QgsLazInfo &lazInfo, int32_t pointCount, const QgsPointCloudAttributeCollection &requestedAttributes, QgsPointCloudExpression &filterExpression )
{
  // COPC only supports point formats 6, 7 and 8
  int lasPointFormat = lazInfo.pointFormat();
  if ( lasPointFormat != 6 && lasPointFormat != 7 && lasPointFormat != 8 )
  {
    QgsDebugMsg( QStringLiteral( "Unexpected point format record (%1) - only 6, 7, 8 are supported for COPC format" ).arg( lasPointFormat ) );
    return nullptr;
  }

  std::unique_ptr<char []> decodedData( new char[ lazInfo.pointRecordLength() ] );

  lazperf::reader::chunk_decompressor decompressor( lasPointFormat, lazInfo.extrabytesCount(), data.data() );

  const size_t requestedPointRecordSize = requestedAttributes.pointRecordSize();
  QByteArray blockData;
  blockData.resize( requestedPointRecordSize * pointCount );
  char *dataBuffer = blockData.data();

  std::size_t outputOffset = 0;

  QVector<QgsLazInfo::ExtraBytesAttributeDetails> extrabyteAttributesDetails = lazInfo.extrabytes();
  std::vector< RequestedAttributeDetails > requestedAttributeDetails = __prepareRequestedAttributeDetails( requestedAttributes, extrabyteAttributesDetails );
  std::unique_ptr< QgsPointCloudBlock > block = std::make_unique< QgsPointCloudBlock >(
        pointCount, requestedAttributes,
        blockData, lazInfo.scale(), lazInfo.offset()
      );

  int skippedPoints = 0;
  const bool filterIsValid = filterExpression.isValid();
  if ( !filterExpression.prepare( block.get() ) && filterIsValid )
  {
    // skip processing if the expression cannot be prepared
    block->setPointCount( 0 );
    return block.release();
  }

  for ( int i = 0 ; i < pointCount; ++i )
  {
    decompressor.decompress( decodedData.get() );
    char *buf = decodedData.get();

    decodePoint( buf, lasPointFormat, dataBuffer, outputOffset, requestedAttributeDetails );

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

#if defined(_MSC_VER)
std::wstring QgsLazDecoder::toNativePath( const QString &filename )
{
  std::wstring_convert< std::codecvt_utf8_utf16< wchar_t > > converter;
  return converter.from_bytes( filename.toStdString() );
}
#else
std::string QgsLazDecoder::toNativePath( const QString &filename )
{
  return filename.toStdString();
}
#endif

///@endcond
