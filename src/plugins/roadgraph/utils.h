/***************************************************************************
 *   Copyright (C) 2009 by Sergey Yakushev                                 *
 *   yakushevs <at> list.ru                                                *
 *                                                                         *
 *   This is file define road-graph plugins utils                          *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/
#ifndef ROADGRAPHPLUGIN_UTILS_H
#define ROADGRAPHPLUGIN_UTILS_H

//  QT includes
#include <qlist.h>

// Qgis includes
#include "qgspoint.h"

// standart includes
#include <map>
#include <set>

// forward declaration Qgis-classes

/**
@author Sergey Yakushev
*/

/**
 * \function infinity()
 * \brief return big const double value
 */
double infinity();

/**
 * return distance from lenght <x1,y1><x2,y2> to point <x,y> or infinity() value
 * in union point
 */
double distance( const QgsPoint& p1, const QgsPoint& p2, const QgsPoint& p, QgsPoint& center );

/**
 * \class QgsPointCompare
 * \brief This class implement operation "<", ">", "==" and etc. for use QgsPoint type in STL containers
 *
 */
class QgsPointCompare
{
public:
  bool operator()( const QgsPoint& a, const QgsPoint& b ) const;
};

/**
 * \class ArcAttributes
 * \brief This class contained arcs attributes.
 */
class ArcAttributes
{
public:
  ArcAttributes();
  ArcAttributes( double cost, double time, int mFeatureId );
public:
  double mCost;
  double mTime;
  int mFeatureId;
};

typedef std::map< QgsPoint, ArcAttributes, QgsPointCompare > AdjacencyMatrixString;
typedef std::map< QgsPoint, AdjacencyMatrixString,  QgsPointCompare > AdjacencyMatrix;


/**
 * \class DijkstraFinder
 * \brief This class find shortest path via two points using Dijkstra's algorithm
 */
class DijkstraFinder 
{
public:
  enum OptimizationCriterion { byTime=1, byCost=2 };
private:
  class DijkstraIterator {
  public:
    DijkstraIterator()
    {
      mCost = infinity();
      mTime = infinity();
    }
    double mCost;
    double mTime;
    QgsPoint mBackPoint;
    QgsPoint mFrontPoint;
  };
  class CompareDijkstraIterator {
  public:
    CompareDijkstraIterator( OptimizationCriterion criterion = byCost ) :
      mCriterion( criterion )
    { }
    bool operator()( const DijkstraIterator& a, const DijkstraIterator& b ) const
    {
      if ( mCriterion == byCost )
      {
        return a.mCost < b.mCost;
      }
      return a.mTime < b.mTime;
    }
    bool operator==( const CompareDijkstraIterator& a ) const
    {
      return mCriterion == a.mCriterion;
    }
  private:
    OptimizationCriterion mCriterion;
  };
public:
  /**
   * constructor.
   * m is adjacency matrix of graph, criterion is a citerion of shortest path
   */
  DijkstraFinder( const AdjacencyMatrix &m, OptimizationCriterion criterion );

  /**
   * find all shortest path from point frontPoint to all points of graph.
   * return tree.
   */
  std::map< QgsPoint , DijkstraIterator, QgsPointCompare > find( const QgsPoint& p );
  
  /**
   * return shortest path form point frontPoint to endPoint
   */
  AdjacencyMatrix find( const QgsPoint& frontPoint, const QgsPoint& endPoint );
  
private:
  const AdjacencyMatrix& mAdjacencyMatrix;
  OptimizationCriterion mCriterion;
};
#endif
