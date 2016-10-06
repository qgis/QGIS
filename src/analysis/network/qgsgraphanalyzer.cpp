/***************************************************************************
    qgsgraphanlyzer.cpp - QGIS Tools for graph analysis
                             -------------------
    begin                : 14 april 2011
    copyright            : (C) Sergey Yakushev
    email                : Yakushevs@list.ru
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
// C++ standard includes
#include <limits>

// QT includes
#include <QMap>
#include <QVector>
#include <QPair>

//QGIS-uncludes
#include "qgsgraph.h"
#include "qgsgraphanalyzer.h"

void QgsGraphAnalyzer::dijkstra( const QgsGraph* source, int startPointIdx, int criterionNum, QVector<int>* resultTree, QVector<double>* resultCost )
{
  QVector< double > * result = nullptr;
  if ( resultCost )
  {
    result = resultCost;
  }
  else
  {
    result = new QVector<double>();
  }

  result->clear();
  result->insert( result->begin(), source->vertexCount(), std::numeric_limits<double>::infinity() );
  ( *result )[ startPointIdx ] = 0.0;

  if ( resultTree )
  {
    resultTree->clear();
    resultTree->insert( resultTree->begin(), source->vertexCount(), -1 );
  }

  // QMultiMap< cost, vertexIdx > not_begin
  // I use it and not create any struct or class.
  QMultiMap< double, int > not_begin;
  QMultiMap< double, int >::iterator it;

  not_begin.insert( 0.0, startPointIdx );

  while ( !not_begin.empty() )
  {
    it = not_begin.begin();
    double curCost = it.key();
    int curVertex = it.value();
    not_begin.erase( it );

    // edge index list
    QgsGraphArcIdList l = source->vertex( curVertex ).outArc();
    QgsGraphArcIdList::iterator arcIt;
    for ( arcIt = l.begin(); arcIt != l.end(); ++arcIt )
    {
      const QgsGraphArc arc = source->arc( *arcIt );
      double cost = arc.property( criterionNum ).toDouble() + curCost;

      if ( cost < ( *result )[ arc.inVertex()] )
      {
        ( *result )[ arc.inVertex()] = cost;
        if ( resultTree )
        {
          ( *resultTree )[ arc.inVertex()] = *arcIt;
        }
        not_begin.insert( cost, arc.inVertex() );
      }
    }
  }
  if ( !resultCost )
  {
    delete result;
  }
}

QgsGraph* QgsGraphAnalyzer::shortestTree( const QgsGraph* source, int startVertexIdx, int criterionNum )
{
  QgsGraph *treeResult = new QgsGraph();
  QVector<int> tree;

  QgsGraphAnalyzer::dijkstra( source, startVertexIdx, criterionNum, &tree );

  // sourceVertexIdx2resultVertexIdx
  QVector<int> source2result( tree.size(), -1 );

  // Add reachable vertices to the result
  source2result[ startVertexIdx ] = treeResult->addVertex( source->vertex( startVertexIdx ).point() );
  int i = 0;
  for ( i = 0; i < source->vertexCount(); ++i )
  {
    if ( tree[ i ] != -1 )
    {
      source2result[ i ] = treeResult->addVertex( source->vertex( i ).point() );
    }
  }

  // Add arcs to result
  for ( i = 0; i < source->vertexCount(); ++i )
  {
    if ( tree[ i ] != -1 )
    {
      const QgsGraphArc& arc = source->arc( tree[i] );

      treeResult->addArc( source2result[ arc.outVertex()], source2result[ arc.inVertex()],
                          arc.properties() );
    }
  }

  return treeResult;
}
