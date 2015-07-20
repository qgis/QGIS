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

#include "geomfunction.h"
#include "feature.h"
#include "util.h"
#include "qgis.h"

namespace pal
{

  void heapsort( int *sid, int *id, const double* const x, int N )
  {
    unsigned int n = N, i = n / 2, parent, child;
    int tx;
    for ( ;; )
    {
      if ( i > 0 )
      {
        i--;
        tx = sid[i];
      }
      else
      {
        n--;
        if ( n == 0 ) return;
        tx = sid[n];
        sid[n] = sid[0];
      }
      parent = i;
      child = i * 2 + 1;
      while ( child < n )
      {
        if ( child + 1 < n  &&  x[id[sid[child + 1]]] > x[id[sid[child]]] )
        {
          child++;
        }
        if ( x[id[sid[child]]] > x[id[tx]] )
        {
          sid[parent] = sid[child];
          parent = child;
          child = parent * 2 + 1;
        }
        else
        {
          break;
        }
      }
      sid[parent] = tx;
    }
  }


  void heapsort2( int *x, double* heap, int N )
  {
    unsigned int n = N, i = n / 2, parent, child;
    double t;
    int tx;
    for ( ;; )
    {
      if ( i > 0 )
      {
        i--;
        t = heap[i];
        tx = x[i];
      }
      else
      {
        n--;
        if ( n == 0 ) return;
        t = heap[n];
        tx = x[n];
        heap[n] = heap[0];
        x[n] = x[0];
      }
      parent = i;
      child = i * 2 + 1;
      while ( child < n )
      {
        if ( child + 1 < n  &&  heap[child + 1] > heap[child] )
        {
          child++;
        }
        if ( heap[child] > t )
        {
          heap[parent] = heap[child];
          x[parent] = x[child];
          parent = child;
          child = parent * 2 + 1;
        }
        else
        {
          break;
        }
      }
      heap[parent] = t;
      x[parent] = tx;
    }
  }




  /*
   * \brief return true if the two seg intersect
   */

  bool isSegIntersects( double x1, double y1, double x2, double y2,  // 1st segment
                        double x3, double y3, double x4, double y4 )  // 2nd segment
  {
    /*
       std::cout << "SegInrersect ? " << std::endl;
       std::cout << "   cp1 : " << cross_product (x1, y1, x2, y2, x3, y3) << std::endl;
       std::cout << "   cp2 : " << cross_product (x1, y1, x2, y2, x4, y4) << std::endl;
       std::cout << "   cp3 : " << cross_product (x3, y3, x4, y4, x1, y1) << std::endl;
       std::cout << "   cp4 : " << cross_product (x3, y3, x4, y4, x2, y2) << std::endl;
    */
    return ( cross_product( x1, y1, x2, y2, x3, y3 ) * cross_product( x1, y1, x2, y2, x4, y4 ) < 0
             && cross_product( x3, y3, x4, y4, x1, y1 ) * cross_product( x3, y3, x4, y4, x2, y2 ) < 0 );
  }

  /*
   * \brief compute the point wherre two lines intersects
   * \return true if the ok false if line are parallel
   */
  bool computeLineIntersection( double x1, double y1, double x2, double y2,  // 1st line (segment)
                                double x3, double y3, double x4, double y4,  // 2nd line segment
                                double *x, double *y )
  {

    double a1, a2, b1, b2, c1, c2;
    double denom;

    a1 = y2 - y1;
    b1 = x1 - x2;
    c1 = x2 * y1 - x1 * y2;

    a2 = y4 - y3;
    b2 = x3 - x4;
    c2 = x4 * y3 - x3 * y4;

    if (( denom = a1 * b2 - a2 * b1 ) == 0 )
    {
      return false;
    }
    else
    {
      *x = ( b1 * c2 - b2 * c1 ) / denom;
      *y = ( a2 * c1 - a1 * c2 ) / denom;
    }

    return true;

  }



  /*
   * \brief Compute the convex hull in O(n·log(n))
   * \param id set of point (i.e. point no 0 is (x,y) = x[id[0]],y[id[0]])
   * \param x x coordinates
   * \param y y coordinates
   * \param n Size of subset (vector id)
   * \param cHull returns the point id (id of id's vector...) whom are parts of the convex hull
   * \return convexHull's size
   */
  int convexHullId( int *id, const double* const x, const double* const y, int n, int *&cHull )
  {
    int i;

    cHull = new int[n];
    for ( i = 0; i < n; i++ )
    {
      cHull[i] = i;
    }


    if ( n <= 3 ) return n;

    int* stack = new int[n];
    double* tan = new double [n];
    int ref;

    int second, top;
    double result;

    // find the lowest y value
    heapsort( cHull, id, y, n );

    // find the lowest x value from the lowest y
    ref = 1;
    while ( ref < n && qgsDoubleNear( y[id[cHull[ref]]], y[id[cHull[0]]] ) ) ref++;

    heapsort( cHull, id, x, ref );

    // the first point is now for sure in the hull as well as the ref one
    for ( i = ref; i < n; i++ )
    {
      if ( qgsDoubleNear( y[id[cHull[i]]], y[id[cHull[0]]] ) )
        tan[i] = FLT_MAX;
      else
        tan[i] = ( x[id[cHull[0]]] - x[id[cHull[i]]] ) / ( y[id[cHull[i]]] - y[id[cHull[0]]] );
    }

    if ( ref < n )
      heapsort2( cHull + ref, tan + ref, n - ref );

    // the second point is in too
    stack[0] = cHull[0];
    if ( ref == 1 )
    {
      stack[1] = cHull[1];
      ref++;
    }
    else
      stack[1] = cHull[ref-1];


    top = 1;
    second = 0;

    for ( i = ref; i < n; i++ )
    {
      result = cross_product( x[id[stack[second]]], y[id[stack[second]]],
                              x[id[stack[top]]], y[id[stack[top]]], x[id[cHull[i]]], y[id[cHull[i]]] );
      // Coolineaire !! garder le plus éloigné
      if ( qgsDoubleNear( result, 0.0 ) )
      {
        if ( dist_euc2d_sq( x[id[stack[second]]], y[id[stack[second]]], x[id[cHull[i]]], y[id[cHull[i]]] )
             >  dist_euc2d_sq( x[id[stack[second]]], y[id[stack[second]]], x[id[stack[top]]], y[id[stack[top]]] ) )
        {
          stack[top] = cHull[i];
        }
      }
      else if ( result > 0 ) //convexe
      {
        second++;
        top++;
        stack[top] = cHull[i];
      }
      else
      {
        while ( result < 0 && top > 1 )
        {
          second--;
          top--;
          result = cross_product( x[id[stack[second]]],
                                  y[id[stack[second]]], x[id[stack[top]]],
                                  y[id[stack[top]]], x[id[cHull[i]]], y[id[cHull[i]]] );
        }
        second++;
        top++;
        stack[top] = cHull[i];
      }
    }

    for ( i = 0; i <= top; i++ )
    {
      cHull[i] = stack[i];
    }

    delete[] stack;
    delete[] tan;

    return top + 1;
  }

// reorder points to have cross prod ((x,y)[i], (x,y)[i+1), point) > 0 when point is outside
  int reorderPolygon( int nbPoints, double *x, double *y )
  {
    int inc = 0;
    int *cHull;
    int cHullSize;
    int i;

    int *pts = new int[nbPoints];
    for ( i = 0; i < nbPoints; i++ )
      pts[i] = i;


    cHullSize = convexHullId( pts, x, y, nbPoints, cHull );

    if ( pts[cHull[0]] < pts[cHull[1]] && pts[cHull[1]] < pts[cHull[2]] )
      inc = 1;
    else if ( pts[cHull[0]] > pts[cHull[1]] && pts[cHull[1]] > pts[cHull[2]] )
      inc = -1;
    else if ( pts[cHull[0]] > pts[cHull[1]] && pts[cHull[1]] < pts[cHull[2]] && pts[cHull[2]] < pts[cHull[0]] )
      inc = 1;
    else if ( pts[cHull[0]] > pts[cHull[1]] && pts[cHull[1]] < pts[cHull[2]] && pts[cHull[2]] > pts[cHull[0]] )
      inc = -1;
    else if ( pts[cHull[0]] < pts[cHull[1]] && pts[cHull[1]] > pts[cHull[2]] && pts[cHull[2]] > pts[cHull[0]] )
      inc = -1;
    else if ( pts[cHull[0]] < pts[cHull[1]] && pts[cHull[1]] > pts[cHull[2]] && pts[cHull[2]] < pts[cHull[0]] )
      inc = 1;
    else
    {
      std::cout << "Warning wrong cHull -> geometry: " << nbPoints << std::endl;
      for ( i = 0; i < nbPoints; i++ )
      {
        std::cout << x[i] << ";" << y[i] << std::endl;
      }
      std::cout << "hull : " << cHullSize << std::endl;
      for ( i = 0; i < cHullSize; i++ )
      {
        std::cout << pts[cHull[i]] << " ";
      }
      std::cout << std::endl;
      delete[] cHull;
      delete[] pts;
      return -1;
    }

    if ( inc == -1 ) // re-order points
    {
      double tmp;
      int j;
      for ( i = 0, j = nbPoints - 1; i <= j; i++, j-- )
      {
        tmp = x[i];
        x[i] = x[j];
        x[j] = tmp;

        tmp = y[i];
        y[i] = y[j];
        y[j] = tmp;
      }
    }


    delete[] cHull;
    delete[] pts;

    return 0;

  }

  void findLineCircleIntersection( double cx, double cy, double radius,
                                   double x1, double y1, double x2, double y2,
                                   double& xRes, double& yRes )
  {
    double dx = x2 - x1;
    double dy = y2 - y1;

    double A = dx * dx + dy * dy;
    double B = 2 * ( dx * ( x1 - cx ) + dy * ( y1 - cy ) );
    double C = ( x1 - cx ) * ( x1 - cx ) + ( y1 - cy ) * ( y1 - cy ) - radius * radius;

    double det = B * B - 4 * A * C;
    if ( A <= 0.0000001 || det < 0 )
      // Should never happen, No real solutions.
      return;

    if ( det == 0 )
    {
      // Could potentially happen.... One solution.
      double t = -B / ( 2 * A );
      xRes = x1 + t * dx;
      yRes = y1 + t * dy;
    }
    else
    {
      // Two solutions.
      // Always use the 1st one
      // We only really have one solution here, as we know the line segment will start in the circle and end outside
      double t = ( -B + sqrt( det ) ) / ( 2 * A );
      xRes = x1 + t * dx;
      yRes = y1 + t * dy;
    }
  }


} // end namespace
