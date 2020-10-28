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
#include "qgsvector3d.h"
#include "qgsconfig.h"

#include <QFile>
#include <iostream>
#include <memory>

#if defined ( HAVE_ZSTD )
#include <zstd.h>
#endif

#if defined ( HAVE_LAZPERF )
#include "laz-perf/io.hpp"
#include "laz-perf/common/common.hpp"
#endif

///@cond PRIVATE

template <typename T>
bool _storeTripleToStream( QByteArray &data, int position, int size, T *value )
{
  if ( size == 3 * sizeof( qint32 ) )
  {
    for ( int i = 0; i < 3; ++i )
    {
      qint32 val = qint32( value[i] );
      data.insert( position + i * sizeof( T ), ( char * )( &val ), sizeof( qint32 ) );
    }
  }
  else if ( size == 3 * sizeof( double ) )
  {
    for ( int i = 0; i < 3; ++i )
    {
      double val = double( value[i] );
      data.insert( position + i * sizeof( T ), ( char * )( &val ), sizeof( double ) );
    }
  }
  else
  {
    // unsupported
    return false;
  }
  return true;
}

template <typename T>
bool _storeToStream( QByteArray &data, int position, int size, T value )
{
  if ( size == sizeof( char ) )
  {
    char val = char( value );
    data[position] = val;
  }
  else if ( size == sizeof( qint32 ) )
  {
    qint32 val = qint32( value );
    data.insert( position, ( char * )( &val ), sizeof( qint32 ) );
  }
  else if ( size == sizeof( double ) )
  {
    double val = double( value );
    data.insert( position, ( char * )( &val ), sizeof( double ) );
  }
  else
  {
    // unsupported
    return false;
  }
  return true;
}

bool _serialize( QByteArray &data, int outputPosition, int outputSize, const char *input, int inputSize, int inputPosition )
{
  if ( inputSize == outputSize )
  {
    data.insert( outputPosition, input + inputPosition, inputSize );
    return true;
  }

  if ( inputSize == sizeof( char ) )
  {
    char val = *( input + inputPosition );
    return _storeToStream<char>( data, outputPosition, outputSize, val );
  }
  else if ( inputSize == sizeof( qint32 ) )
  {
    qint32 val = *( qint32 * )( input + inputPosition );
    return _storeToStream<qint32>( data, outputPosition, outputSize, val );
  }
  else if ( inputSize == sizeof( double ) )
  {
    double val = *( double * )( input + inputPosition );
    return _storeToStream<double>( data, outputPosition, outputSize, val );
  }
  else if ( inputSize == 3 * sizeof( qint32 ) )
  {
    qint32 *val = ( qint32 * )( input + inputPosition );
    return _storeTripleToStream<qint32>( data, outputPosition, outputSize, val );
  }
  else if ( inputSize == 3 * sizeof( double ) )
  {
    double *val = ( double * )( input + inputPosition );
    return _storeTripleToStream<double>( data, outputPosition, outputSize, val );
  }
  else
  {
    // unsupported
    return false;
  }
}

// //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

QgsPointCloudBlock *_decompressBinary( const QByteArray &dataUncompressed, const QgsPointCloudAttributeCollection &attributes, const QgsPointCloudAttributeCollection &requestedAttributes )
{
  const int pointRecordSize = attributes.pointRecordSize( );
  const int requestedPointRecordSize = requestedAttributes.pointRecordSize();
  const int count = dataUncompressed.size() / pointRecordSize;
  QByteArray data( nullptr, requestedPointRecordSize * count );
  const char *s = dataUncompressed.data();

  for ( int i = 0; i < count; ++i )
  {
    int outputOffset = 0;
    const QVector<QgsPointCloudAttribute> requestedAttributesVector = requestedAttributes.attributes();
    for ( const QgsPointCloudAttribute &requestedAttribute : requestedAttributesVector )
    {
      QgsPointCloudAttribute *foundAttribute = nullptr;
      int inputOffset = attributes.offset( requestedAttribute.name(), foundAttribute );
      // invalid request
      if ( inputOffset < 0 || !foundAttribute )
        return nullptr;

      Q_ASSERT( requestedAttribute.name() == foundAttribute->name() );

      _serialize( data, i * requestedPointRecordSize + outputOffset, requestedAttribute.size(), s, foundAttribute->size(), i * pointRecordSize + inputOffset );

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
  std::ifstream file( filename.toLatin1().constData(), std::ios::binary );
  if ( ! file.good() )
    return nullptr;

  auto start = common::tick();

  laszip::io::reader::file f( file );

  const size_t count = f.get_header().point_count;
  char buf[256]; // a buffer large enough to hold our point

  const int requestedPointRecordSize = requestedAttributes.pointRecordSize();
  QByteArray data( nullptr, requestedPointRecordSize * count );

  for ( size_t i = 0 ; i < count ; i ++ )
  {
    f.readPoint( buf ); // read the point out
    laszip::formats::las::point10 p = laszip::formats::packers<laszip::formats::las::point10>::unpack( buf );
    int outputOffset = 0;
    const QVector<QgsPointCloudAttribute> requestedAttributesVector = requestedAttributes.attributes();
    for ( const QgsPointCloudAttribute &requestedAttribute : requestedAttributesVector )
    {

      if ( requestedAttribute.name() == QStringLiteral( "position" ) )
      {
        QVector<qint32> position = {p.x, p.y, p.z};
        _storeTripleToStream<qint32>( data, i * requestedPointRecordSize + outputOffset, requestedAttribute.size(), position.data() );
      }
      else if ( requestedAttribute.name() == QStringLiteral( "classification" ) )
      {
        _storeToStream<char>( data, i * requestedPointRecordSize + outputOffset, requestedAttribute.size(), p.classification );
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
