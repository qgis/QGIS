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

#ifndef _PAL_GEOMETRY_H
#define _PAL_GEOMETRY_H

#include <cstdlib> // for size_t needed in geos_c.h
#include <geos_c.h>

namespace pal
{

  /**
   * \brief Interface that allows Pal to access user's geometries
   */
  class CORE_EXPORT PalGeometry
  {
    public:

      /**
       * \brief get the GEOSGeometry of the feature
       * This method is called by Pal each time it needs a geom's coordinates
       *
       * @return GEOSGeometry * a pointer the geos geom
       */
      virtual const GEOSGeometry* getGeosGeometry() = 0;


      /**
       * \brief Called by Pal when it doesn't need the coordinates anymore
       * @param the_geom is the geoms geom  from PalGeometry::getfeomGeometry()
       */
      virtual void releaseGeosGeometry( const GEOSGeometry *the_geom ) = 0;


      virtual ~PalGeometry() {}
  };

} // end namespace pal

#endif
