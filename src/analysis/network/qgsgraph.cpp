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
#include <QSet>

int QgsGraph::addVertex( const QgsPointXY &pt )
{
  mGraphVertices[ mNextVertexId ] = QgsGraphVertex( pt );
  return mNextVertexId++;
}

int QgsGraph::addEdge( int fromVertexIdx, int toVertexIdx, const QVector< QVariant > &strategies )
{
  QgsGraphEdge e;

  e.mStrategies = strategies;
  e.mToIdx = toVertexIdx;
  e.mFromIdx  = fromVertexIdx;

  mGraphEdges[ mNextEdgeId ] = e;
  const int edgeIdx = mGraphEdges.size() - 1;

  mGraphVertices[ toVertexIdx ].mIncomingEdges.push_back( edgeIdx );
  mGraphVertices[ fromVertexIdx ].mOutgoingEdges.push_back( edgeIdx );

  return mNextEdgeId++;
}

const QgsGraphVertex &QgsGraph::vertex( int idx ) const
{
  auto it = mGraphVertices.constFind( idx );
  if ( it != mGraphVertices.constEnd() )
    return ( it ).value();
  Q_ASSERT_X( false, "QgsGraph::vertex()", "Invalid vertex ID" );

  // unreachable...
  return ( *const_cast< QHash<int, QgsGraphVertex>* >( &mGraphVertices ) )[ idx ];
}

void QgsGraph::removeVertex( int index )
{
  auto it = mGraphVertices.constFind( index );
  if ( it != mGraphVertices.constEnd() )
  {
    QSet< int > affectedEdges = qgis::listToSet( it->incomingEdges() );
    affectedEdges.unite( qgis::listToSet( it->outgoingEdges() ) );

    mGraphVertices.erase( it );

    // remove affected edges
    for ( int edgeId : std::as_const( affectedEdges ) )
    {
      mGraphEdges.remove( edgeId );
    }
  }
}

const QgsGraphEdge &QgsGraph::edge( int idx ) const
{
  auto it = mGraphEdges.constFind( idx );
  if ( it != mGraphEdges.constEnd() )
    return ( it ).value();
  Q_ASSERT_X( false, "QgsGraph::edge()", "Invalid edge ID" );

  // unreachable...
  return ( *const_cast< QHash<int, QgsGraphEdge>* >( &mGraphEdges ) )[ idx ];
}

void QgsGraph::removeEdge( int index )
{
  auto it = mGraphEdges.constFind( index );
  if ( it != mGraphEdges.constEnd() )
  {
    const int fromVertex = it->fromVertex();
    const int toVertex = it->toVertex();
    mGraphEdges.erase( it );

    // clean up affected vertices
    auto vertexIt = mGraphVertices.find( fromVertex );
    if ( vertexIt != mGraphVertices.end() )
    {
      vertexIt->mOutgoingEdges.removeAll( index );
      if ( vertexIt->mOutgoingEdges.empty() && vertexIt->mIncomingEdges.empty() )
        mGraphVertices.erase( vertexIt );
    }

    vertexIt = mGraphVertices.find( toVertex );
    if ( vertexIt != mGraphVertices.end() )
    {
      vertexIt->mIncomingEdges.removeAll( index );
      if ( vertexIt->mOutgoingEdges.empty() && vertexIt->mIncomingEdges.empty() )
        mGraphVertices.erase( vertexIt );
    }
  }
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

bool QgsGraph::hasVertex( int index ) const
{
  auto it = mGraphVertices.constFind( index );
  return it != mGraphVertices.constEnd();
}

bool QgsGraph::hasEdge( int index ) const
{
  auto it = mGraphEdges.constFind( index );
  return it != mGraphEdges.constEnd();
}

int QgsGraph::findOppositeEdge( int index ) const
{
  auto it = mGraphEdges.constFind( index );
  if ( it != mGraphEdges.constEnd() )
  {
    const int fromVertex = it->fromVertex();
    const int toVertex = it->toVertex();

    // look for edges which start at toVertex
    const QgsGraphEdgeIds candidates = mGraphVertices.value( toVertex ).outgoingEdges();
    for ( int candidate : candidates )
    {
      if ( mGraphEdges.value( candidate ).toVertex() == fromVertex )
        return candidate;
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
