/***************************************************************************
     qgsrasterchangecoords.cpp
     --------------------------------------
    Date                 : 25-June-2011
    Copyright            : (C) 2011 by Luiz Motta
    Email                : motta.luiz at gmail.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsrasterchangecoords.h"

#include "qgspoint.h"
#include "qgsogrutils.h"

#include <gdal.h>

#include <QFile>

void QgsRasterChangeCoords::setRaster( const QString &fileRaster )
{
  GDALAllRegister();
  const gdal::dataset_unique_ptr hDS( GDALOpen( fileRaster.toUtf8().constData(), GA_ReadOnly ) );
  double adfGeoTransform[6];
  if ( GDALGetProjectionRef( hDS.get() ) && GDALGetGeoTransform( hDS.get(), adfGeoTransform ) == CE_None )
  {
    mHasExistingGeoreference = true;
    mUL_X = adfGeoTransform[0];
    mUL_Y = adfGeoTransform[3];
    mResX = adfGeoTransform[1];
    mResY = adfGeoTransform[5];
  }
  else
  {
    mHasExistingGeoreference = false;
  }
}

QVector<QgsPointXY> QgsRasterChangeCoords::getPixelCoords( const QVector<QgsPointXY> &mapCoords )
{
  const int size = mapCoords.size();
  QVector<QgsPointXY> pixelCoords( size );
  for ( int i = 0; i < size; i++ )
  {
    pixelCoords[i] = toColumnLine( mapCoords.at( i ) );
  }
  return pixelCoords;
}

QgsRectangle QgsRasterChangeCoords::getBoundingBox( const QgsRectangle &rect, bool toPixel )
{
  QgsRectangle rectReturn;
  const QgsPointXY p1( rect.xMinimum(), rect.yMinimum() );
  const QgsPointXY p2( rect.xMaximum(), rect.yMaximum() );
  QgsPointXY( QgsRasterChangeCoords::* func )( const QgsPointXY & );

  func = toPixel ? &QgsRasterChangeCoords::toColumnLine : &QgsRasterChangeCoords::toXY;
  rectReturn.set( ( this->*func )( p1 ), ( this->*func )( p2 ) );

  return rectReturn;
}

QgsPointXY QgsRasterChangeCoords::toColumnLine( const QgsPointXY &pntMap )
{
  if ( ! mHasExistingGeoreference )
    return QgsPointXY( pntMap.x(), pntMap.y() );

  const double col = ( pntMap.x() - mUL_X ) / mResX;
  const double line = ( mUL_Y - pntMap.y() ) / mResY;
  return QgsPointXY( col, line );
}

QgsPointXY QgsRasterChangeCoords::toXY( const QgsPointXY &pntPixel )
{
  if ( ! mHasExistingGeoreference )
    return QgsPointXY( pntPixel.x(), pntPixel.y() );

  const double x = mUL_X + ( pntPixel.x() *  mResX );
  const double y = mUL_Y + ( pntPixel.y() * -mResY );
  return QgsPointXY( x, y );
}
