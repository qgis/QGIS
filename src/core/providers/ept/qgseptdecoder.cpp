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
#include <QDataStream>

#if defined ( HAVE_ZSTD )
#include <zstd.h>
#endif

#if defined ( HAVE_LAZPERF )
#include "laz-perf/io.hpp"
#include "laz-perf/common/common.hpp"
#endif

///@cond PRIVATE

QgsPointCloudBlock *QgsEptDecoder::decompressBinary( const QString &filename, const QgsPointCloudAttributeCollection &attributes, const QgsPointCloudAttributeCollection &requestedAttributes )
{
  Q_ASSERT( QFile::exists( filename ) );

  int pointRecordSize = attributes.pointRecordSize( );

  QFile f( filename );
  bool r = f.open( QIODevice::ReadOnly );
  Q_ASSERT( r );

  QByteArray data;
  QDataStream stream( &data, QIODevice::WriteOnly );

  int count = f.size() / pointRecordSize;
  for ( int i = 0; i < count; ++i )
  {
    QByteArray bytes = f.read( pointRecordSize );
    char *s = bytes.data();

    qint32 x = *( double * )( s );
    qint32 y = *( double * )( s + 8 );
    qint32 z = *( double * )( s + 16 );
    char cls = bytes[30];

    stream << x << y << z << cls;

    //++count;
  }
  return new QgsPointCloudBlock(
           count,
           requestedAttributes,
           data
         );
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
  Q_ASSERT( QFile::exists( filename ) );

  QFile f( filename );
  bool r = f.open( QIODevice::ReadOnly );
  Q_ASSERT( r );

  QByteArray dataCompressed = f.readAll();
  QByteArray dataUncompressed = decompressZtdStream( dataCompressed );

  int pointRecordSize = attributes.pointRecordSize( );
  // from here it's the same as "binary"

  int count = dataUncompressed.size() / pointRecordSize;

  QByteArray data;
  QDataStream stream( &data, QIODevice::WriteOnly );

  const char *ptr = dataUncompressed.constData();
  for ( int i = 0; i < count; ++i )
  {
    // WHY??? X,Y,Z are int32 values stored as doubles
    // double *bytesD = ( double * )( ptr + pointRecordSize * i );
    // TODO: we should respect the schema (offset and data type of X,Y,Z)

    // TODO type based on requested attributes
    qint32 x = *( double * )( ptr + pointRecordSize * i );
    qint32 y = *( double * )( ptr + pointRecordSize * i + 8 );
    qint32 z = *( double * )( ptr + pointRecordSize * i + 16 );
    char cls = *( char * )( ptr + pointRecordSize * i + 30 );

    stream << x << y << z << cls;
  }
  return new QgsPointCloudBlock(
           count,
           requestedAttributes,
           data
         );
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
  Q_UNUSED(attributes)

  std::ifstream file( filename.toLatin1().constData(), std::ios::binary );
  Q_ASSERT( file.good() );

  auto start = common::tick();

  laszip::io::reader::file f( file );

  size_t count = f.get_header().point_count;
  char buf[256]; // a buffer large enough to hold our point

  QByteArray data;
  QDataStream stream( &data, QIODevice::WriteOnly );

  for ( size_t i = 0 ; i < count ; i ++ )
  {
    f.readPoint( buf ); // read the point out
    laszip::formats::las::point10 p = laszip::formats::packers<laszip::formats::las::point10>::unpack( buf );
    qint32 x = p.x ;
    qint32 y = p.y ;
    qint32 z = p.z ;
    char cls = p.classification;
    stream << x << y << z << cls;
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
