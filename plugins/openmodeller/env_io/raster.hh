/**
 * Declarations of Raster and RasterFormat classes.
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

#ifndef _RASTERHH_
#define _RASTERHH_

#include <defs.hh>
#include <env_io/header.hh>



/****************************************************************/
/************************* Raster Format ************************/

/** 
 * Defines the names of the raster file formats.
 *
 */

/****************/
class RasterFormat
{
public:

  typedef enum {
    Unknown = 0,

    // GDAL compatible raster formats.
    AAIGrid, AIG, BMP, BSB, CEOS, DOQ1, DOQ2,
    DTED, ECW, EHdr, ENVI, Envisat, FAST, FITS,
    GIF, GIO, GRASS, GTiff, HDF4, HFA, HKV, JDEM,
    JPEG, JPEG2000, JP2KAK, L1B, MFF, MrSID, NITF,
    OGDI, PAux, PNG, PNM, SDTS, SAR_CEOS, USGSDEM,
    XPM,

    Undefined  // Used as the number of formats.

  } Format;


public:

  RasterFormat( char *file_name );

  int  code()       { return f_code; }
  char *name()      { return f_name[f_code]; }
  char *extension() { return f_ext[f_code]; }


private:

  int f_code;

  static char *f_name[];
  static char *f_ext[];
};




/****************************************************************/
/**************************** Raster ****************************/

/** 
 * Base class to read a georeferenced map stored in some raster
 * file format.
 *
 * This class is an interface between the modelling algorithms
 * and the map datas.
 *
 */

/**********/
class Raster
{
public:

  /** 
   * If 'categ' != 0 this is a categorical map (ie it can't be
   * interpolated). Othewise this is a continuos map.
   *
   */
  Raster( int categ=0 );
  Raster( Header &hdr );

  virtual ~Raster() {}

  /** Get the header. */
  Header &header()     { return f_hdr; }

  /** Returns not zero if this map is categorical. */
  int isCategorical()  { return f_hdr.categ; }

  /**
   * Normalize the map values in the interval [min, max].
   * For categorical maps it has no effect.
   */
  int normalize( Scalar min, Scalar max );

  /** Deativate the normalization process. */
  int denormalize()                           { f_normal = 0; }


  /** Lowest longitude. */
  Coord xMin() { return f_hdr.xmin; }

  /** Lowest latitude. */
  Coord yMin() { return f_hdr.ymin; }

  /** Highest longitude. */
  Coord xMax() { return f_hdr.xmax; }

  /** Highest latitude. */
  Coord yMax() { return f_hdr.ymax; }

  /** Get map limits. */
  int getRegion( Coord *xmin, Coord *ymin,
		 Coord *xmax, Coord *ymax);

  /** Longitudinal map dimension. */
  int dimX()   { return f_hdr.xdim; }

  /** Latitudinal map dimension. */
  int dimY()   { return f_hdr.ydim; }

  /** Longitudinal and latitudinal dimensions. */
  int getDim( int *xdim, int *ydim );

  /** Cell dimensions. */
  int getCell( Coord *xcel, Coord *ycel );

  /**
   * Returns not zero if it is stored like a grid map and
   * zero if it is stored like a pixel map.
   */
  int getGrid()   { return f_hdr.grid; }

  /** Returns the "noval" value. */
  Scalar noVal()  { return f_hdr.noval; }

  /** Number of bands. */
  int numBand()   { return f_hdr.nband; }

  /**
   * Fills '*val' with the map value at (x,y).
   * Returns zero if (x,y) is out of range.
   */
  int get( Coord x, Coord y, Scalar *val );

  /**
   * Put '*val' at the (x,y) coordinate.
   * Returns 0 if (x,y) is out of range or the map is read only.
   */
  int put( Coord x, Coord y, Scalar *val );

  /**
   * The same as 'get()', but uses the image (col,row) instead
   * of the (longitude, latitude) coordinates.
   * The (x,y) must be in the image range. Returns 0 if the
   * value is 'noval'.
   */
  virtual int iget( int x, int y, Scalar *val ) = 0;

  /**
   * The same as 'put()', but uses the image (col,row) instead
   * of the (longitude, latitude) coordinates.
   * The (x,y) must be in the image range.
   */
  virtual int iput( int x, int y, Scalar *val ) = 0;

  virtual int load( char *file ) = 0;
  virtual int save( char *file ) = 0;


  /** Print header in stdout. */
  virtual int print( char *msg="" );


protected:

  int setHeader( Header &hdr );

  /**
   * Convert georeferenced coordinates (x,y) in the map to
   * (col,row) coordinated in raster image.
   */
  int convX( Coord x );
  int convY( Coord y );

  /**
   * Normalizes the first band of '*val'.
   * todo: to normalize the other bands.
   */
  void calcNormal( Scalar *val );

  /** Find the minimum and maximum values in 'band'. */
  int calcMinMax( Scalar *min, Scalar *max, int band=0 );


private:

  Header f_hdr;

  /** Todo: to normalize all bands, not just the first. */
  int    f_normal;  // Should normalizes?
  Scalar f_offset;  // Values used to normalize.
  Scalar f_scale;
};


#endif
