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

#ifndef LABELPOSITION_H
#define LABELPOSITION_H

#define SIP_NO_FILE


#include "qgis_core.h"
#include "pointset.h"
#include "rtree.hpp"
#include <fstream>

namespace pal
{

  class FeaturePart;
  class Pal;
  class Label;


  /**
   * \ingroup core
   * \brief LabelPosition is a candidate feature label position
   * \class pal::LabelPosition
   * \note not available in Python bindings
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

      //! Copy constructor
      LabelPosition( const LabelPosition &other );

      ~LabelPosition() override { delete nextPart; }

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
       * Returns TRUE if the label position intersects a \a geometry.
       */
      bool intersects( const GEOSPreparedGeometry *geometry );

      /**
       * Returns TRUE if the label position is within a \a geometry.
       */
      bool within( const GEOSPreparedGeometry *geometry );

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
       * \returns TRUE or FALSE
       */
      bool isInConflict( LabelPosition *ls );

      //! Returns bounding box - amin: xmin,ymin - amax: xmax,ymax
      void getBoundingBox( double amin[2], double amax[2] ) const;

      //! Gets distance from this label to a point. If point lies inside, returns negative number.
      double getDistanceToPoint( double xp, double yp ) const;

      //! Returns TRUE if this label crosses the specified line
      bool crossesLine( PointSet *line ) const;

      //! Returns TRUE if this label crosses the boundary of the specified polygon
      bool crossesBoundary( PointSet *polygon ) const;

      /**
       * Returns cost of position intersection with polygon (testing area of intersection and center).
       * Cost ranges between 0 and 12, with extra cost if center of label position is covered.
       */
      int polygonIntersectionCost( PointSet *polygon ) const;

      /**
       * Returns TRUE if any intersection between polygon and position exists.
      */
      bool intersectsWithPolygon( PointSet *polygon ) const;

      //! Shift the label by specified offset
      void offsetPosition( double xOffset, double yOffset );

      /**
       * Returns the id
       */
      int getId() const;


      /**
       * Returns the feature corresponding to this labelposition
       */
      FeaturePart *getFeaturePart();

      int getNumOverlaps() const { return nbOverlap; }
      void resetNumOverlaps() { nbOverlap = 0; } // called from problem.cpp, pal.cpp

      int getProblemFeatureId() const { return probFeat; }

      /**
       * Set problem feature ID and assigned label candidate ID.
       *  called from pal.cpp during extraction */
      void setProblemIds( int probFid, int lpId )
      {
        probFeat = probFid;
        id = lpId;
        if ( nextPart ) nextPart->setProblemIds( probFid, lpId );
      }

      /**
       * Returns the candidate label position's geographical cost.
       * \see setCost
       */
      double cost() const { return mCost; }

      /**
       * Sets the candidate label position's geographical cost.
       * \param newCost new cost for position
       * \see cost
      */
      void setCost( double newCost ) { mCost = newCost; }

      /**
       * Sets whether the position is marked as conflicting with an obstacle feature.
       * \param conflicts set to TRUE to mark candidate as being in conflict
       * \note This method applies to all label parts for the candidate position.
       * \see conflictsWithObstacle
       */
      void setConflictsWithObstacle( bool conflicts );

      /**
       * Returns whether the position is marked as conflicting with an obstacle feature.
       * \see setConflictsWithObstacle
       */
      bool conflictsWithObstacle() const { return mHasObstacleConflict; }

      //! Make sure the cost is less than 1
      void validateCost();

      /**
       * Returns the down-left x coordinate.
       * \see getY()
       */
      double getX( int i = 0 ) const;

      /**
       * Returns the down-left y coordinate.
       * \see getX()
       */
      double getY( int i = 0 ) const;

      double getWidth() const { return w; }
      double getHeight() const { return h; }

      /**
       * Returns the angle to rotate text (in rad).
       */
      double getAlpha() const;
      bool getReversed() const { return reversed; }
      bool getUpsideDown() const { return upsideDown; }

      Quadrant getQuadrant() const { return quadrant; }
      LabelPosition *getNextPart() const { return nextPart; }
      void setNextPart( LabelPosition *next ) { nextPart = next; }

      // -1 if not multi-part
      int getPartId() const { return partId; }
      void setPartId( int id ) { partId = id; }

      //! Increases the count of upside down characters for this label position
      int incrementUpsideDownCharCount() { return ++mUpsideDownCharCount; }

      //! Returns the number of upside down characters for this label position
      int upsideDownCharCount() const { return mUpsideDownCharCount; }

      void removeFromIndex( RTree<LabelPosition *, double, 2, double> *index );
      void insertIntoIndex( RTree<LabelPosition *, double, 2, double> *index );

      typedef struct
      {
        Pal *pal = nullptr;
        FeaturePart *obstacle = nullptr;
      } PruneCtx;

      //! Check whether the candidate in ctx overlap with obstacle feat
      static bool pruneCallback( LabelPosition *candidatePosition, void *ctx );

      // for counting number of overlaps
      typedef struct
      {
        LabelPosition *lp = nullptr;
        int *nbOv = nullptr;
        double *cost = nullptr;
        double *inactiveCost = nullptr;
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

      FeaturePart *feature = nullptr;

      // bug # 1 (maxence 10/23/2008)
      int probFeat;

      int nbOverlap;

      double alpha;
      double w;
      double h;

      LabelPosition *nextPart = nullptr;
      int partId;

      //True if label direction is the same as line / polygon ring direction.
      //Could be used by the application to draw a directional arrow ('<' or '>')
      //if the layer arrangement is P_LINE
      bool reversed;

      bool upsideDown;

      LabelPosition::Quadrant quadrant;

      bool isInConflictSinglePart( LabelPosition *lp );
      bool isInConflictMultiPart( LabelPosition *lp );

    private:
      double mCost;
      bool mHasObstacleConflict;
      int mUpsideDownCharCount;

      /**
       * Calculates the total number of parts for this label position
       */
      int partCount() const;

      /**
       * Calculates the polygon intersection cost for a single label position part
       * \returns double between 0 - 12
       */
      double polygonIntersectionCostForParts( PointSet *polygon ) const;

  };

} // end namespace

#endif
