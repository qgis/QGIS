/***************************************************************************
                         qgspointcloudrenderer.cpp
                         --------------------
    begin                : October 2020
    copyright            : (C) 2020 by Peter Petrik
    email                : zilolv at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgseptdecoder.h"
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
bool _storeToStream( char *s, size_t position, QgsPointCloudAttribute::DataType type, T value )
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

bool __serialize( char *data, size_t outputPosition, QgsPointCloudAttribute::DataType outputType,
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
      return _storeToStream<char>( data, outputPosition, outputType, val );
    }
    case QgsPointCloudAttribute::UChar:
    {
      const unsigned char val = *( input + inputPosition );
      return _storeToStream<unsigned char>( data, outputPosition, outputType, val );
    }
    case QgsPointCloudAttribute::Short:
    {
      const short val = *reinterpret_cast< const short * >( input + inputPosition );
      return _storeToStream<short>( data, outputPosition, outputType, val );
    }
    case QgsPointCloudAttribute::UShort:
    {
      const unsigned short val = *reinterpret_cast< const unsigned short * >( input + inputPosition );
      return _storeToStream<unsigned short>( data, outputPosition, outputType, val );
    }
    case QgsPointCloudAttribute::Int32:
    {
      const qint32 val = *reinterpret_cast<const qint32 * >( input + inputPosition );
      return _storeToStream<qint32>( data, outputPosition, outputType, val );
    }
    case QgsPointCloudAttribute::UInt32:
    {
      const quint32 val = *reinterpret_cast<const quint32 * >( input + inputPosition );
      return _storeToStream<quint32>( data, outputPosition, outputType, val );
    }
    case QgsPointCloudAttribute::Int64:
    {
      const qint64 val = *reinterpret_cast<const qint64 * >( input + inputPosition );
      return _storeToStream<qint64>( data, outputPosition, outputType, val );
    }
    case QgsPointCloudAttribute::UInt64:
    {
      const quint64 val = *reinterpret_cast<const quint64 * >( input + inputPosition );
      return _storeToStream<quint64>( data, outputPosition, outputType, val );
    }
    case QgsPointCloudAttribute::Float:
    {
      const float val = *reinterpret_cast< const float * >( input + inputPosition );
      return _storeToStream<float>( data, outputPosition, outputType, val );
    }
    case QgsPointCloudAttribute::Double:
    {
      const double val = *reinterpret_cast< const double * >( input + inputPosition );
      return _storeToStream<double>( data, outputPosition, outputType, val );
    }
  }
  return true;
}

// //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

QgsPointCloudBlock *_decompressBinary( const QByteArray &dataUncompressed, const QgsPointCloudAttributeCollection &attributes, const QgsPointCloudAttributeCollection &requestedAttributes, const QgsVector3D &scale, const QgsVector3D &offset, QgsPointCloudExpression &filterExpression )
{
  const std::size_t pointRecordSize = attributes.pointRecordSize( );
  const std::size_t requestedPointRecordSize = requestedAttributes.pointRecordSize();
  const int count = dataUncompressed.size() / pointRecordSize;
  QByteArray data;
  data.resize( requestedPointRecordSize * count );
  char *destinationBuffer = data.data();
  const char *s = dataUncompressed.data();

  const QVector<QgsPointCloudAttribute> requestedAttributesVector = requestedAttributes.attributes();

  // calculate input attributes and offsets once in advance

  struct AttributeData
  {
    AttributeData( int inputOffset, int inputSize, QgsPointCloudAttribute::DataType inputType, int requestedSize, QgsPointCloudAttribute::DataType requestedType )
      : inputOffset( inputOffset )
      , inputSize( inputSize )
      , inputType( inputType )
      , requestedSize( requestedSize )
      , requestedType( requestedType )
    {}

    int inputOffset;
    int inputSize;
    QgsPointCloudAttribute::DataType inputType;
    int requestedSize;
    QgsPointCloudAttribute::DataType requestedType;
  };

  std::vector< AttributeData > attributeData;
  attributeData.reserve( requestedAttributesVector.size() );
  for ( const QgsPointCloudAttribute &requestedAttribute : requestedAttributesVector )
  {
    int inputAttributeOffset;
    const QgsPointCloudAttribute *inputAttribute = attributes.find( requestedAttribute.name(), inputAttributeOffset );
    if ( !inputAttribute )
    {
      return nullptr;
    }
    attributeData.emplace_back( AttributeData( inputAttributeOffset, inputAttribute->size(), inputAttribute->type(),
                                requestedAttribute.size(), requestedAttribute.type() ) );
  }

  int skippedPoints = 0;
  std::unique_ptr< QgsPointCloudBlock > block = std::make_unique< QgsPointCloudBlock >(
        count,
        requestedAttributes,
        data, scale, offset
      );

  const bool filterIsValid = filterExpression.isValid();
  if ( !filterExpression.prepare( block.get() ) && filterIsValid )
  {
    // skip processing if the expression cannot be prepared
    block->setPointCount( 0 );
    return block.release();
  }

  // now loop through points
  size_t outputOffset = 0;
  for ( int i = 0; i < count; ++i )
  {
    for ( const AttributeData &attribute : attributeData )
    {
      __serialize( destinationBuffer, outputOffset,
                   attribute.requestedType, s,
                   attribute.inputType, attribute.inputSize, i * pointRecordSize + attribute.inputOffset );

      outputOffset += attribute.requestedSize;
    }

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
  block->setPointCount( count - skippedPoints );
  return block.release();
}

QgsPointCloudBlock *QgsEptDecoder::decompressBinary( const QString &filename, const QgsPointCloudAttributeCollection &attributes, const QgsPointCloudAttributeCollection &requestedAttributes, const QgsVector3D &scale, const QgsVector3D &offset, QgsPointCloudExpression &filterExpression )
{
  if ( ! QFile::exists( filename ) )
    return nullptr;

  QFile f( filename );
  const bool r = f.open( QIODevice::ReadOnly );
  if ( !r )
    return nullptr;

  const QByteArray dataUncompressed = f.read( f.size() );
  return _decompressBinary( dataUncompressed, attributes, requestedAttributes, scale, offset, filterExpression );
}

QgsPointCloudBlock *QgsEptDecoder::decompressBinary( const QByteArray &data, const QgsPointCloudAttributeCollection &attributes, const QgsPointCloudAttributeCollection &requestedAttributes, const QgsVector3D &scale, const QgsVector3D &offset, QgsPointCloudExpression &filterExpression )
{
  return _decompressBinary( data, attributes, requestedAttributes, scale, offset, filterExpression );
}

/* *************************************************************************************** */

QByteArray decompressZtdStream( const QByteArray &dataCompressed )
{
  // NOTE: this is very primitive implementation because we expect the uncompressed
  // data will be always less than 10 MB

  const int MAXSIZE = 10000000;
  QByteArray dataUncompressed;
  dataUncompressed.resize( MAXSIZE );

  ZSTD_DStream *strm = ZSTD_createDStream();
  ZSTD_initDStream( strm );

  ZSTD_inBuffer m_inBuf;
  m_inBuf.src = reinterpret_cast<const void *>( dataCompressed.constData() );
  m_inBuf.size = dataCompressed.size();
  m_inBuf.pos = 0;

  ZSTD_outBuffer outBuf { reinterpret_cast<void *>( dataUncompressed.data() ), MAXSIZE, 0 };
  const size_t ret = ZSTD_decompressStream( strm, &outBuf, &m_inBuf );
  Q_ASSERT( !ZSTD_isError( ret ) );
  Q_ASSERT( outBuf.pos );
  Q_ASSERT( outBuf.pos < outBuf.size );

  ZSTD_freeDStream( strm );
  dataUncompressed.resize( outBuf.pos );
  return dataUncompressed;
}

QgsPointCloudBlock *QgsEptDecoder::decompressZStandard( const QString &filename, const QgsPointCloudAttributeCollection &attributes, const QgsPointCloudAttributeCollection &requestedAttributes, const QgsVector3D &scale, const QgsVector3D &offset, QgsPointCloudExpression &filterExpression )
{
  if ( ! QFile::exists( filename ) )
    return nullptr;

  QFile f( filename );
  const bool r = f.open( QIODevice::ReadOnly );
  if ( !r )
    return nullptr;

  const QByteArray dataCompressed = f.readAll();
  const QByteArray dataUncompressed = decompressZtdStream( dataCompressed );
  return _decompressBinary( dataUncompressed, attributes, requestedAttributes, scale, offset, filterExpression );
}

QgsPointCloudBlock *QgsEptDecoder::decompressZStandard( const QByteArray &data, const QgsPointCloudAttributeCollection &attributes, const QgsPointCloudAttributeCollection &requestedAttributes, const QgsVector3D &scale, const QgsVector3D &offset, QgsPointCloudExpression &filterExpression )
{
  const QByteArray dataUncompressed = decompressZtdStream( data );
  return _decompressBinary( dataUncompressed, attributes, requestedAttributes, scale, offset, filterExpression );
}

/* *************************************************************************************** */


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

  const size_t count = f.header().point_count;
  const QgsVector3D scale( f.header().scale.x, f.header().scale.y, f.header().scale.z );
  const QgsVector3D offset( f.header().offset.x, f.header().offset.y, f.header().offset.z );

  QByteArray bufArray( f.header().point_record_length, 0 );
  char *buf = bufArray.data();

  const size_t requestedPointRecordSize = requestedAttributes.pointRecordSize();
  QByteArray data;
  data.resize( requestedPointRecordSize * count );
  char *dataBuffer = data.data();

  const QVector<QgsPointCloudAttribute> requestedAttributesVector = requestedAttributes.attributes();

  std::size_t outputOffset = 0;

  enum class LazAttribute
  {
    X,
    Y,
    Z,
    Classification,
    Intensity,
    ReturnNumber,
    NumberOfReturns,
    ScanDirectionFlag,
    EdgeOfFlightLine,
    ScanAngleRank,
    UserData,
    PointSourceId,
    GpsTime,
    Red,
    Green,
    Blue,
    ExtraBytes,
    MissingOrUnknown
  };

  struct RequestedAttributeDetails
  {
    RequestedAttributeDetails( LazAttribute attribute, QgsPointCloudAttribute::DataType type, int size, int offset = -1 )
      : attribute( attribute )
      , type( type )
      , size( size )
      , offset( offset )
    {}

    LazAttribute attribute;
    QgsPointCloudAttribute::DataType type;
    int size;
    int offset; // Used in case the attribute is an extra byte attribute
  };

  QVector<QgsEptDecoder::ExtraBytesAttributeDetails> extrabytesAttr = QgsEptDecoder::readExtraByteAttributes<FileType>( file );

  std::vector< RequestedAttributeDetails > requestedAttributeDetails;
  requestedAttributeDetails.reserve( requestedAttributesVector.size() );
  for ( const QgsPointCloudAttribute &requestedAttribute : requestedAttributesVector )
  {
    if ( requestedAttribute.name().compare( QLatin1String( "X" ), Qt::CaseInsensitive ) == 0 )
    {
      requestedAttributeDetails.emplace_back( RequestedAttributeDetails( LazAttribute::X, requestedAttribute.type(), requestedAttribute.size() ) );
    }
    else if ( requestedAttribute.name().compare( QLatin1String( "Y" ), Qt::CaseInsensitive ) == 0 )
    {
      requestedAttributeDetails.emplace_back( RequestedAttributeDetails( LazAttribute::Y, requestedAttribute.type(), requestedAttribute.size() ) );
    }
    else if ( requestedAttribute.name().compare( QLatin1String( "Z" ), Qt::CaseInsensitive ) == 0 )
    {
      requestedAttributeDetails.emplace_back( RequestedAttributeDetails( LazAttribute::Z, requestedAttribute.type(), requestedAttribute.size() ) );
    }
    else if ( requestedAttribute.name().compare( QLatin1String( "Classification" ), Qt::CaseInsensitive ) == 0 )
    {
      requestedAttributeDetails.emplace_back( RequestedAttributeDetails( LazAttribute::Classification, requestedAttribute.type(), requestedAttribute.size() ) );
    }
    else if ( requestedAttribute.name().compare( QLatin1String( "Intensity" ), Qt::CaseInsensitive ) == 0 )
    {
      requestedAttributeDetails.emplace_back( RequestedAttributeDetails( LazAttribute::Intensity, requestedAttribute.type(), requestedAttribute.size() ) );
    }
    else if ( requestedAttribute.name().compare( QLatin1String( "ReturnNumber" ), Qt::CaseInsensitive ) == 0 )
    {
      requestedAttributeDetails.emplace_back( RequestedAttributeDetails( LazAttribute::ReturnNumber, requestedAttribute.type(), requestedAttribute.size() ) );
    }
    else if ( requestedAttribute.name().compare( QLatin1String( "NumberOfReturns" ), Qt::CaseInsensitive ) == 0 )
    {
      requestedAttributeDetails.emplace_back( RequestedAttributeDetails( LazAttribute::NumberOfReturns, requestedAttribute.type(), requestedAttribute.size() ) );
    }
    else if ( requestedAttribute.name().compare( QLatin1String( "ScanDirectionFlag" ), Qt::CaseInsensitive ) == 0 )
    {
      requestedAttributeDetails.emplace_back( RequestedAttributeDetails( LazAttribute::ScanDirectionFlag, requestedAttribute.type(), requestedAttribute.size() ) );
    }
    else if ( requestedAttribute.name().compare( QLatin1String( "EdgeOfFlightLine" ), Qt::CaseInsensitive ) == 0 )
    {
      requestedAttributeDetails.emplace_back( RequestedAttributeDetails( LazAttribute::EdgeOfFlightLine, requestedAttribute.type(), requestedAttribute.size() ) );
    }
    else if ( requestedAttribute.name().compare( QLatin1String( "ScanAngleRank" ), Qt::CaseInsensitive ) == 0 )
    {
      requestedAttributeDetails.emplace_back( RequestedAttributeDetails( LazAttribute::ScanAngleRank, requestedAttribute.type(), requestedAttribute.size() ) );
    }
    else if ( requestedAttribute.name().compare( QLatin1String( "UserData" ), Qt::CaseInsensitive ) == 0 )
    {
      requestedAttributeDetails.emplace_back( RequestedAttributeDetails( LazAttribute::UserData, requestedAttribute.type(), requestedAttribute.size() ) );
    }
    else if ( requestedAttribute.name().compare( QLatin1String( "PointSourceId" ), Qt::CaseInsensitive ) == 0 )
    {
      requestedAttributeDetails.emplace_back( RequestedAttributeDetails( LazAttribute::PointSourceId, requestedAttribute.type(), requestedAttribute.size() ) );
    }
    else if ( requestedAttribute.name().compare( QLatin1String( "GpsTime" ), Qt::CaseInsensitive ) == 0 )
    {
      requestedAttributeDetails.emplace_back( RequestedAttributeDetails( LazAttribute::GpsTime, requestedAttribute.type(), requestedAttribute.size() ) );
    }
    else if ( requestedAttribute.name().compare( QLatin1String( "Red" ), Qt::CaseInsensitive ) == 0 )
    {
      requestedAttributeDetails.emplace_back( RequestedAttributeDetails( LazAttribute::Red, requestedAttribute.type(), requestedAttribute.size() ) );
    }
    else if ( requestedAttribute.name().compare( QLatin1String( "Green" ), Qt::CaseInsensitive ) == 0 )
    {
      requestedAttributeDetails.emplace_back( RequestedAttributeDetails( LazAttribute::Green, requestedAttribute.type(), requestedAttribute.size() ) );
    }
    else if ( requestedAttribute.name().compare( QLatin1String( "Blue" ), Qt::CaseInsensitive ) == 0 )
    {
      requestedAttributeDetails.emplace_back( RequestedAttributeDetails( LazAttribute::Blue, requestedAttribute.type(), requestedAttribute.size() ) );
    }
    else
    {
      bool foundAttr = false;
      for ( QgsEptDecoder::ExtraBytesAttributeDetails &eba : extrabytesAttr )
      {
        if ( requestedAttribute.name().compare( eba.attribute.trimmed() ) == 0 )
        {
          requestedAttributeDetails.emplace_back( RequestedAttributeDetails( LazAttribute::ExtraBytes, eba.type, eba.size, eba.offset ) );
          foundAttr = true;
          break;
        }
      }
      if ( !foundAttr )
      {
        // this can possibly happen -- e.g. if a style built using a different point cloud format references an attribute which isn't available from the laz file
        requestedAttributeDetails.emplace_back( RequestedAttributeDetails( LazAttribute::MissingOrUnknown, requestedAttribute.type(), requestedAttribute.size() ) );
      }
    }
  }

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

  lazperf::las::point10 p;
  lazperf::las::gpstime gps;
  lazperf::las::rgb rgb;

  for ( size_t i = 0 ; i < count ; i ++ )
  {
    f.readPoint( buf ); // read the point out
    p.unpack( buf );
    gps.unpack( buf + sizeof( lazperf::las::point10 ) );
    rgb.unpack( buf + sizeof( lazperf::las::point10 ) + sizeof( lazperf::las::gpstime ) );


    for ( const RequestedAttributeDetails &requestedAttribute : requestedAttributeDetails )
    {
      switch ( requestedAttribute.attribute )
      {
        case LazAttribute::X:
          _storeToStream<qint32>( dataBuffer, outputOffset, requestedAttribute.type, p.x );
          break;
        case LazAttribute::Y:
          _storeToStream<qint32>( dataBuffer, outputOffset, requestedAttribute.type, p.y );
          break;
        case LazAttribute::Z:
          _storeToStream<qint32>( dataBuffer, outputOffset, requestedAttribute.type, p.z );
          break;
        case LazAttribute::Classification:
          _storeToStream<unsigned char>( dataBuffer, outputOffset, requestedAttribute.type, p.classification );
          break;
        case LazAttribute::Intensity:
          _storeToStream<unsigned short>( dataBuffer, outputOffset, requestedAttribute.type, p.intensity );
          break;
        case LazAttribute::ReturnNumber:
          _storeToStream<unsigned char>( dataBuffer,  outputOffset, requestedAttribute.type, p.return_number );
          break;
        case LazAttribute::NumberOfReturns:
          _storeToStream<unsigned char>( dataBuffer,  outputOffset, requestedAttribute.type, p.number_of_returns_of_given_pulse );
          break;
        case LazAttribute::ScanDirectionFlag:
          _storeToStream<unsigned char>( dataBuffer, outputOffset, requestedAttribute.type, p.scan_direction_flag );
          break;
        case LazAttribute::EdgeOfFlightLine:
          _storeToStream<unsigned char>( dataBuffer, outputOffset, requestedAttribute.type, p.edge_of_flight_line );
          break;
        case LazAttribute::ScanAngleRank:
          _storeToStream<char>( dataBuffer, outputOffset, requestedAttribute.type, p.scan_angle_rank );
          break;
        case LazAttribute::UserData:
          _storeToStream<unsigned char>( dataBuffer, outputOffset, requestedAttribute.type, p.user_data );
          break;
        case LazAttribute::PointSourceId:
          _storeToStream<unsigned short>( dataBuffer, outputOffset, requestedAttribute.type, p.point_source_ID );
          break;
        case LazAttribute::GpsTime:
          // lazperf internally stores gps value as int64 field, but in fact it is a double value
          _storeToStream<double>( dataBuffer, outputOffset, requestedAttribute.type,
                                  *reinterpret_cast<const double *>( reinterpret_cast<const void *>( &gps.value ) ) );
          break;
        case LazAttribute::Red:
          _storeToStream<unsigned short>( dataBuffer, outputOffset, requestedAttribute.type, rgb.r );
          break;
        case LazAttribute::Green:
          _storeToStream<unsigned short>( dataBuffer, outputOffset, requestedAttribute.type, rgb.g );
          break;
        case LazAttribute::Blue:
          _storeToStream<unsigned short>( dataBuffer, outputOffset, requestedAttribute.type, rgb.b );
          break;
        case LazAttribute::ExtraBytes:
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
        case LazAttribute::MissingOrUnknown:
          // just store 0 for unknown/missing attributes
          _storeToStream<unsigned short>( dataBuffer, outputOffset, requestedAttribute.type, 0 );
          break;
      }

      outputOffset += requestedAttribute.size;
    }

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

QgsPointCloudBlock *QgsEptDecoder::decompressLaz( const QString &filename,
    const QgsPointCloudAttributeCollection &attributes,
    const QgsPointCloudAttributeCollection &requestedAttributes,
    const QgsVector3D &scale, const QgsVector3D &offset,
    QgsPointCloudExpression &filterExpression )
{
  const QByteArray arr = filename.toUtf8();
  std::ifstream file( arr.constData(), std::ios::binary );

  return __decompressLaz<std::ifstream>( file, attributes, requestedAttributes, scale, offset, filterExpression );
}

QgsPointCloudBlock *QgsEptDecoder::decompressLaz( const QByteArray &byteArrayData,
    const QgsPointCloudAttributeCollection &attributes,
    const QgsPointCloudAttributeCollection &requestedAttributes,
    const QgsVector3D &scale, const QgsVector3D &offset,
    QgsPointCloudExpression &filterExpression )
{
  std::istringstream file( byteArrayData.toStdString() );
  return __decompressLaz<std::istringstream>( file, attributes, requestedAttributes, scale, offset, filterExpression );
}

///@endcond
