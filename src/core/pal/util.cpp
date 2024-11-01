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

#include "qgsgeos.h"
#include "util.h"

#include "qgslogger.h"
#include <cfloat>

QLinkedList<const GEOSGeometry *> *pal::Util::unmulti( const GEOSGeometry *the_geom )
{
  QLinkedList<const GEOSGeometry *> *queue = new QLinkedList<const GEOSGeometry *>;
  QLinkedList<const GEOSGeometry *> *final_queue = new QLinkedList<const GEOSGeometry *>;

  const GEOSGeometry *geom = nullptr;

  queue->append( the_geom );
  int nGeom;
  int i;

  GEOSContextHandle_t geosctxt = QgsGeosContext::get();

  while ( !queue->isEmpty() )
  {
    geom = queue->takeFirst();
    const int type = GEOSGeomTypeId_r( geosctxt, geom );
    switch ( type )
    {
      case GEOS_MULTIPOINT:
      case GEOS_MULTILINESTRING:
      case GEOS_MULTIPOLYGON:
      case GEOS_GEOMETRYCOLLECTION:
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
        QgsDebugError( QStringLiteral( "unexpected geometry type:%1" ).arg( type ) );
        delete final_queue;
        delete queue;
        return nullptr;
    }
  }
  delete queue;

  return final_queue;
}
