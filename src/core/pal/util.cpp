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

#include <stddef.h>
#include <geos_c.h>

#include <sstream>

#include <iostream>
#include <cfloat>
//#include <cfloat>
#include <cstdarg>
#include <ctime>

#include <pal/layer.h>

#include "internalexception.h"
#include "util.h"
#include "labelposition.h"
#include "feature.h"
#include "geomfunction.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#ifndef M_PI_2
#define M_PI_2 1.57079632679489661923
#endif

#ifndef M_SQRT_2
#define M_SQRT_2 0.707106781186547524401
#endif

#ifndef M_SQRT2
#define M_SQRT2 1.41421356237309504880
#endif

namespace pal
{

  void sort( double* heap, int* x, int* y, int N )
  {
    unsigned int n = N, i = n / 2, parent, child;
    double t;
    int tx;
    int ty;
    for ( ;; )
    {
      if ( i > 0 )
      {
        i--;
        t = heap[i];
        tx = x[i];
        ty = y[i];
      }
      else
      {
        n--;
        if ( n == 0 ) return;
        t = heap[n];
        tx = x[n];
        ty = y[n];
        heap[n] = heap[0];
        x[n] = x[0];
        y[n] = y[0];
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
          y[parent] = y[child];
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
      y[parent] = ty;
    }
  }

  void tabcpy( int n, const int* const x, const int* const y,
               const double* const prob, int *cx, int *cy, double *p )
  {
    int i;

    for ( i = 0; i < n; i++ )
    {
      cx[i] = x[i];
      cy[i] = y[i];
      p[i] = prob[i];
    }
  }


  void sort( void** items, int N, bool ( *greater )( void *l, void *r ) )
  {

    if ( N <= 0 )
      return;

    unsigned int n = ( unsigned int ) N, i = n / 2, parent, child;

    void *t = NULL;

    for ( ;; )
    {
      if ( i > 0 )
      {
        i--;
        t = items[i];
      }
      else
      {
        n--;
        if ( n == 0 ) return;
        t = items[n];
        items[n] = items[0];
      }
      parent = i;
      child = i * 2 + 1;
      while ( child < n )
      {
        if ( child + 1 < n  &&  greater( items[child + 1], items[child] ) )
        {
          child++;
        }
        if ( greater( items[child], t ) )
        {
          items[parent] = items[child];
          parent = child;
          child = parent * 2 + 1;
        }
        else
        {
          break;
        }
      }
      items[parent] = t;
    }
  }




//inline bool ptrGeomEq (const geos::geom::Geometry *l, const geos::geom::Geometry *r){
  inline bool ptrGeomEq( const GEOSGeometry *l, const GEOSGeometry *r )
  {
    return l == r;
  }

//LinkedList<const geos::geom::Geometry*> * unmulti (geos::geom::Geometry *the_geom){
  LinkedList<const GEOSGeometry*> * unmulti( GEOSGeometry *the_geom )
  {

    //LinkedList<const geos::geom::Geometry*> *queue = new  LinkedList<const geos::geom::Geometry*>(ptrGeomEq);
    //LinkedList<const geos::geom::Geometry*> *final_queue = new  LinkedList<const geos::geom::Geometry*>(ptrGeomEq);
    LinkedList<const GEOSGeometry*> *queue = new  LinkedList<const GEOSGeometry*> ( ptrGeomEq );
    LinkedList<const GEOSGeometry*> *final_queue = new  LinkedList<const GEOSGeometry*> ( ptrGeomEq );

    //const geos::geom::Geometry *geom;
    const GEOSGeometry *geom;

    queue->push_back( the_geom );
    int nGeom;
    int i;

    while ( queue->size() > 0 )
    {
      geom = queue->pop_front();
      switch ( GEOSGeomTypeId( geom ) )
      {
          //case geos::geom::GEOS_MULTIPOINT:
          //case geos::geom::GEOS_MULTILINESTRING:
          //case geos::geom::GEOS_MULTIPOLYGON:
        case GEOS_MULTIPOINT:
        case GEOS_MULTILINESTRING:
        case GEOS_MULTIPOLYGON:
          nGeom = GEOSGetNumGeometries( geom );
          for ( i = 0; i < nGeom; i++ )
          {
            queue->push_back( GEOSGetGeometryN( geom, i ) );
          }
          break;
        case GEOS_POINT:
        case GEOS_LINESTRING:
        case GEOS_POLYGON:
          final_queue->push_back( geom );
          break;
        default:
          throw InternalException::UnknownGeometry();
      }
    }
    delete queue;

    return final_queue;
  }


} // namespace



