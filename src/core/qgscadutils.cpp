/***************************************************************************
                              qgscadutils.cpp
                             -------------------
    begin                : September 2017
    copyright            : (C) 2017 by Martin Dobias
    email                : wonder dot sk at gmail dot com
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgscadutils.h"

#include "qgslogger.h"
#include "qgssnappingutils.h"
#include "qgsgeometryutils.h"

// tolerances for soft constraints (last values, and common angles)
// for angles, both tolerance in pixels and degrees are used for better performance
static const double SOFT_CONSTRAINT_TOLERANCE_PIXEL = 15;
static const double SOFT_CONSTRAINT_TOLERANCE_DEGREES = 10;


/// @cond PRIVATE
struct EdgesOnlyFilter : public QgsPointLocator::MatchFilter
{
  bool acceptMatch( const QgsPointLocator::Match &m ) override { return m.hasEdge(); }
};
/// @endcond


QgsCadUtils::AlignMapPointOutput QgsCadUtils::alignMapPoint( const QgsPointXY &originalMapPoint, const QgsCadUtils::AlignMapPointContext &ctx )
{
  QgsCadUtils::AlignMapPointOutput res;
  res.valid = true;
  res.softLockCommonAngle = -1;

  // try to snap to anything
  const QgsPointLocator::Match snapMatch = ctx.snappingUtils->snapToMap( originalMapPoint, nullptr, true );
  res.snapMatch = snapMatch;
  QgsPointXY point = snapMatch.isValid() ? snapMatch.point() : originalMapPoint;
  QgsPointXY edgePt0, edgePt1;
  if ( snapMatch.hasEdge() )
  {
    snapMatch.edgePoints( edgePt0, edgePt1 );
    // note : res.edgeMatch should be removed, as we can just check snapMatch.hasEdge()
    res.edgeMatch = snapMatch;
  }
  else
  {
    res.edgeMatch = QgsPointLocator::Match();
  }

  QgsPointXY previousPt, penultimatePt;
  if ( ctx.cadPoints().count() >= 2 )
    previousPt = ctx.cadPoint( 1 );
  if ( ctx.cadPoints().count() >= 3 )
    penultimatePt = ctx.cadPoint( 2 );

  // *****************************
  // ---- X constraint
  if ( ctx.xConstraint.locked )
  {
    if ( !ctx.xConstraint.relative )
    {
      point.setX( ctx.xConstraint.value );
    }
    else if ( ctx.cadPoints().count() >= 2 )
    {
      point.setX( previousPt.x() + ctx.xConstraint.value );
    }
    if ( snapMatch.hasEdge() && !ctx.yConstraint.locked )
    {
      // intersect with snapped segment line at X coordinate
      const double dx = edgePt1.x() - edgePt0.x();
      if ( dx == 0 )
      {
        point.setY( edgePt0.y() );
      }
      else
      {
        const double dy = edgePt1.y() - edgePt0.y();
        point.setY( edgePt0.y() + ( dy * ( point.x() - edgePt0.x() ) ) / dx );
      }
    }
  }

  // *****************************
  // ---- Y constraint
  if ( ctx.yConstraint.locked )
  {
    if ( !ctx.yConstraint.relative )
    {
      point.setY( ctx.yConstraint.value );
    }
    else if ( ctx.cadPoints().count() >= 2 )
    {
      point.setY( previousPt.y() + ctx.yConstraint.value );
    }
    if ( snapMatch.hasEdge() && !ctx.xConstraint.locked )
    {
      // intersect with snapped segment line at Y coordinate
      const double dy = edgePt1.y() - edgePt0.y();
      if ( dy == 0 )
      {
        point.setX( edgePt0.x() );
      }
      else
      {
        const double dx = edgePt1.x() - edgePt0.x();
        point.setX( edgePt0.x() + ( dx * ( point.y() - edgePt0.y() ) ) / dy );
      }
    }
  }

  // *****************************
  // ---- Common Angle constraint
  if ( !ctx.angleConstraint.locked && ctx.cadPoints().count() >= 2 && ctx.commonAngleConstraint.locked && ctx.commonAngleConstraint.value != 0 )
  {
    const double commonAngle = ctx.commonAngleConstraint.value * M_PI / 180;
    // see if soft common angle constraint should be performed
    // only if not in HardLock mode
    double softAngle = std::atan2( point.y() - previousPt.y(),
                                   point.x() - previousPt.x() );
    double deltaAngle = 0;
    if ( ctx.commonAngleConstraint.relative && ctx.cadPoints().count() >= 3 )
    {
      // compute the angle relative to the last segment (0° is aligned with last segment)
      deltaAngle = std::atan2( previousPt.y() - penultimatePt.y(),
                               previousPt.x() - penultimatePt.x() );
      softAngle -= deltaAngle;
    }
    const int quo = std::round( softAngle / commonAngle );
    if ( std::fabs( softAngle - quo * commonAngle ) * 180.0 * M_1_PI <= SOFT_CONSTRAINT_TOLERANCE_DEGREES )
    {
      // also check the distance in pixel to the line, otherwise it's too sticky at long ranges
      softAngle = quo * commonAngle;
      // http://mathworld.wolfram.com/Point-LineDistance2-Dimensional.html
      // use the direction vector (cos(a),sin(a)) from previous point. |x2-x1|=1 since sin2+cos2=1
      const double dist = std::fabs( std::cos( softAngle + deltaAngle ) * ( previousPt.y() - point.y() )
                                     - std::sin( softAngle + deltaAngle ) * ( previousPt.x() - point.x() ) );
      if ( dist / ctx.mapUnitsPerPixel < SOFT_CONSTRAINT_TOLERANCE_PIXEL )
      {
        res.softLockCommonAngle = 180.0 / M_PI * softAngle;
      }
    }
  }

  // angle can be locked in one of the two ways:
  // 1. "hard" lock defined by the user
  // 2. "soft" lock from common angle (e.g. 45 degrees)
  bool angleLocked = false, angleRelative = false;
  double angleValueDeg = 0;
  if ( ctx.angleConstraint.locked )
  {
    angleLocked = true;
    angleRelative = ctx.angleConstraint.relative;
    angleValueDeg = ctx.angleConstraint.value;
  }
  else if ( res.softLockCommonAngle != -1 )
  {
    angleLocked = true;
    angleRelative = ctx.commonAngleConstraint.relative;
    angleValueDeg = res.softLockCommonAngle;
  }

  // *****************************
  // ---- Angle constraint
  // input angles are in degrees
  if ( angleLocked )
  {
    double angleValue = angleValueDeg * M_PI / 180;
    if ( angleRelative && ctx.cadPoints().count() >= 3 )
    {
      // compute the angle relative to the last segment (0° is aligned with last segment)
      angleValue += std::atan2( previousPt.y() - penultimatePt.y(),
                                previousPt.x() - penultimatePt.x() );
    }

    const double cosa = std::cos( angleValue );
    const double sina = std::sin( angleValue );
    const double v = ( point.x() - previousPt.x() ) * cosa + ( point.y() - previousPt.y() ) * sina;
    if ( ctx.xConstraint.locked && ctx.yConstraint.locked )
    {
      // do nothing if both X,Y are already locked
    }
    else if ( ctx.xConstraint.locked )
    {
      if ( qgsDoubleNear( cosa, 0.0 ) )
      {
        res.valid = false;
      }
      else
      {
        double x = ctx.xConstraint.value;
        if ( !ctx.xConstraint.relative )
        {
          x -= previousPt.x();
        }
        point.setY( previousPt.y() + x * sina / cosa );
      }
    }
    else if ( ctx.yConstraint.locked )
    {
      if ( qgsDoubleNear( sina, 0.0 ) )
      {
        res.valid = false;
      }
      else
      {
        double y = ctx.yConstraint.value;
        if ( !ctx.yConstraint.relative )
        {
          y -= previousPt.y();
        }
        point.setX( previousPt.x() + y * cosa / sina );
      }
    }
    else
    {
      point.setX( previousPt.x() + cosa * v );
      point.setY( previousPt.y() + sina * v );
    }

    if ( snapMatch.hasEdge() && !ctx.distanceConstraint.locked )
    {
      // magnetize to the intersection of the snapped segment and the lockedAngle

      // line of previous point + locked angle
      const double x1 = previousPt.x();
      const double y1 = previousPt.y();
      const double x2 = previousPt.x() + cosa;
      const double y2 = previousPt.y() + sina;
      // line of snapped segment
      const double x3 = edgePt0.x();
      const double y3 = edgePt0.y();
      const double x4 = edgePt1.x();
      const double y4 = edgePt1.y();

      const double d = ( x1 - x2 ) * ( y3 - y4 ) - ( y1 - y2 ) * ( x3 - x4 );

      // do not compute intersection if lines are almost parallel
      // this threshold might be adapted
      if ( std::fabs( d ) > 0.01 )
      {
        point.setX( ( ( x3 - x4 ) * ( x1 * y2 - y1 * x2 ) - ( x1 - x2 ) * ( x3 * y4 - y3 * x4 ) ) / d );
        point.setY( ( ( y3 - y4 ) * ( x1 * y2 - y1 * x2 ) - ( y1 - y2 ) * ( x3 * y4 - y3 * x4 ) ) / d );
      }
    }
  }

  // *****************************
  // ---- Distance constraint
  if ( ctx.distanceConstraint.locked && ctx.cadPoints().count() >= 2 )
  {
    if ( ctx.xConstraint.locked || ctx.yConstraint.locked )
    {
      // perform both to detect errors in constraints
      if ( ctx.xConstraint.locked )
      {
        const QgsPointXY verticalPt0( point.x(), point.y() );
        const QgsPointXY verticalPt1( point.x(), point.y() + 1 );
        res.valid &= QgsGeometryUtils::lineCircleIntersection( previousPt, ctx.distanceConstraint.value, verticalPt0, verticalPt1, point );
      }
      if ( ctx.yConstraint.locked )
      {
        const QgsPointXY horizontalPt0( point.x(), point.y() );
        const QgsPointXY horizontalPt1( point.x() + 1, point.y() );
        res.valid &= QgsGeometryUtils::lineCircleIntersection( previousPt, ctx.distanceConstraint.value, horizontalPt0, horizontalPt1, point );
      }
    }
    else
    {
      const double dist = std::sqrt( point.sqrDist( previousPt ) );
      if ( dist == 0 )
      {
        // handle case where mouse is over origin and distance constraint is enabled
        // take arbitrary horizontal line
        point.set( previousPt.x() + ctx.distanceConstraint.value, previousPt.y() );
      }
      else
      {
        const double vP = ctx.distanceConstraint.value / dist;
        point.set( previousPt.x() + ( point.x() - previousPt.x() ) * vP,
                   previousPt.y() + ( point.y() - previousPt.y() ) * vP );
      }

      if ( snapMatch.hasEdge() && !ctx.angleConstraint.locked )
      {
        // we will magnietize to the intersection of that segment and the lockedDistance !
        res.valid &= QgsGeometryUtils::lineCircleIntersection( previousPt, ctx.distanceConstraint.value, edgePt0, edgePt1, point );
      }
    }
  }

  // *****************************
  // ---- calculate CAD values
  QgsDebugMsgLevel( QStringLiteral( "point:             %1 %2" ).arg( point.x() ).arg( point.y() ), 4 );
  QgsDebugMsgLevel( QStringLiteral( "previous point:    %1 %2" ).arg( previousPt.x() ).arg( previousPt.y() ), 4 );
  QgsDebugMsgLevel( QStringLiteral( "penultimate point: %1 %2" ).arg( penultimatePt.x() ).arg( penultimatePt.y() ), 4 );
  //QgsDebugMsg( QStringLiteral( "dx: %1 dy: %2" ).arg( point.x() - previousPt.x() ).arg( point.y() - previousPt.y() ) );
  //QgsDebugMsg( QStringLiteral( "ddx: %1 ddy: %2" ).arg( previousPt.x() - penultimatePt.x() ).arg( previousPt.y() - penultimatePt.y() ) );

  res.finalMapPoint = point;

  return res;
}

void QgsCadUtils::AlignMapPointContext::dump() const
{
  QgsDebugMsg( QStringLiteral( "Constraints (locked / relative / value" ) );
  QgsDebugMsg( QStringLiteral( "Angle:    %1 %2 %3" ).arg( angleConstraint.locked ).arg( angleConstraint.relative ).arg( angleConstraint.value ) );
  QgsDebugMsg( QStringLiteral( "Distance: %1 %2 %3" ).arg( distanceConstraint.locked ).arg( distanceConstraint.relative ).arg( distanceConstraint.value ) );
  QgsDebugMsg( QStringLiteral( "X:        %1 %2 %3" ).arg( xConstraint.locked ).arg( xConstraint.relative ).arg( xConstraint.value ) );
  QgsDebugMsg( QStringLiteral( "Y:        %1 %2 %3" ).arg( yConstraint.locked ).arg( yConstraint.relative ).arg( yConstraint.value ) );
}
