/***************************************************************************
    qgsgrassgislib.h  -  Fake GRASS gis lib
                             -------------------
    begin                : Nov 2012
    copyright            : (C) 2012 by Radim Blazek
    email                : radim dot blazek at gmail dot com
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSGRASSGISLIB_H
#define QGSGRASSGISLIB_H

// GRASS header files
extern "C"
{
#include <grass/gis.h>
}

#include <stdexcept>
#include "qgscoordinatereferencesystem.h"
#include "qgsdistancearea.h"
#include "qgsexception.h"
#include "qgsproviderregistry.h"
#include "qgsrectangle.h"
#include "qgsrasterdataprovider.h"
#include "qgsrasterprojector.h"

#include <QProcess>
#include <QString>
#include <QMap>
#include <QHash>
#include <QTemporaryFile>
class QgsCoordinateReferenceSystem;
class QgsRectangle;

class GRASS_LIB_EXPORT QgsGrassGisLib
{
  public:
    // Region term is used in modules (g.region), internally it is hold in structure
    // Cell_head, but variables keeping that structure are usually called window
#if 0
    class Region
    {
        QgsRectangle extent;
        double ewRes; // east-west resolution
        double nsRes; // north south resolution
    };
#endif

    class Raster
    {
      public:
        int fd; // fake file descriptor
        QString name; // name passed from grass module, uri
        QgsRasterDataProvider *provider = nullptr;
        QgsRasterProjector *projector = nullptr;
        // Input points to provider or projector
        QgsRasterInterface *input = nullptr;
        int band;
        int row; // next row to be written
        double noDataValue; // output no data value

        Raster()
          : provider( 0 )
          , projector( 0 )
          , input( 0 )
          , band( 1 )
          , row( 0 )
          , fd( -1 )
          , noDataValue( -1 )
        {}
    };

    static QgsGrassGisLib *instance();

    QgsGrassGisLib();

    int G__gisinit( const char *version, const char *programName );
    char *G_find_cell2( const char *name, const char *mapset );
    int G_open_cell_old( const char *name, const char *mapset );
    int G_open_raster_new( const char *name, RASTER_MAP_TYPE wr_type );
    int G_open_fp_cell_new( const char *name );
    int G_close_cell( int fd );
    RASTER_MAP_TYPE G_raster_map_type( const char *name, const char *mapset );
    RASTER_MAP_TYPE G_get_raster_map_type( int fd );
    //int G_raster_map_is_fp( const char *name, const char *mapset );
    int G_read_fp_range( const char *name, const char *mapset, struct FPRange *drange );

    int readRasterRow( int fd, void *buf, int row, RASTER_MAP_TYPE data_type, bool noDataAsZero = false );
    int G_get_null_value_row( int fd, char *flags, int row ) const;
    int putRasterRow( int fd, const void *buf, RASTER_MAP_TYPE data_type );
    int G_get_cellhd( const char *name, const char *mapset, struct Cell_head *cellhd );

    double G_area_of_cell_at_row( int row );
    double G_area_of_polygon( const double *x, const double *y, int n );
    double G_database_units_to_meters_factor( void );
    int beginCalculations( void );
    double distance( double e1, double n1, double e2, double n2 );

    int G_set_geodesic_distance_lat1( double lat1 );
    int G_set_geodesic_distance_lat2( double lat2 );
    double G_geodesic_distance_lon_to_lon( double lon1, double lon2 );

    int G_get_ellipsoid_parameters( double *a, double *e2 );

    //! Gets QGIS raster type for GRASS raster type
    Qgis::DataType qgisRasterType( RASTER_MAP_TYPE grassType );

    //! Gets GRASS raster type for QGIS raster type
    RASTER_MAP_TYPE grassRasterType( Qgis::DataType qgisType );

    //! Gets no data value for GRASS data type
    double noDataValueForGrassType( RASTER_MAP_TYPE grassType );

    /**
     * Grass does not seem to have any function to init Cell_head,
     * initialization is done in G__read_Cell_head_array
    */
    void initCellHead( struct Cell_head *cellhd );

    //! Gets raster from map of opened rasters, open it if it is not yet open
    Raster raster( QString name );

    void *resolve( const char *symbol );

    // Print error function set to be called by GRASS lib
    static int errorRoutine( const char *msg, int fatal );

    // Error called by fake lib
    void fatal( QString msg );
    void warning( QString msg );

  private:
    //! Pointer to canonical Singleton object
    static QgsGrassGisLib *_instance;

    //! Raster maps, key is fake file descriptor
    QMap<int, Raster> mRasters;

    //! Region to be used for data processing and output
    struct Cell_head mWindow;

    //! Current region extent
    QgsRectangle mExtent;
    //! Current region rows
    int mRows = 0;
    //! Current region columns
    int mColumns = 0;
    //! X resolution
    double mXRes = 0;
    //! Y resolution
    double mYRes = 0;
    //! Current coordinate reference system
    QgsCoordinateReferenceSystem mCrs;
    QgsDistanceArea mDistanceArea;
    //! Lat1, lat2 used for geodesic distance calculation
    double mLat1 = 0;
    double mLat2 = 0;
};

#endif // QGSGRASSGISLIB_H
