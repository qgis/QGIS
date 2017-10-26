/***************************************************************************
  qgsvirtuallayerblob.h : Functions to manipulate SpatiaLite geometry blobs
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

#include "qgsgeometry.h"

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
  unsigned char start = 0x00;
  unsigned char endianness = 0x01;
  int32_t srid = -1;
  double mbrMinX = -DBL_MAX;
  double mbrMinY = -DBL_MAX;
  double mbrMaxX = DBL_MAX;
  double mbrMaxY = DBL_MAX;
  unsigned char end = 0x7C;

  SpatialiteBlobHeader() = default;

  static const size_t LENGTH = 39;

  void readFrom( const char *p );

  void writeTo( char *p ) const;
};

/**
 * Convert a QgsGeometry into a SpatiaLite geometry BLOB
 * The blob will be allocated and must be handled by the caller
 */
void qgsGeometryToSpatialiteBlob( const QgsGeometry &geom, int32_t srid, char *&blob, int &size );

/**
 * Return the bounding box of a SpatiaLite geometry blob
 */
QgsRectangle spatialiteBlobBbox( const char *blob, size_t size );

/**
 * Convert a SpatiaLite geometry BLOB to a QgsGeometry
 */
QgsGeometry spatialiteBlobToQgsGeometry( const char *blob, size_t size );

/**
 * Get geometry type and srid from a SpatiaLite geometry blob
 */
QPair<QgsWkbTypes::Type, long> spatialiteBlobGeometryType( const char *blob, size_t size );

#endif
