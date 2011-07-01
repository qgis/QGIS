/***************************************************************************
 *   Copyright (C) 2009 by Sergey Yakushev                                 *
 *   yakushevs <at> list.ru                                                *
 *                                                                         *
 *   This is a plugin generated from the QGIS plugin template              *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

/**
 * \file utils.cpp
 * \brief implementation of Road-graph plugins utils
 */

#include "utils.h"

//qt includes

// Qgis includes
#include <qgspoint.h>

// standard includes
#include <set>
#include <cmath>

double infinity()
{
  return 1e12;
}

// return distance between line of two points and center
double distance( const QgsPoint& p1, const QgsPoint& p2, const QgsPoint& p, QgsPoint& center )
{

  // first line
  double A1, B1, C1;
  A1 = p1.y() - p2.y();
  B1 = p2.x() - p1.x();
  C1 = p1.x() * ( -A1 ) + p1.y() * ( -B1 );

  // second line. First and second line is perpendicular.
  double A2, B2, C2;
  A2 = B1;
  B2 = -A1;
  C2 = -p.x() * A2 - p.y() * B2;

  // union point
  double x, y, det;
  det = A1 * B2 - B1 * A2;
  x = ( C2 * B1 - B2 * C1 ) / det;
  y = ( -A1 * C2 + C1 * A2 ) / det;

  center = QgsPoint( x, y );

  det = sqrt( A1 * A1 + B1 * B1 );
  A1 /= det;
  B1 /= det;
  C1 /= det;
  if ( std::min( p1.x(), p2.x() ) <= x && std::max( p1.x(), p2.x() ) >= x &&
       std::min( p1.y(), p2.y() ) <= y && std::max( p1.y(), p2.y() ) >= y )
    return std::abs( A1*p.x() + B1*p.y() + C1 );

  return infinity();
}// RoadGraphPlugin::distance()


bool QgsPointCompare::operator()( const QgsPoint& a, const QgsPoint& b ) const
{
  return a.x() == b.x() ? a.y() < b.y() : a.x() < b.x();
}

ArcAttributes::ArcAttributes()
{
  mCost = infinity();
  mTime = infinity();
}

ArcAttributes::ArcAttributes( double cost, double time, QgsFeatureId featureId ) :
    mCost( cost ), mTime( time ), mFeatureId( featureId )
{
}


DijkstraFinder::DijkstraFinder( const AdjacencyMatrix& m, DijkstraFinder::OptimizationCriterion c ):
    mAdjacencyMatrix( m ), mCriterion( c )
{
}

std::map< QgsPoint , DijkstraFinder::DijkstraIterator, QgsPointCompare> DijkstraFinder::find( const QgsPoint& p )
{
  CompareDijkstraIterator ci( mCriterion );
  std::set< DijkstraIterator, CompareDijkstraIterator > not_begin( ci );
  std::set< DijkstraIterator, CompareDijkstraIterator >::iterator it;
  std::map< QgsPoint, DijkstraIterator, QgsPointCompare> res;
  if ( mAdjacencyMatrix.find( p ) == mAdjacencyMatrix.end() )
  {
    return res;
  }

  AdjacencyMatrixString::const_iterator arcIt;
  AdjacencyMatrixString::const_iterator end = mAdjacencyMatrix.find( p )->second.end();

  DijkstraIterator f;
  f.mCost = 0;
  f.mTime = 0;
  f.mFrontPoint = p;
  f.mBackPoint = p;
  res[ p ] = f;
  not_begin.insert( f );

  while ( !not_begin.empty() )
  {
    it = not_begin.begin();
    DijkstraIterator i = *it;
    not_begin.erase( it );

    if ( mAdjacencyMatrix.find( i.mBackPoint ) == mAdjacencyMatrix.end() )
    {
      continue;
    }
    end = mAdjacencyMatrix.find( i.mBackPoint )->second.end();
    for ( arcIt = mAdjacencyMatrix.find( i.mBackPoint )->second.begin(); arcIt != end; ++arcIt )
    {
      DijkstraIterator di = i;
      di.mCost += arcIt->second.mCost;
      di.mTime += arcIt->second.mTime;

      if ( ci( di, res[ arcIt->first ] ) )
      {
        di.mFrontPoint = di.mBackPoint;
        di.mBackPoint = arcIt->first;
        not_begin.insert( di );
        res[ arcIt->first ] = di;
      }
    }
  }
  return res;
} // DijkstraFinder::find( const QgsPoint& p )

AdjacencyMatrix DijkstraFinder::find( const QgsPoint& frontPoint, const QgsPoint& backPoint )
{
  std::map< QgsPoint , DijkstraFinder::DijkstraIterator, QgsPointCompare> r = find( frontPoint );
  std::map< QgsPoint , DijkstraFinder::DijkstraIterator, QgsPointCompare>::iterator it;
  if ( r.find( backPoint ) == r.end() )
  {
    return AdjacencyMatrix();
  }

  AdjacencyMatrix m;
  m[ frontPoint ];
  QgsPoint nextPoint = backPoint;
  QgsPoint firstPoint = backPoint;
  while ( true )
  {
    if ( firstPoint != nextPoint )
      m[ nextPoint ][ firstPoint ] = mAdjacencyMatrix.find( nextPoint )->second.find( firstPoint )->second;

    if ( r[ nextPoint ].mFrontPoint == r[ nextPoint ].mBackPoint )
      break;
    firstPoint = nextPoint;
    nextPoint = r[ nextPoint ].mFrontPoint;
  }
  return m;
} // DijkstraFinder::find( const QgsPoint& frontPoint, const QgsPoint& backPoint )
