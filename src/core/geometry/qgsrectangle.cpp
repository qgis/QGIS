/***************************************************************************
                          qgsrectangle.cpp  -  description
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

#include "qgsgeometry.h"
#include "qgspointxy.h"
#include "qgsrectangle.h"
#include "qgslogger.h"
#include "qgsbox3d.h"
#include "qgspolygon.h"
#include "qgslinestring.h"

#include <QString>
#include <QTextStream>
#include <QTransform>

#include <algorithm>
#include <cmath>
#include <limits>

QgsRectangle QgsRectangle::fromWkt( const QString &wkt )
{
  const QgsGeometry geom = QgsGeometry::fromWkt( wkt );
  if ( geom.isEmpty() )
    return QgsRectangle();

  if ( const QgsPolygon *polygon = qgsgeometry_cast< const QgsPolygon * >( geom.constGet()->simplifiedTypeRef() ) )
  {
    if ( polygon->numInteriorRings() > 0 )
      return QgsRectangle();

    if ( const QgsLineString *exterior = qgsgeometry_cast< QgsLineString * >( polygon->exteriorRing() ) )
    {
      if ( exterior->numPoints() == 5
           && qgsDoubleNear( exterior->xAt( 0 ), exterior->xAt( 4 ) )
           && qgsDoubleNear( exterior->yAt( 0 ), exterior->yAt( 4 ) )
           && geom.isGeosValid() )
        return QgsRectangle( exterior->xAt( 0 ), exterior->yAt( 0 ), exterior->xAt( 2 ), exterior->yAt( 2 ) );
    }
  }
  return QgsRectangle();
}

QgsRectangle QgsRectangle::fromCenterAndSize( const QgsPointXY &center, double width, double height )
{
  const double xMin = center.x() - width / 2.0;
  const double xMax = xMin + width;
  const double yMin = center.y() - height / 2.0;
  const double yMax = yMin + height;
  return QgsRectangle( xMin, yMin, xMax, yMax );
}

QgsRectangle QgsRectangle::scaled( double scaleFactor, const QgsPointXY *center ) const
{
  QgsRectangle scaledRect = QgsRectangle( *this );
  scaledRect.scale( scaleFactor, center );
  return scaledRect;
}

QgsRectangle QgsRectangle::operator-( const QgsVector v ) const
{
  const double xmin = mXmin - v.x();
  const double xmax = mXmax - v.x();
  const double ymin = mYmin - v.y();
  const double ymax = mYmax - v.y();
  return QgsRectangle( xmin, ymin, xmax, ymax );
}

QgsRectangle QgsRectangle::operator+( const QgsVector v ) const
{
  const double xmin = mXmin + v.x();
  const double xmax = mXmax + v.x();
  const double ymin = mYmin + v.y();
  const double ymax = mYmax + v.y();
  return QgsRectangle( xmin, ymin, xmax, ymax );
}

QgsRectangle &QgsRectangle::operator-=( const QgsVector v )
{
  mXmin -= v.x();
  mXmax -= v.x();
  mYmin -= v.y();
  mYmax -= v.y();
  return *this;
}

QgsRectangle &QgsRectangle::operator+=( const QgsVector v )
{
  mXmin += v.x();
  mXmax += v.x();
  mYmin += v.y();
  mYmax += v.y();
  return *this;
}

QString QgsRectangle::asWktCoordinates() const
{
  QString rep =
    qgsDoubleToString( mXmin ) + ' ' + qgsDoubleToString( mYmin ) + QLatin1String( ", " ) +
    qgsDoubleToString( mXmax ) + ' ' + qgsDoubleToString( mYmax );

  return rep;
}

QString QgsRectangle::asWktPolygon() const
{
  if ( isNull() )
  {
    return QStringLiteral( "Polygon EMPTY" );
  }

  return QStringLiteral( "Polygon ((%1 %2, %3 %2, %3 %4, %1 %4, %1 %2))" ).arg(
           qgsDoubleToString( mXmin ),
           qgsDoubleToString( mYmin ),
           qgsDoubleToString( mXmax ),
           qgsDoubleToString( mYmax )
         );
}

QString QgsRectangle::toString( int precision ) const
{
  QString rep;

  if ( precision < 0 )
  {
    precision = 0;
    if ( ( width() < 10 || height() < 10 ) && ( width() > 0 && height() > 0 ) )
    {
      precision = static_cast<int>( std::ceil( -1.0 * std::log10( std::min( width(), height() ) ) ) ) + 1;
      // sanity check
      if ( precision > 20 )
        precision = 20;
    }
  }

  if ( isNull() )
    rep = QStringLiteral( "Null" );
  else
    rep = QStringLiteral( "%1,%2 : %3,%4" )
          .arg( mXmin, 0, 'f', precision )
          .arg( mYmin, 0, 'f', precision )
          .arg( mXmax, 0, 'f', precision )
          .arg( mYmax, 0, 'f', precision );

  QgsDebugMsgLevel( QStringLiteral( "Extents : %1" ).arg( rep ), 4 );

  return rep;
}

QString QgsRectangle::asPolygon() const
{
  if ( isNull() )
  {
    return QStringLiteral( "EMPTY" );
  }

  QString rep;

  QTextStream foo( &rep );

  foo.setRealNumberPrecision( 8 );
  foo.setRealNumberNotation( QTextStream::FixedNotation );
  // NOTE: a polygon isn't a polygon unless its closed. In the case of
  //       a rectangle, that means 5 points (last == first)
  foo
      << mXmin << ' ' << mYmin << ", "
      << mXmin << ' ' << mYmax << ", "
      << mXmax << ' ' << mYmax << ", "
      << mXmax << ' ' << mYmin << ", "
      << mXmin << ' ' << mYmin;

  return rep;

}

QgsBox3D QgsRectangle::toBox3d( double zMin, double zMax ) const
{
  return QgsBox3D( mXmin, mYmin, zMin, mXmax, mYmax, zMax );
}

QgsRectangle QgsRectangle::snappedToGrid( double spacing ) const
{
  if ( isNull() ) return *this;

  // helper function
  auto gridifyValue = []( double value, double spacing ) -> double
  {
    if ( spacing > 0 )
      return  std::round( value / spacing ) * spacing;
    else
      return value;
  };

  return QgsRectangle(
           gridifyValue( mXmin, spacing ),
           gridifyValue( mYmin, spacing ),
           gridifyValue( mXmax, spacing ),
           gridifyValue( mYmax, spacing )
         );
}

QDataStream &operator<<( QDataStream &out, const QgsRectangle &rectangle )
{
  out << rectangle.xMinimum() << rectangle.yMinimum() << rectangle.xMaximum() << rectangle.yMaximum();
  return out;
}

QDataStream &operator>>( QDataStream &in, QgsRectangle &rectangle )
{
  double xmin, ymin, xmax, ymax;
  in >> xmin >> ymin >> xmax >> ymax;
  rectangle.setXMinimum( xmin );
  rectangle.setYMinimum( ymin );
  rectangle.setXMaximum( xmax );
  rectangle.setYMaximum( ymax );
  return in;
}
