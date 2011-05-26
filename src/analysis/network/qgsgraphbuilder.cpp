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
 * \file qgsgraphbuilder.cpp
 * \brief implementation of QgsGraphBuilder
 */

#include "qgsgraphbuilder.h"
#include "qgsgraph.h"

// Qgis includes
#include <qgsfeature.h>
#include <qgsgeometry.h>

QgsGraphBuilder::QgsGraphBuilder( const QgsCoordinateReferenceSystem& crs, bool otfEnabled, double topologyTolerance, const QString& ellipsoidID ) :
    QgsGraphBuilderInterface( crs, otfEnabled, topologyTolerance, ellipsoidID )
{
  mGraph = new QgsGraph();
}

QgsGraphBuilder::~QgsGraphBuilder()
{
  if ( mGraph != NULL )
    delete mGraph;
}

QgsPoint QgsGraphBuilder::addVertex( const QgsPoint& pt )
{
  int id = pointId( pt );
  if ( id != -1 )
    return mGraph->vertex( id ).point();
   
  if ( topologyTolerance() > 0 )
  {
    int newId = mGraph->addVertex( pt );

    QgsFeature f( newId );
    f.setGeometry( QgsGeometry::fromPoint( pt ) );
    mPointIndex.insertFeature( f );
   
    return pt;
  }
  int newId = mGraph->addVertex( pt );
  
  mPointMap[ pt ] = newId;
  return pt;
}

void QgsGraphBuilder::addArc( const QgsPoint& pt1, const QgsPoint& pt2, const QVector< QVariant >& prop )
{
  int pt1_id = pointId( pt1 );
  int pt2_id = pointId( pt2 );
  if ( pt1_id == -1 )
  {
    // FIXME to QgsDebug
    std::cerr << "haven't vertex at (" << pt1.x() << ";" <<  pt1.y() << ")\n";
    return;
  }
  if ( pt2_id == -1 )
  {
    std::cerr << "haven't vertex at (" << pt2.x() << ";" <<  pt2.y() << ")\n";
    return;
  }
  mGraph->addEdge( pt1_id, pt2_id, prop );
}

QgsGraph* QgsGraphBuilder::graph()
{
  QgsGraph* res = mGraph;
  mGraph = NULL;
  return res;
}

int QgsGraphBuilder::pointId( const QgsPoint& pt )
{
  if ( topologyTolerance() > 0.0 )
  {
    QgsRectangle r( pt.x() - topologyTolerance(), pt.y() - topologyTolerance(), pt.x() + topologyTolerance(), pt.y() + topologyTolerance() );
    QList< int > searchResult = mPointIndex.intersects( r );
    if ( !searchResult.empty() )
    {
      return searchResult.front();
    }

  }else
  {
    std::map< QgsPoint, int, QgsPointCompare >::iterator it = mPointMap.find( pt );
    if ( it != mPointMap.end() )
    {
      return it->second;
    }
  }
  return -1;
}
