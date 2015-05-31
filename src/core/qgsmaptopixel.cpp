/***************************************************************************
                qgsmaptopixel.cpp  -  description
                             -------------------
    begin                : Sat Jun 22 2002
    copyright            : (C) 2002 by Gary E.Sherman
    email                : sherman at mrcc.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgsmaptopixel.h"

#include <QPoint>
#include <QTextStream>
#include <QVector>
#include <QTransform>

#include "qgslogger.h"

QgsMapToPixel::QgsMapToPixel( double mapUnitsPerPixel,
                              double xc,
                              double yc,
                              int width,
                              int height,
                              double rotation )
    : mMapUnitsPerPixel( mapUnitsPerPixel )
    , mWidth( width )
    , mHeight( height )
    , mRotation( rotation )
    , xCenter( xc )
    , yCenter( yc )
    , xMin( xc - ( mWidth * mMapUnitsPerPixel / 2.0 ) )
    , yMin( yc - ( mHeight * mMapUnitsPerPixel / 2.0 ) )
{
  Q_ASSERT( mapUnitsPerPixel > 0 );
  updateMatrix();
}

QgsMapToPixel::QgsMapToPixel()
    : mMapUnitsPerPixel( 1 )
    , mWidth( 1 )
    , mHeight( 1 )
    , mRotation( 0.0 )
    , xCenter( 0.5 )
    , yCenter( 0.5 )
    , xMin( 0 )
    , yMin( 0 )
{
  updateMatrix();
}

// @deprecated in 2.8
QgsMapToPixel::QgsMapToPixel( double mapUnitsPerPixel,
                              double height,
                              double ymin,
                              double xmin )
    : mMapUnitsPerPixel( mapUnitsPerPixel )
    , mWidth( -1 )
    , mHeight( height )
    , mRotation( 0.0 )
    , xCenter( 0.0 )
    , yCenter( 0.0 )
    , xMin( xmin )
    , yMin( ymin )
{
  updateMatrix();
}

QgsMapToPixel::~QgsMapToPixel()
{
}

int QgsMapToPixel::mapHeight() const
{
  return mHeight;
}

int QgsMapToPixel::mapWidth() const
{
  return mWidth;
}

void QgsMapToPixel::updateMatrix()
{
  mMatrix = transform();
}

QgsPoint QgsMapToPixel::toMapPoint( qreal x, qreal y ) const
{
  bool invertible;
  QTransform matrix = mMatrix.inverted( &invertible );
  assert( invertible );
  qreal mx, my;
  matrix.map( x, y, &mx, &my );
  //QgsDebugMsg(QString("XXX toMapPoint x:%1 y:%2 -> x:%3 y:%4").arg(x).arg(y).arg(mx).arg(my));
  return QgsPoint( mx, my );
}

QgsPoint QgsMapToPixel::toMapCoordinates( QPoint p ) const
{
  QgsPoint mapPt = toMapPoint( p.x(), p.y() );
  return QgsPoint( mapPt );
}

QgsPoint QgsMapToPixel::toMapCoordinates( int x, int y ) const
{
  return toMapPoint( x, y );
}

QgsPoint QgsMapToPixel::toMapCoordinatesF( double x, double y ) const
{
  return toMapPoint( x, y );
}

void QgsMapToPixel::setMapUnitsPerPixel( double mapUnitsPerPixel )
{
  mMapUnitsPerPixel = mapUnitsPerPixel;
  updateMatrix();
}

double QgsMapToPixel::mapUnitsPerPixel() const
{
  return mMapUnitsPerPixel;
}

void QgsMapToPixel::setMapRotation( double degrees, double cx, double cy )
{
  mRotation = degrees;
  xCenter = cx;
  yCenter = cy;
  if ( mWidth < 0 )
  {
    // set width not that we can compute it
    mWidth = (( xCenter - xMin ) * 2 ) / mMapUnitsPerPixel;
  }
  updateMatrix();
}

double QgsMapToPixel::mapRotation() const
{
  return mRotation;
}

// @deprecated in 2.8
void QgsMapToPixel::setYMinimum( double ymin )
{
  yCenter = ymin + mHeight * mMapUnitsPerPixel / 2.0;
  mRotation = 0.0;
  updateMatrix();
}

// @deprecated in 2.8
void QgsMapToPixel::setXMinimum( double xmin )
{
  xCenter = xmin + mWidth * mMapUnitsPerPixel / 2.0;
  mRotation = 0.0;
  updateMatrix();
}

// @deprecated in 2.8
void QgsMapToPixel::setParameters( double mapUnitsPerPixel, double xmin, double ymin, double ymax )
{
  mMapUnitsPerPixel = mapUnitsPerPixel;
  xMin = xmin;
  yMin = ymin;
  mHeight = ymax;
  xCenter = xmin + mWidth * mMapUnitsPerPixel / 2.0;
  yCenter = ymin + mHeight * mMapUnitsPerPixel / 2.0;
  mRotation = 0.0;
  updateMatrix();
}

void QgsMapToPixel::setParameters( double mapUnitsPerPixel,
                                   double xc,
                                   double yc,
                                   int width,
                                   int height,
                                   double rotation )
{
  mMapUnitsPerPixel = mapUnitsPerPixel;
  xCenter = xc;
  yCenter = yc;
  mWidth = width;
  mHeight = height;
  mRotation = rotation;
  xMin = xc - ( mWidth * mMapUnitsPerPixel / 2.0 );
  yMin = yc - ( mHeight * mMapUnitsPerPixel / 2.0 );
  updateMatrix();
}

QString QgsMapToPixel::showParameters() const
{
  QString rep;
  QTextStream( &rep ) << "Map units/pixel: " << mMapUnitsPerPixel
  << " center: " << xCenter << "," << yCenter
  << " rotation: " << mRotation
  << " size: " << mWidth << "x" << mHeight;
  return rep;
}

QgsPoint QgsMapToPixel::transform( qreal x, qreal y ) const
{
  transformInPlace( x, y );
  return QgsPoint( x, y );
}

QgsPoint QgsMapToPixel::transform( const QgsPoint &p ) const
{
  qreal x = p.x(), y = p.y();
  transformInPlace( x, y );
// QgsDebugMsg(QString("Point to pixel...X : %1-->%2, Y: %3 -->%4").arg(p.x()).arg(dx).arg(p.y()).arg(dy));
  return QgsPoint( x, y );
}

void QgsMapToPixel::transform( QgsPoint *p ) const
{
  qreal x = p->x(), y = p->y();
  transformInPlace( x, y );
// QgsDebugMsg(QString("Point to pixel...X : %1-->%2, Y: %3 -->%4").arg(p->x()).arg(x).arg(p->y()).arg(y));
  p->set( x, y );
}

void QgsMapToPixel::transformInPlace( qreal &x, qreal &y ) const
{
  // Map 2 Pixel
  qreal mx, my;
  mMatrix.map( x, y, &mx, &my );
  //QgsDebugMsg(QString("XXX transformInPlace X : %1-->%2, Y: %3 -->%4").arg(x).arg(mx).arg(y).arg(my));
  x = mx; y = my;
}

QTransform QgsMapToPixel::transform() const
{
  double rotation = mapRotation();
  if ( qgsDoubleNear( rotation, 0.0 ) )
  {
    //no rotation, return a simplified matrix
    return QTransform::fromScale( 1.0 / mMapUnitsPerPixel, -1.0 / mMapUnitsPerPixel )
           .translate( -xMin, - ( yMin + mHeight * mMapUnitsPerPixel ) );
  }

  double cy = mapHeight() / 2.0;
  double cx = mapWidth() / 2.0;
  return QTransform::fromTranslate( cx, cy )
         .rotate( rotation )
         .scale( 1 / mMapUnitsPerPixel, -1 / mMapUnitsPerPixel )
         .translate( -xCenter, -yCenter );
}
