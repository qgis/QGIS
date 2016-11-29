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
#include <queue>

QgsInternalGeometryEngine::QgsInternalGeometryEngine( const QgsGeometry& geometry )
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
  QList<QgsLineString*> linesToProcess;

  const QgsMultiCurve* multiCurve = dynamic_cast< const QgsMultiCurve* >( mGeometry );
  if ( multiCurve )
  {
    for ( int i = 0; i < multiCurve->partCount(); ++i )
    {
      linesToProcess << static_cast<QgsLineString*>( multiCurve->geometryN( i )->clone() );
    }
  }

  const QgsCurve* curve = dynamic_cast< const QgsCurve* >( mGeometry );
  if ( curve )
  {
    linesToProcess << static_cast<QgsLineString*>( curve->segmentize() );
  }

  QgsMultiPolygonV2* multipolygon = linesToProcess.size() > 1 ? new QgsMultiPolygonV2() : nullptr;
  QgsPolygonV2* polygon = nullptr;

  if ( !linesToProcess.empty() )
  {
    Q_FOREACH ( QgsLineString* line, linesToProcess )
    {
      QTransform transform = QTransform::fromTranslate( x, y );

      QgsLineString* secondline = line->reversed();
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
      return QgsGeometry( multipolygon );
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
    Cell( double x, double y, double h, const QgsPolygonV2* polygon )
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
  bool operator()( const Cell* lhs, const Cell* rhs )
  {
    return rhs->max > lhs->max;
  }
};

Cell* getCentroidCell( const QgsPolygonV2* polygon )
{
  double area = 0;
  double x = 0;
  double y = 0;

  const QgsLineString* exterior = static_cast< const QgsLineString*>( polygon->exteriorRing() );
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

QgsGeometry QgsInternalGeometryEngine::poleOfInaccessibility( double precision , double* distanceFromBoundary ) const
{
  if ( distanceFromBoundary )
    *distanceFromBoundary = DBL_MAX;

  if ( !mGeometry || mGeometry->isEmpty() )
    return QgsGeometry();

  if ( precision <= 0 )
    return QgsGeometry();

  const QgsSurface* surface = dynamic_cast< const QgsSurface* >( mGeometry );
  if ( !surface )
    return QgsGeometry();

  QScopedPointer< QgsPolygonV2 > segmentizedPoly;
  const QgsPolygonV2* polygon = dynamic_cast< const QgsPolygonV2* >( mGeometry );
  if ( !polygon )
  {
    segmentizedPoly.reset( static_cast< QgsPolygonV2*>( surface->segmentize() ) );
    polygon = segmentizedPoly.data();
  }

  // start with the bounding box
  QgsRectangle bounds = polygon->boundingBox();

  // initial parameters
  double cellSize = qMin( bounds.width(), bounds.height() );

  if ( qgsDoubleNear( cellSize, 0.0 ) )
    return QgsGeometry( new QgsPointV2( bounds.xMinimum(), bounds.yMinimum() ) );

  double h = cellSize / 2.0;
  std::priority_queue< Cell*, std::vector<Cell*>, GreaterThanByMax > cellQueue;

  // cover polygon with initial cells
  for ( double x = bounds.xMinimum(); x < bounds.xMaximum(); x += cellSize )
  {
    for ( double y = bounds.yMinimum(); y < bounds.yMaximum(); y += cellSize )
    {
      cellQueue.push( new Cell( x + h, y + h, h, polygon ) );
    }
  }

  // take centroid as the first best guess
  QScopedPointer< Cell > bestCell( getCentroidCell( polygon ) );

  // special case for rectangular polygons
  QScopedPointer< Cell > bboxCell( new Cell( bounds.xMinimum() + bounds.width() / 2.0,
                                   bounds.yMinimum() + bounds.height() / 2.0,
                                   0, polygon ) );
  if ( bboxCell->d > bestCell->d )
  {
    bestCell.reset( bboxCell.take() );
  }

  while ( cellQueue.size() > 0 )
  {
    // pick the most promising cell from the queue
    QScopedPointer< Cell > cell( cellQueue.top() );
    cellQueue.pop();
    Cell* currentCell = cell.data();

    // update the best cell if we found a better one
    if ( currentCell->d > bestCell->d )
    {
      bestCell.reset( cell.take() );
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
