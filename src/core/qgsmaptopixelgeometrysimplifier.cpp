/***************************************************************************
    qgsmaptopixelgeometrysimplifier.cpp
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

#include <limits>

#include "qgsmaptopixelgeometrysimplifier.h"
#include "qgsapplication.h"
#include "qgslogger.h"


QgsMapToPixelSimplifier::QgsMapToPixelSimplifier( int simplifyFlags, double tolerance, SimplifyAlgorithm simplifyAlgorithm )
    : mSimplifyFlags( simplifyFlags )
    , mSimplifyAlgorithm( simplifyAlgorithm )
    , mTolerance( tolerance )
{
}

QgsMapToPixelSimplifier::~QgsMapToPixelSimplifier()
{
}

//////////////////////////////////////////////////////////////////////////////////////////////
// Helper simplification methods

//! Returns the squared 2D-distance of the vector defined by the two points specified
float QgsMapToPixelSimplifier::calculateLengthSquared2D( double x1, double y1, double x2, double y2 )
{
  float vx = static_cast< float >( x2 - x1 );
  float vy = static_cast< float >( y2 - y1 );

  return ( vx * vx ) + ( vy * vy );
}

//! Returns whether the points belong to the same grid
bool QgsMapToPixelSimplifier::equalSnapToGrid( double x1, double y1, double x2, double y2, double gridOriginX, double gridOriginY, float gridInverseSizeXY )
{
  int grid_x1 = qRound(( x1 - gridOriginX ) * gridInverseSizeXY );
  int grid_x2 = qRound(( x2 - gridOriginX ) * gridInverseSizeXY );
  if ( grid_x1 != grid_x2 ) return false;

  int grid_y1 = qRound(( y1 - gridOriginY ) * gridInverseSizeXY );
  int grid_y2 = qRound(( y2 - gridOriginY ) * gridInverseSizeXY );
  if ( grid_y1 != grid_y2 ) return false;

  return true;
}

//! Returns the BBOX of the specified WKB-point stream
inline static QgsRectangle calculateBoundingBox( QGis::WkbType wkbType, QgsConstWkbPtr wkbPtr, int numPoints )
{
  QgsRectangle r;
  r.setMinimal();

  int skipZM = ( QGis::wkbDimensions( wkbType ) - 2 ) * sizeof( double );
  Q_ASSERT( skipZM >= 0 );

  for ( int i = 0; i < numPoints; ++i )
  {
    double x, y;
    wkbPtr >> x >> y;
    wkbPtr += skipZM;
    r.combineExtentWith( x, y );
  }

  return r;
}

//////////////////////////////////////////////////////////////////////////////////////////////
// Helper simplification methods for Visvalingam method

// It uses a refactored code of the liblwgeom implementation:
// https://github.com/postgis/postgis/blob/svn-trunk/liblwgeom/effectivearea.h
// https://github.com/postgis/postgis/blob/svn-trunk/liblwgeom/effectivearea.c

#define LWDEBUG //
#define LWDEBUGF //
#define FP_MAX qMax
#define FLAGS_GET_Z( flags ) ( ( flags ) & 0x01 )
#define LW_MSG_MAXLEN 256
#define lwalloc qgsMalloc
#define lwfree qgsFree
#define lwerror qWarning

#include "simplify/effectivearea.h"
#include "simplify/effectivearea.c"

double* getPoint_internal( const POINTARRAY* inpts, int pointIndex )
{
  return inpts->pointlist + ( pointIndex * inpts->dimension );
}

//////////////////////////////////////////////////////////////////////////////////////////////

//! Generalize the WKB-geometry using the BBOX of the original geometry
static bool generalizeWkbGeometryByBoundingBox(
  QGis::WkbType wkbType,
  QgsConstWkbPtr sourceWkbPtr,
  QgsWkbPtr targetWkbPtr,
  int &targetWkbSize,
  const QgsRectangle &envelope, bool writeHeader )
{
  QgsWkbPtr savedTargetWkb( targetWkbPtr );
  unsigned int geometryType = QGis::singleType( QGis::flatType( wkbType ) );

  int skipZM = ( QGis::wkbDimensions( wkbType ) - 2 ) * sizeof( double );
  Q_ASSERT( skipZM >= 0 );

  // If the geometry is already minimal skip the generalization
  int minimumSize = geometryType == QGis::WKBLineString ? 4 + 2 * ( 2 * sizeof( double ) + skipZM ) : 8 + 5 * ( 2 * sizeof( double ) + skipZM );

  if ( writeHeader )
    minimumSize += 5;

  if ( sourceWkbPtr.remaining() <= minimumSize )
  {
    targetWkbSize = 0;
    return false;
  }

  double x1 = envelope.xMinimum();
  double y1 = envelope.yMinimum();
  double x2 = envelope.xMaximum();
  double y2 = envelope.yMaximum();

  // Write the main header of the geometry
  if ( writeHeader )
  {
    targetWkbPtr << ( char ) QgsApplication::endian() << geometryType;

    if ( geometryType == QGis::WKBPolygon ) // numRings
    {
      targetWkbPtr << 1;
    }
  }

  // Write the generalized geometry
  if ( geometryType == QGis::WKBLineString )
  {
    targetWkbPtr << 2 << x1 << y1 << x2 << y2;
  }
  else
  {
    targetWkbPtr << 5 << x1 << y1 << x2 << y1 << x2 << y2 << x1 << y2 << x1 << y1;
  }

  targetWkbSize += targetWkbPtr - savedTargetWkb;

  return true;
}

//! Simplify the WKB-geometry using the specified tolerance
bool QgsMapToPixelSimplifier::simplifyWkbGeometry(
  int simplifyFlags,
  SimplifyAlgorithm simplifyAlgorithm,
  QGis::WkbType wkbType,
  QgsConstWkbPtr sourceWkbPtr,
  QgsWkbPtr targetWkbPtr,
  int &targetWkbSize,
  const QgsRectangle &envelope, double map2pixelTol,
  bool writeHeader, bool isaLinearRing )
{
  bool isGeneralizable = true;
  bool result = false;

  // Save initial WKB settings to use when the simplification creates invalid geometries
  QgsConstWkbPtr sourcePrevWkbPtr( sourceWkbPtr );
  QgsWkbPtr targetPrevWkbPtr( targetWkbPtr );
  int targetWkbPrevSize = targetWkbSize;

  // Can replace the geometry by its BBOX ?
  if (( simplifyFlags & QgsMapToPixelSimplifier::SimplifyEnvelope ) &&
      isGeneralizableByMapBoundingBox( envelope, map2pixelTol ) )
  {
    isGeneralizable = generalizeWkbGeometryByBoundingBox( wkbType, sourceWkbPtr, targetWkbPtr, targetWkbSize, envelope, writeHeader );
    if ( isGeneralizable )
      return true;
  }

  if ( !( simplifyFlags & QgsMapToPixelSimplifier::SimplifyGeometry ) )
    isGeneralizable = false;

  // Write the main header of the geometry
  if ( writeHeader )
  {
    QgsWKBTypes::Type geometryType = sourceWkbPtr.readHeader();

    targetWkbPtr << ( char ) QgsApplication::endian() << QgsWKBTypes::flatType( geometryType );

    targetWkbSize += targetWkbPtr - targetPrevWkbPtr;
  }

  unsigned int flatType = QGis::flatType( wkbType );

  // Write the geometry
  if ( flatType == QGis::WKBLineString || isaLinearRing )
  {
    QgsWkbPtr savedTargetWkbPtr( targetWkbPtr );
    double x = 0.0, y = 0.0, lastX = 0, lastY = 0;
    QgsRectangle r;
    r.setMinimal();

    int skipZM = ( QGis::wkbDimensions( wkbType ) - 2 ) * sizeof( double );
    Q_ASSERT( skipZM >= 0 );

    int numPoints;
    sourceWkbPtr >> numPoints;

    if ( numPoints <= ( isaLinearRing ? 5 : 2 ) )
      isGeneralizable = false;

    QgsWkbPtr numPtr( targetWkbPtr );

    int numTargetPoints = 0;
    targetWkbPtr << numTargetPoints;
    targetWkbSize += 4;

    bool isLongSegment;
    bool hasLongSegments = false; //-> To avoid replace the simplified geometry by its BBOX when there are 'long' segments.
    bool badLuck = false;

    // Check whether the LinearRing is really closed.
    if ( isaLinearRing )
    {
      QgsConstWkbPtr checkPtr( sourceWkbPtr );

      double x1, y1, x2, y2;

      checkPtr >> x1 >> y1;
      checkPtr += skipZM + ( numPoints - 2 ) * ( 2 * sizeof( double ) + skipZM );
      checkPtr >> x2 >> y2;

      isaLinearRing = qgsDoubleNear( x1, x2 ) && qgsDoubleNear( y1, y2 );
    }

    // Process each vertex...
    if ( simplifyAlgorithm == SnapToGrid )
    {
      double gridOriginX = envelope.xMinimum();
      double gridOriginY = envelope.yMinimum();

      // Use a factor for the maximum displacement distance for simplification, similar as GeoServer does
      float gridInverseSizeXY = map2pixelTol != 0 ? ( float )( 1.0f / ( 0.8 * map2pixelTol ) ) : 0.0f;

      for ( int i = 0; i < numPoints; ++i )
      {
        sourceWkbPtr >> x >> y;
        sourceWkbPtr += skipZM;

        if ( i == 0 ||
             !isGeneralizable ||
             !equalSnapToGrid( x, y, lastX, lastY, gridOriginX, gridOriginY, gridInverseSizeXY ) ||
             ( !isaLinearRing && ( i == 1 || i >= numPoints - 2 ) ) )
        {
          targetWkbPtr << x << y;
          lastX = x;
          lastY = y;
          numTargetPoints++;
        }

        r.combineExtentWith( x, y );
      }
    }
    else if ( simplifyAlgorithm == Visvalingam )
    {
      map2pixelTol *= map2pixelTol; //-> Use mappixelTol for 'Area' calculations.

      POINTARRAY inpts;
      inpts.pointlist = ( double* )( const unsigned char* )sourceWkbPtr;
      inpts.dimension = QGis::wkbDimensions( wkbType );
      inpts.npoints = numPoints;
      inpts.flags = 0;

      EFFECTIVE_AREAS* ea;
      ea = initiate_effectivearea( &inpts );

      int set_area = 0;
      ptarray_calc_areas( ea, isaLinearRing ? 4 : 2, set_area, map2pixelTol );

      for ( int i = 0; i < numPoints; ++i )
      {
        if ( ea->res_arealist[ i ] > map2pixelTol )
        {
          double* coord = getPoint_internal( &inpts, i );
          x = coord[ 0 ];
          y = coord[ 1 ];

          targetWkbPtr << x << y;
          lastX = x;
          lastY = y;
          numTargetPoints++;
        }
      }
      destroy_effectivearea( ea );

      sourceWkbPtr += numPoints * ( inpts.dimension * sizeof( double ) );
    }
    else
    {
      map2pixelTol *= map2pixelTol; //-> Use mappixelTol for 'LengthSquare' calculations.

      for ( int i = 0; i < numPoints; ++i )
      {
        sourceWkbPtr >> x >> y;
        sourceWkbPtr += skipZM;

        isLongSegment = false;

        if ( i == 0 ||
             !isGeneralizable ||
             ( isLongSegment = ( calculateLengthSquared2D( x, y, lastX, lastY ) > map2pixelTol ) ) ||
             ( !isaLinearRing && ( i == 1 || i >= numPoints - 2 ) ) )
        {
          targetWkbPtr << x << y;
          lastX = x;
          lastY = y;
          numTargetPoints++;

          hasLongSegments |= isLongSegment;
        }

        r.combineExtentWith( x, y );
      }
    }

    QgsWkbPtr nextPointPtr( targetWkbPtr );

    targetWkbPtr = savedTargetWkbPtr;
    targetWkbPtr += sizeof( int );

    if ( numTargetPoints < ( isaLinearRing ? 4 : 2 ) )
    {
      // we simplified the geometry too much!
      if ( !hasLongSegments )
      {
        // approximate the geometry's shape by its bounding box
        // (rect for linear ring / one segment for line string)
        QgsWkbPtr tempWkbPtr( targetWkbPtr );
        int targetWkbTempSize = targetWkbSize;

        sourceWkbPtr = sourcePrevWkbPtr;
        targetWkbPtr = targetPrevWkbPtr;
        targetWkbSize = targetWkbPrevSize;
        if ( generalizeWkbGeometryByBoundingBox( wkbType, sourceWkbPtr, targetWkbPtr, targetWkbSize, r, writeHeader ) )
          return true;

        targetWkbPtr = tempWkbPtr;
        targetWkbSize = targetWkbTempSize;
      }
      else
      {
        // Bad luck! The simplified geometry is invalid and approximation by bounding box
        // would create artifacts due to long segments.
        // We will return invalid geometry and hope that other pieces of QGIS will survive that :-/
      }
      badLuck = true;
    }

    if ( isaLinearRing )
    {
      // make sure we keep the linear ring closed
      targetWkbPtr << x << y;
      if ( !qgsDoubleNear( lastX, x ) || !qgsDoubleNear( lastY, y ) )
      {
        nextPointPtr << x << y;
        numTargetPoints++;
      }
    }

    numPtr << numTargetPoints;
    targetWkbSize += numTargetPoints * sizeof( double ) * 2;

    result = !badLuck && numTargetPoints > 0;
  }
  else if ( flatType == QGis::WKBPolygon )
  {
    int numRings;
    sourceWkbPtr >> numRings;
    targetWkbPtr << numRings;
    targetWkbSize += 4;

    for ( int i = 0; i < numRings; ++i )
    {
      int numPoints_i;
      sourceWkbPtr >> numPoints_i;

      QgsRectangle envelope_i = numRings == 1 ? envelope : calculateBoundingBox( wkbType, sourceWkbPtr, numPoints_i );

      sourceWkbPtr -= sizeof( int );

      int sourceWkbSize_i = sizeof( int ) + numPoints_i * QGis::wkbDimensions( wkbType ) * sizeof( double );
      int targetWkbSize_i = 0;

      result |= simplifyWkbGeometry( simplifyFlags, simplifyAlgorithm, wkbType, sourceWkbPtr, targetWkbPtr, targetWkbSize_i, envelope_i, map2pixelTol, false, true );
      sourceWkbPtr += sourceWkbSize_i;
      targetWkbPtr += targetWkbSize_i;

      targetWkbSize += targetWkbSize_i;
    }
  }
  else if ( flatType == QGis::WKBMultiLineString || flatType == QGis::WKBMultiPolygon )
  {
    int numGeoms;
    sourceWkbPtr >> numGeoms;
    targetWkbPtr << numGeoms;
    targetWkbSize += 4;

    QgsConstWkbPtr sourceWkbPtr2( sourceWkbPtr );

    for ( int i = 0; i < numGeoms; ++i )
    {
      int sourceWkbSize_i = 0;
      int targetWkbSize_i = 0;

      sourceWkbPtr2.readHeader();

      // ... calculate the wkb-size of the current child complex geometry
      if ( flatType == QGis::WKBMultiLineString )
      {
        int numPoints_i;
        sourceWkbPtr2 >> numPoints_i;

        int wkbSize_i = numPoints_i * QGis::wkbDimensions( wkbType ) * sizeof( double );

        sourceWkbSize_i += 9 + wkbSize_i;
        sourceWkbPtr2 += wkbSize_i;
      }
      else
      {
        int numPrings_i;
        sourceWkbPtr2 >> numPrings_i;
        sourceWkbSize_i = 1 + 2 * sizeof( int );

        for ( int j = 0; j < numPrings_i; ++j )
        {
          int numPoints_i;
          sourceWkbPtr2 >> numPoints_i;

          int wkbSize_i = numPoints_i * QGis::wkbDimensions( wkbType ) * sizeof( double );

          sourceWkbSize_i += 4 + wkbSize_i;
          sourceWkbPtr2 += wkbSize_i;
        }
      }
      result |= simplifyWkbGeometry( simplifyFlags, simplifyAlgorithm, QGis::singleType( wkbType ), sourceWkbPtr, targetWkbPtr, targetWkbSize_i, envelope, map2pixelTol, true, false );
      sourceWkbPtr += sourceWkbSize_i;
      targetWkbPtr += targetWkbSize_i;

      targetWkbSize += targetWkbSize_i;
    }
  }

  return result;
}

//////////////////////////////////////////////////////////////////////////////////////////////

//! Returns whether the envelope can be replaced by its BBOX when is applied the specified map2pixel context
bool QgsMapToPixelSimplifier::isGeneralizableByMapBoundingBox( const QgsRectangle& envelope, double map2pixelTol )
{
  // Can replace the geometry by its BBOX ?
  return envelope.width() < map2pixelTol && envelope.height() < map2pixelTol;
}

//! Returns a simplified version the specified geometry (Removing duplicated points) when is applied the specified map2pixel context
QgsGeometry* QgsMapToPixelSimplifier::simplify( QgsGeometry* geometry ) const
{
  QgsGeometry* g = new QgsGeometry();

  int wkbSize = geometry->wkbSize();
  unsigned char* wkb = new unsigned char[ wkbSize ];
  memcpy( wkb, geometry->asWkb(), wkbSize );
  g->fromWkb( wkb, wkbSize );
  simplifyGeometry( g, mSimplifyFlags, mTolerance, mSimplifyAlgorithm );

  return g;
}

//! Simplifies the geometry (Removing duplicated points) when is applied the specified map2pixel context
bool QgsMapToPixelSimplifier::simplifyGeometry( QgsGeometry *geometry, int simplifyFlags, double tolerance, SimplifyAlgorithm simplifyAlgorithm )
{
  int finalWkbSize = 0;

  // Check whether the geometry can be simplified using the map2pixel context
  QGis::GeometryType geometryType = geometry->type();
  if ( !( geometryType == QGis::Line || geometryType == QGis::Polygon ) )
    return false;

  QgsRectangle envelope = geometry->boundingBox();
  QGis::WkbType wkbType = geometry->wkbType();

  QgsConstWkbPtr wkbPtr( geometry->asWkb(), geometry->wkbSize() );

  unsigned char* targetWkb = new unsigned char[wkbPtr.remaining()];
  memcpy( targetWkb, wkbPtr, wkbPtr.remaining() );
  QgsWkbPtr targetWkbPtr( targetWkb, wkbPtr.remaining() );

  try
  {
    if ( simplifyWkbGeometry( simplifyFlags, simplifyAlgorithm, wkbType, wkbPtr, targetWkbPtr, finalWkbSize, envelope, tolerance ) )
    {
      unsigned char *finalWkb = new unsigned char[finalWkbSize];
      memcpy( finalWkb, targetWkb, finalWkbSize );
      geometry->fromWkb( finalWkb, finalWkbSize );
      delete [] targetWkb;
      return true;
    }
  }
  catch ( const QgsWkbException &e )
  {
    Q_UNUSED( e );
    QgsDebugMsg( QString( "Exception thrown by simplifier: %1" ) .arg( e.what() ) );
  }
  delete [] targetWkb;
  return false;
}

//! Simplifies the geometry (Removing duplicated points) when is applied the specified map2pixel context
bool QgsMapToPixelSimplifier::simplifyGeometry( QgsGeometry* geometry ) const
{
  return simplifyGeometry( geometry, mSimplifyFlags, mTolerance, mSimplifyAlgorithm );
}

//! Simplifies the WKB-point array (Removing duplicated points) when is applied the specified map2pixel context
bool QgsMapToPixelSimplifier::simplifyPoints( QgsWKBTypes::Type wkbType, QgsConstWkbPtr& sourceWkbPtr, QPolygonF& targetPoints, int simplifyFlags, double tolerance, SimplifyAlgorithm simplifyAlgorithm )
{
  QgsWKBTypes::Type singleType = QgsWKBTypes::singleType( wkbType );
  QgsWKBTypes::Type flatType = QgsWKBTypes::flatType( singleType );

  // Check whether the geometry can be simplified using the map2pixel context
  if ( flatType == QgsWKBTypes::Point )
    return false;

  bool isaLinearRing = flatType == QgsWKBTypes::Polygon;
  int numPoints;
  sourceWkbPtr >> numPoints;

  // No simplify simple geometries
  if ( numPoints <= ( isaLinearRing ? 6 : 3 ) )
    return false;

  QgsRectangle envelope = calculateBoundingBox( QGis::fromNewWkbType( singleType ), QgsConstWkbPtr( sourceWkbPtr ), numPoints );
  sourceWkbPtr -= sizeof( int );

  int targetWkbSize = 5 + sizeof( int ) + numPoints * ( 2 * sizeof( double ) );
  unsigned char* targetWkb = new unsigned char[ targetWkbSize ];

  //! Simplify the WKB-geometry using the specified tolerance
  try
  {
    QgsWkbPtr targetWkbPtr( targetWkb, targetWkbSize );
    targetWkbPtr << ( char ) QgsApplication::endian() << flatType;
    targetWkbSize = 5;

    if ( simplifyWkbGeometry( simplifyFlags, simplifyAlgorithm, QGis::fromNewWkbType( singleType ), sourceWkbPtr, targetWkbPtr, targetWkbSize, envelope, tolerance, false, isaLinearRing ) )
    {
      QgsConstWkbPtr finalWkbPtr( targetWkb, targetWkbSize );
      finalWkbPtr.readHeader();
      finalWkbPtr >> targetPoints;

      int skipZM = ( QGis::wkbDimensions( QGis::fromNewWkbType( wkbType ) ) - 2 ) * sizeof( double );
      sourceWkbPtr += sizeof( int ) + numPoints * ( 2 * sizeof( double ) + skipZM );

      delete [] targetWkb;
      return true;
    }
  }
  catch ( const QgsWkbException &e )
  {
    QgsDebugMsg( QString( "Exception thrown by simplifier: %1" ) .arg( e.what() ) );
  }
  delete [] targetWkb;
  return false;
}

//! Simplifies the specified WKB-point array
bool QgsMapToPixelSimplifier::simplifyPoints( QgsWKBTypes::Type wkbType, QgsConstWkbPtr& sourceWkbPtr, QPolygonF& targetPoints ) const
{
  return simplifyPoints( wkbType, sourceWkbPtr, targetPoints, mSimplifyFlags, mTolerance, mSimplifyAlgorithm );
}
