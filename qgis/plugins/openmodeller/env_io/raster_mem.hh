/**
 * Declaration of RasterMemory class.
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

#ifndef _RASTER_MEMHH_
#define _RASTER_MEMHH_

#include "env_io/raster.hh"



/****************************************************************/
/************************** Raster Memory ***********************/

/** 
 * A Raster map stored in memory.
 * 
 * Todo: to use virtual memory.
 *
 */

/*********************************/
class RasterMemory : public Raster
{
public:

  /** @see Raster::Raster() **/
  RasterMemory( char *file, int categ=0 );
  RasterMemory( Header &hdr );

  virtual ~RasterMemory();

  int iget( int x, int y, Scalar *val );
  int iput( int x, int y, Scalar *val );

  /** The file format is known by the file name extension. **/
  int load( char *file );
  int save( char *file );


private:

  Scalar *f_map;  /**< Map data in memory. **/
  int     f_size; /**< Band size (in Scalars). **/
};


#endif
