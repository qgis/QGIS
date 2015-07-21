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

#ifndef _POINTSET_H
#define _POINTSET_H

#include "qgsgeometry.h"
#include "rtree.hpp"
#include <cfloat>
#include <cmath>
#include <QLinkedList>

namespace pal
{

  class Pal;
  class Feature;
  class Projection;
  class LabelPosition;

  class PointSet;

  typedef struct _cHullBox
  {
    double x[4];
    double y[4];

    double alpha;

    double width;
    double length;
  } CHullBox;


  class CORE_EXPORT PointSet
  {
      friend class FeaturePart;
      friend class LabelPosition;
      friend class CostCalculator;
      friend class PolygonCostCalculator;
      friend class Layer;

    public:
      PointSet();
      PointSet( int nbPoints, double *x, double *y );
      virtual ~PointSet();

      PointSet* extractShape( int nbPtSh, int imin, int imax, int fps, int fpe, double fptx, double fpty );

      /** Tests whether point set contains a specified point.
       * @param x x-coordinate of point
       * @param y y-coordinate of point
       * @returns true if point set contains a specified point
       */
      bool containsPoint( double x, double y ) const;

      /** Tests whether a possible label candidate will fit completely within the shape.
       * @param x x-coordinate of label candidate
       * @param y y-coordinate of label candidate
       * @param width label width
       * @param height label height
       * @param alpha label angle
       * @returns true if point set completely contains candidate label
       */
      bool containsLabelCandidate( double x, double y, double width, double height, double alpha = 0 ) const;

      CHullBox * compute_chull_bbox();

      /** Split a concave shape into several convex shapes.
       */
      static void splitPolygons( QLinkedList<PointSet *> &shapes_toProcess,
                                 QLinkedList<PointSet *> &shapes_final,
                                 double xrm, double yrm, const QString &uid );

      /** Returns the squared minimum distance between the point set geometry and the point (px,py)
       * Optionally, the nearest point is stored in (rx,ry).
       * @param px x coordinate of the point
       * @param py y coordinate of the points
       * @param rx pointer to x coorinates of the nearest point (can be NULL)
       * @param ry pointer to y coorinates of the nearest point (can be NULL)
       * @returns minimum distance
       */
      double minDistanceToPoint( double px, double py, double *rx = 0, double *ry = 0 ) const;

      void getCentroid( double &px, double &py, bool forceInside = false ) const;

      int getGeosType() const { return type; }

      void getBoundingBox( double min[2], double max[2] ) const
      {
        min[0] = xmin; min[1] = ymin;
        max[0] = xmax; max[1] = ymax;
      }

      /** Returns NULL if this isn't a hole. Otherwise returns pointer to parent pointset. */
      PointSet* getHoleOf() { return holeOf; }

      int getNumPoints() const { return nbPoints; }

      /** Get a point a set distance along a line geometry.
       * @param distance distance to traverse along line
       * @param px final x coord on line
       * @param py final y coord on line
      */
      void getPointByDistance( double distance, double *px, double *py ) const;

      /** Returns the point set's GEOS geometry.
      */
      const GEOSGeometry* geos() const;

      /** Returns length of line geometry.
       */
      double length() const;

    protected:
      mutable GEOSGeometry *mGeos;
      mutable bool mOwnsGeom;

      int nbPoints;
      double *x;
      double *y;   // points order is counterclockwise

      int *cHull;
      int cHullSize;

      int type;

      PointSet* holeOf;
      PointSet* parent;

      PointSet( double x, double y );

      PointSet( const PointSet &ps );

      void deleteCoords();
      void createGeosGeom() const;
      const GEOSPreparedGeometry* preparedGeom() const;
      void invalidateGeos();

      double xmin;
      double xmax;
      double ymin;
      double ymax;

    private:

      mutable const GEOSPreparedGeometry* mPreparedGeom;

  };

} // namespace pal

#endif

