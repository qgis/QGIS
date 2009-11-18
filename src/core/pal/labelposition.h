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

#ifndef _LABELPOSITION_H
#define _LABELPOSITION_H

#include <fstream>

#include "rtree.hpp"


namespace pal
{

  class FeaturePart;
  class Pal;
  class Label;


  /**
   * \brief LabelPositon is a candidate feature label position
   */
  class CORE_EXPORT LabelPosition
  {
      friend class CostCalculator;
      friend class PolygonCostCalculator;

    protected:

      int id;
      double cost;
      FeaturePart *feature;

      // bug # 1 (maxence 10/23/2008)
      int probFeat;

      int nbOverlap;

      double x[4], y[4];
      double alpha;
      double w;
      double h;

      LabelPosition* nextPart;
      int partId;

      bool isInConflictSinglePart( LabelPosition* lp );
      bool isInConflictMultiPart( LabelPosition* lp );

    public:
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
       */
      LabelPosition( int id, double x1, double y1,
                     double w, double h,
                     double alpha, double cost,
                     FeaturePart *feature );

      /** copy constructor */
      LabelPosition( const LabelPosition& other );

      ~LabelPosition() { delete nextPart; }


      /**
       * \brief is the labelposition in the bounding-box ?
       *
       *\param bbox the bounding-box double[4] = {xmin, ymin, xmax, ymax}
       */
      bool isIn( double *bbox );

      /**
       * \brief Check whether or not this overlap with another labelPosition
       *
       * \param ls other labelposition
       * \return true or false
       */
      bool isInConflict( LabelPosition *ls );

      /** return bounding box - amin: xmin,ymin - amax: xmax,ymax */
      void getBoundingBox( double amin[2], double amax[2] ) const;

      /** get distance from this label to a point. If point lies inside, returns negative number. */
      double getDistanceToPoint( double xp, double yp );

      /** returns true if this label crosses the specified line */
      bool isBorderCrossingLine( PointSet* feat );

      /** returns number of intersections with polygon (testing border and center) */
      int getNumPointsInPolygon( int npol, double *xp, double *yp );

      /** shift the label by specified offset */
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
      /** set problem feature ID and assigned label candidate ID.
       *  called from pal.cpp during extraction */
      void setProblemIds( int probFid, int lpId )
      {
        probFeat = probFid; id = lpId;
        if ( nextPart ) nextPart->setProblemIds( probFid, lpId );
      }

      /** return pointer to layer's name. used for stats */
      char* getLayerName() const;

      /**
       * \brief get the position geographical cost
       * \return geographical cost
       */
      double getCost() const;

      /** Modify candidate's cost */
      void setCost( double newCost ) { cost = newCost; }

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
        double scale;
        Pal* pal;
        PointSet *obstacle;
      } PruneCtx;

      /** Check wheter the candidate in ctx overlap with obstacle feat */
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
      static bool polygonObstacleCallback( PointSet *feat, void *ctx );

  };

} // end namespac

#endif
