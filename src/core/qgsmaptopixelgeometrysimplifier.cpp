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

QgsMapToPixelSimplifier::QgsMapToPixelSimplifier( int simplifyFlags, double tolerance )
    : mSimplifyFlags( simplifyFlags )
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
  float vx = ( float )( x2 - x1 );
  float vy = ( float )( y2 - y1 );

  return vx*vx + vy*vy;
}

//! Returns the BBOX of the specified WKB-point stream
inline static QgsRectangle calculateBoundingBox( QGis::WkbType wkbType, unsigned char* wkb, size_t numPoints )
{
  double x, y;
  QgsRectangle r;
  r.setMinimal();

  int sizeOfDoubleX = sizeof( double );
  int sizeOfDoubleY = QGis::wkbDimensions( wkbType ) == 3 /*hasZValue*/ ? 2 * sizeof( double ) : sizeof( double );

  for ( size_t i = 0; i < numPoints; ++i )
  {
    memcpy( &x, wkb, sizeof( double ) ); wkb += sizeOfDoubleX;
    memcpy( &y, wkb, sizeof( double ) ); wkb += sizeOfDoubleY;
    r.combineExtentWith( x, y );
  }

  return r;
}

//! Generalize the WKB-geometry using the BBOX of the original geometry
static bool generalizeWkbGeometryByBoundingBox(
  QGis::WkbType wkbType,
  unsigned char* sourceWkb, size_t sourceWkbSize,
  unsigned char* targetWkb, size_t& targetWkbSize,
  const QgsRectangle& envelope, bool writeHeader )
{
  Q_UNUSED( sourceWkb );
  unsigned char* wkb2 = targetWkb;
  unsigned int geometryType = QGis::singleType( QGis::flatType( wkbType ) );

  int sizeOfDoubleX = sizeof( double );
  int sizeOfDoubleY = QGis::wkbDimensions( wkbType ) == 3 /*hasZValue*/ ? 2 * sizeof( double ) : sizeof( double );

  // If the geometry is already minimal skip the generalization
  size_t minimumSize = geometryType == QGis::WKBLineString ? 4 + 2 * ( sizeOfDoubleX + sizeOfDoubleY ) : 8 + 5 * ( sizeOfDoubleX + sizeOfDoubleY );

  if ( writeHeader )
    minimumSize += 5;

  if ( sourceWkbSize <= minimumSize )
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
    char byteOrder = QgsApplication::endian(); // byteOrder
    memcpy( targetWkb, &byteOrder, 1 );
    targetWkb += 1;

    memcpy( targetWkb, &geometryType, 4 ); // type
    targetWkb += 4;

    if ( geometryType == QGis::WKBPolygon ) // numRings
    {
      int numRings = 1;
      memcpy( targetWkb, &numRings, 4 );
      targetWkb += 4;
    }
  }

  // Write the generalized geometry
  if ( geometryType == QGis::WKBLineString )
  {
    int numPoints = 2;
    memcpy( targetWkb, &numPoints, 4 ); // numPoints;
    targetWkb += 4;

    memcpy( targetWkb, &x1, sizeof( double ) ); targetWkb += sizeof( double );
    memcpy( targetWkb, &y1, sizeof( double ) ); targetWkb += sizeof( double );

    memcpy( targetWkb, &x2, sizeof( double ) ); targetWkb += sizeof( double );
    memcpy( targetWkb, &y2, sizeof( double ) ); targetWkb += sizeof( double );
  }
  else
  {
    int numPoints = 5;
    memcpy( targetWkb, &numPoints, 4 ); // numPoints;
    targetWkb += 4;

    memcpy( targetWkb, &x1, sizeof( double ) ); targetWkb += sizeof( double );
    memcpy( targetWkb, &y1, sizeof( double ) ); targetWkb += sizeof( double );

    memcpy( targetWkb, &x2, sizeof( double ) ); targetWkb += sizeof( double );
    memcpy( targetWkb, &y1, sizeof( double ) ); targetWkb += sizeof( double );

    memcpy( targetWkb, &x2, sizeof( double ) ); targetWkb += sizeof( double );
    memcpy( targetWkb, &y2, sizeof( double ) ); targetWkb += sizeof( double );

    memcpy( targetWkb, &x1, sizeof( double ) ); targetWkb += sizeof( double );
    memcpy( targetWkb, &y2, sizeof( double ) ); targetWkb += sizeof( double );

    memcpy( targetWkb, &x1, sizeof( double ) ); targetWkb += sizeof( double );
    memcpy( targetWkb, &y1, sizeof( double ) ); targetWkb += sizeof( double );
  }
  targetWkbSize += targetWkb - wkb2;

  return true;
}

//! Simplify the WKB-geometry using the specified tolerance
bool QgsMapToPixelSimplifier::simplifyWkbGeometry(
  int simplifyFlags, QGis::WkbType wkbType,
  unsigned char* sourceWkb, size_t sourceWkbSize,
  unsigned char* targetWkb, size_t& targetWkbSize,
  const QgsRectangle& envelope, double map2pixelTol,
  bool writeHeader, bool isaLinearRing )
{
  bool isGeneralizable = true;
  bool hasZValue = QGis::wkbDimensions( wkbType ) == 3;
  bool result = false;

  // Save initial WKB settings to use when the simplification creates invalid geometries
  unsigned char* sourcePrevWkb = sourceWkb;
  unsigned char* targetPrevWkb = targetWkb;
  size_t targetWkbPrevSize = targetWkbSize;

  // Can replace the geometry by its BBOX ?
  if (( simplifyFlags & QgsMapToPixelSimplifier::SimplifyEnvelope ) &&
      isGeneralizableByMapBoundingBox( envelope, map2pixelTol ) )
  {
    isGeneralizable = generalizeWkbGeometryByBoundingBox( wkbType, sourceWkb, sourceWkbSize, targetWkb, targetWkbSize, envelope, writeHeader );
    if ( isGeneralizable )
      return true;
  }

  if ( !( simplifyFlags & QgsMapToPixelSimplifier::SimplifyGeometry ) )
    isGeneralizable = false;

  // Write the main header of the geometry
  if ( writeHeader )
  {
    targetWkb[0] = sourceWkb[0]; // byteOrder
    sourceWkb += 1;
    targetWkb += 1;

    int geometryType;
    memcpy( &geometryType, sourceWkb, 4 );
    int flatType = QGis::flatType(( QGis::WkbType )geometryType );
    memcpy( targetWkb, &flatType, 4 ); // type
    sourceWkb += 4;
    targetWkb += 4;

    targetWkbSize += 5;
  }

  unsigned char* wkb1 = sourceWkb;
  unsigned char* wkb2 = targetWkb;
  unsigned int flatType = QGis::flatType( wkbType );

  // Write the geometry
  if ( flatType == QGis::WKBLineString || isaLinearRing )
  {
    double x, y, lastX = 0, lastY = 0;
    QgsRectangle r;
    r.setMinimal();

    int sizeOfDoubleX = sizeof( double );
    int sizeOfDoubleY = QGis::wkbDimensions( wkbType ) == 3 /*hasZValue*/ ? 2 * sizeof( double ) : sizeof( double );

    int numPoints;
    memcpy( &numPoints, sourceWkb, 4 );
    sourceWkb += 4;
    if ( numPoints <= ( isaLinearRing ? 5 : 2 ) )
      isGeneralizable = false;

    int numTargetPoints = 0;
    memcpy( targetWkb, &numTargetPoints, 4 );
    targetWkb += 4;
    targetWkbSize += 4;

    double* ptr = ( double* )targetWkb;
    map2pixelTol *= map2pixelTol; //-> Use mappixelTol for 'LengthSquare' calculations.

    bool isLongSegment;
    bool hasLongSegments = false; //-> To avoid replace the simplified geometry by its BBOX when there are 'long' segments.

    // Check whether the LinearRing is really closed.
    if ( isaLinearRing )
    {
      double x1, y1, x2, y2;

      unsigned char* startWkbX = sourceWkb;
      unsigned char* startWkbY = startWkbX + sizeOfDoubleX;
      unsigned char* finalWkbX = sourceWkb + ( numPoints - 1 ) * ( sizeOfDoubleX + sizeOfDoubleY );
      unsigned char* finalWkbY = finalWkbX + sizeOfDoubleX;

      memcpy( &x1, startWkbX, sizeof( double ) );
      memcpy( &y1, startWkbY, sizeof( double ) );
      memcpy( &x2, finalWkbX, sizeof( double ) );
      memcpy( &y2, finalWkbY, sizeof( double ) );

      isaLinearRing = ( x1 == x2 ) && ( y1 == y2 );
    }

    // Process each vertex...
    for ( int i = 0; i < numPoints; ++i )
    {
      memcpy( &x, sourceWkb, sizeof( double ) ); sourceWkb += sizeOfDoubleX;
      memcpy( &y, sourceWkb, sizeof( double ) ); sourceWkb += sizeOfDoubleY;

      isLongSegment = false;

      if ( i == 0 ||
           !isGeneralizable ||
           ( isLongSegment = ( calculateLengthSquared2D( x, y, lastX, lastY ) > map2pixelTol ) ) ||
           ( !isaLinearRing && ( i == 1 || i >= numPoints - 2 ) ) )
      {
        memcpy( ptr, &x, sizeof( double ) ); lastX = x; ptr++;
        memcpy( ptr, &y, sizeof( double ) ); lastY = y; ptr++;
        numTargetPoints++;

        hasLongSegments |= isLongSegment;
      }

      r.combineExtentWith( x, y );
    }
    targetWkb = wkb2 + 4;

    if ( numTargetPoints < ( isaLinearRing ? 4 : 2 ) )
    {
      // we simplified the geometry too much!
      if ( !hasLongSegments )
      {
        // approximate the geometry's shape by its bounding box
        // (rect for linear ring / one segment for line string)
        unsigned char* targetTempWkb = targetWkb;
        int targetWkbTempSize = targetWkbSize;

        sourceWkb = sourcePrevWkb;
        targetWkb = targetPrevWkb;
        targetWkbSize = targetWkbPrevSize;
        if ( generalizeWkbGeometryByBoundingBox( wkbType, sourceWkb, sourceWkbSize, targetWkb, targetWkbSize, r, writeHeader ) )
          return true;

        targetWkb = targetTempWkb;
        targetWkbSize = targetWkbTempSize;
      }
      else
      {
        // Bad luck! The simplified geometry is invalid and approximation by bounding box
        // would create artifacts due to long segments. Worst of all, we may have overwritten
        // the original coordinates by the simplified ones (source and target WKB ptr can be the same)
        // so we cannot even undo the changes here. We will return invalid geometry and hope that
        // other pieces of QGIS will survive that :-/
      }
    }
    if ( isaLinearRing )
    {
      // make sure we keep the linear ring closed
      memcpy( &x, targetWkb + 0, sizeof( double ) );
      memcpy( &y, targetWkb + sizeof( double ), sizeof( double ) );
      if ( lastX != x || lastY != y )
      {
        memcpy( ptr, &x, sizeof( double ) ); ptr++;
        memcpy( ptr, &y, sizeof( double ) ); ptr++;
        numTargetPoints++;
      }
    }
    targetWkbSize += numTargetPoints * sizeof( double ) * 2;
    targetWkb = wkb2;

    memcpy( targetWkb, &numTargetPoints, 4 );
    result = numPoints != numTargetPoints;
  }
  else if ( flatType == QGis::WKBPolygon )
  {
    int numRings;
    memcpy( &numRings, sourceWkb, 4 );
    sourceWkb += 4;

    memcpy( targetWkb, &numRings, 4 );
    targetWkb += 4;
    targetWkbSize += 4;

    for ( int i = 0; i < numRings; ++i )
    {
      int numPoints_i;
      memcpy( &numPoints_i, sourceWkb, 4 );
      QgsRectangle envelope_i = numRings == 1 ? envelope : calculateBoundingBox( wkbType, sourceWkb + 4, numPoints_i );

      size_t sourceWkbSize_i = 4 + numPoints_i * ( hasZValue ? 3 : 2 ) * sizeof( double );
      size_t targetWkbSize_i = 0;

      result |= simplifyWkbGeometry( simplifyFlags, wkbType, sourceWkb, sourceWkbSize_i, targetWkb, targetWkbSize_i, envelope_i, map2pixelTol, false, true );
      sourceWkb += sourceWkbSize_i;
      targetWkb += targetWkbSize_i;

      targetWkbSize += targetWkbSize_i;
    }
  }
  else if ( flatType == QGis::WKBMultiLineString || flatType == QGis::WKBMultiPolygon )
  {
    int numGeoms;
    memcpy( &numGeoms, sourceWkb, 4 );
    sourceWkb += 4;
    wkb1 += 4;

    memcpy( targetWkb, &numGeoms, 4 );
    targetWkb += 4;
    targetWkbSize += 4;

    for ( int i = 0; i < numGeoms; ++i )
    {
      size_t sourceWkbSize_i = 0;
      size_t targetWkbSize_i = 0;

      // ... calculate the wkb-size of the current child complex geometry
      if ( flatType == QGis::WKBMultiLineString )
      {
        int numPoints_i;
        memcpy( &numPoints_i, wkb1 + 5, 4 );
        int wkbSize_i = 4 + numPoints_i * ( hasZValue ? 3 : 2 ) * sizeof( double );

        sourceWkbSize_i += 5 + wkbSize_i;
        wkb1 += 5 + wkbSize_i;
      }
      else
      {
        int numPrings_i;
        memcpy( &numPrings_i, wkb1 + 5, 4 );
        sourceWkbSize_i = 9;
        wkb1 += 9;

        for ( int j = 0; j < numPrings_i; ++j )
        {
          int numPoints_i;
          memcpy( &numPoints_i, wkb1, 4 );
          int wkbSize_i = 4 + numPoints_i * ( hasZValue ? 3 : 2 ) * sizeof( double );

          sourceWkbSize_i += wkbSize_i;
          wkb1 += wkbSize_i;
        }
      }
      result |= simplifyWkbGeometry( simplifyFlags, QGis::singleType( wkbType ), sourceWkb, sourceWkbSize_i, targetWkb, targetWkbSize_i, envelope, map2pixelTol, true, false );
      sourceWkb += sourceWkbSize_i;
      targetWkb += targetWkbSize_i;

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

  size_t wkbSize = geometry->wkbSize();
  unsigned char* wkb = ( unsigned char* )malloc( wkbSize );
  memcpy( wkb, geometry->asWkb(), wkbSize );
  g->fromWkb( wkb, wkbSize );
  simplifyGeometry( g, mSimplifyFlags, mTolerance );

  return g;
}

//! Simplifies the geometry (Removing duplicated points) when is applied the specified map2pixel context
bool QgsMapToPixelSimplifier::simplifyGeometry( QgsGeometry* geometry, int simplifyFlags, double tolerance )
{
  size_t targetWkbSize = 0;

  // Check whether the geometry can be simplified using the map2pixel context
  QGis::GeometryType geometryType = geometry->type();
  if ( !( geometryType == QGis::Line || geometryType == QGis::Polygon ) )
    return false;

  QgsRectangle envelope = geometry->boundingBox();
  QGis::WkbType wkbType = geometry->wkbType();

  unsigned char* wkb = ( unsigned char* )geometry->asWkb();
  size_t wkbSize = geometry->wkbSize();

  // Simplify the geometry rewriting temporally its WKB-stream for saving calloc's.
  if ( simplifyWkbGeometry( simplifyFlags, wkbType, wkb, wkbSize, wkb, targetWkbSize, envelope, tolerance ) )
  {
    unsigned char* targetWkb = new unsigned char[targetWkbSize];
    memcpy( targetWkb, wkb, targetWkbSize );
    geometry->fromWkb( targetWkb, targetWkbSize );
    return true;
  }
  return false;
}

//! Simplifies the geometry (Removing duplicated points) when is applied the specified map2pixel context
bool QgsMapToPixelSimplifier::simplifyGeometry( QgsGeometry* geometry ) const
{
  return simplifyGeometry( geometry, mSimplifyFlags, mTolerance );
}
