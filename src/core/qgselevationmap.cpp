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

#include <QPainter>
#include <cmath>

const double QgsElevationMap::mZMin = -5000;
const double QgsElevationMap::mZMax = +5000;


QgsElevationMap::QgsElevationMap( const QSize &size )
  : mElevationImage( size, QImage::Format_ARGB32 )
{
  mElevationImage.fill( QColor::fromRgbF( 0, 0, 0, 0 ) );
  mPainter.reset( new QPainter );
  mPainter->begin( &mElevationImage );
}


QColor QgsElevationMap::encodeElevation( float z )
{
  float zFloat = ( z - mZMin ) / ( mZMax - mZMin );
  zFloat = std::clamp<float>( zFloat, 0.0, 1.0 );

  int zInt = zFloat * 0x00ffffff;

  QColor c;
  c.setRed( ( zInt & 0xff0000 ) >> 16 );
  c.setGreen( ( zInt & 0xff00 ) >> 8 );
  c.setBlue( zInt & 0xff );
  c.setAlphaF( 1.0f );
  return c;
}


float QgsElevationMap::decodeElevation( const QRgb *colorRaw )
{
  float z = ( float )( ( *colorRaw ) & 0x00ffffff ) / ( ( float ) 0x00ffffff );
  z *= mZMax - mZMin;
  return z;
}


void QgsElevationMap::applyEyeDomeLighting( QImage &img, int distance, float strength, float zScale )
{
  int imgWidth = img.width();
  uchar *imgPtr = img.bits();
  const QRgb *elevPtr = reinterpret_cast<const QRgb *>( mElevationImage.constBits() );
  for ( int i = distance; i < img.width() - distance; ++i )
  {
    for ( int j = distance; j < img.height() - distance; ++j )
    {
      int neighbours[] = { -1, 0, 1, 0, 0, -1, 0, 1 };
      float factor = 0.0f;
      float centerDepth = decodeElevation( elevPtr + ( j * imgWidth + i ) );
      for ( int k = 0; k < 4; ++k )
      {
        int iNeighbour = i + distance * neighbours[2 * k];
        int jNeighbour = j + distance * neighbours[2 * k + 1];
        float neighbourDepth = decodeElevation( elevPtr + ( jNeighbour * imgWidth + iNeighbour ) );
        factor += std::max<float>( 0, -( centerDepth - neighbourDepth ) );
      }
      factor /= zScale;
      float shade = exp( -factor / 4 * strength );

      uchar *imgPixel = imgPtr + ( j * imgWidth + i ) * 4;
      uchar &red = *( imgPixel );
      uchar &green = *( imgPixel + 1 );
      uchar &blue = *( imgPixel + 2 );

      red = red * shade;
      green = green * shade;
      blue = blue * shade;
    }
  }
}
