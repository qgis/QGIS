/***************************************************************************
    qgscrosssection.cpp
    ---------------------
    begin                : January 2026
    copyright            : (C) 2026 by Dominik Cindrić
    email                : viper dot miniq at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgscrosssection.h"

#include "qgscoordinatetransform.h"
#include "qgslinestring.h"
#include "qgspolygon.h"

QgsCrossSection::QgsCrossSection( const QgsPoint &p1, const QgsPoint &p2, double halfWidth )
  : mStartPoint( p1 )
  , mEndPoint( p2 )
  , mHalfWidth( halfWidth )
{
}

QgsGeometry QgsCrossSection::asGeometry( const QgsCoordinateTransform *ct ) const
{
  QgsVector vec( mEndPoint - mStartPoint );
  vec = vec.normalized().perpVector();

  QgsGeometry geom;
  if ( mHalfWidth == 0.0 )
  {
    QVector<QgsPointXY> points = { mStartPoint, mEndPoint };
    geom = QgsGeometry( new QgsLineString( points ) );
  }
  else
  {
    QVector<QgsPointXY> points = {
      mStartPoint + vec * mHalfWidth,
      mEndPoint + vec * mHalfWidth,
      mEndPoint - vec * mHalfWidth,
      mStartPoint - vec * mHalfWidth
    };
    geom = QgsGeometry( new QgsPolygon( new QgsLineString( points ) ) );
  }

  if ( ct )
  {
    try
    {
      geom.transform( *ct, Qgis::TransformDirection::Reverse );
    }
    catch ( const QgsCsException & )
    {
      geom = QgsGeometry();
    }
  }
  return geom;
}

void QgsCrossSection::nudgeLeft( double distance )
{
  nudge( distance );
}

void QgsCrossSection::nudgeRight( double distance )
{
  nudge( -distance );
}

void QgsCrossSection::nudge( double distance )
{
  QgsVector vec( mEndPoint - mStartPoint );
  vec = vec.normalized().perpVector();

  const QgsVector offset = vec * distance;
  mStartPoint += offset;
  mEndPoint += offset;
}

bool QgsCrossSection::isValid() const
{
  return ( mStartPoint != mEndPoint ) && ( mHalfWidth > 0.0 );
}
