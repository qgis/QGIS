/**
 * Declaration of Header class.
 * 
 * @file
 * @author Mauro E S Muñoz <mauro@cria.org.br>
 * @date 2003-08-22
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

#ifndef _HEADERHH_
#define _HEADERHH_

#include <defs.hh>


/****************************************************************/
/***************************** Header ***************************/

/** 
 * Header with metadata of georeferenced raster maps.
 */

/**********/
class Header
{
public:

  enum DataType {
    UInt8, Int16, UInt16, Int32, UInt32, Float32, Float64,
    MaxDataType
  };


  Header() { proj = 0; }
  Header( Header &h );
  Header( int xd, int yd, Coord xm, Coord ym,
          Coord xM, Coord yM, Scalar nv, int nb=1, int gd=0 );
  ~Header();

  Header &operator=( Header &h );

  // Calculate (xcel, ycel) using xmin, ymin, xmax, ymax,
  // xdim e ydim.
  void calculateCell();

  char *setProj( char *projection );
  int   hasProj()                      { return proj && *proj; }

  void print( char *msg="" );


  int xdim;    /**< Map width **/
  int ydim;    /**< Map height **/
  Coord xmin;  /**< Lowest longitude **/
  Coord ymin;  /**< Lowest latitude **/
  Coord xmax;  /**< Highest longitude **/
  Coord ymax;  /**< Highest latitude **/
  Coord xcel;  /**< Cell width **/
  Coord ycel;  /**< Cell hight **/

  Scalar noval; /**< Value indicating absence of information. **/
  int nband; /**< Number of bands. **/

  /** "grid" (not zero) or "pixel" (zero) cell alignment **/
  int grid;

  /**
   * If not zero the map represents a categorical variable.
   * This way it cannot be interpolated.
   * Obs: it is not saved in files! (there is no room :( ).
   */
  int categ;

  // Valores mínimo e máximo.
  // 'minmax' != 0, indica que 'min' e 'max' são válidos.

 /** If not zero 'min' and 'max' are valid values. **/
  int minmax;

  Scalar min; /**< Minimum map value. */
  Scalar max; /**< Maximum map value. */

  DataType dtype; /**< File data type. */

  char *proj;  /**< Projection specification (in WKT). */
};


#endif
