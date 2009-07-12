/*
 *   libpal - Automated Placement of Labels Library
 *
 *   Copyright (C) 2008 Maxence Laurent, MIS-TIC, HEIG-VD
 *                      University of Applied Sciences, Western Switzerland
 *                      http://www.hes-so.ch
 *
 *   Contact:
 *      maxence.laurent <at> heig-vd <dot> ch
 *    or
 *      eric.taillard <at> heig-vd <dot> ch
 *
 * This file is part of libpal.
 *
 * libpal is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * libpal is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with libpal.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#define _CRT_SECURE_NO_DEPRECATE

#include <iostream>
#include <fstream>
#include <cmath>
#include <cstring>
#include <cfloat>

#include <pal/layer.h>
#include <pal/pal.h>
#include <pal/label.h>

#include "costcalculator.h"
#include "feature.h"
#include "geomfunction.h"
#include "labelposition.h"

#ifndef M_PI
#define M_PI 3.1415926535897931159979634685
#endif


namespace pal
{

  LabelPosition::LabelPosition( int id, double x1, double y1, double w, double h, double alpha, double cost, Feature *feature ) : id( id ), cost( cost ), /*workingCost (0),*/ alpha( alpha ), feature( feature ), nbOverlap( 0 ), w( w ), h( h )
  {


    // alpha take his value bw 0 and 2*pi rad
    while ( this->alpha > 2*M_PI )
      this->alpha -= 2 * M_PI;

    while ( this->alpha < 0 )
      this->alpha += 2 * M_PI;

    register double beta = this->alpha + ( M_PI / 2 );

    double dx1, dx2, dy1, dy2;

    double tx, ty;

    dx1 = cos( this->alpha ) * w;
    dy1 = sin( this->alpha ) * w;

    dx2 = cos( beta ) * h;
    dy2 = sin( beta ) * h;

    x[0] = x1;
    y[0] = y1;

    x[1] = x1 + dx1;
    y[1] = y1 + dy1;

    x[2] = x1 + dx1 + dx2;
    y[2] = y1 + dy1 + dy2;

    x[3] = x1 + dx2;
    y[3] = y1 + dy2;

    // upside down ?
    if ( this->alpha > M_PI / 2 && this->alpha <= 3*M_PI / 2 )
    {
      tx = x[0];
      ty = y[0];

      x[0] = x[2];
      y[0] = y[2];

      x[2] = tx;
      y[2] = ty;

      tx = x[1];
      ty = y[1];

      x[1] = x[3];
      y[1] = y[3];

      x[3] = tx;
      y[3] = ty;

      if ( this->alpha < M_PI )
        this->alpha += M_PI;
      else
        this->alpha -= M_PI;
    }
  }

  bool LabelPosition::isIn( double *bbox )
  {
    int i;

    for ( i = 0;i < 4;i++ )
    {
      if ( x[i] >= bbox[0] && x[i] <= bbox[2] &&
           y[i] >= bbox[1] && y[i] <= bbox[3] )
        return true;
    }

    return false;

  }

  void LabelPosition::print()
  {
    std::cout << feature->getLayer()->getName() << "/" << feature->getUID() << "/" << id;
    std::cout << " cost: " << cost;
    std::cout << " alpha" << alpha << std::endl;
    std::cout << x[0] << ", " << y[0] << std::endl;
    std::cout << x[1] << ", " << y[1] << std::endl;
    std::cout << x[2] << ", " << y[2] << std::endl;
    std::cout << x[3] << ", " << y[3] << std::endl;
    std::cout << std::endl;
  }

  bool LabelPosition::isInConflict( LabelPosition *ls )
  {
    int i, i2, j;
    int d1, d2;

    if ( this->probFeat == ls->probFeat ) // bugfix #1
      return false; // always overlaping itself !

    double cp1, cp2;


    //std::cout << "Check intersect" << std::endl;
    for ( i = 0;i < 4;i++ )
    {
      i2 = ( i + 1 ) % 4;
      d1 = -1;
      d2 = -1;
      //std::cout << "new seg..." << std::endl;
      for ( j = 0;j < 4;j++ )
      {
        cp1 = cross_product( x[i], y[i], x[i2], y[i2], ls->x[j], ls->y[j] );
        if ( cp1 > 0 )
        {
          d1 = 1;
          //std::cout << "    cp1: " << cp1 << std::endl;
        }
        cp2 = cross_product( ls->x[i], ls->y[i],
                             ls->x[i2], ls->y[i2],
                             x[j], y[j] );

        if ( cp2 > 0 )
        {
          d2 = 1;
          //std::cout << "     cp2 " << cp2 << std::endl;
        }
      }

      if ( d1 == -1 || d2 == -1 ) // disjoint
        return false;
    }
    return true;
  }

  int LabelPosition::getId() const
  {
    return id;
  }

  double LabelPosition::getX() const
  {
    return x[0];
  }

  double LabelPosition::getY() const
  {
    return y[0];
  }

  double LabelPosition::getAlpha() const
  {
    return alpha;
  }

  double LabelPosition::getCost() const
  {
    return cost;
  }

  void LabelPosition::validateCost()
  {
    if ( cost >= 1 )
    {
      std::cout << " Warning: lp->cost == " << cost << " (from feat: " << feature->getUID() << "/" << getLayerName() << ")" << std::endl;
      cost -= int ( cost ); // label cost up to 1
    }
  }

  Feature * LabelPosition::getFeature()
  {
    return feature;
  }

  void LabelPosition::getBoundingBox(double amin[2], double amax[2]) const
  {
    amin[0] = DBL_MAX;
    amax[0] = -DBL_MAX;
    amin[1] = DBL_MAX;
    amax[1] = -DBL_MAX;
    for ( int c = 0;c < 4;c++ )
    {
      if ( x[c] < amin[0] )
        amin[0] = x[c];
      if ( x[c] > amax[0] )
        amax[0] = x[c];
      if ( y[c] < amin[1] )
        amin[1] = y[c];
      if ( y[c] > amax[1] )
        amax[1] = y[c];
    }
  }

  char* LabelPosition::getLayerName() const
  {
    return feature->getLayer()->name;
  }

  bool LabelPosition::costShrink( void *l, void *r )
  {
    return (( LabelPosition* ) l )->cost < (( LabelPosition* ) r )->cost;
  }

  bool LabelPosition::costGrow( void *l, void *r )
  {
    return (( LabelPosition* ) l )->cost > (( LabelPosition* ) r )->cost;
  }


  Label *LabelPosition::toLabel( bool active )
  {
    return new Label( this->x, this->y, alpha, feature->getUID(), feature->getLayer()->getName(), feature->getUserGeometry() );
  }


  bool LabelPosition::polygonObstacleCallback( PointSet *feat, void *ctx )
  {
    PolygonCostCalculator *pCost = ( PolygonCostCalculator* ) ctx;

    LabelPosition *lp = pCost->getLabel();
    if (( feat == lp->feature ) || ( feat->getHoleOf() && feat->getHoleOf() != lp->feature ) )
    {
      return true;
    }

    // if the feature is not a hole we have to fetch corrdinates
    // otherwise holes coordinates are still in memory (feature->selfObs)
    if ( feat->getHoleOf() == NULL )
    {
      (( Feature* ) feat )->fetchCoordinates();
    }

    pCost->update( feat );


    if ( feat->getHoleOf() == NULL )
    {
      (( Feature* ) feat )->releaseCoordinates();
    }


    return true;
  }


  void LabelPosition::removeFromIndex( RTree<LabelPosition*, double, 2, double> *index )
  {
    double amin[2];
    double amax[2];
    getBoundingBox(amin, amax);
    index->Remove( amin, amax, this );
  }


  void LabelPosition::insertIntoIndex( RTree<LabelPosition*, double, 2, double> *index )
  {
    double amin[2];
    double amax[2];
    getBoundingBox(amin, amax);
    index->Insert( amin, amax, this );
  }


  //////////

  bool LabelPosition::pruneCallback( LabelPosition *lp, void *ctx )
  {
    PointSet *feat = (( PruneCtx* ) ctx )->obstacle;
    double scale = (( PruneCtx* ) ctx )->scale;
    Pal* pal = (( PruneCtx* ) ctx )->pal;

    if (( feat == lp->feature ) || ( feat->getHoleOf() && feat->getHoleOf() != lp->feature ) )
    {
      return true;
    }

    CostCalculator::addObstacleCostPenalty(lp, feat);

    return true;
  }


  bool LabelPosition::countOverlapCallback( LabelPosition *lp, void *ctx )
  {
    LabelPosition *lp2 = ( LabelPosition* ) ctx;

    if ( lp2->isInConflict( lp ) )
    {
      lp2->nbOverlap++;
    }

    return true;
  }

  bool LabelPosition::countFullOverlapCallback( LabelPosition *lp, void *ctx )
  {
    LabelPosition *lp2 = (( CountContext* ) ctx )->lp;
    double *cost = (( CountContext* ) ctx )->cost;
    //int *feat = ((CountContext*)ctx)->feat;
    int *nbOv = (( CountContext* ) ctx )->nbOv;
    double *inactiveCost = (( CountContext* ) ctx )->inactiveCost;
    if ( lp2->isInConflict( lp ) )
    {
#ifdef _DEBUG_FULL_
      std::cout <<  "count overlap : " << lp->id << "<->" << lp2->id << std::endl;
#endif
      ( *nbOv ) ++;
      *cost += inactiveCost[lp->probFeat] + lp->getCost();

    }

    return true;
  }


  bool LabelPosition::removeOverlapCallback( LabelPosition *lp, void *ctx )
  {
    LabelPosition *lp2 = ( LabelPosition * ) ctx;

    if ( lp2->isInConflict( lp ) )
    {
      //std::cout << "   hit !" << std::endl;
      lp->nbOverlap--;
      lp2->nbOverlap--;
    }

    return true;
  }



  double LabelPosition::getDistanceToPoint( double xp, double yp )
  {
    int i;
    int j;

    double mx[4];
    double my[4];

    double dist_min = DBL_MAX;
    double dist;

    for ( i = 0;i < 4;i++ )
    {
      j = ( i + 1 ) % 4;
      mx[i] = ( x[i] + x[j] ) / 2.0;
      my[i] = ( y[i] + y[j] ) / 2.0;
    }

    if ( vabs( cross_product( mx[0], my[0], mx[2], my[2], xp, yp ) / h ) < w / 2 )
    {
      dist = cross_product( x[1], y[1], x[0], y[0], xp, yp ) / w;
      if ( vabs( dist ) < vabs( dist_min ) )
        dist_min = dist;

      dist = cross_product( x[3], y[3], x[2], y[2], xp, yp ) / w;
      if ( vabs( dist ) < vabs( dist_min ) )
        dist_min = dist;
    }

    if ( vabs( cross_product( mx[1], my[1], mx[3], my[3], xp, yp ) / w ) < h / 2 )
    {
      dist = cross_product( x[2], y[2], x[1], y[1], xp, yp ) / h;
      if ( vabs( dist ) < vabs( dist_min ) )
        dist_min = dist;

      dist = cross_product( x[0], y[0], x[3], y[3], xp, yp ) / h;
      if ( vabs( dist ) < vabs( dist_min ) )
        dist_min = dist;
    }

    for ( i = 0;i < 4;i++ )
    {
      dist = dist_euc2d( x[i], y[i], xp, yp );
      if ( vabs( dist ) < vabs( dist_min ) )
        dist_min = dist;
    }

    return dist_min;
  }


  bool LabelPosition::isBorderCrossingLine( PointSet* feat )
  {
    double ca, cb;
    for ( int i = 0;i < 4;i++ )
    {
      for ( int j = 0;j < feat->getNumPoints() - 1;j++ )
      {
        ca = cross_product( x[i], y[i], x[( i+1 ) %4], y[( i+1 ) %4],
                            feat->x[j], feat->y[j] );
        cb = cross_product( x[i], y[i], x[( i+1 ) %4], y[( i+1 ) %4],
                            feat->x[j+1], feat->y[j+1] );

        if (( ca < 0 && cb > 0 ) || ( ca > 0 && cb < 0 ) )
        {
          ca = cross_product( feat->x[j], feat->y[j], feat->x[j+1], feat->y[j+1],
                              x[i], y[i] );
          cb = cross_product( feat->x[j], feat->y[j], feat->x[j+1], feat->y[j+1],
                              x[( i+1 ) %4], y[( i+1 ) %4] );
          if (( ca < 0 && cb > 0 ) || ( ca > 0 && cb < 0 ) )
            return true;
        }
      }
    }
    return false;
  }

  int LabelPosition::getNumPointsInPolygon( int npol, double *xp, double *yp )
  {
    int a, k, count = 0;
    double px, py;

    // cheack each corner
    for ( k = 0;k < 4;k++ )
    {
      px = x[k];
      py = y[k];

      for ( a = 0;a < 2;a++ ) // and each middle of segment
      {
        if ( isPointInPolygon( npol, xp, yp, px, py ) )
          count++;
        px = ( x[k] + x[( k+1 ) %4] ) / 2.0;
        py = ( y[k] + y[( k+1 ) %4] ) / 2.0;
      }
    }

    px = ( x[0] + x[2] ) / 2.0;
    py = ( y[0] + y[2] ) / 2.0;

    // and the label center
    if ( isPointInPolygon( npol, xp, yp, px, py ) )
      count += 4; // virtually 4 points

    return count;
  }

} // end namespace

