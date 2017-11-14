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

#include <algorithm>
#include <cmath>
#include <limits>
#include <QRectF>
#include <QString>
#include <QTextStream>
#include <QTransform>
#include <QRegExp>

#include "qgsgeometry.h"
#include "qgspointxy.h"
#include "qgsrectangle.h"
#include "qgslogger.h"
#include "qgsbox3d.h"
#include "qgspolygon.h"
#include "qgslinestring.h"

QgsRectangle::QgsRectangle( double xMin, double yMin, double xMax, double yMax )
  : mXmin( xMin )
  , mYmin( yMin )
  , mXmax( xMax )
  , mYmax( yMax )
{
  normalize();
}

QgsRectangle::QgsRectangle( const QgsPointXY &p1, const QgsPointXY &p2 )
{
  set( p1, p2 );
}

QgsRectangle::QgsRectangle( QRectF const &qRectF )
{
  mXmin = qRectF.topLeft().x();
  mYmin = qRectF.topLeft().y();
  mXmax = qRectF.bottomRight().x();
  mYmax = qRectF.bottomRight().y();
}

QgsRectangle::QgsRectangle( const QgsRectangle &r )
{
  mXmin = r.xMinimum();
  mYmin = r.yMinimum();
  mXmax = r.xMaximum();
  mYmax = r.yMaximum();
}

QgsRectangle QgsRectangle::fromWkt( const QString &wkt )
{
  QgsGeometry geom = QgsGeometry::fromWkt( wkt );
  if ( geom.isMultipart() )
    return QgsRectangle();

  QgsPolygonXY poly = geom.asPolygon();

  if ( poly.size() != 1 )
    return QgsRectangle();

  QgsPolylineXY polyline = geom.asPolygon().at( 0 );
  if ( polyline.size() == 5 && polyline.at( 0 ) == polyline.at( 4 ) && geom.isGeosValid() )
    return QgsRectangle( polyline.at( 0 ).x(), polyline.at( 0 ).y(), polyline.at( 2 ).x(), polyline.at( 2 ).y() );
  else
    return QgsRectangle();
}

QgsRectangle QgsRectangle::fromCenterAndSize( QgsPointXY center, double width, double height )
{
  double xMin = center.x() - width / 2.0;
  double xMax = xMin + width;
  double yMin = center.y() - height / 2.0;
  double yMax = yMin + height;
  return QgsRectangle( xMin, yMin, xMax, yMax );
}

void QgsRectangle::set( const QgsPointXY &p1, const QgsPointXY &p2 )
{
  mXmin = p1.x();
  mXmax = p2.x();
  mYmin = p1.y();
  mYmax = p2.y();
  normalize();
}

void QgsRectangle::set( double xmin_, double ymin_, double xmax_, double ymax_ )
{
  mXmin = xmin_;
  mYmin = ymin_;
  mXmax = xmax_;
  mYmax = ymax_;
  normalize();
}

void QgsRectangle::normalize()
{
  if ( isNull() )
    return;

  if ( mXmin > mXmax )
  {
    std::swap( mXmin, mXmax );
  }
  if ( mYmin > mYmax )
  {
    std::swap( mYmin, mYmax );
  }
}

void QgsRectangle::setMinimal()
{
  mXmin = std::numeric_limits<double>::max();
  mYmin = std::numeric_limits<double>::max();
  mXmax = -std::numeric_limits<double>::max();
  mYmax = -std::numeric_limits<double>::max();
}

void QgsRectangle::scale( double scaleFactor, const QgsPointXY *cp )
{
  // scale from the center
  double centerX, centerY;
  if ( cp )
  {
    centerX = cp->x();
    centerY = cp->y();
  }
  else
  {
    centerX = mXmin + width() / 2;
    centerY = mYmin + height() / 2;
  }
  scale( scaleFactor, centerX, centerY );
}

void QgsRectangle::scale( double scaleFactor, double centerX, double centerY )
{
  double newWidth = width() * scaleFactor;
  double newHeight = height() * scaleFactor;
  mXmin = centerX - newWidth / 2.0;
  mXmax = centerX + newWidth / 2.0;
  mYmin = centerY - newHeight / 2.0;
  mYmax = centerY + newHeight / 2.0;
}

void QgsRectangle::grow( double delta )
{
  mXmin -= delta;
  mXmax += delta;
  mYmin -= delta;
  mYmax += delta;
}

void QgsRectangle::include( const QgsPointXY &p )
{
  if ( p.x() < xMinimum() )
    setXMinimum( p.x() );
  else if ( p.x() > xMaximum() )
    setXMaximum( p.x() );
  if ( p.y() < yMinimum() )
    setYMinimum( p.y() );
  if ( p.y() > yMaximum() )
    setYMaximum( p.y() );
}

QgsRectangle QgsRectangle::buffered( double width ) const
{
  return QgsRectangle( mXmin - width, mYmin - width, mXmax + width, mYmax + width );
}

QgsRectangle QgsRectangle::intersect( const QgsRectangle *rect ) const
{
  QgsRectangle intersection = QgsRectangle();
  if ( rect && intersects( *rect ) )
  {
    intersection.setXMinimum( mXmin > rect->xMinimum() ? mXmin : rect->xMinimum() );
    intersection.setXMaximum( mXmax < rect->xMaximum() ? mXmax : rect->xMaximum() );
    intersection.setYMinimum( mYmin > rect->yMinimum() ? mYmin : rect->yMinimum() );
    intersection.setYMaximum( mYmax < rect->yMaximum() ? mYmax : rect->yMaximum() );
  }
  return intersection;
}

bool QgsRectangle::intersects( const QgsRectangle &rect ) const
{
  double x1 = ( mXmin > rect.mXmin ? mXmin : rect.mXmin );
  double x2 = ( mXmax < rect.mXmax ? mXmax : rect.mXmax );
  if ( x1 > x2 )
    return false;
  double y1 = ( mYmin > rect.mYmin ? mYmin : rect.mYmin );
  double y2 = ( mYmax < rect.mYmax ? mYmax : rect.mYmax );
  return y1 <= y2;
}

bool QgsRectangle::contains( const QgsRectangle &rect ) const
{
  return ( rect.mXmin >= mXmin && rect.mXmax <= mXmax && rect.mYmin >= mYmin && rect.mYmax <= mYmax );
}

bool QgsRectangle::contains( const QgsPointXY &p ) const
{
  return mXmin <= p.x() && p.x() <= mXmax &&
         mYmin <= p.y() && p.y() <= mYmax;
}

void QgsRectangle::combineExtentWith( const QgsRectangle &rect )
{
  if ( isNull() )
    *this = rect;
  else
  {
    mXmin = ( ( mXmin < rect.xMinimum() ) ? mXmin : rect.xMinimum() );
    mXmax = ( ( mXmax > rect.xMaximum() ) ? mXmax : rect.xMaximum() );

    mYmin = ( ( mYmin < rect.yMinimum() ) ? mYmin : rect.yMinimum() );
    mYmax = ( ( mYmax > rect.yMaximum() ) ? mYmax : rect.yMaximum() );
  }

}

void QgsRectangle::combineExtentWith( double x, double y )
{
  if ( isNull() )
    *this = QgsRectangle( x, y, x, y );
  else
  {
    mXmin = ( ( mXmin < x ) ? mXmin : x );
    mXmax = ( ( mXmax > x ) ? mXmax : x );

    mYmin = ( ( mYmin < y ) ? mYmin : y );
    mYmax = ( ( mYmax > y ) ? mYmax : y );
  }
}

QgsRectangle QgsRectangle::operator-( const QgsVector v ) const
{
  double xmin = mXmin - v.x();
  double xmax = mXmax - v.x();
  double ymin = mYmin - v.y();
  double ymax = mYmax - v.y();
  return QgsRectangle( xmin, ymin, xmax, ymax );
}

QgsRectangle QgsRectangle::operator+( const QgsVector v ) const
{
  double xmin = mXmin + v.x();
  double xmax = mXmax + v.x();
  double ymin = mYmin + v.y();
  double ymax = mYmax + v.y();
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

bool QgsRectangle::isEmpty() const
{
  return mXmax < mXmin || mYmax < mYmin || qgsDoubleNear( mXmax, mXmin ) || qgsDoubleNear( mYmax, mYmin );
}

bool QgsRectangle::isNull() const
{
  // rectangle created QgsRectangle() or with rect.setMinimal() ?
  return ( qgsDoubleNear( mXmin, 0.0 ) && qgsDoubleNear( mXmax, 0.0 ) && qgsDoubleNear( mYmin, 0.0 ) && qgsDoubleNear( mYmax, 0.0 ) ) ||
         ( qgsDoubleNear( mXmin, std::numeric_limits<double>::max() ) && qgsDoubleNear( mYmin, std::numeric_limits<double>::max() ) &&
           qgsDoubleNear( mXmax, -std::numeric_limits<double>::max() ) && qgsDoubleNear( mYmax, -std::numeric_limits<double>::max() ) );
}

QString QgsRectangle::asWktCoordinates() const
{
  QString rep =
    qgsDoubleToString( mXmin ) + ' ' + qgsDoubleToString( mYmin ) + ", " +
    qgsDoubleToString( mXmax ) + ' ' + qgsDoubleToString( mYmax );

  return rep;
}

QString QgsRectangle::asWktPolygon() const
{
  QString rep =
    QStringLiteral( "POLYGON((" ) +
    qgsDoubleToString( mXmin ) + ' ' + qgsDoubleToString( mYmin ) + ", " +
    qgsDoubleToString( mXmax ) + ' ' + qgsDoubleToString( mYmin ) + ", " +
    qgsDoubleToString( mXmax ) + ' ' + qgsDoubleToString( mYmax ) + ", " +
    qgsDoubleToString( mXmin ) + ' ' + qgsDoubleToString( mYmax ) + ", " +
    qgsDoubleToString( mXmin ) + ' ' + qgsDoubleToString( mYmin ) +
    QStringLiteral( "))" );

  return rep;
}

QRectF QgsRectangle::toRectF() const
{
  return QRectF( static_cast< qreal >( mXmin ), static_cast< qreal >( mYmin ), static_cast< qreal >( mXmax - mXmin ), static_cast< qreal >( mYmax - mYmin ) );
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

  if ( isEmpty() )
    rep = QStringLiteral( "Empty" );
  else
    rep = QStringLiteral( "%1,%2 : %3,%4" )
          .arg( mXmin, 0, 'f', precision )
          .arg( mYmin, 0, 'f', precision )
          .arg( mXmax, 0, 'f', precision )
          .arg( mYmax, 0, 'f', precision );

  QgsDebugMsgLevel( QString( "Extents : %1" ).arg( rep ), 4 );

  return rep;
}

QString QgsRectangle::asPolygon() const
{
//   QString rep = tmp.sprintf("%16f %16f,%16f %16f,%16f %16f,%16f %16f,%16f %16f",
//     xmin, ymin, xmin, ymax, xmax, ymax, xmax, ymin, xmin, ymin);
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

bool QgsRectangle::operator==( const QgsRectangle &r1 ) const
{
  return qgsDoubleNear( r1.xMaximum(), xMaximum() ) &&
         qgsDoubleNear( r1.xMinimum(), xMinimum() ) &&
         qgsDoubleNear( r1.yMaximum(), yMaximum() ) &&
         qgsDoubleNear( r1.yMinimum(), yMinimum() );
}

bool QgsRectangle::operator!=( const QgsRectangle &r1 ) const
{
  return ( ! operator==( r1 ) );
}

QgsRectangle &QgsRectangle::operator=( const QgsRectangle &r )
{
  if ( &r != this )
  {
    mXmax = r.xMaximum();
    mXmin = r.xMinimum();
    mYmax = r.yMaximum();
    mYmin = r.yMinimum();
  }

  return *this;
}

bool QgsRectangle::isFinite() const
{
  if ( std::isinf( mXmin ) || std::isinf( mYmin ) || std::isinf( mXmax ) || std::isinf( mYmax ) )
  {
    return false;
  }
  if ( std::isnan( mXmin ) || std::isnan( mYmin ) || std::isnan( mXmax ) || std::isnan( mYmax ) )
  {
    return false;
  }
  return true;
}

void QgsRectangle::invert()
{
  std::swap( mXmin, mYmin );
  std::swap( mXmax, mYmax );
}

QgsBox3d QgsRectangle::toBox3d( double zMin, double zMax ) const
{
  return QgsBox3d( mXmin, mYmin, zMin, mXmax, mYmax, zMax );
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
