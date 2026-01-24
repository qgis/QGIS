/***************************************************************************
    qgsbezierdata.cpp  -  Data structure for Poly-Bézier curve digitizing
    ---------------------
    begin                : December 2025
    copyright            : (C) 2025 by Loïc Bartoletti
                           Adapted from BezierEditing plugin work by Takayuki Mizutani
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

#include "qgsgeometryutils.h"
#include "qgsnurbscurve.h"

///@cond PRIVATE

const QgsAnchorWithHandles QgsBezierData::sInvalidAnchor;

void QgsBezierData::addAnchor( const QgsPoint &pt )
{
  mData.append( QgsAnchorWithHandles( pt ) );
}

void QgsBezierData::moveAnchor( int idx, const QgsPoint &pt )
{
  if ( idx < 0 || idx >= mData.count() )
    return;

  QgsAnchorWithHandles &data = mData[idx];

  // Calculate offset
  const double dx = pt.x() - data.anchor.x();
  const double dy = pt.y() - data.anchor.y();
  const double dz = pt.is3D() ? ( pt.z() - data.anchor.z() ) : 0.0;

  // Move anchor
  data.anchor = pt;

  // Move both handles relatively
  data.leftHandle.setX( data.leftHandle.x() + dx );
  data.leftHandle.setY( data.leftHandle.y() + dy );
  if ( pt.is3D() )
    data.leftHandle.setZ( data.leftHandle.z() + dz );

  data.rightHandle.setX( data.rightHandle.x() + dx );
  data.rightHandle.setY( data.rightHandle.y() + dy );
  if ( pt.is3D() )
    data.rightHandle.setZ( data.rightHandle.z() + dz );
}

void QgsBezierData::moveHandle( int idx, const QgsPoint &pt )
{
  const int anchorIdx = idx / 2;
  if ( anchorIdx < 0 || anchorIdx >= mData.count() )
    return;

  if ( idx % 2 == 0 )
    mData[anchorIdx].leftHandle = pt;
  else
    mData[anchorIdx].rightHandle = pt;
}

void QgsBezierData::insertAnchor( int segmentIdx, const QgsPoint &pt )
{
  if ( segmentIdx < 0 || segmentIdx > mData.count() )
    return;

  mData.insert( segmentIdx, QgsAnchorWithHandles( pt ) );
}

void QgsBezierData::deleteAnchor( int idx )
{
  if ( idx < 0 || idx >= mData.count() )
    return;

  mData.removeAt( idx );
}

void QgsBezierData::retractHandle( int idx )
{
  const int anchorIdx = idx / 2;
  if ( anchorIdx < 0 || anchorIdx >= mData.count() )
    return;

  if ( idx % 2 == 0 )
    mData[anchorIdx].leftHandle = mData[anchorIdx].anchor;
  else
    mData[anchorIdx].rightHandle = mData[anchorIdx].anchor;
}

void QgsBezierData::extendHandle( int idx, const QgsPoint &pt )
{
  moveHandle( idx, pt );
}

QgsPoint QgsBezierData::anchor( int idx ) const
{
  if ( idx < 0 || idx >= mData.count() )
    return QgsPoint();
  return mData[idx].anchor;
}

QgsPoint QgsBezierData::handle( int idx ) const
{
  const int anchorIdx = idx / 2;
  if ( anchorIdx < 0 || anchorIdx >= mData.count() )
    return QgsPoint();

  if ( idx % 2 == 0 )
    return mData[anchorIdx].leftHandle;
  else
    return mData[anchorIdx].rightHandle;
}

QVector<QgsPoint> QgsBezierData::anchors() const
{
  QVector<QgsPoint> result;
  result.reserve( mData.count() );
  for ( const QgsAnchorWithHandles &awh : mData )
    result.append( awh.anchor );
  return result;
}

QVector<QgsPoint> QgsBezierData::handles() const
{
  QVector<QgsPoint> result;
  result.reserve( mData.count() * 2 );
  for ( const QgsAnchorWithHandles &awh : mData )
  {
    result.append( awh.leftHandle );
    result.append( awh.rightHandle );
  }
  return result;
}

const QgsAnchorWithHandles &QgsBezierData::anchorWithHandles( int idx ) const
{
  if ( idx < 0 || idx >= mData.count() )
    return sInvalidAnchor;
  return mData[idx];
}

QgsPointSequence QgsBezierData::interpolate() const
{
  QgsPointSequence result;

  if ( mData.count() < 2 )
  {
    // Not enough anchors for a curve, just return anchors
    for ( const QgsAnchorWithHandles &awh : mData )
      result.append( awh.anchor );
    return result;
  }

  // Add first anchor
  result.append( mData.first().anchor );

  // For each segment between consecutive anchors
  for ( int i = 0; i < mData.count() - 1; ++i )
  {
    const QgsPoint &p0 = mData[i].anchor;
    const QgsPoint &p1 = mData[i].rightHandle;
    const QgsPoint &p2 = mData[i + 1].leftHandle;
    const QgsPoint &p3 = mData[i + 1].anchor;

    // Interpolate the segment
    for ( int j = 1; j <= INTERPOLATION_POINTS; ++j )
    {
      const double t = static_cast<double>( j ) / INTERPOLATION_POINTS;
      result.append( QgsGeometryUtils::interpolatePointOnCubicBezier( p0, p1, p2, p3, t ) );
    }
  }

  return result;
}

std::unique_ptr<QgsNurbsCurve> QgsBezierData::asNurbsCurve() const
{
  const int n = mData.count();
  if ( n < 2 )
    return nullptr;

  // Build control points: anchor, handle_right, handle_left, anchor, ...
  // A piecewise cubic Bézier with n anchors has n-1 segments
  // Total control points: 1 + 3*(n-1) = 3n-2
  QVector<QgsPoint> ctrlPts;
  ctrlPts.append( mData[0].anchor );

  for ( int i = 0; i < n - 1; ++i )
  {
    ctrlPts.append( mData[i].rightHandle );
    ctrlPts.append( mData[i + 1].leftHandle );
    ctrlPts.append( mData[i + 1].anchor );
  }

  QVector<double> knots = QgsNurbsCurve::generateKnotsForBezierConversion( n );

  // Uniform weights (non-rational B-spline)
  QVector<double> weights( ctrlPts.count(), 1.0 );

  return std::make_unique<QgsNurbsCurve>( ctrlPts, 3, knots, weights );
}

void QgsBezierData::clear()
{
  mData.clear();
}

int QgsBezierData::findClosestAnchor( const QgsPoint &pt, double tolerance ) const
{
  int closestIdx = -1;
  double minDistSq = tolerance * tolerance;

  for ( int i = 0; i < mData.count(); ++i )
  {
    const double dx = mData[i].anchor.x() - pt.x();
    const double dy = mData[i].anchor.y() - pt.y();
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

  for ( int i = 0; i < mData.count(); ++i )
  {
    const QgsAnchorWithHandles &awh = mData[i];

    // Check left handle (index 2*i)
    if ( !qgsDoubleNear( awh.leftHandle.x(), awh.anchor.x() ) || !qgsDoubleNear( awh.leftHandle.y(), awh.anchor.y() ) )
    {
      const double dx = awh.leftHandle.x() - pt.x();
      const double dy = awh.leftHandle.y() - pt.y();
      const double distSq = dx * dx + dy * dy;
      if ( distSq < minDistSq )
      {
        minDistSq = distSq;
        closestIdx = i * 2;
      }
    }

    // Check right handle (index 2*i+1)
    if ( !qgsDoubleNear( awh.rightHandle.x(), awh.anchor.x() ) || !qgsDoubleNear( awh.rightHandle.y(), awh.anchor.y() ) )
    {
      const double dx = awh.rightHandle.x() - pt.x();
      const double dy = awh.rightHandle.y() - pt.y();
      const double distSq = dx * dx + dy * dy;
      if ( distSq < minDistSq )
      {
        minDistSq = distSq;
        closestIdx = i * 2 + 1;
      }
    }
  }

  return closestIdx;
}

int QgsBezierData::findClosestSegment( const QgsPoint &pt, double tolerance ) const
{
  if ( mData.count() < 2 )
    return -1;

  int closestSegment = -1;
  double minDistSq = tolerance * tolerance;

  // Check each segment
  for ( int i = 0; i < mData.count() - 1; ++i )
  {
    const QgsPoint &p0 = mData[i].anchor;
    const QgsPoint &p1 = mData[i].rightHandle;
    const QgsPoint &p2 = mData[i + 1].leftHandle;
    const QgsPoint &p3 = mData[i + 1].anchor;

    // Sample the curve and find minimum distance
    for ( int j = 0; j <= INTERPOLATION_POINTS; ++j )
    {
      const double t = static_cast<double>( j ) / INTERPOLATION_POINTS;
      const QgsPoint curvePoint = QgsGeometryUtils::interpolatePointOnCubicBezier( p0, p1, p2, p3, t );

      const double dx = curvePoint.x() - pt.x();
      const double dy = curvePoint.y() - pt.y();
      const double distSq = dx * dx + dy * dy;

      if ( distSq < minDistSq )
      {
        minDistSq = distSq;
        closestSegment = i;
      }
    }
  }

  return closestSegment;
}

///@endcond PRIVATE
