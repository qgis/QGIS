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
#include "qgslogger.h"

QgsMapToPixel::QgsMapToPixel( double mapUnitsPerPixel,
                              double ymax,
                              double ymin,
                              double xmin )
    : mMapUnitsPerPixel( mapUnitsPerPixel ),
    yMax( ymax ),
    yMin( ymin ),
    xMin( xmin ),
    xMax( 0 )                 // XXX wasn't originally specified?  Why?
{
}

QgsMapToPixel::~QgsMapToPixel()
{
}

QgsPoint QgsMapToPixel::toMapPoint( double x, double y ) const
{
  double mx = x * mMapUnitsPerPixel + xMin;
  double my = -1 * (( y - yMax ) * mMapUnitsPerPixel - yMin );
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
}

double QgsMapToPixel::mapUnitsPerPixel() const
{
  return mMapUnitsPerPixel;
}

void QgsMapToPixel::setYMaximum( double ymax )
{
  yMax = ymax;
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
  yMax = ymax;

}

QString QgsMapToPixel::showParameters()
{
  QString rep;
  QTextStream( &rep ) << "Map units/pixel: " << mMapUnitsPerPixel
  << " X minimum: " << xMin << " Y minimum: " << yMin << " Y maximum: " << yMax;
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
  x = ( x - xMin ) / mMapUnitsPerPixel;
  y = yMax - ( y - yMin ) / mMapUnitsPerPixel;
}

void QgsMapToPixel::transformInPlace( std::vector<double>& x,
                                      std::vector<double>& y ) const
{
  assert( x.size() == y.size() );
  for ( unsigned int i = 0; i < x.size(); ++i )
    transformInPlace( x[i], y[i] );
}
