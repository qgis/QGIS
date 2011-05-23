/***************************************************************************
    qgsgrass.h  -  Data provider for GRASS format
                             -------------------
    begin                : March, 2004
    copyright            : (C) 2004 by Radim Blazek
    email                : blazek@itc.it
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSGRASS_H
#define QGSGRASS_H

// GRASS header files
extern "C"
{
#include <grass/gis.h>
#include <grass/form.h>
}

#include <stdexcept>
#include "qgsexception.h"
#include <QProcess>
#include <QString>
#include <QMap>
#include <QHash>
#include <QTemporaryFile>
class QgsCoordinateReferenceSystem;
class QgsRectangle;

/*!
   Methods for C library initialization and error handling.
*/
class QgsGrass
{
  public:
    // This does not work (gcc/Linux), such exception cannot be caught
    // so I have enabled the old version, if you are able to fix it, please
    // check first if it realy works, i.e. can be caught!
    /*
    struct Exception : public QgsException
    {
      Exception( const QString &msg ) : QgsException( msg ) {}
    };
    */
    struct Exception : public std::runtime_error
    {
      //Exception( const std::string &msg ) : std::runtime_error( msg ) {}
      Exception( const QString &msg ) : std::runtime_error( msg.toUtf8().constData() ) {}
    };

    struct Color
    {
      double value1, value2;
      int red1, red2, green1, green2, blue1, blue2;
    };

    //! Get info about the mode
    /*! QgsGrass may be running in active or passive mode.
     *  Active mode means that GISRC is set up and GISRC file is available,
     *  in that case default GISDBASE, LOCATION and MAPSET may be read by GetDefaul*() functions.
     *  Passive mode means, that GISRC is not available. */
    static GRASS_EXPORT bool activeMode( void );

    //! Get default GISDBASE, returns GISDBASE name or empty string if not in active mode
    static GRASS_EXPORT QString getDefaultGisdbase( void );

    //! Get default LOCATION_NAME, returns LOCATION_NAME name or empty string if not in active mode
    static GRASS_EXPORT QString getDefaultLocation( void );

    //! Get default MAPSET, returns MAPSET name or empty string if not in active mode
    static GRASS_EXPORT QString getDefaultMapset( void );

    //! Init or reset GRASS library
    /*!
    \param gisdbase full path to GRASS GISDBASE.
    \param location location name (not path!).
    */
    static GRASS_EXPORT void setLocation( QString gisdbase, QString location );

    /*!
    \param gisdbase full path to GRASS GISDBASE.
    \param location location name (not path!).
    \param mapset current mupset. Note that some variables depend on mapset and
               may influence behaviour of some functions (e.g. search path etc.)
    */
    static GRASS_EXPORT void setMapset( QString gisdbase, QString location, QString mapset );

    //! Error codes returned by error()
    enum GERROR { OK, /*!< OK. No error. */
                  WARNING /*!< Warning, non fatal error. Should be printed by application. */
              };

    //! Map type
    enum MapType { None, Raster, Vector, Region };

    //! Reset error code (to OK). Call this before a piece of code where an error is expected
    static GRASS_EXPORT void resetError( void );  // reset error status

    //! Check if any error occured in lately called functions. Returns value from ERROR.
    static GRASS_EXPORT int error( void );

    //! Get last error message
    static GRASS_EXPORT QString errorMessage( void );

    /** \brief Open existing GRASS mapset
     * \return NULL string or error message
     */
    static GRASS_EXPORT QString openMapset( QString gisdbase,
                                            QString location, QString mapset );

    /** \brief Close mapset if it was opened from QGIS.
     *         Delete GISRC, lock and temporary directory
     * \return NULL string or error message
     */
    static GRASS_EXPORT QString closeMapset();

    //! Check if given directory contains a GRASS installation
    static GRASS_EXPORT bool isValidGrassBaseDir( QString const gisBase );

    //! Returns list of locations in given gisbase
    static QStringList GRASS_EXPORT locations( QString gisbase );

    //! Returns list of mapsets in location
    static GRASS_EXPORT QStringList mapsets( QString gisbase, QString locationName );
    static GRASS_EXPORT QStringList mapsets( QString locationPath );

    //! List of vectors and rasters
    static GRASS_EXPORT QStringList vectors( QString gisbase, QString locationName,
        QString mapsetName );
    static GRASS_EXPORT QStringList vectors( QString mapsetPath );

    static GRASS_EXPORT QStringList rasters( QString gisbase, QString locationName,
        QString mapsetName );
    static GRASS_EXPORT QStringList rasters( QString mapsetPath );

    //! Get list of vector layers
    static QStringList vectorLayers( QString, QString, QString, QString );
    
    //! List of elements
    static GRASS_EXPORT QStringList elements( QString gisbase, QString locationName,
        QString mapsetName, QString element );
    static GRASS_EXPORT QStringList elements( QString mapsetPath, QString element );

    // ! Get map region
    static GRASS_EXPORT bool mapRegion( int type, QString gisbase,
                                        QString location, QString mapset, QString map,
                                        struct Cell_head *window );

    // ! String representation of region
    static GRASS_EXPORT QString regionString( struct Cell_head *window );

    // ! Read current mapset region
    static GRASS_EXPORT bool region( QString gisbase, QString location, QString mapset,
                                     struct Cell_head *window );

    // ! Write current mapset region
    static GRASS_EXPORT bool writeRegion( QString gisbase, QString location, QString mapset,
                                          struct Cell_head *window );

    // ! Set (copy) region extent, resolution is not changed
    static GRASS_EXPORT void copyRegionExtent( struct Cell_head *source,
        struct Cell_head *target );

    // ! Set (copy) region resolution, extent is not changed
    static GRASS_EXPORT void copyRegionResolution( struct Cell_head *source,
        struct Cell_head *target );

    // ! Extend region in target to source
    static GRASS_EXPORT void extendRegion( struct Cell_head *source,
                                           struct Cell_head *target );

    static GRASS_EXPORT void init( void );

    // ! test if the directory is mapset
    static GRASS_EXPORT bool isMapset( QString path );

    // ! Get the lock file
    static GRASS_EXPORT QString lockFilePath();

    // ! Get current gisrc path
    static GRASS_EXPORT QString gisrcFilePath();

    // ! Start a GRASS module in any gisdbase/location
    static GRASS_EXPORT QProcess *startModule( QString gisdbase, QString location, QString module, QStringList arguments, QTemporaryFile &gisrcFile );

    // ! Run a GRASS module in any gisdbase/location
    static GRASS_EXPORT QByteArray runModule( QString gisdbase, QString location, QString module, QStringList arguments );

    // ! Get info string from qgis.g.info module
    static GRASS_EXPORT QString getInfo( QString info, QString gisdbase,
                                         QString location, QString mapset = "", QString map = "", MapType type = None, double x = 0.0, double y = 0.0 );

    // ! Get location projection
    static GRASS_EXPORT QgsCoordinateReferenceSystem crs( QString gisdbase, QString location );

    // ! Get location projection calling directly GRASS library
    static GRASS_EXPORT QgsCoordinateReferenceSystem crsDirect( QString gisdbase, QString location );

    // ! Get map extent
    static GRASS_EXPORT QgsRectangle extent( QString gisdbase, QString location,
        QString mapset, QString map, MapType type = None );

    // ! Get raster map size
    static GRASS_EXPORT void size( QString gisdbase, QString location,
                                   QString mapset, QString map, int *cols, int *rows );

    // ! Get raster info
    static GRASS_EXPORT QHash<QString, QString> info( QString gisdbase, QString location,
        QString mapset, QString map, MapType type );

    // ! List of Color
    static GRASS_EXPORT QList<QgsGrass::Color> colors( QString gisdbase, QString location,
        QString mapset, QString map );

    // ! Get map value / feautre info
    static GRASS_EXPORT QMap<QString, QString> query( QString gisdbase, QString location,
        QString mapset, QString map, MapType type, double x, double y );

    //! Library version
    static GRASS_EXPORT int versionMajor();
    static GRASS_EXPORT int versionMinor();
    static GRASS_EXPORT int versionRelease();
    static GRASS_EXPORT QString versionString();

    // set environment variable
    static GRASS_EXPORT void putEnv( QString name, QString value );

#if defined(WIN32)
    static GRASS_EXPORT QString shortPath( const QString &path );
#endif

  private:
    static int initialized; // Set to 1 after initialization
    static bool active; // is active mode
    static QString defaultGisdbase;
    static QString defaultLocation;
    static QString defaultMapset;

    /* last error in GRASS libraries */
    static GERROR lastError;         // static, because used in constructor
    static QString error_message;

    // G_set_error_routine has two versions of the function's first argument it expects:
    // - char* msg - older version
    // - const char* msg - in CVS from 04/2007
    // this way compiler chooses suitable call
    static int error_routine( const char *msg, int fatal ); // static because pointer to this function is set later
    static int error_routine( char *msg, int fatal ); // static because pointer to this function is set later

    // Current mapset lock file path
    static QString mMapsetLock;
    // Current mapset GISRC file path
    static QString mGisrc;
    // Temporary directory where GISRC and sockets are stored
    static QString mTmp;
};

#endif // QGSGRASS_H
