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

#ifndef _POINT_SET_H_
#define _POINT_SET_H_

#if defined(_VERBOSE_) || (_DEBUG_) || (_DEBUG_FULL_)
#include <iostream>
#endif

#include "pointset.h"
#include "util.h"
#include "pal.h"
#include "geomfunction.h"
#include "qgsgeos.h"
#include "qgsmessagelog.h"
#include <qglobal.h>

namespace pal
{

  PointSet::PointSet()
      : mGeos( 0 )
      , mOwnsGeom( false )
      , holeOf( NULL )
      , parent( NULL )
      , xmin( DBL_MAX )
      , xmax( -DBL_MAX )
      , ymin( DBL_MAX )
      , ymax( -DBL_MAX )
      , mPreparedGeom( 0 )
  {
    nbPoints = cHullSize =  0;
    x = NULL;
    y = NULL;
    cHull = NULL;
    type = -1;
  }

  PointSet::PointSet( int nbPoints, double *x, double *y )
      : mGeos( 0 )
      , mOwnsGeom( false )
      , cHullSize( 0 )
      , holeOf( NULL )
      , parent( NULL )
      , xmin( DBL_MAX )
      , xmax( -DBL_MAX )
      , ymin( DBL_MAX )
      , ymax( -DBL_MAX )
      , mPreparedGeom( 0 )
  {
    this->nbPoints = nbPoints;
    this->x = new double[nbPoints];
    this->y = new double[nbPoints];
    int i;

    for ( i = 0; i < nbPoints; i++ )
    {
      this->x[i] = x[i];
      this->y[i] = y[i];
    }
    type = GEOS_POLYGON;
    cHull = NULL;
  }

  PointSet::PointSet( double aX, double aY )
      : mGeos( 0 )
      , mOwnsGeom( false )
      , xmin( aX )
      , xmax( aY )
      , ymin( aX )
      , ymax( aY )
      , mPreparedGeom( 0 )
  {
    nbPoints = cHullSize = 1;
    x = new double[1];
    y = new double[1];
    x[0] = aX;
    y[0] = aY;

    cHull = NULL;
    parent = NULL;
    holeOf = NULL;

    type = GEOS_POINT;
  }

  PointSet::PointSet( const PointSet &ps )
      : mGeos( 0 )
      , mOwnsGeom( false )
      , parent( 0 )
      , xmin( DBL_MAX )
      , xmax( -DBL_MAX )
      , ymin( DBL_MAX )
      , ymax( -DBL_MAX )
      , mPreparedGeom( 0 )
  {
    int i;

    nbPoints = ps.nbPoints;
    x = new double[nbPoints];
    y = new double[nbPoints];

    for ( i = 0; i < nbPoints; i++ )
    {
      x[i] = ps.x[i];
      y[i] = ps.y[i];
    }

    if ( ps.cHull )
    {
      cHullSize = ps.cHullSize;
      cHull = new int[cHullSize];
      for ( i = 0; i < cHullSize; i++ )
      {
        cHull[i] = ps.cHull[i];
      }
    }
    else
    {
      cHull = NULL;
      cHullSize = 0;
    }

    type = ps.type;

    holeOf = ps.holeOf;

    if ( ps.mGeos )
    {
      mGeos = GEOSGeom_clone_r( geosContext(), ps.mGeos );
      mOwnsGeom = true;
    }
  }

  void PointSet::createGeosGeom() const
  {
    GEOSContextHandle_t geosctxt = geosContext();

    bool needClose = false;
    if ( type == GEOS_POLYGON && ( x[0] != x[ nbPoints - 1] || y[0] != y[ nbPoints - 1 ] ) )
    {
      needClose = true;
    }

    GEOSCoordSequence *coord = GEOSCoordSeq_create_r( geosctxt, nbPoints + ( needClose ? 1 : 0 ), 2 );
    for ( int i = 0; i < nbPoints; ++i )
    {
      GEOSCoordSeq_setX_r( geosctxt, coord, i, x[i] );
      GEOSCoordSeq_setY_r( geosctxt, coord, i, y[i] );
    }

    //close ring if needed
    if ( needClose )
    {
      GEOSCoordSeq_setX_r( geosctxt, coord, nbPoints, x[0] );
      GEOSCoordSeq_setY_r( geosctxt, coord, nbPoints, y[0] );
    }

    switch ( type )
    {
      case GEOS_POLYGON:
        mGeos = GEOSGeom_createPolygon_r( geosctxt, GEOSGeom_createLinearRing_r( geosctxt, coord ), 0, 0 );
        break;

      case GEOS_LINESTRING:
        mGeos = GEOSGeom_createLineString_r( geosctxt, coord );
        break;

      case GEOS_POINT:
        mGeos = GEOSGeom_createPoint_r( geosctxt, coord );
        break;
    }

    mOwnsGeom = true;
  }

  const GEOSPreparedGeometry *PointSet::preparedGeom() const
  {
    if ( !mGeos )
      createGeosGeom();

    if ( !mPreparedGeom )
    {
      mPreparedGeom = GEOSPrepare_r( geosContext(), mGeos );
    }
    return mPreparedGeom;
  }

  void PointSet::invalidateGeos()
  {
    GEOSContextHandle_t geosctxt = geosContext();
    if ( mOwnsGeom ) // delete old geometry if we own it
      GEOSGeom_destroy_r( geosctxt, mGeos );
    GEOSPreparedGeom_destroy_r( geosctxt, mPreparedGeom );
    mOwnsGeom = false;
    mGeos = 0;
    mPreparedGeom = 0;
  }

  PointSet::~PointSet()
  {
    GEOSContextHandle_t geosctxt = geosContext();

    if ( mGeos && mOwnsGeom )
    {
      GEOSGeom_destroy_r( geosctxt, mGeos );
      mGeos = NULL;
    }
    GEOSPreparedGeom_destroy_r( geosctxt, mPreparedGeom );

    deleteCoords();

    if ( cHull )
      delete[] cHull;
  }

  void PointSet::deleteCoords()
  {
    if ( x )
      delete[] x;
    if ( y )
      delete[] y;
    x = NULL;
    y = NULL;
  }

  PointSet* PointSet::extractShape( int nbPtSh, int imin, int imax, int fps, int fpe, double fptx, double fpty )
  {

    int i, j;

    PointSet *newShape = new PointSet();

    newShape->type = GEOS_POLYGON;

    newShape->nbPoints = nbPtSh;

    newShape->x = new double[newShape->nbPoints];
    newShape->y = new double[newShape->nbPoints];
#ifdef _DEBUG_FULL_
    std::cout << "New shape: ";
#endif
    // new shape # 1 from imin to imax
    for ( j = 0, i = imin; i != ( imax + 1 ) % nbPoints; i = ( i + 1 ) % nbPoints, j++ )
    {
      newShape->x[j] = x[i];
      newShape->y[j] = y[i];
#ifdef _DEBUG_FULL_
      std::cout << x[i] << ";" << y[i] << std::endl;
#endif
    }
    // is the cutting point a new one ?
    if ( fps != fpe )
    {
      // yes => so add it
      newShape->x[j] = fptx;
      newShape->y[j] = fpty;
#ifdef _DEBUG_FULL_
      std::cout << fptx << ";" << fpty << std::endl;
#endif
    }

#ifdef _DEBUG_FULL_
    std::cout << "J = " << j << "/" << newShape->nbPoints << std::endl;
    std::cout << std::endl;
    std::cout << "This:    " << this << std::endl;
#endif

    return newShape;
  }

  bool PointSet::containsPoint( double x, double y ) const
  {
    GEOSContextHandle_t geosctxt = geosContext();
    try
    {
      GEOSCoordSequence* seq = GEOSCoordSeq_create_r( geosctxt, 1, 2 );
      GEOSCoordSeq_setX_r( geosctxt, seq, 0, x );
      GEOSCoordSeq_setY_r( geosctxt, seq, 0, y );
      GEOSGeometry* point = GEOSGeom_createPoint_r( geosctxt, seq );
      bool result = ( GEOSPreparedContains_r( geosctxt, preparedGeom(), point ) == 1 );
      GEOSGeom_destroy_r( geosctxt, point );

      return result;
    }
    catch ( GEOSException &e )
    {
      QgsMessageLog::logMessage( QObject::tr( "Exception: %1" ).arg( e.what() ), QObject::tr( "GEOS" ) );
      return false;
    }

  }

  bool PointSet::containsLabelCandidate( double x, double y, double width, double height, double alpha ) const
  {
    GEOSContextHandle_t geosctxt = geosContext();
    GEOSCoordSequence *coord = GEOSCoordSeq_create_r( geosctxt, 5, 2 );

    GEOSCoordSeq_setX_r( geosctxt, coord, 0, x );
    GEOSCoordSeq_setY_r( geosctxt, coord, 0, y );
    if ( !qgsDoubleNear( alpha, 0.0 ) )
    {
      double beta = alpha + ( M_PI / 2 );
      double dx1 = cos( alpha ) * width;
      double dy1 = sin( alpha ) * width;
      double dx2 = cos( beta ) * height;
      double dy2 = sin( beta ) * height;
      GEOSCoordSeq_setX_r( geosctxt, coord, 1, x  + dx1 );
      GEOSCoordSeq_setY_r( geosctxt, coord, 1, y + dy1 );
      GEOSCoordSeq_setX_r( geosctxt, coord, 2, x + dx1 + dx2 );
      GEOSCoordSeq_setY_r( geosctxt, coord, 2, y + dy1 + dy2 );
      GEOSCoordSeq_setX_r( geosctxt, coord, 3, x + dx2 );
      GEOSCoordSeq_setY_r( geosctxt, coord, 3, y + dy2 );
    }
    else
    {
      GEOSCoordSeq_setX_r( geosctxt, coord, 1, x + width );
      GEOSCoordSeq_setY_r( geosctxt, coord, 1, y );
      GEOSCoordSeq_setX_r( geosctxt, coord, 2, x + width );
      GEOSCoordSeq_setY_r( geosctxt, coord, 2, y + height );
      GEOSCoordSeq_setX_r( geosctxt, coord, 3, x );
      GEOSCoordSeq_setY_r( geosctxt, coord, 3, y + height );
    }
    //close ring
    GEOSCoordSeq_setX_r( geosctxt, coord, 4, x );
    GEOSCoordSeq_setY_r( geosctxt, coord, 4, y );

    try
    {
      GEOSGeometry* bboxGeos = GEOSGeom_createLinearRing_r( geosctxt, coord );
      bool result = ( GEOSPreparedContains_r( geosctxt, preparedGeom(), bboxGeos ) == 1 );
      GEOSGeom_destroy_r( geosctxt, bboxGeos );
      return result;
    }
    catch ( GEOSException &e )
    {
      QgsMessageLog::logMessage( QObject::tr( "Exception: %1" ).arg( e.what() ), QObject::tr( "GEOS" ) );
      return false;
    }
  }

  void PointSet::splitPolygons( QLinkedList<PointSet*> &shapes_toProcess,
                                QLinkedList<PointSet*> &shapes_final,
                                double xrm, double yrm, const QString& uid )
  {
#ifdef _DEBUG_
    std::cout << "splitPolygons: " << uid << std::endl;
#else
    Q_UNUSED( uid );
#endif
    int i, j;

    int nbp;
    double *x;
    double *y;

    int *pts;

    int *cHull;
    int cHullSize;

    double cp;
    double bestcp = 0;

    double bestArea = 0;
    double area;

    double base;
    double b, c;
    double s;

    int ihs;
    int ihn;

    int ips;
    int ipn;

    int holeS = -1;  // hole start and end points
    int holeE = -1;

    int retainedPt = -1;
    int pt = 0;

    double labelArea = xrm * yrm;

    PointSet *shape;

    while ( shapes_toProcess.size() > 0 )
    {
#ifdef _DEBUG_FULL_
      std::cout << "Shape popping()" << std::endl;
#endif
      shape = shapes_toProcess.takeFirst();

      x = shape->x;
      y = shape->y;
      nbp = shape->nbPoints;
      pts = new int[nbp];

#ifdef _DEBUG_FULL_
      std::cout << "nbp: " << nbp << std::endl;
      std::cout << " PtSet: ";
#endif

      for ( i = 0; i < nbp; i++ )
      {
        pts[i] = i;
#ifdef _DEBUG_FULL_
        std::cout << x[i] << ";" << y[i] << std::endl;
#endif
      }

#ifdef _DEBUG_FULL_
      std::cout << std::endl;
#endif

      // conpute convex hull
      shape->cHullSize = convexHullId( pts, x, y, nbp, shape->cHull );

      cHull = shape->cHull;
      cHullSize = shape->cHullSize;


#ifdef _DEBUG_FULL_
      std::cout << " CHull: ";
      for ( i = 0; i < cHullSize; i++ )
      {
        std::cout << cHull[i] << " ";
      }
      std::cout << std::endl;
#endif

      bestArea = 0;
      retainedPt = -1;

      // lookup for a hole
      for ( ihs = 0; ihs < cHullSize; ihs++ )
      {
        // ihs->ihn => cHull'seg
        ihn = ( ihs + 1 ) % cHullSize;

        ips = cHull[ihs];
        ipn = ( ips + 1 ) % nbp;
        if ( ipn != cHull[ihn] ) // next point on shape is not the next point on cHull => there is a hole here !
        {
          bestcp = 0;
          pt = -1;
          // lookup for the deepest point in the hole
          for ( i = ips; i != cHull[ihn]; i = ( i + 1 ) % nbp )
          {
            cp = qAbs( cross_product( x[cHull[ihs]], y[cHull[ihs]],
                                      x[cHull[ihn]], y[cHull[ihn]],
                                      x[i], y[i] ) );
            if ( cp - bestcp > EPSILON )
            {
              bestcp = cp;
              pt = i;
            }
          }

#ifdef _DEBUG_FULL_
          std::cout << "Deeper POint: " << pt << " between " << ips << " and " << cHull[ihn]  << std::endl;
#endif
          if ( pt  != -1 )
          {
            // compute the ihs->ihn->pt triangle's area
            base = dist_euc2d( x[cHull[ihs]], y[cHull[ihs]],
                               x[cHull[ihn]], y[cHull[ihn]] );

            b = dist_euc2d( x[cHull[ihs]], y[cHull[ihs]],
                            x[pt], y[pt] );

            c = dist_euc2d( x[cHull[ihn]], y[cHull[ihn]],
                            x[pt], y[pt] );

            s = ( base + b + c ) / 2; // s = half perimeter
            area = s * ( s - base ) * ( s - b ) * ( s - c );
            if ( area < 0 )
              area = -area;

            // retain the biggest area
            if ( area - bestArea > EPSILON )
            {
              bestArea = area;
              retainedPt = pt;
              holeS = ihs;
              holeE = ihn;
            }
          }
        }
      }

      // we have a hole, its area, and the deppest point in hole
      // we're going to find the second point to cup the shape
      // holeS = hole starting point
      // holeE = hole ending point
      // retainedPt = deppest point in hole
      // bestArea = area of triangle HoleS->holeE->retainedPoint
      bestArea = sqrt( bestArea );
      double cx, cy, dx, dy, ex, ey, fx, fy, seg_length, ptx = 0, pty = 0, fptx = 0, fpty = 0;
      int ps = -1, pe = -1, fps = -1, fpe = -1;
      if ( retainedPt >= 0 && bestArea > labelArea ) // there is a hole so we'll cut the shape in two new shape (only if hole area is bigger than twice labelArea)
      {
#ifdef _DEBUG_FULL_
        std::cout << "Trou: " << retainedPt << std::endl;
        std::cout << "Hole: " << cHull[holeS] << " -> " << cHull[holeE] << std::endl;
        std::cout << "iterate from " << ( cHull[holeE] + 1 ) % nbp << "   to " << ( cHull[holeS] - 1 + nbp ) % nbp << std::endl;
#endif
        c = DBL_MAX;


        // iterate on all shape points except points which are in the hole
        bool isValid;
        int k, l;
        for ( i = ( cHull[holeE] + 1 ) % nbp; i != ( cHull[holeS] - 1 + nbp ) % nbp; i = j )
        {
          j = ( i + 1 ) % nbp; // i->j is shape segment not in hole

          // compute distance between retainedPoint and segment
          // whether perpendicular distance (if retaindPoint is fronting segment i->j)
          // or distance between retainedPt and i or j (choose the nearest)
          seg_length = dist_euc2d( x[i], y[i], x[j], y[j] );
          cx = ( x[i] + x[j] ) / 2.0;
          cy = ( y[i] + y[j] ) / 2.0;
          dx = cy - y[i];
          dy = cx - x[i];

          ex = cx - dx;
          ey = cy + dy;
          fx = cx + dx;
          fy = cy - dy;
#ifdef _DEBUG_FULL_
          std::cout << "D: " << dx << " " << dy << std::endl;
          std::cout << "seg_length: " << seg_length << std::endl;
#endif
          if ( seg_length < EPSILON || qAbs(( b = cross_product( ex, ey, fx, fy, x[retainedPt], y[retainedPt] ) / ( seg_length ) ) ) > ( seg_length / 2 ) )    // retainedPt is not fronting i->j
          {
            if (( ex = dist_euc2d_sq( x[i], y[i], x[retainedPt], y[retainedPt] ) ) < ( ey = dist_euc2d_sq( x[j], y[j], x[retainedPt], y[retainedPt] ) ) )
            {
              b = ex;
              ps = i;
              pe = i;
            }
            else
            {
              b = ey;
              ps = j;
              pe = j;
            }
          }
          else   // point fronting i->j => compute pependicular distance  => create a new point
          {
            b = cross_product( x[i], y[i], x[j], y[j], x[retainedPt], y[retainedPt] ) / seg_length;
            b *= b;
            ps = i;
            pe = j;

            if ( !computeLineIntersection( x[i], y[i], x[j], y[j], x[retainedPt], y[retainedPt], x[retainedPt] - dx, y[retainedPt] + dy, &ptx, &pty ) )
            {
              std::cout << "Oups ... il devrait par tomber la..." << std::endl;
            }
#ifdef _DEBUG_FULL_
            std::cout << "intersection : " << x[i] << " " <<  y[i] << " " << x[j] << " " <<  y[j] << " " <<  x[retainedPt] << " " <<  y[retainedPt] << " " <<  x[retainedPt] - dx << " " <<  y[retainedPt] + dy << std::endl;
            std::cout << "   =>   " << ptx << ";" << pty << std::endl;
            std::cout << "   cxy>   " << cx << ";" << cy << std::endl;
            std::cout << "   dxy>   " << dx << ";" << dy << std::endl;
#endif
          }

          isValid = true;
          double pointX, pointY;
          if ( ps == pe )
          {
            pointX = x[pe];
            pointY = y[pe];
          }
          else
          {
            pointX = ptx;
            pointY = pty;
          }

          for ( k = cHull[holeS]; k != cHull[holeE]; k = ( k + 1 ) % nbp )
          {
            l = ( k + 1 ) % nbp;
            //std::cout << "test " << k << " " << l << std::endl;
            if ( isSegIntersects( x[retainedPt], y[retainedPt], pointX, pointY, x[k], y[k], x[l], y[l] ) )
            {
              isValid = false;
              //std::cout << "Invalid point" << pe << ps << std::endl;
              break;
            }
          }


          if ( isValid && b < c )
          {
            //std::cout << "new point: " << ps << " " << pe << std::endl;
            c = b;
            fps = ps;
            fpe = pe;
            fptx = ptx;
            fpty = pty;
          }
        }  // for point which are not in hole

#ifdef _DEBUG_FULL_
        std::cout << " cut from " << retainedPt << " to ";
        if ( fps == fpe )
          std::cout << "point " << fps << std::endl;
        else
        {
          std::cout << "new point (" << fptx << ";" << fpty << "     between " << fps << " and " << fpe << std::endl;
        }
#endif

        // we will cut the shapeu in two new shapes, one from [retainedPoint] to [newPoint] and one form [newPoint] to [retainedPoint]
        int imin = retainedPt;
        int imax = ((( fps < retainedPt && fpe < retainedPt ) || ( fps > retainedPt && fpe > retainedPt ) ) ? qMin( fps, fpe ) : qMax( fps, fpe ) );

        int nbPtSh1, nbPtSh2; // how many points in new shapes ?
        if ( imax > imin )
          nbPtSh1 = imax - imin + 1 + ( fpe != fps );
        else
          nbPtSh1 = imax + nbp - imin + 1 + ( fpe != fps );

        if (( imax == fps ? fpe : fps ) < imin )
          nbPtSh2 = imin - ( imax == fps ? fpe : fps ) + 1 + ( fpe != fps );
        else
          nbPtSh2 = imin + nbp - ( imax == fps ? fpe : fps ) + 1 + ( fpe != fps );


#ifdef _DEBUG_FULL_
        std::cout << "imin: " << imin << "    imax:" << imax << std::endl;
#endif
        if ( retainedPt == -1 || fps == -1 || fpe == -1 )
        {
#ifdef _DEBUG_
          std::cout << std::endl << "Failed to split feature !!! (uid=" << uid << ")" << std::endl;
#endif
          if ( shape->parent )
            delete shape;
        }
        // check for useless spliting
        else if ( imax == imin || nbPtSh1 <= 2 || nbPtSh2 <= 2 || nbPtSh1 == nbp  || nbPtSh2 == nbp )
        {
          shapes_final.append( shape );
        }
        else
        {

          PointSet *newShape = shape->extractShape( nbPtSh1, imin, imax, fps, fpe, fptx, fpty );

          if ( shape->parent )
            newShape->parent = shape->parent;
          else
            newShape->parent = shape;

#ifdef _DEBUG_FULL_
          int i = 0;
          std::cout << "push back:" <<  std::endl;
          for ( i = 0; i < newShape->nbPoints; i++ )
          {
            std::cout << newShape->x[i] << ";" << newShape->y[i] << std::endl;
          }
#endif

          shapes_toProcess.append( newShape );

          if ( imax == fps )
            imax = fpe;
          else
            imax = fps;

          newShape = shape->extractShape( nbPtSh2, imax, imin, fps, fpe, fptx, fpty );

          if ( shape->parent )
            newShape->parent = shape->parent;
          else
            newShape->parent = shape;

#ifdef _DEBUG_FULL_
          std::cout << "push back:" <<  std::endl;
          for ( i = 0; i < newShape->nbPoints; i++ )
          {
            std::cout << newShape->x[i] << ";" << newShape->y[i] << std::endl;
          }
#endif
          shapes_toProcess.append( newShape );

          if ( shape->parent )
            delete shape;
        }
      }
      else
      {
#ifdef _DEBUG_FULL_
        std::cout << "Put shape into shapes_final" << std::endl;
#endif
        shapes_final.append( shape );
      }
      delete[] pts;
    }
  }

  CHullBox * PointSet::compute_chull_bbox()
  {
    int i;
    int j;

    double bbox[4]; // xmin, ymin, xmax, ymax

    double alpha;
    int alpha_d;

    double alpha_seg;

    double dref;
    double d1, d2;

    double bb[16];   // {ax, ay, bx, by, cx, cy, dx, dy, ex, ey, fx, fy, gx, gy, hx, hy}}

    double cp;
    double best_cp;
    double distNearestPoint;

    double area;
    double width;
    double length;

    double best_area = DBL_MAX;
    double best_alpha = -1;
    double best_bb[16];
    double best_length = 0;
    double best_width = 0;


    bbox[0] =   DBL_MAX;
    bbox[1] =   DBL_MAX;
    bbox[2] = - DBL_MAX;
    bbox[3] = - DBL_MAX;

#ifdef _DEBUG_
    std::cout << "Compute_chull_bbox" << std::endl;
#endif

    for ( i = 0; i < cHullSize; i++ )
    {
#ifdef _DEBUG_FULL_
      std::cout << x[cHull[i]] << ";" << y[cHull[i]] << std::endl;
#endif
      if ( x[cHull[i]] < bbox[0] )
        bbox[0] = x[cHull[i]];

      if ( x[cHull[i]] > bbox[2] )
        bbox[2] = x[cHull[i]];

      if ( y[cHull[i]] < bbox[1] )
        bbox[1] = y[cHull[i]];

      if ( y[cHull[i]] > bbox[3] )
        bbox[3] = y[cHull[i]];
    }


    dref = bbox[2] - bbox[0];

    for ( alpha_d = 0; alpha_d < 90; alpha_d++ )
    {
      alpha = alpha_d *  M_PI / 180.0;
      d1 = cos( alpha ) * dref;
      d2 = sin( alpha ) * dref;

      bb[0]  = bbox[0];
      bb[1]  = bbox[3]; // ax, ay

      bb[4]  = bbox[0];
      bb[5]  = bbox[1]; // cx, cy

      bb[8]  = bbox[2];
      bb[9]  = bbox[1]; // ex, ey

      bb[12] = bbox[2];
      bb[13] = bbox[3]; // gx, gy


      bb[2]  = bb[0] + d1;
      bb[3]  = bb[1] + d2; // bx, by
      bb[6]  = bb[4] - d2;
      bb[7]  = bb[5] + d1; // dx, dy
      bb[10] = bb[8] - d1;
      bb[11] = bb[9] - d2; // fx, fy
      bb[14] = bb[12] + d2;
      bb[15] = bb[13] - d1; // hx, hy

      // adjust all points
      for ( i = 0; i < 16; i += 4 )
      {

        alpha_seg = (( i / 4 > 0 ? ( i / 4 ) - 1 : 3 ) ) * M_PI / 2 + alpha;

        best_cp = DBL_MAX;
        for ( j = 0; j < nbPoints; j++ )
        {
          cp = cross_product( bb[i+2], bb[i+3], bb[i], bb[i+1], x[cHull[j]], y[cHull[j]] );
          if ( cp < best_cp )
          {
            best_cp = cp;
          }
        }

        distNearestPoint = best_cp / dref;

        d1 = cos( alpha_seg ) * distNearestPoint;
        d2 = sin( alpha_seg ) * distNearestPoint;

        bb[i]   += d1; // x
        bb[i+1] += d2; // y
        bb[i+2] += d1; // x
        bb[i+3] += d2; // y
      }

      // compute and compare AREA
      width = cross_product( bb[6], bb[7], bb[4], bb[5], bb[12], bb[13] ) / dref;
      length = cross_product( bb[2], bb[3], bb[0], bb[1], bb[8], bb[9] ) / dref;

      area = width * length;

      if ( area < 0 )
        area *= -1;


      if ( best_area - area > EPSILON )
      {
        best_area = area;
        best_length = length;
        best_width = width;
        best_alpha = alpha;
        memcpy( best_bb, bb, sizeof( double ) *16 );
      }
    }

    // best bbox is defined

    CHullBox * finalBb = new CHullBox();

    for ( i = 0; i < 16; i = i + 4 )
    {
      computeLineIntersection( best_bb[i], best_bb[i+1], best_bb[i+2], best_bb[i+3],
                               best_bb[( i+4 ) %16], best_bb[( i+5 ) %16], best_bb[( i+6 ) %16], best_bb[( i+7 ) %16],
                               &finalBb->x[int ( i/4 )], &finalBb->y[int ( i/4 )] );
    }

    finalBb->alpha = best_alpha;
    finalBb->width = best_width;
    finalBb->length = best_length;

#ifdef _DEBUG_FULL_
    std::cout << "FINAL" << std::endl;
    std::cout << "Length : " << best_length << std::endl;
    std::cout << "Width : " << best_width << std::endl;
    std::cout << "Alpha: " << best_alpha << "    " << best_alpha*180 / M_PI << std::endl;
    for ( i = 0; i < 4; i++ )
    {
      std::cout << finalBb->x[0] << " " << finalBb->y[0] << " ";
    }
    std::cout << std::endl;
#endif

    return finalBb;
  }

  double PointSet::minDistanceToPoint( double px, double py, double *rx, double *ry ) const
  {
    if ( !mGeos )
      createGeosGeom();

    if ( !mGeos )
      return 0;

    GEOSContextHandle_t geosctxt = geosContext();
    try
    {
      GEOSCoordSequence *coord = GEOSCoordSeq_create_r( geosctxt, 1, 2 );
      GEOSCoordSeq_setX_r( geosctxt, coord, 0, px );
      GEOSCoordSeq_setY_r( geosctxt, coord, 0, py );
      GEOSGeometry* geosPt = GEOSGeom_createPoint_r( geosctxt, coord );

      int type = GEOSGeomTypeId_r( geosctxt, mGeos );
      const GEOSGeometry* extRing = 0;
      if ( type != GEOS_POLYGON )
      {
        extRing = mGeos;
      }
      else
      {
        //for polygons, we want distance to exterior ring (not an interior point)
        extRing = GEOSGetExteriorRing_r( geosctxt, mGeos );
      }
      GEOSCoordSequence *nearestCoord = GEOSNearestPoints_r( geosctxt, extRing, geosPt );
      double nx;
      double ny;
      ( void )GEOSCoordSeq_getX_r( geosctxt, nearestCoord, 0, &nx );
      ( void )GEOSCoordSeq_getY_r( geosctxt, nearestCoord, 0, &ny );
      GEOSGeom_destroy_r( geosctxt, geosPt );

      if ( rx )
        *rx = nx;
      if ( ry )
        *ry = ny;

      return dist_euc2d_sq( px, py, nx, ny );
    }
    catch ( GEOSException &e )
    {
      QgsMessageLog::logMessage( QObject::tr( "Exception: %1" ).arg( e.what() ), QObject::tr( "GEOS" ) );
      return 0;
    }
  }

  void PointSet::getCentroid( double &px, double &py, bool forceInside ) const
  {
    if ( !mGeos )
      createGeosGeom();

    if ( !mGeos )
      return;

    try
    {
      GEOSContextHandle_t geosctxt = geosContext();
      GEOSGeometry *centroidGeom = GEOSGetCentroid_r( geosctxt, mGeos );
      if ( centroidGeom )
      {
        const GEOSCoordSequence *coordSeq = GEOSGeom_getCoordSeq_r( geosctxt, centroidGeom );
        GEOSCoordSeq_getX_r( geosctxt, coordSeq, 0, &px );
        GEOSCoordSeq_getY_r( geosctxt, coordSeq, 0, &py );
      }

      // check if centroid inside in polygon
      if ( forceInside && !containsPoint( px, py ) )
      {
        GEOSGeometry *pointGeom = GEOSPointOnSurface_r( geosctxt, mGeos );

        if ( pointGeom )
        {
          const GEOSCoordSequence *coordSeq = GEOSGeom_getCoordSeq_r( geosctxt, pointGeom );
          GEOSCoordSeq_getX_r( geosctxt, coordSeq, 0, &px );
          GEOSCoordSeq_getY_r( geosctxt, coordSeq, 0, &py );
          GEOSGeom_destroy_r( geosctxt, pointGeom );
        }
      }

      GEOSGeom_destroy_r( geosctxt, centroidGeom );
    }
    catch ( GEOSException &e )
    {
      QgsMessageLog::logMessage( QObject::tr( "Exception: %1" ).arg( e.what() ), QObject::tr( "GEOS" ) );
      return;
    }
  }

  void PointSet::getPointByDistance( double *d, double *ad, double dl, double *px, double *py )
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

  const GEOSGeometry *PointSet::geos() const
  {
    if ( !mGeos )
      createGeosGeom();

    return mGeos;
  }

  double PointSet::length() const
  {
    if ( !mGeos )
      createGeosGeom();

    if ( !mGeos )
      return -1;

    GEOSContextHandle_t geosctxt = geosContext();

    try
    {
      double len = 0;
      ( void )GEOSLength_r( geosctxt, mGeos, &len );
      return len;
    }
    catch ( GEOSException &e )
    {
      QgsMessageLog::logMessage( QObject::tr( "Exception: %1" ).arg( e.what() ), QObject::tr( "GEOS" ) );
      return -1;
    }
  }


} // end namespace

#endif
