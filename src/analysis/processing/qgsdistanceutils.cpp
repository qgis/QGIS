/***************************************************************************
                         qgsdistanceutils.cpp
                         ---------------------
    begin                : June 2026
    copyright            : (C) 2026 by Alexander Bruy
    email                : alexander dot bruy at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsdistanceutils.h"

#include <algorithm>

#include "qgsdistancearea.h"
#include "qgsprocessingfeedback.h"

#include <QString>
#include <queue>

using namespace Qt::StringLiterals;

///@cond PRIVATE

std::vector<QgsDistanceUtils::NeighborResult> QgsDistanceUtils::nearestNeighbors(
  const QgsPointXY &sourcePoint, const std::vector<std::pair<QgsFeatureId, QgsPointXY>> &targetPoints, const QgsDistanceArea &da, long long k, QgsProcessingFeedback *feedback
)
{
  if ( targetPoints.empty() )
  {
    return {};
  }

  std::vector<QgsDistanceUtils::NeighborResult> result;

  // If k is 0 or greater than total targets, we just calculate and sort all of them
  if ( k <= 0 || static_cast<size_t>( k ) >= targetPoints.size() )
  {
    result.reserve( targetPoints.size() );
    for ( const auto &target : targetPoints )
    {
      if ( feedback && feedback->isCanceled() )
      {
        break;
      }

      double dist = da.measureLine( sourcePoint, target.second );
      result.push_back( { target.first, dist, target.second } );
    }
    std::sort( result.begin(), result.end(), []( const NeighborResult &a, const NeighborResult &b ) { return a.distance < b.distance; } );
    return result;
  }

  // If we only need top K, we use a priority queue, sorted from largest to smallest distance
  auto cmp = []( const NeighborResult &a, const NeighborResult &b ) { return a.distance < b.distance; };
  std::priority_queue<NeighborResult, std::vector<NeighborResult>, decltype( cmp )> points_queue( cmp );

  for ( const auto &target : targetPoints )
  {
    if ( feedback && feedback->isCanceled() )
    {
      break;
    }

    double dist = da.measureLine( sourcePoint, target.second );

    if ( points_queue.size() < static_cast<size_t>( k ) )
    {
      points_queue.push( { target.first, dist, target.second } );
    }
    else if ( dist < points_queue.top().distance )
    {
      points_queue.pop();
      points_queue.push( { target.first, dist, target.second } );
    }
  }

  result.reserve( points_queue.size() );
  while ( !points_queue.empty() )
  {
    result.push_back( points_queue.top() );
    points_queue.pop();
  }

  // reverse to return nearest to furthers points
  std::reverse( result.begin(), result.end() );

  return result;
}

///@endcond PRIVATE
