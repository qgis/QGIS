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
#include "qgscircularstring.h"
#include "qgsgeometry.h"
#include "qgsgeometryutils.h"
#include "qgslinesegment.h"
#include "qgscircle.h"
#include "qgslogger.h"
#include "qgstessellator.h"
#include "qgsfeedback.h"
#include "qgsgeometryengine.h"
#include "qgsmultilinestring.h"
#include <QTransform>
#include <functional>
#include <memory>
#include <queue>
#include <random>

QgsInternalGeometryEngine::QgsInternalGeometryEngine( const QgsGeometry &geometry )
  : mGeometry( geometry.constGet() )
{

}

QString QgsInternalGeometryEngine::lastError() const
{
  return mLastError;
}


enum class Direction
{
  Up,
  Right,
  Down,
  Left,
  None
};

/**
 * Determines the direction of an edge from p1 to p2. maxDev is the tangent of
 * the maximum allowed edge deviation angle. If the edge deviates more than
 * the allowed angle, Direction::None will be returned.
 */
Direction getEdgeDirection( const QgsPoint &p1, const QgsPoint &p2, double maxDev )
{
  double dx = p2.x() - p1.x();
  double dy = p2.y() - p1.y();
  if ( ( dx == 0.0 ) && ( dy == 0.0 ) )
    return Direction::None;
  if ( fabs( dx ) >= fabs( dy ) )
  {
    double dev = fabs( dy ) / fabs( dx );
    if ( dev > maxDev )
      return Direction::None;
    return dx > 0.0 ? Direction::Right : Direction::Left;
  }
  else
  {
    double dev = fabs( dx ) / fabs( dy );
    if ( dev > maxDev )
      return Direction::None;
    return dy > 0.0 ? Direction::Up : Direction::Down;
  }
}

/**
 * Checks whether the polygon consists of four nearly axis-parallel sides. All
 * consecutive edges having the same direction are considered to belong to the
 * same side.
 */
std::pair<bool, std::array<Direction, 4>> getEdgeDirections( const QgsPolygon *g, double maxDev )
{
  std::pair<bool, std::array<Direction, 4>> ret = { false, { Direction::None, Direction::None, Direction::None, Direction::None } };
  // The polygon might start in the middle of a side. Hence, we need a fifth
  // direction to record the beginning of the side when we went around the
  // polygon.
  std::array<Direction, 5> dirs;

  int idx = 0;
  QgsAbstractGeometry::vertex_iterator previous = g->vertices_begin();
  QgsAbstractGeometry::vertex_iterator current = previous;
  ++current;
  QgsAbstractGeometry::vertex_iterator end = g->vertices_end();
  while ( current != end )
  {
    Direction dir = getEdgeDirection( *previous, *current, maxDev );
    if ( dir == Direction::None )
      return ret;
    if ( idx == 0 )
    {
      dirs[0] = dir;
      ++idx;
    }
    else if ( dir != dirs[idx - 1] )
    {
      if ( idx == 5 )
        return ret;
      dirs[idx] = dir;
      ++idx;
    }
    previous = current;
    ++current;
  }
  ret.first = ( idx == 5 ) ? ( dirs[0] == dirs[4] ) : ( idx == 4 );
  std::copy( dirs.begin(), dirs.begin() + 4, ret.second.begin() );
  return ret;
}

bool matchesOrientation( std::array<Direction, 4> dirs, std::array<Direction, 4> oriented )
{
  int idx = std::find( oriented.begin(), oriented.end(), dirs[0] ) - oriented.begin();
  for ( int i = 1; i < 4; ++i )
  {
    if ( dirs[i] != oriented[( idx + i ) % 4] )
      return false;
  }
  return true;
}

/**
 * Checks whether the 4 directions in dirs make up a clockwise rectangle.
 */
bool isClockwise( std::array<Direction, 4> dirs )
{
  const std::array<Direction, 4> cwdirs = { Direction::Up, Direction::Right, Direction::Down, Direction::Left };
  return matchesOrientation( dirs, cwdirs );
}

/**
 * Checks whether the 4 directions in dirs make up a counter-clockwise
 * rectangle.
 */
bool isCounterClockwise( std::array<Direction, 4> dirs )
{
  const std::array<Direction, 4> ccwdirs = { Direction::Right, Direction::Up, Direction::Left, Direction::Down };
  return matchesOrientation( dirs, ccwdirs );
}


bool QgsInternalGeometryEngine::isAxisParallelRectangle( double maximumDeviation, bool simpleRectanglesOnly ) const
{
  if ( QgsWkbTypes::flatType( mGeometry->wkbType() ) != QgsWkbTypes::Polygon )
    return false;

  const QgsPolygon *polygon = qgsgeometry_cast< const QgsPolygon * >( mGeometry );
  if ( !polygon->exteriorRing() || polygon->numInteriorRings() > 0 )
    return false;

  const int vertexCount = polygon->exteriorRing()->numPoints();
  if ( vertexCount < 4 )
    return false;
  else if ( simpleRectanglesOnly && ( vertexCount != 5 || !polygon->exteriorRing()->isClosed() ) )
    return false;

  bool found4Dirs;
  std::array<Direction, 4> dirs;
  std::tie( found4Dirs, dirs ) = getEdgeDirections( polygon, std::tan( maximumDeviation * M_PI / 180 ) );

  return found4Dirs && ( isCounterClockwise( dirs ) || isClockwise( dirs ) );
}

/***************************************************************************
 * This class is considered CRITICAL and any change MUST be accompanied with
 * full unit tests.
 * See details in QEP #17
 ****************************************************************************/

QgsGeometry QgsInternalGeometryEngine::extrude( double x, double y ) const
{
  mLastError.clear();
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
    for ( QgsLineString *line : std::as_const( linesToProcess ) )
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
  mLastError.clear();
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
    for ( int i = 0; i < numGeom; ++i )
    {
      const QgsSurface *surface = qgsgeometry_cast< const QgsSurface * >( gc->geometryN( i ) );
      if ( !surface )
        continue;

      double dist = std::numeric_limits<double>::max();
      QgsPoint p = surfacePoleOfInaccessibility( surface, precision, dist );
      if ( dist > maxDist )
      {
        maxDist = dist;
        bestPoint = p;
      }
    }

    if ( bestPoint.isEmpty() )
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
    if ( p.isEmpty() )
      return QgsGeometry();

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
  mLastError.clear();
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
    for ( QgsAbstractGeometry *g : std::as_const( geometryList ) )
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
  mLastError.clear();
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
    for ( QgsAbstractGeometry *g : std::as_const( geometryList ) )
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
  mLastError.clear();
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
    for ( QgsAbstractGeometry *g : std::as_const( geometryList ) )
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
  mLastError.clear();
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

            std::unique_ptr< QgsPolygon > poly = std::make_unique< QgsPolygon >();
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
  mLastError.clear();
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
  mLastError.clear();
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
    const std::function< bool( const QgsPointXY & ) > &acceptPoint, unsigned long seed, QgsFeedback *feedback, int maxTriesPerPoint )
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
  int tries = 0;
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
      tries = 0;
    }
    else if ( maxTriesPerPoint != 0 )
    {
      tries++;
      // Skip this point if maximum tries is reached
      if ( tries == maxTriesPerPoint )
      {
        tries = 0;
        i++;
      }
    }
  }
  return result;
}

// ported from PostGIS' lwgeom pta_unstroke

std::unique_ptr< QgsCompoundCurve > lineToCurve( const QgsLineString *lineString, double distanceTolerance,
    double pointSpacingAngleTolerance )
{
  std::unique_ptr< QgsCompoundCurve > out = std::make_unique< QgsCompoundCurve >();

  /* Minimum number of edges, per quadrant, required to define an arc */
  const unsigned int minQuadEdges = 2;

  /* Die on null input */
  if ( !lineString )
    return nullptr;

  /* Null on empty input? */
  if ( lineString->nCoordinates() == 0 )
    return nullptr;

  /* We can't desegmentize anything shorter than four points */
  if ( lineString->nCoordinates() < 4 )
  {
    out->addCurve( lineString->clone() );
    return out;
  }

  /* Allocate our result array of vertices that are part of arcs */
  int numEdges = lineString->nCoordinates() - 1;
  QVector< int > edgesInArcs( numEdges + 1, 0 );

  auto arcAngle = []( const QgsPoint & a, const QgsPoint & b, const QgsPoint & c )->double
  {
    double abX = b.x() - a.x();
    double abY = b.y() - a.y();

    double cbX = b.x() - c.x();
    double cbY = b.y() - c.y();

    double dot = ( abX * cbX + abY * cbY ); /* dot product */
    double cross = ( abX * cbY - abY * cbX ); /* cross product */

    double alpha = std::atan2( cross, dot );

    return alpha;
  };

  /* We make a candidate arc of the first two edges, */
  /* And then see if the next edge follows it */
  int i = 0;
  int j = 0;
  int k = 0;
  int currentArc = 1;
  QgsPoint a1;
  QgsPoint a2;
  QgsPoint a3;
  QgsPoint b;
  double centerX = 0.0;
  double centerY = 0.0;
  double radius = 0;

  while ( i < numEdges - 2 )
  {
    unsigned int arcEdges = 0;
    double numQuadrants = 0;
    double angle;

    bool foundArc = false;
    /* Make candidate arc */
    a1 = lineString->pointN( i );
    a2 = lineString->pointN( i + 1 );
    a3 = lineString->pointN( i + 2 );
    QgsPoint first = a1;

    for ( j = i + 3; j < numEdges + 1; j++ )
    {
      b = lineString->pointN( j );

      /* Does this point fall on our candidate arc? */
      if ( QgsGeometryUtils::pointContinuesArc( a1, a2, a3, b, distanceTolerance, pointSpacingAngleTolerance ) )
      {
        /* Yes. Mark this edge and the two preceding it as arc components */
        foundArc = true;
        for ( k = j - 1; k > j - 4; k-- )
          edgesInArcs[k] = currentArc;
      }
      else
      {
        /* No. So we're done with this candidate arc */
        currentArc++;
        break;
      }

      a1 = a2;
      a2 = a3;
      a3 = b;
    }
    /* Jump past all the edges that were added to the arc */
    if ( foundArc )
    {
      /* Check if an arc was composed by enough edges to be
       * really considered an arc
       * See http://trac.osgeo.org/postgis/ticket/2420
       */
      arcEdges = j - 1 - i;
      if ( first.x() == b.x() && first.y() == b.y() )
      {
        numQuadrants = 4;
      }
      else
      {
        QgsGeometryUtils::circleCenterRadius( first, b, a1, radius, centerX, centerY );

        angle = arcAngle( first, QgsPoint( centerX, centerY ), b );
        int p2Side = QgsGeometryUtils::leftOfLine( b.x(), b.y(), first.x(), first.y(), a1.x(), a1.y() );
        if ( p2Side >= 0 )
          angle = -angle;

        if ( angle < 0 )
          angle = 2 * M_PI + angle;
        numQuadrants = ( 4 * angle ) / ( 2 * M_PI );
      }
      /* a1 is first point, b is last point */
      if ( arcEdges < minQuadEdges * numQuadrants )
      {
        // LWDEBUGF( 4, "Not enough edges for a %g quadrants arc, %g needed", num_quadrants, min_quad_edges * num_quadrants );
        for ( k = j - 1; k >= i; k-- )
          edgesInArcs[k] = 0;
      }

      i = j - 1;
    }
    else
    {
      /* Mark this edge as a linear edge */
      edgesInArcs[i] = 0;
      i = i + 1;
    }
  }

  int start = 0;
  int end = 0;
  /* non-zero if edge is part of an arc */
  int edgeType = edgesInArcs[0];

  auto addPointsToCurve = [ lineString, &out ]( int start, int end, int type )
  {
    if ( type == 0 )
    {
      // straight segment
      QVector< QgsPoint > points;
      for ( int j = start; j < end + 2; ++ j )
      {
        points.append( lineString->pointN( j ) );
      }
      std::unique_ptr< QgsCurve > straightSegment = std::make_unique< QgsLineString >( points );
      out->addCurve( straightSegment.release() );
    }
    else
    {
      // curved segment
      QVector< QgsPoint > points;
      points.append( lineString->pointN( start ) );
      points.append( lineString->pointN( ( start + end + 1 ) / 2 ) );
      points.append( lineString->pointN( end + 1 ) );
      std::unique_ptr< QgsCircularString > curvedSegment = std::make_unique< QgsCircularString >();
      curvedSegment->setPoints( points );
      out->addCurve( curvedSegment.release() );
    }
  };

  for ( int i = 1; i < numEdges; i++ )
  {
    if ( edgeType != edgesInArcs[i] )
    {
      end = i - 1;
      addPointsToCurve( start, end, edgeType );
      start = i;
      edgeType = edgesInArcs[i];
    }
  }

  /* Roll out last item */
  end = numEdges - 1;
  addPointsToCurve( start, end, edgeType );

  return out;
}

std::unique_ptr< QgsAbstractGeometry > convertGeometryToCurves( const QgsAbstractGeometry *geom, double distanceTolerance, double angleTolerance )
{
  if ( QgsWkbTypes::geometryType( geom->wkbType() ) == QgsWkbTypes::LineGeometry )
  {
    return lineToCurve( static_cast< const QgsLineString * >( geom ), distanceTolerance, angleTolerance );
  }
  else
  {
    // polygon
    const QgsPolygon *polygon = static_cast< const QgsPolygon * >( geom );
    std::unique_ptr< QgsCurvePolygon > result = std::make_unique< QgsCurvePolygon>();

    result->setExteriorRing( lineToCurve( static_cast< const QgsLineString * >( polygon->exteriorRing() ),
                                          distanceTolerance, angleTolerance ).release() );
    for ( int i = 0; i < polygon->numInteriorRings(); ++i )
    {
      result->addInteriorRing( lineToCurve( static_cast< const QgsLineString * >( polygon->interiorRing( i ) ),
                                            distanceTolerance, angleTolerance ).release() );
    }

    return result;
  }
}

QgsGeometry QgsInternalGeometryEngine::convertToCurves( double distanceTolerance, double angleTolerance ) const
{
  mLastError.clear();
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
      geometryList << convertGeometryToCurves( gc->geometryN( i ), distanceTolerance, angleTolerance ).release();
    }

    QgsGeometry first = QgsGeometry( geometryList.takeAt( 0 ) );
    for ( QgsAbstractGeometry *g : std::as_const( geometryList ) )
    {
      first.addPart( g );
    }
    return first;
  }
  else
  {
    return QgsGeometry( convertGeometryToCurves( mGeometry, distanceTolerance, angleTolerance ) );
  }
}

QgsGeometry QgsInternalGeometryEngine::orientedMinimumBoundingBox( double &area, double &angle, double &width, double &height ) const
{
  mLastError.clear();

  QgsRectangle minRect;
  area = std::numeric_limits<double>::max();
  angle = 0;
  width = std::numeric_limits<double>::max();
  height = std::numeric_limits<double>::max();

  if ( !mGeometry || mGeometry->nCoordinates() < 2 )
    return QgsGeometry();

  std::unique_ptr< QgsGeometryEngine >engine( QgsGeometry::createGeometryEngine( mGeometry ) );
  std::unique_ptr< QgsAbstractGeometry > hull( engine->convexHull( &mLastError ) );
  if ( !hull )
    return QgsGeometry();

  QgsVertexId vertexId;
  QgsPoint pt0;
  QgsPoint pt1;
  QgsPoint pt2;
  // get first point
  hull->nextVertex( vertexId, pt0 );
  pt1 = pt0;
  double totalRotation = 0;
  while ( hull->nextVertex( vertexId, pt2 ) )
  {
    double currentAngle = QgsGeometryUtils::lineAngle( pt1.x(), pt1.y(), pt2.x(), pt2.y() );
    double rotateAngle = 180.0 / M_PI * currentAngle;
    totalRotation += rotateAngle;

    QTransform t = QTransform::fromTranslate( pt0.x(), pt0.y() );
    t.rotate( rotateAngle );
    t.translate( -pt0.x(), -pt0.y() );

    hull->transform( t );

    QgsRectangle bounds = hull->boundingBox();
    double currentArea = bounds.width() * bounds.height();
    if ( currentArea  < area )
    {
      minRect = bounds;
      area = currentArea;
      angle = totalRotation;
      width = bounds.width();
      height = bounds.height();
    }

    pt1 = hull->vertexAt( vertexId );
  }

  if ( width > height )
  {
    width = minRect.height();
    height = minRect.width();
    angle = angle + 90.0;
  }

  QgsGeometry minBounds = QgsGeometry::fromRect( minRect );
  minBounds.rotate( angle, QgsPointXY( pt0.x(), pt0.y() ) );

  // constrain angle to 0 - 180
  if ( angle > 180.0 )
    angle = std::fmod( angle, 180.0 );

  return minBounds;
}

std::unique_ptr< QgsLineString > triangularWavesAlongLine( const QgsLineString *line, double wavelength, const double amplitude, const bool strictWavelength )
{
  const int totalPoints = line->numPoints();
  if ( totalPoints < 2 )
    return nullptr;

  const double *x = line->xData();
  const double *y = line->yData();

  double prevX = *x++;
  double prevY = *y++;

  QVector< double > outX;
  QVector< double > outY;
  const double totalLength = line->length();

  const int maxRepetitions = std::ceil( totalLength / wavelength );
  if ( !strictWavelength )
    wavelength = totalLength / maxRepetitions;

  const int estimatedPoints = maxRepetitions * 2 + 2;
  outX.reserve( estimatedPoints );
  outY.reserve( estimatedPoints );
  outX.append( prevX );
  outY.append( prevY );

  double distanceToNextPointFromStartOfSegment = wavelength / 4;
  int side = -1;
  for ( int i = 1; i < totalPoints; ++i )
  {
    double thisX = *x++;
    double thisY = *y++;

    const double segmentAngleRadians = QgsGeometryUtils::lineAngle( prevX, prevY, thisX, thisY );
    const double segmentLength = std::sqrt( ( thisX - prevX ) * ( thisX - prevX ) + ( thisY - prevY ) * ( thisY - prevY ) );
    while ( distanceToNextPointFromStartOfSegment < segmentLength || qgsDoubleNear( distanceToNextPointFromStartOfSegment, segmentLength ) )
    {
      // point falls on this segment - truncate to segment length if qgsDoubleNear test was actually > segment length
      const double distanceToPoint = std::min( distanceToNextPointFromStartOfSegment, segmentLength );
      double pX, pY;
      QgsGeometryUtils::pointOnLineWithDistance( prevX, prevY, thisX, thisY, distanceToPoint, pX, pY );

      // project point on line out by amplitude
      const double outPointX = pX + side * amplitude * std::sin( segmentAngleRadians + M_PI_2 );
      const double outPointY = pY + side * amplitude * std::cos( segmentAngleRadians + M_PI_2 );

      outX.append( outPointX );
      outY.append( outPointY );

      distanceToNextPointFromStartOfSegment += wavelength / 2;
      side = -side;
    }

    prevX = thisX;
    prevY = thisY;
    distanceToNextPointFromStartOfSegment -= segmentLength;
  }

  outX.append( prevX );
  outY.append( prevY );

  return std::make_unique< QgsLineString >( outX, outY );
}

std::unique_ptr< QgsLineString > triangularWavesRandomizedAlongLine( const QgsLineString *line,
    const double minimumWavelength, const double maximumWavelength,
    const double minimumAmplitude, const double maximumAmplitude,
    std::uniform_real_distribution<> &uniformDist, std::mt19937 &mt )
{
  const int totalPoints = line->numPoints();
  if ( totalPoints < 2 )
    return nullptr;

  const double *x = line->xData();
  const double *y = line->yData();

  double prevX = *x++;
  double prevY = *y++;

  QVector< double > outX;
  QVector< double > outY;
  const double totalLength = line->length();

  const int maxRepetitions = std::ceil( totalLength / minimumWavelength );

  const int estimatedPoints = maxRepetitions * 2 + 2;
  outX.reserve( estimatedPoints );
  outY.reserve( estimatedPoints );
  outX.append( prevX );
  outY.append( prevY );

  double wavelength = uniformDist( mt ) * ( maximumWavelength - minimumWavelength ) + minimumWavelength;
  double distanceToNextPointFromStartOfSegment = wavelength / 4;
  double amplitude = uniformDist( mt ) * ( maximumAmplitude - minimumAmplitude ) + minimumAmplitude;

  int side = -1;
  for ( int i = 1; i < totalPoints; ++i )
  {
    double thisX = *x++;
    double thisY = *y++;

    const double segmentAngleRadians = QgsGeometryUtils::lineAngle( prevX, prevY, thisX, thisY );
    const double segmentLength = std::sqrt( ( thisX - prevX ) * ( thisX - prevX ) + ( thisY - prevY ) * ( thisY - prevY ) );
    while ( distanceToNextPointFromStartOfSegment < segmentLength || qgsDoubleNear( distanceToNextPointFromStartOfSegment, segmentLength ) )
    {
      // point falls on this segment - truncate to segment length if qgsDoubleNear test was actually > segment length
      const double distanceToPoint = std::min( distanceToNextPointFromStartOfSegment, segmentLength );
      double pX, pY;
      QgsGeometryUtils::pointOnLineWithDistance( prevX, prevY, thisX, thisY, distanceToPoint, pX, pY );

      // project point on line out by amplitude
      const double outPointX = pX + side * amplitude * std::sin( segmentAngleRadians + M_PI_2 );
      const double outPointY = pY + side * amplitude * std::cos( segmentAngleRadians + M_PI_2 );
      outX.append( outPointX );
      outY.append( outPointY );

      wavelength = uniformDist( mt ) * ( maximumWavelength - minimumWavelength ) + minimumWavelength;
      amplitude = uniformDist( mt ) * ( maximumAmplitude - minimumAmplitude ) + minimumAmplitude;

      distanceToNextPointFromStartOfSegment += wavelength / 2;
      side = -side;
    }

    prevX = thisX;
    prevY = thisY;
    distanceToNextPointFromStartOfSegment -= segmentLength;
  }

  outX.append( prevX );
  outY.append( prevY );

  return std::make_unique< QgsLineString >( outX, outY );
}

std::unique_ptr< QgsAbstractGeometry > triangularWavesPrivate( const QgsAbstractGeometry *geom, double wavelength, double amplitude, bool strictWavelength )
{
  std::unique_ptr< QgsAbstractGeometry > segmentizedCopy;
  if ( QgsWkbTypes::isCurvedType( geom->wkbType() ) )
  {
    segmentizedCopy.reset( geom->segmentize() );
    geom = segmentizedCopy.get();
  }

  if ( QgsWkbTypes::geometryType( geom->wkbType() ) == QgsWkbTypes::LineGeometry )
  {
    return triangularWavesAlongLine( static_cast< const QgsLineString * >( geom ), wavelength, amplitude, strictWavelength );
  }
  else
  {
    // polygon
    const QgsPolygon *polygon = static_cast< const QgsPolygon * >( geom );
    std::unique_ptr< QgsPolygon > result = std::make_unique< QgsPolygon >();

    result->setExteriorRing( triangularWavesAlongLine( static_cast< const QgsLineString * >( polygon->exteriorRing() ),
                             wavelength, amplitude, strictWavelength ).release() );
    for ( int i = 0; i < polygon->numInteriorRings(); ++i )
    {
      result->addInteriorRing( triangularWavesAlongLine( static_cast< const QgsLineString * >( polygon->interiorRing( i ) ),
                               wavelength, amplitude, strictWavelength ).release() );
    }

    return result;
  }
}

std::unique_ptr< QgsAbstractGeometry > triangularWavesRandomizedPrivate( const QgsAbstractGeometry *geom, double minimumWavelength, double maximumWavelength, double minimumAmplitude, double maximumAmplitude, std::uniform_real_distribution<> &uniformDist, std::mt19937 &mt )
{
  std::unique_ptr< QgsAbstractGeometry > segmentizedCopy;
  if ( QgsWkbTypes::isCurvedType( geom->wkbType() ) )
  {
    segmentizedCopy.reset( geom->segmentize() );
    geom = segmentizedCopy.get();
  }

  if ( QgsWkbTypes::geometryType( geom->wkbType() ) == QgsWkbTypes::LineGeometry )
  {
    return triangularWavesRandomizedAlongLine( static_cast< const QgsLineString * >( geom ), minimumWavelength, maximumWavelength, minimumAmplitude, maximumAmplitude, uniformDist, mt );
  }
  else
  {
    // polygon
    const QgsPolygon *polygon = static_cast< const QgsPolygon * >( geom );
    std::unique_ptr< QgsPolygon > result = std::make_unique< QgsPolygon >();

    result->setExteriorRing( triangularWavesRandomizedAlongLine( static_cast< const QgsLineString * >( polygon->exteriorRing() ),
                             minimumWavelength, maximumWavelength, minimumAmplitude, maximumAmplitude, uniformDist, mt ).release() );
    for ( int i = 0; i < polygon->numInteriorRings(); ++i )
    {
      result->addInteriorRing( triangularWavesRandomizedAlongLine( static_cast< const QgsLineString * >( polygon->interiorRing( i ) ),
                               minimumWavelength, maximumWavelength, minimumAmplitude, maximumAmplitude, uniformDist, mt ).release() );
    }

    return result;
  }
}

QgsGeometry QgsInternalGeometryEngine::triangularWaves( double wavelength, double amplitude, bool strictWavelength ) const
{
  if ( wavelength < 0 || qgsDoubleNear( wavelength, 0 ) )
    return QgsGeometry();

  mLastError.clear();
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
      geometryList << triangularWavesPrivate( gc->geometryN( i ), wavelength, amplitude, strictWavelength ).release();
    }

    QgsGeometry first = QgsGeometry( geometryList.takeAt( 0 ) );
    for ( QgsAbstractGeometry *g : std::as_const( geometryList ) )
    {
      first.addPart( g );
    }
    return first;
  }
  else
  {
    return QgsGeometry( triangularWavesPrivate( mGeometry, wavelength, amplitude, strictWavelength ) );
  }
}

QgsGeometry QgsInternalGeometryEngine::triangularWavesRandomized( double minimumWavelength, double maximumWavelength, double minimumAmplitude, double maximumAmplitude, unsigned long seed ) const
{
  if ( minimumWavelength < 0 || qgsDoubleNear( minimumWavelength, 0 ) || maximumWavelength < 0 || qgsDoubleNear( maximumWavelength, 0 ) || maximumWavelength < minimumWavelength )
    return QgsGeometry();

  mLastError.clear();
  if ( !mGeometry )
  {
    return QgsGeometry();
  }

  std::random_device rd;
  std::mt19937 mt( seed == 0 ? rd() : seed );
  std::uniform_real_distribution<> uniformDist( 0, 1 );

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
      geometryList << triangularWavesRandomizedPrivate( gc->geometryN( i ), minimumWavelength, maximumWavelength, minimumAmplitude, maximumAmplitude, uniformDist, mt ).release();
    }

    QgsGeometry first = QgsGeometry( geometryList.takeAt( 0 ) );
    for ( QgsAbstractGeometry *g : std::as_const( geometryList ) )
    {
      first.addPart( g );
    }
    return first;
  }
  else
  {
    return QgsGeometry( triangularWavesRandomizedPrivate( mGeometry, minimumWavelength, maximumWavelength, minimumAmplitude, maximumAmplitude, uniformDist, mt ) );
  }
}

std::unique_ptr< QgsLineString > squareWavesAlongLine( const QgsLineString *line, double wavelength, const double amplitude, const bool strictWavelength )
{
  const int totalPoints = line->numPoints();
  if ( totalPoints < 2 )
    return nullptr;

  const double *x = line->xData();
  const double *y = line->yData();

  double prevX = *x++;
  double prevY = *y++;

  QVector< double > outX;
  QVector< double > outY;
  const double totalLength = line->length();

  const int maxRepetitions = std::ceil( totalLength / wavelength );
  if ( !strictWavelength )
    wavelength = totalLength / maxRepetitions;

  const int estimatedPoints = maxRepetitions * 4 + 2;
  outX.reserve( estimatedPoints );
  outY.reserve( estimatedPoints );
  outX.append( prevX );
  outY.append( prevY );

  const double startSegmentAngleRadians = QgsGeometryUtils::lineAngle( prevX, prevY, *x, *y );
  outX.append( prevX - amplitude * std::sin( startSegmentAngleRadians + M_PI_2 ) );
  outY.append( prevY - amplitude * std::cos( startSegmentAngleRadians + M_PI_2 ) );

  double distanceToNextPointFromStartOfSegment = wavelength / 2;

  int side = -1;
  for ( int i = 1; i < totalPoints; ++i )
  {
    double thisX = *x++;
    double thisY = *y++;

    const double segmentAngleRadians = QgsGeometryUtils::lineAngle( prevX, prevY, thisX, thisY );
    const double segmentLength = std::sqrt( ( thisX - prevX ) * ( thisX - prevX ) + ( thisY - prevY ) * ( thisY - prevY ) );
    while ( distanceToNextPointFromStartOfSegment < segmentLength || qgsDoubleNear( distanceToNextPointFromStartOfSegment, segmentLength ) )
    {
      // point falls on this segment - truncate to segment length if qgsDoubleNear test was actually > segment length
      const double distanceToPoint = std::min( distanceToNextPointFromStartOfSegment, segmentLength );
      double pX, pY;
      QgsGeometryUtils::pointOnLineWithDistance( prevX, prevY, thisX, thisY, distanceToPoint, pX, pY );

      // project point on line out by amplitude
      const double sinAngle = std::sin( segmentAngleRadians + M_PI_2 );
      const double cosAngle = std::cos( segmentAngleRadians + M_PI_2 );
      outX.append( pX + side * amplitude * sinAngle );
      outY.append( pY + side * amplitude * cosAngle );
      outX.append( pX - side * amplitude * sinAngle );
      outY.append( pY - side * amplitude * cosAngle );

      distanceToNextPointFromStartOfSegment += wavelength / 2;
      side = -side;
    }

    prevX = thisX;
    prevY = thisY;
    distanceToNextPointFromStartOfSegment -= segmentLength;
  }

  // replace last point, which will be projected out to amplitude of wave, with the actual end point of the line
  outX.pop_back();
  outY.pop_back();
  outX.append( prevX );
  outY.append( prevY );

  return std::make_unique< QgsLineString >( outX, outY );
}

std::unique_ptr< QgsLineString > squareWavesRandomizedAlongLine( const QgsLineString *line,
    const double minimumWavelength, const double maximumWavelength,
    const double minimumAmplitude, const double maximumAmplitude,
    std::uniform_real_distribution<> &uniformDist, std::mt19937 &mt )
{
  const int totalPoints = line->numPoints();
  if ( totalPoints < 2 )
    return nullptr;

  const double *x = line->xData();
  const double *y = line->yData();

  double prevX = *x++;
  double prevY = *y++;

  QVector< double > outX;
  QVector< double > outY;
  const double totalLength = line->length();

  const int maxRepetitions = std::ceil( totalLength / minimumWavelength );

  const int estimatedPoints = maxRepetitions * 4 + 2;
  outX.reserve( estimatedPoints );
  outY.reserve( estimatedPoints );
  outX.append( prevX );
  outY.append( prevY );

  double amplitude = uniformDist( mt ) * ( maximumAmplitude - minimumAmplitude ) + minimumAmplitude;

  double segmentAngleRadians = QgsGeometryUtils::lineAngle( prevX, prevY, *x, *y );
  outX.append( prevX - amplitude * std::sin( segmentAngleRadians + M_PI_2 ) );
  outY.append( prevY - amplitude * std::cos( segmentAngleRadians + M_PI_2 ) );

  double wavelength = uniformDist( mt ) * ( maximumWavelength - minimumWavelength ) + minimumWavelength;
  double distanceToNextPointFromStartOfSegment = wavelength / 2;

  int side = -1;
  for ( int i = 1; i < totalPoints; ++i )
  {
    double thisX = *x++;
    double thisY = *y++;

    segmentAngleRadians = QgsGeometryUtils::lineAngle( prevX, prevY, thisX, thisY );
    const double segmentLength = std::sqrt( ( thisX - prevX ) * ( thisX - prevX ) + ( thisY - prevY ) * ( thisY - prevY ) );
    while ( distanceToNextPointFromStartOfSegment < segmentLength || qgsDoubleNear( distanceToNextPointFromStartOfSegment, segmentLength ) )
    {
      // point falls on this segment - truncate to segment length if qgsDoubleNear test was actually > segment length
      const double distanceToPoint = std::min( distanceToNextPointFromStartOfSegment, segmentLength );
      double pX, pY;
      QgsGeometryUtils::pointOnLineWithDistance( prevX, prevY, thisX, thisY, distanceToPoint, pX, pY );

      // project point on line out by amplitude
      const double sinAngle = std::sin( segmentAngleRadians + M_PI_2 );
      const double cosAngle = std::cos( segmentAngleRadians + M_PI_2 );
      outX.append( pX + side * amplitude * sinAngle );
      outY.append( pY + side * amplitude * cosAngle );

      amplitude = uniformDist( mt ) * ( maximumAmplitude - minimumAmplitude ) + minimumAmplitude;
      outX.append( pX - side * amplitude * sinAngle );
      outY.append( pY - side * amplitude * cosAngle );

      wavelength = uniformDist( mt ) * ( maximumWavelength - minimumWavelength ) + minimumWavelength;
      distanceToNextPointFromStartOfSegment += wavelength / 2;
      side = -side;
    }

    prevX = thisX;
    prevY = thisY;
    distanceToNextPointFromStartOfSegment -= segmentLength;
  }

  outX.append( prevX + side * amplitude * std::sin( segmentAngleRadians + M_PI_2 ) );
  outY.append( prevY + side * amplitude * std::cos( segmentAngleRadians + M_PI_2 ) );
  outX.append( prevX );
  outY.append( prevY );

  return std::make_unique< QgsLineString >( outX, outY );
}

std::unique_ptr< QgsAbstractGeometry > squareWavesPrivate( const QgsAbstractGeometry *geom, double wavelength, double amplitude, bool strictWavelength )
{
  std::unique_ptr< QgsAbstractGeometry > segmentizedCopy;
  if ( QgsWkbTypes::isCurvedType( geom->wkbType() ) )
  {
    segmentizedCopy.reset( geom->segmentize() );
    geom = segmentizedCopy.get();
  }

  if ( QgsWkbTypes::geometryType( geom->wkbType() ) == QgsWkbTypes::LineGeometry )
  {
    return squareWavesAlongLine( static_cast< const QgsLineString * >( geom ), wavelength, amplitude, strictWavelength );
  }
  else
  {
    // polygon
    const QgsPolygon *polygon = static_cast< const QgsPolygon * >( geom );
    std::unique_ptr< QgsPolygon > result = std::make_unique< QgsPolygon >();

    result->setExteriorRing( squareWavesAlongLine( static_cast< const QgsLineString * >( polygon->exteriorRing() ),
                             wavelength, amplitude, strictWavelength ).release() );
    for ( int i = 0; i < polygon->numInteriorRings(); ++i )
    {
      result->addInteriorRing( squareWavesAlongLine( static_cast< const QgsLineString * >( polygon->interiorRing( i ) ),
                               wavelength, amplitude, strictWavelength ).release() );
    }

    return result;
  }
}

std::unique_ptr< QgsAbstractGeometry > squareWavesRandomizedPrivate( const QgsAbstractGeometry *geom, double minimumWavelength, double maximumWavelength, double minimumAmplitude, double maximumAmplitude, std::uniform_real_distribution<> &uniformDist, std::mt19937 &mt )
{
  std::unique_ptr< QgsAbstractGeometry > segmentizedCopy;
  if ( QgsWkbTypes::isCurvedType( geom->wkbType() ) )
  {
    segmentizedCopy.reset( geom->segmentize() );
    geom = segmentizedCopy.get();
  }

  if ( QgsWkbTypes::geometryType( geom->wkbType() ) == QgsWkbTypes::LineGeometry )
  {
    return squareWavesRandomizedAlongLine( static_cast< const QgsLineString * >( geom ), minimumWavelength, maximumWavelength, minimumAmplitude, maximumAmplitude, uniformDist, mt );
  }
  else
  {
    // polygon
    const QgsPolygon *polygon = static_cast< const QgsPolygon * >( geom );
    std::unique_ptr< QgsPolygon > result = std::make_unique< QgsPolygon >();

    result->setExteriorRing( squareWavesRandomizedAlongLine( static_cast< const QgsLineString * >( polygon->exteriorRing() ),
                             minimumWavelength, maximumWavelength, minimumAmplitude, maximumAmplitude, uniformDist, mt ).release() );
    for ( int i = 0; i < polygon->numInteriorRings(); ++i )
    {
      result->addInteriorRing( squareWavesRandomizedAlongLine( static_cast< const QgsLineString * >( polygon->interiorRing( i ) ),
                               minimumWavelength, maximumWavelength, minimumAmplitude, maximumAmplitude, uniformDist, mt ).release() );
    }

    return result;
  }
}

QgsGeometry QgsInternalGeometryEngine::squareWaves( double wavelength, double amplitude, bool strictWavelength ) const
{
  if ( wavelength < 0 || qgsDoubleNear( wavelength, 0 ) )
    return QgsGeometry();

  mLastError.clear();
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
      geometryList << squareWavesPrivate( gc->geometryN( i ), wavelength, amplitude, strictWavelength ).release();
    }

    QgsGeometry first = QgsGeometry( geometryList.takeAt( 0 ) );
    for ( QgsAbstractGeometry *g : std::as_const( geometryList ) )
    {
      first.addPart( g );
    }
    return first;
  }
  else
  {
    return QgsGeometry( squareWavesPrivate( mGeometry, wavelength, amplitude, strictWavelength ) );
  }
}

QgsGeometry QgsInternalGeometryEngine::squareWavesRandomized( double minimumWavelength, double maximumWavelength, double minimumAmplitude, double maximumAmplitude, unsigned long seed ) const
{
  if ( minimumWavelength < 0 || qgsDoubleNear( minimumWavelength, 0 ) || maximumWavelength < 0 || qgsDoubleNear( maximumWavelength, 0 ) || maximumWavelength < minimumWavelength )
    return QgsGeometry();

  mLastError.clear();
  if ( !mGeometry )
  {
    return QgsGeometry();
  }

  std::random_device rd;
  std::mt19937 mt( seed == 0 ? rd() : seed );
  std::uniform_real_distribution<> uniformDist( 0, 1 );

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
      geometryList << squareWavesRandomizedPrivate( gc->geometryN( i ), minimumWavelength, maximumWavelength, minimumAmplitude, maximumAmplitude, uniformDist, mt ).release();
    }

    QgsGeometry first = QgsGeometry( geometryList.takeAt( 0 ) );
    for ( QgsAbstractGeometry *g : std::as_const( geometryList ) )
    {
      first.addPart( g );
    }
    return first;
  }
  else
  {
    return QgsGeometry( squareWavesRandomizedPrivate( mGeometry, minimumWavelength, maximumWavelength, minimumAmplitude, maximumAmplitude, uniformDist, mt ) );
  }
}

std::unique_ptr< QgsLineString > roundWavesAlongLine( const QgsLineString *line, double wavelength, const double amplitude, const bool strictWavelength )
{
  const int totalPoints = line->numPoints();
  if ( totalPoints < 2 )
    return nullptr;

  const double *x = line->xData();
  const double *y = line->yData();

  double prevX = *x++;
  double prevY = *y++;

  const double totalLength = line->length();

  const int maxRepetitions = std::ceil( totalLength / wavelength );
  if ( !strictWavelength )
    wavelength = totalLength / maxRepetitions;

  const int segments = 10;

  int side = -1;

  double xOutBuffer[4] { prevX, prevX, prevX, prevX };
  double yOutBuffer[4] { prevY, prevY, prevY, prevY };
  bool isFirstPart = true;

  double distanceToNextPointFromStartOfSegment = wavelength / 8;
  int bufferIndex = 1;
  std::unique_ptr< QgsLineString > out = std::make_unique< QgsLineString >();

  double segmentAngleRadians = 0;
  double remainingDistance = totalLength;
  double totalCoveredDistance = 0;

  for ( int i = 1; i < totalPoints; ++i )
  {
    double thisX = *x++;
    double thisY = *y++;

    segmentAngleRadians = QgsGeometryUtils::lineAngle( prevX, prevY, thisX, thisY );
    const double segmentLength = std::sqrt( ( thisX - prevX ) * ( thisX - prevX ) + ( thisY - prevY ) * ( thisY - prevY ) );
    while ( distanceToNextPointFromStartOfSegment < segmentLength || qgsDoubleNear( distanceToNextPointFromStartOfSegment, segmentLength ) )
    {
      // point falls on this segment - truncate to segment length if qgsDoubleNear test was actually > segment length
      const double distanceToPoint = std::min( distanceToNextPointFromStartOfSegment, segmentLength );
      double pX, pY;
      QgsGeometryUtils::pointOnLineWithDistance( prevX, prevY, thisX, thisY, distanceToPoint, pX, pY );
      remainingDistance = totalLength - totalCoveredDistance - distanceToPoint;

      const double sinAngle = std::sin( segmentAngleRadians + M_PI_2 );
      const double cosAngle = std::cos( segmentAngleRadians + M_PI_2 );

      if ( bufferIndex == 0 )
      {
        xOutBuffer[0] = pX + side * amplitude * sinAngle;
        yOutBuffer[0] = pY + side * amplitude * cosAngle;
        bufferIndex = 1;
        distanceToNextPointFromStartOfSegment += wavelength / 4;
      }
      else if ( bufferIndex == 1 && isFirstPart )
      {
        xOutBuffer[1] = ( xOutBuffer[0] + pX - side * amplitude * sinAngle ) * 0.5;
        yOutBuffer[1] = ( yOutBuffer[0] + pY - side * amplitude * cosAngle ) * 0.5;
        xOutBuffer[2] = pX - side * amplitude * sinAngle;
        yOutBuffer[2] = pY - side * amplitude * cosAngle;
        bufferIndex = 2;
        distanceToNextPointFromStartOfSegment += wavelength / 8;
      }
      else if ( bufferIndex == 1 )
      {
        xOutBuffer[1] = pX + side * amplitude * sinAngle;
        yOutBuffer[1] = pY + side * amplitude * cosAngle;
        xOutBuffer[2] = pX - side * amplitude * sinAngle;
        yOutBuffer[2] = pY - side * amplitude * cosAngle;
        bufferIndex = 2;
        distanceToNextPointFromStartOfSegment += wavelength / 4;
      }
      else if ( bufferIndex == 2 )
      {
        xOutBuffer[3] = pX - side * amplitude * sinAngle;
        yOutBuffer[3] = pY - side * amplitude * cosAngle;

        if ( isFirstPart )
        {
          xOutBuffer[2] = ( xOutBuffer[2] + xOutBuffer[3] ) * 0.5;
          yOutBuffer[2] = ( yOutBuffer[2] + yOutBuffer[3] ) * 0.5;
          isFirstPart = false;
        }

        // flush curve
        std::unique_ptr< QgsLineString > bezier( QgsLineString::fromBezierCurve( QgsPoint( xOutBuffer[0], yOutBuffer[0] ),
            QgsPoint( xOutBuffer[1], yOutBuffer[1] ),
            QgsPoint( xOutBuffer[2], yOutBuffer[2] ),
            QgsPoint( xOutBuffer[3], yOutBuffer[3] ),
            segments ) );
        out->append( bezier.get() );

        // shuffle buffer alone
        xOutBuffer[0] = xOutBuffer[3];
        yOutBuffer[0] = yOutBuffer[3];
        bufferIndex = 1;
        side = -side;
        distanceToNextPointFromStartOfSegment += wavelength / 4;
      }
    }
    totalCoveredDistance += segmentLength;
    prevX = thisX;
    prevY = thisY;
    distanceToNextPointFromStartOfSegment -= segmentLength;
  }

  const double midX = prevX - remainingDistance / 2 * std::sin( segmentAngleRadians );
  const double midY = prevY - remainingDistance / 2 * std::cos( segmentAngleRadians );
  const double pX = midX + side * amplitude * std::sin( segmentAngleRadians + M_PI_2 );
  const double pY = midY + side * amplitude * std::cos( segmentAngleRadians + M_PI_2 );

  std::unique_ptr< QgsLineString > bezier( QgsLineString::fromBezierCurve( out->endPoint(),
      QgsPoint( ( out->endPoint().x() + pX ) * 0.5, ( out->endPoint().y() + pY ) * 0.5 ),
      QgsPoint( ( prevX + pX ) * 0.5, ( prevY + pY ) * 0.5 ),
      QgsPoint( prevX, prevY ),
      segments / 2 ) );
  out->append( bezier.get() );

  return out;
}

std::unique_ptr< QgsLineString > roundWavesRandomizedAlongLine( const QgsLineString *line,
    const double minimumWavelength, const double maximumWavelength,
    const double minimumAmplitude, const double maximumAmplitude,
    std::uniform_real_distribution<> &uniformDist, std::mt19937 &mt )
{
  const int totalPoints = line->numPoints();
  if ( totalPoints < 2 )
    return nullptr;

  const double *x = line->xData();
  const double *y = line->yData();

  double prevX = *x++;
  double prevY = *y++;

  const double totalLength = line->length();

  const int segments = 10;

  int side = -1;

  double xOutBuffer[4] { prevX, prevX, prevX, prevX };
  double yOutBuffer[4] { prevY, prevY, prevY, prevY };
  bool isFirstPart = true;

  double amplitude = uniformDist( mt ) * ( maximumAmplitude - minimumAmplitude ) + minimumAmplitude;
  double wavelength = uniformDist( mt ) * ( maximumWavelength - minimumWavelength ) + minimumWavelength;

  double distanceToNextPointFromStartOfSegment = wavelength / 8;
  int bufferIndex = 1;
  std::unique_ptr< QgsLineString > out = std::make_unique< QgsLineString >();

  double segmentAngleRadians = 0;

  double remainingDistance = totalLength;
  double totalCoveredDistance = 0;

  for ( int i = 1; i < totalPoints; ++i )
  {
    double thisX = *x++;
    double thisY = *y++;

    segmentAngleRadians = QgsGeometryUtils::lineAngle( prevX, prevY, thisX, thisY );
    const double segmentLength = std::sqrt( ( thisX - prevX ) * ( thisX - prevX ) + ( thisY - prevY ) * ( thisY - prevY ) );
    while ( distanceToNextPointFromStartOfSegment < segmentLength || qgsDoubleNear( distanceToNextPointFromStartOfSegment, segmentLength ) )
    {
      // point falls on this segment - truncate to segment length if qgsDoubleNear test was actually > segment length
      const double distanceToPoint = std::min( distanceToNextPointFromStartOfSegment, segmentLength );
      double pX, pY;
      QgsGeometryUtils::pointOnLineWithDistance( prevX, prevY, thisX, thisY, distanceToPoint, pX, pY );
      remainingDistance = totalLength - totalCoveredDistance - distanceToPoint;

      const double sinAngle = std::sin( segmentAngleRadians + M_PI_2 );
      const double cosAngle = std::cos( segmentAngleRadians + M_PI_2 );

      if ( bufferIndex == 0 )
      {
        xOutBuffer[0] = pX + side * amplitude * sinAngle;
        yOutBuffer[0] = pY + side * amplitude * cosAngle;
        bufferIndex = 1;
        distanceToNextPointFromStartOfSegment += wavelength / 4;
      }
      else if ( bufferIndex == 1 && isFirstPart )
      {
        xOutBuffer[1] = ( xOutBuffer[0] + pX - side * amplitude * sinAngle ) * 0.5;
        yOutBuffer[1] = ( yOutBuffer[0] + pY - side * amplitude * cosAngle ) * 0.5;
        xOutBuffer[2] = pX - side * amplitude * sinAngle;
        yOutBuffer[2] = pY - side * amplitude * cosAngle;
        bufferIndex = 2;
        distanceToNextPointFromStartOfSegment += wavelength / 8;
      }
      else if ( bufferIndex == 1 )
      {
        xOutBuffer[1] = pX + side * amplitude * sinAngle;
        yOutBuffer[1] = pY + side * amplitude * cosAngle;
        amplitude = uniformDist( mt ) * ( maximumAmplitude - minimumAmplitude ) + minimumAmplitude;
        xOutBuffer[2] = pX - side * amplitude * sinAngle;
        bufferIndex = 2;
        yOutBuffer[2] = pY - side * amplitude * cosAngle;
        distanceToNextPointFromStartOfSegment += wavelength / 4;
      }
      else if ( bufferIndex == 2 )
      {
        xOutBuffer[3] = pX - side * amplitude * sinAngle;
        yOutBuffer[3] = pY - side * amplitude * cosAngle;

        if ( isFirstPart )
        {
          xOutBuffer[2] = ( xOutBuffer[2] + xOutBuffer[3] ) * 0.5;
          yOutBuffer[2] = ( yOutBuffer[2] + yOutBuffer[3] ) * 0.5;
          isFirstPart = false;
        }

        // flush curve
        std::unique_ptr< QgsLineString > bezier( QgsLineString::fromBezierCurve( QgsPoint( xOutBuffer[0], yOutBuffer[0] ),
            QgsPoint( xOutBuffer[1], yOutBuffer[1] ),
            QgsPoint( xOutBuffer[2], yOutBuffer[2] ),
            QgsPoint( xOutBuffer[3], yOutBuffer[3] ),
            segments ) );
        out->append( bezier.get() );

        // shuffle buffer alone
        xOutBuffer[0] = xOutBuffer[3];
        yOutBuffer[0] = yOutBuffer[3];
        bufferIndex = 1;
        side = -side;

        wavelength = uniformDist( mt ) * ( maximumWavelength - minimumWavelength ) + minimumWavelength;

        distanceToNextPointFromStartOfSegment += wavelength / 4;
      }
    }
    totalCoveredDistance += segmentLength;

    prevX = thisX;
    prevY = thisY;
    distanceToNextPointFromStartOfSegment -= segmentLength;
  }

  const double midX = prevX - remainingDistance / 2 * std::sin( segmentAngleRadians );
  const double midY = prevY - remainingDistance / 2 * std::cos( segmentAngleRadians );
  const double pX = midX + side * amplitude * std::sin( segmentAngleRadians + M_PI_2 );
  const double pY = midY + side * amplitude * std::cos( segmentAngleRadians + M_PI_2 );

  if ( out->isEmpty() )
  {
    std::unique_ptr< QgsLineString > bezier( QgsLineString::fromBezierCurve( line->startPoint(),
        QgsPoint( ( line->startPoint().x() + pX ) * 0.5, ( line->startPoint().y() + pY ) * 0.5 ),
        QgsPoint( ( prevX + pX ) * 0.5, ( prevY + pY ) * 0.5 ),
        QgsPoint( prevX, prevY ),
        segments ) );
    out->append( bezier.get() );
  }
  else
  {
    std::unique_ptr< QgsLineString > bezier( QgsLineString::fromBezierCurve( out->endPoint(),
        QgsPoint( ( out->endPoint().x() + pX ) * 0.5, ( out->endPoint().y() + pY ) * 0.5 ),
        QgsPoint( ( prevX + pX ) * 0.5, ( prevY + pY ) * 0.5 ),
        QgsPoint( prevX, prevY ),
        segments ) );
    out->append( bezier.get() );
  }

  return out;
}

std::unique_ptr< QgsAbstractGeometry > roundWavesPrivate( const QgsAbstractGeometry *geom, double wavelength, double amplitude, bool strictWavelength )
{
  std::unique_ptr< QgsAbstractGeometry > segmentizedCopy;
  if ( QgsWkbTypes::isCurvedType( geom->wkbType() ) )
  {
    segmentizedCopy.reset( geom->segmentize() );
    geom = segmentizedCopy.get();
  }

  if ( QgsWkbTypes::geometryType( geom->wkbType() ) == QgsWkbTypes::LineGeometry )
  {
    return roundWavesAlongLine( static_cast< const QgsLineString * >( geom ), wavelength, amplitude, strictWavelength );
  }
  else
  {
    // polygon
    const QgsPolygon *polygon = static_cast< const QgsPolygon * >( geom );
    std::unique_ptr< QgsPolygon > result = std::make_unique< QgsPolygon >();

    result->setExteriorRing( roundWavesAlongLine( static_cast< const QgsLineString * >( polygon->exteriorRing() ),
                             wavelength, amplitude, strictWavelength ).release() );
    for ( int i = 0; i < polygon->numInteriorRings(); ++i )
    {
      result->addInteriorRing( roundWavesAlongLine( static_cast< const QgsLineString * >( polygon->interiorRing( i ) ),
                               wavelength, amplitude, strictWavelength ).release() );
    }

    return result;
  }
}

std::unique_ptr< QgsAbstractGeometry > roundWavesRandomizedPrivate( const QgsAbstractGeometry *geom, double minimumWavelength, double maximumWavelength, double minimumAmplitude, double maximumAmplitude, std::uniform_real_distribution<> &uniformDist, std::mt19937 &mt )
{
  std::unique_ptr< QgsAbstractGeometry > segmentizedCopy;
  if ( QgsWkbTypes::isCurvedType( geom->wkbType() ) )
  {
    segmentizedCopy.reset( geom->segmentize() );
    geom = segmentizedCopy.get();
  }

  if ( QgsWkbTypes::geometryType( geom->wkbType() ) == QgsWkbTypes::LineGeometry )
  {
    return roundWavesRandomizedAlongLine( static_cast< const QgsLineString * >( geom ), minimumWavelength, maximumWavelength, minimumAmplitude, maximumAmplitude, uniformDist, mt );
  }
  else
  {
    // polygon
    const QgsPolygon *polygon = static_cast< const QgsPolygon * >( geom );
    std::unique_ptr< QgsPolygon > result = std::make_unique< QgsPolygon >();

    result->setExteriorRing( roundWavesRandomizedAlongLine( static_cast< const QgsLineString * >( polygon->exteriorRing() ),
                             minimumWavelength, maximumWavelength, minimumAmplitude, maximumAmplitude, uniformDist, mt ).release() );
    for ( int i = 0; i < polygon->numInteriorRings(); ++i )
    {
      result->addInteriorRing( roundWavesRandomizedAlongLine( static_cast< const QgsLineString * >( polygon->interiorRing( i ) ),
                               minimumWavelength, maximumWavelength, minimumAmplitude, maximumAmplitude, uniformDist, mt ).release() );
    }

    return result;
  }
}

QgsGeometry QgsInternalGeometryEngine::roundWaves( double wavelength, double amplitude, bool strictWavelength ) const
{
  if ( wavelength < 0 || qgsDoubleNear( wavelength, 0 ) )
    return QgsGeometry();

  mLastError.clear();
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
      geometryList << roundWavesPrivate( gc->geometryN( i ), wavelength, amplitude, strictWavelength ).release();
    }

    QgsGeometry first = QgsGeometry( geometryList.takeAt( 0 ) );
    for ( QgsAbstractGeometry *g : std::as_const( geometryList ) )
    {
      first.addPart( g );
    }
    return first;
  }
  else
  {
    return QgsGeometry( roundWavesPrivate( mGeometry, wavelength, amplitude, strictWavelength ) );
  }
}

QgsGeometry QgsInternalGeometryEngine::roundWavesRandomized( double minimumWavelength, double maximumWavelength, double minimumAmplitude, double maximumAmplitude, unsigned long seed ) const
{
  if ( minimumWavelength < 0 || qgsDoubleNear( minimumWavelength, 0 ) || maximumWavelength < 0 || qgsDoubleNear( maximumWavelength, 0 ) || maximumWavelength < minimumWavelength )
    return QgsGeometry();

  mLastError.clear();
  if ( !mGeometry )
  {
    return QgsGeometry();
  }

  std::random_device rd;
  std::mt19937 mt( seed == 0 ? rd() : seed );
  std::uniform_real_distribution<> uniformDist( 0, 1 );

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
      geometryList << roundWavesRandomizedPrivate( gc->geometryN( i ), minimumWavelength, maximumWavelength, minimumAmplitude, maximumAmplitude, uniformDist, mt ).release();
    }

    QgsGeometry first = QgsGeometry( geometryList.takeAt( 0 ) );
    for ( QgsAbstractGeometry *g : std::as_const( geometryList ) )
    {
      first.addPart( g );
    }
    return first;
  }
  else
  {
    return QgsGeometry( roundWavesRandomizedPrivate( mGeometry, minimumWavelength, maximumWavelength, minimumAmplitude, maximumAmplitude, uniformDist, mt ) );
  }
}

std::unique_ptr< QgsMultiLineString > dashPatternAlongLine( const QgsLineString *line,
    const QVector< double> &pattern,
    Qgis::DashPatternLineEndingRule startRule,
    Qgis::DashPatternLineEndingRule endRule,
    Qgis::DashPatternSizeAdjustment adjustment,
    double patternOffset )
{
  const int totalPoints = line->numPoints();
  if ( totalPoints < 2 )
    return nullptr;

  const int patternSize = pattern.size();

  const double *x = line->xData();
  const double *y = line->yData();

  double prevX = *x++;
  double prevY = *y++;

  std::unique_ptr< QgsMultiLineString > result = std::make_unique< QgsMultiLineString >();

  QVector< double > outX;
  QVector< double > outY;
  const double totalLength = line->length();

  double patternLength = 0;
  double patternDashLength = 0;
  double patternGapLength = 0;
  for ( int i = 0; i < pattern.size(); ++i )
  {
    patternLength += pattern.at( i );
    if ( i % 2 == 0 )
      patternDashLength += pattern.at( i );
    else
      patternGapLength += pattern.at( i );
  }

  double firstPatternLength = 0;
  double firstPatternDashLength = 0;
  double firstPatternGapLength = 0;
  switch ( startRule )
  {
    case Qgis::DashPatternLineEndingRule::NoRule:
    case Qgis::DashPatternLineEndingRule::FullDash:
      firstPatternLength = patternLength;
      firstPatternDashLength = patternDashLength;
      firstPatternGapLength = patternGapLength;
      break;
    case Qgis::DashPatternLineEndingRule::HalfDash:
      firstPatternLength = patternLength - pattern.at( 0 ) * 0.5; // remove half of first dash
      firstPatternDashLength = patternDashLength - pattern.at( 0 ) * 0.5;
      firstPatternGapLength = patternGapLength;
      break;
    case Qgis::DashPatternLineEndingRule::FullGap:
      firstPatternLength = pattern.at( patternSize - 1 ); // start at LAST dash
      firstPatternDashLength = 0;
      firstPatternGapLength = pattern.at( patternSize - 1 );
      break;
    case Qgis::DashPatternLineEndingRule::HalfGap:
      firstPatternLength = pattern.at( patternSize - 1 ) * 0.5; // start at half of last dash
      firstPatternDashLength = 0;
      firstPatternGapLength = pattern.at( patternSize - 1 ) * 0.5;
      break;
  }

  const bool isSmallEnoughForSinglePattern = ( totalLength - firstPatternLength ) < patternLength * 0.5;

  double lastPatternLength = isSmallEnoughForSinglePattern ? firstPatternLength : patternLength;
  double lastPatternDashLength = isSmallEnoughForSinglePattern ? firstPatternDashLength : patternDashLength;
  double lastPatternGapLength = isSmallEnoughForSinglePattern ? firstPatternGapLength : patternGapLength;
  switch ( endRule )
  {
    case Qgis::DashPatternLineEndingRule::NoRule:
      lastPatternLength = 0;
      lastPatternDashLength = 0;
      lastPatternGapLength = 0;
      break;
    case Qgis::DashPatternLineEndingRule::FullDash:
      lastPatternLength -= pattern.at( patternSize - 1 ); // remove last gap
      lastPatternGapLength -= pattern.at( patternSize - 1 );
      break;
    case Qgis::DashPatternLineEndingRule::HalfDash:
      lastPatternLength -= pattern.at( patternSize - 1 ) + pattern.at( patternSize - 2 ) * 0.5; // remove last gap, half of last dash
      lastPatternDashLength -= pattern.at( patternSize - 2 ) * 0.5;
      lastPatternGapLength -= pattern.at( patternSize - 1 );
      break;
    case Qgis::DashPatternLineEndingRule::FullGap:
      lastPatternGapLength = patternGapLength;
      break;
    case Qgis::DashPatternLineEndingRule::HalfGap:
      lastPatternLength -= pattern.at( patternSize - 1 ) * 0.5; // remove half of last gap
      lastPatternGapLength -= pattern.at( patternSize - 1 ) * 0.5;
      break;
  }

  const double remainingLengthForCompletePatterns = totalLength - ( !isSmallEnoughForSinglePattern ? firstPatternLength : 0 ) - lastPatternLength;
  const int middlePatternRepetitions = std::max( static_cast< int >( std::round( remainingLengthForCompletePatterns / patternLength ) ), 0 );

  const double totalUnscaledLengthOfPatterns = ( !isSmallEnoughForSinglePattern ? firstPatternLength : 0 ) + middlePatternRepetitions * patternLength + lastPatternLength;
  const double totalUnscaledLengthOfDashes = ( !isSmallEnoughForSinglePattern ? firstPatternDashLength : 0 ) + middlePatternRepetitions * patternDashLength + lastPatternDashLength;
  const double totalUnscaledLengthOfGaps = ( !isSmallEnoughForSinglePattern ? firstPatternGapLength : 0 ) + middlePatternRepetitions * patternGapLength + lastPatternGapLength;

  double dashLengthScalingFactor = 1;
  double gapLengthScalingFactor = 1;
  if ( endRule != Qgis::DashPatternLineEndingRule::NoRule )
  {
    // calculate scaling factors
    const double lengthToShrinkBy = totalUnscaledLengthOfPatterns - totalLength;

    switch ( adjustment )
    {
      case Qgis::DashPatternSizeAdjustment::ScaleBothDashAndGap:
        dashLengthScalingFactor = totalLength / totalUnscaledLengthOfPatterns;
        gapLengthScalingFactor = dashLengthScalingFactor;
        break;
      case Qgis::DashPatternSizeAdjustment::ScaleDashOnly:
        dashLengthScalingFactor = ( totalUnscaledLengthOfDashes - lengthToShrinkBy ) / totalUnscaledLengthOfDashes;
        break;
      case Qgis::DashPatternSizeAdjustment::ScaleGapOnly:
        gapLengthScalingFactor = ( totalUnscaledLengthOfGaps - lengthToShrinkBy ) / totalUnscaledLengthOfGaps;
        break;
    }
  }

  dashLengthScalingFactor = std::max( dashLengthScalingFactor, 0.0 );
  gapLengthScalingFactor = std::max( gapLengthScalingFactor, 0.0 );

  const int maxPatterns = middlePatternRepetitions + 2;
  result->reserve( maxPatterns );

  int patternIndex = 0;
  double distanceToNextPointFromStartOfSegment = pattern.at( 0 ) * dashLengthScalingFactor;
  bool isDash = true;
  switch ( startRule )
  {
    case Qgis::DashPatternLineEndingRule::NoRule:
    case Qgis::DashPatternLineEndingRule::FullDash:
      break;
    case Qgis::DashPatternLineEndingRule::HalfDash:
      distanceToNextPointFromStartOfSegment *= 0.5; // skip to half way through first dash
      break;
    case Qgis::DashPatternLineEndingRule::FullGap:
      patternIndex = patternSize - 1; // start at last gap
      isDash = false;
      distanceToNextPointFromStartOfSegment = pattern.at( patternSize - 1 ) * gapLengthScalingFactor;
      break;
    case Qgis::DashPatternLineEndingRule::HalfGap:
      patternIndex = patternSize - 1; // skip straight to half way through last gap
      isDash = false;
      distanceToNextPointFromStartOfSegment = 0.5 * pattern.at( patternSize - 1 ) * gapLengthScalingFactor;
      break;
  }

  const double adjustedOffset = fmod( patternOffset, patternLength );
  const double scaledOffset = ( adjustedOffset < 0 ? ( adjustedOffset + patternLength ) : adjustedOffset ) * ( gapLengthScalingFactor + dashLengthScalingFactor ) / 2;
  if ( !qgsDoubleNear( scaledOffset, 0 ) )
  {
    // shuffle pattern along by offset
    double remainingOffset = scaledOffset;
    while ( remainingOffset > 0 )
    {
      if ( distanceToNextPointFromStartOfSegment > remainingOffset )
      {
        distanceToNextPointFromStartOfSegment -= remainingOffset;
        break;
      }

      remainingOffset -= distanceToNextPointFromStartOfSegment;
      isDash = !isDash;
      patternIndex++;
      if ( patternIndex == patternSize )
        patternIndex = 0;

      distanceToNextPointFromStartOfSegment = pattern.at( patternIndex ) * ( isDash ? dashLengthScalingFactor : gapLengthScalingFactor );
    }
  }

  if ( isDash )
  {
    outX.append( prevX );
    outY.append( prevY );
  }

  for ( int i = 1; i < totalPoints; ++i )
  {
    double thisX = *x++;
    double thisY = *y++;

    const double segmentLength = std::sqrt( ( thisX - prevX ) * ( thisX - prevX ) + ( thisY - prevY ) * ( thisY - prevY ) );
    while ( distanceToNextPointFromStartOfSegment < segmentLength || qgsDoubleNear( distanceToNextPointFromStartOfSegment, segmentLength ) )
    {
      // point falls on this segment - truncate to segment length if qgsDoubleNear test was actually > segment length
      const double distanceToPoint = std::min( distanceToNextPointFromStartOfSegment, segmentLength );
      double pX, pY;
      QgsGeometryUtils::pointOnLineWithDistance( prevX, prevY, thisX, thisY, distanceToPoint, pX, pY );

      outX.append( pX );
      outY.append( pY );
      if ( isDash )
      {
        result->addGeometry( new QgsLineString( outX, outY ) );
        outX.resize( 0 );
        outY.resize( 0 );
      }

      isDash = !isDash;
      patternIndex++;
      if ( patternIndex >= patternSize )
        patternIndex = 0;

      distanceToNextPointFromStartOfSegment += pattern.at( patternIndex ) * ( isDash ? dashLengthScalingFactor : gapLengthScalingFactor );
    }

    if ( isDash )
    {
      outX.append( thisX );
      outY.append( thisY );
    }

    prevX = thisX;
    prevY = thisY;
    distanceToNextPointFromStartOfSegment -= segmentLength;
  }

  if ( isDash )
  {
    outX.append( prevX );
    outY.append( prevY );
    result->addGeometry( new QgsLineString( outX, outY ) );
  }

  return result;
}

std::unique_ptr< QgsAbstractGeometry > applyDashPatternPrivate( const QgsAbstractGeometry *geom,
    const QVector<double> &pattern,
    Qgis::DashPatternLineEndingRule startRule,
    Qgis::DashPatternLineEndingRule endRule,
    Qgis::DashPatternSizeAdjustment adjustment,
    double patternOffset )
{
  std::unique_ptr< QgsAbstractGeometry > segmentizedCopy;
  if ( QgsWkbTypes::isCurvedType( geom->wkbType() ) )
  {
    segmentizedCopy.reset( geom->segmentize() );
    geom = segmentizedCopy.get();
  }

  if ( QgsWkbTypes::geometryType( geom->wkbType() ) == QgsWkbTypes::LineGeometry )
  {
    return dashPatternAlongLine( static_cast< const QgsLineString * >( geom ), pattern, startRule, endRule, adjustment, patternOffset );
  }
  else
  {
    // polygon
    const QgsPolygon *polygon = static_cast< const QgsPolygon * >( geom );
    std::unique_ptr< QgsMultiLineString > result = std::make_unique< QgsMultiLineString >();

    std::unique_ptr< QgsMultiLineString > exteriorParts = dashPatternAlongLine( static_cast< const QgsLineString * >( polygon->exteriorRing() ), pattern, startRule, endRule, adjustment, patternOffset );
    for ( int i = 0; i < exteriorParts->numGeometries(); ++i )
      result->addGeometry( exteriorParts->geometryN( i )->clone() );

    for ( int i = 0; i < polygon->numInteriorRings(); ++i )
    {
      std::unique_ptr< QgsMultiLineString > ringParts = dashPatternAlongLine( static_cast< const QgsLineString * >( polygon->interiorRing( i ) ), pattern, startRule, endRule, adjustment, patternOffset );
      for ( int j = 0; j < ringParts->numGeometries(); ++j )
        result->addGeometry( ringParts->geometryN( j )->clone() );
    }

    return result;
  }
}

QgsGeometry QgsInternalGeometryEngine::applyDashPattern( const QVector<double> &pattern, Qgis::DashPatternLineEndingRule startRule, Qgis::DashPatternLineEndingRule endRule, Qgis::DashPatternSizeAdjustment adjustment, double patternOffset ) const
{
  if ( pattern.size() < 2 )
    return QgsGeometry( mGeometry->clone() );

  mLastError.clear();
  if ( !mGeometry || mGeometry->isEmpty() )
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
      geometryList << applyDashPatternPrivate( gc->geometryN( i ), pattern, startRule, endRule, adjustment, patternOffset ).release();
    }

    QgsGeometry first = QgsGeometry( geometryList.takeAt( 0 ) );
    for ( QgsAbstractGeometry *g : std::as_const( geometryList ) )
    {
      if ( const QgsGeometryCollection *collection = qgsgeometry_cast< QgsGeometryCollection * >( g ) )
      {
        for ( int j = 0; j < collection->numGeometries(); ++j )
        {
          first.addPart( collection->geometryN( j )->clone() );
        }
        delete collection;
      }
      else
      {
        first.addPart( g );
      }
    }
    return first;
  }
  else
  {
    return QgsGeometry( applyDashPatternPrivate( mGeometry, pattern, startRule, endRule, adjustment, patternOffset ) );
  }
}
