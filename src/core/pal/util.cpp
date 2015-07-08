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

#include <stddef.h>
#include <geos_c.h>

#include <sstream>

#include <iostream>
#include <cfloat>
//#include <cfloat>
#include <cstdarg>
#include <ctime>

#include "layer.h"
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

  inline bool ptrGeomEq( const GEOSGeometry *l, const GEOSGeometry *r )
  {
    return l == r;
  }

  QLinkedList<const GEOSGeometry *> *unmulti( const GEOSGeometry *the_geom )
  {
    QLinkedList<const GEOSGeometry*> *queue = new QLinkedList<const GEOSGeometry*>;
    QLinkedList<const GEOSGeometry*> *final_queue = new QLinkedList<const GEOSGeometry*>;

    const GEOSGeometry *geom;

    queue->append( the_geom );
    int nGeom;
    int i;

    while ( queue->size() > 0 )
    {
      geom = queue->takeFirst();
      GEOSContextHandle_t geosctxt = geosContext();
      switch ( GEOSGeomTypeId_r( geosctxt, geom ) )
      {
        case GEOS_MULTIPOINT:
        case GEOS_MULTILINESTRING:
        case GEOS_MULTIPOLYGON:
          nGeom = GEOSGetNumGeometries_r( geosctxt, geom );
          for ( i = 0; i < nGeom; i++ )
          {
            queue->append( GEOSGetGeometryN_r( geosctxt, geom, i ) );
          }
          break;
        case GEOS_POINT:
        case GEOS_LINESTRING:
        case GEOS_POLYGON:
          final_queue->append( geom );
          break;
        default:
          delete final_queue;
          delete queue;
          return NULL;
      }
    }
    delete queue;

    return final_queue;
  }


} // namespace



