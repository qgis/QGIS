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

#if defined ( HAVE_ZSTD )
#include <zstd.h>
#endif

#if defined ( HAVE_LAZPERF )
#include "laz-perf/io.hpp"
#include "laz-perf/common/common.hpp"
#endif

///@cond PRIVATE

QVector<qint32> QgsEptDecoder::decompressBinary( const QString &filename, int pointRecordSize )
{
  Q_ASSERT( QFile::exists( filename ) );

  QFile f( filename );
  bool r = f.open( QIODevice::ReadOnly );
  Q_ASSERT( r );

  int count = f.size() / pointRecordSize;
  QVector<qint32> data( count * 3 );
  for ( int i = 0; i < count; ++i )
  {
    QByteArray bytes = f.read( pointRecordSize );
    // WHY??? X,Y,Z are int32 values stored as doubles
    double *bytesD = ( double * ) bytes.constData();
    // TODO: we should respect the schema (offset and data type of X,Y,Z)
    data[i * 3 + 0] = ( bytesD[0] );
    data[i * 3 + 1] = ( bytesD[1] );
    data[i * 3 + 2] = ( bytesD[2] );
  }
  return data;
}

QVector<char> QgsEptDecoder::decompressBinaryClasses( const QString &filename, int pointRecordSize )
{
  Q_ASSERT( QFile::exists( filename ) );

  QFile f( filename );
  bool r = f.open( QIODevice::ReadOnly );
  Q_ASSERT( r );

  int count = f.size() / pointRecordSize;
  QVector<char> classes( count );
  for ( int i = 0; i < count; ++i )
  {
    QByteArray bytes = f.read( pointRecordSize );

    // qint32 x = *(double*)(ptData);
    // qint32 y = *(double*)(ptData+8);
    // qint32 z = *(double*)(ptData+16);
    char cls = bytes[30];
    // vertices.push_back( Point3D( x, y, z ) );
    classes[i] = cls;
    //++count;
  }
  return classes;
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

QVector<qint32> QgsEptDecoder::decompressZStandard( const QString &filename, int pointRecordSize )
{
  Q_ASSERT( QFile::exists( filename ) );

  QFile f( filename );
  bool r = f.open( QIODevice::ReadOnly );
  Q_ASSERT( r );

  QByteArray dataCompressed = f.readAll();
  QByteArray dataUncompressed = decompressZtdStream( dataCompressed );

  // from here it's the same as "binary"

  int count = dataUncompressed.size() / pointRecordSize;

  QVector<qint32> data( count * 3 );
  const char *ptr = dataUncompressed.constData();
  for ( int i = 0; i < count; ++i )
  {
    // WHY??? X,Y,Z are int32 values stored as doubles
    double *bytesD = ( double * )( ptr + pointRecordSize * i );
    // TODO: we should respect the schema (offset and data type of X,Y,Z)
    data[i * 3 + 0] = ( bytesD[0] );
    data[i * 3 + 1] = ( bytesD[1] );
    data[i * 3 + 2] = ( bytesD[2] );
  }
  return data;
}

#else // defined(HAVE_ZSTD)
QVector<qint32> QgsEptDecoder::decompressZStandard( const QString &filename, int pointRecordSize )
{
  //TODO graceful error
  Q_UNUSED( filename )
  Q_UNUSED( pointRecordSize )
  Q_ASSERT( false );
}

#endif // defined(HAVE_ZSTD)

/* *************************************************************************************** */

#if defined ( HAVE_LAZPERF )
QVector<qint32> QgsEptDecoder::decompressLaz( const QString &filename )
{
  std::ifstream file( filename.toLatin1().constData(), std::ios::binary );
  Q_ASSERT( file.good() );

  auto start = common::tick();

  laszip::io::reader::file f( file );

  size_t count = f.get_header().point_count;
  char buf[256]; // a buffer large enough to hold our point

  QVector<qint32> data( count * 3 );

  for ( size_t i = 0 ; i < count ; i ++ )
  {
    f.readPoint( buf ); // read the point out
    laszip::formats::las::point10 p = laszip::formats::packers<laszip::formats::las::point10>::unpack( buf );

    data[i * 3 + 0] = p.x ;
    data[i * 3 + 1] = p.y ;
    data[i * 3 + 2] = p.z ;
  }
  float t = common::since( start );
  std::cout << "LAZ-PERF Read through the points in " << t << " seconds." << std::endl;
  return data;
}

#else // defined ( HAVE_LAZPERF )
QVector<qint32> QgsEptDecoder::decompressLaz( const QString &filename )
{
  //TODO graceful return and error message
  Q_UNUSED( filename )
  Q_ASSERT( false );
}
#endif

///@endcond
