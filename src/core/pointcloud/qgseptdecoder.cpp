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
#include <QTemporaryFile>
#include <string>

#include <zstd.h>

#include "laz-perf/io.hpp"
#include "laz-perf/common/common.hpp"

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

    case QgsPointCloudAttribute::Float:
    {
      float val = float( value );
      memcpy( s + position, reinterpret_cast< char * >( &val ),  sizeof( float ) );
      break;
    }
    case QgsPointCloudAttribute::Int32:
    {
      qint32 val = qint32( value );
      memcpy( s + position, reinterpret_cast< char * >( &val ), sizeof( qint32 ) );
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
    case QgsPointCloudAttribute::Float:
    {
      const float val = *reinterpret_cast< const float * >( input + inputPosition );
      return _storeToStream<float>( data, outputPosition, outputType, val );
    }
    case QgsPointCloudAttribute::Int32:
    {
      const qint32 val = *reinterpret_cast<const qint32 * >( input + inputPosition );
      return _storeToStream<qint32>( data, outputPosition, outputType, val );
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

QgsPointCloudBlock *_decompressBinary( const QByteArray &dataUncompressed, const QgsPointCloudAttributeCollection &attributes, const QgsPointCloudAttributeCollection &requestedAttributes, const QgsVector3D &scale, const QgsVector3D &offset )
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
  }
  return new QgsPointCloudBlock(
           count,
           requestedAttributes,
           data, scale, offset
         );
}

QgsPointCloudBlock *QgsEptDecoder::decompressBinary( const QString &filename, const QgsPointCloudAttributeCollection &attributes, const QgsPointCloudAttributeCollection &requestedAttributes, const QgsVector3D &scale, const QgsVector3D &offset )
{
  if ( ! QFile::exists( filename ) )
    return nullptr;

  QFile f( filename );
  const bool r = f.open( QIODevice::ReadOnly );
  if ( !r )
    return nullptr;

  const QByteArray dataUncompressed = f.read( f.size() );
  return _decompressBinary( dataUncompressed, attributes, requestedAttributes, scale, offset );
}

QgsPointCloudBlock *QgsEptDecoder::decompressBinary( const QByteArray &data, const QgsPointCloudAttributeCollection &attributes, const QgsPointCloudAttributeCollection &requestedAttributes, const QgsVector3D &scale, const QgsVector3D &offset )
{
  return _decompressBinary( data, attributes, requestedAttributes, scale, offset );
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

QgsPointCloudBlock *QgsEptDecoder::decompressZStandard( const QString &filename, const QgsPointCloudAttributeCollection &attributes, const QgsPointCloudAttributeCollection &requestedAttributes, const QgsVector3D &scale, const QgsVector3D &offset )
{
  if ( ! QFile::exists( filename ) )
    return nullptr;

  QFile f( filename );
  const bool r = f.open( QIODevice::ReadOnly );
  if ( !r )
    return nullptr;

  const QByteArray dataCompressed = f.readAll();
  const QByteArray dataUncompressed = decompressZtdStream( dataCompressed );
  return _decompressBinary( dataUncompressed, attributes, requestedAttributes, scale, offset );
}

QgsPointCloudBlock *QgsEptDecoder::decompressZStandard( const QByteArray &data, const QgsPointCloudAttributeCollection &attributes, const QgsPointCloudAttributeCollection &requestedAttributes, const QgsVector3D &scale, const QgsVector3D &offset )
{
  const QByteArray dataUncompressed = decompressZtdStream( data );
  return _decompressBinary( dataUncompressed, attributes, requestedAttributes, scale, offset );
}

/* *************************************************************************************** */

struct ExtraBytesAttributeDetails
{
  ExtraBytesAttributeDetails( const QString &attribute, QgsPointCloudAttribute::DataType type, int size, int offset )
    : attribute( attribute )
    , type( type )
    , size( size )
    , offset( offset )
  {}

  QString attribute;
  QgsPointCloudAttribute::DataType type;
  int size;
  int offset;
};

template<typename FileType>
QVector<ExtraBytesAttributeDetails> readExtraByteAttributes( laszip::io::reader::basic_file<FileType> &f, FileType &file )
{
  auto pastFilePos = file.tellg();

  // Read VLR stuff

  struct VlrHeader
  {
    unsigned short reserved;
    char user_id[16];
    unsigned short record_id;
    unsigned short record_length;
    char desc[32];
  };

  struct ExtraByteDescriptor
  {
    unsigned char reserved[2]; // 2 bytes
    unsigned char data_type; // 1 byte
    unsigned char options; // 1 byte
    char name[32]; // 32 bytes
    unsigned char unused[4]; // 4 bytes
    unsigned char no_data[8]; // 8 bytes
    unsigned char deprecated1[16]; // 16 bytes
    unsigned char min[8]; // 8 bytes
    unsigned char deprecated2[16]; // 16 bytes
    unsigned char max[8]; // 8 bytes
    unsigned char deprecated3[16]; // 16 bytes
    unsigned char scale[8]; // 8 bytes
    unsigned char deprecated4[16]; // 16 bytes
    double offset; // 8 bytes
    unsigned char deprecated5[16]; // 16 bytes
    char description[32]; // 32 bytes
  };

  QVector<ExtraByteDescriptor> extraBytes;
  QVector<ExtraBytesAttributeDetails> extrabytesAttr;

  VlrHeader extraBytesVlrHeader;
  int extraBytesDescriptorsOffset = -1;

  file.seekg( f.get_header().header_size );
  for ( unsigned int i = 0; i < f.get_header().vlr_count && file.good() && !file.eof(); ++i )
  {
    VlrHeader vlrHeader;
    file.read( ( char * )&vlrHeader, sizeof( VlrHeader ) );
    file.seekg( vlrHeader.record_length, std::ios::cur );
    if ( std::equal( vlrHeader.user_id, vlrHeader.user_id + 9, "LASF_Spec" ) && vlrHeader.record_id == 4 )
    {
      extraBytesVlrHeader = vlrHeader;
      extraBytesDescriptorsOffset = f.get_header().header_size + sizeof( VlrHeader );
    }
  }

  // Read VLR fields
  if ( extraBytesDescriptorsOffset != -1 )
  {
    file.seekg( extraBytesDescriptorsOffset );
    int n_descriptors = extraBytesVlrHeader.record_length / sizeof( ExtraByteDescriptor );
    for ( int i = 0; i < n_descriptors; ++i )
    {
      ExtraByteDescriptor ebd;
      file.read( ( char * )&ebd, sizeof( ExtraByteDescriptor ) );
      extraBytes.push_back( ebd );
    }
  }

  for ( ExtraByteDescriptor &eb : extraBytes )
  {
    int accOffset = extrabytesAttr.empty() ? 0 : extrabytesAttr.back().offset + extrabytesAttr.back().size;
    // TODO: manage other data types
    switch ( eb.data_type )
    {
      case 0:
        extrabytesAttr.push_back( ExtraBytesAttributeDetails( QString::fromStdString( eb.name ), QgsPointCloudAttribute::Char, eb.options, accOffset ) );
        break;
      case 1:
        extrabytesAttr.push_back( ExtraBytesAttributeDetails( QString::fromStdString( eb.name ), QgsPointCloudAttribute::Char, 1, accOffset ) );
        break;
      case 2:
        extrabytesAttr.push_back( ExtraBytesAttributeDetails( QString::fromStdString( eb.name ), QgsPointCloudAttribute::Char, 1, accOffset ) );
        break;
      case 3:
        extrabytesAttr.push_back( ExtraBytesAttributeDetails( QString::fromStdString( eb.name ), QgsPointCloudAttribute::UShort, 2, accOffset ) );
        break;
      case 4:
        extrabytesAttr.push_back( ExtraBytesAttributeDetails( QString::fromStdString( eb.name ), QgsPointCloudAttribute::Short, 2, accOffset ) );
        break;
      case 5:
        extrabytesAttr.push_back( ExtraBytesAttributeDetails( QString::fromStdString( eb.name ), QgsPointCloudAttribute::Int32, 4, accOffset ) );
        break;
      case 6:
        extrabytesAttr.push_back( ExtraBytesAttributeDetails( QString::fromStdString( eb.name ), QgsPointCloudAttribute::Int32, 4, accOffset ) );
        break;
      case 7:
        extrabytesAttr.push_back( ExtraBytesAttributeDetails( QString::fromStdString( eb.name ), QgsPointCloudAttribute::Int32, 8, accOffset ) );
        break;
      case 8:
        extrabytesAttr.push_back( ExtraBytesAttributeDetails( QString::fromStdString( eb.name ), QgsPointCloudAttribute::Int32, 8, accOffset ) );
        break;
      case 9:
        extrabytesAttr.push_back( ExtraBytesAttributeDetails( QString::fromStdString( eb.name ), QgsPointCloudAttribute::Float, 4, accOffset ) );
        break;
      case 10:
        extrabytesAttr.push_back( ExtraBytesAttributeDetails( QString::fromStdString( eb.name ), QgsPointCloudAttribute::Double, 8, accOffset ) );
        break;
      default:
        break;
    }
  }

  file.seekg( pastFilePos );

  return extrabytesAttr;
}

template<typename FileType>
QgsPointCloudBlock *__decompressLaz( FileType &file, const QgsPointCloudAttributeCollection &attributes, const QgsPointCloudAttributeCollection &requestedAttributes, const QgsVector3D &_scale, const QgsVector3D &_offset )
{
  Q_UNUSED( attributes );
  Q_UNUSED( _scale );
  Q_UNUSED( _offset );

  if ( ! file.good() )
    return nullptr;

#ifdef QGISDEBUG
  const auto start = common::tick();
#endif

  laszip::io::reader::basic_file<FileType> f( file );

  const size_t count = f.get_header().point_count;
  const QgsVector3D scale( f.get_header().scale.x, f.get_header().scale.y, f.get_header().scale.z );
  const QgsVector3D offset( f.get_header().offset.x, f.get_header().offset.y, f.get_header().offset.z );

  QByteArray bufArray( f.get_header().point_record_length, 0 );
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

  QVector<ExtraBytesAttributeDetails> extrabytesAttr = readExtraByteAttributes<FileType>( f, file );

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
      for ( ExtraBytesAttributeDetails &eba : extrabytesAttr )
      {
        if ( requestedAttribute.name().compare( eba.attribute.trimmed() ) )
        {
          requestedAttributeDetails.emplace_back( RequestedAttributeDetails( LazAttribute::ExtraBytes, requestedAttribute.type(), requestedAttribute.size(), eba.offset ) );
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

  for ( size_t i = 0 ; i < count ; i ++ )
  {
    f.readPoint( buf ); // read the point out
    const laszip::formats::las::point10 p = laszip::formats::packers<laszip::formats::las::point10>::unpack( buf );
    const laszip::formats::las::gpstime gps = laszip::formats::packers<laszip::formats::las::gpstime>::unpack( buf + sizeof( laszip::formats::las::point10 ) );
    const laszip::formats::las::rgb rgb = laszip::formats::packers<laszip::formats::las::rgb>::unpack( buf + sizeof( laszip::formats::las::point10 ) + sizeof( laszip::formats::las::gpstime ) );

    char *ebbuf = buf + sizeof( laszip::formats::las::point10 ) + sizeof( laszip::formats::las::gpstime ) + sizeof( laszip::formats::las::rgb );
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
          for ( int i = 0; i < requestedAttribute.size; ++i )
            dataBuffer[outputOffset] = ebbuf[requestedAttribute.offset + i];
        }
        break;
        case LazAttribute::MissingOrUnknown:
          // just store 0 for unknown/missing attributes
          _storeToStream<unsigned short>( dataBuffer, outputOffset, requestedAttribute.type, 0 );
          break;
      }

      outputOffset += requestedAttribute.size;
    }
  }

#ifdef QGISDEBUG
  const float t = common::since( start );
  QgsDebugMsgLevel( QStringLiteral( "LAZ-PERF Read through the points in %1 seconds." ).arg( t ), 2 );
#endif
  QgsPointCloudBlock *block = new QgsPointCloudBlock(
    count,
    requestedAttributes,
    data, scale, offset
  );
  return block;
}

QgsPointCloudBlock *QgsEptDecoder::decompressLaz( const QString &filename,
    const QgsPointCloudAttributeCollection &attributes,
    const QgsPointCloudAttributeCollection &requestedAttributes,
    const QgsVector3D &scale, const QgsVector3D &offset )
{
  const QByteArray arr = filename.toUtf8();
  std::ifstream file( arr.constData(), std::ios::binary );

  return __decompressLaz<std::ifstream>( file, attributes, requestedAttributes, scale, offset );
}

QgsPointCloudBlock *QgsEptDecoder::decompressLaz( const QByteArray &byteArrayData,
    const QgsPointCloudAttributeCollection &attributes,
    const QgsPointCloudAttributeCollection &requestedAttributes,
    const QgsVector3D &scale, const QgsVector3D &offset )
{
  std::istringstream file( byteArrayData.toStdString() );
  return __decompressLaz<std::istringstream>( file, attributes, requestedAttributes, scale, offset );
}

///@endcond
