/***************************************************************************
  qgsgraphbuilder.cpp
  --------------------------------------
  Date                 : 2010-10-25
  Copyright            : (C) 2010 by Yakushev Sergey
  Email                : YakushevS@list.ru
****************************************************************************
*                                                                          *
*    *
*   it under the terms of the GNU General Public License as published by   *
*         *
*                                      *
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
{
  mGraph = new QgsGraph();
}

QgsGraphBuilder::~QgsGraphBuilder()
{
  delete mGraph;
}

void QgsGraphBuilder::addVertex( int, const QgsPointXY &pt )
{
  mGraph->addVertex( pt );
}

void QgsGraphBuilder::addEdge( int pt1id, const QgsPointXY &, int pt2id, const QgsPointXY &, const QVector< QVariant > &prop )
{
  mGraph->addEdge( pt1id, pt2id, prop );
}

QgsGraph *QgsGraphBuilder::graph()
{
  QgsGraph *res = mGraph;
  mGraph = nullptr;
  return res;
}
