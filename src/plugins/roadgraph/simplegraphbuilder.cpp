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
#include <qgsfeature.h>
#include <qgsgeometry.h>

RgSimpleGraphBuilder::RgSimpleGraphBuilder( const QgsCoordinateReferenceSystem& crs, bool ctfEnabled, double topologyTolerance ) :
    RgGraphBuilder( crs,  ctfEnabled, topologyTolerance )
{
}

QgsPoint RgSimpleGraphBuilder::addVertex( const QgsPoint& pt )
{
  double f = topologyTolerance();
  if ( f > 0 )
  {
    QgsRectangle r( pt.x() - f, pt.y() - f, pt.x() + f, pt.y() + f );
    QList< QgsFeatureId > searchResult = mPointIndex.intersects( r );
    if ( !searchResult.empty() )
    {
      QgsFeatureId i = searchResult.front();
      if ( mPointMap[ i ].sqrDist( pt ) < topologyTolerance() )
      {
        return mPointMap[ i ];
      }
    }
    QgsFeatureId newId = mPointMap.size() + 1;

    QgsFeature f( newId );
    f.setGeometry( QgsGeometry::fromPoint( pt ) );
    mPointIndex.insertFeature( f );
    mPointMap.insert( newId, pt );
  }

  mMatrix[ pt ];
  return pt;
}

void RgSimpleGraphBuilder::addArc( const QgsPoint& pt1, const QgsPoint& pt2, double cost, double speed, QgsFeatureId featureId )
{
  mMatrix[ pt1 ][ pt2 ] = ArcAttributes( cost, cost / speed, featureId );
}

AdjacencyMatrix RgSimpleGraphBuilder::adjacencyMatrix()
{
  return mMatrix;
}
