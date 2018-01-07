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


#include <QTransform>
#include <memory>
#include <queue>

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
    *distanceFromBoundary = DBL_MAX;

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
      double dist = DBL_MAX;
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

    double dist = DBL_MAX;
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
  double minScore = DBL_MAX;

  bool isClosed = ring->isClosed();
  int numPoints = ring->numPoints();

  std::unique_ptr< QgsLineString > best( ring->clone() );

  for ( int it = 0; it < iterations; ++it )
  {
    QVector< QgsVector > /* yep */ motions;
    motions.reserve( numPoints );

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
    return doDensify( static_cast< QgsLineString * >( geom->clone() ), extraNodesPerSegment, distance );
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
