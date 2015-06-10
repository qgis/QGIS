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
#endif // GDAL_VERSION_NUM >= 1900

/***************************************************************************/

#if defined(GDAL_VERSION_NUM) && defined(GDAL_COMPUTE_VERSION)
#if GDAL_VERSION_NUM >= GDAL_COMPUTE_VERSION(1,11,0)

QgsOgrMapToPixelSimplifier::QgsOgrMapToPixelSimplifier( int simplifyFlags, double tolerance )
    : QgsMapToPixelSimplifier( simplifyFlags, tolerance )
    , mPointBufferPtr( NULL )
    , mPointBufferCount( 0 )
{
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
QgsPoint* QgsOgrMapToPixelSimplifier::mallocPoints( int numPoints )
{
  if ( mPointBufferPtr && mPointBufferCount < numPoints )
  {
    OGRFree( mPointBufferPtr );
    mPointBufferPtr = NULL;
  }
  if ( !mPointBufferPtr )
  {
    mPointBufferCount = numPoints;
    mPointBufferPtr = ( QgsPoint* )OGRMalloc( mPointBufferCount * sizeof( QgsPoint ) );
  }
  return mPointBufferPtr;
}

//! Returns a point buffer of the specified envelope
QgsPoint* QgsOgrMapToPixelSimplifier::getEnvelopePoints( const QgsRectangle& envelope, int& numPoints, bool isaLinearRing )
{
  QgsPoint* points = NULL;

  double x1 = envelope.xMinimum();
  double y1 = envelope.yMinimum();
  double x2 = envelope.xMaximum();
  double y2 = envelope.yMaximum();

  if ( isaLinearRing )
  {
    numPoints = 5;
    points = mallocPoints( numPoints );
    points[0].set( x1, y1 );
    points[1].set( x2, y1 );
    points[2].set( x2, y2 );
    points[3].set( x1, y2 );
    points[4].set( x1, y1 );
  }
  else
  {
    numPoints = 2;
    points = mallocPoints( numPoints );
    points[0].set( x1, y1 );
    points[1].set( x2, y2 );
  }
  return points;
}

//////////////////////////////////////////////////////////////////////////////////////////////
// Helper simplification methods

//! Simplifies the OGR-geometry (Removing duplicated points) when is applied the specified map2pixel context
bool QgsOgrMapToPixelSimplifier::simplifyOgrGeometry( QGis::GeometryType geometryType, double* xptr, int xStride, double* yptr, int yStride, int pointCount, int& pointSimplifiedCount )
{
  bool isGeneralizable = ( mSimplifyFlags & QgsMapToPixelSimplifier::SimplifyGeometry );

  pointSimplifiedCount = pointCount;
  if ( geometryType == QGis::Point || geometryType == QGis::UnknownGeometry )
    return false;

  pointSimplifiedCount = 0;

  double map2pixelTol = mTolerance * mTolerance; //-> Use mappixelTol for 'LengthSquare' calculations.
  double x, y, lastX = 0, lastY = 0;

  char* xsourcePtr = ( char* )xptr;
  char* ysourcePtr = ( char* )yptr;
  char* xtargetPtr = ( char* )xptr;
  char* ytargetPtr = ( char* )yptr;

  for ( int i = 0, numPoints = geometryType == QGis::Polygon ? pointCount - 1 : pointCount; i < numPoints; ++i )
  {
    memcpy( &x, xsourcePtr, sizeof( double ) ); xsourcePtr += xStride;
    memcpy( &y, ysourcePtr, sizeof( double ) ); ysourcePtr += yStride;

    if ( i == 0 ||
         !isGeneralizable ||
         calculateLengthSquared2D( x, y, lastX, lastY ) > map2pixelTol ||
         ( geometryType == QGis::Line && ( i == 1 || i >= numPoints - 2 ) ) )
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
    if (( mSimplifyFlags & QgsMapToPixelSimplifier::SimplifyEnvelope ) && isGeneralizableByMapBoundingBox( envelope ) )
    {
      QgsPoint* points = getEnvelopePoints( envelope, numPoints, isaLinearRing );

      setGeometryPoints( geometry, points, numPoints );
      OGR_G_FlattenTo2D( geometry );

      return true;
    }
    else if ( mSimplifyFlags & QgsMapToPixelSimplifier::SimplifyGeometry )
    {
      QGis::GeometryType geometryType = isaLinearRing ? QGis::Polygon : QGis::Line;
      int numSimplifiedPoints = 0;

      QgsPoint* points = mallocPoints( numPoints );
      double* xptr = ( double* )points;
      double* yptr = xptr + 1;
      OGR_G_GetPoints( geometry, xptr, 16, yptr, 16, NULL, 0 );

      if ( simplifyOgrGeometry( geometryType, xptr, 16, yptr, 16, numPoints, numSimplifiedPoints ) )
      {
        if (( isaLinearRing && numSimplifiedPoints <= 4 ) || ( !isaLinearRing && numSimplifiedPoints <= 1 ) )
          points = getEnvelopePoints( envelope, numSimplifiedPoints, isaLinearRing );

        setGeometryPoints( geometry, points, numSimplifiedPoints );
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

    for ( int i = 0, numGeometries = OGR_G_GetGeometryCount( geometry ); i < numGeometries; ++i )
    {
      result |= simplifyOgrGeometry( OGR_G_GetGeometryRef( geometry, i ), wkbGeometryType == wkbMultiPolygon );
    }

    if ( result )
      OGR_G_FlattenTo2D( geometry );

    return result;
  }

  return false;
}

//! Load a point array to the specified LineString geometry
void QgsOgrMapToPixelSimplifier::setGeometryPoints( OGRGeometryH geometry, QgsPoint* points, int numPoints )
{
  double* xptr = ( double* )points;
  double* yptr = xptr + 1;

  OGR_G_SetPoints( geometry, numPoints, xptr, 16, yptr, 16, NULL, 0 );
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

#endif // GDAL_VERSION_NUM >= GDAL_COMPUTE_VERSION(1,11,0)
#endif // defined(GDAL_VERSION_NUM) && defined(GDAL_COMPUTE_VERSION)
