/**
 * Declaration of Environment class.
 * 
 * @file
 * @author Mauro E S Muñoz (mauro@cria.org.br)
 * @date   2003-03-13
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


#ifndef _ENVIRONMENTHH_
#define _ENVIRONMENTHH_

#include <defs.hh>

class Map;


/****************************************************************/
/************************* Environment **************************/

/** 
 * Allow access to environmental variables by means of vectors
 * gathering data from all layers on the sampled points.
 */
class Environment
{
public:

  /** Vector with file names that contain the variables to
   *  be used and also the mask layer.
   *
   * @param cs     Common coordinate system (external).
   * @param categs Categorical layers (e.g. soil).
   * @param maps   Continuous layers (e.g. temperature).
   */
  Environment( char *cs, int ncateg, char **categs,
	       int nmap, char **maps, char *mask_file=0 );

  ~Environment();


  int numLayers()   { return f_nlay; }

  /** Rebuild the layer representation. */
  int changeLayers( int ncateg, char **categs, int nmap,
		    char **maps );

  /** Set types[i] = 1 if layer "i" represents a categoprical
   *  variable (e.g. soil), otherwise set types[i] = 0. */
  int varTypes( int *types );

  /** Indicate that all non categorical variable layers must
   *  be normalized according to the interval [min, max]. */
  int normalize( Scalar min, Scalar max );

  /** Read for vector 'sample' all values of environmental variables
   *  of coordinate (x,y). Return 0 if there's a mask and the point
   *  is outside its borders.
   * 
   *  Obs: if a layer has not been configured, its position
   *  inside the response vector will not be modified. */
  int get( Coord x, Coord y, Scalar *sample );

  /** Read for 'sample' all values of environmental variables of a
   *  valid coordinate (inside the mask) randomly chosen. */
  int getRandom( Scalar *sample );


  /** Return 0 if (x,y) fall outside the mask. If there's no 
   *  mask, return != 0 always. */
  int check( Coord x, Coord y );

  /** Get rectangle that contain all valid points.
   *  If there's a mask, use its rectangle. */
  int getRegion( Coord *xmin, Coord *ymin, Coord *xmax,
                 Coord *ymax );

  char *getCoordinateSystem()   { return f_cs; }


private:

  void setCoordSystem( char *cs );

  /** Calculate the widest region common to all layers. Return
   *  0 if region is empty. */
  int calcRegion();

  /** Layer factory.
   *  If 'categ' == 0 then the map represents a continuous variable,
   *  otherwise, a categorical one. */
  Map *newMap( char *file_name, int categ=0 );


  int   f_nlay;   ///< Number of variables.
  Map **f_layers; ///< Vector with all layers that describe the variables.
  Map  *f_mask;   ///< Mask (can be 0).

  Coord f_xmin; ///< Widest region among all layers.
  Coord f_ymin; ///< Widest region among all layers.
  Coord f_xmax; ///< Widest region among all layers.
  Coord f_ymax; ///< Widest region among all layers.

  char *f_cs;  ///< Coordinate system to be used.
};


#endif
