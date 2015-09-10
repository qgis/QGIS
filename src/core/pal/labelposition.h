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

#ifndef _LABELPOSITION_H
#define _LABELPOSITION_H

#include "pointset.h"
#include "rtree.hpp"
#include <fstream>

namespace pal
{

  class FeaturePart;
  class Pal;
  class Label;


  /**
   * \brief LabelPosition is a candidate feature label position
   */
  class CORE_EXPORT LabelPosition : public PointSet
  {
      friend class CostCalculator;
      friend class PolygonCostCalculator;

    public:

      /**
       * \brief Position of label candidate relative to feature.
       */
      enum Quadrant
      {
        QuadrantAboveLeft,
        QuadrantAbove,
        QuadrantAboveRight,
        QuadrantLeft,
        QuadrantOver,
        QuadrantRight,
        QuadrantBelowLeft,
        QuadrantBelow,
        QuadrantBelowRight
      };

      /**
       * \brief create a new LabelPosition
       *
       * \param id id of this labelposition
       * \param x1 down-left x coordinate
       * \param y1 down-left y coordinate
       * \param w label width
       * \param h label height
       * \param alpha rotation in rad
       * \param cost geographic cost
       * \param feature labelpos owners
       * \param isReversed label is reversed
       * \param quadrant relative position of label to feature
       */
      LabelPosition( int id, double x1, double y1,
                     double w, double h,
                     double alpha, double cost,
                     FeaturePart *feature, bool isReversed = false, Quadrant quadrant = QuadrantOver );

      /** Copy constructor */
      LabelPosition( const LabelPosition& other );

      ~LabelPosition() { delete nextPart; }

      /**
       * \brief Is the labelposition in the bounding-box ? (intersect or inside????)
       *
       *\param bbox the bounding-box double[4] = {xmin, ymin, xmax, ymax}
       */
      bool isIn( double *bbox );

      /**
       * \brief Is the labelposition intersect the bounding-box ?
       *
       *\param bbox the bounding-box double[4] = {xmin, ymin, xmax, ymax}
       */
      bool isIntersect( double *bbox );

      /**
       * \brief Is the labelposition inside the bounding-box ?
       *
       *\param bbox the bounding-box double[4] = {xmin, ymin, xmax, ymax}
       */
      bool isInside( double *bbox );

      /**
       * \brief Check whether or not this overlap with another labelPosition
       *
       * \param ls other labelposition
       * \return true or false
       */
      bool isInConflict( LabelPosition *ls );

      /** Return bounding box - amin: xmin,ymin - amax: xmax,ymax */
      void getBoundingBox( double amin[2], double amax[2] ) const;

      /** Get distance from this label to a point. If point lies inside, returns negative number. */
      double getDistanceToPoint( double xp, double yp ) const;

      /** Returns true if this label crosses the specified line */
      bool crossesLine( PointSet* line ) const;

      /** Returns true if this label crosses the boundary of the specified polygon */
      bool crossesBoundary( PointSet* polygon ) const;

      /** Returns cost of position intersection with polygon (testing area of intersection and center).
       * Cost ranges between 0 and 12, with extra cost if center of label position is covered.
       */
      int polygonIntersectionCost( PointSet* polygon ) const;

      /** Shift the label by specified offset */
      void offsetPosition( double xOffset, double yOffset );

      /** \brief return id
       * \return id
       */
      int getId() const;


      /** \brief return the feature corresponding to this labelposition
       * \return the feature
       */
      FeaturePart * getFeaturePart();

      double getNumOverlaps() const { return nbOverlap; }
      void resetNumOverlaps() { nbOverlap = 0; } // called from problem.cpp, pal.cpp

      int getProblemFeatureId() const { return probFeat; }
      /** Set problem feature ID and assigned label candidate ID.
       *  called from pal.cpp during extraction */
      void setProblemIds( int probFid, int lpId )
      {
        probFeat = probFid; id = lpId;
        if ( nextPart ) nextPart->setProblemIds( probFid, lpId );
      }

      /** Return pointer to layer's name. used for stats */
      QString getLayerName() const;

      /** Returns the candidate label position's geographical cost.
       * @see setCost
       */
      double cost() const { return mCost; }

      /** Sets the candidate label position's geographical cost.
       * @param newCost new cost for position
       * @see cost
      */
      void setCost( double newCost ) { mCost = newCost; }

      /** Sets whether the position is marked as conflicting with an obstacle feature.
       * @param conflicts set to true to mark candidate as being in conflict
       * @note This method applies to all label parts for the candidate position.
       * @see conflictsWithObstacle
       */
      void setConflictsWithObstacle( bool conflicts );

      /** Returns whether the position is marked as conflicting with an obstacle feature.
       * @see setConflictsWithObstacle
       */
      bool conflictsWithObstacle() const { return mHasObstacleConflict; }

      /** Make sure the cost is less than 1 */
      void validateCost();

      /**
       * \brief get the down-left x coordinate
       * \return x coordinate
       */
      double getX( int i = 0 ) const;
      /**
       * \brief get the down-left y coordinate
       * \return y coordinate
       */
      double getY( int i = 0 ) const;

      double getWidth() const { return w; }
      double getHeight() const { return h; }

      /**
       * \brief get alpha
       * \return alpha to rotate text (in rad)
       */
      double getAlpha() const;
      bool getReversed() const { return reversed; }
      bool getUpsideDown() const { return upsideDown; }

      Quadrant getQuadrant() const { return quadrant; }

      void print();

      LabelPosition* getNextPart() const { return nextPart; }
      void setNextPart( LabelPosition* next ) { nextPart = next; }

      // -1 if not multi-part
      int getPartId() const { return partId; }
      void setPartId( int id ) { partId = id; }


      void removeFromIndex( RTree<LabelPosition*, double, 2, double> *index );
      void insertIntoIndex( RTree<LabelPosition*, double, 2, double> *index );

      typedef struct
      {
        Pal* pal;
        FeaturePart *obstacle;
      } PruneCtx;

      /** Check whether the candidate in ctx overlap with obstacle feat */
      static bool pruneCallback( LabelPosition *lp, void *ctx );

      // for sorting
      static bool costShrink( void *l, void *r );
      static bool costGrow( void *l, void *r );

      // for counting number of overlaps
      typedef struct
      {
        LabelPosition *lp;
        int *nbOv;
        double *cost;
        double *inactiveCost;
        //int *feat;
      } CountContext;

      /*
       * count overlap, ctx = p_lp
       */
      static bool countOverlapCallback( LabelPosition *lp, void *ctx );

      static bool countFullOverlapCallback( LabelPosition *lp, void *ctx );

      static bool removeOverlapCallback( LabelPosition *lp, void *ctx );

      // for polygon cost calculation
      static bool polygonObstacleCallback( pal::FeaturePart *obstacle, void *ctx );

    protected:

      int id;

      FeaturePart *feature;

      // bug # 1 (maxence 10/23/2008)
      int probFeat;

      int nbOverlap;

      double alpha;
      double w;
      double h;

      LabelPosition* nextPart;
      int partId;

      //True if label direction is the same as line / polygon ring direction.
      //Could be used by the application to draw a directional arrow ('<' or '>')
      //if the layer arrangement is P_LINE
      bool reversed;

      bool upsideDown;

      LabelPosition::Quadrant quadrant;

      bool isInConflictSinglePart( LabelPosition* lp );
      bool isInConflictMultiPart( LabelPosition* lp );

    private:
      double mCost;
      bool mHasObstacleConflict;

      /** Calculates the total number of parts for this label position
       */
      int partCount() const;

      /** Calculates the polygon intersection cost for a single label position part
       * @returns double between 0 - 12
       */
      double polygonIntersectionCostForParts( PointSet* polygon ) const;

  };

} // end namespace

#endif
