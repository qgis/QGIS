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

#define SIP_NO_FILE

#include <QList>
#include "rtree.hpp"

/**
 * \class pal::CostCalculator
 * \note not available in Python bindings
 */

namespace pal
{
  class Feats;
  class LabelPosition;
  class Pal;

  /**
   * \ingroup core
   */
  class CostCalculator
  {
    public:
      //! Increase candidate's cost according to its collision with passed feature
      static void addObstacleCostPenalty( LabelPosition *lp, pal::FeaturePart *obstacle, Pal *pal );

      //! Calculates the costs for polygon label candidates
      static void setPolygonCandidatesCost( std::size_t nblp, std::vector<std::unique_ptr<pal::LabelPosition> > &lPos, RTree<pal::FeaturePart *, double, 2, double> *obstacles, double bbx[4], double bby[4] );

      //! Sets cost to the smallest distance between lPos's centroid and a polygon stored in geometry field
      static void setCandidateCostFromPolygon( LabelPosition *lp, RTree<pal::FeaturePart *, double, 2, double> *obstacles, double bbx[4], double bby[4] );

      //! Sort candidates by costs, skip the worse ones, evaluate polygon candidates
      static std::size_t finalizeCandidatesCosts( Feats *feat, std::size_t max_p, RTree<pal::FeaturePart *, double, 2, double> *obstacles, double bbx[4], double bby[4] );

      /**
       * Sorts label candidates in ascending order of cost
       */
      static bool candidateSortGrow( const std::unique_ptr<pal::LabelPosition> &c1, const std::unique_ptr<pal::LabelPosition> &c2 );

      /**
       * Sorts label candidates in descending order of cost
       */
      static bool candidateSortShrink( const std::unique_ptr<pal::LabelPosition> &c1, const std::unique_ptr<pal::LabelPosition> &c2 );
  };

  /**
   * \ingroup core
   * \brief Data structure to compute polygon's candidates costs
   *
   *  Eight segments from center of candidate to (rpx,rpy) points (0°, 45°, 90°, ..., 315°)
   *  dist store the shortest square distance from the center to an object
   *  ok[i] is the to TRUE whether the corresponding dist[i] is set
   *
   * \note not available in Python bindings
   */
  class PolygonCostCalculator
  {

    public:
      explicit PolygonCostCalculator( LabelPosition *lp );

      void update( pal::PointSet *pset );

      double getCost();

      LabelPosition *getLabel();

    private:

      LabelPosition *lp = nullptr;
      double px, py;
      double dist;
      bool ok;
  };
}

#endif // COSTCALCULATOR_H
