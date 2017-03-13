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

QgsGraph::QgsGraph()
{
}

int QgsGraph::addVertex( const QgsPoint &pt )
{
  mGraphVertexes.append( QgsGraphVertex( pt ) );
  return mGraphVertexes.size() - 1;
}

int QgsGraph::addEdge( int outVertexIdx, int inVertexIdx, const QVector< QVariant > &strategies )
{
  QgsGraphEdge e;

  e.mStrategies = strategies;
  e.mOut = outVertexIdx;
  e.mIn  = inVertexIdx;
  mGraphEdges.push_back( e );
  int edgeIdx = mGraphEdges.size() - 1;

  mGraphVertexes[ outVertexIdx ].mOutEdges.push_back( edgeIdx );
  mGraphVertexes[ inVertexIdx ].mInEdges.push_back( edgeIdx );

  return mGraphEdges.size() - 1;
}

const QgsGraphVertex &QgsGraph::vertex( int idx ) const
{
  return mGraphVertexes[ idx ];
}

const QgsGraphEdge &QgsGraph::edge( int idx ) const
{
  return mGraphEdges[ idx ];
}

int QgsGraph::vertexCount() const
{
  return mGraphVertexes.size();
}

int QgsGraph::edgeCount() const
{
  return mGraphEdges.size();
}

int QgsGraph::findVertex( const QgsPoint &pt ) const
{
  int i = 0;
  for ( i = 0; i < mGraphVertexes.size(); ++i )
  {
    if ( mGraphVertexes[ i ].point() == pt )
    {
      return i;
    }
  }
  return -1;
}

QgsGraphEdge::QgsGraphEdge()
  : mOut( 0 )
  , mIn( 0 )
{

}

QVariant QgsGraphEdge::cost( int i ) const
{
  return mStrategies[ i ];
}

QVector< QVariant > QgsGraphEdge::strategies() const
{
  return mStrategies;
}

int QgsGraphEdge::inVertex() const
{
  return mIn;
}

int QgsGraphEdge::outVertex() const
{
  return mOut;
}

QgsGraphVertex::QgsGraphVertex( const QgsPoint &point )
  : mCoordinate( point )
{

}

QgsGraphEdgeIds QgsGraphVertex::outEdges() const
{
  return mOutEdges;
}

QgsGraphEdgeIds QgsGraphVertex::inEdges() const
{
  return mInEdges;
}

QgsPoint QgsGraphVertex::point() const
{
  return mCoordinate;
}
