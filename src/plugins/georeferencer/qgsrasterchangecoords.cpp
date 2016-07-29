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

#include <qgspoint.h>
#include <gdal.h>

#include <QFile>

#if defined(GDAL_VERSION_NUM) && GDAL_VERSION_NUM >= 1800
#define TO8F(x) (x).toUtf8().constData()
#else
#define TO8F(x) QFile::encodeName( x ).constData()
#endif

QgsRasterChangeCoords::QgsRasterChangeCoords()
    : mHasCrs( false )
    , mUL_X( 0. )
    , mUL_Y( 0. )
    , mResX( 1. )
    , mResY( 1. )
{
}

void QgsRasterChangeCoords::setRaster( const QString &fileRaster )
{
  GDALAllRegister();
  GDALDatasetH hDS = GDALOpen( TO8F( fileRaster ), GA_ReadOnly );
  double adfGeoTransform[6];
  if ( GDALGetProjectionRef( hDS ) && GDALGetGeoTransform( hDS, adfGeoTransform ) == CE_None )
    //if ( false )
  {
    mHasCrs = true;
    mUL_X = adfGeoTransform[0];
    mUL_Y = adfGeoTransform[3];
    mResX = adfGeoTransform[1];
    mResY = adfGeoTransform[5];
  }
  else
  {
    mHasCrs = false;
  }
  GDALClose( hDS );
}

QVector<QgsPoint> QgsRasterChangeCoords::getPixelCoords( const QVector<QgsPoint> &mapCoords )
{
  const int size = mapCoords.size();
  QVector<QgsPoint> pixelCoords( size );
  for ( int i = 0; i < size; i++ )
  {
    pixelCoords[i] = toColumnLine( mapCoords.at( i ) );
  }
  return pixelCoords;
}

QgsRectangle QgsRasterChangeCoords::getBoundingBox( const QgsRectangle &rect, bool toPixel )
{
  QgsRectangle rectReturn;
  QgsPoint p1( rect.xMinimum(), rect.yMinimum() );
  QgsPoint p2( rect.xMaximum(), rect.yMaximum() );
  QgsPoint( QgsRasterChangeCoords::* func )( const QgsPoint & );

  func = toPixel ? &QgsRasterChangeCoords::toColumnLine : &QgsRasterChangeCoords::toXY;
  rectReturn.set(( this->*func )( p1 ), ( this->*func )( p2 ) );

  return rectReturn;
}

QgsPoint QgsRasterChangeCoords::toColumnLine( const QgsPoint &pntMap )
{
  double col = ( pntMap.x() - mUL_X ) / mResX;
  double line = ( mUL_Y - pntMap.y() ) / mResY;
  return QgsPoint( col, line );
}

QgsPoint QgsRasterChangeCoords::toXY( const QgsPoint &pntPixel )
{
  double x = mUL_X + ( pntPixel.x() *  mResX );
  double y = mUL_Y + ( pntPixel.y() * -mResY );
  return QgsPoint( x, y );
}
