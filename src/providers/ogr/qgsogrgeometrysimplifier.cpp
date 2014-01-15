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

QgsOgrTopologyPreservingSimplifier::QgsOgrTopologyPreservingSimplifier( double tolerance ) : QgsTopologyPreservingSimplifier( tolerance )
{
}

QgsOgrTopologyPreservingSimplifier::~QgsOgrTopologyPreservingSimplifier()
{
}

//! Simplifies the specified geometry
bool QgsOgrTopologyPreservingSimplifier::simplifyGeometry( OGRGeometry* geometry )
{
  OGRwkbGeometryType wkbGeometryType = QgsOgrProvider::ogrWkbSingleFlatten( geometry->getGeometryType() );

  if ( wkbGeometryType == wkbLineString || wkbGeometryType == wkbPolygon )
  {
    OGRGeometry* g = geometry->SimplifyPreserveTopology( mTolerance );

    if ( g )
    {
      size_t wkbSize = g->WkbSize();
      unsigned char * wkb = (unsigned char *)OGRMalloc( wkbSize );
      g->exportToWkb( ( OGRwkbByteOrder ) QgsApplication::endian(), wkb );
      geometry->importFromWkb( wkb, wkbSize );
      delete g;

      return true;
    }
  }
  return false;
}

/***************************************************************************/

QgsOgrMapToPixelSimplifier::QgsOgrMapToPixelSimplifier( int simplifyFlags, double map2pixelTol ) : QgsMapToPixelSimplifier( simplifyFlags, map2pixelTol )
{
  mPointBufferCount = 64;
  mPointBufferPtr = ( OGRRawPoint* )OGRMalloc( mPointBufferCount * sizeof( OGRRawPoint ) );
}

QgsOgrMapToPixelSimplifier::~QgsOgrMapToPixelSimplifier()
{
  if ( mPointBufferPtr )
  {
    OGRFree( mPointBufferPtr );
    mPointBufferPtr = NULL;
  }
}

//! Returns a point buffer of the specified size
OGRRawPoint* QgsOgrMapToPixelSimplifier::mallocPoints( int numPoints )
{
  if ( mPointBufferPtr && mPointBufferCount < numPoints )
  {
    OGRFree( mPointBufferPtr );
    mPointBufferPtr = NULL;
  }
  if ( mPointBufferPtr == NULL )
  {
    mPointBufferCount = numPoints;
    mPointBufferPtr = ( OGRRawPoint* )OGRMalloc( mPointBufferCount * sizeof( OGRRawPoint ) );
  }
  return mPointBufferPtr;
}

//////////////////////////////////////////////////////////////////////////////////////////////
// Helper simplification methods

//! Simplifies the OGR-geometry (Removing duplicated points) when is applied the specified map2pixel context
bool QgsOgrMapToPixelSimplifier::simplifyOgrGeometry( QGis::GeometryType geometryType, const QgsRectangle& envelope, double* xptr, int xStride, double* yptr, int yStride, int pointCount, int& pointSimplifiedCount )
{
  Q_UNUSED( envelope )
  bool canbeGeneralizable = ( mSimplifyFlags & QgsMapToPixelSimplifier::SimplifyGeometry );

  pointSimplifiedCount = pointCount;
  if ( geometryType == QGis::Point || geometryType == QGis::UnknownGeometry ) return false;
  pointSimplifiedCount = 0;

  double map2pixelTol = mMapToPixelTol * mMapToPixelTol; //-> Use mappixelTol for 'LengthSquare' calculations.
  double x, y, lastX = 0, lastY = 0;

  char* xsourcePtr = ( char* )xptr;
  char* ysourcePtr = ( char* )yptr;
  char* xtargetPtr = ( char* )xptr;
  char* ytargetPtr = ( char* )yptr;

  for ( int i = 0, numPoints = geometryType == QGis::Polygon ? pointCount - 1 : pointCount; i < numPoints; ++i )
  {
    memcpy( &x, xsourcePtr, sizeof( double ) ); xsourcePtr += xStride;
    memcpy( &y, ysourcePtr, sizeof( double ) ); ysourcePtr += yStride;

    if ( i == 0 || !canbeGeneralizable || QgsMapToPixelSimplifier::calculateLengthSquared2D( x, y, lastX, lastY ) > map2pixelTol || ( geometryType == QGis::Line && ( i == 1 || i >= numPoints - 2 ) ) )
    {
      memcpy( xtargetPtr, &x, sizeof( double ) ); lastX = x; xtargetPtr += xStride;
      memcpy( ytargetPtr, &y, sizeof( double ) ); lastY = y; ytargetPtr += yStride;
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
bool QgsOgrMapToPixelSimplifier::simplifyOgrGeometry( OGRGeometry* geometry, bool isaLinearRing )
{
  OGRwkbGeometryType wkbGeometryType = wkbFlatten( geometry->getGeometryType() );

  // Simplify the geometry rewriting temporally its WKB-stream for saving calloc's.
  if ( wkbGeometryType == wkbLineString )
  {
    OGRLineString* lineString = ( OGRLineString* )geometry;

    int numPoints = lineString->getNumPoints();
    if (( isaLinearRing && numPoints <= 5 ) || ( !isaLinearRing && numPoints <= 4 ) ) return false;

    OGREnvelope env;
    geometry->getEnvelope( &env );
    QgsRectangle envelope( env.MinX, env.MinY, env.MaxX, env.MaxY );

    // Can replace the geometry by its BBOX ?
    if (( mSimplifyFlags & QgsMapToPixelSimplifier::SimplifyEnvelope ) && canbeGeneralizedByMapBoundingBox( envelope ) )
    {
      OGRRawPoint* points = NULL;
      int numPoints = 0;

      double x1 = envelope.xMinimum();
      double y1 = envelope.yMinimum();
      double x2 = envelope.xMaximum();
      double y2 = envelope.yMaximum();

      if ( isaLinearRing )
      {
        numPoints = 5;
        points = mallocPoints( numPoints );
        points[0].x = x1; points[0].y = y1;
        points[1].x = x2; points[1].y = y1;
        points[2].x = x2; points[2].y = y2;
        points[3].x = x1; points[3].y = y2;
        points[4].x = x1; points[4].y = y1;
      }
      else
      {
        numPoints = 2;
        points = mallocPoints( numPoints );
        points[0].x = x1; points[0].y = y1;
        points[1].x = x2; points[1].y = y2;
      }
      lineString->setPoints( numPoints, points );
      lineString->flattenTo2D();
      return true;
    }
    else
      if ( mSimplifyFlags & QgsMapToPixelSimplifier::SimplifyGeometry )
      {
        QGis::GeometryType geometryType = isaLinearRing ? QGis::Polygon : QGis::Line;
        int numSimplifiedPoints = 0;

        OGRRawPoint* points = mallocPoints( numPoints );
        double* xptr = ( double* )points;
        double* yptr = xptr + 1;
        lineString->getPoints( points );

        if ( simplifyOgrGeometry( geometryType, envelope, xptr, 16, yptr, 16, numPoints, numSimplifiedPoints ) )
        {
          lineString->setPoints( numSimplifiedPoints, points );
          lineString->flattenTo2D();
        }
        return numSimplifiedPoints != numPoints;
      }
  }
  else
    if ( wkbGeometryType == wkbPolygon )
    {
      OGRPolygon* polygon = ( OGRPolygon* )geometry;
      bool result = simplifyOgrGeometry( polygon->getExteriorRing(), true );

      for ( int i = 0, numInteriorRings = polygon->getNumInteriorRings(); i < numInteriorRings; ++i )
      {
        result |= simplifyOgrGeometry( polygon->getInteriorRing( i ), true );
      }
      if ( result ) polygon->flattenTo2D();
      return result;
    }
    else
      if ( wkbGeometryType == wkbMultiLineString || wkbGeometryType == wkbMultiPolygon )
      {
        OGRGeometryCollection* collection = ( OGRGeometryCollection* )geometry;
        bool result = false;

        for ( int i = 0, numGeometries = collection->getNumGeometries(); i < numGeometries; ++i )
        {
          result |= simplifyOgrGeometry( collection->getGeometryRef( i ), wkbGeometryType == wkbMultiPolygon );
        }
        if ( result ) collection->flattenTo2D();
        return result;
      }
  return false;
}

//////////////////////////////////////////////////////////////////////////////////////////////

//! Simplifies the specified geometry
bool QgsOgrMapToPixelSimplifier::simplifyGeometry( OGRGeometry* geometry )
{
  OGRwkbGeometryType wkbGeometryType = QgsOgrProvider::ogrWkbSingleFlatten( geometry->getGeometryType() );

  if ( wkbGeometryType == wkbLineString || wkbGeometryType == wkbPolygon )
  {
    return simplifyOgrGeometry( geometry, wkbGeometryType == wkbPolygon );
  }
  return false;
}
