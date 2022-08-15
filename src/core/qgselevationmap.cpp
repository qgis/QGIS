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


QgsElevationMap::QgsElevationMap( const QSize &size )
  : mElevationImage( size, QImage::Format_ARGB32 )
{
  mElevationImage.fill( 0 );
  mPainter.reset( new QPainter );
  mPainter->begin( &mElevationImage );
}

QRgb QgsElevationMap::encodeElevation( float z )
{
  double zScaled = ( z + 8000 ) * 1000;
  unsigned int zInt = static_cast<unsigned int>( std::clamp( zScaled, 0., 16777215. ) );   // make sure to fit into 3 bytes
  return QRgb( zInt | ( 0xff << 24 ) );
}

float QgsElevationMap::decodeElevation( QRgb colorRaw )
{
  unsigned int zScaled = colorRaw & 0xffffff;
  return ( ( double ) zScaled ) / 1000 - 8000;
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
  int imgWidth = img.width();
  QRgb *imgPtr = reinterpret_cast<QRgb *>( img.bits() );
  const QRgb *elevPtr = reinterpret_cast<const QRgb *>( mElevationImage.constBits() );

  const int neighbours[] = { -1, 0, 1, 0, 0, -1, 0, 1 };
  for ( int i = distance; i < img.width() - distance; ++i )
  {
    for ( int j = distance; j < img.height() - distance; ++j )
    {
      float factor = 0.0f;
      float centerDepth = decodeElevation( elevPtr[ j * imgWidth + i ] );
      for ( int k = 0; k < 4; ++k )
      {
        int iNeighbour = i + distance * neighbours[2 * k];
        int jNeighbour = j + distance * neighbours[2 * k + 1];
        float neighbourDepth = decodeElevation( elevPtr[ jNeighbour * imgWidth + iNeighbour ] );
        factor += std::max<float>( 0, -( centerDepth - neighbourDepth ) );
      }
      float shade = exp( -factor * strength / rendererScale );

      QRgb c = imgPtr[ j * imgWidth + i ];
      imgPtr[ j * imgWidth + i ] = qRgba( qRed( c ) * shade, qGreen( c ) * shade, qBlue( c ) * shade, qAlpha( c ) );
    }
  }
}
