/***************************************************************************
    qgsogrgeometrysimplifier.cpp
    ---------------------
    begin                : December 2013
    copyright            : (C) 2013 by Alvaro Huarte
    email                : http://wiki.osgeo.org/wiki/Alvaro_Huarte

 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsogrgeometrysimplifier.h"
#include "qgsogrprovider.h"
#include "qgsapplication.h"

QgsOgrAbstractGeometrySimplifier::~QgsOgrAbstractGeometrySimplifier()
{
}

/***************************************************************************/

#if defined(GDAL_VERSION_NUM) && GDAL_VERSION_NUM >= 1900
QgsOgrTopologyPreservingSimplifier::QgsOgrTopologyPreservingSimplifier( double tolerance )
    : QgsTopologyPreservingSimplifier( tolerance )
{
}

QgsOgrTopologyPreservingSimplifier::~QgsOgrTopologyPreservingSimplifier()
{
}

//! Simplifies the specified geometry
bool QgsOgrTopologyPreservingSimplifier::simplifyGeometry( OGRGeometryH geometry )
{
  OGRwkbGeometryType wkbGeometryType = QgsOgrProvider::ogrWkbSingleFlatten( OGR_G_GetGeometryType( geometry ) );

  if ( wkbGeometryType != wkbLineString && wkbGeometryType != wkbPolygon )
    return false;

  OGRGeometryH g = OGR_G_SimplifyPreserveTopology( geometry, mTolerance );
  if ( !g )
    return false;

  size_t wkbSize = OGR_G_WkbSize( g );
  unsigned char *wkb = new unsigned char[ wkbSize ];
  OGR_G_ExportToWkb( g, ( OGRwkbByteOrder ) QgsApplication::endian(), wkb );
  OGR_G_ImportFromWkb( geometry, wkb, wkbSize );
  delete [] wkb;
  OGR_G_DestroyGeometry( g );

  return true;
}
#endif

/***************************************************************************/

QgsOgrMapToPixelSimplifier::QgsOgrMapToPixelSimplifier( int simplifyFlags, double map2pixelTol )
    : QgsMapToPixelSimplifier( simplifyFlags, map2pixelTol )
{
}

QgsOgrMapToPixelSimplifier::~QgsOgrMapToPixelSimplifier()
{
}

//////////////////////////////////////////////////////////////////////////////////////////////
// Helper simplification methods

//! Simplifies the OGR-geometry (Removing duplicated points) when is applied the specified map2pixel context
bool QgsOgrMapToPixelSimplifier::simplifyOgrGeometry( QGis::GeometryType geometryType, const QgsRectangle& envelope, double *xptr, double *yptr, int pointCount, int& pointSimplifiedCount )
{
  Q_UNUSED( envelope )
  bool canbeGeneralizable = ( mSimplifyFlags & QgsMapToPixelSimplifier::SimplifyGeometry );

  pointSimplifiedCount = pointCount;
  if ( geometryType == QGis::Point || geometryType == QGis::UnknownGeometry ) return false;
  pointSimplifiedCount = 0;

  double map2pixelTol = mMapToPixelTol * mMapToPixelTol; //-> Use mappixelTol for 'LengthSquare' calculations.
  double x, y, lastX = 0, lastY = 0;

  double *xsourcePtr = xptr;
  double *ysourcePtr = yptr;
  double *xtargetPtr = xptr;
  double *ytargetPtr = yptr;

  for ( int i = 0, numPoints = geometryType == QGis::Polygon ? pointCount - 1 : pointCount; i < numPoints; ++i )
  {
    memcpy( &x, xsourcePtr++, sizeof( double ) );
    memcpy( &y, ysourcePtr++, sizeof( double ) );

    if ( i == 0 || !canbeGeneralizable || QgsMapToPixelSimplifier::calculateLengthSquared2D( x, y, lastX, lastY ) > map2pixelTol || ( geometryType == QGis::Line && ( i == 1 || i >= numPoints - 2 ) ) )
    {
      memcpy( xtargetPtr++, &x, sizeof( double ) ); lastX = x;
      memcpy( ytargetPtr++, &y, sizeof( double ) ); lastY = y;
      pointSimplifiedCount++;
    }
  }
  if ( geometryType == QGis::Polygon )
  {
    memcpy( xtargetPtr, xptr, sizeof( double ) );
    memcpy( ytargetPtr, yptr, sizeof( double ) );
    pointSimplifiedCount++;
  }

  return pointSimplifiedCount != pointCount;
}

//! Simplifies the OGR-geometry (Removing duplicated points) when is applied the specified map2pixel context
bool QgsOgrMapToPixelSimplifier::simplifyOgrGeometry( OGRGeometryH geometry, bool isaLinearRing )
{
  OGRwkbGeometryType wkbGeometryType = wkbFlatten( OGR_G_GetGeometryType( geometry ) );

  // Simplify the geometry rewriting temporally its WKB-stream for saving calloc's.
  if ( wkbGeometryType == wkbLineString )
  {
    int numPoints = OGR_G_GetPointCount( geometry );
    if (( isaLinearRing && numPoints <= 5 ) || ( !isaLinearRing && numPoints <= 4 ) )
      return false;

    OGREnvelope env;
    OGR_G_GetEnvelope( geometry, &env );
    QgsRectangle envelope( env.MinX, env.MinY, env.MaxX, env.MaxY );

    // Can replace the geometry by its BBOX ?
    if (( mSimplifyFlags & QgsMapToPixelSimplifier::SimplifyEnvelope ) && canbeGeneralizedByMapBoundingBox( envelope ) )
    {
      double x1 = envelope.xMinimum();
      double y1 = envelope.yMinimum();
      double x2 = envelope.xMaximum();
      double y2 = envelope.yMaximum();

      if ( isaLinearRing )
      {
        OGR_G_SetPoint( geometry, 0, x1, y1, 0.0 );
        OGR_G_SetPoint( geometry, 1, x2, y1, 0.0 );
        OGR_G_SetPoint( geometry, 2, x2, y2, 0.0 );
        OGR_G_SetPoint( geometry, 3, x1, y2, 0.0 );
        OGR_G_SetPoint( geometry, 4, x1, y1, 0.0 );
      }
      else
      {
        OGR_G_SetPoint( geometry, 0, x1, y1, 0.0 );
        OGR_G_SetPoint( geometry, 1, x2, y2, 0.0 );
      }

      OGR_G_FlattenTo2D( geometry );

      return true;
    }
    else if ( mSimplifyFlags & QgsMapToPixelSimplifier::SimplifyGeometry )
    {
      QGis::GeometryType geometryType = isaLinearRing ? QGis::Polygon : QGis::Line;
      int numSimplifiedPoints = 0;

      QVector<double> x( numPoints ), y( numPoints );
      for ( int i = 0; i < numPoints; i++ )
      {
        double z;
        OGR_G_GetPoint( geometry, i, &x[i], &y[i], &z );
      }

      if ( simplifyOgrGeometry( geometryType, envelope, x.data(), y.data(), numPoints, numSimplifiedPoints ) )
      {
        for ( int i = 0; i < numSimplifiedPoints; i++ )
        {
          OGR_G_SetPoint( geometry, i, x[i], y[i], 0.0 );
        }
        OGR_G_FlattenTo2D( geometry );
      }

      return numSimplifiedPoints != numPoints;
    }
  }
  else if ( wkbGeometryType == wkbPolygon )
  {
    bool result = simplifyOgrGeometry( OGR_G_GetGeometryRef( geometry, 0 ), true );

    for ( int i = 1, numInteriorRings = OGR_G_GetGeometryCount( geometry ); i < numInteriorRings; ++i )
    {
      result |= simplifyOgrGeometry( OGR_G_GetGeometryRef( geometry, i ), true );
    }

    if ( result )
      OGR_G_FlattenTo2D( geometry );

    return result;
  }
  else if ( wkbGeometryType == wkbMultiLineString || wkbGeometryType == wkbMultiPolygon )
  {
    bool result = false;

    for ( int i = 1, numGeometries = OGR_G_GetGeometryCount( geometry ); i < numGeometries; ++i )
    {
      result |= simplifyOgrGeometry( OGR_G_GetGeometryRef( geometry, i ), wkbGeometryType == wkbMultiPolygon );
    }
    if ( result )
      OGR_G_FlattenTo2D( geometry );
    return result;
  }

  return false;
}

//////////////////////////////////////////////////////////////////////////////////////////////

//! Simplifies the specified geometry
bool QgsOgrMapToPixelSimplifier::simplifyGeometry( OGRGeometryH geometry )
{
  OGRwkbGeometryType wkbGeometryType = QgsOgrProvider::ogrWkbSingleFlatten( OGR_G_GetGeometryType( geometry ) );

  if ( wkbGeometryType == wkbLineString || wkbGeometryType == wkbPolygon )
  {
    return simplifyOgrGeometry( geometry, wkbGeometryType == wkbPolygon );
  }

  return false;
}
