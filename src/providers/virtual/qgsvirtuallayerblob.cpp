/***************************************************************************
  qgsvirtuallayerblob.cpp : Functions to manipulate Spatialite geometry blobs

begin                : Nov 2015
copyright            : (C) 2015 Hugo Mercier, Oslandia
email                : hugo dot mercier at oslandia dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsvirtuallayerblob.h"
#include <string.h>
#include <limits>

SpatialiteBlobHeader::SpatialiteBlobHeader()
    : start( 0x00 )
    , endianness( 0x01 )
    , srid( -1 )
    , mbrMinX( -DBL_MAX )
    , mbrMinY( -DBL_MAX )
    , mbrMaxX( DBL_MAX )
    , mbrMaxY( DBL_MAX )
    , end( 0x7C )
{}

void SpatialiteBlobHeader::readFrom( const char* p )
{
  // we cannot use directly memcpy( this, p, sizeof(this) ),
  // since there may be padding between struct members
  memcpy( &start, p, 1 );
  p++;
  memcpy( &endianness, p, 1 );
  p++;
  memcpy( &srid, p, 4 );
  p += 4;
  memcpy( &mbrMinX, p, 8 );
  p += 8;
  memcpy( &mbrMinY, p, 8 );
  p += 8;
  memcpy( &mbrMaxX, p, 8 );
  p += 8;
  memcpy( &mbrMaxY, p, 8 );
  p += 8;
  memcpy( &end, p, 1 );
}

void SpatialiteBlobHeader::writeTo( char* p ) const
{
  // we cannot use directly memcpy( this, p, sizeof(this) ),
  // since there may be padding between struct members
  memcpy( p, &start, 1 );
  p++;
  memcpy( p, &endianness, 1 );
  p++;
  memcpy( p, &srid, 4 );
  p += 4;
  memcpy( p, &mbrMinX, 8 );
  p += 8;
  memcpy( p, &mbrMinY, 8 );
  p += 8;
  memcpy( p, &mbrMaxX, 8 );
  p += 8;
  memcpy( p, &mbrMaxY, 8 );
  p += 8;
  memcpy( p, &end, 1 );
}

//
// Convert a QgsGeometry into a Spatialite geometry BLOB
void qgsGeometryToSpatialiteBlob( const QgsGeometry &geom, int32_t srid, char *&blob, int &size )
{
  const int header_len = SpatialiteBlobHeader::length;

  const int wkb_size = geom.wkbSize();
  size = header_len + wkb_size;
  blob = new char[size];

  char* p = blob;

  // write the header
  SpatialiteBlobHeader pHeader;
  QgsRectangle bbox = const_cast<QgsGeometry&>( geom ).boundingBox(); // boundingBox should be const
  pHeader.srid = srid;
  pHeader.mbrMinX = bbox.xMinimum();
  pHeader.mbrMinY = bbox.yMinimum();
  pHeader.mbrMaxX = bbox.xMaximum();
  pHeader.mbrMaxY = bbox.yMaximum();
  pHeader.writeTo( blob );

  p += header_len;

  // wkb of the geometry is
  // name         size    value
  // endianness     1      01
  // type           4      int

  // blob geometry = header + wkb[1:] + 'end'

  // copy wkb
  const unsigned char* wkb = geom.asWkb();

  memcpy( p, wkb + 1, wkb_size - 1 );
  p += wkb_size - 1;

  // end marker
  *p = '\xFE';
}

//
// Return the bouding box of a spatialite geometry blob
QgsRectangle spatialiteBlobBbox( const char* blob, size_t size )
{
  Q_UNUSED( size );

  SpatialiteBlobHeader h;
  h.readFrom( blob );

  return QgsRectangle( h.mbrMinX, h.mbrMinY, h.mbrMaxX, h.mbrMaxY );
}

void copySpatialiteSingleWkbToQgsGeometry( QgsWKBTypes::Type type, const char* iwkb, char* owkb, uint32_t& osize )
{
  int n_dims = QgsWKBTypes::coordDimensions( type );
  switch ( QgsWKBTypes::flatType( type ) )
  {
    case QgsWKBTypes::Point:
      memcpy( owkb, iwkb, n_dims*8 );
      iwkb += n_dims * 8;
      iwkb += n_dims * 8;
      osize = n_dims * 8;
      break;
    case QgsWKBTypes::LineString:
    {
      uint32_t n_points = *( reinterpret_cast<const uint32_t*>( iwkb ) );
      memcpy( owkb, iwkb, 4 );
      iwkb += 4;
      owkb += 4;
      for ( uint32_t i = 0; i < n_points; i++ )
      {
        memcpy( owkb, iwkb, n_dims*8 );
        iwkb += n_dims * 8;
        owkb += n_dims * 8;
      }
      osize += n_dims * 8 * n_points + 4;
      break;
    }
    case QgsWKBTypes::Polygon:
    {
      uint32_t n_rings = *( reinterpret_cast<const uint32_t*>( iwkb ) );
      memcpy( owkb, iwkb, 4 );
      iwkb += 4;
      owkb += 4;
      osize = 4;
      for ( uint32_t i = 0; i < n_rings; i++ )
      {
        uint32_t n_points = *( reinterpret_cast<const uint32_t*>( iwkb ) );
        memcpy( owkb, iwkb, 4 );
        iwkb += 4;
        owkb += 4;
        osize += 4;
        for ( uint32_t j = 0; j < n_points; j++ )
        {
          memcpy( owkb, iwkb, n_dims*8 );
          iwkb += n_dims * 8;
          owkb += n_dims * 8;
          osize += n_dims * 8;
        }
      }
      break;
    }
    default:
      break;
  }
}

//
// copy the spatialite blob to wkb for qgsgeometry
// the only difference is
// each spatialite sub geometry begins with the byte 0x69 (ENTITY)
// which should be converted to an endianness code
void copySpatialiteCollectionWkbToQgsGeometry( const char* iwkb, char* owkb, uint32_t& osize, int endianness )
{
  // copy first byte + type
  memcpy( owkb, iwkb, 5 );

  // replace 0x69 by the endianness
  owkb[0] = endianness;

  QgsWKBTypes::Type type = static_cast<QgsWKBTypes::Type>( *( reinterpret_cast<const uint32_t*>( iwkb + 1 ) ) );

  if ( QgsWKBTypes::isMultiType( type ) )
  {
    // multi type
    uint32_t n_elements = *( reinterpret_cast<const  uint32_t* >( iwkb + 5 ) );
    memcpy( owkb + 5, iwkb + 5, 4 );
    uint32_t p = 0;
    for ( uint32_t i = 0; i < n_elements; i++ )
    {
      uint32_t rsize = 0;
      copySpatialiteCollectionWkbToQgsGeometry( iwkb + 9 + p, owkb + 9 + p, rsize, endianness );
      p += rsize;
    }
    osize = p + 9;
  }
  else
  {
    osize = 0;
    copySpatialiteSingleWkbToQgsGeometry( type, iwkb + 5, owkb + 5, osize );
    osize += 5;
  }
}

QgsGeometry spatialiteBlobToQgsGeometry( const char* blob, size_t size )
{
  const int header_size = SpatialiteBlobHeader::length;
  const int wkb_size = static_cast< const int >( size - header_size );
  char* wkb = new char[wkb_size];

  uint32_t osize = 0;
  copySpatialiteCollectionWkbToQgsGeometry( blob + header_size - 1, wkb, osize, /*endianness*/blob[1] );

  QgsGeometry geom;
  geom.fromWkb( reinterpret_cast< unsigned char* >( wkb ), wkb_size );
  return geom;
}

QPair<QgsWKBTypes::Type, long> spatialiteBlobGeometryType( const char* blob, size_t size )
{
  if ( size < SpatialiteBlobHeader::length + 4 ) // the header + the type on 4 bytes
  {
    return qMakePair( QgsWKBTypes::NoGeometry, long( 0 ) );
  }

  uint32_t srid = *( reinterpret_cast< const uint32_t* >( blob + 2 ) );
  uint32_t type = *( reinterpret_cast< const uint32_t* >( blob + SpatialiteBlobHeader::length ) );

  return qMakePair( static_cast<QgsWKBTypes::Type>( type ), long( srid ) );
}
