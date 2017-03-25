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
  : mGeometry( geometry.geometry() )
{

}

/***************************************************************************
 * This class is considered CRITICAL and any change MUST be accompanied with
 * full unit tests.
 * See details in QEP #17
 ****************************************************************************/

QgsGeometry QgsInternalGeometryEngine::extrude( double x, double y ) const
{
  QList<QgsLineString *> linesToProcess;

  const QgsMultiCurve *multiCurve = dynamic_cast< const QgsMultiCurve * >( mGeometry );
  if ( multiCurve )
  {
    for ( int i = 0; i < multiCurve->partCount(); ++i )
    {
      linesToProcess << static_cast<QgsLineString *>( multiCurve->geometryN( i )->clone() );
    }
  }

  const QgsCurve *curve = dynamic_cast< const QgsCurve * >( mGeometry );
  if ( curve )
  {
    linesToProcess << static_cast<QgsLineString *>( curve->segmentize() );
  }

  std::unique_ptr<QgsMultiPolygonV2> multipolygon( linesToProcess.size() > 1 ? new QgsMultiPolygonV2() : nullptr );
  QgsPolygonV2 *polygon = nullptr;

  if ( !linesToProcess.empty() )
  {
    Q_FOREACH ( QgsLineString *line, linesToProcess )
    {
      QTransform transform = QTransform::fromTranslate( x, y );

      QgsLineString *secondline = line->reversed();
      secondline->transform( transform );

      line->append( secondline );
      line->addVertex( line->pointN( 0 ) );

      polygon = new QgsPolygonV2();
      polygon->setExteriorRing( line );

      if ( multipolygon )
        multipolygon->addGeometry( polygon );

      delete secondline;
    }

    if ( multipolygon )
      return QgsGeometry( multipolygon.release() );
    else
      return QgsGeometry( polygon );
  }

  return QgsGeometry();
}



// polylabel implementation
// ported from the original Javascript implementation developed by Vladimir Agafonkin
// originally licensed under the ISC License

/// @cond PRIVATE
class Cell
{
  public:
    Cell( double x, double y, double h, const QgsPolygonV2 *polygon )
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

Cell *getCentroidCell( const QgsPolygonV2 *polygon )
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

///@endcond

QgsGeometry QgsInternalGeometryEngine::poleOfInaccessibility( double precision, double *distanceFromBoundary ) const
{
  if ( distanceFromBoundary )
    *distanceFromBoundary = DBL_MAX;

  if ( !mGeometry || mGeometry->isEmpty() )
    return QgsGeometry();

  if ( precision <= 0 )
    return QgsGeometry();

  const QgsSurface *surface = dynamic_cast< const QgsSurface * >( mGeometry );
  if ( !surface )
    return QgsGeometry();

  std::unique_ptr< QgsPolygonV2 > segmentizedPoly;
  const QgsPolygonV2 *polygon = dynamic_cast< const QgsPolygonV2 * >( mGeometry );
  if ( !polygon )
  {
    segmentizedPoly.reset( static_cast< QgsPolygonV2 *>( surface->segmentize() ) );
    polygon = segmentizedPoly.get();
  }

  // start with the bounding box
  QgsRectangle bounds = polygon->boundingBox();

  // initial parameters
  double cellSize = qMin( bounds.width(), bounds.height() );

  if ( qgsDoubleNear( cellSize, 0.0 ) )
    return QgsGeometry( new QgsPointV2( bounds.xMinimum(), bounds.yMinimum() ) );

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

  if ( distanceFromBoundary )
    *distanceFromBoundary = bestCell->d;

  return QgsGeometry( new QgsPointV2( bestCell->x, bestCell->y ) );
}


// helpers for orthogonalize
// adapted from original code in potlach/id osm editor

bool dotProductWithinAngleTolerance( double dotProduct, double lowerThreshold, double upperThreshold )
{
  return lowerThreshold > qAbs( dotProduct ) || qAbs( dotProduct ) > upperThreshold;
}

double normalizedDotProduct( const QgsPointV2 &a, const QgsPointV2 &b, const QgsPointV2 &c )
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
  QgsPointV2 a;
  QgsPointV2 b;
  QgsPointV2 c;

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

      sum += 2.0 * qMin( qAbs( dotProduct - 1.0 ), qMin( qAbs( dotProduct ), qAbs( dotProduct + 1 ) ) );
    }
    a = b;
    b = c;
  }

  return sum;
}

QgsVector calcMotion( const QgsPointV2 &a, const QgsPointV2 &b, const QgsPointV2 &c,
                      double lowerThreshold, double upperThreshold )
{
  QgsVector p = a - b;
  QgsVector q = c - b;

  if ( qgsDoubleNear( p.length(), 0.0 ) || qgsDoubleNear( q.length(), 0.0 ) )
    return QgsVector( 0, 0 );

  // 2.0 is a magic number from the original JOSM source code
  double scale = 2.0 * qMin( p.length(), q.length() );

  p = p.normalized();
  q = q.normalized();
  double dotProduct = p * q;

  if ( !dotProductWithinAngleTolerance( dotProduct, lowerThreshold, upperThreshold ) )
  {
    return QgsVector( 0, 0 );
  }

  // wonderful nasty hack which has survived through JOSM -> id -> QGIS
  // to deal with almost-straight segments (angle is closer to 180 than to 90/270).
  if ( dotProduct < -0.707106781186547 )
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
    QgsPointV2 a;
    QgsPointV2 b;
    QgsPointV2 c;
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
    const QgsPolygonV2 *polygon = static_cast< const QgsPolygonV2 * >( geom );
    QgsPolygonV2 *result = new QgsPolygonV2();

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

  double lowerThreshold = cos( ( 90 - angleThreshold ) * M_PI / 180.00 );
  double upperThreshold = cos( angleThreshold * M_PI / 180.0 );

  if ( const QgsGeometryCollection *gc = dynamic_cast< const QgsGeometryCollection *>( mGeometry ) )
  {
    int numGeom = gc->numGeometries();
    QList< QgsAbstractGeometry * > geometryList;
    geometryList.reserve( numGeom );
    for ( int i = 0; i < numGeom; ++i )
    {
      geometryList << orthogonalizeGeom( gc->geometryN( i ), maxIterations, tolerance, lowerThreshold, upperThreshold );
    }

    QgsGeometry first = QgsGeometry( geometryList.takeAt( 0 ) );
    Q_FOREACH ( QgsAbstractGeometry *g, geometryList )
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

QgsLineString *doDensifyByCount( QgsLineString *ring, int extraNodesPerSegment )
{
  QgsPointSequence out;
  double multiplier = 1.0 / double( extraNodesPerSegment + 1 );

  int nPoints = ring->numPoints();
  out.reserve( ( extraNodesPerSegment + 1 ) * nPoints );
  bool withZ = ring->is3D();
  bool withM = ring->isMeasure();
  QgsWkbTypes::Type outType = QgsWkbTypes::Point;
  if ( ring->is3D() )
    outType = QgsWkbTypes::addZ( outType );
  if ( ring->isMeasure() )
    outType = QgsWkbTypes::addM( outType );
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

    out << QgsPointV2( outType, x1, y1, z1, m1 );
    for ( int j = 0; j < extraNodesPerSegment; ++j )
    {
      double delta = multiplier * ( j + 1 );
      xOut = x1 + delta * ( x2 - x1 );
      yOut = y1 + delta * ( y2 - y1 );
      if ( withZ )
        zOut = z1 + delta * ( z2 - z1 );
      if ( withM )
        mOut = m1 + delta * ( m2 - m1 );

      out << QgsPointV2( outType, xOut, yOut, zOut, mOut );
    }
  }
  out << QgsPointV2( outType, ring->xAt( nPoints - 1 ), ring->yAt( nPoints - 1 ),
                     withZ ? ring->zAt( nPoints - 1 ) : 0, withM ? ring->mAt( nPoints - 1 ) : 0 );

  QgsLineString *result = new QgsLineString();
  result->setPoints( out );
  return result;
}

QgsAbstractGeometry *densifyGeometryByCount( const QgsAbstractGeometry *geom, int extraNodesPerSegment )
{
  std::unique_ptr< QgsAbstractGeometry > segmentizedCopy;
  if ( QgsWkbTypes::isCurvedType( geom->wkbType() ) )
  {
    segmentizedCopy.reset( geom->segmentize() );
    geom = segmentizedCopy.get();
  }

  if ( QgsWkbTypes::geometryType( geom->wkbType() ) == QgsWkbTypes::LineGeometry )
  {
    return doDensifyByCount( static_cast< QgsLineString * >( geom->clone() ), extraNodesPerSegment );
  }
  else
  {
    // polygon
    const QgsPolygonV2 *polygon = static_cast< const QgsPolygonV2 * >( geom );
    QgsPolygonV2 *result = new QgsPolygonV2();

    result->setExteriorRing( doDensifyByCount( static_cast< QgsLineString * >( polygon->exteriorRing()->clone() ),
                             extraNodesPerSegment ) );
    for ( int i = 0; i < polygon->numInteriorRings(); ++i )
    {
      result->addInteriorRing( doDensifyByCount( static_cast< QgsLineString * >( polygon->interiorRing( i )->clone() ),
                               extraNodesPerSegment ) );
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

  if ( const QgsGeometryCollection *gc = dynamic_cast< const QgsGeometryCollection *>( mGeometry ) )
  {
    int numGeom = gc->numGeometries();
    QList< QgsAbstractGeometry * > geometryList;
    geometryList.reserve( numGeom );
    for ( int i = 0; i < numGeom; ++i )
    {
      geometryList << densifyGeometryByCount( gc->geometryN( i ), extraNodesPerSegment );
    }

    QgsGeometry first = QgsGeometry( geometryList.takeAt( 0 ) );
    Q_FOREACH ( QgsAbstractGeometry *g, geometryList )
    {
      first.addPart( g );
    }
    return first;
  }
  else
  {
    return QgsGeometry( densifyGeometryByCount( mGeometry, extraNodesPerSegment ) );
  }
}
