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
                              double ymax,
                              double ymin,
                              double xmin )
    : mMapUnitsPerPixel( mapUnitsPerPixel )
    , mHeight( ymax )
    , yMin( ymin )
    , xMin( xmin )
    , mMapRotation( 0 )
{
}

QgsMapToPixel::~QgsMapToPixel()
{
}

QTransform QgsMapToPixel::getMatrix() const
{
  // Matrix to go from screen (pixel) to map (geographical) units

  QTransform matrix;

  double cy = mHeight/2.0;
  double mWidth = ((xCenter-xMin)*2)/mMapUnitsPerPixel;
  double cx = mWidth/2.0;
  double rotation = mapRotation();

  // NOTE: operations are done in the reverse order in which
  //       they are configured, so rotation happens first,
  //       then scaling, then translation

  matrix.translate( xCenter, yCenter );
  matrix.scale( mMapUnitsPerPixel, -mMapUnitsPerPixel );
  // Rotate around viewport center
  matrix.rotate( -rotation );
  matrix.translate( -cx, -cy );

  //QgsDebugMsg(QString("XXX xMin:%1 yMin:%2 mHeight:%3 uPP:%4").arg(xMin).arg(yMin).arg(mHeight).arg(mMapUnitsPerPixel));
  QgsDebugMsg(QString("XXX xCent:%1 yCent:%2 mWidth:%3 mHeight:%4 uPP:%5 rot:%6")
    .arg(xCenter).arg(yCenter).arg(mWidth).arg(mHeight)
    .arg(mMapUnitsPerPixel).arg(rotation));

  return matrix;
}

QgsPoint QgsMapToPixel::toMapPoint( double x, double y ) const
{
  QTransform matrix = getMatrix();
  double mx, my;
  matrix.map(x, y, &mx, &my);
  QgsPoint ret( mx, my );

  //QgsDebugMsg(QString("XXX toMapPoint x:%1 y:%2 -> x:%3 y:%4").arg(x).arg(y).arg(mx).arg(my));

  return ret;
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
}

double QgsMapToPixel::mapUnitsPerPixel() const
{
  return mMapUnitsPerPixel;
}

void QgsMapToPixel::setMapRotation( double degrees, double cx, double cy )
{
  mMapRotation = degrees;
  xCenter = cx;
  yCenter = cy;
  assert(xCenter >= xMin);
  assert(yCenter >= yMin);
  //assert(yCenter <= yMin + mHeight*mMapUnitsPerPixel;
}

double QgsMapToPixel::mapRotation() const
{
  return mMapRotation;
}

void QgsMapToPixel::setViewportHeight( double height )
{
  mHeight = height;
}

void QgsMapToPixel::setYMinimum( double ymin )
{
  yMin = ymin;
}

void QgsMapToPixel::setXMinimum( double xmin )
{
  xMin = xmin;
}

void QgsMapToPixel::setParameters( double mapUnitsPerPixel, double xmin, double ymin, double ymax )
{
  mMapUnitsPerPixel = mapUnitsPerPixel;
  xMin = xmin;
  yMin = ymin;
  mHeight = ymax;

}

QString QgsMapToPixel::showParameters()
{
  QString rep;
  QTextStream( &rep ) << "Map units/pixel: " << mMapUnitsPerPixel
  << " X minimum: " << xMin << " Y minimum: " << yMin
  << " Height: " << mHeight << " Rotation: " << mMapRotation
  << " X center: " << xCenter << " Y center: " << yCenter;
  return rep;

}


QgsPoint QgsMapToPixel::transform( double x, double y ) const
{
  transformInPlace( x, y );
  return QgsPoint( x, y );
}

QgsPoint QgsMapToPixel::transform( const QgsPoint& p ) const
{
  double dx = p.x();
  double dy = p.y();
  transformInPlace( dx, dy );

// QgsDebugMsg(QString("Point to pixel...X : %1-->%2, Y: %3 -->%4").arg(p.x()).arg(dx).arg(p.y()).arg(dy));
  return QgsPoint( dx, dy );
}

void QgsMapToPixel::transform( QgsPoint* p ) const
{
  double x = p->x();
  double y = p->y();
  transformInPlace( x, y );

#ifdef QGISDEBUG
// QgsDebugMsg(QString("Point to pixel...X : %1-->%2, Y: %3 -->%4").arg(p->x()).arg(x).arg(p->y()).arg(y));
#endif
  p->set( x, y );
}

void QgsMapToPixel::transformInPlace( double& x, double& y ) const
{
  // Map 2 Pixel
  bool invertible;
  QTransform matrix = getMatrix().inverted(&invertible);
  assert(invertible);
  double mx, my;
  matrix.map(x, y, &mx, &my);
 QgsDebugMsg(QString("XXX transformInPlace X : %1-->%2, Y: %3 -->%4").arg(x).arg(mx).arg(y).arg(my));
  x = mx; y = my;
}

#ifdef QT_ARCH_ARM
void QgsMapToPixel::transformInPlace( qreal& x, qreal& y ) const
{
  double xd = y, yd = y;
  transformInPlace(xd, yd);
  x = xd; y = yd;
  //x = ( x - xMin ) / mMapUnitsPerPixel;
  //y = mHeight - ( y - yMin ) / mMapUnitsPerPixel;
}
#endif

void QgsMapToPixel::transformInPlace( QVector<double>& x,
                                      QVector<double>& y ) const
{
  assert(0);
  assert( x.size() == y.size() );
  for ( int i = 0; i < x.size(); ++i )
    transformInPlace( x[i], y[i] );
}

#ifdef ANDROID
void QgsMapToPixel::transformInPlace( float& x, float& y ) const
{
  double xd = y, yd = y;
  transformInPlace(xd, yd);
  x = xd; y = yd;
  //x = ( x - xMin ) / mMapUnitsPerPixel;
  //y = mHeight - ( y - yMin ) / mMapUnitsPerPixel;
}

void QgsMapToPixel::transformInPlace( QVector<float>& x,
                                      QVector<float>& y ) const
{
  assert( x.size() == y.size() );
  for ( unsigned int i = 0; i < x.size(); ++i )
    transformInPlace( x[i], y[i] );
}
#endif

