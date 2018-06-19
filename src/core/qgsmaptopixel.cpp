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
#include "qgspointxy.h"


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
  , mXCenter( xc )
  , mYCenter( yc )
  , xMin( xc - ( mWidth * mMapUnitsPerPixel / 2.0 ) )
  , yMin( yc - ( mHeight * mMapUnitsPerPixel / 2.0 ) )
{
  Q_ASSERT( mapUnitsPerPixel > 0 );
  updateMatrix();
}

QgsMapToPixel::QgsMapToPixel( double mapUnitsPerPixel )
  : mMapUnitsPerPixel( mapUnitsPerPixel )
  , mWidth( 0 )
  , mHeight( 0 )
  , mXCenter( 0 )
  , mYCenter( 0 )
{
  updateMatrix();
}

QgsMapToPixel QgsMapToPixel::fromScale( double scale, QgsUnitTypes::DistanceUnit mapUnits, double dpi )
{
  double metersPerPixel = 25.4 / dpi / 1000.0;
  double mapUnitsPerPixel = metersPerPixel * QgsUnitTypes::fromUnitToUnitFactor( QgsUnitTypes::DistanceMeters, mapUnits );
  return QgsMapToPixel( mapUnitsPerPixel * scale );
}

QgsMapToPixel::QgsMapToPixel()
{
  updateMatrix();
}

int QgsMapToPixel::mapHeight() const
{
  return mHeight;
}

int QgsMapToPixel::mapWidth() const
{
  return mWidth;
}

bool QgsMapToPixel::updateMatrix()
{
  QTransform newMatrix = transform();

  // https://issues.qgis.org/issues/12757
  if ( !newMatrix.isInvertible() )
    return false;

  mMatrix = newMatrix;
  return true;
}

QgsPointXY QgsMapToPixel::toMapPoint( double x, double y ) const
{
  bool invertible;
  QTransform matrix = mMatrix.inverted( &invertible );
  assert( invertible );
  qreal mx, my;
  qreal x_qreal = x, y_qreal = y;
  matrix.map( x_qreal, y_qreal, &mx, &my );
  //QgsDebugMsg(QString("XXX toMapPoint x:%1 y:%2 -> x:%3 y:%4").arg(x).arg(y).arg(mx).arg(my));
  return QgsPointXY( mx, my );
}

QgsPointXY QgsMapToPixel::toMapCoordinates( QPoint p ) const
{
  QgsPointXY mapPt = toMapPoint( p.x(), p.y() );
  return QgsPointXY( mapPt );
}

QgsPointXY QgsMapToPixel::toMapCoordinates( int x, int y ) const
{
  return toMapPoint( x, y );
}

QgsPointXY QgsMapToPixel::toMapCoordinatesF( double x, double y ) const
{
  return toMapPoint( x, y );
}

void QgsMapToPixel::setMapUnitsPerPixel( double mapUnitsPerPixel )
{
  double oldUnits = mMapUnitsPerPixel;
  mMapUnitsPerPixel = mapUnitsPerPixel;
  if ( !updateMatrix() )
  {
    mMapUnitsPerPixel = oldUnits;
  }
}

double QgsMapToPixel::mapUnitsPerPixel() const
{
  return mMapUnitsPerPixel;
}

void QgsMapToPixel::setMapRotation( double degrees, double cx, double cy )
{
  double oldRotation = mRotation;
  double oldXCenter = mXCenter;
  double oldYCenter = mYCenter;
  double oldWidth = mWidth;

  mRotation = degrees;
  mXCenter = cx;
  mYCenter = cy;
  if ( mWidth < 0 )
  {
    // set width not that we can compute it
    mWidth = ( ( mXCenter - xMin ) * 2 ) / mMapUnitsPerPixel;
  }

  if ( !updateMatrix() )
  {
    mRotation = oldRotation;
    mXCenter = oldXCenter;
    mYCenter = oldYCenter;
    mWidth = oldWidth;
  }
}

double QgsMapToPixel::mapRotation() const
{
  return mRotation;
}

void QgsMapToPixel::setParameters( double mapUnitsPerPixel,
                                   double xc,
                                   double yc,
                                   int width,
                                   int height,
                                   double rotation )
{
  double oldMUPP = mMapUnitsPerPixel;
  double oldXCenter = mXCenter;
  double oldYCenter = mYCenter;
  double oldWidth = mWidth;
  double oldHeight = mHeight;
  double oldRotation = mRotation;
  double oldXMin = xMin;
  double oldYMin = yMin;

  mMapUnitsPerPixel = mapUnitsPerPixel;
  mXCenter = xc;
  mYCenter = yc;
  mWidth = width;
  mHeight = height;
  mRotation = rotation;
  xMin = xc - ( mWidth * mMapUnitsPerPixel / 2.0 );
  yMin = yc - ( mHeight * mMapUnitsPerPixel / 2.0 );

  if ( !updateMatrix() )
  {
    mMapUnitsPerPixel = oldMUPP;
    mXCenter = oldXCenter;
    mYCenter = oldYCenter;
    mWidth = oldWidth;
    mHeight = oldHeight;
    mRotation = oldRotation;
    xMin = oldXMin;
    yMin = oldYMin;
  }
}

QString QgsMapToPixel::showParameters() const
{
  QString rep;
  QTextStream( &rep ) << "Map units/pixel: " << mMapUnitsPerPixel
                      << " center: " << mXCenter << ',' << mYCenter
                      << " rotation: " << mRotation
                      << " size: " << mWidth << 'x' << mHeight;
  return rep;
}

QgsPointXY QgsMapToPixel::transform( qreal x, qreal y ) const
{
  transformInPlace( x, y );
  return QgsPointXY( x, y );
}

QgsPointXY QgsMapToPixel::transform( const QgsPointXY &p ) const
{
  qreal x = p.x(), y = p.y();
  transformInPlace( x, y );
// QgsDebugMsg(QString("Point to pixel...X : %1-->%2, Y: %3 -->%4").arg(p.x()).arg(dx).arg(p.y()).arg(dy));
  return QgsPointXY( x, y );
}

void QgsMapToPixel::transform( QgsPointXY *p ) const
{
  qreal x = p->x(), y = p->y();
  transformInPlace( x, y );
// QgsDebugMsg(QString("Point to pixel...X : %1-->%2, Y: %3 -->%4").arg(p->x()).arg(x).arg(p->y()).arg(y));
  p->set( x, y );
}

void QgsMapToPixel::transformInPlace( double &x, double &y ) const
{
  // Map 2 Pixel
  qreal mx, my;
  qreal x_qreal = x, y_qreal = y;
  mMatrix.map( x_qreal, y_qreal, &mx, &my );
  //QgsDebugMsg(QString("XXX transformInPlace X : %1-->%2, Y: %3 -->%4").arg(x).arg(mx).arg(y).arg(my));
  x = mx;
  y = my;
}

void QgsMapToPixel::transformInPlace( float &x, float &y ) const
{
  double mx = x, my = y;
  transformInPlace( mx, my );
  x = mx;
  y = my;
}

QTransform QgsMapToPixel::transform() const
{
  // NOTE: operations are done in the reverse order in which
  //       they are configured, so translation to geographical
  //       center happens first, then scaling, then rotation
  //       and finally translation to output viewport center

  double rotation = mapRotation();
  if ( qgsDoubleNear( rotation, 0.0 ) )
  {
    //no rotation, return a simplified matrix
    return QTransform::fromScale( 1.0 / mMapUnitsPerPixel, -1.0 / mMapUnitsPerPixel )
           .translate( -xMin, - ( yMin + mHeight * mMapUnitsPerPixel ) );
  }
  else
  {
    double cy = mapHeight() / 2.0;
    double cx = mapWidth() / 2.0;
    return QTransform::fromTranslate( cx, cy )
           .rotate( rotation )
           .scale( 1 / mMapUnitsPerPixel, -1 / mMapUnitsPerPixel )
           .translate( -mXCenter, -mYCenter );
  }
}
