/***************************************************************************
    costcalculator.cpp
    ---------------------
    begin                : November 2009
    copyright            : (C) 2009 by Martin Dobias
    email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "layer.h"
#include "pal.h"
#include "feature.h"
#include "geomfunction.h"
#include "labelposition.h"
#include "util.h"
#include "costcalculator.h"
#include <cmath>
#include <cfloat>

using namespace pal;

bool CostCalculator::candidateSortGrow( const std::unique_ptr< LabelPosition > &c1, const std::unique_ptr< LabelPosition > &c2 )
{
  return c1->cost() < c2->cost();
}

void CostCalculator::addObstacleCostPenalty( LabelPosition *lp, FeaturePart *obstacle, Pal *pal )
{
  int n = 0;
  double dist;
  double distlabel = lp->feature->getLabelDistance();

  switch ( obstacle->getGeosType() )
  {
    case GEOS_POINT:

      dist = lp->getDistanceToPoint( obstacle->x[0], obstacle->y[0] );
      if ( dist < 0 )
        n = 2;
      else if ( dist < distlabel )
        //note this never happens at the moment - points are not obstacles if they don't fall
        //within the label
        n = 1;
      else
        n = 0;
      break;

    case GEOS_LINESTRING:

      // Is one of label's borders crossing the line ?
      n = ( lp->crossesLine( obstacle ) ? 1 : 0 );
      break;

    case GEOS_POLYGON:
      // behavior depends on obstacle avoid type
      switch ( obstacle->layer()->obstacleType() )
      {
        case QgsLabelObstacleSettings::PolygonInterior:
          // n ranges from 0 -> 12
          n = lp->polygonIntersectionCost( obstacle );
          break;
        case QgsLabelObstacleSettings::PolygonBoundary:
          // penalty may need tweaking, given that interior mode ranges up to 12
          n = ( lp->crossesBoundary( obstacle ) ? 6 : 0 );
          break;
        case QgsLabelObstacleSettings::PolygonWhole:
          // n is either 0 or 12
          n = ( lp->intersectsWithPolygon( obstacle ) ? 12 : 0 );
          break;
      }

      break;
  }

  //scale cost by obstacle's factor
  double obstacleCost = obstacle->obstacleSettings().factor() * double( n );
  if ( n > 0 )
    lp->setConflictsWithObstacle( true );

  switch ( pal->placementVersion() )
  {
    case QgsLabelingEngineSettings::PlacementEngineVersion1:
      break;

    case QgsLabelingEngineSettings::PlacementEngineVersion2:
    {
      // obstacle factor is from 0 -> 2, label priority is from 1 -> 0. argh!
      const double priority = 2 * ( 1 - lp->feature->calculatePriority() );
      const double obstaclePriority = obstacle->obstacleSettings().factor();

      // if feature priority is < obstaclePriorty, there's a hard conflict...
      if ( n > 0 && ( priority < obstaclePriority && !qgsDoubleNear( priority, obstaclePriority, 0.001 ) ) )
      {
        lp->setHasHardObstacleConflict( true );
      }
      break;
    }
  }

  // label cost is penalized
  lp->setCost( lp->cost() + obstacleCost );
}

void CostCalculator::calculateCandidatePolygonRingDistanceCosts( std::vector< std::unique_ptr< LabelPosition > > &lPos, PalRtree<FeaturePart> *obstacles, double bbx[4], double bby[4] )
{
  // first we calculate the ring distance cost for all candidates for this feature. We then use the range
  // of distance costs to calculate a standardised scaling for the costs
  QHash< LabelPosition *, double > polygonRingDistanceCosts;
  double minCandidateRingDistanceCost = std::numeric_limits< double >::max();
  double maxCandidateRingDistanceCost = std::numeric_limits< double >::lowest();
  for ( std::unique_ptr< LabelPosition > &pos : lPos )
  {
    const double candidatePolygonRingDistanceCost = calculatePolygonRingDistance( pos.get(), obstacles, bbx, bby );

    minCandidateRingDistanceCost = std::min( minCandidateRingDistanceCost, candidatePolygonRingDistanceCost );
    maxCandidateRingDistanceCost = std::max( maxCandidateRingDistanceCost, candidatePolygonRingDistanceCost );

    polygonRingDistanceCosts.insert( pos.get(), candidatePolygonRingDistanceCost );
  }

  // define the cost's range, if range is too small, just ignore the ring distance cost
  const double costRange = maxCandidateRingDistanceCost - minCandidateRingDistanceCost;
  if ( costRange <= EPSILON )
    return;

  const double normalizer = 0.0020 / costRange;

  // adjust cost => the best is 0.0001, the worst is 0.0021
  // others are set proportionally between best and worst
  for ( std::unique_ptr< LabelPosition > &pos : lPos )
  {
    const double originalPolygonRingDistanceCost = polygonRingDistanceCosts.value( pos.get() );
    pos->setCost( pos->cost() + 0.0021 - ( originalPolygonRingDistanceCost - minCandidateRingDistanceCost ) * normalizer );
  }
}

double CostCalculator::calculatePolygonRingDistance( LabelPosition *candidate, PalRtree<FeaturePart> *obstacles, double bbx[4], double bby[4] )
{
  // TODO 1: Consider whether distance calculation should use min distance to the candidate rectangle
  // instead of just the center
  CandidatePolygonRingDistanceCalculator ringDistanceCalculator( candidate );

  // first, check max distance to outside of polygon
  // TODO 2: there's a bug here -- if a candidate's center falls outside the polygon, then a larger
  // distance to the polygon boundary is being considered as the best placement. That's clearly wrong --
  // if any part of label candidate sits outside the polygon, we should first prefer candidates which sit
  // entirely WITHIN the polygon, or failing that, candidates which are CLOSER to the polygon boundary, not further from it!
  ringDistanceCalculator.update( candidate->feature );

  // prefer candidates further from the outside of map rather then those close to the outside of the map
  // TODO 3: should be using the actual map boundary here, not the bounding box
  PointSet extent( 4, bbx, bby );
  ringDistanceCalculator.update( &extent );

  // prefer candidates which further from interior rings (holes) of the polygon
  obstacles->intersects( candidate->feature->boundingBox(), [&ringDistanceCalculator, candidate]( const FeaturePart * obstacle )->bool
  {
    // we only care about obstacles which are polygon holes, AND only holes which belong to this same feature
    // because:
    // 1. holes for other features are a good place to put labels for this feature
    // 2. we handle obstacle avoidance for all candidate types elsewhere -- here we are solely concerned with
    // ranking the relative candidates for a single feature while considering that feature's shape alone.
    if ( ( obstacle == candidate->feature ) || ( !obstacle->getHoleOf() ) || ( obstacle->getHoleOf() && obstacle->getHoleOf() != candidate->feature ) )
    {
      return true;
    }

    ringDistanceCalculator.update( obstacle );

    return true;
  } );

  return ringDistanceCalculator.getCost();
}

void CostCalculator::finalizeCandidatesCosts( Feats *feat, PalRtree<FeaturePart> *obstacles, double bbx[4], double bby[4] )
{
  if ( feat->feature->getGeosType() == GEOS_POLYGON )
  {
    int arrangement = feat->feature->layer()->arrangement();
    if ( arrangement == QgsPalLayerSettings::Free || arrangement == QgsPalLayerSettings::Horizontal )
      calculateCandidatePolygonRingDistanceCosts( feat->candidates, obstacles, bbx, bby );
  }

  // add size penalty (small lines/polygons get higher cost)
  feat->feature->addSizePenalty( feat->candidates, bbx, bby );
}

CandidatePolygonRingDistanceCalculator::CandidatePolygonRingDistanceCalculator( LabelPosition *candidate )
  : mPx( ( candidate->x[0] + candidate->x[2] ) / 2.0 )
  , mPy( ( candidate->y[0] + candidate->y[2] ) / 2.0 )
{
}

void CandidatePolygonRingDistanceCalculator::update( const pal::PointSet *pset )
{
  double d = pset->minDistanceToPoint( mPx, mPy );
  if ( d < mMinDistance )
  {
    mMinDistance = d;
  }
}

double CandidatePolygonRingDistanceCalculator::getCost()
{
  return ( 4 * mMinDistance );
}
