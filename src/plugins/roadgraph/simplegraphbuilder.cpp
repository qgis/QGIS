/***************************************************************************
 *   Copyright (C) 2010 by Sergey Yakushev                                 *
 *   yakushevs <at> list.ru                                                *
 *                                                                         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

/**
 * \file simplegraphbuilder.cpp
 * \brief implementation of RgSimpleGraphBuilder
 */

#include "simplegraphbuilder.h"
#include "utils.h"

// Qgis includes

RgSimpleGraphBuilder::RgSimpleGraphBuilder( const QgsCoordinateReferenceSystem& crs, double topologyTolerance ) :
    RgGraphBuilder( crs, topologyTolerance )
{
}

QgsPoint RgSimpleGraphBuilder::addVertex( const QgsPoint& pt )
{
  // I cann't use QgsSpatialIndex in this time.
  // QgsSpatialIndex::nearestNeighbor() return only features id not geometry
  //
  // This code is very slow and need me for a tests.

  double t = topologyTolerance();
  if ( t > 0.0 )
  {
    AdjacencyMatrix::iterator it;
    for ( it = mMatrix.begin(); it != mMatrix.end(); ++it )
    {
      if ( it->first.sqrDist( pt ) < t )
        return it->first;
    }
  }
  mMatrix[ pt ];
  return pt;
}

void RgSimpleGraphBuilder::addArc( const QgsPoint& pt1, const QgsPoint& pt2, double cost, double speed )
{
  mMatrix[ pt1 ][ pt2 ] = ArcAttributes( cost, cost / speed, 0 );
}

AdjacencyMatrix RgSimpleGraphBuilder::adjacencyMatrix()
{
  return mMatrix;
}
