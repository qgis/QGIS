/**
 * Declaration of Map class.
 * 
 * @file
 * @author Mauro E S Muñoz <mauro@cria.org.br>
 * @date 2003-09-05
 * $Id$
 *
 * LICENSE INFORMATION
 * 
 * Copyright(c) 2003 by CRIA -
 * Centro de Referencia em Informacao Ambiental
 *
 * http://www.cria.org.br
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details:
 * 
 * http://www.gnu.org/copyleft/gpl.html
 */

#ifndef _MAPHH_
#define _MAPHH_

#include <defs.hh>
#include <env_io/raster.hh>


class GeoTransform;


/****************************************************************/
/****************************** Map *****************************/

/** 
 * Responsable for the geografical and projectional transformations
 * related to reading and writing in raster maps.
 *
 */

/*******/
class Map
{
public:

  /** 
   * Create a new map based on a predefined raster.
   * 
   * @param rst Raster object
   * @param ocs The coordinates given to this object will be in
   *        this geografical coordinate system (in WKT format).
   * @param del If not zero 'rst' will be destroied with this
   *        object.
   */
  Map( Raster *rst, char *ocs, int del=0 );
  ~Map();

  int isCategorical()  { return f_rst->isCategorical(); }

  /** Normalize map values to the interval [min,max]. */
  int normalize( Scalar min, Scalar max ) {
    return f_rst->normalize( min, max );
  }

  /** Number of bands. */
  int numBand()   { return f_rst->numBand(); }

  /** Get the map limits. */
  int getRegion( Coord *xmin, Coord *ymin, Coord *xmax,
                 Coord *ymax);

  /**
   * Fills 'val' with the map bands values at (x,y).
   * Returns zero if (x,y) is not defined in the map.
   */
  int get( Coord x, Coord y, Scalar *val );

  /**
   * Put the values at 'val' in the map bands at (x,y).
   * @return Return zero if (x,y) is not defined in the map or the
   * map is read only.
   */
  int put( Coord x, Coord y, Scalar *val );

  GeoTransform *getGT()  { return f_gt; }


private:

  Raster       *f_rst;
  GeoTransform *f_gt;
  int           f_del; ///< If not zero destroy 'f_rst' in the destructor.
};


#endif
