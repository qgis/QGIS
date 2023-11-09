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
#include "palrtree.h"

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
   * \brief Calculates label candidate costs considering different factors.
   */
  class CostCalculator
  {
    public:
      //! Increase candidate's cost according to its collision with passed feature
      static void addObstacleCostPenalty( pal::LabelPosition *lp, pal::FeaturePart *obstacle, Pal *pal );

      /**
       * Updates the costs for polygon label candidates by considering the distance between the
       * candidates and the nearest polygon ring (i.e. prefer labels closer to the pole of inaccessibility).
       */
      static void calculateCandidatePolygonRingDistanceCosts( std::vector<std::unique_ptr<pal::LabelPosition> > &lPos, double bbx[4], double bby[4] );

      /**
       * Updates the costs for polygon label candidates by considering the distance between the
       * candidates and the polygon centroid (i.e. given labels at similar distances from polygon rings,
       * prefer labels closer to the centroid).
       */
      static void calculateCandidatePolygonCentroidDistanceCosts( pal::FeaturePart *feature, std::vector<std::unique_ptr<pal::LabelPosition> > &lPos );

      //! Calculates the distance between a label candidate and the closest ring for a polygon feature
      static double calculatePolygonRingDistance( LabelPosition *candidate, double bbx[4], double bby[4] );

      //! Sort candidates by costs, skip the worse ones, evaluate polygon candidates
      static void finalizeCandidatesCosts( Feats *feat, double bbx[4], double bby[4] );

      /**
       * Sorts label candidates in ascending order of cost
       */
      static bool candidateSortGrow( const std::unique_ptr<pal::LabelPosition> &c1, const std::unique_ptr<pal::LabelPosition> &c2 );
  };

  /**
   * \ingroup core
   * \brief Calculates distance from a label candidate to nearest polygon ring.
   * \note not available in Python bindings
   * \since QGIS 3.12
   */
  class CandidatePolygonRingDistanceCalculator
  {

    public:

      /**
       * Constructor for PolygonRingDistanceCalculator, for the specified label \a candidate.
       */
      explicit CandidatePolygonRingDistanceCalculator( LabelPosition *candidate );

      /**
       * Adds a \a ring to the calculation, updating the minimumDistance() value if
       * the rings is closer to the candidate then previously added rings.
       */
      void addRing( const pal::PointSet *ring );

      /**
       * Returns the minimum distance between the candidate and all added rings.
       */
      double minimumDistance() const;

    private:

      double mPx;
      double mPy;
      double mMinDistance = std::numeric_limits<double>::max();
  };
}

#endif // COSTCALCULATOR_H
