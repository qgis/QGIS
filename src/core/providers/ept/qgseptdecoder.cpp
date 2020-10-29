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

#include <QFile>
#include <iostream>
#include <memory>
#include <cstring>

#if defined ( HAVE_ZSTD )
#include <zstd.h>
#endif

#if defined ( HAVE_LAZPERF )
#include "laz-perf/io.hpp"
#include "laz-perf/common/common.hpp"
#endif

///@cond PRIVATE

template <typename T>
bool _storeToStream( QByteArray &data, size_t position, QgsPointCloudAttribute::DataType type, T value )
{
  char *s = data.data();
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

bool _serialize( QByteArray &data, size_t outputPosition, const QgsPointCloudAttribute &outputAttribute, const char *input, const QgsPointCloudAttribute &inputAttribute, size_t inputPosition )
{
  if ( outputAttribute.type() == inputAttribute.type() )
  {
    memcpy( data.data() + outputPosition, input + inputPosition, inputAttribute.size() );
    return true;
  }

  switch ( inputAttribute.type() )
  {
    case QgsPointCloudAttribute::Char:
    {
      char val = *( input + inputPosition );
      return _storeToStream<char>( data, outputPosition, outputAttribute.type(), val );
    }
    case QgsPointCloudAttribute::Short:
    {
      short val = *( short * )( input + inputPosition );
      return _storeToStream<short>( data, outputPosition, outputAttribute.type(), val );
    }
    case QgsPointCloudAttribute::Float:
    {
      float val = *( float * )( input + inputPosition );
      return _storeToStream<float>( data, outputPosition, outputAttribute.type(), val );
    }
    case QgsPointCloudAttribute::Int32:
    {
      qint32 val = *( qint32 * )( input + inputPosition );
      return _storeToStream<qint32>( data, outputPosition, outputAttribute.type(), val );
    }
    case QgsPointCloudAttribute::Double:
    {
      double val = *( double * )( input + inputPosition );
      return _storeToStream<double>( data, outputPosition, outputAttribute.type(), val );
    }
  }
  return true;
}

// //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

QgsPointCloudBlock *_decompressBinary( const QByteArray &dataUncompressed, const QgsPointCloudAttributeCollection &attributes, const QgsPointCloudAttributeCollection &requestedAttributes )
{
  const int pointRecordSize = attributes.pointRecordSize( );
  const int requestedPointRecordSize = requestedAttributes.pointRecordSize();
  const int count = dataUncompressed.size() / pointRecordSize;
  QByteArray data;
  data.resize( requestedPointRecordSize * count );
  const char *s = dataUncompressed.data();

  for ( int i = 0; i < count; ++i )
  {
    size_t outputOffset = 0;
    const QVector<QgsPointCloudAttribute> requestedAttributesVector = requestedAttributes.attributes();
    for ( const QgsPointCloudAttribute &requestedAttribute : requestedAttributesVector )
    {
      int inputAttributeOffset;
      const QgsPointCloudAttribute *inputAttribute = attributes.find( requestedAttribute.name(), inputAttributeOffset );
      if ( !inputAttribute )
      {
        return nullptr;
      }

      _serialize( data, i * requestedPointRecordSize + outputOffset, requestedAttribute, s, *inputAttribute, i * pointRecordSize + inputAttributeOffset );

      outputOffset += requestedAttribute.size();
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

#if defined(HAVE_ZSTD)

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

#else // defined(HAVE_ZSTD)
QgsPointCloudBlock *QgsEptDecoder::decompressZStandard( const QString &filename, const QgsPointCloudAttributeCollection &attributes, const QgsPointCloudAttributeCollection &requestedAttributes )
{
  Q_UNUSED( filename )
  Q_UNUSED( attributes )
  Q_UNUSED( requestedAttributes )
  return nullptr;
}

#endif // defined(HAVE_ZSTD)

/* *************************************************************************************** */

#if defined ( HAVE_LAZPERF )
QgsPointCloudBlock *QgsEptDecoder::decompressLaz( const QString &filename,
    const QgsPointCloudAttributeCollection &attributes,
    const QgsPointCloudAttributeCollection &requestedAttributes )
{
  Q_UNUSED( attributes )

  std::ifstream file( filename.toLatin1().constData(), std::ios::binary );
  if ( ! file.good() )
    return nullptr;

  auto start = common::tick();

  laszip::io::reader::file f( file );

  const size_t count = f.get_header().point_count;
  char buf[256]; // a buffer large enough to hold our point

  const size_t requestedPointRecordSize = requestedAttributes.pointRecordSize();
  QByteArray data;
  data.resize( requestedPointRecordSize * count );

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
        _storeToStream<qint32>( data, i * requestedPointRecordSize + outputOffset, requestedAttribute.type(), p.x );
      }
      else if ( requestedAttribute.name() == QStringLiteral( "Y" ) )
      {
        _storeToStream<qint32>( data, i * requestedPointRecordSize + outputOffset, requestedAttribute.type(), p.y );
      }
      else if ( requestedAttribute.name() == QStringLiteral( "Z" ) )
      {
        _storeToStream<qint32>( data, i * requestedPointRecordSize + outputOffset, requestedAttribute.type(), p.z );
      }
      else if ( requestedAttribute.name() == QStringLiteral( "Classification" ) )
      {
        _storeToStream<char>( data, i * requestedPointRecordSize + outputOffset, requestedAttribute.type(), p.classification );
      }
      outputOffset += requestedAttribute.size();
    }
  }
  float t = common::since( start );
  std::cout << "LAZ-PERF Read through the points in " << t << " seconds." << std::endl;
  return new QgsPointCloudBlock(
           count,
           requestedAttributes,
           data
         );
}

#else // defined ( HAVE_LAZPERF )
QgsPointCloudBlock *QgsEptDecoder::decompressLaz( const QString &filename, const QgsPointCloudAttributeCollection &attributes, const QgsPointCloudAttributeCollection &requestedAttributes )
{
  Q_UNUSED( filename )
  Q_UNUSED( attributes )
  Q_UNUSED( requestedAttributes )
  return nullptr;
}
#endif

///@endcond
