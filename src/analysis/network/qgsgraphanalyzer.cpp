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
// C++ standart includes
#include <limits>

// QT includes
#include <QMap>
#include <QVector>
#include <QPair>

//QGIS-uncludes
#include "qgsgraph.h"
#include "qgsgraphanalyzer.h"

void QgsGraphAnalyzer::shortestpath( const QgsGraph* source, int startPointIdx, int criterionNum, const QVector<int>& destPointCost, QVector<double>& cost, QgsGraph* treeResult )
{
  
  // QMap< cost, vertexIdx > not_begin
  // I use it and not create any struct or class.
  QMap< double, int > not_begin;  
  QMap< double, int >::iterator it;

  // QVector< QPair< cost, edge id > result
  QVector< QPair< double, int > > result;

  result.reserve( source->vertexCount() );
  int i;
  for ( i=0; i < source->vertexCount(); ++i )
  {
    result.push_back( QPair<double, int> ( std::numeric_limits<double>::infinity() , i ) );
  }
  result.push_back ( QPair<double, int>( 0.0, -1 ) );

  not_begin.insert( 0.0, startPointIdx );
  
  // begin Dijkstra algorithm
  while ( !not_begin.empty() )
  {
    it = not_begin.begin();
    double curCost = it.key();
    int curVertex = it.value();
    not_begin.erase( it );

    QgsGraphEdgeList l = source->vertex( curVertex ).outEdges();
    QgsGraphEdgeList::iterator edgeIt;
    for (edgeIt = l.begin(); edgeIt != l.end(); ++edgeIt)
    {
      const QgsGraphEdge& edge = source->edge( *edgeIt );
      double cost = edge.property( criterionNum ).toDouble() + curCost;

      if ( cost < result[ edge.in() ].first )
      {
        result[ edge.in() ] = QPair< double, int >( cost, *edgeIt );
        not_begin.insert( cost, edge.in() );
      }
    }
  }
  
  // fill shortestpath tree
  if ( treeResult != NULL )
  {
    // sourceVertexIdx2resultVertexIdx
    QVector<int> source2result( result.size(), -1 );

    for ( i=0; i < source->vertexCount(); ++i )
    {
      if ( result[ i ].first < std::numeric_limits<double>::infinity() )
      {
        source2result[ i ] = treeResult->addVertex( source->vertex(i).point() );
      }
    }
    for ( i=0; i < source->vertexCount(); ++i )
    {
      if ( result[ i ].first < std::numeric_limits<double>::infinity() && result[i].second != -1)
      {  
        const QgsGraphEdge& edge = source->edge( result[i].second );

        treeResult->addEdge( source2result[ edge.out() ], source2result[ i ], 
            edge.properties() );
      }
    }
  }
  
  // fill shortestpath's costs
  for ( i=0; i < destPointCost.size(); ++i )
  {
    cost[i] = result[ destPointCost[i] ].first;
  }
}
