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

  class Feature;
  class Pal;
  class Label;
  class PriorityQueue;


  /**
   * \brief LabelPositon is a candidate feature label position
   */
  class LabelPosition
  {

      friend bool extractFeatCallback( Feature *ft_ptr, void*ctx );
      friend bool xGrow( void *l, void *r );
      friend bool yGrow( void *l, void *r );
      friend bool xShrink( void *l, void *r );
      friend bool yShrink( void *l, void *r );
      friend bool costShrink( void *l, void *r );
      friend bool costGrow( void *l, void *r );
      friend bool pruneLabelPositionCallback( LabelPosition *lp, void *ctx );
      //friend void setCost (int nblp, LabelPosition **lPos, int max_p, RTree<PointSet*, double, 2, double> *obstacles, double bbx[4], double bby[4]);
      friend bool countOverlapCallback( LabelPosition *lp, void *ctx );
      friend bool countFullOverlapCallback( LabelPosition *lp, void *ctx );
      friend bool removeOverlapCallback( LabelPosition *lp, void *ctx );
      friend bool falpCallback1( LabelPosition *lp, void * ctx );
      friend bool falpCallback2( LabelPosition *lp, void * ctx );
      friend bool subPartCallback( LabelPosition *lp, void *ctx );
      friend bool chainCallback( LabelPosition *lp, void *context );
      friend void ignoreLabel( LabelPosition*, PriorityQueue*, RTree<LabelPosition*, double, 2, double, 8, 4>* );
      friend bool obstacleCallback( PointSet *feat, void *ctx );

      friend bool updateCandidatesCost( LabelPosition *lp, void *context );
      friend bool nokCallback( LabelPosition*, void* );

      friend class Pal;
      friend class Problem;
      friend class Feature;
      friend double dist_pointToLabel( double, double, LabelPosition* );
    private:
      //LabelPosition **overlaped;
      //int nbOverlap;

      int id;
      double cost;
      //double workingCost;
      double x[4], y[4];

      double alpha;
      Feature *feature;

      // bug # 1 (maxence 10/23/2008)
      int probFeat;

      int nbOverlap;

      double w;
      double h;

      //LabelPosition (int id, double x1, double y1, double w, double h, double cost, Feature *feature);
      //LabelPosition (int id, int nbPart, double *x, double *y, double *alpha,

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
                     Feature *feature );

      /**
       * \brief load a stored labelposition
       *
       * Load a labelPosition from a file
       * \param id id of this labelPosition
       * \param feature  this labelposition is for feature
       * \param file load from this stream
       */
      LabelPosition( int id, Feature *feature, std::ifstream *file );

      /**
       * \brief Set cost to the smallest distance between lPos's centroid and a polygon stored in geoetry field
       */
      void setCostFromPolygon( RTree <PointSet*, double, 2, double> *obstacles, double bbx[4], double bby[4] );

      static void setCost( int nblp, LabelPosition **lPos, int max_p, RTree<PointSet*, double, 2, double> *obstacles, double bbx[4], double bby[4] );



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

      /** \brief return id
       * \return id
       */
      int getId();

      /** \brief return the feature id which the labelposition is
       * \return feature id
       */
      //int getFeatureId();


      /** \brief return the feature corresponding to this labelposition
       * \return the feature
       */
      Feature * getFeature();

      /**
       * \brief get the down-left x coordinate
       * \return x coordinate
       */
      double getX();
      /**
       * \brief get the down-left y coordinate
       * \return y coordinate
       */
      double getY();


      /**
       * \brief get alpha
       * \return alpha to rotate text (in rad)
       */
      double getAlpha();
      /**
       * \brief get the position geographical cost
       * \return geographical cost
       */
      double getCost();

      /**
       * \brief get a final lable from this
       * \return a new Label() object
       */
      Label* toLabel( bool active );

      void print();

      void removeFromIndex( RTree<LabelPosition*, double, 2, double> *index );
      void insertIntoIndex( RTree<LabelPosition*, double, 2, double> *index );

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
          double dist[8];
          double rpx[8];
          double rpy[8];
          bool ok[8];

          double dLp[3];

          void updatePoint( PointSet *pset );
          double updateLinePoly( PointSet *pset );
        public:
          PolygonCostCalculator( LabelPosition *lp );

          void update( PointSet *pset );

          double getCost();

          LabelPosition *getLabel();
      };



  };

  /**
   * \brief LabelPosition cmp
   *
   * \return true if l.id < r.id
   * \see Util::sort()
   */
  bool idGrow( void *l, void *r );
  /**
   * \brief LabelPosition cmp
   *
   * \return true if l.id < r.id
   * \see sort
   */
  bool xGrow( void *l, void *r );
  /**
   * \brief LabelPosition cmp
   *
   * \return true if l.id > r.id
   * \see sort
   */
  bool yGrow( void *l, void *r );
  /**
   * \brief LabelPosition cmp
   *
   * \return true if l.id < r.id
   * \see sort
   */
  bool xShrink( void *l, void *r );
  /**
   * \brief LabelPosition cmp
   *
   * \return true if l.id < r.id
   * \see sort
   */
  bool yShrink( void *l, void *r );
  /**
   * \brief LabelPosition cmp
   *
   * \return true if l.id < r.id
   * \see sort
   */
//bool nboGrow (void *l, void *r);


  bool costShrink( void *l, void *r );
  bool costGrow( void *l, void *r );


} // end namespac

#endif
