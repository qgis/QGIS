/***************************************************************************
  qgsgraph.cpp
  --------------------------------------
  Date                 : 2011-04-01
  Copyright            : (C) 2010 by Yakushev Sergey
  Email                : YakushevS <at> list.ru
****************************************************************************
*                                                                          *
*   This program is free software; you can redistribute it and/or modify   *
*   it under the terms of the GNU General Public License as published by   *
*   the Free Software Foundation; either version 2 of the License, or      *
*   (at your option) any later version.                                    *
*                                                                          *
***************************************************************************/

/**
 * \file qgsgraph.cpp
 * \brief implementation QgsGraph, QgsGraphVertex, QgsGraphEdge
 */

#include "qgsgraph.h"

int QgsGraph::addVertex( const QgsPointXY &pt )
{
  mGraphVertices.append( QgsGraphVertex( pt ) );
  return mGraphVertices.size() - 1;
}

int QgsGraph::addEdge( int fromVertexIdx, int toVertexIdx, const QVector< QVariant > &strategies )
{
  QgsGraphEdge e;

  e.mStrategies = strategies;
  e.mToIdx = toVertexIdx;
  e.mFromIdx  = fromVertexIdx;
  mGraphEdges.push_back( e );
  int edgeIdx = mGraphEdges.size() - 1;

  mGraphVertices[ toVertexIdx ].mIncomingEdges.push_back( edgeIdx );
  mGraphVertices[ fromVertexIdx ].mOutgoingEdges.push_back( edgeIdx );

  return mGraphEdges.size() - 1;
}

const QgsGraphVertex &QgsGraph::vertex( int idx ) const
{
  return mGraphVertices[ idx ];
}

const QgsGraphEdge &QgsGraph::edge( int idx ) const
{
  return mGraphEdges[ idx ];
}

int QgsGraph::vertexCount() const
{
  return mGraphVertices.size();
}

int QgsGraph::edgeCount() const
{
  return mGraphEdges.size();
}

int QgsGraph::findVertex( const QgsPointXY &pt ) const
{
  int i = 0;
  for ( i = 0; i < mGraphVertices.size(); ++i )
  {
    if ( mGraphVertices[ i ].point() == pt )
    {
      return i;
    }
  }
  return -1;
}

QVariant QgsGraphEdge::cost( int i ) const
{
  return mStrategies[ i ];
}

QVector< QVariant > QgsGraphEdge::strategies() const
{
  return mStrategies;
}

int QgsGraphEdge::fromVertex() const
{
  return mFromIdx;
}

int QgsGraphEdge::toVertex() const
{
  return mToIdx;
}

QgsGraphVertex::QgsGraphVertex( const QgsPointXY &point )
  : mCoordinate( point )
{

}

QgsGraphEdgeIds QgsGraphVertex::incomingEdges() const
{
  return mIncomingEdges;
}

QgsGraphEdgeIds QgsGraphVertex::outgoingEdges() const
{
  return mOutgoingEdges;
}

QgsPointXY QgsGraphVertex::point() const
{
  return mCoordinate;
}
