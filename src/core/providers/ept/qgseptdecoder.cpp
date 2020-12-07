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
#include <iostream>
#include <memory>
#include <cstring>

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
      char val = char( value );
      s[position] = val;
      break;
    }
    case QgsPointCloudAttribute::Short:
    {
      short val = short( value );
      memcpy( s + position, ( char * )( &val ), sizeof( short ) );
      break;
    }

    case QgsPointCloudAttribute::UShort:
    {
      unsigned short val = static_cast< unsigned short>( value );
      memcpy( s + position, ( char * )( &val ), sizeof( unsigned short ) );
      break;
    }

    case QgsPointCloudAttribute::Float:
    {
      float val = float( value );
      memcpy( s + position, ( char * )( &val ),  sizeof( float ) );
      break;
    }
    case QgsPointCloudAttribute::Int32:
    {
      qint32 val = qint32( value );
      memcpy( s + position, ( char * )( &val ), sizeof( qint32 ) );
      break;
    }
    case QgsPointCloudAttribute::Double:
    {
      double val = double( value );
      memcpy( s + position, ( char * )( &val ), sizeof( double ) );
      break;
    }
  }

  return true;
}

bool _serialize( char *data, size_t outputPosition, QgsPointCloudAttribute::DataType outputType,
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
      char val = *( input + inputPosition );
      return _storeToStream<char>( data, outputPosition, outputType, val );
    }
    case QgsPointCloudAttribute::Short:
    {
      short val = *( short * )( input + inputPosition );
      return _storeToStream<short>( data, outputPosition, outputType, val );
    }
    case QgsPointCloudAttribute::UShort:
    {
      unsigned short val = *reinterpret_cast< const unsigned short * >( input + inputPosition );
      return _storeToStream<unsigned short>( data, outputPosition, outputType, val );
    }
    case QgsPointCloudAttribute::Float:
    {
      float val = *( float * )( input + inputPosition );
      return _storeToStream<float>( data, outputPosition, outputType, val );
    }
    case QgsPointCloudAttribute::Int32:
    {
      qint32 val = *( qint32 * )( input + inputPosition );
      return _storeToStream<qint32>( data, outputPosition, outputType, val );
    }
    case QgsPointCloudAttribute::Double:
    {
      double val = *( double * )( input + inputPosition );
      return _storeToStream<double>( data, outputPosition, outputType, val );
    }
  }
  return true;
}

// //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

QgsPointCloudBlock *_decompressBinary( const QByteArray &dataUncompressed, const QgsPointCloudAttributeCollection &attributes, const QgsPointCloudAttributeCollection &requestedAttributes )
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
      _serialize( destinationBuffer, outputOffset,
                  attribute.requestedType, s,
                  attribute.inputType, attribute.inputSize, i * pointRecordSize + attribute.inputOffset );

      outputOffset += attribute.requestedSize;
    }
  }
  return new QgsPointCloudBlock(
           count,
           requestedAttributes,
           data
         );
}


QgsPointCloudBlock *QgsEptDecoder::decompressBinary( const QString &filename, const QgsPointCloudAttributeCollection &attributes, const QgsPointCloudAttributeCollection &requestedAttributes )
{
  if ( ! QFile::exists( filename ) )
    return nullptr;

  QFile f( filename );
  bool r = f.open( QIODevice::ReadOnly );
  if ( !r )
    return nullptr;

  QByteArray dataUncompressed = f.read( f.size() );
  return _decompressBinary( dataUncompressed, attributes, requestedAttributes );
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
  size_t ret = ZSTD_decompressStream( strm, &outBuf, &m_inBuf );
  Q_ASSERT( !ZSTD_isError( ret ) );
  Q_ASSERT( outBuf.pos );
  Q_ASSERT( outBuf.pos < outBuf.size );

  ZSTD_freeDStream( strm );
  dataUncompressed.resize( outBuf.pos );
  return dataUncompressed;
}

QgsPointCloudBlock *QgsEptDecoder::decompressZStandard( const QString &filename, const QgsPointCloudAttributeCollection &attributes, const QgsPointCloudAttributeCollection &requestedAttributes )
{
  if ( ! QFile::exists( filename ) )
    return nullptr;

  QFile f( filename );
  bool r = f.open( QIODevice::ReadOnly );
  if ( !r )
    return nullptr;

  QByteArray dataCompressed = f.readAll();
  QByteArray dataUncompressed = decompressZtdStream( dataCompressed );
  return _decompressBinary( dataUncompressed, attributes, requestedAttributes );
}

/* *************************************************************************************** */

QgsPointCloudBlock *QgsEptDecoder::decompressLaz( const QString &filename,
    const QgsPointCloudAttributeCollection &attributes,
    const QgsPointCloudAttributeCollection &requestedAttributes )
{
  Q_UNUSED( attributes )

  std::ifstream file( filename.toLatin1().constData(), std::ios::binary );
  if ( ! file.good() )
    return nullptr;

#ifdef QGISDEBUG
  auto start = common::tick();
#endif

  laszip::io::reader::file f( file );

  const size_t count = f.get_header().point_count;
  char buf[256]; // a buffer large enough to hold our point

  const size_t requestedPointRecordSize = requestedAttributes.pointRecordSize();
  QByteArray data;
  data.resize( requestedPointRecordSize * count );
  char *dataBuffer = data.data();

  for ( size_t i = 0 ; i < count ; i ++ )
  {
    f.readPoint( buf ); // read the point out
    laszip::formats::las::point10 p = laszip::formats::packers<laszip::formats::las::point10>::unpack( buf );
    int outputOffset = 0;
    const QVector<QgsPointCloudAttribute> requestedAttributesVector = requestedAttributes.attributes();
    for ( const QgsPointCloudAttribute &requestedAttribute : requestedAttributesVector )
    {

      if ( requestedAttribute.name() == QStringLiteral( "X" ) )
      {
        _storeToStream<qint32>( dataBuffer, i * requestedPointRecordSize + outputOffset, requestedAttribute.type(), p.x );
      }
      else if ( requestedAttribute.name() == QStringLiteral( "Y" ) )
      {
        _storeToStream<qint32>( dataBuffer, i * requestedPointRecordSize + outputOffset, requestedAttribute.type(), p.y );
      }
      else if ( requestedAttribute.name() == QStringLiteral( "Z" ) )
      {
        _storeToStream<qint32>( dataBuffer, i * requestedPointRecordSize + outputOffset, requestedAttribute.type(), p.z );
      }
      else if ( requestedAttribute.name() == QStringLiteral( "Classification" ) )
      {
        _storeToStream<char>( dataBuffer, i * requestedPointRecordSize + outputOffset, requestedAttribute.type(), p.classification );
      }
      outputOffset += requestedAttribute.size();
    }
  }

#ifdef QGISDEBUG
  float t = common::since( start );
  QgsDebugMsgLevel( QStringLiteral( "LAZ-PERF Read through the points in %1 seconds." ).arg( t ), 2 );
#endif

  return new QgsPointCloudBlock(
           count,
           requestedAttributes,
           data
         );
}

///@endcond
