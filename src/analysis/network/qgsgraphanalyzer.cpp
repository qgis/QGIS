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

  // QVector< QPair< cost, arc id > result
  QVector< QPair< double, int > > result;

  result.reserve( source->vertexCount() );
  int i;
  for ( i=0; i < source->vertexCount(); ++i )
  {
    result.push_back( QPair<double, int> ( std::numeric_limits<double>::infinity() , i ) );
  }
  result[ startPointIdx ] = QPair<double, int> ( 0.0, -1 );

  not_begin.insert( 0.0, startPointIdx );
  
  // begin Dijkstra algorithm
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
      const QgsGraphArc& arc = source->arc( *arcIt );
      double cost = arc.property( criterionNum ).toDouble() + curCost;

      if ( cost < result[ arc.in() ].first )
      {
        result[ arc.in() ] = QPair< double, int >( cost, *arcIt );
        not_begin.insert( cost, arc.in() );
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
        const QgsGraphArc& arc = source->arc( result[i].second );

        treeResult->addArc( source2result[ arc.out() ], source2result[ i ], 
            arc.properties() );
      }
    }
  }
  
  // fill shortestpath's costs
  for ( i=0; i < destPointCost.size(); ++i )
  {
    cost[i] = result[ destPointCost[i] ].first;
  }
}
