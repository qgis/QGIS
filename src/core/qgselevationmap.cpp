/***************************************************************************
  qgselevationmap.cpp
  --------------------------------------
  Date                 : August 2022
  Copyright            : (C) 2022 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgselevationmap.h"

#include "qgsrasterblock.h"

#include <QPainter>
#include <algorithm>
#include <cmath>


static const int ELEVATION_OFFSET = 8000;
static const int ELEVATION_SCALE = 1000;


QgsElevationMap::QgsElevationMap( const QSize &size )
  : mElevationImage( size, QImage::Format_ARGB32 )
{
  mElevationImage.fill( 0 );
  mPainter.reset( new QPainter );
  mPainter->begin( &mElevationImage );
}

QRgb QgsElevationMap::encodeElevation( float z )
{
  double zScaled = ( z + ELEVATION_OFFSET ) * ELEVATION_SCALE;
  unsigned int zInt = static_cast<unsigned int>( std::clamp( zScaled, 0., 16777215. ) );   // make sure to fit into 3 bytes
  return QRgb( zInt | ( 0xff << 24 ) );
}

float QgsElevationMap::decodeElevation( QRgb colorRaw )
{
  unsigned int zScaled = colorRaw & 0xffffff;
  return ( ( double ) zScaled ) / ELEVATION_SCALE - ELEVATION_OFFSET;
}

std::unique_ptr<QgsElevationMap> QgsElevationMap::fromRasterBlock( QgsRasterBlock *block )
{
  std::unique_ptr<QgsElevationMap> elevMap( new QgsElevationMap( QSize( block->width(), block->height() ) ) );
  QRgb *dataPtr = reinterpret_cast<QRgb *>( elevMap->mElevationImage.bits() );
  for ( int row = 0; row < block->height(); ++row )
  {
    for ( int col = 0; col < block->width(); ++col )
    {
      bool isNoData;
      double value = block->valueAndNoData( row, col, isNoData );
      if ( !isNoData )
        *dataPtr = encodeElevation( value );
      ++dataPtr;
    }
  }
  return elevMap;
}

void QgsElevationMap::applyEyeDomeLighting( QImage &img, int distance, float strength, float rendererScale )
{
  const int imgWidth = img.width(), imgHeight = img.height();
  QRgb *imgPtr = reinterpret_cast<QRgb *>( img.bits() );
  const QRgb *elevPtr = reinterpret_cast<const QRgb *>( mElevationImage.constBits() );

  const int neighbours[] = { -1, 0, 1, 0, 0, -1, 0, 1 };
  for ( int i = distance; i < imgWidth - distance; ++i )
  {
    for ( int j = distance; j < imgHeight - distance; ++j )
    {
      qgssize index = j * static_cast<qgssize>( imgWidth ) + i;
      float factor = 0.0f;
      float centerDepth = decodeElevation( elevPtr[ index ] );
      for ( int k = 0; k < 4; ++k )
      {
        int iNeighbour = i + distance * neighbours[2 * k];
        int jNeighbour = j + distance * neighbours[2 * k + 1];
        qgssize neighbourIndex = jNeighbour * static_cast<qgssize>( imgWidth ) + iNeighbour;
        float neighbourDepth = decodeElevation( elevPtr[ neighbourIndex ] );
        factor += std::max<float>( 0, -( centerDepth - neighbourDepth ) );
      }
      float shade = exp( -factor * strength / rendererScale );

      QRgb c = imgPtr[ index ];
      imgPtr[ index ] = qRgba( qRed( c ) * shade, qGreen( c ) * shade, qBlue( c ) * shade, qAlpha( c ) );
    }
  }
}
