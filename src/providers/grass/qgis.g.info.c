/****************************************************************************
 *
 * MODULE:       qgis.g.info
 * AUTHOR(S):    Radim Blazek <radim.blazek gmail.com>
 *               using various GRASS modules
 * PURPOSE:      get informations about locations,mapsets,maps
 * COPYRIGHT:    (C) 2010 by Radim Blazek
 *
 *               This program is free software under the GNU General Public
 *               License (>=v2).
 *
 *****************************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <grass/gis.h>
#include <grass/raster.h>
#include <grass/display.h>
#include <grass/gprojects.h>

int main( int argc, char **argv )
{
  struct GModule *module;
  struct Option *info_opt, *rast_opt, *vect_opt, *coor_opt;
  struct Cell_head window;

  /* Initialize the GIS calls */
  G_gisinit( argv[0] );

  module = G_define_module();
  module->description = ( "Get info about locations,mapsets,maps" );

  info_opt = G_define_option();
  info_opt->key = "info";
  info_opt->type = TYPE_STRING;
  info_opt->description = "info key";
  info_opt->options = "proj,window,query";

  rast_opt = G_define_standard_option( G_OPT_R_INPUT );
  rast_opt->key = "rast";
  rast_opt->required = NO;

  vect_opt = G_define_standard_option( G_OPT_V_INPUT );
  vect_opt->key = "vect";
  vect_opt->required = NO;

  coor_opt = G_define_option();
  coor_opt->key = "coor";
  coor_opt->type = TYPE_DOUBLE;
  coor_opt->multiple = YES;

  if ( G_parser( argc, argv ) )
    exit( EXIT_FAILURE );


  if ( strcmp( "proj", info_opt->answer ) == 0 )
  {
    G_get_window( &window );
    /* code from g.proj */
    if ( window.proj != PROJECTION_XY )
    {
      struct Key_Value *projinfo, *projunits;
      char *wkt;
      projinfo = G_get_projinfo();
      projunits = G_get_projunits();
      wkt = GPJ_grass_to_wkt( projinfo, projunits,  0, 0 );
      fprintf( stdout, "%s", wkt );
    }
  }
  else if ( strcmp( "window", info_opt->answer ) == 0 )
  {
    if ( rast_opt->answer )
    {
      G_get_cellhd( rast_opt->answer, "", &window );
      fprintf( stdout, "%f,%f,%f,%f", window.west, window.south, window.east, window.north );
    }
    else if ( vect_opt->answer )
    {
      G_fatal_error( "Not yet supported" );
    }
  }
  else if ( strcmp( "query", info_opt->answer ) == 0 )
  {
    double x, y;
    int row, col;
    x = atof( coor_opt->answers[0] );
    y = atof( coor_opt->answers[1] );
    if ( rast_opt->answer )
    {
      int fd;
      RASTER_MAP_TYPE rast_type;
      DCELL *dcell;
      CELL *cell;
      G_get_cellhd( rast_opt->answer, "", &window );
      G_set_window( &window );
      fd = G_open_cell_old( rast_opt->answer, "" );
      col = ( int ) G_easting_to_col( x, &window );
      row = ( int ) G_northing_to_row( y, &window );
      if ( col == window.cols ) col--;
      if ( row == window.rows ) row--;

      if ( col < 0 || col > window.cols || row < 0 || row > window.rows )
      {
        fprintf( stdout, "value:null\n" );
      }
      else
      {
        rast_type = G_get_raster_map_type( fd );
        cell = G_allocate_c_raster_buf();
        dcell = G_allocate_d_raster_buf();
        void *ptr;
        double val;

        if ( rast_type == CELL_TYPE )
        {
          if ( G_get_c_raster_row( fd, cell, row ) < 0 )
          {
            G_fatal_error(( "Unable to read raster map <%s> row %d" ),
                          rast_opt->answer, row );
          }
          val = cell[col];
          ptr = &(cell[col]);
        }
        else
        {
          if ( G_get_d_raster_row( fd, dcell, row ) < 0 )
          {
            G_fatal_error(( "Unable to read raster map <%s> row %d" ),
                          rast_opt->answer, row );
          }
          val = dcell[col];
          ptr = &(dcell[col]);
        }
        if ( G_is_null_value( ptr, rast_type ) )
        {
          fprintf( stdout, "value:null\n" );
        }
        else
        {
          fprintf( stdout, "value:%f\n", val );
        }
      }
      G_close_cell( fd );
    }
    else if ( vect_opt->answer )
    {
      G_fatal_error( "Not yet supported" );
    }
  }

  exit( EXIT_SUCCESS );
}

