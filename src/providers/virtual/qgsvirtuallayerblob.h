/***************************************************************************
  qgsvirtuallayerblob.h : Functions to manipulate Spatialite geometry blobs
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

#ifndef QGSVIRTUALLAYER_BLOB_H
#define QGSVIRTUALLAYER_BLOB_H

#include <stdint.h>

#include <qgsgeometry.h>

// BLOB header
// name    size    value
// start     1      00
// endian    1      01
// srid      4      int
// mbr_min_x 8      double
// mbr_min_y 8      double
// mbr_max_x 8      double
// mbr_max_y 8      double
// mbr_end   1      7C
struct SpatialiteBlobHeader
{
  unsigned char start;
  unsigned char endianness;
  int32_t srid;
  double mbrMinX;
  double mbrMinY;
  double mbrMaxX;
  double mbrMaxY;
  unsigned char end;

  SpatialiteBlobHeader();

  static const size_t length = 39;

  void readFrom( const char* p );

  void writeTo( char* p ) const;
};

//!
//! Convert a QgsGeometry into a Spatialite geometry BLOB
//! The blob will be allocated and must be handled by the caller
void qgsGeometryToSpatialiteBlob( const QgsGeometry& geom, int32_t srid, char *&blob, int &size );

//!
//! Return the bouding box of a spatialite geometry blob
QgsRectangle spatialiteBlobBbox( const char* blob, size_t size );

//!
//! Convert a Spatialite geometry BLOB to a QgsGeometry
QgsGeometry spatialiteBlobToQgsGeometry( const char* blob, size_t size );

//!
//! Get geometry type and srid from a spatialite geometry blob
QPair<QgsWKBTypes::Type, long> spatialiteBlobGeometryType( const char* blob, size_t size );

#endif
