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
 * \brief implementation QgsGraph, QgsGraphVertex, QgsGraphArc
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
  return mGraphVertexes.size() - 1;
}

int QgsGraph::addArc( int outVertexIdx, int inVertexIdx, const QVector< QVariant >& properties )
{
  QgsGraphArc e;

  e.mProperties = properties;
  e.mOut = outVertexIdx;
  e.mIn  = inVertexIdx;
  mGraphArc.push_back( e );
  int edgeIdx = mGraphArc.size() - 1;

  mGraphVertexes[ outVertexIdx ].mOutArc.push_back( edgeIdx );
  mGraphVertexes[ inVertexIdx ].mInArc.push_back( edgeIdx );

  return mGraphArc.size() - 1;
}

const QgsGraphVertex& QgsGraph::vertex( int idx ) const
{
  return mGraphVertexes[ idx ];
}

const QgsGraphArc& QgsGraph::arc( int idx ) const
{
  return mGraphArc[ idx ];
}


int QgsGraph::vertexCount() const
{
  return mGraphVertexes.size();
}

int QgsGraph::arcCount() const
{
  return mGraphArc.size();
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

QgsGraphArc::QgsGraphArc()
{

}

QVariant QgsGraphArc::property( int i ) const
{
  return mProperties[ i ];
}

QVector< QVariant > QgsGraphArc::properties() const
{
  return mProperties;
}

int QgsGraphArc::inVertex() const
{
  return mIn;
}

int QgsGraphArc::outVertex() const
{
  return mOut;
}

QgsGraphVertex::QgsGraphVertex( const QgsPoint& point )
    : mCoordinate( point )
{

}

QgsGraphArcIdList QgsGraphVertex::outArc() const
{
  return mOutArc;
}

QgsGraphArcIdList QgsGraphVertex::inArc() const
{
  return mInArc;
}

QgsPoint QgsGraphVertex::point() const
{
  return mCoordinate;
}
