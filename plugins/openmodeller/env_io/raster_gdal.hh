/**
 * Declaration of RasterGdal class.
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

#ifndef _RASTER_GDALHH_
#define _RASTER_GDALHH_

#include "env_io/header.hh"

class GDALDataset;


/****************************************************************/
/************************** Raster Gdal *************************/

/** 
 * Manages raster files with GDAL (Geospatial Data Abstraction
 * Library - http://www.remotesensing.org/gdal/index.html)
 *
 * The file type is automaticaly detected by its extension.
 *
 */

/**************/
class RasterGdal
{
public:

  /** Open 'file' for read (mode = 'r') or write (mode = 'w'). **/
  RasterGdal( char *file, char mode );

  /** Create a new file for write based on the header 'hdr'. **/
  RasterGdal( char *file, Header &hdr );

  virtual ~RasterGdal();

  /** Returns the file's header **/
  Header &header()   { return f_hdr; }

  /**
   * Read 'num_rows' rows starting from 'first_row' to the memory
   * pointed to by 'buf'. Each element read is transformed in a
   * 'Scalar'.
   */
  int read ( Scalar *buf, int first_row, int num_rows );

  /**
   * Write 'num_rows' rows starting from 'first_row' to the memory
   * pointed to by 'buf'.
   */
  int write( Scalar *buf, int first_row, int num_rows );


private:

  /** Open a raster file. **/
  int open( char *file, char mode );

  /** Create a new raster file based on 'hdr'. **/
  int create( char *file, Header &hdr );

  void initGdal();


  int          f_type;
  Header       f_hdr;
  GDALDataset *f_ds;
};


#endif
