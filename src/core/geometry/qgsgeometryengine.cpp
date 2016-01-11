/***************************************************************************
  qgsgeometryengine.cpp

 ---------------------
 begin                : 11.1.2016
 copyright            : (C) 2016 by mku
 email                : matthias@opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsgeometryengine.h"

QgsGeometryEngine::QgsGeometryEngine( const QgsAbstractGeometryV2* geometry , int precision )
    : mGeometry( geometry )
    , mPrecision( precision )
    , mGeosEngine( nullptr )
{

}

QgsGeometryEngine::~QgsGeometryEngine()
{
  delete mGeosEngine;
}

void QgsGeometryEngine::geometryChanged()
{
  geosEngine()->geometryChanged();
}

void QgsGeometryEngine::prepareGeometry()
{
  geosEngine()->prepareGeometry();
}

QgsAbstractGeometryV2* QgsGeometryEngine::intersection( const QgsAbstractGeometryV2& geom, QString* errorMsg ) const
{
  return geosEngine()->intersection( geom, errorMsg );
}

QgsAbstractGeometryV2* QgsGeometryEngine::difference( const QgsAbstractGeometryV2& geom, QString* errorMsg ) const
{
  return geosEngine()->intersection( geom, errorMsg );
}

QgsAbstractGeometryV2* QgsGeometryEngine::combine( const QgsAbstractGeometryV2& geom, QString* errorMsg ) const
{
  return geosEngine()->combine( geom, errorMsg );
}

QgsAbstractGeometryV2* QgsGeometryEngine::combine( const QList<QgsAbstractGeometryV2*>& geomList, QString* errorMsg ) const
{
  return geosEngine()->combine( geomList, errorMsg );
}

QgsAbstractGeometryV2* QgsGeometryEngine::symDifference( const QgsAbstractGeometryV2& geom, QString* errorMsg ) const
{
  return geosEngine()->symDifference( geom, errorMsg );
}

QgsAbstractGeometryV2* QgsGeometryEngine::buffer( double distance, int segments, QString* errorMsg ) const
{
  return geosEngine()->buffer( distance, segments, errorMsg );
}

QgsAbstractGeometryV2* QgsGeometryEngine::buffer( double distance, int segments, int endCapStyle, int joinStyle, double mitreLimit, QString* errorMsg ) const
{
  return geosEngine()->buffer( distance, segments, endCapStyle, joinStyle, mitreLimit, errorMsg );
}

QgsAbstractGeometryV2* QgsGeometryEngine::simplify( double tolerance, QString* errorMsg ) const
{
  return geosEngine()->simplify( tolerance, errorMsg );
}

QgsAbstractGeometryV2* QgsGeometryEngine::interpolate( double distance, QString* errorMsg ) const
{
  return geosEngine()->interpolate( distance, errorMsg );
}

QgsAbstractGeometryV2* QgsGeometryEngine::envelope( QString* errorMsg ) const
{
  return geosEngine()->envelope( errorMsg );
}

bool QgsGeometryEngine::centroid( QgsPointV2& pt, QString* errorMsg ) const
{
  return geosEngine()->centroid( pt, errorMsg );
}

bool QgsGeometryEngine::pointOnSurface( QgsPointV2& pt, QString* errorMsg ) const
{
  return geosEngine()->pointOnSurface( pt, errorMsg );
}

QgsAbstractGeometryV2* QgsGeometryEngine::convexHull( QString* errorMsg ) const
{
  return geosEngine()->convexHull( errorMsg );
}

double QgsGeometryEngine::distance( const QgsAbstractGeometryV2& geom, QString* errorMsg ) const
{
  return geosEngine()->distance( geom, errorMsg );
}

bool QgsGeometryEngine::intersects( const QgsAbstractGeometryV2& geom, QString* errorMsg ) const
{
  return geosEngine()->intersects( geom, errorMsg );
}

bool QgsGeometryEngine::touches( const QgsAbstractGeometryV2& geom, QString* errorMsg ) const
{
  return geosEngine()->touches( geom, errorMsg );
}

bool QgsGeometryEngine::crosses( const QgsAbstractGeometryV2& geom, QString* errorMsg ) const
{
  return geosEngine()->crosses( geom, errorMsg );
}

bool QgsGeometryEngine::within( const QgsAbstractGeometryV2& geom, QString* errorMsg ) const
{
  return geosEngine()->within( geom, errorMsg );
}

bool QgsGeometryEngine::overlaps( const QgsAbstractGeometryV2& geom, QString* errorMsg ) const
{
  return geosEngine()->overlaps( geom, errorMsg );
}

bool QgsGeometryEngine::contains( const QgsAbstractGeometryV2& geom, QString* errorMsg ) const
{
  return geosEngine()->contains( geom, errorMsg );
}

bool QgsGeometryEngine::disjoint( const QgsAbstractGeometryV2& geom, QString* errorMsg ) const
{
  return geosEngine()->disjoint( geom, errorMsg );
}

QString QgsGeometryEngine::relate( const QgsAbstractGeometryV2& geom, QString* errorMsg ) const
{
  return geosEngine()->relate( geom, errorMsg );
}

bool QgsGeometryEngine::relatePattern( const QgsAbstractGeometryV2& geom, const QString& pattern, QString* errorMsg ) const
{
  return geosEngine()->relatePattern( geom, pattern, errorMsg );
}

double QgsGeometryEngine::area( QString* errorMsg ) const
{
  return geosEngine()->area( errorMsg );
}

double QgsGeometryEngine::length( QString* errorMsg ) const
{
  return geosEngine()->length( errorMsg ) ;
}

bool QgsGeometryEngine::isValid( QString* errorMsg ) const
{
  return geosEngine()->isValid( errorMsg );
}

bool QgsGeometryEngine::isEqual( const QgsAbstractGeometryV2& geom, QString* errorMsg ) const
{
  return geosEngine()->isEqual( geom, errorMsg );
}

bool QgsGeometryEngine::isEmpty( QString* errorMsg ) const
{
  return geosEngine()->isEmpty( errorMsg );
}

int QgsGeometryEngine::splitGeometry( const QgsLineStringV2& splitLine, QList<QgsAbstractGeometryV2*>& newGeometries, bool topological, QList<QgsPointV2>& topologyTestPoints, QString* errorMsg ) const
{
  Q_UNUSED( splitLine );
  Q_UNUSED( newGeometries );
  Q_UNUSED( topological );
  Q_UNUSED( topologyTestPoints );
  Q_UNUSED( errorMsg );
  return 2;
}

QgsAbstractGeometryV2* QgsGeometryEngine::offsetCurve( double distance, int segments, int joinStyle, double mitreLimit, QString* errorMsg ) const
{
  return geosEngine()->offsetCurve( distance, segments, joinStyle, mitreLimit, errorMsg );
}

QgsGeos* QgsGeometryEngine::geosEngine() const
{
  if ( !mGeosEngine )
    mGeosEngine = new QgsGeos( mGeometry, mPrecision );

  return mGeosEngine;
}
