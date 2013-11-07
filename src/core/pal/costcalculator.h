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

#include "rtree.hpp"

namespace pal
{
  class Feats;

  class CostCalculator
  {
    public:
      /** increase candidate's cost according to its collision with passed feature */
      static void addObstacleCostPenalty( LabelPosition* lp, PointSet* feat );

      static void setPolygonCandidatesCost( int nblp, LabelPosition **lPos, int max_p, RTree<PointSet*, double, 2, double> *obstacles, double bbx[4], double bby[4] );

      /** Set cost to the smallest distance between lPos's centroid and a polygon stored in geoetry field */
      static void setCandidateCostFromPolygon( LabelPosition* lp, RTree <PointSet*, double, 2, double> *obstacles, double bbx[4], double bby[4] );

      /** sort candidates by costs, skip the worse ones, evaluate polygon candidates */
      static int finalizeCandidatesCosts( Feats* feat, int max_p, RTree <PointSet*, double, 2, double> *obstacles, double bbx[4], double bby[4] );
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
      LabelPosition *lp;
      double px, py;
      double dist;
      bool ok;

    public:
      PolygonCostCalculator( LabelPosition *lp );

      void update( PointSet *pset );

      double getCost();

      LabelPosition *getLabel();
  };
}

#endif // COSTCALCULATOR_H
