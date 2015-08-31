/***************************************************************************
    costcalculator.h
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
#ifndef COSTCALCULATOR_H
#define COSTCALCULATOR_H

#include <QList>
#include "rtree.hpp"

namespace pal
{
  class Feats;

  class CostCalculator
  {
    public:
      /** Increase candidate's cost according to its collision with passed feature */
      static void addObstacleCostPenalty( LabelPosition* lp, pal::FeaturePart *obstacle );

      static void setPolygonCandidatesCost( int nblp, QList< LabelPosition* >& lPos, int max_p, RTree<pal::FeaturePart*, double, 2, double> *obstacles, double bbx[4], double bby[4] );

      /** Set cost to the smallest distance between lPos's centroid and a polygon stored in geoetry field */
      static void setCandidateCostFromPolygon( LabelPosition* lp, RTree<pal::FeaturePart *, double, 2, double> *obstacles, double bbx[4], double bby[4] );

      /** Sort candidates by costs, skip the worse ones, evaluate polygon candidates */
      static int finalizeCandidatesCosts( Feats* feat, int max_p, RTree<pal::FeaturePart *, double, 2, double> *obstacles, double bbx[4], double bby[4] );

      /** Sorts label candidates in ascending order of cost
       */
      static bool candidateSortGrow( const LabelPosition *c1, const LabelPosition *c2 );

      /** Sorts label candidates in descending order of cost
       */
      static bool candidateSortShrink( const LabelPosition *c1, const LabelPosition *c2 );
  };

  /**
   * \brief Data structure to compute polygon's candidates costs
   *
   *  eight segment from center of candidat to (rpx,rpy) points (0°, 45°, 90°, ..., 315°)
   *  dist store the shortest square distance from the center to an object
   *  ok[i] is the to true whether the corresponding dist[i] is set
   */
  class PolygonCostCalculator
  {

    public:
      PolygonCostCalculator( LabelPosition *lp );

      void update( pal::PointSet *pset );

      double getCost();

      LabelPosition *getLabel();

    private:

      LabelPosition *lp;
      double px, py;
      double dist;
      bool ok;
  };
}

#endif // COSTCALCULATOR_H
