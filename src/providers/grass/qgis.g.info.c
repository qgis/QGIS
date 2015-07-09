/***************************************************************************
    qgis.g.info.c
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
 * MODULE:       qgis.g.info
 * AUTHOR(S):    Radim Blazek <radim.blazek gmail.com>
 *               using various GRASS modules
 * PURPOSE:      get information about locations, mapsets, maps
 * COPYRIGHT:    (C) 2010 by Radim Blazek
 *
 *               This program is free software under the GNU General Public
 *               License (>=v2).
 *
 *****************************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <float.h>
#include <grass/version.h>
#include <grass/gis.h>
#include <grass/raster.h>
#include <grass/display.h>
#include <grass/gprojects.h>

#if GRASS_VERSION_MAJOR >= 7
#define G_allocate_c_raster_buf Rast_allocate_c_buf
#define G_allocate_d_raster_buf Rast_allocate_d_buf
#define G_close_cell Rast_close
#define G_colors_count Rast_colors_count
#define G_easting_to_col Rast_easting_to_col
#define G_get_c_raster_row Rast_get_c_row
#define G_get_cellhd Rast_get_cellhd
#define G_get_d_raster_row Rast_get_d_row
#define G_get_f_color_rule Rast_get_fp_color_rule
#define G_get_fp_range_min_max Rast_get_fp_range_min_max
#define G_get_raster_map_type Rast_get_map_type
#define G_is_null_value Rast_is_null_value
#define G_northing_to_row Rast_northing_to_row
#define G_open_cell_old Rast_open_old
#define G_raster_map_type Rast_map_type
#define G_read_colors Rast_read_colors
#define G_read_fp_range Rast_read_fp_range
#define G_window_cols Rast_window_cols
#define G_window_rows Rast_window_rows
#endif

int main( int argc, char **argv )
{
  struct GModule *module;
  struct Option *info_opt, *rast_opt, *vect_opt, *coor_opt, *north_opt, *south_opt, *east_opt, *west_opt, *rows_opt, *cols_opt;
  struct Cell_head window;

  /* Initialize the GIS calls */
  G_gisinit( argv[0] );

  module = G_define_module();
  module->description = ( "Get info about locations,mapsets,maps" );

  info_opt = G_define_option();
  info_opt->key = "info";
  info_opt->type = TYPE_STRING;
  info_opt->description = "info key";
  info_opt->options = "proj,window,size,query,info,colors,stats";

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

  north_opt = G_define_option();
  north_opt->key = "north";
  north_opt->type = TYPE_STRING;

  south_opt = G_define_option();
  south_opt->key = "south";
  south_opt->type = TYPE_STRING;

  east_opt = G_define_option();
  east_opt->key = "east";
  east_opt->type = TYPE_STRING;

  west_opt = G_define_option();
  west_opt->key = "west";
  west_opt->type = TYPE_STRING;

  rows_opt = G_define_option();
  rows_opt->key = "rows";
  rows_opt->type = TYPE_INTEGER;

  cols_opt = G_define_option();
  cols_opt->key = "cols";
  cols_opt->type = TYPE_INTEGER;

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
      wkt = GPJ_grass_to_wkt( projinfo, projunits, 0, 0 );
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
  // raster width and height
  else if ( strcmp( "size", info_opt->answer ) == 0 )
  {
    if ( rast_opt->answer )
    {
      G_get_cellhd( rast_opt->answer, "", &window );
      fprintf( stdout, "%d,%d", window.cols, window.rows );
    }
    else if ( vect_opt->answer )
    {
      G_fatal_error( "Not yet supported" );
    }
  }
  // raster information
  else if ( strcmp( "info", info_opt->answer ) == 0 )
  {
    struct FPRange range;
    double zmin, zmax;

    // Data type
    RASTER_MAP_TYPE raster_type = G_raster_map_type( rast_opt->answer, "" );
    fprintf( stdout, "TYPE:%d\n", raster_type );

    // Statistics
    if ( G_read_fp_range( rast_opt->answer, "", &range ) < 0 )
    {
      G_fatal_error(( "Unable to read range file" ) );
    }
    G_get_fp_range_min_max( &range, &zmin, &zmax );
    fprintf( stdout, "MIN_VALUE:%.17e\n", zmin );
    fprintf( stdout, "MAX_VALUE:%.17e\n", zmax );
  }
  else if ( strcmp( "colors", info_opt->answer ) == 0 )
  {
    // Color table
    struct Colors colors;
    int i, ccount;
    if ( G_read_colors( rast_opt->answer, "", &colors ) == 1 )
    {
      //int maxcolor;
      //CELL min, max;

      //G_get_color_range ( &min, &max, &colors);
      ccount = G_colors_count( &colors );
      for ( i = ccount - 1; i >= 0; i-- )
      {
        DCELL val1, val2;
        unsigned char r1, g1, b1, r2, g2, b2;

        G_get_f_color_rule( &val1, &r1, &g1, &b1, &val2, &r2, &g2, &b2, &colors, i );
        fprintf( stdout, "%.17e %.17e %d %d %d %d %d %d\n", val1, val2, r1, g1, b1, r2, g2, b2 );
      }
    }
  }

  else if ( strcmp( "query", info_opt->answer ) == 0 )
  {
    double x, y;
    int row, col;
    //x = atof( coor_opt->answers[0] );
    //y = atof( coor_opt->answers[1] );
    if ( rast_opt->answer )
    {
      int fd;
      RASTER_MAP_TYPE rast_type;
      DCELL *dcell;
      CELL *cell;
      char buff[101];
      G_get_cellhd( rast_opt->answer, "", &window );
      G_set_window( &window );
      fd = G_open_cell_old( rast_opt->answer, "" );
      // wait for coords from stdin
      while ( fgets( buff, 100, stdin ) != 0 )
      {
        if ( sscanf( buff, "%lf%lf", &x, &y ) != 2 )
        {
          fprintf( stdout, "value:error\n" );
        }
        else
        {
          col = ( int ) G_easting_to_col( x, &window );
          row = ( int ) G_northing_to_row( y, &window );
          if ( col == window.cols )
            col--;
          if ( row == window.rows )
            row--;

          if ( col < 0 || col > window.cols || row < 0 || row > window.rows )
          {
            //fprintf( stdout, "value:out\n" );
            fprintf( stdout, "value:nan\n" );
          }
          else
          {
            void *ptr;
            double val;

#if defined(GRASS_VERSION_MAJOR) && defined(GRASS_VERSION_MINOR) && \
    ( ( GRASS_VERSION_MAJOR == 6 && GRASS_VERSION_MINOR > 2 ) || GRASS_VERSION_MAJOR > 6 )
            rast_type = G_get_raster_map_type( fd );
#else
            rast_type = G_raster_map_type( rast_opt->answer, "" );
#endif
            cell = G_allocate_c_raster_buf();
            dcell = G_allocate_d_raster_buf();

            if ( rast_type == CELL_TYPE )
            {
#if GRASS_VERSION_MAJOR < 7
              if ( G_get_c_raster_row( fd, cell, row ) < 0 )
              {
                G_fatal_error(( "Unable to read raster map <%s> row %d" ),
                              rast_opt->answer, row );
              }
#else
              G_get_c_raster_row( fd, cell, row );
#endif
              val = cell[col];
              ptr = &( cell[col] );
            }
            else
            {
#if GRASS_VERSION_MAJOR < 7
              if ( G_get_d_raster_row( fd, dcell, row ) < 0 )
              {
                G_fatal_error(( "Unable to read raster map <%s> row %d" ),
                              rast_opt->answer, row );
              }
#else
              G_get_d_raster_row( fd, dcell, row );
#endif
              val = dcell[col];
              ptr = &( dcell[col] );
            }
            if ( G_is_null_value( ptr, rast_type ) )
            {
              fprintf( stdout, "value:nan\n" );
            }
            else
            {
              fprintf( stdout, "value:%f\n", val );
            }
          }
        }
        fflush( stdout );
      }
      G_close_cell( fd );
    }
    else if ( vect_opt->answer )
    {
      G_fatal_error( "Not yet supported" );
    }
  }
  else if ( strcmp( "stats", info_opt->answer ) == 0 )
  {
    if ( rast_opt->answer )
    {
      int fd;
      RASTER_MAP_TYPE rast_type;
      DCELL *dcell;
      CELL *cell;
      int ncols, nrows;
      int row, col;
      void *ptr;
      double val;
      double min = DBL_MAX;
      double max = -DBL_MAX;
      double sum = 0; // sum of values
      int count = 0; // count of non null values
      double mean = 0;
      double squares_sum = 0; // sum of squares
      double stdev = 0; // standard deviation

      G_get_cellhd( rast_opt->answer, "", &window );
      window.north = atof( north_opt->answer );
      window.south = atof( south_opt->answer );
      window.east = atof( east_opt->answer );
      window.west = atof( west_opt->answer );
      window.rows = ( int ) atoi( rows_opt->answer );
      window.cols = ( int ) atoi( cols_opt->answer );

      G_set_window( &window );
      fd = G_open_cell_old( rast_opt->answer, "" );

      ncols = G_window_cols();
      nrows = G_window_rows();

#if defined(GRASS_VERSION_MAJOR) && defined(GRASS_VERSION_MINOR) && \
    ( ( GRASS_VERSION_MAJOR == 6 && GRASS_VERSION_MINOR > 2 ) || GRASS_VERSION_MAJOR > 6 )
      rast_type = G_get_raster_map_type( fd );
#else
      rast_type = G_raster_map_type( rast_opt->answer, "" );
#endif
      cell = G_allocate_c_raster_buf();
      dcell = G_allocate_d_raster_buf();

      // Calc stats is very slow for large rasters -> prefer optimization for speed over
      // code length and readability (which is not currently true)
      for ( row = 0; row < nrows; row++ )
      {
        if ( rast_type == CELL_TYPE )
        {
#if GRASS_VERSION_MAJOR < 7
          if ( G_get_c_raster_row( fd, cell, row ) < 0 )
          {
            G_fatal_error(( "Unable to read raster map <%s> row %d" ),
                          rast_opt->answer, row );
          }
#else
          G_get_c_raster_row( fd, cell, row );
#endif
        }
        else
        {
#if GRASS_VERSION_MAJOR < 7
          if ( G_get_d_raster_row( fd, dcell, row ) < 0 )
          {
            G_fatal_error(( "Unable to read raster map <%s> row %d" ),
                          rast_opt->answer, row );
          }
#else
          G_get_d_raster_row( fd, dcell, row );
#endif
        }

        for ( col = 0; col < ncols; col++ )
        {
          if ( rast_type == CELL_TYPE )
          {
            val = cell[col];
            ptr = &( cell[col] );
          }
          else
          {
            val = dcell[col];
            ptr = &( dcell[col] );
          }
          if ( ! G_is_null_value( ptr, rast_type ) )
          {
            if ( val < min ) min = val;
            if ( val > max ) max = val;
            sum += val;
            count++;
            squares_sum += pow( val, 2 );
          }
        }
      }
      mean = count > 0 ? sum / count : 0;
      squares_sum -= count * pow( mean, 2 );
      stdev = sqrt( squares_sum / ( count - 1 ) );

      fprintf( stdout, "MIN:%.17e\n", min );
      fprintf( stdout, "MAX:%.17e\n", max );
      fprintf( stdout, "SUM:%.17e\n", sum );
      fprintf( stdout, "MEAN:%.17e\n", mean );
      fprintf( stdout, "COUNT:%d\n", count );
      fprintf( stdout, "STDEV:%.17e\n", stdev );
      fprintf( stdout, "SQSUM:%.17e\n", squares_sum );

      G_close_cell( fd );
    }
    else if ( vect_opt->answer )
    {
      G_fatal_error( "Not yet supported" );
    }
  }

  exit( EXIT_SUCCESS );
}

