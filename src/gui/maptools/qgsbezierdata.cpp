/***************************************************************************
    qgsbezierdata.cpp  -  Data structure for Poly-Bézier curve digitizing
    ---------------------
    begin                : December 2025
    copyright            : (C) 2025 by Loïc Bartoletti
    email                : loic dot bartoletti at oslandia dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsbezierdata.h"

#include <cmath>

#include "qgsnurbscurve.h"

///@cond PRIVATE

void QgsBezierData::addAnchor( const QgsPoint &pt )
{
  mAnchors.append( pt );
  // Add two handles at the anchor position (retracted)
  mHandles.append( pt ); // left handle
  mHandles.append( pt ); // right handle
}

void QgsBezierData::moveAnchor( int idx, const QgsPoint &pt )
{
  if ( idx < 0 || idx >= mAnchors.count() )
    return;

  // Calculate offset
  const double dx = pt.x() - mAnchors[idx].x();
  const double dy = pt.y() - mAnchors[idx].y();
  const double dz = pt.is3D() ? ( pt.z() - mAnchors[idx].z() ) : 0.0;

  // Move anchor
  mAnchors[idx] = pt;

  // Move both handles relatively
  const int leftHandleIdx = idx * 2;
  const int rightHandleIdx = idx * 2 + 1;

  if ( leftHandleIdx < mHandles.count() )
  {
    QgsPoint &lh = mHandles[leftHandleIdx];
    lh.setX( lh.x() + dx );
    lh.setY( lh.y() + dy );
    if ( pt.is3D() )
      lh.setZ( lh.z() + dz );
  }

  if ( rightHandleIdx < mHandles.count() )
  {
    QgsPoint &rh = mHandles[rightHandleIdx];
    rh.setX( rh.x() + dx );
    rh.setY( rh.y() + dy );
    if ( pt.is3D() )
      rh.setZ( rh.z() + dz );
  }
}

void QgsBezierData::moveHandle( int idx, const QgsPoint &pt )
{
  if ( idx < 0 || idx >= mHandles.count() )
    return;

  mHandles[idx] = pt;
}

void QgsBezierData::insertAnchor( int segmentIdx, const QgsPoint &pt )
{
  if ( segmentIdx < 0 || segmentIdx > mAnchors.count() )
    return;

  mAnchors.insert( segmentIdx, pt );

  // Insert two handles at the position
  const int handleInsertIdx = segmentIdx * 2;
  mHandles.insert( handleInsertIdx, pt );     // left handle
  mHandles.insert( handleInsertIdx + 1, pt ); // right handle
}

void QgsBezierData::deleteAnchor( int idx )
{
  if ( idx < 0 || idx >= mAnchors.count() )
    return;

  mAnchors.removeAt( idx );

  // Remove both handles
  const int handleIdx = idx * 2;
  if ( handleIdx + 1 < mHandles.count() )
  {
    mHandles.removeAt( handleIdx + 1 ); // remove right handle first
    mHandles.removeAt( handleIdx );     // then left handle
  }
}

void QgsBezierData::retractHandle( int idx )
{
  if ( idx < 0 || idx >= mHandles.count() )
    return;

  const int anchorIdx = idx / 2;
  if ( anchorIdx < mAnchors.count() )
  {
    mHandles[idx] = mAnchors[anchorIdx];
  }
}

void QgsBezierData::extendHandle( int idx, const QgsPoint &pt )
{
  if ( idx < 0 || idx >= mHandles.count() )
    return;

  mHandles[idx] = pt;
}

QgsPoint QgsBezierData::anchor( int idx ) const
{
  if ( idx < 0 || idx >= mAnchors.count() )
    return QgsPoint();
  return mAnchors[idx];
}

QgsPoint QgsBezierData::handle( int idx ) const
{
  if ( idx < 0 || idx >= mHandles.count() )
    return QgsPoint();
  return mHandles[idx];
}

QgsPoint QgsBezierData::evaluateBezier( const QgsPoint &p0, const QgsPoint &p1, const QgsPoint &p2, const QgsPoint &p3, double t )
{
  // Cubic Bézier formula: B(t) = (1-t)³P₀ + 3(1-t)²tP₁ + 3(1-t)t²P₂ + t³P₃
  const double t1 = 1.0 - t;
  const double t1_2 = t1 * t1;
  const double t1_3 = t1_2 * t1;
  const double t_2 = t * t;
  const double t_3 = t_2 * t;

  const double x = t1_3 * p0.x() + 3.0 * t1_2 * t * p1.x() + 3.0 * t1 * t_2 * p2.x() + t_3 * p3.x();
  const double y = t1_3 * p0.y() + 3.0 * t1_2 * t * p1.y() + 3.0 * t1 * t_2 * p2.y() + t_3 * p3.y();

  QgsPoint result( x, y );

  // Handle Z if present
  if ( p0.is3D() )
  {
    const double z = t1_3 * p0.z() + 3.0 * t1_2 * t * p1.z() + 3.0 * t1 * t_2 * p2.z() + t_3 * p3.z();
    result.addZValue( z );
  }

  return result;
}

QgsPointSequence QgsBezierData::interpolate() const
{
  QgsPointSequence result;

  if ( mAnchors.count() < 2 )
  {
    // Not enough anchors for a curve, just return anchors
    for ( const QgsPoint &anchor : mAnchors )
      result.append( anchor );
    return result;
  }

  // Add first anchor
  result.append( mAnchors.first() );

  // For each segment between consecutive anchors
  for ( int i = 0; i < mAnchors.count() - 1; ++i )
  {
    const QgsPoint &p0 = mAnchors[i];
    const QgsPoint &p1 = mHandles[i * 2 + 1];     // right handle of anchor i
    const QgsPoint &p2 = mHandles[( i + 1 ) * 2]; // left handle of anchor i+1
    const QgsPoint &p3 = mAnchors[i + 1];

    // Interpolate the segment
    for ( int j = 1; j <= INTERPOLATION_POINTS; ++j )
    {
      const double t = static_cast<double>( j ) / INTERPOLATION_POINTS;
      result.append( evaluateBezier( p0, p1, p2, p3, t ) );
    }
  }

  return result;
}

QgsNurbsCurve *QgsBezierData::asNurbsCurve() const
{
  const int n = mAnchors.count();
  if ( n < 2 )
    return nullptr;

  // Build control points: anchor, handle_right, handle_left, anchor, ...
  // A piecewise cubic Bézier with n anchors has n-1 segments
  // Total control points: 1 + 3*(n-1) = 3n-2
  QVector<QgsPoint> ctrlPts;
  ctrlPts.append( mAnchors[0] );

  for ( int i = 0; i < n - 1; ++i )
  {
    ctrlPts.append( mHandles[i * 2 + 1] );     // right handle of anchor i
    ctrlPts.append( mHandles[( i + 1 ) * 2] ); // left handle of anchor i+1
    ctrlPts.append( mAnchors[i + 1] );
  }

  // Build knot vector with multiplicity 3 at junctions for C0 continuity
  // Format: [0,0,0,0, 1,1,1, 2,2,2, ..., n-1,n-1,n-1,n-1]
  // Total knots: 4 + 3*(n-2) + 4 = 3n + 2
  // Actually for n-1 segments with degree 3: ctrlPts.count() + 4 = 3n-2+4 = 3n+2
  QVector<double> knots;

  // First 4 knots are 0
  for ( int i = 0; i < 4; ++i )
    knots.append( 0.0 );

  // Interior knots with multiplicity 3
  for ( int i = 1; i < n - 1; ++i )
  {
    for ( int j = 0; j < 3; ++j )
      knots.append( static_cast<double>( i ) );
  }

  // Last 4 knots are n-1
  for ( int i = 0; i < 4; ++i )
    knots.append( static_cast<double>( n - 1 ) );

  // Uniform weights (non-rational B-spline)
  QVector<double> weights( ctrlPts.count(), 1.0 );

  return new QgsNurbsCurve( ctrlPts, 3, knots, weights );
}

void QgsBezierData::clear()
{
  mAnchors.clear();
  mHandles.clear();
}

int QgsBezierData::findClosestAnchor( const QgsPoint &pt, double tolerance ) const
{
  int closestIdx = -1;
  double minDistSq = tolerance * tolerance;

  for ( int i = 0; i < mAnchors.count(); ++i )
  {
    const double dx = mAnchors[i].x() - pt.x();
    const double dy = mAnchors[i].y() - pt.y();
    const double distSq = dx * dx + dy * dy;
    if ( distSq < minDistSq )
    {
      minDistSq = distSq;
      closestIdx = i;
    }
  }

  return closestIdx;
}

int QgsBezierData::findClosestHandle( const QgsPoint &pt, double tolerance ) const
{
  int closestIdx = -1;
  double minDistSq = tolerance * tolerance;

  for ( int i = 0; i < mHandles.count(); ++i )
  {
    // Skip handles that are at anchor position (retracted)
    const int anchorIdx = i / 2;
    if ( anchorIdx < mAnchors.count() )
    {
      const QgsPoint &anchor = mAnchors[anchorIdx];
      if ( qFuzzyCompare( mHandles[i].x(), anchor.x() ) && qFuzzyCompare( mHandles[i].y(), anchor.y() ) )
        continue;
    }

    const double dx = mHandles[i].x() - pt.x();
    const double dy = mHandles[i].y() - pt.y();
    const double distSq = dx * dx + dy * dy;
    if ( distSq < minDistSq )
    {
      minDistSq = distSq;
      closestIdx = i;
    }
  }

  return closestIdx;
}

int QgsBezierData::findClosestSegment( const QgsPoint &pt, double tolerance ) const
{
  if ( mAnchors.count() < 2 )
    return -1;

  int closestSegment = -1;
  double minDist = tolerance;

  // Check each segment
  for ( int i = 0; i < mAnchors.count() - 1; ++i )
  {
    const QgsPoint &p0 = mAnchors[i];
    const QgsPoint &p1 = mHandles[i * 2 + 1];
    const QgsPoint &p2 = mHandles[( i + 1 ) * 2];
    const QgsPoint &p3 = mAnchors[i + 1];

    // Sample the curve and find minimum distance
    for ( int j = 0; j <= INTERPOLATION_POINTS; ++j )
    {
      const double t = static_cast<double>( j ) / INTERPOLATION_POINTS;
      const QgsPoint curvePoint = evaluateBezier( p0, p1, p2, p3, t );

      const double dx = curvePoint.x() - pt.x();
      const double dy = curvePoint.y() - pt.y();
      const double dist = std::sqrt( dx * dx + dy * dy );

      if ( dist < minDist )
      {
        minDist = dist;
        closestSegment = i;
      }
    }
  }

  return closestSegment;
}

///@endcond PRIVATE
