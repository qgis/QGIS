/**
 * Declaration of RasterFile class.
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

#ifndef _RASTER_FILEHH_
#define _RASTER_FILEHH_

#include "env_io/raster.hh"


class RasterGdal;


/****************************************************************/
/*************************** Raster File ************************/

/** 
 * A Raster map stored in a file.
 * 
 * Todo: to use virtual memory.
 *
 */

/******************************/
class RasterFile : public Raster
{
public:

  /**
   * Associates this object to a file.
   * @see Raster::Raster()
   */
  RasterFile( char *file, int categ=0 );

  /** Create a new file. **/
  RasterFile( char *file, Header &hdr ); // Cria um novo mapa.

  virtual ~RasterFile();

  int iget( int x, int y, Scalar *val );
  int iput( int x, int y, Scalar *val );

  /** The file format is known by the file name extension. **/
  int load( char *file );

  /**
   * The file format is known by the file name extension.
   * Make a copy of this file into 'file' but keep with this
   * file opened.
   */
  int save( char *file );


private:

  int loadRow( int row );
  int saveRow();           // Save the 'f_last' row.


  RasterGdal *f_rst;

  Scalar *f_data; // One line data for all bands.
  int     f_size; // Size of one line.

  int f_last;
  int f_changed;

  char f_file[256];  // File name (needed by 'save()').
};


#endif
