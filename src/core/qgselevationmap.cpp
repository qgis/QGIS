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
}

QgsElevationMap::QgsElevationMap( const QImage &image )
  :  mElevationImage( image )
{}

QgsElevationMap::QgsElevationMap( const QgsElevationMap &other )
  :   mElevationImage( other.mElevationImage )
{}

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

QgsElevationMap &QgsElevationMap::operator=( const QgsElevationMap &other )
{
  mPainter.reset();
  mElevationImage = other.mElevationImage;
  return *this;
}

void QgsElevationMap::applyEyeDomeLighting( QImage &img, int distance, float strength, float rendererScale ) const
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

void QgsElevationMap::applyHillShading( QImage &img, bool multiDirectional, double altitude, double azimuth, double zFactor, double cellSizeX, double cellSizeY ) const
{
  // algs from  qgshillshaderenderer.cpp
  double zenithRad = std::max( 0.0, 90 - altitude ) * M_PI / 180.0 ;
  double cosZenithRad = std::cos( zenithRad );
  double sin_altRadian = std::sin( zenithRad );
  double cos_alt_mul_z = cosZenithRad * zFactor ;
  double azimuthRad = -1 * azimuth * M_PI / 180.0;
  double cos_az_mul_cos_alt_mul_z = std::cos( azimuthRad ) * cos_alt_mul_z;
  double sin_az_mul_cos_alt_mul_z = std::sin( azimuthRad ) * cos_alt_mul_z;

  // From qgshillshaderenderer.cpp, -32.87001872802012 comes from ( 127.0 * std::cos(225.0 *  M_PI / 180.0))
  // but this expression actually equals -89.8025612106916. To be consistent, we keep -32.87001872802012
  // and divide it by 254 because, here, values are between 0.0 and 1.0, not between 0 and 255
  double cos225_az_mul_cos_alt_mul_z_mul_0_5 = -32.87001872802012 * cos_alt_mul_z / 255;
  double square_z =  zFactor * zFactor;

  QRgb *imgPtr = reinterpret_cast<QRgb *>( img.bits() );
  const QRgb *elevPtr = reinterpret_cast<const QRgb *>( mElevationImage.constBits() );

  const int imgWidth = img.width(), imgHeight = img.height();

  auto colRowToIndex = [&]( int c, int r )->qgssize
  {
    return  static_cast<qgssize>( r * imgWidth + c );
  };

  double defaultValue = decodeElevation( elevPtr[colRowToIndex( 0, 0 )] );
  double pixelValues[9] =
  {
    0, defaultValue, 0,
    0, defaultValue, 0,
    0, decodeElevation( elevPtr[colRowToIndex( 0, 1 )] ), 0
  };


  for ( int rowC = 0; rowC < imgHeight ; ++rowC )
  {
    int rowU = std::max( 0, rowC - 1 );
    int rowD = std::min( imgHeight - 1, rowC + 1 );

    pixelValues[1] = decodeElevation( elevPtr[colRowToIndex( 0, rowU )] );
    pixelValues[4] = decodeElevation( elevPtr[colRowToIndex( 0, rowC )] );
    pixelValues[7] = decodeElevation( elevPtr[colRowToIndex( 0, rowD )] );

    pixelValues[2] = decodeElevation( elevPtr[colRowToIndex( 1, rowU )] );
    pixelValues[5] = decodeElevation( elevPtr[colRowToIndex( 1, rowC )] );
    pixelValues[8] = decodeElevation( elevPtr[colRowToIndex( 1, rowD )] );


    for ( int colC = 0; colC < imgWidth  ; ++colC )
    {
      qgssize centerIndex = colRowToIndex( colC, rowC );
      int colR = std::min( imgWidth - 1, colC + 1 );

      pixelValues[0] = pixelValues[1];
      pixelValues[3] = pixelValues[4];
      pixelValues[6] = pixelValues[7];

      pixelValues[1] = pixelValues[2];
      pixelValues[4] = pixelValues[5];
      pixelValues[7] = pixelValues[8];

      pixelValues[2] = decodeElevation( elevPtr[colRowToIndex( colR, rowU )] );
      pixelValues[5] = decodeElevation( elevPtr[colRowToIndex( colR, rowC )] );
      pixelValues[8] = decodeElevation( elevPtr[colRowToIndex( colR, rowD )] );


      if ( elevPtr[centerIndex] != 0 )
      {
        // This is center cell. Use this in place of nodata neighbors
        const double x22 = pixelValues[4];

        const double x11 =  std::isnan( pixelValues[0] ) ? x22 : pixelValues[0];
        const double x21 =  std::isnan( pixelValues[3] ) ? x22 : pixelValues[3];
        const double x31 =  std::isnan( pixelValues[6] ) ? x22 : pixelValues[6];

        const double x12 = std::isnan( pixelValues[1] ) ? x22 : pixelValues[1];
        const double x32 =  std::isnan( pixelValues[7] ) ? x22 : pixelValues[7];

        const double x13 =  std::isnan( pixelValues[2] ) ? x22 : pixelValues[2];
        const double x23 =  std::isnan( pixelValues[5] ) ? x22 : pixelValues[5];
        const double x33 =  std::isnan( pixelValues[8] ) ? x22 : pixelValues[8];

        const double derX = ( ( x13 + x23 + x23 + x33 ) - ( x11 + x21 + x21 + x31 ) ) / ( 8 * cellSizeX );
        const double derY = ( ( x31 + x32 + x32 + x33 ) - ( x11 + x12 + x12 + x13 ) ) / ( 8 * -cellSizeY );

        double shade = 0;
        if ( !multiDirectional )
        {
          // Standard single direction hillshade
          shade = std::clamp( ( sin_altRadian -
                                ( derY * cos_az_mul_cos_alt_mul_z -
                                  derX * sin_az_mul_cos_alt_mul_z ) ) /
                              std::sqrt( 1 + square_z * ( derX * derX + derY * derY ) ),
                              0.0, 1.0 );
        }
        else
        {
          // Weighted multi direction as in http://pubs.usgs.gov/of/1992/of92-422/of92-422.pdf
          // Fast formula from GDAL DEM
          const double xx = derX * derX;
          const double yy = derY * derY;
          const double xx_plus_yy = xx + yy;
          // Flat?
          if ( xx_plus_yy == 0.0 )
          {
            shade = std::clamp( sin_altRadian, 0.0, 1.0 );
          }
          else
          {
            // ... then the shade value from different azimuth
            double val225_mul_0_5 = sin_altRadian * 0.5 +
                                    ( derX - derY ) * cos225_az_mul_cos_alt_mul_z_mul_0_5;
            val225_mul_0_5 = ( val225_mul_0_5 <= 0.0 ) ? 0.0 : val225_mul_0_5;
            double val270_mul_0_5 = sin_altRadian * 0.5 -
                                    derX * cos_alt_mul_z * 0.5;
            val270_mul_0_5 = ( val270_mul_0_5 <= 0.0 ) ? 0.0 : val270_mul_0_5;
            double val315_mul_0_5 = sin_altRadian * 0.5 +
                                    ( derX + derY ) * cos225_az_mul_cos_alt_mul_z_mul_0_5;
            val315_mul_0_5 = ( val315_mul_0_5 <= 0.0 ) ? 0.0 : val315_mul_0_5;
            double val360_mul_0_5 = sin_altRadian * 0.5 -
                                    derY * cos_alt_mul_z * 0.5;
            val360_mul_0_5 = ( val360_mul_0_5 <= 0.0 ) ? 0.0 : val360_mul_0_5;

            // ... then the weighted shading
            const double weight_225 = 0.5 * xx_plus_yy - derX * derY;
            const double weight_270 = xx;
            const double weight_315 = xx_plus_yy - weight_225;
            const double weight_360 = yy;
            const double cang_mul_127 = (
                                          ( weight_225 * val225_mul_0_5 +
                                            weight_270 * val270_mul_0_5 +
                                            weight_315 * val315_mul_0_5 +
                                            weight_360 * val360_mul_0_5 ) / xx_plus_yy ) /
                                        ( 1 + square_z * xx_plus_yy );

            shade = std::clamp( cang_mul_127, 0.0, 1.0 );
          }
        }

        QRgb c = imgPtr[ centerIndex ];
        imgPtr[ centerIndex ] = qRgba( qRed( c ) * shade, qGreen( c ) * shade, qBlue( c ) * shade, qAlpha( c ) );
      }
    }
  }
}

QPainter *QgsElevationMap::painter() const
{
  if ( !mPainter )
  {
    mPainter.reset( new QPainter );
    mPainter->begin( &mElevationImage );
  }
  return mPainter.get();

}

void QgsElevationMap::combine( const QgsElevationMap &otherElevationMap )
{
  QRgb *elevPtr = reinterpret_cast<QRgb *>( mElevationImage.bits() );
  const QRgb *otherElevPtr = reinterpret_cast<const QRgb *>( otherElevationMap.mElevationImage.constBits() );

  int width = mElevationImage.width();
  int widthOther = otherElevationMap.mElevationImage.width();
  int combinedHeight = std::min( mElevationImage.height(), otherElevationMap.mElevationImage.height() );
  int combinedWidth = std::min( width,  widthOther );

  for ( int row = 0; row < combinedHeight; ++row )
  {
    for ( int col = 0; col < combinedWidth; ++col )
    {
      qgssize index = static_cast<qgssize>( col + row * width );
      qgssize indexOther = static_cast<qgssize>( col + row * widthOther );
      if ( otherElevPtr[indexOther] != 0 )
        elevPtr[index] = otherElevPtr[indexOther];
    }
  }
}


bool QgsElevationMap::isValid() const
{
  return mElevationImage.isNull();
}

void QgsElevationMap::fillWithRasterBlock( QgsRasterBlock *block, int top, int left )
{
  QRgb *dataPtr = reinterpret_cast<QRgb *>( mElevationImage.bits() );

  int widthMap = mElevationImage.width();
  int heightMap = mElevationImage.height();
  int widthBlock = block->width();
  int combinedHeight = std::min( mElevationImage.height(), block->height() + top );
  int combinedWidth = std::min( widthMap,  widthBlock + left );
  for ( int row = std::max( 0, top ); row < combinedHeight; ++row )
  {
    if ( row >= heightMap )
      continue;

    for ( int col = std::max( 0, left ); col < combinedWidth; ++col )
    {
      if ( col >= widthMap )
        continue;

      bool isNoData = true;
      double value = block->valueAndNoData( ( row - top ), ( col - left ), isNoData );
      qgssize index = static_cast<qgssize>( col + row * widthMap );
      if ( isNoData )
        dataPtr[index] = 0;
      else
        dataPtr[index] = encodeElevation( value );
    }
  }
}
