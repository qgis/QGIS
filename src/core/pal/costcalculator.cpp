
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
    /*unit_convert( double( lp->feature->distlabel ),
                                     pal::PIXEL,
                                     pal->map_unit,
                                     pal->dpi, scale, 1 );*/

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
    std::cout << "Nblabel kept for feat " << feat->feature->uid << "/" << feat->feature->layer->name << ": " << max_p << "/" << feat->nblp << std::endl;
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
    int i;
    double hyp = max( lp->feature->xmax - lp->feature->xmin, lp->feature->ymax - lp->feature->ymin );
    hyp *= 10;

    px = ( lp->x[0] + lp->x[2] ) / 2.0;
    py = ( lp->y[0] + lp->y[2] ) / 2.0;

    /*
               3  2  1
                \ | /
              4 --x -- 0
                / | \
               5  6  7
    */

    double alpha = lp->getAlpha();
    for ( i = 0; i < 8; i++, alpha += M_PI / 4 )
    {
      dist[i] = DBL_MAX;
      ok[i] = false;
      rpx[i] = px + cos( alpha ) * hyp;
      rpy[i] = py + sin( alpha ) * hyp;
    }
  }

  void PolygonCostCalculator::update( PointSet *pset )
  {
    if ( pset->type == GEOS_POINT )
    {
      updatePoint( pset );
    }
    else
    {
      double rx, ry;
      if ( pset->getDist( px, py, &rx, &ry ) < updateLinePoly( pset ) )
      {
        PointSet *point = new PointSet( ry, ry );
        update( point );
        delete point;
      }
    }
  }

  void PolygonCostCalculator::updatePoint( PointSet *pset )
  {
    double beta = atan2( pset->y[0] - py, pset->x[0] - px ) - lp->getAlpha();

    while ( beta < 0 )
    {
      beta += 2 * M_PI;
    }

    double a45 = M_PI / 4;

    int i = ( int )( beta / a45 );

    for ( int j = 0; j < 2; j++, i = ( i + 1 ) % 8 )
    {
      double rx, ry;
      rx = px - rpy[i] + py;
      ry = py + rpx[i] - px;
      double ix, iy; // the point that we look for
      if ( computeLineIntersection( px, py, rpx[i], rpy[i], pset->x[0], pset->y[0], rx, ry, &ix, &iy ) )
      {
        double d = dist_euc2d_sq( px, py, ix, iy );
        if ( d < dist[i] )
        {
          dist[i] = d;
          ok[i] = true;
        }
      }
      else
      {
        std::cout << "this shouldn't occurs !!!" << std::endl;
      }
    }
  }

  double PolygonCostCalculator::updateLinePoly( PointSet *pset )
  {
    int i, j, k;
    int nbP = ( pset->type == GEOS_POLYGON ? pset->nbPoints : pset->nbPoints - 1 );
    double min_dist = DBL_MAX;

    for ( i = 0; i < nbP; i++ )
    {
      j = ( i + 1 ) % pset->nbPoints;

      for ( k = 0; k < 8; k++ )
      {
        double ix, iy;
        if ( computeSegIntersection( px, py, rpx[k], rpy[k], pset->x[i], pset->y[i], pset->x[j], pset->y[j], &ix, &iy ) )
        {
          double d = dist_euc2d_sq( px, py, ix, iy );
          if ( d < dist[k] )
          {
            dist[k] = d;
            ok[k] = true;
          }
          if ( d < min_dist )
          {
            min_dist = d;
          }
        }
      }
    }
    return min_dist;
  }

  LabelPosition* PolygonCostCalculator::getLabel()
  {
    return lp;
  }

  double PolygonCostCalculator::getCost()
  {
    int i;

    for ( i = 0; i < 8; i++ )
    {
      /*
      if ( i == 0 || i == 4 ) // horizontal directions
        dist[i] -= lp->w / 2;
      else if (i == 2 || i == 6 ) // vertical directions
        dist[i] -= lp->h / 2;
      else // other directions
        dist[i] -= ( lp->w / 2 ) / cos( M_PI / 4 );
      */

      if ( !ok[i] || dist[i] < EPSILON )
      {
        dist[i] = EPSILON;
      }
    }

    double a, b, c, d;

    a = min( dist[0], dist[4] );
    b = min( dist[1], dist[5] );
    c = min( dist[2], dist[6] );
    d = min( dist[3], dist[7] );

    //if (a!=EPSILON || b!=EPSILON || c!=EPSILON || d!=EPSILON)
    //  std::cout << "res " << (a*b*c*d) << "       " << a << " " << b << " " << c << " " << d << std::endl;
    return ( a*b*c*d );
  }

}
