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

#ifndef POINTSET_H
#define POINTSET_H

#define SIP_NO_FILE


#include <cfloat>
#include <cmath>
#include <QLinkedList>
#include <geos_c.h>
#include <memory>
#include <vector>

#include "qgis_core.h"
#include "qgsrectangle.h"
#include "qgsgeos.h"

namespace pal
{

  class Pal;
  class Projection;
  class LabelPosition;

  class PointSet;

  /**
   * Represents the minimum area, oriented bounding box surrounding a convex hull.
   */
  struct OrientedConvexHullBoundingBox
  {
    double x[4];
    double y[4];

    double alpha;

    double width;
    double length;
  };

  /**
   * \class pal::PointSet
   * \brief The underlying raw pal geometry class.
   * \note not available in Python bindings
   * \ingroup core
   */
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

      /**
       * Does... something completely inscrutable.
       */
      std::unique_ptr< PointSet > extractShape( int nbPtSh, int imin, int imax, int fps, int fpe, double fptx, double fpty );

      /**
       * Returns a copy of the point set.
       */
      std::unique_ptr< PointSet > clone() const;

      /**
       * Tests whether point set contains a specified point.
       * \param x x-coordinate of point
       * \param y y-coordinate of point
       * \returns TRUE if point set contains a specified point
       */
      bool containsPoint( double x, double y ) const;

      /**
       * Tests whether a possible label candidate will fit completely within the shape.
       * \param x x-coordinate of label candidate
       * \param y y-coordinate of label candidate
       * \param width label width
       * \param height label height
       * \param alpha label angle
       * \returns TRUE if point set completely contains candidate label
       */
      bool containsLabelCandidate( double x, double y, double width, double height, double alpha = 0 ) const;

      /**
       * Computes an oriented bounding box for the shape's convex hull.
       */
      OrientedConvexHullBoundingBox computeConvexHullOrientedBoundingBox( bool &ok );

      /**
       * Split a polygon using some random logic into some other polygons.
       *
       * \warning this code is completely unreadable and cannot be understood by mortals
       */
      static QLinkedList<PointSet *> splitPolygons( PointSet *inputShape, double labelWidth, double labelHeight );

      /**
       * Extends linestrings by the specified amount at the start and end of the line,
       * by extending the existing lines following the same direction as the original line
       * start or end.
       *
       * The \a smoothDistance argument specifies the distance over which to smooth the direction
       * of the line at its start and end points.
       */
      void extendLineByDistance( double startDistance, double endDistance, double smoothDistance );

      /**
       * Offsets linestrings by the specified \a distance.
       */
      void offsetCurveByDistance( double distance );

      /**
       * Returns the squared minimum distance between the point set geometry and the point (px,py)
       * Optionally, the nearest point is stored in (rx,ry).
       * \param px x coordinate of the point
       * \param py y coordinate of the points
       * \param rx pointer to x coorinates of the nearest point (can be NULL)
       * \param ry pointer to y coorinates of the nearest point (can be NULL)
       * \returns minimum distance
       */
      double minDistanceToPoint( double px, double py, double *rx = nullptr, double *ry = nullptr ) const;

      void getCentroid( double &px, double &py, bool forceInside = false ) const;

      int getGeosType() const { return type; }

      /**
       * Returns the point set bounding box.
       */
      QgsRectangle boundingBox() const
      {
        return QgsRectangle( xmin, ymin, xmax, ymax );
      }

      /**
       * Returns TRUE if the bounding box of this pointset intersects the bounding box
       * of another pointset.
       */
      bool boundingBoxIntersects( const PointSet *other ) const;

      //! Returns NULLPTR if this isn't a hole. Otherwise returns pointer to parent pointset.
      PointSet *getHoleOf() const { return holeOf; }

      int getNumPoints() const { return nbPoints; }

      /**
       * Gets a point a set distance along a line geometry.
       * \param d array of distances between points
       * \param ad cumulative total distance from pt0 to each point (ad0 = pt0->pt0)
       * \param dl distance to traverse along line
       * \param px final x coord on line
       * \param py final y coord on line
      */
      void getPointByDistance( double *d, double *ad, double dl, double *px, double *py );

      /**
       * Returns a GEOS geometry representing the point interpolated on the shape by distance.
       */
      geos::unique_ptr interpolatePoint( double distance ) const;

      /**
       * Returns the distance along the geometry closest to the specified GEOS \a point.
       */
      double lineLocatePoint( const GEOSGeometry *point ) const;

      /**
       * Returns the point set's GEOS geometry.
      */
      const GEOSGeometry *geos() const;

      /**
       * Returns length of line geometry.
       */
      double length() const;

      /**
       * Returns area of polygon geometry.
       */
      double area() const;

      /**
       * Returns TRUE if pointset is closed.
       */
      bool isClosed() const;

      /**
       * Returns a WKT representation of the point set.
       */
      QString toWkt() const;

      /**
       * Returns a vector of edge distances as well as its total length
       */
      std::tuple< std::vector< double >, double > edgeDistances() const;

      int nbPoints;
      std::vector< double > x;
      std::vector< double > y;   // points order is counterclockwise

    protected:
      mutable GEOSGeometry *mGeos = nullptr;
      mutable bool mOwnsGeom = false;

      std::vector< int > convexHull;

      int type;

      PointSet *holeOf = nullptr;
      PointSet *parent = nullptr;

      mutable double mArea = -1;
      mutable double mLength = -1;


      PointSet( double x, double y );

      PointSet( const PointSet &ps );

      void deleteCoords();
      void createGeosGeom() const;
      const GEOSPreparedGeometry *preparedGeom() const;

      void invalidateGeos();

      double xmin = std::numeric_limits<double>::max();
      double xmax = std::numeric_limits<double>::lowest();
      double ymin = std::numeric_limits<double>::max();
      double ymax = std::numeric_limits<double>::lowest();

    private:

      mutable const GEOSPreparedGeometry *mGeosPreparedBoundary = nullptr;
      mutable const GEOSPreparedGeometry *mPreparedGeom = nullptr;

      mutable GEOSGeometry *mMultipartGeos = nullptr;
      mutable const GEOSPreparedGeometry *mMultipartPreparedGeos = nullptr;

      PointSet &operator= ( const PointSet & ) = delete;

  };

} // namespace pal

#endif

