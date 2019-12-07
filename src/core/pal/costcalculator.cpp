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

bool CostCalculator::candidateSortShrink( const std::unique_ptr< LabelPosition > &c1, const std::unique_ptr< LabelPosition > &c2 )
{
  return c1->cost() > c2->cost();
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

void CostCalculator::setPolygonCandidatesCost( std::size_t nblp, std::vector< std::unique_ptr< LabelPosition > > &lPos, RTree<FeaturePart *, double, 2, double> *obstacles, double bbx[4], double bby[4] )
{
  double normalizer;
  // compute raw cost
  for ( std::size_t i = 0; i < nblp; ++i )
    setCandidateCostFromPolygon( lPos[ i ].get(), obstacles, bbx, bby );

  // lPos with big values came first (value = min distance from label to Polygon's Perimeter)
  // IMPORTANT - only want to sort first nblp positions. The rest have not had the cost
  // calculated so will have nonsense values
  std::sort( lPos.begin(), lPos.begin() + nblp, candidateSortShrink );

  // define the value's range
  double cost_max = lPos.front()->cost();
  double cost_min = lPos.back()->cost();

  cost_max -= cost_min;

  if ( cost_max > EPSILON )
  {
    normalizer = 0.0020 / cost_max;
  }
  else
  {
    normalizer = 1;
  }

  // adjust cost => the best is 0.0001, the worst is 0.0021
  // others are set proportionally between best and worst
  for ( std::size_t i = 0; i < nblp; ++i )
  {
    LabelPosition *pos = lPos[ i ].get();
    //if (cost_max - cost_min < EPSILON)
    if ( cost_max > EPSILON )
    {
      pos->setCost( 0.0021 - ( pos->cost() - cost_min ) * normalizer );
    }
    else
    {
      //pos->cost = 0.0001 + (pos->cost - cost_min) * normalizer;
      pos->setCost( 0.0001 );
    }
  }
}

void CostCalculator::setCandidateCostFromPolygon( LabelPosition *lp, RTree <FeaturePart *, double, 2, double> *obstacles, double bbx[4], double bby[4] )
{
  double amin[2];
  double amax[2];

  PolygonCostCalculator *pCost = new PolygonCostCalculator( lp );

  // center
  //cost = feat->getDistInside((this->x[0] + this->x[2])/2.0, (this->y[0] + this->y[2])/2.0 );

  pCost->update( lp->feature );

  PointSet *extent = new PointSet( 4, bbx, bby );

  pCost->update( extent );

  delete extent;

  lp->feature->getBoundingBox( amin, amax );

  obstacles->Search( amin, amax, LabelPosition::polygonObstacleCallback, pCost );

  lp->setCost( pCost->getCost() );

  delete pCost;
}

std::size_t CostCalculator::finalizeCandidatesCosts( Feats *feat, std::size_t max_p, RTree <FeaturePart *, double, 2, double> *obstacles, double bbx[4], double bby[4] )
{
  // If candidates list is smaller than expected
  if ( max_p > feat->candidates.size() )
    max_p = feat->candidates.size();
  //
  // sort candidates list, best label to worst
  std::sort( feat->candidates.begin(), feat->candidates.end(), candidateSortGrow );

  // try to exclude all conflitual labels (good ones have cost < 1 by pruning)
  double discrim = 0.0;
  std::size_t stop = 0;
  do
  {
    discrim += 1.0;
    for ( stop = 0; stop < feat->candidates.size() && feat->candidates[ stop ]->cost() < discrim; stop++ )
      ;
  }
  while ( stop == 0 && discrim < feat->candidates.back()->cost() + 2.0 );

  // THIS LOOKS SUSPICIOUS -- it clamps all costs to a fixed value??
  if ( discrim > 1.5 )
  {
    for ( std::size_t k = 0; k < stop; k++ )
      feat->candidates[ k ]->setCost( 0.0021 );
  }

  if ( max_p > stop )
    max_p = stop;

  // Sets costs for candidates of polygon

  if ( feat->feature->getGeosType() == GEOS_POLYGON )
  {
    int arrangement = feat->feature->layer()->arrangement();
    if ( arrangement == QgsPalLayerSettings::Free || arrangement == QgsPalLayerSettings::Horizontal )
      setPolygonCandidatesCost( stop, feat->candidates, obstacles, bbx, bby );
  }

  // add size penalty (small lines/polygons get higher cost)
  feat->feature->addSizePenalty( max_p, feat->candidates, bbx, bby );

  return max_p;
}

PolygonCostCalculator::PolygonCostCalculator( LabelPosition *lp ) : lp( lp )
{
  px = ( lp->x[0] + lp->x[2] ) / 2.0;
  py = ( lp->y[0] + lp->y[2] ) / 2.0;

  dist = std::numeric_limits<double>::max();
  ok = false;
}

void PolygonCostCalculator::update( PointSet *pset )
{
  double d = pset->minDistanceToPoint( px, py );
  if ( d < dist )
  {
    dist = d;
  }
}

LabelPosition *PolygonCostCalculator::getLabel()
{
  return lp;
}

double PolygonCostCalculator::getCost()
{
  return ( 4 * dist );
}
