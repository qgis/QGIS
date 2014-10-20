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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <qglobal.h>

#include "pointset.h"
#include "util.h"

#include <pal/pal.h>


#include "geomfunction.h"

namespace pal
{


  PointSet::PointSet()
  {
    nbPoints = cHullSize =  0;
    x = NULL;
    y = NULL;
    cHull = NULL;
    type = -1;
  }

  PointSet::PointSet( int nbPoints, double *x, double *y )
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

  PointSet::PointSet( double x, double y )
  {
    nbPoints = cHullSize =  1;
    this->x = new double[1];
    this->y = new double[1];
    this->x[0] = x;
    this->y[0] = y;

    cHull = NULL;
    parent = NULL;
    holeOf = NULL;

    type = GEOS_POINT;
  }

  PointSet::PointSet( PointSet &ps )
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
  }

  PointSet::~PointSet()
  {
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


  void PointSet::splitPolygons( LinkedList<PointSet*> *shapes_toProcess,
                                LinkedList<PointSet*> *shapes_final,
                                double xrm, double yrm, char *uid )
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

    while ( shapes_toProcess->size() > 0 )
    {
#ifdef _DEBUG_FULL_
      std::cout << "Shape popping()" << std::endl;
#endif
      shape = shapes_toProcess->pop_front();

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
            cp = vabs( cross_product( x[cHull[ihs]], y[cHull[ihs]],
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
        double isValid;
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
          if ( seg_length < EPSILON || vabs(( b = cross_product( ex, ey, fx, fy, x[retainedPt], y[retainedPt] ) / ( seg_length ) ) ) > ( seg_length / 2 ) )    // retainedPt is not fronting i->j
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
        int imax = ((( fps < retainedPt && fpe < retainedPt ) || ( fps > retainedPt && fpe > retainedPt ) ) ? min( fps, fpe ) : max( fps, fpe ) );

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
          shapes_final->push_back( shape );
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

          shapes_toProcess->push_back( newShape );

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
          shapes_toProcess->push_back( newShape );

          if ( shape->parent )
            delete shape;
        }
      }
      else
      {
#ifdef _DEBUG_FULL_
        std::cout << "Put shape into shapes_final" << std::endl;
#endif
        shapes_final->push_back( shape );
      }
      delete[] pts;
    }
  }


  PointSet* PointSet::createProblemSpecificPointSet( double bbmin[2], double bbmax[2], bool *inside )
  {
#ifdef _DEBUG_FULL_
    std::cout << "CreateProblemSpecific:" << std::endl;
#endif
    PointSet *shape = new PointSet();
    shape->x = new double[nbPoints];
    shape->y = new double[nbPoints];
    shape->nbPoints = nbPoints;
    shape->type = type;

    shape->xmin = xmin;
    shape->xmax = xmax;
    shape->ymin = ymin;
    shape->ymax = ymax;

    *inside = true;

    for ( int i = 0; i < nbPoints; i++ )
    {
      shape->x[i] = this->x[i];
      shape->y[i] = this->y[i];

      // check whether it's not outside
      if ( x[i] < bbmin[0] || x[i] > bbmax[0] || y[i] < bbmin[1] || y[i] > bbmax[1] )
        *inside = false;
    }

    shape->holeOf = NULL;
    shape->parent = NULL;

    return shape;
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

#if 0
  double PointSet::getDistInside( double px, double py )
  {

    double dist[8];
    double rpx[8];
    double rpy[8];
    bool ok[8];

    if ( !isPointInPolygon( nbPoints, x, y, px, py ) )
    {
      double d = getDist( px, py );
      //std::cout << "Outside : " << d << std::endl;
      if ( d < 0 )
      {
        d = -( d * d * d * d );
      }
      else
      {
        d = d * d * d * d;
      }
      return d;
    }

    int i;

    double width = ( xmax - xmin ) * 2;
    double height = ( ymax - ymin ) * 2;

    /*
               1  0   7
                \ | /
              2 --x -- 6
                / | \
              3   4  5
    */

    // Compute references points
    for ( i = 0; i < 8; i++ )
    {
      dist[i] = DBL_MAX;
      ok[i] = false;
      rpx[i] = px;
      rpy[i] = py;
    }

    rpx[0] += 0;
    rpy[0] += height;

    rpx[1] -= width;
    rpy[1] += height;

    rpx[2] -= width;
    rpy[2] += 0;

    rpx[3] -= width;
    rpy[3] -= height;

    rpx[4] += 0;
    rpy[4] -= height;

    rpx[5] += width;
    rpy[5] -= height;

    rpx[6] += width;
    rpy[6] += 0;

    rpx[7] += width;
    rpy[7] += height;

    int j, k;
    for ( i = 0; i < nbPoints; i++ )
    {
      j = ( i + 1 ) % nbPoints;

      for ( k = 0; k < 8; k++ )
      {
        double ix, iy;
        if ( computeSegIntersection( px, py, rpx[k], rpy[k], x[i], y[i], x[j], y[j], &ix, &iy ) )
        {
          double d = dist_euc2d_sq( px, py, ix, iy );
          if ( dist[k] > d )
          {
            dist[k] = d;
            ok[k] = true;
            //std::cout << "new dist for " << k << ": " << dist[k] << std::endl;
          }
        }
      }
    }

    double a, b, c, d;


    for ( i = 0; i < 8; i++ )
    {
      if ( !ok[i] )
      {
        std::cout << "ERROR!!!!!!!!!!!!!!!!!" << std::endl;
        dist[i] = 0;
      }
      //std::cout << "dist[" << i << "]: " << dist[i] << std::endl;
    }

    a = min( dist[0], dist[4] );
    b = min( dist[1], dist[5] );
    c = min( dist[2], dist[6] );
    d = min( dist[3], dist[7] );
    /*
        std::cout << "a: " << a << std::endl;
        std::cout << "b: " << b << std::endl;
        std::cout << "c: " << c << std::endl;
        std::cout << "d: " << d << std::endl;
      */
    //a = (a+b+c+d)/4.0;

    //a = min(a,b);
    //c = min(c,d);
    //return min(a,c);


    return ( a*b*c*d );
  }
#endif

  double PointSet::getDist( double px, double py, double *rx, double *ry )
  {
    if ( nbPoints == 1 || type == GEOS_POINT )
    {
      if ( rx && ry )
      {
        *rx = x[0];
        *ry = y[0];
      }
      return dist_euc2d_sq( x[0], y[0], px, py );
    }

    int a, b;
    int nbP = ( type == GEOS_POLYGON ? nbPoints : nbPoints - 1 );

    double best_dist = DBL_MAX;
    double d;

    for ( a = 0; a < nbP; a++ )
    {
      b = ( a + 1 ) % nbPoints;

      double px2, py2;
      px2 = px - y[b] + y[a];
      py2 = py + x[b] - x[a];
      double ix, iy;

      // (px,py)->(px2,py2) is a line perpendicular to a->b
      // Check the line p->p2 cross the segment a->b
      if ( computeLineSegIntersection( px, py, px2, py2,
                                       x[a], y[a], x[b], y[b],
                                       &ix, &iy ) )
      {
        d = dist_euc2d_sq( px, py, ix, iy );
      }
      else
      {
        double d1 = dist_euc2d_sq( x[a], y[a], px, py );
        double d2 = dist_euc2d_sq( x[b], y[b], px, py );
        if ( d1 < d2 )
        {
          d = d1;
          ix = x[a];
          iy = y[a];
        }
        else
        {
          d = d2;
          ix = x[b];
          iy = y[b];
        }
      }

      if ( d < best_dist )
      {
        best_dist = d;
        if ( rx && ry )
        {
          *rx = ix;
          *ry = iy;
        }
      }
    } // end for (a in nbPoints)

    return best_dist;
  }



  void PointSet::getCentroid( double &px, double &py, bool forceInside )
  {
    // for explanation see this page:
    // http://local.wasp.uwa.edu.au/~pbourke/geometry/polyarea/

    int i, j;
    double cx = 0, cy = 0, A = 0, tmp, sumx = 0, sumy = 0;
    for ( i = 0; i < nbPoints; i++ )
    {
      j = i + 1; if ( j == nbPoints ) j = 0;
      tmp = (( x[i] - x[0] ) * ( y[j] - y[0] ) - ( x[j] - x[0] ) * ( y[i] - y[0] ) );
      cx += ( x[i] + x[j] - 2 * x[0] ) * tmp;
      cy += ( y[i] + y[j] - 2 * y[0] ) * tmp;
      A += tmp;
      sumx += x[i];
      sumy += y[i];
    }

    if ( A == 0 )
    {
      px = sumx / nbPoints;
      py = sumy / nbPoints;
    }
    else
    {
      px = cx / ( 3 * A ) + x[0];
      py = cy / ( 3 * A ) + y[0];
    }

    // check if centroid inside in polygon
    if ( forceInside && !isPointInPolygon( nbPoints, x, y, px, py ) )
    {
      GEOSContextHandle_t geosctxt = geosContext();
      GEOSCoordSequence *coord = GEOSCoordSeq_create_r( geosctxt, nbPoints, 2 );

      for ( int i = 0; i < nbPoints; ++i )
      {
        GEOSCoordSeq_setX_r( geosctxt, coord, i, x[i] );
        GEOSCoordSeq_setY_r( geosctxt, coord, i, y[i] );
      }

      GEOSGeometry *geom = GEOSGeom_createPolygon_r( geosctxt, GEOSGeom_createLinearRing_r( geosctxt, coord ), 0, 0 );

      if ( geom )
      {
        GEOSGeometry *pointGeom = GEOSPointOnSurface_r( geosctxt, geom );

        if ( pointGeom )
        {
          const GEOSCoordSequence *coordSeq = GEOSGeom_getCoordSeq_r( geosctxt, pointGeom );
          GEOSCoordSeq_getX_r( geosctxt, coordSeq, 0, &px );
          GEOSCoordSeq_getY_r( geosctxt, coordSeq, 0, &py );

          GEOSGeom_destroy_r( geosctxt, pointGeom );
        }
        GEOSGeom_destroy_r( geosctxt, geom );
      }
    }
  }

} // end namespace

#endif
