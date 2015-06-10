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

#ifndef _POINTSET_H
#define _POINTSET_H

#include <cfloat>

#include <cmath>
#include <stddef.h>
#include <geos_c.h>

#include "rtree.hpp"
#include "linkedlist.hpp"

namespace pal
{

  class Pal;
  class Feature;
  class Projection;
  class LabelPosition;

  typedef struct _cross
  {
    int pt;
    double d;
    double x;
    double y;
    int seg;        // seg{0,1,2,3}
    int nextCorner; // pt{0,1,2,3}
    int way;

  } Crossing;

  class PointSet;

  typedef struct _cHullBox
  {
    double x[4];
    double y[4];

    double alpha;

    double width;
    double length;
  } CHullBox;



  inline bool ptrCrossingCompare( Crossing * a, Crossing * b )
  {
    return a == b;
  }

  inline bool crossingDist( void *a, void *b )
  {
    return (( Crossing* ) a )->d > (( Crossing* ) b )->d;
  }


  class CORE_EXPORT PointSet
  {
      friend class FeaturePart;
      friend class LabelPosition;
      friend class CostCalculator;
      friend class PolygonCostCalculator;
      friend class Layer;

    protected:
      int nbPoints;
      double *x;
      double *y;   // points order is counterclockwise

      int *cHull;
      int cHullSize;

      int type;

      PointSet* holeOf;
      PointSet* parent;

      PointSet( double x, double y );

      PointSet( PointSet &ps );

      void deleteCoords();

      double xmin;
      double xmax;
      double ymin;
      double ymax;

    public:
      PointSet();
      PointSet( int nbPoints, double *x, double *y );
      virtual ~PointSet();

      PointSet* extractShape( int nbPtSh, int imin, int imax, int fps, int fpe, double fptx, double fpty );

      PointSet* createProblemSpecificPointSet( double bbmin[2], double bbmax[2], bool *inside );

      CHullBox * compute_chull_bbox();


      /*
       * split a concave shape into several convex shapes
       *
       */
      static void splitPolygons( LinkedList<PointSet*> *shapes_toProcess,
                                 LinkedList<PointSet*> *shapes_final,
                                 double xrm, double yrm, char *uid );



      /**
       * \brief return the minimum distance bw this and the point (px,py)
       *
       * compute the minimum distance bw the point (px,py) and this.
       * Optionnaly, store the nearest point in (rx,ry)
       *
       * @param px x coordinate of the point
       * @param py y coordinate of the points
       * @param rx pointer to x coorinates of the nearest point (can be NULL)
       * @param ry pointer to y coorinates of the nearest point (can be NULL)
       */
      double getDist( double px, double py, double *rx, double *ry );



      //double getDistInside(double px, double py);

      void getCentroid( double &px, double &py, bool forceInside = false );


      int getGeosType() const { return type; }

      void getBoundingBox( double min[2], double max[2] ) const
      {
        min[0] = xmin; min[1] = ymin;
        max[0] = xmax; max[1] = ymax;
      }

      /** returns NULL if this isn't a hole. Otherwise returns pointer to parent pointset. */
      PointSet* getHoleOf() { return holeOf; }

      int getNumPoints() const { return nbPoints; }

      /*
       * Iterate on line by real step of dl on x,y points
       * @param nbPoint # point in line
       * @param x x coord
       * @param y y coord
       * @param d ??
       * @param ad distance from pt0 to each point (ad0 = pt0->pt0)
       * @param dl ??
       * @param px current x coord on line
       * @param py current y coord on line
       */
      inline void getPoint( double *d, double *ad, double dl,
                            double *px, double *py )
      {
        int i;
        double dx, dy, di;
        double distr;

        i = 0;
        if ( dl >= 0 )
        {
          while ( i < nbPoints && ad[i] <= dl ) i++;
          i--;
        }

        if ( i < nbPoints - 1 )
        {
          if ( dl < 0 )
          {
            dx = x[nbPoints-1] - x[0];
            dy = y[nbPoints-1] - y[0];
            di = sqrt( dx * dx + dy * dy );
          }
          else
          {
            dx = x[i+1] - x[i];
            dy = y[i+1] - y[i];
            di = d[i];
          }

          distr = dl - ad[i];
          *px = x[i] + dx * distr / di;
          *py = y[i] + dy * distr / di;
        }
        else    // just select last point...
        {
          *px = x[i];
          *py = y[i];
        }
      }
  };

} // namespace pal

#endif

