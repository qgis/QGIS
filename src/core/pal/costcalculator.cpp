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

#include <iostream>
#include <fstream>
#include <cmath>
#include <cstring>
#include <cfloat>

#include <pal/layer.h>
#include <pal/pal.h>

#include "feature.h"
#include "geomfunction.h"
#include "labelposition.h"
#include "util.h"

#include "costcalculator.h"

namespace pal
{

  void CostCalculator::addObstacleCostPenalty( LabelPosition* lp, PointSet* feat )
  {
    int n = 0;
    double dist;
    double distlabel = lp->feature->getLabelDistance();
#if 0
    unit_convert( double( lp->feature->distlabel ),
                  pal::PIXEL,
                  pal->map_unit,
                  pal->dpi, scale, 1 );
#endif

    switch ( feat->getGeosType() )
    {
      case GEOS_POINT:

        dist = lp->getDistanceToPoint( feat->x[0], feat->y[0] );
        if ( dist < 0 )
          n = 2;
        else if ( dist < distlabel )
          n = 1;
        else
          n = 0;
        break;

      case GEOS_LINESTRING:

        // Is one of label's borders crossing the line ?
        n = ( lp->isBorderCrossingLine( feat ) ? 1 : 0 );
        break;

      case GEOS_POLYGON:
        n = lp->getNumPointsInPolygon( feat->getNumPoints(), feat->x, feat->y );
        break;
    }

    // label cost is penalized
    lp->setCost( lp->getCost() + double( n ) );
  }


  ////////

  void CostCalculator::setPolygonCandidatesCost( int nblp, LabelPosition **lPos, int max_p, RTree<PointSet*, double, 2, double> *obstacles, double bbx[4], double bby[4] )
  {
    int i;

    double normalizer;
    // compute raw cost
#ifdef _DEBUG_
    std::cout << "LabelPosition for feat: " << lPos[0]->feature->uid << std::endl;
#endif

    for ( i = 0; i < nblp; i++ )
      setCandidateCostFromPolygon( lPos[i], obstacles, bbx, bby );

    // lPos with big values came fisrts (value = min distance from label to Polygon's Perimeter)
    //sort ( (void**) lPos, nblp, costGrow);
    sort(( void** ) lPos, nblp, LabelPosition::costShrink );


    // define the value's range
    double cost_max = lPos[0]->getCost();
    double cost_min = lPos[max_p-1]->getCost();

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
    for ( i = 0; i < max_p; i++ )
    {
#ifdef _DEBUG_
      std::cout << "   lpos[" << i << "] = " << lPos[i]->cost;
#endif
      //if (cost_max - cost_min < EPSILON)
      if ( cost_max > EPSILON )
      {
        lPos[i]->cost = 0.0021 - ( lPos[i]->getCost() - cost_min ) * normalizer;
      }
      else
      {
        //lPos[i]->cost = 0.0001 + (lPos[i]->cost - cost_min) * normalizer;
        lPos[i]->cost = 0.0001;
      }

#ifdef _DEBUG_
      std::cout <<  "  ==>  " << lPos[i]->cost << std::endl;
#endif
    }
  }


  void CostCalculator::setCandidateCostFromPolygon( LabelPosition* lp, RTree <PointSet*, double, 2, double> *obstacles, double bbx[4], double bby[4] )
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

  int CostCalculator::finalizeCandidatesCosts( Feats* feat, int max_p, RTree <PointSet*, double, 2, double> *obstacles, double bbx[4], double bby[4] )
  {
    // If candidates list is smaller than expected
    if ( max_p > feat->nblp )
      max_p = feat->nblp;
    //
    // sort candidates list, best label to worst
    sort(( void** ) feat->lPos, feat->nblp, LabelPosition::costGrow );

    // try to exclude all conflitual labels (good ones have cost < 1 by pruning)
    double discrim = 0.0;
    int stop;
    do
    {
      discrim += 1.0;
      for ( stop = 0; stop < feat->nblp && feat->lPos[stop]->getCost() < discrim; stop++ )
        ;
    }
    while ( stop == 0 && discrim < feat->lPos[feat->nblp-1]->getCost() + 2.0 );

    if ( discrim > 1.5 )
    {
      int k;
      for ( k = 0; k < stop; k++ )
        feat->lPos[k]->setCost( 0.0021 );
    }

    if ( max_p > stop )
      max_p = stop;

#ifdef _DEBUG_FULL_
    std::cout << "Nblabel kept for feat " << feat->feature->getUID() << "/" << feat->feature->getLayer()->getName() << ": " << max_p << "/" << feat->nblp << std::endl;
#endif

    // Sets costs for candidates of polygon

    if ( feat->feature->getGeosType() == GEOS_POLYGON )
    {
      int arrangement = feat->feature->getLayer()->getArrangement();
      if ( arrangement == P_FREE || arrangement == P_HORIZ )
        setPolygonCandidatesCost( stop, ( LabelPosition** ) feat->lPos, max_p, obstacles, bbx, bby );
    }

    // add size penalty (small lines/polygons get higher cost)
    feat->feature->addSizePenalty( max_p, feat->lPos, bbx, bby );

    return max_p;
  }



  //////////

  PolygonCostCalculator::PolygonCostCalculator( LabelPosition *lp ) : lp( lp )
  {
    px = ( lp->x[0] + lp->x[2] ) / 2.0;
    py = ( lp->y[0] + lp->y[2] ) / 2.0;

    dist = DBL_MAX;
    ok = false;
  }

  void PolygonCostCalculator::update( PointSet *pset )
  {
    double rx, ry;
    pset->getDist( px, py, &rx, &ry );
    double d = dist_euc2d_sq( px, py, rx, ry );
    if ( d < dist )
    {
      dist = d;
    }
  }

  LabelPosition* PolygonCostCalculator::getLabel()
  {
    return lp;
  }

  double PolygonCostCalculator::getCost()
  {
    return ( 4 * dist );
  }
}
