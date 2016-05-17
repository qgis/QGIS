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

QgsGraphBuilder::QgsGraphBuilder( const QgsCoordinateReferenceSystem& crs, bool otfEnabled, double topologyTolerance, const QString& ellipsoidID )
    : QgsGraphBuilderInterface( crs, otfEnabled, topologyTolerance, ellipsoidID )
{
  mGraph = new QgsGraph();
}

QgsGraphBuilder::~QgsGraphBuilder()
{
  delete mGraph;
}

void QgsGraphBuilder::addVertex( int, const QgsPoint& pt )
{
  mGraph->addVertex( pt );
}

void QgsGraphBuilder::addArc( int pt1id, const QgsPoint&, int pt2id, const QgsPoint&, const QVector< QVariant >& prop )
{
  mGraph->addArc( pt1id, pt2id, prop );
}

QgsGraph* QgsGraphBuilder::graph()
{
  QgsGraph* res = mGraph;
  mGraph = nullptr;
  return res;
}
