/***************************************************************************
    quantizedmeshgeometry.cpp
    ---------------------
    begin                : July 2017
    copyright            : (C) 2017 by Martin Dobias
    email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "quantizedmeshgeometry.h"

#include <zlib.h>

#include <QFile>
#include <QByteArray>


// gzip decompression snipped from https://stackoverflow.com/questions/2690328/qt-quncompress-gzip-data

#define GZIP_WINDOWS_BIT 15 + 16
#define GZIP_CHUNK_SIZE 32 * 1024

/**
 * \brief Decompresses the given buffer using the standard GZIP algorithm
 * \param input The buffer to be decompressed
 * \param output The result of the decompression
 * \return \c true if the decompression was successful, \c false otherwise
 */
bool gzipDecompress( QByteArray input, QByteArray &output )
{
  // Prepare output
  output.clear();

  // Is there something to do?
  if ( input.isEmpty() )
    return true;

  // Prepare inflater status
  z_stream strm;
  strm.zalloc = Z_NULL;
  strm.zfree = Z_NULL;
  strm.opaque = Z_NULL;
  strm.avail_in = 0;
  strm.next_in = Z_NULL;

  // Initialize inflater
  int ret = inflateInit2( &strm, GZIP_WINDOWS_BIT );

  if ( ret != Z_OK )
    return false;

  // Extract pointer to input data
  const char *input_data = input.constData();
  int input_data_left = input.length();

  // Decompress data until available
  do
  {
    // Determine current chunk size
    int chunk_size = std::min( GZIP_CHUNK_SIZE, input_data_left );

    // Check for termination
    if ( chunk_size <= 0 )
      break;

    // Set inflater references
    strm.next_in = ( unsigned char * )input_data;
    strm.avail_in = chunk_size;

    // Update interval variables
    input_data += chunk_size;
    input_data_left -= chunk_size;

    // Inflate chunk and cumulate output
    do
    {

      // Declare vars
      char out[GZIP_CHUNK_SIZE];

      // Set inflater references
      strm.next_out = ( unsigned char * )out;
      strm.avail_out = GZIP_CHUNK_SIZE;

      // Try to inflate chunk
      ret = inflate( &strm, Z_NO_FLUSH );

      switch ( ret )
      {
        case Z_NEED_DICT:
          ret = Z_DATA_ERROR;
        case Z_DATA_ERROR:
        case Z_MEM_ERROR:
        case Z_STREAM_ERROR:
          // Clean-up
          inflateEnd( &strm );

          // Return
          return ( false );
      }

      // Determine decompressed size
      int have = ( GZIP_CHUNK_SIZE - strm.avail_out );

      // Cumulate result
      if ( have > 0 )
        output.append( ( char * )out, have );

    }
    while ( strm.avail_out == 0 );

  }
  while ( ret != Z_STREAM_END );

  // Clean-up
  inflateEnd( &strm );

  // Return
  return ( ret == Z_STREAM_END );
}


const char *read_zigzag_encoded_int16_array( const char *dataPtr, int count, qint16 *out )
{
  for ( int i = 0; i < count; ++i )
  {
    quint16 encoded = *( quint16 * ) dataPtr;
    dataPtr += 2;
    qint16 decoded = ( encoded >> 1 ) ^ ( -( encoded & 1 ) );
    *out++ = decoded;
  }
  return dataPtr;
}


static QString _tileFilename( int tx, int ty, int tz )
{
  return QString( "/tmp/terrain-%1-%2-%3" ).arg( tz ).arg( tx ).arg( ty );
}

QuantizedMeshTile *QuantizedMeshGeometry::readTile( int tx, int ty, int tz, const QgsRectangle &extent )
{
  QString filename = _tileFilename( tx, ty, tz );
  QFile f( filename );
  if ( !f.open( QIODevice::ReadOnly ) )
    return nullptr;

  QByteArray data;
  if ( !gzipDecompress( f.readAll(), data ) )
    return nullptr;

  if ( data.isEmpty() )
    return nullptr;

  QuantizedMeshTile *t = new QuantizedMeshTile;
  t->extent = extent;

  const char *dataPtr = data.constData();
  memcpy( &t->header, dataPtr, sizeof( QuantizedMeshHeader ) );
  dataPtr += sizeof( QuantizedMeshHeader );

  // vertex data - immediately after header
  // with zig-zag encoding

  //struct VertexData
  //{
  //    unsigned int vertexCount;
  //    unsigned short u[vertexCount];
  //    unsigned short v[vertexCount];
  //    unsigned short height[vertexCount];
  //};

  quint32 vertexCount = *( quint32 * ) dataPtr;
  dataPtr += 4;
  t->uvh.resize( 3 * vertexCount );
  qint16 *vptr = t->uvh.data();
  dataPtr = read_zigzag_encoded_int16_array( dataPtr, vertexCount * 3, vptr );
  // the individual values are just deltas of previous values!
  qint16 u = 0, v = 0, h = 0;
  for ( uint i = 0; i < vertexCount; ++i )
  {
    qint16 du = vptr[i], dv = vptr[vertexCount + i], dh = vptr[vertexCount * 2 + i];
    u += du;
    v += dv;
    h += dh;
    vptr[i] = u;
    vptr[vertexCount + i] = v;
    vptr[vertexCount * 2 + i] = h;
  }

  Q_ASSERT( vertexCount < 65537 ); // supporting currently only 2-byte vertex indices

  // index data - if less than 65537 vertices (otherwise indices would be 4-byte)
  // with "high watermark" encoding

  //struct IndexData16
  //{
  //  unsigned int triangleCount;
  //  unsigned short indices[triangleCount * 3];
  //}

  quint32 triangleCount = *( quint32 * ) dataPtr;
  dataPtr += 4;
  t->indices.resize( 3 * triangleCount );
  quint16 *indicesPtr = t->indices.data();
  quint16 *srcIdxPtr = ( quint16 * )dataPtr;
  int highest = 0;
  for ( uint i = 0; i < triangleCount * 3; ++i )
  {
    quint16 code = *srcIdxPtr++;
    *indicesPtr++ = highest - code;
    if ( code == 0 )
      ++highest;
  }

  // TODO: edge indices

  // TODO: extensions

  //qDebug() << hdr.CenterX << " " << hdr.CenterY << " " << hdr.CenterZ;
  //qDebug() << "VC " << vertexCount;
  //qDebug() << "TC " << triangleCount;

  return t;
}


#include <QGuiApplication>
#include <QNetworkRequest>
#include <QNetworkReply>
#include "qgsnetworkaccessmanager.h"

void QuantizedMeshGeometry::downloadTileIfMissing( int tx, int ty, int tz )
{
  // i am not proud of this bit of code... quick&dirty!

  QString tileFilename = _tileFilename( tx, ty, tz );
  if ( !QFile::exists( tileFilename ) )
  {
    qDebug() << "downloading tile " << tx << " " << ty << " " << tz;
    bool downloaded = false;
    QString url = QString( "http://assets.agi.com/stk-terrain/tilesets/world/tiles/%1/%2/%3.terrain" ).arg( tz ).arg( tx ).arg( ty );
    QNetworkRequest request( url );
    request.setRawHeader( QByteArray( "Accept-Encoding" ), QByteArray( "gzip" ) );
    request.setRawHeader( QByteArray( "Accept" ), QByteArray( "application/vnd.quantized-mesh,application/octet-stream;q=0.9" ) );
    request.setRawHeader( QByteArray( "User-Agent" ), QByteArray( "Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Ubuntu Chromium/58.0.3029.110 Chrome/58.0.3029.110 Safari/537.36" ) );
    QNetworkReply *reply = QgsNetworkAccessManager::instance()->get( request );
    connect( reply, &QNetworkReply::finished, [reply, tileFilename, &downloaded]
    {
      QFile fOut( tileFilename );
      fOut.open( QIODevice::WriteOnly );
      fOut.write( reply->readAll() );
      fOut.close();
      reply->deleteLater();
      downloaded = true;
    } );

    while ( !downloaded )
    {
      qApp->processEvents();
    }
  }
}

// --------------

#include "qgscoordinatetransform.h"
#include "qgsmaptopixel.h"
#include "map3d.h"

QuantizedMeshGeometry::QuantizedMeshGeometry( QuantizedMeshTile *t, const Map3D &map, const QgsMapToPixel &mapToPixel, const QgsCoordinateTransform &terrainToMap, QNode *parent )
  : QGeometry( parent )
{
  int vertexCount = t->uvh.count() / 3;
  int indexCount = t->indices.count();

  int vertexEntrySize = sizeof( float ) * ( 3 + 2 );

  double xMinWgs = t->extent.xMinimum();
  double yMinWgs = t->extent.yMinimum();
  double widthWgs = t->extent.width();
  double heightWgs = t->extent.height();
  QgsPointXY ptMinProjected = QgsPointXY( map.originX, map.originY );

  QByteArray vb;
  const qint16 *uvh = t->uvh.constData();
  vb.resize( vertexCount * vertexEntrySize );
  float *vbptr = ( float * ) vb.data();
  for ( int i = 0; i < vertexCount; ++i )
  {
    qint16 u = uvh[i], v = uvh[vertexCount + i], h = uvh[vertexCount * 2 + i];
    float uNorm = u / 32767.f;  // 0...1
    float vNorm = v / 32767.f;  // 0...1
    float hNorm = h / 32767.f;  // 0...1
    float xWgs = xMinWgs + widthWgs * uNorm;
    float yWgs = yMinWgs + heightWgs * vNorm;
    float hWgs = t->header.MinimumHeight + hNorm * ( t->header.MaximumHeight - t->header.MinimumHeight );

    QgsPointXY ptProjected = terrainToMap.transform( xWgs, yWgs );
    QgsPointXY ptFinal( ptProjected.x() - ptMinProjected.x(), ptProjected.y() - ptMinProjected.y() );

    // our plane is (x,-z) with y growing towards camera
    *vbptr++ = ptFinal.x();
    *vbptr++ = hWgs;
    *vbptr++ = -ptFinal.y();

    QgsPointXY uv = mapToPixel.transform( ptProjected ) / map.tileTextureSize;
    // texture coords
    *vbptr++ = uv.x();
    *vbptr++ = uv.y();
  }

  QByteArray ib;
  ib.resize( indexCount * 2 );
  memcpy( ib.data(), t->indices.constData(), ib.count() );
  /*
  quint16* ibptr = (quint16*)ib.data();
  const quint16* srcptr = t->indices.constData();
  // reverse order of indices to triangles
  for (int i = 0; i < indexCount/3; ++i)
  {
    ibptr[i*3] = srcptr[i*3+2];
    ibptr[i*3+1] = srcptr[i*3+1];
    ibptr[i*3+2] = srcptr[i*3];
  }
  */
  m_vertexBuffer = new Qt3DRender::QBuffer( this );
  m_indexBuffer = new Qt3DRender::QBuffer( this );

  m_vertexBuffer->setData( vb );
  m_indexBuffer->setData( ib );

  m_positionAttribute = new Qt3DRender::QAttribute( this );
  m_positionAttribute->setName( Qt3DRender::QAttribute::defaultPositionAttributeName() );
  m_positionAttribute->setVertexBaseType( Qt3DRender::QAttribute::Float );
  m_positionAttribute->setVertexSize( 3 );
  m_positionAttribute->setAttributeType( Qt3DRender::QAttribute::VertexAttribute );
  m_positionAttribute->setBuffer( m_vertexBuffer );
  m_positionAttribute->setByteStride( vertexEntrySize );
  m_positionAttribute->setCount( vertexCount );

  m_texCoordAttribute = new Qt3DRender::QAttribute( this );
  m_texCoordAttribute->setName( Qt3DRender::QAttribute::defaultTextureCoordinateAttributeName() );
  m_texCoordAttribute->setVertexBaseType( Qt3DRender::QAttribute::Float );
  m_texCoordAttribute->setVertexSize( 2 );
  m_texCoordAttribute->setAttributeType( Qt3DRender::QAttribute::VertexAttribute );
  m_texCoordAttribute->setBuffer( m_vertexBuffer );
  m_texCoordAttribute->setByteStride( vertexEntrySize );
  m_texCoordAttribute->setByteOffset( 3 * sizeof( float ) );
  m_texCoordAttribute->setCount( vertexCount );

  m_indexAttribute = new Qt3DRender::QAttribute( this );
  m_indexAttribute->setAttributeType( Qt3DRender::QAttribute::IndexAttribute );
  m_indexAttribute->setVertexBaseType( Qt3DRender::QAttribute::UnsignedShort );
  m_indexAttribute->setBuffer( m_indexBuffer );
  m_indexAttribute->setCount( indexCount );

  addAttribute( m_positionAttribute );
  addAttribute( m_texCoordAttribute );
  addAttribute( m_indexAttribute );
}
