/***************************************************************************
    qgis.d.rast.c
    ---------------------
    begin                : February 2010
    copyright            : (C) 2010 by Radim Blazek
    email                : radim dot blazek at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
/****************************************************************************
 *
 * MODULE:       qgis.d.rast
 * AUTHOR(S):    Radim Blazek <radim.blazek gmail.com>
 *               using d.rast from GRASS
 * PURPOSE:      display raster maps in active graphics display
 * COPYRIGHT:    (C) 2010 by Radim Blazek
 *
 *               This program is free software under the GNU General Public
 *               License (>=v2).
 *
 *****************************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include <limits.h>
#ifdef WIN32
#include <fcntl.h>
#include <io.h>
#endif
#include <grass/version.h>
#include <grass/gis.h>
#include <grass/raster.h>
#include <grass/display.h>

#ifdef _MSC_VER
#include <float.h>
#define INFINITY (DBL_MAX+DBL_MAX)
#define NAN (INFINITY-INFINITY)
#endif

#if GRASS_VERSION_MAJOR >= 7
#define G_allocate_raster_buf Rast_allocate_buf
#define G_close_cell Rast_close
#define G_free_colors Rast_free_colors
#define G_get_cellhd Rast_get_cellhd
#define G_get_raster_map_type Rast_get_map_type
#define G_get_raster_row Rast_get_row
#define G_is_null_value Rast_is_null_value
#define G_lookup_raster_colors Rast_lookup_colors
#define G_open_cell_old Rast_open_old
#define G_raster_map_type Rast_map_type
#define G_raster_size Rast_cell_size
#define G_read_colors Rast_read_colors
#define G_window_cols Rast_window_cols
#define G_window_rows Rast_window_rows
#endif

int display( char *name, char *mapset, RASTER_MAP_TYPE data_type, char *format );

int main( int argc, char **argv )
{
  char *mapset;
  char *name;
  struct GModule *module;
  struct Option *map;
  struct Option *win;
  struct Option *format;
  struct Cell_head window;
  RASTER_MAP_TYPE raster_type;

  /* Initialize the GIS calls */
  G_gisinit( argv[0] );

  module = G_define_module();
  module->keywords = ( "display, raster" );
  module->description = ( "Output raster map layers in a format suitable for display in QGIS" );

  map = G_define_standard_option( G_OPT_R_MAP );
  map->description = ( "Raster map to be displayed" );

  format = G_define_option();
  format->key = "format";
  format->type = TYPE_STRING;
  format->description = "format";
  format->options = "color,value";

  win = G_define_option();
  win->key = "window";
  win->type = TYPE_DOUBLE;
  win->multiple = YES;
  win->description = "xmin,ymin,xmax,ymax,ncols,nrows";

  if ( G_parser( argc, argv ) )
    exit( EXIT_FAILURE );

  name = map->answer;

  /* Make sure map is available */
#if GRASS_VERSION_MAJOR < 7
  mapset = G_find_cell2( name, "" );
  if ( mapset == NULL )
    G_fatal_error(( "Raster map <%s> not found" ), name );
#else
  mapset = "";
#endif

  /* It can happen that GRASS data set is 'corrupted' and zone differs in WIND and
   * cellhd, and G_open_cell_old fails, so it is better to read window from map */
  /* G_get_window( &window ); */
  G_get_cellhd( name, mapset, &window );
  window.west = atof( win->answers[0] );
  window.south = atof( win->answers[1] );
  window.east = atof( win->answers[2] );
  window.north = atof( win->answers[3] );
  window.cols = atoi( win->answers[4] );
  window.rows = atoi( win->answers[5] );
  G_adjust_Cell_head( &window, 1, 1 );
  G_set_window( &window );

  raster_type = G_raster_map_type( name, "" );

  display( name, mapset, raster_type, format->answer );

  exit( EXIT_SUCCESS );
}

static int cell_draw( char *, char *, struct Colors *, RASTER_MAP_TYPE, char *format );

int display( char *name,
             char *mapset,
             RASTER_MAP_TYPE data_type,
             char *format )
{
  struct Colors colors;

  if ( G_read_colors( name, mapset, &colors ) == -1 )
    G_fatal_error(( "Color file for <%s> not available" ), name );

  //G_set_null_value_color(r, g, b, &colors);

  /* Go draw the raster map */
  cell_draw( name, mapset, &colors, data_type, format );

  /* release the colors now */
  G_free_colors( &colors );

  return 0;
}

static int cell_draw( char *name,
                      char *mapset,
                      struct Colors *colors,
                      RASTER_MAP_TYPE data_type,
                      char *format )
{
  int cellfile;
  void *xarray;
  int row;
  int ncols, nrows;
  static unsigned char *red, *grn, *blu, *set;
  int i;
  void *ptr;
  int big_endian;
  long one = 1;
  FILE *fo;
  size_t raster_size;
#ifdef NAN
  double dnul = NAN;
  float fnul = NAN;
#else
  double dnul = strtod( "NAN", 0 );
  float fnul = strtof( "NAN", 0 );
  // another possibility would be nan()/nanf() - C99
  // and 0./0. if all fails
#endif

  assert( dnul != dnul );
  assert( fnul != fnul );

  big_endian = !( *(( char * )( &one ) ) );

  ncols = G_window_cols();
  nrows = G_window_rows();

  /* Make sure map is available */
  if (( cellfile = G_open_cell_old( name, mapset ) ) == -1 )
    G_fatal_error(( "Unable to open raster map <%s>" ), name );

  /* Allocate space for cell buffer */
  xarray = G_allocate_raster_buf( data_type );
  red = G_malloc( ncols );
  grn = G_malloc( ncols );
  blu = G_malloc( ncols );
  set = G_malloc( ncols );

  /* some buggy C libraries require BOTH setmode() and fdopen(bin) */
#ifdef Q_OS_WIN
  if ( _setmode( _fileno( stdout ), _O_BINARY ) == -1 )
    G_fatal_error( "Cannot set stdout mode" );
#endif
  // Unfortunately this is not sufficient on Windows to switch stdout to binary mode
  fo = fdopen( fileno( stdout ), "wb" );

  raster_size = G_raster_size( data_type );
  //fprintf( fo, "%d %d", data_type, raster_size );
  //exit(0);
  /* loop for array rows */
  for ( row = 0; row < nrows; row++ )
  {
    G_get_raster_row( cellfile, xarray, row, data_type );
    ptr = xarray;

    G_lookup_raster_colors( xarray, red, grn, blu, set, ncols, colors,
                            data_type );

    for ( i = 0; i < ncols; i++ )
    {
      unsigned char alpha = 255;
      //G_debug ( 0, "row = %d col = %d", row, i );
      if ( G_is_null_value( ptr, data_type ) )
      {
        alpha = 0;
      }

      if ( strcmp( format, "color" ) == 0 )
      {
        // We need data suitable for QImage 32-bpp
        // the data are stored in QImage as QRgb which is unsigned int.
        // Because it depends on byte order of the platform we have to
        // consider byte order (well, middle endian ignored)
        if ( big_endian )
        {
          // I have never tested this
          fprintf( fo, "%c%c%c%c", alpha, red[i], grn[i], blu[i] );
        }
        else
        {
          fprintf( fo, "%c%c%c%c", blu[i], grn[i], red[i], alpha );
        }
      }
      else
      {
        if ( data_type == CELL_TYPE )
        {
          //G_debug ( 0, "valx = %d", *((CELL *) ptr));
        }
        if ( G_is_null_value( ptr, data_type ) )
        {
          // see comments in QgsGrassRasterProvider::noDataValue()
          if ( data_type == CELL_TYPE )
          {
            //int nul = -2000000000;
            int nul = INT_MIN;
            fwrite( &nul, 4, 1, fo );
          }
          else if ( data_type == DCELL_TYPE )
          {
            //double nul = -1e+300;
            fwrite( &dnul, 8, 1, fo );
          }
          else if ( data_type == FCELL_TYPE )
          {
            //double nul = -1e+30;
            fwrite( &fnul, 4, 1, fo );
          }
        }
        else
        {
          fwrite( ptr, raster_size, 1, fo );
        }
      }
      ptr = G_incr_void_ptr( ptr, raster_size );
    }
  }

  G_close_cell( cellfile );
  fclose( fo );

  return ( 0 );
}
