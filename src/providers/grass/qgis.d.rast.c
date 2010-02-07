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
#define PACKAGE "grassmods"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <grass/gis.h>
#include <grass/raster.h>
#include <grass/display.h>
#include <grass/glocale.h>

int display(char *name, char *mapset, RASTER_MAP_TYPE data_type);

int main(int argc, char **argv)
{
    char *mapset;
    char *name;
    int fp;
    struct GModule *module;
    struct Option *map;
    struct Option *win;
    struct Cell_head window;

    /* Initialize the GIS calls */
    G_gisinit(argv[0]);

    module = G_define_module();
    module->keywords = ("display, raster");
    module->description = ("Output raster map layers in a format suitable for display in QGIS");

    map = G_define_standard_option(G_OPT_R_MAP);
    map->description = ("Raster map to be displayed");

    win = G_define_option();
    win->key = "window";
    win->type = TYPE_DOUBLE;
    win->multiple = YES;
    win->description = "xmin,ymin,xmax,ymax,ncols,nrows";
     
    if (G_parser(argc, argv))
	exit(EXIT_FAILURE);

    name = map->answer;

    G_get_window(&window);
    window.west = atof(win->answers[0]);
    window.south = atof(win->answers[1]);
    window.east = atof(win->answers[2]);
    window.north = atof(win->answers[3]);
    window.cols = atoi(win->answers[4]);
    window.rows = atoi(win->answers[5]);
    G_adjust_Cell_head(&window,1,1);
    G_set_window(&window);

    /* Make sure map is available */
    mapset = G_find_cell2(name, "");
    if (mapset == NULL)
	G_fatal_error(("Raster map <%s> not found"), name);


    fp = G_raster_map_is_fp(name, mapset);

    /* use DCELL even if the map is FCELL */
    if (fp)
	display(name, mapset, DCELL_TYPE );
    else
	display(name, mapset, CELL_TYPE );

    exit(EXIT_SUCCESS);
}

static int cell_draw(char *, char *, struct Colors *, RASTER_MAP_TYPE);

int display(char *name,
	    char *mapset,
	    RASTER_MAP_TYPE data_type)
{
    struct Colors colors;

    if (G_read_colors(name, mapset, &colors) == -1)
	G_fatal_error(("Color file for <%s> not available"), name);

    //G_set_null_value_color(r, g, b, &colors);

    /* Go draw the raster map */
    cell_draw(name, mapset, &colors, data_type);

    /* release the colors now */
    G_free_colors(&colors);

    return 0;
}

static int cell_draw(char *name,
		     char *mapset,
		     struct Colors *colors,
		     RASTER_MAP_TYPE data_type)
{
    int cellfile;
    void *xarray;
    int row;
    int ncols, nrows;
    static unsigned char *red, *grn, *blu, *set;
    int i;
    void *ptr;
    int big_endian;
    long one= 1;
    FILE *fo;

    big_endian = !(*((char *)(&one)));

    ncols = G_window_cols();
    nrows = G_window_rows();

    /* Make sure map is available */
    if ((cellfile = G_open_cell_old(name, mapset)) == -1)
	G_fatal_error(("Unable to open raster map <%s>"), name);

    /* Allocate space for cell buffer */
    xarray = G_allocate_raster_buf(data_type);
    red = G_malloc(ncols);
    grn = G_malloc(ncols);
    blu = G_malloc(ncols);
    set = G_malloc(ncols);

    /* some buggy C libraries require BOTH setmode() and fdopen(bin) ? */
    //setmode(fileno(stdin), O_BINARY);
    fo = fdopen (fileno(stdout), "wb");
    
    /* loop for array rows */
    for ( row = 0; row < nrows; row++ ) {
	G_get_raster_row(cellfile, xarray, row, data_type);
        ptr = xarray;

        G_lookup_raster_colors(xarray, red, grn, blu, set, ncols, colors,
                                data_type);

        for (i = 0; i < ncols; i++) { 
            unsigned char alpha = 255;
            if ( G_is_null_value(ptr, data_type) ) {
                alpha = 0;
            }
            ptr = G_incr_void_ptr(ptr, G_raster_size(data_type));
            

            // We need data suitable for QImage 32-bpp
            // the data are stored in QImage as QRgb which is unsigned int.
            // Because it depends on byte order of the platform we have to 
            // consider byte order (well, middle endian ignored)
            if ( big_endian ) {
                // I have never tested this 
                fprintf(fo, "%c%c%c%c", alpha, red[i],grn[i],blu[i]);
            } else {
                fprintf(fo, "%c%c%c%c", blu[i],grn[i],red[i],alpha);
            }
        }
    }

    G_close_cell(cellfile);
    return (0);
}
