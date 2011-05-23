/***************************************************************************
 *   Copyright (C) 2011 by Sergey Yakushev                                 *
 *   yakushevs <at >list.ru                                                *
 *                                                                         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

/**
 * \file qgsgraph.cpp
 * \brief implementation QgsGraph, QgsGraphVertex, QgsGraphEdge
 */

#include "qgsgraph.h"

QgsGraph::QgsGraph()
{
}


QgsGraph::~QgsGraph()
{

}

int QgsGraph::addVertex( const QgsPoint& pt )
{
  mGraphVertexes.append( QgsGraphVertex( pt ) );
  return mGraphVertexes.size()-1;
}

int QgsGraph::addEdge( int outVertexIdx, int inVertexIdx, const QVector< QVariant >& properties )
{
  QgsGraphEdge e;

  e.mProperties = properties;
  e.mOut = outVertexIdx;
  e.mIn  = inVertexIdx;
  mGraphEdges.push_back( e );
  int edgeIdx = mGraphEdges.size()-1;

  mGraphVertexes[ outVertexIdx ].mOutEdges.push_back( edgeIdx );
  mGraphVertexes[ inVertexIdx ].mInEdges.push_back( edgeIdx );
   
  return mGraphEdges.size()-1;
}

const QgsGraphVertex& QgsGraph::vertex( int idx ) const
{
  return mGraphVertexes[ idx ];
}

const QgsGraphEdge& QgsGraph::edge( int idx ) const
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

int QgsGraph::findVertex( const QgsPoint& pt ) const
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
{
 
}

QVariant QgsGraphEdge::property(int i) const
{
  return mProperties[ i ];
}

QVector< QVariant > QgsGraphEdge::properties() const
{
  return mProperties;
}

int QgsGraphEdge::in() const
{
  return mIn;
}

int QgsGraphEdge::out() const
{
  return mOut;
}

QgsGraphVertex::QgsGraphVertex( const QgsPoint& point )
  : mCoordinate( point )
{

}

QgsGraphEdgeList QgsGraphVertex::outEdges() const
{
  return mOutEdges;
}

QgsGraphEdgeList QgsGraphVertex::inEdges() const
{
  return mInEdges;
}

QgsPoint QgsGraphVertex::point() const
{
  return mCoordinate;
}
