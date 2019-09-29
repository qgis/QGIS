/***************************************************************************
  qgsinternalgeometryengine.cpp - QgsInternalGeometryEngine

 ---------------------
 begin                : 13.1.2016
 copyright            : (C) 2016 by Matthias Kuhn
 email                : matthias@opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsinternalgeometryengine.h"

#include "qgslinestring.h"
#include "qgsmultipolygon.h"
#include "qgspolygon.h"
#include "qgsmulticurve.h"
#include "qgsgeometry.h"
#include "qgsgeometryutils.h"
#include "qgslinesegment.h"
#include "qgscircle.h"
#include "qgslogger.h"
#include "qgstessellator.h"
#include "qgsfeedback.h"
#include <QTransform>
#include <functional>
#include <memory>
#include <queue>
#include <random>

QgsInternalGeometryEngine::QgsInternalGeometryEngine( const QgsGeometry &geometry )
  : mGeometry( geometry.constGet() )
{

}

/***************************************************************************
 * This class is considered CRITICAL and any change MUST be accompanied with
 * full unit tests.
 * See details in QEP #17
 ****************************************************************************/

QgsGeometry QgsInternalGeometryEngine::extrude( double x, double y ) const
{
  QVector<QgsLineString *> linesToProcess;

  const QgsMultiCurve *multiCurve = qgsgeometry_cast< const QgsMultiCurve * >( mGeometry );
  if ( multiCurve )
  {
    linesToProcess.reserve( multiCurve->partCount() );
    for ( int i = 0; i < multiCurve->partCount(); ++i )
    {
      linesToProcess << static_cast<QgsLineString *>( multiCurve->geometryN( i )->clone() );
    }
  }

  const QgsCurve *curve = qgsgeometry_cast< const QgsCurve * >( mGeometry );
  if ( curve )
  {
    linesToProcess << static_cast<QgsLineString *>( curve->segmentize() );
  }

  std::unique_ptr<QgsMultiPolygon> multipolygon( linesToProcess.size() > 1 ? new QgsMultiPolygon() : nullptr );
  QgsPolygon *polygon = nullptr;

  if ( !linesToProcess.empty() )
  {
    std::unique_ptr< QgsLineString > secondline;
    for ( QgsLineString *line : qgis::as_const( linesToProcess ) )
    {
      QTransform transform = QTransform::fromTranslate( x, y );

      secondline.reset( line->reversed() );
      secondline->transform( transform );

      line->append( secondline.get() );
      line->addVertex( line->pointN( 0 ) );

      polygon = new QgsPolygon();
      polygon->setExteriorRing( line );

      if ( multipolygon )
        multipolygon->addGeometry( polygon );
    }

    if ( multipolygon )
      return QgsGeometry( multipolygon.release() );
    else
      return QgsGeometry( polygon );
  }

  return QgsGeometry();
}



// polylabel implementation
// ported from the original JavaScript implementation developed by Vladimir Agafonkin
// originally licensed under the ISC License

/// @cond PRIVATE
class Cell
{
  public:
    Cell( double x, double y, double h, const QgsPolygon *polygon )
      : x( x )
      , y( y )
      , h( h )
      , d( polygon->pointDistanceToBoundary( x, y ) )
      , max( d + h * M_SQRT2 )
    {}

    //! Cell center x
    double x;
    //! Cell center y
    double y;
    //! Half the cell size
    double h;
    //! Distance from cell center to polygon
    double d;
    //! Maximum distance to polygon within a cell
    double max;
};

struct GreaterThanByMax
{
  bool operator()( const Cell *lhs, const Cell *rhs )
  {
    return rhs->max > lhs->max;
  }
};

Cell *getCentroidCell( const QgsPolygon *polygon )
{
  double area = 0;
  double x = 0;
  double y = 0;

  const QgsLineString *exterior = static_cast< const QgsLineString *>( polygon->exteriorRing() );
  int len = exterior->numPoints() - 1; //assume closed
  for ( int i = 0, j = len - 1; i < len; j = i++ )
  {
    double aX = exterior->xAt( i );
    double aY = exterior->yAt( i );
    double bX = exterior->xAt( j );
    double bY = exterior->yAt( j );
    double f = aX * bY - bX * aY;
    x += ( aX + bX ) * f;
    y += ( aY + bY ) * f;
    area += f * 3;
  }
  if ( area == 0 )
    return new Cell( exterior->xAt( 0 ), exterior->yAt( 0 ), 0, polygon );
  else
    return new Cell( x / area, y / area, 0.0, polygon );
}

QgsPoint surfacePoleOfInaccessibility( const QgsSurface *surface, double precision, double &distanceFromBoundary )
{
  std::unique_ptr< QgsPolygon > segmentizedPoly;
  const QgsPolygon *polygon = qgsgeometry_cast< const QgsPolygon * >( surface );
  if ( !polygon )
  {
    segmentizedPoly.reset( static_cast< QgsPolygon *>( surface->segmentize() ) );
    polygon = segmentizedPoly.get();
  }

  // start with the bounding box
  QgsRectangle bounds = polygon->boundingBox();

  // initial parameters
  double cellSize = std::min( bounds.width(), bounds.height() );

  if ( qgsDoubleNear( cellSize, 0.0 ) )
    return QgsPoint( bounds.xMinimum(), bounds.yMinimum() );

  double h = cellSize / 2.0;
  std::priority_queue< Cell *, std::vector<Cell *>, GreaterThanByMax > cellQueue;

  // cover polygon with initial cells
  for ( double x = bounds.xMinimum(); x < bounds.xMaximum(); x += cellSize )
  {
    for ( double y = bounds.yMinimum(); y < bounds.yMaximum(); y += cellSize )
    {
      cellQueue.push( new Cell( x + h, y + h, h, polygon ) );
    }
  }

  // take centroid as the first best guess
  std::unique_ptr< Cell > bestCell( getCentroidCell( polygon ) );

  // special case for rectangular polygons
  std::unique_ptr< Cell > bboxCell( new Cell( bounds.xMinimum() + bounds.width() / 2.0,
                                    bounds.yMinimum() + bounds.height() / 2.0,
                                    0, polygon ) );
  if ( bboxCell->d > bestCell->d )
  {
    bestCell = std::move( bboxCell );
  }

  while ( !cellQueue.empty() )
  {
    // pick the most promising cell from the queue
    std::unique_ptr< Cell > cell( cellQueue.top() );
    cellQueue.pop();
    Cell *currentCell = cell.get();

    // update the best cell if we found a better one
    if ( currentCell->d > bestCell->d )
    {
      bestCell = std::move( cell );
    }

    // do not drill down further if there's no chance of a better solution
    if ( currentCell->max - bestCell->d <= precision )
      continue;

    // split the cell into four cells
    h = currentCell->h / 2.0;
    cellQueue.push( new Cell( currentCell->x - h, currentCell->y - h, h, polygon ) );
    cellQueue.push( new Cell( currentCell->x + h, currentCell->y - h, h, polygon ) );
    cellQueue.push( new Cell( currentCell->x - h, currentCell->y + h, h, polygon ) );
    cellQueue.push( new Cell( currentCell->x + h, currentCell->y + h, h, polygon ) );
  }

  distanceFromBoundary = bestCell->d;

  return QgsPoint( bestCell->x, bestCell->y );
}

///@endcond

QgsGeometry QgsInternalGeometryEngine::poleOfInaccessibility( double precision, double *distanceFromBoundary ) const
{
  if ( distanceFromBoundary )
    *distanceFromBoundary = std::numeric_limits<double>::max();

  if ( !mGeometry || mGeometry->isEmpty() )
    return QgsGeometry();

  if ( precision <= 0 )
    return QgsGeometry();

  if ( const QgsGeometryCollection *gc = qgsgeometry_cast< const QgsGeometryCollection *>( mGeometry ) )
  {
    int numGeom = gc->numGeometries();
    double maxDist = 0;
    QgsPoint bestPoint;
    bool found = false;
    for ( int i = 0; i < numGeom; ++i )
    {
      const QgsSurface *surface = qgsgeometry_cast< const QgsSurface * >( gc->geometryN( i ) );
      if ( !surface )
        continue;

      found = true;
      double dist = std::numeric_limits<double>::max();
      QgsPoint p = surfacePoleOfInaccessibility( surface, precision, dist );
      if ( dist > maxDist )
      {
        maxDist = dist;
        bestPoint = p;
      }
    }

    if ( !found )
      return QgsGeometry();

    if ( distanceFromBoundary )
      *distanceFromBoundary = maxDist;
    return QgsGeometry( new QgsPoint( bestPoint ) );
  }
  else
  {
    const QgsSurface *surface = qgsgeometry_cast< const QgsSurface * >( mGeometry );
    if ( !surface )
      return QgsGeometry();

    double dist = std::numeric_limits<double>::max();
    QgsPoint p = surfacePoleOfInaccessibility( surface, precision, dist );
    if ( distanceFromBoundary )
      *distanceFromBoundary = dist;
    return QgsGeometry( new QgsPoint( p ) );
  }
}


// helpers for orthogonalize
// adapted from original code in potlatch/id osm editor

bool dotProductWithinAngleTolerance( double dotProduct, double lowerThreshold, double upperThreshold )
{
  return lowerThreshold > std::fabs( dotProduct ) || std::fabs( dotProduct ) > upperThreshold;
}

double normalizedDotProduct( const QgsPoint &a, const QgsPoint &b, const QgsPoint &c )
{
  QgsVector p = a - b;
  QgsVector q = c - b;

  if ( p.length() > 0 )
    p = p.normalized();
  if ( q.length() > 0 )
    q = q.normalized();

  return p * q;
}

double squareness( QgsLineString *ring, double lowerThreshold, double upperThreshold )
{
  double sum = 0.0;

  bool isClosed = ring->isClosed();
  int numPoints = ring->numPoints();
  QgsPoint a;
  QgsPoint b;
  QgsPoint c;

  for ( int i = 0; i < numPoints; ++i )
  {
    if ( !isClosed && i == numPoints - 1 )
      break;
    else if ( !isClosed && i == 0 )
    {
      b = ring->pointN( 0 );
      c = ring->pointN( 1 );
    }
    else
    {
      if ( i == 0 )
      {
        a = ring->pointN( numPoints - 1 );
        b = ring->pointN( 0 );
      }
      if ( i == numPoints - 1 )
        c = ring->pointN( 0 );
      else
        c = ring->pointN( i + 1 );

      double dotProduct = normalizedDotProduct( a, b, c );
      if ( !dotProductWithinAngleTolerance( dotProduct, lowerThreshold, upperThreshold ) )
        continue;

      sum += 2.0 * std::min( std::fabs( dotProduct - 1.0 ), std::min( std::fabs( dotProduct ), std::fabs( dotProduct + 1 ) ) );
    }
    a = b;
    b = c;
  }

  return sum;
}

QgsVector calcMotion( const QgsPoint &a, const QgsPoint &b, const QgsPoint &c,
                      double lowerThreshold, double upperThreshold )
{
  QgsVector p = a - b;
  QgsVector q = c - b;

  if ( qgsDoubleNear( p.length(), 0.0 ) || qgsDoubleNear( q.length(), 0.0 ) )
    return QgsVector( 0, 0 );

  // 2.0 is a magic number from the original JOSM source code
  double scale = 2.0 * std::min( p.length(), q.length() );

  p = p.normalized();
  q = q.normalized();
  double dotProduct = p * q;

  if ( !dotProductWithinAngleTolerance( dotProduct, lowerThreshold, upperThreshold ) )
  {
    return QgsVector( 0, 0 );
  }

  // wonderful nasty hack which has survived through JOSM -> id -> QGIS
  // to deal with almost-straight segments (angle is closer to 180 than to 90/270).
  if ( dotProduct < -M_SQRT1_2 )
    dotProduct += 1.0;

  QgsVector new_v = p + q;
  // 0.1 magic number from JOSM implementation - think this is to limit each iterative step
  return new_v.normalized() * ( 0.1 * dotProduct * scale );
}

QgsLineString *doOrthogonalize( QgsLineString *ring, int iterations, double tolerance, double lowerThreshold, double upperThreshold )
{
  double minScore = std::numeric_limits<double>::max();

  bool isClosed = ring->isClosed();
  int numPoints = ring->numPoints();

  std::unique_ptr< QgsLineString > best( ring->clone() );

  QVector< QgsVector > /* yep */ motions;
  motions.reserve( numPoints );

  for ( int it = 0; it < iterations; ++it )
  {
    motions.resize( 0 ); // avoid re-allocations

    // first loop through an calculate all motions
    QgsPoint a;
    QgsPoint b;
    QgsPoint c;
    for ( int i = 0; i < numPoints; ++i )
    {
      if ( isClosed && i == numPoints - 1 )
        motions << motions.at( 0 ); //closed ring, so last point follows first point motion
      else if ( !isClosed && ( i == 0 || i == numPoints - 1 ) )
      {
        b = ring->pointN( 0 );
        c = ring->pointN( 1 );
        motions << QgsVector( 0, 0 ); //non closed line, leave first/last vertex untouched
      }
      else
      {
        if ( i == 0 )
        {
          a = ring->pointN( numPoints - 1 );
          b = ring->pointN( 0 );
        }
        if ( i == numPoints - 1 )
          c = ring->pointN( 0 );
        else
          c = ring->pointN( i + 1 );

        motions << calcMotion( a, b, c, lowerThreshold, upperThreshold );
      }
      a = b;
      b = c;
    }

    // then apply them
    for ( int i = 0; i < numPoints; ++i )
    {
      ring->setXAt( i, ring->xAt( i ) + motions.at( i ).x() );
      ring->setYAt( i, ring->yAt( i ) + motions.at( i ).y() );
    }

    double newScore = squareness( ring, lowerThreshold, upperThreshold );
    if ( newScore < minScore )
    {
      best.reset( ring->clone() );
      minScore = newScore;
    }

    if ( minScore < tolerance )
      break;
  }

  delete ring;

  return best.release();
}


QgsAbstractGeometry *orthogonalizeGeom( const QgsAbstractGeometry *geom, int maxIterations, double tolerance, double lowerThreshold, double upperThreshold )
{
  std::unique_ptr< QgsAbstractGeometry > segmentizedCopy;
  if ( QgsWkbTypes::isCurvedType( geom->wkbType() ) )
  {
    segmentizedCopy.reset( geom->segmentize() );
    geom = segmentizedCopy.get();
  }

  if ( QgsWkbTypes::geometryType( geom->wkbType() ) == QgsWkbTypes::LineGeometry )
  {
    return doOrthogonalize( static_cast< QgsLineString * >( geom->clone() ),
                            maxIterations, tolerance, lowerThreshold, upperThreshold );
  }
  else
  {
    // polygon
    const QgsPolygon *polygon = static_cast< const QgsPolygon * >( geom );
    QgsPolygon *result = new QgsPolygon();

    result->setExteriorRing( doOrthogonalize( static_cast< QgsLineString * >( polygon->exteriorRing()->clone() ),
                             maxIterations, tolerance, lowerThreshold, upperThreshold ) );
    for ( int i = 0; i < polygon->numInteriorRings(); ++i )
    {
      result->addInteriorRing( doOrthogonalize( static_cast< QgsLineString * >( polygon->interiorRing( i )->clone() ),
                               maxIterations, tolerance, lowerThreshold, upperThreshold ) );
    }

    return result;
  }
}

QgsGeometry QgsInternalGeometryEngine::orthogonalize( double tolerance, int maxIterations, double angleThreshold ) const
{
  if ( !mGeometry || ( QgsWkbTypes::geometryType( mGeometry->wkbType() ) != QgsWkbTypes::LineGeometry
                       && QgsWkbTypes::geometryType( mGeometry->wkbType() ) != QgsWkbTypes::PolygonGeometry ) )
  {
    return QgsGeometry();
  }

  double lowerThreshold = std::cos( ( 90 - angleThreshold ) * M_PI / 180.00 );
  double upperThreshold = std::cos( angleThreshold * M_PI / 180.0 );

  if ( const QgsGeometryCollection *gc = qgsgeometry_cast< const QgsGeometryCollection *>( mGeometry ) )
  {
    int numGeom = gc->numGeometries();
    QVector< QgsAbstractGeometry * > geometryList;
    geometryList.reserve( numGeom );
    for ( int i = 0; i < numGeom; ++i )
    {
      geometryList << orthogonalizeGeom( gc->geometryN( i ), maxIterations, tolerance, lowerThreshold, upperThreshold );
    }

    QgsGeometry first = QgsGeometry( geometryList.takeAt( 0 ) );
    for ( QgsAbstractGeometry *g : qgis::as_const( geometryList ) )
    {
      first.addPart( g );
    }
    return first;
  }
  else
  {
    return QgsGeometry( orthogonalizeGeom( mGeometry, maxIterations, tolerance, lowerThreshold, upperThreshold ) );
  }
}

// if extraNodesPerSegment < 0, then use distance based mode
QgsLineString *doDensify( const QgsLineString *ring, int extraNodesPerSegment = -1, double distance = 1 )
{
  QVector< double > outX;
  QVector< double > outY;
  QVector< double > outZ;
  QVector< double > outM;
  double multiplier = 1.0 / double( extraNodesPerSegment + 1 );

  int nPoints = ring->numPoints();
  outX.reserve( ( extraNodesPerSegment + 1 ) * nPoints );
  outY.reserve( ( extraNodesPerSegment + 1 ) * nPoints );
  bool withZ = ring->is3D();
  if ( withZ )
    outZ.reserve( ( extraNodesPerSegment + 1 ) * nPoints );
  bool withM = ring->isMeasure();
  if ( withM )
    outM.reserve( ( extraNodesPerSegment + 1 ) * nPoints );
  double x1 = 0;
  double x2 = 0;
  double y1 = 0;
  double y2 = 0;
  double z1 = 0;
  double z2 = 0;
  double m1 = 0;
  double m2 = 0;
  double xOut = 0;
  double yOut = 0;
  double zOut = 0;
  double mOut = 0;
  int extraNodesThisSegment = extraNodesPerSegment;
  for ( int i = 0; i < nPoints - 1; ++i )
  {
    x1 = ring->xAt( i );
    x2 = ring->xAt( i + 1 );
    y1 = ring->yAt( i );
    y2 = ring->yAt( i + 1 );
    if ( withZ )
    {
      z1 = ring->zAt( i );
      z2 = ring->zAt( i + 1 );
    }
    if ( withM )
    {
      m1 = ring->mAt( i );
      m2 = ring->mAt( i + 1 );
    }

    outX << x1;
    outY << y1;
    if ( withZ )
      outZ << z1;
    if ( withM )
      outM << m1;

    if ( extraNodesPerSegment < 0 )
    {
      // distance mode
      extraNodesThisSegment = std::floor( std::sqrt( ( x2 - x1 ) * ( x2 - x1 ) + ( y2 - y1 ) * ( y2 - y1 ) ) / distance );
      if ( extraNodesThisSegment >= 1 )
        multiplier = 1.0 / ( extraNodesThisSegment + 1 );
    }

    for ( int j = 0; j < extraNodesThisSegment; ++j )
    {
      double delta = multiplier * ( j + 1 );
      xOut = x1 + delta * ( x2 - x1 );
      yOut = y1 + delta * ( y2 - y1 );
      if ( withZ )
        zOut = z1 + delta * ( z2 - z1 );
      if ( withM )
        mOut = m1 + delta * ( m2 - m1 );

      outX << xOut;
      outY << yOut;
      if ( withZ )
        outZ << zOut;
      if ( withM )
        outM << mOut;
    }
  }
  outX << ring->xAt( nPoints - 1 );
  outY << ring->yAt( nPoints - 1 );
  if ( withZ )
    outZ << ring->zAt( nPoints - 1 );
  if ( withM )
    outM << ring->mAt( nPoints - 1 );

  QgsLineString *result = new QgsLineString( outX, outY, outZ, outM );
  return result;
}

QgsAbstractGeometry *densifyGeometry( const QgsAbstractGeometry *geom, int extraNodesPerSegment = 1, double distance = 1 )
{
  std::unique_ptr< QgsAbstractGeometry > segmentizedCopy;
  if ( QgsWkbTypes::isCurvedType( geom->wkbType() ) )
  {
    segmentizedCopy.reset( geom->segmentize() );
    geom = segmentizedCopy.get();
  }

  if ( QgsWkbTypes::geometryType( geom->wkbType() ) == QgsWkbTypes::LineGeometry )
  {
    return doDensify( static_cast< const QgsLineString * >( geom ), extraNodesPerSegment, distance );
  }
  else
  {
    // polygon
    const QgsPolygon *polygon = static_cast< const QgsPolygon * >( geom );
    QgsPolygon *result = new QgsPolygon();

    result->setExteriorRing( doDensify( static_cast< const QgsLineString * >( polygon->exteriorRing() ),
                                        extraNodesPerSegment, distance ) );
    for ( int i = 0; i < polygon->numInteriorRings(); ++i )
    {
      result->addInteriorRing( doDensify( static_cast< const QgsLineString * >( polygon->interiorRing( i ) ),
                                          extraNodesPerSegment, distance ) );
    }

    return result;
  }
}

QgsGeometry QgsInternalGeometryEngine::densifyByCount( int extraNodesPerSegment ) const
{
  if ( !mGeometry )
  {
    return QgsGeometry();
  }

  if ( QgsWkbTypes::geometryType( mGeometry->wkbType() ) == QgsWkbTypes::PointGeometry )
  {
    return QgsGeometry( mGeometry->clone() ); // point geometry, nothing to do
  }

  if ( const QgsGeometryCollection *gc = qgsgeometry_cast< const QgsGeometryCollection *>( mGeometry ) )
  {
    int numGeom = gc->numGeometries();
    QVector< QgsAbstractGeometry * > geometryList;
    geometryList.reserve( numGeom );
    for ( int i = 0; i < numGeom; ++i )
    {
      geometryList << densifyGeometry( gc->geometryN( i ), extraNodesPerSegment );
    }

    QgsGeometry first = QgsGeometry( geometryList.takeAt( 0 ) );
    for ( QgsAbstractGeometry *g : qgis::as_const( geometryList ) )
    {
      first.addPart( g );
    }
    return first;
  }
  else
  {
    return QgsGeometry( densifyGeometry( mGeometry, extraNodesPerSegment ) );
  }
}

QgsGeometry QgsInternalGeometryEngine::densifyByDistance( double distance ) const
{
  if ( !mGeometry )
  {
    return QgsGeometry();
  }

  if ( QgsWkbTypes::geometryType( mGeometry->wkbType() ) == QgsWkbTypes::PointGeometry )
  {
    return QgsGeometry( mGeometry->clone() ); // point geometry, nothing to do
  }

  if ( const QgsGeometryCollection *gc = qgsgeometry_cast< const QgsGeometryCollection *>( mGeometry ) )
  {
    int numGeom = gc->numGeometries();
    QVector< QgsAbstractGeometry * > geometryList;
    geometryList.reserve( numGeom );
    for ( int i = 0; i < numGeom; ++i )
    {
      geometryList << densifyGeometry( gc->geometryN( i ), -1, distance );
    }

    QgsGeometry first = QgsGeometry( geometryList.takeAt( 0 ) );
    for ( QgsAbstractGeometry *g : qgis::as_const( geometryList ) )
    {
      first.addPart( g );
    }
    return first;
  }
  else
  {
    return QgsGeometry( densifyGeometry( mGeometry, -1, distance ) );
  }
}

///@cond PRIVATE
//
// QgsLineSegmentDistanceComparer
//

// adapted for QGIS geometry classes from original work at https://github.com/trylock/visibility by trylock
bool QgsLineSegmentDistanceComparer::operator()( QgsLineSegment2D ab, QgsLineSegment2D cd ) const
{
  Q_ASSERT_X( ab.pointLeftOfLine( mOrigin ) != 0,
              "line_segment_dist_comparer",
              "AB must not be collinear with the origin." );
  Q_ASSERT_X( cd.pointLeftOfLine( mOrigin ) != 0,
              "line_segment_dist_comparer",
              "CD must not be collinear with the origin." );

  // flip the segments so that if there are common endpoints,
  // they will be the segment's start points
  if ( ab.end() == cd.start() || ab.end() == cd.end() )
    ab.reverse();
  if ( ab.start() == cd.end() )
    cd.reverse();

  // cases with common endpoints
  if ( ab.start() == cd.start() )
  {
    const int oad = QgsGeometryUtils::leftOfLine( cd.endX(), cd.endY(), mOrigin.x(), mOrigin.y(), ab.startX(), ab.startY() );
    const int oab = ab.pointLeftOfLine( mOrigin );
    if ( ab.end() == cd.end() || oad != oab )
      return false;
    else
      return ab.pointLeftOfLine( cd.end() ) != oab;
  }
  else
  {
    // cases without common endpoints
    const int cda = cd.pointLeftOfLine( ab.start() );
    const int cdb = cd.pointLeftOfLine( ab.end() );
    if ( cdb == 0 && cda == 0 )
    {
      return mOrigin.sqrDist( ab.start() ) < mOrigin.sqrDist( cd.start() );
    }
    else if ( cda == cdb || cda == 0 || cdb == 0 )
    {
      const int cdo = cd.pointLeftOfLine( mOrigin );
      return cdo == cda || cdo == cdb;
    }
    else
    {
      const int abo = ab.pointLeftOfLine( mOrigin );
      return abo != ab.pointLeftOfLine( cd.start() );
    }
  }
}

//
// QgsClockwiseAngleComparer
//

bool QgsClockwiseAngleComparer::operator()( const QgsPointXY &a, const QgsPointXY &b ) const
{
  const bool aIsLeft = a.x() < mVertex.x();
  const bool bIsLeft = b.x() < mVertex.x();
  if ( aIsLeft != bIsLeft )
    return bIsLeft;

  if ( qgsDoubleNear( a.x(), mVertex.x() ) && qgsDoubleNear( b.x(), mVertex.x() ) )
  {
    if ( a.y() >= mVertex.y() || b.y() >= mVertex.y() )
    {
      return b.y() < a.y();
    }
    else
    {
      return a.y() < b.y();
    }
  }
  else
  {
    const QgsVector oa = a - mVertex;
    const QgsVector ob = b - mVertex;
    const double det = oa.crossProduct( ob );
    if ( qgsDoubleNear( det, 0.0 ) )
    {
      return oa.lengthSquared() < ob.lengthSquared();
    }
    else
    {
      return det < 0;
    }
  }
}

///@endcond PRIVATE

//
// QgsRay2D
//

bool QgsRay2D::intersects( const QgsLineSegment2D &segment, QgsPointXY &intersectPoint ) const
{
  const QgsVector ao = origin - segment.start();
  const QgsVector ab = segment.end() - segment.start();
  const double det = ab.crossProduct( direction );
  if ( qgsDoubleNear( det, 0.0 ) )
  {
    const int abo = segment.pointLeftOfLine( origin );
    if ( abo != 0 )
    {
      return false;
    }
    else
    {
      const double distA = ao * direction;
      const double distB = ( origin - segment.end() ) * direction;

      if ( distA > 0 && distB > 0 )
      {
        return false;
      }
      else
      {
        if ( ( distA > 0 ) != ( distB > 0 ) )
          intersectPoint = origin;
        else if ( distA > distB ) // at this point, both distances are negative
          intersectPoint = segment.start(); // hence the nearest point is A
        else
          intersectPoint = segment.end();
        return true;
      }
    }
  }
  else
  {
    const double u = ao.crossProduct( direction ) / det;
    if ( u < 0.0 || 1.0 < u )
    {
      return false;
    }
    else
    {
      const double t = -ab.crossProduct( ao ) / det;
      intersectPoint = origin + direction * t;
      return qgsDoubleNear( t, 0.0 ) || t > 0;
    }
  }
}

QVector<QgsPointXY> generateSegmentCurve( const QgsPoint &center1, const double radius1, const QgsPoint &center2, const double radius2 )
{
  // ensure that first circle is smaller than second
  if ( radius1 > radius2 )
    return generateSegmentCurve( center2, radius2, center1, radius1 );

  QgsPointXY t1;
  QgsPointXY t2;
  QgsPointXY t3;
  QgsPointXY t4;
  QVector<QgsPointXY> points;
  if ( QgsGeometryUtils::circleCircleOuterTangents( center1, radius1, center2, radius2, t1, t2, t3, t4 ) )
  {
    points << t1
           << t2
           << t4
           << t3;
  }
  return points;
}

// partially ported from JTS VariableWidthBuffer,
// https://github.com/topobyte/jts/blob/master/jts-lab/src/main/java/com/vividsolutions/jts/operation/buffer/VariableWidthBuffer.java

QgsGeometry QgsInternalGeometryEngine::variableWidthBuffer( int segments, const std::function< std::unique_ptr< double[] >( const QgsLineString *line ) > &widthFunction ) const
{
  if ( !mGeometry )
  {
    return QgsGeometry();
  }

  std::vector< std::unique_ptr<QgsLineString > > linesToProcess;

  const QgsMultiCurve *multiCurve = qgsgeometry_cast< const QgsMultiCurve * >( mGeometry );
  if ( multiCurve )
  {
    for ( int i = 0; i < multiCurve->partCount(); ++i )
    {
      if ( static_cast< const QgsCurve * >( multiCurve->geometryN( i ) )->nCoordinates() == 0 )
        continue; // skip 0 length lines

      linesToProcess.emplace_back( static_cast<QgsLineString *>( multiCurve->geometryN( i )->clone() ) );
    }
  }

  const QgsCurve *curve = qgsgeometry_cast< const QgsCurve * >( mGeometry );
  if ( curve )
  {
    if ( curve->nCoordinates() > 0 )
      linesToProcess.emplace_back( static_cast<QgsLineString *>( curve->segmentize() ) );
  }

  if ( linesToProcess.empty() )
  {
    QgsGeometry g;
    g.mLastError = QStringLiteral( "Input geometry was not a curve type geometry" );
    return g;
  }

  QVector<QgsGeometry> bufferedLines;
  bufferedLines.reserve( linesToProcess.size() );

  for ( std::unique_ptr< QgsLineString > &line : linesToProcess )
  {
    QVector<QgsGeometry> parts;
    QgsPoint prevPoint;
    double prevRadius = 0;
    QgsGeometry prevCircle;

    std::unique_ptr< double[] > widths = widthFunction( line.get() ) ;
    for ( int i = 0; i < line->nCoordinates(); ++i )
    {
      QgsPoint thisPoint = line->pointN( i );
      QgsGeometry thisCircle;
      double thisRadius = widths[ i ] / 2.0;
      if ( thisRadius > 0 )
      {
        QgsGeometry p = QgsGeometry( thisPoint.clone() );

        QgsCircle circ( thisPoint, thisRadius );
        thisCircle = QgsGeometry( circ.toPolygon( segments * 4 ) );
        parts << thisCircle;
      }
      else
      {
        thisCircle = QgsGeometry( thisPoint.clone() );
      }

      if ( i > 0 )
      {
        if ( prevRadius > 0 || thisRadius > 0 )
        {
          QVector< QgsPointXY > points = generateSegmentCurve( prevPoint, prevRadius, thisPoint, thisRadius );
          if ( !points.empty() )
          {
            // snap points to circle vertices

            int atVertex = 0;
            int beforeVertex = 0;
            int afterVertex = 0;
            double sqrDist = 0;
            double sqrDistPrev = 0;
            for ( int j = 0; j < points.count(); ++j )
            {
              QgsPointXY pA = prevCircle.closestVertex( points.at( j ), atVertex, beforeVertex, afterVertex, sqrDistPrev );
              QgsPointXY pB = thisCircle.closestVertex( points.at( j ), atVertex, beforeVertex, afterVertex, sqrDist );
              points[j] = sqrDistPrev < sqrDist ? pA : pB;
            }
            // close ring
            points.append( points.at( 0 ) );

            std::unique_ptr< QgsPolygon > poly = qgis::make_unique< QgsPolygon >();
            poly->setExteriorRing( new QgsLineString( points ) );
            if ( poly->area() > 0 )
              parts << QgsGeometry( std::move( poly ) );
          }
        }
      }
      prevPoint = thisPoint;
      prevRadius = thisRadius;
      prevCircle = thisCircle;
    }

    bufferedLines << QgsGeometry::unaryUnion( parts );
  }

  return QgsGeometry::collectGeometry( bufferedLines );
}

QgsGeometry QgsInternalGeometryEngine::taperedBuffer( double start, double end, int segments ) const
{
  start = std::fabs( start );
  end = std::fabs( end );

  auto interpolateWidths = [ start, end ]( const QgsLineString * line )->std::unique_ptr< double [] >
  {
    // ported from JTS VariableWidthBuffer,
    // https://github.com/topobyte/jts/blob/master/jts-lab/src/main/java/com/vividsolutions/jts/operation/buffer/VariableWidthBuffer.java
    std::unique_ptr< double [] > widths( new double[ line->nCoordinates() ] );
    widths[0] = start;
    widths[line->nCoordinates() - 1] = end;

    double lineLength = line->length();
    double currentLength = 0;
    QgsPoint prevPoint = line->pointN( 0 );
    for ( int i = 1; i < line->nCoordinates() - 1; ++i )
    {
      QgsPoint point = line->pointN( i );
      double segmentLength = point.distance( prevPoint );
      currentLength += segmentLength;
      double lengthFraction = lineLength > 0 ? currentLength / lineLength : 1;
      double delta = lengthFraction * ( end - start );
      widths[i] = start + delta;
      prevPoint = point;
    }
    return widths;
  };

  return variableWidthBuffer( segments, interpolateWidths );
}

QgsGeometry QgsInternalGeometryEngine::variableWidthBufferByM( int segments ) const
{
  auto widthByM = []( const QgsLineString * line )->std::unique_ptr< double [] >
  {
    std::unique_ptr< double [] > widths( new double[ line->nCoordinates() ] );
    for ( int i = 0; i < line->nCoordinates(); ++i )
    {
      widths[ i ] = line->mAt( i );
    }
    return widths;
  };

  return variableWidthBuffer( segments, widthByM );
}

QVector<QgsPointXY> QgsInternalGeometryEngine::randomPointsInPolygon( const QgsGeometry &polygon, int count,
    const std::function< bool( const QgsPointXY & ) > &acceptPoint, unsigned long seed, QgsFeedback *feedback )
{
  if ( polygon.type() != QgsWkbTypes::PolygonGeometry || count == 0 )
    return QVector< QgsPointXY >();

  // step 1 - tessellate the polygon to triangles
  QgsRectangle bounds = polygon.boundingBox();
  QgsTessellator t( bounds, false, false, false, true );

  if ( polygon.isMultipart() )
  {
    const QgsMultiSurface *ms = qgsgeometry_cast< const QgsMultiSurface * >( polygon.constGet() );
    for ( int i = 0; i < ms->numGeometries(); ++i )
    {
      if ( feedback && feedback->isCanceled() )
        return QVector< QgsPointXY >();

      if ( QgsPolygon *poly = qgsgeometry_cast< QgsPolygon * >( ms->geometryN( i ) ) )
      {
        t.addPolygon( *poly, 0 );
      }
      else
      {
        std::unique_ptr< QgsPolygon > p( qgsgeometry_cast< QgsPolygon * >( ms->geometryN( i )->segmentize() ) );
        t.addPolygon( *p, 0 );
      }
    }
  }
  else
  {
    if ( const QgsPolygon *poly = qgsgeometry_cast< const QgsPolygon * >( polygon.constGet() ) )
    {
      t.addPolygon( *poly, 0 );
    }
    else
    {
      std::unique_ptr< QgsPolygon > p( qgsgeometry_cast< QgsPolygon * >( polygon.constGet()->segmentize() ) );
      t.addPolygon( *p, 0 );
    }
  }

  if ( feedback && feedback->isCanceled() )
    return QVector< QgsPointXY >();

  const QVector<float> triangleData = t.data();
  if ( triangleData.empty() )
    return QVector< QgsPointXY >(); //hm

  // calculate running sum of triangle areas
  std::vector< double > cumulativeAreas;
  cumulativeAreas.reserve( t.dataVerticesCount() / 3 );
  double totalArea = 0;
  for ( auto it = triangleData.constBegin(); it != triangleData.constEnd(); )
  {
    if ( feedback && feedback->isCanceled() )
      return QVector< QgsPointXY >();

    const float aX = *it++;
    ( void )it++; // z
    const float aY = -( *it++ );
    const float bX = *it++;
    ( void )it++; // z
    const float bY = -( *it++ );
    const float cX = *it++;
    ( void )it++; // z
    const float cY = -( *it++ );

    const double area = QgsGeometryUtils::triangleArea( aX, aY, bX, bY, cX, cY );
    totalArea += area;
    cumulativeAreas.emplace_back( totalArea );
  }

  std::random_device rd;
  std::mt19937 mt( seed == 0 ? rd() : seed );
  std::uniform_real_distribution<> uniformDist( 0, 1 );

  // selects a random triangle, weighted by triangle area
  auto selectRandomTriangle = [&cumulativeAreas, totalArea]( double random )->int
  {
    int triangle = 0;
    const double target = random * totalArea;
    for ( auto it = cumulativeAreas.begin(); it != cumulativeAreas.end(); ++it, triangle++ )
    {
      if ( *it > target )
        return triangle;
    }
    Q_ASSERT_X( false, "QgsInternalGeometryEngine::randomPointsInPolygon", "Invalid random triangle index" );
    return 0; // no warnings
  };


  QVector<QgsPointXY> result;
  result.reserve( count );
  for ( int i = 0; i < count; )
  {
    if ( feedback && feedback->isCanceled() )
      return QVector< QgsPointXY >();

    const double triangleIndexRnd = uniformDist( mt );
    // pick random triangle, weighted by triangle area
    const int triangleIndex = selectRandomTriangle( triangleIndexRnd );

    // generate a random point inside this triangle
    const double weightB = uniformDist( mt );
    const double weightC = uniformDist( mt );
    double x;
    double y;

    // get triangle
    const double aX = triangleData.at( triangleIndex * 9 ) + bounds.xMinimum();
    const double aY = -triangleData.at( triangleIndex * 9 + 2 ) + bounds.yMinimum();
    const double bX = triangleData.at( triangleIndex * 9 + 3 ) + bounds.xMinimum();
    const double bY = -triangleData.at( triangleIndex * 9 + 5 ) + bounds.yMinimum();
    const double cX = triangleData.at( triangleIndex * 9 + 6 ) + bounds.xMinimum();
    const double cY = -triangleData.at( triangleIndex * 9 + 8 ) + bounds.yMinimum();
    QgsGeometryUtils::weightedPointInTriangle( aX, aY, bX, bY, cX, cY, weightB, weightC, x, y );

    QgsPointXY candidate( x, y );
    if ( acceptPoint( candidate ) )
    {
      result << QgsPointXY( x, y );
      i++;
    }
  }
  return result;
}
