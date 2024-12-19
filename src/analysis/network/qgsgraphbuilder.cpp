/***************************************************************************
  qgsgraphbuilder.cpp
  --------------------------------------
  Date                 : 2010-10-25
  Copyright            : (C) 2010 by Yakushev Sergey
  Email                : YakushevS@list.ru
****************************************************************************
*                                                                          *
*   This program is free software; you can redistribute it and/or modify   *
*   it under the terms of the GNU General Public License as published by   *
*   the Free Software Foundation; either version 2 of the License, or      *
*   (at your option) any later version.                                    *
*                                                                          *
***************************************************************************/

/**
 * \file qgsgraphbuilder.cpp
 * \brief implementation of the QgsGraphBuilder
 */

#include "qgsgraphbuilder.h"
#include "qgsgraph.h"

#include "qgsgeometry.h"

QgsGraphBuilder::QgsGraphBuilder( const QgsCoordinateReferenceSystem &crs, bool otfEnabled, double topologyTolerance, const QString &ellipsoidID )
  : QgsGraphBuilderInterface( crs, otfEnabled, topologyTolerance, ellipsoidID )
  , mGraph( std::make_unique< QgsGraph >() )
{
}

QgsGraphBuilder::~QgsGraphBuilder() = default;

void QgsGraphBuilder::addVertex( int, const QgsPointXY &pt )
{
  mGraph->addVertex( pt );
}

void QgsGraphBuilder::addEdge( int pt1id, const QgsPointXY &, int pt2id, const QgsPointXY &, const QVector< QVariant > &prop )
{
  mGraph->addEdge( pt1id, pt2id, prop );
}

QgsGraph QgsGraphBuilder::graph() const
{
  return *mGraph;
}

QgsGraph *QgsGraphBuilder::takeGraph()
{
  QgsGraph *res = mGraph.release();

  // create a new graph in case this builder is used for additional work
  mGraph = std::make_unique< QgsGraph >();

  return res;
}
